# Anforderungen — Paket 2026-10-13, Nachtrag E4: zweites Release für C65 (Heap des ESP unter PWA-Last)

**Status: freigegeben 14.10.2026** — vom Nutzer, samt Ent-29, Ent-30, Ent-31 und Ent-39; Ent-32 bis Ent-38 als
Lead-Empfehlungen übernommen (Lead).

**Erstellt:** 2026-10-14, Stand `2f6a324` (Zweig `pwa-decoupling`). Am Gerät: STM 3.2.34, ESP 3.2.36, PWA 1.4.99
(Release E3, Commit `a03cd9e`).
**Auslöser:** Nutzerentscheid 14.10.2026 — **E4 in diesem Paket, vor T2 und Z** (Ent-28), nach dem Abbruch von
G7 an der Haltegrenze K30 (L450) und der Analyse L456.
**Grundlage:** `BEFUNDE.md` L456, L450, L449, C65, C70; Schritt E3 dieser Spec (`requirements.md` Problem §18,
AKR; `design.md` §18). Fundstellen vom `spec-writer` am Code `2f6a324` und an der Core-Quelle 3.1.2 nachgelesen,
soweit mit **✔ SW** gekennzeichnet.

**Teil dieser Spec.** Dieses Dokument ist der Anforderungs-Nachtrag für Schritt **E4** zu `requirements.md`.
Es steht in einer eigenen Datei, weil dem `spec-writer` in dieser Sitzung kein Werkzeug für Teiländerungen zur
Verfügung stand und ein Neuschreiben der 1'001 Zeilen von `requirements.md` das Risiko einer stillen Abweichung
im freigegebenen Text getragen hätte. **Wer `requirements.md` liest, liest diesen Nachtrag mit;** was E4 dort
ändert, steht in §„Was E4 an `requirements.md` ändert". Design: `design-e4.md` (§19). Tasks: `tasks.md`,
Schritt E4.

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen** wie in `requirements.md`: **✔ SW** am Code nachgelesen; **✔ B** in `BEFUNDE.md` belegt; **◐**
abgeleitet, nicht belegt; **●** gemeldet, vor der Umsetzung zu prüfen.
**Kennungen:** Schritt **E4**, Gerätenachweis **G8**, Rest von G7 auf dem neuen Stand **G7R**;
Akzeptanzkriterien **AKV** (Verbindungen und Heap, E4); Entscheidungen **Ent-28 bis Ent-39**; Teile **0–6**
(Nummern aus dem Auftrag, `design-e4.md` §19.1).

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Gerätenachweis |
|---|---|---|---|---|
| **E4** | **C65, zweites Release:** **0** I-4 Messfelder beim Tief und Scan-Zeile · **1** WLAN-Scan nur beim Öffnen und per Knopf · **2** Schreiben in Stücken ≤ `TCP_MSS` · **3** Backlog nach freiem Heap · **4** Warteschlange für `/api/*`, `power_status`, Ladesperre, überholte Abrufe, Netzfristen · **5** TIME_WAIT begrenzen · **6** verwaiste lesende Abfragen überspringen | ESP **und** PWA | nach E3.17; **ein** Release, beide Teile unmittelbar hintereinander, Reihenfolge nach Ent-29 | **G8**, danach **G7R** |
| **T2** | wie geplant (AKZ.6) | — | nach G8, G7R und G6 | — |

---

## Problem

### 19. C65 nach E3 — warum das Tief nicht steigt (L450, L456)

**Befund am Gerät, ✔ B (L450, ESP 3.2.36, PWA 1.4.99):** E3 hat den echten Tiefstwert sichtbar gemacht —
`heap_low_block` **280** Byte, `heap_low_free` **688**, Pfad `flush`; `req_max_ms` 2'871 bei
`/api/network_scan`; `fs write short: app-index.html.gz n=1072 w=0 blk=1048 snd=1072` — nach der
Entscheidungsregel (`design.md` §18.3) eindeutig Heap. G7 wurde nach zwei Ladungen angehalten (K30, Ent-23:
Tiefstwert < 1'072). `keine Quittung` trat nach E3 nicht mehr auf (M-3 wirkt).

**Analyse, Status je Aussage** (Einzelheiten und Fundstellen `design-e4.md` §19.0):

- **Posten beim Tief (●, L456):** fünf wartende Verbindungen je ~0,8 KB ⇒ ~4,0 KB; Sendepuffer der aktiven
  ~1,6 KB; TIME_WAIT ≤ ~0,95 KB; Strings/`FS()` ~0,35 KB — Summe ~6,9 KB gegen gemessen ~7,0 KB. Die Zahlen sind
  aus lwIP upstream gerechnet; **I-4** misst sie am Gerät.
- **Bis zu fünf wartende Verbindungen, je sofort mit `ClientContext` auf dem Heap, ✔ SW** (`WiFiServer.cpp:38-40`,
  `:187-205`); der Backlog ist zur Laufzeit änderbar (`tcp_backlog_set`, `lwip/tcp.h:444-454`, ✔ SW); bei vollem
  Backlog bleibt ein SYN unbeantwortet und der Client wiederholt (✔ SW, Kommentar des Cores; Wiederholzeit ~1 s ◐).
- **Der WLAN-Scan läuft bei offenem Netzwerkmodul alle 15 s, mit Wiederholung, ✔ SW** (`app.js:3401-3411`,
  `:2191-2200`, `:13135-13145`); er hält `loop()` synchron an (`http.cpp:9437`, ✔ SW) — laut L456 der
  wahrscheinlichste Einzelgrund für `heap_low_block` 280 (●).
- **Der Core schreibt 1'072 Byte am Stück und versucht bei Speichermangel kein kleineres Stück, ✔ SW**
  (`ClientContext.h:507-541`); er wartet danach die volle Frist (`:471-489`) — daher `w=0` bei `blk=1048`. Dass
  536 Byte rund 0,67 statt 1,35 KB brauchen, ist **◐** (lwIP upstream).
- **PWA:** `loadSecondaryData` läuft ausserhalb der Ladesperre (✔ SW, `app.js:3137`, `:3147-3151`);
  `display_power` und `ambilight_power` sind zwei eigene Verbindungen mit 2'200 ms Frist und dem Rückfallwert
  `"off"`, den die Anzeige als echtes Aus nimmt (✔ SW, `app.js:3227-3264`) — ein abgelaufener Abruf zeigt das
  Display also **still** als aus. Die Netzfrist zählt ab dem Aufruf (✔ SW, `app.js:13094-13115`).
- **`power_status` liefert beide Werte, ✔ SW** (`http.cpp:8962-8973`); `settings_xml` trägt sie ebenfalls,
  aus demselben Feld `numvars[]` (✔ SW, `http.cpp:8757-8761`) — der Kommentar B11 in `app.js:11095-11106` sagt es.
- **TIME_WAIT:** im Core schon auf 5 begrenzt; `tcp_kill_timewait()` ist deklariert, `tcp_tw_pcbs` nur im
  privaten Kopf (✔ SW, `lwipopts.h:3677-3696`, `lwip/priv/tcp_priv.h:337-340`). Ob der Link gelingt: **◐**.
- **Verwaiste Abfragen:** Eine Anfrage, deren Gegenstelle abgebaut hat, wird trotzdem ausgeführt; die Antwort
  erreicht niemanden (✔ SW, `http.cpp:16025-16028`, C53).

**Werkzeug, ✔ SW:** `tools/check-pwa.mjs` wartet fest 2'500 ms (`:79`), 600 ms (`:133`) und 4'000 ms (`:210`)
— mit einer SYN-Verzögerung von ~1 s und einer Warteschlange, die beim Start nur einen Abruf zulässt, reicht das
nicht.

**Ausserhalb der Frage, Nutzerentscheid:** Jedes Öffnen der PWA lädt alle Assets neu (C70) — **nicht** in E4.

---

## Ziel

17. **Heap unter Last, zweiter Schritt (E4, C65):** Ein `check-pwa.sh` drückt den grössten freien Block **nicht
    mehr unter 1'072 Byte** — in drei Läufen hintereinander —, ohne `fs write short`, ohne `http write lost` mit
    `fail`, ohne `keine Quittung`; der Ruhewert nach Last liegt bei mindestens 5'120. Beim Tief ist ablesbar,
    **wer** den Heap belegt (wartende Verbindungen, PCB-Zustände, TIME_WAIT, unquittierte Byte, Pfad). Der
    WLAN-Scan läuft nur noch, wenn jemand ihn will. Die PWA schickt höchstens zwei Abrufe gleichzeitig, beim
    Start einen, und zeigt bei einem verlorenen Abruf nie mehr still „aus". **Nicht versprochen:** dass die
    bleibende Zerstückelung (M-A) verschwindet und dass jede Massnahme am Gerät **einzeln** belegbar ist —
    welche es ist, sagt `design-e4.md` §19.13.

---

## Akzeptanzkriterien

Wie in `requirements.md`: Jedes Kriterium nennt sein **Instrument** und den **Weg der Meldung** (DIR-014).
Prüfstände sind **einmal gegen den Vorgängerstand fehlgeschlagen** (ESP 3.2.36 bzw. PWA 1.4.99), haben **je
Massnahme mindestens eine Sabotage**, die anschlägt, und melden ihre Fallzahl, die gegen das gezählt wird, was es
geben müsste (L235). Jede Messung steht **sofort** im Messprotokoll E4 in `BEFUNDE.md` (L424). Logzeilen im ESP
in Umschrift (DIR-015). **Was nie am Gerät gefahren wird:** `design-e4.md` §19.20 zu §17.3.

### Schritt E4 — Code und Prüfstand

- [ ] **AKV.1 (Teil 0, I-4)** — Erreicht `heap_low_block` einen neuen Tiefstwert (strikt kleiner), hält
      `heap_low_note()` im selben Augenblick und **nur dann** fest: `heap_low_accepts_pending`,
      `heap_low_backlog`, `heap_low_pcb_states` (11 Zahlen, Index = lwIP `enum tcp_state`), `heap_low_tw`,
      `heap_low_unacked` (`TCP_SND_BUF − availableForWrite()`, **−1** ohne Verbindung), `heap_low_req_path`
      (ohne Query, ≤ 31 Zeichen, `-` ohne Anfrage). Kein Heap, kein `String`, keine Ausgabe in der Messung.
      `/api/device_ready` gibt sie **hinter** `upd_cache_max_ms` aus, danach `http_backlog` und
      `api_orphan_skipped`; bestehende Felder unverändert in Name, Reihenfolge und Typ. `heap_low_reset=1` setzt
      die Momentaufnahme mit I-1 zurück (Zahlen 0, `unacked` −1, Pfad leer), `http_backlog` und
      `api_orphan_skipped` nicht. Prüfstand **c65i**; Sabotagen „Momentaufnahme bei jedem Aufruf" und „Query im
      Pfad" ⇒ je FEHL; gegen 3.2.36 ⇒ FEHL.
- [ ] **AKV.2 (Teil 0, Scan-Zeile)** — Jeder `network_scan` schreibt nach `scanDelete ()` genau eine Zeile
      `- scan n=<n> vor=<free>/<blk> nach=<free>/<blk> flush=<free>/<blk> ms=<scan>/<ausgabe>` (Messpunkte
      `design-e4.md` §19.2), einmal formatiert, in Mitschnitt **und** `/api/stm32_log` textgleich, ≤ 119
      Zeichen. Prüfstand **c65i**; Sabotage „nur `Serial`" ⇒ FEHL.
- [ ] **AKV.3 (Teil 1, Scan, Nutzerentscheid)** — `network_scan` wird nur abgerufen beim Wechsel in das
      Netzwerkmodul, beim Start mit dem Netzwerkmodul als Startmodul und per Knopf `network-scan-button`; die
      Selbstaktualisierung und jedes andere Neuladen rufen ihn **nie**; **ein** Versuch, keine Wiederholung nach
      Zeitüberschreitung; die alte Liste bleibt stehen. Prüfstand **c65q**: 4 Takte `auto` bei offenem
      Netzwerkmodul ⇒ 0 Abrufe; Öffnen ⇒ 1; Knopf ⇒ +1; Abruf läuft ab ⇒ kein zweiter. Sabotage „Scan im `auto`"
      ⇒ FEHL; gegen 1.4.99 ⇒ FEHL.
- [ ] **AKV.4 (Teil 2, Schreibstücke, Ent-33 Variante A)** — `http_flush()` und die Dateiauslieferung rufen
      `http_client.write()` nie mit mehr als `TCP_MSS` Byte je Aufruf; die Fortschrittsregel
      (`design-e4.md` §19.4) gilt; `MAX_HTTP_RESPONSE_LEN` und der Lesepuffer bleiben 1'072; Zeilen und Zähler
      aus I-2 unverändert, `w=` nennt die Byte des 1'072-Byte-Blocks. Prüfstand **c65w** (Schreiblängen
      aufgezeichnet, ausgelieferte Byte gleich Eingabe), **c65b** weiter grün. Sabotage „1'072 je Aufruf" ⇒
      FEHL; gegen 3.2.36 ⇒ FEHL.
- [ ] **AKV.5 (Teil 3, Backlog, Ent-34)** — `http_server` ist eine Unterklasse von `WiFiServer`, startet mit
      Backlog 3 und führt ihn in `http_server_loop()` vor `accept()` nach `ESP.getFreeHeap()` nach: ≥ 5'900 ⇒ 3,
      ≥ 5'100 ⇒ 2, sonst 1; erhöht erst bei Schwelle + 256, gesenkt sofort; `tcp_backlog_set` nur bei
      Änderung. Ohne `TCP_LISTEN_BACKLOG` bricht der Build ab (`#error`). Prüfstand **c65k** (auf- und
      absteigende Folgen, Zahl der Setzvorgänge). Sabotagen „Hysterese entfernt" und „Nachführung entfernt" ⇒ je
      FEHL; gegen 3.2.36 ⇒ FEHL. **Nur mit AKV.6 im selben Release.**
- [ ] **AKV.6 (Teil 4, Warteschlange, Ent-35)** — Alle `/api/`-Abrufe der PWA laufen durch **eine**
      Warteschlange in `fetchWithTimeout()`: höchstens **2** unterwegs, in der Startphase (bis `load` **und**
      erster `loadCoreData()`, höchstens 15 s) **1**; Schreibende vor Lesenden, je Klasse in
      Ankunftsreihenfolge; Netzfrist ab dem **Senden**, auch für den äusseren Zeitgeber in
      `settleWithTimeout()`; Lesende nach 15 s in der Schlange ohne Senden verworfen (Rückfallwert),
      Schreibende nie; lesende Abrufe nach Zeitüberschreitung **nicht** wiederholt; jeder Taktgeber hat höchstens
      einen Abruf offen. Bestandsliste aller `fetch(`/`XMLHttpRequest` auf `/api/` im Taskbericht. Prüfstand
      **c65q**. Sabotagen „Grenze 3" und „Frist ab Aufruf" ⇒ je FEHL; gegen 1.4.99 ⇒ FEHL.
- [ ] **AKV.7 (Teil 4, Displaystrom, Ent-30)** — `loadCoreData()` ruft weder `display_power` noch
      `ambilight_power`; nach Ent-30 A `power_status` (eine Verbindung), nach Ent-30 B keinen eigenen Abruf
      (Werte aus `settings_xml`, `numvar` 3 und 30). Scheitert ein Abruf, gilt der Wert aus `settings_xml` —
      **nie** ein fester Rückfall `"off"`. Der Kommentar B11 (`app.js:11095-11106`) ist nachgeführt. Prüfstand
      **c65q**: 0 Abrufe auf `display_power`/`ambilight_power`; `power_status` läuft ab, `settings_xml` sagt an ⇒
      Anzeige an. Sabotage „Rückfall `"off"`" ⇒ FEHL; gegen 1.4.99 ⇒ FEHL.
- [ ] **AKV.8 (Teil 4, Ladesperre und überholte Abrufe)** — `loadData()` gibt die Ladesperre erst nach dem Ende
      von `loadSecondaryData()` frei (auch wenn es wirft); die Selbstaktualisierung beginnt keinen Lauf, solange
      Nebenabrufe laufen. Ein lesender Abruf eines überholten Laufs (`requestId ≠ loadRequestSerial`) wird beim
      Herausnehmen **nicht gesendet**; `lastUpdateStatusFetchAt` und `lastUpdateTableFilesFetchAt` werden erst
      beim tatsächlichen Senden gesetzt. Prüfstand **c65q**; Sabotage „Zeitstempel beim Einreihen" ⇒ FEHL.
- [ ] **AKV.9 (Teil 4, Netzfristen)** — `CONNECTION_STABILITY` entspricht der Tabelle `design-e4.md` §19.6
      (+1'000 ms je lesende Frist, `apiWriteTimeoutMs` unverändert). Prüfstand **c65q** (statisch, Tabelle
      gegen Datei). Die Wirkung je Konstante steht als Tabelle im Taskbericht E4.7.
- [ ] **AKV.10 (Teil 5, TIME_WAIT, Ent-36)** — `tcp_tw_limit()` lässt höchstens **2** PCBs in TIME_WAIT,
      entfernt die ältesten, über das Core-Makro `TCP_TW_LIMIT`; aufgerufen unmittelbar nach einem erfolgreichen
      `accept()` (vor dem Messpunkt `accept`) und unmittelbar vor `connect()` in `httpclient()` — genau diese
      zwei Stellen. Der private lwIP-Kopf steht nach Ent-32 in **einer** Datei. Prüfstand **c65z**; Sabotagen
      „Grenze 5" und „Aufruf nach dem Messpunkt" ⇒ je FEHL. **Scheitert der Link** von `tcp_kill_timewait`:
      Teil 5 entfällt aus E4, Befund in C65, kein Nachbau.
- [ ] **AKV.11 (Teil 6, verwaiste Lesende, Nutzerentscheid)** — Im GET-Verteiler wird ein Pfad aus der
      **Weissliste** (`design-e4.md` §19.8, Tabelle vollständiger Pfade im Flash, kein Muster) bei
      `http_client.connected () == 0` nicht ausgeführt, nichts gesendet, `api_orphan_skipped` +1 (sättigend);
      Setter, Wartung und Auslöser werden immer ausgeführt (C53). `device_ready` und `reconnect_probe` stehen
      nicht in der Liste. Der Taskbericht E4.5 nennt je Eintrag, warum der Handler keine Nebenwirkung hat, die
      beim Überspringen verloren ginge. Prüfstand **c65o** (Liste gegen Verteiler, kein `_set`/`_default`, keine
      Ausnahme, Setter mit `connected () == 0` ausgeführt). Sabotagen „`ldr_min_set` in der Liste" und „Prüfung
      nach dem Verteiler" ⇒ je FEHL; gegen 3.2.36 ⇒ FEHL.
- [ ] **AKV.12 (RAM)** — „Globale Variablen" und Programmgrösse vor E4.1 und nach **jedem** ESP-Task sofort ins
      Messprotokoll; Zuwachs über ganz E4 **höchstens 200 Byte** (gerechnet ~56, `design-e4.md` §19.9); darüber
      anhalten und vorlegen. `app.js.gz` vorher und nachher, berichtet.
- [ ] **AKV.13 (Werkzeuge)** — (i) `check-pwa` wartet auf Netzruhe statt fest (1'500 ms ohne offenen Abruf,
      höchstens 20 s, sonst Fehlschlag mit Liste) und gibt je Lauf aus: höchstens gleichzeitige `/api/`-Abrufe
      (Startphase und danach), Abrufe je Endpunkt (`network_scan`, `display_power`, `ambilight_power`,
      `power_status`), Verbindungsaufbauten ≥ 900 ms. Gegenproben gegen die Vorschau mit 1'200 ms Verzögerung:
      alte Fassung FEHL, neue OK; nie endender Abruf ⇒ „keine Netzruhe nach 20 s", Exit ≠ 0. (ii)
      `smoke-device.sh` verlangt ab 3.2.37 die Felder aus AKV.1 samt 11 Zahlen in `heap_low_pcb_states`, setzt
      nichts zurück; Gegenproben über `SMOKE_DR_OVERRIDE` (Feld fehlt, 10 Zahlen ⇒ FEHLT; vollständig ⇒ OK).
      (iii) Wache: `- scan` als Hinweis, Gegenprobe über `WATCHLOG_QUELLE`. Jede Meldung kommt in Konsole und
      Mitschrift an (AKZ.5).
- [ ] **AKV.14 (Release-Inhalt)** — AKZ.3 für E4 gegen das Tag von E3: Produktänderungen nur in
      `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino`, `version.h`, `data/app/app.js`, `data/app/sw.js`
      (`CACHE_NAME`) und, falls neue Texte, `data/app/i18n/en.json`; **keine** Änderung an `src/**`,
      `index.html`, `styles.css`.

### Schritt E4 — Einspielen und Gerät (G8)

Lesend freigegeben unter **Nutzerentscheid N-5, 13.10.2026, Gespräch**. Haltegrenze K30 nach Ent-23; wie G8
trotzdem misst, legt Ent-31 fest. Ablauf `design-e4.md` §19.13.

- [ ] **AKV.15 (Einspielen, Ent-29)** — PWA und ESP **unmittelbar hintereinander**, Reihenfolge nach Ent-29,
      dazwischen kein Gerätenachweis, keine schreibende Aktion, keine Aktion an der Oberfläche. Wache vorher
      gestartet. Danach Smoketest samt Update-Quelle und neuen Feldern, `install-app.sh --check` ohne
      Abweichung (DIR-017), `check-pwa.sh` (neue Fassung) ohne Fehler; gemeldete Versionen ESP 3.2.37 und PWA
      1.4.100. Zeiten sofort ins Messprotokoll. Die Einspielzeilen nennen den Rückweg samt Befehl
      (`design-e4.md` §19.12), **bevor** eingespielt wird.
- [ ] **AKV.16 (erster Lauf und Tor, Ent-31)** — G8.1/G8.2: Ruhe, Rücksetzen, **ein** `check-pwa.sh`, 15 s danach
      lesen. Alle Felder aus I-1, I-3 und I-4 sofort ins Messprotokoll. `heap_low_block` < 1'072 oder `low=` <
      1'072 ⇒ **Ende von G8**, keine weitere Ladung, Nutzer fragen.
- [ ] **AKV.17 (Ziel 17)** — In **allen drei** Läufen (G8.2, G8.4) `heap_low_block` ≥ 1'072 und `low=` ≥ 1'072;
      Δ`fs_write_short` = 0; Δ`write_lost_failed` = 0; 0 × `var_send_buf: keine Quittung` vom ersten Rücksetzen
      bis 60 s nach dem letzten Lauf. **Nicht erfüllt** ⇒ C65 bleibt offen mit Befund samt I-4-Werten; T2 nur
      nach Entscheidung des Nutzers.
- [ ] **AKV.18 (Ruhe nach Last)** — G8.3: 150 s ohne Anfrage, `max` der Heap-Zeile **≥ 5'120** (K30), berichtet
      gegen 5'808 (3.2.35) und 7'264 (3.2.34).
- [ ] **AKV.19 (Wirkung je Teil, soweit trennbar)** — je Lauf im Messprotokoll: (a) G8.5: genau **1**
      `network_scan` beim Öffnen und **1** je Knopfdruck in 50 s offenem Netzwerkmodul, je eine `- scan`-Zeile,
      deren `ms=<scan>/<ausgabe>` zu C68; (b) aus `check-pwa`: höchstens **2** gleichzeitige `/api/`, in der
      Startphase **1**; 0 × `display_power`, 0 × `ambilight_power`; (c) beim Tief `heap_low_accepts_pending` ≤
      `heap_low_backlog`, `http_backlog` in Ruhe 3, Zahl der Verbindungsaufbauten ≥ 900 ms (berichtet); (d)
      `heap_low_tw` ≤ 2 in jedem Lauf; (e) G8.6: Δ`api_orphan_skipped` = 1 bei `curl -m 0.5` hinter einem
      laufenden Scan, = 0 bei `-m 8`, und der zweite Abruf bekommt seine Antwort. Teil 2 ist am Gerät **nicht**
      trennbar — belegt durch c65w (AKV.4); das steht so im Bericht.
- [ ] **AKV.20 (Gesundheit)** — Im G8-Fenster 0 Exceptions, 0 Neustarts, `diag` lückenlos, `ovr=0`;
      `watch-log.sh --pruefe` mit Exit 0 im Bericht (AKZ.5).
- [ ] **AKV.21 (G7R)** — Nach bestandenem G8 auf dem Stand 3.2.37/1.4.100: AKR.12 (ii), AKR.13, AKR.14, AKR.15,
      AKR.16, AKR.18, AKR.19 wie in `requirements.md` beschrieben; AKR.17 ohne die Zero-Window-Probe, sofern
      Ent-39 nicht anders entscheidet. K30 vor jeder Ladung.

---

## Nicht Teil dieser Änderung (E4)

| Ausgeschlossen | Grund |
|---|---|
| **C70** Asset-Cache (versionierte URLs, `ETag`/`304`) | Nutzerentscheid 14.10.2026 (L456); Folgebefund besteht |
| **C67** `FS()` ohne Heap | Folgebefund besteht (L453) |
| **C66** Teildatei in `download_file_to_local()` | Folgebefund besteht (L452), ändert die Last nicht |
| **C68** Brücke während langer Antworten lesen | Folgebefund; E4 misst nur (Scan-Zeile) |
| **C69** gekürzte Notes bei Lesefehler | Folgebefund ohne Bezug zur Last |
| Ursache der bleibenden Zerstückelung (M-A) | ohne Beleg; G8.3 liefert neue Werte |
| Überspringen verwaister **Setter**, Wartungs- oder Auslöseaufrufe | Nutzerentscheid: immer ausführen (C53) |
| Teile zur Laufzeit abschaltbar | `design-e4.md` §19.17 |
| Änderungen an der Legacy-Oberfläche, an `index.html`, `styles.css`, am STM | nicht nötig |
| Mehrere Browsersitzungen, erzwungene SYN-Verluste, verwaiste Setter **am Gerät** | nur am Prüfstand (c65k, c65o, c65q) |
| Anpassen der Backlog-Schwellen ohne eigenen Task | Die Schwellen sind gerechnet; eine Änderung nach G8 ist ein eigener Task mit Begründung aus I-4 |

---

## Betroffene Laufzeiten (E4)

- [ ] **ESP8266** — `http.cpp`, `httpclient.cpp`, `base.h`; ein OTA (ESP 3.2.37)
- [ ] **PWA** — `app.js` (gegebenenfalls `i18n/en.json`); ein Upload (PWA 1.4.100)
- [ ] **STM32** — keine
- [ ] **Werkzeug** — `check-pwa.mjs`/`check-pwa.sh`, `smoke-device.sh`, `watch-log.sh`, Prüfstände c65i, c65w,
      c65k, c65z, c65o, c65q
- [ ] **Dokumentation** — `BEFUNDE.md` (Messprotokoll E4 vom Lead, Befunde vom `doc-writer`), `TESTPLAN-PWA.md`,
      `CHANGELOG.md`
- [ ] **Build/Release** — ein Release mit zwei Komponenten; Commit, Tag, Push

---

## Entscheidungen (E4)

**Gefallen am 14.10.2026:**

- **Ent-28 — E4 in diesem Paket** (Nutzer): zweites Release für C65 **vor T2 und Z**, Umfang Teile 0–6 aus L456
  in der dort empfohlenen Reihenfolge. Darin: **WLAN-Scan** nur beim Öffnen des Netzwerkmoduls und per Knopf,
  keine Wiederholung; **verwaiste lesende** `/api/`-Abfragen überspringen, Setter und Wartung immer ausführen;
  **C70 nicht** in E4. **M-2** (Ent-19: Folgebefund) geht damit in E4 auf — Backlog und `power_status`
  (Nutzerentscheid 14.10.2026 zu C65, „Verbindungen begrenzen").

**Entschieden vom Nutzer am 14.10.2026 (Freigabe E4):**

- **Ent-29 — Einspielreihenfolge: A, PWA zuerst, dann ESP**, unmittelbar hintereinander. *Entwurf:* (A) PWA
  zuerst, dann ESP; (B) ESP zuerst, dann PWA, wie EP (Auftrag); Empfehlung A. Die Abhängigkeit läuft hier
  umgekehrt zu EP: Der neue ESP setzt die neue PWA voraus (Backlog trifft sonst die alte PWA mit drei
  gleichzeitigen Abrufen, 2'200 ms Frist und Rückfall „aus"), die neue PWA den neuen ESP nicht. Mit A entsteht
  die unsichere Paarung 3.2.37/1.4.99 nie. DIR-002 („Firmware zuerst, wenn die PWA sie voraussetzt") steht A
  nicht entgegen. `design-e4.md` §19.12.
- **Ent-30 — Displaystrom in der Kernladung: B, Werte aus `settings_xml`**, kein eigener Abruf — eine
  Verbindung statt drei. *Entwurf:* (A) `power_status` statt `display_power` + `ambilight_power` (Auftrag, M-2
  aus L449); (B) Werte aus `settings_xml`; Empfehlung B. `settings_xml` gibt dasselbe Feld `numvars[]` aus wie
  `display_power` (✔ SW, `http.cpp:8757-8761` gegen `:8945`), wird ohnehin geladen, und der Kommentar B11
  (`app.js:11100-11106`) begründet genau das. Der Rückfall „aus" entfällt.
- **Ent-31 — Wie G8 trotz Haltegrenze misst: erste Ladung messen, dann Tor.** Der **erste** `check-pwa` nach dem
  Einspielen ist die Messung; K30 gilt danach als Tor vor jeder weiteren Ladung. Unterschreitet er 1'072, endet
  G8 sofort, und der Lead fragt Dich. Begründung: ohne Ladung keine Messung; zwei Ladungen mit Tiefstwerten von
  400 und 280 Byte hat die Uhr in L450 ohne Neustart überstanden; die Gefahr liegt in der Wiederholung.
  *Verworfene Alternative:* vor dem ersten Lauf ein Lauf mit nur dem Hauptmodul ohne Neuladen — misst nicht, was
  T2 braucht.
- **Ent-39 — Zero-Window-Probe in G7R (G7.4): entfällt.** M-3 bleibt an c65t/c65b und an `ms=` in echten
  Schreibfehlerzeilen belegt; `keine Quittung` ist seit E3 nicht mehr aufgetreten (L450). *Verworfene
  Alternative:* `sudo sysctl net.inet.tcp.doautorcvbuf=0` für die Dauer der Probe.

**Lead-Empfehlungen, vom Lead übernommen am 14.10.2026** (Begründung im Design):

- **Ent-32 — privater lwIP-Kopf:** `lwip/priv/tcp_priv.h` wird in **genau einer** Datei (`http.cpp`)
  eingebunden, nur für lesende Zugriffe auf `tcp_active_pcbs`/`tcp_tw_pcbs` (I-4) und für das Core-Makro
  `TCP_TW_LIMIT` (Teil 5); gekapselt in zwei Funktionen. Ein Core-Wechsel kann den Kopf ändern — der Build
  bricht dann sichtbar, statt still falsch zu zählen. Ohne den Kopf gäbe es weder I-4 noch Teil 5. `design-e4.md`
  §19.2, §19.7.
- **Ent-33 — Schreibstücke Variante A** (immer ≤ 536): kostet keine Zeit; B wartet je Fall die volle Frist.
  `design-e4.md` §19.4.
- **Ent-34 — Backlog-Schwellen** nach `getFreeHeap()`: ≥ 5'900 ⇒ 3, ≥ 5'100 ⇒ 2, sonst 1, Hysterese 256
  aufwärts; gerechnet aus den Posten von L456, in G8 an I-4 überprüft. `design-e4.md` §19.5.
- **Ent-35 — Warteschlange:** Startphase bis `load` und erster Kernladung (höchstens 15 s); Schreibende vor
  Lesenden; Lesende nach 15 s Wartezeit verworfen, Schreibende nie; keine Wiederholung lesender Abrufe nach
  Zeitüberschreitung; Netzfristen +1'000 ms. `design-e4.md` §19.6.
- **Ent-36 — TIME_WAIT-Grenze 2**, aufgerufen nach `accept()` und vor `connect()`. `design-e4.md` §19.7.
- **Ent-37 — Inhalt der Weissliste** nach `design-e4.md` §19.8; `device_ready` und `reconnect_probe`
  ausgenommen.
- **Ent-38 — RAM-Grenze E4: 200 Byte** „Globale Variablen" (gerechnet ~56).

---

## Was E4 an `requirements.md` ändert

Diese Stellen gelten ab der Freigabe von E4 in der hier genannten Fassung; der Text in `requirements.md` bleibt
als Stand der Freigabe von E3 stehen.

| Stelle in `requirements.md` | Änderung |
|---|---|
| Kopf, Kennungen | dazu: Schritt **E4**, Gerätenachweise **G8** und **G7R**, Kriterien **AKV**, Entscheidungen **Ent-28 bis Ent-39** |
| Überblick | Zeile **E4** zwischen E3 und T2 (oben); T2 „nach G8, G7R und G6" statt „nach G7 und G6" |
| Ziel | dazu Ziel **17** (oben) |
| AKR.11–AKR.21 | **AKR.20 ist erfüllt** (L450). **AKR.11 ist nicht vollständig erfüllt** — `check-pwa.sh` 8/1 (`net::ERR_CONNECTION_RESET` bei `/api/network_scan`, L450); es bleibt offen und wird durch AKV.15 auf dem Stand E4 abgelöst. Die übrigen G7-Kriterien laufen als **G7R** auf dem Stand E4 (AKV.21); AKR.17 ohne Zero-Window-Probe nach Ent-39 |
| AKZ.3 | dazu E4: AKV.14 |
| AKZ.5 | dazu ab E4: `- scan` (Hinweis) |
| AKZ.6 | T2 nach G8, G7R und G6 |
| AKZ.9 | für E4: `http.cpp` (UTF-8), `httpclient.cpp`, `base.h`, `app.js` — Kodierung **und** Zeilenende vor dem Patch, im Taskbericht |
| „Nicht Teil dieser Änderung", Zeile **M-2** | **aufgehoben** (Ent-28): M-2 ist Teil von E4 |
| „Nicht Teil dieser Änderung", Zeilen M-4, Teildatei, Brücke, Zerstückelung | bleiben; Kennungen jetzt C67, C66, C68, M-A |
| Betroffene Laufzeiten, Build/Release | dazu E4; **sieben** Releases (S1, S2, E2, EP, P2, E3, E4), acht mit S3 |
| Ent-19 | „M-2 nicht in E3, als Folgebefund" bleibt für E3 wahr; M-2 geht mit Ent-28 in E4 |
