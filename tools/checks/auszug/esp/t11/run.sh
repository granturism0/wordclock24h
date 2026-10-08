#!/bin/sh
# Pruefstand Review F.6 A2: http_remote_stm32_filename_matches () mit Praefixen ungleich 6 Zeichen.
# Aufruf: run.sh <verzeichnis mit http.cpp, vars.h> <bezeichnung>
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
python3 "$S/extract.py" func "$Q/http.cpp" http_build_stm32_default_filename http_remote_stm32_filename_matches \
  > "$D/a2_code.inc" 2>"$D/extract.log"
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" \
   "$D/t11.cpp" -o "$D/t11.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t11.bin"
