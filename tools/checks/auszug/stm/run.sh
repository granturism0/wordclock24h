#!/bin/sh
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
# HAVE_ACK nur, wenn der gepruefte Stand die Quittungszuordnung kennt (S.16). Fest gesetzt
# liess sich ein alter Stand gar nicht uebersetzen, und die Gegenprobe zeigte "kaputt" statt
# "schlaegt an" -- fuer A13 und A20 war damit DIR-014 nicht belegt (doc-writer, 08.10.2026).
grep -q esp8266_ack_drops "$1/esp8266/esp8266.c" 2>/dev/null && HAVE_ACK=-DHAVE_ACK || HAVE_ACK=
# Aufruf: run.sh <src-Verzeichnis> <name>   -- extrahiert, uebersetzt nativ und mit Zieltypen, laeuft beide
S=$(dirname "$0"); SRC=$1; N=$2; X="$S/x-$N.c"; EXTRA=$3
python3 "$S/extract.py" "$SRC" "$X" >/dev/null || exit 2
rc=0
for mode in nativ ziel; do
  D=""; [ $mode = ziel ] && D="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function -Wno-tautological-constant-out-of-range-compare $D $EXTRA $HAVE_ACK -I "$SRC/esp8266" -I "$SRC/vars" \
     -I $ROOT/tools/checks -DSNIPPET="\"$X\"" "$S/aks5.c" -o "$S/aks5-$N-$mode" || exit 2
  echo "=== $N, $mode ==="
  "$S/aks5-$N-$mode" | tail -1
  "$S/aks5-$N-$mode" >/dev/null || rc=1
done
exit $rc
