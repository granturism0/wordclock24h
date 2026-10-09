# Tasks — Paket 2026-10-09 „Brückenlast" (L338/C43, L339/A58, Prüfsumme ESP→STM)

**Erstellt:** 2026-10-09, Stand `dea187a`; am selben Tag dreimal nachgeführt (Teil D; dann V.1
und Ent-5 bis Ent-8; zuletzt das Ergebnis des Reviews E.5 — Nachtrag M1 in E.1, Vermerk in E.3,
E.4, E.5, E.9, D.8, Z.1). **Am 10.10.2026 nachgeführt** mit dem Stand, wie ihn die Umsetzer
gemeldet haben: E.9 (ESP 3.2.27/3.2.28, G1), M.1, M.1b, M.2 (☑), M.2b (☑), Prüfpunkte für M.3,
Toleranz in M.7, neu **M.7b** (Messpaket „Stabilität ESP"). Momentaufnahme (DIR-006).

**Status: freigegeben, alle Entscheidungen getroffen** (Ent-1 bis Ent-8, dazu die Entscheidungen
zum Review E.5, `design.md` §7). V ist erledigt.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wo die Dateien disjunkt sind** (R3b). Die
wichtigsten Abhängigkeiten: **M.1 beginnt erst nach G1 (E.9)**, damit Vorher und Nachher von L339
unter demselben ESP laufen. **D.1 beginnt erst nach dem C3-Release-Build (C3.6)**, der D-Release-Build
erst nach G3 (C3.8). **Jeder Release-Build nimmt den ganzen Arbeitsbaum mit** — kein Release-Build,
solange ein Produktcode-Task eines anderen Schritts offen ist. **Der Nachtrag M1 zu E.1 muss vor
E.6 abgeschlossen und sein Prüfstandsteil abgelegt sein** (E.6 hängt deshalb zusätzlich an E.1
und E.3).

**`Nutzer` in der Agentenspalte heisst:** Der Lead liefert die Zeile fertig zum Einfügen, der
Nutzer führt sie aus. Er spielt **jedes** Update selbst ein.

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash` (L238, L269).
**Prüfstände baut der Umsetzer im Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab.**

**Gerätenachweise:** G1 bis G3 sind freigegeben (Ent-1); G4 und die Wetterabrufe nach M sind
lesend bzw. Abrufe wie in G1. Jeder Gerätelauf mit `./tools/watch-log.sh` (DIR-013).

---

## Schritt V — Vorbereitung — erledigt

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| V.1 | Acht Fragen (Seitengrösse, periodische Schreiber, `log_init`, Flash, ESP-Core, STM ohne `WEATHER`, `CMD`-Absender und Reset-/Flash-Pfade, Präfix `CMC`) | `firmware-analyst` | — | **Erledigt 09.10.2026.** Ergebnisse in `design.md` §6; (1) durch Ent-7 entschieden; (7) geht als Liste an E.2b | — | ☑ |

---

## Schritt E — ESP-Release (A1, Teil C, Teil D ESP-Seite)

**Einspielreihenfolge:** nur ESP. **Teil D wirkt hier noch nicht** (kein STM kündigt `0x04` an).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E.1 | **A1 — L338 und L-Befund 200 ms:** `query_weather()` (`weather.cpp:307-427`) nach `design.md` §1.2–§1.4: (1) **eine** Lesehilfe, liest solange `available () \|\| connected ()`, gibt in jedem leeren Durchlauf die Kontrolle ab; (2) `delay (200)` (`:359`) und `if (available ())` (`:361`) entfallen — eine Antwort nach mehr als 200 ms wird gelesen; (3) Sicherheitsnetz `WEATHER_TOTAL_TIMEOUT_MS` = **5'000 ms**, **DNS und Verbindung getrennt** begrenzt (`WiFi.hostByName (…, rest)`, `connect (ip, 80)`, `setTimeout (rest)` vor `print ()`), `stop ()` (rund 300 ms) eingerechnet; (4) Messzeile `- weather fc=… ms=… <ok\|fehler\|dns\|connfail\|timeout\|leer>` auf jedem Pfad, `snprintf` in Stackpuffer, über `Serial` **und** `stm32_log_append ()`; (5) **Endzeile**: genau eine von `WEATHER`/`WEATHER_FC`/`WICON`/`WICON_FC`, sonst `ERROR weather <ergebnis>`. Kommentar an der Konstante: hängt an der A2-Frist im STM (6 s). **Prüfstand t12** im Scratchpad (§1.5). **Nachtrag 09.10.2026 (Review E.5, M1, Entscheidung des Nutzers „jetzt mitnehmen"):** (6) **neues Ergebnis `fehler`** nach `design.md` §1.4: `parse_weather()` und `parse_weather_fc()` melden über ihren **Rückgabewert** drei Fälle — keine Endzeile / Endzeile in Erfolgsform (Zweig `cod == 200`) / Endzeile mit Fehler (`… Error <cod>`, `… Parse Error`); `weather_read_answer()` bildet daraus `ok`, `fehler` bzw. `leer` — **kein Textvergleich** der gesendeten Zeile; (7) die **Endzeilen bleiben byte-gleich** mit `95ba131`, Reihenfolge der Zweige unverändert; (8) Kommentare mit der Ergebnisliste (`weather.cpp`, Kopf von `query_weather()`) um `fehler` ergänzt; (9) **t12 um AKE.6b erweitert** (echte Parser, Fälle nach AKE.6b), Ablage durch den Lead in E.3. **Nicht** Teil des Nachtrags: eine Längengrenze in `weather_read_line()` (M2, ausgeklammert) | `esp-developer` | V.1 | **AKE.1–AKE.6** am Prüfstand, Fallzahl gemeldet; **gegen das alte `weather.cpp` fehlgeschlagen** (Körper ohne `\n` ≥ 5'000 ms; Antwort nach 250 ms ungeparst und ohne Endzeile). Signatur von `hostByName` mit Frist und Verhalten von `connect (ip, …)` an der Core-Quelle belegt. Kodierung und Zeilenende von `weather.cpp` vor dem Patch (AKZ.8). **Nachtrag M1:** **AKE.6b** am Prüfstand, Fallzahl gemeldet; **gegen `95ba131` fehlgeschlagen** (cod 401 und „ohne `cod`" ⇒ dort `ok`); `git diff` zeigt in den Parsern keine geänderte `Serial`-Ausgabe, nur Rückgabewerte; `grep` in `weather_read_answer()`/`query_weather()` findet keinen Vergleich auf `Error`/`Parse` | ☐ | ☐ |
| E.2 | **Teil C — Echo:** **eine** Funktion für die Echo-Zeile, gerufen von POST (`http.cpp:14071-14079`) **und** GET (`:14129-14143`); Methode und Pfad, Querystring ⇒ `?...` (drei ASCII-Punkte, Nachtrag Review E.5). **Prüfstand t13.** **Zusätzlich:** weitere Stellen in `http.cpp`, die Anfragewerte über `Serial` ausgeben — **gemeldet, nicht mitkorrigiert** | `esp-developer` | E.1 | **AKE.8** am Prüfstand t13; `grep` findet kein `Serial.print*` mit `sRequest`/`sParam` mehr. Kodierung (UTF-8) und Zeilenende von `http.cpp` festgestellt | ☐ | ☐ |
| E.2b | **Teil D, ESP-Seite** nach `design.md` §5.2, §5.3, §5.6: (1) **ein** Kommandoabsender für **alle** Formen aus V.1 (7) — ohne Fähigkeit byte-gleich `CMD …\r\n`, mit Fähigkeit `CMC <nutzlast>*hhhh\r\n`; (2) `var_crc()` aus `ESP-uclock.ino` sichtbar, **keine zweite Rechnung** (ohne `static`, L177); (3) Flag `0x04` in `var_frame_line()` setzt, Eröffnung ohne `0x04` löscht; nachgesendete Eröffnung desselben Abgleichs ändert nichts; (4) Fähigkeit **vor** jedem STM-Reset und STM-Flash aus V.1 (7) löschen; (5) jeder Wechsel als Zeile im Logring (`stm32_log_append()`). **Prüfstand t14** | `esp-developer` | E.2, V.1 (7) | **AKD.1–AKD.4** am Prüfstand: Fallzahl der Kommandoformen gleich V.1 (7); jede Form ohne Fähigkeit byte-gleich; Marken gegen `tools/checks/var-crc.c`; **gegen den Ausgangsstand fehlgeschlagen**. Kodierung und **Zeilenenden** von `vars.cpp`, `udpsrv.cpp`, `stm32flash.cpp` (UTF-8) und `ESP-uclock.ino` (**gemischt**, byte-genau patchen) festgestellt | ☐ | ☐ |
| E.3 | **Prüfstände ablegen:** t12, t13, t14 unter `tools/checks/auszug/esp/`, Einträge in `auszug.sh`; **zwei statische Prüfungen:** kein `Serial.print*` mit `sRequest`/`sParam` in `http.cpp`; kein `Serial.print*` einer `CMD`-Zeile ausserhalb des Absenders. **Nach dem Nachtrag M1 erneut für t12:** erweiterten t12 ablegen | Lead | E.2b; **für t12 erneut: E.1 (Nachtrag M1)** | Drei Prüfstände mehr, alle OK. **Gegenprobe** gegen den Ausgangsstand: t12 und t14 FEHL, beide statischen Prüfungen schlagen an (DIR-014); t13 dort nicht übersetzbar, **nicht** gezählt. Meldung kommt bei Exit ≠ 0 an. **Nachtrag M1:** t12 OK gegen den Baum, **Gegenprobe gegen `95ba131` FEHL** an den AKE.6b-Fällen; Fallzahl von t12 gegenüber `95ba131` um die AKE.6b-Fälle gestiegen (**zähl nach**) | ☐ | — |
| E.4 | **Werkzeug:** `tools/watch-log.sh` meldet `- weather` mit Ergebnis ≠ `ok` oder `ms` ≥ 1'000, `eep … ms=` ≥ 200 (ab M) und `cmd abgewiesen` (ab D). `grep` über `tools/**`, `.claude/**`, `app.js`: braucht etwas die volle `- request`-Zeile oder das Präfix `(CMD `? **Vermerk Review E.5 (M1):** Für das neue Ergebnis `fehler` braucht `watch-log.sh` **keine Änderung** — es meldet jedes Ergebnis ausser `ok` (`watch-log.sh:149-150`). Nur der Kommentar `:147` nennt die Liste noch ohne `fehler` | Lead | — | Gegenprobe je Meldung mit eingespielter Zeile (DIR-014); `grep`-Ergebnis im Bericht; wertet ein Werkzeug `(CMD ` aus, wird es um `(CMC ` erweitert | ☐ | — |
| E.5 | **Review ESP-Seite** — **bestanden 09.10.2026, M1 nachgezogen (E.1, Nachtrag), M2 als Befund** (ausgeklammert, `requirements.md` „Nicht Teil dieser Änderung"; Eintrag in Z.1). Vermerkt: N3 (Suchtext `Faehigkeit an`), N4 (`CMD` nach reinem ESP-Neustart gewollt), Marker `?...` — `design.md` §7 | `code-reviewer` | E.3 | Vier Punkte, dazu: (1) Lesehilfe bei **jedem** Ende richtig, Kontrolle abgegeben? (2) Gleiche Zeichenkette an den Parser? (3) **DNS, Verbindung, Lesen und `stop ()` zusammen ≤ 5'100 ms**, ohne doppelt wirkende Frist? (4) **Genau eine Endzeile** auf jedem Pfad? (5) Längengrenze gegen einen feindlichen Server nötig — **Antwort mit Begründung**? (6) Kein Wert in Mess- und Echo-Zeile? (7) Beide Echo-Stellen dieselbe Funktion? (8) **D:** kein `CMD`-Absender ausserhalb; Fähigkeit auf **allen vier** Wegen gelöscht und nur über `0x04` gesetzt; dieselbe `var_crc()`? **Die Antwort steht am Code** | — | ☑ |
| E.6 | ESP-Version anheben | `release-engineer` | E.5, **E.1 (Nachtrag M1)**, **E.3 (t12 erneut)** | S4 zeigt den neuen Stand | ☐ | — |
| E.7 | `git status`, Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | E.6, E.4 | `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| E.8 | **Nutzer:** ESP einspielen | **Nutzer** | E.7 | neue ESP-Version in `/api/update_status` | — | — |
| E.9 | **Abnahme am Gerät, G1** (`design.md` §4.1): Smoketest **samt Update-Quelle** (DIR-009), `install-app.sh --check` (DIR-017); je **ein** `weather_get_now` und `weather_get_forecast`; **60 Minuten lesend**. **Stand 10.10.2026 (gemeldet):** **ESP 3.2.27 brachte einen Rückschritt** (L353): Der Wetterkörper wurde nach dem ersten TCP-Segment abgeschnitten. Die Korrektur kam mit **ESP 3.2.28**. **G1 lief unter 3.2.28:** Wetter **67 ms**, Vorhersage **98 ms**, beide `ok`, **kein `v`-Anstieg**. **Das 60-Minuten-Fenster wertet der Lead aus** — offen | Lead | E.8 | **AKE.7** (Messzeile `ok`, `ms` < 1'000, ±150 ms gegen `weather`→`WEATHER`, kein `v`-Anstieg; über 60 min Median und grösster Wert, typisch < 1 s; ein `fehler` wird mit der Endzeile berichtet, ist kein Fehlschlag von A1); **AKE.9** (keine `- request`-Zeile mit `=`, Zahl > 0); **AKD.10, erster Teil** (nur `CMD`, Eröffnungen ohne `0x04`, keine Logzeile mit dem Suchtext **`Faehigkeit an`** — Umschrift, N3). L327-Anstiege getrennt. `watch-log.sh` mitgelesen. **Stand 10.10.2026:** Einzelabrufe erfüllt (s. links); ±150-ms-Abgleich, AKE.9, AKD.10 (erster Teil) und das 60-Minuten-Fenster nicht gemeldet | — | ☐ |

---

## Schritt M — STM-Release: Messzeile (Teil B) und Wetterwarten (A2)

**Warum zusammen:** `design.md` §1.6 — A2 wirkt nur in `var_send_buf()` während eines
Wetterabrufs, die Messzeile nur in `eeprom_write()`; G2 stösst keinen Abruf an.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| M.1 | **Messzeile in `eeprom_write()`** (`src/eeprom/eeprom.c:201-240`) nach `design.md` §2.2: `eep a=… n=… z=… ms=… d=…/…`, bei `z ≥ 1`, **ohne weitere Schwelle**; **Zeitbasis unabhängig von `eeprom_waitstates()`/`eeprom_ms_tick`** (Vorschlag `diag_tick_cnt` über einen Lesezugriff aus `src/main.c`, umgerechnet über `F_INTERRUPTS`); kein Inhaltsbyte; `d` über `esp8266_uart_rxdrops()`. **Keine Verhaltensänderung.** Füllstand nur bei höchstens rund 40 Byte. **Stand 10.10.2026 (gemeldet, ✔ am Code `eeprom.c:205-267`, `main.c:655-675`):** umgesetzt; Zeitbasis `diag_tick_cnt` über `diag_ticks()` und `diag_ticks_per_ms`, Auflösung **66,7 µs**, auf ganze ms **abgeschnitten** ⇒ AKM.2 mit 1 ms Toleranz nach unten; **kein Füllstand**. ● `rtc` jetzt mit 1 vorbelegt (`eeprom.c:221`) — Prüfpunkt für M.3 | `stm-developer` | **E.9** | **AKM.1**, **AKM.2** am Quelltext; Bericht nennt Zeitbasis und ihre Auflösung, Kodierung und Zeilenende von `eeprom.c` und `main.c`. `git diff` zeigt ausserhalb der Messzeile keine Änderung an der Schreiblogik | ☐ | ☐ |
| M.1b | **A2 — Wetterwarten** nach `design.md` §1.6: `weather_query()` (`src/weather/weather.c:213-285`) merkt den Anstoss; `var_send_buf()` (`src/vars/vars.c:501-704`) gibt während eines laufenden Abrufs erst auf, wenn die normalen 3 s **und** 6 s ab dem Anstoss abgelaufen sind; die Endzeile (`ESP8266_WEATHER`, `_WEATHER_FC`, `_WEATHER_ICON`, `_WEATHER_FC_ICON`, `ESP8266_ERROR`) beendet den Abruf; nach der Frist wird der Zustand gelöscht; ein zweiter Anstoss verlängert nicht; **echte** Frist mindestens 6 s (Zeitbasis im Bericht). **Keine neue `watchdog_reload()`-Stelle.** **Prüfstand W2** im Scratchpad (§1.7). **Stand 10.10.2026 (gemeldet, ✔ am Code `vars.c:484-538`, `:568-569`, `:656-695`, `weather.c:253`, `:287`, `main.c:3214`):** umgesetzt. (1) **Lücke im Entwurf geschlossen:** Die Endzeile kommt unmittelbar **vor** dem `.` des wartenden Kommandos; das wartende Kommando hält den gesehenen Anstoss lokal fest und behält seine Frist, die Endzeile löscht den Zustand nur für **folgende** Kommandos. Folge: Schweigt der ESP nach der Endzeile, wartet das laufende Kommando bis 7 s statt 3 s. (2) **Obergrenze 7 Sekundenschritte je Warten** ab dem eigenen Start; ein verschachtelter Anstoss verlängert nicht, er bekommt nur die Restzeit. (3) **Zeitbasis `uptime`**, `VAR_WEATHER_WAIT_SEC` = 7, echte Frist 6 bis 7 s (am Prüfstand 6'001 bis 7'000 ms) | `stm-developer` | M.1 | **AKW.1–AKW.3** am Prüfstand, Fallzahl gemeldet; **gegen den E-Stand fehlgeschlagen** (Timeout nach 3 s). S7 zeigt denselben Bestand. Kodierung und Zeilenende von `weather.c` und `vars.c` festgestellt. **Stand 10.10.2026:** AKW.1, AKW.2 am Prüfstand W2 erfüllt (37 Fälle, Gegenprobe FEHL); AKW.3 offen bis M.3 | ☐ | ☐ |
| M.2 | **Flash-Gate, Testbau** nach M.1 und nach M.1b: nur F103. **☑ erledigt 10.10.2026 (gemeldet):** Testbau F103 **1'420 Byte frei** (vorher 1'688), **Zuwachs 268 Byte**; Gate (≥ 1'024) **bestanden**. Für C3 und D bleiben 396 Byte über der Reserve (`design.md` §5.7) | Lead | M.1b | **AKM.4:** Rest ≥ 1'024 Byte nach jedem der beiden Teile, Zuwächse neben V.1 (4) und §5.7. Darunter: **anhalten und vorlegen** | — | — |
| M.2b | **Prüfstand W2 ablegen:** `tools/checks/auszug/stm/w2/`, Eintrag in `auszug.sh`. **☑ erledigt 10.10.2026 (gemeldet):** abgelegt unter `tools/checks/auszug/stm/w2/`, **37 Fälle**; Gegenprobe gegen `release/3.2.22-3.2.28-1.4.94` **FEHL**, Kernfall „Timeout nach 3000 ms" | Lead | M.1b | OK gegen den Baum; Gegenprobe gegen das E-Release-Tag FEHL (DIR-014) | ☑ | — |
| M.3 | **Review STM-Seite** | `firmware-analyst` | M.2, M.2b | Messzeile: Schreiblogik **unverändert**; Zeitbasis **unabhängig** von `eeprom_waitstates()` — sonst ist AKM.2 leer; kein Inhaltswert; kein Aufruf vor `log_init()`. **A2:** Abrufzustand auf **jedem** Pfad gelöscht (Endzeile, Frist, ESP-Neustart mitten im Abruf); kein Weg, auf dem das verlängerte Warten über 6 s ab dem ersten Anstoss hinausgeht; keine neue `watchdog_reload()`-Stelle; verschachtelter Aufruf (`var_send_nested`) unverändert. **Nachtrag 10.10.2026:** „über 6 s" heisst jetzt **über 7 Sekundenschritte**, sowohl ab dem ersten Anstoss als auch ab dem eigenen Start. Zusätzlich: (a) Hält das wartende Kommando den Anstoss **vor** dem Abholen der Nachricht fest, sodass die Endzeile seine Frist nicht beendet (`vars.c:658-662`)? (b) Ist gewollt, dass der Zeitüberschreitungs-Abbruch (`vars.c:691`) auch den Zustand eines erst **während** des Wartens verschachtelt angestossenen Abrufs löscht (`design.md` §1.6, ●)? (c) Ist die Vorbelegung `rtc = 1` in `eeprom_write()` (`eeprom.c:221`) als Beseitigung undefinierten Verhaltens bei `cnt == 0` vertretbar, obwohl M.1 „keine Verhaltensänderung" verlangt? | — | ☐ |
| M.4 | STM-Version anheben | `release-engineer` | M.3 | S4 zeigt den neuen Stand | ☐ | — |
| M.5 | `git status`, Build, **Gegenprobe des Gates** (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | M.4 | Rest aus S8b neben M.2; `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| M.6 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; danach **M2** (Referenz für AKC.7) | **Nutzer** | M.5 | erwartete STM-Version; M2 liegt vor | — | — |
| M.7 | **Vorher-Nachweis, G2** (`design.md` §4.2): Vorbedingung, sieben `weather_city_set` in rund 1,5 s, ≥ 4 s warten, Original zurück, STM-Reset, Wetterort prüfen; bei `d = 0` einmal mit 14 Aufrufen. **Dazu Wetterabrufe** (§4.3): je ein `weather_get_now` und `weather_get_forecast`, **60 Minuten lesend** | Lead | M.6 | **AKM.2** (**`15·z − 1 ≤ ms ≤ 17·z + 10`** — 1 ms Toleranz nach unten wegen des Abschneidens auf ganze ms, Nachtrag 10.10.2026; **verfehlt ⇒ Halt**); **AKM.3** (`z ≥ 25`, `ms ≥ 400`); `d` berichtet; Wetterort nach dem Reset gleich dem Original; **AKW.4** (kein `v`-Anstieg nach einem Abruf, kein Watchdog-Reset). Fiel ein automatischer Abruf in den Burst, G2 wiederholt. Smoketest; `watch-log.sh` mitgelesen | — | ☐ |
| M.7b | **Messpaket „Stabilität ESP"** — vom Nutzer am 10.10.2026 dazugenommen. Läuft **nach dem M-Flash, zusammen mit G2**, über `tools/mess-stabilitaet.sh` (C9, C9c2, C9e, A28). **Nicht gleichzeitig mit dem Burst von G2:** Parallele Anfragen und die Helligkeitsänderung der Phase 4 (C9/L169) erzeugen selbst `d`, `eep`-Zeilen und Brückenlast — gleichzeitig wäre AKM.3 nicht mehr zuordenbar (R3b). **Phase 4 schreibt** (Helligkeit ändern, danach zurück): Die Freigabe des Nutzers (R5) muss sie ausdrücklich umfassen | Lead | M.6 | **keine neue Exception**; **keine Ausfälle bei bis zu 8 parallelen Anfragen**; **Heap-Verlauf mit offener PWA berichtet**; **`d` berichtet**. `watch-log.sh` mitgelesen (DIR-013) | — | ☐ |

---

## Schritt C3 — STM-Release, seitenweise schreiben (Teil B, Stufe 2)

**Einspielreihenfolge:** nur STM, **nach** M. Der `pwa-tester`-Durchlauf folgt nach D und deckt
C3 mit; wird D am Flash-Gate angehalten, läuft er hier nach C3.8.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| C3.1 | **`eeprom_write()` seitenweise** nach `design.md` §2.3: `EEPROM_PAGE_SIZE` als **Build-Konstante je Ziel — 32 auf F411, 8 auf F103** (Ent-7), über das bestehende Zielmakro; je Seite **ein** Lesen, **höchstens ein** Schreiben, **höchstens ein** Wartezyklus; Lesefehler ⇒ Seite schreiben; Schreibfehler ⇒ `return 0`, keine weitere Seite. Messzeile bleibt. Kommentar `:180-200` nachgeführt. **Prüfstand C3** (§2.5) **für beide Seitengrössen**, **samt drei Sabotagen** (Firmware 64 gegen Baustein 32; Firmware 32 gegen Baustein 8; Seitenende um eins verschoben) | `stm-developer` | **M.7** | **AKC.1–AKC.5** am Prüfstand, Fallzahl je Seitengrösse und Zahl der Vergleiche; Zyklen alt/neu für S.26- und Wetterort-Folge bei P = 32 und P = 8. Alle drei Sabotagen FEHL. Zeilenende von `eeprom.c` unverändert | ☐ | ☐ |
| C3.2 | **Prüfstand ablegen:** `tools/checks/auszug/stm/c3/`, Eintrag in `auszug.sh` (beide Seitengrössen in einem Lauf) | Lead | C3.1 | OK gegen den Baum; **Gegenprobe** gegen das M-Release-Tag: Zyklenkriterium FEHL; Sabotagen FEHL. **Zähl nach:** Fälle und Vergleiche je Seitengrösse wie in C3.1 angegeben | ☐ | — |
| C3.3 | **Flash-Gate, Testbau** nach C3.1: **F103 und F411** (beide Seitengrössen übersetzen) | Lead | C3.1 | **AKC.8:** F103-Rest ≥ 1'024 Byte, Zuwachs notiert; F411 baut. Darunter: **anhalten und vorlegen** | — | — |
| C3.4 | **Review STM-Seite, Seitenrechnung** | `firmware-analyst` | C3.2, C3.3 | **AKC.1** (Seitengrösse je Ziel, richtiges Makro, kein Ziel ohne Wert); Seitenende für jede Startadresse richtig, auch auf einer Seitengrenze; kein Überlauf von `start_addr + cnt`; Puffergrösse folgt P; Fehlerpfade wie AKC.4; Zustand nach Abbruch | — | ☐ |
| C3.5 | **Zweites Review, unabhängig**, ohne Kenntnis des Berichts aus C3.4 | `code-reviewer` | C3.2, C3.3 | Dieselben Punkte, dazu: Bildet der Prüfstand den Umbruch **wie die Hardware** nach, für **beide** Seitengrössen, und würde er einen Überlauf sehen? | — | ☐ |
| C3.5b | STM-Version anheben | `release-engineer` | C3.4, C3.5 | S4 zeigt den neuen Stand | ☐ | — |
| C3.6 | `git status`, Build, **Gegenprobe des Gates** (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | C3.5b | Rest aus S8b neben M.2 und C3.3; **unter 1'024 Byte kein Rollout**; `guardrails.sh --full` Exit 0 | ☐ | — |
| C3.7 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` | **Nutzer** | C3.6 | erwartete STM-Version | — | — |
| C3.8 | **Abnahme am Gerät (F411):** (1) Integrität: Abzug nach Flash und Reset gegen M2 (`diff-snapshot.sh --soll`). (2) **G3**, derselbe Ablauf wie M.7 (ohne die Wetterabrufe). (3) Smoketest | Lead | C3.7 | **AKC.7 (1)**; **AKC.6** (`z ≤ 2`, `ms ≤ 60`, `d` unverändert, `diag` lückenlos); **AKC.7 (2)**. Gerätewerte neben die Prüfstandzahlen für P = 32; die F103-Werte (`z ≤ 5`) stehen nur aus dem Prüfstand im Bericht. `watch-log.sh` mitgelesen | — | ☐ |

---

## Schritt D — STM-Release, Prüfsumme ESP→STM (eigenes Release, Ent-6)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| D.1 | **Teil D, STM-Seite** nach `design.md` §5.3, §5.4, §5.6: (1) `var_crc()` aus `src/vars/vars.c` sichtbar (`vars.h`), **keine zweite Rechnung**; (2) Eröffnung setzt **immer** `0x04`; (3) Zweig `CMC ` in `esp8266.c`: strenge Hexprüfung, Rechnung, bei Gleichheit Marke ab und **derselbe** Weg wie `CMD`; sonst nicht anwenden, Zähler sättigend, gedrosselte Zeile `cmd abgewiesen #<n> len=<l>` über `log_printf()`; (4) **N1 (Ent-5):** Vollabgleich über `var_sync_pending` vormerken, **höchstens einer je 60 s**. **Prüfstand D** (§5.8) | `stm-developer` | **C3.6** | **AKD.5–AKD.8** am Prüfstand, Fallzahl gemeldet; **gegen den C3-Stand fehlgeschlagen** („`CMC` richtig ⇒ angewandt"); Fall „alter STM" gegen den Ausgangsstand **besteht**. Kodierung und Zeilenende von `esp8266.c`, `vars.c`, `vars.h`, `main.c`. Kein neues `diag`-Feld | ☐ | ☐ |
| D.2 | **Flash-Gate, Testbau** nach D.1: nur F103 | Lead | D.1 | **AKD.9:** Rest ≥ 1'024 Byte, Zuwachs neben V.1 (4). Darunter: **anhalten und vorlegen**, mit den Sparstufen aus §5.7 | — | — |
| D.3 | **Prüfstand ablegen:** `tools/checks/auszug/stm/d/`, Eintrag in `auszug.sh` | Lead | D.1 | OK; Gegenprobe gegen das C3-Release-Tag FEHL (DIR-014); `auszug.sh` meldet insgesamt **sechs** Prüfstände mehr als vor dem Paket (AKZ.2) | ☐ | — |
| D.4 | **Review STM-Seite, Teil D** | `firmware-analyst` | D.2, D.3 | Vier Punkte, dazu: Kann `CMC` auf **irgendeinem** Pfad ohne stimmende Marke angewandt werden? Hexprüfung streng (nicht `htoi()`)? Dieselbe `var_crc()`, gegen `tools/checks/var-crc.c`? `0x04` in **jeder** Eröffnung? Längste Nutzlast samt Marke ≤ `ESP8266_MAX_CMD_LEN`? Kein Wert in der Abweisungszeile? N1-Drossel auf jedem Pfad, kein Abgleich während eines laufenden? **Gegen E.2b:** gleiche Rechnung, gleiches Präfix | — | ☐ |
| D.5 | STM-Version anheben | `release-engineer` | D.4, **C3.8** | S4 zeigt den neuen Stand | ☐ | — |
| D.6 | `git status`, Build, **Gegenprobe des Gates** (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | D.5 | Rest aus S8b neben M.2, C3.3, D.2; **unter 1'024 Byte kein Rollout**; `guardrails.sh --full` Exit 0 | ☐ | — |
| D.7 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; danach **M2** | **Nutzer** | D.6 | erwartete STM-Version; M2 liegt vor | — | — |
| D.8 | **Abnahme am Gerät, G4 Teil 2**, lesend: Eröffnung mit `0x04`; Logzeile mit dem Suchtext **`Faehigkeit an`** im Logring (Umschrift, N3); Smoketest; `diff-snapshot.sh --soll` gegen den Abzug aus D.7 | Lead | D.7 | **AKD.10, zweiter Teil** bis auf den Durchlauf; keine Abweisungszeile. Ein `CMD`-Fenster nach einem reinen ESP-Neustart ist gewollt (N4) und wird getrennt berichtet. `watch-log.sh` mitgelesen | — | ☐ |
| D.9 | **Testdurchlauf Phasen 0–4 und 9**, danach STM-Reset und Phase 9 erneut | `pwa-tester` | D.8 | **AKZ.4**; **AKC.7 (3)**; **AKD.10**: jede Speicheraktion als `(CMC …*hhhh)`, Zahl der `CMC`- und `CMD`-Zeilen gemeldet, **null** Abweisungen. Unvollständig ⇒ **nicht abgenommen** | — | — |

---

## Schritt Z — Abschluss

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: **L338/C43** (erledigt: A1 und A2, G1-Werte, Median der Abrufdauer), **L-Befund 200 ms** (neu, Nummer vom Lead, mit E.1 erledigt), **L339/A58** (erledigt, G2/G3-Werte, Prüfstandzahlen für P = 32 und P = 8), **C3** (erledigt), **Befund Teil D** (neu, Familie A32/L42, mit diesem Paket geschlossen, Ent-4/Ent-5), **beide Messzeilen bleiben** (Ent-2); als **neue, offene** Befunde: weitere Wert-Ausgaben aus E.2, die STM-eigene Ausgabe `(CMD …)`/`(CMC …)` mit Werten auf der Log-UART, die Längenrechnung der `diag`-Zeile ohne `a=`, **Befund M2** (Review E.5: keine Längengrenze je Zeile in `weather_read_line()`; Heap-Erschöpfung nur über MITM/DNS-Fälschung; keine Regression; Voraussetzung einer Grenze ist eine Messung der grössten echten Vorhersageantwort am Gerät — Nummer vom Lead; darf der Lead schon vorher eintragen) | `doc-writer` | D.9 | S10 läuft durch; jedes „erledigt" mit Datei:Zeile dieses Stands; jeder neue offene Befund in der ToDo-Liste | ☐ | — |
| Z.2 | `knowledge/architecture-checklist.md` §2: „16 ms pro Byte" ⇒ je Schreibzyklus, seit C3 je Seite; „Der RX-Ring ist 256 Byte" ⇒ ohne Zahl, Verweis auf `UART_RXBUFLEN` in `src/uart/uart-driver.h`. §4: Frage, ob ein neues Kommando ESP→STM über den zentralen Absender läuft | `doc-writer` | D.9 | `grep` findet „16 ms pro Byte" und „256 Byte" dort nicht mehr | ☐ | — |
| Z.3 | `CLAUDE.md`: „Offene technische Themen" Nr. 2 (C3 umgesetzt), Abschnitt „Hardware" („16 ms pro Byte") | Lead | D.9 | S9/S11 laufen durch; keine Versionsnummer neu | ☐ | — |

---

## Einspielreihenfolge, fertig zum Einfügen

Der Lead liefert die Zeilen je Schritt mit den Versionen **dieses** Releases:

1. **E:** ESP-OTA (Nutzer), dann `./tools/smoke-device.sh`, `./tools/install-app.sh --check`.
2. **M:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`, dann M2; danach G2 (M.7) und das
   Messpaket „Stabilität ESP" (M.7b), nicht gleichzeitig mit dem Burst.
3. **C3:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`.
4. **D:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`, dann M2.

**Firmware-Reihenfolge:** ESP vor STM. A2 (M) wirkt auch mit dem alten ESP-Wetterpfad, erkennt
dessen Ende dann aber nur an `WEATHER`/`ERROR` — deshalb nach E. D-STM gegen einen ESP ohne D wäre
harmlos, aber ohne Wirkung. **Nie parallel:** Zwischen den Schritten liegt jeweils der
Gerätenachweis (E.9, M.7, C3.8). Geht einer daneben, ist die Ursache eindeutig (R3b).

## Abschluss

- [ ] Alle Tasks erledigt oder mit Grund ausgelassen
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt — oder ausdrücklich als offen
      berichtet
- [ ] Vier Releases gebaut, ausgerollt, committet, getaggt, gepusht; Flash-Umfang je Release
      benannt
