#!/usr/bin/env bash
# Browser-Sitzung (Paket 2026-10-15, design.md §5.3, C72): Ein echter Browser oeffnet die PWA,
# verbirgt sie, schliesst den Tab und wird beendet. Am Ende jeder Phase liest das Skript die Uhr
# (/api/device_ready) und die Mac-Seite (netstat, lsof).
#
#   ./tools/browser-sitzung.sh safari      Safari auf diesem Mac, per open/osascript bedient
#   ./tools/browser-sitzung.sh chrome      Chrome auf diesem Mac
#   ./tools/browser-sitzung.sh manuell     ein Geraet des Nutzers (iPhone ...): das Skript nennt
#                                          jede Phase, der Nutzer bedient, das Skript liest (G9.3)
#   BROWSER_SITZUNG_HOST=<host>            andere Adresse (Gegenprobe: 192.0.2.1 ist nie erreichbar)
#   BROWSER_SITZUNG_DAUER="180 120 180 300 180"   Dauer der Phasen S0..S4 in Sekunden
#   BROWSER_SITZUNG_MITSCHRIFT=datei       absoluter Pfad; Vorgabe: Datei im Scratchpad-/TMP-Ordner
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
# Zwischen den Lesungen liest das Skript NICHT: Jede Lesung ist selbst eine Verbindung und hinter-
# laesst auf der Uhr ein TIME_WAIT (~120 s). Das Skript nennt deshalb den lokalen Port jeder Lesung
# und bis wann ihr TIME_WAIT reicht -- sonst haelt man es fuer eine Verbindung des Browsers
# (Review 0.5, M5). Die Minutenreihe liefert die Heap-Zeile im Mitschnitt.
#
# NICHT REIN LESEND: Beim Oeffnen kann die PWA selbst schreiben (ambilight_online, L462).
#
# VORBEDINGUNG: Der Browser laeuft nicht -- geprueft am Anfang UND unmittelbar vor dem Oeffnen.
# Das Skript beendet nie einen Browser, den der Nutzer offen hat. macOS fragt beim ersten Mal nach
# der Erlaubnis zur Automation; die muss der Nutzer einmal geben.
#
# Exit 0 Sitzung vollstaendig, 1 eine Phase gescheitert (Bedienung oder Lesung), 2 Vorbedingung.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

ART=${1:-}
case "$ART" in
  safari)  APP="Safari"; PROZ="Safari" ;;
  chrome)  APP="Google Chrome"; PROZ="Google Chrome" ;;
  manuell) APP="Geraet des Nutzers"; PROZ="" ;;
  *) echo "Aufruf: $0 safari|chrome|manuell" >&2; exit 2 ;;
esac

[ -f tools/device.conf ] && . tools/device.conf
HOST=${BROWSER_SITZUNG_HOST:-${DEVICE_HOST:-}}
[ -n "$HOST" ] || { echo "DEVICE_HOST fehlt (tools/device.conf)." >&2; exit 2; }
URL="http://$HOST/app/"
DAUER=${BROWSER_SITZUNG_DAUER:-180 120 180 300 180}
read -r D0 D1 D2 D3 D4 REST <<<"$DAUER"
for d in "$D0" "$D1" "$D2" "$D3" "$D4"; do
  case "$d" in ''|*[!0-9]*) echo "BROWSER_SITZUNG_DAUER braucht fuenf ganze Zahlen: '$DAUER'" >&2; exit 2 ;; esac
done
[ -z "${REST:-}" ] || { echo "BROWSER_SITZUNG_DAUER hat mehr als fuenf Werte: '$DAUER'" >&2; exit 2; }

MIT=${BROWSER_SITZUNG_MITSCHRIFT:-${TMPDIR:-/tmp}/browser-sitzung-$ART-$(date +%Y%m%d-%H%M%S).txt}
case "$MIT" in /*) ;; *) echo "BROWSER_SITZUNG_MITSCHRIFT muss ein absoluter Pfad sein" >&2; exit 2 ;; esac
FEHL=0
PHASE="Start"
ESP_ZULETZT=""   # "Port bis hh:mm:ss" der letzten eigenen Lesung
ZEILE_PHASE=""   # Zusammenfassung je Phase

aus() { printf '%s\n' "$*"; printf '%s\n' "$*" >> "$MIT"; }

# Abbruch (Strg-C, Signal): sagen, in welcher Phase, und dass der Browser noch laufen kann (Review S9).
abbruch() {
  aus "=== ABGEBROCHEN in $PHASE um $(date +%H:%M:%S) ==="
  [ -n "$PROZ" ] && pgrep -xq "$PROZ" && aus "    $APP laeuft noch -- bei Bedarf selbst beenden"
  exit 2
}
trap abbruch INT TERM HUP

lesung() {   # $1 = Phase
  local p=$1 dr port rc n
  aus "--- $(date +%H:%M:%S) Lesung Ende $p"
  [ -n "$ESP_ZULETZT" ] && aus "  (TIME_WAIT der vorigen eigenen Lesung: $ESP_ZULETZT)"
  # -w: lokaler Port dieser Lesung, damit ihr TIME_WAIT in der naechsten Lesung erkennbar ist.
  dr=$(curl -s -m 10 -w '\n%{local_port} %{http_code}' "http://$HOST/api/device_ready"); rc=$?
  port=$(printf '%s' "$dr" | tail -1)
  dr=$(printf '%s' "$dr" | sed '$d')
  if [ "$rc" -ne 0 ] || [ "${port#* }" != "200" ]; then
    aus "  device_ready: NICHT ERREICHBAR oder kein 200 (curl $rc, http ${port#* })"; FEHL=$((FEHL+1))
  else
    ESP_ZULETZT="Port ${port%% *} bis $(date -v+120S +%H:%M:%S)"
    aus "  eigene Lesung: lokaler Port ${port%% *} (TIME_WAIT auf der Uhr bis etwa $(date -v+120S +%H:%M:%S))"
    # Python-Fehlschlag muss zaehlen (Review M1): Ergebnis in eine Variable, Exit getrennt pruefen.
    local felder
    felder=$(printf '%s' "$dr" | python3 -c '
import json,sys
try: d=json.load(sys.stdin)
except Exception: print("  device_ready: keine JSON-Antwort"); sys.exit(1)
k=[x for x in d if x in ("free_heap","max_free_block","http_backlog","heap_low_block","heap_low_path","heap_low_ms","heap_low_pcb_states","heap_low_tw","no_request_timeouts","no_request_aborts") or x.startswith("tcp_now")]
for x in k: print("  %s=%s" % (x, json.dumps(d[x])))
if not any(x.startswith("tcp_now") for x in d): print("  HINWEIS: keine tcp_now_*-Felder (Firmware vor R1) -- Entscheidungsregel nicht anwendbar")
g=lambda k: d.get(k, -1)
print("WERTE %s %s %s %s %s" % (g("free_heap"), g("max_free_block"), g("no_request_timeouts"), g("no_request_aborts"), g("http_accept_dead")))
'); rc=$?
    WERTE=$(printf '%s\n' "$felder" | sed -n 's/^WERTE //p')
    felder=$(printf '%s\n' "$felder" | grep -v '^WERTE ')
    aus "$felder"
    # Lochsumme und Deltas gegen S0 (design.md 5.4, AKM.3/AKM.4, A.3/A.4): L = free - max; Δ der drei Zaehler.
    set -- $WERTE
    if [ $# -eq 5 ]; then
      if [ "$p" = S0 ]; then B_FREE=$1; B_TO=$3; B_AB=$4; B_AD=$5; fi
      aus "  Lochsumme L=free-max: $(( $1 - $2 ))   Luecke free(S0)-free: $(( ${B_FREE:-$1} - $1 ))   Δtimeouts=$(( $3 - ${B_TO:-$3} )) Δaborts=$(( $4 - ${B_AB:-$4} )) Δaccept_dead=$(( $5 - ${B_AD:-$5} ))  (gegen S0)"
      ZEILE_PHASE="$ZEILE_PHASE
  $p  free=$1 max=$2 L=$(( $1 - $2 )) Luecke=$(( ${B_FREE:-$1} - $1 )) Δtimeouts=$(( $3 - ${B_TO:-$3} )) Δaborts=$(( $4 - ${B_AB:-$4} )) Δaccept_dead=$(( $5 - ${B_AD:-$5} ))"
    fi
    [ "$rc" -eq 0 ] || FEHL=$((FEHL+1))
  fi
  # Mac-Seite IMMER lesen, auch wenn die Uhr nicht antwortet (Review S4) -- etwa nach einem Neustart.
  local ns ls
  ns=$(netstat -anv -p tcp 2>&1) || { aus "  netstat FEHLGESCHLAGEN"; FEHL=$((FEHL+1)); }
  ns=$(printf '%s\n' "$ns" | awk -v h="$HOST" '$5 ~ "^"h"\\." {print "    " $1, $4, $5, $6}')
  n=$(printf '%s' "$ns" | grep -c . )
  aus "  netstat (Mac -> Uhr): $n Verbindung(en)"
  [ "$n" -gt 0 ] && aus "$ns"
  ls=$(lsof +c 0 -nP -i "@$HOST" 2>/dev/null | awk 'NR>1 {print "    " $1, $2, $9, $10}')
  n=$(printf '%s' "$ls" | grep -c . )
  aus "  lsof (Prozess je Verbindung): $n"
  [ "$n" -gt 0 ] && aus "$ls"
}

warte() { aus "    ($1 s)"; sleep "$1"; }

osa() {   # AppleScript ausfuehren; Fehlschlag zaehlt und erscheint in Konsole UND Mitschrift
  local fehler
  if ! fehler=$(osascript -e "$1" 2>&1 >/dev/null); then
    aus "  BEDIENUNG FEHLGESCHLAGEN: $2 -- $fehler"; FEHL=$((FEHL+1)); return 1
  fi
}

tabs_mit_host() {   # Zahl der Tabs, deren URL die Uhr enthaelt
  osascript -e "tell application \"$APP\" to count (every tab of every window whose URL contains \"$HOST\")" 2>/dev/null \
    | tr -d ' ' | awk -F, '{s=0; for(i=1;i<=NF;i++) s+=$i; print s}'
}

zuruf() {   # manuell: Anweisung an den Nutzer, Bestaetigung per Eingabetaste
  aus ">>> $1"
  if [ -t 0 ]; then read -r -p "    Eingabetaste, sobald erledigt ... " _; else aus "    (keine Konsole -- es wird nicht gewartet)"; fi
  aus "    bestaetigt $(date +%H:%M:%S)"
}

# ------------------------------------------------------------------ Vorbedingungen
if ! curl -s -m 5 -o /dev/null "http://$HOST/api/device_ready"; then
  echo "ABBRUCH: Uhr unter $HOST nicht erreichbar." >&2; exit 2
fi
if [ -n "$PROZ" ] && pgrep -xq "$PROZ"; then
  echo "ABBRUCH: $APP laeuft bereits. Das Skript beendet keinen Browser, den der Nutzer offen hat -- bitte zuerst selbst beenden." >&2
  exit 2
fi
touch "$(git rev-parse --git-dir)/geraet-gemessen" 2>/dev/null || true

aus "=== Browser-Sitzung $APP gegen $URL — $(date '+%Y-%m-%d %H:%M:%S') — Dauer S0..S4: $D0 $D1 $D2 $D3 $D4 s ==="
aus "    Mitschrift: $MIT"

PHASE=S0; aus "S0 Ruhe, kein Browser"; warte "$D0"; lesung S0

PHASE=S1; aus "S1 $(date +%H:%M:%S) $APP oeffnet die PWA"
if [ "$ART" = manuell ]; then
  zuruf "Oeffne auf deinem Geraet $URL und lass die Seite sichtbar."
else
  # Unmittelbar vor dem Oeffnen nochmals: Hat der Nutzer in S0 selbst den Browser gestartet,
  # wuerde open einen Tab in SEINEM Browser oeffnen und S4 ihn beenden (Review S1).
  if pgrep -xq "$PROZ"; then aus "ABBRUCH vor S1: $APP wurde inzwischen gestartet -- nicht meiner."; exit 2; fi
  open -a "$APP" "$URL" || { aus "  BEDIENUNG FEHLGESCHLAGEN: open"; FEHL=$((FEHL+1)); }
  sleep 8
  n=$(tabs_mit_host); aus "    Tabs mit der Uhr: ${n:-?}"
  [ "${n:-0}" -ge 1 ] || { aus "  BEDIENUNG FEHLGESCHLAGEN: PWA-Tab nicht gefunden (Willkommensdialog?)"; FEHL=$((FEHL+1)); }
fi
warte "$D1"; lesung S1

PHASE=S2; aus "S2 $(date +%H:%M:%S) neuer Tab about:blank vorn, PWA verborgen"
case "$ART" in
  manuell) zuruf "Oeffne einen neuen, leeren Tab im Vordergrund. Die Uhr-Seite bleibt offen, aber verborgen." ;;
  safari)  osa 'tell application "Safari" to tell front window to set current tab to (make new tab with properties {URL:"about:blank"})' "neuer Tab" ;;
  chrome)  osa 'tell application "Google Chrome" to tell front window to make new tab with properties {URL:"about:blank"}' "neuer Tab" ;;
esac
warte "$D2"; lesung S2

PHASE=S3; aus "S3 $(date +%H:%M:%S) PWA-Tab geschlossen, Browser laeuft weiter"
if [ "$ART" = manuell ]; then
  zuruf "Schliesse den Tab mit der Uhr. Den Browser NICHT beenden."
else
  osa "tell application \"$APP\" to close (every tab of every window whose URL contains \"$HOST\")" "Tab schliessen"
  n=$(tabs_mit_host); aus "    Tabs mit der Uhr danach: ${n:-?}"
  [ "${n:-1}" -eq 0 ] || { aus "  BEDIENUNG FEHLGESCHLAGEN: PWA-Tab noch offen"; FEHL=$((FEHL+1)); }
fi
warte "$D3"; lesung S3

PHASE=S4; aus "S4 $(date +%H:%M:%S) $APP beendet"
if [ "$ART" = manuell ]; then
  zuruf "Beende den Browser ganz (auf dem iPhone: aus der App-Uebersicht wischen)."
else
  osa "tell application \"$APP\" to quit" "beenden"
fi
warte "$D4"; lesung S4

if [ -n "$PROZ" ] && pgrep -xq "$PROZ"; then aus "  WARNUNG: $APP laeuft nach S4 noch"; FEHL=$((FEHL+1)); fi
aus "--- Zusammenfassung je Phase (Lesung am Phasenende; Δ gegen S0):$ZEILE_PHASE"
aus "    M2 setzt voraus, dass Δtimeouts den Anstieg traegt; Δtimeouts ~ 0 => K1 fuer L461 wirkungslos (A.3)."
aus "=== Ende $(date +%H:%M:%S): ${FEHL} Fehlschlag/-schlaege ==="
[ "$FEHL" -eq 0 ] && exit 0 || exit 1
