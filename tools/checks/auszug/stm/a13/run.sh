#!/bin/sh
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
# HAVE_ACK nur, wenn der gepruefte Stand die Quittungszuordnung kennt (S.16). Fest gesetzt
# liess sich ein alter Stand gar nicht uebersetzen, und die Gegenprobe zeigte "kaputt" statt
# "schlaegt an" -- fuer A13 und A20 war damit DIR-014 nicht belegt (doc-writer, 08.10.2026).
grep -q esp8266_ack_drops "$1/esp8266/esp8266.c" 2>/dev/null && HAVE_ACK=-DHAVE_ACK || HAVE_ACK=
# run.sh <src-Verzeichnis> <name>: A20-Lauf des AKS.5-Pruefstands, nativ und mit Zieltypen
S=$(dirname "$0")/..; X="$S/x-a13-$2.c"; rc=0
python3 "$S/extract.py" "$1" "$X" >/dev/null || exit 2
for m in nativ ziel; do
  Z=""; [ $m = ziel ] && Z="-DZIELTYPEN"
  cc -std=c99 -Wall -Wno-unused-function -Wno-tautological-constant-out-of-range-compare -DA13 $Z $HAVE_ACK -I "$1/esp8266" -I "$1/vars" \
     -I $ROOT/tools/checks -DSNIPPET="\"$X\"" "$S/aks5.c" -o "$S/a13/a13-$2-$m" || exit 2
  echo "=== $2, $m ==="; "$S/a13/a13-$2-$m" | grep -a "Log:\|FEHL\|Faelle"; "$S/a13/a13-$2-$m" >/dev/null || rc=1
done
exit $rc
