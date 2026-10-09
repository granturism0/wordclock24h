#!/bin/sh
# Pruefstand E.1 (L338, L-Befund 200 ms, Paket 2026-10-09). Aufruf: run.sh <verzeichnis mit weather.cpp> <bezeichnung>
# query_weather () samt Parsern aus dem ECHTEN weather.cpp, gegen eine WiFiClient-Attrappe mit
# Core-3.1.2-Semantik auf simulierter Zeit: AKE.1 bis AKE.6.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
python3 "$S/extract.py" range "$Q/weather.cpp" "WiFiClient openweather_client" "void" 2>"$D/extract.log" \
  | sed -e 's/^parse_weather (/real_parse_weather (/' -e 's/^parse_weather_fc (/real_parse_weather_fc (/' > "$D/weather_code.inc"
[ -s "$D/weather_code.inc" ] || { echo "AUSZUG LEER ($B)"; cat "$D/extract.log"; exit 2; }
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/t12.cpp" -o "$D/t12.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t12.bin"
