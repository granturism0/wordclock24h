# Tasks — Paket 2026-10-11 „Nachtfenster, Netzzeit, Spiele, I²C, Brücke" (A23, A69, A53, A57, A60, A61, A62, A63, A35, A21, A34, C35)

**Erstellt:** 2026-10-11, Stand `5c34d80`. Am selben Tag mit den Entscheidungen des Nutzers
nachgeführt. Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.

**Status: freigegeben** — vom Nutzer am 11.10.2026 (Ent-16), samt Ent-1 bis Ent-15. **Zwei
Entscheidungen gegen die Empfehlung:** Ent-3 (Ära-Rechnung jetzt) und Ent-7 (C) („Messwert
übernehmen" weist eine Inversion ab). Ent-3 ist im Entwurf ausgearbeitet (`design.md` §2); die
daraus folgende Bestätigungsregel (**Ent-3b**) hat der Nutzer am selben Tag entschieden, ebenso
die eigene Freigabe der LDR-Abweisungsprobe (**Ent-14e**). Kein Task ist mehr blockiert.

## Entscheidungen (11.10.2026)

| Ent | Gegenstand | Entschieden | wirkt auf |
|---|---|---|---|
| **Ent-1** | A23 Displayfarbe | (a) RAM-Sicherung; DC aktualisiert die Sicherung: **ja**; IR-Farbschritte im Modus 0: **ja** | S1.7 |
| **Ent-2** | A69 Nachtfenster | (1) **nur die Displayliste**; (2) **beim ersten übernommenen `TIME` einmal auswerten**; (3) **ein übersprungenes Ereignis nachholen**; (4) **Startsequenz wie heute, danach aus** | S1.3, 0.5 |
| **Ent-3** | A62 Verfallsdatum 2036 | **Ära-Rechnung jetzt** (gegen die Empfehlung) — Pivot ohne Protokollwechsel, `design.md` §2 | S1.1, E.2 |
| **Ent-3b** | Bestätigungsregel bei Zeitsprung (Folge aus Ent-3) | **Bestätigungsregel wie vorgeschlagen:** bei mehr als 600 s Abweichung und immer nach dem Start gilt eine Zeit erst, wenn eine zweite Antwort nach 15 s sie auf ±5 s bestätigt; höchstens 3 Versuche (`design.md` §2.4) | S1.1 |
| **Ent-4** | A53 Spiele | **V2**, keine Inaktivitätsgrenze | S1.9 |
| **Ent-5** | A35 | **(b)** nur ESP, zählen und melden | E.1 |
| **Ent-6** | C35 | **(A)** nur PWA, Hauptmodul beim Öffnen und alle 5 min; kein 15-s-Takt im Hauptmodul | P.1 |
| **Ent-7** | A57 | **(C)** „Messwert übernehmen" weist eine Inversion ab (gegen die Empfehlung); **nicht** der Zahlen-Setter (A59). A57 wird erst nach G1 geschlossen | S1.11, S1.21, Z.1 |
| **Ent-8** | A61 | Auswerteregel im Werkzeug | 0.1 |
| **Ent-9** | A34 | schliessen, Reste überführen | 0.6 |
| **Ent-10** | A63 | beobachten | 0.6 |
| **Ent-11** | A68 | ja, in S1 | S1.5 |
| **Ent-12** | A67 | ja, STM- und ESP-Hälfte | S1.13, E.4 |
| **Ent-13** | NTP-Puffer und Plausibilität | ja, beides — **nach Ent-3 tragend** | E.2 |
| **Ent-14** | schreibende Gerätenachweise | **alle erteilt:** (a) STM-Reset im Nacht-Aus-Fenster; (b) Setter während eines Spiels; (c) Setter-Burst im A21-Lauf; (d) Farbanimation, LDR S25/S26 und 32-Byte-Wert vor/nach S2, `pwa-tester`-Durchlauf mit S-Fällen | S1.21, S2.8, M.1, T.2 |
| **Ent-14e** | LDR-Abweisungsprobe AKE.2 | **ausdrücklich freigegeben (eigene Freigabe des Nutzers):** Maximum per Zahlen-Setter kurz auf 1 bzw. Minimum auf 4094, danach zurück auf 0/4095 | S1.21 |
| **Ent-15** | A60 Umfang | nur die Fristen | S2.1 |
| **Ent-16** | Spec | **freigegeben** | alle |

**Vom Lead festgelegt:** `CHANGELOG.md` je Release schreibt der **Lead**. ESP-OTA mit
`curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"`. **L391** ist für den NTP-Puffer
reserviert; bis Schritt 0 schreibt niemand in `BEFUNDE.md`.

---

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wo die Dateien disjunkt sind** (R3b). Drei
Ketten tragen das Paket (`design.md` §0.1):

1. **Code des nächsten Releases erst nach dem Release-Build des vorigen:** S2.1 nach S1.19, E.1
   nach S2.7, P.1 nach E.9. Grund: `make release-zip` baut STM, ESP und PWA-Assets zusammen
   (Lehre aus Paket 2026-10-09).
2. **Release-Build erst nach dem Gerätenachweis des vorigen:** S2.7 nach S1.21 (G1), E.9 nach
   S2.9 (G2), P.6 nach E.11 (G3). Danach M.1 nach P.8 (G4), T.2 nach M.1 (G5).
3. **Innerhalb S1 nacheinander, Testbau nach jedem Task** (Flash-Gate, AKZ.F). Liegt ein Testbau
   unter **1'024 Byte**: **anhalten und vorlegen** (`design.md` §1.3).

**Vor jedem Release-Build** prüft der Lead AKZ.3: `git diff --name-only <letztes Release-Tag>`
samt Arbeitsbaum zeigt Produktänderungen **nur** in der Komponente, die dieser Build anhebt.
Sonst **kein** Build. Den `CHANGELOG.md`-Eintrag schreibt der **Lead** vor dem Commit.

**Einspielen macht der Lead** — die Zeilen stehen trotzdem je Schritt fertig da (AKZ.8). Jeder
Gerätelauf mit `./tools/watch-log.sh`, **vor** dem Einspielen gestartet (DIR-013). Schreibend nur
im Rahmen von Ent-14 und Ent-14e und mit Rückstellung.

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. **Prüfstände baut
der Umsetzer im Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab** und fährt die
Gegenprobe gegen den Vorgängerstand (DIR-014).

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — ohne Firmware

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **A61-Auswerteregel als Werkzeug** nach `design.md` §12.1 (Auslöser, Dauer, Δ`v` beide Zahlen, Zeitüberschreitungen, Stillstand); synthetischer Mitschnitt als Prüfstand | Lead | Freigabe | **AKA.1**: Gegenprobe (falscher Auslöser) FEHL mit Exit ≠ 0, Meldung kommt an; **Zähl nach** gegen `grep -c 'info: configuration sent'` | ☐ | ☐ |
| 0.2 | **Review 0.1** | `code-reviewer` | 0.1 | Regeln wie §12.1; „unbekannt" statt geraten; beide Zahlen von `v=`; Exit-Weg der Meldung | — | ☐ |
| 0.3 | **Befunde nachführen, Teil 1:** **L391** NTP-Puffer mit Massnahme in Gruppe C; Berichtigungen an **L331**, **L360**, **L371** (dazu: Ära-Rechnung beschlossen; die Kette endet danach am 31.12.2099 an der RTC), **L233**; Fundstellen A69 (`night.c:155`), A61 (`main.c:4037-4048`), A62 (`main.c:3274`); A53 „mit A16 zusammen" überholt, `snake.c:291`; A21 blinder Fleck ESP-Empfangspuffer; A57 mit dem Vermerk „Ent-7 (C), Schliessung nach G1" — alles nach `design.md` §12.3 | `doc-writer` | Freigabe | S10/S11 grün; jede Berichtigung mit Datei:Zeile von `5c34d80`; keine Versionsnummer in lebender Doku | ☐ | — |
| 0.4 | **Messung A61/A34** mit 0.1 über die vorhandenen Mitschnitte (G3-OTA 10.10.2026 23:06, STM-Reset 11.10.2026 01:58:14, Langzeitmitschnitt) | Lead | 0.2 | **AKA.2**: Tabelle je Abgleich; Entscheidungsregel A34 (über 5 s Stillstand ⇒ Task 10 als neuer Befund) angewandt und benannt | — | — |
| 0.5 | **Testplan V3/S1 um den Nachtzustand** nach `design.md` §12.4 (Ent-2): Soll nach einem Reset im Aus-Fenster ab STM-Release 1 = Display aus; Rückstellung nach S1 im Aus-Fenster | `doc-writer` | Freigabe | **AKA.4**; S9/S11 grün. **Muss vor S1.21 (G1) fertig sein** | ☐ | — |
| 0.6 | **Befunde nachführen, Teil 2:** A34 schliessen (Ent-9) mit dem Ergebnis aus 0.4 und den überführten Resten; A61 mit dem Messergebnis; A63 als Beobachtung **offen** in der ToDo-Liste (Ent-10); A35 auf „kein hoher Befund". **A57 bleibt offen** bis nach G1 (Ent-7) | `doc-writer` | 0.3, 0.4 | **AKA.3**; S10 grün | ☐ | — |

---

## Schritt S1 — STM-Release 1: A62 (mit Ära), A69, A68, A23, A53, A57, A67-STM

**Einspielreihenfolge:** nur STM. **Am Tag** einspielen, den Reset im Aus-Fenster eigens fahren
(`design.md` §15.1).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S1.1 | **A62 mit Ära-Rechnung** nach `design.md` §2 (Ent-3, Ent-3b): eigener Parser (1–10 Ziffern + `'\0'`, Überlaufprüfung ⇒ über 4'294'967'295 verwerfen); Totbereich `[2'208'988'800, 3'976'214'400)` verwerfen; Rechnung NTP→Unix weiter über die `uint32`-Differenz (`timeserver.c:253`), kein 64-Bit-Pfad; Bestätigungsregel nach Ent-3b samt Begrenzung der Schnellabfragen; Logzeilen `time: ungueltig, verworfen` und `time: sprung, bestaetigung angefordert`; Zeitzone beim Laden ±12 (`timeserver.c:67`); kein `strtoul` mehr. **Prüfstand a62** | `stm-developer` | Freigabe, **Ent-3b** | **AKN.1**–**AKN.3**, **AKN.5** am Prüfstand, Fallzahl; Referenz Python `datetime` **und** gt; Gegenprobe gegen `release/3.2.28-3.2.31-1.4.95` FEHL; Sabotagen „Totbereich entfernt", „genau 10 Ziffern", „Bestätigung entfernt" FEHL. Kodierung und Zeilenende von `main.c`, `timeserver.c` | ☐ | — |
| S1.2 | **Flash-Gate** nach S1.1; Map-Prüfung: kein `strtoul`, weiter kein `__udivmoddi4`/`__aeabi_ldivmod` (S8b) | Lead | S1.1 | **AKN.4**, **AKZ.F**; Einsparung neben −110…−20 | — | — |
| S1.3 | **A69** nach `design.md` §3 (Ent-2): `night_last_event()` und Hilfsfunktion Wochentag in `night.c`; Auswertung beim Start, beim ersten übernommenen `TIME` (falls die RTC beim Start nicht gültig war) und bei Vorwärtssprüngen (übersprungenes Ereignis); nur die Displayliste; Logzeile `night: start aus (hh:mm)`. Melden: ob der IP-Lauftext bei ausgeschaltetem Display sichtbar ist (am Code). **Prüfstand a69** | `stm-developer` | S1.2 | **AKW.1**–**AKW.3**; **Zähl nach:** Tabellen × 10'080; Sabotagen `<=`→`<` und Wochentagsumlauf FEHL; Gleichstandsverhalten benannt. Kodierung und Zeilenende von `night.c` | ☐ | — |
| S1.4 | **Flash-Gate** nach S1.3 | Lead | S1.3 | **AKZ.F**; Zuwachs neben +160…+310 | — | — |
| S1.5 | **A68** nach `design.md` §4 (Ent-11): Sendefunktion der Ambilight-Power öffentlich, Aufruf an jeder Stelle der **Bestandsliste**; keine zweite Sendefunktion. Prüfstand a69 um das Sendeprotokoll erweitern | `stm-developer` | S1.4 | **AKB.1**; Bestandsliste; Gegenprobe FEHL. Kodierung und Zeilenende von `vars.c`, `vars.h` | ☐ | — |
| S1.6 | **Flash-Gate** nach S1.5 | Lead | S1.5 | **AKZ.F** | — | — |
| S1.7 | **A23** nach `design.md` §5 (Ent-1): `saved_display_colors`, gesetzt beim Laden, in DC (auch während einer Animation) und in den IR-Farbschritten im Modus 0; zurück auf **jedem** Weg auf Modus 0 samt `display_calc_dimmed_display_colors()` und `var_send_display_colors()`. **Prüfstand a23** | `stm-developer` | S1.6 | **AKD.1**; Gegenprobe FEHL; kein EEPROM-Schreiben im Animationspfad. Kodierung und Zeilenende von `display.c` | ☐ | — |
| S1.8 | **Flash-Gate** nach S1.7 | Lead | S1.7 | **AKZ.F**; Zuwachs neben +50…+90 | — | — |
| S1.9 | **A53 V2** nach `design.md` §6 (Ent-4): Zähler in beiden Spielschleifen, nach dem Spiel bei Zähler > 0 `var_sync_pending = 1` und `spiel: %u Zeilen verworfen`; `snake.c:291` initialisiert. **Prüfstand a53** | `stm-developer` | S1.8 | **AKS.1**, **AKS.2**; Gegenprobe FEHL; ob `.` zählt, begründet. Kodierung und Zeilenende von `tetris.c`, `snake.c` | ☐ | — |
| S1.10 | **Flash-Gate** nach S1.9 | Lead | S1.9 | **AKZ.F**; Zuwachs neben +80…+100 | — | — |
| S1.11 | **A57 (C)** nach `design.md` §5a (Ent-7): `ldr_set_min_value()`/`ldr_set_max_value()` übernehmen den Messwert **nicht**, wenn er die Reihenfolge verletzt (neues Minimum ≥ Maximum bzw. neues Maximum ≤ Minimum); Wert bleibt, EEPROM unberührt, Logzeile `ldr: minimum abgewiesen` bzw. `ldr: maximum abgewiesen`; die Rückmeldung `var_send_ldr_*` (`main.c:1609`, `:1617`) bleibt und meldet den alten Wert. **Zahlen-Setter (A59, `main.c:1907-1914`) unverändert.** **Prüfstand a57** | `stm-developer` | S1.10 | **AKE.1**; Gegenprobe gegen 3.2.28 FEHL; L59 besteht weiter (Zahlen-Setter nimmt die Inversion an). Kodierung und Zeilenende von `ldr.c` | ☐ | — |
| S1.12 | **Flash-Gate** nach S1.11 | Lead | S1.11 | **AKZ.F**; Zuwachs neben +30…+50 | — | — |
| S1.13 | **A67 STM-Hälfte** nach `design.md` §7 (Ent-12): neue Zeile (Wortlaut **hier** festgelegt, Vorschlag `OTA start`), exakter Vergleich, setzt nur `is_online = 0`. **Prüfstand a67** | `stm-developer` | S1.12 | **AKK.1**; Gegenprobe FEHL; EP besteht weiter. Wortlaut im Bericht für E.4. Kodierung und Zeilenende von `esp8266.c` | ☐ | — |
| S1.14 | **Flash-Gate** nach S1.13 | Lead | S1.13 | **AKZ.F**; Zuwachs neben +30…+50 | — | — |
| S1.15 | **Prüfstände a62, a69, a23, a53, a57, a67 ablegen**; `watch-log.sh` um `time: ungueltig` (Alarm), `time: sprung` (Hinweis; Alarm bei mehr als drei in Folge), `spiel: … verworfen`, `night: start aus`, `ldr: … abgewiesen` (Hinweise) | Lead | S1.14 | Alle OK, Gegenproben FEHL; je Wachmeldung eine eingespielte Zeile, die anschlägt **und ankommt** (DIR-014); Wache neu gestartet (L362); gt, EP, W2, L59 weiter OK | ☐ | — |
| S1.16 | **Review S1** | `firmware-analyst` | S1.15 | **A62:** Überlauf, Totbereich, Ära-Abbildung über die `uint32`-Differenz, Bestätigungsregel (Zustand nach Reset, Begrenzung der Schnellabfragen), Zeitzone, keine 64-Bit-Arithmetik, Grenze 2099 durch die RTC benannt. **A69:** §3.1, Start vor `watchdog_init()`, Offline-Senden, Sprungerkennung, Gleichstand. **A68:** Bestandsliste vollständig. **A23:** jeder Weg auf Modus 0. **A53:** Zähler auf jedem Ausgang. **A57:** Gleichheit abgewiesen wie beim Start (`ldr.c:128`), Zahlen-Setter unberührt. **A67:** kein Prüfsummenzustand berührt. Keine neue `watchdog_reload()`-Stelle (S7). Architektur-Checkliste | — | ☐ |
| S1.17 | STM-Version anheben | `release-engineer` | S1.16 | S4 zeigt den neuen Stand (erwartet 3.2.29) | ☐ | — |
| S1.18 | `CHANGELOG.md`-Eintrag S1 | Lead | S1.16 | Inhalt gleich dem Release; Ent-3 und Ent-7 als Entscheidung des Nutzers benannt | ☐ | — |
| S1.19 | **AKZ.3**, `git status`, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S1.17, S1.18 | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht | ☐ | — |
| S1.20 | **Einspielen am Tag:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; Smoketest. Vorher lesend `dspcolor[0]`, `numvar` 17/18 und Display-/Ambilight-Power festhalten | Lead | S1.19 | erwartete STM-Version; Vorher-Werte im Bericht | — | — |
| S1.21 | **G1** nach `design.md` §15.1: Tagtests (AKN.6, AKD.2 Teil 1, AKS.3, **AKE.2** (Ent-14e), **AKI.2 vorher**); **STM-Reset im Aus-Fenster** für AKW.4, AKD.2 Teil 2 und AKN.7; am Morgen AKW.5 | Lead | S1.20, **0.5** | **AKN.6**, **AKN.7**, **AKW.4**, **AKW.5**, **AKD.2**, **AKS.3**, **AKE.2**, AKI.2 (vorher); `watch-log.sh` mitgelesen, freigegebener Reset getrennt berichtet; alle Testwerte zurückgestellt und lesend nachgeprüft | — | — |

---

## Schritt S2 — STM-Release 2 (isoliert): A60 und L360

**Einspielreihenfolge:** nur STM, **nach G1**.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S2.1 | **A60** nach `design.md` §8 (Ent-15): **zuerst am Code belegen**, dass keine ISR `delay_*` ruft; dann `i2c_wait_for_flags()` eng pollen über `delay_counter` (Frist 5 ms wie heute, überlaufsicher), **beide** BUSY-Schleifen (`i2c.c:413`, `:472`) mit derselben Frist, Zweige zusammenlegen, kein RM0008-Abschlussverfahren. **Prüfstand a60** | `stm-developer` | **S1.19** | **AKI.1**; Beleg „keine ISR mit `delay_*`" mit Fundstellen; Gegenproben FEHL. Kodierung und Zeilenende von `i2c.c` | ☐ | — |
| S2.2 | **Flash-Gate** nach S2.1 | Lead | S2.1 | **AKZ.F**; Zuwachs neben 0…+60 | — | — |
| S2.3 | **Prüfstand a60 ablegen**; Logwache: Schwelle `eep` 200 ms unverändert, Fristablauf-Zeile als Alarm | Lead | S2.2 | OK, Gegenprobe FEHL; Wachmeldung kommt an | ☐ | — |
| S2.4 | **Review S2** | `firmware-analyst` | S2.3 | Frist überall gleich, auch beim Schreiben; ISR-Frage am Code bestätigt; Verhalten bei Fristablauf vor `watchdog_init()`; keine Änderung am Busprotokoll ausser dem Pollen; gerechnete Zeiten plausibel | — | ☐ |
| S2.5 | STM-Version anheben | `release-engineer` | S2.4 | S4 zeigt den neuen Stand (erwartet 3.2.30) | ☐ | — |
| S2.6 | `CHANGELOG.md`-Eintrag S2 | Lead | S2.4 | Inhalt gleich dem Release | ☐ | — |
| S2.7 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S2.5, S2.6, **S1.21** | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht | ☐ | — |
| S2.8 | **Einspielen:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`; Smoketest | Lead | S2.7 | erwartete STM-Version | — | — |
| S2.9 | **G2**: AKI.2 nachher (S25/S26 über S27 zurück, 32-Byte-Wert zurück; Ent-14 d), AKI.3 (60 min lesend) | Lead | S2.8 | **AKI.2** (Vorher/Nachher-Tabelle), **AKI.3**; `watch-log.sh` mitgelesen | — | — |

---

## Schritt E — ESP-Release: A35, NTP (tragend), `ovr=`, A67-ESP

**Einspielreihenfolge:** nur ESP, **nach G2**. Wache läuft **vor** dem OTA.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E.1 | **A35 (b)** nach `design.md` §9: Zähler in `var_set_parameter()` an jeder still entfallenden Zuweisung, `V…` ausgenommen, Rückgabe unverändert; gedrosselte Zeile `- var ignoriert #n code=X idx=yy` **im Mitschnitt und in `/api/stm32_log`** (Weg belegt). **Prüfstand a35** | `esp-developer` | **S2.7** | **AKQ.1**; Gegenprobe FEHL; **Zähl nach**. Kodierung und Zeilenende von `vars.cpp`, `ESP-uclock.ino` (**gemischt**) | ☐ | — |
| E.2 | **NTP** nach `design.md` §10.1 (Ent-13, **tragend** nach Ent-3): höchstens 48 Byte lesen, längere Pakete verwerfen; LI = 3, Stratum 0, Transmit 0 verwerfen; Zeile `- ntp verworfen` mit Grund. **Keine** Ära- oder Pivot-Rechnung im ESP (er rechnet nicht mit der Zeit, §2.2). **Prüfstand ntp** | `esp-developer` | E.1 | **AKU.1**, **AKU.2**; Gegenprobe (Wächterbyte, Transmit 0) FEHL. Kodierung und Zeilenende von `ntp.cpp` | ☐ | — |
| E.3 | **`ovr=`** nach `design.md` §10.2 | `esp-developer` | E.2 | **AKO.1** | ☐ | — |
| E.4 | **A67 ESP-Hälfte** nach `design.md` §7: Zeile aus S1.13 **wörtlich** vor dem Download in allen drei OTA-Pfaden, `SYNCVARS` bei Fehlschlag | `esp-developer` | E.3, S1.13 | **AKK.2**; Kodierung (UTF-8) und Zeilenende von `http.cpp` | ☐ | — |
| E.5 | **Prüfstände a35 und ntp ablegen**; statische Prüfung AKK.2; `watch-log.sh` um `var ignoriert`, `ntp verworfen`, `ovr=` > 0 | Lead | E.4 | OK, Gegenproben FEHL bzw. schlägt an; Wachmeldungen kommen an; Wache neu gestartet | ☐ | — |
| E.6 | **Review E** | `code-reviewer` | E.5 | Keine Zuweisung zählt doppelt, `V…` nie; Quittung unverändert; Meldeweg im Mitschnitt; NTP: kein Lesen über 48 Byte auf **keinem** Pfad, Plausibilität **vor** dem Senden von `TIME`; `ovr=` sättigt; A67 in allen drei Pfaden samt Fehlerzweig; ESP-RAM-Zuwachs beziffert | — | ☐ |
| E.7 | ESP-Version anheben | `release-engineer` | E.6 | S4 zeigt den neuen Stand (erwartet 3.2.32) | ☐ | — |
| E.8 | `CHANGELOG.md`-Eintrag E | Lead | E.6 | Inhalt gleich dem Release | ☐ | — |
| E.9 | **AKZ.3**, Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | E.7, E.8, **S2.9** | `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| E.10 | **Einspielen:** Wache läuft (bestätigt) → `curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"` → `./tools/smoke-device.sh` (samt Update-Quelle), `./tools/install-app.sh --check` | Lead | E.9 | neue ESP-Version; Update-Quelle richtig (DIR-009); PWA-Dateien vollständig (DIR-017) | — | — |
| E.11 | **G3**: Mitschnitt des OTA (Δ`v`, Vorher +2, L380), danach 60 min lesend | Lead | E.10 | **AKQ.2/AKU.3/AKO.2** | — | — |

---

## Schritt P — PWA-Release: C35

**Einspielreihenfolge:** nur PWA, **nach G3** (Firmware zuerst, DIR-002).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P.1 | **C35 (A)** nach `design.md` §11 (Ent-6): `update_status` im Hauptmodul beim Öffnen und höchstens alle 5 min; Update-/Wartungsmodul wie heute, beim Wechsel sofort; Fehlschlag zählt als Abruf. **Prüfstand c35** | `pwa-developer` | **E.9** | **AKC.1**; Gegenprobe FEHL; Kodierung und Zeilenende von `app.js` | ☐ | — |
| P.2 | **Prüfstand c35 ablegen** | Lead | P.1 | OK, Gegenprobe FEHL; S8 und `innerhtml.mjs` grün | ☐ | — |
| P.3 | **Review P** | `code-reviewer` | P.2 | Keine neue `innerHTML`-Stelle, kein leeres `catch`; Zeitpunkt des letzten Abrufs auf jedem Pfad gesetzt; Moduswechsel holt sofort | — | ☐ |
| P.4 | `APP_VERSION` **und** `CACHE_NAME` anheben | `release-engineer` | P.3 | S4 zeigt den neuen Stand (erwartet 1.4.96) | ☐ | — |
| P.5 | `CHANGELOG.md`-Eintrag P | Lead | P.3 | Inhalt gleich dem Release | ☐ | — |
| P.6 | **AKZ.3**, `git status` (R2: still), Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | P.4, P.5, **E.11** | `guardrails.sh --full` Exit 0; Tag gepusht | ☐ | — |
| P.7 | **Einspielen:** `./tools/install-app.sh --check`, `./tools/install-app.sh` | Lead | P.6 | neue `APP_VERSION` am Gerät | — | — |
| P.8 | **G4**: `./tools/check-pwa.sh` (DIR-016), Smoketest, 15 min Hauptmodul offen mit Zählung im Mitschnitt | Lead | P.7 | **AKC.2** | — | — |

---

## Schritt M — Abschlusslauf A21

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| M.1 | **Abschlusslauf** nach `design.md` §13: 30 min, drei Tabs (Haupt-, System-, Updatemodul), Wetterabrufe, **Setter-Burst** 14 × Wetterort mit Rückstellung (Ent-14 c); Logwache parallel; Auswertung mit 0.1 | Lead | **P.8** | **AKM.1**, **AKM.2**: Urteil „A21 schliessen" oder „neu fassen" mit Begründung | — | — |

---

## Schritt T — Testplan und Durchlauf

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T.1 | **`TESTPLAN-PWA.md` nachführen** (über 0.5 hinaus): S47 mit Soll „Farbe nach Animation aus gleich vorher" (A23); Fall Spiel mit Setter (A53); S25/S26 mit dem Hinweis auf A57 (C) — bei 0/4095 nie abgewiesen, `numvar 17/18` direkt danach protokollieren; Hauptmodul-Abrufzahl (C35); V3-Auswertung um `night: start aus`, `time: sprung`, `spiel: … verworfen`, `ldr: … abgewiesen`, `var ignoriert`, `ntp verworfen`, `ovr=` | `doc-writer` | Freigabe | S9/S11 grün; keine Versionsnummer; jeder neue Fall mit Instrument und Rückstellung | ☐ | — |
| T.2 | **Testdurchlauf Phasen 0–4 und 9** mit dem Plan aus 0.5 und T.1 (Ent-14 d) | `pwa-tester` | **M.1**, T.1 | **AKZ.6**; `watch-log.sh` mitgelesen; Mitschnitt ausgewertet. Unvollständig ⇒ **nicht abgenommen** | — | — |

---

## Schritt Z — Abschluss

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: A62 (Ära, neue Grenze 2099), A69, A68, A23, A53, **A57 schliessen mit G1-Beleg** (S1); A60, L359, L360 (S2); A35, L391 (E); C35 (P); A21 nach M.1; A67 mit offenem Gerätenachweis (nächster ESP-OTA); neue Befunde; ToDo-Liste nachgeführt | `doc-writer` | T.2 | S10 grün; jedes „erledigt" mit Beleg | ☐ | — |
| Z.2 | `knowledge/architecture-checklist.md`: I²C-Zeit je Byte auf den in G2 **gemessenen** Stand | `doc-writer` | S2.9 | S9/S11 grün | ☐ | — |
| Z.3 | `CLAUDE.md`, Abschnitt Hardware: I²C-Wartezeit je Byte auf den Stand nach A60 | Lead | S2.9 | S9/S11 grün; keine Versionsnummer neu | ☐ | — |
| Z.4 | **Spec-Endstand** | `spec-writer` | Z.1 | Jede Abweichung offen benannt | — | — |

---

## Einspielreihenfolge, fertig zum Einfügen

Der Lead spielt ein; vor **jedem** Schritt läuft `./tools/watch-log.sh`.

1. **S1 (am Tag):** `./tools/flash-stm.sh --check` · `./tools/flash-stm.sh` ·
   `./tools/smoke-device.sh` — danach G1 mit dem **eigenen STM-Reset im Aus-Fenster** (Ent-14 a).
2. **S2 (nach G1):** `./tools/flash-stm.sh --check` · `./tools/flash-stm.sh` ·
   `./tools/smoke-device.sh` — danach G2.
3. **E (nach G2):** Wache bestätigt · `curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"` ·
   `./tools/smoke-device.sh` · `./tools/install-app.sh --check` — danach G3.
4. **P (nach G3):** `./tools/install-app.sh --check` · `./tools/install-app.sh` ·
   `./tools/check-pwa.sh` · `./tools/smoke-device.sh` — danach G4, M, T.

**Firmware vor PWA**, STM vor ESP (A67: die STM-Hälfte muss stehen, bevor der ESP ankündigt).
**Nie parallel:** Zwischen zwei Einspielvorgängen liegt jeweils der Gerätenachweis des vorigen
(R3b).

## Abschluss

- [ ] Alle Tasks erledigt oder mit Grund ausgelassen
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release
- [ ] AKZ.3 vor jedem Release-Build geprüft und im Bericht
- [ ] Flash-Gate nach jedem STM-Task gemessen und berichtet
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt — oder ausdrücklich als offen berichtet
- [ ] Vier Releases (S1, S2, E, P) gebaut, ausgerollt, committet, getaggt, gepusht
