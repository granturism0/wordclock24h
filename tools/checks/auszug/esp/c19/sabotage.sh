#!/bin/sh
# Sabotagen fuer c19: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <ESP-Quellverzeichnis>
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd); T=$(mktemp -d "${TMPDIR:-/tmp}/c19sab.XXXXXX"); rc=0
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
s update_seite_alt       '        else if (! strcmp (action, "saveuphost"))\n        {\n            if (! http_legacy_set_update_source (UPDATE_HOST_VAR, "uphost", MAX_UPDATE_HOST_LEN))\n            {\n                update_source_rejected = 1;\n            }\n        }\n        else if (! strcmp (action, "saveuppath"))\n        {\n            if (! http_legacy_set_update_source (UPDATE_PATH_VAR, "uppath", MAX_UPDATE_PATH_LEN))\n            {\n                update_source_rejected = 1;\n            }\n        }\n        else if (! strcmp (action, "flash"))' '        else if (! strcmp (action, "saveuphost"))\n        {\n            set_strvar (UPDATE_HOST_VAR, http_get_param ("uphost"));\n        }\n        else if (! strcmp (action, "saveuppath"))\n        {\n            set_strvar (UPDATE_PATH_VAR, http_get_param ("uppath"));\n        }\n        else if (! strcmp (action, "flash"))'
s leer_erlaubt           '    if (! http_get_string_param (name, &value) || ! http_strvar_len_ok (value, maxlen))' '    value = http_get_param (name);\n    if (! http_strvar_len_ok (value, maxlen))'
s grenze_64              '    if (! http_get_string_param (name, &value) || ! http_strvar_len_ok (value, maxlen))' '    if (! http_get_string_param (name, &value) || ! http_strvar_len_ok (value, maxlen + 1))'
s ohne_anzeige_update    '    begin_box ("Update");\n\n    if (update_source_rejected)\n    {\n        http_send_update_source_rejected ();\n    }\n' '    begin_box ("Update");\n'
s ohne_anzeige_fs        '    message_tables_file_missing ();\n\n    if (update_source_rejected)\n    {\n        http_send_update_source_rejected ();\n    }\n' '    message_tables_file_missing ();\n'
s merker_nicht_gesetzt   '            if (! http_legacy_set_update_source (UPDATE_PATH_VAR, "uppath", MAX_UPDATE_PATH_LEN))\n            {\n                update_source_rejected = 1;\n            }\n        }\n        else if (! strcmp (action, "savedwnfil"))' '            if (! http_legacy_set_update_source (UPDATE_PATH_VAR, "uppath", MAX_UPDATE_PATH_LEN))\n            {\n                ;\n            }\n        }\n        else if (! strcmp (action, "savedwnfil"))'
rm -rf "$T"; exit $rc
