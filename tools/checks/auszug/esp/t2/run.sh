#!/bin/sh
# Pruefstand S.4 (C31). Aufruf: run.sh <eepromdata.cpp> <bezeichnung>
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D")
python3 "$S/extract.py" func "$1" eeprom_read > "$D/eeprom_code.inc" || exit 2
cc -x c++ -std=c++17 -Wall -I"$D" -DBEZEICHNUNG="\"$2\"" "$D/t2.cpp" -o "$D/t2.bin" -lc++ 2>"$D/cc.log" || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($2):"; head -20 "$D/cc.log"; exit 2; }
"$D/t2.bin"
