# Tasks — Wiederholung auf der Kommandobrücke (A32 / L230)

**Momentaufnahme vom 2026-10-04** (DIR-006). Gehört zu `requirements.md` und
`design.md` in diesem Verzeichnis.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Quelltext-Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

---

## Die drei Dinge, die hier anders sind als sonst

**1. Die Spalte „Hängt ab von" ist verbindlich, auch bei disjunkten Dateien (R3b).**
Zwischen den Runden stehen Flash- und Messzeilen. Wer über eine solche Zeile hinweg
zwei Tasks parallel startet, zerstört die Messung — am 03.10.2026 genau so geschehen.
Dateibesitz regelt **wer**, nicht **wann**.

**2. Runde 0 ist keine Formalie.** AK5 vergleicht den Abzug nach einem ESP-Neustart
gegen einen Referenzlauf. Wird der nicht **vor** dem ersten Flash genommen, ist AK5
für immer unerfüllbar — und zwar nicht sichtbar, sondern still.

**3. Drei Flash-Runden, drei Tags.** STM → ESP → STM. Die Reihenfolge ist nicht
beliebig: Hängt der STM die Prüfsumme an, bevor der ESP sie versteht, beschädigt er
Zeichenkettenwerte (`design.md`, R-3).

---

## Runde 0 — messen, bevor etwas geändert wird

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **Referenzabzug und Ruhemessung.** `./tools/snapshot-device.sh referenz-vor-a32`; `./tools/measure-log.sh 600` | Lead | — | Der Abzug liegt als Datei vor; die vier Ruhefenster (`l=`-Differenz je 10 s) sind unten eingetragen. Rein lesend (DIR-008) | — | — |
| 0.2 | **▶ REFERENZLAUF R0 — ESP-Neustart ohne A32.** `./tools/watch-log.sh` **parallel** (DIR-013), ESP-Neustart durch den Nutzer, danach zweiter Abzug und `./tools/diff-snapshot.sh` | Lead + Nutzer | 0.1 | Die **Zahl der abweichenden Felder** und die `v=`-Werte vor/nach sind unten eingetragen. Ohne diese Zahl ist AK5 wertlos | — | — |

### Ergebnis von R0 — hier eintragen, bevor Runde 1 beginnt

```
Datum/Uhrzeit:
Ruhefenster (l je 10 s):      /      /      /
v= vor dem Neustart:          v= danach:
abweichende Felder:           davon ausserhalb ihres Bereichs:
HARDWARE_CONFIGURATION:       overlay[0].type:
```

---

## Runde 1 — die Nachsendeliste im STM

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | G | R |
|---|---|---|---|---|---|---|
| 1.1 | **Nachsendeliste.** Struktur, die fünf Konstanten aus Design 3.2, Ausschlussliste `var_retry_is_worth_it()` (3.3), zweiter Parameter `idlen` an `var_send_buf()` und an **allen** Aufrufstellen in derselben Datei, die beiden Kennungsregeln (3.4), vier sättigende Zähler mit Zugriffsfunktionen | `stm-developer` | 0.2 | `grep -a 'var_send_buf (' src/vars/vars.c` zeigt **keine** Aufrufstelle ohne zweiten Parameter. Jede der fünf Konstanten trägt den Kommentar, wogegen sie tauscht — so wie `VAR_SEND_RELOAD_BUDGET_SEC` es vormacht | ☐ | ☐ |
| 1.2 | **Drain im Hauptloop.** Neben dem IR-Abzug (`main.c`, Anker: Kommentar „genau EIN Kommando je Hauptloop-Durchlauf"), Ablauf aus Design 3.5 **einschliesslich** der Wiedereintrittsprüfung; die drei Logzeilen aus 3.6, `Liste voll` höchstens einmal je 60 s | `stm-developer` | 1.1 | `grep -c watchdog_reload src/**` unverändert; `./tools/guardrails.sh` S7 meldet denselben Bestand wie vorher (AK6). Keine neue Aufrufstelle | ☐ | ☐ |
| 1.3 | **`!v` und `CAP var-crc` verstehen** — `ESP8266_NAK`, `esp8266.cap_var_crc`, Zweige in `schedule_esp8266_messages()`. **Noch ohne Wirkung**, kein ESP sendet das heute | `stm-developer` | 1.2 | Im Mitschnitt nach dem Flash erscheint **keine** neue Zeile und `v=` bleibt bei 0/0 — die Änderung ist belegt wirkungslos, nicht vermutet wirkungslos | ☐ | ☐ |
| 1.4 | **`measure-log.sh`: `var retry:`-Zeilen.** Zählen und mit Kennung melden; `aufgegeben` und `Liste voll` getrennt ausweisen | Lead | 0.2 (**darf parallel zu 1.1–1.3 laufen**, anderer Besitzer, disjunkte Dateien) | Das Skript meldet an einem Mitschnitt mit künstlich eingefügter Zeile die richtige Zahl. Gegenprobe gefahren, sonst ist die Auswertung unbelegt | ☐ | ☐ |
| 1.5 | **STM-Version anheben** (DIR-004) | `release-engineer` | 1.3, 1.4 | `make stm-version-file` zeigt den neuen Stand | ☐ | ☐ |
| 1.6 | **Bauen, Speicher messen, Release-ZIP, Rollout** nach `test8` (DIR-005, **nie `--delete`**). `arm-none-eabi-size` für **F103 und F411**, vorher/nachher | Lead | 1.5 | `.bss`-Zuwachs ≤ 512 Byte auf **beiden** Zielen (AK7); beide Zahlen unten eingetragen. Reicht es nicht: `VAR_RETRY_SLOTS` auf 2, Task 1.1 erneut | ☐ | ☐ |
| — | **▶ FLASH-RUNDE 1 — nur STM.** `./tools/flash-stm.sh --check`, dann `./tools/flash-stm.sh`. **Kein ESP-OTA.** Danach STM-Reset (gehört zum Vorgang, DIR-010) | Nutzer | 1.6 | Die gemeldete STM-Version entspricht 1.5 — weist das Skript selbst nach | — | — |
| 1.7 | **Smoketest und Ruhemessung.** `./tools/smoke-device.sh`; `./tools/measure-log.sh 600` | Lead | Flash 1 | Smoketest ohne Fehlschlag; vier Ruhefenster mit `l=`-Differenz ≥ **1'400'000** (AK1); `v=0/0` und keine `var retry:`-Zeile über 10 Minuten (AK2) | — | — |
| 1.8 | **▶ NACHWEIS N1 — die Nachsendung greift.** `./tools/watch-log.sh` **parallel** (DIR-013), ESP-Neustart durch den Nutzer, danach Abzug und `diff-snapshot.sh` gegen R0 | Lead + Nutzer | 1.7 | Mindestens eine Zeile `var retry: ok …` im **Mitschnitt** (AK3, führendes Mittel) und zusätzlich in `/api/stm32_log`; Zahl abweichender Felder **nicht grösser** als in R0 (AK5). Während des Fensters kein Watchdog-Reset, der nicht gemeldet wird | — | — |

### Ergebnis von N1 — hier eintragen, bevor Runde 2 beginnt

```
Datum/Uhrzeit:                 .bss F103:        .bss F411:
Ruhefenster (l je 10 s):      /      /      /
v= vor/nach:                   var retry: ok ... (Zahl):
aufgegeben (Zahl):             Liste voll (Zahl):
abweichende Felder N1:         gegen R0:
```

**Steht „var retry: ok" bei 0 und gleichzeitig `v=` über 0**, ist entweder die
Ausschlussliste zu weit (nur veraltende Kennungen liefen in den Timeout) oder der
Drain wird nicht erreicht. **Dann wird Runde 2 nicht gestartet**, sondern 1.2
nachgesehen. Das ist ein Abbruchkriterium, keine Formalie.

---

## Runde 2 — der ESP erkennt verfälschte Zeilen

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | G | R |
|---|---|---|---|---|---|---|
| 2.1 | **Referenzrechnung und Prüfvektoren.** `tools/checks/var-crc.c` nach Design 6.1; mindestens **sechs** Vektoren erzeugen, darunter `OT0002`, `OT000`, `OT002`, `OT000e`, ein `S`-Kommando mit 63 Zeichen und die leere Nutzlast. Vektoren unten eintragen | Lead | 1.8 | Das Programm läuft auf dem Mac und gibt die sechs Werte aus; sie stehen unten. **Erst danach** darf eine der beiden Umsetzungen entstehen — sonst gibt es zwei Wahrheiten und keinen Schiedsrichter | ☐ | ☐ |
| 2.2 | **Prüfung, Ablehnung, Fähigkeitsmeldung im ESP.** `var `-Zweig nach Design 6.2: Marke erkennen, prüfen, bei Fehlschlag **nicht anwenden** und `!v` senden; `var_form_error_cnt`; Zeile `- var abgewiesen len=<n>` über `Serial.println()` **und** `stm32_log_append()`; `CAP var-crc` in `setup()` **vor** jeder WLAN-Meldung | `esp-developer` | 2.1 | Die Umsetzung gibt für die sechs Vektoren aus 2.1 dieselben Werte (Prüfung durch Review gegen die Tabelle). `grep -c stm32_log_append` zeigt die neue Stelle — ohne sie bleibt die Messung über die API unsichtbar, das ist die Lehre aus C14/L185 | ☐ | ☐ |
| 2.3 | **ESP-Version anheben** (DIR-004) | `release-engineer` | 2.2 | `make esp-version-file` zeigt den neuen Stand | ☐ | ☐ |
| 2.4 | **Bauen, Release-ZIP, Rollout** | Lead | 2.3 | Release-ZIP liegt vor; ausdrücklich benannt: **ESP flashen, STM nicht, PWA nicht** | ☐ | ☐ |
| — | **▶ FLASH-RUNDE 2 — nur ESP (OTA).** `./tools/watch-log.sh` läuft **während** des OTA mit (DIR-013) | Nutzer | 2.4 | Das OTA-Fenster ist der **Nachweis für AK4**: Während der ESP schreibt, ist die Brücke tot. Danach muss die Uhr weiterlaufen oder einen Watchdog-Reset gezeigt haben — sie darf nicht stehenbleiben. `Liste voll` höchstens einmal je 60 s | — | — |
| 2.5 | **Nachweis — inert und Fähigkeitsmeldung.** `./tools/install-app.sh --check` (die PWA ändert sich nicht, aber der Blick hat am 02.10.2026 gefehlt); Smoketest; Abzugsvergleich | Lead | Flash 2 | `CAP var-crc` steht im Mitschnitt des ESP-Starts (**AK9**, deterministisch); Smoketest ohne Fehlschlag und Abzug unverändert (**AK8**); `var_form_error_cnt` = 0 | — | — |
| 2.6 | **Tischprüfung statt Gerätenachweis.** Die sechs Vektoren gegen beide Umsetzungen; Ergebnis unten | Lead | 2.5 | **AK10 ist erfüllt, wenn beide Umsetzungen dieselben sechs Werte liefern.** Dass die Ablehnung am Gerät feuert, ist mit den heutigen Mitteln **nicht** erzwingbar — der einzige Sender von `var`-Zeilen ist der STM. Das steht hier, damit niemand später eine Messung sucht, die es nicht geben kann (L177) | — | — |

### Prüfvektoren — hier eintragen (Task 2.1)

```
Nutzlast                                   len    crc
OT0002
OT000
OT002
OT000e
S03<63 Zeichen>
<leer>
```

---

## Runde 3 — der STM hängt die Prüfsumme an

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | G | R |
|---|---|---|---|---|---|---|
| 3.1 | **Prüfsumme anhängen und `!v` auswerten.** Anhängen **nur** bei gesetztem `cap_var_crc` (Design 6.4); Trennung `got_answer` / `applied` (6.3); bei `!v` **nicht erneut warten**, sondern sofort vormerken mit `due = uptime`; die Prüfsumme geht direkt auf die UART, **kein** grösserer Puffer | `stm-developer` | 2.6 | Die Umsetzung liefert für die sechs Vektoren aus 2.1 dieselben Werte (Review gegen die Tabelle). `grep` zeigt keinen neuen Puffer > 72 Byte in `var_send_buf()` | ☐ | ☐ |
| 3.2 | **`smoke-device.sh`: Rückroll-Falle.** Meldet einen Zeichenkettenwert, der auf `*` + vier Hexziffern endet, als Fehlschlag | Lead | 2.6 (**darf parallel zu 3.1 laufen**, anderer Besitzer) | **Gegenprobe gefahren**: mit einem künstlich präparierten Wert schlägt die Prüfung an, ohne ihn nicht (AK13). Ohne Gegenprobe ist die Prüfung unbelegt | ☐ | ☐ |
| 3.3 | **STM-Version anheben** (DIR-004, zweite Erhöhung) | `release-engineer` | 3.1, 3.2 | `make stm-version-file` zeigt den neuen Stand | ☐ | ☐ |
| 3.4 | **Bauen, Speicher messen, Release-ZIP, Rollout** | Lead | 3.3 | `.bss` gegen Runde 1 unverändert (die Prüfsumme kostet keinen Speicher); ausdrücklich benannt: **STM flashen, ESP nicht, PWA nicht** | ☐ | ☐ |
| — | **▶ FLASH-RUNDE 3 — nur STM**, danach Reset | Nutzer | 3.4 | Gemeldete STM-Version entspricht 3.3 | — | — |
| 3.5 | **Nachweis — die Prüfsumme steht auf der Leitung.** `./tools/watch-log.sh`, `./tools/measure-log.sh 600`, Smoketest | Lead | Flash 3 | `var`-Zeilen im Mitschnitt enden auf `*<4 Hexziffern>` (**AK11**, direkt ablesbar); vier Ruhefenster ≥ 1'400'000 (**AK14**); Smoketest ohne Fehlschlag, einschliesslich der neuen Prüfung aus 3.2 | — | — |
| 3.6 | **▶ NACHWEIS N3 — kein verfälschter Wert mehr.** **Dreimal** ESP-Neustart durch den Nutzer, jedes Mal mit Mitschnitt und Abzug | Lead + Nutzer | 3.5 | In keinem der drei Abzüge liegt ein Wert ausserhalb seines Bereichs; ausdrücklich geprüft: `overlay[*].type` ∈ 0..10 und `HARDWARE_CONFIGURATION ≠ 65535` (**AK12**). **Im Protokoll festhalten:** Drei saubere Durchgänge belegen keine Zustellgarantie | — | — |

### Ergebnis von N3 — hier eintragen

```
Durchgang 1:  v=        abweichende Felder:        ausserhalb Bereich:        !v:
Durchgang 2:  v=        abweichende Felder:        ausserhalb Bereich:        !v:
Durchgang 3:  v=        abweichende Felder:        ausserhalb Bereich:        !v:
Ruhefenster (l je 10 s):      /      /      /
```

---

## Abschluss

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | G | R |
|---|---|---|---|---|---|---|
| 4.1 | **Befundkatalog und Changelog.** L230/A32 auf den erreichten Stand; **L205 mit der Herleitung aus Design 1.2** (Burst-Verlust und `htoi` über das Zeilenende); L42/L103 um die Einordnung „Empfängerzustand, nicht Übertragung"; **A6/Runde 4a: bewertet, bleibt, Reihenfolge A32 → 4a** (Design 5); drei neue Befunde anlegen: unquittierte Gegenrichtung, nicht zuordenbare Quittung, verschachtelter Sendefall ohne Nachsendung | `doc-writer` | 3.6 | `./tools/guardrails.sh` S10 läuft durch — jeder offene Befund steht in der ToDo-Liste. Keine Versionsnummer in einem lebenden Dokument (S9) | ☐ | ☐ |

---

## Die erlaubten Parallelitäten — abschliessend

| Zusammen erlaubt | Warum |
|---|---|
| 1.4 (Lead, `tools/**`) neben 1.1–1.3 (`stm-developer`, `src/**`) | Verschiedene Besitzer, disjunkte Dateien. 1.4 muss **vor** 1.7 fertig sein, sonst fehlt dort die Auswertung |
| 3.2 (Lead, `tools/**`) neben 3.1 (`stm-developer`, `src/**`) | Dasselbe |

**Alles andere läuft seriell.** Insbesondere laufen nie zwei Tasks des
`stm-developer` gleichzeitig, auch nicht bei disjunkten Dateien — der Hook
`file-ownership.py` liesse es zu, R3 nicht. Und über eine Flash- oder Messzeile
hinweg wird **nichts** parallelisiert (R3b).

---

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um. Keine stillen Mitkorrekturen —
   der Abschnitt „Nicht Teil dieser Änderung" in `requirements.md` ist die Grenze.
   Besonders `A20/L115` und `A1/L85` liegen direkt daneben und bleiben liegen.
2. **Umschrift in `src/**` und in `ESP-uclock.ino`.** Diese Dateien sind ASCII
   beziehungsweise ISO-8859-1: `Bruecke`, `waehrend`, `zurueck`, `Pruefsumme`. **Vor
   dem Patchen die Datei prüfen** — `http.cpp` und `stm32flash.cpp` sind UTF-8, ein
   `latin-1`-Patcher beschädigt sie.
3. **Kein `make`.** Teammates bauen nicht (R1); der Hook `no-build.py` weist es ab.
   Der Lead baut einmal je Runde.
4. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings.
5. Review ausdrücklich gegen `knowledge/architecture-checklist.md`, und zwar gegen
   die **beantworteten** Punkte in `design.md`, Abschnitt 8.
6. Erst dann nächster Task oder Schreibübergabe.

## Was der Lead nicht tut

Der Lead schreibt in diesem Paket **keine Zeile unter `src/**` und keine unter
`ESP8266/ESP-uclock/`** — auch nicht die eine offensichtliche. Genau diese Abkürzung
hat am 03.10.2026 R3 gebrochen.

## Abschluss-Prüfliste

- [ ] Alle Tasks erledigt
- [ ] R0, N1, die Prüfvektoren und N3 sind **eingetragen**, nicht nur gelaufen
- [ ] `./tools/guardrails.sh --full` mit Exit 0 (schliesst Compile-Smoke-Tests ein)
- [ ] `./tools/smoke-device.sh` ohne Fehlschlag
- [ ] Alle 14 Akzeptanzkriterien erfüllt **oder ausdrücklich und begründet abgewählt**
- [ ] Drei Release-ZIPs, drei Rollouts, **drei Commits und drei Tags** (DIR-011) —
      nicht gesammelt, nicht auf Nachfrage
- [ ] Jedes Mal ausdrücklich benannt, **was zu flashen ist**
- [ ] `./tools/install-app.sh --check` gelaufen, obwohl sich die PWA nicht ändert
