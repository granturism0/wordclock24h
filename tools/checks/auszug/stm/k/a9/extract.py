import sys
t = open(sys.argv[1], 'rb').read().decode('latin-1').replace('\r\n', '\n')
a = t.index('#define MAX_LDR_BRIGHTNESS'); b = t.index('\n}\n', t.index('ldr_poll_brightness  (void)')) + 3
open(sys.argv[2], 'w').write('#line %d "ldr.c"\n' % (t[:a].count('\n') + 1) + t[a:b])
