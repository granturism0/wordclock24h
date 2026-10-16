#!/usr/bin/env python3
"""c70c statisch (AKL.1, AKL.2, AKL.9; design.md 18.6). Aufruf: c70c_static.py <ESP-Quellverzeichnis> [bez].
Exit 0 OK, 1 FEHL, 2 KAPUTT.
  S0  Gegenstand vollstaendig: das Entfernen der Kommentare laesst die Zeilenzahl von http.cpp gleich (L477/E40); die
      Zahl der immutable-Zeichenketten nach Kommentarentfernung ist nicht groesser als roh gezaehlt und nicht 0
  S1  "Cache-Control: public, max-age=31536000, immutable" steht im Code in genau ZWEI Zeichenketten, beide in
      http_send_fs_file () - in der 304 und in der 200 -, jede als Zweig "immutable ? ... : ..." mit dem no-cache-Zweig
      gleicher Form daneben; "immutable" steht in keiner anderen Zeichenkette von http.cpp (Legacy, /api/, Hinweisseiten)
  S2  http_send_fs_file (): immutable wird genau einmal gesetzt, innerhalb von "if (cache)" und "if (*token)", und zwar
      aus "app_asset == APP_ASSET_VERSIONED && http_app_token_hits (token, etag)"
  S3  http_app_token_hits (): vorhanden; Laenge exakt (strlen (etag) == n + 2), Anfuehrungszeichen vorn und hinten,
      memcmp ueber n; leere Marke trifft nie; kein String, kein Heap, keine Ausgabe
  S4  http_get_param ("h") steht in http.cpp genau einmal, in http_send_fs_file (); der Wert (token) geht nur in
      http_app_token_hits () und "*token" - nie in http_send, Serial, log_printf, snprintf, strcpy
  S5  APP_ASSET_VERSIONED wird genau einmal zugewiesen, in http_app (), unter "app_asset && strcmp (asset_path,
      "app/index.html") && strcmp (asset_path, "app/sw.js")"; der Index-Zweig (/app, /app/) setzt es nicht
  S6  Zaehler: static uint16_t http_app_immutable und http_app_token_miss; jeder genau einmal erhoeht, unter "< 0xFFFF";
      nirgends sonst zugewiesen (nichts setzt zurueck)
  S7  device_ready: app_full < app_immutable < app_token_miss < Schluss (AKL.2)
  S8  http () setzt die Parameter (http_set_params) vor dem Verteiler, der http_app () ruft - so erreicht h den App-Pfad
  S9  keine neue Ausgabe auf Serial in http_send_fs_file () (UART): genau die bekannten Zeilen
  S10 http.cpp ohne NUL-Byte und als UTF-8 lesbar (DIR-015); CRLF-Zeilen gemeldet
"""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
roh_b = open(os.path.join(q, 'http.cpp'), 'rb').read()
roh = roh_b.decode('utf-8', 'replace').replace('\r\n', '\n')

def ohne_kommentare(s):
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
def in_fn(z, i, b): return i is not None and i < z <= i + len(b)

IMM = 'Cache-Control: public, max-age=31536000, immutable'
NOC = 'Cache-Control: no-cache'
# S0
imm_code = [(k + 1, lit) for k, l in enumerate(L) for lit in literale(l) if IMM in lit]
imm_roh = roh.count(IMM)
chk(f'S0 immutable-Zeichenketten vollstaendig gesehen ({len(imm_code)} im Code, {imm_roh} roh - Kommentare duerfen nur abziehen)', 0 < len(imm_code) <= imm_roh)
# S1
fi, fb = body('http_send_fs_file (const char * filename')
chk('S1 http_send_fs_file () gefunden', fi is not None)
chk(f'S1 immutable in genau zwei Zeichenketten ({len(imm_code)}: Zeilen {[z for z, _ in imm_code]})', len(imm_code) == 2)
chk(f'S1 beide in http_send_fs_file () (ausserhalb: {[z for z, _ in imm_code if not in_fn(z, fi, fb)]})', imm_code and all(in_fn(z, fi, fb) for z, _ in imm_code))
for z, lit in imm_code:
    zeile, folge = L[z - 1], L[z] if z < len(L) else ''
    chk(f'S1 Zeile {z}: als Zweig "immutable ? FS(...)"', re.search(r'\bhttp_send\s*\(\s*immutable\s*\?\s*FS\s*\(\s*"\\r\\n' + re.escape(IMM) + r'\\r\\nConnection: close\\r\\n\\r\\n"\s*\)', zeile) is not None)
    chk(f'S1 Zeile {z}: daneben der no-cache-Zweig gleicher Form', re.search(r'^\s*:\s*FS\s*\(\s*"\\r\\n' + re.escape(NOC) + r'\\r\\nConnection: close\\r\\n\\r\\n"\s*\)\s*\)\s*;', folge) is not None)
# eine in der 304, eine in der 200
st304 = next((k + 1 for k, l in enumerate(L) if '"HTTP/1.1 304 Not Modified' in l), None)
st200 = next((k + 1 for k, l in enumerate(L) if '"HTTP/1.1 200 OK' in l), None)
zz = sorted(z for z, _ in imm_code)
chk(f'S1 eine in der 304 (nach Zeile {st304}, vor der 200 {st200}), eine nach der 200 ({zz})',
    len(zz) == 2 and st304 and st200 and st304 < zz[0] < st200 < zz[1])
andere = [(k + 1, lit) for k, l in enumerate(L) for lit in literale(l) if 'immutable' in lit and IMM not in lit and lit != '"app_immutable"']
chk(f'S1 "immutable" in keiner anderen Zeichenkette ({andere})', not andere)
# S2
zuw = [(k, l.strip()) for k, l in enumerate(fb) if re.search(r'\bimmutable\s*=(?!=)', l) and not re.search(r'uint_fast8_t\s+immutable\s*=\s*0\s*;', l)]
chk(f'S2 immutable genau einmal gesetzt ({[x for _, x in zuw]})', len(zuw) == 1)
if len(zuw) == 1:
    k = zuw[0][0]
    chk('S2 aus app_asset == APP_ASSET_VERSIONED && http_app_token_hits (token, etag)',
        re.fullmatch(r'immutable\s*=\s*\(\s*app_asset\s*==\s*APP_ASSET_VERSIONED\s*&&\s*http_app_token_hits\s*\(\s*token\s*,\s*etag\s*\)\s*\)\s*;', zuw[0][1]) is not None)
    # umschliessende Bedingungen: rueckwaerts die oeffnenden Bloecke sammeln
    tiefe = 0; bed = []
    for j in range(k - 1, 0, -1):
        t = fb[j].strip()
        if t == '}': tiefe += 1
        elif t == '{':
            if tiefe == 0:
                bed.append(fb[j - 1].strip())
            else:
                tiefe -= 1
    chk(f'S2 innerhalb von if (*token) und if (cache) ({bed[:3]})', len(bed) >= 2 and re.fullmatch(r'if\s*\(\s*\*token\s*\)', bed[0]) and re.fullmatch(r'if\s*\(\s*cache\s*\)', bed[1]))
init = [l.strip() for l in fb if re.search(r'uint_fast8_t\s+immutable\s*=\s*0\s*;', l)]
chk('S2 immutable mit 0 vorbelegt', len(init) == 1)
# S3
ti, tb = body('http_app_token_hits (const char * token, const char * etag)')
tt = ' '.join(x.strip() for x in tb)
chk('S3 http_app_token_hits () vorhanden', ti is not None)
chk('S3 Laenge exakt: strlen (etag) == n + 2', re.search(r'strlen\s*\(\s*etag\s*\)\s*==\s*n\s*\+\s*2', tt) is not None)
chk('S3 n = strlen (token), n > 0', re.search(r'n\s*=\s*strlen\s*\(\s*token\s*\)', tt) and re.search(r'\bn\s*>\s*0\s*&&', tt))
chk("S3 Anfuehrungszeichen vorn und hinten", "etag[0] == '\"'" in tt and "etag[n + 1] == '\"'" in tt)
chk('S3 memcmp (etag + 1, token, n)', re.search(r'!\s*memcmp\s*\(\s*etag\s*\+\s*1\s*,\s*token\s*,\s*n\s*\)', tt) is not None)
schlecht = [l.strip() for l in tb[1:] if re.search(r'\bString\b|\bnew\b|\bmalloc\s*\(|\bstrdup\s*\(|\bSerial\b|\bhttp_send\b|\blog_printf\b', l)]
chk(f'S3 kein String, kein Heap, keine Ausgabe ({schlecht})', ti is not None and not schlecht)
# S4
gp = [(k + 1, l.strip()) for k, l in enumerate(L) if re.search(r'\bhttp_get_param\s*\(\s*"h"\s*\)', l)]
chk(f'S4 http_get_param ("h") genau einmal, in http_send_fs_file () ({gp})', len(gp) == 1 and in_fn(gp[0][0], fi, fb))
tok = [l.strip() for l in fb if re.search(r'\btoken\b', l)]
erlaubt = lambda l: (re.search(r'const char \*\s+token\s*=\s*http_get_param\s*\(\s*"h"\s*\)\s*;', l) or re.fullmatch(r'if\s*\(\s*\*token\s*\)', l)
                     or re.search(r'http_app_token_hits\s*\(\s*token\s*,\s*etag\s*\)', l))
fremd = [l for l in tok if not erlaubt(l)]
chk(f'S4 token nur gelesen, verglichen, nie ausgegeben ({fremd})', tok and not fremd)
chk('S4 kein http_send/Serial/log_printf mit token oder http_get_param im ganzen http.cpp ausser S4',
    not [l for l in L if re.search(r'(http_send|Serial\.\w+|log_printf|snprintf\w*)\s*\([^;]*\b(token|http_get_param\s*\(\s*"h")', l)])
# S5
ai, ab = body('http_app (const char * path)')
av = [(k, l.strip()) for k, l in enumerate(L) if re.search(r'=\s*APP_ASSET_VERSIONED\s*;', l)]
chk(f'S5 APP_ASSET_VERSIONED genau einmal zugewiesen, in http_app () ({[x for _, x in av]})', len(av) == 1 and in_fn(av[0][0] + 1, ai, ab) and re.fullmatch(r'app_asset\s*=\s*APP_ASSET_VERSIONED\s*;', av[0][1]))
if len(av) == 1:
    k = av[0][0]
    vor = L[k - 2].strip() if L[k - 1].strip() == '{' else L[k - 1].strip()
    chk(f'S5 nur unter app_asset && nicht Dokument && nicht sw.js ({vor})',
        re.fullmatch(r'if\s*\(\s*app_asset\s*&&\s*strcmp\s*\(\s*asset_path\s*,\s*"app/index\.html"\s*\)\s*&&\s*strcmp\s*\(\s*asset_path\s*,\s*"app/sw\.js"\s*\)\s*\)', vor) is not None)
chk('S5 #define APP_ASSET_VERSIONED 2, APP_ASSET_LISTED 1', re.search(r'^#define APP_ASSET_VERSIONED\s+2\b', S, re.M) and re.search(r'^#define APP_ASSET_LISTED\s+1\b', S, re.M))
ix = [l.strip() for l in ab if re.search(r'app_asset\s*=\s*\(http_find_app_install_asset\s*\(\s*"app/index\.html"\s*\)\s*!=\s*0\s*\)\s*;', l)]
chk('S5 Index-Zweig: app_asset bleibt 0/1 (APP_ASSET_LISTED)', len(ix) == 1)
# S6
for v in ('http_app_immutable', 'http_app_token_miss'):
    dek = [l for l in L if re.match(r'^static uint16_t\s+' + v + r'\s*=\s*0\s*;', l)]
    chk(f'S6 static uint16_t {v} = 0', len(dek) == 1)
    inc = [(k, l.strip()) for k, l in enumerate(L) if re.search(r'\b' + v + r'\s*\+\+', l)]
    chk(f'S6 {v} genau einmal erhoeht ({len(inc)})', len(inc) == 1 and in_fn(inc[0][0] + 1, fi, fb))
    if len(inc) == 1:
        k = inc[0][0]
        w = L[k - 2].strip() if L[k - 1].strip() == '{' else L[k - 1].strip()
        chk(f'S6 {v}: saettigend unter "< 0xFFFF" ({w})', re.search(r'\b' + v + r'\s*<\s*0xFFFF\s*\)', w) is not None)
    zw = [l.strip() for l in L if re.search(r'\b' + v + r'\s*=(?!=)', l) and not re.match(r'^static uint16_t', l)]
    chk(f'S6 {v} nirgends sonst zugewiesen ({zw})', not zw)
# S7
di, db = body('http_api_device_ready ()')
idx = lambda pat: next((k for k, l in enumerate(db) if pat in l), None)
fu, im, tm, zu = idx('FS("app_full")'), idx('FS("app_immutable")'), idx('FS("app_token_miss")'), idx('http_send (FS("}"));')
chk(f'S7 device_ready: app_full < app_immutable < app_token_miss < Schluss ({fu}, {im}, {tm}, {zu})', None not in (fu, im, tm, zu) and fu < im < tm < zu)
if None not in (im, tm):
    chk('S7 app_immutable gibt http_app_immutable aus', re.search(r'FS\("app_immutable"\)\s*,\s*\(unsigned long\)\s*http_app_immutable\s*\)', db[im]) is not None)
    chk('S7 app_token_miss gibt http_app_token_miss aus', re.search(r'FS\("app_token_miss"\)\s*,\s*\(unsigned long\)\s*http_app_token_miss\s*\)', db[tm]) is not None)
# S8
hi, hb = body('http (const char * path, const char * const_param)')
sp = next((k for k, l in enumerate(hb) if re.search(r'\bhttp_set_params\s*\(\s*param\s*\)\s*;', l)), None)
ap = next((k for k, l in enumerate(hb) if re.search(r'\brtc\s*=\s*http_app\s*\(\s*path\s*\)\s*;', l)), None)
chk(f'S8 http (): http_set_params (param) vor rtc = http_app (path) ({sp}, {ap})', None not in (sp, ap) and sp < ap)
# S9
ser = [l.strip() for l in fb if re.search(r'\bSerial\s*\.', l)]
SER_SOLL = sorted(['Serial.print (F("- fs seek error: "));', 'Serial.println (filename);', 'Serial.flush ();',
                   'Serial.print ("- fs read error: ");', 'Serial.println (filename);', 'Serial.flush ();',
                   'Serial.println (line);', 'Serial.flush ();'])          # Stand R2: Fehlerzeilen, I-2-Zeile
chk(f'S9 http_send_fs_file (): Serial genau in den acht bekannten Zeilen wie R2 ({len(ser)}: {sorted(ser) if sorted(ser) != SER_SOLL else "gleich"})', sorted(ser) == SER_SOLL)
# S10
chk('S10 http.cpp ohne NUL-Byte', roh_b.count(b'\0') == 0)
try: roh_b.decode('utf-8'); u = True
except UnicodeDecodeError: u = False
chk('S10 http.cpp als UTF-8 lesbar', u)
print(f"c70c statisch ({bez}): {n} Pruefungen, {len(imm_code)} immutable-Zeichenketten, http_get_param (\"h\") {len(gp)}x, CRLF-Zeilen {roh_b.count(chr(13).encode() + chr(10).encode())}")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c70c statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
