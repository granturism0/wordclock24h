# Tasks — Paket 2026-10-11 „Nachtfenster, Netzzeit, Spiele, I²C, Brücke" (A23, A69, A53, A57, A60, A61, A62, A63, A35, A21, A34, C35)

**Erstellt:** 2026-10-11, Stand `5c34d80`. Am selben Tag mit den Entscheidungen des Nutzers, der
Teilung nach dem Flash-Gate, dem Review S1 (M1, M2), der vorgezogenen Einsparung N1, dem
Matrix-Lauf, der Auslieferung von S1 und dem Code von S2 nachgeführt. Momentaufnahme (DIR-006);
der lebende Stand steht in `BEFUNDE.md`.

**Status: freigegeben** — vom Nutzer am 11.10.2026 (Ent-16), samt Ent-1 bis Ent-15, Ent-3b,
Ent-14e und Ent-15b. **Zwei Entscheidungen gegen die Empfehlung:** Ent-3 (Ära-Rechnung jetzt) und
Ent-7 (C). **Ent-17 „Release teilen"**, **Ent-18 Matrix-Lauf**.
**Stand:** **S1 ausgeliefert** — STM 3.2.29, Commit `a6065d5`, Tag
`release/3.2.29-3.2.31-1.4.95` gepusht; **G1-Tagteil belegt**, Nachtteil und AKW.5 offen. **S2
(A60) Code fertig, reviewt, nicht gebaut** — der Build folgt erst nach G1. **Offen, vor G3 beim
Nutzer zu bestätigen:** der zweite STM-Reset im Aus-Fenster in G3 (S3.18).

## Entscheidungen (11.10.2026)

| Ent | Gegenstand | Entschieden | wirkt auf |
|---|---|---|---|
| **Ent-1** | A23 Displayfarbe | (a) RAM-Sicherung; DC aktualisiert die Sicherung: **ja**; IR-Farbschritte im Modus 0: **ja** | S3.4 |
| **Ent-2** | A69 Nachtfenster | (1) **nur die Displayliste**; (2) **beim ersten übernommenen `TIME` einmal auswerten**; (3) **ein übersprungenes Ereignis nachholen**; (4) **Startsequenz wie heute, danach aus** — präzisiert nach Review S1 (M2): **„IP erst zeigen, dann aus"** | S1.3, S1.11, 0.5 |
| **Ent-3** | A62 Verfallsdatum 2036 | **Ära-Rechnung jetzt** (gegen die Empfehlung) — Pivot ohne Protokollwechsel, `design.md` §2 | S1.1, E.2 |
| **Ent-3b** | Bestätigungsregel bei Zeitsprung (Folge aus Ent-3) | **Bestätigungsregel wie vorgeschlagen:** bei mehr als 600 s Abweichung und immer nach dem Start gilt eine Zeit erst, wenn eine zweite Antwort nach 15 s sie auf ±5 s bestätigt; höchstens 3 Versuche (`design.md` §2.4). Die Umsetzung mit zusätzlichem Mindestabstand von 10 s liegt im Rahmen (Lead, 11.10.2026) | S1.1 |
| **Ent-4** | A53 Spiele | **V2**, keine Inaktivitätsgrenze | S3.6 |
| **Ent-5** | A35 | **(b)** nur ESP, zählen und melden | E.1 |
| **Ent-6** | C35 | **(A)** nur PWA, Hauptmodul beim Öffnen und alle 5 min; kein 15-s-Takt im Hauptmodul | P.1 |
| **Ent-7** | A57 | **(C)** „Messwert übernehmen" weist eine Inversion ab (gegen die Empfehlung); **nicht** der Zahlen-Setter (A59). A57 wird erst nach G3 geschlossen | S3.8, S3.18, Z.1 |
| **Ent-8** | A61 | Auswerteregel im Werkzeug | 0.1 |
| **Ent-9** | A34 | schliessen, Reste überführen | 0.6 |
| **Ent-10** | A63 | beobachten | 0.6 |
| **Ent-11** | A68 | ja, in S1 | S1.5 |
| **Ent-12** | A67 | ja, STM- und ESP-Hälfte | S3.10, E.4 |
| **Ent-13** | NTP-Puffer und Plausibilität | ja, beides — **nach Ent-3 tragend** | E.2 |
| **Ent-14** | schreibende Gerätenachweise | **alle erteilt:** (a) STM-Reset im Nacht-Aus-Fenster — **der eine Reset nach den Tagtests von S1 (G1)**; (b) Setter während eines Spiels; (c) Setter-Burst im A21-Lauf; (d) Farbanimation, LDR S25/S26 und 32-Byte-Wert vor/nach S2, `pwa-tester`-Durchlauf mit S-Fällen | S1.21, S2.9, S3.18, M.1, T.2 |
| **Ent-14e** | LDR-Abweisungsprobe AKE.2 | **ausdrücklich freigegeben (eigene Freigabe des Nutzers):** Maximum per Zahlen-Setter kurz auf 1 bzw. Minimum auf 4094, danach zurück auf 0/4095 | S3.18 |
| **Ent-15** | A60 Umfang | nur die Fristen | S2.1 |
| **Ent-15b** | A60: Bus beim Fristablauf freitakten (erweitert Ent-15) | **ja** — nach **jeder** Frist, wenn danach BUSY gesetzt ist oder SDA low liest: `i2c_init` neu, das heisst `i2c_reset_bus` mit 9 Takten und Pins zurück auf AF (`design.md` §8). Die Logzeile `i2c: frist abgelaufen` hat der Lead entschieden | S2.1 |
| **Ent-16** | Spec | **freigegeben** | alle |
| **Ent-17** | Flash-Gate nach S1.3 | **„Release teilen":** S1 = A62, A69, A68; A23, A53, A57 (C), A67-STM in **S3** nach einer Flash-Analyse. **Alle übrigen Entscheidungen bleiben** (auch Ent-2 (3), A53 V2, A67). Die Sparstufen aus `design.md` §1.4 sind **nicht** gewählt und greifen nur nach einer neuen Rückfrage. N1 (malloc-Stubs) hat der Lead aus S3.1 in S1 vorgezogen, damit M2 in den Flash passt | S1, S3 |
| **Ent-18** | Komplettbuild aller 72 STM-Varianten | **eigenes Make-Ziel, nur auf Anforderung**, normaler Build unverändert; **erster Lauf vor STM-Release 1**; **Flash-Gate 1'024 Byte für alle 72 Varianten**, für jeden Matrix-Lauf; BlackPill-F411-Varianten mit `startup_stm32f411xe.S` wie im Teilbuild (das Original nimmt `startup_stm32f401xc.S`; Abweichung dokumentiert). Die Teilbuild-Messung je Task bleibt auf F103 WC12h SK6812-RGBW; **geplant wird ab dem Matrix-Minimum** (`design.md` §1.3). **Festlegung des Leads im Rahmen von Ent-18 („auf Anforderung"):** Die Matrix läuft vor **jedem** STM-Release, das Flash kostet — also auch vor S2 und vor S3, je als feste Abhängigkeit des Release-Builds — und zwar **nach der Versionsanhebung**, auf genau dem Stand, der ausgeliefert wird (wie S1.16). Läufe im Testbau zählen **nur als Vorabwert**. Begründung des Leads: rund 1 Minute Aufwand, und das Gate soll den ausgelieferten Stand prüfen | S1.15, S1.16, S1.19, S2.2, S2.6a, S2.7, S3.15a, S3.16, Z.5 |
| **offen** | Zweiter STM-Reset im Aus-Fenster in G3 (A23, EEPROM-Frage) | **vor G3 beim Nutzer zu bestätigen** — nicht durch Ent-14 (a) gedeckt; der Lead fragt vor G3 nach | S3.18 |

**Vom Lead festgelegt:** `CHANGELOG.md` je Release schreibt der **Lead**. ESP-OTA mit
`curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"`. **L391** ist für den NTP-Puffer
reserviert. Die Logzeile **`set_display_power: new=…`** ist **geschützt** (Nachweis in AKW.4) und
keine Einsparung. **Matrix-Lauf vor jedem STM-Release, das Flash kostet, nach der
Versionsanhebung** (Ent-18).

---

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wo die Dateien disjunkt sind** (R3b). Drei
Ketten tragen das Paket (`design.md` §0.1):

1. **Code des nächsten Releases erst nach dem Release-Build des vorigen:** S2.1 nach S1.19, S3.2
   nach S2.7, E.1 nach S3.16, P.1 nach E.9. Grund: `make release-zip` baut STM, ESP und
   PWA-Assets zusammen (Lehre aus Paket 2026-10-09). Die **Flash-Analyse S3.1** ist lesend und
   darf früher beginnen; ihre Zahlen gelten erst gegen die Map des S2-Builds.
2. **Release-Build erst nach dem Gerätenachweis des vorigen:** S2.7 nach S1.21 (G1 **vollständig**,
   also auch Nachtteil und AKW.5), S3.16 nach S2.9 (G2), E.9 nach S3.18 (G3), P.6 nach E.11 (G4).
   Danach M.1 nach P.8 (G5), T.2 nach M.1 (G6). **Jeder STM-Release-Build zusätzlich erst nach
   einem grünen Matrix-Lauf auf dem angehobenen Stand** (S1.16, S2.6a nach S2.5, S3.15a nach
   S3.14).
3. **Innerhalb eines STM-Releases nacheinander, Testbau nach jedem Task** (Flash-Gate, AKZ.F).
   Liegt ein Testbau unter **1'024 Byte**: **anhalten und vorlegen** — keine Sparstufe ohne neue
   Rückfrage (Ent-17).

**Vor jedem Release-Build** prüft der Lead AKZ.3: `git diff --name-only <letztes Release-Tag>`
samt Arbeitsbaum zeigt Produktänderungen **nur** in der Komponente, die dieser Build anhebt —
bei mehreren STM-Releases hintereinander auch **nur** die Tasks **dieses** Releases. Sonst **kein**
Build. Den `CHANGELOG.md`-Eintrag schreibt der **Lead** vor dem Commit.

**Einspielen macht der Lead** — die Zeilen stehen trotzdem je Schritt fertig da (AKZ.8). Jeder
Gerätelauf mit `./tools/watch-log.sh`, **vor** dem Einspielen gestartet (DIR-013). Schreibend nur
im Rahmen von Ent-14 und Ent-14e und mit Rückstellung.

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. **Prüfstände baut
der Umsetzer im Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab** und fährt die
Gegenprobe gegen den Vorgängerstand (DIR-014). Die Map liest der Analyst aus der Datei, die der
Lead ihm nennt.

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — ohne Firmware

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **A61-Auswerteregel als Werkzeug** nach `design.md` §12.1 (Auslöser, Dauer, Δ`v` beide Zahlen, Zeitüberschreitungen, Stillstand); synthetischer Mitschnitt als Prüfstand | Lead | Freigabe | **AKA.1**: Gegenprobe (falscher Auslöser) FEHL mit Exit ≠ 0, Meldung kommt an; **Zähl nach** gegen `grep -c 'info: configuration sent'` | ☐ | ☐ |
| 0.2 | **Review 0.1** | `code-reviewer` | 0.1 | Regeln wie §12.1; „unbekannt" statt geraten; beide Zahlen von `v=`; Exit-Weg der Meldung | — | ☐ |
| 0.3 | **Befunde nachführen, Teil 1:** **L391** NTP-Puffer mit Massnahme in Gruppe C; Berichtigungen an **L331**, **L360**, **L371** (dazu: Ära-Rechnung beschlossen; die Kette endet danach am 31.12.2099 an der RTC), **L233**; Fundstellen A69 (`night.c:155`), A61 (`main.c:4037-4048`), A62 (`main.c:3274`); A53 „mit A16 zusammen" überholt, `snake.c:291`; A21 blinder Fleck ESP-Empfangspuffer; A57 mit dem Vermerk „Ent-7 (C), Schliessung nach G3" — alles nach `design.md` §12.3 | `doc-writer` | Freigabe | S10/S11 grün; jede Berichtigung mit Datei:Zeile von `5c34d80`; keine Versionsnummer in lebender Doku | ☐ | — |
| 0.4 | **Messung A61/A34** mit 0.1 über die vorhandenen Mitschnitte (G3-OTA 10.10.2026 23:06, STM-Reset 11.10.2026 01:58:14, Langzeitmitschnitt) | Lead | 0.2 | **AKA.2**: Tabelle je Abgleich; Entscheidungsregel A34 (über 5 s Stillstand ⇒ Task 10 als neuer Befund) angewandt und benannt | — | — |
| 0.5 | **Testplan V3/S1 um den Nachtzustand** nach `design.md` §12.4 (Ent-2, mit M2): Soll nach einem Reset im Aus-Fenster ab STM-Release 1 = **IP-Lauftext, danach Display aus** (ohne IP spätestens nach 60 s); Rückstellung nach S1 im Aus-Fenster | `doc-writer` | Freigabe | **AKA.4**; S9/S11 grün. **Muss vor dem Nachtteil von S1.21 (G1) fertig sein** | ☐ | — |
| 0.6 | **Befunde nachführen, Teil 2:** A34 schliessen (Ent-9) mit dem Ergebnis aus 0.4 und den überführten Resten; A61 mit dem Messergebnis; A63 als Beobachtung **offen** in der ToDo-Liste (Ent-10); A35 auf „kein hoher Befund". **A57 bleibt offen** bis nach G3 (Ent-7) | `doc-writer` | 0.3, 0.4 | **AKA.3**; S10 grün | ☐ | — |

---

## Schritt S1 — STM-Release 1: A62 (mit Ära), A69, A68, dazu N1, M1, M2

**Ausgeliefert:** STM 3.2.29, Commit `a6065d5`, Tag `release/3.2.29-3.2.31-1.4.95` (gepusht).
**Offen:** Nachtteil von G1 (Reset nach 01:00) und AKW.5 am Morgen.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S1.1 | **A62 mit Ära-Rechnung** nach `design.md` §2 (Ent-3, Ent-3b): eigener Parser (1–10 Ziffern + `'\0'`, Überlaufprüfung ⇒ über 4'294'967'295 verwerfen); Totbereich `[2'208'988'800, 3'976'214'400)` verwerfen; Rechnung NTP→Unix weiter über die `uint32`-Differenz (`timeserver.c:253`), kein 64-Bit-Pfad; Bestätigungsregel nach Ent-3b samt Begrenzung der Schnellabfragen; Logzeilen `time: ungueltig, verworfen` und `time: sprung, bestaetigung angefordert`; Zeitzone beim Laden ±12 (`timeserver.c:67`); kein `strtoul` mehr. **Prüfstand a62** | `stm-developer` | Freigabe, Ent-3b | **AKN.1**–**AKN.3**, **AKN.5** am Prüfstand, Fallzahl; Referenz Python `datetime` **und** gt; Gegenprobe gegen `release/3.2.28-3.2.31-1.4.95` FEHL; Sabotagen „Totbereich entfernt", „genau 10 Ziffern", „Bestätigung entfernt" FEHL. **☑ Ausgeliefert mit 3.2.29.** Zusätzlich gilt ein **Mindestabstand von 10 s** zwischen Kandidat und Bestätigung (strenger als die Regel); `cmd: set time to` wird **unbedingt** geloggt (Beobachtbarkeit für AKN.6). Beides im Rahmen von Ent-3b | ☑ | ☑ |
| S1.2 | **Flash-Gate** nach S1.1; Map-Prüfung: kein `strtoul`, weiter kein `__udivmoddi4`/`__aeabi_ldivmod` (S8b) | Lead | S1.1 | **AKN.4**, **AKZ.F**. **☑ 1'700 Byte frei, +208** (geschätzt −110…−20); `strtoul` weg, keine 64-Bit-Routinen | — | — |
| S1.3 | **A69** nach `design.md` §3 (Ent-2): `night_last_event()` und Hilfsfunktion Wochentag in `night.c`; Auswertung beim Start, beim ersten übernommenen `TIME` (falls die RTC beim Start nicht gültig war) und bei Vorwärtssprüngen (übersprungenes Ereignis); nur die Displayliste; Logzeile `night: start aus (hh:mm)`. **Prüfstand a69** | `stm-developer` | S1.2 | **AKW.1**–**AKW.3**; **Zähl nach:** Tabellen × 10'080; Sabotagen `<=`→`<` und Wochentagsumlauf FEHL; Gleichstandsverhalten benannt. **☑ Ausgeliefert mit 3.2.29** | ☑ | ☑ |
| S1.4 | **Flash-Gate** nach S1.3 | Lead | S1.3 | **AKZ.F**. **☑ 1'132 Byte frei, +568** (geschätzt +160…+310) — Anlass für Ent-17 | — | — |
| S1.5 | **A68** nach `design.md` §4 (Ent-11): Sendefunktion der Ambilight-Power öffentlich, Aufruf an jeder Stelle der **Bestandsliste**; keine zweite Sendefunktion. Prüfstand a69 um das Sendeprotokoll erweitern | `stm-developer` | S1.4 | **AKB.1**; Bestandsliste; Gegenprobe FEHL. **☑ Erledigt** | ☑ | — |
| S1.6 | **Flash-Gate** nach S1.5 | Lead | S1.5 | **AKZ.F**. **☑ 1'088 Byte frei, +44** (geschätzt +10…+30) | — | — |
| S1.7 | **Prüfstände a62 und a69 ablegen**; `watch-log.sh` um `time: ungueltig` (Alarm), `time: sprung` (Hinweis; Alarm bei mehr als drei in Folge), `night: start aus` (Hinweis) | Lead | S1.6 | Beide OK, Gegenproben FEHL; je Wachmeldung eine eingespielte Zeile, die anschlägt **und ankommt** (DIR-014); Wache neu gestartet (L362); gt, EP, W2, L59 weiter OK. **☑ Erledigt; mit dem Release committet** | ☑ | — |
| S1.8 | **Review S1** | `firmware-analyst` | S1.7 | **☑ BESTANDEN 11.10.2026**, mit zwei mittleren Punkten **M1**, **M2** (S1.11) und sechs niedrigen Restpunkten (benannt, kein Fix; `design.md` §3.4) | — | ☑ |
| S1.9 | **N1 — malloc-Stubs** (vom Lead aus S3.1 vorgezogen): starke Definitionen von `_malloc_r`, `_realloc_r`, `_free_r` in `src/stubs/stubs.c` mit `__attribute__((used, externally_visible))` | `stm-developer` | S1.8 | **AKP.0**. **☑ Erledigt, beide Reviews BESTANDEN** | ☑ | ☑ |
| S1.10 | **Flash-Gate** nach S1.9 | Lead | S1.9 | **AKZ.F**. **☑ 1'756 Byte frei, −668** | — | — |
| S1.11 | **M1 und M2 aus dem Review S1** nach `design.md` §3.2: **M1** — Startauswertung nur bei `gmain.year >= 2026`, sonst wertet das erste bestätigte `TIME` aus. **M2** — beim Start im Aus-Fenster nur **vormerken** (`night_off_due`); angewandt **nach dem IP-Lauftext** im Innenzweig der **unveränderten** Restore-Bedingung, ohne IP nach **60 s**; jedes Schalten verwirft die Vormerkung | `stm-developer` | S1.10 | **AKW.2**, **AKW.6**. **☑ Erledigt** | ☑ | — |
| S1.12 | **Flash-Gate** nach S1.11 | Lead | S1.11 | **AKZ.F**. **☑ 1'608 Byte frei, +148** (geschätzt 80–100) | — | — |
| S1.13 | **Nachprüfung M1/M2** | `firmware-analyst` | S1.12 | **☑ BESTANDEN** | — | ☑ |
| S1.14 | Prüfstand a69 um M1/M2 ergänzt ablegen | Lead | S1.13 | **☑ Erledigt** | ☑ | — |
| S1.15 | **Make-Ziel für den Komplettbuild aller 72 STM-Varianten** nach `design.md` §1.6 (Ent-18) | `release-engineer` | S1.14 | **AKZ.M.1**, **AKZ.M.2**. **☑ `make matrix`, `make clean-matrix`**; geändert `CMakeLists.txt`, `Makefile`, neu `cmake/matrix-report.sh`; normaler Build byte-identisch; Gegenproben (Schwelle 2000 ⇒ 3 × GATE, `-O0` baut nicht) je Exit ≠ 0 | ☑ | — |
| S1.16 | **Matrix-Lauf vor S1** | Lead | S1.15 | **AKZ.M.3**. **☑ 72/72, 0 unter dem Gate; engste Variante `wc24h-stm32f103-sk6812-rgbw` 1'416 Byte frei**; vor dem Build wiederholt, gleiches Ergebnis | — | — |
| S1.16a | **Warnungsbestand:** 7 Warnungen `unused parameter` in den malloc-Stubs mit `(void)` beseitigen | `stm-developer` | S1.16 | **☑ Erledigt:** Warnungen weg, Bestand **17 statt 18** | ☑ | — |
| S1.17 | STM-Version anheben | `release-engineer` | S1.13 | **☑ 3.2.29** | ☑ | — |
| S1.18 | `CHANGELOG.md`-Eintrag S1 | Lead | S1.13 | **☑ in `a6065d5` enthalten** | ☑ | — |
| S1.19 | **AKZ.3**, `git status`, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S1.17, S1.18, S1.16, S1.16a | **☑ Commit `a6065d5`, Tag `release/3.2.29-3.2.31-1.4.95` gepusht** | ☑ | — |
| S1.20 | **Einspielen am Tag:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; Smoketest | Lead | S1.19 | **☑ Eingespielt.** Ein Fehlalarm der Logwache beim Flashen (Abbildzeile mit vier Leerzeichen) ist behoben, **L400** | — | — |
| S1.21 | **G1** nach `design.md` §15.1: Tagtests (AKN.6, **AKI.2 vorher**); **STM-Reset im Aus-Fenster** (Ent-14 a) für AKW.4 (IP-Lauftext, danach aus) und AKN.7; am Morgen AKW.5 | Lead | S1.20, **0.5** | **Tagteil ☑:** **AKN.6** — `time: sprung` 20:44:39 → `cmd: set time to` 20:44:54; **AKI.2 vorher** — `eep n=2` 24–26 ms, 32 Byte über 2 Seiten 92 ms; alles zurückgestellt. **Offen:** Nachtteil (AKW.4, AKN.7) nach 01:00, AKW.5 am Morgen | — | — |

---

## Schritt S2 — STM-Release 2 (isoliert): A60 und L360

**Stand:** Code fertig und reviewt, **nicht gebaut** — der Release-Build folgt erst nach **G1
vollständig** und nach dem Matrix-Lauf S2.6a auf dem angehobenen Stand. Engste Matrix-Variante
nach S2 im Testbau (Vorabwert): **1'308 Byte frei**, 72/72 grün.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S2.1 | **A60** nach `design.md` §8 (Ent-15, Ent-15b): ISR-Frage am Code belegt; `i2c_wait_for_flags()` pollt eng mit Frist über `delay_counter` — **Abweichung von §8, am Code begründet:** `delay_counter` ist ein **Abwärtszähler** im SysTick, der bei 0 stehen bleibt, nicht freilaufend. **Beide** BUSY-Schleifen über `i2c_wait_not_busy()` mit `I2C_ERROR_BUSY` (−8). Logzeile **`i2c: frist abgelaufen`** (Lead; nur Debug-UART, nicht `/api/stm32_log`). **Freitakten** (Ent-15b) nach **jeder** Frist in `i2c_handle_timeout()`, wenn danach BUSY gesetzt ist oder SDA low liest: `i2c_last_clockspeed = 0; i2c_init (i2c_clockspeed)` (= `i2c_reset_bus` mit 9 Takten, Pins zurück auf AF) — zuerst nur im BUSY-Pfad, vom Lead nach der Nachprüfung erweitert, weil Folgezugriffe an der SB-Frist scheiterten. **Prüfstand a60** | `stm-developer` | S1.19 | **AKI.1**, **AKI.4**. **☑ Erledigt** | ☑ | — |
| S2.2 | **Flash-Gate** nach S2.1 | Lead | S2.1 | **AKZ.F**. **☑ Teilbuild:** 1'608 → **1'500** (A60) → 1'496 (Freitakten im BUSY-Pfad) → **1'464** (nach jeder Frist). **Matrix im Testbau (Vorabwert): engste Variante 1'308, 72/72 grün** | — | — |
| S2.3 | **Prüfstand a60 ablegen**; Logwache: Schwelle `eep` 200 ms unverändert, `i2c: frist abgelaufen` als Alarm | Lead | S2.2 | **☑ a60** in `tools/checks/auszug/stm/a60/`: **4'998 Prüfungen**; Sabotagen `fristnie`, `busyohne`, `msec`, `ohnelog`, `ohnefrei`, `nurbusy` je FEHL; S16 im Baum **44/44 OK**. **Gegenprobe DIR-014 mit angekommener Meldung:** `./tools/checks/auszug.sh --gegen release/3.2.29-3.2.31-1.4.95` meldet `FEHL  A60 I2C-Fristen …` mit **Exit 1** (41 von 44, 1 fehlgeschlagen) | ☑ | — |
| S2.4 | **Review S2** | `firmware-analyst` | S2.3 | **☑ BESTANDEN, drei Nachprüfungen BESTANDEN.** Benannt (vorbestehend, nicht geändert): `GPIO_RESET_BIT(SDA_PORT, SCL_PIN)` in `i2c.c` trifft auf Nucleo den falschen Port. **Erwartung G2:** `n=2` 16–17 ms, 32 Byte 34–40 ms | — | ☑ |
| S2.5 | STM-Version anheben | `release-engineer` | S2.4 | S4 zeigt den neuen Stand (erwartet 3.2.30) | ☐ | — |
| S2.6 | `CHANGELOG.md`-Eintrag S2 | Lead | S2.4 | Inhalt gleich dem Release; Ent-15b als Entscheidung des Nutzers benannt | ☐ | — |
| S2.6a | **Matrix-Lauf vor S2** (Ent-18, Festlegung des Leads), **nach der Versionsanhebung** auf genau dem Stand, der ausgeliefert wird — wie S1.16 | Lead | **S2.5** | **AKZ.M.4**: 72/72, **alle ≥ 1'024 Byte**, engste Variante benannt. Liegt eine Variante darunter: **kein Build**, anhalten und vorlegen (Ent-17). Der Lauf aus dem Testbau (72/72, engste 1'308) zählt **nur als Vorabwert** | — | — |
| S2.7 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S2.5, S2.6, **S2.6a**, **S1.21 vollständig** | `guardrails.sh --full` Exit 0 mit unverändertem Warnungsbestand; Rest ≥ 1'024; Tag gepusht. **Die Map dieses Builds ist die Grundlage von S3.1** | ☐ | — |
| S2.8 | **Einspielen:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; Smoketest. Einen Wachalarm während des Flashens **am Kontext prüfen** (L400) | Lead | S2.7 | erwartete STM-Version | — | — |
| S2.9 | **G2**: AKI.2 nachher (S25/S26 über S27 zurück, 32-Byte-Wert zurück; Ent-14 d), AKI.3 (60 min lesend). **Messpunkt:** steht `i2c: frist abgelaufen` **vor** `rtc is online`? — das wäre der Hinweis auf die DS1307-Probe beim Start | Lead | S2.8 | **AKI.2** (Vorher 24–26 / 92 ms gegen Nachher; Erwartung 16–17 / 34–40 ms), **AKI.3**; `watch-log.sh` mitgelesen | — | — |

---

## Schritt S3 — STM-Release 3: Flash schaffen, dann A23, A53, A57 (C), A67-STM (Ent-17)

**Einspielreihenfolge:** nur STM, **nach G2**, **vor** E. **Bedarf nach Messfaktor:** Die vier
Punkte sind auf 190–294 Byte geschätzt; mit dem gemessenen Faktor 2 bis 3 (`design.md` §1.2) sind
**rund 400–900 Byte** einzuplanen — gegen einen Spielraum der engsten Matrix-Variante von **284
Byte** nach S2 (1'308 frei). **Die Flash-Analyse muss rund 120–620 Byte schaffen.** N1 ist bereits
in S1 umgesetzt. **Vor dem Release-Build läuft die Matrix auf dem angehobenen Stand** (S3.15a).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S3.1 | **Flash-Analyse** nach `design.md` §1.5: Map des S2-Builds lesen — **auch die der engsten Matrix-Variante** (`wc24h-stm32f103-sk6812-rgbw`); je Kandidat **gemessene** Grösse aus der Map (Symbol, Abschnitt, Bytes), Verhaltensänderung ja/nein, Abhängigkeiten. Kandidaten mindestens: toter Zweig `FILE ` (A52), doppelte und lange Texte, die zweite `log_printf` in `set_display_power()` (am Ende der Funktion), Debugtexte im normalen Bau, weitere Bibliotheksmodule nach dem Muster von N1, was wc24h zusätzlich belegt. **Geschützt:** `set_display_power: new=…`, `i2c: frist abgelaufen` (Wachalarm, AKI.3). **Nicht mehr Kandidat:** N1. **Lesend** — darf vor S2.7 beginnen, die Zahlen gelten gegen die S2-Map | `firmware-analyst` | S2.7 (für die Zahlen) | **AKP.1**: Liste mit Bytes laut Map, Summe gegen den Bedarf von rund 120–620 Byte, Vorschlag der Reihenfolge | — | ☐ |
| S3.2 | **Einsparungen umsetzen**, je Einsparung einzeln, in der Reihenfolge aus S3.1; nur verhaltensneutrale. Eine Einsparung mit Verhaltensänderung oder an einer Zeile, nach der ein Werkzeug, der Testplan oder ein AK sucht, **nur nach Rückfrage beim Nutzer** | `stm-developer` | S3.1, **S2.7** | **AKP.2**; je Einsparung ein Testbau (S3.3); alle bestehenden Prüfstände weiter OK | ☐ | — |
| S3.3 | **Testbau je Einsparung** | Lead | jede Einsparung aus S3.2 | **AKP.2**: gemessener Gewinn je Einsparung neben der Map-Grösse aus S3.1 | — | — |
| S3.4 | **A23** nach `design.md` §5 (Ent-1). **Prüfstand a23** | `stm-developer` | S3.3 | **AKD.1**; Gegenprobe FEHL; kein EEPROM-Schreiben im Animationspfad. Kodierung und Zeilenende von `display.c` | ☐ | — |
| S3.5 | **Flash-Gate** nach S3.4 | Lead | S3.4 | **AKZ.F** | — | — |
| S3.6 | **A53 V2** nach `design.md` §6 (Ent-4). **Prüfstand a53** | `stm-developer` | S3.5 | **AKS.1**, **AKS.2**; Gegenprobe FEHL; ob `.` zählt, begründet. Kodierung und Zeilenende von `tetris.c`, `snake.c` | ☐ | — |
| S3.7 | **Flash-Gate** nach S3.6 | Lead | S3.6 | **AKZ.F** | — | — |
| S3.8 | **A57 (C)** nach `design.md` §5a (Ent-7). **Zahlen-Setter (A59) unverändert.** **Prüfstand a57** | `stm-developer` | S3.7 | **AKE.1**; Gegenprobe gegen 3.2.28 FEHL; L59 besteht weiter. Kodierung und Zeilenende von `ldr.c` | ☐ | — |
| S3.9 | **Flash-Gate** nach S3.8 | Lead | S3.8 | **AKZ.F** | — | — |
| S3.10 | **A67 STM-Hälfte** nach `design.md` §7 (Ent-12): neue Zeile (Wortlaut **hier** festgelegt, Vorschlag `OTA start`), exakter Vergleich, setzt nur `is_online = 0`. **Prüfstand a67** | `stm-developer` | S3.9 | **AKK.1**; Gegenprobe FEHL; EP besteht weiter. Wortlaut im Bericht für E.4 | ☐ | — |
| S3.11 | **Flash-Gate** nach S3.10 | Lead | S3.10 | **AKZ.F** | — | — |
| S3.12 | **Prüfstände a23, a53, a57, a67 ablegen**; `watch-log.sh` um `spiel: … verworfen` und `ldr: … abgewiesen` (Hinweise); geänderte Logzeilen aus S3.2 in Wache und Werkzeugen nachgeführt | Lead | S3.11 | Alle OK, Gegenproben FEHL; a62, a69, a60, gt, EP, W2, L59 weiter OK | ☐ | — |
| S3.13 | **Review S3** | `firmware-analyst` | S3.12 | Einsparungen verhaltensneutral, geschützte Zeilen unverändert; A23, A53, A57, A67 wie in `design.md`; keine neue `watchdog_reload()`-Stelle (S7); Architektur-Checkliste | — | ☐ |
| S3.14 | STM-Version anheben | `release-engineer` | S3.13 | S4 zeigt den neuen Stand (erwartet 3.2.31) | ☐ | — |
| S3.15 | `CHANGELOG.md`-Eintrag S3 | Lead | S3.13 | Inhalt gleich dem Release; Ent-7 als Entscheidung des Nutzers benannt | ☐ | — |
| S3.15a | **Matrix-Lauf vor S3** (Ent-18, Festlegung des Leads), **nach der Versionsanhebung** auf genau dem Stand, der ausgeliefert wird — wie S1.16 | Lead | **S3.14** | **AKZ.M.5**: 72/72, **alle ≥ 1'024 Byte**, engste Variante benannt. Liegt eine Variante darunter: **kein Build**, anhalten und vorlegen (Ent-17). Läufe aus Testbauten zählen **nur als Vorabwert** | — | — |
| S3.16 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S3.14, S3.15, **S3.15a**, **S2.9** | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht | ☐ | — |
| S3.17 | **Einspielen am Tag:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; Smoketest. Vorher lesend `dspcolor[0]`, `numvar` 17/18, LDR-Rohwert, `numvar[8]` festhalten | Lead | S3.16 | erwartete STM-Version; Vorher-Werte im Bericht | — | — |
| S3.18 | **G3** nach `design.md` §15.2: Tagtests (AKD.2 Teil 1, AKS.3, **AKE.2** (Ent-14e)); **zweiter STM-Reset im Aus-Fenster** für AKD.2 Teil 2 und erneut AKW.4 — **vor G3 beim Nutzer zu bestätigen** | Lead | S3.17, **Bestätigung des zweiten Resets** | **AKD.2**, **AKS.3**, **AKE.2**; ohne Bestätigung bleibt AKD.2 Teil 2 offen und wird so berichtet | — | — |

---

## Schritt E — ESP-Release: A35, NTP (tragend), `ovr=`, A67-ESP

**Einspielreihenfolge:** nur ESP, **nach G3**. Wache läuft **vor** dem OTA.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E.1 | **A35 (b)** nach `design.md` §9. **Prüfstand a35** | `esp-developer` | **S3.16** | **AKQ.1**; Gegenprobe FEHL; **Zähl nach**. Kodierung und Zeilenende von `vars.cpp`, `ESP-uclock.ino` (**gemischt**) | ☐ | — |
| E.2 | **NTP** nach `design.md` §10.1 (Ent-13, **tragend**). **Keine** Ära- oder Pivot-Rechnung im ESP. **Prüfstand ntp** | `esp-developer` | E.1 | **AKU.1**, **AKU.2**; Gegenprobe (Wächterbyte, Transmit 0) FEHL | ☐ | — |
| E.3 | **`ovr=`** nach `design.md` §10.2 | `esp-developer` | E.2 | **AKO.1** | ☐ | — |
| E.4 | **A67 ESP-Hälfte** nach `design.md` §7: Zeile aus S3.10 **wörtlich** vor dem Download in allen drei OTA-Pfaden, `SYNCVARS` bei Fehlschlag | `esp-developer` | E.3, S3.10 | **AKK.2**; Kodierung (UTF-8) und Zeilenende von `http.cpp` | ☐ | — |
| E.5 | **Prüfstände a35 und ntp ablegen**; statische Prüfung AKK.2; `watch-log.sh` um `var ignoriert`, `ntp verworfen`, `ovr=` > 0 | Lead | E.4 | OK, Gegenproben FEHL bzw. schlägt an; Wachmeldungen kommen an | ☐ | — |
| E.6 | **Review E** | `code-reviewer` | E.5 | wie `design.md` §9, §10, §7 | — | ☐ |
| E.7 | ESP-Version anheben | `release-engineer` | E.6 | S4 zeigt den neuen Stand (erwartet 3.2.32) | ☐ | — |
| E.8 | `CHANGELOG.md`-Eintrag E | Lead | E.6 | Inhalt gleich dem Release | ☐ | — |
| E.9 | **AKZ.3**, Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | E.7, E.8, **S3.18** | `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| E.10 | **Einspielen:** Wache läuft (bestätigt) → `curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"` → `./tools/smoke-device.sh` (samt Update-Quelle), `./tools/install-app.sh --check` | Lead | E.9 | neue ESP-Version; Update-Quelle richtig (DIR-009); PWA-Dateien vollständig (DIR-017) | — | — |
| E.11 | **G4**: Mitschnitt des OTA (Δ`v`, Vorher +2, L380), danach 60 min lesend | Lead | E.10 | **AKQ.2/AKU.3/AKO.2** | — | — |

---

## Schritt P — PWA-Release: C35

**Einspielreihenfolge:** nur PWA, **nach G4** (Firmware zuerst, DIR-002).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P.1 | **C35 (A)** nach `design.md` §11 (Ent-6). **Prüfstand c35** | `pwa-developer` | **E.9** | **AKC.1**; Gegenprobe FEHL; Kodierung und Zeilenende von `app.js` | ☐ | — |
| P.2 | **Prüfstand c35 ablegen** | Lead | P.1 | OK, Gegenprobe FEHL; S8 und `innerhtml.mjs` grün | ☐ | — |
| P.3 | **Review P** | `code-reviewer` | P.2 | Keine neue `innerHTML`-Stelle, kein leeres `catch`; Zeitpunkt des letzten Abrufs auf jedem Pfad gesetzt; Moduswechsel holt sofort | — | ☐ |
| P.4 | `APP_VERSION` **und** `CACHE_NAME` anheben | `release-engineer` | P.3 | S4 zeigt den neuen Stand (erwartet 1.4.96) | ☐ | — |
| P.5 | `CHANGELOG.md`-Eintrag P | Lead | P.3 | Inhalt gleich dem Release | ☐ | — |
| P.6 | **AKZ.3**, `git status` (R2: still), Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | P.4, P.5, **E.11** | `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| P.7 | **Einspielen:** `./tools/install-app.sh --check`, `./tools/install-app.sh` | Lead | P.6 | neue `APP_VERSION` am Gerät | — | — |
| P.8 | **G5**: `./tools/check-pwa.sh` (DIR-016), Smoketest, 15 min Hauptmodul offen mit Zählung im Mitschnitt | Lead | P.7 | **AKC.2** | — | — |

---

## Schritt M — Abschlusslauf A21

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| M.1 | **Abschlusslauf (G6)** nach `design.md` §13: 30 min, drei Tabs (Haupt-, System-, Updatemodul), Wetterabrufe, **Setter-Burst** 14 × Wetterort mit Rückstellung (Ent-14 c); Logwache parallel; Auswertung mit 0.1 | Lead | **P.8** | **AKM.1**, **AKM.2**: Urteil „A21 schliessen" oder „neu fassen" mit Begründung | — | — |

---

## Schritt T — Testplan und Durchlauf

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T.1 | **`TESTPLAN-PWA.md` nachführen** (über 0.5 hinaus): S47 mit Soll „Farbe nach Animation aus gleich vorher" (A23); Fall Spiel mit Setter (A53); S25/S26 mit dem Hinweis auf A57 (C); Hauptmodul-Abrufzahl (C35); V3-Auswertung um `night: start aus`, `time: sprung`, `i2c: frist abgelaufen`, `spiel: … verworfen`, `ldr: … abgewiesen`, `var ignoriert`, `ntp verworfen`, `ovr=`; Logzeilen, die S3.2 geändert hat | `doc-writer` | Freigabe; für den letzten Teil S3.13 | S9/S11 grün; keine Versionsnummer; jeder neue Fall mit Instrument und Rückstellung | ☐ | — |
| T.2 | **Testdurchlauf Phasen 0–4 und 9** mit dem Plan aus 0.5 und T.1 (Ent-14 d) | `pwa-tester` | **M.1**, T.1 | **AKZ.6**; `watch-log.sh` mitgelesen; Mitschnitt ausgewertet. Unvollständig ⇒ **nicht abgenommen** | — | — |

---

## Schritt Z — Abschluss

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: A62 (Ära, neue Grenze 2099), A69 samt M1/M2 und den **sechs niedrigen Restpunkten** aus Review S1 (`design.md` §3.4) als benannte Befunde, A68, N1 (S1); A60 samt Freitakten (Ent-15b), L359, L360 (S2) und der **vorbestehende Nucleo-Befund** aus Review S2.4 (`GPIO_RESET_BIT(SDA_PORT, SCL_PIN)` trifft den falschen Port) als neuer Befund; A23, A53, **A57 schliessen mit G3-Beleg**, A52 falls in S3.2 umgesetzt (S3); A35, L391 (E); C35 (P); A21 nach M.1; A67 mit offenem Gerätenachweis (nächster ESP-OTA); die Flash-Lehre (Schätzungen Faktor 1,5 bis 3 daneben; engste Matrix-Variante rund 156–192 Byte unter dem Teilbuild); der Hinweis zur A57-Abweisung in der PWA (`design.md` §5a) als neuer Befund; ToDo-Liste nachgeführt | `doc-writer` | T.2 | S10 grün; jedes „erledigt" mit Beleg | ☐ | — |
| Z.2 | `knowledge/architecture-checklist.md`: I²C-Zeit je Byte auf den in G2 **gemessenen** Stand | `doc-writer` | S2.9 | S9/S11 grün | ☐ | — |
| Z.3 | `CLAUDE.md`, Abschnitt Hardware: I²C-Wartezeit je Byte auf den Stand nach A60 | Lead | S2.9 | S9/S11 grün; keine Versionsnummer neu | ☐ | — |
| Z.4 | **Spec-Endstand** | `spec-writer` | Z.1 | Jede Abweichung offen benannt | — | — |
| Z.5 | `README-CMAKE.md`: das Matrix-Ziel (`make matrix` / `make clean-matrix`, 72 Varianten, Gate 1'024 Byte für alle) und die **Abweichung beim F411-Startup**; dazu zwei Berichtigungen im Abschnitt Release-ZIP | `doc-writer` | S1.15 | **AKZ.M.1** (Dokumentationsteil); S9/S11 grün. **Stand: in Arbeit, vorgezogen** | ☐ | — |

---

## Einspielreihenfolge, fertig zum Einfügen

Der Lead spielt ein; vor **jedem** Schritt läuft `./tools/watch-log.sh`.

1. **S1** — **☑ eingespielt** (3.2.29); offen: G1-Nachtteil mit dem **eigenen STM-Reset im
   Aus-Fenster** (Ent-14 a) und AKW.5.
2. **S2 (nach G1 vollständig und Matrix S2.6a auf dem angehobenen Stand):**
   `./tools/flash-stm.sh --check` · `./tools/flash-stm.sh` · `./tools/smoke-device.sh` — danach G2.
3. **S3 (nach G2 und Matrix S3.15a auf dem angehobenen Stand, am Tag):**
   `./tools/flash-stm.sh --check` · `./tools/flash-stm.sh` · `./tools/smoke-device.sh` — danach G3;
   der **zweite STM-Reset im Aus-Fenster nur nach Bestätigung durch den Nutzer**.
4. **E (nach G3):** Wache bestätigt · `curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"` ·
   `./tools/smoke-device.sh` · `./tools/install-app.sh --check` — danach G4.
5. **P (nach G4):** `./tools/install-app.sh --check` · `./tools/install-app.sh` ·
   `./tools/check-pwa.sh` · `./tools/smoke-device.sh` — danach G5, M (G6), T.

**Firmware vor PWA**, STM vor ESP. **Nie parallel:** Zwischen zwei Einspielvorgängen liegt jeweils
der Gerätenachweis des vorigen (R3b).

## Abschluss

- [ ] Alle Tasks erledigt oder mit Grund ausgelassen
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release
- [ ] AKZ.3 vor jedem Release-Build geprüft und im Bericht
- [ ] Flash-Gate nach jedem STM-Task und nach jeder Einsparung gemessen und berichtet
- [x] Matrix-Lauf vor S1 grün (72/72, alle ≥ 1'024 Byte; engste Variante 1'416)
- [ ] Matrix-Lauf vor S2 und vor S3 grün, je nach der Versionsanhebung (S2.6a, S3.15a)
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt — oder ausdrücklich als offen berichtet
- [ ] Fünf Releases (S1, S2, S3, E, P) gebaut, ausgerollt, committet, getaggt, gepusht — **S1 ☑**
