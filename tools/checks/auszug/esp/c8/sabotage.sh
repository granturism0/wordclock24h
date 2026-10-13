#!/bin/sh
# Sabotagen fuer c8: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <verzeichnis ESP-uclock>
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
sab puffer_128        ESP-uclock.ino '#define CMD_BUFFER_SIZE     260' '#define CMD_BUFFER_SIZE     128'
sab puffer_259        ESP-uclock.ino '#define CMD_BUFFER_SIZE     260' '#define CMD_BUFFER_SIZE     259'
sab ring_groesser     ESP-uclock.ino '#define STM32_LOG_LINE_LEN  120' '#define STM32_LOG_LINE_LEN  255'
sab ohne_endmarke     ESP-uclock.ino '            dst[STM32_LOG_LINE_LEN - 1] = LOG_TRUNC_MARK;' '            dst[STM32_LOG_LINE_LEN - 1] = line[STM32_LOG_LINE_LEN - 1];'
sab ohne_anfangsmarke ESP-uclock.ino '            dst[0] = LOG_TRUNC_MARK;' '            dst[0] = line[pos++]; n = (n > 0) ? n - 1 : 0;'
sab nur_ein_eintrag   ESP-uclock.ino '    } while (pos < len);' '    } while (0);'
sab grenze_119        ESP-uclock.ino '        if (pos == 0 && len <= STM32_LOG_LINE_LEN)' '        if (pos == 0 && len < STM32_LOG_LINE_LEN)'
sab folge_120         ESP-uclock.ino '            if (n > STM32_LOG_LINE_LEN - 1)\n            {\n                n = STM32_LOG_LINE_LEN - 1;' '            if (n > STM32_LOG_LINE_LEN - 2)\n            {\n                n = STM32_LOG_LINE_LEN - 2;'
rm -rf "$T"; exit $rc
