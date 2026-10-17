# Tasks — Paket 2026-10-17 „C80: bleibende Heap-Zerstückelung nach frühem Verkehr — Firmware-Ursache“

**Erstellt:** 2026-10-17, `spec-writer`, Stand `353a693`. Anforderungen: `requirements.md`; Design: `design.md`.
**Status: freigegeben am 17.10.2026 (Task 0.1); Ent-C1 = A, Ent-C2 ≤ 320 Byte, Ent-C4 bestätigt.** **Nachtrag N1**
(17.10.2026, Stand `6b3cbe9`): Stand der Tasks 0.1, V.1, V.2, 1.1 und 1.2 nachgeführt; V.3 läuft. Neu ist Task **V.4**
(Nachtrag N2, nur wenn V.3 den Umfang von Stufe 1 ändert). Offen vor Task 1.3: **V.3** und gegebenenfalls **V.4**
**[offen: V.3]** (→ N2). Offen vor Stufe 3: Ent-C3. Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.

**Nachtrag N2** (17.10.2026, Stand `a803e98`, Task V.4): **V.3 erledigt** (L501); die Zusatzprobe des Leads ist als
**V.2b** nachgetragen; **V.4 erledigt** — Nachtrag N2 geschrieben, er ändert den Umfang von Stufe 1. **Offen vor Task 1.3
nur noch V.4: Deine Freigabe von N2 und Dein Entscheid Ent-C5** (`requirements.md`, Entscheidungen). Die Nachträge des
Leads aus Stufe V (Gegenproben (a)/(c), `--pruefe`, Ankündigung, Ent-C1 ins Messprotokoll) sind erledigt (L500 „Nachtrag
Protokoll“). Offen vor Stufe 3: Ent-C3. Jede Änderung trägt die Marke **N2**; Überholtes bleibt stehen. *Zum Stand N1
oben, überholt (N2):* „V.3 läuft“ und „Offen vor Task 1.3: V.3“.

*Stand bei Erstellung, überholt durch N1:* **Status: Entwurf, zur Freigabe.** Offen vor dem Start: Deine Freigabe; Ent-C2
(Lead, bei der Freigabe). Offen vor Task 1.3: Ent-C1. Offen vor Stufe 3: Ent-C3.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem schreibenden Task läuft `./tools/guardrails.sh`; erst bei
Exit 0 geht die Schreibberechtigung weiter. Wo „Nutzer“ steht, ist eine Handlung oder ein Entscheid von Dir gemeint, kein
Schreibzugriff.

**Die Spalte „Hängt ab von“ ist verbindlich, auch wo die Dateien disjunkt sind** (R3b):

1. **Reihenfolge der Stufen:** V ⇒ 1 ⇒ 2 ⇒ 3 ⇒ R ⇒ Z. Code für das nächste Release darf entstehen, während gemessen
   wird; **gebaut und eingespielt** wird erst nach dem Ende der Messung davor.
2. **Parallel erlaubt:** V.1/V.2 (Lead, Gerät) und 1.1 (`esp-developer`, nur lesend, ohne Gerät) — verschiedene Agenten,
   keine Abhängigkeit. **Nicht parallel:** 1.3 wartet auf V.3 **und** 1.2. **N1:** und auf V.4, falls V.3 einen Nachtrag
   zum Umfang verlangt. **N2:** V.3 ☑, 1.2 ☑; 1.3 wartet nur noch auf die Abnahme von V.4 (Deine Freigabe von N2 samt
   Ent-C5). Die Werkzeugarbeit für Ent-C5 und die Summenprobe (Teil von 1.6, Lead) darf parallel zu 1.3 entstehen —
   verschiedene Agenten, disjunkte Dateien, keine Abhängigkeit; ihre Gegenproben laufen in 1.6.
3. **Ein Schreiber je Datei:** Alle ESP-Tasks eines Releases schreibt **ein** `esp-developer`, nacheinander.
4. **Eine Messung zur Zeit (R5):** Während V.2, 2.1 und 3.8 kein OTA ausser dem Auslöser der Arme, kein Upload, kein
   Browser an der Uhr, kein zweites Werkzeug. Beginn und Ende kündigt der Lead an. **N1:** In V.2 hat in Lauf 5 ein
   Safari-Tab aus der wiederhergestellten Sitzung des Nutzers die Uhr angefragt (L500; `requirements.md` AKV.4 N1).
   **N2:** gilt ebenso für den Lauf E in 1.11 (AKI.14 (b)) — ein Browser an der Uhr in W verfälscht Töpfe und Karte `T`.
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
vereinheitlicht Zeilenenden still (CLAUDE.md, 04.10.2026). **N1:** Stand des Berichts 1.1 in `design.md` §4.10 —
`ESP-uclock.ino` wechselt das Zeilenende in 15 Abschnitten; gepatcht wird byte-genau je Abschnitt. **N2:** In
`ESP-uclock.ino` wird nur noch `loop()` und der Bereich um `ESP_HEAP_LOG` geändert, `setup()` nicht (`design.md` §4.10 N2).

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — Freigabe

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | Spec freigeben; Ent-C2 bestätigen; Ent-C4 bestätigen oder ändern — **N1: erledigt 17.10.2026**: Spec freigegeben, Ent-C4 bestätigt (Du), Ent-C2 ≤ 320 Byte bestätigt (Lead) | Nutzer (Ent-C2: Lead) | — | Freigabe ☑ | — | — |

## Stufe V — Vorprobe ohne Build (ESP 3.2.41)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| V.1 | **Werkzeug der Arme** aus dem Scratchpad nach `tools/` übernehmen; Arm E, Mitschrift der ersten Antwort, Exit-Codes, `--abnahme` (`design.md` §3.2); Gegenproben (a), (c) — **N1: Werkzeug erledigt** als `tools/startprobe.py` (Commit `f012309`); Beleg der Gegenproben (a) und (c) steht nicht im Messprotokoll — vom Lead nachzutragen — **N2: erledigt**; Gegenproben (a) und (c) belegt in L500 „Nachtrag Protokoll“; „vom Lead nachzutragen“ ist überholt | Lead | 0.1 | **AKV.1** (ohne (b)) — N1: teilweise — **N2: erfüllt** | ☐ | — |
| V.2 | **Vorprobe am Gerät:** neun Läufe A B E · E A B · B E A; Logwache; Ergebnis ins Messprotokoll; Gegenprobe (b) aus AKV.1 mit `--abnahme 7104` — **N1: gelaufen 17.10.2026 16:35–17:14**, Ergebnis L500 (Commit `6b3cbe9`): A 6'600 × 3, E 6'600 · 6'600 · 6'432, B 7'104 × 3; Gegenprobe (b) eingelöst. Nachzutragen: `--pruefe` am Ende und Ankündigung von Beginn und Ende; Lauf 5 mit fremdem Verursacher, gekennzeichnet — **N2: Nachträge erledigt** (L500 „Nachtrag Protokoll“): Ankündigung und Marken im Mitschnitt; `--pruefe` vor dem Lauf, Auswertung der Logwache bis nach dem Ende; ein Antwortverlust in Lauf 9, gekennzeichnet | Lead | V.1 | **AKV.1 (b)**, **AKV.2–AKV.4**, AKZ.6, AKZ.8 — N1: AKV.1 (b), AKV.2, AKV.3 erfüllt; AKV.4, AKZ.6 teilweise — **N2: AKV.1–AKV.4 erfüllt; AKZ.6 und AKZ.8 für Stufe V belegt** | — | — |
| **V.2b** (N2, nachgetragen) | **Zusatzprobe am Gerät** (17.10.2026 18:23–18:52, `tools/startprobe.py R,E8,E14,E10,R,E12,B`, Stand `3b9528a`): eine Anfrage nach 8,1 / 10,1 / 12,1 / 14,1 s ⇒ 7'104 (4 von 4); B 7'112; RST ⇒ 7'104 und 6'928; Logwache bis zum Ende nachgelesen, kein Neustart ausser den gewollten, keine Exception, kein Antwortverlust. Werkzeugfehler (E-Läufe fälschlich „ungültig“) behoben, mit Gegenprobe | Lead | V.2 | Messprotokoll **L501** ☑ | — | — |
| V.3 | **Lesart der Vorprobe** nach `design.md` §3.4; Hinweise für Stufe 2; ob eine Annahme aus §2 nicht trägt — **N1: läuft.** Ergebnis nach `design.md` §3.5. Zu beantworten ausserdem: Q8 (`design.md` §12 N1) — was im Fenster nach `setup()` belegt wird (auch die Hintergrundbelegung bis +30 s in Arm B) und ob Stufe 1 dort Erfassungspunkte braucht **[offen: V.3]** (→ N2) — **N2: erledigt 17.10.2026**; zusammengefasst in L501, eingetragen in `design.md` §3.5 N2; Q8 beantwortet (`design.md` §12 N2). Ergebnis: Stufe 1 braucht Erfassungspunkte nach `setup()` | `firmware-analyst` | V.2 | Bericht ☑ | — | — |
| **V.4** (N1) | **Nachtrag N2 zum Umfang von Stufe 1** — nur wenn V.3 ihn verlangt (zusätzliche Erfassungspunkte nach `setup()`, Änderung an M2/M3); mit Wirkung auf AKI.4, AKI.5, AKI.9, AKI.10 und `design.md` §4.3, §4.4, §4.6. Verlangt V.3 nichts, entfällt V.4 mit Vermerk — **N2: erledigt 17.10.2026** (geschrieben): N2 in `requirements.md`, `design.md` und `tasks.md`. Neu gefasst: AKI.1, AKI.2, AKI.4–AKI.6, AKI.8–AKI.11, AKM.1–AKM.4; neu: AKI.13, AKI.14, Ent-C5; `design.md` §2–§5, §8–§10, §12 | `spec-writer` | V.3 | Nachtrag freigegeben (Nutzer) oder Vermerk „entfällt“ — **N2: Freigabe von N2 und Ent-C5 durch Dich ausstehend** | — | — |

## Stufe 1 — Instrument (Release I, nur ESP)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 1.1 | **Machbarkeitsbericht, nur lesend:** Zugriff auf Blockfeld, Blockzahl und Blockform (Core 3.1.2, mit Fundstellen); Mittel der Sperre; Beziehung `gross`↔`max=` und `fsum`↔`free=`; Erreichbarkeit jeder Wegmarke aus §4.5 und von `_unclaimed` über `WcHttpServer`; Gangdauer geschätzt; Kodierung und Zeilenende jeder Zieldatei; Platz im BSS gerechnet — **N1: erledigt 17.10.2026**; Ergebnisse in `design.md` §4.2–§4.10 (Marke N1). Der Bericht selbst liegt nicht im Repository | `esp-developer` | 0.1 | Bericht ☑ | — | — |
| 1.2 | **Ent-C1** vorlegen (Rückfrage an Dich mit den Optionen aus `design.md` §4.9 und dem Bericht 1.1); Entscheid ins Messprotokoll — **N1: entschieden 17.10.2026: A.** Eintrag ins Messprotokoll steht aus (in `BEFUNDE.md` am Stand `6b3cbe9` nicht gefunden) — vom Lead nachzutragen — **N2: Eintrag erledigt** (L500 „Nachtrag Protokoll“, Ent-C1, Ent-C2, Ent-C4) | Lead, Entscheid Nutzer | 1.1 | Ent-C1 ☑ | — | — |
| 1.3 | **M1–M4** nach `design.md` §4 und Ent-C1, hinter `ESP_HEAP_MAP`; **Prüfstand c80i** im Scratchpad. **N1:** einschliesslich der Präzisierungen mit Marke N1 in `design.md` §4.2–§4.10 (Blöcke × 8, Wächter `sum`, Sperre ohne Verzweigung, `srv=`, exportierte Funktionen in `http.cpp`, Wegmarken mit Kontexten, Bindungsmittel); BSS für die endgültige Zeilenform nachrechnen (`design.md` §4.6 N1). **N2 — Umfang nach N2:** M1 mit `m` nur ausserhalb W und `ms=` an `S`/`F`/`X`/`T` (§4.2 N2); **M2 = Verursacher-Töpfe** in `loop()` samt `sys`, Summenprobe, Ausgabe nach W (§4.3 N2); **M3 = `S`, `F`, `X`, `T`** (§4.4 N2); **M4 = ein Satz Wegmarken bei `X` bzw. `W`**, `tcp` zuerst (§4.5 N2); keine Ausgabe in W; W nach Ent-C5. **Entfallen:** s0–s8, `srv=`, `u`/`p`, die exportierte Funktion für `_unclaimed`, Karte `U`; **`setup()` bleibt unverändert**. Prüfstand mit den Fällen 15–22 und Sabotagen N2 (§4.8 N2). **BSS vor dem Bau nachrechnen** (gerechnet 304, §4.6 N2); über 320 ⇒ Kürzungsreihenfolge aus AKI.9 N2, dann vorlegen | `esp-developer` | 1.2 ☑, **V.3** ☑; **N1:** und V.4, falls nicht entfallen — **N2:** V.4 (Deine Freigabe von N2 und Ent-C5) | **AKI.1–AKI.7**, AKZ.1 — **N2:** dazu AKI.13 (a) und AKI.14 (a) als Prüfstand im Scratchpad | ☐ | — |
| 1.4 | ESP bauen; „Globale Variablen“ (ΔRAM(I)) und Programmgrösse sofort ins Messprotokoll; über Ent-C2 ⇒ anhalten und vorlegen | Lead | 1.3 | **AKI.9** (RAM) | — | — |
| 1.5 | **c80i** unter `tools/checks/auszug/esp/c80i/` ablegen, in S16; Gegenprobe gegen 3.2.41; jede Sabotage; Fallzahl nachgezählt — **N2:** einschliesslich der Fälle 15–22 und der Sabotagen N2; Fall 14 und „`srv=` fehlt“ entfallen | Lead | 1.4 | **AKI.8**, AKZ.5 — **N2:** dazu **AKI.13 (a)**, **AKI.14 (a)** | ☐ | — |
| 1.6 | **Werkzeuge:** Logwache meldet `abbruch=`; Werkzeug der Arme sammelt `start`/`karte`/`marken` je Lauf und meldet fehlende Zeilen; Prüfung, dass `watch-log.sh`, `smoke-device.sh`, `mess-stabilitaet.sh` ihre Werte unverändert lesen; je Gegenprobe — **N2:** `start` ist überholt; das Werkzeug sammelt `topf`/`karte`/`marken`, prüft das Zählsoll nach AKI.10 N2 und die Summenprobe (Exit ≠ 0), meldet „nicht eingetreten“ für `F`, `X`, `T` und das Fehlen von `marken` als FEHL; Beginn der Folge als Einstellung (Ent-C5); Arm E gültig bis 3,2 s; Logwache ohne Meldung für `topf`, `X`, `T`; je Gegenprobe mit Meldung in Konsole und Mitschrift | Lead | 1.5 | **AKI.11**, AKM.2 (Werkzeugteil) — **N2:** dazu **AKI.13 (b)**, **AKI.14 (c)** (Werkzeugteil) | ☐ | — |
| 1.7 | **Review Release I** gegen `knowledge/architecture-checklist.md` und `design.md` §4 | `code-reviewer` | 1.6 | Checkliste | — | ☐ |
| 1.8 | ESP-Version anheben | `release-engineer` | 1.7 | AKZ.3, S4 | ☐ | — |
| 1.9 | `CHANGELOG.md`; Dateiliste gegen `release/3.2.34-3.2.41-1.4.102`; `git status`; `--full`; `make release-zip`; Commit, Tag, Push im selben Zug; `deploy.sh`; Einspielzeilen samt Rückweg | Lead | 1.8 | **AKZ.2**, **AKZ.4**, AKZ.5 | ☐ | — |
| 1.10 | **Release I einspielen:** OTA | Nutzer | 1.9 | — | — | — |
| 1.11 | **Nach dem Einspielen:** `install-app.sh --check`, Smoketest samt Update-Quelle, `check-pwa.sh`, Logwache; **Gegenprobe am Gerät** — jede Zeilenart im Mitschnitt und über `/api/stm32_log`, Zählung je Start, `ovr=`/`d=`; zehn Paare Heap-Zeile/Karte in Ruhe; `us=` aus drei Starts — **N2:** Zählung nach AKI.10 N2; `us=` aus der Kopfzeile der Töpfe; **zusätzlich ein Lauf Arm E** mit dem Werkzeug (Folge nach Ent-C5), damit `T`, `X` und die Summenprobe vor Stufe 2 einmal am Gerät belegt sind; Summenprobe in drei Starts nachgerechnet | Lead | 1.10 | **AKI.3** (Gerät), **AKI.9** (`us=`), **AKI.10**, **AKI.12** — **N2:** dazu **AKI.13 (c)**, **AKI.14 (b), (d)** | — | — |

## Stufe 2 — Messung (auf Release I)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 2.1 | **Arme A, B, E** je dreimal, A B E · E A B · B E A; Mitschrift je Lauf; Logwache; Ergebnis ins Messprotokoll. Misslingt AKM.1 ⇒ anhalten (Z4) — **N2:** danach **zwei Läufe RST**; Folge nach Ent-C5; Arm E gültig bis 3,2 s; je Lauf Zählsoll und Summenprobe. Misslingt AKM.1 bei Ent-C5 = A, zuerst die Rückfalldiagnose aus AKM.1 N2 (drei Läufe A mit Folge ab +30 s), dann gegebenenfalls Z4. In den Läufen B und RST wird AKI.14 (c) am Gerät belegt | Lead | 1.11 | **AKM.1**, **AKM.2**, **AKM.5**, AKZ.6 — **N2:** dazu **AKI.14 (c)** (Gerät) | — | — |
| 2.2 | **Auswertung F1–F5 und Zuordnung Z1–Z4** aus dem Messprotokoll, jede Aussage mit Kennzeichen — **N2:** Rangfolge F3, F2, F4, F1; F5 erledigt; F6 informativ; zuerst Wid-1 und Wid-2, dann Z2 (L-b′ oder L-c), Z3 oder Z4 nach `design.md` §5.4 N2; Z1 entfällt | `firmware-analyst` | 2.1 | **AKM.3**, **AKM.4** | — | — |
| 2.3 | **C80 nachführen** (Stand, Zuordnung); Rückbau des Instruments in der ToDo-Liste, falls noch nicht geschehen | `doc-writer` | 2.2 | AKZ.7 (Eintrag), AKZ.8 | ☐ | — |
| 2.4 | **Nachtrag A.1 zu dieser Spec:** bei Z1/Z2 Abhilfe-Optionen mit Preis, Risiko, Komponenten und Prüfstand; bei Z3/Z4 Nachtrag zum Instrument (⇒ Schleife) — **N2:** bei Z2; ist der Verursacher nur als Topf benannt, mit der Option „erst identifizieren“; das Ergebnis von F6 (RST) gehört hinein | `spec-writer` | 2.2 | Nachtrag | — | — |
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
| R.1 | M2–M4 entfernen; M1 entfernen oder `ESP_HEAP_MAP` 0; Kommentar nachführen — **N2:** M2 sind die Töpfe in `loop()` | `esp-developer` | 3.8, 3.9 | AKZ.1 | ☐ | — |
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

**N1 — offen für den Lead aus Stufe V** (damit es nicht untergeht): Beleg der Gegenproben (a) und (c) zu AKV.1;
`--pruefe`-Ergebnis und Ankündigung zu V.2 (AKV.4); Ent-C1 = A ins Messprotokoll (Task 1.2).

**N2 — Stand:** Die drei Punkte aus N1 sind erledigt (L500 „Nachtrag Protokoll“). **Zählung nach N2:** 39 Kriterien (AKV 4,
AKI 14, AKM 5, AKA 7, AKZ 9); erfüllt AKV.1–AKV.4. **Offen vor Task 1.3:** Deine Freigabe von N2 und Ent-C5.
