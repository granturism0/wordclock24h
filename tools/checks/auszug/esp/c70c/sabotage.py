#!/usr/bin/env python3
"""Sabotagen fuer c70c (Paket 2026-10-15, R2b.1, C70 Variante (c); AKL.1, AKL.2; design.md 18.6, 18.8): jede muss FEHL
melden (DIR-014), nicht OK und nicht KAPUTT. Aufruf: sabotage.py <ESP-Quellverzeichnis>. Exit 0, wenn JEDE anschlaegt.
Anker zeilenendenneutral (\\r?\\n), jeder muss genau so oft treffen wie angegeben. Je Sabotage wird gemeldet, ob der
dynamische Teil, der statische oder beide anschlagen - eine Sabotage, die nur statisch auffiele, waere ein Hinweis auf
eine Luecke im dynamischen Teil."""
import os, re, shutil, subprocess, sys, tempfile
D = os.path.dirname(os.path.abspath(__file__)); Q = os.path.abspath(sys.argv[1])
H = 'http.cpp'
ZUW = '                    immutable = (app_asset == APP_ASSET_VERSIONED && http_app_token_hits (token, etag));'
TERN_304 = '''                http_send (etag);
                http_send (immutable ? FS("\\r\\nCache-Control: public, max-age=31536000, immutable\\r\\nConnection: close\\r\\n\\r\\n")
                                     : FS("\\r\\nCache-Control: no-cache\\r\\nConnection: close\\r\\n\\r\\n"));

                if (http_app_not_modified < 0xFFFF)'''
TERN_200 = '''                }

                http_send (immutable ? FS("\\r\\nCache-Control: public, max-age=31536000, immutable\\r\\nConnection: close\\r\\n\\r\\n")
                                     : FS("\\r\\nCache-Control: no-cache\\r\\nConnection: close\\r\\n\\r\\n"));
            }'''
NOC = 'http_send (FS("\\r\\nCache-Control: no-cache\\r\\nConnection: close\\r\\n\\r\\n"));'
ETAG_200 = '''                if (etag[0])
                {
                    http_send (FS("\\r\\nETag: "));
                    http_send (etag);
                }
'''
HEBEN = '        if (app_asset && strcmp (asset_path, "app/index.html") && strcmp (asset_path, "app/sw.js"))'
IMM_INC = '''                        if (http_app_immutable < 0xFFFF)
                        {
                            http_app_immutable++;
                        }
'''
MISS_INC = '''                    else if (http_app_token_miss < 0xFFFF)
                    {
                        http_app_token_miss++;
                    }
'''
VERGL = "    return n > 0 && strlen (etag) == n + 2 && etag[0] == '\"' && etag[n + 1] == '\"' && ! memcmp (etag + 1, token, n);"
S = [
  # die sieben aus dem Auftrag zuerst
  ('immutable ohne h (auch bei fehlendem oder leerem h)', [
      (H, '                if (*token)\n                {\n' + ZUW, '                if (1)\n                {\n' + ZUW.replace('http_app_token_hits (token, etag)', '(! *token || http_app_token_hits (token, etag))'), 1)]),
  ('immutable bei falschem h (ohne Markenvergleich)', [(H, ZUW, ZUW.replace(' && http_app_token_hits (token, etag)', ''), 1)]),
  ('immutable fuer das Dokument unter /app/index.html', [(H, HEBEN, HEBEN.replace('strcmp (asset_path, "app/index.html") && ', ''), 1)]),
  ('immutable fuer das Dokument unter /app und /app/', [(H, '        app_asset    = (http_find_app_install_asset ("app/index.html") != 0);',
                                                           '        app_asset    = (http_find_app_install_asset ("app/index.html") != 0) ? APP_ASSET_VERSIONED : 0;', 1)]),
  ('immutable fuer sw.js', [(H, HEBEN, HEBEN.replace(' && strcmp (asset_path, "app/sw.js")', ''), 1)]),
  ('nur bei 200, nicht bei 304 (304 bei Treffer mit no-cache)', [(H, TERN_304, '                http_send (etag);\n                ' + NOC + '\n\n                if (http_app_not_modified < 0xFFFF)', 1)]),
  ('h in der Antwort ausgegeben (eigener Kopf)', [(H, ETAG_200, ETAG_200 + '                http_send (FS("\\r\\nX-Marke: "));\n                http_send (http_get_param ("h"));\n', 1)]),
  ('Zaehler fehlen in device_ready', [(H, '''    http_json_send_uint_field (FS("app_immutable"), (unsigned long) http_app_immutable);            // R2b C70 (c): 200 und 304 mit immutable
    http_json_send_uint_field (FS("app_token_miss"), (unsigned long) http_app_token_miss);          // R2b C70 (c): Marke h ohne immutable
''', '', 1)]),
  # weitere aus AKL.1/AKL.2
  ('nur bei 304, nicht bei 200', [(H, TERN_200, '                }\n\n                ' + NOC + '\n            }', 1)]),
  ('h nur bei Treffer gespiegelt', [(H, ETAG_200, ETAG_200 + '                if (immutable)\n                {\n                    http_send (FS("\\r\\nX-Marke: "));\n                    http_send (http_get_param ("h"));\n                }\n', 1)]),
  ('Vergleich nur ueber die Laenge', [(H, VERGL, VERGL.replace(" && ! memcmp (etag + 1, token, n)", ''), 1)]),
  ('Vergleich als Praefix', [(H, VERGL, "    return n > 0 && strlen (etag) > n + 1 && etag[0] == '\"' && ! memcmp (etag + 1, token, n);", 1)]),
  ('Vergleich mit den Anfuehrungszeichen (h = ganzes ETag)', [(H, VERGL, "    return n > 0 && strlen (etag) == n && ! memcmp (etag, token, n);", 1)]),
  ('leere Marke trifft (n > 0 weg, ohne ETag-Pruefung)', [(H, VERGL, "    return strlen (etag) == n + 2 || n == 0;", 1)]),
  ('immutable im Zweig HTTP/1.0', [
      (H, '            if (cache)\n            {\n                const char *    token', '            if (1)\n            {\n                const char *    token', 1),
      (H, ZUW, ZUW.replace('(app_asset == APP_ASSET_VERSIONED && http_app_token_hits (token, etag))', '(cache ? (app_asset == APP_ASSET_VERSIONED && http_app_token_hits (token, etag)) : 1)'), 1)]),
  ('Weissliste beim Heben ignoriert', [(H, HEBEN, HEBEN.replace('if (app_asset && ', 'if ('), 1)]),
  ('app_immutable nie erhoeht', [(H, IMM_INC, '', 1)]),
  ('app_token_miss fehlt in einem Zweig (nie erhoeht)', [(H, MISS_INC, '', 1)]),
  ('Dokument und sw.js mit h nicht als Fehltreffer gezaehlt', [(H, '                    else if (http_app_token_miss < 0xFFFF)', '                    else if (app_asset == APP_ASSET_VERSIONED && http_app_token_miss < 0xFFFF)', 1)]),
  ('leeres h zaehlt als Fehltreffer', [(H, '                if (*token)\n                {\n' + ZUW, '                if (1)\n                {\n' + ZUW, 1)]),
  ('app_immutable nicht saettigend', [(H, '                        if (http_app_immutable < 0xFFFF)', '                        if (1)', 1)]),
  ('app_token_miss nicht saettigend', [(H, '                    else if (http_app_token_miss < 0xFFFF)', '                    else if (1)', 1)]),
  ('Zaehler in device_ready vertauscht', [(H, '(FS("app_immutable"), (unsigned long) http_app_immutable)', '(FS("app_immutable"), (unsigned long) http_app_token_miss)', 1)]),
  ('max-age 1 Tag statt 1 Jahr (Ent-23)', [(H, 'public, max-age=31536000, immutable\\r\\nConnection: close\\r\\n\\r\\n")\n                                     : FS("\\r\\nCache-Control: no-cache\\r\\nConnection: close\\r\\n\\r\\n"));\n            }',
                                          'public, max-age=86400, immutable\\r\\nConnection: close\\r\\n\\r\\n")\n                                     : FS("\\r\\nCache-Control: no-cache\\r\\nConnection: close\\r\\n\\r\\n"));\n            }', 1)]),
  ('h erreicht http_app () nicht (Parameter in http () nicht gesetzt)', [(H, '    if (param[0])\n    {\n        http_set_params (param);\n    }', '    if (0)\n    {\n        http_set_params (param);\n    }', 1)]),
  ('Logzeile je Anfrage mit Marke (UART)', [(H, '                if (*token)\n                {\n' + ZUW, '                if (*token)\n                {\n                    Serial.println (token);\n' + ZUW, 1)]),
]
rc = 0; zahl = 0
for name, eingriffe in S:
    zahl += 1
    T = tempfile.mkdtemp(prefix='c70csab.')
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
    out = r.stdout
    fehler = [l.strip() for l in out.split('\n') if l.strip().startswith('FEHLER')]
    dyn = 'FEHL  c70c sab' in out; sta = 'FEHL  c70c statisch sab' in out
    wo = 'dynamisch und statisch' if dyn and sta else 'nur dynamisch' if dyn else 'NUR STATISCH' if sta else '-'
    if r.returncode == 1: print(f"  schlaegt an ({wo}): {name}  ({len(fehler)} Meldungen, z. B. {fehler[0][:110] if fehler else '-'})")
    else:
        last = [l for l in (r.stdout + r.stderr).strip().split('\n') if l.strip()][-3:]
        print(f"  BLIEB {'GRUEN' if r.returncode == 0 else 'KAPUTT'}: {name} (rc={r.returncode}, {' | '.join(last)[:300]})"); rc = 1
    shutil.rmtree(T)
print(f"{zahl} Sabotagen, {'alle schlagen an' if rc == 0 else 'NICHT alle schlagen an'}")
sys.exit(rc)
