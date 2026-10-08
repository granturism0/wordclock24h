#!/bin/sh
# Pruefstand F.5d (C9k / L172). Aufruf: run.sh <verzeichnis mit httpclient.cpp> <bezeichnung>
# httpclient_read_header () wartet ueber httpclient_wait_for_data (): ein Kopf in zwei
# Segmenten wird vollstaendig gelesen, ein abgerissener Kopf ist ein Fehlschlag statt 200/len 0.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
{ python3 "$S/extract.py" range "$Q/httpclient.cpp" "#define HTTPCLIENT_READ_TIMEOUT" "static bool"
  python3 "$S/extract.py" func "$Q/httpclient.cpp" httpclient_wait_for_data httpclient_read_header
} > "$D/c9k_code.inc" 2>"$D/extract.log"
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/t9.cpp" -o "$D/t9.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t9.bin"
