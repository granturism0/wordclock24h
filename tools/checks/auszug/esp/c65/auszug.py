#!/usr/bin/env python3
"""Auszug fuer die Pruefstaende c65a/b/m/n (Paket 2026-10-13, E3). Aufruf: auszug.py <ESP-Quellverzeichnis> <ziel> [praefix]

Schneidet den ECHTEN Code aus http.cpp, httpclient.cpp, base.h und ESP-uclock.ino aus. Fehlt ein Baustein (Ausgangsstand
3.2.35), bleibt sein Auszug leer und in <ziel>/<praefix>have.inc fehlt das passende HAVE_-Makro: Der Pruefstand
uebersetzt dann mit Ersatz und meldet FEHL, nicht KAPUTT.
"""
import os, re, sys
q, ziel = sys.argv[1], sys.argv[2]
px = sys.argv[3] if len(sys.argv) > 3 else ''
os.makedirs(ziel, exist_ok=True)

def text(name):
    p = os.path.join(q, name)
    return open(p, 'rb').read().decode('utf-8').replace('\r\n', '\n') if os.path.exists(p) else ''

H, C, I, B = text('http.cpp'), text('httpclient.cpp'), text('ESP-uclock.ino'), text('base.h')
have = []

def func(s, kopf):
    """Funktion, deren Kopfzeile (Spalte 0) mit kopf beginnt, samt Rueckgabetyp-Zeile davor, bis '}' in Spalte 0."""
    L = s.split('\n')
    for i, l in enumerate(L):
        if l.startswith(kopf):
            j = i
            while not L[j].startswith('}'):
                j += 1
            return '\n'.join(L[i - 1:j + 1]) + '\n'
    return ''

def block(s, start, ende):
    L = s.split('\n')
    i = next((k for k, l in enumerate(L) if l.startswith(start)), None)
    if i is None: return ''
    j = next((k for k in range(i, len(L)) if L[k].startswith(ende)), None)
    return '\n'.join(L[i:j + 1]) + '\n' if j is not None else ''

def block_vor(s, start, ende):
    """wie block, aber start ist die LETZTE passende Zeile vor ende"""
    L = s.split('\n')
    j = next((k for k, l in enumerate(L) if l.startswith(ende)), None)
    if j is None: return ''
    i = max(k for k in range(j) if L[k].startswith(start))
    return '\n'.join(L[i:j + 1]) + '\n'

def lines(s, rx):
    r = re.compile(rx)
    return ''.join(l + '\n' for l in s.split('\n') if r.search(l))

def put(name, inhalt, makro=None):
    open(os.path.join(ziel, px + name), 'w').write(inhalt)
    if makro and inhalt.strip():
        have.append(makro)

# ---------------- gemeinsam
put('defs.inc', lines(H, r'^#define (HTTP_FIRST_LINE_TIMEOUT|HTTP_DEBUG_CLIENT_LOG|HTTP_WRITE_TIMEOUT_MS|HTTP_FILE_WRITE_TIMEOUT_MS|HTTP_OTA_TIMEOUT_MS|MAX_UPDATE_FILENAME_LEN|UPDATE_[A-Z_]+|DEFAULT_UPDATE_(HOST|PATH)|ESP_WORDCLOCK_TXT|APP_VERSION_TXT|RELEASENOTE_HTML|WC_TXT|WC_LIST_TXT|WC_TABLES_LIST_TXT|POST_(ICON|ICON_WEATHER|TABLES|DISPLAY)_FILE)\s')
              + lines(B, r'^#define HEAP_LOW_PATH_'))
put('i1.inc', lines(H, r'^static (uint32_t|uint8_t) +(heap_low_|heap_line_low)')
              + func(H, 'heap_low_note (') + func(H, 'heap_low_line_take (') + func(H, 'heap_low_path_name ('), 'HAVE_I1')
put('i3.inc', lines(H, r'^static (uint32_t|char|unsigned long|uint_fast8_t|uint8_t) +(req_max_|http_last_request_millis|http_request_seen)')
              + func(H, 'http_request_note (') + block(H, 'class HttpRequestTimer', '};'), 'HAVE_I3')
put('zaehler.inc', block(H, '#define MAX_HTTP_RESPONSE_LEN', 'static uint_fast8_t http_write_broken')
              + lines(H, r'^static uint16_t +http_no_request_(timeouts|aborts)'))
if re.search(r'^static uint16_t +http_fs_write_short', H, re.M): have.append('HAVE_FS_WRITE_SHORT')
put('device_ready.inc', func(H, 'http_api_device_ready ('))
put('server_loop.inc', func(H, 'http_server_loop ('))
put('client.inc', lines(C, r'^static uint16_t +httpclient_connects') + func(C, 'httpclient (') + func(C, 'httpclient_connects_get ('))
if re.search(r'^static uint16_t +httpclient_connects', C, re.M): have.append('HAVE_CONNECTS')
heap = func(I, 'esp_heap_log (')
put('heaplog.inc', next((l + '\n' for l in I.split('\n') if l.startswith('static uint32_t       esp_uart_overruns')), '') + ('static void\n' + heap.split('\n', 1)[1] if heap else ''))
# ---------------- I-2
put('flush.inc', func(H, 'http_flush (void)') + func(H, 'http_concat_response (') + func(H, 'http_send (const char * s)'))
put('fsfile.inc', func(H, 'http_fs_file_exists_and_nonempty (') + func(H, 'http_send_fs_file ('))
# ---------------- M-1
put('upd_base.inc', lines(H, r'^static (char|unsigned long|uint16_t|uint_fast8_t|uint8_t) +update_(cache|server)_')
              + func(H, 'update_checksum_text (') + func(H, 'update_source_checksum (')
              + func(H, 'update_server_blocked (') + func(H, 'update_server_note (') + func(H, 'update_server_open (')
              + func(H, 'update_server_fetch_line (') + func(H, 'http_fetch_remote_line ('))
put('upd_text.inc', block_vor(H, 'typedef struct', '} HTTP_JSON_TEXT_OUT;') + func(H, 'http_json_text_put (') + func(H, 'http_json_text_flush ('), 'HAVE_TEXT_OUT')
put('upd_live_notes.inc', func(H, 'http_json_send_remote_text_field ('))
put('upd_tables.inc', lines(H, r'^static (char|uint16_t|uint_fast8_t|uint8_t|unsigned long) +update_(tables_cache|list_too_big)')
              + func(H, 'http_tables_send_entry (') + func(H, 'http_tables_send_cached (') + func(H, 'update_list_too_big_note (')
              + func(H, 'http_api_update_table_files ('), )
if 'update_tables_cache[' in H: have.append('HAVE_TABLES_CACHE')
put('upd_cache.inc', ''.join(func(H, n + ' (') for n in (
    'update_cache_line', 'update_cache_open', 'update_cache_file_matches', 'update_cache_space_ok', 'update_cache_write',
    'update_cache_finish', 'update_cache_step_run', 'update_cache_poll', 'update_cache_send_notes', 'update_cache_send_stm32_list')), 'HAVE_CACHE')
put('upd_status.inc', func(H, 'http_api_update_status (') + func(H, 'http_json_send_remote_update_support_fields (')
              + func(H, 'http_json_send_remote_update_url_fields ('))
put('upd_helpers.inc', func(H, 'app_asset_filename (') + func(H, 'app_asset_storage_filename (')
              + func(H, 'http_json_send_field_prefix (') + func(H, 'http_json_send_bool_field (')
              + func(H, 'http_json_send_uint_field (') + func(H, 'http_json_send_string_field ('))
put('fsexists.inc', func(H, 'http_fs_file_exists_and_nonempty ('))
open(os.path.join(ziel, px + 'have.inc'), 'w').write(''.join(f'#define {px.upper()}{m} 1\n' for m in have))
for k in ('device_ready.inc', 'server_loop.inc', 'flush.inc', 'fsfile.inc', 'upd_status.inc', 'upd_base.inc'):
    if not open(os.path.join(ziel, px + k)).read().strip():
        sys.exit(f'AUSZUG LEER: {k}')
