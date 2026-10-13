#!/bin/sh
# Sabotagen fuer c46: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <ESP-Quellverzeichnis>
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd); T=$(mktemp -d "${TMPDIR:-/tmp}/c46sab.XXXXXX"); rc=0
s () {
    rm -rf "$T/q"; mkdir -p "$T/q"; cp "$Q/http.cpp" "$Q/vars.h" "$T/q/"
    python3 - "$T/q/http.cpp" "$2" "$3" <<'P' || { echo "  ANKER FEHLT: $1"; rc=1; return; }
import sys; p = sys.argv[1]; b = open(p, 'rb').read()
a = sys.argv[2].encode().decode('unicode_escape').encode(); r = sys.argv[3].encode().decode('unicode_escape').encode()
assert b.count(a) == 1, b.count(a)
open(p, 'wb').write(b.replace(a, r))
P
    r=$(sh "$D/run.sh" "$T/q" "sab-$1" 2>&1 | tail -1)
    case "$r" in FEHL*) echo "  schlaegt an: $1";; *) echo "  BLIEB GRUEN: $1 ($r)"; rc=1;; esac
}
s grenze_255_abgewiesen  '    if (query_len <= HTTP_API_MAX_QUERY_LEN ||' '    if (query_len < HTTP_API_MAX_QUERY_LEN ||'
s kennung_8              '#define HTTP_API_ERROR_QUERY_TOO_LONG               9' '#define HTTP_API_ERROR_QUERY_TOO_LONG               8'
s praefix_ohne_schraegstrich 'strncmp (path, "/api/", 5))' 'strncmp (path, "/api", 4))'
s post_laeuft_trotzdem   '        if (! query_rejected)\n        {\n            http_post (sPath);\n        }\n' '        http_post (sPath);\n'
s get_ohne_pruefung      '    if (http_api_query_too_long (path, strlen (const_param)))     // C46: abweisen statt still kuerzen\n    {\n        return 0;\n    }\n' ''
s ohne_logzeile          '    Serial.println (F(" Zeichen"));\n' ''
rm -rf "$T"; exit $rc
