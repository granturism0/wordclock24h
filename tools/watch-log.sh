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
#   - weather ... oom>0      Koerper unvollstaendig: Zeichen verloren oder n != cl (ESP ab 3.2.29, C54)
#   eep ... ms=<n>          EEPROM-Schreibvorgang ab 200 ms (STM, Messzeile M.1)
#   cmd abgewiesen          Kommando mit falscher Pruefsumme verworfen (STM, Teil D)
#   ds18xx verloren/ungueltig  Temperatursensor: Lese-/CRC-Fehler (STM, A2)
#   - var ignoriert / - ntp verworfen   ESP verwirft Setter bzw. NTP-Antwort (ESP ab Paket 2026-10-11)
#   ovr= in der Heap-Zeile  Anstieg der UART-Ueberlaeufe im ESP (A21)
#   - ovr #n t=ms          Zeitpunkt eines erkannten UART-Ueberlaufs im ESP (A76), Hinweis
#   icon: verworfen ...    Icon bei Display aus verworfen (A80), Hinweis
#   icon_freeze: ENTER     Icon wartet auf das Display -- seit A80 ein Alarm
#   i2c: frist abgelaufen   I2C-Zugriff ueber 5 ms, Bus zurueckgesetzt (STM ab Paket 2026-10-11, A60)
#   time: ungueltig         Netzzeit verworfen (STM ab Paket 2026-10-11, A62)
#   time: sprung            Bestaetigung angefordert -- Hinweis; Alarm ab der vierten in Folge
#                           ohne "cmd: set time to" dazwischen (Server liefert dauerhaft Unsinn)
#   Hinweise                night: start aus / sprung aus|ein (A69), spiel: n Zeilen verworfen (A53),
#                           ldr: minimum/maximum abgewiesen (A57), time: zeitzone ungueltig
#
# Zeilen mit "%" werden uebergangen: Beim STM-Flashen laeuft das Firmware-Abbild durch den
# Mitschnitt, und dessen Formatzeichenketten ("cmd abgewiesen #%u len=%u") loesten am
# 10.10.2026 um 21:21 und 21:54 Fehlalarme aus (Paket 2026-10-11, beim Bau von abgleich.sh).
#
# AUSSCHLIESSLICH LESEND. Das Skript spricht nicht mit dem Geraet, es liest nur mit.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

# E36 (L423): Herzschlag. Jede Runde schreibt "Epoche Intervall Runde" in diese Datei.
# "./tools/watch-log.sh --pruefe" sagt von AUSSEN, ob die Wache lebt: Am 12.10.2026 lief
# der Prozess weiter und schrieb von 00:20 bis 01:33 nichts -- niemand konnte es sehen,
# weil die einzige Instanz, die es haette melden koennen, die stehende Wache selbst war.
WL_SFX=${WATCHLOG_QUELLE:+.probe}
LEBT="$(git rev-parse --git-dir 2>/dev/null)/watch-log.lebt$WL_SFX"
if [ "${1:-}" = "--pruefe" ]; then
  [ -s "$LEBT" ] || { echo "Logwache: kein Herzschlag ($LEBT fehlt) -- laeuft keine Wache?"; exit 1; }
  # Zeile: Epoche Intervall Runde PID Frist. Grenze wie in der Wache selbst (Review 0.2, M2).
  read -r t iv rd wpid fr < "$LEBT" 2>/dev/null
  case "$t$iv$rd${wpid:-0}${fr:-0}" in *[!0-9]*|"") echo "Logwache: Herzschlagdatei unlesbar ($LEBT)"; exit 1;; esac
  fr=${fr:-20}
  alter=$(( $(date +%s) - t ))
  grenze=$(( 3 * iv > iv + fr + 10 ? 3 * iv : iv + fr + 10 ))
  um=$(date -r "$t" '+%H:%M:%S' 2>/dev/null || echo "$t")
  if [ -n "${wpid:-}" ] && ! kill -0 "$wpid" 2>/dev/null; then
    echo "Logwache: BEENDET (PID $wpid lebt nicht), letzte Runde $rd um $um -- seither ist nichts beobachtet"
    exit 3
  fi
  if [ "$alter" -gt "$grenze" ]; then
    echo "Logwache: letzter Herzschlag vor ${alter} s (Runde $rd um $um, Intervall ${iv} s, Grenze ${grenze} s) -- die Wache STEHT"
    exit 1
  fi
  echo "Logwache: lebt, letzter Herzschlag vor ${alter} s (Runde $rd, Intervall ${iv} s, PID ${wpid:-?})"
  exit 0
fi

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
MITSCHRIFT="$(git rev-parse --git-dir 2>/dev/null)/watch-log.out$WL_SFX"
KONSOLE=0; [ -t 1 ] && KONSOLE=1
exec > >(while IFS= read -r z; do
           printf '%s\n' "$z"
           printf '%s\n' "$z" >> "$MITSCHRIFT"
         done) 2>&1

# Warnen, wenn stdout keine Konsole ist -- dann ist die Mitschrift der einzige Weg,
# an die Meldungen zu kommen, und das soll derjenige wissen, der sie startet.
if [ "$KONSOLE" -eq 0 ]; then
  printf 'Hinweis: stdout ist keine Konsole. Meldungen stehen in %s\n\n' "$MITSCHRIFT"
fi

# E36 (L423): Jeder Abruf des Mitschnitts hat eine Frist. log.sh spricht per ssh mit dem
# Pi; eine bestehende Verbindung kann nach einem Wechsel der Netzschnittstelle unbegrenzt
# haengen, und die Wache wartete synchron darauf. macOS hat kein "timeout", deshalb
# laeuft der Abruf im Hintergrund und wird nach FRIST Sekunden beendet.
FRIST=${WATCHLOG_FRIST:-20}
# Endet die Wache mitten in einem Abruf, laufen log.sh und ssh sonst als Waisen weiter.
HOLE_PID=""
HOLE_TMP=""
trap '[ -n "$HOLE_PID" ] && { pkill -P "$HOLE_PID" 2>/dev/null; kill "$HOLE_PID" 2>/dev/null; }; [ -n "$HOLE_TMP" ] && rm -f "${HOLE_TMP:?}"; exit' TERM INT HUP
HOLE_OUT=""
HOLE_HAENGT=0
hole() {
  local tmp pid n=0
  tmp=$(mktemp "${TMPDIR:-/tmp}/watchlog.XXXXXX") || { HOLE_OUT=""; HOLE_HAENGT=1; return 2; }
  HOLE_TMP=$tmp
  "$LOG" "$@" > "$tmp" 2>/dev/null &
  pid=$!
  HOLE_PID=$pid
  HOLE_HAENGT=0
  while kill -0 "$pid" 2>/dev/null; do
    if [ "$n" -ge $((FRIST * 10)) ]; then
      pkill -P "$pid" 2>/dev/null; kill "$pid" 2>/dev/null
      sleep 0.2; pkill -9 -P "$pid" 2>/dev/null; kill -9 "$pid" 2>/dev/null
      HOLE_HAENGT=1
      break
    fi
    sleep 0.1; n=$((n + 1))
  done
  wait "$pid" 2>/dev/null
  HOLE_PID=""
  HOLE_OUT=$(cat "$tmp")
  rm -f "${tmp:?}"
  HOLE_TMP=""
}

# Ausgangsstand, damit nur NEUES gemeldet wird. Ohne diese Grundlinie meldet der
# erste Durchgang die gesamte Vorgeschichte und man gewoehnt sich das Wegsehen an.
hole tail 3000
basis_fehlt=$HOLE_HAENGT
basis_roh=$HOLE_OUT
basis_exc=$(printf '%s' "$basis_roh" | grep -ac 'Exception (29)')
basis_rst=$(printf '%s' "$basis_roh" | grep -ac 'rst cause')
basis_wdt=$(printf '%s' "$basis_roh" | grep -aci 'wdt reset')
letzte_d=""
letzte_a=""

# Ereigniszeilen (Wetter, EEPROM, Abweisung) stehen einzeln im Mitschnitt, nicht als
# Zaehler. Jede wird genau einmal gemeldet: Was beim Start schon im Fenster steht, ist
# Grundlinie; danach merkt sich die Wache jede gemeldete Zeile samt Zeitstempel.
EREIGNIS_MUSTER='- weather fc=|(^|[^A-Za-z])eep a=[0-9]|cmd abgewiesen|ds18xx verloren|ds18xx erkannt|DS18xxx temperature: ungueltig|time: ungueltig|time: sprung|time: zeitzone ungueltig|cmd: set time to|night: (start|sprung) (aus|ein)|spiel: [0-9]+ Zeilen verworfen|ldr: (minimum|maximum) abgewiesen|i2c: frist abgelaufen|- var ignoriert #|- ntp verworfen #|- ota: Rueckfall-Neustart|- heap free=[0-9]+ max=[0-9]+ ovr=[0-9]+|- ovr #[0-9]+ t=[0-9]+|icon: verworfen, display aus|icon_freeze: ENTER'
# Mit WATCHLOG_QUELLE (Gegenprobe) eigene Zustandsdateien: Sonst ueberschreibt ein Probelauf
# die Grundlinie einer parallel laufenden Geraetewache, und die meldet alte Zeilen neu (L410).
GESEHEN="$(git rev-parse --git-dir 2>/dev/null)/watch-log.gesehen$WL_SFX"
printf '%s\n' "$basis_roh" | grep -aE -e "$EREIGNIS_MUSTER" > "$GESEHEN" || true
letzte_vt=""
letzte_vn=""
sprung_folge=0
letzte_ovr=""
letzte_diag=""
still_sek=0   # wie lange diag schon auf demselben Wert steht (N2, L321)


# DIR-020/E36: Solange die Wache laeuft, darf der Mac nicht einschlafen. -i haelt das System
# wach (auch auf Akku, solange der Deckel offen ist); -w endet mit diesem Prozess.
WACH="ohne caffeinate"
if [ -z "${WATCHLOG_QUELLE:-}" ] && command -v caffeinate >/dev/null 2>&1; then
  caffeinate -i -w $$ >/dev/null 2>&1 &
  sleep 0.3
  if kill -0 $! 2>/dev/null; then WACH="caffeinate -i aktiv"; else WACH="caffeinate NICHT gestartet"; fi
fi
printf '=== Logwache ab %s (%s, Intervall %s s, Frist %s s, Herzschlag %s) ===\n' \
  "$(date '+%H:%M:%S')" "$WACH" "$INTERVAL" "$FRIST" "$LEBT"
if [ "$basis_fehlt" -eq 1 ]; then
  printf '  *** Grundlinie FEHLT: der erste Abruf des Mitschnitts hing ueber %s s *** -- sie wird mit der ersten erfolgreichen Runde gesetzt, bis dahin wird nichts als neu gemeldet\n\n' "$FRIST"
else
  printf '  Grundlinie: %s Exceptions, %s Neustarts, %s Watchdog-Resets im Fenster\n\n' \
    "$basis_exc" "$basis_rst" "$basis_wdt"
fi

runde=0
vorige_runde=""
# Grenze fuer die Selbstmeldung: dreimal das Intervall, mindestens aber Intervall + Frist
# + Spielraum -- ein Abruf darf bis zur Frist dauern, ohne als Stillstand zu gelten.
SELBST_GRENZE=$(( 3 * INTERVAL > INTERVAL + FRIST + 10 ? 3 * INTERVAL : INTERVAL + FRIST + 10 ))
selbst_alarm=0
while :; do
  runde=$((runde+1))
  jetzt_s=$(date +%s)
  if [ -n "$vorige_runde" ] && [ $((jetzt_s - vorige_runde)) -gt "$SELBST_GRENZE" ]; then
    printf '  %s  *** Wache hat %s s ausgesetzt (Grenze %s s) *** -- Mac im Ruhezustand, Prozess gestoppt? Dieses Fenster ist NICHT beobachtet\n' \
      "$(date '+%H:%M:%S')" "$((jetzt_s - vorige_runde))" "$SELBST_GRENZE"
    selbst_alarm=1
  fi
  vorige_runde=$jetzt_s
  # Atomar ersetzen (Review 0.2, N1): ein Leser darf nie eine leere oder halbe Zeile sehen.
  printf '%s %s %s %s %s\n' "$jetzt_s" "$INTERVAL" "$runde" "$$" "$FRIST" > "$LEBT.tmp" && mv -f "$LEBT.tmp" "$LEBT"
  hole tail 3000
  roh=$HOLE_OUT
  if [ "$HOLE_HAENGT" -eq 1 ]; then
    # Eine abgebrochene Runde wird NICHT ausgewertet (N4: halbe Zeilen gaeben Fehlalarme
    # und landeten gekuerzt in GESEHEN) und nie als "ruhig" gemeldet (M1).
    printf '  %s  *** Mitschnitt nicht erreichbar: Abruf hing ueber %s s und wurde abgebrochen *** -- Verbindung zum Pi? Diese Runde ist NICHT beobachtet\n' \
      "$(date '+%H:%M:%S')" "$FRIST"
    [ "$ONCE" -eq 1 ] && break
    sleep "$INTERVAL"; continue
  fi
  if [ "$basis_fehlt" -eq 1 ]; then
    basis_roh=$roh
    basis_exc=$(printf '%s' "$roh" | grep -ac 'Exception (29)')
    basis_rst=$(printf '%s' "$roh" | grep -ac 'rst cause')
    basis_wdt=$(printf '%s' "$roh" | grep -aci 'wdt reset')
    printf '%s\n' "$roh" | grep -aE -e "$EREIGNIS_MUSTER" > "$GESEHEN" || true
    basis_fehlt=0
    printf '  %s  Grundlinie nachgeholt: %s Exceptions, %s Neustarts, %s Watchdog-Resets im Fenster\n' \
      "$(date '+%H:%M:%S')" "$basis_exc" "$basis_rst" "$basis_wdt"
  fi

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
  alarm=$selbst_alarm      # eine Runde nach einem Aussetzer ist nie "ruhig" (M1)
  selbst_alarm=0

  # Neue Ereigniszeilen. grep -Fxv -f mit leerer Musterdatei liefert nichts -- deshalb
  # eine Zeile, die im Mitschnitt nie vorkommt, als Platzhalter.
  [ -s "$GESEHEN" ] || printf '%s\n' '#watch-log-leer#' > "$GESEHEN"
  neu=$(printf '%s' "$roh" | grep -aE -e "$EREIGNIS_MUSTER" | grep -aFxv -f "$GESEHEN")
  if [ -n "$neu" ]; then
    printf '%s\n' "$neu" >> "$GESEHEN"
    while IFS= read -r z; do
      # Text mit fuehrendem Leerzeichen (Stempel, dann vier statt drei): Binaerrest aus dem
      # Firmware-Abbild beim STM-Flashen. Am 11.10.2026 um 20:44 meldete die Wache so
      # "ds18xx verloren" aus dem Abbild von 3.2.29 -- ohne "%", am Filter darunter vorbei.
      # Nur fuer Ereigniszeilen: Die Reset-Flags ("     PINRST") sind echt und eingerueckt,
      # sie laufen hier nicht durch.
      case "$z" in *[0-9][0-9]:[0-9][0-9]"    "*) continue ;; esac
      case "$z" in
        *%*)
          # Formatzeichenkette aus dem Firmware-Abbild beim Flashen, keine echte Zeile
          ;;
        *"- var ignoriert #"*)
          # A35 (ESP ab Paket 2026-10-11): ein Setter vom STM wurde still nicht uebernommen
          # (Index/Wert ausser Bereich oder unbekannter Buchstabe) -- meist Versionsversatz.
          printf '  %s  Bruecke: Zuweisung im ESP nicht uebernommen   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *"- ota: Rueckfall-Neustart"*)
          # E.6: Nach einem lokalen ESP-Upload kam der Neustartaufruf der PWA nicht, der ESP startet selbst neu
          printf '  %s  ESP: Rueckfall-Neustart nach lokalem Upload   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *"- ntp verworfen #"*)
          # C58: NTP-Antwort verworfen (Laenge, Absender, Port, LI=3, Stratum 0, Transmit 0)
          printf '  %s  Netzzeit: Antwort im ESP verworfen   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *"icon: verworfen, display aus"*)
          # A80 (Paket 2026-10-13): ein Icon kam bei ausgeschaltetem Display und wurde verworfen,
          # statt bis zum Einschalten vorgemerkt zu bleiben. Erwartet im Nachtfenster -- Hinweis.
          printf '  %s  Hinweis: Icon verworfen, Display aus   <%s>\n' "$stamp" "$z" ;;
        *"icon_freeze: ENTER"*)
          # A80: seit STM-Release S2 darf ein Icon nicht mehr auf das Einschalten warten (L19, L422).
          printf '  %s  *** Icon wartet auf das Display (icon_freeze) *** -- Rueckfall von A80?   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *"- ovr #"*" t="*)
          # A76 (L408): Zeitpunkt, zu dem der ESP einen UART-Ueberlauf ERKANNT hat (ms seit
          # ESP-Start, gedrosselt 1/min). t kurz nach dem Start heisst: waehrend setup().
          # Der Alarm kommt ueber den ovr=-Anstieg der Heap-Zeile; hier nur der Zeitpunkt.
          t=$(printf '%s' "$z" | sed -E 's/.* t=([0-9]+).*/\1/')
          printf '  %s  ESP: UART-Ueberlauf erkannt %s ms nach ESP-Start   <%s>\n' "$stamp" "$t" "$z" ;;
        *"- heap free="*" ovr="*)
          # A21: ovr= zaehlt loop()-Durchlaeufe mit UART-Ueberlauf im ESP, laufend -- gemeldet
          # wird der ANSTIEG, nicht jeder Stand.
          ovr=$(printf '%s' "$z" | sed -E 's/.* ovr=([0-9]+).*/\1/')
          if [ -n "$letzte_ovr" ] && [ "$ovr" -gt "$letzte_ovr" ] 2>/dev/null; then
            printf '  %s  ESP: %d UART-Ueberlauf/-laeufe (ovr=%s)   <%s>\n' "$stamp" "$((ovr - letzte_ovr))" "$ovr" "$z"
            alarm=1
          elif [ -z "$letzte_ovr" ] && [ "$ovr" -gt 0 ] 2>/dev/null; then
            printf '  %s  ESP: UART-Ueberlaeufe seit dem ESP-Start (ovr=%s)   <%s>\n' "$stamp" "$ovr" "$z"
            alarm=1
          fi
          letzte_ovr=$ovr ;;
        *"i2c: frist abgelaufen"*)
          # A60 (Paket 2026-10-11): ein I2C-Zugriff (RTC/EEPROM) hat seine Frist von 5 ms
          # ueberschritten, der Bus wurde zurueckgesetzt. Vorher waere das ein Haenger gewesen.
          printf '  %s  I2C: Frist abgelaufen   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *"time: ungueltig"*)
          printf '  %s  Netzzeit: verworfen   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *"time: sprung"*)
          sprung_folge=$((sprung_folge + 1))
          if [ "$sprung_folge" -gt 3 ]; then
            printf '  %s  Netzzeit: %d Spruenge in Folge ohne Bestaetigung   <%s>\n' "$stamp" "$sprung_folge" "$z"
            alarm=1
          else
            printf '  %s  Hinweis: Netzzeit-Bestaetigung angefordert (%d in Folge)   <%s>\n' "$stamp" "$sprung_folge" "$z"
          fi ;;
        *"cmd: set time to"*)
          sprung_folge=0 ;;
        *"time: zeitzone ungueltig"*)
          printf '  %s  Hinweis: Zeitzone im EEPROM ungueltig, Vorgabe gilt   <%s>\n' "$stamp" "$z" ;;
        *"night: start aus"*)
          printf '  %s  Hinweis: Start im Nacht-Aus-Fenster   <%s>\n' "$stamp" "$z" ;;
        *"night: sprung "*)
          # A69: Uhr sprang ueber eine Timer-Minute (Sommerzeit, NTP-Korrektur, Stillstand), Ereignis nachgeholt
          printf '  %s  Hinweis: Timer nach Zeitsprung nachgeholt   <%s>\n' "$stamp" "$z" ;;
        *"spiel: "*" Zeilen verworfen"*)
          printf '  %s  Hinweis: Spiel hat ESP-Zeilen verworfen, Abgleich folgt   <%s>\n' "$stamp" "$z" ;;
        *"ldr: "*" abgewiesen"*)
          printf '  %s  Hinweis: LDR-Grenze nicht uebernommen (Reihenfolge)   <%s>\n' "$stamp" "$z" ;;
        *"- weather fc="*)
          # - weather fc=<0|1> ms=<n> <ok|fehler|dns|connfail|timeout|leer> [n=<b> cl=<b|-> oom=<z>]   (design.md §1.4, C54)
          wms=$(printf '%s' "$z" | grep -aoE 'ms=[0-9]+' | head -1 | cut -d= -f2)
          werg=$(printf '%s' "$z" | sed -E 's/.*- weather fc=[0-9]+ ms=[0-9]+ ([a-z]+).*/\1/')
          # Ab ESP 3.2.29 haengen n=<koerper> cl=<content-length|-> oom=<verlorene zeichen> an (C54).
          # "ok" allein reicht dann nicht: "Wetter morgen: 15 Grad, " ohne Beschreibung meldet ok,
          # obwohl der Koerper am Ende fehlte (L357). Ein Verlust beim Anhaengen (oom > 0) oder ein
          # Koerper kuerzer als angekuendigt (n != cl) ist deshalb ein eigener Alarm.
          woom=$(printf '%s' "$z" | grep -aoE 'oom=[0-9]+' | head -1 | cut -d= -f2)
          wn=$(printf '%s' "$z" | grep -aoE ' n=[0-9]+' | head -1 | cut -d= -f2)
          wcl=$(printf '%s' "$z" | grep -aoE 'cl=[0-9]+' | head -1 | cut -d= -f2)
          if [ "$werg" != "ok" ] || [ "${wms:-0}" -ge 1000 ]; then
            printf '  %s  Wetter: %s nach %s ms   <%s>\n' "$stamp" "$werg" "${wms:-?}" "$z"
            alarm=1
          elif [ "${woom:-0}" -gt 0 ] || { [ -n "$wn" ] && [ -n "$wcl" ] && [ "$wn" != "$wcl" ]; }; then
            printf '  %s  Wetter: Koerper unvollstaendig (n=%s cl=%s oom=%s)   <%s>\n' "$stamp" "${wn:-?}" "${wcl:--}" "${woom:-0}" "$z"
            alarm=1
          fi ;;
        *"ds18xx verloren"*)
          # STM ab Paket 2026-10-10 (A2): drei Lese-/CRC-Fehler in Folge, Sensor gilt als nicht erkannt
          printf '  %s  DS18xx: Sensor verloren   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *"DS18xxx temperature: ungueltig"*)
          # Index 255: Lese- oder CRC-Fehler -- oder unter 0 Grad (Grenze, benannt; zaehlt nicht als Verlust)
          printf '  %s  DS18xx: Messwert ungueltig   <%s>\n' "$stamp" "$z"
          alarm=1 ;;
        *"ds18xx erkannt"*)
          printf '  %s  Hinweis: DS18xx (wieder) erkannt   <%s>\n' "$stamp" "$z" ;;
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

  # Zweite Zahl von v=: verschachtelte var_send-Aufrufe (ein Kommando, das waehrend des
  # Wartens auf eine Quittung ein weiteres ausloest). Bis 11.10.2026 nur in der Ruhezeile,
  # nie ausgewertet -- vom Analysten des Pakets 2026-10-11 bemerkt, nicht von der Wache.
  vn=${v#*/}
  if [ -n "$v" ] && [ -n "$vn" ] && [ -n "$letzte_vn" ] && [ "$vn" -gt "$letzte_vn" ] 2>/dev/null; then
    printf '  %s  Bruecke: %d verschachtelte var_send-Aufruf(e) (v=%s)\n' "$stamp" "$((vn - letzte_vn))" "${v#v=}"
    alarm=1
  fi
  [ -n "$v" ] && [ -n "$vn" ] && letzte_vn=$vn

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
