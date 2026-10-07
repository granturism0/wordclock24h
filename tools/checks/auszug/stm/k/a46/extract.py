# extract.py <main.c> <tftled.c> <esp-spiffs.c> <base.c> <aus.c>
import sys, re
def rd(p): return open(p, 'rb').read().decode('latin-1').replace('\r\n', '\n')
def span(t, a, name):
    b = t.index('\n}\n', a) + 3
    return '#line %d "%s"\n' % (t[:a].count('\n') + 1, name) + t[a:b]
m, tf, es, ba = [rd(p) for p in sys.argv[1:5]]
out = []
a = ba.index('uint16_t\nhtoi ('); out.append(span(ba, a, 'base.c'))
k = m.index('esp8266_cmd_reject (const char * line, uint_fast8_t have, uint_fast8_t want)\n{')
a = m.rfind('\n', 0, k - 1) + 1                         # Zeile davor: "static void" bzw. "void ... // A46"
out.append(span(m, a, 'main.c'))
a = tf.index('void\ntftled_layout_get_line ('); out.append(span(tf, a, 'tftled.c'))
a = es.index('static int  icon_block;'); out.append('#line %d "esp-spiffs.c"\n' % (es[:a].count('\n') + 1) + es[a:es.index('\n', a) + 1])
a = es.index('#define ICON_HEAD_LEN'); out.append(es[a:es.index('\n', a) + 1])
a = es.index('uint_fast8_t\nesp_diffs_read_icon ('); out.append(span(es, a, 'esp-spiffs.c'))
open(sys.argv[5], 'w').write('\n'.join(out))
