# Design — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`); Nachträge mit den Entscheiden des Nutzers, den
Lead-Entscheiden, dem Prüfserver-Befund des Leads zu `HTTP/1.0`, **Nachtrag A.2** (P1–P8), der **Trennmessung C71**
(L468), **Nachtrag A.3** (P7, P9, P11, P12) und **Nachtrag A.4** (Review R1.7, 16.10.2026). **Status: freigegeben am
15.10.2026 bis aufs Tor; Nachträge A.2 bis A.4 zur Freigabe.** Was sich ändert und was widerlegt ist: §17. Offen Ent-2
(vor R2.1), Ent-8, Ent-10, Ent-14, Ent-17 (Tor), Ent-16 (vor R3). Momentaufnahme (DIR-006); der lebende Stand steht in
`BEFUNDE.md`.

**Zeilenangaben:** Alle `Datei:Zeile`-Anker beziehen sich auf `72cfda7`. Seit R1.1 liegen die Zeilen in `http.cpp` ab
rund `:2839` um rund +210 später (Lead, 15.10.2026). **Massgeblich ist die genannte Funktion bzw. das Symbol**; neue
Anker ab A.3 nennen nur noch diese.

**Kennzeichen** wie in `requirements.md`: **✔ SW** am Code `72cfda7` bzw. Core 3.1.2 nachgelesen; **✔ B** in
`BEFUNDE.md`; **✔ L** vom Lead gemessen; **✔ FA** vom `firmware-analyst` am Core belegt; **◐ FA** vom
`firmware-analyst` als plausibel gemeldet; **? FA** vom `firmware-analyst` ausdrücklich als Spekulation gekennzeichnet;
**✔ R** / **◐ R** vom `code-reviewer` in R1.7 belegt bzw. als Lesart gemeldet; **◐** abgeleitet; **●** gemeldet, vor
der Umsetzung zu prüfen.

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
| Kein Service Worker über `http://`; `layout-previews.json` mit `no-store` | **✔ SW** | `registerAppServiceWorker()`, `loadLayoutPreviewRowsMap()` in `app.js` |
| `install-app.sh` und Auto-Installation laden `index.html` zuerst | **✔ SW** | `tools/install-app.sh:36-38`; `APP_INSTALL_ASSETS` |
| Core-`crc32()` ist CRC-32/MPEG-2, bitweise | **✔ FA** / **✔ SW** | `cores/esp8266/crc32.cpp:26-43` |
| 31 schreibende LittleFS-Stellen, vollständig; AutoFormat in `LittleFS.begin()` | **✔ FA** | §4.4 |
| `LittleFS.remove()` vor `LittleFS.begin()` in `http_api_app_file_upload()` wirkungslos; `LittleFS.format()` in `setup()` in `#if 0` | **✔ FA** / **✔ SW** | Nebenbefunde |
| Nach dem Start fällt `max` von 7'184 auf 4'672 im Fenster des Startabgleichs und bleibt | **✔ B** | L460, L457 |
| Schreiben im **laufenden Betrieb** hinterlässt nichts — mit und ohne Abruf | **✔ L** | L468 (b), (c) |
| Start mit `unveraendert` erzeugt keine Zerstückelung | **✔ L** / **✔ B** | V3 in T2, L466; L468 |
| C71 entsteht nur, wenn beim Start geschrieben wird | **◐** | aus L460, L468, V3 |
| PUYA-Puffer (0x85) beim ersten Flash-Schreiben als Ursache | **nicht bestätigt** | L468 (b) gültiger Test des PUYA-Zweigs (✔ FA) |
| Einzige belegte Asymmetrie Start/Betrieb: der erste `ClientContext` des statischen httpclient (`hc=0`) | **✔ FA** | `httpclient.cpp:16`; `httpclient_stop()` gibt den Kontext nicht frei |
| **Hypothese H-CC:** TIME_WAIT-PCB aus Schritt 4 trennt den freigegebenen Bereich; Best-Fit legt den `ClientContext` aus Schritt 5 hinein | **? FA** | Prüfung G9.6 (vierter Start) |
| **C75 (neu):** `httpclient()` gibt bei leerer Antwort (`len == 0`) mit offenem `ClientContext` zurück; Aufrufer, die bei `len <= 0` abbrechen, schliessen nicht — darunter der Update-Abgleich (`update_cache_write()`: `len <= 0` ⇒ Zeile `fehlgeschlagen abruf`, Rückkehr ohne `httpclient_stop()`) | **✔ R** / **✔ SW** | `httpclient.cpp:220-228`; Aufrufer in `http.cpp` (Stand Review: `:9105`) und `update_cache_write()` |
| `ESP.getHeapStats()` liefert `frag`; die Zahl freier Einträge steht nur im privaten `umm_local.h` | **✔ R** | I-6 nutzt `frag=` |
| `frag = 100·(1 − √Σh²/Σh)` reagiert auf ein zusätzliches kleines Loch neben einem grossen kaum (20'000 + 100 Byte ⇒ 0–1 Punkte) | **✔ R** (gerechnet) | §3.3, §5.5 |
| `- update cache nach …` misst `tcp=`/`hc` rund 10 ms nach `n` (dazwischen `Serial.flush()`) | **✔ R** (H2) | nur vermerkt |
| lwIP entfernt TIME_WAIT nach 2·MSL über dieselbe `tmr`, also bei `ruhe_s` > rund 120 — nicht 120 s nach dem Schliessen | **◐ R** (H4, passt zu P4) | §5.5: 130-s-Regel bleibt konservativ |
| Wetter-Abruf mit eigenem `WiFiClient`, von `hc` nicht erfasst | **✔ SW** | `weather.cpp:16` (`openweather_client`) |
| Grund der 60 s: STM-Vollabgleich nach `IPADDRESS` liefert Host und Pfad | **✔ FA** (P11) | A79, L421 |
| `http_server_begin()` steht in `wifi_connect()` vor `IPADDRESS` | **✔ FA** / **✔ SW** | `wifi.cpp` |
| Ausser LittleFS schreibt beim Start praktisch nichts Flash; `.tmp`-Löschen und AutoFormat ohne Logzeile | **✔ FA** (P12) | `eeprom_commit()`; `update_cache_poll()`; `LittleFS.begin()` |
| Safari-Sitzung: `free` −1,7 KB über Minuten; frei erst beim Beenden | **✔ B** / **●** | L461 |
| `stop()` ⇒ `tcp_close()` ⇒ FIN, `TF_RXCLOSED`; FIN_WAIT_2 läuft rund 20 s nach dem letzten Segment ohne RST ab | **✔ SW** / **◐ FA** (P1) | P1-Probe §5.6 |
| FIN_WAIT_1, CLOSING, LAST_ACK ohne eigene Frist, nur Wiederholungen bis `TCP_MAXRTX` 12 | **◐ FA** | `lwipopts.h:1258` |
| `pcb->tmr` je angenommenem Segment, nicht nach `TF_RXCLOSED`, nicht beim Zustandswechsel | **◐ FA** (P4) | §3.1 |
| SYN_RCVD trägt den Listener; ESTABLISHED mit `callback_arg == NULL` möglich (`TF_CLOSEPEND`) | **◐ FA** (P3, P8) | `lwip/tcp.h:255` |
| Tote `ClientContext` in `_unclaimed`; `!http_client` kehrt ohne Zähler zurück | **◐ FA** / **✔ SW** | `http_server_loop()` |
| K1 wirkt nur im Teilfall `no_request_timeouts`; bei `aborts` hat der Core schon abgebaut | **✔ FA** (P7) | `ClientContext.h:598-614` |
| Späte Anfrage auf geschlossene Verbindung bekommt schon heute RST | **◐ FA** (P7) | — |
| **C74:** Warteschlaufe auf die erste Zeile endet nach dem letzten `delay(1)` ohne erneutes `available()` | **✔ FA** | nicht Teil des Pakets |
| `MEMP_NUM_TCP_PCB` begrenzt bei `MEMP_MEM_MALLOC` nichts; TIME_WAIT im Core ≤ 6, nach `tcp_tw_limit()` 2 | **✔ FA** (P2) | Kommentar am I-4-Block in R1.1 berichtigt |

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

- **`ruhe_s` (◐ FA P4):** `(tcp_ticks − pcb->tmr) × TCP_SLOW_INTERVAL / 1000`; `tmr` wird bei jedem angenommenen Segment
  gesetzt, aber nicht mehr nach `TF_RXCLOSED` und nicht beim Zustandswechsel — „seit dem letzten empfangenen Segment vor
  unserem `close()`“. Nach H4 (◐ R) endet TIME_WAIT, wenn `ruhe_s` rund 120 überschreitet.
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

Je Durchgang von `update_cache_poll()` drei Zeilen, dazu Einzelzeilen. **Stand nach Review R1.7 (A.4):**

**(a) vor Schritt 1 — Startzustand:**

```
- update cache vorher up=<s> free=<n>/<blk> frag=<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> hc=<httpclient_connects>
```

Die Zeile steht **nach** dem Löschen liegengebliebener `.tmp` im ersten Durchgang (Review: vertretbar) — `vorher` und `v`
enthalten die Wirkung dieses Gangs; ob er lief, zeigt Zeile (f).

**(b) am Ende oder beim Abbruch — Verlauf, die tragende Zeile:**

```
- update cache heap v=<free>/<blk> 1=<free>/<blk> 2=… 3=… 4=… 5=… n=<free>/<blk>
```

**Tragende Grösse für C71 ist die Lochsumme `L = free − blk` je Messpunkt** — die Summe aller freien Bereiche ausser dem
grössten. Eine neue, dauerhafte kleine Belegung mitten im grossen Block zeigt sich als Sprung in `L` bei fast
unverändertem `free`: `blk` fällt, der abgetrennte Rest zählt zu `L`. An der Heap-Zeile gilt entsprechend
`L = free − max`.

**(c) unmittelbar nach (b) — Zustand am Ende:**

```
- update cache nach frag=<v>/<1>/<2>/<3>/<4>/<5>/<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> hc=<n> acc=<Δ>
```

- **`frag`** aus `ESP.getHeapStats()` (`fe` — Zahl freier Einträge — steht nur im privaten `umm_local.h`, ✔ R).
  **Geringe Empfindlichkeit, benannt:** `frag = 100·(1 − √Σh²/Σh)` bewegt sich bei einem zusätzlichen kleinen Loch
  neben einem grossen kaum (20'000 + 100 Byte ⇒ 0–1 Punkte). `frag` ist deshalb **nur ergänzend**; keine Regel stützt
  sich allein darauf.
- **`tcp=` und `hc` werden rund 10 ms nach `n` gemessen** (dazwischen `Serial.flush()`, H2) — im Messprotokoll vermerkt,
  nicht geändert.
- `hc` erfasst den Wetter-Client (`weather.cpp:16`) nicht. `acc`: Δ des Zählers `http_accept_count` über den Durchgang.

**(d) `- heap erste anfrage free=<n> max=<n>`** — beim ersten **lebenden** `accept()` nach dem Start, also auch bei einer
Vorrats-Verbindung ohne Anfrage (Review).

**(e) `- flash id=<hex6> vendor=<hex2>`** einmal in `setup()`, vor WLAN und `EEPROM.begin()`.

**(f) `- update cache tmp geloescht <datei>`** — wenn der erste Durchgang eine liegengebliebene Temporärdatei löscht.
Das AutoFormat in `LittleFS.begin()` bleibt ohne Logzeile (benannte Restlücke).

Je Zeile ≤ 119 Zeichen, einmal formatiert, Mitschnitt und Ring. Bestehende Zeilen `- update cache <datei> …` unverändert.

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
zählt selbst. App-Dateien schreiben oder löschen können: Auto-Installation (`http_try_auto_install_app_files()` ⇒
`download_file_to_local()`), Legacy `savedwnfil`/`dwntable`, Legacy-Löschen, `http_api_app_file_upload()`,
`fs_remove`, Formatieren.

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
vorher `heap_low_reset=1`, danach lesen. Je Laden: Leitungsstatus, `If-None-Match` gesendet, `encodedDataLength`,
`heap_low_block`, `heap_low_path`, I-4, `app_*`. `--erwarte-304`. Keine Haltegrenze.

### 5.2 Probe U (AKW.3)

`layout-previews.json` um Leerraum ergänzt, `gzip -9`, über `app_file_upload`; danach Original zurück.

### 5.3 Browser-Sitzung (`browser-sitzung`, Lead)

| Phase | Was | Dauer |
|---|---|---|
| **S0** | Ruhe | 180 s |
| **S1** | Browser öffnet `http://<uhr>/app/`, sichtbar | 120 s |
| **S2** | neuer Tab `about:blank` vorne — PWA verborgen | 180 s |
| **S3** | PWA-Tab geschlossen, Browser läuft | 300 s |
| **S4** | Browser beendet | 180 s |

Betriebsarten `osascript` und **`manuell`**. Lesung am Phasenende: `device_ready`, `netstat`, `lsof`; je eigene Lesung
Port und Ende ihres TIME_WAIT-Fensters (Lesezeit + 120 s — nach H4 ◐ eine obere Schranke). Je Sitzung
Δ`no_request_timeouts` gegen Δ`no_request_aborts` und Δ`http_accept_dead`.

### 5.4 Entscheidungsregel C72 (AKW.5)

„Lücke“ = `free_heap`(S0) − `free_heap`(Phase). Eigene Lesungen und ihr TIME_WAIT-Fenster zählen nicht. Zugeordnet wird
auf G10; G9 und die P1-Probe daneben.

| Ergebnis | Bild | Kandidat (Ent-8) |
|---|---|---|
| **M1** — geschlossen, nicht abgebaut | in S2 oder S3 ein PCB mit `arg` = 0, Gegenstelle Mac, in FIN_WAIT_1, CLOSING oder LAST_ACK mit `nrtx` > 0, oder ESTABLISHED/CLOSE_WAIT (`TF_CLOSEPEND`), in zwei Lesungen hintereinander; Lücke ≥ 500; in S4 verschwunden. FIN_WAIT_2 mit `ruhe_s` ≥ 30 gehört auch hierher und belegt zugleich **gegen** P1 | **K2** |
| **M2** — Strom von Vorab-Verbindungen | in S2/S3 steigen `nrt` oder `http_accept_dead` um ≥ 1 je Minute; in ≥ 2 Heap-Zeilen ausserhalb eigener Fenster `fw2`/`tw` > 0; kein PCB aus M1; Lücke ≥ 500; in S4 stehen beide still | **K1 nur, wenn Δ`no_request_timeouts` den Anstieg trägt**; sonst kein K1, Befund, K5 als Zusatz prüfen |
| **M3** — beides | M1 und M2 | **K2**, dazu K1 unter derselben Bedingung |
| **M4** — angenommen und hängend | ein PCB mit `arg` = 1, nicht die eigene Lesung, ESTABLISHED oder CLOSE_WAIT, `ruhe_s` ≥ 30 — oder `tcp_now_unclaimed` ≥ 1 in zwei Lesungen hintereinander; SYN_RCVD zählt nicht | **kein Eingriff**; eigener Befund |
| **M5** — keine Verbindung | Lücke ≥ 500, aber ausser eigenen Einträgen und ≤ 2 TIME_WAIT nichts — oder keine Lücke ≥ 500 | **kein Eingriff** |

**Widerlegt die P1-Probe P1**, ist **K2 zwingend**.

### 5.5 Startmessung und Entscheidungsregel C71 (AKM.5, AKS.2) — der entscheidende Schritt für C71

**Was feststeht (✔ L, L468):** Schreiben im laufenden Betrieb hinterlässt nichts; ein Start mit `unveraendert` auch
nicht. **Was geprüft wird (? FA, H-CC):** ob ein `ClientContext` des httpclient sich beim Start in den vom Schreiben
freigegebenen Bereich legt, und ob ein vor dem Abgleich angelegter Kontext (`hc` > 0) das verhindert.

**Ablauf je Start:** kein Browser; ESP-Neustart über den Weg aus V3 oder `/api/local_esp_restart`; **keine HTTP-Lesung,
bis `- update cache heap …` erschienen ist, plus 130 s**; Mitschnitt mit allen I-6-Zeilen und Heap-Zeilen; danach
Smoketest samt Update-Quelle. Vermerke: Flash-Schreiben beim Start (samt `tmp geloescht`), Wetter-Overlay ja/nein,
**`tcp=`/`hc` in `nach` rund 10 ms nach `n` gemessen (H2)**.

**Die 130-s-Regel (◐ R, H4):** Nach der Lesart des Reviewers entfernt lwIP einen TIME_WAIT-PCB, wenn
`tcp_ticks − tmr` 2·`TCP_MSL` = 120 s überschreitet, und `tmr` steht nach `TF_RXCLOSED` still (P4). Jeder PCB, der beim
Messpunkt `n` in TIME_WAIT liegt, hat sein letztes Segment **vor** `n` empfangen — er ist also spätestens rund 120 s
nach `n` fort, eher früher. 130 s nach `n` liegt damit hinter jedem solchen Ende, mit 10 s Rand für den 500-ms-Takt
des Langsam-Timers und die Minutentaktung der Heap-Zeile. **Die Regel stimmt zu H4 und bleibt konservativ;** sie hängt
an einer Lesart der binären Bibliothek (◐). Deshalb gilt zusätzlich, dass die bewertete Heap-Zeile `tw=0` zeigt.

**Starts:**

| # | Was | Erwartet |
|---|---|---|
| 1, 2 | ohne Änderung | `unveraendert`, keine Zerstückelung (wie V3) |
| 3 | **erzwungenes Schreiben beim Start** (`/upd-wc-list.txt` vorher per `/api/fs_remove` löschen) | `geschrieben`; nach L460 Zerstückelung |
| 4 *(empfohlen, Abweichung)* | wie 3, dazu rund 50 s nach dem Start ein kalter `update_status` (`hc` > 0 in `vorher`); als Abweichung von „keine HTTP-Lesung“ gekennzeichnet | keine Zerstückelung ⇒ H-CC gestützt; Zerstückelung trotzdem ⇒ H-CC widerlegt |

**Auswertung (A.4):** Tragend ist die **Lochsumme `L = free − blk`** aus `- update cache heap` (`v` und `n`) und an der
bewerteten Heap-Zeile (`free − max`). **„Zerstückelt“** heisst: an der Heap-Zeile ≥ 130 s nach `n` mit `tw=0` ist
`max` um ≥ 1'000 tiefer als `blk` bei `v`, **und** `L` ist um ≥ 1'000 höher als bei `v`, bei einem `free`, das bis auf
höchstens 300 Byte zu `v` zurückgekehrt ist. `frag` wird daneben berichtet, trägt aber keine Entscheidung (geringe
Empfindlichkeit, §3.3).

**Durchgänge mit offenem Client (C75):** Endet ein Schritt mit leerer Antwort oder `fehlgeschlagen abruf`, bleibt der
`ClientContext` des httpclient offen, und der Durchgang misst mit offenem Client. Ein solcher Start wird im
Messprotokoll gekennzeichnet und **nicht** allein für N1a/N1b herangezogen; im Zweifel wird er wiederholt.

| Ergebnis | Bild | Kandidat (Ent-17) |
|---|---|---|
| **N0** — PUYA-Puffer (bedingt) | nur wenn `vendor=85`: im ersten Schreibschritt ΔFree ≈ −264 ohne Erholung | **L0** |
| **N1a** — Startzustand, H-CC gestützt | Start 3 „zerstückelt“ (`hc=0` in `vorher`), Start 4 **nicht** (`hc` > 0); der Sprung in `L` liegt bei Schritt 4 oder 5 | **L7** (mit C75) nach P9b |
| **N1b** — H-CC widerlegt oder nicht geprüft | Start 3 **und** 4 „zerstückelt“, bzw. Start 4 nicht gefahren | **L6** |
| **N2** — auch ohne Schreiben | Start 1 oder 2 „zerstückelt“ — widerspricht V3 | **L2** (mit Vorbedingung, §7) |
| **N3** — auch im laufenden Betrieb | widerspräche L468 (c) | **L4b** |
| **N4** — nicht im Abgleich | `L` bei `v` schon hoch (oder `erste anfrage` klein), oder kein Start „zerstückelt“ | **keine Massnahme** |

### 5.6 P1-Probe (AKM.7)

Ein Python-Socket vom Mac sendet `GET /api/device_ready HTTP/1.0\r\n\r\n`, liest bis zum FIN des ESP und schliesst
**nicht**. Aus einer zweiten Verbindung `device_ready` nach 10 s und nach 25 s; `netstat` auf der Mac-Seite. Erwartet
unter P1: nach 10 s FIN_WAIT_2, `arg` = 0, Port der Probe; Mac-Seite CLOSE_WAIT; nach 25 s weg. **Noch da ⇒ P1
widerlegt, K2 zwingend.**

---

## 6. Eingriffskandidaten C72 (R3, Ent-8)

| | Mechanismus | Wirkung (◐ bis G11) | Risiko | Speicher |
|---|---|---|---|---|
| **K1** | im Zweig „keine Anfrage“ von `http_server_loop()`: `abort()` statt `stop()`, **nur im Teilfall `no_request_timeouts`**; im Teilfall `aborts` hat der Core schon abgebaut | still gehaltene Vorab-Verbindung ohne FIN_WAIT und TIME_WAIT | RST an eine unbenutzte Verbindung; kein neues Fenster (späte Anfragen bekommen schon heute RST); der Wettlauf **C74** erzeugt schon heute Resets — deshalb AKE.3 gegen die Grundlinie R2 | 0 |
| **K2** | einmal je Sekunde: PCBs mit `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING, LAST_ACK und — bei `TF_CLOSEPEND` — ESTABLISHED, CLOSE_WAIT, mit `ruhe_s` > Frist ⇒ `tcp_abort()`; Gang neu nach jedem Abbau; Zähler `tcp_reaped` | gebundener Speicher nach der Frist frei | schreibender Zugriff auf lwIP (Ent-10); nie `arg` = 1 oder 2 | ~4 Byte |
| **K3** | FIN_WAIT_2-Frist verkürzen | — | in die Bibliothek übersetzt — **geht in K2 auf** | — |
| **K4** | Höchstzahl je Gegenstelle | — | hoch — **nicht empfohlen** | 0 |
| **K5** | `HTTP_TW_LIMIT` 1 | ~0,2 KB (●) | wie E4 §19.7, schärfer | 0 |

**Frist in K2 (Ent-14):** 5 s empfohlen. Ist Δ`timeouts` ≈ 0, ist K1 für L461 wirkungslos.

---

## 7. Kandidaten C71 (R3 nach Ent-16 A, Ent-17)

| | Mechanismus | Wirkung | Risiko | Speicher | Stand |
|---|---|---|---|---|---|
| **L7** — `ClientContext` des httpclient nach jedem Abruf freigeben, **samt C75** | in `httpclient_stop()` nach `client.stop()` den Kontext freigeben (`client = WiFiClient()`); **dazu C75:** jeder Weg, der nach `httpclient()` bei `len <= 0` aufgibt — `httpclient()` selbst bei leerer Antwort oder die Aufrufer, darunter `update_cache_write()` —, ruft `httpclient_stop()` | unter H-CC: das Loch schliesst sich wieder, `blk` bleibt am Stück; kein Durchgang endet mehr mit offenem Client | ◐: unter H-CC neu zu bewerten (**P9b**); jeder Abruf legt einen frischen Kontext an. C75 allein ist ohne Risiko — ein geschlossener Client nach einer leeren Antwort ist der gewollte Zustand | 0 | **Kandidat bei N1a**, erst nach P9b |
| **L6** — Abgleich nach dem Start verzögern | `UPDATE_CACHE_START_MS` von 60 s auf 300 s; „5 s nach der letzten Anfrage“ bleibt | der Abgleich trifft einen gesetzten Heap (L468 (c)) | (1) Notes bis 300 s leer, wenn die Datei ungültig ist; (2) eine ungültige Liste holt `update_status` 5 min lang live in der Anfrage (Muster L449); (3) `refresh=1` und Quellenwechsel wirken 300 s nicht; (4) nach einem OTA über die PWA trifft der Abgleich einen Heap mit C72-Resten. **Unter H-CC wirkt L6 nur, wenn vor 300 s schon ein `ClientContext` angelegt wurde** | 0 | **Kandidat bei N1b** |
| **L2** — Abgleich vor dem ersten Netzverkehr | — | — | **in dieser Form nicht umsetzbar:** `http_server_begin()` steht in `wifi_connect()` vor `IPADDRESS`, es gäbe nur `quelle-leer`; echte Vorbedingung: Host und Pfad auf dem ESP, oder Vollabgleich vor `http_server_begin()` | 0 | nur mit Vorbedingung, eigener Nachtrag |
| **L0** — PUYA-Puffer früh anlegen (Ab-1) | nur bei `vendor=85` | Puffer unten | Wahl des Worts | 0 | **bedingt** |
| **L4b** — nie zwei `File` gleichzeitig | in `update_cache_write()` | — | E4.5a-Vergleich umzustellen | 0 | nur für N3 |
| **L1**, **L3**, **L4a**, **L5** | — | — | — | — | **verworfen** (A.2) |

**Nachzuziehen bei L6:** `TESTPLAN-PWA.md:798`, `:809`, `:959` (`doc-writer`) und `c65n.cpp:52-54`, `:81`, `:87`, `:214`
(Lead), Stand `72cfda7`.

**C75 ohne L7:** Wählt Ent-17 nicht L7, bleibt C75 ein eigener Befund in `BEFUNDE.md` (vom Lead eingetragen); dieses
Paket behebt ihn dann nicht.

**Empfehlung:** N1a ⇒ L7 (mit C75, nach P9b); N1b ⇒ L6 mit den vier Risiken. Ohne vierten Start bleibt nur N1b.

---

## 8. Prüfstände

Ablage `tools/checks/auszug/esp/c72/`, `…/c71/`, `…/c70/`, PWA neben b48; in S16. Fallzahl gegen das, was es geben
müsste (L235); einmal gegen den Vorgängerstand fehlgeschlagen; je Massnahme eine Sabotage (DIR-014).

| Prüfstand | Release | Gegenstand | Fälle (Auszug) | Sabotage ⇒ FEHL |
|---|---|---|---|---|
| **c72i** | R1 | I-5 | Attrappen `tcp_active_pcbs` (alle 11 Zustände; `callback_arg` NULL, `ClientContext`, Listener; ESTABLISHED mit NULL), `tcp_tw_pcbs`, `tcp_ticks`, `_unclaimed`; `arg` 0/1/2; Liste ≤ 8 + `tcp_now_more`; `http_accept_dead` nur bei nicht leerem `_unclaimed`; kein `http_send` im Listengang; Heap-Zeile ≤ 119, Mitschnitt = Ring | Ausgabe in der Schleife; Kappung entfernt; SYN_RCVD als `ClientContext`; toter Abholversuch ohne Zähler; leerer Durchlauf gezählt; nur `Serial` |
| **c71i** | R1 | I-6 | `vorher` einmal vor Schritt 1 (nach dem `.tmp`-Gang) mit `up`, `free/blk`, `frag`, `tcp=`, `hc`; `heap` mit `free`/`blk` je Schritt; `nach` mit `frag` je Messpunkt, `tcp=`, `hc`, `acc`; Durchgang mit 5, 3, 0 Schritten; `-`; Werte nach `LittleFS.end()`; `tmp geloescht`; `erste anfrage` einmal beim ersten lebenden `accept()`; `flash id` vor `EEPROM.begin()`; je ≤ 119 | Schritt ohne Wert; nur `blk`; `vorher` nach Schritt 1; `nach` ohne `hc`; `acc` nicht als Δ; `tmp geloescht` fehlt; nur `Serial` |
| **c70e** | R2 | C70 (a) | `HTTP/1.1 200` + `ETag`; `304` ohne Körper und `Content-Length`; `W/`, Liste, `*`, überlang; leere Datei nie `304`; Index ohne `app_complete`; ausserhalb der Weissliste `HTTP/1.0` ohne `ETag`; Körperlänge = `Content-Length`; `seek` scheitert. **T:** zwei `.gz` gleicher Grösse, anderer Inhalt ⇒ andere `ETag`; abgeschnitten; ohne `1f 8b`. **V:** CRC-32/MPEG-2; Invalidierung an allen Stellen. **Statisch:** `HTTP/1.1` an genau zwei Stellen mit `Connection: close` | `ETag` nur aus Grösse; `304` ohne Grössenprüfung; mit Körper; ohne `Connection: close`; `HTTP/1.1` in `/api/`; V: Invalidierung entfernt, CRC mit zlib |
| **c70p** | R2 | PWA | `"no-cache"` in `loadLayoutPreviewRowsMap`; Sprachdatei mit `?v=` | `"no-store"` |
| **c65b, c65w, LittleFS-Attrappe** | R2 | Bestand | Kopfzeilen in einem Schreibvorgang mit `HTTP/1.1` und `ETag`; `seek`, Trailer- bzw. zweites Lesen | wie bisher |
| **c72e** | R3 | Eingriff C72 | K1 `abort()` nur im Teilfall Timeout; K2 nur `callback_arg == NULL` + Zustand + `ruhe_s` > Frist; SYN_RCVD, `ClientContext`, TIME_WAIT, LISTEN unberührt; ein Gang je Sekunde; Neubeginn nach Abbau; K5 Grenze 1 | K1 `stop()` bzw. auch im Teilfall Abbruch; K2 PCB mit `callback_arg` ≠ NULL bzw. SYN_RCVD abgebaut, Frist entfernt; K5 Grenze 2 |
| **c71e** | R3 | Massnahme C71 | **L7:** `httpclient_stop()` gibt den Kontext frei, der nächste Abruf funktioniert, kein Zugriff auf den freigegebenen Kontext; **C75:** nach einer leeren Antwort (`len == 0`) und nach `len <= 0` in `update_cache_write()` ist der Client geschlossen; **L6:** erster Durchgang nicht vor 300 s, 5-s-Bedingung bleibt, c65n nachgezogen; **L0/L4b:** wie A.2 | L7: Kontext nicht freigegeben; C75: leere Antwort lässt den Client offen; L6: Frist 60 s |

---

## 9. Werkzeuge (Lead)

- **`ladeprobe`** — liegt vor; Leitungsstatus.
- **`browser-sitzung`** — `osascript` und `manuell`; je eigene Lesung Port und TIME_WAIT-Ende; je Sitzung
  Δ`no_request_timeouts`, Δ`no_request_aborts`, Δ`http_accept_dead`.
- **P1-Probe** — Python-Skript nach §5.6.
- **`check-pwa`** (R2) — INFO `304` je Laden, Byte je Laden, je Lauf `ERR_CONNECTION_RESET` und `ERR_EMPTY_RESPONSE`.
- **`smoke-device.sh`** — ab R1 I-5-Felder, ab R2 `app_*`.
- **`watch-log.sh`** — ab R1 Heap-Zeile mit Zusatz und die I-6-Zeilen als Hinweis; mit V `- app etag`.
- **Auswertung C71** — rechnet `L = free − blk` je Messpunkt aus `- update cache heap` und `free − max` aus den
  Heap-Zeilen; `frag` nur daneben.
- **R2:** `tools/**` nach Prüfungen auf `HTTP/1.0` durchsuchen; c65b, c65w, LittleFS-Attrappe nachziehen.
- **R3 mit L6:** c65n nachziehen.

---

## 10. RAM-Budget (AKZ.8)

| Teil | Release | statisch (Byte) |
|---|---|---|
| I-5 samt `http_accept_dead` | R1 | ~2 |
| Heap-Zeile | R1 | 0 |
| I-6 (Schrittwerte, `frag`, `http_accept_count`, Merker) | R1 | ~50 |
| `If-None-Match`-Puffer | R2 | 36 |
| Zähler `app_*` | R2 | 4 (T) / 6 (V) |
| `ETag`-Tabelle | R2 | 0 (T) / 104 (V) |
| K2 | R3 | ~4 |
| C71 L7 (mit C75)/L6 | R3 | 0 |
| **Summe** | | **rund 100 (T) / 205 (V)** — Grenze 250 (Ent-11) |

---

## 11. Releaseschnitt, Einspielen, Rückweg

**R1** — ESP. Rückweg: ESP 3.2.37 aus `wordclock-release-2026-10-14-180012.zip` (L457).

**R2** — ESP und PWA, Firmware zuerst (DIR-002). Alle vier Paarungen sicher. Ablauf: Wache; OTA; Smoketest samt
Update-Quelle; `install-app.sh --check`; `install-app.sh`; `--check`; `check-pwa.sh`. Rückweg: ESP auf R1, PWA bleibt.

**R3** — ESP. Rückweg auf R2; mit Ent-16 A nimmt er C71 **und** C72 zurück.

Du spielst jedes Release ein; der Lead liefert die Zeilen samt Rückweg, **bevor** eingespielt wird.

---

## 12. Prüfpunkte (`firmware-analyst`, ergänzt um den Review R1.7)

| P | Frage | Stand |
|---|---|---|
| **P1** | FIN_WAIT_2 nach `tcp_close()`? | **◐ FA:** ja, rund 20 s, ohne RST — P1-Probe |
| **P2** | `MEMP_NUM_TCP_PCB` bei `MEMP_MEM_MALLOC`? | **✔ FA:** begrenzt nichts |
| **P3** | `tcp_abort()` ohne `callback_arg` | **◐ FA:** SYN_RCVD trägt den Listener; K2 nur `NULL` |
| **P4** | `tmr` | **◐ FA:** §3.1; dazu **H4 (◐ R):** TIME_WAIT endet über dieselbe `tmr` bei `ruhe_s` > rund 120 |
| **P5** | `crc32()` | **✔ FA:** CRC-32/MPEG-2 |
| **P6** | Schreibstellen | **✔ FA:** 31; AutoFormat; Nebenbefunde |
| **P7** | `stop()`/`abort()` im Zweig „keine Anfrage“ | **✔ FA:** K1 nur bei `no_request_timeouts`; C74 |
| **P8** | Tote `ClientContext` | **◐ FA:** I-5 ergänzt |
| **P9** | C71: was beim Start lebt | **offen, H-CC (? FA)**; Prüfung G9.6; tragende Grösse `L = free − blk` (A.4) |
| **P9b** | Schliesst L7 das Loch, oder legt sich der nächste Kontext wieder hinein? | **offen**, vor Ent-17 |
| **P10** | LittleFS-Puffer | **◐ FA:** L1, L3 verworfen |
| **P11** | 60 s nach Start | **✔ FA:** STM-Vollabgleich; L2 nicht in der gedachten Form |
| **P12** | Flash-Schreiber beim Start | **✔ FA:** ausser LittleFS praktisch nichts; I-6 (f) |

---

## 13. Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Legacy parallel? Wetter-Endpunkte? Restore-Bedingung? Richtige Schicht?

> **Legacy** bleibt parallel, unverändert und bei `HTTP/1.0`; C70 setzt nur in `http_send_fs_file()` an. **Wetter:**
> unberührt — L7 und C75 betreffen nur den httpclient des Update-Servers, nicht `openweather_client`; K2 lässt laufende
> ausgehende Abrufe in Ruhe. **Restore-Bedingung:** STM unberührt. **`.gz` und Grösse > 0:** bleibt. **Schicht:** C70 in
> der Auslieferung; C71 dort, wo die Startmessung die Ursache zeigt — bei H-CC am Lebenszyklus des `ClientContext`
> (L7), sonst am Zeitpunkt (L6); C72 erst gemessen, dann im ESP.

**Scalable systems** — STM-Kommandos? UART? Watchdog? Hauptloop?

> **STM-Kommandos:** keine. **UART:** Heap-Zeile + rund 38 Byte je Minute; drei I-6-Zeilen je Durchgang, zwei je Start,
> `tmp geloescht` nur im Ausnahmefall. **Hauptloop:** T kostet je Asset-Antwort ein `seek` (◐ ms); jede `304`
> Millisekunden statt rund einer Sekunde; K2 einmal je Sekunde; unter L6 holt `update_status` eine ungültige Liste
> 5 min lang live in der Anfrage. L7 kostet je Abruf eine Anforderung und Freigabe von rund 56 Byte. **Watchdog:** keine
> Schleife ohne Grenze.

**Secure by design** — `innerHTML`? Fremddaten? Credentials?

> `If-None-Match` begrenzt, nur verglichen. Logzeilen nennen Zahlen, Dateinamen, Flash-Kennung. Kein neuer Endpunkt.
> K1/K2 senden RST nur an Verbindungen ohne Anfrage bzw. vom ESP geschlossene. L6, L7 und C75 ändern nichts an der
> Quelle des Abgleichs.

**Stable & reliable** — Fehler ausgewertet? Stille Verwerfungen? Zustand nach Abbruch? Flags?

> **Veralteter Cache:** mit T durch Bau ausgeschlossen. **`HTTP/1.1`:** `Connection: close`, exaktes `Content-Length`.
> **K1:** kein neues Fenster; AKE.3 gegen die Grundlinie R2. **C75:** ein Abruf mit leerer Antwort hinterlässt heute
> einen offenen Client — mit L7 nicht mehr. **Messung ohne Selbsttäuschung:** C71 wird an einer Grösse entschieden, die
> den Befund trägt (`L = free − blk`), nicht an einer, die ihn kaum sieht (`frag`); „erholt sich nicht“ erst nach
> TIME_WAIT (130-s-Regel, ◐ H4, plus `tw=0`); Durchgänge mit offenem Client gekennzeichnet; H2 vermerkt; die Abweichung
> im vierten Start ist gekennzeichnet; das AutoFormat bleibt als benannte Lücke.

---

## 14. Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| `ETag` bei `HTTP/1.0`; `HTTP/1.1` für alle | greift in Chrome nicht bzw. Keep-Alive-Risiko |
| `Last-Modified` | Rückfallebene nur für Safari |
| `ETag` aus Grösse allein / App-Version; CRC je Anfrage über die ganze Datei; `ETag`-Dateien | Kollision, unbekannt, zu teuer, zusätzliche Schreibvorgänge |
| `max-age` ohne Versionierung; Variante (b) jetzt | „sofort wirksam“ verletzt bzw. veralteter Cache |
| `fe` (Zahl freier Einträge) aus `umm_local.h` | privater Kopf (✔ R); `L = free − blk` trägt dieselbe Aussage aus öffentlichen Werten |
| `frag` als tragende Grösse für C71 | zu unempfindlich für ein kleines Loch neben einem grossen (A.4) |
| `device_ready` im Sekundentakt; HTTP-Lesungen in der Startmessung (ausser Start 4, gekennzeichnet) | verändern das Gemessene |
| K1 bei M2, wenn nur `aborts` steigen | dort hat der Core schon abgebaut (P7) |
| K3, K4; L1, L3, L4a, L5 | §6, §7 |
| L0 als Hauptkandidat | L468 (b) gültiger Test |
| L2 in der Form „vor `http_server_begin()`“ | ergäbe nur `quelle-leer` (P11) |
| C72-Eingriff oder C71-Massnahme ohne Messung; C71 in R2 | Mechanismus nicht belegt bzw. Cache nicht getrennt messbar |

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
| `ESP8266/ESP-uclock/http.cpp` (UTF-8, eine CRLF-Zeile) | R1: I-5, Kommentar am I-4-Block, Zählfunktion, I-6, `http_accept_count`. R2: `If-None-Match`, `ETag`, `HTTP/1.1` in `http_send_fs_file()` und `304`, Zähler. R3: K1/K2/K5; C71 L6 (`UPDATE_CACHE_START_MS`) bzw. mit L7 der C75-Teil in `update_cache_write()` und den übrigen Aufrufern | `esp-developer` |
| `ESP8266/ESP-uclock/httpclient.cpp` | R3 mit L7: `httpclient_stop()` gibt den Kontext frei; C75: leere Antwort schliesst | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` (gemischte Zeilenenden) | R1: `esp_heap_log()`, I-6 (e) in `setup()`; R3: K2-Aufruf | `esp-developer` |
| `ESP8266/ESP-uclock/stm32flash.cpp` (UTF-8) | R2 nur mit V | `esp-developer` |
| `ESP8266/ESP-uclock/base.h` | Prototypen | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | R2: `loadLayoutPreviewRowsMap()` | `pwa-developer` |
| `tools/**`, Prüfstände (dazu c65n bei L6) | §8, §9 | Lead |
| Versionsdateien | R1, R2, R3 | `release-engineer` |
| `TESTPLAN-PWA.md`, `BEFUNDE.md` | Felder, Zeilen, Lesart (`L = free − blk`, `frag` nur ergänzend); bei L6 die drei Stellen mit „rund 60 s“; Träger; Nebenbefunde | `doc-writer` |
| `CHANGELOG.md` | je Release | Lead |

---

## 17. Nachträge A.2, L468, A.3 und A.4 — was sich ändert, was widerlegt ist

**A.2 (P1–P8):** `ruhe_s` berichtigt; `arg` 0/1/2; M1 über FIN_WAIT_1/CLOSING/LAST_ACK mit `nrtx`; M2 ohne eigene
TIME_WAIT-Fenster; M4 ohne SYN_RCVD; Kommentar am I-4-Block falsch; Ent-2 offen (Variante T); Bestand 31; L1, L3, L4a,
L5 verworfen; P1-Probe, `tcp_now_unclaimed`, `http_accept_dead`, `manuell`, AKC.8.

**L468 (Trennmessung):** K-a/N0 nicht bestätigt; nur Schreiben beim Start zerstückelt; L6.

**A.3 (P7, P9, P11, P12):** K1 nur bei `no_request_timeouts`, AKE.3 gegen Grundlinie R2, M2 ⇒ K1 nur mit Δ`timeouts`;
H-CC (? FA), vierter Start, N1a/N1b, L7 (= Ab-2), P9b; L6 mit vier Risiken; L2 nicht umsetzbar; 130-s-Regel; I-6 um
`nach`, `acc`, `tmp geloescht` ergänzt; C74; Anker über Funktionsnamen.

**A.4 (Review R1.7, 16.10.2026):**

| Stelle (Stand vor A.4) | Neu |
|---|---|
| I-6 `fe=` (Zahl freier Einträge) | **`frag=`** aus `ESP.getHeapStats()` — `fe` steht nur im privaten `umm_local.h` |
| N1a „`fe` steigt um ≥ 1 und bleibt“ | **nicht auswertbar** — ersetzt durch die Lochsumme **`L = free − blk`** aus `- update cache heap` und `free − max` aus der Heap-Zeile; „zerstückelt“ als `max` ≥ 1'000 tiefer als `blk` bei `v`, `L` ≥ 1'000 höher, `free` ±300 zurück; `frag` nur ergänzend |
| `nach` misst `tcp=`/`hc` „bei `n`“ | rund 10 ms nach `n` (H2) — vermerkt, nicht geändert |
| 130-s-Regel als feste Grösse | **◐ (H4):** TIME_WAIT endet über `tmr`, spätestens rund 120 s nach dem letzten Segment vor `n`; die Regel ist dazu konservativ, `tw=0` bleibt Bedingung |
| L7 | **mit C75:** `httpclient_stop()` auch bei `len <= 0` (`httpclient()`, `update_cache_write()`, weitere Aufrufer); G9.6 kennzeichnet Durchgänge mit offenem Client |
| `- update cache vorher` „vor Schritt 1“ | steht **nach** dem `.tmp`-Gang; `v` enthält dessen Wirkung |
| `- heap erste anfrage` „erster Anfrage-Eingang“ | beim ersten **lebenden** `accept()`, auch bei einer Vorrats-Verbindung |

**Ausstehend:** P9 (H-CC durch G9.6), P9b (L7).
