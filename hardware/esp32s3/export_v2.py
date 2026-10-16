#!/usr/bin/env python3
"""Exportiert die Netzliste des V2-Schaltplans (nur lesend) nach v2/v2.xml und leitet v2/nets.json ab."""
import os, subprocess, json, xml.etree.ElementTree as ET
HERE = os.path.dirname(os.path.abspath(__file__))
V2 = os.path.expanduser("~/Documents/WordClock-USB C - STM32F411 - V2/WordClock-USB C - STM32F411 - V2.kicad_sch")
CLI = "/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli"
os.makedirs(HERE + "/v2", exist_ok=True)
subprocess.run([CLI, "sch", "export", "netlist", "--format", "kicadxml", "-o", HERE + "/v2/v2.xml", V2], check=True)
nets = []
for n in ET.parse(HERE + "/v2/v2.xml").getroot().iter("net"):
    nets.append([n.get("name"), [[x.get("ref"), x.get("pin"), x.get("pintype") and x.get("pinfunction", "")] for x in n.findall("node")]])
nets.sort(key=lambda t: t[0])
json.dump(nets, open(HERE + "/v2/nets.json", "w"))
print("V2: %d Netze, %d Pins" % (len(nets), sum(len(x[1]) for x in nets)))
