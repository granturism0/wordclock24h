# WordClock App Bundle

> **Dieser Weg wird nicht mehr verwendet. Dieses Dokument ist historisch.**
>
> Geprüft am 2026-09-29, im Code und auf dem Update-Server:
>
> - Der ESP meldet der PWA ausdrücklich `app_bundle_available: 0`,
>   `update_download_app_bundle_api_supported: 0` und
>   `app_bundle_upload_api_supported: 0` (`http.cpp:9532`, `:9540`, `:9543`)
> - Die drei zugehörigen URLs sendet er als **leeren String** (`http.cpp:9622`, `:9625`,
>   `:9635`). Einen Handler gibt es nicht
> - Die Legacy-`/fs`-Seite bietet **keinen** Bundle-Upload mehr
> - In `app.js` gibt es genau **ein** Vorkommen: einen Default-Eintrag, der von keiner
>   Stelle gelesen wird
> - Auf dem Update-Server liegt **keine** `app-bundle.txt`
> - `build-app-bundle.py` läuft ohnehin nicht mehr durch: es liest die vorhandenen
>   `*.gz` als UTF-8 und bricht mit `UnicodeDecodeError: byte 0x8b` ab. Kein
>   Makefile-Ziel ruft es auf; das vorhandene `data/app-bundle.txt` stammt vom
>   29. April 2026
>
> **Der aktuelle Weg** ist `make release-zip` gefolgt von `./tools/deploy.sh`. Die
> App-Assets werden einzeln als `.gz` ausgeliefert, nicht als Bundle.
>
> Kandidaten zum Entfernen, sobald du es entscheidest: `tools/build-app-bundle.py`,
> `tools/release-app-bundle.sh`, `data/app-bundle.txt` und dieses Dokument.



Die moderne PWA läuft parallel zur Legacy-Seite unter `/app` und wird als einzelnes Bundle per LittleFS OTA verteilt.

Quellen:

- [data/app](../../ESP8266/ESP-uclock/data/app)
- [app-bundle.txt](../../ESP8266/ESP-uclock/data/app-bundle.txt)

## Zweck

Die PWA ersetzt die Legacy-Seite nicht hart, sondern läuft parallel:

- Legacy: `http://<esp-ip>/`
- Legacy direkt: `http://<esp-ip>/legacy`
- PWA: `http://<esp-ip>/app`

Die Firmware liefert `/app` aus LittleFS aus. Die eigentlichen App-Dateien liegen dort bewusst mit flachen Dateinamen, damit der ESP8266 sie robuster verarbeiten kann.

## PWA-Struktur

Die wichtigsten Dateien:

- [index.html](../../ESP8266/ESP-uclock/data/app/index.html)
  Einstieg, Modulstruktur, Panels
- [app.js](../../ESP8266/ESP-uclock/data/app/app.js)
  UI-Logik, API-Aufrufe, WordClock-Vorschau, Statusführung
- [styles.css](../../ESP8266/ESP-uclock/data/app/styles.css)
  Layout, Responsivität, Komponentenstil
- [manifest.webmanifest](../../ESP8266/ESP-uclock/data/app/manifest.webmanifest)
  PWA-Metadaten
- [sw.js](../../ESP8266/ESP-uclock/data/app/sw.js)
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

- [app-bundle.txt](../../ESP8266/ESP-uclock/data/app-bundle.txt)
- `build/esp8266/app-version.txt`

Zusätzliche Hilfsdateien:

- [build-app-bundle.py](../../ESP8266/ESP-uclock/tools/build-app-bundle.py)
- [release-app-bundle.sh](../../ESP8266/ESP-uclock/tools/release-app-bundle.sh)

## Verteilung

Es gibt zwei Wege.

1. Download vom Update-Server

- `app-bundle.txt` auf denselben Update-Server legen wie die übrigen OTA-Dateien
- zusätzlich `app-version.txt` auf denselben Update-Server legen
- auf dem Gerät `http://<esp-ip>/fs` öffnen
- `Update Host` und `Update Path` wie gewohnt setzen
- im Bereich `WordClock App bundle (/app)` den Download auslösen

2. Manueller OTA-Upload

- `http://<esp-ip>/fs` öffnen
- im Bereich `WordClock App bundle (/app)` die Datei `app-bundle.txt` hochladen

## Wichtige ESP-Pfade

- `/` Legacy-Oberfläche
- `/legacy` direkte Legacy-Oberfläche
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
- Änderungen in [http.cpp](../../ESP8266/ESP-uclock/http.cpp)

## Praktischer Ablauf

Nur PWA geändert:

1. Dateien unter [data/app](../../ESP8266/ESP-uclock/data/app) anpassen
2. `sh ESP8266/ESP-uclock/tools/release-app-bundle.sh`
3. neues [app-bundle.txt](../../ESP8266/ESP-uclock/data/app-bundle.txt) und `build/esp8266/app-version.txt` auf den Server legen oder das Bundle über `/fs` hochladen
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
- `app-version.txt` wird für die verfügbare Server-App-Version benötigt.
- `Neu laden` in der PWA triggert einen schnelleren Bootstrap-Load und wartet nicht nur auf den normalen Auto-Refresh.

## Aktueller Zielstand

Der aktuelle Zielstand der Migration ist:

- Legacy bleibt bewusst unter `/` und `/legacy`
- `/app` ist die produktive PWA
- Remote-Update, Import/Export und der PWA-Unterbau wurden entkoppelt und konsolidiert
- der weitere Betrieb soll primär über gezielte Bugfixes erfolgen, nicht mehr über breite Architekturumbauten
