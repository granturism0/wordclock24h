# Tasks — Paket 2026-10-17 „C80: bleibende Heap-Zerstückelung nach frühem Verkehr — Firmware-Ursache“

**Erstellt:** 2026-10-17, `spec-writer`, Stand `353a693`. Anforderungen: `requirements.md`; Design: `design.md`.
**Status: Entwurf, zur Freigabe.** Offen vor dem Start: Deine Freigabe; Ent-C2 (Lead, bei der Freigabe). Offen vor Task
1.3: Ent-C1. Offen vor Stufe 3: Ent-C3. Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem schreibenden Task läuft `./tools/guardrails.sh`; erst bei
Exit 0 geht die Schreibberechtigung weiter. Wo „Nutzer“ steht, ist eine Handlung oder ein Entscheid von Dir gemeint, kein
Schreibzugriff.

**Die Spalte „Hängt ab von“ ist verbindlich, auch wo die Dateien disjunkt sind** (R3b):

1. **Reihenfolge der Stufen:** V ⇒ 1 ⇒ 2 ⇒ 3 ⇒ R ⇒ Z. Code für das nächste Release darf entstehen, während gemessen
   wird; **gebaut und eingespielt** wird erst nach dem Ende der Messung davor.
2. **Parallel erlaubt:** V.1/V.2 (Lead, Gerät) und 1.1 (`esp-developer`, nur lesend, ohne Gerät) — verschiedene Agenten,
   keine Abhängigkeit. **Nicht parallel:** 1.3 wartet auf V.3 **und** 1.2.
3. **Ein Schreiber je Datei:** Alle ESP-Tasks eines Releases schreibt **ein** `esp-developer`, nacheinander.
4. **Eine Messung zur Zeit (R5):** Während V.2, 2.1 und 3.8 kein OTA ausser dem Auslöser der Arme, kein Upload, kein
   Browser an der Uhr, kein zweites Werkzeug. Beginn und Ende kündigt der Lead an.
5. **Schleife:** Ergibt Stufe 2 Z3 oder Z4, folgt ein Nachtrag zum Instrument (`spec-writer`), ein Release I′ nach dem
   Ablauf 1.3–1.11 und eine neue Stufe 2 — **keine** Abhilfe.

**Bauen nur der Lead (R1).** Kein Agent ausser dem Lead führt `make` aus. **Versionen** hebt der `release-engineer` im
Auftrag des Leads (R4), und nur `ESP_VERSION` (DIR-004).

**Messwerte sofort (L424):** Jeder messende Task trägt sein Ergebnis **vor dem nächsten Task** ins Messprotokoll
(eigene L-Zeile in `BEFUNDE.md`, AKZ.8). Messzeilen schreibt der Lead, Befundnachführungen der `doc-writer` — nie
gleichzeitig (R3).

**Einspielen:** Du spielst jedes Release selbst ein; der Lead liefert die fertigen Zeilen samt Reihenfolge und Rückweg
(AKZ.4). Jeder Gerätelauf mit `./tools/watch-log.sh`, `--pruefe` am Ende (DIR-013); auf später geplante Läufe mit
`caffeinate` (DIR-020).

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. Prüfstände baut der Umsetzer im
Scratchpad; der Lead legt sie unter `tools/checks/auszug/esp/` ab und fährt die Gegenproben (DIR-014).

**Vor jedem Patch** Kodierung **und** Zeilenende der Datei feststellen und im Taskbericht nennen (AKZ.1, DIR-015) —
`ESP-uclock.ino` hat gemischte Zeilenenden, `http.cpp` ist UTF-8 mit einer CRLF-Zeile; ein Patch über das Edit-Werkzeug
vereinheitlicht Zeilenenden still (CLAUDE.md, 04.10.2026).

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — Freigabe

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | Spec freigeben; Ent-C2 bestätigen; Ent-C4 bestätigen oder ändern | Nutzer (Ent-C2: Lead) | — | Freigabe | — | — |

## Stufe V — Vorprobe ohne Build (ESP 3.2.41)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| V.1 | **Werkzeug der Arme** aus dem Scratchpad nach `tools/` übernehmen; Arm E, Mitschrift der ersten Antwort, Exit-Codes, `--abnahme` (`design.md` §3.2); Gegenproben (a), (c) | Lead | 0.1 | **AKV.1** (ohne (b)) | ☐ | — |
| V.2 | **Vorprobe am Gerät:** neun Läufe A B E · E A B · B E A; Logwache; Ergebnis ins Messprotokoll; Gegenprobe (b) aus AKV.1 mit `--abnahme 7104` | Lead | V.1 | **AKV.1 (b)**, **AKV.2–AKV.4**, AKZ.6, AKZ.8 | — | — |
| V.3 | **Lesart der Vorprobe** nach `design.md` §3.4; Hinweise für Stufe 2; ob eine Annahme aus §2 nicht trägt | `firmware-analyst` | V.2 | Bericht | — | — |

## Stufe 1 — Instrument (Release I, nur ESP)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 1.1 | **Machbarkeitsbericht, nur lesend:** Zugriff auf Blockfeld, Blockzahl und Blockform (Core 3.1.2, mit Fundstellen); Mittel der Sperre; Beziehung `gross`↔`max=` und `fsum`↔`free=`; Erreichbarkeit jeder Wegmarke aus §4.5 und von `_unclaimed` über `WcHttpServer`; Gangdauer geschätzt; Kodierung und Zeilenende jeder Zieldatei; Platz im BSS gerechnet | `esp-developer` | 0.1 | Bericht | — | — |
| 1.2 | **Ent-C1** vorlegen (Rückfrage an Dich mit den Optionen aus `design.md` §4.9 und dem Bericht 1.1); Entscheid ins Messprotokoll | Lead, Entscheid Nutzer | 1.1 | Ent-C1 | — | — |
| 1.3 | **M1–M4** nach `design.md` §4 und Ent-C1, hinter `ESP_HEAP_MAP`; **Prüfstand c80i** im Scratchpad | `esp-developer` | 1.2, **V.3** | **AKI.1–AKI.7**, AKZ.1 | ☐ | — |
| 1.4 | ESP bauen; „Globale Variablen“ (ΔRAM(I)) und Programmgrösse sofort ins Messprotokoll; über Ent-C2 ⇒ anhalten und vorlegen | Lead | 1.3 | **AKI.9** (RAM) | — | — |
| 1.5 | **c80i** unter `tools/checks/auszug/esp/c80i/` ablegen, in S16; Gegenprobe gegen 3.2.41; jede Sabotage; Fallzahl nachgezählt | Lead | 1.4 | **AKI.8**, AKZ.5 | ☐ | — |
| 1.6 | **Werkzeuge:** Logwache meldet `abbruch=`; Werkzeug der Arme sammelt `start`/`karte`/`marken` je Lauf und meldet fehlende Zeilen; Prüfung, dass `watch-log.sh`, `smoke-device.sh`, `mess-stabilitaet.sh` ihre Werte unverändert lesen; je Gegenprobe | Lead | 1.5 | **AKI.11**, AKM.2 (Werkzeugteil) | ☐ | — |
| 1.7 | **Review Release I** gegen `knowledge/architecture-checklist.md` und `design.md` §4 | `code-reviewer` | 1.6 | Checkliste | — | ☐ |
| 1.8 | ESP-Version anheben | `release-engineer` | 1.7 | AKZ.3, S4 | ☐ | — |
| 1.9 | `CHANGELOG.md`; Dateiliste gegen `release/3.2.34-3.2.41-1.4.102`; `git status`; `--full`; `make release-zip`; Commit, Tag, Push im selben Zug; `deploy.sh`; Einspielzeilen samt Rückweg | Lead | 1.8 | **AKZ.2**, **AKZ.4**, AKZ.5 | ☐ | — |
| 1.10 | **Release I einspielen:** OTA | Nutzer | 1.9 | — | — | — |
| 1.11 | **Nach dem Einspielen:** `install-app.sh --check`, Smoketest samt Update-Quelle, `check-pwa.sh`, Logwache; **Gegenprobe am Gerät** — jede Zeilenart im Mitschnitt und über `/api/stm32_log`, Zählung je Start, `ovr=`/`d=`; zehn Paare Heap-Zeile/Karte in Ruhe; `us=` aus drei Starts | Lead | 1.10 | **AKI.3** (Gerät), **AKI.9** (`us=`), **AKI.10**, **AKI.12** | — | — |

## Stufe 2 — Messung (auf Release I)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 2.1 | **Arme A, B, E** je dreimal, A B E · E A B · B E A; Mitschrift je Lauf; Logwache; Ergebnis ins Messprotokoll. Misslingt AKM.1 ⇒ anhalten (Z4) | Lead | 1.11 | **AKM.1**, **AKM.2**, **AKM.5**, AKZ.6 | — | — |
| 2.2 | **Auswertung F1–F5 und Zuordnung Z1–Z4** aus dem Messprotokoll, jede Aussage mit Kennzeichen | `firmware-analyst` | 2.1 | **AKM.3**, **AKM.4** | — | — |
| 2.3 | **C80 nachführen** (Stand, Zuordnung); Rückbau des Instruments in der ToDo-Liste, falls noch nicht geschehen | `doc-writer` | 2.2 | AKZ.7 (Eintrag), AKZ.8 | ☐ | — |
| 2.4 | **Nachtrag A.1 zu dieser Spec:** bei Z1/Z2 Abhilfe-Optionen mit Preis, Risiko, Komponenten und Prüfstand; bei Z3/Z4 Nachtrag zum Instrument (⇒ Schleife) | `spec-writer` | 2.2 | Nachtrag | — | — |
| 2.5 | **Ent-C3:** Optionen per Rückfrage vorlegen; Deine Freigabe des Nachtrags | Lead, Entscheid Nutzer | 2.4 | **AKA.1** | — | — |

## Stufe 3 — Abhilfe (Release A)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 3.1 | **Abhilfe** nach dem freigegebenen Nachtrag; Prüfstand mit Sabotage im Scratchpad; Instrument bleibt | `esp-developer` (weitere Rollen nur, wenn der Nachtrag sie nennt — je eigener Task) | 2.5 | AKZ.1 | ☐ | — |
| 3.2 | ESP bauen; ΔRAM(A) und Programmgrösse ins Messprotokoll | Lead | 3.1 | — | — | — |
| 3.3 | Prüfstand ablegen, in S16; gegen Release I ⇒ FEHL; Sabotage; Fallzahl | Lead | 3.2 | **AKA.2**, AKZ.5 | ☐ | — |
| 3.4 | **Review Release A** | `code-reviewer` | 3.3 | Checkliste | — | ☐ |
| 3.5 | ESP-Version anheben | `release-engineer` | 3.4 | AKZ.3 | ☐ | — |
| 3.6 | `CHANGELOG.md`; Dateiliste gegen das Tag von Release I; `--full`; `make release-zip`; Commit, Tag, Push; `deploy.sh`; Einspielzeilen samt Rückweg | Lead | 3.5 | **AKZ.2**, **AKZ.4** | ☐ | — |
| 3.7 | **Release A einspielen** | Nutzer | 3.6 | — | — | — |
| 3.8 | **Abnahme am Gerät:** `install-app.sh --check`, Smoketest; Arme A, D, B (und jeder in Stufe 2 schlechte Arm) je dreimal mit `--abnahme`; Karte lesen; Ladeprobe warm; `check-pwa.sh` mit `low=`; Legacy; Logwache | Lead | 3.7 | **AKA.3–AKA.6**, AKZ.6 | — | — |
| 3.9 | **`pwa-tester`** (Phasen 0–4 und 9), wenn die Abhilfe mehr als eine Komponente berührt; sonst Auslassung mit Grund im Bericht | `pwa-tester` bzw. Lead | 3.8 | **AKA.7** | — | — |

**Reihenfolge bei mehreren Komponenten:** Nennt der Nachtrag ausser dem ESP eine weitere Komponente, kommt beim
Einspielen die **Firmware zuerst** (DIR-002), und der `pwa-tester` (3.9) läuft auf dem eingespielten Stand, **bevor**
Release A als abgenommen gilt und bevor Stufe R beginnt. Die genaue Folge legt der Nachtrag fest.

## Stufe R — Rückbau (nächstes ESP-Release nach der Abnahme)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R.1 | M2–M4 entfernen; M1 entfernen oder `ESP_HEAP_MAP` 0; Kommentar nachführen | `esp-developer` | 3.8, 3.9 | AKZ.1 | ☐ | — |
| R.2 | bauen; „Globale Variablen“ gegen Release A und gegen 3.2.41 | Lead | R.1 | **AKZ.7** (RAM) | — | — |
| R.3 | c80i auf den Rückbau umstellen (keine neue Zeile; Gegenprobe mit 1 ⇒ FEHL); Werkzeuge nachziehen | Lead | R.2 | **AKZ.7** | ☐ | — |
| R.4 | ESP-Version anheben | `release-engineer` | R.3 | AKZ.3 | ☐ | — |
| R.5 | Release wie 1.9; Einspielzeilen | Lead | R.4 | AKZ.2, AKZ.4 | ☐ | — |
| R.6 | einspielen | Nutzer | R.5 | — | — | — |
| R.7 | `install-app.sh --check`, Smoketest, `check-pwa.sh`, Logwache | Lead | R.6 | AKZ.6 | — | — |

Wird der Rückbau mit einem anderen ESP-Release zusammengelegt, gelten R.1–R.7 innerhalb jenes Releases; der Rückbau ist
dann ein eigener Task darin.

## Schritt Z — Abschluss

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | C80 schliessen oder Stand nachführen; Rückbau in der ToDo-Liste abhaken | `doc-writer` | R.7 | AKZ.8 | ☐ | — |
| Z.2 | **Endstand** `endstand.md` je Kriterium und Zählung | `spec-writer` | Z.1 | **AKZ.9** | — | — |

---

## Abschluss

- [ ] Alle Tasks erledigt
- [ ] `./tools/guardrails.sh --full` vor jedem Release mit Exit 0
- [ ] Alle Akzeptanzkriterien aus `requirements.md` belegt, entfallen mit Grund oder offen mit dem, was fehlt (`endstand.md`)
- [ ] Je Release ZIP gebaut, getaggt, gepusht; Flash-Umfang benannt: **nur ESP**
