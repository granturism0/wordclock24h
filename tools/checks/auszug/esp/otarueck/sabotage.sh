#!/bin/sh
# Sabotagen fuer otarueck: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <ESP-Quellverzeichnis>
D=$(cd "$(dirname "$0")" && pwd); Q=$1; T=$(mktemp -d); rc=0
s () {
    rm -rf "$T/q"; mkdir -p "$T/q"; cp "$Q/http.cpp" "$Q/http.h" "$Q/ESP-uclock.ino" "$T/q/"
    python3 - "$T/q/$2" "$3" "$4" <<'P' || { echo "  ANKER FEHLT: $1"; rc=1; return; }
import sys; p = sys.argv[1]; b = open(p, 'rb').read()
a = sys.argv[2].encode().decode('unicode_escape').encode(); r = sys.argv[3].encode().decode('unicode_escape').encode()
assert b.count(a) == 1, b.count(a)
open(p, 'wb').write(b.replace(a, r))
P
    r=$("$D/run.sh" "$T/q" "sab-$1" | tail -1)
    case "$r" in FEHL*) echo "  schlaegt an: $1";; *) echo "  BLIEB GRUEN: $1 ($r)"; rc=1;; esac
}
s frist_groesser    http.cpp '(millis () - ota_restart_since) >= OTA_RESTART_FALLBACK_MSEC' '(millis () - ota_restart_since) > OTA_RESTART_FALLBACK_MSEC'
s frist_5s          http.cpp '#define OTA_RESTART_FALLBACK_MSEC   10000UL' '#define OTA_RESTART_FALLBACK_MSEC   5000UL'
s nicht_geloescht   http.cpp '        ota_restart_pending = 0;\n        Serial.println (F("- ota: Rueckfall-Neustart"));' '        Serial.println (F("- ota: Rueckfall-Neustart"));'
s ohne_vormerkung   http.cpp 'if (ota_restart_pending && (millis ()' 'if ((millis ()'
s ohne_zeile        http.cpp '        Serial.println (F("- ota: Rueckfall-Neustart"));\n' ''
s endpunkt_loescht_nicht http.cpp '    ota_restart_pending = 0;                                        // normaler Weg' '    ;                                        // normaler Weg'
s vormerken_fehlt   http.cpp '                ota_restart_pending = 1;' '                ;'
s zeitpunkt_fehlt   http.cpp '                ota_restart_since   = millis ();' '                ;'
s nicht_in_loop     ESP-uclock.ino '    http_ota_restart_poll ();' '    ;'
s ohne_deklaration  http.h 'extern void                 http_ota_restart_poll (void);' ''
s im_fehlerzweig    http.cpp '                ota_restart_pending = 1;                                // Rueckfall-Neustart vormerken (Review E.6)\n                ota_restart_since   = millis ();\n                ok = 1;' '                ok = 1;'
s im_fehlerzweig_2  http.cpp '                error_code = Update.getError ();\n                Update.end ();\n            }' '                error_code = Update.getError ();\n                Update.end ();\n                ota_restart_pending = 1;\n                ota_restart_since   = millis ();\n            }'
rm -rf "$T"; exit $rc
