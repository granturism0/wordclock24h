#!/usr/bin/env python3
"""Pruefung: Bauteile, die aus V2 uebernommen wurden, haben im V3-Schaltplan dieselben Nachbarn wie in V2,
und liegen (soweit uebernommen) auf denselben Koordinaten.

Ein gezeichneter Draht darf nie ein Netz veraendern. vergleich.py (H2) prueft die ganze Netzliste gegen die
Soll-Netzliste; dieses Skript prueft zusaetzlich von V2 aus: Fuer jeden Pin eines uebernommenen Bauteils ist die
Menge der anderen UEBERNOMMENEN Pins im selben Netz in V2 und V3 gleich. Ausgenommen sind Bauteile, deren
Anschluss sich gewollt aendert (die Liste steht unten, jeweils mit Grund), und Bauteile, die es in V3 neu gibt.

Aufruf: python3 nachbarn.py [Schaltplan]      Exit 0 nur ohne Abweichung. Umgebungsvariable NACHBARN_V3 fuer die Gegenprobe.
"""
import os, sys, subprocess, tempfile, xml.etree.ElementTree as ET
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import sch
CLI = "/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli"
V3 = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("NACHBARN_V3", HERE + "/WordClock-ESP32S3-V3.kicad_sch")
V2SCH = os.path.expanduser("~/Documents/WordClock-USB C - STM32F411 - V2/WordClock-USB C - STM32F411 - V2.kicad_sch")
GEWOLLT = {"R12": "EN-Pull-up: in V2 am ESP-12F, in V3 an EN (steht an der Stelle von R2)"}

def nets(path_xml):
    r = ET.parse(path_xml).getroot(); d = {}
    for n in r.iter("net"):
        nm = n.get("name").lstrip("/")
        if nm.startswith("unconnected-"): continue
        d[nm] = {(x.get("ref"), x.get("pin")) for x in n.findall("node")}
    libs = {c.get("ref"): ((c.find("libsource").get("lib") + ":" + c.find("libsource").get("part")) if c.find("libsource") is not None else "") for c in r.iter("comp")}
    return d, libs

def export(sch_path):
    out = tempfile.mktemp(suffix=".xml")
    subprocess.run([CLI, "sch", "export", "netlist", "--format", "kicadxml", "-o", out, sch_path], capture_output=True, check=True)
    return out

n2, l2 = nets(os.path.join(HERE, "v2", "v2.xml"))
x3 = export(V3); n3, l3 = nets(x3); os.remove(x3)
kept = {r for r in l2 if r in l3 and l2[r] == l3[r] and r not in GEWOLLT}
def pinmap(nets_):
    d = {}
    for nm, pins in nets_.items():
        for ref, pin in pins: d[(ref, pin)] = (nm, {p for p in pins if p[0] in kept and p != (ref, pin)})
    return d
p2, p3 = pinmap(n2), pinmap(n3)
errs = []; pins = 0; renamed = []
for (ref, pin), (nm2, nb2) in sorted(p2.items()):
    if ref not in kept: continue
    pins += 1
    if (ref, pin) not in p3: errs.append("%s.%s ist in V3 nicht mehr angeschlossen (V2: Netz %s)" % (ref, pin, nm2)); continue
    nm3, nb3 = p3[(ref, pin)]
    if nb2 != nb3:
        errs.append("%s.%s: Nachbarn V2 %s, V3 %s" % (ref, pin, sorted("%s.%s" % p for p in nb2 - nb3) or "-", sorted("%s.%s" % p for p in nb3 - nb2) or "-"))
    elif nm2 != nm3 and not nm2.startswith("Net-(") and not nm3.startswith("Net-("):
        renamed.append("%s.%s: %s -> %s" % (ref, pin, nm2, nm3))
# Koordinaten der uebernommenen Bauteile
g2 = sch.Geo(sch.parse(open(V2SCH, encoding="utf-8").read())); g3 = sch.Geo(sch.parse(open(V3, encoding="utf-8").read()))
def places(g):
    d = {}
    for s in g.syms:
        p = sch.placement(s)
        if not p["lib"].startswith("power:"): d[p["ref"]] = (p["lib"], p["x"], p["y"], p["rot"], p["mir"])
    return d
c2, c3 = places(g2), places(g3)
same = [r for r in c2 if r in c3 and c2[r] == c3[r]]
moved = [r for r in c2 if r in c3 and c2[r] != c3[r]]
gone = [r for r in c2 if r not in c3]
w2 = {tuple(sorted(w)) for w in g2.wires}; w3 = {tuple(sorted(w)) for w in g3.wires}
l2s = {(n, p) for n, p in g2.labels}; l3s = {(n, p) for n, p in g3.labels}
print("Bauteile aus V2 uebernommen: %d (davon %d auf V2-Koordinaten, %d verschoben: %s; %d entfallen)" % (len(c2) - len(gone), len(same), len(moved), ", ".join(moved) or "-", len(gone)))
print("Draehte: V2 %d, davon in V3 unveraendert %d; V3 neu/geaendert %d; Beschriftungen V2 %d, davon unveraendert (Name und Lage) %d"
      % (len(w2), len(w2 & w3), len(w3 - w2), len(l2s), len(l2s & l3s)))
print("Nachbarpruefung: %d Pins an %d uebernommenen Bauteilen betrachtet (ausgenommen, mit Grund: %s)" % (pins, len(kept), "; ".join("%s: %s" % kv for kv in GEWOLLT.items())))
for r in renamed: print("  Netzname geaendert:", r)
if errs:
    msg = "ABWEICHUNG: %d Pin(s) mit anderen Nachbarn als in V2\n  " % len(errs) + "\n  ".join(errs[:30])
    print(msg); print(msg, file=sys.stderr); sys.exit(1)
if pins == 0: print("ABWEICHUNG: die Pruefung hat nichts betrachtet", file=sys.stderr); sys.exit(1)
print("OK: alle uebernommenen Bauteile haben dieselben Nachbarn wie in V2")
