#!/usr/bin/env python3
"""Statische Pruefung AKK.2 (Paket 2026-10-11, E.4, A67 ESP-Haelfte).
Aufruf: static.py <http.cpp> [bezeichnung]

Zaehlt die OTA-Pfade SELBST (jede Zeile mit "ESPhttpUpdate.update (" oder "Update.begin ("), statt eine Zahl
vorauszusetzen, und verlangt fuer jeden:
  1. in derselben Funktion VOR dem Aufruf ein http_ota_announce ();
  2. in derselben Funktion NACH dem Aufruf ein http_ota_aborted ();
  3. http_ota_announce() sendet woertlich "OTA start" (eigene Zeile, println), http_ota_aborted() "SYNCVARS".
Ausserdem: genau 3 Pfade erwartet (design.md 7) -- ein vierter faellt so auf, statt still durchzurutschen.
"""
import re, sys
path = sys.argv[1]; name = sys.argv[2] if len(sys.argv) > 2 else path
L = open(path, 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
fails = []
def func_bounds(i):
    a = i
    while a > 0 and not (L[a].startswith('{') and re.match(r'^[A-Za-z_]\w* \(', L[a - 1])):
        a -= 1
    b = i
    while b < len(L) and not L[b].startswith('}'):
        b += 1
    return a - 1, b
def code(l):
    t = l.strip()
    if t.startswith('*') or t.startswith('/*'):          # Blockkommentar
        return ''
    return l.split('//')[0]
calls = [i for i, l in enumerate(L) for _ in re.findall(r'\bESPhttpUpdate\.update\s*\(|\bUpdate\.begin\s*\(', code(l))]   # je TREFFER, nicht je Zeile
print(f"akk2 ({name}): {len(calls)} OTA-Aufrufstellen gefunden")
if len(calls) != 3:
    fails.append(f"erwartet 3 OTA-Pfade, gefunden {len(calls)}")
for i in calls:
    a, b = func_bounds(i)
    fn = L[a].strip()
    before = [k for k in range(a, i) if re.search(r'\bhttp_ota_announce \(\);', code(L[k]))]
    after  = [k for k in range(i, b) if re.search(r'\bhttp_ota_aborted \(\);', code(L[k]))]
    st = 'OK  ' if before and after else 'FEHL'
    print(f"  {st} :{i + 1} in {fn}: announce {'davor :' + str(before[-1] + 1) if before else 'FEHLT'}, "
          f"aborted {'danach :' + str(after[0] + 1) if after else 'FEHLT'}")
    if not before: fails.append(f":{i + 1} {fn}: kein http_ota_announce () davor")
    if not after:  fails.append(f":{i + 1} {fn}: kein http_ota_aborted () danach")
def body(fname):
    try:
        i = next(k for k, l in enumerate(L) if l.startswith(fname + ' ('))
    except StopIteration:
        return None
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return '\n'.join(L[i:j])
ba = body('http_ota_announce'); bb = body('http_ota_aborted')
# Wortlaut auf der Leitung: println mit dem Literal, direkt oder ueber F() (Flash statt DRAM, Review E.6) -- beides
# erzeugt dieselben Bytes. print ohne ln, ein anderes Literal oder ein zusammengesetzter Text faellt durch.
def sends(body, text):
    return body is not None and re.search(r'Serial\.println \((F\()?"' + re.escape(text) + r'"\)?\);', body) is not None
if not sends(ba, 'OTA start'): fails.append('http_ota_announce () sendet nicht woertlich "OTA start" per println')
if not sends(bb, 'SYNCVARS'):  fails.append('http_ota_aborted () sendet nicht "SYNCVARS" per println')
lit = [i + 1 for i, l in enumerate(L) if '"OTA start' in code(l)]
if len(lit) != 1: fails.append(f'Zeichenkette "OTA start" {len(lit)}x statt genau 1x (nur im Helfer): Zeilen {lit}')
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  akk2 {name}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
