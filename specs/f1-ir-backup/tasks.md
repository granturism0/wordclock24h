# Tasks — F1, IR-Codes ins Backup

Einzelschritte mit Abhängigkeiten. **Pro Task genau ein schreibender Agent** (R3, R4).
Nach jedem Task läuft `./tools/guardrails.sh`; erst bei Exit 0 geht die
Schreibberechtigung weiter.

Die Besitzkarte ist per Hook erzwungen (`tools/hooks/file-ownership.py`, geprüft auf
`Write`, `Edit` **und `Bash`**). Kein Task greift über zwei Besitzer — das liesse sich
gar nicht umsetzen.

| # | Task | Agent | Hängt ab von | Guardrails | Review |
|---|---|---|---|---|---|
| 1 | STM: Zugriff auf die IR-Codes — `remote_ir_get_code()`, `remote_ir_set_code()` in `src/remote-ir/remote-ir.c` und `.h` | `stm-developer` | — | ☐ | ☐ |
| 2 | STM: RPC-Nummer, `var_send_ir_code()`, Export-Zähler im Hauptloop, `case 'I'` in `schedule_esp8266_cmd()` — `src/vars/vars.h`, `src/vars/vars.c`, `src/main.c` | `stm-developer` | 1 | ☐ | ☐ |
| 3 | ESP: Puffer, Maske, `case 'I'` in `var_set_parameter()`, RPC-Nummer — `vars.h`, `vars.cpp` | `esp-developer` | 2 | ☐ | ☐ |
| 4 | ESP: drei Endpunkte `ir_codes_request`, `ir_codes_get`, `ir_code_set` samt Dispatcher-Zweigen — `http.cpp` | `esp-developer` | 3 | ☐ | ☐ |
| 5 | **Lead:** Versionen anheben, bauen, STM und ESP flashen, Round-Trip am Gerät nachweisen | Lead / `release-engineer` | 4 | ☐ | ☐ |
| 6 | PWA: Namensliste, Abzugsfunktion mit Vollständigkeitsprüfung, Abschnitt `settings.ir` im Export, `BACKUP_VERSION` auf 3 — `app.js` | `pwa-developer` | 5 | ☐ | ☐ |
| 7 | PWA: Importstufe mit Bestätigung, Rückfalldatei und Gegenprobe, i18n-Schlüssel in beiden Sprachtabellen — `app.js` | `pwa-developer` | 6 | ☐ | ☐ |
| 8 | **Lead:** App-Version und `CACHE_NAME` anheben, `app-gz`, Release-ZIP, Rollout, `install-app.sh` | Lead / `release-engineer` | 7 | ☐ | ☐ |
| 9 | Doku: `BEFUNDE.md` (F1, L35, L84), `TESTPLAN-PWA.md` (neue Phase), Gefahrenliste um `/api/ir_code_set` | `doc-writer` | 8 | ☐ | ☐ |
| 10 | **Lead:** `tools/hooks/no-danger.py` um `/api/ir_code_set` ergänzen | Lead | 4 | ☐ | ☐ |

## Warum diese Reihenfolge

**1 vor 2.** Task 2 ruft die Funktionen aus Task 1 auf. Getrennt, weil Task 1 für sich
abgeschlossen und reviewbar ist: ein neues Modulinterface, zwei Funktionen, eine
Indexprüfung. Task 2 ist dagegen ein Eingriff in `main.c`, die Datei mit dem höchsten
Risiko — sie soll nicht zusammen mit etwas anderem geprüft werden.

**2 vor 3 und 4.** Die STM-Seite definiert das Protokoll. Der ESP spiegelt es. Ist die
Reihenfolge umgekehrt, baut der `esp-developer` gegen eine Vermutung.

**5 ist der erste echte Prüfpunkt, und er kommt vor der PWA.** Nach Task 4 sind Export
und Restore **vollständig mit HTTP-Abfragen nachweisbar** — `curl` gegen die drei neuen
Endpunkte, der Ablauf aus `design.md`, Abschnitt „Prüfbarkeit ohne Fernbedienung".
Keine Zeile `app.js` ist dafür nötig. Das ist der Grund, diese Stufe eigens zu
schneiden: Wäre die PWA schon gebaut, liesse sich bei einem Fehlschlag nicht mehr
sagen, ob die Firmware oder die App schuld ist.

**6 vor 7.** Beide gehören `pwa-developer` und derselben Datei, also ohnehin
sequenziell. Getrennt, weil Task 6 nur **liest und exportiert** — ungefährlich — und
Task 7 den destruktiven Teil baut. Ein Review, das beides zugleich prüft, prüft keines
von beiden richtig.

**9 am Schluss.** `BEFUNDE.md` führt den Stand, nicht die Absicht. Ein Befund wird auf
„erledigt" gesetzt, wenn er erledigt **und nachgewiesen** ist, nicht wenn der Code
geschrieben ist.

**10 kann parallel zu 5 bis 9 laufen.** Es berührt nur `tools/**` und damit keine
Datei, an der sonst jemand arbeitet. Es sollte aber vor Task 7 fertig sein, damit der
Hook greift, sobald die PWA den Endpunkt tatsächlich ansprechen kann.

## Was in welchem Task ausdrücklich NICHT passiert

| Task | Nicht darin |
|---|---|
| 1 | Kein Anfassen von `remote_ir_learn()`, `remote_ir_read_codes_from_eep()`, `remote_ir_write_codes_to_eep()` |
| 2 | **Kein** Aufruf in `var_send_all_variables()` (AK2, Befund L85). Kein `snprintf`-Umbau von `var_send_string()` — das ist L86 und ein eigener Auftrag |
| 3 | Kein Ablegen des Puffers im ESP-EEPROM. Kein Mitführen aus `ir_code_set` |
| 4 | Kein `atoi` ohne Prüfung. Keine Vorgabewerte für fehlende Parameter — die Lektion aus L70 |
| 6 | Kein Umdeuten, Umbenennen oder Entfernen eines bestehenden Backup-Feldes (AK18) |
| 7 | Kein Löschen von IR-Codes. `protocol: null` wird übersprungen, nicht geschrieben |
| 9 | `REVIEW.md` und `REVIEW-2026-09-29.md` bleiben unverändert — Momentaufnahmen (DIR-006) |

## Prüfpunkte je Task

| # | Wie geprüft wird |
|---|---|
| 1 | Compile-Smoke über `./tools/guardrails.sh --full`. Code-Review: `idx >= N_REMOTE_IR_CMDS` wird in **beiden** Funktionen abgewiesen; `remote_ir_set_code()` schreibt RAM **und** EEPROM |
| 2 | Compile-Smoke. Code-Review gegen AK2 und AK3: kein `I`-Erzeuger in `var_send_all_variables()`, und der Sendezustand wird im Hauptloop getaktet, nicht in einer Schleife. `grep` auf `GET_IR_CODES_RPC_VAR` zeigt die neue Nummer unmittelbar vor `MAX_RPC_VARIABLES` |
| 3 | Compile-Smoke. Code-Review: Enum-Reihenfolge identisch zu `src/vars/vars.h`; `MAX_IR_CODES` trägt den Quellenverweis auf `remote-ir.h:69` |
| 4 | Compile-Smoke. Code-Review gegen die Prüftabelle in `design.md` |
| 5 | **Am Gerät**, Ablauf aus `design.md`: Referenzabzug, fünf Negativprüfungen, synthetischer Schreibwert, Abzug, STM-Reset, Abzug, Rückschreiben, Abzug. Schritte ab 4 nur mit ausdrücklicher Freigabe des Nutzers (R5). Protokoll gehört in die Übergabe |
| 6 | Sicherung anlegen, Datei öffnen: `version` ist 3, `settings.ir.keys` hat genau 20 Einträge, Namen stimmen mit `remote-ir.h:38-67` überein. Gegenprobe: Abzug künstlich scheitern lassen (ESP-Neustart während des Pollens) — der Abschnitt fehlt dann **ganz**, und der Hinweis sagt es |
| 7 | Import einer Datei mit allen 20 Tasten; die Rückfalldatei wird heruntergeladen; beide Rückfragen erscheinen; die Gegenprobe meldet 0 Abweichungen. Import einer Datei mit nur 19 Einträgen → Stufe entfällt mit Hinweis. Import einer Version-2-Datei → läuft durch, IR-Stufe entfällt stumm |
| 8 | `./tools/install-app.sh --check` nennt keine fehlende Datei. `./tools/smoke-device.sh` läuft durch |
| 9 | Guardrail S10: jeder offene Befund steht in der ToDo-Liste. Guardrail S9: keine Versionsnummer in einem lebenden Dokument |
| 10 | Der Hook weist einen Testaufruf auf `/api/ir_code_set` ab |

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings
3. Review durch den zuständigen Review-Agenten, ausdrücklich gegen
   `knowledge/architecture-checklist.md`
4. Erst dann nächster Task oder Schreibübergabe

Teammates führen **niemals** `make` aus (R1). Der `pwa-developer` meldet „fertig", der
Lead baut. `make app-gz` erst bei stillem Arbeitsverzeichnis (R2) — eine `.gz` einer
halb geschriebenen `app.js` ist der White-Screen-Fehlerfall.

## Abschluss

- [ ] Alle Tasks erledigt
- [ ] `./tools/guardrails.sh --full` mit Exit 0 (schliesst Compile-Smoke-Tests ein)
- [ ] Alle 20 Akzeptanzkriterien aus `requirements.md` erfüllt
- [ ] Round-Trip-Nachweis aus Task 5 protokolliert, einschliesslich der fünf
      Negativprüfungen
- [ ] Release-ZIP gebaut, Flash-Umfang benannt: **STM32 und ESP neu flashen, PWA per
      `install-app.sh` hochladen** — alle drei, in dieser Reihenfolge
- [ ] Rollout auf die Synology (DIR-005), Ziel `/volume1/web/wordclock/test8`, **ohne**
      `--delete`
- [ ] `./tools/smoke-device.sh` nach dem Flashen (DIR-009), Update-Quelle geprüft
- [ ] **Offener Restpunkt ausdrücklich übergeben:** Dass die Uhr auf die
      wiederhergestellten Codes auch reagiert, ist nur mit der Fernbedienung in der
      Hand prüfbar und bleibt beim Nutzer. Nicht stillschweigend als erledigt führen

## Freigabe

Diese Spec ist ein Entwurf. **Ohne Freigabe des Nutzers beginnt keine Implementierung.**
Zwei Punkte gehören vor der Freigabe ausdrücklich entschieden, weil sie Geschmack und
nicht Technik sind:

1. Soll die Rückfalldatei **automatisch** heruntergeladen werden (so entworfen) oder
   erst nach einer eigenen Nachfrage? Automatisch ist aufdringlich, aber es ist der
   einzige Rückweg, und wer ihn an eine weitere Entscheidung knüpft, verliert ihn im
   Zweifel.
2. Soll der IR-Abschnitt beim Import **vorbelegt übernommen** werden (so entworfen,
   weil der Hauptzweck der Restore nach einem EEPROM-Reset ist) oder vorbelegt
   übersprungen?
