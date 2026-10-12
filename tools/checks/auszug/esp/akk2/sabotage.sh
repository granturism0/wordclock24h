#!/bin/sh
# Sabotagen fuer die statische Pruefung AKK.2: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <http.cpp>
D=$(cd "$(dirname "$0")" && pwd); Q=$1; T=$(mktemp -d); rc=0
s () {
    cp "$Q" "$T/h.cpp"
    python3 - "$T/h.cpp" "$2" "$3" <<'P' || { echo "  ANKER FEHLT: $1"; rc=1; return; }
import sys; p = sys.argv[1]; b = open(p, 'rb').read()
a = sys.argv[2].encode().decode('unicode_escape').encode(); r = sys.argv[3].encode().decode('unicode_escape').encode()
assert b.count(a) >= 1
open(p, 'wb').write(b.replace(a, r, 1))
P
    r=$(python3 "$D/static.py" "$T/h.cpp" "sab-$1" | tail -1)
    case "$r" in FEHL*) echo "  schlaegt an: $1 ($r)";; *) echo "  BLIEB GRUEN: $1 ($r)"; rc=1;; esac
}
s ohne_announce_legacy '                http_ota_announce ();' '                ;'
s ohne_aborted_api     '    http_ota_aborted ();                                            // A67: update() kehrt' '    ;                                            // A67: update() kehrt'
s ohne_aborted_lokal   '            http_ota_aborted ();                                    // ueber' '            ;                                    // ueber'
s ohne_announce_lokal  '        http_ota_announce ();                                       // A67: STM offline, VOR dem Lesen des Rumpfs' '        ;'
s wortlaut             'Serial.println (F("OTA start"));' 'Serial.println (F("OTA-start"));'
s wortlaut_print       'Serial.println (F("OTA start"));' 'Serial.print (F("OTA start"));'
s wortlaut_ohne_F      'Serial.println (F("OTA start"));' 'Serial.println ("OTA start ");'
s syncvars             'Serial.println (F("SYNCVARS"));' 'Serial.println (F("SYNC"));'
s syncvars_print       'Serial.println (F("SYNCVARS"));' 'Serial.print (F("SYNCVARS"));'
s vierter_pfad_zeile   '    t_httpUpdate_return ret = ESPhttpUpdate.update (http_client, update_host, 80, path);' '    t_httpUpdate_return ret = ESPhttpUpdate.update (http_client, update_host, 80, path); ESPhttpUpdate.update (http_client, update_host, 80, path);'
s vierter_pfad_eng     '        if (Update.begin (max_sketch_space, U_FLASH))' '        Update.begin(1, U_FLASH);\n        if (Update.begin (max_sketch_space, U_FLASH))'
rm -rf "$T"; exit $rc
