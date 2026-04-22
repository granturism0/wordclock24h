# WordClock PWA

## Zweck

Die moderne WordClock-Oberfläche läuft parallel zur Legacy-Seite unter:

- `/app`

Die Legacy-Seite bleibt weiterhin erreichbar unter:

- `/`
- `/legacy`

Damit gilt aktuell bewusst:

- `root` bleibt Legacy/Fallback
- `/app` ist die moderne PWA
- `/legacy` ist die feste direkte Legacy-URL

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

## Architekturstand

Der aktuelle Stand ist bewusst aufgeteilt:

- `Import/Export` ist konsolidiert
- `Remote-Update` ist entkoppelt
- `/app` läuft wieder als echte PWA mit Manifest und Service Worker
- Legacy bleibt als robuster Fallback unter `/` und `/legacy`

Die PWA ist damit produktiv nutzbar, ohne die Legacy-Seite hart zu ersetzen.

## Wichtige Hinweise

- Die PWA-Dateien werden flach im LittleFS abgelegt, z. B. `app-index.html`, `app-app.js`, `app-styles.css`
- Das Routing von `/app/...` auf diese Dateien übernimmt die Firmware in [http.cpp](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/http.cpp)
- Wenn unter `/app` noch keine App installiert ist, liefert die Firmware eine Hinweisseite
- Die PWA-Quelldateien unter `data/app` tragen einheitliche Dateikopf-Kommentare
- Für die App-Update-Prüfung muss auf dem Update-Server zusätzlich `app-version.txt` liegen
- Relevante `STM32`-Reset-Ursachen werden im Überblick angezeigt, wenn sie beim aktuellen Boot erkannt wurden
- Die Hauptseite zeigt `Letzter Start`, sofern die passende Laufzeitinformation vom Gerät geliefert wird
- `Neu laden` und Start nach Reload versuchen den ersten Snapshot aggressiver nachzuladen, statt sichtbar auf den normalen Auto-Refresh-Takt zu warten
- Die Modulnavigation passt sich responsiv an:
  - mobil mit horizontalem Scrollen
  - auf breiten Displays mit voller Breite, solange kein echter Überlauf besteht

## Einfrierpunkt

Der Umbau wurde bis zu einem stabilen PWA-/Legacy-Zielstand durchgezogen. Ab jetzt gilt:

- keine großen Strukturumbauten mehr “aus Prinzip”
- nur noch gezielte Bugfixes oder UX-Nachzüge nach echtem Praxisfund
- der aktuelle Stand ist als bewusster Freeze-Kandidat zu verstehen

## Ergänzende Doku

Ausführlicher beschrieben in:

- [APP-BUNDLE.md](/Users/daniel/Documents/GitHub/wordclock24h/ESP8266/ESP-uclock/APP-BUNDLE.md)
