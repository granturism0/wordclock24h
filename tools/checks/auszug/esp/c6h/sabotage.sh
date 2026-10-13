#!/bin/sh
# Sabotagen fuer c6h: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <ESP-Quellverzeichnis>
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd); T=$(mktemp -d "${TMPDIR:-/tmp}/c6hsab.XXXXXX"); rc=0
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
s neue_aktion_ohne_wache '        else if (! strcmp (action, "rsteeprom"))\n' '        else if (! strcmp (action, "rstx"))\n        {\n            do_reset = 1;\n        }\n        else if (! strcmp (action, "rsteeprom"))\n'
s neue_aktion_mit_senke  '        else if (! strcmp (action, "rsteeprom"))\n' '        else if (! strcmp (action, "rstx"))\n        {\n            stm32_reset ();\n        }\n        else if (! strcmp (action, "rsteeprom"))\n'
s neuer_api_weg          '    ota_restart_pending = 0;                                        // normaler Weg' '    stm32_reset ();\n    ota_restart_pending = 0;                                        // normaler Weg'
s site_nicht_erfasst     '                http_capture_request_fetch_site (sHeaderLine);\n' '                ;\n'
s site_wirkt_nach        "    http_request_fetch_site[0] = '\\\\0';                      // ebenfalls" '    ;                      // ebenfalls'
s same_site_erlaubt      '! strcmp (http_request_fetch_site, "none"))' '! strcmp (http_request_fetch_site, "none") || ! strcmp (http_request_fetch_site, "same-site"))'
s fs_remove_ohne_wache   '    if (http_request_denied_by_origin (F("fs_remove"), 0))     // C6v/L312\n    {\n        return 0;\n    }\n' ''
s rsteeprom2_ohne_wache  '            if (http_request_denied_by_origin (F("legacy rsteeprom2"), 1))    // C60/L426\n            {\n                return 0;\n            }\n' ''
s local_reset_ohne_wache '            if (http_request_denied_by_origin (F("legacy local reset"), 1))   // C60\n            {\n                return 0;\n            }\n' ''
s fs_post_offen          '    if (*action && post == POST_ICON_NONE)' '    if (*action)'
s legacy_als_json        '            if (http_request_denied_by_origin (F("legacy fs format"), 1))' '            if (http_request_denied_by_origin (F("legacy fs format"), 0))'
s wache_ohne_return      '            if (http_request_denied_by_origin (F("legacy update reset"), 1))  // C60\n            {\n                return 0;\n            }\n' '            if (http_request_denied_by_origin (F("legacy update reset"), 1))  // C60\n            {\n                ;\n            }\n'
rm -rf "$T"; exit $rc
