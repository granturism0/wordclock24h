#!/bin/sh
# Pruefstand C53/L356 . Aufruf: run.sh <verzeichnis mit http.cpp> <bezeichnung>
# Das Warten auf die erste Anfragezeile in http_server_loop () aus dem ECHTEN http.cpp, gegen eine
# WiFiClient-Attrappe mit Core-3.1.2-Semantik auf simulierter Zeit in Mikrosekunden.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
grep -a -E '^#define (HTTP_FIRST_LINE_TIMEOUT|HTTP_DEBUG_CLIENT_LOG) |^static uint16_t +http_no_request_(timeouts|aborts) ' "$Q/http.cpp" > "$D/defs.inc"
[ "$(wc -l < "$D/defs.inc")" -eq 4 ] || { echo "DEFINITIONEN UNVOLLSTAENDIG ($B)"; cat "$D/defs.inc"; exit 2; }
python3 "$S/extract.py" block "$Q/http.cpp" "unsigned long start_millis    = millis ();" "http_client.setNoDelay(1);" > "$D/block.inc" 2>"$D/extract.log"
[ -s "$D/block.inc" ] || { echo "AUSZUG LEER ($B)"; cat "$D/extract.log"; exit 2; }
grep -q 'while ((millis () - start_millis) < HTTP_FIRST_LINE_TIMEOUT)' "$D/block.inc" || { echo "AUSZUG OHNE WARTESCHLEIFE ($B)"; exit 2; }
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/t15.cpp" -o "$D/t15.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t15.bin"
