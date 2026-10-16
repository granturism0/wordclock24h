# Design — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge mit den Entscheiden des Nutzers, den
Lead-Entscheiden, dem Prüfserver-Befund zu `HTTP/1.0`, **A.2** (P1–P8), der **Trennmessung** (L468), **A.3** (P7, P9,
P11, P12), **A.4** (Review R1.7, 16.10.2026) und **A.5** (Ursache von C71 am Gerät belegt, L468, 16.10.2026, Commit
`89ec34c`). **Status: freigegeben am 15.10.2026 bis aufs Tor; Nachträge A.2 bis A.5 zur Freigabe.** Was sich ändert und
was widerlegt ist: §17. Offen: Ent-2 (Lead, vor R2.1), **Ent-18** (neu, Nutzer, vor R2), Ent-8, Ent-10, Ent-14 (Tor),
Ent-17 (faktisch L7, Freigabe offen), Ent-16 (nur mit Ent-18 B). Momentaufnahme (DIR-006); der lebende Stand steht in
`BEFUNDE.md`.

**Zeilenangaben:** Alle `Datei:Zeile`-Anker beziehen sich auf `72cfda7`. In `http.cpp` liegen sie seit R1 ab rund
`:2839` um rund +210 später (Lead). **Massgeblich ist die genannte Funktion bzw. das Symbol.**

**Kennzeichen** wie in `requirements.md`: **✔ SW** am Code `72cfda7` bzw. Core 3.1.2 nachgelesen; **✔ B** in
`BEFUNDE.md`; **✔ L** vom Lead gemessen; **✔ FA** vom `firmware-analyst` am Core belegt; **◐ FA** vom
`firmware-analyst` als plausibel gemeldet; **? FA** Spekulation; **✔ R** / **◐ R** vom `code-reviewer` in R1.7 belegt
bzw. als Lesart gemeldet; **◐** abgeleitet; **●** gemeldet, vor der Umsetzung zu prüfen.

---

## 1. Befund mit Verifikationsstatus

| Aussage | Status | Fundstelle / Bemerkung |
|---|---|---|
| Volles Neuladen mit einem Client reisst K30: `low=752`, `max` 3'584 | **✔ B** | L466 |
| Jedes Öffnen holt alle Assets, rund 200 KB, davon `app-app.js.gz` 163'621 Byte | **✔ B** / **✔ SW** | C70, E3.0; `http_send_fs_file()` |
| Auslieferung `HTTP/1.0 200 OK`, gzip, `no-cache`, `Connection: close`, kein Validator | **✔ SW** / **✔ L** | `http_send_fs_file()`; `curl -D` |
| Chrome nimmt einen `ETag` aus `HTTP/1.0` nicht als Validator; mit `HTTP/1.1` schon | **✔ L** / **◐** (Ursache) | Prüfserver 15.10.2026 22:21–22:23; Safari ungeprüft |
| Nur `http_send_fs_file()` muss auf `HTTP/1.1`; übrige `/app/`-Antworten in `http_app()` bleiben 1.0; kein Keep-Alive | **✔ FA** | `http_send_fs_file()`, `http_app()` |
| CDP meldet bei `304` in `responseReceived` den Cache-Status `200` | **✔ L** | Leitungsstatus in `responseReceivedExtraInfo.statusCode` |
| `If-None-Match` wird nicht ausgewertet; Query vor `http_app()` abgetrennt; ESP kennt die installierte App-Version nicht | **✔ SW** | Kopfzeilenschleife in `http_server_loop()`; `APP_VERSION_TXT` |
| Kein Service Worker über `http://`; `layout-previews.json` mit `no-store` | **✔ SW** | `registerAppServiceWorker()`, `loadLayoutPreviewRowsMap()` |
| `install-app.sh` und Auto-Installation laden `index.html` zuerst | **✔ SW** | `tools/install-app.sh:36-38`; `APP_INSTALL_ASSETS` |
| Core-`crc32()` ist CRC-32/MPEG-2, bitweise | **✔ FA** / **✔ SW** | `cores/esp8266/crc32.cpp:26-43` |
| 31 schreibende LittleFS-Stellen, vollständig; AutoFormat in `LittleFS.begin()` | **✔ FA** | §4.4 |
| `LittleFS.remove()` vor `LittleFS.begin()` in `http_api_app_file_upload()` wirkungslos; `LittleFS.format()` in `setup()` in `#if 0` | **✔ FA** / **✔ SW** | Nebenbefunde |
| Nach dem Start fällt `max` von 7'184 auf 4'672 und bleibt | **✔ B** | L460, L457 |
| **C71, Ursache belegt (A.5):** `http_api_update_status()` prüft am Ende per `GET` `wc12h-icon.txt` und `wc12h-weather.txt` auf Existenz und ruft bei `len > 0` sofort `httpclient_stop()`, **ohne den Rumpf zu lesen** | **✔ L** | L468 (16.10.2026, `89ec34c`) |
| `WiFiClient::stop()` ⇒ `ClientContext::close()` lässt `_rx_buf` stehen; frei wird es erst bei Refcount 0 (`unref()` ⇒ `discard_received()`), also beim nächsten `connect()` des statischen Clients | **✔ SW** / **✔ L** | `ClientContext.h:73-92` (`close()` ohne Freigabe), `:115-127` (`unref()`); `httpclient.cpp:16` |
| Die ungelesene Antwort (Kopf 250 + Rumpf 1'215 Byte) liegt in drei pbufs: 616 + 616 + 472 = **1'704 Byte** | **✔ L** | M3: Kopf 250, `Content-Length` 1215, Rumpf gleich Repo |
| Ein kalter `update_table_files`, der zu Ende liest und den Client neu verbindet, gibt **exakt +1'704** frei (5'624/4'744 ⇒ 7'328/7'152) | **✔ L** | M1 |
| G9.6: Start 3 (Schreiben beim Start) zerstückelt **nicht**; Start 4 (ein `update_status`) belegt die 1'704; ein zweiter `update_status` belegt nichts zusätzlich | **✔ L** | L468 |
| Die Safari-Lücke aus L461 betrug genau 1'704 Byte — sie gehört zu C71, nicht zu C72 | **✔ L** / **✔ B** | L461, L468 |
| Dasselbe Muster bei der Prüfung auf `app.js.gz` (bis rund 2'464 Byte) und im Zweig `zu-gross` von `update_cache_write()` | **✔ L** (Lead) | `http.cpp` ~2701-2710 (nach R1 verschoben); `update_cache_write()` |
| Direkte `client.stop()` in den Fehlerwegen von `httpclient()` lassen einen Rumpf (etwa eine 404-Seite) ebenso liegen | **✔ SW** | `httpclient.cpp:216`, `:224` |
| **C75:** `httpclient()` gibt bei `len == 0` mit offenem Client zurück; Aufrufer, die bei `len <= 0` aufgeben, schliessen nicht | **✔ R** / **✔ SW** | `httpclient.cpp:220-228`; `update_cache_write()` |
| **Widerlegt (A.5):** H-CC (Platzierung des `ClientContext`) als Ursache; „Schreiben beim Start zerstückelt“ (N1a/N1b); PUYA-Puffer (N0); Vermutung „der Zwischenspeicher hält einen String“ | **✔ L** | Start 3 ohne Zerstückelung; M1 gibt exakt die pbuf-Belegung frei |
| **Nicht unmittelbar erklärt:** L460 (G8, 14.10.2026) sah den Fall im Startfenster laut Protokoll **ohne** Anfrage | **◐** | §5.5 offener Punkt |
| Wetter-Abruf mit eigenem `WiFiClient`, von `hc` nicht erfasst und von L7 nicht berührt | **✔ SW** | `weather.cpp:16` (`openweather_client`) |
| Grund der 60 s nach dem Start: STM-Vollabgleich nach `IPADDRESS` liefert Host und Pfad | **✔ FA** (P11) | A79, L421 |
| `http_server_begin()` steht in `wifi_connect()` vor `IPADDRESS` | **✔ FA** / **✔ SW** | `wifi.cpp` |
| Ausser LittleFS schreibt beim Start praktisch nichts Flash; `.tmp`-Löschen und AutoFormat ohne Logzeile | **✔ FA** (P12) | `eeprom_commit()`; `update_cache_poll()` |
| Safari-Sitzung: `free` −1,7 KB über Minuten; frei erst beim Beenden | **✔ B** / **●** | L461 — die 1'704 Byte sind C71 |
| `stop()` ⇒ `tcp_close()` ⇒ FIN, `TF_RXCLOSED`; FIN_WAIT_2 läuft rund 20 s nach dem letzten Segment ohne RST ab | **✔ SW** / **◐ FA** (P1) | P1-Probe §5.6 |
| FIN_WAIT_1, CLOSING, LAST_ACK ohne eigene Frist, nur Wiederholungen bis `TCP_MAXRTX` 12 | **◐ FA** | `lwipopts.h:1258` |
| `pcb->tmr` je angenommenem Segment, nicht nach `TF_RXCLOSED`, nicht beim Zustandswechsel; TIME_WAIT endet über dieselbe `tmr` | **◐ FA** (P4) / **◐ R** (H4) | §3.1, §5.5 |
| SYN_RCVD trägt den Listener; ESTABLISHED mit `callback_arg == NULL` möglich (`TF_CLOSEPEND`) | **◐ FA** (P3, P8) | `lwip/tcp.h:255` |
| Tote `ClientContext` in `_unclaimed`; `!http_client` kehrt ohne Zähler zurück | **◐ FA** / **✔ SW** | `http_server_loop()` |
| K1 wirkt nur im Teilfall `no_request_timeouts`; bei `aborts` hat der Core schon abgebaut | **✔ FA** (P7) | `ClientContext.h:598-614` |
| **C74:** Warteschlaufe auf die erste Zeile endet nach dem letzten `delay(1)` ohne erneutes `available()` | **✔ FA** | nicht Teil des Pakets |
| `ESP.getHeapStats()` liefert `frag`; `fe` nur im privaten `umm_local.h`; `frag` reagiert kaum auf ein kleines Loch | **✔ R** | §3.3 |
| `tcp=`/`hc` in `- update cache nach` rund 10 ms nach `n` | **✔ R** (H2) | vermerkt |
| `MEMP_NUM_TCP_PCB` begrenzt bei `MEMP_MEM_MALLOC` nichts; TIME_WAIT im Core ≤ 6, nach `tcp_tw_limit()` 2 | **✔ FA** (P2) | — |

---

## 2. Zuschnitt, Releases, Reihenfolge

| Release | Inhalt | Laufzeit | danach | Schreiber |
|---|---|---|---|---|
| **R1** | **I-5** (§3.1, §3.2) und **I-6** (§3.3) | ESP | **G9** ohne Cache | `esp-developer` |
| **R1b** *(nur mit Ent-18 A)* | **L7** samt Fehlerwegen und C75 (§7) — Behebung von C71 | ESP | **G9b** | `esp-developer` |
| **R2** | **C70** (§4): `ETag` nach Ent-2, `304`, `HTTP/1.1` für App-Assets; PWA-Teil | ESP **und** PWA | **G10** mit Cache, **Tor** | `esp-developer`, `pwa-developer` |
| **R3** | **Eingriff C72** (§6, Ent-8); mit Ent-18 B auch **L7** | ESP | **G11** | `esp-developer` |

**Ent-18 (neu, Nutzer) — wann L7 kommt.** Der Schnitt aus Ent-9/Ent-16 legte die C71-Massnahme in R3, weil die Ursache
offen war. Jetzt ist sie belegt, und sie trifft **jeden Nutzer nach jedem Start**, sobald `update_status` einmal kalt
abgefragt wird — das tut die PWA von sich aus.

| | (A) eigenes Release **R1b** vor R2 | (B) wie bisher in **R3** | (C) zusammen mit **R2** |
|---|---|---|---|
| Wirkung für Dich | Fehler im Bestand sofort behoben; der K30-Ruhewert 5'120 wird erreichbar | erst nach dem Tor | mit R2 |
| Messungen danach | C72-Sitzungen und G10 ohne die 1'704 Byte als Störgrösse | jede C72-Sitzung muss den Client vorher freistellen (§5.3); G10 trägt die Belegung mit | G10 mischt die Wirkung von L7 und C70 — gegen die Absicht von Ent-9 |
| Aufwand | ein OTA mehr; Ladeprobe-Grundlinie auf R1b neu (G9b) | keiner | keiner |
| Risiko | klein: eine Zeile in `httpclient_stop()`, zwei Fehlerwege, C75; Prüfstand c71e | — | wie A, aber nicht getrennt messbar |

**Empfehlung A.** Ein belegter, kleiner und gut prüfbarer Fehler im Bestand, der jeden Nutzer trifft und die Messungen
aller folgenden Schritte verfälscht, gehört nicht ans Ende. Die Browser-Sitzungen G9.3–G9.5 laufen dann auf R1b;
Ent-16 entfällt.

**Abhängigkeiten nach R3b:** Alle ESP-Teile schreibt **ein** `esp-developer` nacheinander. Der PWA-Teil in R2 darf
parallel laufen. Code für R2 darf entstehen, während G9/G9b laufen; gebaut und eingespielt erst danach. Code für R3
erst nach dem Tor.

---

## 3. Instrumente (R1)

### 3.1 I-5 — laufende Verbindungszustände in `device_ready` (C72)

**Hinter `api_orphan_skipped`** (in `http_api_device_ready()`):

| Feld | Inhalt |
|---|---|
| `tcp_now_states` | 11 Zahlen: PCBs je Zustand in `tcp_active_pcbs`, Index = `enum tcp_state`, sättigend 255; die eigene Verbindung mitgezählt |
| `tcp_now_tw` | Länge von `tcp_tw_pcbs` |
| `tcp_now_pending` | `accepts_pending` am Listen-PCB |
| `tcp_now_unclaimed` | Länge der Liste `_unclaimed` der `WcHttpServer` (über `ClientContext::next()`) |
| `tcp_now_self_port` | `http_client.remotePort()` dieser Anfrage |
| `tcp_now_list` | höchstens 8 Felder `[zustand, gegenstelle, port, ruhe_s, nrtx, arg]`, erst `tcp_active_pcbs`, dann `tcp_tw_pcbs` |
| `tcp_now_more` | PCBs, die nicht in die Liste passten |
| `http_accept_dead` | Zähler seit dem Start, sättigend: `accept()` holte einen Kontext aus `_unclaimed`, dessen Gegenstelle schon geschlossen hatte; gezählt **nur**, wenn vor `accept()` `_unclaimed` nicht leer war |

- **`ruhe_s` (◐ FA P4):** `(tcp_ticks − pcb->tmr) × TCP_SLOW_INTERVAL / 1000` — „seit dem letzten empfangenen Segment
  vor unserem `close()`“. Nach H4 (◐ R) endet TIME_WAIT, wenn `ruhe_s` rund 120 überschreitet.
- **`arg` (◐ FA P3/P8):** 0 = `callback_arg == NULL`; 1 = `ClientContext`; 2 = Listener (SYN_RCVD). ESTABLISHED oder
  CLOSE_WAIT mit `arg` = 0 ist ein vom ESP geschlossener PCB mit `TF_CLOSEPEND`.
- `nrtx`: Wiederholungen des laufenden Segments. `gegenstelle`: letztes Oktett (Ent-5).

**Ohne Abgabe:** Listen in einen Stackpuffer, bevor das erste `http_send()` läuft; kein `String`, kein Heap. **Kappung
auf 8** (P2).

### 3.2 I-5 — Heap-Zeile (Ent-4)

```
- heap free=<n> max=<n> ovr=<n> low=<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>
```

Zählfunktion in `http.cpp`, aufgerufen aus `esp_heap_log()` in `ESP-uclock.ino`. Längste Form 105 ≤ 119.

### 3.3 I-6 — Startzustand, Heap im Startabgleich, Flash-Kennung (C71)

Je Durchgang von `update_cache_poll()` drei Zeilen, dazu Einzelzeilen:

**(a) vor Schritt 1** (nach dem Löschen liegengebliebener `.tmp`; `vorher` und `v` enthalten dessen Wirkung):

```
- update cache vorher up=<s> free=<n>/<blk> frag=<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> hc=<httpclient_connects>
```

**(b) am Ende oder beim Abbruch — Verlauf:**

```
- update cache heap v=<free>/<blk> 1=<free>/<blk> 2=… 3=… 4=… 5=… n=<free>/<blk>
```

**(c) unmittelbar nach (b):**

```
- update cache nach frag=<v>/<1>/<2>/<3>/<4>/<5>/<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> hc=<n> acc=<Δ>
```

- Tragend für Zerstückelung ist die **Lochsumme `L = free − blk`** je Messpunkt, an der Heap-Zeile `free − max` (A.4).
- **`frag`** aus `ESP.getHeapStats()` — nur ergänzend, geringe Empfindlichkeit (20'000 + 100 Byte ⇒ 0–1 Punkte).
- `tcp=`/`hc` in (c) rund 10 ms nach `n` (H2). `hc` erfasst den Wetter-Client nicht. `acc`: Δ von
  `http_accept_count`.

**(d) `- heap erste anfrage free=<n> max=<n>`** beim ersten lebenden `accept()` nach dem Start, auch bei einer
Vorrats-Verbindung. **(e) `- flash id=<hex6> vendor=<hex2>`** in `setup()` vor WLAN und `EEPROM.begin()`. **(f)
`- update cache tmp geloescht <datei>`**; das AutoFormat bleibt ohne Logzeile.

**Nach A.5:** Die Belegung von C71 entsteht in `http_api_update_status()`, **ausserhalb** des Abgleichs; I-6 zeigt sie
nur mittelbar (`hc`, `free`/`blk` beim nächsten Durchgang). Die Wirkung von L7 wird deshalb um einen kalten
`update_status` gemessen (§5.5, AKS.4); I-6 bleibt als Instrument für den Startzustand.

Je Zeile ≤ 119 Zeichen, einmal formatiert, Mitschnitt und Ring. Bestehende Zeilen `- update cache <datei> …`
unverändert.

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
| **(a)** mit `HTTP/1.1` und PWA-Teil | ≈ N × 110 ≈ **0,8 KB**; Prüfserver: 290 statt 4'045 Byte (✔ L) | N, je sehr kurz |
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

#### `ETag`-Quelle (Ent-2, offen)

| | **T — gzip-Trailer** (Lead-Empfehlung) | **V — Voll-CRC** |
|---|---|---|
| **Wert** | `"<gz-grösse>-<16 Hexziffern>"`: die letzten 8 Byte der `.gz` — CRC-32 (zlib) und Länge des **entpackten** Inhalts (RFC 1952) | `"<grösse>-<8 Hexziffern>"`: Core-`crc32()` — **CRC-32/MPEG-2** |
| **Wann** | bei **jeder** Antwort: `fp.size()`, `seek` ans Ende − 8, 8 Byte, `seek` an den Anfang; erste 2 Byte auf `1f 8b` | einmal je Asset nach Start oder Schreiben; RAM-Tabelle |
| **Kosten** | je Antwort ein `seek` und 10 Byte (◐ ms) | einmalig 0,15–0,3 s für `app.js.gz` (◐ FA), Zeile `- app etag`, 104 Byte RAM |
| **Gültigkeit** | **durch Bau** | Grössenvergleich + `app_etag_invalidate()` an jeder der 31 Stellen |
| **Trennschärfe** | für jede gültige `.gz` wie V; abgeschnitten ⇒ anderer Wert | über die gespeicherten Bytes |
| **Ungültige `.gz`** | kein `ETag` (sichere Richtung) | `ETag` über die Bytes |
| **Werkzeuge** | erwarteter Wert aus den letzten 8 Byte der Repo-Datei | nur CRC-32/MPEG-2 |

**Empfehlung T**; V als Rückfallebene, falls AKC.4 eine messbare Mehrdauer je Anfrage zeigt.

#### Statuszeile `HTTP/1.1` — nur für App-Assets (AKC.7)

Nur die Kopfzeilen in `http_send_fs_file()` und die neue `304` tragen `HTTP/1.1` (✔ FA). Ohne `ETag` geht
`HTTP/1.1 200` ohne `ETag` hinaus.

| Risiko | Bewertung | Massnahme |
|---|---|---|
| Keep-Alive-Erwartung unter 1.1 | real ohne `Connection: close` | `Connection: close` in beiden; c70e statisch |
| Körperende durch Verbindungsende | gering: `Content-Length`, Abbruch mit RST | exakt; c70e vergleicht |
| `304` mit Körper oder `Content-Length` | verwirrt den Browser | ohne beides |
| `Date` | ohne verlässliche Uhr nicht zu senden (RFC 9110 §6.6.1, ◐) | kein `Date` |
| Werkzeuge mit Prüfung auf `HTTP/1.0` | ◐ | Task R2.6 |
| Prüfstand-Attrappen c65b/c65w, LittleFS-Attrappe | ✔ FA | AKC.8, R2.4a |
| Legacy, `/api/*`, Hinweisseiten in `http_app()` | bleiben 1.0 | AKC.5, Sabotage |
| Safari | ungeprüft | AKW.4; sonst `Last-Modified` (G10.7) |

#### Schreibstellen (nur Variante V tragend; ✔ FA vollständig)

Stand `72cfda7`: `http.cpp` 27, `stm32flash.cpp` 3, `ESP-uclock.ino` 1 (in `#if 0`) — **31**; nach R1 verschoben, c70e
zählt selbst.

#### `If-None-Match`, Antworten, Zähler

`If-None-Match` in der Kopfzeilenschleife von `http_server_loop()` in einen statischen Puffer von 36 Byte, je Anfrage
geleert; Kopfname ohne Rücksicht auf Gross-/Kleinschreibung; `W/` übergangen, Komma-Liste, `*` kein Treffer,
überlange Werte treffen nicht.

```
HTTP/1.1 200 OK\r\nContent-Type: <typ>\r\nContent-Encoding: gzip\r\nContent-Length: <n>\r\n
ETag: "<wert>"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n<körper>

HTTP/1.1 304 Not Modified\r\nETag: "<wert>"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n
```

- **`304` nur**, wenn der Wert trifft, die Datei mit Grösse > 0 besteht und beim Index `app_complete` wahr ist.
- Scheitert ein `seek`, wird nichts ausgeliefert, was nicht vom Dateianfang stammt.
- **Zähler** hinter I-5: `app_not_modified`, `app_full`, mit V `app_etag_calc`.

### 4.5 Was C70 nicht anfasst

`sw.js` (nur `CACHE_NAME`, R2); `reloadAppPage()`; Safaris Zurück-Cache (◐).

### 4.6 PWA-Teil (Ent-3: immer)

`loadLayoutPreviewRowsMap()`: `"no-store"` ⇒ `"no-cache"`. Firmware zuerst, dann PWA.

---

## 5. Messungen

### 5.1 Ladeprobe (`ladeprobe`, Lead; liegt vor)

Dauerhaftes Profil: kalt, dann fünfmal warm (dreimal `Page.reload` ohne `ignoreCache`, zweimal `Page.navigate`), je
vorher `heap_low_reset=1`, danach lesen. Je Laden: Leitungsstatus (`responseReceivedExtraInfo`), `If-None-Match`
gesendet, `encodedDataLength`, `heap_low_block`, `heap_low_path`, I-4, `app_*`. `--erwarte-304`. Keine Haltegrenze.
**Mit Ent-18 A** läuft auf R1b eine neue Grundlinie (G9b), damit G10 die Wirkung von C70 ohne die von L7 zeigt.

### 5.2 Probe U (AKW.3)

`layout-previews.json` um Leerraum ergänzt, `gzip -9`, über `app_file_upload`; danach Original zurück. Ohne
Einzelfreigabe (`CLAUDE.md:416`).

### 5.3 Browser-Sitzung (`browser-sitzung`, Lead)

| Phase | Was | Dauer |
|---|---|---|
| **S0** | Ruhe | 180 s |
| **S1** | Browser öffnet `http://<uhr>/app/`, sichtbar | 120 s |
| **S2** | neuer Tab `about:blank` vorne — PWA verborgen | 180 s |
| **S3** | PWA-Tab geschlossen, Browser läuft | 300 s |
| **S4** | Browser beendet | 180 s |

**Betriebsarten:** `osascript` (das Skript bedient) und **`manuell`** (das Skript nennt Phase und Handgriff, der Nutzer
bedient auf seinem Gerät und bestätigt, das Skript liest). In G10 vor S0 einmal öffnen und beenden.

**Lesung am Phasenende:** `device_ready` ohne Parameter, `netstat`, `lsof`. Je eigene Lesung lokaler Port und Ende ihres
TIME_WAIT-Fensters (Lesezeit + 120 s; nach H4 eine obere Schranke). Je Sitzung Δ`no_request_timeouts` gegen
Δ`no_request_aborts` und Δ`http_accept_dead` über S1–S3.

**Client freistellen (A.5), vor jeder C72-Sitzung, solange L7 nicht eingespielt ist:** Die PWA fragt beim Öffnen
`update_status` ab; ist der Abruf kalt, liegt danach bis zum nächsten `connect()` des httpclient eine Antwort von
1'704 Byte im Heap (C71). Ohne Gegenmassnahme erschiene sie in S2/S3 als „Lücke“ und landete fälschlich in M2 oder M5.
Deshalb: vor S0 den Client freistellen — ein kalter `update_table_files`, der zu Ende liest, oder ein Durchgang des
Abgleichs — und je Phasenende `hc` (I-6, `httpclient_connects`) und `update_info_cached` mitprotokollieren (das Feld
dort lesen, wo es ohnehin anfällt; kein eigener kalter Abruf). Steigt `hc` oder wechselt `update_info_cached` während
der Sitzung, wird eine Lücke um 1'704 Byte C71 zugerechnet, nicht C72. **Mit L7 entfällt das Freistellen**; es wird
für den Vergleich mit G9 einmal mitgefahren.

### 5.4 Entscheidungsregel C72 (AKW.5)

„Lücke“ = `free_heap`(S0) − `free_heap`(Phase), **nach Abzug einer C71-Belegung** (§5.3). Eigene Lesungen und ihr
TIME_WAIT-Fenster zählen nicht; Heap-Zeilen in einem eigenen Fenster zählen für M2 nicht. Zugeordnet wird auf G10; G9
und die P1-Probe daneben.

**Was nach A.5 für C72 bleibt:** Die 1'704 Byte aus L461 gehören zu C71. Für C72 bleiben aus L461 nur der Anteil an
`max` (4'672 ⇒ 3'816), soweit die C71-Belegung ihn nicht erklärt, und die Erholung um +600 um 20:07. **Ob K1 und K2
danach überhaupt noch einen Gegenstand haben, entscheiden die Browser-Sitzungen G9.3–G9.5** (mit Ent-18 A auf R1b); bis
dahin bleiben sie Kandidaten. Zeigt keine Sitzung eine Lücke ≥ 500 nach Abzug von C71, gilt M5: Der C72-Eingriff
entfällt, und C72 wird mit diesem Ergebnis geschlossen oder umformuliert.

| Ergebnis | Bild | Kandidat (Ent-8) |
|---|---|---|
| **M1** — geschlossen, nicht abgebaut | in S2 oder S3 ein PCB mit `arg` = 0, Gegenstelle Mac, in FIN_WAIT_1, CLOSING oder LAST_ACK mit `nrtx` > 0, oder ESTABLISHED/CLOSE_WAIT (`TF_CLOSEPEND`), in zwei Lesungen hintereinander; Lücke ≥ 500; in S4 verschwunden. FIN_WAIT_2 mit `ruhe_s` ≥ 30 gehört auch hierher und belegt zugleich **gegen** P1 | **K2** |
| **M2** — Strom von Vorab-Verbindungen | in S2/S3 steigen `nrt` oder `http_accept_dead` um ≥ 1 je Minute; in ≥ 2 Heap-Zeilen ausserhalb eigener Fenster `fw2`/`tw` > 0; kein PCB aus M1; Lücke ≥ 500; in S4 stehen beide still | **K1 nur, wenn Δ`no_request_timeouts` den Anstieg trägt**; sonst kein K1, Befund, K5 als Zusatz prüfen |
| **M3** — beides | M1 und M2 | **K2**, dazu K1 unter derselben Bedingung |
| **M4** — angenommen und hängend | ein PCB mit `arg` = 1, nicht die eigene Lesung, ESTABLISHED oder CLOSE_WAIT, `ruhe_s` ≥ 30 — oder `tcp_now_unclaimed` ≥ 1 in zwei Lesungen hintereinander; SYN_RCVD zählt nicht | **kein Eingriff**; eigener Befund |
| **M5** — keine Verbindung | Lücke ≥ 500 ohne Einträge ausser eigenen und ≤ 2 TIME_WAIT — **oder keine Lücke ≥ 500 nach Abzug von C71** | **kein Eingriff** |

**Widerlegt die P1-Probe P1**, ist **K2 zwingend**.

### 5.5 C71 — Ergebnis und Entscheidungsregel (A.5)

**Belegt (✔ L, L468):**

| Messung | Ergebnis |
|---|---|
| G9.6 Start 3 — Schreiben beim Start | **keine** Zerstückelung (Lochsumme `L` unverändert) |
| G9.6 Start 4 — ein kalter `update_status` | belegt **1'704 Byte** (`free` und `max`) bis zum nächsten `connect()` des httpclient |
| zweiter `update_status` | belegt nichts zusätzlich |
| M3 | Kopf 250, `Content-Length` 1215, Rumpf gleich Repo — drei pbufs 616 + 616 + 472 = 1'704 |
| M1 | ein kalter `update_table_files`, der zu Ende liest und neu verbindet, gibt **exakt +1'704** frei (5'624/4'744 ⇒ 7'328/7'152) |

**Ergebnisse, neu gefasst:**

| Ergebnis | Bild | Stand | Kandidat (Ent-17) |
|---|---|---|---|
| **N5** — Belegung nach kaltem `update_status` | Nach einer Existenzprüfung ohne Rumpflesen bleibt die Antwort im `_rx_buf` des statischen httpclient bis zum nächsten `connect()` | **trifft zu** (M1, M3, Start 4) | **L7** |
| N0 — PUYA-Puffer | — | **nicht zutreffend** (L468 (b)) | — |
| N1a, N1b — Startzustand, Platzierung (H-CC) | — | **widerlegt** (Start 3 ohne Zerstückelung; M1 gibt exakt die Belegung frei) | — |
| N2, N3, N4 | — | gegenstandslos | — |

**Offener Punkt (◐):** L460 sah den Fall nach dem OTA von E4 im Startfenster, laut Protokoll **ohne Anfrage**. N5
erklärt das nur, wenn dort doch ein `update_status` oder ein anderer Abruf ohne Rumpflesen lief — die Prüfung auf
`app.js.gz` oder der Zweig `zu-gross` in `update_cache_write()`. Der Lead prüft den Mitschnitt von L457/L460 darauf und
trägt das Ergebnis ins Messprotokoll. Die Wirkung von L7 belegt AKS.4 unabhängig davon.

**Messung der Wirkung von L7 (AKS.4):** wie Start 4 und M1 — ESP in Ruhe, Heap-Zeile lesen, **einen** kalten
`update_status`, danach keine HTTP-Lesung, bis eine Heap-Zeile ≥ 130 s danach mit `tw=0` vorliegt (◐ H4); `free` und
`max` vorher und nachher.

### 5.6 P1-Probe (AKM.7)

Ein Python-Socket vom Mac sendet `GET /api/device_ready HTTP/1.0\r\n\r\n`, liest bis zum FIN des ESP und schliesst
**nicht**. Aus einer zweiten Verbindung `device_ready` nach 10 s und nach 25 s; `netstat` auf der Mac-Seite. Erwartet
unter P1: nach 10 s FIN_WAIT_2, `arg` = 0, Port der Probe; Mac-Seite CLOSE_WAIT; nach 25 s weg. **Noch da ⇒ P1
widerlegt, K2 zwingend.**

---

## 6. Eingriffskandidaten C72 (R3, Ent-8) — Kandidaten, bis G9.3–G9.5 entschieden haben

| | Mechanismus | Wirkung (◐ bis G11) | Risiko | Speicher |
|---|---|---|---|---|
| **K1** | im Zweig „keine Anfrage“ von `http_server_loop()`: `abort()` statt `stop()`, **nur im Teilfall `no_request_timeouts`**; im Teilfall `aborts` hat der Core schon abgebaut | still gehaltene Vorab-Verbindung ohne FIN_WAIT und TIME_WAIT | RST an eine unbenutzte Verbindung; kein neues Fenster (späte Anfragen bekommen schon heute RST); der Wettlauf **C74** erzeugt schon heute Resets — deshalb AKE.3 gegen die Grundlinie R2 | 0 |
| **K2** | einmal je Sekunde: PCBs mit `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING, LAST_ACK und — bei `TF_CLOSEPEND` — ESTABLISHED, CLOSE_WAIT, mit `ruhe_s` > Frist ⇒ `tcp_abort()`; Gang neu nach jedem Abbau; Zähler `tcp_reaped` | gebundener Speicher nach der Frist frei | schreibender Zugriff auf lwIP (Ent-10); nie `arg` = 1 oder 2 | ~4 Byte |
| **K3** | FIN_WAIT_2-Frist verkürzen | — | in die Bibliothek übersetzt — **geht in K2 auf** | — |
| **K4** | Höchstzahl je Gegenstelle | — | hoch — **nicht empfohlen** | 0 |
| **K5** | `HTTP_TW_LIMIT` 1 | ~0,2 KB (●) | wie E4 §19.7, schärfer | 0 |

**Frist in K2 (Ent-14):** 5 s empfohlen. Ist Δ`timeouts` ≈ 0, ist K1 für L461 wirkungslos.

---

## 7. Massnahme C71 — L7 (Ent-17 faktisch entschieden; Release nach Ent-18)

**Begründung (A.5):** Ein Abruf, dessen Rumpf nicht gelesen wird, lässt die Antwort im `_rx_buf` des `ClientContext`
liegen, denn `WiFiClient::stop()` ⇒ `close()` gibt sie nicht frei (`ClientContext.h:73-92`); erst Refcount 0 ruft
`discard_received()` (`:115-127`). Der statische httpclient (`httpclient.cpp:16`) hält den Kontext bis zum nächsten
`connect()`. **Belegt:** 1'704 Byte nach jedem kalten `update_status` (M1, M3, Start 4). Die Platzierung des
56-Byte-Kontexts (früher H-CC) ist nur noch **Nebenwirkung** — mit L7 ist er nach jedem Abruf ohnehin frei.

**L7 besteht aus drei Teilen:**

| Teil | Was | Deckt ab |
|---|---|---|
| **L7a** | `httpclient_stop()`: nach `client.stop()` den Kontext freigeben (`client = WiFiClient()`) — Refcount 0, `discard_received()` | **zentral** jede Prüfung ohne Rumpflesen: `update_status` (`wc12h-icon.txt`, `wc12h-weather.txt`), die Prüfung auf `app.js.gz` (bis rund 2'464 Byte), den Zweig `zu-gross` in `update_cache_write()` — ohne Änderung an diesen Aufrufern |
| **L7b** | die direkten `client.stop()` in den Fehlerwegen von `httpclient()` (`httpclient.cpp:216` keine Antwort, `:224` Status ≠ 200) über `httpclient_stop()` führen | den Rumpf einer Fehlerseite (etwa 404), der sonst liegen bliebe |
| **L7c = C75** | `httpclient()` bei `len == 0` und jeder Aufrufer, der bei `len <= 0` aufgibt (darunter `update_cache_write()`), schliesst über `httpclient_stop()` | jeden Abruf, der heute mit offenem Client endet |

**Risiko (◐):** Nach der Freigabe darf kein Code mehr auf den alten Kontext zugreifen (c71e prüft das, und dass jeder
weitere Abruf funktioniert). Jeder Abruf legt einen frischen Kontext an (rund 56 Byte). RAM 0.

**Option, nicht Ersatz: `HEAD` statt `GET` für Existenzprüfungen.**

| | Bewertung |
|---|---|
| Wirkung | kein Rumpf, also nichts im `_rx_buf` nach dem Kopf; rund 1,2 KB weniger Übertragung je Prüfung |
| Was es nicht deckt | Fehlerseiten (`:216`, `:224`), `len <= 0` (C75), den Zweig `zu-gross`, jeden künftigen Aufrufer, der den Rumpf nicht liest — **L7 bleibt nötig** |
| Aufwand und Risiko | `httpclient()` kennt nur `GET`; eine Methode als Parameter; ob der Update-Server `HEAD` mit gleichem Status beantwortet, ist ◐ (am Server zu prüfen); ein Aufrufer, der nach `HEAD` doch liest, wartete bis zur Frist |
| Empfehlung | **nicht in diesem Paket**; als spätere Verbesserung benannt |

**Verworfen mit Beleg (A.5):**

| Kandidat | Warum nicht |
|---|---|
| **L6** (Abgleich nach 300 s) | Start 3 zerstückelt nicht; die Ursache liegt nicht im Zeitpunkt des Abgleichs (M1) |
| **L0** (PUYA-Puffer früh anlegen) | L468 (b); N0 nicht zutreffend |
| **L2** (Abgleich vor `http_server_begin()`) | nicht umsetzbar (P11) und ohne Bezug zur Ursache |
| **L4b**, **L1**, **L3**, **L4a**, **L5** | ohne Bezug zur Ursache bzw. schon in A.2 verworfen |
| Vermutung „der Zwischenspeicher hält einen String“ | widerlegt: M1 gibt exakt die pbuf-Belegung der ungelesenen Antwort frei |

Mit dem Wegfall von L6 ist an `TESTPLAN-PWA.md` (Startfrist 60 s) und c65n nichts nachzuziehen.

---

## 8. Prüfstände

Ablage `tools/checks/auszug/esp/c72/`, `…/c71/`, `…/c70/`, PWA neben b48; in S16. Fallzahl gegen das, was es geben
müsste (L235); einmal gegen den Vorgängerstand fehlgeschlagen; je Massnahme eine Sabotage (DIR-014). Attrappen bilden
die Core-Semantik nach.

| Prüfstand | Release | Gegenstand | Fälle (Auszug) | Sabotage ⇒ FEHL |
|---|---|---|---|---|
| **c72i** | R1 | I-5 | Attrappen `tcp_active_pcbs` (alle 11 Zustände; `callback_arg` NULL, `ClientContext`, Listener; ESTABLISHED mit NULL), `tcp_tw_pcbs`, `tcp_ticks`, `_unclaimed`; `arg` 0/1/2; Liste ≤ 8 + `tcp_now_more`; `http_accept_dead` nur bei nicht leerem `_unclaimed`; kein `http_send` im Listengang; Heap-Zeile ≤ 119, Mitschnitt = Ring | Ausgabe in der Schleife; Kappung entfernt; SYN_RCVD als `ClientContext`; toter Abholversuch ohne Zähler; leerer Durchlauf gezählt; nur `Serial` |
| **c71i** | R1 | I-6 | `vorher` einmal vor Schritt 1 (nach dem `.tmp`-Gang) mit `up`, `free/blk`, `frag`, `tcp=`, `hc`; `heap` mit `free`/`blk` je Schritt; `nach` mit `frag` je Messpunkt, `tcp=`, `hc`, `acc`; 5, 3, 0 Schritte; `-`; Werte nach `LittleFS.end()`; `tmp geloescht`; `erste anfrage` beim ersten lebenden `accept()`; `flash id` vor `EEPROM.begin()`; je ≤ 119 | Schritt ohne Wert; nur `blk`; `vorher` nach Schritt 1; `nach` ohne `hc`; `acc` nicht als Δ; `tmp geloescht` fehlt; nur `Serial` |
| **c71e** | **R1b** bzw. R3 (Ent-18) | **L7** | Attrappe `ClientContext` mit Refcount und `_rx_buf` — Semantik wie der Core: `close()` gibt `_rx_buf` **nicht** frei, Refcount 0 ruft `discard_received()`. **(1) Existenzprüfung mit ungelesenem Rumpf** (Kopf 250 + 1'215 Byte in drei pbufs) ⇒ nach `httpclient_stop()` Refcount 0, `discard_received()` gelaufen, Heap-Attrappe wieder auf dem Stand vorher; **(2) Fehlerweg „keine Antwort“** (`httpclient.cpp:216`) und **(3) Status 404 mit Rumpf** (`:224`) ⇒ ebenso frei; **(4) `len == 0`** (C75) ⇒ geschlossen und frei; **(5) Zweig `zu-gross`** in `update_cache_write()` ⇒ frei; **(6)** ein weiterer Abruf nach der Freigabe funktioniert, kein Zugriff auf den alten Kontext; **(7)** ein Abruf, der zu Ende liest, verhält sich wie vorher | **„nur `stop()`“** (ohne Freigabe des Kontexts) ⇒ FEHL in (1); Fehlerweg mit direktem `client.stop()` ⇒ FEHL in (3); `len == 0` offen ⇒ FEHL in (4) |
| **c70e** | R2 | C70 (a) | `HTTP/1.1 200` + `ETag`; Treffer ⇒ `HTTP/1.1 304` ohne Körper und `Content-Length`; Fehltreffer; `W/`, Liste, `*`, überlang, Kopfname klein; leere Datei nie `304`; Index ohne `app_complete` ⇒ Installationsseite (`HTTP/1.0`); ausserhalb der Weissliste `HTTP/1.0` ohne `ETag`; Körperlänge = `Content-Length`; `seek` scheitert. **T:** zwei `.gz` gleicher Grösse, anderer Inhalt ⇒ andere `ETag`; abgeschnitten; ohne `1f 8b`. **V:** CRC-32/MPEG-2; Invalidierung an allen Stellen. **Statisch:** `HTTP/1.1` an genau zwei Stellen mit `Connection: close` | `ETag` nur aus Grösse; `304` ohne Grössenprüfung; mit Körper; ohne `Connection: close`; `HTTP/1.1` in `/api/`; V: Invalidierung entfernt, CRC mit zlib |
| **c70p** | R2 | PWA | `"no-cache"` in `loadLayoutPreviewRowsMap`; Sprachdatei mit `?v=` | `"no-store"` |
| **c65b, c65w, LittleFS-Attrappe** | R2 | Bestand | Kopfzeilen in einem Schreibvorgang mit `HTTP/1.1` und `ETag`; `seek`, Trailer- bzw. zweites Lesen | wie bisher |
| **c72e** | R3 | Eingriff C72 | K1 `abort()` nur im Teilfall Timeout; K2 nur `callback_arg == NULL` + Zustand + `ruhe_s` > Frist; SYN_RCVD, `ClientContext`, TIME_WAIT, LISTEN unberührt; ein Gang je Sekunde; Neubeginn nach Abbau; K5 Grenze 1 | K1 `stop()` bzw. auch im Teilfall Abbruch; K2 PCB mit `callback_arg` ≠ NULL bzw. SYN_RCVD abgebaut, Frist entfernt; K5 Grenze 2 |

---

## 9. Werkzeuge (Lead)

- **`ladeprobe`** — liegt vor; Leitungsstatus; mit Ent-18 A neue Grundlinie auf R1b.
- **`browser-sitzung`** — `osascript` und `manuell`; je eigene Lesung Port und TIME_WAIT-Ende; je Sitzung
  Δ`no_request_timeouts`, Δ`no_request_aborts`, Δ`http_accept_dead`; **vor S0 den Client freistellen und je Phase
  `hc` und `update_info_cached` protokollieren, solange L7 nicht eingespielt ist**.
- **P1-Probe** — Python-Skript nach §5.6.
- **Messung AKS.4** — kalter `update_status`, danach Heap-Zeilen bis ≥ 130 s mit `tw=0`; `free`/`max` vorher/nachher.
- **`check-pwa`** (R2) — INFO `304` je Laden, Byte je Laden, je Lauf `ERR_CONNECTION_RESET` und `ERR_EMPTY_RESPONSE`.
- **`smoke-device.sh`** — ab R1 I-5-Felder, ab R2 `app_*`.
- **`watch-log.sh`** — ab R1 Heap-Zeile mit Zusatz und die I-6-Zeilen als Hinweis; mit V `- app etag`.
- **R2:** `tools/**` nach Prüfungen auf `HTTP/1.0` durchsuchen; c65b, c65w, LittleFS-Attrappe nachziehen.

---

## 10. RAM-Budget (AKZ.8)

| Teil | Release | statisch (Byte) |
|---|---|---|
| I-5 samt `http_accept_dead` | R1 | ~2 |
| Heap-Zeile | R1 | 0 |
| I-6 (Schrittwerte, `frag`, `http_accept_count`, Merker) | R1 | ~50 |
| L7 (a–c) | R1b bzw. R3 | 0 |
| `If-None-Match`-Puffer | R2 | 36 |
| Zähler `app_*` | R2 | 4 (T) / 6 (V) |
| `ETag`-Tabelle | R2 | 0 (T) / 104 (V) |
| K2 | R3 | ~4 |
| **Summe** | | **rund 100 (T) / 205 (V)** — Grenze 250 (Ent-11) |

---

## 11. Releaseschnitt, Einspielen, Rückweg

**R1** — ESP. Rückweg: ESP 3.2.37 aus `wordclock-release-2026-10-14-180012.zip` (L457).

**R1b** (Ent-18 A) — ESP. Rückweg auf R1 aus dessen ZIP; keine Paarungsfrage (die PWA ist nicht betroffen).

**R2** — ESP und PWA, Firmware zuerst (DIR-002). Alle Paarungen sicher. Ablauf: Wache; OTA; Smoketest samt
Update-Quelle; `install-app.sh --check`; `install-app.sh`; `--check`; `check-pwa.sh`. Rückweg: ESP auf den Vorstand,
PWA bleibt.

**R3** — ESP. Rückweg auf R2; mit Ent-18 B nimmt er L7 **und** den C72-Eingriff zurück.

Du spielst jedes Release ein; der Lead liefert die Zeilen samt Rückweg, **bevor** eingespielt wird.

---

## 12. Prüfpunkte (`firmware-analyst`, Review R1.7, Messungen L468)

| P | Frage | Stand |
|---|---|---|
| **P1** | FIN_WAIT_2 nach `tcp_close()`? | **◐ FA:** ja, rund 20 s, ohne RST — P1-Probe |
| **P2** | `MEMP_NUM_TCP_PCB` bei `MEMP_MEM_MALLOC`? | **✔ FA:** begrenzt nichts |
| **P3** | `tcp_abort()` ohne `callback_arg` | **◐ FA:** SYN_RCVD trägt den Listener; K2 nur `NULL` |
| **P4** | `tmr` | **◐ FA:** §3.1; dazu H4 (◐ R) |
| **P5** | `crc32()` | **✔ FA:** CRC-32/MPEG-2 |
| **P6** | Schreibstellen | **✔ FA:** 31; AutoFormat; Nebenbefunde |
| **P7** | `stop()`/`abort()` im Zweig „keine Anfrage“ | **✔ FA:** K1 nur bei `no_request_timeouts`; C74 |
| **P8** | Tote `ClientContext` | **◐ FA:** I-5 ergänzt |
| **P9** | C71: was bindet den Block? | **beantwortet (✔ L, A.5):** die ungelesene Antwort im `_rx_buf` des statischen httpclient (N5) |
| **P9b** | Platzierung eines frischen Kontexts unter L7 | **nicht mehr tragend** — mit L7 ist der Kontext nach jedem Abruf frei |
| **P10** | LittleFS-Puffer | **◐ FA:** L1, L3 verworfen |
| **P11** | 60 s nach Start | **✔ FA** |
| **P12** | Flash-Schreiber beim Start | **✔ FA** |

---

## 13. Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Legacy parallel? Wetter-Endpunkte? Restore-Bedingung? Richtige Schicht?

> **Legacy** bleibt parallel, unverändert und bei `HTTP/1.0`; C70 setzt nur in `http_send_fs_file()` an. **Wetter:** L7
> betrifft nur den httpclient (`httpclient.cpp`), nicht `openweather_client`; `/api/weather_get_now` und
> `/api/weather_get_forecast` bleiben unberührt; K2 lässt laufende ausgehende Abrufe in Ruhe. **Restore-Bedingung:** STM
> unberührt. **`.gz` und Grösse > 0:** bleibt. **Schicht:** L7 setzt dort an, wo die Ursache liegt — im Lebenszyklus
> des Clients (`httpclient_stop()`), zentral für alle Aufrufer, statt jede Prüfung einzeln zum Rumpflesen zu zwingen.
> C70 in der Auslieferung; C72 erst gemessen, dann im ESP.

**Scalable systems** — STM-Kommandos? UART? Watchdog? Hauptloop?

> **STM-Kommandos:** keine. **UART:** Heap-Zeile + rund 38 Byte je Minute; I-6-Zeilen je Durchgang; L7 fügt keine
> Zeile hinzu. **Hauptloop:** L7 kostet je Abruf eine Freigabe und beim nächsten eine Anforderung von rund 56 Byte —
> vernachlässigbar; Rumpflesen an jeder Prüfstelle hätte je Prüfung 1,2 KB Übertragung gekostet. T kostet je
> Asset-Antwort ein `seek`; jede `304` Millisekunden statt rund einer Sekunde; K2 einmal je Sekunde. **Watchdog:** keine
> Schleife ohne Grenze.

**Secure by design** — `innerHTML`? Fremddaten? Credentials?

> L7 ändert keine Ausgabe und keinen Endpunkt. Inhalte vom Update-Server bleiben nicht vertrauenswürdig; **L7 härtet**
> zusätzlich: Ein (etwa fremd konfigurierter) Update-Host, der grosse Rümpfe schickt, kann den Heap nach einer Prüfung
> nicht mehr dauerhaft binden. `If-None-Match` begrenzt, nur verglichen. K1/K2 senden RST nur an Verbindungen ohne
> Anfrage bzw. vom ESP geschlossene.

**Stable & reliable** — Fehler ausgewertet? Stille Verwerfungen? Zustand nach Abbruch? Flags?

> **Zustand nach Abbruch:** Heute hinterlässt jeder abgebrochene oder nicht zu Ende gelesene Abruf (Fehlerseite, leere
> Antwort, `zu-gross`, Existenzprüfung) eine gebundene Antwort bis zum nächsten Abruf; mit L7 ist der Zustand nach jedem
> Weg definiert: Client geschlossen, Kontext frei. **Veralteter Cache:** mit T durch Bau ausgeschlossen. **`HTTP/1.1`:**
> `Connection: close`, exaktes `Content-Length`. **Messung ohne Selbsttäuschung:** Die frühere Hypothese wurde am Gerät
> widerlegt und verworfen; die C71-Belegung wird aus den C72-Sitzungen herausgerechnet, sonst würde sie dort als C72
> gezählt; der offene Punkt L460 ist benannt; „erholt sich nicht“ erst nach TIME_WAIT (◐ H4, `tw=0`).

---

## 14. Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| L6, L0, L2, L4b, L1, L3, L4a, L5 | §7 |
| H-CC als Ursache; „Zwischenspeicher hält String“ | widerlegt (Start 3, M1) |
| Jede Existenzprüfung den Rumpf lesen lassen | dezentral; jeder künftige Aufrufer müsste es wissen; L7 deckt zentral ab |
| `HEAD` statt `GET` als Ersatz für L7 | deckt Fehlerwege, `len <= 0` und `zu-gross` nicht (§7) |
| L7 zusammen mit R2 (Ent-18 C) | vermischt die Wirkung von L7 und C70 in G10 |
| C72-Sitzung ohne Freistellen des Clients (vor L7) | die 1'704 Byte erschienen als C72-Lücke |
| `ETag` bei `HTTP/1.0`; `HTTP/1.1` für alle; `Last-Modified`; (b) jetzt | §4 |
| `fe` aus `umm_local.h`; `frag` als tragende Grösse | privater Kopf bzw. zu unempfindlich (A.4) |
| `device_ready` im Sekundentakt; HTTP-Lesungen in Startmessungen (ausser gekennzeichnet) | verändern das Gemessene |
| K1 bei M2, wenn nur `aborts` steigen; K3; K4 | §6 |

---

## 15. Versionsfolgen

- [ ] ESP `version.h` — R1 (eingespielt), **R1b** (Ent-18 A), R2, R3 (falls)
- [ ] `APP_VERSION` und `CACHE_NAME` — R2
- [ ] STM — nicht

Nur der `release-engineer` (R4). Die Nummern bestimmt der Lead.

---

## 16. Betroffene Module

| Datei | Änderung | Agent |
|---|---|---|
| `ESP8266/ESP-uclock/httpclient.cpp` | **L7a** `httpclient_stop()` gibt den Kontext frei; **L7b** Fehlerwege über `httpclient_stop()`; **L7c** `len == 0` schliesst — Kodierung und Zeilenende vorher feststellen | `esp-developer` |
| `ESP8266/ESP-uclock/http.cpp` (UTF-8, eine CRLF-Zeile) | R1: I-5, I-6; L7c an Aufrufern mit `len <= 0` (darunter `update_cache_write()`); R2: `If-None-Match`, `ETag`, `HTTP/1.1` in `http_send_fs_file()` und `304`, Zähler; R3: K1/K2/K5 | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` (gemischte Zeilenenden) | R1: `esp_heap_log()`, I-6 (e); R3: K2-Aufruf | `esp-developer` |
| `ESP8266/ESP-uclock/stm32flash.cpp`, `base.h` | R2 nur mit V; Prototypen | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | R2: `loadLayoutPreviewRowsMap()` | `pwa-developer` |
| `tools/**`, Prüfstände | §8, §9 | Lead |
| Versionsdateien | R1b, R2, R3 | `release-engineer` |
| `TESTPLAN-PWA.md`, `BEFUNDE.md` | Lesart nach A.5; Freistellen des Clients vor C72-Sitzungen (bis L7); Träger an C71, C72, C75 | `doc-writer` |
| `CHANGELOG.md` | je Release | Lead |

---

## 17. Nachträge — was sich ändert, was widerlegt ist

**A.2 (P1–P8):** `ruhe_s` berichtigt; `arg` 0/1/2; M1 über FIN_WAIT_1/CLOSING/LAST_ACK mit `nrtx`; M2 ohne eigene
TIME_WAIT-Fenster; M4 ohne SYN_RCVD; Kommentar am I-4-Block; Ent-2 offen (T); Bestand 31; P1-Probe,
`tcp_now_unclaimed`, `http_accept_dead`, `manuell`, AKC.8.

**L468, Trennmessung (15.10.2026):** K-a/N0 nicht bestätigt; Schreiben im laufenden Betrieb ohne Spur.

**A.3 (P7, P9, P11, P12):** K1 nur bei `no_request_timeouts`; AKE.3 gegen Grundlinie R2; H-CC als Hypothese, vierter
Start; L6 mit vier Risiken; L2 nicht umsetzbar; 130-s-Regel; I-6 um `nach`, `acc`, `tmp geloescht` ergänzt; C74.

**A.4 (Review R1.7):** `frag` statt `fe`; tragend `L = free − blk`; H2; H4 (130-s-Regel konservativ); C75 in L7;
`vorher` nach dem `.tmp`-Gang; `erste anfrage` beim ersten lebenden `accept()`.

**A.5 (16.10.2026, L468, `89ec34c`) — Ursache von C71 belegt:**

| Stelle (Stand vor A.5) | Neu |
|---|---|
| §5.5 N0, N1a, N1b, N2–N4 | **N5** „Belegung nach kaltem `update_status`“ trifft zu (M1, M3, Start 4); N0 nicht zutreffend; N1a/N1b widerlegt; N2–N4 gegenstandslos |
| H-CC als Ursache von C71 | **widerlegt**; Platzierung nur Nebenwirkung |
| Vermutung „Zwischenspeicher hält String“ | **widerlegt** |
| §7 L7 „Kontext freigeben, nach P9b“ | **L7 = L7a + L7b (Fehlerwege `:216`/`:224`) + L7c (C75)**; Begründung die belegte Belegung von 1'704 Byte; deckt `update_status`, `app.js.gz`-Prüfung und `zu-gross` zentral ab; `HEAD` als Option bewertet, nicht Ersatz |
| §7 L6, L0, L2, L4b | **verworfen** mit Beleg Start 3 und M1; nichts in `TESTPLAN-PWA.md`/c65n nachzuziehen |
| Ent-17 offen bis zum Tor | **faktisch entschieden: L7**, vorbehaltlich Deiner Freigabe |
| §8 c71e | Existenzprüfung mit ungelesenem Rumpf, Fehlerwege, `len == 0`, `zu-gross`; Sabotage „nur `stop()`“ |
| §5.3/§5.4 C72 | 1'704 Byte aus L461 gehören zu C71; Client vor jeder C72-Sitzung freistellen, `hc` und `update_info_cached` protokollieren; Lücke nach Abzug von C71; K1/K2 bleiben Kandidaten, bis G9.3–G9.5 entscheiden |
| §2 Releaseschnitt | **Ent-18 neu:** L7 als eigenes Release **R1b vor R2** (Empfehlung A) |
| P9, P9b | P9 beantwortet; P9b nicht mehr tragend |
| — | **offen (◐):** L460 sah den Fall ohne Anfrage; der Lead prüft den damaligen Mitschnitt |
