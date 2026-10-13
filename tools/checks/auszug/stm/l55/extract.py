# Pruefstand l55 (Paket 2026-10-13, S2.1, B55-STM): "Messwert uebernehmen" meldet RPCR xx r.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis>
# WOERTLICH: ldr_write_config_to_eep, ldr_set_min_value, ldr_set_max_value (ldr.c); der RPC-Zweig beider
# Indizes aus main.c; esp8266_send_cmd (esp8266.c, damit die Bytes auf der Leitung die echten sind);
# die Aufzaehlung RPC_VARIABLE (vars.h, damit xx die echten Indizes traegt).
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
c = lies('ldr/ldr.c'); m = lies('main.c'); e = lies('esp8266/esp8266.c'); v = lies('vars/vars.h')
def fn(t, name, wo):
    k = t.index('\n' + name + ' (')           # Typzeile davor, egal welcher Typ
    a = t.rindex('\n', 0, k) + 1; b = t.index('\n}\n', k) + 3
    return '#line %d "%s"\n' % (t[:a].count('\n') + 1, wo) + t[a:b]
teile = [fn(c, 'ldr_write_config_to_eep', 'ldr.c'), fn(c, 'ldr_set_min_value', 'ldr.c'),
         fn(c, 'ldr_set_max_value', 'ldr.c'), fn(e, 'esp8266_send_cmd', 'esp8266.c')]
a = v.index('typedef enum\n{\n    LDR_MIN_VALUE_RPC_VAR'); b = v.index('} RPC_VARIABLE;', a) + 15
teile.insert(0, v[a:b] + '\n')
a = m.index('        case LDR_MIN_VALUE_RPC_VAR:'); b = m.index('        case LEARN_IR_RPC_VAR:', a)
teile.append('static void rpc (uint_fast8_t var_idx)\n{\n    switch (var_idx)\n    {\n' + m[a:b] + '    }\n}\n')
open(os.path.join(w, 'auszug.c'), 'w').write('\n'.join(teile))
print('extract: vars.h (RPC_VARIABLE), ldr.c (3 Funktionen), esp8266.c (send_cmd), main.c (RPC-Zweig)')
