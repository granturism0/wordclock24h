# Anforderungen — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70 Asset-Cache, C72 offen gehaltene Verbindungen)

**Status: Entwurf, zur Freigabe** — vom `spec-writer` am 15.10.2026 erstellt. Ohne Freigabe des Nutzers beginnt
keine Umsetzung. Die offenen Entscheidungen stehen unten als **Ent-1 bis Ent-15**; der Lead legt sie dem Nutzer
einzeln vor.

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`). Am Gerät laut letztem Messprotokoll: STM 3.2.34,
ESP 3.2.37, PWA 1.4.100 (`BEFUNDE.md`, L466). Das ist der Ausgangsstand, **kein** Sollwert; die Versionen der
Releases dieses Pakets setzt der Lead über den `release-engineer` (R4).
**Auslöser:** Nutzerentscheid 15.10.2026 — dieses Paket als nächster Schritt nach Paket 2026-10-13 (E3, E4).
**Grundlage:** `BEFUNDE.md` C65, C70, C71, C72, L456, L457, L460, L461, L466; `specs/paket-2026-10-13/`
(`design-e4.md` §19, `requirements-e4.md`, `endstand.md`); `TESTPLAN-PWA.md` K30 (7.1), Abschnitt 0 Punkt 5, 5b;
`ERWEITERUNGEN.md` X1, X14, X16 (nennen C65/C70 als Voraussetzung).

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen:** **✔ SW** — vom `spec-writer` am Code `72cfda7` bzw. an der Core-Quelle 3.1.2
(`~/Library/Arduino15/packages/esp8266/hardware/esp8266/3.1.2/`) nachgelesen; **✔ B** — in `BEFUNDE.md` belegt;
**◐** — abgeleitet oder Annahme, nicht belegt; **●** — gemeldet, vor der Umsetzung zu prüfen.

**Kennungen dieses Pakets:** Schritte **0** (Grundlagen), **A** (Analyse), **R1** (Release: Messinstrument I-5 und
C70), **G9** (Gerätemessung), **R2** (Release: Eingriff C72, nur nach dem Tor), **G10**, **T3** (`pwa-tester`),
**Z**. Akzeptanzkriterien **AKI** (Instrument I-5), **AKC** (C70), **AKM** (Messung G9), **AKE** (Eingriff C72 und
G10), **AKZ** (übergreifend). Entscheidungen **Ent-1 bis Ent-15**. Eingriffskandidaten **K1 bis K5**,
Messergebnisse nach der Entscheidungsregel **M1 bis M5** (`design.md` §5.4). Prüfpunkte am Core **P1 bis P8**
(`design.md` §11). Die Nummern gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Nachweis |
|---|---|---|---|---|
| **0** | Ausgangswerte, Werkzeuge `ladeprobe` und `browser-sitzung`, Ladeprobe auf dem heutigen Stand | — | — | Messprotokoll |
| **A** | Prüfpunkte P1–P8 am Core und am Code (`firmware-analyst`, lesend) | — | — | Bericht |
| **R1** | **I-5** laufende Verbindungszustände (C72) **und** **C70** `ETag`/`304` (nach Ent-1, Ent-9); gegebenenfalls ein PWA-Teil (Ent-3) | ESP (+ PWA) | Firmware zuerst | **G9** |
| **G9** | Ladeprobe kalt/warm, Update-Probe, Browser-Sitzungen Safari und Chrome, Auswertung, **Tor** | — | — | Messprotokoll, Ent-8 |
| **R2** | **Eingriff C72** nach dem Tor (Kandidat nach Ent-8) — entfällt, wenn das Tor keinen Eingriff ergibt | ESP | — | **G10** |
| **T3** | `pwa-tester` Phasen 0–4 und 9 auf dem Endstand (DIR-012) | — | — | AKZ.7 |

---

## Problem

### 1. Ein volles Neuladen der PWA reisst die Haltegrenze K30 — mit nur einem Client (C65, C70)

**Befund am Gerät, ✔ B (L466, ESP 3.2.37, PWA 1.4.100):** In T2 fiel der Tiefstwert um 21:28:37 auf `low=752`
(Grenze 1'072), `max` 3'584, beim Neuladen der PWA in Englisch (K33) mit **einem** Client. E4 hatte das Tief unter
Last vorher auf 1'336–2'184 gehoben (G8, G7R, L457). Die I-4-Momentaufnahme zu diesem Tief ging verloren, weil V3
die Felder zurücksetzte (L466).

**Ursache der Last, ✔ SW:** Jedes Öffnen und jedes Neuladen holt **alle** App-Assets vollständig vom ESP:

- Die Dateiauslieferung setzt `Cache-Control: no-cache` und `Connection: close`, aber weder `ETag` noch
  `Last-Modified` (`ESP8266/ESP-uclock/http.cpp:2067-2078`). Eine Anfrage mit `If-None-Match` wird nicht
  ausgewertet: Der Server liest von den Kopfzeilen nur `User-Agent` und `Sec-Fetch-Dest`
  (`http.cpp:16783-16802`). Einen `304`-Weg gibt es nicht.
- Über `http://` gibt es keinen Service Worker: `registerAppServiceWorker()` meldet den unsicheren Kontext und
  bricht ab, wenn `navigator.serviceWorker` fehlt (`data/app/app.js:2135-2163`). Der Precache in `sw.js` greift nur
  unter https (`data/app/sw.js:77-101`).
- Umfang je Öffnen: rund 200 KB (C70, ✔ B), davon `app-app.js.gz` 163'621 Byte (Messprotokoll E3, E3.0, ✔ B). Die
  übrigen Grössen sind **nicht** einzeln belegt; Schritt 0 misst sie (AKZ.1).
- Nebenbefund, ✔ SW: `layout-previews.json` wird mit `cache: "no-store"` geholt (`app.js:11655`) — selbst ein
  Validator brächte dort nichts. Die Sprachdatei dagegen trägt schon `?v=<APP_VERSION>` und `cache: "default"`
  (`app.js:1744-1745`).

### 2. Nach einer Safari-Sitzung bleibt der Heap niedrig, bis Safari beendet wird (C72)

**Befund am Gerät, ✔ B (L461):** Nach einer kurzen Safari-Sitzung (19:59:16–20:00:09, ESP 3.2.37) fiel `free` von
6'536 auf 4'832, `max` von 4'672 auf 3'816 und blieb dort bis mindestens 20:06; `http_backlog` stand währenddessen
auf 1. T2 hielt deswegen um 20:05 an. Laut T2-Bericht erholte sich der Wert um 20:09:37 ohne Browser auf
6'536/4'672. **Beobachtung des Nutzers (● — nicht gemessen):** frei wurde der Heap erst, als er Safari **beendete**;
Verlassen der Seite genügte nicht. Beide Angaben stehen nebeneinander in L461; welche für den Mechanismus zählt,
klärt erst G9.

**Was belegt ist, ✔ SW:**

- Die PWA pausiert ihre Abfragen bei verborgener Seite (`app.js:2079-2101`). An ihren Abrufen liegt es nicht.
- Der Server nimmt je Durchlauf **eine** Verbindung an, wartet höchstens `HTTP_FIRST_LINE_TIMEOUT` = 250 ms auf die
  erste Zeile (`http.cpp:239`, `:16660-16679`), zählt eine Verbindung ohne Anfrage als `no_request_timeouts` bzw.
  `no_request_aborts` und beendet sie mit `stop()` (`:16681-16710`). Nach jeder Antwort ebenfalls `stop()`
  (`:16844-16851`). `stop()` endet im Core in `tcp_close()` (`ClientContext.h:73-92`), also in einem regulären
  Abbau mit FIN.
- Eine vom ESP geschlossene Verbindung verbleibt danach in FIN_WAIT_1/FIN_WAIT_2 bis zur Antwort der Gegenstelle,
  dann bis zu 2 × `TCP_MSL` = 120 s in TIME_WAIT (`lwip/priv/tcp_priv.h:137`), dort seit E4 auf 2 begrenzt
  (`http.cpp:464-470`). Für FIN_WAIT_2 sieht lwIP 20 s vor (`tcp_priv.h:131`) — ob die Bibliothek das so übersetzt
  hat, ist **◐** (binär, Prüfpunkt P1).
- Die heutige Messung der Verbindungszustände (I-4) hält sie **nur** beim neuen Block-Tiefstwert fest
  (`http.cpp:473-530`, Aufruf aus `heap_low_note()`), nicht laufend. Für einen Zustand, der Minuten dauert, ohne
  ein neues Tief zu erzeugen, gibt es kein Instrument.

**Welcher Zustand den Heap bindet, ist nicht belegt.** Kandidaten: wartende, nie abgeholte Verbindungen
(ESTABLISHED), vom ESP geschlossene, deren Gegenstelle nicht antwortet (FIN_WAIT_1/FIN_WAIT_2, LAST_ACK), ein
fortlaufender Strom von Vorab-Verbindungen (jede kurz, zusammen dauerhaft), Empfangspuffer — oder gar keine
Verbindung, dann läge es nicht an C72.

### 3. Was ausdrücklich nicht dazugehört

**C71** (Ruhewert 4'672 statt ≥ 5'120 nach dem Startabgleich des Zwischenspeichers, L460) ist eine eigene Ursache
und **nicht** Teil dieses Pakets (Ent-13). Die Analyse hat keinen billigen Weg gezeigt, ihn mitzunehmen: Die
Spur (LittleFS mounten, Temporärdatei, Umbenennen) ist unbelegt, und jede Massnahme dort wäre ein Eingriff ohne
Ursache. C71 betrifft dieses Paket an genau einer Stelle: Die Ruhewert-Grenze aus K30 (5'120) ist ohne C71 nicht
erreichbar; für T3 entscheidet Ent-12.

---

## Ziel

1. **C70:** Ein zweites Öffnen oder Neuladen der PWA in einem Browser, der sie schon geladen hat, holt **keinen
   Asset-Körper** mehr vom ESP, solange sich die Datei nicht geändert hat — jede Asset-Anfrage wird mit `304`
   beantwortet. Eine neu hochgeladene App-Datei wird beim nächsten Laden **sofort** ausgeliefert, ohne
   Wartezeit und ohne dass der Nutzer etwas löschen muss. Der warme Ladevorgang drückt `heap_low_block` nicht unter
   1'072 Byte. **Nicht versprochen:** dass ein **kalter** Erstaufruf (frischer Browser, frisch hochgeladene App)
   K30 hält — er bleibt so teuer wie heute und wird gemessen und berichtet.
2. **C72:** Laufend ablesbar, **welche** Verbindungen der ESP gerade hält (Zustand, Gegenstelle, Ruhezeit), auch
   ohne neuen Tiefstwert. Eine kontrollierte Messung mit Safari und Chrome ordnet den Befund aus L461 einem der
   Ergebnisse M1–M5 zu. **Erst danach** und nur, wenn das Ergebnis einen Kandidaten trägt, folgt ein Eingriff als
   eigenes Release; nach ihm bleibt der Heap nach einer Safari-Sitzung **ohne** Beenden von Safari nicht mehr
   messbar tiefer als vorher (AKE.2).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg der Meldung** (DIR-014). Prüfstände sind **einmal gegen
den Vorgängerstand fehlgeschlagen** (ESP 3.2.37 bzw. PWA 1.4.100), haben **je Massnahme mindestens eine
Sabotage**, die anschlägt, und melden ihre Fallzahl, die gegen das gezählt wird, was es geben müsste (L235). Jede
Messung steht **sofort** im Messprotokoll dieses Pakets in `BEFUNDE.md` (L424). Logzeilen im ESP in Umschrift
(DIR-015).

### Schritt 0 — Grundlagen

- [ ] **AKZ.1 (Ausgangswerte)** — Im Messprotokoll, bevor R1.1 beginnt: Grösse **jeder** der 13 Dateien aus
      `APP_INSTALL_ASSETS` im LittleFS (`/api/fs_list`, Name und Byte, Summe); Liste der `/app/`-Anfragen eines
      Öffnens im Hauptmodul, Deutsch und Englisch, aus der Ladeprobe (Pfad, Status, Byte); „Globale Variablen“
      und Programmgrösse des Stands E4 (Bezug: 69'060 / 531'552 Byte, L457); Kodierung **und** Zeilenende von
      `http.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`, `base.h`, `app.js` (AKZ.4); Verfügbarkeit von `netstat`,
      `lsof`, `osascript` auf dem Mac.
- [ ] **AKZ.2 (Werkzeuge, DIR-014)** — (i) `ladeprobe`: lädt die PWA in einem **dauerhaften** Profil, einmal
      kalt (Profil leer) und danach N-mal warm (Neuladen ohne `ignoreCache` und erneutes Öffnen); setzt vor jedem
      Fenster `device_ready?heap_low_reset=1` und liest danach; gibt je Laden aus: Zahl der `/app/`-Anfragen, je
      Status (`200`/`304`/andere), Summe `encodedDataLength`, `heap_low_block`, `heap_low_path`, die I-4-Felder und
      ab R1 `app_not_modified`/`app_full`. Gegenproben: gegen die Vorschau (`tools/preview/server.py`, liefert kein
      `ETag`) mit `--erwarte-304` ⇒ Exit ≠ 0 und Meldung „warm: 304 erwartet, 200 erhalten“ in der Ausgabe; gegen
      einen Prüfserver, der `304` liefert ⇒ OK. (ii) `browser-sitzung`: fährt die Phasen aus `design.md` §5.3 und
      liest je Phasenende `device_ready`, `netstat -anv -p tcp` und `lsof -nP -i @<uhr>`; Gegenprobe: Uhr nicht
      erreichbar ⇒ Exit ≠ 0 mit Meldung, keine stille Leerzeile. (iii) `check-pwa` gibt zusätzlich je Lauf aus:
      Zahl der `/app/`-Antworten mit `304` und Summe der Byte je Laden (INFO, kein Fehlschlag). (iv)
      `smoke-device.sh` verlangt ab der ESP-Version von R1 die Felder aus AKI.1 und AKC.4, setzt nichts zurück;
      Gegenprobe über `SMOKE_DR_OVERRIDE` (Feld fehlt ⇒ FEHLT; vollständig ⇒ OK). (v) `watch-log.sh` wertet die
      Heap-Zeile mit Zusatz aus AKI.2 unverändert aus (`free=`, `max=`, `ovr=`, `low=`) und führt `- app etag` als
      Hinweis; Gegenprobe über `WATCHLOG_QUELLE` mit einer Zeile `low=752 tcp=…` ⇒ Alarm kommt an. Jede Meldung
      erscheint in Konsole **und** Mitschrift.
- [ ] **AKZ.3 (Ladeprobe auf dem Ausgangsstand)** — Ein Lauf `ladeprobe` (1 kalt, 5 warm) auf ESP 3.2.37 / PWA
      1.4.100, Deutsch, unter dem Tor aus Ent-7. Ins Messprotokoll: je Laden Anfragen, Status, Byte,
      `heap_low_block`. Erwartet (◐): auch warm nur `200`, je Laden rund 200 KB. Das ist der Vergleichswert für
      AKM.2.

### Schritt R1 — Instrument I-5 (C72)

- [ ] **AKI.1 (laufende Verbindungszustände in `device_ready`)** — `/api/device_ready` gibt **hinter**
      `api_orphan_skipped`, dem bisher letzten Feld, aus: `tcp_now_states` (11 Zahlen, Index = lwIP
      `enum tcp_state`, Zählung über `tcp_active_pcbs` **in diesem Augenblick**, die eigene Verbindung
      eingeschlossen), `tcp_now_tw` (Länge von `tcp_tw_pcbs`), `tcp_now_pending` (`WcHttpServer::pending()`),
      `tcp_now_self_port` (Port der Gegenstelle dieser Anfrage), `tcp_now_list` (höchstens 8 Einträge, je
      `[zustand, gegenstelle, port, ruhe_s, nrtx, eigen]`, erst die aktiven, dann TIME_WAIT; `gegenstelle` =
      letztes Oktett der IPv4-Adresse nach Ent-5; `ruhe_s` = Sekunden seit dem letzten Setzen von `pcb->tmr`;
      `eigen` = 1, solange ein `ClientContext` am PCB hängt, 0 nach unserem `close()`), `tcp_now_more` (Zahl der
      nicht gelisteten PCBs). Die Listen werden **vollständig** in einen Stackpuffer gelesen, **bevor** das erste
      `http_send()` läuft; kein Heap, kein `String`, kein `yield()` während des Lesens. Bestehende Felder
      unverändert in Name, Reihenfolge und Typ; nichts davon wird zurückgesetzt. Prüfstand **c72i**; Sabotagen
      „Ausgabe innerhalb der Listenschleife“ und „Kappung auf 8 entfernt“ ⇒ je FEHL; gegen 3.2.37 ⇒ FEHL.
- [ ] **AKI.2 (Heap-Zeile, nach Ent-4)** — Die Heap-Zeile (alle 60 s, `ESP-uclock.ino:601-656`) endet auf
      ` tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>`; Zahlen je Zustand sättigend bei 255.
      Einmal formatiert, in Mitschnitt **und** `/api/stm32_log` textgleich, längste Form ≤ 119 Zeichen. Die
      Zählung liegt in `http.cpp` (privater lwIP-Kopf nur dort, Ent-32 aus Paket 2026-10-13); `ESP-uclock.ino`
      ruft sie über einen Prototyp in `base.h`. Prüfstand **c72i**; Sabotage „nur `Serial`“ ⇒ FEHL. Entfällt mit
      Ent-4 = nein.
- [ ] **AKI.3 (keine Wirkung auf den Betrieb)** — I-5 legt nichts auf den Heap und ändert kein Verhalten:
      Prüfstände c65a, c65b, c65i, c65k, c65o, c65z, c65w bleiben grün; Δ „Globale Variablen“ durch I-5 ≤ 16 Byte
      (gerechnet 0).

### Schritt R1 — C70 Asset-Cache (Variante nach Ent-1; hier für die Empfehlung (a))

- [ ] **AKC.1 (`ETag`)** — Jede `200`-Antwort auf ein Asset aus `APP_INSTALL_ASSETS`, das als `.gz` ausgeliefert
      wird, trägt `ETag: "<grösse>-<crc32>"` (Grösse dezimal, CRC-32 des gespeicherten `.gz`-Inhalts als acht
      Hexziffern, Ent-2). `Content-Type`, `Content-Encoding`, `Content-Length`, `Cache-Control: no-cache` und
      `Connection: close` bleiben unverändert. Andere Antworten (Legacy, `/api/*`, Installationsseite, Assets
      ausserhalb der Weissliste) tragen **kein** `ETag`.
- [ ] **AKC.2 (`If-None-Match` und `304`)** — Der GET-Zweig liest `If-None-Match` (Name ohne Rücksicht auf
      Gross-/Kleinschreibung; `W/`-Präfix und Komma-Liste werden verstanden; `*` gilt als **kein** Treffer).
      Stimmt ein Wert mit dem aktuellen `ETag` überein **und** die Datei besteht mit Grösse > 0 **und** — beim
      Index — die Installation ist vollständig (`http.cpp:2826-2829`), antwortet der ESP `HTTP/1.0 304 Not
      Modified` mit `ETag`, `Cache-Control: no-cache`, `Connection: close` und **ohne** Körper. In jedem anderen
      Fall läuft der heutige Weg unverändert (`200`, Installationsseite oder `404`). Prüfstand **c70e**.
- [ ] **AKC.3 (Invalidierung)** — Der `ETag` wird aus dem **Inhalt** berechnet, nicht aus Version oder Grösse
      allein. Eine RAM-Tabelle hält ihn je Asset; sie ist nach dem Start leer, ein Eintrag gilt nur, solange die
      Grösse der Datei mit der gespeicherten übereinstimmt, und **jede** schreibende LittleFS-Stelle in den
      ESP-Quellen (`open` mit Schreibmodus, `remove`, `rename`, `format`) ruft danach `app_etag_invalidate()`
      — oder steht mit Grund in einer Ausnahmeliste im Prüfstand. Prüfstand **c70e** zählt die Stellen selbst
      (Grep-Bestand am `72cfda7`: 31 in drei Dateien, `design.md` §4.4; vollständig erst mit P6) und meldet jede
      ohne Invalidierung mit Datei:Zeile. Sabotagen: „eine Invalidierung entfernt“ ⇒ FEHL; „`ETag` nur aus der
      Grösse“ ⇒ FEHL (zwei Inhalte gleicher Grösse); „`304` ohne Grössenprüfung“ ⇒ FEHL; „`304` mit Körper“ ⇒
      FEHL; gegen 3.2.37 ⇒ FEHL.
- [ ] **AKC.4 (Kosten und Zähler)** — Die CRC wird nur bei Bedarf berechnet (erste Auslieferung nach Start oder
      Invalidierung), im selben LittleFS-Mount und mit derselben geöffneten Datei wie die Auslieferung, ohne
      Heap und ohne `String`, mit dem vorhandenen statischen Puffer. Je Berechnung genau eine Zeile
      `- app etag <datei> n=<byte> ms=<dauer>` in Mitschnitt und Ring. `device_ready` gibt hinter den Feldern
      aus AKI.1 aus: `app_not_modified` (Zahl der `304`), `app_full` (Zahl der `200` mit Körper auf App-Assets),
      `app_etag_calc` (Zahl der Berechnungen) — seit dem Start, sättigend, nie zurückgesetzt. Die längste
      Berechnung (`app-app.js.gz`) bleibt unter 2'000 ms (◐ geschätzt 0,2–0,5 s, gemessen in G9).
- [ ] **AKC.5 (Invarianten)** — App-Assets weiter **ausschliesslich** als `.gz` mit Prüfung Grösse > 0
      (`http.cpp:2044-2049`); kein Plain-Fallback; `/app/?action=install` und die Installationsseite unverändert;
      die Legacy-Oberfläche liefert byte-gleich wie vor R1 (Stichprobe `/`, `/legacy`, `/fs`, `/update` per
      `curl`, Körper gegen den Stand 3.2.37 verglichen bis auf Zeit- und Messwerte, Kopfzeilen ohne `ETag`).
- [ ] **AKC.6 (PWA-Teil, nur mit Ent-3 = ja)** — `loadLayoutPreviewRowsMap()` holt `layout-previews.json` mit
      `cache: "no-cache"` statt `"no-store"` (`app.js:11655`); sonst keine Änderung an `app.js`; in `sw.js` nur
      `CACHE_NAME`. Prüfstand **c70p** (statisch); Sabotage `"no-store"` ⇒ FEHL; gegen 1.4.100 ⇒ FEHL.
- [ ] **AKC.7 (Variante (b) oder (c), nur falls Ent-1 sie wählt)** — Die Kriterien für versionierte URLs
      (`design.md` §4.3) ersetzen dann AKC.1–AKC.3 in dem Teil, den die Variante betrifft; der `spec-writer`
      legt sie vor R1.3 als Nachtrag vor. Ohne diesen Nachtrag beginnt R1.3 nicht.

### Schritt G9 — Messung

Schreibend ist in G9 nur Probe U (eine App-Datei hochladen und zurücklegen) und, unvermeidlich, was ein
Browser beim Laden selbst schreibt (L462). Beides steht unter **Ent-15**.

- [ ] **AKM.1 (Einspielen R1)** — Nach den Einspielzeilen (AKZ.6): erst ESP-OTA, dann — falls R1 einen PWA-Teil
      hat — `install-app.sh`; danach Smoketest samt Update-Quelle und neuen Feldern **ohne** FEHLT,
      `install-app.sh --check` vollständig, `check-pwa.sh` ohne FEHLT; gemeldete Versionen gleich denen des Tags.
      Die Wache läuft vorher. Zeiten sofort ins Messprotokoll.
- [ ] **AKM.2 (Ladeprobe nach R1)** — Drei Läufe `ladeprobe` (je 1 kalt, 5 warm), einer davon in Englisch. Je
      **warmem** Laden: **jede** `/app/`-Anfrage mit Status `304` (Ausnahme nur `layout-previews.json`, solange
      Ent-3 = nein), Summe `encodedDataLength` über `/app/` ≤ 3'000 Byte zuzüglich der Grösse von
      `layout-previews.json.gz` bei Ent-3 = nein; Δ`app_full` = 0 (bei Ent-3 = nein ≤ 1, eben diese Datei);
      `heap_low_block` im warmen Fenster ≥ 1'072. Je **kaltem** Laden: `heap_low_block`, `low=`, Byte berichtet
      gegen AKZ.3 — **kein** Sollwert (Ziel 1).
- [ ] **AKM.3 (Probe U — Update sofort wirksam)** — Im dauerhaften Profil warm laden; dann **eine** App-Datei mit
      anderem Inhalt hochladen (Kandidat `app/layout-previews.json`, um Leerraum ergänzt und neu gepackt; Weg
      `app_file_upload`); einmal neu laden: genau diese Datei `200` mit **neuem** `ETag` und neuem Inhalt, alle
      anderen `304`; im Mitschnitt genau eine Zeile `- app etag app-layout-previews.json.gz …`. Dann die
      Originaldatei zurücklegen, neu laden: wieder `200` mit dem ursprünglichen `ETag`; danach
      `install-app.sh --check` ohne Abweichung. Alles ins Messprotokoll.
- [ ] **AKM.4 (Browser-Sitzung Safari)** — Phasen S0 bis S4 nach `design.md` §5.3 (Ruhe; Safari öffnet `/app/`,
      120 s sichtbar; Tab verborgen, 180 s; Tab geschlossen, Safari läuft, 300 s; Safari beendet, 180 s), Bedienung
      nach Ent-6. Je Phasenende im Messprotokoll: `free_heap`, `max_free_block`, `tcp_now_*`,
      `no_request_timeouts`, `no_request_aborts`, `app_*`, die Heap-Zeilen mit `tcp=` im Phasenfenster, die
      Mac-Seite (`netstat`-Zustände zur Uhr, Prozess laut `lsof`). Jede eigene Lesung ist eine Verbindung und wird
      in der Liste über `tcp_now_self_port` und ihren TIME_WAIT-Eintrag kenntlich gemacht, nicht weggerechnet.
- [ ] **AKM.5 (Browser-Sitzung Chrome)** — dieselben Phasen mit Chrome, unmittelbar nach AKM.4 oder getrennt
      durch eine Ruhe von mindestens 300 s; Messpunkte wie AKM.4.
- [ ] **AKM.6 (Auswertung und Tor)** — Der Lead ordnet das Ergebnis nach der Entscheidungsregel (`design.md`
      §5.4) **einem** von M1–M5 zu, mit den Messwerten, die die Zuordnung tragen, und schreibt es ins
      Messprotokoll; der `doc-writer` führt C72 nach (nicht gleichzeitig, R3). Erst danach legt der Lead Ent-8,
      Ent-10 und Ent-14 vor. Ein Eingriff ohne zugeordnetes Ergebnis findet nicht statt.
- [ ] **AKM.7 (Gesundheit G9)** — Im ganzen G9-Fenster 0 Exceptions, 0 Neustarts ausser dem OTA, `diag`
      lückenlos, `ovr=0`; `watch-log.sh --pruefe` mit Exit 0 im Bericht; kein `low=` < 1'072 ausser in einem
      kalten Laden unter dem Tor aus Ent-7, dort berichtet.

### Schritt R2 und G10 — Eingriff C72 (nur nach dem Tor)

Entfällt vollständig, wenn das Tor keinen Eingriff ergibt (M4, M5 oder Ent-8 = kein Eingriff); dann stehen
AKE.1–AKE.3 im Endstand als „entfallen“ mit dem Ergebnis aus AKM.6 als Grund.

- [ ] **AKE.1 (Umsetzung des gewählten Kandidaten)** — Der nach Ent-8 gewählte Kandidat ist nach `design.md`
      §6 umgesetzt. Für **K1:** Eine angenommene Verbindung ohne Anfrage wird mit `abort()` (RST) statt
      `stop()` beendet; `no_request_timeouts`/`no_request_aborts` zählen unverändert; jeder andere Abbau bleibt.
      Für **K2:** Höchstens einmal je Sekunde aus `loop()` werden PCBs in `tcp_active_pcbs` mit
      `callback_arg == NULL` (vom ESP geschlossen) in FIN_WAIT_1, FIN_WAIT_2, CLOSING oder LAST_ACK, deren Ruhezeit
      die Frist aus Ent-14 überschreitet, mit `tcp_abort()` abgebaut; PCBs mit angehängtem `ClientContext` werden
      **nie** angefasst; Zähler `tcp_reaped` in `device_ready` (hinter AKC.4, sättigend). Für **K5:**
      `HTTP_TW_LIMIT` 1. Prüfstand **c72e**; Sabotagen (K1: „`stop()` statt `abort()`“; K2: „PCB mit
      `callback_arg` ≠ NULL abgebaut“ und „Frist entfernt“; K5: „Grenze 2“) ⇒ je FEHL; gegen den Stand R1 ⇒ FEHL.
- [ ] **AKE.2 (Wirkung, G10)** — Browser-Sitzung Safari wie AKM.4 auf dem Stand R2: 180 s nach Beginn von S2
      (Tab verborgen) **und** 180 s nach Beginn von S3 (Tab geschlossen, Safari läuft) gilt je
      `free_heap` ≥ `free_heap`(S0) − 300 Byte und in `tcp_now_list` kein PCB mit `eigen` = 0 ausser in TIME_WAIT
      und keiner mit Ruhezeit über der Frist aus Ent-14. Zum Vergleich steht der Befund aus L461 (−1'700) und der
      Wert aus AKM.4 daneben. `max_free_block` berichtet (C71 wirkt dort mit). Chrome wie AKM.5, berichtet.
- [ ] **AKE.3 (keine Regression durch den Eingriff)** — Auf dem Stand R2: drei `check-pwa.sh` ohne FEHLT und
      ohne `net::ERR_CONNECTION_RESET`; Ladeprobe warm wie AKM.2; Δ`write_lost_failed` = 0 über die drei Läufe;
      Smoketest ohne FEHLT; im Mitschnitt keine Exception.

### Übergreifend

- [ ] **AKZ.4 (Kodierung und Zeilenende)** — Vor jedem Patch stellt der Umsetzer Kodierung **und** Zeilenende der
      Datei fest und nennt beides im Taskbericht (`http.cpp` UTF-8 mit einer CRLF-Zeile, `stm32flash.cpp` UTF-8,
      `ESP-uclock.ino` gemischt; CLAUDE.md DIR-015). Der Diff jeder Datei enthält nur inhaltlich geänderte Zeilen;
      der Lead prüft das mit `git diff --stat` gegen die Zahl der beabsichtigten Änderungen.
- [ ] **AKZ.5 (Release-Inhalt)** — R1 ändert nur `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino`, `version.h` und, mit
      Ent-3, `data/app/app.js` und `data/app/sw.js` (`CACHE_NAME`); R2 nur `ESP8266/ESP-uclock/*.cpp`, `*.h`,
      `*.ino`, `version.h`. **Keine** Änderung an `src/**`, `index.html`, `styles.css`, `manifest.webmanifest`.
      Geprüft gegen das jeweils vorige Tag, Ergebnis im Messprotokoll.
- [ ] **AKZ.6 (Releases)** — Je Release ein `make release-zip`, ein Commit, ein Tag nach DIR-011 mit den
      Versionen dieses Commits, Push samt Tag im selben Zug; Einspielzeilen nennen Reihenfolge (Firmware zuerst,
      DIR-002), `install-app.sh --check` nach dem OTA (DIR-017) und den Rückweg samt Befehl, **bevor**
      eingespielt wird (`design.md` §10).
- [ ] **AKZ.7 (T3, DIR-012)** — `pwa-tester` Phasen 0–4 und 9 auf dem Endstand (nach G10, oder nach dem Tor,
      wenn R2 entfällt). Haltegrenze: `low=` < 1'072 ⇒ anhalten (Alarm der Wache); Ruhewert nach Ent-12. Bei jedem
      Halt die I-4- **und** I-5-Felder aus `device_ready`, gelesen ohne Parameter, **bevor** zurückgesetzt wird.
      Die Uhr gehört dem Lauf (`TESTPLAN-PWA.md` Abschnitt 0 Punkt 5); der Lead kündigt Beginn und Ende an.
      Abgenommen mit Bericht und Abdeckungszahl.
- [ ] **AKZ.8 (Guardrails und Prüfstände)** — Nach jedem Task `./tools/guardrails.sh` Exit 0; vor jedem Release
      `--full` Exit 0, Ergebnis im Messprotokoll; die neuen Prüfstände laufen in S16, ihre Fallzahl ist
      nachgezählt (L235).
- [ ] **AKZ.9 (RAM)** — „Globale Variablen“ und Programmgrösse vor R1.1 und nach **jedem** ESP-Task sofort ins
      Messprotokoll; Zuwachs über das ganze Paket höchstens die Grenze aus Ent-11 (empfohlen 250 Byte, gerechnet
      rund 150, `design.md` §9); darüber anhalten und vorlegen. `app.js.gz` vorher und nachher, falls Ent-3 = ja.

**Zahl der Kriterien:** 29 — AKZ 9, AKI 3, AKC 7, AKM 7, AKE 3. Davon hängen an Entscheidungen: AKI.2 (Ent-4),
AKC.6 (Ent-3), AKC.7 (Ent-1), AKE.1–AKE.3 (Tor, Ent-8), AKM.3 (Ent-15).

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **C71** Zerstückelung beim Startabgleich des Zwischenspeichers | eigene Ursache, ohne Beleg (L460); Ent-13 |
| Ein **kalter** Erstaufruf unter 1'072 | C70 ändert ihn nicht; gemessen und berichtet (Ziel 1) |
| **C67** `FS()` ohne Heap, **C66** Teildatei im Download, **C68**, **C69** | eigene Befunde; nicht für die Last eines Ladens geplant |
| Änderungen an `index.html`, `styles.css`, `manifest.webmanifest`, Icons | nicht nötig für Variante (a); Variante (b)/(c) bräuchte `index.html` — dann Nachtrag (AKC.7) |
| Neuer Service Worker, Änderung an `sw.js` ausser `CACHE_NAME` | unter `http://` ohne Wirkung; unter https bleibt er, wie er ist (`design.md` §4.5) |
| `Cache-Control: max-age` ohne Versionierung | verletzt „Update sofort wirksam“ (`design.md` §13) |
| Höchstzahl Verbindungen je Gegenstelle (K4) | Risiko für parallele Asset-Ladungen (`design.md` §6) |
| Ein C72-Eingriff vor dem Tor, auch „vorsorglich“ | Mechanismus nicht belegt; erst messen (`design.md` §13) |
| Änderungen an der Legacy-Oberfläche | Invariante; AKC.5 prüft, dass sie unverändert ausliefert |
| STM-Firmware (`src/**`) | nicht betroffen |
| Browser zu steuern (Safari-Vorabverbindungen abstellen) | nicht möglich; der Eingriff liegt im ESP (C72) |
| Rückfrage-, Bestätigungs- oder Herkunftsschutz für `app_file_upload` | Schutz-Spec (C61) |
| Nachjustieren der Backlog-Schwellen aus E4 | eigener Task mit Begründung aus I-4/I-5, nicht still |
| L462 (Laden schreibt `ambilight_online`, B61) | eigener Befund; hier nur als Nebenwirkung des Ladens benannt (Ent-15) |

---

## Betroffene Laufzeiten

- [ ] **ESP8266** — `http.cpp`, `stm32flash.cpp`, `base.h`, `ESP-uclock.ino` (R1); `http.cpp`, `base.h`,
      gegebenenfalls `ESP-uclock.ino` (R2); je ein OTA
- [ ] **PWA** — nur mit Ent-3: `app.js` (eine Zeile), `sw.js` (`CACHE_NAME`); ein Upload
- [ ] **STM32** — keine
- [ ] **Werkzeug** — neu `ladeprobe`, `browser-sitzung`; geändert `check-pwa`, `smoke-device.sh`, `watch-log.sh`;
      Prüfstände c72i, c70e, gegebenenfalls c70p und c72e
- [ ] **Dokumentation** — `BEFUNDE.md` (Messprotokoll vom Lead, Befunde vom `doc-writer`), `TESTPLAN-PWA.md`,
      `CHANGELOG.md`
- [ ] **Build/Release** — R1 (ESP, mit Ent-3 auch PWA), R2 (ESP, nur nach dem Tor); je Commit, Tag, Push

---

## Entscheidungen

**Nutzer** heisst: der Nutzer entscheidet, der Lead legt einzeln vor. **Lead-Empfehlung** heisst: technischer
Detailentscheid, den der Lead treffen kann; er wird mit der Spec vorgelegt und gilt, wenn der Nutzer nicht
widerspricht. Begründungen in `design.md`.

| Ent | Gegenstand | Optionen | Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-1** | Variante C70 | (a) `ETag` aus dem Inhalt + `304`, `no-cache` bleibt; (b) versionierte URLs mit langem `max-age`, `index.html` ohne Cache; (c) beides | **(a)** — korrekt in jeder Lage, ändert weder `index.html` noch eine Versionsstelle; (c) nur, falls AKM.2 warm unter 1'072 bleibt (`design.md` §4.3) | Nutzer |
| **Ent-2** | Quelle des `ETag` | CRC-32 des Inhalts samt Grösse; Grösse allein; App-Version | **CRC-32 + Grösse** — Grösse allein kollidiert, die App-Version kennt der ESP nicht (`design.md` §4.4) | Lead-Empfehlung |
| **Ent-3** | PWA-Teil in R1 (`layout-previews.json` mit `no-cache`) | ja; nein | **nach Messung:** ja, wenn `layout-previews.json.gz` ≥ 2'048 Byte (AKZ.1); sonst nein, R1 bleibt reines ESP-Release ohne PWA-Upload | Nutzer |
| **Ent-4** | `tcp=` und `nrt=` in der Heap-Zeile | ja; nein | **ja** — Minutenreihe ohne eigene Anfrage, rund 38 Byte je Minute mehr auf der UART | Lead-Empfehlung |
| **Ent-5** | Gegenstelle in `tcp_now_list` | letztes Oktett; volle Adresse | **letztes Oktett** — genügt im eigenen /24, und Messwerte wandern ins öffentliche `BEFUNDE.md` | Lead-Empfehlung |
| **Ent-6** | Wer bedient die Browser in G9/G10, auf welchem Gerät | (A) Lead per `open`/`osascript` auf dem Mac; (B) Nutzer auf Zuruf auf seinem Gerät aus L461; (C) A, dazu einmal B | **C** — A ist wiederholbar und zeitlich genau; B prüft, ob das Bild aus L461 auf dem Gerät des Nutzers dasselbe ist | Nutzer |
| **Ent-7** | Haltegrenze in der Ladeprobe (AKZ.3, AKM.2) | wie Ent-31 aus Paket 2026-10-13: erstes Laden messen, dann Tor; oder kein kaltes Laden | **erstes Laden messen, dann Tor:** `low=` < 1'072 in einem **warmen** Laden ⇒ Ende der Probe, Nutzer fragen; ein kaltes Laden darunter wird berichtet, nicht wiederholt | Nutzer |
| **Ent-8** | Eingriff C72 (am Tor nach G9) | K1, K2, K1 + K2, K5 als Zusatz, kein Eingriff | **nach Ergebnis** (`design.md` §5.4): M1 ⇒ K2; M2 ⇒ K1; M3 ⇒ K1 + K2; M4/M5 ⇒ kein Eingriff | Nutzer |
| **Ent-9** | Releaseschnitt | (A) drei Releases: I-5, C70, Eingriff; (B) zwei: I-5 + C70, Eingriff | **B** — ein OTA weniger, das Du einspielst; C70 ist kein C72-Eingriff, und sein Vorher-Wert kommt aus AKZ.3 | Nutzer |
| **Ent-10** | Schreibender Zugriff auf lwIP-Listen (für K2) | erlauben; nicht erlauben | erweitert Ent-32 aus Paket 2026-10-13 (dort „nur lesend“); **erlauben, nur mit K2 und nur für PCBs ohne `ClientContext`** | Nutzer, am Tor |
| **Ent-11** | RAM-Grenze des Pakets | — | **250 Byte** „Globale Variablen“ (gerechnet rund 150) | Lead-Empfehlung |
| **Ent-12** | Ruhewert-Grenze in T3 | 5'120 (K30); 4'096 (wie T2, L460) | **4'096**, solange C71 offen ist; der Wert wird berichtet | Nutzer |
| **Ent-13** | C71 in diesem Paket | ja; nein | **nein** — ohne Ursache kein billiger Weg (Problem §3) | Nutzer |
| **Ent-14** | Frist in K2 | 5 s; 20 s (lwIP-Vorgabe für FIN_WAIT_2) | **5 s** für FIN_WAIT_1/FIN_WAIT_2/CLOSING/LAST_ACK nach unserem `close()`; endgültig am Tor mit den Ruhezeiten aus G9 | Nutzer, am Tor |
| **Ent-15** | Schreibende Schritte in G9/G10 | Probe U (AKM.3) und Laden in Safari (L462: schreibt `ambilight_online`) ohne Einzelfreigabe | Der Auftrag an den `spec-writer` nennt die Uhr eine Test-Uhr, auf der schreibende Messungen ohne Einzelfreigabe erlaubt seien. **CLAUDE.md R5 sagt anderes** (Freigabe im selben Gespräch). Bitte mit der Spec ausdrücklich bestätigen; ohne Bestätigung entfällt Probe U, und AKM.3 bleibt offen | Nutzer |
