# Tasks — Paket 2026-10-09 „Brückenlast" (L338/C43, L339/A58)

**Erstellt:** 2026-10-09, Stand `dea187a`. Momentaufnahme (DIR-006).

**Status: zur Freigabe (Ent-1).** Ohne Freigabe des Nutzers beginnt keine Implementierung.
Die Aufteilung in E, M und C3 folgt der Empfehlung **V1** aus Ent-3; bei V3 gilt der Hinweis
unter Schritt M.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wo die Dateien disjunkt sind** (R3b). Die
wichtigste Abhängigkeit dieses Pakets: **M.1 beginnt erst nach dem Gerätenachweis G1 (E.9)**,
damit die Vorher-Messung G2 unter dem **neuen** ESP läuft und sich Vorher und Nachher nur in C3
unterscheiden (`design.md` §0). Und: **Jeder Release-Build nimmt den ganzen Arbeitsbaum mit** —
kein Release-Build, solange ein Produktcode-Task eines anderen Schritts offen ist.

**`Nutzer` in der Agentenspalte heisst:** Der Lead liefert die Zeile fertig zum Einfügen, der
Nutzer führt sie aus. Er spielt **jedes** Update selbst ein.

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash` (L238, L269).
Bauen, Gerätemessung und Git gehören zum Lead. **Prüfstände baut der Umsetzer im Scratchpad,
der Lead legt sie unter `tools/checks/auszug/` ab** — `tools/**` ist sein Revier.

**Gerätenachweise G1–G3** brauchen die Freigabe aus **Ent-1** im selben Gespräch. Jeder läuft
mit `./tools/watch-log.sh` (DIR-013).

---

## Schritt V — Vorbereitung, rein lesend

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| V.1 | **Sechs Fragen, je mit Fundstelle:** (1) **Seitengrösse** der verbauten EEPROMs aus dem Datenblatt — V2: AT24C32M (`HARDWARE.md`, U6; KiCad-Projekt unter `~/Documents/WordClock-USB C - STM32F411 - V2`); F103: Baustein auf dem RTC-Modul, falls aus Repo oder Nutzerangabe bestimmbar, sonst ausdrücklich als offen melden. (2) **Periodische EEPROM-Schreiber:** welche der `eep_write()`-Aufrufer laufen ohne Nutzeraktion, und wie oft höchstens? (3) Kann `eeprom_write()` **vor `log_init()`** oder vor der Initialisierung der ESP-UART laufen? (4) **Flash-Gate Stufe 1:** Schätzung für Messzeile (M.1) und C3 (C3.1), je Unter- und Obergrenze, gegen 664 Byte nutzbar (`design.md` §2.6). (5) **ESP-Core:** Begrenzt `WiFiClient::setTimeout()` im verwendeten Core auch DNS und `connect()`? Core-Version aus Makefile/Build-Konfiguration, Antwort aus der Core-Quelle. (6) Wie reagiert der STM, wenn auf `weather`/`weather_fc` **keine** `WEATHER`-Antwort kommt — bleibt ein Zustand oder Flag offen? | `firmware-analyst` | — | Antworten mit Fundstellen. (1) geht an C3.1 und AKC.1; bleibt der F103-Baustein offen, gilt dort die kleinste sichere Seitengrösse, und der Bericht nennt sie. (2) entscheidet die Schwelle in AKM.1. (3) und (6) gehen an M.1 bzw. E.1. (4): **Passt die Obergrenze nicht in 664 Byte, geht der Bericht über den Lead an den Nutzer**, bevor M.1 beginnt. **Entscheidet nichts** | — | — |

---

## Schritt E — ESP-Release (Teil A: L338, Teil C: Echo)

**Einspielreihenfolge:** nur ESP. Keine PWA- oder STM-Änderung vorausgesetzt.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E.1 | **Teil A — L338:** `query_weather()` (`weather.cpp:307-427`) nach `design.md` §1.2: **eine** Lesehilfe mit Gesamtfrist `WEATHER_TOTAL_TIMEOUT_MS` = 2'500 ms (§1.3), die liest, solange `available () \|\| connected ()`, und in jedem leeren Durchlauf die Kontrolle abgibt; `delay (200)` (`:359`) und `if (available ())` (`:361`) entfallen; Parselogik unverändert; Timeout vor `connect ()` auf die Restfrist, **sofern V.1 (5) das trägt**; Messzeile `- weather fc=… ms=… <ergebnis>` auf **jedem** Pfad, per `snprintf` in einen Stackpuffer, ausgegeben über `Serial` **und** `stm32_log_append ()` wie `esp_heap_log()`. Kommentar an der Konstante: hängt an der 3-s-Wartezeit des STM. **Prüfstand t12** im Scratchpad bauen (§1.4) | `esp-developer` | V.1 (5), (6) | **AKE.1–AKE.5** am Prüfstand, Fallzahl gemeldet; **gegen das alte `weather.cpp` einmal fehlgeschlagen** (Fall „Körper ohne `\n`" ≥ 5'000 ms, Fall „Antwort nach 300 ms" ungeparst). Bericht nennt Kodierung und Zeilenende von `weather.cpp` vor dem Patch (AKZ.8) und, falls `connect ()` nicht begrenzbar ist, die verbleibende Lücke | ☐ | ☐ |
| E.2 | **Teil C — Echo:** **eine** Funktion für die Echo-Zeile, gerufen von der POST-Stelle (`http.cpp:14071-14079`) **und** der GET-Stelle (`:14129-14143`); Methode und Pfad, Querystring ersetzt durch `?…`, keine HTTP-Version (`design.md` §3). **Prüfstand t13** im Scratchpad. **Zusätzlich:** Liste weiterer Stellen in `http.cpp`, die Anfragewerte über `Serial` ausgeben — **gemeldet, nicht mitkorrigiert** | `esp-developer` | E.1 | **AKE.7** am Prüfstand t13 (Fälle: Setter mit Wert, `appid`, `GET /?a`, ohne Query, nur `?`, POST mit Query). `grep` findet kein `Serial.print*` mit `sRequest` oder `sParam` mehr. Kodierung (UTF-8) und Zeilenende von `http.cpp` vor dem Patch festgestellt (AKZ.8) | ☐ | ☐ |
| E.3 | **Prüfstände ablegen:** t12 und t13 unter `tools/checks/auszug/esp/`, Einträge in `tools/checks/auszug.sh`; **statische Echo-Prüfung** (kein `Serial.print*` mit `sRequest`/`sParam` in `http.cpp`) in `auszug.sh` oder als Guardrail-Stufe | Lead | E.2 | `auszug.sh` meldet zwei Prüfstände mehr, beide OK. **Gegenprobe** `auszug.sh --gegen <Tag des Ausgangsstands>`: t12 meldet FEHL, die statische Prüfung schlägt an (DIR-014). t13 ist dort nicht übersetzbar — **nicht** als Gegenprobe gezählt (`design.md` §3). Die Meldung kommt bei Exit ≠ 0 an, nicht nur auf stderr | ☐ | — |
| E.4 | **Werkzeug:** `tools/watch-log.sh` meldet `- weather … timeout`, `- weather … connfail` und Wetter-`ms` ≥ 2'500; dazu bereits jetzt `eep … ms=` ≥ 200 (wirksam ab M). **Zusätzlich** `grep` über `tools/**`, `.claude/**` und `app.js`, dass keine Auswertung die volle `- request`-Zeile braucht (Stand 09.10.2026: nur `watch-log.sh:128`, dem der Pfad genügt) | Lead | — | Gegenprobe je Meldung mit einer eingespielten Zeile (DIR-014); die Meldung erscheint dort, wo der Lead während eines Laufs liest. `grep`-Ergebnis im Bericht (Vorbedingung für AKE.8) | ☐ | — |
| E.5 | **Review ESP-Seite** gegen die Checkliste | `code-reviewer` | E.3 | Vier Punkte, dazu: (1) Liest die Lesehilfe bei **jedem** Ende — `\n`, Verbindungsende, Frist — richtig weiter oder ab, und gibt sie in jedem leeren Durchlauf die Kontrolle ab (AKE.5)? (2) Erhält der Parser für wohlgeformte Antworten dieselbe Zeichenkette (AKE.4)? (3) Passt `WEATHER_TOTAL_TIMEOUT_MS` zur 3-s-Wartezeit von `var_send_buf()` im STM-Quelltext? (4) Braucht die Lesehilfe eine Längengrenze gegen einen feindlichen Server — **Antwort mit Begründung**; „ja" ist ein eigener Befund, ausser die Grenze entsteht ohne Mehraufwand (`design.md`, Secure by design). (5) Enthält keine der beiden neuen Zeilen einen Wert? (6) Rufen **beide** Echo-Stellen dieselbe Funktion? **Die Antwort steht am Code** | — | ☐ |
| E.6 | ESP-Version anheben | `release-engineer` | E.5 | S4 zeigt den neuen Stand | ☐ | — |
| E.7 | `git status` (kein offener Produktcode-Task), Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | E.6, E.4 | `guardrails.sh --full` Exit 0 (AKZ.2); Tag `release/<stm>-<esp>-<app>` gepusht (AKZ.6) | ☐ | — |
| E.8 | **Nutzer:** ESP einspielen | **Nutzer** | E.7 | neue ESP-Version in `/api/update_status` | — | — |
| E.9 | **Abnahme am Gerät, G1** (`design.md` §4.1): Smoketest **samt Update-Quelle** (DIR-009), `install-app.sh --check` (DIR-017); je **ein** `weather_get_now` und `weather_get_forecast`; danach **60 Minuten lesend** mitschneiden | Lead | E.8, Ent-1 | **AKE.6:** Messzeile `ok`, `ms` < 2'000, auf ±150 ms gleich dem Abstand `weather`→`WEATHER`; `v` steigt im 6-s-Fenster nicht; in 60 min (≥ 3 automatische Abrufe) kein `v`-Anstieg nach einem `weather`-Kommando; Messzeile im Mitschnitt **und** in `/api/stm32_log`. **AKE.8:** keine `- request`-Zeile mit `=`, Zahl der geprüften Zeilen > 0 gemeldet. Kein ESP-Neustart; ein `v`-Anstieg durch Neustart (L327) wird getrennt berichtet. `watch-log.sh` mitgelesen | — | ☐ |

---

## Schritt M — STM-Release mit Messzeile (Teil B, Stufe 1)

**Warum vor C3:** Die Messzeile muss einmal den schlechten Wert zeigen (DIR-014), und in L204
war die EEPROM-Spur schon einmal falsch (`design.md` §0, Ent-3).

**Bei Ent-3 = V3** entfallen M.4 bis M.7: M.1 bis M.3 laufen dann unmittelbar vor C3.1 und
gehen mit dem C3-Release hinaus; AKM.3 wird durch AKC.5 und S.26 ersetzt (`design.md` §4.3).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| M.1 | **Messzeile in `eeprom_write()`** (`src/eeprom/eeprom.c:201-240`) nach `design.md` §2.2: `eep a=… n=… z=… ms=… d=…/…`, nur bei `z ≥ 1` (Schwelle `ms ≥ 20` nur nach V.1 (2)), kein Inhaltsbyte, `d` über `esp8266_uart_rxdrops()`. **Keine Verhaltensänderung** — Schreibfolge, Rückgabewerte und Wartezyklen bleiben byte-genau. Füllstand nur, wenn er höchstens rund 40 Byte kostet. Keine Ausgabe vor `log_init()` (V.1 (3)) | `stm-developer` | **E.9**, V.1 (2), (3), (4) | **AKM.1** am Quelltext; Bericht nennt Zeitbasis, Kodierung und Zeilenende von `eeprom.c` (AKZ.8), und ob der Füllstand dabei ist. `git diff` zeigt **ausserhalb** der Messzeile keine Änderung an der Schreiblogik | ☐ | ☐ |
| M.2 | **Flash-Gate, Testbau** nach M.1: nur F103, kein Release-ZIP | Lead | M.1 | **AKM.4:** Rest ≥ 1'024 Byte, Zuwachs notiert. Darunter: Halt, Bericht an den Nutzer | — | — |
| M.3 | **Review STM-Seite** gegen die Checkliste | `firmware-analyst` | M.2 | Die Schreiblogik ist **unverändert** (Zeile für Zeile gegen den Ausgangsstand); `z` zählt genau die Aufrufe von `eeprom_waitstates()`; die Zeile kann keinen Inhaltswert ausgeben; kein Aufruf vor `log_init()` | — | ☐ |
| M.4 | STM-Version anheben | `release-engineer` | M.3 | S4 zeigt den neuen Stand | ☐ | — |
| M.5 | `git status`, Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | M.4 | `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| M.6 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; danach **M2** (Abzug, Referenz für AKC.7) | **Nutzer** | M.5 | erwartete STM-Version; M2 liegt vor | — | — |
| M.7 | **Vorher-Nachweis, G2** (`design.md` §4.2): Vorbedingung prüfen, sieben `weather_city_set` in rund 1,5 s, ≥ 4 s warten, Original zurück, STM-Reset, Wetterort lesend prüfen. Bei `d = 0`: einmal mit 14 Aufrufen in rund 3 s wiederholen | Lead | M.6, Ent-1 | **AKM.2** (`15·z ≤ ms ≤ 17·z + 10` für jede Zeile — **verfehlt ⇒ Halt**, C3 beginnt nicht); **AKM.3** (`z ≥ 25`, `ms ≥ 400` je Wechsel); `d` berichtet; Wetterort nach dem Reset gleich dem Original. Smoketest; `watch-log.sh` mitgelesen | — | ☐ |

---

## Schritt C3 — STM-Release, seitenweise schreiben (Teil B, Stufe 2)

**Einspielreihenfolge:** nur STM, **nach** M. **Testdurchlauf:** ja (C3.9), weil C3 jeden
EEPROM-Schreibvorgang ändert (AKZ.4).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| C3.1 | **`eeprom_write()` seitenweise** nach `design.md` §2.3: `EEPROM_PAGE_SIZE` mit Datenblattquelle aus V.1 (1) im Kommentar; je Seite **ein** Lesen, **höchstens ein** Schreiben (erster bis letzter Unterschied), **höchstens ein** Wartezyklus; Lesefehler ⇒ ganze Seite schreiben; Schreibfehler ⇒ `return 0`, keine weitere Seite. Messzeile bleibt. Kommentar `:180-200` auf „je Seite" nachgeführt, historische Messung kenntlich. **Prüfstand C3** im Scratchpad nach §2.5, **samt zwei Sabotage-Fassungen** (Seitengrösse 64; Seitenende um eins verschoben) | `stm-developer` | **M.7**, V.1 (1) | **AKC.2–AKC.5** am Prüfstand, mit Fallzahl und Zahl der Vergleiche; Zyklen alt/neu für die S.26- und die Wetterort-Folge im Bericht. Beide Sabotagen melden FEHL. Zeilenende von `eeprom.c` unverändert | ☐ | ☐ |
| C3.2 | **Prüfstand ablegen:** `tools/checks/auszug/stm/c3/`, Eintrag in `auszug.sh` | Lead | C3.1 | OK gegen den Baum; **Gegenprobe:** gegen das M-Release-Tag meldet das Zyklenkriterium (AKC.2) FEHL; beide Sabotagen aus dem Scratchpad melden FEHL (DIR-014). **Zähl nach:** Zahl der Fälle und Vergleiche entspricht der Angabe aus C3.1 | ☐ | — |
| C3.3 | **Flash-Gate, Testbau** nach C3.1: nur F103 | Lead | C3.1 | **AKC.8:** Rest ≥ 1'024 Byte, Zuwachs notiert. Darunter: Halt, Bericht an den Nutzer | — | — |
| C3.4 | **Review STM-Seite, Seitenrechnung** | `firmware-analyst` | C3.2, C3.3 | **AKC.1** (Seitengrösse belegt); Seitenende für jede Startadresse richtig, auch für `start_addr` genau auf einer Seitengrenze und `cnt`, das genau dort endet; kein Überlauf von `start_addr + cnt` in den Zieltypen; Lese- und Schreibfehlerpfade wie AKC.4; Zustand nach Abbruch (Seiten-Präfix) wie `design.md` §2.3 | — | ☐ |
| C3.5 | **Zweites Review, unabhängig** — gleicher Gegenstand, ohne Kenntnis des Berichts aus C3.4 | `code-reviewer` | C3.2, C3.3 | Dieselben Punkte wie C3.4, dazu: Bildet der Prüfstand den Seitenumbruch **wie die Hardware** nach, und würde er einen Überlauf tatsächlich sehen? **Grund für zwei Reviews:** Ein Fehler hier ist Datenverlust in jeder Einstellung (`design.md` §2.4) | — | ☐ |
| C3.5b | STM-Version anheben | `release-engineer` | C3.4, C3.5 | S4 zeigt den neuen Stand | ☐ | — |
| C3.6 | `git status`, Build, **Gegenprobe des Gates** (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | C3.5b | Rest aus S8b neben M.2 und C3.3 gestellt; **unter 1'024 Byte kein Rollout**, Bericht an den Nutzer; `guardrails.sh --full` Exit 0; `auszug.sh` meldet drei Prüfstände mehr als vor dem Paket (AKZ.2) | ☐ | — |
| C3.7 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` (setzt danach zurück) | **Nutzer** | C3.6 | erwartete STM-Version | — | — |
| C3.8 | **Abnahme am Gerät:** (1) **Integrität:** Abzug nach Flash und Reset gegen M2 aus M.6 (`diff-snapshot.sh --soll`). (2) **Nachher-Nachweis, G3**, derselbe Ablauf wie M.7. (3) Smoketest | Lead | C3.7, Ent-1 | **AKC.7 (1)** keine Abweichung ausser flüchtigen Feldern; **AKC.6** `z ≤ 2`, `ms ≤ 60`, `d` unverändert, `diag` lückenlos; **AKC.7 (2)** Wetterort nach dem Reset gleich dem Original. Gerätewerte neben die Prüfstandzahlen aus AKC.5. `watch-log.sh` mitgelesen | — | ☐ |
| C3.9 | **Testdurchlauf Phasen 0–4 und 9**, danach STM-Reset und Phase 9 erneut | `pwa-tester` | C3.8 | **AKZ.4**, **AKC.7 (3):** Nach dem Reset zeigt Phase 9 die Einstellungen so, wie der Durchlauf sie hinterlassen hat. Unvollständig ⇒ **nicht abgenommen** | — | — |

---

## Schritt Z — Abschluss

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: **L338/C43** (erledigt mit Datei:Zeile, G1-Werte), **L339/A58** (erledigt mit G2/G3-Werten und Prüfstandzahlen; ob der Überlauf in G2 nachgestellt wurde oder über S.26 belegt ist), **C3** (erledigt), **neuer Befund (f)** samt ToDo, falls Ent-4 so entschieden, Kennung vom Lead; Instrumentierung nach **Ent-2** (bleibt, oder: Rückbau im nächsten regulären Release, als ToDo); weitere Wert-Ausgaben aus E.2 als neuer Befund; die Antwort zu E.5 (4), falls „ja" | `doc-writer` | C3.9 | S10 läuft durch; jedes „erledigt" mit Datei:Zeile dieses Stands | ☐ | — |
| Z.2 | `knowledge/architecture-checklist.md` §2: „EEPROM-Schreibzugriffe kosten rund 16 ms pro Byte" ⇒ je Schreibzyklus, seit C3 je Seite; „Der RX-Ring ist 256 Byte" ⇒ ohne Zahl, mit Verweis auf `UART_RXBUFLEN` in `src/uart/uart-driver.h` (die Zahl ist veraltet: der Ring hat 1'024) | `doc-writer` | C3.9 | `grep` findet „16 ms pro Byte" und „256 Byte" dort nicht mehr; Sinn der Fragen unverändert | ☐ | — |
| Z.3 | `CLAUDE.md`: „Offene technische Themen" Nr. 2 (C3 umgesetzt, Verweis auf `BEFUNDE.md`), Abschnitt „Hardware" („16 ms pro Byte" ⇒ je Schreibzyklus, seit C3 je Seite) | Lead | C3.9 | S9/S11 laufen durch; keine Versionsnummer neu in der Datei | ☐ | — |

---

## Einspielreihenfolge, fertig zum Einfügen

Der Lead liefert die Zeilen je Schritt mit den Versionen **dieses** Releases:

1. **E:** ESP-OTA (Nutzer), dann `./tools/smoke-device.sh`, `./tools/install-app.sh --check`.
2. **M:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`, dann M2.
3. **C3:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`.

**Nie parallel:** Zwischen den Schritten liegt jeweils der Gerätenachweis (E.9, M.7). Geht einer
daneben, ist die Ursache eindeutig — diese Zusage hängt an der Reihenfolge (R3b).

## Abschluss

- [ ] Alle Tasks erledigt oder mit Grund ausgelassen
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt — oder ausdrücklich als offen
      berichtet
- [ ] Drei Releases (bei V1) gebaut, ausgerollt, committet, getaggt, gepusht; Flash-Umfang je
      Release benannt
