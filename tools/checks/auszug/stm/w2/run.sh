#!/bin/sh
# Pruefstand W2 (A2). Aufruf: run.sh <src-Verzeichnis>
# Uebersetzt nativ und mit -DPRUEFSTAND_SCHNELLE_TYPEN, laesst beide laufen. Exit 0 nur, wenn beide bestehen.
S=$(cd "$(dirname "$0")" && pwd); SRC=$1
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
X="$S/x-w2.c"
# HAVE_A2 nur, wenn der gepruefte Stand A2 kennt -- sonst waere der Ausgangsstand nicht uebersetzbar.
grep -q var_weather_query_end "$SRC/vars/vars.c" 2>/dev/null && HAVE_A2=-DHAVE_A2 || HAVE_A2=
python3 "$S/extract.py" "$SRC" "$X" || exit 2
rc=0
for mode in nativ ziel; do
  D=""; [ $mode = ziel ] && D="-DPRUEFSTAND_SCHNELLE_TYPEN"
  cc -std=c99 -Wall -Wno-unused-function -Wno-tautological-constant-out-of-range-compare $D $HAVE_A2 -I "$SRC/esp8266" -I "$SRC/vars" -I "$SRC/weather" \
     -I "$ROOT/tools/checks" -DSNIPPET="\"$X\"" "$S/w2.c" -o "$S/w2-$mode" || exit 2
  echo "=== W2, $mode ==="
  "$S/w2-$mode" > "$S/w2-$mode.txt"; r=$?
  grep -E "FEHL|Faelle" "$S/w2-$mode.txt"
  [ $r -ne 0 ] && rc=1
done
exit $rc
