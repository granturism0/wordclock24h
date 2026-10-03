# Design — Sprachtabellen auslagern, Deutsch fest im Bundle

## Was am Gerätequelltext nachgeprüft wurde

Drei Punkte des Auftrags waren Fragen an `http.cpp`, keine Annahmen. Sie sind am
Quelltext beantwortet, bevor dieser Entwurf entstand:

**(a) Darf eine Datei in einem Unterordner liegen, obwohl der Namensraum flach ist?**
**Ja — und das ist belegt, nicht gehofft.** `app_asset_filename()`
(`http.cpp:1435-1458`) ersetzt **jeden** `/` im Pfad durch `-`:

```c
for (const char * p = asset_path; *p && idx < maxlen - 1; p++)
{
    if (*p == '/') { filename[idx++] = '-'; }
    else           { filename[idx++] = *p; }
}
```

Im LittleFS entsteht dadurch **kein** Unterordner, sondern ein abgeflachter Name.
Das ist kein neuer Weg: `app/icons/icon-192.png` steht seit jeher in der Weissliste
und liegt auf dem Gerät als `app-icons-icon-192.png.gz`. Für uns gilt damit:

| Repo | Weissliste `APP_INSTALL_ASSETS` | URL | Name im LittleFS |
|---|---|---|---|
| `data/app/i18n/en.json` | `app/i18n/en.json` | `/app/i18n/en.json` | `app-i18n-en.json.gz` |

`http_app()` (`http.cpp:1614-1626`) baut aus jedem Request unter `/app/` den
Asset-Pfad `app/<rest>` und reicht ihn an `http_find_stored_app_asset_filename()`
(`:1531`) — die **ausschliesslich** nach `.gz` sucht, zuerst abgeflacht, hilfsweise
unter dem blossen Basisnamen. Ein Plain-Fallback existiert nicht; der Pfad wäre also
auch dann regelkonform, wenn jemand ihn aufweichen wollte.

**Namenskollision ist dadurch möglich und wird hier vermieden:** `app/i18n/en.json`
und ein hypothetisches `app/en.json` flachen beide **nicht** auf denselben Namen ab
(`app-i18n-en.json` vs. `app-en.json`), aber der zweite Suchpfad in
`http_find_stored_app_asset_filename()` greift auf den blossen Basisnamen `en.json.gz`
zurück. Solange es nur **eine** Datei je Basisnamen gibt, ist das harmlos. Regel für
künftige Sprachen: **Der Basisname einer Sprachdatei ist der Sprachcode und kommt
sonst nirgends im Asset-Baum vor.**

**(b) Was tut die Weissliste eigentlich?** Sie ist **nicht** der Türsteher beim
Ausliefern — `http_app()` fragt sie gar nicht. Sie entscheidet an drei anderen
Stellen, und jede davon trifft uns:

| Stelle | Funktion | Folge ohne Eintrag |
|---|---|---|
| Upload vom Rechner | `http_api_app_file_upload()` → `http_find_app_install_asset()` (`:6397`, `:1461`) | `error_code = 1`, die Datei kommt **nie** aufs Gerät — `install-app.sh` schlägt für genau diese Datei fehl |
| OTA-Nachinstallation | `http_try_auto_install_app_files()` (`:1324`) | Die Datei wird beim Selbstinstallieren der App nicht mitgeholt |
| Vollständigkeitsprüfung | `http_app_installation_complete()` (`:1228`) | Das Gerät hält die Installation für vollständig, obwohl die Sprache fehlt |

Daraus folgt die harte Rollout-Reihenfolge aus `requirements.md`: **ESP zuerst
flashen, dann die PWA hochladen.** Genau dieser Mechanismus hat am 29.04.2026 die PWA
„verschwinden" lassen (CLAUDE.md, `BEFUNDE.md` L21) — die Dateien lagen da, nur unter
Namen, die niemand mehr suchte.

**(c) Muss der ESP für `.json` angepasst werden?** **Nein.**
`http_app_asset_supports_gzip()` (`:1482-1502`) listet `.json` bereits auf, und
`app/layout-previews.json` beweist den Weg vom Server bis in den Browser. Die einzige
nötige ESP-Änderung ist der Weisslisteneintrag.

**Puffergrössen gegengeprüft** (weil ein stilles Abschneiden hier teuer wäre):
`app-i18n-en.json.gz` ist 19 Zeichen. Die Puffer sind `flat_gz[72]` (`:1236`),
`local_filename[64]` (`:6401`) und `http_app_asset_local_filename[128]` (`:257`). Der
längste bestehende Eintrag ist `app-manifest.webmanifest.gz` mit 27 Zeichen. Luft
genug; Sprachcodes nach ISO 639-1 sind zweistellig.

## Was die Auslagerung messbar bringt — und was nicht

Am 03.10.2026 nachgemessen (Tabelle in `requirements.md`, Abschnitt „Grösse"):
Quelle **−59'434 Byte (10,3 %)**, gepackt **−13'454 Byte (10,4 %)**, Sendeframes à
256 Byte **506 → 453**.

Der Faktor ist **1,12**. Das ist der ehrliche Rahmen für jede Aussage zu L125: Eine
Datei, die 53 Frames weniger braucht, trifft weniger Gelegenheiten für den Stillstand
in der Sendeschleife — die Ursache bleibt unberührt. Die Stillstandsstellen aus L125
lagen bei 25'344 · 76'800 · 83'712 Byte und damit alle **unterhalb** der neuen Grösse.
Wer aus dieser Änderung „der Absturz ist weg" liest, liest etwas hinein (DIR-003).

Der eigentliche Gewinn ist der, den der Nutzer wollte: **Die vierte Sprache kostet am
Startgewicht null.** Heute kostet jede rund 60 kB — bei jedem Abruf, für jeden Nutzer.

## Lösungsweg

### 1. Datenablage

```
ESP8266/ESP-uclock/data/app/
├── app.js            ← enthält nur noch die deutsche Tabelle
└── i18n/
    └── en.json       ← flaches Objekt, 1 Ebene: { "app.title": "WordClock", … }
```

**JSON, nicht JavaScript.** Die Datei kommt über HTTP vom Gerät, und die
Architektur-Checkliste behandelt Inhalte von dort als nicht vertrauenswürdig. Ein
`<script>`- oder `import`-Weg wäre Codeausführung aus einer unvertrauenswürdigen
Quelle; `response.json()` ist Datenübernahme. Nebenbei spart JSON die
`const`-Hülle und passt in die bestehende `.gz`-Mechanik ohne jede Sonderregel.

**Flach, keine Verschachtelung.** Die Schlüssel heissen heute schon
`maintenance.target_expected`; ein verschachteltes Objekt würde die rund 900
Aufrufstellen zum Auflösen zwingen. Der Punkt bleibt Teil des Schlüssels, nicht
Pfadtrenner.

**JSON erzwingt eindeutige Schlüssel — und deckt damit L130 auf.**
`"system.debug_eyebrow"` steht heute zweimal je Sprache (`app.js:95`/`:110` und
`:1026`/`:1041`). In JavaScript gewinnt stillschweigend das zweite Vorkommen; in JSON
ist das Objekt entweder ungültig oder verliert beim Parsen eine Zuweisung. Das
Duplikat wird deshalb **vor** dem Export entfernt, in beiden Sprachen (T3). Danach
gilt für beide Seiten **928 Zeilen = 928 Schlüssel**.

### 2. Sprachliste in `app.js`

Ersetzt `const I18N` als Einstiegspunkt. Deutsch trägt seine Tabelle direkt, jede
weitere Sprache nur ihren Eigennamen:

```js
const DEFAULT_LANGUAGE = "de";
const I18N_DE = { /* unverändert die heutige de-Tabelle, ohne das Duplikat */ };
// Eine Zeile je Sprache. Der Dateiname folgt dem Code: /app/i18n/<code>.json
// Der Eigenname steht bewusst in der Sprache selbst -- wer Englisch sucht, findet
// "English", auch wenn die Oberflaeche gerade deutsch ist.
const LANGUAGES = [
  { code: "de", label: "Deutsch", table: I18N_DE },
  { code: "en", label: "English" }
];
```

Eine vierte Sprache ist danach `{ code: "it", label: "Italiano" }` plus `it.json`
plus eine Zeile in `APP_INSTALL_ASSETS`. **Keine Logikänderung, keine Änderung an
`install-app.sh`, `deploy.sh`, `Makefile` oder den Guardrails** — diese vier greifen
nach dem Umbau über `i18n/*.json` (AK20).

### 3. Laufzeitzustand und Auflösung

```js
let currentLanguage = DEFAULT_LANGUAGE;
const loadedTables = { de: I18N_DE };   // einmal geladen, bleibt im Speicher
let pendingLanguage = null;             // genau ein Ladevorgang gleichzeitig
```

`translate()` **bleibt synchron** und behält seine Fallback-Kette:

```js
function translate(key) {
  const active = loadedTables[currentLanguage] || I18N_DE;
  return active[key] || I18N_DE[key] || key;
}
```

Das ist der Kern der Zusicherung aus AK1 und AK6: Fehlt die Tabelle **oder** fehlt ein
einzelner Schlüssel darin, steht deutscher Text da — nie ein roher Schlüssel.

`normalizeLanguage()` (heute `app.js:2210`) prüft künftig gegen `LANGUAGES` statt
gegen `I18N`:

```js
function normalizeLanguage(language) {
  return LANGUAGES.some((entry) => entry.code === language) ? language : DEFAULT_LANGUAGE;
}
```

### 4. Ladepfad

```js
async function ensureLanguageTable(code) {
  if (loadedTables[code]) { return loadedTables[code]; }

  const url = "/app/i18n/" + code + ".json?v=" + APP_VERSION;
  const response = await fetchWithTimeout(url, { cache: "default" }, 4000);

  if (response.status === 404) { throw { kind: "missing" }; }
  if (!response.ok)            { throw { kind: "failed", status: response.status }; }

  const payload = await response.json();
  loadedTables[code] = sanitizeLanguageTable(payload);   // wirft bei Unsinn
  return loadedTables[code];
}
```

**`?v=<APP_VERSION>` als Abgleich gegen alte Stände.** Jede App-Version fragt eine
eigene URL ab; eine im Service Worker liegende Tabelle einer älteren Version wird
dadurch nicht stillschweigend weiterbenutzt. Dass der ESP damit umgeht, ist belegt:
`http_app()` wertet Parameter getrennt vom Pfad aus — `/app/?action=install`
(`:1650`) ist derselbe Mechanismus. **`v=` trägt immer ein Gleichheitszeichen** —
ein Parameter ohne `=` lässt den ESP abstürzen (`GET /?a`, CLAUDE.md R5).

`sanitizeLanguageTable()` ist die Grenze zwischen Fremddaten und Anwendung:

```js
function sanitizeLanguageTable(payload) {
  if (!payload || typeof payload !== "object" || Array.isArray(payload)) {
    throw { kind: "failed", reason: "shape" };
  }
  const table = Object.create(null);   // kein Prototyp -> kein __proto__-Weg
  Object.keys(payload).forEach((key) => {
    if (typeof payload[key] === "string") { table[key] = payload[key]; }
  });
  if (!Object.keys(table).length) { throw { kind: "failed", reason: "empty" }; }
  return table;
}
```

### 5. Umschalten

```js
async function requestLanguage(code, persist) {
  const target = normalizeLanguage(code);
  const previous = currentLanguage;

  if (target === previous || loadedTables[target]) {
    applyLanguage(target, persist);
    return;
  }

  pendingLanguage = target;
  setLanguageSelectBusy(true);
  try {
    await ensureLanguageTable(target);
    if (pendingLanguage !== target) { return; }   // inzwischen weitergeklickt
    applyLanguage(target, persist);
  } catch (error) {
    if (pendingLanguage !== target) { return; }
    applyLanguage(previous, false);               // sichtbar zurueck, nicht stumm
    showStatusBanner(translateFormat(
      error && error.kind === "missing" ? "language.load_missing" : "language.load_failed",
      { language: languageLabel(target) }), "error");
  } finally {
    if (pendingLanguage === target) { pendingLanguage = null; }
    setLanguageSelectBusy(false);
  }
}
```

`applyLanguage(code, persist)` ist das heutige `setCurrentLanguage()`
(`app.js:2276`) mit zwei Schärfungen:

- `window.localStorage.setItem(...)` **nur** wenn die Tabelle wirklich steht (AK5)
- `document.documentElement.lang = currentLanguage` (heute `app.js:2238`) wird damit
  automatisch richtig: Es läuft erst, nachdem der Wechsel geglückt ist (AK4)

**Start (heute `app.js:2461`, `setCurrentLanguage(getStoredLanguage(), false)`):**
Die Oberfläche startet **immer** auf Deutsch und ruft danach `requestLanguage(stored,
false)` auf. Es gibt also keinen Moment, in dem die App auf eine Datei wartet. Geht
der Abruf daneben, erscheint dieselbe Meldung wie beim Umschalten; der gespeicherte
Wunsch bleibt stehen und wird beim nächsten Start erneut versucht.

### 6. Auswahlfeld

Die Optionen werden aus `LANGUAGES` erzeugt, damit eine neue Sprache nicht in zwei
Dateien eingetragen werden muss. `index.html` behält nur Deutsch als statischen
Inhalt — das ist die Sprache, die ohne jeden Abruf gilt:

```html
<select id="language-select" aria-label="Sprache" data-i18n-aria-label="hero.language_label">
  <option value="de">Deutsch</option>
</select>
```

`app.js` baut die Liste beim Start neu auf (Eigenname aus `LANGUAGES`, nicht über
`translate()`). Die Schlüssel `language.de` und `language.en` verlieren damit ihre
einzige Verwendung und werden aus beiden Tabellen entfernt; bliebe irgendwo ein
Verweis, meldete S3 ihn als **KRITISCH** — die Prüfung deckt diesen Schritt ab.

### 7. Service Worker

**Die Sprachdateien kommen nicht in die `ASSETS`-Liste von `sw.js`.** Zwei Gründe,
beide hart:

- `cache.addAll()` ist atomar (`sw.js:29`). Ein einziges 404 — etwa weil der ESP noch
  die alte Weissliste trägt — liesse die **gesamte** Service-Worker-Installation
  scheitern. Damit verlöre ein gescheitertes Sprach-Asset die Offline-Fähigkeit der
  ganzen App. Das steht in keinem Verhältnis.
- Vorabladen widerspricht dem Zweck: Die Datei soll **nicht** bei jedem Nutzer landen.

Nötig ist es auch nicht: `staleWhileRevalidate()` (`sw.js:84`) fängt **jeden** Pfad
unter `/app/` ab und legt ihn nach dem ersten erfolgreichen Abruf in den Cache. Wer
einmal auf Englisch umgeschaltet hat, hat Englisch danach auch offline. Alte Einträge
verschwinden beim Wechsel von `CACHE_NAME` (`sw.js:34`) — deshalb bleibt die
Kopplung `APP_VERSION` ↔ `CACHE_NAME` aus R4 unverändert Pflicht.

### 8. Werkzeuge — alle über `i18n/*.json`, keine Sprache beim Namen

| Datei | Heute | Nachher |
|---|---|---|
| `Makefile:22` `GZIP_SOURCES` | feste Liste | `+ $(wildcard $(APP_DIR)/i18n/*.json)` |
| `tools/install-app.sh:38` `ASSETS` | feste Liste | `+ for f in "$D"/i18n/*.json; do ASSETS="$ASSETS app/i18n/$(basename "$f")"; done` (Glob-Treffer prüfen, damit das Muster nicht als Literal durchrutscht) |
| `tools/deploy.sh:59` `GZ_SRC` | feste Liste | zusätzlich eine Schleife über `$APP/i18n/*.json.gz`; **`mkdir -p "$STAGE/app/i18n"`** vor dem Kopieren (`deploy.sh:91`) — sonst fehlt der Ordner auf dem Server und die OTA-Nachinstallation findet `app/i18n/en.json.gz` nicht |
| `tools/guardrails.sh:142` S5 | feste Liste | `+ $APP/i18n/*.json` |
| `tools/checks/i18n-keys.mjs` | liest `de:` und `en:` aus `app.js`, vergleicht **Mengen** | liest `de` aus `app.js` und **jede** `data/app/i18n/*.json`; zusätzlich **Zeilenzahl gegen Schlüsselzahl** je Sprache (L130) |
| `tools/hooks/file-ownership.py:53` | `(app\|sw)\.js$` | `+ ^ESP8266/ESP-uclock/data/app/i18n/.*\.json$` für `pwa-developer` |

**`install-app.sh` braucht sonst nichts:** Die Abflachung in `check_stored()`
(`install-app.sh:69`) macht bereits `tr '/' '-'`, und die Quellableitung
`src="$D/${a#app/}.gz"` (`:95`) ergibt für `app/i18n/en.json` korrekt
`data/app/i18n/en.json.gz`. Beides für Unterordner gebaut, weil `app/icons/**` sie
schon nutzt.

**`i18n-keys.mjs` ist der kritische Punkt unter den Werkzeugen**, und zwar aus zwei
Richtungen.

*Erstens* endet es heute mit Exit 1, wenn es die `en:`-Tabelle nicht findet
(`i18n-keys.mjs:12-15`) — der Umbau bricht die Stufe also **sicher**, wenn sie nicht
mitgezogen wird. Das ist gewollt: lieber ein lauter Fehlschlag als eine Prüfung, die
nach dem Umbau stillschweigend nichts mehr prüft. Genau diese Verwechslung war L123.
Deshalb AK14: eine Gegenprobe mit absichtlich gelöschtem Schlüssel.

*Zweitens* hat die Stufe eine Lücke, die erst durch diese Arbeit auffiel und als
**L130** im Katalog steht: Sie sammelt die Schlüssel in ein `Set` (`:16-20`) und
vergleicht nur de gegen en. Ein doppelter Schlüssel steht in **beiden** Tabellen, beide
Mengen bleiben gleich gross, die Stufe schweigt. Trügen die zwei Vorkommen
verschiedene Werte, zeigte die Oberfläche still den zweiten — der Fehler wäre genau
der stille, den dieses Projekt sonst überall einfängt. Die zweite Kennzahl **Zeilen
gegen Schlüssel** schliesst das, und AK15 verlangt dafür ebenfalls eine Gegenprobe:
eine Zeile verdoppeln, die Meldung muss kommen.

## Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `tools/hooks/file-ownership.py` | `pwa-developer` darf `data/app/i18n/*.json` schreiben | Lead |
| `ESP8266/ESP-uclock/http.cpp` | `app/i18n/en.json` in `APP_INSTALL_ASSETS` (`:215-229`), mit Kommentar warum | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/i18n/en.json` | **neu** — die 928 englischen Schlüssel | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | `en`-Tabelle raus, Duplikat aus L130 raus, `LANGUAGES`, Ladepfad, Fallback, zwei neue deutsche Schlüssel | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/index.html` | nur noch die deutsche Option im Auswahlfeld (`:47-50`) | `ui-developer` |
| `tools/install-app.sh`, `tools/deploy.sh`, `tools/guardrails.sh`, `tools/checks/i18n-keys.mjs` | Glob statt fester Liste, Duplikatprüfung | Lead |
| `Makefile` | `GZIP_SOURCES` um `$(wildcard …/i18n/*.json)` | `release-engineer` |
| `ESP8266/ESP-uclock/version.h`, `app.js` (`APP_VERSION`), `sw.js` (`CACHE_NAME`) | Versionen anheben | `release-engineer` |

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Werden die
Wetter-Endpunkte eingehalten? Bleibt die Restore-Bedingung vollständig? Greift die
Änderung an der richtigen Schicht an?

> **Antwort:** Die Legacy-Oberfläche wird nicht berührt; sie hat eigene Texte im
> ESP-Quelltext und bleibt die Stabilitäts-Referenz. Die Wetter-Endpunkte
> (`/api/weather_get_now`, `/api/weather_get_forecast`) kommen in dieser Änderung
> nicht vor, der entfernte Legacy-Bypass wird nicht angefasst. Die
> `pending_weather_ticker_restore`-Bedingung in `src/main.c` wird nicht berührt — am
> STM ändert sich nichts.
> **`.gz`-only wird eingehalten, und zwar strenger als nötig:** `en.json` existiert auf
> dem Gerät ausschliesslich als `app-i18n-en.json.gz`; der Suchpfad
> `http_find_stored_app_asset_filename()` kennt gar keinen Plain-Zweig, und die
> Existenzprüfung `http_fs_file_exists_and_nonempty()` (`:1206`) prüft wie gefordert
> zusätzlich auf Grösse > 0. Eine leere `.gz` würde hier zu einem Parse-Fehler führen
> — der landet im `catch` und damit im Rückfall auf Deutsch, nicht im Weissbild.
> **Richtige Schicht — und hier liegt die wichtigste Abgrenzung:** Das Wachstum von
> `app.js` ist ein Strukturproblem der PWA und wird in der PWA behoben. Der
> ESP-Absturz beim Ausliefern (L125) ist ein ESP-Problem und wird **nicht** hier
> behoben. Die Datei wird um Faktor **1,12** kleiner (506 → 453 Frames, gemessen) —
> das ist eine **Erwartung auf weniger Gelegenheiten, kein Nachweis** (DIR-003). C9
> bleibt offen und muss am ESP erledigt werden.

**Scalable systems** — Wie viele STM-Kommandos erzeugt die Aktion? Wie viele Byte pro
Minute zusätzlich auf der UART? Bleibt jeder Pfad unter 20 s Watchdog? Wie lange
blockiert der Hauptloop?

> **Antwort:** **Null STM-Kommandos.** Die Sprachdatei wird vom ESP aus dem LittleFS
> ausgeliefert; der STM ist an diesem Pfad nicht beteiligt, es wird keine Variable
> gelesen oder geschrieben.
> **UART:** Ein zusätzlicher HTTP-Request erzeugt die bekannte HTTP-Debugzeile auf der
> STM-UART — am Gerät gemessen rund **106 Byte je Request** (C2). Er fällt **einmal
> je Sprachwechsel** an, nicht periodisch, und für den deutschsprachigen Nutzer
> **nie**. Zum Vergleich: Der heutige Abruf von `app.js` erzeugt dieselbe Zeile, und
> die Pollerschleife erzeugt sie mehrfach pro Minute. Der Beitrag zur Last des
> 256-Byte-Empfangsrings ist damit kleiner als der eines einzigen Pollzyklus.
> **Watchdog:** Der Auslieferungspfad ist derselbe wie für `layout-previews.json`.
> `en.json.gz` ist mit **13'771 Byte** (gemessen) in der Grössenklasse von
> `styles.css` (9'913 B), nicht in der von `app.js` (129'297 B) — erfolgreiche Abrufe
> dieser Klasse dauern laut L125-Messung deutlich unter den 1,30 bis 1,52 s, die
> `app.js` braucht. Kein Pfad nähert sich den 20 s.
> **Hauptloop/EEPROM:** Keine EEPROM-Schreibzugriffe, also keine 16-ms-Blöcke. Der
> Hauptloop blockiert nicht länger als bei jeder anderen Asset-Auslieferung.
> **Hartkodierte Grenzen:** Geprüft und oben dokumentiert — die Namenspuffer (`72`,
> `64`, `128` Byte) tragen 19 Zeichen mit grossem Abstand; die Weissliste ist ein
> `sizeof`-berechnetes Array ohne feste Obergrenze. Jede weitere Sprache kostet
> **einen Zeiger im Flash** und bei jedem `http_app_installation_complete()` einen
> weiteren LittleFS-`exists`-Aufruf. Bei einem Dutzend Sprachen wäre das zu
> überdenken; bei drei bis vier ist es nicht messbar.

**Secure by design** — `escapeHtml` bei jedem `innerHTML`? Fremddaten als nicht
vertrauenswürdig behandelt? Keine Credentials in Logausgaben?

> **Antwort:** **Die Sprachdatei ist Fremddatum.** Sie kommt per HTTP von einem Gerät,
> dessen Update-Host konfigurierbar ist und dessen Verbindung unverschlüsselt läuft.
> Drei Massnahmen folgen daraus, alle oben ausformuliert: **(1)** JSON statt
> JavaScript — `response.json()` ist Datenübernahme, ein `<script>`-Import wäre
> Codeausführung aus genau dieser Quelle. **(2)** `sanitizeLanguageTable()` nimmt nur
> eigene Schlüssel mit Zeichenkettenwert und baut das Ziel mit `Object.create(null)`,
> womit ein Schlüssel `__proto__` oder `constructor` folgenlos bleibt. **(3)** Die
> Texte gehen über die bestehenden Wege in die Seite: `element.textContent`
> (`app.js:2245`), `setAttribute("aria-label"/"placeholder")` (`:2255`, `:2262`) und
> `showStatusBanner()`, das `banner.textContent` setzt (`app.js:2715`). **Kein neues
> `innerHTML`**, also auch kein neuer `escapeHtml`-Bedarf.
> Eine Einschränkung, die benannt gehört: Übersetzungen landen über `translateFormat`
> in vorhandenen `innerHTML`-Stellen, falls es solche gibt. Der Umsetzer prüft das für
> die neuen Schlüssel ausdrücklich — sie werden ausschliesslich über
> `showStatusBanner` ausgegeben, und das ist `textContent`.
> **Keine Credentials**: Die Dateien enthalten Oberflächentexte, keine Gerätedaten.
> Der neue Abruf protokolliert nichts ausser der ohnehin anfallenden HTTP-Debugzeile,
> die nur den Pfad nennt.
> **Der Upload-Endpunkt wird nicht aufgeweicht:** `http_find_app_install_asset()`
> bleibt ein exakter Namensvergleich. Begründung unter „Verworfene Alternativen".

**Stable & reliable** — Wird jeder Fehler ausgewertet? Leere `catch`? Stille
Verwerfungen? Ist der Zustand nach Abbruch mitten in einer Sequenz definiert? Wird ein
gesetztes Flag auf jedem Pfad wieder aufgelöst?

> **Antwort:** **Kein leeres `catch`.** Jeder Fehlerweg endet in einer sichtbaren
> Meldung und im Rückfall auf Deutsch. Das ist ausdrücklich **anders als beim
> Vorbild**: `loadLayoutPreviewRowsMap()` (`app.js:10017`) verschluckt seinen Fehler
> mit `.catch(() => {})` und liefert eine leere Karte. Für eine Vorschau mag das
> tragbar sein; für die Sprache ist es genau die stille Verwerfung, die die Checkliste
> meint.
> **Zwei Fehlerklassen werden unterschieden**, weil sie verschiedene Abhilfen haben:
> HTTP 404 → `language.load_missing` (die Datei liegt nicht auf der Uhr, es hilft nur
> ein neuer Upload), alles andere → `language.load_failed` (Zeitüberschreitung,
> Netzfehler, kaputtes JSON — ein zweiter Versuch kann helfen).
> **Flag auf jedem Pfad aufgelöst:** `pendingLanguage` wird im `finally` geräumt,
> `setLanguageSelectBusy(false)` ebenfalls. Beide laufen auch dann, wenn
> `applyLanguage` selbst wirft.
> **Zustand nach Abbruch mitten in einer Sequenz ist definiert:** Schaltet der Nutzer
> während eines laufenden Ladevorgangs weiter, verwirft der Token-Vergleich
> `pendingLanguage !== target` die veraltete Antwort — weder Anzeige noch
> Auswahlfeld noch `localStorage` werden von einer überholten Anfrage verändert. Es
> gibt keinen Zwischenzustand, in dem das Auswahlfeld eine Sprache zeigt, die nicht
> angezeigt wird (AK3), und `document.documentElement.lang` folgt derselben Regel
> (AK4).
> **Kein Wert wird still zurechtgebogen:** `normalizeLanguage()` fällt bei unbekanntem
> Code auf Deutsch zurück — das ist heute schon so und betrifft nur Werte aus
> `localStorage`, die der Nutzer nicht direkt setzt. Neu ist, dass ein **unvollständig
> geladener** Zustand gar nicht erst entsteht: Die Tabelle wird erst eingehängt,
> nachdem sie geprüft wurde.
> **Eine stille Verwerfung wird dabei aufgedeckt und geschlossen:** Der doppelte
> Schlüssel aus L130 ist genau so ein Fall — JavaScript nimmt klaglos das zweite
> Vorkommen, und die bestehende Prüfung sieht es nicht, weil sie Mengen vergleicht.
> Heute ist es folgenlos, weil beide Werte gleich sind; das ist Glück, keine
> Eigenschaft. Die neue Kennzahl Zeilen gegen Schlüssel macht daraus einen lauten
> Fehlschlag.
> **Teilweise fehlende Schlüssel sind kein Fehlerfall, sondern ein definierter
> Zustand:** `translate()` fällt je Schlüssel auf Deutsch zurück. Ein alter, im
> Service Worker liegender Stand kann die Oberfläche also gemischt zeigen, aber nie
> leer und nie mit rohen Schlüsseln. Der `?v=`-Parameter macht diesen Fall selten,
> schliesst ihn aber nicht aus — deshalb bleibt die Fallback-Kette erhalten.

## Verworfene Alternativen

**1. Eine Manifestdatei `i18n/index.json`, die die verfügbaren Sprachen aufzählt.**
Das wäre die reinste Form des Nutzerwunsches: Eine neue Sprache wäre dann wirklich
**nur** eine Datei, ohne jede Zeile in `app.js`. Verworfen aus zwei Gründen.
Erstens kostet es einen **zusätzlichen Abruf bei jedem Start** — auf dem Gerät, dessen
Auslieferung der kritische Befund L125 ist. Zweitens, und schwerer: Scheitert der
Abruf, ist das Auswahlfeld **leer** oder nur deutsch, und der englischsprachige
Nutzer kommt nicht mehr an seine Sprache, obwohl deren Tabelle im
Service-Worker-Cache liegt. Eine Zeile Daten in `app.js` gegen einen Startabruf auf
dem schwächsten Glied — die Zeile gewinnt. **Der Unterschied ist ehrlich zu benennen:
Eine vierte Sprache ist nach diesem Entwurf eine Datei plus zwei Datenzeilen
(`LANGUAGES`, `APP_INSTALL_ASSETS`), nicht eine Datei allein.** Die Logik bleibt
unangetastet, und das war die Forderung.

**2. Die Sprachdatei als JavaScript laden (`<script src>` oder `import()`).**
Spart den Parse-Schritt und wäre kürzer. Verworfen: Codeausführung aus einer Datei,
die über unverschlüsseltes HTTP von einem konfigurierbaren Host kommt. Dazu kommt ein
praktischer Haken — ein `import()` auf eine gzip-only ausgelieferte Datei hängt am
`Content-Type`, und der Service Worker müsste den Modulpfad gesondert behandeln.
**Und ein dritter Punkt, den L130 gerade geliefert hat:** JavaScript nimmt doppelte
Schlüssel stillschweigend hin. JSON tut das nicht — die strengere Form ist hier die
bessere.

**3. Deutsch ebenfalls auslagern, damit alle Sprachen gleich behandelt werden.**
Symmetrisch und architektonisch sauberer. Verworfen, weil es Anforderung 1 verletzt:
Die Oberfläche muss ohne **jeden** Abruf bedienbar sein. Die Asymmetrie ist kein
Entwurfsfehler, sondern die Zusicherung selbst.

**4. Beim Build je Sprache ein eigenes `app.js` erzeugen.** Kleinste mögliche Datei je
Nutzer. Verworfen: Die Sprache liesse sich zur Laufzeit nicht mehr wechseln, die
Weissliste und `install-app.sh` müssten je Sprache ein eigenes Bündel führen, und das
Gerät müsste wissen, welches es ausliefert. Das vervielfacht genau die Mechanik, die
am 29.04.2026 schon einmal zum Verschwinden der PWA geführt hat.

**5. Alle Fremdsprachen in **eine** `i18n.json`.** Eine Datei weniger in jeder Liste.
Verworfen: Dann lädt der italienische Nutzer auch Englisch und Französisch mit — und
mit der vierten Sprache wächst der Download für alle. Genau das ist die heutige
Krankheit, nur eine Datei weiter geschoben.

**6. `http_find_app_install_asset()` auf ein Präfix `app/i18n/` aufweichen**, damit
der ESP für neue Sprachen gar nicht mehr angefasst werden muss. Reizvoll, und es wäre
die vollständige Erfüllung von „nur eine neue Datei". Verworfen: Diese Funktion ist
die **einzige** Prüfung vor einem schreibenden Zugriff ins LittleFS
(`http_api_app_file_upload`, `:6395`). Ein Präfixvergleich liesse beliebige Namen
unter diesem Präfix zu — in einem Dateisystem, in dem `fs_remove?filename=app.js.gz`
bereits als gefährlicher Endpunkt geführt wird (CLAUDE.md, R5). Zwölf exakte Namen
sind überschaubar; ein Präfix ist eine Tür.

## Versionsfolgen

- [ ] STM `src/main.h` anheben — **nein**, `src/**` wird nicht berührt
- [x] ESP `version.h` anheben — `http.cpp` geändert
- [x] App `APP_VERSION` anheben — `app.js` geändert
- [x] `CACHE_NAME` in `sw.js` anheben — gehört zwingend zu `APP_VERSION` (R4)

Nur der `release-engineer` führt das aus (R4).
