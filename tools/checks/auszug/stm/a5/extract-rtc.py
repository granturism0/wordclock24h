# Schneidet rtc_get_temperature_index() aus rtc.c (ISO-8859-1, deshalb latin-1 gelesen).
import sys
t = open(sys.argv[1], 'rb').read().decode('latin-1').replace('\r\n', '\n')
a = t.index('uint_fast8_t\nrtc_get_temperature_index (void)\n{')
b = t.index('\n}\n', a) + 3
open(sys.argv[2], 'w').write('#line %d "rtc.c"\n' % (t[:a].count('\n') + 1) + t[a:b])
