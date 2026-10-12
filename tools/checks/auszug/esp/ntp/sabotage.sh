#!/bin/sh
# Sabotagen fuer ntp: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <verzeichnis mit ntp.cpp>
D=$(cd "$(dirname "$0")" && pwd); Q=$1; T=$(mktemp -d); rc=0
sab () {
    name=$1; rm -rf "$T/$name"; mkdir -p "$T/$name"; cp "$Q/ntp.cpp" "$T/$name/"
    python3 - "$T/$name/ntp.cpp" "$2" "$3" <<'P' || { echo "  ANKER FEHLT: $name"; rc=1; return; }
import sys; p = sys.argv[1]
s = open(p, 'rb').read().decode().replace('\r\n', '\n')
a = sys.argv[2].encode().decode('unicode_escape'); b = sys.argv[3].encode().decode('unicode_escape')
assert s.count(a) == 1, (a, s.count(a))
open(p, 'wb').write(s.replace(a, b).replace('\n', '\r\n').encode())
P
    r=$("$D/run.sh" "$T/$name" "sab-$name" | tail -1)
    case "$r" in FEHL*) echo "  schlaegt an: $name ($r)";; *) echo "  BLIEB GRUEN: $name ($r)"; rc=1;; esac
}
sab alte_reihenfolge  'if (packetsize != NTP_PACKET_SIZE)                                  // L391: NICHT lesen, Rest verwirft parsePacket()\n        {\n            ntp_reject ("laenge", packetsize);\n            return;\n        }\n\n        nread = ntp_udp.read(ntp_packet_buffer, NTP_PACKET_SIZE);' 'nread = ntp_udp.read(ntp_packet_buffer, packetsize);\n        if (packetsize != NTP_PACKET_SIZE)\n        {\n            ntp_reject ("laenge", packetsize);\n            return;\n        }'
# gleichwertig und deshalb KEINE Sabotage: read(..., packetsize) allein -- nach der vorgezogenen Pruefung ist packetsize dort immer 48
sab laenge_danach      'if (packetsize != NTP_PACKET_SIZE)                                  // L391' 'if (0)                                  // L391'
sab flush_statt_nichts '            ntp_reject ("laenge", packetsize);\n            return;' '            ntp_reject ("laenge", packetsize);\n            ntp_udp.flush ();\n            return;'
sab ohne_li            'if ((ntp_packet_buffer[0] >> 6) == 3)' 'if (0)'
sab ohne_stratum       'if (ntp_packet_buffer[1] == 0)' 'if (0)'
sab ohne_transmit      'if (secsSince1900 == 0)' 'if (0)'
sab ohne_absender      'if (! ntp_server_ip.isSet () || ntp_udp.remoteIP () != ntp_server_ip)' 'if (0)'
sab ohne_isset         'if (! ntp_server_ip.isSet () || ntp_udp.remoteIP () != ntp_server_ip)' 'if (ntp_udp.remoteIP () != ntp_server_ip)'
sab ohne_port          'if (ntp_udp.remotePort () != NTP_PORT)' 'if (0)'
sab server_nicht_gemerkt '    ntp_server_ip = address;' '    (void) address;'
sab keine_drossel      'if (! ntp_reject_logged || (millis () - ntp_reject_last) >= NTP_REJECT_LOG_INTERVAL)' 'if (1)'
sab ohne_mitschnitt    '        Serial.println (line);\n        Serial.flush ();\n        stm32_log_append (line);' '        stm32_log_append (line);'
sab ohne_ring          '        Serial.flush ();\n        stm32_log_append (line);' '        Serial.flush ();'
sab pivot_im_esp       '        Serial.print ("TIME ");\n        Serial.println(secsSince1900);' '        if (secsSince1900 < 0x80000000UL) secsSince1900 += 0;\n        Serial.print ("TIME ");\n        Serial.println(secsSince1900 < 0x80000000UL ? secsSince1900 + 1 : secsSince1900);'
rm -rf "$T"; exit $rc
