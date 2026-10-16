#!/usr/bin/env python3
"""c71e statisch (AKS.1; design.md 7). Aufruf: c71e_static.py <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 OK, 1 FEHL.
  S1  httpclient_stop (): client.stop (), danach client = WiFiClient () (L7a)
  S2  httpclient.cpp: kein client.stop () ausserhalb von httpclient_stop () (L7b)
  S3  httpclient (): vor JEDEM "return -1;" steht httpclient_stop (); ein Zweig "if (len <= 0)" schliesst ueber
      httpclient_stop () und gibt len zurueck (L7c = C75)
  S4  update_cache_write (): Zweig zu-gross schliesst ueber httpclient_stop () vor dem return
  S5  Aufruferliste: jeder Aufruf von httpclient () und von update_server_open () in http.cpp und stm32flash.cpp; die
      umgebende Funktion enthaelt mindestens so viele httpclient_stop () wie Abrufe (Ausnahme: der Durchreicher
      update_server_open selbst, der len unveraendert zurueckgibt). Jeder Aufrufer schliesst nur im Zweig len > 0
      selbst; ein fehlendes Schliessen dort liesse den Kontext bis zum naechsten Abruf stehen. Gezaehlt und mit dem Stand von R1b verglichen (16 Aufrufstellen) -
      eine neue Aufrufstelle laesst die Pruefung anschlagen, bis sie hier eingeordnet ist.
"""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
def code(name):
    p = os.path.join(q, name)
    if not os.path.exists(p): return []
    s = open(p, 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n')
    s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
    s = re.sub(r"'(?:\\.|[^'\\\n])'", "''", s)                # Zeichenliterale zuerst ('"' oeffnete sonst eine Zeichenkette)
    s = re.sub(r'"(?:\\.|[^"\\\n])*"', '""', s)              # Zeichenketten enden in ihrer Zeile - sonst verschwinden Zeilen
    return [re.sub(r'//.*', '', l) for l in s.split('\n')]
def body(L, kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return []
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return L[i:j + 1]
fails = []; n = 0
def chk(name, ok):
    global n; n += 1
    if not ok: fails.append(name)
C, H = code('httpclient.cpp'), code('http.cpp')
st = [l.strip() for l in body(C, 'httpclient_stop (void)') if l.strip()]
i1 = next((k for k, l in enumerate(st) if re.match(r'client\.stop\s*\(\s*\)\s*;', l)), None)
i2 = next((k for k, l in enumerate(st) if re.match(r'client\s*=\s*WiFiClient\s*\(\s*\)\s*;', l)), None)
chk('S1 httpclient_stop (): client.stop (), danach client = WiFiClient ()', i1 is not None and i2 is not None and i1 < i2)
sb = body(C, 'httpclient_stop (void)')
aussen = [k + 1 for k, l in enumerate(C) if re.search(r'\bclient\.stop\s*\(', l) and l not in sb]
chk(f'S2 kein client.stop () ausserhalb von httpclient_stop () (Zeilen {aussen})', not aussen)
hb = body(C, 'httpclient (const char')
code_zeilen = [l.strip() for l in hb if l.strip()]
rets = [k for k, l in enumerate(code_zeilen) if re.match(r'return\s+-1\s*;', l)]
ohne = [k for k in rets if not re.match(r'httpclient_stop\s*\(\s*\)\s*;', code_zeilen[k - 1])]
chk(f'S3 vor jedem "return -1;" in httpclient () steht httpclient_stop () ({len(rets)} Rueckwege, ohne: {len(ohne)})', rets and not ohne)
lz = next((k for k, l in enumerate(code_zeilen) if re.match(r'if\s*\(\s*len\s*<=\s*0\s*\)', l)), None)
chk('S3 Zweig "if (len <= 0)": httpclient_stop (); return len;', lz is not None and code_zeilen[lz + 1] == '{' and
    re.match(r'httpclient_stop\s*\(\s*\)\s*;', code_zeilen[lz + 2]) and re.match(r'return\s+len\s*;', code_zeilen[lz + 3]))
uw = body(H, 'update_cache_write (')
zg = next((k for k, l in enumerate(uw) if re.search(r'if\s*\(\s*\(size_t\)\s*len\s*>\s*max_len\s*\)', l)), None)
zb = []
if zg is not None:
    e = next(k for k in range(zg + 1, len(uw)) if re.match(r'^    }\s*$', uw[k]))
    zb = [l.strip() for l in uw[zg + 1:e] if l.strip() and l.strip() != '{']
ist = next((k for k, l in enumerate(zb) if re.match(r'httpclient_stop\s*\(\s*\)\s*;', l)), None)
ret = next((k for k, l in enumerate(zb) if l.startswith('return')), None)
chk('S4 update_cache_write (): zu-gross schliesst ueber httpclient_stop () vor dem return', None not in (ist, ret) and ist < ret)
def funktion(L, k):
    i = max((j for j in range(k + 1) if re.match(r'^[a-z_][a-z0-9_]* \(', L[j])), default=None)
    if i is None: return None, []
    e = next(j for j in range(i, len(L)) if L[j].startswith('}'))
    return L[i].split(' (')[0], L[i:e + 1]
aufrufe = []
for datei in ('http.cpp', 'stm32flash.cpp'):
    L = code(datei)
    for k, l in enumerate(L):
        for m in re.finditer(r'\b(httpclient|update_server_open)\s*\(', l):
            if re.match(r'^[a-z_][a-z0-9_]* \(', l) or re.match(r'^\s*(static\s+)?(extern\s+)?int\s+(httpclient|update_server_open)\s*\(', l): continue
            name, b = funktion(L, k)
            stops = sum(len(re.findall(r'\bhttpclient_stop\s*\(\s*\)', x)) for x in b)
            rufe  = sum(len(re.findall(r'\b(?:httpclient|update_server_open)\s*\(', x)) for x in b[1:])
            aufrufe.append((datei, k + 1, m.group(1), name, stops >= rufe, stops, rufe))
print(f"c71e statisch ({bez}): {len(aufrufe)} Aufrufstellen von httpclient () bzw. update_server_open ():")
for d, z, was, name, zu, st, ru in aufrufe:
    print(f"      {d}:{z}  {was} () in {name} ()  -> {st} x httpclient_stop () fuer {ru} Abruf(e){'' if zu else ('  (Durchreicher: gibt len zurueck, schliessen die Aufrufer)' if name == 'update_server_open' else '  ZU WENIG')}")
fehlend = [a for a in aufrufe if not a[4] and a[3] != 'update_server_open']
chk(f'S5 jede Aufrufstelle in einer Funktion mit httpclient_stop () (ohne: {[(a[0], a[1]) for a in fehlend]})', aufrufe and not fehlend)
chk(f'S5 Zahl der Aufrufstellen wie in R1b eingeordnet: 16 (gefunden {len(aufrufe)})', len(aufrufe) == 16)
roh = open(os.path.join(q, 'http.cpp'), 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n').count('\n')
chk(f'S0 Gegenstand vollstaendig: http.cpp nach dem Entfernen von Kommentaren und Zeichenketten gleich lang ({len(H) - 1} von {roh} Zeilen)', len(H) - 1 == roh)
print(f"c71e statisch ({bez}): {n} Pruefungen")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c71e statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
