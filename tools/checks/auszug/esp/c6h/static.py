#!/usr/bin/env python3
"""c6h, statischer Teil: Bestandsliste aller zerstoerenden Wege in http.cpp (AKH.1, Paket 2026-10-13 E1).

Senken: stm32_reset (), RESET_EEPROM_RPC_VAR, LittleFS.format (), LittleFS.remove (<Name>).
Jede Senke wird einem Weg zugeordnet:
  api     in einer Funktion http_api_*: Herkunftspruefung mit return davor im Rumpf
  aktion  in einem Block if (! strcmp (action, "X")): Herkunftspruefung mit return davor im Block
  merker  in einem Block if (V): JEDE Zuweisung V = <ungleich 0> in derselben Funktion steht in einem
          Aktionsblock mit Herkunftspruefung davor - der Weg ist dann jene Aktion
  intern  LittleFS.remove () mit einem Namen, den nicht die Anfrage bestimmt (Liste INTERN, je mit Grund)
Alles andere ist ein ungepruefter Weg => FEHL. Die gefundenen Wege muessen zudem genau ERWARTET sein:
Ein neuer Weg, auch ein geprueft, verlangt einen Eintrag hier (und einen Fall in c6h.cpp).
Exit 0 OK, 1 FEHL, 2 Datei nicht lesbar.
"""
import re, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ausschnitt import text, blank, functions, match

ERWARTET = {
    ('http_api_maintenance_format_fs', '-'), ('http_api_maintenance_reset_stm32', '-'),
    ('http_api_maintenance_reset_eeprom', '-'), ('http_api_fs_remove', '-'),
    ('http_main', 'rststm32'), ('http_main', 'rsteeprom2'),
    ('http_fs', 'format'), ('http_fs', 'remove'),
    ('http_update', 'reset'), ('flash_stm32_local', 'reset'),
}
# LittleFS.remove () mit festem bzw. intern gebildetem Namen - keine Auswahl durch die Anfrage
INTERN = {
    ('http_remove_table_family_files', 'current'):          'alte Tabellendateien beim Upload einer neuen (POST /api/fs_upload_tables)',
    ('download_file_as_flattened_app_asset', 'http_app_asset_stale_filename'): 'Altname eines App-Assets nach dem Download',
    ('download_file_to_local', 'local_filename'):          'halbe Datei nach gescheitertem Download',
    ('http_api_fs_upload', 'target_filename'):             'halbe Datei nach gescheitertem Upload',
    ('http_api_app_file_upload', 'local_filename'):        'halbe Datei nach gescheitertem Upload',
    ('http_api_app_file_upload', 'stale_filename'):        'Altname nach erfolgreichem Upload',
    ('run_local_stm32_flash_response', '"stm32.hex"'):      'Zwischendatei des lokalen STM-Flashs',
    ('http_api_local_stm32_upload', '"stm32.hex"'):         'halbe Zwischendatei nach gescheitertem Upload',
    ('flash_stm32_local', '"stm32.hex"'):                   'halbe Zwischendatei nach gescheitertem Upload',
}
WACHE = re.compile(r'if\s*\(\s*http_request_denied_by_origin\s*\(\s*F\s*\(\s*"[^"]*"\s*\)\s*,\s*[01]\s*\)\s*\)\s*\{\s*return\b')
SENKE = re.compile(r'\bstm32_reset\s*\(\s*\)|\bRESET_EEPROM_RPC_VAR\b|\bLittleFS\s*\.\s*format\s*\(|\bLittleFS\s*\.\s*remove\s*\(([^)]*)\)')
AKTION = re.compile(r'if\s*\(\s*!\s*strcmp\s*\(\s*action\s*,\s*"(\w+)"\s*\)\s*\)')

def main():
    path, bez = sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else 'neu'
    try:
        s = text(path)
    except OSError as e:
        print(f'DATEI ({bez}): {e}'); return 2
    b = blank(s)
    # Kommentare entfernt, Zeichenketten erhalten (fuer Aktionsnamen und F("...") der Wache)
    nc = ''.join(s[i] if (b[i] != ' ' or s[i] in ' \t\n' or _in_string(s, b, i)) else ' ' for i in range(len(s)))
    fehler, wege, intern = [], set(), []
    fns = functions(s, b)
    def zeile(p): return s.count('\n', 0, p) + 1
    def wache_vor(a, e): return WACHE.search(nc, a, e) is not None
    def aktionsbloecke(r, e):
        res = []
        for m in AKTION.finditer(nc, r, e):
            j = b.index('{', m.end()); res.append((m.group(1), j, match(b, j)))
        return res
    n_senken = 0
    for name, k, r, e in fns:
        bl = aktionsbloecke(r, e)
        for m in SENKE.finditer(nc, r, e):
            n_senken += 1
            p = m.start(); was = m.group(0).split('(')[0].strip(); z = zeile(p)
            if m.group(1) is not None:
                arg = m.group(1).strip()
                if (name, arg) in INTERN:
                    intern.append(f'{name}:{z} LittleFS.remove ({arg}) - {INTERN[(name, arg)]}'); continue
            drin = [x for x in bl if x[1] < p < x[2]]
            if drin:
                a, j, _ = min(drin, key=lambda x: x[2] - x[1])
                if wache_vor(j, p): wege.add((name, a)); print(f'  Weg {name}:{z} {was} - Aktion "{a}", geprueft')
                else: fehler.append(f'{name}:{z} {was} in Aktion "{a}" OHNE Herkunftspruefung davor')
                continue
            if name.startswith('http_api_'):
                if wache_vor(r, p): wege.add((name, '-')); print(f'  Weg {name}:{z} {was} - API, geprueft')
                else: fehler.append(f'{name}:{z} {was} - API-Handler OHNE Herkunftspruefung davor')
                continue
            # Merker: innerster Block "if (V)" bzw. "else if (V)" um die Senke
            merker = None
            for mm in re.finditer(r'\bif\s*\(\s*([A-Za-z_]\w*)\s*\)', nc[r:e]):
                j = b.index('{', r + mm.end())
                if j < p < match(b, j): merker = mm.group(1)
            if not merker:
                fehler.append(f'{name}:{z} {was} - weder API noch Aktion noch Merker: UNGEPRUEFTER Weg'); continue
            setz = [x for x in re.finditer(r'\b' + merker + r'\s*=\s*([^;=][^;]*);', nc[r:e]) if x.group(1).strip() not in ('0',)]
            if not setz: fehler.append(f'{name}:{z} {was} - Merker {merker} wird nirgends gesetzt?'); continue
            for x in setz:
                q = r + x.start(); dd = [y for y in bl if y[1] < q < y[2]]
                if not dd: fehler.append(f'{name}:{zeile(q)} {merker} gesetzt ausserhalb jeder Aktion'); continue
                a, j, _ = min(dd, key=lambda y: y[2] - y[1])
                if wache_vor(j, q): wege.add((name, a)); print(f'  Weg {name}:{zeile(q)} {merker}=... -> {was}:{z} - Aktion "{a}", geprueft')
                else: fehler.append(f'{name}:{zeile(q)} Aktion "{a}" setzt {merker} (-> {was}:{z}) OHNE Herkunftspruefung davor')
    for i in intern: print('  intern ' + i)
    gesamt = sum(1 for _ in SENKE.finditer(nc))
    if gesamt != n_senken: fehler.append(f'{gesamt} Senken in der Datei, aber nur {n_senken} in Funktionen erfasst - Zuordnung unvollstaendig')
    for x in sorted(ERWARTET - wege): fehler.append(f'erwarteter Weg {x[0]} "{x[1]}" nicht als geprueft gefunden')
    for x in sorted(wege - ERWARTET): fehler.append(f'NEUER Weg {x[0]} "{x[1]}" - Bestandsliste in static.py und Fall in c6h.cpp nachfuehren')
    nint = sum(1 for _ in INTERN)
    print(f'c6h static ({bez}): {n_senken} Senken in {len(fns)} Funktionen, {len(wege)} gepruefte Wege (erwartet {len(ERWARTET)}), '
          f'{len(intern)} interne Loeschungen (Liste {nint})')
    if fehler:
        for f in fehler: print('FEHL ' + f)
        print(f'FEHL ({bez}): c6h static, {len(fehler)} Befunde'); return 1
    print(f'OK ({bez}): c6h static'); return 0

def _in_string(s, b, i):
    # Zeichen, das blank() geleert hat, aber kein Kommentar ist: innerhalb einer Zeichenkette
    a = s.rfind('\n', 0, i) + 1
    # Kommentar? Dann steht in b ab dem Zeilenanfang bis i kein Anfuehrungszeichen-Paar; robust:
    # Zeichenketten erkennt man daran, dass das oeffnende Anfuehrungszeichen in b erhalten ist.
    j = i
    while j > a and b[j - 1] == ' ' and s[j - 1] not in '"\'':
        j -= 1
    return j > a and s[j - 1] in '"\'' and b[j - 1] in '"\''

if __name__ == '__main__':
    sys.exit(main())
