#!/usr/bin/env python3
"""Zieht Quelltextstuecke aus einer Firmwaredatei (neu oder alt), damit die Pruefstaende den
ECHTEN Code uebersetzen und nicht eine Abschrift.

  extract.py func  <datei> <name> [...]      Funktion(en) ab Kopfzeile "name (" bis "}" in Spalte 0,
                                             samt Rueckgabetyp-Zeile davor; fehlt sie, leer + Hinweis
  extract.py range <datei> <start> <ende>    ab erster Zeile, die mit <start> beginnt, bis vor die
                                             erste danach, die mit <ende> beginnt
  extract.py block <datei> <kopf> <folgekopf>  wie range, aber die Muster duerfen eingerueckt sein
"""
import sys
def lines(path):
    return open(path, 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n').split('\n')
def func(path, name):
    L = lines(path)
    for i, l in enumerate(L):
        if l.startswith(name + ' (') or l.startswith(name + '('):
            j = i
            while not L[j].startswith('}'):
                j += 1
            return '\n'.join(L[i - 1:j + 1]) + '\n'
    sys.stderr.write(f"  (Funktion {name} fehlt in {path})\n")
    return ''
def rng(path, start, end, strip=False):
    L = lines(path)
    key = (lambda s: s.strip()) if strip else (lambda s: s)
    i = next(k for k, l in enumerate(L) if key(l).startswith(start))
    j = next(k for k in range(i + 1, len(L)) if key(L[k]).startswith(end))
    return '\n'.join(L[i:j]) + '\n'
mode, path = sys.argv[1], sys.argv[2]
if mode == 'func':
    sys.stdout.write(''.join(func(path, n) for n in sys.argv[3:]))
elif mode == 'range':
    sys.stdout.write(rng(path, sys.argv[3], sys.argv[4]))
elif mode == 'block':
    sys.stdout.write(rng(path, sys.argv[3], sys.argv[4], strip=True))
