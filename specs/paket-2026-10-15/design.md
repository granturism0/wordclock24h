# Design — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge am selben Tag mit den Entscheiden des
Nutzers, den Lead-Entscheiden, dem Prüfserver-Befund des Leads zu `HTTP/1.0`, dem **Nachtrag A.2** (Ergebnisse des
`firmware-analyst`) und der **Trennmessung C71** des Leads (L468). **Status: freigegeben am 15.10.2026 bis aufs Tor;
Nachtrag A.2 zur Freigabe.** Was A.2 und die Trennmessung ändern und widerlegen: §17. Offen Ent-2 (vor R2.1), Ent-8,
Ent-10, Ent-14, Ent-17 (Tor), Ent-16 (vor R3). Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen** wie in `requirements.md`: **✔ SW** am Code `72cfda7` bzw. Core 3.1.2 nachgelesen; **✔ B** in
`BEFUNDE.md`; **✔ L** vom Lead gemessen; **✔ FA** vom `firmware-analyst` am Core belegt; **◐ FA** vom
`firmware-analyst` als plausibel gemeldet (lwIP 2.1.3 nur binär, Upstream aus dem Gedächtnis); **◐** abgeleitet;
**●** gemeldet, vor der Umsetzung zu prüfen.

---

## 1. Befund mit Verifikationsstatus

| Aussage | Status | Fundstelle / Bemerkung |
|---|---|---|
| Volles Neuladen mit einem Client reisst K30: `low=752`, `max` 3'584 | **✔ B** | L466 |
| Jedes Öffnen holt alle Assets, rund 200 KB, davon `app-app.js.gz` 163'621 Byte | **✔ B** / **✔ SW** | C70, E3.0; `http.cpp:2067-2078` |
| Auslieferung `HTTP/1.0 200 OK`, gzip, `no-cache`, `Connection: close`, kein Validator | **✔ SW** / **✔ L** | `http.cpp:2067-2077`; `curl -D` |
| Chrome nimmt einen `ETag` aus `HTTP/1.0` nicht als Validator; mit `HTTP/1.1` schon | **✔ L** / **◐** (Ursache) | Prüfserver 15.10.2026 22:21–22:23; Safari ungeprüft |
| Nur `http_send_fs_file()` muss auf `HTTP/1.1`; übrige `/app/`-Antworten bleiben 1.0; kein Keep-Alive | **✔ FA** | `http.cpp:2067`; `:2791`, `:2841`, `:2866`, `:2872`, `:2897`; `:2075`, `:2077`, `:2236` |
| CDP meldet bei `304` in `responseReceived` den Cache-Status `200` | **✔ L** | Leitungsstatus in `responseReceivedExtraInfo.statusCode` |
| `If-None-Match` wird nicht ausgewertet; Query vor `http_app()` abgetrennt; ESP kennt die installierte App-Version nicht | **✔ SW** | `http.cpp:16783-16802`, `:16812-16823`, `:111`, `:14294` |
| Kein Service Worker über `http://`; `layout-previews.json` mit `no-store` | **✔ SW** | `app.js:2135-2163`, `:11655` |
| `install-app.sh` und Auto-Installation laden `index.html` zuerst | **✔ SW** | `tools/install-app.sh:36-38`; `http.cpp:747-786` |
| Core-`crc32()` ist CRC-32/MPEG-2 (0x04C11DB7, MSB zuerst, Start 0xFFFFFFFF, kein Schluss-XOR), bitweise mit `pgm_read_byte` | **✔ FA** / **✔ SW** | `cores/esp8266/crc32.cpp:26-43`; Kosten ◐ FA 0,15–0,3 s für `app.js.gz` |
| 31 schreibende LittleFS-Stellen, vollständig; jedes `LittleFS.begin()` formatiert bei Fehlschlag (AutoFormat) | **✔ FA** | Liste §4.4; AutoFormat durch Prüfung auf Existenz und Grösse > 0 abgedeckt |
| `http.cpp:7866` `LittleFS.remove()` **vor** `LittleFS.begin()` (`:7888`) ist wirkungslos | **✔ FA** / **✔ SW** | Nebenbefund |
| `ESP-uclock.ino:291` `LittleFS.format()` steht in `#if 0` | **✔ SW** | `ESP-uclock.ino:287-295` |
| Nach dem Start fällt `max` von 7'184 auf 4'672 im Fenster des Startabgleichs und bleibt | **✔ B** | L460, L457 |
| **Schreiben im laufenden Betrieb hinterlässt nichts** — erstes Flash-Schreiben seit dem Start ohne Abruf (`fs_remove`, 22:40:02) und Schreiben mit Abruf (`refresh=1`, 22:43:38, `upd-wc-list.txt geschrieben n=69`) | **✔ L** | L468: Heap vorher 7'136/6'192, danach wieder 7'136/6'192; kein ΔFree um −264, `max` unverändert |
| **Start mit `unveraendert`** erzeugt keine Zerstückelung | **✔ L** / **✔ B** | V3 in T2 (L466: AKR.21 `unveraendert`); Ruhewert 6'192 (L468) |
| Erstes Flash-Schreiben legt bei PUYA (0x85) einen 256-Byte-Puffer an, nie freigegeben | **✔ SW** (Core) | `Esp.cpp:726-731`, `:813-814`, `:862-864` |
| …als Ursache von C71 | **nicht bestätigt** (L468) | (b) zeigte kein ΔFree ≈ −264 — kein PUYA-Chip oder anderer Weg |
| **C71 entsteht nur, wenn beim Start geschrieben wird** | **◐** (aus L460, L468, V3) | Kandidaten müssen an den Startzustand: was in den ersten rund 60 s lebt |
| Der statische `WiFiClient client` des httpclient hält nach dem ersten `connect()` einen `ClientContext`, bis zum nächsten `connect()` | **✔ SW** | `httpclient.cpp:16`, `:319-321`; Zähler `httpclient_connects` (E3 I-1) |
| Abgleich frühestens 60 s nach dem Start | **✔ SW** | `UPDATE_CACHE_START_MS` `http.cpp:13398`, `:13978-13983` |
| ESP-seitige Flash-Schreiber ausser LittleFS: `EEPROM.commit()` | **✔ SW** | `eepromdata.cpp:187`; `EEPROM.begin(512)` `ESP-uclock.ino:297` |
| Safari-Sitzung: `free` −1,7 KB über Minuten; frei erst beim Beenden | **✔ B** / **●** | L461 |
| `stop()` ⇒ `tcp_close()` ⇒ FIN, `TF_RXCLOSED`; FIN_WAIT_2 läuft rund 20 s nach dem letzten Segment ohne RST ab | **✔ SW** / **◐ FA** (P1) | `ClientContext.h:73-92`; `tcp_priv.h:131`; P1-Probe §5.6 |
| FIN_WAIT_1, CLOSING, LAST_ACK ohne eigene Frist, nur Wiederholungen bis `TCP_MAXRTX` 12 | **◐ FA** | `lwipopts.h:1258` |
| `pcb->tmr` je angenommenem Segment, nicht nach `TF_RXCLOSED`, nicht beim Zustandswechsel | **◐ FA** (P4) | §3.1 |
| SYN_RCVD trägt den Listener als `callback_arg`; ESTABLISHED mit `callback_arg == NULL` möglich (`TF_CLOSEPEND`) | **◐ FA** (P3, P8) | `lwip/tcp.h:255` |
| Tote `ClientContext` in `_unclaimed` stapeln sich, während `loop()` steht; `!http_client` kehrt ohne Zähler zurück | **◐ FA** / **✔ SW** | `http.cpp:16598-16601`; `WiFiServer.h:76` |
| `MEMP_NUM_TCP_PCB` begrenzt bei `MEMP_MEM_MALLOC` nichts; TIME_WAIT im Core ≤ 6, nach `tcp_tw_limit()` 2 | **✔ FA** (P2) | Kommentar `http.cpp:325` falsch |

---

## 2. Zuschnitt, Releases, Reihenfolge (Ent-9: drei Releases)

| Release | Inhalt | Laufzeit | danach | Schreiber |
|---|---|---|---|---|
| **R1** | **I-5** (§3.1, §3.2) und **I-6** (§3.3) | ESP | **G9** ohne Cache, mit **P1-Probe** | `esp-developer` |
| **R2** | **C70** (§4): `ETag` nach Ent-2, `304`, `HTTP/1.1` für App-Assets; PWA-Teil | ESP **und** PWA | **G10** mit Cache, **Tor** | `esp-developer`, `pwa-developer` |
| **R3** | **Eingriff C72** (§6, Ent-8), **Massnahme C71** (§7, Ent-17); Platz von C71 nach Ent-16 | ESP | **G11** | `esp-developer` |

Instrumente zuerst, dann Ausgangsstand **ohne** Cache, dann Cache und dieselben Messungen **mit** Cache, dann Tor auf
M1–M5 und N0–N4, dann Eingriff. Jedes Release einzeln mit Commit, Tag und Einspielen. **Mit Ent-16 B** wandert C71 in
ein eigenes R4 nach G11. R3 findet statt, sobald C72 **oder** C71 einen Eingriff hat.

**Abhängigkeiten nach R3b:** Alle ESP-Teile schreibt **ein** `esp-developer` nacheinander. Der PWA-Teil in R2 darf
parallel laufen. Code für R2 darf entstehen, während G9 läuft; gebaut und eingespielt erst danach. Code für R3 erst
nach dem Tor.

---

## 3. Instrumente (R1)

### 3.1 I-5 — laufende Verbindungszustände in `device_ready` (C72)

**Hinter `api_orphan_skipped`** (`http.cpp:14770`):

| Feld | Inhalt |
|---|---|
| `tcp_now_states` | 11 Zahlen: PCBs je Zustand in `tcp_active_pcbs`, Index = `enum tcp_state`, sättigend 255; die eigene Verbindung mitgezählt |
| `tcp_now_tw` | Länge von `tcp_tw_pcbs` |
| `tcp_now_pending` | `accepts_pending` am Listen-PCB |
| `tcp_now_unclaimed` | Länge der Liste `_unclaimed` der `WcHttpServer` (✔ FA: `ClientContext::next()` öffentlich; `<include/ClientContext.h>` einbinden) — lebende **und** tote Kontexte |
| `tcp_now_self_port` | `http_client.remotePort()` dieser Anfrage |
| `tcp_now_list` | höchstens 8 Felder `[zustand, gegenstelle, port, ruhe_s, nrtx, arg]`, erst `tcp_active_pcbs`, dann `tcp_tw_pcbs` |
| `tcp_now_more` | PCBs, die nicht in die Liste passten |
| `http_accept_dead` | Zähler seit dem Start, sättigend: `accept()` holte einen Kontext aus `_unclaimed`, dessen Gegenstelle schon geschlossen hatte (`!http_client` bei `http.cpp:16598-16601`); gezählt **nur**, wenn vor `accept()` `_unclaimed` nicht leer war |

- **`ruhe_s` (◐ FA P4):** `(tcp_ticks − pcb->tmr) × TCP_SLOW_INTERVAL / 1000`. `tmr` wird bei jedem angenommenen
  Segment gesetzt, **aber nur, solange `TF_RXCLOSED` nicht gesetzt ist**, und **nicht** beim Zustandswechsel. Für einen
  vom ESP geschlossenen PCB heisst `ruhe_s` „seit dem letzten empfangenen Segment vor unserem `close()`“. Ein
  TIME_WAIT-PCB endet rund 120 s nach dem Schliessen; sein `ruhe_s` misst nicht die Zeit in TIME_WAIT.
- **`arg` (◐ FA P3/P8):** 0 = `callback_arg == NULL`; 1 = `ClientContext`; 2 = Listener (SYN_RCVD trägt den
  `callback_arg` des Listeners, erkennbar am Vergleich mit dem von `_listen_pcb`). ESTABLISHED oder CLOSE_WAIT mit
  `arg` = 0 ist ein vom ESP geschlossener PCB, dessen FIN nicht eingereiht werden konnte (`TF_CLOSEPEND`,
  `lwip/tcp.h:255`).
- `nrtx`: Wiederholungen des laufenden Segments. `gegenstelle`: letztes Oktett (Ent-5).

**Ohne Abgabe:** Listen in einen Stackpuffer (rund 90 Byte), **bevor** das erste `http_send()` läuft. Kein `String`,
kein Heap; Gleichzeitigkeit wie I-4 (`http.cpp:327-330`). **Kappung auf 8** (P2: nichts begrenzt die Zahl). Der
Kommentar `http.cpp:325` wird in R1.1 berichtigt: aktive PCBs unbegrenzt, TIME_WAIT im Core höchstens 6 (Core-Patch
ruft `TCP_TW_LIMIT(5)`, dazu der Versatz um eins), nach dem eigenen `tcp_tw_limit()` 2.

### 3.2 I-5 — Heap-Zeile (Ent-4)

```
- heap free=<n> max=<n> ovr=<n> low=<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>
```

Zählfunktion in `http.cpp`, Prototyp in `base.h`. Längste Form 105 ≤ 119; Puffer `line[80]` ⇒ 120. Zählt ohne eigene
Verbindung, jede Minute. `watch-log.sh` liest die bestehenden Felder über Muster, die einen Zusatz vertragen (◐ —
Gegenprobe AKZ.2 (iv)). `ESP-uclock.ino` hat gemischte Zeilenenden: byte-genau.

### 3.3 I-6 — Heap im Startabgleich, Startzustand, Flash-Kennung (C71)

Nach L468 ist die Frage nicht mehr „was tut das Schreiben“, sondern „**zwischen was legt es sich beim Start**“. I-6
misst deshalb den Abgleich **und** den Zustand unmittelbar davor.

**(a) Startzustand, einmal je Durchgang unmittelbar vor Schritt 1:**

```
- update cache vorher up=<s> free=<n>/<blk> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> hc=<httpclient_connects>
```

- `up`: Sekunden seit dem Start. `tcp=` wie die Heap-Zeile (Zählfunktion aus I-5). `hc`: der vorhandene Zähler
  `httpclient_connects` (E3 I-1) — ist er 0, gibt es noch keinen `ClientContext` des statischen httpclient
  (`httpclient.cpp:16`); ist er > 0, liegt einer seit dem letzten `connect()` auf dem Heap. Längste Form rund 90 Zeichen.

**(b) Verlauf, eine Zeile je Durchgang, am Ende oder beim Abbruch:**

```
- update cache heap v=<free>/<blk> 1=<free>/<blk> 2=<free>/<blk> 3=<free>/<blk> 4=<free>/<blk> 5=<free>/<blk> n=<free>/<blk>
```

- `v` vor Schritt 1; `1`–`3` nach jeder Versionszeile; `4`, `5` nach Notes bzw. Liste, also **nach**
  `httpclient_stop()` und `LittleFS.end()`; `-` für nicht gelaufen; `n` nach dem letzten Schritt. Längste Form 117.
- Je Schritt `free` **und** `blk`: Bleibt `blk` nach einem Schreibschritt um ≥ 1'000 tiefer, obwohl `free` fast
  zurückkommt, liegt eine langlebige kleine Belegung mitten im grossen Block — das Bild von N1.

**(c) `- heap erste anfrage free=<n> max=<n>`** einmal je Start beim ersten angenommenen Anfrage-Eingang.

**(d) `- flash id=<hex6> vendor=<hex2>`** einmal in `setup()`, vor WLAN und `EEPROM.begin()` (`ESP-uclock.ino:297`).
Nach L468 nur noch für N0 nötig (bedingt), aber billig.

Werte von (b) in einem statischen Feld (rund 30 Byte). Je Zeile einmal formatiert, Mitschnitt und Ring. Bestehende
Zeilen `- update cache <datei> …` unverändert; die neuen enthalten kein `fehlgeschlagen`.

---

## 4. C70 — Asset-Cache, Variante (a) (Ent-1)

### 4.1 Varianten

**(a)** `ETag`, `If-None-Match` ⇒ `304`, `no-cache` bleibt — **entschieden**. **(b)** versionierte URLs mit langem
`max-age`. **(c)** beides — nur nach G10.7.

### 4.2 Was beim zweiten Öffnen noch über die Leitung geht

N ≈ 7 `/app/`-Anfragen je Öffnen (◐). Kopf `200` rund 190 Byte, `304` rund 110 Byte (◐).

| Variante | Byte vom ESP beim zweiten Öffnen | Verbindungen |
|---|---|---|
| heute | ≈ 200 KB + N × 190 | N |
| **(a)** mit `HTTP/1.1` und PWA-Teil | ≈ N × 110 ≈ **0,8 KB**; Prüfserver: 290 statt 4'045 Byte für Dokument + Asset (✔ L) | N, je sehr kurz |
| (b) | `index.html.gz` + nicht versionierte Referenzen | 2–4 |
| (c) | ≈ 0,3–0,5 KB | 1–4 |

### 4.3 Bewertung der Varianten (Grundlage von Ent-1)

| | (a) | (b) | (c) |
|---|---|---|---|
| **Korrekt nach Update** | **ja, durch Bau** | **nein:** `index.html` wird zuerst hochgeladen; dazwischen landet die alte `app.js` ein Jahr lang unter der neuen URL | wie (b) |
| **Versionsstellen** | keine neue | fünfte Stelle | wie (b) |
| **Dateien** | `http.cpp`, eine Zeile `app.js` | `index.html`, `sw.js`, `http.cpp`, `Makefile` | alle |
| **Legacy** | unberührt (`HTTP/1.0`) | unberührt | unberührt |

### 4.4 Umsetzung

#### `ETag`-Quelle (Ent-2, wieder offen)

| | **T — gzip-Trailer** (Lead-Empfehlung) | **V — Voll-CRC** |
|---|---|---|
| **Wert** | `"<gz-grösse>-<16 Hexziffern>"`: die letzten 8 Byte der `.gz` — CRC-32 (zlib-Variante) und Länge des **entpackten** Inhalts (RFC 1952) | `"<grösse>-<8 Hexziffern>"`: Core-`crc32()` über den gespeicherten Inhalt — **CRC-32/MPEG-2** |
| **Wann** | bei **jeder** Antwort: `fp.size()`, `seek` ans Ende − 8, 8 Byte, `seek` an den Anfang; erste 2 Byte auf `1f 8b` prüfen | einmal je Asset nach Start oder Schreiben; RAM-Tabelle |
| **Kosten** | je Antwort ein `seek` und 10 Byte (◐ ms) | einmalig 0,15–0,3 s für `app.js.gz` (◐ FA), Zeile `- app etag`, 104 Byte RAM |
| **Gültigkeit** | **durch Bau** — kein Restrisiko einer übersehenen Schreibstelle | Grössenvergleich + `app_etag_invalidate()` an jeder der 31 Stellen |
| **Trennschärfe** | für jede **gültige** `.gz` wie V; abgeschnitten ⇒ anderer Wert. Gleicher entpackter Inhalt bei gleicher Grösse, aber anderen Bytes ⇒ gleicher `ETag`, gleicher angezeigter Inhalt — kein Fehler | über die gespeicherten Bytes |
| **Ungültige `.gz`** | kein `ETag` (sichere Richtung) | `ETag` über die Bytes |
| **Werkzeuge** | erwarteter Wert aus den letzten 8 Byte der Repo-Datei | nur CRC-32/MPEG-2, nicht zlib/`binascii` |

**Empfehlung T**; V als Rückfallebene, falls AKC.4 eine messbare Mehrdauer je Anfrage zeigt. Die `.gz` entstehen über
`make app-gz` (`gzip -9`); ein gültiger Trailer ist dort sicher (◐).

#### Statuszeile `HTTP/1.1` — nur für App-Assets (AKC.7)

Nur die Kopfzeilen in `http_send_fs_file()` (`http.cpp:2067`) und die neue `304` tragen `HTTP/1.1` (✔ FA). Ohne
`ETag` geht `HTTP/1.1 200` ohne `ETag` hinaus.

| Risiko | Bewertung | Massnahme |
|---|---|---|
| Keep-Alive-Erwartung unter 1.1 | real ohne `Connection: close` | `Connection: close` in beiden (heute `:2077`), Schliessen wie heute (`:2236`); c70e statisch |
| Körperende durch Verbindungsende | gering: `Content-Length` (`:2075`), Abbruch mit RST | exakt; c70e vergleicht |
| `304` mit Körper oder `Content-Length` | verwirrt den Browser | ohne beides |
| `Date` | ohne verlässliche Uhr nicht zu senden (RFC 9110 §6.6.1, ◐) | kein `Date` |
| Werkzeuge mit Prüfung auf `HTTP/1.0` | ◐ nicht durchsucht | Task R2.6 |
| Prüfstand-Attrappen: c65b/c65w erwarten Kopfzeilen in **einem** Schreibvorgang; LittleFS-Attrappe ohne `seek` | ✔ FA | Lead zieht nach (AKC.8, R2.4a) |
| Legacy, `/api/*`, Hinweisseiten | bleiben 1.0 | AKC.5, Sabotage |
| Safari | ungeprüft | AKW.4; sonst `Last-Modified` (G10.7) |

#### Schreibstellen (nur Variante V tragend; ✔ FA vollständig)

`http.cpp` 27 (`:1532`, `:2332`, `:6824`, `:6874`, `:7213`, `:7220`, `:7227`, `:7234`, `:7320`, `:7374`, `:7763`,
`:7776`, `:7866`, `:7890`, `:7903`, `:10381`, `:13132`, `:13677`, `:13759`, `:13766`, `:13843`, `:13848`, `:15151`,
`:15192`, `:15201`, `:15585`, `:15617`), `stm32flash.cpp` 3 (`:1282`, `:1315`, `:1410`), `ESP-uclock.ino` 1 (`:291`,
in `#if 0`) — **31**. App-Dateien schreiben oder löschen können (✔ FA): Auto-Installation (`:2325` ⇒ `:6824`/`:6874`,
`:2332`), Legacy `savedwnfil`/`dwntable` (`:7342`/`:7350`), Legacy-Löschen (`:7374`), `app_file_upload`
(`:7890`/`:7903`), `fs_remove` (`:13132`), Formatieren (`:7320`, `:10381`). Nebenbefund `:7866`: löscht vor
`LittleFS.begin()` (`:7888`), wirkungslos.

#### `If-None-Match`, Antworten, Zähler

`If-None-Match` in der Kopfzeilenschleife (`http.cpp:16783-16802`) in einen statischen Puffer von 36 Byte, je Anfrage
geleert; Kopfname ohne Rücksicht auf Gross-/Kleinschreibung; `W/` übergangen, Komma-Liste, `*` kein Treffer,
überlange Werte treffen nicht.

```
HTTP/1.1 200 OK\r\nContent-Type: <typ>\r\nContent-Encoding: gzip\r\nContent-Length: <n>\r\n
ETag: "<wert>"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n<körper>

HTTP/1.1 304 Not Modified\r\nETag: "<wert>"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n
```

- **`304` nur**, wenn der Wert trifft, die Datei mit Grösse > 0 besteht (`:2049`) und beim Index `app_complete` wahr
  ist (`:2826-2829`). Nur Weisslisten-Assets als `.gz`.
- Scheitert ein `seek`, wird nichts ausgeliefert, was nicht vom Dateianfang stammt: Datei schliessen, neu öffnen,
  sonst Ende wie bei einem Lesefehler heute.
- **Zähler** hinter I-5: `app_not_modified`, `app_full`, mit V `app_etag_calc`.

### 4.5 Was C70 nicht anfasst

`sw.js` (nur `CACHE_NAME`, R2); `reloadAppPage()` (`app.js:13225-13253`); Safaris Zurück-Cache (◐).

### 4.6 PWA-Teil (Ent-3: immer)

`loadLayoutPreviewRowsMap()` (`app.js:11647-11664`): `"no-store"` ⇒ `"no-cache"`. Firmware zuerst, dann PWA.

---

## 5. Messungen

### 5.1 Ladeprobe (`ladeprobe`, Lead; liegt vor)

Dauerhaftes Profil: kalt, dann fünfmal warm (dreimal `Page.reload` ohne `ignoreCache`, zweimal `Page.navigate`), je
vorher `heap_low_reset=1`, danach lesen. Je Laden: Leitungsstatus (`responseReceivedExtraInfo`), `If-None-Match`
gesendet (`requestWillBeSentExtraInfo`), `encodedDataLength`, `heap_low_block`, `heap_low_path`, I-4, `app_*`.
`--erwarte-304`. Keine Haltegrenze. Zu jedem Lauf „Flash-Schreiben seit Start“.

### 5.2 Probe U (AKW.3)

`layout-previews.json` um Leerraum ergänzt, `gzip -9`, über `app_file_upload`; danach Original zurück. Ohne
Einzelfreigabe (`CLAUDE.md:416`).

### 5.3 Browser-Sitzung (`browser-sitzung`, Lead)

Die Uhr gehört der Messung; Safari und Chrome zu Beginn beendet; Wache läuft.

| Phase | Was | Dauer |
|---|---|---|
| **S0** | Ruhe | 180 s |
| **S1** | Browser öffnet `http://<uhr>/app/`, sichtbar | 120 s |
| **S2** | neuer Tab `about:blank` vorne — PWA verborgen | 180 s |
| **S3** | PWA-Tab geschlossen, Browser läuft | 300 s |
| **S4** | Browser beendet | 180 s |

**Betriebsarten:** `osascript` — das Skript bedient; macOS-Erlaubnis zur Automation einmal durch den Nutzer.
**`manuell`** (Review M6) — das Skript nennt Phase und Handgriff, der Nutzer bedient auf seinem Gerät und bestätigt, das
Skript liest; nur die ESP-Seite. In G10 vor S0 einmal öffnen und beenden.

**Lesung am Phasenende:** `device_ready` ohne Parameter, `netstat -anv -p tcp | grep <uhr>`, `lsof -nP -i @<uhr>`. **Für
jede eigene Lesung** gibt das Werkzeug ihren lokalen Port und das Ende ihres TIME_WAIT-Fensters aus (Lesezeit + 120 s,
Review M5). Dazwischen keine Lesung.

### 5.4 Entscheidungsregel C72 (AKW.5)

„Lücke“ = `free_heap`(S0) − `free_heap`(Phase). **Eigenes Fenster** = TIME_WAIT-Fenster der eigenen Lesungen; ein
TIME_WAIT-Eintrag mit dem Port einer eigenen Lesung zählt nie, und **Heap-Zeilen in einem eigenen Fenster zählen für M2
nicht**. Zugeordnet wird auf G10; G9 und die P1-Probe stehen daneben.

| Ergebnis | Bild | Kandidat (Ent-8) |
|---|---|---|
| **M1** — geschlossen, nicht abgebaut | in S2 oder S3 ein PCB mit `arg` = 0, Gegenstelle Mac, in **FIN_WAIT_1, CLOSING oder LAST_ACK mit `nrtx` > 0**, oder ESTABLISHED/CLOSE_WAIT (`TF_CLOSEPEND`), in zwei Lesungen hintereinander; Lücke ≥ 500; in S4 verschwunden. **FIN_WAIT_2 mit `ruhe_s` ≥ 30** gehört auch hierher, ist aber zugleich ein Beleg **gegen P1** | **K2** |
| **M2** — Strom von Vorab-Verbindungen | in S2/S3 steigen `nrt` **oder** `http_accept_dead` zusammen um ≥ 1 je Minute; in ≥ 2 Heap-Zeilen **ausserhalb** eigener Fenster `fw2`/`tw` > 0; kein PCB aus M1; Lücke ≥ 500; in S4 stehen beide still | **K1** |
| **M3** — beides | M1 und M2 | **K1 + K2** |
| **M4** — angenommen und hängend | ein PCB mit `arg` = 1, nicht die eigene Lesung, ESTABLISHED oder CLOSE_WAIT, `ruhe_s` ≥ 30 — oder `tcp_now_unclaimed` ≥ 1 in zwei Lesungen hintereinander. **SYN_RCVD (`arg` = 2) zählt nicht** | **kein Eingriff**; eigener Befund |
| **M5** — keine Verbindung | Lücke ≥ 500, aber ausser eigenen Einträgen und ≤ 2 TIME_WAIT nichts — **oder** keine Lücke ≥ 500 | **kein Eingriff** |

Ist TIME_WAIT ausserhalb eigener Fenster in jeder Lesung voll und trägt sichtbar zur Lücke bei, kommt **K5** als Zusatz
in Frage. **Widerlegt die P1-Probe P1**, ist **K2 zwingend**.

### 5.5 Startmessung und Entscheidungsregel C71 (AKM.5, AKS.2) — der entscheidende Schritt für C71

**Was L468 festlegt (✔ L):** Schreiben im **laufenden Betrieb** — mit und ohne Abruf — hinterlässt nichts; ein Start
mit `unveraendert` auch nicht (V3). C71 entsteht also nur, wenn **beim Start** geschrieben wird. G9.6 prüft das unter
kontrollierten Bedingungen und zeigt mit I-6 (a), was zu diesem Zeitpunkt lebt.

**Ablauf je Start:** kein Browser, keine HTTP-Lesung in den ersten 5 min (sie wäre eine Anfrage und verschöbe den
Abgleich um 5 s und den Messpunkt `erste anfrage`); ESP-Neustart über den Weg aus V3 (STM-Reset, L183) oder
`/api/local_esp_restart`; Mitschnitt der ersten 5 min (`- flash id`, Heap-Zeilen, `- update cache vorher …`,
`- update cache …`, `- update cache heap …`); danach Smoketest samt Update-Quelle und `- heap erste anfrage`.

**Drei Starts:** zwei ohne Änderung (erwartet `unveraendert`), einer mit **erzwungenem Schreiben** beim Start (vorher
`/upd-wc-list.txt` per `/api/fs_remove` löschen; erwartet `geschrieben`). Je Start „Flash-Schreiben seit Start“.

**Trennmessung (erledigt, ✔ L, L468):** (b) erstes Schreiben ohne Abruf und (c) Schreiben mit Abruf im laufenden
Betrieb — beide ohne bleibende Spur.

| Ergebnis | Bild | Kandidat (Ent-17) |
|---|---|---|
| **N0** — PUYA-Puffer (bedingt) | nur wenn `vendor=85`: im ersten Schritt mit Flash-Schreiben ΔFree ≈ −264 ohne Erholung. **Nach L468 (b) unwahrscheinlich** | **L0** |
| **N1** — Startzustand | `blk` fällt nur im Start **mit** Schreiben, in Schritt 4 oder 5, um ≥ 1'000 und erholt sich nicht; in den Starts mit `unveraendert` nicht; `- update cache vorher` zeigt, was zu diesem Zeitpunkt lebt (`up` rund 60, `hc`, `tcp=`) | **L6**, sonst **L2** |
| **N2** — auch ohne Schreiben | Fall auch in einem Start mit `unveraendert` — widerspricht V3 | **L2** nach P11 |
| **N3** — auch im laufenden Betrieb | Fall bei einem Schreiben, das nicht beim Start liegt — widerspricht L468 (c) | **L4b** |
| **N4** — nicht im Abgleich | `v` schon geteilt (oder `erste anfrage` klein), oder kein Schritt fällt um ≥ 1'000 | **keine Massnahme** |

### 5.6 P1-Probe (AKM.7)

Ein Python-Socket vom Mac sendet `GET /api/device_ready HTTP/1.0\r\n\r\n`, liest die Antwort bis zum FIN des ESP und
schliesst **nicht**. Aus einer zweiten Verbindung `device_ready` nach 10 s und nach 25 s; dazu `netstat` auf der
Mac-Seite. **Erwartet unter P1:** nach 10 s ein Eintrag FIN_WAIT_2 (Zustand 6), `arg` = 0, Gegenstelle Mac, Port =
lokaler Port der Probe; Mac-Seite CLOSE_WAIT; nach 25 s ist der Eintrag weg (ohne RST). **Noch da nach 25 s ⇒ P1
widerlegt, K2 zwingend.** Die zweite Lesung wird als eigene markiert. Danach schliesst die Probe ihren Socket.

---

## 6. Eingriffskandidaten C72 (R3, Ent-8)

| | Mechanismus | Wirkung (◐ bis G11) | Risiko | Speicher |
|---|---|---|---|---|
| **K1** | Zweig „keine Anfrage“ (`http.cpp:16681-16710`): `abort()` statt `stop()` | Vorab-Verbindung ohne FIN_WAIT und TIME_WAIT | RST an eine **unbenutzte** Verbindung (◐); AKE.3 sucht `ERR_CONNECTION_RESET` | 0 |
| **K2** | einmal je Sekunde: PCBs mit `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING, LAST_ACK und — bei `TF_CLOSEPEND` — ESTABLISHED, CLOSE_WAIT, mit `ruhe_s` > Frist ⇒ `tcp_abort()`; nach jedem Abbau Gang von vorn; Zähler `tcp_reaped` | gebundener Speicher nach der Frist frei; FIN_WAIT_1/LAST_ACK enden nicht erst nach `TCP_MAXRTX` | schreibender Zugriff auf lwIP (Ent-10); nie `arg` = 1 oder 2; RST an eine Gegenstelle, die nicht mehr antwortet. `ruhe_s` steht nach `close()` still — die Frist zählt ab spätestens `close()` | ~4 Byte |
| **K3** | FIN_WAIT_2-Frist verkürzen | — | in die Bibliothek übersetzt — **geht in K2 auf** | — |
| **K4** | Höchstzahl je Gegenstelle | — | hoch — **nicht empfohlen** | 0 |
| **K5** | `HTTP_TW_LIMIT` 1 (`http.cpp:464`) | ~0,2 KB (●) | wie E4 §19.7, schärfer | 0 |

**Frist in K2 (Ent-14):** 5 s empfohlen; endgültig mit den Ruhezeiten.

---

## 7. Kandidaten C71 (R3 nach Ent-16 A, Ent-17)

Abgeglichen mit A.2 und der Trennmessung L468. Weil C71 nur entsteht, wenn **beim Start** geschrieben wird, müssen die
Kandidaten an den Startzustand: entweder zu einem Zeitpunkt schreiben, an dem die späten Belegungen des Starts
(WLAN-Aufbau, erste Verbindungen, NTP/UDP, verzögert angelegte Objekte, der erste `ClientContext` des statischen
httpclient) schon liegen (**L6**) — oder **bevor** sie entstehen (**L2**).

| | Mechanismus | Wirkung | Risiko | Speicher | Stand |
|---|---|---|---|---|---|
| **L6** — Abgleich nach dem Start verzögern (neu) | `UPDATE_CACHE_START_MS` (`http.cpp:13398`) von 60 s auf 300 s; die Bedingung „5 s nach der letzten Anfrage“ bleibt | der Abgleich trifft einen Heap, dessen Startbelegungen gesetzt sind — der Zustand, in dem L468 (c) **mit** Abruf und Schreiben keine Spur zeigte (✔ L) | nach einem Start bis zu 5 min die alten Release Notes und die alte Liste (die Dateien bleiben gültig; heute 60 s); fehlt eine Datei (erster Start nach Formatieren), zeigt `update_status` sie bis zu 5 min leer (heute 60 s, E3.13); ob 300 s genügen, ist **◐** — G11 belegt es mit dem erzwungenen Schreiben beim Start (AKS.2), `- update cache vorher` zeigt `up`, `hc` und `tcp=` | 0 | **Empfehlung für N1**: eine Konstante, kein Eingriff in den Ablauf, c65n nachziehen |
| **L2** — Abgleich vor dem ersten Netzverkehr | erster Durchgang, bevor `http_server_begin()` Anfragen annimmt | der Abgleich liegt vor den späten Belegungen; was er hinterlässt (`ClientContext` des httpclient), liegt unten | die 60-s-Bedingung hat einen Grund (A79, L421 — P11); der Start wartet auf WLAN und Update-Server, bei schlafendem Server bis zu 5 s je Abruf (`http.cpp:13383-13384`); breiterer Eingriff in die Startreihenfolge | 0 | Alternative zu L6, falls G11 mit L6 die 5'120 nicht erreicht oder P11 eine frühe Ausführung erlaubt |
| **L0** — PUYA-Puffer früh anlegen (Analyst: Ab-1) | nur bei `vendor=85`: früh in `setup()`, vor WLAN, Server und `EEPROM.begin()`, ein Flash-Schreiben, das nichts ändert (`0xFFFFFFFF` auf ein ausgerichtetes Wort, P12) | 256-Byte-Puffer liegt unten | Wahl des Worts (P12); ein Schreibzyklus je Start | 0 | **bedingt** — nach L468 (b) als C71-Hebel praktisch draussen; nur falls `- flash id` 0x85 meldet und N0 sich zeigt |
| **L4b** — nie zwei `File` gleichzeitig | in `update_cache_write()` die alte Datei erst nach dem Schliessen der Temporärdatei öffnen (`:13677`, `:13699`) | weniger gleichzeitige Belegungen beim Schreiben | E4.5a-Vergleich umzustellen | 0 | nur für N3 — widerspräche L468 (c) |
| **L1** — LittleFS einmal mounten | — | — | — | ◐ FA 0,2 KB dauerhaft | **verworfen** (A.2) |
| **L3** — Puffer statisch | — | — | sprengt die RAM-Grenze | ◐ FA 3 KB | **verworfen** (A.2) |
| **L4a** — Reihenfolge umstellen | — | — | verschiebt nur den Ort | 0 | **verworfen** (A.2) |
| **L5** — ausgehende Verbindung mit RST | — | TIME_WAIT endet ohnehin nach 120 s; C71 bleibt | — | 0 | **verworfen**; ebenso **Ab-2** (`ClientContext`-Hygiene, `client = WiFiClient()`) — kein C71-Hebel |

**L6 gegen L2:** L6 ändert eine Zahl und nutzt einen am Gerät belegten Zustand (L468 (c)); L2 ändert die
Startreihenfolge und hängt an einer Bedingung, deren Grund offen ist (P11). **Empfehlung L6**, L2 als zweiter Schritt.

---

## 8. Prüfstände

Ablage `tools/checks/auszug/esp/c72/`, `…/c71/`, `…/c70/`, PWA neben b48; in S16. Fallzahl gegen das, was es geben
müsste (L235); einmal gegen den Vorgängerstand fehlgeschlagen; je Massnahme eine Sabotage (DIR-014).

| Prüfstand | Release | Gegenstand | Fälle (Auszug) | Sabotage ⇒ FEHL |
|---|---|---|---|---|
| **c72i** | R1 | I-5 | Attrappen `tcp_active_pcbs` (alle 11 Zustände; `callback_arg` NULL, `ClientContext`, Listener in SYN_RCVD; ESTABLISHED mit NULL), `tcp_tw_pcbs`, `tcp_ticks`, `_unclaimed` mit lebenden und toten Kontexten; `arg` 0/1/2; Liste ≤ 8 + `tcp_now_more`; `http_accept_dead` nur bei nicht leerem `_unclaimed`; statisch kein `http_send` im Listengang; Heap-Zeile ≤ 119, Mitschnitt = Ring | Ausgabe in der Schleife; Kappung entfernt; SYN_RCVD als `ClientContext`; toter Abholversuch ohne Zähler; leerer Durchlauf gezählt; Heap-Zeile nur `Serial` |
| **c71i** | R1 | I-6 | `vorher`-Zeile genau einmal je Durchgang vor Schritt 1 mit `up`, `free/blk`, `tcp=`, `hc`; Verlauf mit 5, 3, 0 Schritten; `free`/`blk` je Schritt; `-`; Werte nach `LittleFS.end()`; `erste anfrage` einmal; `- flash id` einmal vor `EEPROM.begin()`; ≤ 119 | Schritt ohne Wert; nur `blk`; `vorher` fehlt oder nach Schritt 1; nur `Serial`; `flash id` nach `EEPROM.begin()` |
| **c70e** | R2 | C70 (a) | `HTTP/1.1 200` + `ETag`; Treffer ⇒ `HTTP/1.1 304` ohne Körper und `Content-Length`; Fehltreffer; `W/`, Liste, `*`, überlang, Kopfname klein; leere Datei nie `304`; Index ohne `app_complete` ⇒ Installationsseite (`HTTP/1.0`); Asset ausserhalb der Weissliste ⇒ `HTTP/1.0` ohne `ETag`; Körperlänge = `Content-Length`; `seek` scheitert ⇒ nichts ausser vom Anfang. **T:** zwei gültige `.gz` gleicher Grösse, anderer Inhalt ⇒ andere `ETag`; abgeschnitten ⇒ anderer; ohne `1f 8b` ⇒ kein `ETag`. **V:** CRC-32/MPEG-2; Invalidierung an jeder der 31 Stellen. **Statisch:** `HTTP/1.1` an genau zwei Stellen, beide mit `Connection: close` | `ETag` nur aus Grösse; `304` ohne Grössenprüfung; `304` mit Körper; `304` ohne `Connection: close`; `HTTP/1.1` in `/api/`; V: Invalidierung entfernt, CRC mit zlib |
| **c70p** | R2 | PWA | `"no-cache"` in `loadLayoutPreviewRowsMap`; Sprachdatei mit `?v=` | `"no-store"` |
| **c65b, c65w, LittleFS-Attrappe** | R2 | Bestand | Kopfzeilen in einem Schreibvorgang mit `HTTP/1.1` und `ETag`; `seek`, Trailer- bzw. zweites Lesen; keine Fälle verloren | wie bisher |
| **c72e** | R3 | Eingriff C72 | K1 `abort()`; K2 nur `callback_arg == NULL` + Zustand aus der Menge + `ruhe_s` > Frist; SYN_RCVD, `ClientContext`, TIME_WAIT, LISTEN unberührt; ein Gang je Sekunde; Neubeginn nach Abbau; K5 Grenze 1 | K1 `stop()`; K2 PCB mit `callback_arg` ≠ NULL bzw. SYN_RCVD abgebaut, Frist entfernt; K5 Grenze 2 |
| **c71e** | R3 | Massnahme C71 | **L6:** erster Durchgang nicht vor 300 s nach dem Start, 5-s-Bedingung bleibt, Wiederholung nach Fehlschlag unverändert; c65n nachgezogen; **L2:** Abgleich vor `http_server_begin()`; **L0:** nur `vendor=85`, vor `EEPROM.begin()`, `0xFFFFFFFF`, Inhalt gleich; **L4b:** nie zwei `File` offen | L6: Startfrist 60 s; sonst Umkehrung der jeweiligen Regel |

---

## 9. Werkzeuge (Lead)

- **`ladeprobe`** — liegt vor; Leitungsstatus; Gegenproben AKZ.2 (i).
- **`browser-sitzung`** — `osascript` und `manuell`; je eigene Lesung lokaler Port und TIME_WAIT-Ende.
- **P1-Probe** — Python-Skript nach §5.6.
- **`check-pwa`** (R2) — INFO `304` je Laden aus `responseReceivedExtraInfo.statusCode`, Byte je Laden.
- **`smoke-device.sh`** — ab R1 I-5-Felder, ab R2 `app_*`.
- **`watch-log.sh`** — ab R1 Heap-Zeile mit Zusatz, `- update cache vorher`, `- update cache heap`, `- flash id` als
  Hinweis; mit V `- app etag`.
- **R2:** `tools/**` nach Prüfungen auf `HTTP/1.0` bei App-Assets durchsuchen; c65b, c65w, LittleFS-Attrappe nachziehen.
- **Nur V:** jede nachgerechnete CRC als CRC-32/MPEG-2.

---

## 10. RAM-Budget (AKZ.8)

| Teil | Release | statisch (Byte) |
|---|---|---|
| I-5 samt `http_accept_dead` | R1 | ~2 |
| Heap-Zeile | R1 | 0 |
| I-6 | R1 | ~30 |
| `If-None-Match`-Puffer | R2 | 36 |
| Zähler `app_*` | R2 | 4 (T) / 6 (V) |
| `ETag`-Tabelle | R2 | 0 (T) / 104 (V) |
| K2 | R3 | ~4 |
| C71 L6/L2/L0/L4b | R3 | 0 |
| **Summe** | | **rund 80 (T) / 185 (V)** — Grenze 250 (Ent-11) |

---

## 11. Releaseschnitt, Einspielen, Rückweg

**R1** — ESP. Rückweg: ESP 3.2.37 aus `wordclock-release-2026-10-14-180012.zip` (L457).

**R2** — ESP und PWA, Firmware zuerst (DIR-002). Alle vier Paarungen R1/R2 × 1.4.100/R2-PWA sind sicher. Ablauf: Wache;
OTA; Smoketest samt Update-Quelle; `install-app.sh --check`; `install-app.sh`; `--check`; `check-pwa.sh`. Der Upload
ist ein Flash-Schreiben — im Messprotokoll vermerkt; nach L468 im laufenden Betrieb ohne Spur. Rückweg: ESP auf R1,
PWA bleibt.

**R3** — ESP. Rückweg auf R2; mit Ent-16 A nimmt er C71 **und** C72 zurück.

Du spielst jedes Release ein; der Lead liefert die Zeilen samt Rückweg, **bevor** eingespielt wird.

---

## 12. Prüfpunkte (`firmware-analyst`)

| P | Frage | Stand |
|---|---|---|
| **P1** | FIN_WAIT_2 läuft nach `tcp_close()` ab? RST? | **◐ FA:** ja, rund 20 s nach dem letzten Segment, ohne RST — am Gerät mit der **P1-Probe** |
| **P2** | `MEMP_NUM_TCP_PCB` bei `MEMP_MEM_MALLOC`? | **✔ FA: begrenzt nichts**; Kommentar `http.cpp:325` in R1.1 berichtigen |
| **P3** | `tcp_abort()` auf PCB ohne `callback_arg`; Gang danach? | **◐ FA:** SYN_RCVD trägt den Listener — K2 nur `NULL`; Gang neu |
| **P4** | Wann wird `tmr` gesetzt? | **◐ FA:** §3.1 |
| **P5** | `crc32()` im Core | **✔ FA:** CRC-32/MPEG-2; dazu Variante T |
| **P6** | Schreibende LittleFS-Stellen | **✔ FA:** 31, vollständig; AutoFormat; `:7866` wirkungslos; **✔ SW:** `ESP-uclock.ino:291` in `#if 0` |
| **P7** | `stop()`/`abort()` im Zweig „keine Anfrage“ | **offen** — für K1 bis zum Tor |
| **P8** | Tote `ClientContext` in `_unclaimed` | **◐ FA:** stapeln sich, früher Rücksprung ohne Zähler ⇒ I-5 ergänzt |
| **P9** | C71: was überlebt den Startabgleich? | **umgestellt nach L468:** nicht das Schreiben an sich, sondern **was beim Start lebt**, wenn geschrieben wird. Offen: welche späten Startbelegungen in den ersten rund 60 s entstehen (WLAN, NTP/UDP, mDNS falls vorhanden, verzögert angelegte Objekte, erster `ClientContext` des httpclient) und welche davon zwischen den Temporärbelegungen des Schreibens liegen bleiben. Das Bild liefert I-6 (a) in G9.6 |
| **P10** | LittleFS gemountet; Puffergrösse | **◐ FA:** 0,2 KB bzw. 3 KB ⇒ L1, L3 verworfen; Rest nicht mehr nötig |
| **P11** | Warum 60 s nach Start (A79, L421)? Abgleich vor `http_server_begin()`? Ist 300 s nach Start unbedenklich? | **offen** — für L2 und L6 |
| **P12** | ESP-Flash-Schreiber ausser LittleFS (`EEPROM.commit()`), Logzeile? Wort für L0? | **offen** — für den Vermerk „Flash-Schreiben seit Start“; L0 nur noch bedingt |

---

## 13. Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Legacy parallel? Wetter-Endpunkte? Restore-Bedingung? Richtige Schicht?

> **Legacy** bleibt parallel, unverändert und bei `HTTP/1.0`; C70 setzt nur in `http_send_fs_file()` an (AKC.5,
> AKC.7). **Wetter:** unberührt; K2 lässt laufende ausgehende Abrufe in Ruhe. **Restore-Bedingung:** STM unberührt.
> **`.gz` und Grösse > 0:** bleibt; `304` setzt die Grössenprüfung voraus; mit T bekommt eine Datei ohne gzip-Kopf
> keinen `ETag`. **Schicht:** C70 in der Auslieferung des ESP; C71 am Zeitpunkt des Startabgleichs, wo die Messung die
> Ursache zeigt (L6: eine Konstante); C72 erst gemessen, dann im ESP.

**Scalable systems** — STM-Kommandos? UART? Watchdog? Hauptloop?

> **STM-Kommandos:** keine. **UART:** Heap-Zeile + rund 38 Byte je Minute; `- update cache vorher` und
> `- update cache heap` je Durchgang, `- heap erste anfrage` und `- flash id` je Start; mit V `- app etag`.
> **Hauptloop:** T kostet je Asset-Antwort ein `seek` und 10 Byte (◐ ms), V einmalig 0,15–0,3 s je Asset; jede `304`
> Millisekunden statt rund einer Sekunde; K2 einmal je Sekunde; L6 verschiebt nur den Zeitpunkt des Abgleichs.
> **Watchdog:** keine Schleife ohne Grenze. **Feste Grenzen:** Liste 8, `If-None-Match` 36 Byte, Startfrist 300 s mit L6.

**Secure by design** — `innerHTML`? Fremddaten? Credentials?

> `If-None-Match` ist Eingabe aus dem Netz — begrenzt, nur verglichen, nie ausgegeben. Logzeilen nennen nur Zahlen,
> Dateinamen und die Flash-Kennung. Kein neuer Endpunkt, kein neuer Parameter. K1/K2 senden RST nur an Verbindungen
> ohne Anfrage bzw. vom ESP geschlossene. L6 verlängert nur das Fenster, in dem die alte, gültige Release-Note-Datei
> ausgeliefert wird — sie kommt nie vom Server des Ursprungsprojekts (unverändert, `http.cpp:13340-13342`).

**Stable & reliable** — Fehler ausgewertet? Stille Verwerfungen? Zustand nach Abbruch? Flags?

> **Veralteter Cache:** mit T durch Bau ausgeschlossen; mit V durch Inhalt, Grössenvergleich und Invalidierung an jeder
> Stelle. **`HTTP/1.1`:** `Connection: close`, exaktes `Content-Length`, Abbruch mit RST. **Abbruch:** scheitert `seek`
> oder Trailer-Lesen, geht die Antwort ohne `ETag` bzw. nichts ausser vom Dateianfang. **C71:** L6 lässt den Ablauf und
> alle Sicherungen des Zwischenspeichers unverändert (AKS.3). **Stille Verwerfungen:** `304`, `accept_dead`,
> K2-Abbauten gezählt. **Messung ohne Selbsttäuschung:** eigene Lesungen und ihr TIME_WAIT-Fenster markiert und aus M2
> herausgenommen; Entscheidungsregeln vor der Messung fest; „Flash-Schreiben seit Start“ zu jedem Ruhewert; Neustarts
> sind Ergebnisse (Ent-7); eine am Gerät widerlegte Hypothese (K-a, L468) wird nicht weiter verfolgt, sondern nur noch
> bedingt geführt.

---

## 14. Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| `ETag` bei `HTTP/1.0` | greift in Chrome nicht |
| `HTTP/1.1` für alle Antworten | kein Gewinn, Keep-Alive-Risiko |
| `Last-Modified` | Zeitstempel unsicher; Rückfallebene nur für Safari (G10.7) |
| `ETag` aus Grösse allein / App-Version | Kollision bzw. dem ESP unbekannt |
| CRC über die ganze Datei bei jeder Anfrage | 0,15–0,3 s je `app.js`-Anfrage; T braucht 8 Byte |
| `ETag` in eigenen Dateien | zusätzliche Flash-Schreibvorgänge |
| `max-age` ohne Versionierung; Variante (b) jetzt | „sofort wirksam“ verletzt bzw. Fenster mit veraltetem Cache |
| `device_ready` im Sekundentakt; HTTP-Lesungen in der Startmessung | verändern das Gemessene |
| `tcpdump` als Hauptinstrument | `sudo`; Zusatz für M4 |
| K3, K4 | §6 |
| L1, L3, L4a, L5, Ab-2 | §7 |
| L0 als Hauptkandidat | L468 (b): kein ΔFree ≈ −264 beim ersten Schreiben; nur noch bedingt |
| C72-Eingriff oder C71-Massnahme ohne Messung | Mechanismus nicht belegt |
| C71 in R2 (Ent-16 C) | G10 soll den Cache getrennt zeigen |

---

## 15. Versionsfolgen

- [ ] ESP `version.h` — **R1**, **R2**, **R3** (falls)
- [ ] `APP_VERSION` und `CACHE_NAME` — **R2**
- [ ] STM — **nicht**

Nur der `release-engineer` (R4). Die Nummern bestimmt der Lead.

---

## 16. Betroffene Module

| Datei | Änderung | Agent |
|---|---|---|
| `ESP8266/ESP-uclock/http.cpp` (UTF-8, eine CRLF-Zeile) | R1: I-5 samt `_unclaimed`, `http_accept_dead`, `arg`, Kommentar `:325`; Zählfunktion; I-6 (a)–(c). R2: `If-None-Match`, `ETag` (T bzw. V), `HTTP/1.1` in `http_send_fs_file()` und `304`, Zähler. R3: K1/K2/K5; C71 L6 (`:13398`), L2 oder L4b | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` (gemischte Zeilenenden) | R1: Heap-Zeile, I-6 (d); R2 nur mit V: Invalidierung `:291` (in `#if 0`); R3: K2-Aufruf, L2 bzw. L0 | `esp-developer` |
| `ESP8266/ESP-uclock/stm32flash.cpp` (UTF-8) | R2 nur mit V: Invalidierung an drei Stellen | `esp-developer` |
| `ESP8266/ESP-uclock/base.h` | Prototypen | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | R2: eine Zeile | `pwa-developer` |
| `tools/**`, Prüfstände | §8, §9 | Lead |
| Versionsdateien | R1, R2, R3 | `release-engineer` |
| `TESTPLAN-PWA.md`, `BEFUNDE.md` | Felder, Zeilen, Lesart; Träger an C65, C70, C71, C72; Nebenbefunde `:7866`, `ESP-uclock.ino:291`, Ab-2 | `doc-writer` |
| `CHANGELOG.md` | je Release | Lead |

---

## 17. Nachtrag A.2 und Trennmessung L468 (15.10.2026) — was sich ändert, was widerlegt ist

**Widerlegt oder berichtigt (A.2):**

| Stelle (Stand vor A.2) | Neu |
|---|---|
| §3.1 `ruhe_s` „für TIME_WAIT die Zeit seit dem Eintritt“ | **falsch** — `tmr` steht nach `TF_RXCLOSED` und beim Zustandswechsel still |
| §3.1 `eigen` = 1 ⇔ `ClientContext` | ungenau — SYN_RCVD trägt den Listener; jetzt `arg` 0/1/2; ESTABLISHED mit `arg` = 0 möglich |
| §5.4 M1 über FIN_WAIT_2 ≥ 30 s | unter P1 unmöglich — M1 über FIN_WAIT_1/CLOSING/LAST_ACK mit `nrtx` > 0 und `TF_CLOSEPEND`; FIN_WAIT_2 ≥ 30 s belegt **gegen** P1 |
| §5.4 M2 über TIME_WAIT in Heap-Zeilen | die eigene Lesung erzeugt dieses TIME_WAIT — eigene Fenster zählen nicht; `http_accept_dead` ergänzt `nrt` |
| §5.4 M4 | SYN_RCVD ausgeschlossen; `tcp_now_unclaimed` |
| `http.cpp:325` „Listen sind kurz“ (Code, E4) | falsch (P2) — Berichtigung in R1.1 |
| Ent-2 „CRC-32 + Grösse“, zlib-artig gedacht | Core-CRC ist CRC-32/MPEG-2; Ent-2 offen, Empfehlung T |
| §4.4 Bestand „vollständig erst nach P6“ | vollständig (31); eine Stelle in `#if 0` |
| §7 L1, L3, L4, L5 | L1, L3, L4a, L5 verworfen; L4b bleibt (bedingt) |

**Widerlegt durch die Trennmessung L468 (✔ L):**

| Annahme | Befund |
|---|---|
| K-a / N0: der PUYA-Puffer des ersten Flash-Schreibens teilt den Block (A.2, ◐ FA) | **nicht bestätigt** — erstes Schreiben seit dem Start im laufenden Betrieb ohne ΔFree ≈ −264, `max` unverändert. L0 bleibt nur bedingt (`vendor=85`) |
| C71 hängt am Schreiben an sich (Spec vor L468, N1/N3) | **widerlegt** — Schreiben mit und ohne Abruf im laufenden Betrieb hinterlässt nichts; Start mit `unveraendert` ebenso (V3). C71 entsteht nur beim **Schreiben beim Start** |

**Neu:** P1-Probe (§5.6); `tcp_now_unclaimed`, `http_accept_dead`; Betriebsart `manuell`; I-6 (a) `- update cache
vorher` mit `up`, `tcp=`, `hc`; I-6 (d) `- flash id`; N-Regel umgestellt auf den Startzustand; **L6** (Abgleich nach
300 s) als Empfehlung, L2 als Alternative; Vermerk „Flash-Schreiben seit Start“; AKC.8 (Attrappen); P12; AKZ.9
(Trennmessung, erfüllt). **Bestätigt:** `HTTP/1.1` nur in `http_send_fs_file()`, kein Keep-Alive (✔ FA).

**Ausstehend:** P7, P9 (Startbelegungen), P11, P12.
