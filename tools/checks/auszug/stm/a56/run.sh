#!/bin/sh
# Pruefstand a56 (Paket 2026-10-13, S2.3, A56): Ambilight-Power uebersteht einen STM-Neustart.
# Aufruf: run.sh <src-Verzeichnis>   Zwischendateien unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
# Laeuft nativ und mit den Zieltypen, je mit beiden Seitengroessen (F103: 8, F411: 32).
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
[ -n "$ROOT" ] || ROOT=$(git rev-parse --show-toplevel 2>/dev/null) || exit 2
D=$(cd "$(dirname "$0")" && pwd); SRC=$1
W=$(mktemp -d "${TMPDIR:-/tmp}/a56.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" || exit 2
rc=0
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN -include $ROOT/tools/checks/pruefstand.h"
  for p in 8 32; do
    cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -DEEPROM_PAGE_SIZE=$p -I "$W" $Z "$D/a56.c" -o "$W/a56" || exit 2
    echo "=== a56, $m, Seite $p ==="; "$W/a56" || rc=1
  done
done
exit $rc
