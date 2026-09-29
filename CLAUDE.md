# wordclock24h — Arbeits- und Koordinationsregeln

Sprache: **Deutsch**, mit echten Umlauten — auch in der UI und in Commit-Messages.

Anrede: durchgehend **Du-Form**, niemals „Sie". Das gilt **in erster Linie für die
Antworten an den Nutzer** — er wird geduzt. Ebenso für die UI-Texte der PWA, für
Meldungen und Fehlertexte. Bestehende Du-Formulierungen nicht auf „Sie" umschreiben.

## Versionsstände (Single Source of Truth)

| Was | Datei | Symbol |
|---|---|---|
| STM/WordClock | `src/main.h` | `#define VERSION` |
| ESP | `ESP8266/ESP-uclock/version.h` | `#define ESP_VERSION` |
| PWA-App | `ESP8266/ESP-uclock/data/app/app.js` | `const APP_VERSION` |
| SW-Cache | `ESP8266/ESP-uclock/data/app/sw.js` | `const CACHE_NAME` |

**Hier stehen bewusst keine Versionsnummern.** Eine Kopie des Standes in der Doku
veraltet still — in `README-CMAKE.md` stand über Monate `3.2.0 / 3.2.0 / 1.2.43`,
ohne dass es jemandem auffiel. Den gültigen Stand zeigt `./tools/guardrails.sh`
in Stufe S4, und Stufe S9 prüft, dass keine Doku wieder eine eigene Kopie anlegt.

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

**Jede Komponente wird genau dann versioniert, wenn sich ihr Code geändert hat —
kein Gleichschritt.** Ändert ein Release nur den STM-Code, steigt nur die
STM-Version; ESP und PWA bleiben stehen.

| Komponente | Anheben | Versionsdatei | Quellen |
|---|---|---|---|
| STM32 | `VERSION` | `src/main.h` | `src/**`, `CMakeLists.txt`, `cmake/**` |
| ESP8266 | `ESP_VERSION` | `ESP8266/ESP-uclock/version.h` | `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` |
| PWA | `APP_VERSION` **und** `CACHE_NAME` | `data/app/app.js`, `sw.js` | `data/app/**` ohne `.gz` |

Die Trennung von ESP und PWA ist dabei der wunde Punkt: beide liegen unter
`ESP8266/`, sind aber getrennt versioniert und getrennt auszuliefern. Eine Prüfung,
die `ESP8266` als Ganzes betrachtet, hält jede PWA-Änderung für eine ESP-Änderung.
Die `.gz` sind Ableitungen und zählen nicht als Quelle.

Beide Richtungen sind falsch:
- **Code geändert, Version steht** → das Fabrikat ist keinem Commit mehr zuzuordnen.
- **Version angehoben, Code unverändert** → die Uhr bietet ein OTA-Update auf
  identische Firmware an. Ungefährlich, aber ein Flash ohne Gegenwert.

`APP_VERSION` und `CACHE_NAME` gehören dagegen **immer** zusammen, unabhängig von
DIR-004: ohne `CACHE_NAME`-Bump liefert der Service Worker neue `index.html` mit
alter `app.js`. Das ist das einzige Kritisch-Finding in S4, der Rest ist Hoch.

Guardrail-Stufe S4 prüft das gegen das letzte `release/*`-Tag, nicht gegen den
letzten Commit — ein Bump kann mehrere Commits zurückliegen und wäre gegen `HEAD`
unsichtbar. Beim Vergleich wird die Versionszeile aus dem Diff ihrer eigenen Datei
herausgefiltert; sonst wäre jeder Bump für sich schon eine „Codeänderung" und die
Gegenprobe könnte nie anschlagen.

### Dokumentation nachführen (DIR-006)

Die Dokumentation zerfällt in zwei Sorten, und die Unterscheidung ist die ganze Regel.

**Lebend** — wird nachgeführt, darf nie veralten:
`CLAUDE.md`, `BEFUNDE.md`, `CHANGELOG.md`, alle `README*.md`, `knowledge/**`,
`.claude/agents/**`.

**Momentaufnahme** — trägt ein Datum und wird **nicht** fortgeschrieben:
`REVIEW.md`, `REVIEW-2026-09-29.md`, `gap-analysis.md`, `specs/**`.

Nach jedem Release führt der `doc-writer` `CHANGELOG.md` nach, bei neuen Werkzeugen
oder Abläufen die README-Dateien, und bei geschlossenen Befunden `BEFUNDE.md`. Ein
Release gilt erst als fertig, wenn der Changelog-Eintrag steht.

**Versionsnummern gehören nicht in lebende Dokumente.** Eine Kopie des Standes
veraltet still: der Kopf von `README-CMAKE.md` lag monatelang rund dreissig
PWA-Versionen hinter den Quellen, ohne dass es auffiel. Eine Anweisung
an einen Agenten („führe die Doku nach") hat das nicht verhindert und wird es nicht —
das ist eine Absichtserklärung, keine Prüfung. Deshalb prüft **Guardrail-Stufe S9**
die lebenden Dokumente gegen die Quellen und meldet zusätzlich absolute
Benutzerpfade. Eine bewusst historische Angabe in einem lebenden Dokument wird mit
`<!-- historisch -->` am Zeilenende markiert.

### Warum es zwei Review-Dokumente gibt

`REVIEW.md` (2026-08-12) deckt PWA-Korrektheit, PWA↔STM-Display und UI/UX ab,
`REVIEW-2026-09-29.md` den API-Vertrag PWA↔ESP, die STM-Restmodule, die UI über die
Gerätespanne und die Kodierung. Review 2 wiederholt Review 1 nicht, sondern schliesst
dessen ausdrücklich benannte Lücken. Den **aktuellen Stand** aller 34 Massnahmen führt
`BEFUNDE.md` — die Reviews selbst bleiben unverändert.

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

Der **vollständige Massnahmenkatalog** steht in `BEFUNDE.md` — 34 Massnahmen aus den
beiden Reviews plus die Befunde aus der laufenden Arbeit, jeweils mit Status und
nachprüfbarem Beleg. Die beiden Themen hier stehen zusätzlich, weil sie den
Projektkontext tragen, den man dem Code nicht ansieht.

Die schwersten offenen Punkte aus dem Katalog, damit sie nicht untergehen:
`normalize_http_parameters` ohne Null-Prüfung (ESP-Absturz per `GET /?a`, aus dem
ganzen LAN auslösbar), Backup-Import ohne Guard (kann das Gerät ohne WLAN, ohne AP
und ohne Webserver zurücklassen) und `watchdog_reload()` mit weiterhin genau **einer**
Aufrufstelle.

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
