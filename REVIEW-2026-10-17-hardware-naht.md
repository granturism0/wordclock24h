# Inventar der Hardware-Naht in `src/**` — Momentaufnahme 17.10.2026

**Dies ist eine Momentaufnahme mit Datum und wird nicht fortgeschrieben** (DIR-006). Zeilennummern gelten für
den Baum vom 17.10.2026 und veralten mit jeder Änderung; den lebenden Stand zeigen die Guardrails und der Code.

- **Auftrag:** Der Lead hat den `firmware-analyst` (nur lesend) beauftragt, die hardwarenahen Schnittstellen der
  STM32-Firmware zu inventarisieren. Anlass: eine Quelle für alle Boards (`specs/multiboard-quelle/`) und das
  geplante Ein-Chip-Board mit ESP32-S3 (`specs/esp32s3-port/`).
- **Herkunft:** Bericht des Analysten, **vom Lead nicht Zeile für Zeile nachgeprüft**. Die Belegzeichen sind die des
  Analysten: ✔ am Code nachgeprüft (Datei:Zeile), ○ Annahme (mit Einschätzung).
- **Pfade** sind relativ zu `src/`, wo nichts anderes steht.
- **Kodierung:** Das Werkzeug des Analysten hat `-a` nicht, las die ISO-8859-1-Dateien (`display.c`, `irmp.c`,
  `rtc.c`, `ds18xx.c`) aber trotzdem; es gibt keine Lücke durch „Binary file matches“.

## 0. Wo der Skill `stm-firmware` und `CLAUDE.md` vom Code abweichen

Wichtig, weil die Naht sonst aus veralteten Texten abgeleitet würde. **Der Skill ist an diesen Stellen nachzuführen**
(Aufgabe des Lead, `.claude/skills/stm-firmware/SKILL.md`):

- ✔ `display_test()` bedient den Watchdog inzwischen: 30 × `delay_msec(100)` mit `watchdog_reload()` bei
  `display.c:6371-6375` und `:6400-6404`. Die 45-s-Blockade ohne Reload gibt es nicht mehr.
- ✔ `display_wait_for_tables()` ist begrenzt: 15 s gesamt, 3 s ohne Fortschritt (`display.c:4310-4348`,
  `watchdog_reload()` bei `:4325`).
- ✔ `remote_ir_learn()` hat ein Zeitlimit und ruft `watchdog_reload()` auf (`remote-ir.c:149-158`).
- ✔ Die zwei unbedingten `log_printf` je Refresh in `sk6812.c` sind weg; zwei saturierende Zähler
  (`sk6812.c:139-140`, `:465-468`, `:478-483`); eine Logzeile nur noch beim Warten (`:485`).
- ✔ `esp8266_get_message()` ruft nur noch `log_flush()` (`esp8266.c:423`); `esp8266_uart_flush()` ist entfernt
  (Begründung `:425-432`).
- ✔ `debug_log_*` ist ohne `DEBUG` leer (`log/log.h:23-31`); `DEBUG` setzt nur der Diagnosebau
  (`WORDCLOCK_DEBUG`, `CMakeLists.txt:22-26`).
- ✔ Stellen mit `watchdog_reload()`: `main.c:4025` (Loopkopf), `display.c:4325`, `:5042`, `:5051`, `:6374`, `:6403`,
  `remote-ir.c:150`, `tetris.c:651`, `snake.c:339`, `vars.c:785`. Verbindlich ist Guardrail S7.

## 1. Querschnitt

**Board-Makros** (`CMakeLists.txt:301-306`, `:327-333`, `:463-504`):

| Ziel | Makros |
|---|---|
| F103 | `STM32F10X`, `STM32F10X_MD`, `STM32F103`, `STM32F103C8`, `BLUEPILL_BOARD` |
| F411 BlackPill | `STM32F4XX`, `STM32F411`, `STM32F411CE`, `BLACKPILL_BOARD` |
| F401 BlackPill | `STM32F4XX`, `STM32F401`, `STM32F401CC`, `BLACKPILL_BOARD` |
| Nucleo | `NUCLEO_BOARD` (F401RE, F411RE) |
| Black Board | `BLACK_BOARD` (F407VE, mit TFT) |
| LED-Typ | `DSP_USE_LED=${TGT_LED}` (`CMakeLists.txt:216`); Vorgabe `WS2812_GRB_LED`, beim Black Board `TFTLED_RGB_LED` (`display-config.h:30-36`) |
| Modus | `WCLOCK24H=0/1` |

✔ `DISCO_BOARD` kommt in `board-led.c:21`, `button.c:15` und `esp8266.c:40` vor, hat aber kein CMake-Ziel.

**Interrupt- und Timer-Landkarte BlackPill F411** (F103 in Klammern):

| Quelle | Peripherie | Priorität | Beleg |
|---|---|---|---|
| Zeitgeber | TIM2, 15 kHz (`F_INTERRUPTS`, `irmpconfig.h:29-30`) | 0x0F/0x0F im Code | `main.c:802-825` |
| LED-DMA | DMA1_Stream2 (F103: DMA1_Channel2), TIM3 CH4 auf PB1 (F103: TIM1 CH1 auf PA8) | 0/0 | `sk6812.c:180-205`, `:657-661` |
| Log | USART1 | 0/0 | `log-uart.c:56-58`, `uart-driver.h:544-545` |
| ESP-Brücke | USART2 | 0/0 | `esp8266-uart.c:56-58` |
| DFPlayer | USART6 (F103: USART3) | 0/0 | `dfplayer-uart.c:58-63` |
| I2C | I2C1 (PB6/PB7), gepollt, ohne IRQ | | `i2c.c:61-75` |
| ADC | ADC1 (PA5), gepollt | | `adc-config.h:37-46` |
| `delay` | SysTick | niedrigste | `delay.c:102` |
| Watchdog | IWDG | | `main.c:508-574` |

✔ SysTick bekommt die niedrigste Priorität über `SysTick_Config` (`cmsis/core_cm4.h:2025`).

✔ Zeitgeber F411: Prescaler 65, Period 100 ergibt 100 MHz / 66 / 101 = **15001,5 Hz** (`main.c:776-779`, `:795`). Der
Kommentar `main.c:735` nennt „15015“ und ist falsch gerechnet. F103: exakt 15000 Hz (`main.c:783-786`).

○ (plausibel, Messwert fehlt) **Die effektiven NVIC-Prioritäten sind unklar.** ✔ In `src/**` steht kein
`NVIC_PriorityGroupConfig` und kein `NVIC_SetPriorityGrouping`. ✔ `SPL/src/misc.c:148-157` rechnet bei PRIGROUP=0
(Resetwert) mit `tmppre = 4 - 7`: 8-Bit-Unterlauf, undefinierter Shift. ○ Es kann sein, dass alle IRQs effektiv auf
Priorität 0 landen; dann gäbe es kein Nesting zwischen TIM2, DMA und UART, und der SysTick müsste bei jedem anderen
IRQ warten. **Nicht geprüft**; per Instrumentierung klärbar (Abschnitt 7). Keine Fix-Empfehlung (DIR-003).

## 2. Je Datei

Gliederung je Eintrag: (1) Schnittstelle, (2) belegte Peripherie, (3) Aufrufkontext und Blockieren, (4) Zeitverhalten,
(5) Ersatz auf dem ESP32-S3.

### delay (`delay/delay.c`, `delay.h`)
1. ✔ `delay_init` (`:90`), `delay_usec` (`:40`), `delay_msec` (`:62`), `delay_sec` (`:77`). Öffentlich ist
   `extern volatile uint32_t delay_counter` (`delay.h:31`); andere Module schreiben ihn direkt: `i2c.c:207`, `:236`,
   `onewire.c:35`.
2. ✔ SysTick mit `SysTick_Config(SystemCoreClock / (1e6/res))` (`:99-102`). Reload F411 = 500, F103 = 360.
   `SystemCoreClock` kommt aus `main.c:57-77`, nicht aus einer Messung.
3. ✔ Nur aus `main()`-Init und Hauptloop, nie aus einer ISR. Immer blockierend, reiner Busy-Wait ohne
   `watchdog_reload` (`:51-54`, `:66-69`, `:79-83`).
4. ✔ `main.c:3782` und `onewire.c:246` setzen 5 µs. `delay_usec(n)` wartet n/5+1 Ticks, also zwischen n und n+5 µs
   (`:48`); `delay_msec` hat kein +1 (`:64`). **Ein einziger globaler Zähler, die Funktionen sind nicht reentrant.**
   ○ (plausibel) SysTick mit 200 kHz kostet geschätzt 5-8 % CPU, nicht gemessen.
5. S3: `esp_rom_delay_us()` für Mikrosekunden-Busy-Waits (1-Wire), `vTaskDelay()` für Millisekunden. Der Wechsel von
   Busy-Wait zu Yield ändert die Semantik. `delay_counter` muss weg; `i2c`/`onewire` rechnen Fristen dann über
   `esp_timer_get_time()`.

### uart (`uart/uart.h`, `uart-driver.h`) — Basis für log, esp8266, dfplayer
1. ✔ Je Instanz (Präfix): `_init`, `_putc`, `_puts`, `_char_available`, `_getc`, `_poll`, `_rsize`, `_rxmax`,
   `_rxdrops`, `_rxore`, `_rxbuflen`, `_flush` (`uart.h:29-40`). `_read`/`_write` sind deklariert (`:41-42`), im Treiber
   nicht definiert.
2. ✔ Je Instanz eine USARTx über `UART_NUMBER`/`UART_ALTERNATE` (Board-Makros in den drei `*-uart.c`). RX-Interrupt
   immer an, TXE nur beim Senden. Der IRQ-Handler greift direkt auf `SR`, `DR`, `CR1` zu (`uart-driver.h:793-863`).
3. ✔ `putc`, `getc`, `flush` im Hauptloop (und `main()`); die ISR leert und füllt die Ringe. Blockierend: `putc` bei
   vollem TX-Ring (`:561-564`), `getc` bei leerem RX (`:626-629`), `flush` (`:755-758`). Kein Timeout, kein Reload.
   ✔ `__disable_irq`/`__enable_irq` (`:573-575`) sind nicht verschachtelungsfest.
4. ✔ Ringgrössen: log TX 128 / RX 256 (`log-uart.c:67-68`), ESP TX 128 / RX 1024 (`esp8266-uart.c:68`, `:86`),
   DFPlayer 32/32 (`dfplayer-uart.c:47-48`). Baudraten: log und ESP 115200 (`main.c:3765`, `esp8266.c:1091`), DFPlayer
   9600 (`dfplayer.c:1214`). 86,8 µs je Byte; der 1024-Byte-Ring deckt 89 ms. ✔ RX-Verwurf wird gezählt (`:835-838`).
5. S3: `uart_driver_install()`. Der ESP8266-UART entfällt; das Log geht auf UART0 oder USB-Serial-JTAG. Die Zähler
   `rxmax`, `rxdrops`, `rxore` der Diagnosezeile haben dort keine Entsprechung.

### adc (`adc/adc.c`, `adc.h`, `adc-config.h`), `ldr` als Aufrufer
1. ✔ `adc_init`, `adc_start_single_conversion`, `adc_poll_conversion_value(uint16_t*)`, `gadc.is_up`
   (`adc.h:27-36`). ✔ Einziger Aufrufer ist `ldr.c` (`:40`, `:52`, `:176`, `:178`, `:224`, `:226`, `:263`).
2. ✔ ADC1, Einzelwandlung per Software-Trigger, 12 Bit rechtsbündig, ohne IRQ und DMA. BlackPill/BluePill PA5 /
   Kanal 5; Nucleo/Black PC4 / Kanal 14 (`adc-config.h:15-57`). F4: Prescaler Div4, 3 Zyklen (`adc.c:77`,
   `adc-config.h:71`). F103: ADCCLK = PCLK2/6, 1,5 Zyklen, Selbstkalibrierung mit unbegrenzten `while`-Schleifen
   (`adc.c:36`, `:81`, `:90-100`, nur im Boot).
3. ✔ Start: Hauptloop `main.c:4453-4457` (Flag aus der TIM2-ISR alle 1/4 s, `:876-879`). Abholen: `main.c:4327`.
   Nicht blockierend, ausser `ldr_set_min_value`/`ldr_set_max_value`: bis zu 10 × `delay_msec(10)` = 100 ms
   (`ldr.c:178-187`, `:226-235`), ohne Reload.
4. ✔ Wertebereich 0..4095 (`ldr.h:35`). `ldr_value` zieht pro Poll nur um 1 nach (`ldr.c:84-91`), also 4 Schritte je
   Sekunde; der Poll hängt an `animation_stop_flag` (`main.c:4324`).
5. S3: `adc_oneshot_read()` auf ADC1; Start und Poll fallen zu einem blockierenden Aufruf zusammen. Dämpfung und
   Kalibrierung festlegen, damit 4095 als Maximum weiter passt. ○ Rauschen unter WLAN-Last: mitteln.

### apa102 (`apa102/apa102.c`, `.h`, `-config.h`)
1. ✔ `apa102_init`, `_refresh`, `_set_led`, `_set_all_leds` (`apa102.h:48-51`), Typ `APA102_RGB`. Übersetzt nur bei
   `DSP_USE_APA102` (`apa102.c:28`).
2. ✔ SPI2 nur TX, SCK PB13, MOSI PB15, DMA1, TC-IRQ Prio 0 (`:34-82`, `:331-354`). SPI-Takt PCLK/4: 10,5 MHz (F4),
   9 MHz (F103).
3. ✔ Hauptloop; die ISR setzt nur `apa102_dma_status = 0` (`:240`). `apa102_refresh` blockiert mit unbegrenztem Spin
   (`:251-254`).
4. ✔ 32 Bit je LED, Endframe n/2 Bit (`:93-97`).
5. S3: `spi_master` mit DMA. ○ (plausibel) In der F4-Konfiguration steht für SPI2-TX `DMA_Channel_5` (`apa102.c:54`);
   laut Referenzhandbuch gehört SPI2_TX auf Stream4/Channel 0. Wird nie gebaut (kein APA102-Ziel), ungeprüft.

### board-led, button, wpsbutton
- ✔ `board_led_init/_on/_off` (`board-led.h:25-27`): GPIO Push-Pull; BlackPill/BluePill PC13 aktiv low, Nucleo PA5 aktiv
  high, Black PA6 aktiv low (`board-led.c:21-63`). Aufruf aus `main()` (`main.c:3785`, `:3797`) **und aus der TIM2-ISR**
  über `dcf77_tick` (`dcf77.c:188`, `:193`); nur ein GPIO-Schreibzugriff, ISR-fest. ✔ `board-led.h:15` schützt mit
  `STM32F1XX` statt `STM32F10X`.
- ✔ `button_init/_pressed` (`button.h:25-26`), `wpsbutton_init/_pressed` (`wpsbutton.h:25-26`): GPIO-Eingang mit Pull-up,
  aktiv low. BlackPill Button PA0, WPS PA7; BluePill Button PA6, WPS PA7 (`button.c:36-48`, `wpsbutton.c:27-37`).
  ✔ Kommentar `wpsbutton.c:27` nennt „PB5“, der Code nimmt PA7. Aufruf im Hauptloop per Poll (`main.c:3795`, `:4172`,
  `:4180`, nur wenn `esp8266_is_up`). Entprellung nur über `uptime`-Differenz > 5 s (`main.c:4172`, `:4180`).
- S3: GPIO mit Pull-up. „ESP8266 in den Flashmodus“ und „Access Point“ entfallen mit einem Chip; WLAN-Provisionierung
  tritt an ihre Stelle.

### dcf77 (`dcf77/dcf77.c`, `.h`)
1. ✔ `dcf77_tick`, `dcf77_time(struct tm*)`, `dcf77_init` (`dcf77.h:29-31`).
2. ✔ GPIO ohne eigenen Timer; BlackPill/BluePill DATA PB8, PON PB9 (`dcf77.c:43-65`). `dcf77_tick` hängt an TIM2 alle
   `F_INTERRUPTS/100` = 150 Ticks = 10 ms, nur bei `dcf77_enabled` (`main.c:891-899`); `dcf77_update` setzt es nur, wenn
   Display und Ambilight aus sind (`main.c:1435`).
3. ✔ `dcf77_tick` läuft **in der ISR**, `dcf77_time` im Hauptloop (`main.c:4390`). `dcf77_tick` ruft `log_message` auf
   (`:255`, `:259`, `:305`, …): UART-Ausgabe aus der ISR.
4. ✔ Zustandsmaschine rechnet in 10-ms-Tick-Einheiten (Schwellen `dcf77.c:227-251`).
5. S3: GPIO lesen per `esp_timer` mit 10 ms oder RMT RX. Die Logik ist hardwarefrei.
- ○ (plausibel, ungemessen) Zwei Risiken der ISR-Logausgabe: `uart_putc` spinnt bei vollem Ring (`uart-driver.h:561`);
  stehen alle IRQs auf Priorität 0, kann die UART-ISR die TIM2-ISR nicht verdrängen, es gäbe einen Deadlock (braucht
  128 anstehende Byte). Der Schreibindex `uart_txstop` ist unter Preemption nicht geschützt. ○ `time_is_valid`
  (`dcf77.c:71`) ist nicht `volatile`, wird in der ISR geschrieben und aus anderer Übersetzungseinheit gelesen; ohne
  LTO unkritisch, der Bau läuft mit LTO (`uart-driver.h:774-779`), ob das zählt, ist nicht geprüft.

### dfplayer (`dfplayer/dfplayer-uart.c`, `dfplayer.h`, `dfplayer.c`)
1. ✔ Treiber über `dfplayer_uart_*`; Anwendung `dfplayer.h:110-164` (`dfplayer_init`, `_read_message`,
   `_play_folder`, `_set_volume`, Queue-Funktionen, `_read_config_from_eep` …).
2. ✔ BlackPill USART6 ALT1 (PA11 TX / PA12 RX), BluePill USART3 PB10/PB11 (`dfplayer-uart.c:50-64`), 9600 Baud, Ringe
   32/32.
3. ✔ Hauptloop, bei jedem Durchlauf (`main.c:5252`). Blockierend: `dfplayer_init2` mit 4 × `delay_msec(100)` = 400 ms
   ohne Reload (`dfplayer.c:1224-1242`), erreicht aus `dfplayer_read_message` (`:491`).
4. ✔ 9600 Baud = 1,04 ms je Byte, 10-Byte-Rahmen = 10,4 ms. Die 100-ms-Pausen sind Annahmen über die Antwortzeit.
5. S3: UART1 mit 9600 Baud; Protokoll bleibt, `delay_msec` wird `vTaskDelay`.

### esp8266 (`esp8266/esp8266.c`, `esp8266-uart.c`, `.h`, `-config.h`)
1. ✔ `esp8266_get_message` (liefert die Codes `ESP8266_*`, `esp8266.h:25-54`), `_send_cmd`, `_send_data`,
   `_send_log_line`, `_reset`, `_powerdown`, `_powerup`, `_connect_to_access_point`, `_accesspoint`, `_wps`, `_flash`,
   `_init` (`esp8266.h:124-140`). Dazu Globals `esp8266` (`:86-116`), `esp8266_ten_ms_tick`, `esp8266_ack_expect`,
   `esp8266_ack_drops`, `esp8266_cmc_sync`.
2. ✔ BlackPill: USART2 PA2/PA3 ALT0, 115200 Baud; RST = PB10, CH_PD = PB3, FLASH (GPIO0) = PA4 (`esp8266.c:91-106`,
   `esp8266-uart.c:56-58`). BluePill: USART2, RST = PA0, CH_PD = PA1, FLASH = PA4 (`:108-123`).
   `esp8266_ten_ms_tick` wird in der TIM2-ISR gesetzt (`main.c:904-908`).
3. ✔ Nur Hauptloop (`esp-spiffs.c:39`: `esp8266_get_message()` darf nur dort laufen). Aufrufer:
   `schedule_esp8266_messages` (`main.c:4206`, `:3412`), die Warteschleife in `var_send_buf` (`vars.c:678`),
   `display_wait_for_tables` (`display.c:4324`), `tetris.c:657`, `snake.c:242`. Blockierend nur beim Boot:
   `esp8266_reset` 50 ms + 500-ms-Poll (`:939-964`), `esp8266_flash` mit Endlosschleife (`:1060-1071`); beide vor
   `watchdog_init` (`main.c:3798`, `:3988` gegen `:4021`).
4. ✔ Die Brücke rechnet in 10-ms-Ticks (`MSEC(x)=x/10`, `esp8266.c:27`) und `uptime`-Sekunden (3 s Quittungsfrist,
   `vars.c:48`). Die ESP-Bootzeit wird mit rund 10 s angenommen (`main.c:595-598`).
5. **S3: Das Modul entfällt komplett.** Das Zeilenprotokoll (`var …*crc`, `ACK xy`, `.`, `!v`, `CMD`, `TIME`, `TAB*`,
   `FILE`, `ICON`, `LOG`) wird zu einer prozessinternen Nachrichtenschnittstelle (Queues zwischen Uhr-Task und
   Netz-Task). RST, CH_PD, FLASH, WPS, AP-Modus und Flashen entfallen. Die Logspiegelung braucht einen Ringpuffer für das
   Web. **Das ist die grösste Einzelaufgabe.**
- ✔ Brückenwissen steckt in der Logik: `weather.c:233-282`, `timeserver.c:241-245`, `esp-spiffs.c:37-146`,
  `tables.c:88-424`, `tetris.c:657`, `snake.c:242` rufen `esp8266_send_cmd` oder `esp8266_get_message` direkt auf.

### i2c (`i2c/i2c.c`, `.h`)
1. ✔ `i2c_init(clockspeed)`, `i2c_read`/`i2c_write` mit `(slave_addr, addr, is_16_bit_addr, data, cnt)`, Rückgabe
   `I2C_OK = 0` oder `I2C_ERROR_*` (−1…−8), `i2c_nack_quiet` (`i2c.h:27-41`). ✔ Slave-Adressen sind 8-Bit-Formen mit
   Schreib-Bit (`eeprom.c:30`: 0xA0, `rtc.c:17`: 0xD0).
2. ✔ BlackPill/BluePill: I2C1, SCL PB6, SDA PB7 (`i2c.c:61-87`). Nucleo: I2C3 PA8/PC9. Black: I2C1 PB8/PB9. Gepollt, ohne
   IRQ und DMA. F4 mit internem Pull-up (`:397`). 100 kHz, bei Fehlschlag 50 kHz (`main.c:3744`, `:3909`).
3. ✔ Nur Hauptloop und `main()`-Init, nie aus einer ISR (`i2c.c:147-148`, `:176`, `:191-196`). Blockierend: jedes Flag
   bis 5 ms (`I2C_TIMEOUT_TICKS = 5 * 200`, `:26-27`, `:204-219`), Bus-frei-Warte ebenso (`:233-248`). Bei Frist: STOP,
   Software-Reset, Neuinitialisierung, ggf. 9-Takt-Freitakten mit `delay_msec(1)` (`:130-182`, `:315-359`). Kein Reload.
4. ✔ Die Fristen setzen die 5-µs-SysTick-Auflösung und den exklusiven Besitz von `delay_counter` voraus (`i2c.c:27`,
   `:196`, `:207`).
5. S3: neuer `i2c_master`-Treiber mit Timeouts und Bus-Reset. Adressen auf 7 Bit umstellen (0x50, 0x68), die 16-Bit-
   Speicheradresse des AT24C32 behalten. Die `delay_counter`-Kopplung entfällt.

### irmp (`irmp/irmp.c`, `irmp.h`, `irmpconfig.h`)
1. ✔ `irmp_init`, `irmp_get_data(IRMP_DATA*)`, `irmp_is_busy`, `irmp_ISR`, `irmp_set_callback_ptr` (`irmp.h:171-181`).
   `F_INTERRUPTS` kommt aus `irmpconfig.h:29-30` und wird von `main.c` mitbenutzt.
2. ✔ GPIO-Eingang ohne Pull: BlackPill PB4, BluePill PB3, Nucleo PC10, Black PC1 (`irmpconfig.h:127-141`,
   `irmp.c:1790-1800`). Kein eigener Timer; gelesen wird über `GPIO_ReadInputDataBit` (`irmp.h:47`, `irmp.c:2275`).
3. ✔ `irmp_ISR` in der TIM2-ISR (`main.c:870`), `irmp_get_data` im Hauptloop (`main.c:3970`, `remote-ir.c:125-131`).
4. ✔ Alle Schwellen sind aus `F_INTERRUPTS` berechnet (`irmp.c:58-230`); zulässig 10 bis 20 kHz (`irmp.h:163-164`). Die ISR
   muss in 66,7 µs fertig sein (○ ungemessen).
5. S3: GPTimer mit 15 kHz und `IRAM_ATTR`-ISR, die `irmp_ISR` ruft und per `gpio_get_level` liest; Alternative RMT RX
   mit anderem Decoder. Der Dekoder ist hardwarefrei bis auf `input()` und Init.

### log (`log/log-uart.c`, `log.c`, `log.h`)
1. ✔ `log_putc`, `log_puts`, `log_message`, `log_flush` (Makros, `log.h:18-21`), `log_printf`, `log_vprintf`,
   `log_printf_prefix`, `log_init` (`:33-37`), `debug_log_*` (leer ohne `DEBUG`, `:23-31`).
2. ✔ BlackPill/BluePill USART1 PA9/PA10, 115200 Baud, TX 128, RX 256 (`log-uart.c:56-68`). RX dient der Durchreichung im
   ESP-Flashmodus (`esp8266.c:1067`).
3. ✔ Hauptloop aus fast allen Modulen; `log_message` auch in der DCF77-ISR. ✔ `log_printf` nutzt einen statischen
   256-Byte-Puffer (`log.c:35`), nicht reentrant. ✔ Jeder Aufruf schreibt auf die Log-UART (blockiert bei vollem Ring)
   **und** spiegelt die Zeile über `esp8266_send_log_line` auf die ESP-UART (`log.c:44-45`). ✔ `log_flush` ist ein Spin
   bis TX leer (`uart-driver.h:755-758`), ausgelöst in jedem Loop-Durchlauf über `esp8266_get_message` (`esp8266.c:423`);
   128 Byte brauchen 11 ms.
5. S3: `esp_log` bzw. `printf` über UART0 oder USB-Serial-JTAG, dazu ein Ringpuffer für die Web-Anzeige; `log_flush`
   entfällt; die Zeile sollte nicht blockieren.

### main (`main.c`, `main.h`)
1. ✔ Öffentlich (`main.h:133-144`): `gmain` (Softuhr), `uptime`, `watchdog_reload`, `schedule_esp8266_messages`,
   `diag_ticks` (`main.c:702`), `game_count_dropped`, `main_set_ambilight_clock_wait_cycles`, `main_get_reset_cause`.
2. ✔ Belegt: TIM2 mit NVIC (`main.c:802-825`), `TIM_CLK`/`TIM_PERIOD` je Board (`:750-793`); IWDG und LSI
   (`:508-574`), RCC-Resetflags (`:437-493`); `fault_reset` mit SysTick und `NVIC_SystemReset` (`:616-644`); JTAG-Remap nur
   F103 (`:3751-3753`); `RCC_GetClocksFreq` (`:3875`); `#ifndef BLACK_BOARD` um die EEPROM-Ticks (`:688`, `:862`, `:910`).
3. ✔ Hauptloop `while (1)` (`:4023`), `watchdog_reload()` am Kopf (`:4025`). ISR `TIM2_IRQHandler` (`:853-1026`) macht bei
   15 kHz: IRMP, Flags (LDR alle 1/4 s, Animation 1/64 s), `dcf77_tick` und `esp8266_ten_ms_tick` je 150 Ticks,
   `eeprom_ms_tick` je 15 Ticks, Ambilight-Takt und die **Softuhr** (`uptime++`, `gmain.second/minute/…`, `:940-1025`).
   Blockierend im Loop: `eep_*`, `rtc_*`, `temp_*` (1-Wire), `led_refresh`, `var_send_buf` (bis 3 s), `delay_sec(1)` im
   Start (`:3973`).
4. ✔ Zeitbasis ist `F_INTERRUPTS` Ticks je Sekunde (`:942`); die Uhr läuft auf F411 um 0,01 % schnell (○ plausibel, wird
   per RTC und NTP nachgeführt). ✔ IWDG-Reload `20000 * 32 / 256`, auf F103 mit 40 statt 32 (`:538-540`); die LSI-Frequenz ist
   angenommen.
5. S3: Hauptloop wird ein FreeRTOS-Task. TIM2 wird ein GPTimer (nur wenn IRMP weiter abtastet), sonst `esp_timer`. IWDG wird
   zum Task-Watchdog (`esp_task_wdt_reset`). RCC-Flags werden `esp_reset_reason`. `gmain` braucht Mutex oder Atomic, weil
   kein ISR mehr schreibt. Die Flag-Mechanik (`ldr_conversion_flag` u. a.) wird zu Timern oder Task-Notifications.

### onewire (`onewire/onewire.c`, `.h`, `-config.h`)
1. ✔ `onewire_reset`, `_read_bit`, `_read_byte`, `_read`, `_write_byte`, `_write`, `_get_rom_code`, `_init`
   (`onewire.h:36-44`). ✔ `onewirte_read_rom_code` ist deklariert (`:43`) und nirgends definiert.
2. ✔ Ein GPIO als Open-Drain, gelesen über dasselbe Pin: BlackPill/BluePill PB5 (`onewire-config.h:29-38`). Kein Timer;
   die Zeitbasis ist SysTick über `delay_usec` und direktes Schreiben von `delay_counter` (`onewire.c:35`).
3. ✔ Hauptloop (`main.c:4628-4648`, `:3960`, `:3992`). **Interrupts werden nicht gesperrt.** Blockierend: Reset ca.
   1,15 ms (`onewire.c:76-85`, dazu 300 µs Abtastfenster); Lesen eines Scratchpads (Reset, MATCH ROM, 9 Byte) rechnerisch
   grob 13 bis 14 ms; `ds18xx_start_conversion(do_wait=1)` wartet bis 13333 Bit-Lesungen, ca. 0,87 s (`ds18xx.c:181`,
   `:209-214`), aufgerufen mit `do_wait = read_temperature_flag` (`main.c:4638`). Kein Reload, alles unter 20 s.
4. ✔ Die Slots stehen auf `delay_usec` mit 5-µs-Quantisierung (`onewire.c:17`, `:246`). Der Lese-Slot tastet nach
   `delay_usec(5)` plus `delay_usec(10)` ab, also 15 bis 25 µs nach der fallenden Flanke (`:108-113` gegen `delay.c:48`).
   ○ (plausibel) Die Spezifikation verlangt Abtasten innerhalb von 15 µs; das deckt sich mit dem offenen Punkt A63 in
   `BEFUNDE.md`. Dass der Sensor deshalb falsch liest, ist nicht gemessen. ○ Jeder ISR-Eintritt (TIM2, DMA-Refill, UART)
   dehnt einen Slot.
5. S3: RMT-basierte 1-Wire-Komponente oder Bit-Banging mit `esp_rom_delay_us` in einer kritischen Sektion je Slot. CRC- und
   ROM-Logik in `ds18xx.c` sind hardwarefrei.

### power (`power/power.c`, `.h`)
1. ✔ `power_init`, `power_on`, `power_off`, `power.is_on` (Start 0xFF) (`power.h:25-34`, `power.c:24-27`).
2. ✔ GPIO Push-Pull: BlackPill/BluePill PB0, Nucleo PC8 (`power.c:73-89`); beim TFT `tft_backlight_on/off`
   (`:30-69`). PB0 schaltet die 5-V-Versorgung der LED-Kette.
3. ✔ `power_init` und `power_on` in `main()` (`main.c:3776`, `:3779`, vor `delay_init`). `power_on` in `display_clock` mit
   anschliessendem `delay_msec(200)` (`display.c:2954-2955`), `power_off` bei `display.c:5675`. Nicht blockierend.
4. ✔ Die 200 ms Einschwingzeit stehen im Aufrufer, nicht im Treiber.
5. S3: GPIO oder Lastschalter; die 200 ms bleiben beim Aufrufer.

### sk6812 (`sk6812/sk6812.c`, `.h`, `-config.h`)
1. ✔ `sk6812_init` (`sk6812.c:584`), `sk6812_refresh(uint_fast16_t n_leds)` (`:459`), `sk6812_set_led(n, SK6812_RGBW*)`
   (`:541`), `sk6812_set_all_leds(rgbw, n, refresh)` (`:559`), `sk6812_get_refresh_cnt` (`:521`),
   `sk6812_get_dma_wait_cnt` (`:531`), Typ `SK6812_RGBW` (`sk6812.h:35-43`), `SK6812_MAX_LEDS = DSP_MAX_LEDS`
   (`sk6812-config.h:18`). Kompiliert nur bei `DSP_USE_SK6812_RGBW/RGB` (`:40`).
2. ✔ BlackPill (F411): TIM3 CH4 → PB1 (AF TIM3), DMA1 Stream2 Channel 5, `DMA1_Stream2_IRQHandler`, NVIC Prio 0/0
   (`:180-205`, `:657-661`). BluePill: TIM1 CH1 → PA8, DMA1 Channel2 (`:207-231`). Nucleo/Black: TIM3 CH1 → PC6, DMA1 Stream4
   (`:153-178`). DMA im Circular-Modus mit einem 2-LED-Puffer, Halbwort auf F4, Byte auf F1 (`:121`, `:142-147`);
   Half-Transfer und Transfer-Complete füllen abwechselnd die Hälften nach (`:406-452`). ✔ Beim Init wird der Datenpin
   geprüft: Eingang mit Pulldown, 1 ms warten, bei High Open-Drain, sonst Push-Pull (`:599-622`). ✔ Der Treiber liest
   `uptime` aus `main.c` und ruft `log_printf` auf (`:36`, `:291`, `:426`, `:462`).
3. ✔ Init einmal über `display_init` → `led_init` (`display.c:6737`, `main.c:3944`). Refresh aus dem Hauptloop; die ISR liest
   nur den aktuellen Puffer (`rgbw_buf[current_rgbw_buf_idx]`) und schreibt `dma_buf` und den Status. Blockierend: Spin beim
   Refresh-Start, solange der vorige Transfer läuft (`:470-496`), ohne Timeout und ohne Reload; Spin in `sk6812_clear_all`
   (`:318-321`); `delay_msec(1)` im Init (`:601`). Nach dem DMA-Start kehrt `sk6812_refresh` zurück, bis auf das Kopieren von
   `SK6812_MAX_LEDS` Einträgen (`:510-513`).
4. ✔ Bitperiode 1250 ns, Pause 250 µs (`:91-97`), also 201 Bitperioden; der Kommentar `:108` („// 67“) ist veraltet.
   F411-Timer: Period 124 bei 100 MHz. Je LED 40 µs (RGBW) bzw. 30 µs (RGB). ✔ Rechnerisch (nicht gemessen), RGBW:

   | Fall | LEDs | Dauer |
   |---|---|---|
   | 24h, Display ohne Ambilight | 289 | 11,6 ms + 0,25 ms |
   | 24h, ganze Kette | 409 | 16,4 ms + 0,25 ms |
   | 12h, Display | 114 | 4,6 ms |
   | 12h, ganze Kette | 234 | 9,4 ms |

   ✔ Der Animationstakt kommt alle 15,6 ms (`main.c:885`); zwei Refreshes können also in die Wartezeit laufen. ✔ Die DMA-ISR
   läuft je LED, also alle 40 µs.
5. S3: RMT TX mit DMA (`rmt_new_tx_channel(with_dma=true)`) und eigener Bytes-Encoder. Bitformen T0H 300 ns / T1H 750 ns,
   z. B. bei 10 MHz Auflösung. **Byte-Reihenfolge G, R, B, W** (`:354-373`). Reset-Lücke ≥ 80 µs, aktuell 250 µs,
   **beibehalten wegen SK6812C/SKC6812**. Die ISR-Nachfüllung entfällt. Die Semantik „Refresh wartet auf den vorigen“ bleibt
   mit `rmt_tx_wait_all_done`, aber mit Timeout. `uptime` und `log_printf` müssen aus dem Treiber heraus. Der 5-V-Pegelwandler
   (V2: SN74AHCT1G125) bleibt Hardware.

### ws2812 (`ws2812/ws2812.c`, `.h`, `-config.h`)
1. ✔ `ws2812_init`, `_refresh`, `_set_led`, `_set_all_leds` (`ws2812.h:46-49`), Typ `WS2812_RGB`. Vorgabe-LED
   (`display-config.h:30-36`).
2. ✔ Wie sk6812, mit identischer Peripherie je Board (`ws2812.c:147-198`).
3. ✔ Wie sk6812; Refresh wartet unbegrenzt (`:425-428`), ohne Logzeile und Zähler.
4. ✔ Bitperiode 1270 ns, T0H 470 / T1H 800 ns, Pause 300 µs (`:65-72`).
5. S3: RMT wie sk6812, mit anderen Zeiten.

### BLACK_BOARD-Gruppe: ssd1963, w25qxx, xpt2046 (mit `touch`, `tft`, `tftled`, `flash`)
✔ Alle durch `BLACK_BOARD` bzw. `DSP_USE_TFTLED_RGB` geschützt (`ssd1963.c:12`, `w25qxx.c:19`, `xpt2046.c:12`, `flash.c:18`,
`eeprom.c:13`); sie gehen in die F103- und F411-Bauten nicht ein. FSMC-Pins an PD/PE (`ssd1963.c:589-657`), W25Q auf SPI1 mit
DMA2, XPT2046 auf SPI2 mit RXNE-IRQ und EXTI5 (`xpt2046.c:36-61`, `:265-292`). Blockierend: `xpt2046_get_data` spinnt
unbegrenzt (`:128`), die W25Q-Statusabfragen ebenso (`w25qxx.c:235`, `:266`, `:303`, `:346`). ✔ `flash.c` hält 4 Sektoren im
RAM und schreibt 5 s nach der letzten Änderung (`flash.c:30-31`); `eep_flush` ist an dieses Backend gebunden (`eep.h:30`): ein
Vorbild für ein flash-gestütztes EEPROM-Backend auf ESP32. S3: entfällt oder wird durch einen Simulator ersetzt.

### eeprom, ldr, rtc, ds18xx, tempsensor als Aufrufer der Hardware-Module
- **eeprom** (`eeprom/eeprom.c`): ✔ `eeprom_init` (`:89`), `_read` (`:182`), `_write` (`:248`), `_get_address`,
  `eeprom_is_up`, `eeprom_ms_tick` (`eeprom.h:21-27`). Nutzt `i2c_init/_read/_write` (`:94`, `:102`, `:188`, `:275`,
  `:290`) und `i2c_nack_quiet`. ✔ Der Wartezyklus ist ein Spin auf `eeprom_ms_tick` aus der TIM2-ISR, 16 Ticks, ohne Reload
  und ohne Timeout (`:63-81`). ✔ Seitengrösse 32 (F4) bzw. 8 (F103) (`:44-50`). ✔ **Quer-Kopplungen:** ruft
  `esp8266_uart_rxdrops()` (`:254`, `:312`), `diag_ticks()` aus `main.c` (`:255`, `:312`) und `log_printf` (`:311`).
  Nach aussen: `eep.h` (`eep_init/_read/_write/_is_up/_flush`, `:21-43`).
- **ldr** (`ldr/ldr.c`): ✔ nutzt `adc_*`, `delay_msec` (`:180`, `:228`), `eep_read/_write`, `log_printf` (`:200`, `:243`).
  Die Messlogik selbst ist hardwarefrei.
- **rtc** (`rtc/rtc.c`): ✔ nutzt nur `i2c_*` (`:80`, `:105`, `:131`) und `eep_*`. Die BCD- und Registerlogik ist hardwarefrei.
- **ds18xx** (`ds18xx/ds18xx.c`): ✔ nutzt nur `onewire_*`. CRC8 und Familiencodes sind hardwarefrei.
- **tempsensor** (`tempsensor/tempsensor.c`): ✔ nutzt nur `ds18xx_*` und `eep_*`.

## 3. Aufrufer → hardwarenahe Schnittstelle

| Aufrufer | Funktion (Datei:Zeile des Aufrufs) | Angerufenes Modul |
|---|---|---|
| **main.c** | `log_init` (3765), `log_flush`, `log_printf` | log |
| | `irmp_init` (3773), `irmp_get_data` (3970), `irmp_ISR` in ISR (870) | irmp |
| | `power_init/_on` (3776/3779) | power |
| | `delay_init(5µs)` (3782), `delay_sec(1)` (3973) | delay |
| | `board_led_init` (3785), `board_led_on` (3797) | board-led |
| | `button_init` (3788), `button_pressed` (3795, 4172) | button |
| | `wpsbutton_init` (3806), `wpsbutton_pressed` (4180) | wpsbutton |
| | `esp8266_flash` (3798), `esp8266_init` (3988), `esp8266_accesspoint` (4178), `esp8266_wps` (4186), `esp8266_get_message` (3412), `esp8266_uart_rx*` (4146-4149), `esp8266_ten_ms_tick` (906) | esp8266 |
| | `rtc_init` (3885), `rtc_get_date_time` (3937, 4000, 4430), `rtc_set_date_time` (2459, 3448, 4398, 5246), `rtc_get_temperature_index` (3997, 4687) | rtc |
| | `eep_init` (3886), `eep_flush` (3951, 4190), `eep_read/_write` (1039-1392) | eep → eeprom/flash |
| | `ldr_init` (3957), `ldr_start_conversion` (4455), `ldr_poll_brightness` (4327), `ldr_set_min/max_value` (1642, 1648) | ldr |
| | `dcf77_init` (3958), `dcf77_tick` in ISR (898), `dcf77_time` (4390) | dcf77 |
| | `temp_init` (3960, 4628), `temp_start_conversion` (4638), `temp_read_temp_index` (3992, 4648) | tempsensor → ds18xx → onewire |
| | `dfplayer_init` (3961), `dfplayer_read_message` (5252), `dfplayer_play_folder` (2097, 4794, 5275, 5327) | dfplayer |
| | `touch_init` (3954), `touch_check` (4193), `tftled_init` (3792), `ssd1963_*` | Black Board |
| | `watchdog_init` (4021), `watchdog_reload` (4025), `fault_reset` (616) | IWDG, SysTick, NVIC |
| | `timer2_init` (3803) | TIM2 |
| **display.c** | `led_refresh` über `display_refresh_*` (484, 495, 506, 518); `led_set_led` (540, 562, 569, 779-794, 890-894, 924, 962, 982, 6359, 6388); `led_init` (6737) | LED-Treiber |
| | `power_on` (2954), `power_off` (5675) | power |
| | `delay_msec` (2955, 5050, 6373, 6402) | delay |
| | `watchdog_reload` (4325, 5042, 5051, 6374, 6403); `schedule_esp8266_messages` (4324); `uptime` (4317-4344) | main.c |
| | `eep_read` (3313-3461), `eep_write` (2918, 3659-4101) | eep |
| **vars.c** | `esp8266_uart_puts`, `_flush` (596-614) | esp8266-uart |
| | `schedule_esp8266_messages` (678), `watchdog_reload` (785), `uptime` (644, 697, 783) | main.c |
| | liest `ldr.*` (1406-1418), `grtc.*` (1328, 1453-1462), `ds18xx.is_up` (1447), `gtemp.*` (1468-1474), `dfplayer.*` (1702-1744), `ssd1963.flags` (1751), `eep_is_up` (1316) | Zustandsstrukturen der Hardware-Module |
| **eeprom.c** | `i2c_init/_read/_write` (94, 102, 188, 275, 290); `eeprom_ms_tick` (69, TIM2-ISR); `diag_ticks` (255, 312); `esp8266_uart_rxdrops` (254, 312) | i2c, main.c, esp8266-uart |
| **ldr.c** | `adc_init` (263), `adc_start_single_conversion` (40, 176, 224), `adc_poll_conversion_value` (52, 178, 226); `delay_msec` (180, 228); `eep_read/_write` (114-115, 155-156) | adc, delay, eep |
| **rtc.c** | `i2c_init` (131), `i2c_read` (80), `i2c_write` (105); `eep_read/_write` (266, 301) | i2c, eep |

## 4. Die LED-Treiber-Makros in `display/display.h`

✔ Sechs Funktionsmakros plus ein Typ. `LED_RGB` steht bei `display.h:77`, `:85`, `:93`, `:103`. Zuordnung im SK6812-Zweig
(`display.h:91-99`):

| Makro | Ziel | Kontext | Rückgabe / Semantik |
|---|---|---|---|
| `led_init` | `sk6812_init` | einmal, `display_init` (`display.c:6737`) | void. Stellt Pin, TIM3, DMA, NVIC ein, löscht alle LEDs (`sk6812.c:584-669`), wartet 1 ms |
| `led_refresh(n)` | `sk6812_refresh` | Hauptloop (`display.c:484-518`) | void. `n` = Zahl der LEDs ab Kettenanfang. Blockiert beim Start bis zum Ende des vorigen Transfers |
| `led_set_led(idx, rgb)` | `sk6812_set_led` | Hauptloop | void. Schreibt nur in den „nächsten“ Puffer, sendet nichts (`sk6812.c:541-552`). Index über `SK6812_MAX_LEDS` wird verworfen |
| `led_set_all_leds(rgb, n, refresh)` | `sk6812_set_all_leds` | Hauptloop | void. Nur `sk6812.c` selbst ruft es (`clear_all`); ✔ in `display.c` kein Aufruf; beim WS2812-Zweig auskommentiert (`display.h:89`) |
| `led_get_refresh_cnt` | `sk6812_get_refresh_cnt` | Diagnosezeile (`main.c:4144`) | `uint32_t`, saturierend; für andere Treiber Fallback 0 (`display.h:113-116`) |
| `led_get_dma_wait_cnt` | `sk6812_get_dma_wait_cnt` | wie oben (`main.c:4145`) | `uint16_t`, saturierend, Fallback 0 |

**Was `sk6812_refresh` blockiert und warum** ✔ (`sk6812.c:459-514`):
1. Zählt den Aufruf (`:465-468`).
2. Spin auf `sk6812_dma_status != 0` (`:470-496`). Warum: Der DMA-Puffer und die ISR-Variablen (`current_dma_buf_pos`,
   `current_led_offset`, `current_data_pause_len`, `current_leds`) gehören bis zur letzten Pause dem laufenden Transfer; ein
   früherer Start würde sie überschreiben. Der Wartepfad hat weder Timeout noch `watchdog_reload()`.
3. Puffer wechseln, ISR-Variablen setzen, beide Hälften vorfüllen, `sk6812_dma_start` (`:498-508`).
4. Zuletzt `rgbw_buf[current]` nach `rgbw_buf[next]` kopieren, damit `set_led` auf dem Stand des letzten Frames weiterarbeitet
   (`:510-513`). Die Funktion kehrt zurück, die Übertragung läuft in der ISR weiter.

○ (plausibel, aus dem Code folgt es) **Die `w=`-Zählung kann einen Stillstand nicht von normalem Warten trennen:**
`waited = uptime - wait_start` zählt ganze `uptime`-Sekunden (`:474`). Ein regulärer Wartefall von 12 bis 17 ms, der eine
Sekundengrenze streift, erzeugt „waiting 1s“ und erhöht den Zähler (`:478-483`). Ein echter Stillstand zeigt `waited >= 2` oder
mehrere Zeilen nacheinander; eine einzelne `waited=1`-Zeile belegt keinen.

## 5. Steckt in der hardwarefreien Logik doch Hardwarewissen?

**Registerzugriffe:** ✔ keine in `display.c` (nur Kommentare, `:5007-5018`, `:6366`, `:6395`; auch kein `volatile`). ✔ In `alarm`,
`overlay`, `tables`, `weather`, `night`, `base`, `font`, `esp-spiffs`, `remote-ir`, `timeserver`, `tetris` keine Register,
`GPIO_`, `NVIC` oder `__disable_irq`; Treffer nur `delay_*`, `uptime`, `watchdog_reload`.

**Annahmen über Interrupts und Zeitbasis:**
- ✔ `gmain.*` (`volatile`, `main.h:124-130`) und `uptime` (`volatile`, `main.h:134`) werden in der TIM2-ISR geschrieben und in
  `display.c`, `vars.c`, `timeserver.c` gelesen (`display.c:1215`, `:3097`, `:5545`, `:4317-4344`, `vars.c:644`, `:697`).
  Mehrfeld-Lesezugriffe (Stunde, Minute, Sekunde) sind nicht atomar. ○ Kurz vor einem Minutenwechsel sind inkonsistente
  Lesungen denkbar.
- ✔ `uptime` hat Sekundenauflösung; auf ihr hängen Timeouts in der Logik (`vars.c:48`, `display.c:4310-4312`, `tetris.c:635`,
  `:742`). Eine „3-s-Frist“ ist real zwischen 2 und 3 s.
- ✔ Die 5-µs-Annahme steckt in den Hardware-Modulen (`i2c.c:27`, `onewire.c:17`, `ssd1963.c:2086`, `w25qxx.c:126`,
  `xpt2046.c:147`), nicht in der Logik.
- ✔ Blockierende Wartezeiten in der Logik: `display.c:2955`, `:5050`, `:6373`, `:6402`, `remote-ir.c:149`, `tetris.c:681`,
  `:700`, `:761`, `snake.c:266`, `:372`, `ldr.c:180`, `:228`, `main.c:3973`. Auf FreeRTOS sollten alle `vTaskDelay` werden.
- ✔ **Rückwärtsabhängigkeit:** Die Logik ruft in `main.c` zurück: `schedule_esp8266_messages` (`display.c:4324`, `vars.c:678`) und
  `watchdog_reload` (`display.c`, `vars.c:785`, `remote-ir.c`, `tetris.c`, `snake.c`).
- ✔ **Hardware-Module hängen an Anwendungswissen:** `sk6812.c` (`uptime`, `log_printf`), `eeprom.c`
  (`esp8266_uart_rxdrops`, `diag_ticks`, `log_printf`), `dcf77.c` (`board_led`, `log_message` in der ISR). Jeder Austausch eines
  Moduls wird damit ein Eingriff in die anderen.
- ✔ `delay_counter` ist ein öffentlich beschreibbarer Globalzustand (`delay.h:31`) und wird von `i2c.c` und `onewire.c` direkt
  benutzt.
- ○ Die Flag-Mechanik ISR → Hauptloop (`main.c:674-686`, z. B. `ds3231_flag = 0` bei `:4450`) arbeitet ohne kritische Sektion; ein
  zwischen Abfrage und Löschen gesetztes Flag geht verloren. Folgen gering (Minutentakt).

## 6. Ausdrücklich nicht gefunden

- Eine Ursache der sporadischen F411-Hänger ergibt sich aus diesem Inventar **nicht**. DIR-003 bleibt unberührt: kein DMA-Fix
  und kein Recovery.
- Offen: ob die NVIC-Prioritäten tatsächlich alle auf 0 stehen (Abschnitt 1); ob der 1-Wire-Lese-Slot zu spät abtastet (A63).
- Die ISR-Last und alle Refresh-Dauern sind aus Konstanten gerechnet, nicht gemessen.

## 7. Vorschlag für Instrumentierung (Beweisführung, keine Umsetzung)

Alle Zeilen nur einmal beim Start oder einmal je Minute, nie in der ISR.

1. **NVIC-Prioritäten:** `main.c`, nach den `NVIC_Init`-Aufrufen (nach `:3803`), vor `watchdog_init`. Ausgeben:
   `SCB->AIRCR >> 8 & 7` und `NVIC->IP[...]` für TIM2, DMA1_Stream2, USART1, USART2. H1: alle auf 0. Bestätigt bei gleichen
   Werten, widerlegt bei TIM2 = 0xF0 und den anderen 0x00.
2. **Echte Refresh- und Wartezeit:** `sk6812.c`, in `sk6812_refresh` und der DMA-ISR (`:426-427`). Tick-Differenz zwischen
   `sk6812_dma_start` und TC sowie Wartedauer in Ticks (66,7 µs), Maximum je Minute. H2: 24h-Kette ≈ 16,6 ms ≈ 250 Ticks.
3. **Echter Stillstand gegen Sekundengrenze:** `sk6812.c:474-483`, `waited` plus Tick-Differenz. H3: `w` enthält Fehlalarme.
4. **1-Wire-Abtastzeit:** `onewire.c`, `onewire_read_bit` (`:104-117`), SysTick-Zähler bei der Abtastung. H4: später als
   15 µs, bestätigt bei Werten über 3 Ticks.
5. **ISR-Einfluss auf SysTick:** `delay.c:27`, Zähler aller `SysTick_Handler`-Aufrufe gegen die `diag_tick_cnt`-Rate. H5: SysTick
   verliert Takte, wenn andere ISRs laufen.
