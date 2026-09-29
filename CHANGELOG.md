# Changelog

## 2026-09-30 Regeln maschinell erzwungen, Ablaeufe als Skills

Reine Werkzeug- und Dokumentationsaenderung, kein Produktcode, deshalb kein Versionsbump.

Nach einer Recherche zum aktuellen Stand der Claude-Code-Funktionen umgesetzt. Drei
Faehigkeiten waren ungenutzt, und alle drei zielen auf Probleme, die dieses Projekt hat.

### Regeln, die jetzt greifen statt nur dazustehen

- **R1 "nur der Lead baut"** haengt als PreToolUse-Hook im Frontmatter aller
  schreibenden Agenten ausser dem `release-engineer`. Abgewiesen werden `make`, `cmake`,
  `arduino-cli` und `guardrails.sh --full`; harmlose Ziele wie `make stm-version-file`
  bleiben erlaubt. Grund: f103 und f411 bauen beide mit -j4 in dasselbe Verzeichnis,
  parallele Builds korrumpieren den CMake-Cache.
- **Ein Stop-Hook** blockiert das Turn-Ende, wenn ueberwachte Dateien geaendert sind und
  die Guardrails dafuer nicht liefen. Drei Absicherungen verhindern Dauerblockaden:
  `stop_hook_active`, ein Hash pro Aenderungsstand (pro Stand wird hoechstens einmal
  blockiert) und ein Durchlassen bei jedem Fehler im Hook selbst. Die Hashberechnung
  liegt bewusst nur an einer Stelle — zwei Fassungen wuerden auseinanderlaufen und
  dauerhaft blockieren.

### Ablaeufe als Skills

`CLAUDE.md` war auf 248 Zeilen gewachsen und lud bei jeder Sitzung komplett, auch die
STM-Interna waehrend PWA-Arbeit. Die Ablaeufe liegen jetzt unter `.claude/skills/`:

- `/release` — Build, Versionspflicht, Rollout, was zu flashen ist
- `/pwa-vorschau` — PWA ohne Geraet ansehen, mit den drei Chrome-Eigenheiten
- `/doku-nachfuehren` — CHANGELOG, READMEs, Befundkatalog
- `stm-firmware` — belegtes Detailwissen zur Firmware, laedt automatisch ueber
  `paths: src/**`

`CLAUDE.md` ist damit auf 178 Zeilen. Die Kurzregeln bleiben dort, weil sie immer
gelten; die Prozeduren laden nur bei Bedarf.

### Weiteres

- Die zwoelf Agenten haben Farben nach Rolle: Firmware orange, PWA blau, nur lesend
  violett, pruefen und bauen gruen, Doku und Spec cyan.
- Projektweite Berechtigungen in `.claude/settings.json` statt verstreut und mit
  absoluten Pfaden in der lokalen Datei. Bewusst **ohne** `deny` auf `deploy.sh` — eine
  deny-Regel ist nicht ueberstimmbar, und der Rollout soll moeglich sein und nur
  nachfragen.
- S9 prueft jetzt auch die Skills.

## 2026-09-29 Versionierung je Komponente, Befundkatalog, Doku-Pruefungen

Reine Werkzeug- und Dokumentationsaenderung. **Kein Produktcode beruehrt, deshalb kein
Versionsbump** — genau der Fall, den die neue Regel beschreibt.

### Geaendert

- **Versionspflicht DIR-004 von Gleichschritt auf komponentenweise umgestellt.** Jede
  Komponente wird genau dann versioniert, wenn sich ihr Code geaendert hat. Aendert ein
  Release nur den STM-Code, steigt nur dessen Version. Vorher wurden alle drei bei jedem
  Build angehoben, was ein OTA-Update auf identische Firmware zur Folge hatte.
- Guardrail-Stufe S4 prueft das jetzt **je Komponente und in beiden Richtungen** — auch
  ein Bump ohne Codeaenderung ist ein Befund. Entscheidend war die Trennung von ESP und
  PWA: beide liegen unter `ESP8266/`, sind aber getrennt versioniert. Die vorherige
  Pruefung hielt jede PWA-Aenderung fuer eine ESP-Aenderung.
- Bei der Aenderungserkennung wird die Versionszeile aus dem Diff ihrer eigenen Datei
  gefiltert. Ohne das waere jeder Bump fuer sich schon eine "Codeaenderung" und die
  Gegenprobe koennte nie anschlagen.

### Neu

- **`BEFUNDE.md`** — lebender Massnahmenkatalog. Die beiden Reviews sind Momentaufnahmen
  und werden nicht fortgeschrieben; ihr Stand lebt jetzt an einer Stelle: alle 34
  Massnahmen mit Status und nachpruefbarem Beleg, dazu die Befunde aus der laufenden
  Arbeit unter eigenen `L`-Nummern.
- **Guardrail S9** — Aktualitaet der lebenden Dokumentation. Vergleicht Versionsangaben
  gegen die Quellen und meldet absolute Benutzerpfade.
- **Guardrail S10** — Vollstaendigkeit des Katalogs. Schlaegt an, wenn eine
  Massnahmennummer aus einem Review in `BEFUNDE.md` fehlt oder die `L`-Nummerierung eine
  Luecke hat.

### Behoben in der Dokumentation

- Der Kopf von `README-CMAKE.md` behauptete einen "aktuellen Abschlussstand", der rund
  dreissig PWA-Versionen zurueck lag. Versionsangaben aus den lebenden Dokumenten
  entfernt statt gepflegt — was nicht dasteht, kann nicht veralten.
- Sieben absolute Pfade `/Users/<name>/...` in `CHANGELOG.md`, `README-CMAKE.md` und
  `.claude/settings.json`. Sie zeigen bei jedem anderen Klon ins Leere.
- 14 Stellen mit `ss`-Verstoss gegen die Schweizer Schreibung in vier lebenden Dokumenten.
- Ein Abschnitt in `README-CMAKE.md` beschrieb einen Stand vom April als "aktuell
  verifiziert".

## 2026-09-29 Restore-Luecke, Guardrails und Werkzeugschicht

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-09-29-2341.zip`
- STM32: `3.2.6` · ESP8266: `3.2.2` · PWA: `1.4.70` · SW-Cache: `wordclock-app-v62`

### Behoben

- **Display blieb nach einem Wetter-Ticker bis zu 60 Sekunden dunkel**, bei
  `WCLOCK24H == 0` bis zu fuenf Minuten. Ursache: `pending_weather_ticker_restore` wurde
  unbedingt geloescht, `UPDATE_ALL` aber nur gesetzt, wenn gerade kein anderes Update
  anstand. Da die Flags disjunkte Bits sind, ging das Restore in diesem Fall verloren.
  Ausgeloest wurde es aus der PWA durch das Speichern der Dimmkurve, ohne PWA durch die
  Helligkeitsautomatik.
- **Zwei Aufrufe nicht vorhandener Funktionen in der PWA.** Beim Layout-Tabellen-Upload
  wirkte der Button tot, ohne Fehlermeldung. Bei laufender Farbanimation fror die
  WordClock-Vorschau auf der alten Farbe ein.
- **Drei fehlende Uebersetzungsschluessel** standen woertlich als `common.setting` auf
  Buttons, in beiden Sprachen. Dazu 14 Schluessel, die in der englischen Oberflaeche
  deutschen Text zeigten.
- **Release-Notes werden nicht mehr ungefiltert eingesetzt.** Der Inhalt kommt vom
  konfigurierbaren Update-Host ueber HTTP. Ersetzt durch einen Whitelist-Filter, der den
  Baum aus frisch erzeugten Elementen neu aufbaut; Attribute werden nicht uebernommen.
- **Drei stille Fehlerschlucker.** Beim Overlay-Loeschen im Backup-Import wird die Zahl
  fehlgeschlagener Plaetze jetzt sichtbar gemeldet statt verschluckt.

### Neu: Messung statt Vermutung

- **`do_display_icon`-Freeze wird sichtbar.** Wird das Display ausgeschaltet, waehrend
  ein Icon laeuft, bleibt ein Flag dauerhaft gesetzt — mit der Folge, dass
  Temperatur-Restore, Wetter-Ticker-Restore und Helligkeitsautomatik einfrieren, solange
  das Display aus ist. Eingebaut ist **nur die Messung**, kein Fix: eine Logzeile bei
  Zustandswechsel. Erscheint sie beim Ausschalten, ist der Freeze belegt; bleibt sie aus,
  ist er widerlegt.

### Neu: Werkzeuge

- **`./tools/guardrails.sh`** — acht Pruefstufen, Laufzeit Sekunden, ohne neue
  Abhaengigkeiten. Findet unter anderem undefinierte Funktionsaufrufe, fehlende
  i18n-Schluessel, veraltete `.gz` und ungefiltertes `innerHTML`.
- **`tools/preview/`** — die PWA laeuft ohne Geraet, inklusive Vermessung ueber mehrere
  Bildschirmgroessen. Ersetzt keinen Test am Geraet, faengt aber Layoutfehler ab.
- **`./tools/deploy.sh`** — Rollout auf den Update-Server. Prueft jedes Artefakt, bevor
  es etwas ueberträgt, und loescht auf dem Ziel nichts.

### Bekannt und bewusst offen

- Die unbedingten Logausgaben im Refresh-Pfad und das fehlende `watchdog_reload()` in den
  blockierenden Pfaden bleiben. Beide wuerden das Timing veraendern und damit die
  `do_display_icon`-Messung unbrauchbar machen. Eigener Schritt danach.
- Der App-Bundle-Weg (`app-bundle.txt`) wird **nicht mehr verwendet**: Der ESP meldet
  die Unterstuetzung ausdruecklich als nicht vorhanden, die zugehoerigen URLs sind leer,
  die Legacy-Seite bietet keinen Upload, und auf dem Update-Server liegt keine solche
  Datei. `build-app-bundle.py` laeuft ohnehin nicht mehr durch. `APP-BUNDLE.md` ist als
  historisch gekennzeichnet; die vier Dateien koennen entfernt werden.

### Konventionen

- Bei jedem Build werden **alle drei Komponenten im Gleichschritt** versioniert, auch
  wenn sich die jeweilige nicht geaendert hat.
- `tools/deploy.sh` setzt nach jedem Rollout ein Tag `release/<stm>-<esp>-<app>`.


## 2026-04-29 Bilingual PWA Finalization And Safari Reload Hardening

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-29-2252.zip`
- PWA-Version: `1.4.61`

Wichtige Punkte:

- Die PWA-Oberfläche ist jetzt durchgängig zweisprachig mit:
  - Default `Deutsch`
  - optional `English`
  - persistierter Sprachwahl im Browser
- Der Sprachumbau umfasst jetzt nicht nur statische Labels, sondern auch:
  - Save-/Busy-/Success-Buttontexte
  - Laufzeit- und Wartungsmeldungen
  - Remote-Update-/STM32-Flash-Fortschritt
  - LittleFS-/Update-/Dateiaktionen
  - Overlay-, Timer- und DFPlayer-Aktionen
- Mehrere hart codierte deutsche Rücksprungtexte in Button-Handlern wurden auf zentrale `i18n`-Keys umgestellt
- `restoreText`-Synchronisierung für Buttons wurde vereinheitlicht, damit Buttons nach einem Klick nicht wieder auf alte deutsche Idle-Texte zurückspringen
- Mobile-Safari-Reload wurde auf ESP-Seite weiter gehärtet:
  - kürzere Behandlung leerer/angebrochener Requests
  - eigene kurze Request-Line-Leselogik statt direkter Blockierung über `readStringUntil('\r')`
- Der Auto-Refresh-Takt der PWA liegt jetzt mit `+1 s` Versatz auf:
  - `:02`
  - `:17`
  - `:32`
  - `:47`

Wichtige technische Hinweise:

- Restliche Browser-Systemtexte an nativen Dateifeldern wie `Datei auswählen` oder `Keine Datei ausgewählt` kommen weiterhin vom Browser selbst und nicht aus der PWA.
- Der aktuelle Sprachstand wurde bewusst nicht nur über HTML-Attribute, sondern zusätzlich über die Laufzeitpfade in [app.js](/ESP8266/ESP-uclock/data/app/app.js) bereinigt, weil dort die meisten Rücksprungtexte sassen.

## 2026-04-29 Gzip PWA Rollout, Restore Timing And Safari Hardening

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-29-1957.zip`
- PWA-Version: `1.4.37`

Wichtige Punkte:

- PWA-App-Dateien werden jetzt gzip-komprimiert gebaut, hochgeladen und vom ESP bevorzugt als `.gz` ausgeliefert
- `/app` prüft installierte App-Dateien nicht mehr nur auf Existenz, sondern auch auf Dateigrösse `> 0`, damit Crash-Reste mit leeren Dateien nicht als gültige Installation gelten
- Remote-App-Installpfad für `/app/?action=install` wurde auf Stack- und Timing-Probleme gehärtet
- Mobile-Safari-Ladepfad wurde stabilisiert:
  - statische `/app`-Assets senden jetzt `Connection: close`
  - der ESP beendet die Verbindung nach dem Dateistream explizit
- Import/Restore wurde in mehreren empfindlichen Bereichen entschärft:
  - Overlay-Restore mit absteigendem Delete, sequentiellem Wiederaufbau und zusätzlicher Entkopplung
  - Timer-Restore mit grösseren Abständen und Reload-Schritten
  - `ticker_deceleration` mit zusätzlichem Schutzabstand vor und nach dem Write, damit serielle Kommandos nicht ineinanderlaufen
- Der globale Hinweis für fehlende Layout-Tabellen erscheint jetzt erst, wenn die Tabellen-Info wirklich geladen ist, und flackert nicht mehr beim Start auf

Wichtiger technischer Hinweis:

- Der iPhone-Safari-Fall war zuletzt kein klassischer Frontend-Fehler, sondern ein empfindlicher HTTP-/Connection-Fall beim Ausliefern der PWA-Assets.
- Der Restore-Fehler bei `ticker_deceleration` zeigte sich im Log als Kommando-Interleaving auf der seriellen STM32-Strecke und wurde deshalb bewusst über Timing-Abstände statt über ein Format- oder Mapping-Rework gelöst.

## 2026-04-15 PWA Update Visibility And UX Polish

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-15-0127.zip`
- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.55`

Wichtige Punkte:

- die PWA zeigt die Versionsstände jetzt in der Reihenfolge `WordClock`, `ESP`, `App`
- zusätzlich zur lokalen `App-Version` wird jetzt auch `App verfügbar` vom Update-Server angezeigt
- dafür wird beim Build eine `app-version.txt` erzeugt und zusammen mit `app-bundle.txt` ins Release aufgenommen
- der Update-Server muss für die PWA-Versionsprüfung jetzt sowohl `app-bundle.txt` als auch `app-version.txt` bereitstellen
- mehrere bislang stille PWA-Aktionen haben jetzt konsistentes Button-Feedback, darunter `Overrides anwenden`, `Overrides zurücksetzen`, Overlay-Aktionen und Teile des Wetterdialogs

## 2026-04-15 PWA ESP Update Timing Fix

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-15-0024.zip`
- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.45`

Wichtige Punkte:

- der ESP-Updatepfad der PWA wartet jetzt deutlich länger, bevor Reconnect und Reload gestartet werden
- damit orientiert sich die PWA wieder am Legacy-Verhalten des ESP-Updates mit rund `40` Sekunden Reconnect-Zeit
- der zu frühe PWA-Reload nach ca. `25-30` Sekunden wird vermieden und bricht das ESP-Update nicht mehr vorzeitig ab
- der PWA-Button für ESP-Updates verwendet jetzt wieder den bewährten Legacy-Updatepfad als Top-Level-Navigation statt eines versteckten `iframe`
- Legacy-Weboberfläche liefert HTML jetzt mit `UTF-8`-Charset aus und verwendet wieder gut lesbare Linkfarben statt Gelb/Weiss auf Weiss

## 2026-04-14 Runtime Recovery And Reset Visibility

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-15-0024.zip`
- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.44`

Wichtige Punkte:

- `STM32`-Schutzmassnahmen ergänzt: `IWDG` mit ca. `20 s` Timeout
- `HardFault`, `MemManage`, `BusFault` und `UsageFault` führen nicht mehr in eine Endlosschleife, sondern loggen kurz und starten das Board kontrolliert neu
- Reset-Ursachen aus den `RCC`-Flags werden beim Boot weiterhin geloggt
- relevante Reset-Ursachen werden jetzt zusätzlich ohne serielles Kabel in die PWA gespiegelt
- in der PWA erscheint im Überblick nur beim aktuellen Boot und nur bei relevanten Flags ein Eintrag `Letzter STM32-Neustart`
- frühe blockierende Watchdog-Initialisierung wurde korrigiert, damit der Controller nach `Reset flags:` normal weiter bootet

Hinweise:

- normale Einschalt-/Pin-Resets werden in der PWA bewusst nicht angezeigt
- für die Anzeige in `/app` müssen sowohl die `STM32`-Firmware als auch das aktuelle `app-bundle.txt` ausgerollt werden

## 2026-04-09 Finalized Restore Release

Aktueller verifizierter Abschlussstand:

- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.43`

Wichtige Punkte:

- Backup/Restore läuft jetzt wieder sauber mit automatischem `STM32`-Reset und anschliessendem PWA-Reload
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

- Der Overlay-Fix liegt in [vars.cpp](/ESP8266/ESP-uclock/vars.cpp): `set_overlay_var()` macht jetzt am Ende ein `Serial.flush()`.
- Der Live-Farbpfad für `Rainbow` wurde in [display.c](/src/display/display.c) gedrosselt:
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

- Der Fehler lag im SK6812-Treiber in [src/sk6812/sk6812.c](/src/sk6812/sk6812.c).
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
- Der kritische Rückbau erfolgte in [src/display/display.c](/src/display/display.c), damit die STM-Farbpfade wieder dem funktionierenden Verhalten aus `3.1.5` entsprechen.
- Zusätzliche `var_send_display_colors();`-Aufrufe direkt in `display_init_color_animation_rainbow()` und `display_init_color_animation_daylight()` wurden als instabil verifiziert und bleiben in diesem Basisstand bewusst draussen.
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
