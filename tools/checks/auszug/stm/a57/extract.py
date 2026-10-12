# Pruefstand A57 (Paket 2026-10-11, S3.8): "Messwert uebernehmen" weist eine Inversion ab (Ent-7 = C).
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis> [sabotage]
# WOERTLICH: ldr_write_config_to_eep, ldr_set_min_value, ldr_set_max_value (ldr.c); aus main.c die beiden
# RPC-Zweige samt Rueckmeldung und den Zahlen-Setter (A59).
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
sab = sys.argv[3] if len(sys.argv) > 3 else ''
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
c = lies('ldr/ldr.c'); m = lies('main.c')
SAB = {
    'gleichheit': ('        if (adc_value >= ldr.ldr_max_value)\n', '        if (adc_value > ldr.ldr_max_value)\n'),
    'maxseite':   ('        if (adc_value <= ldr.ldr_min_value)', '        if (adc_value < ldr.ldr_min_value)'),
    'ohnelog':    ('            log_printf ("ldr: minimum abgewiesen (%u >= max %u)\\r\\n", (unsigned int) adc_value, (unsigned int) ldr.ldr_max_value);\n', '            ;\n'),
    'eeprom':     ('            log_printf ("ldr: maximum abgewiesen (%u <= min %u)\\r\\n", (unsigned int) adc_value, (unsigned int) ldr.ldr_min_value);\n',
                   '            log_printf ("ldr: maximum abgewiesen (%u <= min %u)\\r\\n", (unsigned int) adc_value, (unsigned int) ldr.ldr_min_value);\n            ldr_write_config_to_eep ();\n'),
}
if sab:
    if sab not in SAB or c.count(SAB[sab][0]) != 1: sys.exit('extract: Sabotage %s -- Anker fehlt' % sab)
    c = c.replace(*SAB[sab])
def fn(t, kopf, wo):
    a = t.index(kopf); b = t.index('\n}\n', a) + 3
    return '#line %d "%s"\n' % (t[:a].count('\n') + 1, wo) + t[a:b]
teile = [fn(c, 'uint_fast8_t\nldr_write_config_to_eep (void)', 'ldr.c'),
         fn(c, 'uint_fast16_t\nldr_set_min_value (void)', 'ldr.c'),
         fn(c, 'uint_fast16_t\nldr_set_max_value (void)', 'ldr.c')]
a = m.index('        case LDR_MIN_VALUE_RPC_VAR:'); b = m.index('        case LEARN_IR_RPC_VAR:', a)
rpc = m[a:b]
a = m.index('        case LDR_MIN_VALUE_NUM_VAR:'); b = m.index('        case TIMEZONE_NUM_VAR:', a)
num = m[a:b]
teile.append('static void rpc (int var_idx)\n{\n    switch (var_idx)\n    {\n' + rpc + '    }\n}\n')
teile.append('static void zahl (int var_idx, uint_fast16_t val)\n{\n    switch (var_idx)\n    {\n' + num + '    }\n}\n')
open(os.path.join(w, 'auszug.c'), 'w').write('\n'.join(teile))
print('extract: ldr.c (3 Funktionen), main.c (RPC, Zahlen-Setter)%s' % ((', Sabotage ' + sab) if sab else ''))
