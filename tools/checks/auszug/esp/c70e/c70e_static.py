#!/usr/bin/env python3
"""c70e statisch (AKC.1, AKC.2, AKC.4, AKC.7, AKZ.8; design.md 4.4). Aufruf: c70e_static.py <ESP-Quellverzeichnis> [bez].
Exit 0 OK, 1 FEHL, 2 KAPUTT.
  S0  Gegenstand vollstaendig: das Entfernen der Kommentare laesst die Zeilenzahl von http.cpp gleich (L477/E40),
      und die Zahl der gefundenen Antwort-Statuszeilen ist nicht kleiner als die roh gezaehlte
  S1  "HTTP/1.1" steht in http.cpp in genau ZWEI Zeichenketten, beide in http_send_fs_file (): "HTTP/1.1 304 Not
      Modified" und "HTTP/1.1 200 OK" - Legacy, /api/ und die Hinweisseiten bleiben HTTP/1.0 (AKC.7)
  S2  nach jeder der beiden Statuszeilen steht vor dem naechsten http_flush () "Connection: close\\r\\n\\r\\n" (AKC.7)
  S3  die 304 sendet zwischen Statuszeile und http_flush () weder Content-Length noch Content-Type noch Koerper
  S4  die Leseschleife in http_send_fs_file () laeuft nicht nach einer 304 (Bedingung mit not_modified)
  S5  http_server_loop (), GET-Zweig: http_clear_request_if_none_match () vor der Kopfzeilenschleife,
      http_capture_request_if_none_match (sHeaderLine) in ihr (AKC.2)
  S6  Puffer: #define APP_INM_LEN 36, static char http_request_if_none_match[APP_INM_LEN] (AKZ.8)
  S7  kein String-Objekt und kein Heap in den C70-Hilfsfunktionen (AKC.4); keine Ausgabe ausser der Fehlerzeile
  S8  device_ready: app_not_modified und app_full hinter http_accept_dead, vor dem Schluss (AKC.4)
  S9  http_send_fs_file () hat genau einen Aufrufer, http_app (), und der gibt app_asset aus der Weissliste mit
  S10 http.cpp ohne NUL-Byte und als UTF-8 lesbar (DIR-015)
"""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
roh_b = open(os.path.join(q, 'http.cpp'), 'rb').read()
roh = roh_b.decode('utf-8', 'replace').replace('\r\n', '\n')

def ohne_kommentare(s):
    """Kommentare durch Leerzeichen ersetzen (Zeilenumbrueche bleiben), Zeichenketten und Zeichenliterale behalten."""
    o = []; i = 0; n = len(s)
    while i < n:
        c = s[i]
        if c == '/' and i + 1 < n and s[i + 1] == '/':
            while i < n and s[i] != '\n': i += 1
        elif c == '/' and i + 1 < n and s[i + 1] == '*':
            j = s.find('*/', i + 2); j = n if j < 0 else j + 2
            o.append(''.join('\n' if x == '\n' else ' ' for x in s[i:j])); i = j
        elif c in '"\'':
            j = i + 1
            while j < n and s[j] != c and s[j] != '\n':
                j += 2 if s[j] == '\\' else 1
            o.append(s[i:j + 1]); i = j + 1
        else:
            o.append(c); i += 1
    return ''.join(o)

S = ohne_kommentare(roh)
L = S.split('\n')
fails = []; n = 0
def chk(name, ok):
    global n; n += 1
    if not ok: fails.append(name)
if len(L) != roh.count('\n') + 1:
    print(f'KAPUTT: Kommentarentfernung veraendert die Zeilenzahl ({roh.count(chr(10)) + 1} -> {len(L)}) - die Pruefung saehe ihren Gegenstand nicht vollstaendig (L477/E40)'); sys.exit(2)

def body(kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return None, []
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return i, L[i:j + 1]
def literale(l): return re.findall(r'"(?:\\.|[^"\\])*"', l)

# S0 / S1
status = [(k + 1, lit) for k, l in enumerate(L) for lit in literale(l) if re.match(r'"HTTP/1\.[01] ', lit)]
roh_status = len(re.findall(r'"HTTP/1\.[01] ', roh))
chk(f'S0 Statuszeilen vollstaendig gesehen ({len(status)} nach Kommentarentfernung, {roh_status} roh - Kommentare duerfen nur abziehen)', 0 < len(status) <= roh_status)
fi, fb = body('http_send_fs_file (const char * filename')
f_von, f_bis = (fi + 1, fi + len(fb)) if fi is not None else (0, -1)
h11 = [(z, lit) for z, lit in status if lit.startswith('"HTTP/1.1')]
ausserhalb = [z for z, lit in h11 if not (f_von <= z <= f_bis)]
chk(f'S1 HTTP/1.1 an genau zwei Stellen ({len(h11)} gefunden: Zeilen {[z for z, _ in h11]})', len(h11) == 2)
chk(f'S1 HTTP/1.1 nur in http_send_fs_file () (ausserhalb: Zeilen {ausserhalb})', h11 and not ausserhalb)
chk('S1 die beiden sind die 304 und die 200', sorted(lit.split('\\r')[0] for _, lit in h11) == ['"HTTP/1.1 200 OK', '"HTTP/1.1 304 Not Modified'])
# S2 / S3
def bis_flush(z, eigener_block):
    """ab der Statuszeile bis zum naechsten http_flush () - bei der 304 nur bis zum Ende ihres eigenen Blocks: Sie muss
    fuer sich vollstaendig sein, der Abschluss der 200 dahinter zaehlt nicht."""
    out = []; ende = ' ' * max(0, len(L[z - 1]) - len(L[z - 1].lstrip()) - 4) + '}'
    for k in range(z - 1, len(L)):
        if eigener_block and L[k].rstrip() == ende: break
        out.append(L[k])
        if re.search(r'\bhttp_flush\s*\(\s*\)', L[k]): break
    return out
for z, lit in h11:
    st = bis_flush(z, '304' in lit); lits = [x for l in st for x in literale(l)]
    chk(f'S2 Zeile {z} {lit[:26]}: Connection: close vor dem naechsten http_flush ()', any('Connection: close\\r\\n\\r\\n' in x for x in lits))
    if '304' in lit:
        chk(f'S3 304 (Zeile {z}): weder Content-Length noch Content-Type', not any('Content-Length' in x or 'Content-Type' in x for x in lits))
        sends = [l.strip() for l in st if re.search(r'\bhttp_send\s*\(', l)]
        chk(f'S3 304 (Zeile {z}): genau drei http_send () - Statuszeile, ETag, Abschluss ({len(sends)})', len(sends) == 3 and 'http_send (etag);' in sends[1])
# S4
lese = [l.strip() for l in fb if re.match(r'\s*while\s*\(.*fp\.available\s*\(', l)]
chk(f'S4 Leseschleife nur ohne 304: {lese}', len(lese) == 1 and re.search(r'!\s*not_modified\s*&&', lese[0]))
# S5
si, sb = body('http_server_loop (void)')
RL = roh.split('\n')
g = next((k for k in range(len(sb)) if RL[si + k].startswith('    else // GET')), None) if si is not None else None
w = next((k for k in range(g or 0, len(sb)) if 'while (read_line (sHeaderLine))' in sb[k]), None) if g is not None else None
cl = [k for k in range(g or 0, (w or 0)) if re.search(r'\bhttp_clear_request_if_none_match\s*\(\s*\)\s*;', sb[k])] if w is not None else []
e = next((k for k in range(w, len(sb)) if sb[k].startswith('        }')), None) if w is not None else None
ca = [k for k in range(w or 0, (e or 0)) if re.search(r'\bhttp_capture_request_if_none_match\s*\(\s*sHeaderLine\s*\)\s*;', sb[k])] if e is not None else []
chk('S5 GET-Zweig: Puffer vor der Kopfzeilenschleife geleert', len(cl) == 1)
chk('S5 GET-Zweig: If-None-Match in der Kopfzeilenschleife erfasst', len(ca) == 1)
# S6
chk('S6 #define APP_INM_LEN 36', re.search(r'^#define APP_INM_LEN\s+36\b', S, re.M) is not None)
chk('S6 static char http_request_if_none_match[APP_INM_LEN]', re.search(r'^static char\s+http_request_if_none_match\[APP_INM_LEN\];', S, re.M) is not None)
# S7
for kopf in ('http_clear_request_if_none_match (void)', 'http_capture_request_if_none_match (const String& line)', 'http_app_etag (File& fp', 'http_app_if_none_match_hits (const char * etag)'):
    i, b = body(kopf)
    innen = b[1:] if b else []
    schlecht = [l.strip() for l in innen if re.search(r'\bString\b|\bnew\b|\bmalloc\s*\(|\bstrdup\s*\(|\bSerial\b|\bhttp_send\b', l)]
    chk(f'S7 {kopf.split(" (")[0]} (): vorhanden, kein String, kein Heap, keine Ausgabe {schlecht}', b and not schlecht)
# S8
di, db = body('http_api_device_ready ()')
idx = lambda pat: next((k for k, l in enumerate(db) if pat in l), None)
a, nm, fu, zu = idx('FS("http_accept_dead")'), idx('FS("app_not_modified")'), idx('FS("app_full")'), idx('http_send (FS("}"));')
chk(f'S8 device_ready: http_accept_dead < app_not_modified < app_full < Schluss ({a}, {nm}, {fu}, {zu})', None not in (a, nm, fu, zu) and a < nm < fu < zu)
# S9
rufe = [(k + 1, l.strip()) for k, l in enumerate(L) if re.search(r'\bhttp_send_fs_file\s*\(', l) and not l.startswith('http_send_fs_file (')]
ai, ab = body('http_app (const char * path)')
in_app = [x for x in rufe if ai is not None and ai < x[0] <= ai + len(ab)]
chk(f'S9 http_send_fs_file (): genau ein Aufrufer, in http_app () ({rufe})', len(rufe) == 1 and len(in_app) == 1)
chk('S9 http_app () gibt app_asset mit', in_app and re.search(r'http_send_fs_file\s*\(\s*filename\s*,\s*content_type\s*,\s*gzip_encoded\s*,\s*app_asset\s*\)', in_app[0][1]))
chk('S9 app_asset aus http_find_app_install_asset () (Index und Asset)', sum(1 for l in ab if re.search(r'app_asset\s*=\s*\(http_find_app_install_asset\s*\(', l)) == 2)
# S10
chk('S10 http.cpp ohne NUL-Byte', roh_b.count(b'\0') == 0)
try: roh_b.decode('utf-8'); u = True
except UnicodeDecodeError: u = False
chk('S10 http.cpp als UTF-8 lesbar', u)
print(f"c70e statisch ({bez}): {n} Pruefungen, {len(status)} Statuszeilen in http.cpp, davon {len(h11)} HTTP/1.1")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c70e statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
