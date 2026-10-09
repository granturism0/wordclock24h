#!/bin/sh
# Statische Pruefung E.2b (AKD.1): Keine Serial-Ausgabe beginnt eine Zeile mit "CMD " oder "CMC "
# ausserhalb von stm_cmd_send () in ESP-uclock.ino. Aufruf: static.sh <verzeichnis ESP-uclock> <bezeichnung>
# Kommentarzeilen zaehlen nicht. Exit 1 bei Treffer, die Treffer werden genannt.
python3 - "$1" "$2" <<'PY'
import re, sys, glob
q, b = sys.argv[1], sys.argv[2]
treffer, absender, geprueft = [], 0, 0
for p in sorted(glob.glob(q + '/*.cpp') + glob.glob(q + '/*.ino') + glob.glob(q + '/*.h')):
    L = open(p, 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n').split('\n')
    fn = ''
    for i, l in enumerate(L):
        m = re.match(r'^([a-z_0-9]+)\s*\(', l)
        if m: fn = m.group(1)
        s = l.strip()
        if s.startswith('*') or s.startswith('//') or s.startswith('/*'): continue
        if re.search(r'Serial\.(print|println|printf|write)', l):
            geprueft += 1
            if re.search(r'"CM[DC] ', l):
                if fn == 'stm_cmd_send' and p.endswith('.ino'): absender += 1
                else: treffer.append('%s:%d  %s' % (p.split('/')[-1], i + 1, s))
if treffer:
    print('FEHL (%s): %d Kommandozeile(n) ausserhalb des Absenders (%d Serial-Aufrufe geprueft):' % (b, len(treffer), geprueft))
    for t in treffer: print('  ' + t)
    sys.exit(1)
if absender != 2:
    print('FEHL (%s): im Absender %d statt 2 Zeilenanfaenge (CMD/CMC) gefunden' % (b, absender)); sys.exit(1)
print('OK (%s): keine Kommandozeile ausserhalb des Absenders, %d Serial-Aufrufe geprueft, im Absender CMD und CMC' % (b, geprueft))
PY
