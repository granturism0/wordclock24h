# Tasks — Portierung auf einen einzelnen ESP32-S3 (Variante B)

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst bei
Exit 0 geht die Schreibberechtigung weiter. **Ohne Freigabe der Spec durch den Nutzer (Task 0.6) beginnt keine
Implementierung** — die Analysen 0.1a–c, 0.4 und 0.5 lesen nur und dürfen vorher laufen.

## Was hier anders ist als in den bisherigen Specs

**1. Es gibt keinen neuen Besitzer ohne Vorarbeit.** Der Software-Baum für den S3 (Arbeitsname `esp32s3/`) und die Rolle dafür (Arbeitsname `s3-developer`)
**existieren noch nicht**. Wie der Baum heisst, wo er liegt und wer ihn besitzt, schlägt `sw-architect` in `specs/multiboard-quelle/` vor; **die Rolle legt allein der Lead an**.
Der Besitz-Hook kennt das Verzeichnis nicht (✔ `tools/hooks/file-ownership.py`: das Wurzelmuster in Zeile 98 nennt `src/`, `ESP8266/ESP-uclock/`, `tools/`, `.claude/`,
`knowledge/`, `specs/`, `cmake/`, `Makefile`, `CMakeLists.txt` und `*.md`). Task 0.3 legt beides an. **Bis dahin schreibt niemand in den S3-Baum** — auch nicht „ausnahmsweise", auch nicht der Lead (R3).
**`hardware/**` hat bereits eigene Besitzer** (`hw-architect`, `hw-datasheet-analyst`, `hw-designer`, `hw-reviewer`); der Software-Baum ist **nicht** `hardware/esp32s3/`.

**2. Die Reihenfolge ist Entscheidung des Nutzers (E7) und hängt an Toren.** Jede Stufe endet mit einem **Tor**: Der
Nutzer erklärt sie für bestanden, **erst dann** beginnt die nächste. Die Abhängigkeitsspalte nennt das Tor ausdrücklich.
**R3b gilt:** Dass zwei Tasks verschiedene Dateien berühren, heisst nicht, dass sie zusammen laufen dürfen. Zwischen
Stufe 2 und Stufe 3 steht ein Gerätenachweis, und der soll **die LED-Ausgabe isoliert** zeigen. Wer Stufe 3 vorher
beginnt, macht die Zusage „geht sie daneben, ist die Ursache eindeutig" zunichte — genau das ist am 03.10.2026 mit
dem Beobachtbarkeits-Paket passiert.

**3. Mehrere Tasks gehören demselben Agenten und laufen streng seriell.** Alle Tasks des `s3-developer` laufen nie
gleichzeitig, auch nicht bei disjunkten Dateien. Dasselbe für `esp-developer` und `stm-developer`.

**4. Drei Releases, nicht eines.** Der Port ändert den STM-Zweig (Task 5.1), den ESP8266-Zweig (Tasks 5.3a–c) **und**
bringt ein neues Abbild. Jedes ausgerollte Release wird sofort committet, getaggt **und gepusht** (DIR-011) — nicht
gesammelt am Ende. Zeilen `5.1r` und `5.3r` führen die beiden Releases der alten Ziele; Task 6.3 das des S3.

**5. Hardware.** Es gibt zwei Geräte: die **alte Uhr** (Test-Uhr im Keller, STM+ESP8266) und das **DevKit-Testgerät**.
Beide sind exklusiv (R5): ein Verursacher je Messung, der Lead kündigt Beginn und Ende an. **Der Nutzer flasht und löst
Backup-Import und Reset aus**; Agenten tun das nie. Die Nachweise mit **schreibenden** Handlungen am Gerät stehen als
fertige Zeile in der Spalte „Nachweis", nicht als Aufforderung zum Selbersuchen.

**6. Das Bauwerkzeug ist heute nicht gesperrt.** `tools/hooks/no-build.py` weist `make`, `cmake` und `arduino-cli` ab; ein anderes Bauwerkzeug des gewählten Frameworks (zum Beispiel `idf.py`)
nicht (✔ CLAUDE.md nennt nur diese drei). Task 0.3 ergänzt es — **bevor** der `s3-developer` die erste Zeile schreibt.

**7. „Abnahme am Gerät" bei Papier-Tasks.** Die Tasks der Stufe 0 und der Gesamtreview haben **kein** Gerät. Ihre Abnahme
ist eine **prüfbare Papierabnahme** (Spalte so benannt), keine Behauptung, es gäbe einen Gerätetest.

**8. Die Hardware-Spezifikation ist nicht Teil dieser Tasks.** Schaltplan, Layout und Bestellung liegen bei den Hardware-Rollen unter `hardware/`. Die Tasks 0.1a–c sind **Zulieferungen** an diese Spec;
die Stufen 2 bis 5 laufen am **DevKit-Testaufbau** (F8) und **warten nicht** auf den Nachbau; der Nachbau erscheint erst in Task 6.6.

---

## Stufe 0 — Klärung (Papier, kein Gerät)

| # | Task | Agent | Schreibt in (Besitz) | Hängt ab von | Abnahme | Guardrails | Review |
|---|---|---|---|---|---|---|---|
| 0.1a | **Komponentenblätter ergänzen.** Vorhanden: `esp32s3-wroom-1.md`, `sy8301abc.md`, `tps2121.md`. **Offen:** (1) USB-C **6P gegen 16P**, Aussenmasse aus den Datenblättern (**F16**); (2) 74AHCT1G125 mit **3,3-V-Eingang** bei 5 V Versorgung (**F18**); (3) SKC6812-Zeiten und Toleranz (**F9**, Datenblatt liegt nicht unter `datenblaetter/`); (4) Errata **RMT-176** (Einzelbeschreibung); (5) GL5528 mit **Messwerten** statt der Annahmen 1 MΩ / 20 kΩ / 10 kΩ; (6) zulässige **Quellimpedanz** des ADC, soweit herauszufinden (**F15**). Alles mit Datenblattseite, ✔/●/○ wie bisher | `hw-datasheet-analyst` | `hardware/komponenten/**` | — | **Papier:** Je Punkt (1)–(6) ein Blatt-Abschnitt mit Seitenzahl, oder „nicht angegeben" mit Suchbegriff | — | ☐ |
| 0.1b | **Pinzuordnung entscheiden und Fragen beantworten.** Vorschlag aus `requirements.md` gegen die Komponentenblätter; Ergebnis nach `hardware/architektur/pinbelegung.md`. **F2** (GPIO6 oder GPIO21 für `WS2812_POWER`, `R20` im Nachbau), **F3** (Schiene der Pull-ups `R8`/`R9`, Speisung von TSOP und DCF77), **F7** (Pull-ups), **F15** (`R22`/Dämpfung — **nur den Rahmen**, die Messreihe AK17 entscheidet), **F17** (Ort des BOOT-Tasters im unveränderten Format), Pegel/Strom-Budget (**F1**) | `hw-architect` | `hardware/architektur/**` | 0.1a (soweit es die Frage berührt) | **Papier:** `pinbelegung.md` führt **11** Signale mit GPIO, Netz und Begründung; F1–F3, F7, F15, F17 stehen mit „entschieden, Grund …" oder „offen, weil …" | — | ☐ |
| 0.1c | **Gutachten Pin-Tabelle.** Pin-Tabelle (`design.md` §2) und `pinbelegung.md` gegen den **Schaltplan des Nachbaus** (sobald `hw-designer` einen Stand liefert) und gegen den DevKit-Schaltplan; Netzliste als **Mengen von `Ref.Pin`**, nennt die Zahl der betrachteten Netze. **Liegt kein Schaltplan-Stand vor, bleibt der Task offen und AK1 unbestanden** — er wird nicht „aus dem Vorschlag" abgehakt | `hw-reviewer` | `hardware/review/**` | 0.1b, Schaltplan-Stand `hw-designer` (ausserhalb dieser Spec) | **Papier:** Gutachten nennt je Zeile der Pin-Tabelle „Netz X, Header-Pin Y, abgeglichen" und die **Zahl der Netze** (AK1) | — | — |
| 0.3 | **Besitz, Hooks, Werkzeug.** Rolle für den S3-Baum anlegen (Besitz **nur** dort, **ohne** Schreibrecht auf `src/**`, `ESP8266/**`, `tools/**`, `hardware/**`) — **Name und Verzeichnis nach `multiboard-quelle`**; `file-ownership.py` um das Verzeichnis ergänzen **und** in das Wurzelmuster aufnehmen; `no-build.py` um das Bauwerkzeug des gewählten Frameworks ergänzen; **Pinprüfung** (liest die **eine** Pinquelle, zählt **11**, weist Ausschlussliste E2 ab); **Symbolprüfung** (`nm`, meldet Doppelungen **und** Querverweise zwischen STM- und Netz-Komponente, nennt die **Zahl der Symbole**; AK23a); **Unverändert-Prüfung** (`git diff` über die Dateiliste aus 0.4(k), nennt die **Zahl der Dateien**; AK35); `tools/device.conf`-Vorlage um ein zweites Gerät erweitern | Lead | `.claude/**`, `tools/**` | `multiboard-quelle` freigegeben (Verzeichnis, Besitzer), 0.4(k) | **Papier plus Gegenprobe (DIR-014):** Jede neue Prüfung wird **einmal absichtlich verletzt** und schlägt an, **die Meldung kommt beim Lead an** (nicht nur ein Exit-Code). Der Hook weist einen Schreibversuch des `esp-developer` auf den S3-Baum ab. Pinprüfung meldet „11 Signale", Unverändert-Prüfung die Dateizahl aus 0.4(k) | ☐ | ☐ |
| 0.4 | **Inventur der STM-Seite (nur lesen).** (a) Je Kopfdatei der Hardware-Treiber die genaue Schnittstelle (Funktionsnamen, Rückgabewerte, Aufrufstellen); (b) **Liste aller Schreibstellen** auf die Display-Zustandsmaschine (Grundlage der Namensliste für AK23); (c) **Ist-Stand der Quittungsprüfung** je Kommando in `vars.c` (Spec `specs/bruecke/` ist eine Momentaufnahme); (d) Nahtstellen in `main.c` (83 Treffer) mit Zeilennummern; (e) welche Funktionen der Zeitgeber-Rumpf ruft, und ob sie Flash-Rodata lesen (`dcf77_tick`, `irmp_ISR`); (f) wo `F_INTERRUPTS` steht und was an ihm hängt (F14); (g) Layout-Dateien des EEPROM (AK13); (h) **Funktionsinventar** (E8): jede für einen Nutzer sichtbare Funktion mit Fundstelle; (i) Zahl der `watchdog_reload ()`-Aufrufstellen und `WD_EXPECTED` heute; (j) Entprellung der Taster: Software oder Hardware; **(k) die verbindliche Liste: je `.c`-Datei unter `src/` „hardwarefrei" oder „hardwarenah", mit Beleg** — **der Auftrag zählt 20 hardwarenahe, meine Zählung trifft 15 von 25** (`design.md` §3.6); (l) die Kopfdateien `rtc.h`, `eeprom.h`, `ldr.h`, `ds18xx.h`, `tempsensor.h`: **was holen sie aus den STM-Kopfdateien**, wenn `STM32F10X`/`STM32F4XX` definiert sind, und **ob die Module ohne beide Definitionen übersetzen können** (beantwortet die Übersetzungsprobe aus 1.1 vorab) | `firmware-analyst` | — (Bericht; Eintrag durch 0.2) | — | **Papier:** jeder Punkt (a)–(l) hat eine Antwort mit Datei:Zeile; die Zahlen werden **nachgezählt**, nicht aus dieser Spec übernommen; (k) nennt **eine** Zahl hardwarenaher Dateien und erklärt die Differenz zu 20 und zu 15 | — | ☐ |
| 0.5 | **Inventur der ESP-Seite (nur lesen).** (a) Alle ESP8266-spezifischen Stellen (Muster aus `design.md` §5), je Stelle ein Ersatzvorschlag; (b) jede der 495 `Serial.`/`debugmsg`/`statusmsg`-Stellen als **Brücke**, **Debug** oder **Status** sortiert; (c) alle Stellen, die den STM flashen oder zurücksetzen (72 Treffer); (d) die `WiFiClient`-Leseschleifen (L173, L353, L356/C53) mit dem, was am ESP32 anders ist; (e) Speicherort der ESP-eigenen Variablen; (f) **Nutzung von Zufallszahlen** (✔K: der ADC speist den Zufallszahlengenerator, Lesen mindert seine Zufälligkeit) | `firmware-analyst` | — (Bericht; Eintrag durch 0.2) | — | **Papier:** (b) ergibt drei Teilmengen, deren Summe **495** ist (oder die Abweichung ist erklärt) | — | ☐ |
| 0.2 | **Ergebnisse eintragen** in `design.md` §11; Spec nachziehen, wo ein Bericht eine Annahme widerlegt (und die Widerlegung **ausdrücklich nennen**) | `spec-writer` | `specs/esp32s3-port/**` | 0.1a–c (soweit vorhanden), 0.4, 0.5 | **Papier:** Die Tabelle §11 enthält **keine** Zeile „offen" ausser mit Begründung | — | ☐ |
| 0.6 | **Freigabe der Spec** und Entscheidung der Fragen **F6, F8, F10, F11, F13** (und F14, falls 0.4 ihn nicht beantwortet; F12 entscheidet `multiboard-quelle`). **F8** (Testaufbau) ist Voraussetzung für Stufe 2: ohne externen Pegelwandler und LED-Board am DevKit gibt es keinen Nachweis | Nutzer | — | 0.2, 0.3 | Entscheidung im Gespräch festgehalten und vom `spec-writer` in `requirements.md` (Status „Freigegeben am …") eingetragen | — | — |

**Tor G0:** Task 0.6 **und** `specs/multiboard-quelle/` vom Nutzer freigegeben, soweit sie Framework, Verzeichnisse, Besitzer und Board-Beschreibung festlegt (**F12**). Erst danach beginnen Stufe 1 und Task 7.5.
Zum Zeitpunkt dieser Spec liegt `specs/multiboard-quelle/` **nicht vor**; Stufe 1 beginnt nicht, solange sie fehlt.

---

## Stufe 1 — Gerüst und USB (Muss (b))

| # | Task | Agent | Schreibt in (Besitz) | Hängt ab von | Nachweis | Abnahme am Gerät | Guardrails | Review |
|---|---|---|---|---|---|---|---|---|
| 1.1 | **Gerüst.** Baum nach `multiboard-quelle`: `app_main`, Hardwareschicht (leer), Systemkonfiguration (IRAM-sichere Zeitgeber- und RMT-Interrupts, Task-Watchdog 20 s, **Konsole primär UART0, sekundär USB-Serial/JTAG** nach ✔K §9, **kein USB-Geräte-Stack**, Panic = Neustart, **Brownout-Grund lesbar**), Partitionstabelle (zwei OTA-Plätze + LittleFS, **8 MB Flash** ✔K), Pinquelle (11 Signale aus §2), `net_task` auf Kern 0 / `display_task` auf Kern 1 als **leere** Tasks, Komponente für die portablen `src/**`-Dateien (Liste aus 0.4(k)); **als allererste Anweisung `GPIO6` und `GPIO5` auf Low**; Ausgabe der Abbild-Summe und des Reset-Grunds beim Start. **Übersetzungsprobe:** Die hardwarefreien Module aus `design.md` §3.6 übersetzen **ohne** `STM32F10X`/`STM32F4XX` — **jeder Fehlschlag wird gemeldet, nicht behoben** (Ersatz-Kopfdatei oder Ausnahmeverfahren nach §3.6). Hält fest, **was die USB-Zweitkonsole zulässt** (nur Ausgabe, nur bei nicht verbundenem UART0) | `s3-developer` | S3-Baum | G0, 0.3 | — | siehe 1.4 | ☐ | ☐ |
| 1.2 | **Build-Ziel.** Ziel für den S3 (Name offen), Abbild-Summe im Bericht, **keine** Änderung an den bestehenden Zielen | `release-engineer` | `Makefile`, `CMakeLists.txt`, `cmake/**` | 1.1 | — | — | ☐ | ☐ |
| 1.3 | **Bauen.** Einmal, nur der Lead (R1). Abbild-Summe notieren; das Ergebnis der Übersetzungsprobe aus 1.1 in den Bericht | Lead | — | 1.2 | — | — | — | — |
| 1.4 | **▶ NACHWEIS N1 (Muss b).** Nutzer flasht über das **eine** USB-C-Kabel am DevKit-Anschluss „USB" und prüft: **AK3** — dreimal normaler Weg ohne Taster (`esptool`, DTR/RTS); **Rückfall 1**: Firmware hängt (Testabbild mit Endlosschleife), Flashen gelingt, zweimal; **Rückfall 2**: Testabbild konfiguriert **GPIO19/20 um**, das USB-Gerät verschwindet, **manueller Weg** (BOOT-Taster gedrückt halten, EN kurz drücken), einmal; **Rückfall 3**: danach wieder normaler Weg ohne Taster. **AK3a** — **zwei Läufe, nicht gleichzeitig:** (1) Kabel am Anschluss „UART", Konsole am Terminal; (2) Kabel **nur** am Anschluss „USB", dieselben Zeilen am Terminal. **Keine Eingabe verlangt.** **AK3b** — Debugger hält im `display_task` an, 3 s, läuft weiter, kein Reset. **AK4** — Konsole ohne Gegenstelle 10 Minuten, `diag`-Folge lückenlos. **Mitschnitte in den Bericht** | Nutzer (flasht, steckt) + Lead (misst, liest den Mitschnitt mit) | — | 1.3 | **MESSUNG — hier läuft nichts anderes am DevKit** | **AK3, AK3a, AK3b, AK4** | — | — |

**Tor G1 — Muss (b):** Task 1.4. Fällt **eines** der Kriterien durch, stoppt das Projekt hier: Ohne zuverlässiges Flashen
gibt es keine weiteren Gerätenachweise. **Kein Weiterarbeiten „auf Verdacht".**

---

## Stufe 2 — LED-Ausgabe, isoliert (nur die Treiber-API)

**Diese Stufe berührt keine Zeile aus `src/**`, vor allem keine aus `display.c` und `main.c`, und keine aus `ESP8266/**`.** Das ist die Zusage dieser Reihenfolge und von E11.
**Es entsteht der Treiber mit den sechs Funktionen von `sk6812.h` — sonst nichts.** Keine Zustandsmaschine, kein Anzeigemodul.

| # | Task | Agent | Schreibt in (Besitz) | Hängt ab von | Nachweis | Abnahme am Gerät | Guardrails | Review |
|---|---|---|---|---|---|---|---|---|
| 2.1 | **RMT-Treiber** mit **genau den sechs Funktionen** von `sk6812.h` (`sk6812_init`, `_refresh`, `_set_led`, `_set_all_leds`, `_get_refresh_cnt`, `_get_dma_wait_cnt`), gegen die **originale** `sk6812.h`; Zeiten aus `sk6812.c:91-97`; **Farbbyte-Reihenfolge am Quelltext belegen** (Grün zuerst ✔, Rest nachlesen); Doppelpufferung wie `sk6812.c:126`; beide Zähler **echt**; und ein **Testprogramm**, das die Muster aus AK6 erzeugt, Sendefehler zählt, die Pause ≥ 250 µs einhält und bis **234** LEDs sendet. DMA-Betrieb, Auflösung 10 MHz; die Wahl des DMA-Kanals und den Speicherbedarf **messen und im Kopfkommentar festhalten**; **Errata RMT-176 lesen**, bevor der DMA-Betrieb gewählt wird | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm) | **G1** | — | siehe 2.2 | ☐ | ☐ |
| 2.2 | **▶ NACHWEIS N2.** Lead baut, Nutzer flasht. **Aufbau (F8):** DevKit → externer 74AHCT125 → Original-LED-Board über `H7`-Kabel; LED-Versorgung **über Handschalter** (F2). Messungen: Logikanalysator an `H7` Pin 4, Oszilloskop an `GPIO6` bei Einschalten, Reset und Flashen **mit Dauer jedes Glitches**. **AK5** (Zeiten ±150 ns, Pause ≥ 250 µs, drei Bilder), **AK6** (114 LEDs Muster, Nutzer sieht es), **AK6a** (Übersetzung gegen die originale `sk6812.h` ohne Warnung; `git diff` auf `sk6812.h` und `display.h` leer; Zähler zeigen echte Werte), **AK7** (10 Minuten mit `ping`-Dauerlast und HTTP-Schleife, 0 Sendefehler), **AK8** (234 LEDs, Bilddauer), **AK9** (GPIO6-Ruhepegel aufgezeichnet), **AK10** | Lead (misst) + Nutzer (flasht, sieht) | — | 2.1 | **MESSUNG — hier läuft nichts anderes am DevKit** | **AK5, AK6, AK6a, AK7, AK8, AK9, AK10** | — | — |

**Tor G2:** Task 2.2. **Fällt AK9 durch** (LED-Versorgung geht beim Reset an), ist F2 beantwortet **und die Platine ändert sich** — das
Tor gilt trotzdem als bestanden, **wenn** der Nutzer die Platinenänderung beschliesst und das Ergebnis in `requirements.md` (F2) einträgt.

---

## Stufe 3 — I²C, EEPROM, RTC

| # | Task | Agent | Schreibt in (Besitz) | Hängt ab von | Nachweis | Abnahme am Gerät | Guardrails | Review |
|---|---|---|---|---|---|---|---|---|
| 3.1 | **I²C-Schicht** mit der Schnittstelle von `i2c.h`: NACK ≠ Störung, Bus-Freitakten, Frist ≤ 5 ms je Flag-Äquivalent, **keine** Wartezeit des IDF-Treibers darüber | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm) | **G2** | **Nachweis N3a** (Lead baut, Nutzer flasht): **AK14** (leere Adresse → NACK-Code, Störung unterscheidbar), **AK15** (SDA 100 ms nach Masse, Erholung) | AK14 (Teil NACK), AK15 | ☐ | ☐ |
| 3.2 | **EEPROM und RTC** aus dem **unveränderten** `src/eeprom/eeprom.c` und `src/rtc/rtc.c` in einem Testprogramm; die 1-kHz-Quelle (`eeprom_ms_tick`) stellt das Testprogramm bereit | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm) | 3.1 **samt N3a** | **Nachweis N3b** (Lead baut, Nutzer flasht, **Test-EEPROM**, nicht produktiv): **AK11** (4'096 Byte Rundlauf), **AK12** (Zeiten ≤ 20 / ≤ 45 ms), **AK13** (`git diff` leer), **AK14** (RTC: Zeit setzen, 60 s ± 1 s) | AK11, AK12, AK13, AK14 | ☐ | ☐ |

**Tor G3:** Tasks 3.1 und 3.2 samt Nachweisen. **Warum 3.2 nach N3a wartet:** Ein EEPROM-Fehler soll nicht auf einen I²C-Fehler
zurückgehen können. Die Dateien sind verschieden; die Reihenfolge ist trotzdem verbindlich (R3b).

---

## Stufe 4 — Sensoren (Reihenfolge verbindlich)

| # | Task | Agent | Schreibt in (Besitz) | Hängt ab von | Nachweis | Abnahme am Gerät | Guardrails | Review |
|---|---|---|---|---|---|---|---|---|
| 4.1 | **Zeitgeber, `delay`, Watchdog.** GPTimer mit gewähltem `F_INTERRUPTS` (Empfehlung 20 kHz, F14, **von aussen gesetzt**, kein Eingriff in `src/`), Interrupt auf Kern 1, IRAM; `delay_usec`/`delay_msec`/`delay_sec`; Watchdog-Rumpf (`esp_task_wdt_reset`); Testprogramm mit Zählern; Linker-Fragment-Gerüst. **Benötigt aus 0.4:** F14 entschieden, `dcf77_tick`-Frage beantwortet | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm) | **G3**, 0.4 | **Nachweis N4.1** (Lead baut, Nutzer flasht): **AK16** — Softuhr gegen RTC 10 Minuten ≤ 100 ms; 100 Schreibvorgänge auf LittleFS, `diag_tick_cnt` stimmt auf ± 0,1 % | **AK16** | ☐ | ☐ |
| 4.2 | **ADC und LDR (Muss a).** Oneshot ADC1 an `GPIO1`, **Dämpfung und `R22` als benannte Konstanten**, Kalibrierung ab Werk, Umrechnung auf **12 Bit / 3,3 V**, 32-fache Mittelung (Startwert), Start/Abfrage zweistufig wie `adc.c`; Testprogramm ruft das **unveränderte** `ldr.c`. **Aufbau:** Original-LED-Board über `H7`, **`R22` und 100 nF auf dem Steckbrett** (F8, F15; Startwert 47 kΩ / 12 dB, **10 kΩ / 6 dB als zweite Messreihe**) | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm) | 4.1 **samt N4.1** | **Nachweis N4.2 (Muss a, Nutzer misst mit Multimeter, Lead protokolliert):** **AK17** (**Messreihe** ≥ 8 Spannungsstufen von 0,10 V bis Bereichsende, Soll/Ist/Abweichung, Toleranz Gesamtfehler + 1 %, Stufen unter dem Fehler nicht als bestanden gezählt), **AK17a** (Teiler am LED-Board über `H7`, drei Beleuchtungen), **AK17b** (Kalibrierung **neu** gemessen, nach Reset unverändert), **AK17c** (Helligkeit monoton, Rauschen ≤ ±2 %, **auch unter WLAN-Dauerlast**, Sättigung benannt) | **AK17, AK17a, AK17b, AK17c** | ☐ | ☐ |
| 4.3 | **1-Wire und DS18xx.** Bit-Banging **oder** RMT-1-Wire — die Wahl nach einer Logikanalysator-Messung der Slot-Zeiten, im Kopfkommentar festgehalten; `ds18xx.c` **unverändert**; A63/A64/A65 **nicht** korrigieren | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm) | 4.2 **samt N4.2** | **Nachweis N4.3:** **AK18** (Referenzthermometer ±0,5 K; Sensor abziehen → `ds18xx verloren`, stecken → `ds18xx erkannt`) und `diag_tick_cnt` während der Messung ohne Lücke (der 20-kHz-Takt bleibt ungestört) | **AK18** | ☐ | ☐ |
| 4.4 | **IRMP und Fernbedienung.** `irmp.o` per Linker-Fragment in IRAM/DRAM, Pin-Zugriff über die Hardwareschicht, `remote-ir.c` **unverändert** | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm, Linker-Fragment) | 4.3 **samt N4.3** | **Nachweis N4.4:** **AK19** (fünf Tasten, Protokoll/Adresse/Befehl wie an der alten Uhr; `learn_ir`; 100 Tastendrücke unter `ping`-Dauerlauf, 0 Ausfälle) | **AK19** | ☐ | ☐ |
| 4.5 | **DCF77, Taster, Status-LED, LED-Versorgung** (`dcf77.c`-Pin, `button.c`, `wpsbutton.c`, `board-led.c`, `power.c`) | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm) | 4.4 **samt N4.4** | **Nachweis N4.5:** **AK20** (Taster-Ereignisse im Mitschnitt; DCF77 mit Signalgenerator gültig in ≤ 5 Minuten, gestörtes Muster keine Zeit) und `power_off()` ist der Anfangszustand (AK9, wiederholt) | **AK20** | ☐ | ☐ |

**Tor G4:** Tasks 4.1–4.5 samt Nachweisen. **Muss (a) ist erst bestanden, wenn AK17, AK17a, AK17b und AK17c am Gerät bestanden sind.**
**Mit der Messreihe aus 4.2 wird F15 entschieden** (`R22` und Dämpfung); der Nutzer trägt das Ergebnis in `requirements.md` ein. **Fällt AK17 durch**, wird F15 mit der anderen Paarung wiederholt oder der Wert geändert, **die Controller-Platine ändert sich, das LED-Board nicht
(E10)**, und 4.2 wiederholt sich. Das Tor öffnet **nicht** mit einer Softwarekorrektur.

---

## Stufe 5 — Brücke und Task-Aufteilung

| # | Task | Agent | Schreibt in (Besitz) | Hängt ab von | Nachweis | Abnahme am Gerät | Guardrails | Review |
|---|---|---|---|---|---|---|---|---|
| 5.1 | **Nahtstellen in `src/**` herauslösen.** Zeitgeber, Watchdog, Takt, `main()`-Plattformteil und Interrupt-Rumpf in eine eigene Plattformdatei; `F_INTERRUPTS` wählbar. **Die Zeilen von `pending_ticker_restore` bleiben bytegleich** (AK24). **`display.c` und die übrigen hardwarefreien Module bleiben unverändert** (AK35; Ausnahmen nur nach `design.md` §3.6). **Verhalten unverändert.** Zeilenende und Kodierung jeder angefassten Datei vorher feststellen; Umschrift in neuem Text (DIR-015) | `stm-developer` | `src/**` (ausser den Modulen aus 0.4(k) „hardwarefrei") | **G4**, 0.4 | siehe 5.1r | siehe 5.1r | ☐ | ☐ |
| 5.1v | **STM-Version anheben** (DIR-004) | `release-engineer` | `src/main.h` | 5.1 | — | — | ☐ | ☐ |
| 5.1r | **▶ RELEASE R-STM (alte Uhr).** Lead baut, Rollout (DIR-005, nie `--delete`), **commit, Tag, Push** (DIR-011). **Nutzer flasht den STM** der alten Uhr (`./tools/flash-stm.sh --check`, dann `./tools/flash-stm.sh`). Danach `./tools/smoke-device.sh`, **`pwa-tester` Phasen 0–4 und 9**, `diag`-Folge und Reset-Grund wie zuvor; `git diff` auf die vier `pending_ticker_restore`-Zeilen leer; **Unverändert-Prüfung aus 0.3 über die Modulliste: 0 Abweichungen, Zahl der Dateien genannt** | Lead (+ Nutzer flasht, `pwa-tester`) | — | 5.1v | **MESSUNG** an der alten Uhr; Logwache läuft (DIR-013) | **AK33** (Teil alte Uhr), **AK24** (Wortlaut), **AK35** | — | — |
| 5.2 | **Brücke (Transport).** Stream-Buffer je Richtung; `esp8266_uart_*`-Schnittstelle in der Hardwareschicht; Verlustzähler `d=`; `BRIDGE_TX_BLOCK_MS`; **Task-Kennungsprüfung (Debug-Build) an den Übergängen**; Testprogramm mit Erzeuger und Leser | `s3-developer` | S3-Baum (Hardwareschicht, Testprogramm) | **G4** | **Nachweis N5.2** (Lead baut, Nutzer flasht): Folge mit **194 `var`-Zeilen** gegen den Leser: alle Quittungen, 0 Verluste; **Rückstau** (Leser steht 2 s): jedes verworfene Zeichen gezählt, `display_task`-Δu ≤ `DIAG_INTERVAL_SEC + 2 s` (**AK22**); **absichtlich falscher Aufrufer-Task bricht im Debug-Build ab** (AK23b) | **AK22, AK23b** | ☐ | ☐ |
| 5.2b | **Netz-Komponente und `StmLink`.** Komponente für die Netz-Seite mit Dateiliste, `StmLink`-Klasse (Print/Stream über die Brücke), Konsolen-Ausgabe für Debugtexte | `s3-developer` | S3-Baum | 5.2 **samt N5.2**, 0.5 | Baut ohne Warnungen im Lead-Build; `nm`-Symbolprüfung aus 0.3 meldet **0** Doppelungen und **0** Querverweise, nennt die Zahl der Symbole (AK23a, AK34) | siehe 5.4 | ☐ | ☐ |
| 5.3a | **Netz-Seite, Teil a:** WLAN/AP, UDP, NTP, Wetter, `vars.cpp`, `eepromdata.cpp`, `tables.cpp`, `display.cpp`, `base.cpp`, `httpclient.cpp`, `.ino`-Grundgerüst als **`#ifdef ESP32`-Zweige** (ESP8266 bleibt in Verhalten unverändert; **sofern `multiboard-quelle` kein anderes Trennverfahren festlegt**). Brücken-/Debug-/Status-Sortierung aus 0.5(b) umsetzen | `esp-developer` | `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` (**nicht** `data/app/**`) | 5.2b | **Nachweis N5.3a** (Lead baut, Nutzer flasht DevKit): WLAN verbindet, AP-Rückfall greift, NTP-Zeit, UDP antwortet, `/api/weather_get_now` liefert | **AK21** (Teil) | ☐ | ☐ |
| 5.3b | **Netz-Seite, Teil b: `http.cpp`.** Ersatz der lwIP-Privatzugriffe (Stellen aus 0.5(a)); Legacy-Oberfläche und PWA-Auslieferung (`.gz` only, Grösse > 0); **`WiFiClient`-Leseschleife am ESP32 neu gemessen** mit Antworten **> 536 Byte** | `esp-developer` | `ESP8266/ESP-uclock/http.cpp` und Kopfdateien | 5.3a **samt N5.3a** | **Nachweis N5.3b:** Legacy-Oberfläche lädt, `/app/` liefert, `./tools/smoke-device.sh` (lesender Teil) grün; Release Notes und Wetter **vollständig** abgerufen (kein „Parse Error"); die Messung der `WiFiClient`-Semantik steht im Bericht | **AK21** (Teil) | ☐ | ☐ |
| 5.3c | **Netz-Seite, Teil c: Update und STM-Endpunkte.** `HTTPUpdate`/`Update`; Prüfung auf fremdes und abgeschnittenes Abbild; STM-Flash/-Reset-Endpunkte mit **definiertem Fehler** (F10); **Update-Vorgabe leer** (DIR-009) | `esp-developer` | `ESP8266/ESP-uclock/http.cpp`, `ESP-uclock.ino`, `vars.cpp` | 5.3b **samt N5.3b** | **Nachweis N5.3c:** **AK27** (Update, abgeschnittenes **und** fremdes `.bin` abgewiesen, Gerät läuft weiter), **AK28** (Antworten der STM-Endpunkte lesend geprüft) | **AK27, AK28** | ☐ | ☐ |
| 5.3v | **ESP-Version anheben** (DIR-004) | `release-engineer` | `ESP8266/ESP-uclock/version.h` | 5.3c | — | — | ☐ | ☐ |
| 5.3r | **▶ RELEASE R-ESP (alte Uhr).** Lead baut ESP8266-Ziel, Rollout, **commit, Tag, Push** (DIR-011). **Nutzer flasht den ESP** der alten Uhr (OTA). Danach `./tools/smoke-device.sh`, **Update-Quelle prüfen (DIR-009)**, `./tools/install-app.sh --check` (DIR-017), `pwa-tester` Phasen 0–4 und 9 | Lead (+ Nutzer flasht, `pwa-tester`) | — | 5.3v | **MESSUNG** an der alten Uhr; Logwache läuft | **AK33** (Teil alte Uhr), **AK21** | — | — |
| 5.4 | **Task-Aufteilung.** `app_main` legt `display_task` (Kern 1) und `net_task` (Kern 0) an, Reihenfolge nach `design.md` §4.5 (**GPIO6 zuerst**), TWDT je Task, `esp_reset_reason()` → `IWDGRST`-Wort **und eigenes Wort für Brownout**, Task-Kennungsprüfung in allen Übergängen der Hardwareschicht | `s3-developer` | S3-Baum | **5.1r, 5.3r**, 5.2b | siehe 5.6 | siehe 5.6 | ☐ | ☐ |
| 5.5 | **Werkzeuge.** `smoke-device.sh` und `watch-log.sh` gegen den S3 (Reset-Wort **und Brownout-Wort**, Heap-Feld), `guardrails.sh` S7 (`WD_EXPECTED` plus die neue Stelle im Netz-Task), `flash-s3.sh` (siehe 6.2), `deploy.sh`-Serverlayout. **Darf parallel zu 5.4 laufen** — Besitzer und Dateien verschieden, keine Abhängigkeit | Lead | `tools/**` | 5.3c | Jede **neue oder geänderte** Prüfung wurde **einmal absichtlich verletzt** und hat angeschlagen, **die Meldung ist angekommen** (DIR-014, AK34); die Pinprüfung betrachtet **11** Signale | — | ☐ | ☐ |
| 5.6 | **▶ NACHWEIS N5 (Brücke und Tasks).** Lead baut, Nutzer flasht das Gesamtabbild ins DevKit; Logwache läuft. **AK21** (mitgeschnittene Kommandofolge, Zeile für Zeile; `git diff` auf `data/app/**` leer), **AK22** (Rückstau im Gesamtsystem), **AK23** (statisch **und** Debug-Build mit absichtlicher Verletzung), **AK24** (Wetter-Ticker, Zeit läuft weiter), **AK25** (Reset-Wort in der Logwache, **absichtlich** erzeugt durch einen Testpfad im Debug-Build; Brownout-Wort vorhanden), **AK26** (zehn `ticker_set`, Ergebnis **eintragen, nicht deuten**), **AK28**; **AK37 am DevKit** (3,3-V-Minimum am 3V3-Pin des Moduls über ≥ 10 Minuten WLAN-Dauersenden, Spitzenstrom eintragen; **keine** Aussage über die Versorgung des Nachbaus, F1). **Backup-Import und Neustart löst allein der Nutzer aus** | Lead + Nutzer + `pwa-tester` | — | 5.4, 5.5 | **MESSUNG — hier läuft nichts anderes am DevKit** | **AK21–AK26, AK28, AK37 (DevKit)** | — | — |
| 5.7 | **Gesamtreview** gegen `knowledge/architecture-checklist.md`: alle vier Punkte der Checkliste, `design.md` §8 gegen den fertigen Code; `grep` über alle neuen Dateien auf Logzeilen mit Schlüsseln, Hostnamen, Pfaden; Zählung der `watchdog_reload ()`-Stellen; **Unverändert-Prüfung (AK35) über die Modulliste, mit Dateizahl** | `code-reviewer` | — (ändert nichts) | 5.6 | — | **Papier:** Bericht nennt je Punkt „erfüllt" mit Beleg oder „offen, weil …" | — | — |

**Tor G5:** Tasks 5.6 und 5.7. **Die Reihenfolge `5.1r`/`5.3r` vor `5.4` ist Absicht:** Die alten Ziele müssen mit dem gemeinsamen
Quelltext **zuerst** laufen. Läuft der gemeinsame Quelltext an der alten Uhr nicht, hat der S3 keine Vergleichsgrundlage.

---

## Stufe 6 — Release und Abnahme

| # | Task | Agent | Schreibt in (Besitz) | Hängt ab von | Nachweis | Abnahme am Gerät | Guardrails | Review |
|---|---|---|---|---|---|---|---|---|
| 6.1 | **Versionen und Release-ZIP für den S3.** Beide Symbole (`VERSION`, `ESP_VERSION`) nach Entscheidung F13; S3-Abbild im Release-ZIP | `release-engineer` | `Makefile`, `CMakeLists.txt`, `cmake/**`, `src/main.h`, `ESP8266/ESP-uclock/version.h` | G5 | — | — | ☐ | ☐ |
| 6.2 | **`tools/flash-s3.sh`** — prüft Abbild und Zielgerät, flasht über USB, **weist die gemeldete Version nach** und schlägt fehl, wenn das Gerät danach nicht antwortet (DIR-010-Lehre); `--check` prüft, dass alles bereitliegt | Lead | `tools/**` | 5.5 | Gegenprobe (DIR-014): Abbild-Datei absichtlich leer → das Skript bricht ab **mit Meldung beim Lead** | — | ☐ | ☐ |
| 6.3 | **Bauen, Release-ZIP, Rollout** (DIR-005), **commit, Tag, Push** (DIR-011) | Lead | — | 6.1, 6.2 | — | — | — | — |
| 6.4 | **▶ ERST-FLASH S3.** Nutzer: `./tools/flash-s3.sh --check`, dann `./tools/flash-s3.sh`; danach `./tools/install-app.sh --check` und `./tools/install-app.sh` — **das LittleFS des S3 ist leer**; dann `./tools/smoke-device.sh` und die **Prüfung der Update-Quelle** (DIR-009) | Nutzer | — | 6.3 | — | — | — | — |
| 6.5 | **▶ NACHWEIS N6.** **AK27** (Update über den Update-Server, fremdes Abbild abgewiesen), **AK29** (Backup alt → neu, **Nutzer löst den Import aus**; `snapshot-device.sh` an beiden, Unterschiede nur in den benannten Feldern **und** den LDR-Werten), **AK30** (`pwa-tester`, Phasen 0–4 und 9, plus Funktionsinventar aus 0.4), **AK31** (72 Stunden mit Logwache und Freispeicher-Verlauf), **AK33** (`guardrails.sh --full` Exit 0; alte Ziele bauen), **AK35** (Unverändert-Prüfung über die ganze Modulliste). **Nach dem Backup-Import wird die LDR-Kalibrierung neu gemessen (AK17b)** | Lead + Nutzer + `pwa-tester` | — | 6.4 | **MESSUNG — hier läuft nichts anderes am Gerät** | **AK27, AK29, AK30, AK31, AK33, AK35** | — | — |
| 6.6 | **▶ NACHBAU-ABNAHME.** **Dasselbe Abbild** (gleiche Summe wie AK2) auf der nachgebauten Controller-Platine; **dasselbe LED-Board** wechselt zwischen alter Uhr und Nachbau. **AK2, AK3 (inklusive Rückfall 2 mit den Tastern des Nachbaus), AK3a (nur USB), AK3b, AK4, AK9, AK17–AK17c, AK30, AK32a, AK37 am Nachbau bei 4,5 / 4,75 / 5,0 V und mit dem Netzteil des Nutzers**; **AK36** (Formatprüfung, 14 Merkmale, durch ein Prüfskript der Hardware-Rollen; Nutzer prüft den Einbau ins Gehäuse). **Voraussetzung:** Platine liegt vor (Nutzer), F1, F4, F16, F17 beantwortet | Nutzer + Lead (+ `hw-reviewer` für AK36) | — | 6.5, Platine | **MESSUNG** am Nachbau | **AK2, AK32, AK32a, AK36, AK37** | — | — |

---

## Stufe 7 — Dokumentation

| # | Task | Agent | Schreibt in (Besitz) | Hängt ab von | Abnahme | Guardrails | Review |
|---|---|---|---|---|---|---|---|
| 7.5 | **Eintrag in `ERWEITERUNGEN.md`** (Kennung `X<n>`) mit Verweis auf diese Spec. **Darf direkt nach der Freigabe laufen** — Abhängigkeit nur G0 | `doc-writer` | `ERWEITERUNGEN.md` | G0 | **Papier:** Eintrag vorhanden, Guardrail-Zählung stimmt | ☐ | ☐ |
| 7.1 | **`HARDWARE.md`**: Abschnitt „ESP32-S3" mit Pinbelegung (aus dem **abgeglichenen** §2), Teiler und `R22`, USB-Beschaltung; **die STM32-Teile bleiben** (alte Ziele). **Die Aussagen, die der Schaltplan nicht trägt (Skill `kicad-schema`), werden dabei gegen die Netzliste berichtigt** | `doc-writer` | `HARDWARE.md` | 6.6 | **Papier:** jede Pinangabe an beiden Enden belegt (Netz und Makro), wie bisher | ☐ | ☐ |
| 7.2 | **`BEFUNDE.md`, `CHANGELOG.md`**: neue Befunde der Port-Arbeit (insbesondere aus N2, N4.2 und N5), Status, Beleg; ToDo-Liste mitziehen (Guardrail S10); **keine Versionsnummern in lebender Doku** (DIR-006) | `doc-writer` | `BEFUNDE.md`, `CHANGELOG.md` | 6.5 | **Papier:** Guardrail S9 und S10 grün | ☐ | ☐ |
| 7.3 | **`knowledge/directives.md`, `CLAUDE.md`:** DIR-010 gilt **nur für die alten Ziele**, neue Direktive für den S3-Flash, Zeile der S3-Rolle in der Besitztabelle R3, Versionsstand-Tabelle um das S3-Abbild | `doc-writer` | `knowledge/**`, `CLAUDE.md` | 6.5, Entscheidung des Nutzers | **Papier:** Guardrail S11 meldet keine unbekannte Kennung | ☐ | ☐ |

---

## Parallelität — die einzigen erlaubten Fälle

| Zusammen erlaubt | Warum |
|---|---|
| 0.1a, 0.4, 0.5 (`hw-datasheet-analyst`, `firmware-analyst`, lesend beziehungsweise nur `hardware/komponenten/**`) | Nur lesen oder disjunkte Dateien; keine Abhängigkeit |
| 0.3 (Lead, `.claude/**`, `tools/**`) neben 0.1a/0.4/0.5 | Verschiedene Besitzer, keine Abhängigkeit (0.3 wartet aber auf 0.4(k) und `multiboard-quelle`) |
| 7.5 (`doc-writer`) neben Stufe 1–6 | Besitzer verschieden, Abhängigkeit nur G0 |
| 5.2 (`s3-developer`, DevKit) neben 5.1/5.1v/5.1r (`stm-developer`, `release-engineer`, Lead, **alte Uhr**) | Verschiedene Geräte, verschiedene Besitzer, **keine** Abhängigkeit dazwischen. **Nur mit Freigabe des Nutzers** — er flasht beide, und der Lead kündigt jede Messung an |
| 5.5 (Lead, `tools/**`) neben 5.4 (`s3-developer`, S3-Baum) | Disjunkt, keine Abhängigkeit; **5.6 wartet auf beide** |

**Alles andere läuft seriell.** Insbesondere: **nie** zwei Tasks desselben Agenten gleichzeitig, **nie** Stufe n+1 vor dem Tor von Stufe n
(auch nicht „nur die Codierung" — der Nachweis davor ist der Zweck), **nie** zwei Messungen am selben Gerät.

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um. Der Abschnitt „Nicht Teil dieser Änderung" in `requirements.md` ist die Grenze —
   keine stillen Mitkorrekturen (A63/A64/A65, C6 u. a., die `C116`-Stückliste, die Ambilight-Reihenschaltung, **`display.c`**).
2. **Kodierung und Zeilenenden** jeder angefassten Datei vorher feststellen (CLAUDE.md, DIR-015). In `src/**` und
   `ESP8266/ESP-uclock/*.cpp` Umschrift; neue Dateien im S3-Baum ASCII.
3. **Kein `make`, kein `cmake`, kein `arduino-cli`, kein `idf.py`** durch Agenten (R1). Der Lead baut einmal je Stufe beziehungsweise je Nachweis.
4. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings.
5. Review ausdrücklich gegen `knowledge/architecture-checklist.md` (Spalte „Review"; Gesamtreview in 5.7).
6. **Gerätenachweis vor dem nächsten Task der Kette** (Spalte „Nachweis"). Der Nachweis nennt, **was er gemessen hat**, nicht nur „bestanden".
7. Erst dann nächster Task oder Schreibübergabe.

## Was der Lead nicht tut

Der Lead schreibt in diesem Paket **keine Zeile unter `src/**`, `ESP8266/**`, im S3-Baum oder unter `hardware/**`** — auch nicht die „offensichtliche"
Korrektur. Er baut, misst, pflegt `tools/**` und `.claude/**`, kündigt Messungen an und rollt aus. **Vor Task 0.3 gibt es den S3-Baum
nicht — wer ihn anlegt, ohne den Besitz zu registrieren, schafft ein Verzeichnis, dessen Schreiber niemand prüft.**

## Abschluss

- [ ] Alle Tasks erledigt, alle Tore G0–G5 und die Nachweise N1–N6 mit Mitschnitt im Bericht
- [ ] **Muss (a):** AK17 (Messreihe), AK17a, AK17b, AK17c bestanden **am Gerät** — und am Nachbau wiederholt (AK32); **F15 entschieden** (`R22`, Dämpfung)
- [ ] **Muss (b):** AK3 (inklusive beider Rückfälle), AK3a, AK3b bestanden **am Gerät** — und am Nachbau wiederholt (AK32)
- [ ] **AK35:** `display.c` und alle hardwarefreien Module unverändert, **Dateizahl genannt**, jede Ausnahme einzeln freigegeben
- [ ] **AK36:** Leiterplattenformat 1:1, **14 Merkmale** verglichen, Gehäuse passt; **F16** beantwortet
- [ ] **AK37:** 3,3-V-Schiene am DevKit **und** am Nachbau unter WLAN-Last gemessen, Minimum ≥ 3,0 V, bei 4,5 / 4,75 / 5,0 V
- [ ] `./tools/guardrails.sh --full` mit Exit 0 (schliesst Compile-Smoke-Tests ein)
- [ ] `./tools/smoke-device.sh` an **beiden** Geräten grün; `pwa-tester` an **beiden** Geräten ohne neuen Befund (DIR-012)
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt oder **ausdrücklich und begründet** abgewählt
- [ ] Jede neu gebaute Prüfung hat **einmal versagt**, samt Weg der Meldung (DIR-014, AK34); die Pinprüfung betrachtet **11** Signale, die Formatprüfung **14** Merkmale
- [ ] Drei Release-ZIPs beziehungsweise Rollouts, **jeweils benannt**, was zu flashen ist und in welcher Reihenfolge:
      (1) STM der alten Uhr, (2) ESP8266 der alten Uhr, (3) S3 per USB, danach `install-app.sh`
- [ ] Jedes Release ist **committet, getaggt und gepusht** (DIR-011)
- [ ] `./tools/install-app.sh --check` am S3 gelaufen (DIR-017)
- [ ] Die Frage, ob die PWA den entfallenden STM-Flash-Weg zeigen soll, ist als **Folgespec** angelegt (F10)
