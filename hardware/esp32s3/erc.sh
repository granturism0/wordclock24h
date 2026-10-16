#!/bin/sh
# ERC des V3-Schaltplans; Ergebnis nach erc.json, Zusammenfassung auf stdout
cd "$(dirname "$0")"
/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli sch erc --severity-all --format json -o erc.json WordClock-ESP32S3-V3.kicad_sch 2>&1 | grep -v Fontconfig
python3 - <<'PY'
import json, collections
d = json.load(open('erc.json')); vs = [v for s in d['sheets'] for v in s['violations']]
print(collections.Counter((v['type'], v['severity']) for v in vs))
for v in vs:
    if v['type'] != 'pin_to_pin': print(v['type'], [i['description'] for i in v['items']])
PY
