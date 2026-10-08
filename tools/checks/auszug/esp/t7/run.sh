#!/bin/sh
# Pruefstand F.5b (E12). Aufruf: run.sh <verzeichnis mit http.cpp> <bezeichnung>
# tft_flags_set und dfplayer_bell_flags_set weisen einen fehlenden Parameter mit Kennung 1 ab
# (unbekannter Wert: 2), statt das Flag zu loeschen -- und schreiben dann nichts.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
python3 "$S/extract.py" func "$Q/http.cpp" http_get_on_off_value http_get_on_off_required http_api_tft_flags_set http_api_dfplayer_bell_flags_set \
  > "$D/e12_code.inc" 2>"$D/extract.log"
# Vor F.5b gibt es http_get_on_off_required () nicht: Attrappe, damit die alte Fassung uebersetzt und ANSCHLAEGT
grep -q '^http_get_on_off_required' "$D/e12_code.inc" || echo 'static int http_get_on_off_required (const char *) { return 0; }' >> "$D/e12_code.inc"
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/t7.cpp" -o "$D/t7.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t7.bin"
