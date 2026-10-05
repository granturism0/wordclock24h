#!/usr/bin/env python3
# Wortprobe fuer tools/umlaute-doku.py -- beide Richtungen.
#
#     python3 tools/umlaute-doku.probe.py
#
# Die erste Gegenprobe des Werkzeugs zaehlte nur Zeilen und Codespannen. Dass
# "aktuell" danach "aktüll" hiess, sah sie nicht (L297). Diese Probe prueft, ob ein
# Wort danach noch das richtige Wort ist -- und zwar auch die, die NICHT umgeschrieben
# werden duerfen. Ein Werkzeug, das nur in eine Richtung geprueft ist, ist halb.
import sys
src = open("tools/umlaute-doku.py", encoding="utf-8").read().replace("sys.exit(main())", "")
ns = {}; exec(compile(src, "umlaute-doku.py", "exec"), ns)
FAELLE = {
    # muss bleiben
    "aktuell": "aktuell", "eventuell": "eventuell", "manuell": "manuell", "virtuell": "virtuell",
    "individuell": "individuell", "Kongruenz": "Kongruenz", "Duell": "Duell", "Poet": "Poet",
    "Koeffizient": "Koeffizient", "Quelle": "Quelle", "Request": "Request", "neue": "neue",
    "zuerst": "zuerst", "Steuerung": "Steuerung", "dauerhaft": "dauerhaft",
    # muss umgeschrieben werden
    "fuer": "für", "Geraet": "Gerät", "ueberschreibt": "überschreibt", "laeuft": "läuft",
    "Loesung": "Lösung", "Bruecke": "Brücke", "Groesse": "Grösse", "Uebersicht": "Übersicht",
}
schlecht = [(a, ns["ersetze"](a), b) for a, b in FAELLE.items() if ns["ersetze"](a) != b]
for a, i, b in schlecht:
    print("  FEHLT %s -> %s, erwartet %s" % (a, i, b))
print("  %d von %d Woertern richtig" % (len(FAELLE) - len(schlecht), len(FAELLE)))
sys.exit(1 if schlecht else 0)
