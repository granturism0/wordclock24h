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

Daraus folgt die oft zitierte Zahl von rund 16 ms pro Byte: Es ist der
**Schreibzyklus des I2C-EEPROMs** (`EEPROM_WAITSTATES 15`), nicht die Flash-Programmierung
des STM32. Siehe dazu den Befund L7 in `BEFUNDE.md` — der Baustein könnte 32 Byte in
einem einzigen Zyklus schreiben, die Firmware schreibt byteweise.

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
