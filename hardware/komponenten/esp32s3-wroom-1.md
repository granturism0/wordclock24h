# Komponentenblatt: ESP32-S3-WROOM-1-N8 (Controller der Umbau-Platine)

Stand der Prüfung: **17.10.2026**. Verfasser: Datenblatt-Analyst. Alle Zahlen tragen ein Zeichen:
✔ im Datenblatt gelesen (Seite), ● Sekundärquelle (genannt), ○ Annahme/Erinnerung (nur als offene Frage).
Die Quellen liegen unter `hardware/komponenten/datenblaetter/` (nicht im Git).

## 1. Quellen, Variante, Gehäuse

| Kürzel | Unterlage | Revision | Datum | Adresse / Datei |
|---|---|---|---|---|
| **DS-M** | ESP32-S3-WROOM-1 & WROOM-1U Datasheet | **v1.8** | 2026-03-02 (Revisionstabelle S. 51) | `espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf`, 53 S. |
| **DS-C** | ESP32-S3 Series Datasheet | **v2.2** | 2026-03-05 (S. 84) | `…/esp32-s3_datasheet_en.pdf`, 87 S. |
| **HDG** | ESP32-S3 Hardware Design Guidelines | „Release master“, Titelseite **12.10.2026**; die letzte Zeile der eigenen Revisionstabelle ist v1.8 vom 2025-07-03 (S. 38) | siehe links | `docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/…-master-esp32s3.pdf`, 42 S. |
| **DK** | ESP32-S3-DevKitC-1 v1.1 User Guide (HTML, Hardware v1.1) | ohne Revisionsdatum; abgerufen 17.10.2026 | — | `docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.1.html` |
| **DK-Sch** | ESP32-S3-DevKitC-1 Schaltplan V1.1, Blatt 2 | Rev V1.1 | Blattdatum 30.11.2022 (Datei `SCH_…_20221130.pdf`) | `dl.espressif.com/dl/schematics/SCH_ESP32-S3-DevKitC-1_V1.1_20221130.pdf` |
| **TRM** | ESP32-S3 Technical Reference Manual (nur Kap. 8, 33, 39 gelesen) | **v1.8** | 2026-03-04 (S. 1524) | `…/esp32-s3_technical_reference_manual_en.pdf`, 1531 S. |
| **DS-M2** | ESP32-S3-WROOM-2 Datasheet (nur Pinbelegung S. 10–12) | v1.7 | 2026-03-02 | `…/esp32-s3-wroom-2_datasheet_en.pdf` |
| **Errata** | ESP32-S3 Series SoC Errata (HTML, nur Zusammenfassung) | v1.3 | 2025-03-31 | `docs.espressif.com/projects/esp-chip-errata/en/latest/esp32s3/` |
| **DS-F411** | STM32F411xC/xE Datasheet **DS10314 Rev 8**, Januar 2024 (nur Abschnitt 6.3.20 „12-bit ADC characteristics“, S. 114–117, gelesen). ST-Server von hier aus nicht erreichbar (HTTP/2-Fehler, Zeitüberschreitung); gelesen wurde die **Archivkopie** `web.archive.org/web/2023id_/https://www.st.com/resource/en/datasheet/stm32f411re.pdf`, 151 S., Dokument unverändert ST (Titel, Revisionsstand im Kopf der Seiten geprüft). Diese Quelle ist damit ein Abbild, nicht der Originalserver. | Rev 8 | 01/2024 | `datenblaetter/stm32f411re_datasheet_wayback2023.pdf` |
| **FW** | `src/adc/adc.c`, `src/adc/adc-config.h`, `src/ldr/ldr.c`, `src/ldr/ldr.h`, `src/system_stm32f4xx.c`, `src/main.c` (nur gelesen) | Stand Arbeitsbaum | 17.10.2026 | Repo |
| **V2-Netz** | `hardware/esp32s3/v2/nets.json` (KiCad-Export der V2) | — | 17.10.2026 | Repo |
| **IDF** | ESP-IDF Programming Guide v6.1 (ADC-Oneshot, ADC-Kalibrierung, USB-Serial/JTAG-Konsole, Kconfig) | v6.1, stable | abgerufen 17.10.2026 | `docs.espressif.com/projects/esp-idf/en/stable/esp32s3/…` |

Seitenzählung: DS-M, DS-C, TRM und DK-Sch: gedruckte Seite = PDF-Seite. **HDG: gedruckte Seite = PDF-Seite − 3**;
angegeben ist „S. <gedruckt> (PDF <n>)“.

- **Bauteil:** ESP32-S3-WROOM-1-**N8** (8 MB Quad-SPI-Flash, kein PSRAM, −40 … 85 °C, PCB-Antenne) ✔ DS-M S. 3.
  Zweitwahl **N8R2** (8 MB Flash + 2 MB Quad-PSRAM, −40 … 85 °C) ✔ S. 3.
- **Gehäuse:** SMD-Modul, 41 Pads (40 Randpads + EPAD/GND), **18,0 × 25,5 × 3,1 mm** ✔ DS-M S. 3, S. 11.
  Rastermass der Randpads 1,27 mm; Pad 1,5 × 0,9 mm; Antennenbereich 6 mm tief am oberen Modulrand ✔ DS-M S. 45.

## 2. Absolute Maximalwerte und Betriebsbedingungen

| Grösse | Wert | Beleg |
|---|---|---|
| Versorgung VDD33 (Modul) absolut | −0,3 … **3,6 V** | ✔ DS-M Tab. 6-1, S. 27 |
| Versorgung der Stromeingänge (Chip) absolut | −0,3 … 3,6 V | ✔ DS-C Tab. 5-1, S. 64 |
| Summe der Ausgangsströme aller IOs absolut | **1500 mA** (Fussnote: alle IOs 24 h auf High gegen Masse getestet, danach funktionsfähig) | ✔ DS-C S. 64 |
| **Spannungsgrenze je Einzelpin / Strom je Einzelpin absolut** | **nicht angegeben** — weder in DS-M, DS-C noch HDG (Volltextsuche nach „5 V tolerant“, „over-voltage“: keine Fundstelle) | ✔ gesucht, nicht gefunden |
| Lagertemperatur | Modul −40 … 105 °C (DS-M S. 27); Chip −40 … 150 °C (DS-C S. 64) | ✔ |
| Versorgung empfohlen | 3,0 / 3,3 / 3,6 V | ✔ DS-M Tab. 6-2, S. 27 |
| Versorgungsstrom der externen Quelle | **≥ 0,5 A** | ✔ DS-M S. 27; ✔ DS-C S. 64 („single supply: 500 mA or more“); ✔ HDG S. 5 (PDF 8) |
| VIH / VIL | ≥ 0,75 · VDD (max VDD + 0,3 V) / ≤ 0,25 · VDD (min −0,3 V) | ✔ DS-M Tab. 6-3, S. 27; DS-C S. 65 |
| VOH / VOL (hochohmige Last) | ≥ 0,8 · VDD / ≤ 0,1 · VDD | ✔ DS-M S. 27 |
| IOH / IOL bei VDD = 3,3 V, PAD_DRIVER = 3 | **typ. 40 mA / 28 mA** (Typwerte, keine Grenzwerte) | ✔ DS-M S. 27; DS-C S. 65 |
| Standard-Treiberstärke je Pin | 20 mA; GPIO17/18: 10 mA; GPIO19/20: 40 mA | ✔ DS-C S. 18, Fussnote 5 |
| Interne Pull-up/-down | typ. 45 kΩ | ✔ DS-M S. 27 |
| Eingangsstrom IIH / IIL | max. 50 nA; Pinkapazität typ. 2 pF | ✔ DS-M S. 27 |
| ESD (Chip) | HBM ±2000 V, CDM ±1000 V; Latch-up ±200 mA | ✔ DS-C Tab. 5-13, S. 69 |

## 3. Pins

### 3.1 Modulpins (41 Pads) ✔ DS-M Tab. 3-1, S. 11–12

IO0 = Pad 27, IO1 = 39, IO2 = 38, IO3 = 15, IO4–IO7 = 4–7, IO8 = 12, IO9 = 17, IO10–IO14 = 18–22,
IO15–IO18 = 8–11, IO19 (USB D−) = 13, IO20 (USB D+) = 14, IO21 = 23, IO35–IO38 = 28–31,
IO39–IO42 = 32–35, RXD0 (GPIO44) = 36, TXD0 (GPIO43) = 37, IO45 = 26, IO46 = 16, IO47 = 24, IO48 = 25,
EN = 3, 3V3 = 2, GND = 1, 40, EPAD 41. **36 GPIOs am Modul** ✔ DS-M S. 2.

### 3.2 Was das DevKitC-1 v1.1 am Header herausführt ✔ DK „Header Block“, Tab. J1/J3

- **J1 (22 Pins):** 3V3, 3V3, RST (=EN), **GPIO 4, 5, 6, 7, 15, 16, 17, 18, 8, 3, 46, 9, 10, 11, 12, 13, 14**, 5V, GND.
- **J3 (22 Pins):** GND, **TX (GPIO43), RX (GPIO44), GPIO 1, 2, 42, 41, 40, 39, 38, 37, 36, 35, 0, 45, 48, 47, 21, 20, 19**, GND, GND, GND.
- Das sind **alle 36 Modul-GPIOs**: 17 auf J1, 19 auf J3. Nicht herausgeführt ist nur das, was das Modul gar nicht herausführt
  (GPIO22–32, Flash-SPI). **GPIO47, GPIO48 und GPIO21 liegen am Header** (J3 Pin 17, 16, 18).
- **Auf dem DevKit belegt (Konflikte für unsere Software):** GPIO38 = adressierbare RGB-LED (v1.1; v1.0: GPIO48) — ✔ DK; GPIO0 = BOOT-Taster; EN = Reset-Taster;
  GPIO43/44 = UART0 zur USB-UART-Brücke CP2102N; GPIO19/20 = zweite Micro-USB-Buchse („ESP USB“) **direkt, ohne Vorwiderstände, nur mit ESD-Dioden** ✔ DK-Sch Blatt 2.
  Der Auto-Programmier-Kreis (DTR/RTS über zwei Transistoren auf EN und GPIO0) hängt an GPIO0/EN ✔ DK-Sch Blatt 2.
- **Tabellen der Fundstelle:** J1/J3 sind im DK vollständig abgedruckt; der Schaltplan-Link steht dort unter „Related Documents“ (✔ Blatt 2 gelesen).

### 3.3 Welche Variante belegt welche GPIOs intern

| Modul | Interne Belegung | Beleg |
|---|---|---|
| **N8** (Quad-Flash, kein PSRAM) | nur Flash-SPI-Pins (am Modul nicht herausgeführt); **alle 36 GPIOs frei** | ✔ DS-M S. 3, S. 12 |
| **N8R2** (+ 2 MB Quad-PSRAM) | PSRAM hängt an denselben SPI-Pins wie der Flash (SPICS1, SPICLK, SPID, SPIQ, SPIWP, SPIHD); **keine zusätzliche GPIO verloren** | ✔ DS-C Tab. 2-14, S. 31; ✔ DS-M S. 12, Fussnote b nennt nur Octal-Varianten |
| **N8R8, N16R8, (N4R8)** (Octal-PSRAM) | **GPIO35, 36, 37 belegt**, nicht verwendbar; zudem −40 … 65 °C | ✔ DS-M S. 12, Fussnote b; S. 3 |
| **N16R16VA** (Octal, 1,8 V) | wie R8 (GPIO35–37) **und GPIO47/GPIO48 arbeiten mit 1,8 V** | ✔ DS-M S. 12, Fussnote c |
| **N32R16V** = WROOM-2 (anderes Modul, 33 GPIOs) | Pads 28–30 (IO35–37) **nicht beschaltet („NC“)**; IO47/IO48 laufen mit 1,8 V | ✔ DS-M2 S. 10–12 |
| DevKitC-1 mit N8R8 / N32R16V | GPIO35–37 laut DK „nicht für externe Nutzung“ | ✔ DK, Hinweis unter „Description of Components“ |

**Folgerung zur DevKit-Verträglichkeit:** Das heute verkaufte DevKit ist **N8R8 / N32R16V** (N8 und N8R2 sind dort als EOL geführt, ✔ DK „Ordering Information“).
Die Software darf also **GPIO35, 36, 37 nicht verwenden** und sollte **47/48 nicht als 3,3-V-Ausgang voraussetzen**, wenn sie auf jedem DevKit laufen soll.

## 4. Strapping- und Boot-Pins; Zustand der GPIOs bei Reset

### 4.1 Die vier Strapping-Pins

| Pin | Zustand bei Reset ohne externe Beschaltung | Funktion | Beleg |
|---|---|---|---|
| **GPIO0** | schwacher Pull-up (Bit = 1) | 1 = SPI-Boot; 0 (zusammen mit GPIO46 = 0) = Joint-Download-Boot (UART und USB) | ✔ DS-M Tab. 4-1/4-3, S. 13–14 |
| **GPIO46** | schwacher Pull-down (Bit = 0) | Boot-Modus (siehe GPIO0); steuert zudem ROM-Meldungen auf UART0 | ✔ DS-M S. 13–15; TRM S. 534–536 |
| **GPIO45** | schwacher Pull-down (Bit = 0) | **VDD_SPI-Spannung:** 0 → 3,3 V (Standard), 1 → 1,8 V | ✔ DS-M Tab. 4-4, S. 15 |
| **GPIO3** | **schwebend, kein interner Pull** | wählt die JTAG-Quelle — **nur wirksam, wenn EFUSE_STRAP_JTAG_SEL = 1 gebrannt ist**; im Auslieferungszustand (alle eFuses 0) „Ignored“, JTAG kommt vom USB-Serial/JTAG | ✔ DS-M Tab. 4-5, S. 15–16 |

- Alle Strapping-Pins werden beim Reset in Latches übernommen; danach sind sie normale IOs ✔ DS-M S. 13–14. **Haltezeit tH ≥ 3 ms** nach EN-Anstieg, Vorlaufzeit tSU ≥ 0 ✔ Tab. 4-2, S. 14.
- **Erlaubte Beschaltung:** GPIO0 mit Pull-up (empfohlen) und Taster nach GND; **keine grossen Kondensatoren an GPIO0**, sonst kann der Chip in den Download-Modus fallen ✔ HDG S. 15 (PDF 18).
- **GPIO45 darf beim Einschalten des N8 nicht hoch gezogen werden**: Das Modul hat bei Varianten ohne PSRAM keine gebrannte VDD_SPI-Vorgabe, High würde den Flash mit 1,8 V speisen ✔ DS-M S. 39, Text über dem Modulschaltplan. Bei PSRAM-Modulen ist VDD_SPI per eFuse fest, GPIO45 egal (ebd.).
- **GPIO46 muss für den UART-/USB-Download niedrig sein** (Standard), sonst kein Joint-Download ✔ Tab. 4-3, S. 14.
- **Verboten/unklar:** „Only documented patterns should be used“ ✔ TRM S. 535. Das TRM kennt zusätzlich einen **SPI-Download-Modus (GPIO0 = 0, GPIO46 = 1, GPIO1 = 1, GPIO2 = 0)** ✔ TRM Tab. 8.2-1, S. 535; GPIO1/GPIO2 sind dort offene Eingänge — siehe Widersprüche.
- Die JTAG-Pins GPIO39–42 sind keine Strapping-Pins, werden bei aktivem Pad-JTAG aber gebraucht ✔ DS-C S. 26.

### 4.2 Alle anderen GPIOs bei Reset und Boot ✔ DS-C Tab. 2-1, S. 16–17 (Tabelle als Bild geprüft); bestätigt HDG Tab. 9, S. 15–16

Ohne Eintrag in „At Reset“ ist weder Pull-up noch Pull-down noch Eingang aktiv, der Pin ist also **hochohmig** (Auslegung der Tabelle; das TRM sagt für GPIO1/GPIO2 ausdrücklich „floating … high-impedance“, S. 535).

| Pin | At Reset | Nach Reset |
|---|---|---|
| GPIO0 | WPU, IE | WPU, IE |
| GPIO1, 2, 3 | IE (hochohmig, kein Pull) | IE |
| GPIO4–8, 15, 16, 19, 21 | **nichts** (hochohmig) | 9–14, 17, 18: IE; 4–8, 15, 16, 19, 21: nichts |
| GPIO20 | **USB-Pull-up (D+) aktiv** | USB_PU |
| GPIO35–38 | nichts | IE |
| GPIO39 (MTCK) | nichts | IE; **WPU aktiv, solange EFUSE_DIS_PAD_JTAG = 0** (Fussnote 7) |
| GPIO40–42 (MTDO/MTDI/MTMS) | nichts | IE |
| GPIO43 (U0TXD), GPIO44 (U0RXD) | WPU, IE | WPU, IE |
| GPIO45, GPIO46 | WPD, IE | WPD, IE |

**Einschaltstörungen (Power-up-Glitches)** ✔ DS-C Tab. 2-2, S. 18–19; HDG Tab. 10, S. 17 (PDF 20):

- **GPIO1–14, 15, 16 (32-kHz-Pins), 17, 18: Low-Pegel-Ausgang für typ. 60 µs.** GPIO18 zusätzlich High-Glitch 60 µs.
- **GPIO19 und GPIO20: zwei High-Pegel-Glitches zu je ca. 60 µs; Gesamtdauer mit Verzögerung 3,2 ms (GPIO19) bzw. 2 ms (GPIO20).** GPIO20 zudem Pull-down-Glitch.
- **Kein Glitch aufgeführt für GPIO0, 21, 35–48 sowie die UART0-Pins.**

**Folgerung für ein Gate, das beim Booten nicht schalten darf (z. B. 5-V-Versorgung der LED-Kette, PB0 auf V2):**
(a) Der Pin ist beim Reset hochohmig, das Gate braucht einen **externen Pull** auf den Ausschaltpegel; (b) ein **P-Kanal-Gate (Low = ein) an GPIO1–18 würde 60 µs lang eingeschaltet**,
ein N-Kanal-Gate mit Pull-down nicht; (c) GPIO19/20 sind für Gates tabu (3,2 ms High); (d) **GPIO21** ist ohne Strapping-Funktion, ohne Glitch und hochohmig im Reset —
von den Datenblättern her der sauberste Kandidat; GPIO38 ist es auch, trägt aber am DevKit die RGB-LED; GPIO39 hat im Reset einen Pull-up.

## 5. 5-V-Toleranz und Pinströme (Frage 3)

- **Nicht 5-V-tolerant, soweit das Datenblatt es sagt:** Es gibt keine Aussage „5 V tolerant“. VIH ist auf **VDD + 0,3 V** begrenzt (✔ DS-M S. 27, DS-C S. 65), also ≈ 3,6 V bei 3,3 V Versorgung;
  eine absolute Maximalspannung je Pin steht nirgends (✔ gesucht). Behandle jeden GPIO als **maximal 3,6 V**. Die Schlussfolgerung „nicht 5-V-tolerant“ ist **meine Ableitung aus VIH max**, keine ausdrückliche Herstellerangabe.
- Das gilt auch für die ADC-Eingänge; der LDR-Teiler hängt an 3,3 V und bleibt damit unter der Grenze (siehe 6).
- **Ausgangsstrom:** typisch 40 mA Quelle / 28 mA Senke je Pin mit PAD_DRIVER = 3, Standardeinstellung **20 mA**; Summe aller IOs absolut 1500 mA (siehe 2).
  Ein Dauerbetrieb an der Typgrenze ist damit **nicht zugesichert**; für Gates und Pegelwandler (µA bis wenige mA) unkritisch.
- Ein 3,3-V-Ausgang erfüllt VOH ≥ 0,8 · VDD = 2,64 V ✔ DS-M S. 27 — ob das für den 5-V-Pegelwandler (74AHCT1G125, VIH bei 5 V Versorgung) reicht, ist im Datenblatt des Wandlers zu prüfen (nicht hier gelesen, ○).

## 6. ADC (Frage 4)

### 6.1 Fakten

- Zwei 12-Bit-SAR-ADC, 20 Kanäle ✔ DS-M S. 25. **ADC1 = GPIO1–10, ADC2 = GPIO11–20** ✔ HDG Tab. 11, S. 18 (PDF 21); TRM Tab. 39.3-1, S. 1468–1469; DS-M Tab. 3-1.
- **ADC2 gegen WLAN:** ADC2 wird vom WLAN-Controller (PWDET) mitbenutzt; ein Arbiter regelt den Zugriff, eine laufende ADC2-Wandlung kann **unterbrochen** oder **nicht gestartet** werden, die Daten tragen dafür Flag-Bits ✔ TRM §39.3.8, S. 1474–1475.
  HDG: „ADC1 is recommended for use“ ✔ S. 18 (PDF 21). IDF: der Treiber schützt `adc_oneshot_read()` gegen Kollision mit dem WLAN ● IDF adc_oneshot, Abschnitt „Hardware Limitations“.
  **Folgerung: ADC1-Pin nehmen (GPIO1, 2, 4–10; GPIO3 meiden, Strapping).** Der Errata-Eintrag ADC-183 („Digital Controller/DMA von ADC2 funktioniert nicht“) trifft v0.0–v0.2 ✔ Errata-Zusammenfassung — für Einzelmessungen ohne DMA ohne Belang.
- **Dämpfungsstufen:** 0 dB (k ≈ 100 %), 2,5 dB (≈ 75 %), 6 dB (≈ 50 %), 12 dB (≈ 25 %); Referenz Vref = 1100 mV „by design“ ✔ TRM §39.3.5, S. 1469.
  Die **nutzbaren Bereiche und Gesamtfehler nach Kalibrierung** ✔ DS-C Tab. 5-6, S. 66; HDG S. 18 (PDF 21):

  | Stufe | nutzbarer Bereich | Gesamtfehler (kalibriert) |
  |---|---|---|
  | ATTEN0 (0 dB) | 0 … 850 mV | ±5 mV |
  | ATTEN1 (2,5 dB) | 0 … 1100 mV | ±6 mV |
  | ATTEN2 (6 dB) | 0 … 1600 mV | ±10 mV |
  | ATTEN3 (12 dB) | 0 … 2900 mV | ±50 mV |

  Die Zuordnung ATTEN0…3 zu 0/2,5/6/12 dB nennt das DS-C nicht; sie folgt der Reihenfolge im TRM und den IDF-Konstanten (`ADC_ATTEN_DB_12` im IDF-Beispiel) ●.
- **Genauigkeit:** DNL −4 … +4 LSB, INL −8 … +8 LSB, bis 100 kSPS; gemessen mit **100 nF** am Eingang, Gleichspannung, 25 °C, **WLAN abgeschaltet** ✔ DS-C Tab. 5-5, S. 66. Für besseres DNL mehrfach abtasten und mitteln (Fussnote ebd.).
- **Nichtlinearität am unteren Ende:** Das DS-C nennt **keinen eigenen Wert** für den unteren Bereich — nur DNL/INL über alles und den kalibrierten Gesamtfehler (siehe Tabelle). **Nicht gelesen = nicht angegeben.** Behandle Spannungen unterhalb des Gesamtfehlers der gewählten Stufe als nicht aussagekräftig.
- **Kondensator am Eingang:** **0,1 µF** gegen Masse empfohlen ✔ HDG S. 18 (PDF 21); der Kondensator ist Messbedingung für die Tabelle ✔ DS-C S. 66.
- **Empfohlene Quellimpedanz:** **in keiner der vier Unterlagen angegeben** (Suche nach „impedance“ ohne Treffer zum ADC). Offene Frage.
- **Kalibrierung:** Die wahre Referenz liegt je Chip zwischen **1000 und 1200 mV**; ESP-IDF kalibriert mit Werten aus eFuses, die Espressif bei der Modulfertigung brennt (Curve-Fitting-Schema) ● IDF adc_calibration. Ohne Kalibrierung ist ein Fehler von bis zu ±9 % allein durch die Referenz möglich (Ableitung aus 1000/1200 mV gegen 1100 mV).
- **Zufallszahlen:** Der ADC speist den RNG; Lesen vermindert dessen Zufälligkeit ● IDF.

### 6.2 Rechnung für den LDR-Teiler

Teiler: 3,3 V — GL5528 (R_LDR) — Knoten — R22 — GND. **V_Knoten = 3,3 V · R22 / (R_LDR + R22).**
R_LDR: dunkel 1 MΩ, Zimmerlicht 20 kΩ, hell 10 kΩ (Vorgaben des Auftrags, ○ als Eingangsdaten). Alle Werte in mV, selbst gerechnet.

| R22 | dunkel (1 MΩ) | Zimmer (20 kΩ) | hell (10 kΩ) | Quellimpedanz im Dunkeln R_LDR ∥ R22 | Strom hell |
|---|---|---|---|---|---|
| 1 kΩ (heute) | 3,3 | 157 | 300 | 1,0 kΩ | 300 µA |
| 10 kΩ | 32,7 | 1100 | 1650 | 9,9 kΩ | 165 µA |
| 22 kΩ | 71,0 | 1729 | 2269 | 21,5 kΩ | 103 µA |
| 47 kΩ | 148,1 | 2315 | 2721 | 44,9 kΩ | 58 µA |
| 100 kΩ | 300,0 | 2750 | 3000 | 90,9 kΩ | 30 µA |
| (zum Vergleich 4,7 kΩ) | 15,4 | 628 | 1055 | 4,7 kΩ | 224 µA |

Beispielrechnung 10 kΩ, Zimmer: 3,3 · 10 / (20 + 10) = 1,10 V; dunkel: 3,3 · 10 k / (1 M + 10 k) = 32,7 mV.

Bewertung gegen die Stufen (Fehler = kalibrierter Gesamtfehler aus 6.1):

| R22 | passt „hell“ in … | dunkel gegenüber Fehler der kleinsten passenden Stufe | Abstand Zimmer–hell |
|---|---|---|---|
| 1 kΩ | alle Stufen | 3,3 mV gegen ±5 mV bei 0 dB: **0,7 × Fehler — das Dunkel ist nicht messbar** | 143 mV |
| 10 kΩ | nur **12 dB** (1650 mV > 1600 mV der 6-dB-Stufe, Übersteuerung ab R_LDR ≈ 10,6 kΩ) | bei 6 dB: 32,7 mV gegen ±10 mV = 3,3 ×; bei 12 dB: 0,7 × | 550 mV |
| 22 kΩ | nur 12 dB | 71 mV gegen ±50 mV = 1,4 × | 540 mV |
| 47 kΩ | nur 12 dB | 148 mV gegen ±50 mV = 3,0 × | 406 mV (Zimmer 2,3 V, hell 2,7 V: obere Stufenhälfte) |
| 100 kΩ | **keine** (hell 3000 mV > 2900 mV) | — | 250 mV, hell übersteuert |

> **Überholt am 17.10.2026 (Vorgabe des Nutzers):** LED-Ansteuerung, Power-Schalten und LDR-Lesung bleiben 100 % pin- und funktionskompatibel zu V2, weil dasselbe LED-Board weiterverwendet wird.
> **R22 bleibt 1 kΩ.** Die Empfehlungen 10 kΩ / 22 kΩ / 47 kΩ unten sind **zurückgezogen**, solange der Nutzer keine Abweichung freigibt; sie bleiben als Rechnung stehen. Massgebend ist Abschnitt **6.3**.

**Ergebnis der ursprünglichen Rechnung (keine Entscheidung, durch die Vorgabe überholt):**

1. **R22 = 1 kΩ ist der schlechteste Wert:** Das Dunkel liegt unter dem Gesamtfehler jeder Stufe; Zimmer und hell nutzen nur 157 … 300 mV.
2. **R22 = 10 kΩ mit 6 dB (ATTEN2, 0 … 1600 mV, ±10 mV)** ergibt den besten Kompromiss aus Auflösung und Fehler: dunkel 33 mV (3,3 × Fehler), Zimmer 1,10 V, hell 1,65 V —
   die hellsten Fälle (R_LDR < 10,6 kΩ) laufen in die Sättigung, was für eine Helligkeitsregelung unschädlich ist. Passt nur, wenn „hell“ nicht unterschieden werden muss.
3. **Ohne Sättigung:** **10 kΩ oder 22 kΩ mit 12 dB** (±50 mV): alle drei Zustände im Bereich, aber der Fehler von ±50 mV frisst bei 22 kΩ das Dunkel (71 mV) fast auf, bei 10 kΩ ganz.
   **47 kΩ mit 12 dB** hält das Dunkel mit 148 mV ≈ 3 × Fehler, hat aber den kleinsten Zimmer-hell-Abstand der brauchbaren Werte und liegt nahe an der 2,9-V-Grenze (hell 2721 mV; bei +10 % Versorgung bereits darüber).
4. **100 kΩ scheidet aus** (hell > 2,9 V).
5. **Zusatzpunkt 4,7 kΩ mit 6 dB** (nicht gefragt): kein Übersteuern (hell 1055 mV), Dunkel 15,4 mV (1,5 × Fehler), Zimmer 628 mV.
6. **Versorgungsabhängigkeit:** Die Knotenspannung ist proportional zur 3,3-V-Schiene (3,0 … 3,6 V = ±9 %), die ADC-Referenz aber nicht — die Messung ist **nicht ratiometrisch**. Für eine reine Helligkeitsstufung genügt das; für absolute Lux-Werte nicht.
7. **Quellimpedanz und 100 nF:** Bei 100 kΩ sind es 91 kΩ · 100 nF = 9 ms Zeitkonstante, bei 10 kΩ 1 ms; für einen trägen Helligkeitsregler unerheblich. Ein Fehler durch Eingangsleckstrom (max. 50 nA, ✔ DS-M S. 27 — Gültigkeit für den Analogpfad ○) wäre bei 91 kΩ bis zu 4,5 mV — bei 10 kΩ 0,5 mV.
   Eine Aussage zur zulässigen Quellimpedanz ist nach den Unterlagen nicht möglich.
8. Die Teilerwerte sind **nicht** gemessen: Die Werte 1 MΩ / 20 kΩ / 10 kΩ sind Annahmen aus dem Auftrag. Das Datenblatt des GL5528 liegt unter `datenblaetter/gl5528b.pdf` und wurde hier nicht ausgewertet (○).

### 6.3 LDR-Lesung mit R22 = 1 kΩ (V2-kompatibel)

Vorgabe: dasselbe LED-Board, derselbe Teiler wie V2. Eingangsdaten (○, Annahmen des Auftrags und von `HARDWARE.md`): LDR dunkel 1 MΩ, Zimmer 20 kΩ, hell 10 kΩ; Teiler an 3,3 V.
Knotenspannung V = 3,3 V · 1 kΩ / (R_LDR + 1 kΩ): **dunkel 3,3 mV, Zimmer 157,1 mV, hell 300,0 mV** (selbst gerechnet; identisch zu V2).

**Was die STM32-Firmware tatsächlich liest** (✔ Quelltext gelesen):

- `adc.c`: **eine einzelne Software-Wandlung**, 12 Bit, rechtsbündig, **keine Mittelung, keine Überabtastung**, Abtastzeit **3 Zyklen** (`ADC_SampleTime_3Cycles`), `ADC_PRESCALER = Div4` (`adc-config.h`); Referenz = VDDA = VREF+ (Datenblatt, siehe unten).
  Kanal auf BlackPill: **PA5 / ADC_Channel_5**; im V2-Netz hängt PA5 (U2 Pin 15) an `LDR` zusammen mit H5.2, H7.5 und R22.2 — **sonst nichts, insbesondere kein Kondensator** ✔ `hardware/esp32s3/v2/nets.json`.
- Takt F411: HSE 8 MHz → SYSCLK 100 MHz, **APB2-Teiler 1** → PCLK2 100 MHz ✔ `system_stm32f4xx.c`; **fADC = 100 / 4 = 25 MHz**; 3 Zyklen = **120 ns** Abtastzeit.
- `ldr.c`: `ldr_raw_value` ist der **ungeklammerte Rohwert 0 … 4095** (`LDR_MAX_RAW_VALUE = 4095`, `ldr.h`). Er geht unverändert an den ESP/die Oberfläche.
  Die Automatik klammert eine **Kopie** auf `ldr_min_value … ldr_max_value` (EEPROM, Zähler der 12-Bit-Skala) und rechnet **`ldr_value = 32 · (raw − min) / (max − min)`**, begrenzt auf 31; der Wert wandert **je Messung um höchstens 1 Stufe**.
  `ldr_set_min_value()`/`ldr_set_max_value()` übernehmen **eine einzelne Messung** als Grenze und weisen min ≥ max ab. Eine Messung wird **viermal je Sekunde** gestartet (`F_INTERRUPTS / 4`, `main.c` im Timer-Interrupt), die Automatik ändert den Wert also um höchstens 4 Stufen je Sekunde.
- Beispielspanne aus `HARDWARE.md`: dunkel ≈ 4, hell ≈ 373 Zähler, also **≈ 369 Zähler auf 32 Stufen ≈ 11,5 Zähler ≈ 9,3 mV je Helligkeitsstufe**. Die tatsächlich gespeicherten Grenzen des Nutzers sind nicht bekannt (○).

#### (1) Was der S3-ADC bei 0 dB liefert

| Grösse | Wert | Beleg |
|---|---|---|
| Bereich ATTEN0 (0 dB) | **0 … 850 mV** (alle drei Zustände liegen darin; hell = 300 mV hat 550 mV Reserve) | ✔ DS-C Tab. 5-6, S. 66; HDG S. 18 (PDF 21) |
| **Gesamtfehler kalibriert** | **±5 mV** | ✔ DS-C Tab. 5-6, S. 66 |
| Nominelle Stufe (1 LSB) | 1100 mV / 4095 = **0,269 mV** (Vref 1100 mV „by design“); im 850-mV-Bereich ≈ 3164 Codes | ✔ TRM §39.3.5, S. 1469 (Rechnung selbst) |
| DNL / INL (roh) | −4 … +4 LSB (≈ ±1,07 mV) / −8 … +8 LSB (≈ ±2,15 mV), gemessen mit 100 nF, Gleichspannung, 25 °C, **WLAN aus** | ✔ DS-C Tab. 5-5, S. 66 |
| Referenzstreuung ohne Kalibrierung | wahre Vref 1000 … 1200 mV je Chip = **±9,1 %** gegen 1100 mV | ● IDF adc_calibration (Rechnung selbst) |
| **Rauschen** | **in keiner Unterlage angegeben** (Volltextsuche DS-C, DS-M, HDG, TRM-Kap. 39, IDF: kein Wert) | ✔ gesucht, nicht gefunden |
| Untere Linearität | kein eigener Wert; der kalibrierte Gesamtfehler ±5 mV gilt über 0 … 850 mV | ✔ S. 66 |

- **Ausdeutung:** Der kalibrierte Fehler ±5 mV entspricht **±18,6 LSB des S3**. Der Dunkelwert (3,3 mV) liegt **unterhalb des Gesamtfehlers**: Der S3 liefert dort „0 … 8 mV“, also nicht zu unterscheidende Werte.
  Zimmer (157 mV) hat ±3,2 %, hell (300 mV) ±1,7 % relativen Fehler (Rechnung).
- **Mehrfachabtastung/Mittelung:** Das DS-C empfiehlt sie ausdrücklich, um das DNL zu verbessern („sample multiple times and apply a filter, or calculate the average value“) ✔ DS-C Fussnote zu Tab. 5-5, S. 66.
  Mittelung von N unkorrelierten Messungen senkt zufälliges Rauschen um **1/√N** (N = 4: 0,5; 16: 0,25; 64: 0,125; Mathematik, kein Datenblattwert) und glättet DNL-Sprünge; sie senkt **nicht** Offset, Verstärkungsfehler und die Kalibrierunsicherheit (±5 mV).
  Der DIG-ADC1-Regler besitzt zusätzlich eine Hardware-Filtereinheit (filter0/1) ✔ TRM §39.3.3, S. 1468; ob sie für Einzelmessungen zugänglich ist, ist eine Treiberfrage (○).
  Die Messbedingung „WLAN aus“ (S. 66) heisst: **mit WLAN-Betrieb ist die Genauigkeit nicht zugesichert**; Mittelung ist hier Pflicht, nicht Kür. Die Firmware misst nur viermal je Sekunde; 16 … 64 Abtastungen hintereinander kosten Mikro- bis Millisekunden (Laufzeit nicht gemessen, ○).

#### (2) Abbildung auf die STM-Skala (0 … 4095 gegen 3,3 V)

Die STM-Skala ist **ratiometrisch**: VREF+ ist intern mit VDDA verbunden ✔ DS-F411 Tab. 65 Fussnote 3, S. 115; und die 3,3 V des LDR-Teilers sind dieselbe Schiene. Der STM-Zähler ist daher
**N_STM = 4095 · R22 / (R_LDR + R22)**, unabhängig von der Schienenspannung (Rechnung).
Der S3 misst dagegen **absolut in mV** (interne Referenz). Mit der Schienenspannung V_rail ergibt sich:

**N_STM = V_mV · 4095 / V_rail_mV**; mit V_rail = 3300 mV: **N_STM = V_mV · 1,2409** (1 STM-Zähler = **0,806 mV**).

| Zustand | Spannung | STM-Zähler (ideal) | S3-Fehler ±5 mV in STM-Zählern |
|---|---|---|---|
| dunkel | 3,3 mV | 4,1 | **±6,2** (nicht auflösbar, 0 … 10) |
| Zimmer | 157,1 mV | 195,0 | **±6,2** (±3,2 %) |
| hell | 300,0 mV | 372,3 | **±6,2** (±1,7 %) |

- **Zusätzlicher Fehler durch die Schiene:** Weicht die tatsächliche 3,3-V-Schiene um den Anteil t von 3300 mV ab, verschiebt sich der Zähler um t · N. Bei t = 2 %: Zimmer ±3,9, hell ±7,4 Zähler; bei t = 3 %: ±5,8 / ±11,2; bei t = 5 %: ±9,7 / ±18,6 (Rechnung). Die Toleranz des 3,3-V-Reglers der V3 steht in dessen Datenblatt, nicht hier (○ offen).
  **Soll die Abbildung die Schiene nicht annehmen, muss sie gemessen werden**; ein ADC-Kanal auf dem S3-Modul für die eigene 3V3-Schiene ist in den gelesenen Unterlagen **nicht** beschrieben (offen).
- **Genauigkeit der Abbildung in Zählern der STM-Skala (Worst Case, Summe):** ±6,2 (S3-Gesamtfehler) **plus** t · N (Schiene). Typisch (Schiene ±2 %): **Zimmer ±10 Zähler, hell ±14 Zähler**. Gegenüber dem F411 (ET max ±5 Zähler, typ. ±2) ist das etwa **2- bis 3-mal so ungenau** (siehe 3); das Rauschen beider Wandler ist nicht beziffert.
- **Gültigkeit von `ldr_min_value`/`ldr_max_value`:** Die gespeicherten Werte sind **Zähler der 12-Bit-Ratio-Skala** und bleiben gültig, **wenn der S3 Zähler gleicher Bedeutung liefert**, d. h. nach obiger Formel. Eine Verschiebung um ±10 Zähler bei einer Spanne von ≈ 369 Zählern ändert die Helligkeitsstufe (je ≈ 11,5 Zähler) um **etwa ±1 Stufe** (Rechnung); die Firmware ändert den Wert ohnehin nur schrittweise.
  Unterhalb von `ldr_min_value` klammert `ldr.c` auf die Mindeststufe: Ein S3-Wert von 0 statt 4 (Dunkel) ändert deshalb **nichts** an der Helligkeit.
- **Überlauf:** Bei 0 dB und 850 mV endet der nutzbare Bereich bei N_STM = 1055. Die Firmware klammert auf `ldr_max_value`; `LDR_MAX_RAW_VALUE` bleibt 4095. Der Rohwert, den der ESP/die PWA zeigt, wäre auf dem S3 oberhalb von 1055 Zählern nicht mehr gültig. Das tritt bei R22 = 1 kΩ erst bei einem LDR unter etwa 2,9 kΩ auf (850 mV = 3,3 V · 1 kΩ / (R_LDR + 1 kΩ)); „hell“ = 10 kΩ (300 mV) liegt weit darunter.

#### (3) Vergleich mit dem STM32F411-ADC (DS-F411, Rev 8)

| Grösse | STM32F411 (so konfiguriert: fADC = 25 MHz, 3 Zyklen) | ESP32-S3 (0 dB, kalibriert) |
|---|---|---|
| Auflösung | 12 Bit, 1 LSB = **0,806 mV** an 3,3 V | 12 Bit, 1 LSB = **0,269 mV** nominell (850 mV nutzbar) |
| Referenz | **VREF+ = VDDA, ratiometrisch** ✔ Tab. 65, Fussn. 3, S. 115 | interne 1100 mV, **absolut** (±9 % je Chip, kalibriert) |
| Gesamtfehler ET | **±2 typ. / ±5 max. LSB** (30-MHz-Tabelle, RAIN < 10 kΩ) ≙ **±1,6 / ±4,0 mV**; 18-MHz-Tabelle ±3 / ±4 LSB ✔ Tab. 66–67, S. 116 | **±5 mV** ≙ ±6,2 STM-Zähler ✔ S. 66 |
| Offsetfehler EO | ±1,5 typ. / ±2,5 max. LSB (30 MHz) bzw. ±2 / ±3 (18 MHz) ≙ **±1,2 / ±2,0 mV** ✔ S. 116 | im ±5 mV enthalten, nicht einzeln angegeben |
| Verstärkungsfehler EG | ±1,5 / ±4 LSB (30 MHz); ±1 / ±3 (18 MHz) — am Messbereichsende (N = 4095), bei N = 373 nur ≈ 9 % davon ✔ S. 116 | im ±5 mV enthalten |
| DNL ED | ±1 / ±2 LSB ✔ S. 116 | ±4 LSB (≈ ±1,07 mV) ✔ S. 66 |
| INL EL | ±1,5 / ±3 LSB (30 MHz); ±2 / ±3 (18 MHz) ≙ ±2,4 mV max ✔ S. 116 | ±8 LSB (≈ ±2,15 mV) ✔ S. 66 |
| Rauschen | **nicht angegeben** (Datenblatt nennt keine Rauschzahl im gelesenen Abschnitt) | **nicht angegeben** |
| Bedingungen | Tab. 67 „fADC = 30 MHz, RAIN < 10 kΩ, VDDA 2,4–3,6 V“, charakterisiert, **nicht in der Produktion geprüft** ✔ S. 116 | DC, 25 °C, **WLAN aus**, 100 nF ✔ S. 66 |

Die Firmware läuft mit **25 MHz**, zwischen den Tabellen für 18 MHz (Tab. 66) und 30 MHz (Tab. 67, ✔ S. 116); Tab. 67 dient als Obergrenze (ET max ±5 LSB). Die Tabellen gelten **nur bei VDDA = VREF**, wie hier.

Gegenübergestellt bei den drei Spannungen (Zähler der STM-Skala, absolute Unsicherheit):

| | dunkel 3,3 mV (4,1 Z.) | Zimmer 157 mV (195 Z.) | hell 300 mV (372 Z.) |
|---|---|---|---|
| **F411**, ET max ±5 LSB | ±5 Z. = ±4,0 mV (4,1 ± 5: **nicht von 0 unterscheidbar**) | ±5 Z. (±2,6 %) | ±5 Z. (±1,3 %) |
| F411, ET typ. ±2 LSB | ±2 Z. | ±2 Z. (±1,0 %) | ±2 Z. (±0,5 %) |
| **S3 0 dB**, ±5 mV | ±6,2 Z. (**nicht von 0 unterscheidbar**) | ±6,2 Z. (±3,2 %) | ±6,2 Z. (±1,7 %) |
| S3 + Schiene ±2 % | ±6,3 Z. | ±10 Z. | ±14 Z. |
| S3 + Mittelung (zufälliges Rauschen ↓) | unverändert, der Fehler ist systematisch | unverändert | unverändert |

**Ergebnis des Vergleichs:** Der absolute Gesamtfehler des S3 (±5 mV) liegt **im selben Bereich wie der Worst Case des F411 (±4,0 mV)**, ist aber **doppelt so gross wie der typische F411 (±1,6 mV)**.
Der **grössere Unterschied ist die Referenz**: Der F411 ist ratiometrisch (Schienenschwankung kürzt sich heraus), der S3 nicht.
Beide ADC lösen den Dunkelwert (3,3 mV) nicht sauber auf; die Firmware klammert dort auf `ldr_min_value`.
Auflösungsmässig ist der S3 mit 0,269 mV je LSB **3 × feiner** als der F411 (0,806 mV); dieser Vorteil wird vom ±5-mV-Fehler vollständig aufgehoben (±5 mV = ±18,6 S3-LSB).

**Eine Besonderheit des F411 bei 3 Zyklen:** Die Formel für die höchste zulässige Quellimpedanz (Gl. 1, S. 116) ergibt mit k = 3, fADC = 25 MHz, CADC = 7 pF **1472 Ω als Grenze für RAIN + RADC**; mit RADC bis 6 kΩ (Tab. 65, S. 115) ist die Bedingung „Fehler < ¼ LSB“ **nicht erfüllbar**
(selbst mit RAIN = 0 nicht, bei CADC = 4 pF ergibt sich 2576 Ω). Die Firmware läuft damit **ausserhalb der Herstellerregel** für Quellimpedanzen — mit 1 kΩ Quelle und ohne Kondensator (V2).
Wirkung (eigene Abschätzung ○): Die Abtast-RC-Zeit ist (1 kΩ + 6 kΩ) · 7 pF = 49 ns gegen 120 ns Abtastzeit, der Rest eines Spannungssprungs zwischen zwei Messungen beträgt e^(−120/49) ≈ 8,6 %.
Bei stabilem Licht ist der Sprung ≈ 0, bei plötzlich geändertem Licht (3 → 300 mV) wäre der **erste** Abtastwert um bis zu rund 25 mV zu klein. Das ist **V2-Verhalten**, das die Kalibrierwerte des Nutzers bereits enthalten. Auf dem S3 entfällt es mit dem 100-nF-Kondensator (siehe 4), der Messwert ist dort also **etwas korrekter** als auf V2.

#### (4) Wirkung des 100-nF-Kondensators am ADC-Pin (C32) bei R22 = 1 kΩ

- **Auf den Gleichwert: keine Wirkung.** Ein Kondensator führt im Beharrungszustand keinen Strom. Der Leckstrom des Eingangs (≤ 50 nA ✔ DS-M Tab. 6-3, S. 27 — Gültigkeit für den Analogpfad ○) erzeugt an R_LDR ∥ R22 ≤ 1 kΩ höchstens **50 µV** (< 0,2 S3-LSB, < 0,07 STM-Zähler).
- **Auf die Zeitkonstante:** Der Kondensator sieht den Innenwiderstand des Teilers R_LDR ∥ R22: dunkel 999 Ω, Zimmer 952 Ω, hell 909 Ω → **τ = 100 µs / 95 µs / 91 µs**, Grenzfrequenz **1,59 / 1,67 / 1,75 kHz** (Rechnung). Einschwingen auf 1 % nach ≈ 0,5 ms. Die Firmware misst alle 250 ms und der GL5528 selbst ist um Grössenordnungen träger (Datenblatt `gl5528b.pdf` hier nicht gelesen, ○): **die Messung wird nicht langsamer.**
  Netzbrummen (100 Hz) und Flackern der LED-Kette liegen **unter** der Grenzfrequenz und werden **nicht** gefiltert; Störspitzen im 10-kHz-Bereich schon.
- **Auf die ADC-Wandlung:** Der Kondensator ist ein Ladungsspeicher für die Abtastkapazität. Beim F411 würde der Ladungsausgleich mit 7 pF nur **21 µV** verschieben (300 mV · 7 pF / 100 nF; Rechnung), also 0,03 Zähler. Die S3-Abtastkapazität ist nicht angegeben (offen), die Herstellerempfehlung 100 nF gilt als Messbedingung der Genauigkeitswerte ✔ DS-C S. 66; HDG S. 18 (PDF 21).
- **V2 hat keinen solchen Kondensator** (Netz `LDR` = H5.2, H7.5, R22.2, U2.15 ✔ `nets.json`); **„C32“ kommt im V2-Netz nicht vor** (gesucht: `ref="C32"` fehlt in `v2.xml`) — die Bezeichnung gehört zur neuen Platine. Der Kondensator **ändert die Funktion nicht**, er fügt nur den Tiefpass (1,6 … 1,75 kHz) hinzu. Das ist **eine Ergänzung**, die den Messwert nicht ändert; ob sie unter „100 % kompatibel“ fällt, entscheidet der Nutzer.
- **Ladestrom des Teilers** (R22 = 1 kΩ): 300 µA bei hell, ohne Einfluss auf die Messung.

#### (5) Ist die Lesung funktional gleichwertig zu V2?

**Ja, für den Betrieb der Helligkeitsautomatik; nein im strengen Sinn zählergleicher Messwerte.** Begründung:

1. **Ja:** Alle drei Zustände liegen bei 0 dB im Bereich (≤ 300 mV von 850 mV, ✔ S. 66). Die Firmware verwendet nur Verhältnisse zu `ldr_min_value`/`ldr_max_value`, 32 Stufen, Änderung um 1 Stufe je Messung. Eine Abweichung von **±6 Zählern (S3) bzw. ±5 Zählern (F411 Worst Case)** bei ≈ 11,5 Zählern je Stufe liegt **in derselben Grössenordnung** (≈ ±0,5 Stufe).
   Der Dunkelwert wird auf beiden Wandlern nicht aufgelöst und von `ldr.c` geklammert.
2. **Nein, nicht zählergleich:** (a) **Der S3 misst absolut, der F411 ratiometrisch.** Die Schienentoleranz geht beim S3 voll in den Zähler ein (±2 % → ±4 … ±7 Zähler). (b) Der kalibrierte Fehler des S3 (±5 mV = ±6,2 Zähler) ist **grösser als der typische des F411 (±2 Zähler)**. (c) **Rauschen und Linearität unterhalb von 5 mV sind beim S3 nicht angegeben**, beim F411 ist das Rauschen ebenfalls nicht beziffert (✔ gelesen, nicht gefunden).
   (d) Mit WLAN-Betrieb gelten die Datenblattwerte des S3 **nicht** (✔ S. 66); die Genauigkeit unter Sendelast ist unbekannt.
3. **Was dazu nötig ist (ohne Hardwareänderung):** ATTEN0 (0 dB), **eFuse-Kalibrierung benutzen**, Mittelung über ≥ 16 Abtastungen, Abbildung **N_STM = V_mV · 4095 / 3300** und Klammerung wie bisher. Dann bleiben die gespeicherten Grenzen auf **±1 Stufe** gültig. Eine **einmalige Nachkalibrierung** durch den Nutzer (Min/Max neu setzen) stellt zählergleiche Werte auf dem S3 her; das ist ein Schritt, kein Hardwareumbau.

**Was fehlt, und welche Abweichung wäre minimal — die Entscheidung trifft der Nutzer, nicht der Analyst:**

| Lücke | Minimale Massnahme | Eingriff in „100 % V2-kompatibel“ |
|---|---|---|
| S3 misst absolut (Schiene geht in den Zähler) | **Keine Hardware:** Schiene als 3300 mV annehmen; bei Bedarf Nachkalibrierung durch den Nutzer. Alternativ die 3V3-Schiene über einen **zweiten ADC1-Pin** (Teiler 2 × 100 kΩ, 0,1 µF) messen und N_STM damit ratiometrisch rechnen | keine / ein zusätzlicher Pin (nicht V2) |
| Grösserer Fehler des S3 (±5 mV) | Mittelung, Kalibrierung, Nachkalibrierung | keine |
| Rauschen/untere Linearität unbekannt | **Messung am DevKit** (Teiler 1 kΩ + Widerstand/Poti), dort die Streuung bei WLAN an/aus bestimmen — dafür ist kein Platinenentwurf nötig | keine |
| Dunkelwert 3,3 mV nicht auflösbar | in V2 ebenso nicht auflösbar (F411: ±5 Z.); keine Massnahme | keine |
| (nur falls Auflösung unzureichend) R22 grösser | 4,7 kΩ / 6 dB hielte alles in ±10 mV bei 15 mV Dunkelwert (6.2) | **ändert V2-Spannungen — nicht freigegeben** |

#### Offene Punkte zu 6.3

- Rauschen des S3-ADC (mit und ohne WLAN) und seine Linearität unter 5 mV: **Messung am DevKit**, keine Unterlage nennt sie.
- Toleranz der 3,3-V-Schiene der V3 (bestimmt den Schienenfehler in Zählern).
- Die tatsächlich gespeicherten `ldr_min_value`/`ldr_max_value` des Nutzers: nicht bekannt, die Beispielzahlen stammen aus `HARDWARE.md`.
- ST-Datenblatt nur als Archivkopie gelesen (Server nicht erreichbar); bei Bedarf gegen den Originalserver prüfen.

## 7. USB-Serial/JTAG (Frage 5)

- **Pins:** GPIO19 = **D−**, GPIO20 = **D+** ✔ DS-M S. 11; HDG S. 18 (PDF 21); IDF-Tabelle ●. Integrierter PHY, „no or very few external components“ ✔ DS-M S. 23.
- **Funktionen:** CDC-ACM-Konsole, **Flashen mit esptool** (`idf.py flash -p PORT`), **JTAG** gleichzeitig zur seriellen Verbindung (OpenOCD) ● IDF-Konsolenseite; ✔ DS-M S. 22–23; TRM §33.2. Festfunktions-Gerät, nicht umkonfigurierbar ● IDF.
- **Reset und Download per DTR/RTS** über das CDC-ACM-Gerät, **ohne Strapping-Beschaltung**: Setzen/Löschen von RTS und DTR setzt ein Download-Flag und löst den Reset aus ✔ TRM Tab. 33.3-2 bis 33.4-4, S. 1247–1254 (esptool macht das). Abschaltbar über eFuses ✔ S. 1248.
- **ROM-Download manuell:** GPIO0 = 0 und GPIO46 = 0 beim Reset, also **GPIO0 nach GND, dann EN kurz auf Low** ✔ DS-M Tab. 4-3, S. 14; HDG S. 35 (PDF 38).
- **Vorwiderstände:** HDG empfiehlt **Platz für Reihenwiderstände (Startwert 22/33 Ω) und Kondensatoren nach Masse (zunächst unbestückt), dicht am Chip** ✔ HDG S. 18 (PDF 21). Das Referenzschema des Moduls zeigt **0 Ω** (R4, R6) und unbestückte C5/C6 ✔ DS-M Abb. 9-1, S. 41; das **DevKit hat gar keine** (nur ESD-Dioden und eine Schottky-Diode auf VBUS) ✔ DK-Sch Blatt 2. Differentielle Leitung 90 Ω ± 10 %, durchgehende Bezugsmasse ✔ HDG S. 30 (PDF 33).
- **D+ beim Einschalten:** „D+ schwankt zwischen High und Low; der High-Pegel ist stark und braucht einen kräftigen Pull-down“ — für einen stabilen Anfangszustand empfiehlt das HDG einen **externen Pull-up** ✔ HDG S. 19 (PDF 22). Das deckt sich mit den 3,2 ms/2 ms-Glitches (4.2). Internes D+-Pull-up ist aktiv (Reset-Zustand USB_PU, schaltbar 1,1 kΩ/2,2 kΩ) ✔ DS-C S. 18 Fussn. 6; TRM Reg. 33.3, S. 1257.
- **VBUS-Erkennung:** Der Chip hat **keinen VBUS-Pin**, und das TRM-Kapitel 33 beschreibt **keine** VBUS-Erkennung für den Serial/JTAG-Teil (Kapitel gelesen, S. 1243–1257). Das DevKit führt VBUS über eine Schottky-Diode nur auf die 5-V-Schiene, **nicht auf einen GPIO** ✔ DK-Sch Blatt 2. Für den OTG-Teil existiert ein VBUS-Valid-Signal über die GPIO-Matrix ✔ TRM S. 485 — für Serial/JTAG nicht nötig.
- **Wenn die Anwendung GPIO19/20 umkonfiguriert** (USB-PHY abschalten, andere USB-Funktion, UART/LEDC auf diese Pins): **die USB-Verbindung verschwindet, und das automatische Download-Verfahren entfällt**; dann hilft nur **GPIO0 auf Low und Reset von Hand** ✔ HDG S. 36 (PDF 39); ● IDF-Konsolenseite („USB Pin Reconfiguration“). HDG: „retain control of the strapping pins“. **Folgerung: BOOT-Taster (GPIO0) und EN-Taster (Reset) bleiben auf der Platine, GPIO19/20 dürfen nirgends sonst verwendet werden.**
- **Schlaf:** In Deep-Sleep verschwindet das Gerät vom Host; in Light-Sleep antwortet es nicht, ein Neustecken kann nötig sein ● IDF; ✔ TRM S. 1253 (APB_CLK ≥ 40 MHz für konforme Funktion).
- **OTG-Download ist beim Chip fehlerhaft** (Errata USBOTG-4289, alle Revisionen) ✔ Errata-Zusammenfassung — betrifft nicht den Serial/JTAG-Download.

## 8. Versorgung, EN, Antenne (Frage 6)

- **Strom:** Spitze beim WLAN-Senden **355 mA** (802.11b, 1 Mbit/s, +20,5 dBm, 100 % Sendeanteil), 297 mA bei 802.11g/54 Mbit/s, 286 mA bei HT20/MCS7; Empfang 95 mA ✔ DS-M Tab. 6-4, S. 28. Bluetooth-LE-Senden bis 344 mA ✔ Tab. 6-5. **Die Quelle muss ≥ 0,5 A liefern** ✔ S. 27.
  Für Module mit PSRAM kann der Verbrauch höher liegen ✔ S. 29. Modem-Sleep 13 … 92 mA je nach Takt und Last ✔ Tab. 6-6, S. 29.
- **Entkopplung am Modul:** Referenzschema **22 µF + 0,1 µF** an VDD33 ✔ DS-M Abb. 9-1 (C1, C3), S. 41. HDG zur Chipversorgung: **≥ 10 µF am Platineneingang** mit ESD-Diode, **10 µF je VDD3P3-Pin**, 0,1 µF an VDD3P3_CPU/VDD3P3_RTC ✔ HDG S. 5–8 (PDF 8–11) — das gilt für den Chip; im Modul ist das erledigt, am Modul genügt die Referenz von Abb. 9-1.
- **EN-Beschaltung:** **R = 10 kΩ, C = 1 µF (RC-Verzögerung)** ✔ DS-M S. 41; ✔ HDG S. 8 (PDF 11); das DevKit verwendet R5 = 10 kΩ und C6 = 1 µF, dazu Reset-Taster ✔ DK-Sch Blatt 2. **EN nie schweben lassen** ✔ DS-M S. 11. Schwellen: Low ≤ 0,25 · VDD, High ≥ 0,75 · VDD ✔ S. 27–28; tSTBL ≥ 50 µs, tRST ≥ 50 µs ✔ Tab. 4-6, S. 16.
  Rechnung: 10 kΩ · 1 µF = 10 ms; 0,75 · 3,3 V erreicht EN nach ln 4 · τ ≈ 13,9 ms (selbst gerechnet).
  Bei langsam steigender oder wackliger Versorgung genügt das RC-Glied nicht; HDG empfiehlt dort einen **Spannungswächter mit Schwelle ≈ 3,0 V** ✔ HDG S. 8–9 (PDF 11–12).
- **EPAD:** Löten an Masse nicht Pflicht, verbessert die Wärmeabfuhr; richtige Pastenmenge, sonst heben sich die Randpads ✔ DS-M S. 41.
- **Antenne und Platzierung am Rand:** Modulantenne **ausserhalb der Platine** oder, wenn nicht möglich, der Speisepunkt **so nah wie möglich an der Platinenkante**, Platine beidseits und darunter ausschneiden; das Modul **nicht in die Platinenmitte** setzen ✔ HDG S. 30 (PDF 33), Abb. S. 31 (PDF 34).
  Freihaltefläche: **6 mm Tiefe** über die Antennenbreite, seitlich **mindestens 15 mm** frei auf der Seite der Aussparung, höchstens 2 mm bzw. 1 mm auf der Gegenseite ✔ HDG Abb. 26, S. 31 (PDF 34); DS-M Abb. 11-1, S. 45 („Antenna Area“ 6 mm, Modul 25,5 mm lang).
  Im Gehäuse **mindestens 15 mm Abstand in alle Richtungen** ✔ HDG S. 30. USB-Leitungen, UART-Leitungen und Testpunkte **weit weg von der Antenne** ✔ HDG S. 28 (PDF 31). Zwei-Lagen-Platinen gelten als möglich, „no official two-layer modules“ ✔ S. 22.
- **Landemuster:** 40 Randpads 1,5 × 0,9 mm, EPAD mit Via-Gitter ✔ DS-M Abb. 11-1, S. 45.

## 9. Konsole (Frage 7)

- **UART0 = GPIO43 (TXD0) und GPIO44 (RXD0)**; im Reset WPU+IE ✔ DS-M S. 12; DS-C S. 17. HDG: **499 Ω in Reihe in U0TXD** gegen Oberwellen, Leitung kurz und mit Masse umgeben ✔ HDG S. 13 (PDF 16), S. 28 (PDF 31).
- **ROM-Meldungen** gehen beim Start standardmässig **gleichzeitig auf UART0 und USB-Serial/JTAG** ✔ DS-M S. 15; TRM §8.3, S. 536–537. Abschaltbar per Register/eFuse (GPIO46 wirkt mit).
- **Anwendung — beide Kanäle gleichzeitig:** In ESP-IDF wählt `CONFIG_ESP_CONSOLE_UART_DEFAULT` (UART0) als Haupt- und `CONFIG_ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG` als Nebenausgabe. **Die Nebenausgabe ist nur Ausgabe, nur nicht blockierend, ohne REPL und erscheint nur, wenn UART0 „nicht verbunden“ ist**;
  Eingabe oder blockierende REPL gibt es nur, wenn USB-Serial/JTAG als **Hauptkonsole** gewählt ist (`CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG`) ● IDF Kconfig-Referenz, „Channel for console secondary output“. Echte Gleichzeitigkeit mit Eingabe auf beiden Kanälen liefert diese Konfiguration **nicht**;
  dafür müsste die Firmware selbst UART0 und `usb_serial_jtag` getrennt bedienen (Treiber `usb_serial_jtag`, ● IDF-Beispiel `usb_serial_jtag_echo`). Welches Framework das Projekt verwendet (ESP-IDF oder Arduino-ESP32), steht in diesen Unterlagen nicht: offen.
- **Am DevKit** läuft die Konsole über die USB-UART-Brücke an UART0, am eigenen Board über das native USB: **dieselbe Firmware muss also beide Wege bedienen**, und die Boot-Meldungen erscheinen ohnehin auf beiden.
- Datenpufferung der USB-Konsole: Nach vollem Puffer wartet der Chip einmalig 50 ms, wenn kein Terminal liest ● IDF — eine kurze Pause im Programm, kein Absturz.

## 10. Fertigung bei JLCPCB (Prüfdatum **17.10.2026**)

Quelle: `jlcpcb.com/partdetail/<Nr>` (Klasse, Montageart), `lcsc.com/product-detail/<Nr>.html` (Gehäuse, Lagerbestand), JLCPCB-Teilesuche (Lagerbestand, abgerufen 17.10.2026).

| Teil | LCSC/JLCPCB-Nr. | Klasse | Lager JLCPCB | Lager LCSC | Gehäuse | Montage |
|---|---|---|---|---|---|---|
| **ESP32-S3-WROOM-1-N8** (gewählt) | **C2913198** | **Extended** | **416** | 416 | SMD, 25,5 × 18 mm | SMT, „PCBA Type: Standard Only“, MSL 3 |
| ESP32-S3-WROOM-1-N8R2 | C2913204 | Extended | 712 | 712 | gleich | gleich |
| ESP32-S3-WROOM-1-N8R8 | C2913201 | Extended | 4934 | **0** (Abweichung) | gleich | gleich; Octal-PSRAM, GPIO35–37 belegt, 65 °C |
| ESP32-S3-WROOM-1-N4 | C2913197 | Extended | 1942 | nicht geprüft | gleich | — |
| ESP32-S3-WROOM-1-N16 | C2913199 | Extended | 1439 | nicht geprüft | gleich | — |
| ESP32-S3-WROOM-1-N16R8 | C2913202 | Extended | 32836 | nicht geprüft | gleich | — |
| ESP32-S3-WROOM-1U-N8 (andere Bauform 19,2 mm) | C2980297 | Extended | 896 | nicht geprüft | SMD, 19,2 × 18 mm | — |

- **Klasse:** In der Suche „ESP32-S3-WROOM-1“ erschien **keine** Basic- oder Preferred-Zeile; alle Module sind **Extended** (Feld `componentLibraryType = expand`, `preferredComponentFlag = false`; Teileseite zeigt „Extended“). **Grund für Extended:** ein Funkmodul gibt es nicht im Standard-Katalog; eine Basic-Alternative, die die Aufgabe erfüllt, wurde nicht gefunden.
  Eine Extended-Nummer kostet bei JLCPCB je **zusätzlicher Bauteilart** eine Rüstgebühr (Bestellseite, ○ nicht geprüft).
- **Lagerbestand N8: nur 416** — gering gegenüber N8R8 (4934) und N16R8 (32836). Für eine Kleinserie reicht das; für Nachbestellung prüfen. **N8R2 (712) ist pinkompatibel und lässt alle GPIOs frei** — aber der Chip **ESP32-S3R2 ist End-of-Life** (DS-C S. 1 und S. 13, Tab. 1-1), der Nachfolger heisst ESP32-S3RH2; die Modulnummer führt das DS-M v1.8 ohne EOL-Vermerk. Das ist ein Beschaffungsrisiko für N8R2, nicht für N8.
- **Abweichung der Bestände:** LCSC-Seite meldet für C2913201 Lager 0, die JLCPCB-Suche 4934. Massgebend für die Bestückung ist der JLCPCB-Bestand (Bestückungslager), die LCSC-Zahl gehört zum LCSC-Versand.
- **SMT-bestückbar:** ja (JLCPCB: „SMT Assembly“); **nur im Standard-PCBA-Verfahren** („Standard Only“), also **nicht im Economic-Verfahren**. MSL 3: Backen/Lagerfenster beachten ✔ jlcpcb.com Teileseite; Reflow-Profil DS-M S. 47.
- **Alternativen mit Nummer (gleiches Landemuster 18 × 25,5 mm):** C2913197 (N4, 4 MB Flash — kleiner als der heutige ESP-Flash-Bedarf nicht geprüft, ○), C2913199 (N16), C2913204 (N8R2).

## 11. Fallen und Errata

1. **GPIO45 am N8 nie hoch** (Flash-Spannung 1,8 V statt 3,3 V) ✔ DS-M S. 39.
2. **GPIO19/20-High-Glitch 3,2 ms/2 ms** und **60-µs-Low-Glitch auf GPIO1–18** beim Einschalten — jeder Schaltausgang braucht einen externen Pull und darf bei Low-aktiv nicht dort liegen ✔ (4.2).
3. **GPIO3 ohne internen Pull** — schwebend; wirkt nur, wenn EFUSE_STRAP_JTAG_SEL gebrannt ist ✔ DS-M S. 15–16.
4. **GPIO39 (MTCK) hat im Reset einen Pull-up**, solange EFUSE_DIS_PAD_JTAG = 0 ✔ DS-C S. 17, Fussn. 7.
5. **USB-Pins umkonfiguriert → Gerät verschwindet** ✔ HDG S. 36; deshalb Strapping-Zugriff behalten.
6. **Octal-PSRAM-Module sperren GPIO35–37**; die DevKits mit N8R8 verhalten sich so ✔ (3.3).
7. **ADC2 und WLAN** teilen sich den ADC: ADC1 verwenden ✔ TRM S. 1474–1475.
8. **Errata (nur Zusammenfassung gelesen, Einzelbeschreibungen nicht):** CACHE-126, RTC-126, ANALOG-160 („Chip wird beschädigt bei BIAS_SLEEP = 0 und PD_CUR = 1“, alle Revisionen), LCD-239, USBOTG-4289, RMT-176, TOUCH-100, ADC-183.
   ANALOG-160 ist eine Registerkombination der Software, nicht der Beschaltung — wer Sleep-Modi selbst konfiguriert, liest die Einzelbeschreibung.
9. Touch-Sensor hat den Conducted-Susceptibility-Test nicht bestanden ✔ DS-M S. 26 — für uns ohne Belang, solange kein Touch genutzt wird.

## 12. Folgerung für unsere Platine

- **Modul:** ESP32-S3-WROOM-1-N8 (C2913198, Extended, 416 Stück am 17.10.2026), alle 36 GPIOs frei; DevKit-gleiche Pins nur aus der Menge **{0, 1, 2, 3, 4–18, 19, 20, 21, 38–42, 43, 44, 45, 46}** wählen, **nie GPIO35–37**, möglichst nicht 47/48.
- **USB:** GPIO19/20 ausschliesslich USB; Platz für 22/33-Ω-Reihenwiderstände und Kondensatoren dicht am Modul; ESD-Schutz am Stecker; **BOOT-Taster an GPIO0 und EN-Taster behalten** (Rückweg, falls die Anwendung USB abschaltet).
- **EN:** 10 kΩ nach 3V3 und 1 µF nach GND, Taster nach GND; **GPIO0 mit Pull-up** (kein grosser Kondensator); **GPIO45 und GPIO46 ohne starken Pull-up beim Start**.
- **Versorgung:** Quelle ≥ 0,5 A; 22 µF + 0,1 µF am Modul, 10 µF am Platineneingang; Spannungswächter bei wackliger Versorgung.
- **LDR:** ADC1-Pin (GPIO1, 2 oder 4–10, nicht GPIO3), 0,1 µF am Pin, **R22 = 1 kΩ (Vorgabe des Nutzers vom 17.10.2026, V2-kompatibel)**, ADC mit **0 dB (ATTEN0, 0 … 850 mV)**, kalibriert, mit Mehrfachabtastung, Abbildung auf die STM-Skala nach 6.3. Die frühere Empfehlung 10 kΩ ist zurückgezogen.
- **5-V-Gate der LED-Kette:** Pin mit hochohmigem Reset, externem Pull auf Aus und ohne Glitch: **GPIO21** (oder GPIO38/40–42/45–48 mit den genannten Einschränkungen).
- **Antenne:** Modul am Platinenrand mit der Antenne **über die Kante hinaus** oder Speisepunkt an der Kante und Aussparung, 6 mm tief, 15 mm seitlich frei; USB- und UART-Leitungen fern von der Antenne.
- **Konsole:** Firmware muss UART0 und USB-Serial/JTAG bedienen (am DevKit UART0, am Board USB).

## 13. Widersprüche und Auffälligkeiten zwischen den Unterlagen

1. **Boot-Modi:** DS-M/DS-C/HDG kennen nur zwei Modi (SPI-Boot, Joint-Download) aus GPIO0/GPIO46; **das TRM führt einen dritten (SPI-Download, GPIO0 = 0, GPIO46 = 1, GPIO1 = 1, GPIO2 = 0)** und nennt GPIO1/GPIO2 für den Bootmodus (TRM Tab. 8.2-1, S. 535). GPIO1/2 sind im Reset hochohmig (TRM S. 535) — **GPIO1/GPIO2 nicht beim Start auf 1/0 ziehen, wenn GPIO0 = 0 und GPIO46 = 1.**
2. **ADC-Vollausschlag:** Das TRM nennt nominal k ≈ 25 % bei 12 dB, also rechnerisch 4,4 V Vollausschlag (S. 1469); das DS-C begrenzt den **nutzbaren** Bereich auf 0 … 2900 mV (S. 66). Massgebend für uns: der Bereich des DS-C. Zudem 1100 mV „by design“ (TRM) gegen 1000 … 1200 mV je Chip (IDF).
3. **USB-Vorwiderstände:** HDG empfiehlt 22/33 Ω; das Referenzschema des Moduls zeigt 0 Ω; das DevKit hat keine. Drei Antworten auf dieselbe Frage.
4. **HDG-Revision:** Titelseite „Release master, 12.10.2026“, aber die eigene Revisionstabelle endet bei v1.8 vom 2025-07-03 — der Stand ist ein laufender Master, keine feste Version.
5. **Absolute Maximalwerte:** DS-M nennt nur VDD33 und Lagertemperatur 105 °C, DS-C zusätzlich die Summe der Ausgangsströme und 150 °C; **kein Dokument nennt Grenzwerte je Einzelpin.**
6. **USB-Wortwahl:** DK schreibt „USB 1.1“ für den ESP-USB-Port, die Datenblätter „USB 2.0 Full-Speed“ — sachlich dasselbe (12 Mbit/s), unterschiedliche Bezeichnung.
7. **Lagerbestand N8R8:** LCSC-Seite 0, JLCPCB-Suche 4934 (siehe 10).

## 14. Offene Punkte

- Zulässige **Quellimpedanz** und Verhalten des ADC-Eingangs (Abtastkondensator, Rückwirkung) — in keiner Unterlage angegeben; ggf. Anfrage an Espressif oder Messung am DevKit.
- **Nichtlinearität des ADC unterhalb von ≈ 50 mV** — nicht angegeben; Messung am DevKit (GPIO an GND über 1 MΩ-Teiler) wäre belastbar.
- Welches **Framework** läuft (ESP-IDF oder Arduino-ESP32) — bestimmt die Konsolenkonfiguration (9).
- **Einzelbeschreibungen der Errata** (ANALOG-160, RTC-126, CACHE-126) nicht gelesen.
- **Echte Messwerte des LDR** (1 MΩ / 20 kΩ / 10 kΩ sind Annahmen); Datenblatt GL5528 nicht ausgewertet.
- **Pegelwandler 74AHCT1G125 mit 3,3-V-Ausgang des ESP32-S3** — Eingangsschwelle im Wandler-Datenblatt prüfen (liegt unter `datenblaetter/sn74ahct1g125.pdf`, hier nicht gelesen).
- JLCPCB: **Lagerbestand und Rüstgebühr** der Extended-Nummer zum Bestelltermin erneut prüfen; Basic-Alternative nicht vorhanden.
- GPIO-Zustand **bei EN = Low** (während des Resets) ist nicht ausdrücklich beschrieben; die Aussagen oben gelten für Reset-Zustand nach Loslassen von EN und für den Einschaltvorgang.
