# Anforderungen — Eine Quelle für alle Boards (STM32F103, STM32F411 und weitere, ESP8266, ESP32-S3)

**Status:** Entwurf — zur Freigabe durch den Nutzer. Ohne Freigabe beginnt keine Umsetzung.
**Stand:** 2026-10-17
**Verfasser:** `sw-architect`
**Auslöser:** Vorgabe des Nutzers vom 17.10.2026, wörtlich: *„wenn möglich immer noch die gleiche Source für alle
Varianten, damit wir nicht plötzlich auseinander driften — eine Entwicklung für alle Boards, welche aber auch das
neue abdeckt“.* Und: *„LED-Ansteuerungslogik so weit wie möglich übernehmen“.* Der Nutzer verlangt 100%ige
Professionalität.
**Schwesterspezifikation:** `specs/esp32s3-port/` (Verfasser `spec-writer`). Diese Spec **ersetzt sie nicht** und
überschreibt nichts darin. Diese Spec ist die **Architektur-Klammer**: Naht, Verzeichnisse, Board-Beschreibung, Bau-Matrix und
Schutz vor Auseinanderdriften gehören hierher; die Aufgaben des S3-Ports (RMT-Treiber, Tasks, Brücke, Netz-Seite)
bleiben dort. Die Stellen, an denen beide einander berühren oder widersprechen, stehen in „Abstimmung mit
`specs/esp32s3-port/`“ unten.

## Wie diese Spec ihre Aussagen kennzeichnet

| Zeichen | Bedeutung |
|---|---|
| **✔** | Am Quelltext, an der Build-Konfiguration oder an einem vorhandenen Bauergebnis **am 17.10.2026 nachgemessen**. Das Messverfahren steht in `design.md`, Anhang A, damit jede Zahl wiederholbar ist |
| **●** | Aus einer Sekundärquelle übernommen und **nicht selbst nachgeprüft**: `specs/esp32s3-port/`, `HARDWARE.md`, das Komponentenblatt `hardware/komponenten/esp32s3-wroom-1.md`, die Meldung des Lead vom 17.10.2026, ein älterer Bauergebnis-Bericht |
| **○** | Annahme oder Herstellerwissen ohne Beleg im Repo. **Ein Entscheid, der auf ○ steht, ist ein offener Punkt** und steht in „Offene Fragen“ |

**Eine Zahl ohne Messverfahren ist eine Meinung.** Wo diese Spec eine Zahl nennt, ist sie gezählt, nicht geschätzt;
wo sie geschätzt ist, steht „○“.

## Problem

Heute gibt es **ein** Quellverzeichnis `src/**` für den STM32, aber **mehrere** Wege, es zu übersetzen, **mehrere**
Boards darin, und daneben einen **zweiten, getrennten** Quellbaum für den ESP. Das ist bis heute beherrschbar. Mit dem
geplanten Ein-Chip-Board (ESP32-S3-WROOM-1 ersetzt STM32 **und** ESP-12F) entsteht ein dritter Zielrechner, und genau dort
beginnt das Auseinanderdriften, wenn nichts dagegen steht. Belegt (alle ✔, Verfahren in `design.md` Anhang A):

1. **Die Variabilität ist gross und liegt zum Teil an den falschen Stellen.** 123 Quelldateien (`*.c`, `*.h`, ohne
   `src/stubs`) tragen **100 Zeilen mit Board-Makros** (`BLACKPILL_BOARD`, `BLUEPILL_BOARD`, `BLACK_BOARD`,
   `NUCLEO_BOARD`) in **31 Dateien** und **226 Zeilen mit Chipfamilien-Makros** (`STM32F10X`, `STM32F4XX`, …) in
   **57 Dateien**. Dazu kommt die LED-Typ-Achse: `display.c` allein trägt **34** Bedingungszeilen auf `DSP_USE_*`.
2. **Die Naht zwischen Logik und Hardware ist da, aber nicht dort, wo man sie vermutet.** `display.c` nennt in seinen
   eigenen `#include`-Zeilen keinen STM-Header — das stimmt. **Transitiv stimmt es nicht:** `display.c` →
   `display.h` → `sk6812.h` → `stm32f4xx.h` und alle SPL-Kopfdateien. Alle 20 Logikdateien ziehen
   26 SPL-Kopfdateien und 4 Chip-/CMSIS-Kopfdateien (`core_cm4.h`, `stm32f4xx.h`, `stm32f4xx_conf.h`, `system_stm32f4xx.h`) herein. 26 von 76 Kopfdateien binden Hersteller-Header ein, und **8 davon binden
   `misc.h` unbedingt** ein (`sk6812.h`, `ws2812.h`, `apa102.h`, `tftled.h`, `onewire.h`, `uart.h`, `w25qxx.h`, `xpt2046.h`), ohne dass ein
   Familienmakro es schützt (insgesamt 11 `misc.h`-Zeilen in 10 Kopfdateien; zwei Dateien, `irmpsystem.h` und `uart-driver.h`, schützen sie). Gleichzeitig übersetzen **28 von 47** `.c`-Dateien
   gegen **leere** Hersteller-Header ohne einen Fehler — der Code selbst benutzt keinen Herstellertyp, nur die
   Kopfdateien. Die Naht ist also eine **Include-Naht**, keine Code-Naht, und sie ist **durchlässig** (siehe 3).
3. **Eine Prüfung gegen Auseinanderdriften gibt es nicht.** Im Routinelauf (`guardrails.sh --full`, Stufe S8) bauen
   **zwei** von **72** Varianten (`make f103`, `make f411`). Die übrigen 70 baut nur `make matrix`, und das ruft
   niemand bei jeder Änderung. Der letzte Bericht stammt vom 13.10.2026 (● `build/matrix/matrix-report.txt`:
   72 von 72 gebaut; engste Variante `wc24h-stm32f103-sk6812-rgbw` mit **1 172 Byte** frei — und das ist ein **Flash-Gate
   mit realer Enge**, kein Spielraum für eine Abstraktionsschicht).
4. **Das Auseinanderdriften hat bereits stattgefunden — in der Dokumentation der Pins und in den Makronamen:**
   - **Drei falsche Pin-Kommentare in drei Dateien** (Code jeweils richtig): `src/esp8266/esp8266-uart.c:56` beschreibt
     `BLACKPILL_BOARD` als „USART6 ALT1: PA11 | PA12“, die Zeilen darunter wählen `UART_NUMBER 2`, `UART_ALTERNATE 0`, also
     USART2 auf PA2/PA3 — so steht es in `HARDWARE.md` und im Schaltplan. `src/button/button.c:36` nennt unter
     `BLACKPILL_BOARD` „Nucleo Board PC13“, der Code nimmt PA0. `src/wpsbutton/wpsbutton.c:27` nennt „PB5“, der Code nimmt PA7
     (●/✔ auch im Inventar des `firmware-analyst`, `REVIEW-2026-10-17-hardware-naht.md`). **Wer die Pins aus den Kommentaren
     abliest, baut ein falsches Board.** Ebenso falsch gerechnet: der Kommentar „15015“ am F411-Zeitgeber (`main.c:735`),
     tatsächlich 100 MHz / 66 / 101 = 15 001,5 Hz (● Inventar).
   - **Tote Boardzweige:** `DISCO_BOARD` steht in **5 Zeilen in 4 Dateien** (`board-led.c`, `button.c`, `esp8266.c`,
     `irmpconfig.h`), wird aber **nirgends definiert** (0 Treffer in `CMakeLists.txt` und beiden `.ebp`). Ebenso
     `HW_STM32_F446RE`: ein Code im Konfigurationswort, aber kein Bauziel.
   - Vier Kopfdateien (`power.h`, `board-led.h`, `ds18xx.h`, `onewire.h`) schalten auf `STM32F1XX`. **Dieses Makro wird
     nirgends definiert** (0 Treffer in `CMakeLists.txt`, `cmake/**`, beiden `.ebp`; alle anderen Stellen heissen `STM32F10X`).
     Die Zweige laufen auf dem F103 nie. Harmlos, weil andere Kopfdateien dasselbe einbinden — aber **niemand hat es
     bemerkt**, und genau so wächst eine zweite Welt unbemerkt.
   - Der Chip-Code im Konfigurationswort (`src/main.h`, `HW_STM32_*`) und die Namenstabelle in `ESP8266/ESP-uclock/http.cpp`
     sind **zwei Abschriften derselben Zuordnung**. Dasselbe gilt für `eeprom-data.h` ↔ `eepromdata.h`, `vars.h` ↔ `vars.h`,
     `tables.h` ↔ `tables.h`: **5 gleichnamige Kopfdateien** und **5 gleichnamige Quelldateien** in den beiden Bäumen,
     mindestens **6 gleichnamige globale Funktionen** (`dayofweek`, `eeprom_read`, `htoi`, `strsubst`, `tables_init`,
     `var_crc`; ● per Skript gezählt, nicht per `nm`).
5. **Die LED-Logik ist zu etwa einem Fünftel wiederverwendbar im Treiber und zu fast ganz im Aufrufer.** `display.c`
   ruft den Treiber über **drei** Funktionen (19 Aufrufstellen: `led_init` 1, `led_refresh` 4, `led_set_led` 14),
   `main.c` zwei Zähler. `led_set_all_leds` benutzt `display.c` **gar nicht** (0 Treffer). Im Treiber `sk6812.c`
   (671 Zeilen) sind rund 140 Zeilen portable Logik (Bildpuffer mit Doppelpuffer, `set_led`, `set_all_leds`, `refresh`-Rahmen,
   Zähler), rund 270 Zeilen Timer/DMA/Interrupt, der Rest Konfiguration pro Board. Wer den Treiber für den S3 neu
   schreibt, kopiert die 140 Zeilen — und sie driften.
6. **Die Brücke ist die zweite Naht, und sie ist heute eine UART — aber ihr Wissen steckt in sechs Logikmodulen.**
   `weather.c`, `timeserver.c`, `esp-spiffs.c`, `tables.c`, `tetris.c` und `snake.c` rufen `esp8266_send_cmd` und
   `esp8266_get_message` **direkt** (● Inventar, Abschnitt 2/5). Die richtige Naht ist deshalb **nicht** die Brücke als
   Ganzes, sondern ihre unterste Schicht (die 14 `esp8266_uart_*`-Funktionen, `uart.h`); darüber bleibt `esp8266.c` mit dem
   Zeilenprotokoll unverändert (`specs/esp32s3-port` E5). Auf dem S3 werden beide Welten in **einem** Abbild gelinkt. Das
   Zusammenlegen zweier Quellbäume mit Namenskollisionen (siehe 4) ist ein Entwurfsproblem, kein Handgriff.
7. **Die Naht hat Rückwärts-Abhängigkeiten, und die machen jeden Modulaustausch zum Eingriff in die anderen** (● Inventar,
   Abschnitt 5): `sk6812.c` liest `uptime` aus `main.c` und ruft `log_printf`; `eeprom.c` ruft `esp8266_uart_rxdrops()`,
   `diag_ticks()` und `log_printf`; `dcf77.c` ruft **aus der Timer-ISR** `log_message` und `board_led_on/off`; die Logik ruft
   `schedule_esp8266_messages` und `watchdog_reload` in `main.c` zurück; `delay_counter` ist ein öffentlich beschreibbarer
   Globalzustand, den `i2c.c` und `onewire.c` direkt benutzen. Eine Hardware-Schicht, die nur Funktionsnamen ersetzt,
   erbt alle diese Kopplungen. **Diese Kopplungen müssen als benannte Plattformdienste in die Schnittstelle** (`design.md` §3.3).

## Ziel

**Eine** Quelle, aus der jedes Board gebaut wird: die vorhandenen STM32-Boards unverändert, ESP8266 unverändert, und der
ESP32-S3 als weiteres Board **ohne** einen zweiten Entwicklungsstrang. Unterschiede liegen an **drei benannten Orten** — in
der **Board-Beschreibung**, in der **Hardware-Schicht** und in der **Netz-Plattformschicht** — und **nicht quer durch die
Logik**. Eine Prüfung stellt bei jeder Änderung sicher, dass kein Board still bricht; eine zweite weist nach, dass der
Umbau die Bestandsboards nicht verändert hat.

## Vorgaben und Randbedingungen

| # | Vorgabe | Quelle |
|---|---|---|
| V1 | Dieselbe Quelle für alle Varianten, eine Entwicklung für alle Boards, die auch den S3 abdeckt | Nutzer, 17.10.2026 |
| V2 | LED-Ansteuerungslogik so weit wie möglich übernehmen | Nutzer, 17.10.2026 |
| V3 | Bestandsboards bleiben **unverändert**, **nachgewiesen** (Binärgleichheit oder Gerätetest nach DIR-012) | Rolle `sw-architect`, DIR-012, DIR-014 |
| V4 | PWA, `/api/*` und die Kommandozeichenfolgen der Brücke bleiben für **alle** Varianten gleich; die PWA läuft parallel zur Legacy-Oberfläche | CLAUDE.md, Invarianten |
| V5 | App-Assets nur als `.gz`, Existenzprüfung mit Grösse > 0; Wetter nur über `/api/weather_get_now` und `/api/weather_get_forecast`; `pending_ticker_restore` mit allen vier Teilbedingungen | CLAUDE.md, Invarianten |
| V6 | Die vier Versionsstellen und das **Zeilenformat**, das das `Makefile` per `grep` ausliest (`'^#define VERSION' src/main.h`, `'^#define ESP_VERSION' ESP8266/ESP-uclock/version.h`, `'^const APP_VERSION'`, `CACHE_NAME`), ändern sich nicht | CLAUDE.md |
| V7 | Builds laufen seriell, nur der Lead baut (R1); jede Datei hat genau einen Schreiber (R3); Reihenfolge ist nicht Besitz (R3b); Versionen bumpt nur der Lead bzw. `release-engineer` (R4) | CLAUDE.md |
| V8 | **Der S3 hat nur ein Binärabbild für DevKit und Nachbau** (kein `#ifdef DEVKIT`), nur GPIOs, die das DevKit herausführt, und nicht GPIO0, 3, 45, 46, 19/20, 43/44, 35–37, 38, 48 als Nutzsignale | `specs/esp32s3-port` E1/E2 ● |
| V9 | Das LED-Board bleibt unverändert; **alle Software-Anforderungen an die Hardware** laufen über `hardware/architektur/pinbelegung.md` (liefert `hw-architect`, **existiert am 17.10.2026 noch nicht** ✔: `hardware/architektur/` ist leer) | Nutzer, `specs/esp32s3-port` E10 ● |
| V10 | Der Port ist **1:1**: kein Funktionsverlust, keine neuen Funktionen; diese Spec fügt **keine** Funktion hinzu | `specs/esp32s3-port` E8 ● |
| V11 | **Die LED-Board-Schnittstelle bleibt 100 % pin- und funktionskompatibel.** Für die Software heisst das, **in der Hardware-Schicht erhalten**: die `led_*`-Schnittstelle mit der Bitreihenfolge **G, R, B, W** (MSB zuerst) und den Zeiten **Periode 1250 ns, T0H 300 ns, T1H 750 ns, Pause 250 µs**; `power_on()`/`power_off()` mit den **200 ms Einschwingzeit im Aufrufer** (`display.c:2954-2955`, nicht im Treiber); die LDR-Lesung mit **Rohwerten 0..4095 der STM-Skala**, die Normierung gegen `ldr_min_value`/`ldr_max_value` bleibt in `ldr.c` | Nutzer über Lead, 17.10.2026; ✔ `sk6812.c:91-97`, `:354-373` |

## Akzeptanzkriterien

Jedes Kriterium nennt, **womit** es nachprüfbar ist. Die Reihenfolge folgt den Stufen in `design.md` §9; Stufe 0 und 1 ändern
**keine** Quelldatei.

### Messen und absichern, bevor etwas umgebaut wird (Stufe 0 und 1)

- [ ] **AK1 — Baseline ist reproduzierbar.** Zweimal gebaut (jeweils frisch, `make clean-stm clean-esp` vorher) liefern alle
      **72** Matrix-Varianten, `f103`, `f411` und das ESP8266-Abbild **dieselbe SHA-256-Summe**. Die Zahl der verglichenen
      Abbilder steht im Bericht und ist **72 + 2 + 1** (die beiden Routineziele sind zusätzlich Teilmenge der 72 und werden
      **nicht doppelt gezählt**; die Prüfung nennt beide Zählungen). Eine Prüfung, die eine andere Zahl meldet, gilt als falsch.
      Weicht eine Summe zwischen den zwei Läufen ab, ist der Build **nicht** deterministisch, und **alle** späteren
      „binärgleich“-Nachweise dieser Spec entfallen — dann ersetzt Stufe 3 den Hash durch den Gerätetest, und das steht im Bericht.
- [ ] **AK2 — Jede Quelldatei ist klassifiziert.** `boards/modules.txt` führt **jede** `*.c` unter `src/` in genau einer
      Klasse (Logik, gemischt, Hardware, TFT/Black-Board, Plattform). Die Prüfung zählt: **49** `.c`-Dateien (47 plus 2 in
      `src/stubs`) gegen die Zahl der Einträge; eine Datei ohne Eintrag und ein Eintrag ohne Datei **schlagen fehl**. Die Prüfung
      gleicht zusätzlich die Dateilisten der beiden `.ebp` (**219** bzw. **155** `Unit`-Einträge, davon die `.c`/`.S`)
      gegen die Liste ab, damit **eine neue Datei, die nur in einer Liste steht**, auffällt.
- [ ] **AK3 — Logik übersetzt gegen Attrappen.** Jedes Modul der Klasse „Logik“ (heute **20** Dateien, darunter `tetris.c` und `snake.c`) übersetzt mit
      `cc -fsyntax-only` gegen **leere Hersteller-Header** fehlerfrei, für **mindestens drei** Board-Konfigurationen
      (BlackPill-F411, BluePill-F103, S3). Die Prüfung nennt die **Zahl der Fälle** (**20 × 3 = 60** ab Stufe 2; in Stufe 1, vor der S3-Konfiguration, **20 × 2 = 40**) und schlägt fehl,
      wenn sie 0 Fälle betrachtet. **Die Prüfung belegt Übersetzbarkeit, nicht Verhalten** (siehe `design.md` §6, Grenzen).
- [ ] **AK4 — Streuung ist gedeckelt und messbar.** Ein Zähler führt je Datei die Zahl der Board- und Familien-Bedingungen
      (Ausgangsstand: **100 / 226 Zeilen in 31 / 57 Dateien**). **Er darf nirgends steigen**; `display.c` bleibt bei **0**.
      Eine neue Bedingung in einer Logikdatei **schlägt fehl**. Wächst eine Zahl in einer Hardware-/gemischten Datei, muss die
      Baseline in **demselben Commit** mit Begründung angehoben werden.
- [ ] **AK5 — Unbekannte Makronamen schlagen an.** Eine Prüfung weist `#if defined (<NAME>)` mit einem Chip-, Familien-
      oder Boardnamen ab, der **nirgends definiert** ist. Die Prüfung findet den Bestand (`STM32F1XX`, **4 Dateien**) und nennt ihn;
      die 4 Stellen stehen in einer **benannten Ausnahmeliste**, nicht in einem stillen Filter.
- [ ] **AK6 — Jede neue Prüfung hat einmal versagt (DIR-014).** Je Prüfung aus AK1–AK5 und AK9–AK12 existiert eine
      **Gegenprobe**, die eine Verletzung an einer **Kopie** herstellt (`--root`), und der Lauf zeigt: Die Prüfung schlägt an,
      der **Exitcode** ist ≠ 0, die **Meldung erscheint auf dem Kanal, den der Empfänger liest** (Ausgabe von `guardrails.sh`,
      nicht eine gepufferte Pipe). Die Gegenproben stehen in `design.md` §6.4 und werden in `tasks.md` je Task abgenommen.

### Eine Quelle, ein Board mehr (Stufe 2 bis 4)

- [ ] **AK7 — S3 baut `display.c` und alle Logikmodule unverändert.** `git diff <Stand vor Stufe 2> -- ` auf **alle 20**
      Dateien der Klasse „Logik“ ist **leer**. Jede Ausnahme steht mit Grund und Beleg in `design.md` §9.3 und ist vom
      Nutzer vorab freigegeben. (Deckt sich mit `specs/esp32s3-port` AK35 und **ersetzt** deren Dateiliste durch die Klasse
      „Logik“ aus `boards/modules.txt`, damit die Liste nicht von Hand gepflegt werden muss.)
- [ ] **AK8 — Der LED-Treiber hat einen gemeinsamen Kern.** Bildpuffer (Doppelpuffer), `set_led`, `set_all_leds`, der
      `refresh`-Rahmen (Warten auf das vorige Bild, Pufferwechsel, Kopie) und beide Zähler stehen **einmal** in einer
      Einbindedatei, die **STM- und S3-Treiber textuell einbinden**; die Timing-Konstanten in Nanosekunden
      (`SK6812_T0H_TIME` usw.) stehen **einmal** in einer Kopfdatei. Beleg: `grep -c` auf die Funktionsnamen in `src/` und
      `esp32s3/` zeigt **je eine** Definition. **Für alle 72 Varianten bleibt die SHA-256-Summe gleich** (Binärgleichheit,
      weil die Einbindedatei den **gleichen Tokenstrom** liefert).
- [ ] **AK9 — Eine Board-Beschreibung je Board, die Pins und Merkmale trägt.** `boards/<id>/board.h` für den S3 führt **alle
      11 Signale** (LDR, DS18B20, LED-Daten, LED-Versorgung, IR, SDA, SCL, DCF77, Taster USER, Taster WPS, Status-LED) und die
      Merkmale aus `design.md` §5.3. Eine Prüfung gleicht sie gegen `hardware/architektur/pinbelegung.md` ab (**Zahl der
      Signale: 11**), weist jede GPIO-Nummer der Sperrliste (V8) ab und verlangt, dass **kein** Pin in `esp32s3/**` ein zweites
      Mal von Hand steht (`grep` auf GPIO-Literale ausserhalb von `board.h`: **0 Treffer**).
- [ ] **AK10 — Kein Zielwechsel durch Hintertür.** Weder `esp32s3/**` noch `boards/**` enthalten `DEVKIT`, `BLACKPILL`,
      `BLUEPILL` usw. in einer Bedingung; Unterschiede zwischen DevKit und Nachbau sind **keine** Software-Unterschiede
      (V8). `grep -rn DEVKIT esp32s3/ boards/` ohne Treffer.
- [ ] **AK11 — Gleiche Semantik-Schalter und gleiche Datenlayouts.** Der S3-Bau übersetzt mit den **gleichen**
      Sprach-Schaltern wie die Bestandsziele (mindestens `-std=gnu11`, `-fno-strict-aliasing`; zusätzlich diejenigen, die
      Zeichen- und Aufzählungstypen auf die Semantik des ARM-Ziels festlegen, **Liste und Beleg aus Task 3.5**), und eine
      Prüfung `_Static_assert`-t die Grössen und Versätze aller Typen, die über **EEPROM, Brücke oder Backup** laufen, sowie die Breiten von
      `uint_fast8_t`, `uint_fast16_t`, `time_t` und `char`-Vorzeichen — **auf jedem Ziel**, einschliesslich des S3. Ohne diese
      Gleichheit ist „Datenformat unverändert“ (`specs/esp32s3-port` AK13) nicht belegbar.
- [ ] **AK12 — Die Restore-Bedingung bleibt wörtlich.** Der Block `pending_ticker_restore && ! display_ticker_active () &&
      ! display.do_display_icon && show_icon_stop_time == 0 && show_overlay_idx >= MAX_OVERLAYS` (✔ heute `src/main.c:4910-4914`)
      ist nach **jedem** Task unverändert; eine Prüfung vergleicht den Text des Blocks gegen eine Referenz und schlägt fehl,
      wenn **eine** der vier Teilbedingungen fehlt.

### Bestandsboards unverändert (Stufe 3 und 5)

- [ ] **AK13 — Binärgleichheit der Bestandsziele.** Nach jedem Task der Klasse „Hash-gleich“ (`tasks.md`, Spalte
      „Nachweis“) stimmen **alle 72** Matrix-HEX-Summen und die des ESP8266-Abbilds mit der Baseline überein.
      Der Vergleich nennt Variante und beide Summen bei jeder Abweichung. **Weicht eine Summe ab**, ist der Task entweder
      falsch oder gehört in Klasse F (Funktionsgleichheit, siehe AK14) — **das entscheidet der Lead, nicht der Umsetzer.**
- [ ] **AK14 — Klasse F bekommt einen Gerätenachweis.** Jede Änderung, die den Bestandscode nicht nur umordnet, sondern verändert
      (Klasse F, `design.md` §9.1), wird mit `./tools/smoke-device.sh` **und** dem `pwa-tester` an der Test-Uhr (STM32F411,
      Platine V2) abgenommen (DIR-012) und nennt je Variante die **Flash-Differenz** in Byte. Für den F103 gilt zusätzlich: Eine
      Änderung der Klasse F darf die freie Flashmenge der engsten Variante (**1 172 Byte**, ●) **nicht verringern**, ohne dass der
      Nutzer es vorab freigibt.
- [ ] **AK15 — `guardrails.sh` und `--full` laufen mit Exit 0**, und beide Zählungen (AK2, AK4) stimmen.

### Ein Binärabbild, zwei Welten (Stufe 4)

- [ ] **AK16 — Die Versionen bleiben greppbar.** `grep '^#define VERSION' src/main.h | head -n 1 | cut -d'"' -f2` und
      `grep '^#define ESP_VERSION' ESP8266/ESP-uclock/version.h | head -n 1 | cut -d'"' -f2` liefern **unverändert** je eine
      Versionszeichenkette; Guardrail S4 meldet für beide Dateien „genau einmal“. Wie das S3-Abbild versioniert wird, entscheidet
      **Frage Q2**; **bis dahin gilt keine Annahme**.
- [ ] **AK17 — Keine Symbolkollision.** Das gelinkte S3-Abbild enthält **kein** global definiertes Symbol doppelt
      (die Prüfung nennt die **Zahl der geprüften Symbole**) und die zwei Include-Pfade (STM-Komponente, Netz-Komponente)
      teilen **kein** Verzeichnis, in dem gleichnamige Kopfdateien liegen (5 Namen, siehe Problem 4).

- [ ] **AK18 — LED-Board-Schnittstelle als prüfbarer Vertrag (V11).** Die Hardware-Schicht jedes Ziels erfüllt die
      Verträge aus `design.md` §3.5 (Bitreihenfolge G,R,B,W; Zeiten in Nanosekunden aus **einer** Kopfdatei; Pause ≥ 250 µs;
      Refresh wartet auf das vorige Bild; `power_on/off` ohne eigene Wartezeit; LDR-Rohwert 0..4095). Beleg je Ziel:
      **STM** — Quelltext-Prüfung, dass die Konstanten aus der gemeinsamen Kopfdatei kommen (`grep`: genau **eine**
      Definition je Konstante); **S3** — die Messungen der S3-Spec (AK5, AK6, AK8, AK17), **nicht** ein zweites
      Messverfahren hier.
- [ ] **AK19 — Plattformdienste statt Rückwärtsgriffe.** Die Dienste, die Hardware-Module und Logik heute aus `main.c` beziehen
      (`uptime`, `watchdog_reload`, `schedule_esp8266_messages`, `diag_ticks`, `esp8266_uart_rxdrops`, `log_*`), stehen **in
      der Dienstliste `boards/platform-services.txt`** (`design.md` §3.3, **keine neue Kopfdatei**) und sind die **einzigen** Rückwärts-Abhängigkeiten; ein Zähler listet sie und schlägt
      fehl, wenn ein neuer hinzukommt. **Ausgangsstand ✔ gezählt** (Dateien ausser `main.c`): `uptime` **7**, `watchdog_reload`
      **6**, `schedule_esp8266_messages` **4**, `diag_ticks` **1**, `esp8266_uart_rxdrops` **1** (beide nur `eeprom.c`) —
      zusammen **11 verschiedene Dateien** —, dazu `log_*` in **19 Dateien**. `delay_counter` ist ausserhalb von `delay.c`/`delay.h`
      in **2 Dateien** beschreibbar (`i2c.c`, `onewire.c`; der S3 ersetzt beide); die Zahl darf nicht steigen.
- [ ] **AK20 — Aufrufkontext ist Teil der Schnittstelle.** Jede Funktion der Hardware-Schicht trägt im Kopf ihrer Kopfdatei
      eine **Kontextangabe** (`H` Hauptloop, `I` Timer-Interrupt, `D` DMA-/Treiber-Interrupt, `B` blockiert, mit Obergrenze).
      **Die ISR-Aufrufe der Bestandsboards sind benannt:** `irmp_ISR`, `dcf77_tick` (ruft `log_message` und `board_led_*` **aus der
      ISR**), das Setzen der Flags und die Softuhr. Auf dem S3 ist `log_message` aus der Timer-ISR **entweder ISR-fest**
      (Ringpuffer-Schreiben, IRAM) **oder** der Aufruf wird in den Hauptloop verlegt — **die Entscheidung trifft S3-Task 4.1,
      sie darf nicht offen bleiben**; Nachweis: kein Aufruf einer blockierenden Treiberfunktion aus der S3-Timer-ISR
      (`nm`/Aufrufgraph, **Zahl der geprüften ISR-Pfade** im Bericht).

## Nicht Teil dieser Änderung

- **Der Funktionsumfang des S3-Ports** und seine Gerätenachweise (RMT-Timing, USB, ADC, Tasks, Brücke, Update). Die Nachweise stehen in
  `specs/esp32s3-port/`; diese Spec liefert nur den Rahmen, in dem sie landen.
- **Hardwareentscheide und Pinzuordnung** (`hw-architect`); **Bauteilwahl** (`hw-datasheet-analyst`, `hw-designer`).
- **Code.** Diese Spec ändert keine Zeile. Die Spec legt fest, was die Entwickler ändern.
- **Das Verschieben von Dateien nach neuen Verzeichnissen** (`src/logic/…`). Es würde `git blame` für Tausende Zeilen
  verwerfen und ändert nichts an der Naht; die Klasse steht in `boards/modules.txt`, nicht im Pfad (Begründung `design.md` §11).
- **Das Umbenennen von `ESP8266/ESP-uclock/`** (Makefile, Guardrails, Hooks, Doku, `deploy.sh` hängen daran). Das Verzeichnis
  bleibt, obwohl es künftig auch den ESP32-Build der Netz-Seite trägt (Frage Q6).
- **Gemeinsame Kopfdateien für das Brückenprotokoll** (`eeprom-data.h` ↔ `eepromdata.h`, `vars.h`, `tables.h`, die
  `HW_*`-Konstanten). Das ist eine **bestehende** Auseinanderdrift-Quelle, aber ihre Beseitigung ändert beide Seiten und das
  Protokoll; sie gehört in eine **Folgespec** (`design.md` §11, Alternative 7). Diese Spec **benennt** sie und schützt den Bestand
  durch die vorhandenen Prüfstände (`tools/checks/var-crc.c`, S13/S16), mehr nicht.
- **Die Korrektur der vier `STM32F1XX`-Stellen** und des falschen Kommentars in `esp8266-uart.c:56`. Beides ist Bestandscode; die
  Korrektur ist ein eigener Task der **Stufe 5** mit Hash-Nachweis, und **bis dahin** stehen beide in der Ausnahmeliste.
- **Das Abstellen der offenen Hänger-, DS18xx- und Sicherheitsthemen** aus `CLAUDE.md` und `BEFUNDE.md`. Der Umbau nimmt sie
  weder vorweg noch verdeckt er sie (DIR-003: kein pauschaler DMA-Fix, kein Recovery-Mechanismus).
- **Neue Funktionen, neue Boards ausser dem S3**, neue LED-Typen. Der S3 nutzt **nur** `SK6812_RGBW_LED`.
- **Eine Laufzeit-Hardwareschicht** (Funktionszeigertabellen). Verworfen in `design.md` §4.

## Abstimmung mit `specs/esp32s3-port/`

Ich überschreibe nichts dort. Die folgenden Punkte gehen an den Lead, damit der `spec-writer` sie dort nachzieht oder
ausdrücklich verwirft. Jeder nennt, **was ich gemessen habe** und **was bis dahin gilt**.

| # | Stelle in der S3-Spec | Befund | Bis zur Klärung |
|---|---|---|---|
| **W1** | AK6a und AK35: „`git diff src/sk6812/sk6812.h` leer“, „der Testtreiber wird gegen die **originale** `sk6812.h` übersetzt“ | ✔ Das Original bindet `misc.h` **unbedingt** ein (`sk6812.h:29`) und hängt an `stm32f4xx.h` hinter Familienmakros. **Ohne** Attrappen übersetzt kein einziges der 20 Logikmodule (16 scheitern an `misc.h`, 4 an `uint_fast8_t`). **Mit** einer Attrappe `misc.h`, drei erzwungenen Einbindungen (`-include stdint.h` u. a.) und einem Platzhalter für die Chipkennung übersetzen **19 von 20** unverändert; nur `eeprom.c` bleibt bei `EEPROM_PAGE_SIZE` stehen. Die S3-Spec nennt diese Attrappen **nirgends**. | `design.md` §3.2 und §4 führen sie als **Zulieferung des S3-Gerüsts** ein (S3-Task 1.1); AK6a ist damit erfüllbar, aber nur so |
| **W2** | `design.md` §2 und Task 0.3: `esp32s3/pins.h` mit Prüfskript | Zwei Quellen für Pins wären das, was diese Spec verhindern soll. | **`boards/<id>/board.h` ersetzt `esp32s3/pins.h`** (Empfehlung); die 11-Signale-Prüfung der S3-Spec wird die Prüfung aus AK9 |
| **W3** | `design.md` §1.1: neue IDF-Komponente `stm_core` mit **von Hand** gepflegter Dateiliste | Eine zweite Dateiliste (neben 2 `.ebp` und `CMakeLists.txt`) driftet. | Die Liste **kommt aus** `boards/modules.txt`; AK2 gleicht die Quellen gegeneinander ab |
| **W4** | `design.md` §4.1: „hardwarenah“ sind elf Dateien | ✔ Gemessen **19** `.c` bei Attrappen-Übersetzung (plus 1 Grenzfall `delay.c`), plus **8** TFT-/Black-Board-Dateien, die auf dem S3 entfallen (`tetris.c` und `snake.c` gehören **nicht** dazu: Das sind Spiele auf der Wortmatrix und damit Logik, also S3-Funktion nach E8). Ergänzt gegenüber der S3-Spec: `irmp.c`, `log-uart.c`, `dfplayer-uart.c`, `esp8266.c`, `main.c`, `apa102.c`, `ws2812.c`, `system_stm32f4xx.c`. Kein Widerspruch in der Sache, aber die Dateiliste in S3-Task 1.1 muss `boards/modules.txt` folgen | `design.md` §1.4 |
| **W5** | `requirements.md`: „`display.c` **6'739** Zeilen“ | ✔ `wc -l src/display/display.c` = **6 738**, und eine byteweise Zeilenzählung (Python `splitlines`) ergibt ebenfalls 6 738; 261 481 Byte, keine `\r`-Zeichen, Datei endet mit Zeilenumbruch. Die Zahl 6 739 ist um **eine** Zeile abweichend und von mir nicht reproduzierbar; 7 374 (Lead-Auftrag) ist es auch nicht. Unerheblich für die Sache | Dort anpassen oder lassen |
| **W10** | Inventar des `firmware-analyst`, Abschnitt `esp8266`: „S3: Das Modul **entfällt komplett**“ | Das widerspricht dem Entscheid des Nutzers **E5** („Die Kommandobrücke bleibt als Schicht“) und der Tatsache, dass **sechs Logikmodule** `esp8266_send_cmd`/`esp8266_get_message` direkt rufen (● Inventar, ✔ Stichprobe `esp-spiffs.c:39`). Eine Entfernung der Schicht änderte **alle sechs** Module und das Protokoll | Ich folge **E5**: `esp8266.c` (Zeilenprotokoll) bleibt, ersetzt wird die **unterste** Schicht (`esp8266_uart_*`) und die Steuerpins (`design.md` §3.4). Die Aussage des Analysten ist eine Empfehlung, kein Entscheid |
| **W6** | Task 5.1: Plattformanteil von `main.c` herauslösen, „am STM nachgewiesen“ | ✔ Der Plattformanteil von `main.c` liegt zusammenhängend in den Zeilen ~200–900 plus Teilen von `main()` (91 der 5 378 Zeilen mit Herstellerzugriff). **Wie** man ihn herauslöst, bestimmt, ob die Bestandsabbilder **byte-identisch** bleiben (textuelle Einbindung: ja; Funktionsauslagerung: nicht zuverlässig). Die S3-Spec nennt das Mittel nicht | `design.md` §9.1: Klasse H2 zuerst, Klasse F nur mit Begründung |
| **W7** | F13: Versionierung des S3-Abbilds | ✔ Das Abbild enthält STM-Logik **und** Netz-Logik; ändert sich nur `esp32s3/**`, ändert sich **weder** `VERSION` noch `ESP_VERSION` (DIR-004: Quellen je Komponente). Das Fabrikat wäre unzuordenbar | **Frage Q2** |
| **W8** | Tasks 5.1/5.3: `STM32F103`-Nachweis | ✔ Die engste Variante hat 1 172 Byte frei (●, 13.10.2026). Ein Nachweis „am STM“ trifft den **F411** an der Test-Uhr; der **F103** hat nach meiner Kenntnis **kein** Prüfgerät (○, **Frage Q3**) | Beim F103 trägt **nur** der Hash-Vergleich (AK13) |
| **W9** | Task 0.3: `s3-developer` besitzt `esp32s3/**` | Gut. Zusätzlich braucht der S3 ein **Board-Verzeichnis** (`boards/<id>/`), und `boards/modules.txt` mit der Baseline hat einen **eigenen Besitzer** (Lead). Der Besitz-Hook kennt `boards/` und `esp32s3/` nicht (✔ Wurzelmuster `tools/hooks/file-ownership.py:109`) | `tasks.md` Task 0.3 |

## Betroffene Laufzeiten

- [x] STM32 (`src/**`) — **Neu-Flashen nötig, wenn Stufe 3 eine Änderung der Klasse F enthält**; bei **nachgewiesener
      Binärgleichheit** (Klasse H1/H2) ist das Fabrikat **dasselbe Abbild**, und es gibt **nichts zu flashen**
      (Frage Q9: Versionspflicht in diesem Fall)
- [x] ESP8266 (`ESP8266/ESP-uclock/*.cpp`) — Quelle der Netz-Seite für **beide** Ziele; Änderungen sind `#ifdef ESP32`-Zweige
      (S3-Spec Tasks 5.3a–c); **Neu-Flashen nötig** erst dort
- [ ] PWA (`data/app/**`) — **nicht berührt** (V4)
- [x] Build/Release — neue Dateien `boards/**`, Prüfungen unter `tools/**`, Guardrails, Hook-Registrierung, Matrix-Vergleich,
      ein weiteres Make-Ziel für den S3 (`release-engineer`)
- [x] **ESP32-S3** (neu) — nur über die S3-Spec; diese Spec stellt die Quelle bereit

## Offene Fragen — nicht geraten

Jede Frage nennt, **wer sie beantwortet**, **woran** man es tut und **was bis dahin gilt**.

| # | Frage | Beantwortet durch | Bis dahin |
|---|---|---|---|
| **Q1** | **Reichweite der Board-Beschreibung.** Empfehlung: Der S3 bekommt **sofort** `boards/<id>/board.h`; die Pins der Bestandsboards wandern **erst in Stufe 5** (nach dem ersten Licht auf dem S3) und **nur mit Hash-Nachweis** aus den rund 20 `#if`-Ketten dorthin (grobe Zählung: BlackPill 134, BluePill 123, Nucleo 76 `#define`-Zeilen in Board-Zweigen). Soll Stufe 5 überhaupt kommen, und wenn ja, vor oder nach dem Nachbau-Test? | Nutzer | Stufe 5 ist **nicht freigegeben**; der Zähler (AK4) friert den Bestand ein |
| **Q2** | **Versionierung des S3-Abbilds** (S3-Spec F13, hier W7). Optionen: (a) ein **drittes Symbol** `S3_VERSION` im selben `#define`-Format, bumpt bei jeder Änderung an `esp32s3/**`, `boards/esp32s3-*` **oder** der eingebundenen Quellen; `VERSION`/`ESP_VERSION` laufen weiter als Komponentenversionen. Preis: ein neues Makefile-Ziel, eine Zeile in S4, ein Tag-Schema. (b) `ESP_VERSION` für das S3-Abbild mitführen: zwingt jeden STM-Commit zu einem ESP8266-Bump → **offeriert ein OTA auf identische ESP8266-Firmware** (DIR-004 verbietet das). **Empfehlung (a).** | Nutzer | keine; S3-Release ist blockiert |
| **Q3** | **Gibt es ein F103-Prüfgerät** (BluePill mit LED-Board)? Wenn nein, sind F103 und alle 70 Nicht-Test-Uhr-Varianten **nur über den Hash** belegt. | Nutzer | Der Hash trägt allein; **keine** Klasse-F-Änderung an Dateien, die der F103 baut, ohne ausdrückliche Freigabe |
| **Q4** | **Chip-Code des S3 im Konfigurationswort** (`hardware_configuration`, 3 Bit `HW_STM32_MASK`, belegt sind 0–6; **7 ist „Reserved“**, ✔ `main.h:23-30`). Die Konstanten stehen **zweimal** (STM: `main.h`, ESP: `http.cpp`/`vars.h`); der ESP leitet daraus **den Dateinamen für das STM-Flashen** ab, die PWA zeigt den Namen. Optionen: (a) Code 7 für den S3 verwenden und beide Abschriften nachziehen; (b) das Feld erweitern (**bricht** das Wortformat zwischen Brücke, PWA und Update-Server). **Empfehlung (a)**, Name und Wert nach Entscheid des Nutzers | Nutzer, dann `stm-developer` und `esp-developer` | S3 meldet **keinen** gültigen Chip-Code; `smoke-device.sh` darf ihn nicht als F411 deuten |
| **Q5** | **Toolchain-Stand auf dem Rechner.** ✔ Auf diesem Rechner liegen **Arduino-ESP32 3.3.12 mit IDF 5.5.5** (`~/Library/Arduino15/packages/esp32`, `esp32s3-libs/3.3.12`, `esp-x32`, `openocd-esp32`), aber **kein `idf.py`** (`which idf.py`: nicht gefunden). **Ich empfehle ESP-IDF 5.5.x mit Arduino-ESP32 als Komponente** (`design.md` §8.1, mit Preis). Wer installiert IDF, und in welcher Version? | Lead (Installation), Nutzer (Freigabe der Versionsbindung) | Entwurf rechnet mit IDF 5.5.x, **○ nicht gebaut** |
| **Q6** | `ESP8266/ESP-uclock/` trägt künftig auch den ESP32-Build der Netz-Seite. Umbenennen? **Empfehlung: nein**, ausserhalb dieser Spec. | Nutzer | bleibt |
| **Q7** | **Format von `hardware/architektur/pinbelegung.md`.** Die Prüfung AK9 braucht ein **maschinenlesbares** Gegenstück (Tabelle mit festen Spalten **oder** eine JSON-Datei neben dem Dokument). `hw-architect` hat es noch nicht geschrieben. | `hw-architect` | AK9 prüft vorerst nur `board.h` gegen die Sperrliste und die Signalzahl (**11**) |
| **Q8** | **Welche Taktung der Zeitbasis am S3** (`F_INTERRUPTS`). ✔ Das Makro steht hinter `#ifndef` in `irmpconfig.h:29-30` und ist damit **per Übersetzer-Schalter** setzbar, ohne die Datei zu ändern. Den Wert klärt S3-Spec F14 | `stm-developer`, `firmware-analyst` (S3-Task 0.4) | Der S3 setzt ihn aus `board.h`; Bestandsboards bleiben 15 000 |
| **Q9** | **Versionspflicht bei nachweislich binärgleicher Umstellung.** DIR-004: „Code geändert ohne Bump macht das Fabrikat unzuordenbar, Bump ohne Codeänderung bietet ein OTA-Update auf identische Firmware an.“ Ist die Umstellung **hash-gleich**, ist das Fabrikat **identisch**. Guardrail S4 würde trotzdem „Code seit Tag geändert, Version unverändert“ melden. Darf in diesem Fall **ohne Bump** committet werden (S4 lernt den Hash-Nachweis), oder bumpt man bewusst? **Empfehlung: ohne Bump, mit Nachweis im Commit; S4 um die Ausnahme erweitern.** | Nutzer (Direktiven nur mit Bestätigung) | **Bumpen** (sicherer Weg), bis entschieden |
| **Q10** | **`boards/**` als Quelle der STM-Komponente.** DIR-004 nennt die STM-Quellen: `src/**`, `CMakeLists.txt`, `cmake/**` (✔ `tools/guardrails.sh:80`: `STM_SRC="src CMakeLists.txt cmake"`). Sobald `boards/<stm>/board.h` in den STM-Bau eingeht (Stufe 5), muss `boards/**` in die Liste, sonst ändert sich Code **ohne** Versionspflicht. | Nutzer (DIR-004), `Lead` (S4) | gilt erst mit Stufe 5 |
| **Q11** | **Netz-Seite des S3 und die STM-Endpunkte** (S3-Spec F10) und die Steuerpins zum zweiten Chip (`esp8266_reset/_powerdown/_powerup/_flash`, ✔ `esp8266.c:91ff.`): Auf dem S3 gibt es **keinen** zweiten Chip. `design.md` §3.4 führt sie als eigene Naht „Gegenstelle“; wie die Funktionen antworten, bleibt S3-Spec F10. | Nutzer | S3-Spec F10 |
| **Q12** | **Frist im `refresh`-Wait.** Der gemeinsame LED-Kern wartet auf das vorige Bild **ohne Timeout und ohne `watchdog_reload()`** (✔ Inventar §4: `sk6812.c:470-496`); die S3-Spec sieht dort eine Frist vor (`rmt_tx_wait_all_done` mit Timeout). Eine Frist im **gemeinsamen** Kern ist **Klasse F** für alle 72 Varianten. Optionen: (a) Frist **nur im S3-Backend** (`sk6812_dma_status` wird bei Zeitüberschreitung vom Backend gelöscht, Kern unverändert); (b) Frist im Kern (F, Gerätetest, F103-Flash). **Empfehlung (a).** | `stm-developer`, `firmware-analyst`, Nutzer | der Kern bleibt **unverändert**; die Frist gehört dem Backend |
| **Q13** | **Welches Layout bedient das S3-Abbild?** Die Bestands-Release-Ziele bauen **12h** (`WCLOCK24H=0`, ✔ `CMakeLists.txt`: `WCLOCK_MODE_DEFINE WCLOCK24H=0`); die Matrix baut beide. Eine Platine mit 114 + 120 LEDs ist das 12h-Layout (`HARDWARE.md`, ●). Ein Abbild je Layout oder eines? | Nutzer | **ein** Abbild, 12h, Annahme ○ |

## Schreibregeln für die Umsetzung

Diese Spec ist Deutsch mit echten Umlauten (DIR-001). **Die Quelldateien sind es nicht** (DIR-015): `src/**` ist überwiegend
ASCII oder ISO-8859-1 (✔ acht der 125 C-Dateien sind ISO-8859-1: `base.c`, `base.h`, `display.c`, `ds18xx.c`, `irmp.c`, `rtc.c`,
`tempsensor.c`, `w25qxx.c`; `grep` braucht `-a`), `http.cpp` und `stm32flash.cpp` sind UTF-8. **Vor jedem Patch Kodierung und
Zeilenende feststellen.** Neue Dateien unter `boards/**`, `esp32s3/**` und `tools/**` sind ASCII, Kommentare in Umschrift. **Ein
Patch, der Quelltext umordnet, muss byte-genau sein** — ein Werkzeug, das Zeilenenden vereinheitlicht, macht aus einem
hash-gleichen Umbau einen unlesbaren (CLAUDE.md, 04.10.2026).
