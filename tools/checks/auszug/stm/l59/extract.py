# Pruefstand L59 (A59, Paket 2026-10-10): schneidet die ECHTEN Stellen aus main.c, ldr.c, ldr.h.
# Aufruf: extract.py <src-Verzeichnis> <ausgabe.c>
import sys, re
src, out = sys.argv[1], sys.argv[2]
def rd(p): return open(p, 'rb').read().decode('ascii').replace('\r\n', '\n')
def one(t, s, what):
    assert t.count(s) == 1, (what, s, t.count(s)); return t.index(s)
def func(t, head, path):
    a = one(t, head, path); b = t.index('\n}\n', a) + 3
    return '#line %d "%s"\n' % (t[:a].count('\n') + 1, path) + t[a:b]
m, c, h = rd(src + '/main.c'), rd(src + '/ldr/ldr.c'), rd(src + '/ldr/ldr.h')
o = []
k = re.search(r'^#define LDR_MAX_RAW_VALUE[^\n]*\n', h, re.M)
o.append(k.group(0) if k else '/* ldr.h dieses Stands kennt LDR_MAX_RAW_VALUE nicht */\n')
a = one(c, '#define MAX_LDR_BRIGHTNESS', 'ldr.c'); b = c.index('\n};\n', a) + 4
o.append('#line %d "ldr.c"\n' % (c[:a].count('\n') + 1) + c[a:b])
o.append(func(c, 'uint_fast8_t\nldr_poll_brightness  (void)', 'ldr.c'))
o.append(func(c, 'void\nldr_read_config_from_eep (uint32_t eep_version)', 'ldr.c'))
o.append(func(c, 'uint_fast8_t\nldr_write_config_to_eep (void)', 'ldr.c'))
a = one(m, '        case LDR_MIN_VALUE_NUM_VAR:\n', 'main.c'); b = one(m, '        case TIMEZONE_NUM_VAR:', 'main.c')
body = m[a:b]
# statisch: in den beiden Zweigen kein eigener EEPROM-Schreibweg (AKL.1 "keine zweite Schreibfunktion")
fremd = len(re.findall(r'\beep_write\b|\beeprom_write\b|_write_config_to_eep\b', body)) - body.count('ldr_write_config_to_eep')
o.append('#define L59_FREMDE_SCHREIBWEGE %d\n' % fremd)
o.append('static void\nl59_case (uint_fast8_t var_idx, uint_fast16_t val)\n{\n    switch (var_idx)\n    {\n'
         + '#line %d "main.c"\n' % (m[:a].count('\n') + 1) + body + '        default: break;\n    }\n}\n')
open(out, 'w').write('\n'.join(o))
print('extrahiert aus', src, ': Zweige', body.count('\n'), 'Zeilen, fremde Schreibwege', fremd)
