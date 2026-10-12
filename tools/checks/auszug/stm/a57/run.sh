#!/bin/sh
# Pruefstand A57 (Paket 2026-10-11, S3.8): "Messwert uebernehmen" weist eine Inversion ab, Zahlen-Setter nicht.
# Aufruf: run.sh <src-Verzeichnis> [sabotage]   Sabotagen: gleichheit maxseite ohnelog eeprom
# Zwischendateien unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
D=$(cd "$(dirname "$0")" && pwd); SRC=$1; SAB=${2:-}
W=$(mktemp -d "${TMPDIR:-/tmp}/a57.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" $SAB || exit 2
rc=0
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN -include $ROOT/tools/checks/pruefstand.h"
  cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -I "$W" $Z "$D/a57.c" -o "$W/a57" || exit 2
  echo "=== A57, $m${SAB:+, Sabotage $SAB} ==="; "$W/a57" || rc=1
done
exit $rc
