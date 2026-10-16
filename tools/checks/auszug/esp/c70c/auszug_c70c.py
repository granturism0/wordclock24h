#!/usr/bin/env python3
"""Auszug fuer den Pruefstand c70c (Paket 2026-10-15, R2b.1, C70 Variante (c); AKL.1-AKL.3; design.md 18.6).
Aufruf: auszug_c70c.py <ESP-Quellverzeichnis> <ziel> <c65-Verzeichnis>

Ruft zuerst den Auszug von c65 (defs.inc, zaehler.inc, i1.inc, i4.inc, flush.inc, have.inc - die Auslieferung ueber
http_flush (), die Zaehler), dann schneidet er ECHTEN Code aus:
  par.inc     aus http.cpp MAX_HTTP_PARAMS, HTTP_PARAMETERS, http_parameters[], normalize_http_parameters (),
              http_set_params (), http_get_param (); aus base.cpp htoi () - der Weg der Marke h von der Query bis
              in http_send_fs_file ()
  c70.inc     den C70-Block von "#define APP_INM_LEN" bis zum Ende von http_app_if_none_match_hits () - ab R2b mit
              APP_ASSET_LISTED, APP_ASSET_VERSIONED und http_app_token_hits ()
  fs.inc      http_fs_file_exists_and_nonempty () und http_send_fs_file ()
  app.inc     PWA_PREFIX, PWA_INDEX_FILE, APP_INSTALL_ASSETS, http_content_type (), app_asset_filename (),
              http_find_app_install_asset (), http_app_asset_supports_gzip (), app_asset_storage_filename (),
              http_find_stored_app_asset_filename (), http_app_installation_complete (), http_app ()
  kopf.inc    den GET-Zweig von http_server_loop () bis vor http_log_request () - die Kopfzeilenschleife
  vorhttp.inc http () von der Kopfzeile bis VOR "if (hardware_configuration == 0xFFFF)": Laengengrenze, Kopie in
              param[256], '+' als Leerzeichen, http_set_params () - so wie jede Anfrage an /app/ ihn durchlaeuft
Ohne die R2b-Zaehler (Stand R2, 3.2.40) fehlt C70C_HAVE_ZAEHLER in c70chave.inc: Der Pruefstand uebersetzt mit Ersatz
und meldet FEHL, nicht KAPUTT.
"""
import os, re, subprocess, sys
q, ziel, c65 = sys.argv[1], sys.argv[2], sys.argv[3]
r = subprocess.run([sys.executable, os.path.join(c65, 'auszug.py'), q, ziel], capture_output=True, text=True)
if r.returncode:
    sys.exit('c65-Auszug: ' + (r.stdout + r.stderr).strip())
def lesen(name):
    return open(os.path.join(q, name), 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n')
H = lesen('http.cpp'); L = H.split('\n')
BL = lesen('base.cpp').split('\n')

def func(kopf, LL=L):
    i = next((k for k, l in enumerate(LL) if l.startswith(kopf) and not l.rstrip().endswith(';')), None)
    if i is None: return ''
    j = next(k for k in range(i, len(LL)) if LL[k].startswith('}'))
    return '\n'.join(LL[i - 1:j + 1]) + '\n'
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
if re.search(r'^static uint16_t +http_app_immutable\b', H, re.M) and re.search(r'^static uint16_t +http_app_token_miss\b', H, re.M):
    have.append('C70C_HAVE_ZAEHLER')
par = ''.join(l + '\n' for l in L if re.match(r'^#define MAX_HTTP_PARAMS\s', l))
# HTTP_PARAMETERS: das typedef unmittelbar vor "} HTTP_PARAMETERS;"
k = next(i for i, l in enumerate(L) if l.startswith('} HTTP_PARAMETERS;'))
s = max(i for i in range(k) if L[i].startswith('typedef struct'))
par += '\n'.join(L[s:k + 1]) + '\n'
par += ''.join(l + '\n' for l in L if re.match(r'^static HTTP_PARAMETERS\s+http_parameters\[MAX_HTTP_PARAMS\];', l))
par += func('htoi (char * buf, uint8_t max_digits)', BL).replace('uint16_t\nhtoi', 'static uint16_t\nhtoi')
par += func('normalize_http_parameters (char * p)') + func('http_set_params (char * paramlist)') + func('http_get_param (const char * name)')
put('par.inc', par)
c70 = von_bis_funktion('#define APP_INM_LEN', 'http_app_if_none_match_hits (')
put('c70.inc', c70)
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
i = next((k for k, l in enumerate(L) if l.startswith('    else // GET')), None)
j = next((k for k in range(i, len(L)) if 'http_log_request (sRemoteIp' in L[k]), None) if i is not None else None
kopf = '\n'.join(L[i:j]) + '\n' if j is not None else ''
put('kopf.inc', kopf)
# http (): von der Kopfzeile bis vor die Hardwareerkennung; der Pruefstand schliesst mit "return http_app (path); }"
hk = next((k for k, l in enumerate(L) if l.startswith('http (const char * path, const char * const_param)')), None)
he = next((k for k in range(hk, len(L)) if L[k].startswith('    if (hardware_configuration == 0xFFFF)')), None) if hk is not None else None
vorhttp = '\n'.join(L[hk - 1:he]) + '\n' if he is not None else ''
put('vorhttp.inc', vorhttp.replace('uint_fast8_t\nhttp (const char * path', 'static uint_fast8_t\nhttp (const char * path', 1))
put('c70chave.inc', ''.join('#define %s 1\n' % m for m in have))
for name, s in (('par.inc', par), ('c70.inc', c70), ('fs.inc', fs), ('app.inc', app), ('kopf.inc', kopf), ('vorhttp.inc', vorhttp)):
    if not s.strip(): sys.exit('AUSZUG LEER: ' + name)
for muss in ('http_set_params (char * paramlist)', 'http_get_param (const char * name)', 'normalize_http_parameters (char * p)', 'htoi (char * buf', 'MAX_HTTP_PARAMS', '} HTTP_PARAMETERS;', 'http_parameters[MAX_HTTP_PARAMS]'):
    if muss not in par: sys.exit('AUSZUG UNVOLLSTAENDIG (par.inc): ' + muss)
for muss in ('http_send_fs_file (', 'http_fs_file_exists_and_nonempty ('):
    if muss not in fs: sys.exit('AUSZUG UNVOLLSTAENDIG: ' + muss)
for muss in ('APP_INSTALL_ASSETS[]', 'http_app (const char * path)', 'http_find_stored_app_asset_filename (', 'http_app_installation_complete (void)'):
    if muss not in app: sys.exit('AUSZUG UNVOLLSTAENDIG: ' + muss)
for muss in ('http_set_params (param);', 'strncpy (param, const_param, 255);'):
    if muss not in vorhttp: sys.exit('AUSZUG UNVOLLSTAENDIG (vorhttp.inc): ' + muss)
if 'while (read_line (sHeaderLine))' not in kopf: sys.exit('AUSZUG OHNE KOPFZEILENSCHLEIFE')
print('auszug c70c: ' + ', '.join(have or ['ohne R2b-Zaehler']))
