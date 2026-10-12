#!/bin/sh
# Pruefstand ntp (Paket 2026-10-11, E.2, Ent-13, L391/C58): NTP-Antwort -- Laenge vor dem Lesen, Waechterbereich hinter
# dem Puffer, Absender/Port, Plausibilitaet (LI 3, Stratum 0, Transmit 0), keine Aerenrechnung, Drosselung.
# Aufruf: run.sh <verzeichnis mit ntp.cpp> <bezeichnung>
# Echter Code: ntp.cpp ohne #include-Zeilen. Angleichung: der Puffer liegt in einer Struktur VOR einem 2-KiB-Waechter
# (sonst entscheidet der Compiler, was hinter dem Puffer liegt); "static unsigned long" -> uint32_t (L256).
D=$(cd "$(dirname "$0")" && pwd); Q=$1; B=$2
R=${ROOT:-$(git -C "$Q" rev-parse --show-toplevel 2>/dev/null)}
python3 - "$Q/ntp.cpp" "$D/ntp_code.inc" <<'P' || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; exit 2; }
import sys, re
s = open(sys.argv[1], 'rb').read().decode('utf-8').replace('\r\n', '\n')
s = '\n'.join(l for l in s.split('\n') if not l.startswith('#include'))
a = 'byte            ntp_packet_buffer[NTP_PACKET_SIZE];'
assert s.count(a) == 1, 'Pufferdefinition nicht gefunden'
s = s.replace(a, 'struct NtpMem { byte buf[NTP_PACKET_SIZE]; byte guard[2048]; } ntp_mem;\n#define ntp_packet_buffer ntp_mem.buf')
s = re.sub(r'^static unsigned long ', 'static uint32_t ', s, flags=re.M)
open(sys.argv[2], 'w').write(s)
P
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -I"$R/tools/checks" -DBEZEICHNUNG="\"$B\"" \
   "$D/ntp.cpp" -o "$D/ntp.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/ntp.bin"
