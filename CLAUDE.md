# wordclock24h — Arbeits- und Koordinationsregeln

Sprache: **Deutsch**, Schweizer Schreibung (`ss` statt `ß`), mit **echten Umlauten** —
in den Antworten an den Nutzer, in der UI der PWA und in Commit-Botschaften.

**Eine Ausnahme, und sie ist technisch, nicht sprachlich:** Die C-Dateien unter
`src/**` und `ESP8266/ESP-uclock/*.cpp` sind ASCII oder ISO-8859-1, nicht UTF-8.
Dort gilt die Umschrift (`Geraet`, `waehrend`) — nicht weil die Sprache es verlangt,
sondern weil ein Werkzeug, das ein `ä` in eine ISO-8859-1-Datei schreibt und dabei
UTF-8 annimmt, die Datei beschädigt. Das ist hier bereits passiert.

Wer in derselben Sitzung an beiden Welten arbeitet, trägt die Gewohnheit hinüber.
Genau so kamen „Schluessel" und „ungueltig" in die deutschen Fehlertexte der PWA
(`BEFUNDE.md`, L52). **Geprüft wird das jetzt** — `tools/checks/umlaute.mjs` in
Stufe S8 meldet Umschrift in den deutschen PWA-Texten, anhand einer benannten Liste
statt anhand der Buchstabenfolge: Sonst schlüge sie bei „aktuell", „Quelle",
„Steuerung" und „zuerst" an, und eine Prüfung mit Fehlalarmen liest niemand mehr.

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

## Abläufe liegen als Skills bereit

Die ausführlichen Abläufe stehen nicht mehr hier, sondern unter `.claude/skills/`.
Sie laden nur, wenn sie gebraucht werden — das hält den Dauerkontext klein, ohne dass
Wissen verloren geht.

| Skill | Wofür | Lädt |
|---|---|---|
| `/release` | Build, Versionspflicht DIR-004, Rollout DIR-005, was zu flashen ist | bei Bedarf |
| `/pwa-vorschau` | PWA ohne Gerät ansehen und vermessen | bei Bedarf |
| `/doku-nachfuehren` | CHANGELOG, READMEs, Befundkatalog, DIR-006 | bei Bedarf |
| `stm-firmware` | belegtes Detailwissen zur STM-Firmware und zur Platine | automatisch bei Arbeit an `src/**` |

Für den vollständigen PWA-Durchlauf gibt es keinen Skill, sondern einen **Agenten**:
`pwa-tester` arbeitet `TESTPLAN-PWA.md` ab. Einsetzen nach grösseren Umbauten an
`app.js`, `http.cpp` oder der Display-Zustandsmaschine und vor einem Release, das mehr
als eine Komponente berührt. Er fährt die Phasen 0 bis 4 und 9; Backup-Import,
Verbindungstrennung und die gefährlichen Funktionen bleiben beim Nutzer. Zwei
PreToolUse-Hooks weisen die gefährlichen Endpunkte ab, bevor der Agent sie erreicht
(`tools/hooks/no-danger.py`).

Die Kurzregeln bleiben hier, weil sie immer gelten:

- **Kein Build ohne Versionserhöhung der geänderten Komponenten (DIR-004).** Kein
  Gleichschritt — ändert ein Release nur den STM-Code, steigt nur dessen Version.
  `APP_VERSION` und `CACHE_NAME` gehören dagegen immer zusammen.
- **Nach jeder relevanten Änderung kompletter Build und Release-ZIP**, nicht nur
  `app-gz`. Immer explizit sagen, was zu flashen ist.
- **Das fertige Fabrikat wird auf die Synology ausgerollt (DIR-005)**, Ziel
  `/volume1/web/wordclock/test8`. Das Skript löscht nichts; **niemals `--delete`
  ergänzen** — dort liegen Dateien, die der Nutzer selbst pflegt.
- **Dokumentation ist lebend oder Momentaufnahme (DIR-006).** Lebend: `CLAUDE.md`,
  `BEFUNDE.md`, `CHANGELOG.md`, alle `README*.md`, `knowledge/**`, `.claude/**`.
  `HARDWARE.md`, `TESTPLAN-PWA.md`. Momentaufnahme mit Datum, wird nicht fortgeschrieben: `REVIEW*.md`,
  `gap-analysis.md`, `specs/**`. **In lebende Dokumente gehören keine
  Versionsnummern** — eine Kopie des Standes veraltet still. Guardrail S9 prüft das.

### Die PWA kommt nicht durch den Rollout aufs Gerät

`tools/deploy.sh` bringt die Assets auf den **Update-Server**. Auf der Uhr liegen sie
im LittleFS und müssen eigens hochgeladen werden:

```
./tools/install-app.sh --check     Version und abgelegte Dateien gegen die Weissliste
./tools/install-app.sh             lädt hoch
```

**Nach jedem ESP-Update prüfen.** Ein Firmware-Wechsel löscht das Dateisystem nicht —
das ist am 02.10.2026 direkt belegt: Nach dem OTA lieferte das Gerät `app.js`
unverändert aus, bevor irgendetwas neu hochgeladen wurde. Aber der ESP sucht
**ausschliesslich** nach dem abgeflachten `.gz`-Namen, und `APP_INSTALL_ASSETS` in
`http.cpp` ist eine Weissliste. Ändert sich ein Name oder kommt ein Asset dazu, ist
die Datei nicht weg — sie wird nur nicht mehr gefunden, und die PWA wirkt
verschwunden. Genau so ist es am 29.04.2026 beim Umstieg auf `.gz`-only passiert
(`BEFUNDE.md`, L21). `--check` nennt jede fehlende Datei beim Namen.

Das ist lange übersehen worden: Am 02.10.2026 lief auf dem Gerät noch **1.4.69**,
während Repo und Server bei 1.4.71 standen — alle PWA-Korrekturen der Tage davor waren
nirgends wirksam. `/api/update_download_assets` hilft nicht, obwohl der Name es
nahelegt: Der Endpunkt lädt nur die Icon- und Wetterdatei nach und meldet trotzdem
`{"ok":true}`.

### STM32 flashen (DIR-010)

```
./tools/flash-stm.sh --check     prüft, ob alles bereitliegt
./tools/flash-stm.sh             flasht und setzt danach zurück
```

**Nicht von Hand `/api/remote_stm32_flash` aufrufen.** Drei Dinge sieht man dem
Endpunkt nicht an, und alle drei führen dazu, dass er **stillschweigend nichts tut**:

- **`filename` ist Pflicht.** Fehlt er, setzt `http_api_remote_stm32_flash()`
  `error_code = 2` und bricht ab. Der Aufruf sieht erfolgreich aus, die Firmware
  bleibt alt. Am 03.10.2026 genau so passiert — bemerkt hat es der Nutzer, nicht die
  Prüfung. Den richtigen Namen meldet das Gerät selbst als `stm32_default` in
  `/api/update_status`; er hängt an der erkannten Hardware.
- **Ist `HARDWARE_CONFIGURATION` gleich 65535**, bildet der ESP gar keinen
  Dateinamenfilter und weist **jeden** Namen ab. Diesen Zustand hinterlässt ein
  ESP-Neustart (`BEFUNDE.md`, L42). Dann zuerst den STM zurücksetzen, dann flashen.
- **Nach dem Flashen muss der STM zurückgesetzt werden.** Der ESP meldet selbst
  „STM32-Flash abgeschlossen. Warte auf Reset" — er löst ihn aber nicht aus. Ohne
  Reset läuft die alte Firmware weiter. Der Reset ist Teil des Vorgangs, kein
  Nachklapp.

Das Skript erzwingt alle drei Punkte, prüft Quelle und Dateigrösse auf dem Server
vorher, und weist am Ende die gemeldete Version nach.

### Nach dem Flashen prüfen (DIR-009)

```
./tools/smoke-device.sh
```

**Nach jedem ESP-Update gehört die Update-Quelle geprüft.** Verliert der ESP bei
seinem Neustart den Variablenspeicher (`BEFUNDE.md`, L42), sind Host und Pfad leer,
und er fällt auf seine eingebauten Vorgaben zurück — die zeigen auf den Server des
**Ursprungsprojekts**. Das nächste Update holte damit fremde Firmware, ohne jede
Fehlermeldung. Am 03.10.2026 eingetreten. Der Smoketest prüft das inzwischen selbst
gegen `DEVICE_UPDATE_HOST`/`DEVICE_UPDATE_PATH` aus `tools/device.conf`; ohne diese
Werte in der Konfiguration meldet er nur leere Felder, nicht die falsche Quelle.

**Die Guardrail-Stufen sind ausnahmslos statisch** — sie lesen Quelltext und sprechen
nicht mit dem Gerät. Nach einer Änderung an `http.cpp`, dessen Parameter-Auswertung
*jeder* Request durchläuft, sagen sie nichts darüber, ob die Uhr noch funktioniert.

Der Smoketest prüft am Gerät: Erreichbarkeit, gemeldete Versionen, alle PWA-Assets,
elf lesende API-Endpunkte auf die **jeweils erwartete Antwortform**, die
Legacy-Oberfläche, die Absturzfestigkeit bei Parametern ohne `=` (ESP 3.2.3) und die
Abwehr eingebetteter Wartungsaufrufe (ESP 3.2.4).

**Ausschliesslich lesend** (DIR-008). Einzige Ausnahme ist `maintenance_reset_stm32`
mit `Sec-Fetch-Dest: image` — dort *muss* 403 kommen, und käme stattdessen 200, würde
lediglich der STM neu starten.

### Warum es zwei Review-Dokumente gibt

`REVIEW.md` (2026-08-12) deckt PWA-Korrektheit, PWA↔STM-Display und UI/UX ab,
`REVIEW-2026-09-29.md` den API-Vertrag PWA↔ESP, die STM-Restmodule, die UI über die
Gerätespanne und die Kodierung. Review 2 wiederholt Review 1 nicht, sondern schliesst
dessen ausdrücklich benannte Lücken. Den **aktuellen Stand** aller 34 Massnahmen führt
`BEFUNDE.md` — die Reviews selbst bleiben unverändert.

# Koordination bei mehreren Agents / Teammates

Dieses Repo hat mehrere **geteilte, nicht partitionierbare** Ressourcen. Teammates
teilen standardmässig dasselbe Arbeitsverzeichnis. Parallele Arbeit ist deshalb nur
unter diesen Regeln erlaubt.

## R1 — Builds laufen ausschliesslich seriell, und nur der Lead baut

Teammates führen **niemals** `make` aus. Sie ändern nur Quelldateien und melden
„fertig" zurück. Der Lead baut einmal am Ende.

**Das ist seit 2026-09-30 erzwungen, nicht mehr nur aufgeschrieben.** Alle schreibenden
Agenten ausser dem `release-engineer` tragen im Frontmatter einen PreToolUse-Hook
(`tools/hooks/no-build.py`), der `make`, `cmake`, `arduino-cli` und
`guardrails.sh --full` abweist. Harmlose Ziele wie `make stm-version-file` bleiben
erlaubt.

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

## R3 — Jede Datei hat genau einen Schreiber, **den Lead eingeschlossen**

Die PWA-Hauptdatei ist eine einzige Datei mit über 12'000 Zeilen und Sammelpunkt
fast aller PWA-Arbeit. Zwei Schreiber in derselben Datei überschreiben sich
gegenseitig, und das fällt erst auf, wenn der Stand schon kaputt ist.

**Die Regel lautete früher „pro Runde arbeitet ein Agent daran".** Das regelte die
Gleichzeitigkeit *unter Agenten* und sagte über den Lead nichts — und genau diese
Lücke ist am 03.10.2026 genutzt worden: Der Lead hat während eines laufenden
Auftrags selbst in dieser Datei geschrieben. Der zuständige Agent fand die Korrektur
vor und brach seinen Patch ab, weil **sein** Skript jeden Anker auf „genau einmal"
prüft. Hätte er stur ersetzt, stünden zwei konkurrierende Implementierungen
derselben Prüfung darin. Verhindert hat das seine Sorgfalt, nicht die Regel.

**Der Lead schreibt keinen Produktcode.** Er baut, prüft, pflegt Werkzeug und
Dokumentation, bumpt Versionen und rollt aus. Alles andere geht an den zuständigen
Agenten — auch wenn ein Auftrag einmal nicht ankommt und es schneller ginge, selbst
Hand anzulegen. Genau diese Abkürzung hat die Regel gebrochen.

**Für die Agenten erzwungen.** `tools/hooks/file-ownership.py` weist jeden
Schreibzugriff auf eine fremde Datei ab, registriert im Frontmatter jedes
schreibenden Agenten. Geprüft werden `Write`, `Edit` **und `Bash`**: Die Agenten
patchen durchgehend über Python-Heredocs, ein Hook nur auf `Write`/`Edit` hätte gar
nichts gesehen. Lesen bleibt frei.

**Für den Lead gilt die Regel, nicht der Zwang — und das ist eine benannte Lücke.**
Der erste Versuch registrierte den Hook zusätzlich projektweit in
`.claude/settings.json` mit `--agent lead`. Gemessen am 03.10.2026: Ein dort
eingetragener Hook feuert **auch innerhalb jeder Unteragenten-Sitzung**, und die
Nutzlast enthält kein Feld, das beide unterscheidet — `session_id` und
`transcript_path` sind identisch. Ein einziges Deny blockiert den Aufruf, also
sperrte ausgerechnet der Hook, der dem `stm-developer` die Hoheit über `src/**`
sichern soll, genau ihn davon aus; sein erster Auftrag scheiterte daran. Der
Eintrag ist wieder entfernt. Erzwingbar wäre es nur über eine Verständigung der
beiden Hook-Aufrufe via `tool_use_id`, und die hinge an einer Reihenfolge, die
nirgends zugesichert ist.

Jedes Erkennungsmuster muss den Pfad enthalten. Der erste Entwurf prüfte auf
`open(p,'w')` ohne Pfadbezug und blockierte damit eine Änderung an dieser Datei
hier, nur weil im neuen Text ein fremder Dateiname vorkam. Ein Hook, der die eigene
Dokumentation blockiert, wird umgangen statt befolgt.

| Wer | Schreibt |
|---|---|
| `stm-developer` | `src/**` |
| `esp-developer` | `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` |
| `pwa-developer` | die PWA-Logik: Hauptdatei und Service Worker |
| `ui-developer` | Markup, Stilvorlage, Manifest, `icons/**` |
| `doc-writer` | `*.md`, `knowledge/**` |
| `spec-writer` | `specs/**` |
| `release-engineer` | `Makefile`, `cmake/**`, die vier Versionsdateien |
| Lead | `tools/**`, `.claude/**`, Doku, Build, Rollout |

## R4 — Versionsnummern und `CACHE_NAME` bumpt nur der Lead

Vier Stellen (Tabelle oben) müssen zueinander passen. Teammates bumpen nichts;
sonst kollidieren zwei unabhängige Erhöhungen und das Release ist inkonsistent.

## R5 — Hardware ist exklusiv, und die Uhr laeuft produktiv

Es gibt eine physische Uhr und einen Serial-Port. Flashen und Live-Test macht der
Nutzer bzw. genau ein Agent. Kein paralleles Flashen, kein paralleler Display-Test.

**Die Uhr ist unter `http://$DEVICE_HOST/app/` erreichbar (DIR-008).** Die Adresse
steht in `tools/device.conf` (gitignored, Vorlage `tools/device.conf.example`) und
bewusst nirgends im Repo — es ist öffentlich, und interne Netzstruktur gehört dort
nicht hinein. Lesende Abfragen sind frei. Alles Schreibende braucht die ausdrückliche Freigabe des Nutzers
im selben Gespräch — es ist seine Uhr im Dauerbetrieb, kein Testgerät.

Diese Endpunkte sind aus unseren eigenen Befunden heraus gefährlich:

| Endpunkt | Was passiert |
|---|---|
| `GET /?a` (Parameter **ohne** `=`) | **ESP stürzt ab.** Nie senden, auch nicht versehentlich |
| `/api/test_display` | 45 s Blockade ⇒ garantierter Watchdog-Reset |
| `/api/learn_ir` | unbegrenzte Blockade ⇒ garantierter Watchdog-Reset |
| `/api/maintenance_reset_eeprom`, `/api/maintenance_format_fs` | Datenverlust, PWA weg |
| `/api/fs_remove?filename=app.js.gz` | löscht die PWA vom Gerät |
| Backup-**Import** | kann das Gerät ohne WLAN, ohne AP und ohne Webserver zurücklassen |

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

`RELEASE_ZIP` in `Makefile:18` nutzt Minutengenauigkeit, `Makefile:74` macht `rm -f`
darauf. Zwei Release-Builds in derselben Minute überschreiben sich **kommentarlos**,
beide melden Erfolg. Bis das behoben ist (`BEFUNDE.md`, L2): nie zwei Release-Builds in
derselben Minute starten. Ausführlich im Skill `/release`.

## Hardware

Die Platine ist eine Eigenentwicklung — **WordClock USB-C / STM32F411 V2**, kein
BlackPill-Modul mit Zusatzplatine, aber bewusst pinkompatibel dazu. Pinbelegung,
Signalwege und der Abgleich gegen den Firmware-Code stehen in `HARDWARE.md`, die
Quelle ist das KiCad-Projekt unter `~/Documents/WordClock-USB C - STM32F411 - V2`.

Drei Punkte, die man dem Code allein nicht ansieht:

- **Das EEPROM hängt am I2C** (AT24C32M, zusammen mit der DS3231-RTC). Der STM32F411
  hat keins. Die oft zitierten „16 ms pro Byte" sind der EEPROM-Schreibzyklus.
- **`PB0` schaltet die 5-V-Versorgung der LED-Kette.** `U3` (SN74AHCT1G125) hebt die
  Datenleitung von 3,3 V auf 5 V — das frühere Pegelproblem ist auf V2 gelöst.
- **Der ESP hat nur einen vollwertigen UART, und das ist die Brücke zum STM.** Jede
  Debugzeile des ESP landet deshalb zwangsläufig auf der STM-UART.

## Offene technische Themen

Der **vollständige Massnahmenkatalog** steht in `BEFUNDE.md` — 34 Massnahmen aus den
beiden Reviews plus die Befunde aus der laufenden Arbeit, jeweils mit Status und
nachprüfbarem Beleg. Die beiden Themen hier stehen zusätzlich, weil sie den
Projektkontext tragen, den man dem Code nicht ansieht.

**Was als Nächstes ansteht, steht im Abschnitt „ToDo" ganz oben in `BEFUNDE.md`** —
die Arbeitsliste, nach Aufwand und Risiko sortiert. Die Tabellen darunter führen den
Stand, die Liste führt die Arbeit. Guardrail S10 prüft, dass dort **jeder** offene
Befund genannt ist; eine Zeile auf „offen" zu setzen, ohne sie in die Liste zu nehmen,
schlägt fehl.

Die schwersten offenen Punkte aus dem Katalog, damit sie nicht untergehen:
`normalize_http_parameters` ohne Null-Prüfung (ESP-Absturz per `GET /?a`, aus dem
ganzen LAN auslösbar), Backup-Import ohne Guard (kann das Gerät ohne WLAN, ohne AP
und ohne Webserver zurücklassen) und `watchdog_reload()` mit weiterhin genau **einer**
Aufrufstelle.

1. **DS18xx-Messwertvalidierung im STM** (klarster nächster Fix)
   **Neu belegt (01.10.2026):** `temp_init()` läuft genau einmal beim Start
   (`main.c:3140`). Scheitert die Erkennung dort, liefert der Sensor bis zum nächsten
   Reset nur den Fehlercode — am Gerät beobachtet, 11 Fehlerwerte vor einem Reset,
   1506 gültige danach. Es braucht deshalb **beides**: CRC-Prüfung beim Lesen **und**
   erneute Erkennung zur Laufzeit.
   Scratchpad-CRC wird beim Read nicht validiert; „online" heisst nur „beim Init
   gefunden", nicht „letzter Messwert gültig". Gewünscht: CRC-Prüfung im Read,
   separates Flag „letzter Messwert gültig", UI-Status um „Messwert ungültig"
   erweitern. Die UI-Seite ist bereits korrigiert (STM-Fehlerwert `255` wurde
   fälschlich als „127.5 °C" formatiert).
   Dateien: `src/ds18xx/ds18xx.c`, `src/tempsensor/tempsensor.c`, `src/vars/vars.c`,
   `src/main.c`, `data/app/app.js`

2. **Sporadische Hänger auf BlackPill STM32F411 + neues LED-Board** — nicht gelöst.
   BluePill F103 läuft stabiler. Bild: Uhr steht, Web-UI zeigt eingefrorene Zeit,
   STM-Reset per Web-UI geht meist noch, teils Hänger exakt bei Anzeige von „IP" in
   der Startsequenz.

   **Ein Hänger wurde am 30.09.2026 vollständig mitgeschnitten** (`haenger-2026-09-30.md`).
   Ergebnis: **kein Watchdog-Reset über 18 Minuten**, und mitten im Hänger wurde ein
   Ticker-Kommando noch vollständig ausgeführt. Der Hauptloop lief also, ausgefallen war
   nur der **zeitgesteuerte Zweig** — `show_time`, `read rtc`, Temperatur, Refresh.
   Damit ist die Blockade-Spur für diesen Fall tot. Die neue Frage lautet: Was macht den
   periodischen Zweig unerreichbar, während der Rest weiterläuft?
   **Bewusste Entscheidung: keinen pauschalen DMA-Fix und keinen Recovery-Mechanismus
   einbauen.** Die Logs zeigten keinen klaren DMA-Stillstand. Erst per gezielterer
   Instrumentierung erhärten. Mechanische Kontaktprobleme (Stiftleiste statt
   Lötverbindung) sind als Ursache noch im Rennen.
