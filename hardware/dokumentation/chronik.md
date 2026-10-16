# Chronik — was wurde wann entschieden, gebaut, gefunden

Stand: **17.10.2026** (Erstfassung). Eintragsregel: Datum, was, **Beleg**. Neue Einträge **unten** anfügen, bestehende nicht umschreiben; ein Irrtum wird durch einen **neuen** Eintrag berichtigt („Berichtigung zum Eintrag vom …“).

**Wie zuverlässig die Zeiten sind.** Die Arbeitsschritte vom Anfang des 17.10.2026 sind erst nachträglich committet worden (Commits des Lead ab 01:08). Die Uhrzeiten der Arbeitsschritte stammen deshalb aus den **Änderungszeiten der Dateien** („zuletzt geändert“) beim Anlegen dieser Chronik; eine spätere Änderung
verschiebt sie. Die Zeiten zeigen die **Reihenfolge**, nicht den genauen Beginn. Was der Nutzer wann gesagt hat, ist nur dort datiert, wo eine Rollenbeschreibung oder Spec das Datum nennt.

## Vor dem 17.10.2026 (Bestand, V2)

| Datum | Ereignis | Beleg |
|---|---|---|
| 23.03.2026 | Letzte Fertigungsausgabe des V2-Projekts (`production/bom.csv`, `positions.csv`, `designators.csv`, `netlist.ipc`; `jlcpcb/production_files/`). **Ohne `R23` und `C10`** | Dateidaten im V2-Projekt (nur gelesen) |
| 12.04.2026 | V2-Schaltplan zuletzt geändert | Dateidatum |
| 30.09.2026 | V2-Layout zuletzt geändert. `HARDWARE.md` erfasst (Commits „Hardware erfasst und gegen die Firmware abgeglichen“, „LED-Board erfasst, LED-Typ korrigiert“) | Dateidatum; `git log -- HARDWARE.md` |
| 10.10.2026 | `HARDWARE.md` zuletzt geändert (Release STM 3.2.24, EEPROM schreibt seitenweise) | `git log -- HARDWARE.md` |

## 17.10.2026

### Vorgaben des Nutzers und Aufbau des Hardware-Teams

| Zeit | Ereignis | Beleg |
|---|---|---|
| (ohne Uhrzeit) | **Der Nutzer legt die Vorgaben 1–13 fest:** 1:1-Umbau auf einen ESP32-S3; Modul WROOM-1, DevKit-kompatibel; ein USB-C; LDR und LEDs auf dem LED-Board; LED-/Power-/LDR-Schnittstelle 100 % kompatibel; analoger LDR; Format 1:1; JLCPCB (Basic zuerst); KiCad; Hardware-Team mit Reviews und Guardrails; eine Quelle für alle Boards; alles dokumentieren; 100 %ige Professionalität | [vorgaben-des-nutzers.md](vorgaben-des-nutzers.md); `.claude/agents/hw-doc-writer.md` |
| (ohne Uhrzeit) | **Der Nutzer legt fest: `R22` bleibt 1 kΩ**, ein früherer Vorschlag von rund 47 kΩ entfällt | Auftrag des Leads an den Dokumentar; `hardware/esp32s3/gen.py` (Kommentar) |
| 00:34 | Verzeichnisse `hardware/architektur/` und `hardware/review/` angelegt (beide **bis heute leer**) | Verzeichnisdatum |
| 00:37 | V2-Netzliste exportiert (`export_v2.py`, `v2/v2.xml`, `v2/nets.json`): 109 Bauteile, 100 Netze | Dateien; `kicad-schema`-Skill |
| 00:38 | ERC-Skript (`erc.sh`), Parser (`sx.py`), PWR_FLAG-Liste (`flags.json`), V2-ERC (`v2/erc_v2.json`) | Dateien |
| 00:40 | Netzlistenvergleich (`vergleich.py`); Skill `jlcpcb-bauteile` | Dateien |
| 00:41 | Rollenbeschreibungen `hw-architect`, `hw-datasheet-analyst`, `hw-designer`, `hw-reviewer`; Skill `kicad-schema` (mit den Korrekturen an `HARDWARE.md`); **JLCPCB-Bestand der V2-Stückliste** (`komponenten/jlcpcb-bestand.md`, Prüfdatum 2026-10-17) | Dateien |
| 00:46 | Rollenbeschreibung `sw-architect` | Datei |

### Komponentenblätter des Datenblatt-Analysten (Zeit = zuletzt geändert)

| Zeit | Blatt | Wichtigster Befund |
|---|---|---|
| 00:47 (zuletzt 01:08) | `esp32s3-wroom-1.md` | Kein Grenzwert je Einzelpin, keine „5-V-tolerant“-Angabe; GPIO19/20 mit 3,2 ms/2 ms High-Glitch; Boot-Modus des TRM; **um 01:08 auf `R22` = 1 kΩ umgestellt** (§6.3), frühere Empfehlungen zurückgezogen |
| 00:49 | `sy8301abc.md` | Spannungsreserve bei knapper 5-V-Schiene kritischer als der Strom; `R29` 31,6 kΩ gegen 32 kΩ |
| 00:50 | `tps2121.md` | `PR1` direkt an der USB-Schiene (6 V absolut, 5,5 V empfohlen) |
| 00:51 | `si7149dp.md` | VGS ≤ −4,5 V halten; Datenblatt von 2008 |
| 00:52 | `mmbf170.md` | **`Q2` ist ein Shikues-Nachbau mit Schwelle max. 2,5 V**, nicht das Diodes-Teil (3,0 V); Basic-Alternative AO3400A |
| 00:55 | `usb-c-16p.md` | V2-Buchse hat **keine Datenkontakte**; HRO C165948 gewählt; Dp/Dn der B-Seite vertauscht |
| 00:56 | `usblc6-2sc6.md` | Leitungen müssen durch den Baustein laufen; CC-Pins ungeschützt |
| 00:57 | `sn74ahct1g125.md` | VIH 2 V gegen VOH 2,64 V; `OE` an GND nicht TI-Empfehlung |
| 00:58 | `ds3231mz.md` | **MZ ≠ DS3231** (MEMS, nicht TCXO); Pull-ups an `32KHZ`/`SQW` kosten rund 0,7 mA |
| 00:59 | `at24c32.md` | HGSEMI-Nachbau; Adresse 0x57; ID-Seite auf 0xBE |
| 01:01 | `ds18b20.md` | V2-Teil ist ein HXY-Nachbau |
| 01:02 | `gl5528.md` | zwei Hersteller-Datenblätter, die abweichen; Hersteller auf dem LED-Board unbekannt |

### Prüfwerkzeuge, Schaltplan, Spec

| Zeit | Ereignis | Beleg |
|---|---|---|
| 00:51–00:54 | Prüfdaten und Prüfung: `hardware-erc.baseline`, `hardware-pinregeln.json`, `hardware-drc.baseline`, `hardware-led-schnittstelle.ausnahmen` (**leer**), `hardware.py` (H1–H8), `hardware-gegenproben.sh` | Dateien |
| 00:55 | Guardrail-Stufen S19 und S20 in `tools/guardrails.sh`; Spec `specs/esp32s3-port/requirements.md` (Entwurf) | Dateien |
| 00:56 | `CLAUDE.md` (R3-Tabelle um die Hardware-Rollen und die Beweispflicht erweitert); `tools/hooks/file-ownership.py`; Rollenbeschreibungen `hw-doc-writer`, `hw-layouter` | Dateien |
| 00:57 | **Schaltplan V3 erzeugt** (`gen.py` → `WordClock-ESP32S3-V3.kicad_sch`, `expected.json`): 20 Bauteile entfallen, 8 neu, `C1` = 1 µF | [aenderungen-gegenueber-v2.md](aenderungen-gegenueber-v2.md) |
| 00:59–01:01 | `specs/esp32s3-port/design.md` und `tasks.md` | Dateien |
| 01:08 | `specs/multiboard-quelle/requirements.md` (Entwurf des `sw-architect`); Lead: Commit „Hardware: Komponentenblatt ESP32-S3-WROOM-1 mit LDR-Lesung bei R22 = 1 kΩ“ | Datei; `git log` |
| 01:14 | Lead: `HARDWARE.md` nach der V2-Netzliste berichtigt; `BEFUNDE.md` L485–L491 und ToDo E42–E48; lebendes Rahmendokument `knowledge/hardware-esp32s3-umbau.md` | Commit „Doku: HARDWARE.md nach der V2-Netzliste berichtigt, Befunde L485-L491, Wissensdokument zum S3-Umbau“ |

### Funde des Dokumentars bei der Erstfassung (Nachprüfung an den Quellen)

| Befund | Beleg |
|---|---|
| `HARDWARE.md` weicht vom V2-Schaltplan ab (IR/DCF77 ohne Stecker, `R20` fehlt in der Beschreibung, Dioden-Einspeisung über `H6`/`H3`, `SQW`/`32K` nur an Pull-ups, `TCXO`) | [wissensstand.md](wissensstand.md) W-2 |
| **`production/bom.csv` von V2 ist unvollständig: `R23` und `C10` fehlen** (92 von 94 Bauteilen) | W-4 |
| **Das LCSC-Feld von `R23` trägt die Nummer des 82-Ω-Widerstands** (C17841) | W-4d |
| **`D1`–`D3`: Layout trägt SOD-123, Schaltplan SOD-123W** | W-5 e2 |
| DRC ohne Projektregeln: 504 `clearance` und 35 `copper_edge_clearance`; mit Projektregeln: 241 Meldungen, 5 Fehler `annular_width` | W-3b (nachgemessen) |
| **`H6`: Sende-/Empfangsleitung der Konsole in V3 möglicherweise gegenüber V2 vertauscht** | OP-35 |
| H2 prüft den Schaltplan gegen eine Soll-Netzliste, die derselbe Generator schreibt | W-20a |
| Die `R22`-Festlegung steht im Widerspruch zu E9(a) der Spec, zu `jlcpcb-bestand.md` und zum Skill `jlcpcb-bauteile` | W-12 |

### Stand der Prüfung am Ende der Erstfassung

`python3 tools/checks/hardware.py`: **keine KRITISCH-Befunde**; drei HOCH-Warnungen (H4 Stückliste fehlt, H5 Haken ohne Seitenangabe, H8 `C32` zusätzlich am LDR-Netz). `./tools/checks/hardware-gegenproben.sh`: alle Proben bestanden.

### Dokumentation

| Zeit | Ereignis | Beleg |
|---|---|---|
| nach 01:08 | **Erstfassung** von `hardware/README.md` und `hardware/dokumentation/` (diese Dateien) durch den Hardware-Dokumentar | diese Datei |
