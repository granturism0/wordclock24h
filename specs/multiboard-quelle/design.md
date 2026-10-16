# Design — Eine Quelle für alle Boards

**Stand:** 2026-10-17. Verfasser `sw-architect`. Zeichen: **✔** am Quelltext, an der Build-Konfiguration oder an einem vorhandenen
Bauergebnis nachgemessen (Verfahren: Anhang A) · **●** Sekundärquelle, nicht selbst nachgeprüft (S3-Spec, `HARDWARE.md`,
Komponentenblatt, Inventar `REVIEW-2026-10-17-hardware-naht.md`, Lead-Meldung) · **○** Annahme, ein offener Punkt.
Die Aufgaben stehen in `tasks.md`, die Akzeptanzkriterien (AK) und offenen Fragen (Q) in `requirements.md`.

**Zusammenfassung der Empfehlung** (Begründung in §4):

1. **Auswahl zur Bauzeit, in zwei Mechanismen.** Reine Hardwaredateien (11 von 47 `.c`) werden **durch Dateiauswahl** ersetzt. Die
   fünf gemischten Dateien (`main.c`, `irmp.c`, `esp8266.c`, `dcf77.c`, `sk6812.c`: zusammen **12 797 Zeilen**) werden **nicht
   geforkt**, sondern bekommen je **einen benannten Seam-Guard** an der Stelle ihres Hardwareblocks; der S3 bindet dort seinen
   Backend-Text ein. Für Bestandsboards bleibt der Tokenstrom **byte-identisch**, also das Abbild **hash-gleich**.
2. **Eine Board-Beschreibung je Board** (`boards/<id>/board.h`) trägt Pins und Merkmale. Der S3 bekommt sie sofort; die Bestandsboards
   in einer späteren, freizugebenden Stufe.
3. **Zwei Schutzebenen:** Sekunden-Prüfungen in `guardrails.sh` (Klassifikation, Attrappen-Übersetzung, Streuungszähler,
   Dienstliste, Pins) und ein **Hash-Vergleich aller 72 Matrix-Abbilder plus ESP8266** bei jeder Bestandsänderung.
4. **Eine Laufzeit-Schicht wird verworfen** (kostet auf dem F103 gerechnet mindestens 300 Byte von 1 172 freien, für null Nutzen).

---

## 1. Bestandsaufnahme der Variabilität

### 1.1 Boards, Chips, Bauwege

✔ Quellen: `CMakeLists.txt`, `Makefile`, `wclock24h.ebp`, `wclock24h-F103/wclock24h-F103.ebp`, `src/main.h`.

| Board-Makro | Chip(s) | Takt (HSE) | SPL-Baum | Matrix-Varianten¹ | Anmerkung |
|---|---|---|---|---|---|
| `BLUEPILL_BOARD` | STM32F103C8 (Cortex-M3) | 8 MHz | `wclock24h-F103/SPL` (F1) | 10 | engste Flash-Lage: 64 KiB |
| `BLACKPILL_BOARD` | STM32F401CC, STM32F411CE (M4) | 8 oder 25 MHz | `SPL` (F4) | 40 (4 Chips×Takte × 10) | **trägt auch die eigene Platine V2** (pinkompatibel) |
| `NUCLEO_BOARD` | F401RE, F411RE (F446RE nur als Code, kein Ziel) | 8 MHz | `SPL` (F4) | 20 | |
| `BLACK_BOARD` | STM32F407VE (M4) + TFT/SSD1963 | 8 MHz | `SPL` (F4) | 2 | Flash statt EEPROM, Touch |
| (`DISCO_BOARD`) | — | — | — | 0 | in 5 Zeilen, **nie definiert** |
| **ESP32-S3** (neu) | ESP32-S3-WROOM-1 (Xtensa, 2 Kerne) | — | — | **0** | ersetzt STM **und** ESP-12F |
| ESP8266 (ESP-12F) | Netz-Seite | 80 MHz | Arduino-ESP8266 | 1 (`make esp`) | eigener Quellbaum |

¹ 72 Matrix-Varianten = 2 Modi (12h/24h) × (BluePill 5 LED-Typen + BlackPill 4×5 + Nucleo 2×5) + 2 (F407, nur TFT) = 2×(5+20+10) + 2 = **72** ✔
(`CMakeLists.txt:510` prüft die Zahl 72, `cmake/matrix-report.sh` rechnet sie unabhängig nach).

**Bauwege ✔ (vier, plus der neue):**

| Weg | Was | Quelle der Dateiliste | Routinemässig? |
|---|---|---|---|
| `make f103` / `make f411` | CMake, Ziele `wordclock_f103_rgbw`, `wordclock_f411_rgbw` (je 12h, SK6812-RGBW) | `.ebp` (per `embitz_collect_sources`) | **ja** — Teil von `release-zip` und `guardrails.sh --full` |
| `make matrix` | CMake `-DWORDCLOCK_MATRIX=ON`, 72 Varianten | `.ebp` je Ziel | **nein** — auf Anforderung; letzter Bericht 13.10.2026 (●): 72/72 gebaut, engste Variante **1 172 Byte** frei |
| EmBitz (`.ebp`, 28 + 4 Ziele) | die IDE des Ursprungsprojekts | `.ebp` selbst | nein, nicht in der Prüfkette |
| `make esp` | `arduino-cli`, ESP8266 | Sketch-Ordner `ESP8266/ESP-uclock/` | **ja** |
| *neu:* S3 | ESP-IDF (Empfehlung §8.1) | `boards/modules.txt` | muss ja werden |

**Routineabdeckung: 2 von 72 STM-Varianten.** Die Dateilisten stehen heute **dreimal** (zwei `.ebp`, `CMakeLists.txt` filtert sie nur). Eine
vierte, handgepflegte Liste für den S3 (`stm_core`, S3-Spec §1.1) wäre die vierte Kopie.

### 1.2 Streuung der Unterscheidung ✔

123 Quelldateien (`src/**/*.c|*.h` ohne `src/stubs`; 49 `.c` insgesamt, 47 ohne Stubs, 76 `.h`), 46 389 Zeilen.

| Messgrösse | Zeilen | Bedingungszeilen (`#if/#elif/#ifdef/#ifndef`) | Dateien |
|---|---|---|---|
| Board-Makros (`BLACKPILL_BOARD`, `BLUEPILL_BOARD`, `BLACK_BOARD`, `NUCLEO_BOARD`) | **100** | 90 | **31** |
| Chipfamilien-/Chip-Makros (`STM32F10X`, `STM32F4XX`, `STM32F103…`, `STM32F4xx…`) | **226** | 169 | **57** |
| LED-Typ (`DSP_USE_*`) in `display.c` | — | **34** von 57 Bedingungszeilen | 1 |
| Layout (`WCLOCK24H`) in `display.c` / `tables.c` / `main.c` | — | 6 / 3 / 6 | 3 |
| Board- **oder** Familienmakros in **`display.c`** | **0** | **0** | 0 |

Nach Makro (Zeilen): `BLACK_BOARD` 47 (zählt auch das Schutzgerüst der TFT-Dateien), `BLACKPILL_BOARD` 19, `BLUEPILL_BOARD` 19, `NUCLEO_BOARD` 19.
Die meisten Board-Zeilen: `main.c` 12, `button.c` 8; je 4 in 10 weiteren Hardwaredateien.

**Grobe Zählung der `#define`-Zeilen, die ein Board festlegt** (Skript, Heuristik, ●): BlackPill **134**, BluePill **123**, Nucleo **76**
(`BLACK_BOARD` nicht vergleichbar). Das ist der Umfang, den eine Board-Beschreibung für ein Bestandsboard ablösen müsste (Q1).

**Familienmakros in Logikdateien ✔:** nur `eeprom.c` (3 Zeilen: `EEPROM_PAGE_SIZE`, 32 auf F4, 8 auf F103, sonst `#error`) und `vars.c` (2 Zeilen,
Black Board); in Kopfdateien der Logik (`ldr.h`, `rtc.h`, `eeprom.h`, `eep.h`, `tempsensor.h`, `ds18xx.h`, `delay.h`) als **Include-Schalter**.
**Falsche Makronamen ✔:** `STM32F1XX` in `power.h:15`, `board-led.h:15`, `ds18xx.h:16`, `onewire.h:15` — **nirgends definiert**.

### 1.3 Dateiklassen ✔

Klassifiziert nach Rolle, nachgemessen mit der Attrappen-Übersetzung (Anhang A.3): **28 von 47** `.c` übersetzen gegen leere
Hersteller-Header ohne Fehler; 1 hat 2 Fehler (`delay.c`); 18 haben je 20.

| Klasse | Bedeutung | Dateien | Zeilen | Umgang auf dem S3 |
|---|---|---|---|---|
| **L** Logik | übersetzt unverändert gegen Attrappen; kein Registerzugriff | **20**: `alarm`, `base`, `dfplayer`, `display`, `ds18xx`, `eeprom`¹, `esp-spiffs`, `ldr`, `log`, `night`, `overlay`, `remote-ir`, `rtc`, `tables`, `tempsensor`, `timeserver`, `vars`, `weather`, **`tetris`**, **`snake`** | 15 878 | **übernommen, unverändert** (AK7) |
| **M** gemischt | überwiegend Logik, ein Hardwareanteil | **4**: `main.c` (91 von 5 378 Zeilen mit Herstellerzugriff), `irmp.c` (67/5 114), `esp8266.c` (59/1 097), `dcf77.c` (29/537) | 12 126 | übernommen mit **Seam-Guard** (§3.6) |
| **G** geteilt | Kern und Hardware in einer Datei | **1**: `sk6812.c` (120/671) | 671 | **Kern übernommen**, Backend ersetzt (§3.6) |
| **H** Hardware | im Kern Registerzugriff | **11**: `adc`, `board-led`, `button`, `delay`, `i2c`, `onewire`, `power`, `wpsbutton`, `dfplayer-uart`, `esp8266-uart`, `log-uart` | 1 651 | **ersetzt** durch `esp32s3/hal/<name>.c` mit derselben Kopfdatei |
| **H–** Hardware, auf dem S3 ungenutzt | andere LED-Typen | **2**: `apa102.c`, `ws2812.c` | 946 | entfallen (S3 nutzt nur SK6812-RGBW) |
| **P** Plattform | Start-up, Takt | **1**: `system_stm32f4xx.c` (+ 4 `startup_*.S`, 5 Linkerskripte) | 946 | entfällt (IDF übernimmt) |
| **T** TFT/Black Board | `BLACK_BOARD` bzw. `DSP_USE_TFTLED_RGB` | **8**: `flash`, `font`, `ssd1963`, `tft`, `tftled`, `touch`, `w25qxx`, `xpt2046` | 4 160 | entfällt |

Summe 20 + 4 + 1 + 11 + 2 + 1 + 8 = **47** ✔. ¹ `eeprom.c` ist Logik **bis auf eine Stelle** (Seitengrösse, §9.3 E3).
**`tetris.c` und `snake.c` sind Logik, nicht TFT** (Spiele auf der Wortmatrix, `#if WCLOCK24H`); beide benutzen die Brücke
(`esp8266_get_message`) und `watchdog_reload` (● Inventar). Beide gehören damit zur S3-Funktion (E8).

### 1.4 Abgleich mit der S3-Spec und dem Inventar

Die S3-Spec nennt elf hardwarenahe Dateien für `hal/` (§4.1 ●). Gemessen sind es **19 `.c`** ausserhalb von L und T (Tabelle oben: 4 M + 1 G + 11 H + 2 H– + 1 P).
Die Differenz zur S3-Liste: `irmp.c`, `main.c`, `esp8266.c`, `dcf77.c`, `sk6812.c` (M/G — **nicht ersetzt, sondern geteilt**), `log-uart.c`,
`dfplayer-uart.c` (H, in der S3-Spec nicht einzeln genannt), `apa102.c`, `ws2812.c`, `system_stm32f4xx.c` (entfallen). **Verbindlich ist
`boards/modules.txt`** (AK2), nicht diese Tabelle und nicht die Liste der S3-Spec. Das Inventar des `firmware-analyst` (`REVIEW-2026-10-17-hardware-naht.md`)
stimmt mit der Klassifikation überein; seine Zeilennummern gelten für den Baum vom 17.10.2026 und veralten.

### 1.5 Die ESP-Seite ✔

`ESP8266/ESP-uclock/`: 12 `.cpp` + 1 `.ino` + 13 `.h`, **27 802 Zeilen** (davon `http.cpp` 17 651, `ESP-uclock.ino` 2 471, `vars.cpp` 1 649, `stm32flash.cpp` 1 438).

| Plattformnaht | Zahl |
|---|---|
| `#include` plattformspezifischer Köpfe (`ESP8266WiFi.h`, `LittleFS.h`, `WiFiUdp.h`, `ESP8266httpUpdate.h`, `ESP8266HTTPClient.h`, `Updater.h`, `lwip/*`) | 23 Zeilen |
| `ESP.`-Aufrufe | 34 (`http.cpp` 29, `ESP-uclock.ino` 5) |
| SDK-Aufrufe (`wifi_*`, `system_*`, `os_*`, `ets_*`, `spi_flash_*`) | 20 (`.ino` 6, `http.cpp` 9, `wifi.cpp` 5) |
| lwIP-Zeilen (`tcp_pcb`, `lwip`, privater Kopf in `http.cpp:19`) | 55 |
| `Serial`-Vorkommen | 472 |
| `#ifdef ESP32`/`ARDUINO_ARCH_*`-Schalter heute | **0** |
| Bedingungszeilen insgesamt (`#if…`) | 32, **keine** auf einem Board |

Die Netz-Seite kennt **ein** Board. Der Port nach ESP32 ist damit **nicht** ein Board-Zweig, sondern eine **zweite Plattform** der Netz-Seite — Gegenstand der
S3-Spec (Tasks 5.3a–c, `#ifdef ESP32`), hier nur als Rahmen: dieselben Quelldateien, Zweig je Plattform, **kein Fork**.

### 1.6 Zwei Quellbäume, die in einem Abbild landen ✔

5 gleichnamige Kopfdateien (`base.h`, `display.h`, `tables.h`, `vars.h`, `weather.h`), 5 gleichnamige Quelldateien (`base`, `display`, `tables`, `vars`, `weather`),
mindestens 6 gleichnamige globale Funktionen (`dayofweek`, `eeprom_read`, `htoi`, `strsubst`, `tables_init`, `var_crc`; ● Skript, Heuristik). Abweichung der
Kopfdateien nach `diff` (Zeilen): `eeprom-data.h`↔`eepromdata.h` 291, `base.h`↔`base.h` 86, `tables.h`↔`tables.h` 78, `vars.h`↔`vars.h` 452 — also **gleiche
Namen, verschiedener Inhalt**. `htoi`, `strsubst`, `dayofweek` und `var_crc` sind zusätzlich **Code-Doppelungen** zwischen den Welten (bestehend; Folgespec, §11 Alt. 7).

---

## 2. Befunde, die Ausgangsannahmen korrigieren

| # | Ausgangsannahme | Gemessen |
|---|---|---|
| B1 | „`display.c` … enthalten keinen STM-Header“ | **Direkt ja, transitiv nein.** Alle **20** Logikdateien ziehen 26 SPL- und 4 Chip-/CMSIS-Kopfdateien herein. Die Ursache sind **Kopfdateien**, nicht Code (§3.2) |
| B2 | „`main.h` und `base.h` ziehen nur `time.h`/`stdint.h`/…“ | `base.h` ja; **`main.h` nicht in der Wirkung:** Es **bricht ab** (`#error unknown STM32 hardware`, `main.h:82`; `#error unknown HW oscillator frequency`), wenn weder Chip noch HSE bekannt sind. Der S3 braucht dort **einen Zweig** (§9.3 E1) |
| B3 | „20 `.c`-Dateien sind hardwarenah“ | 19 `.c` plus 1 Grenzfall (`delay.c`) bei Attrappen-Übersetzung; 11 davon Klasse H. Die Liste war eine Schätzung ohne Messverfahren |
| B4 | „die LED-Treiberschnittstelle sind sechs Funktionen“ | `display.c` braucht **drei** (`led_init`, `led_refresh`, `led_set_led`) und den Typ `LED_RGB`; `led_set_all_leds` benutzt nur der Treiber selbst; die zwei Zähler `main.c`. Alle sechs bleiben (S3-Spec AK6a), aber **die Naht ist enger, als sie aussieht** |
| B5 | „die Brücke ist eine UART“ | Auch; **ihr Wissen steckt in sechs Logikmodulen** (§3.4). Die Naht liegt **unter** `esp8266.c`, nicht an ihm |
| B6 | „Hardware-Module hängen nur an Hardware“ | `sk6812.c` (`uptime`, `log_printf`), `eeprom.c` (`diag_ticks`, `esp8266_uart_rxdrops`, `log_printf`), `dcf77.c` (`board_led_*`, `log_message` **aus der ISR**) rufen in die Anwendung zurück (§3.3) |

---

## 3. Die Naht zwischen Logik und Hardware

### 3.1 Prinzip

Die Naht ist **die Menge der Kopfdateien der Klasse H** (plus drei Plattformdienste in `main.c`), mit **denselben Funktionsnamen, Typen und Rückgabewerten**
auf jedem Ziel. Eine Schnittstelle gilt erst, wenn sie **vier** Dinge nennt: **Funktionen und Typen**, **Aufrufkontext** (wer ruft aus welchem Kontext),
**Zeitverhalten** (blockiert, wie lange, welche Zeitannahme) und **Zuständigkeit je Ziel** (welche Datei liefert sie). Kontextkürzel: **H** Hauptloop (auf dem S3 der
`display_task`), **I** Timer-Interrupt (heute TIM2, 15 kHz), **D** Treiber-/DMA-Interrupt, **B** blockiert (Obergrenze genannt), **Init** einmal beim Start.

### 3.2 Die Include-Naht und die Attrappen ✔ (gemessen)

Der Code der Logik benutzt keine Herstellertypen; ihre Kopfdateien binden sie ein. Das ist **messbar**:

| Versuch (Anhang A) | Ergebnis |
|---|---|
| Include-Baum von `display.c` (F411, `cc -MM`) | 26 SPL-Kopfdateien + `core_cm4.h`, `stm32f4xx.h`, `stm32f4xx_conf.h`, `system_stm32f4xx.h`; für **alle 20** Logikdateien dasselbe |
| Logik gegen **20 leere** Hersteller-Header (je `#include <stdint.h>`, `<stddef.h>`), BlackPill-Konfiguration | **28 von 47** `.c` ohne Fehler (alle 20 Logik- und alle 8 T-Dateien) |
| Logik **ohne** Attrappen und **ohne** Familienmakro (so, wie der S3-Bau startet) | **0 von 20.** 16 scheitern an `misc.h` (8 Kopfdateien binden es unbedingt ein), 4 (`eeprom`, `ldr`, `overlay`, `rtc`) an `uint_fast8_t` |
| Logik mit **einer Attrappe `misc.h`**, `-include stdint.h -include stdbool.h -include stddef.h`, Platzhalter-Chipkennung (`-DSTM32F411CE`, nur für `main.h`) | **19 von 20** ohne Fehler; **nur `eeprom.c`** stoppt bei `EEPROM_PAGE_SIZE` (`eeprom.c:49`). Dasselbe mit BluePill-Kennung (`STM32F103C8`, 8 MHz): `display.c`, `vars.c` fehlerfrei, `eeprom.c` wie oben |

**Folgerung.** Die Logik des S3 kann **ohne Änderung an einer Logikdatei** gebaut werden, wenn das S3-Gerüst **vier Dinge** liefert:
(1) eine Attrappe `misc.h`; (2) die erzwungenen Einbindungen `-include stdint.h`, `-include stdbool.h`, `-include stddef.h`; (3) einen **echten** Zweig für
Chip und Oszillator im Konfigurationswort (§9.3 E1) statt des Platzhalters; (4) die Seitengrösse aus `board.h` (§9.3 E3). **Das ist die Zulieferung, die in der S3-Spec
fehlt (W1)**; ohne sie ist deren AK6a („gegen die originale `sk6812.h` übersetzt“) nicht erfüllbar.

**Was die Attrappen nicht belegen — und was daran gefährlich ist.** Die Attrappen belegen **Übersetzbarkeit auf dem Entwicklungsrechner**, nicht Verhalten. Der Mac ist nicht die
Zielplattform (CLAUDE.md-Lehre, `tools/checks/pruefstand.h`, BEFUNDE L256): Dort ist `uint_fast8_t` ein `unsigned char`, auf dem STM32 ein `unsigned int`. Die Logik rechnet
mit `uint_fast8_t`, `uint_fast16_t`, `time_t`, `char`. **Deshalb AK11:** die Breiten und Layouts werden **auf jedem Ziel** per `_Static_assert` festgenagelt — sonst
kann der S3 übersetzen und anders rechnen.

### 3.3 Plattformdienste — die Rückwärtsnaht

Hardware-Module und Logik beziehen heute Dienste aus Dateien, die sie nicht kennen sollten. Diese Dienste sind **die** Rückwärtsschnittstelle und stehen in einer
Dienstliste `boards/platform-services.txt` (Symbol, Anbieter, Verbraucher, Kontext). Jedes Ziel liefert sie; jede Änderung der Liste ist ein Befund.

| Dienst | Anbieter heute | Verbraucher ausser `main.c` ✔ | Kontext | S3 |
|---|---|---|---|---|
| `uptime` (`volatile uint32_t`, Sekunden) | `main.c:685`, Timer-ISR `:947` | 7 Dateien (`vars`, `sk6812`, `timeserver`, `tetris`, `esp8266`, `flash`, `display`) | Schreiben **I**, Lesen H | Softuhr bleibt **im Timer-ISR-Körper**; auf **demselben Kern** wie `display_task` (sonst Atomizität, ○ siehe §3.7) |
| `gmain.*` (Softuhr, `volatile`) | `main.c`, Timer-ISR | `display`, `vars`, `timeserver` (● Inventar §5) | wie oben; **Mehrfeld-Lesezugriffe nicht atomar** (bestehend, ○) | unverändert, **nicht** vom `net_task` lesen (S3-Spec AK23) |
| `watchdog_reload ()` | `main.c:581` | 6 Dateien (`vars`, `snake`, `tetris`, `remote-ir`, `esp8266`, `display`) | H | `esp_task_wdt_reset`; Aufrufstellen bleiben (S7: `WD_EXPECTED=10`) |
| `schedule_esp8266_messages ()` | `main.c` | 4 Dateien (`vars`, `remote-ir`, `display`, `tftled`) | H (Warteschleifen rufen es) | unverändert |
| `diag_ticks ()`, `esp8266_uart_rxdrops ()` | `main.c:702`, `esp8266-uart.c` | **nur `eeprom.c`** (Diagnosezeile des Schreibens) | H | liefern, auch wenn die Brücke eine Warteschlange ist (Zähler `d=`, S3-Spec AK22) |
| `log_*` (`log_printf`, `log_message`, `log_puts`, `log_putc`, `log_flush`) | `log.c` + `log-uart.c` | **19 Dateien** | H; **zusätzlich I** (`dcf77_tick` ruft es aus der ISR) | zwei Senken (§3.5.4), **ISR-fest oder Aufruf verlegt** (AK20) |
| `delay_counter` (`volatile`, **öffentlich beschreibbar**) | `delay.c` | `i2c.c`, `onewire.c` (schreiben ihn **direkt**) | H | entfällt mit beiden Dateien (S3 ersetzt sie) |

**Regel.** Eine Hardware-Datei darf **nur** diese Dienste rückwärts rufen; ein Zähler (AK19) schlägt an, wenn eine Hardware-Datei einen weiteren
`extern`-Gegenstand aus `main.c`/`display.c` benutzt. Die **Entkopplung** (Dienst als Funktion statt als Variable) ist **keine** Aufgabe dieser Spec — sie änderte Bestandscode
der Klasse F; die Liste hält den Zustand fest, damit er nicht wächst.

### 3.4 Die Hardware-Schicht: Schnittstelle je Baustein

Spalten: Funktionen/Typen (✔ aus den Kopfdateien), Kontext, Zeitverhalten (● Inventar, wo nicht ✔), S3-Anbieter.

| Baustein (Kopfdatei) | Funktionen und Typen | Kontext | Blockieren / Zeitannahme | S3-Anbieter |
|---|---|---|---|---|
| **delay** (`delay.h`) | `delay_init (uint_fast8_t)`, `delay_usec/msec/sec (uint32_t)` | H, Init; **nie I** | B, Busy-Wait **ohne** Reload; Auflösung 5 µs, `delay_usec(n)` wartet n…n+5 µs; ein globaler Zähler, **nicht reentrant** | `hal/delay.c`: `esp_rom_delay_us` / `vTaskDelay` (Semantik wird weicher, Reihenfolge der Schritte bleibt) |
| **i2c** (`i2c.h`) | `i2c_init (uint32_t)`; `int_fast16_t i2c_read/write (slave8, addr, is16, buf, cnt)`; `I2C_OK = 0`, `I2C_ERROR_*` −1…−8; `i2c_nack_quiet` | H, Init; nie I | B, **jedes Flag bis 5 ms**, bei Frist STOP, Reset, 9-Takt-Freitakten; **NACK ≠ Störung** (A73); Slave-Adresse als **8-Bit-Form** (`0xA0`, `0xD0`) | `hal/i2c.c`, intern 7 Bit, Vertrag unverändert |
| **adc** (`adc.h`) | `adc_init`, `adc_start_single_conversion`, `uint_fast8_t adc_poll_conversion_value (uint16_t *)` | H (Start alle 1/4 s per ISR-Flag) | **nicht blockierend** (Start/Poll getrennt); Wert **12 Bit, 0..4095, Referenz 3,3 V** | `hal/adc.c`: ADC1 oneshot; Dämpfung `BOARD_LDR_ADC_ATTEN_DB`; Normierung auf **0..4095 der STM-Skala** (V11) |
| **onewire** (`onewire.h`) | `onewire_reset/_read_bit/_read_byte/_read/_write_byte/_write/_get_rom_code/_init` | H; **keine IRQ-Sperre** | B: Reset 1,15 ms; Slot-Zeiten über `delay_usec`, 5-µs-Quantisierung; offen A63/A64/A65 | `hal/onewire.c` (S3-Task 4.3); **A63–A65 nicht verdecken** |
| **LED** (`sk6812.h`, Makros in `display.h`) | `sk6812_init`, `_refresh (uint_fast16_t)`, `_set_led (n, SK6812_RGBW *)`, `_set_all_leds (…)`, `_get_refresh_cnt`, `_get_dma_wait_cnt`; Typ `SK6812_RGBW` | H (init, set, refresh); **D** (Nachfüllen) | `refresh`: **B**, Spin auf das vorige Bild, **ohne Timeout**; danach kehrt es zurück | **Kern übernommen**, Backend `esp32s3/hal/` (§3.6) |
| **power** (`power.h`) | `power_init/_on/_off`, `power.is_on` (Start 0xFF) | H | nein; **200 ms Einschwingen im Aufrufer** (`display.c:2954-2955`) | `hal/power.c`; **erster HAL-Aufruf** beim Start, **vor** WLAN, Konsole, Tasks (bekannter Startpegel, ● Komponentenblatt: GPIO1–18 ≈ 60 µs Low, danach hochohmig; extern `R20` 100 kΩ Pull-down) |
| **board-led** (`board-led.h`) | `board_led_init/_on/_off` | H **und I** (`dcf77_tick`) | nein, ein GPIO-Schreibzugriff; **muss ISR-fest sein** | `hal/board-led.c`, IRAM |
| **button, wpsbutton** | `*_init`, `uint_fast8_t *_pressed ()` | H (Poll) | nein; aktiv low; Entprellung im Aufrufer (`uptime`-Differenz > 5 s) | `hal/button.c`, `hal/wpsbutton.c`; Pull-up intern/extern = Board-Merkmal (S3-Spec F7) |
| **dcf77** (Klasse M) | `dcf77_tick ()` (**I**), `dcf77_time (struct tm *)` (H), `dcf77_init` | I und H | Zustandsmaschine in 10-ms-Ticks (150 Ticks bei 15 kHz); **`log_message` aus der ISR** | Pin-Zugriff über Seam-Guard; Logik unverändert |
| **irmp** (Klasse M) | `irmp_init`, `irmp_get_data (IRMP_DATA *)`, `irmp_ISR ()` (**I**), `irmp_is_busy` | I, H | ISR ≤ 1/`F_INTERRUPTS` (66,7 µs bei 15 kHz, ○ ungemessen); alle Schwellen aus `F_INTERRUPTS` (10…20 kHz zulässig) | Zielzweig im Pin-Zugriff/Init (Seam-Guard); `F_INTERRUPTS` per `-D` (steht hinter `#ifndef`, `irmpconfig.h:29`) |
| **uart** (`uart.h`, `uart-driver.h`; **3 Instanzen**) | je 14 Funktionen: `_uart_init/_putc/_puts/_char_available/_getc/_poll/_rsize/_rxmax/_rxdrops/_rxore/_rxbuflen/_flush/_read/_write` (die beiden letzten **deklariert, nicht definiert**) | H; Ring füllt **D** | `putc` bei vollem Ring und `getc` bei leerem **B ohne Timeout**; Ringe: log TX 128/RX 256, ESP TX 128/**RX 1024** (89 ms bei 115 200 Baud), DFPlayer 32/32; **Verwurf wird gezählt** | Instanz `esp8266` → Warteschlange (§3.4.1); `log` → Senken (§3.5.4); `dfplayer` → **Nullimplementierung** (auf V2 sind PA11/PA12 unbeschaltet) |
| **Brücke** (`esp8266.h`, Klasse M) | `esp8266_get_message`, `_send_cmd`, `_send_data`, `_send_log_line`, `_reset/_powerdown/_powerup/_flash`, `_init` … | H (`esp-spiffs.c:39`: nur Hauptloop) | 10-ms-Ticks (`MSEC(x)=x/10`), 3-s-Quittungsfrist, `esp8266_reset` 50 ms + 500-ms-Poll beim Boot | §3.4.1 |
| **Zeitgeber** (`main.c`, Klasse M) | `timer2_init`, `TIM2_IRQHandler` | **I**, 15 kHz | Körper ≈ Softuhr, Flags, IRMP, DCF77-Tick, Ambilight-Takt | §3.7 |
| **Watchdog/Reset** (`main.c`) | `watchdog_init`, `watchdog_reload`, `fault_reset`, `log_reset_flags` | H, Init | 20 s (`WATCHDOG_TIMEOUT_MS`) | `esp_task_wdt`, `esp_reset_reason` (S3-Spec 4.1, 5.4) |

#### 3.4.1 Die Brücke: unterste Schicht, nicht das Modul

Entscheid des Nutzers **E5**: Die Kommandobrücke bleibt als Schicht (S3-Spec). Dem folge ich (W10). **Die Naht ist die Instanz `esp8266` aus `uart.h`**: `esp8266.c` mit dem Zeilenprotokoll
(`var …*crc`, `ACK`, `CMD`, `TIME`, `TAB*`, `FILE`, `ICON`, `LOG`) bleibt **unverändert**; die 14 `esp8266_uart_*`-Funktionen bekommt eine Warteschlange statt einer UART
(S3-Spec 5.2). **Die Steuerpins zur Gegenstelle** (`ESP8266_RST`, `_CH_PD`, `_FLASH`; BlackPill PB10/PB3/PA4, BluePill PA0/PA1/PA4, ● Inventar) sind eine **eigene Naht „Gegenstelle“**: `esp8266_reset`,
`_powerdown`, `_powerup`, `_flash` und der Boot-Pfad (`esp8266_flash` ist eine Endlosschleife **vor** `watchdog_init`). Auf dem S3 gibt es **keinen** zweiten Chip; wie diese Funktionen antworten, regelt
S3-Spec F10 (Q11), die Naht liegt in `esp8266.c` als Seam-Guard (§3.6).

### 3.5 Verträge, die jede Hardware-Schicht erfüllt (V11)

Das LED-Board bleibt 100 % kompatibel (Nutzer). Der Vertrag, den **jedes** Ziel erfüllt und der **prüfbar** sein muss:

**3.5.1 LED (`led_*`).** (a) **Byte-Reihenfolge je LED: G, R, B, W**, MSB zuerst (✔ `sk6812.c:354-373`). (b) Zeiten **in Nanosekunden aus einer Kopfdatei** (✔ heute `sk6812.c:91-97`: Periode 1250,
T0H 300, T0L 950, T1H 750, T1L 500, Pause **250 000 ns**); Umrechnung in Zählerschritte ist Sache des Backends. Die Pause bleibt **250 µs** (SKC6812: > 200 µs, `HARDWARE.md`; SK6812 genügte 80 µs — **nicht verkürzen**).
(c) `sk6812_set_led` schreibt **nur** in den „nächsten“ Puffer und sendet nichts; Index ≥ `SK6812_MAX_LEDS` wird **verworfen**. (d) `sk6812_refresh (n)`: zählt den Aufruf (saturierend), **wartet auf das Ende des vorigen
Bildes**, wechselt den Doppelpuffer, startet die Übertragung, **kopiert danach den aktuellen in den nächsten Puffer** (damit `set_led` auf dem Stand des letzten Bildes weiterarbeitet) und kehrt zurück. (e) Beide Zähler **saturieren** (32/16 Bit).
(f) `led_init` löscht alle LEDs. (g) Maximal `SK6812_MAX_LEDS = DSP_MAX_LEDS` (12h: 114 Display + 120 Ambilight = 234 ✔ `display-config.h`).
**Diese Verträge stehen im Kern** (§3.6) — der S3 erbt sie, statt sie nachzubauen.

**3.5.2 Power.** `power_on()`/`power_off()` schalten nur den Pin; **die 200 ms Einschwingzeit stehen im Aufrufer** und bleiben dort. Der S3 darf sie nicht in den Treiber ziehen.

**3.5.3 LDR.** `adc_poll_conversion_value` liefert **Rohwerte 0..4095** (12 Bit, Referenz 3,3 V). Die Normierung gegen `ldr_min_value`/`ldr_max_value` (EEPROM) bleibt in `ldr.c` **unverändert**. Wechselt auf dem S3 die Dämpfung oder
der Teilerwiderstand (offen bei `hw-architect`: 10 kΩ mit 6 dB **oder** 47 kΩ mit 12 dB, ● Meldung des Lead), muss die HAL die Wandlung **auf diese Skala** umrechnen und die Dämpfung **je Board konfigurierbar** halten
(`BOARD_LDR_ADC_ATTEN_DB`). Die Kalibrierwerte werden dann **neu gemessen** (S3-Spec AK17b); das ist **Daten**, kein Vertragsbruch.

**3.5.4 Log.** `log_*` ist ein Dienst (§3.3). Auf dem S3 sind **UART0 (GPIO43/44) und USB-Serial/JTAG (GPIO19/20) zwei getrennte Kanäle** (● Komponentenblatt, Lead-Meldung): In ESP-IDF ist die USB-Zweitkonsole nur Ausgabe, nicht blockierend,
ohne Eingabe, und erscheint nur, wenn UART0 nicht verbunden ist. **Die Log-Schicht bedient beide Senken selbst** (`BOARD_LOG_SINKS`), jede **nicht blockierend** mit eigenem Verwurfszähler. Und `log_message` wird heute **aus der Timer-ISR** gerufen (`dcf77_tick`);
auf dem S3 ist das **verboten**, solange der Pfad nicht ISR-fest ist: **Entscheid in S3-Task 4.1 (AK20)** zwischen (i) ISR-festem Ringpuffer-Schreiben (IRAM, kein Treiberaufruf) und (ii) Verlegen des `dcf77_tick`-Aufrufs in den Hauptloop (**Änderung der Klasse F**, mit Gerätenachweis).

### 3.6 LED-Treiber und Seam-Guards — die Technik

**Befund ✔:** `sk6812.c` (671 Zeilen) besteht aus (Zeilenbereiche): Konfiguration je Board **1–239**; Kern **`sk6812_clear_all` 306–326, `sk6812_refresh` 458–518, Zähler 520–537, `sk6812_set_led` 540–556, `sk6812_set_all_leds` 558–580**
(zusammen rund **140 Zeilen**, portabel); Hardware **`sk6812_dma_init` 240–287, `sk6812_dma_start` 288–300, `sk6812_setup_dma_buf` 332–402, ISR 405–456, `sk6812_init` 583–669** (rund **270 Zeilen**, TIM/DMA/NVIC). Der Kern ruft aus dem Hardwareteil nur
`sk6812_setup_dma_buf (0|1)` und `sk6812_dma_start ()` und liest/schreibt `sk6812_dma_status` (0 = bereit, 1 = belegt).

**Technik.** Der Hardwarecode liegt **in drei getrennten Blöcken, zwischen denen Kernfunktionen stehen** (A: `sk6812_dma_init` und `sk6812_dma_start`, ca. 240–300; dann der Kern `sk6812_clear_all`; B: `sk6812_setup_dma_buf` und die ISR, ca. 332–456; dann der Kern `refresh` … `set_all_leds`; C: `sk6812_init`, ca. 583–669).
**Ein Guard-Name, drei Regionen**; der Kern bleibt **an seinem Platz**:

```
/* Block A */
#if defined (WC_LED_BACKEND_EXTERNAL)
#  include "sk6812-backend.inc"      /* S3: esp32s3/hal/; liefert setup_dma_buf, dma_start, Fertig-Rueckruf UND sk6812_init;
                                        deklariert static sk6812_clear_all vorwaerts; sieht die statics des Kerns */
#else
   ... bisheriger STM-Text von Block A, unveraendert ...
#endif
   ... Kern: sk6812_clear_all ...
#if ! defined (WC_LED_BACKEND_EXTERNAL)
   ... Block B (setup_dma_buf, ISR), unveraendert ...
#endif
   ... Kern: refresh, Zaehler, set_led, set_all_leds ...
#if ! defined (WC_LED_BACKEND_EXTERNAL)
   ... Block C (sk6812_init), unveraendert ...
#endif
```

Warum so — und nicht anders: (1) **Hash:** Der Tokenstrom für jeden STM-Build ist **derselbe** (der Präprozessor wirft den Guard-Zweig weg) ⇒ Abbild **byte-identisch**. (2) **Kein Fork:** Der S3 übersetzt **dieselbe Datei** `sk6812.c`, also dieselbe
Logik: Doppelpuffer, Warten, Zähler, `GRBW` an **einer** Stelle — das ist „LED-Ansteuerungslogik so weit wie möglich übernehmen“ (V2), **wörtlich**: Von 671 Zeilen sind 140 echt gemeinsam, und der Teil, der **Verhalten** trägt (Reihenfolge der Schritte in `refresh`), ist dabei.
(3) **Eine Einbindedatei mit Auslagern des Kerns** (der erste Gedanke) ist verworfen: Das Auslagern ändert die **Reihenfolge der Definitionen** in der Übersetzungseinheit, und der Linker legt Funktionen in dieser Reihenfolge ab ⇒ Adressen ändern sich ⇒ **Hash weicht ab**, obwohl das Verhalten gleich ist (§11 Alt. 6).
(4) Das Backend ist **Text, kein Link-Symbol**, weil es die `static`-Variablen des Kerns sieht (`rgbw_buf`, `current_rgbw_buf_idx`, `current_leds`, `sk6812_dma_status`) und der Kern **nicht angefasst** wird.

**Backend-Vertrag (was `sk6812-backend.inc` liefern muss):** `sk6812_init (void)` (ruft am Ende `sk6812_clear_all (SK6812_MAX_LEDS)` des Kerns; dazu **Vorwärtsdeklaration**, weil der Kern erst danach steht); `static void sk6812_setup_dma_buf (uint_fast8_t)` (S3: darf leer sein); `static void sk6812_dma_start (void)` (S3: erzeugt aus `rgbw_buf[current_rgbw_buf_idx]` mit `current_leds` die RMT-Folge **G,R,B,W**
und setzt `sk6812_dma_status` am Ende der **Pause** auf 0 — **Pause 250 µs eingerechnet**); `sk6812_dma_status` wird aus dem Fertig-Rückruf geschrieben (Kontext D). **Das `wait`-Verhalten bleibt**: ein hängendes Bild blockiert den nächsten `refresh` (S3-Spec fügt dort eine Frist hinzu; **das ist eine
Verhaltensänderung der Klasse F im Kern und gehört nicht in diesen Umbau**, solange der Kern gemeinsam ist — Q12).

**Dieselbe Technik gilt für die anderen gemischten Dateien** — **vier Seam-Guards insgesamt**, jeder **nach der Funktion** benannt, nicht nach dem Board (Katalog):

| Guard | Datei(en) | Region | Backend liefert |
|---|---|---|---|
| `WC_PLATFORM_EXTERNAL` | `main.c` | (a) Takt/`SystemCoreClock` (≈ 57–77), (b) Reset-Ursache, Watchdog, `fault_reset`, SysTick-Frist (≈ 395–680), (c) Zeitgeber-Tabellen und `timer2_init` (≈ 720–830), (d) die ersten Zeilen von `main()` (`SystemInit`, JTAG-Remap) | je Region ein `.inc`: dieselben Funktionen mit **denselben Namen**; die ISR-Quittung als Makro `WC_TICK_ACK()` |
| `WC_LED_BACKEND_EXTERNAL` | `sk6812.c` | **drei** Hardwareblöcke A, B, C (§ oben) | Backend-Vertrag oben |
| `WC_IRMP_TARGET_EXTERNAL` | `irmp.h`, `irmpconfig.h`, `irmp.c` | Pin-Zugriff (`IRMP_PIN`/`input()`) und `irmp_init`-Zweig (42 Herstellerzeilen) | Pin lesen, Pin einrichten |
| `WC_PEER_EXTERNAL` | `esp8266.c` | Steuerpins und Boot-Pfad (`reset/powerdown/powerup/flash`) | S3-Spec F10 (Q11) |

`dcf77.c` braucht **keinen** Guard, wenn sein Pin über `board.h` kommt (29 Herstellerzeilen, nur die Pin-Auswahl; Kern bleibt). **Zeilennummern oben sind Richtwerte des Baums vom 17.10.2026**; der Umsetzer legt die Schnitte an **Funktionsgrenzen** und **nie
mitten in einer Funktion**.

**Preis dieser Technik** (ehrlich): vier Guard-Namen in Bestandsdateien (heute geplant **3 + 4 + je mindestens 1 Region**; die Regionenzahl für `irmp` und `esp8266` legt Task 0.4 fest) = neue Bedingungsfamilien, die der Streuungszähler **namentlich und mit Regionenzahl** kennt (Katalog oben) und die **nicht wachsen** dürfen; jede Region muss sauber begrenzt sein; Lesen der Datei verlangt, den Guard zu kennen.
Dafür **keine** geforkte Zeile und **kein** Byte Änderung auf Bestandsboards.

### 3.7 Zeitbasis, Interrupts, Kerne

✔ `TIM2_IRQHandler` (`main.c:853-1026`) ist **zur Hälfte Anwendung**: Bis auf die erste Anweisung (`TIM_ClearITPendingBit`) enthält er die **Softuhr** (`uptime++`, `gmain.second/minute/hour/…`), die Flags (LDR alle 1/4 s, Animation 1/64 s), `irmp_ISR`, `dcf77_tick`
(je 150 Ticks), `esp8266_ten_ms_tick`, `eeprom_ms_tick` (je 15 Ticks) und den Ambilight-Takt. **Er muss auf dem S3 derselbe Text sein.** Deshalb: nur `WC_TICK_ACK()` ersetzt die Quittung; der GPTimer-Rückruf ruft den Körper.
**Zeitbasis ist `F_INTERRUPTS` Ticks je Sekunde** (`main.c:942`); eine Abweichung der **echten** Rate von der nominalen verschiebt die Softuhr (✔ Inventar: F411 100 MHz/66/101 = 15 001,5 Hz, Softuhr ≈ 0,01 % schnell; per RTC/NTP nachgeführt). Daraus folgt für `board.h`:
**`BOARD_TICK_HZ` ist die echte Rate**, und auf dem S3 muss ein **ganzzahliges** Teilerverhältnis her (S3-Spec F14/AK16; Q8).
**Prioritäten:** Auf dem STM sind die **effektiven NVIC-Prioritäten unklar** (○ Inventar §1: `PRIGROUP` = 0, `misc.c:148-157` rechnet mit `tmppre = 4 − 7`; es kann sein, dass **alle** IRQs auf 0 stehen). Der S3 **erbt das nicht zufällig**: Die Hardware-Schicht **legt Prioritäten fest und dokumentiert sie**
(S3-Spec 4.1). Keine Empfehlung zum STM-Befund (DIR-003).
**Kerne.** `display_task` und die Timer-ISR laufen auf **demselben Kern** (S3-Spec §3.2), damit die `volatile`-Flag-Mechanik (ISR schreibt, Hauptloop liest, **ohne** kritische Sektion) unverändert trägt. **Das ist eine Randbedingung dieser Spec**, kein Detail: Lägen ISR und Hauptloop auf verschiedenen Kernen,
wären `gmain.*` (Mehrfeld) und die Flags **Wettläufe**, die es heute nicht gibt.

### 3.8 Die Include-Naht als Dauerlösung?

Die Attrappen lassen die Header der Logik **unverändert**. Die Alternative — **26 Kopfdateien** auf **eine** `platform.h` umstellen — beseitigt die Durchlässigkeit, ändert aber Bestand (Klasse A/H1, hash-prüfbar) und braucht die Freigabe (Q1). Empfehlung: **Attrappen jetzt, Normalisierung in Stufe 5**, nicht blockierend für den S3.
Das Gegenmittel gegen das Wachsen der Durchlässigkeit ist der Zähler (§6): keine **neue** Kopfdatei der Klasse L/M darf Hersteller-Header einbinden.

---

## 4. Optionen und Empfehlung

| | **(a) Bauzeit: Dateien/Build-System** | **(b) Bauzeit: `#if` je Board** | **(c) Laufzeit-Schicht** | **Empfehlung: (a) + benannte Seam-Guards** |
|---|---|---|---|---|
| Was | je Ziel andere Dateien/Verzeichnisse; Board-Beschreibung als Daten | S3-Zweig in jede der 31 Dateien mit Board-Bedingung | Funktionszeigertabellen je Baustein, Board per Laufzeit | (a) für Klasse H; je ein Guard in 4 gemischten Dateien; Board-Beschreibung (§5) |
| Preis am Bestand | rein (a) **nicht machbar** für M/G: `main.c` 5 378 + `irmp.c` 5 114 + `esp8266.c` 1 097 + `dcf77.c` 537 + `sk6812.c` 671 = **12 797 Zeilen** würden **geforkt** | **○ geschätzt +25 Zeilen** (ein Zweig mehr in rund 25 der 31 Dateien; 100 → ~125), jede S3-Änderung berührt Bestandsdateien, jede braucht Bestandsnachweis; der S3-Text ohne SPL passt nicht in Dateien, deren Typen SPL sind | **mind. 300 Byte** Tabellen (○ Rechnung: 14 UART-Funktionen × 3 Instanzen = 42 Zeiger, dazu adc 3, i2c 3, onewire 8, delay 4, button 2, wps 2, board-led 3, power 3, LED 6 = **76 Zeiger = 304 Byte**, ohne Code) gegen **1 172 Byte frei** (F103 24h) = **26 %** des Spielraums, für **null** Nutzen auf dem STM; ISR-Pfade (IRMP-Pinlesen alle 66,7 µs) bekommen einen indirekten Aufruf (○) | 4 Guards (neue Bedingungsfamilien, namentlich gezählt); 0 Byte Änderung |
| Bestand bleibt gleich? | ja, aber nur durch Fork | ja (Zweig nicht genommen) | **nein** — Binärgleichheit ausgeschlossen | **ja, byte-identisch** (AK13) |
| Wie bricht es | Fork driftet: der Treiber ändert sich an einer Stelle, nicht an der anderen — **das ist der Fehler, den der Nutzer ausschliessen will** | still: ein Zweig kompiliert nicht, weil niemand dieses Board baut (heute 2 von 72 routinemässig) | zur Laufzeit: falsche Tabelle, kein Compilerfehler | Guard-Region zu gross/zu klein → Hash-Vergleich schlägt an (§6) |
| Wie prüft man es | Dateiliste gegen Klassifikation | nur durch Bau jedes Boards | nur am Gerät | Hash-Vergleich (72+1), Attrappen-Übersetzung, Zähler |
| Wartung je neues Board | nur Dateien + Board-Beschreibung | n Dateien anfassen | Tabelle ergänzen | Board-Beschreibung + Klassifikation |

**Empfehlung: (a) mit benannten Seam-Guards.** Begründung nach **Risiko, nicht nach Vorliebe**: (1) Das Einzige, was ein Bestandsboard **still verändern** kann, ist ein Eingriff in `src/**`; diese Empfehlung hält den Tokenstrom fest und macht das **messbar** (Hash).
(2) Die zwei billigen Optionen scheitern an der **einen Zahl**, die zählt: (b) an der Wartung und am Nachweis, (c) am Flash des F103. (3) Reines (a) scheitert an 12 797 Zeilen, die niemand zweimal pflegen soll.
**Was bleibt von (b):** ein `#if` auf **Funktion** (`WC_*_EXTERNAL`), nie auf Board. Der Zähler (§6.1 C3) unterscheidet **Board-/Familienmakros** (gedeckelt, sinkend) von **Seam-Guards** (genau vier, benannt).

---

## 5. Die Board-Beschreibung

### 5.1 Form — Empfehlung

Je Board ein Verzeichnis `boards/<id>/`:

| Datei | Inhalt | Wer schreibt | Wie gepflegt |
|---|---|---|---|
| `board-pins.h` | **die Signale**: je Signal Netzname, Pin(s), Richtung, aktiver Pegel | **S3: erzeugt** aus dem maschinenlesbaren Teil von `hardware/architektur/pinbelegung.md` (Q7), **Zwischenlösung: handgeschrieben + Abgleichprüfung**; STM: handgeschrieben (Stufe 5) | niemals ein zweites Mal von Hand neben `pinbelegung.md` |
| `board.h` | **die Merkmale** (§5.3) und `#include "board-pins.h"` | handgeschrieben | wie Code, geprüft |

**Warum zwei Dateien:** Pins sind **Hardwarewahrheit** (`hw-architect`, Schaltplan) und dürfen nur **aus** dieser Quelle kommen; Merkmale (EEPROM-Seitengrösse, Tickrate, Konsolensenken) sind **Softwareentscheide**. Wer beides in einer Hand-Datei pflegt, hat zwei Quellen für Pins.
**Der S3 hat nur eine Beschreibung für DevKit und Nachbau** (V8): Das DevKit ist der Nachbau mit Kabeln; es gibt **kein** `devkit.h`.

### 5.2 Signale (heute, ✔ aus den Hardwaredateien) — 11 Kernsignale

| # | Signal | BlackPill (Bestand) | S3 (Vorschlag der S3-Spec ●, **nicht abgeglichen**) |
|---|---|---|---|
| 1 | `LED_DATA` | PB1 (TIM3 CH4, DMA1 S2) | GPIO5 (RMT) |
| 2 | `LED_POWER` | PB0 | GPIO6 |
| 3 | `ONEWIRE` | PB5 | GPIO2 |
| 4 | `LDR_ADC` | PA5 (ADC1, Kanal 5) | GPIO1 (ADC1) |
| 5 | `I2C_SCL` | PB6 (I2C1) | GPIO9 |
| 6 | `I2C_SDA` | PB7 | GPIO8 |
| 7 | `IR_IN` | PB4 | GPIO7 |
| 8 | `DCF77_DATA` (+ `PON` PB9) | PB8 | GPIO15 |
| 9 | `BTN_USER` | PA0 | GPIO16 |
| 10 | `BTN_WPS` | PA7 | GPIO17 |
| 11 | `STATUS_LED` | PC13 (aktiv low) | GPIO18 |

Dazu, **nur Bestandsboards**: Brücken-UART (USART2 PA2/PA3), Log-UART (USART1 PA9/PA10), Steuerpins zur Gegenstelle (PB10, PB3, PA4), DFPlayer (USART6 PA11/PA12, auf V2 **unbeschaltet**, ● `HARDWARE.md`). **Auf dem S3 entfallen** Brücken-UART und Steuerpins (ein Chip),
die Log-Senken sind UART0/USB (§3.5.4). Die Pinzuordnung des S3 entscheidet `hw-architect`; diese Spec **widerspricht ihr nicht** und fordert nur: **Kein Pin der Sperrliste** (GPIO0, 3, 45, 46, 19/20, 43/44, 35–37, 38, 48; V8).

### 5.3 Merkmale (Schlüssel, die jedes Board setzt)

| Schlüssel | Bedeutung | Heute abgeleitet aus | Beispiel BlackPill / S3 |
|---|---|---|---|
| `BOARD_ID`, `BOARD_NAME_TEXT` | Kennung, Text für „Hardware: …“ (`main.c:3812ff.`) | `#if BOARD`-Kette in `main()` | `"STM32F411CE BlackPill"` / `"ESP32-S3"` |
| `BOARD_HW_CHIP_CODE`, `BOARD_HW_OSC_CODE` | Felder im `hardware_configuration`-Wort | `main.h:67-82`, `:109-114` | `HW_STM32_F411CE`, `HW_OSC_FREQUENCY_25MHZ` / **Q4** |
| `BOARD_LED_TYPE` | Vorgabe-LED-Typ | `display-config.h:30-36` | `SK6812_RGBW_LED` / `SK6812_RGBW_LED` |
| `BOARD_TICK_HZ` | **echte** Rate der Zeitbasis (`F_INTERRUPTS`) | `irmpconfig.h`, `main.c:720-795` | 15 000 / aus S3-Spec F14 |
| `BOARD_EEPROM_PAGE_SIZE` | Seitengrösse des EEPROMs | `eeprom.c:44-49` (Familie!) | 32 / 32 (AT24C32, ● S3-Spec E4) |
| `BOARD_EEP_KIND` | I²C-EEPROM oder Flash-Backend | `eep.h` (`BLACK_BOARD`) | `I2C` / `I2C` |
| `BOARD_I2C_CLOCK_HZ` | Anfangstakt | `main.c:3744` (100 kHz, bei Fehlschlag 50 kHz) | 100 000 / 100 000 |
| `BOARD_LDR_ADC_BITS`, `BOARD_LDR_ADC_ATTEN_DB` | Skala und Dämpfung des LDR-Eingangs | `adc-config.h` | 12, — / 12, **6 oder 12 (offen)** |
| `BOARD_LED_POWER_ON_LEVEL`, `BOARD_STATUS_LED_ON_LEVEL` | aktive Pegel | `power.c`, `board-led.c` | high, low / **aus Schaltplan** |
| `BOARD_BUTTON_PULL` | Pull-up intern/extern/keiner | `button.c:60-71` | `INTERNAL_UP` / **S3-Spec F7 offen** |
| `BOARD_BRIDGE_KIND` | `UART` oder `QUEUE` | `esp8266-uart.c` | `UART` / `QUEUE` |
| `BOARD_HAS_PEER_CHIP` | zweiter Chip vorhanden | — | 1 / 0 |
| `BOARD_LOG_SINKS` | Senken des Log | `log-uart.c` | `UART1` / `UART0|USB_JTAG` |
| `BOARD_HAS_DFPLAYER`, `BOARD_HAS_TFT` | optionale Geräte | `dfplayer-uart.c`, `BLACK_BOARD` | 0, 0 / 0, 0 |

**Regeln.** (1) Jeder Schlüssel steht in **jeder** Beschreibung (ein fehlender ist ein Prüffehler; die **Zahl der Schlüssel** nennt die Prüfung, heute **19**). (2) **Kein Merkmal wird aus einem Chip- oder Boardmakro abgeleitet** (genau das tut `eeprom.c:44`). (3) `board.h` enthält **keine** Update-Adresse und keinen Hostnamen
(DIR-009: die Update-Vorgabe des S3 bleibt leer, S3-Spec 5.3c).

### 5.4 Auswahl im Build

Die Beschreibung wird über den **Include-Pfad** ausgewählt (`-I boards/<id>`), nicht über ein Makro: Die Logik schreibt `#include "board.h"`, nie `#if BOARD`. Bestandsboards behalten ihre Makros (`-DBLACKPILL_BOARD` usw.) bis Stufe 5; **bis dahin gilt:
`board.h` existiert für den S3, die Ketten für den Bestand** — und der Zähler friert die Ketten ein.

### 5.5 Verbindung zu `hardware/architektur/pinbelegung.md`

**Existiert heute nicht** (✔ `hardware/architektur/` ist leer, 17.10.2026). Die Prüfung AK9 braucht von `hw-architect` **eine** maschinenlesbare Form (Q7): eine Tabelle mit festen Spalten `Signal | Netz | GPIO | Richtung | Pegel | Bemerkung` **oder** eine JSON-Datei daneben. Daraus entsteht `board-pins.h` (Empfehlung) oder
wird damit **verglichen** (Zwischenlösung). In beiden Fällen prüft sie: Zahl der Signale (**11**), Sperrliste, **jede** Zeile an beiden Enden, und dass `esp32s3/**` kein GPIO-Literal ausserhalb von `board-pins.h` trägt.
Der Abgleich Netzliste ↔ Tabelle ist **Sache von `hw-reviewer`** (S3-Spec AK1), nicht dieser Prüfung.

---

## 6. Bau-Matrix und Schutz vor Auseinanderdriften

Zwei Zeitskalen, **weil der Bau nach R1 nur der Lead (bzw. `release-engineer`) auslöst**: Sekunden-Prüfungen laufen in `guardrails.sh` bei **jeder** Änderung an `src/**`, `boards/**`, `esp32s3/**`, `ESP8266/**`; der Bau-Vergleich läuft **bei jeder Änderung der Klasse H1/H2/A/F** (§9.1) und vor jedem Release.

### 6.1 Prüfungen (Eigentümer Lead, `tools/**`; C1–C6 Stufe 1 [Tasks 1.2–1.4, 1.7], C8 Task 1.6, C7 Task 3.5b, C9 Task 4.1)

| # | Prüfung | Läuft | Betrachtet (Zahl) | Schlägt an, wenn |
|---|---|---|---|---|
| **C1** | **Klassifikation vollständig** (`boards/modules.txt`) | `guardrails.sh` | **49** `.c` (47 + 2 Stubs); Abgleich gegen die `.c`/`.S`-Einheiten der **zwei** `.ebp` | Datei ohne Eintrag, Eintrag ohne Datei, Datei nur in einer `.ebp` |
| **C2** | **Attrappen-Übersetzung** der Klasse L | `guardrails.sh` (Sekundenbruchteile je Datei, ✔ 0,1 s für zwei) | **20 Dateien × 3 Boards = 60 Fälle** (BlackPill, BluePill, S3-Konfiguration); **in Stufe 1 erst 40** (ohne S3), 60 ab Task 2.3 | ein Fall mit Fehler; **0 Fälle** gilt als Fehler |
| **C3** | **Streuungszähler** je Datei: Board-Makros, Familienmakros, Seam-Guards, unbekannte Makronamen | `guardrails.sh` | **123** Dateien; Ausgangsstand 100/226 Zeilen, 31/57 Dateien; **`display.c` 0/0** | eine Zahl steigt; neue Bedingung in L; ein Guard ausserhalb des Katalogs (§3.6); ein **nirgends definierter** Chip-/Boardname (Bestand: `STM32F1XX` ×4, `DISCO_BOARD` ×5 — **benannte Ausnahmeliste**) |
| **C4** | **Dienstliste** (`boards/platform-services.txt`) | `guardrails.sh` | 7 Zeilen der Dienstliste; Verbraucher: 11 Dateien (ohne `log_*`) + 19 (`log_*`) | ein neuer Rückwärtsgriff einer Hardwaredatei; `delay_counter` in einer weiteren Datei |
| **C5** | **Pins** (S3) | `guardrails.sh` | **11** Signale | Zahl ≠ 11, Sperrliste, GPIO-Literal ausserhalb `board-pins.h`, Abweichung von `pinbelegung.md` |
| **C6** | **`pending_ticker_restore`-Block wörtlich** | `guardrails.sh` | 1 Block, **4** Teilbedingungen | eine Teilbedingung fehlt oder ist verändert |
| **C7** | **Layout-/Breitenprüfung** (`_Static_assert`) | je Ziel im Bau; Host-Fassung in `guardrails.sh` | Typen über EEPROM/Brücke/Backup; `uint_fast8_t`, `uint_fast16_t`, `time_t`, Vorzeichen von `char` | Grösse/Versatz/Breite weicht vom ARM-Wert ab |
| **C8** | **Hash-Vergleich** gegen die Baseline | `make matrix` + Vergleich (**Lead/`release-engineer`**, R1) | **72** Matrix-HEX + **1** ESP8266-`.bin`; die beiden Routineziele sind Teilmenge (nicht doppelt gezählt) | eine Summe weicht ab; Baseline fehlt (**kein** stilles Überspringen) |
| **C9** | **Symbolkollision S3** | S3-Bau (Lead) | **alle** globalen Symbole des Abbilds | ein Symbol doppelt |
| **C10** | **Flash-Gate F103** | `guardrails.sh` S8b + Matrix-Bericht | engste Variante **1 172 Byte** (●, 13.10.2026) | Verringerung ohne Freigabe (AK14) |

**Einbindung.** C1–C7 als neue Stufe(n) in `tools/guardrails.sh` (Nummer vergibt der Lead; ✔ letzte heute S18), **jede mit nicht gepufferter Meldung und Exitcode** (DIR-014, fünf Fälle der Gattung). Die Überwachungsliste des Stop-Hooks (`tools/hooks/guardrails-before-stop.py`) wird um
`boards/**` und `esp32s3/**` erweitert. C8 gehört in `--full` bzw. in den Release-Ablauf (Skill `/release`), nicht in den Sekundenlauf.

### 6.2 Baseline — erst beweisen, dann verlassen

**Der Hash-Vergleich trägt nur, wenn der Bau deterministisch ist.** Task 1.5 baut **zweimal frisch** und vergleicht (AK1). Offene Risiken, die der Task ausschliesst oder benennt: **Zeitmarken** im Abbild (✔ `__DATE__`/`__TIME__`/`__FILE__`/`__LINE__` kommen im Code **nicht** vor), Pfade in Debug-Info (nicht im HEX), **LTO** (`-flto=4`, mit fester Zahl von Link-Jobs
bewusst gewählt, `CMakeLists.txt:72`), **Werkzeugstand** (✔ `arm-none-eabi-gcc` aus `/Applications/ArmGNUToolchain/15.2.rel1`): ein Compilerwechsel macht **jede** Baseline ungültig; die Baseline trägt daher den **Werkzeugstand** und wird bei Wechsel **an unverändertem Quelltext** neu erzeugt.
**Die ESP8266-Summe** hängt am Arduino-Kern und am `arduino-cli` (Pfad im `Makefile`); gleicher Vorbehalt.

### 6.3 Was die Prüfung **nicht** abdeckt (ausdrücklich)

- **Verhalten.** Attrappen-Übersetzung ist **Syntax auf dem Mac** (Breiten anders als auf dem Ziel); Hash-Gleichheit belegt „dasselbe Abbild“, nicht „richtig“. Verhalten belegen **nur** Gerät und `pwa-tester` (DIR-012).
- **Die 70 Varianten ohne Gerät.** Auf der Test-Uhr läuft **eine** Variante (F411, V2). **F103 und alle Nicht-Test-Uhr-Varianten sind nur über den Hash belegt** (Q3).
- **Das TFT-Board** (`BLACK_BOARD`): die Klasse T wird von C2 **nicht** erfasst (sie ist ausserhalb der Black-Board-Konfiguration fast leer); nur `make matrix` baut sie (zwei von 72).
- **Der S3-Bau selbst:** C2 prüft **Übersetzbarkeit gegen Attrappen**, nicht gegen die IDF; der echte Bau ist ein Lead-Lauf (R1) und wird **nicht** in `guardrails.sh` ohne `--full` gefahren. Die ESP32-Fassung der Netz-Seite wird **nirgends** in der Sekundenstufe übersetzt (○ keine IDF auf dem Rechner, Q5).
- **Kombinationen ausserhalb der 72.** Die Matrix ist eine **benannte Teilmenge** der möglichen `#if`-Kombinationen, keine Vollständigkeit.
- **Die ESP-Seite.** Der S3-Port ändert sie über `#ifdef ESP32`; ob die ESP8266-Fassung dabei unverändert bleibt, belegt **nur** die ESP8266-Summe (C8) — sie ist mit demselben Vorbehalt behaftet wie 6.2.

### 6.4 Gegenproben (DIR-014) — jede neue Prüfung muss einmal fehlschlagen

Jede Prüfung nimmt **`--root <Verzeichnis>`** und läuft damit gegen eine **Kopie**; die Gegenprobe verletzt die Kopie, nicht den Baum, und zeigt **Exitcode und Meldung auf dem Kanal des Empfängers** (Lauf über `guardrails.sh`, Ausgabe nicht durch `| tail` oder Pufferung verschluckt).

| Probe | Verletzung (an einer Kopie) | Erwartung |
|---|---|---|
| **P1** (C1) | leere `src/zz-neu/zz.c` hinzufügen | Fehler „50 `.c`, 49 klassifiziert“; Exit ≠ 0 |
| **P2** (C2) | in `display.c` `#include "stm32f4xx_gpio.h"` und `GPIO_InitTypeDef x;` | Fehler mit Datei und Fall („display.c, Konfiguration S3“); Zahl der Fälle **60** im Kopf |
| **P3** (C3) | `#if defined (BLACKPILL_BOARD)` in `display.c`; ausserdem ein 5. Guard | Fehler „display.c: Board-Makros 1, erlaubt 0“; Fehler „Guard ausserhalb des Katalogs“ |
| **P4** (C4) | `i2c.c` ruft `extern uint32_t foo_from_main` | Fehler „neuer Rückwärtsgriff“ |
| **P5** (C5) | `board-pins.h` setzt GPIO19 | Fehler „Sperrliste“; zweite Probe: 10 statt 11 Signale → Fehler „Zahl“ |
| **P6** (C6) | `show_icon_stop_time == 0 &&` aus dem Block streichen | Fehler „Teilbedingung fehlt“ |
| **P7** (C7) | erwartete `sizeof` eines Brückentyps um 1 ändern | Übersetzungsfehler `_Static_assert` |
| **P8** (C8) | ein Byte in einer Baseline-Summe ändern; ausserdem Baseline-Datei entfernen | Fehler **mit Variantenname und beiden Summen**; zweite Probe: Fehler „Baseline fehlt“, **nicht** Überspringen |
| **P9** (C9) | in `esp32s3` eine Funktion `htoi` definieren | Fehler „doppelt“ |
| **P10** | **Weg der Meldung:** jede der Proben P1–P9 einmal **über `./tools/guardrails.sh`** laufen lassen | die Meldung steht in der Ausgabe, Exit ≠ 0; `tools/hooks/guardrails-before-stop.py` blockiert das Turn-Ende |

---

## 7. Verzeichnisse und Besitz (R3)

```
src/                         Logik, STM-Treiber, Seam-Guards              stm-developer   (R3: `^src/`)
boards/
  modules.txt                Klassifikation + S3-Umgang je .c             Lead            (Daten, wie tools/**)
  platform-services.txt      Dienstliste (§3.3)                           Lead
  baseline/                  Hash- und Zähler-Baseline                    Lead            (nur der Lead erzeugt sie)
  wordclock-esp32s3-v3/      board.h, board-pins.h                        s3-developer
  <stm-boards>/              (erst Stufe 5)                               stm-developer
esp32s3/                     IDF-Projekt: main/, hal/, shim/, test/,      s3-developer
                             sdkconfig.defaults, partitions.csv, linker.lf
ESP8266/ESP-uclock/          Netz-Seite beider Ziele (#ifdef ESP32)       esp-developer   (unverändert `^ESP8266/…\.(cpp|h|ino)$`)
tools/checks/boards-*        die Prüfungen C1–C7, C9                       Lead
Makefile, CMakeLists.txt     Ziele, Matrix-Vergleich, S3-Ziel             release-engineer
```

**Rollenvorschläge für den S3-Teil (anlegen darf nur der Lead):**

| Rolle | Neu? | Schreibt | Schreibt **nicht** |
|---|---|---|---|
| **`s3-developer`** | **ja** | `esp32s3/**`, `boards/wordclock-esp32s3-*/**` | `src/**`, `ESP8266/**`, `tools/**`, `boards/modules.txt`, `boards/baseline/**` |
| Reviews der HAL und der Seam-Backends | nein — `firmware-analyst` (lesend) gegen `knowledge/architecture-checklist.md` | — | — |
| Prüfwerkzeug C1–C9 | nein — Lead | `tools/**`, `boards/modules.txt`, `boards/platform-services.txt`, `boards/baseline/**` | Produktcode |

**Der Besitz-Hook ist eine Erinnerung, kein Zwang** (CLAUDE.md, R3): Das Wurzelmuster (`tools/hooks/file-ownership.py:109`) kennt `boards/` und `esp32s3/` **nicht**; **ohne Eintrag lässt er Schreibzugriffe dorthin ungeprüft durch**. Der Lead ergänzt: Rolle `s3-developer`, Wurzelmuster, `stm-developer`-Ausnahmen für die Dateien, die ihm **nicht**
gehören (`boards/modules.txt`, `boards/baseline/**`, `boards/wordclock-esp32s3-*`), und in `tools/hooks/no-build.py` den Aufruf `idf.py` (S3-Spec Task 0.3). **Beobachtung:** `src/main.h` hat **zwei** zugelassene Schreiber (`stm-developer` über `^src/`, `release-engineer` über
`^src/main\.h$`, ✔ `file-ownership.py:51,70-71`). Für §9.3 E1 gilt deshalb **R3b**: erst der `stm-developer` (Zweig im Konfigurationswort), danach der `release-engineer` (Versionszeile) — **nie gleichzeitig**.

---

## 8. Die Ein-Chip-Variante: zwei Welten, ein Abbild

### 8.1 Framework — Entscheid mit Preis

✔ **Stand auf diesem Rechner:** Arduino-ESP32 **3.3.12** mit **IDF 5.5.5** (`~/Library/Arduino15/packages/esp32/tools/esp32s3-libs/3.3.12`, `versions.txt`), Toolchain `esp-x32` 2601, `openocd-esp32` v0.12.0-esp32-20260424; **kein `idf.py`** (`which idf.py`: nicht gefunden).
✔ **Fest eingebaute Konfiguration der vorkompilierten Bibliotheken** (`…/esp32s3-libs/3.3.12/sdkconfig`): `CONFIG_GPTIMER_ISR_HANDLER_IN_IRAM=y`, aber `# CONFIG_GPTIMER_ISR_CACHE_SAFE is not set`, `# CONFIG_RMT_ISR_IRAM_SAFE is not set`, `# CONFIG_I2C_ISR_IRAM_SAFE is not set`,
`CONFIG_ESP_TASK_WDT_TIMEOUT_S=5`, `CONFIG_ESP_CONSOLE_UART_DEFAULT=y` **und** `CONFIG_ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG=y`, `CONFIG_SPIRAM=y` (Quad, Auto), Flash 16 MB, `CONFIG_ARDUINO_RUNNING_CORE=1`.

| Option | Was | Preis | Bricht, wenn … | Prüfung |
|---|---|---|---|---|
| **O1 ESP-IDF 5.5.x + Arduino-ESP32 als Komponente** (**Empfehlung**) | `idf.py` im Verzeichnis `esp32s3/`; die Netz-Seite nutzt die Arduino-API (`WiFiClient`, `LittleFS`, `HTTPUpdate`) | **drei** Werkzeugketten im Repo (arm-none-eabi/CMake, arduino-cli/ESP8266, idf.py); IDF **muss installiert werden** (Q5); Version an Arduino-ESP32 gebunden (✔ 3.3.12 ↔ 5.5.5); `no-build.py` um `idf.py` | die Komponente verlangt eine andere IDF-Version als installiert | S3-Task 1.1 zeigt, dass **jeder** gewünschte Schalter wirkt |
| **O2 `arduino-cli` mit Arduino-ESP32** | dasselbe Werkzeug wie `make esp`; ein Make-Ziel mehr | **Kein Zugriff auf die Kconfig:** ✔ Cache-sichere ISRs (`GPTIMER_ISR_CACHE_SAFE`, `RMT_ISR_IRAM_SAFE`, `I2C_ISR_IRAM_SAFE`) sind in den Bibliotheken **nicht** gesetzt; Task-Watchdog 5 s (zur Laufzeit änderbar, ○ `esp_task_wdt_reconfigure`); Partitionstabelle über Boardoptionen; eigene Bibliotheken nur über `lib-builder` (schwer) | die **Timer-ISR bei einem Flash-Schreibzugriff** (LittleFS, OTA) maskiert wird: IRMP und Softuhr stehen für die Dauer (○ Mechanismus nach IDF-Regel: ISRs ohne `ESP_INTR_FLAG_IRAM` sind bei gesperrtem Cache aus; Dauer nicht gemessen) | **S3-AK16** (100 Schreibvorgänge, Zähler auf ±0,1 %) |
| **O3 PlatformIO** | `espidf`+`arduino` | ein **vierter** Werkzeugstand, nicht im Repo, nicht in den Hooks | wie O1 | — |

**Empfehlung O1**, **○ ungeprüft bis S3-Task 1.1**. **Entscheidungspunkt für den Nutzer:** Verzichtet er auf AK16 (Interrupt bleibt bei Flash-Schreiben fehlerfrei), wird O2 **billiger und ausreichend**. Das ist die einzige Anforderung, die O2 ausschliesst; **ich rate sie nicht aufzugeben**, weil sie IRMP (Fernbedienung)
und die Softuhr trägt.

### 8.2 Wie die Quellen im Abbild landen

Zwei IDF-Komponenten, **getrennte** Include-Pfade (kein gemeinsames Verzeichnis, in dem gleichnamige Kopfdateien liegen, AK17):

| Komponente | Quelle | Dateiliste aus | Compile-Optionen |
|---|---|---|---|
| `stm_core` | `src/**` Klassen L, M, G; dazu `esp32s3/hal/*` für Klasse H; **Seam-Backends** per `-I esp32s3/hal`; Attrappen `esp32s3/shim/` | `boards/modules.txt` (Spalte S3: `use`, `replace`, `drop`) | `-std=gnu11 -fno-strict-aliasing` wie Bestand (✔ `CMakeLists.txt`); dazu die Schalter für ARM-Semantik von `char`/Aufzählungen (Task 3.5 belegt, AK11); `-DWC_PLATFORM_EXTERNAL -DWC_LED_BACKEND_EXTERNAL -DWC_IRMP_TARGET_EXTERNAL -DWC_PEER_EXTERNAL`; `-include stdint.h -include stdbool.h -include stddef.h`; `-DDSP_USE_LED=SK6812_RGBW_LED -DWCLOCK24H=<Q13>` |
| `net_core` | `ESP8266/ESP-uclock/*.cpp`, `.ino` (Zweig `#ifdef ESP32`) | Sketch-Ordner (Dateinamen, **keine** zweite Liste) | Arduino-ESP32 |

**Kollisionen** (✔ 5 Kopfdatei- und 5 Quelldateinamen, ≥ 6 Funktionsnamen gleich): (1) getrennte Komponenten = getrennte Objektnamen und Include-Pfade; (2) **C9** vergleicht nach dem Linken die globalen Symbole **beider** Seiten mit `nm` und meldet **jede** Doppelung (Zahl der geprüften Symbole im Bericht);
(3) eine Doppelung wird **ohne Änderung des Bestands** über `-D<alt>=<neu>` **für eine Komponente** aufgelöst (`-Dhtoi=net_htoi` für `net_core`), **nicht** durch Umbenennen im Quelltext. `VERSION` (nur `stm_core`, `main.h`) und `ESP_VERSION` (nur `net_core`, `version.h`) kollidieren **nicht**, solange keine der Komponenten die andere Kopfdatei einbindet — C3/C9 halten das fest.

### 8.3 Versionsstellen und das `grep`-Format

**Das Zeilenformat ändert sich nicht** (V6): `#define VERSION "x.y.z"` in `src/main.h` und `#define ESP_VERSION "x.y.z"` in `ESP8266/ESP-uclock/version.h` sind weiter je **genau eine** Zeile (S4 meldet „genau einmal“). Beide Symbole **bleiben**, weil die PWA zwei Versionen zeigt
(S3-Spec F13). **Das offene Problem ist nicht das Format, sondern die Zuordnung** (W7): Das S3-Abbild enthält `src/**` **und** `ESP8266/**` **und** `esp32s3/**`; eine Änderung **nur** an `esp32s3/**` bumpt keine der zwei Versionen (DIR-004), das Fabrikat wäre unzuordenbar.
**Empfehlung (Q2): ein drittes Symbol `S3_VERSION`** im **selben Format** (`#define S3_VERSION "x.y.z"`, eine Zeile, `grep '^#define S3_VERSION' … | head -n 1 | cut -d'"' -f2`), geführt vom `release-engineer`, gebumpt bei **jeder** Änderung am Abbild (`esp32s3/**`, `boards/wordclock-esp32s3-*`, **oder** einer
eingebundenen Quelle); `VERSION` und `ESP_VERSION` laufen als Komponentenversionen weiter. Preis: ein Make-Ziel (`s3-version-file`, gleiche Form wie `esp-version-file`), eine Zeile in S4, ein Tag-Schema (`release/<stm>-<esp>-<app>` passt nicht von selbst; **Entscheid des Nutzers**). **Die Alternative `ESP_VERSION` mitführen
ist ausgeschlossen:** Das zwänge jeden STM-Commit zu einem ESP8266-Bump und böte ein OTA auf identische ESP8266-Firmware an (DIR-004).

### 8.4 Was von den Direktiven für den S3 entfällt oder ersetzt wird

| Direktive | Bestandsboards | S3 |
|---|---|---|
| DIR-002 (voller Bau, Release-ZIP) | gilt | das ZIP enthält ein weiteres Abbild; Flash-Umfang je Ziel benennen, **Firmware zuerst** |
| DIR-004 (Versionspflicht) | gilt je Komponente; Quellen STM = `src/**`, `CMakeLists.txt`, `cmake/**` (`guardrails.sh:80`) | `boards/**` kommt in die STM-Quellen (Q10); S3-Version nach Q2 |
| DIR-005 (Rollout Synology) | gilt | S3-Abbild und Versionsdatei **dazu**; **nie** `--delete` |
| DIR-009 (Smoketest, Update-Quelle) | gilt | gilt; die Update-Vorgabe des S3 bleibt **leer** (sonst Fremdfirmware, 03.10.2026) |
| DIR-010 (STM32 flashen) | gilt | **entfällt** (kein STM); ersetzt durch `tools/flash-s3.sh` (S3-Spec 6.2), das **die gemeldete Version nachweist** |
| DIR-011 (Commit, Tag, Push) | gilt | Tag-Schema nach Q2 |
| DIR-012/013/016 | gilt | `pwa-tester` und Logwache **am S3**; die Logwache liest die Konsole (UART0/USB) |
| DIR-014 | gilt | **alle** Prüfungen aus §6 haben einmal versagt (P1–P10) |
| DIR-017 (PWA nicht im Rollout) | gilt | das LittleFS des S3 ist **leer**: `install-app.sh` zuerst, `--check` danach |

---

## 9. Übergang ohne Veränderung der Bestandsboards

### 9.1 Änderungsklassen und ihr Nachweis

| Klasse | Was | Beispiel | Nachweis |
|---|---|---|---|
| **H1** | Konstanten, Kommentare, Makros **verschieben oder umbenennen**, Tokenstrom nach Präprozessor gleich | Zeiten (ns) in eine Kopfdatei | **alle 72 + ESP8266 hash-gleich** |
| **H2** | Text unter einen **Seam-Guard** stellen (Bestandszweig bleibt unverändert) | `WC_LED_BACKEND_EXTERNAL` um den Hardwareblock | **hash-gleich**; zusätzlich: Guard-Zweig **nicht genommen** (kein neuer Code im Bestand) |
| **A** | **additiver** Zweig, der im Bestand nicht genommen wird | S3-Zweig im Konfigurationswort `main.h`; `BOARD_EEPROM_PAGE_SIZE` vor der Familienkette | hash-gleich |
| **F** | **Funktionsänderung** am Bestand (Reihenfolge, Frist, Typ, Fehlerpfad) | Frist im `refresh`-Wait; Verlegen von `dcf77_tick`-Log; Korrektur `STM32F1XX` **falls** sie Code verändert | **Gerätetest** (`smoke-device.sh` **und** `pwa-tester`, DIR-012) **plus** Flash-Differenz je Variante (AK14); **F103: ohne ausdrückliche Freigabe keine Klasse-F-Änderung an Dateien, die der F103 baut** (Q3) |

**Hash-gleich ⇒ derselbe Fabrikinhalt ⇒ nichts zu flashen, nichts zu testen.** Das ist der Grund, warum Klasse H1/H2/A der Weg ist, und der Grund, warum der Hash **bewiesen deterministisch** sein muss (AK1, 6.2). Weicht ein Hash ab, **entscheidet der Lead**, ob der Task falsch ist oder nach F gehört (AK13) — nicht der Umsetzer.

### 9.2 Stufen

| Stufe | Inhalt | Ändert Bestand? | Voraussetzung |
|---|---|---|---|
| **0** Messen | Rollen/Hook (Lead), Freigabe, **Baseline**: zweimal bauen, 72 + ESP, Hash-Datei (AK1) | **nein** | — |
| **1** Schutz | `boards/modules.txt`, `platform-services.txt`, Prüfungen C1–C7 in `guardrails.sh`, Gegenproben P1–P8, P10 | **nein** | 0 |
| **2** S3 additiv | `boards/wordclock-esp32s3-v3/`, `esp32s3/shim/` (Attrappen), Gerüst, Quellliste aus `modules.txt`; der S3 baut die Klasse L **19 von 20** | **nein** | 1; `pinbelegung.md` (Q7) |
| **3** Nähte | E1 `main.h` (A), E3 `eeprom.c` (A), `sk6812.c` (H2), `main.c` (H2), `irmp` (A/H2), `esp8266.c` (H2/F nach Q11), jeweils **hash-geprüft**; danach Gerätetest **nur**, wenn ein Task F war | **ja**, aber **hash-gleich** | 0, 1 |
| **4** S3-Funktion | S3-Spec Stufen 2–6 auf diesen Nähten | nein | 2, 3 (je nach S3-Task) |
| **5** Aufräumen *(optional, **nicht freigegeben**, Q1)* | Pins der Bestandsboards nach `board.h`; falsche Kommentare; `STM32F1XX`; Header auf `platform.h`; `boards/**` in DIR-004 (Q10) | **ja**, H1/A, **hash-geprüft** | S3 zeigt Licht (S3-Spec Stufe 2) |

**Verhältnis zur S3-Spec (E7: LED zuerst):** Die LED-Stufe der S3-Spec (2) braucht `sk6812.c` mit Backend-Guard (**H2, Stufe 3**) — **nicht** `main.c`. Stufe-2-Testprogramme der S3-Spec dürfen deshalb früh laufen; `main.c` wird erst in S3-Task 5.1 verlangt. **Die zwei Stufen laufen nicht gegeneinander** (R3b-Tabelle in `tasks.md`).

### 9.3 Eingriffe in Bestandsdateien, die der S3 unvermeidbar macht (Stand der Messung)

Gemessen mit dem S3-Versuch (§3.2). **Weitere Stellen liefert das Inventar nicht vollständig; S3-Task 0.4 ergänzt sie hier**, und jede Ergänzung ist **freigabepflichtig** (AK7).

| # | Datei, Stelle | Eingriff | Klasse | Beleg |
|---|---|---|---|---|
| **E1** | `src/main.h:67-82` (Chip-Kette) und `:109-114` (Oszillator-Kette), je mit `#error` | Zweig für `BOARD_HW_CHIP_CODE`/`BOARD_HW_OSC_CODE` aus `board.h` | A | ✔ `main.h:82`, `:114` `#error` bei fehlendem Chip bzw. Oszillator; Q4 |
| **E2** | `src/main.c`, vier Regionen (§3.6) | `WC_PLATFORM_EXTERNAL`-Guard, Quittungsmakro `WC_TICK_ACK()` | H2 | ✔ 91 Herstellerzeilen, zusammenhängende Blöcke |
| **E3** | `src/eeprom/eeprom.c:44-49` | `#ifdef BOARD_EEPROM_PAGE_SIZE` **vor** der Familienkette | A | ✔ `#error EEPROM_PAGE_SIZE` (einziger Logikfehler im S3-Versuch) |
| **E4** | `src/irmp/irmp.h`, `irmpconfig.h`, `irmp.c` (Pin-Zugriff, `irmp_init`) | `WC_IRMP_TARGET_EXTERNAL`-Zweig | A/H2 | ✔ Zielsystemketten (PIC, STM32, Stellaris), `#error target system not defined` |
| **E5** | `src/sk6812/sk6812.c` Hardwareblock | `WC_LED_BACKEND_EXTERNAL` | H2 | ✔ §3.6 |
| **E6** | `src/esp8266/esp8266.c` Steuerpins/Boot-Pfad | `WC_PEER_EXTERNAL` | H2 (F nach Q11) | ● Inventar |
| **E7** | `src/dcf77/dcf77.c` Pin | Pin aus `board.h` | A | ✔ 29 Herstellerzeilen, nur Pinwahl |
| — | **nicht nötig**: `display.c`, `vars.c`, `tables.c` u. a. (Klasse L ausser `eeprom.c`) | — | — | ✔ 19 von 20 übersetzen unverändert |

### 9.4 Nachweis, dass die Bestandsboards unverändert sind

1. **Baseline (Stufe 0):** SHA-256 aller 72 Matrix-HEX **und** des ESP8266-Abbilds, mit Werkzeugstand, in `boards/baseline/`.
2. **Nach jedem Task der Stufe 3 und 5:** gleiche Läufe, Vergleich (C8). Bei Gleichheit: Fabrikat identisch, **Task abgenommen**.
3. **Bei Abweichung:** der Lead entscheidet (9.1); Klasse F verlangt **Gerätetest** und die **Flash-Differenz** je Variante; **F103** zusätzlich: Freigabe, wenn der Spielraum (1 172 Byte, engste Variante) sinkt.
4. **Der Gerätetest an der Test-Uhr** (F411, V2) belegt **nur diese eine Variante**; die übrigen 71 bleiben Hash-belegt (§6.3). Das steht im Bericht, nicht im Kleingedruckten.

---

## 10. Prüfung gegen die Architektur-Checkliste

**Proper architecture** — *Bleibt die PWA parallel zu Legacy?* Ja: Diese Spec berührt `ESP8266/ESP-uclock/data/app/**` **nicht** (V4, `git diff` leer, S3-Spec AK21); Legacy bleibt Stabilitäts-Referenz, auch auf dem S3. *Wetter-Endpunkte?* Unberührt (`/api/weather_get_now`, `/api/weather_get_forecast`; kein Rückbau auf den
Legacy-Bypass). *Restore-Bedingung?* Bleibt **wörtlich**: Der Block `main.c:4910-4914` liegt ausserhalb **aller** Seam-Regionen (die Regionen betreffen Takt, Reset/Watchdog, Zeitgeberinit und die ersten Zeilen von `main()`), und **C6** vergleicht seinen Text gegen eine Referenz (alle **vier** Teilbedingungen). Dass der Display-Zustand auf dem S3 in
**einem** Task liegt, macht keine Teilbedingung überflüssig (S3-Spec E5). *Richtige Schicht?* Ja: Die Unterschiede stehen **an der Hardware** (Board-Beschreibung, HAL-Dateien, Seam-Backends); die Logik bleibt unberührt; nichts wird in `app.js` umgangen.
> **Antwort:** erfüllt, mit der benannten Lücke, dass `main.c` ein Fünftel dieser Regionen berührt und der Block **nicht** in ihnen liegt — Beweis ist C6 plus der Hash.

**Scalable systems** — *STM-Kommandos?* Keine neuen: die Spec fügt **keine** Funktion hinzu (V10), das Protokoll bleibt. *Byte pro Minute auf der UART?* Unverändert 0 zusätzliche; auf dem S3 entfällt die UART (S3-Spec). *Pfade unter 20 s Watchdog?* Es entsteht **kein** neuer blockierender Pfad: Die Klasse-F-Kandidaten
(Frist im `refresh`-Wait, `dcf77`-Log) **verkürzen** Blockaden oder verlegen Aufrufe; sie stehen **ausserhalb** dieses Umbaus (Q12) bzw. unter Gerätenachweis. Die `watchdog_reload`-Aufrufstellen bleiben die **zehn** aus S7 (`WD_EXPECTED=10`); der S3 ergänzt **eine** im Netz-Task (S3-Spec). *Hauptloop-Blockade?* Unverändert; `eeprom_write` bleibt
seitenweise (≈ 15 ms je geänderter Seite, ● Checkliste). **Harte Grenze dieser Spec: Flash des F103** — **1 172 Byte** frei in der engsten Variante (●, 13.10.2026), darum **keine Laufzeit-Schicht** (§4) und **kein** Byte Änderung ohne Freigabe (AK14). *Ticks und Interrupts:* der Timer-ISR-Körper bleibt **derselbe Text**
(§3.7); die Last des S3 (20 kHz?) ist S3-Spec F14. *Wachsende Tabellen?* Die Variantenzahl ist **72 + 1**, fest in drei Prüfungen verankert (CMake, Matrix-Bericht, C8); jedes neue Board **erhöht** sie an einer benannten Stelle, nicht still.
> **Antwort:** erfüllt; die kritische Ressource ist der F103-Flash, und die Empfehlung ist so gewählt, dass sie **null Byte** kostet.

**Secure by design** — *`innerHTML`/`escapeHtml`?* Nicht betroffen (keine PWA-Änderung). *Fremddaten?* Die Update-Vorgabe des S3 bleibt **leer** (DIR-009: der ESP fiel schon einmal auf den Server des Ursprungsprojekts zurück, 03.10.2026); `board.h` enthält **keinen** Host, keinen Pfad, keine Zugangsdaten. *Credentials im Log?* Der S3
bekommt zwei Log-Senken (UART0, USB): **beide sind lesbar von jedem mit Zugriff auf die Buchse**, und USB-Serial/JTAG gewährt Lese-/Schreibzugriff auf den Flash (S3-Spec, „Nicht Teil“). **Die Log-Schicht darf keine Schlüssel ausgeben** (`cmd: set weather appid = …` ist heute ein Befund bei Diagnosebau, `CMakeLists.txt:16-39`);
das Streuungsgrep aus S3-Task 5.7 prüft es. *Neue Endpunkte?* Keine. *Neue Angriffsfläche durch die Spec selbst?* Die Prüfwerkzeuge schreiben nur in `boards/baseline` und `/tmp`-Kopien; kein Netz.
> **Antwort:** erfüllt für diese Spec; die zwei Senken sind der einzige neue Zugang und stehen in S3-Spec Task 5.7.

**Stable & reliable** — *Wird jeder Fehler ausgewertet?* Jede Prüfung meldet **auf dem Kanal des Empfängers mit Exitcode** (DIR-014) und nennt die **Zahl der betrachteten Fälle**; **0 Fälle ist ein Fehler**, nicht ein „OK“ (der sechste Fall der Gattung, BEFUNDE L235). *Leere `catch`/stille Verwerfung?* **Eine fehlende Baseline ist ein Fehler**, kein
Überspringen; eine Datei ohne Klassifikation **schlägt an**, statt ignoriert zu werden; der Hash-Vergleich nennt **Variante und beide Summen**. *Stille Zurechtbiegung?* Der Hash-Vergleich ist **kein** Schwellenwert, sondern Gleichheit. *Zustand nach Abbruch mitten in einer Stufe?* Jede Stufe hat einen **Rückweg** (git), und Stufe 3 arbeitet **Task für Task** mit Hash-Vergleich — ein
Abbruch lässt den Baum **entweder** hash-gleich **oder** mit einem benannten, abgelehnten Task. *Flags auf jedem Pfad aufgelöst?* Die Seam-Guards sind **zur Übersetzungszeit** entschieden; es gibt **kein** Laufzeit-Flag, das zu setzen oder aufzulösen wäre. *Die `WiFiClient`-Leseschleife?* Gegenstand der Netz-Seite des S3-Ports (S3-Spec 5.3b);
diese Spec stellt nur sicher, dass **dieselbe** Quelle gilt.
> **Antwort:** erfüllt, mit der benannten Voraussetzung, dass der Bau **deterministisch** ist (AK1) — ohne sie entfällt jedes „binärgleich“ dieser Spec, und die Spec sagt, was dann gilt.

---

## 11. Verworfene Alternativen

1. **Reine Optionen (a), (b), (c)** — §4, mit Zahlen.
2. **Dateien in neue Verzeichnisse verschieben** (`src/logic/…`, `src/hal/…`). Verworfen: `git blame` wäre für Tausende Zeilen verloren (DIR-015-Lehre, 432 statt 9 Zeilen); 219 + 155 `.ebp`-Pfade, die CMake-Filter und alle `-I`-Pfade müssten folgen; **kein** Gewinn an Naht. Die Klasse steht in `boards/modules.txt`, **nicht im Pfad**.
3. **Zweiter Quellbaum für den S3** (Fork von `main.c`, `irmp.c`, `esp8266.c`, `dcf77.c`, `sk6812.c`): 12 797 Zeilen doppelt — das ist exakt der Auseinanderdrift, den der Nutzer ausschliesst.
4. **Hersteller-Header normalisieren** (26 Kopfdateien auf `platform.h`) **als Voraussetzung** für den S3: ändert Bestand und blockiert den S3 an einer Freigabe. **Verschoben** nach Stufe 5 (Q1), nicht verworfen.
5. **Laufzeit-Schicht** — §4, 304 Byte Tabellen gegen 1 172 Byte Spielraum.
6. **Kern des LED-Treibers in eine Einbindedatei auslagern:** ändert die Definitionsreihenfolge, damit Adressen und Hash (§3.6). Ersetzt durch Backend-Einbindung an der Stelle des Hardwareblocks.
7. **Gemeinsame Kopfdateien für das Brückenprotokoll und die Layouts** (`eeprom-data.h`↔`eepromdata.h`, `vars.h`, `tables.h`, `HW_*`, `htoi`, `strsubst`, `dayofweek`, `var_crc`): sinnvoll, aber **ändert beide Seiten und das Protokoll**; die Prüfstände (`var-crc.c`, S13/S16) decken den Bestand. **Folgespec**, nicht hier.
8. **`esp8266.c` auf dem S3 entfernen** (Inventar-Empfehlung): widerspricht E5 und sechs Logikmodulen (W10).
9. **Ein Code-Generator für `board.h` aller Boards:** zu früh; nur für `board-pins.h` des S3 **und nur wenn** `pinbelegung.md` maschinenlesbar wird (Q7).
10. **KiCad-Netzliste als Pinquelle der Software:** Die Netzliste ist **Hardwarewahrheit** und gehört `hw-reviewer` (S3-Spec AK1); die Software liest `pinbelegung.md`, nicht die Netzliste.
11. **Die Attrappen als erzeugte Header aus dem Hersteller-SDK:** Das wäre eine zweite Kopie der Hersteller-Header; die Attrappen sind **absichtlich leer** und werden vom Zähler überwacht.
12. **Den Hash durch den Gerätetest ersetzen:** Ein Gerät belegt **eine** der 72 Varianten.

## 12. Versionsfolgen

- [ ] STM `src/main.h` anheben — **nur bei Änderung der Klasse F** (Stufe 3/5); bei **nachweislich hash-gleichem** Umbau **nicht** (Frage Q9; bis dahin **bumpen**)
- [ ] ESP `version.h` anheben — erst mit den `#ifdef ESP32`-Zweigen der Netz-Seite (S3-Spec 5.3), **nicht** durch diese Spec
- [ ] App `APP_VERSION` — **nein** (PWA unberührt)
- [ ] `CACHE_NAME` in `sw.js` — **nein**
- [ ] `S3_VERSION` — **falls Q2(a)** entschieden wird, Neuanlage durch den `release-engineer`

Nur der `release-engineer` führt das aus (R4).

## 13. Ergebnisse der Klärungs-Tasks — hier eintragen

| Task | Ergebnis | eingetragen von / am |
|---|---|---|
| S3-Task 0.4 (Inventar der STM-Seite, `firmware-analyst`) | Inventar liegt als `REVIEW-2026-10-17-hardware-naht.md` vor (Momentaufnahme, vom Lead nicht Zeile für Zeile nachgeprüft). **Eingearbeitet:** Rückwärtsabhängigkeiten (§3.3), Brückenwissen in sechs Logikmodulen (§3.4), Kontext je Baustein (§3.4), `delay_counter`, ISR-Aufrufe (`dcf77_tick` → `log_message`), Prioritätenfrage (§3.7), Pinkommentar `wpsbutton.c:27`. **Noch offen für Task 0.4:** vollständige Liste der Schreibstellen der Display-Zustandsmaschine; Bestätigung der Klassen M/G durch Zeile für Zeile Lesen; weitere Eingriffe in §9.3 | — |
| Stufe 0: Baseline-Reproduzierbarkeit (AK1) | — | — |
| Stufe 1: Messzahlen von C1–C4 am Baum | — | — |

---

## Anhang A — Messverfahren (damit jede Zahl wiederholbar ist)

Alle Läufe am 17.10.2026, `bash`, Apple `cc` (clang), Baum auf `pwa-decoupling`. **Hinweis:** zsh spaltet nicht gequotete Variablen nicht — die Skripte laufen mit `bash`.

- **A.1 Board-/Familienmakros.** Pro Datei `src/**/*.c|*.h` (ohne `src/stubs`), Kodierung `latin-1`: Zeilen, die `\b(BLACKPILL_BOARD|BLUEPILL_BOARD|BLACK_BOARD|NUCLEO_BOARD)\b` bzw. `\b(STM32F10X|STM32F4XX|STM32F103|STM32F103C8|STM32F411|STM32F411CE|STM32F401|STM32F401CC|STM32F401RE|STM32F411RE|STM32F446RE|STM32F407VE|STM32F407|STM32F1XX)\b` enthalten; Bedingungszeilen
  zusätzlich `^\s*#\s*(if|ifdef|ifndef|elif)\b`. Ergebnis 100/90/31 bzw. 226/169/57.
- **A.2 Include-Baum.** `cc -MM -I. -Isrc -Isrc/* -Iinc -ISPL/inc -Icmsis -DSTM32F4XX -DSTM32F411xE -DSTM32F411CE -DBLACKPILL_BOARD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=25000000 -DARM_MATH_CM4 -DWCLOCK24H=0 -DDSP_USE_LED=SK6812_RGBW_LED <datei>`; gezählt `SPL/inc/*` (26) und `inc/`, `cmsis/` (4).
- **A.3 Attrappen.** 20 leere Header (`stm32f4xx`, `_gpio`, `_rcc`, `_adc`, `_i2c`, `_tim`, `_usart`, `_dma`, `_exti`, `_iwdg`, `_syscfg`, `_flash`, `_spi`, `_pwr`, `_rtc`, `_fsmc`, `_conf`, `misc`, `system_stm32f4xx`, `core_cm4`), je `#include <stdint.h>` und `<stddef.h>`, **vor** allen `-I`-Pfaden;
  `cc -fsyntax-only -std=gnu11 -w <Pfade> <Defines wie A.2> <datei>`; Fehlerzählung `grep -c 'error'`. **`-DDSP_USE_LED=SK6812_RGBW_LED` ist nötig:** ohne den Schalter wählt `display-config.h` den Vorgabetyp WS2812 (eine erste Messung ohne ihn lieferte dieselben Zahlen, aber für den falschen Treiber; deshalb wiederholt).
- **A.4 Gegenprobe ohne Attrappen und Familienmakro.** Nur `-Isrc -Isrc/*`, `-DSTM32F411CE -DBLACKPILL_BOARD -DHSE_VALUE=25000000 -DWCLOCK24H=0 -DDSP_USE_LED=SK6812_RGBW_LED`, keine Vendor-Pfade.
- **A.5 S3-Versuch.** Wie A.4, dazu `-I<Verzeichnis mit einer Attrappe misc.h (#include <stdint.h>)>` und `-include stdint.h -include stdbool.h -include stddef.h`; BluePill-Fassung mit `-DSTM32F103C8 -DHSE_VALUE=8000000`.
- **A.6 Klassifikation nach Zeilen.** `wc -l` über die Dateilisten je Klasse; Herstellerzeilen je Datei: `grep -acE "\b(GPIO_|TIM_|RCC_|NVIC_|USART_|DMA_|ADC_|I2C_|EXTI_|SysTick|IWDG_|SCB|__disable_irq|__enable_irq|SystemInit|FLASH_)[A-Za-z0-9_]*"`.
- **A.7 Namenskollisionen (Heuristik, ●).** Funktionsnamen am Zeilenanfang beider Bäume (`^name (` im STM-Stil, `type name(` im ESP-Stil) geschnitten; **kein `nm`**.
- **A.8 `misc.h`.** `grep -n '"misc.h"'` über `src` — 8 unbedingte Zeilen in Kopfdateien, 3 geschützte.
- **A.9 Flash und Matrix.** `build/matrix/matrix-report.txt` (13.10.2026, 22:56) und `build/stm-rgbw-12h/*.hex` (Nutzdaten der Datensätze Typ 00): F103 12h SK6812-RGBW 63 996 Byte (frei 1 540), 24h 1 172 frei.
- **A.10 sdkconfig.** `grep` auf `CONFIG_*_IRAM_SAFE`, `CONFIG_GPTIMER_*`, `CONFIG_ESP_TASK_WDT_*`, `CONFIG_ESP_CONSOLE_*` in `…/esp32s3-libs/3.3.12/sdkconfig`; IDF-Stand `versions.txt`.
- **A.11 ESP-Seite.** `grep -c` auf Includes, `ESP.`, SDK-Präfixe, `lwip`/`tcp_pcb`, `Serial` in `ESP8266/ESP-uclock/*.cpp|*.ino|*.h`.
- **A.12 Dienstverbraucher.** `grep -rlaE` je Symbol in `src/**/*.c` ohne `main.c`.

**Was ich nicht gemessen habe:** den Bau eines einzigen Abbilds (R1), die Dauer von `make matrix`, die Zahl der `*.S`-Dateien je Ziel, `nm`-Ergebnisse, die tatsächliche NVIC-Priorität, jedes Verhalten am Gerät. Wo diese Spec darüber spricht, steht ○.
