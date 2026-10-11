# Design — Paket 2026-10-11 „Nachtfenster, Netzzeit, Spiele, I²C, Brücke" (A23, A69, A53, A57, A60, A61, A62, A63, A35, A21, A34, C35)

**Erstellt:** 2026-10-11, Stand `5c34d80`. **Freigegeben** vom Nutzer am 11.10.2026 (Ent-16);
Ent-1 bis Ent-15, Ent-3b (Bestätigungsregel, §2.4) und Ent-14e (Freigabe AKE.2) entschieden, am
selben Tag eingearbeitet. Momentaufnahme (DIR-006).

**Entscheidungsvermerk 11.10.2026.** **Ent-1** (a), DC und IR führen die Sicherung nach (§5) ·
**Ent-2** nur Displayliste, erstes übernommenes `TIME`, übersprungenes Ereignis nachholen,
Startsequenz wie heute (§3) · **Ent-3 Ära-Rechnung jetzt** — gegen die Empfehlung (§2) · **Ent-3b**
Bestätigungsregel wie vorgeschlagen (§2.4) · **Ent-4** V2, keine Inaktivitätsgrenze (§6) ·
**Ent-5** (b) (§9) · **Ent-6** (A) (§11) · **Ent-7 (C)** — gegen die Empfehlung (§5a) · **Ent-8**
Werkzeug (§12.1) · **Ent-9** A34 schliessen · **Ent-10** A63 beobachten · **Ent-11** A68 ja (§4) ·
**Ent-12** A67 ja (§7) · **Ent-13** NTP samt Plausibilität, jetzt tragend (§10.1) · **Ent-14** alle
Freigaben erteilt (§15) · **Ent-14e** AKE.2 ausdrücklich freigegeben (§15.1) · **Ent-15** nur
Fristen (§8) · **Ent-16** freigegeben.

**Schrittfolge:** **0 (ohne Firmware) → S1 (STM: A62 mit Ära, A69, A68, A23, A53, A57, A67-STM)
→ S2 (STM: A60) → E (ESP: A35, NTP, `ovr=`, A67-ESP) → P (PWA: C35) → M (Abschlusslauf A21) → T
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
`ntp.cpp`.

---

## 0. Warum diese Reihenfolge

### 0.1 Die Regeln aus `make release-zip` und R3b

`make release-zip` baut STM, ESP **und** die PWA-Assets in einem Zug. Liegt im Baum eine Änderung
an einer Komponente, deren Version der Build **nicht** anhebt, entsteht ein Fabrikat mit alter
Versionsnummer und neuem Inhalt (Lehre aus Paket 2026-10-09). Für zwei STM-Releases
hintereinander gilt dasselbe **innerhalb** der Komponente: Code für S2 im Baum vor dem
S1-Build ginge mit S1 hinaus.

| Regel | Umsetzung in `tasks.md` |
|---|---|
| Produktcode des **nächsten Releases** entsteht erst **nach dem Release-Build** des vorigen | S2.1 hängt an S1.19, E.1 an S2.7, P.1 an E.9 |
| Der **Release-Build** folgt erst **nach dem Gerätenachweis** des vorigen Releases | S2.7 an S1.21 (G1), E.9 an S2.9 (G2), P.6 an E.11 (G3); M.1 an P.8 (G4); T.2 an M.1 (G5) |
| Erlaubt parallel: Gerätenachweis des vorigen und Codearbeit des nächsten | Der Baum wirkt nicht aufs Gerät; gebaut wird erst danach |
| Erlaubt jederzeit: Doku, Spec, Werkzeug, lesende Analysen | Gehen nicht ins Fabrikat. **Ausnahme:** 0.5 (Testplan V3/S1) muss **vor** G1 fertig sein |
| Innerhalb S1 **nacheinander**, je Task ein Testbau | Alle S1-Tasks berühren `main.c`; das Flash-Gate wird je Task gemessen (§1) |

### 0.2 Release-Zuschnitt

| # | Release | Inhalt | Komponente | Hängt ab von | Nachweis |
|---|---|---|---|---|---|
| — | **Schritt 0** | A61-Werkzeug, Messung A61/A34, Befunde (L391 u. a.), Testplan V3/S1 | Werkzeug, Doku | Freigabe | lesend, Mitschnitte |
| 1 | **STM-Release 1** | A62 mit Ära, A69, A68, A23, A53, A57 (C), A67-STM | STM | Freigabe, Ent-3b; 0.5 vor G1 | **G1** |
| 2 | **STM-Release 2, isoliert** | A60 + L360 | STM | Code nach Build 1; Build nach G1 | **G2** |
| 3 | **ESP-Release** | A35 (b), NTP-Puffer + Plausibilität (tragend), `ovr=`, A67-ESP | ESP | Code nach Build 2; Build nach G2 | **G3** |
| 4 | **PWA-Release** | C35 (A) | PWA | Code nach Build 3; Build nach G3 | **G4** |
| — | **Abschlusslauf A21** | 30 min Last, Setter-Burst | — | G4 | **G5** |
| — | **`pwa-tester`** | Phasen 0–4 und 9 (DIR-012: mehr als eine Komponente) | — | G5, T.1 | AKZ.6 |
| — | **Schritt Z** | BEFUNDE, Checkliste, CLAUDE.md, Spec-Endstand | Doku | T.2 (Z.2/Z.3 schon nach G2) | — |

| Frage | Antwort |
|---|---|
| Warum **A62 zuerst**? | Spart Flash (`strtoul` fällt weg, Map belegt) und ist Voraussetzung für Ent-2 (2) und die Sprungerkennung in A69: Erst mit A62 weiss der STM, ob eine Netzzeit übernommen wurde |
| Warum alle S1-Punkte in **einem** STM-Release? | Sie teilen keine Logzeile, und jede hat einen eigenen sichtbaren Nachweis: `time: …`/`cmd: set time` (A62), `night: start aus` (A69), Ambilight-`numvar` (A68), `dspcolor[0]` (A23), `spiel: … verworfen` (A53), `ldr: … abgewiesen` (A57). G1 trennt sie über diese Zeilen. Keine ändert I²C- oder EEPROM-Zeiten — die `eep`-Grundlinie bleibt für S2 vergleichbar |
| Warum **A60 isoliert**? | Ändert die Zeit **jedes** I²C-Zugriffs und möglicherweise die Zahl gelesener Bytes. Eine Abweichung danach muss eindeutig A60 gehören; die Vorher-Messung läuft auf S1 |
| Warum **S vor E**, obwohl die ESP-Plausibilität für die Ära tragend ist? | DIR-002: Firmware zuerst, STM vor ESP; A67 braucht die STM-Hälfte vor der ESP-Hälfte. Zwischen S1 und E schützt die Bestätigungsregel des STM (§2.4, Ent-3b) allein — sie fängt eine wiederholte 0 ebenso ab |
| Warum **E vor P**? | Firmware vor PWA; der Abschlusslauf soll alle neuen Stände sehen |
| Warum **M nach P**? | Die Last ändert sich mit C35 — gemessen wird der **neue** Normalbetrieb, plus ein Updatemodul, das die frühere Last trägt (§13) |
| Warum der **`pwa-tester` nach M**? | Er erzeugt selbst viel Setterlast; im Abschlusslauf wäre die Ursache eines Befunds nicht mehr trennbar |

---

## 1. Flash F103

### 1.1 Schätzungen

Ausgangslage **1'908 Byte frei**, Gate **1'024**, Spielraum **884 Byte**. Die Zahlen sind
Schätzungen der Analyse, ausser wo „belegt" steht; ● markiert Schätzungen des `spec-writer`.

| Task | Kosten | Quelle |
|---|---|---|
| A62 `strtoul` entfällt | **−240** | **belegt (Map)** |
| A62 Parser 1–10 Ziffern mit Überlaufprüfung, Gültigkeit als **eine** Grenze auf der Unixzeit | +55…+80 | Analyse (Parser 50–70) + ● |
| A62 Bestätigungsregel (Ent-3b), Zustand rund 16 Byte RAM, eine Logzeile | ● +70…+130 | `spec-writer` |
| A62 Zeitzone beim Laden | ● +4…+12 | `spec-writer` |
| **A62 zusammen** | **≈ −110…−20** | |
| A69 Kern, mit Hilfsfunktion | +120…+220 | Analyse |
| A69 Ent-2 (2) erstes übernommenes `TIME` | ● +10…+30 | `spec-writer` |
| A69 Ent-2 (3) übersprungenes Ereignis | ● +30…+60 | `spec-writer` |
| A68 | ● +10…+30 | `spec-writer` |
| A23 | +50…+90 | Analyse |
| A53 V2 samt `snake.c:291` | +80…+104 | Analyse [SPEKULATION] |
| A57 (C) samt zwei Logzeilen | +30…+50 | Nutzer/Analyse (+30…+40) + ● Logzeilen |
| A67 STM-Hälfte | +30…+50 | Analyse |
| A60 | 0…+60 | Analyse |

### 1.2 Rechnung (mit den Entscheidungen)

| Stand | günstig | ungünstig |
|---|---|---|
| Ausgang | 1'908 | 1'908 |
| nach A62 (Ära, Bestätigung, Zeitzone) | 2'018 | 1'928 |
| nach A69 (Kern + Ent-2 (2) + (3)) | 1'858 | 1'618 |
| nach A68 | 1'848 | 1'588 |
| nach A23 | 1'798 | 1'498 |
| nach A53 (V2) | 1'718 | 1'394 |
| nach A57 (C) | 1'688 | 1'344 |
| nach A67-STM | 1'658 | 1'294 |
| **nach S1** | **≈ 1'658** | **≈ 1'294** |
| **nach S2** (A60) | **≈ 1'658** | **≈ 1'234** |

**Warnung aus dem Vorpaket:** Dort kostete S1 **rund 800 Byte** statt der geschätzten 75–125 —
ein Faktor 6 bis 10. Hier liegen die Zuwächse (ohne die belegte Einsparung von 240 Byte) ungünstig
bei rund **850 Byte**; schon **das Doppelte** brächte den Rest nach S1 auf rund **450 Byte** —
**unter das Gate**. Darum: Testbau nach **jedem** Task, und das Verzichtbare kommt **zuletzt**
(A67-STM als S1.13).

### 1.3 Sparstufen — vorzulegen, keine ohne den Nutzer

In dieser Reihenfolge, je mit gemessener Einsparung:

1. **Ent-2 (3)** zurücknehmen: keine Sprungauswertung (● −30…−60).
2. **A67-STM** zurückstellen (−30…−50); dann entfällt auch die ESP-Hälfte.
3. **A53** V2 → V2b (−70…−90).
4. Logzeilentexte kürzen — **Wortlaut** dann in `requirements.md` und `watch-log.sh`
   gleichziehen.
5. **A52** (toter Zweig `FILE `, rund 25 Byte).
6. `atoi`-Ersatz — **Verhaltensänderung**, eigene Entscheidung.

**Nicht als Sparstufe:** die Bestätigungsregel (§2.4) — sie ist nach Ent-3 die tragende Prüfung
gegen verstümmelte Zeittexte. Ebenso A57 (C), das der Nutzer ausdrücklich gewählt hat.

**Unter 1'024 Byte: anhalten, Zahlen vorlegen** (AKZ.F). Kein Rollout darunter.

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
- **600 s Toleranz** für den regulären Abgleich (Gangabweichung über eine Stunde ist weit
  kleiner); **5 s** für die Bestätigung (zwei Antworten im Abstand von rund 15 s).
- **Die Referenz ist UTC** (die NTP-Sekunde) — die Sommerzeit spielt hier keine Rolle; sie wirkt
  erst in der Umrechnung.
- **Höchstens drei Schnellabfragen** in Folge; danach der reguläre Takt. Ein Server, der
  dauerhaft Unsinn liefert, erzeugt also keine Abfrageschleife.
- Zwei **gleiche** Fehlwerte (zweimal 0) bestätigen sich **nicht**: Erwartet ist 0 + 15.
- `uptime` (`main.c:685`, Sekunden, in der ISR gezählt) ist die Zeitbasis.

**Verworfene Alternativen zu Ent-3b:**

| Variante | Folge |
|---|---|
| **(i) keine Bestätigung**, nur die ESP-Plausibilität | Eine verstümmelte `TIME`-Zeile stellt Uhr **und RTC** für bis zu eine Stunde um Jahrzehnte vor; A69 würde den „Sprung" als übersprungene Timer deuten. Zwischen S1 und E schützt dann gar nichts gegen die 0 |
| **(ii) Protokollwechsel:** der ESP sendet immer 10 Stellen mit führenden Nullen (oder eine Prüfsumme am `TIME`) | Der STM könnte „genau 10 Ziffern" behalten. Braucht den ESP **vor** dem STM (Reihenfolge kippt), und ein alter ESP würde nach 2036 von einem neuen STM abgewiesen |

**Gewählt: die Bestätigungsregel.** Ohne Protokollwechsel, unabhängig vom ESP-Stand, rund
70–130 Byte.

### 2.5 Wechselwirkungen

- **A69 Ent-2 (2):** „erstes übernommenes `TIME`" heisst nach §2.4 „erstes **bestätigtes**".
- **A69 Ent-2 (3):** Ein bestätigter Sprung ist ein Sprung — die Nachholregel greift.
- **Zeitzone (`timeserver.c:67`):** Betrag über 12 ⇒ nicht übernehmen (Vorgabe), eine Logzeile;
  dieselbe Grenze wie der Setter (`:196`).
- **Kein** `strtoul` mehr im Fabrikat (Map, S8b), und weiter **kein** `__udivmoddi4` oder
  `__aeabi_ldivmod`.

**Prüfstand a62** (§14): Vektoren über die Ärengrenze (`4294967295` → 2036-02-07 06:28:15,
`0` → 06:28:16, `1`), die Totbereichsgrenzen (`2208988799` gültig → 2106-02-07 06:28:15,
`2208988800` verworfen, `3976214399` verworfen, `3976214400` gültig → 2026-01-01), kurze Formen
(`123`), Formfehler (`""`, `" 1"`, `"+1"`, `"1x"`, 11 Ziffern, `4294967296`, `9999999999`).
**Referenz:** Python `datetime` mit `unix = n − 2208988800 + (2³² falls n < 2208988800)`, gültig
genau dann, wenn 1'767'225'600 ≤ unix < 2³² — **unabhängig** vom C-Code; dazu die Felder über
**gt**. **Bestätigung** mit simulierter `uptime`: Start + n, n + 15 ⇒ übernommen bei der zweiten;
Start + 0, 0 ⇒ nie; stündlicher Abgleich ±30 s ⇒ sofort; einzelner Fehlwert ⇒ verworfen, danach
regulär weiter; echter Sprung zweimal ⇒ übernommen; vier Fehlwerte ⇒ drei Schnellabfragen, dann
regulär; Abgleich über die Ärengrenze ⇒ sofort. **Sabotagen:** Totbereich entfernt (1970er Zeit
angenommen), „genau 10 Ziffern" (Ära-1-Zeit verworfen), Bestätigung entfernt (0 nach dem Start
angenommen) — je FEHL.

---

## 3. S1 — A69: Nachtfenster (Ent-2)

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
Verhalten — der **erste passende** Timer in Tabellenreihenfolge, und nur, wenn er den Zustand
**ändern** würde. Weicht die neue Funktion bei Gleichstand ab, wird das benannt und mit dem Review
entschieden.

### 3.2 Wo sie greift

| Stelle | Wirkung |
|---|---|
| **Start**, nach RTC-Lesen (`main.c:3833`), vor `watchdog_init()` | Ergebnis „aus" ⇒ `set_display_power (FALSE, TRUE)` wie der Timer (`:4290`), Logzeile `night: start aus (hh:mm)`. „ein" oder keins ⇒ nichts |
| **Ambilight (Ent-2 (1))** | Nur über die Displayliste mitgeschaltet, wie beim Aus-Timer; die Ambilight-Liste wird beim Start **nicht** ausgewertet |
| **RTC beim Start nicht gültig (Ent-2 (2))** | Beim **ersten übernommenen** (nach §2.4: bestätigten) `TIME` einmal auswerten, über ein Merkerbit |
| **Vorwärtssprung > 1 min (Ent-2 (3))** | Liegt das jüngste Ereignis **im übersprungenen Intervall** (Abstand < Sprungweite), wird es angewandt; sonst nichts. Erkannt am Unterschied zwischen der zuletzt ausgewerteten und der aktuellen Minute (Minute des Tages plus Wochentag). Rückwärts: nichts |
| **Startsequenz (Ent-2 (4))** | Unverändert; die Auswertung liegt danach. Ob der IP-Lauftext bei ausgeschaltetem Display sichtbar ist, **liest der `stm-developer` am Code** und meldet es |

**Offline senden:** `set_display_power()` sendet bei `! esp8266.is_online` nichts (`main.c:1453`);
der Vollabgleich trägt `display_power` und `ambilight_power` später nach (`vars.c:1831-1832`, ✔ A).
**G1 prüft genau das** (AKW.4).

**`minutes == 1440`:** Bleibt zulässig und unwirksam; der Prüfstand hat den Fall.

### 3.3 Warum nicht einfacher

„Der letzte vergangene Timer **des Tages**" reicht nicht: Ein Aus-Timer um 23:00 am Vortag und
ein Neustart um 01:00 fänden nichts. Darum bis zu sieben Tage zurück — 64 Vergleiche.

---

## 4. S1 — A68: Ambilight-Kopie (Ent-11)

`var_send_display_ambilight_power()` (heute `static`, `vars.c:1324`) wird öffentlich und dort
gerufen, wo die Ambilight-Power sich ändert, ohne dass gesendet wird — mindestens in
`set_display_power()` bei `do_sync_ambilight` (`main.c:1437-1440`), **hinter** derselben
`is_online`-Bedingung wie die Display-Power. Weitere Stellen (u. a. Ambilight-Timer
`main.c:4300-4304`) stellt der `stm-developer` als **Bestandsliste** fest (AKB.1). **Keine zweite**
Sendefunktion. A56 bleibt eigener Punkt.

---

## 5. S1 — A23: Displayfarbe (Ent-1)

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

## 5a. S1 — A57: „Messwert übernehmen" weist eine Inversion ab (Ent-7 = C)

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
  Abweisung meldet er damit den **alten** Wert; die ESP-Kopie bleibt richtig. Keine Änderung nötig.
- **Sichtbare Meldung in der PWA — nicht ohne neue API, ✔ SW:** `/api/ldr_min_set` antwortet
  `{"ok":true}` unmittelbar nach dem RPC (`http.cpp:10077-10086`), **bevor** der STM gemessen hat;
  die PWA zeigt daraufhin „gespeichert" (`runSimpleAction`, `app.js:13100`, `:13266`). Den
  gemessenen Wert kennt sie nicht, ein unveränderter Wert ist auch ohne Abweisung möglich. Eine
  Meldung bräuchte einen Endpunkt, der auf die Rückmeldung des STM wartet. **Als Hinweis**:
  `doc-writer` trägt in Z.1 einen neuen PWA/ESP-Befund ein; nicht in diesem Paket.
- Kosten: +30…+50 Byte.

**A57 wird erst nach G1 geschlossen** (AKE.2), nicht in Schritt 0.

---

## 6. S1 — A53: Spiele (Ent-4 = V2)

- In beiden Spielschleifen (`tetris.c:655-680`, `snake.c:240-265`) ein Zähler für jede Zeile, die
  `esp8266_get_message()` liefert und die Schleife **nicht** als `GT`/`GS` auswertet. Zeitablauf
  ohne Zeile zählt nicht. Ob eine reine Quittung mitzählt, entscheidet der `stm-developer`
  (Empfehlung: ja — einfache Regel).
- Nach `tetris()`/`snake()` in `schedule_esp8266_games()` (`main.c:2946`): bei Zähler > 0
  `var_sync_pending = 1` und `spiel: %u Zeilen verworfen`. Der Vollabgleich läuft regulär über den
  Vormerkweg (`main.c:4040`).
- **`snake.c:291`:** `status` vor dem ersten Lesen initialisieren.
- Keine Inaktivitätsgrenze (Ent-4).

**Was bleibt:** Der Setter während des Spiels ist **verloren** — sichtbar und danach konsistent.

---

## 7. S1 und E — A67: OTA-Ankündigung (Ent-12)

| Seite | Änderung |
|---|---|
| **STM** (S1.13) | Im Zeilenparser (`esp8266.c`) eine **neue** Zeile mit exaktem Vergleich, Vorschlag `OTA start`; setzt **nur** `esp8266.is_online = 0`. Nicht `esp8266_cap_var_crc_seen` (`:550`, Risiko R-3) |
| **ESP** (E.4) | In **allen drei** OTA-Pfaden — `http.cpp:13009`, `:7590`, `:13402` — die Zeile **vor** dem Download; bei Fehlschlag `SYNCVARS` (`esp8266.c:639`) |

Der Wortlaut wird **einmal** festgelegt (S1.13) und wörtlich übernommen (E.4). Am Gerät
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

**Wirkung (gerechnet):** `ms ≈ 16·z + (0,2…0,3)·n + 1` — Vorher/Nachher-Messung AKI.2.
**Doku (Z.2, Z.3):** Checkliste und `CLAUDE.md` auf den gemessenen Stand.

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
Gegenprobe mit synthetischem Mitschnitt; Review `code-reviewer`.

### 12.2 Messung A61/A34

Über den Mitschnitt des ESP-OTA vom 10.10.2026 23:06 (L380), den STM-Reset vom 11.10.2026
01:58:14 (L387) und jeden weiteren Abgleich im Langzeitmitschnitt. A61 bleibt offen, falls ein
Abgleich nahe dem 30-s-Budget liegt.

### 12.3 Befund-Nachführung (`doc-writer`)

| Zeile | Nachführung |
|---|---|
| **L391 (reserviert)** | NTP-Antwort über 48 Byte überschreibt globale Variablen (`ntp.cpp:74`, `:76`, `:18`); vom Netz auslösbar; Massnahme in Gruppe C, Verweis auf Ent-13 |
| **L331 / A57** | Schluss „im EEPROM stand also nicht 18/17" trägt nicht (`ldr.c:128-137` heilt beim Start und schreibt); Mechanismus `ldr.c:168-226`; Ent-7 (C) beschlossen, **Schliessung nach G1** |
| **L360 / A60** | **zwei** BUSY-Schleifen (`i2c.c:413`, `:472`); vor `watchdog_init()` steht die Uhr für immer |
| **L371 / A62** | Fundstelle `main.c:3274`; der ESP sendet nie leer; realistische Auslöser 0, verstümmelt, über 10 Ziffern; **Ära-Rechnung beschlossen** (Ent-3); die `uint32`-Differenz bildet Ära 1 schon richtig ab; **neue Grenze 31.12.2099** (RTC, `rtc.c:204`, `:241`) |
| **L233 / A35** | Fundstelle `ESP-uclock.ino:1468-1479`; Teil 1 samt Zuordnung gelöst; `a=9` vom 10.10. war echt; offen nur Übernahme im engen Sinn; kein hoher Befund mehr |
| **A69** | Fundstelle `night.c:155`; Zeitsprung-Lücke |
| **A61** | Fundstelle `main.c:4037-4048`; Ergebnis aus 12.2 |
| **A53** | „mit A16 zusammen" überholt; `snake.c:291` |
| **A34** | geschlossen (Ent-9), Reste überführt (A61, A19, A53) |
| **A63** | Beobachtung, offen in der ToDo-Liste (Ent-10) |
| **A21** | blinder Fleck ESP-Empfangspuffer; Schliessung erst nach G5 |

### 12.4 Testplan V3/S1 (`doc-writer`, vor G1)

`TESTPLAN-PWA.md` V3 und S1 bekommen ein **Soll im Nachtzustand**: Gilt beim Reset ein
Aus-Fenster, ist das Display danach **aus** (ab STM-Release 1; vorher an, L390). Die Rückstellung
nach S1 im Aus-Fenster ist „wieder aus". Nach dem Reset erscheint zusätzlich
`time: sprung, bestaetigung angefordert` (§2.4) — Soll, kein Fehler.

---

## 13. M — Abschlusslauf A21

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

| Name | Ort | Gegenstand | Gegenprobe (DIR-014) |
|---|---|---|---|
| **a62** | `stm/a62/` | Parser, Ärengrenze, Totbereich, Bestätigung, Zeitzone; Referenz Python `datetime` und gt | alter Stand nimmt `""`/`"0"` an; Sabotagen Totbereich, „genau 10 Ziffern", Bestätigung ⇒ FEHL |
| **a69** | `stm/a69/` | jüngstes Ereignis gegen Minuten-Referenz, alle 10'080 Startpunkte; Start; Sprünge; A68-Sendeprotokoll | Sabotage `<=`→`<`, Wochentagsumlauf; alter Stand ⇒ FEHL |
| **a23** | `stm/a23/` | Moduswechsel, Sicherung (DC, IR), Sendung | alter Stand behält Momentanfarbe ⇒ FEHL |
| **a53** | `stm/a53/` | Spielschleife, Zähler, `var_sync_pending` | alter Stand ⇒ 0 ⇒ FEHL |
| **a57** | `stm/a57/` | `ldr_set_min/max_value()` mit ADC- und EEPROM-Attrappe: unter/gleich/über der Gegengrenze, Schreibzyklen, Logzeile | alter Stand nimmt die Inversion an ⇒ FEHL; **L59 besteht weiter** |
| **a67** | `stm/a67/` | Zeilenparser `OTA start` | alter Stand ignoriert ⇒ FEHL |
| **a60** | `stm/a60/` | Fristen mit Attrappen | Sabotage „Frist nie gesetzt" hängt ⇒ FEHL |
| **a35** | `esp/a35/` | Zählung, `V…` ausgenommen, Drosselung | alter Stand zählt nichts ⇒ FEHL |
| **ntp** | `esp/ntp/` | Paketlängen, Wächterbyte, LI/Stratum/Transmit 0 ⇒ kein `TIME` | alter Stand überschreibt das Wächterbyte und sendet `TIME 0` ⇒ FEHL |
| **c35** | `pwa/c35/` | Abrufzahl je Modul | alter Stand ≈ 120 / 30 min ⇒ FEHL |
| **statisch AKK.2** | Lead | drei OTA-Pfade | Ausgangsstand ⇒ schlägt an |
| **abgleich** | bei 12.1 | Auswerteregel A61 | falscher Auslöser ⇒ FEHL |
| **gt, EP, W2, L59** | bestehen | müssen **weiter bestehen** | — |

**Zähl nach** (DIR-014): Fallzahl je Bericht, vom Lead gegen die Sollzahl verglichen.

---

## 15. Gerätenachweise

**Für alle:** `./tools/watch-log.sh` läuft **vor** jedem Einspielen und während jedes Laufs
(DIR-013); Smoketest nach jedem Einspielen (DIR-009); schreibend nur im Rahmen von Ent-14 und
Ent-14e, mit Rückstellung. **Nie** `GET /?a`, keiner der Endpunkte aus R5. Eingespielt wird vom
**Lead**.

| Nachweis | Nach | Art | Inhalt |
|---|---|---|---|
| **G1** | S1-Flash | schreibend + lesend | Ablauf unten |
| **G2** | S2-Flash | schreibend (S25/S26, 32-Byte-Wert, zurück) + 60 min lesend | AKI.2 nachher, AKI.3 |
| **G3** | ESP-OTA | lesend, 60 min | AKQ.2/AKU.3/AKO.2; Δ`v` beim OTA (Vorher +2); Smoketest samt Update-Quelle; `install-app.sh --check` |
| **G4** | PWA-Hochladen | lesend | AKC.2; `check-pwa.sh` |
| **G5** | — | Abschlusslauf | AKM.1, AKM.2 |

### 15.1 Ablauf G1

Ziel: **ein** Reset im Aus-Fenster belegt A69, die EEPROM-Frage aus A23 und die Bestätigungsregel
nach einem Start.

1. **Vor dem Flash** (lesend): `dspcolor[0]`, `numvar` 17/18, Display-/Ambilight-Power, der
   aktuelle LDR-Rohwert und `numvar[8]` (Auto-Helligkeit) festhalten; Wache läuft.
2. **S1-Flash am Tag** (Lead); Smoketest. Im Mitschnitt nach dem Start: `time: sprung,
   bestaetigung angefordert`, rund 15 s später `cmd: set time to …` (AKN.6).
3. **Tagtests:**
   - AKN.6: die stündliche Netzzeit wird **ohne** Bestätigung übernommen, `read rtc` zur Wanduhr;
   - AKD.2 Teil 1: Regenbogen und Tageslicht, je ein/aus;
   - AKS.3: Spiel mit Setter;
   - **AKE.2 (Ent-14e, ausdrücklich freigegeben):** Maximum per Zahlen-Setter kurz auf 1,
     „Minimum übernehmen" ⇒ `ldr: minimum abgewiesen`, `numvar[17]` unverändert; Maximum zurück
     auf 4095. Minimum per Zahlen-Setter kurz auf 4094, „Maximum übernehmen" ⇒
     `ldr: maximum abgewiesen`; Minimum zurück auf 0. Nur bei ausgeschalteter Auto-Helligkeit und
     einem Rohwert zwischen 2 und 4093;
   - **AKI.2 vorher:** S25/S26 über S27 zurück, 32-Byte-Wert zurück.
4. **Im Aus-Fenster** (nach dem Aus-Timer des Nutzers, 01:00): **ein eigener STM-Reset**
   (Ent-14 a) — AKW.4, AKD.2 Teil 2 (`dspcolor[0]` nach dem Abgleich gleich der Nutzerfarbe), AKN.7
   (Bestätigung nach dem Start, auch im Fenster).
5. **Am Morgen:** AKW.5. Danach ist G1 abgeschlossen; der S2-Release-Build darf folgen.

**Nicht** über den S2-Flash: A60 ändert den EEPROM-Lesepfad, eine Abweichung wäre nicht
zuordenbar.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Schritt |
|---|---|---|---|
| `src/main.c` | A62 Zeitzweig `:3269-3301` samt Bestätigung; A69 Start (`:3833`) und Sprung; A68 `set_display_power()`; A53 `schedule_esp8266_games()` | `stm-developer` | S1 |
| `src/night/night.c`, `night.h` | A69 | `stm-developer` | S1 |
| `src/timeserver/timeserver.c` | A62 Zeitzone beim Laden `:67`; Umrechnung `:253` bleibt | `stm-developer` | S1 |
| `src/vars/vars.c`, `vars.h` | A68 | `stm-developer` | S1 |
| `src/display/display.c` | A23 | `stm-developer` | S1 |
| `src/tetris/tetris.c`, `src/tetris/snake.c` | A53 | `stm-developer` | S1 |
| `src/ldr/ldr.c` | A57 (C) | `stm-developer` | S1 |
| `src/esp8266/esp8266.c` | A67 | `stm-developer` | S1 |
| `src/i2c/i2c.c` | A60 | `stm-developer` | S2 |
| `ESP8266/ESP-uclock/vars.cpp` | A35 | `esp-developer` | E |
| `ESP8266/ESP-uclock/ntp.cpp` | NTP-Puffer, Plausibilität | `esp-developer` | E |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | `ovr=`, Meldeweg | `esp-developer` | E |
| `ESP8266/ESP-uclock/http.cpp` | A67 | `esp-developer` | E |
| `ESP8266/ESP-uclock/data/app/app.js` | C35 | `pwa-developer` | P |
| `tools/**` | Auswerteregel, Prüfstände, `watch-log.sh`, statische Prüfung | Lead | 0, alle |
| `CHANGELOG.md` | je Release | Lead | S1, S2, E, P |
| `BEFUNDE.md`, `TESTPLAN-PWA.md`, `knowledge/architecture-checklist.md` | | `doc-writer` | 0, T, Z |
| `CLAUDE.md` | Hardware-Abschnitt I²C | Lead | Z |
| Versionsdateien | STM (S1, S2), ESP (E), `APP_VERSION` + `CACHE_NAME` (P) | `release-engineer` | je Release |

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015). `ESP-uclock.ino` hat
**gemischte** Zeilenenden; `http.cpp` ist UTF-8; die `src/**`-Dateien sind ASCII oder ISO-8859-1 —
**nachsehen, nicht annehmen**.

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**PWA parallel zu Legacy?** Ja. C35 ändert nur, wie oft die PWA fragt. A67 ergänzt den
Legacy-OTA-Pfad um dieselbe Ankündigung wie die API.

**Wetter-Endpunkte?** Unberührt.

**Restore-Bedingung um `pending_ticker_restore`?** Nicht vereinfacht. A69 schaltet beim Start das
Display aus, bevor ein Ticker läuft; ein später eintreffender IP-Lauftext läuft über die
bestehende Restore-Bedingung. A23, A53, A57 berühren sie nicht.

**Nur `.gz`?** Nicht berührt.

**Richtige Schicht?** A62: Der STM prüft seinen Eingang (Form, Totbereich, Bestätigung), der ESP
seine Quelle (Plausibilität) — die Ära liegt dort, wo umgerechnet wird. A57 dort, wo der Messwert
übernommen wird, nicht im Zahlen-Setter (Import). Die übrigen wie in den Abschnitten.

### Scalable systems

**STM-Kommandos?** A23: eines je Wechsel auf Modus 0. A68: eines je Timer-Aus. A53: ein
Vollabgleich nach einem Spiel mit verworfenen Zeilen. A57: keins neu (die Rückmeldung gab es
schon). A62: eine NTP-Anfrage mehr je Start, höchstens drei Schnellabfragen bei Fehlwerten.

**Byte je Minute auf der UART?** Neue Zeilen nur bei Ereignissen; `ovr=` verlängert die
Heap-Zeile einmal je Minute um rund 6 Zeichen.

**Unter 20 s Watchdog?** Ja. A69 beim Start 64 Vergleiche; A60 begrenzt die zwei BUSY-Schleifen
erstmals; A62 rechnet konstant.

**PWA-Polling?** C35 senkt `update_status` im Hauptmodul von 4 auf rund 0,2 je Minute.

**Wie lange blockiert der Hauptloop?** A60 verkürzt EEPROM-Schreibaufrufe (gerechnet 110 → 35–45 ms
für 32 Byte). A57: eine Abweisung spart den EEPROM-Schreibzyklus.

**Hartkodierte Grenzen?** Gültigkeitsgrenze Unix **1'767'225'600** (2026-01-01), Konstante NTP→Unix
2'208'988'800; Gültigkeit bis **2106-02-07** (`uint32`), Kette bis **31.12.2099** (RTC) — beide
am Code kommentiert. Bestätigung 600 s / 5 s / 15 s / drei Schnellabfragen. Zeitzone ±12. A69
k = 0…7, 8 Timer. A60 5 ms. C35 5 min. A35 60 s.

### Secure by design

**`innerHTML`?** Keine neue Ausgabe.

**Fremddaten?** **Die NTP-Antwort ist Fremddatum vom Netz** — Länge begrenzt, Plausibilität am ESP,
Form, Totbereich und Bestätigung am STM. Ohne diese Kette könnte nach Ent-3 ein Fremdpaket die Uhr
um Jahrzehnte verstellen.

**Credentials in Logausgaben?** Keine: `var ignoriert` nur Code und Index; `ntp verworfen` nur
Grund; `ldr: … abgewiesen` nur Rohwerte.

**Neue schreibende Endpunkte?** Keine.

### Stable & reliable

**Fehler ausgewertet?** A62 verwirft und meldet; A57 weist ab und meldet, die Rückmeldung trägt den
alten Wert; A60 meldet Fristablauf.

**Leere `catch`?** Keine neuen.

**Stille Verwerfungen?** A53 und A35 machen stille Verwerfungen sichtbar. **Benannte Lücke:** Die
PWA meldet „gespeichert", auch wenn A57 (C) den Messwert abweist (§5a) — als Hinweis in Z.1.

**Still zurechtgebogen?** A62 klemmt nichts. Die Zeitzone über 12 fällt **mit** Logzeile auf die
Vorgabe. A57 lässt den alten Wert stehen, statt den neuen zu verbiegen.

**Zustand nach Abbruch?** A62: Nach einem Reset ist die Bestätigung leer — die erste Antwort wird
Kandidat; die RTC überbrückt. A53: `GTq` läuft durch denselben Ausgang. A67: Fehlschlag ⇒
`SYNCVARS`.

**Flags aufgelöst?** A62: `cand_ok` bei jeder Übernahme gelöscht; `schnell` zurückgesetzt. A67:
`is_online` durch `SYNCVARS` oder den Neustart. A69: Merker „erstes übernommenes `TIME`" nach der
Auswertung gelöscht.

---

## Verworfene Alternativen

**A62 ohne Ära (genau 10 Ziffern, Untergrenze 2026).** Vom Nutzer verworfen (Ent-3). **Ära mit
64-Bit-Arithmetik.** Unnötig — die `uint32`-Differenz trägt die Ära; 64 Bit brächte die 952 Byte aus
S0 zurück. **Feste Zeitgrenze am STM statt Bestätigung.** Hilft gegen 0 und verstümmelte Werte nicht
(§2.3). **Protokollwechsel (10 Stellen mit Nullen oder Prüfsumme an `TIME`).** Dreht die Reihenfolge
ESP vor STM und bricht alte ESPs nach 2036 (§2.4). **Ära-Regel auch im ESP.** Der ESP rechnet nicht
mit der Zeit.

**A57 (A) nur schliessen.** Vom Nutzer verworfen (Ent-7). **A57 auch im Zahlen-Setter.** Bräche den
Backup-Import (L68, A59). **Eigene Antwort der PWA auf die Abweisung.** Braucht einen wartenden
Endpunkt — eigener Befund.

**A69: Displayleistung im EEPROM.** Schreibzyklus je Schalten, nach Stromausfall ohnehin veraltet.

**A23 (b) EEPROM neu lesen.** Verliert IR-Schritte und DC während der Animation.

**A53 V1/V3.** Verschachtelte Kommandos im Spiel bzw. der ESP kennt das Spielende nicht.

**A60 mit RM0008-Verfahren oder ACK-Polling.** Nicht belegt bzw. Folgearbeit.

**A35 (c) NAK.** Nachsendestürme bei Versionsversatz.

**C35 (B)/(C).** (A) genügt; (C) kostet ESP-RAM.

**A21: `setRxBufferSize` sofort.** Erst messen.

---

## Versionsfolgen

Je Einspielschritt **nur die geänderte Komponente** (DIR-004). Testbauten heben keine Version an.
Erwartete Nummern nach dem Stand vom 11.10.2026 (Momentaufnahme).

| Schritt | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` | Tag |
|---|---|---|---|---|---|
| S1 (A62 mit Ära, A69, A68, A23, A53, A57, A67-STM) | ☐ 3.2.29 | — | — | — | `release/3.2.29-3.2.31-1.4.95` |
| S2 (A60) | ☐ 3.2.30 | — | — | — | `release/3.2.30-3.2.31-1.4.95` |
| E (A35, NTP, `ovr=`, A67-ESP) | — | ☐ 3.2.32 | — | — | `release/3.2.30-3.2.32-1.4.95` |
| P (C35) | — | — | ☐ 1.4.96 | ☐ angehoben | `release/3.2.30-3.2.32-1.4.96` |

Nur der `release-engineer` führt das aus (R4). Der Tag wird **je Einspielschritt** gesetzt und
gepusht (DIR-011).
