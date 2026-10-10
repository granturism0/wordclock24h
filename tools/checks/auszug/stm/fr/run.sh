#!/bin/sh
# Pruefstand FR (A50). Aufruf: run.sh <src-Verzeichnis>
# Uebersetzt nativ und mit -DPRUEFSTAND_SCHNELLE_TYPEN, laesst beide laufen. Exit 0 nur, wenn beide bestehen.
S=$(cd "$(dirname "$0")" && pwd); SRC=$1
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
X="$S/x-fr.c"
python3 "$S/extract.py" "$SRC" "$X" || exit 2
# Der Ausgangsstand kennt FAULT_RESET_WAIT_MS nicht; die Rechnungstabelle braucht es nur zum Drucken.
grep -q FAULT_RESET_WAIT_MS "$X" || echo "#define FAULT_RESET_WAIT_MS 0UL" >> "$X"
rc=0
for mode in nativ ziel; do
  D=""; [ $mode = ziel ] && D="-DPRUEFSTAND_SCHNELLE_TYPEN"
  cc -std=c99 -O2 -Wall -Wno-unused-function $D -I "$ROOT/tools/checks" -DSNIPPET="\"$X\"" "$S/fr.c" -o "$S/fr-$mode" || exit 2
  echo "=== FR, $mode ==="
  "$S/fr-$mode" > "$S/fr-$mode.txt"; r=$?
  grep -E "FEHL|Faelle" "$S/fr-$mode.txt"
  [ $r -ne 0 ] && rc=1
done
exit $rc
