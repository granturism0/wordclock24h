# Changelog

## 2026-04-09 Finalized Restore Release

Aktueller verifizierter Abschlussstand:

- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.43`

Wichtige Punkte:

- Backup/Restore läuft jetzt wieder sauber mit automatischem `STM32`-Reset und anschließendem PWA-Reload
- `Update-Host/-Pfad` werden nach dem Boot nicht mehr durch ESP-Defaults auf den STM zurückgeschrieben
- `Zeitserver`, `Zeitzone`, `Sommerzeit`, `RTC`- und `DS18xx`-Korrektur bleiben nach dem Restore und Reboot erhalten
- Overlay-Restore ist stabil
- die temporären `EEPDBG`-Diagnoseausgaben wurden wieder entfernt
- harmlose Protokollreste für obsolete/TFT-fremde Variablen wurden bereinigt

## 2026-04-08 Current Working Release

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-08-2131.zip`

Ergänzungen gegenüber den früheren 2026-04-08-Ständen:

- PWA-Backup/Restore weiter stabilisiert
- Overlay-Restore repariert
- Asset-/Layout-Restore weiter gehärtet
- Netzwerk-/Zeiteinstellungen werden beim Restore nochmals verifiziert
- `Ambilight online/offline` wird nach dem `STM32`-Reset nochmals als Laufzeitstatus gesetzt
- Update-/Flash-Aktionen merken jetzt die Scroll-Position und springen bei Erfolg sauber zurück
- PWA-Quelldateien haben jetzt einheitliche Dateikopf-Kommentare

Wichtige technische Hinweise:

- Der Overlay-Fix liegt in [vars.cpp](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/vars.cpp): `set_overlay_var()` macht jetzt am Ende ein `Serial.flush()`.
- Der Live-Farbpfad für `Rainbow` wurde in [display.c](/Users/daniel/Documents/GitHub/wordclock24h/src/display/display.c) gedrosselt:
  - kein Versand bei jedem einzelnen Farbschritt mehr
  - stattdessen nur noch höchstens einmal pro Sekunde an den ESP
- `Daylight` sendet die Live-Farbe weiterhin nur beim echten Stundenwechsel.
- PWA-Version dieses Stands: `1.2.28`

## 2026-04-08 F411 SK6812 Output Fix

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-08-0031.zip`

Ergänzungen gegenüber dem Stand vom 2026-04-07:

- SK6812-Treiber für `STM32F411CE BlackPill` korrigiert
- `TIM3 / PB1` nutzt auf dem BlackPill jetzt konsistent `TIM3_CH4`
- vollständiger Release-Workflow erneut verifiziert über `make release-zip`

Wichtiger technischer Hinweis:

- Der Fehler lag im SK6812-Treiber in [src/sk6812/sk6812.c](/Users/daniel/Documents/GitHub/wordclock24h/src/sk6812/sk6812.c).
- Für `BLACKPILL_BOARD` war das DMA-/GPIO-Mapping bereits auf `TIM3_CH4 / PB1` ausgelegt, die Timer-Output-Compare-Initialisierung lief aber noch fest über `TIM_OC1...`.
- Dadurch konnte auf `STM32F411CE BlackPill` trotz korrekter Pinbelegung kein gültiges SK6812-Ausgangssignal auf `PB1` entstehen.
- Der Treiber verwendet jetzt die zur Board-Konfiguration passende OC-Initialisierung pro Kanal.
- Ein externer Pull-up kann das Verhalten zusätzlich beeinflussen, weil ein SK6812-Eingang an `5V` Versorgung mit reinem `3.3V`-High am Datenpin im Grenzbereich liegen kann.

## 2026-04-06 Stable Baseline

Verifizierter stabiler Referenzstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-06-2339.zip`

Inhalt dieses Basisstands:

- funktionierender `12h RGBW`-STM-Build für `STM32F103`
- funktionierender `12h RGBW`-STM-Build für `STM32F411CE 25 MHz`
- ESP8266-Build als `ESP-WordClock-4M.bin`
- versioniertes `app-bundle.txt`
- gemeinsamer Release-Workflow über `make release-zip`

Wichtiger technischer Hinweis:

- Die Uhranzeige auf der Hardware funktioniert in diesem Stand wieder sauber.
- Der kritische Rückbau erfolgte in [src/display/display.c](/Users/daniel/Documents/GitHub/wordclock24h/src/display/display.c), damit die STM-Farbpfade wieder dem funktionierenden Verhalten aus `3.1.5` entsprechen.
- Zusätzliche `var_send_display_colors();`-Aufrufe direkt in `display_init_color_animation_rainbow()` und `display_init_color_animation_daylight()` wurden als instabil verifiziert und bleiben in diesem Basisstand bewusst draußen.
- Änderungen in diesem Bereich sollten künftig nur schrittweise und testbar wieder eingeführt werden.

## 2026-04-07 Stable Runtime Color Hook

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-07-0123.zip`

Ergänzungen gegenüber der Basis:

- PWA-Vorschau-Refresh sauber auf echte 5-Sekunden-Grenzen synchronisiert
- Auto-Refresh mit `+1.0s` Versatz, damit Minutenpunkte nach dem echten Umschalten zuverlässig erfasst werden
- Live-Farbpfad für `Rainbow` und `Daylight` über sichere Laufzeitstellen wieder aktiviert
- `/app`-Startpfad und automatische Erstinstallation der PWA auf dem ESP weiter gehärtet
- Status- und Fehlerseiten der Legacy-/Autoinstallationspfade sprachlich bereinigt und mit echten Umlauten versehen
- PWA-Kernladepfad auf echte Pflichtdaten reduziert, langsamere Nebenpfade werden nachgeladen

Wichtiger technischer Hinweis:

- `var_send_display_colors();` funktioniert stabil im laufenden `Rainbow`-Pfad
- `var_send_display_colors();` funktioniert stabil beim echten `Daylight`-Stundenwechsel
- `var_send_display_colors();` in den Init-Funktionen von `Rainbow` und `Daylight` bleibt weiterhin bewusst deaktiviert
- dieser Stand ist die aktuelle Referenz für funktionierende Live-Farben ohne Hardware-Ausfall
- PWA-Version dieses Stands: `1.2.11`
