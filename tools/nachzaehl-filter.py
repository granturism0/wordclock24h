#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Welche ToDo-Eintraege in BEFUNDE.md lohnen das Nachzaehlen?
#
#     python3 tools/nachzaehl-filter.py
#
# Der Gedanke: Beilaeufiges Miterledigen ist nur dort moeglich, wo sich an den
# genannten Dateien SEIT dem Beleg etwas geaendert hat. Wo nichts angefasst wurde,
# kann der Befund nicht stillschweigend zugegangen sein.
#
# WAS DER ERSTE LAUF ERGEBEN HAT, UND WARUM ES HIER STEHT
#
# Am 05.10.2026 gegen die Arbeitsliste gefahren: 81 Eintraege, davon **55 nachzaehlen,
# 2 nicht, 24 nicht entscheidbar**. Der Filter war als Einsparung gedacht -- er spart
# zwei Eintraege. Grund: In den Tagen davor wurde fast jede Datei angefasst, also ist
# fast ueberall ein beilaeufiges Miterledigen denkbar.
#
# Das ist kein Argument gegen das Werkzeug, sondern eine Aussage ueber den Zeitpunkt.
# Nach einer ruhigen Phase trennt derselbe Filter scharf. Wer ihn nach einem
# Arbeitsstoss laufen laesst und wenig Ausschuss sieht, hat nichts falsch gemacht --
# er misst, dass gerade viel passiert ist.
#
# DIE 24 UNENTSCHEIDBAREN SIND DER EIGENTLICHE BEFUND
#
# 15 Befundzeilen nennen **keine Datei**, 9 ToDo-Eintraege **keine L-Nummer**. Beide
# Gattungen sind damit fuer jede maschinelle Auswertung unsichtbar -- nicht nur fuer
# diese. Das ist dieselbe Lage wie bei der ToDo-Pruefung, deren Muster acht Eintraege
# nie gesehen hat (L235), und bei DIR-013, das zitiert wurde und im Katalog fehlte.
#
# Dieses Skript MELDET die Zahl deshalb bei jedem Lauf (DIR-014): Eine Auswertung, die
# ein Drittel ihres Gegenstands nicht sieht und trotzdem ein Ergebnis nennt, erzeugt
# Vertrauen, das sie nicht deckt.
import io, re, subprocess, datetime, os

s = io.open('BEFUNDE.md', encoding='utf-8').read()
lines = s.split('\n')

# Belegdatum je L-Nummer.
#
# Erster Versuch war das juengste Datum IM TEXT der Befundzeile. Das scheiterte bei
# 52 von 81 Eintraegen -- zwei Drittel der Arbeitsliste waren damit unsichtbar, und
# der Filter haette trotzdem ein Ergebnis gemeldet. Genau die Gattung aus DIR-014:
# eine Pruefung, die ihren Gegenstand nicht vollstaendig sieht und OK sagt.
#
# Jetzt: der Commit, der die Befundzeile angelegt hat. Den kennt git immer, auch wenn
# im Text kein Datum steht. Das Textdatum bleibt als Verfeinerung -- wo es da ist und
# JUENGER als der Anlage-Commit, ist es das genauere Belegdatum (ein nachtraeglich
# ergaenzter Beleg).
beleg = {}
for ln in lines:
    m = re.match(r'\| (L\d+) \|', ln)
    if not m: continue
    nr = m.group(1)
    # KEIN --diff-filter=A: das filtert auf DATEI-Anlagen, und BEFUNDE.md wurde genau
    # einmal angelegt -- der Filter lieferte deshalb fuer 43 von 81 Eintraegen nichts.
    # -S allein liefert jeden Commit, der die Zeichenkette hinzugefuegt oder entfernt
    # hat; der aelteste davon ist die Anlage der Zeile.
    out = subprocess.run(['git','log','--format=%ad','--date=short',
                          '-S','| %s |' % nr, '--', 'BEFUNDE.md'],
                         capture_output=True, text=True).stdout.strip().split('\n')
    d = None
    if out and out[-1]:
        try: d = datetime.date(*map(int, out[-1].split('-')))
        except ValueError: d = None
    tage = re.findall(r'(\d{2})\.(\d{2})\.(\d{4})', ln)
    if tage:
        t = max(datetime.date(int(y), int(mo), int(dd)) for dd, mo, y in tage)
        d = t if (d is None or t > d) else d
    if d: beleg[nr] = d

# Dateien je L-Nummer aus der letzten Spalte
dateien = {}
for ln in lines:
    m = re.match(r'\| (L\d+) \|', ln)
    if not m: continue
    sp = [c.strip() for c in ln.split('|')]
    if len(sp) < 6: continue
    dateien[m.group(1)] = re.findall(r'`([A-Za-z0-9_./-]+\.(?:cpp|css|html|mjs|ino|js|sh|c|h))(?![A-Za-z0-9])', sp[-2])

cache = {}
def juengster(p):
    if p in cache: return cache[p]
    cands = [p] if os.path.exists(p) else []
    if not cands:
        alle = subprocess.run(['git','ls-files'], capture_output=True, text=True).stdout.split()
        cands = [q for q in alle if q.endswith('/'+p) or q == p]
    if not cands:
        cache[p] = None; return None
    out = subprocess.run(['git','log','-1','--format=%ad','--date=short','--',cands[0]],
                         capture_output=True, text=True).stdout.strip()
    cache[p] = datetime.date(*map(int, out.split('-'))) if out else None
    return cache[p]

todo = []
for ln in lines:
    m = re.match(r'\| \*\*([A-F]\d+[a-z]?)\*\* \|', ln)   # gestrichene (~~..~~) fallen raus
    if not m: continue
    refs = sorted(set(re.findall(r'\bL(\d+)\b', ln)))
    todo.append((m.group(1), ['L'+r for r in refs]))

print("Eintraege in der Arbeitsliste (nicht gestrichen): %d\n" % len(todo))
nach, ohne, unklar = [], [], []
for kenn, refs in todo:
    bl = [beleg.get(r) for r in refs if beleg.get(r)]
    fs = sorted({f for r in refs for f in dateien.get(r, [])})
    if not bl or not fs:
        unklar.append((kenn, refs, "kein Belegdatum" if not bl else "keine Datei genannt"))
        continue
    b = max(bl)
    neuer = [(f, juengster(f)) for f in fs]
    neuer = [(f, d) for f, d in neuer if d and d >= b]
    if neuer:
        nach.append((kenn, b, neuer))
    else:
        ohne.append((kenn, b, fs))

print("=== NACHZAEHLEN (%d) -- Datei seit dem Beleg angefasst\n" % len(nach))
for k, b, n in nach:
    print("  %-6s Beleg %s  ->  %s" % (k, b, ", ".join("%s %s" % (f, d) for f, d in n[:3])))
print("\n=== AUSDRUECKLICH NICHT NACHGEZAEHLT (%d) -- keine der genannten Dateien seit dem Beleg geaendert\n" % len(ohne))
for k, b, fs in ohne:
    print("  %-6s Beleg %s  Dateien: %s" % (k, b, ", ".join(fs[:3])))
print("\n=== NICHT ENTSCHEIDBAR (%d)\n" % len(unklar))
for k, r, w in unklar:
    print("  %-6s %s (%s)" % (k, ",".join(r) or "keine L-Referenz", w))
