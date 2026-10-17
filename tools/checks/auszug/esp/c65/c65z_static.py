#!/usr/bin/env python3
"""c65z statisch (AKV.10, Ent-32, Ent-36): genau zwei Aufrufe von tcp_tw_limit () - in http_server_loop () unmittelbar
nach if (!http_client) return; und VOR heap_low_note (HEAP_LOW_PATH_ACCEPT), in httpclient () VOR client.connect ();
HTTP_TW_LIMIT 2 ueber das Core-Makro TCP_TW_LIMIT; der private lwIP-Kopf lwip/priv/tcp_priv.h in genau einer Datei
(http.cpp). Zaehlt selbst ueber ALLE *.cpp, *.h, *.ino des Verzeichnisses (Code, ohne Kommentare)."""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
def code(name):
    s = open(os.path.join(q, name), 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n')
    s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
    return [re.sub(r'//.*', '', l) for l in s.split('\n')]
dateien = sorted(f for f in os.listdir(q) if f.endswith(('.cpp', '.h', '.ino')))
fails = []; aufrufe = []; priv = []
for f in dateien:
    for k, l in enumerate(code(f)):
        if re.search(r'\btcp_tw_limit\s*\(\s*\)\s*;', l) and not re.match(r'^(extern\s+)?void\s', l.strip()): aufrufe.append((f, k))
        if re.search(r'#\s*include\s*[<"]lwip/priv/tcp_priv\.h[>"]', l): priv.append(f)
print(f"c65z statisch ({bez}): {len(dateien)} Dateien, {len(aufrufe)} Aufrufe von tcp_tw_limit (): " + ', '.join(f"{a}:{b + 1}" for a, b in aufrufe) + f"; tcp_priv.h in {priv}")
if len(aufrufe) != 2: fails.append(f"erwartet genau 2 Aufrufe, gefunden {len(aufrufe)}")
# C80 (Paket 2026-10-17-c80, design.md 9 N2): das befristete Instrument heapkarte.cpp liest die Listen ebenfalls -
# Ausnahme von Ent-32 bis zum Rueckbau (Ent-C4); danach wieder genau http.cpp
if sorted(priv) not in (['http.cpp'], ['heapkarte.cpp', 'http.cpp']): fails.append(f"privater lwIP-Kopf nicht genau in http.cpp: {priv}")
H = code('http.cpp')
def body(L, kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return None, []
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return i, L[i:j]
i, b = body(H, 'http_server_loop (void)')
t = [k for k, l in enumerate(b) if re.search(r'\btcp_tw_limit\s*\(\s*\)', l)]
a = next((k for k, l in enumerate(b) if re.search(r'if\s*\(\s*!\s*http_client\s*\)', l)), None)
r = next((k for k in range(a, len(b)) if re.match(r'^\s*}\s*$', b[k])), None) if a is not None else None
mp = next((k for k, l in enumerate(b) if 'HEAP_LOW_PATH_ACCEPT' in l), None)
if not (len(t) == 1 and r is not None and mp is not None and r < t[0] < mp and not any(b[x].strip() for x in range(r + 1, t[0]))):
    fails.append("http_server_loop (): tcp_tw_limit () nicht unmittelbar nach if (!http_client) return; und vor dem Messpunkt accept")
C = code('httpclient.cpp')
i, b = body(C, 'httpclient (const char')
t = [k for k, l in enumerate(b) if re.search(r'\btcp_tw_limit\s*\(\s*\)', l)]
c = next((k for k, l in enumerate(b) if re.search(r'client\.connect\s*\(', l)), None)
if not (len(t) == 1 and c is not None and t[0] < c and not any(b[x].strip() for x in range(t[0] + 1, c))):
    fails.append("httpclient (): tcp_tw_limit () nicht unmittelbar vor client.connect ()")
hs = '\n'.join(H)
if not re.search(r'#define\s+HTTP_TW_LIMIT\s+2\b', hs): fails.append("HTTP_TW_LIMIT ist nicht 2")
# Das Core-Makro laesst l + 1 uebrig (count_plus_1 ist die Anzahl, c65z Z1 belegt es dynamisch) - Argument HTTP_TW_LIMIT - 1
if not re.search(r'TCP_TW_LIMIT\s*\(\s*HTTP_TW_LIMIT\s*-\s*1\s*\)', hs): fails.append("tcp_tw_limit () nutzt nicht das Core-Makro TCP_TW_LIMIT (HTTP_TW_LIMIT - 1)")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c65z statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
