# Tasks — Grosses Befundpaket

**Momentaufnahme vom 2026-10-04** (DIR-006). Gehört zu `requirements.md` und
`design.md` in diesem Verzeichnis.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

---

## Drei Regeln, die über der Tabelle stehen

**1 — Die Abhängigkeitsspalte ist die Reihenfolge, nicht die Besitzspalte (R3b).**
Zwei Tasks mit disjunkten Dateien dürfen trotzdem nicht gleichzeitig laufen, wenn
zwischen ihnen eine Abhängigkeit steht. Am 03.10.2026 sind Task 2 und Task 3 eines
Pakets parallel gestartet worden, weil die Dateien verschieden waren — dazwischen
stand eine Flash-Runde, die eine Änderung **isoliert** nachweisen sollte. Diese
Zusage war danach nicht mehr einlösbar. In diesem Paket betrifft das vor allem die
Grenzen zwischen den Runden und die Trennung von 4a und 4b.

**2 — Nur der Lead baut, nur der `release-engineer` bumpt (R1, R4).** Kein
Teammate führt `make` aus. Vier Versionsstellen müssen zueinander passen; zwei
unabhängige Erhöhungen machen das Release inkonsistent.

**3 — Während eines Gerätelaufs hat niemand sonst Zugriff (R5, L190).** Wer einen
Geräte-Agenten startet, fasst das Gerät bis zu dessen Rückmeldung selbst nicht an.
Am 04.10.2026 hat der Lead während eines laufenden Durchlaufs dreimal selbst am
Gerät gearbeitet; der Durchlauf war danach dreimal zerschnitten, alle ESP-Zähler
sprangen auf 0, und eine Messung liess sich nicht mehr sicher zuordnen. **Gemerkt hat
es der Tester, nicht der Lead.**

---

## Runde 0 — Beobachtbarkeit und Prüfmethode

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | Guardrails | Review |
|---|---|---|---|---|---|---|
| 0.1 | `esp_heap_log()` baut die Zeile per `snprintf` in `char line[48]`, gibt sie per `Serial.println()` aus **und** übergibt sie `stm32_log_append()` | `esp-developer` | — | Die Zeile wird genau **einmal** formatiert; `grep -a` über `src/**` findet keine STM-Logzeile, die mit `- ` beginnt (sonst Präfix `esp: `, siehe `design.md`); kein `String` im neuen Code | ☐ | ☐ |
| 0.2 | Verlustzeile in `http.cpp` ebenso, innerhalb des bestehenden `if (! http_write_broken)`-Zweigs | `esp-developer` | 0.1 | Der Zweig erzeugt weiterhin **genau eine** Zeile je Verbindung; `base.h` ist bereits eingebunden, kein neuer Include | ☐ | ☐ |
| 0.3 | `TESTPLAN-PWA.md`: S47 auf die belegte Wirkung berichtigen (Farbe **danach** verloren, Vermerk „Ausgangswert vorher notieren" wie S51/S53), S114 um den ESP-Mitstart und die Ungültigkeit von Messreihen ergänzen | `doc-writer` | — (läuft parallel zu 0.1/0.2, andere Datei, kein Gerätetest dazwischen) | `grep` findet in der S47-Zeile den Vermerk und in der S114-Zeile die Wörter „ESP-Zähler" und „Messreihen" | ☐ | ☐ |
| 0.4 | `TESTPLAN-PWA.md`: neuer Abschnitt „Was eine Setter-Gegenprobe nicht zeigt" mit den drei Verfahren V1 bis V3 aus `design.md`; V1 als Pflicht je Setter-Phase | `doc-writer` | 0.3 | Der Abschnitt nennt alle drei Verfahren **mit ihrem Preis**; V3 trägt den Hinweis „höchstens einmal je Durchlauf, am Ende, mit Freigabe" | ☐ | ☐ |
| 0.5 | `.claude/agents/pwa-tester.md`: Rahmenmessung V1 als Pflicht aufnehmen — `d=` vor und nach jeder Setter-Phase, Anstieg macht die Phase ungültig | Lead | 0.4 | Die Agentenanweisung nennt dieselbe Regel wie der Testplan, ohne sie neu zu formulieren | ☐ | ☐ |
| 0.6 | ESP-Version anheben | `release-engineer` | 0.2 | `./tools/guardrails.sh` Stufe S4 zeigt den neuen Stand | ☐ | ☐ |
| 0.7 | Vollständiger Build, Release-ZIP, Rollout auf die Synology; **fertige Einspielzeile an den Nutzer** | Lead | 0.6 | `guardrails.sh --full` Exit 0; `deploy.sh` gelaufen, **ohne** `--delete`; Commit und Tag `release/<stm>-<esp>-<app>` sofort (DIR-011) | ☐ | ☐ |
| 0.8 | ESP einspielen | **Nutzer** | 0.7 | `/api/update_status` meldet die neue ESP-Version | — | — |
| 0.9 | Gerätelauf Runde 0, mit mitlaufendem `./tools/watch-log.sh` (DIR-013) | `pwa-tester` | 0.8 | **AK0.1 bis AK0.4** erfüllt; `d=` am Anfang und am Ende notiert (Beobachtungsauftrag A28) | — | ☐ |
| 0.10 | Probedurchlauf einer Setter-Phase nach der neuen Methode | `pwa-tester` | 0.9, 0.5 | **AK0.7**: Das Protokoll enthält den `d=`-Wert vor und nach der Phase | — | ☐ |
| 0.11 | `BEFUNDE.md` nachführen: C14 und E16 auf erledigt mit Beleg; die acht veralteten B1-Zeilen (B1b, B1c, B1d, B1e, B1g, B1j, B1k, B1l) aus der Arbeitsliste streichen | `doc-writer` | 0.10 | Guardrail S10 läuft durch; keine der acht Zeilen steht noch in der Arbeitsliste, und keine davon ist in den Statustabellen offen | ☐ | ☐ |

**Nach 0.8 zwingend:** `./tools/install-app.sh --check` und `./tools/smoke-device.sh`.
Ein Firmware-Wechsel löscht das Dateisystem nicht, aber die Update-Quelle kann leer
sein (L42) — und dann holt das nächste Update fremde Firmware.

---

## Runde 1 — ESP-Parametervertrag

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | Guardrails | Review |
|---|---|---|---|---|---|---|
| 1.1 | Bestandsaufnahme: jede Stelle in `http.cpp` listen, die einen Zahlenwert **still klemmt** und danach `{"ok":true}` meldet. Rein lesend | `firmware-analyst` | 0.11 | Eine Liste mit Endpunkt, Parameter, Grenzen und Fundstelle. Sie ist der **verbindliche Umfang** von 1.2 — kein Endpunkt mehr, keiner weniger | ☐ | ☐ |
| 1.2 | Die Liste aus 1.1 auf Abweisen umstellen; `detail` in der Form `<param> out of range (<min>..<max>)` | `esp-developer` | 1.1 | Jeder Endpunkt der Liste antwortet im Quelltext über `http_json_error(HTTP_API_ERROR_OUT_OF_RANGE, …)`; kein `if (x > max) x = max;` bleibt in der Liste übrig | ☐ | ☐ |
| 1.3 | `fs_show`: Existenzprüfung **vor** die Kopfzeilen, drei Fälle, neuer Fehlercode 6; ein `LittleFS.begin()`, ein `end()` auf jedem Pfad | `esp-developer` | 1.2 | Der Quelltext zeigt genau ein `begin()`/`end()`-Paar und drei getrennte Antwortformen; `http_fs_file_exists_and_nonempty()` wird hier **nicht** benutzt | ☐ | ☐ |
| 1.4 | Gegenprüfung: sendet PWA oder Legacy heute irgendwo einen Wert, den der neue Vertrag abweist? Rein lesend | `code-reviewer` | 1.2 | Eine Aussage je betroffenem Endpunkt, mit Fundstelle. Findet sich eine Stelle, wird sie **vor** dem Rollout behoben — sonst bricht sie beim Nutzer | ☐ | ☐ |
| 1.5 | PWA: `fs_show`-Antwort über den `Content-Type` auswerten, Fehlercode 6 kennen, „Datei fehlt" und „Datei leer" verschieden melden | `pwa-developer` | 1.3 | Im Code steht keine Prüfung auf die Zeichenfolge `{"ok":false` am Rumpfanfang | ☐ | ☐ |
| 1.6 | ESP- und App-Version anheben, `CACHE_NAME` mit | `release-engineer` | 1.4, 1.5 | S4 zeigt den neuen Stand; `APP_VERSION` und `CACHE_NAME` sind zusammen gestiegen | ☐ | ☐ |
| 1.7 | Build, Release-ZIP, Rollout, Einspielzeilen (ESP **und** `install-app.sh`) | Lead | 1.6 | `guardrails.sh --full` Exit 0; Commit und Tag sofort | ☐ | ☐ |
| 1.8 | ESP und PWA einspielen | **Nutzer** | 1.7 | `install-app.sh --check` meldet alle Dateien mit Grösse > 0 | — | — |
| 1.9 | Gerätelauf Runde 1, `watch-log.sh` mit | `pwa-tester` | 1.8 | **AK1.1 bis AK1.6** erfüllt. Setter-Phasen nach V1 gerahmt | — | ☐ |
| 1.10 | `BEFUNDE.md` nachführen | `doc-writer` | 1.9 | S10 läuft durch | ☐ | ☐ |

**AK1.1 und AK1.2 sind schreibend** und brauchen die Freigabe des Nutzers im selben
Gespräch (R5). Die fertigen Zeilen stehen unten.

---

## Runde 2 — PWA-Eingabefelder

Alle Logiktasks dieser Runde schreiben in **dieselbe** Datei. Sie laufen deshalb
streng nacheinander, auch wenn sie inhaltlich unabhängig sind (R3). Zwei Schreiber
in dieser Datei überschreiben sich, und das fällt erst auf, wenn der Stand schon
kaputt ist.

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | Guardrails | Review |
|---|---|---|---|---|---|---|
| 2.1 | **B1 + B18:** Speicherpfade ausserhalb des `runButtonRequest`-Trichters setzen `hasUnsavedEdits` zurück; Modulwechsel warnt bei offenen Änderungen | `pwa-developer` | 1.10 | **AK2.1** und **AK2.9**. Reine Auslöseaktionen lassen das Flag weiterhin stehen — nachgewiesen an „Wetter abrufen" | ☐ | ☐ |
| 2.2 | **B2:** `visibilitychange` mit `else`-Zweig, beim Zurückkehren einmal sofort laden | `pwa-developer` | 2.1 | **AK2.2** | ☐ | ☐ |
| 2.3 | **B3 + B4:** `apiFetch` im Flash-Pfad, `finishProgressUi(2200)` | `pwa-developer` | 2.2 | **AK2.3**; im Flash-Pfad steht kein rohes `fetch` mehr | ☐ | ☐ |
| 2.4 | **B6 + B1i:** Alle Zahlenfelder über `readNumberInputOrReport()`, Meldung mit **beiden** Zahlen; die fünf Overlay-Felder mit | `pwa-developer` | 2.3 | **AK2.5** und **AK2.7** | ☐ | ☐ |
| 2.5 | **B5 + B7:** Hinweis bei `isSecureContext === false`; `file.size > 0` vor dem Hochladen, Längenprüfung im Service Worker | `pwa-developer` | 2.4 | **AK2.4** und **AK2.6** | ☐ | ☐ |
| 2.6 | **B17:** Fehler aus `fetchStm32Log(true)` auswerten, `stm32LogRefreshInFlight` im `finally` zurücksetzen, leerer Ring bekommt einen Text | `pwa-developer` | 2.5 | **AK2.8**: In **keinem** der drei Fälle bleibt das Feld leer und stumm | ☐ | ☐ |
| 2.7 | **B11 + B12:** Toten `power_status`-Getter entfernen oder benutzen; für die vier nur per Import erreichbaren Setter Bedienelemente oder eine Begründung im Code | `pwa-developer` | 2.6 | Für beide Punkte steht eine **Entscheidung mit Begründung** im Code oder in `BEFUNDE.md`. „Offen gelassen" ist keine | ☐ | ☐ |
| 2.8 | App-Version und `CACHE_NAME` anheben | `release-engineer` | 2.7 | beide zusammen gestiegen | ☐ | ☐ |
| 2.9 | Build, Release-ZIP, Rollout, Einspielzeile | Lead | 2.8 | `guardrails.sh --full` Exit 0; **`git status` vor `app-gz` sauber** (R2) — eine `.gz` einer halb geschriebenen Datei ist der Weisschirm-Fehlerfall; Commit und Tag sofort | ☐ | ☐ |
| 2.10 | PWA einspielen | **Nutzer** | 2.9 | `install-app.sh --check` ohne Fehlmeldung | — | — |
| 2.11 | Gerätelauf Runde 2 | `pwa-tester` | 2.10 | **AK2.1 bis AK2.10**; `./tools/check-pwa.sh` meldet **keinen Fehler beim Laden von `app.js`** — das sieht weder API noch Smoketest noch Screenshot | — | ☐ |
| 2.12 | `BEFUNDE.md` nachführen | `doc-writer` | 2.11 | S10 läuft durch | ☐ | ☐ |

---

## Runde 3 — UI-Kacheln

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | Guardrails | Review |
|---|---|---|---|---|---|---|
| 3.1 | Vorprüfung: Hängt ausser `app.js` noch etwas an der DOM-Reihenfolge der Kacheln? Service Worker und Backup-Import-Dialog mitprüfen. Rein lesend | `code-reviewer` | 2.12 | Eine Aussage je Fundstelle. Findet sich eine Positionsabhängigkeit, wird 3.2 **vorher** angepasst | ☐ | ☐ |
| 3.2 | Variante 2: Markup in die Reihenfolge `source, remote, local, backup, service, files`; alle drei `grid-template-areas`-Blöcke und die sechs `grid-area`-Zuweisungen entfernen | `ui-developer` | 3.1 | **AK3.2** und **AK3.3** | ☐ | ☐ |
| 3.3 | Vermessung in der Vorschau, Modul `maintenance`, bei 900 / 1240 / 1512 px | Lead | 3.2 | **AK3.1**: grösste Höhendifferenz nebeneinanderstehender Kacheln ≤ 400 px (heute 900) | — | ☐ |
| 3.4 | **B14 (L120):** Kontrast des Hakens messen und entscheiden | `ui-reviewer` | 3.2 | **AK3.5**: gemessener Wert ≥ 3:1 (WCAG 1.4.11), mit oder ohne `accent-color`. Eine Änderung **ohne** Messung gilt als nicht erfüllt | — | ☐ |
| 3.5 | Umsetzung der Entscheidung aus 3.4, falls sie eine Änderung verlangt | `ui-developer` | 3.4 | Der gemessene Wert aus 3.4 gilt auch nach der Änderung | ☐ | ☐ |
| 3.6 | **Gegenprobe B1f/B1h** — keine Umsetzung | `ui-reviewer` | 3.2 | **AK3.6**. Weicht etwas ab, entsteht ein **neuer Befund** in `BEFUNDE.md`, kein stiller Nachbau | — | ☐ |
| 3.7 | App-Version und `CACHE_NAME` anheben | `release-engineer` | 3.3, 3.5, 3.6 | beide zusammen gestiegen | ☐ | ☐ |
| 3.8 | Build, Release-ZIP, Rollout, Einspielzeile | Lead | 3.7 | `guardrails.sh --full` Exit 0; `git status` vor `app-gz` sauber; Commit und Tag sofort | ☐ | ☐ |
| 3.9 | PWA einspielen | **Nutzer** | 3.8 | `install-app.sh --check` ohne Fehlmeldung | — | — |
| 3.10 | Gerätelauf Runde 3 | `pwa-tester` | 3.9 | **AK3.4**: `check-pwa.sh` ohne Fehler beim Laden von `app.js`; alle Schaltflächen des Moduls erreichbar | — | ☐ |
| 3.11 | `BEFUNDE.md` nachführen | `doc-writer` | 3.10 | S10 läuft durch | ☐ | ☐ |

---

## Runde 4a — A6, Selbstheilung des Variablensatzes

**Eigene Flash-Runde.** Sie wird von 4b getrennt, damit ein gescheiterter OTA
eindeutig zuzuordnen ist (C13, Risiko 1) und damit die Selbstheilung **isoliert**
nachweisbar bleibt.

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | Guardrails | Review |
|---|---|---|---|---|---|---|
| 4a.1 | Messung am Gerät: ein ESP-Neustart mit mitlaufendem Mitschnitt. Gezählt werden die abgesetzten Kommandos, die Quittungs-Timeouts und der `d=`-Zuwachs | `pwa-tester` (lesend) + Freigabe | 3.11 | **AK4.1**: drei Zahlen, nicht drei Eindrücke. Vorher-/Nachher-Rohabzug liegt bei | — | ☐ |
| 4a.2 | Entscheidung Weg A oder Weg B, schriftlich mit Begründung aus 4a.1 | Lead, Freigabe durch den **Nutzer** | 4a.1 | Die Entscheidung steht in `BEFUNDE.md` mit Bezug auf die Messung. Weg A hat eine **sichtbare Nebenwirkung** (IP-Lauftext je Versuch) und braucht dafür ausdrücklich seine Zustimmung | — | — |
| 4a.3 | Weg A: Der ESP prüft nach dem Hochlauf auf `HARDWARE_CONFIGURATION` = 65535 und sendet `IPADDRESS` erneut — höchstens dreimal, je rund 10 s Abstand, jeder Versuch mit einer Zeile in den Ring | `esp-developer` | 4a.2 | Der Versuchszähler wird beim Erfolg zurückgesetzt; nach dem letzten Fehlversuch steht **eine** deutliche Meldung, kein stilles Weiterlaufen. `var_send_all_variables()` ist **nicht** angefasst | ☐ | ☐ |
| 4a.4 | Weg B (nur falls 4a.2 so entscheidet): stilles Kommando für den Vollabgleich | `esp-developer`, dann `stm-developer` | 4a.2 | Zwei getrennte Tasks mit einer **Flash-Runde dazwischen** — ESP zuerst, STM danach | ☐ | ☐ |
| 4a.5 | ESP-Version anheben | `release-engineer` | 4a.3 | S4 zeigt den neuen Stand | ☐ | ☐ |
| 4a.6 | Build, Release-ZIP, Rollout, Einspielzeile; freien Heap und grössten Block **vor** dem OTA notieren | Lead | 4a.5 | `guardrails.sh --full` Exit 0; die beiden Heapwerte stehen im Protokoll; Commit und Tag sofort | ☐ | ☐ |
| 4a.7 | ESP einspielen | **Nutzer** | 4a.6 | neue ESP-Version in `/api/update_status` | — | — |
| 4a.8 | Gerätelauf: dreimal ESP-Neustart, jedes Mal Variablensatz prüfen | `pwa-tester` | 4a.7 | **AK4.2**, **AK4.3**, **AK4.4**. Die STM-seitige Gegenprobe läuft nach V1, am Ende einmal V3 | — | ☐ |
| 4a.9 | `BEFUNDE.md` nachführen | `doc-writer` | 4a.8 | S10 läuft durch | ☐ | ☐ |

---

## Runde 4b — Flash-Überwachung

| # | Task | Agent | Hängt ab von | Abnahme (nachprüfbar) | Guardrails | Review |
|---|---|---|---|---|---|---|
| 4b.1 | Legacy-Flashpfad ruft **dieselbe** Prüffunktion wie die API (`http_remote_stm32_filename_matches()`), keine zweite Prüfung | `esp-developer` | 4a.9 | `grep` findet den Aufruf an beiden Stellen; im Legacy-Zweig steht kein ungeprüftes `strncpy` des Dateinamens mehr | ☐ | ☐ |
| 4b.2 | Auswahlliste bietet bei `HARDWARE_CONFIGURATION` = 65535 **keine** Datei an, mit Hinweistext, der `./tools/flash-stm.sh` als Rückweg nennt | `esp-developer` | 4b.1 | **AK4.5**, lesend prüfbar. Der Hinweistext nennt den Rückweg — sonst führt die Härtung in eine Sackgasse | ☐ | ☐ |
| 4b.3 | `tools/smoke-device.sh`: `HARDWARE_CONFIGURATION` ≠ 65535 als eigene Prüfung | Lead | 4b.2 | Die Prüfung schlägt im gesunden Zustand nicht an und meldet den Zustand 65535 als Fehler (Gegenprobe gegen einen festen Testwert, nicht am Gerät erzeugt) | ☐ | ☐ |
| 4b.4 | ESP-Version anheben | `release-engineer` | 4b.3 | S4 zeigt den neuen Stand | ☐ | ☐ |
| 4b.5 | Build, Release-ZIP, Rollout, Einspielzeile | Lead | 4b.4 | `guardrails.sh --full` Exit 0; Commit und Tag sofort | ☐ | ☐ |
| 4b.6 | ESP einspielen | **Nutzer** | 4b.5 | neue ESP-Version in `/api/update_status` | — | — |
| 4b.7 | Gerätelauf, abschliessend **ein** STM-Flash über `./tools/flash-stm.sh` | `pwa-tester` + Freigabe | 4b.6 | **AK4.6**: `flash-stm.sh --check` vorher ohne Beanstandung, danach die erwartete STM-Version. **Nicht** `/api/remote_stm32_flash` von Hand (DIR-010) | — | ☐ |
| 4b.8 | `BEFUNDE.md` nachführen, Abschlussbilanz des Pakets | `doc-writer` | 4b.7 | S10 läuft durch; jeder im Paket berührte Befund trägt Status und Beleg | ☐ | ☐ |

---

## Was gleichzeitig laufen darf

Abgeleitet aus der Abhängigkeitsspalte, nicht aus der Besitzspalte:

- **0.1/0.2 und 0.3/0.4.** Andere Dateien, kein Gerätetest dazwischen, keine
  inhaltliche Abhängigkeit. Erlaubt.
- **1.4 und 1.5.** Beide hängen an unterschiedlichen Vorgängern und schreiben in
  verschiedene Dateien. Erlaubt.
- **3.4 und 3.6.** Beide rein lesend, beide nach 3.2. Erlaubt.

**Nicht gleichzeitig, obwohl die Dateien es hergäben:**

- Alles über eine Runden-Grenze hinweg. Zwischen den Runden steht ein Gerätelauf.
- 2.1 bis 2.7 untereinander — eine Datei, ein Schreiber (R3).
- 4a.3 und 4b.1 — beide im ESP, aber durch eine Flash-Runde getrennt. **Genau hier
  lag der Fehler vom 03.10.2026.**

---

## Freigaben, die der Nutzer geben muss

Lesende Abfragen sind frei (DIR-008). Diese Schritte sind es nicht, und sie stehen
hier als fertige Zeilen, damit niemand sie zusammensuchen muss:

| Wann | Was | Warum nicht ohne |
|---|---|---|
| Runde 1, AK1.1/AK1.2 | je ein Setter-Aufruf auf `ambilight_mode_profile_set` mit ungültigem und mit gültigem Wert, Ausgangswert vorher notiert | schreibend, Profilwert wird verändert |
| Runde 4a, 4a.1 und 4a.8 | ESP-Neustart, insgesamt viermal | Neustart, Variablensatz kann verloren gehen |
| Runde 4a, 4a.2 | Zustimmung zur sichtbaren Nebenwirkung von Weg A (IP-Lauftext je Versuch) | es ist seine Uhr im Dauerbetrieb |
| Runde 4b, 4b.7 | **ein** STM-Flash über `./tools/flash-stm.sh` | Firmwarewechsel, ESP startet mit |
| jede Runde | das Einspielen selbst (ESP, PWA, STM) | der Nutzer spielt Updates selbst ein |

**Nie, in keiner Runde:** `GET /?a` (Parameter ohne `=`), `/api/test_display`,
`/api/learn_ir`, `/api/ir_code_set`, `maintenance_reset_eeprom`,
`maintenance_format_fs`, `fs_remove` auf ein App-Asset, Backup-**Import**. Zwei
PreToolUse-Hooks weisen diese Endpunkte ab, bevor ein Agent sie erreicht.

---

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings
3. Review durch den zuständigen Review-Agenten, ausdrücklich gegen
   `knowledge/architecture-checklist.md`
4. Erst dann nächster Task oder Schreibübergabe

**Beim Durchsuchen von `src/**` immer `grep -a`.** Ohne `-a` überspringt `grep` acht
Quelldateien stillschweigend, darunter die grösste (L166).

---

## Abschluss

- [ ] Alle Tasks aller Runden erledigt
- [ ] `./tools/guardrails.sh --full` mit Exit 0 (schliesst Compile-Smoke-Tests ein)
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt — **oder** ausdrücklich
      als nicht erfüllt benannt, mit Grund. Ein stillschweigend ausgelassenes
      Kriterium ist der Fall aus DIR-012
- [ ] Je Runde ein Release-ZIP, ein Commit, ein Tag `release/<stm>-<esp>-<app>`
      (DIR-011) — nicht gesammelt
- [ ] Flash-Umfang je Runde ausdrücklich benannt
- [ ] Der Beobachtungsauftrag A28 hat Daten geliefert: `d=`-Werte aus allen
      Gerätelaufen, mit Fenster und gleichzeitigen Aufrufen
- [ ] Trat während des Pakets eine `Exception (9)` mit Text in `excvaddr` auf, ist
      das Paket angehalten und C13 als eigene Analyse aufgesetzt
