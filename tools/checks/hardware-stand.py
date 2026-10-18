#!/usr/bin/env python3
"""Fingerabdruck der Eingaben von Guardrail S19/S20 (Hardware): alles unter hardware/, die Pruefskripte
tools/checks/hardware*, und die V2-Referenz (die *.kicad_*-Dateien im V2-Projekt, H6 meldet deren Aenderung).
Je Datei Pfad, Groesse und Zeitstempel -- kein Inhalt, damit der Aufruf billig bleibt; ein Zeitstempel, der sich
ohne Inhaltsaenderung bewegt (Checkout), loest nur einen Lauf zu viel aus, nie einen zu wenig.
Ausgabe: "<sha256> <zahl der dateien>". Hintergrund: Auf Wunsch des Nutzers (18.10.2026) laufen S19/S20 nur noch,
wenn sich daran etwas geaendert hat -- jeder Lauf startete kicad-cli mehrfach."""
import glob, hashlib, os, sys
ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
V2DIR = os.environ.get("HW_V2", os.path.expanduser("~/Documents/WordClock-USB C - STM32F411 - V2"))
dateien = []
for basis in (os.path.join(ROOT, "hardware"),):
    for d, _, fs in os.walk(basis):
        dateien += [os.path.join(d, f) for f in fs]
dateien += glob.glob(os.path.join(ROOT, "tools", "checks", "hardware*"))
dateien += glob.glob(os.path.join(V2DIR, "*.kicad_*"))
h = hashlib.sha256()
for p in sorted(set(dateien)):
    try:
        st = os.stat(p)
    except OSError:
        continue
    rel = os.path.relpath(p, ROOT) if p.startswith(ROOT) else "V2:" + os.path.basename(p)
    h.update(f"{rel}\0{st.st_size}\0{st.st_mtime_ns}\n".encode())
print(h.hexdigest(), len(dateien))
