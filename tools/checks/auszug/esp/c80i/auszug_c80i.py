#!/usr/bin/env python3
"""Auszug fuer den Pruefstand c80i (Paket 2026-10-17-c80, Task 1.3; AKI.8, AKI.13 (a), AKI.14 (a)).
Aufruf: auszug_c80i.py <ESP-Quellverzeichnis> <ziel>. Exit 0 ok, 1 Gegenstand fehlt (FEHL, nicht KAPUTT).

Schneidet aus dem Baum, was c80i.cpp uebersetzt - unveraendert, nur CRLF -> LF:
  heapkarte.h     ganz
  rein.inc        heapkarte.cpp zwischen "C80I REIN ANFANG" und "C80I REIN ENDE" (reine Funktionen)
  geraet.inc      heapkarte.cpp nach "C80I REIN ENDE" bis zum letzten "#endif // ESP_HEAP_MAP" (Sperre, Listen, Ausgabe)
  rein_ohne_ord.inc  rein.inc ohne den Waechter ord - nur um zu zeigen, dass it als Rueckhalt greift (Fall 6 it)
  loop.inc        ESP-uclock.ino: Rumpf von loop ()
  heaplog.inc     ESP-uclock.ino: esp_heap_log () ganz
Fehlt eine Datei oder ein Anker, meldet der Auszug FEHLER und endet mit 1: Ein Stand ohne Instrument (3.2.41) ist ein
Befund, kein kaputter Pruefstand.
"""
import os, re, sys
q, ziel = sys.argv[1], sys.argv[2]
fehler = []

def lies(name):
    p = os.path.join(q, name)
    if not os.path.exists(p):
        fehler.append(f'{name} fehlt')
        return ''
    return open(p, 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n')

def schreib(name, text):
    open(os.path.join(ziel, name), 'w').write(text)

H = lies('heapkarte.h'); C = lies('heapkarte.cpp'); I = lies('ESP-uclock.ino')
if H:
    schreib('heapkarte.h', H)
if C:
    a = C.find('/*== C80I REIN ANFANG'); e = C.find('/*== C80I REIN ENDE')
    if a < 0 or e < 0 or e < a:
        fehler.append('heapkarte.cpp: Marken C80I REIN ANFANG/ENDE fehlen')
    else:
        rein = C[C.index('\n', a) + 1:e]
        schreib('rein.inc', rein)
        ord_anker = re.compile(r'\n        if \(nb <= b\)\n        \{\n            return k->abbruch = HK_AB_ORD;\n        \}\n')
        n = len(ord_anker.findall(rein))
        if n != 1:
            fehler.append(f'heapkarte.cpp: Waechter ord nicht genau einmal in der erwarteten Form ({n})')
        schreib('rein_ohne_ord.inc', ord_anker.sub('\n', rein))
        g0 = C.index('\n', e) + 1
        g1 = C.rfind('#endif // ESP_HEAP_MAP')
        if g1 < g0:
            fehler.append('heapkarte.cpp: "#endif // ESP_HEAP_MAP" am Ende fehlt')
        else:
            schreib('geraet.inc', C[g0:g1])
if I:
    L = I.split('\n')
    k = next((x for x, l in enumerate(L) if l.startswith('loop()')), None)
    if k is None or L[k + 1] != '{':
        fehler.append('ESP-uclock.ino: loop () nicht gefunden')
    else:
        j = next(x for x in range(k + 2, len(L)) if L[x].startswith('}'))
        schreib('loop.inc', '\n'.join(L[k + 2:j]) + '\n')
    k = next((x for x, l in enumerate(L) if l.startswith('esp_heap_log (void)')), None)
    if k is None or not L[k - 1].startswith('static void'):
        fehler.append('ESP-uclock.ino: esp_heap_log () nicht gefunden')
    else:
        j = next(x for x in range(k + 1, len(L)) if L[x].startswith('}'))
        schreib('heaplog.inc', '\n'.join(L[k - 1:j + 1]) + '\n')
for f in fehler:
    print('FEHLER Auszug: ' + f)
sys.exit(1 if fehler else 0)
