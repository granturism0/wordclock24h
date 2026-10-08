#!/bin/sh
# Pruefstand F.5e (Overlay-Text, ESP / L319). Aufruf: run.sh <verzeichnis mit http.cpp, vars.h> <bezeichnung>
# http_api_overlay_set () weist einen Text ueber 32 Byte mit Kennung 2 ab; der gespeicherte
# Text bleibt unveraendert, set_overlay_var () wird nicht gerufen. Gezaehlt werden Byte.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
python3 "$S/extract.py" func "$Q/http.cpp" http_get_int_param http_get_opt_int_param http_strvar_len_ok http_check_strvar_len http_api_overlay_set \
  > "$D/ovl_code.inc" 2>"$D/extract.log"
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" \
   "$D/t10.cpp" -o "$D/t10.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t10.bin"
