# Pruefstand a56 (Paket 2026-10-13, S2.3, A56): Ambilight-Power uebersteht einen STM-Neustart.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis>
# WOERTLICH: das Layout aus eeprom-data.h samt den Fremdkonstanten aus ihren Headern (layout.h);
# eeprom_write (eeprom.c, damit die Schreibzyklen die echten sind); aus display.c
# display_save_ambilight_power (fehlt im alten Stand), display_set_ambilight_power,
# display_read_config_from_eep, display_write_config_to_eep; der Startwert aus display_init().
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
ed = lies('eeprom/eeprom-data.h'); dh = lies('display/display.h'); dc = lies('display/display.c')
ee = lies('eeprom/eeprom.c'); ov = lies('overlay/overlay.h')
def define(t, name, wo):
    m = re.search(r'^#define\s+' + name + r'\b.*?(?<!\\)$', t, re.M | re.S)
    if not m: sys.exit('extract: %s fehlt in %s' % (name, wo))
    return re.sub(r'\s*//[^\n]*', '', m.group(0))
L = []
for name, wo, t in [('N_REMOTE_IR_CMDS', 'remote-ir/remote-ir.h', lies('remote-ir/remote-ir.h')),
                    ('DATE_TICKER_FORMAT_LEN', 'display.h', dh), ('MAX_BRIGHTNESS', 'display.h', dh),
                    ('EEPROM_MAX_ANIMATION_MODES', 'display.h', dh), ('EEPROM_MAX_COLOR_ANIMATION_MODES', 'display.h', dh),
                    ('EEPROM_MAX_AMBILIGHT_MODES', 'display.h', dh),
                    ('EEPROM_ANIMATION_DECELERATION_MASK', 'display.h', dh), ('EEPROM_ANIMATION_FAVOURITE_MASK', 'display.h', dh),
                    ('EEPROM_COLOR_ANIMATION_DECELERATION_MASK', 'display.h', dh), ('EEPROM_AMBILIGHT_DECELERATION_MASK', 'display.h', dh),
                    ('MAX_NIGHT_TIMES', 'night.h', lies('night/night.h')), ('MAX_ALARM_TIMES', 'alarm.h', lies('alarm/alarm.h'))]:
    L.append(define(t, name, wo))
we = lies('weather/weather.h')
for n in ('APPID', 'CITY', 'LON', 'LAT'): L.append(define(we, 'MAX_WEATHER_%s_LEN' % n, 'weather.h'))
L.append(define(ov, 'MAX_OVERLAYS', 'overlay.h'))
for n in sorted(set(re.findall(r'^#define\s+(OVERLAY_\w+_LEN)\b', ov, re.M))): L.append(define(ov, n, 'overlay.h'))
L.append(define(ov, 'OVERLAY_ENTRY_SIZE', 'overlay.h')); L.append(define(ov, 'OVERLAY_ENTRIES_SIZE', 'overlay.h'))
L.append(define(ed, 'PACKED_IRMP_DATA_SIZE', 'eeprom-data.h'))
L += re.findall(r'^#define EEPROM_VERSION\w*\s.*$', ed, re.M)
a = ed.index('#define EEPROM_MAX_IR_CODES'); b = ed.rindex('#endif')
rumpf = re.sub(r'#if N_REMOTE_IR_CMDS.*?#endif\n', '', ed[a:b], flags=re.S)
L.append('#line %d "eeprom-data.h"\n' % (ed[:a].count('\n') + 1) + rumpf)
open(os.path.join(w, 'layout.h'), 'w').write('\n'.join(L) + '\n')

def fn(t, name, wo, pflicht=True):
    k = t.find('\n' + name + ' (')
    if k < 0:
        if pflicht: sys.exit('extract: %s fehlt in %s' % (name, wo))
        return '/* %s: im Stand nicht vorhanden */\n' % name
    a = t.rindex('\n', 0, k) + 1; b = t.index('\n}\n', k) + 3
    return '#line %d "%s"\n' % (t[:a].count('\n') + 1, wo) + t[a:b]
i = dc.index('\ndisplay_init (void)'); m = re.search(r'display\.ambilight_power_is_on\s*=\s*(\d+)\s*;', dc[i:])
teile = ['#define INIT_AMBI_POWER %s\n' % m.group(1), fn(ee, 'eeprom_write', 'eeprom.c'),
         fn(dc, 'display_save_ambilight_power', 'display.c', False), fn(dc, 'display_set_ambilight_power', 'display.c'),
         fn(dc, 'display_read_config_from_eep', 'display.c'), fn(dc, 'display_write_config_to_eep', 'display.c')]
open(os.path.join(w, 'auszug.c'), 'w').write('\n'.join(teile))
print('extract: eeprom-data.h (Layout) + Fremdkonstanten, eeprom.c (eeprom_write), display.c (4 Funktionen, Startwert %s)' % m.group(1))
