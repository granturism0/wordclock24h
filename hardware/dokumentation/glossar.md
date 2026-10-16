# Glossar — Begriffe und Kürzel der Hardware-Arbeit

Stand: **17.10.2026**. Alphabetisch innerhalb der Gruppen. Ein Begriff, der in den Dokumenten mehrere Bedeutungen hat, ist ausdrücklich so geführt.

## Projektbegriffe

| Begriff | Bedeutung |
|---|---|
| **V2** | Der heutige Controller-Print „WordClock USB-C / STM32F411 V2“ (Eigenentwicklung). Referenz, **nur lesend**: `~/Documents/WordClock-USB C - STM32F411 - V2` |
| **V3** | Arbeitsname des Umbaus auf den ESP32-S3 (`hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch`; im Schaltplan als „Rev 4.0-Entwurf“ geführt) |
| **Controller-Platine** | Die Platine mit STM32 (V2) bzw. ESP32-S3 (V3), 145,00 × 39,00 mm. **Nur sie wird umgebaut** |
| **LED-Board** | Die separate Platine mit den LEDs und dem LDR; bleibt **unverändert**, hängt über den gespiegelten 7-poligen Stecker `H7` (und `H4`, `H5`) an der Controller-Platine |
| **Nachbau** | Zwei Bedeutungen: (1) der **Nachbau der Platine** mit dem S3-Modul (E1, Spec); (2) ein **Nachbau-Bauteil**, das ein Original nachahmt (z. B. DS18B20 von HXY, AT24C32M von HGSEMI, MMBF170 von Shikues). Der Kontext sagt, welches |
| **DevKit, DevKitC-1** | Espressifs Entwicklungsplatine ESP32-S3-DevKitC-1 (v1.1). Die Software muss darauf laufen (Vorgabe 2), deshalb nur Pins, die es am Header herausführt |
| **Brücke, Kommandobrücke** | Die serielle Verbindung ESP ↔ STM in V2 (115200 Baud); beim S3 eine Warteschlange zwischen zwei Tasks. Software, nicht Hardware |
| **Schnittstelle zum LED-Board** | Der ganze Pfad zwischen Controller-Pin und Stecker `H7`: `U3`, `R11`, `R13`, `R17`, `R31`, `JP2`, `Q1`, `Q2`, `R19`–`R23`, `C10`, `C6`, `H4`, `H5`, `H7`, `R22`; wird von **H8** geprüft |
| **Beweispflicht** | Jede Aussage über ein Bauteil trägt ein Beleg-Zeichen; ein Wert aus dem Gedächtnis ist keiner (`CLAUDE.md`, R3) |
| **Beleg-Zeichen** | ✔ gelesen (mit Seite), ● Sekundärquelle, ○ Annahme. In der Dokumentation zusätzlich **✔R** (vom Dokumentar am Repo nachgeprüft) und **✔K** (aus einem Komponentenblatt übernommen); siehe [wissensstand.md](wissensstand.md) |
| **Chat-Drain** | Wissen, das nur im Gesprächsverlauf steht und mit ihm verschwindet; Anlass dieses Verzeichnisses (Vorgabe 12) |
| **Gegenprobe** | Eine Prüfung gegen eine **hergestellte Verletzung** in einer Kopie; eine Prüfung ist erst fertig, wenn sie einmal angeschlagen hat (DIR-014) |
| **Baseline** | Obergrenze bekannter Meldungen (`hardware-erc.baseline`, `hardware-drc.baseline`); eine Prüfung schlägt an, wenn eine Meldungsart die Obergrenze überschreitet |
| **Ausnahmendatei** | `tools/checks/hardware-led-schnittstelle.ausnahmen`: Abweichungen von der LED-Schnittstelle von V2, **mit Grund und Freigabe des Nutzers** |
| **DIR-nnn** | Nummer einer Direktive im Katalog `knowledge/directives.md` (z. B. DIR-001 Sprache, DIR-006 lebende Dokumente, DIR-014 Prüfung muss einmal scheitern, DIR-015 Kodierung). Nur zitieren, was dort steht |
| **R1–R6, R3b** | Koordinationsregeln in `CLAUDE.md` (R1 nur der Lead baut, R3 ein Schreiber je Datei, R3b Besitz ist nicht Reihenfolge, R5 Hardware ist exklusiv) |

## Kürzel mit mehrfacher Bedeutung

| Kürzel | Bedeutungen |
|---|---|
| **H1 … H10** | (1) **Bauteilnummern** auf der Platine: `H1`, `H2` externer Stromeingang; `H3` SWD-Stecker (V2); `H4` LED-Stecker 3-polig; `H5` LDR-Stecker 2-polig; `H6` Konsole 4-polig; `H7` LED-Stecker 7-polig; `H8`, `H9`, `H10` Befestigungslöcher. (2) **Prüfungen H1–H8** in `tools/checks/hardware.py`. (3) **Anforderungen H1–H12** in `specs/esp32s3-port/requirements.md` („Was die Software von der Hardware voraussetzt“). Immer dazusagen, welche gemeint sind |
| **Rev, Revision** | Stand eines Datenblatts (Seitenangaben gelten für die dort genannte Revision) oder des Schaltplans |
| **A63, A64, A65, L…, C…, G…** | Befundkennungen aus `BEFUNDE.md` (z. B. 1-Wire-Abtastzeitpunkt, 85 °C, Auflösung des DS18B20) |

## Bauteil- und Fertigungsbegriffe

| Begriff | Bedeutung |
|---|---|
| **JLCPCB** | Der Fertiger und Bestücker der Platine (Vorgabe 8) |
| **LCSC** | Zugehöriger Bauteilhändler; die **LCSC-Nummer** (`C` + Ziffern) identifiziert ein Teil bei JLCPCB |
| **Basic / Preferred Extended / Extended** | Klassen der JLCPCB-Teilebibliothek: **Basic** = Standard-Katalog, keine Rüstgebühr, **erste Wahl**; **Preferred Extended** = bevorzugte Erweiterung; **Extended** = Erweiterung, je Nummer eine Rüstgebühr (nicht geprüft). Ein Teil ausserhalb von Basic braucht einen genannten Grund |
| **PCBA, Standard / Economic** | Bestückte Leiterplatte; „Standard Only“ heisst, ein Teil ist nur im Standard-Verfahren bestückbar (so das Funkmodul) |
| **SMT / THT** | Oberflächenmontage (SMD) / Durchsteckmontage. Ob JLCPCB ein THT-Teil bestückt, liefert die Quelle nicht (OP-21) |
| **MSL** | Feuchtigkeits-Empfindlichkeitsstufe; MSL 3 beim Modul: Lagerfenster und Backen beachten |
| **Stückliste, BOM** | `bom.csv`, Format von V2: `Designator,Footprint,Quantity,Value,LCSC Part #`; gleiche Werte → eine Zeile |
| **Fabrication Toolkit** | KiCad-Plugin, das `bom.csv`, `positions.csv`, `designators.csv`, `netlist.ipc` und das ZIP für JLCPCB erzeugt |
| **Footprint** | Lötflächenmuster eines Bauteils im Layout; muss zum **Gehäuse des gewählten Teils** passen, nicht nur zum Symbol |
| **DNP** | „Do not populate“: im Schaltplan vorhanden, aber nicht bestückt (`R7`, `R17`) |
| **`in_bom`** | Schaltplan-Merkmal „kommt in die Stückliste“ (`yes`/`no`); Prüfung H4 zählt Bauteile mit `in_bom = yes` und `dnp = no` |
| **TP, JP** | Testpunkt; Lötbrücke (`JP2` überbrückt `Q1`, `JP3`) |
| **Nachbau-Teil** | siehe oben |

## Elektronikbegriffe

| Begriff | Bedeutung |
|---|---|
| **EN** | „Enable“-/Reset-Pin des ESP32-S3-Moduls; nie schweben lassen; 10 kΩ gegen 3,3 V und 1 µF gegen GND |
| **Strapping** | GPIOs, deren Pegel beim Reset den Boot-Modus oder die Flash-Spannung festlegen: **GPIO0, 3, 45, 46** |
| **Download-Modus** | Zustand, in dem der Chip Firmware annimmt: automatisch über DTR/RTS der USB-Verbindung oder von Hand (GPIO0 = Low, EN kurz Low) |
| **USB-Serial/JTAG** | Der im ESP32-S3 eingebaute USB-Teil (GPIO19 D−, GPIO20 D+) für Konsole, Flashen und JTAG |
| **DTR/RTS** | Handshake-Leitungen des CDC-Geräts, mit denen `esptool` Reset und Download-Modus auslöst |
| **JTAG** | Debug-Schnittstelle; beim S3 über die USB-Buchse (OpenOCD) |
| **ADC1 / ADC2** | Die beiden AD-Wandler des S3: ADC1 = GPIO1–10, ADC2 = GPIO11–20; ADC2 kollidiert mit WLAN, deshalb nur **ADC1** |
| **Dämpfung (ATTEN0–3)** | Eingangsteiler des ADC: 0 / 2,5 / 6 / 12 dB, nutzbare Bereiche 0 … 850 / 1100 / 1600 / 2900 mV |
| **ratiometrisch** | Der Messwert ist ein Verhältnis zur Versorgung (F411: ADC-Referenz = VDDA); der S3 misst **absolut** gegen eine interne Referenz |
| **LDR** | Lichtabhängiger Widerstand (GL5528) auf dem LED-Board; oberer Zweig des Teilers, `R22` ist der untere |
| **SK6812, SKC6812, WS2812** | Adressierbare LED-Typen. Verbaut ist laut `HARDWARE.md` (Z. 227, 239 ff.) die **SKC6812RGBW-BW** von OPSCO (114 Stück), **nicht** die gewöhnliche SK6812; sie verlangt eine Reset-Pause von mehr als 200 µs. Ein Datenblatt der SKC-Variante liegt nicht im Repo (OP-27) |
| **Pegelwandler** | `U3` (SN74AHCT1G125): hebt das 3,3-V-Datensignal auf 5 V; TTL-Eingang (VIH 2 V) |
| **RMT** | Hardware-Block des ESP32, der die LED-Datenleitung zeitgenau ausgibt (Software-Thema) |
| **CC1/CC2, Rd, Rp** | Konfigurationsleitungen der USB-C-Buchse; **Rd** (5,1 kΩ je CC nach GND) meldet „Senke“, **Rp** der Quelle meldet das Stromangebot |
| **VBUS, SBU** | USB-Versorgungsleitung; Sideband-Leitungen (offen lassen) |
| **PD** | USB Power Delivery (Spannungen über 5 V); mit dem ESD-Baustein `U11` nicht verträglich |
| **ESD, TVS** | Schutz gegen Entladungen bzw. Überspannung; `U11` USBLC6-2SC6, `D4`/`D5` SMAJ5.0CA |
| **PTC** | Rückstellende Sicherung (`F1` 1,5 A, `F2` 3 A) |
| **Buck** | Abwärtswandler (`U10` SY8301 erzeugt aus 5 V die 3,3 V) |
| **Brownout** | Einbruch der Versorgung unter die Betriebsgrenze (Risiko bei der 3,3-V-Schiene, OP-14) |
| **Power-Mux** | `U9` TPS2121: wählt zwischen USB und externer 5-V-Einspeisung, USB hat Vorrang |
| **I²C, 1-Wire, UART0** | Bus von RTC und EEPROM; Bus des DS18B20; Konsole des Moduls (GPIO43/44) |
| **Glitch** | Kurzer unerwünschter Pegel eines Pins beim Einschalten (GPIO1–18: 60 µs Low; GPIO19/20: High-Glitches von 3,2 ms bzw. 2 ms) |
| **EPAD** | Masse-Fläche unter dem Modul |

## KiCad-Begriffe

| Begriff | Bedeutung |
|---|---|
| **`kicad-cli`** | Kommandozeile von KiCad (`/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli`, nicht im PATH) |
| **ERC** | Electrical Rules Check: Regelprüfung des Schaltplans |
| **DRC** | Design Rules Check: Regelprüfung des Layouts; liest `.kicad_pro` und `.kicad_dru` neben der Platine |
| **Netzliste** | Export der Verbindungen (`--format kicadxml`); Grundlage jedes Vergleichs |
| **Pin-Menge** | Menge der `Ref.Pin` eines Netzes; Vergleiche gehen nach Pin-Mengen, nicht nach automatisch vergebenen Namen (`Net-(U6-E0)`) |
| **No-Connect** | Zeichen für einen bewusst offenen Pin; ohne es meldet der ERC den Pin |
| **`PWR_FLAG`** | Symbol, das einem Netz „hier speist eine Quelle“ meldet (gegen „Leistungspin nicht getrieben“) |
| **Edge.Cuts** | Lage des Platinenumrisses |
| **`.kicad_sch`, `.kicad_pcb`, `.kicad_pro`, `.kicad_dru`** | Schaltplan, Layout, Projekt, Regeln |
| **Generator** | `hardware/esp32s3/gen.py`: erzeugt aus der V2-Netzliste den V3-Schaltplan und die Soll-Netzliste |
| **Soll-Netzliste** | `hardware/esp32s3/expected.json`; wird vom Generator **im selben Lauf** geschrieben (Grenze von H2, W-20a) |

## Quellenkürzel in den Komponentenblättern

| Kürzel | Unterlage |
|---|---|
| **DS-M** | ESP32-S3-WROOM-1/-1U Datenblatt (Modul) |
| **DS-C** | ESP32-S3 Series Datenblatt (Chip) |
| **HDG** | ESP32-S3 Hardware Design Guidelines |
| **TRM** | ESP32-S3 Technical Reference Manual |
| **DK, DK-Sch** | DevKitC-1 v1.1 User Guide, Schaltplan |
| **IDF** | ESP-IDF Programming Guide (Sekundärquelle ●) |
| **SPEC** | USB Type-C Cable and Connector Specification |
