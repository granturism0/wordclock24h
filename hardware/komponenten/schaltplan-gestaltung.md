# Schaltplan-Gestaltung: Was einen lesbaren Plan ausmacht, belegt am V2-Plan

Stand: 17.10.2026. Anlass: Der V3-Entwurf (`hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch`, erzeugt von `gen.py`) wurde verworfen. Er besteht fast nur aus Drahtstummeln mit Netzbezeichnern. Der V2-Plan des Nutzers ist das Vorbild.

**Beleg-Zeichen:** ✔ selbst gemessen oder gelesen (Fundstelle dabei), ● Sekundärquelle, ○ Annahme oder abgeleitete Schwelle (nicht als Tatsache verwenden).

**Messwerkzeug (Teil A):** Python-Parser über die S-Ausdrücke der `.kicad_sch`, dazu `kicad-cli` 10.0.2. Die Skripte liegen im Scratchpad der Sitzung (`…/scratchpad/sg/m2.py`, `m3.py`, `m4.py`, `m5.py`), nicht im Repo. Die Quelle ist eine Kopie von `~/Documents/WordClock-USB C - STM32F411 - V2/WordClock-USB C - STM32F411 - V2.kicad_sch` (gespeichert mit KiCad 9.0, Format 20250114, Titelblock Rev 3.2, 12.04.2026). Das V2-Projekt wurde nur gelesen.

Definition der Pin-Klassen (Teil A): Ein Bauteilpin zählt als **Label-Stummel**, wenn sein Draht-Netz ein Label trägt und sonst keinen weiteren Bauteilpin und kein Versorgungssymbol. Er zählt als **Labelnetz**, wenn das Netz ein Label und weitere Pins trägt. Er zählt als **gezeichnet**, wenn das Netz ohne Label zu mindestens einem weiteren Pin oder Versorgungssymbol führt. Netze werden geometrisch aus Drahtenden und T-Stellen gebildet. Nicht erfasst: Pins, die sich ohne Draht berühren.

---

## Die zehn wichtigsten Regeln (Kurzfassung)

Die Begründungen stehen in Teil B, die Prüfbarkeit in Teil C.

1. **Ein Block pro Funktion, mit Rahmen und Titel.** V2: 17 gestrichelte Rahmen mit Titel oben links, flach auf einem A3-Blatt (✔ A). Quellen: PCBWay, Hardware Codex, tmpdir/Greenberg (✔/●, B).
2. **Innerhalb eines Blocks zeichnest Du Drähte, keine Labels.** V2: 0 lokale Labels, alle Passiven hängen per Draht am IC (✔ A). Die Quelle tmpdir (●) sagt dasselbe. PCBWay und Flux sagen „nutze Labels“ (Widerspruch, siehe B3).
3. **Labels nur an Blockgrenzen und für lange Wege.** V2: 64 von 67 Labels tragen einen Namen, der in mindestens zwei Blöcken vorkommt (✔ A).
4. **Signalfluss links → rechts, Versorgung oben, Masse unten.** V2: alle 68 VCC-/GND-Symbole stehen in Grundorientierung, also VCC nach oben und GND nach unten (✔ A). Quellen: Wikipedia, Hardware Codex, KLC S4.2 (✔/●, B).
5. **Eingänge eines ICs links, Ausgänge rechts, plus oben, Masse unten.** KLC S4.2 (●). Die Pinseite bestimmt das Symbol, nicht der Plan. Ausnahme der KLC: Leistungswandler haben Eingang links und Ausgang rechts.
6. **Versorgungssymbol direkt am Bauteil, nie als freier Stummel.** V2: 74 von 75 Versorgungssymbolen hängen an einem Draht, ein Symbol bedient mehrere Bauteile (✔ A). Die Netzsprache steht im Symbol („VCC_3.3V“).
7. **Entkoppelkondensator neben den Pin, den er bedient.** Hardware Codex ✔. PCBWay nennt als Alternative einen eigenen Block, wenn die Zuordnung notiert ist (Widerspruch, B4).
8. **Netznamen sprechend, nie `Net-(…)` als sichtbarer Text.** V2: 0 sichtbare `Net-(`-Labels (✔ A). V3: 62 Label-Instanzen mit `Net-(…)` (✔ A). Eine automatisch benannte Netz ohne Label ist dagegen unkritisch: V2 hat 30 davon unter 100 Netzen, niemand sieht sie, weil sie als Draht gezeichnet sind (✔ A).
9. **Nur orthogonale Drähte auf dem 1,27-mm-Raster.** KiCad-Handbuch ✔: 50 mil ist das empfohlene Raster, andere Raster zerstören die Verbindungen. V2: 100 % der Drahtenden auf 1,27 mm, 0 diagonale Drähte (✔ A).
10. **Jede nicht benutzte Pin-Funktion trägt ein Nicht-verbunden-Kreuz; Jumper, Bestückungsvarianten und Steckerbelegung stehen als Text im Plan.** V2: 32 Kreuze, 2 Hinweistexte, 2 Teile mit „nicht bestückt“-Kreuz (R7, R17) (✔ A). PCBWay verlangt Notizen im Plan ✔.

---

## Teil A: Messung am V2-Schaltplan

### A1 Gesamtzahlen (✔ Messung an der V2-Datei)

| Grösse | V2 | V3-Entwurf (zum Vergleich, selbe Skripte) |
|---|---|---|
| Blattgrösse | A3 (420 × 297 mm), ein Blatt, keine Unterblätter (`sheet` = 0) | A2 (594 × 420 mm) |
| Bauteile (ohne Versorgungssymbole) | 109 | 104 |
| Versorgungssymbole (VCC/GND/PWR_FLAG) | 75 (GND 41, VCC 27, PWR_FLAG 7) | 124 (GND 68, VCC 51, PWR_FLAG 5) |
| Drähte | **449**, Summe 2109,5 mm | **281**, Summe 749,3 mm |
| Drahtlänge Median / Mittel / Max | 2,54 / 4,70 / 36,83 mm | 2,54 / 2,67 / 25,40 mm |
| Junctions | 105 | 0 |
| Lokale Labels | **0** | 162 |
| Globale Labels | **67** (31 verschiedene Namen) | 0 |
| Nicht-verbunden-Kreuze | 32 | 32 |
| Textfelder | 19 (17 Blocktitel, 2 Hinweise) | 49 |
| Blockrahmen | 17 (`rule_area`, gestrichelt) | 18 (`rectangle`) |
| Diagonale Drähte | 0 | 0 |
| Drahtenden auf 2,54 / 1,27 / 0,635 mm | 254 von 898 / 898 / 898 | 276 von 562 / 562 / 562 |
| Drahtkreuzungen ohne Junction | 9 | 0 |
| Nutzfläche der Zeichnung | 315 × 243 mm, rund 61 % der A3-Fläche | nicht gemessen |

Folgerung (✔ Messung, ○ Deutung): V2 hat dreimal so viel Draht (Summe) wie V3 bei gleich vielen Bauteilen. Bei V3 besteht jedes Netz aus höchstens einem Segment, also aus Stummeln. Bei V2 laufen die Drähte zwischen den Bauteilen.

### A2 Pins: gezeichnet gegenüber Label (✔ Messung)

| Klasse | V2 | V3 |
|---|---|---|
| Bauteilpins insgesamt | 359 | 328 |
| davon mit Nicht-verbunden-Kreuz | 32 | 32 |
| davon mit Draht | 326 | 296 |
| Draht zu anderem Pin oder Versorgungssymbol | 235 (72 % der Drahtpins) | 123 (42 %) |
| Label-Stummel (ein Pin, Label, sonst nichts) | **51 (16 %)** | **156 (53 %)** |
| Labelnetz mit weiteren Pins | 40 (12 %) | 17 (6 %) |
| Länge eines Label-Stummels, Median / Max | 2,54 / 6,35 mm | 2,54 / 2,54 mm |

Die Zeile „Draht zu anderem Pin“ schliesst die Verbindung eines Pins zu einem Versorgungssymbol ein. Sie trennt Signal- und Versorgungsdrähte nicht.

**Wo V2 Labels benutzt:** 64 von 67 Labels sitzen in Namen, die in mindestens zwei Blöcken vorkommen. 28 von 31 Namen verbinden mehrere Blöcke (✔ Messung `m4.py`). Die drei Ausnahmen sind `32K_DS3231`, `SQW_DS3231` und `GPIO2_ESP`: je ein Label an einem Ausgang, der nirgends weitergeführt wird (dokumentierende Labels). Zwischen Bauteilen **im** Block steht in V2 kein Label.

**Der STM32 ist die Drehscheibe:** 25 der 49 STM32-Pins tragen einen Label-Stummel, 11 sind per Draht verbunden (Versorgung, Entkopplung, Oszillator-Nähe), 13 tragen ein Kreuz. Der ESP-12F: 22 Pins, davon 6 Stummel, 6 per Draht, 10 mit Kreuz. Die kleinen ICs (DS3231, TPS2121, SY8301, EEPROM) hängen fast nur an Drähten: U9 11 von 12, U10 6 von 6, U5 7 von 8, U6 6 von 8 (✔ Messung).

### A3 Messung je Block (✔)

Drähte zählen, wenn beide Enden im Rahmen liegen. Pins = Bauteilpins mit Draht, Stummel = Label-Stummel. Die Summe der Bauteile beträgt 106 von 109, drei liegen ausserhalb der Rahmen (Befestigungslöcher H8–H10).

| Block | Bauteile | Drähte | Junctions | Labels | Versorgungssymbole | Pins | Stummel |
|---|---|---|---|---|---|---|---|
| PowerSupply | 44 | 158 | 47 | 0 (!) | 23 | 104 | 0 |
| Reset | 3 | 12 | 3 | 1 | 2 | 7 | 0 |
| Led STM | 2 | 4 | 0 | 1 | 1 | 4 | 1 |
| ESP12F | 6 | 27 | 5 | 10 | 5 | 22 | 7 |
| LED Power | 8 | 28 | 8 | 2 | 2 | 24 | 1 |
| LED Controll | 6 | 23 | 4 | 3 | 4 | 15 | 1 |
| Switches | 3 | 7 | 0 | 2 | 2 | 6 | 2 |
| Oscillator | 4 | 14 | 4 | 2 | 2 | 10 | 0 |
| STM32 | 9 | 58 | 13 | 25 | 8 | 52 | 25 |
| DS3231 (mit EEPROM U6, RN1, RN2, Batterie) | 7 | 54 | 12 | 6 | 10 | 35 | 2 |
| UART | 3 | 16 | 3 | 3 | 3 | 13 | 3 |
| STM32 Prog. | 2 | 7 | 0 | 2 | 2 | 6 | 2 |
| TSOP Ersatz / DCF77 Ersatz / BOOT1 (je ein Widerstand) | je 1 | je 3 | 0 | je 1 | je 1 | je 2 | je 1 |
| LED-Board-Connector (Titeltext liegt ausserhalb des Rahmens) | 4 | 21 | 3 | 6 | 5 | 14 | 4 |
| DS18B20 | 2 | 8 | 2 | 1 | 2 | 5 | 0 |

### A4 Beschreibung der Zeichnung je Block (✔ Ansicht des gerenderten Blatts, Zahlen aus A3)

- **PowerSupply** (oben, über die ganze Blattbreite): reiner Drahtplan, 158 Drähte, kein einziges Label. Der Fluss läuft von USB1 links über F1, TPS2121 (U9), 5 V-Schiene zum Wandler SY8301 (U10), Spule L2 und Diode D2 zur 3,3 V-Schiene rechts. Die zweite Eingangsschiene (H1, F2, Q3) liegt darunter, parallel zur ersten. Netznamen stehen als Diamant-Symbol am Draht (`USB_5V_INPUT`, `ALT_5V_PROT`, …), siehe A5. Testpunkte TP1 bis TP8 hängen direkt am Draht.
- **Reset, Led STM, Switches, STM32 Prog., TSOP/DCF77/BOOT1 Ersatz:** Mini-Blöcke mit zwei bis sieben Bauteilen. Das Bauteil sitzt zwischen Versorgungssymbol oben und GND unten, der Signalpfad ist ein kurzer Draht mit einem Label am Rand. Das Label ist der Block-Ausgang.
- **ESP12F:** Der Chip steht in der Mitte. Pull-ups und Pull-downs hängen oben beziehungsweise unten per Draht an den Pins, die Signale verlassen den Chip rechts über Stummel mit Labels (`GPIO0_ESP`, `TXD_ESP`, …). Der Reset-Zweig kommt links als Label herein.
- **STM32:** Chip in der Mitte, links die Eingänge und Peripherie-Signale, rechts die Ausgänge und Bus-Signale, jeweils als Stummel mit Label (25 Stück). Die Versorgungs- und Entkopplungsbauteile sind **gezeichnet**: L1, C11, C14 oben am VDD-Pin, C12, C15, C16, C19 unten am VSS, C9 am VCAP1. Nicht benutzte Pins tragen ein Kreuz.
- **LED Power:** zwei Transistoren (Q2 MMBF170 und Q1 SI7149DP), Widerstände R19 bis R23, voll gezeichnet; ein Label am Eingang `WS2812_POWER` und eines am Ausgang `TO_VCC_RGB_5V`. Der Jumper JP2 steht im Strompfad.
- **LED Controll:** U3 (SN74AHCT1G125) mit Eingangslabel links und Ausgangslabel rechts, alles andere per Draht. R17 ist mit rotem Kreuz als nicht bestückt markiert.
- **DS3231:** Das dichteste Bild. U5 steht in der Mitte, die Widerstandsnetze RN1 und RN2 hängen oben und unten per Draht an U5 und U6, was sechs Kreuzungen ohne Junction ergibt (alle in diesem Block, Koordinaten x 219 bis 250 mm, y 194 bis 208 mm). SCL und SDA kommen links als Label, 32K und SQW verlassen den Block rechts als Label. Die Batterie U7 hängt per Draht an VBAT.
- **UART:** Schalter SW4 mit Hinweistext „1-2 -> RUN, 2-3 -> PROG“ und ein Stecker H6 mit Hinweistext „1 > VCC_3.3V, 2 > TX, 3 > RX, 4 > GND“. Ein-/Ausgänge per Label.
- **DS18B20, Oscillator:** vollständig gezeichnet, mit je zwei oder drei Labels am Rand.
- **LED-Board-Connector:** H4, H5 und H7 mit Stummeln und Labels (`TO_VCC_RGB_5V`, `TO_SK6812_IN`, `LDR`). H7 trägt R22 (1 kΩ) am LDR-Zweig per Draht.

### A5 Weitere Messwerte (✔)

- **Versorgungssymbole:** 68 von 68 GND- und VCC-Symbolen stehen in Grundorientierung (Rotation 0), das heisst VCC oben und GND unten. Das **eine** PWR_FLAG-Symbol mit 180° liegt gedreht. Werte: GND 42, `VCC_3.3V` 21, `VCC_5V` 4, `PWR_FLAG` 2; **5 weitere PWR_FLAG-Symbole tragen einen Netznamen als Wert** (`USB_5V_INPUT` ×2, `ALT_5V_INPUT` ×2, `USB_5V_PROT`, `ALT_5V_PROT`). Das V2-Blatt benutzt also das PWR_FLAG-Symbol als Netzmarke in der Versorgung. Das ist eine Eigenart des Nutzers und sieht aus wie ein Namenssymbol (Diamant mit Beschriftung).
- **Ein GND-Symbol (#PWR061)** sitzt ohne Draht direkt auf dem Pin 1 von LED5 (zwei Symbole berühren sich). Das ist die einzige Stelle in V2, an der das so ist (die Regel „zwei Symbole nicht berühren“ aus dem Hardware Codex ist sonst eingehalten).
- **Bauteiltext:** Schriftgrösse für Referenz und Wert überall 1,27 mm (184 von 184). 8 Referenz- oder Wertfelder sind ausgeblendet. Referenz und Wert stehen in der Standardlage der Symbole neben oder über dem Bauteil. An einigen Stellen überlappen sie: SW1/SW2 mit Schalterbeschriftung, H8 bis H10 (drei Befestigungslöcher, Text liegt übereinander), RN1/RN2 nahe an Drähten.
- **Bauteilorientierung:** 72 von 109 Bauteilen in 0°, 21 in 90°, 11 in 180°, 5 in 270°. Widerstände und Kondensatoren stehen also oft senkrecht (Strompfad von VCC oben nach GND unten).
- **Label-Form:** alle 67 globalen Labels haben die Form „input“, auch bei Ausgängen und bidirektionalen Signalen. Der Pfeil unterscheidet Ein- und Ausgang nicht. Die Seite ist das Merkmal: Eingänge eines Blocks stehen links, Ausgänge rechts (Ansicht).
- **Label-Namen:** alle in Grossbuchstaben mit Unterstrich, sprechend (`WS2812_POWER`, `SCL_DS3231`, `TO_VCC_RGB_5V`), und der Block steht im Namen, wo das Signal mehrere Geräte hat (`*_DS3231`, `*_ESP`, `*_STM`, `*_UART`).
- **Netze:** 100 Netze laut KiCad-Netzlistenexport, davon 30 automatisch benannt (`Net-(…)`). Keins davon erscheint als Text im Plan. ✔
- **Titelblock:** Titel „WordClock Controller STM32F411“, Datum 2026-04-12, Revision 3.2, Kommentar „(c) Daniel Kocher“. ✔
- **ERC:** `kicad-cli sch erc` (10.0.2) meldet 95 Verstösse: `pin_to_pin` 86, `pin_not_driven` 3, `lib_symbol_mismatch` 3, `power_pin_not_driven` 2, `footprint_link_issues` 1. Keiner betrifft die Lesbarkeit. ✔

### A6 Was V2 gut lesbar macht (eigene Ansicht des Renderings, ✔ Beobachtung, ○ Deutung)

Gerendert mit `kicad-cli sch export svg`, Vorschau mit `qlmanage -t` (Scratchpad `…/sg/v2.svg.png`, `right.svg.png`).

1. **Das Auge kann Drähten folgen.** Jeder Block ist ein zusammenhängendes Bild, bei dem das Passivbauteil an dem Pin hängt, den es bedient. Man liest Pin → R → Versorgung ohne Suchen.
2. **Die Rahmen und Titel gliedern in Funktionen**, und der Blockname ist ein Wort, das auch in den Label-Namen vorkommt (DS3231 ↔ `SCL_DS3231`).
3. **Labels sind kurz und stehen am Blockrand.** Sie sagen „dieses Signal geht woandershin“ und tragen den Namen des Ziels.
4. **Versorgung ist ein Symbol mit Namen, kein Draht.** `VCC_3.3V` und GND stehen an jedem Bauteil, und ein Symbol bedient mehrere Bauteile. Es entsteht keine Versorgungsschiene aus Draht quer durch das Blatt (Ausnahme: PowerSupply, dort ist die Schiene das Thema).
5. **Dichte Gruppen stehen eng, Blöcke haben Weissraum dazwischen.** Die Bauteile sind per Block auf kleiner Fläche (z. B. Reset 1265 mm², STM32 11'652 mm²), zwischen den Rahmen ist Platz.
6. **Konsistenz:** gleiche Symbolgrösse, gleiche Schrift, Passive stehen gleich (senkrecht), ein Raster.
7. **Hinweise im Plan:** Jumper (RUN/PROG), Steckerbelegung, rote Kreuze für nicht bestückte Teile.

Schwächen von V2, ehrlich: 9 Kreuzungen ohne Junction (6 im DS3231-Block), überlappende Texte (H8 bis H10, SW1/SW2), PWR_FLAG als Netzmarke zweckentfremdet, alle Labels „input“, und der Titeltext des Connector-Blocks liegt ausserhalb des Rahmens.

### A7 Stilregeln des Nutzers (aus V2 abgeleitet)

Die Zahlen stehen oben (✔). Dass es **Regeln des Nutzers** sind und keine Zufälle, ist eine Deutung (○), stützt sich aber auf die Gleichförmigkeit über 17 Blöcke.

1. Ein A3-Blatt, flach, Blöcke durch gestrichelte Rahmen (Regelbereich) mit blauem Titel oben links. Keine Unterblätter.
2. Innerhalb des Blocks: Drähte. Kein lokales Label.
3. Labels (global, Form „input“) nur für Signale, die den Block verlassen, am Rand des Blocks, nach 2,54 bis 6,35 mm Stummel.
4. Der Hub-IC darf viele Stummel haben (STM32: 25), solange jedes Label einen sprechenden Namen trägt.
5. Passive hängen mit Draht an den IC-Pins (Pull-up, Pull-down, Entkopplung), nie über ein Label.
6. Versorgung durch Symbole mit Namen (`VCC_3.3V`, `VCC_5V`, GND), oben beziehungsweise unten am Bauteil, ein Symbol für mehrere Bauteile.
7. Nicht benutzte Pins: Nicht-verbunden-Kreuz. Nicht bestückte Teile: rotes Kreuz und grau.
8. Raster 1,27 mm, nur rechte Winkel.
9. Jumper, Schalterstellungen und Steckerbelegung stehen als blauer Text neben dem Bauteil.
10. Testpunkte und Sicherungen stehen direkt im Strompfad, Netzmarken als Diamant mit Namen.

---

## Teil B: Anerkannte Konventionen und Lesbarkeitsregeln

### B1 Quellen und was ich gelesen habe

| Kürzel | Quelle | Stand | Gelesen |
|---|---|---|---|
| KiCad-HB | KiCad 10.0 Reference Manual, Eeschema, `https://docs.kicad.org/10.0/en/eeschema/eeschema.html` (am 17.10.2026 abgerufen; das Handbuch nennt sich „in Überarbeitung“) | 10.0 | ✔ Abschnitte „Grids and snapping“, „Electrical connections between sheets“, „Multipage and hierarchical design examples“, „Electrical rules checking“ mit Liste der ERC-Prüfungen |
| KLC | KiCad Library Conventions S4.2 „Pins should be grouped by function“, `https://klc.kicad.org/symbol/s4/s4.2.html` | – | ● Nur über eine Suchzusammenfassung. Die Seite selbst ist für den Abruf gesperrt (HTTP 403 bei WebFetch und curl), deshalb **nicht gelesen** |
| Wiki | Wikipedia „Circuit diagram“, `https://en.wikipedia.org/wiki/Circuit_diagram` | abgerufen 17.10.2026 | ✔ Abschnitt „Organization“ und Kreuzungsdarstellung |
| HC | Hardware Codex (Hack Club), „Schematic best practices“, `https://hwdocs.hackclub.dev/user-contrib-guides/schematic-practices/` (beruht auf einem EEVblog-Video) | undatiert | ✔ ganz |
| PCBW | Radek Hladik, „How to make clear and well organized schematics“, PCBWay, `https://www.pcbway.com/blog/PCB_Design_Tutorial/How_to_make_clear_and_well_organized_schematics.html` | 17.04.2021 | ✔ ganz |
| Flux | „PCB Schematic Best Practices“, `https://www.flux.ai/p/blog/pcb-schematic-best-practices` | 2026 laut Titel | ✔ Stichworte gelesen (Fluss, Netzlabel, Blöcke); nicht ganz |
| Altium | Altium Designer Dokumentation „Creating Circuit Connectivity in Your Schematics“, `https://www.altium.com/documentation/altium-designer/creating-circuit-connectivity-schematics` | abgerufen 17.10.2026 | ✔ Abschnitt zu Drähten und Netzlabels |
| tmpdir | Forumsbeitrag „Actually Useful Schematics (in KiCAD!)“, `https://community.tmpdir.org/t/actually-useful-schematics-in-kicad/1536`, fasst die Vortragsfolien und die Checkliste von Andrew Greenberg zusammen | 09.06.2025 | ● (die Folien selbst, Google Docs, nicht abgerufen) |
| IEEE 315 / IEC 60617 | Normen kostenpflichtig | – | **nicht gelesen.** Nur die Titel und die Zusammenfassung „Symbole liegen auf einem Modulraster“ (IEEE 315-1975, über tmpdir ●). Wikipedia nennt IEC 61082-1 (Erstellung von Schaltplänen) und IEC 61346 (Referenzbezeichnungen); beide **nicht gelesen** |
| EMA-Checkliste | `ema-eda.com … 2021SchematicGuidelines_Checklist.pdf` | – | heruntergeladen, **nicht ausgewertet** (kein PDF-Textwerkzeug zur Hand, nicht über `Read` versucht). Keine Aussage daraus |

### B2 Regeln aus den Quellen

| # | Regel | Quelle |
|---|---|---|
| R1 | Fluss links → rechts und oben → unten; Eingänge links, Ausgänge rechts. Positive Versorgung oben, Masse unten. Wikipedia nennt das „üblich, aber nicht universell“. | Wiki ✔, HC ✔, PCBW ✔ (auch innerhalb der Blöcke), Flux ✔, tmpdir ● |
| R2 | Masse zeigt nach unten, Versorgung nach oben. Seitlich nur bei Platznot. | HC ✔, PCBW ✔, tmpdir ● |
| R3 | Pull-ups oberhalb der Leitung, Pull-downs unterhalb. | HC ✔ |
| R4 | Plan in Blöcke nach Funktion teilen (Leistung, MCU, Anschlüsse). Rahmen oder Weissraum, Block beschriften. Nicht zu stark auf Blätter aufteilen. | HC ✔, PCBW ✔, Flux ✔, tmpdir ● |
| R5 | Entkoppelkondensatoren neben den IC-Pin, den sie bedienen. | HC ✔ |
| R6 | Keine Drähte unter Bauteilen, nur horizontale und vertikale Drähte (Ausnahme: absichtlich überkreuzte TX/RX). Höchstens drei Drähte an einer Junction. | PCBW ✔ |
| R7 | Zwei Symbole berühren sich nicht, es steht immer ein Draht dazwischen. Ein Draht läuft nicht durch mehrere Pins hindurch, jeder Pin bekommt seinen eigenen Anschluss. | HC ✔ |
| R8 | Labels hängen nie in der Luft, immer an einem Draht. Ein Label, das nur benennt, und ein Label, das anderswohin verbindet, sehen verschieden aus (verbindendes Label an eigenem Drahtstück). | HC ✔ |
| R9 | Netznamen sprechend (`WATCHDOG_CLK` statt `NetLabel_R1_1`). | Beispiel nur aus einer Suchzusammenfassung zur Flux-Seite (●, im abgerufenen Text nicht wiedergefunden); PCBW ✔ („logical names“) |
| R10 | Versorgung als Symbole mit sinnvollen Namen, nicht als Draht. | tmpdir ● |
| R11 | Titelblock: Projektname, Revision, Datum. | PCBW ✔ |
| R12 | Notizen: Freitext für Blockbeschreibung, Feld am Bauteil für Bauteilhinweise; Testpunkte mit Erwartungswert; „wenn etwas komisch ist, hinterlasse eine Notiz“. | PCBW ✔ (Notizen), tmpdir ● (Testpunkte, Pfeile) |
| R13 | Konsistenz und Symmetrie: gleiche Gruppen gleich anordnen. | PCBW ✔ |
| R14 | Raster 50 mil / 1,27 mm für Symbole und Drähte. Drähte verbinden nur, wenn die Enden genau zusammenfallen. | KiCad-HB ✔ („Grids and snapping“) |
| R15 | Für Verbindungen zwischen Blättern: globale oder hierarchische Labels. Lokale Labels gelten nur im Blatt. Hierarchisch, wenn der Entwurf verschachtelte Blöcke hat oder wiederholte Blöcke; flach, wenn er keine Hierarchie hat. | KiCad-HB ✔ |
| R16 | KLC S4.2 (für Symbole): Plus oben, Eingang/Steuerung links, Ausgang rechts, Masse unten; Leistungswandler: Eingang links, Ausgang rechts. | KLC ● |
| R17 | Kreuzung isolierter Drähte als „Sprung“ (Halbkreis) oder, in CAD, versetzte T-Stellen. Ein Vierwege-Knoten gilt als unklar. | Wiki ✔ (Kreuzungsdarstellung, „staggered“ T-junctions); KiCad-HB ✔ (ERC „Four connection points are joined together“, Voreinstellung: ignoriert) |

### B3 Wo sich die Quellen widersprechen: Labels gegenüber Drähten

- **Für Labels:** PCBW (✔) „Use net labels … reduce the number of wires“, Flux (✔) „Net labels prevent clutter … spaghetti effect“.
- **Für Drähte:** tmpdir/Greenberg (●) „Use wires not labels for connections – your eye can track wires way faster than hunt for labels“ (und: „try to use wires when possible, you can’t always“).
- **Neutral:** Altium (✔): Drähte sind „physical connectivity“ und lassen sich verfolgen, aber viele Drähte machen den Plan „dense and busy“. Labels sparen Draht, aber der Leser muss das Blatt nach den Gegenstellen absuchen. Der Entwickler wähle, auch gemischt.
- **Auflösung (○, eigene Folgerung, deckt sich mit V2):** Drähte für alles, was lokal und kurz ist, Labels nur, wo der Weg lang ist, einen Blockrand überquert oder viele Empfänger hat. V2 macht das genau so (A2).

### B4 Weitere Widersprüche

- **Entkopplung:** HC (✔) „neben den Pin“. PCBW (✔) „es gibt zwei Wege: neben dem IC oder gesammelt in einem Block; dann notiere an jedem Kondensator den IC und Pin“. V2 folgt HC.
- **Rahmen:** PCBW und Flux (✔) empfehlen Rahmen oder gestrichelte Linien. tmpdir (●) sagt Weissraum sei besser und Kästen seien „visual noise“, wenn man übertreibt. V2 benutzt Rahmen (17), aber dünn und gestrichelt.
- **Blätter:** PCBW (✔) „bei kleinen Projekten ein A4, dann A3“. HC (✔) und tmpdir (●) „nicht zu viele Blätter“. KiCad-HB (✔): hierarchisch ist sinnvoll bei verschachtelten oder wiederholten Blöcken, flach sonst. V2 ist ein A3-Blatt.
- **Hierarchie als Blockdiagramm** (tmpdir ●) gegenüber einem flachen Blatt: Für unser Projekt (ein Controller, 17 kleine Blöcke) spricht nichts für Unterblätter (○).

### B5 Prüfbare Einzelregeln, nach den Fragen des Auftrags

- **Labels richtig:** lange Wege, Blockgrenzen, Busse (PCBW, Altium ✔; KiCad-HB ✔ für Blätter). **Drähte richtig:** lokale Verbindungen und Signalpfad im Block (tmpdir ●, V2 ✔).
- **Maximale Drahtlänge und Knickzahl:** keine der gelesenen Quellen nennt eine Zahl. **Keine belegte Schwelle.** Die V2-Werte stehen in Teil C als ○.
- **Kreuzungen:** vermeiden, sonst versetzte T-Stellen oder Sprung (Wiki ✔). KiCad meldet Vierwege-Knoten nur als Option (voreingestellt aus).
- **Junctions:** höchstens drei Drähte an einer Stelle (PCBW ✔). V2: 67 Junctions mit 3, 11 mit 4 Drähten (✔ A, ○ die elf Vierwege-Stellen sind nach PCBW zu prüfen, ich habe sie nicht einzeln angesehen).
- **Mehrfach-Pins eines ICs** (z. B. mehrere VDD, GND): Ich habe in den gelesenen Quellen keine Regel gefunden. V2 (✔): VDD des STM32 oben an einem Draht zusammengefasst, GND-Pins des TPS2121 (3 Stück) einzeln mit eigenem Symbol (✔ Ansicht, ○ V2-Stil).
- **Grosse ICs auf Funktionseinheiten aufteilen:** KiCad kennt mehrteilige Symbole (Units). Quellen gelesen: keine Regel dazu. V2 teilt den STM32 nicht auf (ein Symbol, 49 Pins).
- **Titelblock und Revision:** PCBW ✔.
- **Legende und Jumperhinweise:** PCBW ✔ (Notizen), V2 ✔ (A5).

---

## Teil C: Was sich automatisch prüfen lässt

Alle Schwellen sind aus V2 abgeleitet und damit **○**, solange der Nutzer sie nicht bestätigt. Die „Quelle“ der Regel steht dort, wo ich sie gelesen habe.

| Regel | Quelle | Prüfbar? | Schwelle (○ aus V2) |
|---|---|---|---|
| Anteil Label-Stummel an Drahtpins | tmpdir ●, Altium ✔ (neutral), V2 | ja, per Skript (Drahtnetz + Labelposition) | gesamt **≤ 25 %** (V2 16 %, V3 53 %). Je Block ≤ 50 %, ausgenommen ein Hub-IC (V2 STM32 48 %) |
| Anteil Pins, die mit Draht zu einem anderen Pin oder Versorgungssymbol laufen | Greenberg ● | ja | **≥ 65 %** der Drahtpins (V2 72 %, V3 42 %) |
| Label-Name kommt in mindestens zwei Blöcken oder an mindestens zwei Pins vor | KiCad ERC „Label connected to only one pin“ (Warnung) ✔, V2 | teilweise: ERC deckt „nur ein Pin“. Die Blockzuordnung braucht ein Skript | V2: 28 von 31 Namen erfüllen es, 3 dokumentierende Ausnahmen. Schwelle: ≤ 3 Ausnahmen, jede begründet |
| Kein lokales Label innerhalb eines Blocks | tmpdir ● | ja, Skript | V2: 0. Schwelle: 0 Labels, die nur Pins desselben Blocks verbinden |
| Sichtbare automatische Netznamen `Net-(` im Plan | Flux ✔, HC ✔ | ja (Label-Text) | **0** (V2 0, V3 62 Instanzen). Netze ohne Label bleiben unbenannt und sind erlaubt (V2 30 von 100) |
| Drahtlänge Pin zu Pin in einem Block | – (keine Quelle) | ja | 2-Pin-Netze: ≤ 45 mm (V2 Max 43,2 mm) und ≤ 2 Segmente (V2: 46 von 50 Netzen) |
| Länge eines Label-Stummels | V2 | ja | 2,54 bis 7,62 mm (V2 Max 6,35 mm). Länger wäre ein Weg, kein Stummel |
| Drahtkreuzungen ohne Junction | Wiki ✔ | ja, Skript (rechtwinklige Innenschnitte) | ≤ 1 je Block, gesamt ≤ 9 (V2 9, davon 6 im DS3231-Block). Ziel: weniger als V2 |
| Vierwege-Knoten | KiCad ERC „Four connection points are joined together“ ✔ | ja, ERC schaltbar (voreingestellt „ignorieren“) | 0 oder begründet. V2 11 Stellen mit 4 Drähten |
| Zwei Symbole berühren sich ohne Draht | HC ✔ | ja, Skript (Pin auf Pin) | **0** (V2: 1, LED5 mit GND) |
| Versorgungssymbol am Bauteil, nicht als freier Stummel | HC ✔, tmpdir ● | ja | 100 % der VCC/GND-Symbole mit Draht am Pin (V2 74 von 75, das eine berührt direkt) |
| VCC oben, GND unten (Rotation 0) | HC ✔, PCBW ✔, tmpdir ● | ja (Rotation des Symbols) | 100 % (V2 68 von 68) |
| Pull-up über, Pull-down unter dem Signal | HC ✔ | schwer; Skript vergleicht Y-Lage des Widerstands mit dem Signalpin | nicht abgeleitet |
| Eingänge links, Ausgänge rechts am IC | KLC ● | nur für Bibliothekssymbole, nicht für den Plan. Skript vergleicht Pin-Seite und Pin-Typ | nicht abgeleitet |
| Entkoppelkondensator nahe am Versorgungspin | HC ✔ | ja, Abstand Kondensatorpin zu IC-Versorgungspin über Draht | Drahtweg ≤ 30 mm (○, V2 für C11/C14/C12… nicht einzeln gemessen). Nicht für V2 belegt |
| Raster 1,27 mm für Drahtenden und Pins | KiCad-HB ✔ (Raster), KiCad ERC „off connection grid“ ✔ | ja, ERC | 100 % (V2 898 von 898) |
| Diagonale Drähte | PCBW ✔ | ja | 0 (V2 0) |
| Drähte nicht unter Bauteilen | PCBW ✔ | ja, Skript (Draht gegen Symbolrechteck) | 0. Ich habe es für V2 **nicht gemessen** |
| Text überlappt Text oder Symbol | HC ✔ (Labels nicht in der Luft), eigene Beobachtung | ja, mit Rechtecken aus dem SVG | 0 (V2 mindestens 4 Stellen, nicht gezählt) |
| Titelblock mit Titel, Datum, Revision | PCBW ✔ | ja | alle drei Felder gesetzt (V2 ja) |
| Block hat Rahmen und Titeltext | PCBW ✔, HC ✔ | ja | je Block ein Rahmen und ein Titel in der Nähe der Ecke (V2 16 von 17, der Connector-Titel liegt ausserhalb) |
| Hinweistext für Jumper und Stecker vorhanden | PCBW ✔ | nur Zählung des Textes | Mindestens ein Text je Jumper (V2: 2 Texte für 2 Stecker/Schalter, JP2 und JP3 ohne Text) |
| Nicht benutzte Pins mit Kreuz | KiCad ERC „Pin not connected“ ✔ | ja, ERC | 0 Verstösse (V2 meldet 0 solche Fehler, ✔ ERC-Ergebnis) |

### C1 Was KiCad selbst bietet (✔ KiCad-HB, Liste der ERC-Prüfungen, und eigener Lauf)

- ERC prüft: Pin nicht verbunden, Eingang nicht getrieben, Label nicht verbunden (Fehler), **Label nur an einem Pin** (Warnung), globales Label nur einmal (ignoriert), Draht ohne Verbindung (Fehler), Drahtende ohne Verbindung (Warnung), Pin oder Drahtende neben dem Raster (Warnung), Vierwege-Knoten (ignoriert), Label an zwei Drähten (Warnung), doppelte Referenzen.
- **Nichts davon misst Lesbarkeit.** Es gibt keine ERC-Regel für Label-Stummel gegenüber Drähten, für automatisch benannte Netze, für Drahtlänge, Kreuzungen ohne Junction oder Textüberlappung. Der eigene Lauf an V2 (95 Verstösse) zeigt dazu nichts (✔ A5).
- `kicad-cli sch erc` bietet nur Ausgabeformat, Einheiten und Schweregrade (`--severity-*`, `--exit-code-violations`). Eine eigene Regel kann man dort nicht einhängen. Die Lesbarkeitsprüfung muss deshalb ein eigenes Skript über die `.kicad_sch` sein.
- „Label connected to only one pin“ trifft einen Label-Stummel **nicht**, solange eine Gegenstelle mit demselben Namen existiert, denn die Prüfung zählt Pins im Netz, nicht im Draht (○ nach Wortlaut der Beschreibung; ich habe keinen V3-Lauf dazu ausgewertet).

---

## Teil D: Skriptgesteuert zeichnen

### D1 Strategie, die einen lesbaren Plan erzeugt (○, Folgerung aus A und B)

1. **Eingabe:** Netzliste (V2-Export) plus Zuordnung Bauteil → Block, plus je Block eine Reihenfolge „Eingangsseite links, Ausgang rechts“.
2. **Netzklassen bilden:** Versorgung (VCC, GND, Namen aus der Netzliste) → Symbole am Bauteil. Signalnetz **innerhalb** eines Blocks → Draht. Signalnetz über eine Blockgrenze → Label am Blockrand an beiden Enden.
3. **Platzieren nach Rolle:** Hub-IC in der Blockmitte; Pins nach Funktion links/rechts/oben/unten (KLC-Regel). Passive, die an genau einem IC-Pin hängen (Pull-up, Pull-down, Entkopplung, Vorwiderstand), **unmittelbar am Pin** in Richtung der Pinseite. Versorgung oben, Masse unten. Rasterplatzierung auf 1,27 mm.
4. **Verdrahten:** Manhattan-Drähte mit höchstens einem oder zwei Knicken; Kreuzungen als Gütefaktor minimieren, Vierwege-Knoten verbieten.
5. **Prüfen:** Das Skript aus Teil C läuft nach jedem Lauf und bricht bei einer Überschreitung ab.

Genau diese Verdrahtung (Schritt 4) und die Platzierung der Passiven (Schritt 3) fehlen im V3-Entwurf: Dort steht jedes Bauteil für sich, mit einem Stummel und einem Label an jedem Pin (✔ Ansicht des V3-Renderings; 156 von 296 Drahtpins sind Label-Stummel).

### D2 Erprobte Werkzeuge

Metadaten am 17.10.2026 von PyPI und der GitHub-Schnittstelle gelesen (✔), Funktionsangaben aus den README-Texten (●).

| Werkzeug | Lizenz | Stand | Was es kann | Eignung für unser Ziel |
|---|---|---|---|---|
| **SKiDL** 3.0.0 | MIT | Release und Commit 16.10.2026, 1693 Sterne | Schaltung als Python-Code; Ausgabe Netzliste und, laut README (●), „placed-and-routed“ Schaltplan als KiCad-Datei oder SVG, „KiCad 6–10“; Hierarchie als Unterblätter | **Selbst erprobt, siehe D3.** Funktioniert, aber das Ergebnis ist nicht lesbar genug |
| **kicad-sch-api** 0.5.6 | MIT | Release 19.11.2025, 54 Sterne | Lesen/Schreiben von `.kicad_sch` mit „exakter Formattreue“; Manhattan-Routing mit „basic obstacle avoidance“ (●, README); MCP-Server | Geeignet als **Schreiber** für Bauteil, Draht, Label mit Koordinaten. Keine Platzierungslogik. Mit KiCad 10 nicht erprobt (nicht ausprobiert, ○) |
| **kicad-skip** 0.2.5 | LGPL-2.1 | Release 16.02.2024, letzter Commit 26.05.2024 | Bearbeiten von KiCad-7+-Dateien (Schaltplan, Netzliste, Platine) | Nur Bearbeitung vorhandener Elemente. Seit mehr als einem Jahr ohne Pflege, für KiCad 10 nicht erprobt |
| **kiutils** 1.4.8 | GPL-3.0 | Release 02.02.2024 | Parser für KiCad-6+-Dateien | Parser, kein Layout. Lizenz GPL-3.0 beachten. Seit 2024 ohne Pflege |
| **atopile** 0.15.9 | MIT | Release 12.09.2026, 3994 Sterne | Sprache und Werkzeugkette; laut README (●) werden Bauteile gewählt, Regeln geprüft und das **KiCad-Layout (`.kicad_pcb`)** aktualisiert | Ich habe im README **keine Schaltplan-Erzeugung** gefunden (○, README nur durchsucht). Eine Suchzusammenfassung behauptet „produces schematics“, belegt ist das nicht. Für unseren Zweck nicht geeignet |
| **netlistsvg** | MIT | letzter Commit 25.01.2024, 831 Sterne | Zeichnet SVG aus Yosys-JSON mit `elkjs`-Layout, eigene „Skins“ | Ausgabe ist SVG, kein KiCad-Schaltplan. Digital-Gatter-orientiert. Als Vorschau-Hilfe denkbar, nicht als Quelle der `.kicad_sch` |
| **KiCad 10 IPC-API (`kipy`)** | – | KiCad 10 | Python-Anbindung an ein laufendes KiCad | Für die Platine; laut Sekundärquellen (●) liefert KiCad 10 für den Schaltplan **keinen** Bearbeitungsbefehl, KiCad 11 soll es nachziehen. Nicht geeignet |
| **KiCad „Auto-Annotate“, Autoplace** | – | – | Der Schaltplaneditor hat meines Wissens keine automatische Verdrahtung oder Platzierung | Im Handbuch 10.0 (Abschnitte gelesen, nicht vollständig) **nichts gefunden**, ○ |

### D3 Eigener Versuch mit SKiDL 3.0.0 (✔, Scratchpad `…/sg/sk/`, in einer eigenen Python-Umgebung, nichts im Repo installiert)

Testschaltung: ein DS3231 mit zwei Pull-ups, einem Kondensator und einer Batterie als Block „rtc“, dazu ein Stecker, eine LED mit Vorwiderstand und ein Kondensator als Block „mcu“; KiCad-10.0.2-Symbole.

- **Erster Lauf mit den Voreinstellungen (zwei Wiederholungen): `RoutingFailure`.** Ein zweiter Lauf mit `flatness=1.0, retries=8` lieferte Dateien. Das Ergebnis hängt also vom Zufall der Platzierung ab (○, ich habe nur diese zwei Läufe gemacht).
- **Das Ergebnis ist schlecht lesbar** (Ansicht des SVG, `…/sg/sk/out/skt1_rtc1.svg.png`): Drähte laufen durch den IC-Körper und an Pins entlang, Referenzen, Werte und Pinnamen überlagern sich (R1/R2, BT1/Battery_Cell), die Bauteile sind auf kleinem Raum zusammengedrängt, und die Signalnamen stehen als Labels am rechten Rand. Ob die Verbindungen elektrisch richtig sind, habe ich nicht gegen die Netzliste geprüft (nur die Zeichnung angesehen). Ein Mensch würde diesen Block nicht so zeichnen und nicht so lesen können wie die V2-Blöcke.
- Die Hierarchie wird als zwei Unterblätter erzeugt (`skt1_rtc1`, `skt1_mcu1`), das ist nicht unsere Struktur (flach, ein Blatt).

### D4 Ehrliche Aussage zur Machbarkeit

- **Ein Werkzeug, das aus Netzliste und Blockzuordnung selbstständig einen Plan in V2-Qualität zeichnet, habe ich nicht gefunden.** Das Fehlen belegt nicht, dass keines existiert; ich habe sieben Werkzeuge und eine Suche gesichtet und eines erprobt (SKiDL: lauffähig, aber nicht lesbar und nicht deterministisch). Die Aussage ist ✔ für SKiDL, ● für die anderen.
- Eine Suchzusammenfassung (●) nennt das Platzieren von Hand als Normalfall: „actual auto placement that works well is fairly uncommon“. Das ist ein Forumsbeitrag, keine Messung, ich stütze mich nicht darauf.
- **Was zuverlässig geht (○, Folgerung):** Ein Skript kann Dinge **schreiben** (Bauteile, Drähte, Labels, Symbole, Text, Rahmen) und die Qualitätsprüfung aus Teil C **lesen**. Es kann Platzierungsmuster für kleine, regelmässige Blöcke ausführen (zum Beispiel „Pull-up über dem Pin“).
- **Was nicht verlässlich geht:** die freie Platzierung und Verdrahtung grosser Blöcke (STM32-/ESP32-Umfeld, Versorgungsblock mit 44 Bauteilen) ohne einen Menschen.

**Alternativen, in absteigender Zuverlässigkeit (○):**

1. **V2-Blöcke als Vorlagen übernehmen.** Für Blöcke, die in V3 unverändert bleiben (Reset, Switches, LED Power, LED Controll, DS3231 mit EEPROM, UART, DS18B20, LED-Board-Connector, Ersatzwiderstände), liest das Skript Bauteile, Drähte, Labels und Texte **mit Koordinaten aus der V2-Datei** und schreibt sie unverändert nach V3 (nur Referenzen und Netznamen laut Netzliste prüfen). Damit bleibt die Handzeichnung des Nutzers erhalten. Das ist ein Kopieren von Daten, kein Zeichnen.
2. **Halbautomatisch für geänderte Blöcke** (ESP32-S3 statt STM32 und ESP-12F, PowerSupply mit anderen Bauteilen): Das Skript setzt den IC und die Passiven nach den Regeln in D1 an den Pin und zieht Manhattan-Drähte; ein Mensch (Nutzer oder Designer) schiebt nach, danach läuft die Prüfung aus Teil C.
3. **Handzeichnung in KiCad** durch den Nutzer für die Blöcke, die sich nicht sinnvoll erzeugen lassen, mit der Prüfung aus Teil C als Abnahme.

---

## Offene Punkte

1. **Schwellen in Teil C sind ○.** Der Nutzer sollte bestätigen, ob V2-Niveau (Label-Stummel ≤ 25 %, Kreuzungen ≤ 9) die Messlatte ist.
2. **KLC S4.2** konnte ich nicht selbst lesen (403). Der Wortlaut stammt aus einer Suchzusammenfassung (●). Vor einer Verwendung als Pflichtregel gegenlesen.
3. **IEEE 315, IEC 60617, IEC 61082-1, IEC 61346** nicht gelesen (kostenpflichtig). Keine Aussage daraus.
4. **EMA-Checkliste** und die Folien von Greenberg nicht ausgewertet.
5. **Nicht gemessen in V2:** Drähte unter Bauteilen, Textüberlappungen (nur Sichtbefund), Entkoppelkondensator-Abstand, die elf Vierwege-Knoten, die Frage, warum JP2/JP3 ohne Hinweistext stehen.
6. **SKiDL:** zwei Läufe, ein Testfall. Ob andere Optionen (`flatness`, Platzierungsparameter, Symbol-Hinweise) ein lesbares Bild liefern, ist nicht untersucht.
7. **kicad-sch-api** unter KiCad 10 nicht ausprobiert. Vor einer Entscheidung eine Schreibprobe mit `kicad-cli sch erc` und Render machen.
8. **PWR_FLAG als Netzmarke** (5 Symbole in V2): Soll V3 diese Eigenart übernehmen oder echte Versorgungssymbole mit Namen benutzen? Das ist eine Entscheidung für den Architekten oder Nutzer, nicht für mich.
9. Die Messskripte liegen nur im Scratchpad der Sitzung. Sollen sie ins Repo, übernimmt das der `hw-designer` (Besitz `hardware/esp32s3/**`).
