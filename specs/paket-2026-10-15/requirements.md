# Anforderungen — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70 Asset-Cache, C71 Zerstückelung beim Start, C72 offen gehaltene Verbindungen)

**Status: freigegeben am 15.10.2026 bis aufs Tor** — Entscheide des Nutzers vom 15.10.2026 (über den Lead
übermittelt) und Lead-Entscheide eingearbeitet. **Offen:** Ent-8, Ent-10, Ent-14, Ent-17 (am Tor, Task G10.6) und
**Ent-16** (neu, Platz von C71 im Releaseschnitt, vor R3). **Neu und vom Nutzer noch nicht gesehen:** der C71-Teil
(auf den Nutzerentscheid Ent-13 hin geplant: Instrument I-6, Startmessung, Kandidaten L1–L5) und die Statuszeile
`HTTP/1.1` für App-Assets (AKC.7, nach dem Befund des Leads vom 15.10.2026 22:21–22:23). Der Lead legt beides mit
Ent-16 vor.

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge mit den Entscheiden und dem
Prüfserver-Befund am selben Tag. Am Gerät laut letztem Messprotokoll: STM 3.2.34, ESP 3.2.37, PWA 1.4.100
(`BEFUNDE.md`, L466) — Ausgangsstand, **kein** Sollwert; die Versionen der Releases setzt der Lead über den
`release-engineer` (R4).
**Auslöser:** Nutzerentscheid 15.10.2026 — dieses Paket als nächster Schritt nach Paket 2026-10-13 (E3, E4).
**Grundlage:** `BEFUNDE.md` C65, C70, C71, C72, L456, L457, L460, L461, L466; `specs/paket-2026-10-13/`
(`design-e4.md` §19, `requirements-e4.md`, `endstand.md`); `TESTPLAN-PWA.md` K30 (7.1), Abschnitt 0 Punkt 5, 5b;
`ERWEITERUNGEN.md` X1, X14, X16 (nennen C65/C70 als Voraussetzung); `CLAUDE.md` R5/DIR-008 in der Fassung vom
14.10.2026 (Test-Uhr, `CLAUDE.md:412-416`).

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen:** **✔ SW** — vom `spec-writer` am Code `72cfda7` bzw. an der Core-Quelle 3.1.2
(`~/Library/Arduino15/packages/esp8266/hardware/esp8266/3.1.2/`) nachgelesen; **✔ B** — in `BEFUNDE.md` belegt;
**✔ L** — vom Lead gemessen und dem `spec-writer` gemeldet, noch nicht in `BEFUNDE.md`; **◐** — abgeleitet oder
Annahme, nicht belegt; **●** — gemeldet, vor der Umsetzung zu prüfen.

**Kennungen dieses Pakets:** Schritte **0** (Grundlagen), **A** (Analyse), **R1** (Release: Instrumente I-5, I-6),
**G9** (Messung ohne Cache), **R2** (Release: C70), **G10** (Messung mit Cache, **Tor**), **R3** (Release: Eingriff
C72 und Massnahme C71, nach dem Tor), **G11**, **T3** (`pwa-tester`), **Z**. Akzeptanzkriterien **AKZ**
(übergreifend), **AKI** (Instrumente), **AKM** (Messung G9), **AKC** (C70), **AKW** (Messung G10 und Tor), **AKE**
(Eingriff C72), **AKS** (Massnahme C71). Entscheidungen **Ent-1 bis Ent-17**. Kandidaten C72 **K1–K5**, C71
**L1–L5**; Ergebnisse der Entscheidungsregeln C72 **M1–M5**, C71 **N1–N4** (`design.md` §5.4, §5.5). Prüfpunkte am
Core **P1–P11** (`design.md` §12). Die Nummern gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Nachweis |
|---|---|---|---|---|
| **0** | Ausgangswerte ohne Last, Werkzeuge `ladeprobe` und `browser-sitzung` | — | — | Messprotokoll |
| **A** | Prüfpunkte P1–P11 (C72, C70, **C71**) am Core und am Code (`firmware-analyst`, lesend) | — | — | Bericht |
| **R1** | **I-5** laufende Verbindungszustände, Heap-Zeile mit `tcp=`/`nrt=`; **I-6** Heap im Startabgleich | ESP | ESP-OTA | **G9** |
| **G9** | **Ausgangsstand ohne Cache:** Ladeprobe, Browser-Sitzungen Safari (Lead, dazu einmal Nutzer) und Chrome, Startmessung C71 | — | — | Messprotokoll |
| **R2** | **C70** `ETag`/`304` mit Statuszeile `HTTP/1.1` für App-Assets, PWA-Teil (`layout-previews` mit `no-cache`) | ESP **und** PWA | Firmware zuerst, dann PWA | **G10** |
| **G10** | **Mit Cache:** Ladeprobe, Probe U, Browser-Sitzungen erneut; Zuordnung M1–M5; **Tor** | — | — | Ent-8, Ent-10, Ent-14, Ent-17 |
| **R3** | **Eingriff C72** (Ent-8) und **Massnahme C71** (Ent-17), Platz nach Ent-16 | ESP | ESP-OTA | **G11** |
| **T3** | `pwa-tester` Phasen 0–4 und 9 auf dem Endstand (DIR-012) | — | — | AKZ.6 |

---

## Problem

### 1. Ein volles Neuladen der PWA reisst die Haltegrenze K30 — mit nur einem Client (C65, C70)

**Befund am Gerät, ✔ B (L466, ESP 3.2.37, PWA 1.4.100):** In T2 fiel der Tiefstwert um 21:28:37 auf `low=752`
(Grenze 1'072), `max` 3'584, beim Neuladen der PWA in Englisch (K33) mit **einem** Client. E4 hatte das Tief unter
Last vorher auf 1'336–2'184 gehoben (G8, G7R, L457). Die I-4-Momentaufnahme zu diesem Tief ging verloren, weil V3
die Felder zurücksetzte (L466).

**Ursache der Last, ✔ SW:** Jedes Öffnen und jedes Neuladen holt **alle** App-Assets vollständig vom ESP:

- Die Dateiauslieferung antwortet `HTTP/1.0 200 OK` mit `Cache-Control: no-cache` und `Connection: close`, aber
  ohne `ETag` oder `Last-Modified` (`ESP8266/ESP-uclock/http.cpp:2067-2078`; vom Lead am 15.10.2026 per `curl -D`
  bestätigt, ✔ L). `If-None-Match` wird nicht ausgewertet: Der Server liest von den Kopfzeilen nur `User-Agent` und
  `Sec-Fetch-Dest` (`http.cpp:16783-16802`). Einen `304`-Weg gibt es nicht.
- Über `http://` gibt es keinen Service Worker (`data/app/app.js:2135-2163`); der Precache in `sw.js` greift nur
  unter https (`data/app/sw.js:77-101`).
- Umfang je Öffnen: rund 200 KB (C70, ✔ B), davon `app-app.js.gz` 163'621 Byte (Messprotokoll E3, E3.0, ✔ B). Die
  übrigen Grössen sind **nicht** einzeln belegt; Schritt 0 misst sie (AKZ.1).
- ✔ SW: `layout-previews.json` wird mit `cache: "no-store"` geholt (`app.js:11655`) — dort brächte selbst ein
  Validator nichts. Die Sprachdatei trägt schon `?v=<APP_VERSION>` und `cache: "default"` (`app.js:1744-1745`).

**Befund am Prüfserver, ✔ L (Lead, 15.10.2026 22:21–22:23):** Ein `ETag` allein genügt bei einer
`HTTP/1.0`-Antwort **nicht**. Ein Prüfserver mit `ETag` und `no-cache`, aber Statuszeile `HTTP/1.0`: Chrome (headless,
dauerhaftes Profil) sandte beim Neuladen für das Dokument `/app/` **kein** `If-None-Match`, für `app.js` nur beim
Neuladen, beim erneuten Öffnen gar nicht — fast nur `200`. Derselbe Server mit `HTTP/1.1` (weiter `Content-Length`,
`ETag`, `no-cache`): jede warme Ladung mit `If-None-Match` für Dokument und Asset, überall `304`, 290 statt 4'045 Byte.
**Lesart (◐):** Chromium nimmt einen `ETag` aus einer `HTTP/1.0`-Antwort nicht als Validator; am Prüfserver belegt, im
Chromium-Quelltext nicht nachgelesen. **Safari ist ungeprüft** — das gehört in die Messung (AKW.4).

### 2. Nach dem Start bleibt der grösste freie Block bei rund 4,7 KB (C71)

**Befund am Gerät, ✔ B (L460, L457 G8):** Nach dem OTA auf E4 stand `max` um 18:17:36 bei 7'184 und um 18:18:36 bei
4'672 (`low=` 3'768). Im Fenster lagen nur `upd-releasenote.txt geschrieben` (18:18:26) und
`upd-wc-list.txt geschrieben` (18:18:28), keine Anfrage. Danach blieb der Ruhewert bei 4'672, auch nach drei
Ladungen; nach G7R 4'464. Unter E3 dasselbe Bild schwächer (7'256 ⇒ 5'808, L450). Haltegrenze K30 verlangt ≥ 5'120.

**Was belegt ist, ✔ SW:** Der Startabgleich (`update_cache_poll()`, Kopfkommentar `http.cpp:13338-13388`) läuft
frühestens 60 s nach dem Start und 5 s nach der letzten Anfrage, holt drei Versionszeilen und gleicht im ersten
Durchgang nach **jedem** Start beide Dateien ab (E4.5a, `:13359-13367`). Je Datei: Abruf vom Update-Server öffnen
(`:13595`), **dann** `LittleFS.begin()` (`:13614`), alte Datei lesen, gegebenenfalls Temporärdatei schreiben —
dabei bis zu zwei `File`-Objekte gleichzeitig offen (`:13677`, `:13699`) —, umbenennen, `httpclient_stop()` und
`LittleFS.end()` (`:13770-13771`). Mount, Dateien und ausgehende Verbindung liegen also gleichzeitig auf dem Heap.

**Nicht belegt:** **welche** Belegung das Fenster überlebt und den Block teilt. Die Korrelation mit dem Abgleich ist
zeitlich, nicht ursächlich; ob es beim Abgleich ohne Schreiben (`unveraendert`) ebenso geschieht, ist nicht gemessen.

### 3. Nach einer Safari-Sitzung bleibt der Heap niedrig, bis Safari beendet wird (C72)

**Befund am Gerät, ✔ B (L461):** Nach einer kurzen Safari-Sitzung (19:59:16–20:00:09, ESP 3.2.37) fiel `free` von
6'536 auf 4'832, `max` von 4'672 auf 3'816 und blieb dort bis mindestens 20:06; `http_backlog` stand auf 1. Laut
T2-Bericht erholte sich der Wert um 20:09:37 ohne Browser. **Beobachtung des Nutzers (● — nicht gemessen):** frei
wurde der Heap erst, als er Safari **beendete**. Welche der beiden Angaben den Mechanismus trägt, klärt erst die
Messung.

**Was belegt ist, ✔ SW:** Die PWA pausiert ihre Abfragen bei verborgener Seite (`app.js:2079-2101`). Der Server
nimmt je Durchlauf **eine** Verbindung an, wartet höchstens 250 ms auf die erste Zeile (`http.cpp:239`,
`:16660-16679`), zählt eine Verbindung ohne Anfrage (`no_request_timeouts`/`no_request_aborts`) und beendet sie mit
`stop()` (`:16681-16710`), ebenso nach jeder Antwort (`:16844-16851`); `stop()` endet in `tcp_close()`
(`ClientContext.h:73-92`), also mit FIN. Danach FIN_WAIT_1/FIN_WAIT_2 bis zur Antwort der Gegenstelle, dann bis 120 s
TIME_WAIT (`lwip/priv/tcp_priv.h:137`, seit E4 auf 2 begrenzt, `http.cpp:464-470`). Für FIN_WAIT_2 sieht lwIP 20 s vor
(`tcp_priv.h:131`) — ob die binäre Bibliothek das so übersetzt, ist **◐** (P1). I-4 zählt Verbindungszustände **nur**
beim neuen Block-Tiefstwert (`http.cpp:473-530`), nicht laufend.

**Welcher Zustand den Heap bindet, ist nicht belegt.** Kandidaten: nie abgeholte Verbindungen, vom ESP geschlossene
ohne Antwort der Gegenstelle, ein Strom kurzer Vorab-Verbindungen, Empfangspuffer — oder gar keine Verbindung.

---

## Ziel

1. **C70:** Ein zweites Öffnen oder Neuladen in einem Browser, der die PWA schon geladen hat, holt **keinen
   Asset-Körper** mehr vom ESP, solange sich die Datei nicht geändert hat — in Chrome belegt, in Safari gemessen und
   berichtet; eine neu hochgeladene App-Datei wird beim nächsten Laden **sofort** ausgeliefert. Der warme Ladevorgang
   drückt `heap_low_block` nicht unter 1'072 Byte. **Nicht versprochen:** dass ein **kalter** Erstaufruf K30 hält —
   er wird gemessen und berichtet.
2. **C71:** Der Ruhewert `max` liegt nach jedem Start **und** nach Last bei mindestens 5'120 Byte (K30) — sofern die
   Startmessung eine Ursache im Startabgleich belegt (N1–N3). Belegt sie keine (N4), bleibt C71 offen mit Befund;
   ein Eingriff ohne Ursache findet nicht statt.
3. **C72:** Laufend ablesbar, **welche** Verbindungen der ESP hält. Zwei kontrollierte Messungen — ohne und mit
   Cache — ordnen den Befund aus L461 einem von M1–M5 zu. Erst danach und nur mit tragendem Ergebnis ein Eingriff;
   nach ihm bleibt der Heap nach einer Safari-Sitzung **ohne** Beenden von Safari nicht mehr messbar tiefer als
   vorher (AKE.2).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg der Meldung** (DIR-014). Prüfstände sind **einmal gegen
den Vorgängerstand fehlgeschlagen**, haben **je Massnahme mindestens eine Sabotage**, die anschlägt, und melden ihre
Fallzahl, die gegen das gezählt wird, was es geben müsste (L235). Jede Messung steht **sofort** im Messprotokoll
dieses Pakets in `BEFUNDE.md` (L424). Logzeilen im ESP in Umschrift (DIR-015).

**Haltegrenzen in den Messungen (Ent-7):** In G9, G10 und G11 gibt es **keine** Haltegrenze — alle Ladungen werden
gefahren und protokolliert. Ein Neustart, eine Exception oder ein Watchdog-Reset ist ein **Ergebnis**: mit Uhrzeit,
letzter Anfrage, letzter Heap-Zeile und den I-4-/I-5-Feldern ins Messprotokoll, als Befund nachgeführt, und vor dem
nächsten Schritt Update-Quelle und Versionen geprüft (DIR-009, L42). Übergangen wird keiner.

**Status `304` in CDP-Werkzeugen:** `Network.responseReceived` meldet bei einer `304`-Nachprüfung den Status der
Cache-Kopie, also `200`; den Status auf der Leitung liefert nur `Network.responseReceivedExtraInfo.statusCode` (✔ L,
`tools/ladeprobe.mjs` nutzt das). Jedes Kriterium, das `304` je Laden zählt, meint den Leitungsstatus.

### Schritt 0 — Grundlagen

- [ ] **AKZ.1 (Ausgangswerte ohne Last)** — Im Messprotokoll, bevor R1.1 beginnt: Grösse **jeder** der 13 Dateien
      aus `APP_INSTALL_ASSETS` im LittleFS (`/api/fs_list`, Name und Byte, Summe); „Globale Variablen“ und
      Programmgrösse des Stands E4 (Bezug: 69'060 / 531'552 Byte, L457); Kodierung **und** Zeilenende von
      `http.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`, `base.h`, `app.js` (AKZ.3); Verfügbarkeit von `netstat`,
      `lsof`, `osascript` auf dem Mac; der Prüfserver-Befund zu `HTTP/1.0`/`HTTP/1.1` (Problem §1) als Messzeile.
- [ ] **AKZ.2 (Werkzeuge, DIR-014)** — (i) `ladeprobe`: lädt die PWA in einem **dauerhaften** Profil, einmal kalt
      (Profil leer) und danach N-mal warm (Neuladen ohne `ignoreCache` und erneutes Öffnen); setzt vor jedem
      Fenster `device_ready?heap_low_reset=1` und liest danach; gibt je Laden aus: Zahl der `/app/`-Anfragen, je
      **Leitungsstatus** (`responseReceivedExtraInfo`), ob `If-None-Match` gesendet wurde, Summe
      `encodedDataLength`, `heap_low_block`, `heap_low_path`, I-4-Felder und, soweit vorhanden,
      `app_not_modified`/`app_full`. Gegenproben: gegen die Vorschau (`tools/preview/server.py`, ohne `ETag`) mit
      `--erwarte-304` ⇒ Exit ≠ 0 und Meldung „warm: 304 erwartet, 200 erhalten“; gegen einen Prüfserver mit
      `HTTP/1.1`, `ETag` und `304` ⇒ OK; gegen denselben Prüfserver mit `HTTP/1.0` ⇒ FEHL (die Lage aus Problem §1).
      (ii) `browser-sitzung`: Phasen nach `design.md` §5.3, liest je Phasenende `device_ready`,
      `netstat -anv -p tcp` und `lsof -nP -i @<uhr>`; Gegenprobe: Uhr nicht erreichbar ⇒ Exit ≠ 0 mit Meldung.
      (iii) ab R1: `smoke-device.sh` verlangt die Felder aus AKI.1, ab R2 zusätzlich die aus AKC.4; setzt nichts
      zurück; Gegenprobe über `SMOKE_DR_OVERRIDE`. (iv) ab R1: `watch-log.sh` wertet die Heap-Zeile mit Zusatz
      unverändert aus (`free=`, `max=`, `ovr=`, `low=`) und führt `- update cache heap` als Hinweis; Gegenprobe über
      `WATCHLOG_QUELLE` (`low=752 tcp=…` ⇒ Alarm kommt an). (v) ab R2: `check-pwa` INFO „`/app/` 304 je Laden“
      (Leitungsstatus) und „Byte je Laden“; `watch-log.sh` führt `- app etag` als Hinweis. Jede Meldung erscheint in
      Konsole **und** Mitschrift.

### Schritt R1 — Instrumente

- [ ] **AKI.1 (I-5: laufende Verbindungszustände in `device_ready`)** — Hinter `api_orphan_skipped` (bisher letztes
      Feld): `tcp_now_states` (11 Zahlen, Index = lwIP `enum tcp_state`, Zählung über `tcp_active_pcbs` in diesem
      Augenblick, die eigene Verbindung eingeschlossen), `tcp_now_tw`, `tcp_now_pending` (`WcHttpServer::pending()`),
      `tcp_now_self_port`, `tcp_now_list` (höchstens 8 Einträge, je `[zustand, gegenstelle, port, ruhe_s, nrtx,
      eigen]`, erst aktive, dann TIME_WAIT; `gegenstelle` = letztes Oktett nach Ent-5; `ruhe_s` = Sekunden seit dem
      letzten Setzen von `pcb->tmr`; `eigen` = 1, solange ein `ClientContext` am PCB hängt), `tcp_now_more`. Listen
      **vollständig** in einen Stackpuffer, **bevor** das erste `http_send()` läuft; kein Heap, kein `String`, kein
      `yield()` dabei. Bestehende Felder unverändert; nichts wird zurückgesetzt. Prüfstand **c72i**; Sabotagen
      „Ausgabe innerhalb der Listenschleife“ und „Kappung entfernt“ ⇒ je FEHL; gegen 3.2.37 ⇒ FEHL.
- [ ] **AKI.2 (I-5: Heap-Zeile, Ent-4)** — Die Heap-Zeile (`ESP-uclock.ino:601-656`) endet auf
      ` tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>`; einmal formatiert, in Mitschnitt **und**
      `/api/stm32_log` textgleich, ≤ 119 Zeichen. Zählung in `http.cpp` (privater lwIP-Kopf nur dort), Prototyp in
      `base.h`. Prüfstand **c72i**; Sabotage „nur `Serial`“ ⇒ FEHL.
- [ ] **AKI.3 (I-6: Heap im Startabgleich, C71)** — (a) Am Ende jedes Durchgangs von `update_cache_poll()` (auch bei
      Abbruch) genau eine Zeile
      `- update cache heap vor=<free>/<blk> s1=<blk> s2=<blk> s3=<blk> s4=<blk> s5=<blk> nach=<free>/<blk>`
      (`vor` vor Schritt 1, `s<n>` = grösster Block unmittelbar nach Schritt n, `-` für nicht gelaufen, `nach` nach
      dem letzten Schritt); (b) einmal je Start beim **ersten** angenommenen Anfrage-Eingang vor jeder Verarbeitung
      `- heap erste anfrage free=<n> max=<n>`. Beide einmal formatiert, Mitschnitt **und** Ring textgleich,
      ≤ 119 Zeichen, kein `String`. Prüfstand **c71i**; Sabotagen „ein Schritt ohne Messwert“ und „nur `Serial`“ ⇒
      je FEHL; gegen 3.2.37 ⇒ FEHL.
- [ ] **AKI.4 (keine Wirkung auf den Betrieb)** — I-5 und I-6 legen nichts auf den Heap und ändern kein Verhalten:
      c65a, c65b, c65i, c65k, c65m, c65n, c65o, c65w, c65z bleiben grün; Δ „Globale Variablen“ durch R1 ≤ 32 Byte
      (gerechnet rund 16).

### Schritt G9 — Ausgangsstand ohne Cache (auf R1)

- [ ] **AKM.1 (Einspielen R1)** — Nach den Einspielzeilen (AKZ.5): ESP-OTA; danach Smoketest samt Update-Quelle und
      neuen Feldern **ohne** FEHLT, `install-app.sh --check` vollständig, `check-pwa.sh` ohne FEHLT; gemeldete
      Versionen gleich denen des Tags. Wache läuft vorher. Zeiten sofort ins Messprotokoll.
- [ ] **AKM.2 (Ladeprobe ohne Cache)** — Drei Läufe `ladeprobe` (je 1 kalt, 5 warm), einer in Englisch, ohne
      Haltegrenze (Ent-7). Ins Messprotokoll je Laden: Anfragen nach Leitungsstatus, Byte, `heap_low_block`,
      `heap_low_path`, I-4-Felder. Erwartet (◐): auch warm nur `200`, je Laden rund 200 KB. Vergleichswert für AKW.2.
- [ ] **AKM.3 (Browser-Sitzung Safari)** — Phasen S0–S4 nach `design.md` §5.3, bedient vom Lead per `osascript`
      (Ent-6); **danach einmal** dieselben Phasen durch den Nutzer auf seinem Gerät aus L461, auf Zuruf des Leads.
      Je Phasenende im Messprotokoll: `free_heap`, `max_free_block`, `tcp_now_*`, `no_request_*`, die Heap-Zeilen
      mit `tcp=` im Phasenfenster, die Mac-Seite (`netstat`, `lsof`; beim Gerät des Nutzers nur die ESP-Seite). Jede
      eigene Lesung ist eine Verbindung und wird über `tcp_now_self_port` und ihren TIME_WAIT-Eintrag kenntlich
      gemacht, nicht weggerechnet.
- [ ] **AKM.4 (Browser-Sitzung Chrome)** — dieselben Phasen mit Chrome (Lead), nach mindestens 300 s Ruhe.
- [ ] **AKM.5 (Startmessung C71)** — Drei ESP-Starts ohne jeden Browser (Weg nach `design.md` §5.5), davon einer mit
      erzwungenem Schreiben einer Cache-Datei. Je Start ins Messprotokoll: die Heap-Zeilen der ersten 5 min samt
      `tcp=`, die Zeilen `- update cache …` samt `- update cache heap …`, `- heap erste anfrage …`, Update-Quelle
      nach dem Start. Zuordnung nach der Entscheidungsregel `design.md` §5.5 zu **einem** von N1–N4, mit den Werten,
      die sie tragen.
- [ ] **AKM.6 (Gesundheit G9)** — Exceptions, Neustarts ausser den gewollten (AKM.5), `diag`-Lücken und `ovr`
      vollständig erfasst und als Ergebnis berichtet (Ent-7); `watch-log.sh --pruefe` im Bericht.

### Schritt R2 — C70 Asset-Cache (Variante (a), Ent-1)

- [ ] **AKC.1 (`ETag`)** — Jede `200`-Antwort auf ein Asset aus `APP_INSTALL_ASSETS`, das als `.gz` ausgeliefert
      wird, beginnt mit `HTTP/1.1 200 OK` (AKC.7) und trägt `ETag: "<grösse>-<crc32>"` (Grösse dezimal, CRC-32 des
      gespeicherten `.gz`-Inhalts als acht Hexziffern, Ent-2). `Content-Type`, `Content-Encoding`, `Content-Length`,
      `Cache-Control: no-cache`, `Connection: close` unverändert. Legacy, `/api/*`, Installationsseite und Assets
      ausserhalb der Weissliste tragen **kein** `ETag`.
- [ ] **AKC.2 (`If-None-Match` und `304`)** — Der GET-Zweig liest `If-None-Match` (Name ohne Rücksicht auf
      Gross-/Kleinschreibung; `W/`-Präfix und Komma-Liste verstanden; `*` gilt als **kein** Treffer). Trifft ein Wert
      den aktuellen `ETag` **und** die Datei besteht mit Grösse > 0 **und** — beim Index — die Installation ist
      vollständig (`http.cpp:2826-2829`), antwortet der ESP `HTTP/1.1 304 Not Modified` mit `ETag`,
      `Cache-Control: no-cache`, `Connection: close`, **ohne** Körper und ohne `Content-Length`. Sonst der heutige
      Weg. Prüfstand **c70e**.
- [ ] **AKC.3 (Invalidierung)** — `ETag` aus dem **Inhalt**; RAM-Tabelle je Asset, nach dem Start leer, ein Eintrag
      gilt nur bei gleicher Dateigrösse, und **jede** schreibende LittleFS-Stelle in den ESP-Quellen (`open` mit
      Schreibmodus, `remove`, `rename`, `format`) ruft danach `app_etag_invalidate()` — oder steht mit Grund in einer
      Ausnahmeliste im Prüfstand. c70e zählt die Stellen selbst (Grep-Bestand am `72cfda7`: 31 in drei Dateien,
      `design.md` §4.4; vollständig erst mit P6). Sabotagen: „eine Invalidierung entfernt“, „`ETag` nur aus der
      Grösse“ (zwei Inhalte gleicher Grösse), „`304` ohne Grössenprüfung“, „`304` mit Körper“ ⇒ je FEHL; gegen den
      Stand R1 ⇒ FEHL.
- [ ] **AKC.4 (Kosten und Zähler)** — CRC nur bei Bedarf (erste Auslieferung nach Start oder Invalidierung), im
      selben Mount und mit derselben geöffneten Datei wie die Auslieferung, ohne Heap, ohne `String`. Je Berechnung
      genau eine Zeile `- app etag <datei> n=<byte> ms=<dauer>` in Mitschnitt und Ring. `device_ready` gibt hinter
      den I-5-Feldern aus: `app_not_modified`, `app_full`, `app_etag_calc` — seit dem Start, sättigend, nie
      zurückgesetzt. Die längste Berechnung (`app-app.js.gz`) unter 2'000 ms (◐ geschätzt 0,2–0,5 s).
- [ ] **AKC.5 (Invarianten)** — App-Assets weiter **ausschliesslich** als `.gz` mit Prüfung Grösse > 0
      (`http.cpp:2044-2049`); kein Plain-Fallback; `/app/?action=install` und Installationsseite unverändert; die
      Legacy-Oberfläche liefert wie vor R2 (Stichprobe `/`, `/legacy`, `/fs`, `/update` per `curl`, Körper gegen den
      Stand R1 verglichen bis auf Zeit- und Messwerte, Statuszeile `HTTP/1.0`, Kopfzeilen ohne `ETag`).
- [ ] **AKC.6 (PWA-Teil, Ent-3)** — `loadLayoutPreviewRowsMap()` holt `layout-previews.json` mit
      `cache: "no-cache"` statt `"no-store"` (`app.js:11655`); sonst keine Änderung an `app.js`; in `sw.js` nur
      `CACHE_NAME`. Prüfstand **c70p**; Sabotage `"no-store"` ⇒ FEHL; gegen 1.4.100 ⇒ FEHL.
- [ ] **AKC.7 (Statuszeile `HTTP/1.1` nur für App-Assets)** — Genau die zwei Antworten aus AKC.1 und AKC.2 (`200`
      und `304` auf Weisslisten-Assets als `.gz`) tragen die Statuszeile `HTTP/1.1`; **jede** davon sendet
      `Connection: close`, und der ESP schliesst nach der Antwort wie heute. Die `200` trägt ein `Content-Length`,
      das genau der gesendeten Körperlänge entspricht; bricht die Übertragung ab, endet die Verbindung mit RST
      (`abort()`, wie heute bei `http_write_broken`), nie mit einem regulären Ende. **Alle anderen** Antworten —
      Legacy, `/api/*`, Installations- und Hinweisseiten unter `/app`, `404` — bleiben `HTTP/1.0`. Prüfstand
      **c70e** (statisch: `HTTP/1.1` steht in `http.cpp` an genau diesen zwei Stellen, beide mit `Connection: close`;
      dynamisch: Körperlänge gleich `Content-Length`); Sabotagen „`Connection: close` in der `304` entfernt“ und
      „`HTTP/1.1` in einer `/api/`-Antwort“ ⇒ je FEHL; Gegenprobe der Ladeprobe am Prüfserver mit `HTTP/1.0` ⇒ FEHL
      (AKZ.2 (i)).

### Schritt G10 — mit Cache, und Tor (auf R2)

- [ ] **AKW.1 (Einspielen R2, DIR-002)** — **erst** ESP-OTA, Smoketest, `install-app.sh --check`; **dann**
      `install-app.sh` mit der neuen PWA, erneut `--check`; `check-pwa.sh` ohne FEHLT; Versionen beider Komponenten
      gleich denen des Tags. Zeiten sofort ins Messprotokoll.
- [ ] **AKW.2 (Ladeprobe mit Cache, Chrome)** — Drei Läufe wie AKM.2, mit `--erwarte-304`, ohne Haltegrenze. Je
      **warmem** Laden (Neuladen **und** erneutes Öffnen): für **jede** `/app/`-Anfrage `If-None-Match` gesendet und
      Leitungsstatus `304`, das Dokument `/app/` eingeschlossen; Summe `encodedDataLength` über `/app/` ≤ 3'000 Byte;
      Δ`app_full` = 0; `heap_low_block` im warmen Fenster ≥ 1'072. Je **kaltem** Laden berichtet. Gegenüberstellung
      je Laden mit AKM.2 (Byte, `heap_low_block`) im Messprotokoll.
- [ ] **AKW.3 (Probe U — Update sofort wirksam)** — Im dauerhaften Profil warm laden; **eine** App-Datei mit anderem
      Inhalt hochladen (`app/layout-previews.json` um Leerraum ergänzt, neu gepackt, über `app_file_upload`); neu
      laden: genau diese Datei `200` mit **neuem** `ETag` und neuem Inhalt, alle anderen `304`, genau eine Zeile
      `- app etag app-layout-previews.json.gz …`. Original zurücklegen, neu laden: `200` mit dem ursprünglichen
      `ETag`; danach `install-app.sh --check` ohne Abweichung. Schreibend und ohne Einzelfreigabe zulässig (Ent-15).
- [ ] **AKW.4 (Browser-Sitzungen mit Cache, Safari ungeprüft)** — Safari (Lead) und Chrome wie AKM.3/AKM.4; vor S0
      einmal öffnen und den Browser beenden, damit sein Cache Validatoren hält. Zusätzlich je Phase `app_full` und
      `app_not_modified`. **Für Safari** ist das der erste Nachweis, ob er bei `HTTP/1.1` mit `ETag` nachfragt: Δ
      `app_not_modified` und Δ`app_full` in S1 im Messprotokoll; bleibt `app_not_modified` bei 0, ist das ein Befund
      (C70 wirkt in Safari nicht), kein Fehler der Messung. Je Phase gegenübergestellt mit G9: Lücke `free_heap`,
      `max_free_block`, PCB-Liste.
- [ ] **AKW.5 (Auswertung und Tor)** — Der Lead ordnet C72 nach der Entscheidungsregel (`design.md` §5.4) **einem**
      von M1–M5 zu — auf den Daten von G10 (Stand, auf dem R3 aufsetzt), G9 daneben; weichen beide ab, gilt G10, und
      die Abweichung steht im Bericht. C71 trägt seine Zuordnung aus AKM.5 und den Bericht aus A.1 bei. Beides ins
      Messprotokoll, C71 und C72 vom `doc-writer` nachgeführt (nicht gleichzeitig, R3). **Erst danach** legt der Lead
      Ent-8, Ent-10, Ent-14 und Ent-17 vor. Kein Eingriff ohne zugeordnetes Ergebnis.
- [ ] **AKW.6 (Gesundheit G10)** — wie AKM.6.

### Schritt R3 und G11 — Eingriff C72, Massnahme C71 (nach dem Tor)

AKE.* entfallen, wenn das Tor für C72 keinen Eingriff ergibt (M4, M5 oder Ent-8 = kein Eingriff); AKS.* entfallen,
wenn für C71 N4 gilt oder Ent-17 keine Massnahme wählt. Im Endstand dann „entfallen“ mit dem Ergebnis aus AKW.5 als
Grund. Entfallen beide, entfällt R3 samt G11.

- [ ] **AKE.1 (C72-Eingriff umgesetzt)** — Kandidat nach Ent-8 gemäss `design.md` §6. **K1:** Verbindung ohne Anfrage
      mit `abort()` (RST) statt `stop()`; Zähler unverändert; jeder andere Abbau bleibt. **K2:** höchstens einmal je
      Sekunde PCBs in `tcp_active_pcbs` mit `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING oder LAST_ACK,
      deren Ruhezeit die Frist aus Ent-14 überschreitet, mit `tcp_abort()` abbauen; PCBs mit `ClientContext` **nie**;
      Zähler `tcp_reaped` in `device_ready`. **K5:** `HTTP_TW_LIMIT` 1. Prüfstand **c72e**; Sabotagen (K1 `stop()`;
      K2 „PCB mit `callback_arg` ≠ NULL abgebaut“, „Frist entfernt“; K5 „Grenze 2“) ⇒ je FEHL; gegen den Stand R2 ⇒
      FEHL.
- [ ] **AKE.2 (C72-Wirkung, G11)** — Browser-Sitzung Safari wie AKW.4: 180 s nach Beginn von S2 **und** von S3 je
      `free_heap` ≥ `free_heap`(S0) − 300 Byte; in `tcp_now_list` kein PCB mit `eigen` = 0 ausser TIME_WAIT und
      keiner mit Ruhezeit über der Frist aus Ent-14. Daneben L461 (−1'700), AKM.3, AKW.4. Chrome berichtet.
- [ ] **AKE.3 (C72 ohne Regression)** — drei `check-pwa.sh` ohne FEHLT und ohne `net::ERR_CONNECTION_RESET`;
      Ladeprobe warm wie AKW.2; Δ`write_lost_failed` = 0 über die drei Läufe; Smoketest ohne FEHLT.
- [ ] **AKS.1 (C71-Massnahme umgesetzt)** — Kandidat nach Ent-17 gemäss `design.md` §7, mit der Begründung aus AKM.5
      und A.1. Prüfstand **c71e** mit den Fällen und Sabotagen aus `design.md` §8 für den gewählten Kandidaten; gegen
      den Stand R2 ⇒ FEHL.
- [ ] **AKS.2 (C71-Wirkung, G11)** — Ruhewert `max` der Heap-Zeile **≥ 5'120** (K30) (i) 300 s nach jedem von drei
      ESP-Starts ohne Browser, einer davon mit erzwungenem Schreiben (wie AKM.5), und (ii) 150 s nach einem
      `check-pwa.sh` und nach einer Ladeprobe. Daneben die Werte aus AKM.5, L460 (4'672) und L457 (7'184 vor dem
      Abgleich). Die Zeilen `- update cache heap …` belegen, dass der Block über den Durchgang erhalten bleibt.
- [ ] **AKS.3 (C71 ohne Regression)** — Der Zwischenspeicher arbeitet wie vorher: nach einem Start ohne Änderung je
      Datei `unveraendert`, nach dem erzwungenen Schreiben `geschrieben` und danach `unveraendert`; keine liegen
      gebliebene `*.tmp` (`/api/fs_list`); Release Notes in PWA und Legacy sichtbar; `install-app.sh --check`
      vollständig; c65m, c65n grün.

### Übergreifend

- [ ] **AKZ.3 (Kodierung und Zeilenende)** — Vor jedem Patch Kodierung **und** Zeilenende festgestellt und im
      Taskbericht genannt (`http.cpp` UTF-8 mit einer CRLF-Zeile, `stm32flash.cpp` UTF-8, `ESP-uclock.ino` gemischt;
      DIR-015). Der Diff enthält nur inhaltlich geänderte Zeilen; der Lead prüft mit `git diff --stat`.
- [ ] **AKZ.4 (Release-Inhalt)** — R1 und R3 ändern nur `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino`, `version.h`; R2
      dazu `data/app/app.js` und `data/app/sw.js` (`CACHE_NAME`). **Keine** Änderung an `src/**`, `index.html`,
      `styles.css`, `manifest.webmanifest`. Geprüft gegen das jeweils vorige Tag.
- [ ] **AKZ.5 (Releases)** — **Jedes** Release einzeln (Ent-9): ein `make release-zip`, ein Commit, ein Tag nach
      DIR-011 mit den Versionen dieses Commits, Push samt Tag im selben Zug; Einspielzeilen nennen Reihenfolge
      (Firmware zuerst, DIR-002), `install-app.sh --check` nach dem OTA (DIR-017) und den Rückweg samt Befehl,
      **bevor** eingespielt wird (`design.md` §11).
- [ ] **AKZ.6 (T3, DIR-012)** — `pwa-tester` Phasen 0–4 und 9 auf dem Endstand (nach G11, oder nach dem Tor, wenn R3
      entfällt). Haltegrenze **K30** nach Ent-12: Ruhewert `max` < 5'120 oder `low=` < 1'072 ⇒ anhalten. Bei jedem
      Halt I-4- und I-5-Felder ohne Parameter in den Bericht, bevor zurückgesetzt wird. Uhr exklusiv
      (`TESTPLAN-PWA.md` Abschnitt 0 Punkt 5), Beginn und Ende angekündigt. Abgenommen mit Bericht und
      Abdeckungszahl.
- [ ] **AKZ.7 (Guardrails und Prüfstände)** — Nach jedem Task `./tools/guardrails.sh` Exit 0; vor jedem Release
      `--full` Exit 0, Ergebnis im Messprotokoll; die neuen Prüfstände in S16, Fallzahl nachgezählt (L235).
- [ ] **AKZ.8 (RAM)** — „Globale Variablen“ und Programmgrösse vor R1.1 und nach **jedem** ESP-Task sofort ins
      Messprotokoll; Zuwachs über das Paket höchstens **250 Byte** (Ent-11; gerechnet rund 170 ohne C71-Massnahme,
      `design.md` §10); darüber anhalten und vorlegen. Ein C71-Kandidat mit statischen Puffern (L3) sprengt die
      Grenze und braucht mit Ent-17 eine eigene Grenze. `app.js.gz` vor und nach R2.

**Zahl der Kriterien: 37** — AKZ 8, AKI 4, AKM 6, AKC 7, AKW 6, AKE 3, AKS 3. Am Tor hängen AKE.1–AKE.3 (Ent-8) und
AKS.1–AKS.3 (Ent-17); an Ent-16 hängt nur, in welchem Release AKS.1–AKS.3 nachgewiesen werden.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| Variante (b) und (c) für C70 | Ent-1 = (a); (c) nur nach G10.7 als neuer Nachtrag |
| `Last-Modified`/`If-Modified-Since` statt `ETag` | Zeitstempel im LittleFS unsicher, und ob Chromium ihn bei `HTTP/1.0` nimmt, ist ebenso offen; mit `HTTP/1.1` trägt der `ETag` (`design.md` §4.4, §14) |
| `HTTP/1.1` für Legacy, `/api/*` oder Hinweisseiten | nicht nötig; jede weitere Stelle wäre ein Risiko ohne Gewinn (AKC.7) |
| Keep-Alive, Pipelining, `Transfer-Encoding: chunked` | der Server bedient eine Anfrage je Verbindung und schliesst; daran ändert `HTTP/1.1` nichts |
| Ein **kalter** Erstaufruf unter 1'072 | C70 ändert ihn nicht; gemessen und berichtet (Ziel 1) |
| Eine C71-Massnahme ohne belegte Ursache (N4) | Ziel 2; C71 bleibt dann mit Befund offen |
| **C67** `FS()` ohne Heap, **C66** Teildatei im Download, **C68**, **C69** | eigene Befunde |
| Änderungen an `index.html`, `styles.css`, `manifest.webmanifest`, Icons | für (a) nicht nötig |
| Neuer Service Worker, Änderung an `sw.js` ausser `CACHE_NAME` | unter `http://` ohne Wirkung (`design.md` §4.5) |
| `Cache-Control: max-age` ohne Versionierung | verletzt „Update sofort wirksam“ (`design.md` §14) |
| Höchstzahl Verbindungen je Gegenstelle (K4) | Risiko für parallele Asset-Ladungen (`design.md` §6) |
| Ein C72-Eingriff vor dem Tor, auch „vorsorglich“ | Mechanismus nicht belegt; erst messen |
| Änderungen an der Legacy-Oberfläche | Invariante; AKC.5 prüft, dass sie unverändert ausliefert |
| STM-Firmware (`src/**`) | nicht betroffen |
| Browser zu steuern | nicht möglich; der Eingriff liegt im ESP |
| Schutz für `app_file_upload` | Schutz-Spec (C61) |
| Nachjustieren der Backlog-Schwellen aus E4 | eigener Task mit Begründung aus I-4/I-5 |
| L462 (Laden schreibt `ambilight_online`, B61) | eigener Befund; hier nur Nebenwirkung des Ladens |

---

## Betroffene Laufzeiten

- [ ] **ESP8266** — R1: `http.cpp`, `base.h`, `ESP-uclock.ino`; R2: `http.cpp`, `stm32flash.cpp`, `base.h`,
      `ESP-uclock.ino`; R3: `http.cpp`, `base.h`, gegebenenfalls `ESP-uclock.ino`; je ein OTA
- [ ] **PWA** — R2: `app.js` (eine Zeile), `sw.js` (`CACHE_NAME`); ein Upload nach dem OTA
- [ ] **STM32** — keine
- [ ] **Werkzeug** — neu `ladeprobe` (liegt beim Lead bereits vor, ✔ L), `browser-sitzung`; geändert `check-pwa`,
      `smoke-device.sh`, `watch-log.sh`; Prüfstände c72i, c71i, c70e, c70p, c72e, c71e
- [ ] **Dokumentation** — `BEFUNDE.md` (Messprotokoll vom Lead, Befunde vom `doc-writer`), `TESTPLAN-PWA.md`,
      `CHANGELOG.md`
- [ ] **Build/Release** — drei Releases R1, R2, R3 (Ent-9; R3 nur nach dem Tor; ein viertes nur mit Ent-16 B), je
      Commit, Tag, Push, Einspielen

---

## Entscheidungen

**Entschieden am 15.10.2026 vom Nutzer** (über den Lead übermittelt):

| Ent | Gegenstand | Entscheid | Folge in dieser Spec |
|---|---|---|---|
| **Ent-1** | Variante C70 | **(a)** `ETag` + `304`; (c) wird nur nach G10.7 vorgelegt | AKC.1–AKC.5, AKC.7 |
| **Ent-3** | PWA-Teil | **immer** (`layout-previews` mit `no-cache`), unabhängig von der Grösse — R2 hat einen PWA-Anteil: Firmware zuerst, dann PWA (DIR-002) | AKC.6, AKW.1 |
| **Ent-6** | Bedienung der Browser | **C:** Lead per `osascript`, dazu einmal der Nutzer auf seinem Gerät | AKM.3 |
| **Ent-7** | Haltegrenze in den Messungen | **keine:** alle Ladungen durchfahren und protokollieren; Neustarts auf der Test-Uhr hinnehmbar, aber als **Ergebnis** erfasst, nicht übergangen | Kopf der Kriterien, AKM.6, AKW.6 |
| **Ent-9** | Releaseschnitt | **drei Releases:** R1 Instrument ⇒ G9 Ausgangsstand ohne Cache ⇒ R2 C70 ⇒ G10 mit Cache ⇒ Tor ⇒ R3 Eingriff; jedes einzeln mit Commit, Tag, Einspielen | Überblick, AKZ.5 |
| **Ent-12** | Ruhewert-Grenze in T3 | **5'120** (K30) | AKZ.6 |
| **Ent-13** | C71 | **mitnehmen** | Problem §2, AKI.3, AKM.5, AKS.1–AKS.3 |

**Lead-Entscheide 15.10.2026 (wie empfohlen):** **Ent-2** CRC-32 + Grösse; **Ent-4** `tcp=`/`nrt=` in der Heap-Zeile;
**Ent-5** nur letztes Oktett; **Ent-11** 250 Byte.

**Entfallen:**

- **Ent-15** (schreibende Schritte in der Messung) — **entfällt.** Grund: `CLAUDE.md` R5 und DIR-008 wurden am
  14.10.2026 geändert (Commit `e7c5d63`, `CLAUDE.md:412-416`): „Schreibende Prüfungen, Flashen, Resets und Messungen
  laufen ohne Einzelfreigabe, auch nachts.“ Vom `spec-writer` an `CLAUDE.md:416` nachgelesen. Probe U (AKW.3) und die
  ESP-Starts der Startmessung (AKM.5, AKS.2) bleiben drin. Was bleibt, ist technisch: **eine** Messung zur Zeit, und
  die Gefahrentabelle in `CLAUDE.md` gilt — Probe U und das erzwungene Schreiben berühren nur `layout-previews.json`
  bzw. eine `upd-*`-Datei, keinen Eintrag dieser Tabelle.

**Offen bis zum Tor (G10.6):**

| Ent | Gegenstand | Optionen | Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-8** | Eingriff C72 | K1, K2, K1 + K2, K5 als Zusatz, kein Eingriff | nach Ergebnis (`design.md` §5.4): M1 ⇒ K2; M2 ⇒ K1; M3 ⇒ K1 + K2; M4/M5 ⇒ kein Eingriff | Nutzer |
| **Ent-10** | schreibender Zugriff auf lwIP-Listen (für K2) | erlauben; nicht | erweitert Ent-32 aus Paket 2026-10-13 („nur lesend“); **erlauben, nur mit K2, nur PCBs ohne `ClientContext`** | Nutzer |
| **Ent-14** | Frist in K2 | 5 s; 20 s | **5 s**, endgültig mit den Ruhezeiten aus G9/G10 | Nutzer |
| **Ent-17** | Massnahme C71 | L1–L5 (`design.md` §7), keine | nach Ergebnis (`design.md` §5.5): N1/N3 ⇒ **L4**, mit **L2**, falls A.1 belegt, dass eine langlebige Belegung im Fenster entsteht; N2 ⇒ **L1** oder **L2** nach P10/P11; N4 ⇒ keine. **L3** nur mit eigener RAM-Grenze | Nutzer |

**Neu, offen vor R3:**

| Ent | Gegenstand | Optionen | Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-16** | Platz der C71-Massnahme im Releaseschnitt | (A) in **R3** zusammen mit dem C72-Eingriff; (B) eigenes **R4** nach R3; (C) in R2 mit C70 | **A** — C71 und C72 haben getrennte Messgrössen: C71 den Ruhewert `max` nach einem Start **ohne** Browser (AKS.2), C72 die Lücke in `free_heap` während einer Browser-Sitzung (AKE.2); G11 misst beides in getrennten Phasen. R3 findet statt, sobald einer der beiden Teile Inhalt hat. Das spart Dir ein OTA. **B** wählen, wenn Dir ein getrennter Rückweg wichtiger ist: Bei A nimmt ein Rückflash beide Teile zurück. **C** verworfen — G10 soll die Wirkung des Caches **getrennt** zeigen (Ent-9), und C71 verschöbe dort den Ruhewert und mit ihm `low=` | Nutzer |
