# Tasks — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7`; nachgeführt mit den Entscheiden des Nutzers, den Lead-Entscheiden, dem
Prüfserver-Befund zu `HTTP/1.0`, den Nachträgen **A.2**, **A.3**, **A.4** (Review R1.7), der **Trennmessung** und
**A.5** (Ursache von C71 belegt, L468, 16.10.2026, `89ec34c`). Momentaufnahme (DIR-006); der lebende Stand steht in
`BEFUNDE.md`.
**Status: freigegeben am 15.10.2026 bis aufs Tor; Nachträge A.2 bis A.5 zur Freigabe** — offen Ent-2 (Lead, vor R2.1),
**Ent-18** (neu, Nutzer, vor R2), Ent-8, Ent-10, Ent-14 (Tor), Ent-17 (faktisch L7, Freigabe offen), Ent-16 (nur mit
Ent-18 B). Anforderungen: `requirements.md`; Design: `design.md` (§17).

**Anker:** Zeilenangaben beziehen sich auf `72cfda7`; in `http.cpp` seit R1 ab rund `:2839` um rund +210 später.
Massgeblich sind Funktionen und Symbole.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst bei Exit 0
geht die Schreibberechtigung weiter. Wo „Nutzer“ steht, ist eine Handlung oder ein Entscheid von Dir gemeint, kein
Schreibzugriff.

**Die Spalte „Hängt ab von“ ist verbindlich, auch wo die Dateien disjunkt sind** (R3b):

1. **Releases, jedes mit eigener Messung:** R1 ⇒ G9 ⇒ **(Ent-18 A: R1b ⇒ G9b)** ⇒ R2 ⇒ G10 ⇒ Tor ⇒ R3 ⇒ G11. Code
   für das jeweils nächste Release darf entstehen, während gemessen wird; **gebaut und eingespielt** wird erst nach dem
   Ende der Messung davor.
2. **Mit Ent-18 A** laufen die C72-Browser-Sitzungen (G9.3–G9.5) erst auf R1b — als G9b.3–G9b.5. **Mit Ent-18 B**
   laufen sie auf R1, mit Freistellen des Clients vor jeder Sitzung, und L7 geht in R3.
3. **Ein Schreiber je Datei, nacheinander:** Alle ESP-Tasks eines Releases schreibt **ein** `esp-developer`
   nacheinander; nach jedem misst der Lead „Globale Variablen“ (AKZ.8). Der PWA-Teil R2.3 darf parallel zu R2.1 laufen.
4. **Eine Messung zur Zeit:** Während G9, G9b, G10, G11 kein OTA, kein Upload ausser Probe U, kein zweiter Browser.
   **Keine Haltegrenze** (Ent-7). Zu jedem Ruhe- und Tiefstwert die Vermerke (Flash-Schreiben, Wetter-Overlay,
   **kalter `update_status` seit dem Start und `hc`**).
5. **Firmware zuerst** (DIR-002): in R2 erst ESP-OTA, dann `install-app.sh`.

**Messwerte sofort (L424):** Jeder messende Task hängt sein Ergebnis **vor dem nächsten Task** an das
Messprotokoll in `BEFUNDE.md` an. Messzeilen schreibt der Lead, Befundnachführungen der `doc-writer` — nie
gleichzeitig (R3).

**Einspielen:** Du spielst jedes Release selbst ein; der Lead liefert die fertigen Zeilen samt Reihenfolge und Rückweg
(AKZ.5). Jeder Gerätelauf mit `./tools/watch-log.sh`, `--pruefe` am Ende (DIR-013); auf später geplante Läufe mit
`caffeinate` (DIR-020). Schreibende Schritte ohne Einzelfreigabe (`CLAUDE.md:416`).

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. Prüfstände baut der Umsetzer im
Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab und fährt die Gegenproben (DIR-014). Vor jedem Patch
Kodierung **und** Zeilenende feststellen (AKZ.3).

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — Grundlagen

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **Messprotokoll, Ausgangswerte ohne Last** | Lead | Freigabe | **AKZ.1** | ☐ | — |
| 0.2 | **Werkzeug `ladeprobe`** vervollständigen; Gegenproben | Lead | Freigabe | **AKZ.2 (i)** | ☐ | — |
| 0.3 | **Werkzeug `browser-sitzung`** (`osascript`, `manuell`; Port und TIME_WAIT-Ende; Δ`timeouts`/Δ`aborts`/Δ`accept_dead`; **Freistellen des Clients vor S0 und Protokoll von `hc` und `update_info_cached`, solange L7 nicht eingespielt ist**); Skript der P1-Probe; Auswertung der Lochsumme; **Messung AKS.4** (kalter `update_status`, Heap-Zeilen bis ≥ 130 s mit `tw=0`) | Lead | Freigabe | **AKZ.2 (ii), (vii)** | ☐ | — |
| 0.4 | **Review der Werkzeuge** 0.2 und 0.3 | `code-reviewer` | 0.2, 0.3 | DIR-014, L235 | — | ☐ |
| 0.5 | **Trennmessung C71 auf 3.2.37** — **erledigt** | Lead | — | **AKZ.9** ☑ (L468) | — | — |

## Schritt A — Analyse (lesend)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| A.1 | **P1–P12** — **erledigt**; P9 durch die Messungen des Leads beantwortet (A.5: ungelesene Antwort im `_rx_buf` des statischen httpclient), P9b nicht mehr tragend | `firmware-analyst` | Freigabe | Bericht P1–P12 | — | — |
| A.2 | **Nachträge zur Spec** — **erledigt**: A.2, Trennmessung, A.3 (15.10.2026), A.4, A.5 (16.10.2026) (`design.md` §17); Dir mit dem C71-Teil, AKC.7, Ent-17 und **Ent-18** vorgelegt | `spec-writer` | A.1, 0.5, R1.7, G9.6 | Nachträge freigegeben | — | — |
| A.3 | **L460 nachprüfen (◐):** im Mitschnitt von L457/L460 (14.10.2026, nach dem OTA auf E4) einen Abruf ohne Rumpflesen suchen — `update_status`, Prüfung auf `app.js.gz`, Zweig `zu-gross` —, der den damaligen Fall ohne Anfrage erklärt; Ergebnis ins Messprotokoll | Lead | — | Messprotokoll | — | — |

---

## Schritt R1 — Release: Instrumente I-5 und I-6

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R1.1 | **I-5** nach `design.md` §3.1. **Prüfstand c72i** | `esp-developer` | 0.1, A.2 | **AKI.1**, **AKI.2** | ☐ | — |
| R1.2 | ESP-Teil bauen, RAM sofort ins Messprotokoll | Lead | R1.1 | **AKZ.8** | — | — |
| R1.3 | **I-6** nach `design.md` §3.3 (`frag` statt `fe`). **Prüfstand c71i** | `esp-developer` | R1.2 | **AKI.3** | ☐ | — |
| R1.4 | ESP-Teil bauen, RAM sofort ins Messprotokoll | Lead | R1.3 | **AKZ.8** | — | — |
| R1.5 | Prüfstände c72i, c71i ablegen, in S16; Gegenproben gegen 3.2.37; Sabotagen; Fallzahlen; c65 grün | Lead | R1.4 | **AKI.1–AKI.4**, **AKZ.7** | ☐ | — |
| R1.6 | **Werkzeuge:** `smoke-device.sh` mit den I-5-Feldern; `watch-log.sh` mit Heap-Zeile und I-6-Zeilen; je Gegenprobe | Lead | R1.5 | **AKZ.2 (iii), (iv)** | ☐ | — |
| R1.7 | **Review R1** — Befunde als Nachtrag A.4 eingearbeitet | `code-reviewer` | R1.6 | Checkliste | — | ☐ |
| R1.8 | ESP-Version anheben | `release-engineer` | R1.7 | S4 | ☐ | — |
| R1.9 | `CHANGELOG.md`; **AKZ.4**; `git status`; `--full`; RAM | Lead | R1.8 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R1.10 | `TESTPLAN-PWA.md` und `BEFUNDE.md` nach R1 (I-5, I-6, Lesart `L = free − blk`, Werkzeuge, K30; Träger; Nebenbefunde). **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | R1.9 | S9, S10, S11 | ☐ | — |
| R1.11 | **Einspielzeilen** samt Rückweg | Lead | R1.9 | **AKZ.5** | — | — |
| R1.12 | Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | R1.10, R1.11 | **AKZ.5** | ☐ | — |
| R1.13 | **Einspielen** (Nutzer); Smoketest, `--check`, `check-pwa.sh`, `- flash id` | Lead (Einspielen: Nutzer) | R1.12 | **AKM.1** | — | — |

---

## Schritt G9 — Ausgangsstand ohne Cache (auf R1)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G9.1 | **Ladeprobe ohne Cache:** drei Läufe | Lead | R1.13 | **AKM.2** | — | — |
| G9.2 | **P1-Probe** | Lead | G9.1 | **AKM.7** | — | — |
| G9.3–G9.5 | **Browser-Sitzungen** Safari (`osascript`), Safari beim Nutzer (`manuell`), Chrome — **nur mit Ent-18 B hier**, dann mit Freistellen des Clients vor S0 und Protokoll von `hc`/`update_info_cached`; **mit Ent-18 A als G9b.3–G9b.5 auf R1b** | Lead (Bedienung G9.4: Nutzer) | G9.2, **Ent-18** | **AKM.3**, **AKM.4** | — | — |
| G9.6 | **Startmessung C71** — **gefahren** 16.10.2026: Start 3 ohne Zerstückelung, Start 4 bindet 1'704 Byte; Messungen M1, M3; **Ergebnis N5** (L468). Abnahme mit vollständigem Protokoll | Lead | G9.2 | **AKM.5** | — | — |
| G9.7 | **Gesundheit G9**, `watch-log.sh --pruefe` | Lead | G9.6 | **AKM.6** | — | — |
| G9.8 | **Zuordnung C71 (N5) ins Messprotokoll**; mit Ent-18 B auch vorläufige Zuordnung M1–M5 | Lead | G9.7 | AKW.5 (Vorlauf) | — | — |
| G9.9 | **C71 nachführen** mit Ursache, Messungen M1/M3, Start 3/4 und der Zuordnung der L461-Lücke zu C71; C72 um „1'704 Byte aus L461 gehören zu C71“ ergänzen. **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | G9.8 | S10, S11 | ☐ | — |
| G9.10 | **Ent-18 und Ent-17 vorlegen:** L7 als eigenes Release R1b vor R2 (Empfehlung A) und Freigabe von L7; Du entscheidest | Lead (Entscheid: Nutzer) | G9.9 | Ent-17, Ent-18 | — | — |

---

## Schritt R1b — Release: L7, Behebung von C71 (nur mit Ent-18 A)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R1b.1 | **L7** nach `design.md` §7: **L7a** `httpclient_stop()` gibt nach `client.stop()` den Kontext frei (`client = WiFiClient()`); **L7b** die direkten `client.stop()` in den Fehlerwegen von `httpclient()` (keine Antwort, Status ≠ 200) über `httpclient_stop()`; **L7c = C75** `httpclient()` bei `len == 0` und jeder Aufrufer, der bei `len <= 0` aufgibt (darunter `update_cache_write()`), schliesst über `httpclient_stop()`. Kodierung und Zeilenende von `httpclient.cpp` vorher feststellen. Im Bericht: Liste aller Aufrufer von `httpclient()` mit ihrem Weg bei `len <= 0` und bei ungelesenem Rumpf. **Prüfstand c71e** | `esp-developer` | **G9.10 (Ent-18 A)**, R1.13 | **AKS.1** am Prüfstand | ☐ | — |
| R1b.2 | Bauen, RAM sofort ins Messprotokoll | Lead | R1b.1 | **AKZ.8** | — | — |
| R1b.3 | c71e ablegen, in S16; Gegenprobe gegen R1; **Sabotage „nur `stop()`“ ⇒ FEHL**, „Fehlerweg mit direktem `client.stop()`“ und „`len == 0` offen“ ⇒ FEHL; Fallzahlen; c65m, c65n grün | Lead | R1b.2 | **AKS.1**, **AKZ.7** | ☐ | — |
| R1b.4 | **Review R1b:** kein Zugriff auf den freigegebenen Kontext; jeder Abruf danach funktioniert; jeder Aufrufer schliesst bei `len <= 0`; Fehlerwege; Wetter-Client unberührt; Kodierung und Zeilenenden | `code-reviewer` | R1b.3 | Checkliste | — | ☐ |
| R1b.5 | ESP-Version anheben | `release-engineer` | R1b.4 | S4 | ☐ | — |
| R1b.6 | `CHANGELOG.md`-Eintrag R1b; **AKZ.4** gegen das Tag von R1; `git status`; `--full`; RAM | Lead | R1b.5 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R1b.7 | `TESTPLAN-PWA.md`: Freistellen des Clients vor C72-Sitzungen entfällt (nur noch zum Vergleich); Messung AKS.4. `BEFUNDE.md`: Träger „Release R1b“ an C71 und C75. **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | R1b.6 | S9, S10, S11 | ☐ | — |
| R1b.8 | **Einspielzeilen** samt Rückweg (ESP auf R1); Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | R1b.7 | **AKZ.5** | ☐ | — |
| R1b.9 | **Einspielen** (Nutzer); Smoketest samt Update-Quelle, `--check`, `check-pwa.sh` | Lead (Einspielen: Nutzer) | R1b.8 | **AKZ.5** | — | — |

## Schritt G9b — Wirkung von L7 und Browser-Sitzungen (auf R1b, nur mit Ent-18 A)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G9b.1 | **Messung AKS.4:** dreimal ein kalter `update_status`, je Heap-Zeile ≥ 130 s danach mit `tw=0`; `free` und `max` höchstens 300 Byte unter vorher | Lead | R1b.9 | **AKS.4** | — | — |
| G9b.2 | **Ruhewert nach Start und Last:** drei ESP-Starts (einer mit Schreiben beim Start), danach `check-pwa.sh` und Ladeprobe; Regression des Zwischenspeichers und aller Abrufe | Lead | G9b.1 | **AKS.2**, **AKS.3** | — | — |
| G9b.3 | **Neue Ladeprobe-Grundlinie** (drei Läufe wie AKM.2) für AKW.2 | Lead | G9b.2 | **AKS.4** (Grundlinie) | — | — |
| G9b.4 | **Browser-Sitzung Safari** (`osascript`), dazu einmal **Safari auf Deinem Gerät** (`manuell`), dann **Chrome**; einmal zusätzlich mit Freistellen des Clients zum Vergleich mit G9 | Lead (Bedienung: Nutzer für `manuell`) | G9b.3 | **AKM.3**, **AKM.4** | — | — |
| G9b.5 | **Gesundheit**, `--pruefe`; vorläufige Zuordnung M1–M5 — **entscheidet, ob K1/K2 noch einen Gegenstand haben**; sofort ins Messprotokoll | Lead | G9b.4 | **AKM.6**, AKW.5 (Vorlauf) | — | — |
| G9b.6 | **C71, C72, C75 nachführen** (Wirkung von L7, vorläufige C72-Zuordnung). **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | G9b.5 | S10, S11 | ☐ | — |

---

## Schritt R2 — Release: C70 Asset-Cache (ESP und PWA)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R2.0 | **Ent-2 entscheiden** (T oder V) | Lead | A.2 | Ent-2 | — | — |
| R2.1 | **C70, ESP** nach `design.md` §4.4 und Ent-2. **Prüfstand c70e** | `esp-developer` | R1.13 bzw. **R1b.9**, R2.0 | **AKC.1–AKC.5**, **AKC.7** | ☐ | — |
| R2.2 | ESP-Teil bauen, RAM | Lead | R2.1 | **AKZ.8** | — | — |
| R2.3 | **C70, PWA:** `loadLayoutPreviewRowsMap()` mit `"no-cache"`. **Prüfstand c70p** | `pwa-developer` | R1.13 | **AKC.6** | ☐ | — |
| R2.4a | c65b, c65w, LittleFS-Attrappe nachziehen | Lead | R2.2 | **AKC.8** | ☐ | — |
| R2.4 | c70e, c70p ablegen, in S16; Gegenproben; Sabotagen; Fallzahlen | Lead | R2.4a, R2.3 | **AKC.1–AKC.7**, **AKZ.7** | ☐ | — |
| R2.5 | **Werkzeuge:** `check-pwa` (`304`, Byte, Verbindungsfehler); `smoke-device.sh` mit `app_*`; mit V `- app etag` | Lead | R2.4 | **AKZ.2 (iii), (v)** | ☐ | — |
| R2.6 | `tools/**` nach Prüfungen auf `HTTP/1.0` durchsuchen | Lead | R2.5 | `design.md` §9 | ☐ | — |
| R2.7 | **Review R2** | `code-reviewer` | R2.6 | Checkliste | — | ☐ |
| R2.8 | ESP-Version, `APP_VERSION`, `CACHE_NAME` | `release-engineer` | R2.7 | S4 | ☐ | — |
| R2.9 | `CHANGELOG.md`; **AKZ.4**; `git status`; `--full`; RAM; `app.js.gz` | Lead | R2.8 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R2.10 | `TESTPLAN-PWA.md`, `BEFUNDE.md` nach R2. **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | R2.9 | S9, S10, S11 | ☐ | — |
| R2.11 | **Einspielzeilen:** erst OTA, dann `install-app.sh`; Rückweg | Lead | R2.9 | **AKZ.5** | — | — |
| R2.12 | Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | R2.10, R2.11, **G9.9** bzw. **G9b.6** | **AKZ.5** | ☐ | — |
| R2.13 | **Einspielen** (Nutzer) | Lead (Einspielen: Nutzer) | R2.12 | **AKW.1** | — | — |

---

## Schritt G10 — mit Cache, und Tor (auf R2)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G10.1 | **Ladeprobe mit Cache**, Gegenüberstellung mit der Grundlinie (G9.1 bzw. G9b.3) | Lead | R2.13 | **AKW.2**, **AKC.4** | — | — |
| G10.2 | **Probe U** | Lead | G10.1 | **AKW.3** | — | — |
| G10.3 | **Browser-Sitzungen mit Cache** (ohne L7 mit Freistellen des Clients) | Lead | G10.2 | **AKW.4** | — | — |
| G10.3a | **Grundlinie der Verbindungsfehler:** drei `check-pwa.sh` | Lead | G10.3 | **AKW.6** | — | — |
| G10.4 | **Auswertung:** C72 einem von M1–M5, **Lücke nach Abzug einer C71-Belegung**; Gesundheit | Lead | G10.3a | **AKW.5** (Teil 1), **AKW.6** | — | — |
| G10.5 | **C72 nachführen. Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | G10.4 | S10, S11 | ☐ | — |
| G10.6 | **Tor:** Ent-8, Ent-10, Ent-14 einzeln vorlegen; mit Ent-18 B dazu die Freigabe von Ent-17 und Ent-16. Ergibt das Tor keinen Eingriff (und ist L7 schon in R1b): R3 und G11 entfallen | Lead (Entscheid: Nutzer) | G10.5 | **AKW.5** (Teil 2) | — | — |
| G10.7 | **C70 — weiter oder anhalten** (Variante (c) bzw. `Last-Modified`) | Lead (Entscheid: Nutzer) | G10.1, G10.3 | **AKW.2**, **AKW.4** | — | — |

---

## Schritt R3 — Release: Eingriff C72 (mit Ent-18 B auch L7)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R3.0 | **Nachtrag**, nur wenn Ent-8 etwas wählt, das `design.md` §6 nicht festlegt | `spec-writer` | G10.6 | Nachtrag freigegeben | — | — |
| R3.1 | **C72 — Eingriff** nach Ent-8 (K1 nur Teilfall Timeout; K2 nur `callback_arg == NULL`; K5). **Prüfstand c72e**. Entfällt ohne Eingriff | `esp-developer` | **G10.6** (R3.0) | **AKE.1** | ☐ | — |
| R3.2 | Bauen, RAM | Lead | R3.1 | **AKZ.8** | — | — |
| R3.3 | **Nur mit Ent-18 B: L7** wie R1b.1. **Prüfstand c71e** | `esp-developer` | R3.2 | **AKS.1** | ☐ | — |
| R3.4 | Bauen, RAM | Lead | R3.3 | **AKZ.8** | — | — |
| R3.5 | c72e (und c71e) ablegen, in S16; Gegenproben gegen R2; Sabotagen; Fallzahlen | Lead | R3.4 | **AKE.1**, (**AKS.1**), **AKZ.7** | ☐ | — |
| R3.6 | **Review R3** | `code-reviewer` | R3.5 | Checkliste | — | ☐ |
| R3.7 | ESP-Version anheben | `release-engineer` | R3.6 | S4 | ☐ | — |
| R3.8 | `CHANGELOG.md`; **AKZ.4**; `git status`; `--full`; RAM | Lead | R3.7 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R3.9 | `TESTPLAN-PWA.md`, `BEFUNDE.md` nach R3. **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | R3.8 | S9, S10, S11 | ☐ | — |
| R3.10 | Einspielzeilen samt Rückweg; Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | R3.9 | **AKZ.5** | ☐ | — |
| R3.11 | **Einspielen** (Nutzer) | Lead (Einspielen: Nutzer) | R3.10 | **AKZ.5** | — | — |

## Schritt G11 — Wirkung (auf R3)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G11.1 | **C72 — Browser-Sitzungen** wie G10.3 | Lead | R3.11 | **AKE.2** | — | — |
| G11.2 | **Nur mit Ent-18 B: C71** wie G9b.1 und G9b.2 | Lead | G11.1 | **AKS.2–AKS.4** | — | — |
| G11.3 | **Regression gegen die Grundlinie R2** | Lead | G11.2 | **AKE.3** | — | — |

---

## Schritt T3 — `pwa-tester` auf dem Endstand

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T3 | `pwa-tester` Phasen 0–4 und 9; **K30** (Ent-12): Ruhewert `max` < 5'120 oder `low=` < 1'072 ⇒ anhalten | `pwa-tester` | **G11.3**, oder **G10.6** ohne R3 | **AKZ.6** | — | — |

## Schritt Z — Doku

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: Schliessungen **nur mit Beleg** — C70 (AKW.2–AKW.4), C71 und C75 (AKS.1–AKS.4), C72 (AKE.2 oder M4/M5), Beitrag zu C65 | `doc-writer` | T3 | S10, S11 | ☐ | — |
| Z.2 | Spec-Endstand: jedes Kriterium mit Beleg oder Grund | `spec-writer` | Z.1 | jedes AK belegt oder begründet | — | — |

---

## Abschluss

- [ ] Alle Tasks erledigt; R1b/G9b nur mit Ent-18 A; R3, G11 und einzelne Tasks entfallen nur mit dem Entscheid, der
      sie ausschliesst
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release, Ergebnis im Messprotokoll
- [ ] Alle 41 Akzeptanzkriterien aus `requirements.md` erfüllt oder mit Grund offen (AKZ.9 erfüllt, L468)
- [ ] Releases (R1, mit Ent-18 A R1b, R2, R3 nach dem Tor), je Flash-Umfang und Reihenfolge benannt, je committet,
      getaggt, gepusht und eingespielt
