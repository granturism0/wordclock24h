#!/usr/bin/env python3
"""Sabotagen fuer c71i (Paket 2026-10-15, R1.3; AKI.3; design.md 8): jede muss FEHL melden (DIR-014), nicht OK und
nicht KAPUTT. Aufruf: sabotage.py <ESP-Quellverzeichnis>. Exit 0, wenn JEDE Sabotage anschlaegt.
Anker zeilenendenneutral (\\r?\\n), jeder muss genau so oft treffen wie angegeben."""
import os, re, shutil, subprocess, sys, tempfile
D = os.path.dirname(os.path.abspath(__file__)); Q = os.path.abspath(sys.argv[1])
VORHER = '            update_cache_vorher ();                                 // R1.3 I-6 (a): unmittelbar vor Schritt 1, Messpunkt v\n'
MARK13 = '            update_cache_heap_mark (update_cache_step);             // R1.3 I-6: nach jeder Versionszeile, auch ohne Erfolg\n'
S = [
  # (Name, [(Datei, alt, neu, Anzahl), ...]) -- die sieben aus AKI.3 / design.md 8 zuerst
  ('vorher nach Schritt 1', [('http.cpp', VORHER, '', 1),
      ('http.cpp', MARK13, MARK13 + '            if (update_cache_step == 1) { update_cache_vorher (); }\n', 1)]),
  ('ein Schritt ohne Messwert (4)', [('http.cpp', '            update_cache_heap_mark (4);                             // R1.3 I-6: nach httpclient_stop () und LittleFS.end ()\n', '', 1)]),
  ('nur blk statt free/blk', [('http.cpp', 'PSTR (" %c=%u/%u"), name,\n                                      (unsigned) update_cache_heap_free[i], (unsigned) update_cache_heap_blk[i]);',
                                          'PSTR (" %c=%u"), name,\n                                      (unsigned) update_cache_heap_blk[i]);', 1)]),
  ('nach ohne hc', [('http.cpp', 'PSTR (" %s hc=%u acc=%u"), tcp, (unsigned) httpclient_connects_get (),', 'PSTR (" %s acc=%u"), tcp,', 1)]),
  ('acc nicht als Differenz', [('http.cpp', '(unsigned) (uint16_t) (http_accept_count - update_cache_acc_start));', '(unsigned) http_accept_count);', 1)]),
  ('tmp geloescht fehlt', [('http.cpp', '                update_cache_tmp_note (tmp_before);                 // R1.3 I-6 (f): je tatsaechlich geloeschter Datei\n', '', 1)]),
  ('nur Serial', [('http.cpp', 'update_cache_i6_put (const char * line)\n{\n    Serial.println (line);\n    Serial.flush ();\n    stm32_log_append (line);\n',
                               'update_cache_i6_put (const char * line)\n{\n    Serial.println (line);\n    Serial.flush ();\n', 1)]),
  # weitere
  ('flash id nach EEPROM.begin ()', [('ESP-uclock.ino', '    EEPROM.begin(512);                                        // use 512 bytes as EEPROM\n',
                                       '    EEPROM.begin(512);                                        // use 512 bytes as EEPROM\n    { uint32_t x = ESP.getFlashChipId (); (void) x; }\n', 1)]),
  ('Messpunkt 1-3 nur bei Erfolg', [('http.cpp', MARK13, '', 1), ('http.cpp', '            snprintf_P (update_cache_key + k, sizeof (update_cache_key) - k, (update_cache_step == 1)',
                                     '            update_cache_heap_mark (update_cache_step);\n            snprintf_P (update_cache_key + k, sizeof (update_cache_key) - k, (update_cache_step == 1)', 1)]),
  ('Messpunkt 4 vor update_cache_write ()', [('http.cpp', '            update_cache_heap_mark (4);                             // R1.3 I-6: nach httpclient_stop () und LittleFS.end ()\n', '', 1),
      ('http.cpp', '            w = update_cache_write (RELEASENOTE_HTML,', '            update_cache_heap_mark (4);\n            w = update_cache_write (RELEASENOTE_HTML,', 1)]),
  ('kein Bericht beim Abbruch (Host leer)', [('http.cpp', '        update_cache_heap_report ();                                // R1.3 I-6: Abbruch eines laufenden Durchgangs (sonst nichts)\n', '', 1)]),
  ('kein Bericht am Ende', [('http.cpp', '    update_cache_heap_report ();                                    // R1.3 I-6 (b), (c): am Ende, auch nach Abbruch\n', '', 1)]),
  ('frag-Werte aus dem falschen Messpunkt', [('http.cpp', 'PSTR ("%u"), (unsigned) update_cache_heap_frag[i]);', 'PSTR ("%u"), (unsigned) update_cache_heap_frag[0]);', 1)]),
  ('ohne Kappung auf 65535', [('http.cpp', '    update_cache_heap_blk[point]  = (uint16_t) (blk > 0xFFFF ? 0xFFFF : blk);', '    update_cache_heap_blk[point]  = (uint16_t) blk;', 1)]),
  ('tmp-Zeile auch ohne Loeschen', [('http.cpp', '    uint8_t     gone = (uint8_t) (before & (uint8_t) ~update_cache_tmp_mask ());', '    uint8_t     gone = before | 3u;', 1)]),
  ('erste anfrage bei jedem accept ()', [('http.cpp', '    if (! http_first_request_logged)\n', '    if (1)\n', 1)]),
  ('erste anfrage nur Serial', [('http.cpp', '        Serial.flush ();\n        stm32_log_append (line);\n    }\n}\n\n/* R1.1 I-5 (design.md 3.2', '        Serial.flush ();\n    }\n}\n\n/* R1.1 I-5 (design.md 3.2', 1)]),
  ('http_accept_note () nicht im accept-Pfad', [('http.cpp', '    http_accept_note ();                                                    // R1.3 I-6: http_accept_count, "- heap erste anfrage"\n', '', 1)]),
  ('Flash-Kennung ohne Maske', [('ESP-uclock.ino', '(unsigned long) (flash_id & 0xFFFFFFUL)', '(unsigned long) flash_id', 1)]),
  ('privater Kopf umm_local.h', [('http.cpp', '#include <include/ClientContext.h>', '#include <umm_malloc/umm_local.h>\n#include <include/ClientContext.h>', 1)]),
]
rc = 0
for name, eingriffe in S:
    T = tempfile.mkdtemp(prefix='c71sab.')
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
    r = subprocess.run(['sh', os.path.join(D, 'run.sh'), q2, 'sab'], capture_output=True, text=True)
    fehler = [l.strip() for l in r.stdout.split('\n') if l.strip().startswith('FEHLER')]
    if r.returncode == 1: print(f"  schlaegt an: {name}  ({len(fehler)} Meldungen, z. B. {fehler[0][:110] if fehler else '-'})")
    else:
        last = [l for l in (r.stdout + r.stderr).strip().split('\n') if l.strip()][-2:]
        print(f"  BLIEB {'GRUEN' if r.returncode == 0 else 'KAPUTT'}: {name} (rc={r.returncode}, {' | '.join(last)[:200]})"); rc = 1
    shutil.rmtree(T)
print(f"{len(S)} Sabotagen, {'alle schlagen an' if rc == 0 else 'NICHT alle schlagen an'}")
sys.exit(rc)
