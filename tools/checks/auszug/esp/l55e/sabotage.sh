#!/bin/sh
# Sabotagen fuer l55e: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <verzeichnis ESP-uclock>
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; T=$(mktemp -d); rc=0
export ROOT=${ROOT:-$(git -C "$Q" rev-parse --show-toplevel)}
sab () {
    name=$1; datei=$2; rm -rf "$T/$name"; mkdir -p "$T/$name"; cp "$Q"/*.ino "$Q"/*.cpp "$Q"/*.h "$T/$name/"
    python3 - "$T/$name/$datei" "$3" "$4" <<'PY' || { echo "  ANKER FEHLT: $name"; rc=1; return; }
import sys; p = sys.argv[1]
s = open(p, 'rb').read().decode('utf-8').replace('\r\n', '\n')
a = sys.argv[2].encode().decode('unicode_escape'); b = sys.argv[3].encode().decode('unicode_escape')
assert s.count(a) == 1, (a, s.count(a))
open(p, 'wb').write(s.replace(a, b).encode('utf-8'))
PY
    r=$("$D/run.sh" "$T/$name" "sab-$name" | tail -1)
    case "$r" in FEHL*) echo "  schlaegt an: $name ($r)";; *) echo "  BLIEB GRUEN: $name ($r)"; rc=1;; esac
}
sab ohne_lesen_beim_warten http.cpp '    for (;;)\n    {\n        stm_bridge_poll ();' '    for (;;)\n    {\n        ;'
sab echo_auf_rpcr          ESP-uclock.ino '                    stm_rpcr_seq[htoi (p, 2)]++;' '                    stm_rpcr_seq[htoi (p, 2)]++;\n                    Serial.println (".");'
sab ohne_sperre            ESP-uclock.ino '    if (busy)\n    {\n        return;\n    }' '    if (0)\n    {\n        return;\n    }'
sab sperre_bleibt_zu       ESP-uclock.ino '    busy = 0;   ' '    busy = 1;   '
sab frist_20s              http.cpp '#define HTTP_RPC_WAIT_MSEC                      2000UL' '#define HTTP_RPC_WAIT_MSEC                      20000UL'
sab frist_200ms            http.cpp '#define HTTP_RPC_WAIT_MSEC                      2000UL' '#define HTTP_RPC_WAIT_MSEC                      200UL'
sab keine_drossel          http.cpp 'if (! http_rpc_noanswer_logged || (millis () - http_rpc_noanswer_last) >= HTTP_RPC_NOANSWER_LOG_INTERVAL)' 'if (1)'
sab ohne_ring              http.cpp '        Serial.flush ();\n        stm32_log_append (line);\n    }\n}\n\nstatic int\nhttp_api_ldr_rpc_set' '        Serial.flush ();\n    }\n}\n\nstatic int\nhttp_api_ldr_rpc_set'
sab ohne_laengenpruefung   ESP-uclock.ino 'if (strlen (cmd_buffer) == 9 && isxdigit' 'if (isxdigit'
sab ohne_indexgrenze       ESP-uclock.ino '                    && htoi (p, 2) < STM_RPCR_SLOTS)' '                    && htoi (p, 2) < 255)'
sab abgewiesen_als_7       http.cpp '    else if (result == 1)' '    else if (result == 9)'
sab nicht_ueberlaufsicher  http.cpp 'if ((millis () - start) >= HTTP_RPC_WAIT_MSEC)' 'if (millis () >= start + HTTP_RPC_WAIT_MSEC)'
sab wert_vor_antwort       http.cpp '    seq = stm_rpcr_get (idx, (uint_fast8_t *) 0);' '    seq = stm_rpcr_get (idx, (uint_fast8_t *) 0) + 1;'
sab verstuemmelt_ungedrosselt ESP-uclock.ino 'if (! bad_logged || (millis () - bad_last) >= 60000UL)' 'if (1)'
sab verstuemmelt_nur_ring    ESP-uclock.ino '                        Serial.println (line);                              // "- "' '                        ;                              // "- "'
sab verstuemmelt_nur_mitschnitt ESP-uclock.ino '                        stm32_log_append (line);                            // Mitschnitt UND' '                        ;                            // Mitschnitt UND'
rm -rf "$T"; exit $rc
