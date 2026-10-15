#!/usr/bin/env python3
"""Auszug fuer den Pruefstand c71i (Paket 2026-10-15, R1.3, I-6). Aufruf: auszug_c71.py <ESP-Quellverzeichnis> <ziel> <c65-Verzeichnis> <Stand 3.2.35>

Ruft den Auszug von c65 zweimal (neu_ aus dem Stand, alt_ aus 3.2.35 - der Harness upd_harness.h verlangt beide
Namensraeume, c71i benutzt nur neu), dann schneidet er aus http.cpp http_accept_note () samt seinen zwei Variablen
(ea.inc) und aus ESP-uclock.ino den Block mit ESP.getFlashChipId () in setup () (flash.inc). Fehlt ein Baustein
(Stand vor R1.3), fehlt sein C71_HAVE_-Makro in c71have.inc: Der Pruefstand meldet FEHL, nicht KAPUTT.
"""
import os, re, subprocess, sys
q, ziel, c65, alt = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
for quelle, px in ((q, 'neu_'), (alt, 'alt_')):
    r = subprocess.run([sys.executable, os.path.join(c65, 'auszug.py'), quelle, ziel, px], capture_output=True, text=True)
    if r.returncode:
        sys.exit('c65-Auszug: ' + (r.stdout + r.stderr).strip())
def text(name):
    return open(os.path.join(q, name), 'rb').read().decode('utf-8').replace('\r\n', '\n')
H, I = text('http.cpp'), text('ESP-uclock.ino')
L = H.split('\n')
have = []
ea = ''.join(l + '\n' for l in L if re.match(r'^static (uint16_t|uint8_t) +(http_accept_count|http_first_request_logged)\b', l))
i = next((k for k, l in enumerate(L) if l.startswith('http_accept_note (void)')), None)
if i is not None:
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    ea += '\n'.join(L[i - 1:j + 1]) + '\n'
    have.append('C71_HAVE_EA')
open(os.path.join(ziel, 'ea.inc'), 'w').write(ea)
IL = I.split('\n')
f = next((k for k, l in enumerate(IL) if 'ESP.getFlashChipId' in l), None)
fl = ''
if f is not None:
    a = max(k for k in range(f) if IL[k] == '    {')
    e = next(k for k in range(f, len(IL)) if IL[k] == '    }')
    fl = '\n'.join(IL[a:e + 1]) + '\n'
    have.append('C71_HAVE_FLASH')
open(os.path.join(ziel, 'flash.inc'), 'w').write(fl)
if 'update_cache_vorher (void)' in H:
    have.append('C71_HAVE_I6')
open(os.path.join(ziel, 'c71have.inc'), 'w').write(''.join(f'#define {m} 1\n' for m in have))
