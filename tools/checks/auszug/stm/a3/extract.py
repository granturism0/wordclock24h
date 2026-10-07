# extract.py <main.c> <overlay.c> <base.c> <aus.c>: schneidet die echten Stellen fuer A3/A48/A51 aus.
import sys, re
def rd(p): return open(p, 'rb').read().decode('latin-1').replace('\r\n', '\n')
def func(t, head):
    a = t.index(head); b = t.index('\n}\n', a) + 3
    return '#line %d "%s"\n' % (t[:a].count('\n') + 1, head.split('(')[0].split()[-1]) + t[a:b]
m, o, b = rd(sys.argv[1]), rd(sys.argv[2]), rd(sys.argv[3])
out = []
out.append(func(b, 'uint16_t\nhtoi ('))
out.append(func(o, 'uint_fast8_t\noverlay_read_config_from_eep ('))
out.append(func(o, 'static void\noverlay_save_n_overlays ('))
out.append(func(o, 'void\noverlay_set_n_overlays ('))
a = m.index('static uint32_t     esp8266_cmd_reject_cnt'); e = m.index('\n}\n', a) + 3
out.append(m[a:e])
c = m.index('        case OVERLAY_N_OVERLAYS_NUM_VAR:\n'); c = m.index('        {\n', c); d = m.index('\n        }\n', c) + 11
out.append('static void\na3_case (uint_fast16_t val)\n{\n    switch (0)\n    {\n    default:\n' + m[c:d] + '    }\n}\n')
out.append(func(m, 'static void\nschedule_esp8266_overlay ('))
open(sys.argv[4], 'w').write('\n'.join(out))
