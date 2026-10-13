#!/bin/sh
# Statische Pruefung a79 (Paket 2026-10-13, E2.3, AKE.1): In setup () steht Serial.setRxBufferSize (1024) genau einmal
# und VOR Serial.begin, sonst nirgends; zwischen dem Kopf von setup () und der CAP-Zeile steht keine Ausgabe ausser
# der bisherigen Leerzeile Serial.println (""). Aufruf: static.sh <verzeichnis ESP-uclock> <bezeichnung>  Exit 0/1/2
Q=$1; B=${2:-$1}
python3 - "$Q/ESP-uclock.ino" "$B" <<'P2'
import sys, re
p, b = sys.argv[1], sys.argv[2]
try:
    L = open(p, 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
except Exception as e:
    print('KAPUTT a79 %s: %s' % (b, e)); sys.exit(2)
code = lambda l: re.sub(r'//.*$', '', l)
s = next((k for k, l in enumerate(L) if l.startswith('setup()')), None)
c = next((k for k in range(s or 0, len(L)) if 'Serial.println ("CAP var-crc");' in L[k]), None)
if s is None or c is None:
    print('KAPUTT a79 %s: setup () oder CAP-Zeile nicht gefunden' % b); sys.exit(2)
fehler = []
# Kommentare /* */ ausblenden
body, im = [], False
for k in range(s, c):
    l = L[k]
    if im:
        if '*/' in l: im = False
        continue
    if l.strip().startswith('/*'):
        im = '*/' not in l
        continue
    body.append((k + 1, code(l)))
rx = [n for n, l in body if 'setRxBufferSize' in l]
bg = [n for n, l in body if re.search(r'Serial\.begin\s*\(', l)]
alle_rx = [k + 1 for k, l in enumerate(L) if re.search(r'Serial\.setRxBufferSize\s*\(', code(l))]
if len(rx) != 1: fehler.append('setRxBufferSize in setup () vor CAP: %d statt 1' % len(rx))
elif not re.search(r'Serial\.setRxBufferSize\s*\(\s*1024\s*\)\s*;', dict(body)[rx[0]]): fehler.append('Groesse nicht 1024: ' + dict(body)[rx[0]].strip())
if len(bg) != 1: fehler.append('Serial.begin vor CAP: %d statt 1' % len(bg))
if rx and bg and not rx[0] < bg[0]: fehler.append('setRxBufferSize (Zeile %d) steht NACH Serial.begin (Zeile %d)' % (rx[0], bg[0]))
if len(alle_rx) != 1: fehler.append('setRxBufferSize in der Datei %d-mal (Zeilen %s), erwartet genau einmal' % (len(alle_rx), alle_rx))
aus = [(n, l.strip()) for n, l in body if re.search(r'Serial\.(print|println|printf|write)\s*\(', l)]
if [a for _, a in aus] != ['Serial.println ("");']: fehler.append('Ausgaben vor CAP: %s (erwartet nur die bisherige Leerzeile)' % aus)
for f in fehler: print('      FEHLER: ' + f)
print('%s  a79 %s: %d Fehler (geprueft: setup () Zeilen %d-%d, %d Ausgaben, %d Codezeilen)' % ('FEHL' if fehler else 'OK  ', b, len(fehler), s + 1, c + 1, len(aus), len(body)))
sys.exit(1 if fehler else 0)
P2
