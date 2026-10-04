#!/usr/bin/env bash
# Ueberwacht den seriellen Mitschnitt waehrend eines Testlaufs und meldet, was zaehlt.
#
#   ./tools/watch-log.sh                 laeuft bis Strg-C, meldet alle 30 s
#   ./tools/watch-log.sh --once          ein Durchgang, dann Ende
#   ./tools/watch-log.sh --interval 15   anderer Abstand
#
# WARUM ES DIESES SKRIPT GIBT
#
# Am 03.10.2026 lief ein vollstaendiger Testdurchlauf, und mitten darin stuerzte der
# ESP ab -- Exception 29 nach GET /api/update_status. Bemerkt hat es der NUTZER im
# Mitschnitt, nicht die Pruefung: Waehrend des Tests schaute niemand in die Logs.
# Seine Forderung danach, woertlich: "Zudem erwarte ich von dir, dass du eigentlich
# waehrend der Tests die Logs ueberwachst und auch laufend auswertest wenn du testest!"
#
# Der Punkt ist nicht, dass ein Absturz uebersehen wurde, sondern WANN er auffiel:
# Ein Testbericht, der sauber meldet, waehrend das Geraet zwischendurch neu gestartet
# ist, ist schlimmer als kein Bericht -- er erzeugt Vertrauen, das nicht gedeckt ist.
#
# WAS GEMELDET WIRD
#
#   Exception / rst cause   Absturz mit Neustart
#   wdt reset               Watchdog -- seit STM 3.2.8 waere das ein Rueckfall
#   Luecke in diag          der zeitgesteuerte Zweig des STM steht (Haenger aus L14)
#   Sprung in d=            verworfene Zeichen auf der Bruecke, also Ueberlastung
#   Sprung in v=            Variablenverluste
#
# AUSSCHLIESSLICH LESEND. Das Skript spricht nicht mit dem Geraet, es liest nur mit.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

# Marke fuer den Stop-Hook: hier wurde am GERAET gemessen. Er fragt beim Beenden
# nach, ob die Erkenntnis in BEFUNDE.md steht, falls die Datei seither unberuehrt
# blieb. Grund: Am 04.10.2026 musste der Nutzer dreimal nachfragen (L184).
mkdir -p "$(git rev-parse --git-dir 2>/dev/null)" 2>/dev/null \
  && touch "$(git rev-parse --git-dir)/geraet-gemessen" 2>/dev/null || true

ONCE=0
INTERVAL=30
while [ $# -gt 0 ]; do
  case "$1" in
    --once) ONCE=1; shift ;;
    --interval) INTERVAL="${2:-30}"; shift 2 ;;
    *) echo "Unbekannte Option: $1" >&2; exit 2 ;;
  esac
done

LOG=./tools/logger/log.sh
[ -x "$LOG" ] || { echo "tools/logger/log.sh fehlt oder ist nicht ausfuehrbar." >&2; exit 2; }

# Ausgangsstand, damit nur NEUES gemeldet wird. Ohne diese Grundlinie meldet der
# erste Durchgang die gesamte Vorgeschichte und man gewoehnt sich das Wegsehen an.
basis_exc=$("$LOG" tail 3000 2>/dev/null | grep -ac 'Exception (29)')
basis_rst=$("$LOG" tail 3000 2>/dev/null | grep -ac 'rst cause')
basis_wdt=$("$LOG" tail 3000 2>/dev/null | grep -aci 'wdt reset')
letzte_d=""
letzte_diag=""

# Ohne das puffert die Shell die Ausgabe, wenn sie in eine Datei oder Pipe geht --
# und dann steht dort stundenlang NICHTS, obwohl die Wache laeuft. Am 04.10.2026 genau
# so passiert: Die Wache lief waehrend eines Testlaufs, ihre Ausgabedatei blieb bei
# 0 Byte, und der Lead meldete "keine Abstuerze", ohne etwas gesehen zu haben. Der
# Nutzer hat es bemerkt. Eine Wache, deren Meldungen niemanden erreichen, ist keine.
exec > >(while IFS= read -r z; do printf '%s\n' "$z"; done) 2>&1

printf '=== Logwache ab %s ===\n' "$(date '+%H:%M:%S')"
printf '  Grundlinie: %s Exceptions, %s Neustarts, %s Watchdog-Resets im Fenster\n\n' \
  "$basis_exc" "$basis_rst" "$basis_wdt"

runde=0
while :; do
  runde=$((runde+1))
  roh=$("$LOG" tail 3000 2>/dev/null)

  exc=$(printf '%s' "$roh" | grep -ac 'Exception (29)')
  rst=$(printf '%s' "$roh" | grep -ac 'rst cause')
  wdt=$(printf '%s' "$roh" | grep -aci 'wdt reset')

  d=$(printf '%s' "$roh" | grep -aoE 'd=[0-9]+' | tail -1 | cut -d= -f2)
  v=$(printf '%s' "$roh" | grep -aoE 'v=[0-9]+/[0-9]+' | tail -1)
  diag=$(printf '%s' "$roh" | grep -aoE 'diag [0-9]+' | tail -1 | awk '{print $2}')

  stamp=$(date '+%H:%M:%S')
  alarm=0

  if [ "$exc" -gt "$basis_exc" ]; then
    printf '  %s  *** %d NEUE Exception(s) ***\n' "$stamp" "$((exc - basis_exc))"
    printf '%s' "$roh" | grep -aB3 'Exception (29)' | grep -aE 'request|GET|POST' | tail -2 | sed 's/^/             davor: /'
    printf '%s' "$roh" | grep -aA2 'Exception (29)' | grep -aE 'epc1|ctx:' | tail -2 | sed 's/^/             /'
    basis_exc=$exc; alarm=1
  fi

  if [ "$wdt" -gt "$basis_wdt" ]; then
    printf '  %s  *** WATCHDOG-RESET *** seit STM 3.2.8 waere das ein Rueckfall\n' "$stamp"
    basis_wdt=$wdt; alarm=1
  fi

  if [ "$rst" -gt "$basis_rst" ]; then
    printf '  %s  *** %d Neustart(s) ***\n' "$stamp" "$((rst - basis_rst))"
    basis_rst=$rst; alarm=1
  fi

  # Verworfene Zeichen: nicht der Absolutwert zaehlt, sondern der Sprung. Jeder
  # ESP-Neustart kostet rund 538 (L125); mehr heisst Ueberlastung der Bruecke.
  if [ -n "$d" ] && [ -n "$letzte_d" ] && [ "$d" -gt "$letzte_d" ]; then
    delta=$((d - letzte_d))
    if [ "$delta" -gt 100 ]; then
      printf '  %s  Bruecke: %d Zeichen verworfen (d=%s), %s\n' "$stamp" "$delta" "$d" \
        "$([ "$delta" -gt 500 ] && echo 'ueber einem ESP-Neustart-Mass' || echo 'Ueberlastung')"
      alarm=1
    fi
  fi
  [ -n "$d" ] && letzte_d=$d

  # Luecke in der diag-Folge: der zeitgesteuerte Zweig des STM hat ausgesetzt.
  # Genau das Bild aus dem mitgeschnittenen Haenger (L14) -- Hauptloop laeuft,
  # periodischer Zweig nicht.
  if [ -n "$diag" ] && [ -n "$letzte_diag" ]; then
    erwartet=$((letzte_diag + INTERVAL / 10))
    if [ "$diag" -lt "$((letzte_diag + 1))" ]; then
      printf '  %s  *** diag steht still bei %s *** periodischer Zweig ausgefallen?\n' "$stamp" "$diag"
      alarm=1
    fi
  fi
  [ -n "$diag" ] && letzte_diag=$diag

  [ "$alarm" -eq 0 ] && printf '  %s  ruhig   diag %s  d=%s  %s\n' "$stamp" "${diag:-?}" "${d:-?}" "${v:-}"

  [ "$ONCE" -eq 1 ] && break
  sync 2>/dev/null
  sleep "$INTERVAL"
done
