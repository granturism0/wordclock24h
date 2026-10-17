---
name: kicad-schaltplan-lesbarkeit
description: Wie ein lesbarer Schaltplan in diesem Projekt aussieht — Stilregeln des Nutzers aus seinem V2-Plan, Konventionen mit Quellen, prüfbare Schwellen, und was beim skriptgesteuerten Zeichnen verlässlich geht. Nutzen, sobald ein Schaltplan gezeichnet, erzeugt, geändert oder begutachtet wird.
when_to_use: Beim Zeichnen oder Generieren von hardware/esp32s3/*.kicad_sch, bei jeder Änderung an gen.py, bei einem Schaltplan-Review und bei der Frage, ob ein Plan 'lesbar' ist.
paths: hardware/esp32s3/**
allowed-tools: Read Grep Glob Bash
---

# Ein Schaltplan ist für Menschen da

**Vorgabe des Nutzers vom 17.10.2026:** Ein Schaltplan, der nur aus Netz-Referenzen (Drahtstummel mit Bezeichner an jedem Pin) besteht, ist in keiner Weise lesbar. Der erste V3-Entwurf war genau so gebaut und wurde verworfen. **Vorbild ist sein V2-Plan** (`~/Documents/WordClock-USB C - STM32F411 - V2/…V2.kicad_sch`, nur lesend).

Alle Messwerte, Quellen und die Prüftabelle stehen belegt in `hardware/komponenten/schaltplan-gestaltung.md`. Die Zahlen werden hier nicht kopiert; lies das Blatt.

## Die Stilregeln (Kurzfassung; Begründung und Belege im Blatt)

1. Ein Block pro Funktion, gestrichelter Rahmen, Titel oben links, **flach auf einem Blatt** wie V2.
2. **Innerhalb eines Blocks gezeichnete Drähte, keine Labels.** Labels nur an Blockgrenzen und für lange Wege.
3. Signalfluss links → rechts, Versorgung oben, Masse unten; VCC/GND-Symbole in Grundorientierung direkt am Bauteil.
4. Entkoppelkondensator neben dem Pin, den er bedient.
5. **Sichtbare Netznamen sprechend; nie `Net-(…)` als Text im Plan.**
6. Nur orthogonale Drähte, Raster 1,27 mm, Junctions an Abzweigen.
7. Nicht benutzte Pins mit Kreuz; Jumper, Bestückungsvarianten, Steckerbelegung und DNP als Text im Plan.
8. Pin-Seite nach Funktion: Eingang links, Ausgang rechts (Leistungswandler: Eingang links, Ausgang rechts).

## Was beim Erzeugen per Skript verlässlich geht (✔ untersucht, Teil D des Blatts)

Ein Werkzeug, das aus Netzliste und Blockzuordnung einen Plan in V2-Qualität zeichnet, gibt es nach der Recherche nicht (SKiDL liefert unlesbare Bilder). Verlässlich, in dieser Reihenfolge:
1. **Unveränderte V2-Blöcke aus der V2-Datei übernehmen** (Bauteile, Drähte, Labels, Texte samt Koordinaten) — Kopieren, nicht Zeichnen. Das Ergebnis sieht aus wie der Plan des Nutzers.
2. **Geänderte Blöcke** (ESP32-S3-Umfeld, USB-C, UART, Reset, Teile der Versorgung) nach den Regeln oben platzieren und verdrahten; ein Mensch oder der Reviewer prüft Lage und Lesbarkeit.
3. **Die Prüfung H9 ist die Abnahme**, nicht das Augenmass des Erzeugers. Ein Plan, der H9 nicht besteht, geht nicht zum Nutzer.

Egal welcher Weg: die Netzliste bleibt Wahrheit (H2, Vergleich nach Pin-Mengen). Gezeichnete Drähte dürfen sie nie verändern; ein falsch gezogener Draht ist ein Kurzschluss, den der Vergleich finden muss.

## Offen (Entscheid Nutzer/Architekt)

- Ob V2-Niveau die Messlatte der Schwellen in H9 ist (alle Schwellen ○).
- PWR_FLAG als Netzmarke wie in V2 oder echte benannte Versorgungssymbole.
