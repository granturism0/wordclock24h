# Hardware — WordClock USB-C / STM32F411 V2

Ausgelesen aus dem KiCad-Projekt `WordClock-USB C - STM32F411 - V2` (Schaltplan, PCB,
Stückliste) und **gegen den Firmware-Code abgeglichen**. Jede Pinangabe ist an beiden
Enden belegt: Netzname aus dem PCB, Makro aus der Firmware.

Das Board ist eine Eigenentwicklung und **kein BlackPill-Modul mit Zusatzplatine**. Es
ist aber bewusst **pinkompatibel zur BlackPill** entworfen — deshalb passt die
Firmware-Konfiguration `BLACKPILL_BOARD` ohne Änderung.

**Massgebend ist die Netzliste, nicht dieser Text (seit 17.10.2026).** Bis 17.10.2026 stand
hier, jede Pinangabe sei „an beiden Enden belegt: Netzname aus dem PCB, Makro aus der
Firmware“. **Das stimmt nur für die Pinzuordnung des STM32.** Die Beschaltung drumherum
stammte aus einer Abschrift, und als der Schaltplan am 17.10.2026 mit `kicad-cli` als
Netzliste exportiert wurde (`hardware/esp32s3/v2/v2.xml`, erzeugt von
`hardware/esp32s3/export_v2.py`), stimmten mehrere Aussagen nicht oder waren unvollständig.
Jede Berichtigung steht an ihrer Stelle unten, mit Datum und Fundstelle. Wer einen Pin oder
ein Bauteil belegen will, liest das Netz aus dem Export — die Fundstellen unten lassen sich
mit einem Blick in `v2.xml` (`<net name="…">`, `<node ref="…" pin="…">`) wiederholen.
Die Analyse der Einzelteile (mit Datenblattseiten) steht in `hardware/komponenten/`.

## Die Bausteine

| Ref | Bauteil | Rolle |
|---|---|---|
| U2 | **STM32F411CEU6** (QFN-48) | Hauptrechner, 84 MHz aus 25-MHz-Quarz |
| U4 | **ESP-12F** | WLAN, fest auf der Platine |
| U3 | **SN74AHCT1G125** | **Pegelwandler 3,3 V → 5 V** für die LED-Datenleitung |
| U5 | **DS3231MZ** | Echtzeituhr mit TCXO, Pufferbatterie U7 (CR2032) |
| U6 | **AT24C32M** | **EEPROM am I2C**, 4 KByte — der STM32F411 hat keins |
| U1 | **DS18B20** | Temperatursensor, 1-Wire |
| U9 | TPS2121 | Power-Mux zwischen USB-C und externer 5-V-Einspeisung |
| U10 | SY8301 | Abwärtswandler auf 3,3 V |
| Q1 | SI7149DP (P-MOSFET) | schaltet die **5 V der LED-Kette** |
| Q2 | MMBF170 | treibt das Gate von Q1 |
| X1 | 25-MHz-Quarz | HSE für den STM32 |

## Pinbelegung des STM32 — Schaltplan gegen Firmware

| Pin | Netz im Schaltplan | Firmware | Stimmt |
|---|---|---|---|
| PB1 | `WS2812_BUS` | `SK6812_GPIO_PIN = GPIO_Pin_1`, `TIM3->CCR4`, `GPIO_AF_TIM3` | ✔ |
| PB0 | `WS2812_POWER` | `POWER_PORT GPIOB`, `POWER_PIN GPIO_Pin_0` | ✔ |
| PB5 | `DS18B20` | `ONE_WIRE_PIN GPIO_Pin_5`, `ONE_WIRE_PORT GPIOB` | ✔ |
| PA5 | `LDR` | `ADC_PORT GPIOA`, `ADC_PIN GPIO_Pin_5`, `ADC_Channel_5` | ✔ |
| PB6 / PB7 | `SCL_DS3231` / `SDA_DS3231` | I2C für RTC **und** EEPROM | ✔ |
| PA2 / PA3 | `RXD_ESP` / `TXD_ESP` | Kommandobrücke zum ESP | ✔ |
| PA9 / PA10 | `RX_UART` / `TX_UART` | Debug-UART am Stecker H6 | — |
| PB4 | `TSOP` | IR-Empfänger | — |
| PB8 | `DCF77` | DCF77-Empfänger, ausgewertet in `src/dcf77/` | — |
| PA0 / PA7 | `USER` / `WPS` | Taster SW2 / SW1 | — |
| PC13 | `LED_STM` | Status-LED | — |
| PA13 / PA14 | `DIO_STM` / `DCLK_STM` | SWD am Stecker H3 | — |
| PA4, PB10, PB3 | `GPIO0_ESP`, `RST_ESP`, `ESP_EN` | STM steuert den ESP | — |
| PH0 / PH1 | `XTALA` / `XTALB` | 25-MHz-Quarz ⇒ `HSE_VALUE == 25000000` ⇒ 84 MHz | ✔ |

Unbeschaltet: PC14, PC15, PA1, PA6, PA8, PA11, PA12, PA15, PB9, PB12–PB15. (Am 17.10.2026
gegen den Export nachgezählt: genau diese 13 Pins stehen dort als `unconnected-(U2-…)`.)

> **Berichtigt 17.10.2026 — IR-Empfänger und DCF77 haben auf dem Controller keinen
> Stecker.** Bis 17.10.2026 stand hier in der Tabelle `PB4` / `TSOP` als „IR-Empfänger“ und
> `PB8` / `DCF77` als „DCF77-Empfänger“, und der Schluss dieser Datei sprach von der
> „Leistungsfähigkeit des DCF77-Empfangs“ — als sässe an beiden Pins ein Empfänger auf der
> Platine. **Das stimmt nicht mehr, weil** der Netzlistenexport für beide Netze genau zwei
> Teilnehmer liefert: `TSOP` = `R8.1` und `U2.40` (PB4), `DCF77` = `R9.1` und `U2.45` (PB8).
> `R8` und `R9` (je 100 kΩ) ziehen die Leitungen nach `VCC_3.3V`. Auf keinem Stecker liegt
> eines der beiden Netze (`H3` bis `H7` geprüft). Der Controller hat damit **nur die Pull-ups
> und den Controller-Pin**, aber **keinen Anschluss** für einen IR- oder DCF77-Empfänger. Die
> Firmware wertet beide Eingänge trotzdem aus (`src/dcf77/`, `src/irmp/`); wie ein Empfänger
> an die Platine kommt, ist aus dem Schaltplan **nicht** ablesbar. Beleg: `v2.xml`, Netze
> `TSOP` und `DCF77`.

> **Ergänzt 17.10.2026 — DS3231: `SQW` und `32K` liegen nicht am Controller.** Die
> Pinbelegung oben nennt die RTC nur über den I2C-Bus, und das ist richtig. Dazu gehört aber:
> `INT/SQW` (`U5.3`) und `32KHZ` (`U5.1`) sind Open-Drain-Ausgänge und hängen **nur** an je
> einem Pull-up des Widerstandsarrays `RN2` (`RN2.5` bzw. `RN2.6`, 4,7 kΩ nach 3,3 V) —
> **an keinem Controller-Pin.** Die beiden anderen Elemente von `RN2` sind die I2C-Pull-ups
> (`SCL_DS3231` an `RN2.7`, `SDA_DS3231` an `RN2.8`). Das zweite Array `RN1` zieht die drei
> Adresspins `E0`…`E2` des EEPROMs (`U6.1`–`U6.3`) auf 3,3 V; die Firmware findet das EEPROM
> deshalb nicht auf `0xA0`, sondern über ihre Suchschleife auf `0xAE`. Beleg: Netze
> `SQW_DS3231`, `32K_DS3231`, `Net-(U6-E0)` bis `Net-(U6-E2)`; Auswertung in
> `hardware/komponenten/ds3231mz.md` und `hardware/komponenten/at24c32.md`. (Die Paarung der
> Array-Elemente 2↔7, 3↔6, 4↔5 ist dort als Annahme geführt, das Datenblatt des Arrays nicht
> gelesen.)

## Der Pegelwandler — das 3,3-V-Problem ist gelöst

```
PB1 ──▶ U3.A (SN74AHCT1G125) ──▶ U3.Y ──▶ R11 330Ω ──▶ TO_SK6812_IN ──▶ H4 / H7
                    ▲
              U3.VCC ← R13 (0Ω) ← TO_VCC_RGB_5V
```

Der **SN74AHCT1G125 hat TTL-Eingangspegel**: Er erkennt ein 3,3-V-Signal sicher, obwohl
er selbst mit 5 V versorgt wird, und gibt saubere 5 V aus. Genau das braucht eine
5-V-versorgte SK6812-Kette.

**Die Notiz in `README.md`, dass 3,3 V direkt vom STM32 grenzwertig sei, gilt für die
ältere Bestückung ohne dieses Board.** Auf V2 ist der Pegelwandler bestückt.

Zwei Feinheiten:

- `U3.VCC` kann per 0-Ω-Brücke an der **geschalteten** LED-Spannung (R13) oder an der
  permanenten 5 V (R17) hängen. In der Stückliste ist **nur R13** bestückt — der
  Pegelwandler wird also mit den LEDs zusammen stromlos. Das ist stimmig: Ist die Kette
  aus, soll auch kein Pegel anliegen.
- `R31` (100 kΩ) hält `TO_SK6812_IN` auf definiertem Pegel, solange U3 stromlos ist.

## Die LED-Kette lässt sich hart abschalten

`PB0` → `R19` (82 Ω, Gate-Widerstand) → `Q2` (MMBF170) → Gate von `Q1` (P-MOSFET) →
`TO_VCC_RGB_5V`.

Die Firmware nutzt das in `src/power/power.c` (`power_on()` / `power_off()`). Der
`delay_msec(200)` nach `power_on()` in `display.c:2937` ist also keine Willkür, sondern
die Einschwingzeit der LED-Versorgung.

> **Berichtigt 17.10.2026 — die Schaltkette hat fünf Teile mehr, als hier stand.** Bis
> 17.10.2026 nannte dieser Abschnitt nur `R19`, `Q2` und `Q1`. **Das war unvollständig, und wo
> daraus gelesen wurde, ein Gate-Pull-down an `Q2` fehle, war es falsch:** `R20` existiert.
> Der Export zeigt (Netze `WS2812_POWER`, `Net-(Q2-G)`, `Net-(Q2-D)`, `Net-(C10-Pad2)`):
>
> | Teil | Anschluss | Rolle |
> |---|---|---|
> | `R19` 82 Ω | `PB0` → Gate von `Q2` | Gate-Vorwiderstand |
> | `R20` 100 kΩ | Gate von `Q2` → `GND` | **Gate-Pull-down**: Ohne Treiber sperrt `Q2` |
> | `Q2` MMBF170 | Source `GND`, Drain `Net-(Q2-D)` | N-FET, zieht seinen Drain nach Masse |
> | `R21` 3,3 kΩ | `VCC_5V` → Drain von `Q2` | Pull-up des Drain-Knotens |
> | `R23` 2,2 kΩ | Drain von `Q2` → Gate von `Q1` | Gate-Vorwiderstand von `Q1` |
> | `C10` 100 nF | Gate von `Q1` → `VCC_5V` | bildet mit `R23` das Einschalt-Glied (weicher Anlauf) |
> | `Q1` SI7149DP | Source `VCC_5V`, Drain `TO_VCC_RGB_5V` | P-FET, schaltet die LED-Versorgung |
> | `JP2` | `VCC_5V` ↔ `TO_VCC_RGB_5V` | offene Lötbrücke **parallel** zu `Q1` |
>
> `PB0` hoch ⇒ `Q2` leitet ⇒ das Gate von `Q1` liegt unter der Quelle ⇒ die LED-Kette hat
> 5 V. `PB0` niedrig oder hochohmig ⇒ `R20` sperrt `Q2`, `R21` + `R23` halten das Gate von
> `Q1` auf `VCC_5V` ⇒ die Kette ist aus. **`R23` und `C10` sind kein Spannungsteiler** — das
> Gate nimmt im Gleichstrom keinen Strom auf. Aus `R23` × `C10` folgt eine Einschaltflanke von
> grob 0,2 bis 0,5 ms (Rechnung, ○, `hardware/komponenten/si7149dp.md`). **Wichtig für die
> Fertigung:** `R23` und `C10` fehlen in `production/bom.csv` des V2-Projekts, obwohl der
> Schaltplan sie als bestückt führt (`BEFUNDE.md`, L485). Fehlt `R23` auf der Platine, ist das
> Gate von `Q1` von `PB0` nicht mehr steuerbar, und die Kette liesse sich nicht schalten. Ob sie
> dann dauernd aus ist oder (über `JP2` oder ein schwebendes Gate) dauernd Strom bekommt, ist
> nicht belegt. Ob die Teile auf der Platine sitzen, prüft der Nutzer am Gerät.

## Beide Rechner können einander zurücksetzen

| Richtung | Weg |
|---|---|
| ESP setzt STM zurück | ESP `GPIO14` → `RESET` → STM `NRST` (auch Taster SW3) |
| ESP flasht STM | ESP `GPIO4` → `R18` 100 kΩ → `BOOT0`, dann Reset |
| STM steuert ESP | `PA4` → ESP `GPIO0`, `PB10` → ESP `RST`, `PB3` → ESP `EN` |

**Abweichung im ESP-Code, harmlos:** `stm32flash.cpp:22` bedient zusätzlich `GPIO5` als
zweiten BOOT0-Pin — ein Zugeständnis an ältere Shields („Bug in Shield v3"). Auf diesem
Board ist `GPIO5` **unbeschaltet**. Der Flash-Pfad funktioniert über `GPIO4`.

## Warum die ESP-Debugzeilen auf der STM-UART landen

Der ESP-12F hat nur einen vollwertigen UART, und der ist die **Kommandobrücke zum STM**
(`GPIO1`/`GPIO3` ↔ `PA2`/`PA3`). `ESP-uclock.ino:145` macht `Serial.begin(115200)` auf
genau diesem.

⇒ **Jede `Serial.print`-Debugausgabe des ESP geht an den STM32.** Das ist keine
Nachlässigkeit im Code, sondern eine Folge der Verdrahtung. Damit ist Kernbefund 4 aus
`REVIEW.md` — rund 100 Byte Debugtext pro HTTP-Request auf der STM-UART, gegen einen
256-Byte-Ringpuffer, der still verwirft — hardwareseitig bestätigt.

Der Debug-UART des **STM** ist ein anderer: `PA9`/`PA10` (USART1) am Stecker `H6`, mit
Schiebeschalter `SW4` in der RX-Leitung.

> **Berichtigt 17.10.2026 — `SW4` liegt nicht nur an der Konsole, sondern auch am ESP.** Bis
> 17.10.2026 stand hier `SW4` als Schalter „in der RX-Leitung“ der STM-Konsole, und der
> Abschnitt davor liess den ESP nur über `GPIO1`/`GPIO3` an den STM. **Das stimmt nicht mehr,
> weil** der Export zeigt (Netze `RX_UART`, `TX_UART`, `GPIO15_ESP`, `Net-(H6-Pad2)`):
>
> | Netz | Teilnehmer |
> |---|---|
> | `RX_UART` | `H6.3`, `PA9` (STM, USART1-TX) **und** `GPIO13` des ESP — fest verbunden |
> | `TX_UART` | `PA10` (STM, USART1-RX) und `SW4.2` |
> | `GPIO15_ESP` | `GPIO15` des ESP, `R14` (10 kΩ nach `GND`) und `SW4.1` |
> | `Net-(H6-Pad2)` | `H6.2` und `SW4.3` |
>
> `SW4` (SSSS811101) ist ein Schiebeschalter mit drei Schaltkontakten; seine Pins 4 bis 7 sind
> das Gehäuse an `GND`. **Der ESP hat damit einen zweiten UART-Weg zum STM**, den vertauschten
> UART über `GPIO13`/`GPIO15` — und genau den benutzt `stm32flash.cpp` (Zeile 1356:
> `Serial.swap ()`, „GPIO15 is now TX, GPIO13 is now RX“) zum Flashen des STM. Damit der
> ESP-Sendeweg `GPIO15` den STM-Empfang `PA10` erreicht, muss `SW4` in einer Stellung `SW4.1`
> mit `SW4.2` verbinden — in der anderen verbindet er `PA10` mit dem Konsolenstecker (`H6.2`).
> **Welcher Pin der gemeinsame Kontakt ist, ist nicht belegt:** Der Schluss „Mittelkontakt
> `SW4.2` gemeinsam“ ist aus dem Flash-Weg gefolgert (sonst erreichte `GPIO15` den STM nie),
> das Datenblatt des Schalters (LCSC C109335) liegt nicht im Repository und wurde nicht
> gelesen (○). `H6.3` — die Sendeleitung des STM — hört der ESP in beiden Stellungen mit.
> Die Aussage „jede `Serial.print`-Debugausgabe des ESP geht an den STM32“ bleibt für den
> normalen UART (`GPIO1`/`GPIO3`) richtig. Beleg: `v2.xml`, `ESP8266/ESP-uclock/stm32flash.cpp:1356`;
> die offene Frage führt `BEFUNDE.md`, L486 (`E43`).

## Das EEPROM ist I2C, nicht Flash

`src/eeprom/eeprom.c` nutzt `EEPROM_FIRST_ADDR 0xA0` und `i2c_read`/`i2c_write` mit
16-Bit-Adressierung. Das ist der **AT24C32M** auf dem I2C-Bus, zusammen mit der RTC.

Die teure Grösse beim Schreiben ist der **Schreibzyklus des I2C-EEPROMs**, nicht die
Flash-Programmierung des STM32. Die oft zitierten 16 ms sind nur der Wartezyklus
(`EEPROM_WAITSTATES 15`). Am Gerät gemessen kostet ein Zyklus **rund 24 ms**: dazu kommen
rund 8 ms I2C-Zeit, weil der Treiber in `i2c_wait_for_flags()` je nicht sofort gesetztem
Flag `delay_msec(1)` wartet, statt zu pollen (`BEFUNDE.md`, L358 und L359).

`eeprom_write()` schreibt **seitenweise** (Befund C3, L7). Die Seitengrösse ist eine
Build-Konstante je Ziel, `EEPROM_PAGE_SIZE` in `src/eeprom/eeprom.c`: 32 Byte auf dem
F411, 8 Byte auf dem F103. Je Seite liest die Firmware zuerst zum Vergleich, schreibt
höchstens einmal vom ersten bis zum letzten geänderten Byte und wartet dann einen Zyklus ab.
Unveränderte Seiten überspringt sie. Bis zu dieser Änderung schrieb sie byteweise, ein
Zyklus je geändertem Byte. Die Umstellung kommt mit dem nächsten STM-Release; ihre Wirkung
am Gerät bestätigt die Messung G3, die noch aussteht.

---

# Das LED-Board

Eigenes KiCad-Projekt unter `~/Documents/WordClock-LED_Board`. Verbunden mit dem
Controller über den siebenpoligen Stecker.

## Bestückung

| Ref | Bauteil | Anzahl |
|---|---|---|
| V1–V114 | **SKC6812RGBW-BW**, OPSCO Optoelectronics, LCSC `C5181320` | **114** |
| C1–C115 | 100 nF | 115 — eine Abblockung je LED |
| C116 | Elko 6,3 V | 1 |
| R1 | **GL5528 (LDR)** | 1 — der Helligkeitssensor sitzt **hier**, nicht auf dem Controller |
| R4 | 220 Ω | in Serie zur ersten LED |
| R5, R6, R7 | 0 Ω | Brücken in der Datenkette bei V4/V5, V48/V49, V92/V93 |
| U1 | Stecker 7-polig | zum Controller |

**114 LEDs = 110 Matrix + 4 Minutenpunkte**, und das passt exakt zur Firmware:
`WC_ROWS 10 × WC_COLUMNS 11 = 110` (`wclock24h-config.h:19`) plus `DSP_MINUTE_LEDS 4`
und `DSP_DISPLAY_LEDS 110` (`display-config.h:113`).

## SKC6812 ist nicht SK6812 — und die Firmware weiss das

Verbaut ist die **SKC**-Variante von OPSCO, nicht die gewöhnliche SK6812. Die
Unterschiede sind klein, aber sie betreffen genau das Protokoll:

| | Wert laut Hersteller |
|---|---|
| Versorgung | 3,5–5,5 V |
| Ruhestrom je LED | **0,29 mA** |
| Datenrate | 800 kbit/s |
| PWM-Frequenz | 4 kHz, fest |
| Gehäuse | SMD5050-4P, 1,6 mm hoch |

**Der kritische Unterschied ist die Reset-Pause.** `sk6812.c` führt zwei Timing-Sätze,
und der Kommentar dort benennt es ausdrücklich:

```c
#if 0 // only usable for SK6812
#define SK6812_PAUSE_TIME  100000   // should be longer than 80us for SK6812, should be 200us for SKC6812
#else // usable for SK6812 and SK6812C
#define SK6812_PAUSE_TIME  250000   // ... should be longer than 200us for SKC6812
#endif
```

Aktiv ist der **zweite** Satz: Periode 1250 ns, `T0H` 300 ns, `T1H` 750 ns, Pause
**250 µs**. Das passt zu den 800 kbit/s und liegt über den 200 µs, die die SKC-Variante
verlangt.

**Fallstrick:** Wer den `#if 0` umdreht, bekommt 100 µs Pause. Das genügt einer SK6812,
**nicht** einer SKC6812 — die Folge wären sporadische Anzeigefehler, die wie ein
Wackelkontakt aussehen. Der Zweig bleibt zu.

Ein Refresh kostet damit 114 × 32 bit × 1,25 µs ≈ **4,6 ms** plus 250 µs Pause.

## Der Stecker ist gespiegelt

| | Controller `H7` | LED-Board `U1` |
|---:|---|---|
| 1 | 5 V | GND |
| 2 | 5 V | GND |
| 3 | 3,3 V | LDR |
| 4 | SK6812-Daten | SK6812-Daten |
| 5 | LDR | 3,3 V |
| 6 | GND | 5 V |
| 7 | GND | 5 V |

**Controller-Pin n gehört an LED-Board-Pin (8 − n).** Die Belegung passt nur
spiegelverkehrt zusammen — bei gegenüberliegend montierten Steckern und geradem
Flachbandkabel ergibt sich das von selbst. Bei einzeln gecrimpten Adern führt ein
1:1-Kabel dagegen 5 V auf GND.

## Signalwege zwischen den Platinen

**Daten:** `PB1` → `U3` (Pegelwandler) → `R11` 330 Ω → Stecker → `R4` 220 Ω → `V1.DIN`.

Beide Serienwiderstände liegen hintereinander, zusammen **550 Ω**. Das ist am oberen
Ende des Üblichen (100–470 Ω). Für das 800-kHz-Protokoll zählt die Flankensteilheit an
der ersten LED; mit längerem Kabel wächst die Last und die Reserve schrumpft. Nicht
gemessen, nur gerechnet.

**LDR:** `GL5528` auf dem LED-Board hängt an 3,3 V, der Abgriff geht über den Stecker
zu `PA5` und dort über `R22` (1 kΩ) nach Masse. Der ADC misst also am Mittelpunkt eines
Teilers, dessen oberer Zweig der LDR ist:

| Umgebung | LDR grob | Spannung an PA5 | von 4095 Schritten |
|---|---|---|---|
| dunkel | 1 MΩ | 3 mV | ~4 |
| Zimmerlicht | 20 kΩ | 157 mV | ~195 |
| hell | 10 kΩ | 300 mV | ~373 |

> **Nachgeprüft 17.10.2026 — dieser Teiler stimmt, und es gibt einen zweiten Anschluss.** Der
> Export bestätigt die Aussage oben: `R22` ist 1 kΩ, `R22.1` liegt an `GND`, `R22.2` am Netz
> `LDR` (mit `PA5`, `H7.5` und `H5.2`); der LDR hängt über `H7.3` an `VCC_3.3V`. Neu ist
> nur, dass der Controller neben `H7` einen eigenen zweipoligen LDR-Stecker `H5` hat (`H5.1` an
> `VCC_3.3V`, `H5.2` am Netz `LDR`), **parallel** zu den Pins 3 und 5 von `H7`. Die Tabelle
> darüber (3 mV bis 300 mV) ist nachgerechnet und stimmt. Im Umbau auf den ESP32-S3
> ändert sich `R22` (`specs/esp32s3-port/`, E9); das betrifft diese Beschreibung der V2 nicht.

Im Normalbetrieb wird damit weniger als ein Zehntel des ADC-Bereichs genutzt. Das ist
handhabbar, weil `ldr.c:69` gegen die kalibrierten Grenzen `ldr_min_value` und
`ldr_max_value` aus dem EEPROM normiert — die Auflösung bleibt aber begrenzt.

**Das Kettenende ist offen:** `V114.DOUT` ist nicht herausgeführt. Ein Ambilight-Streifen
lässt sich an diesem Board also **nicht in Reihe anschliessen**. Der zweite Stecker `H4`
am Controller liegt **parallel** zu `H7` auf demselben Datensignal — was dort hängt,
bekäme die Daten ab LED 1, also die Uhrzeitanzeige, nicht die Ambilight-Daten ab
Position 114. Die Firmware reserviert dafür Platz (`DSP_AMBILIGHT_LEDS 120`), die
tatsächliche Zahl steht im EEPROM.

## Stromaufnahme — die Rechnung, nicht gemessen

Belegt ist der **Ruhestrom von 0,29 mA je LED**. Den Strom je Farbkanal nennt das
Datenblatt auf der Vertriebsseite nicht; die folgende Rechnung setzt die für
5050-RGBW übliche Grössenordnung von rund 20 mA je Kanal an. **Diese Zahl ist eine
Annahme, kein Herstellerwert** — für die Grössenordnung reicht sie, für eine Auslegung
nicht.

| Fall | Strom |
|---|---|
| alle 114 aus (nur die internen Controller) | **0,033 A**, belegt |
| 40 LEDs weiss über den W-Kanal | ~0,8 A, angenommen |
| 40 LEDs weiss über R+G+B | ~2,4 A, angenommen |
| alle 114 auf allen vier Kanälen voll | ~9,1 A, angenommen |

Dem stehen die beiden Rückstellsicherungen gegenüber: **`F1` mit 1,5 A Haltestrom im
USB-C-Zweig** und `F2` mit 3,0 A im Zweig der externen Einspeisung.

**Über USB-C wird es bei heller Anzeige eng.** Eine Rückstellsicherung löst nicht hart
aus, sondern erhöht bei Erwärmung allmählich ihren Widerstand — die Spannung sackt,
statt dass etwas abschaltet. Das ist ein Verhalten, das zu „läuft meistens, hängt
gelegentlich" passt.

**Das ist eine Hypothese aus einer Rechnung, keine Messung**, und sie steht neben den
belegten Software-Ursachen aus `REVIEW.md`, sie ersetzt sie nicht. Prüfbar wäre sie
ohne Codeänderung: Spannung an `VCC_5V` (Testpunkt `TP7`) unter heller Anzeige messen,
oder beobachten, ob die Hänger bei dunkler Anzeige seltener auftreten. Siehe Befund L9
in `BEFUNDE.md`.

## Unstimmigkeit in der Stückliste

`C116` ist in `production/bom.csv` mit **680 µF** geführt, im PCB steht als Wert
**100 µF**. Die Stückliste stammt vom 12. März, das PCB wurde am 12. April zuletzt
geändert — vermutlich wurde der Wert danach angepasst und die Stückliste nicht neu
erzeugt. Für den Stützkondensator der LED-Versorgung ist der Unterschied nicht egal.

---

## Stromversorgung

```
USB-C (USB1) ─┐
              ├─▶ TPS2121 (U9, Power-Mux) ─▶ VCC_5V ─┬─▶ Q1 ─▶ TO_VCC_RGB_5V (LED-Kette)
externe 5 V ──┘                                      └─▶ SY8301 (U10) ─▶ D2 ─▶ VCC_3.3V
```

Abgesichert mit `F1` (SMD1206-150-16) im USB-Zweig und `F2` (1812L300/24GR, 3 A) im
externen Zweig, dazu TVS-Dioden `D4`/`D5` (SMAJ5.0CA).

> **Berichtigt 17.10.2026 — das Bild oben war zu grob.** Bis 17.10.2026 führte die Skizze den
> SY8301 direkt auf `VCC_3.3V`, ohne `D2`, ohne `Q3` und ohne die beiden Brücken, und die
> Stecker `H6` und `H3` standen nur als Debug-UART und SWD. **Das stimmt nicht mehr, weil** der
> Export (Netze `Net-(D2-A)`, `Net-(D1-A)`, `Net-(D3-A)`, `VCC_3.3V`, `ALT_5V_PROT`,
> `Net-(F2-Pad2)`) zeigt:
>
> - **`D2` (PMEG2010ER) sitzt zwischen dem Wandler und `VCC_3.3V`, hinter der Pufferkapazität
>   des Wandlers.** Spule `L2` und die Kondensatoren `C27`, `C28`, `C29` hängen am Netz vor der
>   Diode (`Net-(D2-A)`), alle übrigen Verbraucher und `C8` dahinter. Die Rückkopplung `FB`
>   (`R28`/`R29`) misst hinter der Diode; der Spannungsfall der Diode wird damit herausgeregelt.
> - **`H6` und `H3` speisen das 3,3-V-Netz über Dioden ein:** `H6.1` über `D3`, `H3.1` über `D1`
>   (Anode am Stecker, Kathode an `VCC_3.3V`). Wer an `H3` einen Programmierer oder an `H6`
>   einen USB-UART-Adapter mit 3,3-V-Ausgang ansteckt, kann das Netz mitspeisen; `D2` verhindert,
>   dass diese Spannung in den Wandlerausgang zurückfliesst.
> - **`Q3` (SI7149DP) liegt im externen Zweig:** Drain am Netz hinter `F2` (`Net-(F2-Pad2)`),
>   Source an `ALT_5V_PROT` (Eingang `IN2` des TPS2121), Gate über `R24` (100 Ω) und `R25`
>   (100 kΩ nach `GND`) mit `C2` (10 nF) zur Source — ein Verpolschutz mit weichem Anlauf. Die
>   Aufgabe von `Q3` ist aus der Beschaltung gelesen und im Komponentenblatt
>   `hardware/komponenten/tps2121.md` beschrieben, **nicht** am Gerät gemessen.
> - **`JP3`** (offene Lötbrücke) verbindet `USB_5V_PROT` mit `VCC_5V` und umgeht den TPS2121 auf
>   der USB-Seite; **`JP2`** umgeht `Q1` (siehe „Die LED-Kette lässt sich hart abschalten“).
>   Beide sind in der V2-Stückliste nicht geführt (`in_bom = nein`).
> - **Strombudget:** Die Stromgrenze des TPS2121 setzt `R26` (22,1 kΩ) auf rund 4,5 A
>   (Mindestwert 4,0 A, Datenblatt S. 8). Gegen die im Umbau angesetzten 3 A LED plus 0,6 A
>   Logik (Annahmen der Komponentenblätter, nicht gemessen) bleiben damit gegen den
>   **Mindestwert** nur 10 % Reserve. `F1` (1,5 A) und `F2` (3 A) sind nach ihrer Bezeichnung
>   kleiner als diese 3,6 A — gelesen aus dem Teilenamen, Datenblätter nicht geprüft (○). Die
>   offenen Punkte stehen in `BEFUNDE.md`, L489.

**USB-C ist reine Stromversorgung** — ein 6-poliger Typ-C-Verbinder mit `R3`/`R4`
(je 5,1 kΩ) als CC-Widerstände. Keine Datenleitungen zum STM32.

## Unstimmigkeiten in der Stückliste des Controllers (V2), 17.10.2026

Verglichen: `production/bom.csv` und `production/positions.csv` des V2-Projekts gegen die
Bauteile des Schaltplans (Export `v2.xml`) und gegen die Eigenschaften `in_bom` und `dnp` im
Schaltplan. Der Schaltplan führt 109 Bauteile, die Stückliste 92 Referenzen in 50 Zeilen; die
17 fehlenden sind:

| Gruppe | Referenzen | Erklärt durch |
|---|---|---|
| Befestigung, Prüfpunkte, Lötbrücken, externe Einspeisepads | `H1`, `H2`, `H8`, `H9`, `H10`, `TP1`, `TP4`–`TP8`, `JP2`, `JP3` | `in_bom = nein` im Schaltplan |
| Bewusst unbestückte 0-Ω-Brücken | `R7`, `R17` | `dnp = ja` |
| **ohne Erklärung** | **`R23`, `C10`** | `in_bom = ja`, `dnp = nein` — und trotzdem weder in `bom.csv` noch in `positions.csv` |

`R23` und `C10` gehören zum Einschaltweg der LED-Versorgung (siehe oben); die Lücke hat einen
Schaltplan-Eintrag, aber keine Fertigungsdaten. Dazu kommen drei Widersprüche zwischen
Stückliste und Teil (Einzelheiten in `hardware/komponenten/jlcpcb-bestand.md`):

- **`R29`** trägt in Schaltplan und Stückliste 32 kΩ, das bestellte Teil C2930156 ist
  31,6 kΩ. Die Rückkopplung des SY8301 liefert damit 3,33 V statt 3,30 V — beides innerhalb der
  Grenzen des ESP32-S3 (`hardware/komponenten/sy8301abc.md`).
- **`D2`, `D3`** (und `D1`) heissen in der Stückliste Footprint `SOD-123`, das Teil PMEG2010ER
  (C82288) ist `SOD-123W`. Der **Schaltplan** trägt den richtigen Footprint
  (`Nexperia_CFP3_SOD-123W`); der Widerspruch liegt nur in der Stückliste.
- **`Q2`** ist in der Stückliste „MMBF170“ mit C130879 — das ist der Nachbau von Shikues, nicht
  der bekannte Typ von Diodes; die Schwellenspannung weicht ab (`BEFUNDE.md`, L488).

Die fehlenden Teile führt `BEFUNDE.md` als L485, die Widersprüche von `R29`, `D1`–`D3` als L487 und `Q2` als L488.

## Was hier nicht geprüft ist

- Die Leistungsreserve für lange LED-Ketten — `Q1` und die Zuleitungsquerschnitte sind
  nicht gegen die Stromaufnahme der bestückten Matrix gerechnet.
- Die Bestückungsvarianten in `WordClock-LED_Board_Thomas/` — eigenes Unterprojekt, nicht
  angesehen.
- Entprellung und Pull-ups der Taster im Detail.
- Die Leistungsfähigkeit des DCF77-Empfangs — und schon vorher, ob überhaupt ein Empfänger
  angeschlossen werden kann: Auf dem Controller gibt es dafür keinen Stecker (siehe oben,
  „Berichtigt 17.10.2026 — IR-Empfänger und DCF77“). Ausgewertet **wird** der Eingang:
  `src/dcf77/` existiert, 88 Fundstellen über `src/**`.
