#!/bin/sh
# Pruefstand GT (S0, Paket 2026-10-10): my_gmtime() mit 32 Bit gegen die eingefrorene
# 64-Bit-Fassung (base-alt.c, aus release/3.2.25-3.2.30-1.4.94) und gegen gmtime_r() des Hosts.
# Aufruf: run.sh <src-Verzeichnis> [sabotage]
# S0 aendert das Verhalten nicht -- eine Gegenprobe gegen das alte Release muss deshalb
# BESTEHEN. Den Nachweis nach DIR-014 liefert "sabotage" (Jahrhundertregel entfernt).
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
D=$(cd "$(dirname "$0")" && pwd); SRC=$1; rc=0
python3 "$D/extract.py" "$D/base-alt.c" "$D/x-alt.c" || exit 2
python3 "$D/extract.py" "$SRC/base/base.c" "$D/x-neu.c" ${2:-} || exit 2
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN"
  W="-std=c99 -Wall -Wextra -Wno-sign-compare -I $ROOT/tools/checks $Z"
  cc $W -Dmy_gmtime=gm_alt -DSNIPPET="\"$D/x-alt.c\"" -c "$D/impl.c" -o "$D/alt.o" || exit 2
  cc $W -Dmy_gmtime=gm_neu -DSNIPPET="\"$D/x-neu.c\"" -c "$D/impl.c" -o "$D/neu.o" || exit 2
  cc $W -c "$D/gt.c" -o "$D/gt.o" && cc "$D/gt.o" "$D/alt.o" "$D/neu.o" -o "$D/gt" || exit 2
  echo "=== GT, $m ==="; TZ=UTC "$D/gt" || rc=1
done
exit $rc
