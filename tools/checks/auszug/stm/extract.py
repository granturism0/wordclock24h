# Zieht die echten Abschnitte aus esp8266.c und vars.c, damit der Pruefstand den Firmware-Code
# selbst laufen laesst und keine Nachbildung. Aufruf: extract.py <src-Verzeichnis> <ausgabe.c>
import sys
src, out = sys.argv[1], sys.argv[2]
def cut(path, start, end, back_one_line=False):
    t = open(path, 'rb').read().decode('ascii').replace('\r\n', '\n')
    assert t.count(start) == 1 and t.count(end) == 1, (path, start, end)
    a = t.index(start)
    b = t.index(end, a)
    if back_one_line:
        b = t.rfind('\n', 0, b - 1) + 1
    return "#line %d \"%s\"\n" % (t[:a].count('\n') + 1, path) + t[a:b] + "\n"
e = cut(src + '/esp8266/esp8266.c', 'static uint_fast8_t     esp8266_cap_var_crc_seen', ' * send a command to ESP8266', True)
v = cut(src + '/vars/vars.c', '#define VAR_SEND_TIMEOUT_SEC    3', '/* A32: Ende des Abschnitts, den der Tischpruefstand einbindet')
open(out, 'w').write(e + v)
print("extrahiert aus", src, ":", e.count('\n'), "+", v.count('\n'), "Zeilen")
