# Tasks — Sprachtabellen auslagern, Deutsch fest im Bundle

Einzelschritte mit Abhängigkeiten. **Pro Task genau ein schreibender Agent** (R3, R4).
Nach jedem Task läuft `./tools/guardrails.sh`; erst bei Exit 0 geht die
Schreibberechtigung weiter.

> **R3b — die Abhängigkeitsspalte ist Reihenfolge, nicht nur Besitz.**
> T2 (ESP) und T3 (PWA) berühren disjunkte Dateien, und der Besitz-Hook liesse beide
> gleichzeitig zu. Trotzdem **nicht parallel starten**: T5 (Werkzeuge) hängt am
> Ergebnis von T3, und die Abnahme von T2 ist nur dann eindeutig, wenn T3 steht —
> `install-app.sh` kann den Weisslisteneintrag erst dann wirklich prüfen, wenn es eine
> Datei zum Hochladen gibt. Zwischen T3 und T5 liegt bewusst eine Guardrail-Runde, in
> der S3 **fehlschlagen muss**; dieser Nachweis ist nicht einlösbar, wenn T5 schon
> gelaufen ist.

| # | Task | Agent | Hängt ab von | Guardrails | Review |
|---|---|---|---|---|---|
| 1 | Besitzrecht für die neue Datei eintragen | Lead | — | ☐ | ☐ |
| 2 | Weissliste in `http.cpp` erweitern | `esp-developer` | 1 | ☐ | ☐ |
| 3 | Duplikat entfernen, `en.json` anlegen, `app.js` umbauen | `pwa-developer` | 1, 2 | ☐ | ☐ |
| 4 | Auswahlfeld in `index.html` auf Deutsch reduzieren | `ui-developer` | 3 | ☐ | ☐ |
| 5 | Werkzeuge und Prüfungen: Glob und Duplikatwache | Lead | 3, 4 | ☐ | ☐ |
| 6 | `GZIP_SOURCES` und Versionen | `release-engineer` | 5 | ☐ | ☐ |
| 7 | Bauen, prüfen, ausrollen | Lead | 6 | ☐ | ☐ |
| 8 | ESP flashen, PWA hochladen, am Gerät nachweisen | Lead + Nutzer | 7 | ☐ | ☐ |
| 9 | Vollständiger PWA-Durchlauf | `pwa-tester` | 8 | ☐ | ☐ |
| 10 | Befundkatalog und CHANGELOG nachführen | `doc-writer` | 9 | ☐ | ☐ |

---

## T1 — Besitzrecht für die neue Datei eintragen · **Lead**

**Warum zuerst:** `tools/hooks/file-ownership.py:53` erlaubt dem `pwa-developer`
heute ausschliesslich `^ESP8266/ESP-uclock/data/app/(app|sw)\.js$`. Die neue Datei
`data/app/i18n/en.json` liegt unter `ESP8266/ESP-uclock/` und ist damit `TRACKED`
(`:96-98`), gehört aber niemandem — **T3 würde ohne diesen Schritt blockiert**, und
zwar mit einer Meldung, die auf den Lead verweist. Genau diese Falle hat am
03.10.2026 schon einmal den ersten Auftrag an den `stm-developer` gekostet.

- `OWNERSHIP["pwa-developer"]` um `r"^ESP8266/ESP-uclock/data/app/i18n/.*\.json$"`
  ergänzen
- Kommentar dazu: Sprachdateien sind Daten der PWA-Logik und entstehen aus dem Text,
  den derselbe Agent aus `app.js` entfernt. Zwei Schreiber hiessen Schlüsseldrift.
- `release-engineer` bekommt **keinen** Zugriff darauf — er bumpt Versionen, nicht
  Inhalte (R4)

**Abnahme:** `python3 tools/hooks/file-ownership.py --agent pwa-developer` mit einer
`Write`-Nutzlast auf `ESP8266/ESP-uclock/data/app/i18n/en.json` endet mit Exit 0, mit
`--agent ui-developer` mit Exit 2.

---

## T2 — Weissliste in `http.cpp` erweitern · **`esp-developer`**

**Genau eine Zeile Inhalt, plus Kommentar.** In `APP_INSTALL_ASSETS`
(`http.cpp:215-229`), nach `"app/app.js"`:

```c
    "app/i18n/en.json",
```

Der Kommentar gehört dazu und soll das Folgende festhalten, damit die nächste Sprache
nicht wieder zur Suche wird:

- Der Eintrag ist **Pflicht**, sonst weist `http_api_app_file_upload()` die Datei mit
  `error_code = 1` ab (`:6397`, `:6419`) — die Sprache bliebe stumm, obwohl im Repo
  alles stimmt. Derselbe Mechanismus wie beim `.gz`-Umstieg am 29.04.2026
  (`BEFUNDE.md`, L21)
- Im LittleFS heisst die Datei **`app-i18n-en.json.gz`** — `app_asset_filename()`
  (`:1435`) ersetzt jeden `/` durch `-`. Es entsteht kein Unterordner
- `.json` ist in `http_app_asset_supports_gzip()` (`:1491-1501`) bereits enthalten;
  **dort ist nichts zu ändern**
- Jede weitere Sprache ist genau eine weitere Zeile hier

**Ausdrücklich nicht:** `http_find_app_install_asset()` nicht auf einen
Präfixvergleich umbauen (Begründung in `design.md`, Verworfene Alternativen 6). Keine
Änderung am Auslieferungspfad `http_app()`. Keine Änderung an den Puffergrössen — sie
sind in `design.md` nachgerechnet und tragen.

**Abnahme:** Übersetzt sauber (Guardrail S7b für die Kodierung: `http.cpp` ist
**UTF-8**, nicht ISO-8859-1 — CLAUDE.md nennt diese Datei namentlich). Der Nachweis
am Gerät kommt erst in T8.

---

## T3 — Duplikat entfernen, `en.json` anlegen, `app.js` umbauen · **`pwa-developer`**

Der grösste Schritt. Reihenfolge innerhalb des Tasks:

**a) Vorher messen** (für AK11, gemessen statt geschätzt):
```
wc -c ESP8266/ESP-uclock/data/app/app.js ESP8266/ESP-uclock/data/app/app.js.gz
```
Beide Werte in den Bericht. Erwartet sind heute 577'421 und 129'297 Byte; weicht der
Ausgangspunkt ab, hat jemand anders in der Datei gearbeitet — dann **anhalten und
melden**, nicht weiterarbeiten (R3).

**b) Das Duplikat aus L130 entfernen — vor dem Export, nicht danach.**
`"system.debug_eyebrow"` steht **zweimal je Sprache**:

| Sprache | Zeilen |
|---|---|
| Deutsch | `app.js:95` und `app.js:110` |
| Englisch | `app.js:1026` und `app.js:1041` |

Beide Vorkommen tragen denselben Wert `"Debug"`, in JavaScript ist es deshalb heute
folgenlos. **In JSON ist es das nicht:** Ein Objekt kann einen Schlüssel nicht zweimal
führen — je nach Exportweg bricht er ab oder verschluckt stillschweigend eine
Zuweisung. Deshalb gehört der Schritt **vor** b/c, nicht hinterher.

- In **beiden** Sprachen das **zweite** Vorkommen entfernen (`:110`, `:1041`), damit
  die Reihenfolge der ersten Nennung erhalten bleibt
- Danach gilt je Sprache: **928 Zeilen = 928 verschiedene Schlüssel**
- Dieser Schritt ist die einzige erlaubte Schlüsseländerung ausser den in c)
  genannten. Weitere Duplikate, die dabei auffallen: **melden, nicht entfernen** —
  dann steht eine Entscheidung an, ob die Werte gleich sind

**c) `data/app/i18n/en.json` anlegen** — der Inhalt des bereinigten `en:`-Blocks als
flaches JSON-Objekt. Keine Schlüssel umbenennen, keine Texte ändern, keine Schlüssel
weglassen **ausser** `language.en`. Die Datei ist UTF-8. Validieren mit
`node -e 'JSON.parse(require("fs").readFileSync(process.argv[1],"utf8"))' …/en.json`
und die Schlüsselzahl gegenzählen — **928 erwartet, nicht 929**.
Erwartete Grösse rund **57'568 Byte** (AK12).

**d) `app.js` umbauen**, nach `design.md` Abschnitte 2 bis 5:
- `const I18N` → `const I18N_DE` (nur die deutsche Tabelle), plus `const LANGUAGES`
- `language.de` und `language.en` aus der deutschen Tabelle entfernen (die Optionen
  entstehen ab T4 aus `LANGUAGES`)
- `language.load_failed` und `language.load_missing` in die deutsche Tabelle
  aufnehmen — Wortlaut **unverändert** aus `requirements.md` übernehmen
- `normalizeLanguage()` (`:2210`), `translate()` (`:2222`) auf `LANGUAGES` /
  `loadedTables` umstellen; `translate()` **bleibt synchron**
- `ensureLanguageTable()`, `sanitizeLanguageTable()`, `requestLanguage()` neu
- `setCurrentLanguage()` (`:2276`) → `applyLanguage()`: `localStorage` nur bei Erfolg,
  `document.documentElement.lang` (`:2238`) nur beim geglückten Wechsel
- Auswahlfeld aus `LANGUAGES` aufbauen (`#language-select`, heute `:2266`)
- Start (`:2461`): erst Deutsch anwenden, dann `requestLanguage(gespeichert, false)`
- Änderungslauscher (`:2458`) ruft `requestLanguage(...)` statt `setCurrentLanguage`

**e) Nachher messen**, dieselben zwei Werte. Erwartet rund 517'987 Quelle. Die `.gz`
entsteht erst in T7 — sie wird hier **nicht** erzeugt, `make` ist für Teammates tabu
(R1).

**Ausdrücklich nicht:** `LOCAL_APP_REQUIRED_ASSETS` (`:1879`) bleibt unverändert.
`sw.js` bleibt in diesem Task unverändert — die Begründung, warum die Sprachdatei
**nicht** in die `ASSETS`-Liste gehört, steht in `design.md` Abschnitt 7; sie ist eine
Entwurfsentscheidung, keine Vergesslichkeit. Keine der ~176 hartcodierten Strings aus
B8 mitkorrigieren; Fundstellen in den Bericht.

**Abnahme (dieser Task darf S3 brechen!):**
- `node --check` auf `app.js` sauber (S1)
- S2 meldet keine undefinierten Funktionen
- **S3 schlägt jetzt fehl** mit „i18n-Bloecke 'de:' oder 'en:' nicht gefunden" —
  das ist der erwartete Zustand bis T5 und gehört so in den Bericht. Es ist **kein**
  Freibrief, weiterzulaufen: T4 und T5 folgen, bevor irgendetwas gebaut wird
- Im Bericht: Schlüsselzahl in `en.json` (erwartet 928), die vier entfernten
  Duplikatzeilen, Vorher/Nachher-Bytes gegen die Erwartung aus `requirements.md`,
  und eine Aussage dazu, ob die Schlüsselmenge der deutschen Tabelle entspricht

---

## T4 — Auswahlfeld in `index.html` reduzieren · **`ui-developer`**

In `index.html:47-50` bleibt nur die deutsche Option stehen:

```html
<select id="language-select" aria-label="Sprache" data-i18n-aria-label="hero.language_label">
  <option value="de">Deutsch</option>
</select>
```

- Die `data-i18n`-Attribute an den Optionen entfallen — die Eigennamen kommen aus
  `LANGUAGES`, damit eine neue Sprache nicht in zwei Dateien eingetragen werden muss
- Deutsch bleibt **im Markup** stehen: Es ist die Sprache, die ohne jeden Abruf gilt,
  und das Feld soll auch dann etwas Sinnvolles zeigen, wenn `app.js` sehr früh
  scheitert
- `data-i18n-aria-label="hero.language_label"` bleibt

**Ausdrücklich nicht:** keine weiteren Änderungen an `index.html`, insbesondere nicht
an den Kacheln des Moduls `maintenance` — das ist B13 und hat eine eigene Spec.

**Abnahme:** S6 (CSS-Klassen) unverändert, keine neuen Warnungen.

---

## T5 — Werkzeuge und Prüfungen: Glob und Duplikatwache · **Lead**

Vier Dateien, alle unter `tools/**`. **Keine davon darf eine Sprache beim Namen
nennen** (AK20) — das ist der Teil des Auftrags, der die Erweiterbarkeit trägt.

### `tools/checks/i18n-keys.mjs` — zwei Änderungen, nicht eine

**(1) Über Dateigrenzen lesen:**
- Deutsch weiterhin aus `app.js` (`de:`-Block bzw. `I18N_DE`)
- Jede Datei aus `ESP8266/ESP-uclock/data/app/i18n/*.json` als eigene Sprache lesen
- Je Sprache melden: fehlende und überzählige Schlüssel gegenüber Deutsch
- Verwendete Schlüssel (`translate(…)`, `data-i18n…`) weiterhin gegen die
  **Vereinigung** prüfen; ein nirgends definierter Schlüssel bleibt **KRITISCH**
- Findet es die deutsche Tabelle nicht, bleibt es bei Exit 1 — diese Wache darf nicht
  stillschweigend durchwinken, wenn sich die Struktur erneut ändert (L123)

**(2) Zeilenzahl gegen Schlüsselzahl — die Lücke aus L130 schliessen.**
`collect()` (`:16-20`) sammelt in ein `Set`; doppelte Schlüssel fallen dabei
lautlos zusammen. Verglichen werden heute nur **de gegen en**, und ein Duplikat steht
in beiden — beide Mengen bleiben gleich gross, die Stufe schweigt. Genau so ist
`system.debug_eyebrow` unbemerkt doppelt geblieben. Trügen die zwei Vorkommen
verschiedene Werte, zeigte die Oberfläche still den zweiten.

- Je Sprache die **Treffer** zählen, nicht nur die Set-Grösse, und bei Differenz die
  **Namen** der mehrfach belegten Schlüssel melden
- Einstufung: **HOCH** genügt, nicht KRITISCH — ein Duplikat mit gleichem Wert ist
  kein Fehlverhalten, nur ein Risiko. Ein Duplikat mit **verschiedenen** Werten ist
  KRITISCH, und das kann die Stufe unterscheiden, weil sie beide Werte sieht
- Die Zahl gehört in die Bestandszeile, die die Stufe ohnehin ausgibt
  (`Keys: de=… en=… verwendet=…` → zusätzlich die Zeilenzahl)

### Die drei anderen Dateien

**`tools/guardrails.sh:142`** — `GZ_SOURCES` um `$APP/i18n/*.json` erweitern, mit
Glob-Schutz (kein Treffer darf nicht als Literal durchgereicht werden).

**`tools/install-app.sh:38`** — `ASSETS` um die Treffer von `$D/i18n/*.json`
erweitern, als `app/i18n/<name>`. `check_stored()` (`:69`) und die Quellableitung
(`:95`) brauchen **keine** Änderung; beide können Unterordner bereits, weil
`app/icons/**` sie nutzt. Im Kommentar festhalten, dass die Reihenfolge weiterhin zu
`APP_INSTALL_ASSETS` passen muss.

**`tools/deploy.sh:59, 91, 93`** — `GZ_SRC` um die `i18n`-Dateien erweitern **und**
`mkdir -p "$STAGE/app/i18n"` ergänzen. Ohne den Ordner auf dem Update-Server findet
die OTA-Nachinstallation `app/i18n/en.json.gz` nicht; das wäre genau der stille
Fehlschlag, den `download_file_as_flattened_app_asset()` nur mit `reason=download`
quittiert.

### Abnahme

- `./tools/guardrails.sh` läuft wieder mit Exit 0, S3 meldet `de` und `en` mit
  Differenz 0 und je **928 Zeilen = 928 Schlüssel**
- **Gegenprobe 1 (AK14):** einen Schlüssel aus `en.json` entfernen, S3 muss ihn
  melden; danach zurücksetzen. Ohne diese Probe ist nicht belegt, dass die Stufe noch
  prüft
- **Gegenprobe 2 (AK15):** eine Zeile in der deutschen Tabelle verdoppeln, S3 muss sie
  **namentlich** melden; danach zurücksetzen. Eine Prüfung, die den Befund nicht
  findet, der sie ausgelöst hat, ist keine
- `grep -rn 'en\.json' Makefile tools/` ist leer

---

## T6 — `GZIP_SOURCES` und Versionen · **`release-engineer`**

- `Makefile:22`: `GZIP_SOURCES` um `$(wildcard $(APP_DIR)/i18n/*.json)` erweitern.
  `app-gz` und `clean-app-gz` (`:93`, `:100`) brauchen dadurch keine eigene Änderung —
  beide laufen über dieselbe Liste
- `ESP8266/ESP-uclock/version.h`: `ESP_VERSION` anheben (`http.cpp` geändert)
- `app.js`: `APP_VERSION` anheben
- `sw.js`: `CACHE_NAME` anheben — **zwingend zusammen mit `APP_VERSION`** (R4)
- `src/main.h`: **nicht anfassen**, der STM ist nicht berührt (DIR-004 verlangt kein
  Gleichschritt)

**Abnahme:** S4 nennt alle vier Versionszeilen und bestätigt die Versionspflicht.

---

## T7 — Bauen, prüfen, ausrollen · **Lead**

Serieller Build, nur der Lead (R1). Arbeitsverzeichnis muss still sein (R2) —
`git status` vor `app-gz`.

```
make app-gz
make release-zip
./tools/guardrails.sh --full
./tools/deploy.sh
```

**Nach `app-gz` die Messung abschliessen (AK11/AK12):** `wc -c` auf `app.js.gz` und
`i18n/en.json.gz`. Erwartet rund 115'843 und 13'771 Byte.

**Nach `deploy.sh` prüfen**, dass `app/i18n/en.json.gz` im Zielverzeichnis liegt und
nicht 0 Byte ist (AK10). `deploy.sh` ruft `check-update-source.sh` selbst auf —
dessen Ausgabe nicht überlesen (L124).

Nicht zwei Release-Builds in derselben Minute — `RELEASE_ZIP` ist inzwischen
sekundengenau, der alte Fallstrick aus L2 ist entschärft, die Regel bleibt billig.

---

## T8 — ESP flashen, PWA hochladen, am Gerät nachweisen · **Lead + Nutzer**

**Die Reihenfolge ist nicht vertauschbar.** Ohne den Weisslisteneintrag aus T2 weist
der ESP den Upload der Sprachdatei ab.

Der Nutzer spielt Updates selbst ein. Die Zeilen stehen hier fertig; sie werden ihm
so übergeben, nicht als Aufforderung zum Selbersuchen:

```
./tools/flash-stm.sh --check        # nur zur Kontrolle der Update-Quelle
# ESP-Firmware einspielen (Nutzer)
./tools/install-app.sh
./tools/install-app.sh --check
./tools/smoke-device.sh
./tools/check-pwa.sh
```

**Nachweise am Gerät, alle lesend (DIR-008):**

| Nachweis | Befehl / Handgriff | Erwartet |
|---|---|---|
| AK9 | `install-app.sh` | für `app/i18n/en.json` kommt `{"ok":true}` |
| AK8 | `install-app.sh --check` | nennt `app-i18n-en.json.gz`, meldet vollständig |
| AK7 | `curl -s -D- -o/dev/null "http://$DEVICE_HOST/app/i18n/en.json"` | `200`, `Content-Encoding: gzip`, `Content-Type: application/json` |
| AK2 | PWA öffnen, auf Englisch umschalten | genau ein Abruf von `/app/i18n/en.json?v=…`, Oberfläche englisch; zweites Umschalten ohne weiteren Abruf |
| AK1 | Abruf in den DevTools blockieren, neu laden | vollständig bedienbar auf Deutsch, kein roher Schlüssel sichtbar |
| AK3/AK4/AK5 | bei blockiertem Abruf auf Englisch umschalten | Meldung erscheint, Auswahlfeld springt auf Deutsch zurück, `document.documentElement.lang === "de"`, `localStorage` unverändert |

**AK19 — Trockenübung „vierte Sprache"**, hier oder nach T5, aber vor dem Abschluss:
`it.json` mit fünf Schlüsseln anlegen, eine Zeile in `LANGUAGES`, dann `make app-gz`,
`install-app.sh --check` und `guardrails.sh` beobachten. **Danach restlos entfernen.**
Diese Probe ist der einzige Beleg dafür, dass die Erweiterbarkeit wirklich da ist —
ohne sie ist sie behauptet.

**Nicht senden:** `GET /?a` (Parameter ohne `=`), `/api/test_display`,
`/api/learn_ir`, irgendetwas Schreibendes. Der Sprachwechsel schreibt ausschliesslich
`localStorage` im Browser, nichts auf dem Gerät.

**Direkt nach dem Rollout committen und taggen (DIR-011)** — nicht gesammelt, nicht
auf Nachfrage. Tag `release/<stm>-<esp>-<app>` mit den Versionen **dieses** Commits.

---

## T9 — Vollständiger PWA-Durchlauf · **`pwa-tester`**

Pflicht nach DIR-012: Dieses Release berührt ESP **und** PWA **und** Build. Phasen 0
bis 4 und 9 aus `TESTPLAN-PWA.md`. Backup-Import, Verbindungstrennung und die
gefährlichen Funktionen bleiben beim Nutzer.

**Besonders zu prüfen, weil es hier neu ist:** Sprachwechsel in jedem Modul; dass nach
einem Wechsel keine Stelle einen rohen Schlüssel zeigt; dass ein Neuladen die zuletzt
erfolgreich gesetzte Sprache wiederherstellt; dass die Debug-Überschrift aus L130
(`system.debug_eyebrow`) in beiden Sprachen weiterhin „Debug" zeigt.

Ein Auslassen gehört in den Bericht — „Smoketest 27/0" ist nicht „getestet".

---

## T10 — Befundkatalog und CHANGELOG nachführen · **`doc-writer`**

- `BEFUNDE.md`: L127 auf **erledigt** mit Messwerten (Vorher/Nachher aus T3 und T7),
  B15 aus der ToDo-Liste nehmen
- **L130 auf erledigt** — Duplikat entfernt **und** die Prüflücke geschlossen. Beides
  gehört hinein; nur das Duplikat zu entfernen wäre die Symptombehandlung gewesen
- L126 wird dadurch **nicht** erledigt — es beschreibt das Wachstum, nicht nur die
  Tabellen; der erreichte Anteil (10,3 % Quelle, 10,4 % gepackt) gehört aber hinein
- **L125 / C9 bleibt unverändert offen.** Ausdrücklich festhalten, dass diese
  Massnahme den Absturz nicht behoben hat und der Grössenfaktor 1,12 beträgt — damit
  niemand die Zeile später als Entwarnung liest
- `CHANGELOG.md`: Eintrag mit den Versionen dieses Releases
- `ESP8266/ESP-uclock/APP-BUNDLE.md` prüfen: Dort steht die Liste der App-Assets —
  wenn sie eine Aufzählung führt, gehört `app/i18n/*.json` hinein
- Keine Versionsnummern in lebende Dokumente schreiben (DIR-006, Guardrail S9).
  `CLAUDE.md` bekommt **keine** Kopie der Asset-Liste

---

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings
   (**Ausnahme T3**: S3 schlägt dort erwartungsgemäss fehl, siehe oben)
3. Review durch den zuständigen Review-Agenten, ausdrücklich gegen
   `knowledge/architecture-checklist.md`
4. Erst dann nächster Task oder Schreibübergabe

## Abschluss

- [ ] Alle Tasks erledigt
- [ ] `./tools/guardrails.sh --full` mit Exit 0
- [ ] Alle 20 Akzeptanzkriterien aus `requirements.md` erfüllt, jedes mit Nachweis
- [ ] Die drei Messwerte aus AK11/AK12 liegen vor und stehen im Bericht
- [ ] Beide Gegenproben aus T5 gelaufen und zurückgesetzt
- [ ] Release-ZIP gebaut, Flash-Umfang benannt: **ESP ja, STM nein, PWA-Upload ja**
- [ ] Commit und Tag `release/<stm>-<esp>-<app>` gesetzt (DIR-011)
- [ ] `it.json` aus der Trockenübung entfernt — im Repo steht keine vierte Sprache
