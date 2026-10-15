# Tasks — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7`; am selben Tag mit den Entscheiden des Nutzers, den Lead-Entscheiden, dem
Prüfserver-Befund zu `HTTP/1.0`, **Nachtrag A.2**, der **Trennmessung C71** (L468) und **Nachtrag A.3** nachgeführt.
Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.
**Status: freigegeben am 15.10.2026 bis aufs Tor; Nachträge A.2 und A.3 zur Freigabe** — offen Ent-2 (Lead, vor R2.1),
Ent-8, Ent-10, Ent-14, Ent-17 (Tor, G10.6), Ent-16 (vor R3). Anforderungen: `requirements.md`; Design: `design.md` (§17).

**Anker:** Zeilenangaben beziehen sich auf `72cfda7`; seit R1.1 in Arbeit ist, liegen sie in `http.cpp` ab rund `:2839`
um rund +210 später. Massgeblich sind die genannten Funktionen und Symbole.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst bei Exit 0
geht die Schreibberechtigung weiter. Wo „Nutzer“ steht, ist eine Handlung oder ein Entscheid von Dir gemeint, kein
Schreibzugriff.

**Die Spalte „Hängt ab von“ ist verbindlich, auch wo die Dateien disjunkt sind** (R3b):

1. **Drei Releases, jedes mit eigener Messung (Ent-9):** R1 ⇒ G9 (ohne Cache) ⇒ R2 ⇒ G10 (mit Cache) ⇒ Tor ⇒ R3 ⇒
   G11. Code für R2 darf entstehen, während G9 läuft; **gebaut und eingespielt** wird R2 erst nach G9.9. Code für R3
   entsteht erst nach dem Tor (G10.6).
2. **Analyse vor Code:** A.1 ist bis auf P9 (H-CC, wird in G9.6 geprüft) und P9b (L7) erledigt. P9b muss vor Ent-17
   vorliegen, falls G9.6 N1a ergibt.
3. **Ein Schreiber je Datei, nacheinander:** Alle ESP-Tasks eines Releases schreibt **ein** `esp-developer`
   nacheinander; nach jedem misst der Lead „Globale Variablen“ (AKZ.8). Der PWA-Teil R2.3 darf parallel zu R2.1 laufen;
   R2.4 wartet auf beide.
4. **Eine Messung zur Zeit:** Während G9, G10, G11 kein OTA, kein Upload ausser Probe U, kein zweiter Browser.
   **Keine Haltegrenze** (Ent-7): Neustarts und Exceptions sind Ergebnisse — sofort ins Messprotokoll, Update-Quelle
   prüfen (DIR-009), weiter. Zu jedem Ruhe- und Tiefstwert der Vermerk **„Flash-Schreiben seit Start / beim Start“**
   und „Wetter-Overlay ja/nein“.
5. **Firmware zuerst** (DIR-002): in R2 erst ESP-OTA, dann `install-app.sh`.
6. **C71 nach Ent-16:** Mit **A** (Empfehlung) wie unten in R3; mit **B** gehen R3.3, R3.4 und die C71-Teile von R3.5,
   R3.6, R3.9, G11.2 in ein eigenes R4 nach G11.

**Messwerte sofort (L424):** Jeder messende Task hängt sein Ergebnis **vor dem nächsten Task** an das
Messprotokoll in `BEFUNDE.md` an. Messzeilen schreibt der Lead, Befundnachführungen der `doc-writer` — nie
gleichzeitig (R3).

**Einspielen:** Du spielst jedes Release selbst ein; der Lead liefert die fertigen Zeilen samt Reihenfolge und Rückweg
(AKZ.5). Jeder Gerätelauf mit `./tools/watch-log.sh`, vorher gestartet, `--pruefe` am Ende (DIR-013); auf später
geplante Läufe mit `caffeinate` (DIR-020). Schreibende Schritte ohne Einzelfreigabe (`CLAUDE.md:416`).

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. Prüfstände baut der Umsetzer im
Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab und fährt die Gegenproben (DIR-014). Vor jedem Patch
Kodierung **und** Zeilenende feststellen (AKZ.3).

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — Grundlagen

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **Messprotokoll, Ausgangswerte ohne Last:** Entscheide, Prüfserver-Befund zu `HTTP/1.0`/`HTTP/1.1`, Ergebnisse aus A.1 (A.2, A.3); Grössen der 13 App-Dateien; RAM/Programm Build E4; Kodierung und Zeilenende von `http.cpp`, `httpclient.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`, `base.h`, `app.js`; `netstat`, `lsof`, `osascript` vorhanden | Lead | Freigabe | **AKZ.1** | ☐ | — |
| 0.2 | **Werkzeug `ladeprobe`** (liegt vor) vervollständigen: `If-None-Match` je Anfrage, `--erwarte-304`, Vermerk „Flash-Schreiben seit Start“; Gegenproben Vorschau ⇒ FEHL, Prüfserver `HTTP/1.1` ⇒ OK, Prüfserver `HTTP/1.0` ⇒ FEHL | Lead | Freigabe | **AKZ.2 (i)** | ☐ | — |
| 0.3 | **Werkzeug `browser-sitzung`** nach `design.md` §5.3 mit den Betriebsarten `osascript` und **`manuell`**; je eigene Lesung lokaler Port und TIME_WAIT-Ende; **je Sitzung Δ`no_request_timeouts`, Δ`no_request_aborts`, Δ`http_accept_dead`**; Gegenprobe Uhr nicht erreichbar ⇒ Exit ≠ 0. Dazu das **Skript der P1-Probe** (`design.md` §5.6). macOS-Erlaubnis zur Automation einmal durch Dich | Lead | Freigabe | **AKZ.2 (ii)** | ☐ | — |
| 0.4 | **Review der Werkzeuge** 0.2 und 0.3: Kommt jede Meldung an? Leitungsstatus? Eigene TIME_WAIT-Fenster und Δ-Auswertung ausgegeben? | `code-reviewer` | 0.2, 0.3 | DIR-014, L235 | — | ☐ |
| 0.5 | **Trennmessung C71 auf 3.2.37** — **erledigt** 15.10.2026 22:40–22:46: (b) erstes Flash-Schreiben ohne Abruf, (c) Schreiben mit Abruf; beide ohne bleibende Spur; K-a nicht bestätigt | Lead | — | **AKZ.9** ☑ (L468) | — | — |

## Schritt A — Analyse (lesend)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| A.1 | **P1–P12** — **erledigt** bis auf **P9** (Hypothese H-CC; Bestätigung erst durch G9.6) und **P9b** (neu: schliesst das Freigeben des `ClientContext` in `httpclient_stop()` (L7) das Loch, oder legt sich der nächste Kontext wieder hinein?); P9b vor G10.6, falls G9.6 N1a ergibt; der Lead trägt jedes Ergebnis ins Messprotokoll | `firmware-analyst` | Freigabe | Bericht P1–P12, P9b | — | — |
| A.2 | **Nachträge zur Spec** — **erledigt** 15.10.2026: A.2, Trennmessung L468, A.3 (`design.md` §17); Dir mit dem C71-Teil, AKC.7 und Ent-16 vorgelegt | `spec-writer` | A.1, 0.5 | Nachträge freigegeben | — | — |

---

## Schritt R1 — Release: Instrumente I-5 und I-6

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R1.1 | **I-5** nach `design.md` §3.1 in `http_api_device_ready()` und `http_server_loop()`: Felder `tcp_now_*` samt **`tcp_now_unclaimed`** (über `ClientContext::next()`, `<include/ClientContext.h>`), `arg` 0/1/2, `ruhe_s` nach A.2, Zähler **`http_accept_dead`** am frühen Rücksprung bei `!http_client` (nur bei nicht leerem `_unclaimed` vor `accept()`); Listen vor dem ersten `http_send()` in den Stackpuffer; **Kommentar am I-4-Block nach P2 berichtigen**; Zählfunktion, Prototyp in `base.h`; Heap-Zeile in `esp_heap_log()` mit `tcp=`/`nrt=` (`ESP-uclock.ino` byte-genau). **Prüfstand c72i** — *in Arbeit* | `esp-developer` | 0.1, **A.2** (Freigabe) | **AKI.1**, **AKI.2** am Prüfstand | ☐ | — |
| R1.2 | ESP-Teil bauen, „Globale Variablen“ und Programmgrösse sofort ins Messprotokoll | Lead | R1.1 | **AKZ.8** | — | — |
| R1.3 | **I-6** nach `design.md` §3.3 in `update_cache_poll()`, `http_server_loop()` und `setup()`: `- update cache vorher up= free= fe= tcp= hc=`; `- update cache heap v=… 1=…5=… n=…`; **`- update cache nach fe=… tcp=… hc=… acc=…`** mit neuem Zähler `http_accept_count`; `- heap erste anfrage …`; `- flash id=… vendor=…` vor WLAN und `EEPROM.begin()`; **`- update cache tmp geloescht <datei>`** beim Löschen liegengebliebener Temporärdateien. Im Bericht: `fe` oder ersatzweise `frag=`, mit Grund. **Prüfstand c71i** | `esp-developer` | R1.2 | **AKI.3** am Prüfstand | ☐ | — |
| R1.4 | ESP-Teil bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R1.3 | **AKZ.8** | — | — |
| R1.5 | Prüfstände c72i, c71i ablegen, in S16; Gegenproben gegen 3.2.37; Sabotagen; Fallzahlen; c65-Prüfstände grün | Lead | R1.4 | **AKI.1–AKI.4**, **AKZ.7** | ☐ | — |
| R1.6 | **Werkzeuge:** `smoke-device.sh` mit den I-5-Feldern (Gegenprobe `SMOKE_DR_OVERRIDE`); `watch-log.sh` Heap-Zeile mit Zusatz und die I-6-Zeilen als Hinweis (Gegenprobe `WATCHLOG_QUELLE`) | Lead | R1.5 | **AKZ.2 (iii), (iv)** | ☐ | — |
| R1.7 | **Review R1:** kein `yield()` im Listengang; `arg` unterscheidet Listener und `ClientContext`; `http_accept_dead` zählt keine leeren Durchläufe; I-6 `vorher` vor Schritt 1, `nach` mit `hc` und `acc` als Δ, Werte nach `LittleFS.end()` und bei Abbruch; `tmp geloescht`; `- flash id` vor `EEPROM.begin()`; bestehende Cache-Zeilen unverändert; Kodierung und Zeilenenden | `code-reviewer` | R1.6 | Checkliste | — | ☐ |
| R1.8 | ESP-Version anheben | `release-engineer` | R1.7 | S4 | ☐ | — |
| R1.9 | `CHANGELOG.md`-Eintrag R1; **AKZ.4** gegen das Tag von E4; `git status` (R2); `--full`; RAM; sofort ins Messprotokoll | Lead | R1.8 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R1.10 | `TESTPLAN-PWA.md`: I-5-Felder samt `arg`, `ruhe_s` (Lesart nach A.2), `tcp_now_unclaimed`, `http_accept_dead`; Heap-Zeile mit `tcp=`/`nrt=`; die I-6-Zeilen; Ladeprobe, Browser-Sitzung (beide Betriebsarten, Δ`timeouts`/Δ`aborts`), P1-Probe; K30 für T3. `BEFUNDE.md`: Träger „Release R1“ an C65, C70, C71, C72; C71 um L468 und H-CC nachführen; **Nebenbefunde aus A.1 als eigene Zeilen:** `LittleFS.remove()` vor `begin()` in `http_api_app_file_upload()` wirkungslos, `LittleFS.format()` in `setup()` in `#if 0`. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | R1.9 | S9, S10, S11 | ☐ | — |
| R1.11 | **Einspielzeilen:** ESP-OTA, Smoketest, `install-app.sh --check`, `check-pwa.sh`; **Rückweg** (ESP 3.2.37 aus `wordclock-release-2026-10-14-180012.zip`) — benannt, **bevor** eingespielt wird | Lead | R1.9 | **AKZ.5** | — | — |
| R1.12 | Build, ZIP, Rollout, **Commit + Tag + Push** (DIR-011) | Lead | R1.10, R1.11 | **AKZ.5** | ☐ | — |
| R1.13 | **Einspielen:** Wache läuft; Du spielst ein; danach prüft der Lead Smoketest samt Update-Quelle und neuen Feldern, `--check`, `check-pwa.sh`, die Zeile `- flash id`; ob der Abgleich nach dem OTA beim Start schrieb; sofort ins Messprotokoll | Lead (Einspielen: Nutzer) | R1.12 | **AKM.1** | — | — |

---

## Schritt G9 — Ausgangsstand ohne Cache (auf R1)

Der Lead kündigt Beginn und Ende an; jede Phase wird **ausgewertet, bevor** die nächste beginnt.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G9.1 | **Ladeprobe ohne Cache:** drei Läufe (je 1 kalt, 5 warm), einer in Englisch | Lead | R1.13 | **AKM.2** | — | — |
| G9.2 | **P1-Probe** nach `design.md` §5.6: FIN_WAIT_2 nach 10 s da, nach 25 s weg? | Lead | G9.1 | **AKM.7** | — | — |
| G9.3 | **Browser-Sitzung Safari**, `osascript`, S0–S4; Δ`timeouts` gegen Δ`aborts` | Lead | G9.2 | **AKM.3** (Teil Lead) | — | — |
| G9.4 | **Browser-Sitzung Safari auf Deinem Gerät**, `manuell`: das Skript nennt Phase und Handgriff, Du bedienst und bestätigst, das Skript liest die ESP-Seite | Lead (Bedienung: Nutzer) | G9.3 | **AKM.3** (Teil Nutzer) | — | — |
| G9.5 | **Browser-Sitzung Chrome**, `osascript`, mindestens 300 s Ruhe nach G9.4; Δ`timeouts` gegen Δ`aborts` | Lead | G9.4 | **AKM.4** | — | — |
| G9.6 | **Startmessung C71 — der entscheidende Schritt:** ESP-Starts ohne Browser nach `design.md` §5.5, **keine HTTP-Lesung, bis `- update cache heap …` erschienen ist, plus 130 s**: Starts 1, 2 ohne Änderung; Start 3 mit erzwungenem Schreiben beim Start; **Start 4 (empfohlen)** wie 3 mit kaltem `update_status` rund 50 s nach dem Start — **als Abweichung gekennzeichnet**. „Erholt sich nicht“ an einer Heap-Zeile ≥ 130 s nach `n` mit `tw=0`. Danach je Smoketest samt Update-Quelle; Zuordnung N0/N1a/N1b/N2/N3/N4 | Lead | G9.5 | **AKM.5** | — | — |
| G9.7 | **Gesundheit G9**, `watch-log.sh --pruefe`; jeder Neustart als Ergebnis | Lead | G9.6 | **AKM.6** | — | — |
| G9.8 | **Vorläufige Zuordnung** M1–M5 (auf G9 und P1-Probe, samt Δ`timeouts`/Δ`aborts`) und N0–N4, sofort ins Messprotokoll | Lead | G9.7 | AKW.5 (Vorlauf) | — | — |
| G9.9 | **C71, C72 nachführen** mit Messbild, P1-Probe und vorläufigen Zuordnungen. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | G9.8 | S10, S11 | ☐ | — |

---

## Schritt R2 — Release: C70 Asset-Cache (ESP und PWA)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R2.0 | **Ent-2 entscheiden** (Variante T oder V, `design.md` §4.4) und ins Messprotokoll | Lead | A.2 | Ent-2 | — | — |
| R2.1 | **C70, ESP** nach `design.md` §4.4 und Ent-2: `If-None-Match` in der Kopfzeilenschleife von `http_server_loop()`; `ETag` nach **T** (je Antwort Trailer + Grösse, Prüfung `1f 8b`) bzw. **V** (Tabelle, CRC-32/MPEG-2, `app_etag_invalidate()` an allen 31 Stellen, `- app etag`); `HTTP/1.1` **nur** in `http_send_fs_file()` und in der `304`, beide mit `Connection: close`, `200` mit exaktem `Content-Length`, `304` ohne Körper und ohne `Content-Length`; `304` mit Grössenprüfung und `app_complete`; Zähler `app_*`. Im Bericht: alle Statuszeilen `HTTP/1.1` (Soll zwei); bei V alle Schreibstellen mit Invalidierung. **Prüfstand c70e** | `esp-developer` | **R1.13**, **R2.0** | **AKC.1–AKC.5**, **AKC.7** am Prüfstand | ☐ | — |
| R2.2 | ESP-Teil bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R2.1 | **AKZ.8** | — | — |
| R2.3 | **C70, PWA:** `loadLayoutPreviewRowsMap()` mit `cache: "no-cache"`, sonst nichts. **Prüfstand c70p** | `pwa-developer` | R1.13 | **AKC.6** | ☐ | — |
| R2.4a | **Bestehende Prüfstände nachziehen:** c65b und c65w (Kopfzeilen von `http_send_fs_file()` in einem Schreibvorgang, jetzt mit `HTTP/1.1` und `ETag`) und die LittleFS-Attrappe (`seek`, Trailer- bzw. zweites Lesen); Fallzahl vorher/nachher | Lead | R2.2 | **AKC.8** | ☐ | — |
| R2.4 | Prüfstände c70e, c70p ablegen, in S16; Gegenproben gegen R1 bzw. PWA 1.4.100; Sabotagen; Fallzahlen | Lead | R2.4a, R2.3 | **AKC.1–AKC.7**, **AKZ.7** | ☐ | — |
| R2.5 | **Werkzeuge:** `check-pwa` INFO „`/app/` 304 je Laden“ aus `responseReceivedExtraInfo.statusCode`, „Byte je Laden“ und **je Lauf Zahl der `net::ERR_CONNECTION_RESET` und `net::ERR_EMPTY_RESPONSE`**; `smoke-device.sh` mit `app_*`; mit V `watch-log.sh` `- app etag`; je Gegenprobe | Lead | R2.4 | **AKZ.2 (iii), (v)**, bei V **(vi)** | ☐ | — |
| R2.6 | `tools/**` nach Prüfungen auf `HTTP/1.0` bei App-Assets durchsuchen und nachziehen; Ergebnis ins Messprotokoll | Lead | R2.5 | `design.md` §9 | ☐ | — |
| R2.7 | **Review R2:** `HTTP/1.1` an genau zwei Stellen, beide mit `Connection: close`; `Content-Length` exakt; `304` nie ohne Grössenprüfung; T: Trailer je Antwort, `1f 8b`, `seek` zurück an den Anfang; V: jede Schreibstelle invalidiert, CRC-32/MPEG-2; Legacy und `/api/*` weiter 1.0; `If-None-Match`-Puffer geleert und begrenzt; Attrappen-Semantik | `code-reviewer` | R2.6 | Checkliste | — | ☐ |
| R2.8 | ESP-Version, `APP_VERSION` und `CACHE_NAME` anheben | `release-engineer` | R2.7 | S4 zeigt beide | ☐ | — |
| R2.9 | `CHANGELOG.md`-Eintrag R2 (beide Komponenten); **AKZ.4** gegen das Tag von R1; `git status`; `--full`; RAM; `app.js.gz` vorher/nachher | Lead | R2.8 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R2.10 | `TESTPLAN-PWA.md`: `app_*`, `304` als Normalfall, `HTTP/1.1` für App-Assets, `ETag`-Variante, Probe U, Zählung der Verbindungsfehler in `check-pwa`. `BEFUNDE.md`: Träger „Release R2“ an C70. **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | R2.9 | S9, S10, S11 | ☐ | — |
| R2.11 | **Einspielzeilen:** **erst** ESP-OTA, Smoketest, `--check`, **dann** `install-app.sh`, `--check`, `check-pwa.sh`; Rückweg (ESP auf R1, PWA bleibt) — **bevor** eingespielt wird | Lead | R2.9 | **AKZ.5** | — | — |
| R2.12 | Build, ZIP, Rollout, **ein** Commit + Tag + Push | Lead | R2.10, R2.11, **G9.9** | **AKZ.5** | ☐ | — |
| R2.13 | **Einspielen** nach R2.11; danach prüft der Lead Smoketest, `--check`, `check-pwa.sh`, beide Versionen; ob der Abgleich nach dem OTA beim Start schrieb | Lead (Einspielen: Nutzer) | R2.12 | **AKW.1** | — | — |

---

## Schritt G10 — mit Cache, und Tor (auf R2)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G10.1 | **Ladeprobe mit Cache:** drei Läufe, `--erwarte-304`; Gegenüberstellung mit G9.1; Mehrdauer je Anfrage (T) bzw. Dauer `- app etag` (V) | Lead | R2.13 | **AKW.2**, **AKC.4** | — | — |
| G10.2 | **Probe U** nach `design.md` §5.2 | Lead | G10.1 | **AKW.3** | — | — |
| G10.3 | **Browser-Sitzungen mit Cache:** Safari und Chrome (`osascript`), vor S0 je einmal öffnen und beenden; `app_full`/`app_not_modified` je Phase, Δ`timeouts`/Δ`aborts`; Gegenüberstellung mit G9 | Lead | G10.2 | **AKW.4** | — | — |
| G10.3a | **Grundlinie der Verbindungsfehler:** drei `check-pwa.sh` auf dem Stand R2, je Lauf `net::ERR_CONNECTION_RESET`, `net::ERR_EMPTY_RESPONSE`, Δ`write_lost_failed` | Lead | G10.3 | **AKW.6** (Grundlinie) | — | — |
| G10.4 | **Auswertung:** C72 einem von M1–M5 (auf G10; G9, P1-Probe, Δ`timeouts`/Δ`aborts` daneben), C71 einem von N0–N4 (G9.6, AKZ.9, P9, P9b); Gesundheit G10, `--pruefe`; sofort ins Messprotokoll | Lead | G10.3a, A.1 (P9b, falls N1a) | **AKW.5** (Teil 1), **AKW.6** | — | — |
| G10.5 | **C71, C72 nachführen** mit den Zuordnungen. **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | G10.4 | S10, S11 | ☐ | — |
| G10.6 | **Tor:** Der Lead legt **Ent-8** (bei widerlegtem P1: K2 zwingend; K1 nur mit Δ`timeouts`), **Ent-10**, **Ent-14**, **Ent-17** (N1a ⇒ L7, N1b ⇒ L6 samt seinen vier Nebenwirkungen) einzeln vor, dazu **Ent-16**, falls offen; Du entscheidest. Ergibt das Tor weder für C72 noch für C71 einen Eingriff: R3 und G11 entfallen, weiter mit T3 | Lead (Entscheid: Nutzer) | G10.5 | **AKW.5** (Teil 2) | — | — |
| G10.7 | **C70 — weiter oder anhalten:** AKW.2 warm unter 1'072 oder Safari fragt nicht nach ⇒ der Lead legt Dir Variante (c) bzw. `Last-Modified` am Prüfserver vor | Lead (Entscheid: Nutzer) | G10.1, G10.3 | **AKW.2**, **AKW.4** | — | — |

---

## Schritt R3 — Release: Eingriff C72 und Massnahme C71 (nach dem Tor)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R3.0 | **Nachtrag**, nur wenn Ent-8 oder Ent-17 etwas wählt, das `design.md` §6/§7 nicht festlegt (etwa L2 mit eigener Vorbedingung) | `spec-writer` | G10.6 | Nachtrag freigegeben | — | — |
| R3.1 | **C72 — Eingriff** nach Ent-8 und `design.md` §6: **K1** im Zweig „keine Anfrage“ von `http_server_loop()` nur im Teilfall `no_request_timeouts`; **K2** nur `callback_arg == NULL` (nie `ClientContext`, nie SYN_RCVD), Gang neu nach jedem Abbau, Zähler `tcp_reaped`; **K5**. **Prüfstand c72e**. Entfällt ohne Eingriff | `esp-developer` | **G10.6** (R3.0) | **AKE.1** | ☐ | — |
| R3.2 | Bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R3.1 | **AKZ.8** | — | — |
| R3.3 | **C71 — Massnahme** nach Ent-17 und `design.md` §7: **L7** (`httpclient_stop()` gibt den `ClientContext` frei, `client = WiFiClient()`; Kodierung und Zeilenende von `httpclient.cpp` vorher feststellen) oder **L6** (`UPDATE_CACHE_START_MS` auf 300 s; 5-s-Bedingung und Wiederholung unverändert), sonst L0, L4b; Sicherungen des Zwischenspeichers bleiben. **Prüfstand c71e**. Entfällt bei N4 oder ohne Massnahme | `esp-developer` | R3.2 (bzw. G10.6) | **AKS.1** | ☐ | — |
| R3.4 | Bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R3.3 | **AKZ.8** | — | — |
| R3.5 | c72e, c71e ablegen, in S16; Gegenproben gegen R2; Sabotagen; Fallzahlen; c65m, c65n grün — **mit L6 c65n auf die Startfrist nachziehen** (`c65n.cpp:52-54`, `:81`, `:87`, `:214`, Stand `72cfda7`) | Lead | R3.4 | **AKE.1**, **AKS.1**, **AKZ.7** | ☐ | — |
| R3.6 | **Review R3:** K1 nur im Teilfall Timeout; K2 nie `ClientContext` oder Listener; keine Abgabe im Gang; ausgehende Abrufe unberührt; C71: L7 — kein Zugriff auf den freigegebenen Kontext, jeder Abruf danach funktioniert; L6 — nur die Startfrist, nie die Quelle | `code-reviewer` | R3.5 | Checkliste | — | ☐ |
| R3.7 | ESP-Version anheben | `release-engineer` | R3.6 | S4 | ☐ | — |
| R3.8 | `CHANGELOG.md`-Eintrag R3; **AKZ.4** gegen das Tag von R2; `git status`; `--full`; RAM | Lead | R3.7 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R3.9 | `TESTPLAN-PWA.md`: Eingriff, `tcp_reaped`, geändertes Startverhalten — **mit L6** die drei Stellen mit „rund 60 s“ (`TESTPLAN-PWA.md:798`, `:809`, `:959`) und die vier Nebenwirkungen; `BEFUNDE.md`: Träger „Release R3“ an C71, C72. **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | R3.8 | S9, S10, S11 | ☐ | — |
| R3.10 | Einspielzeilen samt Rückweg (Stand R2; nimmt C71 und C72 zurück); Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | R3.9 | **AKZ.5** | ☐ | — |
| R3.11 | **Einspielen:** Wache läuft; Du spielst ein; der Lead prüft Smoketest, `--check`, `check-pwa.sh` | Lead (Einspielen: Nutzer) | R3.10 | **AKZ.5** | — | — |

## Schritt G11 — Wirkung (auf R3)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G11.1 | **C72 — Browser-Sitzungen** Safari und Chrome wie G10.3 | Lead | R3.11 | **AKE.2** | — | — |
| G11.2 | **C71 — drei ESP-Starts** wie G9.6 (einer mit Schreiben beim Start; keine HTTP-Lesung bis `- update cache heap` + 130 s), Bewertung an einer Heap-Zeile ≥ 130 s nach `n` mit `tw=0`; danach Ruhewert 150 s nach `check-pwa.sh` und nach einer Ladeprobe; Zwischenspeicher-Zeilen und `*.tmp` prüfen | Lead | G11.1 | **AKS.2**, **AKS.3** | — | — |
| G11.3 | **Regression gegen die Grundlinie R2 (G10.3a):** drei `check-pwa.sh`, `ERR_CONNECTION_RESET` und `ERR_EMPTY_RESPONSE` nicht mehr als in R2; Ladeprobe warm; Smoketest; Δ`write_lost_failed` | Lead | G11.2 | **AKE.3** | — | — |

---

## Schritt T3 — `pwa-tester` auf dem Endstand

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T3 | `pwa-tester` Phasen 0–4 und 9; **K30** (Ent-12): Ruhewert `max` < 5'120 oder `low=` < 1'072 ⇒ anhalten; bei jedem Halt I-4- und I-5-Felder ohne Parameter in den Bericht; Uhr exklusiv, Beginn und Ende angekündigt | `pwa-tester` | **G11.3**, oder **G10.6** ohne R3; R1.10, R2.10 (R3.9) | **AKZ.6** | — | — |

## Schritt Z — Doku

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: Schliessungen **nur mit Beleg** — C70 (AKW.2–AKW.4), C71 (AKS.2 oder N4), C72 (AKE.2 oder M4/M5), Beitrag zu C65 | `doc-writer` | T3 | S10, S11 | ☐ | — |
| Z.2 | Spec-Endstand: jedes Kriterium mit Beleg oder Grund | `spec-writer` | Z.1 | jedes AK belegt oder begründet | — | — |

---

## Abschluss

- [ ] Alle Tasks erledigt; R3, G11 und einzelne R3-Tasks entfallen nur mit dem Entscheid am Tor
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release, Ergebnis im Messprotokoll
- [ ] Alle 40 Akzeptanzkriterien aus `requirements.md` erfüllt oder mit Grund offen (AKZ.9 erfüllt, L468)
- [ ] Drei Releases (R1, R2, R3 nach dem Tor; R4 nur mit Ent-16 B), je Flash-Umfang und Reihenfolge benannt, je
      committet, getaggt, gepusht und eingespielt
