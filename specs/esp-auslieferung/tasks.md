# Tasks — Den Auslieferungspfad des ESP entlasten

**Datum:** 2026-10-03 (Momentaufnahme, DIR-006)
Gehört zu `requirements.md` und `design.md` im selben Verzeichnis.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem schreibenden Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

---

## Reihenfolge gegenüber der i18n-Auslagerung — zuerst lesen

Es gibt eine zweite Spec, die ebenfalls `ESP8266/ESP-uclock/http.cpp` ändert:
`specs/i18n-auslagerung/` (Massnahme **B15**, Befunde L126/L127). Sie fasst dort die
Weissliste `APP_INSTALL_ASSETS` an, diese Spec die Auslieferungsfunktionen. Zwei
Umbauten, dieselbe Datei, verschiedene Funktionen.

**Diese Spec läuft zuerst. Drei Gründe, in dieser Reihenfolge:**

1. **Sie liegt auf der richtigen Schicht und ist der weit grössere Hebel.**

   | Weg | Frames | Faktor |
   |---|---|---|
   | heute (`app.js.gz` 129'297 B, 256-Byte-Blöcke) | 506 | — |
   | **diese Spec** (Blockgrösse 1024, Nagle für die Auslieferung) | 241 bis 253 | **2,10** |
   | Sprachauslagerung B15 (`app.js.gz` 115'843 B, Blockgrösse unverändert) | 453 | **1,12** |

   **Die 1,12 sind am 03.10.2026 nachgemessen und ersetzen die früher genannten
   1,28.** Jene Zahl stammte aus einer Rechnung, die **beide** Sprachtabellen
   herausnahm; ausgelagert wird aber nur Englisch, Deutsch bleibt im Bundle. Gemessen:
   Quelle 577'421 → 517'987 Byte, gepackt 129'297 → 115'843 Byte. **Der Abstand ist
   damit nicht knapp, sondern deutlich** — 2,10 gegen 1,12. Und die Ursache liegt im
   ESP; diese Spec ändert den ESP, die andere die PWA.

2. **Sie hält den Messpunkt fest.** Der Vorher-Wert gilt für `app.js.gz` in seiner
   heutigen Grösse (L128: 505 Frames bei 129'313 Byte; am 03.10.2026 nachgemessen 506
   bei 129'297 — dieselbe Bezugsgrösse auf einen Frame genau). Läuft die
   Sprachauslagerung zuerst, schrumpft die Datei um rund 13'500 Byte gepackt, und der
   Faktor 2,10 lässt sich gegen keinen bekannten Vorher-Wert mehr prüfen. Umgekehrt
   stört diese Spec die i18n-Messung nicht: Deren Kennzahl ist die **Dateigrösse**, und
   die ändert sich hier nicht um ein Byte.
3. **Ihr Wirkungsradius ist kleiner.** Hier: ESP, ein Flash, keine PWA, kein Makefile,
   keine Guardrail-Stufe. Dort: ESP **und** PWA **und** Build **und** Guardrails **und**
   `install-app.sh`. Die kleine Änderung zuerst durchzulassen hält die Zuordnung
   sauber, wenn danach etwas schiefgeht.

**Beide werden nicht gleichzeitig umgesetzt.** Nicht, weil der Besitz-Hook es
verböte — `http.cpp` gehört in beiden Fällen dem `esp-developer`, der Hook liesse beide
durch. Sondern weil:

- **R3b:** Dateibesitz ist nicht Reihenfolge. Zwei Agenten derselben Rolle gleichzeitig
  in einer Datei mit über 12'000 Zeilen schreiben sich gegenseitig zu, und es fällt erst
  auf, wenn der Stand kaputt ist.
- **R1/R5:** Jede der beiden Specs endet in einem ESP-Flash. Es gibt eine physische Uhr
  im Dauerbetrieb und einen Serial-Port; zwei Flash-Runden ineinander sind nicht
  auflösbar, und ein Fehlschlag wäre nicht mehr zuzuordnen.

**Die Freigabe für `specs/i18n-auslagerung/` wird erteilt, wenn Task 8 dieser Spec
abgeschlossen ist** — also nach der Abnahme von Runde B und der Dokumentation. Vorher
nicht.

**Was die Reihenfolge ausdrücklich nicht heisst:** dass B15 danach überflüssig wäre.
Die Sprachauslagerung ist aus eigenen Gründen richtig (Erweiterbarkeit, Startgewicht),
und beide Wege zusammen senken die Last weiter als jeder für sich. Sie ist nur nicht
der L125-Fix, als den man sie lesen könnte.

---

## Warum zwei Flash-Runden

| Runde | Inhalt | Zweck |
|---|---|---|
| **A** | nur M4 (Task 1) | Das Messmittel muss **vor** dem Umbau auf dem Gerät sein. Ohne Vorher-Wert für `free_heap` und `max_free_block` ist der Nachher-Wert bedeutungslos — und `design.md` nennt eine Nebenwirkung des Mount-Umbaus, die sich **nur** so beobachten lässt |
| **B** | Teil 1 und Teil 2 (Tasks 4 und 5) | Der eigentliche Umbau |

**Teil 1 und Teil 2 gehen bewusst in dieselbe Runde, und das ist eine Entscheidung mit
Preis.** Beide liegen in `http_send_fs_file`, beide hingen sonst an je einem eigenen
Release mit Rollout, Flash und Tag. Entscheidend ist, dass sie **unterscheidbar
bleiben**: Teil 1 ist an der Framezahl messbar (AK7), Teil 2 ist im Normalbetrieb
überhaupt nicht beobachtbar — es sind Fehlerpfad- und Allokationskorrekturen. Geht
Runde B daneben, ist Teil 1 der erste Verdächtige, weil nur er das Verhalten auf der
Leitung ändert.

**Wer das anders will, teilt Runde B in B1 (Teil 2) und B2 (Teil 1) — und zieht die
Konsequenz: zwei Versionsanhebungen, zwei Rollouts, zwei Tags, zwei Abnahmen.** Nicht
zwei Tasks parallel starten und hoffen (R3b, am 03.10.2026 genau so schiefgegangen).

---

## Tasks

| # | Task | Agent | Hängt ab von | Guardrails | Review |
|---|---|---|---|---|---|
| 1 | **M4** — `free_heap` und `max_free_block` in `http_api_device_ready` (`http.cpp:10984`), Werte **vor** der Ausgabe einlesen | `esp-developer` | — | ☐ | ☐ |
| 2 | `ESP_VERSION` anheben, `make release-zip`, `./tools/deploy.sh` | `release-engineer` | 1 | ☐ | ☐ |
| 3 | **Runde A:** ESP flashen, Vorher-Messung, Smoketest, Commit und Tag | Nutzer (Flash) / Lead (Messung, Commit) | 2 | ☐ | ☐ |
| 4 | **Teil 2** — Leseabbruch mit `yield()` und `rtc = 2`, ein Ausstieg in `http_find_stored_app_asset_filename`, `http_app_installation_complete()` nur unter `is_pwa_index` | `esp-developer` | 3 | ☐ | ☐ |
| 5 | **Teil 1** — Blockgrösse 1024 über `http_response`, `setNoDelay(0)` lokal in der Auslieferung | `esp-developer` | 4 | ☐ | ☐ |
| 6 | `ESP_VERSION` anheben, `make release-zip`, `./tools/deploy.sh` | `release-engineer` | 5 | ☐ | ☐ |
| 7 | **Runde B:** ESP flashen, vollständige Abnahme (unten), Commit und Tag | Nutzer (Flash) / Lead (Abnahme, Commit) | 6 | ☐ | ☐ |
| 8 | `BEFUNDE.md` und `CHANGELOG.md` nachführen: C9 und C9b, Messwerte aus Runde A und B, **L125 bleibt offen** | `doc-writer` | 7 | ☐ | ☐ |

**Task 4 und 5 sind nicht parallelisierbar** — sie ändern dieselbe Funktion. Derselbe
Agent, nacheinander, in dieser Reihenfolge: erst die Defekte, dann die Blockgrösse. So
steht die Abbruchlogik schon, wenn der Puffer wächst.

---

## Was in Task 3 gemessen wird (Runde A, Vorher-Werte)

Alle Werte in den Bericht; `BEFUNDE.md` schreibt später der `doc-writer` (Task 8).

1. **Heap im Ruhezustand**, dreimal im Abstand von je einer Minute:
   `curl -s "http://$DEVICE_HOST/api/device_ready"`
2. **Heap unmittelbar nach fünf `app.js`-Abrufen** — die fünf Abrufe nacheinander, dann
   sofort `device_ready`. Jeder Abruf mit
   `-w '%{http_code} %{size_download} %{time_total}\n'`; **die fünf Zeiten sind der
   Vorher-Wert für AK8.**
3. **Was dabei passiert, wird festgehalten, auch wenn es unangenehm ist.** Nach L125 ist
   rund ein Absturz auf fünf Abrufe zu erwarten. Tritt er ein: Uhrzeit notieren, der
   ESP startet neu, der Variablensatz ist weg (L42/L103), und der STM läuft weiter. Das
   ist der bekannte Zustand, kein neuer Befund.
4. `./tools/smoke-device.sh` und `./tools/install-app.sh --check` — Letzteres, weil
   nach **jedem** ESP-Update zu prüfen ist, ob die PWA-Dateien noch gefunden werden.

**Wenn es geht, hier schon den Mitschnitt fahren** (`design.md`, „Wie die Framezahl
nachgewiesen wird"): Ein Vorher-Wert von der eigenen Leitung ist mehr wert als die 505
aus der Rechnung. Er kostet aber einen weiteren `app.js`-Abruf mit dem bekannten
Risiko — deshalb freiwillig, nicht gefordert.

---

## Abnahme nach Task 7 (Runde B)

Diese Reihenfolge, nicht eine andere — die Guardrails sind ausnahmslos statisch und
sagen über das Laufzeitverhalten von `http.cpp` nichts, und durch diese Datei läuft
**jeder** Request.

1. **`./tools/smoke-device.sh`** — Erreichbarkeit, gemeldete Versionen, alle PWA-Assets,
   elf lesende Endpunkte auf die jeweils erwartete Antwortform, Legacy-Oberfläche,
   Absturzfestigkeit bei Parametern ohne `=`, Abwehr eingebetteter Wartungsaufrufe.
   Deckt AK2, AK3, AK6, AK9.
   **Er ist nicht der Test (DIR-012):** Er prüft, ob das Gerät lebt, nicht ob es noch
   tut, was es soll. „27/0" zu schreiben und „getestet" zu meinen, ist das Warnzeichen.
2. **`./tools/install-app.sh --check`** — nach jedem ESP-Update fällig. Die Datei ist
   nach einem Firmware-Wechsel nicht weg, sie wird nur nicht mehr gefunden, wenn die
   Weissliste nicht passt.
3. **`./tools/check-pwa.sh`** — lädt die PWA vom Gerät in einen echten Browser und
   meldet, ob `app.js` beim Laden einen **Fehler wirft**. Das zeigt weder API noch
   Smoketest noch Screenshot. Deckt AK4.
4. **Framezahl messen** nach `design.md`, „Wie die Framezahl nachgewiesen wird".
   Deckt AK7. Wird der Ersatzweg ohne `sudo` genommen, steht das im Bericht.
5. **Fünf `app.js`-Abrufe** mit `%{size_download}` und `%{time_total}`, danach sofort
   `device_ready`. Deckt AK1, AK8, AK10.
6. **`pwa-tester`**, Phasen 0 bis 4 und 9. Pflicht nach DIR-012. Backup-Import,
   Verbindungstrennung und die gefährlichen Funktionen bleiben beim Nutzer. Deckt AK5.
   **Wird er bewusst ausgelassen, gehört das in den Bericht** — am 03.10.2026 ist er
   dreimal übersprungen worden, und gefragt hat der Nutzer.
7. **`./tools/guardrails.sh --full`** — Exit 0, Compile-Smoke eingeschlossen.
   Deckt AK18, AK19.
8. **Quelltextprüfungen** des Umsetzers, im Bericht beantwortet, nicht abgehakt:
   AK12 (Rückgabetyp und Fehlerwert von `File::read` **im Core nachgesehen**),
   AK13 (`begin`/`end`-Zählung über `http.cpp`, vorher 28 zu 27),
   AK14 (Nachweis, dass die Verschiebung von `http_app_installation_complete()`
   verhaltensneutral ist), AK15, AK16, AK17 (UTF-8).
9. **Commit und Tag sofort** — `release/<stm>-<esp>-<app>` mit den Versionen **dieses**
   Commits (DIR-011). Nicht gesammelt, nicht auf Nachfrage. `deploy.sh` nennt den
   fehlenden Tag-Befehl bei jedem Lauf; diese Zeile ist kein Rauschen.

---

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings
3. Review durch den zuständigen Review-Agenten, ausdrücklich gegen
   `knowledge/architecture-checklist.md`
4. Erst dann nächster Task oder Schreibübergabe

**Teammates führen niemals `make` aus (R1).** Der `esp-developer` ändert
`http.cpp` und meldet fertig; gebaut wird in Task 2 und Task 6 vom
`release-engineer`, gerollt und geflasht wird danach.

---

## Abschluss

- [ ] Alle Tasks erledigt
- [ ] `./tools/guardrails.sh --full` mit Exit 0
- [ ] Alle Akzeptanzkriterien AK1 bis AK20 aus `requirements.md` erfüllt oder mit
      Begründung als ersetzt gekennzeichnet
- [ ] Release-ZIP je Runde gebaut, Flash-Umfang ausdrücklich benannt: **nur ESP**,
      kein STM-Flash, kein PWA-Upload
- [ ] Je Rollout ein Commit und ein Tag (DIR-011)
- [ ] `BEFUNDE.md`: C9 und C9b auf erledigt, **L125 bleibt offen** mit dem Zusatz
      „Last gesenkt, Rate erwartet halbiert, Ursache unverändert offen". Der ToDo-Block
      ganz oben wird mitgeführt — Guardrail S10 prüft, dass jeder offene Befund dort
      steht
- [ ] Freigabe für `specs/i18n-auslagerung/` erteilt

---

## Freigabe

**Die Spec beginnt nicht, bevor der Nutzer sie freigibt.** Zwei Punkte gehören ihm
ausdrücklich vorgelegt, weil sie Entscheidungen und nicht Technik sind:

1. **Zwei ESP-Flash-Runden statt einer** — der Preis für den Vorher-Wert der
   Heap-Messung.
2. **Die Vorher-Messung in Runde A kostet nach heutiger Rate voraussichtlich einen
   ESP-Absturz** (fünf `app.js`-Abrufe, rund ein Absturz auf fünf). Es ist seine Uhr im
   Dauerbetrieb. Der Absturz ist folgenlos ausser dem ESP-Neustart und dem verlorenen
   Variablensatz — aber er passiert absichtlich, und das ist etwas anderes.
