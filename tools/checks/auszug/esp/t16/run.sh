#!/bin/sh
# Pruefstand C47 + Ent-11 (Paket 2026-10-10, E.1). Aufruf: run.sh <verzeichnis mit http.cpp, base.cpp> <bezeichnung>
#
# Uebersetzt den ECHTEN Code aus http.cpp: input_field (), select_field (), den Release-Notes-Block aus
# http_update () (von "len = httpclient (... RELEASENOTE_HTML)" bis vor "len = httpclient (... WC_TXT)"),
# die DS18xx-Anzeige aus http_temperature () und die Parameterkette des Legacy-Formulars ('+' -> ' ',
# http_set_params (), normalize_http_parameters (), http_get_param (), htoi () aus base.cpp).
# http_send_attr () und der Release-Notes-Filter fehlen im Ausgangsstand - dann bleiben die Auszuege
# leer, und der Pruefstand laeuft trotzdem: Die Bloecke, die es in BEIDEN Staenden gibt, rufen sie nur
# im neuen. Damit schlaegt die Gegenprobe gegen den Ausgangsstand als FEHL an, nicht als KAPUTT.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
X="$S/extract.py"; [ -f "$X" ] || X="$(git -C "$D" rev-parse --show-toplevel 2>/dev/null)/tools/checks/auszug/esp/extract.py"
{
  python3 "$X" func "$Q/base.cpp" htoi
  python3 "$X" range "$Q/http.cpp" "#define MAX_HTTP_PARAMS" "static int "                  # Grenze, HTTP_PARAMETERS, Feld
  python3 "$X" func "$Q/http.cpp" normalize_http_parameters http_set_params http_get_param
} > "$D/c47_params.inc" 2>"$D/extract.log"
python3 "$X" func "$Q/http.cpp" http_send_attr > "$D/c47_attr.inc" 2>>"$D/extract.log"
python3 "$X" func "$Q/http.cpp" input_field select_field > "$D/c47_fields.inc" 2>>"$D/extract.log"
# '+' -> ' ' aus http () - der Schritt vor http_set_params () bei GET
python3 "$X" block "$Q/http.cpp" "for (p = param; *p; p++)" "// log_printf (\"http parameters" > "$D/c47_plus.inc" 2>>"$D/extract.log"
# Release-Notes-Filter (nur neu): vom Kommentarkopf bis vor http_update ()
python3 - "$Q/http.cpp" > "$D/c47_notesfilter.inc" <<'PY'
import sys
L = open(sys.argv[1], 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
k = [i for i, l in enumerate(L) if l.startswith(' * Release Notes - C47')]
if k:
    a = max(i for i in range(k[0]) if L[i].startswith('/*---'))
    e = next(i for i in range(k[0], len(L)) if L[i].startswith('http_update (void)'))
    print('\n'.join(L[a:e - 1]))
PY
python3 "$X" block "$Q/http.cpp" "len = httpclient (update_host, update_path, RELEASENOTE_HTML);" "len = httpclient (update_host, update_path, WC_TXT);" \
  > "$D/c47_notes.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" "uint_fast8_t ds18xx_is_up = get_numvar (DS18XX_IS_UP_NUM_VAR);" "rtc_temperature_correction = http_decode_temp_correction" \
  > "$D/c47_ds.inc" 2>>"$D/extract.log"
# Ent-11 fuer die RTC: Anzeigeblock aus http_temperature () samt A5-Hilfsfunktionen (in beiden Staenden vorhanden)
{ python3 "$X" range "$Q/http.cpp" "#define RTC_TEMP_HALF_DEG_UNKNOWN" "/* Anzuzeigende"
  python3 "$X" func "$Q/http.cpp" http_rtc_temp_half_deg http_format_half_deg
} > "$D/c47_rtcfn.inc" 2>>"$D/extract.log"
python3 "$X" block "$Q/http.cpp" "if (rtc_is_up)" "uint_fast8_t ds18xx_is_up = get_numvar (DS18XX_IS_UP_NUM_VAR);" > "$D/c47_rtc.inc" 2>>"$D/extract.log"
for f in c47_rtcfn c47_rtc c47_params c47_fields c47_plus c47_notes c47_ds; do
  [ -s "$D/$f.inc" ] || { echo "AUSZUG LEER ($B): $f"; cat "$D/extract.log"; exit 2; }
done
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-deprecated-declarations -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/c47.cpp" -o "$D/c47.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/c47.bin"
