#!/usr/bin/env python3
"""Sabotagen fuer c70e (Paket 2026-10-15, R2.1, C70; AKC.1-AKC.3, AKC.7; design.md 8): jede muss FEHL melden
(DIR-014), nicht OK und nicht KAPUTT. Aufruf: sabotage.py <ESP-Quellverzeichnis>. Exit 0, wenn JEDE anschlaegt.
Anker zeilenendenneutral (\\r?\\n), jeder muss genau so oft treffen wie angegeben."""
import os, re, shutil, subprocess, sys, tempfile
D = os.path.dirname(os.path.abspath(__file__)); Q = os.path.abspath(sys.argv[1])
H = 'http.cpp'
ETAG_200 = '''                if (etag[0])
                {
                    http_send (FS("\\r\\nETag: "));
                    http_send (etag);
                }
'''
S = [
  # die fuenf aus dem Auftrag zuerst
  ('ETag fehlt in der 200', [(H, ETAG_200, '', 1)]),
  ('304 mit Rumpf (Leseschleife laeuft auch nach der 304)', [(H, 'while (! not_modified && ! http_write_broken && fp.available ())', 'while (! http_write_broken && fp.available ())', 1)]),
  ('304 als HTTP/1.0', [(H, 'FS("HTTP/1.1 304 Not Modified', 'FS("HTTP/1.0 304 Not Modified', 1)]),
  ('falsches ETag ergibt 304 (nur die Laenge verglichen)', [(H, 'if ((size_t) (z - p) == n && ! memcmp (p, etag, n))', 'if ((size_t) (z - p) == n)', 1)]),
  ('leere .gz ergibt 304 (Groessenpruefung weg, ETag nur aus der Groesse)', [
      (H, '            rtc = (fp.size () > 0);\n', '            rtc = true;\n', 1),
      (H, "    etag[0] = '\\0';\n\n    if (size < APP_GZIP_MIN)", "    etag[0] = '\\0';\n    snprintf_P (etag, etag_size, PSTR (\"\\\"%lu\\\"\"), (unsigned long) size); return true;\n\n    if (size < APP_GZIP_MIN)", 1)]),
  # weitere aus design.md 8 und AKC.3/AKC.7
  ('leere .gz: Groessenpruefung weg (200 mit leerem Rumpf)', [(H, '            rtc = (fp.size () > 0);\n', '            rtc = true;\n', 1)]),
  ('ETag nur aus der Groesse', [(H, "    etag[0] = '\\0';\n\n    if (size < APP_GZIP_MIN)", "    etag[0] = '\\0';\n    snprintf_P (etag, etag_size, PSTR (\"\\\"%lu\\\"\"), (unsigned long) size); return true;\n\n    if (size < APP_GZIP_MIN)", 1)]),
  ('304 ohne Groessenpruefung (exists statt exists_and_nonempty in http_send_fs_file)', [(H, '''    LittleFS.begin ();

    /* Nicht LittleFS.exists(): Eine 0-Byte-Datei existiert und wuerde mit 200 OK''', '''    LittleFS.begin ();
#define http_fs_file_exists_and_nonempty(f) LittleFS.exists (f)
    /* Nicht LittleFS.exists(): Eine 0-Byte-Datei existiert und wuerde mit 200 OK''', 1)]),
  ('Connection: close in der 304 entfernt', [(H, '''                http_send (etag);
                http_send (immutable ? FS("\\r\\nCache-Control: public, max-age=31536000, immutable\\r\\nConnection: close\\r\\n\\r\\n")
                                     : FS("\\r\\nCache-Control: no-cache\\r\\nConnection: close\\r\\n\\r\\n"));

                if (http_app_not_modified < 0xFFFF)''', '''                http_send (etag);
                http_send (immutable ? FS("\\r\\nCache-Control: public, max-age=31536000, immutable\\r\\n\\r\\n")
                                     : FS("\\r\\nCache-Control: no-cache\\r\\n\\r\\n"));

                if (http_app_not_modified < 0xFFFF)''', 1)]),
  ('Connection: close in der 200 entfernt', [(H, '''                }

                http_send (immutable ? FS("\\r\\nCache-Control: public, max-age=31536000, immutable\\r\\nConnection: close\\r\\n\\r\\n")
                                     : FS("\\r\\nCache-Control: no-cache\\r\\nConnection: close\\r\\n\\r\\n"));
            }''', '''                }

                http_send (immutable ? FS("\\r\\nCache-Control: public, max-age=31536000, immutable\\r\\n\\r\\n")
                                     : FS("\\r\\nCache-Control: no-cache\\r\\n\\r\\n"));
            }''', 1)]),
  ('HTTP/1.1 in einer /api/-Antwort (device_ready)', [(H, '''    http_flush ();
    return 0;
}

static void
http_json_send_remote_update_support_fields ()''', '''    http_flush ();
    return 0;
}

static const char * sab_api = "HTTP/1.1 200 OK\\r\\nContent-Type: application/json\\r\\n\\r\\n";
static void
http_json_send_remote_update_support_fields ()''', 1)]),
  ('HTTP/1.1 fuer jede Datei (Weissliste ignoriert)', [(H, '    uint_fast8_t    cache = (app_asset && gzip_encoded);', '    uint_fast8_t    cache = (gzip_encoded);', 1)]),
  ('Weissliste in http_app () ignoriert', [(H, '        app_asset = (http_find_app_install_asset (asset_path) != 0);', '        app_asset = 1;', 1)]),
  ('Index ohne app_complete ausgeliefert', [(H, '    if (! is_pwa_index || app_complete)\n    {\n        sent = http_send_fs_file', '    if (1)\n    {\n        sent = http_send_fs_file', 1)]),
  ('seek zurueck: Rueckgabe ignoriert', [(H, "    if (! fp.seek (0, SeekSet))\n    {\n        etag[0] = '\\0';\n        return false;\n    }", "    fp.seek (0, SeekSet);", 1)]),
  ('kein seek zurueck an den Anfang', [(H, "    if (! fp.seek (0, SeekSet))\n    {\n        etag[0] = '\\0';\n        return false;\n    }", "", 1)]),
  ('Abbruch mit FIN statt RST', [(H, '                fp.close ();\n                http_client.abort ();', '                fp.close ();\n                http_client.stop ();', 1)]),
  ('* trifft', [(H, '''    if (! n)
    {
        return false;
    }

    while (*p)''', '''    if (! n)
    {
        return false;
    }

    if (! strcmp (p, "*"))
    {
        return true;
    }

    while (*p)''', 1)]),
  ('W/ nicht uebergangen', [(H, "        if (p[0] == 'W' && p[1] == '/')                         // schwaches ETag", "        if (0)                         // schwaches ETag", 1)]),
  ('Kopfname mit Gross-/Kleinschreibung verglichen', [(H, 'if (strncasecmp_P (p, PSTR ("If-None-Match:"), 14))', 'if (strncmp_P (p, PSTR ("If-None-Match:"), 14))', 1)]),
  ('ueberlanger Wert abgeschnitten statt verworfen', [(H, '''    if (n >= APP_INM_LEN)                                       // ueberlang: nicht ablegen, trifft damit nie
    {
        n = 0;
    }''', '''    if (n >= APP_INM_LEN)                                       // ueberlang: nicht ablegen, trifft damit nie
    {
        n = APP_INM_LEN - 1;
    }''', 1)]),
  ('Puffer nicht je Anfrage geleert', [(H, '        http_clear_request_if_none_match ();                                // R2 C70', '        // R2 C70', 1)]),
  ('If-None-Match in der Kopfzeilenschleife nicht erfasst', [(H, '                http_capture_request_if_none_match (sHeaderLine);', '                ;', 1)]),
  ('gzip-Kennung nicht geprueft', [(H, 'fp.read (t, 2) == 2 && t[0] == 0x1f && t[1] == 0x8b &&', 'fp.read (t, 2) == 2 &&', 1)]),
  ('Zaehler fehlen in device_ready', [(H, '''    http_json_send_uint_field (FS("app_not_modified"), (unsigned long) http_app_not_modified);      // R2 C70: 304 auf App-Assets, seit dem Start
    http_json_send_uint_field (FS("app_full"), (unsigned long) http_app_full);                      // R2 C70: 200 mit Rumpf auf App-Assets
''', '', 1)]),
  ('304 nicht gezaehlt', [(H, '''                if (http_app_not_modified < 0xFFFF)
                {
                    http_app_not_modified++;
                }''', '', 1)]),
  ('Puffer 64 statt 36 Byte', [(H, '#define APP_INM_LEN                                 36', '#define APP_INM_LEN                                 64', 1)]),
]
rc = 0; zahl = 0
for name, eingriffe in S:
    zahl += 1
    T = tempfile.mkdtemp(prefix='c70esab.')
    q2 = os.path.join(T, 'q'); shutil.copytree(Q, q2, ignore=shutil.ignore_patterns('data', 'build*', '*.bin'))
    kaputt = False
    for f, a, n, cnt in eingriffe:
        p = os.path.join(q2, f); b = open(p, 'rb').read()
        rx = re.compile(b'\r?\n'.join(re.escape(x.encode()) for x in a.split('\n')))
        found = len(rx.findall(b))
        if found != cnt:
            print(f"  ANKER FEHLT ({found} statt {cnt}): {name}"); kaputt = True; break
        b = rx.sub(lambda m: n.encode(), b)
        open(p, 'wb').write(b)
    if kaputt:
        rc = 1; shutil.rmtree(T); continue
    r = subprocess.run(['sh', os.path.join(D, 'run.sh'), q2, 'sab'], capture_output=True, text=True, errors='replace')
    fehler = [l.strip() for l in r.stdout.split('\n') if l.strip().startswith('FEHLER')]
    if r.returncode == 1: print(f"  schlaegt an: {name}  ({len(fehler)} Meldungen, z. B. {fehler[0][:110] if fehler else '-'})")
    else:
        last = [l for l in (r.stdout + r.stderr).strip().split('\n') if l.strip()][-3:]
        print(f"  BLIEB {'GRUEN' if r.returncode == 0 else 'KAPUTT'}: {name} (rc={r.returncode}, {' | '.join(last)[:300]})"); rc = 1
    shutil.rmtree(T)
print(f"{zahl} Sabotagen, {'alle schlagen an' if rc == 0 else 'NICHT alle schlagen an'}")
sys.exit(rc)
