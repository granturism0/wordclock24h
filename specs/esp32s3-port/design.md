# Design — Portierung auf einen einzelnen ESP32-S3 (Variante B)

**Stand:** 2026-10-17. Zeichen **✔** = im Repo belegt, **✔K** = im Komponentenblatt der Datenblatt-Analyse belegt (`hardware/komponenten/esp32s3-wroom-1.md`,
`sy8301abc.md`; ich habe die Blätter gelesen, nicht die PDFs), **◌** = Herstellerwissen oder Annahme, nicht belegt, mit dem Task, der es prüft
(Legende und Warnung zu `HARDWARE.md` in `requirements.md`). Wo Fachwissen fehlt, steht eine **Empfehlung** und der **Task, der sie belegt** — nichts davon ist hier als Befund gesetzt.

## 1. Lösungsweg im Überblick

```
                    ESP32-S3 (ein Chip, zwei Kerne)
 ┌───────────────────────────────────────────────────────────────────────┐
 │ Kern 1                                    Kern 0                      │
 │ ┌───────────────────────────────┐         ┌──────────────────────────┐│
 │ │ display_task  (bisher STM32)  │         │ net_task   (bisher ESP)  ││
 │ │ main-Schleife, display.c,     │ Strom-  │ Arduino setup/loop:      ││
 │ │ vars.c, overlay, irmp-Auswer- │ puffer  │ WLAN, HTTP, UDP, NTP,    ││
 │ │ tung, Sensoren, EEPROM        │◄───────►│ Wetter, Update, LittleFS ││
 │ │ EINZIGER Schreiber der        │ je      │ spricht nur über die     ││
 │ │ Display-Zustandsmaschine      │ Richtung│ Brücke mit der Anzeige   ││
 │ └──────▲────────────────────────┘         └──────────────────────────┘│
 │        │ ISR 20 kHz (GPTimer, IRAM): irmp, Softuhr, Flags              │
 └────────┼──────────────────────────────────────────────────────────────┘
          │  Hardwareschicht, gleiche Funktionsnamen wie heute:
          ▼  sk6812_* (RMT), i2c, adc, onewire, delay, power, GPIO
   LED-Board über H7 (unverändert) · DS3231 + AT24C32 · DS18B20 · IR · DCF77 · Taster
```

**Vier Grundsätze, aus den Entscheidungen E1–E12 abgeleitet:**

1. **Die Logik bleibt, die Nahtstellen wandern.** `display.c`, `vars.c`, `overlay.c`, `weather.c`, `night.c`
   und die übrigen Module ohne Hardware-Zugriff werden **nicht angefasst** (E11, AK35; Einzelheiten §3.6). Gezählt (Grep vom 17.10.2026, Muster
   `GPIO_|TIM_|RCC_|NVIC_|IWDG_|USART_|DMA_|ADC_|I2C_|EXTI_|SysTick_|SYSCFG_|__disable_irq|__enable_irq`):
   **627 Treffer in 15 von 25 geprüften Dateien**; in `display.c`, `vars.c`, `ds18xx.c`, `ldr.c`, `log.c`,
   `flash.c` und `esp-spiffs.c` sind es **0**. Wo ein Treffer steht, ist eine Nahtstelle. (Grobes Muster:
   in `irmp.c` und `main.c` stehen Treffer, die nur einen Pin lesen oder einen Zeitgeber setzen.)
2. **Die Naht der LED-Ausgabe ist ihre Treiber-API.** `display.c` kennt den LED-Treiber nur über sechs Makros, die auf
   `sk6812_*` abgebildet sind (✔ `display.h:94-99`). Der S3 bekommt einen eigenen Treiber mit **derselben** Schnittstelle; `display.c`
   merkt den Unterschied nicht (§3.6). **Das ist der erste Nachweis (Stufe 2) — und er berührt keine Zeile `display.c`.**
3. **Ein gemeinsamer Quelltext statt eines Forks** (Annahme F11: die alten Ziele bleiben). Eine Kopie von
   `main.c` mit 5'378 Zeilen driftet binnen Wochen von der Vorlage weg — dieselbe Mechanik wie bei Versionsnummern
   in der Doku (DIR-006). Das kostet **Task 5.1**: Die Plattformanteile in `main.c` (Zeitgeber, Watchdog,
   Takt, Interrupt-Rumpf) wandern in eine eigene Datei, **Verhalten unverändert**, am STM nachgewiesen. **`display.c` gehört nicht dazu.**
4. **Die Brücke bleibt eine Brücke.** Beide Seiten sprechen weiter Zeilen miteinander. Das ist teurer als ein
   direkter Funktionsaufruf, aber es ist die einzige Zusage, die Protokoll, PWA und Legacy-Oberfläche unberührt
   lässt — und die einzige, die der Nutzer verlangt hat (E5).

### 1.1 Framework, Build-System, Verzeichnis, Besitz — **entscheidet `specs/multiboard-quelle/`**

**Diese Spec legt das Framework nicht fest.** Die Architektur (Naht, Verzeichnisse, Board-Beschreibung, Bau-Matrix, **Framework mit Preis**) ist Aufgabe von `sw-architect`
in `specs/multiboard-quelle/` (✔ `.claude/agents/sw-architect.md`); die Aufgaben des S3-Ports sind diese Spec. Was ich beitrage, ist **Eingabe** für jene Entscheidung —
Beobachtungen, kein Beschluss:

| Beobachtung | Quelle | Folge für die Entscheidung |
|---|---|---|
| Dieser Entwurf braucht Schalter in der Systemkonfiguration: IRAM-sichere Zeitgeber- und RMT-Interrupts, 20-s-Task-Watchdog, Konsole primär UART0 **und** sekundär USB-Serial/JTAG, eigene Partitionstabelle | ◌ aus dem Entwurf; Task 1.1 zeigt, ob das gewählte Framework sie erlaubt | Ein Framework, das diese Schalter nicht zulässt, **scheidet aus**, wenn der Preis nicht tragbar ist. Das Arduino-Paket liefert ◌ vorkompilierte IDF-Bibliotheken mit fester Systemkonfiguration; ob das hier ein Hindernis ist, **prüft `sw-architect`** |
| Die Konsolenkonfiguration hängt am Framework | ✔K `esp32s3-wroom-1.md` §9, §14 | `sw-architect` nennt sie in seiner Spec; AK3a bleibt unverändert |
| `src/` liegt ausserhalb jedes Sketch-Ordners | ✔ Verzeichnisaufbau | Die Dateiauswahl (welche `src/**`-Dateien in den S3-Build gehen) ist **seine** Entscheidung; §3.6 liefert die **belegte Grundlage** (welche Module hardwarefrei sind) |
| Beide Quellbäume tragen gleichnamige Dateien `display`, `vars`, `tables`, `base` | ✔ `src/display/display.c`, `ESP8266/ESP-uclock/display.cpp` usw. | Kopfdateinamen und globale Symbole **kollidieren** in einem Abbild. Die Spec verlangt: getrennte Include-Pfade und eine **Symbolprüfung** am gelinkten Abbild (AK23, AK34); das Werkzeug baut der Lead (Task 0.3) |
| Pins stehen an **einer** Stelle | ✔ Vorgabe in `sw-architect.md` | **Eine** Quelle für Pins je Board (Board-Beschreibung); `pins`-Kopfdatei und Pinprüfung **lesen sie**, sie pflegen keine zweite Liste (Task 0.3) |
| Bauen darf nur der Lead (R1); `tools/hooks/no-build.py` weist heute `make`, `cmake`, `arduino-cli` ab, **`idf.py` fehlt dort** | ✔ CLAUDE.md | Je nach Framework muss der Hook ergänzt werden (Task 0.3) |
| Besitz des neuen Verzeichnisses | ✔ `tools/hooks/file-ownership.py`, Wurzelmuster Zeile 98 kennt nur `src/`, `ESP8266/ESP-uclock/`, `tools/`, `.claude/`, `knowledge/`, `specs/`, `cmake/`, `Makefile`, `CMakeLists.txt`, `*.md` | **Ohne Eintrag lässt der Hook Schreibzugriffe auf das neue Verzeichnis ungeprüft durch** (und er ist ohnehin nur eine Erinnerung, B22/L171). `sw-architect` schlägt die Besitzer vor, **die Rolle legt allein der Lead an** (Task 0.3). Arbeitsname für den Software-Baum in dieser Spec: **`esp32s3/`**; er ist **nicht** `hardware/esp32s3/` (KiCad, Besitzer `hw-designer`) |

**Annahme für die Aufgabenplanung** (`tasks.md`): Der S3 baut als **eigenes Ziel** neben den alten; die alten Ziele bleiben unverändert baubar (F11, AK33). **Widerspricht `multiboard-quelle` dieser Annahme, geht der Widerspruch
an den Lead** — keine der beiden Specs überschreibt die andere.

## 2. Pinbelegung

Quelle: Vorschlag des Nutzers. **Spalte „abgeglichen" bleibt leer, bis die Tasks 0.1a bis 0.1c den Schaltplan des Nachbaus gelesen haben.**
Die Tabelle ist AK1; eine Zeile ist erst „abgeglichen", wenn Netzname **und** Header-Pin eingetragen sind. **Die Pinzuordnung entscheidet `hw-architect`**
(Ablage `hardware/architektur/pinbelegung.md`, liegt noch nicht vor), die Prüfung leistet `hw-reviewer`; diese Spec trägt das Ergebnis ein (Task 0.2). **Die Pins werden im Code
an einer einzigen Stelle geführt, nicht hier und nicht in einer zweiten Hand-Liste** (§1.1).

| Signal | GPIO | DevKit-Header (✔K §3.2) | Richtung | Zustand beim Einschalten (✔K §4.2) | Besonderheit | abgeglichen (Netz / Header-Pin) |
|---|---|---|---|---|---|---|
| LDR (Abgriff von `H7` Pin 5) | 1 | J3 | Analog-Eingang | 60 µs Low, danach hochohmig | ✔K ADC1 (GPIO1–10), bleibt bei WLAN nutzbar (ADC2 nicht). `R22` und 0,1 µF am Pin (H9). Netz `LDR` trägt auf V2 zusätzlich `H5.2` (✔ `v2.xml`). Das TRM kennt einen dritten Boot-Modus mit **GPIO1 = 1, GPIO2 = 0** bei GPIO0 = 0 und GPIO46 = 1 (✔K §13.1) — GPIO46 ist im Auslieferungszustand Low, der Fall tritt nicht ein | |
| DS18B20 | 2 | J3 | Ein-/Ausgang, offener Kollektor | 60 µs Low, danach hochohmig | Pull-up extern (F7); ein Pull-up hält GPIO2 hoch, der dritte Boot-Modus verlangt 0 (siehe oben) | |
| LED-Daten | 5 | J1 | Ausgang (RMT) | 60 µs Low, danach hochohmig | danach U3; `R31` hält die Leitung bei stromlosem U3 (`HARDWARE.md`) | |
| LED-Versorgung | 6 | J1 | Ausgang | **60 µs Low, danach hochohmig** | **F2:** V2 hat `R20` (100 kΩ) am Gate von Q2 (✔ `v2.xml`); N-Kanal-Q2 → Low-Glitch harmlos (✔K); GPIO21 wäre laut Komponentenblatt der sauberere Kandidat (Entscheid `hw-architect`) | |
| IR | 7 | J1 | Eingang | 60 µs Low, danach hochohmig | 3,3-V-Pegel zwingend (F3); auf V2 kein Stecker, nur Pull-up `R8`/`R9` (✔ Skill) | |
| SDA | 8 | J1 | Ein-/Ausgang, offener Kollektor | 60 µs Low, danach hochohmig | externer Pull-up (F7) | |
| SCL | 9 | J1 | Ein-/Ausgang, offener Kollektor | 60 µs Low, danach hochohmig | externer Pull-up (F7) | |
| DCF77 | 15 | J1 | Eingang | 60 µs Low, danach hochohmig | ✔K GPIO15/16 sind ab Werk Anschlüsse eines 32-kHz-Quarzes; ohne Quarz frei | |
| Taster USER | 16 | J1 | Eingang | 60 µs Low, danach hochohmig | Pull-up (F7) | |
| Taster WPS | 17 | J1 | Eingang | 60 µs Low, danach hochohmig | Pull-up (F7); ✔K Treiberstärke 10 mA | |
| Status-LED | 18 | J1 | Ausgang | 60 µs Low **und** 60 µs High | ✔K Treiberstärke 10 mA; ein High-Glitch beim Einschalten ist für eine LED ohne Folgen | |

**Nicht benutzt, mit Grund (E2):** GPIO0, 3, 45, 46 (Strapping; ✔K §4.1); 19, 20 (USB); 43, 44 (UART0); 35–37 (PSRAM/Flash der R8-Modelle — und das **heute verkaufte DevKit ist N8R8**, ✔K §3.3); 38, 48 (RGB-LED des DevKits;
✔K: bei v1.1 GPIO38, bei v1.0 GPIO48). Zusätzlich ✔K nicht herausgeführt: GPIO22–32. **GPIO19 und GPIO20 dürfen vom Programm nie umkonfiguriert werden** — wer sie als GPIO nutzt, schaltet USB-Serial/JTAG ab und
verliert das **automatische** Flashen, die Konsole und JTAG in einem Zug; dann hilft nur der manuelle Weg (§4.4, AK3 Rückfall 2).

**Prüfung (Lead, Task 0.3):** Ein Skript liest die **eine** Pinquelle, zählt die Signale (**11**) und weist jede Nummer der Ausschlussliste ab. Es versagt einmal absichtlich (AK34). Es ersetzt **nicht** das Gutachten von
`hw-reviewer` über den Schaltplan (AK1).

## 3. Software-Architektur

### 3.1 Schichten

| Schicht | Inhalt | Besitzer | Ändert sich? |
|---|---|---|---|
| Anwendung (STM-Seite) | `display.c`, `vars.c`, `overlay.c`, `weather.c`, `night.c`, `tables.c`, `alarm.c` u. a. | `stm-developer` | **nein** (AK35) |
| Anwendung (Netz-Seite) | `http.cpp`, `vars.cpp`, `weather.cpp`, `ESP-uclock.ino` u. a. | `esp-developer` | Nahtstellen (§5) |
| Nahtstelle STM-Seite | Zeitgeber-Rumpf, Watchdog, Takt, `main()`-Plattformteil | `stm-developer` | **Task 5.1**, Verhalten gleich |
| Hardwareschicht | S3-Treiber mit **denselben Kopfdateien** wie heute (`delay.h`, `i2c.h`, `adc.h`, `onewire.h`, `sk6812.h` …, **unverändert aus `src/`**) | S3-Rolle (Vorschlag `s3-developer`, Task 0.3) | neu |
| Plattform | `app_main`, Tasks, Brücke, Systemkonfiguration, Partitionstabelle | S3-Rolle | neu |

Die Hardwareschicht **behält die Funktionsnamen und Rückgabewerte** der heutigen Treiber und **bindet die originalen Kopfdateien ein**.
Dann bleiben `eeprom.c`, `rtc.c`, `ldr.c`, `ds18xx.c`, `remote-ir.c` und `display.c` **unverändert** im Build. Welche Schnittstelle
das je Kopfdatei genau ist, ist **nicht erhoben** — Task 0.4 listet sie.

### 3.2 Zwei Tasks, ein Schreiber

| | `display_task` | `net_task` |
|---|---|---|
| Kern | **1** | **0** (◌ Arduino-`loopTask` läuft ab Werk auf Kern 1, WLAN auf Kern 0; der Entwurf legt `net_task` daher selbst an, statt sich auf den Voreinstellungswert zu verlassen — Task 1.1 prüft die Zuordnung) |
| Inhalt | bisherige `main()`-Schleife von `src/main.c` | bisherige `setup()`/`loop()` von `ESP-uclock.ino` |
| Priorität | höher als `net_task` | normal |
| Stapel | benannte Konstante, Startwert im Gerüst gemessen (`uxTaskGetStackHighWaterMark`) und mit Reserve eingetragen — **keine Zahl aus dieser Spec** | wie links |
| Watchdog | `watchdog_reload()` ruft `esp_task_wdt_reset()`; registriert, 20 s | Kopf von `loop()` ruft `esp_task_wdt_reset()`; registriert, 20 s (**neue Aufrufstelle**, siehe §3.4) |
| Darf schreiben auf | **alle** Display-Zustände | **nichts** davon; nur Brücke, WLAN, LittleFS, ESP-eigene Variablen |

**Display-Zustände, die nur `display_task` schreibt** (Liste wird in Task 0.4 vollständig erhoben; belegt sind schon
`display`-Struktur, `display_clock_flag`, `pending_ticker_restore` (✔ `main.c:1553`, Schreibstellen `:3400`, `:4256`,
`:4918`), `show_icon_stop_time`, `show_overlay_idx`, Ticker-Zustand hinter `display_ticker_active()`). Der Netz-Task
**berührt diese Namen nicht** — er schickt Kommandozeilen, die der `display_task` ausführt, **genau wie heute**.
Ein HTTP-Handler, der „der Einfachheit halber" `display.…` liest oder schreibt, ist die Verletzung, die AK23
auffangen soll.

**Wie AK23 prüft — ohne `display.c` anzufassen (E11).** Der frühere Entwurf setzte ein Makro an jede Schreibstelle. Das hätte
**Zeilen in `display.c`** verlangt und ist **verworfen** (§9). Stattdessen zwei unabhängige Wege, die beide **an den Grenzen** liegen, nicht in der Anwendung:

(a) *Statisch, am gelinkten Abbild:* `nm` über beide Komponenten. Die Netz-Komponente hat **keine** unaufgelöste Referenz auf ein Symbol
der STM-Komponente (und umgekehrt) — Funktionen **und** globale Variablen. Weil beide Seiten nur über die Brücke verbunden sind, **kann** der Netz-Task
keine Display-Variable schreiben, ohne dass der Linker sie als Querverweis zeigt. Die Prüfung nennt die **Zahl der betrachteten Symbole** (Task 0.3).
(b) *Dynamisch, im Debug-Build, an den Übergängen:* Jede Funktion der **Hardwareschicht** (I²C, ADC, LED-Treiber, 1-Wire, Brücke, Konsole)
ruft `xTaskGetCurrentTaskHandle()` und vergleicht mit dem erwarteten Task (`display_task` für die STM-Seite, `net_task` für `StmLink`).
Das Makro liegt im S3-Baum, ist im Release-Build leer und wird **einmal absichtlich verletzt** (ein Aufruf des LED-Treibers aus
`net_task`); es muss abbrechen. Es sieht **Schreibzugriffe auf die Hardware**, nicht Schreibzugriffe auf Variablen — Letztere schliesst (a) aus.

**`pending_ticker_restore` (E5).** Die Bedingung steht heute bei `main.c:4910-4914` (✔): `pending_ticker_restore &&
! display_ticker_active () && ! display.do_display_icon && show_icon_stop_time == 0 && show_overlay_idx >= MAX_OVERLAYS`.
**Task 5.1 darf diese Zeilen nicht verändern**, auch nicht umformatieren (AK24 prüft den Wortlaut). Dass der
Display-Zustand jetzt in **einem** Task liegt, macht die vier Teilbedingungen nicht überflüssig — sie beschreiben
Zustände des Ablaufs, nicht der Nebenläufigkeit.

### 3.3 Die Brücke als Strompuffer

**Entscheid:** ein **FreeRTOS-Stream-Buffer je Richtung** (Bytestrom), nicht ein Message-Buffer.
Begründung: Beide Gegenstellen parsen heute **Zeilen aus einem Bytestrom** und führen eigene Zeilenpuffer
(✔ `esp8266_get_message()` auf STM-Seite, `Serial.read()`-Schleife bei `ESP-uclock.ino:1618`). Ein Bytestrom lässt beide
Parser **unverändert**; ein Message-Buffer würde das Zeilenende zur Rahmenmarke machen und halbe Zeilen anders
behandeln als der UART (**Abweichung vom Protokoll**).

| Eigenschaft | heute (UART) | am S3 | Warum |
|---|---|---|---|
| Schnittstelle STM-Seite | `esp8266_uart_putc/getc/flush`-Familie (✔ 25 Treffer des Suchmusters `uart_(init\|putc\|getc\|poll\|flush)\|esp8266_uart_\w+\s*\(` in `esp8266.c`) | **dieselben Namen**, ersetzt in der Hardwareschicht (`esp8266-uart.c`) | `esp8266.c` (1'097 Zeilen) bleibt unverändert |
| Schnittstelle Netz-Seite | `Serial.println` usw. (✔ 495 Treffer für `Serial.`/`debugmsg`/`statusmsg` in 13 Dateien) | Klasse **`StmLink`** mit der `Print`/`Stream`-Schnittstelle über den Stream-Buffer | siehe unten |
| Geschwindigkeit | 115'200 Baud ≈ 11,5 kB/s | RAM-Kopie, **ein bis zwei Grössenordnungen schneller** (◌ geschätzt) | **ändert das Lastbild** — AK22 |
| Ringgrösse | ESP 256 Byte, STM 1'024 Byte (✔) | je Richtung benannte Konstante, **Startwert 1'024 Byte** (Spec-Setzung) | nicht kleiner als heute |
| voll | STM: verwirft, zählt in `uart_rxdrops`/`d=` (✔); Senderring 128 Byte blockiert `uart_putc` (✔ `specs/bruecke/`) | Erzeuger wartet **höchstens `BRIDGE_TX_BLOCK_MS`** (Startwert 10 ms, Spec-Setzung), dann wird die **ganze Zeile** verworfen und **gezählt**; die nächste durchkommende Zeile trägt die Marke aus `specs/bruecke/` AK15 (`!n`) | **Kein stilles Verwerfen** (Checkliste 4), kein unbegrenztes Blockieren (Checkliste 2) |

**Die Konsequenz der höheren Geschwindigkeit, ausdrücklich:** Der Vollabgleich (rund 194 `var`-Kommandos zu je
etwa 30 Byte ≈ 6 kB, ◌ Grössenordnung) lief am UART **gedrosselt von der Leitung** (✔ `specs/bruecke/` Problem P3 B). Am S3 gibt es
diese Drosselung nicht. Ein Pfad, der ohne Quittungsprüfung sendet, läuft hier **früher in den Rückstau** als am UART.
Ob die Quittungsprüfung je Kommando im Ist-Stand des Codes vollständig ist, **klärt Task 0.4 am Quelltext** (die
Spec `specs/bruecke/` ist eine Momentaufnahme und nennt ihren Umsetzungsstand nicht). **Gilt sie nicht, ist das ein
Befund, kein Anlass, die Puffer zu vergrössern.** AK22 provoziert den Rückstau absichtlich.

**`StmLink` statt Umbenennen in 495 Zeilen.** Die Netz-Seite schreibt heute ihre Debugtexte **und** ihre Brückenzeilen
auf dasselbe `Serial` (`HARDWARE.md`, „Warum die ESP-Debugzeilen auf der STM-UART landen"). Am S3 trennen sich beide:
Brückenzeilen gehen auf `StmLink`, Debugtexte auf die **Konsole** (§4.4). **Welche der 495 Stellen welche sind, ist
nicht erhoben** — Task 0.5 sortiert sie (Brücke, Debug, Status) und liefert die Liste an `esp-developer`. Die
Zeilen, die der STM **als Antwort** erwartet (`.`, `OK …`, `ERROR …`, `!v`, `CLOSE`, ✔ `ESP-uclock.ino:1700-2407`),
gehören auf die Brücke; ein Debugtext, der **versehentlich** auf der Brücke landet, wäre eine Fehlzeile für den
STM-Parser.

**Wegfall der UART-Last.** Die Debugzeilen der HTTP-Anfragen (✔ rund 100 Byte je Request, Kernbefund 4 aus
`REVIEW.md`) belasten die Brücke am S3 **nicht mehr**, weil sie die Konsole erreichen. Das ist eine
**Verhaltensänderung**, kein Fehler. Der Logspiegel in den Ring des ESP (`stm32_log`, ✔ `ESP-uclock.ino:779`)
läuft weiter über die Brücke und bleibt **ohne blockierenden Flush** (A19).

**Beziehungen, die der Port nicht aufheben darf:** `var_send_busy` wird nur auf die **Punkt-Quittung** hin
zurückgesetzt (✔ `specs/bruecke/` AK1); `got_ack` setzt den Watchdog nur innerhalb des Budgets nach (✔ AK3). Beides ist
Anwendungslogik in `src/**` und bleibt **unberührt**.

### 3.4 Watchdog

| | heute | am S3 |
|---|---|---|
| Mechanismus | IWDG, ✔ 20 s (`WATCHDOG_TIMEOUT_MS`), `watchdog_reload()` in `main.c` | ◌ Task-Watchdog (TWDT), 20 s, **je Task** registriert |
| Aufrufstellen | **7** (✔ Guardrail S7, `WD_EXPECTED`; Stand beim Umsetzen am Quelltext lesen — die Zahl ist die der Spec `specs/bruecke/`, Task 0.4(i) zählt nach) | die **gleichen** (Aufruf bleibt `watchdog_reload()`, Rumpf ändert sich) **plus eine neue** im Kopf von `net_task` |
| Wirkung | Reset des STM; der ESP läuft weiter | Reset des **gesamten Chips**, also auch des WLAN. **Das ist neu und gewollt:** Ein hängender Netz-Task hat sonst niemanden, der ihn zurückholt |
| Start-Meldung | `IWDGRST` in `main.c:457-459` (✔) | `esp_reset_reason()` wird beim Start in **dasselbe Wort** übersetzt; **ein Brownout-Reset erhält ein eigenes Wort** (AK25, AK37); `watch-log.sh` und `measure-log.sh` erkennen beide (Task 5.5 prüft die Muster gegen den Quelltext der Werkzeuge) |
| Fallstrick | Einen Watchdog zu bedienen, ohne einen Ausweg zu schaffen, tauscht Reset gegen unbegrenzte Blockade (✔ `specs/bruecke/`, P1/P7) | gilt unverändert; **jeder neue Warteweg im Port ist begrenzt** (Brücke: `BRIDGE_TX_BLOCK_MS`, I²C: 5 ms, RMT: Timeout) |

Der **Interrupt-Watchdog** und der **Cache-Fehler** (◌ Zugriff auf Flash-Rodata bei ausgeschaltetem Cache) sind
neue Absturzursachen, die der STM32 so nicht kennt; sie führen zum Neustart mit Meldung. Siehe §3.5 und Checkliste 4.

### 3.5 Zeitgeber, Softuhr und Interrupt

Heute: **TIM2** mit `F_INTERRUPTS = 15'000` Hz (✔ `main.c:725-731`, 84 MHz / 700 / 8). Der Rumpf
(`main.c:853-975` ✔) ruft `irmp_ISR()`, zählt alle Zeittakte und setzt Flags (`ldr_conversion_flag`, `animation_flag`,
`esp8266_ten_ms_tick`, `eeprom_ms_tick`, `seconds_flag`, `uptime++`, `gmain.second++`, `ds3231_flag`,
`measure_temperature_flag` …) und ruft `dcf77_tick()` alle 10 ms. **Die Softuhr hängt an diesem Interrupt**, nicht an der RTC.

| Punkt | Entwurf | Belegt durch |
|---|---|---|
| Zeitgeber | **GPTimer**, Alarm mit Auto-Reload, Interrupt auf **Kern 1** (Zeitgeber wird aus `display_task` heraus angelegt) | Task 4.1 |
| `F_INTERRUPTS` | **20'000 Hz** (Empfehlung, F14). ◌ 20'000 Hz teilen sich aus 80 MHz exakt (4'000 Schritte), 15'000 Hz nicht (5'333,3). Der Rest wirkt je nach Zählerauflösung verschieden: bei 1 MHz wären es 67 statt 66,67 Schritte (0,5 %, rund 7 Minuten je Tag, solange nur die Softuhr zählt), bei hoher Auflösung deutlich weniger. Die RTC wird laut `main.c:950-956` einmal je Minute angefragt (was damit geschieht, ist nicht gelesen). Weil die IRMP-Zeitfenster an der Abtastfrequenz hängen, ist ein **exakter** Teiler vorzuziehen | AK16, AK19 |
| Wirkung auf IRMP | Die IRMP-Konstanten leiten sich zur Übersetzungszeit aus `F_INTERRUPTS` ab (✔ `irmp.c:58`, `:90-137`); die Rechnung bleibt, nur der Wert ändert sich — **eine Änderung an der Stelle, an der `F_INTERRUPTS` steht (Task 0.4(f) nennt sie), also `stm-developer` (Task 5.1)**. Bis dahin setzt das Testprogramm in Stufe 4 den Wert **von aussen** (Compiler-Option), ohne Quelltextänderung | AK19 |
| Flash-Zugriffe | ◌ Während Flash geschrieben wird (LittleFS, NVS, OTA), ist der Cache aus; ein Interrupt, der Code oder Rodata **aus dem Flash** liest, stürzt ab. Deshalb: Interrupt-Funktionen und ihre Tabellen **im RAM**. Für `irmp.c` (5'114 Zeilen, viele Tabellen) **ohne Quelltextänderung** über eine Linker-Fragmentdatei, die `.text` und `.rodata` des Objekts in IRAM bzw. DRAM legt (Form hängt vom Framework ab, F12) | AK16 (100 Schreibvorgänge) |
| Rumpf | `tick_isr_body()` aus Task 5.1, in IRAM. Ruft `dcf77_tick()` (✔ in `src/dcf77/dcf77.c`, 537 Zeilen): **Entscheid in Task 0.4**, ob der Aufruf im Interrupt bleibt oder an den Display-Task wandert (Flag + 10-ms-Takt) | Task 0.4 |
| Flags | Der Interrupt **setzt**, `display_task` **löscht** — ein Verbraucher, wie heute. `volatile` bleibt. Der Interrupt läuft auf **demselben Kern** wie der Verbraucher; das schliesst Kern-übergreifende Wettläufe aus | Task 4.1, AK23 |

### 3.6 Wiederverwendung — was unverändert bleibt, was die Hardwareschicht ersetzt (E11, AK35)

**Die Naht ist die Treiber-API.** `display.h` bildet den LED-Treiber über Makros ab (✔ `display.h:78-115`): je nach Konfiguration auf `apa102_*`, `ws2812_*`,
`sk6812_*` oder `tftled_*`; bei den Treibern ohne Zähler ersetzen Makros die Zählerfunktionen durch Konstanten `0` (✔ `display.h:114-115`). Für `sk6812_*`
(✔ `display.h:94-99`) und die Prototypen in `sk6812.h:49-57` gilt:

| Makro in `display.h` | Funktion in `sk6812.h` | Prototyp (✔ `sk6812.h`) |
|---|---|---|
| `led_init` | `sk6812_init` | `void sk6812_init (void)` |
| `led_refresh` | `sk6812_refresh` | `void sk6812_refresh (uint_fast16_t)` |
| `led_set_led` | `sk6812_set_led` | `void sk6812_set_led (uint_fast16_t, SK6812_RGBW *)` |
| `led_set_all_leds` | `sk6812_set_all_leds` | `void sk6812_set_all_leds (SK6812_RGBW *, uint_fast16_t, uint_fast8_t)` |
| `led_get_refresh_cnt` | `sk6812_get_refresh_cnt` | `uint32_t sk6812_get_refresh_cnt (void)` |
| `led_get_dma_wait_cnt` | `sk6812_get_dma_wait_cnt` | `uint16_t sk6812_get_dma_wait_cnt (void)` |

Aufrufe laut Grep: **19** in `display.c`, **2** in `main.c`. `SK6812_RGBW` hat die Felder `red`, `green`, `blue` und, wenn `DSP_USE_SK6812_RGBW == 1`, `white` (✔ `sk6812.h:37-43`);
die Konstante kommt aus der Konfiguration (`wclock24h-config.h`/`display-config.h`), die der S3-Build **unverändert** einbindet. Auf der Leitung geht zuerst **Grün**
(✔ `sk6812.c:354-356`); die Reihenfolge der übrigen Bytes ist Grün, Rot, Blau, Weiss (laut Auftrag — **Task 2.1 belegt sie am Quelltext, bevor er sie nachbaut**).

**Was der S3-Treiber ersetzt und was er übernimmt.** `sk6812.c` hat 671 Zeilen (✔); davon sind **rund 120 bis 128** hardwarenah (Treffer des StdPeriph-Musters 120,
Zählung des Auftraggebers 128 Zeilen): TIM3, DMA1 Stream 2, der Unterbrechungsrumpf, `sk6812_dma_init`/`_start` und `sk6812_setup_dma_buf`, das die **Vergleichswerte
des Zeitgebers** füllt. Das alles entfällt. **Übernommen wird die Logik**, die klein ist: die Doppelpufferung (✔ `rgbw_buf[2][SK6812_MAX_LEDS]`, `sk6812.c:126`),
`set_led` und `set_all_leds`, die Zähler und die Zeitwerte (Periode 1250 ns, T0H 300, T1H 750, T0L 950, T1L 500, Pause 250 µs, ✔ `sk6812.c:91-97`). **Die Zeitwerte bleiben**; nur
ihr Träger wechselt (Zeitgeberwerte → RMT-Symbole). **Der Nachbau geschieht in der Hardwareschicht, nicht durch `#if` in `sk6812.c`** — das hielte die Stufe-2-Zusage
(keine Zeile `src/**`) nicht, und `sk6812.c` trägt Konfiguration für sechs Boards. Eine spätere Zusammenführung wäre eine eigene Aufgabe (nicht in dieser Spec; Sache von `multiboard-quelle`).
**Die beiden Zähler sind echte Zähler:** Bilder gesendet; Fälle, in denen ein Bild auf das vorige warten musste.

**Module, die unverändert übernommen werden** (alle `.c` ohne Treffer des Einbindungsmusters `#include "stm32…"`, `"misc…"`, `"core_cm…"` — Grep vom 17.10.2026 über `display`, `overlay`,
`vars`, `tables`, `font`, `alarm`, `night`, `weather`, `timeserver`, `rtc`, `eeprom`, `ldr`, `ds18xx`, `tempsensor`):

| Gruppe | Dateien | Beleg |
|---|---|---|
| Anzeige und Anwendung | `display.c` (**6'739 Zeilen**), `overlay.c`, `vars.c`, `tables.c`, `font.c`, `alarm.c`, `night.c`, `weather.c`, `timeserver.c` | **0** Einbindungen von STM-Kopfdateien in den `.c`-Dateien dieser Verzeichnisse (Grep) |
| Sensor- und Speicherlogik | `rtc.c`, `eeprom.c`, `ldr.c`, `ds18xx.c`, `tempsensor.c` | dito; **die Kopfdateien** binden STM-Kopfdateien ein (siehe Berichtigung) |
| Typen und Konfiguration | `main.h`, `base.h` | laut Auftrag ziehen sie nur `time.h`, `stdint.h`, `string.h` und `eeprom-data.h` — **Task 0.4 bestätigt** |

**Berichtigung zum Auftrag, am Quelltext.** Der Auftrag nannte für `rtc`, `eeprom`, `ldr`, `ds18xx` und `tempsensor` „keine STM-Header" (0 Treffer). **Für die `.c`-Dateien
stimmt das; die Kopfdateien tragen Einbindungen:** `rtc.h:16-20`, `ds18xx.h:17-19`, `tempsensor.h:17-19`, `eeprom.h:16-18` und `ldr.h:16-24` (dort zusätzlich `_gpio`, `_rcc`, `_adc`) binden
`stm32f10x.h` beziehungsweise `stm32f4xx.h` ein — **jeweils unter `#if defined (STM32F10X)` und `#elif defined (STM32F4XX)`** (✔ `rtc.h:16-20`, die übrigen mit demselben Muster
gelesen). **Der S3-Build definiert keines von beiden**, dann fällt jede dieser Einbindungen weg und die Module übersetzen **ohne** STM-Kopfdateien. **Ob sie dann wirklich ohne
Änderung übersetzen** (ein Typ, den die Kopfdatei sonst aus dem STM-Header holte), ist **nicht belegt** und wird in Task 1.1 durch eine **Übersetzungsprobe** gezeigt. Fällt sie durch,
gilt diese Reihenfolge: (1) eine **Ersatz-Kopfdatei** im S3-Baum mit genau den fehlenden Typen — **kein Eingriff in `src/`**; (2) nur wenn das nicht geht, eine
Ausnahme nach dem Verfahren unten.

**Hardwarenahe Dateien, ersetzt durch eine schmale Hardwareschicht mit den bisherigen Funktionsnamen:**

| Datei(en) heute | Ersatz in der Hardwareschicht | Aufwand (§4.1) | Task |
|---|---|---|---|
| `sk6812.c` | LED-Treiber auf RMT, **sechs Funktionen**, Zeiten wie oben | M | 2.1 |
| `i2c.c` | I²C-Master, gleiche Funktionen (`i2c_init`, `i2c_read`, `i2c_write`, `i2c_reset_bus`) | M | 3.1 |
| `adc.c` | ADC1 oneshot, `adc_init`, `adc_start_single_conversion`, `adc_poll_conversion_value` | M | 4.2 |
| `onewire.c` | 1-Wire | M | 4.3 |
| `delay.c` | `delay_init`, `delay_usec`, `delay_msec`, `delay_sec` | S | 4.1 |
| `log-uart.c` (und die Ausgabeseite von `log.c`) | Konsole, gleiche Aufrufe | S | 1.1 |
| `power.c` | `power_on`, `power_off` | S | 4.5 |
| `watchdog_reload` (in `main.c`) | `esp_task_wdt_reset` | S | 4.1 / 5.1 |
| `irmp.c` (nur Pin-Zugriff und ISR-Aufruf) / `remote-ir.c` | Pin-Zugriff, Platzierung im RAM | M | 4.4 |
| `button.c`, `wpsbutton.c`, `dcf77.c`-Pin, `board-led.c` | GPIO | S | 4.5 |
| `esp8266-uart.c`, `uart-driver.h` | Stream-Buffer (§3.3) | M | 5.2 |

**Die Zahl der hardwarenahen `.c`-Dateien.** Der Auftrag zählt **20**; meine Zählung mit dem StdPeriph-Muster trifft **15 von 25 geprüften Dateien** (§1). Die Differenz hat
zwei mögliche Gründe — verschiedene Muster und eine andere Dateimenge —, **belegt ist keiner.** **Verbindlich ist die Liste aus Task 0.4(a) und (k):** je Datei „hardwarefrei" oder
„hardwarenah", mit Beleg. **AK35 zählt über diese Liste.**

**Ausnahmeverfahren: keine Änderung an `display.c` ohne gesonderten Grund und Beleg.** Wer glaubt, `display.c` oder ein anderes hardwarefreies Modul ändern zu müssen:
(1) nennt den **Grund** (Compilermeldung, Messung, Zeile), (2) prüft zuerst **Ersatz-Kopfdatei, Compiler-Option oder Hardwareschicht** als Weg ohne Änderung,
(3) trägt die Ausnahme in diese Tabelle ein, (4) wartet auf die **Freigabe des Nutzers**, bevor er schreibt.

| # | Datei | Grund | Beleg | Freigabe |
|---|---|---|---|---|
| — | *keine Ausnahme beantragt* | | | |

## 4. Hardwareschicht je Baustein

**Aufwandsmasse** (keine Stunden — die wären erfunden): **S** = unter rund 150 neue Zeilen und **ein** Messnachweis.
**M** = 150 bis 600 Zeilen oder **ein Messverfahren mit Gerät**. **L** = über 600 Zeilen, **mehrere** Messverfahren oder
**ein Architekturentscheid**. Die Zeilenzahl der **ersetzten** Datei (✔ Grep) ist angegeben, weil sie das Mass der
Neuschreibung nicht ist: Die meisten alten Dateien tragen mehr Konfiguration für **andere Boards** als Treibercode.
**Alle Aufwandsangaben sind Schätzungen aus Dateigrösse und Schnittstelle, nicht gemessen.**

### 4.1 Tabelle

| Baustein | heute (STM32) ✔ | am S3 | Aufwand | Besonderheit / Risiko | Task |
|---|---|---|---|---|---|
| **sk6812** (LED-Ausgabe) | `sk6812.c` 671 Z., 120 Treffer; TIM3 CC4 + DMA1 Stream 2; Periode 1250 ns, T0H 300, T1H 750, T0L 950, T1L 500, Pause 250 µs | **RMT TX** (◌ 4 Sendekanäle, 4 Empfangskanäle am S3) mit Encoder für Bit 0/1 und Reset-Symbol; **DMA-Betrieb**, damit ein 114-LED-Bild (3'648 Bit, ≈ 4,6 ms) nicht vom Speicher des WLAN-Stacks abhängt; **API wie `sk6812.h`** (§3.6) | **M** | Auflösung 10 MHz (100 ns) reicht für ±150 ns; die Pause ≥ 250 µs ist **eine** Pause, nicht 250 µs Daten. ◌ Der DMA-Betrieb ist am S3 an **einen** Kanal gebunden; die Wahl fällt in Task 2.1. Ambilight bis 234 LEDs (≈ 9,4 ms): grösserer Sendepuffer, Speicherbedarf **messen**. ✔K Errata RMT-176 steht in der Zusammenfassung (Einzelbeschreibung **nicht gelesen**) — Task 2.1 liest sie, bevor der DMA-Betrieb gewählt wird. `SKC6812` verlangt > 200 µs Pause (`HARDWARE.md`): **der Zweig `#if 0` in `sk6812.c` bleibt zu** — er ist im Nachbau nicht vorhanden, die Pause ist 250 µs | 2.1 |
| **ws2812, apa102, tft, ssd1963, touch, xpt2046, tetris-Anzeige, dfplayer, w25qxx** | `src/ws2812`, `apa102`, … | **nicht gebaut** (diese Hardware gibt es auf dem Print nicht) | — | Die Dateien bleiben im Repo für die alten Ziele; der S3-Build führt sie **nicht** auf | 1.1 |
| **i2c** | `i2c.c` 540 Z., **142** Treffer, 5 ms Frist je Flag, `i2c_reset_bus()` mit neun Takten, Bus-Freitakten bei BUSY (✔ `i2c.c:163-180`) | **I²C-Master** des IDF; Pins GPIO8/9; Takt wie heute (Wert aus `i2c_clockspeed`, Task 0.4 liest ihn) | **M** | Die Fehlersemantik muss **gleich** bleiben: NACK ≠ Störung (A73, L402); Frist je Transaktion statt je Flag, **Gesamtfrist ≤ 5 ms je Flag-Äquivalent** (AK15); Bus-Freitakten bei Störung. Der IDF-Treiber hat eigene Timeouts — Task 3.1 stellt sicher, dass **keine** davon länger wartet, als der alte Treiber es durfte | 3.1 |
| **eeprom** (AT24C32) | `eeprom.c` 317 Z., **4** Treffer; seitenweise, 32 Byte, `EEPROM_WAITSTATES 15`, `eeprom_ms_tick` | **unverändert**, über `i2c.h` und den 1-kHz-Takt aus §3.5 | **S** | `eeprom_ms_tick` kommt aus dem Interrupt (`F_INTERRUPTS / 1000` = 20 Interrupts bei 20 kHz). **Datenformat unverändert, `git diff` leer** (AK13) | 3.2 |
| **rtc** (DS3231) | `rtc.c` 397 Z., **2** Treffer | **unverändert** über `i2c.h` | **S** | RTC-Zeit wird bei `hh:mm:45` gelesen (✔ `main.c:950-956`) — gilt unverändert | 3.2 |
| **adc** (LDR) | `adc.c` 140 Z., 38 Treffer; 12 Bit, **Start/Abfrage getrennt** (`adc_start_single_conversion`, `adc_poll_conversion_value`, ✔ `adc.c:114-140`), alle 1/4 s (✔ `main.c:876`) | **ADC1 oneshot** (✔K nur ADC1, GPIO1), Dämpfung **als benannte Konstante** (6 dB oder 12 dB, F15), **Kalibrierung ab Werk** (✔K ● IDF: Curve-Fitting aus eFuse-Werten), Umrechnung in mV und **zurück in Zählwerte 12 Bit / 3,3 V** (siehe §4.2); **Mehrfachabtastung** | **M** | Die Rückgabe bleibt `uint16_t` 0–4095 gegen 3,3 V (✔ `LDR_MAX_RAW_VALUE 4095`); `ldr.c` bleibt. Start/Abfrage bleibt **zweistufig**: Die Messung läuft im Aufruf von `…_start_…` oder im Hintergrund, `…_poll_…` liefert das Ergebnis genau einmal. ✔K: Der ADC speist den Zufallszahlengenerator; Lesen vermindert dessen Zufälligkeit (● IDF) — **die Uhr nutzt keinen Zufall für Sicherheit**, Task 0.5 prüft es für die Netz-Seite (TLS/Schlüssel) | 4.2 |
| **ldr** | `ldr.c` 264 Z.; ✔ klemmt eine **Kopie** des Rohwerts (`ldr.c:61-77`), 32 Helligkeitsstufen, ändert den Stufenwert je Messung um **höchstens 1** (`ldr.c:84-91`) | **unverändert** | **S** | Der unveränderte Glättungsschritt in `ldr.c` (±1 Stufe je 250 ms) dämpft Rauschen — die Helligkeit flackert nicht, solange der Rohwert nicht über mehrere Stufen springt (AK17c) | 4.2 |
| **onewire** | `onewire.c` 251 Z., 6 Treffer, Zeitbasis `delay_usec` (SysTick) | **Bit-Banging** mit `esp_rom_delay_us` **oder** RMT-1-Wire (◌ IDF-Komponente `onewire_bus`) — **beide müssen den Abnahmetest bestehen**; die Wahl trifft Task 4.3 nach einer Logikanalysator-Messung | **M** | 1-Wire-Slots sind 1–15 µs genau (◌). Eine Interrupt-Sperre über einen ganzen Slot (≤ 70 µs) **stört** den 20-kHz-Interrupt (50 µs Takt) — IR könnte Takte verlieren, und das **zählt** die Messung (`diag_tick_cnt`). **Die offenen Befunde A63/A64/A65 bleiben offen:** der Port übernimmt `ds18xx.c` unverändert und **darf sie nicht verdecken** (zum Beispiel, indem er den Abtastzeitpunkt zufällig besser trifft, ohne das zu melden) | 4.3 |
| **ds18xx** | `ds18xx.c` 404 Z., 0 Treffer, CRC-8, 255 bei Fehler, drei Fehler = „verloren" (A2, ✔ `CLAUDE.md`) | **unverändert** | **S** | AK18 prüft die Semantik am Gerät | 4.3 |
| **irmp** | `irmp.c` 5'114 Z., 67 Treffer; Aufruf `irmp_ISR()` mit 15 kHz | Algorithmus **unverändert**; Pin-Zugriff über die Hardwareschicht; `irmp.o` per Linker-Fragment in IRAM/DRAM | **M** | Die Zeitbasis ändert sich (§3.5) → alle Längen in `irmp.c` leiten sich neu ab. Erkennt die Uhr bei 20 kHz **dieselben** Protokolle, die sie bei 15 kHz erkannte? **AK19 entscheidet**, nicht die Rechnung | 4.4 |
| **remote-ir** | `remote-ir.c`, 0 Treffer | **unverändert** | **S** | `learn_ir` ist bekanntermassen blockierend (✔ CLAUDE.md, R5: „unbegrenzte Blockade ⇒ Watchdog-Reset"). Der Port **übernimmt** das Verhalten, er korrigiert es nicht (E8) | 4.4 |
| **delay** | `delay.c` 104 Z., 4 Treffer; SysTick, **Busy-Wait** auf `delay_counter` (✔ `delay.c:51-69`) | `delay_usec` → `esp_rom_delay_us`; `delay_msec` → `vTaskDelay` ab einer Tick-Länge, darunter Busy-Wait | **S** | ✔ Das heutige `delay_msec` **hält den Hauptloop an**; `vTaskDelay` gibt die CPU ab. Das ist **weicher**, ändert aber nichts an der Reihenfolge der Schritte im `display_task`. **Wer `delay_msec(200)` nach `power_on()` ersetzt, hält die 200 ms Einschwingzeit der LED-Versorgung ein** (`HARDWARE.md`, `display.c:2937`). `delay_sec` bleibt | 4.1 |
| **watchdog** | in `main.c` (`watchdog_init`/`_reload`, ✔ `:399-400`, `:512-546`) | §3.4 | **S** | Die LSI-Eigenheit des IWDG (✔ `main.c:512-516`: Flags warten auf den Takt) entfällt | 4.1 |
| **dcf77** | `dcf77.c` 537 Z., 29 Treffer; Pin PB8, Takt 10 ms aus dem Interrupt | Pin GPIO15; Dekodierung **unverändert** | **S** | Siehe F3 (Pegel). Tick im Interrupt oder im Task: §3.5 | 4.5 |
| **button, wpsbutton** | 21 und 14 Treffer | GPIO16/17, interne Pull-ups **nur falls F7 es zulässt** | **S** | Entprellung: nicht gelesen — Task 0.4(j) stellt fest, ob sie in `button.c` (Software) oder im Pull-up (Hardware) liegt | 4.5 |
| **board-led, power** | 26 und 12 Treffer | GPIO18 / GPIO6 | **S** | `power_off()` ist der **Anfangszustand** und **die allererste Anweisung** der Anwendung (§4.5, F2) | 4.5 |
| **uart-Brücke** | `esp8266.c` 1'097 Z., 59 Treffer; `esp8266-uart.c`, `uart-driver.h` | §3.3 | **M** | Semantik des vollen Puffers; keine blockierende `flush`-Wirkung (A19) | 5.2 |
| **log-uart** (STM-Debug) | `log-uart.c`; PA9/PA10 | **Konsole** (§4.4): UART0 + USB-Serial/JTAG | **S** | Das STM-Log landet auf der Konsole **und** im Ring (✔ `log_vprintf` spiegelt auf die Brücke) | 1.1 |
| **Start-up, Takt, Linker, Speicherkarte** | `system_stm32f4xx.c`, `startup_stm32f407xx.S`, Linker-Skripte | **entfällt** (das Framework übernimmt) | — | — | 1.1 |

### 4.2 Der LDR-Eingang — Muss (a), mit Rechnung

**Was der Nutzer festgelegt hat (E9, E10):** LDR und LED-Board bleiben unverändert; der Abgriff läuft über `H7` Pin 5 auf
`GPIO1`; `R22` auf dem Controller wechselt von 1 kΩ auf ca. 47 kΩ; `ldr.c` und die Normierung bleiben; die Kalibrierung wird
neu gemessen. **Was das Komponentenblatt festhält (✔K `esp32s3-wroom-1.md` §6):** Die Paarung `R22`/Dämpfung ist **noch nicht entschieden** (F15); zwei Paarungen sind brauchbar.

**Der Teiler** (`HARDWARE.md`: GL5528 vom 3,3-V-Anschluss zum Abgriff, `R22` vom Abgriff nach Masse; ✔ V2-Netzliste: `R22` = 1 kΩ, Pin 2 am Netz `LDR`, Pin 1 am Massenetz):
`U = 3,3 V × R22 / (R_LDR + R22)`. Die LDR-Widerstände sind die **groben Werte** aus `HARDWARE.md` (dunkel 1 MΩ,
Zimmerlicht 20 kΩ, hell 10 kΩ) — **eine Annahme dort, kein Messwert**.

**Die Dämpfungsstufen des S3-ADC (✔K DS-C Tab. 5-6, kalibrierter Gesamtfehler):**

| Dämpfung | nutzbarer Bereich | Gesamtfehler | gleichwertiger Zählwert-Bereich (gerechnet, `mV/3300 × 4095`) |
|---|---|---|---|
| 0 dB | 0 … 850 mV | ±5 mV | 0 … 1'055 |
| 2,5 dB | 0 … 1100 mV | ±6 mV | 0 … 1'365 |
| 6 dB | 0 … 1600 mV | ±10 mV | 0 … 1'985 |
| 12 dB | 0 … 2900 mV | ±50 mV | 0 … 3'599 |

**Die beiden Kandidaten, aus dem Komponentenblatt übernommen (✔K §6.2, dort selbst gerechnet):**

| `R22` | dunkel (1 MΩ) | Zimmer (20 kΩ) | hell (10 kΩ) | Quellimpedanz im Dunkeln | passende Stufe |
|---|---|---|---|---|---|
| 1 kΩ (heute) | 3,3 mV | 157 mV | 300 mV | 1,0 kΩ | **ungeeignet:** Dunkel unter dem Gesamtfehler jeder Stufe |
| **10 kΩ** | 32,7 mV | 1'100 mV | 1'650 mV | 9,9 kΩ | **6 dB** (Fehler ±10 mV → dunkel 3,3 × Fehler); hell > 1'600 mV **sättigt** |
| **47 kΩ** | 148,1 mV | 2'315 mV | 2'721 mV | 44,9 kΩ | **12 dB** (Fehler ±50 mV → dunkel ≈ 3 × Fehler); hell nahe der 2'900-mV-Grenze, **bei +10 % Versorgung bereits darüber** |
| 22 kΩ | 71,0 mV | 1'729 mV | 2'269 mV | 21,5 kΩ | nur 12 dB; Dunkel 1,4 × Fehler |
| 100 kΩ | 300 mV | 2'750 mV | 3'000 mV | 90,9 kΩ | **scheidet aus** (hell > 2'900 mV) |

Zählwerte (gerechnet, `U / 3,3 × 4'095`): bei 47 kΩ dunkel rund **184**, Zimmer rund **2'873**, hell rund **3'377**; bei 10 kΩ dunkel rund 41, Zimmer rund 1'365, hell rund 2'047. Zum Vergleich
heute (1 kΩ): rund 4, 195, 373.
**Alle Zahlen sind gerechnet, nicht am Gerät gemessen.**

**Was sich ändert und was daraus folgt:**

1. **Auflösung.** Vorher nutzte die Anzeige knapp 369 von 4'095 Schritten (rund 9 %); bei 47 kΩ jetzt rund 3'190, bei 10 kΩ rund 2'000. Das ist der
   Zweck der Änderung (der ADC ist **unten** ungenau, deshalb soll der dunkle Wert von 3 mV in den Bereich rücken, in dem der Fehler kleiner ist als der Wert).
2. **Die alte Kalibrierung ist ungültig.** `ldr_min_value` und `ldr_max_value` stammen von der Skala mit `R22` = 1 kΩ
   (✔ `ldr.c:114-115` liest sie aus dem EEPROM). Ein Backup-Import von der alten Uhr bringt **falsche** Grenzwerte mit.
   AK17b misst sie neu; AK29 nimmt sie aus dem Backup-Vergleich aus. **Kein Backup-Import ohne anschliessende
   Neukalibrierung** — sonst klemmt `ldr.c` jeden Wert an `ldr_max_value` und die Uhr bleibt dauerhaft dunkel oder hell.
3. **Der Gesamtfehler ist das Mass, nicht ein Prozentsatz.** ✔K: Unterhalb des Gesamtfehlers der gewählten Stufe sind Messwerte „nicht aussagekräftig"; das Datenblatt nennt für den unteren Bereich **keinen eigenen Wert** und keine
   zulässige Quellimpedanz. Deshalb verlangt AK17 eine **Messreihe** mit einer Toleranz „Gesamtfehler plus 1 %" statt eines festen Prozentwerts, und es zählt keine Stufe unter dem Fehler als bestanden.
4. **Quellimpedanz und Kondensator.** Die empfohlene **Quellimpedanz steht in keiner der vier Unterlagen** (✔K §6.1, offen). ✔K: **0,1 µF am Eingang** sind empfohlen **und Messbedingung** der Tabelle. Zeitkonstante bei 47 kΩ (Quellimpedanz im Dunkeln 44,9 kΩ):
   `τ ≈ 4,5 ms`, bei 10 kΩ rund 1 ms (✔K §6.2: 9 ms bei 100 kΩ) — **gegen die 250 ms Abtastperiode vernachlässigbar**, und gegen die Helligkeitsänderung ±1 Stufe je Messung (✔ `ldr.c:84-91`) ebenfalls. Ein Fehler durch Eingangsleckstrom
   (max. 50 nA, ◌ Gültigkeit für den Analogpfad) wäre bei 91 kΩ bis 4,5 mV, bei 10 kΩ 0,5 mV.
5. **Rauschen.** Das Datenblatt empfiehlt **Mehrfachabtastung und Mittelung** (✔K, Fussnote Tab. 5-5); gemessen sind die Werte **mit abgeschaltetem WLAN**. Der Treiber bildet den **Mittelwert über N Einzelmessungen** (Startwert **N = 32**, Spec-Setzung, in Task 4.2 an der Streuung AK17c eingestellt;
   AK17 mittelt zum Nachweis über 64). Der Mittelwert **ist** die Aussage; Ausreisser werden nicht gesondert behandelt. Die Mehrfachabtastung läuft **einmal je 250 ms** im Display-Task und kostet nur Mikrosekunden (◌ geschätzt, Task 4.2 misst die Dauer und trägt sie ein).
6. **Skala bleibt die alte.** Der Treiber gibt `U_mV / 3'300 × 4'095` zurück — ein Zählwert **proportional zur Spannung**
   wie am STM32 (3,3-V-Bezug). So bleiben `ldr.c`, die PWA (zeigt `LDR_RAW_VALUE_NUM_VAR`, ✔ `ldr.c:56-60`) und die
   Kalibrierung **im Rechenweg** unverändert; **nur die Zahlenwerte** sind neu. Die Umrechnung nutzt die **Kalibrierung ab Werk**; ohne sie wären bis ±9 % Fehler allein durch die Referenz möglich (✔K §6.1: wahre Referenz 1000 … 1200 mV je Chip).
7. **Nicht ratiometrisch.** ✔K §6.2: Die Knotenspannung ist proportional zur 3,3-V-Schiene (3,0 … 3,6 V = ±9 %), die ADC-Referenz aber nicht. **Schwankt die Schiene mit der WLAN-Last, schwankt der Messwert mit.** Für eine reine Helligkeitsstufung genügt das; **AK17c
   misst den Zählwert unter WLAN-Dauerlast**, und F1/AK37 betreffen diese Schiene.
8. **Nur ADC1.** ✔K §6.1: ADC2 teilt sich den Wandler mit dem WLAN (Messung kann unterbrochen werden); GPIO1 ist ADC1 (H12). **GPIO3 ist Strapping und ausgeschlossen.**
9. **`H5` bleibt am Netz.** Der 2-polige Stecker `H5` (✔ `v2.xml`: `H5.2` am Netz `LDR`) bleibt bestückt und unverändert (E12). Ein dort angesteckter
   Verbraucher würde den Abgriff belasten; **das Netz mit `R22`-Pin 2 ist dasselbe**, also gilt AK17a auch für diesen Fall.

**Was die Messung in Task 4.2 liefert** (und AK17/AK17a/AK17b/AK17c verlangen): die **Messreihe** (≥ 8 Spannungsstufen, Soll/Ist/Abweichung), Teilerspannung am
unveränderten LED-Board bei drei Beleuchtungen, die neuen `ldr_min_value`/`ldr_max_value` mit Messprotokoll,
Helligkeitsverlauf über fünf Stufen, Rauschen über 60 s, **und alles noch einmal unter WLAN-Last**. **Damit entscheidet die Messung F15**, nicht diese Spec.

### 4.3 Die LED-Ausgabe (Detail)

- **Ablauf je Bild:** Pixelpuffer (RGBW, ✔ 32 Bit je LED, Reihenfolge Grün, Rot, Blau, Weiss) → RMT-Encoder → Sendung per DMA →
  Rückruf „fertig" → Pause ≥ 250 µs, bevor das nächste Bild startet (am Treiber, nicht am Aufrufer). Der Aufrufer
  (`display.c`) sieht **dieselbe** asynchrone Schnittstelle wie heute (die sechs Funktionen aus §3.6).
- **Fehler:** Ein Sendefehler **wird gezählt** und im Mitschnitt gemeldet (AK7: Zähler = 0). Kein Fehler wird verschluckt.
- **Pegelwandler:** Am Nachbau U3 (`HARDWARE.md`: SN74AHCT1G125, TTL-Eingang; Datenblatt `sn74ahct1g125.pdf` liegt vor, der Analyst hat es **nicht gelesen** — F18); am DevKit ein **externer**
  74AHCT-Baustein (F8, AK10). ✔K §5: Ein 3,3-V-Ausgang erfüllt VOH ≥ 2,64 V. Die Leitung `GPIO5` steht im Ruhezustand auf **Low**.
- **Versorgung:** `GPIO6` schaltet über R19/Q2/Q1. Die Software setzt `GPIO6` **als allererste Anweisung der Anwendung auf Low** und schaltet die Versorgung erst nach
  vollständigem Hochlauf ein (`HARDWARE.md`: `power_on()` mit 200 ms Einschwingzeit). **Auf V2 hält `R20` (100 kΩ) das Gate von Q2 auf Masse** (✔ `v2.xml`). ✔K §4.2: **GPIO1–18 geben beim Einschalten 60 µs Low aus**
  und sind danach bis zum ersten Treiben hochohmig; **ein Low-Glitch schaltet den N-Kanal-Q2 nicht ein**, ein P-Kanal-Gate (Low = ein) an GPIO1–18 würde 60 µs lang eingeschaltet — **für das Gate von Q1 gilt das nicht**, weil Q2
  dazwischen liegt. **Was zwischen Reset und erstem Treiben am S3 geschieht, entscheidet die Platine (F2), nicht die Software**; AK9 misst es.
- **Reihenfolge der Spannungen (zu prüfen in Task 2.1):** Bleibt U3 stromlos, solange die LED-Kette aus ist
  (`HARDWARE.md`: `R13` in der Stückliste), liegt am Dateneingang der Kette nur `R31` (100 kΩ nach Masse).
- **Der Weg zur ersten LED bleibt unverändert** (E10): `R11` 330 Ω auf dem Controller, `R4` 220 Ω auf dem LED-Board,
  zusammen 550 Ω (`HARDWARE.md`; dort „nicht gemessen, nur gerechnet"). AK5 misst die Flanken an `H7` Pin 4
  und, wo zugänglich, an `V1.DIN`.

### 4.4 USB, Flashen und Konsole — Muss (b)

**Zwei Dinge vorab, die die Datenblatt-Analyse belegt (✔K §7, §9) und die frühere Fassung dieses Abschnitts anders annahm:** Die Konsolen über UART0 und über USB sind **getrennte Kanäle** — die USB-Zweitkonsole ist in ESP-IDF
**nur Ausgabe, nicht blockierend, ohne Eingabe, und sie erscheint nur, wenn UART0 nicht verbunden ist**. Die Abnahme verlangt deshalb **keine** gleichzeitige Ausgabe und **keine** Eingabe über beide Kanäle (AK3a).

| Punkt | Entwurf | Belegt durch |
|---|---|---|
| Anschluss | **USB-Serial/JTAG** am S3 (GPIO19 = D−, GPIO20 = D+, ✔K §7), 16-poliger Stecker, ESD-Schutz (H3, H4). **Aussenmasse des Steckers: F16** | AK3, AK3a, AK3b |
| **Kein USB-Geräte-Stack** | Das Abbild bindet **keinen** USB-Geräte-Stack (zum Beispiel TinyUSB) ein. ✔K §7: Konfiguriert die Anwendung GPIO19/20 um (USB-PHY abschalten, andere USB-Funktion, UART/LEDC auf diese Pins), **verschwindet das USB-Gerät, und das automatische Download-Verfahren entfällt**. Beim Arduino-Paket heisst die Einstellung „USB CDC On Boot" — **sie bleibt aus**; welche Einstellung im gewählten Framework gilt, steht in `multiboard-quelle` (F12) | AK3 (Rückfall 2), Task 1.1 |
| Konsole, Konfiguration | **UART0** (GPIO43/44) als Hauptkonsole, **USB-Serial/JTAG** als Nebenausgabe (✔K §9: `CONFIG_ESP_CONSOLE_UART_DEFAULT` und `CONFIG_ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG`; Nebenausgabe **nur Ausgabe**, nicht blockierend, ohne REPL). **Eingabe wird nicht verlangt:** Eine Konsoleneingabe ist im STM-Code nicht gefunden (Grep auf `log_getc`, `log_poll`, `log_get_char`, `log_read`: 0 Treffer — **nur diese Namen** geprüft), und alle Befehle laufen heute über die Brücke. Echte Gleichzeitigkeit **mit Eingabe** auf beiden Kanälen liefert diese Konfiguration nicht (✔K §9); sie wäre eine neue Funktion (E8) | AK3a |
| Am DevKit und am Nachbau | **Am DevKit** läuft die Konsole über die USB-UART-Brücke (CP2102N) an UART0, **am Nachbau** über das native USB: **dieselbe Firmware bedient beide Wege** (✔K §9). Die ROM-Meldungen beim Start erscheinen ohnehin auf beiden (✔K §9) und sind **kein Nachweis** der Anwendungskonsole | AK3a |
| Ohne Gegenstelle | ✔K §9 (● IDF): Nach vollem Puffer wartet der Chip **einmalig 50 ms**, wenn kein Terminal liest — eine kurze Pause im Programm, kein Absturz. Wenn das **nicht** zutrifft, wird die Konsolenausgabe über eine **Warteschlange mit eigenem Task** entkoppelt, damit weder `display_task` noch `net_task` an ihr hängen | AK4 |
| **Flashen: der normale Weg** | `esptool`, Reset und Download-Modus per **DTR/RTS** über das CDC-ACM-Gerät, **ohne Strapping-Beschaltung** (✔K §7, TRM Tab. 33.3-2 bis 33.4-4); abschaltbar über eFuses (diese Spec brennt keine) | AK3 |
| **Flashen: der manuelle Weg (Rückfall)** | **GPIO0 auf Low und EN kurz auf Low** (✔K §7; GPIO46 muss dabei Low sein, im Auslieferungszustand ist er es, ✔K §4.1). **BOOT- und EN-Taster bleiben auf der Platine** — sie sind der **einzige** Weg, wenn die Anwendung USB abgeschaltet hat (H1, F17) | AK3 (Rückfall 2) |
| Beschaltung D+/D− | ✔K §7: HDG empfiehlt **Platz für Reihenwiderstände (Startwert 22/33 Ω) und Kondensatoren nach Masse**, dicht am Chip; das Referenzschema des Moduls zeigt 0 Ω, das DevKit hat gar keine (drei Antworten auf eine Frage, ✔K §13.3); Hersteller empfiehlt **externen Pull-up an D+** für einen stabilen Anfangszustand (D+ zeigt beim Einschalten Glitches von 3,2 ms/2 ms, ✔K §4.2). **Entscheid: `hw-architect`**; die Software ist davon unberührt | H3 |
| JTAG | ✔K §4.1: Im Auslieferungszustand (alle eFuses 0) kommt JTAG vom USB-Serial/JTAG; **GPIO3** wirkt **nur**, wenn `EFUSE_STRAP_JTAG_SEL` gebrannt ist (diese Spec brennt keine eFuse); GPIO3 ist **schwebend**, also **unbeschaltet lassen** (H2). GPIO39–42 sind keine Strapping-Pins und bleiben **frei**. ◌ OpenOCD mit USB-Serial/JTAG-Konfiguration | AK3b |
| Haltepunkte und Watchdog | Beim Anhalten im Debugger bleibt der Task-Watchdog stehen oder läuft ab — ◌ ist **konfigurierbar** (Debugger-Option); AK3b verlangt, dass ein 3-s-Halt **keinen** Reset auslöst. Die 20-s-Frist reicht dafür | AK3b |
| Schlaf | ✔K §7: In Deep-Sleep verschwindet das Gerät vom Host; in Light-Sleep antwortet es nicht. **Die Uhr benutzt keinen Schlafmodus** (kein Treffer in der Anforderung; E8: keine neue Funktion) | — |
| Sicherheit | Wer die Buchse erreicht, kann den Flash lesen und schreiben (siehe Checkliste 3) | — |

### 4.5 Ablauf nach dem Einschalten (Reihenfolge, die die Hardware vorgibt)

**1. `GPIO6` (LED-Versorgung) auf Low, `GPIO5` (LED-Daten) auf Low** — als allererste Anweisung von `app_main`, **vor** Konsole und allem Übrigen. Die Zeit **davor** (ROM, Bootloader, 60-µs-Glitches) trägt allein die Platine (F2, H5).
Dann: Konsole → I²C → EEPROM und RTC → Zeitgeber → ADC → 1-Wire → `display_task` anlegen → Brücke → `net_task` anlegen. **Die LED-Versorgung wird erst eingeschaltet, wenn
`display_task` läuft.** Die Reihenfolge ab Schritt 2 ist ein **Entwurf**, nicht aus dem STM32-Code übernommen (die Reihenfolge in
`main()` ist nicht Zeile für Zeile gelesen; Task 0.4 gleicht ab): Sie folgt aus zwei Tasks — die Brücke muss stehen,
bevor einer von beiden spricht.

### 4.6 Versorgung und WLAN-Last (AK37)

**Was die Komponentenblätter belegen (✔K):**

| Grösse | Wert | Quelle |
|---|---|---|
| Quelle für das Modul | **≥ 0,5 A** | `esp32s3-wroom-1.md` §2, DS-M S. 27 |
| Spitze beim WLAN-Senden | **355 mA** (802.11b, 1 Mbit/s, +20,5 dBm, 100 % Sendeanteil); 297 mA bei 802.11g/54 Mbit/s; 286 mA bei HT20/MCS7; Empfang 95 mA | §8, DS-M Tab. 6-4 |
| Versorgung des Moduls | 3,0 / 3,3 / 3,6 V empfohlen, absolut −0,3 … 3,6 V | §2, DS-M Tab. 6-1/6-2 |
| Entkopplung am Modul | 22 µF + 0,1 µF an VDD33; 10 µF am Platineneingang | §8 |
| SY8301 | **1 A** Ausgang; Strombegrenzung 1,6 … 2,5 A; Spitze bei 0,6 A Last rund 0,65 A | `sy8301abc.md` §3, §4.2 |
| Rest der Schaltung | DS18B20, DS3231, EEPROM, Pegelwandler: **klein** (Werte nicht gelesen) | §4.3 |
| Strom-Reserve | bei 600 mA Gesamtlast **mindestens 250 mA** | §4.3 |

**Die eigentliche Frage ist die Spannung, nicht der Strom** (✔K `sy8301abc.md` §4.5, **Rechnung, Datenblatt schweigt zum Verhalten am Tastgradanschlag**): Weil die Rückkopplung **hinter** der Diode `D2` liegt, muss der Knoten davor rund 3,6 V erreichen; der maximale Tastgrad
ist begrenzt (0,80 bei 2 MHz). Bei 0,6 A rechnet das Blatt für den Verbraucher: **3,20 V bei 4,75 V Eingang, 3,00 V bei 4,5 V Eingang** (bei 2,0 MHz). `VCC_5V` hängt hinter dem TPS2121 und der PTC-Sicherung `F1` — eine USB-Schiene mit 4,75–5,0 V minus diese
Verluste liegt **in diesem Bereich**. Und der Wandler **teilt sich `VCC_5V` mit der LED-Kette** (`HARDWARE.md`): Ein helles Bild drückt die Schiene.

**Daraus folgt AK37:** Das Minimum der 3,3-V-Schiene am Modul wird **unter WLAN-Dauersenden und hellstem Normalbild** gemessen, am Nachbau bei **4,5 / 4,75 / 5,0 V** Eingang (das ist **die Messung, die das Komponentenblatt vorschlägt**). **Grenze ist 3,0 V** (Datenblatt-Minimum). **Messung am DevKit sagt nichts über die Versorgung des Nachbaus** —
das DevKit hat seinen eigenen Regler; sie trägt nur Strom und Verhalten des Moduls ein. Der Brownout-Reset bekommt ein eigenes Wort im Log (§3.4), damit er nicht als Watchdog-Reset erscheint.

## 5. Die Netz-Seite (ESP8266 → ESP32)

Die Netz-Seite ist **voraussichtlich der grössere Teil der Arbeit**, auch wenn die Hardware den Auftrag dominiert: Der ESP-Code
umfasst ✔ 26'867 Zeilen (`*.cpp` und `*.ino`, ohne Kopfdateien), davon `http.cpp` allein 17'651 — gegen 627 Nahtstellen-Treffer in
`src/**` (§1). Das ist eine **Schätzung aus Dateigrösse**, keine Messung.

| Bereich | Heute (ESP8266, ✔) | Am S3 | Aufwand | Risiko |
|---|---|---|---|---|
| WLAN, AP-Modus | `ESP8266WiFi.h` in 6 Dateien; Ereignisse, `wifi.cpp` 210 Z. | ◌ `WiFi.h` des Arduino-ESP32 — **ähnliche** API, **andere** Ereignisse und Stationsstruktur | **M** | Der Zustandsablauf (Station → AP-Rückfall) ist Anwendung und soll **gleich** bleiben; Task 5.3a prüft ihn am Gerät |
| UDP, NTP | `WiFiUdp.h`, `udpsrv.cpp` 324 Z., `ntp.cpp` 282 Z. | dieselbe Klasse | **S** | **C59** (NTP-Antwort nicht an die Anfrage gebunden) bleibt offen |
| HTTP-Server | `http.cpp` **17'651 Zeilen**; ✔ nutzt `lwip/tcp.h`, `lwip/priv/tcp_priv.h` und `include/ClientContext.h` (laut Kopf von `http.cpp`: E4 Ent-32, R1.1 I-5/C72, nur lesend) | **nicht 1:1 portierbar**: ◌ `ClientContext` gibt es im Arduino-ESP32 nicht, lwIP-Privates sind dort **anders**. Die betroffene Stelle (Länge von `_unclaimed`) wird **neu entworfen**, nicht übersetzt | **L** | **Hoch.** Task 0.5 listet die Stellen (✔ 87 Treffer des groben Suchmusters `ClientContext\|lwip/\|ESP\.\|system_\|ets_\|os_\|…` in `http.cpp`, mit Fehltreffern) und schlägt je Stelle einen Ersatz vor |
| Leseschleife über `WiFiClient` | ✔ Annahme „erst `available()` nach `connected() == 0` erneut abfragen" (Checkliste 4: L173, L353, L356/C53) gilt für **Core 3.1.2 des ESP8266** | ◌ Der `WiFiClient` des ESP32 hat **eine andere Innenstruktur** | **M** | **Die Zusage aus der Checkliste gilt am S3 nicht automatisch — sie wird neu gemessen.** Wer die ESP8266-Attrappe (t12, ✔ `tools/checks/auszug/esp/t12`) für den S3 wiederverwendet, setzt ihre Semantik stillschweigend voraus (Checkliste 4, letzter Punkt). Task 5.3b misst am Gerät mit Antworten > 536 Byte |
| Update | `ESP8266httpUpdate.h`, `Updater.h`, `http.cpp`, `ESP-uclock.ino` | ◌ `HTTPUpdate.h` und `Update.h` des Arduino-ESP32; **zwei OTA-Abbild-Plätze** | **M** | Siehe Checkliste 3 (fremdes Abbild) und AK27 |
| LittleFS, `.gz`-only | `LittleFS.h`; PWA im Dateisystem | ◌ `LittleFS.h` des Arduino-ESP32 mit **eigener Partition** | **S** | **`.gz` ausschliesslich, Grösse > 0** (✔ CLAUDE.md-Invarianten) gelten unverändert; `APP_INSTALL_ASSETS` (✔ `http.cpp`) und `install-app.sh` prüfen, dass der **abgeflachte** Name gefunden wird (DIR-017) |
| ESP-eigene Variablen | `EEPROM.h` (Flash-Emulation), `eepromdata.cpp` 269 Z., 15 Treffer | ◌ `EEPROM.h` des Arduino-ESP32 (Flash-Partition) **oder** `Preferences`; **Entscheid in Task 5.3a** | **S–M** | Das Format der ESP-Variablen (CRC-Prüfung, ✔ `CAP var-crc` bei `ESP-uclock.ino:284`) bleibt. Ein anderer Speicherort ändert **nicht** das Format, nur den Ablageort |
| Speichermessung | `ESP.getFreeHeap()`, `ESP.getMaxFreeBlockSize()` (✔ `ESP-uclock.ino:658-659`) | ◌ `ESP.getFreeHeap()`, `ESP.getMaxAllocHeap()` | **S** | Das Diagnosefeld behält seinen Namen; **seine Bedeutung wechselt** (grösster freier Block ist am S3 eine andere Grösse). AK31 vergleicht **innerhalb** des S3, nie gegen die alte Uhr |
| STM32-Flash | `stm32flash.cpp` 1'438 Z., 38 Treffer; ✔ 72 Treffer der Namen `stm32_flash\|stm32flash\|stm32_default\|stm32_reset\|reset_stm32\|STM_` in `http.cpp` und `ESP-uclock.ino` | **entfällt** (E6), siehe §6 | **M** (Entfernen ist Arbeit, nicht Löschen) | Jeder Verweis braucht eine **definierte** Antwort statt Kompilierfehler oder stillem Erfolg |
| `ESP8266`-Besonderheiten | 107 Treffer des groben Suchmusters in 5 Dateien (enthält Fehltreffer) | Task 0.5 sortiert sie | — | — |

**Wie der gemeinsame Quelltext entsteht (F11):** Die ESP-Dateien erhalten `#ifdef ESP32`-Zweige an den Nahtstellen. **Das ESP8266-Ziel
muss danach unverändert bauen und sich unverändert verhalten** (AK33). `esp-developer` ändert **nur** in `ESP8266/ESP-uclock/*.cpp`, `*.h`,
`*.ino` (Besitz heute), und verändert **nichts** unter `data/app/**`. **Wie `multiboard-quelle` die Boards trennt (Dateiauswahl, `#if`, Laufzeit-Schicht), geht vor;** die `#ifdef`-Zweige hier sind die Annahme dieser Spec.

## 6. Der entfallende STM32-Flash-Weg (E6) — und die Reste

| Heute ✔ | Am S3 (Empfehlung F10) |
|---|---|
| `stm32flash.cpp` (1'438 Z.) | **nicht gebaut** |
| `/api/remote_stm32_flash` | bleibt als Endpunkt, antwortet mit einem **benannten Fehler** („nicht unterstützt") statt `{"ok":true}`; **nie** stilles Nichtstun (DIR-010-Lehre: der alte Endpunkt tat bei fehlendem `filename` „stillschweigend nichts") |
| `stm32_default` in `/api/update_status` | Feld bleibt, Wert **leer** und ein zusätzliches Merkmal, das die PWA unterscheiden kann (**Folgespec**, nicht in dieser Änderung) |
| `/api/maintenance_reset_stm32` | **Neustart des gesamten Chips** (`esp_restart()`), Antwort **vorher** senden. Gründe: (1) ein „Reset nur des Display-Tasks" müsste Zeitgeber, Brückenpuffer, LED-Ausgabe, I²C-Zustand und alle Zustandsflags **zurücksetzen** — das ist eine eigene Wiederanlauf-Funktion, die es im STM nie gab (der Reset war Hardware), (2) ein nur halber Neustart hinterliesse **genau** die Art von Zustandsabweichung, die DIR-010 beschreibt, (3) der Neustart des ganzen Chips ist nachweisbar (AK3, AK25) |
| `tools/flash-stm.sh`, DIR-010 | **gelten für die alten Ziele unverändert weiter.** Für den S3 gilt ein eigenes Skript (Task 6.2): prüft Abbild und Geräteart, flasht über USB, weist die gemeldete Version nach, **schlägt fehl, wenn das Gerät anschliessend nicht antwortet** (der Fall, der bei DIR-010 den Anlass gab: ein Vorgang, der „erfolgreich" aussieht, obwohl nichts geschah) |
| `tools/smoke-device.sh` | bleibt für beide Geräte; **die Prüfung der Update-Quelle (DIR-009) bleibt Pflicht** — die eingebauten Vorgaben des ESP zeigten auf den Server des Ursprungsprojekts; der Port **darf diese Vorgabe nicht übernehmen**, sondern leer lassen |

## 7. Betroffene Module

| Datei / Verzeichnis | Änderung | Zuständiger Agent |
|---|---|---|
| `specs/esp32s3-port/**` | Spec, später Ergebnisse von 0.1a–c/0.4/0.5 eintragen | `spec-writer` |
| S3-Baum (Arbeitsname `esp32s3/`, **massgebend `multiboard-quelle`**) | Hardwareschicht (`sk6812_*` auf RMT, `i2c`, `adc`, `onewire`, `delay`, `power` …), Tasks, Brücke, Gerüst, Testprogramme, Systemkonfiguration, Partitionstabelle, Linker-Fragment, Ersatz-Kopfdateien falls nötig | **S3-Rolle (Vorschlag `s3-developer`; legt allein der Lead an, Task 0.3)** |
| `src/main.c`, die Stelle von `F_INTERRUPTS`, neue Plattformdatei(en) unter `src/` | Nahtstellen herauslösen, `F_INTERRUPTS` wählbar. **Nicht `src/display/display.c`** (E11, AK35) | `stm-developer` (Task 5.1) |
| `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` | `#ifdef ESP32`-Zweige; `StmLink`-Nutzung; STM-Flash-Endpunkte | `esp-developer` (Tasks 5.3a–c) |
| `Makefile`, `CMakeLists.txt`, `cmake/**`, `src/main.h`, `ESP8266/ESP-uclock/version.h` | neues Target, Versionen | `release-engineer` |
| `tools/**`, `.claude/**` | Hooks (`file-ownership.py`, `no-build.py` je nach Framework), Pin-, Symbol- und `display.c`-Unverändert-Prüfung, `flash-s3.sh`, `smoke-device.sh`, `watch-log.sh`, `guardrails.sh` (S7), `device.conf` für zweites Gerät, Agent der S3-Rolle | **Lead** |
| `HARDWARE.md`, `CHANGELOG.md`, `BEFUNDE.md`, `ERWEITERUNGEN.md`, `knowledge/directives.md`, `CLAUDE.md` | Nachführen nach Abschluss | `doc-writer` |
| `ESP8266/ESP-uclock/data/app/**` | **keine Änderung** | — |
| `hardware/komponenten/**`, `hardware/architektur/**`, `hardware/esp32s3/**`, `hardware/review/**` | **Hardware-Spezifikation, Schaltplan, Layout, Gutachten — ausserhalb dieser Tasks, ausser den Zulieferungen 0.1a–c.** Diese Spec **verweist nur darauf** | `hw-datasheet-analyst`, `hw-architect`, `hw-designer`, `hw-reviewer` |
| `specs/multiboard-quelle/**` | Architektur (Framework, Verzeichnisse, Naht als Schnittstelle, Bau-Matrix). **Nicht diese Spec** | `sw-architect` |
| KiCad-Projekt der Controller-Platine, LED-Board | Controller: Nachbau im **unveränderten Format** (Nutzer, Hardware-Rollen). **LED-Board: nicht berührt** | Nutzer |

## 8. Prüfung gegen die Architektur-Checkliste

Pflicht (`knowledge/architecture-checklist.md`). Jeder Punkt wird beantwortet, nicht abgehakt.

**Proper architecture**

> **Antwort.** *PWA parallel zu Legacy:* Beide Oberflächen werden von derselben HTTP-Schicht ausgeliefert; **der Port ändert
> `data/app/**` nicht** (AK21, `git diff` leer) und die Legacy-Oberfläche bleibt Stabilitäts-Referenz — sie läuft am S3 mit
> demselben Code wie am ESP8266, **sodass ein Fehler der PWA gegen Legacy am S3 abgegrenzt werden kann**. *Wetter:* `/api/weather_get_now` und
> `/api/weather_get_forecast` bleiben; der Wetterabruf nutzt `WiFiClient` gegen eine externe Quelle, **dessen Leseschleife am S3
> neu gemessen wird** (§5, Checkliste 4) — **kein Rückbau auf `/weather?action=`**. *Restore-Bedingung:* alle **vier** Teilbedingungen
> von `pending_ticker_restore` bleiben im Quelltext **unverändert** (`main.c:4910-4914`, AK24); Task 5.1 darf diese Zeilen nicht
> berühren. Dass nur noch **ein** Task die Zustandsmaschine schreibt (AK23), macht keine der vier Bedingungen redundant: Sie
> beschreiben Ablaufzustände (Ticker läuft, Icon aktiv, Stop-Zeit offen, Overlay aktiv), nicht Wettläufe zwischen Tasks. *`.gz`-only:*
> bleibt, Grösse > 0 bleibt, `install-app.sh --check` prüft das LittleFS des S3 **erstmals** (DIR-017, das Dateisystem des Neugeräts
> ist leer). *Richtige Schicht:* Plattformeigenheiten (Zeitgeber, Watchdog, ADC-Skala, USB, **LED-Ausgabe**) werden in der **Hardwareschicht**
> gelöst, **nicht in `display.c`** — die LED-Ausgabe über die **vorhandene** Treiber-API (§3.6), keine zweite Zustandsmaschine (E11). Ein Fehler der
> Hardwareschicht wird **nicht** in der PWA umgangen. **Eine Stelle verletzt die Schicht bewusst:** die **Nahtstellen in `main.c`** (Task 5.1) — dort
> liegen Plattform und Anwendung heute in einer Datei, und sie zu trennen ist der einzige Weg zu einem gemeinsamen Quelltext. Das ist eine
> Änderung am produktiven STM-Zweig und braucht den STM-Flash und den `pwa-tester` an der alten Uhr (AK33). *Der LDR-Teiler:* Die Anpassung an die
> ADC-Eigenschaften des S3 geschieht in **Hardware** (`R22` und Kondensator auf dem Controller, E9) und in der **Hardwareschicht** (Skalenumrechnung), **nicht** in
> `ldr.c` und nicht in der PWA; das LED-Board bleibt unberührt (E10). *Abstimmung:* Die Architektur über alle Boards (Framework, Dateiauswahl, Board-Beschreibung) liegt bei
> `specs/multiboard-quelle/`; diese Spec widerspricht ihr nicht (F12).

**Scalable systems**

> **Antwort.** *Kommandos je Aktion:* unverändert — Protokoll und Anwendungslogik bleiben, also erzeugt jede Aktion **dieselbe Zahl
> an Brückenkommandos** wie heute. *Byte je Minute auf der Brücke:* **weniger**, weil die Debugzeilen der HTTP-Anfragen
> (✔ rund 100 Byte je Anfrage) die Konsole statt der Brücke erreichen (§3.3). *Rückstau:* Am UART begrenzte die Leitung (11,5 kB/s),
> am S3 nur noch der Strompuffer (1'024 Byte je Richtung, Startwert) — **ein Sender ohne Quittung füllt ihn schneller, als der Leser
> abholt**. Das ist die neue Grenze und **AK22 provoziert sie absichtlich**; verworfene Zeichen werden **gezählt** (`d=`), nichts
> verschwindet still. *Unter 20 s:* Der Task-Watchdog bleibt bei 20 s (§3.4). Jeder **neu eingeführte** Wartepfad ist begrenzt: Brücke
> `BRIDGE_TX_BLOCK_MS`, I²C 5 ms je Flag-Äquivalent, RMT mit Timeout, Konsole höchstens eine Pause von rund 50 ms (✔K). **Die bestehenden langen Pfade** (✔ `var_send_buf` bis 3 s je Kommando,
> `eeprom_write` je Seite rund 15 ms, Overlay-Bereich im schlimmsten Fall rund 0,9 s am F411) **bleiben** — am S3 sind sie **nicht belegt**,
> weil der I²C-Treiber ein anderer ist (AK12 misst). *Blockade des Hauptloops:* LED-Ausgabe per DMA asynchron, ADC einmal je 250 ms
> mit 32 Einzelmessungen (Mikrosekunden, ◌ geschätzt); **neu** ist, dass die Blockade des `display_task` das WLAN **nicht** mehr anhält
> (Kern 0) — und umgekehrt. Dabei gilt ausdrücklich: **das ist kein Nachweis, dass der F411-Hänger entfällt** (AK26). *PWA-Abfragen:* unverändert,
> aber jeder HTTP-Request kostet **keinen** Debugtext mehr auf der Brücke. *Versorgung:* Das Modul zieht **355 mA** Spitze beim Senden (✔K); die 3,3-V-Schiene hat bei 0,6 A Last **mindestens 250 mA** Stromreserve, aber
> **knappe Spannungsreserve bei schwacher 5-V-Schiene** (✔K Rechnung) — AK37 misst sie. *Hartkodierte Grenzen:* Brückenpuffer, Stapelgrössen der Tasks,
> Partitionsgrössen, Sendepuffer der LED-Ausgabe, ADC-Dämpfung und `R22`-Wert sind **benannte Konstanten**; die LED-Kette bis 234 LEDs ist nachgewiesen (AK8), nicht
> mehr.

**Secure by design**

> **Antwort.** *`innerHTML`/`escapeHtml`:* kein Eingriff in die PWA. *Update-Server nicht vertrauenswürdig:* **neu am S3** ist die
> Frage, ob ein falsches Abbild (ESP8266-`.bin`, abgeschnittene Datei) das Gerät unbrauchbar macht. ◌ Der IDF prüft Abbild-Kopf, Chip-Kennung und
> Prüfsumme **vor** dem Umschalten der Boot-Partition; der Entwurf **verlässt sich nicht darauf**, sondern prüft in AK27 mit einem
> **tatsächlich fremden** Abbild. *Eingebaute Update-Vorgabe:* leer, **nicht** der Server des Ursprungsprojekts (§6, DIR-009). *Keine Credentials in
> Logausgaben:* **die Konsole ist am S3 ein echter Ausgang** (UART0-Stecker, USB), nicht mehr nur die STM-Debug-UART; die ROM-Meldungen beim Start erscheinen zudem auf beiden Wegen (✔K §9) und tragen
> keine Anwendungsdaten. Alle Regeln aus `specs/bruecke/` (P6: Timeout-Meldung ohne Variablenwerte) gelten **unverändert**, und **jede neue Logzeile des Ports** darf weder WLAN-Schlüssel
> noch Wetter-Schlüssel noch Update-Pfad enthalten — Task 5.7 prüft das mit `grep` über die neuen Dateien. *Neue Angriffsfläche:* USB-Serial/JTAG
> erlaubt jedem mit Zugriff auf die Buchse **Auslesen und Beschreiben des Flash** (◌ ohne eFuse-Sperre), also auch das Lesen der ESP-eigenen Variablen
> mit dem WLAN-Schlüssel. Die Uhr hängt an der Wand, die Buchse ist erreichbar. **Das ist bekannt, nicht behoben:** Secure Boot und Flash-Verschlüsselung
> brennen eFuses und sind unumkehrbar — **dafür braucht es eine eigene Entscheidung des Nutzers** (Nicht Teil dieser Änderung). *Neue Endpunkte, die
> Konfiguration schreiben:* **keine**; die Endpunkte der Gefahrenliste (R5) und der zurückgestellte Schutz (C6 u. a.) bleiben, wie sie sind.
> *Hardwareseitig:* Es gibt **keinen** dokumentierten 5-V-Toleranzwert und **keinen Grenzwert je Pin** (✔K §5); jeder S3-GPIO gilt als **höchstens 3,6 V**, ein 5-V-Pegel an einem S3-Eingang ist ein Bauteilschaden (F3); H7 verlangt
> 3,3-V-verträgliche Eingänge.

**Stable & reliable**

> **Antwort.** *Jeder Fehler wird ausgewertet:* RMT-Sendefehler gezählt (AK7), I²C unterscheidet NACK von Störung (AK14), Brückenverlust gezählt (AK22),
> ADC-Fehler **nicht** als „0" gemeldet, sondern als Fehler, damit `ldr.c` keinen Nullwert als „dunkel" liest (Task 4.2). *Leere Fänge/stille Verwerfungen:*
> ein voller Brückenpuffer hat einen `else`-Zweig mit Zähler (§3.3); es gibt kein „ganze Zeile verworfen" ohne Marke auf der nächsten durchkommenden Zeile.
> *Werte zurechtgebogen:* Der ADC-Wert wird auf die **alte Skala umgerechnet** (§4.2) — das ist eine Umrechnung mit **Beleg** (AK17 nennt Toleranz und Datenblattfehler), keine stille
> Klemmung; `ldr.c` klemmt weiter nur eine Kopie (✔ `ldr.c:56-61`). *Zustand nach Abbruch:* Ein Reset (Watchdog, Absturz, Unterspannung, Strom weg) startet mit
> **LED-Versorgung aus**, Ruhepegeln auf GPIO5/6 und leerer Brücke; was **zwischen** Reset und erstem Treiben auf GPIO6 geschieht, entscheidet die
> Platine (F2) und wird am Oszilloskop **gemessen** (AK9). Der Reset-Grund wird beim Start gemeldet, **Brownout mit eigenem Wort** (§3.4, AK25), damit ein Neustart nicht unbemerkt bleibt (DIR-013).
> *Rückweg nach einem Fehler:* Konfiguriert eine fehlerhafte Anwendung GPIO19/20 um, ist die Uhr per USB nicht mehr flashbar — **der manuelle Weg über BOOT- und EN-Taster ist der Rückweg und wird nachgewiesen (AK3, Rückfall 2)**;
> ohne die Taster wäre ein solcher Fehler ein **Totalverlust** des Zugangs.
> *Flags:* Der Zeitgeber-Interrupt **setzt**, `display_task` **löscht**, beide auf Kern 1 (§3.5); jedes Flag hat damit einen Verbraucher. `var_send_busy` und
> `got_ack` sind Anwendungslogik und bleiben unberührt. *Neue Absturzarten:* Ein Zugriff auf Flash-Rodata aus einem Interrupt bei ausgeschaltetem Cache führt zum Absturz —
> **dagegen** stehen das Linker-Fragment und AK16 (100 Schreibvorgänge). *`WiFiClient`-Leseschleife:* **nicht übernommen, neu gemessen** (§5, Task 5.3b); die Attrappe aus t12 wird
> für den S3 **nicht** wiederverwendet, ohne ihre Semantik gegen den ESP32-Client zu prüfen. *Konsole ohne Gegenstelle:* darf weder `display_task` noch `net_task` blockieren (AK4); die einmalige Pause von 50 ms ist bekannt (✔K).
> *Wiederverwendung:* Eine **übernommene** Logik ist nur so zuverlässig wie die Naht; deshalb prüft AK35 mit **Zählung** der betrachteten Dateien, und die Übersetzungsprobe in Task 1.1
> zeigt, ob die „hardwarefreien" Module ohne STM-Kopfdateien übersetzen (§3.6). *Prüfungen, die zuletzt versagt haben müssen:* DIR-014 und AK34.

## 9. Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| Der S3 ersetzt nur den ESP-12F, der STM32 bleibt | Ist **Variante A**, vom Nutzer nicht gewählt (E1–E12). Nicht neu verhandelt |
| Ein einziger Task mit kooperativer Schleife (STM-Hauptschleife und `loop()` nacheinander) | Der Nutzer verlangt zwei Tasks (E5); ausserdem würde ein blockierender Pfad (✔ `eeprom_write` je Seite 15 ms, `var_send_buf` bis 3 s) das WLAN anhalten — der Zustand, den zwei Chips heute vermeiden |
| Direkte Funktionsaufrufe statt Brücke | Ändert das Protokoll, die PWA und jede Stelle, die heute auf eine Antwortzeile wartet. **Verworfen ausdrücklich vom Nutzer (E5)** |
| Zwei Hardware-UARTs des S3 intern verbinden (Pin-Matrix-Schleife) statt Strompuffer | Erhielte die Baudrate-Drosselung des heutigen UART, kostet aber zwei Peripherien und Interrupts für **nichts**, was der Strompuffer nicht ebenfalls kann; **vom Nutzer als Warteschlange entschieden (E5)** |
| **Eine neue Anzeige-Zustandsmaschine oder ein neues Anzeigemodul für den S3** | **Vom Nutzer ausgeschlossen (E11):** `display.c` (6'739 Zeilen) ist die bewährte Logik; eine zweite Kopie driftet und verdoppelt jeden künftigen Fehler. Die Naht ist die Treiber-API |
| **`#if`-Zweige in `sk6812.c` statt eines Nachbaus in der Hardwareschicht** | Brächte eine Änderung unter `src/**` in Stufe 2, die ausdrücklich **ohne** auskommen soll (E7), und `sk6812.c` trägt Konfiguration für sechs Boards. Eine spätere Zusammenführung bleibt möglich und ist eine eigene Aufgabe |
| **`DISPLAY_OWNER_ASSERT` an jeder Schreibstelle** (erster Entwurf von AK23) | Verlangt Zeilen in `display.c`; **verworfen** zugunsten von Symbolprüfung und Task-Prüfung an den Grenzen (§3.2) |
| **Konsoleneingabe über beide Kanäle als Abnahme** (frühere Fassung von AK3a) | ✔K §9: Die USB-Zweitkonsole ist nur Ausgabe, und sie erscheint nur, wenn UART0 nicht verbunden ist. Echte Gleichzeitigkeit mit Eingabe wäre eine **neue Funktion** (E8); verworfen |
| **Das Framework in dieser Spec festlegen** | Aufgabe von `sw-architect` (`specs/multiboard-quelle/`), „mit Preis"; diese Spec liefert nur Eingaben (§1.1) |
| **Pins in dieser Spec und zusätzlich im Code von Hand führen** | Zwei Listen driften; Vorgabe `sw-architect`: eine Quelle je Board (§2) |
| LED-Ausgabe über SPI-MOSI, I²S oder LCD-Peripherie statt RMT | ◌ funktioniert, aber der RMT-Encoder bildet SK6812-Bits direkt ab und braucht keinen Puffer-Aufblasfaktor; Entscheid kann in Task 2.1 **mit Messung** revidiert werden, wenn RMT-DMA nicht trägt (Errata RMT-176 ist dort zu lesen) |
| `esp_timer` statt GPTimer für den 20-kHz-Takt | ◌ `esp_timer` ruft aus einem Task ab (Verzögerung, nicht 50 µs-genau); IRMP braucht einen **Interrupt** |
| Interne Pull-ups für I²C und 1-Wire | ✔K typ. 45 kΩ, ◌ zu schwach (F7); externe Widerstände sind Pflicht |
| Softwareglättung gegen das Unterlaufen des ADC unten **statt** `R22` zu ändern | Vom Nutzer entschieden (E9): der Widerstand ändert sich, nicht die Software |
| Ein Fork von `main.c` für den S3 | Drift (DIR-006-Mechanik); verworfen zugunsten der Nahtstellen (§1) |
| Secure Boot und Flash-Verschlüsselung gleich mitnehmen | unumkehrbar, eigene Entscheidung nötig; nicht Teil dieser Änderung |
| Das LED-Board für den S3 anpassen (zum Beispiel den LDR-Teiler dorthin verlegen) | Vom Nutzer ausgeschlossen (E10): Änderungen betreffen nur die Controller-Platine |
| Ein anderes Leiterplattenformat (zum Beispiel kleiner, weil der STM32 entfällt) | Vom Nutzer ausgeschlossen (E12): Umriss, Löcher, Stecker, Taster bleiben |

## 10. Versionsfolgen

Nur der `release-engineer` führt das aus (R4). **Empfehlung F13**, bis der Nutzer und `sw-architect` entscheiden:

- [ ] STM `src/main.h` anheben — **Task 5.1v** (Nahtstellen; DIR-004: kein Build ohne Anhebung der geänderten Komponente)
- [ ] ESP `version.h` anheben — **Task 5.3v** (ESP8266-Zweig geändert) **und** für das S3-Abbild (Task 6.1)
- [ ] App `APP_VERSION` — **bleibt** (PWA unverändert, AK21)
- [ ] `CACHE_NAME` in `sw.js` — **bleibt**
- [ ] S3-Abbild trägt `VERSION` **und** `ESP_VERSION` (die PWA zeigt zwei Versionen); Tag `release/<stm>-<esp>-<app>`
- [ ] Der Makefile liest die Versionen weiter per `grep` aus (✔ CLAUDE.md); **das Zeilenformat darf sich nicht ändern**, sonst bricht der Release-Build still

## 11. Ergebnisse der Klärungs-Tasks — hier eintragen

Wird vom `spec-writer` in Task 0.2 aus den Berichten von 0.1a–c, 0.4 und 0.5 gefüllt. **Bis dahin stehen hier keine
Ergebnisse und keine Vermutungen.** Was die vorhandenen Komponentenblätter schon beantworten, steht in `requirements.md` bei den Fragen F1–F6, F15 (mit ✔K).

```
T0.1a Komponentenblätter noch ausstehend: USB-C 6P/16P Aussenmasse (F16), 74AHCT1G125
      mit 3,3-V-Eingang (F18), SKC6812-Toleranz (F9), RMT-Errata RMT-176 (Einzelbeschreibung):  offen
T0.1b Pinzuordnung entschieden, F3 (Schiene der R8/R9-Pull-ups), F7 (Pull-ups),
      F15 (R22/Dämpfung), F17 (BOOT-Taster-Ort), GPIO21-Frage (F2):                              offen
T0.1c Gutachten Pin-Tabelle gegen Schaltplan (AK1), Zahl der betrachteten Netze:                 offen
T0.4  Schnittstellenliste der Kopfdateien, hardwarenahe/hardwarefreie Dateiliste (a, k):         offen
T0.4  Zustandsschreiber (b), Quittungsprüfung je Kommando im Ist-Stand (c):                      offen
T0.4  Funktionsinventar (E8, AK30):                                                              offen
T0.4  F14 (F_INTERRUPTS), dcf77_tick im Interrupt?                                               offen
T0.5  Liste der ESP8266-Stellen, Sortierung der 495 Serial-Stellen, Zufallszahlen-Nutzung:       offen
```
