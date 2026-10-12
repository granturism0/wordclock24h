#!/bin/sh
# Pruefstand A67 (Paket 2026-10-11, S3.10): "OTA start" setzt nur offline.
# Aufruf: run.sh <src-Verzeichnis> [sabotage]   Sabotagen: capflag praefix ohneoffline
# Baut auf tools/checks/auszug/stm/ep auf (extract.py, ep.c). Zwischendateien unter $TMPDIR.
# Exit 0 = OK, 1 = FEHL, 2 = kaputt.
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
D=$(cd "$(dirname "$0")" && pwd); SRC=$1; SAB=${2:-}
EP=$ROOT/tools/checks/auszug/stm/ep
[ -f "$EP/extract.py" ] && [ -f "$EP/ep.c" ] || { echo "a67: Pruefstand ep fehlt ($EP)"; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/a67.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
mkdir -p "$W/src" && cp -R "$SRC/esp8266" "$SRC/vars" "$SRC/weather" "$W/src/" && cp "$SRC/main.c" "$W/src/" || exit 2
if [ -n "$SAB" ]; then
  python3 - "$W/src/esp8266/esp8266.c" "$SAB" <<'PYEOF' || exit 2
import sys
p, sab = sys.argv[1], sys.argv[2]
b = open(p, 'rb').read().decode('latin-1')
a = '                        else if (! strcmp (answer, "OTA start"))\r\n'
SAB = {
  'capflag':     ('                            esp8266.is_online = 0;\r\n                            rtc = ESP8266_STATUS;\r\n                            break;\r\n                        }\r\n                        else if (! strncmp (answer, "OK", 2))',
                  '                            esp8266.is_online = 0;\r\n                            esp8266_cap_var_crc_seen = 1;\r\n                            rtc = ESP8266_STATUS;\r\n                            break;\r\n                        }\r\n                        else if (! strncmp (answer, "OK", 2))'),
  'praefix':     (a, '                        else if (! strncmp (answer, "OTA star", 8))\r\n'),
  'ohneoffline': ('                            esp8266.is_online = 0;\r\n                            rtc = ESP8266_STATUS;\r\n                            break;\r\n                        }\r\n                        else if (! strncmp (answer, "OK", 2))',
                  '                            rtc = ESP8266_STATUS;\r\n                            break;\r\n                        }\r\n                        else if (! strncmp (answer, "OK", 2))'),
}
if sab not in SAB or b.count(SAB[sab][0]) != 1: sys.exit('a67: Sabotage %s -- Anker %d-mal' % (sab, b.count(SAB.get(sab, ('#',))[0])))
open(p, 'wb').write(b.replace(*SAB[sab]).encode('latin-1'))
PYEOF
fi
S=$W/src
grep -q var_weather_query_end "$S/vars/vars.c" 2>/dev/null && HAVE_A2=-DHAVE_A2 || HAVE_A2=
python3 "$EP/extract.py" "$S" "$W/x-ep.c" "$ROOT/ESP8266/ESP-uclock" > /dev/null || exit 2
rc=0
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN"
  cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -Wno-tautological-constant-out-of-range-compare $Z $HAVE_A2 \
     -I "$S/esp8266" -I "$S/vars" -I "$S/weather" -I "$ROOT/tools/checks" -DSNIPPET="\"$W/x-ep.c\"" -DEP_C="\"$EP/ep.c\"" "$D/a67.c" -o "$W/a67" || exit 2
  echo "=== A67, $m${SAB:+, Sabotage $SAB} ==="; "$W/a67" || rc=1
done
exit $rc
