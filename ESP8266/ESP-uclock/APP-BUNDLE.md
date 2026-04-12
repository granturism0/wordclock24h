# WordClock App Bundle

Die moderne PWA läuft parallel zur Legacy-Seite unter `/app` und wird als einzelnes Bundle per LittleFS OTA verteilt.

Quellen:

- [data/app](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app)
- [app-bundle.txt](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app-bundle.txt)

## Zweck

Die PWA ersetzt die Legacy-Seite nicht hart, sondern läuft parallel:

- Legacy: `http://<esp-ip>/`
- PWA: `http://<esp-ip>/app`

Die Firmware liefert `/app` aus LittleFS aus. Die eigentlichen App-Dateien liegen dort bewusst mit flachen Dateinamen, damit der ESP8266 sie robuster verarbeiten kann.

## PWA-Struktur

Die wichtigsten Dateien:

- [index.html](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/index.html)
  Einstieg, Modulstruktur, Panels
- [app.js](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/app.js)
  UI-Logik, API-Aufrufe, WordClock-Vorschau, Statusführung
- [styles.css](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/styles.css)
  Layout, Responsivität, Komponentenstil
- [manifest.webmanifest](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/manifest.webmanifest)
  PWA-Metadaten
- [sw.js](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app/sw.js)
  Service Worker

## Bundle bauen

Einfachster Weg:

```bash
sh ESP8266/ESP-uclock/tools/release-app-bundle.sh
```

Alternativ direkt:

```bash
python3 ESP8266/ESP-uclock/tools/build-app-bundle.py
```

Ergebnis:

- [app-bundle.txt](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app-bundle.txt)

Zusätzliche Hilfsdateien:

- [build-app-bundle.py](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/tools/build-app-bundle.py)
- [release-app-bundle.sh](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/tools/release-app-bundle.sh)

## Verteilung

Es gibt zwei Wege.

1. Download vom Update-Server

- `app-bundle.txt` auf denselben Update-Server legen wie die übrigen OTA-Dateien
- auf dem Gerät `http://<esp-ip>/fs` öffnen
- `Update Host` und `Update Path` wie gewohnt setzen
- im Bereich `WordClock App bundle (/app)` den Download auslösen

2. Manueller OTA-Upload

- `http://<esp-ip>/fs` öffnen
- im Bereich `WordClock App bundle (/app)` die Datei `app-bundle.txt` hochladen

## Wichtige ESP-Pfade

- `/` Legacy-Oberfläche
- `/fs` LittleFS, Uploads, Bundle-Install
- `/app` neue PWA
- `/update` Legacy-Updatebereich

Wenn unter `/app` noch keine PWA installiert ist, liefert die Firmware eine Hinweisseite statt einer rohen Fehlermeldung.

## Erwartete LittleFS-Dateien

Nach erfolgreicher Installation liegen typischerweise diese Dateien im LittleFS:

- `app-index.html`
- `app-app.js`
- `app-styles.css`
- `app-manifest.webmanifest`
- `app-sw.js`
- `app-icons-icon-192.svg`
- `app-icons-icon-512.svg`

Die Route bleibt trotzdem:

- `http://<esp-ip>/app`

Die Firmware mappt `/app/...` intern auf diese flachen LittleFS-Dateinamen.

## Wann Bundle, wann Firmware

Nur Bundle neu bauen und verteilen:

- UI-Änderungen in `index.html`
- Logik-Änderungen in `app.js`
- Styling in `styles.css`
- Änderungen an Manifest, Service Worker oder Icons

Firmware neu flashen nötig:

- neue oder geänderte HTTP-Endpunkte
- Änderungen an LittleFS-Routen
- Änderungen am Legacy-`/fs`- oder `/app`-Verhalten
- Änderungen in [http.cpp](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/http.cpp)

## Praktischer Ablauf

Nur PWA geändert:

1. Dateien unter [data/app](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app) anpassen
2. `sh ESP8266/ESP-uclock/tools/release-app-bundle.sh`
3. neues [app-bundle.txt](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/data/app-bundle.txt) auf den Server legen oder über `/fs` hochladen
4. `/app` neu laden

ESP-Verhalten geändert:

1. Firmware neu flashen
2. danach aktuelles `app-bundle.txt` installieren
3. `/app` prüfen

## Hinweise für den Alltag

- Nach App-Änderungen reicht normalerweise ein neues Bundle.
- Nach Firmware-Änderungen muss die Firmware neu auf den ESP.
- Wenn die App nicht aktuell wirkt, PWA/Browser einmal neu laden.
- Die PWA liest echte Gerätedaten über die API-Endpunkte des ESP und nutzt keine separate Backend-Struktur.

## Aktueller Arbeitsstand

Der aktuell verifizierte gemeinsame Release-Stand ist:

- `build/releases/wordclock-release-2026-04-08-2217.zip`

PWA-Stand dazu:

- Version `1.2.28`

Wichtige Punkte dieses Stands:

- Overlay-Restore ist repariert (Overlays werden nach dem STM32-Reset neu gesetzt)
- `Ambilight online/offline` wird korrekt wiederhergestellt (localStorage-State wird beim Import mitgesetzt)
- Sommerzeit wird nach dem STM32-Reset nochmals gesetzt
- Netzwerk-/Zeiteinstellungen werden nach dem Restore nochmals verifiziert
- `days`-Feld in Overlays wird beim Backup-Import korrekt übernommen (Fallback-Bug behoben)
- Retry-Prüfung für Display- und Klima-Einstellungen deckt nun alle relevanten Felder ab
- Backup-Version wird beim Import geprüft
