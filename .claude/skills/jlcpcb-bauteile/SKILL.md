---
name: jlcpcb-bauteile
description: Regeln für Bauteilwahl und Stückliste bei Fertigung und Bestückung durch JLCPCB — Standard-Katalog (Basic) bevorzugt, jede Zeile mit geprüfter LCSC-Nummer, das Stücklistenformat des V2-Projekts und die Prüfung an der Quelle. Nutzen, sobald ein Bauteil gewählt, ersetzt oder in eine Stückliste eingetragen wird.
when_to_use: Bei jeder Bauteilwahl für die ESP32-S3-Platine, beim Erzeugen oder Prüfen einer Stückliste, vor einer Bestellung und bei der Frage, ob ein Teil bei JLCPCB lieferbar ist.
paths: hardware/**
allowed-tools: Read Grep Glob Bash WebFetch WebSearch
---

# Bauteile für JLCPCB

**Vorgabe des Nutzers vom 17.10.2026:** Die Platine wird bei JLCPCB gefertigt **und bestückt**.
Jedes Bauteil muss dort erhältlich sein, **wenn möglich aus dem Standard-Katalog (Basic)**. Ein
nicht lieferbares Teil ist ein Entwurfsfehler, kein Beschaffungsproblem für später.

## Die Reihenfolge der Wahl

1. **Basic** (Standard-Katalog): erste Wahl.
2. **Preferred Extended**: nur mit genanntem Grund.
3. **Extended**: nur mit genanntem Grund **und** der Aussage, dass es kein Basic-Teil gibt, das die
   Aufgabe erfüllt.

Der Grund steht im Entscheidungsprotokoll (`hardware/architektur/entscheidungen.md`) und im
Komponentenblatt. Am Ende jeder Stückliste steht die **Zahl der Zeilen je Klasse**, damit der Nutzer
sieht, wie viele Nicht-Basic-Nummern er bestellt.

## Was eine Stücklistenzeile belegen muss

| Feld | Beleg |
|---|---|
| LCSC-Nummer | an der Quelle gesehen (`jlcpcb.com/parts`, `lcsc.com`), **nicht aus dem Gedächtnis** |
| Klasse | Basic / Preferred Extended / Extended, so wie die Quelle sie ausweist |
| Lagerbestand | Zahl **und Datum der Prüfung** — Bestände ändern sich |
| Gehäuse | passt zum **Footprint im Schaltplan**, nicht nur zum Symbol |
| Bestückbar | JLCPCB bestückt das Teil per SMT; Handlötteile stehen **getrennt** |

Lässt sich die Quelle nicht abrufen, steht in der Zeile **„nicht geprüft“** mit Grund. Eine Nummer
ohne Prüfung ist eine Annahme (○) und gehört nicht in eine Stückliste, die jemand bestellt.

## Das Format

Die Stückliste von V2 (`production/bom.csv`) hat die Spalten

```
Designator,Footprint,Quantity,Value,LCSC Part #
```

(mit UTF-8-Markierung am Dateianfang, gleiche Werte werden zu einer Zeile mit mehreren Designatoren
zusammengefasst). Dieses Format passt zum Import bei JLCPCB und bleibt für V3 erhalten. Die V2-Datei
ist **nur lesend** das Vorbild; V3 erzeugt eine eigene unter `hardware/esp32s3/`. Die Klasse und das
Prüfdatum kommen als **zusätzliche Spalten** in die Arbeitsfassung, die Bestellfassung für JLCPCB
trägt nur die fünf.

## Wo die Prüfung steht

- Einzelteile: je ein Abschnitt „Fertigung bei JLCPCB“ im **Komponentenblatt**
  (`hardware/komponenten/<bauteil>.md`).
- Der ganze Bestand (Widerstände, Kondensatoren, Stecker, Taster, Netze, Halter):
  `hardware/komponenten/jlcpcb-bestand.md`.
- Prüft nach: der `hw-reviewer`, der die Zeilen der Stückliste mit der Zahl der Bauteile im
  Schaltplan vergleicht. Eine Lücke dort ist ein Befund der Schwere „unbrauchbar“.

## Was V2 schon vorgibt

V2 baut mit 0805-Widerständen und -Kondensatoren. Bleib dabei, wo es geht: Gleiche Gehäuse und Werte
halten die Zahl der verschiedenen Nummern klein, und jede zusätzliche Nummer ist eine weitere
Bestellposition. Neue Werte für den Umbau (47 kΩ für R22, ein 10-µF- und ein 1-µF-Kondensator) sind
**erst gewählt, wenn die Prüfung sie als Basic ausweist**.
