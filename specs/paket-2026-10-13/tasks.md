# Tasks — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (A15, A56, A70, A79, A80, B8, B48, B55, C6, C6b, C6v, C8, C19, C46, C59, E36, F3)

**Erstellt:** 2026-10-13, Stand `159c9d0`. **Status: Entwurf — zur Freigabe.** Offene Fragen N-1 bis N-7
(`requirements.md`). Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.

## Entscheidungen

| Kennung | Gegenstand | Stand | wirkt auf |
|---|---|---|---|
| A56 | Ambilight-Power im EEPROM | **entschieden** 13.10.2026; Speicherort offen (N-1) | S2.3 |
| A70 | (c), (d), (e), L399 beheben; (a), (b) ohne Fix | **entschieden** 13.10.2026 | S2.7 |
| A79 | `setRxBufferSize(1024)` vor `Serial.begin` | **entschieden** 13.10.2026 | E2.3 |
| A80 | Icon bei ausgeschaltetem Display verwerfen | **entschieden** 13.10.2026 | S2.5 |
| C8 | Kommandopuffer aufweiten, RAM messen | **entschieden** 13.10.2026; Ring offen (N-2) | E2.2 |
| B55 | API wartet das Ergebnis ab, meldet ok/abgewiesen | **entschieden** 13.10.2026; braucht STM-Teil (`design.md` §6) | S2.1, E2.1, P1.3 |
| C19 | nur L311 | **entschieden** 13.10.2026 | E1.4 |
| F4 | eigenes Folgepaket | **entschieden** 13.10.2026 | — |
| Freigaben | Updates einspielen, STM-Reset | **erteilt** | alle Einspiel- und Reset-Schritte |
| N-1 | A56 Speicherort | **offen** — Empfehlung (a) ohne Layoutversion | S2.3 |
| N-2 | C8 Ring | **offen** — Empfehlung (a) Folgeeinträge | E2.2 |
| N-3 | C6 zwei Stufen | **offen** — Empfehlung (a) | E1.1, E3.1 |
| N-4 | C6 Umfang | **offen** — Empfehlung: vier Wartungsendpunkte, `network_secrets`, drei Legacy-Aktionen | E1.1 |
| N-5 | schreibende Gerätenachweise | **offen** — Liste `design.md` §17.2 | 0.6, S1.12, S2.20, E2.12, P1.11, T1, T2 |
| N-6 | Flash-Gate reisst in S2 | **offen** — Empfehlung: A70 auslagern | S2.7 |
| N-7 | A15 Messung | **offen** — Empfehlung: statisch, beide Plattformen | 0.6, S1.3 |

---

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst
bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wo die Dateien disjunkt sind** (R3b). Drei Ketten
tragen das Paket (`design.md` §0.1):

1. **Code des nächsten Releases erst nach dem Release-Build des vorigen:** S2.1 nach S1.10, E1.1 nach
   S2.18, E2.1 nach E1.13, P1.1 nach E2.10, E3.1 nach P1.9, P2.1 nach E3.5.
2. **Release-Build erst nach dem Gerätenachweis des vorigen:** S2.18 nach S1.12 (G1), E1.13 nach S2.20
   (G2), E2.10 nach E1.15 (G3), P1.9 nach E2.12 (G4), E3.5 nach T1, P2.10 nach E3.7 (G6). STM-Builds
   zusätzlich nach grünem Matrix-Lauf auf dem angehobenen Stand (S1.9, S2.17).
3. **Innerhalb eines STM-Releases nacheinander, Flash-Gate nach jedem Task** (AKM.1). Unter 1'024 Byte:
   anhalten und vorlegen (bzw. N-6).

**Messwerte sofort (L424):** Jeder Task, der misst (Gate, Matrix, RAM, `--full`, Smoketest,
Gerätewert), hängt sein Ergebnis **vor dem nächsten Task** an das Messprotokoll des Releases in
`BEFUNDE.md` an (`design.md` §1.4). Die Messzeilen schreibt der Lead; Befundnachführungen der
`doc-writer` — nie gleichzeitig (R3).

**Einspielen:** Die Einspielzeilen stehen je Release fertig da (AKZ.8). Jeder Gerätelauf mit
`./tools/watch-log.sh`, **vor** dem Einspielen gestartet, und `--pruefe` am Ende (AKZ.5). Schreibend nur im
Rahmen der Freigaben (Updates, STM-Reset, N-5) und mit Rückstellung. **Nie am Gerät:** `design.md` §17.3.

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. Prüfstände baut der
Umsetzer im Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab und fährt die Gegenprobe gegen
den Vorgängerstand (DIR-014). Vor jedem Patch: Kodierung **und** Zeilenende feststellen (AKZ.9) —
`ESP-uclock.ino` hat gemischte Zeilenenden, `http.cpp` ist UTF-8.

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — ohne Firmware

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **E36:** Ursache untersuchen (`pmset -g log`, 12.10.2026 00:00–01:40), dann Wache umbauen nach `design.md` §16: Herzschlagdatei, `--pruefe`, Selbstmeldung bei Aussetzer, Frist für `log.sh`, `caffeinate -i`; Stop-Hook ruft `--pruefe` bei gesetzter Gerätemarke | Lead | Freigabe | **AKW.1–AKW.5**; je Gegenprobe Meldung in Konsole **und** Mitschrift, Exit-Code genannt | ☐ | — |
| 0.2 | **Review 0.1** | `code-reviewer` | 0.1 | Weg jeder Meldung bis zum Empfänger (DIR-014); kein `\| tail`, keine Pufferung (L181, L194) | — | ☐ |
| 0.3 | **F3:** Guardrail-Stufe Backup-Umfang mit Ausnahmeliste nach `design.md` §15; einmalige dynamische Gegenprobe in der Vorschau | Lead | Freigabe | **AKB.1–AKB.3**; Zahl der betrachteten Endpunkte gegen `grep -c` nachgezählt | ☐ | — |
| 0.4 | **Review 0.3** | `code-reviewer` | 0.3 | Sieht die Stufe ihren Gegenstand vollständig (L235)? | — | ☐ |
| 0.5 | **B8:** Zählwerkzeug, Kandidatenliste, einmalige Zuordnung (deutscher UI-Text oder Ausnahme mit Grund); Zahl sofort bei B8 in `BEFUNDE.md` | Lead | Freigabe | **AKD.1** | ☐ | — |
| 0.6 | **G0 — Grundlinien** nach AKG.0: Disassembly 3.2.32 F103 und F411 (Worktree, R6); Setter-Burst 14 × Wetterort mit Rückstellung (N-5); 30 Heap-Zeilen; längste Zeile in `/api/stm32_log`. Je Wert sofort ins Messprotokoll | Lead | **0.1**, N-5, N-7 | **AKG.0** | — | — |
| 0.7 | **Befunde Teil 1:** neue Befunde aus `requirements.md` (Marker-Weissfarbe, `dcf77_enabled`, Legacy `rsteeprom2`, zweite C19-Stelle, 120er-Ring, Rechnung Netzwerkimport); „Träger: Paket 2026-10-13" an allen Einträgen | `doc-writer` | Freigabe | **AKG.1**; S10, S11 grün | ☐ | — |

---

## Schritt S1 — STM-Release 1 (isoliert): A15

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S1.1 | **A15** nach `design.md` §2.1: `SR` und `CR1` einmal lesen, kein SPL-Aufruf im Normalpfad, TXE nur mit TXEIE, ORE vor `DR`; Kommentar `:777-787` nachführen; Prüfpunkt `uart_putc()`/`CR1` im Bericht. **Prüfstand a15** im Scratchpad | `stm-developer` | Freigabe | **AKI.1** am Prüfstand, Fallzahl, Sabotagen FEHL | ☐ | — |
| S1.2 | **Flash-Gate** F103 und F411, sofort ins Messprotokoll | Lead | S1.1 | **AKM.1** | — | — |
| S1.3 | Prüfstand a15 ablegen, Gegenprobe; **Disassembly nachher** F103 und F411, sofort ins Messprotokoll | Lead | S1.2, **0.6** | **AKI.1**, **AKI.2** | ☐ | — |
| S1.4 | **Review S1** | `firmware-analyst` | S1.3 | Gleichwertigkeit, beide Plattformen, Checkliste | — | ☐ |
| S1.5 | STM-Version anheben | `release-engineer` | S1.4 | S4 zeigt den neuen Stand | ☐ | — |
| S1.6 | `CHANGELOG.md`-Eintrag S1 | Lead | S1.4 | — | ☐ | — |
| S1.7 | **AKZ.3** Release-Inhalt, Ausgabe ins Messprotokoll | Lead | S1.5, S1.6 | **AKZ.3** | — | — |
| S1.8 | `guardrails.sh --full`, sofort ins Messprotokoll | Lead | S1.7 | **AKZ.2** | ☐ | — |
| S1.9 | **Matrix-Lauf** nach der Versionsanhebung, sofort ins Messprotokoll | Lead | S1.8 | **AKM.3**; darunter kein Build | — | — |
| S1.10 | Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S1.9 | **AKZ.7** | ☐ | — |
| S1.11 | **Einspielen:** Wache starten, `flash-stm.sh --check`, `flash-stm.sh`, Smoketest sofort ins Messprotokoll | Lead | S1.10 | **AKZ.4**, **AKZ.8** | — | — |
| S1.12 | **G1:** Setter-Burst wie G0 (N-5), 60 min lesend, `--pruefe` | Lead | S1.11 | **AKI.3** | — | — |

---

## Schritt S2 — STM-Release 2: B55-STM, A56, A80, A70

**Reihenfolge nach N-6:** B55-STM, A56, A80, A70. Reisst das Gate: A70 auslagern (N-6) oder anhalten.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S2.1 | **B55-STM** nach `design.md` §6.1: Ergebnis aus `ldr_set_*_value()`, Zeile `RPCR xx r` nach der Wertmeldung; weitere Aufrufer per `grep`. **Prüfstand l55** | `stm-developer` | **S1.10** | **AKL.1** | ☐ | — |
| S2.2 | Flash-Gate | Lead | S2.1 | **AKM.1** | — | — |
| S2.3 | **A56** nach `design.md` §3 (N-1): Platz, Speichern in `display_set_ambilight_power()` und `display_write_config_to_eep()`, Laden ohne LED-Ausgabe; Prüfpunkt `BLACK_BOARD`. **Prüfstand a56** | `stm-developer` | S2.2, N-1 | **AKA.1**, **AKA.2** | ☐ | — |
| S2.4 | Flash-Gate | Lead | S2.3 | **AKM.1** | — | — |
| S2.5 | **A80** nach `design.md` §5: Verwerfen beim Empfang, `display_icon_cancel()` beim Ausschalten. **Restore-Bedingung nicht anfassen.** **Prüfstand a80** | `stm-developer` | S2.4 | **AKC.1** | ☐ | — |
| S2.6 | Flash-Gate | Lead | S2.5 | **AKM.1** | — | — |
| S2.7 | **A70** nach `design.md` §4: Merker trennen, dynamische zweite Frist (c), Vormerken beim ersten `TIME` (d), `start_ticker()` (e), DCF77 (L399). Kommentar A78 **nicht** anfassen. **Prüfstand a70** | `stm-developer` | S2.6, N-6 | **AKN.1–AKN.5** | ☐ | — |
| S2.8 | Flash-Gate | Lead | S2.7 | **AKM.1** | — | — |
| S2.9 | Prüfstände l55, a56, a80, a70 ablegen, Gegenproben gegen 3.2.32; a69 weiter OK | Lead | S2.8 | Fallzahlen, je FEHL gegen den Vorgänger | ☐ | — |
| S2.10 | Wache: `icon: verworfen` (Hinweis), `icon_freeze: ENTER` (Alarm) — je Gegenprobe | Lead | S2.9 | **AKZ.5** | ☐ | — |
| S2.11 | **Review S2**, ausdrücklich Restore-Invariante (`git diff` der Bedingung) | `firmware-analyst` | S2.10 | **AKN.5**, Checkliste | — | ☐ |
| S2.12 | STM-Version anheben | `release-engineer` | S2.11 | S4 | ☐ | — |
| S2.13 | `CHANGELOG.md`-Eintrag S2 | Lead | S2.11 | — | ☐ | — |
| S2.14 | `TESTPLAN-PWA.md`: Fälle für A56, A80 | `doc-writer` | S2.11 | S9, S11 grün | ☐ | — |
| S2.15 | AKZ.3, ins Messprotokoll | Lead | S2.12, S2.13 | **AKZ.3** | — | — |
| S2.16 | `--full`, ins Messprotokoll | Lead | S2.15 | **AKZ.2** | ☐ | — |
| S2.17 | Matrix-Lauf, ins Messprotokoll | Lead | S2.16 | **AKM.3** | — | — |
| S2.18 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | S2.17, **S1.12** | **AKZ.7** | ☐ | — |
| S2.19 | Einspielen, Smoketest | Lead | S2.18 | **AKZ.4** | — | — |
| S2.20 | **G2:** Ambilight-Probe mit STM-Reset, Icon-Probe, Nacht lesend (N-5) | Lead | S2.19 | **AKA.3**, **AKC.2**, **AKN.6** | — | — |

---

## Schritt E1 — ESP-Release 1: HTTP-Schicht

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E1.1 | **C6 Stufe 1, C6v, Legacy-Herkunft** nach `design.md` §8.1, §8.2, §8.4 (N-3, N-4) | `esp-developer` | **S2.18**, N-3, N-4 | **AKH.1–AKH.3** | ☐ | — |
| E1.2 | **C6b** nach §8.3 | `esp-developer` | E1.1 | **AKS.1**, **AKS.2** | ☐ | — |
| E1.3 | **C46** nach §10.1, Kennung 9 | `esp-developer` | E1.2 | **AKQ.1** | ☐ | — |
| E1.4 | **C19** (nur L311) an beiden Stellen, §9 | `esp-developer` | E1.3 | **AKU.1** | ☐ | — |
| E1.5 | Prüfstände c6, c6b, c46, c19 ablegen, Gegenproben gegen 3.2.33 | Lead | E1.4 | **AKH.4**, **AKS.3** | ☐ | — |
| E1.6 | Wache: `- veraltet: GET`, `- abgewiesen: query` — je Gegenprobe | Lead | E1.5 | **AKZ.5** | ☐ | — |
| E1.7 | **Review E1** (Sicherheit, Leseschleife POST-Kopfzeilen nach C53) | `code-reviewer` | E1.6 | Checkliste | — | ☐ |
| E1.8 | ESP-Version anheben | `release-engineer` | E1.7 | S4 | ☐ | — |
| E1.9 | `CHANGELOG.md`-Eintrag E1, AKZ.3, `--full` — je ins Messprotokoll | Lead | E1.8 | **AKZ.2**, **AKZ.3** | ☐ | — |
| E1.10 | `TESTPLAN-PWA.md`: C6, C6b, C46 | `doc-writer` | E1.7 | S9, S11 | ☐ | — |
| E1.11 | Einspielzeile mit Hinweis „bis P1 kein Backup-Export" | Lead | E1.9 | **AKZ.8** | — | — |
| E1.12 | (frei) | — | — | — | — | — |
| E1.13 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | E1.11, **S2.20** | **AKZ.7** | ☐ | — |
| E1.14 | ESP-OTA, Smoketest samt Update-Quelle, `install-app.sh --check` | Lead | E1.13 | **AKZ.4** | — | — |
| E1.15 | **G3** | Lead | E1.14 | **AKH.5**, **AKS.4**, **AKQ.2** | — | — |
| E1.16 | Werkzeuge auf POST: `flash-stm.sh`, `smoke-device.sh`, `snapshot-device.sh`, `preview/server.py`, Probe in `checks/hooks.sh` | Lead | E1.15 | **AKZ.11** | ☐ | — |

---

## Schritt E2 — ESP-Release 2: Brücke und Netz

Nach **jedem** E2-Task misst der Lead „Globale Variablen" aus dem Build und schreibt sie sofort ins
Messprotokoll (AKM.2).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E2.1 | **B55-ESP** nach `design.md` §6.2: `stm_bridge_poll()` mit Sperre, `RPCR`, Warten bis 2'000 ms, Kennungen 10, 11 | `esp-developer` | **E1.13** | **AKL.2** | ☐ | — |
| E2.2 | **C8** nach §7 (N-2) | `esp-developer` | E2.1, N-2 | **AKK.1** | ☐ | — |
| E2.3 | **A79** nach §12 | `esp-developer` | E2.2 | **AKE.1** | ☐ | — |
| E2.4 | **C59** nach §11 | `esp-developer` | E2.3 | **AKT.1** | ☐ | — |
| E2.5 | Prüfstände l55e, c8, ntp ablegen, Gegenproben; statische Prüfung A79 | Lead | E2.4 | Fallzahlen, FEHL gegen 3.2.33 | ☐ | — |
| E2.6 | Wache: `- rpc keine Antwort` — Gegenprobe | Lead | E2.5 | **AKZ.5** | ☐ | — |
| E2.7 | **Review E2** (Wiedereintritt, Quittungen beim Warten) | `code-reviewer` | E2.6 | Checkliste | — | ☐ |
| E2.8 | ESP-Version anheben | `release-engineer` | E2.7 | S4 | ☐ | — |
| E2.9 | `CHANGELOG.md`, AKZ.3, `--full` — ins Messprotokoll | Lead | E2.8 | **AKZ.2**, **AKZ.3** | ☐ | — |
| E2.10 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | E2.9, **E1.15** | **AKZ.7** | ☐ | — |
| E2.11 | ESP-OTA, Smoketest, `install-app.sh --check` | Lead | E2.10 | **AKZ.4** | — | — |
| E2.12 | **G4:** RAM 30 min, drei ESP-Starts, Vorhersage-Abruf (N-5), zwei Stunden NTP | Lead | E2.11 | **AKM.2**, **AKE.2**, **AKK.2**, **AKT.2** | — | — |

---

## Schritt P1 — PWA-Release 1

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P1.1 | **C6** (`apiPost`) und **C6b** (Export über `network_secrets`) nach §8.3, §8.5 | `pwa-developer` | **E2.10** | **AKH.6**, **AKS.5** | ☐ | — |
| P1.2 | **C46** nach §10.2 | `pwa-developer` | P1.1 | **AKQ.3** | ☐ | — |
| P1.3 | **B55** nach §6.3; Kennungen 9–11 in DE und EN | `pwa-developer` | P1.2 | **AKL.3** | ☐ | — |
| P1.4 | Prüfstände (Mock) für P1.1–P1.3 ablegen, Gegenproben | Lead | P1.3 | Fallzahlen | ☐ | — |
| P1.5 | **Review P1** | `code-reviewer` | P1.4 | Checkliste | — | ☐ |
| P1.6 | `TESTPLAN-PWA.md`: B55, Backup-Export | `doc-writer` | P1.5 | S9, S11 | ☐ | — |
| P1.7 | `APP_VERSION` und `CACHE_NAME` | `release-engineer` | P1.5 | S4 | ☐ | — |
| P1.8 | `CHANGELOG.md`, AKZ.3, `git status` (R2), `--full` — ins Messprotokoll | Lead | P1.7 | **AKZ.2**, **AKZ.3** | ☐ | — |
| P1.9 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | P1.8, **E2.12** | **AKZ.7** | ☐ | — |
| P1.10 | `install-app.sh`, `--check`, `check-pwa.sh` | Lead | P1.9 | **AKG.5** | — | — |
| P1.11 | **G5:** LDR-Proben (N-5), Backup-Export | Lead | P1.10 | **AKL.4**, **AKS.6** | — | — |
| T1 | `pwa-tester` Phasen 0–4 und 9 | `pwa-tester` | P1.11, P1.6 | **AKZ.6** | — | — |

---

## Schritt E3 — ESP-Release 3: C6 Stufe 2

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E3.1 | GET ⇒ 405 nach §8.4 | `esp-developer` | **P1.9**, **E1.16** | **AKH.7** | ☐ | — |
| E3.2 | Prüfstand c6 umstellen, Gegenprobe | Lead | E3.1 | **AKH.7** | ☐ | — |
| E3.3 | **Review E3** | `code-reviewer` | E3.2 | Checkliste | — | ☐ |
| E3.4 | ESP-Version anheben | `release-engineer` | E3.3 | S4 | ☐ | — |
| E3.5 | `CHANGELOG.md`, AKZ.3, `--full`, Build, ZIP, Rollout, Commit + Tag + Push | Lead | E3.4, **T1** | **AKZ.2**, **AKZ.3**, **AKZ.7** | ☐ | — |
| E3.6 | ESP-OTA, Smoketest, `install-app.sh --check` | Lead | E3.5 | **AKZ.4** | — | — |
| E3.7 | **G6:** GET `maintenance_reset_stm32` ⇒ 405, kein Neustart | Lead | E3.6 | **AKH.7** | — | — |

---

## Schritt P2 — PWA-Release 2: B48, B8

P2.3/P2.4 dürfen nach `design.md` §0.1 in einem Worktree (R6) vorgezogen werden; eingespielt erst nach E3.5.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P2.1 | Klasse `.field-error` (B48) | `ui-developer` | **E3.5** | Kontrast gemessen | ☐ | — |
| P2.2 | **B48** nach §13 | `pwa-developer` | P2.1 | **AKF.1** | ☐ | — |
| P2.3 | **B8** in `app.js` und `en.json` nach §14 | `pwa-developer` | P2.2, **0.5** | Werkzeug: 0 offene Texte in `app.js` | ☐ | — |
| P2.4 | **B8** in `index.html` (`data-i18n*`) | `ui-developer` | P2.3 | Werkzeug: 0 offene Texte in `index.html` | ☐ | — |
| P2.5 | B8-Werkzeug als Guardrail-Stufe; Prüfstand B48 in der Vorschau; Gegenproben | Lead | P2.4 | **AKD.2**, **AKF.1** | ☐ | — |
| P2.6 | **Review P2** | `code-reviewer` | P2.5 | Checkliste | — | ☐ |
| P2.7 | `TESTPLAN-PWA.md`: B48 (iPhone), EN-Stichprobe | `doc-writer` | P2.6 | S9, S11 | ☐ | — |
| P2.8 | `APP_VERSION` und `CACHE_NAME` | `release-engineer` | P2.6 | S4 | ☐ | — |
| P2.9 | `CHANGELOG.md`, AKZ.3, `git status`, `--full` | Lead | P2.8 | **AKZ.2**, **AKZ.3** | ☐ | — |
| P2.10 | Build, ZIP, Rollout, Commit + Tag + Push | Lead | P2.9, **E3.7** | **AKZ.7** | ☐ | — |
| P2.11 | `install-app.sh`, `check-pwa.sh` | Lead | P2.10 | **AKD.3** | — | — |
| P2.12 | **G7:** EN-Stichprobe; iPhone durch den Nutzer | Lead, Nutzer | P2.11 | **AKD.3**, **AKF.2** | — | — |
| T2 | `pwa-tester` Phasen 0–4 und 9 | `pwa-tester` | P2.12, P2.7 | **AKZ.6** | — | — |

---

## Schritt Z — Doku

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: Schliessungen je Massnahme **nur mit Beleg** aus dem Messprotokoll | `doc-writer` | T2 | S10, S11 | ☐ | — |
| Z.2 | `CLAUDE.md`: Gefahrentabelle (POST, `network_secrets`), Wache mit `--pruefe` | Lead | E3.7 | S9, S11, `endpunkte.mjs` | ☐ | — |
| Z.3 | Spec-Endstand | `spec-writer` | Z.1 | jedes Kriterium mit Beleg oder Grund | — | — |

## Abschluss

- [ ] Alle Tasks erledigt
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release, Ergebnis im Messprotokoll
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt oder mit Grund offen
- [ ] Sieben Releases gebaut, je Flash-Umfang und Reihenfolge benannt, je committet, getaggt, gepusht
