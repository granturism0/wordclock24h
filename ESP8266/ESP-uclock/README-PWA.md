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

- [index.html](../../ESP8266/ESP-uclock/data/app/index.html)
- [app.js](../../ESP8266/ESP-uclock/data/app/app.js)
- [styles.css](../../ESP8266/ESP-uclock/data/app/styles.css)
- [manifest.webmanifest](../../ESP8266/ESP-uclock/data/app/manifest.webmanifest)
- [sw.js](../../ESP8266/ESP-uclock/data/app/sw.js)
- [http.cpp](../../ESP8266/ESP-uclock/http.cpp)

## Deployment

### Nur PWA geändert

1. Dateien unter `data/app` anpassen, `APP_VERSION` und `CACHE_NAME` gemeinsam anheben
2. `make release-zip`, danach `./tools/deploy.sh` (Update-Server)
3. `./tools/install-app.sh` lädt die Assets ins LittleFS der Uhr, `--check` prüft sie
   gegen die Weissliste
4. `/app` neu laden

### ESP-Logik geändert

1. Firmware einspielen — **vor** einer PWA, die sie voraussetzt
2. `./tools/install-app.sh --check`: Ein Firmware-Wechsel löscht das Dateisystem nicht,
   aber eine geänderte Weissliste findet alte Namen nicht mehr
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
- Komprimierbare PWA-Dateien werden im aktuellen Rollout zusätzlich als `.gz` gebaut, gespeichert und bevorzugt so ausgeliefert
- Das Routing von `/app/...` auf diese Dateien übernimmt die Firmware in [http.cpp](../../ESP8266/ESP-uclock/http.cpp)
- Wenn unter `/app` noch keine App installiert ist, liefert die Firmware eine Hinweisseite
- `/app` betrachtet eine App-Datei nur dann als gültig installiert, wenn sie vorhanden ist und `> 0` Byte gross ist
- Die PWA-Quelldateien unter `data/app` tragen einheitliche Dateikopf-Kommentare
- Für die App-Update-Prüfung muss auf dem Update-Server zusätzlich `app-version.txt` liegen
- Relevante `STM32`-Reset-Ursachen werden im Überblick angezeigt, wenn sie beim aktuellen Boot erkannt wurden
- Die Hauptseite zeigt `Letzter Start`, sofern die passende Laufzeitinformation vom Gerät geliefert wird
- `Neu laden` und Start nach Reload versuchen den ersten Snapshot aggressiver nachzuladen, statt sichtbar auf den normalen Auto-Refresh-Takt zu warten
- Die Modulnavigation passt sich responsiv an:
  - mobil mit horizontalem Scrollen
- auf breiten Displays mit voller Breite, solange kein echter Überlauf besteht

## Stand 2026-04-29

Für den aktuellen PWA-/Restore-Stand sind diese Punkte wichtig:

- Der `.gz`-Pfad umfasst Build, lokalen Upload, Remote-Install und `/app`-Serving konsistent.
- Der Remote-Installpfad für `/app/?action=install` war auf dem ESP empfindlich gegenüber Stacklast und Timing; der aktuelle Stand ist darauf gehärtet.
- Bei Mobile Safari war der kritische Punkt zuletzt die HTTP-Auslieferung der PWA-Assets. Der ESP schliesst diese Responses jetzt explizit sauber ab.
- Restore-Fehler bei Overlays, Timern und `ticker_deceleration` waren keine reinen JSON-Probleme, sondern in mehreren Fällen Timing-/Interleaving-Themen im Write-Pfad zur Uhr.
- Die Timing-Abstände im Restore sind deshalb aktuell bewusst konservativ gewählt und sollten nicht leichtfertig wieder reduziert werden, ohne die seriellen Logs mitzuprüfen.
- Die PWA ist jetzt durchgängig zweisprachig aufgebaut:
  - Default-Sprache `de`
  - optionale Sprache `en`
  - Speicherung der Auswahl im Browser
- Für die Sprachumschaltung reicht es nicht, nur statische HTML-Texte zu übersetzen:
  - auch Button-`restoreText`
  - Busy-/Success-Zustände
  - Wartungs-/Update-Fortschritt
  - Overlay-/Timer-/DFPlayer-Handler
  müssen explizit über `translate(...)` bzw. `translateFormat(...)` laufen
- Native Dateiauswahl-Texte der Browser bleiben browserabhängig und sind bewusst nicht Teil des PWA-i18n-Systems.
- Der Safari-Reload-Pfad war empfindlich gegenüber leeren Verbindungen vor der eigentlichen Request-Line.
  - Die Firmware liest die erste Request-Zeile deshalb jetzt über eine eigene kurze Timeout-Logik.
  - Ziel ist, `empty http request`-Fälle schnell zu verwerfen statt den `/app`-Reload spürbar zu blockieren.

## Einfrierpunkt

Der Umbau wurde bis zu einem stabilen PWA-/Legacy-Zielstand durchgezogen. Ab jetzt gilt:

- keine grossen Strukturumbauten mehr “aus Prinzip”
- nur noch gezielte Bugfixes oder UX-Nachzüge nach echtem Praxisfund
- der aktuelle Stand ist als bewusster Freeze-Kandidat zu verstehen

## Oberfläche ohne Gerät ansehen

`tools/preview/` liefert die PWA aus und simuliert die Geräte-API, sodass die Oberfläche
ohne erreichbare WordClock betrachtet und vermessen werden kann:

```
python3 tools/preview/server.py 8099
./tools/preview/shot.sh --diag 390x844
```

Grenzen und Details in [tools/preview/README.md](../../tools/preview/README.md).

## Bauen und ausrollen

Der Weg ist `make release-zip` gefolgt von `./tools/deploy.sh`. Die App-Assets werden
einzeln als `.gz` ausgeliefert.

Den früheren Bundle-Weg (`app-bundle.txt`) gibt es nicht mehr. Er war tot, nicht
defekt: Der ESP meldet die Bundle-Unterstützung als nicht vorhanden. Die Werkzeuge, die
Datei und `APP-BUNDLE.md` wurden am 07.10.2026 entfernt (E2, `BEFUNDE.md` L1); die
Git-Historie hält sie.
