#!/bin/sh
# Pruefstand A60 (Paket 2026-10-11, S2.1): I2C-Fristen (eng pollen, beide BUSY-Schleifen).
# Aufruf: run.sh <src-Verzeichnis> [sabotage]      Sabotagen: fristnie busyohne msec ohnelog ohnefrei nurbusy afignoriert flagnie flagbleibt
# Zwischendateien in einem eigenen Arbeitsverzeichnis unter $TMPDIR, NIE in diesem Verzeichnis.
# Exit 0 = OK, 1 = FEHL, 2 = kaputt.
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
D=$(cd "$(dirname "$0")" && pwd); SRC=$1; SAB=${2:-}
[ -f "$ROOT/tools/checks/pruefstand.h" ] || { echo "a60: pruefstand.h nicht gefunden (ROOT=$ROOT)"; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/a60.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" $SAB || exit 2
rc=0
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN"
  cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -I "$W" -I "$ROOT/tools/checks" $Z $(cat "$W/flags") "$D/a60.c" -o "$W/a60" || exit 2
  echo "=== A60, $m${SAB:+, Sabotage $SAB} ==="; "$W/a60" || rc=1
done
exit $rc
