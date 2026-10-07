#!/bin/sh
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
# run.sh <main.c> <tftled.c> <esp-spiffs.c> <esp8266-Verzeichnis> <name>
D=$(dirname "$0"); X="$D/x-$5.c"; rc=0
python3 "$D/extract.py" "$1" "$2" "$3" $ROOT/src/base/base.c "$X" || exit 2
for m in ziel nativ; do Z=""; [ $m = ziel ] && Z="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function -Wno-tautological-constant-out-of-range-compare $Z -I "$4" -I $ROOT/tools/checks -DSNIPPET="\"$X\"" "$D/a46.c" -o "$D/a46-$5" || exit 2
  echo "=== $5, $m ==="; "$D/a46-$5" | grep -a "FEHL\|Faelle\|letzte"; "$D/a46-$5" >/dev/null || rc=1; done
exit $rc
