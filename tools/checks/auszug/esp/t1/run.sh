#!/bin/sh
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel)}
# Pruefstand S.1-S.3 gegen eine ESP-uclock.ino. Aufruf: run.sh <ESP-uclock.ino> <vars.h> <bezeichnung>
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D")
R=$ROOT
python3 "$S/t1/gen.py" "$S" "$1" "$D/bruecke_code.inc" ${NATIV:---ziel} || exit 2
cc -x c++ -std=c++17 -Wall -Wno-unused-function -I"$D" -I"$R/tools/checks" -DVARSHDR="\"$2\"" -DBEZEICHNUNG="\"$3\"" \
   "$D/t1.cpp" -o "$D/t1.bin" -lc++ 2>"$D/cc.log" || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($3):"; head -30 "$D/cc.log"; exit 2; }
"$D/t1.bin"
