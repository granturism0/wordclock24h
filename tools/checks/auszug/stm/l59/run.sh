#!/bin/sh
# Pruefstand L59 (A59). Aufruf: run.sh <src-Verzeichnis>. Exit 0 nur, wenn beide Uebersetzungen bestehen.
S=$(cd "$(dirname "$0")" && pwd); SRC=$1
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
X="$S/x-l59.c"; rc=0
python3 "$S/extract.py" "$SRC" "$X" || exit 2
for m in ziel nativ; do
  Z=""; [ $m = ziel ] && Z="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function $Z -I "$ROOT/tools/checks" -DSNIPPET="\"$X\"" "$S/l59.c" -o "$S/l59-$m" || exit 2
  echo "=== L59, $m ==="
  "$S/l59-$m" > "$S/l59-$m.txt"; r=$?
  grep -a "FEHL\|Faelle" "$S/l59-$m.txt"
  [ $r -ne 0 ] && rc=1
done
exit $rc
