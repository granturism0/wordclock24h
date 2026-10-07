#!/bin/sh
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
# run.sh <main.c> <overlay.c> <name>: extrahiert, uebersetzt nativ und mit Zieltypen, laeuft beide.
# Massgeblich ist ZIEL; der native Lauf zeigt nur, ob die Typfalle L256 im Code noch steckt.
D=$(dirname "$0"); X="$D/x-$3.c"; rc=0
python3 "$D/extract.py" "$1" "$2" $ROOT/src/base/base.c "$X" || exit 2
for m in ziel nativ; do
  Z=""; [ $m = ziel ] && Z="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function $Z -I $ROOT/src/overlay -I $ROOT/tools/checks -DSNIPPET="\"$X\"" "$D/a3.c" -o "$D/a3-$3-$m" || exit 2
  echo "=== $3, $m ==="; "$D/a3-$3-$m" | grep -a "FEHL\|Faelle"; if [ $m = ziel ]; then "$D/a3-$3-$m" >/dev/null || rc=1; else echo "  (nativ nur Hinweis: 8-Bit-uint_fast8_t auf dem Host, L256)"; fi
done
exit $rc
