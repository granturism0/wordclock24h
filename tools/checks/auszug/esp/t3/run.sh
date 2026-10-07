#!/bin/sh
# Pruefstand S.4b (A5). Aufruf: run.sh <verzeichnis mit http.cpp, vars.cpp, vars.h> <bezeichnung>
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
{ python3 "$S/extract.py" func "$Q/vars.cpp" vars_init
  python3 "$S/extract.py" func "$Q/http.cpp" http_decode_temp_correction http_encode_temp_correction http_clamp_temp_correction
  python3 "$S/extract.py" range "$Q/http.cpp" "#define RTC_TEMP_HALF_DEG_UNKNOWN" "/* Anzuzeigende" 2>/dev/null
  python3 "$S/extract.py" func "$Q/http.cpp" http_rtc_temp_half_deg http_format_half_deg http_rtc_temp_half_deg_correct
  python3 "$S/extract.py" func "$Q/http.cpp" http_api_temperature_correction_set
} > "$D/a5_code.inc" 2>"$D/extract.log"
python3 - "$Q/http.cpp" > "$D/rtc_block.inc" <<'PY'
import sys
L = open(sys.argv[1], 'rb').read().decode('utf-8').split('\n')
f = next(i for i, l in enumerate(L) if l.startswith('http_temperature (void)'))
a = next(i for i in range(f, len(L)) if L[i].strip() == 'if (rtc_is_up)')
b = next(i for i in range(a, len(L)) if L[i].strip().startswith('uint_fast8_t ds18xx_is_up'))
print('\n'.join(L[a:b]))
PY
cc -x c++ -std=c++17 -Wall -Wno-unused-function -I"$D" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/t3.cpp" -o "$D/t3.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -10; exit 2; }
"$D/t3.bin"; rc=$?
# Statisch: Name -> Index in vars.h, und beide Korrekturstellen ueber DIESELBE Hilfsfunktion
python3 - "$Q" <<'PY' || rc=1
import sys, re
q = sys.argv[1]; bad = 0
t = open(q + '/vars.h', 'rb').read().decode('latin-1')
i = t.index('MAX_NUM_VARIABLES '); j = t.rindex('{', 0, i)
names = [x.strip() for x in re.sub(r'//[^\n]*', '', t[j + 1:i]).split(',') if x.strip()]
k = names.index('RTC_TEMP_HALF_DEG_NUM_VAR') if 'RTC_TEMP_HALF_DEG_NUM_VAR' in names else -1
print(f"  [{' ok ' if k == 49 and len(names) == 50 else 'FEHL'}] vars.h: RTC_TEMP_HALF_DEG_NUM_VAR = {k}, letzter vor MAX_NUM_VARIABLES ({len(names)})")
bad += not (k == 49 and len(names) == 50)
L = open(q + '/http.cpp', 'rb').read().decode('utf-8').split('\n')
def owner(n):
    for m in range(n, -1, -1):
        if re.match(r'^[a-z_]+ \(', L[m]): return L[m].split(' (')[0]
calls = [owner(n) for n, l in enumerate(L) if 'http_rtc_temp_half_deg_correct (' in l and not l.startswith('http_rtc_temp_half_deg_correct')]
ok = sorted(calls) == ['http_api_temperature_correction_set', 'http_temperature']
print(f"  [{' ok ' if ok else 'FEHL'}] Nachrechnen ueber dieselbe Hilfsfunktion in: {', '.join(calls) or 'keiner Stelle'}")
bad += not ok
sys.exit(1 if bad else 0)
PY
exit $rc
