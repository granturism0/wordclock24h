import sys
t = open(sys.argv[1], 'rb').read().decode('latin-1').replace('\r\n', '\n')
def span(k):
    a = t.rfind('\n', 0, t.index(k) - 1) + 1; b = t.index('\n}\n', a) + 3
    return '#line %d "main.c"\n' % (t[:a].count('\n') + 1) + t[a:b]
a = t.index('static uint32_t     esp8266_cmd_reject_cnt'); 
open(sys.argv[2], 'w').write(t[a:t.index('\n', a) + 1] + span('esp8266_idx_ok (uint_fast8_t idx, uint_fast8_t limit, const char * was)\n{') +
                             span('esp8266_cmd_reject (const char * line, uint_fast8_t have, uint_fast8_t want)\n{'))
