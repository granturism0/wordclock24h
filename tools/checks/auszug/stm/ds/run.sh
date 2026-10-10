#!/bin/sh
# Pruefstand DS (A2). Aufruf: run.sh <src-Verzeichnis> [sabotage]. Exit 0 nur, wenn beide Uebersetzungen bestehen.
S=$(cd "$(dirname "$0")" && pwd); SRC=$1; SAB=$2
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
rc=0
python3 "$S/extract.py" "$SRC" "$S/x-ds.c" $SAB || exit 2
for m in ziel nativ; do
  Z=""; [ $m = ziel ] && Z="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable $Z -I "$ROOT/tools/checks" -I "$S" "$S/ds.c" -o "$S/ds-$m" || exit 2
  echo "=== DS, $m ==="
  "$S/ds-$m" > "$S/ds-$m.txt"; r=$?
  grep -a "FEHL\|Faelle\|HINWEIS" "$S/ds-$m.txt"
  [ $r -ne 0 ] && rc=1
done
exit $rc
