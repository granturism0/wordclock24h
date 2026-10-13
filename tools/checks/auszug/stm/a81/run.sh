#!/bin/sh
# Pruefstand a81 (Paket 2026-10-13, S2.5, A81): Marker-Weissfarbe des Ambilights uebersteht einen STM-Neustart.
# Aufruf: run.sh <src-Verzeichnis>   Zwischendateien unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
# Laeuft nativ und mit den Zieltypen, je fuer RGBW und RGB und mit beiden Seitengroessen (F103: 8, F411: 32).
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
[ -n "$ROOT" ] || ROOT=$(git rev-parse --show-toplevel 2>/dev/null) || exit 2
D=$(cd "$(dirname "$0")" && pwd); SRC=$1
[ -d "$SRC" ] || { echo "run.sh: src-Verzeichnis fehlt" >&2; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/a81.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" || exit 2
rc=0
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN -include $ROOT/tools/checks/pruefstand.h"
  for w in 1 0; do
    for p in 8 32; do
      cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -DEEPROM_PAGE_SIZE=$p -DDSP_USE_SK6812_RGBW=$w -I "$W" $Z "$D/a81.c" -o "$W/a81" || exit 2
      echo "=== a81, $m, RGBW $w, Seite $p ==="; "$W/a81" || rc=1
    done
  done
done
exit $rc
