# Design — Paket 2026-10-13, Nachtrag E4: zweites Release für C65 (Heap des ESP unter PWA-Last)

**Erstellt:** 2026-10-14, Stand `2f6a324` (Zweig `pwa-decoupling`). **Status: freigegeben 14.10.2026** — vom
Nutzer, samt Ent-29 (PWA zuerst), Ent-30 (B, Werte aus `settings_xml`), Ent-31 (erste Ladung messen, dann Tor)
und Ent-39 (Zero-Window-Probe entfällt); Ent-32 bis Ent-38 als Lead-Empfehlungen übernommen (Lead).
Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.

**Teil dieser Spec.** Dieses Dokument ist der Design-Nachtrag für Schritt **E4** zu `design.md` (dort §18 für
E3). Es steht in einer eigenen Datei, weil dem `spec-writer` in dieser Sitzung kein Werkzeug für Teiländerungen
zur Verfügung stand und ein Neuschreiben der 1'215 Zeilen von `design.md` das Risiko einer stillen Abweichung im
freigegebenen Text getragen hätte. **Wer `design.md` liest, liest diesen Nachtrag mit:** Die Stellen in
`design.md`, die E4 ändert, nennt §19.20 einzeln. Anforderungen und Akzeptanzkriterien stehen in
`requirements-e4.md`, die Tasks in `tasks.md` (Schritt E4).

**Grundlage:** `BEFUNDE.md` L456 (statische Analyse des `firmware-analyst` nach E3, 14.10.2026), L450
(E3 am Gerät), C65, C70, L449; Schritt E3 dieser Spec (`design.md` §18). **Nutzerentscheide 14.10.2026:**
E4 in diesem Paket, vor T2 und Z; WLAN-Scan nur beim Öffnen des Netzwerkmoduls und per Knopf, ohne
Wiederholung; verwaiste **lesende** `/api/`-Abfragen überspringen, Setter und Wartung immer ausführen; der
Asset-Cache (C70) **nicht** in E4. Aus L449/C65 (Nutzerentscheid 14.10.2026, „Verbindungen begrenzen"): M-2 —
Backlog und `power_status` — geht in E4 statt in einen Folgebefund.

**Kennzeichen** wie in `requirements.md`: **✔ SW** — vom `spec-writer` am Code `2f6a324` bzw. an der
Core-Quelle 3.1.2 (`~/Library/Arduino15/packages/esp8266/hardware/esp8266/3.1.2/`) nachgelesen; **✔ B** — in
`BEFUNDE.md` belegt; **◐** — abgeleitet, nicht belegt; **●** — gemeldet (Analyse L456), vor der Umsetzung zu
prüfen. **Lead-Empfehlung** heisst: technischer Detailentscheid, den der Lead treffen kann; **Entscheidung
offen** hiess im Entwurf: Nutzer — alle sind am 14.10.2026 entschieden.

---

## 19. C65, zweites Release (E4)

### 19.0 Befund mit Verifikationsstatus

| Aussage aus L456 | Status | Fundstelle / Bemerkung |
|---|---|---|
| Der Core nimmt neben der bedienten Verbindung bis zu 5 wartende an, jede sofort mit `ClientContext` auf dem Heap | **✔ SW** | `WiFiServer.cpp:38-40` (`MAX_PENDING_CLIENTS_PER_PORT 5`), `:58-60`, `:187-205` (`new ClientContext` in `_accept`, `tcp_backlog_delayed`) |
| Ein voller Backlog lässt weitere SYN unbeantwortet; der Client wiederholt | **✔ SW** (Kommentar des Cores) / ◐ (Wiederholzeit ~1 s) | `WiFiServer.h:53-64`; Wiederholzeit des Betriebssystems nicht am Gerät gemessen |
| Der Backlog ist zur Laufzeit über `tcp_backlog_set` änderbar; `_listen_pcb` ist `protected` | **✔ SW** | `lwip/tcp.h:444-454` (Makro, setzt 0 auf 1), `:234-237` (`backlog`, `accepts_pending` nur mit `TCP_LISTEN_BACKLOG`); `lwipopts.h:1405` (`TCP_LISTEN_BACKLOG LWIP_FEATURES`); `WiFiServer.h:71-78` |
| Posten beim Tief: wartende Verbindungen je ~0,8 KB, Sendepuffer der aktiven ~1,6 KB, TIME_WAIT ≤ 5 × ~192, Strings ~0,35 KB | **●** | Analyse auf lwIP upstream 2.1 (PLAUSIBEL); Summe ~6,9 KB gegen gemessen ~7,0 KB (L450). I-4 (§19.2) misst die Posten am Gerät |
| Der Core schreibt `min(tcp_sndbuf, Rest)` am Stück und versucht bei `ERR_MEM` kein kleineres Stück | **✔ SW** | `ClientContext.h:507-541` (`next_chunk_size = min(tcp_sndbuf, remaining)`, `ERR_MEM` ⇒ `break`); danach wartet `_write_from_source` bis zur Frist (`:471-489`) |
| Ein `tcp_write()` mit 1'072 Byte braucht ~1,35 KB, mit 536 ~0,67 KB; scheitert ein Teil, scheitert der ganze Aufruf | **◐** | lwIP-Verhalten upstream 2.1 (Segmente je MSS, Rücknahme bei Speicherfehler); die Bibliothek liegt nur als Binärdatei vor |
| `network_scan` läuft bei offenem Netzwerkmodul bei **jeder** Selbstaktualisierung, mit Wiederholung nach Zeitüberschreitung | **✔ SW** | `app.js:3401-3411` (Bedingung nur `networkActive`, `slowReadAttempts` = 2), `:2191-2200` (Takt), `:13135-13145` (`AbortError` gilt als wiederholbar) |
| Der Scan hält `loop()` synchron an und legt `80 × n` Byte an | **✔ SW** / ● | `http.cpp:9437` (`WiFi.scanNetworks ()` synchron), `:9497` (`scanDelete`); die Grösse je Eintrag ● |
| `loadSecondaryData` läuft ausserhalb der Ladesperre | **✔ SW** | `app.js:3137` (`void loadSecondaryData(...)`), `:3147-3151` (`finally` gibt die Sperre frei, bevor die Nebenabrufe enden), `:3036` (Sperre prüft nur `activeLoadCount`) |
| `display_power` und `ambilight_power` je eigene Verbindung, Frist 2'200 ms, Rückfallwert `"off"` | **✔ SW** | `app.js:3227-3240`, `:1085`; `isDisplayPowerOn()` `:3252-3264` nimmt `"off"` als echtes Aus — ein Abbruch zeigt also **still** „aus" |
| `power_status` liefert beide Werte in einer Antwort; die Fähigkeit steht in `update_status` | **✔ SW** | `http.cpp:8962-8973`; `:13907` (`power_status_api_supported`, nur in `update_status`); die PWA liest das Feld nirgends |
| `settings_xml` trägt beide Werte bereits, aus demselben Feld | **✔ SW** | `http.cpp:8757-8761` (`numvars[i]` für alle Indizes); `app.js:1134` (`DISPLAY_POWER: 3`), `:1157` (`DISPLAY_AMBILIGHT_POWER: 30`); Kommentar B11 `app.js:11095-11106` |
| TIME_WAIT ist im Core schon auf 5 begrenzt; `tcp_kill_timewait()` ist deklariert, `tcp_tw_pcbs` nur im privaten Kopf | **✔ SW** | `lwipopts.h:3677-3696` (`TCP_TW_LIMIT`, `MEMP_NUM_TCP_PCB_TIME_WAIT 5`); `lwip/priv/tcp_priv.h:337-340`; Symbol `tcp_kill_timewait` in `liblwip2-536-feat.a` vorhanden (7 Fundstellen, per Suche) — ob es **gelinkt** werden kann, ◐ bis zum ersten Build |
| `tcp_kill_timewait()` sendet kein RST | **◐** | upstream `tcp_abandon()` für TIME_WAIT; Bibliothek binär |
| Eine Netzfrist der PWA läuft ab dem **Aufruf**, nicht ab dem Senden | **✔ SW** | `app.js:13094-13106` (`fetchWithTimeout`), `:13108-13115` (`settleWithTimeout` mit eigenem Zeitgeber ab Aufruf) |
| Eine Antwort, die erst nach dem Abbau der Gegenstelle bedient wird, erreicht niemanden | **✔ SW** | `http.cpp:16025-16028` (C53: in CLOSE_WAIT schreibt der Core nichts); `WiFiServer.cpp:138-142` (Kontext ohne PCB nach Abbau) |

### 19.1 Zuschnitt und Reihenfolge

Sieben Teile in **einem** Release für ESP **und** PWA:

| # | Teil | Laufzeit | Schreiber |
|---|---|---|---|
| 0 | **I-4** Messfelder beim Tief, Scan-Zeile | ESP | `esp-developer` |
| 1 | **WLAN-Scan** nur beim Öffnen und per Knopf | PWA | `pwa-developer` |
| 2 | **Schreiben in Stücken ≤ `TCP_MSS`** | ESP | `esp-developer` |
| 3 | **Backlog nach freiem Heap** | ESP | `esp-developer` |
| 4 | **Warteschlange für `/api/*`**, Displaystrom aus `settings_xml`, Ladesperre, überholte Abrufe, Netzfristen | PWA | `pwa-developer` |
| 5 | **TIME_WAIT begrenzen** | ESP | `esp-developer` |
| 6 | **Verwaiste lesende Abfragen überspringen** | ESP | `esp-developer` |

**Warum ein Release:** Teil 3 ohne Teil 4 schadet — ein kleiner Backlog lässt überzählige SYN fallen, die
Gegenstelle wiederholt nach rund 1 s, und `display_power` mit seiner Frist von 2'200 ms läuft ab und zeigt
**still** „aus" (§19.0, Zeile Rückfallwert). Teil 4 ist umgekehrt **ohne** Teil 3 unschädlich: Die neue PWA
braucht keinen Endpunkt, den ESP 3.2.36 nicht hat. Daraus folgt die Einspielreihenfolge (§19.12).

**Reihenfolge der Tasks:** Die ESP-Teile schreibt **ein** `esp-developer` nacheinander (0, 2, 3, 5, 6), die
PWA-Teile **ein** `pwa-developer` nacheinander (1, 4). Die beiden Stränge berühren disjunkte Dateien, und
zwischen ihnen liegt **kein** Gerätenachweis — sie dürfen parallel laufen (R3b: keine Abhängigkeit, die eine
Reihenfolge verlangt). Der Release-Build wartet auf beide. Nach jedem ESP-Task misst der Lead „Globale
Variablen" (§19.9).

**Warum I-4 zuerst, obwohl im selben Release:** I-4 ist das Instrument, mit dem G8 die Wirkung der übrigen
Teile **getrennt** ablesen kann (§19.13). Es entsteht deshalb vor den Massnahmen und wird am Prüfstand belegt,
bevor eine Massnahme den Zustand verändert, den es misst.

### 19.2 I-4 — was beim Block-Tiefstwert los war (ESP)

**Ziel:** Für den Augenblick, in dem `heap_low_block` einen neuen Tiefstwert erreicht, festhalten, **wer** den
Heap belegt — ohne selbst Heap zu belegen.

**Momentaufnahme in `heap_low_note()`** (`http.cpp:273-294`), **nur** im Zweig „neuer Block-Tiefstwert"
(`blk < heap_low_block`), nicht für `heap_line_low` und nicht für `heap_low_free`:

| Feld (statisch) | Typ | Inhalt | Quelle |
|---|---|---|---|
| `heap_low_accepts_pending` | `uint8_t` | wartende, noch nicht angenommene Verbindungen am Port 80 | `((struct tcp_pcb_listen *) _listen_pcb)->accepts_pending`, über die Unterklasse aus §19.5 |
| `heap_low_backlog` | `uint8_t` | der in diesem Augenblick gesetzte Backlog | Zustand aus §19.5 |
| `heap_low_pcb_states[11]` | `uint8_t`, sättigend | Zahl der PCBs je Zustand in `tcp_active_pcbs`, Index = lwIP `enum tcp_state` (0 `CLOSED` … 10 `TIME_WAIT`) | Liste `tcp_active_pcbs` (privater Kopf, Ent-32) |
| `heap_low_tw` | `uint8_t`, sättigend | Länge von `tcp_tw_pcbs` | dito |
| `heap_low_unacked` | `int16_t` | unquittierte Byte der bedienten Verbindung: `TCP_SND_BUF − http_client.availableForWrite()`; **−1**, wenn `http_client` nicht verbunden ist (sonst stünde dort 1'072 als scheinbarer Höchstwert, weil `availableForWrite()` dann 0 liefert, `http.cpp:1503-1506`) | `WiFiClient` |
| `heap_low_req_path[32]` | `char` | Pfad der gerade bedienten Anfrage, **ohne Query**, höchstens 31 Zeichen; `-` ohne Anfrage (z. B. Pfad `accept` vor dem Lesen, `client` im Startabruf) | Zeiger auf den aktuellen Pfad (unten) |

- **Aktueller Pfad ohne neue Kopie (Lead-Empfehlung):** ein statischer Zeiger `http_cur_path` (4 Byte), gesetzt,
  sobald `sPath` in `http_server_loop()` feststeht (`http.cpp:16133`, `:16144`, `:16191`, `:16196`), und im
  Destruktor von `HttpRequestTimer` (`:15945-15953`) auf `NULL` zurückgesetzt — so kann er nie auf ein
  zerstörtes `String` zeigen. Die Momentaufnahme kopiert bis `?` oder 31 Zeichen, wie `http_request_note()`
  (`:15934-15938`).
- **Kein Heap, keine Ausgabe:** Listen werden nur gelesen, gezählt wird in die statischen Felder. Die Listen
  sind kurz (`MEMP_NUM_TCP_PCB` 5, TIME_WAIT höchstens 5, `lwipopts.h:444`, `:3696`).
- **Gleichzeitigkeit (◐, Prüfpunkt Review):** Im Arduino-Core für den ESP8266 laufen die lwIP-Rückrufe im
  Systemkontext, `loop()` im Benutzerkontext; der Systemkontext läuft nur, wenn der Benutzerkontext abgibt.
  `heap_low_note()` gibt nicht ab, die Listen ändern sich während des Zählens also nicht. Der Reviewer prüft das
  an den Aufrufstellen (kein `yield()`, kein `delay()` zwischen Beginn und Ende der Zählung).
- **Kosten:** nur beim neuen Tiefstwert, also selten; das Zählen ist billiger als das schon vorhandene
  `ESP.getMaxFreeBlockSize()` (`http.cpp:235-243`).
- **Ausgabe in `/api/device_ready`, hinten** — **hinter** `upd_cache_max_ms`, dem bisher letzten Feld:
  `heap_low_accepts_pending`, `heap_low_backlog`, `heap_low_pcb_states` (JSON-Feld mit 11 Zahlen),
  `heap_low_tw`, `heap_low_unacked`, `heap_low_req_path`; danach die beiden Zustandsfelder aus §19.5 und §19.8:
  `http_backlog` (aktueller Backlog) und `api_orphan_skipped`. Bestehende Felder unverändert in Name,
  Reihenfolge und Typ.
- **Rücksetzen** zusammen mit I-1 über `device_ready?heap_low_reset=1` (Ent-21 B): Werte lesen, ausgeben,
  zurücksetzen. Nach dem Rücksetzen gelten Zahlen 0, `heap_low_unacked` −1, `heap_low_req_path` leer; dass
  nichts gemessen wurde, zeigt wie bisher `heap_low_path` leer. `http_backlog` und `api_orphan_skipped` werden
  **nicht** zurückgesetzt (Zustand bzw. Zähler seit dem Start, wie `write_lost_*`).

**Scan-Zeile in `http_api_network_scan()`** (`http.cpp:9429-9500`):

```
- scan n=<netze> vor=<free>/<blk> nach=<free>/<blk> flush=<free>/<blk> ms=<scan>/<ausgabe>
```

- `vor` unmittelbar vor `WiFi.scanNetworks ()` (`:9437`), `nach` unmittelbar danach, `flush` unmittelbar nach
  dem letzten `http_flush ()` (`:9495`) **vor** `WiFi.scanDelete ()` — dann hält der ESP noch die Ergebnisliste,
  und die Differenz `nach` → `flush` ist die Last der Ausgabe. `ms=<scan>/<ausgabe>`: Dauer des Scans und Dauer
  vom Ende des Scans bis nach dem Flush. **Damit ist die offene Frage aus C68 beantwortbar**, ob die 2'871 ms
  aus L450 im Scan oder im Schreiben lagen.
- `n` ist die Zahl vor der Kappung auf `HTTP_MAX_SCAN_NETWORKS` (Rohwert des SDK).
- Ausgegeben **nach** `scanDelete ()`, einmal formatiert in einen Stackpuffer, über `Serial` **und**
  `stm32_log_append ()` (Muster `http.cpp:1559-1569`, L185); längste Form mit zehnstelligen Zahlen unter
  119 Zeichen (ein Ringeintrag). Kein `String`.
- Häufigkeit: Nach Teil 1 einmal je Öffnen des Netzwerkmoduls und je Knopfdruck — keine Zeile im Takt.

### 19.3 WLAN-Scan nur beim Öffnen und per Knopf (PWA, Nutzerentscheid)

- `network_scan` wird **nur** abgerufen, wenn der Ladelauf ausdrücklich danach fragt: ein Merker
  `scan: true` in den Optionen von `loadData()`. Gesetzt wird er an genau drei Stellen: beim Wechsel **in** das
  Netzwerkmodul, beim Start mit dem Netzwerkmodul als Startmodul und im vorhandenen Knopf
  `network-scan-button` (`app.js:1866`, `refreshNetworkScan()` `:10090-10103`). Jeder andere Lauf — die
  Selbstaktualisierung alle 15 s (`auto`), ein Neuladen nach dem Speichern, ein Wiederverbinden — fragt nicht
  und behält die letzte Liste (`getCurrentNetworkInfo()`).
- **Ein Versuch:** `fastReadAttempts` statt `slowReadAttempts` (`:3403`). Läuft die Frist ab, bleibt die alte
  Liste stehen; die PWA wiederholt nicht von sich aus. Der Knopf meldet den Fehlschlag wie heute
  (`network.scan_failed`).
- **Netzfrist** nach §19.6 (Regel „+1'000 ms"), gemessen ab dem Senden.
- `eeprom_settings` im Netzwerkmodul (`:3376-3399`) bleibt, wie es ist (bei `auto` nur, solange nichts da ist).

### 19.4 Schreiben in Stücken ≤ `TCP_MSS` (ESP) — Variante A (Lead-Empfehlung, Ent-33)

**Zwei Varianten aus dem Auftrag:**

- **A — immer in Stücken:** `http_flush()` und die Dateiauslieferung rufen `http_client.write()` mit höchstens
  `TCP_MSS` (536) Byte je Aufruf, in einer Schleife bis zum Ende des Puffers.
- **B — erst nach `w=0` kleiner nachversuchen:** wie heute 1'072 Byte; nur wenn der Aufruf nichts unterbringt,
  ein zweiter Versuch mit 536.

**Empfehlung A, mit Begründung am Core (✔ SW):** Bringt `tcp_write()` 1'072 Byte nicht unter (`ERR_MEM`), kehrt
`_write_some()` ohne Fortschritt zurück, und `_write_from_source()` **wartet die volle Frist** auf einen
Rückruf (`ClientContext.h:485-487`) — seit E3 1'000 ms bei API-Antworten, 2'000 ms bei Dateien. Variante B
versucht das kleinere Stück also erst **nach** dieser Frist: `loop()` stünde jedes Mal bis zu einer Sekunde
still, die Brücke würde nicht gelesen (C68), und der Fehlschlag würde heute schon als `w=0` gezählt. Variante A
kostet nichts an Zeit; der Mehraufwand ist ein Schleifendurchlauf und ein `tcp_output()` je 536 Byte statt je
1'072, bei abgeschaltetem Nagle-Verfahren (`setNoDelay(1)`, `http.cpp:16086`) ohnehin zwei Segmente.

**Was A bringt (◐, am Gerät nur mittelbar sichtbar):** Ein Schreibaufruf braucht rund 0,67 statt 1,35 KB freien
Heap (L456). Sind 0,7 KB frei, aber keine 1,4 KB, kommt die Antwort durch statt `w=0`. Liegt der grösste Block
**unter** einem Segment (~0,6 KB, wie bei `heap_low_block` 280 in L450), hilft auch A nicht — dann helfen nur
die Teile, die die Belegung senken (3, 4, 5, 6).

**Festlegungen:**
- Konstante `HTTP_WRITE_CHUNK` = `TCP_MSS` (der Wert kommt aus der Bauvariante, `http.cpp:137-165`);
  `MAX_HTTP_RESPONSE_LEN` und der Lesepuffer der Dateiauslieferung bleiben 1'072 — gelesen wird weiter in
  Blöcken von 1'072 Byte, geschrieben in zwei Stücken.
- **Fortschrittsregel in `http_flush()`:** Ein Stück, das ganz untergebracht ist, führt zum nächsten. Ein Stück,
  das **nichts** unterbringt, beendet den Flush (die Frist ist im Core schon abgelaufen). Ein **teilweise**
  untergebrachtes Stück wird einmal mit dem Rest nachversucht (bisherige Regel „höchstens ein
  Wiederholungsversuch nach Fortschritt", `http.cpp:1444-1448`), danach Ende. Schlimmster Fehlschlag damit
  unverändert ≤ 2 × 1'000 ms plus `abort()` (`http.cpp:193-205`).
- **Zeilen und Zähler unverändert:** eine Zeile `- http write lost` bzw. `- fs write short` je Verbindung, mit
  `blk`/`snd`/`ms` (I-2); `w=` in `fs write short` nennt die von **diesem 1'072-Byte-Block** ausgelieferten
  Byte (0, 536 oder dazwischen). Der Prüfstand c65b bleibt gültig und wird an die Stücke angepasst.
- I-1-Messpunkte unverändert (`flush` vor dem ersten Stück je Aufruf; `fs` am ersten und jedem achten Block).

### 19.5 Backlog nach freiem Heap (ESP) — nur zusammen mit §19.6

**Unterklasse (✔ SW, `WiFiServer.h:71-78` `protected`):**

```
class WcHttpServer : public WiFiServer
{
    public:
        using WiFiServer::WiFiServer;
        uint8_t pending (void) const;          // accepts_pending des Listen-PCB, 0 ohne _listen_pcb
        void    backlog (uint8_t n);           // tcp_backlog_set (_listen_pcb, n), nur mit _listen_pcb
};
```

- `http_server` (`http.cpp:33`) wird eine Instanz davon; `http_server_begin()` (`:16232-16235`) startet mit
  `begin (80, HTTP_BACKLOG_HIGH)`. Ein `#if !TCP_LISTEN_BACKLOG` mit `#error` sichert ab, dass die Bauvariante
  den Backlog kennt (sonst ist `tcp_backlog_set` leer, `lwip/tcp.h:450-454`, und die Massnahme wäre still
  wirkungslos).
- **Nachführung in `http_server_loop()`** vor dem `accept()` (`:15970`): gewünschten Backlog aus
  `ESP.getFreeHeap()` bestimmen und **nur bei einer Änderung** setzen. `getFreeHeap()` ist ein Zählerstand
  (`http.cpp:240`); `getMaxFreeBlockSize()` geht dagegen mit gesperrten Interrupts über den ganzen Heap
  (`:235-239`) und taugt nicht für jeden Schleifendurchlauf.
- **Wirkung (✔ SW):** `tcp_backlog_set` gilt für **neue** SYN; schon wartende Verbindungen bleiben. Bei vollem
  Backlog antwortet lwIP auf ein SYN nicht; die Gegenstelle wiederholt (`WiFiServer.h:56-61`).

**Schwellen (Lead-Empfehlung, Ent-34), gerechnet aus den Posten von L456 (●):**

| Posten | Bedarf |
|---|---|
| bediente Verbindung: Sendepuffer ~1,35 KB + PCB | ~1,6 KB |
| TIME_WAIT nach §19.7 (höchstens 2 × ~192) | ~0,4 KB |
| Strings und `FS()` während einer Antwort | ~0,35 KB |
| **Grundbedarf** | **~2,4 KB** |
| je wartende Verbindung (PCB ~192, `ClientContext` ~56, Anfrage als pbuf ~500–620) | ~0,8 KB |
| Reserve: Haltegrenze K30 (Ent-23) | 1'072 Byte |

Backlog `n` ist zulässig, solange `free ≥ 1'072 + 2'400 + n × 800`:

| freier Heap (`getFreeHeap()`) | Backlog |
|---|---|
| ≥ 5'900 | **3** |
| ≥ 5'100 | **2** |
| darunter | **1** (0 geht nicht: das Makro setzt 0 auf 1, `lwip/tcp.h:447`) |

- **Hysterese 256 Byte nach oben:** Erhöht wird erst bei `free ≥ Schwelle + 256`, gesenkt sofort. Sonst
  pendelte der Backlog bei jeder Antwort.
- **Warum 3 als Obergrenze statt 5:** In Ruhe liegt `free` um 7'280 (L449, M-A); mehr als drei wartende
  Verbindungen gibt es mit der Warteschlange der PWA (höchstens 2) und einem zweiten Gerät im LAN kaum, und 5 × 0,8
  KB sind genau der Posten, der beim Tief fehlte.
- **Die Schwellen sind gerechnet, nicht gemessen.** G8 liest an `heap_low_accepts_pending` und
  `heap_low_backlog` ab, ob sie greifen (§19.13); eine Anpassung nach G8 ist ein eigener Task, kein stilles
  Nachjustieren.

### 19.6 Warteschlange für `/api/*` (PWA)

**Ein Ort:** Alle Abrufe der PWA auf `/api/` laufen über `fetchWithTimeout()` (`app.js:13094-13106`), das
`fetchWithRetry()`, `apiFetch()`, `settleFetchText()` und `settleFetchJson()` gemeinsam benutzen. Die
Warteschlange sitzt dort. **Prüfpunkt E4.7:** Bestandsliste aller `fetch(`- und `XMLHttpRequest`-Stellen in
`app.js` per `grep` im Bericht; jede, die `/api/` anspricht und **nicht** über `fetchWithTimeout()` läuft, wird
umgestellt oder mit Grund ausgenommen (z. B. ein Upload mit Fortschrittsanzeige). Assets und `/app/…` laufen
**nicht** durch die Schlange.

**Regeln:**

| Regel | Festlegung |
|---|---|
| Gleichzeitig unterwegs | höchstens **2** |
| Startphase | höchstens **1**, bis das `load`-Ereignis des Fensters gefeuert hat **und** der erste `loadCoreData()` beendet ist; spätestens 15 s nach dem Start des Skripts endet die Startphase trotzdem (Lead-Empfehlung, Ent-35) |
| Reihenfolge | zwei Klassen: **Schreibende** (alles über `apiFetch()` mit Setter-, Wartungs- oder Upload-Pfad) vor **Lesenden**; innerhalb einer Klasse streng in Ankunftsreihenfolge — ein Backup-Import bleibt in seiner Reihenfolge (Lead-Empfehlung, Ent-35) |
| Überholte Abrufe | Ein Eintrag kann eine Prüfung „überholt?" mitbringen; sie läuft **beim Herausnehmen**, unmittelbar vor dem Senden. Überholt ist ein lesender Abruf eines `loadData()`-Laufs, dessen `requestId` nicht mehr `loadRequestSerial` ist. Er wird **nicht gesendet**; der Aufrufer erhält seinen Rückfallwert bzw. einen Fehler `superseded`. Schreibende Abrufe sind nie überholt |
| Zeitstempel bei überholten Abrufen | `lastUpdateStatusFetchAt` und `lastUpdateTableFilesFetchAt` (`app.js:3322`, `:3339`) werden erst gesetzt, wenn der Abruf **tatsächlich gesendet** wird. Sonst stünde das Hauptmodul nach einem verworfenen Abruf bis zu 5 min ohne diese Daten (Kommentar C35/N1, `:3316-3320`) |
| Netzfrist | beginnt **beim Senden**, nicht beim Einreihen — für den Zeitgeber in `fetchWithTimeout()` **und** für den äusseren in `settleWithTimeout()` (`:13108-13115`), der heute ab dem Aufruf zählt |
| Wartezeit in der Schlange | für **Lesende** höchstens 15 s; danach wird der Eintrag ohne Senden verworfen (Rückfallwert), damit keine Anzeige auf einen Abruf wartet, der nie kommt. **Schreibende** werden nie verworfen — ein Setter wartet, bis er dran ist; jeder Abruf vor ihm hat seine eigene Netzfrist, die Schlange läuft also immer weiter |
| Wiederholung nach Zeitüberschreitung | **entfällt für lesende Abrufe** (Lead-Empfehlung, Ent-35): Ein abgebrochener Abruf liegt beim ESP noch im Backlog; die Wiederholung wäre eine zweite Verbindung für dieselbe Antwort (L456, C53). Wiederholt wird weiter bei `Failed to fetch` und 5xx. Für den Scan gilt §19.3 ohnehin |
| Periodische Abfragen | Jeder Taktgeber (Fortschritt beim Update `:13265`, STM-Flash `:13438`, `stm32_log` `:3602`, Live-Farbe `:15934`) hat höchstens **einen** Abruf in der Schlange oder unterwegs; ein Takt, der einen offenen findet, entfällt |

**Netzfristen (Regel: +1'000 ms für jede lesende Frist, eine SYN-Wiederholung, Lead-Empfehlung, Ent-35):**

| `CONNECTION_STABILITY` (`app.js:1079-1097`) | heute | E4 |
|---|---|---|
| `coreSettingsTimeoutMs` | 3'200 | 4'200 |
| `corePowerTimeoutMs` (mit Ent-30 B ohne eigenen Abruf; bleibt nur, falls noch ein Leser übrig ist) | 2'200 | 3'200 |
| `updateStatusTimeoutMs`, `updateTableInfoTimeoutMs` | 4'500 | 5'500 |
| `networkScanTimeoutMs` | 3'800 | 4'800 |
| `overlayIconsTimeoutMs` | 2'200 | 3'200 |
| `fsInfoTimeoutMs`, `eepromSettingsTimeoutMs` | 5'500 | 6'500 |
| `fsListTimeoutMs` | 6'500 | 7'500 |
| `stm32LogTimeoutMs` | 3'800 | 4'800 |
| `progressPollTimeoutMs` | 1'800 | 2'800 |
| `frameProbeTimeoutMs` | 2'500 | 3'500 |
| `apiWriteTimeoutMs` | 6'500 | **unverändert** |

Ob dadurch eine Prüfung, eine Anzeige oder ein Ablauf (Wiederverbinden nach Neustart, Fortschritt eines OTA)
anders taktet, steht als Tabelle je Konstante im Taskbericht E4.7.

**Displaystrom in der Kernladung (M-2) — entschieden: Ent-30 B (Nutzer, 14.10.2026):**
- **Umzusetzen ist B:** gar kein eigener Abruf; beide Werte aus `settings_xml` (`numvar` 3 und 30), das ohnehin
  geladen wird und dasselbe Feld `numvars[]` ausgibt wie `display_power` (✔ SW, `http.cpp:8757-8761` gegen
  `:8945`). `loadCoreData()` (`app.js:3227-3240`) ruft nur noch `settings_xml` — eine Verbindung statt drei. Der
  Kommentar B11 (`app.js:11100-11106`) sagt genau das.
- *Verworfen im Entwurf (Variante A, Auftrag):* `power_status` statt `display_power` und `ambilight_power` — zwei
  Verbindungen statt drei. Die Fähigkeit `power_status_api_supported` steht nur in `update_status`, das später
  kommt.
- **Rückfall nie „aus":** Fehlt ein Wert, gilt nie ein fester Wert `"off"`. Heute zeigt ein abgelaufener Abruf
  still „aus" (§19.0) — das ist der Fehler, vor dem §19.1 warnt, und er wird in E4 auch unabhängig vom Backlog
  behoben. Der Kommentar B11 wird nachgeführt.

**`loadSecondaryData` in der Ladesperre:** `loadData()` gibt `activeLoadCount` (`:3150`) erst frei, wenn auch
`loadSecondaryData()` beendet ist (`await` statt `void`, `:3137`). Die erste Darstellung wartet darauf nicht —
sie ist zu diesem Zeitpunkt schon gezeichnet. Folge, gewollt: Die Selbstaktualisierung (`:3036`) beginnt keinen
neuen Lauf, solange Nebenabrufe des vorigen unterwegs sind; der Knopf „WLANs neu laden" meldet Erfolg erst nach
dem Scan.

**Modulwechsel:** Er bleibt ohne eigene Sperre; die Warteschlange (höchstens 2) und das Verwerfen überholter
Abrufe begrenzen die Last, die ein schneller Wechsel erzeugt (L456, „Modulwechsel ohne Sperre").

### 19.7 TIME_WAIT begrenzen (ESP) — Grenze 2 (Lead-Empfehlung, Ent-36)

- **Mittel (✔ SW):** Das Makro des Cores `TCP_TW_LIMIT(l)` (`lwipopts.h:3678-3689`) zählt `tcp_tw_pcbs` und
  ruft `tcp_kill_timewait()` für die ältesten, bis höchstens `l` übrig sind. Es braucht `tcp_tw_pcbs` aus
  `lwip/priv/tcp_priv.h` — derselbe private Kopf wie I-4 (Ent-32).
- **Ort:** eine Funktion `tcp_tw_limit()` in `http.cpp` (Prototyp im gemeinsamen Kopf `base.h`), aufgerufen
  (a) in `http_server_loop()` unmittelbar nach einem erfolgreichen `accept()`, **vor** dem Messpunkt `accept`,
  und (b) in `httpclient()` unmittelbar **vor** `connect()` (`httpclient.cpp`). Beides sind die Stellen, an denen
  gleich neuer Speicher gebraucht wird; I-1 sieht danach den Stand ohne die abgeräumten PCBs.
- **Grenze 2:** Der ESP baut als Server zuerst ab und hält damit nach jeder Antwort ein PCB in TIME_WAIT. Zwei
  decken die zuletzt bedienten Verbindungen; jede weitere kostet ~192 Byte (●) und schützt nur vor verspäteten
  Segmenten einer Verbindung, deren Gegenstelle schon geantwortet hat. Gewinn gegenüber der Core-Grenze 5: bis
  ~0,6 KB.
- **Risiko (benannt, ◐):** Ein verspätetes Segment der abgeräumten Verbindung wird mit RST beantwortet; die
  Gegenstelle hat ihre Seite schon geschlossen. Verwendet ein Client dieselbe Portnummer innerhalb von 2 MSL
  wieder, könnte ein altes Segment in die neue Verbindung fallen — bei Browsern mit fortlaufenden Ports
  vernachlässigbar.
- **Link-Prüfpunkt (◐):** Dass `tcp_kill_timewait` aus `liblwip2-536-feat.a` gelinkt werden kann, zeigt erst der
  Build nach E4.4. Scheitert er, entfällt Teil 5 aus E4 mit Befund (C65) — **kein** Nachbau der Funktion.

### 19.8 Verwaiste lesende `/api/`-Abfragen überspringen (ESP, Nutzerentscheid)

- **Lage (✔ SW):** Bricht die PWA einen Abruf ab, während er beim ESP im Backlog liegt, bleibt die Anfrage im
  Empfangspuffer des Kontexts; `http_server_loop()` liest sie später und führt sie aus (C53), die Antwort erreicht
  niemanden (`http.cpp:16025-16028`). Für einen Scan sind das 2,2 s Stillstand und `80 × n` Byte für nichts.
- **Regel:** In `http()` (GET-Zweig, `http.cpp:16201`) vor dem Verteiler: Steht der Pfad in der **Weissliste
  der lesenden Endpunkte** und meldet `http_client.connected ()` 0, wird der Handler **nicht** ausgeführt, nichts
  gesendet, `api_orphan_skipped` +1 (sättigend, `uint16_t`), und die Verbindung wird wie gewohnt beendet. Keine
  Logzeile je Fall (UART-Last, wie die Zähler `no_request_*`, `http.cpp:181-189`).
- **Warum `connected () == 0` hier genügt (✔ SW):** Auch der Fall „Daten im Puffer, Gegenstelle hat halb
  geschlossen" (CLOSE_WAIT, C53) ist für eine Antwort verloren — der Core schreibt dort nichts mehr. Eine
  lesende Antwort zu überspringen, verliert also nichts, was ankäme.
- **Setter, Wartung, Auslöser:** stehen **nicht** in der Liste und werden wie heute immer ausgeführt (C53 bleibt
  für sie) — der Nutzer hat gedrückt, der Abbau sagt nichts darüber, ob er die Wirkung will.
- **Weissliste, nicht Muster:** eine Tabelle vollständiger Pfade im Flash (`PROGMEM`, Vergleich mit
  `strcmp_P`). Kein Schluss aus dem Namen („endet nicht auf `_set`"): Ein künftiger Endpunkt ist **nicht**
  lesend, bis ihn jemand eingetragen hat.

**Weissliste (Empfehlung; E4.5 prüft jeden Handler auf Nebenwirkungen und meldet Abweichungen):**

| Aufgenommen | Bemerkung |
|---|---|
| `settings_xml`, `display_power`, `ambilight_power`, `power_status` | reine Ausgabe |
| `network_scan` | grösster Einzelgewinn (Scan) |
| `eeprom_settings`, `network_secrets` | reine Ausgabe |
| `update_status`, `update_table_files` | Nebenwirkung nur Zwischenspeicher bzw. Auffrischungsauftrag (§18.5, §18.5a) — Überspringen verschiebt sie, verliert nichts; grösster Gewinn bei kaltem Aufruf (ausgehende Abrufe) |
| `update_progress`, `stm32_log`, `overlay_icons` | reine Ausgabe |
| `fs_info`, `fs_list`, `fs_show` | lesen das LittleFS |
| `ir_codes_get` | Prüfpunkt E4.5: liest nur den Zwischenstand, löst nichts beim STM aus |

| Ausgenommen | Grund |
|---|---|
| `device_ready` | Messinstrument; mit `heap_low_reset=1` setzt es Diagnose zurück — ein übersprungener Rücksetzer verfälschte das nächste Messfenster. Billig |
| `reconnect_probe` | Wiederverbindungsprobe, billig; ihr Sinn ist „antwortet der ESP" |
| `network_get_time`, `ir_codes_request`, `weather_get_now`, `weather_get_forecast`, `temperature_display`, `overlay_display`, `dfplayer_play`, `test_display`, `learn_ir` | lösen etwas aus |
| alle `*_set`, `*_default`, `maintenance_*`, `fs_remove`, `fs_upload_*`, `app_file_upload`, `update_download_*`, `remote_*`, `local_*`, `stm32_log_clear`, `live_display_color`, `overlay_delete`, `network_wps`, `datetime_set` | schreiben oder lösen aus |

### 19.9 RAM-Budget (AKV.12)

| Teil | statisch | Bemerkung |
|---|---|---|
| I-4 Momentaufnahme | ~48 | 1 + 1 + 11 + 1 + 2 + 32 |
| `http_cur_path` (Zeiger) | 4 | statt einer zweiten Kopie von 32 Byte |
| Backlog-Zustand | ~2 | |
| `api_orphan_skipped` | 2 | |
| Weissliste | 0 | im Flash |
| Unterklasse `WcHttpServer` | 0 | gleiche Grösse wie `WiFiServer` |
| Scan-Zeile | 0 | Stackpuffer |
| **Summe** | **~56** | **Grenze 200 Byte „Globale Variablen" über ganz E4** — darüber anhalten und vorlegen |

Gemessen wird nach **jedem** ESP-Task (Ausgang: Build E3, 68'992 Byte, L450). Die Teile 3, 5 und 6 **senken**
die vorübergehende Belegung um bis zu ~2,4 KB (Backlog 5 → 1 unter Last), ~0,6 KB (TIME_WAIT) bzw. eine ganze
Antwort je übersprungener Abfrage — das zeigt G8, nicht die Rechnung. **PWA:** Grösse von `app.js.gz` vorher und
nachher im Messprotokoll (kein Gate; das LittleFS hat rund das Vierfache der grössten Datei frei, L450).

### 19.10 Werkzeuge (Lead)

- **`tools/check-pwa.mjs`** (`check-pwa.sh`): Die festen Wartezeiten — 2'500 ms nach dem Laden (`:79`), 600 ms
  nach dem Öffnen der Navigation (`:133`), 4'000 ms nach jedem Neuladen (`:210`) — werden durch **Warten auf
  Netzruhe** ersetzt: kein Abruf an die Uhr unterwegs seit 1'500 ms (über `Network.requestWillBeSent`,
  `loadingFinished`, `loadingFailed`), höchstens 20 s, sonst Fehlschlag „keine Netzruhe nach 20 s" mit der Liste
  der offenen Abrufe. Die 600 ms werden zu „warten, bis das Element sichtbar ist", höchstens 3 s. Mit einer
  SYN-Verzögerung von ~1 s und der Startphase der Warteschlange (1 gleichzeitig) reichen die festen Zeiten nicht
  mehr.
- **Dazu misst `check-pwa` (neu, Ausgabe je Lauf):** höchstens gleichzeitig unterwegs gewesene `/api/`-Abrufe,
  getrennt für Startphase und danach; Zahl der Abrufe von `network_scan`, `display_power`, `ambilight_power`,
  `power_status`; Zahl der Verbindungen, deren Aufbau ≥ 900 ms dauerte (Zeitfelder `connectStart`/`connectEnd`
  — die Spur eines verworfenen SYN). Das ist das Instrument für AKV.19 (b) und (d) am Gerät.
- **Gegenprobe (DIR-014):** gegen die Vorschau (`tools/preview/server.py`) mit künstlicher Verzögerung von
  1'200 ms auf jedem `/api/`-Abruf: alte Fassung schlägt fehl (feste 2,5 s), neue besteht; dazu ein Abruf, der nie
  endet ⇒ „keine Netzruhe nach 20 s", Exit ≠ 0, und die Meldung erscheint in der Ausgabe.
- **`tools/smoke-device.sh`:** ab ESP 3.2.37 verlangt er in `device_ready` die Felder aus §19.2 samt
  `http_backlog` und `api_orphan_skipped`, setzt nichts zurück, prüft `heap_low_pcb_states` auf genau 11
  Zahlen. Gegenprobe über `SMOKE_DR_OVERRIDE`: ein Feld fehlt ⇒ FEHLT; 10 statt 11 Zahlen ⇒ FEHLT; vollständig ⇒
  OK; gegen 3.2.36 ⇒ „gibt es noch nicht".
- **`tools/watch-log.sh`:** `- scan …` als **Hinweis** (keine Alarmzeile: der Scan ist gewollt); Gegenprobe über
  `WATCHLOG_QUELLE`.

### 19.11 Prüfstände (Lead legt ab; der Umsetzer baut sie im Scratchpad)

Alle mit Fallzahl, gegen das gezählt, was es geben müsste (L235); **einmal gegen den Vorgängerstand
fehlgeschlagen** (ESP 3.2.36, PWA 1.4.99) und **je Massnahme mindestens eine Sabotage**, die anschlägt
(DIR-014). Ablage `tools/checks/auszug/esp/c65/` bzw. für die PWA neben b48; Aufnahme in S16.

| Prüfstand | Gegenstand | Fälle (Auszug) | Sabotage ⇒ FEHL |
|---|---|---|---|
| **c65i** | I-4 und Scan-Zeile | Attrappen für `tcp_active_pcbs` (gemischte Zustände), `tcp_tw_pcbs`, Listen-PCB mit `accepts_pending`; Momentaufnahme **nur** beim neuen Block-Tiefstwert (gleich und höher ⇒ unverändert); Pfad ohne Query, 31 Zeichen, `-` ohne Anfrage; `unacked` −1 ohne Verbindung; Rücksetzen mit I-1, `http_backlog`/`api_orphan_skipped` bleiben; Feldreihenfolge in `device_ready` hinter `upd_cache_max_ms`; Scan-Zeile: Format, ≤ 119 Zeichen, Mitschnitt und Ring textgleich, `flush` vor `scanDelete` | Momentaufnahme bei jedem Aufruf; Query im Pfad; Scan-Zeile nur auf `Serial` |
| **c65w** | Schreibstücke | `WiFiClient`-Attrappe zeichnet jede Schreiblänge auf: in `http_flush()` und der Dateiauslieferung **nie** mehr als `TCP_MSS` je Aufruf; ausgelieferte Byte gleich Eingabe; Fortschrittsregel (ganz, nichts, teilweise); c65b bleibt grün | 1'072 je Aufruf; Teilstück ohne Nachversuch |
| **c65k** | Backlog | Attrappe für `getFreeHeap()` und `tcp_backlog_set`; Schwellen 5'900/5'100 samt Hysterese 256 (auf- und absteigende Folgen); gesetzt nur bei Änderung; Aufruf in `http_server_loop()` vor `accept()`; statisch `begin (80, HTTP_BACKLOG_HIGH)` und `#error` ohne `TCP_LISTEN_BACKLOG` | Hysterese entfernt (Pendeln wird gezählt); Nachführung entfernt |
| **c65z** | TIME_WAIT | Attrappe der TW-Liste mit 0–5 Einträgen: nach `accept()` und vor `connect()` höchstens 2, die **ältesten** entfernt; statisch genau zwei Aufrufstellen | Grenze 5; Aufruf nach dem Messpunkt |
| **c65o** | verwaiste Lesende | Weissliste gegen den Verteiler: jeder Eintrag existiert im Verteiler; kein Eintrag auf `_set`, `_default` oder aus der Ausnahmeliste §19.8; Tabelle statt Muster (statisch); `connected () == 0` ⇒ Lesender übersprungen, Zähler +1, nichts gesendet; Setter mit `connected () == 0` ⇒ ausgeführt (C53); `connected () == 1` ⇒ ausgeführt | `ldr_min_set` in der Liste; Prüfung nach statt vor dem Verteiler |
| **c65q** (PWA) | Warteschlange, Scan, Strom, Sperre, Fristen | Vorschau mit steuerbarer Verzögerung, Headless-Chrome mit Zählung über CDP: nie mehr als 2 `/api/` gleichzeitig, in der Startphase 1; Schreibende vor Lesenden, je Klasse in Reihenfolge; überholter Abruf wird nicht gesendet und setzt keinen Zeitstempel; Lesender nach 15 s Wartezeit verworfen, Schreibender nie; Frist ab Senden (Abruf mit 10 s Wartezeit und 1 s Netz besteht); `network_scan` 0-mal in 4 Takten `auto`, 1-mal beim Öffnen, 1-mal je Knopf, ohne Wiederholung nach Ablauf; `display_power`/`ambilight_power` 0-mal; Rückfall nie `"off"`; keine neue Selbstaktualisierung, solange Nebenabrufe laufen; `CONNECTION_STABILITY` gegen die Tabelle §19.6 | Grenze 3; Frist ab Aufruf; Scan im `auto`; Rückfall `"off"` |

### 19.12 Releaseschnitt, Einspielen, Rückweg

**Ein** Release für ESP und PWA, **ein** `make release-zip`, **ein** Commit, **ein** Tag; Versionen (setzt der
`release-engineer`, R4): **ESP 3.2.37**, **PWA 1.4.100**, `CACHE_NAME` mit. Tag erwartet
`release/3.2.34-3.2.37-1.4.100`.

**Welche Paarungen sind sicher:**

| ESP | PWA | Lage |
|---|---|---|
| 3.2.36 | 1.4.99 | heutiger Stand |
| 3.2.36 | **1.4.100** | **sicher** — die neue PWA braucht nichts Neues vom ESP; `settings_xml` gibt es dort |
| **3.2.37** | 1.4.99 | **unsicher** — kleiner Backlog trifft drei gleichzeitige Abrufe mit 2'200 ms Frist und Rückfall „aus": die Oberfläche kann das Display als **aus** zeigen, ohne dass es aus ist; wer dann „einschalten" drückt, schaltet nichts Sinnvolles |
| 3.2.37 | 1.4.100 | Ziel |

**Einspielreihenfolge — entschieden: Ent-29 A (Nutzer, 14.10.2026): PWA zuerst, dann ESP, unmittelbar
hintereinander, dazwischen nichts.** So entsteht die unsichere Paarung nie. Das widerspricht DIR-002 nicht: Die
Regel „Firmware zuerst" gilt, **wenn eine PWA-Änderung eine Firmware-Änderung voraussetzt** — hier ist es
umgekehrt, der ESP setzt die PWA voraus. Ablauf: Wache läuft; `install-app.sh`, `install-app.sh --check`;
**sofort** ESP-OTA; danach Smoketest samt Update-Quelle, `install-app.sh --check` (DIR-017), `check-pwa.sh`.
*Verworfen im Entwurf (B, Auftrag):* ESP zuerst, dann PWA, wie EP — die unsichere Paarung bestünde dann für die
Minuten zwischen OTA und Upload.

**Rückweg:** Die Regel lautet **„neuer ESP nie mit alter PWA"**.
- **Upload der PWA scheitert:** kein OTA — es bleibt bei 3.2.36 mit 1.4.99, nichts ist passiert.
- **ESP 3.2.37 verhält sich falsch:** ESP zurück auf 3.2.36; die PWA 1.4.100 **bleibt** (sichere Paarung). Den Weg
  für den Rückflash legt der Lead in den Einspielzeilen fest (E4.15) — Quelle ist die Firmware aus dem
  Release-ZIP von E3 (`wordclock-release-2026-10-14-153447.zip`, L450); die Zeile nennt Befehl und
  Reihenfolge, bevor das OTA läuft.
- **PWA 1.4.100 verhält sich falsch:** **erst** den ESP auf 3.2.36 zurück, **dann** die PWA 1.4.99 aus dem Tag
  `release/3.2.34-3.2.36-1.4.99` über `install-app.sh`. Umgekehrt entstünde die unsichere Paarung.
- Die Legacy-Oberfläche bleibt in jedem Zustand bedienbar und ist der Rückweg für Display und STM-Reset.

### 19.13 Gerätenachweis G8 — Wirkung belegen, ohne die Uhr zu gefährden

**Freigabe:** lesend unter **Nutzerentscheid N-5, 13.10.2026, Gespräch**; der Scan ist lesend (er schaltet
nichts), ebenso die curl-Probe in G8.6. **Haltegrenze K30 nach Ent-23** — Anhalten bei Ruhewert < 5'120 oder
Tiefstwert < 1'072.

**Wie G8 trotz Haltegrenze misst — entschieden: Ent-31 (Nutzer, 14.10.2026): erste Ladung messen, dann Tor.**
Der **erste** `check-pwa` nach dem Einspielen **ist** die Messung; K30 gilt **danach** als Tor für jede weitere
Ladung. Unterschreitet der erste Lauf 1'072, endet G8 sofort — keine zweite Ladung, Bericht, und der Lead fragt
den Nutzer. Begründung: Ohne eine Ladung gibt es nichts zu messen; zwei Ladungen mit Tiefstwerten von 400 und
280 Byte haben die Uhr in L450 ohne Neustart überstanden, die Gefahr liegt in der **Wiederholung**, nicht im
einen Lauf. Die Logwache läuft (DIR-013), und jede Ladung wird sofort ausgewertet, bevor die nächste beginnt.

**Ablauf** (Dauer rund 20 min; bei Planung `caffeinate`, DIR-020):

| Schritt | Was | Ins Messprotokoll |
|---|---|---|
| G8.0 | Einspielen nach Ent-29 (E4.17); Smoketest mit den neuen Feldern, `install-app.sh --check`, gemeldete Versionen; `http_backlog` in Ruhe = 3 | Uhrzeiten, Ergebnisse |
| G8.1 | **Ruhe:** 150 s ohne Anfrage, dann `device_ready?heap_low_reset=1` | Heap-Zeilen samt `low=`, `free` in Ruhe |
| G8.2 | **Erster Lauf:** ein `check-pwa.sh` (neue Fassung, Startmodul Hauptmodul), 15 s nach dem Ende `device_ready?heap_low_reset=1` | alle I-1-, I-3- und I-4-Felder, Δ`write_lost_*`, Δ`fs_write_short`, Δ`api_orphan_skipped`, `low=` der Heap-Zeilen im Fenster; aus `check-pwa`: Höchstzahl gleichzeitiger `/api/`, Abrufe je Endpunkt, langsame Verbindungsaufbauten; aus dem Mitschnitt: `keine Quittung`, `http write lost`, `fs write short`, `- scan` |
| **Tor** | `heap_low_block` < 1'072 **oder** `low=` < 1'072 ⇒ **Ende von G8**, Nutzer fragen (Ent-31). Sonst weiter | Entscheidung mit Uhrzeit |
| G8.3 | **Ruhe nach Last:** 150 s ohne Anfrage | `max` der Heap-Zeile gegen 5'120 (K30), 5'808, 7'264 |
| G8.4 | **Zwei weitere Läufe** wie G8.2, je mit Tor davor und danach | wie G8.2 |
| G8.5 | **Scan (Teil 1, I-4):** Werkzeug öffnet das Netzwerkmodul und hält es 50 s offen (≥ 3 Takte der Selbstaktualisierung), drückt dann einmal „WLANs neu laden" | Zahl `network_scan`-Abrufe (Soll 1 + 1), Zahl `- scan`-Zeilen (Soll 2), deren Werte — **ms Scan gegen ms Ausgabe (C68)** |
| G8.6 | **Verwaiste Lesende (Teil 6):** `curl -m 4 …/api/network_scan` und 0,3 s danach `curl -m 0.5 …/api/settings_xml`; dann dieselbe Folge mit `-m 8` für den zweiten | Δ`api_orphan_skipped` = 1 im ersten, 0 im zweiten Fall; der zweite bekommt seine Antwort |
| G8.7 | **Gesundheit** über das ganze Fenster | Exceptions, Neustarts, `diag` lückenlos, `ovr`, `watch-log.sh --pruefe` |

**Wirkung je Massnahme — was am Gerät trennbar ist und was nicht:**

| Teil | Am Gerät abgelesen an | trennbar? |
|---|---|---|
| 1 Scan | Zahl der Abrufe und `- scan`-Zeilen in G8.5 | **ja** |
| 2 Schreibstücke | keine `fs write short` und keine `http write lost` mit `fail` bei `blk` ≥ 600 | **nein**, nur mittelbar — belegt am Prüfstand c65w |
| 3 Backlog | `heap_low_accepts_pending` ≤ `heap_low_backlog` beim Tief; `http_backlog` in Ruhe 3; langsame Verbindungsaufbauten aus `check-pwa` | **teilweise** — die Wirkung auf das Tief überlagert sich mit Teil 4 |
| 4 Warteschlange | Höchstzahl gleichzeitiger `/api/` (≤ 2, Startphase ≤ 1), `display_power`/`ambilight_power` 0 | **ja** (Verhalten der PWA); die Wirkung auf den Heap nur zusammen mit 3 |
| 5 TIME_WAIT | `heap_low_tw` ≤ 2 in jedem Lauf | **ja** (Zustand), Heap-Anteil ● |
| 6 Verwaiste Lesende | Δ`api_orphan_skipped` in G8.6 | **ja** |

**Nicht** vorgesehen: einzelne Teile zur Laufzeit abschaltbar zu machen, um sie am Gerät zu trennen
(verworfen, §19.17). Die Trennung leistet I-4; was dort nicht trennbar ist, steht hier als „nein" bzw.
„teilweise" und wird nicht behauptet.

**Ziel von G8** (AKV.17): in **allen drei** Läufen `heap_low_block` ≥ 1'072 und `low=` ≥ 1'072; Ruhe nach Last
`max` ≥ 5'120; Δ`fs_write_short` = 0, Δ`write_lost_failed` = 0; 0 × `var_send_buf: keine Quittung`. **Nicht
erreicht** ⇒ C65 bleibt offen mit Befund; T2 nur nach Entscheidung des Nutzers. Die nächsten Kandidaten sind
dann C70 (Asset-Cache) und C67 (`FS()` ohne Heap) — mit den I-4-Werten als Begründung.

### 19.14 Danach: G7-Rest und T2

**G7R — der Rest von G7 auf dem Stand E4** (E4.20, erst nach bestandenem G8): G7 wurde nach zwei Ladungen
angehalten (L450). Auf dem Stand 3.2.37/1.4.100 laufen noch: G7.1 (Ruhe), **G7.2** (drei Paare kalt/warm,
AKR.13 — die Zuordnung von M-1 bleibt auch im neuen Stand über kalt gegen warm gültig, Sollwert
Δ`httpclient_connects` kalt 6), G7.3 (alte Messweise, AKR.14 — berichtet), G7.5 (Vorhersage während einer
Ladung, AKR.16), G7.6 (Ruhe nach Last, AKR.12 ii). **G7.4 (Zero-Window-Probe) entfällt — entschieden: Ent-39
(Nutzer, 14.10.2026).** Von diesem Mac aus ist sie nicht herstellbar (L450, `net.inet.tcp.doautorcvbuf`); M-3
bleibt an c65t/c65b und an `ms=` in echten Schreibfehlerzeilen belegt. K30 gilt vor jeder Ladung.

**T2** danach wie geplant (AKZ.6): Phasen 0–4 und 9, **AKL.4** und die Setter-Phasen aus L449 (Phase 3 Rest,
E1–E8), AKR.21; K30 nach Ent-23. Neu im Bericht von T2: die I-4-Felder bei jedem Halt an K30.

### 19.15 Nicht in E4

| Ausgeschlossen | Grund |
|---|---|
| **C70** Asset-Cache (versionierte URLs, `ETag`/`304`) | Nutzerentscheid 14.10.2026 (L456): nicht in E4. Folgebefund C70 besteht. Nach G8 mit den I-4-Werten neu bewerten |
| **C67** `FS()` ohne Heap | Folgebefund C67 besteht (L453); Beitrag ungemessen |
| **C66** Teildatei in `download_file_to_local()` | Folgebefund C66 besteht (L452, hoch); eigener Weg, ändert die Last nicht |
| **C68** Brücke während langer Antworten lesen | Folgebefund; E4 **misst** über die Scan-Zeile, ob die Zeit im Scan oder im Schreiben liegt, baut aber nichts um |
| **C69** gekürzte Notes bei Lesefehler | Folgebefund, ohne Bezug zur Last |
| Ursache der bleibenden Zerstückelung (M-A, Ruhewert 5'808 statt 7'744) | ohne Beleg; I-4 und G8.3 liefern neue Werte |
| Teile zur Laufzeit abschaltbar | §19.17 |
| Wiederholung schreibender Abrufe, Ablauf des Backup-Imports | unverändert; der Import läuft in der Schlange in seiner Reihenfolge |
| Legacy-Oberfläche | unverändert; sie nutzt die Warteschlange nicht und bleibt Stabilitäts-Referenz. Sie trifft auf denselben Backlog — ihre Seiten laden einzeln, ohne Parallelabrufe (◐) |

### 19.16 Architektur-Checkliste — je Teil beantwortet

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Werden die Wetter-Endpunkte eingehalten? Bleibt die
Restore-Bedingung vollständig? Greift die Änderung an der richtigen Schicht an?

> **I-4:** reine Diagnose am ESP, an dem Ort, an dem das Tief schon gemessen wird (`heap_low_note()`); kein
> neuer Endpunkt, Felder hinten an `device_ready`. **Scan:** Die Entscheidung, **wann** gescannt wird, gehört
> zur Oberfläche — dort setzt Teil 1 an; der ESP-Handler bleibt, wie er ist. **Schreibstücke:** setzt in der
> eigenen Schreibschicht (`http_flush()`, Dateiauslieferung) an, nicht im Core — ein Core-Patch würde mit
> jedem Core-Wechsel verloren gehen. **Backlog:** am Server-Objekt über eine Unterklasse, mit dem Mittel, das
> lwIP dafür vorsieht. **Warteschlange:** an **einer** Stelle der PWA, durch die alle `/api/`-Abrufe gehen; die
> Legacy-Oberfläche bleibt parallel und unberührt. **TIME_WAIT:** über das Makro des Cores, nicht über eigene
> Listenpflege. **Verwaiste Lesende:** im Verteiler des ESP, vor dem Handler, über eine Weissliste. **Wetter:**
> kein Teil berührt `/api/weather_get_now` oder `/api/weather_get_forecast`; sie stehen bewusst **nicht** in der
> Weissliste (Auslöser). **Restore-Bedingung:** STM unberührt. **`.gz` und Grösse > 0:** Die
> Dateiauslieferung bleibt `.gz`-only mit Grössenprüfung (`http.cpp:1611`), nur das Schreiben wird gestückelt.

**Scalable systems** — Wie viele STM-Kommandos erzeugt die Aktion? Wie viele Byte pro Minute zusätzlich auf der
UART? Bleibt jeder Pfad unter 20 s Watchdog? Wie lange blockiert der Hauptloop?

> **STM-Kommandos:** keine, in keinem Teil. **UART:** neu ist nur die Scan-Zeile, rund 90 Byte je Öffnen des
> Netzwerkmoduls oder Knopfdruck — vorher lief der Scan alle 15 s ohne Zeile; verwaiste Abfragen zählen ohne
> Zeile. **Hauptloop:** Teil 1 nimmt den Scan (2,2 s Stillstand, L456) aus dem 15-s-Takt; Teil 6 überspringt
> Scans und kalte `update_status`, die niemand mehr abholt; Teil 2 verhindert, dass ein Speicherengpass bei
> 1'072 Byte die volle Frist wartet, wo 536 Byte gepasst hätten; schlimmster Fehlschlag unverändert ≤ 2 ×
> 1'000 ms plus `abort()`. Die Backlog-Nachführung ist ein Zählerstand je Durchlauf, die TIME_WAIT-Grenze ein
> Gang über höchstens fünf Einträge je Annahme. **Watchdog:** keine neue Schleife ohne Grenze; die
> Stückelschleife endet spätestens nach dem Puffer. **Feste Grenzen:** Backlog 1–3, TIME_WAIT 2, PWA 2
> gleichzeitig (Startphase 1), Wartezeit Lesender 15 s; `heap_low_req_path` 31 Zeichen. **PWA-Polling:** sinkt
> — kein Scan im Takt, eine statt drei Verbindungen je Kernladung (Ent-30 B), keine überholten Abrufe.

**Secure by design** — `escapeHtml` bei jedem `innerHTML`? Fremddaten als nicht vertrauenswürdig behandelt? Keine
Credentials in Logausgaben?

> **Credentials:** `heap_low_req_path` trägt **nie** die Query (Schnitt bei `?`, wie I-3) — dort können SSID und
> Schlüssel stehen (`network_client_set`). Die Scan-Zeile nennt nur Zahlen, keine SSID. **Weissliste:**
> `network_secrets` steht darin — übersprungen wird nur, wenn niemand mehr zuhört; ausgegeben wird dabei
> **nichts**, also auch kein Schlüssel. **Fremddaten:** Die Scan-Ergebnisse (fremde SSIDs) werden weiter wie bisher
> maskiert (`HTTP_ESCAPE_SCAN`); die PWA ändert an ihrer Darstellung nichts. **Neue Angriffsfläche:** keine — kein
> neuer Endpunkt, kein neuer Parameter; `device_ready` liefert nur Zahlen und einen Pfad ohne Query. Eine fremde
> Seite kann durch abgebrochene Abrufe höchstens erreichen, dass ein **lesender** Handler nicht läuft — das ist
> das gewollte Verhalten. **Zerstörende Wege:** weiter ungeschützt (Ent-18, eigene Spec); E4 ändert daran nichts.

**Stable & reliable** — Wird jeder Fehler ausgewertet? Leere `catch`? Stille Verwerfungen? Ist der Zustand nach
Abbruch mitten in einer Sequenz definiert? Wird ein gesetztes Flag auf jedem Pfad wieder aufgelöst?

> **Stille Verwerfungen:** Ein verworfener SYN (Backlog) ist für den Client **keine** stille Verwerfung — er
> wiederholt, und die Netzfristen der PWA tragen eine Wiederholung (§19.6). Überholte und zu lange wartende
> Lesende werden mit Rückfallwert beendet, nie Schreibende. Übersprungene verwaiste Lesende werden **gezählt**
> (`api_orphan_skipped`). Der Rückfall „aus" bei `display_power` — heute eine stille Falschanzeige — entfällt.
> **Abbruch in einer Sequenz:** Die Warteschlange hält die Reihenfolge der Schreibenden (Backup-Import);
> verworfen wird nur, was nie gesendet wurde. Ein Stück, das teilweise geschrieben ist, wird wie bisher einmal
> nachversucht, danach `abort()` (RST statt scheinbar vollständiger Antwort). **Flags auf jedem Pfad:**
> `http_cur_path` wird im Destruktor von `HttpRequestTimer` gelöscht, also auf jedem Rückkehrpfad;
> `activeLoadCount` wird im `finally` freigegeben, auch wenn `loadSecondaryData()` wirft; ein Slot der
> Warteschlange wird in `finally` freigegeben, auch bei Abbruch und Fehler. **Neue Fehlerquelle, benannt:**
> Gelingt der Link von `tcp_kill_timewait` nicht, fällt Teil 5 heraus — sichtbar am Build, nicht still
> (§19.7). Fehlt `TCP_LISTEN_BACKLOG`, bricht der Build ab (`#error`), statt einen wirkungslosen Backlog
> auszuliefern.

### 19.17 Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| Backlog fest auf 2 (`begin (80, 2)`, Stand L449) | bei viel freiem Heap unnötig streng (mehr verworfene SYN), bei wenig zu grosszügig; der Backlog nach Heap deckt beide Lagen |
| Backlog nach `getMaxFreeBlockSize()` | geht je Aufruf mit gesperrten Interrupts über den ganzen Heap (`http.cpp:235-239`) — nicht in jedem Schleifendurchlauf |
| Schreibstücke Variante B (erst nach `w=0` kleiner) | kostet je Fall die volle Frist von 1'000 bzw. 2'000 ms (§19.4) |
| `ClientContext` im Core patchen | geht mit jedem Core-Wechsel verloren und ändert das Verhalten für jeden Client des Cores, auch den Wetterabruf |
| TIME_WAIT über eine eigene Kopie der lwIP-Listenpflege | das Makro des Cores tut es schon; eine zweite Pflege derselben Liste ist eine zweite Wahrheit |
| `power_status` statt zweier Abrufe (Ent-30 A) | Nutzerentscheid Ent-30 B: Werte aus `settings_xml`, eine Verbindung statt zwei |
| Verwaiste Abfragen nach Muster (`! endsWith ("_set")`) | ein künftiger auslösender Endpunkt ohne `_set` würde still übersprungen (Nutzerentscheid: Weissliste) |
| Auch verwaiste **Setter** überspringen | Nutzerentscheid: Setter und Wartung immer ausführen (C53) |
| Teile zur Laufzeit abschaltbar (Diagnoseparameter) für eine saubere Trennung in G8 | neuer Zustand im Produktivbetrieb, neuer Parameter an `device_ready`, eine Prüfung mehr; I-4 trennt, was zu trennen ist |
| Zwei Releases (erst ESP mit I-4, dann der Rest) | ein ESP-Release mehr; die Kopplung von 3 an 4 bliebe, und I-4 allein hebt das Tief nicht |
| Wiederholung lesender Abrufe nach Zeitüberschreitung beibehalten | die abgebrochene Anfrage liegt beim ESP noch im Backlog; die Wiederholung ist eine zweite Verbindung für dieselbe Antwort (L456) |
| ESP zuerst, dann PWA (Ent-29 B) | Nutzerentscheid Ent-29 A: die unsichere Paarung 3.2.37/1.4.99 entsteht so nie |
| Zero-Window-Probe in G7R mit `sysctl` (Ent-39 B) | Nutzerentscheid Ent-39: entfällt |

### 19.18 Versionsfolgen

- [ ] ESP `version.h` anheben — **E4: 3.2.37**
- [ ] App `APP_VERSION` anheben — **E4: 1.4.100**
- [ ] `CACHE_NAME` in `sw.js` anheben — **E4**, mit `APP_VERSION`
- [ ] STM — **E4 nicht**

Nur der `release-engineer` führt das aus (R4).

### 19.19 Betroffene Module (E4)

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `ESP8266/ESP-uclock/http.cpp` (UTF-8, Zeilenende vor dem Patch feststellen) | I-4, Scan-Zeile, Schreibstücke, `WcHttpServer` und Backlog, `tcp_tw_limit()`, Weissliste und Überspringen, Felder in `device_ready` | `esp-developer` |
| `ESP8266/ESP-uclock/httpclient.cpp` | Aufruf `tcp_tw_limit()` vor `connect()` | `esp-developer` |
| `ESP8266/ESP-uclock/base.h` | Prototyp `tcp_tw_limit()` | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | Scan-Merker, Warteschlange, Displaystrom aus `settings_xml` (Ent-30 B), Rückfall, Ladesperre, überholte Abrufe, Netzfristen, Kommentar B11 | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/i18n/en.json` | nur falls neue Texte (z. B. Meldung zu einem verworfenen Abruf) | `pwa-developer` |
| `tools/check-pwa.mjs`, `tools/check-pwa.sh`, `tools/smoke-device.sh`, `tools/watch-log.sh`, Prüfstände | §19.10, §19.11 | Lead |
| Versionsdateien | ESP 3.2.37, PWA 1.4.100, `CACHE_NAME` | `release-engineer` |
| `TESTPLAN-PWA.md`, `BEFUNDE.md` | §19.20 | `doc-writer` |
| `CHANGELOG.md` | Eintrag E4 | Lead |

### 19.20 Was E4 an `design.md` ändert

Diese Stellen gelten ab der Freigabe von E4 in der hier genannten Fassung; der Text in `design.md` bleibt als
Stand der Freigabe von E3 stehen.

| Stelle in `design.md` | Änderung |
|---|---|
| §0.1 Regeln, Spalte „Umsetzung" | dazu: E4.1/E4.6 nach E3.14 (Release-Build E3); E4.16 nach E3.17 (Entscheidung nach G7: angehalten, Nutzerentscheid E4) |
| §0.2 Release-Zuschnitt | neue Zeile **7 E4** — ESP + PWA, Nachweis **G8**, Grund: Teil 3 setzt Teil 4 voraus (§19.1); T2 nach G8 und G7R |
| §0.2 Versionen | dazu: **E4 ESP 3.2.37 und PWA 1.4.100** (`CACHE_NAME` mit), Tag erwartet `release/3.2.34-3.2.37-1.4.100` |
| §0.3 Zeitfenster | neue Zeile: PWA 1.4.100 auf ESP 3.2.36 für Minuten (Ent-29 A) — sichere Paarung, keine Massnahme (§19.12) |
| §1.3 ESP-RAM | dazu: E4 ~56 Byte, Grenze 200 (§19.9) |
| §17.2 | neue Zeilen: **G8** lesend (N-5), Scan per Werkzeug, curl-Probe G8.6; **G7R** wie G7, ohne Zero-Window-Probe (Ent-39) |
| §17.3 | dazu: mehrere Browsersitzungen, künstlich erzwungene SYN-Verluste und verwaiste **Setter** am Gerät — nur am Prüfstand (c65k, c65o, c65q) |
| §18.10 Folgebefunde | **M-2 entfällt als Folgebefund** — geht in E4 auf (Backlog §19.5, Displaystrom §19.6) |
| Betroffene Module, Checkliste, Verworfene Alternativen, Versionsfolgen | ergänzt durch §19.19, §19.16, §19.17, §19.18 |
| Verworfene Alternative „C65: M-2 und M-4 gleich mit in E3" | bleibt für E3 gültig; M-2 ist jetzt E4, M-4 (= C67) bleibt Folgebefund |
