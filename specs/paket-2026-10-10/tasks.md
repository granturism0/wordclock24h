# Tasks — Paket 2026-10-10 „Testbefunde und Altlasten" (A59, A2, A50, A55, C47, C55, C44, B44–B47)

**Erstellt:** 2026-10-10, Stand `675e3fd`; am selben Tag mit den Ergebnissen aus Schritt V
nachgeführt (neuer Schritt S0). Momentaufnahme (DIR-006); der lebende Stand steht in
`BEFUNDE.md`.

**Status: freigegeben** — vom Nutzer am 10.10.2026, samt der Entscheidungen Ent-1 bis Ent-14
(`requirements.md`), alle nach Empfehlung. Die Umsetzung beginnt mit S0.1.

**Entscheidungsvermerk 10.10.2026, soweit er Tasks betrifft:** Ent-5 = ja, S0 ist ein **eigenes
Release** (die Varianten „bei Ent-5 = nein" entfallen) · Ent-3 = **0/4095** (S1.9, T.1) · Ent-4 =
(a) Reset des S2-Flashs (S2.8), (b) STM-Reset nach dem Durchlauf, danach Phase 9 erneut (T.2) ·
Ent-6 = (a) (S1.2) · Ent-7 = F1c, bei fehlender Bestätigung der Zählerquelle auf beiden Zielen im
Review S2.5 F1b (S2.1) · Ent-8 = P3 (S2.2) · Ent-2 = R2, Ent-10 streng, Ent-11 ja, Ent-12 wie
vorgeschlagen (E.1, E.2) · Ent-9 = immer abweisen (P.1) · Ent-13 = kein `SKIP ROM` (S1.2) ·
Ent-14 = Sensor nicht abziehen · Ent-1 = alle schreibenden Gerätenachweise (S1.9, E.8) und
S25/S26 im Durchlauf (T.2) freigegeben.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wo die Dateien disjunkt sind** (R3b). Zwei Ketten
tragen das ganze Paket (`design.md` §0.1):

1. **Code der nächsten Komponente erst nach dem Release-Build der vorigen:** S1.1 nach S0.6,
   S2.1 nach S1.7, E.0 nach S2.7, P.1 nach E.6. Grund: `make release-zip` baut STM, ESP und
   PWA-Assets zusammen — unversionierte Änderungen einer anderen Komponente gingen sonst mit
   alter Versionsnummer ins Fabrikat (Lehre aus Paket 2026-10-09, D mit ESP 3.2.29).
2. **Release-Build erst nach dem Gerätenachweis der vorigen:** S1.7 nach S0.8 (G0), S2.7 nach
   S1.9 (G1), E.6 nach S2.9 (G2), P.9 nach E.8 (G3). Grund: Jede Wirkung muss am Gerät einzeln
   zuordenbar sein.

**Vor jedem Release-Build** prüft der Lead AKZ.3: `git diff --name-only <letztes Release-Tag>`
samt Arbeitsbaum zeigt Produktänderungen **nur** in der Komponente, die dieser Build anhebt.
Sonst **kein** Build.

**`Nutzer` in der Agentenspalte heisst:** Der Lead liefert die Zeile fertig zum Einfügen, der
Nutzer führt sie aus. Er spielt **jedes** Update selbst ein.

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. **Prüfstände
baut der Umsetzer im Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab.**

**Gerätenachweise:** Schreibend nur mit Freigabe (Ent-1) und mit Rückstellung; jeder Gerätelauf
mit `./tools/watch-log.sh`, **vor** dem Einspielen gestartet (DIR-013). Spalten **G** =
Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt V — Vorbereitung — erledigt

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| V.1 | Flash-Kosten von A59/A2/A50/A55, Sparmöglichkeiten ohne Verhaltensänderung, Detailfragen zu A2/A50/A55 | `firmware-analyst` | — | **Erledigt 10.10.2026, gemeldet.** Ergebnisse in `design.md` §1; die 952 Byte von `my_gmtime()` hat der Lead an der Map nachgeprüft. Daraus: Schritt S0, Ent-5, Ent-7, Ent-8, Ent-13; Nebenbefunde für Z.2/Z.3 | — | ☑ |

---

## Schritt S0 — STM-Release: `my_gmtime()` mit 32 Bit (Ent-5)

**Einspielreihenfolge:** nur STM. Entfällt, wenn Ent-5 „nein" lautet — dann kippt die Reihenfolge
in S1/S2 (`design.md` §1.2): A50 zuerst.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S0.1 | **`my_gmtime()`** (`src/base/base.c:450-531`) nach `design.md` §2: Schnittstelle bleibt, intern `uint32_t`, Schleifenlogik und `IS_LEAP_YEAR` wörtlich; prüfen, ob `timeserver.c:250-256` angepasst werden muss (die Map entscheidet). **Prüfstand GT** im Scratchpad | `stm-developer` | Freigabe, Ent-5 | **AKG.1** am Prüfstand, Fallzahl (über 300'000), Abweichungen der **alten** Fassung gegen Host-`gmtime_r()` als Befund berichtet; Sabotage FEHL. Kodierung und Zeilenende von `base.c` festgestellt (DIR-015) | ☐ | — |
| S0.2 | **Flash-Gate und Map:** Testbau F103 | Lead | S0.1 | **AKG.2** (beide Symbole fehlen in der Map; Gegenprobe gegen die Ausgangs-Map schlägt an), **AKG.3** (Rest um ≥ 900 Byte gestiegen, Zahl gemeldet), **AKZ.F** | — | — |
| S0.3 | **Prüfstand GT ablegen** (`tools/checks/auszug/stm/gt/`), Map-Prüfung als Werkzeug | Lead | S0.1 | OK gegen den Baum; Sabotage FEHL; **Zähl nach** | ☐ | — |
| S0.4 | **Review** | `firmware-analyst` | S0.2, S0.3 | Gleiches Ergebnis für jede `uint32`-Eingabe, auch `tm_wday`/`tm_yday`? Kein Überlauf in `day * 24 * 3600` (`:470`, `:497`)? Keine 64-Bit-Stelle mehr im Pfad? Aufrufer unverändert oder begründet geändert? | — | ☐ |
| S0.5 | STM-Version anheben | `release-engineer` | S0.4 | S4 zeigt den neuen Stand | ☐ | — |
| S0.6 | **AKZ.3**, `git status`, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S0.5 | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht | ☐ | — |
| S0.7 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` | **Nutzer** | S0.6 | erwartete STM-Version | — | — |
| S0.8 | **G0**, lesend (`design.md` §10) | Lead | S0.7 | **AKG.4**; Smoketest; 60 min ohne Neustart; `watch-log.sh` mitgelesen | — | — |

---

## Schritt S1 — STM-Release: A59 und A2

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S1.1 | **A59** nach `design.md` §3: `LDR_MIN_VALUE_NUM_VAR`/`LDR_MAX_VALUE_NUM_VAR` (`src/main.c:1876-1886`) setzen bei `val ≤ 4095` und schreiben über `ldr_write_config_to_eep()`; Reihenfolge **nicht** sperren; Debugzeile ohne „readonly". **Prüfstand L59** | `stm-developer` | **S0.6** (bei Ent-5 = nein: Freigabe) | **AKL.1**, **AKL.2** am Prüfstand, Fallzahl; Gegenprobe gegen `release/3.2.25-3.2.30-1.4.94` FEHL. Keine zweite EEPROM-Schreibfunktion (`git diff`). Kodierung und Zeilenende von `main.c` | ☐ | — |
| S1.2 | **A2** nach `design.md` §4: (1) eine CRC-8 für Scratchpad und ROM-ID (`ds18xx.c`); (2) `is_up = 1` erst nach gültiger ROM-ID; (3) `gtemp.index = 255` bei jedem Lese- oder CRC-Fehler (`tempsensor.c`); (4) Logzeile `DS18xxx temperature: ungueltig` (`main.c:4365`); (5) Neuerkennung nach Ent-6 im Zweig `main.c:4348-4376` mit den Zeilen `ds18xx erkannt`/`ds18xx verloren`; (6) `DS18XX_IS_UP` bei jedem Wechsel senden (`vars.c:1430` öffentlich, **keine zweite** Sendefunktion). **Prüfstand DS** | `stm-developer` | S1.1 | **AKT.1–AKT.4** am Prüfstand, Fallzahl (≥ 74 für AKT.1), der L18-Fall (falsche ROM-ID beim Start ⇒ erkannt im nächsten Minutenfenster) ausdrücklich; Gegenprobe gegen den S0-Stand FEHL. Wortlaut der drei Logzeilen wie in `requirements.md`. Kodierung und Zeilenende von `ds18xx.c`, `tempsensor.c`, `vars.c`, `vars.h` | ☐ | — |
| S1.3 | **Flash-Gate**, Testbau F103 **nach S1.1 und nach S1.2** | Lead | S1.1; S1.2 | **AKZ.F**, Zuwachs je Task neben V; darunter **anhalten und vorlegen** (Ent-5, Sparstufen `design.md` §1.2) | — | — |
| S1.4 | **Prüfstände L59 und DS ablegen**; `watch-log.sh` um `ds18xx verloren` und `DS18xxx temperature: ungueltig` erweitern | Lead | S1.2 | Beide OK, Gegenproben FEHL; je Wachmeldung eine eingespielte Zeile, die anschlägt **und ankommt** (DIR-014); Wache danach neu gestartet (L362) | ☐ | — |
| S1.5 | **Review** | `firmware-analyst` | S1.3, S1.4 | **A59:** nur 0..4095, derselbe Schreibweg, Reihenfolge frei, Start- und Laufzeitverhalten unverändert. **A2:** CRC richtig (Polynom, Bitreihenfolge, Startwert) gegen ein Datenblattbeispiel; `is_up` auf **jedem** Pfad richtig; 255 auf jedem Fehlerpfad; Nullbyte-Scratchpad (`design.md` §4.1 ●) beantwortet; `ds18xx_init()` mehrfach aufrufbar (`onewire.c:237-239`); **AKT.5** mit Rechnung; keine neue `watchdog_reload()`-Stelle (S7) | — | ☐ |
| S1.6 | STM-Version anheben | `release-engineer` | S1.5 | S4 zeigt den neuen Stand | ☐ | — |
| S1.7 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S1.6, **S0.8** | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht | ☐ | — |
| S1.8 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` | **Nutzer** | S1.7 | erwartete STM-Version | — | — |
| S1.9 | **G1** (`design.md` §10): (1) **LDR-Bereinigung** mit dem Zielwert aus Ent-3, Freigabe Ent-1: `ldr_min_value_set`, dann `ldr_max_value_set`; (2) **60 min lesend** für A2 | Lead | S1.8 | **AKL.3** (`eep a=225 n=2`, `eep a=227 n=2` in Mitschnitt **und** `/api/stm32_log`; ESP-Kopie = Zielwert; Zielwert als vom Nutzer bestätigt genannt); **AKT.6**; Smoketest; `watch-log.sh` mitgelesen | — | — |

---

## Schritt S2 — STM-Release: A50 und A55

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S2.1 | **A50** nach `design.md` §5 und Ent-7: `fault_reset()` (`src/main.c:590-613`) ohne jede `log_`-Ausgabe; Wartezeit rund 15 s (F1c geeicht oder F1b mit gerechneter Dauer), dann `NVIC_SystemReset()`. **Prüfstand FR** | `stm-developer` | **S1.7** | **AKF.1**, **AKF.2** am Prüfstand; Bericht nennt Zählerquelle bzw. gerechnete Dauer je Ziel; Gegenprobe gegen den S1-Stand FEHL | ☐ | — |
| S2.2 | **A55** nach `design.md` §6 (Ent-8 = P3): im Zweig `CAP var-crc` (`src/esp8266/esp8266.c:543-553`) `esp8266.is_online = 0`, Kommentar mit Begründung und Verweis auf `main.c:3945`/`:3953`. **Prüfstand EP**; W2 muss weiter bestehen | `stm-developer` | S2.1 | **AKP.1**, **AKP.3** am Prüfstand, Fallzahl; Gegenprobe gegen den S1-Stand FEHL. Kodierung und Zeilenende von `esp8266.c` | ☐ | — |
| S2.3 | **Flash-Gate**, Testbau F103 **nach S2.1 und nach S2.2** | Lead | S2.1; S2.2 | **AKF.3**, **AKZ.F** | — | — |
| S2.4 | **Prüfstände FR und EP ablegen**; statische Prüfung „kein `log_` in `fault_reset()`" | Lead | S2.2 | OK; Gegenproben FEHL bzw. schlägt an; W2 weiter 37 Fälle OK | ☐ | — |
| S2.5 | **Review** | `firmware-analyst` | S2.3, S2.4 | **A50:** Reset auf jedem Pfad, auch vor `log_init()`/`watchdog_init()`; Zählerquelle läuft im HardFault auf **beiden** Zielen (F1c) bzw. Dauer gerechnet (F1b); Wartezeit über 12 s und unter 20 s. **A55:** die drei Fragen aus `design.md` §6 ● (SYNCVARS/IPADDRESS nach jedem Start, auch AP-Modus; Dauerzustand bei Zeilenverlust; Reihenfolge `CAP` vor `IPADDRESS`) **am ESP-Code beantwortet**; **AKP.2**, **AKP.5** | — | ☐ |
| S2.6 | STM-Version anheben | `release-engineer` | S2.5 | S4 zeigt den neuen Stand | ☐ | — |
| S2.7 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S2.6, **S1.9** | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht | ☐ | — |
| S2.8 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` — der Reset danach ist der Persistenznachweis für A59 (Ent-4 a) | **Nutzer** | S2.7 | erwartete STM-Version | — | — |
| S2.9 | **G2**, lesend | Lead | S2.8 | **AKL.4** (`numvar[17]/[18]` = Zielwert nach dem Vollabgleich); **AKF.4**; 60 min ohne Exception, ohne Neustart, `v` konstant; Smoketest; `watch-log.sh` mitgelesen | — | — |

---

## Schritt E — ESP-Release: C47, C55, C44

**Einspielreihenfolge:** nur ESP. **Der OTA ist zugleich der Gerätenachweis von A55** — die Wache
muss **vor** dem OTA laufen (E.7).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E.0 | **Vorprüfung C44, lesend:** Wie kam `8,5400` in L340 an Koordinaten — API (ersetzt `,`), Legacy `savelonlat` (ersetzt ebenfalls), Import? Gegen den Mitschnitt bzw. die Abzüge von S.26 | `esp-developer` | **S2.7** | Antwort mit Beleg; ist L340 an dieser Stelle falsch, steht das im Bericht für Z.1 | — | — |
| E.1 | **C47** nach `design.md` §7: neue Maskierung (fünf Zeichen, sonst byte-gleich); **Bestandsliste** aller Wertausgaben der Legacy-Seiten (Datei:Zeile, Gruppe maskiert/Zahl/Konstante); alle Werte maskiert, dazu `new_esp_version`/`new_wc_version`; **Release Notes nach Ent-2**. **Prüfstand t15** | `esp-developer` | E.0 | **AKX.1–AKX.3** am Prüfstand, Fallzahl; Rundlauf byte-gleich; Gegenprobe FEHL. Kodierung (UTF-8) und Zeilenende von `http.cpp` festgestellt | ☐ | — |
| E.2 | **C55 und C44** nach `design.md` §8 (Ent-10, Ent-12): je **eine** Prüffunktion, gerufen von API **und** Legacy (`savedtf`, `savelonlat`); dazu **Ent-11** (Legacy-Anzeige 255 als „invalid"), falls gewählt. **Fall-Tabelle** (eine Datei für ESP und PWA) und **Prüfstand t16** | `esp-developer` | E.1 | **AKR.1–AKR.3** am Prüfstand, mindestens 40 Fälle je Regel; Gegenprobe FEHL | ☐ | — |
| E.3 | **Prüfstände t15 und t16 ablegen**, Fall-Tabelle ablegen; **statische Prüfung AKX.2** mit der Erlaubnisliste aus E.1 | Lead | E.2 | OK; Gegenproben FEHL bzw. schlägt an; **Zähl nach:** geprüfte `http_send`-Aufrufe gegen die Bestandsliste | ☐ | — |
| E.4 | **Review** | `code-reviewer` | E.3 | Maskierung vollständig und ohne Wertschaden (kein `?`, kein UTF-8-Eingriff); jede Wertausgabe erfasst — **eigene Stichprobe** gegen die Bestandsliste; Release Notes auch bei über zwei Zeilen geteiltem Tag und bei Zeilen über 128 Zeichen sicher; Regeln an **allen** Schreibwegen, keine zweite Fassung; Legacy-URLs ohne Parameter ohne `=` (R5) | — | ☐ |
| E.5 | ESP-Version anheben | `release-engineer` | E.4 | S4 zeigt den neuen Stand | ☐ | — |
| E.6 | **AKZ.3**, Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | E.5, **S2.9** | `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| E.7 | **Nutzer:** ESP-OTA — **erst, wenn der Lead die laufende Wache bestätigt hat** | **Nutzer** | E.6 | neue ESP-Version in `/api/update_status` | — | — |
| E.8 | **G3** (`design.md` §10): (1) **A55** aus dem Mitschnitt des OTA; (2) Smoketest **samt Update-Quelle** (DIR-009), `install-app.sh --check` (DIR-017); (3) **C47** Wetterort-Testwert mit Rückstellung (Freigabe Ent-1); (4) **C55/C44** Abweisungen, je ein gültiger Wert mit Rückstellung | Lead | E.7 | **AKP.4** (`v` +0 oder +1; `(CAP var-crc)` und danach `SYNCVARS`/`IPADDRESS`; Vollabgleich vollständig; kein Watchdog-Reset); **AKX.4**; **AKR.4**; Originalwerte zurück und lesend nachgeprüft; `watch-log.sh` mitgelesen | — | — |

---

## Schritt P — PWA-Release: B44–B47, Regeln, Anzeige A2

**Einspielreihenfolge:** nur PWA, **nach** dem ESP (die Regeln aus C55/C44 stehen zuerst im ESP).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P.1 | **B44** (Ent-9) und **B45** nach `design.md` §9.1–§9.2: `parseTimeInput()` ohne Rückfall; `saveTimerRow()`, `saveDfplayerAlarm()`, `saveAllTimerRows()` (zuerst alle prüfen); weitere Aufrufer von `parseTimeInput()` im Bericht; `markEditsPersisted()` leert nur die gespeicherten Felder | `pwa-developer` | **E.6** | **AKB.1**, **AKB.2** (beide Fälle, auch L33) in der Vorschau mit echten Ereignissen; Gegenproben gegen den Ausgangsstand FEHL | ☐ | — |
| P.2 | **B46** und **B47** nach `design.md` §9.3–§9.4: `eeprom_settings` auch bei `networkActive`; Hintergrundklick ohne Fokusverlust, Ziehen aus der Karte schliesst nicht. Braucht B47 Markup: **anhalten**, P.4 anstossen | `pwa-developer` | P.1 | **AKB.3**, **AKB.4** in der Vorschau; Gegenproben FEHL | ☐ | — |
| P.3 | **Regeln C55/C44** nach `design.md` §8 an **allen** PWA-Wegen (Speichern, Karte, Import) aus **derselben** Fall-Tabelle wie E.2; **A2-Anzeige** nach §4.4 (drei Zustände, Text in `app.js` und `i18n/en.json`) | `pwa-developer` | P.2, **E.3** | **AKR.5** (Regelgleichheit gegen die Fall-Tabelle, Gegenprobe FEHL, abweichende Regel ⇒ Meldung kommt an); **AKT.7** in der Vorschau | ☐ | — |
| P.4 | *Nur falls nötig:* Markup für B47 | `ui-developer` | P.2 | wie P.2 | ☐ | — |
| P.5 | **Prüfstand Regelgleichheit ablegen**; Vorschau-Läufe AKB.1–AKB.4 und AKT.7 nachvollziehen | Lead | P.3 (P.4) | OK; Gegenprobe FEHL; S8 (Umlaute) und `innerhtml.mjs` grün | ☐ | — |
| P.6 | **Review PWA** | `code-reviewer` | P.5 | Keine neue `innerHTML`-Stelle; kein leeres `catch`; WLAN-Schlüssel aus `eeprom_settings` an keiner neuen Stelle (AKB.3); B45-Gegenfall L33; `saveAllTimerRows()` sendet bei einer ungültigen Zeile nichts; Regeln wörtlich gleich der Fall-Tabelle | — | ☐ |
| P.7 | **Review UI** | `ui-reviewer` | P.5 | B47: Fokus nach Hintergrundklick, nach Escape, nach Ziehen; Tab-Falle unverändert; A2-Anzeige und B44-Meldung verständlich, Du-Form, echte Umlaute | — | ☐ |
| P.8 | `APP_VERSION` **und** `CACHE_NAME` anheben | `release-engineer` | P.6, P.7 | S4 zeigt den neuen Stand | ☐ | — |
| P.9 | **AKZ.3**, `git status` (R2: still), Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | P.8, **E.8** | `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| P.10 | **Nutzer:** `./tools/install-app.sh --check`, `./tools/install-app.sh` | **Nutzer** | P.9 | neue `APP_VERSION` am Gerät | — | — |
| P.11 | **G4, erster Teil:** `./tools/check-pwa.sh` (DIR-016), Smoketest | Lead | P.10 | **AKB.5**; Smoketest ohne Fehlschlag | — | — |

---

## Schritt T — Testplan und Durchlauf

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T.1 | **`TESTPLAN-PWA.md` nachführen:** (1) S25/S26 **rücknehmbar** über S27 — vorher den Wert lesen (nach G1 ist die ESP-Kopie gleich dem STM), danach zurück; Freigabepflicht entfällt bzw. wie andere rücknehmbare S-Fälle; (2) S27-Soll zusätzlich mit `eep a=225 n=2`/`eep a=227 n=2` im Logring als STM-Beleg; Text „STM verwirft als readonly" entfernt, mit Verweis auf A59; (3) neue Fälle: leeres Zeitfeld (Timer, Ambilight-Timer, DFPlayer, „Alle speichern"), zwei Felder ändern, eines speichern, Modul wechseln (und Gegenfall), AP-SSID ohne Wartungsbesuch, Hintergrundklick im Kartenmodal, Datumsformat- und Koordinatenwerte aus der Fall-Tabelle (abgewiesen und gültig), DS18xx-Anzeige; (4) Abschnitt 12 (Altlasten): der blinde Fleck „STM-seitige LDR-Grenzen" ist mit A59 über S27 und den Reset in T.2 abgedeckt | `doc-writer` | Freigabe, Ent-3, Ent-9, Ent-10, Ent-12 | S9/S11 grün; keine Versionsnummer; jeder neue Fall nennt Instrument und Rückstellung | ☐ | — |
| T.2 | **Testdurchlauf Phasen 0–4 und 9** mit dem Plan aus T.1, danach **STM-Reset** (Ent-4 b, Freigabe) und Phase 9 erneut | `pwa-tester` | P.11, T.1 | **AKZ.6**: S25/S26 gefahren und über S27 zurückgestellt; nach dem Reset zeigt Phase 9 die LDR-Grenzen auf dem Wert vor dem Durchlauf; alle neuen Fälle aus T.1 gefahren. `watch-log.sh` mitgelesen; Mitschnitt ausgewertet (Exceptions, Neustarts, `d`, `v`, `ds18xx …`, `ungueltig`). Unvollständig ⇒ **nicht abgenommen** | — | — |

---

## Schritt Z — Abschluss

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: A59/L351/L364, A2 (mit der neuen Lesart von L18: falsch gelesene ROM-ID, nicht gescheiterte Erkennung), A50/L313, A55/L327, C47/L346, C55/L369, C44/L340 (mit dem Ergebnis aus E.0), B44–B47/L365–L368 je mit Datei:Zeile dieses Stands; S0 als eigener Eintrag (Sparmassnahme, 952 Byte); neue Befunde aus den Reviews; **AKS.6** in der Fassung aus AKP.4 vermerkt | `doc-writer` | T.2 | S10 läuft durch; jedes „erledigt" mit Beleg; jeder neue offene Befund in der ToDo-Liste | ☐ | — |
| Z.2 | `CLAUDE.md`: „Offene technische Themen" 1 (DS18xx) auf den Stand nach S1; die Zeilenangabe „`main.c:3140`" für `temp_init()` (steht bei `:3734`, V) — ohne neue Zeilennummer, wo es geht | Lead | T.2 | S9/S11 grün; keine Versionsnummer neu | ☐ | — |
| Z.3 | `tools/logger/README.md:352`: die beschriebene Fault-Ausgabe gab es nie (V); Stand nach A50 | Lead | S2.9 | Text stimmt mit `fault_reset()` überein | ☐ | — |

---

## Einspielreihenfolge, fertig zum Einfügen

Der Lead liefert die Zeilen je Schritt mit den Versionen **dieses** Releases:

1. **S0:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; danach G0.
2. **S1:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; danach G1 mit der
   LDR-Bereinigung.
3. **S2:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` (der Reset belegt A59); danach G2.
4. **E:** Wache läuft (Lead bestätigt) → ESP-OTA (Nutzer) → `./tools/smoke-device.sh`,
   `./tools/install-app.sh --check`; danach G3.
5. **P:** `./tools/install-app.sh --check`, `./tools/install-app.sh`; danach `./tools/check-pwa.sh`
   und T.2.

**Firmware vor PWA:** Die PWA-Regeln aus C55/C44 bauen auf der ESP-Regel auf, die Anzeige aus A2
auf S1. **Nie parallel:** Zwischen zwei Einspielvorgängen liegt jeweils der Gerätenachweis des
vorigen (R3b).

**Bei Ent-5 = nein** entfällt Zeile 1, und in S1/S2 kommt A50 zuerst (`design.md` §1.2).

## Abschluss

- [ ] Alle Tasks erledigt oder mit Grund ausgelassen
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release
- [ ] AKZ.3 vor jedem Release-Build geprüft und im Bericht
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt — oder ausdrücklich als offen berichtet
- [ ] Fünf Releases (S0, S1, S2, E, P) gebaut, ausgerollt, committet, getaggt, gepusht; Flash-Umfang
      und Reihenfolge je Release benannt
