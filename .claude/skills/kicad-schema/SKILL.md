---
name: kicad-schema
description: Arbeitsweise und belegtes Wissen für Schaltplan und Layout in KiCad bei diesem Projekt — kicad-cli, Netzlistenexport, ERC, Vergleich nach Pin-Mengen, die V2-Netzliste als Wahrheit über den Bestand und die Fallen, in die der Generator schon gelaufen ist. Nutzen bei jeder Arbeit an hardware/**, beim Lesen des V2-Projekts und bei jeder Aussage darüber, was an einem Pin hängt.
when_to_use: Wenn ein KiCad-Schaltplan erzeugt, geändert, geprüft oder verglichen wird, wenn jemand wissen will, was im V2-Schaltplan an einem Pin oder Netz hängt, und bevor eine Aussage über die Hardware aus HARDWARE.md übernommen wird.
paths: hardware/**
allowed-tools: Read Grep Glob Bash
---

# KiCad bei der WordClock

Alles hier ist an diesem Projekt ausprobiert, nicht aus der Dokumentation abgeschrieben.

## Werkzeug

- **KiCad 10.0.2**, `kicad-cli` liegt **nicht im PATH**:
  `/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli`. Die Zeile
  `Fontconfig error: Cannot load default config file` auf stderr ist harmlos; filter sie
  (`2>&1 | grep -v Fontconfig`), damit eine echte Meldung nicht darin untergeht.
- Das V2-Projekt (`~/Documents/WordClock-USB C - STM32F411 - V2`, Schaltplan im Format von KiCad 9,
  `generator_version "9.0"`) lädt in KiCad 10 ohne Umbau. Es ist die **Referenz und bleibt nur lesend**.
- Der Schaltplan ist **eine Seite**, keine Hierarchie. Netzbezeichner statt langer Leitungen.

## Die drei Befehle, die tragen

```
kicad-cli sch export netlist --format kicadxml -o v2.xml <datei>.kicad_sch
kicad-cli sch erc --severity-all --format json -o erc.json <datei>.kicad_sch
kicad-cli sch erc --exit-code-violations ...        (Exit-Code ungleich 0 bei Verletzungen)
```

Der XML-Export liefert `components/comp` (Referenz, Wert, Footprint, `libsource` mit Bibliothek und
Teilname, `fields`) und `nets/net` mit `node` (Referenz, **Pinnummer**, `pinfunction`). Nicht
angeschlossene Pins stehen als Netz `unconnected-(<Ref>-<Pinname>-Pad<N>)`. **Automatisch
vergebene Netznamen (`Net-(U6-E0)`) hängen am Pinnamen und ändern sich mit dem Symbol** — verlass
Dich nie auf sie.

## Die V2-Netzliste ist die Wahrheit über den Bestand, nicht `HARDWARE.md`

`HARDWARE.md` entstand aus einer Abschrift und enthält Aussagen, die der Schaltplan nicht trägt.
Am 17.10.2026 beim Export nachgeprüft:

- **IR-Empfänger (`TSOP`) und DCF77 haben auf dem Controller keinen Stecker**, nur 100-kΩ-Pull-ups
  (R8, R9) und den Controller-Pin.
- Der **Gate-Pull-down von Q2 existiert** (R20, 100 kΩ).
- `SQW_DS3231` und `32K_DS3231` liegen nur an den Pull-ups RN2 und am DS3231, **nicht am Controller**.
- `SW4` ist ein Umschalter, der die Konsole `H6` entweder an die STM-UART oder an die zweite
  UART des ESP legt; `H6` und `H3` speisen das 3,3-V-Netz über Dioden (D3, D1) **ein**.
- Die Zahl der Bauteile ist 109, die der Netze 100.

Lies darum vor jeder Aussage über einen Pin das Netz aus dem Export. Den Export erzeugst Du mit
`hardware/esp32s3/export_v2.py` oder dem Befehl oben; er ist reproduzierbar.

## Das Leiterplattenformat bleibt 1:1 (Vorgabe vom 17.10.2026)

Der Nutzer verlangt, dass das **Format der Platine gleich bleibt**: Umriss, Befestigung, Lage der
Stecker. Die Zahlen stammen aus `V2.kicad_pcb` (Edge.Cuts und Footprint-Positionen, per Skript
ausgelesen; Koordinaten relativ zur linken oberen Ecke der Platine):

| Merkmal | Wert in V2 |
|---|---|
| Umriss | **145,00 × 39,00 mm**, zwei Kupferlagen |
| Befestigung | `H8` (3,25 / 28,00), `H9` (72,50 / 3,25), `H10` (141,75 / 28,00); M2,5-Löcher |
| Stromeingang extern | `H1` (35,75 / 27,75) und `H2` (30,25 / 27,75), 2,2-mm-Pads, `H1` am Netz `ALT_5V_INPUT` |
| USB | `USB1` an der **Unterkante**, Mitte (72,50 / 35,38) |
| LED-Stecker | `H7` (38,63 / 4,75) gedreht 180°, `H4` (12,00 / 4,29), LDR-Stecker `H5` (26,00 / 3,25), beide −90° |
| Konsole | `H6` (103,75 / 4,50) |
| Funkmodul | `U4` (ESP-12F) bei (127,00 / 12,25), **Antenne an der Oberkante** |
| Taster | `SW3` (86,00 / 27,75), `SW2` (99,00 / 27,75), `SW1` (112,00 / 27,75) |

Daraus folgt für das Layout: **Das V2-PCB ist die Ausgangsdatei.** Der Designer arbeitet an einer
**Kopie** unter `hardware/esp32s3/` (nie am Original in `~/Documents`), behält Umriss, Löcher und
die Positionen der Stecker und tauscht nur den Innenraum: Der S3-Modul-Footprint sitzt dort, wo
`U4` sitzt, mit der Antenne an der Oberkante; der freie Platz des STM32 bleibt unbelegt oder nimmt
die neue Beschaltung auf. **Der Stecker `USB1` wechselt von 6 auf 16 Pole:** Seine Bauhöhe, Breite
und Lage der Öffnung müssen zum Gehäuse passen. Ob die beiden Stecker dieselben Aussenmasse haben,
ist **nicht belegt** und gehört in das Komponentenblatt, bevor das Layout beginnt.

## Wie ein Schaltplan hier geprüft wird (DIR-014)

Ein Schaltplan, der richtig aussieht, ist keine Aussage. Verlangt sind **drei Belege**:

1. **ERC** (`hardware/esp32s3/erc.sh`): jede Meldung beheben oder mit Begründung als bewusst führen.
   Nicht wegdrücken.
2. **Netzlistenvergleich nach Pin-Mengen**: Netzliste des neuen Schaltplans exportieren und gegen die
   Soll-Netzliste (`expected.json`) vergleichen — als **Mengen von `Ref.Pin`**, nicht nach Namen.
   Benannte Netze (alle ohne `Net-(`) müssen zusätzlich den Namen halten. Die Prüfung gibt aus,
   **wie viele Netze und Pins sie betrachtet hat**, und die Zahl wird mit dem verglichen, was es
   geben müsste. Eine Prüfung mit zu engem Muster meldet OK und sieht den Gegenstand gar nicht.
3. **Provozierter Fehler**: ein Pin am falschen Netz, die Prüfung muss anschlagen, mit Exit-Code
   ungleich 0 und lesbarer Ausgabe. Erst dann gilt sie.

## Fallen aus der Arbeit

- **Stock-Symbole mit `extends`** (zum Beispiel `USBLC6-2SC6`, das auf `USBLC6-2P6` aufbaut) lassen
  sich nicht einfach in den Schaltplan einbetten: Der Basisblock muss mit dem Namen der Ableitung
  eingebettet und die Eigenschaften überschrieben werden.
- **Gestapelte Pins** (USB-C 16P: `A4`, `A9`, `B4`, `B9` sitzen auf derselben Koordinate; GND beim
  ESP32-S3-Modul: `1`, `40`, `41`) verbinden sich dort von selbst. Ein Drahtstummel genügt, doppelte
  sind Rauschen.
- **Stromversorgungs-Symbole**: V2 nutzt `power:VCC` mit dem **Wert** `VCC_3.3V` beziehungsweise
  `VCC_5V` und `power:GND`; der Netzname ist der Wert. Für die ERC-Meldung „Leistungspin nicht
  getrieben" braucht es `PWR_FLAG` an der richtigen Stelle (V2 hat sieben).
- **Bash auf macOS (zsh):** ein ungequoteter Ausdruck wie `--include=*.c` bricht mit „no matches
  found" ab. Muster quoten.
- Dateien im KiCad-Format haben **LF**-Zeilenenden und sind UTF-8; prüf trotzdem beim Patchen,
  statt es anzunehmen (CLAUDE.md, DIR-015).

## Wo was liegt

| Was | Wo |
|---|---|
| Generator, Parser, Soll-Netzliste, ERC- und Vergleichsskripte, Schaltplan | `hardware/esp32s3/` (Schreiber: `hw-designer`) |
| Komponentenblätter mit Datenblattbelegen | `hardware/komponenten/` (Schreiber: `hw-datasheet-analyst`) |
| Entscheidungen, Pinbelegung, Leistungsbudget | `hardware/architektur/` (Schreiber: `hw-architect`) |
| Gutachten | `hardware/review/` (Schreiber: `hw-reviewer`) |
| Fertigungsdaten V2 (Vorbild für das Ausgabeformat) | `~/Documents/WordClock-USB C - STM32F411 - V2/production/` |

Die Fertigungsausgabe von V2 stammt aus dem KiCad-Plugin **Fabrication Toolkit**
(`fabrication-toolkit-options.json` liegt im Projekt) und enthält `bom.csv`, `positions.csv`,
`designators.csv`, `netlist.ipc` und das ZIP für JLCPCB. Für die Stückliste und die Bauteilwahl gilt
der Skill `jlcpcb-bauteile`.
