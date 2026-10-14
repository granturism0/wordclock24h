# Tasks — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (A15, A56, A70, A79, A80, A81, A82, B8, B48, B55, C6, C6b, C6v, C8, C19, C46, C59, C60, C65, E36, F3)

**Erstellt:** 2026-10-13, Stand `159c9d0`; am selben Tag mit den Entscheidungen des Nutzers nachgeführt.
Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.

**Status: freigegeben** — vom Nutzer am 13.10.2026 (Ent-16), samt Ent-1 bis Ent-14 und Ent-17. **Eine
Entscheidung gegen die Empfehlung:** Ent-11 (C6 einstufig). **Ent-15 (Messbau für A15) aufgehoben
13.10.2026** (L432).

**Nachtrag 13.10.2026 (Ent-18, L439, L440):** Der Schutz der zerstörerischen Aufrufe entfällt aus diesem
Paket und wird in einer eigenen Spec geklärt (C6, C6v, C60, C61, C62). **Ent-11 und Ent-12 sind
aufgehoben.** **Release E1 entfällt**; C46 (ESP-Teil) und C19 (L311) liegen im Baum und gehen mit **E2**
hinaus. **EP** verliert die POST-Umstellung und alles zur Herkunft; es bleibt ein Release mit ESP **und**
PWA — Grund ist jetzt allein C6b (`design.md` §8.4). Neuer Schnitt: S1 → S2 → (S3) → E2 → EP → P2.

**Nachtrag 14.10.2026 (Ent-19, L449) — Schritt E3: freigegeben 14.10.2026.** C65 (Heap unter PWA-Last)
kommt als eigenes ESP-Release **E3 vor T2 und Z** dazu: I-1 bis I-3, M-1, M-3 (`design.md` §18). Ent-20 bis
Ent-27 sind entschieden; **Ent-20 = Variante C** (Release Notes und `wc-list.txt` im LittleFS,
`design.md` §18.5a); **Ent-24 entfällt**, P2 läuft bereits auf der Uhr. Schnitt: S1 → S2 → (S3) → E2 → EP →
T1 → P2 (auf der Uhr) → **E3** → **G7** → T2 (nach G6) → Z.

## Entscheidungen (13.10.2026)

| Ent | Gegenstand | Entschieden | wirkt auf |
|---|---|---|---|
| **Ent-1** | A56 | Ambilight-Power im EEPROM | S2.3 |
| **Ent-2** | A70 | (c), (d), (e), L399 beheben; (a), (b) ohne Fix | S2.11 |
| **Ent-3** | A79 | `setRxBufferSize(1024)` vor `Serial.begin`, `ovr` beim Start belegen | E2.3, E2.14 |
| **Ent-4** | A80 | Icon bei ausgeschaltetem Display verwerfen; Restore-Bedingung nicht vereinfachen | S2.7 |
| **Ent-5** | C8 | Brücke aufweiten, RAM messen (präzisiert durch Ent-10) | E2.2 |
| **Ent-6** | B55 | API wartet das Ergebnis des STM ab; STM-Rückmeldung `RPCR` | S2.1, E2.1, EP.6 |
| **Ent-7** | C19 | nur L311 | E1.4 (im Baum), geht mit E2 (Ent-18) |
| **Ent-8** | F4 | eigenes Folgepaket | — |
| **Ent-9** | A56 Speicherort | zwei Byte hinter dem Layoutende, ohne Layoutversion | S2.3 |
| **Ent-10** | C8 | Puffer 260 Byte, Folgeeinträge im Ring, kein Ring-RAM | E2.2 |
| ~~**Ent-11**~~ | C6 | **aufgehoben 13.10.2026 (L439)**, Ent-18. Stand der Freigabe: **einstufig** (gegen die Empfehlung): nur noch POST; ESP und PWA als **ein** Release EP, direkt nacheinander eingespielt, ESP zuerst; Werkzeuge vorher auf POST; kein Backup-Export dazwischen. **Was bleibt:** EP bleibt ein Release mit ESP und PWA — jetzt wegen C6b (`design.md` §8.4) | ~~EP~~ — EP.1, EP.14 entfallen |
| ~~**Ent-12**~~ | C6 Umfang | **aufgehoben 13.10.2026 (L439)**, Ent-18. Stand der Freigabe: Herkunft über `Sec-Fetch-Site` für alle zerstörenden Wege samt Legacy (`rsteeprom2`, `rststm32`, C60) und `/fs` (C6v); POST für die vier Wartungsendpunkte und `network_secrets`; Legacy bleibt GET; Update-Auslöser, `ir_code_set`, `learn_ir`, `test_display` unverändert; C60 eigener Task, hoch | ~~E1.1, E1.2, EP.1~~ — entfallen |
| **Ent-13** | Gerätenachweise | **alle freigegeben**, auch STM-Reset im Nacht-Aus-Fenster für A70 (e); `caffeinate` bei geplanten Tests (DIR-020) | alle G |
| **Ent-14** | Gate-Riss in S2 | A70 ohne Rückfrage nach S3 mit Flash-Analyse | S2.11, S3 |
| ~~**Ent-15**~~ | A15 Messung | **aufgehoben 13.10.2026** (Entscheidung des Nutzers, L432): A15 bleibt, **ohne** Messbau — seit LTO kein `bl` in der ISR, der Gewinn sind wenige Befehle. Statische Messung (AKI.2) und Regressionslauf (AKI.3) bleiben | M.1–M.4, S1.9, S1.10, S1.13 (entfallen bzw. reines Einspielen) |
| **Ent-16** | Spec | **freigegeben** | alle |
| **Ent-17** | A81, A82 | in S2 aufgenommen, soweit das Budget trägt; sonst Nachrang (A82 nach S3) | S2.5, S2.9 |
| **Ent-18** | Schutz der zerstörerischen Aufrufe | **13.10.2026, Entscheidung des Nutzers (L439, L440):** entfällt aus diesem Paket, eigene Spec für C6, C6v, C60, C61, C62. Ent-11 und Ent-12 aufgehoben. Release E1 entfällt; der Herkunftsteil ist auf `sicherung/e1-herkunft` (`2662368`) gesichert, nicht als Schutz zu verwenden. C46 (ESP) und C19 gehen mit E2. EP ohne POST und ohne Herkunft; AKH.1–AKH.7 entfallen, G3 geht in G4 auf | E1, E2.0, E2.10, EP |
| **Ent-19** | C65 | **14.10.2026, Nutzer (L449):** eigenes ESP-Release **E3 vor T2 und Z**; I-1 bis I-3, M-1, M-3; M-2 und M-4 als Folgebefunde | E3, T2 |
| **Ent-20** | Release Notes und `wc-list.txt` | **14.10.2026, Nutzer: Variante C** — im LittleFS, beim Start geholt, aus der Datei gestreamt, neu geschrieben nur bei geändertem Versionsschlüssel; keine PWA-Änderung. Festlegungen `design.md` §18.5a | E3.4a, E3.6, E3.16 |
| **Ent-21** | Rücksetzen I-1/I-3 | **Lead: B** — nur mit `device_ready?heap_low_reset=1` | E3.1, E3.3, E3.7, E3.16 |
| **Ent-22** | `low=` in der Heap-Zeile | **Lead: ja** | E3.1, E3.11, T2 |
| **Ent-23** | Haltegrenze K30 nach E3 | **Nutzer, nach Empfehlung:** anhalten bei Ruhewert < 5'120 (Heap-Zeile, ≥ 60 s nach der letzten Aktion) **oder** Tiefstwert `low=` < 1'072; dazwischen protokollieren | E3.11, E3.17, T2 |
| ~~**Ent-24**~~ | Reihenfolge E3 / Einspielen P2 | **entfällt:** P2 (PWA 1.4.99) läuft nach Nutzerentscheid bereits auf der Uhr (Tag `release/3.2.34-3.2.35-1.4.99`); P2.12 erledigt, nicht an E3 gebunden | P2.12 |
| **Ent-25** | Sendezeitgrenze | **Lead:** API 1'000 ms, Dateien 2'000 ms, vor `ESPhttpUpdate.update` mindestens 5'000 ms | E3.5 |
| **Ent-26** | Eingriffe in G7 | **Nutzer: beide freigegeben** — (a) Zero-Window-Probe, (b) `weather_get_forecast` während einer Ladung | E3.7, E3.16 |
| **Ent-27** | Wirkung von M-1 belegen | **Lead: einstufig** — kalt gegen warm (AKR.13), alte Messweise gegen L449 (AKR.14) | E3, E3.16 |
| **N-5** | Freigabe der Gerätenachweise | **Nutzerentscheid N-5, 13.10.2026, Gespräch** („alle freigegeben, auch der STM-Reset nachts"); Zeile in `BEFUNDE.md` ergänzt der Lead | E3.16 |

**Vom Lead festgelegt (Vorpaket, gilt weiter):** `CHANGELOG.md` je Release schreibt der Lead. Matrix-Lauf
vor jedem STM-Release, das Flash kostet, nach der Versionsanhebung.

---

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst
bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wo die Dateien disjunkt sind** (R3b,
`design.md` §0.1):

1. **Code des nächsten Releases erst nach dem Release-Build des vorigen:** S2.1 nach S1.12, E2.0/E2.1 nach
   S2.22 (bzw. S3.6), EP.2 nach E2.12, P2.1 nach EP.16, **E3.1 nach P2.11**.
2. **Release-Build erst nach dem Gerätenachweis des vorigen:** S2.22 nach S1.14 (G1), E2.12 nach S2.24
   (G2) bzw. S3.8 (G2b), EP.16 nach E2.14 (G4), P2.11 nach T1. STM-Builds zusätzlich nach grünem
   Matrix-Lauf (S1.11, S2.21, S3.5). **E3.14 nach P2.11;** ist G6 (P2.13) dann noch offen, gilt die
   Begründung aus `design.md` §0.1 (E3 ändert keine PWA-Datei).
3. **Innerhalb eines STM-Releases nacheinander, Flash-Gate nach jedem Task** (AKM.1).
4. **EP ist ein Release mit zwei Komponenten:** Zwischen ESP-OTA und PWA-Upload liegt nichts. Grund seit
   Ent-18 allein C6b — die alte PWA exportierte sonst leere WLAN-Schlüssel (`design.md` §8.4).
5. **E3 vor T2:** T2 erst nach G7 (E3.17) **und** G6 (P2.13). Zwischen E3.15 und E3.16 liegt **kein**
   Einspielen.

**Messwerte sofort (L424):** Jeder messende Task hängt sein Ergebnis **vor dem nächsten Task** an das
Messprotokoll in `BEFUNDE.md` an. Messzeilen schreibt der Lead, Befundnachführungen der `doc-writer` — nie
gleichzeitig (R3).

**Einspielen:** Einspielzeilen je Release fertig (AKZ.8). Jeder Gerätelauf mit `./tools/watch-log.sh`, vor
dem Einspielen gestartet, `--pruefe` am Ende (AKZ.5); auf später geplante Tests mit `caffeinate`
(DIR-020). **Nie am Gerät:** `design.md` §17.3.

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. Prüfstände baut der
Umsetzer im Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab und fährt die Gegenprobe
(DIR-014). Vor jedem Patch Kodierung **und** Zeilenende feststellen (AKZ.9) — `ESP-uclock.ino` gemischt,
`http.cpp` UTF-8; für E3 zusätzlich `httpclient.cpp`, `base.h`, `http.h`.

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

## ~~Schritt E1 — ESP-Release 1: Herkunft für alle zerstörenden Wege, C46, C19~~ — entfallen (Ent-18, 13.10.2026, L439)

Der Release E1 wird nicht gebaut und nicht eingespielt. Der Herkunftsteil (E1.1, E1.2) ist umgesetzt, am
Prüfstand c6h belegt (L437) und **zurückgebaut**; gesichert auf dem Zweig `sicherung/e1-herkunft`
(Commit `2662368`) als Grundlage für die Schutz-Spec, **nicht als Schutz zu verwenden** — er wirkt im
Browser über http nicht (L439). C46 (E1.3) und C19 (E1.4) bleiben im Baum und gehen mit **E2**; ihre
Prüfstände c46 und c19 laufen in S16 (L437).

| # | Task (Stand der Freigabe) | Agent | Stand 13.10.2026 |
|---|---|---|---|
| E1.1 | **Herkunft** nach §8.1 für alle API-Wege und `/fs` (**C6v**); Bestandsliste | `esp-developer` | umgesetzt, **zurückgebaut**, gesichert auf `sicherung/e1-herkunft` |
| E1.2 | **C60 (hoch):** Legacy-Reset-Wege mit Herkunftsprüfung | `esp-developer` | umgesetzt, **zurückgebaut**, gesichert auf `sicherung/e1-herkunft`; C60 bleibt offen (Schutz-Spec) |
| E1.3 | **C46** (ESP) nach §10, Kennung 9 | `esp-developer` | erledigt, **im Baum**, Release mit E2 |
| E1.4 | **C19** (nur L311) an beiden Stellen | `esp-developer` | erledigt, **im Baum**, Release mit E2 |
| E1.5 | Prüfstände c6h, c46, c19 ablegen, Gegenproben gegen 3.2.33 | Lead | erledigt (L437); c46 und c19 bleiben in S16, c6h nur auf dem Sicherungszweig (E2.0) |
| E1.6 | Wache: `- abgewiesen: query`, Gegenprobe | Lead | erledigt, bleibt (gilt für C46) |
| E1.7 | **Review E1** | `code-reviewer` | erledigt; Befunde H1 (L439) und M1 (L440) führten zu Ent-18 |
| E1.8 | ESP-Version anheben | `release-engineer` | **entfallen** — die Nummer 3.2.34 geht an E2 |
| E1.9 | `CHANGELOG.md`, AKZ.3, `--full` | Lead | **entfallen**; C46 und C19 im E2-Eintrag (E2.9) |
| E1.10 | `TESTPLAN-PWA.md`: Herkunft, C46 | `doc-writer` | erledigt; K25 und K26 entfallen, K27 und K28 gehen auf E2 über (E2.10) |
| E1.11 | Einspielzeilen | Lead | **entfallen** |
| E1.12 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | **entfallen** |
| E1.13 | ESP-OTA, Smoketest samt Update-Quelle, `install-app.sh --check` | Lead | **entfallen** |
| E1.14 | **G3** | Lead | **entfallen**; AKQ.2 geht in G4 (E2.14) auf, AKH.4 entfällt |

---

## Schritt E2 — ESP-Release 2: Brücke und Netz, dazu C46 und C19

Nach jedem E2-Task misst der Lead „Globale Variablen" und schreibt sie sofort ins Messprotokoll (AKM.2).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E2.0 | **Baum nach dem Rückbau feststellen** (Ent-18): In `http.cpp` kein Herkunftsteil aus E1 (`grep` auf `http_request_denied_by_origin` und `Sec-Fetch-Site` leer); die Prüfung über `Sec-Fetch-Dest` aus ESP 3.2.4 an den drei Wartungsendpunkten gegenüber dem Tag des Ausgangsstands (ESP 3.2.33) unverändert; C46 und C19 vorhanden; S16 grün mit c46 und c19, c6h nicht mehr in S16. Ergebnis ins Messprotokoll E2 | Lead | **S2.22** bzw. S3.6 | **AKQ.1**, **AKU.1** am Prüfstand | ☐ | — |
| E2.1 | **B55-ESP** nach §6.2 | `esp-developer` | E2.0 | **AKL.2** | ☐ | — |
| E2.2 | **C8** nach §7 | `esp-developer` | E2.1 | **AKK.1** | ☐ | — |
| E2.3 | **A79** nach §12 | `esp-developer` | E2.2 | **AKE.1** | ☐ | — |
| E2.4 | **C59** nach §11 | `esp-developer` | E2.3 | **AKT.1** | ☐ | — |
| E2.5 | Prüfstände l55e, c8, ntp, Gegenproben; statische Prüfung A79. c46 und c19 liegen seit E1.5 in S16 und laufen mit | Lead | E2.4 | Fallzahlen | ☐ | — |
| E2.6 | Wache: `- rpc keine Antwort`, Gegenprobe (`- abgewiesen: query` seit E1.6 vorhanden) | Lead | E2.5 | **AKZ.5** | ☐ | — |
| E2.7 | **Review E2** (Wiedereintritt, Quittungen beim Warten), dazu C46, C19 und der Rückbau des Herkunftsteils (Diff gegen den Ausgangsstand ESP 3.2.33) | `code-reviewer` | E2.6 | Checkliste | — | ☐ |
| E2.8 | ESP-Version anheben | `release-engineer` | E2.7 | S4 | ☐ | — |
| E2.9 | `CHANGELOG.md` (samt C46, C19), AKZ.3, `--full` | Lead | E2.8 | **AKZ.2**, **AKZ.3** | ☐ | — |
| E2.10 | `TESTPLAN-PWA.md`: B55 (ESP), C8; **K25 und K26 als entfallen** vermerken (Ent-18, L439), K27 und K28 dem Release E2 zuordnen; die Verweise darauf nachführen (Instrumente „Reset" und „curl mit Kopfzeile", Einleitung zu 7.1, Gefahrentabelle, Ablauf Schritt 9) | `doc-writer` | E2.7 | S9, S11 | ☐ | — |
| E2.11 | Einspielzeilen | Lead | E2.9 | **AKZ.8** | — | — |
| E2.12 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | E2.11, **S2.24** bzw. S3.8 | **AKZ.7** | ☐ | — |
| E2.13 | ESP-OTA, Smoketest samt Update-Quelle, `install-app.sh --check` | Lead | E2.12 | **AKZ.4** | — | — |
| E2.14 | **G4** mit `caffeinate` (nimmt G3 auf: AKQ.2) | Lead | E2.13 | **AKM.2**, **AKE.2**, **AKK.2**, **AKT.2**, **AKQ.2** | — | — |

---

## Schritt EP — ein Release für ESP und PWA (C6b koppelt beide, `design.md` §8.4)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| EP.1 | ~~**C6, ESP:** POST-Zweig nach §8.2, nur POST für die fünf Endpunkte, GET ⇒ 405, anderer POST ⇒ 404~~ — **entfallen** (Ent-18) | — | — | ~~AKH.5~~ | — | — |
| EP.2 | **C6b, ESP** nach §8.3 (`network_secrets` per GET, ohne Herkunftsprüfung) | `esp-developer` | **E2.12** | **AKS.1**, **AKS.2** | ☐ | — |
| EP.3 | Prüfstand c6b ablegen, Gegenprobe gegen den E2-Stand (~~c6p~~ entfällt) | Lead | EP.2 | **AKS.2** | ☐ | — |
| EP.4 | **C6b, PWA** (Export über `network_secrets`); ~~C6 `apiPost`~~ entfällt | `pwa-developer` | EP.3 | **AKS.3** | ☐ | — |
| EP.5 | **C46, PWA** | `pwa-developer` | EP.4 | **AKQ.3** | ☐ | — |
| EP.6 | **B55, PWA**; Kennungen 9–11 in DE und EN | `pwa-developer` | EP.5 | **AKL.3** | ☐ | — |
| EP.7 | PWA-Prüfstände (Mock); **Werkzeuge für C6b** (`snapshot-device.sh` mit `key_set`, `preview/server.py` mit `network_secrets` und `key_set`), gegen die Vorschau gelaufen. Die POST-Umstellung von `flash-stm.sh`, `smoke-device.sh`, `preview/server.py` entfällt (Ent-18) | Lead | EP.6 | **AKZ.11** | ☐ | — |
| EP.8 | **Review EP** (ESP und PWA zusammen) | `code-reviewer` | EP.7 | Checkliste | — | ☐ |
| EP.9 | `TESTPLAN-PWA.md`: Backup-Export, B55 (~~POST~~ entfällt) | `doc-writer` | EP.8 | S9, S11 | ☐ | — |
| EP.10 | ESP-Version, `APP_VERSION` und `CACHE_NAME` | `release-engineer` | EP.8 | S4 zeigt beide | ☐ | — |
| EP.11 | `CHANGELOG.md`-Eintrag EP (beide Komponenten) | Lead | EP.8 | — | ☐ | — |
| EP.12 | AKZ.3 (ESP **und** PWA), `git status` (R2) | Lead | EP.10, EP.11 | **AKZ.3** | — | — |
| EP.13 | `--full` | Lead | EP.12 | **AKZ.2** | ☐ | — |
| EP.14 | ~~`CLAUDE.md`: Gefahrentabelle und Hinweise auf POST~~ — **entfallen** (Ent-18) | — | — | — | — | — |
| EP.15 | Einspielzeilen: OTA, **unmittelbar** `install-app.sh`; Warnung „kein Backup-Export dazwischen"; Rückweg (§8.4) | Lead | EP.13 | **AKZ.8** | — | — |
| EP.16 | Build, ZIP, Rollout, **ein** Commit + Tag + Push | Lead | EP.15, **E2.14** | **AKZ.7** | ☐ | — |
| EP.17 | **Einspielen:** ESP-OTA, sofort `install-app.sh`; danach Smoketest samt Update-Quelle, `--check`, `check-pwa.sh` | Lead | EP.16 | **AKG.5** | — | — |
| EP.18 | **G5** | Lead | EP.17 | **AKL.4**, **AKS.4** (~~AKH.7~~ entfällt) | — | — |
| T1 | `pwa-tester` Phasen 0–4 und 9 | `pwa-tester` | EP.18, EP.9 | **AKZ.6** | — | — |

**Stand 14.10.2026 (L449):** T1 hat an der Haltegrenze K30 gestoppt; lesender Teil 51 von 56 bestanden.
Phase 3 (113 von 114), E1–E8 und **AKL.4** sind nicht gefahren und gehen an **T2** (Schritt T2 unten).

---

## Schritt P2 — PWA-Release: B48, B8

P2.3/P2.4 dürfen im Worktree (R6) vorgezogen werden; in den Hauptbaum erst nach EP.16.

**Stand 14.10.2026:** P2.1 bis P2.12 erledigt (Commit `8445a10`, Tag `release/3.2.34-3.2.35-1.4.99`, PWA
1.4.99 nach Nutzerentscheid auf der Uhr; Ent-24 entfällt). P2.13 (G6) steht unabhängig von E3 aus und muss
vor T2 abgeschlossen sein.

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
| P2.12 | `install-app.sh`, `check-pwa.sh` — **erledigt 14.10.2026** (Nutzerentscheid; nicht an E3 gebunden) | Lead | P2.11 | **AKD.3** | — | — |
| P2.13 | **G6:** EN-Stichprobe; iPhone durch den Nutzer — liegt G6 nach dem OTA von E3, dabei auf `fs write short` und `fail=` achten (Sendezeitgrenze, `design.md` §18.6) | Lead, Nutzer | P2.12 | **AKD.3**, **AKF.2** | — | — |

---

## Schritt E3 — ESP-Release: C65 Heap unter PWA-Last (Nachtrag 14.10.2026, freigegeben 14.10.2026)

`design.md` §18; Akzeptanzkriterien AKR in `requirements.md`. **Ein** `esp-developer` schreibt E3.1 bis
E3.5 nacheinander (in `http.cpp`, `httpclient.cpp`, dem gemeinsamen Kopf und `ESP-uclock.ino`). **Nach jedem
dieser Tasks** baut der Lead den ESP-Teil, misst „Globale Variablen" und Programmgrösse und schreibt beides
sofort ins Messprotokoll E3 (AKR.8, L424), bevor der nächste Task beginnt; über 700 Byte Zuwachs insgesamt
⇒ anhalten und vorlegen.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E3.0 | **Grundlagen (lesend):** Zeile „Messprotokoll E3" in `BEFUNDE.md` anlegen mit der Fundstelle **Nutzerentscheid N-5, 13.10.2026, Gespräch**. Von der Uhr: Länge der **gefilterten** Listen (`stm32_files`, `table_files`), freier Platz und Gesamtgrösse des LittleFS (`/api/fs_info`), Grösse von `app-app.js.gz` (`/api/fs_list`), bekannte Hardware (`stm32_default`). Vom Update-Server: Grösse von `releasenote.html`, `wc-list.txt`, `wc-list-tables.txt`. Daraus: Puffergrösse der Tabellenliste, Höchstlänge der `wc-list`-Datei, Platzschwelle (§18.5a), **Sollwert Δ`httpclient_connects` für einen kalten Lauf** (erwartet 6). „Globale Variablen" und Programmgrösse des Stands `8445a10` (ESP-Teilbuild) | Lead | **P2.11** | Werte im Messprotokoll E3 | — | — |
| E3.1 | **I-1** nach §18.2: Tiefstwerte, vier Messpunkte, `httpclient_connects`, Felder in `device_ready`, Rücksetzen nur mit `heap_low_reset=1` (Ent-21 B), `low=` in der Heap-Zeile (Ent-22; `ESP-uclock.ino` gemischte Zeilenenden, byte-genau). Prüfpunkt Kosten `getMaxFreeBlockSize()` am Core. **Prüfstand c65a** (I-1-Teil) im Scratchpad | `esp-developer` | E3.0 | **AKR.1** | ☐ | — |
| E3.2 | **I-2** nach §18.3: Felder `blk`/`snd`/`ms` in beiden Zeilen, `fs write short` in Zähler und Ring, `fs_write_short` in `device_ready`. **Prüfstand c65b** mit Attrappe nach der Checkliste | `esp-developer` | E3.1 | **AKR.2** | ☐ | — |
| E3.3 | **I-3** nach §18.4: `req_max_ms`, `req_max_path` ohne Query, jeder Rückkehrpfad. c65a ergänzt | `esp-developer` | E3.2 | **AKR.3** | ☐ | — |
| E3.4 | **M-1, Tabellenliste und Antwortform** nach §18.5 (1) und (3): RAM-Zwischenspeicher für `table_files`, `update_table_files` über `update_server_open()`, Kopfkommentar `http.cpp:12596-12620` nachführen. Im Bericht die Tabelle „Wirkung je Leser der PWA" (Vorlage §18.5). **Prüfstand c65m** samt Schlüsselvergleich | `esp-developer` | E3.3 | **AKR.4**, **AKR.6** | ☐ | — |
| E3.4a | **M-1, Release Notes und `wc-list.txt` im LittleFS** nach §18.5a (Ent-20 C): Dateien `/upd-releasenote.txt`, `/upd-wc-list.txt` mit Kopfzeile, Auftrag `update_cache_poll()` aus `loop()` (Zustandsmaschine, ein Abruf je Durchlauf, Bedingungen: WLAN, Host und Pfad gesetzt, 60 s nach Start, 5 s nach der letzten Anfrage, Sperrfenster, 10 min nach Fehlschlag), Temporärdatei und Umbenennen, Platzschwelle, Ausliefern aus der Datei mit gemeinsamer Maskierung, Zeilen `- update cache …`. Prüfpunkt: Bestandsliste aller festen Dateinamen, keine Kollision mit `upd-`. **Prüfstand c65n** | `esp-developer` | E3.4 | **AKR.5**, **AKR.6** | ☐ | — |
| E3.5 | **M-3** nach §18.6 (Ent-25): `setTimeout (1000)` nach `accept()`, `2000` in der Dateiauslieferung, `5000` vor beiden `ESPhttpUpdate.update`; Bestandsliste der Stream-Lesestellen; Kommentare mit 5'000 ms nachführen. **Prüfstand c65t** | `esp-developer` | E3.4a | **AKR.7** | ☐ | — |
| E3.6 | Prüfstände c65a, c65b, c65m, c65n, c65t unter `tools/checks/auszug/esp/c65/` ablegen und in S16 aufnehmen; Gegenproben gegen 3.2.35 und Sabotagen fahren; Fallzahlen nachzählen (L235) | Lead | E3.5 | **AKR.1–AKR.7** am Prüfstand, Fallzahlen | ☐ | — |
| E3.7 | **Werkzeug:** Wache `- fs write short` (Alarm), `- update liste zu gross` und `- update cache … fehlgeschlagen` (Hinweis), je Gegenprobe; `smoke-device.sh` verlangt die neuen `device_ready`-Felder und setzt nichts zurück, Gegenprobe über `SMOKE_DR_OVERRIDE`; Zero-Window-Probe (Ent-26 a, §18.8), einmal gegen einen lokalen, normal lesenden Server fehlgeschlagen (Exit ≠ 0) | Lead | E3.6 | **AKR.9**, **AKR.10**, **AKZ.5** | ☐ | — |
| E3.8 | **Review E3:** alle vier Punkte der Checkliste; Attrappen-Semantik in c65b (`connected()` nach FIN 0); jeder Rückkehrpfad in I-3; keine Query in Logs oder `req_max_path`; Rückwärtsverträglichkeit (AKR.6 und Tabelle aus E3.4); LittleFS: Temporärdatei, Längenprüfung, Schreibhäufigkeit, nie der Server des Ursprungsprojekts, Platzschwelle; OTA-Zeitgrenze; Kodierung und Zeilenenden (AKZ.9) | `code-reviewer` | E3.7 | Checkliste | — | ☐ |
| E3.9 | ESP-Version anheben (erwartet 3.2.36); `APP_VERSION` und `CACHE_NAME` **nicht** | `release-engineer` | E3.8 | S4 | ☐ | — |
| E3.10 | `CHANGELOG.md`-Eintrag E3; AKZ.3 gegen das Tag von P2 (nur ESP-Dateien); `--full`; alles sofort ins Messprotokoll | Lead | E3.9 | **AKZ.2**, **AKZ.3** | ☐ | — |
| E3.11 | `TESTPLAN-PWA.md`: **K30** nach Ent-23 (Ruhewert und `low=`, beide Grenzen, Messweise); Logzeilen-Tabelle (`http write lost` und `fs write short` mit `blk`/`snd`/`ms` und der Entscheidungsregel aus §18.3, `- update liste zu gross`, `- update cache …`, `low=`); neue `device_ready`-Felder und `heap_low_reset`; `write_lost_failed` zählt ab E3 auch abgebrochene Assets; T2-Umfang (AKL.4, Setter-Phasen aus L449, AKR.21 nach dem Reset aus V3) | `doc-writer` | E3.8 | S9, S11 | ☐ | — |
| E3.12 | `BEFUNDE.md`: „Träger: Paket 2026-10-13 (Release E3)" an C65, C49, C54; die Folgebefunde aus §18.10 als eigene Zeilen (M-2, M-4, **Teildatei in `download_file_to_local()`**, Brücke während langer Antworten). **Nicht gleichzeitig mit einer Messzeile des Leads** (R3) | `doc-writer` | E3.10 | S10, S11 | ☐ | — |
| E3.13 | Einspielzeilen: **nur** ESP-OTA; Hinweis, dass in den ersten rund 60 s die Release Notes leer sind (Startabruf), und dass G7 erst nach AKR.20 beginnt | Lead | E3.10 | **AKZ.8** | — | — |
| E3.14 | Build, ZIP, Rollout, Commit + Tag + Push (Tag nach DIR-011 mit den Versionen dieses Commits, erwartet `release/3.2.34-3.2.36-1.4.99`) | Lead | E3.11, E3.12, E3.13 | **AKZ.7** | ☐ | — |
| E3.15 | Wache starten; ESP-OTA; Smoketest samt Update-Quelle; `install-app.sh --check` (ohne Abweichung); `check-pwa.sh`; jedes Ergebnis sofort ins Messprotokoll | Lead | E3.14 | **AKR.11**, **AKZ.4** | — | — |
| E3.16 | **G7** nach `design.md` §18.8: Startabruf (G7.0b), Ruhe, drei Paare kalt/warm, alte Messweise, Zero-Window-Probe, Vorhersageabruf während einer Ladung, Ruhe nach Last; jeder Lauf sofort ins Messprotokoll; `caffeinate`, falls geplant | Lead | E3.15 | **AKR.12–AKR.20** | — | — |
| E3.17 | **Weiter oder anhalten:** G7 gegen K30 (Ent-23) auswerten. Ruhewert < 5'120 (AKR.12 ii) oder `low=` < 1'072 ⇒ **kein** T2, dem Nutzer vorlegen. Ist nur AKR.13 nicht erfüllt, geht es weiter; C65 bleibt dann offen, mit Befund | Lead | E3.16 | **AKR.12**, Ent-23 | — | — |

---

## Schritt T2 — `pwa-tester` nach E3 und P2

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T2 | `pwa-tester` Phasen 0–4 und 9; **dazu AKL.4** und die Setter-Phasen, die T1 an der Haltegrenze ausgelassen hat (L449: Phase 3 Rest, E1–E8). Haltegrenze K30 nach Ent-23, gemessen am Ruhewert der Heap-Zeile und an `low=`; I-2-Zeilen im Mitschnitt nach der Entscheidungsregel lesen; nach dem ESP-Neustart aus V3 die Zeilen `- update cache …` prüfen (AKR.21) | `pwa-tester` | P2.13, P2.7, E3.11, E3.17 | **AKZ.6**, **AKL.4**, **AKR.21** | — | — |

---

## Schritt Z — Doku

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: Schliessungen **nur mit Beleg** aus dem Messprotokoll — für C65, C49, C54 aus dem Messprotokoll E3 und aus T2 | `doc-writer` | T2 | S10, S11 | ☐ | — |
| Z.2 | Spec-Endstand | `spec-writer` | Z.1 | jedes Kriterium mit Beleg oder Grund | — | — |

## Abschluss

- [ ] Alle Tasks erledigt (S3 nur bei Gate-Riss; M.3, M.4, S1.9, S1.10 entfallen; E1.8, E1.9,
      E1.11–E1.14, EP.1 und EP.14 entfallen mit Ent-18)
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release, Ergebnis im Messprotokoll
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt oder mit Grund offen
- [ ] Sechs Releases (S1, S2, E2, EP, P2, E3; sieben mit S3), je Flash-Umfang und Reihenfolge benannt, je
      committet, getaggt, gepusht
