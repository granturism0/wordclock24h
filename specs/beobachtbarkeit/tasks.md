# Tasks — Die Uhr beobachtbar machen

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

## Prüfbarkeit auf zwei Ebenen

Sechs Tasks am STM hiessen sechs Flash-Vorgänge auf einer produktiv laufenden Uhr.
Das wäre keine Sorgfalt, sondern Zumutung. Deshalb:

- **Statisch, nach jedem Task:** `./tools/guardrails.sh`, Review gegen
  `knowledge/architecture-checklist.md`, bei Task 7 zusätzlich der ELF-Vergleich.
- **Am Gerät, in zwei Runden:**
  - **Runde A** nach Task 2 — isoliert genau die Änderung, deren Wirkung die Spec
    beziffert (AK1, AK2, AK3). Geht sie daneben, ist die Ursache eindeutig.
  - **Runde B** nach Task 6 — alles Übrige (AK4 bis AK9).

Geflasht wird vom Nutzer. Was zu flashen ist, steht bei der jeweiligen Runde.

| # | Task | Agent | Hängt ab von | Guardrails | Review |
|---|---|---|---|---|---|
| 1 | `tools/measure-log.sh` um die Auswertung nach Zeilenklassen erweitern (drei häufigste Zeilenanfänge mit Anzahl), danach die **Vorher-Messung unter Ticker-Last** nachholen | Lead | — | ☐ | ☐ |
| 2 | `sk6812.c`: `:476` und `:495` entfernen, `:463` unverändert lassen; Refresh-Zähler und sättigenden DMA-Wartezähler ergänzen, zwei Zugriffsfunktionen in `sk6812.h` | `stm-developer` | 1 | ☐ | ☐ |
| — | **Runde A am Gerät** — AK1, AK2, AK3 | Nutzer + Lead | 2 | ☐ | ☐ |
| 3 | `uart-driver.h`: `else`-Zweig mit sättigendem Verwurfszähler bei `:698`, höchster Füllstand im Erfolgszweig; zwei Zugriffsfunktionen je Präfix, Deklaration in `uart.h` | `stm-developer` | 2 | ☐ | ☐ |
| 4 | `main.c`: Zähler für Zeitgeber-Interrupts in der ISR; Diagnosezeile am Kopf des Hauptloops mit Folgenummer, regulärem Takt und selbstkalibrierendem Rückfall | `stm-developer` | 3 | ☐ | ☐ |
| 5 | `vars.c`: Quittungs-Flag in `var_send_buf()`, `watchdog_reload()` **nur** bei eingetroffener Quittung, nicht im verschachtelten Fall | `stm-developer` | 4 | ☐ | ☐ |
| 6 | `ESP-uclock.ino`: Kappungsmarke in `stm32_log_append()` und beim Überlauf des `cmd_buffer` | `esp-developer` | — | ☐ | ☐ |
| — | **Runde B am Gerät** — AK4 bis AK9 | Nutzer + Lead | 5, 6 | ☐ | ☐ |
| 7 | `CMakeLists.txt`: `option(WORDCLOCK_DEBUG … OFF)`, `DEBUG` durchreichen; Beschreibung mit beiden Warnungen aus `design.md` Abschnitt 7 | Lead | — | ☐ | ☐ |
| 8 | `tools/guardrails.sh`: `WD_EXPECTED` auf 7, Kommentar `:171-177` auf die neue Begründung umschreiben | Lead | 5 | ☐ | ☐ |
| 9 | `tools/smoke-device.sh`: prüft, dass eine Diagnosezeile vorliegt und die Folgenummern lückenlos sind | Lead | 4 | ☐ | ☐ |
| 10 | `knowledge/quick-reference.md:15` richtigstellen: nicht „auf `debug_log_printf` umstellen", sondern entfernen und durch einen Zähler ersetzen, den die Diagnosezeile trägt | `doc-writer` | 2 | ☐ | ☐ |
| 11 | Versionen anheben: `src/main.h`, `ESP8266/ESP-uclock/version.h`. **Nicht** `APP_VERSION`, **nicht** `CACHE_NAME` | `release-engineer` | 10 | ☐ | ☐ |
| 12 | Vollständiger Build, Release-ZIP, Rollout nach DIR-005; Flash-Umfang benennen | Lead | 11 | ☐ | ☐ |

Task 6 und 7 hängen an nichts und können jederzeit laufen — sie berühren weder
`src/**` noch die Messung. Task 6 muss nur **vor** Runde B fertig sein.

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings
3. Review durch den zuständigen Review-Agenten, ausdrücklich gegen
   `knowledge/architecture-checklist.md`
4. Erst dann nächster Task oder Schreibübergabe

## Was bei den einzelnen Tasks leicht schiefgeht

**Task 2** — `sk6812.c:463` ist keine der beiden zu entfernenden Zeilen. Wer alle
drei entfernt, besteht zwar die Guardrail-Prüfung, nimmt aber Gerätetest M3 sein
Instrument. Der DMA-Wartezähler wird an derselben Bedingung erhöht, unter der
`:463` ausgibt — nicht bei jedem Schleifendurchlauf, sonst zählt er Takte statt
Vorfälle.

**Task 3** — `uart-driver.h` wird je UART neu eingebunden; die Zähler entstehen
damit mehrfach, benannt über das Präfixverfahren wie `UART_PREFIX_RSIZE`
(`:434`). Der Verwurfszähler gehört in den **fehlenden** `else`-Zweig, nicht hinter
das `if` — dort zählte er jedes empfangene Zeichen. Der Nachweis dieses Tasks ist
statisch: Übersetzung plus Review. Sichtbar wird er erst mit Task 4.

**Task 4** — Die Zeile steht am Kopf des Hauptloops, nicht in einem flaggesteuerten
Zweig. Der Zähler für die Zeitgeber-Interrupts zählt **jeden** Interrupt, nicht die
Sekunden; nur so trennt die Fallunterscheidung aus `design.md` die Zeilen 2 und 3.
Format und Längenrechnung stehen in `design.md`; wer ein Feld ergänzt, rechnet neu.

**Task 5** — Der Reload steht hinter einer Bedingung. Ohne sie bricht er L25 und
den ausdrücklichen Vermerk in `guardrails.sh:171-177`. Im verschachtelten Fall
(`var_send_nested`) wird nicht geladen.

**Task 8** — Die Zahl allein reicht nicht. Der Kommentar daneben schliesst genau
diese Änderung aus; bleibt er stehen, dreht der nächste Durchgang sie zurück.

**Task 11** — STM **und** ESP werden geändert, also steigen beide Versionen
(DIR-004). Kein Gleichschritt mit der PWA — sie wird nicht angefasst.

## Runde A am Gerät — AK1, AK2, AK3

Zu flashen: **STM32**. Der ESP ist in dieser Runde unverändert.

```
./tools/flash-stm.sh --check
./tools/flash-stm.sh
./tools/smoke-device.sh
./tools/measure-log.sh 30          # Ruhe  -> AK3, erwartet >= 5 min Reichweite
```

Dann startest du über die Oberfläche ein Ticker-Overlay und lässt es laufen:

```
./tools/measure-log.sh 15          # unter Last -> AK2, gefordert >= 60 s
```

15 s statt 30: Läuft der Ring während der Messung vollständig um, weist das Skript
nur eine Untergrenze aus — und eine Untergrenze beantwortet AK2 nicht.

Zusätzlich die Klassenauswertung aus Task 1 lesen. Steht die Reichweite über 60 s,
aber die häufigste Zeilenklasse ist eine unerwartete, ist AK2 **nicht** bestanden,
sondern der nächste Befund gefunden.

## Runde B am Gerät — AK4 bis AK9

Zu flashen: **STM32 und ESP**.

```
./tools/flash-stm.sh
./tools/smoke-device.sh
curl -s "http://$DEVICE_HOST/api/stm32_log" | python3 -m json.tool | grep diag
sleep 60
curl -s "http://$DEVICE_HOST/api/stm32_log" | python3 -m json.tool | grep diag
```

Zu prüfen:

- **AK5** — die Folgenummern sind lückenlos, der Abstand zwischen zwei Abfragen
  entspricht dem Takt
- **AK4** — `rx=` steht über 0 und ändert sich mit der Last, `d=` wird abgelesen.
  Beide auf 0 ist kein Bestehen
- **AK6** — am Code und in `design.md` nachvollziehbar, nicht am Gerät auslösbar:
  Die Zeitgeber-ISR wird dafür nicht künstlich angehalten
- **AK7** — beide Byte-pro-Minute-Werte aus den gemessenen Raten ausrechnen und im
  Umsetzungsbericht nennen
- **AK8** — am Code ablesbar. Die tote Brücke wird **nicht** nachgestellt; das hiesse,
  die Verbindung zur produktiven Uhr zu trennen, und das bleibt beim Nutzer (R5)
- **AK9** — eine Zeile nahe der Grenze suchen (`sk6812_refresh: waiting`, rund 105
  Zeichen) und prüfen, dass eine gekappte Zeile ihre Marke trägt

Alles davon ist lesend. Kein Aufruf aus der Gefahrenliste in `CLAUDE.md`,
insbesondere kein Parameter ohne `=`.

## Abschluss

- [ ] Alle Tasks erledigt
- [ ] `./tools/guardrails.sh --full` mit Exit 0 (schliesst Compile-Smoke-Tests ein)
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt, jedes mit seiner Zahl
- [ ] `BEFUNDE.md`: C7 und C5 auf erledigt, L75 und L85 nachgeführt, der Vermerk
      „blockiert C5" bei AK7 aufgelöst — AK7 ist jetzt durchführbar und wird
      wiederholt
- [ ] Release-ZIP gebaut, Flash-Umfang benannt (STM32 **und** ESP)
- [ ] Nach dem ESP-Update: `./tools/install-app.sh --check` — die PWA wird zwar nicht
      geändert, aber ein Firmware-Wechsel ist der Zeitpunkt, an dem das sonst
      auffällt
