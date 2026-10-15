# Design — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7` (Zweig `pwa-decoupling`). **Status: Entwurf, zur Freigabe.**
Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`. Anforderungen und Akzeptanzkriterien in
`requirements.md`, Tasks in `tasks.md`.

**Kennzeichen** wie in `requirements.md`: **✔ SW** am Code `72cfda7` bzw. an der Core-Quelle 3.1.2
(`~/Library/Arduino15/packages/esp8266/hardware/esp8266/3.1.2/`) nachgelesen; **✔ B** in `BEFUNDE.md` belegt; **◐**
abgeleitet oder Annahme; **●** gemeldet, vor der Umsetzung zu prüfen. Technische Detailfragen, die Fachwissen am
Core brauchen und die der `spec-writer` nicht abschliessend beantworten kann, stehen als **Prüfpunkte P1–P8**
(§11) beim `firmware-analyst` (Task A.1) — die Spec erfindet dort keine Antwort.

---

## 1. Befund mit Verifikationsstatus

| Aussage | Status | Fundstelle / Bemerkung |
|---|---|---|
| Volles Neuladen mit einem Client reisst K30: `low=752`, `max` 3'584 | **✔ B** | L466 (T2, 21:28:37); I-4 dazu verloren (V3 setzte zurück) |
| Jedes Öffnen holt alle Assets, rund 200 KB, davon `app-app.js.gz` 163'621 Byte | **✔ B** / **✔ SW** | C70, Messprotokoll E3 (E3.0); Kopf ohne Validator `http.cpp:2067-2078` |
| Der Server wertet `If-None-Match` nicht aus | **✔ SW** | Kopfzeilenschleife `http.cpp:16783-16802` liest nur `User-Agent`, `Sec-Fetch-Dest`; jede Zeile über `read_line()` (`:6892`) in ein `String` |
| Die Query eines Asset-Pfads wird abgetrennt, bevor `http_app()` ihn sieht | **✔ SW** | `http.cpp:16812-16823` (`sPath` bis `?`), `:15968-15972`, `:2783-2794` |
| Der ESP kennt die installierte App-Version nicht | **✔ SW** | nur die **verfügbare** vom Update-Server (`APP_VERSION_TXT`, `http.cpp:111`, `:14294`); kein lokales Feld |
| Über `http://` kein Service Worker | **✔ SW** | `app.js:2135-2163`; `sw.js:77-137` wirkt nur unter https |
| `layout-previews.json` wird mit `no-store` geholt; die Sprachdatei mit `?v=` und `default` | **✔ SW** | `app.js:11655`; `app.js:1744-1745` |
| `install-app.sh` und die Auto-Installation laden `index.html` **zuerst** | **✔ SW** | `tools/install-app.sh:36-38`; Reihenfolge `APP_INSTALL_ASSETS`, `http.cpp:747-786` |
| `check-pwa` lädt in einem frischen Profil, neu geladen ohne `ignoreCache` | **✔ SW** | `tools/check-pwa.sh:74`, `:119`; `tools/check-pwa.mjs:375` |
| Nach einer Safari-Sitzung `free` −1,7 KB, `max` 3'816, `http_backlog` 1, über Minuten | **✔ B** | L461 |
| Frei erst beim **Beenden** von Safari | **●** | Beobachtung des Nutzers (L461), nicht gemessen; L461 nennt daneben eine Erholung 20:09:37 „ohne Browser“ |
| Die PWA pausiert ihre Abfragen bei verborgener Seite | **✔ SW** | `app.js:2079-2101` |
| Verbindung ohne Anfrage: höchstens 250 ms warten, zählen, `stop()` | **✔ SW** | `http.cpp:239`, `:16660-16710` |
| `stop()` endet in `tcp_close()`, also FIN | **✔ SW** | `ClientContext.h:73-92` (bei Fehler `tcp_abort`) |
| Gegenstelle schliesst eine noch nicht abgeholte Verbindung ohne Daten ⇒ der Core baut sofort ab | **✔ SW** | `ClientContext.h:594-615` (`abort()` bei FIN ohne Puffer) |
| FIN_WAIT_2 läuft nach 20 s ab, TIME_WAIT dauert 2 × `TCP_MSL` = 120 s | **✔ SW** (Kopf) / **◐** (Bibliothek) | `lwip/priv/tcp_priv.h:131`, `:137`; die Bibliothek liegt binär vor (P1) |
| I-4 zählt Zustände nur beim neuen Block-Tiefstwert | **✔ SW** | `http.cpp:473-530`; Aufruf aus `heap_low_note()` |
| PCBs werden über `malloc` angelegt; `MEMP_NUM_TCP_PCB` 5 begrenzt dann womöglich nicht | **◐** | `lwipopts.h:279` (`MEMP_MEM_MALLOC 1`), `:444`; der Kommentar in `http.cpp:325` nimmt die Grenze 5 an (P2) |
| `tcp_ticks`, `tcp_tw_pcbs`, `tcp_active_pcbs` stehen im privaten Kopf; `tcp_pcb` trägt `remote_port`, `flags`, `tmr`, `nrtx`, `callback_arg` | **✔ SW** | `tcp_priv.h:327`, `:337`, `:340`; `lwip/tcp.h:212-219`, `:249-280`, `:305` |
| `_listen_pcb` und `_unclaimed` sind in `WiFiServer` `protected` | **✔ SW** | `WiFiServer.h:71-77`; die Unterklasse `WcHttpServer` gibt es seit E4 (`http.cpp:52-69`) |
| Der Core bietet `crc32()` | **✔ SW** | `cores/esp8266/coredecls.h:34`; Kosten **◐** (P5) |

---

## 2. Zuschnitt, Releases, Reihenfolge

| Release | Inhalt | Laufzeit | Nachweis | Schreiber |
|---|---|---|---|---|
| **R1** | **I-5** (§3) und **C70** (§4, Variante nach Ent-1); mit Ent-3 eine Zeile in `app.js` | ESP (+ PWA) | **G9** | `esp-developer`; `pwa-developer` nur mit Ent-3 |
| **R2** | **Eingriff C72** (§6), nur nach dem Tor (Ent-8) | ESP | **G10** | `esp-developer` |

**Reihenfolge, wie im Auftrag:** Instrument zuerst (R1), dann Messung (G9), dann Eingriff (R2) — der Eingriff hat
ein **Tor** auf das Messergebnis (§5.4). C70 ist kein Eingriff in das C72-Verhalten; es geht nach Ent-9 B mit dem
Instrument in R1. Sein Vorher-Wert stammt aus der Ladeprobe auf dem heutigen Stand (AKZ.3), nicht aus einem
eigenen Release.

**Mit Ent-9 A** zerfällt R1 in R1a (I-5) und R1b (C70) mit G9 dazwischen. Die Tasks in `tasks.md` tragen deshalb
in der Spalte „Task“ den Teil (**I-5** bzw. **C70**); der Schnitt ist mechanisch.

**Warum C70 vor dem C72-Eingriff:** C70 senkt die Last jedes Ladens, C72 die Last einer **ruhenden** Sitzung. Eine
Messung von C72 auf einem Stand mit C70 misst den Zustand, der danach im Betrieb bleibt — das ist der richtige
Ausgangspunkt für den Eingriff. Umgekehrt verfälscht C70 die C72-Messung nicht: Es ändert nicht, wie Verbindungen
abgebaut werden, nur, wie viel durch sie fliesst (◐, AKM.4 zeigt es).

**Abhängigkeiten nach R3b:** Die ESP-Teile schreibt **ein** `esp-developer` nacheinander (I-5, dann C70, beide in
`http.cpp`). Der PWA-Teil (Ent-3) berührt disjunkte Dateien und hat keinen Gerätenachweis zwischen sich und dem
ESP-Strang — er darf parallel laufen. Der Release-Build wartet auf beide. R2 beginnt erst nach dem Entscheid am Tor.

---

## 3. I-5 — laufende Verbindungszustände (ESP, C72)

### 3.1 Was gemessen wird

**In `/api/device_ready`, hinter `api_orphan_skipped`** (`http.cpp:14770`, bisher letztes Feld):

| Feld | Typ | Inhalt |
|---|---|---|
| `tcp_now_states` | 11 Zahlen | PCBs je Zustand in `tcp_active_pcbs` in diesem Augenblick, Index = `enum tcp_state` (0 CLOSED … 10 TIME_WAIT), sättigend 255; die eigene Verbindung (ESTABLISHED) ist mitgezählt |
| `tcp_now_tw` | Zahl | Länge von `tcp_tw_pcbs` |
| `tcp_now_pending` | Zahl | `http_server.pending()` (`accepts_pending` am Listen-PCB) |
| `tcp_now_self_port` | Zahl | `http_client.remotePort()` dieser Anfrage — macht den eigenen Eintrag in der Liste kenntlich |
| `tcp_now_list` | Feld aus höchstens 8 Feldern | je PCB `[zustand, gegenstelle, port, ruhe_s, nrtx, eigen]`, erst `tcp_active_pcbs`, dann `tcp_tw_pcbs` |
| `tcp_now_more` | Zahl | PCBs, die nicht in die Liste passten |

- `gegenstelle`: letztes Oktett der IPv4-Adresse (Ent-5), 0 bei nicht-IPv4.
- `ruhe_s`: `(tcp_ticks − pcb->tmr) × TCP_SLOW_INTERVAL / 1000`, also Sekunden seit dem letzten Setzen von `tmr`
  — upstream bei jedem empfangenen Segment und beim Zustandswechsel (**◐**, P4). Für TIME_WAIT ist das die Zeit
  seit dem Eintritt.
- `nrtx`: Zahl der Wiederholungen des laufenden Segments — ein FIN, das die Gegenstelle nicht quittiert, wird
  hier sichtbar (FIN_WAIT_1, LAST_ACK).
- `eigen`: 1, wenn `callback_arg` ≠ NULL — ein `ClientContext` hängt am PCB (wartend in `_unclaimed`, bediente
  Verbindung, ausgehender Abruf); 0, wenn der ESP mit `close()` abgegeben hat (`ClientContext.h:78`). **Diese
  Unterscheidung trägt die Entscheidungsregel (§5.4) und begrenzt K2 (§6).**

**Ablauf ohne Abgabe (✔ SW, wie I-4):** Die Listen werden in einen Stackpuffer gelesen (8 Einträge à 8 Byte plus
11 Zähler, rund 80 Byte auf dem 4-KB-Stack), **bevor** das erste `http_send()` läuft — `http_send()` kann über
`http_flush()` und `write()` abgeben, und dann liefe lwIP mitten in einer Listenschleife. Kein `String`, kein
Heap. Gelesen wird im Benutzerkontext; lwIP läuft nur, wenn er abgibt (Begründung wie I-4, `http.cpp:327-330`).

**Kappung auf 8:** Ob `MEMP_NUM_TCP_PCB` = 5 die Zahl aktiver PCBs bei `MEMP_MEM_MALLOC` = 1 begrenzt, ist ◐ (P2).
Die Ausgabe darf sich darauf nicht verlassen: höchstens 8 Einträge, der Rest wird in `tcp_now_more` gezählt.

**Nichts wird zurückgesetzt**, nichts wird gespeichert — I-5 ist eine Lesung des Augenblicks. RAM statisch: 0.

### 3.2 Heap-Zeile (nach Ent-4)

Die Heap-Zeile (`ESP-uclock.ino:612-656`, alle 60 s) erhält hinten:

```
- heap free=<n> max=<n> ovr=<n> low=<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>
```

- Die Zahlen kommen aus einer Funktion in `http.cpp` (privater lwIP-Kopf nur dort, Ent-32 aus Paket 2026-10-13),
  Prototyp in `base.h`; CLOSING fehlt in der Zeile (selten), steht aber in `tcp_now_states`.
- Längste Form: 67 Zeichen heute (Kommentar `ESP-uclock.ino:624`) + ` tcp=255/255/255/255/255/255` (28) +
  ` nrt=65535` (10) = **105** ≤ 119 (ein Ringeintrag). Puffer `line[80]` wird auf 120 erweitert.
- **Warum:** Die Lesung über `device_ready` ist selbst eine Verbindung und hinterlässt einen TIME_WAIT-Eintrag. Die
  Heap-Zeile zählt **ohne** eigene Verbindung, jede Minute, auch während einer Phase, in der niemand fragt. Das ist
  das Instrument, das die Frage „Strom kurzer Verbindungen oder wenige lange?“ (M1 gegen M2) beantwortet.
- `watch-log.sh` liest `free=`, `max=`, `ovr=`, `low=` über Muster (`tools/watch-log.sh:368`, `:383`), die einen
  Zusatz hinten vertragen (◐ — AKZ.2 (v) prüft es mit einer Gegenprobe).
- `ESP-uclock.ino` hat **gemischte Zeilenenden** (CLAUDE.md): byte-genau patchen (AKZ.4).

---

## 4. C70 — Asset-Cache

### 4.1 Varianten

**(a) `ETag` aus dem Inhalt, `If-None-Match` ⇒ `304`.** Jede Asset-Antwort trägt einen Validator; der Browser
fragt bei jedem Laden nach (wegen `no-cache`), der ESP antwortet ohne Körper, solange die Datei gleich ist.

**(b) Versionierte URLs mit langem `max-age`.** `index.html` bleibt `no-cache` (ohne Validator: jedes Mal
vollständig), alle anderen Assets werden als `/app/app.js?v=<APP_VERSION>` referenziert und mit
`Cache-Control: max-age=31536000, immutable` ausgeliefert; der Browser fragt gar nicht mehr.

**(c) Kombination.** `ETag` + `304` für alles, dazu versionierte URLs mit langem `max-age` für die Unterressourcen.

### 4.2 Was beim zweiten Öffnen noch über die Leitung geht

Rechnung je Öffnen im Hauptmodul, Deutsch. **N** = Zahl der `/app/`-Anfragen eines Öffnens; **◐ N ≈ 7**
(`index.html`, `styles.css`, `app.js`, `manifest.webmanifest`, ein Icon, `layout-previews.json`, je nach Browser ein
weiteres Icon; Englisch +1 für `i18n/en.json`) — AKZ.1 misst die echte Liste. Kopf einer `200`-Antwort rund
190 Byte, einer `304` rund 110 Byte (◐, aus den Kopfzeilen in §4.4 gezählt). Die Anfrage des Browsers selbst
(rund 0,4–0,7 KB je Verbindung, ◐) landet als `pbuf` auf dem Heap des ESP und ist in allen Varianten gleich je
Verbindung.

| Variante | Byte vom ESP beim zweiten Öffnen (Chrome) | Verbindungen | Bemerkung |
|---|---|---|---|
| heute | ≈ 200 KB + N × 190 | N (≈ 7) | ✔ B (C70) |
| **(a)** | ≈ N × 110 ≈ **0,8 KB**, dazu `layout-previews.json.gz` vollständig, solange es mit `no-store` geholt wird (Ent-3) | N (≈ 7), je sehr kurz | Safari: dasselbe, sofern er `no-cache` mit Validator wie spezifiziert behandelt (◐, AKM.4 zählt `app_not_modified`) |
| **(b)** | `index.html.gz` vollständig + nicht versionierte Referenzen (Manifest, Icons, die aus `index.html` und dem Manifest kommen) + `layout-previews.json.gz` | 2–4 | Safari sendet beim **Neuladen** möglicherweise trotz `max-age` bedingte Anfragen; mit `immutable` nicht (◐) |
| **(c)** | ≈ 110 je nicht versionierter Datei (Index, Manifest, Icons) ≈ **0,3–0,5 KB**, + `layout-previews` wie oben | 1–4 | |

**Was die Zahlen für den Heap bedeuten (◐):** Das Tief beim Laden entstand bisher während der grossen Übertragung —
1'072 Byte unquittiert, dazu wartende Verbindungen (G8, L457: `heap_low_path` `fs`, 1–3 wartend). Mit (a) bleibt
die **Zahl** der Verbindungen, aber jede ist nach einer `304` in Millisekunden vorbei; das Lastbild gleicht dann
dem der `/api/`-Abrufe, unter dem G8 und G7R 1'336–2'184 maßen. (b) und (c) senken zusätzlich die Zahl der
Verbindungen. Ob (a) allein warm über 1'072 bleibt, zeigt AKM.2 — das ist das Kriterium, (c) nachzuschieben
(Ent-1).

### 4.3 Bewertung

| | (a) | (b) | (c) |
|---|---|---|---|
| **Korrekt nach Update** (`install-app.sh`, `app_file_upload`, Auto-Installation) | **ja, durch Bau:** der `ETag` folgt dem Inhalt; die nächste Anfrage nach dem Schreiben liefert `200` | **nein, mit einem Fenster:** `install-app.sh` lädt `index.html` **zuerst** (`tools/install-app.sh:38`), ebenso die Auto-Installation (`http.cpp:749`). Öffnet in diesem Fenster jemand die PWA, fragt der Browser `app.js?v=NEU` an — der ESP ignoriert die Query (§1) und liefert die **alte** Datei, der Browser legt sie für ein Jahr unter der neuen URL ab. Die PWA bleibt kaputt, bis jemand die Browserdaten löscht. Das ist genau der „veraltete Cache“, den der Auftrag ausschliesst | wie (b) für die versionierten Teile |
| **Abhilfe des Fensters** | — | Index zuletzt hochladen (drei Stellen: `install-app.sh`, Reihenfolge der Weissliste, Updater der PWA); ein offener alter Tab trifft das Fenster trotzdem. Prüfen, ob `v` zum Inhalt passt, kann der ESP nicht — er kennt die Version nicht (§1) | wie (b) |
| **Versionsstellen** | keine neue | eine **fünfte** Stelle (`?v=` in `index.html`), die zu `APP_VERSION` passen muss — gegen „Single Source of Truth“ in CLAUDE.md; nur über einen Build-Schritt im `Makefile` vertretbar | wie (b) |
| **Dateien und Schreiber** | `http.cpp` (`esp-developer`), mit Ent-3 eine Zeile `app.js` | `index.html` (`ui-developer`), `sw.js`-Liste (`pwa-developer`), `http.cpp`, `Makefile` (`release-engineer`) | alle aus (a) und (b) |
| **Invarianten** (`.gz`-only, Grösse > 0) | bleiben; `304` nur nach Grössenprüfung (AKC.2) | bleiben | bleiben |
| **Service Worker unter https** | unverändert; seine Laufzeitabrufe gehen durch den HTTP-Cache und werden billiger (§4.5) | die Precache-Liste ohne `?v=` träfe die versionierten URLs nicht mehr (`cache.match` vergleicht die Query) — `sw.js` müsste mit | wie (b) |
| **Legacy** | unberührt (dynamische Seiten tragen kein `ETag`) | unberührt | unberührt |
| **Rückweg auf alte Firmware** | sicher: alte Firmware ignoriert `If-None-Match` und liefert `200` | sicher | sicher |

**Empfehlung (a)** (Ent-1). Sie ist die einzige Variante, die in **jeder** Lage richtig liefert, ohne eine neue
Versionsstelle und ohne an `index.html` zu rühren. Ihr Preis ist die Zahl der Verbindungen. **(c) ist der
benannte zweite Schritt**, falls AKM.2 warm unter 1'072 bleibt — dann mit Nachtrag (AKC.7), Build-Schritt für `?v=`
und geänderter Upload-Reihenfolge.

### 4.4 Umsetzung von (a)

**`ETag`-Quelle (Ent-2, Lead-Empfehlung): CRC-32 des gespeicherten `.gz`-Inhalts samt Grösse**, Format
`"<grösse>-<crc32 als 8 Hexziffern>"`, z. B. `"163621-1a2b3c4d"`.

- *Grösse allein* scheidet aus: Eine Textänderung in `index.html` kann eine gleich grosse `.gz` ergeben; der Browser
  behielte die alte. Das ist kein theoretischer Fall bei kleinen Dateien.
- *App-Version* scheidet aus: Der ESP kennt sie nicht (§1).
- *`Last-Modified`* scheidet aus: Ob LittleFS hier eine verlässliche Änderungszeit führt, hängt an der Systemzeit des
  ESP beim Schreiben (◐), und eine Sekunde Auflösung trennt zwei Uploads im selben Takt nicht.

**RAM-Tabelle je Asset aus `APP_INSTALL_ASSETS` (13 Einträge, `http.cpp:747-786`):** `uint32_t size`,
`uint32_t crc`; `size == 0` heisst „ungültig“. 104 Byte statisch.

- **Nach dem Start leer.**
- **Gültig nur bei gleicher Grösse:** Vor jeder Verwendung wird die aktuelle Dateigrösse (`fp.size()`) mit dem
  Eintrag verglichen; weicht sie ab, wird neu berechnet. Das fängt jede Änderung ab, die die Grösse ändert, auch
  eine übersehene Schreibstelle.
- **Invalidierung an jeder schreibenden Stelle:** `app_etag_invalidate()` setzt die ganze Tabelle ungültig (13 ×
  `size = 0`). Aufgerufen nach **jeder** schreibenden LittleFS-Stelle in den ESP-Quellen — nicht nur dort, wo heute
  App-Dateien entstehen, weil eine Regel ohne Ausnahmen statisch prüfbar ist und eine Liste „welche Stelle schreibt
  App-Dateien“ veraltet. Die Kosten einer überflüssigen Invalidierung sind eine Neuberechnung beim nächsten Laden.
- **Bestand der Schreibstellen** (Grep am `72cfda7`, Muster `LittleFS.open(…"w"|"w+"|"a")`, `.remove`, `.rename`,
  `.format`; ✔ SW, **vollständig erst nach P6**): `http.cpp` 27 (`:1532`, `:2332`, `:6824`, `:6874`, `:7213`, `:7220`,
  `:7227`, `:7234`, `:7320`, `:7374`, `:7763`, `:7776`, `:7866`, `:7890`, `:7903`, `:10381`, `:13132`, `:13677`, `:13759`,
  `:13766`, `:13843`, `:13848`, `:15151`, `:15192`, `:15201`, `:15585`, `:15617`), `stm32flash.cpp` 3 (`:1282`, `:1315`,
  `:1410`), `ESP-uclock.ino` 1 (`:291`) — **31**. Der Prüfstand c70e zählt selbst nach und vergleicht mit der Zahl,
  die P6 als vollständig bestätigt.

**Berechnung bei Bedarf, in derselben geöffneten Datei** (`http_send_fs_file()`, `http.cpp:2037-2120`): Nach
`LittleFS.open()` und vor dem ersten `http_send()` — ist der Eintrag ungültig, wird die Datei einmal ganz gelesen
(in `http_response` als Lesepuffer, der zu diesem Zeitpunkt leer ist; Bedingung wie `:2088-2089`), CRC und Grösse
eingetragen, `fp.seek(0)`, dann wie heute ausgeliefert. Ein Mount, ein Öffnen. Kein `String`, kein Heap. Zwischen
zwei Blöcken `yield()` wie in der Auslieferungsschleife. Je Berechnung genau eine Zeile
`- app etag <datei> n=<byte> ms=<dauer>` in Mitschnitt und Ring (Muster `http.cpp:1844-1854`, einmal formatiert).
**Kosten (◐, P5):** 163 KB lesen und prüfen, geschätzt 0,2–0,5 s, einmal je Start bzw. je Schreibvorgang.

**`If-None-Match` lesen:** in der Kopfzeilenschleife (`http.cpp:16783-16802`) wie `Sec-Fetch-Dest`, in einen
statischen Puffer von 36 Byte, je Anfrage geleert (Muster `http_clear_request_fetch_dest()`, `:849-853`).
Kopfname ohne Rücksicht auf Gross-/Kleinschreibung. Vergleich: das `W/`-Präfix wird übergangen, eine Komma-Liste
Wert für Wert geprüft, `*` gilt nicht als Treffer (ein Browser sendet es für GET nicht; im Zweifel `200`). Ein
überlanger Wert wird abgeschnitten und trifft dann nicht — sichere Richtung.

**Antworten:**

```
HTTP/1.0 200 OK\r\nContent-Type: <typ>\r\nContent-Encoding: gzip\r\nContent-Length: <n>\r\n
ETag: "<grösse>-<crc>"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n<körper>

HTTP/1.0 304 Not Modified\r\nETag: "<grösse>-<crc>"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n
```

- **`304` nur, wenn** der Wert trifft **und** die Datei besteht mit Grösse > 0 (`http_fs_file_exists_and_nonempty()`,
  `:2049`) **und** beim Index `app_complete` wahr ist (`:2826-2829`, sonst Installationsseite wie heute). Eine leere
  oder fehlende Datei bekommt nie `304` — sonst verdeckte der Cache des Browsers den Weissschirm-Fall, statt ihn zu
  zeigen.
- **Nur Assets aus der Weissliste und nur `.gz`** bekommen einen `ETag`; jede andere Auslieferung (`http.cpp:2789`,
  Fallback-Namen in `http_find_stored_app_asset_filename()`) bleibt, wie sie ist.
- **Zähler** in `device_ready` (hinter I-5): `app_not_modified`, `app_full`, `app_etag_calc` (je `uint16_t`,
  sättigend, seit dem Start). Damit ist **am Gerät** ablesbar, ob ein Browser ohne CDP — Safari — nachfragt.

### 4.5 Was C70 (a) nicht anfasst

- **`sw.js`:** unter `http://` ohne Wirkung. Unter https holt der Precache mit `cache: "reload"` (`sw.js:82`) am
  HTTP-Cache vorbei, die Laufzeitabrufe (`:139-202`) gehen durch ihn und werden mit `304` billiger. Der Wechsel von
  `CACHE_NAME` bleibt der Weg, auf dem der Service Worker eine neue Fassung annimmt. Keine Änderung.
- **`reloadAppPage()`** (`app.js:13225-13253`) löscht CacheStorage, nicht den HTTP-Cache, und navigiert nach
  `/app/` — mit (a) wird dabei jedes Asset nachgefragt und, falls neu, geliefert. Keine Änderung.
- **Safari-Eigenheiten (◐):** Safari kann eine Seite aus dem Zurück-Cache wiederherstellen, ohne zu laden (dann
  läuft alter Code ohne jede Anfrage — unabhängig von C70; L461 nennt Abrufe einer Fassung, die es nicht gibt).
  Die Startbildschirm-App auf iOS hat einen eigenen Cache mit denselben Regeln. AKM.4 liest an `app_full` und
  `app_not_modified`, was Safari tatsächlich tut.

### 4.6 PWA-Teil (Ent-3)

`loadLayoutPreviewRowsMap()` (`app.js:11647-11664`) holt mit `cache: "no-store"`; damit speichert der Browser die
Antwort nicht und sendet nie `If-None-Match`. `"no-cache"` holt bei jedem Aufruf frisch nach, jetzt mit Validator.
Ob sich der Release-Anteil lohnt, hängt an der Grösse von `layout-previews.json.gz`: unter rund 2 KB ist der
Gewinn kleiner als der Aufwand eines PWA-Uploads für Dich; darüber nicht. Daher das Kriterium in Ent-3.

---

## 5. Messung G9

### 5.1 Ladeprobe (C70; Werkzeug `ladeprobe`, Lead)

Ein Headless-Chrome mit **dauerhaftem** Profil im Scratchpad, CDP mit `Network.enable`:

1. Profil leer, `device_ready?heap_low_reset=1`, **kalt** laden, Netzruhe abwarten (Regel aus `check-pwa`, 1'500 ms),
   `device_ready` lesen.
2. Fünfmal **warm**: dreimal `Page.reload` mit `ignoreCache: false`, zweimal `Page.navigate` auf `/app/` (das
   „zweite Öffnen“); vor jedem `heap_low_reset=1`, danach lesen.
3. Je Laden ausgeben: `/app/`-Anfragen nach Status, Summe `encodedDataLength`, `heap_low_block`, `heap_low_path`,
   I-4-Felder, `app_not_modified`, `app_full` (ab R1).

Schalter `--erwarte-304` lässt ein warmes Laden mit einer `200` auf ein App-Asset fehlschlagen (Ausnahme
`layout-previews.json`, solange Ent-3 = nein). Gegenproben AKZ.2 (i). Tor nach Ent-7. Englisch über den Schlüssel
der Sprachwahl im `localStorage` (wie K33).

### 5.2 Probe U — Update sofort wirksam (AKM.3)

Eine geänderte, gültige Datei hochladen, ohne ein Release zu brauchen: `layout-previews.json` um Leerraum ergänzt,
mit `gzip -9` gepackt, über `app_file_upload` als `app/layout-previews.json`. Erwartet beim nächsten Laden: genau
diese Datei `200` mit neuem `ETag`, eine `- app etag`-Zeile, alle anderen `304`. Dann das Original zurück — wieder
`200`, alter `ETag`. Schreibend, steht unter Ent-15.

### 5.3 Browser-Sitzung (C72; Werkzeug `browser-sitzung`, Lead)

Vorbedingung: Die Uhr gehört der Messung (`TESTPLAN-PWA.md` Abschnitt 0 Punkt 5) — der Lead kündigt Beginn und Ende
an. Safari und Chrome sind zu Beginn **beendet** (sonst stellen sie Tabs wieder her). Die Wache läuft.

| Phase | Was | Dauer | Lesung am Ende |
|---|---|---|---|
| **S0** | Ruhe, kein Browser | 180 s | Grundwerte |
| **S1** | Browser öffnet `http://<uhr>/app/`, Seite sichtbar | 120 s | |
| **S2** | neuer Tab mit `about:blank` im Vordergrund — die PWA-Seite ist verborgen | 180 s (> 120 s TIME_WAIT) | |
| **S3** | PWA-Tab geschlossen, Browser läuft weiter | 300 s (> 20 s FIN_WAIT_2, > TIME_WAIT) | |
| **S4** | Browser beendet | 180 s | |

Bedienung nach Ent-6: (A) per `open -a Safari <url>` und `osascript` (neuer Tab, Tab schliessen, `quit`) bzw. das
Gegenstück für Chrome; macOS fragt beim ersten Mal nach der Erlaubnis zur Automation — die muss der Nutzer einmal
geben. (B) der Nutzer auf Zuruf.

**Lesung am Phasenende** (eine Verbindung, ihr TIME_WAIT erscheint in der nächsten Lesung und wird so markiert):
`device_ready` ohne Parameter; `netstat -anv -p tcp | grep <uhr>` (Zustände der Mac-Seite); `lsof -nP -i @<uhr>`
(welcher Prozess die Verbindungen hält, etwa der Netzwerkprozess von WebKit). **Dazwischen keine Lesung** — die
Minutenreihe liefert die Heap-Zeile mit `tcp=` und `nrt=` (§3.2).

### 5.4 Entscheidungsregel (AKM.6)

„Lücke“ = `free_heap`(S0) − `free_heap`(Phase). Bezug Mac = Gegenstelle mit dem Oktett des messenden Macs.

| Ergebnis | Bild | Kandidat (Ent-8) |
|---|---|---|
| **M1** — geschlossen, nicht abgebaut | In S2 oder S3 mindestens ein PCB mit `eigen` = 0 in FIN_WAIT_1, FIN_WAIT_2, CLOSING oder LAST_ACK, Gegenstelle Mac, `ruhe_s` ≥ 30; Lücke ≥ 500; in S4 verschwunden | **K2** |
| **M2** — Strom von Vorab-Verbindungen | In S2/S3 steigt `nrt` um ≥ 1 je Minute; in ≥ 2 von 3 Heap-Zeilen `fw2`/`tw` > 0; kein einzelner PCB mit `ruhe_s` ≥ 30; Lücke ≥ 500; in S4 steht `nrt` still | **K1** |
| **M3** — beides | M1 und M2 | **K1 + K2** |
| **M4** — angenommen und hängend | ein PCB mit `eigen` = 1, nicht die eigene Lesung, ESTABLISHED oder CLOSE_WAIT, `ruhe_s` ≥ 30 — oder `tcp_now_pending` ≥ 1 in zwei Lesungen hintereinander | **kein Eingriff** aus K1–K5; eigener Befund, Analyse am Core (das widerspräche `http.cpp:16595-16604`) |
| **M5** — keine Verbindung | Lücke ≥ 500, aber ausser der eigenen Lesung und ≤ 2 TIME_WAIT nichts in der Liste — **oder** keine Lücke ≥ 500 (nicht nachgestellt) | **kein Eingriff**; C72-Hypothese widerlegt bzw. nicht nachgestellt; Befund nach C71 bzw. Wiederholung nach Ent-6 B |

Ist TIME_WAIT in jeder Lesung voll (2) und trägt sichtbar zur Lücke bei, kommt **K5** als Zusatz in Frage. Chrome
wird gleich ausgewertet und berichtet; zeigt Chrome dasselbe, ist es kein Safari-Eigenart, und der Eingriff gilt
trotzdem.

**Warum eine Regel vor der Messung:** Ohne sie liest man in Messwerte, was man vorher vermutet hat. Mit ihr ist das
Tor prüfbar: Der Lead nennt die Werte, die das Ergebnis tragen, und der Nutzer entscheidet auf dieser Grundlage.

---

## 6. Eingriffskandidaten C72

| | Mechanismus | Wirkung (◐ bis G10) | Risiko | Speicher |
|---|---|---|---|---|
| **K1** — Verbindungen ohne Anfrage hart abbauen | im Zweig „keine Anfrage“ (`http.cpp:16681-16710`) `http_client.abort()` statt `stop()` — RST statt FIN | eine Vorab-Verbindung hinterlässt weder FIN_WAIT noch TIME_WAIT; PCB sofort frei (je rund 0,2 KB PCB plus Kontext, L456 ●) | Der Browser sieht ein RST auf einer **unbenutzten** Verbindung und öffnet für die nächste Anfrage eine neue (◐). Trifft eine Anfrage, die nach 250 ms kommt, bekommt sie RST statt FIN ohne Antwort — dieselbe Fehlerklasse wie heute. AKE.3 sucht `ERR_CONNECTION_RESET` in `check-pwa` | 0 |
| **K2** — vom ESP geschlossene PCBs nach Frist abbauen | höchstens einmal je Sekunde aus `loop()`: PCBs in `tcp_active_pcbs` mit `callback_arg == NULL` in FIN_WAIT_1, FIN_WAIT_2, CLOSING oder LAST_ACK und `ruhe_s` > Frist (Ent-14) ⇒ `tcp_abort()`; Zähler `tcp_reaped`. Nach jedem Abbau beginnt der Gang über die Liste von vorn (die Liste ändert sich, P3) | gebundener Speicher wird nach der Frist frei, unabhängig davon, ob die Gegenstelle antwortet | **schreibender** Zugriff auf lwIP über den privaten Kopf (Ent-10 erweitert Ent-32 aus Paket 2026-10-13). Ein PCB mit `ClientContext` wird **nie** angefasst — auch kein ausgehender Abruf (Wetter, Update), solange er läuft. RST an eine Gegenstelle, die ihre Seite schon geschlossen hat oder nicht mehr antwortet. Gleichzeitigkeit wie I-4 (P3) | ~4 Byte (Zeitpunkt des letzten Gangs, Zähler) |
| **K3** — FIN_WAIT_2-Frist verkürzen | `TCP_FIN_WAIT_TIMEOUT` ist in die Bibliothek übersetzt (`tcp_priv.h:131`) und nicht einstellbar | — | — | **geht in K2 auf**; keine eigene Massnahme |
| **K4** — Höchstzahl je Gegenstelle | beim Annehmen PCBs derselben Gegenstelle zählen, über N die neue abbauen | begrenzt einen einzelnen Browser | **hoch:** Ein Browser öffnet für ein Laden bis zu 6 parallele Verbindungen; zwei Browser auf demselben Mac haben dieselbe Adresse. Ein RST mitten im Laden eines Assets heisst `ERR_CONNECTION_RESET`, im schlimmsten Fall fehlt `app.js` — Weissschirm. Der Backlog aus E4 begrenzt die Last bereits | 0 |
| **K5** — TIME_WAIT auf 1 | `HTTP_TW_LIMIT` 2 ⇒ 1 (`http.cpp:464`) | höchstens ~0,2 KB (●) | wie E4 §19.7, eine Stufe schärfer | 0 |

**Empfehlung je Ergebnis:** siehe §5.4. **K4 wird nicht empfohlen**, auch nicht bei M2: K1 erreicht dort dasselbe,
ohne laufende Ladungen zu treffen.

**Frist in K2 (Ent-14):** Empfohlen 5 s. Eine Gegenstelle, die unser FIN quittiert hat (FIN_WAIT_2), braucht für ihr
eigenes FIN normalerweise Millisekunden; wer nach 5 s nicht geantwortet hat, hält die Verbindung absichtlich offen
(◐). Endgültig am Tor mit den gemessenen Ruhezeiten.

---

## 7. Prüfstände

Ablage `tools/checks/auszug/esp/c72/` und `…/esp/c70/` (Vorbild `…/esp/c65/`), PWA neben b48; Aufnahme in S16. Alle
mit Fallzahl gegen das, was es geben müsste (L235); einmal gegen den Vorgängerstand fehlgeschlagen; je Massnahme
mindestens eine Sabotage (DIR-014). Der Umsetzer baut sie im Scratchpad, der Lead legt sie ab und fährt die
Gegenproben.

| Prüfstand | Gegenstand | Fälle (Auszug) | Sabotage ⇒ FEHL |
|---|---|---|---|
| **c72i** | I-5 | Attrappen `tcp_active_pcbs` (alle 11 Zustände, `callback_arg` gesetzt/leer, verschiedene `tmr`), `tcp_tw_pcbs` (0–5), `tcp_ticks`; Zählung je Zustand, sättigend; Liste ≤ 8 + `tcp_now_more`; `ruhe_s`-Rechnung; `eigen`; Feldreihenfolge hinter `api_orphan_skipped`; **statisch:** kein `http_send` zwischen Beginn und Ende der Listengänge; Heap-Zeile Format, ≤ 119 Zeichen, Mitschnitt und Ring textgleich | Ausgabe in der Listenschleife; Kappung entfernt; Heap-Zeile nur `Serial` |
| **c70e** | C70 (a) | Attrappe LittleFS mit Inhalten; ohne `If-None-Match` ⇒ `200` + `ETag`; Treffer ⇒ `304` ohne Körper; Fehltreffer ⇒ `200`; `W/`, Liste, `*`, überlang, Kopfname klein geschrieben; leere Datei ⇒ nie `304`; Index ohne `app_complete` ⇒ Installationsseite; zwei Inhalte gleicher Grösse ⇒ verschiedene `ETag`; nach Invalidierung neu berechnet; nach Grössenänderung neu berechnet; Asset ausserhalb der Weissliste ⇒ kein `ETag`; Zähler; **statisch:** jede schreibende LittleFS-Stelle mit `app_etag_invalidate()` in derselben Funktion danach, Zahl gegen P6 | Invalidierung an einer Stelle entfernt; `ETag` nur aus Grösse; `304` ohne Grössenprüfung; `304` mit Körper |
| **c70p** (nur Ent-3) | PWA | statisch: `loadLayoutPreviewRowsMap` mit `"no-cache"`; Sprachdatei weiter mit `?v=` | `"no-store"` |
| **c72e** (nur R2) | Eingriff | K1: Zweig „keine Anfrage“ ruft `abort()`, Zähler unverändert, alle anderen Abbauwege unverändert. K2: Attrappe mit gemischten PCBs: nur `callback_arg == NULL` und Zustand aus der Menge und `ruhe_s` > Frist ⇒ abgebaut; höchstens ein Gang je Sekunde; Neubeginn nach Abbau; TIME_WAIT und LISTEN unberührt | K1: `stop()`; K2: PCB mit `ClientContext` abgebaut; Frist entfernt |

Die **Attrappe** bildet die Core-Semantik nach, nicht eine Wunschsemantik (Checkliste Punkt 4, letzte Frage): In
c70e liefert `connected()` nach FIN 0, und `available()` stellt nur an Abgabepunkten zu.

---

## 8. Werkzeuge (Lead)

- **`ladeprobe`** (neu) — §5.1; Gegenproben AKZ.2 (i).
- **`browser-sitzung`** (neu) — §5.3; schreibt jede Lesung mit Uhrzeit in eine Datei im Scratchpad **und** in die
  Konsole; Gegenprobe AKZ.2 (ii).
- **`check-pwa`** — INFO-Zeilen „`/app/` 304 je Laden“ und „Byte je Laden“ aus `Network.responseReceived` und
  `loadingFinished`; kein neuer Fehlschlag (das Soll prüft `ladeprobe`).
- **`smoke-device.sh`** — ab der ESP-Version von R1: Felder aus AKI.1 und AKC.4, `tcp_now_states` mit genau 11
  Zahlen, `tcp_now_list` als Feld; setzt nichts zurück.
- **`watch-log.sh`** — Heap-Zeile mit Zusatz; `- app etag` als Hinweis (nicht Alarm: gewollt nach Start und
  Upload); nach R2 `tcp_reaped` nicht in der Wache (Zähler, keine Zeile).

---

## 9. RAM-Budget (AKZ.9)

| Teil | statisch (Byte) | Bemerkung |
|---|---|---|
| I-5 | 0 | Stackpuffer rund 80 Byte, nur während `device_ready` und der Heap-Zeile |
| Heap-Zeile | 0 | Puffer auf dem Stack 80 ⇒ 120 |
| `ETag`-Tabelle | 104 | 13 × (Grösse + CRC) |
| `If-None-Match`-Puffer | 36 | |
| Zähler `app_*` | 6 | |
| K2 (nur R2) | ~4 | |
| **Summe** | **rund 150** | **Grenze 250 Byte über das Paket (Ent-11)** — darüber anhalten und vorlegen |

Gemessen nach **jedem** ESP-Task (Ausgang Build E4: 69'060 Byte, L457).

---

## 10. Releaseschnitt, Einspielen, Rückweg

**R1** — ESP, mit Ent-3 auch PWA; ein `make release-zip`, ein Commit, ein Tag (DIR-011). Versionen setzt der
`release-engineer` (R4); `APP_VERSION` und `CACHE_NAME` nur mit Ent-3.

| ESP | PWA | Lage |
|---|---|---|
| 3.2.37 | 1.4.100 | heute |
| R1 | 1.4.100 | **sicher** — die alte PWA weiss nichts von `ETag`, der Browser benutzt ihn trotzdem |
| 3.2.37 | R1-PWA (Ent-3) | **sicher** — `no-cache` ohne Validator holt vollständig, wie heute |
| R1 | R1-PWA | Ziel |

**Einspielen (DIR-002, DIR-017):** Firmware zuerst, auch wenn keine Paarung unsicher ist — so steht die Regel.
Wache läuft; ESP-OTA; Smoketest samt Update-Quelle; `install-app.sh --check`; mit Ent-3 `install-app.sh` und erneut
`--check`; `check-pwa.sh`. Du spielst ein; der Lead liefert die fertigen Zeilen samt Rückweg, **bevor** eingespielt
wird.

**Rückweg R1:** ESP zurück auf 3.2.37 aus dem Release-ZIP von E4 (`wordclock-release-2026-10-14-180012.zip`,
L457); jede PWA bleibt (beide Paarungen sicher). Ein Browser, der `ETag`-Werte hält, bekommt von der alten Firmware
wieder `200` — kein Schaden.

**R2** — nur ESP. Rückweg auf den Stand R1 aus dessen ZIP.

**Danach:** G10 (nur mit R2), dann **T3** (`pwa-tester`, DIR-012).

---

## 11. Prüfpunkte für den `firmware-analyst` (Task A.1, lesend)

| P | Frage | Wofür |
|---|---|---|
| **P1** | Setzt `tcp_close()` in `liblwip2-536-feat.a` `TF_RXCLOSED`, sodass FIN_WAIT_2 nach `TCP_FIN_WAIT_TIMEOUT` (20 s) abläuft? Sendet lwIP dabei ein RST? | M1, K2, Ent-14 |
| **P2** | Begrenzt `MEMP_NUM_TCP_PCB` = 5 bei `MEMP_MEM_MALLOC` = 1 die Zahl aktiver PCBs? Wenn nicht: Ist der Kommentar „Listen sind kurz“ (`http.cpp:325`) zu berichtigen? | I-5-Kappung, I-4 |
| **P3** | Ist `tcp_abort()` aus `loop()` auf einem PCB ohne `callback_arg` sicher (kein Rückruf, Speicher sofort frei, RST)? Darf der Gang über `tcp_active_pcbs` nach einem Abbau fortgesetzt werden, oder muss er neu beginnen? | K2 |
| **P4** | Wann setzt diese lwIP-Fassung `pcb->tmr` (Empfang, Zustandswechsel, TIME_WAIT-Eintritt)? Sind `nrtx`, `remote_ip`, `callback_arg` in dieser Bauvariante vorhanden? | `ruhe_s`, Entscheidungsregel |
| **P5** | Wie ist `crc32()` im Core 3.1.2 gebaut (bitweise oder Tabelle), was kostet sie je KB? Ist eine Tabelle im Flash vorzuziehen? | AKC.4, Watchdog |
| **P6** | Vollständiger Bestand der schreibenden LittleFS-Stellen in allen ESP-Quellen, auch mit Modus in einer Variablen und über `File`-Objekte, die anderswo geöffnet werden; welche davon können `app-*.gz` schreiben | AKC.3, c70e |
| **P7** | Wartet `WiFiClient::stop()` im Zweig „keine Anfrage“ (Flush bis 300 ms)? Was ändert `abort()` dort an Referenzzähler und Freigabe des `ClientContext`? | K1 |
| **P8** | Was geschieht mit einem `ClientContext` in `_unclaimed`, dessen Gegenstelle vor dem Abholen schliesst (`ClientContext.h:594-615` baut ab)? Bleibt Speicher hängen, bis `accept()` ihn abholt? | M4 |

Ergebnis als Bericht an den Lead; der Lead trägt es ins Messprotokoll. **Widerlegt ein Prüfpunkt eine Annahme dieser
Spec** (etwa P1: FIN_WAIT_2 läuft nie ab), legt der `spec-writer` vor R1.1 einen Nachtrag vor (Task A.2).

---

## 12. Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Werden die Wetter-Endpunkte eingehalten? Bleibt die
Restore-Bedingung vollständig? Greift die Änderung an der richtigen Schicht an?

> **Legacy:** bleibt parallel und unverändert. Ihre Seiten sind dynamisch erzeugt und tragen keinen `ETag`; C70 setzt
> nur in `http_app()`/`http_send_fs_file()` für Weisslisten-Assets an. AKC.5 vergleicht die Legacy-Auslieferung
> byte-genau gegen 3.2.37. **Wetter:** keine Berührung von `/api/weather_get_now` und `/api/weather_get_forecast`;
> K2 lässt laufende ausgehende Abrufe (mit `ClientContext`) ausdrücklich in Ruhe. **Restore-Bedingung:** STM
> unberührt. **`.gz` und Grösse > 0:** Die Auslieferung bleibt `.gz`-only; `304` setzt die Grössenprüfung **voraus**,
> statt sie zu ersetzen — eine leere Datei wird nie durch den Cache des Browsers verdeckt. **Schicht:** Die Ursache
> von C70 liegt in der Auslieferung des ESP (kein Validator) und wird dort behoben, nicht durch einen Umweg in
> `app.js`; der einzige PWA-Teil (Ent-3) beseitigt ein `no-store`, das jeden Validator wirkungslos machte. C72 wird
> erst gemessen und dann im ESP angegangen — Browser lassen sich nicht steuern.

**Scalable systems** — Wie viele STM-Kommandos erzeugt die Aktion? Wie viele Byte pro Minute zusätzlich auf der
UART? Bleibt jeder Pfad unter 20 s Watchdog? Wie lange blockiert der Hauptloop?

> **STM-Kommandos:** keine. **UART:** Heap-Zeile + rund 38 Byte je Minute (Ent-4); `- app etag` je Berechnung
> (einmal je Asset nach Start oder Schreiben, rund 45 Byte) — im Gegenzug fällt je warmem Laden die Übertragung von
> rund 200 KB weg, also rund 1 s Hauptloop und ihre Debugzeilen. **Hauptloop:** Die CRC-Berechnung blockiert einmal
> je Asset nach Start/Schreiben geschätzt 0,2–0,5 s (◐, P5; AKC.4 begrenzt auf < 2 s); jede `304` ist ein paar
> Millisekunden statt rund einer Sekunde für `app.js`. I-5 ist ein Gang über höchstens rund zehn Listeneinträge je
> `device_ready` und je Minute. K2 ein Gang je Sekunde über dieselbe Liste. **Watchdog:** keine neue Schleife ohne
> Grenze; die CRC-Schleife endet mit der Datei und gibt zwischen Blöcken ab. **Feste Grenzen:** Liste 8 Einträge,
> `If-None-Match` 36 Byte, Tabelle 13 Assets (wächst `APP_INSTALL_ASSETS`, wächst sie mit — die Grösse folgt aus
> dem Feld, nicht aus einer Konstante). **PWA-Polling:** unverändert.

**Secure by design** — `escapeHtml` bei jedem `innerHTML`? Fremddaten als nicht vertrauenswürdig behandelt? Keine
Credentials in Logausgaben?

> **Fremddaten:** `If-None-Match` ist Eingabe aus dem Netz: begrenzter Puffer, abgeschnitten statt übergelaufen,
> nur verglichen, nie ausgegeben. **Logausgaben:** `- app etag` nennt Dateiname, Grösse, Dauer — keine Query, keine
> Kopfzeile. `tcp_now_list` nennt nur das letzte Oktett der Gegenstelle (Ent-5) und Ports — keine Zugangsdaten.
> **Neue Angriffsfläche:** kein neuer Endpunkt, kein neuer Parameter. Ein fremder Client kann mit einem falschen
> `If-None-Match` höchstens eine volle Antwort erzwingen — die gibt es heute ohnehin. Mit einem **richtigen** Wert
> bekommt er `304` — das verrät nur den `ETag`, der in jeder `200` steht. K1/K2 senden RST nur an Verbindungen ohne
> Anfrage bzw. an vom ESP schon geschlossene. **Zerstörende Wege:** unverändert ungeschützt (Schutz-Spec, C6, C61);
> Probe U benutzt `app_file_upload` und steht unter Ent-15. **innerHTML:** keine Änderung an der Darstellung.

**Stable & reliable** — Wird jeder Fehler ausgewertet? Leere `catch`? Stille Verwerfungen? Ist der Zustand nach
Abbruch mitten in einer Sequenz definiert? Wird ein gesetztes Flag auf jedem Pfad wieder aufgelöst?

> **Veralteter Cache, die Hauptgefahr von C70:** ausgeschlossen durch drei unabhängige Sicherungen — Inhalt statt
> Version (Ent-2), Grössenvergleich vor jeder Verwendung, Invalidierung an **jeder** Schreibstelle mit statischer
> Prüfung (c70e). Eine übersehene Schreibstelle, die die Grösse nicht ändert, bleibt als Restrisiko; c70e und P6
> sollen genau sie finden. **Abbruch in einer Sequenz:** Bricht die CRC-Berechnung ab (Lesefehler), bleibt der
> Eintrag ungültig, die Antwort geht ohne `ETag` hinaus (wie heute) — kein halber Wert. Bricht ein Upload ab, ist
> die Tabelle schon invalidiert. Ein Mischstand während `install-app.sh` (neuer Index, alte `app.js`) ist heute
> genauso möglich und wird mit (a) beim nächsten Laden von selbst richtig — mit (b) nicht (§4.3). **Stille
> Verwerfungen:** Jede `304` und jede Berechnung wird gezählt (`app_not_modified`, `app_etag_calc`); K2 zählt jeden
> Abbau (`tcp_reaped`). **Flags auf jedem Pfad:** Der `If-None-Match`-Puffer wird am Anfang **jeder** Anfrage
> geleert (wie `Sec-Fetch-Dest`), sonst wirkte der Wert der vorigen nach. **Fehler ausgewertet:** `fp.read()` ≤ 0
> beendet die Berechnung ohne Eintrag; `fp.seek(0)` gescheitert ⇒ keine Auslieferung mit falschem Körper, sondern
> Abbruch wie bei einem Lesefehler heute. **Messung ohne Selbsttäuschung:** Jede Lesung in G9 ist selbst eine
> Verbindung und wird markiert, nicht herausgerechnet; die Entscheidungsregel steht vor der Messung fest.

---

## 13. Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| `ETag` aus der Grösse allein | Kollision bei kleinen Textänderungen ⇒ veralteter Cache (§4.4) |
| `ETag` aus der App-Version | der ESP kennt sie nicht (§1); die PWA müsste sie beim Hochladen mitliefern — eine neue Wahrheit neben `APP_VERSION` |
| `Last-Modified` | Zeitstempel im LittleFS hängen an der Systemzeit beim Schreiben (◐); Sekundenauflösung |
| CRC bei **jeder** bedingten Anfrage neu berechnen (ohne Tabelle) | korrekt ohne Invalidierung, aber 0,2–0,5 s Hauptloop je `app.js`-Anfrage — während dieser Zeit warten weitere Verbindungen im Backlog, und genau das bindet Heap |
| `ETag` in eigenen Dateien neben den Assets | zusätzliche Schreibvorgänge im LittleFS — die Spur von C71 (L460) |
| `Cache-Control: max-age=<kurz>` ohne Versionierung | nach einem Update bis zu `max-age` alte Dateien — verletzt „sofort wirksam“; `reloadAppPage()` lädt unmittelbar nach dem Upload neu |
| Variante (b) jetzt | Fenster mit veraltetem Cache für ein Jahr (§4.3), fünfte Versionsstelle; benannt als zweiter Schritt (c) |
| Service Worker über `http://` | vom Browser ausgeschlossen (unsicherer Kontext) |
| `device_ready` während der Browser-Sitzung im Sekundentakt lesen | jede Lesung ist eine Verbindung und verändert, was gemessen wird; die Heap-Zeile zählt ohne eigene Verbindung |
| `tcpdump` als Hauptinstrument | braucht `sudo`; `netstat`/`lsof` zeigen die Mac-Seite ohne Rechte. Bleibt als Zusatz, falls M4 eine Paketfolge braucht |
| K3 als eigene Massnahme | Frist ist in der Bibliothek übersetzt; geht in K2 auf |
| K4 Höchstzahl je Gegenstelle | trifft parallele Asset-Ladungen (§6) |
| C72-Eingriff ohne Messung (z. B. K1 vorsorglich in R1) | der Mechanismus ist nicht belegt; ein Eingriff ohne Ursache verschleiert, was er beheben soll (Auftrag: erst messen) |
| C71 mitnehmen | keine Ursache belegt; Ent-13 |

---

## 14. Versionsfolgen

- [ ] ESP `version.h` anheben — **R1**, **R2** (falls R2 stattfindet)
- [ ] App `APP_VERSION` anheben — **R1 nur mit Ent-3**
- [ ] `CACHE_NAME` in `sw.js` anheben — mit `APP_VERSION`
- [ ] STM `src/main.h` — **nicht**

Nur der `release-engineer` führt das aus (R4). Die Nummern bestimmt der Lead; diese Spec setzt keine.

---

## 15. Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `ESP8266/ESP-uclock/http.cpp` (UTF-8, eine CRLF-Zeile — vor dem Patch feststellen) | I-5 (`device_ready`, Zählfunktion für die Heap-Zeile); C70 (`If-None-Match`, Tabelle, CRC, `304`, Zähler, Zeile, `app_etag_invalidate()` an allen Schreibstellen); R2: K1/K2 | `esp-developer` |
| `ESP8266/ESP-uclock/stm32flash.cpp` (UTF-8) | `app_etag_invalidate()` an drei Schreibstellen | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` (gemischte Zeilenenden) | Heap-Zeile mit `tcp=`/`nrt=`; `app_etag_invalidate()` nach `LittleFS.format()` (`:291`); R2 (K2): Aufruf aus `loop()` | `esp-developer` |
| `ESP8266/ESP-uclock/base.h` | Prototypen Zählfunktion, `app_etag_invalidate()`, K2 | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | nur Ent-3: eine Zeile (`app.js:11655`) | `pwa-developer` |
| `tools/ladeprobe*`, `tools/browser-sitzung*`, `tools/check-pwa.mjs`, `tools/smoke-device.sh`, `tools/watch-log.sh`, Prüfstände | §7, §8 | Lead |
| Versionsdateien | ESP (R1, R2); `APP_VERSION`/`CACHE_NAME` nur mit Ent-3 | `release-engineer` |
| `TESTPLAN-PWA.md`, `BEFUNDE.md` | neue Felder und ihre Lesart, Heap-Zeile, Ladeprobe, Browser-Sitzung, `304` als Normalfall; Träger an C65, C70, C72 | `doc-writer` |
| `CHANGELOG.md` | je Release | Lead |
