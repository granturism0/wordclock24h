# Anforderungen — Den Auslieferungspfad des ESP entlasten

**Status:** Entwurf
**Datum:** 2026-10-03 (Momentaufnahme, DIR-006 — wird nicht fortgeschrieben)
**Auslöser:** `BEFUNDE.md` Massnahmen **C9** (L128) und **C9b** (L129), beide im
Zusammenhang mit **L125** (ESP-Absturz beim Ausliefern von `app.js`, *offen, KRITISCH*)

**Befundstatus der übernommenen Befunde:**

| Befund | Status | Wie geprüft |
|---|---|---|
| L128 — 505 Frames, `http.cpp:1184` (256 Byte), `:12430` (`setNoDelay(1)`) | `✔ verifiziert` | Beide Fundstellen beim Schreiben dieser Spec am Quelltext nachgelesen; 129'313 / 256 = 505,1 nachgerechnet |
| L129 (a) Endlosschleife 1182–1191 | `✔ verifiziert` für die Codeform, `● gemeldet` für die Core-Aussage | Die Schleife ohne `break` und ohne `yield()` steht so da. Dass `LittleFS.h:411–422` des Cores 3.1.2 bei `lfs_file_read < 0` eine **0** zurückgibt, ist aus L129 übernommen und **nicht** am Core nachgelesen — das muss der Umsetzer tun (siehe AK12) |
| L129 (b) unpaariges `LittleFS.begin()` bei 1535 | `✔ verifiziert` | Alle drei Rückkehrwege (1543, 1557, 1561) kehren ohne `LittleFS.end()` zurück. Zählung in `http.cpp`: **28 × `begin`, 27 × `end`** — nachgezählt, deckt sich mit L129 |
| L129 (c) dreifacher Mount je `/app/*`-Request | `✔ verifiziert`, und **ein Stück schärfer als der Befund** | 1535, 1231, 1156 belegt. Zusätzlich gefunden: `http_app_installation_complete()` wird bei **jedem** `/app/*`-Request gerufen (`:1647`), sein Ergebnis `app_complete` aber nur in zwei Zweigen benutzt, die beide `is_pwa_index` voraussetzen (`:1695`, `:1700`). Für `/app/app.js` ist der Wert **nie nötig** — zwölf Dateiprüfungen und ein Mount laufen folgenlos, unmittelbar vor der grossen Übertragung. Im Katalog als **L129 (d)** geführt |
| L125 — fünf Exceptions, `memcpy` mit Ziel NULL im Systemtask | `✔ verifiziert` (aus L125 übernommen, am Gerät gemessen) | Nicht von dieser Spec nachgemessen. Die Einordnung „Ursache in SDK-/lwIP-Code, kein Projektcode im Systemkontext" ist in L125 belegt |

---

## Das Wichtigste zuerst: Das ist keine bewiesene Lösung

**Diese Änderung behebt L125 nicht nachweislich.** Sie senkt eine Last, von der
vermutet wird, dass sie den Absturz auslöst. Das steht hier oben und nicht im
Kleingedruckten, weil die Versuchung gross ist, nach dem Flash „behoben" zu schreiben.

Was L125 dazu sagt, wörtlich im Sinn: Die Rechnung aus den Messungen ergibt **rund
0,04 % Absturzrisiko je Frame** (1 Absturz auf 5 Abrufe à 505 Frames). Dieselbe Rate
auf `index.html` (45 Frames) angewandt ergäbe rund 1,8 % je Abruf — also mit etwa 91 %
Wahrscheinlichkeit 0 Abstürze auf 5 Abrufe, und genau das wurde gemessen. **Die
Messung widerspricht einem linearen Risiko je Frame nicht.** Daraus folgt:

- Es gibt nach heutigem Stand **wahrscheinlich keine Grössenschwelle**, unterhalb derer
  die Auslieferung sicher ist.
- Faktor 2,10 weniger Frames heisst nach diesem Modell **rund halb so oft**, nicht „nie
  mehr". Erwartungswert nach dem Umbau: statt rund einem Absturz je fünf Abrufe rund
  einer je zehn.
- Ein sauberer Durchlauf nach dem Flash **beweist nichts**. Bei 241 Frames und
  unverändertem Risiko je Frame sind 20 fehlerfreie Abrufe mit etwa 15 %
  Wahrscheinlichkeit auch dann zu erwarten, wenn sich gar nichts verbessert hat.
- Die Ursache liegt in SDK-/lwIP-Code (L125, belegt: kein Projektcode läuft im
  Systemkontext). **Änderbar ist nur die Last, die dorthin führt.**

**Dasselbe Argument an einer zweiten Messgrösse, aus der i18n-Spec** (nachgemessen am
03.10.2026): Die drei Stillstandsstellen aus L125 liegen bei **25'344 · 76'800 ·
83'712 Byte**. Nach der Sprachauslagerung misst `app.js.gz` **115'843 Byte** — **alle
drei Stellen liegen weiterhin darunter.** Die Datei fällt also auch dann nicht aus dem
Gefahrenbereich heraus; sie bietet nur weniger Gelegenheiten. Ob man die Last über die
Framezahl senkt (diese Spec) oder über die Dateigrösse (B15): Beide Wege verschieben
die Wahrscheinlichkeit, keiner beseitigt die Ursache.

Wer nach dieser Änderung in `BEFUNDE.md` den Status von L125 anfasst, schreibt
„Last gesenkt, Rate erwartet halbiert, Ursache unverändert offen" — nicht „erledigt".

---

## Problem

### Teil 1 — Der ESP zerlegt `app.js` in 505 einzelne Funkpakete (L128)

`ESP8266/ESP-uclock/http.cpp:1182-1191` liest die Datei in **256-Byte-Blöcken** und
schreibt jeden Block einzeln auf den Socket:

```c
while (fp.available ())
{
    uint8_t buf[256];
    size_t  len = fp.read (buf, sizeof (buf));

    if (len > 0)
    {
        http_client.write (buf, len);
    }
}
```

`http.cpp:12430` setzt `http_client.setNoDelay(1)` für **jeden** eingehenden Request,
also auch für die Dateiauslieferung. `TCP_WRITE_FLAG_MORE` wird dabei nie gesetzt
(L128: `ClientContext.h:519–522`, die Bedingung `next_chunk_size < remaining` ist bei
256-Byte-Schreibvorgängen nie wahr). Jeder Block geht damit sofort als eigenes Paket
hinaus: **129'313 / 256 = 505 Frames in 1,385 s, also 365 Frames je Sekunde**, jeder
einzeln quittiert.

Zum Vergleich die Dateien, bei denen **nie** ein Absturz auftrat: `index.html.gz`
45 Frames, `styles.css.gz` 39, `sw.js.gz` 4.

### Teil 2 — Drei Defekte in derselben Schleife und ihrem Umfeld (L129)

**(a) Endlosschleife bei Lesefehler.** Liefert `fp.read()` eine 0 — laut L129 der
Rückgabewert des Cores 3.1.2 auch im Fehlerfall `lfs_file_read < 0` —, dann greift
`if (len > 0)` nicht, die Dateiposition rückt nicht vor, `fp.available()` bleibt wahr.
Die Schleife läuft endlos, **ohne `yield()`**: garantierter Soft-Watchdog-Reset. Das
ist ein eigener Fehlerfall, nicht das bei L125 beobachtete Bild.

**(b) Unpaariges `LittleFS.begin()`.** `http_find_stored_app_asset_filename`
(`:1531`) mountet bei `:1535` und kehrt auf **allen drei** Wegen (`:1543`, `:1557`,
`:1561`) ohne `LittleFS.end()` zurück. Nachgezählt über `http.cpp`: 28 × `begin`,
27 × `end`.

**(c) Dreifacher Mount je `/app/*`-Request.** `:1535`, dann
`http_app_installation_complete` (`:1231`/`:1253`, prüft alle zwölf Assets einzeln),
dann `http_send_fs_file` (`:1156`/`:1200`). Jeder Mount alloziert vier 64-Byte-Caches
plus die `lfs_t`-Struktur — wenig Speicher, aber **Fragmentierung des 12'400-Byte-Heaps
unmittelbar bevor die Sendekette ihre Puffer braucht**. Der Zusammenhang mit L125 ist
**nicht belegt** (DIR-003); der Fix ist klein und kostet nichts.

**Zusatzbefund dieser Spec zu (c), im Katalog als L129 (d):** Der Aufruf bei `:1647`
ist für jeden Abruf einer einzelnen Datei **vollständig folgenlos**. `app_complete`
wird nur bei `:1695` (`is_pwa_index && ! app_complete`) und `:1700`
(`! is_pwa_index || app_complete`) gelesen; ist `is_pwa_index` gleich 0, entscheidet
der Wert in beiden Fällen nichts. Für `/app/app.js` laufen also ein Mount, zwölf
`LittleFS.exists`/`open`/`size`/`close` und ein Unmount, deren Ergebnis weggeworfen
wird — direkt vor der Übertragung, bei deren Beginn der Heap zählt.

### Teil 3 — Die Heap-Vermutung ist heute nicht messbar (M4)

L125 nennt als unbelegte Vermutung: „Die Sendekette fordert je Frame einen Puffer an;
scheitert das bei 12'400 Byte Gesamtspeicher und wird der NULL-Rückgabewert nicht
geprüft, trifft `memcpy` die Adresse 0."

**Diese Vermutung lässt sich heute nicht prüfen, auch nicht grob.** `ESP.getFreeHeap()`
kommt in `http.cpp` **nirgends** vor; es gibt keinen Weg, den freien Speicher des ESP
abzufragen — weder über die PWA noch über `curl`. Die 12'400 Byte stammen aus der
`design.md` des Beobachtbarkeits-Pakets, nicht aus einer laufenden Messung.

Ohne diese Zahl ist jede Aussage über „Speicherdruck" vor und nach dem Umbau eine
Behauptung. **Das ist die einzige Messung, die die Heap-Vermutung direkt prüfen kann** —
und sie unterscheidet zwei Fälle, die sich sonst nicht trennen lassen: **zu wenig
Speicher** (`free_heap` klein) gegen **zerstückelten Speicher** (`free_heap` gross,
`max_free_block` klein). Der zweite Fall ist genau das Bild, das eine fehlgeschlagene
Pufferanforderung mit nicht geprüftem NULL-Rückgabewert erzeugt.

---

## Ziel

Der ESP liefert `app.js` mit **rund der Hälfte der Frames** aus, die Sendeschleife hat
auf jedem Pfad einen definierten Ausgang statt einer Endlosschleife, jedes
`LittleFS.begin()` im Auslieferungspfad hat sein `end()`, und der folgenlose
Zwölf-Dateien-Check entfällt für Einzelabrufe. Zwei neue Felder in einer bestehenden
lesenden JSON-Antwort machen den freien Heap und den grössten freien Block erstmals
messbar — vor und nach dem Umbau.

**Die Absturzrate sinkt erwartungsgemäss, die Ursache bleibt offen.** C9 und C9b sind
danach erledigt, **L125 nicht**.

---

## Akzeptanzkriterien

### Nichtregression — jeder Request läuft durch diese Datei

- [ ] **AK1 — `app.js` kommt vollständig an.** Fünf Abrufe hintereinander:
      `curl -s -o /dev/null -w '%{http_code} %{size_download} %{time_total}\n'
      "http://$DEVICE_HOST/app/app.js"`. Jeder Lauf: `200`, `size_download` gleich der
      Dateigrösse von `app.js.gz` auf dem Gerät (`/api/fs_list`), kein Lauf kürzer.
- [ ] **AK2 — Die übrigen Assets ebenfalls.** Je ein Abruf von `/app/`,
      `/app/index.html`, `/app/styles.css`, `/app/sw.js`, `/app/manifest.webmanifest`
      und zwei Icons: `200`, `Content-Encoding: gzip`, erwarteter `Content-Type`.
      `./tools/install-app.sh --check` meldet alle erwarteten Assets.
- [ ] **AK3 — `./tools/smoke-device.sh` ohne Fehlschlag**, einschliesslich der
      Absturzfestigkeit bei Parametern ohne `=` und der Abwehr eingebetteter
      Wartungsaufrufe über `Sec-Fetch-Dest`.
- [ ] **AK4 — `./tools/check-pwa.sh` meldet keinen Fehler beim Laden von `app.js`.**
      Eine Oberfläche, die halb leer bleibt, zeigt weder API noch Smoketest.
- [ ] **AK5 — `pwa-tester` hat Phase 0 bis 4 und 9 abgearbeitet**, ohne neuen Befund.
      Pflicht nach DIR-012, auch wenn nur der ESP geändert wurde: Die geänderte
      Funktion liegt im Weg **jedes** Requests.
- [ ] **AK6 — Die Legacy-Oberfläche lädt weiterhin** (`http://$DEVICE_HOST/`). Sie ist
      die Stabilitäts-Referenz beim Debuggen.

### Framezahl — der eigentliche Hebel

- [ ] **AK7 — Die Framezahl ist gemessen gesunken.** Mitschnitt mit `tcpdump` nach dem
      Verfahren in `design.md` („Wie die Framezahl nachgewiesen wird"). Gezählt werden
      Pakete **mit Nutzlast vom Gerät zum Rechner** für genau einen `app.js`-Abruf.
      Bestanden bei **höchstens 280 Frames** (erwartet 241 bis 253; Boden ist
      `TCP_MSS = 536`). Der Vorher-Wert 505 stammt aus L128 und wird **nicht** noch
      einmal am Gerät erhoben — die Messung kostet sonst eine Absturzrunde ohne
      Erkenntnisgewinn.
- [ ] **AK8 — Die Übertragung wird nicht langsamer.** Mittel aus fünf Abrufen
      (`%{time_total}`) höchstens so gross wie der in Runde A notierte Vorher-Wert.
      Eine Verlangsamung wäre das Warnzeichen für ein Nagle-Problem im Fehlerfall.

### Messmittel M4

- [ ] **AK9 — Die beiden Felder sind da und plausibel.**
      `curl -s "http://$DEVICE_HOST/api/device_ready"` liefert zusätzlich zu
      `ok` und `ready` die Zahlenfelder `free_heap` und `max_free_block`.
      Plausibel heisst: beide > 0, `max_free_block <= free_heap`, `free_heap` in der
      Grössenordnung einiger tausend bis einiger zehntausend Byte. Antwort bleibt
      gültiges JSON (`python3 -m json.tool` ohne Fehler).
- [ ] **AK10 — Vorher und Nachher sind protokolliert.** Je vier Werte, festgehalten im
      Bericht und später in `BEFUNDE.md` (durch den `doc-writer`, nicht durch den
      Umsetzer): `free_heap`/`max_free_block` **im Ruhezustand** und **unmittelbar nach
      fünf `app.js`-Abrufen**, je einmal in Runde A (nur M4 geflasht) und einmal in
      Runde B (Umbau geflasht). **Ohne Bewertungszwang** — die Zahlen sind das
      Ergebnis, auch wenn sie nichts zeigen.
- [ ] **AK11 — Kein neuer Endpunkt, keine neue UART-Last.** `/api/`-Pfade in
      `http.cpp` unverändert in der Zahl. Im Normalbetrieb erzeugt kein geänderter
      Pfad eine zusätzliche Zeile auf der STM-UART; nachprüfbar über den Logring
      (`/api/stm32_log` bzw. `./tools/measure-log.sh`): Ein `app.js`-Abruf erzeugt dort
      nicht mehr Zeilen als vor dem Umbau. Der ESP hat nur einen vollwertigen UART, und
      das ist die Brücke zum STM.

### Die drei Defekte

- [ ] **AK12 — Die Sendeschleife kann nicht mehr endlos laufen.** Nachweis am
      Quelltext, in dieser Reihenfolge prüfbar: (1) Der Rückgabewert von `fp.read()`
      wird in einem **vorzeichenbehafteten** Typ aufgefangen; (2) jeder Durchlauf
      bewirkt entweder Fortschritt in der Datei oder verlässt die Schleife über
      `break`; (3) im Schleifenrumpf steht ein `yield()`. Zusätzlich: Der Umsetzer hat
      im Core 3.1.2 **nachgesehen** — nicht angenommen —, was `File::read` im
      Fehlerfall zurückgibt, und schreibt den gefundenen Rückgabetyp und -wert in
      seinen Bericht. Ein künstlicher Lesefehler ist am Gerät nicht herstellbar; das
      ist der Grund, warum hier der Quelltext zählt und nicht ein Gerätetest.
- [ ] **AK13 — Mounts sind paarig.** In `http_find_stored_app_asset_filename` steht
      jedem `LittleFS.begin()` auf **jedem** Rückkehrweg ein `LittleFS.end()`
      gegenüber. Über die ganze Datei gezählt stimmen die Zahlen überein:
      `grep -c 'LittleFS\.begin' ESP8266/ESP-uclock/http.cpp` und
      `grep -c 'LittleFS\.end'` liefern denselben Wert (vorher 28 zu 27).
- [ ] **AK14 — Der folgenlose Zwölf-Dateien-Check entfällt für Einzelabrufe.**
      `http_app_installation_complete()` wird nur noch gerufen, wenn `is_pwa_index`
      gesetzt ist. Verhaltensnachweis, dass dabei nichts verlorengeht: `/app/` zeigt
      bei unvollständiger Installation weiterhin die Installationsseite statt einer
      halben PWA. Prüfbar ohne Schaden am Gerät, indem die Seite **mit** vollständiger
      Installation abgerufen wird und `200` plus `index.html`-Inhalt kommt — die
      destruktive Gegenprobe (eine Datei löschen) wird **nicht** gemacht.
- [ ] **AK15 — Ein Abbruch erzeugt keine zweite HTTP-Antwort.** Bricht die
      Auslieferung nach gesendeten Kopfzeilen ab, darf der Aufrufer nicht in seinen
      Fallback-Zweig laufen und eine zweite Antwort in dieselbe Verbindung schreiben.
      Nachweis am Quelltext: Der Rückgabewert unterscheidet „nichts gesendet" von
      „abgebrochen, Kopfzeilen sind raus", und beide Nicht-Null-Fälle überspringen den
      Fallback.
- [ ] **AK16 — Keine Messreste im Fabrikat.** `grep -n 'getFreeContStack'
      ESP8266/ESP-uclock/http.cpp` ist leer, und es ist keine temporäre
      `Serial.print`-Ausgabe aus der Entwicklung übriggeblieben.

### Handwerk

- [ ] **AK17 — `http.cpp` ist nach der Änderung weiterhin UTF-8.**
      `file -I ESP8266/ESP-uclock/http.cpp` meldet `charset=utf-8`, und
      `python3 -c "d=open('ESP8266/ESP-uclock/http.cpp','rb').read(); d.decode('utf-8')"`
      läuft ohne Ausnahme durch. Die Datei hat heute 37 Nicht-ASCII-Bytes
      (`CLAUDE.md`); neue Kommentare dürfen diese Zahl erhöhen, aber kein bestehendes
      Zeichen darf sich verändern. **Wer diese Datei mit einem `latin-1`-Patcher
      anfasst, beschädigt sie** — das ist in diesem Projekt schon passiert, in beide
      Richtungen.
- [ ] **AK18 — `./tools/guardrails.sh` läuft mit Exit 0 durch.**
- [ ] **AK19 — `./tools/guardrails.sh --full` läuft mit Exit 0 durch**, Compile-Smoke
      eingeschlossen.
- [ ] **AK20 — Die Guardrails sind nicht die Abnahme.** Sie sind ausnahmslos statisch
      und sagen über das Laufzeitverhalten dieser Datei nichts. AK1 bis AK8 werden am
      Gerät erhoben, nicht aus dem Quelltext beantwortet (DIR-012).

---

## Nicht Teil dieser Änderung

- **Kein Fix für L125.** Siehe oben. Der Status von L125 bleibt offen; wer ihn ändert,
  ändert ihn auf „Last gesenkt, Ursache offen".
- **Keine PWA-Änderung.** `app.js`, `styles.css`, `index.html`, `sw.js` werden nicht
  angefasst. Ein ESP-Problem wird nicht in der PWA umgangen
  (`knowledge/architecture-checklist.md`, „richtige Schicht").
- **Keine Sprachauslagerung.** B15/L127 ist eine eigene Spec
  (`specs/i18n-auslagerung/`) mit eigener Begründung. Reihenfolge siehe `tasks.md`.
- **`APP_INSTALL_ASSETS` wird nicht angefasst.** Diese Weissliste gehört der
  i18n-Spec. Zwei Umbauten in derselben Datei, verschiedene Funktionen — wer hier
  nebenbei die Liste erweitert, bricht die Trennung.
- **Kein `ip=hb2f` im `ESP_FQBN`.** MSS 1460 ergäbe 89 Frames, braucht aber mehr
  lwIP-Speicher; davon sind rund 12'400 Byte für Heap und Stack zusammen da (L128,
  „nicht empfohlen"). Begründung in `design.md`.
- **Kein neuer Endpunkt.** Zwei Felder in einer bestehenden lesenden Antwort, mehr
  nicht.
- **Keine zusätzliche Ausgabe auf der STM-UART im Normalbetrieb.** Eine Logzeile ist
  ausschliesslich im neuen Fehlerpfad erlaubt (Leseabbruch), nicht je Block und nicht
  je Request.
- **Die übrigen 25 `LittleFS.begin()`-Stellen in `http.cpp` bleiben, wie sie sind.**
  Nur der Auslieferungspfad wird paarig gemacht. Wer alle 28 umbaut, baut eine andere
  Änderung — und riskiert sie in genau der Datei, durch die jeder Request läuft.
- **Kein Umbau auf HTTP/1.1, Keep-Alive oder `Transfer-Encoding: chunked`.**
- **Kein Recovery-Mechanismus und kein pauschaler Watchdog-Umbau.** Für den ESP-Absturz
  wird nichts gebaut, was ihn „abfängt"; dieselbe bewusste Entscheidung wie bei den
  F411-Hängern (`CLAUDE.md`, Punkt 2).
- **C2 wird nicht mitgenommen** (HTTP-Debugzeilen für `/api/` unterdrücken). Eigener
  Befund, eigener Auftrag.
- **Keine Änderung an der Legacy-Oberfläche.**

---

## Betroffene Laufzeiten

- [ ] STM32 (`src/**`) — **nicht berührt**
- [x] ESP8266 (`ESP8266/ESP-uclock/http.cpp`) — **Neu-Flashen nötig**, zweimal
      (Runde A: nur M4; Runde B: der Umbau)
- [ ] PWA (`data/app/**`) — **nicht berührt**, kein LittleFS-Upload nötig
- [x] Build/Release — Release-ZIP und Rollout je Runde, Commit und Tag je Rollout
      (DIR-011)

**Versionsfolge:** Nur `ESP8266/ESP-uclock/version.h` steigt, je Runde einmal. `VERSION`
in `src/main.h`, `APP_VERSION` und `CACHE_NAME` bleiben unverändert — kein Gleichschritt
(DIR-004). Ausgeführt ausschliesslich vom `release-engineer` (R4).

## Rollout-Reihenfolge je Runde

1. `make release-zip` (nie zwei Release-Builds in derselben Minute, `BEFUNDE.md` L2)
2. `./tools/deploy.sh` — danach prüft `check-update-source.sh`, ob das Gerät auf
   dasselbe Verzeichnis schaut (L124)
3. **ESP flashen** — durch den Nutzer
4. `./tools/smoke-device.sh`, `./tools/check-pwa.sh`
5. Messungen der Runde (siehe `tasks.md`)
6. Commit und Tag `release/<stm>-<esp>-<app>` sofort, nicht gesammelt (DIR-011)

Ein `install-app.sh`-Lauf ist **nicht** nötig: Die PWA-Dateien ändern sich nicht, und
ein Firmware-Wechsel löscht das LittleFS nicht. `./tools/install-app.sh --check` läuft
trotzdem mit, weil er die Weissliste gegen den tatsächlichen Bestand prüft und das nach
jedem ESP-Update fällig ist.
