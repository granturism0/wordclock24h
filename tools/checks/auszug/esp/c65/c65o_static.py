#!/usr/bin/env python3
"""c65o statisch (AKV.11, Ent-37): Weissliste als Tabelle vollstaendiger Pfade im Flash, Vergleich mit strcmp_P, kein
Muster; genau die 16 Eintraege aus design-e4.md 19.8; jeder Eintrag existiert im GET-Verteiler von http (); kein Eintrag
auf _set/_default und keiner aus der Ausnahmeliste; die Pruefung steht in http () VOR dem Verteiler; api_orphan_skipped
in device_ready. Liest Liste und Verteiler selbst aus http.cpp. Exit 0 OK, 1 FEHL."""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
s = open(os.path.join(q, 'http.cpp'), 'rb').read().decode('utf-8').replace('\r\n', '\n')
c = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
c = '\n'.join(re.sub(r'//.*', '', l) for l in c.split('\n'))
fails = []
SOLL = {'settings_xml', 'display_power', 'ambilight_power', 'power_status', 'network_scan', 'eeprom_settings', 'network_secrets',
        'update_status', 'update_table_files', 'update_progress', 'stm32_log', 'overlay_icons', 'fs_info', 'fs_list', 'fs_show', 'ir_codes_get'}
AUSNAHME = re.compile(r'^(device_ready|reconnect_probe|network_get_time|ir_codes_request|weather_get_now|weather_get_forecast|temperature_display|'
                      r'overlay_display|dfplayer_play|test_display|learn_ir|maintenance_.*|fs_remove|fs_upload_.*|app_file_upload|update_download_.*|'
                      r'remote_.*|local_.*|stm32_log_clear|live_display_color|overlay_delete|network_wps|datetime_set|.*_set|.*_default)$')
m = re.search(r'static\s+const\s+char\s+http_api_read_only_paths\s*\[\s*\]\s*\[[^\]]+\]\s+PROGMEM\s*=\s*\{(.*?)\};', c, re.S)
if not m:
    print(f"c65o statisch ({bez}): keine Tabelle http_api_read_only_paths[][] PROGMEM"); print(f"FEHL  c65o statisch {bez}: 1 Fehler"); sys.exit(1)
liste = re.findall(r'"([^"]*)"', m.group(1))
L = c.split('\n')
i = next(k for k, l in enumerate(L) if l.startswith('http (const char * path'))
j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
H = L[i:j]
verteiler = set(re.findall(r'!\s*strcmp\s*\(\s*path\s*,\s*"(/api/[^"]+)"\s*\)', '\n'.join(H)))
print(f"c65o statisch ({bez}): {len(liste)} Eintraege in der Weissliste, {len(verteiler)} /api/-Pfade im GET-Verteiler von http ()")
namen = []
for e in liste:
    if not e.startswith('/api/'): fails.append(f"kein /api/-Pfad: {e}"); continue
    n = e[5:]; namen.append(n)
    if e not in verteiler: fails.append(f"nicht im Verteiler: {e}")
    if AUSNAHME.match(n): fails.append(f"Setter/Ausloeser/Ausnahme in der Liste: {e}")
if set(namen) != SOLL or len(namen) != len(SOLL):
    fails.append(f"Liste weicht von design-e4.md 19.8 ab: zu viel {sorted(set(namen) - SOLL)}, fehlt {sorted(SOLL - set(namen))}, doppelt {len(namen) - len(set(namen))}")
f = re.search(r'^http_api_is_read_only \(const char \* path\)\n\{(.*?)^\}', c, re.S | re.M)
if not f or 'strcmp_P' not in f.group(1) or 'http_api_read_only_paths' not in f.group(1): fails.append("http_api_is_read_only () vergleicht nicht per strcmp_P gegen die Tabelle")
if f and re.search(r'_set|_default|strstr|endsWith|strrchr', f.group(1)): fails.append("http_api_is_read_only () schliesst aus dem Namen (Muster)")
pr = next((k for k, l in enumerate(H) if re.search(r'http_api_is_read_only\s*\(\s*path\s*\)\s*&&.*!\s*http_client\.connected\s*\(\s*\)', l)), None)
v0 = next((k for k, l in enumerate(H) if re.search(r'!\s*strcmp\s*\(\s*path\s*,\s*"/"\s*\)', l)), None)
if pr is None or v0 is None or not pr < v0: fails.append("Pruefung auf verwaiste Lesende steht nicht VOR dem Verteiler in http ()")
if not re.search(r'http_json_send_uint_field\s*\(\s*FS\("api_orphan_skipped"\)', c): fails.append("api_orphan_skipped fehlt in device_ready")
for x in fails: print("      FEHLER: " + x)
print(f"{'FEHL' if fails else 'OK  '}  c65o statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
