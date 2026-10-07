#!/bin/sh
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
# run.sh <rtc.c> <name> [-DHAVE_HALF]: extrahiert, uebersetzt nativ und mit Zieltypen, laeuft beide
D=$(dirname "$0"); X="$D/x-$2.c"; rc=0
python3 "$D/extract-rtc.py" "$1" "$X" || exit 2
for m in nativ ziel; do
  Z=""; [ $m = ziel ] && Z="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function $Z $3 -I $ROOT/tools/checks -DSNIPPET="\"$X\"" "$D/a5.c" -o "$D/a5-$2-$m" || exit 2
  echo "=== $2, $m ==="; "$D/a5-$2-$m" | grep -a "FEHL\|Faelle"; "$D/a5-$2-$m" >/dev/null || rc=1
done
exit $rc
