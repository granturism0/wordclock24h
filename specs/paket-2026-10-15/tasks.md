# Tasks — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7`; nachgeführt mit den Entscheiden des Nutzers, den Lead-Entscheiden, dem
Prüfserver-Befund zu `HTTP/1.0`, den Nachträgen **A.2**, **A.3**, **A.4** (Review R1.7), der **Trennmessung**, **A.5**
(Ursache von C71 belegt, L468, 16.10.2026, `89ec34c`) und **A.6** (C70 Variante (c), nach dem Tor G10.6/G10.7,
16.10.2026, Stand `2d97259`). Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.
**Status: freigegeben am 15.10.2026 bis aufs Tor; Nachträge A.2 bis A.5 umgesetzt (R1, R1b, R2 eingespielt, L468);
Entscheide Ent-19 bis Ent-23 gefallen, A.6 freigegeben (16.10.2026).** Entschieden am 16.10.2026: Ent-2 (T), Ent-17
(L7), Ent-18 (A), **Ent-8 kein Eingriff** (Ent-10, Ent-14 entfallen; **R3 und G11 entfallen**), **G10.7: (c)** (⇒ R2b,
G10b vor T3), **Ent-19 C** (`?v=<APP_VERSION>&h=<marke>`), **Ent-20 B** (`layout-previews.json` unversioniert),
**Ent-21 E2**, **Ent-22 ja** (Manifest versioniert), **Ent-23 1 Jahr**. Anforderungen: `requirements.md`; Design:
`design.md` (§17, §18). **Endstand 17.10.2026 (Z.2):** je Task im Abschnitt „Endstand 17.10.2026“ am Ende dieser Datei,
je Akzeptanzkriterium in `endstand.md`.

**Anker:** Zeilenangaben beziehen sich auf `72cfda7`, die aus A.6 auf `2d97259`; in `http.cpp` seit R1 ab rund `:2839`
um rund +210 später. Massgeblich sind Funktionen und Symbole.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst bei Exit 0
geht die Schreibberechtigung weiter. Wo „Nutzer“ steht, ist eine Handlung oder ein Entscheid von Dir gemeint, kein
Schreibzugriff.

**Die Spalte „Hängt ab von“ ist verbindlich, auch wo die Dateien disjunkt sind** (R3b):

1. **Releases, jedes mit eigener Messung:** R1 ⇒ G9 ⇒ **(Ent-18 A: R1b ⇒ G9b)** ⇒ R2 ⇒ G10 ⇒ Tor ⇒ ~~R3 ⇒ G11~~
   **(entfallen, Ent-8)** ⇒ **R2b ⇒ G10b** (A.6) ⇒ T3. Code für das jeweils nächste Release darf entstehen, während
   gemessen wird; **gebaut und eingespielt** wird erst nach dem Ende der Messung davor.
2. **Mit Ent-18 A** laufen die C72-Browser-Sitzungen (G9.3–G9.5) erst auf R1b — als G9b.3–G9b.5. **Mit Ent-18 B**
   laufen sie auf R1, mit Freistellen des Clients vor jeder Sitzung, und L7 geht in R3.
3. **Ein Schreiber je Datei, nacheinander:** Alle ESP-Tasks eines Releases schreibt **ein** `esp-developer`
   nacheinander; nach jedem misst der Lead „Globale Variablen“ (AKZ.8). Der PWA-Teil R2.3 darf parallel zu R2.1 laufen.
   **In R2b** dürfen R2b.1 (`esp-developer`), R2b.3 (Lead, Werkzeug), R2b.5 (`ui-developer`) und R2b.6
   (`pwa-developer`) parallel laufen — vier Dateien, keine Abhängigkeit untereinander ausser dem Vertrag aus
   `design.md` §18.7, den die Spec festlegt. R2b.4 (`Makefile`) erst nach R2b.3.
4. **Eine Messung zur Zeit:** Während G9, G9b, G10, G10b, G11 kein OTA, kein Upload ausser Probe U, kein zweiter
   Browser. **Keine Haltegrenze** (Ent-7). Zu jedem Ruhe- und Tiefstwert die Vermerke (Flash-Schreiben,
   Wetter-Overlay, **kalter `update_status` seit dem Start und `hc`**).
5. **Firmware zuerst** (DIR-002): in R2 und in R2b erst ESP-OTA, dann `install-app.sh`.
6. **`make app-gz` nur bei stillem Baum** (R2) — in R2b erzeugt es `index.html.gz` aus mehreren Quellen
   (`design.md` §18.7).

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
| A.4 | **Nachtrag A.6** (C70 Variante (c)) — **erledigt**: geschrieben, Entscheide Ent-19 bis Ent-23 eingearbeitet, freigegeben am 16.10.2026 (`design.md` §17, §18; `requirements.md` AKL, AKG) | `spec-writer` | G10.7 | A.6 freigegeben ☑ | — | — |

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
| R1.7 | **Review R1** — Befunde als Nachtrag A.4 eingearbeitet | `code-reviewer` | R1.6 | Checkliste | — | ☑ |
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
| R2.7 | **Review R2** | `code-reviewer` | R2.6 | Checkliste | — | ☑ |
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
| G10.6 | **Tor:** Ent-8, Ent-10, Ent-14 einzeln vorlegen; mit Ent-18 B dazu die Freigabe von Ent-17 und Ent-16. Ergibt das Tor keinen Eingriff (und ist L7 schon in R1b): R3 und G11 entfallen. **Entschieden 16.10.2026: Ent-8 kein Eingriff; Ent-10, Ent-14 entfallen; R3 und G11 entfallen** (L468) | Lead (Entscheid: Nutzer) | G10.5 | **AKW.5** (Teil 2) | — | — |
| G10.7 | **C70 — weiter oder anhalten** (Variante (c) bzw. `Last-Modified`). **Entschieden 16.10.2026: weiter mit (c)** ⇒ Nachtrag A.6, R2b, G10b | Lead (Entscheid: Nutzer) | G10.1, G10.3 | **AKW.2**, **AKW.4** | — | — |

---

## Schritt R2b — Release: C70 Variante (c), versionierte URLs (Nachtrag A.6)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R2b.0 | **A.6 vorlegen; Ent-19 und Ent-20 entscheiden; A.6 freigeben** — **erledigt 16.10.2026:** Ent-19 C, Ent-20 B, A.6 freigegeben | Lead (Entscheid und Freigabe: Nutzer) | A.4 | Ent-19, Ent-20, Freigabe A.6 ☑ | — | — |
| R2b.0a | **Ent-21, Ent-22, Ent-23 entscheiden** — **erledigt 16.10.2026:** E2, ja, 1 Jahr | Lead | R2b.0 | Ent-21, Ent-22, Ent-23 ☑ | — | — |
| R2b.0b | **Nachzug der Spec** für den von der Empfehlung abweichenden Entscheid Ent-20 B — **erledigt 16.10.2026:** AKL.4, AKL.6–AKL.9, AKG.2–AKG.6 und `design.md` §18 nachgezogen | `spec-writer` | R2b.0a | Nachzug ☑ | — | — |
| R2b.1 | **ESP:** `immutable` nach `design.md` §18.6 — `h` gegen das `ETag` dieser Antwort, Dokument und `sw.js` ausgenommen, `200` und `304` gleich, `max-age=31536000`, `h` nie ausgegeben; Zähler `app_immutable`, `app_token_miss` in `device_ready`. Kodierung und Zeilenende von `http.cpp` vorher feststellen. **Prüfstand c70c** im Scratchpad | `esp-developer` | R2b.0b, G10.7 | **AKL.1–AKL.3** am Prüfstand | ☐ | — |
| R2b.2 | ESP-Teil bauen, RAM sofort ins Messprotokoll | Lead | R2b.1 | **AKZ.8**, **AKL.9** (RAM) | — | — |
| R2b.3 | **Werkzeug Inhaltsmarken** unter `tools/`: Marken aus den `.gz` (Formel `http_app_etag()`), sechs Verweise (Manifest eingeschlossen, Ent-22) in einer Kopie von `index.html` umschreiben, Meta-Element `wc-asset-tokens` **nur mit den Sprachdateien unter `i18n/`** füllen — `layout-previews.json` nie (Ent-20 B) —, alle Abbruchgründe aus AKL.4, Query-Prüfung aus AKL.5; dazu Eingaben für die Probe-Dokumente U2 (neue Marke für `styles.css`) und U3 (andere `v`). Je Abbruchgrund eine Gegenprobe | Lead | R2b.0b | **AKL.4**, **AKL.5** | ☐ | — |
| R2b.4 | **`Makefile`, Ziel `app-gz`:** erst alle `.gz` ausser `index.html`, dann das Werkzeug aus R2b.3, dann `index.html.gz` aus der Kopie über eine Zwischendatei — nie eine leere `.gz`; Abbruch bei Exit ≠ 0; `clean-app-gz` unverändert. Kein Bau (R1) — den Lauf fährt der Lead in R2b.8 | `release-engineer` | R2b.3 | **AKL.4** | ☐ | — |
| R2b.5 | **Markup:** `<meta name="wc-asset-tokens" content="">` im `<head>` von `index.html`, sonst nichts (Ent-21 E2). Kodierung und Zeilenende vorher feststellen | `ui-developer` | R2b.0b | **AKL.7** | ☐ | — |
| R2b.6 | **PWA:** Meta-Element einmal lesen, Marken nach Muster prüfen; die Sprachdateien in `ensureLanguageTable()` mit `?v=<APP_VERSION>&h=<marke>` bzw. ohne gültige Marke `?v=<APP_VERSION>`, weiter `cache: "default"`. **`loadLayoutPreviewRowsMap()` nicht anfassen** (Ent-20 B: ohne Query, `cache: "no-cache"`). **Prüfstand c70p nachziehen** | `pwa-developer` | R2b.0b | **AKL.5**, **AKL.6** | ☐ | — |
| R2b.7 | c70c, c70p ablegen, in S16; Gegenprobe gegen R2 bzw. 1.4.101; Sabotagen; Fallzahlen; c70e, c65b, c65w grün | Lead | R2b.2, R2b.6 | **AKL.1–AKL.3**, **AKL.5**, **AKL.6**, **AKZ.7** | ☐ | — |
| R2b.8 | **Werkzeuge:** `make app-gz` bei stillem Baum (R2) und Ergebnis prüfen (Fallzahl, Marken gegen die `.gz`, keine Marke für `layout-previews.json`); `install-app.sh --check` mit Markenprüfung; `smoke-device.sh` Block C70c; `ladeprobe --erwarte-cache` (Dokument und `layout-previews.json` auf der Leitung erwartet); `probe-u.sh` U1a, U1b, U2, U3; `check-pwa` INFO „aus dem Cache“; PWA-Vorschau mit der Quelle (AKL.7); `deploy.sh` mit dem neuen `app-gz` (`-nt`-Prüfung); je Gegenprobe | Lead | R2b.4, R2b.5, R2b.7 | **AKL.7** (Vorschau), **AKL.8**, **AKZ.2 (viii)** | ☐ | — |
| R2b.9 | **Review R2b** gegen `design.md` §18.10: Dokument und `sw.js` nie `immutable`; `immutable` nur bei Treffer, bei `200` und `304`; `h` nie ausgegeben; Vergleich ohne Heap; `layout-previews.json` ohne Marke und weiter `"no-cache"`; Werkzeug ohne stillen Rückfall; jede Query mit `=`; Kodierung und Zeilenenden | `code-reviewer` | R2b.8 | Checkliste | — | ☑ |
| R2b.10 | ESP-Version, `APP_VERSION`, `CACHE_NAME` | `release-engineer` | R2b.9 | S4 | ☐ | — |
| R2b.11 | `CHANGELOG.md`; **AKL.9** gegen das Tag von R2; `git status`; `--full`; RAM; `app.js.gz` | Lead | R2b.10 | **AKL.9**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R2b.12 | `TESTPLAN-PWA.md` (5b: `app_immutable`, `app_token_miss`; Ladeprobe `--erwarte-cache`; Probe U1a/U1b/U2/U3; `layout-previews.json` bleibt bei (a); Rückweg für Browser; Regel „Kopfänderung an einem App-Asset nur mit neuer `APP_VERSION`“), `BEFUNDE.md` (C70: Träger R2b). **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | R2b.11 | S9, S10, S11 | ☐ | — |
| R2b.13 | **Einspielzeilen:** erst OTA, dann `install-app.sh`; Rückweg (ESP auf R2, PWA auf 1.4.101) samt dem Satz, dass der ESP-Rückweg keine Einträge im Browser heilt und der Rückweg dafür eine neue `APP_VERSION` ist | Lead | R2b.11 | **AKZ.5** | — | — |
| R2b.14 | Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | R2b.12, R2b.13 | **AKZ.5** | ☐ | — |
| R2b.15 | **Einspielen** (Nutzer); Smoketest samt Update-Quelle und C70c, `--check` mit Markenprüfung, `check-pwa.sh` | Lead (Einspielen: Nutzer) | R2b.14 | **AKG.1** | — | — |

## Schritt G10b — Wirkung von (c) (auf R2b)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G10b.1 | **Ladeprobe Chrome `--erwarte-cache`**, drei Läufe, der dritte englisch; warm nur Dokument und `layout-previews.json` auf der Leitung; Gegenüberstellung mit G10.1 und G9b.3 | Lead | R2b.15 | **AKG.2** | — | — |
| G10b.2 | **Probe U1** — **U1a** `layout-previews.json` ersetzt, „sofort neu“ wie AKW.3; **U1b** `styles.css` ersetzt, Dokument gleich, kein `immutable` unter der alten Marke | Lead | G10b.1 | **AKG.3** | — | — |
| G10b.3 | **Probe U2** — `styles.css` mit neuer Marke, Dokument zuerst | Lead | G10b.2 | **AKG.4** | — | — |
| G10b.4 | **Probe U3** — Versionswechsel als Rückweg | Lead | G10b.3 | **AKG.5** | — | — |
| G10b.5 | **Browser-Sitzungen:** Chrome (`osascript`), Safari auf dem Mac (`osascript`), Safari auf **Deinem iPhone** (`manuell`), je mit Vorwärmen und einem Neuladen in S1; versioniertes Manifest geladen? (Ent-22) | Lead (Bedienung iPhone: Nutzer) | G10b.4 | **AKG.6** | — | — |
| G10b.6 | **Gesundheit und Grundlinie:** drei `check-pwa.sh`, Ruhewert 150 s danach, `watch-log.sh --pruefe` | Lead | G10b.5 | **AKG.7** | — | — |
| G10b.7 | **C70 nachführen** (Wirkung von (c), Safari-Ergebnis, Manifest). **Nicht gleichzeitig mit einer Messzeile** | `doc-writer` | G10b.6 | S10, S11 | ☐ | — |

---

## Schritt R3 — Release: Eingriff C72 (mit Ent-18 B auch L7) — **entfallen**

**Entfallen mit Ent-8 = kein Eingriff** (Nutzer, 16.10.2026, Tor G10.6; Zuordnung M5 auf G9b und G10, L468). L7 kam mit
Ent-18 A in R1b. Die Tasks bleiben zur Nachvollziehbarkeit stehen; keiner von ihnen wird ausgeführt, R3.0 eingeschlossen.

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

## Schritt G11 — Wirkung (auf R3) — **entfallen**

**Entfallen mit R3** (Ent-8). AKE.2 und AKE.3 gelten im Endstand als „entfallen“ mit diesem Grund.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G11.1 | **C72 — Browser-Sitzungen** wie G10.3 | Lead | R3.11 | **AKE.2** | — | — |
| G11.2 | **Nur mit Ent-18 B: C71** wie G9b.1 und G9b.2 | Lead | G11.1 | **AKS.2–AKS.4** | — | — |
| G11.3 | **Regression gegen die Grundlinie R2** | Lead | G11.2 | **AKE.3** | — | — |

---

## Schritt T3 — `pwa-tester` auf dem Endstand

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T3 | `pwa-tester` Phasen 0–4 und 9 auf dem Endstand **R2b**; **K30** (Ent-12): Ruhewert `max` < 5'120 oder `low=` < 1'072 ⇒ anhalten | `pwa-tester` | **G10b.7** (A.6; vorher: G11.3 oder G10.6 ohne R3) | **AKZ.6** | — | — |

## Schritt Z — Doku

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: Schliessungen **nur mit Beleg** — C70 (AKW.2–AKW.4 und **AKG.2–AKG.6**), C71 und C75 (AKS.1–AKS.4), C72 (M5 auf G9b und G10, Ent-8 — AKE.* entfallen), Beitrag zu C65 | `doc-writer` | T3 | S10, S11 | ☐ | — |
| Z.2 | Spec-Endstand: jedes Kriterium mit Beleg oder Grund | `spec-writer` | Z.1 | jedes AK belegt oder begründet | — | — |

---

## Abschluss

- [ ] Alle Tasks erledigt; R1b/G9b nur mit Ent-18 A; **R3 und G11 entfallen** (Ent-8); einzelne Tasks entfallen nur mit
      dem Entscheid, der sie ausschliesst — *Endstand 17.10.2026:* ohne Beleg 0.4, A.3, G9.7, R2.6; mit offenem Teil
      die Tasks, die im Endstand unten „Teil offen“ tragen
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release, Ergebnis im Messprotokoll — *Endstand:* nur vor R1
      im Protokoll (L468); vor R1b, R2 und R2b nicht ausgewiesen (AKZ.7)
- [x] Alle **57** Akzeptanzkriterien aus `requirements.md` erfüllt oder mit Grund offen (AKZ.9 erfüllt, L468; AKE.1–AKE.3
      entfallen mit Ent-8) — *Endstand (Z.2):* 34 belegt, 3 entfallen, 20 offen, jedes mit Beleg bzw. dem, was fehlt
      (`endstand.md`); keines gemessen und verfehlt
- [ ] Releases (R1, R1b, R2, **R2b**), je Flash-Umfang und Reihenfolge benannt, je committet, getaggt, gepusht und
      eingespielt — *Endstand:* je ZIP, Tag (lokal) und Einspielen belegt; Push und der Rückweg für R1, R1b, R2 nicht
      ausgewiesen (AKZ.5)

---

## Endstand 17.10.2026

**Z.2, `spec-writer`, Stand `ccdd88b`.** Endstand je Task mit Beleg oder Grund; den Endstand jedes Kriteriums trägt
`endstand.md` (dieser Ordner). Quelle ist `BEFUNDE.md` in diesem Stand, vor allem das Messprotokoll L468 und T3 (L493),
dazu `CHANGELOG.md` und `TESTPLAN-PWA.md`.

**Zu den Spalten oben:** **R** ist nachgezogen, wo ein Review belegt ist — R1.7 (Nachtrag A.4), R2.7 (`CHANGELOG.md`
3.2.40 „ohne Einwände“; Hinweise L479–L481), R2b.9 (bestanden, SOLL-1 bis SOLL-3 in `cf2d03b`). **R1b.4** bleibt ☐:
Das Review lief (Hinweise L476, L477), eine Freigabe steht nicht im Protokoll. **0.4** bleibt ☐: kein Beleg. **G** bleibt
überall ☐, weil das Messprotokoll keinen Guardrail-Lauf je Task ausweist (AKZ.7) — die S16-Zahlen je Release stehen in
der Tabelle unten.

**Endstände der Tasks:** **erledigt** (mit Beleg), **Teil offen** (erledigt, ein verlangter Teil ohne Beleg),
**offen** (ohne Beleg), **entfallen** (mit Entscheid).

| # | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| 0.1 | Teil offen | L468, 0.1; Grösse jeder der 13 Dateien und `httpclient.cpp` fehlen (AKZ.1) |
| 0.2 | Teil offen | Ladeprobe im Einsatz ab 0.4 (L468); Prüfserver-Gegenprobe 22:21–22:23; Fehlalarm-Korrektur `db47c92` mit Gegenprobe; das Review 0.4 fehlt (AKZ.2 (i)) |
| 0.3 | Teil offen | `browser-sitzung` in G9b.4, G10.3, G10b.5; P1-Probe `tools/p1-probe.py` mit Attrappen 2/1/0; Messung AKS.4 in G9b.1; Gegenprobe „Uhr nicht erreichbar“ fehlt (AKZ.2 (ii)) |
| 0.4 | offen | kein Beleg für ein Review der Werkzeuge |
| 0.5 | erledigt | L468, Trennmessung (AKZ.9) |
| A.1 | erledigt | Bericht P1–P12 in `design.md` §12 |
| A.2 | erledigt | Nachträge A.2 bis A.5 in `design.md` §17 |
| A.3 | offen | im Protokoll nicht ausgeführt; seit L7 ohne Folge (Ruhewert nach dem Start 7'152, G9b.2) |
| A.4 | erledigt | A.6 freigegeben 16.10.2026 |
| R1.1 | erledigt | Commit `6231f86`, c72i (AKI.1, AKI.2) |
| R1.2 | erledigt | L468: statisches RAM 69'060, ±0 gegen E4 |
| R1.3 | erledigt | Commit `3643464`, c71i (AKI.3) |
| R1.4 | erledigt | L468: 69'092, +32 gegen E4 |
| R1.5 | erledigt | c72i 17 und c71i 20 Sabotagen, je gegen den Vorstand FEHL; S16 78/78 |
| R1.6 | Teil offen | Smoketest mit Gegenprobe `SMOKE_DR_OVERRIDE` (L468); Gegenprobe der Logwache fehlt (AKZ.2 (iv)) |
| R1.7 | erledigt | Nachtrag A.4 |
| R1.8 | erledigt | ESP 3.2.38 |
| R1.9 | Teil offen | `CHANGELOG.md` 3.2.38; `--full` 0 Kritisch (L468); AKZ.4 nicht belegt |
| R1.10 | erledigt | `TESTPLAN-PWA.md` (I-5-Felder, Lochsumme); `BEFUNDE.md` (L472; Träger an C71, C72) |
| R1.11 | Teil offen | Einspielzeile „nur der ESP (OTA)“ im `CHANGELOG.md` 3.2.38; Rückweg nicht ausgewiesen (AKZ.5) |
| R1.12 | Teil offen | ZIP `…-2026-10-16-010338.zip`, Tag `release/3.2.34-3.2.38-1.4.100` (lokal); Release-Commit und Push nicht ausgewiesen |
| R1.13 | erledigt | AKM.1 |
| G9.1 | erledigt | AKM.2 |
| G9.2 | erledigt | AKM.7 |
| G9.3–G9.5 | entfallen | Ent-18 A: gefahren als G9b.4 auf R1b |
| G9.6 | erledigt | AKM.5 |
| G9.7 | offen | Gesundheit des G9-Fensters nicht protokolliert (AKM.6) |
| G9.8 | erledigt | Zuordnung N5 in L468 |
| G9.9 | erledigt | C71 mit Ursache, M1/M3, Start 3/4 und der Lücke aus L461 nachgeführt (C71, C72, L461) |
| G9.10 | erledigt | Ent-17 und Ent-18 A am 16.10.2026 (L468) |
| R1b.1 | erledigt | Commit `cb01fcd`, c71e (AKS.1); Aufruferliste im Bericht nicht im Repository |
| R1b.2 | erledigt | statisch 0, Programm 534'480 (`CHANGELOG.md` 3.2.39; L468 nur als Paketsumme) |
| R1b.3 | erledigt | c71e in S16 79/79, 10 von 10 Sabotagen, gegen `e362d28` FEHL |
| R1b.4 | Teil offen | Review lief (L476, L477); Freigabe nicht protokolliert |
| R1b.5 | erledigt | ESP 3.2.39 |
| R1b.6 | Teil offen | `CHANGELOG.md` 3.2.39; AKZ.4 und `--full` nicht belegt |
| R1b.7 | erledigt | `TESTPLAN-PWA.md` (Freistellen entfällt, Messung AKS.4); Träger R1b an C71 und C75 |
| R1b.8 | Teil offen | ZIP `…-080958.zip`, Tag, Commit `d4c5cee`; Rückweg und Push nicht ausgewiesen |
| R1b.9 | erledigt | L468: 08:14, Smoketest 34/0, `--check` vollständig, `check-pwa` 9/0 |
| G9b.1 | erledigt | AKS.4, 3/3 |
| G9b.2 | Teil offen | AKS.2 erledigt; AKS.3 ohne „keine `*.tmp`“ und `update_table_files` |
| G9b.3 | erledigt | Grundlinie 200'724–223'176 Byte |
| G9b.4 | Teil offen | AKM.3, AKM.4 erledigt; der eine Vergleichslauf mit Freistellen des Clients ist nicht ausgewiesen |
| G9b.5 | erledigt | `--pruefe` 10:47, Zuordnung M5 vorläufig |
| G9b.6 | erledigt | C71, C72, C75 mit „Stand G9b“ nachgeführt |
| R2.0 | erledigt | Ent-2 T (L468) |
| R2.1 | erledigt | Commit `aea887c`, c70e |
| R2.2 | erledigt | 69'140, +48; Paketsumme +80 (L468) |
| R2.3 | erledigt | Commit `8cf4b07`, c70p |
| R2.4a | Teil offen | nachgezogen, grün; Fallzahl gegen vorher fehlt (AKC.8) |
| R2.4 | erledigt | c70e 27 und c70p 7 Sabotagen, gegen den Vorstand rot; S16 81/81 |
| R2.5 | Teil offen | `check-pwa` `9774642`; Smoketest-Block C70 (`28e63fe`, `3824291`); Gegenproben nicht protokolliert (AKZ.2 (iii), (v)) |
| R2.6 | offen | kein Beleg |
| R2.7 | erledigt | Review R2.7; Hinweise L479–L481, H1, H4 |
| R2.8 | erledigt | ESP 3.2.40, PWA 1.4.101 |
| R2.9 | Teil offen | `CHANGELOG.md` 3.2.40; RAM; AKZ.4 und `--full` nicht belegt |
| R2.10 | erledigt | `TESTPLAN-PWA.md` 5b (`app_not_modified`, `app_full`); C70 „Stand R2“ |
| R2.11 | Teil offen | Reihenfolge „erst OTA, dann App“ im `CHANGELOG.md`; Rückweg nicht ausgewiesen |
| R2.12 | Teil offen | ZIP `…-113409.zip`, Tag, Commit `820a232`; Push nicht ausgewiesen |
| R2.13 | erledigt | AKW.1 |
| G10.1 | Teil offen | AKW.2 erledigt; der Teil AKC.4 (Mehrdauer je Anfrage) fehlt |
| G10.2 | erledigt | AKW.3 |
| G10.3 | erledigt | AKW.4 |
| G10.3a | erledigt | AKW.6 |
| G10.4 | erledigt | AKW.5 Teil 1, Gesundheit |
| G10.5 | erledigt | `99cfc6f` |
| G10.6 | erledigt | Ent-8 kein Eingriff, 16.10.2026 |
| G10.7 | erledigt | weiter mit (c), 16.10.2026 |
| R2b.0, R2b.0a, R2b.0b | erledigt | Entscheide und Nachzug am 16.10.2026 |
| R2b.1 | erledigt | Commit `ffd1184`, c70c |
| R2b.2 | erledigt | 69'136, ±0 gegen R2; Programm +608 |
| R2b.3 | erledigt | `tools/app-marken.py` (`9b9698a`), 14 Gegenproben |
| R2b.4 | erledigt | `Makefile` `057f83d` |
| R2b.5 | erledigt | `d590c99` |
| R2b.6 | erledigt | `28281a3` |
| R2b.7 | erledigt | c70c 339/26, c70p 225/23, gegen den Stand davor rot; S16 82/82 |
| R2b.8 | Teil offen | Werkzeuge `8524d61`, `373ba2d`, `cbc80ec`, `89acb14`, `cee7087`, `5c6622f`; Gegenproben (AKL.8) und Vorschau (AKL.7) nicht protokolliert |
| R2b.9 | erledigt | bestanden, SOLL-1 bis SOLL-3 in `cf2d03b`; Hinweise L483, L484 |
| R2b.10 | erledigt | ESP 3.2.41, PWA 1.4.102 |
| R2b.11 | Teil offen | `CHANGELOG.md` 3.2.41; RAM; Dateiliste gegen R2 (AKL.9) und `--full` nicht belegt |
| R2b.12 | erledigt | `TESTPLAN-PWA.md` 5b und Probe U1a bis U3, Regel zur Kopfänderung; C70 „Stand R2b“ |
| R2b.13 | erledigt | `CHANGELOG.md` 3.2.41: Reihenfolge, Rückweg, Satz zum Browser-Rückweg |
| R2b.14 | Teil offen | ZIP `…-145745.zip`, Tag, Commit `5f1491e`; Push nicht ausgewiesen |
| R2b.15 | erledigt | AKG.1 |
| G10b.1 | Teil offen | AKG.2 ohne Pfadabgleich gegen G10.1 |
| G10b.2 | Teil offen | AKG.3 ohne Gegenprobe ohne Upload |
| G10b.3 | Teil offen | AKG.4 ohne Gegenprobe ohne Upload |
| G10b.4 | Teil offen | AKG.5 ohne Gegenprobe ohne Upload |
| G10b.5 | Teil offen | AKG.6 ohne Manifest-Vermerk für Safari am Mac |
| G10b.6 | erledigt | AKG.7 |
| G10b.7 | erledigt | C70 „Stand G10b“ |
| R3.0–R3.11 | entfallen | Ent-8 kein Eingriff (16.10.2026) |
| G11.1–G11.3 | entfallen | mit R3 (Ent-8) |
| T3 | erledigt | L493, AKZ.6; Phasen 5–8 bleiben beim Nutzer |
| Z.1 | erledigt | Commit `ccdd88b`: C70, C71, C75 erledigt, C72 ohne Eingriff geschlossen, C65 offen |
| Z.2 | erledigt | diese Tabelle und `endstand.md` |

**Zählung der Tasks** (Sammelzeilen einzeln: G9.3–G9.5 als 3, R2b.0 bis R2b.0b als 3, R3.0–R3.11 als 12, G11.1–G11.3
als 3): **113 Tasks — 65 erledigt, 26 Teil offen, 4 offen (0.4, A.3, G9.7, R2.6), 18 entfallen.**
