#!/bin/sh
# Pruefstand ovr (Paket 2026-10-11, E.3, AKO.1): Serial.hasOverrun() saettigend gezaehlt, als ovr= an der Heap-Zeile.
# Aufruf: run.sh <verzeichnis mit ESP-uclock.ino> <bezeichnung>
# Echter Code: die Zaehlstelle aus loop() (Block "if (Serial.hasOverrun ()" bis vor "while (Serial.available())"),
# die Zaehlervariable und esp_heap_log(). Fehlt die Zaehlstelle (Ausgangsstand), bleibt sie leer => FEHL, nicht KAPUTT.
D=$(cd "$(dirname "$0")" && pwd); Q=$1; B=$2
python3 - "$Q/ESP-uclock.ino" "$D" <<'P' || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; exit 2; }
import sys, re
s = open(sys.argv[1], 'rb').read().decode('utf-8').replace('\r\n', '\n'); L = s.split('\n'); d = sys.argv[2]
def grab(start, end_pred):
    st = start if callable(start) else (lambda l, s0=start: l.startswith(s0))
    i = next((k for k, l in enumerate(L) if st(l)), None)
    if i is None: return ''
    j = next(k for k in range(i + 1, len(L)) if end_pred(L[k]))
    return '\n'.join(L[i:j]) + '\n'
var = next((l for l in L if l.startswith('static uint32_t       esp_uart_overruns')), '')
heap = grab('esp_heap_log (void)', lambda l: l.startswith('}'))
heap = 'static void\n' + heap + '}\n' if heap else ''
poll = grab(lambda l: l.startswith('    if (') and 'Serial.hasOverrun ()' in l, lambda l: l.startswith('    while (Serial.available())') or l.startswith('    stm_bridge_poll ();'))
open(d + '/ovr_var.inc', 'w').write((var if var else '#define OVR_FEHLT 1') + '\n')
open(d + '/ovr_heap.inc', 'w').write(heap.replace('static unsigned long ', 'static uint32_t '))
open(d + '/ovr_poll.inc', 'w').write(poll)
if not heap: sys.exit('esp_heap_log fehlt')
P
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -Wno-format -I"$D" -DBEZEICHNUNG="\"$B\"" \
   "$D/ovr.cpp" -o "$D/ovr.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/ovr.bin"
