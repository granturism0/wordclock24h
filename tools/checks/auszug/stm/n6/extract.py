# Pruefstand N6 (Paket 2026-10-11, S3.2): DFPlayer-Logaufrufe alt gegen neu.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis> <seite: alt|neu> [sabotage]
# Schreibt: aufrufe.c (jeder Logaufruf aus dfplayer.c als case), helfer.c (dfp_* aus dfplayer.c, falls da),
# log.c (die echte, ggf. sabotiert), n_aufrufe. Mit "einfrieren" statt seite: legt die Alt-Dateien ab.
import sys, os, re
src, w, seite = sys.argv[1], sys.argv[2], sys.argv[3]
sab = sys.argv[4] if len(sys.argv) > 4 else ''
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
dfp = lies('dfplayer/dfplayer.c'); logc = lies('log/log.c'); logh = lies('log/log.h')
SAB_DFP = {'praefix': ('    log_puts (DFP_PREFIX);\n', '')}
SAB_LOG = {
    'zweizeilen': ('    while (*prefix)\n    {\n        *p++ = *prefix++;\n    }\n', '    (void) log_uart_puts (prefix);\n    esp8266_send_log_line (prefix);\n'),
    'grenze':     ('STRBUF_SIZE - (p - str_buf)', 'STRBUF_SIZE - (p - str_buf) - 1'),
}
if sab:
    tab = SAB_DFP if sab in SAB_DFP else SAB_LOG if sab in SAB_LOG else None
    if not tab: sys.exit('extract: unbekannte Sabotage ' + sab)
    a, n = tab[sab]; ziel = dfp if tab is SAB_DFP else logc
    if ziel.count(a) != 1: sys.exit('extract: Sabotage %s -- Anker %d-mal' % (sab, ziel.count(a)))
    if tab is SAB_DFP: dfp = dfp.replace(a, n)
    else: logc = logc.replace(a, n)
ohne_helfer = dfp
if 'dfp_message (const char * s)' in dfp:              # der Helfer selbst ist kein Logaufruf der Datei
    ha = dfp.index('#define DFP_PREFIX'); hb = dfp.index('\n}\n', dfp.index('dfp_message (const char * s)')) + 3
    ohne_helfer = dfp[:ha] + dfp[hb:]
code = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), ohne_helfer, flags=re.S)
code = re.sub(r'//[^\n]*', '', code)
aufrufe = re.findall(r'\b((?:log_printf|log_message|dfp_printf|dfp_message)\s*\((?:[^;"]|"(?:[^"\\]|\\.)*")*?\))\s*;', code)
helfer = ''
if 'dfp_message (const char * s)' in dfp:
    a = dfp.index('#define DFP_PREFIX'); b = dfp.index('\n}\n', dfp.index('dfp_message (const char * s)')) + 3
    helfer = dfp[a:b]
os.makedirs(w, exist_ok=True)
def schreib(n, t): open(os.path.join(w, n), 'w').write(t)
pre = 'alt-' if seite == 'einfrieren' else ''
schreib(pre + 'aufrufe.c', ''.join('        case %d: %s; break;\n' % (i, s) for i, s in enumerate(aufrufe)) + '#define N_AUFRUFE %d\n' % len(aufrufe))
schreib(pre + 'helfer.c', helfer)
schreib(pre + 'log.c', logc)
schreib(pre + 'log.h', logh)
print('extract (%s): %d Logaufrufe aus dfplayer.c%s' % (seite, len(aufrufe), ', Helfer dfp_*' if helfer else ''))
