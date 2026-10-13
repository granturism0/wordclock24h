# Pruefstand a82 (Paket 2026-10-13, S2.9, A82, AKA.5): DCF77-Merker folgt Display- UND Ambilight-Power.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis>
# WOERTLICH aus main.c: dcf77_update() (falls vorhanden), set_display_power(), Rumpf von
# "case DISPLAY_AMBILIGHT_POWER_NUM_VAR:" (Setter vom ESP) und der Ambilight-Timer-Zweig
# "if (night_check_night_times (1, display.ambilight_power_is_on, ...)".
# Statisch (statisch.txt): jede display_set_ambilight_power()-Stelle in main.c ruft danach in derselben
# Funktion dcf77_update () auf, und display_power_is_on wird in main.c nur in set_display_power() geschrieben.
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
mc = lies('main.c')
def zeile(t, a): return t[:a].count('\n') + 1
def block(t, start):
    k = t.index('{', start); tiefe = 0; i = k
    while True:
        c = t[i]
        if c == '{': tiefe += 1
        elif c == '}':
            tiefe -= 1
            if tiefe == 0: return t[start:i + 1]
        i += 1
def finde(t, s, n=1):
    if t.count(s) != n: sys.exit('extract: "%s" %d-mal in main.c, erwartet %d' % (s.strip()[:50], t.count(s), n))
    return t.index(s)
def fn(t, name, pflicht=True):
    k = t.find('\n' + name + ' (')
    if k < 0:
        if pflicht: sys.exit('extract: %s fehlt in main.c' % name)
        return None
    a = t.rindex('\n', 0, k) + 1
    b = t.index('\n}\n', k) + 3
    return '#line %d "main.c"\n' % zeile(t, a) + t[a:b]
out = []
du = fn(mc, 'dcf77_update', False)
out.append(du if du else '/* dcf77_update: im Stand nicht vorhanden */\n')
out.append(fn(mc, 'set_display_power'))
a = finde(mc, 'case DISPLAY_AMBILIGHT_POWER_NUM_VAR:') + len('case DISPLAY_AMBILIGHT_POWER_NUM_VAR:')
out.append('static void pf_setter (uint_fast8_t val)\n{\n    switch (0) { default:\n#line %d "main.c"\n%s\n    }\n}\n' % (zeile(mc, a), block(mc, a)))
a = finde(mc, 'if (night_check_night_times (1, display.ambilight_power_is_on')
out.append('static void pf_timer (void)\n{\n#line %d "main.c"\n    %s\n}\n' % (zeile(mc, a), block(mc, a)))
open(os.path.join(w, 'auszug-main.c'), 'w').write('\n'.join(out))
# statisch, ohne Kommentare
m = re.sub(r'/\*.*?\*/', lambda x: '\n' * x.group(0).count('\n'), mc, flags=re.S)
m = re.sub(r'//[^\n]*', '', m)
st = []
stellen = [x.start() for x in re.finditer(r'\bdisplay_set_ambilight_power\s*\(', m)]
for p in stellen:
    i = m.index('\n}\n', p)                              # Ende der umschliessenden Funktion
    if not re.search(r'\bdcf77_update\s*\(\s*\)', m[p:i]):
        st.append('display_set_ambilight_power() ohne nachfolgendes dcf77_update () (main.c:%d)' % zeile(m, p))
k = m.find('\nset_display_power ('); e = m.index('\n}\n', k)
for x in re.finditer(r'display\.(display_power_is_on|ambilight_power_is_on)\s*=[^=]', m):
    if not (k <= x.start() < e):
        st.append('%s ausserhalb von set_display_power() geschrieben (main.c:%d)' % (x.group(1), zeile(m, x.start())))
for x in re.finditer(r'\bdcf77_enabled\s*=[^=]', m):
    vor = m[:x.start()]
    if not re.search(r'\ndcf77_update \(void\)\n\{[^}]*$', vor) and not re.search(r'static volatile[^\n;]*$', vor):
        st.append('dcf77_enabled ausserhalb von dcf77_update() gesetzt (main.c:%d)' % zeile(m, x.start()))
info = ('Bestand: %d Stellen display_set_ambilight_power() in main.c' % len(stellen))
print(info)
open(os.path.join(w, 'statisch.txt'), 'w').write(''.join(s + '\n' for s in st))
print('extract: main.c (%sset_display_power, Setter, Timer), %d Ambilight-Stellen' % ('dcf77_update, ' if du else '', len(stellen)))
