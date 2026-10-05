#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Umschrift in einem lebenden Dokument auf echte Umlaute bringen.

    python3 tools/umlaute-doku.py BEFUNDE.md [--probe]

CLAUDE.md verlangt echte Umlaute in der Dokumentation; die Umschrift gilt nur fuer
die Quelldateien unter src/** und ESP8266/ESP-uclock/, weil ein Werkzeug, das ein
"ä" in eine ISO-8859-1-Datei schreibt und dabei UTF-8 annimmt, die Datei beschaedigt
(DIR-015). Wer in derselben Sitzung an beiden Welten arbeitet, traegt die Gewohnheit
hinueber -- genau so war BEFUNDE.md ueber Monate gemischt.

ZWEI DINGE, DIE DIESES SKRIPT ANDERS MACHT ALS EINE WORTLISTE

1. CODEZITATE BLEIBEN UNBERUEHRT. In `backticks` steht Quelltext, und dort ist die
   Umschrift RICHTIG. Wer sie ersetzt, beschaedigt ein Zitat. Der erste Entwurf
   dieses Skripts pflegte eine Wortliste von 72 Eintraegen und erwischte damit
   1334 von rund 3000 Vorkommen -- eine Liste mit 768 Kandidaten pflegt niemand.

2. DREI REGELN STATT EINER LISTE:
     - nach 'q' nie ersetzen          Quelle, Request, Sequenz, Queue
     - nach einem Vokal nie           neue, dauerhaft, bauen, feuert, Steuerung
     - knappe Ausnahmeliste           zuerst = zu+erst, die Silbengrenze sieht man nicht

   Dabei ein Fehler, der nur durch das Nachzaehlen des Restes auffiel: `vor in "aeiou"`
   ist fuer den LEEREN String True, also blieb jedes Wort mit ue/ae/oe am Wortanfang
   unberuehrt (ueberschreibt, Uebersicht). Die Regel sah richtig aus.

Mit --probe wird nichts geschrieben, nur gezaehlt.
"""
import io, re, sys, collections

AUSNAHMEN = {"zuerst", "zueinander", "zuerkannt", "zuerteilt", "duell", "duelle"}

# Silbengrenze zwischen u und e, die man am Vorgaengerbuchstaben NICHT sieht:
# ak-tu-ell, even-tu-ell, ma-nu-ell, Kon-gru-enz. Die erste Fassung dieses Skripts
# machte daraus "aktuell" -> "aktüll", an zwoelf Stellen in BEFUNDE.md, gleich die
# erste in Zeile 5 (L297). Gemeldet hat es der doc-writer, nicht die Gegenprobe --
# die zaehlte nur Zeilen und Codespannen, nicht ob ein Wort danach noch eines ist.
SILBENGRENZE = ("uell", "uenz", "uent", "poet", "poes", "koexist", "koeffiz")
UMLAUT = {"ae": "ä", "oe": "ö", "ue": "ü", "Ae": "Ä", "Oe": "Ö", "Ue": "Ü"}


def ersetze(text):
    def einzeln(m):
        wort = m.group(0)
        if wort.lower() in AUSNAHMEN:
            return wort
        if any(g in wort.lower() for g in SILBENGRENZE):
            return wort

        def paar(mm):
            vor = mm.string[mm.start() - 1] if mm.start() > 0 else ""
            if vor and vor.lower() == "q":
                return mm.group(0)
            if vor and vor.lower() in "aeiou":
                return mm.group(0)
            return UMLAUT[mm.group(0)]

        return re.sub(r"ae|oe|ue|Ae|Oe|Ue", paar, wort)

    return re.sub(r"\b[A-Za-zÄÖÜäöüß]+\b", einzeln, text)


def main():
    if len(sys.argv) < 2:
        print(__doc__.strip().splitlines()[2]); return 2
    pfad = sys.argv[1]
    probe = "--probe" in sys.argv
    s = io.open(pfad, encoding="utf-8").read()

    spannen = []
    g = re.sub(r"```.*?```|`[^`\n]*`",
               lambda m: (spannen.append(m.group(0)), "\x00%d\x00" % (len(spannen) - 1))[1],
               s, flags=re.S)
    neu = ersetze(g)
    n = sum(1 for a, b in zip(g.split(), neu.split()) if a != b)
    ergebnis = re.sub(r"\x00(\d+)\x00", lambda m: spannen[int(m.group(1))], neu)

    # Gegenprobe VOR dem Schreiben: Zeilenzahl und Codespannen muessen gleich bleiben.
    if ergebnis.count("\n") != s.count("\n"):
        print("  ABBRUCH: Zeilenzahl hat sich geaendert."); return 1
    if len(re.findall(r"```.*?```|`[^`\n]*`", ergebnis, re.S)) != len(spannen):
        print("  ABBRUCH: Zahl der Codespannen hat sich geaendert."); return 1

    if not probe:
        io.open(pfad, "w", encoding="utf-8").write(ergebnis)
    rest = collections.Counter(re.findall(r"\b[A-Za-zÄÖÜäöüß]*(?:ae|oe|ue)[A-Za-zÄÖÜäöüß]*\b",
                                          re.sub(r"```.*?```|`[^`\n]*`", "", ergebnis, flags=re.S)))
    print("  %s: %d Woerter %s, %d Codespannen unberuehrt" %
          (pfad, n, "zu aendern" if probe else "geaendert", len(spannen)))
    print("  Rest (erwartet: Quelle, neue, Request, zuerst …): %d Vorkommen, %d verschiedene"
          % (sum(rest.values()), len(rest)))
    return 0


sys.exit(main())
