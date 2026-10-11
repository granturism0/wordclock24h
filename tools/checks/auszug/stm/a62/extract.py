# Pruefstand A62 (Paket 2026-10-11, S1.1): bereitet einen Stand der Firmware fuer a62.c auf.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis> [sabotage]
# Schreibt NUR ins Arbeitsverzeichnis: timeserver.c/.h (ggf. sabotiert), zweig.c (der
# ESP8266_TIME-Zweig aus main.c, woertlich), gm.c (my_gmtime aus base.c), stub/*.h, flags.
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
sab = sys.argv[3] if len(sys.argv) > 3 else ''
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
def ersetze(t, alt, neu, wo):
    if t.count(alt) != 1:
        sys.exit('extract: Sabotage %s -- Anker %d-mal in %s' % (sab, t.count(alt), wo))
    return t.replace(alt, neu)

ts = lies('timeserver/timeserver.c'); th = lies('timeserver/timeserver.h'); mc = lies('main.c')
SAB_TS = {
    'totbereich':   (' || n - 2208988800U < 1767225600U', ''),
    'zehn':         ('if (i == 0 ||', 'if (i != 10 ||'),
    'bestaetigung': ('(net_time.ref_ok  && ', '(1 || '),
    'abstand':      ('up - net_time.cand_up >= 10 &&', ''),
    'ueberlauf':    (' || n > 429496729U || (n == 429496729U && d > 5)', ''),
    'schnell':      ('net_time.quick < 3', 'net_time.quick < 4'),
    'toleranz':     ('+ 5) <= 10', '+ 6) <= 12'),
    'zeitzone':     ('if (tz[1] > 12)', 'if (0)'),
}
SAB_MAIN = {
    'countdown':    ('net_time_countdown = 15;', ';'),
}
if sab in SAB_TS:   ts = ersetze(ts, *SAB_TS[sab], wo='timeserver.c')
elif sab in SAB_MAIN: mc = ersetze(mc, *SAB_MAIN[sab], wo='main.c')
elif sab:           sys.exit('extract: unbekannte Sabotage ' + sab)

a = mc.index('        case ESP8266_TIME:'); b = mc.index('        case ESP8266_TABLES:', a)
zweig = '#line %d "main.c"\n' % (mc[:a].count('\n') + 1) + mc[a:b]

bc = lies('base/base.c')
g1 = bc.index('#define IS_LEAP_YEAR'); g2 = bc.index('\n', bc.index('g_days_per_month[]')) + 1
f = bc.index('struct tm *\nmy_gmtime'); g = bc.index('\n}\n', f) + 3
gm = bc[g1:g2] + '#line %d "base.c"\n' % (bc[:f].count('\n') + 1) + bc[f:g]

eh = lies('esp8266/esp8266.h')
tl = re.search(r'#define\s+ESP8266_MAX_TIME_LEN\s+(\d+)', eh).group(1)

os.makedirs(os.path.join(w, 'stub'), exist_ok=True)
def schreib(p, t): open(os.path.join(w, p), 'w').write(t)
schreib('timeserver.c', '#line 1 "timeserver.c"\n' + ts)
schreib('timeserver.h', th)
schreib('zweig.c', zweig)
schreib('gm.c', '#include <stdint.h>\n#include <time.h>\n#ifdef PRUEFSTAND_SCHNELLE_TYPEN\n#include "pruefstand.h"\n#endif\n' + gm)
schreib('stub/esp8266.h', '''#ifndef STUB_ESP8266_H
#define STUB_ESP8266_H
#include <stdint.h>
#define ESP8266_TIME            7
#define ESP8266_TABLES          8
#define ESP8266_MAX_TIME_LEN    %s
typedef struct { union { char time[ESP8266_MAX_TIME_LEN + 1]; } u; uint_fast8_t is_online; } STUB_ESP8266;
extern STUB_ESP8266 esp8266;
extern void esp8266_send_cmd (const char *, const char *, uint_fast8_t);
#endif
''' % tl)
schreib('stub/base.h', '#include <time.h>\nextern struct tm * my_gmtime (time_t * tvp);\n')
schreib('stub/eep.h', '#include <stdint.h>\nextern uint_fast8_t eep_is_up;\nextern uint_fast8_t eep_read (uint_fast16_t, uint8_t *, uint_fast16_t);\nextern uint_fast8_t eep_write (uint_fast16_t, uint8_t *, uint_fast16_t);\n')
schreib('stub/eeprom-data.h', '#define EEPROM_MAX_IPADDR_LEN 16\n#define EEPROM_MAX_TIMEZONE_LEN 2\n#define EEPROM_DATA_SIZE_TIMESERVER 16\n#define EEPROM_DATA_SIZE_TIMEZONE 2\n#define EEPROM_DATA_OFFSET_TIMESERVER 100\n#define EEPROM_DATA_OFFSET_TIMEZONE 116\n')
schreib('stub/log.h', '''#include <stdarg.h>
extern void stub_log (const char *);
extern void log_printf (const char *, ...);
#define log_message(s)              stub_log (s)
#define debug_log_message(s)
#define debug_log_printf(fmt,...)
''')
flags = []
if 'timeserver_check_net_time' in ts: flags.append('-DHAVE_A62')
schreib('flags', ' '.join(flags))
# Statische Befunde: strtoul im Stand? Neue 64-Bit-Typen in der Pruefung?
st = []
def ohne_kommentar(t): return re.sub(r'/\*.*?\*/|//[^\n]*', '', t, flags=re.S)
for name, t in (('main.c', mc), ('timeserver.c', ts)):
    if re.search(r'\bstrtoul\s*\(', ohne_kommentar(t)): st.append('strtoul-Aufruf in ' + name)
if 'timeserver_check_net_time (const char' in ts:
    a = ts.index('timeserver_check_net_time (const char'); b = ts.index('\n}\n', a)
    for tok in ('int64', 'uint64', 'long long', 'time_t', 'unsigned long'):
        if tok in ts[a:b]: st.append('64-Bit-Verdacht in der Pruefung: ' + tok)
schreib('statisch', '\n'.join(st) + ('\n' if st else ''))
