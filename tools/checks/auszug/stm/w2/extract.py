# Prüfstand W2 (A2, Paket 2026-10-09): zieht den ECHTEN Code aus esp8266.c, vars.c und weather.c.
# Aufruf: extract.py <src-Verzeichnis> <ausgabe.c>
# Zusaetzlich eine statische Pruefung an main.c: schedule_esp8266_messages() reicht JEDE Nachricht
# an var_weather_query_end() weiter, direkt nach esp8266_get_message(). Das Ergebnis geht als
# MAIN_HOOK in den Auszug und zaehlt im Pruefstand als eigener Fall.
import sys, re
src, out = sys.argv[1], sys.argv[2]
def text(path):
    return open(path, 'rb').read().decode('ascii').replace('\r\n', '\n')
def cut(path, start, end, back_one_line=False, to_eof=False):
    t = text(path)
    assert t.count(start) == 1, (path, start)
    a = t.index(start)
    if to_eof:
        b = len(t)
    else:
        assert t.count(end) == 1, (path, end)
        b = t.index(end, a)
        if back_one_line:
            b = t.rfind('\n', 0, b - 1) + 1
    return "#line %d \"%s\"\n" % (t[:a].count('\n') + 1, path) + t[a:b] + "\n"
e = cut(src + '/esp8266/esp8266.c', 'static uint_fast8_t     esp8266_cap_var_crc_seen', ' * send a command to ESP8266', True)
v = cut(src + '/vars/vars.c', '#define VAR_SEND_TIMEOUT_SEC    3', '/* A32: Ende des Abschnitts, den der Tischpruefstand einbindet')
w = cut(src + '/weather/weather.c', 'void\nweather_query (uint_fast8_t query_id)', None, to_eof=True)
m = text(src + '/main.c')
i = m.find('\nschedule_esp8266_messages (void)\n{')
body = m[i:m.find('\n}\n', i)] if i >= 0 else ''
hook = 1 if re.search(r'msg_rtc = esp8266_get_message \(\);\n\s*var_weather_query_end \(msg_rtc\);', body) else 0
open(out, 'w').write("#define MAIN_HOOK %d\n" % hook + e + v + w)
print("extrahiert aus", src, ":", e.count('\n'), "+", v.count('\n'), "+", w.count('\n'), "Zeilen, MAIN_HOOK", hook)
