# Anforderungen — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70 Asset-Cache, C71 Zerstückelung beim Start, C72 offen gehaltene Verbindungen)

**Status: freigegeben am 15.10.2026 bis aufs Tor; Nachträge A.2 bis A.4 zur Freigabe.** Entscheide des Nutzers und
Lead-Entscheide vom 15.10.2026 eingearbeitet. **Nachtrag A.2** (P1–P8), die **Trennmessung C71** (L468), **Nachtrag
A.3** (P7, P9, P11, P12) und **Nachtrag A.4** (Review R1.7, 16.10.2026) ändern I-5, I-6, die Entscheidungsregeln, die
Kandidaten für C71 und C72 und öffnen **Ent-2** neu; was sich ändert und was widerlegt ist, steht in `design.md` §17.
**Offen:** Ent-2 (Lead, vor R2.1), Ent-8, Ent-10, Ent-14, Ent-17 (am Tor, G10.6), Ent-16 (vor R3). **Vom Nutzer noch
nicht gesehen:** der C71-Teil, die Statuszeile `HTTP/1.1` (AKC.7) und die Nachträge A.2 bis A.4.

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge am 15. und 16.10.2026. Am Gerät laut
letztem Messprotokoll: STM 3.2.34, ESP 3.2.37, PWA 1.4.100 (`BEFUNDE.md`, L466) — Ausgangsstand, **kein** Sollwert; die
Versionen der Releases setzt der Lead über den `release-engineer` (R4).
**Auslöser:** Nutzerentscheid 15.10.2026 — dieses Paket als nächster Schritt nach Paket 2026-10-13 (E3, E4).
**Grundlage:** `BEFUNDE.md` C65, C70, C71, C72, L456, L457, L460, L461, L466, L468; `specs/paket-2026-10-13/`;
`TESTPLAN-PWA.md` K30 (7.1), Abschnitt 0 Punkt 5, 5b; `ERWEITERUNGEN.md` X1, X14, X16; `CLAUDE.md` R5/DIR-008 in der
Fassung vom 14.10.2026 (`CLAUDE.md:412-416`); Berichte des `firmware-analyst` zu A.1 und Review R1.7 des
`code-reviewer` (über den Lead).

**Zeilenangaben:** `Datei:Zeile`-Anker beziehen sich auf `72cfda7`. Seit R1.1 liegen die Zeilen in `http.cpp` ab rund
`:2839` um rund +210 später (Lead). **Massgeblich ist die genannte Funktion bzw. das Symbol.**

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen:** **✔ SW** — vom `spec-writer` am Code `72cfda7` bzw. an der Core-Quelle 3.1.2 nachgelesen; **✔ B** —
in `BEFUNDE.md` belegt; **✔ L** — vom Lead gemessen und gemeldet; **✔ FA** — vom `firmware-analyst` am Core belegt;
**◐ FA** — vom `firmware-analyst` als plausibel gemeldet; **? FA** — vom `firmware-analyst` ausdrücklich als
Spekulation gekennzeichnet; **✔ R** / **◐ R** — vom `code-reviewer` in R1.7 belegt bzw. als Lesart gemeldet; **◐** —
abgeleitet oder Annahme; **●** — gemeldet, vor der Umsetzung zu prüfen.

**Kennungen dieses Pakets:** Schritte **0**, **A**, **R1** (Instrumente I-5, I-6), **G9** (ohne Cache), **R2** (C70),
**G10** (mit Cache, **Tor**), **R3** (Eingriff C72, Massnahme C71), **G11**, **T3**, **Z**. Akzeptanzkriterien
**AKZ**, **AKI**, **AKM**, **AKC**, **AKW**, **AKE**, **AKS**. Entscheidungen **Ent-1 bis Ent-17**. `ETag`-Varianten
**T** und **V**. Kandidaten C72 **K1–K5**, C71 **L0–L7**; Ergebnisse C72 **M1–M5**, C71 **N0–N4** (N1a, N1b); Hypothese
**H-CC**; Lochsumme **`L = free − blk`** (`design.md` §3.3); Gerätenachweis **P1-Probe**; Prüfpunkte **P1–P12**, **P9b**;
Review-Punkte **H2**, **H4**. Die Nummern gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Nachweis |
|---|---|---|---|---|
| **0** | Ausgangswerte ohne Last, Werkzeuge `ladeprobe` und `browser-sitzung`; Trennmessung C71 auf 3.2.37 **erledigt** (L468) | — | — | Messprotokoll |
| **A** | P1–P8, P7, P11, P12 erledigt (A.2, A.3); offen P9 (H-CC), P9b | — | — | Bericht |
| **R1** | **I-5** laufende Verbindungszustände; Heap-Zeile mit `tcp=`/`nrt=`; **I-6** Startzustand, Heap im Startabgleich, Flash-Kennung, `.tmp`-Löschen | ESP | ESP-OTA | **G9** |
| **G9** | **Ausgangsstand ohne Cache:** Ladeprobe, **P1-Probe**, Browser-Sitzungen Safari (Lead, dazu einmal Nutzer) und Chrome, **Startmessung C71** mit vier Starts | — | — | Messprotokoll |
| **R2** | **C70** `ETag`/`304` (Variante nach Ent-2), Statuszeile `HTTP/1.1` für App-Assets, PWA-Teil | ESP **und** PWA | Firmware zuerst, dann PWA | **G10** |
| **G10** | **Mit Cache:** Ladeprobe, Probe U, Browser-Sitzungen erneut, Grundlinie der Verbindungsfehler; Zuordnung M1–M5, N0–N4; **Tor** | — | — | Ent-8, Ent-10, Ent-14, Ent-17 |
| **R3** | **Eingriff C72** (Ent-8) und **Massnahme C71** (Ent-17), Platz nach Ent-16 | ESP | ESP-OTA | **G11** |
| **T3** | `pwa-tester` Phasen 0–4 und 9 auf dem Endstand (DIR-012) | — | — | AKZ.6 |

---

## Problem

### 1. Ein volles Neuladen der PWA reisst die Haltegrenze K30 — mit nur einem Client (C65, C70)

**Befund am Gerät, ✔ B (L466, ESP 3.2.37, PWA 1.4.100):** In T2 fiel der Tiefstwert um 21:28:37 auf `low=752`
(Grenze 1'072), `max` 3'584, beim Neuladen der PWA in Englisch (K33) mit **einem** Client. E4 hatte das Tief unter
Last vorher auf 1'336–2'184 gehoben (G8, G7R, L457).

**Ursache der Last, ✔ SW:** Jedes Öffnen und Neuladen holt **alle** App-Assets vollständig vom ESP:

- `http_send_fs_file()` antwortet `HTTP/1.0 200 OK` mit `Cache-Control: no-cache` und `Connection: close`, ohne `ETag`
  oder `Last-Modified` (✔ L per `curl -D`). `If-None-Match` wird in `http_server_loop()` nicht ausgewertet. Einen
  `304`-Weg gibt es nicht.
- Über `http://` gibt es keinen Service Worker (`registerAppServiceWorker()` in `app.js`; `sw.js` nur unter https).
- Umfang je Öffnen rund 200 KB (C70, ✔ B), davon `app-app.js.gz` 163'621 Byte (E3.0, ✔ B).
- `layout-previews.json` wird in `loadLayoutPreviewRowsMap()` mit `cache: "no-store"` geholt (✔ SW).

**Befund am Prüfserver, ✔ L (Lead, 15.10.2026 22:21–22:23):** Mit `HTTP/1.0` sandte Chrome für das Dokument kein
`If-None-Match`, für `app.js` nur beim Neuladen — fast nur `200`. Mit `HTTP/1.1` (weiter `Content-Length`, `ETag`,
`no-cache`): jede warme Ladung mit `If-None-Match`, überall `304`, 290 statt 4'045 Byte. Lesart (◐): Chromium nimmt
einen `ETag` aus einer `HTTP/1.0`-Antwort nicht als Validator. **Safari ist ungeprüft.** **✔ FA:** Nur
`http_send_fs_file()` muss die Statuszeile ändern; die übrigen Antworten in `http_app()` bleiben `HTTP/1.0`; Keep-Alive
entsteht nicht.

### 2. Nach dem Start bleibt der grösste freie Block bei rund 4,7 KB (C71)

**Befund am Gerät, ✔ B (L460, L457 G8):** Nach dem OTA auf E4 stand `max` um 18:17:36 bei 7'184 und um 18:18:36 bei
4'672. Im Fenster lagen nur `upd-releasenote.txt geschrieben` und `upd-wc-list.txt geschrieben`, keine Anfrage.
Danach blieb der Ruhewert bei 4'672; nach G7R 4'464. Unter E3 dasselbe Bild schwächer (7'256 ⇒ 5'808, L450). K30
verlangt ≥ 5'120.

**Ablauf des Startabgleichs, ✔ SW:** `update_cache_poll()` läuft frühestens `UPDATE_CACHE_START_MS` = 60 s nach dem
Start und 5 s nach der letzten Anfrage, holt drei Versionszeilen und gleicht im ersten Durchgang nach **jedem** Start
beide Dateien ab (E4.5a). Je Datei öffnet `update_cache_write()` den Abruf, mountet LittleFS, liest die alte Datei,
schreibt gegebenenfalls eine Temporärdatei, benennt um, schliesst den Abruf und hängt aus.

**Trennmessung, ✔ L (Lead, ESP 3.2.37, L468):** Erstes Flash-Schreiben seit dem Start ohne Abruf (b) und Schreiben mit
Abruf (c) im laufenden Betrieb — beide ohne bleibende Spur. Mit V3 aus T2: **C71 entsteht nur, wenn beim Start
geschrieben wird** (◐). Der PUYA-Verdacht (K-a) ist nicht bestätigt; L468 (b) war ein gültiger Test (✔ FA).

**Hypothese H-CC, ? FA (A.3, Spekulation):** Die einzige belegte Asymmetrie zwischen Start und Betrieb ist der erste
`ClientContext` des statischen httpclient (`httpclient.cpp:16`, `hc=0`). Beim Start mit Schreiben trennt ein PCB aus
Schritt 4 in TIME_WAIT den freigegebenen Temporärbereich vom grossen Block; Best-Fit legt den `ClientContext` aus
Schritt 5 dorthin. Der Wetter-Abruf hat einen eigenen Client (`weather.cpp:16`), den `hc` nicht erfasst.

**C75, ✔ R (Review R1.7, neu):** `httpclient()` gibt bei leerer Antwort (`len == 0`) mit offenem `ClientContext` zurück
(`httpclient.cpp:220-228`); Aufrufer, die bei `len <= 0` abbrechen — darunter `update_cache_write()` —, schliessen
nicht. Ein Durchgang mit leerer Antwort misst also mit offenem Client.

**Warum 60 s, ✔ FA (P11):** Der STM-Vollabgleich nach `IPADDRESS` liefert Host und Pfad; ein blockierender Abruf darin
liesse den Empfangspuffer überlaufen (A79, L421). `http_server_begin()` steht in `wifi_connect()` **vor** `IPADDRESS`.

**Flash-Schreiben beim Start, ✔ FA (P12):** Ausser LittleFS praktisch nichts. Ohne Logzeile bleiben das Löschen
liegengebliebener `.tmp` und das AutoFormat in `LittleFS.begin()`.

### 3. Nach einer Safari-Sitzung bleibt der Heap niedrig, bis Safari beendet wird (C72)

**Befund am Gerät, ✔ B (L461):** Nach einer kurzen Safari-Sitzung fiel `free` von 6'536 auf 4'832, `max` von 4'672
auf 3'816 und blieb dort bis mindestens 20:06; `http_backlog` stand auf 1. Laut T2-Bericht Erholung um 20:09:37 ohne
Browser. **Beobachtung des Nutzers (●):** frei erst beim **Beenden** von Safari.

**Was belegt ist:**

- ✔ SW: Die PWA pausiert bei verborgener Seite. `http_server_loop()` nimmt je Durchlauf eine Verbindung an, wartet
  höchstens 250 ms auf die erste Zeile, zählt eine Verbindung ohne Anfrage (`no_request_timeouts`/`_aborts`) und
  beendet sie mit `stop()` ⇒ `tcp_close()` ⇒ FIN. Bei `!http_client` kehrt sie **ohne Zähler** zurück.
- ✔ FA (P7): Im Teilfall `no_request_aborts` hat der Core den PCB schon abgebaut. Nur im Teilfall
  `no_request_timeouts` bleibt nach `stop()` ein PCB in FIN_WAIT/TIME_WAIT. Eine zu späte Anfrage bekommt schon heute
  RST (◐ FA).
- ◐ FA (P1): FIN_WAIT_2 läuft rund 20 s nach dem letzten empfangenen Segment ohne RST ab; FIN_WAIT_1, CLOSING und
  LAST_ACK haben keine eigene Frist. **◐ R (H4):** TIME_WAIT endet über dieselbe `tmr`, also spätestens rund 120 s nach
  dem letzten Segment vor dem Schliessen.
- ✔ FA (P2): `MEMP_NUM_TCP_PCB` begrenzt bei `MEMP_MEM_MALLOC` nichts.

**Welcher Zustand den Heap bindet, ist nicht belegt.**

---

## Ziel

1. **C70:** Ein zweites Öffnen oder Neuladen in einem Browser, der die PWA schon geladen hat, holt **keinen
   Asset-Körper** mehr vom ESP, solange sich die Datei nicht geändert hat — in Chrome belegt, in Safari gemessen und
   berichtet; eine neu hochgeladene App-Datei wird beim nächsten Laden **sofort** ausgeliefert. Der warme Ladevorgang
   drückt `heap_low_block` nicht unter 1'072 Byte. **Nicht versprochen:** dass ein **kalter** Erstaufruf K30 hält.
2. **C71:** Der Ruhewert `max` liegt nach jedem Start — auch nach einem Start, bei dem der Abgleich schreibt — **und**
   nach Last bei mindestens 5'120 Byte (K30), sofern die Startmessung eine Ursache belegt (N0–N3). Belegt sie keine
   (N4), bleibt C71 offen mit Befund.
3. **C72:** Laufend ablesbar, **welche** Verbindungen der ESP hält. Zwei kontrollierte Messungen — ohne und mit
   Cache — und die P1-Probe ordnen den Befund aus L461 einem von M1–M5 zu. Erst danach und nur mit tragendem Ergebnis
   ein Eingriff; nach ihm bleibt der Heap nach einer Safari-Sitzung **ohne** Beenden von Safari nicht mehr messbar
   tiefer als vorher (AKE.2).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg der Meldung** (DIR-014). Prüfstände sind **einmal gegen
den Vorgängerstand fehlgeschlagen**, haben **je Massnahme mindestens eine Sabotage** und melden ihre Fallzahl (L235).
Jede Messung steht **sofort** im Messprotokoll (L424). Logzeilen im ESP in Umschrift (DIR-015).

**Haltegrenzen in den Messungen (Ent-7):** In G9, G10 und G11 **keine** — alle Ladungen werden gefahren und
protokolliert. Ein Neustart, eine Exception oder ein Watchdog-Reset ist ein **Ergebnis**: mit Uhrzeit, letzter
Anfrage, letzter Heap-Zeile und den I-4-/I-5-Feldern ins Messprotokoll, als Befund nachgeführt, dann Update-Quelle
und Versionen geprüft (DIR-009, L42).

**Flash-Schreiben seit dem Start / beim Start:** Zu jedem Ruhewert und jedem Tiefstwert in G9, G10 und G11 vermerkt der
Lead, ob seit dem letzten ESP-Start schon geschrieben wurde und ob **beim Start** — `ja` mit Beleg (`- update cache …
geschrieben`, `- update cache tmp geloescht …`, App-Upload), `nein` oder `unbekannt` (AutoFormat ohne Logzeile, P12).
Dazu: Wetter-Overlay im Startfenster ja/nein.

**Status `304` in CDP-Werkzeugen:** `Network.responseReceived` meldet bei einer `304`-Nachprüfung `200`; den
Leitungsstatus liefert nur `Network.responseReceivedExtraInfo.statusCode` (✔ L). Jedes Kriterium, das `304` zählt,
meint den Leitungsstatus.

### Schritt 0 — Grundlagen

- [ ] **AKZ.1 (Ausgangswerte ohne Last)** — Im Messprotokoll, bevor R1.1 beginnt: Grösse **jeder** der 13 Dateien
      aus `APP_INSTALL_ASSETS` (`/api/fs_list`); „Globale Variablen“ und Programmgrösse des Stands E4 (69'060 / 531'552
      Byte, L457); Kodierung **und** Zeilenende von `http.cpp`, `httpclient.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`,
      `base.h`, `app.js`; `netstat`, `lsof`, `osascript` vorhanden; der Prüfserver-Befund zu `HTTP/1.0`/`HTTP/1.1`; die
      Ergebnisse von A.1 (A.2, A.3).
- [ ] **AKZ.2 (Werkzeuge, DIR-014)** — (i) `ladeprobe`: dauerhaftes Profil, 1 × kalt, N × warm (Neuladen ohne
      `ignoreCache` und erneutes Öffnen); `device_ready?heap_low_reset=1` vor jedem Fenster, danach lesen; je Laden
      Zahl der `/app/`-Anfragen nach **Leitungsstatus**, ob `If-None-Match` gesendet wurde, Summe
      `encodedDataLength`, `heap_low_block`, `heap_low_path`, I-4-Felder, `app_not_modified`/`app_full`, soweit
      vorhanden. Gegenproben: Vorschau mit `--erwarte-304` ⇒ Exit ≠ 0 und Meldung „warm: 304 erwartet, 200
      erhalten“; Prüfserver `HTTP/1.1` mit `ETag` ⇒ OK; Prüfserver `HTTP/1.0` ⇒ FEHL. (ii) `browser-sitzung`: Phasen
      nach `design.md` §5.3 in den Betriebsarten `osascript` und **`manuell`**; je Phasenende `device_ready`,
      `netstat -anv -p tcp`, `lsof -nP -i @<uhr>`; für **jede eigene Lesung** lokaler Port und Ende ihres
      TIME_WAIT-Fensters; je Sitzung Δ`no_request_timeouts`, Δ`no_request_aborts`, Δ`http_accept_dead`; Gegenprobe: Uhr
      nicht erreichbar ⇒ Exit ≠ 0 mit Meldung. (iii) ab R1: `smoke-device.sh` verlangt die Felder aus AKI.1, ab R2 die
      aus AKC.4; setzt nichts zurück; Gegenprobe `SMOKE_DR_OVERRIDE`. (iv) ab R1: `watch-log.sh` wertet die Heap-Zeile
      mit Zusatz unverändert aus und führt die I-6-Zeilen als Hinweis; Gegenprobe über `WATCHLOG_QUELLE`
      (`low=752 tcp=…` ⇒ Alarm kommt an). (v) ab R2: `check-pwa` INFO „`/app/` 304 je Laden“ (Leitungsstatus), „Byte je
      Laden“ und je Lauf die Zahl der `net::ERR_CONNECTION_RESET` und `net::ERR_EMPTY_RESPONSE`; mit V `- app etag` als
      Hinweis. (vi) Werkzeuge und Prüfstände, die eine CRC des ESP nachrechnen (nur V), benutzen **CRC-32/MPEG-2**.
      (vii) Die Auswertung der Startmessung rechnet je Messpunkt **`L = free − blk`** aus `- update cache heap` und
      `free − max` aus den Heap-Zeilen. Jede Meldung erscheint in Konsole **und** Mitschrift.
- [x] **AKZ.9 (Trennmessung C71 auf 3.2.37)** — Erstes Flash-Schreiben seit dem Start ohne Abruf (b) und Schreiben mit
      Abruf (c), je mit Heap-Zeilen vorher und nachher, im laufenden Betrieb. **Erfüllt, Beleg `BEFUNDE.md` L468**
      (Lead, 15.10.2026 22:40–22:46): beide ohne bleibende Spur; K-a nicht bestätigt (Problem §2).

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
- [ ] **AKI.3 (I-6: Startzustand, Heap im Startabgleich, Flash-Kennung, C71)** — Je Durchgang von
      `update_cache_poll()`: (a) vor Schritt 1 — **nach** dem Löschen liegengebliebener `.tmp` — `- update cache vorher
      up=<s> free=<n>/<blk> frag=<n> tcp=<…> hc=<httpclient_connects>`; (b) am Ende (auch bei Abbruch)
      `- update cache heap v=<free>/<blk> 1=<free>/<blk> 2=… 3=… 4=… 5=… n=<free>/<blk>` (`-` für nicht gelaufen) —
      **die für C71 tragende Zeile**; (c) unmittelbar danach `- update cache nach frag=<v>/<1>/<2>/<3>/<4>/<5>/<n>
      tcp=<…> hc=<n> acc=<Δ>` — `frag` aus `ESP.getHeapStats()` (nur ergänzend, geringe Empfindlichkeit), `acc` = Δ
      eines neuen Zählers angenommener Verbindungen; `tcp=`/`hc` hier rund 10 ms nach `n` gemessen (H2). Dazu (d)
      einmal je Start beim ersten **lebenden** `accept()` — auch bei einer Vorrats-Verbindung — `- heap erste anfrage
      free=<n> max=<n>`; (e) einmal in `setup()`, vor WLAN und `EEPROM.begin()`, `- flash id=<hex6> vendor=<hex2>`;
      (f) wenn der erste Durchgang eine liegengebliebene Temporärdatei löscht, `- update cache tmp geloescht <datei>`.
      Je Zeile ≤ 119 Zeichen, einmal formatiert, Mitschnitt **und** Ring textgleich, kein `String`. Prüfstand **c71i**;
      Sabotagen „`vorher` nach Schritt 1“, „ein Schritt ohne Messwert“, „nur `blk` statt `free`/`blk`“, „`nach` ohne
      `hc`“, „`acc` nicht als Δ“, „`tmp geloescht` fehlt“, „nur `Serial`“ ⇒ je FEHL; gegen 3.2.37 ⇒ FEHL.
- [ ] **AKI.4 (keine Wirkung auf den Betrieb)** — I-5 und I-6 legen nichts auf den Heap und ändern kein Verhalten:
      c65a, c65b, c65i, c65k, c65m, c65n, c65o, c65w, c65z bleiben grün; Δ „Globale Variablen“ durch R1 ≤ 60 Byte
      (gerechnet rund 50).

### Schritt G9 — Ausgangsstand ohne Cache (auf R1)

- [ ] **AKM.1 (Einspielen R1)** — ESP-OTA; Smoketest samt Update-Quelle und neuen Feldern **ohne** FEHLT,
      `install-app.sh --check` vollständig, `check-pwa.sh` ohne FEHLT; Versionen gleich denen des Tags; die Zeile
      `- flash id …` steht im Mitschnitt, Hersteller ins Messprotokoll. Wache läuft vorher.
- [ ] **AKM.2 (Ladeprobe ohne Cache)** — Drei Läufe `ladeprobe` (je 1 kalt, 5 warm), einer in Englisch. Je Laden:
      Anfragen nach Leitungsstatus, Byte, `heap_low_block`, `heap_low_path`, I-4-Felder, Flash-Schreiben seit Start.
      Erwartet (◐): auch warm nur `200`. Vergleichswert für AKW.2.
- [ ] **AKM.3 (Browser-Sitzung Safari)** — Phasen S0–S4 nach `design.md` §5.3, Betriebsart `osascript` (Lead);
      **danach einmal** Betriebsart `manuell` auf dem Gerät des Nutzers aus L461. Je Phasenende: `free_heap`,
      `max_free_block`, `tcp_now_*`, `http_accept_dead`, `no_request_*`, Heap-Zeilen mit `tcp=` im Phasenfenster,
      Mac-Seite (`netstat`, `lsof`; beim Gerät des Nutzers nur die ESP-Seite), lokaler Port und TIME_WAIT-Ende jeder
      eigenen Lesung. **Je Sitzung:** Δ`no_request_timeouts` gegen Δ`no_request_aborts` und Δ`http_accept_dead` über
      S1–S3 — ist Δ`timeouts` ≈ 0, ist K1 für L461 wirkungslos.
- [ ] **AKM.4 (Browser-Sitzung Chrome)** — dieselben Phasen und Auswertungen, `osascript`, nach mindestens 300 s Ruhe.
- [ ] **AKM.5 (Startmessung C71 — der entscheidende Schritt)** — ESP-Starts ohne Browser nach `design.md` §5.5; **keine
      HTTP-Lesung, bis `- update cache heap …` erschienen ist, plus 130 s** (◐ H4: TIME_WAIT endet spätestens rund
      120 s nach dem letzten Segment vor `n`; zusätzlich muss die bewertete Heap-Zeile `tw=0` zeigen). Starts 1 und 2
      ohne Änderung, Start 3 mit erzwungenem Schreiben **beim Start**, **Start 4 (empfohlen)** wie 3 mit einem kalten
      `update_status` rund 50 s nach dem Start — als Abweichung gekennzeichnet. **Auswertung an der Lochsumme
      `L = free − blk`:** „zerstückelt“ heisst, an der Heap-Zeile ≥ 130 s nach `n` mit `tw=0` liegt `max` um ≥ 1'000
      unter `blk` bei `v`, `L` (`free − max`) um ≥ 1'000 über `L` bei `v`, und `free` ist bis auf höchstens 300 Byte zu
      `v` zurückgekehrt; `frag` wird berichtet, entscheidet nichts. Je Start im Messprotokoll: alle I-6-Zeilen, die
      Heap-Zeilen bis zur Bewertung, Flash-Schreiben beim Start, Wetter-Overlay ja/nein, der Vermerk H2 (`tcp=`/`hc` in
      `nach` rund 10 ms nach `n`), Update-Quelle danach. **Ein Start mit leerer Antwort oder `fehlgeschlagen abruf` misst
      mit offenem Client (C75)** — gekennzeichnet, nicht allein für N1a/N1b herangezogen, im Zweifel wiederholt.
      Zuordnung zu **einem** von N0, N1a, N1b, N2, N3, N4 mit den tragenden Werten; AKZ.9 steht daneben.
- [ ] **AKM.6 (Gesundheit G9)** — Exceptions, ungewollte Neustarts, `diag`-Lücken, `ovr` vollständig erfasst und als
      Ergebnis berichtet (Ent-7); `watch-log.sh --pruefe` im Bericht.
- [ ] **AKM.7 (P1-Probe)** — Nach `design.md` §5.6: ein Socket sendet `GET /api/device_ready`, liest die Antwort und
      schliesst **nicht**; aus einer zweiten Verbindung `device_ready` nach 10 s und nach 25 s. **Erwartet (P1):** nach
      10 s ein Eintrag FIN_WAIT_2 mit `arg` = 0, Gegenstelle Mac und dem lokalen Port der Probe; nach 25 s ist er weg;
      die Mac-Seite zeigt CLOSE_WAIT. **Ist er nach 25 s noch da, ist P1 widerlegt** — dann gilt K2 am Tor als zwingend
      (Ent-8). Ergebnis ins Messprotokoll, bevor die Browser-Sitzungen beginnen.

### Schritt R2 — C70 Asset-Cache (Variante (a), Ent-1)

- [ ] **AKC.1 (`ETag`)** — Jede `200`-Antwort auf ein Asset aus `APP_INSTALL_ASSETS`, das als `.gz` ausgeliefert wird,
      beginnt mit `HTTP/1.1 200 OK` (AKC.7) und trägt einen `ETag` nach Ent-2: **Variante T**
      `"<gz-grösse>-<trailer>"` (letzte 8 Byte der `.gz` als Hex) oder **Variante V** `"<grösse>-<crc32>"` (CRC über den
      ganzen gespeicherten Inhalt). Beginnt die Datei nicht mit `1f 8b`, **kein** `ETag` (nur T). `Content-Type`,
      `Content-Encoding`, `Content-Length`, `Cache-Control: no-cache`, `Connection: close` unverändert. Legacy,
      `/api/*`, Installationsseite und Assets ausserhalb der Weissliste tragen **kein** `ETag`.
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
      Grösse, und **jede** der 31 schreibenden LittleFS-Stellen (✔ FA) ruft danach `app_etag_invalidate()`; die CRC ist
      **CRC-32/MPEG-2** (`crc32.cpp:26-43`, ✔ SW). c70e zählt die Stellen selbst; Sabotage zusätzlich „eine
      Invalidierung entfernt“ ⇒ FEHL. In beiden Varianten gegen den Stand R1 ⇒ FEHL.
- [ ] **AKC.4 (Kosten und Zähler)** — **T:** je Antwort ein `seek` und 8 Byte Lesen mehr, im selben Mount und mit
      derselben geöffneten Datei; Mehrdauer je Anfrage im Messprotokoll (◐ ms). **V:** CRC nur bei Bedarf, je Berechnung
      eine Zeile `- app etag <datei> n=<byte> ms=<dauer>`, `app-app.js.gz` unter 2'000 ms (◐ FA 0,15–0,3 s). In beiden:
      kein Heap, kein `String`; `device_ready` gibt hinter den I-5-Feldern `app_not_modified`, `app_full` und (nur V)
      `app_etag_calc` aus — seit dem Start, sättigend.
- [ ] **AKC.5 (Invarianten)** — App-Assets weiter **ausschliesslich** als `.gz` mit Prüfung Grösse > 0
      (`http_fs_file_exists_and_nonempty()`); kein Plain-Fallback; `/app/?action=install` und Installationsseite
      unverändert; die Legacy-Oberfläche liefert wie vor R2 (Stichprobe `/`, `/legacy`, `/fs`, `/update` per `curl` gegen
      den Stand R1 bis auf Zeit- und Messwerte, Statuszeile `HTTP/1.0`, ohne `ETag`).
- [ ] **AKC.6 (PWA-Teil, Ent-3)** — `loadLayoutPreviewRowsMap()` holt `layout-previews.json` mit `cache: "no-cache"`
      statt `"no-store"`; sonst keine Änderung an `app.js`; in `sw.js` nur `CACHE_NAME`. Prüfstand **c70p**; Sabotage
      `"no-store"` ⇒ FEHL; gegen 1.4.100 ⇒ FEHL.
- [ ] **AKC.7 (Statuszeile `HTTP/1.1` nur für App-Assets)** — Nur die Kopfzeilen in `http_send_fs_file()` und die neue
      `304` tragen `HTTP/1.1`; **beide** senden `Connection: close`, der ESP schliesst danach wie heute. Die `200` trägt
      ein `Content-Length` gleich der gesendeten Körperlänge; ein Abbruch endet mit RST, nie regulär. Die übrigen
      Antworten in `http_app()`, Legacy und `/api/*` bleiben `HTTP/1.0`. Prüfstand **c70e** (statisch: `HTTP/1.1` an genau
      diesen zwei Stellen, beide mit `Connection: close`; dynamisch: Körperlänge = `Content-Length`); Sabotagen
      „`Connection: close` in der `304` entfernt“ und „`HTTP/1.1` in einer `/api/`-Antwort“ ⇒ je FEHL; Ladeprobe am
      Prüfserver mit `HTTP/1.0` ⇒ FEHL (AKZ.2 (i)).
- [ ] **AKC.8 (bestehende Prüfstände nachgezogen)** — c65b und c65w (rufen `http_send_fs_file()` auf und erwarten die
      Kopfzeilen in **einem** Schreibvorgang) und die LittleFS-Attrappe (`seek`, zweimaliges bzw. Trailer-Lesen) sind
      auf den Stand R2 nachgezogen und grün; ihre Fallzahl ist gegen den Stand vorher gezählt, keine Fälle verloren.

### Schritt G10 — mit Cache, und Tor (auf R2)

- [ ] **AKW.1 (Einspielen R2, DIR-002)** — **erst** ESP-OTA, Smoketest, `install-app.sh --check`; **dann**
      `install-app.sh` mit der neuen PWA, erneut `--check`; `check-pwa.sh` ohne FEHLT; beide Versionen gleich denen
      des Tags. Ob der Abgleich nach dem OTA beim Start geschrieben hat, steht im Messprotokoll.
- [ ] **AKW.2 (Ladeprobe mit Cache, Chrome)** — Drei Läufe wie AKM.2, `--erwarte-304`. Je **warmem** Laden (Neuladen
      **und** erneutes Öffnen): für **jede** `/app/`-Anfrage `If-None-Match` gesendet und Leitungsstatus `304`, das
      Dokument eingeschlossen; Summe `encodedDataLength` über `/app/` ≤ 3'000 Byte; Δ`app_full` = 0;
      `heap_low_block` im warmen Fenster ≥ 1'072. Kalt berichtet. Gegenüberstellung mit AKM.2.
- [ ] **AKW.3 (Probe U — Update sofort wirksam)** — Im dauerhaften Profil warm laden; `app/layout-previews.json` mit
      anderem Inhalt (Leerraum, neu gepackt) über `app_file_upload` hochladen; neu laden: genau diese Datei `200` mit
      **neuem** `ETag` und neuem Inhalt, alle anderen `304` (bei V: genau eine Zeile `- app etag …`). Original
      zurücklegen, neu laden: `200` mit dem ursprünglichen `ETag`; danach `install-app.sh --check` ohne Abweichung.
      Ohne Einzelfreigabe zulässig (Ent-15).
- [ ] **AKW.4 (Browser-Sitzungen mit Cache, Safari ungeprüft)** — Safari und Chrome wie AKM.3/AKM.4 (`osascript`,
      samt Δ`timeouts`/Δ`aborts`); vor S0 einmal öffnen und beenden. Zusätzlich je Phase `app_full`,
      `app_not_modified`. Für Safari der erste Nachweis, ob er bei `HTTP/1.1` mit `ETag` nachfragt; bleibt
      `app_not_modified` bei 0, ist das ein Befund. Je Phase Gegenüberstellung mit G9.
- [ ] **AKW.5 (Auswertung und Tor)** — C72 nach `design.md` §5.4 **einem** von M1–M5 zugeordnet — auf G10, G9 und die
      P1-Probe daneben, mit der Auswertung Δ`timeouts` gegen Δ`aborts`; C71 nach §5.5 **einem** von N0–N4, mit AKM.5,
      AKZ.9, P9 und P9b. Beides ins Messprotokoll, vom `doc-writer` nachgeführt (nicht gleichzeitig, R3). **Erst danach**
      legt der Lead Ent-8, Ent-10, Ent-14 und Ent-17 vor.
- [ ] **AKW.6 (Gesundheit und Grundlinie G10)** — wie AKM.6; dazu **drei `check-pwa.sh`** auf dem Stand R2 mit der Zahl
      der `net::ERR_CONNECTION_RESET` und `net::ERR_EMPTY_RESPONSE` je Lauf und Δ`write_lost_failed` als **Grundlinie**
      für AKE.3.

### Schritt R3 und G11 — Eingriff C72, Massnahme C71 (nach dem Tor)

AKE.* entfallen ohne C72-Eingriff (M4, M5 oder Ent-8 = keiner); AKS.* entfallen bei N4 oder ohne Massnahme nach
Ent-17; im Endstand dann „entfallen“ mit Grund. Entfallen beide, entfällt R3 samt G11.

- [ ] **AKE.1 (C72-Eingriff umgesetzt)** — Kandidat nach Ent-8 gemäss `design.md` §6. **K1:** im Zweig „keine
      Anfrage“ von `http_server_loop()` im Teilfall `no_request_timeouts` `abort()` statt `stop()`; der Teilfall
      `no_request_aborts` bleibt, wie er ist; beide Zähler unverändert; jeder andere Abbau bleibt. **K2:** höchstens
      einmal je Sekunde PCBs in `tcp_active_pcbs` mit `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING,
      LAST_ACK oder — bei `TF_CLOSEPEND` — ESTABLISHED und CLOSE_WAIT, deren Ruhezeit die Frist aus Ent-14
      überschreitet, mit `tcp_abort()` abbauen; PCBs mit `ClientContext` **oder** Listener (SYN_RCVD) **nie**; Zähler
      `tcp_reaped`. **K5:** `HTTP_TW_LIMIT` 1. Prüfstand **c72e**; Sabotagen (K1 `stop()` bzw. „auch im Teilfall
      Abbruch“; K2 „PCB mit `callback_arg` ≠ NULL abgebaut“, „SYN_RCVD abgebaut“, „Frist entfernt“; K5 „Grenze 2“) ⇒ je
      FEHL; gegen den Stand R2 ⇒ FEHL.
- [ ] **AKE.2 (C72-Wirkung, G11)** — Browser-Sitzung Safari wie AKW.4: 180 s nach Beginn von S2 **und** von S3 je
      `free_heap` ≥ `free_heap`(S0) − 300 Byte; in `tcp_now_list` kein PCB mit `arg` = 0 ausser TIME_WAIT und keiner mit
      Ruhezeit über der Frist aus Ent-14; Heap-Zeilen im TIME_WAIT-Fenster der eigenen Lesung gesondert gekennzeichnet.
      Daneben L461 (−1'700), AKM.3, AKW.4. Chrome berichtet.
- [ ] **AKE.3 (C72 ohne Regression, gegen die Grundlinie R2)** — drei `check-pwa.sh` ohne FEHLT; je Lauf
      `net::ERR_CONNECTION_RESET` **und** `net::ERR_EMPTY_RESPONSE` gezählt — Massstab: **nicht mehr als in der
      Grundlinie R2** (AKW.6), weil der Wettlauf C74 schon heute Resets erzeugt; Ladeprobe warm wie AKW.2;
      Δ`write_lost_failed` nicht über der Grundlinie; Smoketest ohne FEHLT.
- [ ] **AKS.1 (C71-Massnahme umgesetzt)** — Kandidat nach Ent-17 gemäss `design.md` §7 — **L7** bei N1a nach P9b:
      `httpclient_stop()` gibt den `ClientContext` des httpclient frei (`client = WiFiClient()`), **und C75**: jeder Weg,
      der nach `httpclient()` bei `len <= 0` aufgibt — `httpclient()` selbst bei leerer Antwort, `update_cache_write()`
      und die übrigen Aufrufer —, ruft `httpclient_stop()`; **L6** bei N1b (`UPDATE_CACHE_START_MS` auf 300 s); sonst
      L0, L2 (nur mit ihrer Vorbedingung) oder L4b —, begründet aus AKM.5, AKZ.9, P9 und P9b. Prüfstand **c71e** nach
      `design.md` §8; gegen den Stand R2 ⇒ FEHL.
- [ ] **AKS.2 (C71-Wirkung, G11)** — Ruhewert `max` der Heap-Zeile **≥ 5'120** (K30) (i) in jedem von drei ESP-Starts
      ohne Browser — **einer davon mit erzwungenem Schreiben beim Start** — an einer Heap-Zeile ≥ 130 s nach `n` mit
      `tw=0`, ohne HTTP-Lesung davor, und (ii) 150 s nach einem `check-pwa.sh` und nach einer Ladeprobe — je mit dem
      Vermerk „Flash-Schreiben beim Start“. Kein Start gilt nach der Regel aus AKM.5 als „zerstückelt“ (Lochsumme `L`).
      Daneben AKM.5, L460 (4'672), L457 (7'184), L468 (6'192).
- [ ] **AKS.3 (C71 ohne Regression)** — nach einem Start ohne Änderung je Datei `unveraendert`, nach dem erzwungenen
      Schreiben `geschrieben` und danach `unveraendert`; keine `*.tmp` (`/api/fs_list`); Release Notes in PWA und Legacy
      sichtbar; `install-app.sh --check` vollständig; c65m, c65n grün. **Mit L7:** jeder weitere Abruf des httpclient
      funktioniert nach dem Freigeben; nach einer leeren Antwort ist der Client geschlossen (C75, c71e). **Mit L6:** die
      vier Nebenwirkungen aus `design.md` §7 im Testplan beschrieben — `TESTPLAN-PWA.md` (die drei Stellen mit „rund
      60 s“) und c65n (Startfrist) nachgezogen.

### Übergreifend

- [ ] **AKZ.3 (Kodierung und Zeilenende)** — Vor jedem Patch festgestellt und im Taskbericht genannt (`http.cpp` UTF-8
      mit einer CRLF-Zeile, `stm32flash.cpp` UTF-8, `ESP-uclock.ino` gemischt, `httpclient.cpp` vor R3 festzustellen;
      DIR-015). Der Diff enthält nur inhaltlich geänderte Zeilen; der Lead prüft mit `git diff --stat`.
- [ ] **AKZ.4 (Release-Inhalt)** — R1 und R3 ändern nur `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino`, `version.h`; R2
      dazu `data/app/app.js` und `data/app/sw.js` (`CACHE_NAME`). **Keine** Änderung an `src/**`, `index.html`,
      `styles.css`, `manifest.webmanifest`. Geprüft gegen das jeweils vorige Tag.
- [ ] **AKZ.5 (Releases)** — **Jedes** Release einzeln (Ent-9): ein `make release-zip`, ein Commit, ein Tag nach
      DIR-011, Push samt Tag im selben Zug; Einspielzeilen mit Reihenfolge (Firmware zuerst, DIR-002),
      `install-app.sh --check` nach dem OTA (DIR-017) und Rückweg samt Befehl, **bevor** eingespielt wird.
- [ ] **AKZ.6 (T3, DIR-012)** — `pwa-tester` Phasen 0–4 und 9 auf dem Endstand. Haltegrenze **K30** nach Ent-12:
      Ruhewert `max` < 5'120 oder `low=` < 1'072 ⇒ anhalten; bei jedem Halt I-4- und I-5-Felder ohne Parameter in den
      Bericht, bevor zurückgesetzt wird. Uhr exklusiv, Beginn und Ende angekündigt. Abgenommen mit Bericht und
      Abdeckungszahl.
- [ ] **AKZ.7 (Guardrails und Prüfstände)** — Nach jedem Task `./tools/guardrails.sh` Exit 0; vor jedem Release
      `--full` Exit 0; die neuen Prüfstände in S16, Fallzahl nachgezählt (L235).
- [ ] **AKZ.8 (RAM)** — „Globale Variablen“ und Programmgrösse vor R1.1 und nach **jedem** ESP-Task sofort ins
      Messprotokoll; Zuwachs über das Paket höchstens **250 Byte** (Ent-11; gerechnet rund 100 mit Variante T, rund
      205 mit V, `design.md` §10); darüber anhalten und vorlegen. `app.js.gz` vor und nach R2.

**Zahl der Kriterien: 40** — AKZ 9 (davon AKZ.9 erfüllt, L468), AKI 4, AKM 7, AKC 8, AKW 6, AKE 3, AKS 3. Am Tor hängen
AKE.1–AKE.3 (Ent-8) und AKS.1–AKS.3 (Ent-17); an Ent-2 hängen die Varianten in AKC.1, AKC.3, AKC.4.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| Variante (b) und (c) für C70 | Ent-1 = (a); (c) nur nach G10.7 als neuer Nachtrag |
| `Last-Modified`/`If-Modified-Since` statt `ETag` | Rückfallebene nur, falls Safari nicht nachfragt (G10.7) |
| `HTTP/1.1` für Legacy, `/api/*`, Hinweisseiten; Keep-Alive, Chunked, Pipelining | nicht nötig (AKC.7) |
| Ein **kalter** Erstaufruf unter 1'072 | C70 ändert ihn nicht |
| Eine C71-Massnahme ohne belegte Ursache (N4) | Ziel 2 |
| `frag` als tragende Grösse für C71; `fe` aus dem privaten `umm_local.h` | zu unempfindlich bzw. privater Kopf (A.4); getragen wird von `L = free − blk` |
| L0 ohne `vendor=85`; L2 in der Form „vor `http_server_begin()`“ | L468 (b) bzw. nur `quelle-leer` (P11) |
| **C75** ohne L7 | wählt Ent-17 nicht L7, bleibt C75 ein eigener Befund (vom Lead eingetragen) |
| **C74** — Wettlauf in der Warteschlaufe auf die erste Zeile | eigener Befund; hier nur Grund für die Grundlinie in AKE.3 |
| **C67**, **C66**, **C68**, **C69** | eigene Befunde |
| Änderungen an `index.html`, `styles.css`, `manifest.webmanifest`, Icons | für (a) nicht nötig |
| Neuer Service Worker, Änderung an `sw.js` ausser `CACHE_NAME` | unter `http://` ohne Wirkung |
| `Cache-Control: max-age` ohne Versionierung | verletzt „Update sofort wirksam“ |
| Höchstzahl Verbindungen je Gegenstelle (K4) | Risiko für parallele Asset-Ladungen |
| Ein C72-Eingriff vor dem Tor | Mechanismus nicht belegt |
| Änderungen am Wetter-Client (`openweather_client`) | `hc` erfasst ihn nicht; er wird nur vermerkt |
| Änderung am Messzeitpunkt von `tcp=`/`hc` in `- update cache nach` (H2) | vermerkt, nicht geändert |
| Änderungen an der Legacy-Oberfläche, am STM | Invariante bzw. nicht betroffen |
| Browser steuern, Schutz für `app_file_upload` (C61), Backlog-Schwellen aus E4 | nicht möglich bzw. eigene Specs/Tasks |
| L462 (Laden schreibt `ambilight_online`, B61) | eigener Befund |
| Nebenbefunde aus A.1: `LittleFS.remove()` vor `LittleFS.begin()` in `http_api_app_file_upload()`; `LittleFS.format()` in `setup()` in `#if 0` | eigene Befundzeilen (Task R1.10) |

---

## Betroffene Laufzeiten

- [ ] **ESP8266** — R1: `http.cpp`, `base.h`, `ESP-uclock.ino`; R2: `http.cpp`, mit V auch `stm32flash.cpp` und
      `ESP-uclock.ino`, `base.h`; R3: `http.cpp`, `base.h`, mit L7 `httpclient.cpp`, gegebenenfalls `ESP-uclock.ino`;
      je ein OTA
- [ ] **PWA** — R2: `app.js` (eine Zeile), `sw.js` (`CACHE_NAME`); ein Upload nach dem OTA
- [ ] **STM32** — keine
- [ ] **Werkzeug** — `ladeprobe` (liegt vor), `browser-sitzung` (mit `manuell`), P1-Probe, Auswertung der Lochsumme;
      geändert `check-pwa`, `smoke-device.sh`, `watch-log.sh`; Prüfstände c72i, c71i, c70e, c70p, c72e, c71e;
      nachgezogen c65b, c65w, LittleFS-Attrappe, mit L6 c65n
- [ ] **Dokumentation** — `BEFUNDE.md`, `TESTPLAN-PWA.md` (mit L6 die drei Stellen mit „rund 60 s“), `CHANGELOG.md`
- [ ] **Build/Release** — drei Releases R1, R2, R3 (R3 nur nach dem Tor; R4 nur mit Ent-16 B)

---

## Entscheidungen

**Entschieden am 15.10.2026 vom Nutzer** (über den Lead übermittelt):

| Ent | Gegenstand | Entscheid | Folge |
|---|---|---|---|
| **Ent-1** | Variante C70 | **(a)** `ETag` + `304`; (c) nur nach G10.7 | AKC.* |
| **Ent-3** | PWA-Teil | **immer**; R2 mit PWA-Anteil, Firmware zuerst | AKC.6, AKW.1 |
| **Ent-6** | Bedienung der Browser | **C:** Lead per `osascript`, einmal der Nutzer (`manuell`) | AKM.3 |
| **Ent-7** | Haltegrenze in den Messungen | **keine**; Neustarts als Ergebnis | Kopf, AKM.6, AKW.6 |
| **Ent-9** | Releaseschnitt | **drei Releases** R1 ⇒ G9 ⇒ R2 ⇒ G10 ⇒ Tor ⇒ R3 | AKZ.5 |
| **Ent-12** | Ruhewert-Grenze in T3 | **5'120** | AKZ.6 |
| **Ent-13** | C71 | **mitnehmen** | AKI.3, AKM.5, AKS.* |

**Lead-Entscheide 15.10.2026:** **Ent-4** `tcp=`/`nrt=` in der Heap-Zeile; **Ent-5** nur letztes Oktett; **Ent-11**
250 Byte. **Ent-2** war als „CRC-32 + Grösse“ übernommen und ist mit Nachtrag A.2 **wieder offen** (unten).

**Entfallen:** **Ent-15** (schreibende Schritte in der Messung) — `CLAUDE.md` R5 und DIR-008 am 14.10.2026 geändert
(Commit `e7c5d63`, `CLAUDE.md:412-416`: „Schreibende Prüfungen, Flashen, Resets und Messungen laufen ohne
Einzelfreigabe, auch nachts.“), vom `spec-writer` nachgelesen. Probe U, die ESP-Starts und der kalte `update_status`
in Start 4 bleiben drin. Es bleibt **eine** Messung zur Zeit; die Gefahrentabelle gilt.

**Offen, vor R2.1:**

| Ent | Gegenstand | Optionen | Bewertung und Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-2** | Quelle des `ETag` | **T** — `<gz-grösse>-<gzip-trailer>`, je Anfrage aus der Datei; **V** — `<grösse>-<CRC-32/MPEG-2>` über den ganzen Inhalt, RAM-Tabelle mit Invalidierung an 31 Stellen | **T** (`design.md` §4.4): Der gzip-Trailer **ist** eine CRC über den entpackten Inhalt — für jede gültige `.gz` so trennscharf wie V. Gelesen bei jeder Anfrage (ein `seek`, 8 Byte), entfallen Tabelle, Invalidierung, Zeile `- app etag` und 104 Byte RAM — und mit ihnen das Restrisiko einer übersehenen Schreibstelle. Eine ungültige `.gz` bekommt keinen `ETag` (sichere Richtung). V bleibt Rückfallebene, falls das `seek` messbar kostet (AKC.4) | Lead |

**Offen bis zum Tor (G10.6):**

| Ent | Gegenstand | Optionen | Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-8** | Eingriff C72 | K1, K2, K1 + K2, K5 als Zusatz, kein Eingriff | nach `design.md` §5.4: M1 ⇒ K2; M2 ⇒ K1 **nur, wenn Δ`no_request_timeouts` den Anstieg trägt**; M3 ⇒ K2, K1 unter derselben Bedingung; M4/M5 ⇒ kein Eingriff. **Widerlegt die P1-Probe P1, ist K2 zwingend** | Nutzer |
| **Ent-10** | schreibender Zugriff auf lwIP-Listen (für K2) | erlauben; nicht | **erlauben, nur mit K2, nur PCBs mit `callback_arg == NULL`** | Nutzer |
| **Ent-14** | Frist in K2 | 5 s; 20 s | **5 s**, endgültig mit den Ruhezeiten aus G9/G10 | Nutzer |
| **Ent-17** | Massnahme C71 | L7 (mit C75), L6, L0, L2 (mit Vorbedingung), L4b, keine | nach `design.md` §5.5: **N1a** ⇒ **L7 mit C75** nach P9b; **N1b** ⇒ **L6** mit seinen vier Nebenwirkungen; N0 ⇒ L0; N2 ⇒ L2 nur mit Vorbedingung; N3 ⇒ L4b; N4 ⇒ keine | Nutzer |

**Offen vor R3:**

| Ent | Gegenstand | Optionen | Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-16** | Platz der C71-Massnahme | (A) in R3 mit dem C72-Eingriff; (B) eigenes R4; (C) in R2 | **A** — getrennte Messgrössen (C71: Ruhewert nach Start ohne Browser; C72: `free`-Lücke in einer Sitzung), ein OTA weniger. **B**, wenn ein getrennter Rückweg wichtiger ist. **C** verworfen: G10 soll den Cache getrennt zeigen | Nutzer |
