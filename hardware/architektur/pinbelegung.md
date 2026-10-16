# Pinbelegung der Controller-Platine (ESP32-S3-WROOM-1-N8)

Stand: **17.10.2026**. Schreiber: `hw-architect`. Lebendes Dokument (DIR-006): Zahlen tragen Datum und Fundstelle.
Die Gründe zu den Wahlen stehen in `entscheidungen.md` (Kennungen `AE-nn`); dieses Dokument nennt sie nur.

**Beleg-Zeichen:** ✔ gelesen (Quelle, Seite), ● Sekundärquelle, ○ Annahme oder eigene Rechnung.
Datenblattwerte stammen aus den Komponentenblättern (`hardware/komponenten/esp32s3-wroom-1.md`, im Folgenden **Blatt**); die
V2-Pins aus der V2-Netzliste (`hardware/esp32s3/v2/nets.json` und `v2.xml`).

## 1. Regeln, nach denen gewählt wurde

1. **Nur GPIOs, die das DevKitC-1 am Header herausführt** (Blatt §3.2: alle 36 Modul-GPIOs liegen auf J1/J3).
2. **Gesperrt für Signale:** GPIO0 (nur BOOT-Taster), 3, 45, 46 (Strapping), 19/20 (USB), 43/44 (UART0, nur Konsole),
   35–37 (Flash/PSRAM der R8-Varianten, das heutige DevKit ist N8R8), 38 und 48 (RGB-LED des DevKits), Vorgabe des Nutzers vom 17.10.2026.
3. **Kein 5-V-Netz an einem Modulpin.** Jeder GPIO gilt als höchstens 3,6 V (VIH max = VDD + 0,3 V ✔ Blatt §5; ein Grenzwert je Pin steht in keiner Unterlage).
4. **LDR nur an ADC1** (GPIO1–10; ADC2 kollidiert mit WLAN ✔ Blatt §6.1).
5. **LED-Ansteuerung, Power-Schalten und LDR-Lesung sind pin- und funktionskompatibel zu V2** (Vorgabe): Die Netze `WS2812_BUS`, `WS2812_POWER`
   und `LDR` und alles zwischen Controller-Pin und `H7` bleiben wie in V2; nur der Controller-Pin wechselt (Prüfung H8).
6. Ein Netzname aus V2 bleibt, solange das Signal dasselbe ist. Neue Netze sind als **neu** gekennzeichnet.

## 2. Die elf Signale (V2-Controller → S3)

Reset-Zustand: Blatt §4.2 (✔ DS-C Tab. 2-1, S. 16–17; Tab. 2-2, S. 18–19). „hochohmig“ heisst: weder Pull-up noch Pull-down aktiv.
**Beim Einschalten geben GPIO1–18 rund 60 µs lang Low aus** (GPIO18 zusätzlich 60 µs High); danach sind sie hochohmig, bis die Software sie treibt.

| # | Signal | Netzname (V2) | V2-Pin (STM32) | **GPIO** | Modulpad | S3-Funktion | DevKit | Reset / Einschalten | Beschaltung auf der Platine | Grund | Beleg |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | LDR-Abgriff | `LDR` | PA5 (U2.15) | **1** | 39 | ADC1 (GPIO1–10) | J3 | hochohmig; 60-µs-Low-Glitch | `R22` 1 kΩ nach GND (V2-Wert), `C32` 100 nF nach GND (Antrag A1), `H7.5`, `H5.2` | ADC1-Pin, kein Strapping. Das DevKit führt ihn heraus. Der dritte Boot-Modus (GPIO1 = 1, GPIO2 = 0) verlangt zugleich GPIO0 = 0 und GPIO46 = 1; GPIO46 ist offen und damit Low, der Fall tritt nicht ein | ✔ Blatt §3.1, §4.1, §6.1, §13.1 |
| 2 | DS18B20 (1-Wire) | `DS18B20` | PB5 (U2.41) | **2** | 38 | Ein-/Ausgang, offener Kollektor | J3 | hochohmig; 60-µs-Low-Glitch (ein Reset-Impuls unter 480 µs, harmlos) | `R5` 4,7 kΩ nach 3,3 V, `U1.2` | Der dritte Boot-Modus verlangt GPIO2 = 0 nur zusammen mit GPIO0 = 0 und GPIO46 = 1; der Pull-up hält GPIO2 High, der Fall tritt nicht ein | ✔ Blatt §4.1, §13.1; ✔ DS18B20-Blatt §4 |
| 3 | LED-Daten | `WS2812_BUS` | PB1 (U2.19) | **5** | 5 | Ausgang (RMT) | J1 | hochohmig; 60-µs-Low-Glitch | nur `U3.2` (Pegelwandler), nichts sonst | frei, kein Strapping; die RMT-Ausgabe läuft über die GPIO-Matrix auf jeden freien Pin (○ Treiberfrage, nicht gelesen) | ✔ Blatt §3.1 |
| 4 | LED-Versorgung | `WS2812_POWER` | PB0 (U2.18) | **6** | 6 | Ausgang (Push-Pull), High = LED-Versorgung an | J1 | hochohmig, **kein interner Pull**; 60-µs-Low-Glitch | `R19` 82 Ω → Gate `Q2`; `R20` 100 kΩ Gate → GND | N-Kanal-`Q2` mit Pull-down: Low-Glitch und hochohmig schalten nichts ein (`AE-06`). GPIO21 wäre ohne Glitch, bringt für diese Topologie nichts | ✔ Blatt §4.2; ✔ MMBF170-Blatt §3.1 |
| 5 | IR-Empfänger | `TSOP` | PB4 (U2.40) | **7** | 7 | Eingang | J1 | hochohmig; 60-µs-Low-Glitch | `R8` 100 kΩ nach 3,3 V; **kein Stecker** (wie V2, `AE-18`) | frei, kein Strapping | ✔ V2 `nets.json` |
| 6 | I²C-Daten | `SDA_DS3231` | PB7 (U2.43) | **8** | 12 | I²C-Master | J1 | hochohmig; 60-µs-Low-Glitch | `RN2` 4,7 kΩ nach 3,3 V; `U5.7`, `U6.5` | frei, kein Strapping. Das Low-Glitch fällt in die Einschaltrücksetzung von RTC (läuft mit 3,3 V hoch) und EEPROM (Einschalt-Reset 100 µs ✔ AT24C32-Blatt §4); die Firmware gibt dem Bus ohnehin neun Takte frei (✔ REVIEW `i2c`) | ✔ Blatt §4.2; ✔ AT24C32-Blatt §4 |
| 7 | I²C-Takt | `SCL_DS3231` | PB6 (U2.42) | **9** | 17 | I²C-Master | J1 | wie SDA | `RN2` 4,7 kΩ nach 3,3 V; `U5.8`, `U6.6` | wie SDA | wie SDA |
| 8 | DCF77 | `DCF77` | PB8 (U2.45) | **15** | 8 | Eingang | J1 | hochohmig; 60-µs-Low-Glitch | `R9` 100 kΩ nach 3,3 V; **kein Stecker** (`AE-18`) | frei. GPIO15/16 sind ab Werk die Anschlüsse eines 32-kHz-Quarzes; ohne Quarz frei (auf Platine und DevKit kein Quarz) | ✔ Blatt §3.1, Spec `design.md` §2 |
| 9 | Taster USER | `USER` | PA0 (U2.10) | **16** | 9 | Eingang, interner Pull-up | J1 | hochohmig; 60-µs-Low-Glitch | `R1` 330 Ω in Reihe zu `SW2`; **kein externer Pull-up** (wie V2, `AE-07`) | frei, wie GPIO15 | ✔ V2 `nets.json` |
| 10 | Taster WPS | `WPS` | PA7 (U2.17) | **17** | 10 | Eingang, interner Pull-up | J1 | hochohmig; 60-µs-Low-Glitch | `SW1` direkt nach GND; kein externer Pull-up (`AE-07`) | frei, Treiberstärke 10 mA ohne Belang | ✔ V2; ✔ Blatt §2 |
| 11 | Status-LED | `LED_STM` | PC13 (U2.2) | **18** | 11 | Ausgang, **aktiv Low** (Kathode am Pin) | J1 | hochohmig; 60-µs-Low **und** 60-µs-High | `LED1` (grün) mit `R6` 510 Ω nach 3,3 V | frei. Ein Low-Glitch zündet die LED 60 µs lang, ein High-Glitch löscht sie: ohne Folgen. Treiberstärke 10 mA genügt (rund 2,5 mA ○, Vf nicht gelesen) | ✔ Blatt §2, §4.2; ○ Strom |

**Zahl der Signale: 11.** Keiner dieser Pins steht auf der Sperrliste (Regel 2); alle liegen am Header des DevKitC-1 v1.1 (J1: 5, 6, 7, 8, 9, 15, 16, 17, 18; J3: 1, 2; ✔ Blatt §3.2).

## 3. Feste Funktionen am Modul (keine Wahl)

| Modulpin | Pad | Funktion | Netz | Bemerkung |
|---|---|---|---|---|
| EN | 3 | Reset | `EN` (aus V2 `RESET` und `ESP_EN` vereint) | `R12` 10 kΩ nach 3,3 V, `C1` 1 µF nach GND, `SW3` nach GND (`AE-03`). Nie schwebend ✔ DS-M S. 11 |
| IO0 | 27 | BOOT (Strapping) | `BOOT` (**neu**) | `R32` 10 kΩ nach 3,3 V, `SW5` nach GND. **Keine andere Funktion**, kein Kondensator ✔ HDG S. 15 |
| IO19 | 13 | USB D− | `USB_DN` (Steckerseite), `USB_DN_M` (**neu**, Modulseite) | Reihenwiderstand 0 Ω zwischen beiden (`AE-02`) |
| IO20 | 14 | USB D+ | `USB_DP`, `USB_DP_M` (**neu**) | wie D−; Platz für Pull-up (unbestückt, `AE-02`) |
| TXD0 (IO43) | 37 | UART0 Senden | **`RX_UART`** (H6.3, V2-Sinn: „Empfang des Adapters“) | `R33` 510 Ω in Reihe (`AE-05`) |
| RXD0 (IO44) | 36 | UART0 Empfangen | **`TX_UART`** (H6.2, V2-Sinn: „Senden des Adapters“) | ohne Widerstand |
| 3V3 | 2 | Versorgung | `VCC_3.3V` | `C30` 22 µF und `C31` 100 nF an diesem Pad (Referenz 22 µF + 0,1 µF, ✔ DS-M Abb. 9-1, S. 41; `AE-08`) |
| GND | 1, 40, 41 (EPAD) | Masse | `GND` | EPAD löten, Pastenmenge nach DS-M S. 41 |

> **Berichtigung zum Schaltplan-Entwurf (Fehler, `AE-05`):** `hardware/esp32s3/gen.py` (Zeile 132) und `tools/checks/hardware-pinregeln.json`
> (`nur_an_netzen`) legen **TXD0 an `TX_UART` und RXD0 an `RX_UART`**. In V2 ist `TX_UART` der Eingang der Platine (`PA10` = USART1-Empfang, über `SW4` an `H6.2`) und `RX_UART`
> der Ausgang (`PA9` = USART1-Sendung, `H6.3`) — die Namen gelten aus Sicht des Adapters (✔ `HARDWARE.md`, Abschnitt „Debug-UART“; `nets.json`). Mit dem Entwurf träfe der **Ausgang des Moduls auf den Sendeanschluss des Adapters**
> und der Adapter-Empfang auf den Moduleingang: Die Konsole am Stecker `H6` ginge nicht, und ein V2-Kabel verbände zwei Ausgänge. Richtig: **TXD0 → `RX_UART` (H6.3), RXD0 ← `TX_UART` (H6.2).**

## 4. Offen gelassene und gesperrte Pins

Alle 36 GPIOs sind gezählt: 11 Signale (Abschnitt 2) + 5 feste (IO0, IO19, IO20, IO43, IO44) + 8 gesperrte + 12 frei = 36.

| Pin | Pad | Zustand | Grund | Beleg |
|---|---|---|---|---|
| IO3 | 15 | **offen** | Strapping (JTAG-Quelle), schwebend ohne internen Pull; wirkt nur, wenn `EFUSE_STRAP_JTAG_SEL` gebrannt ist (nicht gebrannt) | ✔ DS-M Tab. 4-5, S. 15–16 |
| IO45 | 26 | **offen** | Strapping VDD_SPI. **Beim Einschalten nicht High:** am N8 speiste High den Flash mit 1,8 V, das Modul liefe nicht an. Interner Pull-down hält ihn Low | ✔ DS-M S. 39 |
| IO46 | 16 | **offen** | Strapping Boot-Modus; muss für den UART-/USB-Download Low sein (interner Pull-down) | ✔ DS-M Tab. 4-3, S. 14 |
| IO35, IO36, IO37 | 28, 29, 30 | offen | Octal-PSRAM-Varianten sperren sie; das heutige DevKit ist N8R8 | ✔ Blatt §3.3 |
| IO38 | 31 | offen | RGB-LED des DevKitC-1 v1.1 | ✔ Blatt §3.2 |
| IO48 | 25 | offen | RGB-LED des DevKitC-1 v1.0; bei R16-Modulen 1,8 V | ✔ Blatt §3.2, §3.3 |
| IO4 (Pad 4), IO10 (18), IO11 (19), IO12 (20), IO13 (21), IO14 (22), IO21 (23), IO47 (24), IO39 (32), IO40 (33), IO41 (34), IO42 (35) | | **frei, unbelegt** (Reserve) | IO39–42 sind die Pad-JTAG-Pins und bleiben frei (JTAG läuft über USB); IO47 nicht als 3,3-V-Ausgang voraussetzen (bei R16-Modulen 1,8 V); IO39 hat im Reset einen Pull-up (`EFUSE_DIS_PAD_JTAG` = 0) | ✔ Blatt §3.3, §4.2 |

Die Reserve-Pins erhalten **keine** Stifte, Pads oder Testpunkte (1:1, kein neuer Zugang).

## 5. Vorschlag für `tools/checks/hardware-pinregeln.json` (schreibt der Lead)

Schlüssel `pinbelegung`: **Netzname → Modulpin**, so wie `hardware.py` (H3) es prüft (`sig in conn(p)`). Elf Einträge:

```json
"pinbelegung": {
  "LDR": "IO1",
  "DS18B20": "IO2",
  "WS2812_BUS": "IO5",
  "WS2812_POWER": "IO6",
  "TSOP": "IO7",
  "SDA_DS3231": "IO8",
  "SCL_DS3231": "IO9",
  "DCF77": "IO15",
  "USER": "IO16",
  "WPS": "IO17",
  "LED_STM": "IO18"
}
```

Vier Regeln in `nur_an_netzen` brauchen Korrektur, sonst schlägt H3 gegen den richtigen Entwurf an und lässt den falschen durch:

| Eintrag | steht jetzt | richtig |
|---|---|---|
| `TXD0` | `["TX_UART"]` | `["RX_UART"]` |
| `RXD0` | `["RX_UART"]` | `["TX_UART"]` |
| `USB_D+` | `["USB_DP"]` | `["USB_DP_M"]` (Modulseite hinter dem Reihenwiderstand, `AE-02`) |
| `USB_D-` | `["USB_DN"]` | `["USB_DN_M"]` |

Die übrigen Einträge von `nur_an_netzen` (`IO0` → `BOOT`, `EN`, `3V3`, `GND`) und die Regel `muss_offen_sein` (IO3, IO35–38, IO45, IO46, IO48) stimmen mit den Abschnitten 3 und 4 überein und bleiben.

## 6. Gegenprobe der Zuordnung gegen Sperrliste, Header und Pegel

| Prüfung | Ergebnis |
|---|---|
| Ein Signal auf einer Sperrnummer (0, 3, 45, 46, 19, 20, 43, 44, 35–37, 38, 48) | **keines** der elf (Abschnitt 2); 43/44 nur als UART0-Konsole (fest), 19/20 nur USB, 0 nur BOOT |
| Jedes der elf am DevKit-Header | ja, J1: 5, 6, 7, 8, 9, 15, 16, 17, 18; J3: 1, 2 (✔ Blatt §3.2) |
| Ein Signal mit Fremdbelegung am DevKit | keines (belegt dort: GPIO0 BOOT, EN Reset, GPIO38 RGB, 43/44 UART-Brücke, 19/20 zweite USB-Buchse) |
| Ein 5-V-Netz an einem Modulpin | keines; die Netze `VCC_5V`, `USB_5V_INPUT`, `USB_5V_PROT`, `ALT_5V_INPUT`, `ALT_5V_PROT`, `TO_VCC_RGB_5V` berühren kein `U2`-Pad (H3 prüft das) |
| Signal höher als 3,6 V an einem Modulpin möglich | nein, mit einer Ausnahme: `LDR` ≤ 3,3 V (Teiler an `VCC_3.3V`); `WS2812_BUS` ist ein Ausgang; das Gate von `Q2` wird vom Pin getrieben, die 5 V liegen am Drain-Zweig `R21`, nicht am Pin. **Ausnahme: der Konsoleneingang `H6.2` (GPIO44) bei einem Adapter mit 5-V-Pegel** (`entscheidungen.md` R1) |
| `LDR` an ADC1 | GPIO1 = ADC1 (✔ Blatt §6.1) |
| Strapping-Pins offen | IO3, IO45, IO46 offen; IO0 nur `BOOT`; die Pull-Beschaltung von IO0 (`R32`) ist die vom HDG empfohlene (✔ HDG S. 15) |
| Pegelwandler | `U3` (74AHCT1G125) erwartet VIH ≥ 2 V; das Modul liefert VOH ≥ 0,8 · VDD = 2,64 V bei 3,3 V und 2,4 V bei 3,0 V ⇒ Reserve ≥ 0,4 V (✔ AHCT-Blatt §5.1; ✔ Blatt §2) |

## 7. Belegung der Stecker (unverändert gegenüber V2, soweit nicht genannt)

| Stecker | Pins | Bemerkung |
|---|---|---|
| `H7` (7-polig, zum LED-Board, gespiegelt) | 1, 2 = `TO_VCC_RGB_5V`; 3 = `VCC_3.3V`; 4 = `TO_SK6812_IN`; 5 = `LDR`; 6, 7 = `GND` | **unverändert**; Controller-Pin n gehört an LED-Board-Pin 8 − n (✔ `HARDWARE.md`; `nets.json`) |
| `H4` (3-polig) | 1 = `TO_VCC_RGB_5V`, 2 = `TO_SK6812_IN`, 3 = `GND` | unverändert, parallel zu `H7` |
| `H5` (2-polig) | 1 = `VCC_3.3V`, 2 = `LDR` | unverändert; ein Verbraucher dort belastet den Abgriff (✔ Spec `design.md` §4.2 Punkt 9) |
| `H6` (4-polig, Konsole) | 1 = `Net-(D3-A)` (3,3-V-Einspeisung über `D3`), 2 = `TX_UART`, 3 = `RX_UART`, 4 = `GND` | UART0 des Moduls statt der STM-UART; `SW4` entfällt. **Pin 1 darf nur 3,3 V führen** (`entscheidungen.md`, R1) |
| `H3` (SWD) | entfällt mit dem STM32 | |
| `USB1` | 16-polig | `AE-02`, `AE-16` |

## 8. Aufträge aus diesem Dokument

| An | Auftrag |
|---|---|
| `hw-designer` | `TXD0`/`RXD0` in `gen.py` tauschen; Netze `USB_DP_M`/`USB_DN_M`, `R33`, die beiden 0-Ω-Brücken und die unbestückten Plätze anlegen (`AE-02`, `AE-05`); `expected.json` neu erzeugen |
| Lead | `hardware-pinregeln.json` nach Abschnitt 5 ändern; danach `tools/checks/hardware.py --nur H3` laufen lassen und **einmal absichtlich verletzen** (DIR-014) |
| `hw-reviewer` | die Tabelle aus Abschnitt 2 gegen die Netzliste prüfen (Mengen von `Ref.Pin`, Zahl der Netze nennen) |
