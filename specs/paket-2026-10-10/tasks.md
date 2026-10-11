# Tasks — Paket 2026-10-10 „Testbefunde und Altlasten" (A59, A2, A50, A55, C47, C55, C44, B44–B47)

**Erstellt:** 2026-10-10, Stand `675e3fd`; am selben Tag mit den Ergebnissen aus Schritt V
nachgeführt (neuer Schritt S0). **Am 11.10.2026 nachgeführt — Endstand** (Abschnitt „Endstand"
unten, Kästchen und Vermerke aller Schritte); am selben Tag ergänzt um Z.1 (`eef9d13`), Z.2
(`3d3d8d9`) und den Push der Tags. Momentaufnahme (DIR-006); der lebende Stand steht in
`BEFUNDE.md`.

**Status: umgesetzt, ausgerollt und abgenommen mit Vorbehalten; alle Tasks abgeschlossen.** Alle
fünf Releases sind gebaut, getaggt, gepusht und eingespielt; der `pwa-tester`-Durchlauf (T.2) hat
AKZ.6 **mit Vorbehalten** abgenommen (L387). Z.1 bis Z.3 sind erledigt.

**Status bei der Freigabe: freigegeben** — vom Nutzer am 10.10.2026, samt der Entscheidungen
Ent-1 bis Ent-14 (`requirements.md`), alle nach Empfehlung. Die Umsetzung beginnt mit S0.1.

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

## Endstand 11.10.2026

**Woher die Kästchen kommen.** Gerätenachweise und Prüfstandzahlen stehen in `BEFUNDE.md`
(L364, L371–L390; Massnahmen A2, A50, A55, A59, C44, C47, C55, B44–B47, B49, Massnahme 16) und in
`CHANGELOG.md` (die fünf Einträge „Paket 2026-10-10"). Die Kästchen der Umsetzungs-, Bau- und
Review-Tasks sind dort, wo kein Bericht vorliegt, **aus den Commits und Release-Tags gefolgert**:
Ein Release setzt sie laut Spalte „Hängt ab von" voraus. Spalte **G** ist durchgehend so
gefolgert — einzeln gemeldete Guardrail-Läufe lagen dem `spec-writer` nicht vor. Die Review-Runden
von P.6/P.7 hat der Lead gemeldet; das Ergebnis von T.2 steht als L387 in `BEFUNDE.md`.

**Releases dieses Pakets** — fünf, wie geplant, je eine Komponente:

| Tag | Commit | Komponente | Inhalt |
|---|---|---|---|
| `release/3.2.26-3.2.30-1.4.94` | `6ab8f6b` | STM 3.2.26 | S0 — `my_gmtime()` mit 32 Bit |
| `release/3.2.27-3.2.30-1.4.94` | `86aeddb` | STM 3.2.27 | S1 — A59, A2 |
| `release/3.2.28-3.2.30-1.4.94` | `c55365c` | STM 3.2.28 | S2 — A50, A55 |
| `release/3.2.28-3.2.31-1.4.94` | `2bdbec0` | ESP 3.2.31 | E — C47, C55, C44, Ent-11 |
| `release/3.2.28-3.2.31-1.4.95` | `021932a` | PWA 1.4.95 | P — B44–B47, Regeln C55/C44, A2-Anzeige, Nacharbeit aus P.6/P.7 |

Am Gerät stehen damit STM 3.2.28, ESP 3.2.31, PWA 1.4.95. **Alle fünf Tags liegen auf dem Remote**
(vom Lead mit `git ls-remote --tags` geprüft); der Zweig enthält alle fünf Release-Commits.

**Flash F103 (Gate ≥ 1'024):** Ausgang 1'120 · nach S0 **2'176** · nach S1 **1'376** · nach S2
**1'908** Byte frei. Das Gate hielt in jedem Release. **S1 kostete rund 800 Byte** (mit LTO) statt
der in V geschätzten 75–125 Byte samt Logzeilen; **S2 sparte rund 530 Byte** statt 180–250. Was S1
ohne S0 gekostet hätte, ist nicht gemessen; bei ähnlichen Kosten hätte es das Gate deutlich
verfehlt — Ent-5 war nötiger als gerechnet.

**Abweichungen von der Spec, offen benannt:**

1. **Zwei Gerätefenster kürzer als 60 Minuten, Release-Builds vor ihrem Ende.** G0 lief von
   21:21 bis zum S1-Flash um rund 21:55, also rund 35 min; der S1-Release-Build (21:53) lag davor
   (S1.7 hängt an S0.8). G2 lief vom S2-Flash 22:56 bis zum ESP-OTA 23:06, also rund 10 min; der
   E-Release-Build (23:04) lag darin (E.6 hängt an S2.9). Auch der S2-Release-Build (22:22) lag
   noch im 60-Minuten-Fenster von G1 (bis 22:55); **eingespielt** wurde S2 erst danach. Die
   Zuordenbarkeit der Wirkungen (Zweck von R3b) ist gewahrt, weil jedes **Einspielen** nach dem
   vorigen Nachweis kam; die verlangte **Dauer** von G0 und G2 ist nicht erreicht. Die Zeiten
   stammen aus `BEFUNDE.md` (L372, L377, L380, L381), den Namen der Release-ZIPs und den
   Commit-Zeiten.
2. **AKP.4 nicht wörtlich erfüllt:** Beim OTA stieg `v` um **2**, nicht um 0 oder 1 — beide
   Zeitüberschreitungen fielen aber **vor** den Neustart des ESP, in das Fenster, in dem er seine
   Firmware lädt (L380). Zwischen `CAP var-crc` und `IPADDRESS` keine; die Wirkung von A55 ist
   damit belegt, der Rest ist ein neuer Befund (L380/A67).
3. **Prüfstandnamen verschoben:** `t15` war schon vergeben. Die ESP-Prüfstände heissen **t16**
   (C47, dazu `t16/static.sh` für AKX.2), **t17** (C55/C44, Fall-Tabelle `esp/t17/regeln.tsv`) und
   **t18** (Fehlerwert 255 bei einer Änderung der Temperaturkorrektur — **über die Spec hinaus**,
   zu Ent-11). Die statische Prüfung AKF.1 steckt im Prüfstand FR, die Map-Prüfung AKG.2 ist
   Guardrail S8b.
4. **AKR.4 nur teilweise gemeldet:** In G3 belegt sind `yyyyy` abgewiesen, `D.M` angenommen,
   Koordinaten `abc`/`181` abgewiesen, `7,2700` angenommen (L381). Die verlangten Fälle `<scri`,
   `⏰`, `  ab` und `<"&` am Gerät sind nicht gemeldet; sie decken t17 und die Regelgleichheit ab.
5. **T.2: Phase 9 Schritt 1 ohne Backup-Import** — die Werte wurden stattdessen einzeln
   zurückgestellt; **V3 lief nicht ganz nach der letzten Setter-Aktion**; Phase 3 mit 97 von 114
   Fällen.
6. **Über die Spec hinaus mitgeliefert** (gemeldet, nicht still): in S1 die Prüfung der ROM-ID
   zusätzlich am Familiencode, „erkannt" erst bei gültiger ID **und** gültigem Scratchpad,
   Sensor-EEPROM nur nach gültigem Scratchpad beschrieben, Werte unter 0 °C zählen nicht zu den
   „3 in Folge"; in E die Reparatur von Release-Notes-Zeilen über 128 Zeichen und einer letzten
   Zeile ohne Zeilenende sowie t18; in P die Nacharbeit aus drei Runden P.6 und aus P.7 (L383/B49
   behoben; Vorlagenmarkierung der Dimmkurve; Koordinatenfelder mit Minuszeichen auf dem iPhone).

**Neue Befunde aus diesem Paket** (in `BEFUNDE.md`): L371/A62 (Review S0), L373/A63 und L374/A64
(Review S1.5), L375/A65 (Umsetzer S1), L376/A66 (Umsetzer S2.2), L378/C56 und L379/C57 (Review E),
L380/A67 (G3), L382/B48 (P.7), L383/B49 (P.6, mit 1.4.95 behoben), L384/B50, L385/B51, L386/B52
(P.6). **Aus T.2** (der Durchlauf selbst ist L387):

- **L388/B53** (PWA): Zurücktippen auf den Ausgangswert hebt die Bearbeitungsmarkierung nur bei
  den Feldern des Netzwerkmoduls auf (Abweichung in K10b).
- **L389/A68** (STM): Die ESP-Kopie der Ambilight-Power veraltet, wenn ein Display-Timer das
  Ambilight mitschaltet.
- **L390/A69** (STM): Nach einem STM-Neustart im Nacht-Aus-Fenster bleibt das Display bis zum
  nächsten Ein-Timer an.

**Offen:** keine Tasks. Nicht gemeldet und deshalb nicht angekreuzt: AKZ.3 je Release, die
Kodierungs- und Zeilenendfeststellungen (AKZ.9), einzelne Guardrail-Läufe.

---

## Schritt V — Vorbereitung — erledigt

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| V.1 | Flash-Kosten von A59/A2/A50/A55, Sparmöglichkeiten ohne Verhaltensänderung, Detailfragen zu A2/A50/A55 | `firmware-analyst` | — | **Erledigt 10.10.2026, gemeldet.** Ergebnisse in `design.md` §1; die 952 Byte von `my_gmtime()` hat der Lead an der Map nachgeprüft. Daraus: Schritt S0, Ent-5, Ent-7, Ent-8, Ent-13; Nebenbefunde für Z.2/Z.3. **Endstand:** Die Flash-Schätzung für A2 lag weit daneben — S1 kostete rund 800 statt 75–125 Byte | — | ☑ |

---

## Schritt S0 — STM-Release: `my_gmtime()` mit 32 Bit (Ent-5)

**Einspielreihenfolge:** nur STM. Entfällt, wenn Ent-5 „nein" lautet — dann kippt die Reihenfolge
in S1/S2 (`design.md` §1.2): A50 zuerst.

**Endstand:** ausgerollt als STM 3.2.26 (`release/3.2.26-3.2.30-1.4.94`, Commit `6ab8f6b`).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S0.1 | **`my_gmtime()`** (`src/base/base.c:450-531`) nach `design.md` §2: Schnittstelle bleibt, intern `uint32_t`, Schleifenlogik und `IS_LEAP_YEAR` wörtlich; prüfen, ob `timeserver.c:250-256` angepasst werden muss (die Map entscheidet). **Prüfstand GT** im Scratchpad | `stm-developer` | Freigabe, Ent-5 | **AKG.1** am Prüfstand, Fallzahl (über 300'000), Abweichungen der **alten** Fassung gegen Host-`gmtime_r()` als Befund berichtet; Sabotage FEHL. Kodierung und Zeilenende von `base.c` festgestellt (DIR-015). **Endstand:** GT **692'546 Fälle** (jeder Tag 1970–2106, jeder Monatswechsel, Grenzen 2000, 2038, 2100, 2106, Zufall), neu gegen alt **und** gegen `gmtime_r()` **null Abweichungen** — auch die alte Fassung wich nicht ab; Sabotage an der Jahrhundertregel schlägt an (L372). Kodierung und Zeilenende nicht einzeln gemeldet | ☑ | — |
| S0.2 | **Flash-Gate und Map:** Testbau F103 | Lead | S0.1 | **AKG.2** (beide Symbole fehlen in der Map; Gegenprobe gegen die Ausgangs-Map schlägt an), **AKG.3** (Rest um ≥ 900 Byte gestiegen, Zahl gemeldet), **AKZ.F**. **Endstand:** **2'176 statt 1'120 Byte frei** (+1'056); `__udivmoddi4`/`__aeabi_ldivmod` nicht mehr in der Map (L372) | — | — |
| S0.3 | **Prüfstand GT ablegen** (`tools/checks/auszug/stm/gt/`), Map-Prüfung als Werkzeug | Lead | S0.1 | OK gegen den Baum; Sabotage FEHL; **Zähl nach**. **Endstand:** GT abgelegt (`0890f81`); die Map-Prüfung ist **Guardrail S8b** (`8208dac`) und meldet eine fehlende Map als „nicht geprüft" statt als OK (`9f3ffec`) | ☑ | — |
| S0.4 | **Review** | `firmware-analyst` | S0.2, S0.3 | Gleiches Ergebnis für jede `uint32`-Eingabe, auch `tm_wday`/`tm_yday`? Kein Überlauf in `day * 24 * 3600` (`:470`, `:497`)? Keine 64-Bit-Stelle mehr im Pfad? Aufrufer unverändert oder begründet geändert? **Endstand:** bestanden; Nebenbefund **L371/A62** (ein leerer Zeittext vom ESP stellte die Uhr auf den 07.02.2036 — unabhängig von S0, alt wie neu) | — | ☑ |
| S0.5 | STM-Version anheben | `release-engineer` | S0.4 | S4 zeigt den neuen Stand. **Endstand:** 3.2.26 | ☑ | — |
| S0.6 | **AKZ.3**, `git status`, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S0.5 | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht. **Endstand:** `wordclock-release-2026-10-10-211956.zip`, Tag `release/3.2.26-3.2.30-1.4.94`, auf dem Remote. AKZ.3 dem `spec-writer` nicht gemeldet | ☑ | — |
| S0.7 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` | **Nutzer** | S0.6 | erwartete STM-Version. **Endstand:** eingespielt | — | — |
| S0.8 | **G0**, lesend (`design.md` §10) | Lead | S0.7 | **AKG.4**; Smoketest; 60 min ohne Neustart; `watch-log.sh` mitgelesen. **Endstand (L372):** Der ESP schickte `TIME 4000648905` (21:21:45), unabhängig umgerechnet 21:21:45 MESZ; danach `read rtc 21:22:44` zur Wanduhr 21:22:44,673, `wday=6` (Samstag) — **kein Sprung**. **Abweichung:** Das Fenster lief nur bis zum S1-Flash, rund **35 min** statt 60; ein Timer mit Wochentagsbezug und der Smoketest sind nicht gemeldet | — | — |

---

## Schritt S1 — STM-Release: A59 und A2

**Endstand:** ausgerollt als STM 3.2.27 (`release/3.2.27-3.2.30-1.4.94`, Commit `86aeddb`).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S1.1 | **A59** nach `design.md` §3: `LDR_MIN_VALUE_NUM_VAR`/`LDR_MAX_VALUE_NUM_VAR` (`src/main.c:1876-1886`) setzen bei `val ≤ 4095` und schreiben über `ldr_write_config_to_eep()`; Reihenfolge **nicht** sperren; Debugzeile ohne „readonly". **Prüfstand L59** | `stm-developer` | **S0.6** (bei Ent-5 = nein: Freigabe) | **AKL.1**, **AKL.2** am Prüfstand, Fallzahl; Gegenprobe gegen `release/3.2.25-3.2.30-1.4.94` FEHL. Keine zweite EEPROM-Schreibfunktion (`git diff`). Kodierung und Zeilenende von `main.c`. **Endstand:** L59 **40 Fälle**, Gegenprobe gegen **3.2.26** FEHL (gleichwertig: 3.2.26 trägt den A59-Zweig unverändert); min ≥ max wird angenommen | ☑ | — |
| S1.2 | **A2** nach `design.md` §4: (1) eine CRC-8 für Scratchpad und ROM-ID (`ds18xx.c`); (2) `is_up = 1` erst nach gültiger ROM-ID; (3) `gtemp.index = 255` bei jedem Lese- oder CRC-Fehler (`tempsensor.c`); (4) Logzeile `DS18xxx temperature: ungueltig` (`main.c:4365`); (5) Neuerkennung nach Ent-6 im Zweig `main.c:4348-4376` mit den Zeilen `ds18xx erkannt`/`ds18xx verloren`; (6) `DS18XX_IS_UP` bei jedem Wechsel senden (`vars.c:1430` öffentlich, **keine zweite** Sendefunktion). **Prüfstand DS** | `stm-developer` | S1.1 | **AKT.1–AKT.4** am Prüfstand, Fallzahl (≥ 74 für AKT.1), der L18-Fall (falsche ROM-ID beim Start ⇒ erkannt im nächsten Minutenfenster) ausdrücklich; Gegenprobe gegen den S0-Stand FEHL. Wortlaut der drei Logzeilen wie in `requirements.md`. Kodierung und Zeilenende von `ds18xx.c`, `tempsensor.c`, `vars.c`, `vars.h`. **Endstand:** DS **193 Fälle** (nachgebildeter 1-Wire-Bus, CRC gegen drei Referenzvektoren, L18-Fall), gegen 3.2.26 FEHL. **Über die Spec hinaus, gemeldet:** ROM-ID zusätzlich am Familiencode geprüft; „erkannt" erst bei gültiger ID **und** gültigem Scratchpad; Sensor-EEPROM nur nach gültigem Scratchpad beschrieben; Werte unter 0 °C zählen nicht zu den „3 in Folge". Nebenbefund **L375/A65** | ☑ | — |
| S1.3 | **Flash-Gate**, Testbau F103 **nach S1.1 und nach S1.2** | Lead | S1.1; S1.2 | **AKZ.F**, Zuwachs je Task neben V; darunter **anhalten und vorlegen** (Ent-5, Sparstufen `design.md` §1.2). **Endstand:** nach S1 **1'376 Byte frei** (vorher 2'176) — rund **800 Byte** mit LTO statt der in V geschätzten 75–125; Gate gehalten. Die Zahlen je Task sind dem `spec-writer` nicht gemeldet | — | — |
| S1.4 | **Prüfstände L59 und DS ablegen**; `watch-log.sh` um `ds18xx verloren` und `DS18xxx temperature: ungueltig` erweitern | Lead | S1.2 | Beide OK, Gegenproben FEHL; je Wachmeldung eine eingespielte Zeile, die anschlägt **und ankommt** (DIR-014); Wache danach neu gestartet (L362). **Endstand:** abgelegt, Wache erweitert (`7864862`). Die eingespielten Gegenproben der Wache sind nicht einzeln gemeldet | ☑ | — |
| S1.5 | **Review** | `firmware-analyst` | S1.3, S1.4 | **A59:** nur 0..4095, derselbe Schreibweg, Reihenfolge frei, Start- und Laufzeitverhalten unverändert. **A2:** CRC richtig (Polynom, Bitreihenfolge, Startwert) gegen ein Datenblattbeispiel; `is_up` auf **jedem** Pfad richtig; 255 auf jedem Fehlerpfad; Nullbyte-Scratchpad (`design.md` §4.1 ●) beantwortet; `ds18xx_init()` mehrfach aufrufbar (`onewire.c:237-239`); **AKT.5** mit Rechnung; keine neue `watchdog_reload()`-Stelle (S7). **Endstand:** durchgeführt; Befunde **L373/A63** (1-Wire-Leseslot tastet nach 15–25 µs ab, Datenblatt ≤ 15 µs — erst messen) und **L374/A64** (Einschaltwert 85 °C besteht die CRC). Die Antworten zum Nullbyte-Scratchpad und die Rechnung zu AKT.5 lagen dem `spec-writer` nicht vor | — | ☑ |
| S1.6 | STM-Version anheben | `release-engineer` | S1.5 | S4 zeigt den neuen Stand. **Endstand:** 3.2.27 | ☑ | — |
| S1.7 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S1.6, **S0.8** | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht. **Endstand:** `wordclock-release-2026-10-10-215256.zip`, Tag `release/3.2.27-3.2.30-1.4.94`, auf dem Remote. **Abweichung:** gebaut, bevor G0 seine 60 min erreicht hatte (S0.8) | ☑ | — |
| S1.8 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` | **Nutzer** | S1.7 | erwartete STM-Version. **Endstand:** eingespielt | — | — |
| S1.9 | **G1** (`design.md` §10): (1) **LDR-Bereinigung** mit dem Zielwert aus Ent-3, Freigabe Ent-1: `ldr_min_value_set`, dann `ldr_max_value_set`; (2) **60 min lesend** für A2 | Lead | S1.8 | **AKL.3** (`eep a=225 n=2`, `eep a=227 n=2` in Mitschnitt **und** `/api/stm32_log`; ESP-Kopie = Zielwert; Zielwert als vom Nutzer bestätigt genannt); **AKT.6**; Smoketest; `watch-log.sh` mitgelesen. **Endstand (L364, L373, L377):** Nach dem Flash zeigte die ESP-Kopie erstmals die STM-Werte **5/26**. `ldr_min_value_set?value=0` als `(CMC N110000*4e77)` ⇒ `eep a=225 n=2 z=1 ms=24`; `ldr_max_value_set?value=4095` als `(CMC N12ff0f*031a)` ⇒ `eep a=227 n=2 z=1 ms=25`; jetzt **0/4095**, der vom Nutzer bestätigte Zielwert (Ent-3). Fenster **21:55–22:55**: **60 DS18xx-Messungen gültig** (27,5 °C), 0 `ungueltig`, 0 `ds18xx verloren`, keine Exception, kein Neustart, keine Zeitüberschreitung, `d=0`. Ob die `eep`-Zeilen auch in `/api/stm32_log` standen, ist nicht gemeldet | — | — |

---

## Schritt S2 — STM-Release: A50 und A55

**Endstand:** ausgerollt als STM 3.2.28 (`release/3.2.28-3.2.30-1.4.94`, Commit `c55365c`).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S2.1 | **A50** nach `design.md` §5 und Ent-7: `fault_reset()` (`src/main.c:590-613`) ohne jede `log_`-Ausgabe; Wartezeit rund 15 s (F1c geeicht oder F1b mit gerechneter Dauer), dann `NVIC_SystemReset()`. **Prüfstand FR** | `stm-developer` | **S1.7** | **AKF.1**, **AKF.2** am Prüfstand; Bericht nennt Zählerquelle bzw. gerechnete Dauer je Ziel; Gegenprobe gegen den S1-Stand FEHL. **Endstand:** **F1c** — SysTick-Zähler gepollt, Takt aus dem RCC, auch vor `SystemInit`, 15 s; FR **65 Fälle**, beide Ziele, gegen 3.2.27 FEHL | ☑ | — |
| S2.2 | **A55** nach `design.md` §6 (Ent-8 = P3): im Zweig `CAP var-crc` (`src/esp8266/esp8266.c:543-553`) `esp8266.is_online = 0`, Kommentar mit Begründung und Verweis auf `main.c:3945`/`:3953`. **Prüfstand EP**; W2 muss weiter bestehen | `stm-developer` | S2.1 | **AKP.1**, **AKP.3** am Prüfstand, Fallzahl; Gegenprobe gegen den S1-Stand FEHL. Kodierung und Zeilenende von `esp8266.c`. **Endstand:** EP **63 Fälle**, `v` +0..1 statt +2..4, gegen 3.2.27 FEHL. Am Code belegt und gemeldet: **L376/A66** (gehen `IPADDRESS` und alle drei `SYNCVARS` verloren, bleibt der STM offline bis zum nächsten WLAN-Ereignis). Dass W2 weiter mit 37 Fällen besteht, ist nicht einzeln gemeldet | ☑ | — |
| S2.3 | **Flash-Gate**, Testbau F103 **nach S2.1 und nach S2.2** | Lead | S2.1; S2.2 | **AKF.3**, **AKZ.F**. **Endstand:** nach S2 **1'908 Byte frei** (vorher 1'376) — rund **530 Byte gespart** statt 180–250 | — | — |
| S2.4 | **Prüfstände FR und EP ablegen**; statische Prüfung „kein `log_` in `fault_reset()`" | Lead | S2.2 | OK; Gegenproben FEHL bzw. schlägt an; W2 weiter 37 Fälle OK. **Endstand:** abgelegt (`3dafdcb`); die statische Prüfung ist Teil von FR (`stm/fr/extract.py` zählt `log_`-Aufrufe in `fault_reset()`) | ☑ | — |
| S2.5 | **Review** | `firmware-analyst` | S2.3, S2.4 | **A50:** Reset auf jedem Pfad, auch vor `log_init()`/`watchdog_init()`; Zählerquelle läuft im HardFault auf **beiden** Zielen (F1c) bzw. Dauer gerechnet (F1b); Wartezeit über 12 s und unter 20 s. **A55:** die drei Fragen aus `design.md` §6 ● (SYNCVARS/IPADDRESS nach jedem Start, auch AP-Modus; Dauerzustand bei Zeilenverlust; Reihenfolge `CAP` vor `IPADDRESS`) **am ESP-Code beantwortet**; **AKP.2**, **AKP.5**. **Endstand:** bestanden; F1c bleibt (Zählerquelle auf beiden Zielen). Die Antwort zum Dauerzustand ist L376/A66 | — | ☑ |
| S2.6 | STM-Version anheben | `release-engineer` | S2.5 | S4 zeigt den neuen Stand. **Endstand:** 3.2.28 | ☑ | — |
| S2.7 | **AKZ.3**, Build, Gegenprobe des Gates (S8b), Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S2.6, **S1.9** | `guardrails.sh --full` Exit 0; Rest ≥ 1'024; Tag gepusht. **Endstand:** `wordclock-release-2026-10-10-222244.zip`, Tag `release/3.2.28-3.2.30-1.4.94`, auf dem Remote. **Abweichung:** gebaut um 22:22, während das 60-Minuten-Fenster von G1 noch lief (bis 22:55); eingespielt erst danach | ☑ | — |
| S2.8 | **Nutzer:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` — der Reset danach ist der Persistenznachweis für A59 (Ent-4 a) | **Nutzer** | S2.7 | erwartete STM-Version. **Endstand:** eingespielt um 22:56 | — | — |
| S2.9 | **G2**, lesend | Lead | S2.8 | **AKL.4** (`numvar[17]/[18]` = Zielwert nach dem Vollabgleich); **AKF.4**; 60 min ohne Exception, ohne Neustart, `v` konstant; Smoketest; `watch-log.sh` mitgelesen. **Endstand (L364, L377):** Nach dem Neustart meldet der Vollabgleich **0/4095** — die Werte stehen im EEPROM (**AKL.4 erfüllt**). `CAP var-crc` ohne folgende Zeitüberschreitung, Sensor gleich wieder gültig (27 °C), Smoketest 31/0. **Abweichung:** Das Fenster lief nur bis zum ESP-OTA um 23:06, rund **10 min** statt 60 | — | — |

---

## Schritt E — ESP-Release: C47, C55, C44

**Einspielreihenfolge:** nur ESP. **Der OTA ist zugleich der Gerätenachweis von A55** — die Wache
muss **vor** dem OTA laufen (E.7).

**Endstand:** ausgerollt als ESP 3.2.31 (`release/3.2.28-3.2.31-1.4.94`, Commit `2bdbec0`).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| E.0 | **Vorprüfung C44, lesend:** Wie kam `8,5400` in L340 an Koordinaten — API (ersetzt `,`), Legacy `savelonlat` (ersetzt ebenfalls), Import? Gegen den Mitschnitt bzw. die Abzüge von S.26 | `esp-developer` | **S2.7** | Antwort mit Beleg; ist L340 an dieser Stelle falsch, steht das im Bericht für Z.1. **Endstand:** L340 präzisiert — `8,5400` wurde schon vorher als `8.5400` gespeichert; der Befund war an dieser Stelle ungenau, `<"&` dagegen kam roh durch (`BEFUNDE.md`, C44) | — | — |
| E.1 | **C47** nach `design.md` §7: neue Maskierung (fünf Zeichen, sonst byte-gleich); **Bestandsliste** aller Wertausgaben der Legacy-Seiten (Datei:Zeile, Gruppe maskiert/Zahl/Konstante); alle Werte maskiert, dazu `new_esp_version`/`new_wc_version`; **Release Notes nach Ent-2**. **Prüfstand t15** | `esp-developer` | E.0 | **AKX.1–AKX.3** am Prüfstand, Fallzahl; Rundlauf byte-gleich; Gegenprobe FEHL. Kodierung (UTF-8) und Zeilenende von `http.cpp` festgestellt. **Endstand:** Prüfstand heisst **t16** (`t15` war vergeben), **63 Fälle** samt RTC-Fehlerwert 255 als „invalid", gegen 3.2.30 FEHL. Erfasst: Formularfelder, SSIDs, Dateinamen und -inhalt, Versionen und Dateilisten vom Update-Server, Namen vom STM, Zwischenseite „App nicht installiert". Release Notes R2; **dazu** Zeilen über 128 Zeichen und eine letzte Zeile ohne Zeilenende repariert (gingen bisher verloren). ESP-RAM statisch +380 Byte | ☑ | — |
| E.2 | **C55 und C44** nach `design.md` §8 (Ent-10, Ent-12): je **eine** Prüffunktion, gerufen von API **und** Legacy (`savedtf`, `savelonlat`); dazu **Ent-11** (Legacy-Anzeige 255 als „invalid"), falls gewählt. **Fall-Tabelle** (eine Datei für ESP und PWA) und **Prüfstand t16** | `esp-developer` | E.1 | **AKR.1–AKR.3** am Prüfstand, mindestens 40 Fälle je Regel; Gegenprobe FEHL. **Endstand:** Prüfstand heisst **t17**, **147 Fälle** plus Orakel, Fall-Tabelle `tools/checks/auszug/esp/t17/regeln.tsv`, gegen 3.2.30 FEHL. Abweisung mit Kennung 2 und Klartext. Ent-11 umgesetzt (DS18xx **und** RTC). **Über die Spec hinaus:** Eine Änderung der Temperaturkorrektur lässt den Fehlerwert 255 unangetastet und klammert gültige Werte auf 0–125 °C wie der STM — Prüfstand **t18**, 82'944 Fälle | ☑ | — |
| E.3 | **Prüfstände t15 und t16 ablegen**, Fall-Tabelle ablegen; **statische Prüfung AKX.2** mit der Erlaubnisliste aus E.1 | Lead | E.2 | OK; Gegenproben FEHL bzw. schlägt an; **Zähl nach:** geprüfte `http_send`-Aufrufe gegen die Bestandsliste. **Endstand:** t16 und t17 abgelegt (`6513f49`), t18 (`83a2744`), RTC-Fall in t16 (`99018fe`); statische Prüfung `esp/t16/static.sh`: **788 Argumente an 83 Seitenfunktionen**. Nach Review E (M3) erweitert auf Aufrufstellen der Helfer und alle HTML-Seiten (`be5cdbc`) | ☑ | — |
| E.4 | **Review** | `code-reviewer` | E.3 | Maskierung vollständig und ohne Wertschaden (kein `?`, kein UTF-8-Eingriff); jede Wertausgabe erfasst — **eigene Stichprobe** gegen die Bestandsliste; Release Notes auch bei über zwei Zeilen geteiltem Tag und bei Zeilen über 128 Zeichen sicher; Regeln an **allen** Schreibwegen, keine zweite Fassung; Legacy-URLs ohne Parameter ohne `=` (R5). **Endstand:** bestanden mit Nacharbeit M3 (statische Prüfung erweitert, E.3). Befunde **L378/C56** (ein einzelnes ungültiges UTF-8-Byte kommt über das Legacy-Formular als U+FFFD zurück; vorbestehend; t16 bildet den Browser dafür nicht nach) und **L379/C57** (`%` am Ende in `normalize_http_parameters()`) | — | ☑ |
| E.5 | ESP-Version anheben | `release-engineer` | E.4 | S4 zeigt den neuen Stand. **Endstand:** 3.2.31 | ☑ | — |
| E.6 | **AKZ.3**, Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | E.5, **S2.9** | `guardrails.sh --full` Exit 0; Tag gepusht. **Endstand:** `wordclock-release-2026-10-10-230432.zip`, Tag `release/3.2.28-3.2.31-1.4.94`, auf dem Remote. **Abweichung:** gebaut nach rund 8 min G2 statt nach 60 | ☑ | — |
| E.7 | **Nutzer:** ESP-OTA — **erst, wenn der Lead die laufende Wache bestätigt hat** | **Nutzer** | E.6 | neue ESP-Version in `/api/update_status`. **Endstand:** OTA um 23:06; der Mitschnitt des OTA liegt vor (L380) | — | — |
| E.8 | **G3** (`design.md` §10): (1) **A55** aus dem Mitschnitt des OTA; (2) Smoketest **samt Update-Quelle** (DIR-009), `install-app.sh --check` (DIR-017); (3) **C47** Wetterort-Testwert mit Rückstellung (Freigabe Ent-1); (4) **C55/C44** Abweisungen, je ein gültiger Wert mit Rückstellung | Lead | E.7 | **AKP.4** (`v` +0 oder +1; `(CAP var-crc)` und danach `SYNCVARS`/`IPADDRESS`; Vollabgleich vollständig; kein Watchdog-Reset); **AKX.4**; **AKR.4**; Originalwerte zurück und lesend nachgeprüft; `watch-log.sh` mitgelesen. **Endstand (L380, L381), 23:06–23:09:** (1) Zwischen `CAP var-crc` und `IPADDRESS` **keine** Zeitüberschreitung (vorher vier) — **aber `v` 0 → 2** um 23:06:18 und 23:06:23 (`N10`), **vor** dem Neustart (`rst cause` 23:06:24), während der ESP seine Firmware lud: **AKP.4 nicht wörtlich erfüllt**, neuer Befund L380/A67. (2) Smoketest 31/0, PWA vollständig. (3) Wetterort `x"y<z` ⇒ Legacy-Seite `x&quot;y&lt;z`. (4) `yyyyy` abgewiesen, `D.M` angenommen; `abc`/`181` abgewiesen, `7,2700` als `7.2700` angenommen. Alles zurückgestellt (Aarberg, `D.M.Y`, 7.2682/47.0441). Heap danach 8'744 frei / 8'280 Block (vorher 9'168 / 8'712). Nicht gemeldet: `<scri`, `⏰`, `  ab`, `<"&` am Gerät; Vollständigkeit des Vollabgleichs | — | — |

---

## Schritt P — PWA-Release: B44–B47, Regeln, Anzeige A2

**Einspielreihenfolge:** nur PWA, **nach** dem ESP (die Regeln aus C55/C44 stehen zuerst im ESP).

**Endstand:** ausgerollt als PWA 1.4.95 (`release/3.2.28-3.2.31-1.4.95`, Commit `021932a`), mit der
Nacharbeit aus drei Runden P.6 und aus P.7.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P.1 | **B44** (Ent-9) und **B45** nach `design.md` §9.1–§9.2: `parseTimeInput()` ohne Rückfall; `saveTimerRow()`, `saveDfplayerAlarm()`, `saveAllTimerRows()` (zuerst alle prüfen); weitere Aufrufer von `parseTimeInput()` im Bericht; `markEditsPersisted()` leert nur die gespeicherten Felder | `pwa-developer` | **E.6** | **AKB.1**, **AKB.2** (beide Fälle, auch L33) in der Vorschau mit echten Ereignissen; Gegenproben gegen den Ausgangsstand FEHL. **Endstand:** Vorschau-Proben mit echten Ereignissen, gegen den Vorgängerstand angeschlagen (CHANGELOG 1.4.95). Dazu: Felder, die nie gespeichert werden (Kartensuche, DFPlayer-Testtitel, Dateiwahl, Update-Listen), lösen keine Rückfrage mehr aus | ☑ | — |
| P.2 | **B46** und **B47** nach `design.md` §9.3–§9.4: `eeprom_settings` auch bei `networkActive`; Hintergrundklick ohne Fokusverlust, Ziehen aus der Karte schliesst nicht. Braucht B47 Markup: **anhalten**, P.4 anstossen | `pwa-developer` | P.1 | **AKB.3**, **AKB.4** in der Vorschau; Gegenproben FEHL. **Endstand:** wie P.1; sind die Einstellungen nicht ladbar, steht ein Hinweis im Netzwerkstatus. Nebenbefund aus P.6: L385/B51 (Abruf ohne Obergrenze bei Fehlschlag) | ☑ | — |
| P.3 | **Regeln C55/C44** nach `design.md` §8 an **allen** PWA-Wegen (Speichern, Karte, Import) aus **derselben** Fall-Tabelle wie E.2; **A2-Anzeige** nach §4.4 (drei Zustände, Text in `app.js` und `i18n/en.json`) | `pwa-developer` | P.2, **E.3** | **AKR.5** (Regelgleichheit gegen die Fall-Tabelle, Gegenprobe FEHL, abweichende Regel ⇒ Meldung kommt an); **AKT.7** in der Vorschau. **Endstand:** Regelgleichheit **147 Fälle**; ein Backup mit ungültigen Feldern nennt diese beim Import; „nicht gefunden" und „Messwert ungültig" getrennt | ☑ | — |
| P.4 | *Nur falls nötig:* Markup für B47 | `ui-developer` | P.2 | wie P.2. **Endstand:** Ob B47 selbst Markup brauchte, lag dem `spec-writer` nicht vor. Im Release steht Markup an den Koordinatenfeldern (`index.html`, `inputmode="text"` mit Begründung: die Dezimaltastatur von iOS hat kein Minuszeichen) — aus der Nacharbeit, nicht aus B47 | ☑ | — |
| P.5 | **Prüfstand Regelgleichheit ablegen**; Vorschau-Läufe AKB.1–AKB.4 und AKT.7 nachvollziehen | Lead | P.3 (P.4) | OK; Gegenprobe FEHL; S8 (Umlaute) und `innerhtml.mjs` grün. **Endstand:** `tools/checks/auszug/pwa/regeln/` abgelegt (`f3fb3fa`), Vorschau-Mock mit gültigem Datumsformat. Vorschau-Proben P.1–P.3 und H1–H4, jede gegen den Vorgängerstand angeschlagen | ☑ | — |
| P.6 | **Review PWA** | `code-reviewer` | P.5 | Keine neue `innerHTML`-Stelle; kein leeres `catch`; WLAN-Schlüssel aus `eeprom_settings` an keiner neuen Stelle (AKB.3); B45-Gegenfall L33; `saveAllTimerRows()` sendet bei einer ungültigen Zeile nichts; Regeln wörtlich gleich der Fall-Tabelle. **Endstand: drei Runden** — **BLOCKIERT, BLOCKIERT, BESTANDEN mit Nacharbeit**. Befunde: **L383/B49** (Update-Listen beim Neuladen, mit 1.4.95 behoben), **L384/B50**, **L385/B51**, **L386/B52** (offen) | — | ☑ |
| P.7 | **Review UI** | `ui-reviewer` | P.5 | B47: Fokus nach Hintergrundklick, nach Escape, nach Ziehen; Tab-Falle unverändert; A2-Anzeige und B44-Meldung verständlich, Du-Form, echte Umlaute. **Endstand: bestanden.** Befund **L382/B48** (Fehlermeldungen auf dem iPhone hinter der Tastatur, offen) | — | ☑ |
| P.8 | `APP_VERSION` **und** `CACHE_NAME` anheben | `release-engineer` | P.6, P.7 | S4 zeigt den neuen Stand. **Endstand:** 1.4.95 | ☑ | — |
| P.9 | **AKZ.3**, `git status` (R2: still), Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | P.8, **E.8** | `guardrails.sh --full` Exit 0; Tag gepusht. **Endstand:** `wordclock-release-2026-10-11-003756.zip`, Tag `release/3.2.28-3.2.31-1.4.95`, auf dem Remote | ☑ | — |
| P.10 | **Nutzer:** `./tools/install-app.sh --check`, `./tools/install-app.sh` | **Nutzer** | P.9 | neue `APP_VERSION` am Gerät. **Endstand:** eingespielt | — | — |
| P.11 | **G4, erster Teil:** `./tools/check-pwa.sh` (DIR-016), Smoketest | Lead | P.10 | **AKB.5**; Smoketest ohne Fehlschlag. **Endstand:** Browserprüfung **8/0**, Smoketest **31/0** | — | — |

---

## Schritt T — Testplan und Durchlauf

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T.1 | **`TESTPLAN-PWA.md` nachführen:** (1) S25/S26 **rücknehmbar** über S27 — vorher den Wert lesen (nach G1 ist die ESP-Kopie gleich dem STM), danach zurück; Freigabepflicht entfällt bzw. wie andere rücknehmbare S-Fälle; (2) S27-Soll zusätzlich mit `eep a=225 n=2`/`eep a=227 n=2` im Logring als STM-Beleg; Text „STM verwirft als readonly" entfernt, mit Verweis auf A59; (3) neue Fälle: leeres Zeitfeld (Timer, Ambilight-Timer, DFPlayer, „Alle speichern"), zwei Felder ändern, eines speichern, Modul wechseln (und Gegenfall), AP-SSID ohne Wartungsbesuch, Hintergrundklick im Kartenmodal, Datumsformat- und Koordinatenwerte aus der Fall-Tabelle (abgewiesen und gültig), DS18xx-Anzeige; (4) Abschnitt 12 (Altlasten): der blinde Fleck „STM-seitige LDR-Grenzen" ist mit A59 über S27 und den Reset in T.2 abgedeckt | `doc-writer` | Freigabe, Ent-3, Ent-9, Ent-10, Ent-12 | S9/S11 grün; keine Versionsnummer; jeder neue Fall nennt Instrument und Rückstellung. **Endstand:** erledigt, Commit `31b3318` — S25/S26 rücknehmbar, Bedienfälle **K1–K17**, Reset in Phase 9 (Schritt 9, V3) | ☑ | — |
| T.2 | **Testdurchlauf Phasen 0–4 und 9** mit dem Plan aus T.1, danach **STM-Reset** (Ent-4 b, Freigabe) und Phase 9 erneut | `pwa-tester` | P.11, T.1 | **AKZ.6**: S25/S26 gefahren und über S27 zurückgestellt; nach dem Reset zeigt Phase 9 die LDR-Grenzen auf dem Wert vor dem Durchlauf; alle neuen Fälle aus T.1 gefahren. `watch-log.sh` mitgelesen; Mitschnitt ausgewertet (Exceptions, Neustarts, `d`, `v`, `ds18xx …`, `ungueltig`). Unvollständig ⇒ **nicht abgenommen**. **Endstand (L387): „AKZ.6 abgenommen, mit Vorbehalten".** S25/S26 mit `eep`-Beleg gefahren und zurückgestellt; LDR-Grenzen **0/4095** nach dem STM-Reset gleich dem Vorher-Wert; K1–K17 gefahren (**17/17**), davon **K10b abgewichen** (L388/B53); Phase 3: **97 von 114** gefahren; `--soll` ohne unerklärte Abweichung; Mitschnitt ohne Exception und ohne Neustart ausser dem freigegebenen Reset. **Abweichungen:** Phase 9 Schritt 1 **ohne Backup-Import** — die Werte wurden einzeln zurückgestellt; **V3 nicht ganz nach der letzten Setter-Aktion**. Neue Befunde L388/B53, L389/A68, L390/A69 | — | — |

---

## Schritt Z — Abschluss

**Endstand:** Z.1 bis Z.3 erledigt.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: A59/L351/L364, A2 (mit der neuen Lesart von L18: falsch gelesene ROM-ID, nicht gescheiterte Erkennung), A50/L313, A55/L327, C47/L346, C55/L369, C44/L340 (mit dem Ergebnis aus E.0), B44–B47/L365–L368 je mit Datei:Zeile dieses Stands; S0 als eigener Eintrag (Sparmassnahme, 952 Byte); neue Befunde aus den Reviews; **AKS.6** in der Fassung aus AKP.4 vermerkt | `doc-writer` | T.2 | S10 läuft durch; jedes „erledigt" mit Beleg; jeder neue offene Befund in der ToDo-Liste. **Endstand:** erledigt, Commit `eef9d13`. A2 gestrichen, mit der neuen Lesart von L18; Massnahme 16 erledigt; B44–B47 und L365–L368 mit dem Gerätebeleg aus T.2; T.2 als L387; neu L388/B53, L389/A68, L390/A69. Vorher schon eingetragen: A50, A55, A59, C44, C47, C55, S0 als L372, die Review-Befunde L371–L386 | ☑ | — |
| Z.2 | `CLAUDE.md`: „Offene technische Themen" 1 (DS18xx) auf den Stand nach S1; die Zeilenangabe „`main.c:3140`" für `temp_init()` (steht bei `:3734`, V) — ohne neue Zeilennummer, wo es geht | Lead | T.2 | S9/S11 grün; keine Versionsnummer neu. **Endstand:** erledigt, Commit `3d3d8d9` — Thema 1 steht auf dem Stand nach A2. Die Zeilenangabe war schon vor S0 entfernt (`551fa68`, „`temp_init()` in `main.c`") | ☑ | — |
| Z.3 | `tools/logger/README.md:352`: die beschriebene Fault-Ausgabe gab es nie (V); Stand nach A50 | Lead | S2.9 | Text stimmt mit `fault_reset()` überein. **Endstand:** erledigt, Commit `6e203b2`; Vorarbeit schon in `551fa68` | ☑ | — |

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

**Endstand:** So eingespielt, in dieser Reihenfolge und je eine Komponente: STM 3.2.26, 3.2.27,
3.2.28, ESP 3.2.31, PWA 1.4.95. Zwischen zwei Einspielvorgängen lag jeweils ein Nachweis; G0 und
G2 waren dabei kürzer als 60 min (Abschnitt „Endstand", Punkt 1).

## Abschluss

- [x] Alle Tasks erledigt oder mit Grund ausgelassen — Z.1 (`eef9d13`), Z.2 (`3d3d8d9`) und Z.3
      (`6e203b2`) erledigt
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release — **dem `spec-writer` nicht
      einzeln gemeldet**; die Releases setzen es voraus
- [ ] AKZ.3 vor jedem Release-Build geprüft und im Bericht — **nicht gemeldet**; dass jedes
      Release genau eine Komponente anhebt, zeigen die Tags
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt — oder ausdrücklich als offen berichtet
      — **berichtet** in `requirements.md`, Abschnitt „Endstand 11.10.2026"; nicht erfüllt bzw.
      offen u. a. AKP.4 (wörtlich), AKF.4 (Fensterdauer von G2), AKG.4 (Timerteil nicht
      gemeldet), AKR.4 (teilweise gemeldet)
- [x] Fünf Releases (S0, S1, S2, E, P) gebaut, ausgerollt, committet, getaggt, gepusht; Flash-Umfang
      und Reihenfolge je Release benannt — alle fünf Tags auf dem Remote (Lead, `git ls-remote --tags`)
