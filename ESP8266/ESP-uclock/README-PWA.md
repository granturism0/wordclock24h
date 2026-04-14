# WordClock PWA

## Zweck

Die moderne WordClock-Oberfläche läuft parallel zur Legacy-Seite unter:

- `/app`

Die Legacy-Seite bleibt weiterhin erreichbar unter:

- `/`

## Relevante Dateien

- [index.html](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/index.html)
- [app.js](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/app.js)
- [styles.css](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/styles.css)
- [manifest.webmanifest](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/manifest.webmanifest)
- [sw.js](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/sw.js)
- [http.cpp](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/http.cpp)

## Bundle bauen

```bash
sh ESP8266/ESP-uclock/tools/release-app-bundle.sh
```

Erzeugt:

- [app-bundle.txt](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app-bundle.txt)
- `build/esp8266/app-version.txt`

## Deployment

### Nur PWA geändert

1. Dateien unter `data/app` anpassen
2. Bundle neu bauen
3. `app-bundle.txt` und bei Server-Deployment zusätzlich `app-version.txt` aktualisieren
4. `/app` neu laden

### ESP-Logik geändert

1. Firmware neu flashen
2. danach das aktuelle `app-bundle.txt` installieren
3. `/app` prüfen

## Wichtige Hinweise

- Die PWA-Dateien werden flach im LittleFS abgelegt, z. B. `app-index.html`, `app-app.js`, `app-styles.css`
- Das Routing von `/app/...` auf diese Dateien übernimmt die Firmware in [http.cpp](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/http.cpp)
- Wenn unter `/app` noch keine App installiert ist, liefert die Firmware eine Hinweisseite
- Die PWA-Quelldateien unter `data/app` tragen einheitliche Dateikopf-Kommentare
- Die aktuelle PWA-Version des zuletzt gebauten Arbeitsstands ist `1.2.55`
- Relevante `STM32`-Reset-Ursachen werden im Überblick angezeigt, wenn sie beim aktuellen Boot erkannt wurden
- Die Update-Übersicht zeigt neben `App-Version` jetzt auch `App verfügbar`
- Für `App verfügbar` muss auf dem Update-Server eine `app-version.txt` liegen

## Aktueller Stand

Der aktuell verifizierte gemeinsame Stand ist:

- `build/releases/wordclock-release-2026-04-15-0022.zip`
- `build/releases/wordclock-release-2026-04-15-0127.zip`

Wichtig daran:

- `STM32`-Faults führen nicht mehr zu dauerhaftem Hängen, sondern zu einem kontrollierten Reset
- der `STM32`-Watchdog startet das Board nach echten Hängern automatisch neu
- relevante Reset-Ursachen werden ohne serielles Kabel in der PWA sichtbar
- die Watchdog-Initialisierung erfolgt nun erst kurz vor dem Hauptloop und blockiert den Boot nicht mehr
- die PWA kann jetzt auch eine verfügbare Server-App-Version anzeigen

## Ergänzende Doku

Ausführlicher beschrieben in:

- [APP-BUNDLE.md](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/APP-BUNDLE.md)
