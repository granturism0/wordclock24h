# WordClock App Bundle

Die moderne App unter `/app` wird als einzelnes Bundle fuer LittleFS OTA verteilt.

Dateiquellen:

- [data/app](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app)

Erzeugte Bundle-Datei:

- [data/app-bundle.txt](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app-bundle.txt)

## Bundle neu bauen

Einfachster Weg:

```bash
sh ESP8266/ESP-uclock/tools/release-app-bundle.sh
```

Alternativ direkt:

```bash
python3 ESP8266/ESP-uclock/tools/build-app-bundle.py
```

## Deployment

Es gibt zwei Wege:

1. Download-Server

- `app-bundle.txt` auf denselben Update-Server legen wie die bestehenden OTA-Dateien
- auf dem Geraet `http://<esp-ip>/fs` aufrufen
- `Update Host` und `Update Path` wie gewohnt verwenden
- `Download WordClock App bundle` klicken

2. Manueller OTA-Upload

- `http://<esp-ip>/fs` aufrufen
- im Abschnitt `WordClock App bundle (/app)` die Datei `app-bundle.txt` hochladen

## Erwartete Dateien im LittleFS

Nach erfolgreicher Installation liegen die App-Dateien absichtlich mit flachen Namen im LittleFS, damit der ESP8266 sie robuster verarbeiten kann:

- `app-index.html`
- `app-app.js`
- `app-styles.css`
- `app-manifest.webmanifest`
- `app-sw.js`
- `app-icons-icon-192.svg`
- `app-icons-icon-512.svg`

Die Web-Route bleibt trotzdem:

- `http://<esp-ip>/app`

Die Firmware mappt `/app/...` intern auf diese flachen Dateinamen.

## Wichtig

- Bei Aenderungen an der App muss normalerweise nur das Bundle neu gebaut und verteilt werden.
- Die Firmware muss nur dann neu geflasht werden, wenn sich der ESP-Code selbst aendert, etwa in [http.cpp](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/http.cpp).
- Das Bundle erwartet Dateien unter `app/...` und wird beim Installieren direkt in diese LittleFS-Pfade entpackt.
