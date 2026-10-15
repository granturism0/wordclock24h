#!/bin/sh
# Sabotagen fuer ovr: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <verzeichnis mit ESP-uclock.ino>
D=$(cd "$(dirname "$0")" && pwd); Q=$1; T=$(mktemp -d); rc=0
sab () {
    name=$1; mkdir -p "$T/$name"; cp "$Q/ESP-uclock.ino" "$T/$name/"
    python3 - "$T/$name/ESP-uclock.ino" "$2" "$3" <<'P' || { echo "  ANKER FEHLT: $name"; rc=1; return; }
import sys; p = sys.argv[1]; s = open(p, 'rb').read()
a = sys.argv[2].encode(); b = sys.argv[3].encode()
assert s.count(a) == 1, (a, s.count(a))
open(p, 'wb').write(s.replace(a, b))
P
    r=$("$D/run.sh" "$T/$name" "sab-$name" | tail -1)
    case "$r" in FEHL*) echo "  schlaegt an: $name ($r)";; *) echo "  BLIEB GRUEN: $name ($r)"; rc=1;; esac
}
sab reihenfolge     'if (Serial.hasOverrun () && esp_uart_overruns < 0xFFFFFFFFUL)' 'if (esp_uart_overruns < 0xFFFFFFFFUL && Serial.hasOverrun ())'
sab nicht_saettigend 'if (Serial.hasOverrun () && esp_uart_overruns < 0xFFFFFFFFUL)' 'if (Serial.hasOverrun ())'
sab ohne_ovr_feld   'PSTR ("- heap free=%lu max=%lu ovr=%lu low=%lu %s nrt=%u"),' 'PSTR ("- heap free=%lu max=%lu ovx=%lu low=%lu %s nrt=%u"),'   # R1.1: Feld umbenannt statt entfernt (sonst liest %s eine Zahl)
sab puffer_zu_klein '        char            line[120];' '        char            line[48];'   # R1.1: 120 statt 80
sab nie_gezaehlt    '        esp_uart_overruns++;' '        ;'
rm -rf "$T"; exit $rc
