# Schneidet IS_LEAP_YEAR, g_days_per_month und my_gmtime() aus base.c aus.
# Aufruf: extract.py base.c ziel.c [sabotage]
import sys
t = open(sys.argv[1], 'rb').read().decode('latin-1').replace('\r\n', '\n')
a = t.index('#define IS_LEAP_YEAR'); a2 = t.index('\n', t.index('g_days_per_month[]')) + 1
f = t.index('struct tm *\nmy_gmtime'); g = t.index('\n}\n', f) + 3
kopf = t[a:a2]
if len(sys.argv) > 3 and sys.argv[3] == 'sabotage':
    alt = '((((y) % 4) == 0) && ((((y) % 100) != 0) || (((y) % 400) == 0)))'
    assert kopf.count(alt) == 1
    kopf = kopf.replace(alt, '(((y) % 4) == 0)')          # Jahrhundertregel entfernt
out = '#line %d "base.c"\n' % (t[:a].count('\n') + 1) + kopf + '#line %d "base.c"\n' % (t[:f].count('\n') + 1) + t[f:g]
open(sys.argv[2], 'w').write(out)
