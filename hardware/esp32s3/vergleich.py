#!/usr/bin/env python3
"""Vergleicht die Netzliste des erzeugten Schaltplans mit der Soll-Netzliste (expected.json).

Verglichen wird nach PIN-MENGEN, nicht nach Namen: Jedes Soll-Netz muss im Schaltplan als Netz mit
genau derselben Pin-Menge vorkommen. Benannte Netze (alle ausser 'Net-(...)') muessen ausserdem
ihren Namen halten. Zusaetzlich: Bauteilmenge und Werte, und kein ueberzaehliges Netz.
Exit 0 nur, wenn alles gleich ist; sonst 1 mit lesbarer Liste auf stdout UND stderr.
"""
import os, sys, json, subprocess, tempfile, xml.etree.ElementTree as ET
HERE = os.path.dirname(os.path.abspath(__file__))
CLI = "/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli"
SCH = os.environ.get("VERGLEICH_SCH", HERE + "/WordClock-ESP32S3-V3.kicad_sch")  # Umgebungsvariable nur fuer den DIR-014-Nachweis
exp = json.load(open(HERE + "/expected.json"))
xml = tempfile.mktemp(suffix=".xml")
r = subprocess.run([CLI, "sch", "export", "netlist", "--format", "kicadxml", "-o", xml, SCH], capture_output=True, text=True)
if r.returncode != 0 or not os.path.exists(xml):
    print("FEHLER: kicad-cli konnte die Netzliste nicht erzeugen:\n" + r.stderr); sys.exit(2)
root = ET.parse(xml).getroot()
act = {}
for n in root.iter("net"):
    act[n.get("name")] = {"%s.%s" % (x.get("ref"), x.get("pin")) for x in n.findall("node")}
acomp = {c.get("ref"): c.findtext("value") for c in root.iter("comp")}
os.remove(xml)

errors = []
# 1. Netze: ohne Einzelpin-Netze 'unconnected-(...)' (KiCad-Name fuer bewusst offene Pins)
act_real = {k: v for k, v in act.items() if not k.startswith("unconnected-")}
act_nc = {k: v for k, v in act.items() if k.startswith("unconnected-")}
by_pins = {}
for k, v in act_real.items(): by_pins.setdefault(frozenset(v), []).append(k)
exp_nets = exp["nets"]
n_named = n_auto = pins_checked = 0
for name, pins in exp_nets.items():
    ps = frozenset(pins); pins_checked += len(ps)
    auto = name.startswith("Net-(")
    n_auto += auto; n_named += (not auto)
    if ps in by_pins:
        got = by_pins[ps]
        if not auto and name not in got and "/" + name not in got:
            errors.append("Netz '%s': Pins stimmen, aber der Name ist %s" % (name, got))
    else:
        cand = [(k, v) for k, v in act_real.items() if v & ps]
        d = []
        for k, v in cand:
            d.append("im Schaltplan '%s': fehlt %s, zusaetzlich %s" % (k, sorted(ps - v) or "-", sorted(v - ps) or "-"))
        errors.append("Netz '%s' (%d Pins) nicht gefunden: %s" % (name, len(ps), "; ".join(d) or "kein Pin dieses Netzes hat ein Netz"))
exp_sets = {frozenset(v) for v in exp_nets.values()}
for k, v in act_real.items():
    if frozenset(v) not in exp_sets and len(v) > 0:
        errors.append("ueberzaehliges Netz im Schaltplan: '%s' = %s" % (k, sorted(v)))
# Einzelpin-Netze der Soll-Liste duerfen nicht vorkommen, ohne dass sie erwartet sind
# 2. Bauteile
ep = exp["parts"]
for ref in sorted(set(ep) | set(acomp)):
    if ref not in acomp: errors.append("Bauteil %s fehlt im Schaltplan" % ref)
    elif ref not in ep: errors.append("Bauteil %s ist im Schaltplan, aber nicht Soll" % ref)
    elif (acomp[ref] or "") != ep[ref][1]: errors.append("Bauteil %s: Wert '%s' statt '%s'" % (ref, acomp[ref], ep[ref][1]))
# 3. Offene Pins: jeder Pin im Schaltplan, der an keinem Soll-Netz haengt, muss ein No-Connect sein
exp_pins = {p for v in exp_nets.values() for p in v}
nc_pins = {p for v in act_nc.values() for p in v}
open_extra = sorted(nc_pins & exp_pins)
if open_extra: errors.append("Pins sind im Schaltplan offen, obwohl sie ein Netz haben sollen: %s" % open_extra)

pins_act = sum(len(v) for v in act_real.values())
summary = ("verglichen: %d Soll-Netze (%d benannt, %d automatisch benannt), %d Soll-Pins; im Schaltplan %d Netze, %d Pins, "
           "%d bewusst offene Pins (No-Connect); %d Bauteile (Soll %d)"
           % (len(exp_nets), n_named, n_auto, pins_checked, len(act_real), pins_act, len(nc_pins), len(acomp), len(ep)))
print(summary)
if errors:
    msg = "ABWEICHUNG: %d Meldung(en)\n  " % len(errors) + "\n  ".join(errors)
    print(msg); print(msg, file=sys.stderr); sys.exit(1)
if len(exp_nets) != len(act_real) or pins_checked != pins_act:
    print("ABWEICHUNG: Zahlen stimmen nicht (Netze %d/%d, Pins %d/%d)" % (len(act_real), len(exp_nets), pins_act, pins_checked), file=sys.stderr); sys.exit(1)
print("OK: Netzliste des Schaltplans == Soll-Netzliste")
