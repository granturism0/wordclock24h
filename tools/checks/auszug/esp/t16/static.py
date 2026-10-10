#!/usr/bin/env python3
"""Statische Pruefung C47 / AKX.2 (Paket 2026-10-10), zweite Fassung (Review E, M3).

Aufruf: static.py <http.cpp> <bestand.tsv> <bezeichnung>

Gegenstand: jede Ausgabe in HTML-Seiten von http.cpp - die Legacy-Seiten (http_style .. http_update) und
die HTML-Seiten ausserhalb davon (EXTRA). Geprueft wird nicht nur der Rumpf der Ausgabehelfer, sondern
jede AUFRUFSTELLE:

  Senke        http_send (x), http_send_xml_escaped (x) - roh. http_send_attr (x) ist maskiert, fertig.
  Weiterreicher  Ein Helfer, der einen seiner Parameter roh ausgibt (direkt oder ueber einen anderen
               Helfer), heisst an dieser Stelle "roh". Das wird bis zum Fixpunkt ausgerechnet - also auch
               table_row -> text_column -> http_send. Wer einen Parameter an eine rohe Stelle weiterreicht,
               wird selbst roh, und gepruefte Argumente sind dann die an SEINEN Aufrufstellen.
  Argument an roher Stelle ist zulaessig, wenn es
    - ein Literal ist ("...", FS("..."), Zahl, NULL, Bedingung mit zwei Literalen),
    - eine lokale Variable/ein lokales Feld ist, die in der Funktion nur Literale zugewiesen bekommt,
    - ein lokaler Puffer ist, der nur ueber sprintf/snprintf mit Zahlformaten (kein %s, %c) bzw. mit
      strcpy/strcat von Literalen beschrieben wird ("Zahl"),
    - oder in bestand.tsv steht (funktion, aufgerufen, argument), mit Gruppe Zahl/Konstante/gefiltert.
  Alles andere ist ein Befund. Ein Listeneintrag, der im Code nicht mehr vorkommt, ebenfalls.

Gegenprobe: ein eingeschleustes table_row ("X", sv->str, "") und ein rohes http_send (remote_app_version)
muessen gemeldet werden.
"""
import re, sys
from collections import Counter

src, liste, bez = sys.argv[1:4]
TEXT = open(src, 'rb').read().decode('utf-8').replace('\r\n', '\n')
L = TEXT.split('\n')

EXTRA = ['http_app', 'http_api_remote_esp_update', 'reset_button', 'run_local_stm32_flash_response',
         'flash_stm32_local', 'http_post']
RAW_SINKS = {'http_send': {0}, 'http_send_xml_escaped': {0}}
MASK_SINKS = {'http_send_attr'}

# ---- Funktionen ------------------------------------------------------------------------------------
tot = set(); d0 = None                       # Zeilen in #if 0 .. #endif (Spalte 0) zaehlen nicht
for i, l in enumerate(L):
    if l.startswith('#if 0'): d0 = i
    elif l.startswith('#endif') and d0 is not None: tot.update(range(d0, i + 1)); d0 = None
heads = [i for i, l in enumerate(L) if re.match(r'^[a-z_][a-z_0-9]* ?\(', l) and i not in tot]
funcs = []                                   # (name, kopfzeile, rumpf-start, rumpf-ende, params)
for k, i in enumerate(heads):
    name = re.match(r'^([a-z_][a-z_0-9]*)', L[i]).group(1)
    j = i
    while ')' not in L[j]:
        j += 1
    kopf = ' '.join(L[i:j + 1])
    plist = kopf[kopf.index('(') + 1:kopf.rindex(')')]
    params = []
    for p in plist.split(','):
        p = p.split('=')[0].strip()
        m = re.search(r'([A-Za-z_][A-Za-z_0-9]*)\s*(\[[^\]]*\])?$', p)
        params.append(m.group(1) if m and p not in ('void', '') else None)
    e = j
    while e < len(L) and not L[e].startswith('}'):
        e += 1
    if e < len(L) and '{' in '\n'.join(L[j:j + 3]):
        funcs.append((name, i, j + 1, e, params))
names = [f[0] for f in funcs]
if 'http_style' not in names or 'http_update' not in names:
    print(f'KAPUTT ({bez}): Anker http_style/http_update fehlen'); sys.exit(2)
a = next(f[1] for f in funcs if f[0] == 'http_style')
z = next(f[1] for f in funcs if f[0] == 'http_update')
scope = [f for f in funcs if a <= f[1] <= z or f[0] in EXTRA]
scope_names = set(f[0] for f in scope)

# ---- Quelltext ohne Kommentare, Zeichenketten bleiben ------------------------------------------------
def body_text(f):
    s = '\n'.join(L[f[2]:f[3] + 1])
    out, i, n = [], 0, len(s)
    while i < n:
        if s.startswith('//', i):
            while i < n and s[i] != '\n': i += 1
        elif s.startswith('/*', i):
            j = s.find('*/', i + 2); i = n if j < 0 else j + 2
        elif s[i] in '"\'':
            q = s[i]; j = i + 1
            while j < n and s[j] != q:
                j += 2 if s[j] == '\\' else 1
            out.append(s[i:j + 1]); i = j + 1
        elif s.startswith('#if 0', i) and (i == 0 or s[i - 1] == '\n'):
            j = s.find('#endif', i); i = n if j < 0 else j + 6
        else:
            out.append(s[i]); i += 1
    return ''.join(out)

def split_args(s, start):
    """s[start] == '(' ; liefert (argumente, ende)."""
    depth, i, cur, args = 0, start, [], []
    while i < len(s):
        c = s[i]
        if c in '"\'':
            q = c; j = i + 1
            while s[j] != q:
                j += 2 if s[j] == '\\' else 1
            cur.append(s[i:j + 1]); i = j + 1; continue
        if c == '(':
            depth += 1
            if depth > 1: cur.append(c)
        elif c == ')':
            depth -= 1
            if depth == 0:
                args.append(''.join(cur).strip()); return args, i
            cur.append(c)
        elif c == ',' and depth == 1:
            args.append(''.join(cur).strip()); cur = []
        else:
            cur.append(c)
        i += 1
    return args, i

def calls(body, callee):
    for m in re.finditer(r'(?<![A-Za-z0-9_])' + re.escape(callee) + r'\s*\(', body):
        args, _ = split_args(body, m.end() - 1)
        yield args

LIT = re.compile(r'^(\s*"([^"\\]|\\.)*"\s*)+$')
def is_literal(a):
    a = a.strip()
    if LIT.match(a) or re.match(r'^FS\((\s*"([^"\\]|\\.)*"\s*)+\)$', a): return True
    if re.match(r'^-?\d+$', a) or a in ('NULL', '0', '(const char *) NULL', '(const char *) 0', '(char *) 0', '(const char **) 0'): return True
    m = re.match(r'^[^?]*\?\s*("([^"\\]|\\.)*")\s*:\s*("([^"\\]|\\.)*")$', a)
    return bool(m)

NUMFMT = re.compile(r'%[-+ 0#]*\d*(l|ll|h)?[diuxX]')
def local_class(body, var):
    """'const' wenn nur Literale zugewiesen, 'zahl' wenn nur Zahl-sprintf/Literal-strcpy, sonst None."""
    base = re.match(r'^([A-Za-z_][A-Za-z_0-9]*)', var)
    if not base: return None
    v = base.group(1)
    if not re.match(r'^[A-Za-z_][A-Za-z_0-9]*(\s*\[[A-Za-z_0-9 +\-]*\])?$', var.strip()): return None
    # Deklaration mit Literal bzw. Literal-Feld
    decl = re.search(r'(?:const\s+)?char\s*\*\s*(?:const\s+)?' + v + r'\s*(\[[^\]]*\])?\s*=\s*(\{[^}]*\}|[^;]+);', body)
    assigns = [(m.group(1), m.group(2)) for m in re.finditer(r'(?<![A-Za-z0-9_.>])' + v + r'(\s*\[[^\]]*\])?\s*=(?!=)\s*([^;]+);', body)
               if not (decl and decl.start() <= m.start() < decl.end())]
    writes = [m for m in re.finditer(r'\b(s?n?printf|strcpy|strcat|strncpy|snprintf|sprintf|http_format_half_deg|memcpy)\s*\(\s*' + v + r'\b', body)]
    if decl or assigns:
        vals = []
        if decl:
            init = decl.group(2).strip()
            vals += [x.strip() for x in init.strip('{}').split(',')] if init.startswith('{') else [init]
        vals += [x[1].strip() for x in assigns]
        vals = [x for x in vals if x]
        if vals and all(is_literal(x) for x in vals) and not writes:
            return 'const'
        return None
    if not writes: return None
    ok = True
    for m in writes:
        args, _ = split_args(body, body.index('(', m.start()))
        fn = m.group(1)
        if fn in ('sprintf', 'snprintf'):
            fmt = args[1 if fn == 'sprintf' else 2] if len(args) > (1 if fn == 'sprintf' else 2) else ''
            if not LIT.match(fmt) or re.search(r'%[-+ 0#]*\d*[sc]', fmt): ok = False
        elif fn in ('strcpy', 'strcat'):
            if len(args) < 2 or not is_literal(args[1]): ok = False
        else:
            ok = False
    return 'zahl' if ok else None

def param_index(f, a):
    """Index des Parameters, wenn a der Parameter selbst oder ein Element davon ist (ids[i])."""
    m = re.match(r'^([A-Za-z_][A-Za-z_0-9]*)(\s*\[[^\]]*\])?$', a)
    return f[4].index(m.group(1)) if m and m.group(1) in f[4] else None

# ---- Fixpunkt: welche Parameter welcher Funktion sind roh ----------------------------------------------
bodies = {id(f): body_text(f) for f in funcs}
raw = {k: set(v) for k, v in RAW_SINKS.items()}
fbyname = {}
for f in funcs:
    fbyname.setdefault(f[0], []).append(f)
changed = True
while changed:
    changed = False
    for f in funcs:
        body = bodies[id(f)]
        for callee, pos in list(raw.items()):
            if callee in RAW_SINKS and f[0] not in scope_names:
                continue
            for args in calls(body, callee):
                for p in pos:
                    if p < len(args):
                        a = args[p].strip()
                        idx = param_index(f, a)
                        if idx is not None:
                            if idx not in raw.setdefault(f[0], set()):
                                raw[f[0]].add(idx); changed = True

# ---- Klassifikation jeder rohen Aufrufstelle -----------------------------------------------------------
ist = Counter(); stats = Counter(); weiter = Counter(); je = Counter()
for f in funcs:
    body = bodies[id(f)]
    for callee, pos in raw.items():
        if callee in RAW_SINKS and f[0] not in scope_names:
            continue
        for args in calls(body, callee):
            for p in sorted(pos):
                if p >= len(args): continue
                a = args[p].strip()
                stats['geprueft'] += 1
                je[callee] += 1
                if is_literal(a): stats['literal'] += 1; continue
                if param_index(f, a) is not None: stats['weitergereicht'] += 1; weiter[(f[0], callee, a)] += 1; continue
                lc = local_class(body, a)
                if lc == 'const': stats['lokale Konstante'] += 1; continue
                if lc == 'zahl': stats['lokale Zahl'] += 1; continue
                ist[(f[0], callee, re.sub(r'\s+', ' ', a))] += 1
    for args in calls(body, 'http_send_attr'):
        if f[0] in scope_names: stats['maskiert'] += 1

soll = Counter(); gruppe = {}
for zl in open(liste, encoding='utf-8'):
    zl = zl.rstrip('\n')
    if not zl or zl.startswith('#') or zl.startswith('funktion\t'): continue
    fn, aus, arg, g, grund = zl.split('\t')
    soll[(fn, aus, arg)] += 1; gruppe[(fn, aus, arg)] = g
bad = 0
for key, c in sorted(ist.items()):
    if c > soll.get(key, 0):
        bad += 1; print(f'  FEHL roh, nicht eingeordnet: {key[0]}: {key[1]} ({key[2]}) x{c - soll.get(key, 0)}')
for key, c in sorted(soll.items()):
    if c > ist.get(key, 0):
        bad += 1; print(f'  FEHL Liste veraltet: {key[0]}: {key[1]} ({key[2]})')
for key, g in gruppe.items():
    if g not in ('Zahl', 'Konstante', 'gefiltert') or key[1] == 'http_send_xml_escaped':
        bad += 1; print(f'  FEHL Gruppe {g} bei {key}')
for key in ist:
    if key[1] == 'http_send_xml_escaped':
        bad += 1; print(f'  FEHL http_send_xml_escaped in {key[0]} - ersetzt ungueltiges UTF-8 durch ?')
helfer = sorted(k for k in raw if k not in RAW_SINKS)
print(f'  rohe Helfer (Fixpunkt): {len(helfer)}: {", ".join(helfer)}')
print(f'  Aufrufstellen-Argumente an rohen Stellen: {stats["geprueft"]} = {stats["literal"]} Literal + '
      f'{stats["lokale Konstante"]} lokale Konstante + {stats["lokale Zahl"]} lokale Zahl + '
      f'{stats["weitergereicht"]} weitergereicht + {sum(ist.values())} gegen die Liste; maskiert: {stats["maskiert"]}')
print('  je Ziel: ' + ', '.join(f'{k} {v}' for k, v in sorted(je.items())))
print(f'{"FEHLGESCHLAGEN" if bad else "OK"} ({bez}): {len(scope)} Seitenfunktionen, {sum(ist.values())} Argumente gegen '
      f'{sum(soll.values())} Listeneintraege, {bad} Befunde')
sys.exit(1 if bad else 0)
