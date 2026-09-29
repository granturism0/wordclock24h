# wordclock24h — Arbeits- und Koordinationsregeln

Sprache: **Deutsch**, mit echten Umlauten — auch in der UI und in Commit-Messages.

Anrede: durchgehend **Du-Form**, niemals „Sie". Das gilt **in erster Linie für die
Antworten an den Nutzer** — er wird geduzt. Ebenso für die UI-Texte der PWA, für
Meldungen und Fehlertexte. Bestehende Du-Formulierungen nicht auf „Sie" umschreiben.

## Versionsstände (Single Source of Truth)

| Was | Datei | Aktuell |
|---|---|---|
| STM/WordClock | `src/main.h` → `#define VERSION` | 3.2.4 |
| ESP | `ESP8266/ESP-uclock/version.h` → `#define ESP_VERSION` | 3.2.1 |
| PWA-App | `ESP8266/ESP-uclock/data/app/app.js` → `const APP_VERSION` | 1.4.69 |
| SW-Cache | `ESP8266/ESP-uclock/data/app/sw.js` → `const CACHE_NAME` | wordclock-app-v61 |

Der Makefile liest diese Werte per `grep` aus (Targets `stm-version-file`,
`esp-version-file`, `app-version-file`). Das Format der Zeilen darf sich nicht
ändern, sonst bricht der Release-Build still.

## Architektur-Invarianten (nicht ohne Rückfrage aufweichen)

- Die PWA unter `/app` läuft **parallel** zur Legacy-Weboberfläche, nicht als Ersatz.
  Legacy bleibt funktionsfähig und ist die Stabilitäts-Referenz beim Debuggen.
- App-Assets werden **ausschliesslich als `.gz`** ausgeliefert. Keine Plain-Fallbacks.
- Existenzprüfungen für Assets müssen zusätzlich **Dateigrösse > 0** prüfen.
  Eine leere `.gz` führt zu White-Screen/Crash — das ist real passiert.
- Wetter läuft über `/api/weather_get_now` und `/api/weather_get_forecast`.
  Der frühere Legacy-Bypass (`/weather?action=...`) wurde bewusst entfernt.
  **Nicht zurückbauen.**
- Wetter-Ticker ist asynchron: `pending_weather_ticker_restore` in `src/main.c`.
  Display-Restore erst wenn Ticker inaktiv, kein Icon aktiv, kein Icon-Stop-Timer
  offen, kein Overlay aktiv. Diese Bedingung nicht vereinfachen.

## Vorschau der PWA ohne Gerät

```
python3 tools/preview/server.py 8099      # Terminal 1
./tools/preview/shot.sh --diag 390x844    # Terminal 2
```

Liefert `data/app` aus und simuliert die Geräte-API. Damit lässt sich die Oberfläche
ansehen und vermessen, ohne dass eine WordClock erreichbar ist — horizontaler Überlauf,
abgeschnittener Text, Touch-Targets unter 44 px, Fokusstil, `color-scheme`.

**Ersetzt keinen Test am Gerät.** Chrome statt iOS Safari, kein Notch also keine
Safe-Area, `127.0.0.1` ist ein sicherer Kontext und das echte Gerät nicht, und die
Mock-Daten sind kürzer als echte. Details und die drei eingebauten Chrome-Eigenheiten
stehen in `tools/preview/README.md`.

## Build

```
make app-gz        # nur die .gz-Artefakte
make release-zip   # app-gz + app-version-file + f103 + f411 + esp + ZIP
```

Nach jeder relevanten Änderung: **kompletter Build und Release-ZIP**, nicht nur
`app-gz`. Immer explizit sagen, was zu flashen ist: nur App/LittleFS, oder auch STM
bzw. ESP.

### Versionspflicht bei jedem Build (DIR-004)

**Kein Build ohne Versionserhöhung — und zwar aller drei Komponenten im Gleichschritt.**
Jedes Fabrikat muss als Einheit identifizierbar und eindeutig einem Commit zuzuordnen
sein.

| Komponente | Anheben | Datei |
|---|---|---|
| STM32 | `VERSION` | `src/main.h` |
| ESP8266 | `ESP_VERSION` | `ESP8266/ESP-uclock/version.h` |
| PWA | `APP_VERSION` **und** `CACHE_NAME` | `data/app/app.js`, `sw.js` |

**Alle vier Stellen, bei jedem Build** — auch wenn sich die jeweilige Komponente nicht
geändert hat. `APP_VERSION` und `CACHE_NAME` gehören ohnehin zusammen: ohne
`CACHE_NAME`-Bump liefert der Service Worker neue `index.html` mit alter `app.js`.
Guardrail-Stufe S4 prüft alle vier.

**Bekannte Nebenwirkung:** Wird eine Komponente angehoben, deren Code unverändert ist,
bietet die Uhr danach ein Update auf identische Firmware an. Ungefährlich, aber ein
OTA-Flash ohne Gegenwert.

### Dokumentation nachführen (DIR-006)

Nach jedem Release führt der `doc-writer` `CHANGELOG.md` nach, und bei neuen Werkzeugen
oder Abläufen auch die README-Dateien. Ein Release gilt erst als fertig, wenn der
Changelog-Eintrag steht.

`REVIEW*.md` und `gap-analysis.md` sind Momentaufnahmen mit Datum und werden **nicht**
fortgeschrieben.

### Rollout auf die Synology (DIR-005)

```
./tools/deploy.sh --dry-run    zeigt, was uebertragen wuerde
./tools/deploy.sh              uebertraegt
```

Ziel ist `/volume1/web/wordclock/test8` auf `diskstation.lan` (SSH, Port 5002). Übertragen
werden App-Assets, beide `.hex`, die ESP-`.bin` und die Versionsdateien. Zusätzlich wird
die H2-Kopfzeile in `releasenote.html` auf die aktuelle STM-Version nachgezogen.

**Das Ziel ist zugleich der Update-Server, von dem die Uhr per OTA lädt.** Deshalb prüft
das Skript jedes Artefakt, bevor es irgendetwas überträgt, und bricht ab statt einen
kaputten Stand auszurollen. Dort liegen ausserdem Dateien, die du selbst pflegst —
`wc-list.txt`, `wc-list-tables.txt`, die Layout-Tabellen und `releasenote.html`. Das
Skript löscht nichts; **niemals `--delete` ergänzen.**

---

# Koordination bei mehreren Agents / Teammates

Dieses Repo hat mehrere **geteilte, nicht partitionierbare** Ressourcen. Teammates
teilen standardmässig dasselbe Arbeitsverzeichnis. Parallele Arbeit ist deshalb nur
unter diesen Regeln erlaubt.

## R1 — Builds laufen ausschliesslich seriell, und nur der Lead baut

Teammates führen **niemals** `make` aus. Sie ändern nur Quelldateien und melden
„fertig" zurück. Der Lead baut einmal am Ende.

Grund — alle Build-Targets schreiben in geteilte Verzeichnisse:
- `f103` und `f411` hängen beide an `configure` und bauen in **dasselbe**
  `build/stm-rgbw-12h` mit `-j4`. Parallel → CMake-Cache-Korruption.
- `esp` baut in **dasselbe** `build/esp8266` (arduino-cli `--build-path`).
- `release-zip` belegt STM **und** ESP **und** app-gz gleichzeitig. Während eines
  Release-Builds darf gar nichts anderes bauen.

## R2 — `app-gz` nur bei stillem Arbeitsverzeichnis

`make app-gz` macht `gzip -9 -k -f` auf `app.js` & Co. Läuft das, während ein
anderer Agent `app.js` noch editiert, entsteht eine **`.gz` einer halb
geschriebenen Datei** — genau der White-Screen-Fehlerfall aus R-Invarianten.
Vor `app-gz`: `git status` prüfen, alle Editier-Tasks müssen abgeschlossen sein.

## R3 — `app.js` hat immer genau einen Besitzer

`ESP8266/ESP-uclock/data/app/app.js` ist eine einzige Datei mit ~12'200 Zeilen und
Sammelpunkt fast aller PWA-Arbeit. Zwei Agents mit Edit auf dieser Datei
überschreiben sich gegenseitig. Pro Runde arbeitet **ein** Agent an `app.js`.
Andere PWA-Aufgaben warten oder laufen gegen `index.html` / `styles.css` / `sw.js`.

## R4 — Versionsnummern und `CACHE_NAME` bumpt nur der Lead

Vier Stellen (Tabelle oben) müssen zueinander passen. Teammates bumpen nichts;
sonst kollidieren zwei unabhängige Erhöhungen und das Release ist inkonsistent.

## R5 — Hardware ist exklusiv

Es gibt eine physische Uhr und einen Serial-Port. Flashen und Live-Test macht der
Nutzer bzw. genau ein Agent. Kein paralleles Flashen, kein paralleler Display-Test.

## R6 — Wer wirklich parallel bauen muss, braucht ein Worktree

Nur wenn eine Aufgabe zwingend eigene Build-Artefakte braucht:
Agent-Tool mit `isolation: "worktree"`. Das entkoppelt `build/` und die
Quelldateien. Kostet Setup-Zeit und Plattenplatz — nicht als Default nutzen.

## Sichere Parallelisierung

Gut parallelisierbar, weil rein lesend oder disjunkt:
- Analyse/Recherche über `src/**` (49 C-Dateien) — z. B. DS18xx-Pfad untersuchen
- Lesen von Logs und Hänger-Mitschnitten
- Arbeit an disjunkten STM-Modulen, solange niemand baut
- Doku

Nicht parallelisierbar: alles unter R1–R5.

## Bekannte Falle im Release-Build

`RELEASE_ZIP` in `Makefile:18` nutzt `date +"%Y-%m-%d-%H%M"` — **Minutengenauigkeit**.
Zwei `release-zip`-Läufe in derselben Minute erzeugen denselben Dateinamen, und
`Makefile:74` macht `rm -f` darauf. Der zweite Lauf überschreibt den ersten
kommentarlos; beide Läufe melden Erfolg. Das erklärt vermutlich frühere
Beobachtungen, dass Zeitstempel/Dateien im Release „nicht aktuell" wirkten.
Bis das behoben ist: nie zwei Release-Builds in derselben Minute starten.

## Offene technische Themen

1. **DS18xx-Messwertvalidierung im STM** (klarster nächster Fix)
   Scratchpad-CRC wird beim Read nicht validiert; „online" heisst nur „beim Init
   gefunden", nicht „letzter Messwert gültig". Gewünscht: CRC-Prüfung im Read,
   separates Flag „letzter Messwert gültig", UI-Status um „Messwert ungültig"
   erweitern. Die UI-Seite ist bereits korrigiert (STM-Fehlerwert `255` wurde
   fälschlich als „127.5 °C" formatiert).
   Dateien: `src/ds18xx/ds18xx.c`, `src/tempsensor/tempsensor.c`, `src/vars/vars.c`,
   `src/main.c`, `data/app/app.js`

2. **Sporadische Hänger auf BlackPill STM32F411 + neues LED-Board** — nicht gelöst,
   nicht bewiesen. BluePill F103 läuft stabiler. Bild: Uhr steht, Web-UI zeigt
   eingefrorene Zeit, STM-Reset per Web-UI geht meist noch, teils Hänger exakt bei
   Anzeige von „IP" in der Startsequenz.
   **Bewusste Entscheidung: keinen pauschalen DMA-Fix und keinen Recovery-Mechanismus
   einbauen.** Die Logs zeigten keinen klaren DMA-Stillstand. Erst per gezielterer
   Instrumentierung erhärten. Mechanische Kontaktprobleme (Stiftleiste statt
   Lötverbindung) sind als Ursache noch im Rennen.
