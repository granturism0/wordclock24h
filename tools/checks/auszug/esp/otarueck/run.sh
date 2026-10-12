#!/bin/sh
# Pruefstand otarueck (Paket 2026-10-11, Review E.6 Punkt 2): Rueckfall-Neustart nach erfolgreichem lokalem ESP-Upload.
# Aufruf: run.sh <ESP-Quellverzeichnis> <bezeichnung>
# Teil 1 dynamisch, ECHTER Code: von "#define OTA_RESTART_FALLBACK_MSEC" bis zum Ende von http_ota_restart_poll ()
#   (Frist 10 s, genau ein Neustart, nie ohne Vormerkung, Wickeln der Uhr; "static unsigned long" -> uint32_t, L256).
# Teil 2 statisch: Verdrahtung -- Vormerken NUR im Erfolgszweig nach Update.end (true), Loeschen in
#   /api/local_esp_restart vor ESP.restart (), Aufruf aus loop (), Deklaration in http.h.
#   Statisch statt Attrappe, weil die Handler an WiFiClient und Update haengen; die Verdrahtung sind vier Zeilen.
D=$(cd "$(dirname "$0")" && pwd); Q=$1; B=$2
python3 - "$Q/http.cpp" "$D/rueck_code.inc" <<'P'
import sys, re
L = open(sys.argv[1], 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
i = next((k for k, l in enumerate(L) if l.startswith('#define OTA_RESTART_FALLBACK_MSEC')), None)
out = '#define RUECK_FEHLT 1\n'
if i is not None:
    f = next(k for k in range(i, len(L)) if L[k].startswith('http_ota_restart_poll ('))
    e = next(k for k in range(f, len(L)) if L[k].startswith('}'))
    out = re.sub(r'^static unsigned long ', 'static uint32_t ', '\n'.join(L[i:e + 1]), flags=re.M) + '\n'
open(sys.argv[2], 'w').write(out)
P
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/rueck.cpp" -o "$D/rueck.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/rueck.bin"; r1=$?
python3 "$D/static.py" "$Q" "$B"; r2=$?
[ $r1 -eq 0 ] && [ $r2 -eq 0 ] && { echo "OK    otarueck $B"; exit 0; }
echo "FEHL  otarueck $B"; exit 1
