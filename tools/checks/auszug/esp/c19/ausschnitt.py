#!/usr/bin/env python3
"""Zieht Stuecke aus http.cpp mit Klammerabgleich (Zeichenketten und Kommentare uebersprungen).

  ausschnitt.py func  <datei> <name> [...]       Funktion samt Rueckgabetyp-Zeile; fehlt sie: leer + Hinweis
  ausschnitt.py block <datei> <funktion> <anker>  ab der ersten Zeile in <funktion>, die (eingerueckt) mit
                                                  <anker> beginnt, bis zur passenden schliessenden Klammer
  ausschnitt.py lines <datei> <regex>             alle Zeilen, auf die <regex> passt
"""
import re, sys

def text(path):
    return open(path, 'rb').read().decode('utf-8').replace('\r\n', '\n')

def blank(s):
    """Kommentare und Zeichenketten durch Leerzeichen ersetzen, Laenge und Zeilen bleiben."""
    o = list(s); i = 0; n = len(s)
    while i < n:
        c = s[i]
        if s.startswith('//', i):
            j = s.find('\n', i); j = n if j < 0 else j
            for k in range(i, j): o[k] = ' '
            i = j
        elif s.startswith('/*', i):
            j = s.find('*/', i + 2); j = n if j < 0 else j + 2
            for k in range(i, j):
                if o[k] != '\n': o[k] = ' '
            i = j
        elif c in '"\'':
            j = i + 1
            while j < n and s[j] != c:
                j += 2 if s[j] == '\\' else 1
            for k in range(i + 1, min(j, n)): o[k] = ' '
            i = j + 1
        else:
            i += 1
    return ''.join(o)

def match(b, start):
    """Index nach der zu b[start]=='{' passenden Klammer."""
    d = 0
    for k in range(start, len(b)):
        if b[k] == '{': d += 1
        elif b[k] == '}':
            d -= 1
            if d == 0: return k + 1
    raise ValueError('Klammer offen')

def functions(s, b):
    """[(name, kopf_start, rumpf_start, ende)] fuer alle Funktionen in Spalte 0."""
    res = []
    for m in re.finditer(r'^\{', b, re.M):
        i = m.start()
        # Kopfzeile: letzte Zeile in Spalte 0 vor '{', die mit name ( beginnt
        vor = s[:i].split('\n')
        for k in range(len(vor) - 1, max(len(vor) - 6, -1), -1):
            mm = re.match(r'([A-Za-z_]\w*)\s*\(', vor[k])
            if mm:
                kopf = sum(len(x) + 1 for x in vor[:k - 1]) if k > 0 else 0
                res.append((mm.group(1), kopf, i, match(b, i)))
                break
    return res

def func(path, names):
    s = text(path); b = blank(s); out = []
    fs = {f[0]: f for f in functions(s, b)}
    for n in names:
        if n not in fs:
            sys.stderr.write(f'  (Funktion {n} fehlt in {path})\n'); continue
        _, k, _, e = fs[n]; out.append(s[k:e] + '\n')
    return ''.join(out)

def block(path, fname, anker):
    s = text(path); b = blank(s)
    fs = {f[0]: f for f in functions(s, b)}
    if fname not in fs:
        sys.stderr.write(f'  (Funktion {fname} fehlt)\n'); return ''
    _, k, r, e = fs[fname]
    pos = r
    for line in s[r:e].split('\n'):
        if line.strip().startswith(anker):
            i = s.index(line, pos)
            j = b.index('{', i)
            ls = s.rfind('\n', 0, i) + 1
            return s[ls:match(b, j)] + '\n'
        pos += len(line) + 1
    sys.stderr.write(f'  (Anker {anker!r} fehlt in {fname})\n'); return ''

if __name__ == '__main__':
    mode, path = sys.argv[1], sys.argv[2]
    if mode == 'func': sys.stdout.write(func(path, sys.argv[3:]))
    elif mode == 'block': sys.stdout.write(block(path, sys.argv[3], sys.argv[4]))
    elif mode == 'lines':
        r = re.compile(sys.argv[3])
        sys.stdout.write(''.join(l + '\n' for l in text(path).split('\n') if r.search(l)))
