# wordclock24h — Arbeits- und Koordinationsregeln

Sprache: **Deutsch**, Schweizer Schreibung (`ss` statt `ß`), mit **echten Umlauten** —
in den Antworten an den Nutzer, in der UI der PWA und in Commit-Botschaften.

**Eine Ausnahme, und sie ist technisch, nicht sprachlich (DIR-015):** Die Quelldateien unter
`src/**` und `ESP8266/ESP-uclock/` sind überwiegend ASCII oder ISO-8859-1.
Dort gilt die Umschrift (`Geraet`, `waehrend`) — nicht weil die Sprache es verlangt,
sondern weil ein Werkzeug, das ein `ä` in eine ISO-8859-1-Datei schreibt und dabei
UTF-8 annimmt, die Datei beschädigt. Das ist hier bereits passiert.

**„Überwiegend" ist wörtlich gemeint — prüf die Datei, bevor du sie schreibst.** Hier
stand bis zum 03.10.2026 pauschal „`ESP8266/ESP-uclock/*.cpp` sind ASCII oder
ISO-8859-1", und das ist falsch: `http.cpp` (37 Nicht-ASCII-Bytes) und
`stm32flash.cpp` (48) sind **UTF-8**. Wer sie nach dieser Regel mit einem
`latin-1`-Patcher anfasst, beschädigt sie — derselbe Schaden wie oben, nur in der
Gegenrichtung. Aufgefallen ist es einem Agenten, der nachgesehen hat, statt der
Anweisung zu folgen. Die Umschrift in neuem Text schadet in keiner der beiden
Welten; die **Kodierungsannahme beim Patchen** ist das Gefährliche.

**Dasselbe gilt für die Zeilenenden, und das ist seit dem 04.10.2026 belegt.**
`ESP-uclock.ino` hat **gemischte** Zeilenenden — 1043 CRLF von 1240 Zeilen, der Rest
LF; `http.cpp` umgekehrt genau eine CRLF-Zeile. Ein Patch über das Edit-Werkzeug
vereinheitlicht sie **stillschweigend**: aus neun inhaltlich geänderten Zeilen wurden
432. Aufgefallen ist es dem Umsetzer, der zurückgesetzt und byte-genau in Binärform
gepatcht hat. Der Schaden ist nicht kosmetisch — ein Diff mit 432 Zeilen ist nicht
mehr prüfbar, ein echter Fehler darin fällt niemandem auf, und `git blame` zeigt
danach für die ganze Datei den falschen Commit. **Vor dem Patchen das ortsübliche
Zeilenende der Datei feststellen, nicht nur ihre Kodierung.**

Wer in derselben Sitzung an beiden Welten arbeitet, trägt die Gewohnheit hinüber.
Genau so kamen „Schluessel" und „ungueltig" in die deutschen Fehlertexte der PWA
(`BEFUNDE.md`, L52). **Geprüft wird das jetzt** — `tools/checks/umlaute.mjs` in
Stufe S8 meldet Umschrift in den deutschen PWA-Texten, anhand einer benannten Liste
statt anhand der Buchstabenfolge: Sonst schlüge sie bei „aktuell", „Quelle",
„Steuerung" und „zuerst" an, und eine Prüfung mit Fehlalarmen liest niemand mehr.

Anrede: durchgehend **Du-Form**, niemals „Sie". Das gilt **in erster Linie für die
Antworten an den Nutzer** — er wird geduzt. Ebenso für die UI-Texte der PWA, für
Meldungen und Fehlertexte. Bestehende Du-Formulierungen nicht auf „Sie" umschreiben.

**Die Kennungen in Klammern sind keine Zierde.** Sie wurden am 05.10.2026 nachgetragen,
weil `knowledge/directives.md` zwar DIR-000 bis DIR-014 fuehrte, die vier Regeln hier aber
keine Nummer hatten — und **was keine Nummer hat, kann niemand zitieren**. Aufgefallen ist
es einem Agenten, der eine Direktive belegen wollte und keine fand. Guardrail S11 prueft
seither, dass jede zitierte Kennung auch im Katalog steht; die Gegenrichtung prueft sie
bewusst nicht, denn eine Regel darf gelten, ohne zitiert zu werden.

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
- Ticker sind asynchron: `pending_ticker_restore` in `src/main.c` (früher
  `pending_weather_ticker_restore` — er trägt seit L211 alle Ticker, nicht nur den Wetterticker).
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
| `/doku-nachfuehren` | CHANGELOG, READMEs, Befundkatalog, DIR-006, DIR-018 | bei Bedarf |
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
- **Nach jeder relevanten Änderung kompletter Build und Release-ZIP (DIR-002)**, nicht nur
  `app-gz`. Immer explizit sagen, was zu flashen ist — **und in welcher Reihenfolge,
  wenn eine Komponente die andere voraussetzt.** Setzt eine PWA-Änderung eine
  Firmware-Änderung voraus, kommt die **Firmware zuerst**. Am 04.10.2026 belegt: Die
  neue Oberfläche meldete auf alter Firmware „Datei ist leer — das ist kein Fehler,
  die Datei gibt es", wo die Datei in Wahrheit fehlte (`BEFUNDE.md`, L241). Sie war
  damit für das Zeitfenster zwischen den beiden Einspielvorgängen **schlechter als
  ihre Vorgängerin**.
- **Das fertige Fabrikat wird auf die Synology ausgerollt (DIR-005)**, Ziel
  `/volume1/web/wordclock/test8`. Das Skript löscht nichts; **niemals `--delete`
  ergänzen** — dort liegen Dateien, die der Nutzer selbst pflegt.
- **Die PWA wird im Browser geprüft, nicht nur über die API (DIR-016).** `./tools/check-pwa.sh`
  lädt sie vom Gerät in einen echten Browser und meldet unter anderem, ob `app.js`
  beim Laden einen **Fehler wirft** — dann bleibt die Oberfläche halb leer, und
  weder API noch Smoketest noch Screenshot zeigen das. Bis 03.10.2026 hat das kein
  Durchlauf geprüft, obwohl an `app.js` ständig gearbeitet wird.
- **Waehrend eines Testlaufs wird der Mitschnitt mitgelesen (DIR-013).** `./tools/watch-log.sh`
  laeuft parallel und meldet Exceptions, Neustarts, Watchdog-Resets, Spruenge in den
  verworfenen Zeichen (`d=`) und ein Stillstehen der `diag`-Folge. Am 03.10.2026 lief
  ein vollstaendiger Durchlauf, und mitten darin stuerzte der ESP ab — bemerkt hat es
  **der Nutzer** im Mitschnitt, nicht die Pruefung. Seine Forderung danach: „Zudem
  erwarte ich von dir, dass du eigentlich waehrend der Tests die Logs ueberwachst und
  auch laufend auswertest wenn du testest!" **Das Problem ist nicht der uebersehene
  Absturz, sondern der Bericht, der sauber meldet, waehrend das Geraet zwischendurch
  neu gestartet ist** — er erzeugt Vertrauen, das nicht gedeckt ist. Gleiches gilt fuer
  jede Messung am Geraet, nicht nur fuer den vollen Durchlauf.
- **Eine neu gebaute Prüfung ist erst fertig, wenn sie einmal fehlgeschlagen ist
  (DIR-014).** Nicht „der Code sieht richtig aus", sondern: einmal eine Verletzung
  herstellen und sehen, dass die Prüfung anschlägt — und dass ihre Meldung **ankommt**.
  Am 03./04.10.2026 ist dieselbe Gattung **fünfmal** aufgetreten: Die Logwache schrieb
  in eine gepufferte Pipe und ihre Datei blieb 0 Byte (`BEFUNDE.md`, L181). Die
  Heap-Zeile lief am API-Ring vorbei, 36 Zeilen im Mitschnitt gegen 0 über die API
  (L185). Die Messmarken-Prüfung schrieb auf stderr und gab dann `return 0` zurück —
  bei exit 0 liest das niemand (L191). Eine Wache wurde mit `| tail` gestartet, das
  erst beim Streamende ausgibt (L194). Und die Eindeutigkeitsprüfung für
  ToDo-Kennungen stürzte mit `ReferenceError` ab, **nachdem** die Stufe „OK" gemeldet
  hatte (L224). **In allen fünf Fällen war der Mechanismus geprüft und der Weg der
  Meldung bis zum Empfänger nicht.** Dreimal hat es der Nutzer oder ein Agent bemerkt,
  nicht der Erbauer.

  **Und sie muss ihren Gegenstand vollständig sehen.** Am 04.10.2026 kam ein sechster
  Fall dazu, eine Stufe schärfer als die fünf davor: Die Prüfung gegen veraltete
  ToDo-Einträge lief, meldete OK — und ihr Muster `[A-F]\d+` sah die Kennungen mit
  Buchstabensuffix (`B1b`, `C6c`) **gar nicht**. Nach der Korrektur meldete sie 28
  Einträge, über ein Fünftel der Arbeitsliste (`BEFUNDE.md`, L235). Eine Prüfung mit zu
  engem Muster ist schlimmer als keine: Sie erzeugt genau das Vertrauen, das sie nicht
  deckt. **Zähl deshalb nach, wie viele Fälle die Prüfung überhaupt betrachtet**, und
  vergleich die Zahl mit dem, was es geben müsste.

- **Eine Ankündigung im Schlusssatz ist eine Zusage für DIESEN Turn (DIR-019).**
  „Als Nächstes mache ich X" und dann Turn-Ende ist keine Planung, sondern eine
  Unterbrechung, die der Nutzer auflösen muss — und er hat nichts entschieden,
  worauf zu warten wäre. Am 05.10.2026 dreimal hintereinander passiert, zuletzt
  wörtlich „Ich fange damit an", gefolgt von nichts. Seine Frage danach: **„Wieso
  muss ich dich immer wieder auffordern weiterzufahren?"** Wer weiterarbeiten kann,
  arbeitet weiter, statt es anzukündigen. Wer es **nicht** kann, schreibt **warum** —
  welche Entscheidung, welche Freigabe, welches Gerät fehlt. Der Stop-Hook prüft den
  letzten Satz des Turns darauf (`BEFUNDE.md`, L283).
- **Der Smoketest ist nicht der Test (DIR-012).** `smoke-device.sh` prüft, ob das
  Gerät **lebt** — nicht, ob es noch tut, was es soll. Ein Endpunkt, der
  `{"ok":true}` meldet und nichts tut, besteht ihn. Vor jedem Release, das **mehr
  als eine Komponente** berührt, läuft der `pwa-tester`. Das stand hier schon am
  03.10.2026 und wurde trotzdem dreimal übersprungen; gefragt hat der Nutzer.
  **Warnzeichen:** „Smoketest 27/0" zu schreiben und „getestet" zu meinen — oder
  eine Frage nach dem Verhalten aus dem Quelltext zu beantworten statt vom Gerät.
  Wird bewusst ausgelassen, gehört das in den Bericht. Einzelheiten im Skill `/release`.
- **Jedes ausgerollte Release wird sofort committet und getaggt (DIR-011).** Nicht
  gesammelt, nicht „auf Nachfrage" — das gehört zum Rollout wie der Smoketest. Das
  Tag heisst `release/<stm>-<esp>-<app>` und trägt die Versionen **dieses** Commits.
  Am 03.10.2026 sind drei Releases in einem Commit gelandet, weil nach jedem Rollout
  nur gemeldet wurde „nicht committet". Nachholbar war das nicht: Die Versionsdateien
  tragen nur den Endstand, und ein Tag mit alter Versionsnummer auf einem neuen Stand
  wäre eine Falschaussage. **`deploy.sh` nennt den fehlenden Tag-Befehl bei jedem
  Lauf** — diese Zeile ist kein Rauschen. Einzelheiten im Skill `/release`.
  **Und der Commit samt Tag gehört zum Remote, im selben Zug.** Das stand bis zum
  04.10.2026 nirgends — nicht hier, nicht im Direktivenkatalog. `BEFUNDE.md` führte
  es als E4 („sechs `release/*`-Tags liegen nur lokal"), also als **Aufgabe** statt
  als Ablaufregel, und eine Aufgabe erinnert niemanden. Ergebnis: 23 Commits und
  11 Tags lagen lokal, **gefragt hat der Nutzer**. Seitdem meldet es der Stop-Hook,
  und zwar auch bei sauberem Arbeitsbaum — genau dort lag die Lücke, denn nach dem
  Commit ist ja nichts mehr geändert.
- **Dokumentation ist lebend oder Momentaufnahme (DIR-006).** Lebend: `CLAUDE.md`,
  `BEFUNDE.md`, `CHANGELOG.md`, alle `README*.md`, `knowledge/**`, `.claude/**`.
  `HARDWARE.md`, `TESTPLAN-PWA.md`. Momentaufnahme mit Datum, wird nicht fortgeschrieben: `REVIEW*.md`,
  `gap-analysis.md`, `specs/**`. **In lebende Dokumente gehören keine
  Versionsnummern** — eine Kopie des Standes veraltet still. Guardrail S9 prüft das.

### Die PWA kommt nicht durch den Rollout aufs Gerät (DIR-017)

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

## R2b — Der Lead committet mit Pfaden, solange Agenten laufen

Kein `git add -A` und kein `git commit -a`, solange ein Agent am Baum arbeitet. Die Pfade
werden einzeln genannt, oder es wird gewartet. Sonst landet halbfertige fremde Arbeit
unter einer Botschaft, die sie nicht erwähnt, und `git blame` zeigt danach auf den
falschen Commit (`BEFUNDE.md`, L158). Am 07./08.10.2026 so gehandhabt, als STM-, ESP- und
PWA-Agenten gleichzeitig liefen; festgeschrieben als B20.

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

### R3b — Dateibesitz ist nicht Reihenfolge

R3 regelt, **wer** eine Datei schreibt. Es regelt nicht, **wann**. Eine Spec, die
Task 3 von Task 2 abhängig macht und einen Gerätetest dazwischenlegt, meint genau
diese Reihenfolge — auch wenn die Dateien disjunkt sind und der Hook beide
durchlässt.

Am 03.10.2026 hat der Lead Task 2 und Task 3 des Beobachtbarkeits-Pakets parallel
gestartet, weil beide verschiedene Dateien berührten. Zwischen ihnen stand in
`tasks.md` eine Flash-Runde, die eine Änderung **isoliert** nachweisen sollte:
„Geht sie daneben, ist die Ursache eindeutig." Diese Zusage war danach nicht mehr
einlösbar. Aufgefallen ist es dem umsetzenden Agenten, der fremde Zeitstempel im
Baum sah — nicht dem Lead, der die Reihenfolge zu verantworten hatte.

**Vor dem Parallelisieren die Abhängigkeitsspalte der Spec lesen, nicht nur die
Besitzspalte.** Zwei Agenten derselben Rolle gleichzeitig sind erlaubt; zwei Tasks
mit einer Abhängigkeit dazwischen nicht.

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
| `/api/ir_code_set` | überschreibt einen angelernten IR-Code. Der einzige Rückweg ist erneutes Anlernen über `learn_ir` — die Zeile darüber |
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

## Behobene Falle im Release-Build

Hier stand bis zum 03.10.2026, `RELEASE_ZIP` nutze Minutengenauigkeit und zwei Builds
in derselben Minute überschrieben sich kommentarlos. **Das stimmt nicht mehr:**
`Makefile:34` nutzt `RELEASE_STAMP := $(shell date +"%Y-%m-%d-%H%M%S")` — sekundengenau
und mit `:=` genau einmal expandiert. Zwei Builds können sich nicht mehr überschreiben.

Die Stelle bleibt als Beispiel stehen, weil sie zweierlei zeigt: Die Warnung nannte
**Zeilennummern** (`18`, `74`), und beide stimmten längst nicht mehr — heute sind es 34
und 109. Und niemandem fiel auf, dass die Falle weg war; gemeldet hat es ein Agent, der
beim Bauen nachgesehen hat. **Eine Warnung, die niemand nachprüft, überlebt ihre
Ursache.** Das ist derselbe Mechanismus wie bei den Versionsnummern in der Doku.

## Hardware

Die Platine ist eine Eigenentwicklung — **WordClock USB-C / STM32F411 V2**, kein
BlackPill-Modul mit Zusatzplatine, aber bewusst pinkompatibel dazu. Pinbelegung,
Signalwege und der Abgleich gegen den Firmware-Code stehen in `HARDWARE.md`, die
Quelle ist das KiCad-Projekt unter `~/Documents/WordClock-USB C - STM32F411 - V2`.

Drei Punkte, die man dem Code allein nicht ansieht:

- **Das EEPROM hängt am I2C** (AT24C32M, zusammen mit der DS3231-RTC). Der STM32F411
  hat keins. Die oft zitierten „16 ms pro Byte" sind nur der Wartezyklus: **Gemessen kostet
  ein Schreibzyklus rund 24 ms** (`BEFUNDE.md`, L358), weil der I2C-Treiber je Byte bis
  1 ms wartet (L359). Seit C3 wird seitenweise geschrieben, ein Zyklus je geänderter Seite.
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
und ohne Webserver zurücklassen) und die langen Busy-Waits ohne `watchdog_reload()`.

Bei Letzterem stand hier bis zum 03.10.2026 „mit weiterhin genau **einer**
Aufrufstelle" — falsch, es sind sechs. **Den Bestand nennt Guardrail S7, nicht diese
Datei**, aus demselben Grund, aus dem hier keine Versionsnummern stehen. Offen ist
nicht die Zahl der Aufrufstellen, sondern dass einzelne lange Pfade keinen haben:
namentlich `var_send_all_variables()` mit rund 190 quittungspflichtigen Kommandos
(`BEFUNDE.md`, L85).

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

2. **Sporadische Hänger auf BlackPill STM32F411 + neues LED-Board** — **seit 04.10.2026
   reproduzierbar** (`BEFUNDE.md`, L204). Zehn schnelle `ticker_set` mit je 32 Zeichen
   legen den Hauptloop für rund 90 Sekunden stillt: zwei Durchläufe in 38 Sekunden statt
   140'000 je Sekunde, Gerätezeit 87 Sekunden im Rückstand, **kein Watchdog-Reset**, und
   der LED-Refresh läuft weiter. Das ist exakt das Bild aus dem Mitschnitt vom 30.09.2026.
   Zweimal ausgelöst, einmal kontrolliert.

   **Die Spur:** Jeder dieser Setter schreibt 32 Byte ins I2C-EEPROM; byteweise waren das
   gemessen 0,77 Sekunden Blockade je Aufruf (G2, L339/L358) — zehnfach überlappend mit der
   Kommandoannahme. **Ein Nutzer löst das aus, ohne etwas Ungewöhnliches zu tun:**
   mehrfaches schnelles Speichern eines Textfelds genügt. Damit gehört es zu A1
   (`watchdog_reload()` in den langen Pfaden) und C3 (EEPROM seitenweise statt byteweise).

   **Was davon unberührt bleibt:** Die frühere Analyse ist damit nicht falsch, sondern
   ergänzt. Der Hauptloop steht, der periodische Zweig fällt aus, die Anzeige-ISR läuft —
   die Frage „was macht den periodischen Zweig unerreichbar" hat jetzt eine Antwort für
   **diesen** Auslöser. Ob die Hänger im Alltagsbetrieb dieselbe Ursache haben, ist damit
   noch nicht gezeigt. Mechanische Kontaktprobleme bleiben für jene Fälle im Rennen.

   *Der folgende Absatz hielt den Stand vor dieser Reproduktion fest:*

   Hier stand bis dahin: nicht gelöst.
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
