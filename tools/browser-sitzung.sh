#!/usr/bin/env bash
# Browser-Sitzung (Paket 2026-10-15, design.md §5.3, C72): Ein echter Browser auf dem Mac
# oeffnet die PWA, verbirgt sie, schliesst den Tab und wird beendet. Am Ende jeder Phase
# liest das Skript die Uhr (/api/device_ready) und die Mac-Seite (netstat, lsof).
#
#   ./tools/browser-sitzung.sh safari
#   ./tools/browser-sitzung.sh chrome
#   BROWSER_SITZUNG_HOST=<host>         andere Adresse (Gegenprobe: 192.0.2.1 ist nie erreichbar)
#   BROWSER_SITZUNG_DAUER="180 120 180 300 180"   Dauer der Phasen S0..S4 in Sekunden
#   BROWSER_SITZUNG_MITSCHRIFT=datei    jede Zeile zusaetzlich dorthin (Vorgabe: Scratch-Datei)
#
# WARUM ES DAS GIBT (L461)
#
# Nach einer kurzen Safari-Sitzung blieb der Heap des ESP minutenlang 1,7 KB tiefer -- bis der
# Nutzer Safari BEENDETE. Welcher Verbindungszustand den Speicher haelt, ist nicht belegt. Diese
# Sitzung stellt das Bild kontrolliert nach, Phase fuer Phase, mit derselben Lesung am Ende.
#
#   S0  Ruhe, kein Browser                         Grundwerte
#   S1  Browser oeffnet /app/, Seite sichtbar
#   S2  neuer Tab about:blank vorn, PWA verborgen   (> 120 s TIME_WAIT)
#   S3  PWA-Tab geschlossen, Browser laeuft weiter  (> FIN_WAIT_2, > TIME_WAIT)
#   S4  Browser beendet
#
# Zwischen den Lesungen liest das Skript NICHT: Jede Lesung ist selbst eine Verbindung. Die
# Minutenreihe liefert die Heap-Zeile im Mitschnitt.
#
# VORBEDINGUNG: Der Browser laeuft nicht. Laeuft er, bricht das Skript ab -- es beendet nie einen
# Browser, den der Nutzer offen hat. macOS fragt beim ersten Mal nach der Erlaubnis zur
# Automation; die muss der Nutzer einmal geben.
#
# Exit 0 Sitzung vollstaendig, 1 eine Phase gescheitert (Bedienung oder Lesung), 2 Vorbedingung.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

BROWSER=${1:-}
case "$BROWSER" in
  safari) APP="Safari"; PROZ="Safari" ;;
  chrome) APP="Google Chrome"; PROZ="Google Chrome" ;;
  *) echo "Aufruf: $0 safari|chrome" >&2; exit 2 ;;
esac

[ -f tools/device.conf ] && . tools/device.conf
HOST=${BROWSER_SITZUNG_HOST:-${DEVICE_HOST:-}}
[ -n "$HOST" ] || { echo "DEVICE_HOST fehlt (tools/device.conf)." >&2; exit 2; }
URL="http://$HOST/app/"
read -r D0 D1 D2 D3 D4 <<<"${BROWSER_SITZUNG_DAUER:-180 120 180 300 180}"
MIT=${BROWSER_SITZUNG_MITSCHRIFT:-$(mktemp -t browser-sitzung).txt}
FEHL=0

aus() { printf '%s\n' "$*"; printf '%s\n' "$*" >> "$MIT"; }

lesung() {   # $1 = Phase
  local p=$1 dr
  aus "--- $(date +%H:%M:%S) Lesung Ende $p"
  if ! dr=$(curl -s -m 10 "http://$HOST/api/device_ready"); then
    aus "  device_ready: NICHT ERREICHBAR"; FEHL=$((FEHL+1)); return 1
  fi
  printf '%s' "$dr" | python3 -c '
import json,sys
try: d=json.load(sys.stdin)
except Exception as e: print("  device_ready: keine JSON-Antwort"); sys.exit(1)
k=[x for x in d if x in ("free_heap","max_free_block","http_backlog","heap_low_block","heap_low_path","heap_low_pcb_states","heap_low_tw") or x.startswith("tcp_now")]
for x in k: print("  %s=%s" % (x, json.dumps(d[x])))
' | while IFS= read -r z; do aus "$z"; done
  [ "${PIPESTATUS[0]}" -eq 0 ] || FEHL=$((FEHL+1))
  aus "  netstat (Mac -> Uhr):"
  netstat -anv -p tcp 2>/dev/null | awk -v h="$HOST" '$5 ~ "^"h"\\." {print "    " $1, $4, $5, $6}' | while IFS= read -r z; do aus "$z"; done
  aus "  lsof (Prozess je Verbindung):"
  lsof -nP -i "@$HOST" 2>/dev/null | awk 'NR>1 {print "    " $1, $2, $9, $10}' | while IFS= read -r z; do aus "$z"; done
}

warte() { aus "    ($1 s)"; sleep "$1"; }

osa() {   # AppleScript ausfuehren; Fehlschlag zaehlt
  if ! osascript -e "$1" >/dev/null 2>>"$MIT"; then
    aus "  BEDIENUNG FEHLGESCHLAGEN: $2"; FEHL=$((FEHL+1)); return 1
  fi
}

# ------------------------------------------------------------------ Vorbedingungen
if ! curl -s -m 5 -o /dev/null "http://$HOST/api/device_ready"; then
  echo "ABBRUCH: Uhr unter $HOST nicht erreichbar." >&2; exit 2
fi
if pgrep -xq "$PROZ"; then
  echo "ABBRUCH: $APP laeuft bereits. Das Skript beendet keinen Browser, den der Nutzer offen hat -- bitte zuerst selbst beenden." >&2
  exit 2
fi
touch "$(git rev-parse --git-dir)/geraet-gemessen" 2>/dev/null || true

aus "=== Browser-Sitzung $APP gegen $URL — $(date '+%Y-%m-%d %H:%M:%S') — Dauer S0..S4: $D0 $D1 $D2 $D3 $D4 s ==="
aus "    Mitschrift: $MIT"

aus "S0 Ruhe, kein Browser"; warte "$D0"; lesung S0

aus "S1 $(date +%H:%M:%S) $APP oeffnet die PWA"
open -a "$APP" "$URL" || { aus "  BEDIENUNG FEHLGESCHLAGEN: open"; FEHL=$((FEHL+1)); }
warte "$D1"; lesung S1

aus "S2 $(date +%H:%M:%S) neuer Tab about:blank vorn, PWA verborgen"
if [ "$BROWSER" = safari ]; then
  osa 'tell application "Safari" to tell front window to set current tab to (make new tab with properties {URL:"about:blank"})' "neuer Tab"
else
  osa 'tell application "Google Chrome" to tell front window to make new tab with properties {URL:"about:blank"}' "neuer Tab"
fi
warte "$D2"; lesung S2

aus "S3 $(date +%H:%M:%S) PWA-Tab geschlossen, Browser laeuft weiter"
if [ "$BROWSER" = safari ]; then
  osa "tell application \"Safari\" to close (every tab of every window whose URL contains \"$HOST\")" "Tab schliessen"
else
  osa "tell application \"Google Chrome\" to close (every tab of every window whose URL contains \"$HOST\")" "Tab schliessen"
fi
warte "$D3"; lesung S3

aus "S4 $(date +%H:%M:%S) $APP beendet"
osa "tell application \"$APP\" to quit" "beenden"
warte "$D4"; lesung S4

if pgrep -xq "$PROZ"; then aus "  WARNUNG: $APP laeuft nach S4 noch"; FEHL=$((FEHL+1)); fi
aus "=== Ende $(date +%H:%M:%S): ${FEHL} Fehlschlag/-schlaege ==="
[ "$FEHL" -eq 0 ] && exit 0 || exit 1
