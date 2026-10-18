# Komponentenblatt: ESP32-S3-Modulvergleich — ist das WROOM-1-N8 das richtige Modul?

Stand der Prüfung: **18.10.2026** (JLCPCB-Abfrage 02:28 Uhr). Verfasser: Datenblatt-Analyst. Anlass: Auftrag des Leads, die Wahl
von `U2` (ESP32-S3-WROOM-1-N8, C2913198) gegen alle für uns denkbaren Varianten zu belegen — vor allem wegen des Formats (AE-16:
Antenne, `H6`, `SW1`, Pad 14). Zeichen: ✔ im Dokument gelesen (Seite, Abschnitt, URL), ● Sekundärquelle (genannt), ○ Annahme oder eigene
Rechnung (Formel steht dabei) — ○ ist keine Regel. **Was ich nicht lesen konnte, steht als „nicht gelesen“ da.** Schreibt nur diese Datei;
Skripte und Zwischenstände liegen im Scratchpad, nicht im Repo.

## 0. Kurzfassung

1. **Das WROOM-1-N8 erfüllt die Funktion** (alle 11 Signale, USB, UART0, ADC1, 8 MB Flash, kein PSRAM nötig, 85 °C). Es ist aber **das Modul, das die
   drei bekannten Format-Konflikte verursacht** (Pad 1/40 an der Aussparungskante, `SW1` gegen Pad 14, `R12`, `H6`; `layout-regeln.md` §11).
2. **Das ESP32-S3-MINI-1-N8 (C2913206, Extended, 3'882 Stück)** ist pin- und funktionsgleich auf **GPIO-Ebene** (alle 11 Signale und UART0/USB/EN/BOOT vorhanden;
   Strapping, USB, Strom, Temperatur gleich), aber **15,4 × 20,5 × 2,4 mm** gegen 18,0 × 25,5 × 3,1 mm. In der V2-Aussparung (16,5 × 7,5 mm) liegt seine ganze Antenne
   **innerhalb** der Aussparung. Gerechnet gegen die V2-Festpunkte: **`SW1` 3,15 mm statt 0,0 mm, `R12` 2,4 mm statt 0,3 mm, `H10` 5,4 mm statt 2,55 mm, `H6` 2,1 mm weiter weg**;
   kein neuer Konflikt. **`H6` bleibt unter den 15 mm des HDG** — das löst kein Modul.
3. **Die Software ist modulunabhängig:** GPIO-Nummern, Strapping, USB-Pins und Flash-Modus sind bei WROOM-1-N8 und MINI-1-N8 identisch; das Pin-Mapping am DevKit hängt an GPIO-Nummern,
   nicht am Modul. Die Kompatibilität bleibt **nicht** nur mit dem WROOM-1 gewahrt.
4. **Empfehlung des Analysten:** Wechsel auf **ESP32-S3-MINI-1-N8**, wenn der Nutzer den Aufwand (neues Symbol/Footprint, alle Modulpads neu, `R12`/EN-Lage) freigibt;
   das WROOM-1-N8 bleibt die vertretbare Rückfallebene mit den bekannten Massnahmen. **Antennenleistung ist für keines der beiden belegt** (kein Datenblatt nennt Gewinn oder
   Wirkungsgrad der PCB-Antenne) — entscheiden kann nur die RSSI-Messung (Spec F4). Die Entscheidung trifft der Architekt/Nutzer (AE-16, F6) — **`requirements.md` E1 nennt ausdrücklich das WROOM-1**; ein Wechsel ist eine Änderung der Nutzervorgabe.

## 1. Quellen

| Kürzel | Unterlage | Revision, Datum | Datei / Adresse |
|---|---|---|---|
| **DS-W1** | ESP32-S3-WROOM-1 & WROOM-1U Datasheet | v1.8, 2026-03-02 (Revisionstabelle S. 51) | `datenblaetter/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf`, 53 S. |
| **DS-MI** | ESP32-S3-MINI-1 & MINI-1U Datasheet | **v1.7, 2026-03-02** (S. 51) | `datenblaetter/esp32-s3-mini-1_mini-1u_datasheet_en.pdf` (neu geladen 18.10.2026 von `espressif.com/sites/default/files/documentation/…`), 53 S. |
| **DS-W2** | ESP32-S3-WROOM-2 Datasheet | v1.7 (Titelseite) | `datenblaetter/esp32-s3-wroom-2_datasheet_en.pdf`, 50 S. |
| **DS-PI** | ESP32-S3-PICO-1 Series Datasheet | **v1.2, 2026-07-09** (S. 33) | `datenblaetter/esp32-s3-pico-1_datasheet_en.pdf` (neu geladen), 34 S. |
| **DS-C** | ESP32-S3 Series Datasheet | v2.2, 2026-03-05 | `datenblaetter/esp32-s3_datasheet_en.pdf`, 87 S. |
| **HDG** | ESP32-S3 Hardware Design Guidelines | „Release master“, 12.10.2026; gedruckte Seite = PDF-Seite − 3 | `datenblaetter/esp32-s3_hardware_design_guidelines_en.pdf` |
| **DK-C** | ESP32-S3-DevKitC-1 v1.1 User Guide (HTML) | abgerufen 17./18.10.2026 | `docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html` |
| **DK-M** | ESP32-S3-DevKitM-1 User Guide (HTML) | abgerufen 18.10.2026 (Auszug über ein Zusammenfassungswerkzeug, ●) | `…/esp32-s3-devkitm-1/user_guide.html` |
| **JLC-API** | JLCPCB-Teilebibliothek, Such-Schnittstelle von `jlcpcb.com/parts` (`…/smtGood/selectSmtComponentList`) | 2026-10-18 02:28 | ● Distributor-Datenbank (wie `jlcpcb-bestand.md`) |
| **JLC-PART / LCSC** | `jlcpcb.com/partdetail/…/C2913206`, `…/C2913198`; `lcsc.com/product-detail/C2913206.html`, `C2913198.html` | 2026-10-18 | ● Auszug über ein Zusammenfassungswerkzeug (nicht die Rohseite) |
| **KC-FP / KC-SYM** | KiCad 10.0.2: `footprints/RF_Module.pretty/*.kicad_mod` (Version 20260206), `symbols/RF_Module.kicad_sym` | in `KiCad.app` | per `pcbnew`-Skript gelesen |
| **V2-PCB** | `V2.kicad_pcb` (nur lesend, **Kopie** im Scratchpad) | Mär 2026 | Koordinaten = KiCad − (81,5 / 76,75) |
| **Repo** | `hardware/komponenten/esp32s3-wroom-1.md`, `layout-regeln.md`, `jlcpcb-fertigung.md`, `hardware/architektur/*.md`, `specs/esp32s3-port/*`, `specs/multiboard-quelle/design.md`, `BEFUNDE.md`, `build/**` | 17./18.10.2026 | |

**Nicht gelesen:** das Espressif-Produktauswahlwerkzeug `products.espressif.com` (liefert nur eine Anwendungshülle ohne Daten, 18.10.2026); die RF-Zertifikate
(`espressif.com/en/support/documents/certificates`, nicht abgerufen); der EasyEDA-Footprint der LCSC-Nummern (API antwortet mit einer Fehlerseite).

## 2. Katalog

### 2.1 Chips (Zeile = Gehäuse QFN56 7 × 7 mm) ✔ DS-C Tab. 1-1, S. 13; Titelseite S. 1

| Chip | Flash im Gehäuse | PSRAM im Gehäuse | Umgebung | VDD_SPI | Stand |
|---|---|---|---|---|---|
| ESP32-S3 | — | — | −40 … 105 °C | 3,3 / 1,8 V | aktuell |
| ESP32-S3FN8 | 8 MB Quad | — | −40 … 85 °C | 3,3 V | aktuell |
| ESP32-S3FH4R2 | 4 MB Quad | 2 MB Quad | −40 … 85 °C | 3,3 V | aktuell |
| ESP32-S3RH2 | — | 2 MB Quad | −40 … 105 °C | 3,3 V | aktuell; **Nachfolger von S3R2** |
| ESP32-S3R8 | — | 8 MB Octal | −40 … 65 °C | 3,3 V | aktuell |
| ESP32-S3R16V | — | 16 MB Octal | −40 … 65 °C | 1,8 V | aktuell |
| ESP32-S3R2 | — | 2 MB Quad | −40 … 85 °C | 3,3 V | **EOL** (→ S3RH2) |
| ESP32-S3R8V | — | 8 MB Octal | −40 … 65 °C | 1,8 V | **EOL** |

Bei Octal-PSRAM-Chips (R8, R16V) kann mit eingeschaltetem PSRAM-ECC bis 85 °C gearbeitet werden, der nutzbare PSRAM sinkt um 1/16 ✔ DS-C S. 13, Fussnote 3.
Chip-Revisionen v0.1/v0.2 (FN8 und ESP32-S3 identisch) ✔ ebd.

### 2.2 Module — alle Espressif-Module des S3 laut HDG

HDG S. 36 (PDF 39), Abschnitt „ESP32-S3 Modules“ (Liste „nur Muster und Serienmodule“, Stand Master 12.10.2026) ✔: **MINI-1, MINI-1U, WROOM-1, WROOM-1U, WROOM-2.**
Weitere Neuerscheinungen bis Oktober 2026 habe ich **nicht gefunden** (HDG-Liste; Espressif-Modulseite ●; Websuche ●); der Produktauswähler war nicht lesbar (§1) — „keine gefunden“ ist deshalb keine Aussage „es gibt keine“.
**Kein Datenblatt trägt das Wasserzeichen „NRND“ oder „EOL“** (Wasserzeichen nur, wenn **alle** Varianten des Datenblatts betroffen sind) ✔ DS-W1 S. 49, DS-MI S. 49, DS-W2 S. 1.

| Modul | Chip | Flash / PSRAM (Optionen) | Antenne | Masse mm | Pads | GPIO am Modul | durch Octal belegt | Umgebung | Stand (✔ Quelle) |
|---|---|---|---|---|---|---|---|---|---|
| **WROOM-1** | ESP32-S3 (Flash extern im Modul), S3R2, S3R8, S3R16V | N4, N8, N16, H4 (4 MB, 105 °C) ohne PSRAM · N4R2, N8R2, N16R2 · N4R8, N8R8, N16R8 · N16R16VA | PCB | **18,0 × 25,5 × 3,1** | 41 | **36** | R8/R16VA: **GPIO35–37** | N/R2/H4: 85 (H4 105) °C; **R8/R16VA 65 °C** | in Serie ✔ DS-W1 S. 3, 12 |
| **WROOM-1U** | wie WROOM-1 | wie WROOM-1 | **U.FL-Buchse** (Hirose W.FL / I-PEX MHF III) | 18,0 × 19,2 × 3,2 | 41 | 36 | wie WROOM-1 | wie WROOM-1 | in Serie ✔ DS-W1 S. 3 |
| **WROOM-2** | S3R8V / S3R16V (Octal-Flash **und** -PSRAM) | N32R16V (Serie); N16R8V, N32R8V **EOL** | PCB | 18,0 × 25,5 × 3,1 | 41 (Pads 28–30 = NC) | **33** | IO35–37 **nicht beschaltet**; IO47/IO48 laufen mit **1,8 V** | 65 °C | in Serie, zwei Varianten EOL ✔ DS-W2 S. 3, 10–12 |
| **MINI-1** | **ESP32-S3FN8** (N8), ESP32-S3FH4R2 (N4R2) | **N8** (8 MB Quad, kein PSRAM), N4R2 (4 MB + 2 MB Quad) | PCB | **15,4 × 20,5 × 2,4** | **65** | **39** | keine (nur Quad; bei N4R2 ist IO26 durch den PSRAM belegt) | 85 °C | in Serie ✔ DS-MI S. 3, 10–12 |
| **MINI-1U** | wie MINI-1 | wie MINI-1 | U.FL | 15,4 × 15,4 × 2,4 | 65 | 39 | wie MINI-1 | 85 °C | in Serie ✔ DS-MI S. 3, 42 |
| **PICO-1** (SiP, kein Modul) | ESP32-S3 im LGA56-Gehäuse | N8R2 (85 °C), N8R8 (65 °C) — **PSRAM immer dabei** | **keine im Gehäuse**; Antenne und Anpassung auf der Platine (Blockbild S. 3, ○ Auslegung der Zeichnung) | 7 × 7 (LGA56) | 56 | nicht gelesen | N8R8: Octal | siehe links | in Serie ✔ DS-PI S. 1, 9 (v1.2 vom 2026-07-09) |
| nacktes ESP32-S3 (QFN56) | siehe 2.1 | extern | extern, **eigenes HF-Layout** | 7 × 7 | 56 | 45 (DS-C Titelseite) | — | — | für uns **ausgeschieden**: HF-Entwurf und eigene Zulassung, 1:1-Format nicht zu halten (○) |

Welche Pins die Octal-Varianten sperren: Octal-PSRAM belegt **IO35, IO36, IO37** ✔ DS-W1 S. 12, Fussnote b; die Flash-Pins GPIO26–32 führt keines der Module heraus (WROOM) bzw. nur IO26 (MINI, **nur** bei N8 frei) ✔ DS-MI S. 12, Fussnote b.
Bei **R16V**-Chips sind IO47/IO48 auf 1,8 V gesetzt ✔ DS-W1 S. 12, Fussnote c; DS-W2 S. 12.

**Zertifizierungen:** WROOM-1/-1U und MINI-1/-1U nennen beide „RF certification: See certificates; Green certification: RoHS/REACH“ ✔ DS-W1 S. 2, DS-MI S. 2. Die Zertifikate selbst: **nicht gelesen**. Die -U-Module sind für die Zulassung mit einer bestimmten Antenne (Gewinn ≤ 2,33 dBi) geprüft; eine andere Antenne kann zusätzliche Prüfungen verlangen ✔ DS-W1 und DS-MI, Abschnitt 10.2.

### 2.3 Geräte, die uns betreffen (Entwicklungsplatinen)

- **DevKitC-1 v1.1** trägt WROOM-1 / WROOM-1U / WROOM-2. **Heute in Serie: N8R8, 1U-N8R8, N32R16V; als „EOL“ geführt: DevKitC-1-N8, -N8R2, 1U-N8, 1U-N8R2** ✔ DK-C „Ordering Information“ (abgerufen 17./18.10.2026, ● Auszug). Das ist die Kitvariante, nicht das Modul: Die Module N8 und N8R2 stehen in DS-W1 v1.8 ohne EOL-Vermerk.
- **DevKitM-1** trägt MINI-1 / MINI-1U (8 MB Flash), RGB-LED an **GPIO48**; Header J1 = GPIO0–18, J3 = GPIO46, 45, 44, 43, 42–39, 38–33, 26, 21, 20, 19, 48, 47 ● DK-M (Auszug). Alle elf Signal-GPIOs liegen auf J1; ob die Seite zwei Pins mit RGB-LED/USB-Brücke besonders belegt, steht dort nicht vollständig im Auszug (nicht gelesen).

## 3. Anforderungen unserer Anwendung und Erfüllung je Kandidat

### 3.1 Die Anforderungen (Quelle: `pinbelegung.md`, `requirements.md` E2/E3/E9, `entscheidungen.md`)

| Nr. | Anforderung | Beleg |
|---|---|---|
| A1 | 11 Signale an **GPIO1, 2, 5, 6, 7, 8, 9, 15, 16, 17, 18** | ✔ `pinbelegung.md` §2 |
| A2 | USB D−/D+ an **GPIO19/20**, UART0 an **GPIO43/44**, **EN**, **GPIO0** (BOOT) | ✔ `pinbelegung.md` §3 |
| A3 | LDR an **ADC1** (GPIO1) | ✔ `pinbelegung.md` §2, DS-W1 / DS-MI Tab. 3-1 |
| A4 | nur GPIOs, die das DevKitC-1 herausführt; **nicht** 0, 3, 45, 46, 19/20, 43/44, 35–37, 38, 48 für Signale | ✔ `requirements.md` E2 |
| A5 | **Header-Pins am DevKit:** 11 Signale + GPIO0 + GPIO19/20 + GPIO43/44 + EN = **17** (plus 3V3, GND); alle liegen auf J1/J3 | ✔ `esp32s3-wroom-1.md` §3.2 |
| A6 | Flash für zwei OTA-Plätze + LittleFS (siehe 3.2), **8 MB** angesetzt | ✔ `esp32s3-port/tasks.md` Task 1.1 |
| A7 | WLAN mit eingebauter Antenne, **Format 1:1** (145 × 39 mm, Aussparung 16,5 × 7,5 mm, `H6`, `SW1`, `H10` fest) | ✔ `layout-regeln.md` §2, §17; AE-16 |
| A8 | −40 … 85 °C genügt (Innenraum) | ○ (keine Vorgabe in den Specs; Innenraumuhr) |
| A9 | Bestückung bei JLCPCB, Standard-Katalog bevorzugt | ✔ Vorgabe vom 17.10.2026 |

### 3.2 Flash-Bedarf (Frage: realistische Grösse)

Gemessene Eingänge (Repo, 18.10.2026):

- **ESP8266-Teil:** Programm **531'552 Byte** (Build E4, `BEFUNDE.md` L457), aktueller Stand `build/esp8266/ESP-WordClock-4M.bin` **540'720 Byte** ✔ Datei; Flash des ESP8266 heute **4 MB mit 1 MB Dateisystem** (`eesz=4M1M`, `Makefile:11`) ✔.
- **STM-Teil** (läuft im S3-Abbild mit, `esp32s3-port` E11 und F13): F411-Abbild **.text 64'264 Byte**, .data 312, .bss 11'508 ✔ `arm-none-eabi-size` auf `build/stm-rgbw-12h/wc12h-stm32f411ce-25-sk6812-rgbw`.
- **PWA im Dateisystem:** 13 Dateien, alle `.gz`, zusammen **262'143 Byte** (`app.js.gz` 172'027) ✔ `ESP8266/ESP-uclock/data/app`, Weissliste `APP_INSTALL_ASSETS` in `http.cpp`.
- **Standard-Partitionstabellen von Arduino-ESP32** (●, `github.com/espressif/arduino-esp32`, `tools/partitions/default.csv` und `default_8MB.csv`, Master am 18.10.2026): **4 MB** — zwei App-Plätze zu je **0x140000 = 1'310'720 Byte**, Dateisystem 0x160000 = 1'441'792; **8 MB** — zwei App-Plätze zu je **0x330000 = 3'342'336**, Dateisystem 0x180000 = 1'572'864.

Folgerung:

- Ein S3-Abbild gibt es noch nicht; seine Grösse ist **nicht gemessen**. Der ESP8266-Wert ist ein Anhaltspunkt für die Anwendungslogik, ersetzt aber weder den IDF-WLAN-Stack noch die Arduino-Schicht; die Grösse bleibt **○**, geschätzt **1,3 … 2 MB** (kein Beleg).
- **8 MB (N8) trägt es sicher:** 2 × 3,2 MB Plätze plus 1,5 MB Dateisystem; das Dateisystem braucht rund 0,26 MB PWA plus Zwischenspeicher ✔ Eingänge oben. **4 MB (N4)** trägt zwei OTA-Plätze nur, wenn das Abbild unter **rund 1,6 MB** bleibt (2 × 1,6 + 0,5 Dateisystem + 0,1 Verwaltung ≈ 3,8 MB, ○ Rechnung) und der Standardplatz von 1,25 MB wäre ○ knapp. **Das zeigt erst der erste Bau.** Die Ersparnis von N4 gegen N8 ist 0,5 USD je Stück (JLC-API, 18.10.2026: 4,42 gegen 4,91 bei 1 Stück). **N8 bleibt gesetzt** (Spec F6).
- **N16 oder mehr:** kein Bedarf.

### 3.3 PSRAM: nötig?

- **Nein, aus der Firmware begründet:** Der ESP8266 läuft mit **statischem RAM 69'060 Byte** (DATA 1'748, RODATA 20'288, BSS 47'024; `BEFUNDE.md` L457) und **5'400 … 10'400 Byte freiem Heap** (L134, L254) — er ist heute der Engpass, der S3 nicht. Dazu kommt der STM-Teil mit **11'820 Byte** statisch (.data + .bss, `size` oben). Der S3 hat **512 KB SRAM** ✔ DS-MI S. 2 / DS-C. Summe statisch rund 81 KB (○ Rechnung), Rest für den IDF-Stack ist ein Vielfaches des ESP8266-Heaps. Genaue Grösse unter IDF: **nicht gemessen**.
- **Die Spec schliesst PSRAM-Nutzung aus** (neue Funktion) ✔ `specs/esp32s3-port/requirements.md`, Abschnitt „Nicht Teil dieser Änderung“ („PSRAM-Nutzung“).
- **PSRAM kostet:** Octal (R8) sperrt GPIO35–37 und senkt die Umgebung auf 65 °C ✔ DS-W1 S. 3, 12; R2 (Quad) hat denselben Gehäuse-Chip, der **EOL** ist (S3R2) ✔ DS-C S. 13; PSRAM-Module können mehr Strom ziehen ✔ `esp32s3-wroom-1.md` §8 (aus DS-W1 S. 29).
- **Hinweis zur Arduino-Variante:** Die vorkompilierten Bibliotheken von Arduino-ESP32 3.3.12 tragen `CONFIG_SPIRAM=y` (Quad, Auto) und liegen in fünf Speicherarten (`dio_opi`, `dio_qspi`, `opi_opi`, `qio_opi`, `qio_qspi`) ✔ `~/Library/Arduino15/packages/esp32/tools/esp32s3-libs/3.3.12/`, `specs/multiboard-quelle/design.md` §8.1. Das **DevKitC-1 (N8R8) braucht `qio_opi`, unsere Platine (N8, MINI-1-N8) `qio_qspi`**. Ein Abbild ohne PSRAM (`qio_qspi`, SPIRAM aus) sollte auf beiden laufen, ein Octal-Abbild nicht auf unserer Platine (○, am DevKit zu prüfen). **Das hängt am Chip/Flash, nicht an WROOM gegen MINI.**

### 3.4 Erfüllungstabelle

| Anforderung | WROOM-1-N8 | MINI-1-N8 | WROOM-1-N8R8 / N16R8 | WROOM-1U / MINI-1U | WROOM-2 | PICO-1 |
|---|---|---|---|---|---|---|
| A1–A3 (alle GPIOs und ADC1 vorhanden) | ja ✔ DS-W1 S. 11–12 | **ja** ✔ DS-MI S. 11–12 (IO0, 1, 2, 5–9, 15–18, 19, 20, 43, 44, EN; Skriptprüfung §6.1) | ja | wie Basismodul | ja ✔ DS-W2 S. 11–12 (alle 11 vorhanden) | nicht gelesen |
| A4 (kein GPIO35–37 nötig) | ja | ja (dort frei) | ja, **gesperrt**, aber nicht verwendet | ja | IO35–37 NC | — |
| A6 (8 MB Flash) | ja | **ja** (8 MB Quad) | ja / 16 MB | ja | 16/32 MB Octal | 8 MB |
| PSRAM überflüssig | ja | ja | ja, aber vorhanden und ungenutzt, **65 °C**, Octal | — | Octal, **65 °C** | **PSRAM immer verbaut** |
| A7 integrierte Antenne | ja | ja | ja | **nein** (Buchse, Antenne fehlt, Gehäuse/Kabel offen) | ja | **nein** (Antenne auf Platine) |
| A7 Format 1:1 | **drei Konflikte** (§4) | **keine neuen** (§4) | wie WROOM-1 | — | wie WROOM-1 | Eigen-HF-Layout |
| A8 Umgebung 85 °C | ja | ja | **nein (65 °C)** | ja | **nein (65 °C)** | N8R2 ja |
| A9 bei JLCPCB bestückbar | ja, Extended, 1'696 (18.10.) | ja, Extended, **3'882** | ja | ja | **Bestand 0** | **Bestand 0** |

## 4. Format 1:1: Antennenzone, `H6`, `SW1`, Pad 14, MINI-1 gegen WROOM-1

### 4.1 Eingänge

- **Antennenbereich WROOM-1: 6 mm tief über die ganze Modulbreite (18,0 mm)** ✔ DS-W1 Abb. 11-1, S. 45; HDG Abb. 26, S. 31 (PDF 34).
- **Antennenbereich MINI-1: obere 5,05 mm über die ganze Modulbreite 15,4 mm; Masse 15,4 × 20,5 ± 0,2; Unterteil (Blechdeckel) 14 × 13,55** ✔ DS-MI Abb. 10-1, S. 42; Abb. 11-1 „Antenna Area“, S. 45.
  **Alle Pads der Oberseite (Pins 46–62, 65) sind Masse**; auf der Antennenseite liegt also **kein Signalpin** ✔ DS-MI Abb. 3-1, S. 10.
- **Landemuster MINI-1:** 60 Randpads 0,4 × 0,8 mm + 4 Eckpads 0,8 × 0,8 mm, Reihenabstand 14 mm, EPAD aus 3 × 3 Feldern zu 1,2 mm (4,5 mm gesamt) ✔ DS-MI Abb. 11-1, S. 45. **Raster 0,85 mm** (Zeichnung zeigt „1,7“ für zwei Teilungen; der Footprint rechnet 0,85) ✔ DS-MI S. 45 und KC-FP.
  Pads der Randreihen liegen **unter dem Modul** (Unteransicht, Abb. 10-1), also gilt wie beim WROOM-1 mit EPAD: Röntgenprüfung, ● JLC-PART C2913206 „X-ray Inspection is required“; für C2913198 steht das **nicht** auf der Teileseite (JLCPCB schaltet sie nach `jlcpcb-fertigung.md` §5 beim Modul mit verdeckter Fläche ohnehin zu; ○ gleiche Gebühr je Stück).
- **V2-Festpunkte** (Platinenkoordinaten, ✔ V2-PCB, Kopie; stimmt mit `layout-regeln.md` §17 überein): Aussparung x = 118,75 … 135,25 (16,5 mm), y = 0 … 7,5; `H6`-Pads bis x = 108,46, Gehäuse bis x = 109,29; `SW1`-Pads bis x = 117,5, y = 24,95 … 30,55; `R12` Pads bis x = 117,2; `H10` ab x = 139,05. Das Modul sitzt mit Mitte x = 127,0 (AE-16).
- **Standard-Footprint WROOM-1:** Rule Area 48 × 21 mm (15 mm seitlich, ✔ `layout-regeln.md` §11), die `H6` mit drei Pads trifft.
- **Standard-Footprint MINI-1 in KiCad 10:** **es gibt keinen `ESP32-S3-MINI-1`.** Das Symbol `RF_Module:ESP32-S3-MINI-1` verweist auf den Footprint **`RF_Module:ESP32-S2-MINI-1`** (das MINI-1U-Symbol auf `ESP32-S2-MINI-1U`) ✔ KC-SYM, Eigenschaft Footprint; `RF_Module.pretty` enthält keine S3-MINI-Datei (✔ Verzeichnis). Der S2-Footprint hat **73 Pads** (60 + 4 Eck + 9 EPAD), **keine Rule Area**, aber ein **Courtyard mit 15 mm Rand** (x ± 22,7 mm um die Modulmitte, ✔ KC-FP per Skript).
  **Gegenprobe gegen DS-MI Abb. 11-1:** Reihenabstand ±7,0 mm (= 14), Pad 0,8 × 0,4 mm, Raster 0,85 mm, Eckpads 0,8 × 0,8 mm, EPAD 3 × 3 × 1,2 mm im Raster 1,65 mm (= 4,5 mm) — **stimmt** ✔; Pinnummern des Symbols gegen DS-MI Tab. 3-1 **alle 45 Signalpins gleich** (nur 23/24 heissen im Symbol `USB_D−`/`USB_D+` statt IO19/IO20; Skript, §6.1).
  **Abweichung:** der Fab-Umriss des S2-Footprints ist **15,5 × 20,1 mm**, das Datenblatt gibt **20,5 ± 0,2 mm** ✔ KC-FP gegen DS-MI S. 42 — der Layouter nimmt die Pad-Lage, nicht den Fab-Umriss.

### 4.2 Rechnung gegen die Festpunkte (Skript, Kopie von V2, 18.10.2026)

Lage: Mitte x = 127,0; Oberkante des Moduls bei y = T. Abstand = **kleinster Abstand Pad–Pad** (nicht Gehäuse), Platinenkoordinaten.

**Wahl von T:** Kupfer ≥ 0,3 mm (V2-Regel; JLCPCB-Mindestmass 0,2 mm, `layout-regeln.md` §11) vom Aussparungsgrund y = 7,5.
- WROOM-1: Pad 1 liegt 7,49 mm unter der Oberkante, seine Oberkante bei T + 7,04 ⇒ **T ≥ 0,66 … 0,76** ✔ `layout-regeln.md` §11. **Gerechnet mit T = 0,76.**
- MINI-1: die Pad-Reihe der Oberseite beginnt **5,05 mm** unter der Modul-Oberkante (✔ DS-MI Abb. 10-1/11-1: die 5,05 mm sind der Antennenbereich bis zur Pad-Aussenkante; KC-FP: Pad-Oberkante 0,1 mm tiefer, Zuordnung ○ ±0,1) ⇒ **T ≥ 7,5 + 0,3 − 5,05 = 2,75 mm** (2,65 mm bei 0,2 mm Kupferabstand) ○ Rechnung. **Gerechnet mit T = 2,75.** Dann reicht der Antennenbereich von y = 2,75 bis 7,8: **0,3 mm des unteren Randes liegen auf Platinenmaterial** (ohne Kupfer), der Rest in der Aussparung ○.
  Ergebnis in den Pads: y = 7,8 … 22,6, x = 119,6 … 134,4.

| Grösse | **WROOM-1, T = 0,76** | **MINI-1, T = 2,75** |
|---|---|---|
| Antennenbereich über der Aussparung (16,5 mm breit) | 18,0 mm breit: **0,75 mm je Seite auf FR-4** | 15,4 mm breit: **0,55 mm Luft je Seite**, nichts über FR-4 (seitlich) |
| Pad 1 / Pad 40 gegen Aussparungsgrund | liegen ab T ≥ 0,76 knapp unterhalb (Vorgabe: Modul **absenken**) | Pads der Oberseite (Masse) ab y = 7,8; **kein Signalpad an der Aussparung** |
| **`SW1`** (Pads x ≤ 117,5) | **0,0 mm** (Überlappung in y; Abhilfe: Modul ≥ 0,75 mm nach +x oder `SW1` ≥ 0,5 mm nach −x) | **3,15 mm** (kein Eingriff nötig) |
| **`R12`** (Pads x ≤ 117,2) | 0,3 mm | **2,4 mm** |
| **`H10`** (Pad ab x = 139,05) | 2,55 mm | **5,38 mm** |
| **`H6`** — Abstand Pad-Kante zu Modul-Pad (x) | 9,04 mm (Gehäuse–Modul 9,54 mm, `layout-regeln.md`) | **11,14 mm** (**+2,1 mm**); Abstand Gehäuse `H6` (x = 109,29) bis Antennenkante (x = 119,3): **10,0 mm** |
| Sperrfläche / Rule Area in der Standardbibliothek | 48 × 21 mm, trifft **3 `H6`-Pads** und das Gehäuse | **keine Rule Area**; wollte man die 15 mm des HDG nachbilden (x = 104,3 … 149,7), lägen **2 `H6`-Pads** darin (Pad 1 bei 107,56, Pad 2 bei 105,02) ○ |
| Courtyard des Standardfootprints überlappt | `SW1`, `R12`, `H10`, `RN1`, `H6`, `U5`, `SW2` (**7** Teile) | `R12`, `RN1`, `H6`, `U5` (**4** Teile) |
| freier Streifen unter dem Modul bis zur Platinenkante (y = 31,25) | rund **5,0 mm** (Modulkante y ≈ 26,3) | rund **8,0 mm** (Modulkante y ≈ 23,3) ○ |
| USB-Pads nahe `SW1`/Antenne | Pad 13/14 (D−/D+) am linken Rand bei y = 22,8 / 24,1, `SW1`-Kante 0 mm | Pad 23/24 mitten an der Unterkante (x = 127,0 / 127,85, y ≈ 22,3), `SW1` ≥ 3 mm; Weg zu `USB1` (x = 72,5) über den Streifen ○ |

**Was sich nicht ändert:** `H6` bleibt **10 mm** von der Antenne entfernt, die HDG-Forderung **15 mm** frei (Abb. 26, S. 31) ist weder mit dem WROOM-1 (9,0 … 9,5 mm) noch mit dem MINI-1 (10 … 11 mm) zu erfüllen, solange `H6` im Format bleibt. Das HDG fordert zudem die Trägerplatine **beidseits und unterhalb der Antenne auszuschneiden**; die V2-Aussparung tut das mit 0,55 mm (MINI-1) bzw. mit 0,75 mm Überstand (WROOM-1).
**Was nicht belegt ist:** ob das MINI-1 mit seiner kleineren, tiefer in der Aussparung sitzenden Antenne **besser oder schlechter** abstrahlt. **Kein Datenblatt nennt Gewinn, Wirkungsgrad oder Strahlungsdiagramm der PCB-Antenne** (Volltextsuche nach „gain“, „efficiency“, „radiation“ in DS-W1 und DS-MI: nur der Gewinn der **externen** Zertifizierungsantenne der -U-Module, 2,33 dBi). Die Konduktionswerte (Sendeleistung, Empfindlichkeit) sind in beiden Datenblättern **gleich** (Tab. 7-4 beide: −98,2 dBm bei 802.11b/1 Mbit/s ✔ DS-W1 und DS-MI S. 33). Entscheiden kann nur die **RSSI-Messung am Prototyp** (Spec F4).

### 4.3 Mechanik

- **Bauhöhe:** MINI-1 **2,4 ± 0,15 mm**, WROOM-1 **3,1 mm** ✔ DS-MI S. 42; DS-W1 S. 3. Ob das Gehäuse beide aufnimmt: Gehäusezeichnung **nicht im Repo** (`layout-regeln.md` §20).
- **Befestigung:** Module haben keine Befestigungspins (Lötfläche). Die **Pad-Lage ändert sich vollständig** (WROOM: Pads links 1–14, unten 15–26, rechts 27–40; MINI: **links 1–15, unten 16–30, rechts 31–45, oben 46–62/65 nur Masse**; ✔ DS-MI Abb. 3-1, S. 10).
- **Lage der Signale am MINI-1** (○ aus der Pintabelle DS-MI Tab. 3-1, Pad-Reihenfolge der Zeichnung): links IO0 (4), IO1 (5), IO2 (6), IO5–IO9 (9–13); unten IO15–IO18 (19–22), **USB D−/D+ (23/24)**; rechts **TXD0 (39), RXD0 (40)**, **EN (45)**. Gegenüber WROOM-1 wandern **LDR (GPIO1) und DS18B20 (GPIO2) auf die linke Seite** (näher an `H5`/`H7`/`U1`), UART bleibt rechts, **EN wandert vom linken auf den rechten Rand** (Pad 45, y ≈ 9,25 bei T = 2,75) — `R12`, `C1`, `SW3` müssten dorthin oder lang zurück (○ Lageentscheid des Layouters).

## 5. JLCPCB-Fertigung je Kandidat (Prüfdatum 18.10.2026, 02:28 Uhr)

**Alle Module sind Extended**, keines Basic oder Preferred (`componentLibraryType = expand`, `preferredComponentFlag = false` in allen 52 Treffern der Suchen „ESP32-S3-WROOM-1“ (31), „ESP32-S3-MINI-1“ (8), „ESP32-S3-WROOM-2“ (7), „ESP32-S3-PICO-1“ (6)) ● JLC-API.
**Grund Extended:** ein Funkmodul gibt es im Standard-Katalog nicht; Basic-Alternative nicht gefunden. Bestückung bei den beiden Hauptkandidaten: „SMT Assembly“, **PCBA Type „Standard Only“, MSL 3** ● JLC-PART für C2913198 und C2913206.

| Teil | LCSC/JLC | Klasse | Bestand JLC (18.10.) | Preis 1 Stück / ab 100 (USD) | Gehäuse laut JLC | Bemerkung |
|---|---|---|---|---|---|---|
| **WROOM-1-N8** (heute `U2`) | **C2913198** | Extended | **1'696** (LCSC-Seite: 554; Blatt 17.10.: 416) | 4,9123 / 3,4267 | SMD, 25,5 × 18 mm | **Bestand schwankt stark (Widerspruch 1)** |
| **MINI-1-N8** | **C2913206** | Extended | **3'882** (LCSC-Seite: 3'882) | **4,7246** / 3,3565 | „SMD“ ohne Mass | Mindestmenge 2 (`minPurchaseNum`) |
| MINI-1U-N8 | C2980299 | Extended | 1'084 | 4,8976 | „VFQFN-56-EP“ (falsch bezeichnet) | U.FL, hier ungeeignet |
| MINI-1-N4R2 | C3013941 | Extended | 3'012 | 4,9956 | SMD | 4 MB, PSRAM 2 MB Quad |
| WROOM-1-N4 | C2913197 | Extended | 1'902 | 4,4193 / 3,43 | SMD, 25,5 × 18 | 4 MB: nur nach erstem Bau |
| WROOM-1-N16 | C2913199 | Extended | 1'415 | 5,1164 / 3,861 | wie oben | |
| WROOM-1-N4R2 / N8R2 / N16R2 | C2913203 / C2913204 / C2913205 | Extended | 622 / **522** / 891 | 5,28 / 4,65 / 5,27 | wie oben | Chip **S3R2 EOL** (✔ DS-C S. 13) |
| WROOM-1-N8R8 | C2913201 | Extended | **6'061** | 5,0249 / 3,6536 | wie oben | Octal, 65 °C, IO35–37 gesperrt; **bis 17.10. LCSC-Bestand 0** |
| WROOM-1-N16R8 | C2913202 | Extended | **32'577** | 5,3368 / 3,7402 | wie oben | Octal, 65 °C |
| WROOM-1U-N8 / -N8R2 | C2980297 / C3013944 | Extended | 2'837 / 4'936 | 4,60 / 5,07 | SMD, 19,2 × 18 | U.FL |
| WROOM-2 (alle) | C3021270, C19170935, C42417293, … | Extended | **0** | 10,87 … 37,31 | | nicht lieferbar |
| PICO-1-N8R2 / -N8R8 | C7558093 / C7545129 | Extended | **0** | 5,01 / 10,12 | LGA-56 | nicht lieferbar |

- **Bestand ist ein Momentwert.** Zwischen dem Blatt vom 17.10. und dem Abruf von heute haben sich die Zahlen derselben Teile stark bewegt (N8 416 → 1'696, 1U-N8 896 → 2'837, N8R2 712 → 522, N8R8 4'934 → 6'061). **Keine der Zahlen belegt Lieferfähigkeit zum Bestelltermin** (`jlcpcb-fertigung.md`, H4: Prüfung vor Bestellung, ab 2026-11-01 neu).
- **Mehrpreis:** Jede Extended-Nummer kostet bei JLCPCB eine Rüstgebühr je Bauteilart (`jlcpcb-fertigung.md` §10); ein Wechsel auf MINI-1-N8 **ersetzt** die Zeile `U2`, **fügt keine hinzu** ✔ Zeilenzahl der Stückliste bleibt (46).
- **Footprint-Gegenprobe:** WROOM-1: JLC-Gehäuse „SMD,25.5x18mm“ ✔ passt zu DS-W1 (25,5 × 18,0). MINI-1-N8: JLC-Gehäuse „SMD“ ohne Mass; der **EasyEDA-Footprint der Nummer war nicht abrufbar** (Fehlerseite) — **nicht geprüft**. Dass Pad-Geometrie des KiCad-Footprints und DS-MI übereinstimmen, ist belegt (§4.1); ob die JLC-Bestückungsdaten dieselbe Lage nehmen, zeigt die Vorschau im Bestelldialog. Beide Footprints stehen **nicht** in der Drehtabelle des Fabrication Toolkit (`plugins/transformations.csv`, `jlcpcb-fertigung.md`): Drehung/Versatz je Feld prüfen (`FT Rotation Offset`).
- **Mit besserem Bestand als WROOM-1-N8:** MINI-1-N8 (3'882, gleiche Funktion), WROOM-1-N8R8 (6'061, aber Octal, 65 °C), WROOM-1-N16R8 (32'577, wie vorige). **Von diesen drei hat nur MINI-1-N8 dieselben Randbedingungen** (85 °C, alle GPIOs, kein PSRAM, 8 MB).

## 6. Software-Folgen

### 6.1 Was gleich bleibt (alles ✔ DS-MI gegen DS-W1)

| Punkt | WROOM-1-N8 | MINI-1-N8 | Beleg |
|---|---|---|---|
| Strapping-Pins | GPIO0 (Pull-up), GPIO3 (schwebend), GPIO45 (Pull-down), GPIO46 (Pull-down) | **gleich** | DS-MI Tab. 4-1, S. 13; DS-W1 Tab. 4-1, S. 13 |
| Boot-Modi | SPI-Boot; Joint-Download (GPIO0 = 0, GPIO46 = 0) | **gleich** | DS-MI Tab. 4-3, S. 14 |
| VDD_SPI / GPIO45 | Tab. 4-4; DS-W1 S. 39: bei Modulen ohne PSRAM **GPIO45 beim Einschalten nicht High** | Tab. 4-4 **gleich** (S. 15); der Modulschaltplan DS-MI S. 39 trägt **keinen** solchen Hinweis; DS-C Tab. 1-1 nennt für S3FN8 VDD_SPI = 3,3 V | ✔ DS-MI S. 15, 39; DS-C S. 13 |
| USB-Pins | GPIO19 = D−, GPIO20 = D+ | **gleich** (Pins 23/24) | DS-MI Tab. 3-1, S. 11 |
| UART0 | GPIO43/44 | **gleich** (Pins 39/40) | ebd. S. 12 |
| ADC1 | IO1 = ADC1_CH0 | gleich | ebd. S. 11 |
| Versorgung / Strom | 3,0 … 3,6 V; ≥ 0,5 A; Spitze 355 mA | **gleich** (Tab. 6-2, 6-4) | DS-MI S. 27–28 |
| Entkopplung / EN | 22 µF + 0,1 µF; 10 kΩ + 1 µF | **gleich** | DS-MI Abb. 9-1, S. 41 |
| Flash | 8 MB Quad, 80 MHz Voreinstellung | 8 MB Quad im Chip (S3FN8), 80 MHz | DS-MI S. 3; DS-W1 S. 3 |
| ESD (Modul) | — | HBM ±2000 V, CDM ±500 V | DS-MI S. 47 (DS-W1: nicht verglichen) |

**Regel GPIO45 bleibt unverändert: offen, nie High beim Einschalten.** Ob beim MINI-1-N8 (Flash im Gehäuse) ein High an GPIO45 auch 1,8 V bewirkt, ist **nicht gelesen** — die Regel kostet nichts und gilt weiter.

### 6.2 ESP-IDF und Werkzeuge

- Beide Module sind ESP32-S3: Ziel `esp32s3`, dieselbe IDF und derselbe Arduino-Kern. Auf diesem Rechner: **Arduino-ESP32 3.3.12 mit IDF 5.5.5**, fünf Speicherarten ✔ `specs/multiboard-quelle/design.md` §8.1. **Eine Modul-Datei (Board-Definition) für MINI-1-N8 ist nicht nötig:** es zählt Flash-Art (Quad) und PSRAM (keins) = `qio_qspi`, wie beim WROOM-1-N8 ○ (Arduino-Boardmenü „ESP32S3 Dev Module“ nicht geprüft).
- Chip-Revisionen v0.1/v0.2 für FN8 und für den nackten S3 ✔ DS-C S. 13; die Errata (CACHE-126, RTC-126, ANALOG-160, USBOTG-4289, …) gelten für **beide** Module (Chip, nicht Modul) ✔ `esp32s3-wroom-1.md` §11.

### 6.3 Pin-Mapping gegen DevKitC-1 und Software-Kompatibilität

- **Die Software kennt GPIO-Nummern, keine Modulpads.** Alle elf Signale (GPIO1, 2, 5–9, 15–18) liegen am DevKitC-1 auf J1/J3 ✔ `esp32s3-wroom-1.md` §3.2, und sind am **MINI-1** vorhanden ✔ DS-MI Tab. 3-1 (Skriptprüfung: alle Pinnummern des KiCad-Symbols stimmen mit der Tabelle überein, §4.1).
- **Folge:** Ein Abbild für die MINI-1-Platine läuft auf dem DevKitC-1 und umgekehrt, **sofern die Speichereinstellung stimmt** (§3.3: ohne PSRAM, Quad-Flash). **Die Kompatibilität hängt also nicht am WROOM-1.**
- **DevKitM-1** (MINI-1-N8, ● DK-M) führt alle Signal-GPIOs auf J1 und hat die RGB-LED an **GPIO48** (DevKitC-1 v1.1: GPIO38 ✔ DK-C): beide stehen auf der Sperrliste (`requirements.md` E2) — **keine neue Kollision**. Wer will, kann es als zweites Testgerät führen (○, Beschaffung offen).
- **GPIO-Zahl:** WROOM-1 36 (alle nutzbar bei N8/N4/N16/R2; 33 bei R8/R16VA); MINI-1 **39** (bei N8; **38** bei N4R2, IO26 am PSRAM); WROOM-2 33. Die „12 freien“ Reserve-Pins in `pinbelegung.md` §4 werden bei MINI-1 **15** (zusätzlich IO26, IO33, IO34; IO35–37 bleiben nach Regel gesperrt) ○ Rechnung.

## 7. Empfehlung

### 7.1 Begründung

1. **Funktion:** Beide Module (N8) erfüllen jede Anforderung aus §3 (A1–A6, A8, A9). Der Wechsel **verschlechtert keine** Funktion.
2. **Format:** Von den sechs in `layout-regeln.md` §11 gerechneten Kollisionen löst das MINI-1 **vier** (`SW1`/Pad 14: 0,0 → 3,15 mm; `R12`: 0,3 → 2,4 mm; Pads 1/40 an der Aussparungskante: entfällt; Rule-Area-Fehler an `H6`: keiner im Standardfootprint) und entschärft eine (`H6`: +2,1 mm). **Eine bleibt:** die 15 mm des HDG seitlich der Antenne (und damit die Frage nach der Antennenleistung).
   Der WROOM-1 braucht dafür **drei Eingriffe**, die „exakt 1:1“ verletzen: Modul um ≥ 0,75 mm nach +x **oder** `SW1` um ≥ 0,5 mm nach −x, Modul um ≥ 0,76 mm nach unten, `R12` verschieben, Rule Area im Projekt-Footprint verkleinern.
   Beim MINI-1 entfällt der Eingriff an `SW1`; nötig bleibt: Modul um **2,75 mm** unter die Platinenkante setzen (○ Rechnung §4.2), Courtyard im Projekt-Footprint auf 15 mm Rand zur Antennenfläche einstellen statt auf das ganze Modul.
3. **Beschaffung:** C2913206 3'882 Stück gegen 1'696 (JLC, 18.10.) bzw. 554 (LCSC) für C2913198; beide Extended, beide „Standard Only“, Preis beim MINI-1-N8 um 0,19 USD niedriger bei 1 Stück (JLC-API). **Beide Bestände sind Momentwerte.**
4. **Software:** keine Änderung (§6).
5. **Weitere Vorteile (○):** niedriger (2,4 gegen 3,1 mm), freier Streifen unter dem Modul 8 gegen 5 mm (Leitungsführung für UART, 1-Wire, ADC, USB); LDR und DS18B20 liegen am MINI-1 auf der Seite der Gegenstellen.
6. **Gegen den Wechsel:** (a) Antennenleistung **nicht belegt** (kleineres Modul, kleinere Antenne; nur Messung entscheidet), (b) KiCad liefert **keinen S3-Footprint** — der Projekt-Footprint wird eine Kopie des S2-Footprints mit anderem Namen (Pad-Geometrie gegen Datenblatt geprüft, Fab-Umriss 20,1 gegen 20,5 mm), (c) **alle** Modulpads, die Tabelle in `pinbelegung.md` §2/§3, die Pad-Tabelle in `layout-regeln.md` §11 und der Schaltplan-Generator ändern sich, (d) `EN` liegt rechts, `R12`/`C1`/`SW3` müssen neu platziert werden, (e) alle Lötstellen unter dem Modul (Nacharbeit erschwert, ○), (f) Pitch 0,85 mm mit Pads 0,4 mm — Lackbrücke 0,10 mm ist machbar (Pad-Lücke 0,45 mm gegen 0,10 mm, ✔ `jlcpcb-fertigung.md` §16, Zeile „Lackbrücke“).

### 7.2 Empfehlung

**Wechsel auf ESP32-S3-MINI-1-N8 (C2913206)**, vorbehaltlich:
(1) Freigabe durch Architekt und **Nutzer**: **`requirements.md` E1 legt als Zielprodukt „den Nachbau mit dem Modul ESP32-S3-WROOM-1“ fest** (Nutzervorgabe) — das MINI-1 widerspricht dem Wortlaut und braucht eine ausdrückliche Änderung von E1; dazu AE-16, F6, OP-23 und `pinbelegung.md`. Der Nutzer wurde nach N8 gefragt (F6), nicht nach der Bauform,
(2) **RSSI-Vergleich** von MINI-1-N8 und WROOM-1-N8 in der Aussparung — beide Module sind einzeln erhältlich (Kosten rund 10 USD); ohne Messung gilt die Antennenleistung für beide als unbelegt,
(3) Layouter bestätigt T = 2,75 mm und die Lage von `R12`/`C1`/EN.
**Rückfallebene:** WROOM-1-N8 bleibt zulässig, wenn der Nutzer das Format nicht antasten will (die drei bekannten Eingriffe aus `layout-regeln.md` §11, §19). **Nicht empfohlen:** -U-Module (Antenne und Gehäuse offen), WROOM-2 und PICO-1 (Bestand 0, andere Anforderungen), R8/R16 (65 °C, GPIO35–37), N4 (Flash-Reserve erst nach erstem S3-Abbild belegbar), N8R2/N4R2 (Chip läuft aus bzw. PSRAM unnötig).

### 7.3 Was ein Wechsel ändert

| Ort | Änderung |
|---|---|
| **Schaltplan** (`hw-designer`) | `U2`: Symbol `RF_Module:ESP32-S3-MINI-1` (65 Pins, Nummern gegen DS-MI Tab. 3-1 geprüft), Wert, LCSC C2913206, Footprint-Zuordnung auf den Projekt-Footprint; `gen.py`, `expected.json`; Netznamen (`IO1`, …) und die Regeln in `hardware-pinregeln.json` bleiben, die Zuordnung zu Pads ändert sich (H3 prüft Namen, nicht Pads). GND-Pins (1, 2, 42, 43, 46–65) werden mehrfach belegt; die Parität darf nichts melden. |
| **Layout** (`hw-layouter`) | Projekt-Footprint (Kopie von `ESP32-S2-MINI-1`, Name S3, Rule Area/Courtyard nach §4.2, Fab-Umriss 20,5 mm); Modul bei T ≥ 2,75 mm; `SW1`, `H6`, `H10` bleiben; `R12`/`C1` an EN (Pad 45, rechts oben); `SW5` an IO0 (Pad 4, links); USB-Paar von Pad 23/24 zum `USB1` über den Streifen unter dem Modul; UART von Pad 39/40 (rechts) nach `H6`. |
| **Spec / Architektur** | `requirements.md` **E1** (Wortlaut „WROOM-1“), F6, AE-16 (Text „alle 36 GPIOs“, Lage, Aussparung), AE-15 (Zeile `U2`: Alternative WROOM-1-N8 C2913198), `pinbelegung.md` (Spalte „Modulpad“ aller Zeilen, §3, §4: 39 statt 36 GPIOs), `layout-regeln.md` §11, §17, Zeile `U2`; neues Komponentenblatt `esp32s3-mini-1.md` (Pads, Strapping, Strom — nur Verweise auf DS-MI, keine Neuberechnung) |
| **Firmware** | **keine** Änderung (GPIO-Nummern gleich); Speicherart `qio_qspi` wie bisher |
| **Stückliste / JLCPCB** | Zeile `U2` C2913198 → C2913206; Zeilenzahl 46 unverändert; Drehung/Versatz in der Vorschau prüfen; Röntgenprüfung beim Modul |

## 8. Widersprüche und Lücken

1. **Bestand von C2913198:** Blatt `esp32s3-wroom-1.md` §10, `entscheidungen.md` AE-15 und `requirements.md` F6 nennen **416** (JLC und LCSC, 17.10.); die JLC-API liefert am 18.10. **1'696**, die LCSC-Seite **554**. Ursache nicht geklärt (anderes Feld, Nachlieferung, Schwankung); die anderen Teile bewegen sich ebenso. **Die Aussage „Beschaffungsrisiko wegen 416 Stück“ ist damit überholt, die Lieferfähigkeit bleibt unbelegt.**
2. **KiCad:** Symbol `ESP32-S3-MINI-1` verweist auf einen **S2**-Footprint; der Fab-Umriss (20,1 mm) weicht vom Datenblatt (20,5 mm) ab.
3. **Distributor-Eigenschaften unzuverlässig:** C2980299 (MINI-1U-N8) trägt bei LCSC „−40 … +105 °C“ und „1,8 … 3,6 V“, das Datenblatt „−40 … 85 °C“, „3,0 … 3,6 V“ (✔ DS-MI S. 2, 27) — ● gegen ✔; ebenso unterscheiden sich die „Empfindlichkeit“ von C2913198 (−98,2) und C2913206 (−104,5), obwohl DS-W1/DS-MI Tab. 7-4 beide −98,2 dBm nennen. **Für Eigenschaften zählt das Datenblatt, nicht das Feld der Teilebibliothek.**
4. **Datenblattstände:** DS-W1 v1.8 (2026-03-02) zeigt im Blockbild (Abb. 2-1) und in den Varianten noch `ESP32-S3R2`, DS-C v2.2 (2026-03-05) erklärt `S3R2` für **EOL** (→ S3RH2). Welcher Chip im heutigen Lager von C2913204 steckt, ist **nicht belegt**.
5. **DevKitC-1:** N8 und N8R2 sind dort **EOL**, die Module nicht (DS-W1). Wer die Variante des DevKits beschaffen will, bekommt N8R8 (Octal, 65 °C).
6. **Antenne:** kein Datenblatt gibt Gewinn/Wirkungsgrad der PCB-Antenne an; der Satz „MINI-1 ist besser/schlechter abgestrahlt“ ist **nicht belegt**.
7. **HDG-Zahl 6 mm** (Abb. 26, allgemein) gegen **5,05 mm** (MINI-1, DS-MI): die Messung hier nutzt das Datenblatt des Moduls.
8. **Lagezuordnung 5,05 mm:** die Zeichnung zeigt das Mass vom Modulrand bis zur Pad-Aussenkante; der KiCad-Footprint liegt 0,1 mm anders. Die Rechnung ±0,1 mm.
9. **Spec-Wortlaut:** `specs/esp32s3-port/requirements.md` E1 („Nachbau der Platine mit dem Modul ESP32-S3-WROOM-1“) gegen die Empfehlung MINI-1 (§7.2): kein technischer, aber ein **formaler Widerspruch**, den nur der Nutzer auflösen kann.
10. **Nicht gelesen:** Produktauswahl-Seite, RF-Zertifikate, EasyEDA-Footprints, DK-M-Tabellen vollständig, PICO-1 GPIO-Zahl und Pinbelegung, Flash-/PSRAM-Spezifikation (DS-MI Tab. 6-8/6-9) für den Vergleich der Datenerhaltung.

## 9. Offene Punkte

- **RSSI-Messung** MINI-1-N8 gegen WROOM-1-N8 in der V2-Aussparung (Spec F4). Ohne sie bleibt die Antennenfrage offen.
- **Entscheid des Architekten/Nutzers:** Wechsel ja/nein (AE-16, F6, OP-23).
- **Gehäusezeichnung** (Höhe 2,4/3,1 mm, 15 mm Freiraum, Öffnung `USB1`): liegt nicht im Repo.
- **Erster S3-Bau:** Abbildgrösse und Speicherverbrauch; erst danach lässt sich N4 beurteilen.
- **DevKitC-1-Test mit `qio_qspi`-Abbild** (N8R8-Hardware, Octal-PSRAM ungenutzt): am Gerät prüfen.
- **JLCPCB-Vorschau** für C2913206: Lage und Drehung des MINI-Footprints, Bestand zum Bestelltermin (H4).
- **GPIO45 bei Chips mit Flash im Gehäuse** (Wirkung von High beim Einschalten): nicht gelesen.

## Anhang: Rechenverfahren (nachvollziehbar)

Skript (`pcbnew` aus KiCad 10.0.2, `hardware/komponenten/` unberührt; Dateien nur im Scratchpad der Sitzung): lädt eine **Kopie** von `V2.kicad_pcb`, liest Pads und Courtyards von `H6`, `SW1`, `SW2`, `R12`, `H10`, `U5`, `U6`, `RN1`, `RN2`, `C18` (in V3 entfallen: `U4`, `H3`, `SW4`, `R14`–`R16`, `R18`), lädt die Footprints `RF_Module:ESP32-S3-WROOM-1` und `RF_Module:ESP32-S2-MINI-1`, setzt sie mit Mitte x = 127,0 und Oberkante T (WROOM: Ursprung 12,75 mm unter der Oberkante; MINI: 9,9 mm unter der Oberkante, so dass die Pad-Aussenkante 5,05 mm unter der Oberkante liegt) und gibt den kleinsten Pad–Pad-Abstand, Pads innerhalb 0,3 mm um die Aussparung (x 118,75 … 135,25, y 0 … 7,5) und die Courtyard-Überlappungen aus. Gegenprobe: das WROOM-Ergebnis bei T = 0,76 reproduziert die Zahlen aus `layout-regeln.md` §11 (`SW1` 0,0 mm, `R12` 0,3 mm, `H10` 2,55 mm); bei T = 0,05 meldet es Pad 1 und Pad 40 in der Aussparung, wie dort belegt.
