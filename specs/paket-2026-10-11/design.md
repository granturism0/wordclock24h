# Design — Paket 2026-10-11 „Nachtfenster, Netzzeit, Spiele, I²C, Brücke" (A23, A69, A53, A57, A60, A61, A62, A63, A35, A21, A34, C35)

**Erstellt:** 2026-10-11, Stand `5c34d80`. **Freigegeben** vom Nutzer am 11.10.2026 (Ent-16);
Ent-1 bis Ent-15, Ent-3b (Bestätigungsregel, §2.4) und Ent-14e (Freigabe AKE.2) entschieden, am
selben Tag eingearbeitet. **Ent-17 (Flash-Gate): „Release teilen"** — eingearbeitet in §0 und §1.
**Review S1 BESTANDEN** mit M1 und M2 (§3.2, §3.4); **N1** in S1 vorgezogen (§1.1); **Ent-18
Matrix-Lauf** (§1.6). **Offen:** der zweite STM-Reset im Aus-Fenster in G3 — vor G3 beim Nutzer
zu bestätigen (§15.2). Momentaufnahme (DIR-006).

**Entscheidungsvermerk 11.10.2026.** **Ent-1** (a), DC und IR führen die Sicherung nach (§5) ·
**Ent-2** nur Displayliste, erstes übernommenes `TIME`, übersprungenes Ereignis nachholen,
Startsequenz wie heute — präzisiert (M2): „IP erst zeigen, dann aus" (§3) · **Ent-3 Ära-Rechnung
jetzt** — gegen die Empfehlung (§2) · **Ent-3b** Bestätigungsregel wie vorgeschlagen; die Umsetzung
mit zusätzlichem Mindestabstand von 10 s liegt im Rahmen (§2.4) · **Ent-4** V2, keine
Inaktivitätsgrenze (§6) · **Ent-5** (b) (§9) · **Ent-6** (A) (§11) · **Ent-7 (C)** — gegen die
Empfehlung (§5a) · **Ent-8** Werkzeug (§12.1) · **Ent-9** A34 schliessen · **Ent-10** A63
beobachten · **Ent-11** A68 ja (§4) · **Ent-12** A67 ja (§7) · **Ent-13** NTP samt Plausibilität,
jetzt tragend (§10.1) · **Ent-14** alle Freigaben erteilt; (a) gilt dem einen Reset in G1 (§15) ·
**Ent-14e** AKE.2 ausdrücklich freigegeben (§15.2) · **Ent-15** nur Fristen (§8) · **Ent-16**
freigegeben · **Ent-17** Release teilen: S1 = A62, A69, A68 (dazu N1, M1, M2); S3 = Flash schaffen,
dann A23, A53, A57, A67-STM (§0, §1) · **Ent-18** Matrix-Lauf aller 72 Varianten vor S1, Gate für
alle 72 (§1.6).

**Schrittfolge:** **0 (ohne Firmware) → S1 (STM: A62 mit Ära, A69 samt M1/M2, A68, N1; Matrix-Lauf
vor dem Release-Build) → S2 (STM: A60) → S3 (STM: Flash-Analyse und Einsparungen, dann A23, A53,
A57, A67-STM) → E (ESP: A35, NTP, `ovr=`, A67-ESP) → P (PWA: C35) → M (Abschlusslauf A21) → T
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
`ntp.cpp`. Die Zeilennummern in `src/**` haben sich seither durch S1 verschoben.

---

## 0. Warum diese Reihenfolge

### 0.1 Die Regeln aus `make release-zip` und R3b

`make release-zip` baut STM, ESP **und** die PWA-Assets in einem Zug. Liegt im Baum eine Änderung
an einer Komponente, deren Version der Build **nicht** anhebt, entsteht ein Fabrikat mit alter
Versionsnummer und neuem Inhalt (Lehre aus Paket 2026-10-09). Für mehrere STM-Releases
hintereinander gilt dasselbe **innerhalb** der Komponente: Code für S2 oder S3 im Baum vor dem
S1-Build ginge mit S1 hinaus.

| Regel | Umsetzung in `tasks.md` |
|---|---|
| Produktcode des **nächsten Releases** entsteht erst **nach dem Release-Build** des vorigen | S2.1 hängt an S1.19, S3.2 an S2.7, E.1 an S3.16, P.1 an E.9 |
| Der **Release-Build** folgt erst **nach dem Gerätenachweis** des vorigen Releases | S2.7 an S1.21 (G1), S3.16 an S2.9 (G2), E.9 an S3.18 (G3), P.6 an E.11 (G4); M.1 an P.8 (G5); T.2 an M.1 (G6) |
| **Der Release-Build S1 folgt erst nach dem grünen Matrix-Lauf** | S1.19 an S1.16 (Ent-18) |
| Erlaubt parallel: Gerätenachweis des vorigen und Codearbeit des nächsten | Der Baum wirkt nicht aufs Gerät; gebaut wird erst danach |
| Erlaubt jederzeit: Doku, Spec, Werkzeug, **lesende Analysen** | Gehen nicht ins Fabrikat. Die Flash-Analyse S3.1 darf deshalb früh beginnen; ihre Zahlen gelten gegen die S2-Map. **Ausnahme:** 0.5 (Testplan V3/S1) muss **vor** G1 fertig sein |
| Innerhalb eines STM-Releases **nacheinander**, je Task ein Testbau | Das Flash-Gate wird je Task und je Einsparung gemessen (§1) |

### 0.2 Release-Zuschnitt (Ent-17, Ent-18)

| # | Release | Inhalt | Komponente | Code erst nach | Build erst nach | Nachweis |
|---|---|---|---|---|---|---|
| — | **Schritt 0** | A61-Werkzeug, Messung A61/A34, Befunde (L391 u. a.), Testplan V3/S1 | Werkzeug, Doku | Freigabe | — | lesend, Mitschnitte |
| 1 | **STM-Release 1** | A62 mit Ära, A69 samt M1/M2, A68, N1 | STM | Freigabe | Nachprüfung S1.13; **Matrix-Lauf S1.16 grün**; 0.5 vor G1 | **G1** |
| 2 | **STM-Release 2, isoliert** | A60 + L360 | STM | Build S1 (S1.19) | G1 (S1.21) | **G2** |
| 3 | **STM-Release 3** | Flash-Analyse, Einsparungen; dann A23, A53, A57 (C), A67-STM | STM | Build S2 (S2.7) | G2 (S2.9) | **G3** |
| 4 | **ESP-Release** | A35 (b), NTP-Puffer + Plausibilität (tragend), `ovr=`, A67-ESP | ESP | Build S3 (S3.16) | G3 (S3.18) | **G4** |
| 5 | **PWA-Release** | C35 (A) | PWA | Build E (E.9) | G4 (E.11) | **G5** |
| — | **Abschlusslauf A21** | 30 min Last, Setter-Burst | — | — | G5 (P.8) | **G6** |
| — | **`pwa-tester`** | Phasen 0–4 und 9 (DIR-012) | — | — | G6 (M.1), T.1 | AKZ.6 |
| — | **Schritt Z** | BEFUNDE, Checkliste, CLAUDE.md, README-CMAKE, Spec-Endstand | Doku | — | T.2 (Z.2/Z.3 schon nach G2) | — |

| Frage | Antwort |
|---|---|
| Warum **S1 nur A62, A69, A68** (dazu N1, M1, M2)? | Ent-17. Nach S1.3 waren noch 1'132 Byte frei; A68 passte (+44) und gehört fachlich zu A69. M1 und M2 sind Korrekturen aus dem Review **an A69**, kein neuer Inhalt. N1 hat der Lead vorgezogen, damit M2 in den Flash passt — verhaltensneutral und von zwei Reviews bestätigt |
| Warum **S2 vor S3**? | (1) **A60 bleibt isoliert messbar** — die Vorher-Messung (AKI.2) läuft in G1 auf S1, die Nachher-Messung in G2; dazwischen ändert sich nur A60. Läge S3 dazwischen, enthielte der Nachher-Stand auch die Einsparungen aus S3.2, die Logzeilen und Texte ändern können — die Vergleichbarkeit der `eep`-Zeilen wäre nicht mehr gesichert. (2) **Die Flash-Analyse soll gegen die endgültige Map arbeiten.** (3) A60 ist klein und fertig spezifiziert; S3 braucht zuerst die Analyse |
| **Risiko dieser Reihenfolge** | Nach N1 deutlich kleiner: Nach S1 bleiben **584 Byte** über dem Gate; A60 ist auf 0…+60 geschätzt, nach §1.2 bis rund 180. Fällt der Testbau nach A60 (S2.2) trotzdem unter 1'024 Byte: anhalten und vorlegen |
| Warum **S3 vor E**? | Die ESP-Hälfte von A67 setzt die STM-Hälfte voraus. DIR-002: Firmware zuerst, STM vor ESP |
| Warum im S3 **zuerst Platz, dann die vier Punkte**? | Ent-17. Die vier Punkte sind auf 190–294 Byte geschätzt, nach dem gemessenen Faktor **rund 400–900 Byte**; der Rest nach S2 liegt bei rund 1'430–1'610 (§1.3), also bei 400–590 über dem Gate. Ob das reicht, entscheidet die Messung; die Analyse muss die Lücke schliessen |
| Reihenfolge der **vier Punkte** in S3 | A23, A53, A57, A67-STM — A67-STM zuletzt, weil seine ESP-Hälfte an ihm hängt und ein Halt am Gate dort am wenigsten Folgearbeit verwirft |
| Warum alle S3-Punkte in **einem** Release? | Getrennte Nachweise: `dspcolor[0]` (A23), `spiel: … verworfen` (A53), `ldr: … abgewiesen` (A57), Prüfstand a67 (A67). Keine ändert I²C-Zeiten |
| Warum **E vor P**, **M nach P**, **`pwa-tester` nach M**? | Firmware vor PWA; der Abschlusslauf misst den **neuen** Normalbetrieb nach C35 (§13); der `pwa-tester` erzeugt selbst Setterlast und würde M verfälschen |

---

## 1. Flash F103

### 1.1 Gemessen statt geschätzt (11.10.2026, Testbauten des Leads, F103 WC12h SK6812-RGBW)

| Stand | geschätzt | **gemessen** | frei |
|---|---|---|---|
| Ausgang (3.2.28) | — | — | **1'908** |
| nach S1.1 — A62 samt Bestätigungsregel (Mindestabstand 10 s, `cmd: set time to` unbedingt) | −110…−20 | **+208** | **1'700** |
| nach S1.3 — A69 samt Sprüngen | +160…+310 | **+568** | **1'132** |
| nach S1.5 — A68 | +10…+30 | **+44** | **1'088** |
| nach S1.9 — **N1** malloc-Stubs | — | **−668** | **1'756** |
| nach S1.11 — **M1 + M2** | 80–100 | **+148** | **1'608** |

Keine 64-Bit-Routinen in der Map, `strtoul` verschwunden (AKN.4 erfüllt). **S1 hält das Gate mit
584 Byte.**

**N1 — malloc-Stubs.** Starke Definitionen von `_malloc_r`, `_realloc_r` und `_free_r` in
`src/stubs/stubs.c`, mit `__attribute__((used, externally_visible))`; ohne das Attribut verwirft
LTO `_realloc_r`, und der Link scheitert (belegt). Danach fehlen in der Map die Module `mallocr`,
`freer`, `reallocr`, `msizer`, `sbrkr` und `mlock` — **668 Byte**. Verhaltensneutral, beide
Reviews BESTANDEN. Vom Lead aus der Kandidatenliste von S3.1 in S1 vorgezogen, weil M2 sonst nicht
in den Flash gepasst hätte; S3.2 enthält N1 nicht mehr.

### 1.2 Lehre — gilt für den Rest des Pakets

**Die Flash-Schätzungen lagen um den Faktor 1,5 bis 3 daneben** (A62 sogar im Vorzeichen; M1+M2
rund 1,5- bis 1,9-fach; im Vorpaket bei S1 um den Faktor 6 bis 10). Daraus:

1. **Geplant wird mit der Messung je Task, nicht mit Summen von Schätzungen.** Eine Summe
   addiert die Fehler.
2. Eine Schätzung dient nur noch als **Untergrenze** und zur Reihenfolge; für die Planung zählt sie
   **mit Faktor 3**.
3. Was vor einem Task gilt, ist der **gemessene Rest des letzten Testbaus** — nicht eine
   Fortschreibung.
4. Eine **Einsparung** ist ebenso zu messen wie ein Zuwachs — N1 zeigt, dass ein einzelnes Symbol
   ganze Bibliotheksmodule nach sich ziehen kann; das sieht man nur in der Map.
5. Gemessen wird an **einer** Variante (F103 WC12h SK6812-RGBW). Ob sie die engste ist, zeigt erst
   der Matrix-Lauf (§1.6).

### 1.3 Gate und Planung je Release

**Gate: 1'024 Byte frei, nach jedem STM-Task und nach jeder Einsparung** (Teilbuild), dazu **für
alle 72 Varianten** in jedem Matrix-Lauf (§1.6). Darunter: **anhalten und vorlegen** — keine
Sparstufe ohne neue Rückfrage (Ent-17).

| Release | gemessener Rest davor | noch offen | Planung nach §1.2 |
|---|---|---|---|
| **S1** | — | Matrix-Lauf S1.16 | **im Teilbuild abgeschlossen:** 1'608 frei (S1.12), 584 über dem Gate. Offen ist, ob **alle 72 Varianten** das Gate halten |
| **S2** | **1'608** (S1.12) | A60 (Schätzung 0…+60) | mit Faktor 3 bis rund 180 Byte gegen einen Spielraum von **584** — planbar |
| **S3** | Messung nach S2.2 (erwartet rund 1'430–1'610) | A23, A53, A57, A67-STM (Schätzung zusammen 190–294) | mit Faktor 2 bis 3 **rund 400–900 Byte** gegen einen Spielraum von rund 400–590 — die Flash-Analyse (§1.5) muss die Lücke schliessen; jeder Punkt einzeln mit Gate |

### 1.4 Sparstufen — **nicht gewählt** (Ent-17)

Diese Stufen standen in der Spec; der Nutzer hat **keine** davon gewählt. Sie greifen **nur nach
einer neuen Rückfrage**:

1. Ent-2 (3) zurücknehmen (keine Sprungauswertung).
2. A67-STM zurückstellen (dann entfällt auch die ESP-Hälfte).
3. A53 V2 → V2b.
4. Logzeilentexte kürzen.

Ebenfalls nicht ohne Rückfrage: `atoi`-Ersatz (Verhaltensänderung). **Nicht als Sparstufe** gelten
die Bestätigungsregel (§2.4, tragend nach Ent-3), A57 (C), M2 und die geschützte Logzeile
`set_display_power: new=…`.

### 1.5 Flash-Analyse vor S3 (S3.1, `firmware-analyst`)

**Gegenstand:** die Map des **S2-Release-Builds** (vom Lead genannt). Gelesen werden die
**gemessenen** Grössen — Symbolgrössen und Abschnitte (`.text.*`, `.rodata.*`) —, keine
Schätzungen.

**Je Kandidat:** Symbol bzw. Text · Abschnitt · Bytes laut Map · Verhaltensänderung ja/nein ·
**Abhängigkeiten**: sucht ein Werkzeug (`watch-log.sh`, Auswerteregel 0.1, Smoketest,
`measure-log.sh`), der Testplan, ein AK dieser Spec oder `BEFUNDE.md` nach der Zeile?

**Mindestens zu prüfen:**

| Kandidat | Hinweis |
|---|---|
| Toter Zweig `FILE ` in `esp8266_get_message()` (A52) | Rund 25 Byte im Vorpaket genannt; an der Map messen |
| Doppelte und lange Texte | gleiche Zeichenketten an mehreren Stellen; lange Debugtexte |
| Die **zweite** `log_printf` in `set_display_power()` (am Ende der Funktion) | Kandidat. **Die erste, `set_display_power: new=…`, ist geschützt** (Nachweis in AKW.4) und **kein** Kandidat |
| Debugtexte im normalen Bau | **Erst an der Map belegen, ob sie im Fabrikat landen** — im Vorpaket stellte sich heraus, dass `debug_log_*` im Fabrikat schon weggeworfen wird und C42 nichts sparte |
| Weitere Bibliotheksmodule nach dem Muster von N1 | Zieht ein einzelnes Symbol ein ganzes Modul herein? (N1 selbst ist erledigt) |

**Ergebnis:** Liste mit Summe, gemessen gegen den Bedarf aus §1.3; Vorschlag der Reihenfolge
(grösster verhaltensneutraler Gewinn zuerst). **Umsetzung (S3.2):** der `stm-developer`, **eine
Einsparung je Schritt**, nach jeder ein Testbau (S3.3) mit gemessenem Gewinn neben der Map-Grösse.
Verhaltensneutral heisst: kein Zweig, der heute erreichbar ist, ändert seine Wirkung; keine
Logzeile, nach der etwas sucht, verschwindet oder ändert ihren Wortlaut — sonst Rückfrage.

**Erst danach** folgen A23, A53, A57, A67-STM, **einer nach dem anderen mit Gate**.

### 1.6 Matrix-Lauf aller 72 STM-Varianten (Ent-18)

**Anlass:** Das Gate ist bisher nur an **einer** Variante gemessen. Das Original-ZIP enthält
**72 HEX-Dateien**; ob eine andere Variante enger liegt (wc24h, ein anderer LED-Typ auf dem F103),
ist unbekannt.

**Umfang:** {wc12h, wc24h} × {F103, F401CC-25, F401CC-8, F411CE-25, F411CE-8, Nucleo401,
Nucleo411} × fünf LED-Typen = 70, dazu je eine F407-TFT-Variante = **72**.

**Make-Ziel (S1.15, `release-engineer`):**

- **eigenes Ziel, nur auf Anforderung**; `make` und `make release-zip` bleiben **unverändert**;
- baut jede Variante in ein eigenes Verzeichnis (kein gemeinsamer CMake-Cache, R1);
- gibt je Variante den freien Flash aus (Tabelle: Variante, frei) und den **kleinsten Rest mit
  Variante**;
- **Exit ≠ 0**, wenn eine Variante nicht baut oder unter **1'024 Byte** frei liegt — die Meldung
  nennt die Variante (DIR-014; Gegenprobe mit künstlich hoher Grenze);
- **BlackPill-F411-Varianten** mit `startup_stm32f411xe.S` wie der Teilbuild; das Original nimmt
  `startup_stm32f401xc.S`. Die Abweichung steht im Makefile-Kommentar (`release-engineer`) und in
  `README-CMAKE.md` (`doc-writer`, Z.5).

**Wann:** **erster Lauf vor STM-Release 1** (S1.16), und der Release-Build S1 hängt am grünen
Lauf. Danach **nur auf Anforderung**; läuft er, gilt das Gate für alle 72. Die Teilbuild-Messung je
Task bleibt auf F103 WC12h SK6812-RGBW.

**Nebenwirkung, benannt:** 72 Bauten dauern; der Lauf belegt die Build-Verzeichnisse — während er
läuft, baut nichts anderes (R1).

---

## 2. S1 — A62: Netzzeit mit Ära-Rechnung (Ent-3)

### 2.1 Befund am Code

| Stelle | Befund | Folge für die Ära |
|---|---|---|
| ESP `ntp.cpp:78-83` | Setzt die Sekunde aus Byte 40–43 als 32-Bit-Zahl zusammen und sendet `TIME ` + `Serial.println(unsigned long)` — **dezimal, ohne führende Nullen** | Ab 07.02.2036 06:28:16 UTC beginnt die Zahl wieder bei 0 und wird **kürzer**: Ära-1-Werte haben 1 bis 10 Ziffern (10 Ziffern erst ab 2067). Die Vorgabe „genau 10 Ziffern" hätte 2036–2067 jede Zeit verworfen — der Parser muss **1–10** annehmen |
| ESP sonst | Keine eigene Zeitrechnung: kein `gmtime`, kein `time_t`, kein Zeitstempel in `weather.cpp`; die Uhrzeit für Anzeige und API kommt vom STM (`var_send_tm`) | Der ESP braucht **keine** Pivot-Regel. Er reicht die 32-Bit-Zahl durch |
| STM `timeserver.c:253` | `curtime = (time_t) (seconds_since_1900 - 2208988800U)` — beide Operanden `uint32`, die Differenz **läuft modulo 2³² um** | Für Ära-1-Werte n < 2'208'988'800 ergibt die Differenz n + 2³² − 2'208'988'800 — **genau die Unixzeit der Ära 1**. Die Arithmetik bildet die Ära **schon heute** richtig ab |
| STM `base.c:461` (S0) | `my_gmtime()` rechnet intern mit `uint32_t` | Gültig bis 2106-02-07 06:28:15 UTC — ohne 64-Bit-Arithmetik; **S8b bleibt grün** |
| STM `rtc.c:204`, `:241` | Die RTC speichert das Jahr als zwei BCD-Ziffern (`tm_year − 100`), das Jahrhundertbit wird nicht genutzt | **Neue Grenze der Kette: 31.12.2099.** Danach schreibt `INT_TO_BCD` ein ungültiges Jahr. Nicht Teil dieses Pakets, in L371 benannt |
| STM `timeserver.c:254` | Zeitzone wird auf `time_t` (64 Bit) addiert, `my_gmtime()` kürzt auf 32 Bit | Wirkt erst an der 2106-Grenze, also nach der RTC-Grenze — ohne Belang |

**Folgerung:** Das 2036-Problem war **nicht** die Arithmetik, sondern die Gültigkeitsfrage. Die
Ära-Rechnung ist ohne Protokollwechsel und ohne 64-Bit-Code machbar.

### 2.2 Die Regel: eine Grenze auf der Unixzeit

```
ESP8266_TIME:
    n = parse (esp8266.u.time)          // 1..10 Ziffern, dann '\0'; Ueberlauf (> 4'294'967'295) => ungueltig
    u = (uint32_t) (n - 2208988800U)    // Aera 0 und 1 in einem Schritt (Umlauf modulo 2^32)
    gueltig  <=>  u >= 1'767'225'600    // Unix 2026-01-01 00:00:00 UTC
```

Das ist der Pivot aus dem Vorschlag des Nutzers, nur **in einer** Grenze ausgedrückt:

| NTP-Sekunde n | Ära | `u` | Ergebnis |
|---|---|---|---|
| 3'976'214'400 … 4'294'967'295 | 0 | 1'767'225'600 … 2'085'978'495 | gültig, 2026-01-01 … 2036-02-07 06:28:15 |
| 0 … 2'208'988'799 | 1 | 2'085'978'496 … 4'294'967'295 | gültig, 2036-02-07 06:28:16 … 2106-02-07 06:28:15 |
| 2'208'988'800 … 3'976'214'399 | **Totbereich** | 0 … 1'767'225'599 | **verworfen** (1970–2025 in Ära 0 oder nach 2106 in Ära 1) |

Logzeile bei Verwerfung: **`time: ungueltig, verworfen`**. `net_time_countdown` bleibt, wie er
beim Anfordern gesetzt wurde (`main.c:4264`, 3800 s) — dasselbe wie bei einer ausbleibenden Zeit.

### 2.3 Warum eine zweite Prüfung nötig ist — und warum keine feste Zeitgrenze

Mit dem Pivot sind zwei Fehlerbilder **gültige** Zeiten:

- **Die 0** (Kiss-of-Death, unsynchronisierter Server; `ntp.cpp` prüft heute nichts) ergibt
  2036-02-07 06:28:16 — genau der Fehler aus L371, nur jetzt formal gültig.
- **Eine verstümmelte Zeile** — `TIME` trägt keine Prüfsumme. Fällt aus `4000648905` eine Ziffer
  heraus, wird `400064890` daraus: eine Ära-1-Zeit im Oktober 2048. Mit „genau 10 Ziffern" war das
  abgefangen, mit 1–10 nicht mehr.

**Eine feste Grenze am STM** („nicht vor dem Build-Datum, nicht mehr als N Jahre danach") hilft
nicht: Jedes N, das die Ära-Rechnung sinnvoll macht (eine alte Firmware soll nach 2036
weiterlaufen), enthält 2036 und die Jahre der verstümmelten Werte. Ein kleines N macht die
Ära-Rechnung für jede nicht nachgeführte Firmware wertlos.

**Darum zwei Prüfungen, an zwei Stellen:**

1. **ESP-Plausibilität (Ent-13, jetzt tragend):** LI = 3, Stratum 0, Transmit = 0 ⇒ der ESP sendet
   **kein** `TIME` (§10.1). Das fängt die 0 an der Quelle.
2. **Bestätigungsregel am STM (Ent-3b, §2.4):** fängt, was über die Brücke verstümmelt ankommt, und
   bleibt wirksam, solange ein ESP ohne E läuft.

### 2.4 Bestätigungsregel (Ent-3b — entschieden 11.10.2026)

**Entschieden wie vom `spec-writer` vorgeschlagen:** Ein Sprung gegenüber der fortgeschriebenen
letzten übernommenen Zeit gilt erst, wenn eine **zweite** Antwort ihn bestätigt.

```
Zustand (rund 16 Byte RAM):  ref, ref_up, ref_ok;  cand, cand_up, cand_ok;  schnell (0..3)

bei gueltigem n:
    erwartet = ref + (uptime - ref_up)                       // uint32, Differenz als int32 => uebersteht die Aerengrenze
    wenn ref_ok und |n - erwartet| <= 600 s:   uebernehmen
    sonst wenn cand_ok und |n - (cand + (uptime - cand_up))| <= 5 s:   uebernehmen   // bestaetigt
    sonst:
        cand = n, cand_up = uptime, cand_ok = 1
        log "time: sprung, bestaetigung angefordert"
        wenn schnell < 3:  net_time_countdown = 15, schnell++          // gleich nochmals fragen
uebernehmen:  ref = n, ref_up = uptime, ref_ok = 1, cand_ok = 0, schnell = 0, dann wie heute
```

- **Nach jedem STM-Start** gibt es keine Referenz: Die erste Antwort wird Kandidat, die zweite
  (rund 15 s später) bestätigt sie. Bis dahin läuft die Uhr aus der RTC weiter. Kosten: eine
  NTP-Anfrage mehr je Start.
- **600 s Toleranz** für den regulären Abgleich; **5 s** für die Bestätigung.
- **Die Referenz ist UTC** (die NTP-Sekunde) — die Sommerzeit spielt hier keine Rolle.
- **Höchstens drei Schnellabfragen** in Folge; danach der reguläre Takt.
- Zwei **gleiche** Fehlwerte (zweimal 0) bestätigen sich **nicht**: Erwartet ist 0 + 15.
- `uptime` (`main.c:685`, Sekunden, in der ISR gezählt) ist die Zeitbasis.
- **Umsetzung (Lead, 11.10.2026), im Rahmen von Ent-3b:** Die zweite Abfrage nach 15 s ist
  umgesetzt (`net_time_countdown = 15`, höchstens 3 Schnellabfragen). **Zusätzlich** verlangt die
  Bestätigung einen **Mindestabstand von 10 s** zwischen Kandidat und Bestätigung — das macht die
  Regel strenger: Zwei Antworten in derselben Sekunde bestätigen sich nicht. `cmd: set time to`
  wird **unbedingt** geloggt — eine Frage der Beobachtbarkeit (AKN.6), keine Entscheidung. Review
  S1.8: BESTANDEN.

**Verworfene Alternativen zu Ent-3b:**

| Variante | Folge |
|---|---|
| **(i) keine Bestätigung**, nur die ESP-Plausibilität | Eine verstümmelte `TIME`-Zeile stellt Uhr **und RTC** für bis zu eine Stunde um Jahrzehnte vor; A69 würde den „Sprung" als übersprungene Timer deuten. Zwischen S1 und E schützt dann gar nichts gegen die 0 |
| **(ii) Protokollwechsel:** der ESP sendet immer 10 Stellen mit führenden Nullen (oder eine Prüfsumme am `TIME`) | Der STM könnte „genau 10 Ziffern" behalten. Braucht den ESP **vor** dem STM (Reihenfolge kippt), und ein alter ESP würde nach 2036 von einem neuen STM abgewiesen |

**Gewählt: die Bestätigungsregel.** Ohne Protokollwechsel, unabhängig vom ESP-Stand.

### 2.5 Wechselwirkungen

- **A69 Ent-2 (2) und M1:** „erstes übernommenes `TIME`" heisst nach §2.4 „erstes
  **bestätigtes**" — und nach M1 auch dann, wenn die RTC beim Start ein Jahr vor 2026 meldet.
- **A69 Ent-2 (3):** Ein bestätigter Sprung ist ein Sprung — die Nachholregel greift.
- **Zeitzone (`timeserver.c:67`):** Betrag über 12 ⇒ nicht übernehmen (Vorgabe), eine Logzeile;
  dieselbe Grenze wie der Setter (`:196`).
- **Kein** `strtoul` mehr im Fabrikat (Map, S8b), und weiter **kein** `__udivmoddi4` oder
  `__aeabi_ldivmod` — im Testbau nach S1.1 bestätigt.

**Prüfstand a62** (§14): Vektoren über die Ärengrenze (`4294967295` → 2036-02-07 06:28:15,
`0` → 06:28:16, `1`), die Totbereichsgrenzen (`2208988799` gültig → 2106-02-07 06:28:15,
`2208988800` verworfen, `3976214399` verworfen, `3976214400` gültig → 2026-01-01), kurze Formen
(`123`), Formfehler (`""`, `" 1"`, `"+1"`, `"1x"`, 11 Ziffern, `4294967296`, `9999999999`).
**Referenz:** Python `datetime` mit `unix = n − 2208988800 + (2³² falls n < 2208988800)`, gültig
genau dann, wenn 1'767'225'600 ≤ unix < 2³² — **unabhängig** vom C-Code; dazu die Felder über
**gt**. **Bestätigung** mit simulierter `uptime`: Start + n, n + 15 ⇒ übernommen bei der zweiten;
Start + 0, 0 ⇒ nie; stündlicher Abgleich ±30 s ⇒ sofort; einzelner Fehlwert ⇒ verworfen, danach
regulär weiter; echter Sprung zweimal ⇒ übernommen; vier Fehlwerte ⇒ drei Schnellabfragen, dann
regulär; Abgleich über die Ärengrenze ⇒ sofort; zwei Antworten im Abstand unter 10 s ⇒ keine
Bestätigung. **Sabotagen:** Totbereich entfernt, „genau 10 Ziffern", Bestätigung entfernt — je
FEHL.

---

## 3. S1 — A69: Nachtfenster (Ent-2, samt M1 und M2)

### 3.1 Die Funktion

In `night.c`, neben `night_check_night_times()`:

```
night_last_event (is_ambilight, wday, m, *out_minutes_back) -> 0 = keins, 1 = aus, 2 = ein
    bester = kein
    fuer k = 0..7:                                  // k = 7: derselbe Wochentag vor einer Woche
        tag = (wday + 7 - k) % 7
        fuer jeden Timer i der Liste:
            nur aktive; nur wenn wochentag_gilt (flags, tag)
            bei k = 0 nur minutes <= m
            abstand = k*1440 + m - minutes
            kleinster abstand gewinnt
    return Art des besten (aus/ein), abstand
```

`wochentag_gilt()` ist die herausgelöste Prüfung aus `night.c:166-180`; `night_check_night_times()`
ruft sie ebenfalls. **Gleichstand** (zwei Timer derselben Minute): Referenz ist das heutige
Verhalten — der **erste passende** Timer in Tabellenreihenfolge. Restpunkt aus dem Review (§3.4).

### 3.2 Wo sie greift (nach Review S1)

| Stelle | Wirkung |
|---|---|
| **Start**, nach RTC-Lesen, vor `watchdog_init()` — **nur bei `gmain.year >= 2026`** (**M1**) | Eine RTC mit leerer Batterie gilt als gültig und meldet 2000-01-01; ohne die Jahresgrenze bestimmte sie das Fenster gegen ein falsches Datum. Bei einem Jahr vor 2026 wertet das **erste bestätigte** `TIME` aus (Ent-2 (2)) |
| **Ergebnis „aus" beim Start** (**M2**, Ent-2 (4): „IP erst zeigen, dann aus") | **Nicht sofort schalten**, sondern **vormerken** (`night_off_due`). Grund: Der IP-Lauftext kommt erst nach `IPADDRESS`, und `display_set_ticker` tut bei ausgeschaltetem Display nichts — ein sofortiges Ausschalten hätte die IP unterdrückt. **Angewandt** wird die Vormerkung **nach dem IP-Lauftext** im Innenzweig der **unveränderten** Restore-Bedingung um `pending_ticker_restore`; kommt keine IP, nach einer **Frist von 60 s**. **Jedes Schalten** des Displays vorher **verwirft** die Vormerkung. Angewandt wie beim Timer (`set_display_power (FALSE, TRUE)`), Logzeile `night: start aus (hh:mm)` |
| **Ambilight (Ent-2 (1))** | Nur über die Displayliste mitgeschaltet, wie beim Aus-Timer; die Ambilight-Liste wird beim Start **nicht** ausgewertet |
| **Vorwärtssprung > 1 min (Ent-2 (3))** | Liegt das jüngste Ereignis **im übersprungenen Intervall**, wird es angewandt; sonst nichts. Rückwärts: nichts |

**Offline senden:** `set_display_power()` sendet bei `! esp8266.is_online` nichts; der Vollabgleich
trägt `display_power` und `ambilight_power` später nach (`vars.c:1831-1832`, ✔ A). Mit M2 fällt das
Schalten ohnehin meist **nach** `IPADDRESS` — dann sendet es direkt. **G1 prüft das** (AKW.4).

**Restore-Bedingung:** M2 hängt sich in den **Innenzweig** der bestehenden Bedingung (Ticker
inaktiv, kein Icon, kein Icon-Stop-Timer offen, kein Overlay); **keine** der vier Teilbedingungen
wird geändert (CLAUDE.md, Architektur-Invariante). Nachgewiesen über `git diff` und die
Nachprüfung S1.13.

**`minutes == 1440`:** Bleibt zulässig und unwirksam; der Prüfstand hat den Fall.

### 3.3 Warum nicht einfacher

„Der letzte vergangene Timer **des Tages**" reicht nicht: Ein Aus-Timer um 23:00 am Vortag und
ein Neustart um 01:00 fänden nichts. Darum bis zu sieben Tage zurück — 64 Vergleiche.

### 3.4 Niedrige Restpunkte aus dem Review S1 — benannt, kein Fix

| Restpunkt | Wirkung |
|---|---|
| Gleichstand Aus/Ein in derselben Minute | Welcher Timer gilt, hängt an der Tabellenreihenfolge |
| Rücksprung über 3,5 Tage modulo Woche | Ein sehr grosser Rücksprung kann als Vorwärtssprung gelesen werden |
| `quick` nach drei Fehlwerten | Nach drei Schnellabfragen gilt wieder der reguläre Takt — bis zu rund 2 h bis zur ersten Netzzeit |
| `NIGHT_OFF_AFTER_IP` ohne Frist | Kommt der Restore nie, bleibt die Vormerkung offen; die 60-s-Frist gilt nur ohne IP |
| Erstes `TIME` bei ungültiger RTC | Schaltet sofort und kann den IP-Lauftext abschneiden |
| Anderer Ticker | Ein Ticker, der vor dem Restore startet, überschreibt den IP-Lauftext |

Der `doc-writer` trägt sie in Z.1 als Befunde ein.

---

## 4. S1 — A68: Ambilight-Kopie (Ent-11)

`var_send_display_ambilight_power()` (heute `static`, `vars.c:1324`) wird öffentlich und dort
gerufen, wo die Ambilight-Power sich ändert, ohne dass gesendet wird — mindestens in
`set_display_power()` bei `do_sync_ambilight`, **hinter** derselben `is_online`-Bedingung wie die
Display-Power. Weitere Stellen stellt der `stm-developer` als **Bestandsliste** fest (AKB.1).
**Keine zweite** Sendefunktion. A56 bleibt eigener Punkt. **Stand 11.10.2026:** umgesetzt (S1.5),
Testbau +44 Byte, Review BESTANDEN.

---

## 5. S3 — A23: Displayfarbe (Ent-1)

- `saved_display_colors` nach dem Muster `saved_ambilight_colors` (`display.c:3557`).
- **Gesetzt:** in `display_read_config_from_eep()` **vor** `display_init_color_animation()`
  (`:3568`); in `display_set_display_colors()` (DC) — **auch während** einer Animation; in den
  IR-Farbschritten im Modus 0 (`main.c:4911-4955`, `display.c:5603ff`).
- **Zurück:** in **jedem** Weg auf Modus 0 (Setter `:4456-4473`, Inkrement/Dekrement
  `:4480-4527`): Sicherung kopieren, `display_calc_dimmed_display_colors()`, **unbedingt
  `var_send_display_colors()`**.
- **Nicht:** kein EEPROM-Schreiben im Animationspfad; keine Änderung an der Ambilight-Sicherung.

Kosten: ein zusätzliches quittiertes Kommando je Wechsel auf Modus 0.

---

## 5a. S3 — A57: „Messwert übernehmen" weist eine Inversion ab (Ent-7 = C)

**Entwurf (`ldr.c:168-226`):**

```
ldr_set_min_value:  nach erfolgreicher Wandlung
    wenn adc_value >= ldr.ldr_max_value:   log "ldr: minimum abgewiesen (%u >= max %u)", nichts setzen, nichts schreiben
    sonst wie heute
ldr_set_max_value:  analog mit  adc_value <= ldr.ldr_min_value  =>  "ldr: maximum abgewiesen (%u <= min %u)"
```

- **Gleichheit wird abgewiesen** — dieselbe Regel, die der Start heilt (`ldr.c:128`, `max <= min`).
- **Nur diese beiden Funktionen.** Der Zahlen-Setter aus A59 (`main.c:1907-1914`) nimmt die
  verdrehte Reihenfolge **weiter** an — der Backup-Import setzt Minimum und Maximum nacheinander
  (`app.js:6536-6538`, L68); Prüfstand L59 muss unverändert bestehen.
- **Rückmeldung an ESP und PWA, ✔ SW:** Der RPC-Zweig sendet nach dem Aufruf bereits
  `var_send_ldr_min_value()` bzw. `var_send_ldr_max_value()` (`main.c:1609`, `:1617`). Bei einer
  Abweisung meldet er damit den **alten** Wert; die ESP-Kopie bleibt richtig.
- **Sichtbare Meldung in der PWA — nicht ohne neue API, ✔ SW:** `/api/ldr_min_set` antwortet
  `{"ok":true}` unmittelbar nach dem RPC (`http.cpp:10077-10086`), **bevor** der STM gemessen hat;
  die PWA zeigt daraufhin „gespeichert" (`app.js:13100`, `:13266`). **Als Hinweis**: `doc-writer`
  trägt in Z.1 einen neuen PWA/ESP-Befund ein; nicht in diesem Paket.

**A57 wird erst nach G3 geschlossen** (AKE.2), nicht in Schritt 0.

---

## 6. S3 — A53: Spiele (Ent-4 = V2)

- In beiden Spielschleifen (`tetris.c:655-680`, `snake.c:240-265`) ein Zähler für jede Zeile, die
  `esp8266_get_message()` liefert und die Schleife **nicht** als `GT`/`GS` auswertet. Zeitablauf
  ohne Zeile zählt nicht. Ob eine reine Quittung mitzählt, entscheidet der `stm-developer`
  (Empfehlung: ja — einfache Regel).
- Nach `tetris()`/`snake()` in `schedule_esp8266_games()`: bei Zähler > 0 `var_sync_pending = 1`
  und `spiel: %u Zeilen verworfen`. Der Vollabgleich läuft regulär über den Vormerkweg.
- **`snake.c:291`:** `status` vor dem ersten Lesen initialisieren.
- Keine Inaktivitätsgrenze (Ent-4).

**Was bleibt:** Der Setter während des Spiels ist **verloren** — sichtbar und danach konsistent.

---

## 7. S3 und E — A67: OTA-Ankündigung (Ent-12)

| Seite | Änderung |
|---|---|
| **STM** (S3.10) | Im Zeilenparser (`esp8266.c`) eine **neue** Zeile mit exaktem Vergleich, Vorschlag `OTA start`; setzt **nur** `esp8266.is_online = 0`. Nicht `esp8266_cap_var_crc_seen` (`:550`, Risiko R-3) |
| **ESP** (E.4) | In **allen drei** OTA-Pfaden — `http.cpp:13009`, `:7590`, `:13402` — die Zeile **vor** dem Download; bei Fehlschlag `SYNCVARS` (`esp8266.c:639`) |

Der Wortlaut wird **einmal** festgelegt (S3.10) und wörtlich übernommen (E.4). Am Gerät
nachweisbar erst beim **nächsten** ESP-OTA nach E; in Z als offener Nachweis vermerkt.

---

## 8. S2 — A60 und L360: I²C (Ent-15)

```
i2c_wait_for_flags (flag1, flag2):
    start = delay_counter                       // Tick 5 us, laeuft in der TIM2-ISR
    solange Flag(s) nicht gesetzt:
        wenn (delay_counter - start) >= I2C_TIMEOUT * 200:  return 0     // 5 ms wie heute
    return 1

BUSY-Schleifen (:413 Lesen, :472 Schreiben):    dieselbe Frist, bei Ablauf Fehler wie oben
```

- **Voraussetzung:** Keine ISR nutzt `delay_*` — der `stm-developer` **bestätigt am Code** vor der
  Umsetzung (S2.1).
- Differenz überlaufsicher; Zweige mit/ohne `flag2` zusammenlegen; **kein** POS/BTF-Verfahren.

**Wirkung (gerechnet):** `ms ≈ 16·z + (0,2…0,3)·n + 1` — Vorher/Nachher-Messung AKI.2 (G1/G2).
**Flash:** Spielraum nach S1 584 Byte (§1.3). **Doku (Z.2, Z.3):** Checkliste und `CLAUDE.md` auf
den gemessenen Stand.

---

## 9. E — A35: still verworfene Zuweisungen (Ent-5 = b)

- In `var_set_parameter()` (`vars.cpp`) jede Bedingung, an der eine Zuweisung still entfällt (Index
  ≥ MAX, Wert ausser Bereich — 28 Stellen laut Analyse —, unbekannter Buchstabe `vars.cpp:971`,
  `:1503`), an **einen** Zähler; `V…`-Zeilen zählen nicht.
- **Rückgabe unverändert** — kein NAK.
- Gedrosselt (erste sofort, danach höchstens eine je 60 s): `- var ignoriert #n code=X idx=yy`,
  **ohne Wert** (A20).
- **Weg der Meldung:** `stm32_log_append()` schreibt nur in den API-Ring (`ESP-uclock.ino:1247`);
  die Logwache liest den Mitschnitt. Die Zeile muss an **beiden** Stellen ankommen, wie die
  Heap-Zeile (`:484-500`) — Lehre aus L185.

---

## 10. E — NTP-Antwort und ESP-Empfangsüberlauf

### 10.1 NTP (Ent-13, nach Ent-3 tragend)

```
packetsize = ntp_udp.parsePacket ()
wenn packetsize:
    wenn packetsize != NTP_PACKET_SIZE:   verwerfen (Rest lesen ohne Puffer bzw. flush), log
    sonst: lesen in ntp_packet_buffer
           LI = buf[0] >> 6; stratum = buf[1]; transmit = buf[40..43]
           wenn LI == 3 oder stratum == 0 oder transmit == 0:  verwerfen, log, KEIN "TIME"
           sonst wie heute (ntp.cpp:78-83)
```

Logzeile `- ntp verworfen` mit Grund (`len=`, `li=`, `stratum=`, `zeit=0`), Weg wie §9. **Keine**
Ära- oder Pivot-Rechnung im ESP (§2.1). Neuer Befund **L391** (Schritt 0).

### 10.2 `ovr=` (für A21)

`Serial.hasOverrun()` im ESP-Hauptloop abfragen; **sättigender** Zähler; Ausgabe als `ovr=` an der
Heap-Zeile (`ESP-uclock.ino:500`), Längenkommentar (`:496`) und Puffer nachgeführt. RAM 4 Byte.
**Kein** `setRxBufferSize` in diesem Paket.

---

## 11. P — C35 (Ent-6 = A)

In `app.js` (`loadSecondaryData()`, `:3185-3187`; Selbstaktualisierung `:2130-2149`):

- **Hauptmodul:** `update_status` beim Öffnen und danach nur, wenn der letzte Abruf **mindestens
  5 Minuten** zurückliegt.
- **Update- und Wartungsmodul:** wie heute alle 15 s; beim Wechsel dorthin **sofort**.
- Eine Variable für den letzten Abruf; Fehlschläge zählen als Abruf.
- Keine Änderung an Antwortform, ESP oder Anzeige.

---

## 12. Schritt 0 — ohne Firmware

### 12.1 A61-Auswerteregel (Ent-8)

Ein Werkzeug des Leads unter `tools/` (Name nach Wahl, z. B. `tools/abgleich.sh`), das einen
Mitschnitt liest und je Vollabgleich ausgibt:

| Feld | Regel |
|---|---|
| Ende | `info: configuration sent` |
| Auslöser | die letzte `(SYNCVARS)`- bzw. `(IPADDRESS …)`-Zeile vor dem Ende **nach** dem vorigen Abgleich; fehlt beides, **CMC-Abweisung** (`esp8266.c:536-541`, `:397`); sonst **unbekannt** — nie geraten |
| Dauer | Zeitstempel Auslöser bzw. erste Sendung bis Ende |
| Δ`v` | beide Zahlen von `v=` aus der letzten `diag`-Zeile davor und der ersten danach |
| Zeitüberschreitungen | `var_send_buf: keine Quittung` im Intervall |
| Stillstand | Δ`u` der `diag`-Zeilen bzw. Lücke der `diag`-Folge |

Exit ≠ 0 bei über 5 s Stillstand oder über 10 Zeitüberschreitungen in einem Abgleich (L191).
Gegenprobe mit synthetischem Mitschnitt; Review `code-reviewer`. **Abhängigkeit für S3.1:** Die
Zeilen, nach denen dieses Werkzeug sucht, darf eine Einsparung nicht ohne Rückfrage ändern.

### 12.2 Messung A61/A34

Über den Mitschnitt des ESP-OTA vom 10.10.2026 23:06 (L380), den STM-Reset vom 11.10.2026
01:58:14 (L387) und jeden weiteren Abgleich im Langzeitmitschnitt. A61 bleibt offen, falls ein
Abgleich nahe dem 30-s-Budget liegt.

### 12.3 Befund-Nachführung (`doc-writer`)

| Zeile | Nachführung |
|---|---|
| **L391 (reserviert)** | NTP-Antwort über 48 Byte überschreibt globale Variablen (`ntp.cpp:74`, `:76`, `:18`); vom Netz auslösbar; Massnahme in Gruppe C, Verweis auf Ent-13 |
| **L331 / A57** | Schluss „im EEPROM stand also nicht 18/17" trägt nicht (`ldr.c:128-137` heilt beim Start und schreibt); Mechanismus `ldr.c:168-226`; Ent-7 (C) beschlossen, **Schliessung nach G3** |
| **L360 / A60** | **zwei** BUSY-Schleifen (`i2c.c:413`, `:472`); vor `watchdog_init()` steht die Uhr für immer |
| **L371 / A62** | Fundstelle `main.c:3274`; der ESP sendet nie leer; realistische Auslöser 0, verstümmelt, über 10 Ziffern; **Ära-Rechnung beschlossen** (Ent-3); die `uint32`-Differenz bildet Ära 1 schon richtig ab; **neue Grenze 31.12.2099** (RTC, `rtc.c:204`, `:241`) |
| **L233 / A35** | Fundstelle `ESP-uclock.ino:1468-1479`; Teil 1 samt Zuordnung gelöst; `a=9` vom 10.10. war echt; offen nur Übernahme im engen Sinn; kein hoher Befund mehr |
| **A69** | Fundstelle `night.c:155`; Zeitsprung-Lücke |
| **A61** | Fundstelle `main.c:4037-4048`; Ergebnis aus 12.2 |
| **A53** | „mit A16 zusammen" überholt; `snake.c:291` |
| **A34** | geschlossen (Ent-9), Reste überführt (A61, A19, A53) |
| **A63** | Beobachtung, offen in der ToDo-Liste (Ent-10) |
| **A21** | blinder Fleck ESP-Empfangspuffer; Schliessung erst nach G6 |

### 12.4 Testplan V3/S1 (`doc-writer`, vor G1)

`TESTPLAN-PWA.md` V3 und S1 bekommen ein **Soll im Nachtzustand**: Gilt beim Reset ein
Aus-Fenster, zeigt die Uhr ab STM-Release 1 den **IP-Lauftext und ist danach aus** (ohne IP
spätestens nach 60 s; vorher blieb sie an, L390). Die Rückstellung nach S1 im Aus-Fenster ist
„wieder aus". Nach dem Reset erscheint zusätzlich `time: sprung, bestaetigung angefordert` (§2.4)
— Soll, kein Fehler.

---

## 13. M — Abschlusslauf A21 (G6)

| Quelle | Rate |
|---|---|
| Tab 1, Hauptmodul sichtbar | Selbstaktualisierung 15 s, `update_status` höchstens alle 5 min |
| Tab 2, System | `stm32_log` alle 2,5 s |
| **Tab 3, Updatemodul** | `update_status` alle 15 s — trägt nach C35 die frühere Last |
| Wetter | je ein Abruf jetzt und Vorhersage |
| Setter-Burst (Ent-14 c) | 14 Aufrufe auf den Wetterort wie A58, danach Original zurück |

**Kriterium:** AKM.2. Ergebnis in Z: A21 geschlossen oder neu gefasst; steigt `ovr`, ein neuer
Befund.

---

## 14. Prüfstände

Gebaut vom Umsetzer im Scratchpad, abgelegt vom Lead unter `tools/checks/auszug/`.

| Name | Ort | Release | Gegenstand | Gegenprobe (DIR-014) |
|---|---|---|---|---|
| **a62** | `stm/a62/` | S1 | Parser, Ärengrenze, Totbereich, Bestätigung samt Mindestabstand, Zeitzone; Referenz Python `datetime` und gt | alter Stand nimmt `""`/`"0"` an; Sabotagen Totbereich, „genau 10 Ziffern", Bestätigung ⇒ FEHL |
| **a69** | `stm/a69/` | S1 | jüngstes Ereignis gegen Minuten-Referenz, alle 10'080 Startpunkte; Start mit Jahresgrenze (M1); Vormerkung und Anwendung nach IP bzw. 60 s (M2); Sprünge; A68-Sendeprotokoll | Sabotage `<=`→`<`, Wochentagsumlauf; alter Stand ⇒ FEHL |
| **Matrix** | Make-Ziel | S1 | 72 Varianten, Gate je Variante | künstlich hohe Grenze ⇒ Exit ≠ 0 mit Name der Variante |
| **a60** | `stm/a60/` | S2 | Fristen mit Attrappen | Sabotage „Frist nie gesetzt" hängt ⇒ FEHL |
| **a23** | `stm/a23/` | S3 | Moduswechsel, Sicherung (DC, IR), Sendung | alter Stand behält Momentanfarbe ⇒ FEHL |
| **a53** | `stm/a53/` | S3 | Spielschleife, Zähler, `var_sync_pending` | alter Stand ⇒ 0 ⇒ FEHL |
| **a57** | `stm/a57/` | S3 | `ldr_set_min/max_value()` mit ADC- und EEPROM-Attrappe | alter Stand nimmt die Inversion an ⇒ FEHL; **L59 besteht weiter** |
| **a67** | `stm/a67/` | S3 | Zeilenparser `OTA start` | alter Stand ignoriert ⇒ FEHL |
| **a35** | `esp/a35/` | E | Zählung, `V…` ausgenommen, Drosselung | alter Stand zählt nichts ⇒ FEHL |
| **ntp** | `esp/ntp/` | E | Paketlängen, Wächterbyte, LI/Stratum/Transmit 0 ⇒ kein `TIME` | alter Stand überschreibt das Wächterbyte und sendet `TIME 0` ⇒ FEHL |
| **c35** | `pwa/c35/` | P | Abrufzahl je Modul | alter Stand ≈ 120 / 30 min ⇒ FEHL |
| **statisch AKK.2** | Lead | E | drei OTA-Pfade | Ausgangsstand ⇒ schlägt an |
| **abgleich** | bei 12.1 | 0 | Auswerteregel A61 | falscher Auslöser ⇒ FEHL |
| **gt, EP, W2, L59** | bestehen | alle | müssen **weiter bestehen** — auch nach N1 und jeder Einsparung in S3.2 | — |

**Zähl nach** (DIR-014): Fallzahl je Bericht, vom Lead gegen die Sollzahl verglichen.

---

## 15. Gerätenachweise

**Für alle:** `./tools/watch-log.sh` läuft **vor** jedem Einspielen und während jedes Laufs
(DIR-013); Smoketest nach jedem Einspielen (DIR-009); schreibend nur im Rahmen von Ent-14 und
Ent-14e, mit Rückstellung. **Nie** `GET /?a`, keiner der Endpunkte aus R5. Eingespielt wird vom
**Lead**.

| Nachweis | Nach | Art | Inhalt |
|---|---|---|---|
| **G1** | S1-Flash | schreibend (S25/S26, 32-Byte-Wert) + Reset im Fenster (Ent-14 a) + lesend | §15.1 |
| **G2** | S2-Flash | schreibend (S25/S26, 32-Byte-Wert, zurück) + 60 min lesend | AKI.2 nachher, AKI.3 |
| **G3** | S3-Flash | schreibend; zweiter Reset im Fenster **nur nach Bestätigung durch den Nutzer** | §15.2 |
| **G4** | ESP-OTA | lesend, 60 min | AKQ.2/AKU.3/AKO.2; Δ`v` beim OTA (Vorher +2); Smoketest samt Update-Quelle; `install-app.sh --check` |
| **G5** | PWA-Hochladen | lesend | AKC.2; `check-pwa.sh` |
| **G6** | — | Abschlusslauf | AKM.1, AKM.2 |

### 15.1 Ablauf G1 (S1: A62, A69 samt M1/M2, A68, N1)

1. **Vor dem Flash** (lesend): `numvar` 17/18, Display-/Ambilight-Power festhalten; Wache läuft.
2. **S1-Flash am Tag** (Lead); Smoketest. Im Mitschnitt nach dem Start: `time: sprung,
   bestaetigung angefordert`, danach `cmd: set time to …` (AKN.6).
3. **Tagtests:** AKN.6 (stündliche Netzzeit ohne Bestätigung, `read rtc` zur Wanduhr);
   **AKI.2 vorher** (S25/S26 über S27 zurück, 32-Byte-Wert zurück).
4. **Im Aus-Fenster** (nach dem Aus-Timer des Nutzers, 01:00): **ein eigener STM-Reset**
   (Ent-14 a — **dieser** eine Reset ist freigegeben) — AKW.4 (**IP-Lauftext, danach aus**), AKN.7.
5. **Am Morgen:** AKW.5. Danach ist G1 abgeschlossen; der S2-Release-Build darf folgen.

### 15.2 Ablauf G3 (S3: A23, A53, A57, A67-STM)

1. **Vor dem Flash** (lesend): `dspcolor[0]`, `numvar` 17/18, LDR-Rohwert, `numvar[8]`
   (Auto-Helligkeit) festhalten; Wache läuft.
2. **S3-Flash am Tag** (Lead); Smoketest.
3. **Tagtests:**
   - AKD.2 Teil 1: Regenbogen und Tageslicht, je ein/aus;
   - AKS.3: Spiel mit Setter;
   - **AKE.2 (Ent-14e, ausdrücklich freigegeben):** Maximum per Zahlen-Setter kurz auf 1,
     „Minimum übernehmen" ⇒ `ldr: minimum abgewiesen`, `numvar[17]` unverändert; Maximum zurück
     auf 4095. Minimum per Zahlen-Setter kurz auf 4094, „Maximum übernehmen" ⇒
     `ldr: maximum abgewiesen`; Minimum zurück auf 0. Nur bei ausgeschalteter Auto-Helligkeit und
     einem Rohwert zwischen 2 und 4093.
4. **Im Aus-Fenster — nur nach Bestätigung durch den Nutzer:** ein **zweiter** eigener STM-Reset —
   AKD.2 Teil 2 (`dspcolor[0]` nach dem Abgleich gleich der Nutzerfarbe), AKW.4 erneut. **Nicht
   durch Ent-14 (a) gedeckt** — die Freigabe galt dem einen Reset in G1; der Lead fragt vor G3
   nach. Ohne Bestätigung bleibt AKD.2 Teil 2 offen und wird so berichtet.
5. Danach ist G3 abgeschlossen; der E-Release-Build darf folgen.

**Die EEPROM-Frage aus A23 nicht über einen Flash beantworten**, sondern über einen eigenen Reset
nach dem Animationstest — sonst fiele eine Änderung des Fabrikats in dieselbe Beobachtung.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Schritt |
|---|---|---|---|
| `src/main.c` | A62 Zeitzweig samt Bestätigung; A69 Start, M1, M2 (Vormerkung im Innenzweig der Restore-Bedingung), Sprung; A68 `set_display_power()` (S1); A53 `schedule_esp8266_games()`, ggf. Einsparungen (S3) | `stm-developer` | S1, S3 |
| `src/night/night.c`, `night.h` | A69 | `stm-developer` | S1 |
| `src/timeserver/timeserver.c` | A62 Zeitzone beim Laden `:67`; Umrechnung `:253` bleibt | `stm-developer` | S1 |
| `src/vars/vars.c`, `vars.h` | A68 | `stm-developer` | S1 |
| `src/stubs/stubs.c` | N1 malloc-Stubs | `stm-developer` | S1 |
| `Makefile` | Matrix-Ziel für 72 Varianten | `release-engineer` | S1 |
| `src/i2c/i2c.c` | A60 | `stm-developer` | S2 |
| Dateien der Einsparungen (aus S3.1, z. B. `src/esp8266/esp8266.c` für A52) | verhaltensneutrale Einsparungen | `stm-developer` | S3 |
| `src/display/display.c` | A23 | `stm-developer` | S3 |
| `src/tetris/tetris.c`, `src/tetris/snake.c` | A53 | `stm-developer` | S3 |
| `src/ldr/ldr.c` | A57 (C) | `stm-developer` | S3 |
| `src/esp8266/esp8266.c` | A67 | `stm-developer` | S3 |
| `ESP8266/ESP-uclock/vars.cpp` | A35 | `esp-developer` | E |
| `ESP8266/ESP-uclock/ntp.cpp` | NTP-Puffer, Plausibilität | `esp-developer` | E |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | `ovr=`, Meldeweg | `esp-developer` | E |
| `ESP8266/ESP-uclock/http.cpp` | A67 | `esp-developer` | E |
| `ESP8266/ESP-uclock/data/app/app.js` | C35 | `pwa-developer` | P |
| Map des S2-Builds (lesend) | Flash-Analyse | `firmware-analyst` | S3 |
| `tools/**` | Auswerteregel, Prüfstände, `watch-log.sh`, statische Prüfung | Lead | 0, alle |
| `CHANGELOG.md` | je Release | Lead | S1, S2, S3, E, P |
| `BEFUNDE.md`, `TESTPLAN-PWA.md`, `knowledge/architecture-checklist.md`, `README-CMAKE.md` | | `doc-writer` | 0, T, Z |
| `CLAUDE.md` | Hardware-Abschnitt I²C | Lead | Z |
| Versionsdateien | STM (S1, S2, S3), ESP (E), `APP_VERSION` + `CACHE_NAME` (P) | `release-engineer` | je Release |

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015). `ESP-uclock.ino` hat
**gemischte** Zeilenenden; `http.cpp` ist UTF-8; die `src/**`-Dateien sind ASCII oder ISO-8859-1 —
**nachsehen, nicht annehmen**.

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**PWA parallel zu Legacy?** Ja. C35 ändert nur, wie oft die PWA fragt. A67 ergänzt den
Legacy-OTA-Pfad um dieselbe Ankündigung wie die API.

**Wetter-Endpunkte?** Unberührt.

**Restore-Bedingung um `pending_ticker_restore`?** **Nicht vereinfacht und nicht geändert.** M2
hängt die Anwendung der Nacht-Vormerkung in den **Innenzweig** der bestehenden Bedingung — sie
greift also erst, wenn alle vier Teilbedingungen erfüllt sind (Ticker inaktiv, kein Icon, kein
Icon-Stop-Timer, kein Overlay). Nachgewiesen über `git diff` und die Nachprüfung S1.13. A23, A53,
A57 und die Einsparungen aus S3.2 berühren sie nicht.

**Nur `.gz`?** Nicht berührt.

**Richtige Schicht?** A62: Der STM prüft seinen Eingang (Form, Totbereich, Bestätigung), der ESP
seine Quelle (Plausibilität). M1 dort, wo die RTC gelesen wird. A57 dort, wo der Messwert
übernommen wird, nicht im Zahlen-Setter (Import). N1 in den Stubs, wo die Bibliothek ersetzt wird.
Die übrigen wie in den Abschnitten.

### Scalable systems

**STM-Kommandos?** A23: eines je Wechsel auf Modus 0. A68: eines je Timer-Aus. A53: ein
Vollabgleich nach einem Spiel mit verworfenen Zeilen. A57: keins neu. A62: eine NTP-Anfrage mehr je
Start, höchstens drei Schnellabfragen bei Fehlwerten. M2: das Ausschalten nach dem IP-Lauftext
sendet direkt (ESP online), statt über den Abgleich.

**Byte je Minute auf der UART?** Neue Zeilen nur bei Ereignissen; `cmd: set time to` bei jeder
Übernahme (rund einmal je Stunde); `ovr=` verlängert die Heap-Zeile einmal je Minute um rund
6 Zeichen. Einsparungen aus S3.2 können UART-Last nur senken.

**Unter 20 s Watchdog?** Ja. A69 beim Start 64 Vergleiche; M2 wartet nicht, es merkt vor (Frist
60 s über den Hauptloop, keine Warteschleife); A60 begrenzt die zwei BUSY-Schleifen erstmals; A62
rechnet konstant.

**PWA-Polling?** C35 senkt `update_status` im Hauptmodul von 4 auf rund 0,2 je Minute.

**Wie lange blockiert der Hauptloop?** A60 verkürzt EEPROM-Schreibaufrufe (gerechnet 110 → 35–45 ms
für 32 Byte). A57: eine Abweisung spart den EEPROM-Schreibzyklus. Der Matrix-Lauf belegt die
Build-Verzeichnisse für seine Dauer (R1), nicht das Gerät.

**Hartkodierte Grenzen?** Gültigkeitsgrenze Unix **1'767'225'600** (2026-01-01), Konstante NTP→Unix
2'208'988'800; Gültigkeit bis **2106-02-07** (`uint32`), Kette bis **31.12.2099** (RTC). M1:
Startauswertung ab **Jahr 2026**. M2: **60 s** Frist ohne IP. Bestätigung 600 s / 5 s / 15 s /
Mindestabstand 10 s / drei Schnellabfragen. Zeitzone ±12. A69 k = 0…7, 8 Timer. A60 5 ms. C35
5 min. A35 60 s. **Flash-Gate 1'024 Byte**, für den Teilbuild und alle 72 Varianten (§1).

### Secure by design

**`innerHTML`?** Keine neue Ausgabe.

**Fremddaten?** **Die NTP-Antwort ist Fremddatum vom Netz** — Länge begrenzt, Plausibilität am ESP,
Form, Totbereich und Bestätigung am STM.

**Credentials in Logausgaben?** Keine: `var ignoriert` nur Code und Index; `ntp verworfen` nur
Grund; `ldr: … abgewiesen` nur Rohwerte.

**Neue schreibende Endpunkte?** Keine.

### Stable & reliable

**Fehler ausgewertet?** A62 verwirft und meldet; A57 weist ab und meldet; A60 meldet Fristablauf;
der Matrix-Lauf endet bei einem Bau- oder Gate-Fehler mit Exit ≠ 0 und nennt die Variante.

**Leere `catch`?** Keine neuen.

**Stille Verwerfungen?** A53 und A35 machen stille Verwerfungen sichtbar. **Benannte Lücken:** Die
PWA meldet „gespeichert", auch wenn A57 (C) den Messwert abweist (§5a); die Restpunkte aus §3.4.
**Risiko der Einsparungen:** Eine gekürzte oder entfernte Logzeile, nach der ein Werkzeug sucht,
liesse dieses Werkzeug still nichts mehr finden — darum die Abhängigkeitsliste in S3.1, die
geschützte Zeile und die Rückfragepflicht in S3.2.

**Still zurechtgebogen?** A62 klemmt nichts. Die Zeitzone über 12 fällt **mit** Logzeile auf die
Vorgabe. A57 lässt den alten Wert stehen.

**Zustand nach Abbruch?** A62: Nach einem Reset ist die Bestätigung leer; die RTC überbrückt. M2:
Jedes Schalten verwirft die Vormerkung, ohne IP greift die 60-s-Frist; offen bleibt der Fall
„Restore kommt nie" (§3.4). A53: `GTq` läuft durch denselben Ausgang. A67: Fehlschlag ⇒
`SYNCVARS`. **Halt am Gate in S3:** Jeder Punkt ist ein eigener Task mit eigenem Testbau.

**Flags aufgelöst?** A62: `cand_ok` bei jeder Übernahme gelöscht; `schnell` zurückgesetzt. M2:
`night_off_due` gelöscht beim Anwenden, beim Ablauf der Frist und bei jedem Schalten — ausser im
benannten Restpunkt „Restore kommt nie" nach IP. A67: `is_online` durch `SYNCVARS` oder den
Neustart. A69: Merker „erstes übernommenes `TIME`" nach der Auswertung gelöscht.

---

## Verworfene Alternativen

**Ent-17, verworfene Zuschnitte:** **S3 vor S2** — A60 wäre nicht mehr gegen denselben Stand
gemessen, und die Flash-Analyse arbeitete gegen eine Map, die sich danach noch ändert (§0.2).
**Alles in S1 belassen und Sparstufen anwenden** — vom Nutzer nicht gewählt. **Einsparungen und
A60 in einem Release** — A60 wäre nicht isoliert.

**M2: beim Start sofort ausschalten** (ursprünglicher Entwurf). Unterdrückte den IP-Lauftext,
erfüllte Ent-2 (4) nicht. **M2: Restore-Bedingung erweitern.** Architektur-Invariante; stattdessen
der Innenzweig.

**Matrix: in `release-zip` einbauen.** Hätte den normalen Build verlangsamt und verändert — Ent-18
verlangt ein eigenes Ziel. **F411 mit dem Original-Startup.** Weicht vom Teilbuild ab; Ent-18:
`startup_stm32f411xe.S` bleibt.

**A62 ohne Ära (genau 10 Ziffern, Untergrenze 2026).** Vom Nutzer verworfen (Ent-3). **Ära mit
64-Bit-Arithmetik.** Unnötig. **Feste Zeitgrenze am STM statt Bestätigung.** Hilft gegen 0 und
verstümmelte Werte nicht (§2.3). **Protokollwechsel.** Dreht die Reihenfolge ESP vor STM (§2.4).
**Ära-Regel auch im ESP.** Der ESP rechnet nicht mit der Zeit.

**A57 (A) nur schliessen.** Vom Nutzer verworfen (Ent-7). **A57 auch im Zahlen-Setter.** Bräche den
Backup-Import. **Eigene Antwort der PWA auf die Abweisung.** Braucht einen wartenden Endpunkt.

**A69: Displayleistung im EEPROM.** Schreibzyklus je Schalten, nach Stromausfall ohnehin veraltet.

**A23 (b) EEPROM neu lesen.** Verliert IR-Schritte und DC während der Animation.

**A53 V1/V3.** Verschachtelte Kommandos im Spiel bzw. der ESP kennt das Spielende nicht.

**A60 mit RM0008-Verfahren oder ACK-Polling.** Nicht belegt bzw. Folgearbeit.

**A35 (c) NAK.** Nachsendestürme bei Versionsversatz.

**C35 (B)/(C).** (A) genügt; (C) kostet ESP-RAM.

**A21: `setRxBufferSize` sofort.** Erst messen.

---

## Versionsfolgen

Je Einspielschritt **nur die geänderte Komponente** (DIR-004). Testbauten und Matrix-Läufe heben
keine Version an. Erwartete Nummern nach dem Stand vom 11.10.2026 (Momentaufnahme).

| Schritt | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` | Tag |
|---|---|---|---|---|---|
| S1 (A62 mit Ära, A69 samt M1/M2, A68, N1) | ☐ 3.2.29 | — | — | — | `release/3.2.29-3.2.31-1.4.95` |
| S2 (A60) | ☐ 3.2.30 | — | — | — | `release/3.2.30-3.2.31-1.4.95` |
| S3 (Einsparungen, A23, A53, A57, A67-STM) | ☐ 3.2.31 | — | — | — | `release/3.2.31-3.2.31-1.4.95` |
| E (A35, NTP, `ovr=`, A67-ESP) | — | ☐ 3.2.32 | — | — | `release/3.2.31-3.2.32-1.4.95` |
| P (C35) | — | — | ☐ 1.4.96 | ☐ angehoben | `release/3.2.31-3.2.32-1.4.96` |

Nur der `release-engineer` führt das aus (R4). Der Tag wird **je Einspielschritt** gesetzt und
gepusht (DIR-011).
