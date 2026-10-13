#!/bin/sh
# Pruefstand c6h (Paket 2026-10-13, E1.1/E1.2; AKH.1-AKH.3). Aufruf: run.sh <verzeichnis mit http.cpp und vars.h> <bezeichnung>
#
# Teil 1 (c6h.cpp): echter Code, jede Kombination Sec-Fetch-Dest x Sec-Fetch-Site gegen jeden zerstoerenden Weg.
# Teil 2 (static.py): Bestandsliste aller Wege zu stm32_reset (), RESET_EEPROM_RPC_VAR, LittleFS.format () und
#   LittleFS.remove () in http.cpp - jeder muss hinter der Herkunftspruefung liegen; ein neuer Weg ohne Pruefung
#   und ein neuer Weg, der nicht in der Liste steht, schlagen an.
# Exit 0 OK, 1 FEHL, 2 Auszug oder Uebersetzung gescheitert.
D=$(cd "$(dirname "$0")" && pwd)
# ohne Argument: der Arbeitsbaum, in dem der Aufruf steht
ROOT=${ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null || git rev-parse --show-toplevel 2>/dev/null)}
Q=$(cd "${1:-$ROOT/ESP8266/ESP-uclock}" 2>/dev/null && pwd) || { echo "Verzeichnis fehlt: ${1:-$ROOT/ESP8266/ESP-uclock}"; exit 2; }; B=${2:-neu}
[ -f "$Q/http.cpp" ] || { echo "http.cpp fehlt in $Q"; exit 2; }
X="$D/ausschnitt.py"
{
  python3 "$X" lines "$Q/http.cpp" '^#define (HTTP_API_ERROR_[A-Z_]+|MAX_UPDATE_FILENAME_LEN|HTTP_API_MAX_QUERY_LEN)\s'
} > "$D/c6h_defs.inc" 2>"$D/extract.log"
{
  python3 "$X" lines "$Q/http.cpp" '^static char +http_request_fetch_(dest|site)\['
  python3 "$X" func "$Q/http.cpp" http_clear_request_fetch_dest http_capture_request_fetch_dest http_capture_request_fetch_site \
      http_request_is_embedded_subresource http_request_is_cross_origin http_request_denied_by_origin http_deny_embedded_subresource \
      http_get_string_param http_strvar_len_ok http_legacy_set_update_source http_send_update_source_rejected \
      http_api_maintenance_format_fs http_api_maintenance_reset_stm32 http_api_maintenance_reset_eeprom http_api_fs_remove
} > "$D/c6h_code.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" http_server_loop 'while (read_line (sHeaderLine))' > "$D/c6h_loop.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" http_main 'if (*action)' > "$D/c6h_main.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" http_fs 'if (*action' > "$D/c6h_fs.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" http_update 'if (*action)' > "$D/c6h_update.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" flash_stm32_local 'if (*action)' > "$D/c6h_local.inc" 2>>"$D/extract.log"
for f in c6h_defs c6h_code c6h_loop c6h_main c6h_fs c6h_update c6h_local; do
  [ -s "$D/$f.inc" ] || { echo "AUSZUG LEER ($B): $f"; cat "$D/extract.log"; exit 2; }
done
grep -q 'http_api_fs_remove (' "$D/c6h_code.inc" && grep -q 'http_api_maintenance_reset_eeprom (' "$D/c6h_code.inc" \
  || { echo "AUSZUG OHNE API-HANDLER ($B)"; exit 2; }
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable \
   -I"$D" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/c6h.cpp" -o "$D/c6h.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/c6h.bin"; r1=$?
python3 "$D/static.py" "$Q/http.cpp" "$B"; r2=$?
[ $r1 -ge 2 ] || [ $r2 -ge 2 ] && exit 2
[ $r1 -ne 0 ] || [ $r2 -ne 0 ] && { echo "FEHL ($B): c6h"; exit 1; }
echo "OK ($B): c6h"; exit 0
