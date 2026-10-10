#!/bin/sh
# Pruefstand A2-Nachtrag (Paket 2026-10-10, Schritt E): Temperaturindex bei Korrekturaenderung.
# Aufruf: run.sh <verzeichnis mit http.cpp, vars.h> <bezeichnung>
# Echter Code: http_api_temperature_correction_set () samt Hilfsfunktionen (dieselben Auszuege wie t3) und
# die Legacy-Zweige "savetcorrrtc"/"savetcorrds18xx" aus http_temperature (). Gegen den Ausgangsstand
# uebersetzbar (http_temp_index_correct () kommt dort nur nicht vor) => FEHL, nicht KAPUTT.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
X="$S/extract.py"; [ -f "$X" ] || X="$(git -C "$D" rev-parse --show-toplevel 2>/dev/null)/tools/checks/auszug/esp/extract.py"
{ python3 "$X" func "$Q/http.cpp" http_decode_temp_correction http_encode_temp_correction http_clamp_temp_correction
  python3 "$X" range "$Q/http.cpp" "#define RTC_TEMP_HALF_DEG_UNKNOWN" "/* Anzuzeigende"
  python3 "$X" func "$Q/http.cpp" http_rtc_temp_half_deg_correct http_api_temperature_correction_set
} > "$D/tidx_code.inc" 2>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" 'if (! strcmp (action, "savetcorrrtc"))' 'else if (! strcmp (action, "displaytemperature"))' > "$D/tidx_legacy.inc" 2>>"$D/extract.log"
for f in tidx_code tidx_legacy; do [ -s "$D/$f.inc" ] || { echo "AUSZUG LEER ($B): $f"; cat "$D/extract.log"; exit 2; }; done
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" \
   "$D/tidx.cpp" -o "$D/tidx.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/tidx.bin"
