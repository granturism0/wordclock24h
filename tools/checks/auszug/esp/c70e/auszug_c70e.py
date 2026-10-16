#!/usr/bin/env python3
"""Auszug fuer den Pruefstand c70e (Paket 2026-10-15, R2.1, C70; AKC.1-AKC.5, AKC.7; design.md 4.4, 8).
Aufruf: auszug_c70e.py <ESP-Quellverzeichnis> <ziel> <c65-Verzeichnis>

Ruft zuerst den Auszug von c65 (defs.inc, zaehler.inc, i1.inc, i4.inc, flush.inc, have.inc - die Auslieferung ueber
http_flush ()), dann schneidet er aus http.cpp ECHTEN Code:
  c70.inc     den C70-Block von "#define APP_INM_LEN" bis zum Ende von http_app_if_none_match_hits ()
  fs.inc      http_fs_file_exists_and_nonempty () und http_send_fs_file ()
  app.inc     PWA_PREFIX, PWA_INDEX_FILE, APP_INSTALL_ASSETS, http_content_type (), app_asset_filename (),
              http_find_app_install_asset (), http_app_asset_supports_gzip (), app_asset_storage_filename (),
              http_find_stored_app_asset_filename (), http_app_installation_complete (), http_app ()
  kopf.inc    den GET-Zweig von http_server_loop () bis vor http_log_request () - die Kopfzeilenschleife
Fehlt der C70-Block (Stand 3.2.39), bleibt c70.inc leer und C70_HAVE_BLOCK fehlt in c70have.inc: Der Pruefstand
uebersetzt mit Ersatz und meldet FEHL, nicht KAPUTT.
"""
import os, re, subprocess, sys
q, ziel, c65 = sys.argv[1], sys.argv[2], sys.argv[3]
r = subprocess.run([sys.executable, os.path.join(c65, 'auszug.py'), q, ziel], capture_output=True, text=True)
if r.returncode:
    sys.exit('c65-Auszug: ' + (r.stdout + r.stderr).strip())
H = open(os.path.join(q, 'http.cpp'), 'rb').read().decode('utf-8').replace('\r\n', '\n')
L = H.split('\n')

def func(kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return ''
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return '\n'.join(L[i - 1:j + 1]) + '\n'
def von_bis_funktion(start, kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(start)), None)
    if i is None: return ''
    j = next((k for k in range(i, len(L)) if L[k].startswith(kopf)), None)
    if j is None: return ''
    while not L[j].startswith('}'): j += 1
    return '\n'.join(L[i:j + 1]) + '\n'
def block(start, ende):
    i = next((k for k, l in enumerate(L) if l.startswith(start)), None)
    if i is None: return ''
    j = next(k for k in range(i, len(L)) if L[k].startswith(ende))
    return '\n'.join(L[i:j + 1]) + '\n'
def put(name, s):
    open(os.path.join(ziel, name), 'w').write(s)

have = []
c70 = von_bis_funktion('#define APP_INM_LEN', 'http_app_if_none_match_hits (')
put('c70.inc', c70)
if c70.strip(): have.append('C70_HAVE_BLOCK')
if re.search(r'^static uint16_t +http_app_not_modified', H, re.M): have.append('C70_HAVE_ZAEHLER')
if 'http_send_fs_file (const char * filename, const char * content_type, uint_fast8_t gzip_encoded, uint_fast8_t app_asset)' in H:
    have.append('C70_HAVE_4ARG')
# http_fs_file_exists_and_nonempty (): die DEFINITION (Kopf in Spalte 0 ohne ';'), nicht der Prototyp
i = next(k for k, l in enumerate(L) if l.startswith('http_fs_file_exists_and_nonempty (const char * filename)') and not l.rstrip().endswith(';'))
j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
fs = '\n'.join(L[i - 1:j + 1]) + '\n' + func('http_send_fs_file (const char * filename')
put('fs.inc', fs)
defs = ''.join(l + '\n' for l in L if re.match(r'^#define (PWA_PREFIX|PWA_INDEX_FILE)\s', l))
app = (defs + block('static const char * const APP_INSTALL_ASSETS[] =', '};') + func('http_content_type (const char * path)')
       + func('app_asset_filename (') + func('http_find_app_install_asset (') + func('http_app_asset_supports_gzip (')
       + func('app_asset_storage_filename (') + func('http_find_stored_app_asset_filename (')
       + func('http_app_installation_complete (void)') + func('http_app (const char * path)'))
put('app.inc', app)
# GET-Zweig: von "    else // GET" bis VOR die Zeile mit http_log_request (...) danach; der Pruefstand schliesst die Klammer
i = next((k for k, l in enumerate(L) if l.startswith('    else // GET')), None)
j = next((k for k in range(i, len(L)) if 'http_log_request (sRemoteIp' in L[k]), None) if i is not None else None
kopf = '\n'.join(L[i:j]) + '\n' if j is not None else ''
put('kopf.inc', kopf)
put('c70have.inc', ''.join('#define %s 1\n' % m for m in have))
for name, s in (('fs.inc', fs), ('app.inc', app), ('kopf.inc', kopf)):
    if not s.strip(): sys.exit('AUSZUG LEER: ' + name)
for muss in ('http_send_fs_file (', 'http_fs_file_exists_and_nonempty ('):
    if muss not in fs: sys.exit('AUSZUG UNVOLLSTAENDIG: ' + muss)
for muss in ('APP_INSTALL_ASSETS[]', 'http_app (const char * path)', 'http_find_stored_app_asset_filename (', 'http_app_installation_complete (void)'):
    if muss not in app: sys.exit('AUSZUG UNVOLLSTAENDIG: ' + muss)
if 'while (read_line (sHeaderLine))' not in kopf: sys.exit('AUSZUG OHNE KOPFZEILENSCHLEIFE')
