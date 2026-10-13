# Pruefstand A69 (Paket 2026-10-11, S1.3): bereitet einen Firmwarestand fuer a69.c auf.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis> [sabotage]
# Schreibt NUR ins Arbeitsverzeichnis: night.c/night.h (ggf. sabotiert), main-a69.c (night_wm,
# night_apply() und der Displayteil des Minutenzweigs, woertlich aus main.c), stub/*.h, flags, statisch.
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
sab = sys.argv[3] if len(sys.argv) > 3 else ''
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
def ersetze(t, alt, neu, wo):
    if t.count(alt) != 1: sys.exit('extract: Sabotage %s -- Anker %d-mal in %s' % (sab, t.count(alt), wo))
    return t.replace(alt, neu)
nc = lies('night/night.c'); nh = lies('night/night.h'); mc = lies('main.c')
SAB_N = {
    'kleinergleich': ('(k || ntp->minutes <= m)', '(k || ntp->minutes < m)'),
    'umlauf':        ('(wday + 7 - k) % 7', '(wday - k) % 7'),
    'tag1440':       ('ntp->minutes < 24 * 60 && ', ''),
    'erstertag':     ('for (k = 0; k < 8; k++)', 'for (k = 1; k < 8; k++)'),
}
SAB_M = {
    'intervall':     ('back < skipped', 'back <= skipped'),
    'startein':      ('! (at_start && on)', '1'),
    'vorwaerts':     ('d < 7 * 12 * 60', 'd < 7 * 24 * 60'),
    'jahr':          ('if (gmain.year >= 2026)', 'if (1)'),
    'iplauf':        ('            night_off_due = ip_ticker_end;\n', ''),
    'verwerfen':     ('    night_off_due = 0;                                                      // A69/M2', '    ;                                                      // A69/M2'),
}
if sab in SAB_N: nc = ersetze(nc, *SAB_N[sab], wo='night.c')
elif sab in SAB_M: mc = ersetze(mc, *SAB_M[sab], wo='main.c')
elif sab: sys.exit('extract: unbekannte Sabotage ' + sab)

teile = []; flags = []
# set_display_power() woertlich (A68: Sendeprotokoll), umbenannt -- a69.c legt einen zaehlenden Mantel darum
a = mc.index('static uint_fast8_t\nset_display_power ('); b = mc.index('\n}\n', a) + 3
spd = mc[a:b].replace('set_display_power (uint_fast8_t', 'set_display_power_echt (uint_fast8_t', 1)
g = re.search(r'^static uint_fast16_t\s+night_wm\b[^\n]*\n', mc, flags=re.M)
teile_kopf = '#line %d "main.c"\n' % (mc[:a].count('\n') + 1) + spd
# M2: Vormerkung (Merker und Konstanten vor set_display_power), sonst Ersatz fuer alte Staende
if 'static uint32_t                 night_off_due' in mc:
    x = mc.index('/* A69/M2 (Review S1.16'); y = mc.index('\n', mc.index('static uint32_t                 night_off_due')) + 1
    teile_kopf = '#line %d "main.c"\n' % (mc[:x].count('\n') + 1) + mc[x:y] + teile_kopf
else:
    teile_kopf = '#define NIGHT_OFF_AFTER_IP 0xFFFFFFFF\nstatic uint32_t night_off_due = 0;   /* Stand ohne M2 */\n' + teile_kopf
if g: teile.append(g.group(0))
else: teile.append('static uint_fast16_t night_wm = 0xFFFF;   /* Stand ohne A69 */\n')
if '\nnight_apply (uint_fast16_t skipped' in mc:
    a = mc.index('static void\nnight_apply ('); b = mc.index('\n}\n', a) + 3
    teile.append('#line %d "main.c"\n' % (mc[:a].count('\n') + 1) + mc[a:b])
    flags.append('-DHAVE_APPLY')
    if 'static void\nnight_off_apply (void)' in mc:
        a = mc.index('static void\nnight_off_apply (void)'); b = mc.index('\n}\n', a) + 3
        teile.append('#line %d "main.c"\n' % (mc[:a].count('\n') + 1) + mc[a:b])
    else:
        teile.append('static void night_off_apply (void) { }   /* Stand ohne M2 */\n')
else:
    teile.append('static void night_apply (uint_fast16_t s, uint_fast8_t a) { (void) s; (void) a; }   /* Stand ohne A69 */\n')
# Minutenzweig, Displayteil: ab A69-Block (falls da) bzw. ab der Displaypruefung bis vor die Ambilight-Pruefung
e0 = mc.index('            if (night_check_night_times (1, display.ambilight_power_is_on')
e = mc.index('            if (display_clock_flag == 0)', e0)            # A68: Ambilight-Timer gehoert jetzt dazu
s = mc.rfind('            if (night_wm != 0xFFFF)', 0, e)
if s < 0 or s < mc.rfind('if (show_time_flag)', 0, e): s = mc.rfind('            if (night_check_night_times (0, display.display_power_is_on', 0, e0)
# Startblock: alles im RTC-Block nach der Zeile "read rtc" (A69, M1, M2)
r = mc.index('log_printf ("read rtc: %s')
r = mc.index('gmain.tm.tm_sec);\n', r) + len('gmain.tm.tm_sec);\n')
re_ = mc.index('    }\n\n    main_set_ambilight_clock_wait_cycles', r)   # Block kann leer sein (Stand ohne A69)
teile.append('static void startblock (void)\n{\n#line %d "main.c"\n' % (mc[:r].count('\n') + 1) + mc[r:re_] + '\n}\n')
# Hauptloop: IP-Lauftext samt Frist (bis vor den IR-Abzug) und die Restore-Bedingung, woertlich
h1 = mc.index('        if (ip_ticker_pending && ! var_sync_pending)'); h2 = mc.index('        /* Abzug der IR-Codes', h1)
h3 = mc.index('        if (pending_ticker_restore &&'); h4 = mc.index('        if (display_clock_flag)', h3)
teile.append('static void hauptloop_teile (void)\n{\n#line %d "main.c"\n' % (mc[:h1].count('\n') + 1) + mc[h1:h2]
             + '#line %d "main.c"\n' % (mc[:h3].count('\n') + 1) + mc[h3:h4] + '}\n')
teile.append('static void minutenzweig (void)\n{\n#line %d "main.c"\n' % (mc[:s].count('\n') + 1) + mc[s:e] + '}\n')
if 'night_last_event' in nc: flags.append('-DHAVE_A69')

os.makedirs(os.path.join(w, 'stub'), exist_ok=True)
def schreib(p, t): open(os.path.join(w, p), 'w').write(t)
schreib('night.c', '#line 1 "night.c"\n' + nc)
schreib('night.h', nh)
schreib('main-a69.c', teile_kopf + ''.join(teile))
schreib('stub/eep.h', '#include <stdint.h>\nextern uint_fast8_t eep_is_up;\nextern uint_fast8_t eep_read (uint_fast16_t, unsigned char *, uint_fast16_t);\nextern uint_fast8_t eep_write (uint_fast16_t, unsigned char *, uint_fast16_t);\n')
schreib('stub/eeprom-data.h', '#define EEPROM_VERSION_1_7 17\n#define EEPROM_VERSION_2_5 25\n#define EEPROM_DATA_SIZE_NIGHT_TIME 24\n#define EEPROM_DATA_SIZE_AMBI_NIGHT_TIME 24\n#define EEPROM_DATA_OFFSET_NIGHT_TIME 0\n#define EEPROM_DATA_OFFSET_AMBI_NIGHT_TIME 24\n')
schreib('stub/log.h', '#include <stdarg.h>\nextern void log_printf (const char *, ...);\n#define log_message(s) log_printf ("%s\\r\\n", s)\n#define debug_log_message(s)\n#define debug_log_printf(fmt,...)\n')
schreib('flags', ' '.join(flags))
# Statisch: Aufrufstellen in main.c (Kommentare entfernt)
m = re.sub(r'/\*.*?\*/|//[^\n]*', '', mc, flags=re.S)
st = []
if 'night_apply' in m:
    wi = m.index('watchdog_init ();')
    rb = m.rfind('if (grtc.rtc_is_up && rtc_get_date_time (&gmain.tm))', 0, wi)
    blk = m[rb:wi]
    if not re.search(r'if \(gmain\.year >= 2026\)\s*\{\s*night_apply \(0xFFFF, 2\);\s*\}\s*\}', blk):
        st.append('Start: night_apply (0xFFFF, 2) nicht unter gmain.year >= 2026 im RTC-Block vor watchdog_init() (M1/M2)')
    c = m.index('case ESP8266_TIME:'); ce = m.index('case ESP8266_TABLES:', c)
    tb = m[c:ce]; ok = tb.find('TIMESERVER_NET_TIME_OK')
    if not (ok >= 0 and re.search(r'if \(night_wm == 0xFFFF\)\s*\{\s*night_apply \(0xFFFF, [^;]*\);', tb[ok:])):
        st.append('TIME: night_apply (0xFFFF, 1) nicht im Uebernahmezweig hinter night_wm == 0xFFFF')
else:
    st.append('night_apply fehlt in main.c')
# A68 Bestandsliste: jede display_set_ambilight_power()-Stelle in main.c sendet danach die
# Ambilight-Power -- ausser dem Setter vom ESP (DISPLAY_AMBILIGHT_POWER_NUM_VAR: der ESP ist die Quelle)
stellen = [x.start() for x in re.finditer(r'display_set_ambilight_power \(', m)]
ausnahme = m.find('case DISPLAY_AMBILIGHT_POWER_NUM_VAR:')
for p in stellen:
    zeile = m[:p].count('\n') + 1
    if ausnahme >= 0 and ausnahme < p < m.find('break;', ausnahme): continue
    nach = m[p:p + 700]; ende = nach.find('\n}\n')
    if 'var_send_display_ambilight_power ()' not in (nach if ende < 0 else nach[:ende]):
        st.append('A68: display_set_ambilight_power() ohne nachfolgende Sendung (main.c, Zeile %d ohne Kommentare)' % zeile)
st.append('A68: Bestand %d Stellen' % len(stellen)) if len(stellen) != 3 else None
schreib('statisch', ''.join(s + '\n' for s in st))
