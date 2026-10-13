# Pruefstand A70 (Paket 2026-10-13, S2.11): bereitet einen Firmwarestand fuer a70.c auf. Grundlage: a69/extract.py.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis>
# Schreibt NUR ins Arbeitsverzeichnis. WOERTLICH aus main.c: Vormerkung (Kopf), set_display_power(), night_wm,
# night_apply(), night_off_apply(), start_ticker() (falls vorhanden), Startblock nach "read rtc", Hauptloop
# (IP-Lauftext, Frist, Restore-Bedingung), Minutenzweig (Displayteil), im TIME-Zweig der Block
# "if (night_wm == 0xFFFF)", der ganze DCF77-Block "if (dcf77_time(&gmain.tm))" und der Zweig ESP8266_WEATHER.
# Dazu statisch: Bestandsliste der Tickerstarts (AKN.3), Restore-Bedingung woertlich (AKN.5), TIME-Zweig.
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
nc = lies('night/night.c'); nh = lies('night/night.h'); mc = lies('main.c')
def zl(a): return '#line %d "main.c"\n' % (mc[:a].count('\n') + 1)
def block(t, start):
    k = t.index('{', start); tiefe = 0; i = k
    while True:
        c = t[i]
        if c == '{': tiefe += 1
        elif c == '}':
            tiefe -= 1
            if tiefe == 0: return t[start:i + 1]
        i += 1
def finde(s, n=1):
    if mc.count(s) != n: sys.exit('extract: "%s" %d-mal in main.c, erwartet %d' % (s.strip()[:60], mc.count(s), n))
    return mc.index(s)
def fn(name, pflicht=True):
    k = mc.find('\n' + name + ' (')
    while k >= 0 and mc[mc.index('\n', k + 1) - 1] == ';':        # Prototyp ueberspringen
        k = mc.find('\n' + name + ' (', k + 1)
    if k < 0:
        if pflicht: sys.exit('extract: %s fehlt in main.c' % name)
        return None
    a = mc.rindex('\n', 0, k) + 1; b = mc.index('\n}\n', k) + 3
    return zl(a) + mc[a:b]

kopf = []
a = finde('static uint_fast8_t\nset_display_power (')
x = finde('/* A69/M2 (Review S1.16'); y = mc.index('\n', finde('static uint32_t                 night_off_due')) + 1
kopf.append(zl(x) + mc[x:y])
if 'night_off_ip' not in mc[x:y]:
    kopf.append('static uint_fast8_t night_off_ip; static uint32_t ip_ticker_end;   /* Stand ohne A70 */\n')
    if 'NIGHT_OFF_AFTER_IP' not in mc[x:y]: sys.exit('extract: weder night_off_ip noch NIGHT_OFF_AFTER_IP')
kopf.append(fn('set_display_power').replace('set_display_power (uint_fast8_t', 'set_display_power_echt (uint_fast8_t', 1))   # a70.c legt einen zaehlenden Mantel darum
teile = []
g = re.search(r'^static uint_fast16_t\s+night_wm\b[^\n]*\n', mc, flags=re.M)
if not g: sys.exit('extract: night_wm fehlt')
teile.append(g.group(0))
teile.append(fn('night_apply'))
teile.append(fn('night_off_apply'))
st_fn = fn('start_ticker', False)
teile.append(st_fn if st_fn else 'static void start_ticker (const unsigned char * t) { display_set_ticker (t, 0); pending_ticker_restore = 1; }   /* Stand ohne A70: wie vorher */\n')
# Startblock nach "read rtc"
r = mc.index('log_printf ("read rtc: %s'); r = mc.index('gmain.tm.tm_sec);\n', r) + len('gmain.tm.tm_sec);\n')
re_ = mc.index('    }\n\n    main_set_ambilight_clock_wait_cycles', r)
teile.append('static void startblock (void)\n{\n' + zl(r) + mc[r:re_] + '\n}\n')
# Hauptloop
h1 = finde('        if (ip_ticker_pending && ! var_sync_pending)'); h2 = mc.index('        /* Abzug der IR-Codes', h1)
h3 = finde('        if (pending_ticker_restore &&'); h4 = mc.index('        if (display_clock_flag)', h3)
teile.append('static void hauptloop_teile (void)\n{\n' + zl(h1) + mc[h1:h2] + zl(h3) + mc[h3:h4] + '}\n')
# Minutenzweig, Displayteil
e0 = mc.index('            if (night_check_night_times (1, display.ambilight_power_is_on')
e = mc.index('            if (display_clock_flag == 0)', e0)
s = mc.rfind('            if (night_wm != 0xFFFF)', 0, e)
teile.append('static void minutenzweig (void)\n{\n' + zl(s) + mc[s:e] + '}\n')
# TIME-Zweig: Block "if (night_wm == 0xFFFF)" hinter TIMESERVER_NET_TIME_OK
c = finde('case ESP8266_TIME:'); ce = mc.index('case ESP8266_TABLES:', c)
t = mc.index('if (night_wm == 0xFFFF)', mc.index('TIMESERVER_NET_TIME_OK', c))
if t > ce: sys.exit('extract: TIME-Zweig ohne night_wm-Block')
teile.append('static void time_erst (void)\n{\n' + zl(t) + '                ' + block(mc, t) + '\n}\n')
# DCF77-Block ganz
d = finde('if (dcf77_time(&gmain.tm))')
teile.append('static void dcf77_block (void)\n{\n' + zl(d) + '        ' + block(mc, d) + '\n}\n')
# Wetter
wz = finde('case ESP8266_WEATHER:') + len('case ESP8266_WEATHER:')
teile.append('static void pf_weather (void)\n{\n    switch (0) { default:\n' + zl(wz) + block(mc, wz) + '\n    }\n}\n')

os.makedirs(os.path.join(w, 'stub'), exist_ok=True)
def schreib(p, t): open(os.path.join(w, p), 'w').write(t)
schreib('night.c', '#line 1 "night.c"\n' + nc)
schreib('night.h', nh)
schreib('main-a70.c', ''.join(kopf) + ''.join(teile))
schreib('stub/eep.h', '#include <stdint.h>\nextern uint_fast8_t eep_is_up;\nextern uint_fast8_t eep_read (uint_fast16_t, unsigned char *, uint_fast16_t);\nextern uint_fast8_t eep_write (uint_fast16_t, unsigned char *, uint_fast16_t);\n')
schreib('stub/eeprom-data.h', '#define EEPROM_VERSION_1_7 17\n#define EEPROM_VERSION_2_5 25\n#define EEPROM_DATA_SIZE_NIGHT_TIME 24\n#define EEPROM_DATA_SIZE_AMBI_NIGHT_TIME 24\n#define EEPROM_DATA_OFFSET_NIGHT_TIME 0\n#define EEPROM_DATA_OFFSET_AMBI_NIGHT_TIME 24\n')
schreib('stub/log.h', '#include <stdarg.h>\nextern void log_printf (const char *, ...);\n#define log_message(s) log_printf ("%s\\r\\n", s)\n#define debug_log_message(s)\n#define debug_log_printf(fmt,...)\n')

# ---- statisch (ohne Kommentare, Zeilen erhalten) ----
m = re.sub(r'/\*.*?\*/', lambda q: '\n' * q.group(0).count('\n'), mc, flags=re.S)
m = re.sub(r'//[^\n]*', '', m)
def mz(p): return m[:p].count('\n') + 1
st = []; info = []
# AKN.3 Bestandsliste: jeder display_set_ticker()-Aufruf in main.c steht in start_ticker() oder im IP-Lauftext
sk = m.find('\nstart_ticker (const unsigned char'); se = m.index('\n}\n', sk) if sk >= 0 else -1
ip = m.index('if (ip_ticker_pending && ! var_sync_pending)'); ipe = ip + len(block(m, ip))
aufrufe = [q.start() for q in re.finditer(r'\bdisplay_set_ticker\s*\(', m)]
for p in aufrufe:
    if sk >= 0 and sk < p < se: continue
    if ip < p < ipe: continue
    st.append('AKN.3: Tickerstart an start_ticker() vorbei (main.c:%d)' % mz(p))
n_st = len(re.findall(r'\bstart_ticker\s*\([^)]', m)) - (1 if sk >= 0 else 0) - len(re.findall(r'static void\s+start_ticker\s*\(const unsigned char \*\);', m))
info.append('Bestand: %d display_set_ticker()-Aufrufe in main.c, %d Aufrufe von start_ticker()' % (len(aufrufe), n_st))
if sk >= 0 and n_st != 5: st.append('AKN.3: %d Aufrufe von start_ticker(), erwartet 5 (ticker_set, Wetter, Vorhersage, Overlay, Datum)' % n_st)
# AKN.5: Restore-Bedingung woertlich; Innenzweig prueft den Merker
rb = block(mc, finde('if (pending_ticker_restore &&'))
k = rb.find('if (! display_clock_flag)')
schreib('restore.txt', rb[:rb.index('\n', k)] + '\n' if k >= 0 else '')
inner = re.sub(r'//[^\n]*', '', rb[k:])
if not re.search(r'display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;\s*if \(night_off_ip\)\s*\{\s*night_off_apply \(\);\s*\}\s*\}\s*\}$', inner):
    st.append('AKN.5: Innenzweig der Restore-Bedingung prueft nicht "if (night_off_ip) night_off_apply ();"')
# TIME-Zweig
tb = m[m.index('case ESP8266_TIME:'):m.index('case ESP8266_TABLES:')]
if not re.search(r'if \(night_wm == 0xFFFF\)\s*\{\s*night_apply \(0xFFFF, [^;]*\);\s*\}', tb[tb.find('TIMESERVER_NET_TIME_OK'):]):
    st.append('TIME: night_apply (0xFFFF, ...) nicht im Uebernahmezweig hinter night_wm == 0xFFFF')
if 'NIGHT_OFF_AFTER_IP' in m: st.append('(c): NIGHT_OFF_AFTER_IP noch im Code (design.md 4.1: entfaellt)')
schreib('statisch', ''.join(q + '\n' for q in st))
schreib('info', ''.join(q + '\n' for q in info))
