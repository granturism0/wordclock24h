#!/bin/sh
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
D=$(dirname "$0"); X="$D/x-$2.c"; rc=0
python3 "$D/extract.py" "$1" "$X" || exit 2
for m in ziel nativ; do Z=""; [ $m = ziel ] && Z="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function -Wno-format $Z -I $ROOT/tools/checks -DSNIPPET="\"$X\"" "$D/a14.c" -o "$D/a14-$2" || exit 2
  echo "=== $2, $m ==="; "$D/a14-$2" | grep -a "FEHL\|Faelle"; "$D/a14-$2" >/dev/null || rc=1; done
exit $rc
