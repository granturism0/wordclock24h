#!/usr/bin/env bash
# Browser-Sitzung (Paket 2026-10-15, design.md §5.3, C72): Ein echter Browser oeffnet die PWA,
# verbirgt sie, schliesst den Tab und wird beendet. Am Ende jeder Phase liest das Skript die Uhr
# (/api/device_ready) und die Mac-Seite (netstat, lsof).
#
#   ./tools/browser-sitzung.sh safari      Safari auf diesem Mac, per open/osascript bedient
#   ./tools/browser-sitzung.sh chrome      Chrome auf diesem Mac, mit EIGENEM Wegwerfprofil ueber die
#                                          DevTools-Schnittstelle bedient -- nie das Profil des Nutzers
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
  [ "$ART" = chrome ] && { chrome_weg; aus "    eigenes Chrome beendet, Wegwerfprofil geloescht"; exit 2; }
  [ -n "$PROZ" ] && pgrep -xq "$PROZ" && aus "    $APP laeuft noch -- bei Bedarf selbst beenden"
  exit 2
}
trap abbruch INT TERM HUP

# Chrome (16.10.2026): Bis dahin steuerte das Skript das Chrome des NUTZERS per AppleScript und beendete
# es mit quit. Am selben Tag hat genau dieses Muster bei Safari die gespeicherte Sitzung des Nutzers
# (rund 40 Tabs) zerstoert -- ein Browser, der beendet wird, ueberschreibt seine letzte Sitzung. Chrome
# laeuft deshalb wie in tools/ladeprobe.sh mit einem eigenen Profil aus mktemp, sichtbar (ein verborgener
# Tab soll sich wie im Alltag verhalten), und beendet wird nur DIESER Prozess.
CHROME=${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}
CDP_PORT=${BROWSER_SITZUNG_CDP_PORT:-9233}
WORK=""; CHROME_PID=""; PROFIL=""
chrome_weg() {
  [ -n "$CHROME_PID" ] && kill "$CHROME_PID" 2>/dev/null && wait "$CHROME_PID" 2>/dev/null
  CHROME_PID=""
  case "$WORK" in "${TMPDIR:-/tmp}"*browser-sitzung.*) rm -rf "${WORK:?}" ;; esac
  WORK=""
}
trap '[ "$ART" = chrome ] && chrome_weg' EXIT     # jeder Ausgang raeumt das eigene Chrome weg
chrome_start() {   # $1 URL; legt das Wegwerfprofil an, falls es noch keins gibt (Vorwaermen nutzt dasselbe)
  [ -x "$CHROME" ] || { aus "ABBRUCH: Chrome nicht gefunden: $CHROME"; exit 2; }
  [ -n "$WORK" ] || WORK=$(mktemp -d "${TMPDIR:-/tmp}/browser-sitzung.XXXXXX") || exit 2
  PROFIL="$WORK/profil"
  "$CHROME" --user-data-dir="$WORK/profil" --remote-debugging-port="$CDP_PORT" --no-first-run \
    --no-default-browser-check --disable-sync --new-window "$1" > "$WORK/chrome.log" 2>&1 &
  CHROME_PID=$!
  aus "    eigenes Chrome PID $CHROME_PID, Profil $WORK/profil, DevTools 127.0.0.1:$CDP_PORT"
}
chrome_halt() { [ -n "$CHROME_PID" ] && kill "$CHROME_PID" 2>/dev/null && wait "$CHROME_PID" 2>/dev/null; CHROME_PID=""; }   # Profil bleibt
cdp() { curl -s -m 5 "$@"; }
cdp_tabs() {   # Zahl der Seiten im eigenen Chrome, deren URL die Uhr enthaelt
  cdp "http://127.0.0.1:$CDP_PORT/json/list" | python3 -c "import json,sys; h=sys.argv[1]; print(sum(1 for t in json.load(sys.stdin) if t.get('type')=='page' and ('//'+h+'/') in t.get('url','')))" "$HOST" 2>/dev/null
}
cdp_ids() {    # Kennungen dieser Seiten
  cdp "http://127.0.0.1:$CDP_PORT/json/list" | python3 -c "import json,sys; h=sys.argv[1]; [print(t['id']) for t in json.load(sys.stdin) if t.get('type')=='page' and ('//'+h+'/') in t.get('url','')]" "$HOST" 2>/dev/null
}

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
k=[x for x in d if x in ("free_heap","max_free_block","http_backlog","heap_low_block","heap_low_path","heap_low_ms","heap_low_pcb_states","heap_low_tw","no_request_timeouts","no_request_aborts") or x.startswith("tcp_now") or x in ("app_full","app_not_modified")]
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
  [ "$ART" = chrome ] && { cdp_tabs; return; }
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
KOPFLOS=""
if [ "$ART" = chrome ]; then
  # Ein SICHTBARES Chrome ist das des Nutzers -- abbrechen. Kopflose Chrome sind Pruefstaende mit Wegwerfprofil
  # (c65q, check-pwa, Ladeprobe); sie werden vermerkt, denn sie koennten die Uhr ansprechen.
  for pid in $(pgrep -x "Google Chrome"); do
    if ps -o command= -p "$pid" | grep -q -- "--headless"; then KOPFLOS="$KOPFLOS $pid"
    else echo "ABBRUCH: Google Chrome laeuft bereits sichtbar (PID $pid). Das Skript beendet keinen Browser des Nutzers -- bitte zuerst selbst beenden." >&2; exit 2; fi
  done
elif [ -n "$PROZ" ] && pgrep -xq "$PROZ"; then
  echo "ABBRUCH: $APP laeuft bereits. Das Skript beendet keinen Browser, den der Nutzer offen hat -- bitte zuerst selbst beenden." >&2
  exit 2
fi
touch "$(git rev-parse --git-dir)/geraet-gemessen" 2>/dev/null || true

aus "=== Browser-Sitzung $APP gegen $URL — $(date '+%Y-%m-%d %H:%M:%S') — Dauer S0..S4: $D0 $D1 $D2 $D3 $D4 s ==="
aus "    Mitschrift: $MIT"
[ -n "$KOPFLOS" ] && aus "    WARNUNG: kopfloses Chrome laeuft (PID$KOPFLOS) -- Pruefstand? Spricht es die Uhr an, ist die Messung nicht exklusiv"

# G10 (AKW.4): Vorwaermen -- der Browser oeffnet die PWA einmal und wird beendet, damit S1 mit gefuelltem Cache
# beginnt und nachfragen kann (304). Chrome behaelt dabei sein Wegwerfprofil; Safari wird nur per quit beendet.
if [ -n "${BROWSER_SITZUNG_VORWAERMEN:-}" ] && [ "$ART" != manuell ]; then
  PHASE=V; aus "V  $(date +%H:%M:%S) Vorwaermen: $APP oeffnet die PWA einmal und wird beendet"
  if [ "$ART" = chrome ]; then chrome_start "$URL"; sleep 20; chrome_halt
  else open -a "$APP" "$URL" || { aus "  BEDIENUNG FEHLGESCHLAGEN: open (Vorwaermen)"; FEHL=$((FEHL+1)); }
       sleep 20; osa "tell application \"$APP\" to quit" "beenden (Vorwaermen)"; sleep 5
  fi
  if [ "$ART" = chrome ]; then pgrep -f -- "--user-data-dir=$PROFIL" >/dev/null && { aus "  BEDIENUNG FEHLGESCHLAGEN: eigenes Chrome laeuft nach dem Vorwaermen noch"; FEHL=$((FEHL+1)); }
  else pgrep -xq "$PROZ" && { aus "  BEDIENUNG FEHLGESCHLAGEN: $APP laeuft nach dem Vorwaermen noch"; FEHL=$((FEHL+1)); }; fi
fi

PHASE=S0; aus "S0 Ruhe, kein Browser"; warte "$D0"; lesung S0

PHASE=S1; aus "S1 $(date +%H:%M:%S) $APP oeffnet die PWA"
if [ "$ART" = manuell ]; then
  zuruf "Oeffne auf deinem Geraet $URL und lass die Seite sichtbar."
else
  # Unmittelbar vor dem Oeffnen nochmals: Hat der Nutzer in S0 selbst den Browser gestartet,
  # wuerde open einen Tab in SEINEM Browser oeffnen und S4 ihn beenden (Review S1).
  # Chrome laeuft mit eigenem Profil und eigenem Prozess -- ein fremdes Chrome (Pruefstand) beruehrt die Bedienung nicht.
  if [ "$ART" != chrome ] && pgrep -xq "$PROZ"; then aus "ABBRUCH vor S1: $APP wurde inzwischen gestartet -- nicht meiner."; exit 2; fi
  if [ "$ART" = chrome ]; then
    chrome_start "$URL"
  else
    open -a "$APP" "$URL" || { aus "  BEDIENUNG FEHLGESCHLAGEN: open"; FEHL=$((FEHL+1)); }
  fi
  sleep 8
  n=$(tabs_mit_host); aus "    Tabs mit der Uhr: ${n:-?}"
  [ "${n:-0}" -ge 1 ] || { aus "  BEDIENUNG FEHLGESCHLAGEN: PWA-Tab nicht gefunden (Willkommensdialog?)"; FEHL=$((FEHL+1)); }
fi
warte "$D1"; lesung S1

PHASE=S2; aus "S2 $(date +%H:%M:%S) neuer Tab about:blank vorn, PWA verborgen"
case "$ART" in
  manuell) zuruf "Oeffne einen neuen, leeren Tab im Vordergrund. Die Uhr-Seite bleibt offen, aber verborgen." ;;
  safari)  osa 'tell application "Safari" to tell front window to set current tab to (make new tab with properties {URL:"about:blank"})' "neuer Tab" ;;
  chrome)
    neu=$(cdp -X PUT "http://127.0.0.1:$CDP_PORT/json/new?about:blank" | python3 -c "import json,sys; print(json.load(sys.stdin)['id'])" 2>/dev/null)
    if [ -n "$neu" ] && cdp "http://127.0.0.1:$CDP_PORT/json/activate/$neu" >/dev/null; then aus "    neuer Tab $neu vorn"
    else aus "  BEDIENUNG FEHLGESCHLAGEN: neuer Tab (DevTools)"; FEHL=$((FEHL+1)); fi ;;
esac
warte "$D2"; lesung S2

PHASE=S3; aus "S3 $(date +%H:%M:%S) PWA-Tab geschlossen, Browser laeuft weiter"
if [ "$ART" = manuell ]; then
  zuruf "Schliesse den Tab mit der Uhr. Den Browser NICHT beenden."
else
  if [ "$ART" = chrome ]; then
    for id in $(cdp_ids); do cdp "http://127.0.0.1:$CDP_PORT/json/close/$id" >/dev/null || { aus "  BEDIENUNG FEHLGESCHLAGEN: Tab $id schliessen"; FEHL=$((FEHL+1)); }; done
  else
    osa "tell application \"$APP\" to close (every tab of every window whose URL contains \"$HOST\")" "Tab schliessen"
  fi
  n=$(tabs_mit_host); aus "    Tabs mit der Uhr danach: ${n:-?}"
  [ "${n:-1}" -eq 0 ] || { aus "  BEDIENUNG FEHLGESCHLAGEN: PWA-Tab noch offen"; FEHL=$((FEHL+1)); }
fi
warte "$D3"; lesung S3

PHASE=S4; aus "S4 $(date +%H:%M:%S) $APP beendet"
if [ "$ART" = manuell ]; then
  zuruf "Beende den Browser ganz (auf dem iPhone: aus der App-Uebersicht wischen)."
else
  if [ "$ART" = chrome ]; then chrome_weg; aus "    eigenes Chrome beendet, Wegwerfprofil geloescht"
  else osa "tell application \"$APP\" to quit" "beenden"; fi
fi
warte "$D4"; lesung S4

# Chrome: nur das EIGENE Profil zaehlt -- Pruefstaende (c65q, check-pwa, Ladeprobe) starten eigene kopflose Chrome.
if [ "$ART" = chrome ]; then
  [ -n "$PROFIL" ] && pgrep -f -- "--user-data-dir=$PROFIL" >/dev/null && { aus "  WARNUNG: eigenes Chrome laeuft nach S4 noch"; FEHL=$((FEHL+1)); }
elif [ -n "$PROZ" ] && pgrep -xq "$PROZ"; then aus "  WARNUNG: $APP laeuft nach S4 noch"; FEHL=$((FEHL+1)); fi
aus "--- Zusammenfassung je Phase (Lesung am Phasenende; Δ gegen S0):$ZEILE_PHASE"
aus "    M2 setzt voraus, dass Δtimeouts den Anstieg traegt; Δtimeouts ~ 0 => K1 fuer L461 wirkungslos (A.3)."
aus "=== Ende $(date +%H:%M:%S): ${FEHL} Fehlschlag/-schlaege ==="
[ "$FEHL" -eq 0 ] && exit 0 || exit 1
