# Wissensstand der Hardware — gesicherte Erkenntnisse mit Beleg

Stand: **17.10.2026** (Erstfassung des Hardware-Dokumentars). Gilt für den Umbau der Controller-Platine
von STM32F411 + ESP-12F (V2) auf einen einzelnen ESP32-S3-WROOM-1 (Arbeitsname **V3**).

Dieses Dokument trägt, **was gesichert ist, und womit**. Was nur vermutet wird, steht in
[offene-punkte.md](offene-punkte.md). Entscheide stehen in [entscheidungsuebersicht.md](entscheidungsuebersicht.md),
nicht hier.

## Wie die Belege gekennzeichnet sind

Das Hardware-Team kennt drei Zeichen (✔ gelesen, ● Sekundärquelle, ○ Annahme). Weil in diesem Dokument zweierlei
„gelesen“ vorkommt, gibt es dafür zwei Schreibweisen:

| Zeichen | Bedeutung |
|---|---|
| **✔R** | Vom Dokumentar am 17.10.2026 **selbst an einer Datei, einem Export oder durch einen Befehl nachgeprüft**. Die Fundstelle (Datei, Befehl) steht dabei |
| **✔K** | Aus einem **Komponentenblatt** übernommen, das die Aussage dort mit ✔ und Datenblattseite trägt. Der Dokumentar hat das PDF **nicht** gelesen; die Seite steht im Blatt |
| **●** | Sekundärquelle (Distributor-Datenbank, Hersteller-Forum). Quelle genannt |
| **○** | Annahme, Rechnung oder Erinnerung. **Nie** als Tatsache zu lesen |

Eine Annahme wird hier nie zur Tatsache. Wo ein Blatt ○ führt, steht ○.

**Drei Hinweise zur Nachprüfbarkeit.**

1. Die Datenblatt-PDFs liegen unter `hardware/komponenten/datenblaetter/` und sind **nicht im Git**
   (`.gitignore`, Urheberrecht und Gewicht). Wer ein ✔K nachprüfen will, lädt das PDF anhand der Adresse im
   Quellenabschnitt des jeweiligen Blatts neu. Ändert der Hersteller die Datei, kann die Seitenzahl abweichen
   (die Blätter nennen Revision und Datum der gelesenen Fassung).
2. **Zählungen** (Bauteile, Netze, Meldungen) stehen hier nur mit Quelle und Befehl. Den **aktuellen** Stand zeigt
   `python3 tools/checks/hardware.py`; eine abgeschriebene Zahl veraltet still (DIR-006).
3. Das V2-Projekt unter `~/Documents/WordClock-USB C - STM32F411 - V2` wurde nur **gelesen**, auch von den
   Prüfungen hier. Zwei Leseläufe (DRC-Vergleich, Footprint-Auslesen) liefen auf Kopien bzw. im Lesemodus.

---

## 1. Der Bestand: V2 ist die Netzliste, nicht `HARDWARE.md`

**W-1.** Massgebend für „was hängt woran“ ist der **Netzlistenexport des V2-Schaltplans**
(`hardware/esp32s3/v2/v2.xml`, erzeugt mit `kicad-cli sch export netlist --format kicadxml`), **nicht** `HARDWARE.md`.
✔R: Export gelesen, 109 Bauteile, 100 Netze (davon 68 echte Netze ohne „unconnected-“), `hardware/esp32s3/v2/v2.xml`.
Der Export ist reproduzierbar (`python3 hardware/esp32s3/export_v2.py`), und Prüfung H6 meldet, sobald sich der
V2-Schaltplan gegenüber dem gespeicherten Stand ändert.

Ursache der Abweichungen: `HARDWARE.md` entstand aus einer Abschrift (so benennt es der Skill
`.claude/skills/kicad-schema/SKILL.md`; `CLAUDE.md`, R3, nennt es als Anlass der Beweispflicht des Hardware-Teams).

### W-2. Korrekturen an `HARDWARE.md` und den Annahmen davor

Jede Zeile ist am V2-Netzlistenexport nachgeprüft (✔R, `hardware/esp32s3/v2/v2.xml`). „Art“ sagt, ob
`HARDWARE.md` etwas **Falsches** sagt oder etwas **auslässt**.

**Stand von `HARDWARE.md`.** Der Lead hat `HARDWARE.md` am 17.10.2026 gegen die Netzliste berichtigt (Commit „Doku: HARDWARE.md nach der V2-Netzliste berichtigt, Befunde L485-L491“); die Befunde stehen in `BEFUNDE.md` (L485–L491, ToDo E42–E48), das lebende Rahmendokument
ist `knowledge/hardware-esp32s3-umbau.md`. Die Tabelle unten ist die **unabhängige Gegenprobe des Dokumentars** und hält die Fassung **vor** der Berichtigung fest („Bis 17.10.2026 stand …“). Was die berichtigte Fassung **noch nicht** trägt oder **anders** sagt,
steht im Anschluss an die Tabelle (Punkte a7 und e2, W-4d, OP-35).

| Nr. | Bis 17.10.2026 stand / galt | Das stimmt nicht mehr, weil … | Art |
|---|---|---|---|
| a1 | `HARDWARE.md` (Z. 38–39) führt `TSOP` (IR-Empfänger, PB4) und `DCF77` (PB8) als Controller-Signale auf; `CLAUDE.md` hält fest, dass die Doku sie als **angeschlossen** nannte | Die Netze `TSOP` und `DCF77` bestehen nur aus dem Controller-Pin und je einem **100-kΩ-Pull-up** (`R8` bzw. `R9` nach 3,3 V). **Es gibt auf dem Controller keinen Stecker** dafür. ✔R: Netz `TSOP` = {R8.1, U2.40}, `DCF77` = {R9.1, U2.45} | falsch (Anschluss behauptet) |
| a2 | `HARDWARE.md` (Z. 73) beschreibt den Schaltweg der LED-Versorgung als `PB0 → R19 → Q2 → Gate von Q1` | Der Weg hat mehr Teile: **`R20` (100 kΩ) zieht das Gate von Q2 nach Masse** (Pull-down existiert), `R21` (3,3 kΩ) zieht den Drain von Q2 nach `VCC_5V`, **`R23` (2,2 kΩ) liegt zwischen Drain von Q2 und Gate von Q1**, `C10` (100 nF) zwischen `VCC_5V` und Gate von Q1, `JP2` (offene Lötbrücke) überbrückt `VCC_5V` und `TO_VCC_RGB_5V`. ✔R: Netze `Net-(Q2-G)`, `Net-(Q2-D)`, `Net-(C10-Pad2)`, `TO_VCC_RGB_5V` | fehlt |
| a3 | `HARDWARE.md` (Z. 103–104): Schiebeschalter `SW4` „in der RX-Leitung“ der STM-Debug-UART; Z. 37: PA9/PA10 = `RX_UART`/`TX_UART` | **Das stimmt**, wenn man die Netznamen aus Sicht des Steckers liest: Netz `RX_UART` = **PA9** (STM-USART1-**Sendung**) an `H6` Pin 3 **und an `GPIO13` des ESP-12F** (`U4` Pin 7), Netz `TX_UART` = **PA10** (STM-**Empfang**) über `SW4` an `H6` Pin 2. `SW4` (Pins 1, 2, 3 an `GPIO15_ESP`, `TX_UART`, `Net-(H6-Pad2)`) legt `H6` Pin 2 also wahlweise an den STM-Empfang oder an `GPIO15_ESP`; der Skill beschreibt dasselbe als Umschalter zwischen STM-UART und zweiter ESP-UART. ✔R: `v2.xml`, Netze `RX_UART`, `TX_UART`, `GPIO15_ESP`; die berichtigte `HARDWARE.md` und `BEFUNDE.md` L486 führen denselben Befund samt der offenen Frage, welcher Pin von `SW4` der gemeinsame Kontakt ist (E43). Mit `U4` entfällt in V3 der zweite UART-Weg des ESP. **Folge für V3 (Auffälligkeit, OP-35):** Die Netznamen haben in V2 die Sicht des Steckers, in V3 ordnet der Generator sie **nach dem Namen** zu (`TXD0` → `TX_UART`, `RXD0` → `RX_UART`). Damit liegt der **Sendeausgang** des Moduls an `H6` Pin 2, wo bei V2 der **Empfangseingang** des STM lag | keine Abweichung in V2; **offene Frage** für V3 |
| a4 | `HARDWARE.md` nennt nur `H3` als SWD-Stecker (Z. 42) und sagt nichts über die 3,3-V-Schiene an den Steckern | **`H3` und `H6` speisen das 3,3-V-Netz über Dioden ein**: `D1` (Anode an `H3.1`) und `D3` (Anode an `H6.1`), beide Kathoden an `VCC_3.3V`. Der Wandler `U10` speist über `D2` (Anode hinter `L2`). Das 3,3-V-Netz ist damit eine Diodenverknüpfung aus drei Quellen. ✔R: Netze `Net-(D1-A)`, `Net-(D3-A)`, `VCC_3.3V` | fehlt |
| a5 | `HARDWARE.md` erwähnt `SQW_DS3231` und `32K_DS3231` nicht | Beide Netze bestehen nur aus dem Ausgang des DS3231 (`U5`) und je einem Pull-up im Widerstandsarray `RN2` (Pins 5 und 6). **Sie gehen nicht an den Controller.** ✔R: Netze `SQW_DS3231` = {RN2.5, U5.3}, `32K_DS3231` = {RN2.6, U5.1} | fehlt |
| a6 | `HARDWARE.md` führt keine Bauteilzahl; die Stückliste von V2 zählt 92 Bauteile | **V2 hat 109 Bauteile** (✔R: `v2.xml`, `<comp>` gezählt; das V2-Layout `.kicad_pcb` hat ebenfalls 109 Footprints, ✔R per Skript `sx.py`). Die Differenz zur Stückliste erklärt W-4 | fehlt |
| a7 | `HARDWARE.md` (Z. 18): `U5` DS3231MZ sei eine „Echtzeituhr mit TCXO“ | Der **DS3231M(Z)** hat einen **MEMS-Resonator**, ±5 ppm, nicht den Quarz des DS3231 (±2 ppm). ✔K `hardware/komponenten/ds3231mz.md` §7 (Datenblatt DS3231M, Rev 7, S. 1). Folge: höchstens rund ±0,43 s/Tag | falsch |

Die Belege a1 bis a6 stehen auch im Skill `kicad-schema` und sind in der berichtigten `HARDWARE.md` nachgezogen. **Noch nicht nachgezogen** (Stand der Erstfassung): **a7** (die berichtigte `HARDWARE.md` sagt in Z. 29 weiterhin „Echtzeituhr mit TCXO“; der DS3231M hat einen MEMS-Resonator, ✔K `ds3231mz.md`),
die **LCSC-Nummer von `R23`** (W-4d) und der **Footprint von `D1`–`D3` im Layout** (W-5, e2: `HARDWARE.md` und `BEFUNDE.md` L487 sagen, der Widerspruch liege „nur in der Stückliste“ — das V2-Layout trägt aber ebenfalls SOD-123). Das geht als Übergabeliste an den Lead und den `doc-writer`.

### W-3. Das V2-Layout und seine Zahlen

**W-3a. Format.** ✔R, `tools/checks/hardware.py` (Funktion `pcb_geometry`) gegen `V2.kicad_pcb`: Umriss **145,00 × 39,00 mm**
(Edge.Cuts), zwei Kupferlagen laut Skill. Positionen relativ zur linken oberen Ecke:

| Merkmal | Position (mm) |
|---|---|
| `H8`, `H9`, `H10` (Befestigung, M2,5) | (3,25 / 28,00), (72,50 / 3,25), (141,75 / 28,00) |
| `H1`, `H2` (externer Stromeingang) | (35,75 / 27,75), (30,25 / 27,75) |
| `USB1` | (72,50 / 35,38), Unterkante, Mitte |
| `H7`, `H4`, `H5` (LED- und LDR-Stecker) | (38,63 / 4,75), (12,00 / 4,29), (26,00 / 3,25) |
| `H6` (Konsole) | (103,75 / 4,50) |
| `SW3`, `SW2`, `SW1` | (86,00 / 27,75), (99,00 / 27,75), (112,00 / 27,75) |
| `U4` (ESP-12F, Antenne an der Oberkante) | (127,00 / 12,25) |

Die Prüfung H7 vergleicht den Umriss und 13 Positionen (Toleranz 0,05 mm für Löcher und Pads `H1`, `H2`, `H8`–`H10`, 1,0 mm
für die Footprint-Ursprünge der Stecker und Taster). Das Format der Spec zählt 14 Merkmale (Umriss + 13); die beiden Zahlen
stimmen überein. Ein Layout für V3 gibt es **noch nicht**; H7 meldet bis dahin „noch kein Layout“.

**W-3b. DRC braucht die Projektregeln.** ✔R: `kicad-cli pcb drc` am V2-Layout, einmal im Projektverzeichnis (mit
`.kicad_pro` und `.kicad_dru`), einmal mit einer Kopie nur der `.kicad_pcb` im leeren Verzeichnis:

| Lauf | Ergebnis |
|---|---|
| mit Projektregeln | **241** Meldungen, davon **5 Fehler** (`annular_width`); der Rest sind Warnungen (Siebdruck u. a.), dazu 1 unverbundenes Element |
| ohne Projektregeln | **504** Fehler `clearance` und **35** Fehler `copper_edge_clearance` (mit Projektregeln: 0), dazu 2 Fehler `annular_width` |

Das V2-Layout hat also selbst 241 Meldungen; sie sind in `tools/checks/hardware-drc.baseline` als Obergrenze festgehalten
(H7 verlangt „nicht schlechter als V2“). Ein DRC ohne Projektregeln ist **falsch, nicht bloss schlechter**; H7 verweigert ihn
(Gegenprobe P14b). Die Zahl 504 und die 35 Randabstände hat der Layouter-Auftrag schon genannt; sie sind hier neu
nachgemessen.

### W-4. Die V2-Stückliste ist unvollständig

**W-4a.** `production/bom.csv` des V2-Projekts hat 50 Zeilen und 92 Bauteile (✔R: Summe der Mengen). Der Schaltplan hat 109 Bauteile.
Davon tragen 15 in der Schaltplan-Datei `in_bom = no` und/oder `dnp = yes` (Testpunkte `TP1`, `TP4`–`TP8`, Lötbrücken `JP2`,
`JP3`, Befestigungslöcher `H1`, `H2`, `H8`–`H10`, bestückungsfrei `R7` = 1 MΩ und `R17` = 0 Ω). Rechnung: 109 − 15 = 94 sollten in der
Stückliste stehen, es sind 92.

**Es fehlen genau zwei: `R23` (2,2 kΩ) und `C10` (100 nF).** ✔R: Beide tragen im Schaltplan `in_bom = yes`, `dnp = no`; im V2-Layout haben sie das Attribut
`smd` ohne `exclude_from_bom` und tragen ein LCSC-Feld (`R23`: C17841, `C10`: C49678). Beide fehlen auch in `production/positions.csv`, `production/designators.csv` und in den
Dateien unter `jlcpcb/production_files/` (Stückliste und Bestückungsliste).

**W-4b. Warum das zählt.** `R23` und `C10` sitzen im Schaltweg der LED-Versorgung: Q2 (Drain) → `R23` → Gate von Q1, und `C10`
zwischen `VCC_5V` und dem Gate von Q1 (✔R: `Net-(Q2-D)`, `Net-(C10-Pad2)`). Fehlt `R23` an der Platine, ist das Gate von Q1 vom Treiber
getrennt, und Q1 schaltet nicht; es sei denn, die Lötbrücke `JP2` (parallel zu Q1) ist geschlossen. Ob die Teile an der gebauten V2-Platine
bestückt sind, ist **nicht belegt** (OP-01).

**W-4c. Mutmasslicher Hergang (○).** Die Fertigungsdateien tragen das Datum 23.03.2026, der Schaltplan 12.04.2026, das Layout 30.09.2026 (Dateidaten
im V2-Projekt). `R23` und `C10` sind also vermutlich nach dem Export von März hinzugekommen. **Das ist eine Folgerung aus Dateidaten, keine Aussage des Projekts.**

**W-4d. Ein zweiter Fehler in derselben Gegend.** Das LCSC-Feld von `R23` ist **C17841**. Diese Nummer ist in der Stückliste und im JLCPCB-Bestand der
**82-Ω**-Widerstand von `R19` (✔R: `production/bom.csv` Zeile `R19,0805,1,82Ω,C17841`; ✔K `jlcpcb-bestand.md`). `R23` hat den Wert 2,2 kΩ. Das Feld ist
mutmasslich von `R19` kopiert (○). **Die richtige Nummer für 2,2 kΩ ist nicht geprüft** und darf nicht geraten werden (OP-06).

**W-4e. Die Aussage in `jlcpcb-bestand.md` deckt sich damit.** Das Bestandsblatt vermerkt „1 MΩ und 2,2 kΩ kommen in der Stückliste nicht vor“ und „R7 steht nicht in der Stückliste“.
Das ist richtig und jetzt erklärt: `R7` (1 MΩ) ist bestückungsfrei (`dnp`), `R23` (2,2 kΩ) ist die Lücke aus W-4a.

### W-5. Weitere Bestandsfehler (Komponentenblätter und Gegenprobe)

| Nr. | Befund | Beleg |
|---|---|---|
| e1 | **`R29`** trägt im Schaltplan den Wert **32 kΩ**; das bestellte Teil C2930156 ist **31,6 kΩ** (E96, FOJAN FRC0805F3162TS). Der Rückkopplungsteiler von `U10` ergibt damit 3,30 V (32 kΩ) bzw. 3,33 V (31,6 kΩ); beide sind in Ordnung, aber Schaltplan und Teil müssen **denselben** Wert tragen | ✔R: `v2.xml` (R29 = „32kΩ“, LCSC C2930156) und `bom.csv`; ✔K `jlcpcb-bestand.md`, `sy8301abc.md` §2, §7 |
| e2 | **`D1`, `D2`, `D3`** (PMEG2010ER, Nexperia, **SOD-123W**): Die Stückliste und das **V2-Layout** führen den Footprint `EasyEDA:SOD-123_L2.8-W1.8-LS3.7-RD` (SOD-123), der **V2-Schaltplan** dagegen `Diode_SMD:Nexperia_CFP3_SOD-123W`. Schaltplan und Layout stimmen also nicht überein, und das Layout trägt den Footprint, den das Teil laut JLCPCB-Quelle **nicht** hat. **Das widerspricht der Aussage in `HARDWARE.md` und `BEFUNDE.md` L487** („Ob das Layout den Footprint der Stückliste oder den des Schaltplans verwendet, ist nicht geprüft“): Es ist jetzt geprüft, das Layout trägt den der Stückliste | ✔R: `v2.xml` (Footprint-Feld), `bom.csv` Zeile `"D1, D2, D3"`, Footprint-Auslesen aus `V2.kicad_pcb`; ✔K `jlcpcb-bestand.md` (Gehäuse laut Quelle SOD-123W), `sy8301abc.md` §5 |
| e3 | **`Q2`** ist laut JLCPCB-Bestand das **MMBF170 von Shikues (C130879)**, **nicht** das von Diodes, dessen Datenblatt der Schaltplan als Datenblatt führt (`ds30104`, ✔R `v2.xml`). Die Schwellenspannung weicht ab: **Shikues max. 2,5 V** (✔K `mmbf170.md`, Shikues-Datenblatt REV.08, S. 2) gegen **Diodes max. 3,0 V** (✔K, DS30104 Rev. 16-2, S. 2). Bei einem 3,3-V-Gate gilt nur das Shikues-Blatt; mit dem Diodes-Teil wäre das Durchschalten nicht garantiert | ✔K `mmbf170.md` §1 |
| e4 | **`R26`** (22,1 kΩ, `ILM` von `U9`) ist nicht Basic und hat den niedrigsten Bestand der Widerstände (2'095). 22 kΩ (C17560, Basic) ändert den Strombegrenzungsstrom um +0,6 % | ✔K `jlcpcb-bestand.md`; ✔K `tps2121.md` §10 (Rechnung ○) |
| e5 | **`C29`** ist 100 nF, nicht 22 µF (ein früherer Auftrag nannte alle drei Kondensatoren hinter `L2` mit 22 µF); hinter `L2` hängen `C27`, `C28` (je 22 µF) und `C29` (100 nF) | ✔K `sy8301abc.md` §2, ✔R `v2.xml` |

---

## 2. Das Modul ESP32-S3-WROOM-1 und seine Pinregeln

Alles in diesem Abschnitt ist **✔K** aus `hardware/komponenten/esp32s3-wroom-1.md` (Datenblatt-Auswertung vom 17.10.2026), mit den dort
genannten Seiten (DS-M = Modul-Datenblatt v1.8, DS-C = Chip-Datenblatt v2.2, HDG = Hardware Design Guidelines, TRM = Technical Reference
Manual v1.8, DK = DevKitC-1-v1.1-Anleitung). Die Regeln, die die Prüfung H3 durchsetzt, stehen in
`tools/checks/hardware-pinregeln.json`.

### W-6. Das Modul

- **Zielmodul ESP32-S3-WROOM-1-N8**: 8 MB Quad-SPI-Flash, kein PSRAM, −40 … 85 °C, PCB-Antenne, **alle 36 GPIOs am Modul frei** (DS-M S. 3, S. 12).
  41 Pads (40 Rand-Pads und EPAD/GND), **18,0 × 25,5 × 3,1 mm**, Rastermass 1,27 mm (DS-M S. 3, S. 11, S. 45).
- **Zweitwahl N8R2** (+ 2 MB Quad-PSRAM): pinkompatibel, alle GPIOs frei; der **Chip ESP32-S3R2 läuft aus** (DS-C S. 1, S. 13, Nachfolger S3RH2). Das ist ein
  Beschaffungsrisiko für N8R2, nicht für N8.
- **Octal-PSRAM-Module (N8R8, N16R8)** sperren **GPIO35, 36, 37** und sind nur −40 … 65 °C (DS-M S. 3, S. 12, Fussnote b). Das **heute verkaufte DevKitC-1 ist
  N8R8 oder N32R16V**; N8 und N8R2 sind dort als EOL geführt (DK, „Ordering Information“).
- **Bei JLCPCB nur Extended, kein Basic:** C2913198 (N8), Lager 416 am 2026-10-17 (siehe W-17).

### W-7. Pinregeln des Moduls — alle auf einen Blick

| Pin (Pad) | Regel | Grund | Quelle |
|---|---|---|---|
| **GPIO0** (Pad 27) | Nur an `BOOT`. Pull-up gegen 3,3 V, Taster nach GND; **keine grossen Kondensatoren** | Strapping: 1 = SPI-Boot, 0 (mit GPIO46 = 0) = Download. Ein Kondensator kann den Chip in den Download-Modus ziehen | DS-M Tab. 4-1/4-3, S. 13–14; HDG S. 15 |
| **GPIO3** (Pad 15) | Unbeschaltet lassen | Strapping (JTAG-Quelle), **schwebend, kein interner Pull**; wirkt nur, wenn `EFUSE_STRAP_JTAG_SEL` gebrannt ist | DS-M Tab. 4-5, S. 15–16 |
| **GPIO45** (Pad 26) | Unbeschaltet; **beim Einschalten nie High** | Strapping (`VDD_SPI`): High speist beim N8 den Flash mit 1,8 V statt 3,3 V | DS-M Tab. 4-4, S. 15; S. 39 |
| **GPIO46** (Pad 16) | Unbeschaltet, ohne starken Pull-up beim Start | Strapping; für UART/USB-Download muss GPIO46 niedrig sein | DS-M S. 13–15 |
| **GPIO19/20** (Pad 13/14) | Nur USB (`USB_DN`/`USB_DP`); nie umkonfigurieren | USB-Serial/JTAG. Wird USB abgeschaltet, verschwindet das Gerät; der automatische Download entfällt, nur BOOT + EN helfen | DS-M S. 11; HDG S. 36 |
| **GPIO43/44** (Pad 37/36) | Nur UART0 (`TX_UART`/`RX_UART`) | UART0 = Konsole; im Reset WPU + IE | DS-M S. 12; DS-C S. 17 |
| **GPIO35, 36, 37** (Pad 28–30) | Offen lassen | Flash/PSRAM der R8-Varianten; am heutigen DevKit „nicht für externe Nutzung“ | DS-M S. 12 Fussn. b; DK |
| **GPIO38** | Offen lassen | RGB-LED des DevKitC-1 v1.1 | DK |
| **GPIO48** | Offen lassen | RGB-LED des DevKitC-1 v1.0; bei Octal-1,8-V-Varianten (N16R16VA, N32R16V) arbeiten GPIO47/48 mit **1,8 V** | DK; DS-M S. 12 Fussn. c |
| **GPIO47** | Möglichst nicht | 1,8 V bei den eben genannten Varianten. **Weder `hardware-pinregeln.json` noch Spec-Entscheid E2 sperren GPIO47**; nur das Blatt rät ab | DS-M S. 12 Fussn. c |
| **EN** (Pad 3) | Nur an `EN`; **nie schweben**; 10 kΩ nach 3V3 und 1 µF nach GND, Taster nach GND | RC-Verzögerung 10 ms (EN erreicht 0,75 · 3,3 V nach rund 13,9 ms, ○ gerechnet im Blatt) | DS-M S. 11, S. 41; HDG S. 8 |
| **ADC (LDR)** | Nur an **ADC1** (GPIO1–10), **nicht GPIO3** | ADC2 (GPIO11–20) kollidiert mit WLAN | HDG Tab. 11, S. 18; TRM §39.3.8, S. 1474–1475 |
| **Alle Pins** | Höchstens **3,6 V**; kein 5-V-Netz an einem Modulpin | VIH max. = VDD + 0,3 V. **Es gibt keine Herstellerangabe „5 V tolerant“ und keinen Grenzwert je Einzelpin** (gesucht, nicht gefunden); „nicht 5-V-tolerant“ ist eine Ableitung | DS-M Tab. 6-3, S. 27; DS-C S. 65 |

H3 prüft die Zeilen der Tabelle für `GPIO0`, `GPIO3`, `GPIO35`–`38`, `GPIO45`, `GPIO46`, `GPIO48`, `EN`, USB, UART0, 3V3, GND, die sechs 5-V-Netze und den LDR an ADC1.
**H3 prüft nicht**, welches Signal an welchem GPIO liegt (`"pinbelegung": {}` ist leer, siehe OP-09), und nicht GPIO47.

### W-8. Zustand der GPIOs beim Einschalten und Reset

- Ohne Eintrag in der Tabelle „At Reset“ ist ein Pin **hochohmig** (DS-C Tab. 2-1, S. 16–17; TRM S. 535). Pull-ups im Reset haben GPIO0 (WPU), GPIO43/44 (WPU), GPIO39 (WPU, solange
  `EFUSE_DIS_PAD_JTAG = 0`); Pull-downs GPIO45, GPIO46.
- **Einschaltstörungen** (DS-C Tab. 2-2, S. 18–19; HDG Tab. 10, S. 17): GPIO1–18 geben **typ. 60 µs Low** aus (GPIO18 zusätzlich High-Glitch); **GPIO19 und GPIO20** zwei High-Glitches
  zu je rund 60 µs, **Gesamtdauer 3,2 ms bzw. 2 ms**; kein Glitch bei GPIO0, 21, 35–48 und UART0.
- **Folgerung des Blatts für ein Gate, das beim Booten nicht schalten darf:** externer Pull auf den Ausschaltpegel; ein P-Kanal-Gate an GPIO1–18 wäre 60 µs eingeschaltet, ein N-Kanal-Gate mit Pull-down
  nicht; **GPIO21** ist der sauberste Kandidat (kein Strapping, kein Glitch, hochohmig im Reset); GPIO19/20 sind für Gates tabu.
- **Für die LED-Versorgung** (Gate von Q2, N-Kanal, `R20` 100 kΩ nach Masse, V3 an GPIO6) heisst das: Der Pin ist im Reset hochohmig, `R20` hält das Gate unten, ein 60-µs-Low-Glitch ist harmlos.
  Das ist eine **Zusammenlesung zweier Blätter** (○ als Ableitung): Das Blatt `mmbf170.md` fürchtet eine Gate-Spannung von 2,3 V, falls der Pin im Reset einen internen Pull-up von 45 kΩ hätte
  (Teiler 45 kΩ / `R20`); laut `esp32s3-wroom-1.md` hat GPIO6 im Reset **keinen** Pull-up. Messbar ist es erst am Gerät (AK9 der Spec).

### W-9. USB, Konsole, Boot, Antenne, Versorgung

- **USB-Serial/JTAG** an GPIO19 (D−) und GPIO20 (D+): CDC-ACM-Konsole, Flashen mit esptool, JTAG gleichzeitig; **Reset und Download-Modus per DTR/RTS ohne Strapping-Beschaltung** (TRM Tab. 33.3-2 bis 33.4-4, S. 1247–1254).
  Manueller Download-Modus: GPIO0 = 0 und GPIO46 = 0 beim Reset, also GPIO0 nach GND und EN kurz auf Low (DS-M Tab. 4-3, S. 14; HDG S. 35).
- **Der Chip hat keinen VBUS-Pin**, und das TRM beschreibt keine VBUS-Erkennung für den Serial/JTAG-Teil (TRM Kap. 33 gelesen).
- **Vorwiderstände an D+/D−:** HDG empfiehlt Platz für 22/33 Ω und Kondensatoren dicht am Chip; das Referenzschema des Moduls zeigt 0 Ω; das DevKit hat keine. HDG empfiehlt zudem einen **externen Pull-up an D+** für einen
  stabilen Anfangszustand. Differenzielle Leitung 90 Ω ± 10 % (HDG S. 18–19, S. 30). **V3 hat weder Reihenwiderstände noch einen D+-Pull-up** (✔R: Netze `USB_DP`, `USB_DN` = {U11, U2, USB1}, OP-16).
- **Konsole UART0:** HDG empfiehlt 499 Ω in Reihe in `U0TXD` (HDG S. 13); V3 hat keinen (✔R: Netz `TX_UART` = {H6.2, U2.37}, OP-17). ROM-Meldungen laufen beim Start gleichzeitig auf UART0 und USB-Serial/JTAG (DS-M S. 15).
  In ESP-IDF ist die USB-Zweitkonsole **nur Ausgabe**, erscheint nur, wenn UART0 „nicht verbunden“ ist, und hat keine Eingabe (● IDF Kconfig).
- **Versorgung:** Quelle ≥ 0,5 A (DS-M S. 27); Spitze beim WLAN-Senden **355 mA** (802.11b, Tab. 6-4, S. 28); Referenzschema **22 µF + 0,1 µF** an VDD33 (DS-M Abb. 9-1, S. 41).
  V3 trägt `C30` = 10 µF und `C31` = 100 nF am Modul, `C8` = 22 µF auf dem 3,3-V-Netz (✔R: `expected.json`; OP-18).
- **Antenne:** Fläche 6 mm tief über die Antennenbreite, seitlich mindestens 15 mm frei auf der Seite der Aussparung, im Gehäuse mindestens 15 mm Abstand; Modul **nicht in die Platinenmitte**;
  USB- und UART-Leitungen weit weg (HDG S. 28–31; DS-M Abb. 11-1, S. 45). Ob das im 1:1-Format an der Stelle von `U4` geht, ist **offen** (OP-13).
- **Boot-Modi:** DS-M/DS-C/HDG kennen SPI-Boot und Joint-Download; das **TRM führt einen dritten** (SPI-Download, GPIO0 = 0, GPIO46 = 1, GPIO1 = 1, GPIO2 = 0; TRM Tab. 8.2-1, S. 535). GPIO1/2 sind im Reset hochohmig;
  das ist für **GPIO1 = LDR** zu beachten (OP-26).

### W-10. Der ADC und der LDR-Teiler

**Stand des Blatts.** `komponenten/esp32s3-wroom-1.md` §6 hat der Analyst am 17.10.2026 **nachträglich auf `R22` = 1 kΩ umgestellt**: Die Rechnung für andere Werte (10, 22, 47, 100 kΩ, §6.2) bleibt als Rechnung stehen,
ihre Empfehlungen sind **ausdrücklich zurückgezogen**; massgebend ist §6.3 „LDR-Lesung mit R22 = 1 kΩ (V2-kompatibel)“. Dieser Abschnitt folgt §6.3. Die Spec (E9(a), H9, F15) und `design.md` §4.2 verlangen noch 47 kΩ (siehe W-12).

- ADC1 = GPIO1–10, ADC2 = GPIO11–20; ADC2 wird vom WLAN mitbenutzt, **ADC1 verwenden** (HDG Tab. 11, S. 18; TRM §39.3.8). ✔K
- Dämpfung → nutzbarer Bereich → Gesamtfehler nach Kalibrierung (DS-C Tab. 5-6, S. 66; HDG S. 18): 0 dB 0 … 850 mV ±5 mV; 2,5 dB 0 … 1100 mV ±6 mV; 6 dB 0 … 1600 mV ±10 mV; 12 dB 0 … 2900 mV ±50 mV.
  (Die Zuordnung ATTEN0–3 zu den dB-Werten ist ● aus dem TRM und den IDF-Konstanten.) DNL −4 … +4 LSB, INL −8 … +8 LSB, gemessen mit 100 nF am Eingang und **WLAN abgeschaltet** (DS-C Tab. 5-5, S. 66; **mit WLAN-Betrieb ist die Genauigkeit nicht zugesichert**).
- **Rechnung für `R22` = 1 kΩ** (V = 3,3 V · R22 / (R_LDR + R22); die Eingangsdaten dunkel 1 MΩ / Zimmer 20 kΩ / hell 10 kΩ sind **○ Annahmen**): dunkel **3,3 mV**, Zimmer 157,1 mV, hell 300,0 mV, identisch zu V2. Alle drei liegen bei 0 dB im Bereich
  (hell hat 550 mV Reserve); der Bereich endet bei einem LDR unter etwa 2,9 kΩ.
- **Was die STM32-Firmware liest** (✔K, Quelltext vom Analysten gelesen): eine einzelne Software-Wandlung, 12 Bit, ohne Mittelung, viermal je Sekunde; `ldr.c` klemmt eine **Kopie** des Rohwerts (0 … 4095) auf `ldr_min_value … ldr_max_value`
  und ändert die Helligkeit um höchstens eine Stufe je Messung; Beispielspanne aus `HARDWARE.md` rund 369 Zähler auf 32 Stufen (rund 11,5 Zähler je Stufe).
- **Befund des Blatts:** Der Dunkelwert (3,3 mV) liegt **unter dem Gesamtfehler des S3** (±5 mV), also nicht auflösbar — **aber auch am F411 nicht** (ET max ±5 LSB ≙ ±4,0 mV); `ldr.c` klemmt dort auf `ldr_min_value`. Der absolute Gesamtfehler des S3 liegt im Bereich des Worst Case des F411,
  ist aber doppelt so gross wie dessen typischer Wert.
- **Der grössere Unterschied ist die Referenz:** Der F411 misst **ratiometrisch** (VREF+ = VDDA), der S3 **absolut** gegen eine interne Referenz von 1100 mV (je Chip 1000 … 1200 mV, kalibriert). Die Schienentoleranz geht beim S3 voll in den Zähler ein
  (bei ±2 % rund ±4 … ±7 Zähler; typische Summe ±10 Zähler bei „Zimmer“, ±14 bei „hell“). Die Toleranz der 3,3-V-Schiene von V3 ist nicht beziffert.
- **Funktionale Gleichwertigkeit (Urteil des Blatts, §6.3 (5)):** „Ja, für den Betrieb der Helligkeitsautomatik; nein im strengen Sinn zählergleicher Messwerte.“ Nötig sind (ohne Hardwareänderung): 0 dB, eFuse-Kalibrierung, Mittelung über mindestens 16 Abtastungen,
  Abbildung **N_STM = V_mV · 4095 / 3300** und Klammerung wie bisher; die gespeicherten Grenzen bleiben dann auf ±1 Stufe gültig, eine einmalige Nachkalibrierung durch den Nutzer stellt zählergleiche Werte her.
- **Quellimpedanz:** In **keiner** der vier Unterlagen zum S3 angegeben. Der F411 läuft in V2 mit 3 Zyklen Abtastzeit und 1 kΩ Quelle **ausserhalb der Herstellerregel** (Grenze 1472 Ω für RAIN + RADC, Gl. 1, S. 116); das ist V2-Verhalten, das die Kalibrierwerte des Nutzers enthalten.
- **`C32` (100 nF am ADC-Pin) bei `R22` = 1 kΩ:** keine Wirkung auf den Gleichwert; Tiefpass mit τ rund 100 µs (Grenzfrequenz 1,59 … 1,75 kHz), die Messung wird nicht langsamer; Netzbrummen (100 Hz) wird **nicht** gefiltert. V2 hat keinen solchen Kondensator; er „ändert die Funktion nicht“,
  ob er unter „100 % kompatibel“ fällt, **entscheidet der Nutzer** (so das Blatt, §6.3 (4); OP-03).
- **Minimale Abweichungen, falls je nötig** (Tabelle des Blatts, §6.3, Entscheid beim Nutzer): Schiene über einen zweiten ADC1-Pin messen (Teiler 2 × 100 kΩ, 0,1 µF; ein zusätzlicher Pin, nicht V2) oder `R22` = 4,7 kΩ mit 6 dB (ändert V2-Spannungen, **nicht freigegeben**).
- **GL5528 auf dem LED-Board:** zwei Hersteller-Datenblätter, die voneinander abweichen (Senba: 10 … 20 kΩ bei 10 lux, γ = 0,6, 100 mW; JCHL: 8 … 20 kΩ, γ = 0,65, 90 mW); **welches Teil verbaut ist, ist unbekannt**.
  Dunkelwiderstand ≥ 1 MΩ (ohne Obergrenze), Ansprechzeit 30 ms, Temperaturgang −15 % … +10 % (−30 … +35 °C), Alterung im Datenblatt nicht angegeben (✔K `gl5528.md` §2, §5, §7).
- **Nicht angegeben** (OP-26): Rauschen des S3-ADC mit und ohne WLAN, Nichtlinearität unter 5 mV, Abtastkapazität, Toleranz der 3,3-V-Schiene.

---

## 3. Die LED-Board-Schnittstelle (muss 100 % kompatibel bleiben)

### W-11. Der Pfad zwischen Controller und Stecker `H7`

✔R, Netzlistenexport `hardware/esp32s3/v2/v2.xml`:

| Funktion | Weg |
|---|---|
| **LED-Datenleitung** | Controller-Pin (`WS2812_BUS`) → `U3` Pin 2 (A; SN74AHCT1G125, OE Pin 1 an GND) → `U3` Pin 4 (Y) → `R11` 330 Ω → `TO_SK6812_IN` → `H7` Pin 4 und `H4` Pin 2; `R31` 100 kΩ hält `TO_SK6812_IN` nach Masse |
| **Versorgung von `U3`** | `Net-(U3-VCC)` ← `R13` (0 Ω) ← `TO_VCC_RGB_5V` (also das **geschaltete** 5 V); `R17` (0 Ω, bestückungsfrei) würde ständig 5 V liefern |
| **Schalten der LED-Versorgung** | Controller-Pin (`WS2812_POWER`) → `R19` 82 Ω → Gate von `Q2` (MMBF170); `R20` 100 kΩ Gate → GND; `Q2` Drain (`Net-(Q2-D)`) → `R21` 3,3 kΩ → `VCC_5V` und → `R23` 2,2 kΩ → Gate von `Q1` (`Net-(C10-Pad2)`); `C10` 100 nF Gate von `Q1` ↔ `VCC_5V`; **`Q1` (Si7149DP)**: Source an `VCC_5V`, Drain an `TO_VCC_RGB_5V`; `JP2` (offene Lötbrücke) überbrückt `VCC_5V` und `TO_VCC_RGB_5V`; `C6` 100 nF an `TO_VCC_RGB_5V` |
| **LDR** | Netz `LDR` = {`H7.5`, `H5.2`, `R22.2`, Controller-Pin}; `R22` 1 kΩ nach Masse; der LDR sitzt auf dem LED-Board und hängt an 3,3 V (`H7` Pin 3) |
| **Stecker `H7`** | Pin 1/2 `TO_VCC_RGB_5V`, Pin 3 `VCC_3.3V`, Pin 4 `TO_SK6812_IN`, Pin 5 `LDR`, Pin 6/7 GND |

Die Prüfung **H8** (`tools/checks/hardware.py`) vergleicht den ganzen Pfad: Wert und Pinanschluss von `U3`, `R11`, `R13`, `R17`, `R31`, `JP2`, `Q1`, `Q2`, `R19`–`R23`, `C10`, `C6`, `H4`, `H5`, `H7` und
`R22` gegen V2 sowie die Mitglieder von zehn Schnittstellennetzen (ohne den Controller selbst). Fehlt ein Teil oder weicht ein Wert/Pin ab, ist das **KRITISCH**; ein **zusätzliches** Teil an einem Schnittstellennetz ist **HOCH**.
Eine Abweichung braucht einen Eintrag **mit Grund** in `tools/checks/hardware-led-schnittstelle.ausnahmen` — und die Freigabe des Nutzers. Stand 17.10.2026: **keine Ausnahme eingetragen**.

### W-12. `R22` bleibt 1 kΩ (Korrektur)

**Bis 17.10.2026 stand** in `specs/esp32s3-port/requirements.md` (E9(a), H9, F15), in `design.md` (§2, §4.2) und in `hardware/komponenten/jlcpcb-bestand.md` („Neue Werte für den Umbau“: `R22` 47 kΩ, C17713) sowie im Skill
`jlcpcb-bauteile` („47 kΩ für `R22`“) und — als Empfehlung 10 kΩ mit 6 dB — im Komponentenblatt `esp32s3-wroom-1.md` §6.2/§12 (das der Analyst am 17.10.2026 selbst zurückgezogen hat), `R22` wechsle von 1 kΩ auf rund 47 kΩ. **Das stimmt nicht mehr, weil** der Nutzer am 17.10.2026 verlangt hat, dass LED-Ansteuerung, Power-Schalten und LDR-Lesung 100 % pin- und
funktionskompatibel bleiben; `R22` liegt im LDR-Pfad und bleibt damit **1 kΩ**, solange der Nutzer keine Abweichung freigibt. Der Generator (`hardware/esp32s3/gen.py`) setzt `R22` = 1 kΩ, und H8 schlägt bei einem anderen Wert an
(Gegenprobe P16). Die genannten Dokumente sind **nicht nachgezogen** (Übergabeliste im Bericht).

Offen bleibt die Folge für die Messung (siehe W-10 und OP-02): Mit 1 kΩ liegt das Dunkel (3,3 mV) unter dem ADC-Fehler des S3 — beim F411 von V2 ebenso. Das Blatt hält die Helligkeitsautomatik für funktional gleichwertig; die Zähler sind es nur nach Abbildung und Nachkalibrierung.
Das ist zu **entscheiden** (Nutzer), nicht zu **beheben**.

### W-13. Was sonst an der Schnittstelle hängt (Quellen: Blätter)

- **`Q1`/`Q3` (Si7149DP-T1-GE3)**: P-Kanal 30 V, spezifiziert nur für VGS ≤ −4,5 V (RDS(on) 9,4 mΩ max bei −4,5 V, 5,2 mΩ bei −10 V); im Betrieb VGS ≈ −5 V (rund 85 mW bei 3 A). Wer das Gate ansteuert, muss VGS ≤ −4,5 V halten.
  Einschalten über `R23` × `C10` (τ ≈ 230 µs; −2,5 V nach ≈ 160 µs, −4,5 V nach ≈ 530 µs, ○ gerechnet): weicher Anlauf. Das Datenblatt ist von 2008 (Rev. A, 03.11.2008); Nachfolger Si7149ADP. ✔K `si7149dp.md` §3–§6, §8.
- **`Q2`**: schaltet nur 1,5 mA (`VCC_5V` / `R21`); Verlust 9 µW. Aus-Zustand: `R20` hält das Gate unten, `R21` + `R23` ziehen das Gate von `Q1` hoch (VGS ≈ 0). ✔K `mmbf170.md` §3.
- **`U3` (SN74AHCT1G125DBVR)**: VIH min. 2 V bei VCC 4,5 … 5,5 V gegen VOH min. 2,64 V des S3 (0,8 · VDD bei 3,3 V): **rund 0,64 V Reserve** (✔K `sn74ahct1g125.md` §5.1; DS TI SCLS378P S. 4, ESP-DS S. 65).
  Eingang bei VCC = 0 V: Leckstrom ≤ ±1 µA über den Temperaturbereich (S. 5); Ausgang bei VCC = 0 V **nicht spezifiziert**. `OE` fest an GND ist **nicht** die von TI empfohlene Beschaltung für das Ein- und Ausschalten (S. 8);
  für 1:1 unkritisch, solange `R31` den Pegel hält. Nicht gelesen: das SK6812-Datenblatt (VIH bei 5 V, Pegel 3,8 V) (OP-27).

---

## 4. Versorgung, Wandler, USB

**W-14. `U9` TPS2121 (Prioritäts-Power-Mux).** USB (`IN1`, `PR1`) hat **Vorrang**; die externe Einspeisung (`IN2`) speist nur, solange USB fehlt oder unter etwa 2,55 V (fallend) fällt. Strombegrenzung über `R26` (22,1 kΩ): rund 4,5 A
(4,0 … 5,0 A Streuung); Softstart `C24` 100 nF, 780 V/s; Umschalten bis 100 µs. `CP2`, `OV1`, `OV2` an GND wie vom Datenblatt für „nicht benötigt“ vorgegeben, `ST` offen (entgegen der Empfehlung „GND“, ohne elektrische Folge).
**`PR1` liegt direkt an `USB_5V_PROT`** (absolut 6 V, empfohlen 5,5 V); die Klemmspannung von `D4` ist nicht gelesen (OP-15). ✔K `tps2121.md` §3–§7 (TI SLVSEA3F, Rev. F, 08/2020).

**W-15. `U10` SY8301ABC (5 V → 3,3 V) mit `D2`, `L2`.** Nur für **1 A** spezifiziert (S. 1), Spitzen-Strombegrenzung 1,6 … 2,5 A (S. 4). Bei 0,6 A Gesamtlast sind 60 % der 1 A belegt; die Aussage „mindestens 250 mA Reserve“
(Spec, F1) beruht auf **ungelesenen** Werten der Restschaltung (○, Blatt §4.3). Das Modul braucht eine Quelle von ≥ 0,5 A; die Spitze beim WLAN-Senden ist 355 mA für das **Modul** (DS-M Tab. 6-4) bzw. 340 mA für den **Chip** (DS-C Tab. 5-7) —
zwei Blätter, zwei Zahlen, kein Widerspruch. **Kritischer als der Strom ist die Spannungsreserve bei knapper 5-V-Schiene:** `FB` liegt hinter der Diode `D2` (Gleichstromregelung korrekt, der Spannungsfall der Diode wird herausgeregelt),
aber das Datenblatt nennt weder Dropout noch maximalen Tastgrad. Aus Mindest-Ausschaltzeit (100 ns) und Schaltfrequenz folgt ein Tastgrad von höchstens 0,76 … 0,84; bei 0,6 A und VIN = 4,5 V rechnet das Blatt hinter der Diode **3,18 V (1,6 MHz),
3,00 V (2,0 MHz) oder 2,82 V (2,4 MHz)** (○ Rechnung, konstante Frequenz; **ob der Wandler am Tastgradanschlag die Frequenz senkt, steht nicht im Datenblatt**). Messen am Prototyp bei 4,5 / 4,75 / 5,0 V Eingang und 0,6 A.
Das Datenblatt widerspricht sich bei der Mindest-Eingangsspannung (4 V auf S. 8, 4,5 V auf S. 1/3/4; das Blatt nimmt 4,5 V). `F1` (PTC 1,5 A, USB-Pfad) und `F2` (3 A, extern) sind kleiner als 3 A LED + 0,6 A (nur aus den Bezeichnungen gelesen, ○).
`L2` (Cjiang, Rev. 7.0): Isat 2,0 A (vorsichtig gelesen) liegt über dem Spitzenstrom bei 1 A (≈ 1,06 A), aber **unter** der Strombegrenzung des Wandlers; Rdc 160 mΩ liegt über der „wünschenswerten“ 50 mΩ. ✔K `sy8301abc.md` (Silergy Rev. 0.9A, 08.06.2020) §2, §4, §6, §7.

**W-16. USB-C-Buchse und ESD.**
- **V2 trägt eine 6-polige Buchse ohne Datenkontakte** (`USB1`, C456012, Footprint `TYPE-C-SMD_TYPE-C-6P`; ✔R: `v2.xml`, Footprint-Auslesen aus `V2.kicad_pcb`). Wer sie 1:1 mitnimmt, hat keinen USB-Datenweg.
  Gewählt für V3: **HRO TYPE-C-31-M-12** (C165948, 16 Anschlussnamen, 12 Lötflächen, 8,94 × 7,35 × 6,28 mm laut Zeichnung, 5 A, 20 V) (✔K `usb-c-16p.md` §1).
  Die **Aussenmasse der 6-poligen Buchse** sind nirgends belegt; ob die Gehäuseöffnung dieselbe sein kann, ist offen (OP-11).
- Beschaltung (✔K, SPEC = USB Type-C Release 2.0, 08/2019): `A6` und `B6` → D+, `A7` und `B7` → D− (**B-Seite vertauscht, nicht der Reihe nach verbinden**), Stichlänge ≤ 3,5 mm; `CC1`/`CC2` je **eigener** Rd 5,1 kΩ nach GND,
  nie zusammenlegen; `SBU1`/`SBU2` offen; alle vier VBUS- und alle vier GND-Pins verbinden; Schirm an GND. Ohne Auswertung der CC-Bänder darf die Senke **500 mA** ziehen.
- **ESD `U11` USBLC6-2SC6** (C7519, ST Doc 11265 Rev 5, Okt. 2011): D+/D− laufen **durch** den Baustein (Pin 1→6, 3→4), VBUS an Pin 5, GND an Pin 2, so nah an der Buchse wie möglich (10 mm Bahn je Leitung lassen die Klemmspannung von rund 31 V auf über 300 V steigen).
  Der Baustein schützt **nicht** `CC1`/`CC2`; er klemmt ab 5,25 V VBUS (USB-PD > 5 V wäre damit unverträglich). Kontakt-ESD: 15 kV (Tab. 1) gegen 8 kV (Merkmalsliste) — die Blätter nehmen 8 kV.
  ✔K `usb-c-16p.md`, `usblc6-2sc6.md`.

---

## 5. Fertigung bei JLCPCB

**W-17. Klassen und Bestände** (● Teilebibliothek JLCPCB, abgerufen am **2026-10-17**; die Bestände ändern sich; H4 warnt, wenn die Prüfung älter als 14 Tage ist):

| Teil | LCSC | Klasse | Lager | Bemerkung |
|---|---|---|---|---|
| ESP32-S3-WROOM-1-N8 | C2913198 | **Extended** | 416 | nur „Standard Only“ (nicht Economic), MSL 3; kein Basic-Funkmodul gefunden |
| ESP32-S3-WROOM-1-N8R2 | C2913204 | Extended | 712 | Chip ESP32-S3R2 läuft aus |
| USB-C-Buchse 16P (HRO) | C165948 | Extended | 367'528 | Schirmlaschen: Bestückungsweg nicht geprüft |
| USBLC6-2SC6 (ST) | C7519 | Extended | 36'415 | Nachbauten haben andere Kennwerte |
| TPS2121RUXR | C485916 | Extended | 27'942 | keine Basic-Alternative |
| SY8301ABC | C2927130 | Extended | 20'824 | keine Basic-Alternative |
| SI7149DP-T1-GE3 (`Q1`, `Q3`) | C553982 | Extended | 10'088 | Basic-P-FETs sind SOT-23, bei 3 A zu heiss |
| MMBF170 Shikues (`Q2`) | C130879 | Extended | 1'355 | LCSC selbst zeigt Lager 0 |
| SN74AHCT1G125DBVR (`U3`) | C7484 | Extended | 51'402 | kein Basic-Pegelwandler |
| DS3231MZ+TRL | C107410 | Extended | 2'885 | |
| AT24C32M/TR (HGSEMI) | C2688466 | Extended | 13'120 | Nachbau |
| DS18B20 (HXY, TO-92) | C5451632 | Extended | 3'465 | Nachbau; THT |
| Batteriehalter CR2032 (`U7`) | C18212226 | Extended | 629 | niedrigster Bestand; Footprint-Passung nicht geprüft |

Die Zeilen der übrigen Bauteile (Widerstände, Kondensatoren, Taster, Stiftleisten, Sicherungen, TVS) stehen in `hardware/komponenten/jlcpcb-bestand.md`.
**Basic sind nur die gewöhnlichen Widerstände und Kondensatoren** (0 Ω, 100 Ω, 330 Ω, 510 Ω, 1 k, 3,3 k, 4,7 k, 5,1 k, 10 k, 100 k; 10 nF, 100 nF, 1 µF, 10 µF, 22 µF); **nicht Basic** sind 82 Ω, 22,1 kΩ, 31,6/32 kΩ und 47 µF 10 V.
Die Zahl der Basic- und Extended-Zeilen einer **V3-Stückliste** gibt es erst mit der Stückliste (OP-08). Die Zahlen des Bestandsblatts (42 Zeilen, 73 Bauteile, 15 Basic, 27 Extended) gelten für die **V2-Stückliste abzüglich der entfallenen Teile**
und enthalten weder die neuen V3-Teile noch `R23`/`C10`.

**W-18. Bestückbarkeit.** Ob JLCPCB ein Teil per SMT oder per THT/von Hand bestückt, liefert die Quelle **nicht**. Nicht geprüft: Stiftleisten `H4`–`H7`, DS18B20 (TO-92), Schirmlaschen der USB-Buchse. SMD-Gehäuse sind „SMD“ im Sinn des
Gehäusefelds, mehr nicht.

**W-19. Basic-Alternativen** sind in den Blättern **mit Nummer** genannt (z. B. Q2 → AO3400A C20917 oder 2N7002 C8545; R19 → 100 Ω C17408; R26 → 22 kΩ C17560; R29 → 30 kΩ C17621 oder 33 kΩ C17633; D2/D3 → SS14 C2480/SS34 C8678). Diese Teile sind
**Empfehlungen der Analysten oder Hinweise**, keine Entscheide, und mehrere liegen **im Schnittstellenpfad zum LED-Board** (`Q2`, `R19`): Dort wären sie Abweichungen von V2 und bräuchten die Freigabe des Nutzers (OP-04).

---

## 6. Sensoren, RTC, EEPROM (unverändert übernommen)

- **DS3231MZ+TRL (`U5`):** Adresse 0x68; **MZ ≠ DS3231** (MEMS ±5 ppm, `INT/SQW` nur 1 Hz). `32KHZ` ist nach dem Einschalten **aktiv** (die Firmware schreibt nur Steuerregister 0Eh, nicht 0Fh); beide Ausgänge dürfen offen bleiben. Die Pull-ups an `32KHZ` und
  `SQW` (`RN2` Pin 5 und 6) kosten gerechnet rund 0,7 mA Dauerstrom; **Abhilfe ○:** Pull-ups weglassen. ✔K `ds3231mz.md` §5, §7, §8 (Datenblatt Rev 7, 3/15).
- **AT24C32M/TR (`U6`, HGSEMI-Nachbau):** 7-Bit-Adresse **0x57** (E0…E2 über `RN1` auf 3,3 V, `WCB` an GND); die **ID-Seite liegt auf 1011 E2E1E0 = 0xBE** (eine Adresssuche 0xA0 … 0xBE träfe den Baustein ein zweites Mal);
  EEPROM-Zyklus 15 ms laut Firmware gegen 5 ms laut Datenblatt. ✔K `at24c32.md` §7, §8.
- **DS18B20 (`U1`, HXY-Nachbau):** Umsetzzeit 400 ms (Maxim 750 ms), nur 1'000 EEPROM-Zyklen (Maxim 50'000), tWR 8 … 12 ms. Ein Zusammenhang mit den Befunden A63, A64, A65 ist **nicht belegt**. ✔K `ds18b20.md` §7.

---

## 7. Die Prüfwerkzeuge und was sie sehen

**W-20.** Die Hardware-Prüfung ist `tools/checks/hardware.py` (Guardrail-Stufe S19), die Gegenproben `tools/checks/hardware-gegenproben.sh` (S20). Acht Prüfungen: H1 ERC, H2 Netzliste gegen Soll, H3 Pinregeln, H4 Stückliste, H5 Komponentenblätter,
H6 V2-Referenz, H7 Format und DRC, H8 LED-Board-Schnittstelle. **Jede nennt, wie viele Dinge sie betrachtet hat**, damit eine Prüfung, die nichts sieht, nicht „OK“ meldet (DIR-014). Der Lauf vom 17.10.2026:
`./tools/checks/hardware-gegenproben.sh` bestand alle Proben (Anzahl nennt der Lauf selbst); `python3 tools/checks/hardware.py` meldete **keinen** KRITISCH-Befund und **drei HOCH-Warnungen** (H4: Stückliste fehlt; H5: Haken ohne Seitenangabe;
H8: zusätzliches Teil `C32` am Netz `LDR`).

**W-20a. Die Grenze von H2.** `hardware/esp32s3/gen.py` schreibt die Soll-Netzliste `expected.json` **im selben Lauf** wie den Schaltplan (✔R: `json.dump(... open(HERE + "/expected.json", "w"))`). H2 beweist deshalb,
dass der Schaltplan dem Generator entspricht; es beweist **nicht**, dass der Generator dem Entwurf des Architekten entspricht. Unabhängig verankert sind nur H3 (Pinregeln aus den Datenblättern), H6 und H8 (gegen den V2-Export) und das Gutachten des `hw-reviewer`, das die Netzliste selbst erzeugen soll.
Ein stilles Auseinanderlaufen von Zeichnung und Absicht fände H2 nicht (OP-09).

**W-21. ERC-Baseline.** `tools/checks/hardware-erc.baseline` führt zwei Meldungsarten als Obergrenze: `pin_to_pin` und `lib_symbol_mismatch`. ✔R: `hardware/esp32s3/erc.json` enthält nur **Warnungen**, keinen Fehler. Die `pin_to_pin`-Meldungen betreffen
Pins vom Typ „Unbestimmt“ (importierte EasyEDA-Symbole), die `lib_symbol_mismatch`-Meldungen die Lötbrücke `JP3`. **Die Begründung, warum das „bewusst“ ist, steht in keiner Datei** (OP-30).

**W-22. Gegenproben-Disziplin.** Jede Prüfung ist erst fertig, wenn sie einmal angeschlagen hat (DIR-014). Die Gegenproben bauen die Verletzung in einer **Kopie** und prüfen auch die Gegenrichtung (unveränderter Entwurf besteht);
Details in [ablaeufe.md](ablaeufe.md).

---

## 8. Was es noch nicht gibt (Stand 17.10.2026)

| Gegenstand | Stand |
|---|---|
| Layout V3 (`*.kicad_pcb`, `.kicad_dru`, `layout/`, `fertigung/`) | nicht vorhanden |
| Stückliste V3 (`hardware/esp32s3/bom.csv`) | nicht vorhanden (H4: HOCH) |
| `hardware/architektur/` (Pinbelegung, Entscheidungsprotokoll, Leistungsbudget) | Verzeichnis **leer** |
| `hardware/review/` (Gutachten) | Verzeichnis **leer**; kein Gutachten, keine Bestellung |
| `specs/multiboard-quelle/` (Spec des `sw-architect`) | zum Zeitpunkt der Erstfassung **nur `requirements.md` als Entwurf**; `design.md` und `tasks.md` fehlen, keine Freigabe |
| Komponentenblätter für die Passivteile, Taster, Stiftleisten, Sicherungen, TVS, LEDs, Batteriehalter, SK6812 | nicht vorhanden |
| Gerät (Nachbau oder DevKit-Testaufbau) | kein Gerät; alle Hardware-Aussagen sind Papier |
| Versionierung | `hardware/` (Schaltplan, Blätter, Werkzeug) ist inzwischen im Git (Commits des Lead am 17.10.2026); `hardware/README.md` und `hardware/dokumentation/` waren bei der Erstfassung **noch ungetrackt** |
