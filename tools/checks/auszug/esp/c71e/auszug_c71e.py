#!/usr/bin/env python3
"""Auszug fuer den Pruefstand c71e (Paket 2026-10-15, R1b, L7). Aufruf: auszug_c71e.py <ESP-Quellverzeichnis> <ziel>
httpclient.cpp ganz, ohne #include-Zeilen (hc.inc). Die Attrappen stellt core_c71e.h."""
import os, sys
q, ziel = sys.argv[1], sys.argv[2]
s = open(os.path.join(q, 'httpclient.cpp'), 'rb').read().decode('utf-8').replace('\r\n', '\n')
s = '\n'.join(l for l in s.split('\n') if not l.startswith('#include'))
if 'httpclient (const char * host' not in s or 'httpclient_stop (void)' not in s:
    sys.exit('AUSZUG LEER: httpclient () oder httpclient_stop () fehlt')
open(os.path.join(ziel, 'hc.inc'), 'w').write(s)
