#!/bin/sh
# Pruefstand ovrzeit (L408): Ueberlaufabfrage VOR der Heap-Zeile, Zeile "- ovr #<n> t=<millis>" gedrosselt.
# Aufruf: run.sh <verzeichnis mit ESP-uclock.ino> [bezeichnung]   Exit 0 = OK, 1 = FEHL, 2 = KAPUTT
# Echter Code: esp_uart_overruns, esp_heap_log() und der loop()-Abschnitt von "ntp_poll_time ();" bis vor
# "while (Serial.available())" -- also auch die REIHENFOLGE, nicht nur die Bausteine.
# Arbeitsdateien in einem Wegwerfverzeichnis, nichts bleibt im Pruefstandsordner liegen.
D=$(cd "$(dirname "$0")" && pwd); Q=$1; B=${2:-$1}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
python3 - "$Q/ESP-uclock.ino" "$T" <<'P' || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; exit 2; }
import sys
s = open(sys.argv[1], 'rb').read().decode('ascii').replace('\r\n', '\n'); L = s.split('\n'); t = sys.argv[2]
def idx(pred, start=0):
    return next((k for k in range(start, len(L)) if pred(L[k])), None)
var = next((l for l in L if l.startswith('static uint32_t       esp_uart_overruns')), None)
h = idx(lambda l: l.startswith('esp_heap_log (void)'))
if var is None or h is None: sys.exit('Zaehler oder esp_heap_log fehlt')
he = idx(lambda l: l.startswith('}'), h + 1)
heap = 'static void\n' + '\n'.join(L[h:he + 1]) + '\n'
lp = idx(lambda l: l.startswith('loop()'))
a = idx(lambda l: 'ntp_poll_time ();' in l, lp)
b = idx(lambda l: l.startswith('    while (Serial.available())') or l.startswith('    stm_bridge_poll ();'), a)
if lp is None or a is None or b is None: sys.exit('loop()-Abschnitt nicht gefunden')
open(t + '/z_var.inc', 'w').write(var + '\n')
# unsigned long ist auf dem ESP 32 Bit, auf dem Pruefrechner 64 -- sonst waere der millis()-Umlauf nicht nachgestellt
u = lambda x: x.replace('static unsigned long ', 'static uint32_t ')
open(t + '/z_heap.inc', 'w').write(u(heap))
open(t + '/z_loop.inc', 'w').write(u('\n'.join(L[a + 1:b]) + '\n'))
P
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -Wno-format -I"$T" -DBEZEICHNUNG="\"$B\"" \
   "$D/ovrzeit.cpp" -o "$T/ovrzeit.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -15; exit 2; }
"$T/ovrzeit.bin"
