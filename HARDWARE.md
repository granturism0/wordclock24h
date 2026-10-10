# Hardware — WordClock USB-C / STM32F411 V2

Ausgelesen aus dem KiCad-Projekt `WordClock-USB C - STM32F411 - V2` (Schaltplan, PCB,
Stückliste) und **gegen den Firmware-Code abgeglichen**. Jede Pinangabe ist an beiden
Enden belegt: Netzname aus dem PCB, Makro aus der Firmware.

Das Board ist eine Eigenentwicklung und **kein BlackPill-Modul mit Zusatzplatine**. Es
ist aber bewusst **pinkompatibel zur BlackPill** entworfen — deshalb passt die
Firmware-Konfiguration `BLACKPILL_BOARD` ohne Änderung.

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

Unbeschaltet: PC14, PC15, PA1, PA6, PA8, PA11, PA12, PA15, PB9, PB12–PB15.

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
externe 5 V ──┘                                      └─▶ SY8301 (U10) ─▶ VCC_3.3V
```

Abgesichert mit `F1` (SMD1206-150-16) im USB-Zweig und `F2` (1812L300/24GR, 3 A) im
externen Zweig, dazu TVS-Dioden `D4`/`D5` (SMAJ5.0CA).

**USB-C ist reine Stromversorgung** — ein 6-poliger Typ-C-Verbinder mit `R3`/`R4`
(je 5,1 kΩ) als CC-Widerstände. Keine Datenleitungen zum STM32.

## Was hier nicht geprüft ist

- Die Leistungsreserve für lange LED-Ketten — `Q1` und die Zuleitungsquerschnitte sind
  nicht gegen die Stromaufnahme der bestückten Matrix gerechnet.
- Die Bestückungsvarianten in `WordClock-LED_Board_Thomas/` — eigenes Unterprojekt, nicht
  angesehen.
- Entprellung und Pull-ups der Taster im Detail.
- Die Leistungsfähigkeit des DCF77-Empfangs. Ausgewertet **wird** er: `src/dcf77/`
  existiert, 88 Fundstellen über `src/**`.
