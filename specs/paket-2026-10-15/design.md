# Design — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge am selben Tag mit den Entscheiden des
Nutzers, den Lead-Entscheiden und dem Prüfserver-Befund des Leads zu `HTTP/1.0`. **Status: freigegeben am 15.10.2026
bis aufs Tor** — offen Ent-8, Ent-10, Ent-14, Ent-17 (Tor) und Ent-16 (vor R3); neu und vom Nutzer noch nicht gesehen:
§3.3 (I-6), §5.5, §7 (C71) und in §4.4 die Statuszeile `HTTP/1.1`. Momentaufnahme (DIR-006); der lebende Stand steht
in `BEFUNDE.md`. Anforderungen und Kriterien in `requirements.md`, Tasks in `tasks.md`.

**Kennzeichen** wie in `requirements.md`: **✔ SW** am Code `72cfda7` bzw. an der Core-Quelle 3.1.2
(`~/Library/Arduino15/packages/esp8266/hardware/esp8266/3.1.2/`) nachgelesen; **✔ B** in `BEFUNDE.md` belegt; **✔ L**
vom Lead gemessen und gemeldet; **◐** abgeleitet oder Annahme; **●** gemeldet, vor der Umsetzung zu prüfen.
Detailfragen, die Fachwissen am Core brauchen, stehen als **Prüfpunkte P1–P11** (§12) beim `firmware-analyst`
(Task A.1) — die Spec erfindet dort keine Antwort.

---

## 1. Befund mit Verifikationsstatus

| Aussage | Status | Fundstelle / Bemerkung |
|---|---|---|
| Volles Neuladen mit einem Client reisst K30: `low=752`, `max` 3'584 | **✔ B** | L466 (T2, 21:28:37); I-4 dazu verloren (V3 setzte zurück) |
| Jedes Öffnen holt alle Assets, rund 200 KB, davon `app-app.js.gz` 163'621 Byte | **✔ B** / **✔ SW** | C70, Messprotokoll E3 (E3.0); Kopf ohne Validator `http.cpp:2067-2078` |
| Die Auslieferung antwortet `HTTP/1.0 200 OK`, `Content-Encoding: gzip`, `no-cache`, `Connection: close` | **✔ SW** / **✔ L** | `http.cpp:2067-2077`; `curl -D` des Leads 15.10.2026 |
| Chrome nimmt einen `ETag` aus einer `HTTP/1.0`-Antwort nicht als Validator; mit `HTTP/1.1` schon | **✔ L** (Prüfserver) / **◐** (Ursache) | Lead 15.10.2026 22:21–22:23: bei 1.0 kein `If-None-Match` für das Dokument, für `app.js` nur beim Neuladen; bei 1.1 überall `304`, 290 statt 4'045 Byte. Chromium-Quelltext nicht nachgelesen; Safari ungeprüft |
| CDP meldet bei einer `304`-Nachprüfung in `responseReceived` den Cache-Status `200` | **✔ L** | Leitungsstatus nur in `responseReceivedExtraInfo.statusCode`; `tools/ladeprobe.mjs` nutzt das |
| Der Server wertet `If-None-Match` nicht aus | **✔ SW** | Kopfzeilenschleife `http.cpp:16783-16802` liest nur `User-Agent`, `Sec-Fetch-Dest`; jede Zeile über `read_line()` (`:6892`) |
| Die Query eines Asset-Pfads wird abgetrennt, bevor `http_app()` ihn sieht | **✔ SW** | `http.cpp:16812-16823`, `:15968-15972`, `:2783-2794` |
| Der ESP kennt die installierte App-Version nicht | **✔ SW** | nur die **verfügbare** (`APP_VERSION_TXT`, `http.cpp:111`, `:14294`) |
| Über `http://` kein Service Worker | **✔ SW** | `app.js:2135-2163`; `sw.js:77-137` wirkt nur unter https |
| `layout-previews.json` mit `no-store`; Sprachdatei mit `?v=` und `default` | **✔ SW** | `app.js:11655`; `app.js:1744-1745` |
| `install-app.sh` und die Auto-Installation laden `index.html` **zuerst** | **✔ SW** | `tools/install-app.sh:36-38`; `APP_INSTALL_ASSETS`, `http.cpp:747-786` |
| `check-pwa` lädt in einem frischen Profil, Neuladen ohne `ignoreCache` | **✔ SW** | `tools/check-pwa.sh:74`, `:119`; `tools/check-pwa.mjs:375` |
| Nach dem Start fällt `max` von 7'184 auf 4'672 im Fenster des Startabgleichs und bleibt dort | **✔ B** | L460, L457 (G8); unter E3 7'256 ⇒ 5'808 (L450) |
| Der Startabgleich öffnet den Abruf **vor** `LittleFS.begin()` und schliesst beides erst am Ende; bis zu zwei `File`-Objekte gleichzeitig | **✔ SW** | `update_cache_write()` `http.cpp:13595`, `:13614`, `:13625`, `:13677`, `:13699`, `:13770-13771` |
| Erster Durchgang nach **jedem** Start gleicht beide Dateien ab; frühestens 60 s nach Start, 5 s nach letzter Anfrage | **✔ SW** | `http.cpp:13338-13367` |
| Welche Belegung das Fenster überlebt | **nicht belegt** | P9; Instrument I-6 (§3.3) |
| Nach einer Safari-Sitzung `free` −1,7 KB, `max` 3'816, `http_backlog` 1, über Minuten | **✔ B** | L461 |
| Frei erst beim **Beenden** von Safari | **●** | Beobachtung des Nutzers (L461); L461 nennt daneben eine Erholung 20:09:37 „ohne Browser“ |
| Die PWA pausiert ihre Abfragen bei verborgener Seite | **✔ SW** | `app.js:2079-2101` |
| Verbindung ohne Anfrage: höchstens 250 ms warten, zählen, `stop()` | **✔ SW** | `http.cpp:239`, `:16660-16710` |
| `stop()` endet in `tcp_close()`, also FIN | **✔ SW** | `ClientContext.h:73-92` |
| FIN ohne Daten an einer noch nicht abgeholten Verbindung ⇒ Core baut sofort ab | **✔ SW** | `ClientContext.h:594-615` |
| FIN_WAIT_2 läuft nach 20 s ab, TIME_WAIT 120 s | **✔ SW** (Kopf) / **◐** (Bibliothek) | `lwip/priv/tcp_priv.h:131`, `:137` (P1) |
| I-4 zählt Zustände nur beim neuen Block-Tiefstwert | **✔ SW** | `http.cpp:473-530` |
| `MEMP_NUM_TCP_PCB` 5 begrenzt bei `MEMP_MEM_MALLOC` 1 womöglich nicht | **◐** | `lwipopts.h:279`, `:444`; Kommentar `http.cpp:325` (P2) |
| `tcp_ticks`, `tcp_tw_pcbs`, `tcp_active_pcbs` im privaten Kopf; `tcp_pcb` mit `remote_port`, `flags`, `tmr`, `nrtx`, `callback_arg` | **✔ SW** | `tcp_priv.h:327`, `:337`, `:340`; `lwip/tcp.h:212-219`, `:249-305` |
| `crc32()` im Core vorhanden | **✔ SW** | `cores/esp8266/coredecls.h:34`; Kosten ◐ (P5) |

---

## 2. Zuschnitt, Releases, Reihenfolge (Ent-9: drei Releases)

| Release | Inhalt | Laufzeit | danach | Schreiber |
|---|---|---|---|---|
| **R1** | **I-5** (§3.1, §3.2) und **I-6** (§3.3) — nur Instrumente | ESP | **G9** ohne Cache | `esp-developer` |
| **R2** | **C70** (§4): `ETag`/`304`, `HTTP/1.1` für App-Assets; PWA-Teil (`layout-previews` `no-cache`) | ESP **und** PWA | **G10** mit Cache, **Tor** | `esp-developer`, `pwa-developer` |
| **R3** | **Eingriff C72** (§6, Ent-8) und **Massnahme C71** (§7, Ent-17); Platz von C71 nach Ent-16 (Empfehlung A: hier) | ESP | **G11** | `esp-developer` |

**Reihenfolge wie entschieden:** Instrumente zuerst (R1), dann der Ausgangsstand **ohne** Cache (G9), dann der Cache
(R2) und dieselben Messungen **mit** Cache (G10) — so ist die Wirkung des Caches **getrennt** sichtbar —, dann das Tor
auf M1–M5 und N1–N4, dann der Eingriff (R3). Jedes Release einzeln mit Commit, Tag und Einspielen.

**Mit Ent-16 B** wandert C71 aus R3 in ein eigenes **R4** nach G11; die C71-Tasks in `tasks.md` tragen den Teil in der
Spalte „Task“, der Schnitt ist mechanisch. R3 findet statt, sobald C72 **oder** C71 einen Eingriff hat; ohne beide
entfällt es samt G11.

**Abhängigkeiten nach R3b:** Alle ESP-Teile schreibt **ein** `esp-developer` nacheinander (`http.cpp`, `base.h`,
`ESP-uclock.ino`, `stm32flash.cpp`). Der PWA-Teil in R2 berührt disjunkte Dateien ohne Gerätenachweis dazwischen und
darf parallel zum ESP-Teil laufen; der Release-Build wartet auf beide. Code für R2 darf entstehen, während G9 läuft;
gebaut und eingespielt wird erst danach. Code für R3 entsteht erst nach dem Tor.

---

## 3. Instrumente (R1)

### 3.1 I-5 — laufende Verbindungszustände in `device_ready` (C72)

**Hinter `api_orphan_skipped`** (`http.cpp:14770`, bisher letztes Feld):

| Feld | Inhalt |
|---|---|
| `tcp_now_states` | 11 Zahlen: PCBs je Zustand in `tcp_active_pcbs` in diesem Augenblick, Index = `enum tcp_state` (0 CLOSED … 10 TIME_WAIT), sättigend 255; die eigene Verbindung (ESTABLISHED) mitgezählt |
| `tcp_now_tw` | Länge von `tcp_tw_pcbs` |
| `tcp_now_pending` | `http_server.pending()` |
| `tcp_now_self_port` | `http_client.remotePort()` dieser Anfrage |
| `tcp_now_list` | höchstens 8 Felder `[zustand, gegenstelle, port, ruhe_s, nrtx, eigen]`, erst `tcp_active_pcbs`, dann `tcp_tw_pcbs` |
| `tcp_now_more` | PCBs, die nicht in die Liste passten |

- `gegenstelle`: letztes Oktett der IPv4-Adresse (Ent-5), 0 bei nicht-IPv4.
- `ruhe_s`: `(tcp_ticks − pcb->tmr) × TCP_SLOW_INTERVAL / 1000` — upstream gesetzt bei jedem empfangenen Segment und
  beim Zustandswechsel (**◐**, P4).
- `nrtx`: Wiederholungen des laufenden Segments — ein unquittiertes FIN wird sichtbar.
- `eigen`: 1, wenn `callback_arg` ≠ NULL (ein `ClientContext` hängt daran: wartend, bedient, ausgehend); 0 nach
  unserem `close()` (`ClientContext.h:78`). **Diese Unterscheidung trägt die Entscheidungsregel (§5.4) und begrenzt K2.**

**Ohne Abgabe:** Listen in einen Stackpuffer (8 × 8 Byte + 11 Zähler, rund 80 Byte), **bevor** das erste
`http_send()` läuft — `http_send()` kann über `http_flush()` abgeben, und dann liefe lwIP mitten im Listengang. Kein
`String`, kein Heap; Begründung der Gleichzeitigkeit wie I-4 (`http.cpp:327-330`). **Kappung auf 8**, weil die Zahl
aktiver PCBs womöglich nicht begrenzt ist (P2). Nichts wird zurückgesetzt oder gespeichert.

### 3.2 I-5 — Heap-Zeile (Ent-4)

```
- heap free=<n> max=<n> ovr=<n> low=<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>
```

- Zählfunktion in `http.cpp` (privater lwIP-Kopf nur dort, Ent-32 aus Paket 2026-10-13), Prototyp in `base.h`;
  CLOSING fehlt in der Zeile (selten), steht in `tcp_now_states`.
- Längste Form 67 + 28 + 10 = **105** ≤ 119 Zeichen (Kommentar `ESP-uclock.ino:624`); Puffer `line[80]` ⇒ 120.
- **Warum:** jede `device_ready`-Lesung ist selbst eine Verbindung; die Heap-Zeile zählt ohne eigene Verbindung, jede
  Minute. Sie entscheidet „Strom kurzer Verbindungen oder wenige lange?“ (M1 gegen M2).
- `watch-log.sh` liest `free=`, `max=`, `ovr=`, `low=` über Muster (`tools/watch-log.sh:368`, `:383`), die einen
  Zusatz hinten vertragen (◐ — Gegenprobe AKZ.2 (iv)). `ESP-uclock.ino` hat **gemischte Zeilenenden**: byte-genau.

### 3.3 I-6 — Heap im Startabgleich (C71)

**(a) Eine Zeile je Durchgang von `update_cache_poll()`**, am Ende oder beim Abbruch:

```
- update cache heap vor=<free>/<blk> s1=<blk> s2=<blk> s3=<blk> s4=<blk> s5=<blk> nach=<free>/<blk>
```

- `vor`: unmittelbar vor Schritt 1; `s1`–`s3`: grösster Block nach jeder Versionszeile; `s4`, `s5`: nach Notes bzw.
  Liste, also **nach** `httpclient_stop()` und `LittleFS.end()` (`http.cpp:13770-13771`); `-` für nicht gelaufen;
  `nach`: nach dem letzten Schritt. Längste Form rund 104 Zeichen.
- Die Werte je Schritt liegen in einem kleinen statischen Feld (5 × `uint16_t` + `vor`), weil ein Durchgang sich über
  mehrere Durchläufe von `loop()` verteilt (`:13343-13345`).
- Die bestehenden Zeilen `- update cache <datei> …` bleiben unverändert — `watch-log.sh` und c65n hängen an ihnen; die
  neue Zeile enthält kein `fehlgeschlagen` und löst keinen Hinweis der Wache auf Fehlschläge aus (`tools/watch-log.sh:354`).

**(b) `- heap erste anfrage free=<n> max=<n>`** einmal je Start, beim ersten angenommenen Anfrage-Eingang in
`http_server_loop()`, vor jeder Verarbeitung. Zeigt, ob der Block **vor** dem Startabgleich schon geteilt ist.

**Messung:** `ESP.getMaxFreeBlockSize()` geht mit gesperrten Interrupts über den Heap (`http.cpp:283-291`) — hier
höchstens sieben Aufrufe je Durchgang, also je Start, und einer je Start für (b). Einmal formatiert, Mitschnitt und
Ring (Muster `http.cpp:1844-1854`). RAM rund 16 Byte.

---

## 4. C70 — Asset-Cache, Variante (a) (Ent-1)

### 4.1 Varianten (zur Erinnerung)

**(a)** `ETag` aus dem Inhalt, `If-None-Match` ⇒ `304`, `no-cache` bleibt — **entschieden**. **(b)** versionierte URLs
mit langem `max-age`. **(c)** beides — wird nur nach G10.7 vorgelegt, falls AKW.2 warm unter 1'072 bleibt.

### 4.2 Was beim zweiten Öffnen noch über die Leitung geht

**N** = `/app/`-Anfragen je Öffnen, **◐ N ≈ 7** (Dokument, `styles.css`, `app.js`, Manifest, ein bis zwei Icons,
`layout-previews.json`; Englisch +1). Kopf `200` rund 190 Byte, `304` rund 110 Byte (◐). Die Anfrage des Browsers
(0,4–0,7 KB, ◐) liegt in jeder Variante je Verbindung als `pbuf` auf dem Heap des ESP.

| Variante | Byte vom ESP beim zweiten Öffnen | Verbindungen |
|---|---|---|
| heute | ≈ 200 KB + N × 190 | N |
| **(a)** mit `HTTP/1.1` und PWA-Teil (Ent-3) | ≈ N × 110 ≈ **0,8 KB**; Prüfserver des Leads: 290 statt 4'045 Byte für Dokument + Asset (✔ L) | N, je sehr kurz |
| (b) | `index.html.gz` + nicht versionierte Referenzen | 2–4 |
| (c) | ≈ 0,3–0,5 KB | 1–4 |

**Heap (◐):** Das Tief beim Laden entstand bisher während der grossen Übertragung — 1'072 Byte unquittiert und
wartende Verbindungen (G8, L457). Mit (a) bleibt die **Zahl** der Verbindungen, aber jede ist nach einer `304` in
Millisekunden vorbei; das Bild gleicht dann dem der `/api/`-Abrufe, unter dem G8 und G7R 1'336–2'184 massen. Ob das
warm über 1'072 bleibt, zeigt AKW.2.

### 4.3 Bewertung (Grundlage von Ent-1)

| | (a) | (b) | (c) |
|---|---|---|---|
| **Korrekt nach Update** | **ja, durch Bau:** `ETag` folgt dem Inhalt | **nein, mit einem Fenster:** `index.html` wird zuerst hochgeladen (`tools/install-app.sh:38`, `http.cpp:749`); wer dazwischen öffnet, bekommt die alte `app.js` unter der neuen URL ein Jahr lang — der ESP ignoriert die Query und kennt die Version nicht (§1) | wie (b) |
| **Versionsstellen** | keine neue | fünfte Stelle (`?v=` in `index.html`) | wie (b) |
| **Dateien** | `http.cpp`, eine Zeile `app.js` | `index.html`, `sw.js`-Liste, `http.cpp`, `Makefile` | alle |
| **Invarianten** | bleiben; `304` nur nach Grössenprüfung | bleiben | bleiben |
| **Legacy** | unberührt (bleibt `HTTP/1.0`) | unberührt | unberührt |

### 4.4 Umsetzung

**`ETag`-Quelle (Ent-2): CRC-32 des gespeicherten `.gz`-Inhalts samt Grösse**, Format
`"<grösse>-<crc32 als 8 Hexziffern>"`, z. B. `"163621-1a2b3c4d"`. Grösse allein kollidiert bei kleinen Textänderungen;
die App-Version kennt der ESP nicht; `Last-Modified` hängt an der Systemzeit beim Schreiben (◐) und löst die
`HTTP/1.0`-Frage nicht (§14).

**Statuszeile `HTTP/1.1` — nur für App-Assets (AKC.7).** Nach dem Prüfserver-Befund des Leads (§1) greift der `ETag`
in Chrome nur mit `HTTP/1.1`. Deshalb tragen **genau zwei** Antworten die neue Statuszeile: die `200` und die `304`
auf ein Weisslisten-Asset als `.gz`. Die Statuszeile hängt am **Asset**, nicht am Gelingen der CRC-Berechnung: Ist
kein gültiger `ETag` da (Lesefehler beim Berechnen), geht `HTTP/1.1 200` **ohne** `ETag` hinaus. **Bewertung des
Risikos:**

| Risiko | Bewertung | Massnahme |
|---|---|---|
| **Keep-Alive-Erwartung:** Unter `HTTP/1.1` gilt eine Verbindung als dauerhaft, wenn nichts anderes gesagt wird; der Browser schickte die nächste Anfrage auf eine Verbindung, die der ESP schliesst — Wiederholung, im schlechten Fall `ERR_EMPTY_RESPONSE` (◐) | **real, wenn `Connection: close` fehlt** | `Connection: close` in **beiden** Antworten Pflicht (heute schon in der `200`, `http.cpp:2077`); c70e prüft statisch, Sabotage „ohne `Connection: close` in der `304`“ |
| **Körperende:** Unter 1.1 mit `Connection: close` darf der Körper auch am Verbindungsende enden — ein abgebrochener Körper sähe dann vollständig aus | gering: die `200` trägt heute schon `Content-Length` (`:2075-2076`), und ein Schreibabbruch endet mit RST (`abort()`, `:16831-16842`) | `Content-Length` = genaue Dateigrösse, Abbruch weiter RST; c70e vergleicht Körperlänge und Kopf |
| **`304` mit Körper oder falschem `Content-Length`** | würde den nächsten Lesevorgang des Browsers verwirren | `304` ohne Körper, ohne `Content-Length` |
| **`Date`-Kopf:** Ein 1.1-Ursprungsserver **mit** verlässlicher Uhr soll `Date` senden; **ohne** verlässliche Uhr darf er es nicht (RFC 9110 §6.6.1, ◐ hier nicht nachgelesen) | gering; für `no-cache` mit Validator ohne Bedeutung | kein `Date` |
| **Chunked, Pipelining, `100-continue`** | nicht berührt: eine GET-Anfrage je Verbindung, der Server liest nur die Kopfzeilen | nichts |
| **Werkzeuge, die die Statuszeile lesen** | ein Werkzeug, das auf `HTTP/1.0 200` prüft, schlüge bei Assets an (◐, nicht durchsucht) | Lead durchsucht `tools/**` auf `HTTP/1.0` (Task R2.6) |
| **Legacy, `/api/*`, Hinweisseiten** | bleiben `HTTP/1.0`; gemischte Versionen am selben Host sind je Antwort unabhängig | AKC.5 prüft die Legacy-Statuszeile; Sabotage „`HTTP/1.1` in einer `/api/`-Antwort“ |
| **Safari** | **ungeprüft** — ob Safari bei 1.1 mit `ETag` nachfragt, zeigt erst AKW.4 über `app_not_modified` | Befund, falls nicht; dann `Last-Modified` am Prüfserver gegenprüfen (G10.7, Nachtrag) |

**RAM-Tabelle je Asset aus `APP_INSTALL_ASSETS`** (13 Einträge, `http.cpp:747-786`): `uint32_t size`, `uint32_t crc`;
`size == 0` heisst „ungültig“. 104 Byte statisch. Nach dem Start leer. **Gültig nur bei gleicher Grösse** — vor jeder
Verwendung mit `fp.size()` verglichen. **Invalidierung** `app_etag_invalidate()` (ganze Tabelle) nach **jeder**
schreibenden LittleFS-Stelle in den ESP-Quellen — eine Regel ohne Ausnahmen ist statisch prüfbar, eine Liste „welche
Stelle schreibt App-Dateien“ veraltet. **Bestand** (Grep am `72cfda7`, Muster `LittleFS.open(…"w"|"w+"|"a")`,
`.remove`, `.rename`, `.format`; vollständig erst nach P6): `http.cpp` 27 (`:1532`, `:2332`, `:6824`, `:6874`, `:7213`,
`:7220`, `:7227`, `:7234`, `:7320`, `:7374`, `:7763`, `:7776`, `:7866`, `:7890`, `:7903`, `:10381`, `:13132`, `:13677`,
`:13759`, `:13766`, `:13843`, `:13848`, `:15151`, `:15192`, `:15201`, `:15585`, `:15617`), `stm32flash.cpp` 3 (`:1282`,
`:1315`, `:1410`), `ESP-uclock.ino` 1 (`:291`) — **31**.

**Berechnung bei Bedarf, in derselben geöffneten Datei** (`http_send_fs_file()`, `http.cpp:2037-2120`): nach
`LittleFS.open()` und vor dem ersten `http_send()`; ist der Eintrag ungültig, wird die Datei einmal ganz gelesen (in
`http_response` als Lesepuffer, zu diesem Zeitpunkt leer; Bedingung wie `:2088-2089`), CRC und Grösse eingetragen,
`fp.seek(0)`, dann wie heute ausgeliefert. Ein Mount, ein Öffnen, kein `String`, kein Heap, `yield()` zwischen Blöcken.
Je Berechnung eine Zeile `- app etag <datei> n=<byte> ms=<dauer>`. **Kosten (◐, P5):** geschätzt 0,2–0,5 s für
`app-app.js.gz`, einmal je Start bzw. je Schreibvorgang. Liest `fp.read()` ≤ 0 während der Berechnung, bleibt der
Eintrag ungültig, und die Antwort geht ohne `ETag` hinaus (siehe oben). Scheitert `fp.seek(0)`, wird **nichts**
ausgeliefert, was nicht vom Dateianfang stammt: Die Datei wird geschlossen und neu geöffnet; gelingt auch das nicht,
endet die Verbindung wie bei einem Lesefehler heute.

**`If-None-Match` lesen:** in der Kopfzeilenschleife (`http.cpp:16783-16802`) wie `Sec-Fetch-Dest`, in einen
statischen Puffer von 36 Byte, je Anfrage geleert (Muster `:849-853`); Kopfname ohne Rücksicht auf
Gross-/Kleinschreibung; `W/` übergangen, Komma-Liste Wert für Wert, `*` kein Treffer; überlange Werte abgeschnitten —
sie treffen dann nicht (sichere Richtung).

**Antworten:**

```
HTTP/1.1 200 OK\r\nContent-Type: <typ>\r\nContent-Encoding: gzip\r\nContent-Length: <n>\r\n
ETag: "<grösse>-<crc>"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n<körper>

HTTP/1.1 304 Not Modified\r\nETag: "<grösse>-<crc>"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n
```

- **`304` nur, wenn** der Wert trifft **und** die Datei mit Grösse > 0 besteht (`http_fs_file_exists_and_nonempty()`,
  `:2049`) **und** beim Index `app_complete` wahr ist (`:2826-2829`). Eine leere oder fehlende Datei bekommt nie
  `304` — sonst verdeckte der Browser-Cache den Weissschirm-Fall.
- **Nur Weisslisten-Assets als `.gz`** bekommen `ETag` und `HTTP/1.1`; jede andere Auslieferung (`:2789-2792`,
  Fallback-Namen) bleibt, wie sie ist.
- **Zähler** in `device_ready` hinter I-5: `app_not_modified`, `app_full`, `app_etag_calc` (je `uint16_t`, sättigend).
  Damit ist **am Gerät** ablesbar, ob ein Browser ohne CDP — Safari — nachfragt.

### 4.5 Was C70 nicht anfasst

- **`sw.js`:** unter `http://` ohne Wirkung; unter https holt der Precache mit `cache: "reload"` (`sw.js:82`) am
  HTTP-Cache vorbei, die Laufzeitabrufe (`:139-202`) werden mit `304` billiger. Nur `CACHE_NAME` (R2).
- **`reloadAppPage()`** (`app.js:13225-13253`) löscht CacheStorage, nicht den HTTP-Cache; nach `/app/` wird jedes
  Asset nachgefragt und, falls neu, geliefert.
- **Safari (◐):** Zurück-Cache stellt eine Seite ohne Laden wieder her (unabhängig von C70). AKW.4 liest an
  `app_full`/`app_not_modified`, was Safari tut.

### 4.6 PWA-Teil (Ent-3: immer)

`loadLayoutPreviewRowsMap()` (`app.js:11647-11664`) holt mit `cache: "no-store"`; der Browser speichert nicht und
sendet nie `If-None-Match`. `"no-cache"` holt bei jedem Aufruf frisch nach, jetzt mit Validator. Firmware zuerst, dann
PWA (DIR-002) — beide Paarungen sind sicher (§11).

---

## 5. Messungen

### 5.1 Ladeprobe (Werkzeug `ladeprobe`, Lead; liegt bereits vor, ✔ L)

Headless-Chrome mit **dauerhaftem** Profil, CDP mit `Network.enable`:

1. Profil leer, `device_ready?heap_low_reset=1`, **kalt** laden, Netzruhe abwarten (1'500 ms, Regel aus `check-pwa`),
   `device_ready` lesen.
2. Fünfmal **warm**: dreimal `Page.reload` mit `ignoreCache: false`, zweimal `Page.navigate` auf `/app/` (das „zweite
   Öffnen“); je vorher `heap_low_reset=1`, danach lesen.
3. Je Laden: `/app/`-Anfragen nach **Leitungsstatus** (`responseReceivedExtraInfo.statusCode` — `responseReceived`
   meldet bei `304` den Cache-Status `200`, ✔ L), ob `If-None-Match` gesendet wurde (`requestWillBeSentExtraInfo`),
   Summe `encodedDataLength`, `heap_low_block`, `heap_low_path`, I-4-Felder, `app_not_modified`, `app_full`.

`--erwarte-304` lässt ein warmes Laden mit Leitungsstatus `200` auf ein App-Asset fehlschlagen. Gegenproben AKZ.2 (i),
darunter der Prüfserver mit `HTTP/1.0` (muss FEHL melden). **Keine Haltegrenze** (Ent-7). Englisch über den
Sprachschlüssel im `localStorage` (wie K33).

### 5.2 Probe U — Update sofort wirksam (AKW.3)

`layout-previews.json` um Leerraum ergänzt, `gzip -9`, über `app_file_upload` als `app/layout-previews.json`. Beim
nächsten Laden: genau diese Datei `200` mit neuem `ETag`, eine `- app etag`-Zeile, alle anderen `304`. Original
zurück — wieder `200`, alter `ETag`. Ohne Einzelfreigabe zulässig (Ent-15 entfällt, `CLAUDE.md:416`).

### 5.3 Browser-Sitzung (Werkzeug `browser-sitzung`, Lead)

Vorbedingung: die Uhr gehört der Messung (`TESTPLAN-PWA.md` Abschnitt 0 Punkt 5), Beginn und Ende angekündigt;
Safari und Chrome zu Beginn **beendet**; die Wache läuft.

| Phase | Was | Dauer |
|---|---|---|
| **S0** | Ruhe, kein Browser | 180 s |
| **S1** | Browser öffnet `http://<uhr>/app/`, Seite sichtbar | 120 s |
| **S2** | neuer Tab `about:blank` im Vordergrund — PWA verborgen | 180 s (> 120 s TIME_WAIT) |
| **S3** | PWA-Tab geschlossen, Browser läuft | 300 s (> 20 s FIN_WAIT_2, > TIME_WAIT) |
| **S4** | Browser beendet | 180 s |

**Bedienung (Ent-6 C):** der Lead per `open -a Safari <url>` und `osascript` (neuer Tab, Tab schliessen, `quit`) bzw.
das Gegenstück für Chrome — macOS fragt beim ersten Mal nach der Erlaubnis zur Automation, die Du einmal gibst.
**Einmal** fährt der Nutzer dieselben Phasen auf seinem Gerät aus L461 auf Zuruf des Leads; dort gibt es nur die
ESP-Seite. In **G10** wird vor S0 einmal geöffnet und der Browser beendet, damit sein Cache Validatoren hält.

**Lesung am Phasenende** (eine Verbindung; ihr TIME_WAIT erscheint in der nächsten Lesung und wird so markiert):
`device_ready` ohne Parameter; `netstat -anv -p tcp | grep <uhr>`; `lsof -nP -i @<uhr>`. **Dazwischen keine Lesung** —
die Minutenreihe liefert die Heap-Zeile mit `tcp=` und `nrt=`.

### 5.4 Entscheidungsregel C72 (AKW.5)

„Lücke“ = `free_heap`(S0) − `free_heap`(Phase). Bezug Mac = Gegenstelle mit dem Oktett des messenden Geräts.
Zugeordnet wird auf den Daten von **G10** (der Stand, auf dem R3 aufsetzt); G9 steht daneben, eine Abweichung kommt in
den Bericht.

| Ergebnis | Bild | Kandidat (Ent-8) |
|---|---|---|
| **M1** — geschlossen, nicht abgebaut | in S2 oder S3 mindestens ein PCB mit `eigen` = 0 in FIN_WAIT_1, FIN_WAIT_2, CLOSING oder LAST_ACK, Gegenstelle Mac, `ruhe_s` ≥ 30; Lücke ≥ 500; in S4 verschwunden | **K2** |
| **M2** — Strom von Vorab-Verbindungen | in S2/S3 steigt `nrt` um ≥ 1 je Minute; in ≥ 2 von 3 Heap-Zeilen `fw2`/`tw` > 0; kein PCB mit `ruhe_s` ≥ 30; Lücke ≥ 500; in S4 steht `nrt` still | **K1** |
| **M3** — beides | M1 und M2 | **K1 + K2** |
| **M4** — angenommen und hängend | ein PCB mit `eigen` = 1, nicht die eigene Lesung, ESTABLISHED oder CLOSE_WAIT, `ruhe_s` ≥ 30 — oder `tcp_now_pending` ≥ 1 in zwei Lesungen hintereinander | **kein Eingriff** aus K1–K5; eigener Befund, Analyse am Core |
| **M5** — keine Verbindung | Lücke ≥ 500, aber ausser der eigenen Lesung und ≤ 2 TIME_WAIT nichts in der Liste — **oder** keine Lücke ≥ 500 (nicht nachgestellt) | **kein Eingriff**; C72-Hypothese widerlegt bzw. nicht nachgestellt |

Ist TIME_WAIT in jeder Lesung voll und trägt sichtbar zur Lücke bei, kommt **K5** als Zusatz in Frage. Chrome wird
gleich ausgewertet; zeigt Chrome dasselbe, ist es keine Safari-Eigenart.

### 5.5 Startmessung und Entscheidungsregel C71 (AKM.5, AKS.2)

**Ablauf je Start** (kein Browser, keine Lesung über HTTP in den ersten 5 min — sie wäre eine Anfrage und verschöbe den
Startabgleich um 5 s und den Messpunkt `erste anfrage`):

1. ESP neu starten über den Weg, den V3 im Testplan benutzt (STM-Reset reisst den ESP mit, L183), oder
   `/api/local_esp_restart`; beides ohne Einzelfreigabe zulässig (`CLAUDE.md:416`).
2. Mitschnitt der ersten 5 min: Heap-Zeilen (`max`, `low=`, `tcp=`), `- update cache …`, `- update cache heap …`.
3. Danach Smoketest samt **Update-Quelle** (DIR-009, L42) und `- heap erste anfrage …`.

**Drei Starts:** zwei ohne Änderung (erwartet je Datei `unveraendert`), einer mit **erzwungenem Schreiben** — vorher
`/upd-wc-list.txt` über `/api/fs_remove` löschen (keine App-Datei, kein Eintrag der Gefahrentabelle), erwartet
`geschrieben`.

| Ergebnis | Bild | Kandidat (Ent-17) |
|---|---|---|
| **N1** — im Abgleich, mit und ohne Schreiben | in jedem Start fällt der Block in einem Schritt um ≥ 1'000 (`s4`/`s5` gegen den Schritt davor) und erholt sich bis zur nächsten Heap-Zeile nicht | **L4**; mit **L2**, falls P9 eine langlebige Belegung im Fenster belegt |
| **N2** — schon beim Abgleich ohne Schreiben, in den Versionszeilen oder beim Mount | Fall in `s1`–`s3` oder in `s4`/`s5` auch bei `unveraendert` | **L1** oder **L2** nach P10/P11 |
| **N3** — nur beim Schreiben | Fall nur im Start mit `geschrieben` | **L4** |
| **N4** — nicht im Abgleich | `vor` ist schon geteilt (oder `erste anfrage` zeigt den kleinen Block), oder kein Schritt fällt um ≥ 1'000 | **keine Massnahme**; C71 bleibt offen mit Befund |

---

## 6. Eingriffskandidaten C72 (R3, Ent-8)

| | Mechanismus | Wirkung (◐ bis G11) | Risiko | Speicher |
|---|---|---|---|---|
| **K1** | im Zweig „keine Anfrage“ (`http.cpp:16681-16710`) `abort()` statt `stop()` — RST statt FIN | eine Vorab-Verbindung hinterlässt weder FIN_WAIT noch TIME_WAIT | Browser sieht RST auf einer **unbenutzten** Verbindung (◐: öffnet neu); eine Anfrage nach 250 ms bekommt RST statt FIN ohne Antwort — dieselbe Fehlerklasse wie heute; AKE.3 sucht `ERR_CONNECTION_RESET` | 0 |
| **K2** | höchstens einmal je Sekunde aus `loop()`: PCBs in `tcp_active_pcbs` mit `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING, LAST_ACK und `ruhe_s` > Frist (Ent-14) ⇒ `tcp_abort()`; Zähler `tcp_reaped`; nach jedem Abbau Gang von vorn (P3) | gebundener Speicher nach der Frist frei | **schreibender** Zugriff auf lwIP (Ent-10); ein PCB mit `ClientContext` **nie** — auch kein laufender Wetter- oder Update-Abruf; RST an eine Gegenstelle, die nicht mehr antwortet | ~4 Byte |
| **K3** | FIN_WAIT_2-Frist verkürzen | — | in die Bibliothek übersetzt (`tcp_priv.h:131`) — **geht in K2 auf** | — |
| **K4** | Höchstzahl je Gegenstelle | begrenzt einen Browser | **hoch:** bis zu 6 parallele Asset-Verbindungen, zwei Browser mit derselben Adresse; RST mitten im Laden ⇒ fehlendes `app.js` | 0 — **nicht empfohlen** |
| **K5** | `HTTP_TW_LIMIT` 2 ⇒ 1 (`http.cpp:464`) | höchstens ~0,2 KB (●) | wie E4 §19.7, eine Stufe schärfer | 0 |

**Frist in K2 (Ent-14):** empfohlen 5 s; endgültig mit den gemessenen Ruhezeiten.

---

## 7. Kandidaten C71 (R3 nach Ent-16 A, Ent-17)

| | Mechanismus | Wirkung (◐ bis G11) | Risiko | Speicher |
|---|---|---|---|---|
| **L1** — LittleFS einmal mounten | beim Start einmal `LittleFS.begin()`, die Mount-Puffer bleiben liegen; alle `begin()`/`end()`-Paare (rund 30, ✔ SW Grep) werden zu No-ops bzw. entfallen; `format()` hängt kurz aus und wieder ein | keine wiederholten Anforderungen und Freigaben der Mount-Puffer zwischen langlebigen Belegungen; die Puffer liegen ganz unten im Heap, der grosse Block bleibt am Stück | breiter Eingriff (viele Funktionen); Verhalten von `begin()` bei schon gemountetem FS ist P10; ein Fehler beim Aushängen vor `format()` | dauerhaft auf dem Heap, Grösse nach P10 (◐ rund 1 KB) — senkt `free`, nicht `max` |
| **L2** — Abgleich vor dem ersten Netzverkehr | den ersten Durchgang nach dem Start ausführen, **bevor** `http_server_begin()` Anfragen annimmt, statt frühestens 60 s danach | der Abgleich findet einen leeren, unzerstückelten Heap vor; was er hinterlässt, liegt unten | die 60-s-Bedingung hat einen Grund (A79, L421 — P11); Startzeit länger um die Dauer des Abgleichs; schläft der Update-Server, wartet der Start (Sperrfenster) | 0 |
| **L3** — Puffer statisch | Mount- und Dateipuffer aus statischem Speicher statt `malloc` | keine Heap-Anforderung im Fenster | ob die Core-API das zulässt, ist P10; **sprengt die RAM-Grenze 250 Byte** (Ent-11) — nur mit eigener Grenze in Ent-17 | statisch ◐ ≥ 1 KB |
| **L4** — `File`-Objekte und Reihenfolge | in `update_cache_write()`: erst den Abruf vollständig in die Temporärdatei, **dann** die alte Datei öffnen — nie zwei `File` gleichzeitig (`:13677` und `:13699`); `LittleFS.begin()` erst, wenn der Abruf steht, und `LittleFS.end()` **vor** `httpclient_stop()`, damit die Freigaben in umgekehrter Reihenfolge der Anlage geschehen | weniger gleichzeitige Belegungen im Fenster, kein Loch zwischen zwei Freigaben | der E4.5a-Vergleich liest heute alte Datei und Server **gleichzeitig**; ihn umzustellen, ändert c65n — Prüfstand nachziehen | 0 |
| **L5** — ausgehende Verbindung mit RST beenden | `httpclient_stop()` im Abgleich per `abort()`, damit kein TIME_WAIT-PCB 120 s lang im Fenster liegt | nur, wenn der Fall nach 120 s verschwindet — L460 zeigt einen **bleibenden** Fall; daher nur Zusatz | RST an den Update-Server | 0 |

**Empfehlung je Ergebnis:** siehe §5.5. Die Massnahme folgt der Ursache, die P9 und I-6 zeigen — nicht umgekehrt.

---

## 8. Prüfstände

Ablage `tools/checks/auszug/esp/c72/`, `…/esp/c71/`, `…/esp/c70/` (Vorbild `…/esp/c65/`), PWA neben b48; Aufnahme
in S16. Fallzahl gegen das, was es geben müsste (L235); einmal gegen den Vorgängerstand fehlgeschlagen; je Massnahme
mindestens eine Sabotage (DIR-014). Der Umsetzer baut sie im Scratchpad, der Lead legt sie ab und fährt die
Gegenproben. Attrappen bilden die Core-Semantik nach (Checkliste Punkt 4, letzte Frage).

| Prüfstand | Release | Gegenstand | Fälle (Auszug) | Sabotage ⇒ FEHL |
|---|---|---|---|---|
| **c72i** | R1 | I-5 | Attrappen `tcp_active_pcbs` (alle 11 Zustände, `callback_arg` gesetzt/leer, verschiedene `tmr`), `tcp_tw_pcbs` (0–5), `tcp_ticks`; Zählung, Sättigung, Liste ≤ 8 + `tcp_now_more`, `ruhe_s`, `eigen`; Feldreihenfolge; statisch kein `http_send` im Listengang; Heap-Zeile Format, ≤ 119, Mitschnitt und Ring gleich | Ausgabe in der Listenschleife; Kappung entfernt; Heap-Zeile nur `Serial` |
| **c71i** | R1 | I-6 | Durchgang mit 5, 3 und 0 Schritten (Abbruch); `-` für nicht gelaufen; Werte nach `LittleFS.end()`; genau eine Zeile je Durchgang; `erste anfrage` genau einmal je Start; ≤ 119 | ein Schritt ohne Messwert; nur `Serial`; Zeile je Schritt statt je Durchgang |
| **c70e** | R2 | C70 (a) | ohne `If-None-Match` ⇒ `HTTP/1.1 200` + `ETag`; Treffer ⇒ `HTTP/1.1 304` ohne Körper und ohne `Content-Length`; Fehltreffer ⇒ `200`; `W/`, Liste, `*`, überlang, Kopfname klein; leere Datei nie `304`; Index ohne `app_complete` ⇒ Installationsseite (`HTTP/1.0`); zwei Inhalte gleicher Grösse ⇒ verschiedene `ETag`; nach Invalidierung und nach Grössenänderung neu berechnet; Lesefehler beim Berechnen ⇒ `HTTP/1.1 200` ohne `ETag`; Asset ausserhalb der Weissliste ⇒ kein `ETag`, `HTTP/1.0`; Körperlänge = `Content-Length`; Zähler; **statisch:** jede schreibende LittleFS-Stelle mit `app_etag_invalidate()` danach (Zahl gegen P6); `HTTP/1.1` in `http.cpp` an genau zwei Stellen, beide mit `Connection: close` | Invalidierung entfernt; `ETag` nur aus Grösse; `304` ohne Grössenprüfung; `304` mit Körper; `304` ohne `Connection: close`; `HTTP/1.1` in einer `/api/`-Antwort |
| **c70p** | R2 | PWA | statisch: `loadLayoutPreviewRowsMap` mit `"no-cache"`; Sprachdatei weiter mit `?v=` | `"no-store"` |
| **c72e** | R3 | Eingriff C72 | K1: Zweig „keine Anfrage“ ruft `abort()`, Zähler und andere Wege unverändert. K2: gemischte PCBs — nur `callback_arg == NULL` + Zustand aus der Menge + `ruhe_s` > Frist abgebaut; höchstens ein Gang je Sekunde; Neubeginn nach Abbau; TIME_WAIT und LISTEN unberührt. K5: Grenze 1 | K1 `stop()`; K2 PCB mit `ClientContext` abgebaut, Frist entfernt; K5 Grenze 2 |
| **c71e** | R3 | Massnahme C71 | je nach Ent-17: L1 — `begin()` einmal, kein `end()` ausser vor `format()`; L2 — Abgleich vor `http_server_begin()`; L4 — nie zwei `File` gleichzeitig offen, Reihenfolge `end()` vor `httpclient_stop()`, E4.5a-Vergleich weiter richtig (c65n grün) | je Kandidat die Umkehrung seiner Regel |

---

## 9. Werkzeuge (Lead)

- **`ladeprobe`** — §5.1; liegt vor (✔ L) und liest den Leitungsstatus; Gegenproben AKZ.2 (i) samt Prüfserver
  `HTTP/1.0` ⇒ FEHL und `HTTP/1.1` ⇒ OK.
- **`browser-sitzung`** (neu) — §5.3; jede Lesung mit Uhrzeit in Datei **und** Konsole; Gegenprobe AKZ.2 (ii).
- **`check-pwa`** (R2) — INFO „`/app/` 304 je Laden“ aus `responseReceivedExtraInfo.statusCode` (nicht aus
  `responseReceived`, das bei `304` `200` meldet) und „Byte je Laden“; kein neuer Fehlschlag.
- **`smoke-device.sh`** — ab R1 die I-5-Felder, ab R2 die `app_*`-Felder; setzt nichts zurück.
- **`watch-log.sh`** — ab R1 Heap-Zeile mit Zusatz, `- update cache heap` als Hinweis; ab R2 `- app etag` als Hinweis.
- **Statuszeile (R2):** Lead durchsucht `tools/**` nach Prüfungen auf `HTTP/1.0` bei App-Assets und zieht sie nach.

---

## 10. RAM-Budget (AKZ.8)

| Teil | Release | statisch (Byte) | Bemerkung |
|---|---|---|---|
| I-5 | R1 | 0 | Stackpuffer rund 80 Byte, nur während der Lesung |
| Heap-Zeile | R1 | 0 | Stackpuffer 80 ⇒ 120 |
| I-6 | R1 | ~16 | 5 Schrittwerte, `vor`, Merker „erste Anfrage“ |
| `ETag`-Tabelle | R2 | 104 | 13 × (Grösse + CRC) |
| `If-None-Match`-Puffer | R2 | 36 | |
| Zähler `app_*` | R2 | 6 | |
| K2 | R3 | ~4 | |
| C71 L1/L2/L4/L5 | R3 | 0 | L1 belegt Heap dauerhaft (P10) |
| **Summe** | | **rund 170** | **Grenze 250 Byte (Ent-11)**; L3 sprengt sie und braucht mit Ent-17 eine eigene |

Gemessen nach **jedem** ESP-Task (Ausgang Build E4: 69'060 Byte, L457).

---

## 11. Releaseschnitt, Einspielen, Rückweg

**R1** — nur ESP. Rückweg: ESP 3.2.37 aus `wordclock-release-2026-10-14-180012.zip` (L457).

**R2** — ESP und PWA, **Firmware zuerst, dann PWA** (DIR-002, Ent-3). Versionen setzt der `release-engineer`;
`APP_VERSION` und `CACHE_NAME` mit.

| ESP | PWA | Lage |
|---|---|---|
| R1 | 1.4.100 | vor R2 |
| R2 | 1.4.100 | **sicher** — Zwischenstand zwischen OTA und Upload; die alte PWA holt `layout-previews` weiter vollständig |
| R1 | R2-PWA | **sicher** — `no-cache` ohne Validator holt vollständig, wie heute (Rückweg) |
| R2 | R2-PWA | Ziel |

Ablauf: Wache; ESP-OTA; Smoketest samt Update-Quelle; `install-app.sh --check`; `install-app.sh`; `--check`;
`check-pwa.sh`. **Rückweg R2:** ESP auf den Stand R1 aus dessen ZIP; die PWA bleibt (sicher). Ein Browser, der
`ETag`-Werte hält, bekommt von R1 wieder `HTTP/1.0 200` — kein Schaden.

**R3** — nur ESP. Rückweg auf den Stand R2 aus dessen ZIP; mit Ent-16 A nimmt er C71 **und** C72 zurück.

Du spielst jedes Release ein; der Lead liefert die fertigen Zeilen samt Rückweg, **bevor** eingespielt wird.

---

## 12. Prüfpunkte für den `firmware-analyst` (Task A.1, lesend)

| P | Frage | Wofür |
|---|---|---|
| **P1** | Setzt `tcp_close()` in `liblwip2-536-feat.a` `TF_RXCLOSED`, sodass FIN_WAIT_2 nach 20 s abläuft? Sendet lwIP dabei RST? | M1, K2, Ent-14 |
| **P2** | Begrenzt `MEMP_NUM_TCP_PCB` = 5 bei `MEMP_MEM_MALLOC` = 1 die Zahl aktiver PCBs? Ist der Kommentar `http.cpp:325` zu berichtigen? | I-5-Kappung, I-4 |
| **P3** | `tcp_abort()` aus `loop()` auf einem PCB ohne `callback_arg`: Rückrufe, Freigabe, RST? Darf der Listengang danach fortgesetzt werden? | K2 |
| **P4** | Wann setzt diese lwIP-Fassung `pcb->tmr`? Sind `nrtx`, `remote_ip`, `callback_arg` vorhanden? | `ruhe_s`, §5.4 |
| **P5** | Bauart und Kosten von `crc32()` im Core 3.1.2; Tabelle im Flash vorzuziehen? | AKC.4, Watchdog |
| **P6** | Vollständiger Bestand der schreibenden LittleFS-Stellen in allen ESP-Quellen, auch mit Modus in einer Variablen; welche können `app-*.gz` schreiben | AKC.3, c70e |
| **P7** | Wartet `stop()` im Zweig „keine Anfrage“ (Flush bis 300 ms)? Was ändert `abort()` am `ClientContext`? | K1 |
| **P8** | `ClientContext` in `_unclaimed`, dessen Gegenstelle vor dem Abholen schliesst — bleibt Speicher hängen? | M4 |
| **P9** | **C71:** Was legt ein Durchgang des Startabgleichs auf den Heap, in welcher Reihenfolge, und was **überlebt** ihn? Kandidaten: Mount-Puffer von LittleFS (`lfs_mount`/`lfs_unmount`), `File`-Objekte (geteilte Zeiger, Dateipuffer), `WiFiClient`/`ClientContext` des Abrufs, DNS-Ergebnis, TIME_WAIT-PCB, `String`s in `update_server_open()`/`update_server_fetch_line()`, erstmals angelegte Einzelobjekte (lazy) des Cores. Am Ablauf `http.cpp:13595`–`:13771` | N1–N4, L1–L5 |
| **P10** | **C71:** Ist `LittleFS.begin()` bei gemountetem FS ein No-op? Wie gross sind die Mount-Puffer (`lfs_config`) und lassen sie sich statisch vorgeben? Bestand der `begin()`/`end()`-Paare | L1, L3 |
| **P11** | **C71:** Warum frühestens 60 s nach dem Start (A79, L421)? Was belegt der Heap bis dahin dauerhaft? Ginge der erste Durchgang vor `http_server_begin()`, ohne die Bedingung zu verletzen? | L2 |

Ergebnis als Bericht an den Lead, der es ins Messprotokoll trägt. **Widerlegt ein Prüfpunkt eine Annahme dieser Spec**
(etwa P1: FIN_WAIT_2 läuft nie ab), legt der `spec-writer` vor R1.1 einen Nachtrag vor (Task A.2). P9–P11 dürfen bis
zum Tor nachgeliefert werden — sie entscheiden erst Ent-17.

---

## 13. Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Werden die Wetter-Endpunkte eingehalten? Bleibt die
Restore-Bedingung vollständig? Greift die Änderung an der richtigen Schicht an?

> **Legacy:** bleibt parallel, unverändert und bei `HTTP/1.0`. Ihre Seiten sind dynamisch und tragen keinen `ETag`;
> C70 setzt nur in `http_app()`/`http_send_fs_file()` für Weisslisten-Assets an. AKC.5 prüft die Legacy-Auslieferung
> samt Statuszeile gegen den Stand R1. **Wetter:** keine Berührung von `/api/weather_get_now` und
> `/api/weather_get_forecast`; K2 lässt laufende ausgehende Abrufe in Ruhe; L5 beträfe nur den Abruf des
> Startabgleichs, nicht das Wetter. **Restore-Bedingung:** STM unberührt. **`.gz` und Grösse > 0:** `.gz`-only bleibt;
> `304` setzt die Grössenprüfung **voraus**. **Schicht:** C70 liegt in der Auslieferung des ESP (kein Validator, falsche
> Statuszeile) und wird dort behoben; der PWA-Teil beseitigt ein `no-store`, das jeden Validator wirkungslos machte.
> C71 wird am Startabgleich gemessen und dort angegangen, wo die Messung die Ursache zeigt. C72 wird erst gemessen,
> dann im ESP angegangen — Browser lassen sich nicht steuern.

**Scalable systems** — STM-Kommandos? Byte pro Minute auf der UART? Unter 20 s Watchdog? Blockade des Hauptloops?

> **STM-Kommandos:** keine. **UART:** Heap-Zeile + rund 38 Byte je Minute; `- update cache heap` einmal je Durchgang
> (Start, `refresh=1`), `- heap erste anfrage` einmal je Start, `- app etag` einmal je Asset nach Start oder Schreiben —
> im Gegenzug fallen je warmem Laden rund 200 KB Übertragung weg. **Hauptloop:** CRC einmal je Asset nach Start oder
> Schreiben, geschätzt 0,2–0,5 s (◐, P5; AKC.4: < 2 s); jede `304` Millisekunden statt rund einer Sekunde für `app.js`;
> I-5 ein Gang über rund zehn Einträge je Lesung und Minute; I-6 höchstens sieben Heap-Gänge je Start; K2 ein Gang je
> Sekunde. L2 verlängert den Start um die Dauer des Abgleichs (◐, bis 5 s je Abruf, `http.cpp:13383-13384`),
> blockiert aber keine Anfrage, weil noch keine angenommen wird. **Watchdog:** keine Schleife ohne Grenze; die
> CRC-Schleife endet mit der Datei und gibt ab. **Feste Grenzen:** Liste 8, `If-None-Match` 36 Byte, Tabelle so gross
> wie `APP_INSTALL_ASSETS`. **PWA-Polling:** unverändert.

**Secure by design** — `escapeHtml` bei `innerHTML`? Fremddaten? Credentials in Logausgaben?

> **Fremddaten:** `If-None-Match` ist Eingabe aus dem Netz — begrenzter Puffer, abgeschnitten, nur verglichen, nie
> ausgegeben. **Logausgaben:** `- app etag`, `- update cache heap`, `- heap erste anfrage` nennen nur Dateinamen und
> Zahlen; `tcp_now_list` das letzte Oktett und Ports — keine Query, keine Zugangsdaten. **Neue Angriffsfläche:** kein
> neuer Endpunkt, kein neuer Parameter; ein richtiger `If-None-Match` verrät nur den `ETag`, der in jeder `200` steht.
> K1/K2 senden RST nur an Verbindungen ohne Anfrage bzw. an vom ESP geschlossene. **Zerstörende Wege:** unverändert
> (Schutz-Spec, C6, C61); Probe U und das erzwungene Schreiben berühren keine Datei der Gefahrentabelle.

**Stable & reliable** — Fehler ausgewertet? Leere `catch`? Stille Verwerfungen? Zustand nach Abbruch? Flags auf jedem
Pfad?

> **Veralteter Cache (C70):** ausgeschlossen durch drei unabhängige Sicherungen — Inhalt statt Version, Grössenvergleich
> vor jeder Verwendung, Invalidierung an **jeder** Schreibstelle mit statischer Prüfung. Restrisiko: eine übersehene
> Schreibstelle, die die Grösse nicht ändert — c70e und P6 sollen sie finden. **`HTTP/1.1`:** `Connection: close` in
> beiden Antworten, exaktes `Content-Length`, Abbruch mit RST — ein Browser kann weder eine Verbindung
> wiederverwenden noch einen gekürzten Körper als vollständig nehmen. **Abbruch in einer Sequenz:** Bricht die
> CRC-Berechnung ab, bleibt der Eintrag ungültig, die Antwort geht ohne `ETag`; scheitert der Rücksprung an den
> Dateianfang, geht nichts hinaus, was nicht vom Anfang stammt (§4.4). Bricht ein Upload ab, ist die Tabelle schon
> invalidiert. **C71:** L4 ändert den Ablauf von `update_cache_write()`; die bestehenden Sicherungen (Temporärdatei,
> Längenprüfung, Umbenennen nur nach vollständigem Lesen, E3.8 N4) bleiben, c65n muss grün bleiben (AKS.3). **Stille
> Verwerfungen:** jede `304`, jede Berechnung und jeder Abbau durch K2 werden gezählt. **Flags auf jedem Pfad:** der
> `If-None-Match`-Puffer wird am Anfang **jeder** Anfrage geleert; der I-6-Merker „erste Anfrage“ wird einmal gesetzt
> und nie zurückgesetzt (je Start). **Messung ohne Selbsttäuschung:** jede Lesung ist eine Verbindung und wird markiert;
> die Entscheidungsregeln stehen vor der Messung fest; Neustarts sind Ergebnisse, keine Störungen (Ent-7).

---

## 14. Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| `ETag` bei `HTTP/1.0` belassen | greift in Chrome nicht (Prüfserver des Leads, §1) |
| `HTTP/1.1` für alle Antworten | kein Gewinn ausserhalb der Assets; jede weitere Stelle ein Risiko (Keep-Alive-Erwartung) |
| `Last-Modified`/`If-Modified-Since` statt `ETag` | Zeitstempel im LittleFS hängen an der Systemzeit beim Schreiben (◐), Sekundenauflösung; ob Chromium ihn bei `HTTP/1.0` nimmt, ist so offen wie beim `ETag`. Bleibt Rückfallebene, falls Safari bei 1.1 nicht nachfragt (AKW.4, G10.7) |
| `ETag` aus Grösse allein bzw. aus der App-Version | Kollision bzw. der ESP kennt sie nicht (§4.4) |
| CRC bei jeder bedingten Anfrage neu berechnen | 0,2–0,5 s Hauptloop je `app.js`-Anfrage, während weitere Verbindungen im Backlog Heap binden |
| `ETag` in eigenen Dateien neben den Assets | zusätzliche Schreibvorgänge im LittleFS — genau die Spur von C71 |
| `Cache-Control: max-age=<kurz>` ohne Versionierung | verletzt „sofort wirksam“ |
| Variante (b) jetzt | Fenster mit veraltetem Cache für ein Jahr, fünfte Versionsstelle |
| Service Worker über `http://` | vom Browser ausgeschlossen |
| `device_ready` während der Sitzung im Sekundentakt lesen | jede Lesung ist eine Verbindung und verändert das Gemessene |
| HTTP-Lesungen während der Startmessung | verschieben den Startabgleich (5-s-Regel) und den Messpunkt `erste anfrage` |
| `tcpdump` als Hauptinstrument | braucht `sudo`; `netstat`/`lsof` genügen; Zusatz für M4 |
| K3, K4 | siehe §6 |
| C72-Eingriff ohne Messung | Mechanismus nicht belegt |
| C71-Massnahme vor der Startmessung | Korrelation ist keine Ursache; L1 oder L3 auf Verdacht wären breite Eingriffe ohne Beleg |
| C71 in R2 (Ent-16 C) | G10 soll die Wirkung des Caches getrennt zeigen |

---

## 15. Versionsfolgen

- [ ] ESP `version.h` anheben — **R1**, **R2**, **R3** (falls R3 stattfindet)
- [ ] App `APP_VERSION` anheben — **R2**
- [ ] `CACHE_NAME` in `sw.js` anheben — **R2**, mit `APP_VERSION`
- [ ] STM `src/main.h` — **nicht**

Nur der `release-engineer` führt das aus (R4). Die Nummern bestimmt der Lead; diese Spec setzt keine.

---

## 16. Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `ESP8266/ESP-uclock/http.cpp` (UTF-8, eine CRLF-Zeile) | R1: I-5, Zählfunktion, I-6; R2: `If-None-Match`, Tabelle, CRC, `HTTP/1.1` für die zwei Asset-Antworten, `304`, Zähler, `- app etag`, `app_etag_invalidate()` an allen Schreibstellen; R3: K1/K2/K5, C71 (L1/L2/L4/L5) | `esp-developer` |
| `ESP8266/ESP-uclock/stm32flash.cpp` (UTF-8) | R2: `app_etag_invalidate()` an drei Schreibstellen; R3 mit L1: `begin()`/`end()` | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` (gemischte Zeilenenden) | R1: Heap-Zeile; R2: Invalidierung nach `LittleFS.format()` (`:291`); R3: K2-Aufruf, L1/L2 beim Start | `esp-developer` |
| `ESP8266/ESP-uclock/base.h` | Prototypen | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | R2: eine Zeile (`app.js:11655`) | `pwa-developer` |
| `tools/ladeprobe*`, `tools/browser-sitzung*`, `tools/check-pwa.mjs`, `tools/smoke-device.sh`, `tools/watch-log.sh`, Prüfstände | §8, §9 | Lead |
| Versionsdateien | ESP (R1, R2, R3); `APP_VERSION`/`CACHE_NAME` (R2) | `release-engineer` |
| `TESTPLAN-PWA.md`, `BEFUNDE.md` | Felder und Zeilen samt Lesart, `304` als Normalfall, Instrumente, K30 nach Ent-12; Träger an C65, C70, C71, C72 | `doc-writer` |
| `CHANGELOG.md` | je Release | Lead |
