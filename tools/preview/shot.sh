#!/usr/bin/env bash
# Rendert die PWA in exakten Viewports und legt PNGs ab.
#
#   ./tools/preview/shot.sh                 Standardsatz an Geräteklassen
#   ./tools/preview/shot.sh 390x844         eine bestimmte Grösse
#   ./tools/preview/shot.sh --diag 390x844  mit eingeblendeter Messung
#
# Voraussetzung: tools/preview/server.py läuft (Port 8099).
#
# Drei Chrome-Eigenheiten sind hier fest verdrahtet, weil sie sonst jedes Mal
# Zeit kosten:
#   1. --window-size setzt im Headless-Modus nur die BILDgrösse, nicht das
#      Layout-Viewport. Deshalb läuft alles über /frame?w=…&h=… mit iframe.
#   2. Chrome beendet sich nach --screenshot nicht zuverlässig. Deshalb wird
#      der Prozess nach KILL_AFTER Sekunden beendet; die Datei ist dann da.
#   3. Ein zweiter Lauf auf demselben --user-data-dir blockiert am Profil-Lock.
#      Deshalb bekommt jeder Lauf ein frisches Profil.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

PORT=${PORT:-8099}
OUT=${OUT:-tools/preview/shots}
KILL_AFTER=${KILL_AFTER:-40}
CHROME=${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}
DIAG=""

[ "${1:-}" = "--diag" ] && { DIAG="&diag=1"; shift; }
SIZES=("$@")
[ ${#SIZES[@]} -eq 0 ] && SIZES=(320x568 390x844 430x932 852x393 820x1180 1280x820 1920x1080)

if [ ! -x "$CHROME" ]; then
  echo "Chrome nicht gefunden: $CHROME" >&2
  echo "Pfad per CHROME=... überschreiben." >&2
  exit 2
fi
if ! curl -s -o /dev/null "http://127.0.0.1:$PORT/app/"; then
  echo "Server antwortet nicht auf Port $PORT." >&2
  echo "Erst starten:  python3 tools/preview/server.py $PORT" >&2
  exit 2
fi

mkdir -p "$OUT"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

for vp in "${SIZES[@]}"; do
  W=${vp%x*}; H=${vp#*x}
  NAME="$OUT/${vp}${DIAG:+-diag}.png"
  rm -f "$NAME"
  "$CHROME" --headless=new --disable-gpu --no-sandbox --hide-scrollbars \
    --user-data-dir="$TMP/$vp" --virtual-time-budget=12000 \
    --window-size=$((W + 20)),$((H + 40)) --screenshot="$NAME" \
    "http://127.0.0.1:$PORT/frame?w=$W&h=$H$DIAG" >/dev/null 2>&1 &
  PID=$!
  sleep "$KILL_AFTER"
  kill $PID 2>/dev/null
  wait $PID 2>/dev/null
  if [ -s "$NAME" ]; then
    printf '  %-12s %s\n' "$vp" "$NAME"
  else
    printf '  %-12s FEHLGESCHLAGEN\n' "$vp"
  fi
done
