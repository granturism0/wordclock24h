# Design — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge mit den Entscheiden des Nutzers, den
Lead-Entscheiden, dem Prüfserver-Befund zu `HTTP/1.0`, **A.2** (P1–P8), der **Trennmessung** (L468), **A.3** (P7, P9,
P11, P12), **A.4** (Review R1.7, 16.10.2026), **A.5** (Ursache von C71 am Gerät belegt, L468, 16.10.2026, Commit
`89ec34c`) und **A.6** (C70 Variante (c), nach dem Tor G10.6/G10.7, 16.10.2026, Stand `2d97259`). **Status: freigegeben
am 15.10.2026 bis aufs Tor; Nachträge A.2 bis A.5 umgesetzt (R1, R1b, R2 eingespielt, L468); Entscheide Ent-19 bis
Ent-23 gefallen, A.6 freigegeben (16.10.2026).** Was sich ändert und was widerlegt ist: §17; der Entwurf zu (c): §18.
Entschieden seit A.5: Ent-2 (T), Ent-17 (L7), Ent-18 (A), Ent-8 (kein Eingriff; Ent-10, Ent-14 entfallen), G10.7 ((c)),
**Ent-19 C, Ent-20 B, Ent-21 E2, Ent-22 ja, Ent-23 1 Jahr**. Momentaufnahme (DIR-006); der lebende Stand steht in
`BEFUNDE.md`.

**Zeilenangaben:** Alle `Datei:Zeile`-Anker beziehen sich auf `72cfda7`, die in §18 auf `2d97259`. In `http.cpp` liegen
sie seit R1 ab rund `:2839` um rund +210 später (Lead). **Massgeblich ist die genannte Funktion bzw. das Symbol.**

**Kennzeichen** wie in `requirements.md`: **✔ SW** am Code `72cfda7` (in §18 `2d97259`) bzw. Core 3.1.2 nachgelesen;
**✔ B** in `BEFUNDE.md`; **✔ L** vom Lead gemessen; **✔ FA** vom `firmware-analyst` am Core belegt; **◐ FA** vom
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
| **A.6:** Auf R2 warm 626–835 Byte je Laden, aber 6–8 Anfragen, alle `304`; jede kostet eine TCP-Verbindung und ein `accept` | **✔ B** / **✔ SW** | L468 G10.1; AKC.7 (`Connection: close`) |
| **A.6:** Die Befunde am Code zu versionierten URLs | **✔ SW** | §18.2 |

---

## 2. Zuschnitt, Releases, Reihenfolge

| Release | Inhalt | Laufzeit | danach | Schreiber |
|---|---|---|---|---|
| **R1** | **I-5** (§3.1, §3.2) und **I-6** (§3.3) | ESP | **G9** ohne Cache | `esp-developer` |
| **R1b** *(nur mit Ent-18 A)* | **L7** samt Fehlerwegen und C75 (§7) — Behebung von C71 | ESP | **G9b** | `esp-developer` |
| **R2** | **C70** (§4): `ETag` nach Ent-2, `304`, `HTTP/1.1` für App-Assets; PWA-Teil | ESP **und** PWA | **G10** mit Cache, **Tor** | `esp-developer`, `pwa-developer` |
| **R2b** *(A.6)* | **C70 Variante (c)** (§18): `immutable` nur bei treffender Inhaltsmarke; Build schreibt die Verweise; PWA-Teil; `layout-previews.json` bleibt bei (a) | ESP **und** PWA **und** Build | **G10b**, danach T3 | `esp-developer`, `pwa-developer`, `ui-developer`, `release-engineer`, Lead (Werkzeug) |
| ~~**R3**~~ | ~~**Eingriff C72** (§6, Ent-8); mit Ent-18 B auch **L7**~~ — **entfallen** (Ent-8 = kein Eingriff, 16.10.2026) | — | — | — |

**Ent-18 (neu, Nutzer) — wann L7 kommt.** Der Schnitt aus Ent-9/Ent-16 legte die C71-Massnahme in R3, weil die Ursache
offen war. Jetzt ist sie belegt, und sie trifft **jeden Nutzer nach jedem Start**, sobald `update_status` einmal kalt
abgefragt wird — das tut die PWA von sich aus. *Entschieden am 16.10.2026: A (L468).*

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
erst nach dem Tor. **R2b (A.6):** ESP-Teil, Werkzeug, Markup und PWA-Teil dürfen parallel entstehen — vier Dateien,
vier Schreiber, verbunden nur durch den Vertrag in §18.7 (Name `wc-asset-tokens`, Format der Einträge, Parameter `h`).
Das `Makefile` folgt dem Werkzeug. Gebaut wird einmal, vom Lead, bei stillem Baum (R1, R2).

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
`max-age`. **(c)** beides — nur nach G10.7. **G10.7 am 16.10.2026: (c) entschieden** (L468) — Entwurf in §18.

### 4.2 Was beim zweiten Öffnen noch über die Leitung geht

N ≈ 7 `/app/`-Anfragen je Öffnen (◐). Kopf `200` rund 190 Byte, `304` rund 110 Byte (◐).

| Variante | Byte vom ESP beim zweiten Öffnen | Verbindungen |
|---|---|---|
| heute | ≈ 200 KB + N × 190 | N |
| **(a)** mit `HTTP/1.1` und PWA-Teil | ≈ N × 110 ≈ **0,8 KB**; Prüfserver: 290 statt 4'045 Byte (✔ L) | N, je sehr kurz |
| (b) | `index.html.gz` + nicht versionierte Referenzen | 2–4 |
| (c) | ≈ 0,3–0,5 KB | 1–4 |

*A.6:* Gemessen auf R2 (✔ B, G10.1) 626–835 Byte bei 6–8 Verbindungen. Soll für (c) nach §18 mit Ent-20 B: **zwei**
Verbindungen je warmem Laden (das Dokument und `layout-previews.json`).

### 4.3 Bewertung der Varianten (Grundlage von Ent-1)

| | (a) | (b) | (c) |
|---|---|---|---|
| **Korrekt nach Update** | **ja, durch Bau** | **nein:** `index.html` wird zuerst hochgeladen; dazwischen landet die alte `app.js` ein Jahr lang unter der neuen URL | wie (b) |
| **Versionsstellen** | keine neue | fünfte Stelle | wie (b) |
| **Dateien** | `http.cpp`, eine Zeile `app.js` | `index.html`, `sw.js`, `http.cpp`, `Makefile` | alle |
| **Legacy** | unberührt (`HTTP/1.0`) | unberührt | unberührt |

*A.6:* Beide Einwände gegen (b)/(c) gelten für eine Marke aus `APP_VERSION`. Mit der **Inhaltsmarke**, die der ESP gegen
das `ETag` prüft, ist „korrekt nach Update“ wieder **durch Bau** erfüllt (§18.3), und eine fünfte Versionsstelle entsteht
nicht, weil der Build die Marken erzeugt und `APP_VERSION` aus `app.js` liest (§18.7).

### 4.4 Umsetzung

#### `ETag`-Quelle (Ent-2, offen)

*Entschieden am 16.10.2026: T (L468).*

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

`loadLayoutPreviewRowsMap()`: `"no-store"` ⇒ `"no-cache"`. Firmware zuerst, dann PWA. *A.6:* bleibt in R2b so
(Ent-20 B, `layout-previews.json` unversioniert).

---

## 5. Messungen

### 5.1 Ladeprobe (`ladeprobe`, Lead; liegt vor)

Dauerhaftes Profil: kalt, dann fünfmal warm (dreimal `Page.reload` ohne `ignoreCache`, zweimal `Page.navigate`), je
vorher `heap_low_reset=1`, danach lesen. Je Laden: Leitungsstatus (`responseReceivedExtraInfo`), `If-None-Match`
gesendet, `encodedDataLength`, `heap_low_block`, `heap_low_path`, I-4, `app_*`. `--erwarte-304`. Keine Haltegrenze.
**Mit Ent-18 A** läuft auf R1b eine neue Grundlinie (G9b), damit G10 die Wirkung von C70 ohne die von L7 zeigt.
**Ab R2b** dazu `--erwarte-cache` (§18.8).

### 5.2 Probe U (AKW.3)

`layout-previews.json` um Leerraum ergänzt, `gzip -9`, über `app_file_upload`; danach Original zurück. Ohne
Einzelfreigabe (`CLAUDE.md:416`). **Ab R2b** die Abläufe U1a, U1b, U2, U3 (§18.8).

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
entfällt, und C72 wird mit diesem Ergebnis geschlossen oder umformuliert. **Ergebnis auf R1b und R2: M5 (✔ B, L468);
Ent-8 kein Eingriff.**

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

*Ent-8 am 16.10.2026: kein Eingriff — keiner der Kandidaten wird umgesetzt (L468). Der Abschnitt bleibt als Begründung.*

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
| **c70c** *(A.6)* | **R2b** | C70 (c), ESP | die Fälle aus AKL.1 und AKL.2: Treffer bei `200` und bei `304`; falsche, verschobene, Präfix-, gequotete, leere, fehlende Marke; Dokument (`/app`, `/app/`, `/app/index.html`) und `sw.js` mit eigener Marke; ohne gzip-Kopf, < 18 Byte, ausserhalb der Weissliste; zwei `.gz` gleicher Grösse; beide Zähler je Fall; ohne `h` Kopf und Rumpf gleich R2; `h` nie im Kopf | `immutable` ohne Vergleich; Vergleich über Länge bzw. Präfix; Dokument bzw. `sw.js` nicht ausgenommen; `304` bei Treffer mit `no-cache`; `immutable` im Zweig `HTTP/1.0`; `h` ausgegeben; Zähler fehlt in einem Zweig |
| **c70p** *(A.6 nachgezogen)* | **R2b** | C70 (c), PWA | Marke aus `wc-asset-tokens` nur nach Muster; Sprachdatei mit `?v=…&h=…` bzw. ohne Marke mit `?v=…`, `cache: "default"`; `layout-previews.json` unverändert ohne Query und mit `"no-cache"` (Ent-20 B); jede Query nur `name=wert` | Sprachdatei mit `"no-cache"` bzw. `"no-store"`; Marke ohne Musterprüfung; Parameter ohne `=`; `layout-previews.json` mit `h`; `layout-previews.json` nicht mehr `"no-cache"` |
| **c72e** | R3 — **entfallen** (Ent-8) | Eingriff C72 | K1 `abort()` nur im Teilfall Timeout; K2 nur `callback_arg == NULL` + Zustand + `ruhe_s` > Frist; SYN_RCVD, `ClientContext`, TIME_WAIT, LISTEN unberührt; ein Gang je Sekunde; Neubeginn nach Abbau; K5 Grenze 1 | K1 `stop()` bzw. auch im Teilfall Abbruch; K2 PCB mit `callback_arg` ≠ NULL bzw. SYN_RCVD abgebaut, Frist entfernt; K5 Grenze 2 |

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
- **R2b (A.6):** Werkzeug für die Inhaltsmarken, `install-app.sh --check` mit Markenprüfung, Smoketest-Block C70c,
  `ladeprobe --erwarte-cache`, `probe-u.sh` U1a/U1b/U2/U3, `check-pwa` INFO „aus dem Cache“ — §18.7, §18.8.

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
| K2 | R3 — entfallen | ~4 |
| Zähler `app_immutable`, `app_token_miss` *(A.6)* | R2b | 4 |
| **Summe** | | **rund 100 (T) / 205 (V)** — Grenze 250 (Ent-11) |

**Gemessen (✔ B, L468):** R1 +32, R1b 0, R2 +48 — **+80** nach R2. Mit R2b gerechnet **+84**; Grenze 250.

---

## 11. Releaseschnitt, Einspielen, Rückweg

**R1** — ESP. Rückweg: ESP 3.2.37 aus `wordclock-release-2026-10-14-180012.zip` (L457).

**R1b** (Ent-18 A) — ESP. Rückweg auf R1 aus dessen ZIP; keine Paarungsfrage (die PWA ist nicht betroffen).

**R2** — ESP und PWA, Firmware zuerst (DIR-002). Alle Paarungen sicher. Ablauf: Wache; OTA; Smoketest samt
Update-Quelle; `install-app.sh --check`; `install-app.sh`; `--check`; `check-pwa.sh`. Rückweg: ESP auf den Vorstand,
PWA bleibt.

**R2b** (A.6) — ESP und PWA, Firmware zuerst (DIR-002). Paarungen und Rückweg in §18.11.

**R3** — ESP. Rückweg auf R2; mit Ent-18 B nimmt er L7 **und** den C72-Eingriff zurück. — **entfallen** (Ent-8).

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

*Für A.6 (C70 Variante (c)) beantwortet §18.10 die vier Punkte eigens.*

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
| *A.6:* Versionsmarke nur aus `APP_VERSION` (Ent-19 A) | ESP kann sie nicht prüfen; falsche Langzeit-Einträge (§18.3) |
| *A.6:* nur Inhaltsmarke ohne `v` (Ent-19 B) | kein Rückweg für alle Browser zugleich; Ent-19 C entschieden |
| *A.6:* Versionsbestandteil im Pfad (Ent-19 D) | §18.5 |
| *A.6:* Platzhalter in `index.html` (Ent-21 E1) | Vorschau fragt mit Platzhaltern an, ein vergessener Verweis fällt nicht auf; E2 entschieden |
| *A.6:* `layout-previews.json` versionieren (Ent-20 A für diese Datei) | Ent-20 B entschieden: eine ersetzte Datei soll sofort ankommen |
| *A.6:* installierte App-Version auf dem ESP ablegen | nicht nötig; die Inhaltsmarke entscheidet allein, und eine abgelegte Version deckte Probe U nicht (§18.6) |
| *A.6:* Dokument zuletzt hochladen statt Markenprüfung | deckt Einzel-Uploads, die PWA-eigene Upload-Seite und `/fs` nicht; die Markenprüfung deckt alle Wege |
| *A.6:* `immutable` auch für das Dokument oder `sw.js` | nähme den einzigen Weg, über den eine Korrektur ankommt (§18.9) |

---

## 15. Versionsfolgen

- [ ] ESP `version.h` — R1 (eingespielt), **R1b** (Ent-18 A, eingespielt), R2 (eingespielt), **R2b** (A.6); R3
      entfallen
- [ ] `APP_VERSION` und `CACHE_NAME` — R2 (eingespielt), **R2b**
- [ ] STM — nicht

Nur der `release-engineer` (R4). Die Nummern bestimmt der Lead.

---

## 16. Betroffene Module

| Datei | Änderung | Agent |
|---|---|---|
| `ESP8266/ESP-uclock/httpclient.cpp` | **L7a** `httpclient_stop()` gibt den Kontext frei; **L7b** Fehlerwege über `httpclient_stop()`; **L7c** `len == 0` schliesst — Kodierung und Zeilenende vorher feststellen | `esp-developer` |
| `ESP8266/ESP-uclock/http.cpp` (UTF-8, eine CRLF-Zeile) | R1: I-5, I-6; L7c an Aufrufern mit `len <= 0` (darunter `update_cache_write()`); R2: `If-None-Match`, `ETag`, `HTTP/1.1` in `http_send_fs_file()` und `304`, Zähler; **R2b:** `immutable` nach §18.6 in `http_app()`/`http_send_fs_file()`, Zähler `app_immutable`, `app_token_miss`; R3: entfallen | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` (gemischte Zeilenenden) | R1: `esp_heap_log()`, I-6 (e); R3: entfallen | `esp-developer` |
| `ESP8266/ESP-uclock/stm32flash.cpp`, `base.h` | R2 nur mit V; Prototypen; R2b gegebenenfalls Prototyp | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | R2: `loadLayoutPreviewRowsMap()`; **R2b:** Marken aus `wc-asset-tokens`, `ensureLanguageTable()`; `loadLayoutPreviewRowsMap()` bleibt (Ent-20 B) | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/index.html` | **R2b:** ein leeres Meta-Element `wc-asset-tokens` (Ent-21 E2) | `ui-developer` |
| `Makefile` | **R2b:** `app-gz` — `index.html.gz` aus der umgeschriebenen Kopie | `release-engineer` |
| `tools/**`, Prüfstände | §8, §9; **R2b:** Werkzeug für die Inhaltsmarken, §18.8 | Lead |
| Versionsdateien | R1b, R2, **R2b** | `release-engineer` |
| `TESTPLAN-PWA.md`, `BEFUNDE.md` | Lesart nach A.5; Freistellen des Clients vor C72-Sitzungen (bis L7); Träger an C71, C72, C75; **R2b:** 5b (`app_immutable`, `app_token_miss`), Ladeprobe `--erwarte-cache`, Probe U1a/U1b/U2/U3, Rückweg für Browser, Träger an C70 | `doc-writer` |
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

**A.6 (16.10.2026, nach dem Tor G10.6/G10.7, Stand `2d97259`) — C70 Variante (c):**

| Stelle (Stand vor A.6) | Neu |
|---|---|
| Ent-2, Ent-17, Ent-18 offen | **entschieden** (L468): T, L7, A |
| Ent-8, Ent-10, Ent-14 offen bis zum Tor | **Ent-8 kein Eingriff** (M5); Ent-10, Ent-14 entfallen; **R3 und G11 entfallen**; §6 bleibt als Begründung |
| §4.1 (c) „nur nach G10.7“ | **G10.7: (c) entschieden**; Entwurf §18; Release **R2b**, Messung **G10b**, beide vor T3 |
| §4.3 Einwände gegen (b)/(c) | gelten für eine Marke aus `APP_VERSION`; mit der vom ESP geprüften **Inhaltsmarke** ausgeräumt (§18.3) |
| AKC.1 „`Cache-Control: no-cache` unverändert“ | gilt ab R2b nur ohne Treffer nach AKL.1 |
| AKZ.4 „keine Änderung an `index.html`“ | in R2b ein leeres Meta-Element (Ent-21 E2), sonst unverändert; der Build schreibt eine Kopie um |
| — | **neu:** AKL.1–AKL.9 (R2b), AKG.1–AKG.7 (G10b); Prüfstand c70c, c70p nachgezogen; RAM +4 |

**A.6, Entscheide vom 16.10.2026 eingearbeitet (Ent-19 bis Ent-23; A.6 freigegeben):**

| Stelle (vorgelegter Stand A.6) | Neu |
|---|---|
| Ent-19 offen, Empfehlung C | **C** — `?v=<APP_VERSION>&h=<marke>` (§18.5) |
| Ent-20 offen, Empfehlung A | **B** — `layout-previews.json` bleibt **unversioniert**: kein `h`, kein `immutable`, weiter `ETag`/`304`, `loadLayoutPreviewRowsMap()` bleibt bei `cache: "no-cache"`. Für versionierte Dateien gilt A sinngemäss (§18.4) |
| §4.6, AKC.6: in R2b `"no-cache"` ⇒ `"default"` für `layout-previews.json` | **entfällt** — bleibt `"no-cache"`; `"default"` nur für die Sprachdateien |
| Meta-Element mit `layout-previews.json` und `i18n/*` | nur noch **`i18n/*`**; das Werkzeug bricht ab, wenn `layout-previews.json` eine Marke bekäme (AKL.4) |
| Soll warm: **eine** Leitungsanfrage (Dokument) | **zwei:** Dokument und `layout-previews.json`, je `304` (AKG.2, AKG.6) |
| Probe U1 mit `layout-previews.json` als versionierter Datei | **U1a** `layout-previews.json` „sofort neu“ wie AKW.3; **U1b** `styles.css` als versionierte Datei, kein `immutable` unter der alten Marke (AKG.3) |
| Probe U2 mit `layout-previews.json` | mit **`styles.css`** (AKG.4) |
| Ent-21, Ent-22, Ent-23 offen | **E2**, **ja** (Prüfung in G10b bleibt), **1 Jahr** |
| Ziel 1b „einzeln ersetzte Datei mit dem nächsten Dokument“ | gilt nur noch für **versionierte** Dateien; für `layout-previews.json` Ziel 1 unverändert |

---

## 18. C70 Variante (c) — versionierte URLs mit Langzeit-Cache (Nachtrag A.6)

### 18.1 Ausgangslage und Entscheid

**Entscheid des Nutzers am Tor G10.7 (✔ B, L468):** C70 geht mit (c) weiter. Zum `ETag`/`304` aus R2 (Variante (a),
ESP 3.2.40, PWA 1.4.101) kommen versionierte URLs mit langem `max-age`, damit ein unverändertes Asset beim warmen Laden
**gar nicht mehr** über die Leitung nachgefragt wird. Ent-8 lautet „kein Eingriff“; R3 und G11 entfallen. Das neue
Release heisst **R2b** und kommt vor T3. Die Empfehlung des Leads war, bei (a) anzuhalten.

**Entscheide zum Entwurf (16.10.2026):** Ent-19 **C** (Nutzer), Ent-20 **B** (Nutzer), Ent-21 **E2**, Ent-22 **ja**,
Ent-23 **1 Jahr** (Lead). A.6 ist damit freigegeben.

**Messlage nach (a) (✔ B, G10.1):** warm 626–835 Byte je Laden, alles `304`, dazu 6–8 Anfragen. Jede Nachfrage kostet
eine TCP-Verbindung und ein `accept` auf dem ESP (`Connection: close`, AKC.7). Heap und Verbindungen sind der knappe
Punkt (C65) — das ist der Hebel von (c).

### 18.2 Befund am Code (✔ SW, Stand `2d97259`)

| Aussage | Status | Fundstelle |
|---|---|---|
| Der GET-Zweig trennt die Anfragezeile am ersten `?`: `sPath` ohne Query geht an `http()`, die Query als `sParam` | **✔ SW** | `http_server_loop()`, `http.cpp:17523-17545` |
| `http()` weist eine zu lange Query nur unter `/api/` ab (C46); für `/app/` kürzt es still auf 255 Zeichen und setzt die Parameter | **✔ SW** | `http()`, `http.cpp:16527-16537`; `http_api_query_too_long()` `:16428` |
| `http_app()` bildet `asset_path` = `app/<pfad>` aus dem Pfad **ohne** Query; `http_find_stored_app_asset_filename()` und `http_find_app_install_asset()` vergleichen exakt (`strcmp`) — die Query erreicht die Weissliste nie | **✔ SW** | `http_app()`, `http.cpp:3225-3263`; `http_find_app_install_asset()` `:2970-2988` |
| Ein Parameter ohne `=` bekommt `value = NULL`; `normalize_http_parameters()` prüft auf `NULL` (seit ESP 3.2.3) | **✔ SW** | `http_set_params()` `:3457-3504`, `normalize_http_parameters()` `:3427` |
| `http_get_param()` liefert nie `NULL` (C20); höchstens 16 Parameter | **✔ SW** | `:3509`; `MAX_HTTP_PARAMS` `:153` |
| In `http_app()` wird nur `action` gelesen; `h` und `v` kollidieren mit keinem Parameter im Pfad `/app/` | **✔ SW** | `http_app()` `:3299` |
| Das `ETag` nach T wird je Antwort in `http_app_etag()` aus derselben geöffneten Datei gerechnet, aus der danach der Rumpf kommt; ohne gzip-Kopf oder unter 18 Byte kein `ETag` | **✔ SW** | `http_app_etag()` `:2347-2373`; `http_send_fs_file()` `:2437-2457` |
| `200` und `304` senden heute beide fest `Cache-Control: no-cache`; `HTTP/1.1` nur bei `app_asset && gzip_encoded` | **✔ SW** | `http_send_fs_file()` `:2440`, `:2483-2509` |
| `index.html` verweist sechsmal auf Assets: Manifest, `icon-192.svg`, `icon-192.png`, `icon-180.png` (apple-touch), `styles.css`, `app.js` | **✔ SW** | `data/app/index.html:23-30`, `:908` |
| `styles.css` lädt nichts nach (kein `url(`, kein `@import`); das Manifest nennt vier Icons, die beim Laden der Seite nicht geholt werden (◐) | **✔ SW** / **◐** | `styles.css`; `manifest.webmanifest:15-40` |
| `app.js` holt zur Laufzeit `layout-previews.json` (`cache: "no-cache"`, bleibt so nach Ent-20 B) und `i18n/<code>.json?v=<APP_VERSION>` (`cache: "default"`); der Kommentar dort nennt das `=` schon als Pflicht | **✔ SW** | `app.js:1331`, `:11655`; `ensureLanguageTable()` `:1742-1745` |
| Ein `fetch` mit `cache: "no-cache"` fragt immer nach, auch bei frischem Eintrag | **◐** | Fetch-Standard; in G10b sichtbar |
| `sw.js` hält einen Vorab-Cache mit URLs **ohne** Query und vergleicht ohne `ignoreSearch`; über `http://` gibt es keinen Service Worker | **✔ SW** | `sw.js:13-26`, `:181-202`; §1 |
| `make app-gz` packt jede Quelle aus `GZIP_SOURCES` mit `gzip -9 -k -f` an Ort und Stelle, `index.html` eingeschlossen; die `.gz` sind nicht versioniert (`.gitignore`) | **✔ SW** | `Makefile:26`, `:108-113` |
| `deploy.sh` verlangt jede `.gz` vorhanden, nicht leer und **nicht älter** als ihre Quelle | **✔ SW** | `tools/deploy.sh:69-71` |
| `install-app.sh` lädt `index.html` **als erste** Datei hoch, dann die übrigen einzeln, mit einer Sekunde Abstand | **✔ SW** | `tools/install-app.sh:38-42`, `:116-125` |
| Der ESP kennt die installierte App-Version nicht | **✔ SW** | §1; `APP_VERSION_TXT` ist die Server-Version |
| `probe-u.sh` rechnet das erwartete `ETag` mit derselben Formel wie der ESP | **✔ SW** | `tools/probe-u.sh:25-27` |
| Chrome prüft beim Neuladen nur das Dokument nach und nimmt frische Unterressourcen aus dem Cache; Safari beachtet `immutable` | **◐** | Herstellerangaben; erst G10b belegt es für dieses Gerät |

### 18.3 Kernfrage: Wann ist ein Langzeit-Eintrag richtig? — Regel I-L

**Regel I-L:** Eine URL, die je mit `immutable` beantwortet wurde, bezeichnet für immer genau die Bytes, die dabei
geliefert wurden. Denn der Browser fragt sie bis zum Ablauf von `max-age` nicht mehr nach, und **ein ESP-Rückweg heilt
einen solchen Eintrag nicht** — der Browser fragt ja nicht.

**Warum eine Marke aus `APP_VERSION` das nicht leistet (✔ SW, §18.2):**

| Fall | Was geschähe mit `?v=<APP_VERSION>` und `immutable` |
|---|---|
| **Upload-Fenster:** `install-app.sh` lädt `index.html` zuerst | Ein Browser lädt zwischen den Uploads: Das neue Dokument nennt `app.js?v=NEU`, auf dem Gerät liegt noch die alte `app.js` ⇒ die **alte** Datei liegt ein Jahr lang unter der neuen URL. Genau der Einwand aus §4.3 |
| **Probe U / Einzel-Upload bei gleicher Version** | Eine ersetzte Datei kommt unter **derselben** URL. Browser mit Eintrag behalten die alte, Browser ohne Eintrag frieren die neue ein — und eine spätere Korrektur bei gleicher Version erreicht keinen von beiden |
| **Upload in der PWA oder unter `/fs`** | wie Probe U |

**Daraus folgt der Entwurf:** Die URL trägt eine **Inhaltsmarke** `h` — den `ETag`-Wert nach Variante T ohne
Anführungszeichen, also Grösse der `.gz` und ihr gzip-Trailer (CRC-32 und Länge des entpackten Inhalts) —, und **der ESP
sendet `immutable` nur, wenn `h` gleich dem `ETag` ist, das er für genau diese Antwort aus genau dieser Datei gerechnet
hat.** Dann gilt I-L durch Bau: Unter `h=X` kann mit `immutable` nur ein Inhalt mit `ETag` X hinausgehen. Die
Reihenfolge des Uploads, die Version und der Weg des Uploads spielen keine Rolle mehr.

**Was bleibt (◐):** (1) Zwei verschiedene Inhalte mit gleicher `.gz`-Grösse, gleicher entpackter Länge **und** gleichem
CRC-32 — Wahrscheinlichkeit rund 2⁻³² je Änderung. (2) Eine Änderung nur am Kopf, etwa am `Content-Type` aus
`http_content_type()` (Hinweis H4 aus Review R2.7): Der Rumpf und damit die Marke bleiben gleich, ein Browser behält den
alten Kopf. (3) Eine Quelle, die beim Packen halb geschrieben war (R2): Ihre `.gz` und ihre Marke passen zueinander, die
kaputte Datei würde also unter ihrer Marke festgehalten. Alle drei fängt der Rückweg über das Dokument ab — (3) schon
mit dem nächsten richtigen Stand, (1) und (2) über die Version (§18.9, Ent-19 C).

### 18.4 Welche URLs (Punkt 1)

| URL | versioniert | Begründung |
|---|---|---|
| `/app`, `/app/`, `/app/index.html` | **nie** — `no-cache` mit `ETag`, auch wenn eine Marke mitkommt | Über das Dokument kommt jede neue Marke und jede Korrektur an. Der ESP schliesst es **selbst** aus, nicht nur das Werkzeug (AKL.1 (iv)) |
| `/app/sw.js` | **nie** — Registrierungs-URL bleibt `/app/sw.js` | Ein Service Worker darf nie lange gecacht werden; eine neue URL legte eine zweite Registrierung an. Der ESP schliesst ihn aus |
| `styles.css`, `app.js`, `icons/icon-192.svg`, `icons/icon-192.png`, `icons/icon-180.png` | **ja** — Verweise im Dokument | beim Laden geholt (G10.1) |
| `manifest.webmanifest` | **ja** (Ent-22) | Chrome holt es beim Neuladen (G9.1); ob es versioniert geladen wird, prüft G10b (AKG.6) |
| jede Datei unter `i18n/` | **ja** — Laufzeitabruf in `ensureLanguageTable()`, Marke aus dem Meta-Element | beim Sprachwechsel bzw. Laden in Englisch geholt |
| `layout-previews.json` | **nein** (Ent-20 B) — kein `h`, kein `immutable`, weiter `ETag`/`304`; `loadLayoutPreviewRowsMap()` bleibt bei `cache: "no-cache"` und ohne Query; das Werkzeug nimmt sie nicht ins Meta-Element | eine ersetzte Datei kommt beim nächsten Laden sofort an (Probe U1a wie AKW.3); Preis: je Laden eine `304` |
| Icons **im Manifest** (`icon-192.png`, `icon-512.png`, `icon-mask.png`, `icon-512.svg`) | nein — bleiben bei (a) | beim Laden der Seite nicht geholt (◐). `icon-192.png` steht dann zweimal im Cache des Browsers, mit und ohne Marke — harmlos |
| Legacy, `/api/*`, Installations- und Hinweisseiten, alles ausserhalb der Weissliste | nein, unverändert | AKL.3 |

**Zusage für eine einzeln ersetzte Datei (Ent-20):** Für `layout-previews.json` gilt Ziel 1 unverändert — sofort. Für
eine **versionierte** Datei gilt sinngemäss Ent-20 A: Sie ist wirksam, sobald ein Dokument sie mit ihrer neuen Marke
nennt; ein Browser mit Cache-Eintrag zeigt bis dahin die alte, ein Browser ohne Eintrag sofort die neue. Ein ganzer Stand
über `install-app.sh` oder die Auto-Installation ist beim nächsten Laden wirksam.

### 18.5 Form der Marke (Punkt 2) — Ent-19 (entschieden: C)

| | **(A)** `?v=<APP_VERSION>` | **(B)** `?h=<marke>` | **(C)** `?v=<APP_VERSION>&h=<marke>` | **(D)** Pfad `/app/<marke>/app.js` |
|---|---|---|---|---|
| ESP prüft den Inhalt | **nein** | ja | ja | ja |
| Regel I-L | **verletzt** (§18.3) | gilt | gilt | gilt |
| Unveränderte Datei über einen PWA-Release hinweg | neu geholt | **bleibt im Cache** | neu geholt (≈ 200 KB je Release und Browser) | bleibt |
| Rückweg für einen falschen Eintrag | — | nur über eine Inhaltsänderung genau dieser Datei | **jede neue `APP_VERSION`** | wie B |
| Aufwand ESP | — | ein Vergleich | ein Vergleich | Pfad zerlegen; berührt Abflachung (`app_asset_filename()`), Weissliste, Upload, Installation |
| Bewertung | verworfen | möglich | **entschieden (Nutzer, 16.10.2026)** | verworfen |

**Entschieden C:** `h` trägt die Korrektheit, `v` den Rückweg. Die Version kommt aus `const APP_VERSION` in `app.js` —
der einzigen Quelle; `CACHE_NAME` gehört dazu und steigt wie bisher mit ihr (R4).

**Invarianten der Query:**

- **Jeder Parameter mit `=`.** Ein Parameter ohne `=` hat den ESP früher abgestürzt; das ist seit 3.2.3 behoben, steht
  aber in der Gefahrenliste (`CLAUDE.md`). Das Werkzeug (AKL.4) und `app.js` (AKL.6) erzeugen nur `name=wert`.
- **Zeichen** nur `[0-9a-z.=&-]` — nichts, was `normalize_http_parameters()` umdeutet (`%`, `+`).
- **Länge:** `v=` + Version (z. B. 7) + `&h=` + Marke (höchstens 10 + 1 + 16 = 27) ≈ 40 Zeichen; Grenze im Werkzeug
  **64**, weit unter 255 (C46; für `/app/` würde `http()` still kürzen, §18.2).
- **Nur `.gz`, Grösse > 0:** unverändert — `immutable` gibt es nur in dem Zweig, der das schon verlangt (§18.6).
- Die Query wird vor `http_app()` abgetrennt, die Weissliste vergleicht weiter exakt (✔ SW) — an beidem ändert R2b
  nichts.

### 18.6 ESP (Punkt 3)

**Entscheidungstabelle** für eine Antwort auf ein App-Asset (`http_app()` ⇒ `http_send_fs_file()`):

| Fall | `Cache-Control` | Zähler |
|---|---|---|
| Zweig `HTTP/1.0` (ausserhalb der Weissliste, keine `.gz`) | wie heute, `h` ohne Wirkung | — |
| Zweig `HTTP/1.1` ohne `ETag` (keine gzip-Datei, < 18 Byte) | `no-cache` | `app_token_miss`, wenn `h` gesetzt |
| `h` fehlt oder leer — darunter jede Anfrage auf `layout-previews.json` (Ent-20 B) | `no-cache` (wie R2) | — |
| `h` gesetzt, ≠ `ETag` | `no-cache` | `app_token_miss` |
| `h` = `ETag`, aber Dokument (`/app`, `/app/`, `app/index.html`) oder `app/sw.js` | `no-cache` | `app_token_miss` |
| `h` = `ETag`, alles andere | **`public, max-age=31536000, immutable`** (Ent-23: 1 Jahr) | `app_immutable` |

- **`200` und `304` gleich.** Eine `304` aktualisiert die gespeicherten Kopfzeilen im Browser (◐ RFC 9111 §4.3.4); eine
  `304` mit `no-cache` nach einem Treffer würde den Eintrag zurückstufen, eine mit `immutable` ohne Treffer ihn falsch
  hochstufen. Darum entscheidet dieselbe Tabelle.
- **Vergleich ohne Heap:** `strlen(h) + 2 == strlen(etag)`, `etag[0] == '"'`, `memcmp(etag + 1, h, strlen(h)) == 0`;
  `h` über `http_get_param("h")` (nie `NULL`). Der Wert von `h` wird **nie** ausgegeben — der ESP sendet sein eigenes
  `ETag`, keine Spiegelung der Anfrage.
- **`layout-previews.json`** braucht im ESP keine Sonderregel: Sie wird nie mit `h` angefragt (Werkzeug und `app.js`
  geben ihr keine Marke, AKL.4, AKL.6). Käme doch eine treffende Marke, bekäme sie `immutable` unter dieser Marke — nach
  I-L richtig, und das Dokument nennt diese URL nicht.
- **Abweichende oder fehlende Version:** `v` wertet der ESP **nicht** aus — er kennt die installierte Version nicht und
  braucht sie nicht. Eine abweichende `v` mit treffender Marke bekommt `immutable` (der Inhalt ist bestätigt; die URL ist
  nur eine andere). Eine fehlende `v` ebenso.
- **Wechsel der App-Version:** Das neue Dokument nennt neue URLs (neue `v`, neue Marken geänderter Dateien). Der Browser
  hat sie nicht ⇒ Abruf ⇒ Treffer ⇒ `immutable`. Alte Einträge liegen ungenannt im Cache, bis der Browser sie verdrängt.
- **Probe U (Datei ersetzt, Version gleich):** Für eine versionierte Datei nennt das Dokument die alte Marke, die Datei
  hat ein neues `ETag` ⇒ kein Treffer ⇒ `no-cache`. Der neue Inhalt wird **nie** unter der alten Marke festgeschrieben
  (AKG.3 U1b). Für `layout-previews.json` gilt (a) unverändert (U1a).
- **Upload-Fenster (Dokument zuerst):** Das neue Dokument nennt die neue Marke, die Datei ist noch alt ⇒ kein Treffer ⇒
  `no-cache` mit dem alten `ETag`. Beim nächsten Laden fragt der Browser nach; nach dem Upload trifft die Marke, es kommt
  `200` mit `immutable` (AKG.4).
- **Das `ETag` bleibt Rückfall:** jede Antwort trägt es wie in R2; ohne Marke verhält sich R2b genau wie R2 (AKL.3).
- **Kosten:** ein Vergleich von höchstens 29 Zeichen je Antwort, kein zusätzliches Lesen — das `ETag` ist schon
  gerechnet. 4 Byte RAM für die Zähler. Keine Logzeile je Anfrage (UART).

### 18.7 Build, Markup und PWA (Punkt 4) — Ent-21 (entschieden: E2)

**Wer was schreibt (R3):**

| Teil | Datei | Agent |
|---|---|---|
| Entscheidung `immutable`, Zähler | `http.cpp` | `esp-developer` |
| Werkzeug: Marken rechnen, Kopie umschreiben, Meta-Element füllen, prüfen | `tools/` (Name bestimmt der Lead) | Lead |
| Aufruf im Build | `Makefile`, Ziel `app-gz` | `release-engineer` |
| leeres Meta-Element | `data/app/index.html` | `ui-developer` |
| Marken lesen, Abruf der Sprachdateien | `data/app/app.js` | `pwa-developer` |
| `CACHE_NAME`, `APP_VERSION`, ESP-Version | Versionsdateien | `release-engineer` (R4) |

**Ablauf in `make app-gz` (E2):**

1. Alle Quellen aus `GZIP_SOURCES` **ausser** `index.html` packen wie heute.
2. Das Werkzeug liest `APP_VERSION` aus `app.js` (dieselbe Zeile wie `app-version-file`), rechnet für jedes
   versionierte Asset die Inhaltsmarke aus seiner eben erzeugten `.gz` (Formel von `http_app_etag()`), schreibt eine
   **Kopie** von `index.html`, in der die sechs bekannten Verweise `?v=<APP_VERSION>&h=<marke>` tragen und das
   Meta-Element mit den Marken der Sprachdateien gefüllt ist, und prüft alle Abbruchgründe aus AKL.4.
3. Erst bei Exit 0 entsteht `index.html.gz` aus der Kopie (`gzip -9 -n`), über eine Zwischendatei, die erst nach
   Erfolg umbenannt wird — **nie** eine leere oder halbe `.gz`. Die Kopie wird entfernt.

**Vertrag zwischen Werkzeug und `app.js`:** `<meta name="wc-asset-tokens" content="<pfad>=<marke>;<pfad>=<marke>…">`,
`<pfad>` relativ zu `/app/`, **nur** Dateien unter `i18n/` (heute `i18n/en.json`); `layout-previews.json` steht nicht
darin (Ent-20 B). `app.js` liest es einmal, nimmt nur Einträge, deren Marke `^[0-9]+-[0-9a-f]{16}$` entspricht, und
baut für die Sprachdatei `?v=<APP_VERSION>&h=<marke>`; ohne gültigen Eintrag `?v=<APP_VERSION>` — die heutige Form, die
der ESP mit `no-cache` beantwortet. Die Sprachdatei weiter mit `cache: "default"`. `loadLayoutPreviewRowsMap()` bleibt
unverändert: `/app/layout-previews.json` ohne Query, `cache: "no-cache"`.

**Was dabei sicher bleibt:** Die Quelle `index.html` hat ein leeres Meta-Element und Verweise ohne Query; die
PWA-Vorschau lädt sie unverändert. Fehlt im ausgelieferten Dokument eine Marke — etwa weil jemand die Quelle ungepackt
hochlädt —, gilt Verhalten (a), nie ein falscher Eintrag. `install-app.sh --check` meldet das beim Namen (AKL.8 (i)).

**E1 (verworfen):** Platzhalter in den sechs Verweisen der Quelle, vom `ui-developer` gepflegt. Die Vorschau fragte mit
Platzhaltern an; jeder neue Verweis bräuchte einen Platzhalter, und einen vergessenen sähe man nicht. E2 zählt statt
dessen die Verweise (genau 6) und bricht bei einer Abweichung ab.

**Single Source of Truth:** `APP_VERSION` in `app.js`. Im Dokument steht sie nur abgeleitet, im gepackten Fabrikat,
nicht in einer Quelle — eine **fünfte Versionsstelle** entsteht nicht, S4 bleibt bei vier. `CACHE_NAME` steigt wie
bisher mit `APP_VERSION`.

**R2 (Arbeitsverzeichnis still) gilt erst recht:** `app-gz` erzeugt jetzt eine Datei aus mehreren Quellen. Ist eine
Quelle beim Packen halb geschrieben, passen ihre `.gz` und ihre Marke **zueinander** — der ESP erkennt das nicht, und
ein Browser hielte die kaputte Datei unter dieser Marke fest. Geheilt wird es erst mit dem nächsten richtig gepackten
Stand, dessen Dokument eine andere Marke nennt (§18.9). Bis dahin wäre es derselbe Weisschirm wie heute, nur in den
Browsern, die in diesem Fenster geladen haben, auch nach einem erneuten Hochladen derselben Fassung. Vor `app-gz` sind
deshalb alle Editier-Tasks abgeschlossen und `git status` geprüft — wie bisher.

**`deploy.sh`:** Die Prüfung „`.gz` nicht älter als die Quelle“ hält, weil `index.html.gz` nach der Quelle entsteht
(◐, R2b.8 prüft es).

### 18.8 Messung G10b (Punkt 5)

**Instrumente, zwei unabhängige (L235):** (1) CDP in `ladeprobe`: **Leitung** = Antwort mit
`responseReceivedExtraInfo` (Leitungsstatus), **Cache** = ohne, von CDP als aus dem Speicher- bzw. Plattencache bedient
gemeldet. (2) Die Gerätezähler `app_full`, `app_not_modified`, `app_immutable`, `app_token_miss` je Fenster. Beide
müssen dasselbe zählen: Leitungsanfragen an `/app/` = Δ`app_full` + Δ`app_not_modified`.

**Neues Soll für das warme Laden (Chrome, AKG.2):** über die Leitung **nur das Dokument und `layout-previews.json`**,
beide mit `304`; **null** Anfragen an versionierte Assets; Δ`app_not_modified` = Zahl dieser Leitungsanfragen (erwartet
2), Δ`app_full` = 0.

**Probe U — vier Abläufe (`probe-u.sh`, Lead):**

| Ablauf | Schritt | Erwartet |
|---|---|---|
| **U1a** unversioniert: `layout-previews.json` ersetzt (AKG.3, wie AKW.3) | vor #2 ersetzen | #2: genau diese Datei `200` mit neuem `ETag`; Dokument `304`; alle versionierten Dateien aus dem Cache |
| | — | #3: `layout-previews.json` `304` mit neuem `ETag` |
| | vor #4 Original zurück | #4: `200` mit ursprünglichem `ETag` |
| **U1b** versioniert: `styles.css` ersetzt, Dokument gleich (AKG.3) | ersetzen | direkter Abruf mit **alter** Marke: `200`, neues `ETag`, **`no-cache`**, Δ`app_token_miss` +1; frisches Profil: neuer Inhalt; warmer Browser: weiter aus dem Cache |
| | Original zurück | direkter Abruf: `immutable`, ursprüngliches `ETag` |
| **U2** `styles.css` mit neuer Marke, Dokument zuerst (AKG.4) | vor #2 Dokument mit neuer Marke | #2: Dokument `200`; `styles.css` mit neuer Marke `200`, **alter** Inhalt, `no-cache` (Fenster) |
| | vor #3 die Datei | #3: Dokument `304`; `styles.css` `200`, neues `ETag`, `immutable` |
| | — | #4: alles ausser Dokument und `layout-previews.json` aus dem Cache |
| | vor #5 beide Originale | #5: Dokument `200`; `styles.css` mit ursprünglicher Marke **aus dem Cache** |
| **U3** Versionswechsel als Rückweg (AKG.5) | Dokument mit anderer `v` | nächstes Laden: **jede** versionierte URL `200` mit `immutable`; folgendes: alles Versionierte aus dem Cache; Original zurück ⇒ ursprüngliche URLs aus dem Cache |

`styles.css` eignet sich als versionierte Prüfdatei, weil sie bei jedem Laden geholt wird und ein angehängter
Zeilenumbruch gültiges CSS bleibt. Die Dokumente für U2 und U3 erzeugt das Werkzeug aus R2b.3 (gleicher Code wie im
Build, andere Eingaben). Nach jedem Ablauf `install-app.sh --check` ohne Abweichung.

**Safari und iPhone (AKG.6):** wie G10.3 bzw. G9b.4, mit Vorwärmen; zusätzlich je ein Neuladen in S1. Kein CDP — nur
die Gerätezähler. Gemessen und berichtet; erwartet Δ`app_not_modified` ≤ 2; eine Nachfrage an versionierte Dateien
trotz `immutable` ist ein Befund. Ob das versionierte Manifest geladen wird, wird vermerkt (Ent-22).

**Gegenproben und Sabotagen (DIR-014), jede mit angekommener Meldung:**

| Instrument | Gegenprobe ⇒ erwartet |
|---|---|
| `ladeprobe --erwarte-cache` | gegen R2 (kein `immutable`) ⇒ Exit ≠ 0, „warm: Cache erwartet, 304 erhalten“; Prüfserver mit `immutable` ⇒ OK |
| `probe-u.sh` U1a–U3 | ohne Upload ⇒ FEHL je Ablauf; gegen einen Prüfserver, der `immutable` auch bei falscher Marke sendet ⇒ U1b FEHL |
| `install-app.sh --check` | Dokument mit verfälschter Marke (Attrappe) ⇒ FEHL mit Dateiname |
| `smoke-device.sh` C70c | gegen 3.2.40 bzw. `SMOKE_DR_OVERRIDE` ⇒ FEHLT |
| Werkzeug Inhaltsmarken | je Abbruchgrund aus AKL.4 ⇒ Exit ≠ 0 mit Meldung, keine neue `index.html.gz` |
| c70c, c70p | Sabotagen aus §8 ⇒ je FEHL; gegen R2 bzw. 1.4.101 ⇒ FEHL |

### 18.9 Risiken und Rückweg (Punkt 6)

| Risiko | Ausschluss | Wenn es doch eintritt |
|---|---|---|
| **`immutable` auf einem Inhalt, den die Marke nicht bezeichnet** (Upload-Fenster, Probe U, Einzel-Upload) | **durch Bau:** `immutable` nur bei `h` = `ETag` dieser Antwort (§18.6); c70c, AKG.3, AKG.4 | — |
| **Dokument oder `sw.js` mit `immutable`** — nähme den einzigen Weg, über den eine Korrektur ankommt | der ESP schliesst beide **selbst** aus, unabhängig von `h` und vom Werkzeug; c70c-Sabotage; Smoketest C70c | ESP-Korrektur; ein so gecachtes Dokument liefe bis `max-age` — deshalb doppelt gesichert |
| Quelle beim Packen halb geschrieben (R2) — Marke und kaputte `.gz` passen zueinander | R2: stiller Baum vor `app-gz`, wie bisher | mit dem nächsten richtig gepackten Stand geheilt; dazwischen in den betroffenen Browsern fest (§18.7) |
| Nur der Kopf ändert sich (H4, etwa `Content-Type`) | in R2b ändert sich kein Kopf eines App-Assets (AKL.3); **Regel für später:** jede Änderung am Kopf eines App-Assets kommt mit einer neuen `APP_VERSION` | Rückweg Version (C) |
| CRC-32-Kollision bei gleichen Grössen | ◐ rund 2⁻³² je Änderung | Rückweg Version (C) |
| Marke aus einer anderen `.gz` als der hochgeladenen | kein Treffer ⇒ `no-cache` (sichere Richtung); sichtbar als `app_token_miss` und in `install-app.sh --check` | neu bauen und hochladen |
| Meta-Element oder Verweis ohne Marke | Verhalten (a), sichere Richtung; Zählprüfung im Werkzeug, `--check` | — |
| `layout-previews.json` bekommt versehentlich eine Marke | Werkzeug bricht ab (AKL.4); c70p-Sabotage (AKL.6) | nach I-L trotzdem kein falscher Eintrag, nur die Zusage aus Ent-20 B wäre verletzt |
| Manifest-URL ändert sich mit jeder Version (Ent-22 ja) | über `http://` in Chrome nicht installierbar (◐); Kennung über `id`/`start_url` (●) — Prüfung in G10b bleibt | Manifest wieder unversioniert in einem PWA-Release |
| Service Worker in einem sicheren Kontext: Vorab-Cache trifft versionierte URLs nicht | ◐ harmlos — der Abruf geht ins Netz bzw. in den HTTP-Cache; über `http://` gegenstandslos | — |
| Verdrängung aus dem Browser-Cache | harmlos: erneuter Abruf, Treffer, `immutable` | — |
| RAM | +4 Byte; Paket +84 von 250 | — |

**Rückwege, und was sie können:**

| Weg | Wirkung auf neue Antworten | Wirkung auf **vorhandene** Einträge im Browser |
|---|---|---|
| **PWA-Release mit neuer `APP_VERSION`** (Ent-19 C) | neue URLs für **alle** versionierten Assets | **umgeht jeden** Eintrag — das Dokument ist `no-cache` und nennt nur noch neue URLs. **Der Rückweg, der immer bleibt** |
| Inhaltsänderung genau der betroffenen Datei | neue Marke für diese Datei | umgeht den Eintrag dieser Datei |
| ESP zurück auf R2 (3.2.40) | kein neues `immutable` mehr | **keine** — vorhandene Einträge bleiben bis `max-age`. Das gehört in die Einspielzeilen (AKZ.5) |
| Im Browser: Website-Daten löschen; am Desktop hartes Neuladen | — | löscht bzw. ersetzt die Einträge dieses einen Browsers |

### 18.10 Prüfung gegen die Architektur-Checkliste (A.6)

**Proper architecture** — Legacy parallel? Wetter-Endpunkte? Restore-Bedingung? Nur `.gz`? Richtige Schicht?

> **Legacy** bleibt parallel und unverändert: `HTTP/1.0`, ohne `ETag`, ohne `immutable`; `h` wirkt nur im Zweig
> `HTTP/1.1` der App-Assets (AKL.3). **Wetter:** `/api/weather_get_now` und `/api/weather_get_forecast` unberührt, kein
> Rückbau. **Restore-Bedingung:** STM unberührt. **Nur `.gz`, Grösse > 0:** `immutable` gibt es ausschliesslich in dem
> Zweig, der Weissliste, `.gz`, Grösse > 0 und gzip-Kopf schon verlangt; der Build hinterlässt nie eine leere
> `index.html.gz` (AKL.4). **Schicht:** Die Kosten (Verbindung und `accept` je Nachfrage, C65) entstehen am ESP, lassen
> sich aber nur im Cache des Browsers vermeiden. Deshalb entscheidet der ESP, **ob** ein Eintrag lange gelten darf
> (Markenprüfung), der Build erzeugt die Marken, und die PWA baut nur URLs. `app.js` umgeht kein ESP-Problem.

**Scalable systems** — STM-Kommandos? UART? Watchdog? Polling? Hauptloop? Hartkodierte Grenzen?

> **STM-Kommandos:** keine. **UART:** keine zusätzliche Zeile; `http_log_request()` protokolliert ohne Query. **Watchdog,
> Hauptloop:** je Antwort ein Vergleich von höchstens 29 Zeichen, keine Schleife; ein warmes Laden sinkt von 6–8 auf
> **zwei** `accept` (Dokument, `layout-previews.json`) — weniger PCBs, `ClientContext` und TIME_WAIT, also weniger Last
> als heute. **Polling:** unverändert, `/api/*` ist nicht betroffen. **Grenzen:** Die Zahl der Verweise (6) prüft das
> Werkzeug — ein neuer Verweis lässt den Build scheitern, statt still unversioniert zu bleiben. Eine neue Sprache kommt
> über das Verzeichnis `i18n/` mit, wie im `Makefile` (`wildcard`). Zwei Parameter von 16; Query ≤ 64 von 255.

**Secure by design** — `innerHTML`? Fremddaten? Credentials? Neue schreibende Endpunkte?

> Kein `innerHTML`; das Meta-Element wird nur gelesen und jede Marke gegen ein festes Muster geprüft. `h` ist Fremdeingabe
> aus dem LAN: nur verglichen, nie gespeichert, nie ausgegeben (keine Kopfspiegelung, AKL.1). Keine Credentials, kein
> neuer Endpunkt. **`immutable` verlängert einen Angriff nicht:** Wer über das ungeschützte `app_file_upload` (C61) eine
> fremde Datei hochlädt, bekommt ohne passendes Dokument kein `immutable`; mit eigenem Dokument wird seine Datei zwar
> eingefroren, aber nur unter **ihrer** Marke — sobald das richtige Dokument zurückliegt, nennt es sie nicht mehr. Das
> Dokument selbst ist nie `immutable`.

**Stable & reliable** — Fehler ausgewertet? Leere `catch`? Stille Verwerfungen? Zustand nach Abbruch? Flags?

> **Build:** Jeder Abbruchgrund endet mit Exit ≠ 0 und Meldung; es gibt keinen stillen Rückfall auf ein Dokument ohne
> Marken (AKL.4). **Laufzeit:** Der Rückfall auf (a) ist gewollt still, aber **gezählt** (`app_token_miss`) und von
> `install-app.sh --check` beim Namen gemeldet. **`app.js`:** Ein ungültiger Meta-Eintrag führt zur URL ohne `h`, nicht in
> einen leeren `catch`; c70p prüft beides. **Zustand nach Abbruch:** Ein abgebrochener Upload hinterlässt das
> Upload-Fenster — durch die Markenprüfung sicher (AKG.4). **Flags:** keine. **Rückfall:** Das `ETag` bleibt in jeder
> Antwort.

### 18.11 Releases, Einspielen, Rückweg R2b

**R2b** — ESP und PWA, **Firmware zuerst** (DIR-002). Ablauf: Wache; OTA; Smoketest samt Update-Quelle und Block C70c;
`install-app.sh --check`; `install-app.sh`; `--check` samt Markenprüfung; `check-pwa.sh`.

**Paarungen, alle sicher (✔ SW):**

| ESP | PWA | Verhalten |
|---|---|---|
| R2b | R2b | (c); `layout-previews.json` bei (a) |
| R2b | 1.4.101 | keine Marke in den URLs ⇒ `no-cache` ⇒ wie R2 |
| 3.2.40 | R2b | `h` und `v` ohne Wirkung (nur `action` wird gelesen) ⇒ `no-cache` ⇒ wie R2 |

**Rückweg:** ESP auf R2 aus dessen ZIP; PWA auf 1.4.101 mit `install-app.sh` vom Tag von R2. **Die Einspielzeilen sagen
ausdrücklich:** Der ESP-Rückweg stoppt neue `immutable`, heilt aber keine Einträge, die Browser schon haben; die sind nach
I-L richtig. Für einen Eintrag, der dennoch falsch wäre, ist der Rückweg ein PWA-Release mit neuer `APP_VERSION`
(§18.9).

**Versionen (R4):** ESP-Version, `APP_VERSION` und `CACHE_NAME` steigen; die Nummern bestimmt der Lead.
