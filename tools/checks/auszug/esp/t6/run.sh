#!/bin/sh
# Pruefstand F.5i (C23 / L207). Aufruf: run.sh <verzeichnis mit http.cpp> <bezeichnung>
# /api/stm32_log liefert gueltiges JSON in UTF-8: ISO-8859-1-Umlaut, Anfuehrungszeichen,
# Rueckstrich, Steuerzeichen -- dazu eine schon UTF-8-kodierte Zeile, die NICHT doppelt
# kodiert werden darf. Die Antwort wird zweimal geprueft: im Pruefstand gegen den erwarteten
# Wortlaut, danach mit Python streng als UTF-8 dekodiert und als JSON gelesen.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
{ python3 "$S/extract.py" range "$Q/http.cpp" "#define HTTP_ESCAPE_XML" "static void             http_send_escaped"
  python3 "$S/extract.py" func "$Q/http.cpp" utf8_sequence_len http_escape_text http_send_escaped http_send_json_escaped http_api_stm32_log
} > "$D/c23_code.inc" 2>"$D/extract.log"
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/t6.cpp" -o "$D/t6.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t6.bin" "$D/antwort.json"; rc=$?
python3 - "$D/antwort.json" <<'PY' || rc=1
import json, sys
raw = open(sys.argv[1], 'rb').read()
try:
    txt = raw.decode('utf-8')                       # streng: ein rohes ISO-8859-1-Byte scheitert hier
    obj = json.loads(txt)                           # streng: rohe Steuerzeichen scheitern hier
except Exception as e:
    print(f"  [FEHL] Antwort ist kein gueltiges JSON in UTF-8: {type(e).__name__}: {e}")
    sys.exit(1)
want = ["Temperatur 21\u00b0C gem\u00e4ss F\u00fchler", 'Ticker "Hallo"', "Pfad C:\\wc", "Steuer\u0001zeichen\u001f", "Tab\tCR\rLF\n", "schon UTF-8: \u00e4\u00f6\u00fc", "~\u007f"]
if obj.get("lines") != want:
    print(f"  [FEHL] gelesene Zeilen weichen ab: {obj.get('lines')!r}")
    sys.exit(1)
print("  [ ok ] Python liest die Antwort streng als UTF-8 und JSON, alle Zeilen wortgleich")
PY
exit $rc
