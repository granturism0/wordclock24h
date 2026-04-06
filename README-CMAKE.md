# STM-Build mit CMake

Dieses Setup baut die STM-Firmware außerhalb von EmBitz für:

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
- PWA-Version dieses Stands: `1.2.11`
- die instabilen Init-Aufrufe bleiben weiterhin bewusst draußen

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
