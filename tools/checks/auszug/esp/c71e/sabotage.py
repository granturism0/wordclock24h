#!/usr/bin/env python3
"""Sabotagen fuer c71e (Paket 2026-10-15, R1b, L7; AKS.1; design.md 8): jede muss FEHL melden (DIR-014), nicht OK und
nicht KAPUTT. Aufruf: sabotage.py <ESP-Quellverzeichnis>. Exit 0, wenn JEDE Sabotage anschlaegt.
Anker zeilenendenneutral (\\r?\\n), jeder muss genau so oft treffen wie angegeben."""
import os, re, shutil, subprocess, sys, tempfile
D = os.path.dirname(os.path.abspath(__file__)); Q = os.path.abspath(sys.argv[1])
S = [
  # (Name, [(Datei, alt, neu, Anzahl), ...]) -- die drei aus AKS.1 / design.md 8 zuerst
  ('nur stop () (ohne Freigabe des Kontexts)', [('httpclient.cpp', '    client = WiFiClient ();                                     // R1b L7a: Kontext samt ungelesener Antwort freigeben\n', '', 1)]),
  ('Fehlerweg 404 mit direktem client.stop ()', [('httpclient.cpp', '        httpclient_stop ();                                                             // R1b L7b: Rumpf der Fehlerseite (etwa 404) nicht liegen lassen\n', '        client.stop ();\n', 1)]),
  ('len == 0 offen (C75)', [('httpclient.cpp', '    if (len <= 0)                                                                       // R1b L7c (C75): 200 ohne Rumpf - nichts zu lesen\n    {\n        httpclient_stop ();\n        return len;\n    }\n\n', '', 1)]),
  # weitere
  ('Fehlerweg keine Antwort mit direktem client.stop ()', [('httpclient.cpp', '        httpclient_stop ();                                                         // auch bei Reset der Gegenstelle: nicht 5 s leer drehen; R1b L7b: freigeben\n', '        client.stop ();\n', 1)]),
  ('DNS-Fehlschlag ohne Freigabe', [('httpclient.cpp', '        httpclient_stop ();                                                         // R1b L7: scheitert schon der DNS, laesst connect () einen alten Kontext stehen\n', '', 1)]),
  ('Freigabe vor stop () (Reihenfolge)', [('httpclient.cpp', '    client.stop ();\n    client = WiFiClient ();', '    client = WiFiClient ();\n    client.stop ();', 1)]),
  ('len == 0 schliesst, gibt aber 0 statt len zurueck (Rueckgabe geaendert)', [('httpclient.cpp', '        httpclient_stop ();\n        return len;\n    }\n\n    return len;', '        httpclient_stop ();\n        return -1;\n    }\n\n    return len;', 1)]),
  ('zu-gross ohne httpclient_stop ()', [('http.cpp', "        httpclient_stop ();\n        update_cache_line (fname, PSTR (\"fehlgeschlagen\"), PSTR (\"zu-gross\"), -1);", "        update_cache_line (fname, PSTR (\"fehlgeschlagen\"), PSTR (\"zu-gross\"), -1);", 1)]),
  ('Existenzpruefung app.js.gz ohne Schliessen', [('http.cpp', '            app_files_available = true;\n            httpclient_stop ();\n        }\n    }\n    else if', '            app_files_available = true;\n        }\n    }\n    else if', 1)]),
  ('neuer Aufrufer, nicht eingeordnet', [('stm32flash.cpp', '    len = httpclient (host, path, filename);\n', '    len = httpclient (host, path, filename);\n    (void) httpclient (host, path, filename);\n', 1)]),
]
rc = 0
for name, eingriffe in S:
    T = tempfile.mkdtemp(prefix='c71esab.')
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
