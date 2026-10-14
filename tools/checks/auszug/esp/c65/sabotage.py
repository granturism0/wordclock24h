#!/usr/bin/env python3
"""Sabotagen fuer c65a, c65b, c65m, c65n, c65e, c65t: jede muss FEHL melden (DIR-014), nicht OK und nicht KAPUTT.
Aufruf: sabotage.py <ESP-Quellverzeichnis>. Exit 0, wenn JEDE Sabotage anschlaegt."""
import os, re, shutil, subprocess, sys, tempfile
D = os.path.dirname(os.path.abspath(__file__)); Q = os.path.abspath(sys.argv[1])
RUN = {'a': [os.path.join(D, 'run_a.sh')], 'b': [os.path.join(D, 'run_b.sh')],
       'm': [os.path.join(D, 'run_mn.sh')], 'n': [os.path.join(D, 'run_mn.sh')], 'e': [os.path.join(D, 'run_mn.sh')], 't': ['python3', os.path.join(D, 'c65t.py')]}
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
