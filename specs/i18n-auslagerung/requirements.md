# Anforderungen — Sprachtabellen auslagern, Deutsch fest im Bundle

**Status:** Entwurf
**Auslöser:** Nutzerwunsch vom 03.10.2026, `BEFUNDE.md` L127 (Massnahme **B15**), Zusammenhang L125, L126 und L130
**Befundstatus:** `✔ verifiziert` — die Zahlen in L127 sind gemessen, die Erwartung in
AK11 ist am 03.10.2026 am heutigen Stand nachgemessen, und die Mechanik in `http.cpp`
ist für diese Spec zusätzlich am Quelltext nachgelesen (siehe `design.md`, Abschnitt
„Was am Gerätequelltext nachgeprüft wurde")

## Auftrag des Nutzers, wörtlich

> „könnten wir die Sprache die zusätzlichen Sprachen nicht einfach in eine separate
> Datei auslagern und somit bspw. Deutsch als Default nehmen?"

> „Das würde uns auch die Möglichkeit geben, dass wir später bspw. Italienisch oder
> Französisch noch dazunehmen."

**Das Ziel ist die Erweiterbarkeit, nicht die Einsparung.** Die rund 59'400 Byte, die
heute wegfallen, sind der Nebeneffekt. Gebaut wird eine Struktur, in der eine vierte
Sprache eine neue Datei ist und keine neue Logik.

## Problem

`const I18N` steht in `ESP8266/ESP-uclock/data/app/app.js` **ab Zeile 15 bis
Zeile 1878** — 1'864 Zeilen, **123'116 Byte, 21,3 % der Datei** (L127, gemessen).
Beide Sprachen sind vollständig und gleich gross: je **929 Zeilen** mit Schlüsseln,
Deutsch 63'664 Byte, Englisch 59'433 Byte. Benutzt wird immer genau **eine**.

Drei Folgen, die zusammengehören:

1. **Jede neue Sprache kostet rund 60 kB Quelle im Startbundle** — bei jedem Abruf,
   für jeden Nutzer, auch für den, der sie nie wählt. Drei Sprachen wären 180 kB,
   vier 240 kB. Der Wunsch des Nutzers verschlechtert die Lage also genau so lange,
   wie die Tabellen in der Datei stehen.

2. **`app.js.gz` ist die einzige Datei, bei deren Auslieferung der ESP abstürzt**
   (L125, **offen, KRITISCH**): fünf Exceptions am 03.10.2026 zwischen 18:17 und
   18:24, alle identisch (`epc1=0x4000df64` = `memcpy+0x1c`, `excvaddr=0x00000000`,
   `ctx: sys`, jeweils `rst cause:2`). Je fünf Abrufe im Vergleich: `app.js`
   (129'313 B) 1/5 Abstürze, `styles.css` (9'913 B) 0/5, `index.html` (11'377 B) 0/5,
   `sw.js` (1'010 B) 0/5. Jeder dieser Abstürze nimmt die Oberfläche mit **und**
   löscht den Variablensatz des ESP (L42/L103, gemessen: 538 verworfene Zeichen auf
   der Brücke je Neustart).

3. **`app.js` ist in zwölf Tagen um 20 % gewachsen** (L126): 1.4.70 → 1.4.82 von
   481'081 auf 577'425 Byte Quelle, gepackt von 98'193 auf 129'313 Byte. Der grösste
   Einzelposten nach dem Code sind die Übersetzungen.

**Was diese Änderung ausdrücklich NICHT behauptet:** Sie behebt L125 nicht. Der
Absturz liegt auf dem ESP, und ein kleineres `app.js` verschiebt nur die
Wahrscheinlichkeit, es beseitigt die Ursache nicht. Die Massnahme **C9** bleibt
danach unverändert offen. Wer das verwechselt, repariert ein ESP-Problem in `app.js`
— genau der Fehler, den die Architektur-Checkliste unter „richtige Schicht" meint.

### Nebenbefund, der hier mit erledigt wird: L130

`"system.debug_eyebrow"` steht **doppelt** — `app.js:95` und `:110` (Deutsch), `:1026`
und `:1041` (Englisch). Es sind also 929 Zeilen, aber nur **928 verschiedene
Schlüssel**. Beide Vorkommen tragen denselben Wert `"Debug"`, heute ist es damit
folgenlos.

**Für diese Änderung ist es nicht folgenlos:** Ein JSON-Objekt kann einen Schlüssel
nicht zweimal führen. `JSON.parse` verwirft stillschweigend das erste Vorkommen,
manche Exportwege brechen ab. Das Duplikat wird deshalb **vor** dem Export entfernt,
in beiden Sprachen (T3).

**Die Prüflücke dahinter ist der eigentliche Befund (L130):**
`tools/checks/i18n-keys.mjs` sammelt die Schlüssel in ein `Set` (`:16-20`) und
vergleicht die **Mengen** de gegen en. Beide enthalten das Duplikat, beide Mengen sind
gleich gross — S3 meldet nichts. Trügen die zwei Vorkommen verschiedene Werte, zeigte
die Oberfläche still den zweiten, und keine Prüfung sagte etwas dazu. Deshalb bekommt
die Stufe in T5 eine zweite Kennzahl: **Zeilenzahl gegen Schlüsselzahl**, nicht nur de
gegen en.

## Ziel

Deutsch bleibt fest in `app.js` eingebaut; jede weitere Sprache liegt als eigene
`.json`-Datei unter `data/app/i18n/` und wird erst beim Umschalten geholt. Scheitert
das Nachladen, bleibt die Oberfläche vollständig bedienbar auf Deutsch und sagt es.
Eine vierte Sprache ist danach **eine neue Datei plus je eine Datenzeile** in der
Sprachliste der App und in der Weissliste des ESP — kein Eingriff in die Logik, keine
Änderung an `install-app.sh`, `deploy.sh`, `Makefile` oder den Guardrails.

## Akzeptanzkriterien

### Funktion

- [ ] **AK1 — Deutsch ohne jeden Abruf.** Mit blockiertem Netzzugriff auf
      `/app/i18n/*` (DevTools → Network → Block request URL) ist die Oberfläche
      vollständig auf Deutsch bedienbar: Alle Module lassen sich öffnen, kein
      Textplatz zeigt einen rohen Schlüssel wie `maintenance.target_expected`.
      Nachweis: Bildschirmfoto plus leere Konsole ausser der erwarteten Fehlermeldung.
- [ ] **AK2 — Englisch wird nachgeladen und angewendet.** Umschalten auf Englisch
      erzeugt **genau einen** Abruf von `/app/i18n/en.json?v=<APP_VERSION>`, danach
      sind die statisch übersetzten Stellen englisch. Zweites Umschalten hin und
      zurück erzeugt **keinen** weiteren Abruf (Tabelle bleibt im Speicher).
- [ ] **AK3 — Fehlschlag ist sichtbar, nicht stumm.** Schlägt der Abruf fehl, zeigt
      die Statusleiste die Meldung aus `language.load_failed` bzw.
      `language.load_missing` (Wortlaut siehe unten), die Oberfläche bleibt auf
      Deutsch, und **das Auswahlfeld springt auf Deutsch zurück**. Es darf nie
      „English" anzeigen, während deutscher Text dasteht.
- [ ] **AK4 — `document.documentElement.lang` stimmt mit dem Angezeigten überein.**
      Nach einem fehlgeschlagenen Wechsel steht dort `de`, nicht `en`. Prüfbar in der
      Konsole: `document.documentElement.lang`.
- [ ] **AK5 — Die gespeicherte Sprache wird erst nach Erfolg gesetzt.**
      `localStorage["wordclock-language"]` bleibt nach einem gescheiterten Wechsel auf
      dem alten Wert.
- [ ] **AK6 — Fehlender Schlüssel fällt auf Deutsch zurück, nie auf den Rohschlüssel.**
      Mit einer künstlich um einen Schlüssel gekürzten `en.json` erscheint an dieser
      Stelle der deutsche Text.

### Dateien und Auslieferung

- [ ] **AK7 — Der ESP findet die Datei.** `curl -s -D- -o/dev/null
      "http://$DEVICE_HOST/app/i18n/en.json"` antwortet mit `200`,
      `Content-Encoding: gzip` und `Content-Type: application/json`. Kein
      Plain-Fallback existiert — es gibt auf dem Gerät nur `app-i18n-en.json.gz`.
- [ ] **AK8 — `install-app.sh --check` kennt die Datei.** Der Lauf nennt
      `app-i18n-en.json.gz` und meldet „Alle erwarteten Assets liegen auf dem Gerät."
      Fehlt sie, nennt er sie namentlich.
- [ ] **AK9 — Der Upload wird vom ESP angenommen.** `install-app.sh` meldet für
      `app/i18n/en.json` `{"ok":true}`. (Ohne den Weisslisteneintrag aus Task 2 kommt
      hier `error_code: 1` — das ist der Prüfpunkt für Punkt 2 des Auftrags.)
- [ ] **AK10 — Der Update-Server trägt die Datei.** Nach `deploy.sh` liegt
      `app/i18n/en.json.gz` im Zielverzeichnis, nicht nur im Repo.

### Grösse — die Erwartung ist gemessen, nicht geschätzt

Am **03.10.2026** am heutigen Stand nachgemessen, indem der `en:`-Block
(Zeile 947–1877, 929 Zeilen, 59'433 Byte) entfernt und das Ergebnis gepackt wurde:

| | Quelle | `.gz` | Frames à 256 Byte |
|---|---|---|---|
| `app.js` heute | 577'421 | 129'297 | 506 |
| `app.js` ohne `en` | 517'987 | 115'843 | 453 |
| `en.json` separat | 57'568 | 13'771 | — |

Quelle **−59'434 Byte (10,3 %)**, gepackt **−13'454 Byte (10,4 %)**, Frames
506 → 453 (**Faktor 1,12**).

> **Was die Frames sagen und was nicht:** Die Stillstandsstellen aus L125 lagen bei
> 25'344 · 76'800 · 83'712 Byte, und 76'800 sind exakt 75 × 1024. Eine Datei, die 53
> Frames weniger braucht, trifft weniger Gelegenheiten — **sie beseitigt die Ursache
> nicht.** Faktor 1,12 ist keine Grössenordnung. Diese Zeile steht hier, damit niemand
> sie später als Beleg für „L125 erledigt" liest (DIR-003).

- [ ] **AK11 — `app.js` schrumpft wie vorhergesagt.** Der umsetzende Agent hält den
      Vorher-Wert (`wc -c` auf `app.js` und `app.js.gz`) **vor** seiner Änderung fest
      und den Nachher-Wert danach. Erwartet gegen die Tabelle oben: Quelle rund
      **−59'400 Byte**, gepackt rund **−13'450 Byte**. Untergrenze für ein Bestehen:
      **−55'000 Byte Quelle**. Weicht das Ergebnis deutlich **nach unten** ab, ist
      vermutlich nicht alles ausgelagert worden; weicht es nach **oben** ab, wurde
      mehr entfernt als der `en:`-Block. Beides gehört in den Bericht, nicht
      stillschweigend hingenommen.
- [ ] **AK12 — Die ausgelagerte Datei passt zur Messung.** `en.json` liegt bei rund
      **57'568 Byte** (gepackt rund 13'771). Die Datei ist kleiner als die entfernten
      59'433 Byte, weil die JavaScript-Einrückung und die `en: {`-Hülle entfallen —
      eine Abweichung **nach oben** heisst, dass beim Umformen etwas schiefgegangen ist.

### Prüfungen

- [ ] **AK13 — Schlüsselparität wird weiter geprüft, jetzt über Dateigrenzen.**
      `./tools/guardrails.sh` Stufe S3 liest die deutsche Tabelle aus `app.js` und
      **jede** Datei unter `data/app/i18n/*.json` und meldet je Sprache fehlende und
      überzählige Schlüssel. Ergebnis beim Abschluss: Differenz 0.
- [ ] **AK14 — S3 erkennt eine künstlich entfernte Zeile.** Einmal gegenprobiert:
      Ein aus `en.json` gelöschter Schlüssel wird gemeldet. Eine Prüfung, die nach dem
      Umbau nichts mehr prüft, ist schlimmer als keine (dieselbe Falle wie L123).
- [ ] **AK15 — S3 erkennt einen doppelten Schlüssel (L130).** Die Stufe vergleicht
      **Zeilenzahl gegen Schlüsselzahl** je Sprache und meldet jede Differenz mit dem
      Namen des doppelten Schlüssels. Gegenprobe: eine Zeile in `app.js` absichtlich
      verdoppeln, die Meldung muss kommen; danach zurücksetzen. Im Endzustand gilt für
      beide Sprachen **Zeilen = Schlüssel = 928**.
- [ ] **AK16 — `./tools/guardrails.sh` läuft mit Exit 0 durch**, Stufe S5 eingeschlossen.
- [ ] **AK17 — `./tools/guardrails.sh --full` läuft mit Exit 0 durch.**
- [ ] **AK18 — Der `pwa-tester` hat Phase 0 bis 4 und 9 abgearbeitet.** Pflicht nach
      DIR-012: Dieses Release berührt ESP **und** PWA **und** Build.

### Erweiterbarkeit — der eigentliche Zweck

- [ ] **AK19 — Trockenübung „vierte Sprache".** Der Umsetzer legt probeweise eine
      `it.json` mit **fünf** Schlüsseln an, trägt eine Zeile in die Sprachliste ein
      und weist nach: `make app-gz` packt sie, `install-app.sh --check` erwartet sie,
      S5 prüft sie, S3 meldet die fehlenden Schlüssel. **Danach wird die Datei wieder
      entfernt** — Italienisch ist nicht Teil dieser Änderung (siehe unten).
      Ohne diese Probe ist die Erweiterbarkeit behauptet, nicht belegt.
- [ ] **AK20 — Kein Werkzeug nennt eine Sprache beim Namen.** In `install-app.sh`,
      `deploy.sh`, `Makefile` und `guardrails.sh` steht nirgends `en.json`; alle vier
      greifen über `i18n/*.json`. Prüfbar: `grep -rn 'en\.json' Makefile tools/` ist
      leer.

## Wortlaut der Fehlermeldungen

Beide Schlüssel stehen in der **deutschen** Tabelle in `app.js` — sie werden gebraucht,
wenn gerade keine andere Tabelle da ist. `{language}` ist der Eigenname der Sprache aus
der Sprachliste („English", später „Italiano"), nicht der Sprachcode.

| Schlüssel | Text |
|---|---|
| `language.load_failed` | `Die Texte für {language} konnten nicht geladen werden. Die Oberfläche bleibt auf Deutsch — prüf die Verbindung zur Uhr und versuch es noch einmal.` |
| `language.load_missing` | `Die Sprachdatei für {language} liegt nicht auf der Uhr. Die Oberfläche bleibt auf Deutsch — lad die App-Dateien neu auf die Uhr, dann steht die Sprache wieder zur Verfügung.` |

`load_missing` bei HTTP 404 (die Datei fehlt im LittleFS — anderer Weg zur Lösung),
`load_failed` bei allem anderen (Zeitüberschreitung, Netzfehler, kaputtes JSON).
Die Unterscheidung ist kein Luxus: Beim ersten Fall hilft nur ein neuer Upload, beim
zweiten ein zweiter Versuch.

Beide Texte laufen über `showStatusBanner(message, "error")` (`app.js:2709`), das
`textContent` setzt — kein `innerHTML`, also kein Weg für Fremdinhalt in die Seite.

## Nicht Teil dieser Änderung

- **Keine neue Sprache.** Weder Italienisch noch Französisch. Erst die Struktur, dann
  später die Sprache als **reine Datenergänzung**. Die `it.json` aus AK19 ist ein
  Prüfmittel mit fünf Schlüsseln und wird wieder entfernt.
- **Kein Fix für L125/C9.** Der ESP-Absturz beim Ausliefern von `app.js` bleibt offen.
  Diese Änderung macht die Datei um Faktor 1,12 kleiner, mehr nicht.
- **Die deutsche Tabelle wird nicht ausgelagert.** Sie ist die Zusicherung aus AK1.
- **Die rund 176 hartcodierten Strings aus B8 werden nicht mitgezogen.** Eigener
  Befund, eigener Auftrag. Wer sie beim Umformen entdeckt, meldet sie im Bericht und
  lässt sie stehen.
- **Ausser dem Duplikat aus L130 wird kein Schlüssel angefasst.** Keine Umbenennung,
  keine Textkorrektur, kein Aufräumen ungenutzter Schlüssel. Das Duplikat ist die
  einzige Ausnahme, und zwar weil JSON es erzwingt — nicht weil es gerade auffiel.
- **`translate()` bleibt synchron.** Keine Umstellung der rund 900 Aufrufstellen auf
  `await`. Wer das anfängt, baut eine andere Änderung.
- **Keine Spracherkennung über `navigator.language`.** Das Verhalten beim ersten Start
  bleibt wie heute: Deutsch, bis der Nutzer umschaltet.
- **`LOCAL_APP_REQUIRED_ASSETS` (`app.js:1879`) bleibt unverändert.** Das ist die Liste
  der Dateien, ohne die die PWA kaputt ist — eine Sprachdatei gehört ausdrücklich
  nicht dazu, sonst liesse sich ein älteres Bündel nicht mehr lokal einspielen.
- **Die Weissliste in `http.cpp` wird nicht auf ein Präfix aufgeweicht.** Begründung
  in `design.md`, „Verworfene Alternativen".
- **Keine Änderung an der Legacy-Oberfläche.**

## Betroffene Laufzeiten

- [ ] STM32 (`src/**`) — **nicht berührt**
- [x] ESP8266 (`ESP8266/ESP-uclock/*.cpp`) — **Neu-Flashen nötig**, und zwar
      **bevor** die PWA hochgeladen wird (sonst weist der ESP die Sprachdatei ab)
- [x] PWA (`data/app/**`) — LittleFS-Upload
- [x] Build/Release — `Makefile`, `tools/**`, Guardrails

## Rollout-Reihenfolge — nicht vertauschbar

1. `make release-zip`
2. `./tools/deploy.sh` (bringt `app/i18n/en.json.gz` auf den Update-Server)
3. **ESP flashen** — ohne den Weisslisteneintrag weist der ESP den Upload der
   Sprachdatei mit `error_code: 1` ab
4. `./tools/install-app.sh` (lädt die PWA samt Sprachdatei ins LittleFS)
5. `./tools/install-app.sh --check`, `./tools/smoke-device.sh`, `./tools/check-pwa.sh`
6. Commit und Tag `release/<stm>-<esp>-<app>` (DIR-011)

**Was passiert, wenn Schritt 3 vergessen wird:** `install-app.sh` meldet
„1 Datei(en) fehlgeschlagen" und endet mit Exit 1 — der Fehler ist laut, nicht still.
Auf dem Gerät bleibt die Oberfläche voll bedienbar auf Deutsch, Englisch zeigt die
Meldung `language.load_missing`. Das ist der entworfene Degradationsfall und kein
Schaden.
