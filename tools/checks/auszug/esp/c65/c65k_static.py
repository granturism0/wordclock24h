#!/usr/bin/env python3
"""c65k statisch (AKV.5): WcHttpServer ist Unterklasse von WiFiServer, http_server ist eine Instanz davon, Start mit
begin (80, HTTP_BACKLOG_HIGH), #error ohne TCP_LISTEN_BACKLOG, http_backlog_track () genau einmal und VOR accept ()
in http_server_loop (). Aufruf: c65k_static.py <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 OK, 1 FEHL."""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
s = open(os.path.join(q, 'http.cpp'), 'rb').read().decode('utf-8').replace('\r\n', '\n')
c = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
c = '\n'.join(re.sub(r'//.*', '', l) for l in c.split('\n'))
fails = []
def chk(name, ok):
    if not ok: fails.append(name)
chk('class WcHttpServer : public WiFiServer', re.search(r'^class\s+WcHttpServer\s*:\s*public\s+WiFiServer\b', c, re.M))
chk('WcHttpServer http_server(80)', re.search(r'^WcHttpServer\s+http_server\s*\(\s*80\s*\)\s*;', c, re.M))
chk('kein WiFiServer http_server mehr', not re.search(r'^WiFiServer\s+http_server\b', c, re.M))
chk('#if ! TCP_LISTEN_BACKLOG / #error', re.search(r'^#if\s*!\s*TCP_LISTEN_BACKLOG\s*\n#error', c, re.M))
L = c.split('\n')
def body(kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return []
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return L[i:j]
b = body('http_server_begin (void)')
chk('http_server.begin (80, HTTP_BACKLOG_HIGH) in http_server_begin ()', any(re.search(r'http_server\.begin\s*\(\s*80\s*,\s*HTTP_BACKLOG_HIGH\s*\)', l) for l in b))
b = body('http_server_loop (void)')
t = [k for k, l in enumerate(b) if re.search(r'\bhttp_backlog_track\s*\(\s*\)', l)]
a = [k for k, l in enumerate(b) if re.search(r'http_server\.accept\s*\(', l)]
chk(f'http_backlog_track () genau einmal in http_server_loop () (gefunden {len(t)})', len(t) == 1)
chk('http_backlog_track () vor accept ()', len(t) == 1 and len(a) == 1 and t[0] < a[0])
chk('#define HTTP_BACKLOG_HEAP_HIGH 5900 / MID 5100 / HYST 256', re.search(r'#define HTTP_BACKLOG_HEAP_HIGH\s+5900\b', c) and re.search(r'#define HTTP_BACKLOG_HEAP_MID\s+5100\b', c) and re.search(r'#define HTTP_BACKLOG_HYST\s+256\b', c))
print(f"c65k statisch ({bez}): 9 Pruefungen")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c65k statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
