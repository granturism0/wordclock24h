#!/usr/bin/env python3
"""Pruefstand c65t (Paket 2026-10-13, E3.5, AKR.7; design.md 18.6), statisch.
Aufruf: c65t.py <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 OK, 1 FEHL, 2 KAPUTT.

Zaehlt SELBST alle http_client.setTimeout () (Code ohne Kommentare) und alle ESPhttpUpdate.update (http_client, ...),
loest die Konstanten ueber #define auf und verlangt:
  1. in http_server_loop () nach "if (!http_client) ... return;" und VOR dem ersten Schreiben eine Grenze von 1000 ms;
  2. in http_send_fs_file () VOR dem ersten http_send () und vor der Leseschleife 2000 ms;
  3. vor JEDEM ESPhttpUpdate.update (http_client, ...) in derselben Funktion, nach dem letzten http_client.stop (),
     eine Grenze von mindestens 5000 ms, ohne weitere setTimeout () dazwischen.
"""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
p = os.path.join(q, 'http.cpp')
if not os.path.exists(p): print(f"KAPUTT c65t {bez}: http.cpp fehlt"); sys.exit(2)
s = open(p, 'rb').read().decode('utf-8').replace('\r\n', '\n')
s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
L = [re.sub(r'//.*', '', l) for l in s.split('\n')]
defs = {m.group(1): m.group(2) for l in L for m in [re.match(r'#define\s+(\w+)\s+(\d+)\w*\s*$', l.strip())] if m}
def wert(x):
    x = x.strip()
    return int(defs.get(x, x)) if re.fullmatch(r'\w+', x) and (x.isdigit() or x in defs) else None
def bounds(kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return None
    return i, next(k for k in range(i, len(L)) if L[k].startswith('}'))
st = [(k, wert(m.group(1))) for k, l in enumerate(L) for m in [re.search(r'\bhttp_client\.setTimeout\s*\(([^)]*)\)', l)] if m]
ota = [k for k, l in enumerate(L) if re.search(r'\bESPhttpUpdate\.update\s*\(\s*http_client\b', l)]
print(f"c65t ({bez}): {len(st)} x http_client.setTimeout (Zeilen {', '.join(str(k + 1) + '=' + str(v) for k, v in st)}), "
      f"{len(ota)} x ESPhttpUpdate.update (http_client, ...) (Zeilen {', '.join(str(k + 1) for k in ota)})")
fails = []
if len(ota) != 2: fails.append(f"erwartet 2 OTA-Aufrufe mit http_client, gefunden {len(ota)}")
# 1. nach accept
b = bounds('http_server_loop (void)')
if not b: fails.append("http_server_loop () fehlt")
else:
    a = next((k for k in range(*b) if re.search(r'if\s*\(\s*!\s*http_client\s*\)', L[k])), None)
    r = next((k for k in range(a, b[1]) if L[k].strip() == '}'), None) if a is not None else None
    w = next((k for k in range(r or b[0], b[1]) if re.search(r'\bhttp_(send|flush)\s*\(|http_client\.write\s*\(|\bhttp\s*\(|http_post\s*\(', L[k])), None) if r else None
    ok = [k for k, v in st if r is not None and r < k < (w or b[1]) and v == 1000]
    if not ok: fails.append("http_server_loop (): keine Grenze von 1000 ms nach dem Annehmen und vor dem ersten Schreiben")
# 2. Dateiauslieferung
b = bounds('http_send_fs_file (')
if not b: fails.append("http_send_fs_file () fehlt")
else:
    s1 = next((k for k in range(*b) if re.search(r'\bhttp_send\s*\(', L[k])), None)
    lp = next((k for k in range(*b) if re.search(r'while\s*\(.*fp\.available', L[k])), None)
    ok = [k for k, v in st if b[0] < k < min(x for x in (s1, lp, b[1]) if x is not None) and v == 2000]
    if not ok: fails.append("http_send_fs_file (): keine Grenze von 2000 ms vor den Kopfzeilen und der Leseschleife")
# 3. vor jedem OTA
for o in ota:
    fa = max(k for k in range(o) if L[k].startswith('{'))
    stop = max([k for k in range(fa, o) if re.search(r'http_client\.stop\s*\(', L[k])] or [fa])
    vor = [(k, v) for k, v in st if stop < k < o]
    if len(vor) != 1 or vor[0][1] is None or vor[0][1] < 5000:
        fails.append(f":{o + 1}: vor ESPhttpUpdate.update keine eigene Grenze >= 5000 ms nach dem letzten stop () ({vor})")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c65t {bez}: {len(fails)} Fehler (Pruefungen: 1 accept, 1 Datei, {len(ota)} OTA)")
sys.exit(1 if fails else 0)
