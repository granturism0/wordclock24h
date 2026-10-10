# Pruefstand EP (A55, Paket 2026-10-10): zieht den ECHTEN Zeilenparser aus esp8266.c und var_send_buf()
# samt Umgebung aus vars.c (dieselben Schnitte wie W2), dazu weather.c (von vars.c gebraucht).
# Statisch an main.c und am ESP-Code, als Makros in den Auszug:
#   EP_MAIN_SYNC     IPADDRESS- und SYNCVARS-Zweig merken den Vollabgleich vor (var_sync_pending = 1)
#   EP_MAIN_SYNC_UNGATED  der Vollabgleich haengt NICHT an esp8266.is_online
#   EP_MAIN_GATED / EP_MAIN_SITES  Aufrufstellen der vier periodischen Sendungen, davon mit is_online davor
#   EP_MAIN_REPLY    davon Antwort auf eine ESP-Zeile (case ESP8266_...): der ESP lebt, kein is_online noetig
#   EP_ESP_CAP_FIRST CAP steht in setup() vor wifi_ap()/wifi_connect() und vor FIRMWARE
#   EP_ESP_IP_SITES  Stellen in wifi.cpp, die IPADDRESS senden; EP_ESP_SYNC_AFTER_IP SYNCVARS erst nach eigener IP
import sys, re
src, out = sys.argv[1], sys.argv[2]
esp = sys.argv[3] if len(sys.argv) > 3 else src + '/../ESP8266/ESP-uclock'
def text(path):
    return open(path, 'rb').read().decode('ascii', 'replace').replace('\r\n', '\n')
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

m = re.sub(r'/\*.*?\*/|//[^\n]*', '', text(src + '/main.c'), flags=re.S)       # Kommentare zaehlen nicht
def case_body(name):
    i = m.find('case ' + name + ':')
    return m[i:m.find('break;', i)] if i >= 0 else ''
sync = int(bool(re.search(r'var_sync_pending\s*=\s*1\s*;', case_body('ESP8266_IPADDRESS'))) and
           bool(re.search(r'var_sync_pending\s*=\s*1\s*;', case_body('ESP8266_SYNCVARS'))))
blk = re.search(r'if \(var_sync_pending\)\s*\{(.*?)\}', m, flags=re.S)
ungated = int(bool(blk) and 'var_send_all_variables' in blk.group(1) and 'is_online' not in blk.group(1))
sites = gated = reply = 0
for fn in ('var_send_tm', 'var_send_rtc_temp_index', 'var_send_ds18xx_temp_index', 'var_send_ldr_raw_value'):
    for mm in re.finditer(r'\b' + fn + r'\s*\(\s*\)\s*;', m):
        sites += 1
        before = m[:mm.start()].split('\n')[-8:]                                   # die umschliessende Bedingung
        c = m.rfind('case ESP8266_', 0, mm.start())
        if c >= 0 and 'break;' not in m[c:mm.start()]:
            reply += 1                                                              # Antwort auf eine ESP-Zeile: ESP lebt
        elif any('esp8266.is_online' in l for l in before):
            gated += 1
ino = text(esp + '/ESP-uclock.ino')
s = ino[ino.index('\nsetup()'):]
s = s[:s.index('\n}\n')]
pc = s.find('Serial.println ("CAP var-crc")')
cap_first = int(pc >= 0 and all(pc < s.find(k) for k in ('wifi_ap (', 'wifi_connect (', 'Serial.print ("FIRMWARE ")')))
wifi = text(esp + '/wifi.cpp')
ip_sites = len(re.findall(r'statusmsg \("IPADDRESS", wifi_ip_address\)', wifi))
vc = ino[ino.index('\nvar_sync_check (void)'):]
vc = vc[:vc.index('\n}\n')]
sync_after_ip = int(vc.find('if (! wifi_ip_address[0])') >= 0 and vc.find('if (! wifi_ip_address[0])') < vc.find('Serial.println ("SYNCVARS")'))
hdr = ("#define EP_MAIN_SYNC %d\n#define EP_MAIN_SYNC_UNGATED %d\n#define EP_MAIN_SITES %d\n#define EP_MAIN_GATED %d\n"
       "#define EP_MAIN_REPLY %d\n#define EP_ESP_CAP_FIRST %d\n#define EP_ESP_IP_SITES %d\n#define EP_ESP_SYNC_AFTER_IP %d\n"
       % (sync, ungated, sites, gated, reply, cap_first, ip_sites, sync_after_ip))
open(out, 'w').write(hdr + e + v + w)
print("extrahiert aus", src, ":", e.count('\n'), "+", v.count('\n'), "+", w.count('\n'), "Zeilen;", hdr.replace('\n', ' ').replace('#define ', ''))
