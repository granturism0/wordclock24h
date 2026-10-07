#!/bin/sh
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
# run.sh <tables.c> <name>: beide Uebersetzungen, WCLOCK24H 0 und 1; Pruefsummen nach $D/sum-<name>.txt
D=$(dirname "$0"); X="$D/x-$2.c"; rc=0; : > "$D/sum-$2.txt"
python3 "$D/extract.py" "$1" "$X" || exit 2
for w in 0 1; do for m in ziel nativ; do
  Z=""; [ $m = ziel ] && Z="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function $Z -DWCLOCK24H=$w -I "$D" -I $ROOT/src/tables -I $ROOT/tools/checks -DSNIPPET="\"$X\"" "$D/a24.c" -o "$D/a24-$2" || exit 2
  echo "=== $2, WCLOCK24H=$w, $m ==="; "$D/a24-$2" | grep -a "FEHL\|Faelle\|Pruefsumme" | tee -a "$D/sum-$2.txt" | grep -v Pruefsumme; "$D/a24-$2" >/dev/null || rc=1
done; done
exit $rc
