# Design — Portierung auf einen einzelnen ESP32-S3 (Variante B)

**Stand:** 2026-10-17 (dritter Stand: R22 = 1 kΩ, IR/DCF77 ohne Stecker, Konsole `H6`, Abgleich mit `specs/multiboard-quelle/`, §12). Zeichen **✔** = im Repo belegt, **✔K** = im Komponentenblatt der Datenblatt-Analyse belegt (`hardware/komponenten/esp32s3-wroom-1.md`,
`sy8301abc.md`; ich habe die Blätter gelesen, nicht die PDFs), **◌** = Herstellerwissen oder Annahme, nicht belegt, mit dem Task, der es prüft
(Legende und Warnung zu `HARDWARE.md` in `requirements.md`). Wo Fachwissen fehlt, steht eine **Empfehlung** und der **Task, der sie belegt** — nichts davon ist hier als Befund gesetzt.
**Wo `specs/multiboard-quelle/` (Entwurf, nicht freigegeben) dieselbe Arbeit beschreibt, führt sie aus und diese Spec verweist; deren Zahlen werden hier nicht kopiert.**

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
          ▼  sk6812-Backend (RMT), i2c, adc, onewire, delay, power, GPIO
   LED-Board über H7 (unverändert) · DS3231 + AT24C32 · DS18B20 · IR · DCF77 · Taster
```

**Vier Grundsätze, aus den Entscheidungen E1–E13 abgeleitet:**

1. **Die Logik bleibt, die Nahtstellen wandern.** `display.c`, `vars.c`, `overlay.c`, `weather.c`, `night.c`
   und die übrigen Module ohne Hardware-Zugriff werden **nicht angefasst** (E11, AK35; Einzelheiten §3.6). Gezählt (Grep vom 17.10.2026, Muster
   `GPIO_|TIM_|RCC_|NVIC_|IWDG_|USART_|DMA_|ADC_|I2C_|EXTI_|SysTick_|SYSCFG_|__disable_irq|__enable_irq`):
   **627 Treffer in 15 von 25 geprüften Dateien**; in `display.c`, `vars.c`, `ds18xx.c`, `ldr.c`, `log.c`,
   `flash.c` und `esp-spiffs.c` sind es **0**. Wo ein Treffer steht, ist eine Nahtstelle. **Einschränkung, gemessen von `multiboard-quelle` (§2 B1, §3.2 dort):**
   Die Logikdateien nennen **direkt** keinen Herstellerheader, **transitiv** ziehen alle über ihre Kopfdateien welche herein; die Naht ist eine **Include-Naht** und braucht Attrappen (§3.6).
2. **Die Naht der LED-Ausgabe ist ihre Treiber-API.** `display.c` kennt den LED-Treiber nur über Makros, die auf
   `sk6812_*` abgebildet sind (✔ `display.h:94-99`; `display.c` selbst braucht davon drei, die Schnittstelle hat sechs). Der S3 liefert **dieselbe** Schnittstelle als **Backend des gemeinsamen Kerns** von `sk6812.c`; `display.c`
   merkt den Unterschied nicht (§3.6). **Das ist der erste Nachweis (Stufe 2) — und er berührt keine Zeile `display.c`.**
3. **Ein gemeinsamer Quelltext statt eines Forks** (Annahme F11: die alten Ziele bleiben). Eine Kopie von
   `main.c` mit 5'378 Zeilen driftet binnen Wochen von der Vorlage weg — dieselbe Mechanik wie bei Versionsnummern
   in der Doku (DIR-006). Das leistet **`multiboard-quelle`** mit **Seam-Guards** (`WC_PLATFORM_EXTERNAL` in `main.c` u. a., §3.6) statt mit einer ausgelagerten Plattformdatei: Der Bestandszweig bleibt wörtlich, der Hash
   der Bestandsabbilder bleibt gleich (dort Tasks 3.1–3.8, hash-nachgewiesen). Das ersetzt die frühere **Task 5.1** dieser Spec. **`display.c` gehört nicht dazu.**
4. **Die Brücke bleibt eine Brücke.** Beide Seiten sprechen weiter Zeilen miteinander. Das ist teurer als ein
   direkter Funktionsaufruf, aber es ist die einzige Zusage, die Protokoll, PWA und Legacy-Oberfläche unberührt
   lässt — und die einzige, die der Nutzer verlangt hat (E5). **Ersetzt wird nur die unterste UART-Schicht, nicht `esp8266.c`** (W10, §3.3).

### 1.1 Framework, Build-System, Verzeichnis, Besitz — **entscheidet `specs/multiboard-quelle/`**

**Diese Spec legt das Framework nicht fest.** Die Architektur (Naht, Verzeichnisse, Board-Beschreibung, Bau-Matrix, **Framework mit Preis**) ist Aufgabe von `sw-architect`
in `specs/multiboard-quelle/` (✔ `.claude/agents/sw-architect.md`). **Stand beim dritten Nachtrag:** Jene Spec liegt als Entwurf vor, **nicht freigegeben**; sie empfiehlt in `design.md` §8.1 ESP-IDF 5.5.x mit Arduino-ESP32 als Komponente (O1),
**entschieden ist es nicht** (Q5 beim Nutzer; `idf.py` ist auf dem Rechner nicht installiert). Was ich beitrage, ist **Eingabe**:

| Beobachtung | Quelle | Folge für die Entscheidung |
|---|---|---|
| Dieser Entwurf braucht Schalter in der Systemkonfiguration: IRAM-sichere Zeitgeber- und RMT-Interrupts, 20-s-Task-Watchdog, Konsole primär UART0 **und** sekundär USB-Serial/JTAG, eigene Partitionstabelle | ◌ aus dem Entwurf; Task 1.1 zeigt, ob das gewählte Framework sie erlaubt | Ein Framework, das diese Schalter nicht zulässt, **scheidet aus**, wenn der Preis nicht tragbar ist. `multiboard-quelle` belegt für die vorkompilierten Arduino-Bibliotheken, dass mehrere dieser Schalter dort **nicht** gesetzt sind (`design.md` §8.1, Option O2); die Anforderung AK16 (Interrupt fehlerfrei bei Flash-Schreiben) ist die, die O2 ausschliesst |
| Die Konsolenkonfiguration hängt am Framework | ✔K `esp32s3-wroom-1.md` §9, §14 | `sw-architect` nennt sie in seiner Spec; AK3a bleibt unverändert |
| `src/` liegt ausserhalb jedes Sketch-Ordners | ✔ Verzeichnisaufbau | **Die Dateiauswahl kommt aus `boards/modules.txt`** (Spalte S3: `use`, `replace`, `drop`), **nicht** aus einer von Hand gepflegten Liste (W3). §3.6 trägt die frühere Messung dieser Spec als Grundlage |
| Beide Quellbäume tragen gleichnamige Dateien `display`, `vars`, `tables`, `base` | ✔ `src/display/display.c`, `ESP8266/ESP-uclock/display.cpp` usw. | Kopfdateinamen und globale Symbole **kollidieren** in einem Abbild. Zwei IDF-Komponenten mit getrennten Include-Pfaden und **eine Symbolprüfung am gelinkten Abbild** (`multiboard-quelle` C9, Task 4.1, AK17; Auflösung per `-D<alt>=<neu>`, nicht im Quelltext); diese Spec verlangt sie in AK23 (a) und AK34 |
| Pins stehen an **einer** Stelle | ✔ Vorgabe in `sw-architect.md` | **`boards/<id>/board.h`** (Signale in `board-pins.h`; für den S3 `boards/wordclock-esp32s3-v3/`, dort Task 2.1). **Es gibt kein `esp32s3/pins.h`** (W2); die Pinprüfung ist C5 / AK9 dort |
| Bauen darf nur der Lead (R1); `tools/hooks/no-build.py` weist heute `make`, `cmake`, `arduino-cli` ab, **`idf.py` fehlt dort** | ✔ CLAUDE.md | Ergänzung durch den Lead in `multiboard-quelle` Task 0.3 |
| Besitz des neuen Verzeichnisses | ✔ `tools/hooks/file-ownership.py`, Wurzelmuster kennt nur `src/`, `ESP8266/ESP-uclock/`, `tools/`, `.claude/`, `knowledge/`, `specs/`, `cmake/`, `Makefile`, `CMakeLists.txt`, `*.md` | **Ohne Eintrag lässt der Hook Schreibzugriffe auf das neue Verzeichnis ungeprüft durch** (und er ist ohnehin nur eine Erinnerung, B22/L171). Besitz nach `multiboard-quelle` `design.md` §7: `esp32s3/**` und `boards/wordclock-esp32s3-*/**` für `s3-developer`; `boards/modules.txt`, `boards/baseline/**` für den Lead; **die Rolle legt allein der Lead an** (dort Task 0.3). Das Verzeichnis `esp32s3/` ist **nicht** `hardware/esp32s3/` (KiCad, Besitzer `hw-designer`) |

**Annahme für die Aufgabenplanung** (`tasks.md`): Der S3 baut als **eigenes Ziel** neben den alten; die alten Ziele bleiben unverändert baubar (F11, AK33). **Widerspricht `multiboard-quelle` dieser Annahme, geht der Widerspruch
an den Lead** — keine der beiden Specs überschreibt die andere.

## 2. Pinbelegung

Quelle: Vorschlag des Nutzers. **Spalte „abgeglichen" bleibt leer, bis die Tasks 0.1a bis 0.1c den Schaltplan des Nachbaus gelesen haben.**
Die Tabelle ist AK1; eine Zeile ist erst „abgeglichen", wenn Netzname **und** Header-Pin eingetragen sind. **Die Pinzuordnung entscheidet `hw-architect`**
(Ablage `hardware/architektur/pinbelegung.md`, liegt noch nicht vor; **maschinenlesbare Form nach `multiboard-quelle` Q7/Task 0.5**), die Prüfung leistet `hw-reviewer`; diese Spec trägt das Ergebnis ein (Task 0.2). **Die Pins werden im Code
an einer einzigen Stelle geführt (`boards/<id>/board.h` mit `board-pins.h`), nicht hier und nicht in einer zweiten Hand-Liste** (§1.1, W2).

| Signal | GPIO | DevKit-Header (✔K §3.2) | Richtung | Zustand beim Einschalten (✔K §4.2) | Besonderheit | abgeglichen (Netz / Header-Pin) |
|---|---|---|---|---|---|---|
| LDR (Abgriff von `H7` Pin 5) | 1 | J3 | Analog-Eingang | 60 µs Low, danach hochohmig | ✔K ADC1 (GPIO1–10), bleibt bei WLAN nutzbar (ADC2 nicht). **`R22` = 1 kΩ wie V2** (E13; bis 17.10.2026 stand hier „`R22` und 0,1 µF am Pin (H9)“ mit `R22` ≈ 47 kΩ); ob 0,1 µF (`C32`) bestückt wird, ist offen (F15 Teil b). Netz `LDR` trägt auf V2 zusätzlich `H5.2` (✔ `v2.xml`). Das TRM kennt einen dritten Boot-Modus mit **GPIO1 = 1, GPIO2 = 0** bei GPIO0 = 0 und GPIO46 = 1 (✔K §13.1) — GPIO46 ist im Auslieferungszustand Low, der Fall tritt nicht ein | |
| DS18B20 | 2 | J3 | Ein-/Ausgang, offener Kollektor | 60 µs Low, danach hochohmig | Pull-up extern (F7); ein Pull-up hält GPIO2 hoch, der dritte Boot-Modus verlangt 0 (siehe oben) | |
| LED-Daten | 5 | J1 | Ausgang (RMT) | 60 µs Low, danach hochohmig | danach U3; `R31` hält die Leitung bei stromlosem U3 (`HARDWARE.md`) | |
| LED-Versorgung | 6 | J1 | Ausgang | **60 µs Low, danach hochohmig** | **F2:** V2 hat `R20` (100 kΩ) am Gate von Q2 (✔ `v2.xml`); N-Kanal-Q2 → Low-Glitch harmlos (✔K); GPIO21 wäre laut Komponentenblatt der sauberere Kandidat (Entscheid `hw-architect`; E13: ein anderer Pin ist eine Abweichung) | |
| IR | 7 | J1 | Eingang mit Pull-up | 60 µs Low, danach hochohmig | 3,3-V-Pegel zwingend (F3); **auf V2 kein Stecker**, nur Pull-up `R8`/`R9` (✔ Skill); **wo der Empfänger hängt: offen (F20)**; hängt nicht an `H7` | |
| SDA | 8 | J1 | Ein-/Ausgang, offener Kollektor | 60 µs Low, danach hochohmig | externer Pull-up (F7) | |
| SCL | 9 | J1 | Ein-/Ausgang, offener Kollektor | 60 µs Low, danach hochohmig | externer Pull-up (F7) | |
| DCF77 | 15 | J1 | Eingang mit Pull-up | 60 µs Low, danach hochohmig | ✔K GPIO15/16 sind ab Werk Anschlüsse eines 32-kHz-Quarzes; ohne Quarz frei. **Auf V2 kein Stecker** (wie IR; F20) | |
| Taster USER | 16 | J1 | Eingang | 60 µs Low, danach hochohmig | Pull-up (F7) | |
| Taster WPS | 17 | J1 | Eingang | 60 µs Low, danach hochohmig | Pull-up (F7); ✔K Treiberstärke 10 mA | |
| Status-LED | 18 | J1 | Ausgang | 60 µs Low **und** 60 µs High | ✔K Treiberstärke 10 mA; ein High-Glitch beim Einschalten ist für eine LED ohne Folgen | |

**Nicht benutzt, mit Grund (E2):** GPIO0, 3, 45, 46 (Strapping; ✔K §4.1); 19, 20 (USB); 43, 44 (UART0); 35–37 (PSRAM/Flash der R8-Modelle — und das **heute verkaufte DevKit ist N8R8**, ✔K §3.3); 38, 48 (RGB-LED des DevKits;
✔K: bei v1.1 GPIO38, bei v1.0 GPIO48). Zusätzlich ✔K nicht herausgeführt: GPIO22–32. **GPIO19 und GPIO20 dürfen vom Programm nie umkonfiguriert werden** — wer sie als GPIO nutzt, schaltet USB-Serial/JTAG ab und
verliert das **automatische** Flashen, die Konsole und JTAG in einem Zug; dann hilft nur der manuelle Weg (§4.4, AK3 Rückfall 2).

**Zwei Anschlüsse stehen ausserhalb dieser Tabelle und sind offen (dritter Stand):** (1) **IR und DCF77 haben auf dem Controller keinen Stecker** — die Anforderung und die offene Frage stehen in §4.7 und F20. (2) **Die Konsole `H6`** (UART0, GPIO43/44) trägt die Zuordnung
`TXD0` → Netz `TX_UART` → `H6` Pin 2, die gegenüber V2 vertauscht sein könnte (F19, §4.4, AK38). GPIO43/44 stehen bewusst **nicht** in der Tabelle (E2: Sperrliste), sie sind die Konsole.

**Prüfung:** Ein Skript liest die **eine** Pinquelle (`board.h`/`board-pins.h`), zählt die Signale (**11**) und weist jede Nummer der Ausschlussliste ab. **Das ist die Prüfung C5 / AK9 von `multiboard-quelle`** (dort Task 2.1, Gegenprobe P5), nicht eine zweite in dieser Spec.
Sie ersetzt **nicht** das Gutachten von `hw-reviewer` über den Schaltplan (AK1).

## 3. Software-Architektur

### 3.1 Schichten

| Schicht | Inhalt | Besitzer | Ändert sich? |
|---|---|---|---|
| Anwendung (STM-Seite) | `display.c`, `vars.c`, `overlay.c`, `weather.c`, `night.c`, `tables.c`, `alarm.c` u. a. | `stm-developer` | **nein** (AK35; Ausnahme `eeprom.c`, §3.6) |
| Anwendung (Netz-Seite) | `http.cpp`, `vars.cpp`, `weather.cpp`, `ESP-uclock.ino` u. a. | `esp-developer` | Nahtstellen (§5) |
| Nahtstelle STM-Seite | Seam-Guards in `main.c`, `sk6812.c`, `irmp`, `esp8266.c`; Pin-Zweige in `main.h`, `eeprom.c`, `dcf77.c` | `stm-developer` | **ja, hash-gleich** — `multiboard-quelle` Tasks 3.1–3.8 (ersetzen die frühere Task 5.1) |
| Hardwareschicht | S3-Treiber und Backends mit **denselben Kopfdateien** wie heute (`delay.h`, `i2c.h`, `adc.h`, `onewire.h`, `sk6812.h` …, **unverändert aus `src/`**), dazu Attrappen (`esp32s3/shim/`) | S3-Rolle (Vorschlag `s3-developer`, `multiboard-quelle` Task 0.3) | neu |
| Plattform | `app_main`, Tasks, Brücke, Systemkonfiguration, Partitionstabelle | S3-Rolle | neu |

Die Hardwareschicht **behält die Funktionsnamen und Rückgabewerte** der heutigen Treiber und **bindet die originalen Kopfdateien ein**.
Dann bleiben `rtc.c`, `ldr.c`, `ds18xx.c`, `remote-ir.c` und `display.c` **unverändert** im Build (`eeprom.c` mit dem Zweig E3). Welche Schnittstelle
das je Kopfdatei genau ist, steht in `multiboard-quelle` `design.md` §3.4 (Funktionen, Kontext, Blockierzeit, S3-Anbieter); Task 0.4 dieser Spec bestätigt sie, statt sie ein zweites Mal aufzulisten.

### 3.2 Zwei Tasks, ein Schreiber

| | `display_task` | `net_task` |
|---|---|---|
| Kern | **1** | **0** (◌ Arduino-`loopTask` läuft ab Werk auf Kern 1, WLAN auf Kern 0; der Entwurf legt `net_task` daher selbst an, statt sich auf den Voreinstellungswert zu verlassen — Task 1.1 prüft die Zuordnung) |
| Inhalt | bisherige `main()`-Schleife von `src/main.c` | bisherige `setup()`/`loop()` von `ESP-uclock.ino` |
| Priorität | höher als `net_task` | normal |
| Stapel | benannte Konstante, Startwert im Gerüst gemessen (`uxTaskGetStackHighWaterMark`) und mit Reserve eingetragen — **keine Zahl aus dieser Spec** | wie links |
| Watchdog | `watchdog_reload()` ruft `esp_task_wdt_reset()`; registriert, 20 s | Kopf von `loop()` ruft `esp_task_wdt_reset()`; registriert, 20 s (**neue Aufrufstelle**, siehe §3.4) |
| Darf schreiben auf | **alle** Display-Zustände | **nichts** davon; nur Brücke, WLAN, LittleFS, ESP-eigene Variablen |

**Display-Zustände, die nur `display_task` schreibt** (Liste wird in Task 0.4 vollständig erhoben — **zusammen mit `multiboard-quelle` Task 0.4(a), ein Bericht, nicht zwei**; belegt sind schon
`display`-Struktur, `display_clock_flag`, `pending_ticker_restore` (✔ `main.c:1553`, Schreibstellen `:3400`, `:4256`,
`:4918`), `show_icon_stop_time`, `show_overlay_idx`, Ticker-Zustand hinter `display_ticker_active()`). Der Netz-Task
**berührt diese Namen nicht** — er schickt Kommandozeilen, die der `display_task` ausführt, **genau wie heute**.
Ein HTTP-Handler, der „der Einfachheit halber" `display.…` liest oder schreibt, ist die Verletzung, die AK23
auffangen soll.

**Wie AK23 prüft — ohne `display.c` anzufassen (E11).** Der frühere Entwurf setzte ein Makro an jede Schreibstelle. Das hätte
**Zeilen in `display.c`** verlangt und ist **verworfen** (§9). Stattdessen zwei unabhängige Wege, die beide **an den Grenzen** liegen, nicht in der Anwendung:

(a) *Statisch, am gelinkten Abbild:* `nm` über beide Komponenten. Die Netz-Komponente hat **keine** unaufgelöste Referenz auf ein Symbol
der STM-Komponente (und umgekehrt) — Funktionen **und** globale Variablen. Weil beide Seiten nur über die Brücke verbunden sind, **kann** der Netz-Task
keine Display-Variable schreiben, ohne dass der Linker sie als Querverweis zeigt. Die Prüfung nennt die **Zahl der betrachteten Symbole**; sie ist **C9 von `multiboard-quelle`** (Task 4.1 dort, Gegenprobe P9), nicht ein zweites Werkzeug.
(b) *Dynamisch, im Debug-Build, an den Übergängen:* Jede Funktion der **Hardwareschicht** (I²C, ADC, LED-Treiber, 1-Wire, Brücke, Konsole)
ruft `xTaskGetCurrentTaskHandle()` und vergleicht mit dem erwarteten Task (`display_task` für die STM-Seite, `net_task` für `StmLink`).
Das Makro liegt im S3-Baum, ist im Release-Build leer und wird **einmal absichtlich verletzt** (ein Aufruf des LED-Treibers aus
`net_task`); es muss abbrechen. Es sieht **Schreibzugriffe auf die Hardware**, nicht Schreibzugriffe auf Variablen — Letztere schliesst (a) aus.

**`pending_ticker_restore` (E5).** Die Bedingung steht heute bei `main.c:4910-4914` (✔): `pending_ticker_restore &&
! display_ticker_active () && ! display.do_display_icon && show_icon_stop_time == 0 && show_overlay_idx >= MAX_OVERLAYS`.
**Die Seam-Guards in `main.c` (`multiboard-quelle` Task 3.6) dürfen diese Zeilen nicht verändern**, auch nicht umformatieren (AK24; **Prüfung C6 dort vergleicht den Text gegen eine Referenz, nach jedem Task**). Dass der
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
| Schnittstelle STM-Seite | `esp8266_uart_putc/getc/flush`-Familie (✔ 25 Treffer des Suchmusters `uart_(init\|putc\|getc\|poll\|flush)\|esp8266_uart_\w+\s*\(` in `esp8266.c`) — die **Instanz `esp8266`** aus `uart.h` (`multiboard-quelle` §3.4: je Instanz 14 Funktionen) | **dieselben Namen**, ersetzt in der Hardwareschicht (`esp8266-uart.c`) | **`esp8266.c` (1'097 Zeilen) bleibt mit dem Zeilenprotokoll erhalten** (E5, W10). Sechs Logikmodule (`weather.c`, `timeserver.c`, `esp-spiffs.c`, `tables.c`, `tetris.c`, `snake.c`) rufen `esp8266_send_cmd`/`esp8266_get_message` direkt — eine Entfernung der Schicht änderte sie alle (Beleg dort, Problem 6). Einzige Änderung in `esp8266.c`: der Seam-Guard `WC_PEER_EXTERNAL` um die Steuerpins und den Boot-Pfad (§6, `multiboard-quelle` Task 3.7) |
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
| Aufrufstellen | ✔ Guardrail S7, `WD_EXPECTED`; **Stand beim Umsetzen am Quelltext lesen**. Bis 17.10.2026 stand hier die Zahl 7 (aus `specs/bruecke/`); `multiboard-quelle` nennt in `tasks.md` (Task 3.6) für S7 eine andere Zahl — **die Zahlen widersprechen sich, Task 0.4(i) zählt nach und trägt sie ein** (§12) | die **gleichen** (Aufruf bleibt `watchdog_reload()`, Rumpf ändert sich) **plus eine neue** im Kopf von `net_task` |
| Wirkung | Reset des STM; der ESP läuft weiter | Reset des **gesamten Chips**, also auch des WLAN. **Das ist neu und gewollt:** Ein hängender Netz-Task hat sonst niemanden, der ihn zurückholt |
| Start-Meldung | `IWDGRST` in `main.c:457-459` (✔) | `esp_reset_reason()` wird beim Start in **dasselbe Wort** übersetzt; **ein Brownout-Reset erhält ein eigenes Wort** (AK25, AK37); `watch-log.sh` und `measure-log.sh` erkennen beide (Task 5.5 prüft die Muster gegen den Quelltext der Werkzeuge) |
| Fallstrick | Einen Watchdog zu bedienen, ohne einen Ausweg zu schaffen, tauscht Reset gegen unbegrenzte Blockade (✔ `specs/bruecke/`, P1/P7) | gilt unverändert; **jeder neue Warteweg im Port ist begrenzt** (Brücke: `BRIDGE_TX_BLOCK_MS`, I²C: 5 ms, **RMT: Frist im Backend**, nicht im gemeinsamen Kern — `multiboard-quelle` Q12, Empfehlung (a); der Kern wartet dort weiter ohne Frist und bleibt unverändert) |

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
| Wirkung auf IRMP | Die IRMP-Konstanten leiten sich zur Übersetzungszeit aus `F_INTERRUPTS` ab (✔ `irmp.c:58`, `:90-137`); die Rechnung bleibt, nur der Wert ändert sich. **Bis 17.10.2026 stand hier: „eine Änderung an der Stelle von `F_INTERRUPTS` durch `stm-developer`".** **Das Makro steht hinter `#ifndef` (`irmpconfig.h`) und wird per Übersetzer-Schalter aus `BOARD_TICK_HZ` in `board.h` gesetzt — ohne Quelltextänderung** (`multiboard-quelle` Q8) | AK19 |
| Flash-Zugriffe | ◌ Während Flash geschrieben wird (LittleFS, NVS, OTA), ist der Cache aus; ein Interrupt, der Code oder Rodata **aus dem Flash** liest, stürzt ab. Deshalb: Interrupt-Funktionen und ihre Tabellen **im RAM**. Für `irmp.c` (5'114 Zeilen, viele Tabellen) **ohne Quelltextänderung** über eine Linker-Fragmentdatei, die `.text` und `.rodata` des Objekts in IRAM bzw. DRAM legt (Form hängt vom Framework ab, F12) | AK16 (100 Schreibvorgänge) |
| Rumpf | Der Rumpf `TIM2_IRQHandler` ist zur Hälfte Anwendung und **bleibt auf dem S3 derselbe Text**; nur die Quittung ist das Makro `WC_TICK_ACK()` hinter dem Seam-Guard `WC_PLATFORM_EXTERNAL`, der GPTimer-Rückruf ruft den Körper (`multiboard-quelle` §3.7, Task 3.6). **`dcf77_tick()`** (✔ in `src/dcf77/dcf77.c`) ruft `log_message` und `board_led_*` **aus der ISR**: Entscheid **`multiboard-quelle` Task 3.9 / AK20** (ISR-festes Ringpuffer-Schreiben oder Aufruf in den Hauptloop, Letzteres Klasse F), Umsetzung Task 4.1; **er steht vor Stufe 4** | Task 0.4 (ISR-Pfade, **mit** `multiboard-quelle` Task 0.4(c)), 3.9 dort |
| Flags | Der Interrupt **setzt**, `display_task` **löscht** — ein Verbraucher, wie heute. `volatile` bleibt. Der Interrupt läuft auf **demselben Kern** wie der Verbraucher; das schliesst Kern-übergreifende Wettläufe aus (`multiboard-quelle` §3.7 nennt das eine Randbedingung, kein Detail) | Task 4.1, AK23 |

### 3.6 Wiederverwendung — was unverändert bleibt, was Backend und Hardwareschicht ersetzen (E11, AK35; neu gefasst 17.10.2026)

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

Aufrufe laut Grep: **19** in `display.c` (nach `multiboard-quelle` §2 B4 davon `led_init`, `led_refresh`, `led_set_led`; `led_set_all_leds` benutzt nur der Treiber selbst), **2** in `main.c` (die Zähler). `SK6812_RGBW` hat die Felder `red`, `green`, `blue` und, wenn `DSP_USE_SK6812_RGBW == 1`, `white` (✔ `sk6812.h:37-43`);
die Konstante kommt aus der Konfiguration (`wclock24h-config.h`/`display-config.h`), die der S3-Build **unverändert** einbindet. Auf der Leitung geht zuerst **Grün**
(✔ `sk6812.c:354-356`); die Reihenfolge der übrigen Bytes ist Grün, Rot, Blau, Weiss (laut Auftrag und `multiboard-quelle` V11 — **Task 2.1 belegt sie am Quelltext, bevor er sie nachbaut**).

**Was der S3 liefert — Backend statt zweitem Treiber (Neufassung).** **Bis 17.10.2026 stand hier:** `sk6812.c` (671 Zeilen) werde für den S3 nachgebaut, die rund 120 bis 128 hardwarenahen Zeilen entfielen, übernommen werde nur „die Logik, die klein ist" (Doppelpuffer, `set_led`, `set_all_leds`,
Zähler, Zeitwerte), und das geschehe **„in der Hardwareschicht, nicht durch `#if` in `sk6812.c`"**, weil das die Stufe-2-Zusage (keine Zeile `src/**`) nicht hielte. **Das ist ersetzt.** `multiboard-quelle` zeigt (`design.md` §3.6): Ein Nachbau kopiert den portablen Kern und lässt ihn driften;
stattdessen steht in `sk6812.c` **ein Seam-Guard `WC_LED_BACKEND_EXTERNAL` um die drei Hardwareblöcke** (A: DMA-Init und -Start, B: Puffer-Aufbereitung und ISR, C: `sk6812_init`), **der Kern bleibt an seinem Platz**, und der Tokenstrom für jeden Bestandsbuild bleibt derselbe (Abbild **byte-gleich**, Hash-Nachweis,
Klasse H2). Der S3 übersetzt **dieselbe Datei**; sein Beitrag ist das **Backend** `esp32s3/hal/sk6812-backend.inc`:

- **Backend-Vertrag** (dort §3.6): `sk6812_init` (ruft am Ende `sk6812_clear_all` des Kerns), `sk6812_setup_dma_buf` (auf dem S3 darf sie leer sein), `sk6812_dma_start` (erzeugt aus dem aktuellen Puffer die RMT-Folge **G, R, B, W** und setzt `sk6812_dma_status` am Ende der **Pause** auf 0 — **die Pause von 250 µs ist eingerechnet**), Fertig-Rückruf (Kontext D).
- **Die Zeitwerte** (Periode 1250 ns, T0H 300, T1H 750, T0L 950, T1L 500, Pause 250 µs, ✔ `sk6812.c:91-97`) stehen nach `multiboard-quelle` Task 3.3 **einmal** in `sk6812-timing.h`; der Träger wechselt (Zeitgeberwerte → RMT-Symbole), die Werte nicht. **Der Zweig `#if 0` für SKC6812 bleibt zu.**
- **Beide Zähler sind echte Zähler** (Bilder gesendet; Fälle, in denen ein Bild auf das vorige warten musste) — sie liegen im Kern und sind damit **geerbt**, nicht nachgebaut.
- **Eine Frist im `refresh`-Wait** (die Vorgängerfassung dieser Spec sah `rmt_tx_wait_all_done` mit Timeout vor) ist eine **Änderung der Klasse F im gemeinsamen Kern** und gehört **nicht** hinein: `multiboard-quelle` Q12, Empfehlung (a) — die Frist gehört dem **Backend**, indem es `sk6812_dma_status` bei Zeitüberschreitung löscht. **Entscheid des Nutzers.**
- **Preis, ehrlich:** `sk6812.c` ist damit **Bestandsdatei mit Guard** (nicht „keine Zeile `src/**`"); sie bleibt für alle Bestandsziele hash-gleich nachzuweisen. Wer sie liest, muss den Guard kennen. Dafür gibt es **keine geforkte Zeile**.
- **Reihenfolge:** Der Guard (dort Task 3.3) setzt die reproduzierbare Baseline und die Tasks 3.1/3.2 voraus. Der S3-Stufe 2 geht damit Arbeit am Bestand voraus — **F21**.

**Module, die unverändert übernommen werden** (alle `.c` ohne Treffer des Einbindungsmusters `#include "stm32…"`, `"misc…"`, `"core_cm…"` — Grep vom 17.10.2026 über `display`, `overlay`,
`vars`, `tables`, `font`, `alarm`, `night`, `weather`, `timeserver`, `rtc`, `eeprom`, `ldr`, `ds18xx`, `tempsensor`). **Verbindlich ist ab jetzt die Klasse „Logik" in `boards/modules.txt`** (`multiboard-quelle` AK2, AK7); sie ersetzt diese Handliste, sobald die Datei existiert:

| Gruppe | Dateien | Beleg |
|---|---|---|
| Anzeige und Anwendung | `display.c` (**6'738 Zeilen**, nachgezählt; bis 17.10.2026 stand hier 6'739, W5), `overlay.c`, `vars.c`, `tables.c`, `font.c`, `alarm.c`, `night.c`, `weather.c`, `timeserver.c` | **0** Einbindungen von STM-Kopfdateien in den `.c`-Dateien dieser Verzeichnisse (Grep) |
| Sensor- und Speicherlogik | `rtc.c`, `eeprom.c` (**mit Zweig E3**), `ldr.c`, `ds18xx.c`, `tempsensor.c` | dito; **die Kopfdateien** binden STM-Kopfdateien ein (siehe Berichtigung) |
| Typen und Konfiguration | `base.h`; **`main.h` nicht** | `base.h` ja. **`main.h` bricht ohne Chip- und Oszillatorangabe mit `#error` ab** (✔ `multiboard-quelle` §2 B2): Der S3 braucht dort **einen Zweig** (E1). **Bis 17.10.2026 stand hier:** „laut Auftrag ziehen `main.h` und `base.h` nur `time.h`, `stdint.h` …" — für `main.h` widerlegt |

**Berichtigung zum Auftrag, am Quelltext — und ihre Widerlegung durch Messung.** Der Auftrag nannte für `rtc`, `eeprom`, `ldr`, `ds18xx` und `tempsensor` „keine STM-Header" (0 Treffer). **Für die `.c`-Dateien
stimmt das; die Kopfdateien tragen Einbindungen:** `rtc.h:16-20`, `ds18xx.h:17-19`, `tempsensor.h:17-19`, `eeprom.h:16-18` und `ldr.h:16-24` binden
`stm32f10x.h` beziehungsweise `stm32f4xx.h` ein — jeweils unter `#if defined (STM32F10X)` und `#elif defined (STM32F4XX)` (✔ `rtc.h:16-20`, die übrigen mit demselben Muster
gelesen). **Bis 17.10.2026 stand hier die Annahme:** „Der S3-Build definiert keines von beiden, dann fällt jede dieser Einbindungen weg und die Module übersetzen **ohne** STM-Kopfdateien; eine Übersetzungsprobe in Task 1.1 zeigt es."
**Die Messung von `multiboard-quelle` (`design.md` §3.2) widerlegt das (W1):** Ohne Attrappen übersetzt **keines** der Logikmodule, weil acht Kopfdateien (darunter `sk6812.h`) `misc.h` **unbedingt** einbinden und weitere an `uint_fast8_t` scheitern.
**Mit** einer Attrappe `misc.h`, den erzwungenen Einbindungen (`-include stdint.h` u. a.) und einem Platzhalter für die Chipkennung übersetzen alle bis auf `eeprom.c` unverändert; `eeprom.c` stoppt bei `EEPROM_PAGE_SIZE`. Die genauen Zahlen stehen **dort**, nicht hier.
**Folgerung für das S3-Gerüst (Zulieferung, in der Spec bis dahin nicht genannt):** (1) Attrappe `misc.h`; (2) die erzwungenen Einbindungen; (3) ein **echter** Zweig für Chip und Oszillator in `main.h` (E1, Klasse A, statt des Platzhalters); (4) die Seitengrösse aus `board.h` (E3). Alles liegt in `esp32s3/shim/` und `boards/`;
das ist **`multiboard-quelle` Task 2.2** und **Teil von Task 1.1 hier — nicht doppelt anlegen**. **Was die Attrappen nicht belegen:** Übersetzbarkeit auf dem Entwicklungsrechner, nicht Verhalten; deshalb nagelt `multiboard-quelle` AK11 die Breiten und Layouts auf jedem Ziel per `_Static_assert` fest (siehe AK13).

**Eingriffe in Bestandsdateien, die der S3 unvermeidbar macht** (Stand der Messung dort, `design.md` §9.3; **jede weitere Stelle ist freigabepflichtig**; Klasse A/H2 heisst: hash-gleich nachzuweisen):

| # dort | Datei, Stelle | Eingriff | Klasse | Freigabe |
|---|---|---|---|---|
| E1 | `src/main.h` (Chip- und Oszillator-Kette) | Zweig aus `BOARD_HW_CHIP_CODE`/`BOARD_HW_OSC_CODE` | A | mit `multiboard-quelle` (Chip-Code: dort Q4) |
| E2 | `src/main.c` (vier Regionen) | Seam-Guard `WC_PLATFORM_EXTERNAL`, Makro `WC_TICK_ACK()` | H2 | mit `multiboard-quelle` |
| E3 | `src/eeprom/eeprom.c` | Zweig `BOARD_EEPROM_PAGE_SIZE` vor der Familienkette | A | mit `multiboard-quelle` |
| E4 | `src/irmp/**` | Seam-Guard `WC_IRMP_TARGET_EXTERNAL` (Pin-Zugriff, `irmp_init`) | A/H2 | mit `multiboard-quelle` |
| E5 | `src/sk6812/sk6812.c` | Seam-Guard `WC_LED_BACKEND_EXTERNAL`, Zeiten in eine Kopfdatei | H2 + H1 | mit `multiboard-quelle` (Nachtrag zu E11, F21) |
| E6 | `src/esp8266/esp8266.c` | Seam-Guard `WC_PEER_EXTERNAL` | H2 (F nach dort Q11 / hier F10) | mit `multiboard-quelle` |
| E7 | `src/dcf77/dcf77.c` | Pin aus `board-pins.h` | A | mit `multiboard-quelle` |

**Hardwarenahe Dateien, ersetzt oder per Backend bedient durch eine schmale Hardwareschicht mit den bisherigen Funktionsnamen:**

| Datei(en) heute | Ersatz in der Hardwareschicht | Aufwand (§4.1) | Task |
|---|---|---|---|
| `sk6812.c` | **Backend** `sk6812-backend.inc` (RMT) hinter `WC_LED_BACKEND_EXTERNAL`; **Kern gemeinsam** (siehe oben) | M | 2.1 |
| `i2c.c` | I²C-Master, gleiche Funktionen (`i2c_init`, `i2c_read`, `i2c_write`, `i2c_reset_bus`) | M | 3.1 |
| `adc.c` | ADC1 oneshot, `adc_init`, `adc_start_single_conversion`, `adc_poll_conversion_value` | M | 4.2 |
| `onewire.c` | 1-Wire | M | 4.3 |
| `delay.c` | `delay_init`, `delay_usec`, `delay_msec`, `delay_sec` | S | 4.1 |
| `log-uart.c` (und die Ausgabeseite von `log.c`) | Konsole, gleiche Aufrufe | S | 1.1 |
| `power.c` | `power_on`, `power_off` | S | 4.5 |
| `watchdog_reload` (in `main.c`) | `esp_task_wdt_reset` (Backend der Region „Reset-Ursache, Watchdog" von `WC_PLATFORM_EXTERNAL`) | S | 4.1 / 5.4 |
| `main.c` (Takt, Zeitgeber, erste Zeilen von `main()`) | Backends der übrigen Regionen von `WC_PLATFORM_EXTERNAL` (`.inc` in `esp32s3/hal/`) | M | 4.1 / 5.4 |
| `irmp.c` (nur Pin-Zugriff und ISR-Aufruf) / `remote-ir.c` | Backend hinter `WC_IRMP_TARGET_EXTERNAL`, Platzierung im RAM | M | 4.4 |
| `button.c`, `wpsbutton.c`, `dcf77.c`-Pin, `board-led.c` | GPIO | S | 4.5 |
| `esp8266-uart.c`, `uart-driver.h` | Stream-Buffer (§3.3) — **nur die unterste Schicht**; `esp8266.c` bleibt | M | 5.2 |

**Die Zahl der hardwarenahen `.c`-Dateien.** **Bis 17.10.2026 stand hier:** der Auftrag zähle 20, meine Zählung mit dem StdPeriph-Muster treffe 15 von 25 geprüften Dateien, verbindlich sei Task 0.4(a) und (k). **Das ist durch die Messung von `multiboard-quelle` ersetzt (W4):** Sie klassifiziert
jede `.c` unter `src/` in genau einer Klasse und führt die Zählung mit Verfahren; **verbindlich ist `boards/modules.txt`** (`multiboard-quelle` AK2), nicht diese Tabelle und nicht meine frühere Zahl. Neu gegenüber meiner Liste sind dort `irmp.c`, `log-uart.c`, `dfplayer-uart.c`, `esp8266.c`, `main.c`, `apa102.c`, `ws2812.c`,
`system_stm32f4xx.c`; `tetris.c` und `snake.c` sind **Logik** (Spiele auf der Wortmatrix, S3-Funktion nach E8). **AK35 zählt über die Klasse „Logik" dort.** Task 0.4(k) dieser Spec entfällt als eigene Zählung; er **bestätigt** die Klassen (`multiboard-quelle` Task 0.4(b)).

**Ausnahmeverfahren: keine Änderung an `display.c` ohne gesonderten Grund und Beleg.** Wer glaubt, `display.c` oder ein anderes hardwarefreies Modul ändern zu müssen:
(1) nennt den **Grund** (Compilermeldung, Messung, Zeile), (2) prüft zuerst **Attrappe, Compiler-Option oder Hardwareschicht** als Weg ohne Änderung,
(3) trägt die Ausnahme in diese Tabelle ein, (4) wartet auf die **Freigabe des Nutzers**, bevor er schreibt.

| # | Datei | Grund | Beleg | Freigabe |
|---|---|---|---|---|
| E3 | `src/eeprom/eeprom.c` (Klasse „Logik", **nicht** `display.c`) | Zweig `BOARD_EEPROM_PAGE_SIZE` vor der Familienkette `:44-49`; ohne ihn bricht die Datei auf dem S3 mit `#error EEPROM_PAGE_SIZE` — **der einzige Logikfehler im S3-Versuch** | `multiboard-quelle` `design.md` §3.2 und §9.3 (E3); Klasse A, hash-gleich nachzuweisen (dort Task 3.2) | **beantragt, steht aus** — mit der Freigabe von `multiboard-quelle` |

## 4. Hardwareschicht je Baustein

**Aufwandsmasse** (keine Stunden — die wären erfunden): **S** = unter rund 150 neue Zeilen und **ein** Messnachweis.
**M** = 150 bis 600 Zeilen oder **ein Messverfahren mit Gerät**. **L** = über 600 Zeilen, **mehrere** Messverfahren oder
**ein Architekturentscheid**. Die Zeilenzahl der **ersetzten** Datei (✔ Grep) ist angegeben, weil sie das Mass der
Neuschreibung nicht ist: Die meisten alten Dateien tragen mehr Konfiguration für **andere Boards** als Treibercode.
**Alle Aufwandsangaben sind Schätzungen aus Dateigrösse und Schnittstelle, nicht gemessen.**

### 4.1 Tabelle

| Baustein | heute (STM32) ✔ | am S3 | Aufwand | Besonderheit / Risiko | Task |
|---|---|---|---|---|---|
| **sk6812** (LED-Ausgabe) | `sk6812.c` 671 Z., 120 Treffer; TIM3 CC4 + DMA1 Stream 2; Periode 1250 ns, T0H 300, T1H 750, T0L 950, T1L 500, Pause 250 µs | **Backend des gemeinsamen Kerns** (§3.6): **RMT TX** (◌ 4 Sendekanäle, 4 Empfangskanäle am S3) mit Encoder für Bit 0/1 und Reset-Symbol; **DMA-Betrieb**, damit ein 114-LED-Bild (3'648 Bit, ≈ 4,6 ms) nicht vom Speicher des WLAN-Stacks abhängt; **API wie `sk6812.h`** | **M** | Auflösung 10 MHz (100 ns) reicht für ±150 ns; die Pause ≥ 250 µs ist **eine** Pause, nicht 250 µs Daten. ◌ Der DMA-Betrieb ist am S3 an **einen** Kanal gebunden; die Wahl fällt in Task 2.1. Ambilight bis 234 LEDs (≈ 9,4 ms): grösserer Sendepuffer, Speicherbedarf **messen**. ✔K Errata RMT-176 steht in der Zusammenfassung (Einzelbeschreibung **nicht gelesen**) — Task 2.1 liest sie, bevor der DMA-Betrieb gewählt wird. `SKC6812` verlangt > 200 µs Pause (`HARDWARE.md`): **der Zweig `#if 0` in `sk6812.c` bleibt zu**. **Frist im Wait: Backend, nicht Kern (Q12)** | 2.1 |
| **ws2812, apa102, tft, ssd1963, touch, xpt2046, tetris-Anzeige, dfplayer, w25qxx** | `src/ws2812`, `apa102`, … | **nicht gebaut** (diese Hardware gibt es auf dem Print nicht); `dfplayer`: Nullimplementierung (`multiboard-quelle` §3.4) | — | Die Dateien bleiben im Repo für die alten Ziele; im S3-Build steht ihr Umgang in `boards/modules.txt` (`drop`/`replace`) | 1.1 |
| **i2c** | `i2c.c` 540 Z., **142** Treffer, 5 ms Frist je Flag, `i2c_reset_bus()` mit neun Takten, Bus-Freitakten bei BUSY (✔ `i2c.c:163-180`) | **I²C-Master** des IDF; Pins GPIO8/9; Takt wie heute (Wert `BOARD_I2C_CLOCK_HZ` aus `board.h`) | **M** | Die Fehlersemantik muss **gleich** bleiben: NACK ≠ Störung (A73, L402); Frist je Transaktion statt je Flag, **Gesamtfrist ≤ 5 ms je Flag-Äquivalent** (AK15); Bus-Freitakten bei Störung. Der IDF-Treiber hat eigene Timeouts — Task 3.1 stellt sicher, dass **keine** davon länger wartet, als der alte Treiber es durfte | 3.1 |
| **eeprom** (AT24C32) | `eeprom.c` 317 Z., **4** Treffer; seitenweise, 32 Byte, `EEPROM_WAITSTATES 15`, `eeprom_ms_tick` | **unverändert bis auf den Zweig E3** (§3.6), über `i2c.h` und den 1-kHz-Takt aus §3.5 | **S** | `eeprom_ms_tick` kommt aus dem Interrupt (`F_INTERRUPTS / 1000` = 20 Interrupts bei 20 kHz). **Datenformat unverändert, `git diff` leer ausser E3** (AK13) | 3.2 |
| **rtc** (DS3231) | `rtc.c` 397 Z., **2** Treffer | **unverändert** über `i2c.h` | **S** | RTC-Zeit wird bei `hh:mm:45` gelesen (✔ `main.c:950-956`) — gilt unverändert | 3.2 |
| **adc** (LDR) | `adc.c` 140 Z., 38 Treffer; 12 Bit, **Start/Abfrage getrennt** (`adc_start_single_conversion`, `adc_poll_conversion_value`, ✔ `adc.c:114-140`), alle 1/4 s (✔ `main.c:876`) | **ADC1 oneshot** (✔K nur ADC1, GPIO1), Dämpfung **0 dB als benannte Konstante** (`BOARD_LDR_ADC_ATTEN_DB`; bis 17.10.2026 stand hier „6 dB oder 12 dB, F15"), **Kalibrierung ab Werk** (✔K ● IDF: Curve-Fitting aus eFuse-Werten), Umrechnung in mV und **zurück in Zählwerte 12 Bit / 3,3 V** (siehe §4.2); **Mehrfachabtastung** | **M** | Die Rückgabe bleibt `uint16_t` 0–4095 gegen 3,3 V (✔ `LDR_MAX_RAW_VALUE 4095`); `ldr.c` bleibt. Start/Abfrage bleibt **zweistufig**: Die Messung läuft im Aufruf von `…_start_…` oder im Hintergrund, `…_poll_…` liefert das Ergebnis genau einmal. ✔K: Der ADC speist den Zufallszahlengenerator; Lesen vermindert dessen Zufälligkeit (● IDF) — **die Uhr nutzt keinen Zufall für Sicherheit**, Task 0.5 prüft es für die Netz-Seite (TLS/Schlüssel) | 4.2 |
| **ldr** | `ldr.c` 264 Z.; ✔ klemmt eine **Kopie** des Rohwerts (`ldr.c:61-77`), 32 Helligkeitsstufen, ändert den Stufenwert je Messung um **höchstens 1** (`ldr.c:84-91`) | **unverändert** | **S** | Der unveränderte Glättungsschritt in `ldr.c` (±1 Stufe je 250 ms) dämpft Rauschen — die Helligkeit flackert nicht, solange der Rohwert nicht über mehrere Stufen springt (AK17c) | 4.2 |
| **onewire** | `onewire.c` 251 Z., 6 Treffer, Zeitbasis `delay_usec` (SysTick) | **Bit-Banging** mit `esp_rom_delay_us` **oder** RMT-1-Wire (◌ IDF-Komponente `onewire_bus`) — **beide müssen den Abnahmetest bestehen**; die Wahl trifft Task 4.3 nach einer Logikanalysator-Messung | **M** | 1-Wire-Slots sind 1–15 µs genau (◌). Eine Interrupt-Sperre über einen ganzen Slot (≤ 70 µs) **stört** den 20-kHz-Interrupt (50 µs Takt) — IR könnte Takte verlieren, und das **zählt** die Messung (`diag_tick_cnt`). **Die offenen Befunde A63/A64/A65 bleiben offen:** der Port übernimmt `ds18xx.c` unverändert und **darf sie nicht verdecken** (zum Beispiel, indem er den Abtastzeitpunkt zufällig besser trifft, ohne das zu melden) | 4.3 |
| **ds18xx** | `ds18xx.c` 404 Z., 0 Treffer, CRC-8, 255 bei Fehler, drei Fehler = „verloren" (A2, ✔ `CLAUDE.md`) | **unverändert** | **S** | AK18 prüft die Semantik am Gerät | 4.3 |
| **irmp** | `irmp.c` 5'114 Z., 67 Treffer; Aufruf `irmp_ISR()` mit 15 kHz | Algorithmus **unverändert**; Pin-Zugriff und `irmp_init` hinter dem Seam-Guard `WC_IRMP_TARGET_EXTERNAL` (`multiboard-quelle` Task 3.4); `irmp.o` per Linker-Fragment in IRAM/DRAM | **M** | Die Zeitbasis ändert sich (§3.5) → alle Längen in `irmp.c` leiten sich neu ab. Erkennt die Uhr bei 20 kHz **dieselben** Protokolle, die sie bei 15 kHz erkannte? **AK19 entscheidet**, nicht die Rechnung. **Der Eingang hängt an GPIO7 mit Pull-up, ohne Stecker (§4.7, F20)** | 4.4 |
| **remote-ir** | `remote-ir.c`, 0 Treffer | **unverändert** | **S** | `learn_ir` ist bekanntermassen blockierend (✔ CLAUDE.md, R5: „unbegrenzte Blockade ⇒ Watchdog-Reset"). Der Port **übernimmt** das Verhalten, er korrigiert es nicht (E8) | 4.4 |
| **delay** | `delay.c` 104 Z., 4 Treffer; SysTick, **Busy-Wait** auf `delay_counter` (✔ `delay.c:51-69`) | `delay_usec` → `esp_rom_delay_us`; `delay_msec` → `vTaskDelay` ab einer Tick-Länge, darunter Busy-Wait | **S** | ✔ Das heutige `delay_msec` **hält den Hauptloop an**; `vTaskDelay` gibt die CPU ab. Das ist **weicher**, ändert aber nichts an der Reihenfolge der Schritte im `display_task`. **Wer `delay_msec(200)` nach `power_on()` ersetzt, hält die 200 ms Einschwingzeit der LED-Versorgung ein** (`HARDWARE.md`, `display.c:2937`; dort im Aufrufer, `multiboard-quelle` §3.5.2). `delay_sec` bleibt | 4.1 |
| **watchdog** | in `main.c` (`watchdog_init`/`_reload`, ✔ `:399-400`, `:512-546`) | §3.4 (Backend der Watchdog-Region von `WC_PLATFORM_EXTERNAL`) | **S** | Die LSI-Eigenheit des IWDG (✔ `main.c:512-516`: Flags warten auf den Takt) entfällt | 4.1 |
| **dcf77** | `dcf77.c` 537 Z., 29 Treffer; Pin PB8, Takt 10 ms aus dem Interrupt | Pin GPIO15 aus `board-pins.h` (E7); Dekodierung **unverändert** | **S** | Siehe F3 (Pegel), **F20 (kein Stecker)**. Tick im Interrupt oder im Task: §3.5 (`multiboard-quelle` Task 3.9) | 4.5 |
| **button, wpsbutton** | 21 und 14 Treffer | GPIO16/17, interne Pull-ups **nur falls F7 es zulässt** (`BOARD_BUTTON_PULL`) | **S** | Entprellung: nicht gelesen — Task 0.4(j) stellt fest, ob sie in `button.c` (Software) oder im Pull-up (Hardware) liegt | 4.5 |
| **board-led, power** | 26 und 12 Treffer | GPIO18 / GPIO6 | **S** | `power_off()` ist der **Anfangszustand** und **die allererste Anweisung** der Anwendung (§4.5, F2) | 4.5 |
| **uart-Brücke** | `esp8266.c` 1'097 Z., 59 Treffer; `esp8266-uart.c`, `uart-driver.h` | §3.3 — **nur die unterste Schicht** (Instanz `esp8266` aus `uart.h`) | **M** | Semantik des vollen Puffers; keine blockierende `flush`-Wirkung (A19) | 5.2 |
| **log-uart** (STM-Debug) | `log-uart.c`; PA9/PA10 | **Konsole** (§4.4): UART0 + USB-Serial/JTAG | **S** | Das STM-Log landet auf der Konsole **und** im Ring (✔ `log_vprintf` spiegelt auf die Brücke) | 1.1 |
| **Start-up, Takt, Linker, Speicherkarte** | `system_stm32f4xx.c`, `startup_stm32f407xx.S`, Linker-Skripte | **entfällt** (das Framework übernimmt) | — | — | 1.1 |

### 4.2 Der LDR-Eingang — Muss (a), mit Rechnung

**Was der Nutzer festgelegt hat (E9, E10, E13):** LDR und LED-Board bleiben unverändert; der Abgriff läuft über `H7` Pin 5 auf
`GPIO1`; **`R22` auf dem Controller bleibt 1 kΩ** (E13, 17.10.2026); `ldr.c` und die Normierung bleiben. **Bis 17.10.2026 stand hier:** `R22` wechsle auf ca. 47 kΩ, die Kalibrierung werde neu gemessen, und die Datenblatt-Analyse
rechnete 10 kΩ/6 dB und 47 kΩ/12 dB als Kandidaten. **Das ist zurückgezogen**; die Rechnung dazu steht unten als **Vergleich**, ausdrücklich nicht als Entwurf. **Was das Komponentenblatt festhält (✔K `esp32s3-wroom-1.md` §6.3):**
Mit `R22` = 1 kΩ ist die Lesung **funktional gleichwertig, nicht zählergleich**; mit 0 dB, Werkskalibrierung, Mittelung über mindestens 16 Abtastungen und der Abbildung `N_STM = V_mV × 4095 / V_Schiene_mV` bleiben die gespeicherten Grenzen auf **±1 Helligkeitsstufe** gültig,
eine **einmalige Nachkalibrierung** macht sie zählergleich.

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

**Der Entwurf: `R22` = 1 kΩ, 0 dB (✔K §6.3, gerechnet, nicht am Gerät gemessen):**

| Zustand | Spannung | STM-Zähler (ideal, `V × 4095 / 3300`) | S3-Gesamtfehler ±5 mV in Zählern |
|---|---|---|---|
| dunkel (1 MΩ) | 3,3 mV | 4,1 | **±6,2** — nicht auflösbar (0 … 10) |
| Zimmer (20 kΩ) | 157,1 mV | 195,0 | ±6,2 (±3,2 %) |
| hell (10 kΩ) | 300,0 mV | 372,3 | ±6,2 (±1,7 %) |

Ein STM-Zähler entspricht 0,806 mV, ein S3-LSB nominell 0,269 mV (Vref 1100 mV „by design", ✔K TRM §39.3.5). **Zusätzlicher Fehler durch die Schiene** (nicht ratiometrisch): bei 2 % Abweichung der 3,3-V-Schiene Zimmer ±3,9, hell ±7,4 Zähler; bei 3 %: ±5,8 / ±11,2; bei 5 %: ±9,7 / ±18,6 (✔K §6.3 (2), Rechnung).
**Vergleich mit V2 (✔K §6.3 (3)):** Der STM32F411 hat ET max ±5 LSB (≙ ±4,0 mV), typisch ±2 LSB, und ist ratiometrisch; der S3-Gesamtfehler (±5 mV) liegt **im Bereich des F411-Worst-Case** und ist **doppelt so gross wie dessen Typwert**. **Beide lösen den Dunkelwert nicht auf**; `ldr.c` klammert dort auf `ldr_min_value`.

**Die frühere Kandidatentabelle (zurückgezogen, bis 17.10.2026 Entwurfsgrundlage; als Vergleich stehen gelassen, ✔K §6.2):**

| `R22` | dunkel (1 MΩ) | Zimmer (20 kΩ) | hell (10 kΩ) | Quellimpedanz im Dunkeln | passende Stufe |
|---|---|---|---|---|---|
| 1 kΩ (**gilt wieder, V2**) | 3,3 mV | 157 mV | 300 mV | 1,0 kΩ | 0 dB; Dunkel unter dem Fehler — **wie auf V2** |
| 10 kΩ | 32,7 mV | 1'100 mV | 1'650 mV | 9,9 kΩ | 6 dB; hell > 1'600 mV sättigt |
| 47 kΩ | 148,1 mV | 2'315 mV | 2'721 mV | 44,9 kΩ | 12 dB; hell nahe der 2'900-mV-Grenze |
| 22 kΩ | 71,0 mV | 1'729 mV | 2'269 mV | 21,5 kΩ | nur 12 dB |
| 100 kΩ | 300 mV | 2'750 mV | 3'000 mV | 90,9 kΩ | scheidet aus (hell > 2'900 mV) |

**Was daraus folgt:**

1. **Auflösung: wie V2.** Die Anzeige nutzt knapp 369 von 4'095 Schritten (rund 9 %; Beispielspanne aus `HARDWARE.md`, rund 11,5 Zähler ≈ 9,3 mV je Helligkeitsstufe). Der S3 ist nominell dreimal feiner, der Fehler von ±5 mV hebt das auf (✔K §6.3 (3)). Die frühere Folge
   „Auflösung steigt auf rund 3'190 Schritte" gilt **nicht mehr**.
2. **Die gespeicherte Kalibrierung bleibt gültig, auf ±1 Stufe.** `ldr_min_value` und `ldr_max_value` sind Zähler der 12-Bit-Skala (✔ `ldr.c:114-115` liest sie aus dem EEPROM) und bleiben gültig, **wenn** der S3 Zähler gleicher Bedeutung liefert (Abbildung oben). Eine Verschiebung um ±10 Zähler bei
   einer Spanne von rund 369 Zählern ändert die Stufe um etwa ±1 (✔K §6.3 (2), Rechnung). Ein Backup-Import von der alten Uhr bringt deshalb **brauchbare** Grenzen mit; **AK17b** prüft das bei fünf Beleuchtungen und verlangt danach die **einmalige Nachkalibrierung**, die Zählergleichheit herstellt. (Bis 17.10.2026 stand
   hier: „Die alte Kalibrierung ist ungültig; kein Backup-Import ohne anschliessende Neukalibrierung" — das war die Folge von 47 kΩ.) Die tatsächlich gespeicherten Grenzen des Nutzers sind **nicht bekannt** (OP-26).
3. **Der Gesamtfehler ist das Mass, nicht ein Prozentsatz.** ✔K: Unterhalb des Gesamtfehlers sind Messwerte „nicht aussagekräftig"; das Datenblatt nennt für den unteren Bereich **keinen eigenen Wert** und keine zulässige Quellimpedanz. Deshalb verlangt AK17 eine **Messreihe** mit der Toleranz „Gesamtfehler plus 1 %",
   und es zählt keine Stufe unter dem Fehler als bestanden — **das Dunkel zählt nicht, auf V2 ebenso**.
4. **Quellimpedanz und Kondensator.** Quellimpedanz im Dunkeln 1,0 kΩ (R_LDR ∥ `R22`); die empfohlene **Quellimpedanz steht in keiner der vier Unterlagen** (✔K §6.1, offen). **`C32` (100 nF) ist nicht in V2**; ✔K: 0,1 µF sind empfohlen **und Messbedingung** der Tabelle. Gerechnet ändert er den Gleichwert nicht (Leckstrom ≤ 50 nA ≙ ≤ 50 µV)
   und fügt einen Tiefpass mit τ ≈ 91–100 µs (Grenzfrequenz rund 1,6–1,75 kHz) hinzu; die Messung wird nicht langsamer (✔K §6.3 (4)). **Ob er unter „100 % kompatibel" fällt, entscheidet der Nutzer** (F15 Teil b, OP-03); AK17 läuft deshalb **mit und ohne**.
5. **Rauschen.** Das Datenblatt empfiehlt **Mehrfachabtastung und Mittelung** (✔K, Fussnote Tab. 5-5); **das Rauschen ist in keiner Unterlage beziffert**, die Werte gelten **mit abgeschaltetem WLAN**. Der Treiber bildet den **Mittelwert über N Einzelmessungen** (Startwert **N = 32**, Spec-Setzung, mindestens 16 nach ✔K §6.3;
   in Task 4.2 an der Streuung AK17c eingestellt; AK17 mittelt zum Nachweis über 64). Der Mittelwert **ist** die Aussage; Ausreisser werden nicht gesondert behandelt. Die Mehrfachabtastung läuft **einmal je 250 ms** im Display-Task und kostet nur Mikrosekunden (◌ geschätzt, Task 4.2 misst die Dauer und trägt sie ein).
6. **Skala bleibt die alte.** Der Treiber gibt `V_mV × 4'095 / 3'300` zurück — ein Zählwert **proportional zur Spannung** wie am STM32 (3,3-V-Bezug). So bleiben `ldr.c`, die PWA (zeigt `LDR_RAW_VALUE_NUM_VAR`, ✔ `ldr.c:56-60`) und die Normierung **im Rechenweg** unverändert. Die Umrechnung nutzt die **Kalibrierung ab Werk**;
   ohne sie wären bis ±9 % Fehler allein durch die Referenz möglich (✔K §6.1: wahre Referenz 1000 … 1200 mV je Chip). **Bereichsende:** Bei 0 dB endet der nutzbare Bereich bei 850 mV ≙ 1'055 Zählern; der Rohwert, den die PWA zeigt, ist darüber nicht mehr gültig. Das tritt erst bei einem LDR unter rund 2,9 kΩ ein; „hell" (10 kΩ, 300 mV) liegt weit
   darunter. **Das Protokoll von AK17c nennt es, statt es zu kaschieren.**
7. **Nicht ratiometrisch.** ✔K §6.2/§6.3: Die Knotenspannung ist proportional zur 3,3-V-Schiene (3,0 … 3,6 V = ±9 %), die ADC-Referenz aber nicht. **Schwankt die Schiene mit der WLAN-Last, schwankt der Messwert mit.** Für eine reine Helligkeitsstufung genügt das; **AK17c misst den Zählwert unter WLAN-Dauerlast und die
   Schiene dazu**, und F1/AK37 betreffen diese Schiene. **Die Schiene wird als 3300 mV angenommen**; sie zu messen (zweiter ADC1-Pin, Teiler 2 × 100 kΩ, 0,1 µF) wäre eine **Abweichung von V2** und braucht die Freigabe des Nutzers (✔K §6.3, „nicht freigegeben").
8. **Nur ADC1.** ✔K §6.1: ADC2 teilt sich den Wandler mit dem WLAN (Messung kann unterbrochen werden); GPIO1 ist ADC1 (H12). **GPIO3 ist Strapping und ausgeschlossen.**
9. **`H5` bleibt am Netz.** Der 2-polige Stecker `H5` (✔ `v2.xml`: `H5.2` am Netz `LDR`) bleibt bestückt und unverändert (E12). Ein dort angesteckter
   Verbraucher würde den Abgriff belasten; **das Netz mit `R22`-Pin 2 ist dasselbe**, also gilt AK17a auch für diesen Fall.

**Was die Messung in Task 4.2 liefert** (und AK17/AK17a/AK17b/AK17c verlangen): die **Messreihe** (≥ 8 Stufen vom Dunkel bis 850 mV, Soll/Ist/Abweichung in Zählwerten, Toleranz 5 mV + 1 %), Teilerspannung am
unveränderten LED-Board bei drei Beleuchtungen, die Grenzwerte der alten Uhr gegen den S3 bei fünf Beleuchtungen (±1 Stufe) und die einmalige Nachkalibrierung mit Messprotokoll,
Helligkeitsverlauf über fünf Stufen, Rauschen über 60 s, **und alles noch einmal unter WLAN-Last**. **Damit entscheidet die Messung F15 („reicht 1 kΩ bei 0 dB"), nicht diese Spec.** Fällt sie durch, **beantragt der Lead beim Nutzer** eine Abweichung — zweiter ADC1-Pin oder `R22` = 4,7 kΩ mit 6 dB (beide ✔K „nicht freigegeben");
nichts davon wird ohne Freigabe umgesetzt.

### 4.3 Die LED-Ausgabe (Detail)

- **Ablauf je Bild:** Pixelpuffer (RGBW, ✔ 32 Bit je LED, Reihenfolge Grün, Rot, Blau, Weiss) → RMT-Encoder → Sendung per DMA →
  Rückruf „fertig" → Pause ≥ 250 µs, bevor das nächste Bild startet (**am Treiber, nicht am Aufrufer — im Backend eingerechnet**, §3.6). Der Aufrufer
  (`display.c`) sieht **dieselbe** asynchrone Schnittstelle wie heute (die sechs Funktionen aus §3.6).
- **Fehler:** Ein Sendefehler **wird gezählt** und im Mitschnitt gemeldet (AK7: Zähler = 0). Kein Fehler wird verschluckt.
- **Pegelwandler:** Am Nachbau U3 (`HARDWARE.md`: SN74AHCT1G125, TTL-Eingang; Datenblatt `sn74ahct1g125.pdf` liegt vor, der Analyst hat es **nicht gelesen** — F18); am DevKit ein **externer**
  74AHCT-Baustein (F8, AK10). ✔K §5: Ein 3,3-V-Ausgang erfüllt VOH ≥ 2,64 V. Die Leitung `GPIO5` steht im Ruhezustand auf **Low**.
- **Versorgung:** `GPIO6` schaltet über R19/Q2/Q1. Die Software setzt `GPIO6` **als allererste Anweisung der Anwendung auf Low** und schaltet die Versorgung erst nach
  vollständigem Hochlauf ein (`HARDWARE.md`: `power_on()` mit 200 ms Einschwingzeit **im Aufrufer**). **Auf V2 hält `R20` (100 kΩ) das Gate von Q2 auf Masse** (✔ `v2.xml`). ✔K §4.2: **GPIO1–18 geben beim Einschalten 60 µs Low aus**
  und sind danach bis zum ersten Treiben hochohmig; **ein Low-Glitch schaltet den N-Kanal-Q2 nicht ein**, ein P-Kanal-Gate (Low = ein) an GPIO1–18 würde 60 µs lang eingeschaltet — **für das Gate von Q1 gilt das nicht**, weil Q2
  dazwischen liegt. **Was zwischen Reset und erstem Treiben am S3 geschieht, entscheidet die Platine (F2), nicht die Software**; AK9 misst es.
- **Reihenfolge der Spannungen (zu prüfen in Task 2.1):** Bleibt U3 stromlos, solange die LED-Kette aus ist
  (`HARDWARE.md`: `R13` in der Stückliste), liegt am Dateneingang der Kette nur `R31` (100 kΩ nach Masse).
- **Der Weg zur ersten LED bleibt unverändert** (E10, E13): `R11` 330 Ω auf dem Controller, `R4` 220 Ω auf dem LED-Board,
  zusammen 550 Ω (`HARDWARE.md`; dort „nicht gemessen, nur gerechnet"). AK5 misst die Flanken an `H7` Pin 4
  und, wo zugänglich, an `V1.DIN`.

### 4.4 USB, Flashen und Konsole — Muss (b)

**Zwei Dinge vorab, die die Datenblatt-Analyse belegt (✔K §7, §9) und die frühere Fassung dieses Abschnitts anders annahm:** Die Konsolen über UART0 und über USB sind **getrennte Kanäle** — die USB-Zweitkonsole ist in ESP-IDF
**nur Ausgabe, nicht blockierend, ohne Eingabe, und sie erscheint nur, wenn UART0 nicht verbunden ist**. Die Abnahme verlangt deshalb **keine** gleichzeitige Ausgabe und **keine** Eingabe über beide Kanäle (AK3a).

| Punkt | Entwurf | Belegt durch |
|---|---|---|
| Anschluss | **USB-Serial/JTAG** am S3 (GPIO19 = D−, GPIO20 = D+, ✔K §7), 16-poliger Stecker, ESD-Schutz (H3, H4). **Aussenmasse des Steckers: F16** | AK3, AK3a, AK3b |
| **Kein USB-Geräte-Stack** | Das Abbild bindet **keinen** USB-Geräte-Stack (zum Beispiel TinyUSB) ein. ✔K §7: Konfiguriert die Anwendung GPIO19/20 um (USB-PHY abschalten, andere USB-Funktion, UART/LEDC auf diese Pins), **verschwindet das USB-Gerät, und das automatische Download-Verfahren entfällt**. Beim Arduino-Paket heisst die Einstellung „USB CDC On Boot" — **sie bleibt aus**; welche Einstellung im gewählten Framework gilt, steht in `multiboard-quelle` (F12) | AK3 (Rückfall 2), Task 1.1 |
| Konsole, Konfiguration | **UART0** (GPIO43/44) als Hauptkonsole, **USB-Serial/JTAG** als Nebenausgabe (✔K §9: `CONFIG_ESP_CONSOLE_UART_DEFAULT` und `CONFIG_ESP_CONSOLE_SECONDARY_USB_SERIAL_JTAG`; Nebenausgabe **nur Ausgabe**, nicht blockierend, ohne REPL). **Eingabe wird nicht verlangt:** Eine Konsoleneingabe ist im STM-Code nicht gefunden (Grep auf `log_getc`, `log_poll`, `log_get_char`, `log_read`: 0 Treffer — **nur diese Namen** geprüft), und alle Befehle laufen heute über die Brücke. Echte Gleichzeitigkeit **mit Eingabe** auf beiden Kanälen liefert diese Konfiguration nicht (✔K §9); sie wäre eine neue Funktion (E8). Im Board-Merkmal heisst das `BOARD_LOG_SINKS` (`multiboard-quelle` §5.3) | AK3a |
| Am DevKit und am Nachbau | **Am DevKit** läuft die Konsole über die USB-UART-Brücke (CP2102N) an UART0, **am Nachbau** über das native USB **und, wenn `H6` UART0 trägt, über `H6`** (siehe nächste Zeile): **dieselbe Firmware bedient alle Wege** (✔K §9). Die ROM-Meldungen beim Start erscheinen ohnehin auf beiden (✔K §9) und sind **kein Nachweis** der Anwendungskonsole | AK3a |
| **Konsole am Stecker `H6` — Polarität (F19)** | Auf V2 ist `H6` die Konsole des STM (Netze `TX_UART`, `RX_UART`, **Netznamen aus Sicht des Steckers**, also des Adapters; `SW4` schaltet sie, ✔ Skill `kicad-schema`). **Nach OP-35 hängt dort `TX_UART` am STM-Empfang (PA10) auf Pin 2 und `RX_UART` an der STM-Sendung (PA9) auf Pin 3.** V3 ordnet nach dem **Namen** zu (`TXD0` am Netz `TX_UART`, also Pin 2); **das wäre gegenüber V2 vertauscht** und liesse einen für V2 verdrahteten Adapter **Ausgang auf Ausgang** treffen. **Anforderung:** Am Nachbau liegt an Pin 2 der **Controller-Empfang** (`RXD0`, GPIO44), an Pin 3 die **Controller-Sendung** (`TXD0`, GPIO43) (H13). **Das ist nicht geprüft und die Belegung des vorhandenen Adapters nicht belegt** — beides ist F19. Bis zur Antwort wird die Konsole nur über USB angeschlossen | AK38 (Papier und Gerät), AK3a |
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
das DevKit hat seinen eigenen Regler; sie trägt nur Strom und Verhalten des Moduls ein. Der Brownout-Reset bekommt ein eigenes Wort im Log (§3.4), damit er nicht als Watchdog-Reset erscheint. **Die Schiene geht beim LDR voll in den Zählwert ein** (§4.2).

### 4.7 IR-Empfänger und DCF77 — ohne Stecker (dritter Stand)

**Bis 17.10.2026 stand in dieser Spec:** IR und DCF77 „bleiben". **Die V2-Netzliste kennt dafür auf dem Controller keinen Stecker** (✔ Skill `kicad-schema`; `BEFUNDE.md` L491/E48; OP-36): nur die Pull-ups `R8`/`R9` (100 kΩ) und den Controller-Pin. **Die Anforderung** (`requirements.md`, Abschnitt „IR-Empfänger und DCF77 — was 1:1 hier heisst"):

| Punkt | Entwurf | Prüfung |
|---|---|---|
| Controller-Pin | GPIO7 (IR), GPIO15 (DCF77), je **mit Pull-up wie V2** (Schiene: F3) | AK39 (1), Gutachten Task 0.1c |
| Firmware-Funktion | `irmp.c` (Algorithmus unverändert, Pin-Zugriff hinter `WC_IRMP_TARGET_EXTERNAL`), `remote-ir.c`, `dcf77.c` (Pin aus `board-pins.h`) bleiben | AK19, AK20 |
| Stecker | **keiner**, und die Spec fordert keinen (E8, E12) | AK39 (1) |
| Anschluss der Empfänger | **offen, F20**; nicht an `H7` | Nutzer |
| Testaufbau | DevKit: Steckbrett an GPIO7/GPIO15 mit Pull-up; Nachbau: Draht am Pin | AK19, AK20, AK39 (2) |
| Pegel | 3,3-V-verträglich, kein 5-V-Pegel an einem S3-Pin (H7, F3) | AK39 (2) |

Die Software ist davon **nicht betroffen**: Der Pin kommt aus der Board-Beschreibung, die Funktion aus dem unveränderten Bestandscode. Betroffen ist allein, **wie ein Nutzer die Empfänger an der Platine befestigt** — das ist Hardware und offen.

## 5. Die Netz-Seite (ESP8266 → ESP32)

Die Netz-Seite ist **voraussichtlich der grössere Teil der Arbeit**, auch wenn die Hardware den Auftrag dominiert: Der ESP-Code
umfasst ✔ 26'867 Zeilen (`*.cpp` und `*.ino`, **ohne** Kopfdateien; `multiboard-quelle` §1.5 nennt mit Kopfdateien eine höhere Zahl — keine Abweichung in der Sache), davon `http.cpp` allein 17'651 — gegen 627 Nahtstellen-Treffer in
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
muss danach unverändert bauen und sich unverändert verhalten** (AK33; Nachweis über den Hash des ESP8266-Abbilds, `multiboard-quelle` AK13, **plus** Gerätetest, weil Task 5.3 Funktionscode berührt). `esp-developer` ändert **nur** in `ESP8266/ESP-uclock/*.cpp`, `*.h`,
`*.ino` (Besitz heute), und verändert **nichts** unter `data/app/**`. Die Netz-Komponente nimmt den Sketch-Ordner **dateiweise** (keine zweite Dateiliste, `multiboard-quelle` §8.2). **Wie `multiboard-quelle` die Boards trennt, geht vor;** die `#ifdef`-Zweige hier sind die Annahme dieser Spec (`multiboard-quelle` führt sie ebenso, dort §1.5).

## 6. Der entfallende STM32-Flash-Weg (E6) — und die Reste

| Heute ✔ | Am S3 (Empfehlung F10) |
|---|---|
| `stm32flash.cpp` (1'438 Z.) | **nicht gebaut** |
| `/api/remote_stm32_flash` | bleibt als Endpunkt, antwortet mit einem **benannten Fehler** („nicht unterstützt") statt `{"ok":true}`; **nie** stilles Nichtstun (DIR-010-Lehre: der alte Endpunkt tat bei fehlendem `filename` „stillschweigend nichts") |
| `stm32_default` in `/api/update_status` | Feld bleibt, Wert **leer** und ein zusätzliches Merkmal, das die PWA unterscheiden kann (**Folgespec**, nicht in dieser Änderung) |
| `/api/maintenance_reset_stm32` | **Neustart des gesamten Chips** (`esp_restart()`), Antwort **vorher** senden. Gründe: (1) ein „Reset nur des Display-Tasks" müsste Zeitgeber, Brückenpuffer, LED-Ausgabe, I²C-Zustand und alle Zustandsflags **zurücksetzen** — das ist eine eigene Wiederanlauf-Funktion, die es im STM nie gab (der Reset war Hardware), (2) ein nur halber Neustart hinterliesse **genau** die Art von Zustandsabweichung, die DIR-010 beschreibt, (3) der Neustart des ganzen Chips ist nachweisbar (AK3, AK25) |
| `esp8266_reset/_powerdown/_powerup/_flash` in `esp8266.c` (Steuerpins zum zweiten Chip, Boot-Pfad; ✔ `multiboard-quelle` §3.4.1) | **Seam-Guard `WC_PEER_EXTERNAL`** schneidet sie ab (`multiboard-quelle` Task 3.7); **wie die Funktionen auf dem S3 antworten, ist F10** (dort Q11). `esp8266.c` bleibt sonst unverändert (W10) |
| `tools/flash-stm.sh`, DIR-010 | **gelten für die alten Ziele unverändert weiter.** Für den S3 gilt ein eigenes Skript (Task 6.2): prüft Abbild und Geräteart, flasht über USB, weist die gemeldete Version nach, **schlägt fehl, wenn das Gerät anschliessend nicht antwortet** (der Fall, der bei DIR-010 den Anlass gab: ein Vorgang, der „erfolgreich" aussieht, obwohl nichts geschah) |
| `tools/smoke-device.sh` | bleibt für beide Geräte; **die Prüfung der Update-Quelle (DIR-009) bleibt Pflicht** — die eingebauten Vorgaben des ESP zeigten auf den Server des Ursprungsprojekts; der Port **darf diese Vorgabe nicht übernehmen**, sondern leer lassen. **Der Chip-Code des S3 im Konfigurationswort ist offen** (`multiboard-quelle` Q4); bis dahin darf `smoke-device.sh` ihn nicht als F411 deuten |

## 7. Betroffene Module

| Datei / Verzeichnis | Änderung | Zuständiger Agent |
|---|---|---|
| `specs/esp32s3-port/**` | Spec, später Ergebnisse von 0.1a–c/0.4/0.5 eintragen | `spec-writer` |
| S3-Baum `esp32s3/` (Hardwareschicht, Backends, Attrappen) und `boards/wordclock-esp32s3-*/` (Board-Beschreibung) — **Namen nach `multiboard-quelle` `design.md` §7, dort noch nicht freigegeben** | Hardwareschicht (`sk6812-backend.inc`, `i2c`, `adc`, `onewire`, `delay`, `power` …), Tasks, Brücke, Gerüst, Testprogramme, Systemkonfiguration, Partitionstabelle, Linker-Fragment, Attrappen | **S3-Rolle (Vorschlag `s3-developer`; legt allein der Lead an, `multiboard-quelle` Task 0.3)** |
| `src/main.c`, `main.h`, `sk6812.c`, `irmp/**`, `esp8266.c`, `eeprom.c`, `dcf77.c` | **Seam-Guards und Pin-Zweige, hash-gleich** (§3.6, E1–E7). **Nicht `src/display/display.c`** (E11, AK35). Diese Spec **ändert sie nicht selbst** — `multiboard-quelle` Tasks 3.1–3.8 tun es (bis 17.10.2026 stand hier: „Task 5.1") | `stm-developer` |
| `boards/modules.txt`, `boards/platform-services.txt`, `boards/baseline/**`, `tools/checks/boards-*` | Klassifikation, Dienstliste, Hash-Baseline, Prüfungen C1–C9 | Lead (`multiboard-quelle`) |
| `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` | `#ifdef ESP32`-Zweige; `StmLink`-Nutzung; STM-Flash-Endpunkte | `esp-developer` (Tasks 5.3a–c) |
| `Makefile`, `CMakeLists.txt`, `cmake/**`, `src/main.h`, `ESP8266/ESP-uclock/version.h` | neues Target, Versionen | `release-engineer` |
| `tools/**`, `.claude/**` | Hooks (`file-ownership.py`, `no-build.py` je nach Framework), `flash-s3.sh`, `smoke-device.sh`, `watch-log.sh`, `guardrails.sh` (S7), `device.conf` für zweites Gerät, Agent der S3-Rolle | **Lead** |
| `HARDWARE.md`, `CHANGELOG.md`, `BEFUNDE.md`, `ERWEITERUNGEN.md`, `knowledge/directives.md`, `CLAUDE.md` | Nachführen nach Abschluss | `doc-writer` |
| `ESP8266/ESP-uclock/data/app/**` | **keine Änderung** | — |
| `hardware/komponenten/**`, `hardware/architektur/**`, `hardware/esp32s3/**`, `hardware/review/**` | **Hardware-Spezifikation, Schaltplan, Layout, Gutachten — ausserhalb dieser Tasks, ausser den Zulieferungen 0.1a–c.** Diese Spec **verweist nur darauf** | `hw-datasheet-analyst`, `hw-architect`, `hw-designer`, `hw-reviewer` |
| `specs/multiboard-quelle/**` | Architektur (Framework, Verzeichnisse, Naht als Schnittstelle, Bau-Matrix). **Nicht diese Spec** | `sw-architect` |
| KiCad-Projekt der Controller-Platine, LED-Board | Controller: Nachbau im **unveränderten Format** (Nutzer, Hardware-Rollen). **LED-Board und LDR-Pfad: nicht berührt** | Nutzer |

## 8. Prüfung gegen die Architektur-Checkliste

Pflicht (`knowledge/architecture-checklist.md`). Jeder Punkt wird beantwortet, nicht abgehakt.

**Proper architecture**

> **Antwort.** *PWA parallel zu Legacy:* Beide Oberflächen werden von derselben HTTP-Schicht ausgeliefert; **der Port ändert
> `data/app/**` nicht** (AK21, `git diff` leer) und die Legacy-Oberfläche bleibt Stabilitäts-Referenz — sie läuft am S3 mit
> demselben Code wie am ESP8266, **sodass ein Fehler der PWA gegen Legacy am S3 abgegrenzt werden kann**. *Wetter:* `/api/weather_get_now` und
> `/api/weather_get_forecast` bleiben; der Wetterabruf nutzt `WiFiClient` gegen eine externe Quelle, **dessen Leseschleife am S3
> neu gemessen wird** (§5, Checkliste 4) — **kein Rückbau auf `/weather?action=`**. *Restore-Bedingung:* alle **vier** Teilbedingungen
> von `pending_ticker_restore` bleiben im Quelltext **unverändert** (`main.c:4910-4914`, AK24); die Seam-Guards in `main.c` dürfen diese Zeilen nicht
> berühren, und die Prüfung C6 von `multiboard-quelle` vergleicht den Text nach jedem Task. Dass nur noch **ein** Task die Zustandsmaschine schreibt (AK23), macht keine der vier Bedingungen redundant: Sie
> beschreiben Ablaufzustände (Ticker läuft, Icon aktiv, Stop-Zeit offen, Overlay aktiv), nicht Wettläufe zwischen Tasks. *`.gz`-only:*
> bleibt, Grösse > 0 bleibt, `install-app.sh --check` prüft das LittleFS des S3 **erstmals** (DIR-017, das Dateisystem des Neugeräts
> ist leer). *Richtige Schicht:* Plattformeigenheiten (Zeitgeber, Watchdog, ADC-Skala, USB, **LED-Ausgabe**) werden in der **Hardwareschicht** bzw. in **Backends hinter Seam-Guards** gelöst,
> **nicht in `display.c`** — die LED-Ausgabe über die **vorhandene** Treiber-API und den gemeinsamen Kern (§3.6), keine zweite Zustandsmaschine (E11). Ein Fehler der
> Hardwareschicht wird **nicht** in der PWA umgangen. **Eine Stelle verletzt die Schicht bewusst:** die **Seam-Guards in Bestandsdateien** (`main.c`, `sk6812.c`, `irmp`, `esp8266.c`, §3.6) — dort
> liegen Plattform und Anwendung heute in einer Datei, und sie zu trennen ist der einzige Weg zu einem gemeinsamen Quelltext. Das ist eine
> Änderung am produktiven STM-Zweig; **sie ist hash-gleich nachzuweisen** (`multiboard-quelle` AK13), und **nur** eine Änderung der Klasse F braucht den STM-Flash und den `pwa-tester` an der alten Uhr (AK33, DIR-012). *Der LDR-Teiler:* Die
> Anpassung an die ADC-Eigenschaften des S3 geschieht **allein in der Hardwareschicht** (Skalenumrechnung, Mittelung, 0 dB); `R22` bleibt 1 kΩ (E13), **nicht** in `ldr.c`, nicht in der PWA; LED-Board und LDR-Pfad bleiben unberührt (E10, E13). *Abstimmung:* Die Architektur über alle Boards (Framework, Dateiauswahl, Board-Beschreibung) liegt bei
> `specs/multiboard-quelle/`; diese Spec widerspricht ihr nicht, wo sie nicht freigegeben ist, sondern benennt die Punkte (F12, F21, §12).

**Scalable systems**

> **Antwort.** *Kommandos je Aktion:* unverändert — Protokoll und Anwendungslogik bleiben, also erzeugt jede Aktion **dieselbe Zahl
> an Brückenkommandos** wie heute. *Byte je Minute auf der Brücke:* **weniger**, weil die Debugzeilen der HTTP-Anfragen
> (✔ rund 100 Byte je Anfrage) die Konsole statt der Brücke erreichen (§3.3). *Rückstau:* Am UART begrenzte die Leitung (11,5 kB/s),
> am S3 nur noch der Strompuffer (1'024 Byte je Richtung, Startwert) — **ein Sender ohne Quittung füllt ihn schneller, als der Leser
> abholt**. Das ist die neue Grenze und **AK22 provoziert sie absichtlich**; verworfene Zeichen werden **gezählt** (`d=`), nichts
> verschwindet still. *Unter 20 s:* Der Task-Watchdog bleibt bei 20 s (§3.4). Jeder **neu eingeführte** Wartepfad ist begrenzt: Brücke
> `BRIDGE_TX_BLOCK_MS`, I²C 5 ms je Flag-Äquivalent, RMT mit Frist **im Backend** (Q12), Konsole höchstens eine Pause von rund 50 ms (✔K). **Die bestehenden langen Pfade** (✔ `var_send_buf` bis 3 s je Kommando,
> `eeprom_write` je Seite rund 15 ms, Overlay-Bereich im schlimmsten Fall rund 0,9 s am F411) **bleiben** — am S3 sind sie **nicht belegt**,
> weil der I²C-Treiber ein anderer ist (AK12 misst). *Blockade des Hauptloops:* LED-Ausgabe per DMA asynchron, ADC einmal je 250 ms
> mit 32 Einzelmessungen (Mikrosekunden, ◌ geschätzt); **neu** ist, dass die Blockade des `display_task` das WLAN **nicht** mehr anhält
> (Kern 0) — und umgekehrt. Dabei gilt ausdrücklich: **das ist kein Nachweis, dass der F411-Hänger entfällt** (AK26). *PWA-Abfragen:* unverändert,
> aber jeder HTTP-Request kostet **keinen** Debugtext mehr auf der Brücke. *Versorgung:* Das Modul zieht **355 mA** Spitze beim Senden (✔K); die 3,3-V-Schiene hat bei 0,6 A Last **mindestens 250 mA** Stromreserve, aber
> **knappe Spannungsreserve bei schwacher 5-V-Schiene** (✔K Rechnung) — AK37 misst sie. *Hartkodierte Grenzen:* Brückenpuffer, Stapelgrössen der Tasks,
> Partitionsgrössen, Sendepuffer der LED-Ausgabe, ADC-Dämpfung (0 dB) und Mittelungszahl sind **benannte Konstanten** (Board-Merkmale bzw. Konstanten der Hardwareschicht); `R22` ist Hardware und bleibt 1 kΩ (E13); die LED-Kette bis 234 LEDs ist nachgewiesen (AK8), nicht
> mehr.

**Secure by design**

> **Antwort.** *`innerHTML`/`escapeHtml`:* kein Eingriff in die PWA. *Update-Server nicht vertrauenswürdig:* **neu am S3** ist die
> Frage, ob ein falsches Abbild (ESP8266-`.bin`, abgeschnittene Datei) das Gerät unbrauchbar macht. ◌ Der IDF prüft Abbild-Kopf, Chip-Kennung und
> Prüfsumme **vor** dem Umschalten der Boot-Partition; der Entwurf **verlässt sich nicht darauf**, sondern prüft in AK27 mit einem
> **tatsächlich fremden** Abbild. *Eingebaute Update-Vorgabe:* leer, **nicht** der Server des Ursprungsprojekts (§6, DIR-009; `board.h` trägt **keine** Update-Adresse, `multiboard-quelle` §5.3). *Keine Credentials in
> Logausgaben:* **die Konsole ist am S3 ein echter Ausgang** (UART0 an `H6`, USB), nicht mehr nur die STM-Debug-UART; die ROM-Meldungen beim Start erscheinen zudem auf beiden Wegen (✔K §9) und tragen
> keine Anwendungsdaten. Alle Regeln aus `specs/bruecke/` (P6: Timeout-Meldung ohne Variablenwerte) gelten **unverändert**, und **jede neue Logzeile des Ports** darf weder WLAN-Schlüssel
> noch Wetter-Schlüssel noch Update-Pfad enthalten — Task 5.7 prüft das mit `grep` über die neuen Dateien. *Neue Angriffsfläche:* USB-Serial/JTAG
> erlaubt jedem mit Zugriff auf die Buchse **Auslesen und Beschreiben des Flash** (◌ ohne eFuse-Sperre), also auch das Lesen der ESP-eigenen Variablen
> mit dem WLAN-Schlüssel. Die Uhr hängt an der Wand, die Buchse ist erreichbar. **Das ist bekannt, nicht behoben:** Secure Boot und Flash-Verschlüsselung
> brennen eFuses und sind unumkehrbar — **dafür braucht es eine eigene Entscheidung des Nutzers** (Nicht Teil dieser Änderung). *Neue Endpunkte, die
> Konfiguration schreiben:* **keine**; die Endpunkte der Gefahrenliste (R5) und der zurückgestellte Schutz (C6 u. a.) bleiben, wie sie sind.
> *Hardwareseitig:* Es gibt **keinen** dokumentierten 5-V-Toleranzwert und **keinen Grenzwert je Pin** (✔K §5); jeder S3-GPIO gilt als **höchstens 3,6 V**, ein 5-V-Pegel an einem S3-Eingang ist ein Bauteilschaden (F3); H7 verlangt
> 3,3-V-verträgliche Eingänge. *Konsole `H6`:* Eine **Ausgang-auf-Ausgang**-Verbindung durch einen für V2 verdrahteten Adapter wäre ein Hardware-Fehler (F19, AK38); bis zur Antwort kein Adapter an `H6`.

**Stable & reliable**

> **Antwort.** *Jeder Fehler wird ausgewertet:* RMT-Sendefehler gezählt (AK7), I²C unterscheidet NACK von Störung (AK14), Brückenverlust gezählt (AK22),
> ADC-Fehler **nicht** als „0" gemeldet, sondern als Fehler, damit `ldr.c` keinen Nullwert als „dunkel" liest (Task 4.2). *Leere Fänge/stille Verwerfungen:*
> ein voller Brückenpuffer hat einen `else`-Zweig mit Zähler (§3.3); es gibt kein „ganze Zeile verworfen" ohne Marke auf der nächsten durchkommenden Zeile.
> *Werte zurechtgebogen:* Der ADC-Wert wird auf die **alte Skala umgerechnet** (§4.2) — das ist eine Umrechnung mit **Beleg** (AK17 nennt Toleranz und Datenblattfehler), keine stille
> Klemmung; `ldr.c` klemmt weiter nur eine Kopie (✔ `ldr.c:56-61`). Dass das **Dunkel unter dem Gesamtfehler** liegt, wird **benannt** (AK17c) statt verdeckt; auf V2 ist es ebenso. *Zustand nach Abbruch:* Ein Reset (Watchdog, Absturz, Unterspannung, Strom weg) startet mit
> **LED-Versorgung aus**, Ruhepegeln auf GPIO5/6 und leerer Brücke; was **zwischen** Reset und erstem Treiben auf GPIO6 geschieht, entscheidet die
> Platine (F2) und wird am Oszilloskop **gemessen** (AK9). Der Reset-Grund wird beim Start gemeldet, **Brownout mit eigenem Wort** (§3.4, AK25), damit ein Neustart nicht unbemerkt bleibt (DIR-013).
> *Rückweg nach einem Fehler:* Konfiguriert eine fehlerhafte Anwendung GPIO19/20 um, ist die Uhr per USB nicht mehr flashbar — **der manuelle Weg über BOOT- und EN-Taster ist der Rückweg und wird nachgewiesen (AK3, Rückfall 2)**;
> ohne die Taster wäre ein solcher Fehler ein **Totalverlust** des Zugangs.
> *Flags:* Der Zeitgeber-Interrupt **setzt**, `display_task` **löscht**, beide auf Kern 1 (§3.5); jedes Flag hat damit einen Verbraucher. `var_send_busy` und
> `got_ack` sind Anwendungslogik und bleiben unberührt. *Neue Absturzarten:* Ein Zugriff auf Flash-Rodata aus einem Interrupt bei ausgeschaltetem Cache führt zum Absturz —
> **dagegen** stehen das Linker-Fragment und AK16 (100 Schreibvorgänge). *`WiFiClient`-Leseschleife:* **nicht übernommen, neu gemessen** (§5, Task 5.3b); die Attrappe aus t12 wird
> für den S3 **nicht** wiederverwendet, ohne ihre Semantik gegen den ESP32-Client zu prüfen. *Konsole ohne Gegenstelle:* darf weder `display_task` noch `net_task` blockieren (AK4); die einmalige Pause von 50 ms ist bekannt (✔K).
> *Wiederverwendung:* Eine **übernommene** Logik ist nur so zuverlässig wie die Naht; deshalb prüft AK35 mit **Zählung** der betrachteten Dateien, die Attrappen-Übersetzung (`multiboard-quelle` C2) zeigt, dass die „hardwarefreien" Module ohne STM-Kopfdateien übersetzen,
> und die `_Static_assert`-Prüfung (dort C7) nagelt Breiten und Layouts auf dem S3 fest, damit er nicht übersetzt und **anders rechnet** (§3.6). *IR und DCF77 ohne Stecker:* Pin mit Pull-up bleibt, ein schwebender Eingang wird durch AK39 (1) ausgeschlossen. *Prüfungen, die zuletzt versagt haben müssen:* DIR-014 und AK34.

## 9. Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| Der S3 ersetzt nur den ESP-12F, der STM32 bleibt | Ist **Variante A**, vom Nutzer nicht gewählt (E1–E13). Nicht neu verhandelt |
| Ein einziger Task mit kooperativer Schleife (STM-Hauptschleife und `loop()` nacheinander) | Der Nutzer verlangt zwei Tasks (E5); ausserdem würde ein blockierender Pfad (✔ `eeprom_write` je Seite 15 ms, `var_send_buf` bis 3 s) das WLAN anhalten — der Zustand, den zwei Chips heute vermeiden |
| Direkte Funktionsaufrufe statt Brücke | Ändert das Protokoll, die PWA und jede Stelle, die heute auf eine Antwortzeile wartet. **Verworfen ausdrücklich vom Nutzer (E5)** |
| **`esp8266.c` für den S3 entfernen** („das Modul entfällt komplett", Empfehlung aus dem Inventar des `firmware-analyst`) | Widerspricht E5 und ändert **sechs Logikmodule**, die `esp8266_send_cmd`/`esp8266_get_message` direkt rufen (W10, `multiboard-quelle` Problem 6). Ersetzt wird nur die unterste Schicht |
| Zwei Hardware-UARTs des S3 intern verbinden (Pin-Matrix-Schleife) statt Strompuffer | Erhielte die Baudrate-Drosselung des heutigen UART, kostet aber zwei Peripherien und Interrupts für **nichts**, was der Strompuffer nicht ebenfalls kann; **vom Nutzer als Warteschlange entschieden (E5)** |
| **Eine neue Anzeige-Zustandsmaschine oder ein neues Anzeigemodul für den S3** | **Vom Nutzer ausgeschlossen (E11):** `display.c` (6'738 Zeilen) ist die bewährte Logik; eine zweite Kopie driftet und verdoppelt jeden künftigen Fehler. Die Naht ist die Treiber-API |
| **Ein zweiter LED-Treiber mit eigenem Kern für den S3 statt Backend** (die frühere Fassung dieser Spec; dort „`#if`-Zweige in `sk6812.c` statt eines Nachbaus" als **verworfen** geführt) | **Umgekehrt entschieden (Stand 17.10.2026, aus `multiboard-quelle` §3.6, §11):** Ein zweiter Treiber kopiert den portablen Kern (Doppelpuffer, `set_led`, `refresh`-Rahmen, Zähler) und lässt ihn driften; der Guard in `sk6812.c` ist hash-gleich nachzuweisen und hält den Kern **einmal**. Preis: `sk6812.c` ist Bestandsdatei mit Guard, die Stufe-2-Zusage „keine Zeile `src/**`" entfällt (F21). **Bis zur Freigabe von `multiboard-quelle` gilt dies als Empfehlung, nicht als Entscheid** |
| **`DISPLAY_OWNER_ASSERT` an jeder Schreibstelle** (erster Entwurf von AK23) | Verlangt Zeilen in `display.c`; **verworfen** zugunsten von Symbolprüfung und Task-Prüfung an den Grenzen (§3.2) |
| **Konsoleneingabe über beide Kanäle als Abnahme** (frühere Fassung von AK3a) | ✔K §9: Die USB-Zweitkonsole ist nur Ausgabe, und sie erscheint nur, wenn UART0 nicht verbunden ist. Echte Gleichzeitigkeit mit Eingabe wäre eine **neue Funktion** (E8); verworfen |
| **Das Framework in dieser Spec festlegen** | Aufgabe von `sw-architect` (`specs/multiboard-quelle/`), „mit Preis"; diese Spec liefert nur Eingaben (§1.1) |
| **Pins in dieser Spec und zusätzlich im Code von Hand führen** (`esp32s3/pins.h`) | Zwei Listen driften; **eine Quelle je Board: `boards/<id>/board.h`** (W2, §2) |
| **Eine von Hand gepflegte Dateiliste für die STM-Komponente** (frühere `stm_core`-Liste) | Eine dritte Liste neben beiden `.ebp` und `CMakeLists.txt` driftet; die Liste kommt aus `boards/modules.txt` (W3) |
| **Ersatz-Kopfdatei oder „die Einbindungen fallen von selbst weg" statt Attrappen** (frühere Annahme in §3.6) | Gemessen widerlegt (W1): ohne Attrappen übersetzt kein Logikmodul. Attrappen und erzwungene Einbindungen sind die Lösung ohne Eingriff in Logikdateien |
| LED-Ausgabe über SPI-MOSI, I²S oder LCD-Peripherie statt RMT | ◌ funktioniert, aber der RMT-Encoder bildet SK6812-Bits direkt ab und braucht keinen Puffer-Aufblasfaktor; Entscheid kann in Task 2.1 **mit Messung** revidiert werden, wenn RMT-DMA nicht trägt (Errata RMT-176 ist dort zu lesen) |
| `esp_timer` statt GPTimer für den 20-kHz-Takt | ◌ `esp_timer` ruft aus einem Task ab (Verzögerung, nicht 50 µs-genau); IRMP braucht einen **Interrupt** |
| Interne Pull-ups für I²C und 1-Wire | ✔K typ. 45 kΩ, ◌ zu schwach (F7); externe Widerstände sind Pflicht |
| **`R22` auf ca. 47 kΩ (oder 10 kΩ) ändern**, damit der ADC das Dunkel auflöst (frühere Fassung von E9(a)) | **Vom Nutzer am 17.10.2026 ausgeschlossen (E13):** LDR-Lesung 100 % pin- und funktionskompatibel, dasselbe LED-Board. Auf V2 liegt das Dunkel ebenfalls unter dem Wandlerfehler. Eine Abweichung nur nach Freigabe (AK17) |
| Softwareglättung gegen das Unterlaufen des ADC unten **statt** `R22` zu ändern (frühere Fassung: „vom Nutzer entschieden (E9)") | Entfällt als Frage: `R22` bleibt, und `ldr.c` klammert das Dunkel wie auf V2. Die Mittelung im Treiber bleibt (§4.2) |
| IR und DCF77 mit einem **neuen Stecker** versehen | Mehr als V2 (E8), im unveränderten Format zu lösen (E12); braucht die Entscheidung des Nutzers (F20) |
| Secure Boot und Flash-Verschlüsselung gleich mitnehmen | unumkehrbar, eigene Entscheidung nötig; nicht Teil dieser Änderung |
| Das LED-Board für den S3 anpassen (zum Beispiel den LDR-Teiler dorthin verlegen) | Vom Nutzer ausgeschlossen (E10): Änderungen betreffen nur die Controller-Platine |
| Ein anderes Leiterplattenformat (zum Beispiel kleiner, weil der STM32 entfällt) | Vom Nutzer ausgeschlossen (E12): Umriss, Löcher, Stecker, Taster bleiben |
| Ein Fork von `main.c` für den S3 | Drift (DIR-006-Mechanik); verworfen zugunsten der Seam-Guards (§1) |

## 10. Versionsfolgen

Nur der `release-engineer` führt das aus (R4). **Bis 17.10.2026 stand hier die Empfehlung F13** („beide Symbole bleiben und werden gemeinsam geführt"); **sie ist zurückgenommen, es gilt keine Annahme** (F13, `multiboard-quelle` W7/Q2/Q9):

- [ ] STM `src/main.h` anheben — **nur wenn der Hash der Bestandsabbilder abweicht oder Klasse F im Spiel ist**; bei Hash-Gleichheit ist die Versionspflicht offen (`multiboard-quelle` Q9, bis zum Entscheid wird gebumpt). Frühere Zeile: „Task 5.1v"
- [ ] ESP `version.h` anheben — **Task 5.3v** (ESP8266-Zweig geändert); **für das S3-Abbild nicht mitzuführen** (`multiboard-quelle` Q2, Empfehlung: drittes Symbol im selben Format)
- [ ] App `APP_VERSION` — **bleibt** (PWA unverändert, AK21)
- [ ] `CACHE_NAME` in `sw.js` — **bleibt**
- [ ] Versionsstelle und Tag-Schema des S3-Abbilds — **offen (F13)**; Tag `release/<stm>-<esp>-<app>` passt nicht von selbst
- [ ] Der Makefile liest die Versionen weiter per `grep` aus (✔ CLAUDE.md); **das Zeilenformat darf sich nicht ändern**, sonst bricht der Release-Build still

## 11. Ergebnisse der Klärungs-Tasks — hier eintragen

Wird vom `spec-writer` in Task 0.2 aus den Berichten von 0.1a–c, 0.4 und 0.5 gefüllt. **Bis dahin stehen hier keine
Ergebnisse und keine Vermutungen.** Was die vorhandenen Komponentenblätter schon beantworten, steht in `requirements.md` bei den Fragen F1–F6, F15 (mit ✔K).

```
T0.1a Komponentenblätter noch ausstehend: USB-C 6P/16P Aussenmasse (F16), 74AHCT1G125
      mit 3,3-V-Eingang (F18), SKC6812-Toleranz (F9), RMT-Errata RMT-176 (Einzelbeschreibung):  offen
T0.1b Pinzuordnung entschieden (in maschinenlesbarer Form, multiboard-quelle Task 0.5),
      F3 (Schiene der R8/R9-Pull-ups), F7 (Pull-ups), F15 (reicht R22 = 1 kOhm bei 0 dB, C32),
      F17 (BOOT-Taster-Ort), GPIO21-Frage (F2), F19 (Konsole H6, Polaritaet), F20 (IR/DCF77):    offen
T0.1c Gutachten Pin-Tabelle gegen Schaltplan (AK1), Zahl der betrachteten Netze;
      zusaetzlich H6 Pin 2/3 gegen V2 (AK38) und IR/DCF77-Pull-ups ohne Stecker (AK39):          offen
T0.4  Schnittstellenliste der Kopfdateien: ersetzt durch multiboard-quelle design 3.4;
      Bestaetigung der Klassen (boards/modules.txt) zusammen mit multiboard-quelle Task 0.4:     offen
T0.4  Zustandsschreiber (b), Quittungspruefung je Kommando im Ist-Stand (c):                      offen
T0.4  Funktionsinventar (E8, AK30):                                                              offen
T0.4  F14 (F_INTERRUPTS per -D, Q8), dcf77_tick im Interrupt (multiboard Task 3.9):              offen
T0.4  Zahl der watchdog_reload ()-Aufrufstellen (Widerspruch 7 vs. S7-Wert, Abschnitt 12):       offen
T0.5  Liste der ESP8266-Stellen, Sortierung der 495 Serial-Stellen, Zufallszahlen-Nutzung:       offen
```

## 12. Abgleich mit `specs/multiboard-quelle/` (W1–W10, dritter Stand 17.10.2026)

`multiboard-quelle` (Entwurf des `sw-architect`, **nicht freigegeben**) hat in ihrer `requirements.md` zehn Abstimmungspunkte zur Spec des S3-Ports gelistet; deren Task 0.2 verlangt je Zeile ein „übernommen / verworfen, weil …". **Die Zahlen und Messungen stehen dort und werden hier nicht kopiert.**
„Übernommen" heisst: diese Spec ist nachgezogen; es heisst **nicht**, dass die Gegenseite freigegeben ist — die Freigabe liegt beim Nutzer (Q1–Q13).

| W | Gegenstand | Antwort | Wo nachgezogen |
|---|---|---|---|
| **W1** | Attrappen für die Hersteller-Header fehlen; ohne sie ist AK6a nicht erfüllbar | **übernommen.** Die Annahme „die Einbindungen fallen weg" ist widerlegt; Attrappen, erzwungene Einbindungen und Platzhalter sind Zulieferung des S3-Gerüsts (Task 1.1 = dort Task 2.2, nicht doppelt anlegen) | §3.6 (Berichtigung), §9, AK6a, Task 1.1 |
| **W2** | `esp32s3/pins.h` → `boards/<id>/board.h` | **übernommen.** Es gibt eine Pinquelle; die 11-Signale-Prüfung ist C5 / AK9 dort | §1.1, §2, AK1, AK34, Tasks 0.1b, 0.3 |
| **W3** | Dateiliste der STM-Komponente aus `boards/modules.txt` | **übernommen.** | §1.1, §3.6, AK35, Task 1.1 |
| **W4** | „hardwarenah" ist mehr als elf Dateien; verbindlich ist `boards/modules.txt` | **übernommen**; meine früheren Zahlen (20 / 15 von 25 / elf) sind zurückgenommen | §3.6, Task 0.4 |
| **W5** | `display.c` hat 6'738 Zeilen, nicht 6'739 | **übernommen**, **nachgezählt** (Grep auf jede Zeile, 17.10.2026) | `requirements.md` Problem, §3.6, §9 |
| **W6** | Plattformanteil von `main.c` herauslösen: Mittel bestimmt, ob die Bestandsabbilder byte-gleich bleiben | **übernommen.** Die frühere Task 5.1 wird **durch `multiboard-quelle` Tasks 3.1–3.8 ausgeführt** (Seam-Guards statt ausgelagerter Plattformdatei, Klasse H2, Hash); 5.1v und 5.1r entfallen als eigene Tasks bzw. werden bedingt | §1 (3), §3.5, §7, `tasks.md` Stufe 5 |
| **W7** | Versionierung des S3-Abbilds (F13) | **übernommen als Befund, offen als Entscheid.** Meine Empfehlung „beide Symbole gemeinsam" ist zurückgenommen; **keine Annahme, S3-Release blockiert** bis Q2 | F13, §10 |
| **W8** | Nachweis für den F103 | **übernommen.** Der Hash trägt allein, solange es kein Prüfgerät gibt (Q3); AK33 nennt es | AK33 |
| **W9** | Besitz von `boards/` und `esp32s3/`; Hook kennt sie nicht | **übernommen.** Rolle, Hook, `no-build.py` leistet dort Task 0.3; hier bleibt die `device.conf`-Vorlage | §1.1, Task 0.3 |
| **W10** | `esp8266.c` entfällt nicht komplett (E5) | **übernommen.** Ersetzt wird nur die unterste UART-Schicht; `esp8266.c` bleibt, mit dem Guard `WC_PEER_EXTERNAL` (Verhalten: F10 = dort Q11) | §1 (4), §3.3, §6, §9, AK21 |

**Fragen von `multiboard-quelle`, die diese Spec berühren — sie bleiben beim Nutzer:**

| Q dort | Berührt hier | Bis zum Entscheid gilt |
|---|---|---|
| Q1 (Reichweite der Board-Beschreibung, Stufe 5) | keine Wirkung auf den S3-Port ausser den Pins der Bestandsboards | Stufe 5 dort nicht freigegeben |
| Q2, Q9 (Version des S3-Abbilds; Versionspflicht bei hash-gleicher Umstellung) | F13, §10, Task 5.1v, Task 6.1 | keine Annahme; wo es ein Bump zu entscheiden gilt, wird gebumpt |
| Q3 (F103-Prüfgerät) | AK33 | nur Hash |
| Q4 (Chip-Code des S3 im Konfigurationswort) | `smoke-device.sh`, PWA-Anzeige, E1 | der S3 meldet keinen gültigen Chip-Code; `smoke-device.sh` deutet ihn nicht als F411 |
| Q5 (Toolchain: ESP-IDF-Installation, Versionsbindung) | F12, Task 1.1 | Entwurf dort rechnet mit IDF 5.5.x, ungebaut |
| Q7 (maschinenlesbares `pinbelegung.md`) | AK1, Task 0.1b | `board.h` wird vorerst nur gegen die Sperrliste und die Signalzahl geprüft |
| Q8 (`F_INTERRUPTS` per Übersetzer-Schalter) | F14, AK16 | S3 setzt aus `board.h`, Bestand bleibt |
| Q11 (Steuerpins zum zweiten Chip) | F10 | Empfehlung F10 gilt |
| Q12 (Frist im `refresh`-Wait: Kern oder Backend) | §3.4, §3.6, Task 2.1 | Frist **im Backend**, Kern unverändert |
| Q13 (Layout des S3-Abbilds: 12h oder 24h) | AK8 (234 LEDs = 12h-Layout) | ein Abbild, 12h, Annahme ○ dort |

**Widersprüche, die offen bleiben (an den Lead; keine Spec überschreibt die andere):**

1. **Wortlaut von E11 („eigener Treiber") gegen Kern und Backend.** Diese Spec führt die Änderung als **Nachtrag** und lässt E11 stehen; der Nutzer bestätigt sie mit der Freigabe von `multiboard-quelle`. Bis dahin gilt E11 wie geschrieben und F21 ist offen.
2. **Reihenfolge E7 gegen die Baseline-Pflicht (F21).** Das erste Licht auf dem S3 hängt jetzt an Arbeit am Bestand (dort Tasks 3.1–3.3, davor Baseline 1.5/1.6).
3. **`multiboard-quelle` führt `BOARD_LDR_ADC_ATTEN_DB` noch als „6 oder 12 (offen)" und die Kalibrierung als „neu gemessen"** (dort `design.md` §3.5.3, §5.3). Durch E13 überholt: 0 dB, Grenzwerte prüfen und einmalig nachkalibrieren. **Dort nachzuziehen.**
4. **`eeprom.c` ist Klasse „Logik", erhält aber E3.** Die dortige AK7 verlangt für alle Logikdateien einen leeren Diff und nennt Ausnahmen nur über §9.3; hier ist E3 als **Ausnahme beantragt** geführt (AK35, §3.6). Dort klarzustellen, dass E3 die benannte Ausnahme ist.
5. **Zahl der `watchdog_reload ()`-Aufrufstellen:** Diese Spec nannte 7 (aus `specs/bruecke/`), `multiboard-quelle` nennt einen anderen Wert für den S7-Stand. Task 0.4(i) zählt nach; keine der beiden Zahlen ist hier als gültig gesetzt.
6. **Dort Task 3.9 (ISR-Log-Entscheid, `sw-architect`) und hier Task 4.1 (Umsetzung):** Der Entscheid muss **vor** Stufe 4 liegen; steht er aus, beginnt 4.1 nicht.
7. **Framework:** Die Empfehlung dort (O1) ist **nicht entschieden**; Task 1.1 hier beginnt erst nach dem Entscheid (F12).
