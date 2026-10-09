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
#   - weather ... <nicht ok> Wetterabruf gescheitert, oder ms >= 1000 (ESP ab Paket 2026-10-09)
#   eep ... ms=<n>          EEPROM-Schreibvorgang ab 200 ms (STM, Messzeile M.1)
#   cmd abgewiesen          Kommando mit falscher Pruefsumme verworfen (STM, Teil D)
#
# AUSSCHLIESSLICH LESEND. Das Skript spricht nicht mit dem Geraet, es liest nur mit.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

# Marke fuer den Stop-Hook: hier wurde am GERAET gemessen. Er fragt beim Beenden
# nach, ob die Erkenntnis in BEFUNDE.md steht, falls die Datei seither unberuehrt
# blieb. Grund: Am 04.10.2026 musste der Nutzer dreimal nachfragen (L184).
[ -z "${WATCHLOG_QUELLE:-}" ] && mkdir -p "$(git rev-parse --git-dir 2>/dev/null)" 2>/dev/null \
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

# WATCHLOG_QUELLE ersetzt den Mitschnitt durch ein Skript, das wie log.sh auf "tail <n>"
# antwortet -- nur fuer die Gegenprobe der Meldungen (DIR-014), nie im Geraetelauf.
LOG=${WATCHLOG_QUELLE:-./tools/logger/log.sh}
[ -x "$LOG" ] || { echo "tools/logger/log.sh fehlt oder ist nicht ausfuehrbar." >&2; exit 2; }

# Ausgangsstand, damit nur NEUES gemeldet wird. Ohne diese Grundlinie meldet der
# erste Durchgang die gesamte Vorgeschichte und man gewoehnt sich das Wegsehen an.
basis_exc=$("$LOG" tail 3000 2>/dev/null | grep -ac 'Exception (29)')
basis_rst=$("$LOG" tail 3000 2>/dev/null | grep -ac 'rst cause')
basis_wdt=$("$LOG" tail 3000 2>/dev/null | grep -aci 'wdt reset')
letzte_d=""
letzte_a=""

# Ereigniszeilen (Wetter, EEPROM, Abweisung) stehen einzeln im Mitschnitt, nicht als
# Zaehler. Jede wird genau einmal gemeldet: Was beim Start schon im Fenster steht, ist
# Grundlinie; danach merkt sich die Wache jede gemeldete Zeile samt Zeitstempel.
EREIGNIS_MUSTER='- weather fc=|(^|[^A-Za-z])eep a=[0-9]|cmd abgewiesen'
GESEHEN="$(git rev-parse --git-dir 2>/dev/null)/watch-log.gesehen"
"$LOG" tail 3000 2>/dev/null | grep -aE -e "$EREIGNIS_MUSTER" > "$GESEHEN" || true
letzte_vt=""
letzte_diag=""
still_sek=0   # wie lange diag schon auf demselben Wert steht (N2, L321)

# Ohne das puffert die Shell die Ausgabe, wenn sie in eine Datei oder Pipe geht --
# und dann steht dort stundenlang NICHTS, obwohl die Wache laeuft. Am 04.10.2026 genau
# so passiert: Die Wache lief waehrend eines Testlaufs, ihre Ausgabedatei blieb bei
# 0 Byte, und der Lead meldete "keine Abstuerze", ohne etwas gesehen zu haben. Der
# Nutzer hat es bemerkt. Eine Wache, deren Meldungen niemanden erreichen, ist keine.
#
# NACHTRAG 04.10.2026 (L194): Das allein reicht NICHT. Beim OTA auf 3.2.19 blieb die
# Ausgabe erneut leer, und diesmal lag es am Aufruf:
#
#     ./tools/watch-log.sh --interval 20 2>&1 | tail -40     <-- SO NICHT
#
# `tail -N` sammelt und gibt erst aus, wenn der Stream endet. Bei einer Wache, die
# bis Strg-C laeuft, endet er nie. Viermal in einer Sitzung so aufgerufen, viermal
# folgenlos -- und jedes Mal wurde berichtet, die Wache sei mitgelaufen.
#
# Ein Melder darf nicht davon abhaengen, wie der Aufrufer ihn pipet. Deshalb geht
# jede Meldung ZUSAETZLICH in eine feste Datei, an stdout und jeder Pipe vorbei.
MITSCHRIFT="$(git rev-parse --git-dir 2>/dev/null)/watch-log.out"
exec > >(while IFS= read -r z; do
           printf '%s\n' "$z"
           printf '%s\n' "$z" >> "$MITSCHRIFT"
         done) 2>&1

# Warnen, wenn stdout keine Konsole ist -- dann ist die Mitschrift der einzige Weg,
# an die Meldungen zu kommen, und das soll derjenige wissen, der sie startet.
if [ ! -t 1 ]; then
  printf 'Hinweis: stdout ist keine Konsole. Meldungen stehen in %s\n\n' "$MITSCHRIFT"
fi

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

  # NUR aus der letzten diag-Zeile lesen, nicht aus dem ganzen Mitschnitt.
  #
  # Bis zum 05.10.2026 suchte die Wache 'd=[0-9]+' ueber alle Zeilen. Das traf auch
  # Request-Zeilen: Um 23:14:00 meldete sie "diag 136 d=20", die Rohzeile hatte d=0 --
  # gelesen war "re*d=20*" aus display_color_set?red=20. Umgekehrt verdeckt eine
  # spaetere Zeile mit "...d=<klein>" einen echten Sprung und setzt letzte_d falsch
  # zurueck. Die Wache haette damit genau beim Setterverkehr, fuer den sie laeuft, ein
  # falsches "ruhig" melden koennen (L-Befund aus dem Testdurchlauf, DIR-014).
  diag_zeile=$(printf '%s' "$roh" | grep -aE '(^|[^A-Za-z])diag [0-9]+ ' | tail -1)
  d=$(printf '%s' "$diag_zeile" | grep -aoE '(^| )d=[0-9]+' | tail -1 | cut -d= -f2)
  v=$(printf '%s' "$diag_zeile" | grep -aoE '(^| )v=[0-9]+/[0-9]+' | tail -1 | sed 's/^ //')
  # a= seit STM 3.2.22 (S.16): Quittungen, deren Zuordnung nicht zum wartenden Kommando
  # passte und die deshalb verworfen wurden. Auf einer gesunden Bruecke bleibt das 0;
  # jeder Anstieg heisst, eine Quittung kam zu spaet oder gehoerte einem anderen Kommando.
  # Aeltere STM fuehren das Feld nicht -- dann bleibt a leer und nichts wird gemeldet.
  a=$(printf '%s' "$diag_zeile" | grep -aoE '(^| )a=[0-9]+' | tail -1 | cut -d= -f2)
  diag=$(printf '%s' "$diag_zeile" | grep -aoE 'diag [0-9]+' | tail -1 | awk '{print $2}')

  stamp=$(date '+%H:%M:%S')
  alarm=0

  # Neue Ereigniszeilen. grep -Fxv -f mit leerer Musterdatei liefert nichts -- deshalb
  # eine Zeile, die im Mitschnitt nie vorkommt, als Platzhalter.
  [ -s "$GESEHEN" ] || printf '%s\n' '#watch-log-leer#' > "$GESEHEN"
  neu=$(printf '%s' "$roh" | grep -aE -e "$EREIGNIS_MUSTER" | grep -aFxv -f "$GESEHEN")
  if [ -n "$neu" ]; then
    printf '%s\n' "$neu" >> "$GESEHEN"
    while IFS= read -r z; do
      case "$z" in
        *"- weather fc="*)
          # - weather fc=<0|1> ms=<n> <ok|dns|connfail|timeout|leer>   (design.md §1.4)
          wms=$(printf '%s' "$z" | grep -aoE 'ms=[0-9]+' | head -1 | cut -d= -f2)
          werg=$(printf '%s' "$z" | sed -E 's/.*- weather fc=[0-9]+ ms=[0-9]+ ([a-z]+).*/\1/')
          if [ "$werg" != "ok" ] || [ "${wms:-0}" -ge 1000 ]; then
            printf '  %s  Wetter: %s nach %s ms   <%s>\n' "$stamp" "$werg" "${wms:-?}" "$z"
            alarm=1
          fi ;;
        *"cmd abgewiesen"*)
          printf '  %s  Bruecke: Kommando abgewiesen   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *)
          # eep a=<adr> n=<cnt> z=<zyklen> ms=<dauer> d=<vorher>/<nachher>  (design.md §2.2)
          ems=$(printf '%s' "$z" | grep -aoE ' ms=[0-9]+' | head -1 | cut -d= -f2)
          if [ "${ems:-0}" -ge 200 ]; then
            printf '  %s  EEPROM: Schreibvorgang %s ms   <%s>\n' "$stamp" "$ems" "$z"
            alarm=1
          fi ;;
      esac
    done <<< "$neu"
  fi

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

  # Ein EINZELNER Anstieg ist auch im gesunden Betrieb moeglich: Loest ein ESP-Kommando
  # waehrend eines Abgleichs ein verschachteltes var aus, trifft dessen Quittung das
  # naechste Kommando und wird richtigerweise verworfen (Review S.21, H3). Deshalb erst
  # ab drei je Abfrage Alarm; darunter nur ein Hinweis, damit man es trotzdem sieht.
  if [ -n "$a" ] && [ -n "$letzte_a" ] && [ "$a" -gt "$letzte_a" ]; then
    if [ "$((a - letzte_a))" -ge 3 ]; then
      printf '  %s  Bruecke: %d Quittung(en) verworfen, Zuordnung passte nicht (a=%s)\n' "$stamp" "$((a - letzte_a))" "$a"
      alarm=1
    else
      printf '  %s  Hinweis: %d Quittung verworfen (a=%s), einzeln unbedenklich\n' "$stamp" "$((a - letzte_a))" "$a"
    fi
  fi
  [ -n "$a" ] && letzte_a=$a

  # v=<timeouts>/<verschachtelt>: var_send-Zeitueberschreitungen des STM. Der Kopf sagte
  # "Sprung in v= -- Variablenverluste" seit seiner Entstehung zu, ausgewertet wurde v
  # aber nie -- es stand nur in der Ruhezeile. Im Testdurchlauf S.26 (08.10.2026) liefen
  # zwei Anstiege (v=0 -> 1 -> 2, je ein Wetterabruf) als "ruhig" durch. Eine Zusage im
  # Kopf, die der Code nicht haelt, ist dieselbe Gattung wie DIR-014.
  vt=${v#v=}; vt=${vt%%/*}
  if [ -n "$vt" ] && [ -n "$letzte_vt" ] && [ "$vt" -gt "$letzte_vt" ] 2>/dev/null; then
    printf '  %s  Bruecke: %d var_send-Zeitueberschreitung(en) (v=%s)\n' "$stamp" "$((vt - letzte_vt))" "${v#v=}"
    alarm=1
  fi
  [ -n "$vt" ] && letzte_vt=$vt

  # Luecke in der diag-Folge: der zeitgesteuerte Zweig des STM hat ausgesetzt.
  # Genau das Bild aus dem mitgeschnittenen Haenger (L14) -- Hauptloop laeuft,
  # periodischer Zweig nicht.
  if [ -n "$diag" ] && [ -n "$letzte_diag" ]; then
    erwartet=$((letzte_diag + INTERVAL / 10))
    # Ein KLEINERER Wert ist kein Stillstand, sondern ein STM-Neustart: Die Folge beginnt
    # danach wieder bei 1. Bis zum 05.10.2026 lief beides in denselben Zweig, und jeder
    # Flash erzeugte einen Fehlalarm "diag steht still bei 1" (L295). Ein Neustart ist
    # selbst meldenswert -- aber als Neustart, nicht als Hänger.
    if [ "$diag" -lt "$letzte_diag" ]; then
      printf '  %s  *** STM neu gestartet *** diag %s -> %s\n' "$stamp" "$letzte_diag" "$diag"
      alarm=1
    elif [ "$diag" -eq "$letzte_diag" ]; then
      # Erst ab 30 s Stillstand melden, nicht nach EINER Abfrage ohne neue Zeile. Die
      # diag-Zeilen kommen rund alle 10 s; liefert der ESP unter Seitenlast eine davon
      # verspaetet aus, sieht eine einzelne Abfrage denselben Wert. Am 05.10.2026 genau
      # so: "steht still bei 359", waehrend check-pwa.sh alle Module lud -- im Ring
      # folgten 360 und 361 lueckenlos (L321). Ein Fehlalarm im Testfenster gewoehnt
      # ans Leitsymptom von L14/L204, und dann wird der echte Haenger ueberlesen.
      still_sek=$((still_sek + INTERVAL))
      if [ "$still_sek" -ge 30 ]; then
        printf '  %s  *** diag steht still bei %s seit %s s *** periodischer Zweig ausgefallen?\n' "$stamp" "$diag" "$still_sek"
        alarm=1
      fi
    else
      still_sek=0
    fi
  fi
  [ -n "$diag" ] && letzte_diag=$diag

  [ "$alarm" -eq 0 ] && printf '  %s  ruhig   diag %s  d=%s  %s%s\n' "$stamp" "${diag:-?}" "${d:-?}" "${v:-}" "${a:+  a=$a}"

  [ "$ONCE" -eq 1 ] && break
  sync 2>/dev/null
  sleep "$INTERVAL"
done
