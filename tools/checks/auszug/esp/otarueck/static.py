#!/usr/bin/env python3
"""Teil 2 von otarueck: Verdrahtung des Rueckfall-Neustarts, statisch. Aufruf: static.py <ESP-Quellverzeichnis> <bez>"""
import re, sys
q, name = sys.argv[1], sys.argv[2]
def lines(f): return open(f'{q}/{f}', 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
def code(l):
    t = l.strip()
    return '' if t.startswith('*') or t.startswith('/*') else l.split('//')[0]
def func(L, name):
    i = next((k for k, l in enumerate(L) if re.match(re.escape(name) + r'\s*\(', l)), None)   # 'loop() ' ohne Leerzeichen
    if i is None: return None, None
    return i, next(k for k in range(i, len(L)) if L[k].startswith('}'))
H = lines('http.cpp'); fails = []
print(f"otarueck statisch ({name})")
sets = [k for k, l in enumerate(H) if re.search(r'\bota_restart_pending\s*=\s*1\s*;', code(l))]
if len(sets) != 1: fails.append(f'ota_restart_pending = 1 {len(sets)}x statt genau 1x')
a, b = func(H, 'http_api_local_esp_update')
if a is None: fails.append('http_api_local_esp_update () fehlt')
else:
    g = [k for k in range(a, b) if 'if (stream_ok && Update.end (true))' in code(H[k])]
    if len(g) != 1: fails.append('Erfolgsbedingung "if (stream_ok && Update.end (true))" nicht genau einmal')
    else:
        g = g[0]; e = next(k for k in range(g + 2, b) if H[k].strip() == '}')     # Ende des Erfolgszweigs
        ok = [k for k in sets if g < k < e]
        sn = [k for k in range(g, e) if re.search(r'\bota_restart_since\s*=\s*millis \(\)', code(H[k]))]
        print(f"  Vormerken im Erfolgszweig :{g + 1}..:{e + 1}: {'ja :' + str(ok[0] + 1) if ok else 'NEIN'}, Zeitpunkt {'ja' if sn else 'NEIN'}")
        if not ok: fails.append('Vormerkung nicht im Erfolgszweig nach Update.end (true)')
        if not sn: fails.append('ota_restart_since nicht im Erfolgszweig gesetzt')
a, b = func(H, 'http_api_local_esp_restart')
if a is None: fails.append('http_api_local_esp_restart () fehlt')
else:
    c = [k for k in range(a, b) if re.search(r'\bota_restart_pending\s*=\s*0\s*;', code(H[k]))]
    r = [k for k in range(a, b) if 'ESP.restart ()' in code(H[k])]
    print(f"  /api/local_esp_restart loescht vor ESP.restart (): {'ja' if c and r and c[0] < r[0] else 'NEIN'}")
    if not (c and r and c[0] < r[0]): fails.append('/api/local_esp_restart loescht die Vormerkung nicht vor ESP.restart ()')
I = lines('ESP-uclock.ino'); a, b = func(I, 'loop')
calls = [k for k in range(a, b) if re.search(r'^\s*http_ota_restart_poll \(\);', code(I[k]))] if a is not None else []
print(f"  Aufruf aus loop (): {len(calls)}x")
if len(calls) != 1: fails.append(f'http_ota_restart_poll () {len(calls)}x statt 1x in loop ()')
if not any('http_ota_restart_poll (void);' in code(l) for l in lines('http.h')): fails.append('Deklaration in http.h fehlt')
for f in fails: print('      FEHLER: ' + f)
sys.exit(1 if fails else 0)
