#!/usr/bin/env python3
"""Sabotagen fuer c65a, c65b, c65m, c65n, c65e, c65t und (E4) c65i, c65w, c65k, c65z, c65o: jede muss FEHL melden (DIR-014), nicht OK und nicht KAPUTT.
Aufruf: sabotage.py <ESP-Quellverzeichnis>. Exit 0, wenn JEDE Sabotage anschlaegt."""
import os, re, shutil, subprocess, sys, tempfile
D = os.path.dirname(os.path.abspath(__file__)); Q = os.path.abspath(sys.argv[1])
RUN = {'a': [os.path.join(D, 'run_a.sh')], 'b': [os.path.join(D, 'run_b.sh')],
       'm': [os.path.join(D, 'run_mn.sh')], 'n': [os.path.join(D, 'run_mn.sh')], 'e': [os.path.join(D, 'run_mn.sh')], 't': ['python3', os.path.join(D, 'c65t.py')],
       **{x: ['sh', os.path.join(D, 'run_x.sh'), x] for x in 'iwkzo'}}
S = [
  # (Name, Pruefstand, Datei, alt, neu, Anzahl)
  ('c65a Messpunkt accept entfernt', 'a', 'http.cpp', '    heap_low_note (HEAP_LOW_PATH_ACCEPT);', '', 1),
  ('c65a Messpunkt client entfernt', 'a', 'httpclient.cpp', '    heap_low_note (HEAP_LOW_PATH_CLIENT);', '', 1),
  ('c65a Lesen ohne Parameter setzt zurueck', 'a', 'http.cpp', '    if (reset_param[0] && strcmp (reset_param, "0"))', '    if (1)', 1),
  ('c65a Query im Pfad (Anfragezeile UND Kopierschritt)', 'a', [('http.cpp', "path[n] && path[n] != '?' && n <", 'path[n] && n <', 1),
      ('http.cpp', 'sPath  = sRequest.substring(start_position, end_position_question);', 'sPath  = sRequest.substring(start_position, end_position_space);', 2)]),
  ('c65a I-3 nur im Regelfall', 'a', 'http.cpp', '    HttpRequestTimer request_timer (sPath);', '', 1),
  ('c65b fs ohne Ring', 'b', 'http.cpp', '                        stm32_log_append (line);\n                    }\n\n                    rtc = 2;', '                    }\n\n                    rtc = 2;', 1),
  ('c65b fs ohne Zaehler fs_write_short', 'b', 'http.cpp', '                        http_fs_write_short++;', '', 1),
  ('c65b snd vor dem Versuch erhoben', 'b', 'http.cpp', '                lost_snd = http_client.availableForWrite ();', '                lost_snd = 1072;', 1),
  ('c65m warm holt trotzdem', 'm', 'http.cpp', '            && (millis () - update_tables_cache_millis) < update_tables_cache_window', '            && 0', 1),
  ('c65m abgerissen wird abgelegt', 'm', 'http.cpp', '                complete = (len == 0);', '                complete = 1;', 1),
  ('c65m Liste gekuerzt abgelegt', 'm', 'http.cpp', '                                    buffering = 0;                                         // ab hier live wie bis 3.2.35', '                                    buffering = 1; used = 0;', 1),
  ('c65n Umbenennen auch ohne vollstaendiges Lesen', 'n', 'http.cpp', '        if (! reason && ! LittleFS.rename (tmpname, fname))', '        if (! LittleFS.rename (tmpname, fname))', 1),
  ('c65n Hardware im Schluessel', 'n', 'http.cpp', 'PSTR ("|%04x"), (unsigned) update_checksum_text (0, host, path));', 'PSTR ("|%04x"), (unsigned) update_source_checksum (host, path));', 1),
  ('c65n ungueltige Notes-Datei setzt keinen Auftrag', 'n', 'http.cpp', '    else\n    {\n        update_cache_due = 1;\n    }\n\n    LittleFS.end ();\n\n    http_send (FS("\\""));', '\n    LittleFS.end ();\n\n    http_send (FS("\\""));', 1),
  ('c65n Host-Rueckfall auf DEFAULT_UPDATE_HOST', 'n', 'http.cpp', '    host = get_strvar (UPDATE_HOST_VAR)->str;\n    path = get_strvar (UPDATE_PATH_VAR)->str;\n', '    host = get_strvar (UPDATE_HOST_VAR)->str[0] ? get_strvar (UPDATE_HOST_VAR)->str : DEFAULT_UPDATE_HOST;\n    path = get_strvar (UPDATE_PATH_VAR)->str;\n', 1),
  ('c65t Zeile nach accept entfernt', 't', 'http.cpp', '    http_client.setTimeout (HTTP_WRITE_TIMEOUT_MS);', '', 1),
  ('c65t Zeile vor OTA (API) entfernt', 't', 'http.cpp', '    http_client.setTimeout (HTTP_OTA_TIMEOUT_MS);                   // E3 M-3 (Ent-25): nie mit der 1-s-Grenze der Anfragen\n    t_httpUpdate_return', '    t_httpUpdate_return', 1),
  ('c65t Zeile vor OTA (Legacy) entfernt', 't', 'http.cpp', '                http_client.setTimeout (HTTP_OTA_TIMEOUT_MS);                   // E3 M-3 (Ent-25): nie mit der 1-s-Grenze der Anfragen\n', '', 1),
  ('c65t Datei mit 1000 statt 2000', 't', 'http.cpp', '            http_client.setTimeout (HTTP_FILE_WRITE_TIMEOUT_MS);', '            http_client.setTimeout (HTTP_WRITE_TIMEOUT_MS);', 1),
  # E3.8 (Review-Auflagen M1, M2, N4 und upd_cache_max_ms), Lead 14.10.2026
  ('c65e M1/M2 erster Fehlschlag beendet den Durchgang nicht', 'e', 'http.cpp', '                update_cache_failed = 1;\n                update_cache_finish ();\n                return;\n            }\n\n            snprintf_P (update_cache_key + k', '            }\n\n            snprintf_P (update_cache_key + k', 1),
  ('c65e M2 update_status bildet den Schluessel auch mit leerem Feld', 'e', 'http.cpp', 'if (esp_len > 0 && app_len > 0 && wc_len > 0)', 'if (esp_len > 0 || app_len > 0 || wc_len > 0)', 1),
  ('c65e M1 Liste trotz Server-Fehlschlag', 'e', 'http.cpp', '            if (w == 2)                                             // Server antwortet nicht: Liste nicht mehr versuchen (M1)', '            if (0)', 1),
  ('c65e N4 .tmp nicht geloescht', 'e', 'http.cpp', '                if (LittleFS.exists (UPDATE_CACHE_NOTES_TMP))\n                {\n                    LittleFS.remove (UPDATE_CACHE_NOTES_TMP);\n                }\n\n                if (LittleFS.exists (UPDATE_CACHE_LIST_TMP))\n                {\n                    LittleFS.remove (UPDATE_CACHE_LIST_TMP);\n                }', '', 1),
  ('c65e Maximum des Auftrags nicht nachgefuehrt', 'e', 'http.cpp', '        req_max_upd_cache_ms = (uint32_t) (millis () - update_cache_step_millis);', '', 1),
  ('c65a upd_cache_max_ms nicht zurueckgesetzt', 'a', 'http.cpp', '        req_max_upd_cache_ms = 0;                                   // E3.8: mit I-1 und I-3', '', 1),
  ('c65a upd_cache_max_ms gibt den Live-Wert aus', 'a', 'http.cpp', '(unsigned long) upd_max_ms);', '(unsigned long) req_max_upd_cache_ms);', 1),
  # E4 (design-e4.md 19.11): je Massnahme mindestens eine Sabotage
  ('c65i Momentaufnahme bei jedem Aufruf', 'i', 'http.cpp', '        heap_low_snapshot ();                                   // E4 I-4: NUR beim neuen Block-Tiefstwert\n    }\n', '    }\n    heap_low_snapshot ();\n', 1),
  ('c65i Query im Pfad', 'i', 'http.cpp', "path[idx] && path[idx] != '?' && idx <", 'path[idx] && idx <', 1),
  ('c65i Scan-Zeile nur auf Serial', 'i', 'http.cpp', '    Serial.println (buf);\n    Serial.flush ();\n    stm32_log_append (buf);\n', '    Serial.println (buf);\n    Serial.flush ();\n', 1),
  ('c65i flush nach scanDelete gemessen', 'i', [('http.cpp', '    out_end    = millis ();', '    WiFi.scanDelete ();\n    out_end    = millis ();', 1)]),
  ('c65w 1072 je Aufruf', 'w', 'http.cpp', '#define HTTP_WRITE_CHUNK                            TCP_MSS', '#define HTTP_WRITE_CHUNK                            MAX_HTTP_RESPONSE_LEN', 1),
  ('c65w Teilstueck ohne Nachversuch', 'w', 'http.cpp', '                if (written == 0 || retried || ! http_client.connected ())', '                if (1)', 1),
  ('c65w Nachversuch je Stueck statt je Aufruf', 'w', 'http.cpp', '                    continue;                                   // Stueck ganz untergebracht', '                    retried = 0; continue;', 1),
  ('c65k Hysterese entfernt', 'k', 'http.cpp', '#define HTTP_BACKLOG_HYST                       256', '#define HTTP_BACKLOG_HYST                       0', 1),
  ('c65k Nachfuehrung entfernt', 'k', 'http.cpp', '    http_backlog_track ();                                                  // E4 Teil 3: Backlog nach freiem Heap, VOR accept ()\n', '', 1),
  ('c65k bei jedem Durchlauf gesetzt', 'k', 'http.cpp', '    if (want != http_backlog)\n    {\n        http_server.backlog (want);', '    {\n        http_server.backlog (want);', 1),
  ('c65z Grenze 5', 'z', 'http.cpp', '#define HTTP_TW_LIMIT                           2', '#define HTTP_TW_LIMIT                           5', 1),
  ('c65z Makro mit der Grenze selbst (laesst 3)', 'z', 'http.cpp', '    TCP_TW_LIMIT (HTTP_TW_LIMIT - 1);', '    TCP_TW_LIMIT (HTTP_TW_LIMIT);', 1),
  ('c65z Aufruf nach dem Messpunkt', 'z', 'http.cpp', '    tcp_tw_limit ();                                                        // E4 Teil 5: TIME_WAIT <= 2, VOR dem Messpunkt\n    heap_low_note (HEAP_LOW_PATH_ACCEPT);', '    heap_low_note (HEAP_LOW_PATH_ACCEPT);\n    tcp_tw_limit ();', 1),
  ('c65z Aufruf vor connect () entfernt', 'z', 'httpclient.cpp', '    tcp_tw_limit ();                                                                // E4 Teil 5 (C65): TIME_WAIT <= 2, unmittelbar VOR connect ()\n', '', 1),
  ('c65o ldr_min_set in der Liste', 'o', 'http.cpp', '    "/api/ir_codes_get",\n};', '    "/api/ir_codes_get",\n    "/api/ldr_min_set",\n};', 1),
  ('c65o Pruefung nach dem Verteiler', 'o', [
      ('http.cpp', '    if (http_api_is_read_only (path) && ! http_api_orphan_trigger (path) && ! http_client.connected ())\n    {\n        if (api_orphan_skipped < 0xFFFF)\n        {\n            api_orphan_skipped++;\n        }\n\n        return 0;\n    }\n\n', '', 1),
      ('http.cpp', '        http_send_FS ("\\r\\n\\r\\n404 Not Found\\r\\n");\n        http_flush ();\n    }\n\n    return rtc;\n}', '        http_send_FS ("\\r\\n\\r\\n404 Not Found\\r\\n");\n        http_flush ();\n    }\n\n    if (http_api_is_read_only (path) && ! http_client.connected ())\n    {\n        api_orphan_skipped++;\n    }\n\n    return rtc;\n}', 1)]),
  # E4.5a (Lead 14.10.2026): Abgleich von Notes und Liste nach dem Start und nach refresh=1
  ('c65e Abgleich ohne Inhaltsvergleich (immer geschrieben)', 'e', 'http.cpp', '    if (compare)\n    {\n        File        old;', '    if (0)\n    {\n        File        old;', 1),
  ('c65e kein Abgleich beim Start', 'e', 'http.cpp', 'static uint8_t       update_cache_force        = 1;', 'static uint8_t       update_cache_force        = 0;', 1),
  ('c65e refresh=1 fordert keinen Abgleich an', 'e', 'http.cpp', '        update_cache_due   = 1;\n        update_cache_force = 1;\n', '        update_cache_due   = 1;\n', 1),
  ('c65e gleicher Anfang nicht aus der alten Datei', 'e', 'http.cpp', '                    while (! reason && same > 0)', '                    while (0)', 1),
  ('c65e Abgleich nach Fehlschlag nicht vorgemerkt', 'e', 'http.cpp', '        if (update_cache_forcing)\n        {\n            update_cache_force = 1;                                 // E4.5a: der Abgleich steht weiter aus\n        }\n', '', 1),
  # Review E4.10 M1: update_status?refresh=1 ist ein Ausloeser
  ('c65o update_status?refresh=1 verwaist uebersprungen', 'o', 'http.cpp', '    if (http_api_is_read_only (path) && ! http_api_orphan_trigger (path) && ! http_client.connected ())', '    if (http_api_is_read_only (path) && ! http_client.connected ())', 1),
]
rc = 0
for e in S:
    name, w = e[0], e[1]
    eingriffe = e[2] if isinstance(e[2], list) else [tuple(e[2:6])]
    T = tempfile.mkdtemp(prefix='c65sab.')
    q2 = os.path.join(T, 'q'); shutil.copytree(Q, q2, ignore=shutil.ignore_patterns('data', 'build*', '*.bin'))
    kaputt = False
    for f, a, n, cnt in eingriffe:
        p = os.path.join(q2, f); b = open(p, 'rb').read()
        rx = re.compile(b'\r?\n'.join(re.escape(x.encode()) for x in a.split('\n')))
        found = len(rx.findall(b))
        if found != cnt:
            print(f"  ANKER FEHLT ({found} statt {cnt}): {name}"); kaputt = True; break
        b = rx.sub(n.encode().replace(b'\\', b'\\\\'), b)
        open(p, 'wb').write(b)
    if kaputt:
        rc = 1; shutil.rmtree(T); continue
    # run_mn.sh braucht den Typ als drittes Argument -- fuer m, n UND e (c65e, E3.8). Fehlte er,
    # lief stillschweigend c65m, und vier Sabotagen an c65e blieben gruen (Lead 14.10.2026).
    args = RUN[w] + [q2, 'sab'] + ([w] if w in 'mne' else [])
    r = subprocess.run(args, capture_output=True, text=True)
    last = [l for l in r.stdout.strip().split('\n') if l.strip()][-1:] or ['']
    if r.returncode == 1: print(f"  schlaegt an: {name}")
    else: print(f"  BLIEB {'GRUEN' if r.returncode == 0 else 'KAPUTT'}: {name} (rc={r.returncode}, {last[0][:100]})"); rc = 1
    shutil.rmtree(T)
print(f"{len(S)} Sabotagen, {'alle schlagen an' if rc == 0 else 'NICHT alle schlagen an'}")
sys.exit(rc)
