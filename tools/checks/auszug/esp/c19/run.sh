#!/bin/sh
# Pruefstand c19 (Paket 2026-10-13, E1.4; AKU.1). Aufruf: run.sh <verzeichnis mit http.cpp und vars.h> <bezeichnung>
# Exit 0 OK, 1 FEHL, 2 Auszug oder Uebersetzung gescheitert.
D=$(cd "$(dirname "$0")" && pwd)
# ohne Argument: der Arbeitsbaum, in dem der Aufruf steht
ROOT=${ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null || git rev-parse --show-toplevel 2>/dev/null)}
Q=$(cd "${1:-$ROOT/ESP8266/ESP-uclock}" 2>/dev/null && pwd) || { echo "Verzeichnis fehlt: ${1:-$ROOT/ESP8266/ESP-uclock}"; exit 2; }; B=${2:-neu}
X="$D/ausschnitt.py"
python3 "$X" lines "$Q/http.cpp" '^#define (HTTP_API_ERROR_[A-Z_]+|MAX_UPDATE_FILENAME_LEN)\s' > "$D/c19_defs.inc" 2>"$D/extract.log"
{
  python3 "$X" lines "$Q/http.cpp" '^static char +http_request_fetch_(dest|site)\['
  python3 "$X" func "$Q/http.cpp" http_request_is_embedded_subresource http_request_is_cross_origin http_request_denied_by_origin \
      http_get_string_param http_strvar_len_ok http_check_strvar_len http_legacy_set_update_source http_send_update_source_rejected \
      http_api_update_host_set http_api_update_path_set
} > "$D/c19_code.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" http_fs 'if (*action' > "$D/c19_fs.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" http_update 'if (*action)' > "$D/c19_update.inc" 2>>"$D/extract.log"
# Anzeige der Abweisung (fehlt im Ausgangsstand - dann leer, und der Pruefstand meldet "keine Abweisung")
python3 "$X" block "$Q/http.cpp" http_fs 'if (update_source_rejected)' > "$D/c19_fs_anzeige.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" http_update 'if (update_source_rejected)' > "$D/c19_update_anzeige.inc" 2>>"$D/extract.log"
for f in c19_defs c19_code c19_fs c19_update; do
  [ -s "$D/$f.inc" ] || { echo "AUSZUG LEER ($B): $f"; cat "$D/extract.log"; exit 2; }
done
grep -q '"saveuphost"' "$D/c19_fs.inc" && grep -q '"saveuphost"' "$D/c19_update.inc" || { echo "AUSZUG OHNE saveuphost ($B)"; exit 2; }
grep -q 'http_api_update_path_set (' "$D/c19_code.inc" || { echo "AUSZUG OHNE API-SETTER ($B)"; exit 2; }
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable \
   -I"$D" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/c19.cpp" -o "$D/c19.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/c19.bin"
