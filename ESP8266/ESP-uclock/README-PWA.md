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

## Deployment

### Nur PWA geändert

1. Dateien unter `data/app` anpassen
2. Bundle neu bauen
3. `app-bundle.txt` über `/fs` hochladen oder vom Update-Server laden
4. `/app` neu laden

### ESP-Logik geändert

1. Firmware neu flashen
2. danach das aktuelle `app-bundle.txt` installieren
3. `/app` prüfen

## Wichtige Hinweise

- Die PWA-Dateien werden flach im LittleFS abgelegt, z. B. `app-index.html`, `app-app.js`, `app-styles.css`
- Das Routing von `/app/...` auf diese Dateien übernimmt die Firmware in [http.cpp](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/http.cpp)
- Wenn unter `/app` noch keine App installiert ist, liefert die Firmware eine Hinweisseite

## Ergänzende Doku

Ausführlicher beschrieben in:

- [APP-BUNDLE.md](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/APP-BUNDLE.md)
