#!/bin/sh
# Pruefstand c46 (Paket 2026-10-13, E1.3; AKQ.1). Aufruf: run.sh <verzeichnis mit http.cpp und vars.h> <bezeichnung>
# Exit 0 OK, 1 FEHL, 2 Auszug oder Uebersetzung gescheitert.
D=$(cd "$(dirname "$0")" && pwd)
# ohne Argument: der Arbeitsbaum, in dem der Aufruf steht
ROOT=${ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null || git rev-parse --show-toplevel 2>/dev/null)}
Q=$(cd "${1:-$ROOT/ESP8266/ESP-uclock}" 2>/dev/null && pwd) || { echo "Verzeichnis fehlt: ${1:-$ROOT/ESP8266/ESP-uclock}"; exit 2; }; B=${2:-neu}
X="$D/ausschnitt.py"
python3 "$X" lines "$Q/http.cpp" '^#define (HTTP_API_ERROR_[A-Z_]+|HTTP_API_MAX_QUERY_LEN|HTTP_ESCAPE_[A-Z_]+)\s' > "$D/c46_defs.inc" 2>"$D/extract.log"
python3 "$X" func "$Q/http.cpp" utf8_sequence_len http_escape_text http_send_escaped http_send_json_escaped http_json_error \
    http_api_query_too_long > "$D/c46_code.inc" 2>>"$D/extract.log"
# Kopf von http (): ab der Oeffnungsklammer bis vor die Zerlegung ("char *          p;")
python3 - "$Q/http.cpp" > "$D/c46_http.inc" 2>>"$D/extract.log" <<'PY'
import sys
L = open(sys.argv[1], 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
i = L.index('http (const char * path, const char * const_param)')
j = next(k for k in range(i, len(L)) if L[k].strip().startswith('char *          p;'))
print('\n'.join(L[i + 2:j]))
PY
python3 "$X" block "$Q/http.cpp" http_server_loop 'if (start_position == 0)' > "$D/c46_post.inc" 2>>"$D/extract.log"
for f in c46_defs c46_code c46_http c46_post; do
  [ -s "$D/$f.inc" ] || { echo "AUSZUG LEER ($B): $f"; cat "$D/extract.log"; exit 2; }
done
grep -q 'strncpy (param, const_param, 255);' "$D/c46_http.inc" || { echo "AUSZUG http () OHNE strncpy ($B)"; exit 2; }
grep -q 'http_post (sPath);' "$D/c46_post.inc" || { echo "AUSZUG POST OHNE http_post ($B)"; exit 2; }
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable \
   -I"$D" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/c46.cpp" -o "$D/c46.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/c46.bin"
