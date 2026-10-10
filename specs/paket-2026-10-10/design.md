# Design — Paket 2026-10-10 „Testbefunde und Altlasten" (A59, A2, A50, A55, C47, C55, C44, B44–B47)

**Erstellt:** 2026-10-10, Stand `675e3fd`. **Freigegeben** vom Nutzer am 10.10.2026; Ent-1 bis
Ent-14 entschieden, alle nach Empfehlung (`requirements.md`). Am selben Tag mit den Ergebnissen
aus Schritt V nachgeführt (§1). Momentaufnahme (DIR-006).

**Entscheidungsvermerk 10.10.2026.** Wo dieses Dokument Varianten nennt, gilt: **Ent-1** Spec
freigegeben samt der schreibenden Gerätenachweise (G1, G3) und S25/S26 im Durchlauf (T) ·
**Ent-2** R2 (§7.3) · **Ent-3** 0/4095 (§3) · **Ent-4** (a) Reset des S2-Flashs, (b) STM-Reset
nach dem Durchlauf, danach Phase 9 erneut (§10) · **Ent-5** S0 als eigenes Release (§0.2, §2) ·
**Ent-6** (a), Neuerkennung je Minute und „3 in Folge" (§4.3) · **Ent-7** F1c; bestätigt das
Review S2.5 die Zählerquelle nicht auf beiden Zielen, F1b (§5) · **Ent-8** P3 (§6) · **Ent-9**
immer abweisen (§9.1) · **Ent-10** strenge Fassung (§8.1) · **Ent-11** ja, „invalid" im
ESP-Release (§4.4) · **Ent-12** wie vorgeschlagen (§8.2) · **Ent-13** kein `SKIP ROM`, ROM-ID per
CRC (§4.1) · **Ent-14** Sensor wird nicht abgezogen, Prüfstand DS genügt.

**Schrittfolge:** **V (erledigt) → S0 (STM, `my_gmtime`) → S1 (STM: A59, A2) → S2 (STM: A50, A55)
→ E (ESP: C47, C55, C44) → P (PWA) → T (`pwa-tester`) → Z**.

---

## 0. Warum diese Reihenfolge

### 0.1 Die Regel aus `make release-zip`

`make release-zip` baut STM, ESP **und** die PWA-Assets in einem Zug, und `tools/deploy.sh` legt
das Fabrikat auf den Update-Server. Liegt im Baum eine Änderung an einer Komponente, deren
Version dieser Build **nicht** anhebt, entsteht ein Fabrikat mit **alter Versionsnummer und neuem
Inhalt** — beim STM eine Firmware, die sich als alte meldet; bei der PWA ein `app.js` unter altem
`CACHE_NAME`, das der Service Worker womöglich nie holt.

**Lehre aus Paket 2026-10-09** (`tasks.md`, „Endstand"): Der STM-Teil von D kam als STM 3.2.25
**zusammen mit ESP 3.2.29**, weil ESP-Änderungen im Baum lagen. Ent-6 jenes Pakets („eigenes
Release") war danach nur noch über einen Umweg erfüllbar.

**Daraus folgt hier:** Jeder Release-Build enthält Produktänderungen **nur** der Komponenten, die
er anhebt (AKZ.3). Weil alle drei Komponenten in diesem Paket berührt werden und jede einzeln
ausgeliefert werden soll, wird **komponentenweise nacheinander** gearbeitet:

| Regel | Umsetzung in `tasks.md` |
|---|---|
| Der Produktcode der nächsten Komponente entsteht erst **nach dem Release-Build** der vorigen | S1.1 hängt an S0.6, S2.1 an S1.7, E.0 an S2.7, P.1 an E.6 |
| Der **Release-Build** einer Komponente folgt erst **nach dem Gerätenachweis** der vorigen | S1.7 an S0.8 (G0), S2.7 an S1.9 (G1), E.6 an S2.9 (G2), P.9 an E.8 (G3) — sonst wären zwei Wirkungen am Gerät nicht trennbar (R3b) |
| Erlaubt parallel: Gerätenachweis der vorigen und Codearbeit der nächsten | Der Baum beeinflusst das Gerät nicht; Builds laufen erst danach |
| Erlaubt jederzeit: Doku (`*.md`), Spec, Werkzeug, lesende Analysen | Gehen nicht ins Fabrikat |

Zwei Komponenten in **einem** Release gäbe es nur, wenn beide ohnehin gleichzeitig geändert
werden müssten; das ist hier nirgends der Fall (A55 ist mit P3 STM-allein, Ent-8).

### 0.2 Die Reihenfolge der Komponenten

| Frage | Antwort |
|---|---|
| Warum **S0 zuerst**? | Ohne S0 passen A59 und A2 **nicht** vor A50 in die 96 Byte (§1.2). Mit S0 bleibt die Reihenfolge des Nutzers erhalten |
| Warum S0 als **eigenes** Release (Ent-5)? | `my_gmtime()` trägt Datum, Uhrzeit und Wochentag (und damit die Timer). Ein Fehler dort muss am Gerät **eindeutig** S0 zuzuordnen sein, nicht A59 oder A2 |
| Warum **A59 und A2 vor A50 und A55**? | Vom Nutzer festgelegt. A59 trägt die Bereinigung der LDR-Grenzen, die T braucht; A2 ist das älteste offene Thema |
| Warum **A59 und A2 in einem** Release? | Sie teilen keine Funktion und keine Logzeile: A59 wirkt nur auf `N11`/`N12` (sichtbar als `eep a=225`/`a=227`), A2 nur im Temperaturzweig (sichtbar als `DS18xxx temperature`, `ds18xx …`). G1 trennt sie über diese Zeilen |
| Warum **A50 und A55 in einem** Release? | A50 wirkt nur im Fault, der am Gerät nicht auftritt; A55 nur beim ESP-Neustart. G2 ist für beide nur „keine Regression" |
| Warum **S2 vor E**? | Der ESP-OTA des Schritts E **ist** der Gerätenachweis von A55 (AKP.4) — genau die Lage, in der L327 die +4 gemessen hat. Kein zusätzlicher ESP-Neustart nötig |
| Warum **E vor P**? | Die PWA zieht die Regeln aus C55/C44 nach; die Firmware-Regel muss zuerst stehen (CLAUDE.md, DIR-002: „Firmware zuerst", L241). Und die Anzeige aus A2 setzt S1 voraus |
| Warum **P zuletzt** und B44–B47 nicht vorab? | Sie hängen an nichts, aber ein PWA-Release vor S0 hätte die STM-Arbeit bis nach seinem Build blockiert (§0.1). In P bündeln sie sich mit C44/C55-PWA und A2-UI zu einem Release, das der `pwa-tester` ohnehin abschliessend prüft |

---

## 1. Ergebnisse aus Schritt V (eingearbeitet)

### 1.1 Befunde

| Frage | Ergebnis (V, Fundstellen vom Analysten belegt) | Wirkung in dieser Spec |
|---|---|---|
| Flash-Kosten | A59 ≈ 25 Byte; A2 ≈ 50 (CRC-Mindestfassung) bis 85–100 Byte (vollständig); A55 ≈ 6 Byte; **A50 spart** ≈ 180–250 Byte | §1.2 |
| Sparmöglichkeit 1 | `my_gmtime()` mit 64-Bit-`time_t` zieht `__udivmoddi4` und `__aeabi_ldivmod` herein, **952 Byte** (Map `:874`, `:1028`; vom Lead an der Map nachgeprüft). Mit `uint32` gleiches Verhalten 1970–2106 | **S0**, Ent-5 |
| Sparmöglichkeit 2 | `atoi`/`strtoul` ersetzen ≈ 450 Byte, **Verhaltensänderung** | nur Reserve, „Nicht Teil" |
| A52, C42 | A52 ≈ 25 Byte; C42 und Debugtexte **0 Byte** (werden schon weggeworfen) | A52 nur Reserve; C42 entfällt |
| A59 | Kleinster Eingriff: setzen und `ldr_write_config_to_eep()` bei `val ≤ 4095`; Reihenfolge **nicht** sperren (Import `app.js:6536-6538`, L68); Laufzeit `ldr.c:72`, `:92-95` und Start `ldr.c:128-137` fangen sie ab | §3, AKL.1–AKL.2 |
| A2 | **L18 widerlegt die Annahme „Erkennung scheiterte"**: `is_up = 1` schon nach der Präsenz, auch bei falscher ROM-ID (`ds18xx.c:298-303`); danach `MATCH ROM` ins Leere, 255 für immer. Nötig: CRC-8 für Scratchpad **und** ROM-ID, `is_up` nur bei gültiger ID, bei Fehler `gtemp.index = 0xFF` (`tempsensor.c:76`), **kein neues numvar**, Neuerkennung je Minute im Zweig `main.c:4348-4356` (20–40 ms), danach `DS18XX_IS_UP` senden (`vars.c:1430`). Alternative `SKIP ROM` | §4, AKT.1–AKT.6, Ent-6, Ent-13 |
| A50 | Nur der HardFault läuft (drei Handler unerreichbar, kein `SHCSR`); darin keine USART-ISR, `log_flush()` ewig (`uart-driver.h:755`); **nie ein Zeichen angekommen**. Sofortiger Reset ⇒ Reset-Takt 3–4 s, ESP nie erreichbar. Vorschlag 1b: Schleife `main.c:602` auf über 12 s | §5, Ent-7 |
| A55 | Die vier Zeitüberschreitungen sind STM-Sendungen (`N15`/`N31`/`N17`/`T00`). Kleinster Eingriff: im `CAP`-Zweig (`esp8266.c:543-553`) `is_online = 0`; `IPADDRESS`/`SYNCVARS` setzen es wieder; Muster `main.c:3945`, `:3953`; ≈ 6 Byte; `v` +0..1, nicht garantiert 0 | §6, Ent-8 |
| Nebenbefunde | CLAUDE.md nennt `temp_init()` bei `main.c:3140`, es steht bei `:3734`; `tools/logger/README.md:352` beschreibt eine Fault-Ausgabe, die nie ankam | Z.2, Z.3 (Lead) |

### 1.2 Flash-Rechnung F103

Ausgangslage **1'120 Byte frei**, Reserve **1'024**. Die Zuwächse sind Schätzungen aus V; die
Logzeilen aus AKT.3/AKT.4 sind dort ● nicht eingerechnet (Schätzung des `spec-writer`: 40 bis
80 Byte für drei kurze Texte).

| Stand | mit S0 (Ent-5 = ja) | ohne S0 |
|---|---|---|
| Ausgang | 1'120 | 1'120 |
| nach S0 (`my_gmtime`) | ≈ 2'070 | — |
| nach A59 + A2 (S1), samt Logzeilen | ≈ 1'890 bis 1'975 | **≈ 940 bis 1'025 — unter dem Gate** |
| nach A50 + A55 (S2) | ≈ 2'060 bis 2'220 | — |

**Ohne S0** ist die Reihenfolge des Nutzers nicht haltbar: Erst A50 (spart 180–250), dann A59 und
A2, dann A55 — Endstand rund 1'110 bis 1'270, also knapp. Darum Ent-5.

**Regel unverändert:** Testbau nach jedem STM-Task; unter 1'024 Byte **anhalten und vorlegen**
(AKZ.F). Sparstufen in dieser Reihenfolge vorzuschlagen, keine ohne Nutzer: (1) Logzeilentexte
aus AKT.3/AKT.4 kürzen; (2) A52; (3) A55 entfällt; (4) `atoi`-Ersatz (Verhaltensänderung, eigene
Entscheidung).

---

## 2. S0 — `my_gmtime()` mit 32 Bit

**Ausgangslage (✔ am Code `src/base/base.c:450-531`):** `time_t tv` und `time_t days_since_epoch`
sind 64 Bit; die Divisionen `tv / 86400` (`:465`), `tv / (24 * 3600)` (`:484`, `:509`),
`tv / 3600`, `tv / 60` und der Rest `% 7` laufen damit über die 64-Bit-Hilfsroutinen. Einziger
Aufrufer ist `timeserver.c` (`:256`, `:307`), dessen Eingabe `uint32` ist (`:253`).

**Entwurf:** Die Schnittstelle bleibt (`struct tm * my_gmtime (time_t *)`, `base.h:58`), damit der
Aufrufer unverändert bleibt; **intern** wird der Wert sofort in `uint32_t` übernommen und nur noch
damit gerechnet. Die Schleifenlogik (Jahre, Monate, `IS_LEAP_YEAR` `:28`) bleibt **wörtlich**; nur
die Typen ändern sich. Ob der Aufrufer zusätzlich angepasst werden muss, damit keine weitere
64-Bit-Stelle bleibt, entscheidet die Map (AKG.2), nicht die Annahme.

**Verhaltensgrenze, ausdrücklich:** Für Eingaben ab 2³² (nach 2106-02-07) rechnete die alte
Fassung weiter, die neue nicht. Die Eingabe ist `uint32`, solche Werte entstehen nicht.

**Prüfstand GT** (§11): alt gegen neu über den ganzen 32-Bit-Bereich in Stichproben, dazu die
Grenzen; zusätzlich das `gmtime_r()` des Hosts als Referenz — **nicht** als Massstab. Weicht schon
die alte Fassung von der Referenz ab, ist das ein **Befund**, kein Auftrag (der Massstab ist „wie
heute"). Die Sabotage „Jahrhundertregel entfernt" muss am 2100-03-01 anschlagen.

**Warum der Wochentag eigens geprüft wird:** `tm_wday` (`:527`) entscheidet, an welchen Tagen die
Timer schalten. Ein Fehler dort fiele am Display nicht auf, wohl aber nachts.

---

## 3. S1 — A59: LDR-Zahlensetter

**Änderung (`src/main.c:1876-1886`):** Die beiden Readonly-Zweige werden zu:

```
LDR_MIN_VALUE_NUM_VAR:  wenn val <= 4095: ldr.ldr_min_value = val; ldr_write_config_to_eep ()
LDR_MAX_VALUE_NUM_VAR:  wenn val <= 4095: ldr.ldr_max_value = val; ldr_write_config_to_eep ()
```

Die Debugzeile bleibt (Text ohne „readonly"). **Kein** Zurücksenden an den ESP: Er hat den Wert
vor dem Senden selbst gesetzt (`http.cpp:9891`, `:9927`). Die Grenze 4095 steht als Konstante
**einmal** im STM (heute `MAX_VALUE` in `ldr.c:22`, `static`); ob sie über `ldr.h` sichtbar wird
oder eine kleine Setzfunktion in `ldr.c` entsteht, entscheidet der `stm-developer` nach Flash —
**keine zweite Schreibfunktion** für das EEPROM.

**Kosten am Gerät:** `ldr_write_config_to_eep()` schreibt Minimum und Maximum mit zwei
`eep_write()`; unveränderte Bytes kosten keinen Schreibzyklus (Seitenvergleich, C3). Beide liegen
in derselben 32-Byte-Seite (Adresse 225–228). Erwartet: **eine** `eep`-Zeile je Aufruf für den
geänderten Wert, rund 25–65 ms (L370: `z=1` 23–65 ms).

**Bereinigung am Gerät (G1, Ent-3):** `ldr_min_value_set?value=<min>`, dann
`ldr_max_value_set?value=<max>`. Bei 0/4095 ist die Reihenfolge gleichgültig — weder 0 noch 4095
erzeugt mit dem heutigen STM-Stand 5/26 eine Verdrehung.

**Wirkung auf den Testplan:** S25/S26 („Messwert übernehmen") werden **rücknehmbar**: Vorher den
Wert lesen (nach der Bereinigung ist die ESP-Kopie gleich dem STM), danach über S27 zurück. S27
bekommt als Soll zusätzlich die `eep`-Zeile — der einzige **lesende** Beleg, dass der STM den Wert
angewandt hat (`settings_xml` zeigt die ESP-Kopie, L188).

**Wirkung auf den Backup-Import:** Er wirkt danach am STM. Heute zieht der nächste Vollabgleich
die ESP-Kopie still auf den STM-Wert zurück (V).

---

## 4. S1 — A2: DS18xx

### 4.1 CRC-8

**Eine** Funktion `ds18xx_crc8 (const uint8_t *, uint_fast8_t len)` in `ds18xx.c` (Dallas/Maxim,
Polynom 0x31 reflektiert = 0x8C, Startwert 0). Bitweise, ohne Tabelle (Flash). Verwendet für:

- **Scratchpad** in `ds18xx_read_raw_temp()`: CRC über Byte 0–7 gleich Byte 8, sonst `rtc = 0`.
- **ROM-ID** in `ds18xx_init()`: CRC über Byte 0–6 gleich Byte 7, sonst **`is_up = 0`** und
  `rtc = 0` — `ds18xx.is_up = 1` erst **nach** bestandener Prüfung (heute `:303` vorher).

● **Für das Review S1.5:** Neun Nullbyte bestehen die CRC (CRC von Nullen ist 0). Eine
kurzgeschlossene Leitung scheitert schon an der Präsenz; ob ein Nullbyte-Scratchpad bei
bestandener Präsenz vorkommen kann, beantwortet das Review am Eindrahttreiber — und ob dafür eine
Zusatzprüfung (z. B. reserviertes Byte 5 = 0xFF beim DS18B20) nötig ist.

### 4.2 Ungültig heisst 255, überall

`temp_read_temp_index()` (`tempsensor.c:46-79`) setzt `gtemp.index` auf **255**, wenn das Lesen
scheitert — heute bleibt der alte Wert stehen (`:76` liegt im Erfolgszweig). Der STM sendet dann 255
über den bestehenden Weg (`var_send_ds18xx_temp_index()`, `main.c:4367-4370`). Die PWA zeigt 255
bereits als „ungültig" (`app.js:14411-14413`); **kein neues numvar** (V).

Das Merkmal „letzter Messwert gültig" ist **intern** (z. B. ein Zähler ungültiger Messungen in
Folge, §4.3); nach aussen trägt es der Wert 255. Die Logzeile `main.c:4365` gibt bei 255
`DS18xxx temperature: ungueltig` aus statt „127.5".

**Grenze, benannt:** Ein **negativer** Messwert ergibt schon heute 255 (`tempsensor.c:57`) und
erscheint künftig als „Messwert ungültig". Das Indexschema kennt keine Minusgrade („Nicht Teil").

### 4.3 Neuerkennung (Ent-6)

Im Zweig `measure_temperature_flag` (`main.c:4348-4356`, Sekunde 49):

```
wenn ! ds18xx.is_up:
    temp_init ()                       // liest ROM-ID mit CRC, setzt is_up nur bei Erfolg
    wenn ds18xx.is_up:  Zeile "ds18xx erkannt", DS18XX_IS_UP senden, Messung wie bisher anstossen
sonst: wie heute
```

Und bei **Ent-6 (a)** im Lesezweig (`:4358-4376`): Nach **drei** ungültigen Messungen in Folge
`is_up = 0`, Zeile `ds18xx verloren`, `DS18XX_IS_UP` senden; die nächste Minute versucht die
Erkennung neu. Ein gültiger Wert setzt den Zähler zurück.

`var_send_ds18xx_is_up()` (`vars.c:1430-1434`) wird dafür ausserhalb von `vars.c` aufrufbar (oder
eine gleichwertige öffentliche Funktion) — **keine zweite** Sendefunktion für dieselbe Variable.
Gesendet wird nur, wenn `esp8266.is_online` gilt, wie bei der Temperatur selbst.

**Blockade (AKT.5):** Ein Erkennungsversuch kostet laut V 20–40 ms: ein Rücksetzpuls rund 1 ms,
64 Bit ROM-ID rund 5 ms, das Lesen des Scratchpads für die Auflösung rund 6 ms; das Schreiben der
Auflösung (`ds1822B20_write_resolution()`, bis 15 ms Wartezeit) nur, wenn sie abweicht — nach dem
ersten Mal nie mehr. Einmal je Minute, weit unter dem Watchdog.

`onewire_init()` hat eine Sperre gegen Mehrfachaufruf (`onewire.c:237-239`); `ds18xx_init()` darf
deshalb mehrfach laufen. Das bestätigt das Review.

### 4.4 Anzeige

**PWA (P):** Die Zeile „DS18xx online" (`app.js:7700`) zeigt künftig drei Zustände: „nicht
gefunden" (`IS_UP` = 0), „Messwert ungültig" (`IS_UP` = 1, Index 255), sonst wie heute. Der Text
„ungültig" bei 255 (`:917`) wird für diese Stelle „Messwert ungültig". Neuer Schlüssel in `app.js`
und `i18n/en.json`.

**Legacy (E, Ent-11):** `http.cpp:3921-3932` formatiert bei `IS_UP` = 1 **und** Index 255
„invalid" statt „127.5&deg;C".

---

## 5. S2 — A50: `fault_reset()`

**Ausgangslage:** §3 in `requirements.md`. Im HardFault läuft keine ISR; die Ausgabe kam nie an.

**Entwurf (Ent-7):**

```
fault_reset (reason):
    __disable_irq ()
    warte rund 15 s           // F1c: geeicht; F1b: Schleife verlängert
    NVIC_SystemReset ()
```

- **Keine** `log_*`-Aufrufe mehr (AKF.1, statisch geprüft). `reason` bleibt als Parameter, damit
  die vier Handler unverändert bleiben; ob er im Code noch gebraucht wird, entscheidet der
  `stm-developer` (Flash).
- **Warum warten:** Ein deterministischer Fault kurz nach dem Start erzeugte sonst einen
  Reset-Takt von 3–4 s; jeder Start setzt den ESP zurück (`esp8266.c` Reset beim Start), der ESP
  braucht rund 10 s — die Uhr wäre aus dem Netz **nicht mehr flashbar**. Mit rund 15 s Wartezeit
  ist der ESP zwischen zwei Resets einige Sekunden erreichbar.
- **Warum unter 20 s:** Läuft der Watchdog, setzt er nach höchstens 20 s ab dem letzten Nachladen
  ohnehin zurück; eine längere Wartezeit wäre wirkungslos. Vor `watchdog_init()` ist die
  Wartezeit die einzige Grenze.
- **F1c (Empfehlung):** ein Zähler, der im HardFault weiterläuft. Kandidaten: das
  SysTick-Zählregister (`SysTick->VAL`, `COUNTFLAG` pollen — die SysTick-**Unterbrechung** läuft
  nicht, der **Zähler** schon) oder der DWT-Zyklenzähler (Cortex-M3 und -M4). ● Welche Quelle auf
  **beiden** Zielen vor jeder Initialisierung verlässlich läuft, bestätigt das Review S2.5 am Code
  (SysTick wird erst in der Initialisierung konfiguriert).
- **F1b:** die bestehende Zählschleife (`main.c:602`, 2'000'000 Durchläufe) verlängern. 0 Byte,
  aber die Dauer hängt an Takt, Optimierung und Flash-Wartezyklen; der Bericht nennt die
  **gerechnete** Dauer für F103 und F411 und ihre Unsicherheit.

**Prüfstand FR** (§11) prüft den Weg zum Reset; die Wartezeit nur über den Zählerstand.

---

## 6. S2 — A55: ESP-Neustart (Ent-8 = P3)

**Entwurf (V):** Im Zweig `CAP var-crc` (`esp8266.c:543-553`) zusätzlich
`esp8266.is_online = 0`. Folge:

- Der ESP sendet `CAP var-crc` rund 1 s nach seinem Start, vor `FIRMWARE`. Ab dann senden die
  `is_online`-abhängigen Pfade des STM (u. a. `main.c:4034`, `:4073`) nichts mehr — also auch
  `N15`/`N31`/`N17`/`T00` nicht.
- `SYNCVARS` (`esp8266.c:597-626`) oder `IPADDRESS` (`:590-596`) setzen `is_online` wieder;
  `SYNCVARS` stösst zugleich den Vollabgleich an (Weg B), der alle Werte nachträgt.
- Zwischen dem Ende des alten ESP und seiner `CAP`-Zeile (rund 1 s) kann **eine** Sendung in die
  3-s-Wartezeit laufen — daher `v` +0..1, nicht garantiert 0.

**Warum P3 und nicht P2:** P3 nutzt einen Zustand, den es gibt, und ein Lebenszeichen, das jeder
ESP-Start liefert — auch nach einem Absturz oder Watchdog des ESP. P2 bräuchte einen neuen
Zustand mit Höchstfrist und eine Sonderregel, damit eine tote Brücke nicht dauerhaft pausiert.

**Was P3 nicht ändert:** Eine **tote** Brücke sendet kein `CAP` — alles wie heute, samt
Watchdog-Pfad (`vars.c:738-760`). Ein ESP ohne A32 (kein `CAP`) — ebenfalls wie heute.

● **Für das Review S2.5 (AKP.2):** (1) Sendet jeder ESP-Start `SYNCVARS` **oder** `IPADDRESS`
zuverlässig nach `CAP` — auch im AP-Modus? Am ESP-Code belegen (`ESP-uclock.ino`). (2) Gehen beide
Zeilen verloren (Ring, L255), bleibt der STM „offline", bis eine der beiden kommt — derselbe
Zustand wie nach der AP-Taste. Ist das ein Dauerzustand? Wenn ja: als Befund melden, nicht in
dieses Paket ziehen. (3) Liegt `CAP` je zeitlich **nach** einer `IPADDRESS`-Zeile derselben
Sitzung (dann bliebe `is_online` fälschlich 0)? Laut V ist `CAP` die erste Zeile; das Review
bestätigt die Reihenfolge am ESP-Code.

---

## 7. E — C47: Legacy-Seite maskiert

### 7.1 Maskierung für Werte

Eine **neue** Funktion, z. B. `http_send_attr (const char *)`, die zeichenweise ausgibt und genau
`&` → `&amp;`, `<` → `&lt;`, `>` → `&gt;`, `"` → `&quot;`, `'` → `&#39;` ersetzt. **Jedes andere
Byte unverändert.** Bewusst **nicht** `HTTP_ESCAPE_XML` (`http.cpp:7711-7835`): Das ersetzt
ungültiges UTF-8 durch `?` (`:7768-7777`), und ein Legacy-Formular, das einen ISO-8859-1-Wert mit
`?` zurückbekäme, schriebe ihn beim nächsten Speichern so ins Gerät (Problem 5). Die neue Funktion
dient für Attribut- **und** Textinhalt; beide Kontexte sind mit diesen fünf Ersetzungen sicher.

### 7.2 Bestand

Der `esp-developer` listet **jede** Stelle in den Legacy-Seitenfunktionen, die einen nicht
konstanten Wert über `http_send` ausgibt (mindestens `input_field()` `:2687`, die Updateblöcke
`:7463`, `:7481`; dazu Tabellenzeilen mit Wertinhalt, `table_row()` u. a.), und ordnet jede einer
von drei Gruppen zu: **maskiert** (gespeicherter oder fremder Wert), **Zahl** (aus `sprintf` mit
`%d`), **Konstante**. Die statische Prüfung (AKX.2) liest diese Liste als Erlaubnisliste.

### 7.3 Release Notes (Ent-2 = R2)

Im Lesezweig `:7391-7411` ein Zustand „in einem Tag": Zeichen zwischen `<` und `>` entfallen, der
Zustand überdauert Zeilenwechsel (ein über zwei Zeilen geteiltes Tag); ein einzelnes `>` ausserhalb
eines Tags wird `&gt;`; alle anderen Zeichen — auch `&` mit Entität — bleiben. Jede Zeile endet
mit `<br>`. Damit ist der Inhalt reiner Text im Elementkontext. **Der 128-Byte-Zeilenpuffer
(`:7387`) bleibt**; eine längere Zeile wird wie heute an der Puffergrenze umbrochen — mit dem
Zustand ist das unschädlich.

---

## 8. E und P — Regeln für Datumsformat und Koordinaten

### 8.1 Datumsformat (Ent-10)

```
gültig  ⇔  1 ≤ Länge ≤ 5
         ∧ jedes Zeichen ∈ { d D m M y Y . - / Leerzeichen }
         ∧ mindestens ein Platzhalter
         ∧ höchstens ein Tag-Platzhalter (d|D), ein Monat (m|M), ein Jahr (y|Y)
         ∧ erstes und letztes Zeichen kein Leerzeichen
```

Beispiele: `D.M.Y` ✓, `d-m-Y` ✓, `M/Y` ✓, `D` ✓; `yyyyy` ✗, `<scri` ✗, `⏰` ✗, `  ab` ✗,
`D.M.` ✓ (abschliessender Punkt ist kein Leerzeichen). Leer bleibt wie heute abgewiesen
(`http.cpp:8545-8554`).

### 8.2 Koordinaten (Ent-12)

```
vorher: ',' → '.'                                    (wie heute, :8713-8714)
gültig  ⇔  Länge ≤ 8
         ∧ Form  -?[0-9]+(\.[0-9]+)?                   (kein '+', kein Exponent, keine Leerzeichen)
         ∧ |Wert| ≤ 180 (Länge) bzw. ≤ 90 (Breite)
leer: wie heute (Paar; leer nur mit Ort, :8693-8711)
```

Die PWA erzeugt aus der Karte höchstens acht Zeichen (`formatWeatherCoordinate()`,
`app.js:8677-8695`) — diese Werte sind nach der Regel gültig; der Prüfstand enthält Beispiele
daraus.

### 8.3 Eine Fall-Tabelle für beide Seiten

Der ESP prüft in C, die PWA in JavaScript; Code teilen geht nicht. Gleichheit wird deshalb
**gemessen**: Eine Tabelle (`tools/checks/…/regeln.tsv` o. ä.: Feld, Eingabe, erwartet) läuft
gegen den Auszug der ESP-Funktion (t16) **und** gegen die PWA-Funktion (Node). Weicht eine Seite
ab, meldet der Prüfstand den Fall beim Namen. Die Tabelle ist die **eine** Quelle der Regel; eine
spätere Änderung beginnt dort.

**Schreibwege** — jeder ruft dieselbe Prüfung:

| Seite | Weg | Fundstelle |
|---|---|---|
| ESP | API Datumsformat, Legacy `savedtf` | `http.cpp:8540-8566`, `:4460-4466` |
| ESP | API Koordinaten, Legacy `savelonlat` | `:8680-8722`, `:4002-4013` |
| PWA | Speichern Datumsformat, Koordinaten | `app.js:8341-8349`, `:8404-8418` |
| PWA | Kartenauswahl | `:8960-8985` |
| PWA | Backup-Import | `:6501`, `:6555-6582` |

---

## 9. P — B44 bis B47

### 9.1 B44 (Ent-9)

`parseTimeInput()` (`app.js:12631-12637`) liefert für leere oder unvollständige Eingaben **kein**
Ergebnis (z. B. `null`) statt `00:00`. `saveTimerRow()` (`:13218-13236`) und
`saveDfplayerAlarm()` (`:12996-13011`) brechen dann mit einer Meldung ab, die die Zeile nennt.
`saveAllTimerRows()` (`:13252-13274`) **prüft zuerst alle Zeilen** und sendet nichts, wenn eine
ungültig ist — sonst stünde das Gerät halb gespeichert da. Weitere Aufrufer von
`parseTimeInput()` prüft der `pwa-developer` und nennt sie im Bericht.

### 9.2 B45

`markEditsPersisted()` (`:2176-2179`) leert heute die ganze Menge. Künftig entfernt ein
erfolgreiches Speichern **nur die Felder, die es gesendet hat** (bzw. die im Bereich des
Speicherknopfs liegen); das anschliessende Neuladen entfernt über `prefillDeviceValue()`
(`:3603-3605`) zusätzlich jedes Feld, das jetzt dem Gerätewert entspricht. Wie der Bereich
bestimmt wird, entscheidet der `pwa-developer` und begründet es; **beide** Fälle aus AKB.2 sind die
Abnahme — vor allem der Gegenfall L33: Bleibt nach dem Speichern nichts Ungespeichertes übrig,
muss `hasUnsavedEdits` falsch sein, sonst ruht die Selbstaktualisierung für den Rest der Sitzung.

### 9.3 B46

`eeprom_settings` wird zusätzlich geladen, wenn das **Netzwerkmodul** aktiv ist
(`networkActive`, `app.js:2891`), über denselben Abruf wie in der Wartung (`:2984`). Der Endpunkt
liefert den WLAN-Schlüssel im Klartext (`http.cpp:8976-8996`); er wird bereits heute in der
Wartung geladen. Der Code-Review prüft, dass er an **keiner** neuen Stelle angezeigt oder
protokolliert wird (AKB.3).

### 9.4 B47

Die Ursache: Schliessen bei **`mousedown`** (`:8583`), danach setzt die Standardaktion des Klicks
den Fokus auf den Body. Zwei Wege, der `pwa-developer` wählt und der `ui-reviewer` prüft:
(a) bei `mousedown` auf dem Hintergrund `preventDefault()` und dann schliessen; (b) Schliessen erst
bei `click`, **nur** wenn auch `mousedown` auf dem Hintergrund begann — sonst schlösse ein Ziehen
aus der Karte heraus das Modal (Kartenverschieben endet oft ausserhalb). Markup ist nach heutigem
Stand **nicht** nötig; braucht es doch welches, geht es an den `ui-developer` (P.4).

---

## 10. Gerätenachweise

**Für alle:** `./tools/watch-log.sh` läuft **vor** jedem Einspielen und während jedes Laufs
(DIR-013); Smoketest nach jedem Einspielen (DIR-009, nach dem ESP-Update samt Update-Quelle);
schreibend nur mit Freigabe (Ent-1) und mit Rückstellung. **Nie** `GET /?a`, keiner der Endpunkte
aus R5.

| Nachweis | Nach | Art | Inhalt |
|---|---|---|---|
| **G0** | S0-Flash | lesend | Uhrzeit ±2 s gegen den Mac, Datum, **Wochentag**; nächste Netzzeit ohne Sprung; ein vorhandener Timer schaltet am richtigen Tag (Mitschnitt). 60 min ohne Neustart |
| **G1** | S1-Flash | **schreibend** (LDR) + lesend | **A59:** Bereinigung nach Ent-3 (`eep a=225`, `a=227`, ESP-Kopie). **A2:** 60 min `DS18xxx temperature:` plausibel, kein `ungueltig`, `IS_UP` = 1. Der Zielwert wird im Bericht als vom Nutzer bestätigter Wert genannt |
| **G2** | S2-Flash | lesend | **A59-Persistenz** (AKL.4): nach dem Reset des Flashs `numvar[17]/[18]` = Zielwert. **A50/A55:** 60 min ohne Exception, ohne Neustart, `v` konstant |
| **G3** | ESP-OTA | **schreibend** (Testwerte) + lesend | **A55** während des OTA (AKP.4). **C47:** Wetterort-Testwert (AKX.4). **C55/C44:** Abweisungen und je ein gültiger Wert mit Rückstellung (AKR.4). Ent-11 sichtbar, falls gewählt. `install-app.sh --check` |
| **G4** | PWA-Hochladen | Durchlauf | `check-pwa.sh` (DIR-016), dann T.2 (`pwa-tester`) |

**Ein Lauf, eine Wirkung:** Zwischen zwei Einspielvorgängen liegt jeweils der Nachweis des vorigen
(R3b). Fällt ein Wetterabruf oder ein Vollabgleich in einen Nachweis, wird er getrennt berichtet.

---

## 11. Prüfstände

Gebaut vom Umsetzer im Scratchpad, abgelegt vom Lead unter `tools/checks/auszug/` mit Eintrag in
`auszug.sh` (Werkzeugvorbehalt: `firmware-analyst` und `code-reviewer` haben kein `Bash`).

| Name | Ort | Gegenstand | Gegenprobe (DIR-014) |
|---|---|---|---|
| **GT** | `stm/gt/` | `my_gmtime()` alt gegen neu, Referenz Host-`gmtime_r()` | Sabotage Jahrhundertregel ⇒ FEHL |
| **Map** | Lead, statisch | `__udivmoddi4`, `__aeabi_ldivmod` nicht in der F103-Map | Ausgangs-Map ⇒ schlägt an |
| **L59** | `stm/l59/` | LDR-Zweige, EEPROM-Attrappe | `release/3.2.25-3.2.30-1.4.94` ⇒ FEHL |
| **DS** | `stm/ds/` | CRC-8, ROM-ID, 255, Neuerkennung, simulierte Zeit | alter Stand ⇒ FEHL (gekipptes Bit angenommen, nie erkannt) |
| **FR** | `stm/fr/` | Weg zum Reset ohne ISR; Zählerstand | alter Stand hängt bei vollem Ring ⇒ FEHL |
| **EP** | `stm/ep/` | Zeilenparser `CAP`/`SYNCVARS`/`IPADDRESS` und die periodischen Sendungen, simulierte Zeit | alter Stand ⇒ mehr als eine Zeitüberschreitung ⇒ FEHL |
| **W2** | besteht | A2-Wetterwarten | muss mit 37 Fällen **weiter bestehen** |
| **t15** | `esp/t15/` | Attributmaskierung, Rundlauf, Release Notes | alter Stand reicht `"` und `<script>` roh ⇒ FEHL |
| **t16** | `esp/t16/` | Regeln C55/C44 gegen die Fall-Tabelle | alter Stand nimmt `<"&`, `<scri` an ⇒ FEHL |
| **Regelgleichheit** | `pwa/regeln.mjs` o. ä. | dieselbe Fall-Tabelle gegen die PWA-Funktionen | alte PWA nimmt `<"&` an ⇒ FEHL; absichtlich abweichende Regel ⇒ Meldung kommt an |
| **statisch AKX.2** | Lead | kein nicht-literales `http_send (` ausserhalb der Erlaubnisliste | Ausgangsstand ⇒ schlägt an |
| **statisch AKF.1** | Lead | kein `log_` in `fault_reset()` | Ausgangsstand ⇒ schlägt an |

**Zähl nach** (DIR-014): Jeder Bericht nennt die Fallzahl, und der Lead vergleicht sie mit der
Zahl, die es nach dem Kriterium geben müsste.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Schritt |
|---|---|---|---|
| `src/base/base.c` (ggf. `base.h`, `src/timeserver/timeserver.c`) | `my_gmtime()` intern `uint32` | `stm-developer` | S0 |
| `src/main.c` | A59-Zweige `:1876-1886`; A2 Neuerkennung `:4348-4376`, Logzeile `:4365`; A50 `fault_reset()` `:590-613` | `stm-developer` | S1, S2 |
| `src/ds18xx/ds18xx.c` | CRC-8, Scratchpad- und ROM-ID-Prüfung, `is_up` erst danach | `stm-developer` | S1 |
| `src/tempsensor/tempsensor.c` | `gtemp.index = 255` bei Fehler; Zähler ungültiger Messungen (Ent-6) | `stm-developer` | S1 |
| `src/vars/vars.c`, `src/vars/vars.h` | `var_send_ds18xx_is_up()` öffentlich | `stm-developer` | S1 |
| `src/ldr/ldr.c`, `ldr.h` | nur falls die Grenze 4095 oder eine Setzfunktion sichtbar werden muss | `stm-developer` | S1 |
| `src/esp8266/esp8266.c` | `CAP`-Zweig `:543-553`: `is_online = 0` | `stm-developer` | S2 |
| `ESP8266/ESP-uclock/http.cpp` | Maskierung, Bestand, Release Notes (Ent-2); Regeln C55/C44 an API und Legacy; Legacy-Anzeige 255 (Ent-11) | `esp-developer` | E |
| `ESP8266/ESP-uclock/data/app/app.js`, `i18n/en.json` | B44–B47; Regeln C55/C44 an allen Wegen; DS18xx-Zustände | `pwa-developer` | P |
| `ESP8266/ESP-uclock/data/app/index.html`, `styles.css` | nur falls B47 Markup braucht | `ui-developer` | P |
| `tools/checks/**`, `tools/watch-log.sh` | Prüfstände, statische Prüfungen, Wache | Lead | alle |
| `TESTPLAN-PWA.md` | S25/S26 rücknehmbar, S27-Soll mit `eep`-Zeile, neue Fälle (T.1 in `tasks.md`) | `doc-writer` | T |
| `BEFUNDE.md` | A59, A2 (L18 neu gelesen), A50, A55, C47, C55, C44, B44–B47; neue Befunde aus Reviews | `doc-writer` | Z |
| `CLAUDE.md` | „Offene technische Themen" 1 (DS18xx), Zeilenangabe `temp_init()` | Lead | Z |
| `tools/logger/README.md` | `:352` Fault-Ausgabe | Lead | Z |
| Versionsdateien | STM (S0, S1, S2), ESP (E), `APP_VERSION` + `CACHE_NAME` (P) | `release-engineer` | je Schritt |

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015). `http.cpp` ist UTF-8; die
`src/**`-Dateien sind ASCII oder ISO-8859-1 — **nachsehen, nicht annehmen**.

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**PWA parallel zu Legacy?** Ja. Die Legacy-Seite wird **sicherer**, nicht abgebaut; ihre
Speicherwege bekommen dieselben Regeln wie die API. Die Release Notes bleiben dort sichtbar (bei
Ent-2 = R2 als Text).

**Wetter-Endpunkte?** Unberührt; C44 prüft nur, **was** als Koordinate gespeichert wird. Kein
Rückbau auf `/weather?action=…`.

**Restore-Bedingung um `pending_ticker_restore`?** Nicht berührt. C55 ändert nur, welche
Datumsformate gespeichert werden; der Datumsticker läuft wie heute. A2 sendet bei ungültiger
Messung 255 statt eines alten Werts — kein Ticker, kein Overlay hängt daran. A50 und A55 berühren
keinen der vier Teilzustände.

**Nur `.gz`?** Nicht berührt; P liefert wie immer über `app-gz` und `install-app.sh`.

**Richtige Schicht?** A59 im STM, wo der Wert verworfen wurde — nicht in der PWA umgangen. A2 im
Treiber, wo die Bytes ankommen. A50 im Fehlerpfad. A55 im STM, wo die Sendungen entstehen, mit dem
Lebenszeichen des ESP. C47 an der Ausgabe, nicht an der Eingabe (der Wert darf ein `"` enthalten,
er darf es nur nicht roh ausgeben). C55/C44: Regel am **Eingang** des Geräts (ESP), die PWA prüft
vor dem Senden nur, damit der Nutzer die Meldung am Feld sieht.

### Scalable systems

**STM-Kommandos?** A59: ein Setter wie heute, jetzt mit einer `eep`-Zeile. A2: höchstens ein
`DS18XX_IS_UP` je Zustandswechsel. A55: **weniger** Sendungen während eines ESP-Neustarts.
Sonst keine neuen.

**Byte je Minute?** A2: je Minute wie heute eine Temperaturzeile; Zeilen `ds18xx …` nur bei
Wechsel. A59: rund 45 Byte `eep`-Zeile je Setter. A55: keine neue Zeile. E und P: keine
UART-Last.

**Unter 20 s Watchdog?** Ja. A2-Neuerkennung 20–40 ms je Minute. A59: ein EEPROM-Schreibzyklus.
A50: Wartezeit **bewusst unter 20 s**; läuft der Watchdog, kommt er höchstens zuvor. A55 verkürzt
Wartezeiten.

**Wie lange blockiert der Hauptloop?** A2 bis rund 40 ms einmal je Minute (nur während kein
Sensor erkannt ist oder nach ungültigen Messungen; sonst wie heute die Messung selbst). A59 rund
25–65 ms je Setter. A55 senkt die Blockade während eines ESP-Neustarts von rund 12 s (4 × 3 s) auf
höchstens 3 s.

**Hartkodierte Grenzen?** 4095 für die LDR-Grenzen (einmal im STM, einmal im ESP
`HTTP_LDR_MAX_ADC_VALUE` — zwei Laufzeiten, nicht teilbar, Kommentar an beiden); die Wartezeit in
A50 (rund 15 s, hängt am Watchdog-Timeout 20 s und an der ESP-Bootzeit rund 10 s — Kommentar);
`k` = 3 in A2; Datumsformat 5 Zeichen, Koordinaten 8 Zeichen (`vars.h:191-195`); die Fall-Tabelle
ist die eine Quelle der Regeln. `my_gmtime()` gilt bis 2106-02-07.

### Secure by design

**`innerHTML`?** P fügt keine neue `innerHTML`-Stelle hinzu; Meldungen über `textContent`.
Guardrail `innerhtml.mjs` läuft mit.

**Fremddaten?** **C47 ist genau dieser Punkt:** Gespeicherte Werte (aus dem LAN setzbar) und
Antworten des Update-Servers (HTTP, Host konfigurierbar) gelten als nicht vertrauenswürdig und
werden maskiert bzw. bei den Release Notes auf Text reduziert. Das DS18xx-Scratchpad und die
ROM-ID gelten ab A2 als nicht vertrauenswürdig (CRC).

**Credentials in Logausgaben?** Keine neue Logzeile enthält Werte. **B46** lädt `eeprom_settings`
mit dem WLAN-Schlüssel im Klartext nun auch im Netzwerkmodul — der Schlüssel darf in keinem neuen
Feld, keiner Konsolenausgabe, keinem Log erscheinen (AKB.3, Review). Dass der Endpunkt den
Schlüssel überhaupt liefert, ist vorbestehend und „Nicht Teil".

**Neue schreibende Endpunkte?** Keine. A59 macht einen bestehenden Setter **wirksam** — der
Endpunkt ist seit jeher da und vom ESP auf 0..4095 begrenzt; die Wirkung (EEPROM) ist dieselbe
wie bei „Messwert übernehmen".

### Stable & reliable

**Fehler ausgewertet?** A2: ein Lesefehler wird **255**, nicht der alte Wert. A59: ein Wert über
4095 wird nicht angewandt. C55/C44: Abweisung mit Fehlercode und Meldung, nicht still gekürzt.
B44: leeres Feld ⇒ Meldung statt 00:00.

**Leere `catch`?** P fügt keine hinzu; Review P.6.

**Stille Verwerfungen?** A2 ersetzt einen still veralteten Wert durch einen sichtbar ungültigen.
A55: Während `is_online = 0` sendet der STM die periodischen Werte nicht — **gedeckt** durch den
Vollabgleich nach `SYNCVARS`, das ist dieselbe Mechanik wie heute nach jedem ESP-Start (Weg B).

**Still zurechtgebogen?** B44 genau dieser Fall (leer → 00:00) — beseitigt. C44: das Komma wird
wie heute zu einem Punkt; das ist eine dokumentierte Normalisierung, keine Kürzung. Keine Regel
kürzt einen Wert.

**Zustand nach Abbruch?** `saveAllTimerRows()` sendet bei einer ungültigen Zeile **nichts** statt
eines halben Satzes. A59: ein abgebrochener Backup-Import hinterlässt den bis dahin gesetzten
Wert — wie beim ESP. A50: Reset nach fester Wartezeit, auch vor dem Watchdog.

**Flags aufgelöst?** `is_online` (A55): gesetzt durch `IPADDRESS`/`SYNCVARS` wie heute; das Review
klärt, ob ein Zeilenverlust einen Dauerzustand erzeugt (§6, AKP.2). `is_up` (A2): auf jedem Pfad
neu bestimmt — Erkennung je Minute, Verlust nach `k` ungültigen. B45: `unsavedEditFields` wird
nur noch für die gespeicherten Felder geleert; der Gegenfall L33 ist eigens Abnahme.
`modalReturnFocus` (B47) wird beim Schliessen aufgelöst wie heute.

---

## Verworfene Alternativen

**S0 nicht machen und mit A50 Platz schaffen.** Hätte die Reihenfolge des Nutzers umgedreht
(A50 vor A59/A2) und den Spielraum danach knapp gelassen (§1.2). **`atoi`-Ersatz.** 450 Byte, aber
Verhaltensänderung bei Vorzeichen, Leerzeichen, Überlauf (V). **C42 als Sparmassnahme.** Spart
nichts (V).

**A59: verdrehte Reihenfolge im STM abweisen.** Bräche den Backup-Import, der Minimum und Maximum
nacheinander setzt (`app.js:6536-6538`); der ESP lässt sie aus demselben Grund zu (L68). **A59:
Wert an den ESP zurücksenden.** Unnötig, der ESP hat ihn vor dem Senden gesetzt.

**A2: neues numvar „Messwert gültig".** Protokolländerung auf beiden Seiten; 255 trägt dieselbe
Aussage, und die PWA versteht sie schon (V). **A2: Neuerkennung nur bei `is_up == 0` ohne
ROM-CRC.** Hätte L18 nicht behoben — dort war `is_up` gerade 1 (V). **A2: `SKIP ROM`.** Ent-13,
Empfehlung nein. **A2: CRC-Tabelle.** 256 Byte Flash für eine Messung je Minute.

**A50: Ausgabe pollend ins Datenregister.** Fügt eine Diagnose hinzu, die es nie gab, und kostet
Flash, statt welchen zu sparen; nicht beauftragt. **A50: Ursache im RAM für den nächsten Start
merken.** Braucht einen nicht initialisierten Speicherbereich im Linkerskript — eigenes Vorhaben.
**A50: sofort zurücksetzen (F1).** Reset-Takt 3–4 s, ESP nie erreichbar (V).

**A55: P2 (Pause nach der ersten Zeitüberschreitung, mit Höchstfrist).** Neuer Zustand und eine
Sonderregel gegen Dauerpause bei toter Brücke — mehr Code für dasselbe Ergebnis. **A55: P1 (ESP
kündigt den Neustart an).** Braucht ein ESP-Release **vor** S2 und deckt Abstürze nicht.

**C47: `HTTP_ESCAPE_XML` wiederverwenden.** Ersetzt ungültiges UTF-8 durch `?` — Wertschaden beim
erneuten Speichern (Problem 5). **C47: Werte beim Speichern filtern.** Falsche Schicht; `"` ist
in einem Tickertext legitim. **Release Notes R3 (Erlaubnisliste in C).** Fehleranfällig, und die
PWA hat sie bereits.

**C55/C44: Regel nur in der PWA.** Legacy und direkte API-Aufrufe blieben offen. **Regel nur im
ESP.** Der Nutzer sähe die Meldung erst nach dem Senden und ohne Feldbezug. **Gemeinsamer Code.**
Zwei Sprachen; ersetzt durch die gemessene Gleichheit (§8.3).

**B44: leer als „Timer aus" deuten.** Wieder eine stille Deutung (Gattung L81). **B45: gar nicht
mehr leeren.** Bräche L33 (Selbstaktualisierung ruhte für immer). **B47: Schliessen nur bei
`click`.** Ein Ziehen aus der Karte auf den Hintergrund schlösse das Modal.

---

## Versionsfolgen

Je Einspielschritt **nur die geänderte Komponente** (DIR-004). Testbauten heben keine Version an.

| Schritt | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` |
|---|---|---|---|---|
| S0 (`my_gmtime`) | ☐ anheben | — | — | — |
| S1 (A59, A2) | ☐ anheben | — | — | — |
| S2 (A50, A55) | ☐ anheben | — | — | — |
| E (C47, C55, C44, Ent-11) | — | ☐ anheben | — | — |
| P (B44–B47, Regeln, A2-Anzeige) | — | — | ☐ anheben | ☐ anheben |

Nur der `release-engineer` führt das aus (R4). Der Tag `release/<stm>-<esp>-<app>` wird **je
Einspielschritt** gesetzt und gepusht (DIR-011).
