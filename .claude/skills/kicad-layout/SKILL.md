---
name: kicad-layout
description: Arbeitsweise und belegtes Wissen für das Leiterplatten-Layout der ESP32-S3-Controllerplatine in KiCad — skriptgesteuerter Aufbau aus der V2-Kopie, feste Orte, DRC mit JLCPCB-Regeln, Antennenzone, USB-Paar, Leistungsbahnen, Fertigungsdaten. Verweist auf die Komponentenblätter, wiederholt deren Zahlen nicht. Nutzen, sobald am PCB-Layout gearbeitet wird.
when_to_use: Beim Anlegen oder Ändern von hardware/esp32s3/*.kicad_pcb, *.kicad_dru, layout/ und fertigung/; vor jedem Layout-Review; bei der Frage, was ein Agent am Layout selbst leisten kann.
paths: hardware/esp32s3/**
allowed-tools: Read Grep Glob Bash
---

# Layout in KiCad — Wissen und Arbeitsweise

**Das Schema kommt zuerst.** Ein Layout beginnt erst, wenn der Nutzer den Schaltplan freigegeben hat (Vorgabe 17.10.2026).

**Die Zahlen stehen nicht hier**, sondern in den belegten Blättern unter `hardware/komponenten/` (jede Aussage dort mit ✔/●/○):

| Frage | Blatt |
|---|---|
| Allgemeine Regeln: Masse, Entkopplung, Leistungsbahnen, USB, Schaltregler, Modul, Review-Checkliste, **feste Orte**, Fallen | `layout-regeln.md` |
| JLCPCB: Fähigkeiten, Stackup, Bestückung, THT, Dateien, **DRC-Tabelle für die `.kicad_dru`** (§16) | `jlcpcb-fertigung.md` |
| KiCad-Regeldatei, Board-Setup, Footprints und ihre Gegenprobe, CPL-Rotation, Antennen-Rule-Area | `kicad-regeln-und-footprints.md` |
| Skriptgesteuert arbeiten: `pcbnew`, `kicad-cli`, Router, Exporte, Prüfskripte (Anhang A/B) | `kicad-layout-werkzeug.md` |

Eine Zahl, die nicht in einem dieser Blätter belegt ist, ist eine Annahme (○) und gehört vor dem Setzen zum `hw-datasheet-analyst`.

## Was ein Agent selbst leisten kann (✔ erprobt, KiCad 10.0.2)

- Die V2-Platine als **Kopie** laden, Footprints verschieben, speichern; `pcbnew`-Python liegt unter
  `/Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/3.9/bin/python3`.
- Aus der Netzliste (`kicad-cli sch export netlist --format kicadxml`) das V3-PCB aufbauen (Skript, Anhang A des Werkzeugblatts); es gibt **keinen** skriptbaren «Update PCB from Schematic». Abgleich: `kicad-cli pcb drc --schematic-parity`.
- Bahnen und Vias anlegen, DRC, Gerber/Bohrung/Position, Fabrication Toolkit headless, PNG-Render zum Ansehen.
- **Nicht** lösbar durch den Agenten: das Wegfinden mit Urteilsvermögen (Routing). Freerouting ist nicht installiert (kein Java, nichts heruntergeladen); ob es genutzt wird, entscheidet der Nutzer. Die **GUI-Abnahme bleibt beim Nutzer**: F8 «keine Änderungen», DRC, Zonen füllen, Bahnen und 3D ansehen.

## Die harten Grenzen

1. **100 % feste Orte:** Stecker (`H4`–`H7`, `USB1`), Taster (`SW1`–`SW3`), Befestigungslöcher (`H8`–`H10`) und der Umriss mit Aussparung und USB-Hals bleiben auf V2-Koordinate, -Drehung und -Seite. Die Liste mit Werten steht in `layout-regeln.md` §17; sie wird nach der Freigabe durch den Nutzer gesperrt (`SetLocked`) und von H7 geprüft. Neue Teile (`SW5`) haben keinen V2-Ort und sind als solche gekennzeichnet.
2. **LED-Board-Schnittstelle unverändert** (H8): Footprints `H4`, `H5`, `H7` und alles dazwischen nicht anfassen.
3. **Netznamen:** Der V3-Export nennt Netze mit Schrägstrich (`/ALT_5V_INPUT`); V2-Zonen hängen an den Namen ohne. Vor dem Zonenaufbau Namen angleichen.
4. **Regeldatei:** Die V2-`.kicad_dru` ist eine PCBWay-Vorlage. Für V3 gelten die JLCPCB-Werte aus `jlcpcb-fertigung.md` §16. DRC **immer im Verzeichnis mit `.kicad_pro` und `.kicad_dru`**; sonst 504 falsche Abstandsfehler. Exit-Code taugt nur gegen die Baseline, nicht allein.
5. **Antennenzone:** Der Standard-Footprint des Moduls bringt eine Sperrfläche mit (Abmasse im Blatt). Sie schneidet `H6`, das 1:1 liegen muss. Das ist **offen und braucht eine Entscheidung** (Architekt/Nutzer), kein stilles Verkleinern.
6. **Bestückungsdrehung:** Vor jeder Bestellung jedes gerichtete Teil in der JLCPCB-Vorschau prüfen (`USB1`, Modul, TPS2121, PMEG-Dioden haben keine Korrekturregel).

## Reihenfolge der Arbeit

1. Schema freigegeben? (Nutzer.) Architekt hat die offenen Layout-Entscheide gesetzt (Antennenzone/`H6`, `SW1` gegen Modulpad, Pinwahl-Engpass)?
2. V3-PCB aus V2-Kopie und Netzliste erzeugen; `--schematic-parity` = 0 Probleme.
3. Feste Orte prüfen (H7 erweitert), dann Regeldatei und Board-Setup nach JLCPCB.
4. Platzierung → Zonen → Leistungsbahnen → USB-Paar → übrige Bahnen. Nach jedem Schritt DRC gegen Baseline.
5. Render ansehen, Reviewer-Checkliste (`layout-regeln.md` §15), dann Nutzer.
6. Fertigungsdaten erst nach Reviewer-Gutachten und Freigabe.

## Fallen aus der Erprobung

`board.Remove()` zerstört die Python-Hülle, `RemoveNative()` geht. `Duplicate(False)` und `RemoveUnusedNets(None)` brauchen ein Argument. `FootprintLoad` liefert keine Bibliotheks-FPID (`SetFPID`). Pads gleicher Nummer (`U2` Pad 41, `USB1` Pad SH) alle zuweisen. `SaveBoard` legt `.kicad_pro`/`.kicad_prl` mit an. Speichern ändert die Reihenfolge der Bahnen: nie per Zeilen-Diff vergleichen. In zsh wird eine Variable mit mehreren Wörtern nicht aufgeteilt.
