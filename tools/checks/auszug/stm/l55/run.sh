#!/bin/sh
# Pruefstand l55 (Paket 2026-10-13, S2.1, B55-STM): RPCR nach "Messwert uebernehmen".
# Aufruf: run.sh <src-Verzeichnis>   Zwischendateien unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
[ -n "$ROOT" ] || ROOT=$(git rev-parse --show-toplevel 2>/dev/null) || exit 2
D=$(cd "$(dirname "$0")" && pwd); SRC=$1
W=$(mktemp -d "${TMPDIR:-/tmp}/l55.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" || exit 2
rc=0
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN -include $ROOT/tools/checks/pruefstand.h"
  cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -I "$W" $Z "$D/l55.c" -o "$W/l55" || exit 2
  echo "=== l55, $m ==="; "$W/l55" || rc=1
done
exit $rc
