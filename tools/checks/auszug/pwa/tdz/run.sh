#!/bin/sh
# pwa-developer: TDZ-Pruefstand (L411).
# Aufruf: run.sh <ESP-Verzeichnis>   (darin data/app/app.js und data/app/index.html)
# Startet die PWA in node/vm einmal je Modul aus index.html (plus ohne gespeichertes Modul
# und mit ungueltigem Namen), je mit leerem und mit voll belegtem localStorage, und meldet
# jeden ReferenceError -- geworfen, geloggt, still gefangen oder von einem Promise geschluckt.
# Exit 0 = OK, 1 = FEHL (ReferenceError oder Abbruch des Skripts), 2 = Pruefstand kaputt.
# Laeuft ausschliesslich auf Kopien in einem Temp-Verzeichnis.
HERE=$(cd "$(dirname "$0")" && pwd)
ESP="$1"
if [ -z "$ESP" ] || [ ! -s "$ESP/data/app/app.js" ] || [ ! -s "$ESP/data/app/index.html" ]; then
  echo "PRUEFSTAND KAPUTT: app.js/index.html unter '$ESP/data/app' fehlen oder sind leer" >&2
  exit 2
fi
WORK=$(mktemp -d "${TMPDIR:-/tmp}/tdz.XXXXXX") || exit 2
trap 'rm -rf "$WORK"' EXIT
cp "$ESP/data/app/app.js" "$ESP/data/app/index.html" "$WORK/" || exit 2
node "$HERE/harness.mjs" "$WORK/app.js" "$WORK/index.html" > "$WORK/out.json" 2> "$WORK/err.txt"
rc=$?
if [ $rc -ne 0 ]; then
  echo "PRUEFSTAND KAPUTT: harness exit $rc" >&2; cat "$WORK/err.txt" >&2
  exit 2
fi
node "$HERE/evaluate.mjs" "$WORK/out.json"
exit $?
