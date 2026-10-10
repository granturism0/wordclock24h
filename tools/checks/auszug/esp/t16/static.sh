#!/bin/sh
# Statische Pruefung C47 / AKX.2 (Paket 2026-10-10). Aufruf: static.sh <verzeichnis mit http.cpp> <bezeichnung>
#
# Jede Ausgabe ueber http_send () / http_send_attr () / http_send_xml_escaped () mit NICHT-literalem
# Argument in den Legacy-Seitenfunktionen (http_style .. http_update, dazu http_api_remote_esp_update)
# muss in bestand.tsv stehen - als Mehrfachmenge (Funktion, Ausgabe, Argument). Ein Eintrag zu viel im
# Code heisst: neue, nicht eingeordnete Wertausgabe => FEHL. Ein Eintrag zu viel in der Liste heisst: die
# Liste ist veraltet => FEHL (eine Erlaubnisliste, die niemand nachfuehrt, deckt bald das Falsche).
# Dazu: "maskiert" muss http_send_attr sein, http_send darf nur Zahl/Konstante/gefiltert tragen, und
# http_send_xml_escaped kommt in den Legacy-Seiten nicht mehr vor (ersetzt ungueltiges UTF-8 durch '?').
D=$(cd "$(dirname "$0")" && pwd); Q=$1; B=$2
python3 - "$Q/http.cpp" "$D/bestand.tsv" "$B" <<'PY'
import re, sys
from collections import Counter
src, liste, bez = sys.argv[1:]
L = open(src, 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
heads = [(i, l.split(' (')[0].split('(')[0]) for i, l in enumerate(L) if re.match(r'^[a-z_][a-z_0-9]* ?\(', l)]
names = [n for _, n in heads]
if 'http_style' not in names or 'http_update' not in names:
    print(f'KAPUTT ({bez}): Anker http_style/http_update fehlen'); sys.exit(2)
a = next(i for i, n in heads if n == 'http_style')
e = next(i for i, n in heads if n == 'http_update')
legacy = set(n for i, n in heads if a <= i <= e) | {'http_api_remote_esp_update'}
ist = Counter(); n_alle = 0; n_lit = 0
for k, (i, n) in enumerate(heads):
    if n not in legacy:
        continue
    j = heads[k + 1][0] if k + 1 < len(heads) else len(L)
    for m in range(i, j):
        s = L[m].strip()
        if s.startswith(('//', '*', '/*')):
            continue
        for fn, arg in re.findall(r'\b(http_send(?:_attr|_xml_escaped)?) *\((.*)\);', L[m]):
            n_alle += 1
            arg = arg.strip()
            if re.match(r'^(FS\()?"', arg):
                n_lit += 1
                continue
            ist[(n, fn, arg)] += 1
soll = Counter(); gruppe = {}
for z in open(liste, encoding='utf-8'):
    z = z.rstrip('\n')
    if not z or z.startswith('#') or z.startswith('funktion\t'):
        continue
    f, aus, arg, g, grund = z.split('\t')
    soll[(f, aus, arg)] += 1; gruppe.setdefault((f, aus, arg), set()).add(g)
bad = 0
for key, c in sorted(ist.items()):
    if c > soll.get(key, 0):
        bad += 1; print(f'  FEHL nicht in der Bestandsliste: {key[0]}: {key[1]} ({key[2]}) x{c - soll.get(key, 0)}')
for key, c in sorted(soll.items()):
    if c > ist.get(key, 0):
        bad += 1; print(f'  FEHL Liste veraltet, im Code nicht (mehr): {key[0]}: {key[1]} ({key[2]})')
for key, gs in gruppe.items():
    for g in gs:
        if (g == 'maskiert') != (key[1] == 'http_send_attr') or key[1] == 'http_send_xml_escaped':
            bad += 1; print(f'  FEHL Gruppe {g} passt nicht zu {key[1]} in {key[0]} ({key[2]})')
for key in ist:
    if key[1] == 'http_send_xml_escaped':
        bad += 1; print(f'  FEHL http_send_xml_escaped in Legacy-Funktion {key[0]} - ersetzt ungueltiges UTF-8 durch ?')
nl = sum(ist.values())
print(f'{"FEHLGESCHLAGEN" if bad else "OK"} ({bez}): {len(legacy)} Legacy-Funktionen, {n_alle} Ausgabeaufrufe, davon {n_lit} literal, '
      f'{nl} nicht-literal geprueft gegen {sum(soll.values())} Listeneintraege, {bad} Befunde')
sys.exit(1 if bad else 0)
PY
