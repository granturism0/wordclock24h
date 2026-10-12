# Pruefstand N5 (Paket 2026-10-11, S3.2): eigene snprintf/sprintf (stubs.c) gegen die libc des Hosts.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis> [sabotage]
# 1. schneidet die beiden Huellen woertlich aus stubs.c (umbenannt in w_snprintf / w_sprintf)
# 2. sammelt JEDE Formatzeichenkette, mit der src/** sprintf oder snprintf ruft, und erzeugt dazu Vergleiche
#    mit festen Randwerten und Zufallswerten -- so prueft der Stand die Formate, die es WIRKLICH gibt.
import sys, os, re, random
src, w = sys.argv[1], sys.argv[2]
sab = sys.argv[3] if len(sys.argv) > 3 else ''
st = open(os.path.join(src, 'stubs/stubs.c'), 'rb').read().decode('latin-1')
SAB = {
    'abschneiden': ('rtc = vsnprintf (buf, INT_MAX, fmt, ap);', 'rtc = vsnprintf (buf, 16, fmt, ap);'),
    'groesse':     ('rtc = vsnprintf (buf, size, fmt, ap);', 'rtc = vsnprintf (buf, size ? size - 1 : 0, fmt, ap);'),
    # snprintf gibt die GESCHRIEBENE statt der benoetigten Laenge zurueck -- ein naheliegender Fehler
    'rueckgabe':   ('    return rtc;\n}\n\n__attribute__((used, externally_visible)) int\nsprintf', '    return (size && rtc >= (int) size) ? (int) size - 1 : rtc;\n}\n\n__attribute__((used, externally_visible)) int\nsprintf'),
}
if sab:
    if sab not in SAB or st.count(SAB[sab][0]) != 1: sys.exit('extract: Sabotage %s -- Anker fehlt' % sab)
    st = st.replace(*SAB[sab])
def fn(name):
    # Definition samt vorangestelltem __attribute__ -- das erste Muster ohne Attribut fand sie nicht und
    # verglich still libc mit libc (Sabotagen blieben gruen; DIR-014, beim Bau bemerkt)
    m = re.search(r'\n(?:__attribute__\(\([^)]*\)\)\s*)?int\n' + name + r' \(', st)
    if not m: return None
    a = m.start() + 1; b = st.index('\n}\n', a) + 3
    return re.sub(r'\n' + name + r' \(', '\nw_' + name + ' (', st[a:b], count=1)
f1, f2 = fn('snprintf'), fn('sprintf')
hat_n5 = 'vsnprintf (buf, INT_MAX' in st or 'vsnprintf (buf, size' in st
if hat_n5 and (not f1 or not f2):
    sys.exit('extract: Huellen in stubs.c vorhanden, aber nicht ausgeschnitten -- Muster pruefen')
if not f1 or not f2:
    open(os.path.join(w, 'huellen.c'), 'w').write('#define w_snprintf snprintf\n#define w_sprintf sprintf\n#define FASSUNG "libc (Stand ohne N5)"\n')
else:
    if 'w_snprintf (' not in f1 or 'w_sprintf (' not in f2: sys.exit('extract: Umbenennung gescheitert')
    open(os.path.join(w, 'huellen.c'), 'w').write('#define FASSUNG "Huellen aus stubs.c"\n' + f1 + '\n' + f2)
# Formate aus src/**
formate = {}
for d, _, fs in os.walk(src):
    for f in fs:
        if not f.endswith(('.c', '.h')): continue
        t = open(os.path.join(d, f), 'rb').read().decode('latin-1')
        t = re.sub(r'/\*.*?\*/', '', t, flags=re.S); t = re.sub(r'//[^\n]*', '', t)
        for m in re.finditer(r'\b(sn?printf)\s*\(', t):
            if os.path.join(d, f).endswith('stubs/stubs.c'): continue
            call = t[m.end():m.end() + 400].split(';')[0]
            s = re.findall(r'"((?:[^"\\]|\\.)*)"', call)
            if s: formate.setdefault(''.join(s), set()).add(m.group(1))
r = random.Random(20261012)
INTS = [0, 1, -1, 5, 9, 10, 99, 100, 255, 256, 4095, 65535, 65536, 2147483647, -2147483648]
STRS = ['""', '"a"', '"1.2.3.4"', '"\\"q\\""', '"' + 'x' * 40 + '"']
zeilen = []; n_aufrufe = 0
for fmt in sorted(formate):
    conv = re.findall(r'%[-0 #+]*\d*(l?)([dux s])', fmt.replace('%%', ''))
    for k in range(12):
        args = []
        for (l, c) in conv:
            if c == 's': args.append(STRS[k % len(STRS)] if k < 10 else r.choice(STRS))
            else:
                v = INTS[(k * 3 + len(args)) % len(INTS)] if k < 8 else r.randint(-2**31, 2**31 - 1)
                args.append('(long) %d' % v if l else ('%dU' % (v & 0xFFFFFFFF) if c in 'ux' else '%d' % v))
        a = (', ' + ', '.join(args)) if args else ''
        zeilen.append('    VERGL_SPRINTF ("%s"%s);' % (fmt, a))
        for size in (0, 1, 3, 8, 64):
            zeilen.append('    VERGL_SNPRINTF (%d, "%s"%s);' % (size, fmt, a))
        n_aufrufe += 6
open(os.path.join(w, 'faelle.c'), 'w').write('#define N_FORMATE %d\n#define N_VERGLEICHE %d\nstatic void alle_faelle (void)\n{\n%s\n}\n' % (len(formate), n_aufrufe, '\n'.join(zeilen)))
print('extract: %d Formate aus src, %d Vergleiche' % (len(formate), n_aufrufe))
