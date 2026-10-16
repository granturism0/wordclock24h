# Änderungen gegenüber V2 — entfallen, neu, geändert

Stand: **17.10.2026**. Gegenstand: der Schaltplan `hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch`, erzeugt von
`hardware/esp32s3/gen.py` aus der V2-Netzliste (`hardware/esp32s3/v2/`). Die Liste ist aus **drei Quellen** zusammengesetzt und gegeneinander
gelesen: der Entfernungsliste im Generator (`REMOVE`), der Soll-Netzliste (`hardware/esp32s3/expected.json`) und dem V2-Export (`v2/v2.xml`).
Ein Vergleich beider Netzlisten auf Pin-Ebene (alle Pins ausser denen von `U2`, `U4`, `USB1`, `U11`) ergab **genau** die Unterschiede, die hier stehen.

**Zu jedem Eintrag** stehen ein Grund und ein Verweis auf den **Entscheid**. Ein Entscheid-Protokoll des Architekten
(`hardware/architektur/entscheidungen.md`) **gibt es noch nicht**; als Entscheid gelten daher die Festlegungen des Nutzers (E1 bis E12 in
`specs/esp32s3-port/requirements.md`, siehe [entscheidungsuebersicht.md](entscheidungsuebersicht.md)) und, wo darüber hinaus gezeichnet wurde, der Generator selbst. **Das ist eine Lücke, keine Regel** (OP-09).

Die Zahlen (Bauteile je Seite) zeigt der Lauf von `python3 tools/checks/hardware.py` (Prüfung H2) zum jeweils aktuellen Stand; hier stehen sie nur für den Zeitpunkt der Erstfassung:
V2 hat 109 Bauteile; V3 entsteht daraus mit 20 entfallenen und 8 neuen (Bezeichnungen, `U2` und `USB1` zählen auf beiden Seiten) und hat damit 97 Bauteile, wie H2 am 17.10.2026 bestätigt.

---

## 1. Entfallen (20 Referenzen)

Quelle: `REMOVE` in `gen.py`; Grund: der Funktionsträger entfällt, weil ein Chip beide Chips ersetzt (Entscheid E1, E8) oder weil das zugehörige Bauteil mit ihm entfällt.

| Ref | Was (V2) | Grund |
|---|---|---|
| **U2** | STM32F411CEU6 | ersetzt durch den ESP32-S3 (der neue `U2` trägt dieselbe Referenz) |
| **U4** | ESP-12F | ersetzt durch den ESP32-S3 |
| **X1** | Quarz 25 MHz (STM32-HSE) | der S3 braucht ihn nicht |
| **C3, C5** | Lastkondensatoren 8 pF des Quarzes (`XTALA`, `XTALB`) | gehören zu `X1` |
| **R7** | 1 MΩ am Quarz (`XTALA`/`XTALB`), **bestückungsfrei (`dnp`)** | gehört zu `X1`; stand ohnehin nicht in der Stückliste |
| **C9** | 2,2 µF am `VCAP1` des STM32 | STM32-spezifisch |
| **C11, C14, L1** | Filter der analogen Versorgung (`VDDA`) des STM32 | STM32-spezifisch; der ADC des S3 braucht kein solches Filter (Entscheid des Architekten nicht protokolliert) |
| **R2** | 10 kΩ Pull-up am Netz `RESET` | `RESET` und `ESP_EN` werden ein Netz `EN`; den Pull-up trägt `R12` |
| **R10, R18** | Boot-Widerstände des STM32 (`BOOT1`) und des ESP8266 (`GPIO4`/`BOOT0`) | STM32/ESP8266-spezifisch |
| **R14, R15, R16** | Boot-Beschaltung des ESP-12F (`GPIO15`, `GPIO2`, `GPIO4`) | ESP8266-spezifisch |
| **SW4** | Umschalter der Konsole `H6` (STM-UART oder zweite ESP-UART) | der S3 hat **eine** Konsole (UART0, `GPIO43/44`); die zweite UART entfällt |
| **H3** | SWD-Stecker des STM32 | JTAG läuft über den einen USB-C-Anschluss (E3, Muss (b)) |
| **D1** | Diode der 3,3-V-Einspeisung von `H3` | `H3` entfällt |
| **USB1** (6-polig) | USB-C-Buchse nur für Strom, C456012 | ersetzt durch eine 16-polige mit Datenleitungen (E3) |

Die 20 Namen sind die Liste aus `REMOVE`; ✔R: `U2, U4, X1, C3, C5, C9, C11, C14, L1, R7, R2, R10, R14, R15, R16, R18, SW4, H3, D1, USB1`.

**Mit entfallen** (Funktion, nicht Bauteil): das **Flashen des STM32 über die Uhr** und das **Zurücksetzen des STM über den ESP** (E6); der Konsolen-Umschalter; der SWD-Zugang; die 3,3-V-Einspeisung über `H3`.
Die 3,3-V-Schiene bleibt damit eine Diodenverknüpfung aus **zwei** Quellen: dem Wandler `U10` über `D2` und der Konsole `H6` über `D3`.

---

## 2. Neu (8 Referenzen)

| Ref | Was | Grund | Beleg |
|---|---|---|---|
| **U2** | ESP32-S3-WROOM-1-N8 | das Zielmodul (E1); alle 36 GPIOs am Modul frei | ✔K `komponenten/esp32s3-wroom-1.md` §1, §3 |
| **USB1** | USB-C-Buchse 16-polig, HRO TYPE-C-31-M-12 (C165948), Footprint `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` | Datenleitungen für Flashen, Konsole, JTAG (E3, Muss (b)) | ✔K `komponenten/usb-c-16p.md` §9 |
| **U11** | ESD-Schutz USBLC6-2SC6 (C7519), D+/D− und VBUS | E3: „plus ESD-Schutz“ | ✔K `komponenten/usblc6-2sc6.md` §8 |
| **SW5** | BOOT-Taster an `GPIO0` | einziger Rückweg, wenn die Anwendung `GPIO19/20` umkonfiguriert (Spec H1, AK3 Rückfall 2) | ✔K `esp32s3-wroom-1.md` §7, §12 |
| **R32** | 10 kΩ Pull-up `BOOT` → 3,3 V | `GPIO0` mit Pull-up (empfohlen) | ✔K `esp32s3-wroom-1.md` §4.1 |
| **C30** | 10 µF am 3,3-V-Netz (Abblockung Modul) | Entkopplung am Modul | ✔K `esp32s3-wroom-1.md` §8 (Referenz: 22 µF + 0,1 µF) |
| **C31** | 100 nF am 3,3-V-Netz (Abblockung Modul) | wie `C30` | ebd. |
| **C32** | 100 nF am Netz `LDR` (ADC-Filter) | HDG empfiehlt 0,1 µF am ADC-Eingang | ✔K `esp32s3-wroom-1.md` §6.1 (HDG S. 18) |

**Anmerkungen**

- **`SW5` und `R32`** sind im Generator als **Kopien** von `SW1` und `R12` angelegt. Eine **Position im Layout gibt es nicht**; wo ein BOOT-Taster bei 1:1-Format Platz findet, ist offen (Spec F17, OP-12).
- **`C30` ist 10 µF**, die Referenz im Modul-Datenblatt zeigt 22 µF + 0,1 µF (DS-M Abb. 9-1). `C8` (22 µF) liegt auf demselben Netz. Ob das als Entkopplung am Modul genügt, hat **kein Gutachten** bestätigt (OP-18).
- **`C32` hängt an einem Schnittstellennetz zum LED-Board** (`LDR`) und ist damit ein **zusätzliches Teil** im Sinn von Prüfung H8: **HOCH**, solange kein Eintrag `zusatz:C32` mit Grund in
  `tools/checks/hardware-led-schnittstelle.ausnahmen` steht und der Nutzer ihn nicht freigegeben hat (OP-03). Der Nutzer hat 100 % Kompatibilität verlangt; ein Filterkondensator ist elektrisch eine Änderung des LDR-Pfads.
- **Es gibt keine Reihenwiderstände an `USB_DP`/`USB_DN`, keinen externen D+-Pull-up und keine 499 Ω in `U0TXD`**, obwohl die Hardware Design Guidelines dafür Platz bzw. Bauteile empfehlen (OP-16, OP-17).

---

## 3. Geändert

| Was | V2 | V3 | Grund | Beleg |
|---|---|---|---|---|
| **`C1`** (Kondensator am Reset-/EN-Netz) | 100 nF an `RESET` | **1 µF** an `EN` | EN-Beschaltung 10 kΩ + 1 µF (RC-Verzögerung rund 10 ms) | ✔K `esp32s3-wroom-1.md` §8; ✔R `gen.py` (Kommentar „EN-Verzoegerung“) |
| **Netze `RESET` und `ESP_EN`** | zwei Netze | **ein** Netz `EN`; `SW3` (Reset-Taster), `R12` (10 kΩ nach 3,3 V), `C1` und `U2.3` hängen daran | ein Chip, ein Reset | ✔R: `expected.json`, Netz `EN` = {C1.1, R12.1, SW3.1, SW3.2, U2.3} |
| **`H6` (Konsole)** | Pin 2 am Netz `Net-(H6-Pad2)` (über `SW4`), Pin 3 `RX_UART` | Pin 2 **direkt** `TX_UART`, Pin 3 `RX_UART` = **UART0** des Moduls (`GPIO43`/`GPIO44`) | `SW4` entfällt; die Konsole ist UART0 (E3) | ✔R: `expected.json`, `TX_UART` = {H6.2, U2.37}, `RX_UART` = {H6.3, U2.36} |
| **Steuersignale des Controllers** | STM32-Pins (PB0, PB1, PB4, …) | Modulpins (siehe unten) | 1:1-Umbau (E8) | ✔R: `gen.py`, `expected.json` |
| **`USB1`** | 6 Pole: GND, VBUS, CC1, CC2 | 16 Pole: zusätzlich `A6`/`B6` → `USB_DP`, `A7`/`B7` → `USB_DN`, `SBU` offen | Datenleitungen | ✔R: `expected.json`, Netze `USB_DP`, `USB_DN` |

**Zuordnung der Signale (Stand des Generators, 17.10.2026; Vorschlag der Spec, nicht vom `hw-architect` entschieden):**

| Signal | V2 (STM32) | V3 (S3) |
|---|---|---|
| `LDR` | PA5 | GPIO1 (ADC1) |
| `DS18B20` | PB5 | GPIO2 |
| `WS2812_BUS` | PB1 | GPIO5 |
| `WS2812_POWER` | PB0 | GPIO6 |
| `TSOP` | PB4 | GPIO7 |
| `SDA_DS3231` / `SCL_DS3231` | PB7 / PB6 | GPIO8 / GPIO9 |
| `DCF77` | PB8 | GPIO15 |
| `USER` / `WPS` | PA0 / PA7 | GPIO16 / GPIO17 |
| `LED_STM` | PC13 | GPIO18 |
| `USB_DN` / `USB_DP` | — | GPIO19 / GPIO20 |
| `TX_UART` / `RX_UART` | PA10 / PA9 (Netzname aus Sicht des Steckers) | GPIO43 / GPIO44 (nach Name zugeordnet, siehe Anmerkung) |
| `BOOT` / `EN` | — | GPIO0 / EN |

(✔R: V2-Pins aus `v2/v2.xml` (`pinfunction`), V3-Pins aus `gen.py` (Aufrufe `modpin(...)`), Pad-Nummern in `expected.json`.) Die Prüfung H3 erzwingt diese Zuordnung **nicht** (`pinbelegung` in `tools/checks/hardware-pinregeln.json` ist leer, OP-09);
sie prüft nur die Sperr- und Festregeln (siehe [wissensstand.md](wissensstand.md), W-7).

**`TX_UART` und `RX_UART`:** In V2 heisst das Netz `TX_UART` so aus Sicht des Steckers `H6`: Es hängt am **Empfang** des STM (PA10), `RX_UART` an dessen **Sendung** (PA9). In V3 ordnet der Generator nach dem Namen zu: `TXD0` (GPIO43, Sendeausgang) liegt am Netz `TX_UART` und damit an `H6` Pin 2. Ein Konsolenadapter, der für V2 verdrahtet ist, träfe dort einen Ausgang auf seinen Sendeausgang (○ Folgerung; die Pinbezeichnung am Stecker selbst ist in den Symbolen nur numerisch, OP-35). Die Regel `nur_an_netzen` in `hardware-pinregeln.json` schreibt dieselbe Zuordnung fest.

**Das Netz `WS2812_POWER` trägt jetzt GPIO6.** Das Blatt des Moduls nennt als sauberste Wahl für ein Gate, das beim Booten nicht schalten darf, **GPIO21** (kein Strapping, kein Glitch). Der Generator nimmt GPIO6; das ist
mit `R20` am Gate des N-Kanal-`Q2` unkritisch (hochohmig im Reset, 60-µs-Low-Glitch harmlos), aber **nicht als Entscheid protokolliert** (Spec F2: „GPIO6 bleibt der Vorschlag“).

---

## 4. Unverändert (bewusst)

Laut den Hinweisen im Schaltplan und nach dem Vergleich der Pin-Mengen: Versorgung (`U9`, `U10`, `L2`, `D2`, Sicherungen `F1`/`F2`, TVS `D4`/`D5`, `Q3` als Verpolschutz), die 3,3-V-Verknüpfung über `D2`/`D3`, der LED-Pfad
(`U3`, `Q1`, `Q2`, `R11`, `R13`, `R17`, `R19`–`R23`, `R31`, `C6`, `C10`, `JP2`), der LDR-Pfad (`R22` = 1 kΩ), RTC (`U5`, `U7`), EEPROM (`U6`, `RN1`), DS18B20 (`U1`, `R5`), Taster (`SW1`–`SW3`), Status-LEDs, Test-Pads, Stecker `H1`, `H2`, `H4`, `H5`, `H7`–`H10`.
**H8 beweist es für die LED-Schnittstelle** (Bauteile, Werte, Pinanschlüsse und Netz-Mitglieder gegen V2, ohne den Controller).

## 5. Was sich an V2 nicht stillschweigend ändert, aber geändert werden müsste

Das V2-Layout, das als Ausgangsdatei für V3 dient (E12), trägt **Altlasten**, die ein Layout 1:1 mitnehmen würde:

- `D1`–`D3` mit dem Footprint SOD-123 im Layout, aber SOD-123W im Schaltplan und am Teil (OP-07).
- `USB1` mit dem Footprint der 6-poligen Buchse; für V3 kommt der 16-polige Footprint, und die **Aussenmasse beider Buchsen sind nicht verglichen** (OP-11).
- `R23` und `C10` fehlen in der Fertigungs-Stückliste (OP-01, OP-08).
- `R29` trägt 32 kΩ am Schaltplan, das bestellte Teil 31,6 kΩ (OP-05); `R23` trägt die LCSC-Nummer des 82-Ω-Widerstands (OP-06).
