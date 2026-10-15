#!/usr/bin/env python3
"""Auszug fuer den Pruefstand c72i (Paket 2026-10-15, R1.1, I-5). Aufruf: auszug_c72.py <ESP-Quellverzeichnis> <ziel> <c65-Verzeichnis>

Ruft zuerst den Auszug von c65 (device_ready.inc, server_loop.inc, i1/i3/i4.inc, zaehler.inc, heaplog.inc, ...), dann
schneidet er aus http.cpp die Klasse WcHttpServer (server_class.inc) und den I-5-Block von "#define TCP_NOW_LIST_MAX"
bis zum Ende von http_no_request_timeouts_get () (i5.inc). Fehlt der I-5-Block (Stand 3.2.37), bleibt i5.inc leer und
C72_HAVE_I5 fehlt in c72have.inc: Der Pruefstand uebersetzt mit Ersatz und meldet FEHL, nicht KAPUTT.
"""
import os, subprocess, sys
q, ziel, c65 = sys.argv[1], sys.argv[2], sys.argv[3]
r = subprocess.run([sys.executable, os.path.join(c65, 'auszug.py'), q, ziel], capture_output=True, text=True)
if r.returncode:
    sys.exit('c65-Auszug: ' + (r.stdout + r.stderr).strip())
H = open(os.path.join(q, 'http.cpp'), 'rb').read().decode('utf-8').replace('\r\n', '\n')
L = H.split('\n')
def von_bis(start, ende_kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(start)), None)
    if i is None: return ''
    j = next((k for k in range(i, len(L)) if L[k].startswith(ende_kopf)), None)
    if j is None: return ''
    while not L[j].startswith('}'): j += 1
    return '\n'.join(L[i:j + 1]) + '\n'
i = next((k for k, l in enumerate(L) if l.startswith('class WcHttpServer')), None)
j = next((k for k in range(i, len(L)) if L[k].startswith('};')), None) if i is not None else None
cls = '\n'.join(L[i:j + 1]) + '\n' if j is not None else ''
if not cls: sys.exit('AUSZUG LEER: class WcHttpServer')
open(os.path.join(ziel, 'server_class.inc'), 'w').write(cls)
i5 = von_bis('#define TCP_NOW_LIST_MAX', 'http_no_request_timeouts_get (void)')
open(os.path.join(ziel, 'i5.inc'), 'w').write(i5)
open(os.path.join(ziel, 'c72have.inc'), 'w').write('#define C72_HAVE_I5 1\n' if i5.strip() else '')
