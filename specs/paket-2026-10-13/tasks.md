# Tasks — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (A15, A56, A70, A79, A80, A81, A82, B8, B48, B55, C6, C6b, C6v, C8, C19, C46, C59, C60, E36, F3)

**Erstellt:** 2026-10-13, Stand `159c9d0`; am selben Tag mit den Entscheidungen des Nutzers nachgeführt.
Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.

**Status: freigegeben** — vom Nutzer am 13.10.2026 (Ent-16), samt Ent-1 bis Ent-14 und Ent-17. **Eine
Entscheidung gegen die Empfehlung:** Ent-11 (C6 einstufig). **Ent-15 (Messbau für A15) aufgehoben
13.10.2026** (L432).

## Entscheidungen (13.10.2026)

| Ent | Gegenstand | Entschieden | wirkt auf |
|---|---|---|---|
| **Ent-1** | A56 | Ambilight-Power im EEPROM | S2.3 |
| **Ent-2** | A70 | (c), (d), (e), L399 beheben; (a), (b) ohne Fix | S2.11 |
| **Ent-3** | A79 | `setRxBufferSize(1024)` vor `Serial.begin`, `ovr` beim Start belegen | E2.3, E2.14 |
| **Ent-4** | A80 | Icon bei ausgeschaltetem Display verwerfen; Restore-Bedingung nicht vereinfachen | S2.7 |
| **Ent-5** | C8 | Brücke aufweiten, RAM messen (präzisiert durch Ent-10) | E2.2 |
| **Ent-6** | B55 | API wartet das Ergebnis des STM ab; STM-Rückmeldung `RPCR` | S2.1, E2.1, EP.6 |
| **Ent-7** | C19 | nur L311 | E1.4 |
| **Ent-8** | F4 | eigenes Folgepaket | — |
| **Ent-9** | A56 Speicherort | zwei Byte hinter dem Layoutende, ohne Layoutversion | S2.3 |
| **Ent-10** | C8 | Puffer 260 Byte, Folgeeinträge im Ring, kein Ring-RAM | E2.2 |
| **Ent-11** | C6 | **einstufig** (gegen die Empfehlung): nur noch POST; ESP und PWA als **ein** Release EP, direkt nacheinander eingespielt, ESP zuerst; Werkzeuge vorher auf POST; kein Backup-Export dazwischen | EP |
| **Ent-12** | C6 Umfang | Herkunft über `Sec-Fetch-Site` für alle zerstörenden Wege samt Legacy (`rsteeprom2`, `rststm32`, C60) und `/fs` (C6v); POST für die vier Wartungsendpunkte und `network_secrets`; Legacy bleibt GET; Update-Auslöser, `ir_code_set`, `learn_ir`, `test_display` unverändert; **C60 eigener Task, hoch** | E1.1, E1.2, EP.1 |
| **Ent-13** | Gerätenachweise | **alle freigegeben**, auch STM-Reset im Nacht-Aus-Fenster für A70 (e); `caffeinate` bei geplanten Tests (DIR-020) | alle G |
| **Ent-14** | Gate-Riss in S2 | A70 ohne Rückfrage nach S3 mit Flash-Analyse | S2.11, S3 |
| ~~**Ent-15**~~ | A15 Messung | **aufgehoben 13.10.2026** (Entscheidung des Nutzers, L432): A15 bleibt, **ohne** Messbau — seit LTO kein `bl` in der ISR, der Gewinn sind wenige Befehle. Statische Messung (AKI.2) und Regressionslauf (AKI.3) bleiben | M.1–M.4, S1.9, S1.10, S1.13 (entfallen bzw. reines Einspielen) |
| **Ent-16** | Spec | **freigegeben** | alle |
| **Ent-17** | A81, A82 | in S2 aufgenommen, soweit das Budget trägt; sonst Nachrang (A82 nach S3) | S2.5, S2.9 |

**Vom Lead festgelegt (Vorpaket, gilt weiter):** `CHANGELOG.md` je Release schreibt der Lead. Matrix-Lauf
vor jedem STM-Release, das Flash kostet, nach der Versionsanhebung.

---

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst
bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wo die Dateien disjunkt sind** (R3b,
`design.md` §0.1):

1. **Code des nächsten Releases erst nach dem Release-Build des vorigen:** S2.1 nach S1.12, E1.1 nach S2.22
   (bzw. S3.6), E2.1 nach E1.12, EP.1 nach E2.12, P2.1 nach EP.16.
2. **Release-Build erst nach dem Gerätenachweis des vorigen:** S2.22 nach S1.14 (G1), E1.12 nach S2.24
   (G2) bzw. S3.8 (G2b), E2.12 nach E1.14 (G3), EP.16 nach E2.14 (G4), P2.11 nach T1. STM-Builds
   zusätzlich nach grünem Matrix-Lauf (S1.11, S2.21, S3.5).
3. **Innerhalb eines STM-Releases nacheinander, Flash-Gate nach jedem Task** (AKM.1).
4. **EP ist ein Release mit zwei Komponenten:** Zwischen ESP-OTA und PWA-Upload liegt nichts (Ent-11).

**Messwerte sofort (L424):** Jeder messende Task hängt sein Ergebnis **vor dem nächsten Task** an das
Messprotokoll in `BEFUNDE.md` an. Messzeilen schreibt der Lead, Befundnachführungen der `doc-writer` — nie
gleichzeitig (R3).

**Einspielen:** Einspielzeilen je Release fertig (AKZ.8). Jeder Gerätelauf mit `./tools/watch-log.sh`, vor
dem Einspielen gestartet, `--pruefe` am Ende (AKZ.5); auf später geplante Tests mit `caffeinate`
(DIR-020). **Nie am Gerät:** `design.md` §17.3.

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. Prüfstände baut der
Umsetzer im Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab und fährt die Gegenprobe
(DIR-014). Vor jedem Patch Kodierung **und** Zeilenende feststellen (AKZ.9) — `ESP-uclock.ino` gemischt,
`http.cpp` UTF-8.

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — ohne Firmware

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **E36:** Ursache (`pmset -g log`), Wache umbauen nach `design.md` §16 | Lead | Freigabe | **AKW.1–AKW.5** | ☐ | — |
| 0.2 | **Review 0.1** | `code-reviewer` | 0.1 | Weg jeder Meldung bis zum Empfänger (DIR-014) | — | ☐ |
| 0.3 | **F3** nach §15 | Lead | Freigabe | **AKB.1–AKB.3** | ☐ | — |
| 0.4 | **Review 0.3** | `code-reviewer` | 0.3 | sieht die Stufe ihren Gegenstand vollständig (L235)? | — | ☐ |
| 0.5 | **B8:** Zählwerkzeug, Zuordnung, Zahl sofort bei B8 | Lead | Freigabe | **AKD.1** | ☐ | — |
| 0.6 | **`flash-stm.sh --lokal`** (§2.3) und Probe in `checks/hooks.sh`, dass `no-danger.py` Gefahrenendpunkte auch mit `curl -X POST` abweist. **`--lokal` ist gebaut und bleibt als Werkzeug**; in diesem Paket wird es nicht eingesetzt (Ent-15 aufgehoben) | Lead | Freigabe | **AKZ.12**; Hook-Probe angeschlagen | ☐ | — |
| 0.7 | **G0 — Grundlinien** nach AKG.0 (Disassembly 3.2.32 im Worktree, Setter-Burst, Heap, `stm32_log`) | Lead | **0.1** | **AKG.0** | — | — |
| 0.8 | **Befunde:** „Träger: Paket 2026-10-13" an allen Einträgen; neue Zeilen für die zweite C19-Stelle, den 120er-Ring, die Rechnung Netzwerkimport | `doc-writer` | Freigabe | **AKG.1**; S10, S11 | ☐ | — |

## ~~Messbau M0 — vorher~~ — entfallen (Ent-15 aufgehoben 13.10.2026, L432)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| M.1 | ~~Messcode als Patch~~ — **erledigt** vor der Aufhebung | `stm-developer` | — | Patch liegt; **bleibt, wird nicht eingesetzt** | — | — |
| M.2 | ~~Patch ablegen~~ — **erledigt**: `tools/checks/auszug/stm/a15/messbau.patch` | Lead | — | **bleibt, wird nicht eingesetzt** | — | — |
| M.3 | ~~Versionszusatz `-m0` im Worktree~~ — **entfallen** | — | — | — | — | — |
| M.4 | ~~M0 bauen, lokal flashen, messen, zurück~~ — **entfallen** | — | — | — | — | — |

---

## Schritt S1 — STM-Release 1 (isoliert): A15

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S1.1 | **A15** nach §2.1; Prüfpunkt `uart_putc()`/`CR1`. **Prüfstand a15** im Scratchpad | `stm-developer` | Freigabe | **AKI.1** am Prüfstand | ☐ | — |
| S1.2 | **Flash-Gate** F103 und F411, sofort ins Messprotokoll | Lead | S1.1 | **AKM.1** | — | — |
| S1.3 | Prüfstand a15 ablegen, Gegenprobe; Disassembly nachher gegen den Worktree-Bau des Tags | Lead | S1.2, 0.7 | **AKI.1**, **AKI.2** | ☐ | — |
| S1.4 | **Review S1** | `firmware-analyst` | S1.3 | Gleichwertigkeit, beide Plattformen | — | ☐ |
| S1.5 | STM-Version anheben | `release-engineer` | S1.4 | S4 | ☐ | — |
| S1.6 | `CHANGELOG.md`-Eintrag S1 | Lead | S1.4 | — | ☐ | — |
| S1.7 | AKZ.3, ins Messprotokoll | Lead | S1.5, S1.6 | **AKZ.3** | — | — |
| S1.8 | `--full`, ins Messprotokoll | Lead | S1.7 | **AKZ.2** | ☐ | — |
| S1.9 | ~~Worktree mit Messpatch, Zusatz `-m1`~~ — **entfallen** (Ent-15 aufgehoben) | — | — | — | — | — |
| S1.10 | ~~M1 bauen~~ — **entfallen** (Ent-15 aufgehoben) | — | — | — | — | — |
| S1.11 | **Matrix-Lauf** nach der Versionsanhebung | Lead | S1.8 | **AKM.3** | — | — |
| S1.12 | Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | S1.11 | **AKZ.7** | ☐ | — |
| S1.13 | **Einspielen des S1-Release:** Wache starten, `flash-stm.sh --check`, `flash-stm.sh`, Version nachgewiesen, Smoketest sofort ins Messprotokoll (der Messteil entfällt) | Lead | S1.12 | **AKZ.4** | — | — |
| S1.14 | **G1:** Setter-Burst wie G0, 60 min lesend mit `caffeinate` (Regressionslauf) | Lead | S1.13 | **AKI.3** | — | — |

---

## Schritt S2 — STM-Release 2: B55-STM, A56, A81, A80, A82, A70

Reisst das Gate bei A82: A82 nach S3 (Ent-17). Bei A70: A70 nach S3 (Ent-14). Vorher: anhalten und vorlegen.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S2.1 | **B55-STM** nach §6.1. **Prüfstand l55** | `stm-developer` | **S1.12** | **AKL.1** | ☐ | — |
| S2.2 | Flash-Gate | Lead | S2.1 | **AKM.1** | — | — |
| S2.3 | **A56** nach §3.1–§3.3; Prüfpunkt `BLACK_BOARD`. **Prüfstand a56** | `stm-developer` | S2.2 | **AKA.1**, **AKA.2** | ☐ | — |
| S2.4 | Flash-Gate | Lead | S2.3 | **AKM.1** | — | — |
| S2.5 | **A81** nach §3.4 (samt Klemmung, falls fehlend). **Prüfstand a81** | `stm-developer` | S2.4 | **AKA.4** | ☐ | — |
| S2.6 | Flash-Gate | Lead | S2.5 | **AKM.1** | — | — |
| S2.7 | **A80** nach §5; Restore-Bedingung nicht anfassen. **Prüfstand a80** | `stm-developer` | S2.6 | **AKC.1** | ☐ | — |
| S2.8 | Flash-Gate | Lead | S2.7 | **AKM.1** | — | — |
| S2.9 | **A82** nach §3.5. **Prüfstand a82** | `stm-developer` | S2.8 | **AKA.5** | ☐ | — |
| S2.10 | Flash-Gate; bei Riss A82 zurücknehmen und nach S3 (Ent-17) | Lead | S2.9 | **AKM.1** | — | — |
| S2.11 | **A70** nach §4; Kommentar A78 **nicht** anfassen. **Prüfstand a70** | `stm-developer` | S2.10 | **AKN.1–AKN.5** | ☐ | — |
| S2.12 | Flash-Gate; bei Riss A70 zurücknehmen und nach S3 (Ent-14) | Lead | S2.11 | **AKM.1** | — | — |
| S2.13 | Prüfstände l55, a56, a81, a80, a82, a70 ablegen, Gegenproben gegen 3.2.32; a69 OK | Lead | S2.12 | Fallzahlen | ☐ | — |
| S2.14 | Wache: `icon: verworfen` (Hinweis), `icon_freeze: ENTER` (Alarm), je Gegenprobe | Lead | S2.13 | **AKZ.5** | ☐ | — |
| S2.15 | **Review S2**, ausdrücklich Restore-Invariante | `firmware-analyst` | S2.14 | **AKN.5** | — | ☐ |
| S2.16 | STM-Version anheben | `release-engineer` | S2.15 | S4 | ☐ | — |
| S2.17 | `CHANGELOG.md`-Eintrag S2 | Lead | S2.15 | — | ☐ | — |
| S2.18 | `TESTPLAN-PWA.md`: A56, A80, A70 (e) | `doc-writer` | S2.15 | S9, S11 | ☐ | — |
| S2.19 | AKZ.3 | Lead | S2.16, S2.17 | **AKZ.3** | — | — |
| S2.20 | `--full` | Lead | S2.19 | **AKZ.2** | ☐ | — |
| S2.21 | Matrix-Lauf | Lead | S2.20 | **AKM.3** | — | — |
| S2.22 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | S2.21, **S1.14** | **AKZ.7** | ☐ | — |
| S2.23 | Einspielen, Smoketest | Lead | S2.22 | **AKZ.4** | — | — |
| S2.24 | **G2:** Tagteil (Ambilight, Icon), Nachtteil mit `caffeinate` (Timer, Reset im Aus-Fenster für A70 (e)) | Lead | S2.23 | **AKA.3**, **AKC.2**, **AKN.6** | — | — |

## Schritt S3 — nur bei Gate-Riss in S2 (Ent-14, Ent-17)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S3.1 | **Flash-Analyse** an der Map des S2-Builds und der engsten Variante (§1.2) | `firmware-analyst` | S2.24 | Kandidatenliste mit Bytes laut Map | — | — |
| S3.2 | Verhaltensneutrale Einsparungen, **einzeln**, der Lead misst nach jeder | `stm-developer` | S3.1 | Gewinn je Einsparung im Messprotokoll | ☐ | — |
| S3.3 | **A70** (und **A82**, falls zurückgestellt) wie S2.9/S2.11, Gate nach jedem | `stm-developer` | S3.2 | **AKN.1–AKN.5**, **AKA.5** | ☐ | — |
| S3.4 | **Review S3** | `firmware-analyst` | S3.3 | Invariante | — | ☐ |
| S3.4a | STM-Version anheben | `release-engineer` | S3.4 | S4 | ☐ | — |
| S3.5 | `CHANGELOG.md`, AKZ.3, `--full`, Matrix | Lead | S3.4a | **AKZ.2**, **AKZ.3**, **AKM.3** | ☐ | — |
| S3.6 | Build, Commit + Tag + Push | Lead | S3.5 | **AKZ.7** | ☐ | — |
| S3.7 | Einspielen, Smoketest | Lead | S3.6 | **AKZ.4** | — | — |
| S3.8 | **G2b** wie die A70-/A82-Teile von G2 | Lead | S3.7 | **AKN.6** | — | — |

---

## Schritt E1 — ESP-Release 1: Herkunft für alle zerstörenden Wege, C46, C19

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E1.1 | **Herkunft** nach §8.1 für alle API-Wege und `/fs` (**C6v**); Bestandsliste aller Legacy-Wege zu `stm32_reset()`/`RESET_EEPROM_RPC_VAR` | `esp-developer` | **S2.22** bzw. S3.6 | **AKH.1** | ☐ | — |
| E1.2 | **C60 (hoch):** Legacy `rsteeprom2`, `rststm32` und die übrigen Legacy-Reset-Wege mit Herkunftsprüfung | `esp-developer` | E1.1 | **AKH.2** | ☐ | — |
| E1.3 | **C46** (ESP) nach §10, Kennung 9 | `esp-developer` | E1.2 | **AKQ.1** | ☐ | — |
| E1.4 | **C19** (nur L311) an beiden Stellen | `esp-developer` | E1.3 | **AKU.1** | ☐ | — |
| E1.5 | Prüfstände c6h, c46, c19 ablegen, Gegenproben gegen 3.2.33 | Lead | E1.4 | **AKH.3** | ☐ | — |
| E1.6 | Wache: `- abgewiesen: query`, Gegenprobe | Lead | E1.5 | **AKZ.5** | ☐ | — |
| E1.7 | **Review E1** | `code-reviewer` | E1.6 | Checkliste, Sicherheit | — | ☐ |
| E1.8 | ESP-Version anheben | `release-engineer` | E1.7 | S4 | ☐ | — |
| E1.9 | `CHANGELOG.md`, AKZ.3, `--full` | Lead | E1.8 | **AKZ.2**, **AKZ.3** | ☐ | — |
| E1.10 | `TESTPLAN-PWA.md`: Herkunft, C46 | `doc-writer` | E1.7 | S9, S11 | ☐ | — |
| E1.11 | Einspielzeilen | Lead | E1.9 | **AKZ.8** | — | — |
| E1.12 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | E1.11, **S2.24** bzw. S3.8 | **AKZ.7** | ☐ | — |
| E1.13 | ESP-OTA, Smoketest samt Update-Quelle, `install-app.sh --check` | Lead | E1.12 | **AKZ.4** | — | — |
| E1.14 | **G3** | Lead | E1.13 | **AKH.4**, **AKQ.2** | — | — |

---

## Schritt E2 — ESP-Release 2: Brücke und Netz

Nach jedem E2-Task misst der Lead „Globale Variablen" und schreibt sie sofort ins Messprotokoll (AKM.2).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E2.1 | **B55-ESP** nach §6.2 | `esp-developer` | **E1.12** | **AKL.2** | ☐ | — |
| E2.2 | **C8** nach §7 | `esp-developer` | E2.1 | **AKK.1** | ☐ | — |
| E2.3 | **A79** nach §12 | `esp-developer` | E2.2 | **AKE.1** | ☐ | — |
| E2.4 | **C59** nach §11 | `esp-developer` | E2.3 | **AKT.1** | ☐ | — |
| E2.5 | Prüfstände l55e, c8, ntp, Gegenproben; statische Prüfung A79 | Lead | E2.4 | Fallzahlen | ☐ | — |
| E2.6 | Wache: `- rpc keine Antwort`, Gegenprobe | Lead | E2.5 | **AKZ.5** | ☐ | — |
| E2.7 | **Review E2** (Wiedereintritt, Quittungen beim Warten) | `code-reviewer` | E2.6 | Checkliste | — | ☐ |
| E2.8 | ESP-Version anheben | `release-engineer` | E2.7 | S4 | ☐ | — |
| E2.9 | `CHANGELOG.md`, AKZ.3, `--full` | Lead | E2.8 | **AKZ.2**, **AKZ.3** | ☐ | — |
| E2.10 | `TESTPLAN-PWA.md`: B55 (ESP), C8 | `doc-writer` | E2.7 | S9, S11 | ☐ | — |
| E2.11 | Einspielzeilen | Lead | E2.9 | **AKZ.8** | — | — |
| E2.12 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | E2.11, **E1.14** | **AKZ.7** | ☐ | — |
| E2.13 | ESP-OTA, Smoketest, `install-app.sh --check` | Lead | E2.12 | **AKZ.4** | — | — |
| E2.14 | **G4** mit `caffeinate` | Lead | E2.13 | **AKM.2**, **AKE.2**, **AKK.2**, **AKT.2** | — | — |

---

## Schritt EP — ein Release für ESP und PWA (Ent-11)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| EP.1 | **C6, ESP:** POST-Zweig nach §8.2, nur POST für die fünf Endpunkte, GET ⇒ 405, anderer POST ⇒ 404 | `esp-developer` | **E2.12** | **AKH.5** | ☐ | — |
| EP.2 | **C6b, ESP** nach §8.3 | `esp-developer` | EP.1 | **AKS.1**, **AKS.2** | ☐ | — |
| EP.3 | Prüfstände c6p, c6b ablegen, Gegenproben gegen den E2-Stand | Lead | EP.2 | **AKH.5**, **AKS.2** | ☐ | — |
| EP.4 | **C6 und C6b, PWA** (`apiPost`, Export über `network_secrets`) | `pwa-developer` | EP.3 | **AKH.6**, **AKS.3** | ☐ | — |
| EP.5 | **C46, PWA** | `pwa-developer` | EP.4 | **AKQ.3** | ☐ | — |
| EP.6 | **B55, PWA**; Kennungen 9–11 in DE und EN | `pwa-developer` | EP.5 | **AKL.3** | ☐ | — |
| EP.7 | PWA-Prüfstände (Mock); **Werkzeuge auf POST** (`flash-stm.sh`, `smoke-device.sh`, `snapshot-device.sh`, `preview/server.py`), gegen Vorschau und c6p gelaufen | Lead | EP.6 | **AKZ.11** | ☐ | — |
| EP.8 | **Review EP** (ESP und PWA zusammen; Leseschleife nach C53) | `code-reviewer` | EP.7 | Checkliste | — | ☐ |
| EP.9 | `TESTPLAN-PWA.md`: POST, Backup-Export, B55 | `doc-writer` | EP.8 | S9, S11 | ☐ | — |
| EP.10 | ESP-Version, `APP_VERSION` und `CACHE_NAME` | `release-engineer` | EP.8 | S4 zeigt beide | ☐ | — |
| EP.11 | `CHANGELOG.md`-Eintrag EP (beide Komponenten) | Lead | EP.8 | — | ☐ | — |
| EP.12 | AKZ.3 (ESP **und** PWA), `git status` (R2) | Lead | EP.10, EP.11 | **AKZ.3** | — | — |
| EP.13 | `--full` | Lead | EP.12 | **AKZ.2** | ☐ | — |
| EP.14 | `CLAUDE.md`: Gefahrentabelle und Hinweise auf POST | Lead | EP.13 | S9, S11, `endpunkte.mjs` | ☐ | — |
| EP.15 | Einspielzeilen: OTA, **unmittelbar** `install-app.sh`; Warnung „kein Backup-Export dazwischen"; Rückweg Legacy (§8.4) | Lead | EP.13 | **AKZ.8** | — | — |
| EP.16 | Build, ZIP, Rollout, **ein** Commit + Tag + Push | Lead | EP.15, **E2.14** | **AKZ.7** | ☐ | — |
| EP.17 | **Einspielen:** ESP-OTA, sofort `install-app.sh`; danach Smoketest samt Update-Quelle, `--check`, `check-pwa.sh` | Lead | EP.16 | **AKG.5** | — | — |
| EP.18 | **G5** | Lead | EP.17 | **AKH.7**, **AKL.4**, **AKS.4** | — | — |
| T1 | `pwa-tester` Phasen 0–4 und 9 | `pwa-tester` | EP.18, EP.9 | **AKZ.6** | — | — |

---

## Schritt P2 — PWA-Release: B48, B8

P2.3/P2.4 dürfen im Worktree (R6) vorgezogen werden; in den Hauptbaum erst nach EP.16.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P2.1 | `.field-error` (B48) | `ui-developer` | **EP.16** | Kontrast gemessen | ☐ | — |
| P2.2 | **B48** nach §13 | `pwa-developer` | P2.1 | **AKF.1** | ☐ | — |
| P2.3 | **B8** in `app.js` und `en.json` | `pwa-developer` | P2.2, 0.5 | 0 offene Texte in `app.js` | ☐ | — |
| P2.4 | **B8** in `index.html` | `ui-developer` | P2.3 | 0 offene Texte in `index.html` | ☐ | — |
| P2.5 | B8-Werkzeug als Guardrail-Stufe; Prüfstand B48; Gegenproben | Lead | P2.4 | **AKD.2**, **AKF.1** | ☐ | — |
| P2.6 | **Review P2** | `code-reviewer` | P2.5 | Checkliste | — | ☐ |
| P2.7 | `TESTPLAN-PWA.md`: B48 (iPhone), EN-Stichprobe | `doc-writer` | P2.6 | S9, S11 | ☐ | — |
| P2.8 | `APP_VERSION` und `CACHE_NAME` | `release-engineer` | P2.6 | S4 | ☐ | — |
| P2.9 | `CHANGELOG.md`, AKZ.3, `git status`, `--full` | Lead | P2.8 | **AKZ.2**, **AKZ.3** | ☐ | — |
| P2.10 | Einspielzeilen | Lead | P2.9 | **AKZ.8** | — | — |
| P2.11 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | P2.10, **T1** | **AKZ.7** | ☐ | — |
| P2.12 | `install-app.sh`, `check-pwa.sh` | Lead | P2.11 | **AKD.3** | — | — |
| P2.13 | **G6:** EN-Stichprobe; iPhone durch den Nutzer | Lead, Nutzer | P2.12 | **AKD.3**, **AKF.2** | — | — |
| T2 | `pwa-tester` Phasen 0–4 und 9 | `pwa-tester` | P2.13, P2.7 | **AKZ.6** | — | — |

---

## Schritt Z — Doku

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: Schliessungen **nur mit Beleg** aus dem Messprotokoll | `doc-writer` | T2 | S10, S11 | ☐ | — |
| Z.2 | Spec-Endstand | `spec-writer` | Z.1 | jedes Kriterium mit Beleg oder Grund | — | — |

## Abschluss

- [ ] Alle Tasks erledigt (S3 nur bei Gate-Riss; M.3, M.4, S1.9, S1.10 entfallen)
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release, Ergebnis im Messprotokoll
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt oder mit Grund offen
- [ ] Sechs Releases (sieben mit S3), je Flash-Umfang und Reihenfolge benannt, je committet, getaggt,
      gepusht
