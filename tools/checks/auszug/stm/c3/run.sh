#!/bin/sh
# Pruefstand C3 (L339, AKC.2-AKC.5). Aufruf: run.sh <src-Verzeichnis> [bezeichnung]
# Uebersetzt fuer BEIDE Seitengroessen (F411: 32, F103: 8), je nativ und mit -DPRUEFSTAND_SCHNELLE_TYPEN,
# und laesst alle vier laufen. Exit 0 nur, wenn alle vier bestehen; 2, wenn etwas nicht uebersetzt.
# Sabotage (DIR-014) ueber die Umgebung, nur fuer den Nachweis, dass der Pruefstand anschlaegt:
#   C3_SABOTAGE=1  Firmware-Seite 64 gegen Baustein 32 (nur das F411-Ziel)
#   C3_SABOTAGE=2  Firmware-Seite 32 gegen Baustein 8  (nur das F411-Ziel)
#   C3_SABOTAGE=3  Seitenende um eins verschoben (beide Ziele)
S=$(cd "$(dirname "$0")" && pwd); SRC=$1
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
SAB=${C3_SABOTAGE:-0}
X="$S/x-c3.c"; H="$S/x-c3-bereiche.h"
python3 "$S/extract.py" "$SRC" "$X" "$H" "$SAB" || exit 2
rc=0
for ziel in F4XX F10X; do
  for mode in nativ ziel; do
    D=""; [ $mode = ziel ] && D="-DPRUEFSTAND_SCHNELLE_TYPEN"
    B=""; [ "$SAB" = 2 ] && [ $ziel = F4XX ] && B="-DBAUSTEIN_P=8"
    cc -std=c99 -O1 -Wall -Wno-unused-function $D $B -DSTM32$ziel -I "$ROOT/tools/checks" \
       -DSNIPPET="\"$X\"" -DREF="\"$S/ref.c\"" -DBEREICHE="\"$H\"" "$S/c3.c" -o "$S/c3-$ziel-$mode" || exit 2
    echo "=== C3, STM32$ziel, $mode ==="
    "$S/c3-$ziel-$mode" > "$S/c3-$ziel-$mode.txt"; r=$?
    grep -a -E "FEHL|Faelle|Pruefstand|Kriterien|    [A-Z]" "$S/c3-$ziel-$mode.txt"
    [ $r -ne 0 ] && rc=1
  done
done
exit $rc
