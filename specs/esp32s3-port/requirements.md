# Anforderungen — Portierung auf einen einzelnen ESP32-S3 (Variante B)

**Status:** Entwurf — zur Freigabe durch den Nutzer. Ohne Freigabe beginnt keine Implementierung.
**Stand:** 2026-10-17 (Nachträge des Nutzers vom selben Tag eingearbeitet: 1:1-Umbau, zwei Muss-Kriterien,
LED-Board unverändert, Wiederverwendung der LED-Logik, Leiterplattenformat 1:1; dazu die belegten Fakten aus den
Komponentenblättern der Datenblatt-Analyse)
**Auslöser:** Nutzerwunsch vom 17.10.2026: Ein ESP32-S3 ersetzt **beide** Chips, den STM32F411 (U2)
und den ESP-12F (U4). Kein Eintrag in `REVIEW.md`, keine Massnahme aus `BEFUNDE.md`. Ein Feature
gehört nach `ERWEITERUNGEN.md` (Kennung `X<n>`); den Eintrag legt der `doc-writer` in Task 7.5 an,
**nach** der Freigabe.

## Wie diese Spec ihre Aussagen kennzeichnet

Nichts in dieser Spec stammt aus einem Review, deshalb gibt es kein `✔ verifiziert` / `● gemeldet`.
Stattdessen gelten drei Zeichen, damit niemand Vermutetes für Belegtes hält:

| Zeichen | Bedeutung |
|---|---|
| **✔** | Im Repo belegt — Datei, Zeile oder Zählung (Grep und Lesen vom 17.10.2026) |
| **✔K** | Im **Komponentenblatt** der Datenblatt-Analyse belegt (`hardware/komponenten/esp32s3-wroom-1.md`, `sy8301abc.md`), mit der dort genannten Datenblattseite. **Ich habe die Komponentenblätter gelesen, die PDFs nicht** |
| **◌** | Herstellerwissen zum ESP32-S3 oder Annahme des Nutzers. **Nicht im Repo, nicht am Gerät belegt.** Jede ◌-Aussage nennt den Task, der sie prüft |

**Zur Quelle der Hardware-Aussagen — eine Warnung, die der Skill `kicad-schema` selbst ausspricht.**
`HARDWARE.md` „entstand aus einer Abschrift und enthält Aussagen, die der Schaltplan nicht trägt"
(`.claude/skills/kicad-schema/SKILL.md`: IR und DCF77 haben auf dem Controller **keinen Stecker**, der
Gate-Pull-down von Q2 **existiert**, `SW4` schaltet die Konsole `H6`). **Massgebend ist die V2-Netzliste**
`hardware/esp32s3/v2/v2.xml`. Wo diese Spec einen Hardware-Satz aus `HARDWARE.md` übernimmt, ist er mit
„`HARDWARE.md`" gekennzeichnet und **nicht** als ✔ belegt, solange ihn die Netzliste nicht bestätigt. Gegen die
Netzliste geprüft habe ich für diese Spec nur: `R22` = 1 kΩ, `R20` 100 kΩ am Gate von Q2, das Netz `LDR`
(`H5.2`, `H7.5`, `R22.2`, `PA5`), `USB1` = „TYPE-C 6P". **Alles Übrige prüft Task 0.1.**

**Nicht gelesen für diese Spec:** das V2-PCB (`V2.kicad_pcb`, die Zahlen zum Format stehen im Skill und sind von
dort übernommen), die PDFs unter `hardware/komponenten/datenblaetter/`, das Komponentenblatt `tps2121.md`.
**Die Pinzuordnung unten ist deshalb ein Vorschlag, kein Befund.**

## Abstimmung mit anderen Specs und Rollen

- **`specs/multiboard-quelle/`** (Rolle `sw-architect`): Sie legt die **Architektur** fest — Naht zwischen Logik und Hardware, Verzeichnisse,
  Board-Beschreibung, Bau-Matrix, **Framework (ESP-IDF oder Arduino-ESP32)** — „mit Preis". **Diese Spec legt das Framework nicht fest, sondern verweist
  darauf** (F12). Die Aufgaben des S3-Ports sind diese Spec. Das Verzeichnis `esp32s3/` ist hier ein **Arbeitsname**; massgebend ist, was
  `multiboard-quelle` festlegt. Widersprüche zwischen beiden Specs gehen an den Lead, sie werden nicht überschrieben. **Zum Zeitpunkt dieser Spec
  liegt `specs/multiboard-quelle/` noch nicht vor** (Verzeichnis nicht vorhanden); die Rolle `sw-architect` ist angelegt (✔ `.claude/agents/sw-architect.md`).
- **`hw-architect`** entscheidet die Pinzuordnung (Ablage `hardware/architektur/pinbelegung.md`, ✔ laut `sw-architect.md`; **die Datei liegt noch nicht vor**).
  **Pins stehen im Code an genau einer Stelle** (Board-Beschreibung, `multiboard-quelle`) und werden **nicht** ein zweites Mal von Hand geführt.
- **`hw-datasheet-analyst`** liefert die Komponentenblätter. Vorhanden und hier verwendet: `esp32s3-wroom-1.md` (Datenblätter v1.8/v2.2, Stand 17.10.2026),
  `sy8301abc.md`, `tps2121.md`.

## Problem

Heute besteht die Uhr aus zwei Rechnern, die über eine serielle Leitung als „Kommandobrücke"
verbunden sind (`HARDWARE.md`, Abschnitt „Beide Rechner können einander zurücksetzen"):

- **STM32F411** (`HARDWARE.md`: 84 MHz aus 25-MHz-Quarz) führt `src/**` aus: Display-Zustandsmaschine,
  LED-Ausgabe, Sensoren, EEPROM. Umfang der grössten Dateien ✔: `display.c` **6'739 Zeilen** (gelesen am
  17.10.2026; **nicht** 7'374), `main.c` 5'378, `irmp.c` 5'114, `vars.c` 1'916.
- **ESP-12F** (✔ ESP8266) führt `ESP8266/ESP-uclock/**` aus: WLAN, HTTP, PWA-Auslieferung, NTP,
  Wetter, Update. Umfang ✔: `http.cpp` 17'651 Zeilen, `ESP-uclock.ino` 2'471, `vars.cpp` 1'649,
  `stm32flash.cpp` 1'438.
- Die Brücke ist ein UART mit 115200 Baud, ✔ 256 Byte Empfangsring auf der ESP-Seite und 1'024 Byte auf der
  STM-Seite (`UART_RXBUFLEN`). Sie ist die Quelle mehrerer belegter Befunde (A6, A17, A19, A21,
  `specs/bruecke/`). Zwei Chips heissen auch zwei Firmware-Stände, zwei Flash-Wege, zwei Versionen.

## Ziel

Ein **Umbau des jetzigen Prints und seiner Funktion 1:1 auf den ESP32-S3**: **kein Funktionsverlust, keine
neuen Funktionen** (Nutzer, 17.10.2026). Ein **ESP32-S3** führt beides aus: die bisherige STM-Logik in einem
Task, die bisherige ESP-Logik in einem zweiten, verbunden durch dieselbe Kommandobrücke — als Warteschlange
statt als UART. Protokoll, PWA und Legacy-Oberfläche bleiben **unverändert**. Dieselbe Firmware läuft auf dem
**ESP32-S3-DevKitC-1 (v1.1)** und auf dem **Nachbau der Platine** mit dem Modul ESP32-S3-WROOM-1.

**Geändert wird nur die Controller-Platine, und ihr Format bleibt.** Das **LED-Board bleibt unverändert** und
hängt — mit LDR und den 114 LEDs — **unverändert über den gespiegelten 7-poligen Stecker `H7`** am Controller
(`HARDWARE.md`, „Der Stecker ist gespiegelt").

**Die Anzeigelogik wird wiederverwendet, nicht neu gebaut.** Die LED-Ansteuerung hängt in `display.c` an
**sechs** Treiberfunktionen; der S3 bekommt einen eigenen Treiber mit **derselben** Schnittstelle, `display.c` und
alle hardwarefreien Module bleiben **unverändert** (E11). Eine zweite Zustandsmaschine entsteht nicht.

**Was „kein Funktionsverlust" hier heisst, und die eine Spannung darin.** Verloren geht allein, was am
zweiten Chip hing und mit ihm entfällt (E6): das **Flashen des STM32 über die Uhr** und das **Zurücksetzen
des STM über den ESP**. Die Funktion „Neustart" bleibt erhalten (F10, `design.md` §6). Jede andere sichtbare
Funktion — PWA, Legacy-Oberfläche, Wetter, Ticker, Overlays, IR, DCF77, Ambilight, Backup — bleibt. Den
Nachweis führt AK21 plus AK30; Task 0.4 erstellt dafür ein **Funktionsinventar** (jede Funktion, die ein Nutzer
sieht, mit der Stelle, die sie trägt).

## Festgelegte Entscheidungen des Nutzers (nicht neu verhandelt)

| # | Entscheidung |
|---|---|
| E1 | Zielprodukt ist ein **Nachbau der Platine** mit dem **Modul ESP32-S3-WROOM-1**. Zusätzlich muss **dieselbe Software** auf dem DevKitC-1 v1.1 laufen |
| E2 | Daraus folgt: **nur GPIOs, die das DevKit am Header herausführt**, und **keine** der folgenden: GPIO0, 3, 45, 46 (Strapping), GPIO19/20 (natives USB), GPIO43/44 (UART0), GPIO35–37 (PSRAM/Flash der R8-Modelle), GPIO38 und 48 (RGB-LED des DevKits) |
| E3 | **Ein** USB-C-Anschluss mit Datenleitungen an GPIO19/20 (USB-Serial/JTAG): Flashen, Konsole, JTAG. Die Konsole läuft **zusätzlich** über UART0, damit der UART-Port des DevKits nutzbar bleibt. Der heutige 6-polige, reine Strom-Stecker wird ein 16-poliger mit D+/D−, **plus ESD-Schutz** |
| E4 | EEPROM bleibt der **AT24C32 am I²C**, **Datenformat unverändert**. RTC bleibt der **DS3231** |
| E5 | Die **Kommandobrücke bleibt als Schicht** (zwei Tasks mit Warteschlange statt UART), damit Protokoll und PWA unverändert bleiben. Die Display-Zustandsmaschine hat **genau einen Schreiber-Task**. `pending_ticker_restore` (CLAUDE.md) wird **nicht vereinfacht** |
| E6 | **Entfällt:** der STM32-Flash-Weg (`stm32flash.cpp`, `tools/flash-stm.sh`, DIR-010) |
| E7 | Reihenfolge der Umsetzung: **zuerst LED-Ausgabe auf dem S3 isoliert nachweisen, dann I²C/EEPROM/RTC, dann Sensoren, zuletzt Brücke und Task-Aufteilung** |
| E8 | **1:1-Umbau.** Kein Funktionsverlust, keine neuen Funktionen (Nachtrag vom 17.10.2026) |
| E9 | **Muss-Kriterien**, je mit eigenem Abnahmekriterium am Gerät: **(a)** der analoge LDR-Eingang (`GPIO1`, ADC1; die Normierung gegen `ldr_min_value`/`ldr_max_value` aus dem EEPROM **bleibt**; **`R22` auf dem Controller wechselt von 1 kΩ auf ca. 47 kΩ**, weil der ESP32-ADC unten ungenau ist; die **Kalibrierung wird neu gemessen**) und **(b)** **Flashen, Konsole und JTAG über den einen USB-C-Anschluss** (natives USB, GPIO19/20) |
| E10 | **LDR und LEDs bleiben auf dem LED-Board** und hängen unverändert über den gespiegelten Stecker `H7` am Controller. **Das LED-Board wird nicht geändert** — weder Bestückung noch Leiterbild noch Stecker. Änderungen betreffen **nur die Controller-Platine** |
| E11 | **Wiederverwendung der LED-Ansteuerungslogik so weit wie möglich.** Die Naht ist die **Treiber-API** (sechs Funktionen). Der S3 bekommt einen eigenen Treiber mit derselben API (RMT als Backend); **`display.c` und alle hardwarefreien Module werden unverändert übernommen**. Für die hardwarenahen Dateien entsteht eine **schmale Hardwareschicht mit den bisherigen Funktionsnamen**, damit der Rest nicht angefasst wird. **Keine Änderung an `display.c` ohne gesonderten Grund und Beleg.** Zuerst **nur** die Treiber-API auf dem S3, mit Abnahme am Gerät |
| E12 | **Das Leiterplattenformat bleibt 1:1** (Randbedingung, Tabelle unten): Umriss, Befestigungslöcher, Lage der Stecker und Taster wie in V2 |

**Zu E9(a), was die Datenblatt-Analyse dazu festhält (✔K, `esp32s3-wroom-1.md` §6):** `R22` = 1 kΩ ist der schlechteste Wert (das Dunkel liegt unter dem
Gesamtfehler jeder Dämpfungsstufe). Die Analyse rechnet **zwei Paarungen** als brauchbar: **10 kΩ mit 6 dB** und **47 kΩ mit 12 dB**; welche gilt, ist **noch nicht entschieden**
(F15). E9 nennt 47 kΩ; diese Spec behandelt das als Startwert der Messreihe (AK17) und **nimmt die Entscheidung des Nutzers nicht zurück**.

### Vorläufige Pinzuordnung — **Vorschlag, gegen den Schaltplan abzugleichen (Tasks 0.1a–c)**

| Signal | GPIO | bisher am STM32 (`HARDWARE.md`) | Peripherie am S3 | am DevKit-Header (✔K §3.2) |
|---|---|---|---|---|
| LDR (Abgriff vom LED-Board über `H7` Pin 5; Teilerwiderstand `R22` auf dem Controller, neu ca. 47 kΩ) | **GPIO1** | PA5 (✔ Netz `LDR` an `U2` Pin 15, `v2.xml`) | ADC1 (✔K: ADC1 = GPIO1–10) | J3 |
| DS18B20 | **GPIO2** | PB5 | 1-Wire | J3 |
| LED-Daten | **GPIO5** | PB1 (TIM3 CC4 + DMA) | RMT TX, danach der bestehende Pegelwandler U3 | J1 |
| LED-Versorgung (`WS2812_POWER`) | **GPIO6** | PB0 | Ausgang → R19 → Q2 → Q1 | J1 |
| IR-Empfänger | **GPIO7** | PB4 | Eingang, Abtastung per Zeitgeber-ISR | J1 |
| I²C SDA | **GPIO8** | PB7 | I²C-Master (RTC und EEPROM) | J1 |
| I²C SCL | **GPIO9** | PB6 | I²C-Master | J1 |
| DCF77 | **GPIO15** | PB8 | Eingang | J1 |
| Taster USER / WPS | **GPIO16 / GPIO17** | PA0 / PA7 | Eingänge | J1 |
| Status-LED | **GPIO18** | PC13 | Ausgang | J1 |

Gegenprobe gegen E2: Keiner dieser elf Pins steht auf der Ausschlussliste. **Neu belegt (✔K):** Alle elf liegen am Header des DevKitC-1 v1.1 (`J1`: 5, 6, 7, 8, 9, 15, 16, 17, 18; `J3`: 1, 2),
und **keiner trägt am DevKit eine Fremdbelegung** (belegt sind dort GPIO0 BOOT-Taster, EN Reset-Taster, GPIO38 RGB-LED, GPIO43/44 an der USB-UART-Brücke, GPIO19/20 am zweiten USB-Anschluss).
Das ist aus dem **Komponentenblatt** übernommen; den **Abgleich gegen den Schaltplan des Nachbaus** leisten die Tasks 0.1a bis 0.1c, und AK1 verlangt ihn.
**Die Pinzuordnung entscheidet `hw-architect`**; diese Spec übernimmt ihr Ergebnis und widerspricht ihm nicht, ausser wo die Software etwas nicht trägt.

## Randbedingung: Das Leiterplattenformat bleibt 1:1 (E12)

Die Zahlen stammen aus `V2.kicad_pcb` (Edge.Cuts und Footprint-Positionen, per Skript ausgelesen,
Koordinaten relativ zur linken oberen Ecke) und stehen im Skill `.claude/skills/kicad-schema/SKILL.md`; **ich habe
die PCB-Datei nicht selbst gelesen**.

| Merkmal | Wert in V2 | wird verglichen |
|---|---|---|
| Umriss | **145,00 × 39,00 mm**, zwei Kupferlagen | ja (1) |
| Befestigung | `H8` (3,25 / 28,00), `H9` (72,50 / 3,25), `H10` (141,75 / 28,00); M2,5-Löcher | ja (3) |
| Stromeingang extern | `H1` (35,75 / 27,75), `H2` (30,25 / 27,75), 2,2-mm-Pads, `H1` am Netz `ALT_5V_INPUT` | ja (2) |
| USB | `USB1` an der **Unterkante**, Mitte (72,50 / 35,38) | ja (1) |
| LED-Stecker und LDR-Stecker | `H7` (38,63 / 4,75) um 180° gedreht, `H4` (12,00 / 4,29), `H5` (26,00 / 3,25), beide −90° | ja (3) |
| Konsole | `H6` (103,75 / 4,50) | ja (1) |
| Taster | `SW3` (86,00 / 27,75), `SW2` (99,00 / 27,75), `SW1` (112,00 / 27,75) | ja (3) |
| Funkmodul | `U4` (ESP-12F) bei (127,00 / 12,25), **Antenne an der Oberkante** | **nein** — der S3-Modul sitzt dort, mit Antenne an der Oberkante |

**Verglichen werden 14 Merkmale** (1 + 3 + 2 + 1 + 3 + 1 + 3). Die Zahl steht hier, damit eine Prüfung, die eine andere
Zahl meldet, als **falsch** gilt und nicht als „bestanden" (CLAUDE.md, DIR-014).

**Wer was liegt:** **Die Hardware-Spezifikation selbst liegt bei den Hardware-Rollen unter `hardware/`**
(`hw-architect`: `hardware/architektur/`, `hw-datasheet-analyst`: `hardware/komponenten/`, `hw-designer`:
`hardware/esp32s3/`, `hw-reviewer`: `hardware/review/`, ✔ Skill `kicad-schema`, Abschnitt „Wo was liegt"). **Diese Spec verweist nur darauf**
und fordert, was die Software voraussetzt (Abschnitt „Was die Software von der Hardware voraussetzt").
Das V2-Projekt unter `~/Documents` bleibt **nur lesend**; der Designer arbeitet an einer Kopie unter `hardware/esp32s3/`.

**Drei Dinge, die das Format berühren und nicht belegt sind:**

1. **Der Stecker `USB1` wechselt von 6 auf 16 Pole** (E3; ✔ heute „TYPE-C 6P", Footprint `TYPE-C-SMD_TYPE-C-6P`, `v2.xml`).
   Seine Bauhöhe, Breite und Lage der Öffnung müssen zum Gehäuse passen. **Ob der 16-polige Stecker dieselben Aussenmasse hat,
   ist nicht belegt** — Frage F16, Analyst prüft.
2. **Der S3-Modul sitzt dort, wo `U4` sitzt**, mit der Antenne an der Oberkante (✔ Skill). Die Antennenfreihaltung (F4) muss
   **in diesem Raum** gelöst werden, nicht an einem anderen Ort der Platine.
3. **Die Taster.** Auf V2 liegen `SW1` (WPS), `SW2` (USER) und `SW3` (Reset) an den drei Positionen (✔ Skill). Der S3 braucht **zusätzlich** einen **BOOT-Taster an GPIO0 und
   einen EN-Taster** als Rückweg zum Flashen (✔K, H1). **Wo der BOOT-Taster bei unverändertem Format sitzt, ist offen** — Frage F17.

## Akzeptanzkriterien

Jedes Kriterium nennt, **womit** es nachprüfbar ist. Kriterien mit **Spec-Setzung** enthalten eine Zahl,
die diese Spec festlegt, weil kein Herstellerwert und keine Messung vorliegt — der Nutzer darf sie bei der
Freigabe ändern. Alle Gerätenachweise laufen auf dem **DevKit-Testaufbau** (Hardware-Voraussetzung:
F8), bis ein Nachbau existiert; AK32 wiederholt sie auf dem Nachbau.

### Muss-Kriterien des Nutzers (E9) — je eigene Abnahme am Gerät

| Muss | Kriterien | Stufe |
|---|---|---|
| **(a) Analoger LDR-Eingang** | **AK17, AK17a, AK17b, AK17c** | 4 |
| **(b) Flashen, Konsole, JTAG über den einen USB-C-Anschluss** | **AK3 (Flashen), AK3a (Konsole), AK3b (JTAG)** | 1 |

Ein Muss-Kriterium gilt erst, wenn **jedes** seiner Kriterien am Gerät bestanden ist. Fällt eines durch,
**stoppt die Stufe** — es wird nicht „später nachgezogen". Beide stehen bewusst früh: Das USB-Kriterium ist
die Voraussetzung jeder weiteren Gerätearbeit (ohne Flashen kein Test), das LDR-Kriterium berührt die
Platine und damit die Bestellung des Nachbaus.

### Hardware und Plattform

- [ ] **AK1 — Pinbelegung.** Eine Tabelle in `design.md` (Abschnitt 2, Spalte „abgeglichen") ist für alle elf
      Signale mit Schaltplan-Netzname des Nachbaus **und** Pin am DevKit-Header ausgefüllt. Keine Zeile
      verwendet einen Pin der Ausschlussliste E2. Nachprüfbar durch das **Gutachten von `hw-reviewer`** (Netzliste ↔ Tabelle,
      als Mengen von `Ref.Pin`, mit der Zahl der betrachteten Netze) und durch den Vergleich mit `hardware/architektur/pinbelegung.md`.
- [ ] **AK2 — Ein Binärabbild.** Die Firmware wird **einmal** gebaut. Dasselbe Abbild (gleiche SHA-256-Summe,
      vom Lead im Bericht genannt) läuft auf dem DevKit und auf dem Nachbau. Es gibt kein `#ifdef DEVKIT`
      und keine zweite Pinkonfiguration. Nachprüfbar: `grep -rn DEVKIT` über den S3-Quelltext ohne Treffer, zwei
      Geräte melden dieselbe Abbild-Summe über die Konsole beim Start.
- [ ] **AK3 — MUSS (b): Flashen über den einen USB-C-Anschluss.** Mit **nur** dem USB-C-Kabel an GPIO19/20
      (kein UART-Adapter, kein Zusatzdraht) wird ein neues Abbild **vollständig** geflasht (Bootloader,
      Partitionstabelle, Anwendung, LittleFS-Abbild) und läuft danach an. Das Werkzeug ist `esptool`; Reset und Download-Modus löst es über
      **DTR/RTS** aus (✔K `esp32s3-wroom-1.md` §7). **Dreimal hintereinander** auf dem normalen Weg, **ohne** Boot-Taster. **Zusätzlich drei Rückfälle,
      jeder mit Beleg „Hash of data verified" und anschliessendem Anlauf:**
      (1) **Die Firmware hängt** (Testabbild mit Endlosschleife): Flashen gelingt trotzdem — **zweimal**.
      (2) **Die Firmware hat GPIO19/20 umkonfiguriert** (Testabbild, das die USB-Funktion abschaltet): Das USB-Gerät **verschwindet** (✔K §7), der automatische Weg
      entfällt, und **nur der manuelle Weg** hilft — **GPIO0 auf Low (BOOT-Taster) und EN kurz auf Low (Reset-Taster)**. Der manuelle Weg gelingt **einmal**. Dieser Fall beweist,
      dass **BOOT- und EN-Taster auf der Platine** sind und GPIO0/EN nicht anderweitig belegt sind (H1).
      (3) Nach (2) ist die Uhr **ohne Zusatzmittel** wieder im Normalzustand (Abbild aus Schritt 1 des normalen Weges flasht wieder ohne Taster).
      **Die Abnahme „Flashen gelingt auch, wenn die Firmware hängt" gilt damit für beide Arten des Hängens.**
- [ ] **AK3a — MUSS (b): Konsole über USB und über UART0.** Dieselbe Firmware bedient **beide Wege**; sie sind **getrennte Kanäle** (✔K §9). **Verlangt ist Ausgabe, keine Eingabe:**
      In ESP-IDF ist die USB-Zweitkonsole **nur Ausgabe**, nicht blockierend, ohne Eingabe, und sie erscheint **nur, wenn UART0 nicht verbunden ist** (✔K §9, ● IDF). **Prüfung in zwei Läufen, nicht gleichzeitig:**
      (1) **UART0:** Kabel am DevKit-Anschluss „UART" (CP2102N an GPIO43/44, 115200 Baud) — Konsolenzeilen erscheinen im Terminal. (2) **USB:** Kabel **nur** am DevKit-Anschluss „USB"
      (GPIO19/20), UART-Anschluss ohne Kabel — **dieselben** Konsolenzeilen erscheinen im Terminal. Auf dem Nachbau ist nur (2) möglich, soweit dort kein UART-Stecker bestückt ist (F12, F17).
      **Eine Anforderung „Eingabe über beide Kanäle" gibt es nicht**; ob und wie die Firmware Eingabe zulässt, wäre eine neue Funktion (E8). Die ROM-Meldungen beim Start erscheinen ohnehin auf
      beiden Wegen (✔K §9) und gelten **nicht** als Nachweis der Anwendungskonsole.
- [ ] **AK3b — MUSS (b): JTAG über den einen USB-C-Anschluss.** Ein Debugger verbindet sich über **dieselbe**
      USB-Buchse (◌ OpenOCD mit USB-Serial/JTAG-Konfiguration; ✔K §7: JTAG läuft gleichzeitig zur seriellen Verbindung), meldet **beide** Kerne des S3, hält die
      Anwendung an einem Haltepunkt im **Display-Task** an und lässt sie weiterlaufen. Danach läuft der
      Task-Watchdog **nicht** in einen Reset (Haltepunkt über 3 s, dann Weiterlaufen: keine Neustartzeile).
      Beleg: Mitschnitt der Debugger-Sitzung im Test-Bericht. **Auf dem DevKit läuft das über den USB-Anschluss
      „USB", nicht über „UART"** — das DevKit hat zwei Buchsen, der Nachbau eine. ✔K §4.1: GPIO3 ist **schwebend**; die JTAG-Quelle ist im Auslieferungszustand die USB-Schnittstelle,
      **solange EFUSE_STRAP_JTAG_SEL nicht gebrannt ist** (diese Spec brennt keine eFuse).
- [ ] **AK4 — Konsole ohne Gegenstelle.** Läuft die Uhr **ohne** angeschlossenen USB-Host und ohne
      UART-Gegenstelle, blockiert die Konsolenausgabe weder den Display-Task noch den Netz-Task. Messbar:
      `diag`-Folge (Δu) über 10 Minuten ≤ `DIAG_INTERVAL_SEC + 2 s` bei vollem Log. ✔K §9: Nach vollem Puffer wartet der Chip **einmalig 50 ms**, wenn kein Terminal liest — **eine kurze Pause,
      kein Absturz**; die Grenze von 2 s lässt sie zu.

### Wiederverwendung (E11) — gilt über das ganze Paket

- [ ] **AK35 — `display.c` und die hardwarefreien Module bleiben unverändert.** `git diff` gegen den Stand vor dem Paket ist
      für **`src/display/display.c`**, `display.h`, `src/overlay/`, `src/vars/`, `src/tables/`, `src/font/`, `src/alarm/`,
      `src/night/`, `src/weather/`, `src/timeserver/` sowie die `.c`-Dateien von `rtc`, `eeprom`, `ldr`, `ds18xx` und
      `tempsensor` **leer**. **Jede Ausnahme** steht mit **gesondertem Grund und Beleg** (Compilermeldung, Messung, Zeile) in
      `design.md` §3.6, ist vom Nutzer freigegeben, **bevor** sie umgesetzt wird, und ist im Bericht einzeln benannt.
      Die Prüfung nennt die **Zahl der betrachteten Dateien** (Task 0.4 legt die Liste fest); eine Prüfung, die nur eine Teilmenge sieht,
      meldet OK und sieht den Gegenstand nicht (CLAUDE.md, DIR-014, sechster Fall).

### Stufe 2 — LED-Ausgabe, isoliert (Tasks 2.x)

- [ ] **AK5 — Timing.** Am Signal hinter U3, **gemessen am Stecker `H7` Pin 4** (und, wo zugänglich, an
      `V1.DIN` hinter `R4` auf dem unveränderten LED-Board; `HARDWARE.md`: 550 Ω Serienwiderstand in Summe),
      mit Logikanalysator oder Oszilloskop, gilt für den Kanal mit den Sollwerten aus `sk6812.c`
      (✔ aktiver Satz: Periode 1250 ns, T0H 300 ns, T1H 750 ns, Pause 250 µs, `sk6812.c:91-97`): jede Zeit liegt innerhalb
      **±150 ns** (**Spec-Setzung**, nach SKC6812-Datenblatt zu bestätigen — F9), die Pause zwischen zwei
      Bildern beträgt **mindestens 250 µs** (`HARDWARE.md`: SKC verlangt > 200 µs). Messprotokoll mit drei
      Bildern im Test-Bericht.
- [ ] **AK6 — Bild.** Ein Testprogramm **ohne jede Zeile aus `display.c` oder `main.c`** setzt alle 114 LEDs in Rot, Grün,
      Blau, Weiss (W-Kanal) und lässt ein einzelnes Licht von LED 1 bis 114 laufen. Der Nutzer bestätigt
      Reihenfolge und Farben am LED-Board. **Die Farbbyte-Reihenfolge je LED ist Grün, Rot, Blau, Weiss** (GRBW;
      ✔ `sk6812.c:354-356` sendet zuerst `green`; die übrigen drei Bytes belegt Task 2.1 am Quelltext).
- [ ] **AK6a — Treiber-API unverändert (E11).** Der S3-Treiber bietet **genau die sechs Funktionen** von `sk6812.h`
      (✔ `sk6812.h:49-57`): `sk6812_init`, `sk6812_refresh`, `sk6812_set_led`, `sk6812_set_all_leds`,
      `sk6812_get_refresh_cnt`, `sk6812_get_dma_wait_cnt`, mit denselben Parametern und Rückgabetypen. `display.c` erreicht sie über die
      Makros `led_init`, `led_refresh`, `led_set_led`, `led_set_all_leds`, `led_get_refresh_cnt`, `led_get_dma_wait_cnt`
      (✔ `display.h:94-99`; Aufrufe laut Grep: 19 in `display.c`, 2 in `main.c`). Nachweis: Der Testtreiber wird gegen die **originale**
      `sk6812.h` übersetzt (ohne Warnung); `git diff src/sk6812/sk6812.h src/display/display.h` ist **leer**; ein Aufrufer,
      der die sechs Makros verwendet, übersetzt und linkt gegen den S3-Treiber **ohne Änderung**. Die beiden Zähler liefern **echte** Werte
      (`led_get_refresh_cnt` zählt Bilder, `led_get_dma_wait_cnt` zählt Wartefälle auf das vorige Bild) — die Diagnosezeile in `main.c`
      zeigt sie an, und ein stilles `0` wie bei den anderen Treibern (✔ `display.h:114-115`) wäre hier ein **Informationsverlust**, den E8 ausschliesst.
- [ ] **AK7 — Last.** Während das Testprogramm 10 Minuten läuft, ist der WLAN-Stack aktiv und erhält
      Verkehr (Dauer-`ping` mit grossen Paketen von einem zweiten Rechner, **plus** eine laufende
      HTTP-Anfrage-Schleife gegen das Testprogramm). Der Nutzer sieht **kein** Flackern und keinen
      Farbfehler; der Treiber zählt Sendefehler und meldet **0**.
- [ ] **AK8 — Länge.** Der Treiber sendet auch **234** LEDs (114 + `DSP_AMBILIGHT_LEDS 120`) in einem Bild,
      ohne dass das Testbild abbricht. Gemessen wird die Bilddauer: 114 LEDs ≈ 4,6 ms + Pause (`HARDWARE.md`),
      234 LEDs entsprechend ≈ 9,4 ms + Pause, jeweils ±10 %.
- [ ] **AK9 — LED-Versorgung und Ruhepegel.** `GPIO6` schaltet die Versorgung der LED-Kette (Messung an `TP7`
      bzw. an `TO_VCC_RGB_5V`). **Der Pegel von GPIO6 und der Zustand der Versorgung beim Einschalten, beim
      Reset und während des Flashens werden mit dem Oszilloskop aufgezeichnet.** Die LED-Versorgung ist in allen
      drei Zuständen **aus**. **Was die Datenblätter erwarten (✔K §4.2):** Beim Einschalten geben **GPIO1–18 rund 60 µs lang Low** aus, danach sind sie hochohmig bis zum ersten Treiben;
      für ein Gate mit **Pull-down und N-Kanal-Q2** ist ein Low-Glitch **harmlos**, ein **High**-Glitch wäre es nicht (für GPIO6 nicht aufgeführt, für GPIO18 ja). Auf V2 hält `R20` (100 kΩ)
      das Gate von Q2 auf Masse (✔ `v2.xml`). **Die Messung prüft das am Nachbau und am DevKit-Aufbau und trägt die Dauer jedes Glitches ein** — sie ersetzt die Datenblattaussage nicht,
      sie bestätigt sie. **Die Software schaltet `GPIO6` als allererste Anweisung der Anwendung auf Low** (`design.md` §4.5); die Zeit davor (ROM, Bootloader) trägt allein die Platine.
      Schlägt die Messung fehl, ändert sich die Platine — nicht die Software.
- [ ] **AK10 — Pegelwandler.** Das Signal an `GPIO5` erreicht die LED-Kette ausschliesslich über einen
      Pegelwandler mit TTL-Eingang (Nachbau: U3). Auf dem DevKit-Testaufbau ist er ein externes Bauteil gleicher
      Art (F8). **Kein Dauerbetrieb mit 3,3 V direkt an einer 5-V-Kette** (`HARDWARE.md`: grenzwertig). ✔K §5: Ein 3,3-V-Ausgang erfüllt VOH ≥ 0,8 · VDD = 2,64 V;
      ob das für den 74AHCT1G125 bei 5 V Versorgung reicht, steht im Datenblatt des Wandlers (**vom Analysten nicht gelesen**, F18).

### Stufe 3 — I²C, EEPROM, RTC (Tasks 3.x)

- [ ] **AK11 — EEPROM-Rundlauf.** Auf einem **Test-EEPROM** (nicht der Chip einer produktiven Platine) werden
      4'096 Byte mit einem Prüfmuster geschrieben und gelesen: 0 Abweichungen. Das Schreiben geschieht über die
      **unveränderte** Funktion `eeprom_write()` (Seitengrösse 32 Byte, `HARDWARE.md`).
- [ ] **AK12 — Schreibzeiten.** Ein Schreibzyklus über eine Seite kostet **≤ 20 ms**, 32 Byte über zwei Seiten
      **≤ 45 ms** (**Spec-Setzung**; Vergleichswerte am STM32: 15 ms bzw. 35–36 ms, `HARDWARE.md`,
      `BEFUNDE.md` L403). Die Zeiten stammen aus dem Mitschnitt des Treibers, nicht aus einer Schätzung.
- [ ] **AK13 — Datenformat unverändert.** `git diff` auf `src/eeprom/**`, `src/vars/**` und alle Dateien, die das
      EEPROM-Layout festlegen, ist **leer** (Layout-Dateien benennt Task 0.4). Ein Wert, den die alte Firmware
      schreibt, liest die neue unverändert — nachgewiesen in AK29 über den Backup-Weg. **Ausnahme:** die
      LDR-Kalibrierwerte `ldr_min_value`/`ldr_max_value` sind **Werte**, keine Layoutänderung; sie werden mit
      dem neuen `R22` neu gemessen (AK17b) und überschreiben die Backup-Werte der alten Uhr.
- [ ] **AK14 — RTC.** Der DS3231 liefert Zeit und Temperatur. Zeit setzen, 60 s warten, lesen: Differenz
      60 s ± 1 s. Das Lesen einer **leeren Adresse** liefert einen **NACK-Fehlercode**, der sich von einer
      **Bus-Störung** unterscheiden lässt (✔ Semantik aus `BEFUNDE.md` L402/A73: ein NACK ist keine Störung).
- [ ] **AK15 — Bus-Erholung.** Wird SDA absichtlich 100 ms nach Masse gezogen, erholt sich der Bus (neun Takte
      zur Freigabe, ✔ Vorbild `i2c_reset_bus()`), der folgende Zugriff gelingt, und der Fehler steht im Log.
      Kein Zugriff wartet länger als 5 ms je Flag-Äquivalent (✔ `I2C_TIMEOUT 5`).

### Stufe 4 — Sensoren (Tasks 4.x)

- [ ] **AK16 — Softuhr und Zeitgeber.** Der Zeitgeber-Interrupt (heute TIM2 mit `F_INTERRUPTS = 15'000`,
      ✔ `main.c:725-731`) läuft mit **ganzzahligem** Teilerverhältnis; die Softuhr weicht über 10 Minuten
      **höchstens 100 ms** von der RTC ab (**Spec-Setzung**; ein Quarz mit ± 10 ppm läge bei rund 6 ms).
      Während eines Flash-Schreibzugriffs (LittleFS-Schreiben, ◌ Cache aus) bleibt der Interrupt
      **fehlerfrei**: 100 Schreibvorgänge nacheinander, danach stimmt der Zähler `diag_tick_cnt` mit der
      Solldauer auf ± 0,1 % überein.
- [ ] **AK17 — MUSS (a): ADC-Skala als Messreihe.** Der ADC-Treiber liefert denselben **Zählwertbereich wie heute**: 12 Bit, bezogen auf 3,3 V
      (Zählwert = `U / 3,3 V × 4095`; ✔ `ldr.c:61-77` normiert eine geklammerte Kopie gegen `ldr_min_value`/`ldr_max_value` aus dem EEPROM; **`ldr.c` und die Normierung bleiben unverändert**,
      `git diff` leer). **Nachgewiesen wird mit einer Messreihe, nicht mit einem Einzelwert:** Ein Poti an `GPIO1` liefert **mindestens acht** Spannungsstufen von **0,10 V bis zum oberen Bereichsende der gewählten
      Dämpfung** (✔K DS-C Tab. 5-6: **12 dB: 0 … 2900 mV, Gesamtfehler ±50 mV; 6 dB: 0 … 1600 mV, ±10 mV**; 0 dB: 0 … 850 mV, ±5 mV). Je Stufe: Spannung mit Multimeter, **Mittelwert über 64 Messungen** des Treibers,
      Zeile in einer Tabelle Soll / Ist / Abweichung. **Zulässige Abweichung je Stufe: der Gesamtfehler der gewählten Dämpfung laut Datenblatt plus 1 % des Messwerts** (**Spec-Setzung**; ein fester Prozentsatz wäre unter 1 V
      unerfüllbar — 3 % von 0,15 V sind 4,5 mV, der kalibrierte Fehler bei 12 dB ±50 mV). **Stufen, deren Spannung unter dem Gesamtfehler liegt, werden eingetragen, aber nicht als „bestanden" gezählt**
      (✔K §6.1: unterhalb des Gesamtfehlers „nicht aussagekräftig"). **Die obersten zwei Stufen liegen im oberen Fünftel des Bereichs** — dort ist der Messbereich ◌ nicht mehr so genau, wie der Datenblattwert verspricht,
      und die Reihe zeigt es. Fällt eine zählende Stufe durch, gilt AK17 als **nicht bestanden**, und F15 (Wert von `R22`, Dämpfung) ist zu entscheiden.
- [ ] **AK17a — MUSS (a): Teiler am LED-Board über `H7`.** Mit dem **unveränderten** LED-Board, dem
      Original-Kabel und dem **gewählten `R22`** (Startwert ≈ 47 kΩ) stimmen die an `GPIO1` gemessenen Spannungen mit der
      Rechnung aus `design.md` §4.2 auf **±10 %** (Bauteiltoleranz des LDR) überein, und zwar für drei
      Beleuchtungen (abgedeckt, Zimmerlicht, Taschenlampe nah). `H7` Pin 3 liefert 3,3 V, Pin 5 den Abgriff;
      beide werden am Stecker gemessen, nicht am Bauteil — der **gespiegelte** Anschluss ist Teil der Prüfung
      (`HARDWARE.md`: Controller-Pin *n* gehört an LED-Board-Pin 8 − *n*). Das Netz `LDR` trägt auf V2 **zusätzlich**
      `H5.2` (✔ `v2.xml`); `H5` bleibt unbenutzt und unverändert (E12), darf aber den Abgriff **nicht belasten**.
- [ ] **AK17b — MUSS (a): Kalibrierung neu gemessen.** Die Werte `ldr_min_value` und `ldr_max_value` werden am
      Gerät **neu** bestimmt (abgedeckt = Minimum, hellste vorgesehene Beleuchtung = Maximum) und über den
      vorhandenen Weg im EEPROM abgelegt; die alten Werte (Skala mit `R22` = 1 kΩ, rund 4 bis 373 Zählwerte,
      `HARDWARE.md`) gelten **nicht** mehr. Das Messprotokoll nennt: Beleuchtung, Spannung an `GPIO1` (Multimeter),
      Zählwert, gewählten Grenzwert. **Wiederholung nach Neustart:** Die Werte stehen nach einem Reset
      unverändert im EEPROM.
- [ ] **AK17c — MUSS (a): Wirkung am Display.** Fünf Beleuchtungsstufen von dunkel bis hell ergeben eine
      **monoton steigende** Display-Helligkeit (die Uhr dimmt im Dunkeln, nicht umgekehrt); der kleinste Zählwert liegt **über dem Gesamtfehler** der gewählten Dämpfung (bei 12 dB über rund 62 Zählwerten = 50 mV, bei 6 dB
      über rund 12 = 10 mV; **gerechnet**). **Sättigung oben:** Erreicht der hellste vorgesehene Zustand das Bereichsende der Dämpfung, nennt das Protokoll den Anteil der Zeit, und der Nutzer hat die Sättigung bei der
      Wahl von F15 **ausdrücklich angenommen** (✔K §6.2: bei 10 kΩ/6 dB sättigen die hellsten Fälle, was für eine Helligkeitsregelung unschädlich ist). **Rauschen:** Bei konstantem Licht
      streut der gefilterte Zählwert über 60 s um **höchstens ± 2 %** des Messbereichs (**Spec-Setzung**; ◌ der
      ADC rauscht, deshalb mittelt der Treiber, `design.md` §4.2) — die Helligkeit **flackert** nicht. **Auch unter WLAN-Dauerlast** (siehe AK37), weil die Messung
      ✔K **nicht ratiometrisch** ist: Die Knotenspannung folgt der 3,3-V-Schiene, die ADC-Referenz nicht.
- [ ] **AK18 — DS18B20.** Der Sensor liefert die Temperatur, die ein Referenzthermometer ±0,5 K bestätigt.
      Die Fehlersemantik aus A2 bleibt (✔ `CLAUDE.md`, Thema 1): Lese- oder CRC-Fehler liefern **255**, drei
      Fehler in Folge gelten als „verloren". Nachweis: Sensor abziehen — der Mitschnitt zeigt
      `ds18xx verloren`; Sensor stecken — `ds18xx erkannt`.
- [ ] **AK19 — IR.** Die vorhandene Fernbedienung wird erkannt: für fünf Tasten meldet die Uhr **Protokoll,
      Adresse und Befehl** gleich wie die alte Uhr (Vergleichswerte vom Nutzer, an der alten Uhr abgelesen).
      Anlernen über `learn_ir` funktioniert. Während eines `ping`-Dauerlaufs über WLAN fällt in 100 Tastendrücken
      **keiner** aus.
- [ ] **AK20 — DCF77 und Taster.** Taster USER und WPS lösen je ein Ereignis aus (Mitschnitt). DCF77: Ein
      Signalgenerator mit gültigem Bitmuster (oder der angeschlossene Empfänger) ergibt innerhalb von **5 Minuten**
      eine gültige Zeit; ein gestörtes Muster ergibt **keine** Zeit.

### Stufe 5 — Brücke und Task-Aufteilung (Tasks 5.x)

- [ ] **AK21 — Brücke und Protokoll unverändert.** Eine Kommandofolge, die an der alten Uhr mitgeschnitten
      wurde (Vollabgleich mit rund 194 `var`-Kommandos, Ticker, Overlay-Import), läuft gegen die neue Brücke:
      alle Quittungen kommen, **0 Verluste**, die Antworten stimmen Zeile für Zeile bis auf die in `design.md`
      genannten Ausnahmen. `git diff` auf `ESP8266/ESP-uclock/data/app/**` ist **leer** — die PWA ist
      unverändert (`APP_VERSION` und `CACHE_NAME` bleiben).
- [ ] **AK22 — Rückstau ohne Verlust.** Liest der Netz-Task absichtlich 2 s lang nicht, **zählt** die Brücke
      jedes verworfene Zeichen (Feld `d=` der Diagnosezeile, ✔ Name bleibt) — und **verwirft nichts ohne
      Zähler**. Der Display-Task steht dabei nicht (Δu der Diagnosefolge ≤ `DIAG_INTERVAL_SEC + 2 s`).
- [ ] **AK23 — Ein Schreiber.** Die Display-Zustandsmaschine (Zustände benannt in Task 0.4: `display`-Struktur,
      `display_clock_flag`, `pending_ticker_restore`, Ticker-, Icon- und Overlay-Zustand) wird **nur vom
      Display-Task** geschrieben — **ohne Änderung an `display.c`** (E11, AK35). Nachweis zweifach: (a) **Statisch, am gelinkten Abbild:**
      Die Netz-Komponente hat **keine** unaufgelöste Referenz auf ein Symbol der STM-Komponente und umgekehrt (`nm`); die Zahl der geprüften
      Symbole steht im Bericht; (b) **Dynamisch, im Debug-Build, an den Übergängen:** Jede Funktion der **Hardwareschicht und der Brücke** prüft
      die Task-Kennung ihres Aufrufers (`display_task` für die STM-Seite, `net_task` für die Netz-Seite) und bricht bei Verletzung mit Meldung
      ab — und **bricht nachweislich ab**, wenn man eine Verletzung herstellt (DIR-014). **Begründung, warum nicht an den Schreibstellen:**
      Dort läge die Prüfung in `display.c`, und die bleibt unverändert.
- [ ] **AK24 — Restore-Bedingung.** Der Block `pending_ticker_restore && ! display_ticker_active () &&
      ! display.do_display_icon && show_icon_stop_time == 0 && show_overlay_idx >= MAX_OVERLAYS` ist im Quelltext
      **unverändert** (✔ heute `main.c:4910-4914`) — alle **vier** Teilbedingungen, nicht vereinfacht. Dazu
      ein Gerätenachweis: Wetter-Ticker, danach läuft die Zeit weiter (Ablauf aus `TESTPLAN-PWA.md`, vom
      `pwa-tester` gefahren).
- [ ] **AK25 — Watchdog.** Der Task-Watchdog hat **20 s** (✔ wie heute `WATCHDOG_TIMEOUT_MS`). Der
      Display-Task ist registriert, **der Netz-Task auch** (mit eigener Frist). Ein Reset durch den Watchdog
      steht beim nächsten Start im Log mit dem Wort, das `watch-log.sh` erkennt (Task 5.5 baut das Muster, Task 5.6 prüft es);
      **ein Reset durch Unterspannung** (Brownout) steht **mit einem eigenen Wort** im Log, damit er nicht als Watchdog-Reset erscheint (AK37).
      Die Aufrufstellen von `watchdog_reload ()` bleiben **dieselben wie heute** (Task 0.4(i) zählt sie nach) **plus eine** im
      Kopf des Netz-Tasks (`design.md` §3.4); Guardrail S7 zieht mit.
- [ ] **AK26 — Burst.** Die Reproduktion aus `CLAUDE.md` (offene Themen, Punkt 2): zehn schnelle `ticker_set` mit je
      32 Zeichen. Erwartung am S3: **kein Reset**, Δu der Diagnosefolge ≤ `DIAG_INTERVAL_SEC + 5 s`. Das Ergebnis
      wird **eingetragen, nicht gedeutet**: Es ist **kein** Nachweis, dass der F411-Hänger behoben ist (siehe
      „Nicht Teil dieser Änderung").
- [ ] **AK27 — Update.** Ein Abbild wird über den Update-Server eingespielt (`/api/update_*`); danach meldet
      `/api/update_status` die neue Version. Ein **abgeschnittenes** und ein **fremdes** Abbild (ein ESP8266-`.bin`)
      werden **abgewiesen**, die Uhr läuft mit dem alten Stand weiter.
- [ ] **AK28 — STM32-Flash-Weg entfernt.** Es gibt im S3-Abbild weder `stm32flash` noch einen Pfad, der den
      Reset-Pin eines STM ansteuert. Die Endpunkte, die heute den STM flashen oder zurücksetzen, **antworten
      mit einem definierten Fehler** (Festlegung F10) statt still `{"ok":true}` zu melden. Nachprüfbar am Gerät mit
      den lesenden Anteilen von `smoke-device.sh`; die gefährlichen Endpunkte bleiben gesperrt (DIR-008).
- [ ] **AK37 — WLAN unter Last: Spannung am 3,3-V-Netz.** Beim **ersten WLAN-Test unter Last** und bei jeder Wiederholung (am DevKit in Task 5.6, am Nachbau in Task 6.6) wird die Spannung am **3V3-Pin des Moduls**
      mit dem Oszilloskop **mit Minimum-Erfassung** über **mindestens 10 Minuten** aufgezeichnet. **Last:** WLAN-Dauersenden (Dauer-`ping` mit grossen Paketen — die Antworten sind Senden — plus eine
      HTTP-Abrufschleife grosser Dateien von zwei Clients), dazu das **hellste Bild, das die Uhr im Normalbetrieb zeigt** (der Nutzer wählt es; die Datenblattwerte siehe `design.md` §4.6).
      **Kriterien:** (1) das Minimum der 3,3-V-Spannung liegt **nicht unter 3,0 V** (✔K DS-M Tab. 6-2: empfohlener Bereich 3,0 / 3,3 / 3,6 V) und das Maximum nicht über 3,6 V; (2) **kein** Neustart in den
      10 Minuten (Reset-Grund ≠ Unterspannung, AK25); (3) die Quelle liefert **mindestens 0,5 A** (✔K DS-M S. 27), **die Spitze des Moduls beim WLAN-Senden beträgt 355 mA** (✔K DS-M Tab. 6-4; Messung am Aufbau
      trägt die gemessene Spitze ein). **Am Nachbau zusätzlich bei den Eingangsspannungen 4,5 V, 4,75 V und 5,0 V an der 5-V-Schiene** (✔K `sy8301abc.md` §4.5: bei knapper Schiene sinkt die 3,3-V-Schiene unter 3,3 V;
      Rechnung: 3,0 V bei 4,5 V Eingang und 0,6 A) **und mit dem Netzteil, das der Nutzer einsetzen wird**. **Am DevKit** trägt die Messung nur das **Verhalten des Moduls** ein; sie sagt **nichts**
      über die Versorgung des Nachbaus (F1).
      (Zahlen 3,0 V und 10 Minuten: Datenblatt beziehungsweise **Spec-Setzung**.)

### Stufe 6 — Abnahme

- [ ] **AK29 — Migration über Backup.** Der Nutzer exportiert ein Backup von der alten Uhr und importiert es
      am S3. Danach liefert `./tools/snapshot-device.sh` an beiden Geräten Abzüge, die sich **nur** in den von
      Natur aus veränderlichen Feldern unterscheiden (Uptime, LDR-Rohwert, Temperatur, Uhrzeit, Versionen,
      Geräte-ID) **und in den LDR-Kalibrierwerten** (AK17b). **Der Import löst allein der Nutzer aus**
      (CLAUDE.md, R5).
- [ ] **AK30 — Gerätedurchlauf.** Der `pwa-tester` fährt die Phasen 0 bis 4 und 9 von `TESTPLAN-PWA.md` am S3
      ohne **neuen** Befund (DIR-012: der Smoketest allein ist nicht der Test). Dazu das Funktionsinventar aus
      Task 0.4: **jede** dort genannte Funktion ist am S3 entweder nachgewiesen oder als „entfällt mit dem
      zweiten Chip" (E6) benannt — eine dritte Möglichkeit gibt es nicht (E8).
- [ ] **AK31 — Dauerlauf.** 72 Stunden Betrieb mit laufender Logwache (`./tools/watch-log.sh`, DIR-013),
      keine Exception, kein Neustart, kein Sprung in `d=`, keine Lücke in der `diag`-Folge.
      **Freispeicher:** der kleinste je gemeldete Wert fällt nach den ersten 10 Minuten um **höchstens 10 %**
      (**Spec-Setzung**; schliesst einen Speicherverlust aus, nicht eine kleine Reserve).
- [ ] **AK32 — Nachbau.** AK2, AK3, AK3a, AK3b, AK4, AK9, AK17 bis AK17c, AK30 und AK37 werden **auf dem Nachbau**
      wiederholt, mit demselben Abbild wie auf dem DevKit.
- [ ] **AK32a — LED-Board unverändert.** Das LED-Board ist **dasselbe Bauteil wie an der alten Uhr**, ohne
      Nacharbeit: derselbe Satz Platine und Stecker, in beiden Controller-Varianten steckbar (E10). Nachweis: der
      Nutzer steckt **dasselbe** LED-Board nacheinander an die alte Uhr und an den S3-Controller; beide zeigen
      dasselbe Bild (AK6) und denselben LDR-Verlauf (AK17c). Kein Lötpunkt, kein Draht, kein geänderter Wert auf dem
      LED-Board.
- [ ] **AK33 — Bestehende Ziele unverändert.** `./tools/guardrails.sh --full` läuft mit Exit 0. Die Ziele
      STM32F411, STM32F103 und ESP8266 bauen weiter (Task 5.1/5.3 ändern sie, brechen sie nicht), und die
      Legacy-Oberfläche der alten Hardware antwortet nach dem STM-Release aus Task 5.1 wie zuvor (`smoke-device.sh`
      und ein `pwa-tester`-Durchlauf an der alten Uhr).
- [ ] **AK34 — Neue Prüfungen haben einmal versagt (DIR-014).** Jede in dieser Spec neu gebaute Prüfung
      (Pinprüfung, Symbolprüfung, Task-Kennungsprüfung, Logwache-Muster, `display.c`-Unverändert-Prüfung, Formatprüfung) ist **einmal absichtlich
      verletzt** worden und hat angeschlagen — samt Weg der Meldung bis zum Empfänger. Dazu die Zählung: Die
      Pinprüfung betrachtet **elf** Signale, die Formatprüfung **14** Merkmale; zeigt eine eine andere Zahl, ist sie falsch.
- [ ] **AK36 — Leiterplattenformat 1:1 (E12).** Die Leiterplatte des Nachbaus hat dieselben **14 Merkmale** wie V2 (Tabelle
      „Randbedingung"): Umriss 145,00 × 39,00 mm, `H8`/`H9`/`H10`, `H1`/`H2`, `USB1`, `H7`/`H4`/`H5`, `H6`, `SW1`–`SW3` an den Koordinaten
      von V2 (Abweichung höchstens **0,01 mm**, **Spec-Setzung**: Rundung). Nachprüfbar durch ein Prüfskript der Hardware-Rollen, das Edge.Cuts
      und Footprint-Positionen beider PCB-Dateien vergleicht und die **Zahl der verglichenen Merkmale** nennt; es versagt einmal absichtlich
      (verschobene `H9`). Der Nachbau passt in dasselbe Gehäuse (Nutzer, am Gerät). **Zusätzlich:** Der 16-polige `USB1` passt in die
      Gehäuseöffnung des 6-poligen (F16).

## Nicht Teil dieser Änderung

- **Das Boardlayout selbst.** Diese Spec fordert Beschaltung (siehe „Was die Software von der Hardware voraussetzt") und führt
  offene Fragen; die Hardware-Spezifikation und das Layout liegen bei den Hardware-Rollen unter `hardware/`.
- **Das Framework und die Verzeichnisaufteilung** (F12): Sie entscheidet `specs/multiboard-quelle/`. Diese Spec legt sie nicht fest.
- **Das LED-Board** (E10): kein Wechsel von LED-Typ, Stecker, LDR, `R4`, `R5`–`R7`, `C116` oder Abblockung. Die
  Stücklisten-Unstimmigkeit bei `C116` (`HARDWARE.md`: 680 µF gegen 100 µF) bleibt **offen und unberührt**.
- **Das Leiterplattenformat** (E12): kein anderer Umriss, keine verschobenen Löcher, Stecker oder Taster. Eine Änderung braucht eine
  eigene Entscheidung des Nutzers.
- **Eine zweite Display-Zustandsmaschine, ein neues Anzeige-Modul oder eine Kopie von `display.c` für den S3** (E11). Die
  vorhandene Logik wird übernommen.
- **Neue Funktionen** (E8), **auch eine Konsoleneingabe**. Jede Verbesserung, die einem Nutzer auffiele, gehört in eine eigene Spec.
- **Änderungen an `ESP8266/ESP-uclock/data/app/**`.** PWA, Service Worker, Markup, Stilvorlage und
  Übersetzungen bleiben unberührt. Eine Anpassung der Oberfläche an den entfallenden STM32-Flash-Weg ist
  eine **Folgespec** (F10).
- **Der F411-Hänger** (`CLAUDE.md`, offene Themen, Punkt 2). Der S3 hat eine andere CPU, einen anderen
  Bus-Treiber und ein anderes Zeitverhalten. Ein Port, der den Hänger nicht zeigt, **widerlegt ihn nicht** und
  **belegt keine Ursache**. Kein Recovery-Mechanismus, kein pauschaler Fix.
- **Die offenen Befunde A63, A64, A65** (1-Wire-Abtastzeitpunkt, 85 °C, Auflösung). Der Port übernimmt die
  Logik der DS18xx-Schicht **unverändert**; er korrigiert sie nicht nebenbei und verdeckt sie nicht.
- **Der Schutz der zerstörerischen Aufrufe** (`BEFUNDE.md`, Entscheidung vom 13.10.2026: C6, C6v, C60, C61, C62,
  C6b). Er bleibt zurückgestellt. Der S3 erbt den offenen Zustand; er verschlimmert ihn nicht. **Neu am S3:**
  USB-Serial/JTAG gewährt jedem mit Zugriff auf die Buchse Lese- und Schreibzugriff auf den Flash.
- **Secure Boot, Flash-Verschlüsselung und jede eFuse-Programmierung.** Das sind unumkehrbare Eingriffe am Chip
  und brauchen eine eigene Entscheidung des Nutzers. Diese Spec brennt **keine eFuse**.
- **Neue Funktionen im engeren Sinn:** Bluetooth, PSRAM-Nutzung, USB-Geräteklasse, zweites LED-Signal, neue
  API-Endpunkte, neue numerische Variablen, Änderung des Brückenprotokolls.
- **EEPROM-Acknowledge-Polling statt festem Wartezyklus.** Eine Verbesserung, aber eine Änderung des
  Verhaltens. Der Port behält `EEPROM_WAITSTATES`.
- **Entfernen der alten Ziele.** STM32F411/F103 und ESP8266 bleiben baubar und lieferbar (Annahme F11).
- **Algorithmische Änderungen** an `display.c`, `irmp.c`, `overlay.c`, `weather.c` und allen weiteren Modulen
  ausserhalb der Plattformnähte. Eine Änderung dort braucht eine eigene Spec.
- **Versionsnummern in lebenden Dokumenten** (DIR-006).

## Was die Software von der Hardware voraussetzt (Schnittstelle zu den Hardware-Rollen, nur Controller-Platine, E10)

**Die Hardware-Spezifikation liegt unter `hardware/`.** Diese Tabelle nennt nur, **was die Software-Nachweise voraussetzen**.
Wer es nicht erfüllt, macht Nachweise der Stufen 2 bis 5 am Nachbau ungültig. Einzelheiten und Seitenzahlen: `hardware/komponenten/esp32s3-wroom-1.md` (✔K).

| # | Anforderung | Prüfweg |
|---|---|---|
| H1 | **EN-Beschaltung 10 kΩ nach 3V3 und 1 µF nach GND, EN nie schwebend**; **ein EN-Taster (Reset) und ein BOOT-Taster an GPIO0** bleiben **auf der Platine** — sie sind der **einzige Rückweg**, wenn die Anwendung GPIO19/20 umkonfiguriert hat (✔K §7, §12). GPIO0 mit Pull-up, **keine grossen Kondensatoren an GPIO0**. GPIO0 hat **keine** andere Funktion | Schaltplan, AK3 (Rückfall 2) |
| H2 | **GPIO45 nie hoch** beim Einschalten (am N8 würde der Flash mit 1,8 V gespeist; ✔K §4.1); GPIO46 ohne starken Pull-up; **GPIO3 unbeschaltet** (schwebend, ohne internen Pull) | Schaltplan |
| H3 | USB-Anschluss **16-polig** mit D+/D− auf GPIO19/20, **ESD-Schutz** an D+/D− und VBUS, CC1/CC2 je 5,1 kΩ wie heute (`HARDWARE.md`: R3/R4). ✔K §7: **Platz für Reihenwiderstände 22/33 Ω und Kondensatoren dicht am Modul**, differentielle Leitung 90 Ω ± 10 %; Empfehlung des Herstellers: **externer Pull-up an D+** für einen stabilen Anfangszustand | Schaltplan, AK3, AK3a, AK3b |
| H4 | D+/D− als **Differenzpaar** geführt, kurz, ohne Stub zu den Federleisten; **USB- und UART-Leitungen weit weg von der Antenne** (✔K §8) | Layoutprüfung durch `hw-reviewer` |
| H5 | LED-Versorgung ist beim Einschalten, Reset und Flashen **aus** (V2: `R20` hält das Gate von Q2 auf Masse, ✔ `v2.xml`; ✔K §4.2: GPIO1–18 geben 60 µs Low aus, danach hochohmig) | AK9 |
| H6 | Antennenbereich des Moduls **freigehalten**, **an der Oberkante, an der Stelle von `U4`** (E12). ✔K §8: Antennenfläche **6 mm tief**, seitlich **mindestens 15 mm** frei auf der Seite der Aussparung, im Gehäuse **mindestens 15 mm** Abstand in alle Richtungen; Modul **nicht in die Platinenmitte** | F4 |
| H7 | Eingänge IR und DCF77 sind **3,3-V-verträglich**. ✔K §5: Es gibt **keine Aussage „5 V tolerant"** und **keinen Grenzwert je Einzelpin**; behandle jeden GPIO als **höchstens 3,6 V** (Ableitung aus VIH max = VDD + 0,3 V, **keine** Herstellerangabe). Auf V2 haben beide **keinen Stecker**, nur 100-kΩ-Pull-ups (`R8`, `R9`) und den Controller-Pin (✔ Skill `kicad-schema`) | F3 |
| H8 | Die Stromversorgung liefert **mindestens 0,5 A** (✔K DS-M S. 27); die Spitze des Moduls beim WLAN-Senden beträgt **355 mA** (✔K Tab. 6-4); **22 µF + 0,1 µF am Modul**, **10 µF am Platineneingang** (✔K §8) | F1, AK37 |
| H9 | **`R22` auf dem Controller** wechselt von 1 kΩ (✔ V2) auf einen Wert nach F15 (Startwert ca. 47 kΩ, E9). Der ADC-Eingang `GPIO1` erhält zusätzlich **0,1 µF gegen Masse** (✔K HDG S. 18: empfohlen; die Datenblattwerte gelten mit 100 nF) | AK17, AK17c, F15 |
| H10 | Der Stecker `H7` bleibt **in Belegung und Spiegelung unverändert** (E10): Pin 1/2 5 V, 3 3,3 V, 4 Daten, 5 LDR, 6/7 GND (`HARDWARE.md`; gegen die Netzliste zu prüfen, Task 0.1c). Die 3,3 V an Pin 3 kommen vom Rail, den der SY8301 liefert (F1) | AK17a, AK32a |
| H11 | **Das Leiterplattenformat bleibt 1:1** (E12, Tabelle „Randbedingung") | AK36 |
| H12 | **Nur ADC1** für den LDR (✔K §6.1: ADC2 teilt sich den ADC mit dem WLAN); GPIO1 ist ADC1 | AK17 |

## Offene Fragen — nicht geraten

Jede Frage nennt, **wer sie beantwortet**, **woran** man sie beantwortet und **was bis dahin gilt**. Die Komponentenblätter beantworten einige Fragen **teilweise**; wo das der Fall ist, steht
es in der Frage. Quelle der Zahlen ist das Komponentenblatt (✔K), nicht mein Lesen des Datenblatts.

| # | Frage | Beantwortet durch | Bis dahin |
|---|---|---|---|
| **F1** | **SY8301 (U10): trägt er den S3?** **Teilweise beantwortet (✔K `sy8301abc.md` §3, §4.3):** Ausgang **1 A**, Strombegrenzung 1,6–2,5 A; das Modul braucht eine Quelle von ≥ 0,5 A, **Spitze 355 mA** beim Senden; bei 600 mA Gesamtlast bleiben **mindestens 250 mA Reserve**. **Offen und kritischer: die Spannungsreserve bei knapper 5-V-Schiene** (FB hinter der Diode `D2`): Das Blatt rechnet bei 4,5 V Eingang und 0,6 A rund **3,0 V** am Verbraucher, bei 4,75 V rund 3,2 V (Rechnung, **Datenblatt schweigt zum Verhalten am Tastgradanschlag**) und nennt den Brownout als Risiko. Zusatz: `F1` (PTC 1,5 A im USB-Pfad) und `F2` (3 A) sind kleiner als 3 A LED + 0,6 A (nur aus Bezeichnungen gelesen) | `hw-architect` (Strombudget); **Messung AK37** an Prototyp und Nachbau | Kein Nachbau bestellt, bevor AK37 am DevKit-Verhalten des Moduls vorliegt; **AK37 am Nachbau entscheidet** |
| **F2** | **Pull-down am Gate von Q2 und Pin-Zustand beim Einschalten.** **Beantwortet:** Auf V2 existiert `R20` (100 kΩ) zwischen Gate und Masse (✔ `v2.xml`). ✔K §4.2: GPIO1–18 sind im Reset **hochohmig** und geben beim Einschalten **60 µs Low** aus (GPIO18 zusätzlich High); ein Low-Glitch ist für den N-Kanal-Q2 mit Pull-down **harmlos**. **Offen:** ob der Nachbau `R20` übernimmt; das Blatt nennt **GPIO21** als sauberen Kandidaten für ein solches Gate (hochohmig im Reset, kein Glitch, kein Strapping) — Entscheid `hw-architect` | `hw-architect`; Messung AK9 | GPIO6 bleibt der Vorschlag; LED-Versorgung am Testaufbau **nur** über einen Handschalter |
| **F3** | **5-V-Toleranz von IR (GPIO7) und DCF77 (GPIO15).** **Beantwortet (✔K §5):** keine Herstellerangabe „5 V tolerant", **kein Grenzwert je Pin** — behandle jeden GPIO als **höchstens 3,6 V**. **Offen:** Auf V2 haben beide **keinen Stecker**, nur `R8`/`R9` (100 kΩ) und den Controller-Pin (✔ Skill); **an welche Schiene die Pull-ups gehen und woher die angeschlossenen Empfänger gespeist werden, ist nicht belegt.** Die Eingänge des STM32F411 sind an den fraglichen Pins ◌ 5-V-tolerant | `hw-architect` mit Netzliste (Task 0.1b); Datenblätter von TSOP und DCF77-Modul liefert der Nutzer | Testaufbau speist beide aus 3,3 V. Ein 5-V-Signal an einen S3-Pin ist ein **Hardware-Fehler**, kein Softwarefall |
| **F4** | **Antennenfreihaltung.** **Zahlen beantwortet (✔K §8, HDG):** Antennenfläche 6 mm tief, seitlich ≥ 15 mm frei, im Gehäuse ≥ 15 mm Abstand; Modul am Rand, **nicht in der Platinenmitte**; USB- und UART-Leitungen weit weg. **Offen:** ob das **im unveränderten Umriss** (E12) an der Stelle von `U4` (127,00 / 12,25, Antenne an der Oberkante) geht, mit den Nachbarn dort (`H6` bei 103,75 / 4,50) und im Gehäuse | `hw-architect`, `hw-reviewer` am Layout; Empfangsmessung (RSSI) am fertigen Gerät | Keine Aussage zur Reichweite |
| **F5** | **Genaue GPIO-Liste am DevKit-Header.** **Beantwortet (✔K §3.2, DK Tab. J1/J3):** Alle 36 Modul-GPIOs liegen am Header; **alle elf Pins des Vorschlags** liegen dort, keiner mit Fremdbelegung (GPIO38 = RGB-LED bei v1.1, GPIO0 BOOT, GPIO43/44 an der USB-UART-Brücke, GPIO19/20 am zweiten USB). Das **DevKit mit N8R8** sperrt zudem GPIO35–37 (✔K §3.3), E2 schliesst sie schon aus | `hw-reviewer` bestätigt am Nachbau-Schaltplan (Task 0.1c) | Gilt als **abgeglichen gegen das DevKit**, **ungeprüft gegen den Nachbau** |
| **F6** | **Zielmodul-Variante.** Das Komponentenblatt führt **ESP32-S3-WROOM-1-N8** (8 MB Quad-Flash, **kein PSRAM**, alle 36 GPIOs frei, ✔K §1) als **gewählt**, **N8R2** als Zweitwahl; JLCPCB-Bestand N8: 416 Stück am 17.10.2026 (**Beschaffungsrisiko**, ✔K §10). N8R8/N16R8 sperren GPIO35–37 | Nutzer bestätigt N8 | Der Entwurf rechnet mit **8 MB Flash** (jetzt ✔K, nicht mehr nur Annahme) |
| **F7** | **Pull-ups auf der Platine.** I²C (SDA/SCL), 1-Wire (DS18B20) und Taster: sind externe Pull-ups bestückt oder tragen heute die internen des STM32 (`HARDWARE.md`: „Entprellung und Pull-ups der Taster nicht geprüft")? Interne S3-Pull-ups (✔K typ. 45 kΩ) sind für I²C bei 400 kHz und für 1-Wire ◌ **zu schwach** | `hw-architect` mit Netzliste (Task 0.1b) | Testaufbau mit externen Pull-ups 4,7 kΩ an SDA, SCL und DQ |
| **F8** | **DevKit-Testaufbau.** Das DevKit hat weder U3 noch Q1/Q2 noch Sensoren. Wer baut den Aufbau: externer 74AHCT125 für die LED-Daten (AK10), EEPROM-Breakout (nicht produktiv, AK11), DS3231-Modul, DS18B20, TSOP, Taster, **LDR-Teiler mit dem echten LED-Board und `R22` auf dem Steckbrett (AK17a)**? | Nutzer | Stufen 2 bis 4 sind **blockiert**, bis der Aufbau steht |
| **F9** | **Toleranz der SKC6812-Zeiten.** Das Datenblatt der SKC-Variante (`HARDWARE.md`: nur Spannung, Ruhestrom, Datenrate) nennt keine Toleranz, die sich aus dem Repo ergibt. AK5 setzt ±150 ns | Nutzer: Datenblatt OPSCO SKC6812 (liegt **nicht** unter `hardware/komponenten/datenblaetter/`) | **±150 ns** als Setzung |
| **F10** | **Was geschieht mit den STM-Endpunkten und der PWA-Oberfläche dafür?** Heute: `/api/remote_stm32_flash`, `stm32_default` in `/api/update_status`, `/api/maintenance_reset_stm32` (✔ 18 Treffer dieser Namen in `app.js`). Empfehlung des Entwurfs: Endpunkte bleiben, antworten mit einem **definierten Fehlercode** („nicht unterstützt"); die PWA wird in einer **Folgespec** angepasst. `maintenance_reset_stm32` setzt am S3 den **ganzen Chip** zurück (nicht den Display-Task) — Gründe in `design.md` §6. **Das ist die einzige Stelle, an der „kein Funktionsverlust" nicht wörtlich gilt** (siehe „Ziel") | Nutzer: Entscheidung | Die Empfehlung gilt |
| **F11** | **Bleiben die alten Ziele baubar?** Annahme: **ja**, die Uhr im Keller und die produktiven Uhren laufen weiter mit STM+ESP8266 | Nutzer | Annahme gilt; sie bestimmt Task 5.1 und 5.3 (gemeinsamer Quelltext statt Fork) |
| **F12** | **Framework, Build-System, Verzeichnis, Besitz.** **Entscheidet `sw-architect` in `specs/multiboard-quelle/`, mit Preis** (ESP-IDF oder Arduino-ESP32; Auswahl zur Bauzeit, über `#if` oder Laufzeit-Schicht). Diese Spec **verweist darauf und legt nichts fest**; `design.md` §1.1 trägt meine Beobachtungen als **Eingabe** für diese Entscheidung. Die Konsolenkonfiguration (AK3a) hängt am Framework (✔K §9). **Kein heutiger Agent besitzt `esp32s3/`**, der Besitz-Hook kennt es nicht; die Rolle legt allein der Lead an. Ob der Nachbau einen UART0-Stecker erhält, ist offen (F17) | `sw-architect` (Architektur), Lead (Rolle, Hooks, Task 0.3), `hw-architect` (Stecker) | **Task 1.1 beginnt nicht**, bevor `multiboard-quelle` das Framework festlegt |
| **F13** | **Versionierung und Tag.** `src/main.h` (`VERSION`) und `version.h` (`ESP_VERSION`) hängen heute an zwei Chips. Das S3-Abbild enthält beides. Empfehlung: beide Symbole bleiben (die PWA zeigt zwei Versionen), werden gemeinsam geführt. Tag-Schema `release/<stm>-<esp>-<app>` passt weiter; ob ein viertes Feld nötig ist, entscheidet der Nutzer. **Auch `sw-architect` soll das entscheiden** (✔ `sw-architect.md`: „Entscheide in der Spec, wie ein Ein-Chip-Board diese Versionen führt") | `sw-architect` und Nutzer | Empfehlung gilt; **DIR-004** (kein Build ohne Anhebung) gilt weiter |
| **F14** | **`F_INTERRUPTS` am S3.** 15'000 Hz teilen sich aus den Zeitgebertakten des S3 nicht ganzzahlig (◌ 80 MHz / 15'000 = 5'333,3). Kandidaten, die sich exakt teilen: 16'000 und 20'000 Hz. IRMP-Konstanten leiten sich aus `F_INTERRUPTS` ab (✔ `irmp.c:58` u. a.) | `stm-developer` und `firmware-analyst`, Task 0.4 | Entwurf rechnet mit 20'000 Hz; AK16 und AK19 entscheiden |
| **F15** | **`R22`, Dämpfung des ADC, Abblockkondensator.** **Das Komponentenblatt rechnet (✔K §6.2):** `R22` = 1 kΩ ist ungeeignet (Dunkel unter dem Gesamtfehler jeder Stufe). **10 kΩ mit 6 dB** (0 … 1600 mV, ±10 mV): bester Kompromiss aus Auflösung und Fehler, dunkel 33 mV, Zimmer 1,10 V, hell 1,65 V — **die hellsten Fälle sättigen**; **47 kΩ mit 12 dB** (0 … 2900 mV, ±50 mV): alle drei Zustände im Bereich, dunkel 148 mV (≈ 3 × Fehler), hell 2,72 V — **nahe an der 2,9-V-Grenze, bei +10 % Versorgung bereits darüber**; 100 kΩ scheidet aus. **Die Entscheidung fällt nicht** (E9 nennt „ca. 47 kΩ", das Blatt „noch nicht entschieden"). Offen ausserdem: Die **zulässige Quellimpedanz** steht in **keiner** der vier Unterlagen, und die Nichtlinearität **unter rund 50 mV** ist nicht angegeben. **Die Messreihe AK17 entscheidet** | `hw-architect` mit dem Nutzer; Messreihe am DevKit-Testaufbau mit Steckbrett-`R22` | 47 kΩ / 12 dB und 100 nF als Startwert der Messreihe; Dämpfung und `R22` sind **benannte Konstanten**, kein Wert im Code verstreut |
| **F16** | **Hat der 16-polige USB-C-Stecker dieselben Aussenmasse wie der 6-polige von V2?** Heute „TYPE-C 6P", Footprint `TYPE-C-SMD_TYPE-C-6P` (✔ `v2.xml`). Der Skill `kicad-schema` hält fest: „**nicht belegt**"; das Komponentenblatt des Moduls behandelt den Stecker nicht. Betrifft Bauhöhe, Breite, Lage der Öffnung und damit das Gehäuse (E12) | `hw-datasheet-analyst`: Komponentenblatt der beiden Stecker mit Massen aus den Datenblättern (Task 0.1a); `hw-architect` entscheidet vor Layoutbeginn | **Layout beginnt nicht**, bevor die Frage beantwortet ist. Die Software ist davon unberührt |
| **F17** | **Wo sitzen BOOT- und EN-Taster, wenn das Format 1:1 bleibt?** Auf V2 sind `SW1`–`SW3` an drei Positionen belegt (✔ Skill; `SW3` setzt heute den STM zurück). Der S3 braucht **EN** (Reset) und **BOOT an GPIO0** (✔K §7: einziger Rückweg, AK3 Rückfall 2). `SW3` kann EN werden; der BOOT-Taster braucht **eine zusätzliche Stelle** oder eine Doppelbelegung von `SW1`/`SW2` — **GPIO0 darf nach E2 keine Anwendungsfunktion haben** | `hw-architect` | AK3 Rückfall 2 ist ohne BOOT-Taster **nicht nachweisbar**; am DevKit stehen beide Taster zur Verfügung |
| **F18** | **74AHCT1G125 mit 3,3-V-Eingang des S3.** Die Eingangsschwelle bei 5 V Versorgung steht im Datenblatt `sn74ahct1g125.pdf`; der Analyst hat es **nicht gelesen** (✔K §5, §14). Gegenüber steht VOH ≥ 2,64 V (✔K) | `hw-datasheet-analyst` (Task 0.1a) | V2 betreibt den Wandler mit dem STM32 (3,3 V) — der Wandler ist **bewährt**, aber mit dem **S3** nicht belegt |

## Betroffene Laufzeiten

- [x] **ESP32-S3** (neu) — **Erst-Flash** über USB, kein OTA aus einem alten Stand möglich
- [x] STM32 (`src/**`) — **Neu-Flashen nötig**, einmal: Task 5.1 (Nahtstellen in `main.c` und der Zeitbasis herauslösen). Verhalten unverändert; **`display.c` bleibt unberührt**
- [x] ESP8266 (`ESP8266/ESP-uclock/*.cpp`) — **Neu-Flashen nötig**, einmal: Task 5.3 (Quelltext gemeinsam nutzbar machen)
- [ ] PWA (`data/app/**`) — **nicht berührt**; AK21 prüft es
- [x] Build/Release — neues Target, Release-ZIP enthält ein weiteres Abbild, `deploy.sh`-Serverlayout, Hooks,
      `guardrails.sh`, `smoke-device.sh`, `install-app.sh` (Prüfung), neues Flash-Skript (Task 6.2)
- [x] **Hardware** — Nachbau der **Controller-Platine** im **unveränderten Format** (Nutzer, Hardware-Rollen), Testaufbau am DevKit (F8).
      **LED-Board: nicht berührt**

**Flash-Reihenfolge für das Gesamtpaket** (CLAUDE.md, „Firmware zuerst"): Die alten Ziele erhalten ihre Nahtstellen-Releases
(Tasks 5.1r und 5.3r), **bevor** Task 5.4 beginnt. Auf dem S3 selbst gibt es nur **ein** Abbild; danach `./tools/install-app.sh --check` (DIR-017) —
das LittleFS des S3 ist leer, die PWA muss dort **erstmals** hochgeladen werden.

## Schreibregeln für die Umsetzung

Die Spec ist Deutsch mit echten Umlauten und Schweizer `ss`. **Die Quelldateien sind es nicht.**
`src/**` und `ESP8266/ESP-uclock/*.cpp` sind überwiegend ASCII oder ISO-8859-1 (DIR-015); `http.cpp` und
`stm32flash.cpp` sind UTF-8. **Vor jedem Patch Kodierung und Zeilenende der Datei feststellen** (CLAUDE.md):
`ESP-uclock.ino` hat gemischte Zeilenenden, ein Edit-Werkzeug vereinheitlicht sie still. Neue Dateien für den S3
sind ASCII, Kommentare in Umschrift (`Bruecke`, `waehrend`).
