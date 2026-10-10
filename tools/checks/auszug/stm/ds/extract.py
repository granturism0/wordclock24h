# Pruefstand DS (A2, Paket 2026-10-10): schneidet die ECHTEN Stellen aus ds18xx.c/.h, tempsensor.c/.h, onewire.c, main.c.
# Aufruf: extract.py <src-Verzeichnis> <ausgabe.c> [sabotage]
# sabotage = "poly": ersetzt das Polynom 0x8C in ds18xx_crc8 () durch 0x8D (DIR-014).
import sys, re
src, out = sys.argv[1], sys.argv[2]
sab = sys.argv[3] if len(sys.argv) > 3 else ''
def rd(p): return open(p, 'rb').read().decode('ascii').replace('\r\n', '\n')
def one(t, s, what):
    assert t.count(s) == 1, (what, s, t.count(s)); return t.index(s)
def line(t, a, path): return '#line %d "%s"\n' % (t[:a].count('\n') + 1, path)
def func(t, head, path):
    a = one(t, head, path); b = t.index('\n}\n', a) + 3
    return line(t, a, path) + t[a:b]
ds, dh = rd(src + '/ds18xx/ds18xx.c'), rd(src + '/ds18xx/ds18xx.h')
tc, th = rd(src + '/tempsensor/tempsensor.c'), rd(src + '/tempsensor/tempsensor.h')
ow, m = rd(src + '/onewire/onewire.c'), rd(src + '/main.c')
o = []
a = one(dh, '#define DS_RESOLUTION_9_BIT', 'ds18xx.h'); b = dh.index('extern DS18XX_GLOBALS', a)
o.append(line(dh, a, 'ds18xx.h') + dh[a:b])
a = one(th, 'typedef struct', 'tempsensor.h'); b = th.index('} TEMP_GLOBALS;', a) + 15
o.append(line(th, a, 'tempsensor.h') + th[a:b] + '\n')
o.append(func(ow, 'uint_fast8_t\nonewire_get_rom_code (uint8_t * rom_code)', 'onewire.c'))
a = one(ds, '#define ONEWIRE_CONVERT_CMD', 'ds18xx.c')
body = ds[a:]
if sab == 'poly':
    n = body.count('^ 0x8C')
    assert n == 1, ('Sabotage-Anker', n)
    body = body.replace('^ 0x8C', '^ 0x8D')
o.append(line(ds, a, 'ds18xx.c') + body)
a = one(tc, 'TEMP_GLOBALS    gtemp =', 'tempsensor.c'); b = tc.index('};', a) + 3
o.append(line(tc, a, 'tempsensor.c') + tc[a:b])
o.append(func(tc, 'uint_fast8_t\ntemp_start_conversion (uint_fast8_t do_wait)', 'tempsensor.c'))
o.append(func(tc, 'uint_fast8_t\ntemp_read_temp_index (void)', 'tempsensor.c'))
o.append(func(tc, 'uint_fast8_t\ntemp_init (void)', 'tempsensor.c'))
a = one(m, '        if (measure_temperature_flag)\n        {\n', 'main.c'); b = one(m, '        if (read_rtc_temperature_flag)\n', 'main.c')
k = m.rfind('        uint_fast8_t ds18xx_was_up = ds18xx.is_up;\n', 0, a)
if k >= 0 and m.count('ds18xx_was_up = ') == 1:
    a = k                                                   # Sparvorschlag 3: Merkvariable gehoert zum Block
o.append('static void\nds_main_step (void)\n{\n' + line(m, a, 'main.c') + m[a:b] + '}\n')
o.insert(0, '#define HAVE_READ_ERRORS %d\n#define HAVE_CRC8 %d\n' % ('read_errors' in th, 'ds18xx_crc8' in ds))
open(out, 'w').write('\n'.join(o))
print('extrahiert aus', src, ': ds18xx.c', body.count('\n'), 'Zeilen, main.c', m[a:b].count('\n'), 'Zeilen', ('SABOTAGE ' + sab) if sab else '')
