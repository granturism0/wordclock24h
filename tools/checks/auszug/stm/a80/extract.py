# Pruefstand a80 (Paket 2026-10-13, S2.7, A80): Icon bei ausgeschaltetem Display.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis>
# WOERTLICH aus main.c: set_display_power(), der Zweig ESP8266_ICONDATA, der icon_freeze-Block,
# der Zweig "else if (show_icon_stop_time > 0)" und die Restore-Bedingung um pending_ticker_restore;
# aus display.c: DISPLAY_ICON_STATE_* bis Ende display_icon(), dazu display_icon_cancel() falls vorhanden.
# Zusaetzlich: restore.txt = Restore-Bedingung bis einschliesslich "if (! display_clock_flag)" fuer den statischen
# Vergleich (AKN.5: der Innenzweig darf sich mit A70 aendern, die Bedingung nicht).
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
mc = lies('main.c'); dc = lies('display/display.c')
def zeile(t, a): return t[:a].count('\n') + 1
def block(t, start, wo):
    """ab Index start bis zur schliessenden Klammer des ersten '{' danach"""
    k = t.index('{', start); tiefe = 0; i = k
    while True:
        c = t[i]
        if c == '{': tiefe += 1
        elif c == '}':
            tiefe -= 1
            if tiefe == 0: return t[start:i + 1]
        i += 1
def finde(t, s, wo, n=1):
    if t.count(s) != n: sys.exit('extract: "%s" %d-mal in %s, erwartet %d' % (s.strip()[:50], t.count(s), wo, n))
    return t.index(s)
def fn(t, name, wo, pflicht=True):
    k = t.find('\n' + name + ' (')
    if k < 0:
        if pflicht: sys.exit('extract: %s fehlt in %s' % (name, wo))
        return None
    a = t.rindex('\n', 0, k) + 1                         # Zeile mit dem Rueckgabetyp
    b = t.index('\n}\n', k) + 3
    return '#line %d "%s"\n' % (zeile(t, a), wo) + t[a:b]
out = []
out.append(fn(mc, 'set_display_power', 'main.c'))
# ICONDATA: Rumpf von "case ESP8266_ICONDATA:" bis vor den naechsten case
a = finde(mc, 'case ESP8266_ICONDATA:', 'main.c') + len('case ESP8266_ICONDATA:')
rumpf = block(mc, a, 'main.c')
out.append('static void pf_icondata (void)\n{\n    switch (0) { default:\n#line %d "main.c"\n%s\n    }\n}\n' % (zeile(mc, a), rumpf))
# icon_freeze
a = finde(mc, 'static uint_fast8_t icon_freeze_state', 'main.c')
a = mc.rindex('\n', 0, a) + 1
b = finde(mc, 'if (icon_freeze_now != icon_freeze_state)', 'main.c')
fr = mc[a:b] + block(mc, b, 'main.c')
out.append('static void pf_icon_freeze (void)\n{\n#line %d "main.c"\n%s\n}\n' % (zeile(mc, a), fr))
# Ablauf des Icon-Stop-Timers
a = finde(mc, 'else if (show_icon_stop_time > 0)', 'main.c')
out.append('static void pf_icon_stop (void)\n{\n    if (0) { }\n#line %d "main.c"\n    %s\n}\n' % (zeile(mc, a), block(mc, a, 'main.c')))
# Restore-Bedingung
a = finde(mc, 'if (pending_ticker_restore &&', 'main.c')
rb = block(mc, a, 'main.c')
out.append('static void pf_restore (void)\n{\n#line %d "main.c"\n    %s\n}\n' % (zeile(mc, a), rb))
k = rb.find('if (! display_clock_flag)')                     # Invariante: vier Teilbedingungen + ! display_clock_flag
if k < 0: sys.exit('extract: "if (! display_clock_flag)" fehlt in der Restore-Bedingung')
open(os.path.join(w, 'restore.txt'), 'w').write(rb[:rb.index('\n', k)] + '\n')   # Innenzweig gehoert A70 (AKN.5)
# display.c
a = finde(dc, '#define DISPLAY_ICON_STATE_CLEAR_DISPLAY', 'display.c')
k = finde(dc, '\ndisplay_icon (void)', 'display.c')
b = dc.index('\n}\n', k) + 3
d = ['#line %d "display.c"\n' % zeile(dc, a) + dc[a:b]]
c = fn(dc, 'display_icon_cancel', 'display.c', False)
d.append(c if c else '/* display_icon_cancel: im Stand nicht vorhanden */\n#define PF_KEIN_CANCEL 1\n')
open(os.path.join(w, 'auszug-main.c'), 'w').write('\n'.join(out))
open(os.path.join(w, 'auszug-display.c'), 'w').write('\n'.join(d))
print('extract: main.c (set_display_power, ICONDATA, icon_freeze, Icon-Stop, Restore), display.c (display_icon%s)' % (', display_icon_cancel' if c else ''))
