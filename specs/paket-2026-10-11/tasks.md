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

**Endstand 13.10.2026:** Abschnitt „Endstand 13.10.2026" am Schluss. Die Haken in den Spalten G und R
sind dort begründet; der Text oberhalb hält den Stand vom 11.10.2026 fest.

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
| 0.1 | **A61-Auswerteregel als Werkzeug** nach `design.md` §12.1 (Auslöser, Dauer, Δ`v` beide Zahlen, Zeitüberschreitungen, Stillstand); synthetischer Mitschnitt als Prüfstand | Lead | Freigabe | **AKA.1**: Gegenprobe (falscher Auslöser) FEHL mit Exit ≠ 0, Meldung kommt an; **Zähl nach** gegen `grep -c 'info: configuration sent'` | ☑ | ☑ |
| 0.2 | **Review 0.1** | `code-reviewer` | 0.1 | Regeln wie §12.1; „unbekannt" statt geraten; beide Zahlen von `v=`; Exit-Weg der Meldung | — | ☑ |
| 0.3 | **Befunde nachführen, Teil 1:** **L391** NTP-Puffer mit Massnahme in Gruppe C; Berichtigungen an **L331**, **L360**, **L371** (dazu: Ära-Rechnung beschlossen; die Kette endet danach am 31.12.2099 an der RTC), **L233**; Fundstellen A69 (`night.c:155`), A61 (`main.c:4037-4048`), A62 (`main.c:3274`); A53 „mit A16 zusammen" überholt, `snake.c:291`; A21 blinder Fleck ESP-Empfangspuffer; A57 mit dem Vermerk „Ent-7 (C), Schliessung nach G3" — alles nach `design.md` §12.3 | `doc-writer` | Freigabe | S10/S11 grün; jede Berichtigung mit Datei:Zeile von `5c34d80`; keine Versionsnummer in lebender Doku | ☑ | — |
| 0.4 | **Messung A61/A34** mit 0.1 über die vorhandenen Mitschnitte (G3-OTA 10.10.2026 23:06, STM-Reset 11.10.2026 01:58:14, Langzeitmitschnitt) | Lead | 0.2 | **AKA.2**: Tabelle je Abgleich; Entscheidungsregel A34 (über 5 s Stillstand ⇒ Task 10 als neuer Befund) angewandt und benannt | — | — |
| 0.5 | **Testplan V3/S1 um den Nachtzustand** nach `design.md` §12.4 (Ent-2, mit M2): Soll nach einem Reset im Aus-Fenster ab STM-Release 1 = **IP-Lauftext, danach Display aus** (ohne IP spätestens nach 60 s); Rückstellung nach S1 im Aus-Fenster | `doc-writer` | Freigabe | **AKA.4**; S9/S11 grün. **Muss vor dem Nachtteil von S1.21 (G1) fertig sein** | ☑ | — |
| 0.6 | **Befunde nachführen, Teil 2:** A34 schliessen (Ent-9) mit dem Ergebnis aus 0.4 und den überführten Resten; A61 mit dem Messergebnis; A63 als Beobachtung **offen** in der ToDo-Liste (Ent-10); A35 auf „kein hoher Befund". **A57 bleibt offen** bis nach G3 (Ent-7) | `doc-writer` | 0.3, 0.4 | **AKA.3**; S10 grün | ☑ | — |

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
| S2.5 | STM-Version anheben | `release-engineer` | S2.4 | S4 zeigt den neuen Stand (erwartet 3.2.30) | ☑ | — |
| S2.6 | `CHANGELOG.md`-Eintrag S2 | Lead | S2.4 | Inhalt gleich dem Release; Ent-15b als Entscheidung des Nutzers benannt | ☑ | — |
| S2.6a | **Matrix-Lauf vor S2** (Ent-18, Festlegung des Leads), **nach der Versionsanhebung** auf genau dem Stand, der ausgeliefert wird — wie S1.16 | Lead | **S2.5** | **AKZ.M.4**: 72/72, **alle ≥ 1'024 Byte**, engste Variante benannt. Liegt eine Variante darunter: **kein Build**, anhalten und vorlegen (Ent-17). Der Lauf aus dem Testbau (72/72, engste 1'308) zählt **nur als Vorabwert** | — | — |
| S2.7 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S2.5, S2.6, **S2.6a**, **S1.21 vollständig** | `guardrails.sh --full` Exit 0 mit unverändertem Warnungsbestand; Rest ≥ 1'024; Tag gepusht. **Die Map dieses Builds ist die Grundlage von S3.1** | ☑ | — |
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
| S3.1 | **Flash-Analyse** nach `design.md` §1.5: Map des S2-Builds lesen — **auch die der engsten Matrix-Variante** (`wc24h-stm32f103-sk6812-rgbw`); je Kandidat **gemessene** Grösse aus der Map (Symbol, Abschnitt, Bytes), Verhaltensänderung ja/nein, Abhängigkeiten. Kandidaten mindestens: toter Zweig `FILE ` (A52), doppelte und lange Texte, die zweite `log_printf` in `set_display_power()` (am Ende der Funktion), Debugtexte im normalen Bau, weitere Bibliotheksmodule nach dem Muster von N1, was wc24h zusätzlich belegt. **Geschützt:** `set_display_power: new=…`, `i2c: frist abgelaufen` (Wachalarm, AKI.3). **Nicht mehr Kandidat:** N1. **Lesend** — darf vor S2.7 beginnen, die Zahlen gelten gegen die S2-Map | `firmware-analyst` | S2.7 (für die Zahlen) | **AKP.1**: Liste mit Bytes laut Map, Summe gegen den Bedarf von rund 120–620 Byte, Vorschlag der Reihenfolge | — | ☑ |
| S3.2 | **Einsparungen umsetzen**, je Einsparung einzeln, in der Reihenfolge aus S3.1; nur verhaltensneutrale. Eine Einsparung mit Verhaltensänderung oder an einer Zeile, nach der ein Werkzeug, der Testplan oder ein AK sucht, **nur nach Rückfrage beim Nutzer** | `stm-developer` | S3.1, **S2.7** | **AKP.2**; je Einsparung ein Testbau (S3.3); alle bestehenden Prüfstände weiter OK | ☑ | — |
| S3.3 | **Testbau je Einsparung** | Lead | jede Einsparung aus S3.2 | **AKP.2**: gemessener Gewinn je Einsparung neben der Map-Grösse aus S3.1 | — | — |
| S3.4 | **A23** nach `design.md` §5 (Ent-1). **Prüfstand a23** | `stm-developer` | S3.3 | **AKD.1**; Gegenprobe FEHL; kein EEPROM-Schreiben im Animationspfad. Kodierung und Zeilenende von `display.c` | ☑ | — |
| S3.5 | **Flash-Gate** nach S3.4 | Lead | S3.4 | **AKZ.F** | — | — |
| S3.6 | **A53 V2** nach `design.md` §6 (Ent-4). **Prüfstand a53** | `stm-developer` | S3.5 | **AKS.1**, **AKS.2**; Gegenprobe FEHL; ob `.` zählt, begründet. Kodierung und Zeilenende von `tetris.c`, `snake.c` | ☑ | — |
| S3.7 | **Flash-Gate** nach S3.6 | Lead | S3.6 | **AKZ.F** | — | — |
| S3.8 | **A57 (C)** nach `design.md` §5a (Ent-7). **Zahlen-Setter (A59) unverändert.** **Prüfstand a57** | `stm-developer` | S3.7 | **AKE.1**; Gegenprobe gegen 3.2.28 FEHL; L59 besteht weiter. Kodierung und Zeilenende von `ldr.c` | ☑ | — |
| S3.9 | **Flash-Gate** nach S3.8 | Lead | S3.8 | **AKZ.F** | — | — |
| S3.10 | **A67 STM-Hälfte** nach `design.md` §7 (Ent-12): neue Zeile (Wortlaut **hier** festgelegt, Vorschlag `OTA start`), exakter Vergleich, setzt nur `is_online = 0`. **Prüfstand a67** | `stm-developer` | S3.9 | **AKK.1**; Gegenprobe FEHL; EP besteht weiter. Wortlaut im Bericht für E.4 | ☑ | — |
| S3.11 | **Flash-Gate** nach S3.10 | Lead | S3.10 | **AKZ.F** | — | — |
| S3.12 | **Prüfstände a23, a53, a57, a67 ablegen**; `watch-log.sh` um `spiel: … verworfen` und `ldr: … abgewiesen` (Hinweise); geänderte Logzeilen aus S3.2 in Wache und Werkzeugen nachgeführt | Lead | S3.11 | Alle OK, Gegenproben FEHL; a62, a69, a60, gt, EP, W2, L59 weiter OK | ☑ | — |
| S3.13 | **Review S3** | `firmware-analyst` | S3.12 | Einsparungen verhaltensneutral, geschützte Zeilen unverändert; A23, A53, A57, A67 wie in `design.md`; keine neue `watchdog_reload()`-Stelle (S7); Architektur-Checkliste | — | ☑ |
| S3.14 | STM-Version anheben | `release-engineer` | S3.13 | S4 zeigt den neuen Stand (erwartet 3.2.31) | ☑ | — |
| S3.15 | `CHANGELOG.md`-Eintrag S3 | Lead | S3.13 | Inhalt gleich dem Release; Ent-7 als Entscheidung des Nutzers benannt | ☑ | — |
| S3.15a | **Matrix-Lauf vor S3** (Ent-18, Festlegung des Leads), **nach der Versionsanhebung** auf genau dem Stand, der ausgeliefert wird — wie S1.16 | Lead | **S3.14** | **AKZ.M.5**: 72/72, **alle ≥ 1'024 Byte**, engste Variante benannt. Liegt eine Variante darunter: **kein Build**, anhalten und vorlegen (Ent-17). Läufe aus Testbauten zählen **nur als Vorabwert** | — | — |
| S3.16 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S3.14, S3.15, **S3.15a**, **S2.9** | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht | ☑ | — |
| S3.17 | **Einspielen am Tag:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; Smoketest. Vorher lesend `dspcolor[0]`, `numvar` 17/18, LDR-Rohwert, `numvar[8]` festhalten | Lead | S3.16 | erwartete STM-Version; Vorher-Werte im Bericht | — | — |
| S3.18 | **G3** nach `design.md` §15.2: Tagtests (AKD.2 Teil 1, AKS.3, **AKE.2** (Ent-14e)); **zweiter STM-Reset im Aus-Fenster** für AKD.2 Teil 2 und erneut AKW.4 — **vor G3 beim Nutzer zu bestätigen** | Lead | S3.17, **Bestätigung des zweiten Resets** | **AKD.2**, **AKS.3**, **AKE.2**; ohne Bestätigung bleibt AKD.2 Teil 2 offen und wird so berichtet | — | — |

---

## Schritt E — ESP-Release: A35, NTP (tragend), `ovr=`, A67-ESP

**Einspielreihenfolge:** nur ESP, **nach G3**. Wache läuft **vor** dem OTA.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E.1 | **A35 (b)** nach `design.md` §9. **Prüfstand a35** | `esp-developer` | **S3.16** | **AKQ.1**; Gegenprobe FEHL; **Zähl nach**. Kodierung und Zeilenende von `vars.cpp`, `ESP-uclock.ino` (**gemischt**) | ☑ | — |
| E.2 | **NTP** nach `design.md` §10.1 (Ent-13, **tragend**). **Keine** Ära- oder Pivot-Rechnung im ESP. **Prüfstand ntp** | `esp-developer` | E.1 | **AKU.1**, **AKU.2**; Gegenprobe (Wächterbyte, Transmit 0) FEHL | ☑ | — |
| E.3 | **`ovr=`** nach `design.md` §10.2 | `esp-developer` | E.2 | **AKO.1** | ☑ | — |
| E.4 | **A67 ESP-Hälfte** nach `design.md` §7: Zeile aus S3.10 **wörtlich** vor dem Download in allen drei OTA-Pfaden, `SYNCVARS` bei Fehlschlag | `esp-developer` | E.3, S3.10 | **AKK.2**; Kodierung (UTF-8) und Zeilenende von `http.cpp` | ☑ | — |
| E.5 | **Prüfstände a35 und ntp ablegen**; statische Prüfung AKK.2; `watch-log.sh` um `var ignoriert`, `ntp verworfen`, `ovr=` > 0 | Lead | E.4 | OK, Gegenproben FEHL bzw. schlägt an; Wachmeldungen kommen an | ☑ | — |
| E.6 | **Review E** | `code-reviewer` | E.5 | wie `design.md` §9, §10, §7 | — | ☑ |
| E.7 | ESP-Version anheben | `release-engineer` | E.6 | S4 zeigt den neuen Stand (erwartet 3.2.32) | ☑ | — |
| E.8 | `CHANGELOG.md`-Eintrag E | Lead | E.6 | Inhalt gleich dem Release | ☑ | — |
| E.9 | **AKZ.3**, Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | E.7, E.8, **S3.18** | `guardrails.sh --full` Exit 0; Tag gepusht | ☑ | — |
| E.10 | **Einspielen:** Wache läuft (bestätigt) → `curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"` → `./tools/smoke-device.sh` (samt Update-Quelle), `./tools/install-app.sh --check` | Lead | E.9 | neue ESP-Version; Update-Quelle richtig (DIR-009); PWA-Dateien vollständig (DIR-017) | — | — |
| E.11 | **G4**: Mitschnitt des OTA (Δ`v`, Vorher +2, L380), danach 60 min lesend | Lead | E.10 | **AKQ.2/AKU.3/AKO.2** | — | — |

---

## Schritt P — PWA-Release: C35

**Einspielreihenfolge:** nur PWA, **nach G4** (Firmware zuerst, DIR-002).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P.1 | **C35 (A)** nach `design.md` §11 (Ent-6). **Prüfstand c35** | `pwa-developer` | **E.9** | **AKC.1**; Gegenprobe FEHL; Kodierung und Zeilenende von `app.js` | ☑ | — |
| P.2 | **Prüfstand c35 ablegen** | Lead | P.1 | OK, Gegenprobe FEHL; S8 und `innerhtml.mjs` grün | ☑ | — |
| P.3 | **Review P** | `code-reviewer` | P.2 | Keine neue `innerHTML`-Stelle, kein leeres `catch`; Zeitpunkt des letzten Abrufs auf jedem Pfad gesetzt; Moduswechsel holt sofort | — | ☑ |
| P.4 | `APP_VERSION` **und** `CACHE_NAME` anheben | `release-engineer` | P.3 | S4 zeigt den neuen Stand (erwartet 1.4.96) | ☑ | — |
| P.5 | `CHANGELOG.md`-Eintrag P | Lead | P.3 | Inhalt gleich dem Release | ☑ | — |
| P.6 | **AKZ.3**, `git status` (R2: still), Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | P.4, P.5, **E.11** | `guardrails.sh --full` Exit 0; Tag gepusht | ☑ | — |
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
| Z.1 | `BEFUNDE.md`: A62 (Ära, neue Grenze 2099), A69 samt M1/M2 und den **sechs niedrigen Restpunkten** aus Review S1 (`design.md` §3.4) als benannte Befunde, A68, N1 (S1); A60 samt Freitakten (Ent-15b), L359, L360 (S2) und der **vorbestehende Nucleo-Befund** aus Review S2.4 (`GPIO_RESET_BIT(SDA_PORT, SCL_PIN)` trifft den falschen Port) als neuer Befund; A23, A53, **A57 schliessen mit G3-Beleg**, A52 falls in S3.2 umgesetzt (S3); A35, L391 (E); C35 (P); A21 nach M.1; A67 mit offenem Gerätenachweis (nächster ESP-OTA); die Flash-Lehre (Schätzungen Faktor 1,5 bis 3 daneben; engste Matrix-Variante rund 156–192 Byte unter dem Teilbuild); der Hinweis zur A57-Abweisung in der PWA (`design.md` §5a) als neuer Befund; ToDo-Liste nachgeführt | `doc-writer` | T.2 | S10 grün; jedes „erledigt" mit Beleg | ☑ | — |
| Z.2 | `knowledge/architecture-checklist.md`: I²C-Zeit je Byte auf den in G2 **gemessenen** Stand | `doc-writer` | S2.9 | S9/S11 grün | ☐ | — |
| Z.3 | `CLAUDE.md`, Abschnitt Hardware: I²C-Wartezeit je Byte auf den Stand nach A60 | Lead | S2.9 | S9/S11 grün; keine Versionsnummer neu | ☑ | — |
| Z.4 | **Spec-Endstand** | `spec-writer` | Z.1 | Jede Abweichung offen benannt | — | — |
| Z.5 | `README-CMAKE.md`: das Matrix-Ziel (`make matrix` / `make clean-matrix`, 72 Varianten, Gate 1'024 Byte für alle) und die **Abweichung beim F411-Startup**; dazu zwei Berichtigungen im Abschnitt Release-ZIP | `doc-writer` | S1.15 | **AKZ.M.1** (Dokumentationsteil); S9/S11 grün. **Stand: in Arbeit, vorgezogen** | ☑ | — |

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

Die Kästchen dieser Liste sind bewusst nicht nachgezogen; ihr Stand steht im folgenden Abschnitt.

---

## Endstand 13.10.2026

**Woher die Kästchen kommen.** Gerätenachweise und Befunde stehen in `BEFUNDE.md` (L393–L420;
Massnahmen A21, A23, A34, A35, A52, A53, A57, A60–A63, A67–A78, B54–B57, C35, C58, C59), die Inhalte
der Releases in `CHANGELOG.md` (sechs Einträge vom 11. bis 13.10.2026), Commits und Tags im Repo
(Reflog des Zweigs, `.git/refs/tags/release/`); dazu die Angaben des Leads vom 13.10.2026 zu den
Remote-Tags und zur Zählung in G5. **Spalte G ist wie im Vorpaket gefolgert:** Jeder Release-Build
verlangt `guardrails.sh --full` mit Exit 0, und die Tasks davor sind laut „Hängt ab von" seine
Voraussetzung; einzelne Guardrail-Läufe lagen dem `spec-writer` nicht vor. Bei Z.1 (Commit
`6de22d2`) und Z.3 (`CLAUDE.md`) ist der Inhalt am Baum nachgelesen. **Spalte R** ist gesetzt, wo
`BEFUNDE.md` das Review nennt (0.2: „nach vier Runden bestanden", A61; E.6: L406; P.3: Restpunkt
N-b unter C35, jetzt L418), und bei S3.1 und S3.13 aus dem Release S3 gefolgert. Die Abnahmetexte
der Tabellen sind nicht nachgezogen — die Belege stehen hier.

### Releases — sechs statt fünf

| Tag | Commit | Komponente(n) | Inhalt |
|---|---|---|---|
| `release/3.2.29-3.2.31-1.4.95` | `a6065d5` | STM 3.2.29 | S1 — A62 mit Ära, A69 samt M1/M2, A68, N1 |
| `release/3.2.30-3.2.31-1.4.95` | `a3540a9` | STM 3.2.30 | S2 — A60 samt L360 und Freitakten (Ent-15b) |
| `release/3.2.31-3.2.31-1.4.95` | `5e54fe7` | STM 3.2.31 | S3 — Einsparungen (darunter A52), A23, A53, A57 (C), A67-STM, dazu A73 |
| `release/3.2.31-3.2.32-1.4.95` | `3be7f5b` | ESP 3.2.32 | E — A35 (b), C58/L391, `ovr=`, A67-ESP, Nacharbeit aus Review E.6 |
| `release/3.2.32-3.2.33-1.4.96` | `ac420eb` | STM 3.2.32, ESP 3.2.33, PWA 1.4.96 | P (C35) und die Korrekturen aus dem OTA-Mitschnitt L408: A74, A75 (STM), Instrumentierung A76 (ESP) |
| `release/3.2.32-3.2.33-1.4.97` | `31b1fc3` | PWA 1.4.97 | L411/B54 |

Am Gerät stehen damit STM 3.2.32, ESP 3.2.33, PWA 1.4.97 (L413). Der Zweig `pwa-decoupling` steht
auf dem Remote auf demselben Commit wie lokal (`10e2a95`). **Alle sechs Tags liegen auf dem
Remote** — vom Lead am 13.10.2026 mit `git ls-remote --tags origin` geprüft.

### Gerätenachweise

| Nachweis | Stand | Ergebnis | Beleg |
|---|---|---|---|
| **G1 Tag** | STM 3.2.29 | AKN.6: `time: sprung` 20:44:39 → `cmd: set time to` 20:44:54; AKI.2 vorher `eep n=2` 24–26 ms, 32 Byte 92 ms | A62, L400 |
| **G1 Nacht** | STM 3.2.29 | Reset 12.10.2026 01:34:21 nach dem Aus-Timer 01:00: IP-Lauftext, 01:34:39 `night: start aus (01:00)` über den Restore-Zweig, Bestätigung der Netzzeit nach 15 s; Display und Ambilight aus (AKW.4, AKN.7, AKB.1) | L401 |
| **G1 Morgen** | STM 3.2.29 | Ein-Timer 07:00:01 regulär, Ambilight nicht mitgeschaltet, ESP-Kopie richtig (AKW.5) | A68, A69 |
| **G2** | STM 3.2.30 | AKI.2 nachher `n=2` **15 ms**, 32 Byte **35–36 ms** (Erwartung 16–17 bzw. 34–40); AKI.3 07:10–08:11 ohne Fristzeile, ohne Neustart; DS1307-Messpunkt widerlegt; 7 Fristzeilen beim Start aus der EEPROM-Adresssuche ⇒ A73 | L402, L403 |
| **G3 Tag** | STM 3.2.31 | A23 (`dspcolor[0]` zurück auf 0/0/0/63), A53 (`spiel: 1 Zeilen verworfen`, Abgleich), A57 (beide Abweisungen ohne `eep`, zurück auf 0/4095), A73 (keine Fristzeile beim Start) | L404 |
| **G3 Nacht** | STM 3.2.31 | Zweiter STM-Reset, vom Nutzer freigegeben, 13.10.2026 01:06:16: AKD.2 Teil 2 (Nutzerfarbe aus dem EEPROM), AKW.4 erneut | L407 |
| **G4** | ESP 3.2.32 → 3.2.33 | Erster OTA 01:11:40 (auf 3.2.32, ausgeführt von 3.2.31, deshalb ohne `OTA start`): `v` +3, eine Sendung **nach** `CAP`, `var ignoriert code=n`, `ovr=1` ⇒ A74, A75, A76. Zweiter OTA 01:34:52 (auf 3.2.33, ausgeführt von 3.2.32): `(OTA start)`, `v=0/0`, `ovr=0` | L408, L410 |
| **G5** | PWA 1.4.96 | `check-pwa.sh` 8/0; Hauptmodul 15 min (01:38:51–01:53:51): je 3 Abrufe `update_status`/`update_table_files` (01:38:51, 01:44:02, 01:49:17) statt 60. Gezählt nur Anfragen mit Kennung `[Chrome]`; der `pwa-tester` startete bis 01:54 keinen Browser, seine `curl`-Anfragen tragen `[curl/…]`, die Abrufe seines Browsers beginnen 01:54:12, ausserhalb des Fensters | C35, L410; Angabe des Leads |
| **G6 / M.1** | 3.2.32 / 3.2.33 / 1.4.97 | 30 min, drei Browser, Setter-Burst: 180/180 `diag` mit `d=0 o=0 v=0/0 a=0`, `rx` ≤ 234/1024, `ovr=0`, kein Neustart ⇒ **A21 geschlossen** | L413 |
| **T.2** | 3.2.32 / 3.2.33 / 1.4.96 | 471 Prüfzeilen, 450 bestanden, 11 Scheinabweichungen des Werkzeugs; L411 neu. **V3 nicht gefahren** | L412 |
| **T.1** | — | `TESTPLAN-PWA.md` nachgeführt (`73bb009`); der Teil „Logzeilen, die S3.2 geändert hat" vom `doc-writer` offen gelassen | Commit `73bb009` |

### Abweichungen von der Spec, offen benannt

1. **Release-Teilung nach dem Flash-Gate (Ent-17, L393).** Nach S1.3 waren 1'132 Byte frei; A23,
   A53, A57 und A67-STM gingen in ein eigenes STM-Release S3 mit vorgeschalteter Flash-Analyse,
   N1 wurde vorgezogen. Während des Pakets eingearbeitet, hier der Vollständigkeit halber.
2. **Matrix-Lauf über 72 Varianten (Ent-18, L395)** — im ursprünglichen Entwurf nicht vorgesehen.
   Vor S1 belegt (72/72, engste 1'416). Vor S2 laut `CHANGELOG.md` 72/72 ohne Variante unter dem
   Gate; **die engste Variante ist im Beleg nicht genannt.** Vor S3 nennt `CHANGELOG.md` nur, dass
   `make matrix` lief, **kein Ergebnis.** Vor dem Release-Build von STM 3.2.32 (Folge-Release,
   13.10.2026 01:32): 72/72, 0 unter dem Gate, engste Variante `wc24h-stm32f103-sk6812-rgbw` mit
   **1'436 Byte** frei, Teilbuild 1'656 (L416). Der Abstand Teilbuild–engste Variante lag damit
   zwischen 156 und 220 Byte.
3. **A60: Freitakten nach jeder Frist (Ent-15b)** — erweitert Ent-15; zuerst nur im BUSY-Pfad, vom
   Lead nach der Nachprüfung auf jede Frist ausgedehnt.
4. **A73 nachgeschoben (L402).** Die EEPROM-Adresssuche beim Start erzeugte unter 3.2.30 sieben
   Fristzeilen und damit einen Fehlalarm der Wache je Neustart; in S3 mitgenommen.
5. **Ein zusätzliches Release ausserhalb der Planung (L408).** Statt eines reinen PWA-Releases P
   ging ein Release mit **drei** Komponenten hinaus: STM 3.2.32 (A74, A75), ESP 3.2.33 (A76) und
   PWA 1.4.96 (C35). Der Grundsatz „je Release eine Komponente" aus `design.md` §0.1 ist dort
   verlassen; `CHANGELOG.md` begründet die Reihenfolge STM → ESP → PWA damit, dass der OTA schon
   mit dem neuen STM gemessen wird. Folgen: G4 ist auf dem **zweiten** OTA belegt, nicht auf dem
   OTA von E; der P-Build (ZIP 01:32:18) lag rund 20 Minuten nach dem ersten OTA, **nicht nach
   60 Minuten lesendem Betrieb** (E.11). Lesend belegt ist der Betrieb danach: Netzzeit 02:05:04 und
   03:08:40 übernommen, 0 × `ntp verworfen` (C58), dazu die Fenster von T.2 und M.1.
6. **Reihenfolge T vor M.** Geplant war M.1 nach P.8 und T.2 nach M.1. Tatsächlich lief T.2 am
   13.10.2026 01:39–02:35 auf PWA 1.4.96, M.1 02:50–03:20 auf PWA 1.4.97. Der Grund der Spec (die
   Setterlast des Testers verfälscht M) ist nicht berührt, weil M danach lief; **T.2 lief aber
   nicht auf dem Endstand der PWA.** Die Zählung von G5 ist davon nicht berührt: Der Testlauf
   startete seinen Browser erst nach dem G5-Fenster, gezählt wurde nach Kennung `[Chrome]`
   (Angabe des Leads, siehe „Gerätenachweise").
7. **C35 um `update_table_files` erweitert** (Review M1): Das Hauptmodul drosselt auch die
   Tabellenliste, nicht nur `update_status` (`CHANGELOG.md`, C35). Ent-6 nannte nur `update_status`.
8. **A61 geschlossen als Werkzeug (Ent-8).** Der Schliessvermerk nennt vier Abgleiche vom
   13.10.2026, alle unauffällig. Die frühere Schliessbedingung („sobald ein CMC-ausgelöster
   Abgleich ausgewertet ist") nennt er **nicht** als erfüllt; der CMC-Fall ist in den Belegen
   weiterhin nicht gemessen.
9. **Funde während des Abschlusses:** L411 (PWA-Ladefehler beim Start im Modul System, behoben mit
   1.4.97 als eigenem Release) und L414 (`abgleich.sh` las über die Pipe-Form nichts, behoben; die
   A34-Auswertung vom 11.10.2026 ist nicht betroffen, sie las 139'565 Zeilen).
10. **Flash-Zahlen der Release-Builds S2 und S3 sind nicht gemeldet.** Belegt sind die Testbauten
    bis S2 (1'464 Teilbuild, 1'308 engste Variante, Vorabwert), für S3 nur „zusammen rund 500 Byte"
    eingespart (`CHANGELOG.md`), A52 −20 Byte, und der Matrix-Lauf vor STM 3.2.32 (Punkt 2). Die
    Testbauten je Einsparung (S3.3) und je Task (S3.5–S3.11) liegen nicht vor.
11. **Schritt Z:** Z.1 erledigt mit Commit `6de22d2` — Nucleo-Befund als L415/A77
    (`src/i2c/i2c.c:334`), Flash-Lehre L416, A57-Abweisung in der PWA als L417/B55, dazu L418–L420
    aus den Restpunkten der Reviews. Z.3 erledigt: `CLAUDE.md`, Abschnitt Hardware, nennt die nach
    A60 gemessenen Werte (rund 15 ms je Zyklus, 35–36 ms für 32 Byte über zwei Seiten, L403).
    **Z.2 offen:** `knowledge/architecture-checklist.md` ist beim `doc-writer` in Arbeit.

### Lehren

- **Flash-Schätzungen lagen um den Faktor 2 bis 3 daneben, bei A62 im Vorzeichen (L393, L416).**
  Das Gate je Task hat es nach zwei Tasks gezeigt, nicht erst beim Release-Build. Geplant wird mit
  der Messung je Task und ab der engsten Matrix-Variante (`design.md` §1.2); der Abstand zum
  Teilbuild ist nicht fest (156 bis 220 Byte, L416).
- **Vier Prüfungen, deren Eingabeweg nicht geprüft war — dieselbe Gattung wie DIR-014:**
  L409 (Prüfstand-Artefakte mit absoluten Benutzerpfaden im Release-Commit; geprüft war auf
  `192.168.`, nicht auf Artefakte), L410b (ein Probelauf der Logwache teilte Zustandsdateien mit
  der laufenden Gerätewache, die daraufhin alte Ereignisse neu meldete), L411 (`check-pwa.sh`
  startete mit frischem Profil immer im Hauptmodul und sah den Ladefehler im Modul System nie),
  L414 (die dokumentierte Pipe-Form von `abgleich.sh` las nichts und meldete „0 Abgleiche" mit
  Exit 0). In allen vier Fällen war der Mechanismus geprüft, der Weg der Eingabe nicht.
- **Ein OTA wird von der alten Firmware ausgeführt (L408).** Eine Änderung am OTA-Pfad (A67) ist
  erst beim **nächsten** OTA nachweisbar; der Plan hatte G4 auf den OTA von E gelegt.

### Offen nach dem Paket

| Was | Warum offen | Träger |
|---|---|---|
| A63 1-Wire-Leseslot | Beobachtung (Ent-10), bisher kein `ungueltig` | `BEFUNDE.md` A63 |
| A70, A71, A72 | Randfälle aus Review S1 und der Umsetzung von A69, benannt, kein Fix | L396–L399 |
| A76 | Instrumentierung ausgeliefert, wartet auf einen Überlauf mit Zeitpunkt | L408 |
| C59 | NTP-Antwort an die Anfrage binden (Review E.6) | L406 |
| A77, B55, B56, B57, A78 | neu benannt beim Abschluss, alle klein | L415, L417–L420 |
| V3 aus T.2 | STM-Reset von der Berechtigungsprüfung abgewiesen, liegt beim Nutzer; damit auch `numvar[45]` 0→3 aus S79 | L412 |
| T.1, Teil S3.2-Logzeilen | vom `doc-writer` offen gelassen | `73bb009` |
| Z.2 | beim `doc-writer` in Arbeit | diese Spec |
| ESP-Ursache von C35 | ungecachte Abrufe je Aufruf, wirkt weiter über Wartung und Anzeige (Varianten B/C, `design.md` §11) | C35 |

**Spalten G/R, die offen bleiben:** T.1 (Teil offen) und Z.2. Alle übrigen sind gesetzt.

### Neue Befunde aus diesem Paket

L391/C58 (erledigt), L392, L393, L394, L395, L396–L399 (A70–A72), L400, L401, L402/A73, L403, L404,
L405, L406/C59, L407, L408 (A74, A75, A76), L409, L410, L411/B54, L412, L413, L414, L415/A77, L416,
L417/B55, L418/B56, L419/B57, L420/A78.
