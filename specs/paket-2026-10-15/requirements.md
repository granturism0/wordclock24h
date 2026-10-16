# Anforderungen — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70 Asset-Cache, C71 gebundener Heap nach Existenzprüfungen, C72 offen gehaltene Verbindungen)

**Status: freigegeben am 15.10.2026 bis aufs Tor; Nachträge A.2 bis A.5 eingearbeitet und umgesetzt (R1, R1b, R2
eingespielt, L468); Entscheide Ent-19 bis Ent-23 gefallen, A.6 freigegeben (16.10.2026).** **Nachtrag A.6** (16.10.2026,
nach dem Tor G10.6/G10.7, Stand `2d97259`): Du hast entschieden, dass C70 mit **Variante (c)** weitergeht — zum
`ETag`/`304` aus R2 kommen versionierte URLs mit langem `max-age`, damit ein unverändertes Asset beim warmen Laden gar
nicht mehr nachgefragt wird. Dafür gibt es ein eigenes Release **R2b** mit der Messung **G10b**, beide vor T3. Ent-8
lautet „kein Eingriff“; damit entfallen Ent-10, Ent-14, **R3 und G11**. **Entschieden am 16.10.2026:** Ent-19 **C**
(`?v=<APP_VERSION>&h=<marke>`), Ent-20 **B** (`layout-previews.json` bleibt unversioniert), Ent-21 **E2**, Ent-22 **ja**
(Manifest versioniert), Ent-23 **1 Jahr**. Was sich ändert, steht in `design.md` §18 und §17.

*Stand A.5, überholt durch die Entscheide vom 16.10.2026 (L468):* Die Ursache von C71 ist am Gerät belegt (`BEFUNDE.md`
L468, Commit `89ec34c`) — die Massnahme L7 steht damit fest, die bisherigen C71-Hypothesen sind widerlegt. Was sich
ändert und was widerlegt ist, steht in `design.md` §17. Offen waren damals Ent-2, Ent-18, Ent-8, Ent-10, Ent-14, Ent-17
und Ent-16; entschieden sind sie unter „Entscheidungen“.

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge am 15. und 16.10.2026, A.6 am Stand
`2d97259`. Ausgangsstand am Gerät vor R1: STM 3.2.34, ESP 3.2.37, PWA 1.4.100 (`BEFUNDE.md`, L466) — **kein**
Sollwert; die Versionen der Releases setzt der Lead über den `release-engineer` (R4).
**Auslöser:** Nutzerentscheid 15.10.2026 — dieses Paket als nächster Schritt nach Paket 2026-10-13 (E3, E4); für A.6
der Nutzerentscheid G10.7 vom 16.10.2026.
**Grundlage:** `BEFUNDE.md` C65, C70, C71, C72, L456, L457, L460, L461, L466, L468; `specs/paket-2026-10-13/`;
`TESTPLAN-PWA.md` K30 (7.1), Abschnitt 0 Punkt 5, 5b; `ERWEITERUNGEN.md` X1, X14, X16; `CLAUDE.md` R5/DIR-008 in der
Fassung vom 14.10.2026 (`CLAUDE.md:412-416`); Berichte des `firmware-analyst` (A.1), Review R1.7, Messungen des Leads
(L468).

**Zeilenangaben:** `Datei:Zeile`-Anker beziehen sich auf `72cfda7`, die aus A.6 auf `2d97259`. In `http.cpp` liegen sie
seit R1 ab rund `:2839` um rund +210 später (Lead). **Massgeblich ist die genannte Funktion bzw. das Symbol.**

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen:** **✔ SW** — vom `spec-writer` am Code `72cfda7` (in A.6 `2d97259`) bzw. an der Core-Quelle 3.1.2
nachgelesen; **✔ B** — in `BEFUNDE.md` belegt; **✔ L** — vom Lead gemessen; **✔ FA** / **◐ FA** — vom `firmware-analyst`
belegt bzw. als plausibel gemeldet; **? FA** — Spekulation; **✔ R** / **◐ R** — vom `code-reviewer` (R1.7) belegt bzw.
als Lesart gemeldet; **◐** — abgeleitet; **●** — gemeldet, vor der Umsetzung zu prüfen.

**Kennungen dieses Pakets:** Schritte **0**, **A**, **R1** (Instrumente), **G9** (ohne Cache), **R1b** (L7, nur mit
Ent-18 A), **G9b**, **R2** (C70), **G10** (mit Cache, **Tor**), **R2b** (C70 Variante (c), A.6), **G10b**, **R3**
(Eingriff C72; mit Ent-18 B auch L7 — **entfallen**, Ent-8), **G11** (**entfallen**), **T3**, **Z**. Akzeptanzkriterien
**AKZ**, **AKI**, **AKM**, **AKS**, **AKC**, **AKW**, **AKE**, **AKL** (R2b), **AKG** (G10b). Entscheidungen **Ent-1 bis
Ent-23**. `ETag`-Varianten **T**, **V**. **Inhaltsmarke** (A.6): der `ETag`-Wert nach Variante T ohne Anführungszeichen,
als URL-Parameter `h`. Abläufe **U1a**, **U1b**, **U2**, **U3** der Probe U (A.6). Kandidaten C72 **K1–K5**; Massnahme C71
**L7** (L7a, L7b, L7c); Ergebnisse C72 **M1–M5**, C71 **N5** (N0–N4 widerlegt bzw. gegenstandslos); **Messung M1**
(Freigabe durch einen kalten `update_table_files`) und **Messung M3** (Kopf und Rumpf der Antwort) des Leads — nicht zu
verwechseln mit dem C72-Ergebnis M1; Lochsumme **`L = free − blk`**; **P1-Probe**; Prüfpunkte **P1–P12**. Die Nummern
gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Nachweis |
|---|---|---|---|---|
| **0** | Ausgangswerte, Werkzeuge; Trennmessung C71 **erledigt** (L468) | — | — | Messprotokoll |
| **A** | P1–P12 erledigt bzw. durch A.5 beantwortet | — | — | Bericht |
| **R1** | **I-5** laufende Verbindungszustände; Heap-Zeile mit `tcp=`/`nrt=`; **I-6** Startzustand und Heap im Startabgleich | ESP | ESP-OTA | **G9** |
| **G9** | **Ausgangsstand ohne Cache:** Ladeprobe, P1-Probe, Browser-Sitzungen (mit Ent-18 A erst auf R1b), **Startmessung C71** (gefahren, Ergebnis N5) | — | — | Messprotokoll |
| **R1b** *(Ent-18 A)* | **L7** — Behebung von C71: Kontext nach jedem Abruf freigeben, Fehlerwege, C75 | ESP | ESP-OTA | **G9b**: AKS.1–AKS.4, neue Ladeprobe-Grundlinie, Browser-Sitzungen |
| **R2** | **C70** `ETag`/`304` (Ent-2), Statuszeile `HTTP/1.1` für App-Assets, PWA-Teil | ESP **und** PWA | Firmware zuerst, dann PWA | **G10** |
| **G10** | **Mit Cache:** Ladeprobe, Probe U, Browser-Sitzungen, Grundlinie der Verbindungsfehler; Zuordnung M1–M5; **Tor** — gefahren 16.10.2026 (L468) | — | — | Ent-8 (kein Eingriff), G10.7 ((c)) |
| **R2b** *(A.6)* | **C70 Variante (c):** versionierte URLs `?v=…&h=…` mit `immutable`, nur wenn die Inhaltsmarke zum `ETag` passt; `layout-previews.json` bleibt bei (a) (Ent-20 B); Build erzeugt das Dokument; PWA-Teil | ESP **und** PWA **und** Build | Firmware zuerst, dann PWA | **G10b** |
| **G10b** *(A.6)* | **Mit Langzeit-Cache:** Ladeprobe, Probe U1a/U1b/U2/U3, Safari auf Mac und iPhone, Gesundheit | — | — | AKG.1–AKG.7 |
| **R3** | ~~**Eingriff C72** (Ent-8); mit Ent-18 B auch L7~~ — **entfallen** (Ent-8 = kein Eingriff, 16.10.2026) | — | — | — |
| **T3** | `pwa-tester` Phasen 0–4 und 9 auf dem Endstand (DIR-012) | — | — | AKZ.6 |

---

## Problem

### 1. Ein volles Neuladen der PWA reisst die Haltegrenze K30 — mit nur einem Client (C65, C70)

**Befund am Gerät, ✔ B (L466, ESP 3.2.37, PWA 1.4.100):** In T2 fiel der Tiefstwert um 21:28:37 auf `low=752`
(Grenze 1'072), `max` 3'584, beim Neuladen der PWA in Englisch (K33) mit **einem** Client.

**Ursache der Last, ✔ SW:** Jedes Öffnen und Neuladen holt **alle** App-Assets vollständig vom ESP:

- `http_send_fs_file()` antwortet `HTTP/1.0 200 OK` mit `Cache-Control: no-cache` und `Connection: close`, ohne `ETag`
  oder `Last-Modified` (✔ L per `curl -D`). `If-None-Match` wird in `http_server_loop()` nicht ausgewertet.
- Über `http://` gibt es keinen Service Worker.
- Umfang je Öffnen rund 200 KB (C70, ✔ B), davon `app-app.js.gz` 163'621 Byte (E3.0, ✔ B).
- `layout-previews.json` wird in `loadLayoutPreviewRowsMap()` mit `cache: "no-store"` geholt.

**Befund am Prüfserver, ✔ L (Lead, 15.10.2026 22:21–22:23):** Mit `HTTP/1.0` sandte Chrome für das Dokument kein
`If-None-Match`, für `app.js` nur beim Neuladen. Mit `HTTP/1.1`: jede warme Ladung mit `If-None-Match`, überall `304`,
290 statt 4'045 Byte. **Safari ist ungeprüft.** **✔ FA:** Nur `http_send_fs_file()` muss die Statuszeile ändern; die
übrigen Antworten in `http_app()` bleiben `HTTP/1.0`; Keep-Alive entsteht nicht.

### 2. Nach einem kalten `update_status` bleiben 1'704 Byte gebunden (C71) — Ursache belegt

**Befund am Gerät, ✔ B (L460, L457):** Nach dem OTA auf E4 fiel `max` von 7'184 auf 4'672 und blieb dort; nach G7R
4'464. K30 verlangt ≥ 5'120.

**Ursache, ✔ L (L468, 16.10.2026, `89ec34c`):**

- `http_api_update_status()` prüft am Ende per `GET` die Existenz von `wc12h-icon.txt` und `wc12h-weather.txt` und ruft
  bei `len > 0` sofort `httpclient_stop()`, **ohne den Rumpf zu lesen**.
- `WiFiClient::stop()` ⇒ `ClientContext::close()` gibt den Empfangspuffer `_rx_buf` nicht frei; frei wird er erst bei
  Refcount 0 (`unref()` ⇒ `discard_received()`), also beim **nächsten** `connect()` des statischen httpclient (✔ SW:
  `ClientContext.h:73-92`, `:115-127`; `httpclient.cpp:16`).
- **Messung M3:** Kopf 250, `Content-Length` 1215, Rumpf gleich Repo — drei pbufs 616 + 616 + 472 = **1'704 Byte**.
- **Messung M1:** Ein kalter `update_table_files`, der zu Ende liest und neu verbindet, gibt **exakt +1'704** frei
  (5'624/4'744 ⇒ 7'328/7'152).
- **G9.6:** Start 3 (Schreiben beim Start) zerstückelt **nicht**; Start 4 (ein `update_status`) belegt die 1'704; ein
  zweiter `update_status` belegt nichts zusätzlich.
- Dasselbe Muster gilt bei der Prüfung auf `app.js.gz` (bis rund 2'464 Byte) und im Zweig `zu-gross` von
  `update_cache_write()`. Die direkten `client.stop()` in den Fehlerwegen von `httpclient()` (`httpclient.cpp:216`,
  `:224`) lassen einen Fehlerrumpf (etwa 404) ebenso liegen. **C75** (✔ R): Bei `len == 0` kehrt `httpclient()` mit
  offenem Client zurück, und Aufrufer, die bei `len <= 0` aufgeben, schliessen nicht.

**Widerlegt:** die Platzierungshypothese H-CC als Ursache, „Schreiben beim Start zerstückelt“ (N1a/N1b), der
PUYA-Puffer (N0), die Vermutung „der Zwischenspeicher hält einen String“. **Offen (◐):** L460 sah den Fall laut
Protokoll ohne Anfrage im Startfenster; der Lead prüft den damaligen Mitschnitt auf einen Abruf ohne Rumpflesen.

### 3. Nach einer Safari-Sitzung bleibt der Heap niedrig (C72) — die 1'704 Byte gehören zu C71

**Befund am Gerät, ✔ B (L461):** Nach einer kurzen Safari-Sitzung fiel `free` von 6'536 auf 4'832, `max` von 4'672 auf
3'816. **Die Lücke betrug genau 1'704 Byte (✔ L, L468) — sie ist die Belegung aus C71**: Die PWA fragt beim Öffnen
`update_status` ab. **Für C72 bleiben** der Anteil an `max`, soweit C71 ihn nicht erklärt, und die Erholung um +600 um
20:07. Ob dahinter Verbindungszustände stehen, entscheiden die Browser-Sitzungen; K1 und K2 bleiben bis dahin
Kandidaten.

**Was belegt ist:** Die PWA pausiert bei verborgener Seite (✔ SW). Verbindungen ohne Anfrage werden mit `stop()`
abgebaut; K1 wirkt nur im Teilfall `no_request_timeouts` (✔ FA P7). FIN_WAIT_2 läuft rund 20 s nach dem letzten Segment
ohne RST ab (◐ FA P1); TIME_WAIT endet über `tmr`, spätestens rund 120 s nach dem letzten Segment vor dem Schliessen
(◐ R H4). `MEMP_NUM_TCP_PCB` begrenzt nichts (✔ FA P2).

**Stand nach G9b und G10 (✔ B, L468):** M5 auf R1b und auf R2 — in keiner gültigen Sitzung eine Lücke ≥ 500 nach Abzug
von C71, keine gehaltene Verbindung. Ent-8: kein Eingriff.

### 4. Nach (a) kostet jedes warme Laden noch 6–8 Verbindungen (C70, C65 — Nachtrag A.6)

**Befund am Gerät, ✔ B (L468, G10.1 auf R2, ESP 3.2.40, PWA 1.4.101):** Jedes warme Laden schickt auf **jeder**
`/app/`-Anfrage `If-None-Match` und bekommt `304`; über die Leitung gehen **626–835 Byte** je Laden statt 200'724–223'176
(G9b.3). Es bleiben aber **6–8 Anfragen** je Laden: Dokument, `styles.css`, Manifest (nur beim Neuladen), `app.js`, zwei
Icons, `layout-previews.json`, in Englisch die Sprachdatei.

**Was das kostet, ✔ SW / ◐:** Jede `304` ist eine eigene TCP-Verbindung (`Connection: close`, AKC.7) mit `accept()`,
`ClientContext` und PCB auf dem ESP, danach TIME_WAIT. Heap und Verbindungen sind der knappe Punkt (C65). Das Byte-Volumen
ist mit (a) gelöst, die **Zahl der Verbindungen** nicht.

**Warum `max-age` allein nicht genügt, ✔ SW (`design.md` §18.3):** Der ESP kennt die installierte App-Version nicht;
`install-app.sh` lädt `index.html` **zuerst** hoch (`tools/install-app.sh:38-42`); Probe U ersetzt eine Datei bei
gleicher Version. In allen drei Fällen läge unter einer URL, die der Browser ein Jahr lang nicht mehr nachfragt, ein
anderer Inhalt als der, den sie bezeichnet. Ein ESP-Rückweg heilt einen solchen Eintrag nicht.

---

## Ziel

1. **C70:** Ein zweites Öffnen oder Neuladen in einem Browser, der die PWA schon geladen hat, holt **keinen
   Asset-Körper** mehr vom ESP, solange sich die Datei nicht geändert hat — in Chrome belegt, in Safari gemessen und
   berichtet; eine neu hochgeladene App-Datei wird beim nächsten Laden **sofort** ausgeliefert. Der warme Ladevorgang
   drückt `heap_low_block` nicht unter 1'072 Byte. **Nicht versprochen:** dass ein **kalter** Erstaufruf K30 hält.
   **Erreicht auf R2 (✔ B, L468, AKW.2–AKW.4).**
   - **1b. C70 Variante (c), A.6:** Ein warmes Öffnen oder Neuladen schickt in Chrome **nur noch das Dokument und
     `layout-previews.json`** über die Leitung, beide mit `304`; jedes versionierte Asset kommt aus dem Cache des
     Browsers, ohne Nachfrage. In Safari auf dem Mac und auf dem iPhone gemessen und berichtet. **Ein
     Langzeit-Cache-Eintrag ist nie falsch:** Unter einer URL mit `immutable` liegt immer genau der Inhalt, den ihre
     Inhaltsmarke bezeichnet — auch während eines Uploads, bei gleicher Version und nach Probe U. **Ein neuer App-Stand**
     (Dokument und Dateien, über `install-app.sh` oder die Auto-Installation) ist beim nächsten Laden wirksam. **Für
     `layout-previews.json` gilt Ziel 1 unverändert** (Ent-20 B): Sie bleibt unversioniert, eine ersetzte Datei kommt
     beim nächsten Laden sofort an. **Für eine versionierte Datei** gilt sinngemäss Ent-20 A: Wird sie einzeln ersetzt,
     ohne dass ein neues Dokument sie nennt, sieht ein Browser mit Cache-Eintrag sie erst mit dem nächsten Dokument; ein
     Browser ohne Eintrag sofort. **Rückweg** für jeden Eintrag: ein PWA-Release mit neuer `APP_VERSION` (Ent-19 C).
2. **C71:** Kein Abruf des httpclient hinterlässt eine gebundene Antwort — nach einem kalten `update_status` liegen
   `free` und `max` 130 s später höchstens 300 Byte unter dem Stand davor; der Ruhewert `max` liegt nach jedem Start und
   nach Last bei mindestens 5'120 Byte (K30).
3. **C72:** Laufend ablesbar, **welche** Verbindungen der ESP hält. Browser-Sitzungen **ohne** C71-Belegung und die
   P1-Probe ordnen den verbleibenden Befund aus L461 einem von M1–M5 zu. Erst danach und nur mit tragendem Ergebnis ein
   Eingriff; nach ihm bleibt der Heap nach einer Safari-Sitzung **ohne** Beenden von Safari nicht mehr messbar tiefer
   als vorher (AKE.2). **Ergebnis M5, kein Eingriff (Ent-8, 16.10.2026).**

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg der Meldung** (DIR-014). Prüfstände sind **einmal gegen
den Vorgängerstand fehlgeschlagen**, haben **je Massnahme mindestens eine Sabotage** und melden ihre Fallzahl (L235).
Jede Messung steht **sofort** im Messprotokoll (L424). Logzeilen im ESP in Umschrift (DIR-015).

**Haltegrenzen in den Messungen (Ent-7):** In G9, G9b, G10, G10b und G11 **keine** — alle Ladungen werden gefahren und
protokolliert. Ein Neustart, eine Exception oder ein Watchdog-Reset ist ein **Ergebnis**: mit Uhrzeit, letzter
Anfrage, letzter Heap-Zeile und den I-4-/I-5-Feldern ins Messprotokoll, als Befund nachgeführt, dann Update-Quelle
und Versionen geprüft (DIR-009, L42).

**Vermerke zu jedem Ruhe- und Tiefstwert in G9, G9b, G10, G10b und G11:** Flash-Schreiben seit dem Start / beim Start
(Belege `- update cache … geschrieben`, `- update cache tmp geloescht …`, App-Upload; sonst `nein` oder `unbekannt`,
P12); Wetter-Overlay im Startfenster ja/nein; **ob seit dem Start ein kalter `update_status` lief und ob der httpclient
danach neu verbunden hat (`hc`, I-6)** — solange L7 nicht eingespielt ist, bindet ein kalter `update_status` 1'704 Byte.

**Status `304` in CDP-Werkzeugen:** `Network.responseReceived` meldet bei einer `304`-Nachprüfung `200`; den
Leitungsstatus liefert nur `Network.responseReceivedExtraInfo.statusCode` (✔ L). Jedes Kriterium, das `304` zählt,
meint den Leitungsstatus. **Ab A.6:** Eine Antwort **aus dem Cache** ist eine ohne `responseReceivedExtraInfo`, die CDP
als aus dem Speicher- bzw. Plattencache bedient meldet; daneben zählen die Gerätezähler (`app_full`, `app_not_modified`,
`app_immutable`, `app_token_miss`) unabhängig, was über die Leitung ging.

### Schritt 0 — Grundlagen

- [ ] **AKZ.1 (Ausgangswerte ohne Last)** — Im Messprotokoll, bevor R1.1 beginnt: Grösse **jeder** der 13 Dateien
      aus `APP_INSTALL_ASSETS` (`/api/fs_list`); „Globale Variablen“ und Programmgrösse des Stands E4 (69'060 / 531'552
      Byte, L457); Kodierung **und** Zeilenende von `http.cpp`, `httpclient.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`,
      `base.h`, `app.js`; `netstat`, `lsof`, `osascript` vorhanden; der Prüfserver-Befund zu `HTTP/1.0`/`HTTP/1.1`; die
      Ergebnisse von A.1.
- [ ] **AKZ.2 (Werkzeuge, DIR-014)** — (i) `ladeprobe`: dauerhaftes Profil, 1 × kalt, N × warm (Neuladen ohne
      `ignoreCache` und erneutes Öffnen); `device_ready?heap_low_reset=1` vor jedem Fenster, danach lesen; je Laden
      Zahl der `/app/`-Anfragen nach **Leitungsstatus**, ob `If-None-Match` gesendet wurde, Summe
      `encodedDataLength`, `heap_low_block`, `heap_low_path`, I-4-Felder, `app_not_modified`/`app_full`, soweit
      vorhanden. Gegenproben: Vorschau mit `--erwarte-304` ⇒ Exit ≠ 0 und Meldung „warm: 304 erwartet, 200
      erhalten“; Prüfserver `HTTP/1.1` mit `ETag` ⇒ OK; Prüfserver `HTTP/1.0` ⇒ FEHL. (ii) `browser-sitzung`: Phasen
      nach `design.md` §5.3 in den Betriebsarten `osascript` und **`manuell`**; je Phasenende `device_ready`,
      `netstat -anv -p tcp`, `lsof -nP -i @<uhr>`; für **jede eigene Lesung** lokaler Port und Ende ihres
      TIME_WAIT-Fensters; je Sitzung Δ`no_request_timeouts`, Δ`no_request_aborts`, Δ`http_accept_dead`; **solange L7
      nicht eingespielt ist: vor S0 Freistellen des Clients und je Phase `hc` und `update_info_cached`**; Gegenprobe:
      Uhr nicht erreichbar ⇒ Exit ≠ 0 mit Meldung. (iii) ab R1: `smoke-device.sh` verlangt die Felder aus AKI.1, ab R2
      die aus AKC.4, ab R2b die aus AKL.2; setzt nichts zurück; Gegenprobe `SMOKE_DR_OVERRIDE`. (iv) ab R1:
      `watch-log.sh` wertet die Heap-Zeile mit Zusatz unverändert aus und führt die I-6-Zeilen als Hinweis; Gegenprobe
      über `WATCHLOG_QUELLE` (`low=752 tcp=…` ⇒ Alarm kommt an). (v) ab R2: `check-pwa` INFO „`/app/` 304 je Laden“
      (Leitungsstatus), „Byte je Laden“ und je Lauf die Zahl der `net::ERR_CONNECTION_RESET` und
      `net::ERR_EMPTY_RESPONSE`; mit V `- app etag` als Hinweis. (vi) Werkzeuge und Prüfstände, die eine CRC des ESP
      nachrechnen (nur V), benutzen **CRC-32/MPEG-2**. (vii) Auswertung der Lochsumme `L = free − blk` und **Messung
      AKS.4** (kalter `update_status`, Heap-Zeilen bis ≥ 130 s mit `tw=0`, `free`/`max` vorher und nachher). (viii) ab
      R2b die Werkzeuge aus AKL.8. Jede Meldung erscheint in Konsole **und** Mitschrift.
- [x] **AKZ.9 (Trennmessung C71 auf 3.2.37)** — Erstes Flash-Schreiben seit dem Start ohne Abruf (b) und Schreiben mit
      Abruf (c), je mit Heap-Zeilen vorher und nachher, im laufenden Betrieb. **Erfüllt, Beleg `BEFUNDE.md` L468**
      (Lead, 15.10.2026 22:40–22:46): beide ohne bleibende Spur.

### Schritt R1 — Instrumente

- [ ] **AKI.1 (I-5: laufende Verbindungszustände in `device_ready`)** — In `http_api_device_ready()` hinter
      `api_orphan_skipped`: `tcp_now_states` (11 Zahlen, Index = lwIP `enum tcp_state`, Zählung über
      `tcp_active_pcbs` in diesem Augenblick, die eigene Verbindung eingeschlossen), `tcp_now_tw`, `tcp_now_pending`,
      **`tcp_now_unclaimed`**, `tcp_now_self_port`, `tcp_now_list` (höchstens 8 Einträge `[zustand, gegenstelle, port,
      ruhe_s, nrtx, arg]`, erst aktive, dann TIME_WAIT; `gegenstelle` = letztes Oktett; `ruhe_s` = Sekunden seit dem
      letzten Setzen von `pcb->tmr`; `arg` = 0 ohne `callback_arg`, 1 `ClientContext`, 2 Listener), `tcp_now_more`;
      dazu der Zähler **`http_accept_dead`** (nur gezählt, wenn vor `accept()` ein Eintrag in `_unclaimed` stand).
      Listen **vollständig** in einen Stackpuffer, **bevor** das erste `http_send()` läuft; kein Heap, kein `String`,
      kein `yield()`. Bestehende Felder unverändert; nichts wird zurückgesetzt. Der Kommentar am I-4-Block wird nach P2
      berichtigt. Prüfstand **c72i**; Sabotagen „Ausgabe innerhalb der Listenschleife“, „Kappung entfernt“, „SYN_RCVD als
      `ClientContext` gezählt“, „toter Abholversuch ohne Zähler“ ⇒ je FEHL; gegen 3.2.37 ⇒ FEHL.
- [ ] **AKI.2 (I-5: Heap-Zeile, Ent-4)** — Die Heap-Zeile aus `esp_heap_log()` endet auf
      ` tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>`; einmal formatiert, Mitschnitt **und**
      `/api/stm32_log` textgleich, ≤ 119 Zeichen. Prüfstand **c72i**; Sabotage „nur `Serial`“ ⇒ FEHL.
- [ ] **AKI.3 (I-6: Startzustand, Heap im Startabgleich, Flash-Kennung)** — Je Durchgang von `update_cache_poll()`:
      (a) vor Schritt 1 — **nach** dem Löschen liegengebliebener `.tmp` — `- update cache vorher up=<s> free=<n>/<blk>
      frag=<n> tcp=<…> hc=<httpclient_connects>`; (b) am Ende (auch bei Abbruch) `- update cache heap v=<free>/<blk>
      1=<free>/<blk> 2=… 3=… 4=… 5=… n=<free>/<blk>` (`-` für nicht gelaufen); (c) unmittelbar danach
      `- update cache nach frag=<v>/<1>/<2>/<3>/<4>/<5>/<n> tcp=<…> hc=<n> acc=<Δ>` — `frag` aus `ESP.getHeapStats()`
      (nur ergänzend, geringe Empfindlichkeit), `acc` = Δ eines neuen Zählers angenommener Verbindungen; `tcp=`/`hc`
      hier rund 10 ms nach `n` (H2). Dazu (d) einmal je Start beim ersten **lebenden** `accept()` — auch bei einer
      Vorrats-Verbindung — `- heap erste anfrage free=<n> max=<n>`; (e) einmal in `setup()`, vor WLAN und
      `EEPROM.begin()`, `- flash id=<hex6> vendor=<hex2>`; (f) wenn der erste Durchgang eine liegengebliebene
      Temporärdatei löscht, `- update cache tmp geloescht <datei>`. Je Zeile ≤ 119 Zeichen, einmal formatiert,
      Mitschnitt **und** Ring textgleich, kein `String`. Prüfstand **c71i**; Sabotagen „`vorher` nach Schritt 1“, „ein
      Schritt ohne Messwert“, „nur `blk` statt `free`/`blk`“, „`nach` ohne `hc`“, „`acc` nicht als Δ“, „`tmp geloescht`
      fehlt“, „nur `Serial`“ ⇒ je FEHL; gegen 3.2.37 ⇒ FEHL.
- [ ] **AKI.4 (keine Wirkung auf den Betrieb)** — I-5 und I-6 legen nichts auf den Heap und ändern kein Verhalten:
      c65a, c65b, c65i, c65k, c65m, c65n, c65o, c65w, c65z bleiben grün; Δ „Globale Variablen“ durch R1 ≤ 60 Byte
      (gerechnet rund 50).

### Schritt G9 — Ausgangsstand ohne Cache (auf R1)

- [ ] **AKM.1 (Einspielen R1)** — ESP-OTA; Smoketest samt Update-Quelle und neuen Feldern **ohne** FEHLT,
      `install-app.sh --check` vollständig, `check-pwa.sh` ohne FEHLT; Versionen gleich denen des Tags; die Zeile
      `- flash id …` steht im Mitschnitt, Hersteller ins Messprotokoll. Wache läuft vorher.
- [ ] **AKM.2 (Ladeprobe ohne Cache)** — Drei Läufe `ladeprobe` (je 1 kalt, 5 warm), einer in Englisch. Je Laden:
      Anfragen nach Leitungsstatus, Byte, `heap_low_block`, `heap_low_path`, I-4-Felder, Vermerke. Erwartet (◐): auch
      warm nur `200`. Vergleichswert für AKW.2 — mit Ent-18 A ersetzt durch die Grundlinie auf R1b (AKS.4).
- [ ] **AKM.3 (Browser-Sitzung Safari)** — Phasen S0–S4 nach `design.md` §5.3, Betriebsart `osascript` (Lead);
      **danach einmal** Betriebsart `manuell` auf dem Gerät des Nutzers aus L461. Je Phasenende: `free_heap`,
      `max_free_block`, `tcp_now_*`, `http_accept_dead`, `no_request_*`, Heap-Zeilen mit `tcp=` im Phasenfenster,
      Mac-Seite (`netstat`, `lsof`; beim Gerät des Nutzers nur die ESP-Seite), lokaler Port und TIME_WAIT-Ende jeder
      eigenen Lesung. **Je Sitzung:** Δ`no_request_timeouts` gegen Δ`no_request_aborts` und Δ`http_accept_dead` über
      S1–S3 — ist Δ`timeouts` ≈ 0, ist K1 für L461 wirkungslos. **Solange L7 nicht eingespielt ist (A.5):** vor S0 den
      Client freistellen (ein kalter `update_table_files`, der zu Ende liest, oder ein Durchgang des Abgleichs) und je
      Phase `hc` und `update_info_cached` protokollieren; eine Lücke um 1'704 Byte bei steigendem `hc` oder wechselndem
      `update_info_cached` wird C71 zugerechnet, nicht C72. Mit Ent-18 A läuft die Sitzung auf R1b.
- [ ] **AKM.4 (Browser-Sitzung Chrome)** — dieselben Phasen und Auswertungen, `osascript`, nach mindestens 300 s Ruhe.
- [ ] **AKM.5 (Startmessung C71)** — ESP-Starts ohne Browser nach `design.md` §5.5 mit Auswertung an der Lochsumme `L`,
      keine HTTP-Lesung bis `- update cache heap` + 130 s (◐ H4, `tw=0`), Start 4 mit kaltem `update_status` als
      gekennzeichnete Abweichung; ein Start mit leerer Antwort oder `fehlgeschlagen abruf` gekennzeichnet (C75). **Stand
      16.10.2026: gefahren, Ergebnis N5** — Start 3 ohne Zerstückelung, Start 4 bindet 1'704 Byte; dazu Messungen M1
      und M3 (L468). Abnahme durch den Lead mit dem vollständigen Protokoll.
- [ ] **AKM.6 (Gesundheit G9)** — Exceptions, ungewollte Neustarts, `diag`-Lücken, `ovr` vollständig erfasst und als
      Ergebnis berichtet (Ent-7); `watch-log.sh --pruefe` im Bericht.
- [ ] **AKM.7 (P1-Probe)** — Nach `design.md` §5.6: ein Socket sendet `GET /api/device_ready`, liest die Antwort und
      schliesst **nicht**; aus einer zweiten Verbindung `device_ready` nach 10 s und nach 25 s. **Erwartet (P1):** nach
      10 s ein Eintrag FIN_WAIT_2 mit `arg` = 0, Gegenstelle Mac und dem lokalen Port der Probe; nach 25 s ist er weg;
      die Mac-Seite zeigt CLOSE_WAIT. **Ist er nach 25 s noch da, ist P1 widerlegt** — dann gilt K2 am Tor als zwingend
      (Ent-8). Ergebnis ins Messprotokoll, bevor die Browser-Sitzungen beginnen.

### Massnahme C71 — L7 (in R1b nach Ent-18 A, sonst in R3)

- [ ] **AKS.1 (L7 umgesetzt)** — Nach `design.md` §7: **L7a** `httpclient_stop()` gibt nach `client.stop()` den
      `ClientContext` frei (`client = WiFiClient()`; Refcount 0, `discard_received()`); **L7b** die direkten
      `client.stop()` in den Fehlerwegen von `httpclient()` (`httpclient.cpp:216` keine Antwort, `:224` Status ≠ 200)
      laufen über `httpclient_stop()`; **L7c = C75** `httpclient()` bei `len == 0` und jeder Aufrufer, der bei
      `len <= 0` aufgibt (darunter `update_cache_write()`), schliesst über `httpclient_stop()`. Im Bericht: Liste aller
      Aufrufer von `httpclient()` mit ihrem Weg bei `len <= 0` und bei ungelesenem Rumpf (mindestens
      `http_api_update_status()` mit `wc12h-icon.txt`/`wc12h-weather.txt`, die Prüfung auf `app.js.gz`, der Zweig
      `zu-gross` in `update_cache_write()`). Prüfstand **c71e** mit den Fällen (1)–(7) aus `design.md` §8; **Sabotage
      „nur `stop()`“ (ohne Freigabe des Kontexts) ⇒ FEHL**, ebenso „Fehlerweg mit direktem `client.stop()`“ und
      „`len == 0` offen“; gegen den Vorstand ⇒ FEHL.
- [ ] **AKS.2 (Ruhewert nach Start und Last)** — Ruhewert `max` der Heap-Zeile **≥ 5'120** (K30) (i) in drei ESP-Starts
      ohne Browser, einer davon mit erzwungenem Schreiben beim Start, an einer Heap-Zeile ≥ 130 s nach `n` mit `tw=0`,
      und (ii) 150 s nach einem `check-pwa.sh` (die PWA fragt `update_status` dabei ab) und nach einer Ladeprobe — je
      mit den Vermerken. Daneben L460 (4'672), L457 (7'184), L468 (7'328/7'152 nach Freigabe).
- [ ] **AKS.3 (L7 ohne Regression)** — Jeder Abruf des httpclient funktioniert weiter: Release Notes und Liste werden
      abgeglichen (`- update cache … unveraendert`/`geschrieben`), `update_status` liefert alle Felder samt Existenz von
      Icon- und Wetterdatei unverändert gegen den Vorstand, die Prüfung auf `app.js.gz` und `update_table_files`
      liefern wie vorher; `install-app.sh --check` vollständig; keine `*.tmp`; c65m, c65n grün; c71e Fälle (6) und (7).
- [ ] **AKS.4 (keine Belegung nach kaltem `update_status`)** — Messung wie G9.6 Start 4 und Messung M1: ESP in Ruhe,
      Heap-Zeile lesen, **einen** kalten `update_status`, danach keine HTTP-Lesung bis zu einer Heap-Zeile ≥ 130 s danach
      mit `tw=0` (◐ H4). **`free` liegt höchstens 300 Byte unter dem Stand davor, `max` ebenso.** Dreimal; zum Vergleich
      der Wert auf dem Vorstand (−1'704, L468). Mit Ent-18 A dazu auf R1b eine Ladeprobe (drei Läufe wie AKM.2) als
      neue Grundlinie für AKW.2.

### Schritt R2 — C70 Asset-Cache (Variante (a), Ent-1)

- [ ] **AKC.1 (`ETag`)** — Jede `200`-Antwort auf ein Asset aus `APP_INSTALL_ASSETS`, das als `.gz` ausgeliefert wird,
      beginnt mit `HTTP/1.1 200 OK` (AKC.7) und trägt einen `ETag` nach Ent-2: **Variante T**
      `"<gz-grösse>-<trailer>"` (letzte 8 Byte der `.gz` als Hex) oder **Variante V** `"<grösse>-<crc32>"`. Beginnt die
      Datei nicht mit `1f 8b`, **kein** `ETag` (nur T). `Content-Type`, `Content-Encoding`, `Content-Length`,
      `Cache-Control: no-cache`, `Connection: close` unverändert. Legacy, `/api/*`, Installationsseite und Assets
      ausserhalb der Weissliste tragen **kein** `ETag`. *A.6:* `Cache-Control: no-cache` gilt ab R2b nur noch ohne
      Treffer nach AKL.1.
- [ ] **AKC.2 (`If-None-Match` und `304`)** — Die Kopfzeilenschleife von `http_server_loop()` liest `If-None-Match`
      (Name ohne Rücksicht auf Gross-/Kleinschreibung; `W/` und Komma-Liste verstanden; `*` gilt als **kein**
      Treffer). Trifft ein Wert den aktuellen `ETag` **und** die Datei besteht mit Grösse > 0 **und** — beim Index — die
      Installation ist vollständig (`http_app()`), antwortet der ESP `HTTP/1.1 304 Not Modified` mit `ETag`,
      `Cache-Control: no-cache`, `Connection: close`, **ohne** Körper und ohne `Content-Length`. Sonst der heutige Weg.
      Prüfstand **c70e**.
- [ ] **AKC.3 (Gültigkeit des `ETag`)** — **Variante T:** bei **jeder** Anfrage aus der Datei selbst gelesen (Grösse,
      ein `seek` ans Ende − 8, 8 Byte, zurück an den Anfang); keine Tabelle, keine Invalidierung. c70e: zwei gültige
      `.gz` gleicher Grösse mit verschiedenem Inhalt ⇒ verschiedene `ETag`; abgeschnittene Datei ⇒ anderer `ETag`;
      keine gzip-Datei ⇒ kein `ETag`; Sabotagen „`ETag` nur aus der Grösse“, „`304` ohne Grössenprüfung“, „`304` mit
      Körper“ ⇒ je FEHL. **Variante V:** RAM-Tabelle je Asset, nach dem Start leer, ein Eintrag gilt nur bei gleicher
      Grösse, und **jede** der 31 schreibenden LittleFS-Stellen ruft danach `app_etag_invalidate()`; CRC-32/MPEG-2;
      c70e zählt die Stellen selbst; Sabotage zusätzlich „eine Invalidierung entfernt“ ⇒ FEHL. In beiden Varianten
      gegen den Vorstand ⇒ FEHL.
- [ ] **AKC.4 (Kosten und Zähler)** — **T:** je Antwort ein `seek` und 8 Byte Lesen mehr; Mehrdauer je Anfrage im
      Messprotokoll (◐ ms). **V:** CRC nur bei Bedarf, je Berechnung eine Zeile `- app etag <datei> n=<byte>
      ms=<dauer>`, `app-app.js.gz` unter 2'000 ms (◐ FA 0,15–0,3 s). In beiden: kein Heap, kein `String`;
      `device_ready` gibt hinter den I-5-Feldern `app_not_modified`, `app_full` und (nur V) `app_etag_calc` aus.
- [ ] **AKC.5 (Invarianten)** — App-Assets weiter **ausschliesslich** als `.gz` mit Prüfung Grösse > 0
      (`http_fs_file_exists_and_nonempty()`); kein Plain-Fallback; `/app/?action=install` und Installationsseite
      unverändert; die Legacy-Oberfläche liefert wie vorher (Stichprobe `/`, `/legacy`, `/fs`, `/update` per `curl`
      gegen den Vorstand bis auf Zeit- und Messwerte, Statuszeile `HTTP/1.0`, ohne `ETag`).
- [ ] **AKC.6 (PWA-Teil, Ent-3)** — `loadLayoutPreviewRowsMap()` holt `layout-previews.json` mit `cache: "no-cache"`
      statt `"no-store"`; sonst keine Änderung an `app.js`; in `sw.js` nur `CACHE_NAME`. Prüfstand **c70p**; Sabotage
      `"no-store"` ⇒ FEHL; gegen 1.4.100 ⇒ FEHL. *A.6:* Bleibt in R2b so — `layout-previews.json` ist unversioniert
      (Ent-20 B).
- [ ] **AKC.7 (Statuszeile `HTTP/1.1` nur für App-Assets)** — Nur die Kopfzeilen in `http_send_fs_file()` und die neue
      `304` tragen `HTTP/1.1`; **beide** senden `Connection: close`, der ESP schliesst danach wie heute. Die `200` trägt
      ein `Content-Length` gleich der gesendeten Körperlänge; ein Abbruch endet mit RST, nie regulär. Die übrigen
      Antworten in `http_app()`, Legacy und `/api/*` bleiben `HTTP/1.0`. Prüfstand **c70e** (statisch: `HTTP/1.1` an genau
      diesen zwei Stellen, beide mit `Connection: close`; dynamisch: Körperlänge = `Content-Length`); Sabotagen
      „`Connection: close` in der `304` entfernt“ und „`HTTP/1.1` in einer `/api/`-Antwort“ ⇒ je FEHL; Ladeprobe am
      Prüfserver mit `HTTP/1.0` ⇒ FEHL.
- [ ] **AKC.8 (bestehende Prüfstände nachgezogen)** — c65b und c65w und die LittleFS-Attrappe sind auf den Stand R2
      nachgezogen und grün; ihre Fallzahl ist gegen den Stand vorher gezählt, keine Fälle verloren.

### Schritt G10 — mit Cache, und Tor (auf R2)

- [ ] **AKW.1 (Einspielen R2, DIR-002)** — **erst** ESP-OTA, Smoketest, `install-app.sh --check`; **dann**
      `install-app.sh` mit der neuen PWA, erneut `--check`; `check-pwa.sh` ohne FEHLT; beide Versionen gleich denen
      des Tags.
- [ ] **AKW.2 (Ladeprobe mit Cache, Chrome)** — Drei Läufe, `--erwarte-304`. Je **warmem** Laden (Neuladen **und**
      erneutes Öffnen): für **jede** `/app/`-Anfrage `If-None-Match` gesendet und Leitungsstatus `304`, das Dokument
      eingeschlossen; Summe `encodedDataLength` über `/app/` ≤ 3'000 Byte; Δ`app_full` = 0; `heap_low_block` im warmen
      Fenster ≥ 1'072. Kalt berichtet. Gegenüberstellung mit der Grundlinie (AKM.2 bzw. mit Ent-18 A AKS.4).
- [ ] **AKW.3 (Probe U — Update sofort wirksam)** — Im dauerhaften Profil warm laden; `app/layout-previews.json` mit
      anderem Inhalt über `app_file_upload` hochladen; neu laden: genau diese Datei `200` mit **neuem** `ETag`, alle
      anderen `304`. Original zurücklegen, neu laden: `200` mit dem ursprünglichen `ETag`; danach
      `install-app.sh --check` ohne Abweichung.
- [ ] **AKW.4 (Browser-Sitzungen mit Cache, Safari ungeprüft)** — Safari und Chrome wie AKM.3/AKM.4; vor S0 einmal
      öffnen und beenden; ohne L7 mit Freistellen des Clients. Zusätzlich je Phase `app_full`, `app_not_modified`; für
      Safari der erste Nachweis, ob er bei `HTTP/1.1` mit `ETag` nachfragt; bleibt `app_not_modified` bei 0, ist das ein
      Befund.
- [ ] **AKW.5 (Auswertung und Tor)** — C72 nach `design.md` §5.4 **einem** von M1–M5 zugeordnet — **die Lücke nach
      Abzug einer C71-Belegung**, auf G10, G9 und die P1-Probe daneben, mit Δ`timeouts` gegen Δ`aborts`. Ins
      Messprotokoll, vom `doc-writer` nachgeführt (nicht gleichzeitig, R3). **Erst danach** legt der Lead Ent-8, Ent-10
      und Ent-14 vor (mit Ent-18 B auch die Freigabe von Ent-17). **Tor am 16.10.2026:** M5, Ent-8 kein Eingriff (L468).
- [ ] **AKW.6 (Gesundheit und Grundlinie G10)** — wie AKM.6; dazu **drei `check-pwa.sh`** auf R2 mit der Zahl der
      `net::ERR_CONNECTION_RESET` und `net::ERR_EMPTY_RESPONSE` je Lauf und Δ`write_lost_failed` als **Grundlinie** für
      AKE.3 und AKG.7.

### Schritt R2b — C70 Variante (c): versionierte URLs mit Langzeit-Cache (Nachtrag A.6)

Die Kriterien gelten mit den **Entscheiden vom 16.10.2026**: **Ent-19 C**, **Ent-20 B**, **Ent-21 E2**, **Ent-22 ja**,
**Ent-23 1 Jahr**. **Inhaltsmarke** heisst der `ETag`-Wert nach Variante T **ohne** Anführungszeichen:
`<gz-grösse>-<16 Hexziffern>`. **Versioniert** sind die sechs Verweise des Dokuments (Manifest, `icons/icon-192.svg`,
`icons/icon-192.png`, `icons/icon-180.png`, `styles.css`, `app.js`) und jede Datei unter `i18n/`; **nicht versioniert**
sind das Dokument, `sw.js` und `layout-previews.json` (Ent-20 B).

- [ ] **AKL.1 (Langzeit-Cache nur bei bestätigtem Inhalt)** — Eine Antwort des ESP auf ein App-Asset trägt
      `Cache-Control: public, max-age=31536000, immutable` (Ent-23) **genau dann**, wenn alle vier Bedingungen gelten:
      (i) Die Anfrage trägt den Parameter `h` mit nicht leerem Wert; (ii) der Wert ist zeichengleich mit dem `ETag`
      **dieser** Antwort ohne die Anführungszeichen; (iii) die Antwort läuft im Zweig `HTTP/1.1` mit `ETag`
      (Weissliste, `.gz`, Grösse > 0, gzip-Kopf); (iv) es ist **weder** das Dokument (`/app`, `/app/`,
      `/app/index.html`) **noch** `/app/sw.js`. Sonst `Cache-Control: no-cache` wie in R2. Das gilt für `200` **und**
      `304` gleich. `ETag`, `Content-Type`, `Content-Encoding`, `Content-Length`, `Connection: close` und die
      Statuszeile bleiben wie in R2; `v` wertet der ESP nicht aus; der Wert von `h` wird **nie** ausgegeben. Vergleich
      ohne Heap und ohne `String`. Dass `layout-previews.json` nie `immutable` bekommt, folgt daraus, dass weder das
      Dokument noch `app.js` sie mit `h` anfragen (AKL.4, AKL.6); der ESP behandelt sie wie jedes Asset. Prüfstand
      **c70c** mit mindestens: Treffer ⇒ `immutable` (bei `200` und bei `304`); falsche Marke, um ein Zeichen
      verschieden, Präfix der Marke, Marke mit Anführungszeichen, leeres `h`, fehlendes `h` ⇒ `no-cache`; Dokument und
      `sw.js` mit ihrer eigenen, treffenden Marke ⇒ `no-cache`; Datei ohne gzip-Kopf, Datei unter 18 Byte, Asset
      ausserhalb der Weissliste mit `h` ⇒ wie R2; zwei `.gz` gleicher Grösse mit anderem Inhalt ⇒ nur die eigene Marke
      trifft. Sabotagen „`immutable` ohne Markenvergleich“, „Vergleich nur über die Länge bzw. als Präfix“, „Dokument
      nicht ausgenommen“, „`sw.js` nicht ausgenommen“, „`304` bei Treffer mit `no-cache`“, „`immutable` im Zweig
      `HTTP/1.0`“, „`h` im Kopf ausgegeben“ ⇒ je FEHL; gegen R2 (3.2.40) ⇒ FEHL.
- [ ] **AKL.2 (Zähler)** — `device_ready` gibt hinter `app_full` aus: **`app_immutable`** (Antworten mit `immutable`,
      `200` und `304`) und **`app_token_miss`** (Anfragen an ein App-Asset mit nicht leerem `h`, die **kein**
      `immutable` bekamen — Dokument und `sw.js` eingeschlossen). Sättigend bei 65'535, nichts wird zurückgesetzt,
      keine Logzeile je Anfrage. c70c zählt beide in jedem Fall aus AKL.1 nach; Sabotage „Zähler fehlt in einem Zweig“
      ⇒ FEHL.
- [ ] **AKL.3 (ohne Marke wie R2)** — Ohne `h` ist jede Antwort auf jedes der 13 Assets in Kopf und Rumpf gleich wie
      auf R2 (gleiche `ETag`-Werte und Grössen); Legacy (`/`, `/legacy`, `/fs`, `/update`), `/api/*`, Installations-
      und Hinweisseiten unverändert (`HTTP/1.0`, ohne `ETag`, ohne `immutable`; Stichprobe per `curl` gegen R2 bis auf
      Zeit- und Messwerte). c70e, c65b, c65w grün, Fallzahl gegen R2 nicht kleiner. AKC.1–AKC.8 gelten weiter, mit dem
      Vermerk in AKC.1.
- [ ] **AKL.4 (Build erzeugt das versionierte Dokument)** — `make app-gz` erzeugt `index.html.gz` aus einer **Kopie**
      von `index.html`, in der (Ent-21 E2) die **sechs** Verweise `href`/`src` auf `/app/manifest.webmanifest`,
      `/app/icons/icon-192.svg`, `/app/icons/icon-192.png`, `/app/icons/icon-180.png`, `/app/styles.css` und
      `/app/app.js` die Form `<pfad>?v=<APP_VERSION>&h=<Inhaltsmarke>` tragen und das Meta-Element `wc-asset-tokens`
      die Inhaltsmarke **jeder** Datei unter `i18n/` führt — und **nur** diese; `layout-previews.json` erscheint weder
      im Meta-Element noch mit Marke (Ent-20 B). Jede Marke ist aus der im **selben Lauf** erzeugten `<pfad>.gz` nach
      der Formel von `http_app_etag()` gerechnet; `APP_VERSION` stammt aus `const APP_VERSION` in `app.js` und aus
      keiner anderen Quelle. Die Quelle `index.html` ändert der Build nicht; `index.html.gz` und `sw.js.gz` tragen keine
      eigene Marke. Das Werkzeug meldet die Fallzahl (Verweise und Meta-Einträge) und bricht mit Exit ≠ 0 **und**
      Meldung ab, wenn eine `.gz` fehlt, 0 Byte hat oder keine gzip-Datei ist, die Zahl der umgeschriebenen Verweise
      nicht 6 ist, ein Verweis schon eine Query trägt, eine Sprachdatei unter `i18n/` im Meta-Element fehlt,
      `layout-previews.json` eine Marke bekäme, eine Marke nicht `^[0-9]+-[0-9a-f]{16}$` oder `APP_VERSION` nicht
      `^[0-9]+(\.[0-9]+)*$` entspricht. `make` bricht dann ab, und es entsteht **keine** neue `index.html.gz` — nie eine
      leere. Gegenproben: je Abbruchgrund eine hergestellte Verletzung ⇒ Exit ≠ 0 mit Meldung in Konsole **und**
      Make-Ausgabe.
- [ ] **AKL.5 (Form der Query)** — Jede erzeugte Query (im Dokument und in den Laufzeitabrufen von `app.js`) besteht nur
      aus Paaren `name=wert`, **jedes mit `=`**, getrennt durch `&`, aus den Zeichen `[0-9a-z.=&-]`, höchstens 64
      Zeichen (Grenze 255, C46). Das Werkzeug aus AKL.4 prüft jede erzeugte URL und meldet ihre Zahl; c70p prüft die
      Bildung in `app.js`; Sabotage „Parameter ohne `=`“ ⇒ FEHL an beiden Stellen.
- [ ] **AKL.6 (PWA-Teil)** — `app.js` liest das Meta-Element `wc-asset-tokens` einmal, übernimmt eine Marke nur nach
      `^[0-9]+-[0-9a-f]{16}$` und holt `i18n/<code>.json` (`ensureLanguageTable()`) mit `?v=<APP_VERSION>&h=<marke>`,
      ohne gültige Marke mit `?v=<APP_VERSION>` (die heutige Form), weiter mit `cache: "default"`.
      **`loadLayoutPreviewRowsMap()` bleibt unverändert** (Ent-20 B): `/app/layout-previews.json` ohne Query, mit
      `cache: "no-cache"`. Sonst keine Änderung an `app.js`; in `sw.js` nur `CACHE_NAME`. c70p nachgezogen: Sabotagen
      „Sprachdatei mit `"no-cache"` bzw. `"no-store"`“, „Marke ohne Musterprüfung übernommen“, „Parameter ohne `=`“,
      „`layout-previews.json` mit `h`“, „`layout-previews.json` nicht mehr `"no-cache"`“ ⇒ je FEHL; gegen 1.4.101 ⇒
      FEHL.
- [ ] **AKL.7 (Markup)** — `index.html` erhält im `<head>` genau ein neues Element `<meta name="wc-asset-tokens"
      content="">` (in der Quelle leer); sonst keine Änderung (Ent-21 E2). Die Quelle lädt in der PWA-Vorschau danach
      ohne Ladefehler, die Verweise ohne Query.
- [ ] **AKL.8 (Werkzeuge)** — (i) `install-app.sh --check` liest das ausgelieferte Dokument, prüft **jeden**
      versionierten Verweis und jeden Meta-Eintrag am Gerät (Antwort mit `immutable`, `ETag` gleich der Marke), nennt
      jede Abweichung beim Namen („Marke passt nicht“, „kein immutable“, „Meta-Eintrag fehlt“) und endet dann mit
      Exit ≠ 0; Fallzahl gemeldet; Gegenprobe mit verfälschter Marke (Attrappe) ⇒ FEHL. (ii) `smoke-device.sh` Block
      C70c, nur lesend: `app.js` mit treffender Marke ⇒ `immutable`; mit falscher Marke, ohne Marke, `/app/` und
      `/app/sw.js` mit ihrer eigenen Marke ⇒ `no-cache`; Δ`app_immutable` und Δ`app_token_miss` passend; Gegenprobe
      gegen R2 bzw. `SMOKE_DR_OVERRIDE` ⇒ FEHLT. (iii) `ladeprobe --erwarte-cache`: je Laden für jede `/app/`-URL
      **Leitung** oder **Cache** (Abschnitt „Status `304` in CDP-Werkzeugen“), dazu Δ`app_full`, Δ`app_not_modified`,
      Δ`app_immutable`, Δ`app_token_miss`; die unversionierten Pfade (Dokument, `layout-previews.json`) erwartet es auf
      der Leitung mit `304`, alle anderen aus dem Cache; Gegenproben: gegen R2 ⇒ Exit ≠ 0 und Meldung „warm: Cache
      erwartet, 304 erhalten“; Prüfserver mit `immutable` ⇒ OK. (iv) `probe-u.sh` mit den Abläufen U1a, U1b, U2, U3
      nach `design.md` §18.8; Gegenprobe ohne Upload ⇒ FEHL je Ablauf. (v) `check-pwa` INFO Zahl der Antworten aus dem
      Cache je Lauf. Jede Meldung in Konsole **und** Mitschrift.
- [ ] **AKL.9 (RAM und Release-Inhalt)** — Δ „Globale Variablen“ durch R2b ≤ 8 Byte (gerechnet 4), Paketsumme ≤ 250
      (Stand nach R2 +80, L468); Programmgrösse und `app.js.gz` vor und nach R2b im Messprotokoll. R2b ändert nur
      `http.cpp` (gegebenenfalls `base.h`), `version.h`, `data/app/app.js`, `data/app/index.html` (das Meta-Element),
      `data/app/sw.js` (`CACHE_NAME`), `Makefile` (`app-gz`) und `tools/**` — **keine** Änderung an `styles.css`,
      `manifest.webmanifest`, Icons, `layout-previews.json`, `src/**` und den Legacy-Ausgaben. Geprüft gegen das Tag von
      R2.

### Schritt G10b — Wirkung von (c) (auf R2b, Nachtrag A.6)

- [ ] **AKG.1 (Einspielen R2b, DIR-002)** — **erst** ESP-OTA, Smoketest samt Update-Quelle und Block C70c ohne FEHLT;
      **dann** `install-app.sh` mit der neuen PWA; `--check` vollständig **einschliesslich** der Markenprüfung
      (AKL.8 (i)); `check-pwa.sh` ohne FEHLT; Versionen gleich denen des Tags.
- [ ] **AKG.2 (Ladeprobe Chrome — warm keine Nachfrage an versionierte Assets)** — Drei Läufe `ladeprobe
      --erwarte-cache` (je 1 kalt, 5 warm), der dritte englisch, im dauerhaften Profil. Je **warmem** Laden (Neuladen
      **und** erneutes Öffnen): über die Leitung **nur** das Dokument und `layout-previews.json`, jede mit `304`;
      **jede** versionierte URL aus dem Cache; Gerätezähler im warmen Fenster Δ`app_not_modified` gleich der Zahl dieser
      Leitungsanfragen (erwartet 2), Δ`app_full` = 0, Δ`app_immutable` = 0, Δ`app_token_miss` = 0 — beide Instrumente
      zählen dasselbe (L235); Summe `encodedDataLength` über `/app/` ≤ 400 Byte; `heap_low_block` ≥ 1'072. Ausnahme wie
      in G10.1: Der erste Abruf von `i18n/en.json` nach dem Sprachwechsel kommt mit `200` und `immutable` und zählt
      nicht. Kalt: jede versionierte URL `200` mit `immutable`, berichtet. Die Menge der Pfade je Laden ist gegen die in
      G10.1 gesehenen abgeglichen — keiner fehlt, keiner kommt dazu. Gegenüberstellung mit G10.1 (626–835 Byte, 6–8
      Anfragen) und G9b.3.
- [ ] **AKG.3 (Probe U1 — Datei ersetzt, Dokument gleich)** — Im dauerhaften Profil warm, zwei Teile.
      **U1a, unversioniert (Ent-20 B), wie AKW.3:** `layout-previews.json` mit anderem Inhalt über `app_file_upload`
      ersetzen; beim nächsten warmen Laden kommt **genau diese Datei** mit `200` und **neuem** `ETag`, das Dokument mit
      `304`, alle versionierten Dateien aus dem Cache; danach wieder `304` mit dem neuen `ETag`; Original zurück ⇒ `200`
      mit dem ursprünglichen `ETag`. **U1b, versioniert:** `styles.css` mit anderem Inhalt ersetzen, Dokument
      unverändert. Erwartet: (i) ein direkter Abruf mit der **alten** Marke liefert `200`, das **neue** `ETag` und
      `Cache-Control: no-cache` — **kein** `immutable` —, Δ`app_token_miss` = 1; (ii) ein frisches Profil bekommt den
      neuen Inhalt mit `no-cache`; (iii) der warme Browser liefert die Datei weiter aus dem Cache — es ist der Inhalt,
      den sein Dokument nennt (Ent-20 A sinngemäss). Original zurück: Abruf mit der Marke wieder `immutable` mit dem
      ursprünglichen `ETag`. Nach beiden Teilen `install-app.sh --check` ohne Abweichung. Gegenprobe ohne Upload ⇒ FEHL
      in U1a (kein neues `ETag`) und in U1b (i).
- [ ] **AKG.4 (Probe U2 — neuer Inhalt mit neuer Marke, Dokument zuerst)** — Mit `styles.css`, Reihenfolge wie
      `install-app.sh`: vor warm #2 ein Dokument mit der neuen Marke für `styles.css` hochladen, die Datei selbst noch
      nicht; vor warm #3 die Datei. Erwartet: #2 Dokument `200` mit neuem `ETag`, `styles.css` mit neuer Marke `200` mit
      **altem** Inhalt und `no-cache` (das Fenster), Δ`app_token_miss` = 1; #3 Dokument `304`, `styles.css` `200` mit
      neuem `ETag` und `immutable`; #4 alles ausser den unversionierten Pfaden aus dem Cache; vor #5 beide Originale
      zurück ⇒ #5 Dokument `200`, `styles.css` mit der ursprünglichen Marke **aus dem Cache**. In keinem Schritt eine
      andere versionierte Datei über die Leitung. Danach `--check` ohne Abweichung. Gegenprobe ohne Upload ⇒ FEHL.
- [ ] **AKG.5 (Probe U3 — Versionswechsel als Rückweg)** — Ein Dokument mit anderer `v`, sonst gleich (vom Werkzeug aus
      AKL.4 erzeugt), hochladen: Das nächste warme Laden holt **jede** versionierte URL über die Leitung mit `200` und
      `immutable`, das folgende wieder alles Versionierte aus dem Cache; Original zurück ⇒ die URLs der ursprünglichen
      Version kommen aus dem Cache. Belegt, dass eine neue `APP_VERSION` jeden Eintrag umgeht (Rückweg, `design.md`
      §18.9). Gegenprobe ohne Upload ⇒ FEHL.
- [ ] **AKG.6 (Safari auf dem Mac und auf dem iPhone)** — Browser-Sitzungen wie G10.3 bzw. G9b.4 (Safari `osascript`,
      iPhone `manuell`, Chrome `osascript`), mit Vorwärmen vor S0, zusätzlich je ein **Neuladen** in S1. Je Phase
      Δ`app_full`, Δ`app_not_modified`, Δ`app_immutable`, Δ`app_token_miss`. Für Chrome gilt das Soll aus AKG.2. Für
      Safari und das iPhone **gemessen und berichtet**: erwartet (◐) beim Öffnen und beim Neuladen je
      Δ`app_not_modified` ≤ 2 (Dokument und `layout-previews.json`) und Δ`app_full` = 0; fragt Safari versionierte
      Dateien trotz `immutable` nach, ist das ein Befund, kein Abbruch. Ob das versionierte Manifest geladen wird
      (Ent-22), wird je Browser vermerkt. Lücke in S2–S4 wie in G10.3 berichtet.
- [ ] **AKG.7 (Gesundheit und Grundlinie G10b)** — wie AKW.6: drei `check-pwa.sh` auf R2b; `net::ERR_CONNECTION_RESET`
      und `net::ERR_EMPTY_RESPONSE` nicht mehr als in der Grundlinie G10.3a (je 0), Δ`write_lost_failed` und
      Δ`fs_write_short` 0; Ruhewert `max` 150 s danach ≥ 5'120 (K30); `watch-log.sh --pruefe` im Bericht; Neustarts,
      Exceptions und Watchdog-Resets als Ergebnis (Ent-7).

### Schritt R3 und G11 — Eingriff C72 (nach dem Tor) — **entfallen**

**Entfallen mit Ent-8 = kein Eingriff** (Nutzer, 16.10.2026; Zuordnung M5 auf G9b und G10, L468). AKE.1–AKE.3 bleiben
zur Nachvollziehbarkeit stehen und gelten im Endstand als „entfallen“ mit diesem Grund. L7 kam mit Ent-18 A in R1b, also
gehören auch AKS.1–AKS.4 nicht hierher.

AKE.* entfallen ohne C72-Eingriff (M4, M5 oder Ent-8 = keiner); im Endstand dann „entfallen“ mit Grund. Mit Ent-18 B
gehören AKS.1–AKS.4 ebenfalls in diesen Schritt.

- [ ] **AKE.1 (C72-Eingriff umgesetzt)** — Kandidat nach Ent-8 gemäss `design.md` §6. **K1:** im Zweig „keine
      Anfrage“ von `http_server_loop()` im Teilfall `no_request_timeouts` `abort()` statt `stop()`; der Teilfall
      `no_request_aborts` bleibt; beide Zähler unverändert. **K2:** höchstens einmal je Sekunde PCBs mit
      `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING, LAST_ACK oder — bei `TF_CLOSEPEND` — ESTABLISHED und
      CLOSE_WAIT, deren Ruhezeit die Frist aus Ent-14 überschreitet, mit `tcp_abort()` abbauen; PCBs mit
      `ClientContext` **oder** Listener **nie**; Zähler `tcp_reaped`. **K5:** `HTTP_TW_LIMIT` 1. Prüfstand **c72e**;
      Sabotagen nach `design.md` §8; gegen R2 ⇒ FEHL.
- [ ] **AKE.2 (C72-Wirkung, G11)** — Browser-Sitzung Safari wie AKW.4: 180 s nach Beginn von S2 **und** von S3 je
      `free_heap` ≥ `free_heap`(S0) − 300 Byte; in `tcp_now_list` kein PCB mit `arg` = 0 ausser TIME_WAIT und keiner mit
      Ruhezeit über der Frist aus Ent-14. Daneben L461 (nach Abzug der C71-Belegung), AKM.3, AKW.4.
- [ ] **AKE.3 (C72 ohne Regression, gegen die Grundlinie R2)** — drei `check-pwa.sh` ohne FEHLT;
      `net::ERR_CONNECTION_RESET` und `net::ERR_EMPTY_RESPONSE` **nicht mehr als in der Grundlinie R2** (AKW.6; C74
      erzeugt schon heute Resets); Ladeprobe warm wie AKW.2; Δ`write_lost_failed` nicht über der Grundlinie; Smoketest
      ohne FEHLT.

### Übergreifend

- [ ] **AKZ.3 (Kodierung und Zeilenende)** — Vor jedem Patch festgestellt und im Taskbericht genannt (`http.cpp` UTF-8
      mit einer CRLF-Zeile, `stm32flash.cpp` UTF-8, `ESP-uclock.ino` gemischt, `httpclient.cpp` vor L7 festzustellen;
      in R2b dazu `index.html`, `app.js`, `Makefile`; DIR-015). Der Diff enthält nur inhaltlich geänderte Zeilen; der
      Lead prüft mit `git diff --stat`.
- [ ] **AKZ.4 (Release-Inhalt)** — R1, R1b und R3 ändern nur `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino`, `version.h`;
      R2 dazu `data/app/app.js` und `data/app/sw.js` (`CACHE_NAME`). **Keine** Änderung an `src/**`, `index.html`,
      `styles.css`, `manifest.webmanifest`. Geprüft gegen das jeweils vorige Tag. **R2b nach AKL.9** (mit dem
      Meta-Element in `index.html` und dem `Makefile`).
- [ ] **AKZ.5 (Releases)** — **Jedes** Release einzeln: ein `make release-zip`, ein Commit, ein Tag nach DIR-011, Push
      samt Tag im selben Zug; Einspielzeilen mit Reihenfolge (Firmware zuerst, DIR-002), `install-app.sh --check` nach
      dem OTA (DIR-017) und Rückweg samt Befehl, **bevor** eingespielt wird. **Für R2b** nennen die Einspielzeilen
      ausdrücklich, dass ein ESP-Rückweg die Einträge im Browser **nicht** heilt, und den Rückweg dafür (`design.md`
      §18.9).
- [ ] **AKZ.6 (T3, DIR-012)** — `pwa-tester` Phasen 0–4 und 9 auf dem Endstand. Haltegrenze **K30** nach Ent-12:
      Ruhewert `max` < 5'120 oder `low=` < 1'072 ⇒ anhalten; bei jedem Halt I-4- und I-5-Felder ohne Parameter in den
      Bericht, bevor zurückgesetzt wird. Uhr exklusiv, Beginn und Ende angekündigt.
- [ ] **AKZ.7 (Guardrails und Prüfstände)** — Nach jedem Task `./tools/guardrails.sh` Exit 0; vor jedem Release
      `--full` Exit 0; die neuen Prüfstände in S16, Fallzahl nachgezählt (L235).
- [ ] **AKZ.8 (RAM)** — „Globale Variablen“ und Programmgrösse nach **jedem** ESP-Task sofort ins Messprotokoll;
      Zuwachs über das Paket höchstens **250 Byte** (Ent-11; gerechnet rund 100 mit T, rund 205 mit V; L7 0); darüber
      anhalten und vorlegen. `app.js.gz` vor und nach R2. **Stand nach R2: +80 Byte** (R1 +32, R1b 0, R2 +48; L468);
      R2b gerechnet +4.

**Zahl der Kriterien: 57** — AKZ 9 (davon AKZ.9 erfüllt), AKI 4, AKM 7, AKS 4, AKC 8, AKW 6, AKE 3 (entfallen mit
Ent-8), **AKL 9, AKG 7 (neu mit A.6, zusammen 16)**. Mit den Entscheiden vom 16.10.2026 ist kein Kriterium
hinzugekommen und keines entfallen; nachgezogen sind AKC.6 (Vermerk), AKL.1 (Vermerk), AKL.4, AKL.6, AKL.7, AKL.8,
AKL.9, AKG.2, AKG.3, AKG.4, AKG.5 und AKG.6.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| Variante (b) **allein** (versionierte URLs ohne `ETag`/`304`); `Last-Modified` | Ent-1; mit A.6 kommt (c) — (b) läge unter (a), denn das Dokument braucht den Validator |
| Eine Versionsmarke, die der ESP **nicht** prüfen kann (nur `APP_VERSION`, Ent-19 A) | erzeugt falsche Langzeit-Einträge (`design.md` §18.3) |
| Versionsbestandteil im **Pfad** (`/app/<marke>/app.js`) | Ent-19 D verworfen: berührt Abflachung, Weissliste, Upload und Installation; die Query trennt der ESP schon ab (§18.5) |
| **`layout-previews.json` versionieren**; ihr Abruf in `loadLayoutPreviewRowsMap()` | Ent-20 B: bleibt bei (a), damit eine ersetzte Datei sofort ankommt; kostet je Laden eine `304` |
| Die **installierte** App-Version auf dem ESP ablegen oder `v` auswerten | nicht nötig: Die Inhaltsmarke entscheidet allein (§18.6) |
| Reihenfolge in `install-app.sh` oder in der Auto-Installation ändern (Dokument zuletzt) | nicht nötig: Das Upload-Fenster ist durch die Markenprüfung sicher (AKG.4) |
| Icons **im Manifest** versionieren; `apple-touch-startup-image` und andere neue Verweise | beim Laden der Seite nicht geholt (◐); bleiben bei (a) |
| Service Worker: Vorab-Cache auf versionierte URLs umstellen, `sw.js` versionieren | über `http://` kein Service Worker; `sw.js` darf nie lange gecacht werden (AKL.1 (iv)) |
| `HTTP/1.1` für Legacy, `/api/*`, Hinweisseiten; Keep-Alive, Chunked, Pipelining | nicht nötig (AKC.7) |
| Ein **kalter** Erstaufruf unter 1'072 | C70 ändert ihn nicht; mit Ent-19 C ist auch der erste Aufruf nach jedem PWA-Release kalt |
| **`HEAD` statt `GET` für Existenzprüfungen** | als Option bewertet (`design.md` §7): ersetzt L7 nicht; spätere Verbesserung |
| Jede Existenzprüfung den Rumpf lesen lassen | dezentral; L7 deckt alle Aufrufer zentral ab |
| L6, L0, L2, L4b, L1, L3, L4a, L5 | widerlegt bzw. ohne Bezug zur Ursache (`design.md` §7) |
| **C74** — Wettlauf in der Warteschlaufe auf die erste Zeile | eigener Befund; Grund für die Grundlinie in AKE.3 |
| **C67**, **C66**, **C68**, **C69** | eigene Befunde |
| Änderungen an `index.html` (ausser dem Meta-Element aus A.6), `styles.css`, `manifest.webmanifest`, Icons; Service Worker ausser `CACHE_NAME` | nicht nötig |
| Höchstzahl Verbindungen je Gegenstelle (K4); ein C72-Eingriff vor dem Tor; **jeder C72-Eingriff** | Risiko bzw. nicht belegt; Ent-8 = kein Eingriff |
| Änderungen am Wetter-Client (`openweather_client`) | L7 betrifft nur den httpclient |
| Änderung am Messzeitpunkt von `tcp=`/`hc` in `- update cache nach` (H2) | vermerkt, nicht geändert |
| Änderungen an der Legacy-Oberfläche, am STM | Invariante bzw. nicht betroffen |
| Browser steuern, Schutz für `app_file_upload` (C61), Backlog-Schwellen aus E4, L462 (B61) | nicht möglich bzw. eigene Specs/Befunde |
| Nebenbefunde aus A.1 (`LittleFS.remove()` vor `begin()` in `http_api_app_file_upload()`; `LittleFS.format()` in `setup()` in `#if 0`) | eigene Befundzeilen |
| Die Hinweise H1 und H4 aus Review R2.7 beheben (Leeren des `If-None-Match`-Puffers nach `accept()`; `ETag` deckt nur den Rumpf) | H4 ist in A.6 als Risiko benannt und über die Version abgefangen (§18.9); eigene Befunde |

---

## Betroffene Laufzeiten

- [ ] **ESP8266** — R1: `http.cpp`, `base.h`, `ESP-uclock.ino`; **L7** (R1b bzw. R3): `httpclient.cpp`, `http.cpp`
      (Aufrufer bei `len <= 0`); R2: `http.cpp` (mit V auch `stm32flash.cpp`, `ESP-uclock.ino`), `base.h`; **R2b:**
      `http.cpp`, gegebenenfalls `base.h`; R3: entfallen; je ein OTA
- [ ] **PWA** — R2: `app.js` (eine Zeile), `sw.js` (`CACHE_NAME`); **R2b:** `app.js` (Marken lesen, Abruf der
      Sprachdateien), `index.html` (ein Meta-Element), `sw.js` (`CACHE_NAME`); je ein Upload nach dem OTA
- [ ] **STM32** — keine
- [ ] **Build** — **R2b:** `Makefile` (`app-gz`: `index.html.gz` aus der umgeschriebenen Kopie)
- [ ] **Werkzeug** — `ladeprobe`, `browser-sitzung` (mit Freistellen des Clients bis L7), P1-Probe, Messung AKS.4;
      `check-pwa`, `smoke-device.sh`, `watch-log.sh`; Prüfstände c72i, c71i, c71e, c70e, c70p, c72e; nachgezogen c65b,
      c65w, LittleFS-Attrappe; **R2b:** Werkzeug für die Inhaltsmarken, `install-app.sh --check`, Smoketest-Block
      C70c, `ladeprobe --erwarte-cache`, `probe-u.sh` U1a/U1b/U2/U3; Prüfstand **c70c**, c70p nachgezogen
- [ ] **Dokumentation** — `BEFUNDE.md`, `TESTPLAN-PWA.md`, `CHANGELOG.md`
- [ ] **Build/Release** — vier Releases: R1, R1b, R2, **R2b** (R3 entfallen)

---

## Entscheidungen

**Entschieden am 15.10.2026 vom Nutzer** (über den Lead übermittelt):

| Ent | Gegenstand | Entscheid | Folge |
|---|---|---|---|
| **Ent-1** | Variante C70 | **(a)** `ETag` + `304`; (c) nur nach G10.7 — **G10.7 am 16.10.2026: (c)** | AKC.*; mit A.6 AKL.*, AKG.* |
| **Ent-3** | PWA-Teil | **immer**; R2 mit PWA-Anteil, Firmware zuerst | AKC.6, AKW.1 |
| **Ent-6** | Bedienung der Browser | **C:** Lead per `osascript`, einmal der Nutzer (`manuell`) | AKM.3; in G10b auch AKG.6 |
| **Ent-7** | Haltegrenze in den Messungen | **keine**; Neustarts als Ergebnis | Kopf der Kriterien |
| **Ent-9** | Releaseschnitt | **drei Releases** R1 ⇒ G9 ⇒ R2 ⇒ G10 ⇒ Tor ⇒ R3 — mit Ent-18 A um R1b ergänzt; mit A.6 R2b statt R3 | AKZ.5 |
| **Ent-12** | Ruhewert-Grenze in T3 | **5'120** | AKZ.6 |
| **Ent-13** | C71 | **mitnehmen** | AKS.* |

**Lead-Entscheide 15.10.2026:** **Ent-4** `tcp=`/`nrt=` in der Heap-Zeile; **Ent-5** nur letztes Oktett; **Ent-11**
250 Byte. **Ent-2** wieder offen (unten).

**Entfallen:** **Ent-15** — `CLAUDE.md` R5/DIR-008 vom 14.10.2026 (Commit `e7c5d63`, `CLAUDE.md:412-416`): schreibende
Prüfungen und Messungen laufen ohne Einzelfreigabe; vom `spec-writer` nachgelesen.

**Entschieden am 16.10.2026** (✔ B, `BEFUNDE.md` L468; Ent-19 bis Ent-23 über den Lead übermittelt):

| Ent | Gegenstand | Entscheid | Wer | Folge |
|---|---|---|---|---|
| **Ent-2** | Quelle des `ETag` | **T** — Dateigrösse und gzip-Trailer | Lead | AKC.1, AKC.3; Grundlage der Inhaltsmarke in A.6 |
| **Ent-17** | Massnahme C71 | **L7** freigegeben (L7a, L7b, L7c = C75) | Nutzer | AKS.1 |
| **Ent-18** | Wann L7 kommt | **A** — eigenes Release R1b vor R2 | Nutzer | Ent-16 entfällt |
| **Ent-8** | Eingriff C72 (Tor G10.6) | **kein Eingriff** (M5) | Nutzer | **Ent-10 und Ent-14 entfallen; R3 und G11 entfallen**; C72 wird in Z.1 mit dem Beleg aus G9b und G10 geschlossen |
| **G10.7** | C70 weiter oder anhalten | **weiter mit Variante (c)** — Empfehlung des Leads war Anhalten bei (a) | Nutzer | Nachtrag A.6; Release **R2b** und Messung **G10b** vor T3 |
| **Ent-19** | Form der Versionsmarke | **C** — `?v=<APP_VERSION>&h=<marke>` | Nutzer | AKL.4–AKL.6, AKG.5; Rückweg über jede neue `APP_VERSION` |
| **Ent-20** | Zusage bei einer einzeln ersetzten Datei | **B** — `layout-previews.json` bleibt **unversioniert**: kein `h`, kein `immutable`, weiter `ETag`/`304`, `loadLayoutPreviewRowsMap()` bleibt bei `cache: "no-cache"`. Für eine **versionierte** Datei gilt sinngemäss A: wirksam erst mit dem nächsten Dokument | Nutzer | AKL.4, AKL.6, AKG.2, AKG.3 (U1a wie AKW.3, U1b versioniert), AKG.4 mit `styles.css` |
| **Ent-21** | Wo die Marken in die Verweise kommen | **E2** — das Werkzeug schreibt die sechs Verweise in einer Kopie um; in `index.html` steht nur `<meta name="wc-asset-tokens">` | Lead | AKL.4, AKL.7 |
| **Ent-22** | Manifest versionieren | **ja**; die Prüfung in G10b bleibt | Lead | AKL.4 (6 Verweise), AKG.6 |
| **Ent-23** | `max-age` | **1 Jahr** (`max-age=31536000`) | Lead | AKL.1 |

*Vorgelegt mit A.6 und am 16.10.2026 entschieden (siehe oben) — Optionen und Empfehlungen zur Nachvollziehbarkeit:*

| Ent | Gegenstand | Optionen | Empfehlung damals | Wer |
|---|---|---|---|---|
| **Ent-19** | Form der Versionsmarke in der URL | **(A)** `?v=<APP_VERSION>` allein; **(B)** `?h=<Inhaltsmarke>` allein; **(C)** `?v=<APP_VERSION>&h=<Inhaltsmarke>`; **(D)** Versionsbestandteil im Pfad | **C.** A ist ausgeschlossen: Der ESP kann `v` nicht prüfen, und das Upload-Fenster, Probe U und jeder Einzel-Upload legten einen falschen Inhalt für ein Jahr fest (`design.md` §18.3). B prüft wie C, aber eine Korrektur für **alle** Browser braucht dann eine Inhaltsänderung genau der betroffenen Datei. Mit C erneuert jeder PWA-Release mit neuer `APP_VERSION` **alle** URLs — das ist der Rückweg für jeden falschen Eintrag (§18.9). Preis: je PWA-Release und Browser einmal ein voller Ladevorgang (≈ 200 KB). D verworfen (§18.5) | Nutzer |
| **Ent-20** | Zusage bei einer **einzeln** ersetzten Datei | **(A)** annehmen: wirksam, sobald ein Dokument sie mit ihrer neuen Marke nennt; **(B)** `layout-previews.json` unversioniert lassen, damit Probe U wie auf R2 wirkt | A — **entschieden B** | Nutzer |
| **Ent-21** | Wo die Marken in die Verweise kommen | **(E1)** Platzhalter in den sechs Verweisen von `index.html`; **(E2)** leeres Meta-Element, das Werkzeug schreibt die Verweise in einer **Kopie** um | **E2** | Lead |
| **Ent-22** | Manifest versionieren | ja / nein | **ja** — Kennung einer installierten Web-App über `id` bzw. `start_url` (●, in G10b geprüft) | Lead |
| **Ent-23** | `max-age` | **31'536'000** (1 Jahr) / **2'592'000** (30 Tage) | **1 Jahr** mit Ent-19 C | Lead |

*Stand A.5, überholt durch die Entscheide vom 16.10.2026 — bleibt zur Nachvollziehbarkeit stehen:*

**Faktisch entschieden, Deine Freigabe offen:**

| Ent | Gegenstand | Stand |
|---|---|---|
| **Ent-17** | Massnahme C71 | **L7** (L7a Kontext in `httpclient_stop()` freigeben, L7b Fehlerwege, L7c = C75). Die Ursache ist belegt (Messungen M1, M3, G9.6 Start 4; L468); alle anderen Kandidaten sind widerlegt oder ohne Bezug zur Ursache. Freigabe zusammen mit Ent-18 (A) bzw. am Tor (B) |

**Offen, neu — vor R2 (Nutzer):**

| Ent | Gegenstand | Optionen | Empfehlung |
|---|---|---|---|
| **Ent-18** | Wann L7 kommt | **(A)** eigenes kleines Release **R1b** vor R2; **(B)** in R3, wie der Schnitt aus Ent-9/Ent-16 vorsah; **(C)** zusammen mit R2 | **A** — L7 behebt einen Fehler im Bestand, den jeder Nutzer nach jedem Start trifft; die Änderung ist klein (eine Zeile in `httpclient_stop()`, zwei Fehlerwege, C75) und am Prüfstand c71e voll prüfbar; sie macht den K30-Ruhewert 5'120 erreichbar und nimmt die 1'704 Byte als Störgrösse aus den C72-Sitzungen und aus G10. Preis: ein OTA mehr für Dich und eine neue Ladeprobe-Grundlinie auf R1b. **B** hält am bisherigen Schnitt fest; jede C72-Sitzung muss dann den Client vorher freistellen. **C** verworfen: G10 soll die Wirkung des Caches getrennt zeigen (Ent-9) |

**Offen vor R2.1 (Lead):**

| Ent | Gegenstand | Empfehlung |
|---|---|---|
| **Ent-2** | Quelle des `ETag` | **T** (gzip-Trailer, je Anfrage aus der Datei, ohne Tabelle und Invalidierung); V als Rückfallebene (`design.md` §4.4) |

**Offen bis zum Tor (G10.6):**

| Ent | Gegenstand | Empfehlung | Wer |
|---|---|---|---|
| **Ent-8** | Eingriff C72 | nach `design.md` §5.4 auf Sitzungen **ohne** C71-Belegung: M1 ⇒ K2; M2 ⇒ K1 nur, wenn Δ`no_request_timeouts` den Anstieg trägt; M3 ⇒ K2, K1 unter derselben Bedingung; M4/M5 ⇒ kein Eingriff. Widerlegt die P1-Probe P1, ist K2 zwingend | Nutzer |
| **Ent-10** | schreibender Zugriff auf lwIP-Listen (für K2) | erlauben, nur mit K2, nur `callback_arg == NULL` | Nutzer |
| **Ent-14** | Frist in K2 | 5 s, endgültig mit den Ruhezeiten | Nutzer |
| **Ent-16** | Platz der C71-Massnahme | **entfällt mit Ent-18 A**; mit B: in R3 | Nutzer |
