#!/bin/sh
# Pruefstand F.5c (C25 / L270). Aufruf: run.sh <verzeichnis mit http.cpp> <bezeichnung>
# http_flush () zaehlt Browser-Abbau (gone) und Schreibversagen (fail) getrennt, und die
# Verlustzeile im Logring traegt beide Zaehler.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
{ python3 "$S/extract.py" range "$Q/http.cpp" "#define MAX_HTTP_RESPONSE_LEN" "/* Verbindungen, die angenommen"
  python3 "$S/extract.py" func "$Q/http.cpp" http_flush
} > "$D/c25_code.inc" 2>"$D/extract.log"
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/t8.cpp" -o "$D/t8.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t8.bin"
