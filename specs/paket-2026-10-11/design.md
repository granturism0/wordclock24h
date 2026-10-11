# Design — Paket 2026-10-11 „Nachtfenster, Netzzeit, Spiele, I²C, Brücke" (A23, A69, A53, A57, A60, A61, A62, A63, A35, A21, A34, C35)

**Erstellt:** 2026-10-11, Stand `5c34d80`. **Freigegeben** vom Nutzer am 11.10.2026 (Ent-16);
Ent-1 bis Ent-15, Ent-3b, Ent-14e, **Ent-15b** (Freitakten), Ent-17 („Release teilen") und Ent-18
(Matrix-Lauf) entschieden und eingearbeitet. **Stand:** **S1 ausgeliefert** (STM 3.2.29, Commit
`a6065d5`, Tag gepusht), G1-Tagteil belegt, Nachtteil offen; **S2 Code fertig und reviewt, nicht
gebaut**. **Offen:** der zweite STM-Reset im Aus-Fenster in G3 — vor G3 beim Nutzer zu bestätigen
(§15.2). Momentaufnahme (DIR-006).

**Entscheidungsvermerk 11.10.2026.** **Ent-1** (a), DC und IR führen die Sicherung nach (§5) ·
**Ent-2** nur Displayliste, erstes übernommenes `TIME`, übersprungenes Ereignis nachholen,
Startsequenz wie heute — präzisiert (M2): „IP erst zeigen, dann aus" (§3) · **Ent-3 Ära-Rechnung
jetzt** — gegen die Empfehlung (§2) · **Ent-3b** Bestätigungsregel wie vorgeschlagen; die Umsetzung
mit zusätzlichem Mindestabstand von 10 s liegt im Rahmen (§2.4) · **Ent-4** V2, keine
Inaktivitätsgrenze (§6) · **Ent-5** (b) (§9) · **Ent-6** (A) (§11) · **Ent-7 (C)** — gegen die
Empfehlung (§5a) · **Ent-8** Werkzeug (§12.1) · **Ent-9** A34 schliessen · **Ent-10** A63
beobachten · **Ent-11** A68 ja (§4) · **Ent-12** A67 ja (§7) · **Ent-13** NTP samt Plausibilität,
jetzt tragend (§10.1) · **Ent-14** alle Freigaben erteilt; (a) gilt dem einen Reset in G1 (§15) ·
**Ent-14e** AKE.2 ausdrücklich freigegeben (§15.2) · **Ent-15** nur Fristen, **Ent-15b** Bus beim
Fristablauf freitakten (§8) · **Ent-16** freigegeben · **Ent-17** Release teilen: S1 = A62, A69,
A68 (dazu N1, M1, M2); S3 = Flash schaffen, dann A23, A53, A57, A67-STM (§0, §1) · **Ent-18**
Matrix-Lauf aller 72 Varianten, Gate für alle 72; vom Lead angefordert vor **jedem** STM-Release,
das Flash kostet — S1, S2, S3 —, jeweils **nach der Versionsanhebung** (§1.6).

**Schrittfolge:** **0 (ohne Firmware) → S1 ☑ (STM 3.2.29: A62 mit Ära, A69 samt M1/M2, A68, N1) →
S2 (STM: A60 mit Freitakten) → S3 (STM: Flash-Analyse und Einsparungen, dann A23, A53, A57,
A67-STM) → E (ESP: A35, NTP, `ovr=`, A67-ESP) → P (PWA: C35) → M (Abschlusslauf A21) → T
(`pwa-tester`) → Z**.

**Grundlage:** zwei Analysen des `firmware-analyst` vom 11.10.2026 (STM; Brücke), vom Lead
zusammengefasst. Stichproben des `spec-writer` am Code `5c34d80`, alle bestätigt: `night.c:155`,
`main.c:3274` (`strtoul`), `main.c:1425-1456` (`set_display_power`), `main.c:1605-1618`
(LDR-RPC mit Rückmeldung), `main.c:4286-4304` (Timerzweig), `main.c:3836` (`watchdog_init`),
`timeserver.c:248-256`, `base.c:461`, `rtc.c:204`, `:241`, `ldr.c:128-137`, `:168-226`,
`ntp.cpp:18`, `:74-83`, `i2c.c:26`, `:157`, `:173`, `:413`, `:472`, `snake.c:291`,
`tetris.c:655-680`, `timeserver.c:67`, `:196`, `app.js:2131`, `:3187`, `:13100`, `:13266`,
`http.cpp:10077-10086`, `:12384`, `:12771`, `:13009`, `ESP-uclock.ino:500`, `:1247`; kein
`hasOverrun` und keine Zeitrechnung (`gmtime`, `time_t`, `2208988800`) im ESP-Code ausser
`ntp.cpp`. Die Zeilennummern in `src/**` haben sich seither durch S1 und S2 verschoben.

---

## 0. Warum diese Reihenfolge

### 0.1 Die Regeln aus `make release-zip` und R3b

`make release-zip` baut STM, ESP **und** die PWA-Assets in einem Zug. Liegt im Baum eine Änderung
an einer Komponente, deren Version der Build **nicht** anhebt, entsteht ein Fabrikat mit alter
Versionsnummer und neuem Inhalt (Lehre aus Paket 2026-10-09). Für mehrere STM-Releases
hintereinander gilt dasselbe **innerhalb** der Komponente.

| Regel | Umsetzung in `tasks.md` |
|---|---|
| Produktcode des **nächsten Releases** entsteht erst **nach dem Release-Build** des vorigen | S2.1 hängt an S1.19 (eingehalten), S3.2 an S2.7, E.1 an S3.16, P.1 an E.9 |
| Der **Release-Build** folgt erst **nach dem Gerätenachweis** des vorigen Releases | S2.7 an S1.21 (**G1 vollständig**, auch Nachtteil und AKW.5), S3.16 an S2.9 (G2), E.9 an S3.18 (G3), P.6 an E.11 (G4); M.1 an P.8 (G5); T.2 an M.1 (G6) |
| Jeder **STM-Release-Build, der Flash kostet,** folgt erst nach einem **grünen Matrix-Lauf auf dem angehobenen Stand** (Ent-18, Festlegung des Leads) | S1.19 an S1.16 und S1.16a (eingehalten); S2.7 an S2.6a (nach S2.5); S3.16 an S3.15a (nach S3.14) |
| Erlaubt parallel: Gerätenachweis des vorigen und Codearbeit des nächsten | So geschehen: S2-Code während G1 |
| Erlaubt jederzeit: Doku, Spec, Werkzeug, **lesende Analysen** | Die Flash-Analyse S3.1 darf früh beginnen; ihre Zahlen gelten gegen die S2-Map. **Ausnahme:** 0.5 (Testplan V3/S1) muss **vor** dem Nachtteil von G1 fertig sein |
| Innerhalb eines STM-Releases **nacheinander**, je Task ein Testbau | Das Flash-Gate wird je Task und je Einsparung gemessen (§1) |

### 0.2 Release-Zuschnitt (Ent-17, Ent-18)

| # | Release | Inhalt | Komponente | Code erst nach | Build erst nach | Nachweis | Stand |
|---|---|---|---|---|---|---|---|
| — | **Schritt 0** | A61-Werkzeug, Messung A61/A34, Befunde (L391 u. a.), Testplan V3/S1 | Werkzeug, Doku | Freigabe | — | lesend | offen |
| 1 | **STM-Release 1** | A62 mit Ära, A69 samt M1/M2, A68, N1 | STM | Freigabe | Nachprüfung, Matrix grün, Warnungen | **G1** | **☑ 3.2.29**, G1-Nachtteil offen |
| 2 | **STM-Release 2, isoliert** | A60 + L360, Freitakten (Ent-15b) | STM | Build S1 | G1 vollständig, Matrix grün nach Versionsanhebung | **G2** | Code ☑, Review ☑, nicht gebaut |
| 3 | **STM-Release 3** | Flash-Analyse, Einsparungen; dann A23, A53, A57 (C), A67-STM | STM | Build S2 | G2, Matrix grün nach Versionsanhebung | **G3** | offen |
| 4 | **ESP-Release** | A35 (b), NTP-Puffer + Plausibilität, `ovr=`, A67-ESP | ESP | Build S3 | G3 | **G4** | offen |
| 5 | **PWA-Release** | C35 (A) | PWA | Build E | G4 | **G5** | offen |
| — | **Abschlusslauf A21** | 30 min Last, Setter-Burst | — | — | G5 | **G6** | offen |
| — | **`pwa-tester`** | Phasen 0–4 und 9 (DIR-012) | — | — | G6, T.1 | AKZ.6 | offen |
| — | **Schritt Z** | BEFUNDE, Checkliste, CLAUDE.md, README-CMAKE (vorgezogen), Spec-Endstand | Doku | — | T.2 (Z.2/Z.3 nach G2, Z.5 nach S1.15) | — | Z.5 in Arbeit |

| Frage | Antwort |
|---|---|
| Warum **S1 nur A62, A69, A68** (dazu N1, M1, M2)? | Ent-17. A68 passte (+44) und gehört fachlich zu A69. M1 und M2 sind Korrekturen aus dem Review an A69. N1 hat der Lead vorgezogen, damit M2 in den Flash passt |
| Warum **S2 vor S3**? | (1) **A60 bleibt isoliert messbar** — die Vorher-Messung (AKI.2) lief in G1 auf 3.2.29, die Nachher-Messung folgt in G2; dazwischen ändert sich nur A60. (2) **Die Flash-Analyse arbeitet gegen die endgültige Map.** (3) A60 war klein und fertig spezifiziert |
| **Risiko dieser Reihenfolge** | Gemessen: S2 kostet die engste Variante **108 Byte** (1'416 → 1'308), Freitakten eingeschlossen; 72/72 grün. Das Risiko für S2 ist ausgeräumt; der Engpass liegt jetzt vollständig in S3 |
| Warum **S3 vor E**? | Die ESP-Hälfte von A67 setzt die STM-Hälfte voraus. DIR-002: Firmware zuerst, STM vor ESP |
| Warum im S3 **zuerst Platz, dann die vier Punkte**? | Ent-17. Die vier Punkte brauchen nach dem gemessenen Faktor **rund 400–900 Byte**; die engste Variante hat nach S2 **284 Byte** Spielraum (§1.3). Ohne Einsparungen reicht es nicht |
| Reihenfolge der **vier Punkte** in S3 | A23, A53, A57, A67-STM — A67-STM zuletzt, weil seine ESP-Hälfte an ihm hängt |
| Warum alle S3-Punkte in **einem** Release? | Getrennte Nachweise: `dspcolor[0]` (A23), `spiel: … verworfen` (A53), `ldr: … abgewiesen` (A57), Prüfstand a67 (A67). Keine ändert I²C-Zeiten |
| Warum **E vor P**, **M nach P**, **`pwa-tester` nach M**? | Firmware vor PWA; der Abschlusslauf misst den **neuen** Normalbetrieb nach C35 (§13); der `pwa-tester` erzeugt selbst Setterlast und würde M verfälschen |

---

## 1. Flash F103

### 1.1 Gemessen statt geschätzt (11.10.2026, Testbauten des Leads)

| Stand | geschätzt | **gemessen** (Teilbuild F103 WC12h SK6812-RGBW) | frei | engste Matrix-Variante |
|---|---|---|---|---|
| Ausgang (3.2.28) | — | — | **1'908** | — |
| nach S1.1 — A62 samt Bestätigungsregel | −110…−20 | **+208** | 1'700 | — |
| nach S1.3 — A69 samt Sprüngen | +160…+310 | **+568** | 1'132 | — |
| nach S1.5 — A68 | +10…+30 | **+44** | 1'088 | — |
| nach S1.9 — **N1** malloc-Stubs | — | **−668** | 1'756 | — |
| nach S1.11 — **M1 + M2** = **3.2.29** | 80–100 | **+148** | **1'608** | **1'416** (72/72) |
| nach S2.1 — **A60** | 0…+60 | **+108** | 1'500 | — |
| … Freitakten im BUSY-Pfad | — | **+4** | 1'496 | — |
| … Freitakten **nach jeder Frist** (Ent-15b) | — | **+32** | **1'464** | **1'308** (72/72, Vorabwert) |

Keine 64-Bit-Routinen in der Map, `strtoul` verschwunden (AKN.4 erfüllt). Die Differenz zwischen
Teilbuild und engster Variante war nach S1 **192 Byte**, nach S2 **156 Byte** — sie ist also nicht
fest.

**N1 — malloc-Stubs.** Starke Definitionen von `_malloc_r`, `_realloc_r` und `_free_r` in
`src/stubs/stubs.c` mit `__attribute__((used, externally_visible))`; ohne das Attribut verwirft
LTO `_realloc_r` (Linkfehler belegt). Die Module `mallocr`, `freer`, `reallocr`, `msizer`, `sbrkr`
und `mlock` fehlen danach in der Map — **668 Byte**. Verhaltensneutral, beide Reviews BESTANDEN;
die 7 Warnungen `unused parameter` mit `(void)` beseitigt, Bestand **17 statt 18** (S1.16a).

### 1.2 Lehre — gilt für den Rest des Pakets

**Die Flash-Schätzungen lagen um den Faktor 1,5 bis 3 daneben** (A62 sogar im Vorzeichen; A60 rund
doppelt; im Vorpaket bei S1 um den Faktor 6 bis 10). Daraus:

1. **Geplant wird mit der Messung je Task, nicht mit Summen von Schätzungen.**
2. Eine Schätzung dient nur noch als **Untergrenze** und zur Reihenfolge; für die Planung zählt sie
   **mit Faktor 3**.
3. Was vor einem Task gilt, ist der **gemessene Rest des letzten Testbaus**.
4. Eine **Einsparung** ist ebenso zu messen wie ein Zuwachs — N1 zeigt, dass ein einzelnes Symbol
   ganze Bibliotheksmodule nach sich ziehen kann.
5. **Geplant wird ab der engsten Matrix-Variante, nicht ab dem Teilbuild.** Der Abstand ist nicht
   fest (192 Byte nach S1, 156 nach S2); ein Teilbuild über 1'024 Byte garantiert nicht, dass alle
   72 halten.

### 1.3 Gate und Planung je Release

**Gate: 1'024 Byte frei, nach jedem STM-Task und nach jeder Einsparung** (Teilbuild), dazu **für
alle 72 Varianten** im Matrix-Lauf vor jedem STM-Release-Build, der Flash kostet — auf dem
angehobenen Stand (§1.6). Darunter: **anhalten und vorlegen** — keine Sparstufe ohne neue
Rückfrage (Ent-17). **Planungsgrundlage ist die engste Matrix-Variante.**

| Release | engste Variante davor | Inhalt | Ergebnis bzw. Planung |
|---|---|---|---|
| **S1** ☑ | 1'908 (Teilbuild) | A62, A69, A68, N1, M1, M2 | **gemessen:** Teilbuild 1'608, engste Variante **1'416** — 392 über dem Gate |
| **S2** | 1'416 | A60 samt Freitakten | **gemessen (Testbau, Vorabwert):** Teilbuild 1'464, engste Variante **1'308** — **284 über dem Gate**, 72/72 grün. Massgeblich ist der Matrix-Lauf S2.6a nach der Versionsanhebung |
| **S3** | **1'308** | A23, A53, A57, A67-STM (Schätzung zusammen 190–294) | mit Faktor 2 bis 3 **rund 400–900 Byte** gegen einen Spielraum von **284** — **ohne Einsparungen reicht es nicht.** Die Flash-Analyse (§1.5) muss **rund 120–620 Byte** schaffen; jeder Punkt einzeln mit Gate. **Matrix-Lauf nach der Versionsanhebung ist feste Abhängigkeit des Release-Builds** (S3.15a) |

### 1.4 Sparstufen — **nicht gewählt** (Ent-17)

Diese Stufen standen in der Spec; der Nutzer hat **keine** davon gewählt. Sie greifen **nur nach
einer neuen Rückfrage**:

1. Ent-2 (3) zurücknehmen (keine Sprungauswertung).
2. A67-STM zurückstellen (dann entfällt auch die ESP-Hälfte).
3. A53 V2 → V2b.
4. Logzeilentexte kürzen.

Ebenfalls nicht ohne Rückfrage: `atoi`-Ersatz (Verhaltensänderung). **Nicht als Sparstufe** gelten
die Bestätigungsregel (§2.4), A57 (C), M2, das Freitakten (Ent-15b) und die geschützten Logzeilen
`set_display_power: new=…` und `i2c: frist abgelaufen`.

### 1.5 Flash-Analyse vor S3 (S3.1, `firmware-analyst`)

**Gegenstand:** die Map des **S2-Release-Builds** **und die Map der engsten Matrix-Variante**
(`wc24h-stm32f103-sk6812-rgbw`). Gelesen werden die **gemessenen** Grössen — Symbolgrössen und
Abschnitte (`.text.*`, `.rodata.*`) —, keine Schätzungen.

**Je Kandidat:** Symbol bzw. Text · Abschnitt · Bytes laut Map · Verhaltensänderung ja/nein ·
**Abhängigkeiten**: sucht ein Werkzeug (`watch-log.sh`, Auswerteregel 0.1, Smoketest,
`measure-log.sh`), der Testplan, ein AK dieser Spec oder `BEFUNDE.md` nach der Zeile?

**Mindestens zu prüfen:**

| Kandidat | Hinweis |
|---|---|
| Toter Zweig `FILE ` in `esp8266_get_message()` (A52) | Rund 25 Byte im Vorpaket genannt; an der Map messen |
| Doppelte und lange Texte | gleiche Zeichenketten an mehreren Stellen; lange Debugtexte |
| Die **zweite** `log_printf` in `set_display_power()` (am Ende der Funktion) | Kandidat. **Die erste, `set_display_power: new=…`, ist geschützt** (AKW.4) |
| Debugtexte im normalen Bau | **Erst an der Map belegen, ob sie im Fabrikat landen** — im Vorpaket sparte C42 nichts |
| Weitere Bibliotheksmodule nach dem Muster von N1 | Zieht ein einzelnes Symbol ein ganzes Modul herein? |
| Was wc24h zusätzlich belegt | Warum ist die wc24h-Variante rund 156 Byte grösser — gibt es dort einen eigenen Hebel? |

**Geschützt und kein Kandidat:** `set_display_power: new=…` (AKW.4), `i2c: frist abgelaufen`
(Wachalarm, AKI.3), `error: cannot reset I2C bus`.

**Ergebnis:** Liste mit Summe, gemessen gegen den Bedarf von rund 120–620 Byte; Vorschlag der
Reihenfolge. **Umsetzung (S3.2):** eine Einsparung je Schritt, nach jeder ein Testbau (S3.3).
Verhaltensneutral heisst: kein Zweig, der heute erreichbar ist, ändert seine Wirkung; keine
Logzeile, nach der etwas sucht, verschwindet oder ändert ihren Wortlaut — sonst Rückfrage.

### 1.6 Matrix-Lauf aller 72 STM-Varianten (Ent-18)

**Umfang:** {wc12h, wc24h} × {F103, F401CC-25, F401CC-8, F411CE-25, F411CE-8, Nucleo401,
Nucleo411} × fünf LED-Typen = 70, dazu je eine F407-TFT-Variante = **72**.

**Umsetzung (S1.15, `release-engineer`, erledigt):** `make matrix`, `make clean-matrix`; geändert
`CMakeLists.txt` (`EBP_TARGET`, `LED`, `STARTUP`, `OUTPUT_DIR`, Option `WORDCLOCK_MATRIX`),
`Makefile`; neu `cmake/matrix-report.sh`. Normaler Build byte-identisch; `wc12h-f103`- und
`f411ce-25`-RGBW byte-identisch zum Teilbuild. Exit ≠ 0 bei Bau- oder Gate-Fehler mit
Variantennamen; Gegenproben: Schwelle 2000 ⇒ 3 × GATE; `-O0` baut nicht. F411 mit
`startup_stm32f411xe.S` (Abweichung vom Original im Makefile-Kommentar und in `README-CMAKE.md`,
Z.5 in Arbeit).

**Läufe:**

| Lauf | Task | Ergebnis |
|---|---|---|
| vor S1, vor dem Release-Build wiederholt | S1.16 | 72/72, 0 unter dem Gate; engste `wc24h-stm32f103-sk6812-rgbw` **1'416**; gegen das Original +1,6 bis +7,4 % |
| nach S2 (Testbau) | S2.2 | 72/72 grün; engste Variante **1'308** — **nur Vorabwert** |
| vor S2-Release-Build, nach der Versionsanhebung S2.5 | S2.6a | offen; **feste Abhängigkeit** von S2.7 |
| vor S3-Release-Build, nach der Versionsanhebung S3.14 | S3.15a | offen; **feste Abhängigkeit** von S3.16 |

**Wann:** Ent-18 sagt „nur auf Anforderung". **Der Lead hat angefordert:** vor **jedem**
STM-Release, das Flash kostet — S1, S2, S3 —, jeweils als feste Abhängigkeit des Release-Builds und
**nach der Versionsanhebung**, auf genau dem Stand, der ausgeliefert wird (wie S1.16). Läufe aus
Testbauten zählen **nur als Vorabwert**. Begründung des Leads: rund 1 Minute Aufwand, und das Gate
soll den ausgelieferten Stand prüfen. Vor E und P läuft sie nicht (kein STM-Flash). Läuft sie, gilt
das Gate für alle 72. **Nebenwirkung:** belegt die Build-Verzeichnisse für seine Dauer (R1).

---

## 2. S1 — A62: Netzzeit mit Ära-Rechnung (Ent-3) — ausgeliefert mit 3.2.29

### 2.1 Befund am Code

| Stelle | Befund | Folge für die Ära |
|---|---|---|
| ESP `ntp.cpp:78-83` | Setzt die Sekunde aus Byte 40–43 als 32-Bit-Zahl zusammen und sendet `TIME ` + `Serial.println(unsigned long)` — **dezimal, ohne führende Nullen** | Ab 07.02.2036 06:28:16 UTC beginnt die Zahl wieder bei 0 und wird **kürzer**: Ära-1-Werte haben 1 bis 10 Ziffern (10 Ziffern erst ab 2067). Der Parser muss **1–10** annehmen |
| ESP sonst | Keine eigene Zeitrechnung; die Uhrzeit für Anzeige und API kommt vom STM (`var_send_tm`) | Der ESP braucht **keine** Pivot-Regel |
| STM `timeserver.c:253` | `curtime = (time_t) (seconds_since_1900 - 2208988800U)` — beide Operanden `uint32`, die Differenz **läuft modulo 2³² um** | Für Ära-1-Werte ergibt die Differenz **genau die Unixzeit der Ära 1** |
| STM `base.c:461` (S0) | `my_gmtime()` rechnet intern mit `uint32_t` | Gültig bis 2106-02-07 06:28:15 UTC — ohne 64-Bit-Arithmetik; **S8b bleibt grün** |
| STM `rtc.c:204`, `:241` | Die RTC speichert das Jahr als zwei BCD-Ziffern | **Neue Grenze der Kette: 31.12.2099.** Nicht Teil dieses Pakets, in L371 benannt |
| STM `timeserver.c:254` | Zeitzone wird auf `time_t` addiert, `my_gmtime()` kürzt auf 32 Bit | Wirkt erst an der 2106-Grenze — ohne Belang |

**Folgerung:** Das 2036-Problem war **nicht** die Arithmetik, sondern die Gültigkeitsfrage.

### 2.2 Die Regel: eine Grenze auf der Unixzeit

```
ESP8266_TIME:
    n = parse (esp8266.u.time)          // 1..10 Ziffern, dann '\0'; Ueberlauf (> 4'294'967'295) => ungueltig
    u = (uint32_t) (n - 2208988800U)    // Aera 0 und 1 in einem Schritt (Umlauf modulo 2^32)
    gueltig  <=>  u >= 1'767'225'600    // Unix 2026-01-01 00:00:00 UTC
```

| NTP-Sekunde n | Ära | `u` | Ergebnis |
|---|---|---|---|
| 3'976'214'400 … 4'294'967'295 | 0 | 1'767'225'600 … 2'085'978'495 | gültig, 2026-01-01 … 2036-02-07 06:28:15 |
| 0 … 2'208'988'799 | 1 | 2'085'978'496 … 4'294'967'295 | gültig, 2036-02-07 06:28:16 … 2106-02-07 06:28:15 |
| 2'208'988'800 … 3'976'214'399 | **Totbereich** | 0 … 1'767'225'599 | **verworfen** |

Logzeile bei Verwerfung: **`time: ungueltig, verworfen`**. `net_time_countdown` bleibt, wie er
beim Anfordern gesetzt wurde (3800 s).

### 2.3 Warum eine zweite Prüfung nötig ist — und warum keine feste Zeitgrenze

Mit dem Pivot sind zwei Fehlerbilder **gültige** Zeiten: **die 0** (2036-02-07 06:28:16) und
**eine verstümmelte Zeile** (`4000648905` ohne eine Ziffer ⇒ Oktober 2048). Eine feste Grenze am
STM hilft nicht: Jedes N, das die Ära-Rechnung sinnvoll macht, enthält diese Jahre. **Darum zwei
Prüfungen:** die **ESP-Plausibilität** (Ent-13, tragend, §10.1) und die **Bestätigungsregel** am
STM (Ent-3b, §2.4).

### 2.4 Bestätigungsregel (Ent-3b — entschieden 11.10.2026)

```
Zustand (rund 16 Byte RAM):  ref, ref_up, ref_ok;  cand, cand_up, cand_ok;  schnell (0..3)

bei gueltigem n:
    erwartet = ref + (uptime - ref_up)
    wenn ref_ok und |n - erwartet| <= 600 s:   uebernehmen
    sonst wenn cand_ok und |n - (cand + (uptime - cand_up))| <= 5 s:   uebernehmen   // bestaetigt
    sonst:
        cand = n, cand_up = uptime, cand_ok = 1
        log "time: sprung, bestaetigung angefordert"
        wenn schnell < 3:  net_time_countdown = 15, schnell++
uebernehmen:  ref = n, ref_up = uptime, ref_ok = 1, cand_ok = 0, schnell = 0, dann wie heute
```

Nach jedem STM-Start gibt es keine Referenz — die erste Antwort wird Kandidat. **Umsetzung (Lead),
im Rahmen von Ent-3b:** zusätzlich **Mindestabstand von 10 s** zwischen Kandidat und Bestätigung
(strenger); `cmd: set time to` **unbedingt** geloggt (Beobachtbarkeit). Review S1.8 BESTANDEN.
**Am Gerät (G1):** `time: sprung` 20:44:39 → `cmd: set time to` 20:44:54.

**Verworfene Alternativen zu Ent-3b:** (i) keine Bestätigung, nur die ESP-Plausibilität — eine
verstümmelte Zeile stellte Uhr und RTC bis zu eine Stunde um Jahrzehnte vor; (ii) Protokollwechsel
mit festen 10 Stellen — dreht die Reihenfolge ESP vor STM, alte ESPs würden nach 2036 abgewiesen.

### 2.5 Wechselwirkungen

- **A69 Ent-2 (2) und M1:** „erstes übernommenes `TIME`" heisst „erstes **bestätigtes**" — auch
  dann, wenn die RTC beim Start ein Jahr vor 2026 meldet.
- **A69 Ent-2 (3):** Ein bestätigter Sprung ist ein Sprung — die Nachholregel greift.
- **Zeitzone:** Betrag über 12 ⇒ nicht übernehmen (Vorgabe), eine Logzeile.

**Prüfstand a62** (§14) mit Ärengrenze, Totbereich, Formfehlern, Python-`datetime` als Referenz,
Bestätigung mit simulierter `uptime` samt Mindestabstand; Sabotagen je FEHL.

---

## 3. S1 — A69: Nachtfenster (Ent-2, samt M1 und M2) — ausgeliefert mit 3.2.29

### 3.1 Die Funktion

```
night_last_event (is_ambilight, wday, m, *out_minutes_back) -> 0 = keins, 1 = aus, 2 = ein
    fuer k = 0..7:  tag = (wday + 7 - k) % 7
        fuer jeden aktiven Timer, der fuer tag gilt; bei k = 0 nur minutes <= m:
            abstand = k*1440 + m - minutes;  kleinster abstand gewinnt
```

`wochentag_gilt()` ist die herausgelöste Prüfung aus `night.c:166-180`; `night_check_night_times()`
ruft sie ebenfalls. Gleichstand: erster passender Timer in Tabellenreihenfolge (Restpunkt §3.4).

### 3.2 Wo sie greift (nach Review S1)

| Stelle | Wirkung |
|---|---|
| **Start**, nach RTC-Lesen, vor `watchdog_init()` — **nur bei `gmain.year >= 2026`** (**M1**) | Eine RTC mit leerer Batterie meldet 2000-01-01 und gilt sonst als gültig. Bei einem Jahr vor 2026 wertet das **erste bestätigte** `TIME` aus |
| **Ergebnis „aus" beim Start** (**M2**: „IP erst zeigen, dann aus") | **Vormerken** (`night_off_due`); angewandt **nach dem IP-Lauftext** im Innenzweig der **unveränderten** Restore-Bedingung um `pending_ticker_restore`; ohne IP nach **60 s**; jedes Schalten verwirft die Vormerkung. Angewandt wie beim Timer, Logzeile `night: start aus (hh:mm)` |
| **Ambilight (Ent-2 (1))** | Nur über die Displayliste mitgeschaltet |
| **Vorwärtssprung > 1 min (Ent-2 (3))** | Das jüngste Ereignis **im übersprungenen Intervall** wird angewandt; rückwärts nichts |

**Restore-Bedingung:** Keine der vier Teilbedingungen geändert (Architektur-Invariante) — `git diff`
und Nachprüfung S1.13.

### 3.3 Warum nicht einfacher

„Der letzte vergangene Timer **des Tages**" verfehlt Aus-Timer vom Vortag. Darum bis zu sieben Tage
zurück — 64 Vergleiche.

### 3.4 Niedrige Restpunkte aus dem Review S1 — benannt, kein Fix

| Restpunkt | Wirkung |
|---|---|
| Gleichstand Aus/Ein in derselben Minute | Welcher Timer gilt, hängt an der Tabellenreihenfolge |
| Rücksprung über 3,5 Tage modulo Woche | Ein sehr grosser Rücksprung kann als Vorwärtssprung gelesen werden |
| `quick` nach drei Fehlwerten | Danach regulärer Takt — bis zu rund 2 h bis zur ersten Netzzeit |
| `NIGHT_OFF_AFTER_IP` ohne Frist | Kommt der Restore nie, bleibt die Vormerkung offen |
| Erstes `TIME` bei ungültiger RTC | Schaltet sofort und kann den IP-Lauftext abschneiden |
| Anderer Ticker | Ein Ticker vor dem Restore überschreibt den IP-Lauftext |

Der `doc-writer` trägt sie in Z.1 als Befunde ein.

---

## 4. S1 — A68: Ambilight-Kopie (Ent-11) — ausgeliefert mit 3.2.29

`var_send_display_ambilight_power()` öffentlich, gerufen an jeder Stelle der Bestandsliste
(mindestens `set_display_power()` bei `do_sync_ambilight`), hinter derselben `is_online`-Bedingung
wie die Display-Power. Keine zweite Sendefunktion. A56 bleibt eigener Punkt.

---

## 5. S3 — A23: Displayfarbe (Ent-1)

- `saved_display_colors` nach dem Muster `saved_ambilight_colors` (`display.c:3557`).
- **Gesetzt:** beim Laden **vor** `display_init_color_animation()` (`:3568`); in DC — **auch
  während** einer Animation; in den IR-Farbschritten im Modus 0.
- **Zurück:** in **jedem** Weg auf Modus 0 (Setter, Inkrement, Dekrement): Sicherung kopieren,
  `display_calc_dimmed_display_colors()`, **unbedingt `var_send_display_colors()`**.
- **Nicht:** kein EEPROM-Schreiben im Animationspfad; keine Änderung an der Ambilight-Sicherung.

---

## 5a. S3 — A57: „Messwert übernehmen" weist eine Inversion ab (Ent-7 = C)

```
ldr_set_min_value:  wenn adc_value >= ldr.ldr_max_value:  log "ldr: minimum abgewiesen (%u >= max %u)", nichts setzen, nichts schreiben
ldr_set_max_value:  wenn adc_value <= ldr.ldr_min_value:  log "ldr: maximum abgewiesen (%u <= min %u)", ebenso
```

- **Gleichheit wird abgewiesen** — dieselbe Regel, die der Start heilt (`ldr.c:128`).
- **Nur diese beiden Funktionen;** der Zahlen-Setter (A59) bleibt — Backup-Import (L68); L59 muss
  bestehen.
- **Rückmeldung:** Der RPC-Zweig sendet bereits `var_send_ldr_min/max_value()` (`main.c:1609`,
  `:1617`) — bei Abweisung den **alten** Wert.
- **PWA-Meldung nicht ohne neue API:** `/api/ldr_min_set` antwortet vor der Messung
  (`http.cpp:10077-10086`). Als Hinweis in Z.1.

**A57 wird erst nach G3 geschlossen** (AKE.2).

---

## 6. S3 — A53: Spiele (Ent-4 = V2)

- In beiden Spielschleifen ein Zähler für jede Zeile, die die Schleife **nicht** als `GT`/`GS`
  auswertet; Zeitablauf zählt nicht; ob eine Quittung zählt, entscheidet der `stm-developer`.
- Nach dem Spiel bei Zähler > 0 `var_sync_pending = 1` und `spiel: %u Zeilen verworfen`.
- **`snake.c:291`:** `status` initialisieren. Keine Inaktivitätsgrenze.

---

## 7. S3 und E — A67: OTA-Ankündigung (Ent-12)

| Seite | Änderung |
|---|---|
| **STM** (S3.10) | Neue Zeile mit exaktem Vergleich, Vorschlag `OTA start`; setzt **nur** `esp8266.is_online = 0`. Nicht `esp8266_cap_var_crc_seen` (Risiko R-3) |
| **ESP** (E.4) | In **allen drei** OTA-Pfaden (`http.cpp:13009`, `:7590`, `:13402`) vor dem Download; bei Fehlschlag `SYNCVARS` |

Wortlaut einmal festgelegt (S3.10), wörtlich übernommen (E.4). Am Gerät nachweisbar erst beim
**nächsten** ESP-OTA nach E.

---

## 8. S2 — A60 und L360: I²C (Ent-15, Ent-15b) — Code fertig, reviewt, nicht gebaut

### 8.1 Fristen

**Entwurf:** enges Pollen mit Frist über `delay_counter`, **dieselbe** Gesamtfrist wie heute
(5 ms); **beide** BUSY-Schleifen mit Frist; kein POS/BTF-Abschlussverfahren.

**Umsetzung (S2.1) und Abweichung, am Code begründet:** Der Entwurf nahm einen **freilaufenden**
Zähler an (`delay_counter - start`). `delay_counter` ist aber ein **Abwärtszähler** im SysTick, der
bei 0 stehen bleibt. Die Frist wird deshalb über das Herunterzählen gebildet, nicht über eine
Differenz. Die beiden BUSY-Schleifen laufen über **`i2c_wait_not_busy()`**, die bei Ablauf
**`I2C_ERROR_BUSY` (−8)** liefert. Die ISR-Frage (keine ISR nutzt `delay_*`) ist am Code belegt.

**Logzeile `i2c: frist abgelaufen`** — Entscheidung des Leads: ohne sie wären AKI.3 und die
Logwache blind für einen Fristablauf. Sie geht **nur auf die Debug-UART** (Mitschnitt, Logwache),
nicht in `/api/stm32_log`.

### 8.2 Freitakten (Ent-15b, Entscheidung des Nutzers)

Nach einem Fristablauf bleibt der Bus oft belegt; Folgezugriffe scheiterten an der SB-Frist.
Darum nach **jeder** Frist in `i2c_handle_timeout()`:

```
wenn BUSY gesetzt oder SDA liest low:
    i2c_last_clockspeed = 0
    i2c_init (i2c_clockspeed)          // = i2c_reset_bus mit 9 Takten, Pins zurueck auf AF
```

- Zuerst nur im BUSY-Pfad; vom Lead nach der Nachprüfung auf **jede** Frist erweitert.
- **Schlimmster Fall 25 ms je Zugriff**, vor `watchdog_init()` rund **0,55 s** — weit unter 20 s.
- **Toter Bus:** je Zugriff das Zeilenpaar `i2c: frist abgelaufen` und
  `error: cannot reset I2C bus`. Gedrosselt ist das nicht; bei dauerhaft totem Bus entstehen je
  I²C-Zugriff zwei Zeilen (benannt, §Checkliste).
- **Flash:** +4 (BUSY-Pfad) bzw. +32 (jede Frist) im Teilbuild.

### 8.3 Review und Erwartung

**Review S2.4: BESTANDEN**, drei Nachprüfungen BESTANDEN. **Benannt, vorbestehend, nicht geändert:**
`GPIO_RESET_BIT(SDA_PORT, SCL_PIN)` in `i2c.c` trifft auf **Nucleo** den falschen Port — ein
neuer Befund für Z.1.

**Timing:** gerechnet `ms ≈ 16·z + (0,2…0,3)·n + 1`. **Vorher gemessen (G1, 3.2.29):** `n=2`
24–26 ms, 32 Byte über 2 Seiten **92 ms**. **Erwartung laut Review:** `n=2` **16–17 ms**, 32 Byte
**34–40 ms**.

**Messpunkte G2:** Steht `i2c: frist abgelaufen` **vor** `rtc is online`, ist das der Hinweis auf
die DS1307-Probe beim Start (Fristablauf bei der Suche nach einem anderen RTC-Typ) — benennen,
nicht als Fehlschlag werten, wenn danach alles läuft. Ein Wachalarm während des Flashens wird am
Kontext geprüft (L400).

**Doku (Z.2, Z.3):** Checkliste und `CLAUDE.md` auf den in G2 gemessenen Stand.

---

## 9. E — A35: still verworfene Zuweisungen (Ent-5 = b)

- In `var_set_parameter()` jede Bedingung, an der eine Zuweisung still entfällt, an **einen**
  Zähler; `V…`-Zeilen zählen nicht. **Rückgabe unverändert** — kein NAK.
- Gedrosselt (erste sofort, danach höchstens eine je 60 s): `- var ignoriert #n code=X idx=yy`,
  **ohne Wert** (A20).
- **Weg der Meldung:** im Mitschnitt **und** in `/api/stm32_log`; `stm32_log_append()` allein
  reicht nicht (`ESP-uclock.ino:1247`) — Lehre aus L185.

---

## 10. E — NTP-Antwort und ESP-Empfangsüberlauf

### 10.1 NTP (Ent-13, nach Ent-3 tragend)

```
packetsize = ntp_udp.parsePacket ()
wenn packetsize:
    wenn packetsize != NTP_PACKET_SIZE:   verwerfen (Rest lesen ohne Puffer bzw. flush), log
    sonst: lesen; LI = buf[0] >> 6; stratum = buf[1]; transmit = buf[40..43]
           wenn LI == 3 oder stratum == 0 oder transmit == 0:  verwerfen, log, KEIN "TIME"
           sonst wie heute
```

Logzeile `- ntp verworfen` mit Grund, Weg wie §9. **Keine** Ära-Rechnung im ESP. Befund **L391**.

### 10.2 `ovr=` (für A21)

`Serial.hasOverrun()` sättigend gezählt, als `ovr=` an der Heap-Zeile (`ESP-uclock.ino:500`). RAM
4 Byte. **Kein** `setRxBufferSize` in diesem Paket.

---

## 11. P — C35 (Ent-6 = A)

- **Hauptmodul:** `update_status` beim Öffnen und danach höchstens alle **5 Minuten**.
- **Update- und Wartungsmodul:** wie heute alle 15 s; beim Wechsel dorthin **sofort**.
- Eine Variable für den letzten Abruf; Fehlschläge zählen als Abruf. Keine Änderung an ESP oder
  Anzeige.

---

## 12. Schritt 0 — ohne Firmware

### 12.1 A61-Auswerteregel (Ent-8)

Ein Werkzeug des Leads unter `tools/`, das einen Mitschnitt liest und je Vollabgleich ausgibt:

| Feld | Regel |
|---|---|
| Ende | `info: configuration sent` |
| Auslöser | die letzte `(SYNCVARS)`- bzw. `(IPADDRESS …)`-Zeile vor dem Ende nach dem vorigen Abgleich; fehlt beides, **CMC-Abweisung**; sonst **unbekannt** — nie geraten |
| Dauer | Zeitstempel Auslöser bzw. erste Sendung bis Ende |
| Δ`v` | beide Zahlen von `v=` aus den `diag`-Zeilen davor und danach |
| Zeitüberschreitungen | `var_send_buf: keine Quittung` im Intervall |
| Stillstand | Δ`u` der `diag`-Zeilen bzw. Lücke der `diag`-Folge |

Exit ≠ 0 bei über 5 s Stillstand oder über 10 Zeitüberschreitungen (L191). Gegenprobe mit
synthetischem Mitschnitt; Review `code-reviewer`.

### 12.2 Messung A61/A34

Über den ESP-OTA vom 10.10.2026 23:06 (L380), den STM-Reset vom 11.10.2026 01:58:14 (L387) und
jeden weiteren Abgleich im Langzeitmitschnitt.

### 12.3 Befund-Nachführung (`doc-writer`)

| Zeile | Nachführung |
|---|---|
| **L391 (reserviert)** | NTP-Antwort über 48 Byte überschreibt globale Variablen; vom Netz auslösbar; Massnahme in Gruppe C |
| **L331 / A57** | Schluss trägt nicht (`ldr.c:128-137` heilt beim Start und schreibt); Ent-7 (C); **Schliessung nach G3** |
| **L360 / A60** | **zwei** BUSY-Schleifen; vor `watchdog_init()` steht die Uhr für immer — mit S2 behoben, samt Freitakten (Ent-15b) |
| **L371 / A62** | Fundstelle `main.c:3274`; der ESP sendet nie leer; **Ära-Rechnung beschlossen und mit 3.2.29 ausgeliefert**; **neue Grenze 31.12.2099** |
| **L233 / A35** | Fundstelle `ESP-uclock.ino:1468-1479`; kein hoher Befund mehr |
| **A69** | Fundstelle `night.c:155`; Zeitsprung-Lücke |
| **A61** | Fundstelle `main.c:4037-4048`; Ergebnis aus 12.2 |
| **A53** | „mit A16 zusammen" überholt; `snake.c:291` |
| **A34** | geschlossen (Ent-9), Reste überführt |
| **A63** | Beobachtung, offen |
| **A21** | blinder Fleck ESP-Empfangspuffer; Schliessung erst nach G6 |
| **neu (Z.1)** | Nucleo: `GPIO_RESET_BIT(SDA_PORT, SCL_PIN)` trifft den falschen Port (Review S2.4, vorbestehend) |

### 12.4 Testplan V3/S1 (`doc-writer`, vor dem Nachtteil von G1)

Soll nach einem Reset im Aus-Fenster ab STM-Release 1: **IP-Lauftext, danach aus** (ohne IP
spätestens nach 60 s). Rückstellung nach S1 im Aus-Fenster: „wieder aus". Nach dem Reset
`time: sprung, bestaetigung angefordert` — Soll, kein Fehler.

---

## 13. M — Abschlusslauf A21 (G6)

| Quelle | Rate |
|---|---|
| Tab 1, Hauptmodul sichtbar | Selbstaktualisierung 15 s, `update_status` höchstens alle 5 min |
| Tab 2, System | `stm32_log` alle 2,5 s |
| **Tab 3, Updatemodul** | `update_status` alle 15 s — trägt nach C35 die frühere Last |
| Wetter | je ein Abruf jetzt und Vorhersage |
| Setter-Burst (Ent-14 c) | 14 Aufrufe auf den Wetterort wie A58, danach Original zurück |

**Kriterium:** AKM.2. Ergebnis in Z.

---

## 14. Prüfstände

Gebaut vom Umsetzer im Scratchpad, abgelegt vom Lead unter `tools/checks/auszug/`.

| Name | Ort | Release | Gegenstand | Gegenprobe (DIR-014) | Stand |
|---|---|---|---|---|---|
| **a62** | `stm/a62/` | S1 | Parser, Ärengrenze, Totbereich, Bestätigung samt Mindestabstand, Zeitzone | alter Stand nimmt `""`/`"0"` an; Sabotagen ⇒ FEHL | abgelegt |
| **a69** | `stm/a69/` | S1 | jüngstes Ereignis, alle 10'080 Startpunkte; M1; M2; Sprünge; A68-Sendeprotokoll | Sabotagen; alter Stand ⇒ FEHL | abgelegt |
| **Matrix** | `make matrix`, `cmake/matrix-report.sh` | S1, S2, S3 | 72 Varianten, Gate je Variante | Schwelle 2000 ⇒ 3 × GATE; `-O0` baut nicht | erfüllt |
| **a60** | `stm/a60/` | S2 | Fristen (Abwärtszähler), `i2c_wait_not_busy`, Logzeile, Freitakten nach jeder Frist | **4'998 Prüfungen**; Sabotagen `fristnie`, `busyohne`, `msec`, `ohnelog`, `ohnefrei`, `nurbusy` je FEHL; im Baum S16 **44/44 OK**; `auszug.sh --gegen release/3.2.29-3.2.31-1.4.95` meldet `FEHL  A60 I2C-Fristen …` mit **Exit 1** (41 von 44, 1 fehlgeschlagen) — die Meldung kommt an | abgelegt |
| **a23** | `stm/a23/` | S3 | Moduswechsel, Sicherung (DC, IR), Sendung | alter Stand ⇒ FEHL | — |
| **a53** | `stm/a53/` | S3 | Spielschleife, Zähler, `var_sync_pending` | alter Stand ⇒ FEHL | — |
| **a57** | `stm/a57/` | S3 | `ldr_set_min/max_value()` | alter Stand ⇒ FEHL; L59 besteht weiter | — |
| **a67** | `stm/a67/` | S3 | Zeilenparser `OTA start` | alter Stand ⇒ FEHL | — |
| **a35** | `esp/a35/` | E | Zählung, `V…` ausgenommen, Drosselung | alter Stand ⇒ FEHL | — |
| **ntp** | `esp/ntp/` | E | Paketlängen, Wächterbyte, Plausibilität | alter Stand ⇒ FEHL | — |
| **c35** | `pwa/c35/` | P | Abrufzahl je Modul | alter Stand ⇒ FEHL | — |
| **statisch AKK.2** | Lead | E | drei OTA-Pfade | Ausgangsstand ⇒ schlägt an | — |
| **abgleich** | bei 12.1 | 0 | Auswerteregel A61 | falscher Auslöser ⇒ FEHL | — |
| **gt, EP, W2, L59** | bestehen | alle | müssen **weiter bestehen** | — | — |

---

## 15. Gerätenachweise

**Für alle:** `./tools/watch-log.sh` läuft **vor** jedem Einspielen und während jedes Laufs
(DIR-013); Smoketest nach jedem Einspielen (DIR-009); schreibend nur im Rahmen von Ent-14 und
Ent-14e, mit Rückstellung. **Nie** `GET /?a`, keiner der Endpunkte aus R5. Eingespielt wird vom
**Lead**.

| Nachweis | Nach | Art | Stand |
|---|---|---|---|
| **G1** | S1-Flash | Tagtests + Reset im Fenster (Ent-14 a) | **Tagteil ☑:** AKN.6 (`time: sprung` 20:44:39 → `cmd: set time to` 20:44:54); AKI.2 vorher (`n=2` 24–26 ms, 32 Byte 92 ms, zurückgestellt). Fehlalarm der Wache beim Flashen behoben (L400). **Offen:** Nachtteil (AKW.4, AKN.7), AKW.5 |
| **G2** | S2-Flash | S25/S26 und 32-Byte-Wert (zurück) + 60 min lesend | offen; Erwartung `n=2` 16–17 ms, 32 Byte 34–40 ms; Messpunkt `i2c: frist abgelaufen` vor `rtc is online` |
| **G3** | S3-Flash | schreibend; zweiter Reset im Fenster **nur nach Bestätigung durch den Nutzer** | offen |
| **G4** | ESP-OTA | lesend, 60 min | offen |
| **G5** | PWA-Hochladen | lesend | offen |
| **G6** | — | Abschlusslauf | offen |

### 15.1 Ablauf G1 (S1)

1. Vor dem Flash lesend `numvar` 17/18, Display-/Ambilight-Power festhalten. — ☑
2. S1-Flash am Tag; Smoketest; `time: sprung …`, danach `cmd: set time to …` (AKN.6). — ☑
3. Tagtests: AKN.6, **AKI.2 vorher**. — ☑
4. **Im Aus-Fenster** (nach 01:00): **ein eigener STM-Reset** (Ent-14 a) — AKW.4 (**IP-Lauftext,
   danach aus**), AKN.7. — **offen**
5. **Am Morgen:** AKW.5. Danach ist G1 abgeschlossen; der S2-Release-Build darf folgen (nach der
   Versionsanhebung S2.5 und dem Matrix-Lauf S2.6a). — **offen**

### 15.2 Ablauf G3 (S3)

1. Vor dem Flash lesend `dspcolor[0]`, `numvar` 17/18, LDR-Rohwert, `numvar[8]` festhalten.
2. S3-Flash am Tag; Smoketest.
3. Tagtests: AKD.2 Teil 1 (Regenbogen, Tageslicht); AKS.3 (Spiel mit Setter); **AKE.2 (Ent-14e)**:
   Maximum kurz auf 1, „Minimum übernehmen" ⇒ `ldr: minimum abgewiesen`, zurück auf 4095; Minimum
   kurz auf 4094, „Maximum übernehmen" ⇒ `ldr: maximum abgewiesen`, zurück auf 0. Nur bei
   ausgeschalteter Auto-Helligkeit und einem Rohwert zwischen 2 und 4093.
4. **Im Aus-Fenster — nur nach Bestätigung durch den Nutzer:** zweiter eigener STM-Reset — AKD.2
   Teil 2, AKW.4 erneut. Ohne Bestätigung bleibt AKD.2 Teil 2 offen und wird so berichtet.
5. Danach darf der E-Release-Build folgen.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Schritt | Stand |
|---|---|---|---|---|
| `src/main.c` | A62, A69 samt M1/M2, A68 (S1); A53, ggf. Einsparungen (S3) | `stm-developer` | S1, S3 | S1 ☑ |
| `src/night/night.c`, `night.h` | A69 | `stm-developer` | S1 | ☑ |
| `src/timeserver/timeserver.c` | A62 Zeitzone beim Laden | `stm-developer` | S1 | ☑ |
| `src/vars/vars.c`, `vars.h` | A68 | `stm-developer` | S1 | ☑ |
| `src/stubs/stubs.c` | N1 malloc-Stubs, `(void)` | `stm-developer` | S1 | ☑ |
| `CMakeLists.txt`, `Makefile`, `cmake/matrix-report.sh` | Matrix-Ziel | `release-engineer` | S1 | ☑ |
| `src/i2c/i2c.c` | A60: Fristen, `i2c_wait_not_busy()`, Logzeile, Freitakten | `stm-developer` | S2 | Code ☑ |
| Dateien der Einsparungen (aus S3.1) | verhaltensneutrale Einsparungen | `stm-developer` | S3 | — |
| `src/display/display.c` | A23 | `stm-developer` | S3 | — |
| `src/tetris/tetris.c`, `src/tetris/snake.c` | A53 | `stm-developer` | S3 | — |
| `src/ldr/ldr.c` | A57 (C) | `stm-developer` | S3 | — |
| `src/esp8266/esp8266.c` | A67 | `stm-developer` | S3 | — |
| `ESP8266/ESP-uclock/vars.cpp`, `ntp.cpp`, `ESP-uclock.ino`, `http.cpp` | A35, NTP, `ovr=`, A67 | `esp-developer` | E | — |
| `ESP8266/ESP-uclock/data/app/app.js` | C35 | `pwa-developer` | P | — |
| `tools/**` | Auswerteregel, Prüfstände, `watch-log.sh` (L400), statische Prüfung | Lead | 0, alle | teilweise |
| `CHANGELOG.md` | je Release | Lead | S1, S2, S3, E, P | S1 ☑ (in `a6065d5`) |
| `BEFUNDE.md`, `TESTPLAN-PWA.md`, `knowledge/architecture-checklist.md`, `README-CMAKE.md` | | `doc-writer` | 0, T, Z | Z.5 in Arbeit |
| `CLAUDE.md` | Hardware-Abschnitt I²C | Lead | Z | — |
| Versionsdateien | STM (S1, S2, S3), ESP (E), `APP_VERSION` + `CACHE_NAME` (P) | `release-engineer` | je Release | S1 ☑ |

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015).

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**PWA parallel zu Legacy?** Ja. **Wetter-Endpunkte?** Unberührt. **Nur `.gz`?** Nicht berührt.

**Restore-Bedingung um `pending_ticker_restore`?** **Nicht geändert.** M2 hängt sich in den
Innenzweig; nachgewiesen über `git diff` und Nachprüfung S1.13.

**Richtige Schicht?** A62 am STM-Eingang und an der ESP-Quelle; M1 dort, wo die RTC gelesen wird;
A57 dort, wo der Messwert übernommen wird; N1 in den Stubs; A60 im Treiber — das Freitakten dort,
wo der Fristablauf erkannt wird; der Matrix-Lauf im Build-System.

### Scalable systems

**STM-Kommandos?** A23: eines je Wechsel auf Modus 0. A68: eines je Timer-Aus. A53: ein
Vollabgleich nach einem Spiel mit verworfenen Zeilen. A62: eine NTP-Anfrage mehr je Start.

**Byte je Minute auf der UART?** Neue Zeilen nur bei Ereignissen. **Benannt:** Bei dauerhaft totem
I²C-Bus erzeugt **jeder** Zugriff das Zeilenpaar `i2c: frist abgelaufen` / `error: cannot reset
I2C bus` — ungedrosselt. Im Normalbetrieb tritt das nicht auf (AKI.3 prüft 60 min ohne die Zeile).

**Unter 20 s Watchdog?** Ja. Freitakten schlimmstenfalls 25 ms je Zugriff, vor `watchdog_init()`
rund 0,55 s; A69 beim Start 64 Vergleiche; M2 merkt nur vor.

**PWA-Polling?** C35 senkt `update_status` im Hauptmodul von 4 auf rund 0,2 je Minute.

**Wie lange blockiert der Hauptloop?** A60 verkürzt EEPROM-Schreibaufrufe (vorher gemessen 92 ms
für 32 Byte, erwartet 34–40 ms). Der Matrix-Lauf belegt die Build-Verzeichnisse (R1) — jetzt vor
jedem der drei STM-Releases, je rund 1 Minute.

**Hartkodierte Grenzen?** Gültigkeitsgrenze Unix 1'767'225'600; Gültigkeit bis 2106-02-07, Kette
bis 31.12.2099. M1: Jahr 2026. M2: 60 s. Bestätigung 600 s / 5 s / 15 s / 10 s / drei. A60 5 ms,
`I2C_ERROR_BUSY` −8, 9 Takte beim Freitakten. **Flash-Gate 1'024 Byte** für Teilbuild und alle
72 Varianten — engste Variante nach S2 **284 Byte** darüber (Vorabwert).

### Secure by design

**`innerHTML`?** Keine neue Ausgabe. **Fremddaten?** Die NTP-Antwort — Länge, Plausibilität,
Form, Totbereich, Bestätigung. **Credentials in Logausgaben?** Keine. **Neue schreibende
Endpunkte?** Keine.

### Stable & reliable

**Fehler ausgewertet?** A62 verwirft und meldet; A57 weist ab und meldet; A60 meldet Fristablauf
(`i2c: frist abgelaufen`, `I2C_ERROR_BUSY`) und taktet den Bus frei; der Matrix-Lauf endet mit
Exit ≠ 0 und hält den Release-Build an.

**Leere `catch`?** Keine neuen.

**Stille Verwerfungen?** A53 und A35 machen sie sichtbar. **Benannte Lücken:** PWA meldet bei A57
(C) „gespeichert" (§5a); die Restpunkte aus §3.4; `i2c: frist abgelaufen` nicht in
`/api/stm32_log` (nur Mitschnitt).

**Zustand nach Abbruch?** A60: Nach einem Fristablauf wird der Bus freigetaktet, statt im
BUSY-Zustand zu bleiben — Folgezugriffe haben wieder eine Chance; vor `watchdog_init()` hängt die
Uhr nicht mehr. M2: offen bleibt „Restore kommt nie" (§3.4).

**Flags aufgelöst?** A62: `cand_ok`, `schnell`. M2: `night_off_due` beim Anwenden, bei Fristablauf
und bei jedem Schalten. A60: `i2c_last_clockspeed = 0` erzwingt die Neuinitialisierung genau
einmal je Freitakten.

---

## Verworfene Alternativen

**Ent-17, verworfene Zuschnitte:** S3 vor S2; alles in S1 mit Sparstufen; Einsparungen und A60 in
einem Release.

**M2: beim Start sofort ausschalten.** Unterdrückte den IP-Lauftext. **M2: Restore-Bedingung
erweitern.** Architektur-Invariante.

**A60: freilaufende Differenz über `delay_counter`** (ursprünglicher Entwurf). Am Code widerlegt —
`delay_counter` zählt abwärts und bleibt bei 0 stehen. **A60: Freitakten nur im BUSY-Pfad.**
Folgezugriffe scheiterten an der SB-Frist; vom Lead erweitert (Ent-15b). **A60 mit
RM0008-Verfahren oder ACK-Polling.** Nicht belegt bzw. Folgearbeit. **Logzeile auch in
`/api/stm32_log`.** Nicht gewählt.

**Matrix in `release-zip`; F411 mit Original-Startup; Planen ab dem Teilbuild; Matrix nur vor S1;
Testbau-Lauf als Gate-Nachweis.** Siehe §1. Die beiden letzten hat der Lead im Rahmen von Ent-18
ersetzt: Matrix vor jedem STM-Release, das Flash kostet, nach der Versionsanhebung; Testbau-Läufe
sind nur Vorabwerte.

**A62 ohne Ära; mit 64-Bit-Arithmetik; feste Zeitgrenze; Protokollwechsel; Ära-Regel im ESP.**
Siehe §2.

**A57 (A); A57 im Zahlen-Setter; eigene PWA-Antwort.** Siehe §5a.

**A69: Displayleistung im EEPROM.** **A23 (b).** **A53 V1/V3.** **A35 (c).** **C35 (B)/(C).**
**A21: `setRxBufferSize` sofort.**

---

## Versionsfolgen

Je Einspielschritt **nur die geänderte Komponente** (DIR-004). Testbauten und Matrix-Läufe heben
keine Version an. Erwartete Nummern nach dem Stand vom 11.10.2026 (Momentaufnahme).

| Schritt | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` | Tag |
|---|---|---|---|---|---|
| S1 (A62 mit Ära, A69 samt M1/M2, A68, N1) | ☑ 3.2.29 | — | — | — | `release/3.2.29-3.2.31-1.4.95` ☑ (`a6065d5`, gepusht) |
| S2 (A60 samt Freitakten) | ☐ 3.2.30 | — | — | — | `release/3.2.30-3.2.31-1.4.95` |
| S3 (Einsparungen, A23, A53, A57, A67-STM) | ☐ 3.2.31 | — | — | — | `release/3.2.31-3.2.31-1.4.95` |
| E (A35, NTP, `ovr=`, A67-ESP) | — | ☐ 3.2.32 | — | — | `release/3.2.31-3.2.32-1.4.95` |
| P (C35) | — | — | ☐ 1.4.96 | ☐ angehoben | `release/3.2.31-3.2.32-1.4.96` |

Nur der `release-engineer` führt das aus (R4). Der Tag wird **je Einspielschritt** gesetzt und
gepusht (DIR-011).
