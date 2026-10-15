# Anforderungen — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70 Asset-Cache, C71 Zerstückelung beim Start, C72 offen gehaltene Verbindungen)

**Status: freigegeben am 15.10.2026 bis aufs Tor; Nachtrag A.2 vom 15.10.2026 zur Freigabe.** Entscheide des Nutzers
und Lead-Entscheide vom 15.10.2026 eingearbeitet. **Nachtrag A.2** (Ergebnisse des `firmware-analyst` zu P1–P8,
`HTTP/1.1` und C71) und die **Trennmessung C71** des Leads (L468) ändern I-5, I-6, die Entscheidungsregeln, die
C71-Kandidaten und öffnen **Ent-2** neu; was sich ändert und was widerlegt ist, steht in `design.md` §17. **Offen:**
Ent-2 (Lead, vor R2.1), Ent-8, Ent-10, Ent-14, Ent-17 (am Tor, G10.6), Ent-16 (vor R3). **Vom Nutzer noch nicht
gesehen:** der C71-Teil, die Statuszeile `HTTP/1.1` (AKC.7) und der Nachtrag A.2.

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge am selben Tag. Am Gerät laut letztem
Messprotokoll: STM 3.2.34, ESP 3.2.37, PWA 1.4.100 (`BEFUNDE.md`, L466) — Ausgangsstand, **kein** Sollwert; die
Versionen der Releases setzt der Lead über den `release-engineer` (R4).
**Auslöser:** Nutzerentscheid 15.10.2026 — dieses Paket als nächster Schritt nach Paket 2026-10-13 (E3, E4).
**Grundlage:** `BEFUNDE.md` C65, C70, C71, C72, L456, L457, L460, L461, L466, L468; `specs/paket-2026-10-13/`
(`design-e4.md` §19, `requirements-e4.md`, `endstand.md`); `TESTPLAN-PWA.md` K30 (7.1), Abschnitt 0 Punkt 5, 5b;
`ERWEITERUNGEN.md` X1, X14, X16; `CLAUDE.md` R5/DIR-008 in der Fassung vom 14.10.2026 (`CLAUDE.md:412-416`); Bericht
des `firmware-analyst` zu A.1 (über den Lead, 15.10.2026).

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen:** **✔ SW** — vom `spec-writer` am Code `72cfda7` bzw. an der Core-Quelle 3.1.2
(`~/Library/Arduino15/packages/esp8266/hardware/esp8266/3.1.2/`) nachgelesen; **✔ B** — in `BEFUNDE.md` belegt;
**✔ L** — vom Lead gemessen und gemeldet; **✔ FA** — vom `firmware-analyst` am Core belegt (A.1); **◐ FA** — vom
`firmware-analyst` als plausibel gemeldet (lwIP 2.1.3 liegt nur binär vor, Upstream-Stand aus dem Gedächtnis);
**◐** — abgeleitet oder Annahme; **●** — gemeldet, vor der Umsetzung zu prüfen.

**Kennungen dieses Pakets:** Schritte **0**, **A**, **R1** (Instrumente I-5, I-6), **G9** (ohne Cache), **R2** (C70),
**G10** (mit Cache, **Tor**), **R3** (Eingriff C72, Massnahme C71), **G11**, **T3**, **Z**. Akzeptanzkriterien
**AKZ**, **AKI**, **AKM**, **AKC**, **AKW**, **AKE**, **AKS**. Entscheidungen **Ent-1 bis Ent-17**. `ETag`-Varianten
**T** (Trailer) und **V** (Voll-CRC). Kandidaten C72 **K1–K5**, C71 **L0–L6**; Ergebnisse der Entscheidungsregeln C72
**M1–M5**, C71 **N0–N4**; Gerätenachweis **P1-Probe** (`design.md` §5.6). Prüfpunkte **P1–P12** (`design.md` §12). Die
Nummern gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Nachweis |
|---|---|---|---|---|
| **0** | Ausgangswerte ohne Last, Werkzeuge `ladeprobe` und `browser-sitzung`; Trennmessung C71 auf 3.2.37 **erledigt** (L468) | — | — | Messprotokoll |
| **A** | P1–P8 erledigt (Nachtrag A.2); P7, P9, P11, P12 offen (`firmware-analyst`) | — | — | Bericht |
| **R1** | **I-5** laufende Verbindungszustände samt `_unclaimed` und toten Abholungen, Heap-Zeile mit `tcp=`/`nrt=`; **I-6** Startzustand und Heap im Startabgleich, Flash-Kennung | ESP | ESP-OTA | **G9** |
| **G9** | **Ausgangsstand ohne Cache:** Ladeprobe, **P1-Probe**, Browser-Sitzungen Safari (Lead, dazu einmal Nutzer) und Chrome, **Startmessung C71** (entscheidend für C71) | — | — | Messprotokoll |
| **R2** | **C70** `ETag`/`304` (Variante nach Ent-2), Statuszeile `HTTP/1.1` für App-Assets, PWA-Teil | ESP **und** PWA | Firmware zuerst, dann PWA | **G10** |
| **G10** | **Mit Cache:** Ladeprobe, Probe U, Browser-Sitzungen erneut; Zuordnung M1–M5, N0–N4; **Tor** | — | — | Ent-8, Ent-10, Ent-14, Ent-17 |
| **R3** | **Eingriff C72** (Ent-8) und **Massnahme C71** (Ent-17), Platz nach Ent-16 | ESP | ESP-OTA | **G11** |
| **T3** | `pwa-tester` Phasen 0–4 und 9 auf dem Endstand (DIR-012) | — | — | AKZ.6 |

---

## Problem

### 1. Ein volles Neuladen der PWA reisst die Haltegrenze K30 — mit nur einem Client (C65, C70)

**Befund am Gerät, ✔ B (L466, ESP 3.2.37, PWA 1.4.100):** In T2 fiel der Tiefstwert um 21:28:37 auf `low=752`
(Grenze 1'072), `max` 3'584, beim Neuladen der PWA in Englisch (K33) mit **einem** Client. E4 hatte das Tief unter
Last vorher auf 1'336–2'184 gehoben (G8, G7R, L457).

**Ursache der Last, ✔ SW:** Jedes Öffnen und Neuladen holt **alle** App-Assets vollständig vom ESP:

- Die Dateiauslieferung antwortet `HTTP/1.0 200 OK` mit `Cache-Control: no-cache` und `Connection: close`, ohne
  `ETag` oder `Last-Modified` (`ESP8266/ESP-uclock/http.cpp:2067-2078`; ✔ L per `curl -D`). `If-None-Match` wird nicht
  ausgewertet (`http.cpp:16783-16802`). Einen `304`-Weg gibt es nicht.
- Über `http://` gibt es keinen Service Worker (`data/app/app.js:2135-2163`; `data/app/sw.js:77-101`).
- Umfang je Öffnen rund 200 KB (C70, ✔ B), davon `app-app.js.gz` 163'621 Byte (Messprotokoll E3, E3.0, ✔ B).
- `layout-previews.json` wird mit `cache: "no-store"` geholt (`app.js:11655`, ✔ SW).

**Befund am Prüfserver, ✔ L (Lead, 15.10.2026 22:21–22:23):** Ein `ETag` allein genügt bei einer
`HTTP/1.0`-Antwort **nicht**. Mit `HTTP/1.0` sandte Chrome für das Dokument kein `If-None-Match`, für `app.js` nur beim
Neuladen — fast nur `200`. Mit `HTTP/1.1` (weiter `Content-Length`, `ETag`, `no-cache`): jede warme Ladung mit
`If-None-Match`, überall `304`, 290 statt 4'045 Byte. Lesart (◐): Chromium nimmt einen `ETag` aus einer
`HTTP/1.0`-Antwort nicht als Validator. **Safari ist ungeprüft.** **✔ FA:** Nur `http_send_fs_file()`
(`http.cpp:2067`) muss die Statuszeile ändern; die übrigen `/app/`-Antworten (`:2791`, `:2841`, `:2866`, `:2872`,
`:2897`) bleiben `HTTP/1.0`; Keep-Alive entsteht nicht (`Connection: close` `:2077`, `Content-Length` `:2075`,
`stop()` `:2236`).

### 2. Nach dem Start bleibt der grösste freie Block bei rund 4,7 KB (C71)

**Befund am Gerät, ✔ B (L460, L457 G8):** Nach dem OTA auf E4 stand `max` um 18:17:36 bei 7'184 und um 18:18:36 bei
4'672. Im Fenster lagen nur `upd-releasenote.txt geschrieben` (18:18:26) und `upd-wc-list.txt geschrieben`
(18:18:28), keine Anfrage. Danach blieb der Ruhewert bei 4'672; nach G7R 4'464. Unter E3 dasselbe Bild schwächer
(7'256 ⇒ 5'808, L450). K30 verlangt ≥ 5'120.

**Ablauf des Startabgleichs, ✔ SW:** `update_cache_poll()` (`http.cpp:13338-13388`) läuft frühestens
`UPDATE_CACHE_START_MS` = 60 s nach dem Start (`:13398`) und 5 s nach der letzten Anfrage, holt drei Versionszeilen und
gleicht im ersten Durchgang nach **jedem** Start beide Dateien ab (E4.5a). Je Datei: Abruf öffnen (`:13595`),
`LittleFS.begin()` (`:13614`), alte Datei lesen, gegebenenfalls Temporärdatei schreiben — bis zu zwei `File`-Objekte
gleichzeitig (`:13677`, `:13699`) —, umbenennen, `httpclient_stop()`, `LittleFS.end()` (`:13770-13771`).

**Trennmessung, ✔ L (Lead, ESP 3.2.37, L468; seit dem Start 14.10. 21:38 kein Flash-Schreiben):**

- (b) 22:40:02, `fs_remove upd-wc-list.txt` — erstes Flash-Schreiben seit dem Start, **ohne** Abruf: Heap vorher
  7'136/6'192, kurz 6'920/6'192 (TIME_WAIT der eigenen Anfrage), ab 22:42:30 wieder 7'136/6'192. **Kein dauerhaftes
  ΔFree um −264, `max` unverändert.**
- (c) 22:43:33, `update_status?refresh=1` — `upd-wc-list.txt geschrieben n=69` um 22:43:38, **mit** Abruf: während des
  Abrufs 6'920/6'192, `low=` 2'544, ab 22:46:30 wieder 7'136/6'192. **Auch das hinterlässt nichts.**
- Mit V3 aus T2 (Start mit `unveraendert`, keine Zerstückelung, Ruhewert 6'192): **C71 entsteht nur, wenn beim Start
  geschrieben wird** (◐, aus drei Messungen gefolgert).

**Damit ist der Verdacht K-a des `firmware-analyst` nicht bestätigt:** Der Core legt beim ersten Flash-Schreiben bei
PUYA-Flash (Hersteller 0x85) einen 256-Byte-Puffer an und gibt ihn nie frei (`cores/esp8266/Esp.cpp:726-731`, ✔ SW) —
(b) zeigte aber kein ΔFree um −264. Entweder ist es kein PUYA-Chip, oder der Weg ist ein anderer. **Offen ist, was in
den ersten rund 60 s nach dem Start lebt** (WLAN-Aufbau, erste Verbindungen, NTP/UDP, verzögert angelegte Objekte, der
erste `ClientContext` des statischen httpclient, `httpclient.cpp:16`) und zwischen das sich das Schreiben legt.

### 3. Nach einer Safari-Sitzung bleibt der Heap niedrig, bis Safari beendet wird (C72)

**Befund am Gerät, ✔ B (L461):** Nach einer kurzen Safari-Sitzung fiel `free` von 6'536 auf 4'832, `max` von 4'672
auf 3'816 und blieb dort bis mindestens 20:06; `http_backlog` stand auf 1. Laut T2-Bericht Erholung um 20:09:37 ohne
Browser. **Beobachtung des Nutzers (●):** frei erst beim **Beenden** von Safari.

**Was belegt ist:**

- ✔ SW: Die PWA pausiert bei verborgener Seite (`app.js:2079-2101`). Der Server nimmt je Durchlauf eine Verbindung an,
  wartet höchstens 250 ms auf die erste Zeile, zählt eine Verbindung ohne Anfrage (`no_request_timeouts`/`_aborts`)
  und beendet sie mit `stop()` ⇒ `tcp_close()` ⇒ FIN (`http.cpp:239`, `:16660-16710`; `ClientContext.h:73-92`).
- ✔ SW: Schliesst die Gegenstelle eine noch nicht abgeholte Verbindung, kehrt `http_server_loop()` bei
  `!http_client` (`http.cpp:16598-16601`) **ohne Zähler** zurück — `nrt` unterschätzt die Vorab-Verbindungen (◐ FA).
- ◐ FA (P1): `tcp_close()` setzt `TF_RXCLOSED`; FIN_WAIT_2 läuft dann rund 20 s nach dem letzten empfangenen Segment
  **ohne RST** ab (`tcp_priv.h:131`). FIN_WAIT_1, CLOSING und LAST_ACK haben **keine** eigene Frist, nur
  Wiederholungen bis `TCP_MAXRTX` 12 (`lwipopts.h:1258`) — das kann Minuten dauern. TIME_WAIT endet rund 120 s nach
  dem Schliessen. Ob P1 auf der Uhr gilt, zeigt die **P1-Probe** (AKM.7).
- ✔ FA (P2): `MEMP_NUM_TCP_PCB` begrenzt bei `MEMP_MEM_MALLOC` 1 **nichts**; TIME_WAIT hält der Core bei höchstens 6,
  unmittelbar nach dem eigenen `tcp_tw_limit()` bei 2. Der Kommentar `http.cpp:325` („Listen sind kurz“) ist falsch.
- I-4 zählt nur beim neuen Block-Tiefstwert (`http.cpp:473-530`).

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

**Flash-Schreiben seit dem Start:** Zu jedem Ruhewert und jedem Tiefstwert in G9, G10 und G11 vermerkt der Lead, ob seit
dem letzten ESP-Start schon geschrieben wurde — und ob **beim Start** (im ersten Durchgang des Abgleichs) — `ja` mit
Beleg (`- update cache … geschrieben`, App-Upload, ESP-Setter mit `EEPROM.commit()` nach P12), `nein` oder `unbekannt`.
Nach L468 zählt für C71 nur das Schreiben beim Start.

**Status `304` in CDP-Werkzeugen:** `Network.responseReceived` meldet bei einer `304`-Nachprüfung `200`; den
Leitungsstatus liefert nur `Network.responseReceivedExtraInfo.statusCode` (✔ L). Jedes Kriterium, das `304` zählt,
meint den Leitungsstatus.

### Schritt 0 — Grundlagen

- [ ] **AKZ.1 (Ausgangswerte ohne Last)** — Im Messprotokoll, bevor R1.1 beginnt: Grösse **jeder** der 13 Dateien
      aus `APP_INSTALL_ASSETS` (`/api/fs_list`); „Globale Variablen“ und Programmgrösse des Stands E4 (69'060 / 531'552
      Byte, L457); Kodierung **und** Zeilenende von `http.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`, `base.h`, `app.js`;
      `netstat`, `lsof`, `osascript` vorhanden; der Prüfserver-Befund zu `HTTP/1.0`/`HTTP/1.1`; die Ergebnisse von
      A.1 (P1–P8).
- [ ] **AKZ.2 (Werkzeuge, DIR-014)** — (i) `ladeprobe`: dauerhaftes Profil, 1 × kalt, N × warm (Neuladen ohne
      `ignoreCache` und erneutes Öffnen); `device_ready?heap_low_reset=1` vor jedem Fenster, danach lesen; je Laden
      Zahl der `/app/`-Anfragen nach **Leitungsstatus**, ob `If-None-Match` gesendet wurde, Summe
      `encodedDataLength`, `heap_low_block`, `heap_low_path`, I-4-Felder, `app_not_modified`/`app_full`, soweit
      vorhanden. Gegenproben: Vorschau mit `--erwarte-304` ⇒ Exit ≠ 0 und Meldung „warm: 304 erwartet, 200
      erhalten“; Prüfserver `HTTP/1.1` mit `ETag` ⇒ OK; Prüfserver `HTTP/1.0` ⇒ FEHL. (ii) `browser-sitzung`: Phasen
      nach `design.md` §5.3 in den Betriebsarten `osascript` (Lead bedient) und **`manuell`** (das Skript nennt die
      Phase, der Nutzer bedient, das Skript liest); liest je Phasenende `device_ready`, `netstat -anv -p tcp`,
      `lsof -nP -i @<uhr>` und gibt für **jede eigene Lesung** den lokalen Port und das Ende ihres TIME_WAIT-Fensters
      (Lesezeit + 120 s) aus; Gegenprobe: Uhr nicht erreichbar ⇒ Exit ≠ 0 mit Meldung. (iii) ab R1: `smoke-device.sh`
      verlangt die Felder aus AKI.1, ab R2 die aus AKC.4; setzt nichts zurück; Gegenprobe `SMOKE_DR_OVERRIDE`. (iv) ab
      R1: `watch-log.sh` wertet die Heap-Zeile mit Zusatz unverändert aus und führt `- update cache vorher`,
      `- update cache heap` und `- flash id` als Hinweis; Gegenprobe über `WATCHLOG_QUELLE` (`low=752 tcp=…` ⇒ Alarm
      kommt an). (v) ab R2: `check-pwa` INFO „`/app/` 304 je Laden“ (Leitungsstatus) und „Byte je Laden“; mit V
      `- app etag` als Hinweis. (vi) Werkzeuge und Prüfstände, die eine CRC des ESP nachrechnen (nur Variante V),
      benutzen **CRC-32/MPEG-2** (AKC.3), nicht zlib/`binascii`. Jede Meldung erscheint in Konsole **und** Mitschrift.
- [x] **AKZ.9 (Trennmessung C71 auf 3.2.37)** — Erstes Flash-Schreiben seit dem Start ohne Abruf (b) und Schreiben mit
      Abruf (c), je mit Heap-Zeilen vorher und nachher, im laufenden Betrieb. **Erfüllt, Beleg `BEFUNDE.md` L468**
      (Lead, 15.10.2026 22:40–22:46): beide ohne bleibende Spur; K-a nicht bestätigt (Problem §2).

### Schritt R1 — Instrumente

- [ ] **AKI.1 (I-5: laufende Verbindungszustände in `device_ready`)** — Hinter `api_orphan_skipped`:
      `tcp_now_states` (11 Zahlen, Index = lwIP `enum tcp_state`, Zählung über `tcp_active_pcbs` in diesem Augenblick,
      die eigene Verbindung eingeschlossen), `tcp_now_tw`, `tcp_now_pending` (`accepts_pending`),
      **`tcp_now_unclaimed`** (Länge der Liste `_unclaimed` der `WcHttpServer`, über `ClientContext::next()`),
      `tcp_now_self_port`, `tcp_now_list` (höchstens 8 Einträge `[zustand, gegenstelle, port, ruhe_s, nrtx, arg]`,
      erst aktive, dann TIME_WAIT; `gegenstelle` = letztes Oktett; `ruhe_s` = Sekunden seit dem letzten Setzen von
      `pcb->tmr`, also seit dem letzten empfangenen Segment **vor** unserem `close()` (`design.md` §3.1); `arg` = 0 ohne
      `callback_arg`, 1 `ClientContext`, 2 Listener — SYN_RCVD trägt den Listener), `tcp_now_more`; dazu der Zähler
      **`http_accept_dead`** (Abholungen aus `_unclaimed`, deren Gegenstelle schon geschlossen hatte — der frühe
      Rücksprung `http.cpp:16598-16601`, nur gezählt, wenn vor `accept()` ein Eintrag in `_unclaimed` stand; sättigend,
      seit dem Start). Listen **vollständig** in einen Stackpuffer, **bevor** das erste `http_send()` läuft; kein Heap,
      kein `String`, kein `yield()`. Bestehende Felder unverändert; nichts wird zurückgesetzt. Der Kommentar
      `http.cpp:325` wird nach P2 berichtigt. Prüfstand **c72i**; Sabotagen „Ausgabe innerhalb der Listenschleife“,
      „Kappung entfernt“, „SYN_RCVD als `ClientContext` gezählt“, „toter Abholversuch ohne Zähler“ ⇒ je FEHL; gegen
      3.2.37 ⇒ FEHL.
- [ ] **AKI.2 (I-5: Heap-Zeile, Ent-4)** — Die Heap-Zeile (`ESP-uclock.ino:601-656`) endet auf
      ` tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>`; einmal formatiert, Mitschnitt **und**
      `/api/stm32_log` textgleich, ≤ 119 Zeichen. Zählung in `http.cpp`, Prototyp in `base.h`. Prüfstand **c72i**;
      Sabotage „nur `Serial`“ ⇒ FEHL.
- [ ] **AKI.3 (I-6: Startzustand, Heap im Startabgleich, Flash-Kennung, C71)** — (a) Je Durchgang von
      `update_cache_poll()` **unmittelbar vor Schritt 1** genau eine Zeile `- update cache vorher up=<s>
      free=<n>/<blk> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> hc=<httpclient_connects>` (`up` = Sekunden seit dem Start;
      `hc` > 0 heisst: ein `ClientContext` des statischen httpclient liegt schon auf dem Heap); (b) am Ende jedes
      Durchgangs (auch bei Abbruch) genau eine Zeile `- update cache heap v=<free>/<blk> 1=<free>/<blk> 2=… 3=… 4=… 5=…
      n=<free>/<blk>` — je Schritt `free` **und** `blk`, `-` für nicht gelaufen; (c) einmal je Start beim **ersten**
      angenommenen Anfrage-Eingang `- heap erste anfrage free=<n> max=<n>`; (d) einmal in `setup()`, vor WLAN und
      `EEPROM.begin()`, `- flash id=<hex6> vendor=<hex2>`. Je Zeile ≤ 119 Zeichen, einmal formatiert, Mitschnitt **und**
      Ring textgleich, kein `String`. Prüfstand **c71i**; Sabotagen „`vorher` fehlt oder nach Schritt 1“, „ein Schritt
      ohne Messwert“, „nur `blk` statt `free`/`blk`“ und „nur `Serial`“ ⇒ je FEHL; gegen 3.2.37 ⇒ FEHL.
- [ ] **AKI.4 (keine Wirkung auf den Betrieb)** — I-5 und I-6 legen nichts auf den Heap und ändern kein Verhalten:
      c65a, c65b, c65i, c65k, c65m, c65n, c65o, c65w, c65z bleiben grün; Δ „Globale Variablen“ durch R1 ≤ 40 Byte
      (gerechnet rund 30).

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
      eigenen Lesung.
- [ ] **AKM.4 (Browser-Sitzung Chrome)** — dieselben Phasen, `osascript`, nach mindestens 300 s Ruhe.
- [ ] **AKM.5 (Startmessung C71 — der entscheidende Schritt)** — Drei ESP-Starts ohne Browser und ohne HTTP-Lesung in
      den ersten 5 min (`design.md` §5.5), zwei mit erwartetem `unveraendert`, einer mit erzwungenem Schreiben **beim
      Start** (`/upd-wc-list.txt` vorher entfernt). Je Start: `- flash id`, Heap-Zeilen der ersten 5 min samt `tcp=`,
      `- update cache vorher …`, `- update cache …`, `- update cache heap …`, `- heap erste anfrage …`, Update-Quelle
      danach. Erwartet nach L468: Zerstückelung nur im Start mit Schreiben. Zuordnung zu **einem** von N0–N4 mit den
      tragenden Werten; AKZ.9 steht daneben.
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
      `"<gz-grösse>-<trailer>"` (letzte 8 Byte der `.gz` als Hex — CRC-32 und Länge des entpackten Inhalts, aus dem
      gzip-Trailer) oder **Variante V** `"<grösse>-<crc32>"` (CRC über den ganzen gespeicherten Inhalt). Beginnt die Datei
      nicht mit `1f 8b` (kein gzip), **kein** `ETag` (nur T). `Content-Type`, `Content-Encoding`, `Content-Length`,
      `Cache-Control: no-cache`, `Connection: close` unverändert. Legacy, `/api/*`, Installationsseite und Assets
      ausserhalb der Weissliste tragen **kein** `ETag`.
- [ ] **AKC.2 (`If-None-Match` und `304`)** — Der GET-Zweig liest `If-None-Match` (Name ohne Rücksicht auf
      Gross-/Kleinschreibung; `W/` und Komma-Liste verstanden; `*` gilt als **kein** Treffer). Trifft ein Wert den
      aktuellen `ETag` **und** die Datei besteht mit Grösse > 0 **und** — beim Index — die Installation ist vollständig
      (`http.cpp:2826-2829`), antwortet der ESP `HTTP/1.1 304 Not Modified` mit `ETag`, `Cache-Control: no-cache`,
      `Connection: close`, **ohne** Körper und ohne `Content-Length`. Sonst der heutige Weg. Prüfstand **c70e**.
- [ ] **AKC.3 (Gültigkeit des `ETag`)** — **Variante T:** bei **jeder** Anfrage aus der Datei selbst gelesen (Grösse,
      ein `seek` ans Ende − 8, 8 Byte lesen, zurück an den Anfang); keine Tabelle, keine Invalidierung. Prüfstand
      **c70e:** zwei gültige `.gz` gleicher Grösse mit verschiedenem Inhalt ⇒ verschiedene `ETag`; abgeschnittene
      Datei ⇒ anderer `ETag`; keine gzip-Datei ⇒ kein `ETag`; Sabotagen „`ETag` nur aus der Grösse“, „`304` ohne
      Grössenprüfung“, „`304` mit Körper“ ⇒ je FEHL. **Variante V:** RAM-Tabelle je Asset, nach dem Start leer, ein
      Eintrag gilt nur bei gleicher Grösse, und **jede** der 31 schreibenden LittleFS-Stellen (✔ FA, vollständig;
      `design.md` §4.4) ruft danach `app_etag_invalidate()`; die CRC ist die des Cores, **CRC-32/MPEG-2** (Polynom
      0x04C11DB7, MSB zuerst, Start 0xFFFFFFFF, ohne Schluss-XOR; `crc32.cpp:26-43`, ✔ SW). c70e zählt die Stellen
      selbst; Sabotage zusätzlich „eine Invalidierung entfernt“ ⇒ FEHL. In beiden Varianten gegen den Stand R1 ⇒
      FEHL.
- [ ] **AKC.4 (Kosten und Zähler)** — **T:** je Antwort ein `seek` und 8 Byte Lesen mehr, im selben Mount und mit
      derselben geöffneten Datei; Mehrdauer je Anfrage im Messprotokoll (◐ Millisekunden). **V:** CRC nur bei Bedarf,
      im selben Mount und mit derselben Datei, je Berechnung eine Zeile `- app etag <datei> n=<byte> ms=<dauer>`,
      `app-app.js.gz` unter 2'000 ms (◐ FA 0,15–0,3 s). In beiden: kein Heap, kein `String`; `device_ready` gibt hinter
      den I-5-Feldern `app_not_modified`, `app_full` und (nur V) `app_etag_calc` aus — seit dem Start, sättigend.
- [ ] **AKC.5 (Invarianten)** — App-Assets weiter **ausschliesslich** als `.gz` mit Prüfung Grösse > 0
      (`http.cpp:2044-2049`); kein Plain-Fallback; `/app/?action=install` und Installationsseite unverändert; die
      Legacy-Oberfläche liefert wie vor R2 (Stichprobe `/`, `/legacy`, `/fs`, `/update` per `curl` gegen den Stand R1
      bis auf Zeit- und Messwerte, Statuszeile `HTTP/1.0`, ohne `ETag`).
- [ ] **AKC.6 (PWA-Teil, Ent-3)** — `loadLayoutPreviewRowsMap()` holt `layout-previews.json` mit `cache: "no-cache"`
      statt `"no-store"` (`app.js:11655`); sonst keine Änderung an `app.js`; in `sw.js` nur `CACHE_NAME`. Prüfstand
      **c70p**; Sabotage `"no-store"` ⇒ FEHL; gegen 1.4.100 ⇒ FEHL.
- [ ] **AKC.7 (Statuszeile `HTTP/1.1` nur für App-Assets)** — Nur die Kopfzeilen in `http_send_fs_file()`
      (`http.cpp:2067`) und die neue `304` tragen `HTTP/1.1`; **beide** senden `Connection: close`, der ESP schliesst
      danach wie heute (`:2236`). Die `200` trägt ein `Content-Length` gleich der gesendeten Körperlänge; ein Abbruch
      endet mit RST (`abort()`), nie regulär. Die übrigen `/app/`-Antworten (`:2791`, `:2841`, `:2866`, `:2872`,
      `:2897`), Legacy und `/api/*` bleiben `HTTP/1.0`. Prüfstand **c70e** (statisch: `HTTP/1.1` an genau diesen zwei
      Stellen, beide mit `Connection: close`; dynamisch: Körperlänge = `Content-Length`); Sabotagen „`Connection: close`
      in der `304` entfernt“ und „`HTTP/1.1` in einer `/api/`-Antwort“ ⇒ je FEHL; Ladeprobe am Prüfserver mit `HTTP/1.0`
      ⇒ FEHL (AKZ.2 (i)).
- [ ] **AKC.8 (bestehende Prüfstände nachgezogen)** — c65b und c65w (rufen `http_send_fs_file()` auf und erwarten die
      Kopfzeilen in **einem** Schreibvorgang) und die LittleFS-Attrappe (`seek`, zweimaliges bzw. Trailer-Lesen) sind
      auf den Stand R2 nachgezogen und grün; ihre Fallzahl ist gegen den Stand vorher gezählt, keine Fälle verloren
      (L235).

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
- [ ] **AKW.4 (Browser-Sitzungen mit Cache, Safari ungeprüft)** — Safari und Chrome wie AKM.3/AKM.4 (`osascript`); vor
      S0 einmal öffnen und beenden. Zusätzlich je Phase `app_full`, `app_not_modified`. Für Safari der erste Nachweis,
      ob er bei `HTTP/1.1` mit `ETag` nachfragt; bleibt `app_not_modified` bei 0, ist das ein Befund. Je Phase
      Gegenüberstellung mit G9.
- [ ] **AKW.5 (Auswertung und Tor)** — C72 nach `design.md` §5.4 **einem** von M1–M5 zugeordnet — auf G10, G9 und die
      P1-Probe daneben; C71 nach §5.5 **einem** von N0–N4, mit AKM.5, AKZ.9 und P9–P12. Beides ins Messprotokoll, vom
      `doc-writer` nachgeführt (nicht gleichzeitig, R3). **Erst danach** legt der Lead Ent-8, Ent-10, Ent-14 und
      Ent-17 vor.
- [ ] **AKW.6 (Gesundheit G10)** — wie AKM.6.

### Schritt R3 und G11 — Eingriff C72, Massnahme C71 (nach dem Tor)

AKE.* entfallen ohne C72-Eingriff (M4, M5 oder Ent-8 = keiner); AKS.* entfallen bei N4 oder ohne Massnahme nach
Ent-17; im Endstand dann „entfallen“ mit Grund. Entfallen beide, entfällt R3 samt G11.

- [ ] **AKE.1 (C72-Eingriff umgesetzt)** — Kandidat nach Ent-8 gemäss `design.md` §6. **K1:** Verbindung ohne Anfrage
      mit `abort()` statt `stop()`. **K2:** höchstens einmal je Sekunde PCBs in `tcp_active_pcbs` mit
      `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING, LAST_ACK oder — bei `TF_CLOSEPEND` — ESTABLISHED und
      CLOSE_WAIT, deren Ruhezeit die Frist aus Ent-14 überschreitet, mit `tcp_abort()` abbauen; PCBs mit
      `ClientContext` **oder** Listener (SYN_RCVD) **nie**; Zähler `tcp_reaped`. **K5:** `HTTP_TW_LIMIT` 1. Prüfstand
      **c72e**; Sabotagen (K1 `stop()`; K2 „PCB mit `callback_arg` ≠ NULL abgebaut“, „SYN_RCVD abgebaut“, „Frist
      entfernt“; K5 „Grenze 2“) ⇒ je FEHL; gegen den Stand R2 ⇒ FEHL.
- [ ] **AKE.2 (C72-Wirkung, G11)** — Browser-Sitzung Safari wie AKW.4: 180 s nach Beginn von S2 **und** von S3 je
      `free_heap` ≥ `free_heap`(S0) − 300 Byte; in `tcp_now_list` kein PCB mit `arg` = 0 ausser TIME_WAIT und keiner mit
      Ruhezeit über der Frist aus Ent-14; Heap-Zeilen im TIME_WAIT-Fenster der eigenen Lesung gesondert gekennzeichnet.
      Daneben L461 (−1'700), AKM.3, AKW.4. Chrome berichtet.
- [ ] **AKE.3 (C72 ohne Regression)** — drei `check-pwa.sh` ohne FEHLT und ohne `net::ERR_CONNECTION_RESET`;
      Ladeprobe warm wie AKW.2; Δ`write_lost_failed` = 0; Smoketest ohne FEHLT.
- [ ] **AKS.1 (C71-Massnahme umgesetzt)** — Kandidat nach Ent-17 gemäss `design.md` §7 (Empfehlung **L6**: erster
      Abgleich frühestens 300 s nach dem Start, `UPDATE_CACHE_START_MS` `http.cpp:13398`), begründet aus AKM.5, AKZ.9
      und P9–P12. Prüfstand **c71e** nach `design.md` §8; gegen den Stand R2 ⇒ FEHL.
- [ ] **AKS.2 (C71-Wirkung, G11)** — Ruhewert `max` der Heap-Zeile **≥ 5'120** (K30) (i) 300 s nach dem ersten
      Durchgang des Abgleichs in jedem von drei ESP-Starts ohne Browser, **einer davon mit erzwungenem Schreiben beim
      Start**, und (ii) 150 s nach einem `check-pwa.sh` und nach einer Ladeprobe — je mit dem Vermerk
      „Flash-Schreiben seit Start / beim Start“. Daneben AKM.5, L460 (4'672), L457 (7'184), L468 (6'192). Die Zeilen
      `- update cache vorher …` und `- update cache heap …` belegen Zeitpunkt (`up`) und dass `blk` über den Durchgang
      erhalten bleibt.
- [ ] **AKS.3 (C71 ohne Regression)** — nach einem Start ohne Änderung je Datei `unveraendert`, nach dem erzwungenen
      Schreiben `geschrieben` und danach `unveraendert`; keine `*.tmp` (`/api/fs_list`); Release Notes in PWA und Legacy
      sichtbar (mit L6 spätestens rund 300 s nach dem Start, wenn die Datei fehlte); `install-app.sh --check`
      vollständig; c65m, c65n grün bzw. auf die neue Startfrist nachgezogen.

### Übergreifend

- [ ] **AKZ.3 (Kodierung und Zeilenende)** — Vor jedem Patch festgestellt und im Taskbericht genannt (`http.cpp` UTF-8
      mit einer CRLF-Zeile, `stm32flash.cpp` UTF-8, `ESP-uclock.ino` gemischt; DIR-015). Der Diff enthält nur
      inhaltlich geänderte Zeilen; der Lead prüft mit `git diff --stat`.
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
      Messprotokoll; Zuwachs über das Paket höchstens **250 Byte** (Ent-11; gerechnet rund 80 mit Variante T, rund
      185 mit V, `design.md` §10); darüber anhalten und vorlegen. `app.js.gz` vor und nach R2.

**Zahl der Kriterien: 40** — AKZ 9 (davon AKZ.9 erfüllt, L468), AKI 4, AKM 7, AKC 8, AKW 6, AKE 3, AKS 3. Am Tor hängen
AKE.1–AKE.3 (Ent-8) und AKS.1–AKS.3 (Ent-17); an Ent-2 hängen die Varianten in AKC.1, AKC.3, AKC.4.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| Variante (b) und (c) für C70 | Ent-1 = (a); (c) nur nach G10.7 als neuer Nachtrag |
| `Last-Modified`/`If-Modified-Since` statt `ETag` | Zeitstempel im LittleFS unsicher; Rückfallebene nur, falls Safari nicht nachfragt (G10.7) |
| `HTTP/1.1` für Legacy, `/api/*`, Hinweisseiten; Keep-Alive, Chunked, Pipelining | nicht nötig (AKC.7) |
| Ein **kalter** Erstaufruf unter 1'072 | C70 ändert ihn nicht |
| Eine C71-Massnahme ohne belegte Ursache (N4) | Ziel 2 |
| L0 (PUYA-Puffer früh anlegen) als C71-Massnahme ohne `vendor=85` | L468 (b): K-a nicht bestätigt; L0 nur noch bedingt |
| **Ab-2** `ClientContext`-Hygiene in `httpclient_stop()` (`client = WiFiClient()`) | kein C71-Hebel (A.1); höchstens eigener Befund |
| **C67**, **C66**, **C68**, **C69** | eigene Befunde |
| Änderungen an `index.html`, `styles.css`, `manifest.webmanifest`, Icons | für (a) nicht nötig |
| Neuer Service Worker, Änderung an `sw.js` ausser `CACHE_NAME` | unter `http://` ohne Wirkung |
| `Cache-Control: max-age` ohne Versionierung | verletzt „Update sofort wirksam“ |
| Höchstzahl Verbindungen je Gegenstelle (K4) | Risiko für parallele Asset-Ladungen |
| Ein C72-Eingriff vor dem Tor | Mechanismus nicht belegt |
| Änderungen an der Legacy-Oberfläche, am STM | Invariante bzw. nicht betroffen |
| Browser steuern, Schutz für `app_file_upload` (C61), Backlog-Schwellen aus E4 | nicht möglich bzw. eigene Specs/Tasks |
| L462 (Laden schreibt `ambilight_online`, B61) | eigener Befund |
| Nebenbefunde aus A.1: `http.cpp:7866` `LittleFS.remove()` **vor** `LittleFS.begin()` (`:7888`) ist wirkungslos; `ESP-uclock.ino:291` `LittleFS.format()` steht in `#if 0` | eigene Befundzeilen (Task R1.10), hier nicht behoben |

---

## Betroffene Laufzeiten

- [ ] **ESP8266** — R1: `http.cpp`, `base.h`, `ESP-uclock.ino`; R2: `http.cpp`, mit V auch `stm32flash.cpp` und
      `ESP-uclock.ino`, `base.h`; R3: `http.cpp`, `base.h`, gegebenenfalls `ESP-uclock.ino`; je ein OTA
- [ ] **PWA** — R2: `app.js` (eine Zeile), `sw.js` (`CACHE_NAME`); ein Upload nach dem OTA
- [ ] **STM32** — keine
- [ ] **Werkzeug** — `ladeprobe` (liegt vor), `browser-sitzung` (mit `manuell`), P1-Probe; geändert `check-pwa`,
      `smoke-device.sh`, `watch-log.sh`; Prüfstände c72i, c71i, c70e, c70p, c72e, c71e; nachgezogen c65b, c65w, c65n,
      LittleFS-Attrappe
- [ ] **Dokumentation** — `BEFUNDE.md`, `TESTPLAN-PWA.md`, `CHANGELOG.md`
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
Einzelfreigabe, auch nachts.“), vom `spec-writer` nachgelesen. Probe U und die ESP-Starts bleiben drin. Es bleibt
**eine** Messung zur Zeit; die Gefahrentabelle gilt — berührt wird keine ihrer Dateien.

**Offen, vor R2.1:**

| Ent | Gegenstand | Optionen | Bewertung und Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-2** | Quelle des `ETag` | **T** — `<gz-grösse>-<gzip-trailer>`, je Anfrage aus der Datei; **V** — `<grösse>-<CRC-32/MPEG-2>` über den ganzen Inhalt, RAM-Tabelle mit Invalidierung an 31 Stellen | **T** (`design.md` §4.4): Der gzip-Trailer **ist** eine CRC über den entpackten Inhalt, geschrieben vom Kompressor — für jede gültige `.gz` so trennscharf wie V. Weil er bei jeder Anfrage gelesen wird (ein `seek`, 8 Byte), entfallen Tabelle, Invalidierung, Zeile `- app etag` und 104 Byte RAM — und mit ihnen das Restrisiko einer übersehenen Schreibstelle. Nachteile: Eine **ungültige** `.gz` (kein gzip-Kopf) bekommt keinen `ETag` (sichere Richtung); zwei `.gz` mit gleichem entpacktem Inhalt und gleicher Grösse, aber verschiedenen Bytes, teilen einen `ETag` — der Browser zeigt dann denselben Inhalt, also kein Fehler. V bleibt Rückfallebene, falls das `seek` je Anfrage messbar kostet (AKC.4) | Lead |

**Offen bis zum Tor (G10.6):**

| Ent | Gegenstand | Optionen | Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-8** | Eingriff C72 | K1, K2, K1 + K2, K5 als Zusatz, kein Eingriff | nach `design.md` §5.4: M1 ⇒ K2; M2 ⇒ K1; M3 ⇒ K1 + K2; M4/M5 ⇒ kein Eingriff. **Widerlegt die P1-Probe P1, ist K2 zwingend** | Nutzer |
| **Ent-10** | schreibender Zugriff auf lwIP-Listen (für K2) | erlauben; nicht | **erlauben, nur mit K2, nur PCBs mit `callback_arg == NULL`** (kein `ClientContext`, kein Listener) | Nutzer |
| **Ent-14** | Frist in K2 | 5 s; 20 s | **5 s**, endgültig mit den Ruhezeiten aus G9/G10 | Nutzer |
| **Ent-17** | Massnahme C71 | L6, L2, L0, L4b, keine (`design.md` §7) | nach `design.md` §5.5: **N1 ⇒ L6** (Abgleich frühestens 300 s nach dem Start — eine Konstante; nutzt den Zustand, in dem L468 (c) mit Abruf und Schreiben keine Spur zeigte), **L2** (Abgleich vor `http_server_begin()`) nur, falls L6 in G11 die 5'120 nicht erreicht oder P11 eine frühe Ausführung nahelegt; N0 (nur bei `vendor=85`) ⇒ L0; N2 ⇒ L2; N3 ⇒ L4b; N4 ⇒ keine | Nutzer |

**Offen vor R3:**

| Ent | Gegenstand | Optionen | Empfehlung | Wer |
|---|---|---|---|---|
| **Ent-16** | Platz der C71-Massnahme | (A) in R3 mit dem C72-Eingriff; (B) eigenes R4; (C) in R2 | **A** — getrennte Messgrössen (C71: Ruhewert `max` nach Start ohne Browser; C72: `free`-Lücke in einer Sitzung), ein OTA weniger. **B**, wenn ein getrennter Rückweg wichtiger ist. **C** verworfen: G10 soll den Cache getrennt zeigen. Mit **L6** (eine Konstante) ist der C71-Teil zudem klein | Nutzer |
