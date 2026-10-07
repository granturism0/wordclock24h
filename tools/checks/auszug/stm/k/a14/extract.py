import sys
t = open(sys.argv[1], 'rb').read().decode('latin-1').replace('\r\n', '\n')
out = []
for head in ('static void\nvar_send_byte (', 'static void\nvar_send_short (', 'static void\nvar_send_string ('):
    a = t.index(head); b = t.index('\n}\n', a) + 3
    out.append('#line %d "vars.c"\n' % (t[:a].count('\n') + 1) + t[a:b])
open(sys.argv[2], 'w').write('\n'.join(out))
