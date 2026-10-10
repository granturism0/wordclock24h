#!/bin/sh
# Pruefstand C55/C44 (Paket 2026-10-10, E.2). Aufruf: run.sh <verzeichnis mit http.cpp, base.cpp, vars.h> <bezeichnung>
#
# Laeuft die Fall-Tabelle regeln.tsv gegen die ECHTEN Schreibwege aus http.cpp: /api/date_ticker_format_set,
# /api/weather_coordinates_set und die Legacy-Zweige savedtf und savelonlat. Zusaetzlich (nur im neuen
# Stand, wo es die Prueffunktionen gibt) ein erschoepfender Vergleich gegen ein unabhaengig formuliertes
# Orakel (std::regex + strtod). Gegen den Ausgangsstand laufen die Tabellenfaelle ueber dieselben
# Schreibwege; die nehmen dort <"& und <scri an => FEHL.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
X="$S/extract.py"; [ -f "$X" ] || X="$(git -C "$D" rev-parse --show-toplevel 2>/dev/null)/tools/checks/auszug/esp/extract.py"
{
  python3 "$X" range "$Q/http.cpp" "#define HTTP_API_ERROR_MISSING_VALUE" "#define HTTP_API_ERROR_TOO_SHORT"
  python3 "$X" func "$Q/base.cpp" strsubst
  # nur neu: die Meldung steht bei den Prototypen
  python3 - "$Q/http.cpp" <<'PY'
import sys
L = open(sys.argv[1], 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
for i, l in enumerate(L):
    if l.startswith('static const char') and 'http_date_ticker_format_invalid[]' in l:
        j = i
        while not L[j].rstrip().endswith(';'):
            j += 1
        print('\n'.join(L[i:j + 1]))
        print('#define HAVE_REGEL 1')
        break
PY
  python3 "$X" func "$Q/http.cpp" http_get_string_param http_strvar_len_ok http_check_strvar_len
} > "$D/regeln_code.inc" 2>"$D/extract.log"
python3 "$X" func "$Q/http.cpp" http_date_ticker_format_ok http_coordinate_ok http_coordinates_check > "$D/regeln_neu.inc" 2>>"$D/extract.log"
python3 "$X" func "$Q/http.cpp" http_api_date_ticker_format_set http_api_weather_coordinates_set > "$D/regeln_api.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" 'else if (! strcmp (action, "savedtf"))' 'else if (! strcmp (action, "testdisplay"))' > "$D/regeln_dtf.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" 'else if (! strcmp (action, "savelonlat"))' 'else if (! strcmp (action, "getweather"))' > "$D/regeln_lonlat.inc" 2>>"$D/extract.log"
for f in regeln_code regeln_api regeln_dtf regeln_lonlat; do
  [ -s "$D/$f.inc" ] || { echo "AUSZUG LEER ($B): $f"; cat "$D/extract.log"; exit 2; }
done
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-deprecated-declarations -I"$D" -DVARSHDR="\"$Q/vars.h\"" \
   -DTABELLE="\"$D/regeln.tsv\"" -DBEZEICHNUNG="\"$B\"" "$D/regeln.cpp" -o "$D/regeln.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/regeln.bin"
