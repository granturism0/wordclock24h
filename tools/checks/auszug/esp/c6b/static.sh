#!/bin/sh
# Statische Pruefung c6b (Paket 2026-10-13, EP.3, AKS.1/AKS.2): /api/eeprom_settings gibt keinen
# Schluesselwert mehr aus (key/ap_key leer, dazu key_set/ap_key_set), /api/network_secrets gibt beide
# aus, mit Cache-Control: no-store; Verteiler und URL-Liste kennen den Endpunkt; nirgends eine
# CORS-Kopfzeile. Aufruf: static.sh <verzeichnis ESP-uclock> <bezeichnung>  Exit 0/1/2
Q=$1; B=${2:-$1}
python3 - "$Q" "$B" <<'P2'
import sys, re, glob, os
q, b = sys.argv[1], sys.argv[2]
try:
    T = open(os.path.join(q, 'http.cpp'), 'rb').read().decode('utf-8').replace('\r\n', '\n')
except Exception as e:
    print('KAPUTT c6b %s: %s' % (b, e)); sys.exit(2)
ohne_kommentar = lambda s: re.sub(r'//[^\n]*', '', re.sub(r'/\*.*?\*/', '', s, flags=re.S))
C = ohne_kommentar(T)
def rumpf(name):
    m = re.search(r'\n(static\s+)?int\s*\n?\s*' + name + r'\s*\(\s*\)\s*\n\{', C)
    if not m: return None
    i = m.end(); tiefe = 1
    while tiefe and i < len(C):
        tiefe += {'{': 1, '}': -1}.get(C[i], 0); i += 1
    return C[m.end():i]
fehler = []
es = rumpf('http_api_eeprom_settings')
if es is None:
    print('KAPUTT c6b %s: http_api_eeprom_settings nicht gefunden' % b); sys.exit(2)
for k in ('eeprom_ssidkey', 'eeprom_ap_ssidkey'):
    aus = re.findall(r'http_send\w*\s*\(\s*' + k + r'\s*\)', es)
    if aus: fehler.append('eeprom_settings gibt %s aus: %s' % (k, aus))
for f in ('\\"key\\":\\"\\",\\"key_set\\":', '\\"ap_key\\":\\"\\",\\"ap_key_set\\":'):
    if f not in es: fehler.append('eeprom_settings ohne %s' % f.replace('\\', ''))
ns = rumpf('http_api_network_secrets')
if ns is None:
    fehler.append('http_api_network_secrets fehlt')
else:
    for k in ('eeprom_ssidkey', 'eeprom_ap_ssidkey'):
        if not re.search(r'http_send_json_escaped\s*\(\s*' + k + r'\s*\)', ns): fehler.append('network_secrets gibt %s nicht escaped aus' % k)
    if 'Cache-Control: no-store' not in ns: fehler.append('network_secrets ohne Cache-Control: no-store')
    if re.search(r'log_printf|Serial\.|debug', ns): fehler.append('network_secrets schreibt ins Log')
if not re.search(r'strcmp\s*\(\s*path\s*,\s*"/api/network_secrets"\s*\)\s*\)\s*\{\s*rtc\s*=\s*http_api_network_secrets\s*\(\s*\)', C):
    fehler.append('Verteiler kennt /api/network_secrets nicht')
if not re.search(r'"network_secrets_url"\)\s*,\s*"/api/network_secrets"', C):
    fehler.append('URL-Liste ohne network_secrets_url')
cors = []
for p in sorted(glob.glob(os.path.join(q, '*.cpp')) + glob.glob(os.path.join(q, '*.h')) + glob.glob(os.path.join(q, '*.ino'))):
    if 'Access-Control-' in ohne_kommentar(open(p, 'rb').read().decode('latin-1')): cors.append(os.path.basename(p))
if cors: fehler.append('CORS-Kopfzeile in %s' % cors)
for f in fehler: print('      FEHLER: ' + f)
print('%s  c6b %s: %d Fehler (geprueft: 2 Funktionsruempfe, Verteiler, URL-Liste, %d Quelldateien auf CORS)' % ('FEHL' if fehler else 'OK  ', b, len(fehler), len(glob.glob(os.path.join(q, '*.[ch]*'))) + len(glob.glob(os.path.join(q, '*.ino')))))
sys.exit(1 if fehler else 0)
P2
