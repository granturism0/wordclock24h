#!/usr/bin/env python3
"""c65a statisch (AKR.1): genau vier Aufrufe von heap_low_note (HEAP_LOW_PATH_*) an den vier Orten aus design.md 18.2.
Aufruf: c65a_static.py <ESP-Quellverzeichnis> [bezeichnung]. Zaehlt die Aufrufe SELBST ueber beide Dateien (Code, ohne
Kommentare) und prueft je Pfad Funktion und Lage. Exit 0 OK, 1 FEHL."""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
def code_lines(name):
    p = os.path.join(q, name)
    if not os.path.exists(p): return []
    s = open(p, 'rb').read().decode('utf-8').replace('\r\n', '\n')
    s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
    return [re.sub(r'//.*', '', l) for l in s.split('\n')]
def bounds(L, kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return None
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return i, j
fails = []
calls = []
for f in ('http.cpp', 'httpclient.cpp'):
    L = code_lines(f)
    for k, l in enumerate(L):
        for m in re.finditer(r'\bheap_low_note\s*\(\s*(HEAP_LOW_PATH_\w+)\s*\)', l):
            calls.append((f, k, m.group(1)))
print(f"c65a statisch ({bez}): {len(calls)} Aufrufe von heap_low_note gefunden: " + ', '.join(f"{c[2]} {c[0]}:{c[1] + 1}" for c in calls))
if len(calls) != 4: fails.append(f"erwartet genau 4 Aufrufe, gefunden {len(calls)}")
erw = {'HEAP_LOW_PATH_FLUSH': ('http.cpp', 'http_flush (void)'), 'HEAP_LOW_PATH_FS': ('http.cpp', 'http_send_fs_file ('),
       'HEAP_LOW_PATH_ACCEPT': ('http.cpp', 'http_server_loop (void)'), 'HEAP_LOW_PATH_CLIENT': ('httpclient.cpp', 'httpclient (const char')}
for pfad, (datei, kopf) in erw.items():
    treffer = [c for c in calls if c[2] == pfad]
    if len(treffer) != 1:
        fails.append(f"{pfad}: {len(treffer)} Aufrufe statt 1"); continue
    f, k, _ = treffer[0]
    L = code_lines(datei); b = bounds(L, kopf)
    if f != datei or not b or not (b[0] < k < b[1]):
        fails.append(f"{pfad}: nicht in {kopf.split(' ')[0]} ({f}:{k + 1})"); continue
    body = L[b[0]:b[1]]; rel = k - b[0]
    def first(rx, start=0):
        return next((i for i in range(start, len(body)) if re.search(rx, body[i])), None)
    if pfad == 'HEAP_LOW_PATH_FLUSH':
        w = first(r'http_client\.write\s*\(')
        if w is None or not rel < w: fails.append(f"{pfad}: nicht vor dem ersten http_client.write ()")
    elif pfad == 'HEAP_LOW_PATH_FS':
        lp = first(r'while\s*\(.*fp\.available'); w = first(r'http_client\.write\s*\(')
        if lp is None or w is None or not (lp < rel < w): fails.append(f"{pfad}: nicht in der Leseschleife vor dem write ()")
    elif pfad == 'HEAP_LOW_PATH_ACCEPT':
        a = first(r'if\s*\(\s*!\s*http_client\s*\)')
        ret = first(r'^\s*}\s*$', a) if a is not None else None
        nxt = [i for i in range(ret + 1, rel) if body[i].strip()] if ret is not None else None
        w = first(r'http_(send|flush|client\.write)\s*\(')
        if a is None or ret is None or rel < ret or (nxt and len(nxt) > 1) or (w is not None and w < rel):
            fails.append(f"{pfad}: nicht unmittelbar nach if (!http_client) return;")
    elif pfad == 'HEAP_LOW_PATH_CLIENT':
        c = first(r'client\.connect\s*\('); pr = first(r'client\.print\s*\(')
        if c is None or pr is None or not (c < rel < pr): fails.append(f"{pfad}: nicht nach connect () und vor dem Senden")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c65a statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
