# Tasks — Die Kommandobrücke reparieren

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

## Die zwei Dinge, die hier anders sind als sonst

**1. Es gibt zwei Flash-Runden, und zwischen ihnen steht eine Messung.** Die Spalte
„Flash/Messung" sagt, wo. Wer über eine solche Zeile hinweg zwei Tasks parallel
startet, zerstört die Messung — am 03.10.2026 genau so geschehen.

**2. Alle Quelltext-Tasks gehören demselben Agenten** (`stm-developer`) und berühren
teilweise dieselben Dateien. Sie laufen deshalb **streng seriell**. Die einzige
erlaubte Parallelität steht unten ausdrücklich.

---

## Runde 1 — messen und die akute Regression entschärfen

| # | Task | Agent | Dateien | Hängt ab von | Flash/Messung | Guardrails | Review |
|---|---|---|---|---|---|---|---|
| 1 | **Quittung nur auf den Punkt.** `OK …` bekommt einen eigenen Rückgabewert; `main.c:3020` setzt `var_send_busy` nur noch beim Punkt zurück. Design §1 | `stm-developer` | `src/esp8266/esp8266.c`, `esp8266.h`, `src/main.c` | — | | ☐ | ☐ |
| 2 | **Reload-Budget je Hauptloop-Durchlauf.** Nullpunkt am Kopf des Hauptloops neben `watchdog_reload()`, Bedingung in `var_send_buf()`, Konstante `VAR_SEND_RELOAD_BUDGET_SEC = 30` mit der Tabelle aus Design §2 als Kommentar. **Keine neue `watchdog_reload()`-Aufrufstelle** | `stm-developer` | `src/vars/vars.c`, `src/main.c` | 1 | | ☐ | ☐ |
| 3 | **Zwei Messfelder.** Timeout- und Verschachtelungszähler in `vars.c` (sättigende `uint16_t`, Zugriffsfunktionen in `vars.h`), Feld `v=<t>/<n>` in der Diagnosezeile. Längenrechnung aus Design §3 in den Kommentar | `stm-developer` | `src/vars/vars.c`, `vars.h`, `src/main.c` | 2 | | ☐ | ☐ |
| 4 | **`measure-log.sh`: grösstes `Δu`.** Grösste Differenz von `u=` zwischen zwei aufeinanderfolgenden `seq`-Nummern, mit Angabe der beiden Nummern. Ein `seq`-Neuanfang bei 1 wird **getrennt** gemeldet, nicht als grosses `Δu` | Lead | `tools/measure-log.sh` | — (**darf parallel zu 1–3 laufen**) | | ☐ | ☐ |
| 5 | **Guardrail-Kommentar.** Der Kommentar zu `WD_EXPECTED` (`guardrails.sh:221-231`) nennt die neue Begründung: nach Quittung **und** innerhalb des Budgets. Die Zahl bleibt **7** | Lead | `tools/guardrails.sh` | 2 | | ☐ | ☐ |
| 6 | **STM-Version anheben** (DIR-004) | `release-engineer` | `src/main.h` | 3, 4, 5 | | ☐ | ☐ |
| 7 | **Bauen, Release-ZIP, Rollout** nach `test8` (DIR-005, **nie `--delete`**) | Lead | — | 6 | | ☐ | ☐ |
| — | **▶ FLASH-RUNDE 1 — STM.** `./tools/flash-stm.sh --check`, dann `./tools/flash-stm.sh`. Kein ESP-OTA, der ESP ist unverändert | Nutzer | — | 7 | **FLASH** | — | — |
| 8 | **Smoketest und Ruhemessung.** `./tools/smoke-device.sh` grün. `./tools/measure-log.sh 300`: `v=0/0` über 10 Minuten (AK2), grösstes `Δu` ≤ `DIAG_INTERVAL_SEC + 2 s` (AK5), keine Diagnosezeile mit `~` (AK4) | Lead | — | Flash 1 | | — | — |
| 9 | **▶ MESSUNG M1 — die Frage aus A18.** `./tools/snapshot-device.sh referenz`, Diagnosezeile notieren, **ESP-Neustart durch den Nutzer**, danach `/api/stm32_log` lesen und `./tools/snapshot-device.sh` + `./tools/diff-snapshot.sh`. Ergebnis gegen die Deutungstabelle aus Design §3 eintragen | Lead | — | 8 | **MESSUNG — hier läuft nichts anderes am Gerät** | — | — |

### Ergebnis von M1 — hier eintragen, bevor Runde 2 beginnt

```
Datum:
v= vorher:                 v= nachher:
seq-Neuanfang?             groesstes Δu:
Abzugsvergleich:           Abweichungen in:
Gelaufener Weg:            ☐ (A) Timeouts   ☐ (B) verschachtelt   ☐ beide   ☐ keiner
```

**Ist das Ergebnis „keiner von beiden"**, wird Runde 2 nicht wie geplant gebaut. Dann
liegt die Ursache nicht in `var_send_buf()`, und Abschnitt 4 des Designs steht zur
Disposition. Das ist ein Abbruchkriterium, keine Formalie.

---

## Runde 2 — reparieren

| # | Task | Agent | Dateien | Hängt ab von | Flash/Messung | Guardrails | Review |
|---|---|---|---|---|---|---|---|
| 10 | **Vollabgleich vertagen.** `var_send_all_variables()` vermerkt nur noch; Schritttabelle mit Index; Takt im Hauptloop neben dem IR-Abzug (`main.c:3506-3534`), inkl. derselben Wiedereintrittsprüfung. `var_send_overlays()` zerlegen. **AK9: kein Schritt über 32 Kommandos, Zahlen je Eintrag im Kopfkommentar.** Design §4 | `stm-developer` | `src/vars/vars.c`, `vars.h`, `src/main.c` | 9 | | ☐ | ☐ |
| 11 | **Wiederholung, zweiter Auslöser, Abschlusszeile.** Eine Wiederholung je Kommando; `VAR_SYNC_RETRY_DELAY_SEC = 20`, `VAR_SYNC_MAX_TRIES = 3`; genau eine Abschlusszeile je Abgleich, **ohne Variablenwerte**. `var_sync_active` auf **beiden** Endpfaden auflösen | `stm-developer` | `src/vars/vars.c`, `vars.h` | 10 | | ☐ | ☐ |
| 12 | **Timeout-Meldung ohne Geheimnisse** (Design §5b). Kennung, Index und Länge statt `buf`. Gegenprobe: `grep` zeigt kein `%s` mehr auf `buf` | `stm-developer` | `src/vars/vars.c` | 11 | | ☐ | ☐ |
| 13 | **Spiegelung blockiert nicht mehr.** Flush entfernen; Zugriffsfunktion auf den freien Platz im Senderring; ganz oder gar nicht; `!n`-Marke auf der nächsten durchgekommenen Zeile. Design §5 | `stm-developer` | `src/esp8266/esp8266.c`, `esp8266.h`, `src/uart/uart-driver.h`, `src/uart/uart.h` | 12 | | ☐ | ☐ |
| 14 | **Tetris und Snake.** `watchdog_reload()` an den drei Stellen aus Design §7; `GAME_IDLE_TIMEOUT_SEC = 30` als **eine** von beiden Spielen benutzte Konstante; `status` in `snake.c:291` initialisieren. **Nichts am Spielablauf ändern** | `stm-developer` | `src/tetris/tetris.c`, `src/tetris/snake.c` | 13 | | ☐ | ☐ |
| 15 | **`WD_EXPECTED` 7 → 10**, mit Begründung im Kommentar: Hauptloop, `remote_ir_learn`, `display_test` (2), Ticker-Warteschleife (2), `var_send_buf` (nach Quittung und im Budget), Tetris (2), Snake (1) | Lead | `tools/guardrails.sh` | 14 | | ☐ | ☐ |
| 16 | **Smoketest erweitern.** `v=`-Felder auslesen und melden; prüfen, dass nach einem Abgleich eine `var sync:`-Zeile im Ring steht; `!n`-Marken melden statt sie zu verschlucken | Lead | `tools/smoke-device.sh` | 11 (**darf parallel zu 12–15 laufen**) | | ☐ | ☐ |
| 17 | **STM-Version anheben** (DIR-004, zweite Erhöhung) | `release-engineer` | `src/main.h` | 15, 16 | | ☐ | ☐ |
| 18 | **Bauen, Release-ZIP, Rollout** | Lead | — | 17 | | ☐ | ☐ |
| — | **▶ FLASH-RUNDE 2 — STM.** Wieder kein ESP-OTA | Nutzer | — | 18 | **FLASH** | — | — |
| 19 | **▶ NACHWEIS N1 — A6, A17, A19.** Dreimal: Referenzabzug, **ESP-Neustart durch den Nutzer**, Abzugsvergleich (AK12), `var sync:`-Zeile (AK11/AK13), `v=…/<klein>` (AK13), grösstes `Δu` (AK14), `/api/stm32_log` ohne Schlüssel und Hostnamen (AK16), `!n`-Marke mindestens einmal aufgetreten (AK15) | Lead | — | Flash 2 | **MESSUNG — hier läuft nichts anderes am Gerät** | — | — |
| 20 | **▶ NACHWEIS N2 — A16.** Diagnosezeile notieren. **Nutzer startet ein Spiel und rührt es nicht an.** Erwartet: Es endet nach rund 30 s von selbst, Punkteticker läuft, `seq` und `u` danach lückenlos fortgesetzt (AK20). Zweiter Durchgang mit `GTq` | Nutzer + Lead | — | 19 | **erst nach N1** | — | — |
| 21 | **Befundkatalog und Changelog nachführen.** L106, L107, L108, L109, L110, L42, L102, L103 auf den neuen Stand; **neuer Eintrag für P6** (Klartext-Schlüssel in der Timeout-Meldung) mit Beleg `vars.c:104-105`; ToDo-Liste in `BEFUNDE.md` mitziehen (Guardrail S10) | `doc-writer` | `BEFUNDE.md`, `CHANGELOG.md` | 20 | | ☐ | ☐ |

---

## Warum N2 nach N1 steht und nicht daneben

A16 wirkt **ausschliesslich**, solange ein Spiel läuft, und während N1 läuft kein
Spiel. Beide dürfen deshalb in derselben Flash-Runde liegen — aber nicht am selben
Gerät zur selben Zeit. Ein Spiel, das während eines ESP-Neustarts läuft, macht aus
zwei sauberen Nachweisen einen unauswertbaren.

**Wer nur die Brücke braucht und die Spiele nicht:** Task 14 und 15 streichen. Sie
haben keine Abhängigkeit zum Rest, und Task 17 rückt auf. Dann bleibt `WD_EXPECTED`
bei 7.

---

## Die zwei erlaubten Parallelitäten

| Zusammen erlaubt | Warum |
|---|---|
| Task 4 (Lead, `tools/**`) neben 1–3 (`stm-developer`, `src/**`) | Verschiedene Besitzer, disjunkte Dateien. Task 4 muss aber **vor** Task 8 fertig sein, sonst fehlt dort die Kennzahl |
| Task 16 (Lead, `tools/**`) neben 12–15 | Dasselbe. Task 15 ist Lead und berührt eine andere Datei als 16 |

Alles andere läuft seriell. Insbesondere laufen **nie** zwei Tasks des
`stm-developer` gleichzeitig, auch nicht bei disjunkten Dateien — der Hook
`file-ownership.py` liesse es zu, R3 nicht.

---

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um. Keine stillen Mitkorrekturen —
   der Abschnitt „Nicht Teil dieser Änderung" in `requirements.md` ist die Grenze.
2. **Umschrift in `src/**`.** Diese Dateien sind ASCII beziehungsweise ISO-8859-1.
   `Bruecke`, `waehrend`, `zurueck`, `Quittung` — auch in neuen `log_printf`-Texten.
   Ein `ä` in einer dieser Dateien beschädigt sie.
3. **Kein `make`.** Teammates bauen nicht (R1); der Hook `no-build.py` weist es ab.
   Der Lead baut einmal je Runde.
4. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings.
5. Review ausdrücklich gegen `knowledge/architecture-checklist.md`.
6. Erst dann nächster Task oder Schreibübergabe.

## Was der Lead nicht tut

Der Lead schreibt in diesem Paket **keine Zeile unter `src/**`** — auch nicht die
zwei offensichtlichen aus Task 14. Genau diese Abkürzung hat am 03.10.2026 R3
gebrochen: Der zuständige Agent fand die Korrektur vor und brach seinen Patch ab,
weil sein Skript jeden Anker auf „genau einmal" prüft. Verhindert hat den Schaden
seine Sorgfalt, nicht die Regel.

## Abschluss

- [ ] Alle Tasks erledigt
- [ ] Ergebnis von M1 eingetragen, und Runde 2 ist darauf abgestimmt
- [ ] `./tools/guardrails.sh --full` mit Exit 0 (schliesst Compile-Smoke-Tests ein)
- [ ] `./tools/smoke-device.sh` grün
- [ ] Alle 21 Akzeptanzkriterien aus `requirements.md` erfüllt oder ausdrücklich
      und begründet abgewählt
- [ ] Zwei Release-ZIPs gebaut, beide Male ausdrücklich benannt: **STM flashen,
      ESP nicht, PWA nicht**
- [ ] `./tools/install-app.sh --check` trotzdem gelaufen — die PWA ändert sich
      nicht, aber ein Blick kostet nichts und hat am 02.10.2026 gefehlt
