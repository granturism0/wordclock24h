# STM-Build mit CMake

Den aktuellen Versionsstand aller drei Komponenten zeigt `./tools/guardrails.sh`
in Stufe S4. Er wird hier bewusst **nicht** wiederholt — die Angabe war über
Monate falsch, weil sie niemand nachgezogen hat.

Dieses Setup baut die STM-Firmware ausserhalb von EmBitz für:

- `STM32F103` mit `SK6812 RGBW`
- `STM32F411` mit `SK6812 RGBW`

Aktuell bewusst nur als `12h`-Variante.

Die Quelllisten werden direkt aus den bestehenden EmBitz-Projektdateien gelesen:

- `wclock24h-F103/wclock24h-F103.ebp`
- `wclock24h.ebp`

## Voraussetzungen

Im `PATH` verfügbar:

- `cmake`
- `arm-none-eabi-gcc`
- `arm-none-eabi-g++`
- `arm-none-eabi-objcopy`
- optional `arm-none-eabi-size`

Optional kann auch gezielt eine bestimmte Arm-Toolchain verwendet werden, zum Beispiel die ältere EmBitz-kompatible `9.3.1`:

```bash
make f103 ARM_TOOLCHAIN_ROOT="/pfad/zur/toolchain"
make f411 ARM_TOOLCHAIN_ROOT="/pfad/zur/toolchain"
make release-zip ARM_TOOLCHAIN_ROOT="/pfad/zur/toolchain"
```

Der Pfad muss auf das Toolchain-Wurzelverzeichnis zeigen, das `bin/arm-none-eabi-gcc` enthält.

## Konfigurieren

```bash
cmake --preset stm-rgbw-12h
```

## Bauen

Beide Targets:

```bash
cmake --build --preset stm-rgbw-12h
```

Nur `STM32F103 RGBW`:

```bash
cmake --build --preset stm-rgbw-12h --target wordclock_f103_rgbw
```

Nur `STM32F411 RGBW`:

```bash
cmake --build --preset stm-rgbw-12h --target wordclock_f411_rgbw
```

Kurzbefehle über `make`:

```bash
make f103
make f411
make all
make esp
make app-bundle
make release-zip
make clean
```

## ESP8266-Build

Der ESP-Build nutzt die mitinstallierte `arduino-cli` aus der Arduino IDE und kompiliert die Firmware aus:

- `ESP8266/ESP-uclock`

Verwendete Board-Einstellungen aus `ESP-uclock.ino`:

- `Generic ESP8266 Module`
- `115200`
- `80 MHz`
- `26 MHz`
- `40 MHz`
- `DOUT`
- `4M (FS:1MB OTA:~1019KB)`
- `Debug port: Disabled`
- `Debug Level: None`
- `Builtin Led: 2`
- `Reset Method: dtr (aka nodemcu)`
- `lwIP: v2 Lower Memory`
- `VTables: Flash`
- `SSL: All SSL ciphers`
- `MMU: 32KB cache + 32KB IRAM`
- `Non-32-Bit Access: Use pgm_read macros`
- `Erase Flash: Only Sketch`

Build:

```bash
make esp
```

Artefakte liegen danach unter:

- `build/esp8266/`

Wichtigste Dateien:

- `ESP-WordClock-4M.bin`
- `ESP-WordClock-4M.elf`
- `ESP-WordClock-4M.map`

## Release-ZIP

Für ein gemeinsames Paket mit den wichtigsten Artefakten:

```bash
make release-zip
```

Das baut bzw. aktualisiert:

- `ESP8266/ESP-uclock/data/app-bundle.txt`
- `build/stm-rgbw-12h/wc12h-stm32f103-sk6812-rgbw.hex`
- `build/stm-rgbw-12h/wc12h-stm32f411ce-25-sk6812-rgbw.hex`
- `build/stm-rgbw-12h/wc.txt`
- `build/esp8266/ESP-WordClock-4M.bin`
- `build/esp8266/ESP-WordClock.txt`

und packt sie als ZIP unter:

- `build/releases/wordclock-release-YYYY-MM-DD-HHMM.zip`

Zusätzlich werden die Versionsdateien auch bei Einzelbuilds erzeugt:

- STM-Builds: `build/stm-rgbw-12h/wc.txt`
- ESP-Build: `build/esp8266/ESP-WordClock.txt`

## Bekannte gute Basis

Der aktuell verifizierte funktionierende Referenzstand ist:

- `build/releases/wordclock-release-2026-04-06-2339.zip`

Wichtig für diesen Stand:

- die Uhr läuft auf der Hardware wieder sauber
- `src/display/display.c` entspricht für die kritischen STM-Farbpfade wieder dem funktionierenden `3.1.5`-Verhalten
- zusätzliche `var_send_display_colors();`-Aufrufe in den Init-Funktionen von `Rainbow` und `Daylight` sind aktuell bewusst nicht enthalten, weil sie auf echter Hardware direkt wieder zu einem Ausfall geführt haben
- spätere Anpassungen in genau diesem Bereich sollten nur noch einzeln und kontrolliert wieder eingeführt werden

Wenn später erneut ein Anzeigeproblem auftritt, ist dieses ZIP die erste saubere Vergleichsbasis.

Der aktuell verifizierte Arbeitsstand mit sicheren Live-Farben und synchronisiertem PWA-Refresh ist:

- `build/releases/wordclock-release-2026-04-07-0123.zip`

Wichtig für diesen Stand:

- Live-Farben laufen stabil über sichere Laufzeit-Hooks in `src/display/display.c`
- `Rainbow`: Farbversand nur im laufenden Updatepfad
- `Daylight`: Farbversand nur beim echten Stundenwechsel
- PWA-Vorschau zieht im 5-Sekunden-Raster synchron zu echten Zeitgrenzen nach
- der Auto-Refresh läuft mit `+1.0s` Versatz, damit die Minutenpunkte nach der echten Umschaltung sicher erfasst werden
- der `/app`-Pfad prüft die PWA-Vollständigkeit robust und zeigt bei fehlenden Dateien den Lade-/Fehlerflow statt einer halben App
- Status- und Fehlerseiten des Legacy-/Autoinstallationspfads sind bereinigt und mit echten Umlauten versehen
- PWA-Version dieses Stands: `1.2.11` <!-- historisch -->
- die instabilen Init-Aufrufe bleiben weiterhin bewusst draussen

Der damals verifizierte Arbeitsstand vom 8. April 2026 mit stabilem Restore, Overlay-Fix und Scroll-Rücksprung nach Update-Aktionen war:

- `build/releases/wordclock-release-2026-04-08-2131.zip`

Wichtig für diesen Stand:

- PWA-Version: `1.2.28` <!-- historisch -->
- Overlay-Restore ist repariert
  - Ursache war fehlendes `Serial.flush()` in [vars.cpp](/ESP8266/ESP-uclock/vars.cpp) bei `set_overlay_var()`
- der laufende Live-Farbpfad für `Rainbow` ist in [display.c](/src/display/display.c) gedrosselt
  - Live-Farbversand nicht mehr bei jedem einzelnen Farbschritt
  - stattdessen höchstens einmal pro Sekunde
- `Daylight` sendet die Live-Farbe weiterhin nur beim echten Stundenwechsel
- Update-/Flash-Aktionen der PWA merken die Ausgangsposition und springen bei Erfolg an die ursprüngliche Scroll-Position zurück
- PWA-Quelldateien unter `ESP8266/ESP-uclock/data/app` tragen jetzt einheitliche Dateikopf-Kommentare

## Ausgaben

Die Artefakte liegen unter:

- `build/stm-rgbw-12h/`

Pro Target werden erzeugt:

- `.elf`
- `.hex`
- `.bin`
- `.map`

Konkret für die `.hex`-Dateien:

- `wc12h-stm32f103-sk6812-rgbw.hex`
- `wc12h-stm32f411ce-25-sk6812-rgbw.hex`

## Aktuelle Annahmen

- `STM32F103`: `BluePill`, `HSE_VALUE=8000000`
- `STM32F411`: `BlackPill`, `HSE_VALUE=25000000`, Linkerskript `stm32f411ce_flash.ld`
- LED-Typ fest auf `SK6812_RGBW_LED`

Wenn du später weitere Varianten brauchst, können wir darauf aufbauend zusätzliche CMake-Presets oder Optionen ergänzen.

## Werkzeuge rund um den Build

Neben den `make`-Zielen gibt es drei Werkzeuge unter `tools/`.

### `./tools/guardrails.sh`

Prüfungen, die nach jedem Task laufen und vor einer Übergabe bestehen müssen. Acht
Stufen, Laufzeit Sekunden, ohne zusätzliche Abhängigkeiten: Syntax, undefinierte
Funktionsaufrufe, i18n-Schlüssel, Versionspflicht, `.gz`-Artefakte, ungenutzte
CSS-Klassen, Muster aus `knowledge/quick-reference.md`, Kodierung der C-Quellen und ein
Smoke-Test, der `app.js` mit gestubbtem Browser-Umfeld lädt.

`--full` ergänzt die Compile-Smoke-Tests `f103`, `f411` und `esp`. Exit 0 heisst: keine
Kritisch-Findings.

### `python3 tools/preview/server.py 8099`

Liefert die PWA aus und simuliert die Geräte-API, sodass die Oberfläche ohne erreichbare
WordClock betrachtet und vermessen werden kann. `./tools/preview/shot.sh` rendert sie in
mehreren Geräteklassen. Grenzen in `tools/preview/README.md` — das ersetzt keinen Test
am Gerät.

### `./tools/deploy.sh`

Rollt das fertige Release auf den Update-Server aus. Prüft vorher jedes Artefakt auf
Vorhandensein, Grösse grösser null und veraltete `.gz`, und bricht ab statt einen
kaputten Stand auszuliefern. `--dry-run` zeigt die Dateiliste, ohne etwas zu schreiben.
Zugangsdaten in `tools/deploy.conf`, Vorlage daneben.

Das Skript **löscht nichts** — auf dem Ziel liegen Layout-Tabellen und Listendateien,
die nicht Teil des Releases sind.

## ESP-Build auf Apple Silicon

Auf diesem Rechner (macOS 27, arm64) bricht `make esp` ab mit
`bad CPU type in executable`. Die gesamte ESP8266-Werkzeugkette ist x86_64 und
braucht **Rosetta 2**, das hier nicht installiert ist. Der Paketindex bietet keine
native arm64-Toolchain, und 3.1.2 ist die neueste Version.

```
softwareupdate --install-rosetta
```

Das betrifft alle Wege gleichermassen — Makefile, `arduino-cli`, VS Code und die
Arduino IDE gehen über dieselben Binärdateien.

**Bereits erledigt:** Das mitgelieferte Python der Plattform ist durch einen Wrapper
auf `/usr/bin/python3` ersetzt, weil es ebenfalls x86_64 ist. Die Build-Skripte der
Plattform verwenden ausschliesslich Standardmodule, das native System-Python genügt.
Das Original liegt als `python3.x86_64.orig` daneben.

> Ein Update der esp8266-Plattform überschreibt den Wrapper. Datei:
> `~/Library/Arduino15/packages/esp8266/tools/python3/3.7.2-post1/python3`,
> Inhalt: `#!/bin/sh` und `exec /usr/bin/python3 "$@"`.

**Solange der ESP unverändert bleibt, ist das kein Hindernis:** Nach DIR-004 wird er
dann ohnehin nicht neu versioniert, und die vorhandene `.bin` aus `build/esp8266/`
bleibt gültig. `tools/deploy.sh` rollt sie unverändert mit aus. Erst eine echte
Änderung an der ESP-Firmware verlangt einen neuen Build.

## Versionierung

Jede Komponente wird **genau dann** versioniert, wenn sich ihr Code geändert hat —
kein Gleichschritt. Ändert ein Release nur den STM-Code, steigt nur `VERSION` in
`src/main.h`; `ESP_VERSION` und `APP_VERSION` bleiben stehen.

| Geändert | Anheben |
|---|---|
| `src/**`, `CMakeLists.txt`, `cmake/**` | `VERSION` in `src/main.h` |
| `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` | `ESP_VERSION` in `version.h` |
| `data/app/**` (ohne `.gz`) | `APP_VERSION` **und** `CACHE_NAME` |

Ein Bump ohne Codeänderung ist ebenso falsch wie eine Änderung ohne Bump: die Uhr
bietet dann ein OTA-Update auf identische Firmware an. `APP_VERSION` und `CACHE_NAME`
gehören dagegen immer zusammen — ohne `CACHE_NAME`-Bump liefert der Service Worker
neue `index.html` mit alter `app.js`.

`tools/deploy.sh` setzt nach jedem Rollout ein Tag `release/<stm>-<esp>-<app>`. Die
Guardrails messen die Versionspflicht gegen dieses Tag, nicht gegen den letzten Commit.
