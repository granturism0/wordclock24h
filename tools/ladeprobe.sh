#!/usr/bin/env bash
# Ladeprobe (Paket 2026-10-15, design.md §5.1, C70): PWA einmal kalt, dann warm laden und je
# Laden Asset-Verkehr und Heap-Tiefstwert des ESP messen. Logik in tools/ladeprobe.mjs.
#
#   ./tools/ladeprobe.sh                   gegen DEVICE_HOST, 1 kalt + 5 warm, deutsch
#   ./tools/ladeprobe.sh --englisch        Sprache nach dem kalten Laden auf Englisch (wie K33)
#   ./tools/ladeprobe.sh --erwarte-304     ein warmes Laden mit 200 auf ein App-Asset ist FEHL
#   LADEPROBE_HOST=127.0.0.1:8099 ...      gegen Vorschau oder Pruefserver (Gegenprobe, DIR-014)
#   LADEPROBE_WARM=n                       Zahl der warmen Ladungen
#   LADEPROBE_AUSNAHME=/app/pfad           von --erwarte-304 ausgenommen
#   LADEPROBE_MITSCHRIFT=datei             jede Zeile zusaetzlich dorthin
#
# Schreibt am Geraet nur den Ruecksetzer der Tiefstwert-Messung (heap_low_reset=1).
# Exit 0 ohne Fehlschlag, 1 mit Fehlschlag, 2 Werkzeugfehler.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

[ -f tools/device.conf ] && . tools/device.conf
HOST=${LADEPROBE_HOST:-${DEVICE_HOST:-}}
[ -n "$HOST" ] || { echo "DEVICE_HOST fehlt (tools/device.conf)." >&2; exit 2; }
if [ -z "${LADEPROBE_HOST:-}" ]; then
  touch "$(git rev-parse --git-dir)/geraet-gemessen" 2>/dev/null || true
fi

ERWARTE_304=0; ENGLISCH=0
for a in "$@"; do
  case "$a" in
    --erwarte-304) ERWARTE_304=1 ;;
    --englisch) ENGLISCH=1 ;;
    *) echo "Unbekannte Option: $a" >&2; exit 2 ;;
  esac
done

CHROME=${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}
[ -x "$CHROME" ] || { echo "Chrome nicht gefunden: $CHROME" >&2; exit 2; }

WORK=$(mktemp -d)
CDP_PORT=$(python3 -c 'import socket; s=socket.socket(); s.bind(("127.0.0.1",0)); print(s.getsockname()[1]); s.close()')
START_MARKE="data:text/html,ladeprobe-$$-$RANDOM"
# Profil frisch je Lauf (kaltes erstes Laden), dauerhaft innerhalb des Laufs (warme Ladungen).
"$CHROME" --headless=new --disable-gpu --no-sandbox \
  --user-data-dir="$WORK/profil" --remote-debugging-port="$CDP_PORT" \
  "$START_MARKE" >"$WORK/chrome.log" 2>&1 &
CHROME_PID=$!
for _ in $(seq 1 20); do
  curl -s -m 2 "http://127.0.0.1:$CDP_PORT/json/version" >/dev/null 2>&1 && break
  sleep 1
done
if ! curl -s -m 2 "http://127.0.0.1:$CDP_PORT/json/version" >/dev/null 2>&1; then
  echo "ABBRUCH: Chrome hat den Debug-Port nicht geoeffnet." >&2
  kill "$CHROME_PID" 2>/dev/null; rm -rf "$WORK"; exit 2
fi

CDP_PORT="$CDP_PORT" GERAET_URL="http://$HOST/app/" START_MARKE="$START_MARKE" \
  ERWARTE_304="$ERWARTE_304" ENGLISCH="$ENGLISCH" node tools/ladeprobe.mjs
RC=$?
kill "$CHROME_PID" 2>/dev/null; wait "$CHROME_PID" 2>/dev/null
rm -rf "$WORK"
exit $RC
