# Changelog

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
