#!/bin/sh
# Pruefstand c65i (E4.1; AKV.1, AKV.2). Aufruf: run_i.sh <ESP-Quellverzeichnis> <bezeichnung>. Exit 0 OK, 1 FEHL, 2 KAPUTT.
# Allgemein: run_x.sh <x> <ESP-Quellverzeichnis> <bezeichnung> fuer c65i, c65w, c65k, c65z, c65o.
D=$(cd "$(dirname "$0")" && pwd); W=$1; Q=$2; B=${3:-neu}
T=$(mktemp -d "${TMPDIR:-/tmp}/c65$W.XXXXXX"); trap 'rm -rf "$T"' EXIT
python3 "$D/auszug.py" "$Q" "$T" > "$T/auszug.log" 2>&1 || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; cat "$T/auszug.log"; exit 2; }
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format \
   -I"$T" -I"$D" -DBEZEICHNUNG="\"$B\"" "$D/c65$W.cpp" -o "$T/c65$W.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -15; exit 2; }
"$T/c65$W.bin"; rc=$?
if [ -f "$D/c65${W}_static.py" ]; then python3 "$D/c65${W}_static.py" "$Q" "$B"; rs=$?; else rs=0; fi
[ $rc -eq 0 ] && [ $rs -eq 0 ] && exit 0
[ $rc -gt 1 ] && exit 2
exit 1
