# extract.py <src-Verzeichnis> <ausgabe.c>
# Zieht den ECHTEN Code: Zeilenparser aus esp8266.c, var_crc() und die Eroeffnungszeile aus vars.c,
# den Uebergabeblock esp8266_cmc_sync -> var_sync_pending aus main.c (fehlt er, MAIN_HOOK 0).
import sys, re
src, out = sys.argv[1], sys.argv[2]
def text(p): return open(p, 'rb').read().decode('ascii').replace('\r\n', '\n')
def fail(m): sys.stderr.write('extract: ' + m + '\n'); sys.exit(2)
def line(p, t, a): return '#line %d "%s"\n' % (t[:a].count('\n') + 1, p)

e = text(src + '/esp8266/esp8266.c')
s0, s1 = 'static uint_fast8_t     esp8266_cap_var_crc_seen', ' * send a command to ESP8266'
if e.count(s0) != 1 or e.count(s1) != 1: fail('Anker in esp8266.c nicht eindeutig')
a = e.index(s0); b = e.index(s1, a); b = e.rfind('\n', 0, b - 1) + 1
E = line('esp8266.c', e, a) + e[a:b]

v = text(src + '/vars/vars.c')
k = 'var_crc (const char * payload)\n{'
if v.count(k) != 1: fail('var_crc() nicht genau einmal')
a = v.rfind('\n', 0, v.index(k) - 1) + 1; b = v.index('\n}\n', a) + 3
C = line('vars.c', v, a) + v[a:b]

vb = [m for m in re.finditer(r'^.*sprintf \(frame, "VB.*$', v, re.M)]
VB = '#define VB_SPRINTF_COUNT %d\n' % len(vb)
if len(vb) == 1:
    VB += 'static void\nvb_frame (char * frame, unsigned int seq)\n{\n' + line('vars.c', v, vb[0].start()) + vb[0].group(0).replace('(unsigned int) seq)', 'seq)') + '\n}\n'

m = text(src + '/main.c')
h0 = '        if (esp8266_cmc_sync)\n'
if m.count(h0) == 1:
    a = m.index(h0); b = m.index('\n        }\n', a) + 11
    H = '#define MAIN_HOOK 1\nstatic void\nmain_hook (void)\n{\n' + line('main.c', m, a) + m[a:b] + '}\n'
else:
    H = '#define MAIN_HOOK 0\n'
open(out, 'w').write(C + E + VB + H)
print('extrahiert aus', src, ':', E.count('\n'), '+', C.count('\n'), 'Zeilen,', H.split('\n')[0])
