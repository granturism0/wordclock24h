#!/bin/sh
# run.sh <src-Verzeichnis> [ALT]: Pruefstand D, nativ und mit -DPRUEFSTAND_SCHNELLE_TYPEN.
# Ohne ALT: Erwartung Teil D (gegen den C3-Stand FEHL). Mit ALT: Erwartung "alter STM" (AKD.8, soll gegen den Ausgangsstand bestehen).
S=$(cd "$(dirname "$0")" && pwd); SRC=$1; MODE=${2:-}
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
X="$S/x-d.c"
python3 "$S/extract.py" "$SRC" "$X" || exit 2
cc -I "$ROOT/tools/checks" -o "$S/d-varcrc" "$ROOT/tools/checks/var-crc.c" || exit 2
"$S/d-varcrc" | awk '/^  [^ -]/ && $NF ~ /^\*[0-9a-f][0-9a-f][0-9a-f][0-9a-f]$/ && $1 != "Nutzlast" { n = ($1 == "(leer)") ? "" : $1; printf "{ \"%s\", \"%s\" },\n", n, $NF }' > "$S/x-vek.txt"
[ "$(wc -l < "$S/x-vek.txt")" -ge 8 ] || { echo "Schiedsrichter-Vektoren fehlen"; exit 2; }
{ echo "static const char * const REF_VEKTOREN[][2] = {"; cat "$S/x-vek.txt"; echo "};"; } > "$S/x-vektoren.h"
HAVE=""; grep -q esp8266_cmc_sync "$SRC/esp8266/esp8266.h" 2>/dev/null || HAVE="-Desp8266_cmc_sync=esp8266_cmc_sync_fehlt -DKEIN_D"
rc=0
for mode in nativ ziel; do
  Z=""; [ $mode = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN"
  A=""; [ "$MODE" = ALT ] && A="-DALT"
  cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -Wno-tautological-constant-out-of-range-compare $Z $A -I "$S" -I "$SRC/esp8266" -I "$SRC/vars" -I "$ROOT/tools/checks" \
     -DSNIPPET="\"$X\"" "$S/d.c" -o "$S/stm-d-$mode" || exit 2
  echo "=== D ${MODE:-neu}, $mode ==="
  "$S/stm-d-$mode" > "$S/stm-d-$mode.txt"; r=$?
  grep -a -E "^FEHL|Faelle|verschmolzene" "$S/stm-d-$mode.txt" | sort | uniq -c | sort -rn | head -12
  [ $r -ne 0 ] && rc=1
done
exit $rc
