# Anforderungen — Portierung auf einen einzelnen ESP32-S3 (Variante B)

**Status:** Entwurf — zur Freigabe durch den Nutzer. Ohne Freigabe beginnt keine Implementierung.
**Stand:** 2026-10-17 (Nachträge des Nutzers vom selben Tag eingearbeitet: 1:1-Umbau, zwei Muss-Kriterien,
LED-Board unverändert, Wiederverwendung der LED-Logik, Leiterplattenformat 1:1; dazu die belegten Fakten aus den
Komponentenblättern der Datenblatt-Analyse). **Nachtrag vom 17.10.2026, zweiter Stand:** `R22` bleibt **1 kΩ** (E13, Vorgabe des Nutzers
zur 100-%-Kompatibilität; E9(a), H9, F15, AK17–AK17c und `design.md` §2/§4.2 umgestellt — der frühere Stand ist jeweils mit Datum benannt); IR-Empfänger und DCF77
haben auf dem Controller **keinen Stecker** (Abschnitt „IR-Empfänger und DCF77 — was 1:1 hier heisst“, F20, AK39); die Konsole `H6` ist auf eine mögliche **Vertauschung von Sende- und
Empfangsleitung** zu prüfen (F19, AK38). **Nachtrag vom 17.10.2026, dritter Stand:** Abgleich mit `specs/multiboard-quelle/` (Entwurf des `sw-architect`, nicht freigegeben): Die Widersprüche W1–W10 sind in `design.md` §12
mit „übernommen / verworfen / offen“ je Zeile beantwortet; die Entscheide Q1–Q13 jener Spec bleiben beim Nutzer, diese Spec **verweist auf sie und kopiert keine Zahlen** daraus
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
**Für den Nachtrag zu IR, DCF77 und `H6` gelesen:** `hardware/dokumentation/offene-punkte.md` (OP-35, OP-36, OP-02, OP-03), `hardware/dokumentation/vorgaben-des-nutzers.md`
und `hardware/komponenten/esp32s3-wroom-1.md` §6.2/§6.3. **Die Netzliste selbst (`v2.xml`) habe ich dafür nicht erneut gelesen**; die Aussagen zu `H6` stammen aus OP-35 und sind **nicht** als ✔ gesetzt.
**Für den Abgleich mit `specs/multiboard-quelle/` gelesen:** deren `requirements.md` (W1–W10, Q1–Q13), `tasks.md` und `design.md` (§1.3–§1.6, §3, §5, §7, §8, §9). **Deren Messungen habe ich nicht wiederholt**;
mit einer Ausnahme: `display.c` hat **6'738** Zeilen (nachgezählt am 17.10.2026 mit Grep auf jede Zeile; bis dahin stand hier 6'739, W5).

**Nicht gelesen für diese Spec:** das V2-PCB (`V2.kicad_pcb`, die Zahlen zum Format stehen im Skill und sind von
dort übernommen), die PDFs unter `hardware/komponenten/datenblaetter/`, das Komponentenblatt `tps2121.md`.
**Die Pinzuordnung unten ist deshalb ein Vorschlag, kein Befund.**

## Abstimmung mit anderen Specs und Rollen

- **`specs/multiboard-quelle/`** (Rolle `sw-architect`): Sie legt die **Architektur** fest — Naht zwischen Logik und Hardware, Verzeichnisse,
  Board-Beschreibung (`boards/<id>/board.h`), Klassifikation der Quelldateien (`boards/modules.txt`), Bau-Matrix und Hash-Baseline, Attrappen für die Hersteller-Header,
  Seam-Guards in den gemischten Bestandsdateien, **Framework (ESP-IDF oder Arduino-ESP32)** — „mit Preis". **Diese Spec legt das Framework nicht fest, sondern verweist
  darauf** (F12). Die Aufgaben des S3-Ports sind diese Spec; wo beide Specs dieselbe Arbeit beschreiben, **führt die Architektur-Spec aus, diese Spec verweist** (`design.md` §12, `tasks.md`).
  Das Verzeichnis `esp32s3/` ist dort das **IDF-Projekt des S3** (`multiboard-quelle`, `design.md` §7); Freigabe und damit Verbindlichkeit stehen aus. Widersprüche zwischen beiden Specs gehen an den Lead,
  sie werden nicht überschrieben. **Stand beim Nachtrag:** `requirements.md`, `design.md` und `tasks.md` liegen vor (Entwurf, **nicht freigegeben**); die Rolle `sw-architect` ist angelegt
  (✔ `.claude/agents/sw-architect.md`).
- **`hw-architect`** entscheidet die Pinzuordnung (Ablage `hardware/architektur/pinbelegung.md`, ✔ laut `sw-architect.md`; **die Datei liegt noch nicht vor**).
  **Pins stehen im Code an genau einer Stelle** (`boards/<id>/board.h`, die Signale in `board-pins.h`; `multiboard-quelle` Task 2.1) und werden **nicht** ein zweites Mal von Hand geführt. Das ersetzt das
  frühere `esp32s3/pins.h` dieser Spec (W2).
- **`hw-datasheet-analyst`** liefert die Komponentenblätter. Vorhanden und hier verwendet: `esp32s3-wroom-1.md` (Datenblätter v1.8/v2.2, Stand 17.10.2026),
  `sy8301abc.md`, `tps2121.md`.

## Problem

Heute besteht die Uhr aus zwei Rechnern, die über eine serielle Leitung als „Kommandobrücke"
verbunden sind (`HARDWARE.md`, Abschnitt „Beide Rechner können einander zurücksetzen"):

- **STM32F411** (`HARDWARE.md`: 84 MHz aus 25-MHz-Quarz) führt `src/**` aus: Display-Zustandsmaschine,
  LED-Ausgabe, Sensoren, EEPROM. Umfang der grössten Dateien ✔: `display.c` **6'738 Zeilen** (nachgezählt am
  17.10.2026; bis dahin stand hier 6'739, **nicht** 7'374; W5), `main.c` 5'378, `irmp.c` 5'114, `vars.c` 1'916.
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
(`HARDWARE.md`, „Der Stecker ist gespiegelt"). **Auch der LDR-Pfad auf dem Controller bleibt wie in V2:** `R22` behält 1 kΩ (E13).

**Die Anzeigelogik wird wiederverwendet, nicht neu gebaut.** Die LED-Ansteuerung hängt in `display.c` an
Treiberfunktionen hinter den Makros von `display.h` (`display.c` selbst benutzt davon nur drei, die **Schnittstelle** hat sechs — `multiboard-quelle` `design.md` §2 B4);
der S3 liefert diese Schnittstelle **mit derselben Signatur**, `display.c` und alle hardwarefreien Module bleiben **unverändert** (E11). Eine zweite Zustandsmaschine entsteht nicht.
**Wie** der S3-Treiber entsteht, ist durch `multiboard-quelle` neu gefasst: nicht als zweiter Treiber, sondern als **Backend des gemeinsamen Kerns** von `sk6812.c` (Nachtrag zu E11 unten).

**Was „kein Funktionsverlust" hier heisst, und die eine Spannung darin.** Verloren geht allein, was am
zweiten Chip hing und mit ihm entfällt (E6): das **Flashen des STM32 über die Uhr** und das **Zurücksetzen
des STM über den ESP**. Die Funktion „Neustart" bleibt erhalten (F10, `design.md` §6). Jede andere sichtbare
Funktion — PWA, Legacy-Oberfläche, Wetter, Ticker, Overlays, IR, DCF77, Ambilight, Backup — bleibt. Den
Nachweis führt AK21 plus AK30; Task 0.4 erstellt dafür ein **Funktionsinventar** (jede Funktion, die ein Nutzer
sieht, mit der Stelle, die sie trägt). **Für IR und DCF77 heisst „bleibt" auf dem Controller genau das, was V2 hat:** Controller-Pin mit Pull-up und die Firmware-Funktion, **ohne Stecker**
(Abschnitt „IR-Empfänger und DCF77 — was 1:1 hier heisst").

## Festgelegte Entscheidungen des Nutzers (nicht neu verhandelt)

| # | Entscheidung |
|---|---|
| E1 | Zielprodukt ist ein **Nachbau der Platine** mit dem **Modul ESP32-S3-WROOM-1**. Zusätzlich muss **dieselbe Software** auf dem DevKitC-1 v1.1 laufen |
| E2 | Daraus folgt: **nur GPIOs, die das DevKit am Header herausführt**, und **keine** der folgenden: GPIO0, 3, 45, 46 (Strapping), GPIO19/20 (natives USB), GPIO43/44 (UART0), GPIO35–37 (PSRAM/Flash der R8-Modelle), GPIO38 und 48 (RGB-LED des DevKits) |
| E3 | **Ein** USB-C-Anschluss mit Datenleitungen an GPIO19/20 (USB-Serial/JTAG): Flashen, Konsole, JTAG. Die Konsole läuft **zusätzlich** über UART0, damit der UART-Port des DevKits nutzbar bleibt. Der heutige 6-polige, reine Strom-Stecker wird ein 16-poliger mit D+/D−, **plus ESD-Schutz** |
| E4 | EEPROM bleibt der **AT24C32 am I²C**, **Datenformat unverändert**. RTC bleibt der **DS3231** |
| E5 | Die **Kommandobrücke bleibt als Schicht** (zwei Tasks mit Warteschlange statt UART), damit Protokoll und PWA unverändert bleiben. Die Display-Zustandsmaschine hat **genau einen Schreiber-Task**. `pending_ticker_restore` (CLAUDE.md) wird **nicht vereinfacht**. **Gemeint ist die Schicht `esp8266.c` mit dem Zeilenprotokoll; ersetzt wird nur ihre unterste UART-Schicht** (W10) |
| E6 | **Entfällt:** der STM32-Flash-Weg (`stm32flash.cpp`, `tools/flash-stm.sh`, DIR-010) |
| E7 | Reihenfolge der Umsetzung: **zuerst LED-Ausgabe auf dem S3 isoliert nachweisen, dann I²C/EEPROM/RTC, dann Sensoren, zuletzt Brücke und Task-Aufteilung** |
| E8 | **1:1-Umbau.** Kein Funktionsverlust, keine neuen Funktionen (Nachtrag vom 17.10.2026) |
| E9 | **Muss-Kriterien**, je mit eigenem Abnahmekriterium am Gerät: **(a)** der analoge LDR-Eingang (`GPIO1`, ADC1; die Normierung gegen `ldr_min_value`/`ldr_max_value` aus dem EEPROM **bleibt**; **`R22` auf dem Controller bleibt 1 kΩ** (E13); der ADC arbeitet bei 0 dB mit Mittelung und Werkskalibrierung; die Grenzwerte werden **einmalig nachkalibriert**, damit die Zählwerte gleich sind) und **(b)** **Flashen, Konsole und JTAG über den einen USB-C-Anschluss** (natives USB, GPIO19/20). **Bis 17.10.2026 stand hier in (a):** „`R22` auf dem Controller wechselt von 1 kΩ auf ca. 47 kΩ, weil der ESP32-ADC unten ungenau ist; die Kalibrierung wird neu gemessen" — zurückgenommen durch E13 |
| E10 | **LDR und LEDs bleiben auf dem LED-Board** und hängen unverändert über den gespiegelten Stecker `H7` am Controller. **Das LED-Board wird nicht geändert** — weder Bestückung noch Leiterbild noch Stecker. Änderungen betreffen **nur die Controller-Platine** |
| E11 | **Wiederverwendung der LED-Ansteuerungslogik so weit wie möglich.** Die Naht ist die **Treiber-API** (sechs Funktionen). Der S3 bekommt einen eigenen Treiber mit derselben API (RMT als Backend); **`display.c` und alle hardwarefreien Module werden unverändert übernommen**. Für die hardwarenahen Dateien entsteht eine **schmale Hardwareschicht mit den bisherigen Funktionsnamen**, damit der Rest nicht angefasst wird. **Keine Änderung an `display.c` ohne gesonderten Grund und Beleg.** Zuerst **nur** die Treiber-API auf dem S3, mit Abnahme am Gerät. **Nachtrag zum „eigenen Treiber": siehe unter der Tabelle** |
| E12 | **Das Leiterplattenformat bleibt 1:1** (Randbedingung, Tabelle unten): Umriss, Befestigungslöcher, Lage der Stecker und Taster wie in V2 |
| E13 | **Die LED-Ansteuerung, der Power-Safe und die LDR-Lesung bleiben 100 % pin- und funktionskompatibel zu V2.** Wörtlich, 17.10.2026: *„Die LED-Ansteuerung und der Power-Safe, wie auch die LDR-Lesung muss 100% Pin und Funktionskompatibel bleiben, da ich das gleiche LED-Board verwenden will"* (Fundstelle: `hardware/dokumentation/vorgaben-des-nutzers.md`, Nr. 5 und 14). **Folge: `R22` behält den V2-Wert 1 kΩ.** Jede Abweichung der LED-/Power-/LDR-Schnittstelle von V2 braucht die Freigabe des Nutzers; die Prüfung H8 (`tools/checks/hardware.py`) erzwingt `R22` = 1 kΩ |

**Nachtrag zu E11 (dritter Stand, aus `multiboard-quelle`, Freigabe der dortigen Spec steht aus).** E11 lautet im Kern: LED-Logik so weit wie möglich übernehmen, `display.c` unverändert. **Das Mittel dafür ist gemessen anders ausgefallen,
als E11 es formuliert:** Ein „eigener Treiber mit derselben API" kopierte den portablen Kern von `sk6812.c` (Doppelpuffer, `set_led`, `set_all_leds`, `refresh`-Rahmen, Zähler; Umfang siehe `multiboard-quelle` `design.md` §3.6) und liesse ihn
driften. `multiboard-quelle` löst es mit einem **Seam-Guard `WC_LED_BACKEND_EXTERNAL` in `sk6812.c`**: Der Kern bleibt **eine** Quelle für alle Boards, der S3 liefert nur das **Backend** (RMT-Sendung, Fertig-Rückruf, `sk6812_init`) als Text (`sk6812-backend.inc`
in `esp32s3/hal/`). Die Änderung an `sk6812.c` ist für alle Bestandsziele **byte-gleich nachzuweisen** (Klasse H2, Hash). **Folgen für diese Spec:** (1) Die S3-Task 2.1 schreibt das **Backend**, nicht einen zweiten Treiber, und hängt an `multiboard-quelle` Task 3.3 (F21); (2) AK6a bleibt wörtlich,
(3) „Stufe 2 berührt keine Zeile aus `src/**`" gilt **nicht mehr**: Berührt wird `sk6812.c` (Guard) — **nicht** `display.c` und **nicht** `main.c`. **Das ist eine Änderung am Wortlaut von E11, nicht am Willen des Nutzers („so weit wie möglich übernehmen"); der Nutzer bestätigt sie mit der Freigabe von `multiboard-quelle`.**
Bis dahin gilt E11 wie bisher geschrieben **und** F21 ist offen.

**Bis 17.10.2026 stand hier** (E9(a), dieser Absatz, H9, F15, AK17–AK17c, `design.md` §2 und §4.2): `R22` wechsle von 1 kΩ auf ca. 47 kΩ; die
Datenblatt-Analyse rechnete damals **zwei Paarungen** als brauchbar (10 kΩ mit 6 dB, 47 kΩ mit 12 dB), und die Messreihe sollte zwischen ihnen entscheiden. **Das gilt nicht mehr:** E13 schliesst die
Änderung von `R22` aus, weil dasselbe LED-Board weiterverwendet wird und `R22` im LDR-Pfad liegt.

**Zu E9(a), was die Datenblatt-Analyse zu `R22` = 1 kΩ festhält (✔K, `esp32s3-wroom-1.md` §6.2/§6.3):** Die Knotenspannung beträgt **dunkel 3,3 mV, Zimmer 157,1 mV, hell 300,0 mV** (gerechnet, identisch zu V2;
die LDR-Werte 1 MΩ / 20 kΩ / 10 kΩ sind **Annahmen** aus `HARDWARE.md`, ◌). Alle drei liegen im **0-dB-Bereich** (0 … 850 mV, Gesamtfehler ±5 mV, ✔K DS-C Tab. 5-6, S. 66). Das Dunkel liegt **unter** dem Gesamtfehler des S3 —
**beim STM32F411 von V2 aber ebenso** (±5 LSB ≙ ±4,0 mV, ✔K); `ldr.c` klammert dort auf `ldr_min_value`. Die Lesung ist nach dem Blatt **funktional gleichwertig, nicht zählergleich** (der S3 misst absolut, der F411
ratiometrisch). Mit **0 dB, Werkskalibrierung (eFuse), Mittelung über mindestens 16 Abtastungen** und der Abbildung **`N_STM = V_mV × 4095 / V_Schiene_mV`** (Schiene 3300 mV angenommen) bleiben die gespeicherten
Grenzen auf **±1 Helligkeitsstufe** gültig; eine **einmalige Nachkalibrierung** von `ldr_min_value`/`ldr_max_value` macht die Werte zählergleich (✔K §6.3 (5)). **Ob 1 kΩ bei 0 dB reicht, entscheidet die Messreihe AK17 am Gerät** (F15);
fällt sie durch, muss eine Abweichung **beim Nutzer beantragt** werden — sie wird nicht stillschweigend gewählt.

### Vorläufige Pinzuordnung — **Vorschlag, gegen den Schaltplan abzugleichen (Tasks 0.1a–c)**

| Signal | GPIO | bisher am STM32 (`HARDWARE.md`) | Peripherie am S3 | am DevKit-Header (✔K §3.2) |
|---|---|---|---|---|
| LDR (Abgriff vom LED-Board über `H7` Pin 5; Teilerwiderstand `R22` auf dem Controller, **1 kΩ wie V2**; bis 17.10.2026: „neu ca. 47 kΩ") | **GPIO1** | PA5 (✔ Netz `LDR` an `U2` Pin 15, `v2.xml`) | ADC1 (✔K: ADC1 = GPIO1–10), 0 dB | J3 |
| DS18B20 | **GPIO2** | PB5 | 1-Wire | J3 |
| LED-Daten | **GPIO5** | PB1 (TIM3 CC4 + DMA) | RMT TX, danach der bestehende Pegelwandler U3 | J1 |
| LED-Versorgung (`WS2812_POWER`) | **GPIO6** | PB0 | Ausgang → R19 → Q2 → Q1 | J1 |
| IR-Empfänger (**kein Stecker auf dem Controller**, F20) | **GPIO7** | PB4 | Eingang mit Pull-up (V2: `R8`/`R9`), Abtastung per Zeitgeber-ISR | J1 |
| I²C SDA | **GPIO8** | PB7 | I²C-Master (RTC und EEPROM) | J1 |
| I²C SCL | **GPIO9** | PB6 | I²C-Master | J1 |
| DCF77 (**kein Stecker auf dem Controller**, F20) | **GPIO15** | PB8 | Eingang mit Pull-up (V2: `R8`/`R9`) | J1 |
| Taster USER / WPS | **GPIO16 / GPIO17** | PA0 / PA7 | Eingänge | J1 |
| Status-LED | **GPIO18** | PC13 | Ausgang | J1 |

Gegenprobe gegen E2: Keiner dieser elf Pins steht auf der Ausschlussliste. **Neu belegt (✔K):** Alle elf liegen am Header des DevKitC-1 v1.1 (`J1`: 5, 6, 7, 8, 9, 15, 16, 17, 18; `J3`: 1, 2),
und **keiner trägt am DevKit eine Fremdbelegung** (belegt sind dort GPIO0 BOOT-Taster, EN Reset-Taster, GPIO38 RGB-LED, GPIO43/44 an der USB-UART-Brücke, GPIO19/20 am zweiten USB-Anschluss).
Das ist aus dem **Komponentenblatt** übernommen; den **Abgleich gegen den Schaltplan des Nachbaus** leisten die Tasks 0.1a bis 0.1c, und AK1 verlangt ihn.
**Die Pinzuordnung entscheidet `hw-architect`**; diese Spec übernimmt ihr Ergebnis und widerspricht ihm nicht, ausser wo die Software etwas nicht trägt.

### IR-Empfänger und DCF77 — was 1:1 hier heisst

**Bis 17.10.2026 stand hier:** Die Spec liess beide Signale schlicht „bleiben". **Das trägt die Netzliste nicht:** Der Controller hat nach V2 für beide **keinen Stecker**, nur die Pull-ups `R8`/`R9` (100 kΩ) und den
Controller-Pin (✔ Skill `kicad-schema`; `BEFUNDE.md` L491/E48; `hardware/dokumentation/offene-punkte.md` OP-36). Die Anforderung (**Spec-Setzung, aus E8 abgeleitet**, nicht wörtlich vom Nutzer):

1. **Der Controller-Pin bleibt, mit Pull-up wie V2:** GPIO7 (IR) und GPIO15 (DCF77), je ein Pull-up nach Schiene (welche Schiene: F3; welcher Pull-up gehört zu welchem Signal: gegen die Netzliste zu prüfen, Task 0.1c).
2. **Die Firmware-Funktion bleibt:** IR-Erkennung samt `learn_ir` (AK19) und DCF77-Dekodierung (AK20) arbeiten wie bisher.
3. **Einen Stecker gibt es nicht — und diese Spec fordert keinen.** „1:1 gegenüber V2" heisst für diese zwei Signale: kein Stecker. Ein neuer Stecker wäre eine Änderung am Format (E12) und eine neue Funktion (E8) und braucht
   eine Entscheidung des Nutzers (F20).
4. **Wo IR-Empfänger und DCF77-Modul angeschlossen werden, ist offen (F20).** Sie hängen **nicht** an `H7` (dessen Belegung ist 5 V, 3,3 V, LED-Daten, LDR, Masse — `HARDWARE.md`, H10, gegen die Netzliste zu prüfen) und bleiben im
   Format unverändert.

Am DevKit-Testaufbau (F8) hängen beide Empfänger am Steckbrett an GPIO7 und GPIO15 mit Pull-up; am Nachbau ist der Nachweis (AK19, AK20) nur über **Draht am Pin** möglich, solange F20 offen ist. AK39 prüft die Anforderung.

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
      **Die eine Pinquelle im Code ist `boards/<id>/board.h` (Signale in `board-pins.h`); ihr Abgleich gegen `pinbelegung.md` ist die Prüfung C5 / AK9 von `multiboard-quelle`** (W2) — diese Spec führt keine zweite Prüfung.
- [ ] **AK2 — Ein Binärabbild.** Die Firmware wird **einmal** gebaut. Dasselbe Abbild (gleiche SHA-256-Summe,
      vom Lead im Bericht genannt) läuft auf dem DevKit und auf dem Nachbau. Es gibt kein `#ifdef DEVKIT`
      und keine zweite Pinkonfiguration. Nachprüfbar: `grep -rn DEVKIT` über den S3-Quelltext ohne Treffer (`multiboard-quelle` AK10 prüft es für `esp32s3/` und `boards/`), zwei
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
      (GPIO19/20), UART-Anschluss ohne Kabel — **dieselben** Konsolenzeilen erscheinen im Terminal. Auf dem Nachbau ist (1) über **`H6`** möglich — **wenn** dort UART0 liegt und die Zuordnung der beiden
      Leitungen stimmt (F19, AK38; bis 17.10.2026 stand hier „nur (2) möglich, soweit dort kein UART-Stecker bestückt ist (F12, F17)") — und (2) über USB.
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
      `tempsensor` **leer** — **mit einer benannten Ausnahme: `src/eeprom/eeprom.c`** erhält den Zweig `BOARD_EEPROM_PAGE_SIZE` vor der Familienkette (`multiboard-quelle` `design.md` §9.3 E3, Klasse A, hash-gleich nachzuweisen);
      ohne ihn bricht `eeprom.c` auf dem S3 mit `#error EEPROM_PAGE_SIZE`. **Die Dateiliste kommt künftig aus `boards/modules.txt` (Klasse „Logik"), nicht aus dieser Handliste** (W3, `multiboard-quelle` AK7);
      bis diese Datei existiert, gilt die Liste hier. **Jede weitere Ausnahme** steht mit **gesondertem Grund und Beleg** (Compilermeldung, Messung, Zeile) in
      `design.md` §3.6, ist vom Nutzer freigegeben, **bevor** sie umgesetzt wird, und ist im Bericht einzeln benannt. **E3 steht dort bereits als beantragt, Freigabe ausstehend.**
      Die Prüfung nennt die **Zahl der betrachteten Dateien**; eine Prüfung, die nur eine Teilmenge sieht,
      meldet OK und sieht den Gegenstand nicht (CLAUDE.md, DIR-014, sechster Fall).

### Stufe 2 — LED-Ausgabe, isoliert (Tasks 2.x)

- [ ] **AK5 — Timing.** Am Signal hinter U3, **gemessen am Stecker `H7` Pin 4** (und, wo zugänglich, an
      `V1.DIN` hinter `R4` auf dem unveränderten LED-Board; `HARDWARE.md`: 550 Ω Serienwiderstand in Summe),
      mit Logikanalysator oder Oszilloskop, gilt für den Kanal mit den Sollwerten aus `sk6812.c`
      (✔ aktiver Satz: Periode 1250 ns, T0H 300 ns, T1H 750 ns, Pause 250 µs, `sk6812.c:91-97`; nach `multiboard-quelle` Task 3.3 stehen sie **einmal** in `sk6812-timing.h`): jede Zeit liegt innerhalb
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
      **Übersetzbarkeit gegen die originale `sk6812.h` ist nur mit den Attrappen erfüllbar** (W1): Sie zieht `misc.h` unbedingt und `stm32f4xx.h` hinter Familienmakros ein; ohne Attrappen übersetzt kein Logikmodul des S3-Baus.
      Die Attrappen (`misc.h`-Attrappe, die erzwungenen Einbindungen, ein Platzhalter für die Chipkennung bis zum Zweig in `main.h`) liefert das S3-Gerüst (Task 1.1 hier = `multiboard-quelle` Task 2.2);
      die Messung und die Zahlen stehen in `multiboard-quelle` `design.md` §3.2. **Der Kern (Doppelpuffer, `set_led`, `set_all_leds`, `refresh`-Rahmen, beide Zähler) steht einmal** (`multiboard-quelle` AK8): `grep -c` auf die Funktionsnamen
      in `src/` und `esp32s3/` zeigt **je eine** Definition.
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
      sie bestätigt sie. **Die Software schaltet `GPIO6` als allererste Anweisung der Anwendung auf Low** (`design.md` §4.5; im Bestand ist `power_init`/`power_off` der erste HAL-Aufruf, `multiboard-quelle` `design.md` §3.4); die Zeit davor (ROM, Bootloader) trägt allein die Platine.
      Schlägt die Messung fehl, ändert sich die Platine — nicht die Software. **Beachte E13:** Der Power-Safe-Pfad (`R19`–`R23`, `Q1`, `Q2`) bleibt wie V2; eine Platinenänderung dort ist eine
      **Abweichung** und braucht die Freigabe des Nutzers (Prüfung H8).
- [ ] **AK10 — Pegelwandler.** Das Signal an `GPIO5` erreicht die LED-Kette ausschliesslich über einen
      Pegelwandler mit TTL-Eingang (Nachbau: U3). Auf dem DevKit-Testaufbau ist er ein externes Bauteil gleicher
      Art (F8). **Kein Dauerbetrieb mit 3,3 V direkt an einer 5-V-Kette** (`HARDWARE.md`: grenzwertig). ✔K §5: Ein 3,3-V-Ausgang erfüllt VOH ≥ 0,8 · VDD = 2,64 V;
      ob das für den 74AHCT1G125 bei 5 V Versorgung reicht, steht im Datenblatt des Wandlers (**vom Analysten nicht gelesen**, F18).

### Stufe 3 — I²C, EEPROM, RTC (Tasks 3.x)

- [ ] **AK11 — EEPROM-Rundlauf.** Auf einem **Test-EEPROM** (nicht der Chip einer produktiven Platine) werden
      4'096 Byte mit einem Prüfmuster geschrieben und gelesen: 0 Abweichungen. Das Schreiben geschieht über die
      **unveränderte** Funktion `eeprom_write()` (Seitengrösse 32 Byte, `HARDWARE.md`; die Seitengrösse kommt auf dem S3 aus `BOARD_EEPROM_PAGE_SIZE` in `board.h`, `multiboard-quelle` E3).
- [ ] **AK12 — Schreibzeiten.** Ein Schreibzyklus über eine Seite kostet **≤ 20 ms**, 32 Byte über zwei Seiten
      **≤ 45 ms** (**Spec-Setzung**; Vergleichswerte am STM32: 15 ms bzw. 35–36 ms, `HARDWARE.md`,
      `BEFUNDE.md` L403). Die Zeiten stammen aus dem Mitschnitt des Treibers, nicht aus einer Schätzung.
- [ ] **AK13 — Datenformat unverändert.** `git diff` auf `src/eeprom/**`, `src/vars/**` und alle Dateien, die das
      EEPROM-Layout festlegen, ist **leer** (Layout-Dateien benennt Task 0.4) — **ausser** dem Zweig `BOARD_EEPROM_PAGE_SIZE` in `eeprom.c` (E3, AK35; er berührt kein Layout). Ein Wert, den die alte Firmware
      schreibt, liest die neue unverändert — nachgewiesen in AK29 über den Backup-Weg. **Ausnahme:** die
      LDR-Kalibrierwerte `ldr_min_value`/`ldr_max_value` sind **Werte**, keine Layoutänderung; sie werden mit
      der Abbildung auf die STM-Skala geprüft und **einmalig nachkalibriert** (AK17b) und überschreiben dann die Backup-Werte der alten Uhr.
      (Bis 17.10.2026 stand hier: „mit dem neuen `R22` neu gemessen" — `R22` bleibt 1 kΩ, E13.) **Die Grössen und Versätze aller Typen, die über EEPROM, Brücke oder Backup laufen, werden auf dem S3 per `_Static_assert`
      festgenagelt** (`multiboard-quelle` AK11, Prüfung C7): Ohne sie ist „Datenformat unverändert" nicht belegbar.
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
      Solldauer auf ± 0,1 % überein. **`F_INTERRUPTS` setzt der S3 aus `BOARD_TICK_HZ` in `board.h` per Übersetzer-Schalter** (das Makro steht hinter `#ifndef`, `multiboard-quelle` Q8) — ohne Quelltextänderung; **der Interrupt-Rumpf
      ist auf dem S3 derselbe Text wie im Bestand**, nur die Quittung ist ein Makro (`WC_TICK_ACK()`, Seam-Guard `WC_PLATFORM_EXTERNAL`).
- [ ] **AK17 — MUSS (a): ADC-Skala als Messreihe bei `R22` = 1 kΩ, 0 dB.** **Bis 17.10.2026 stand hier** eine Messreihe für 47 kΩ/12 dB (0,10 V bis 2900 mV, Toleranz „Gesamtfehler plus 1 %") mit der Folge „Fällt eine zählende Stufe
      durch, ist F15 (Wert von `R22`, Dämpfung) zu entscheiden"; sie ist durch E13 ersetzt. **Jetzt:** Der ADC-Treiber liefert denselben **Zählwertbereich wie heute**: 12 Bit, ein Zählwert **proportional zur Spannung am Pin, bezogen auf 3,3 V**
      (**`N_STM = V_mV × 4095 / V_Schiene_mV`**, ✔K §6.3 (2); ✔ `ldr.c:61-77` normiert eine geklammerte Kopie gegen `ldr_min_value`/`ldr_max_value` aus dem EEPROM; **`ldr.c` und die Normierung bleiben unverändert**, `git diff` leer).
      **Konfiguration:** 0 dB (benannte Konstante `BOARD_LDR_ADC_ATTEN_DB` in `board.h`), Werkskalibrierung (eFuse), **Mittelung über mindestens 16 Einzelmessungen** (Startwert 32, `design.md` §4.2).
      **Nachgewiesen wird mit einer Messreihe, nicht mit einem Einzelwert:** An `GPIO1` liegt der **echte Teiler** (3,3 V — Festwiderstand anstelle des LDR — Knoten — `R22` = 1 kΩ gegen Masse, damit die **Quellimpedanz** stimmt). Mit
      **mindestens acht** Widerstandsstufen von **dunkel (rund 1 MΩ, 3,3 mV) bis zum Bereichsende 850 mV** (Beispiel: 1 MΩ, 100 kΩ, 47 kΩ, 20 kΩ, 10 kΩ, 4,7 kΩ, 3,3 kΩ), **davon mindestens vier im Betriebsbereich bis 350 mV**. Je Stufe: Spannung am Knoten
      **und** Spannung der 3,3-V-Schiene mit Multimeter, **Mittelwert über 64 Messungen** des Treibers, Zeile in einer Tabelle Soll / Ist / Abweichung **in Zählwerten** (Soll = `V_mV × 4095 / V_Schiene_mV`).
      **Zulässige Abweichung je Stufe: der Gesamtfehler des S3 bei 0 dB plus 1 % des Messwerts**, also **±5 mV ≙ ±6,2 Zähler plus 1 %** (✔K DS-C Tab. 5-6: 0 dB, 0 … 850 mV, ±5 mV; **Spec-Setzung** für die 1 %).
      **Stufen, deren Spannung unter dem Gesamtfehler liegt (hier: das Dunkel mit 3,3 mV), werden eingetragen, aber nicht als „bestanden" gezählt** (✔K §6.1: unterhalb des Gesamtfehlers „nicht aussagekräftig"; **auf V2 ist es ebenso**, ET max ±5 LSB ≙ ±4,0 mV am F411).
      **Zählende Stufen: mindestens sieben.** Die Reihe läuft **mit und ohne** 100 nF am Pin, solange der Nutzer `C32` nicht entschieden hat (F15 Teil b, OP-03). **Das Ergebnis entscheidet, ob eine Abweichung beim Nutzer beantragt werden muss:**
      Besteht jede zählende Stufe und AK17c, ist F15 mit „1 kΩ bei 0 dB reicht" beantwortet, **und es wird nichts beantragt**. Fällt eine zählende Stufe durch oder verfehlt AK17b/AK17c die ±1 Stufe, gilt AK17 als **nicht bestanden**, und der Lead
      legt dem Nutzer die Abweichungen aus ✔K §6.3 **zur Entscheidung** vor (**zweiter ADC1-Pin zur Messung der Schiene**, damit ratiometrisch gerechnet werden kann — ein zusätzlicher Pin, nicht V2; oder **`R22` = 4,7 kΩ mit 6 dB** — ändert V2-Spannungen);
      **beide sind „nicht freigegeben"** (✔K), keine wird ohne Freigabe umgesetzt, und eine Freigabe wird als Eintrag in `tools/checks/hardware-led-schnittstelle.ausnahmen` festgehalten (Verfahren der Prüfung H8).
- [ ] **AK17a — MUSS (a): Teiler am LED-Board über `H7`.** Mit dem **unveränderten** LED-Board, dem
      Original-Kabel und `R22` = **1 kΩ** stimmen die an `GPIO1` gemessenen Spannungen mit der
      Rechnung aus `design.md` §4.2 (aus dem mit Multimeter gemessenen LDR-Widerstand; Erwartung nach den Annahmen aus `HARDWARE.md`: Zimmer rund 157 mV, hell rund 300 mV) auf **±10 %** (Bauteiltoleranz des LDR) überein, und zwar für drei
      Beleuchtungen (abgedeckt, Zimmerlicht, Taschenlampe nah); **das Dunkel (rund 3,3 mV) wird eingetragen und nicht gezählt** (unter dem Gesamtfehler, siehe AK17). `H7` Pin 3 liefert 3,3 V, Pin 5 den Abgriff;
      beide werden am Stecker gemessen, nicht am Bauteil — der **gespiegelte** Anschluss ist Teil der Prüfung
      (`HARDWARE.md`: Controller-Pin *n* gehört an LED-Board-Pin 8 − *n*). Das Netz `LDR` trägt auf V2 **zusätzlich**
      `H5.2` (✔ `v2.xml`); `H5` bleibt unbenutzt und unverändert (E12), darf aber den Abgriff **nicht belasten**.
- [ ] **AK17b — MUSS (a): Grenzwerte prüfen und einmalig nachkalibrieren.** **Bis 17.10.2026 stand hier:** „die alten Werte (Skala mit `R22` = 1 kΩ, rund 4 bis 373 Zählwerte) gelten nicht mehr, die Werte werden neu bestimmt" — das war die Folge von 47 kΩ.
      **Jetzt, bei `R22` = 1 kΩ:** (1) Mit den **Grenzwerten der alten Uhr** (vom Nutzer an der alten Uhr abgelesen; Beispielspanne laut `HARDWARE.md` rund 4 bis 373 Zählwerte, **die tatsächlichen Werte des Nutzers sind nicht bekannt**, OP-26) zeigt der S3 am
      selben LED-Board bei **fünf Beleuchtungen** eine Helligkeitsstufe, die von der der alten Uhr um **höchstens ±1 Stufe** abweicht (✔K §6.3 (5), gerechnet). (2) Danach **einmalige Nachkalibrierung** über den vorhandenen Weg im EEPROM
      (abgedeckt = Minimum, hellste vorgesehene Beleuchtung = Maximum); sie macht die Zählwerte gleich und ist **ein Schritt, kein Hardwareumbau**. Das Messprotokoll nennt je Beleuchtung: Spannung an `GPIO1` (Multimeter), Zählwert S3, Zählwert alte Uhr,
      Helligkeitsstufe beider, gewählten Grenzwert. **Wiederholung nach Neustart:** Die Werte stehen nach einem Reset unverändert im EEPROM.
- [ ] **AK17c — MUSS (a): Wirkung am Display.** Fünf Beleuchtungsstufen von dunkel bis hell ergeben eine
      **monoton steigende** Display-Helligkeit (die Uhr dimmt im Dunkeln, nicht umgekehrt). **Auflösung:** Zimmer (rund 157 mV) und hell (rund 300 mV) liegen **über dem Gesamtfehler** von ±5 mV (rund ±6 Zähler); das **Dunkel liegt darunter und wird von `ldr.c` auf
      `ldr_min_value` geklammert — wie auf V2** (gerechnet, ✔K §6.3 (3)); das Protokoll nennt es. **Bereichsende:** Bei 0 dB endet der Bereich bei 850 mV ≙ 1'055 Zählern (erst bei einem LDR unter rund 2,9 kΩ erreicht); der Rohwert, den die PWA zeigt
      (`LDR_RAW_VALUE_NUM_VAR`), ist oberhalb davon nicht mehr gültig, **das Protokoll nennt, ob der hellste vorgesehene Zustand das Bereichsende erreicht** (✔K §6.3 (2)). **Rauschen:** Bei konstantem Licht
      streut der gefilterte Zählwert über 60 s um **höchstens ±6 Zähler** (**Spec-Setzung**; rund ±0,5 Helligkeitsstufe bei der Beispielspanne von rund 369 Zählern auf 32 Stufen, gerechnet; ◌ der
      ADC rauscht, deshalb mittelt der Treiber, `design.md` §4.2; **das Rauschen ist in keiner Unterlage beziffert**, ✔K) — die Helligkeit **flackert** nicht. **Auch unter WLAN-Dauerlast** (siehe AK37), weil die Messung
      ✔K **nicht ratiometrisch** ist: Die Knotenspannung folgt der 3,3-V-Schiene, die ADC-Referenz nicht (Schiene ±2 % ≙ Zimmer ±3,9, hell ±7,4 Zähler, gerechnet); die Datenblattwerte gelten **mit abgeschaltetem WLAN** (✔K DS-C S. 66).
      Die Schienenspannung wird dabei mitgemessen und im Protokoll eingetragen.
- [ ] **AK18 — DS18B20.** Der Sensor liefert die Temperatur, die ein Referenzthermometer ±0,5 K bestätigt.
      Die Fehlersemantik aus A2 bleibt (✔ `CLAUDE.md`, Thema 1): Lese- oder CRC-Fehler liefern **255**, drei
      Fehler in Folge gelten als „verloren". Nachweis: Sensor abziehen — der Mitschnitt zeigt
      `ds18xx verloren`; Sensor stecken — `ds18xx erkannt`.
- [ ] **AK19 — IR.** Die vorhandene Fernbedienung wird erkannt: für fünf Tasten meldet die Uhr **Protokoll,
      Adresse und Befehl** gleich wie die alte Uhr (Vergleichswerte vom Nutzer, an der alten Uhr abgelesen).
      Anlernen über `learn_ir` funktioniert. Während eines `ping`-Dauerlaufs über WLAN fällt in 100 Tastendrücken
      **keiner** aus. **Am Nachbau hängt der Empfänger an keinem Stecker (F20); der Nachweis dort läuft über Draht am Pin `GPIO7` mit dem Pull-up** (AK39). **`irmp.c` bleibt im Algorithmus unverändert; der Pin-Zugriff und `irmp_init` liegen hinter dem Seam-Guard `WC_IRMP_TARGET_EXTERNAL`** (`multiboard-quelle` Task 3.4, hash-gleich).
- [ ] **AK20 — DCF77 und Taster.** Taster USER und WPS lösen je ein Ereignis aus (Mitschnitt). DCF77: Ein
      Signalgenerator mit gültigem Bitmuster (oder der angeschlossene Empfänger) ergibt innerhalb von **5 Minuten**
      eine gültige Zeit; ein gestörtes Muster ergibt **keine** Zeit. **Am Nachbau hängt auch das DCF77-Modul an keinem Stecker (F20);** Nachweis dort über Draht am Pin `GPIO15` mit dem Pull-up (AK39).
      **`dcf77_tick` ruft `log_message` und `board_led_*` aus der Timer-ISR** (✔ `multiboard-quelle` AK20): Auf dem S3 ist das **entweder ISR-fest** (Ringpuffer-Schreiben im IRAM) **oder** der Aufruf wandert in den Hauptloop (**Klasse F am Bestand**) —
      der Entscheid steht in `multiboard-quelle` Task 3.9, die Umsetzung in S3-Task 4.1; **er darf nicht offen bleiben**, bevor Stufe 4 beginnt.

### Stufe 5 — Brücke und Task-Aufteilung (Tasks 5.x)

- [ ] **AK21 — Brücke und Protokoll unverändert.** Eine Kommandofolge, die an der alten Uhr mitgeschnitten
      wurde (Vollabgleich mit rund 194 `var`-Kommandos, Ticker, Overlay-Import), läuft gegen die neue Brücke:
      alle Quittungen kommen, **0 Verluste**, die Antworten stimmen Zeile für Zeile bis auf die in `design.md`
      genannten Ausnahmen. `git diff` auf `ESP8266/ESP-uclock/data/app/**` ist **leer** — die PWA ist
      unverändert (`APP_VERSION` und `CACHE_NAME` bleiben). **`esp8266.c` mit dem Zeilenprotokoll bleibt** (E5, W10); ersetzt wird die unterste Schicht (die `esp8266_uart_*`-Funktionen der Instanz `esp8266` aus `uart.h`)
      und die Steuerpin-Naht `WC_PEER_EXTERNAL` (`multiboard-quelle` Task 3.7; das Verhalten auf dem S3: F10).
- [ ] **AK22 — Rückstau ohne Verlust.** Liest der Netz-Task absichtlich 2 s lang nicht, **zählt** die Brücke
      jedes verworfene Zeichen (Feld `d=` der Diagnosezeile, ✔ Name bleibt) — und **verwirft nichts ohne
      Zähler**. Der Display-Task steht dabei nicht (Δu der Diagnosefolge ≤ `DIAG_INTERVAL_SEC + 2 s`).
- [ ] **AK23 — Ein Schreiber.** Die Display-Zustandsmaschine (Zustände benannt in Task 0.4: `display`-Struktur,
      `display_clock_flag`, `pending_ticker_restore`, Ticker-, Icon- und Overlay-Zustand) wird **nur vom
      Display-Task** geschrieben — **ohne Änderung an `display.c`** (E11, AK35). Nachweis zweifach: (a) **Statisch, am gelinkten Abbild:**
      Die Netz-Komponente hat **keine** unaufgelöste Referenz auf ein Symbol der STM-Komponente und umgekehrt (`nm`); die Zahl der geprüften
      Symbole steht im Bericht (**diese Prüfung ist C9 / AK17 von `multiboard-quelle`**, Task 4.1 dort); (b) **Dynamisch, im Debug-Build, an den Übergängen:** Jede Funktion der **Hardwareschicht und der Brücke** prüft
      die Task-Kennung ihres Aufrufers (`display_task` für die STM-Seite, `net_task` für die Netz-Seite) und bricht bei Verletzung mit Meldung
      ab — und **bricht nachweislich ab**, wenn man eine Verletzung herstellt (DIR-014). **Begründung, warum nicht an den Schreibstellen:**
      Dort läge die Prüfung in `display.c`, und die bleibt unverändert.
- [ ] **AK24 — Restore-Bedingung.** Der Block `pending_ticker_restore && ! display_ticker_active () &&
      ! display.do_display_icon && show_icon_stop_time == 0 && show_overlay_idx >= MAX_OVERLAYS` ist im Quelltext
      **unverändert** (✔ heute `main.c:4910-4914`) — alle **vier** Teilbedingungen, nicht vereinfacht. **Die Prüfung C6 / AK12 von `multiboard-quelle` vergleicht den Text gegen eine Referenz nach jedem Task.** Dazu
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
      über die Versorgung des Nachbaus (F1). **Die Schiene geht beim LDR voll in den Zählwert ein** (AK17c, ✔K §6.3 (2)): Die hier gemessene Schienentoleranz ist die Eingangsgrösse der Fehlerrechnung in `design.md` §4.2.
      (Zahlen 3,0 V und 10 Minuten: Datenblatt beziehungsweise **Spec-Setzung**.)

### Stufe 6 — Abnahme

- [ ] **AK29 — Migration über Backup.** Der Nutzer exportiert ein Backup von der alten Uhr und importiert es
      am S3. Danach liefert `./tools/snapshot-device.sh` an beiden Geräten Abzüge, die sich **nur** in den von
      Natur aus veränderlichen Feldern unterscheiden (Uptime, LDR-Rohwert, Temperatur, Uhrzeit, Versionen,
      Geräte-ID) **und, falls nachkalibriert wurde, in den LDR-Kalibrierwerten** (AK17b). **Der Import löst allein der Nutzer aus**
      (CLAUDE.md, R5).
- [ ] **AK30 — Gerätedurchlauf.** Der `pwa-tester` fährt die Phasen 0 bis 4 und 9 von `TESTPLAN-PWA.md` am S3
      ohne **neuen** Befund (DIR-012: der Smoketest allein ist nicht der Test). Dazu das Funktionsinventar aus
      Task 0.4: **jede** dort genannte Funktion ist am S3 entweder nachgewiesen oder als „entfällt mit dem
      zweiten Chip" (E6) benannt — eine dritte Möglichkeit gibt es nicht (E8).
- [ ] **AK31 — Dauerlauf.** 72 Stunden Betrieb mit laufender Logwache (`./tools/watch-log.sh`, DIR-013),
      keine Exception, kein Neustart, kein Sprung in `d=`, keine Lücke in der `diag`-Folge.
      **Freispeicher:** der kleinste je gemeldete Wert fällt nach den ersten 10 Minuten um **höchstens 10 %**
      (**Spec-Setzung**; schliesst einen Speicherverlust aus, nicht eine kleine Reserve).
- [ ] **AK32 — Nachbau.** AK2, AK3, AK3a, AK3b, AK4, AK9, AK17 bis AK17c, AK30, AK37, AK38 und AK39 werden **auf dem Nachbau**
      wiederholt, mit demselben Abbild wie auf dem DevKit.
- [ ] **AK32a — LED-Board unverändert.** Das LED-Board ist **dasselbe Bauteil wie an der alten Uhr**, ohne
      Nacharbeit: derselbe Satz Platine und Stecker, in beiden Controller-Varianten steckbar (E10). Nachweis: der
      Nutzer steckt **dasselbe** LED-Board nacheinander an die alte Uhr und an den S3-Controller; beide zeigen
      dasselbe Bild (AK6) und denselben LDR-Verlauf (AK17c, ±1 Stufe). Kein Lötpunkt, kein Draht, kein geänderter Wert auf dem
      LED-Board.
- [ ] **AK33 — Bestehende Ziele unverändert.** `./tools/guardrails.sh --full` läuft mit Exit 0. Die Ziele
      STM32F411, STM32F103 und ESP8266 bauen weiter (der Umbau der Bestandsdateien ändert sie, bricht sie nicht), und die
      Legacy-Oberfläche der alten Hardware antwortet nach dem STM-Release wie zuvor (`smoke-device.sh`
      und ein `pwa-tester`-Durchlauf an der alten Uhr). **Der Nachweis für den STM-Teil ist der Hash-Vergleich aller Bestandsvarianten gegen die Baseline** (`multiboard-quelle` AK13); er gilt für die **Test-Uhr (F411)**
      zusätzlich als Gerätetest, wenn ein Task der Klasse F dabei war. **Für den F103 trägt allein der Hash**, solange es kein Prüfgerät gibt (`multiboard-quelle` Q3, W8).
- [ ] **AK34 — Neue Prüfungen haben einmal versagt (DIR-014).** Jede in dieser Spec neu gebaute Prüfung
      (Task-Kennungsprüfung, Logwache-Muster, Formatprüfung; **die Pin-, Symbol- und Unverändert-Prüfungen sind die Prüfungen C5, C9 und C1/C3 von `multiboard-quelle`** und dort mit ihren Gegenproben P1–P10 geführt) ist **einmal absichtlich
      verletzt** worden und hat angeschlagen — samt Weg der Meldung bis zum Empfänger. Dazu die Zählung: Die
      Pinprüfung betrachtet **elf** Signale, die Formatprüfung **14** Merkmale; zeigt eine eine andere Zahl, ist sie falsch.
- [ ] **AK36 — Leiterplattenformat 1:1 (E12).** Die Leiterplatte des Nachbaus hat dieselben **14 Merkmale** wie V2 (Tabelle
      „Randbedingung"): Umriss 145,00 × 39,00 mm, `H8`/`H9`/`H10`, `H1`/`H2`, `USB1`, `H7`/`H4`/`H5`, `H6`, `SW1`–`SW3` an den Koordinaten
      von V2 (Abweichung höchstens **0,01 mm**, **Spec-Setzung**: Rundung). Nachprüfbar durch ein Prüfskript der Hardware-Rollen, das Edge.Cuts
      und Footprint-Positionen beider PCB-Dateien vergleicht und die **Zahl der verglichenen Merkmale** nennt; es versagt einmal absichtlich
      (verschobene `H9`). Der Nachbau passt in dasselbe Gehäuse (Nutzer, am Gerät). **Zusätzlich:** Der 16-polige `USB1` passt in die
      Gehäuseöffnung des 6-poligen (F16).
- [ ] **AK38 — Konsole `H6`: kein Ausgang auf Ausgang (F19).** Zweistufig, **bevor** ein Adapter an den Nachbau gesteckt wird. **(1) Papier (Task 0.1c):** Das Gutachten von `hw-reviewer` nennt aus der Netzliste des Nachbaus, **welcher Controller-Pin an `H6` Pin 2
      und welcher an Pin 3 liegt**, und stellt beides der V2-Netzliste gegenüber (V2, nach `offene-punkte.md` OP-35: Netz `TX_UART` an PA10 = STM-**Empfang** auf `H6` Pin 2, Netz `RX_UART` an PA9 = STM-**Sendung** auf Pin 3; Netznamen aus Sicht des Steckers),
      mit der **Zahl der betrachteten Netze**. **Erwartet:** Wo V2 den **Empfang** des Controllers auf den Stecker legte (Pin 2), liegt am Nachbau **`RXD0` (GPIO44)**; wo V2 die **Sendung** legte (Pin 3), liegt **`TXD0` (GPIO43)**. **Weicht der Nachbau ab, ist AK38 nicht bestanden:**
      Entweder ändert sich die Platine, oder der Nutzer gibt die Abweichung ausdrücklich frei und verdrahtet den Adapter um (festgehalten in F19). **(2) Gerät, Nachbau (Task 6.6):** Ein **für V2 verdrahteter Adapter** (der vorhandene des Nutzers; seine Belegung nennt der Nutzer, sie ist
      **nicht belegt**) wird **ohne Umbau** an `H6` gesteckt; **vorher** misst der Nutzer mit Durchgangsprüfung am Stecker, dass der **Sendeausgang des Adapters nicht auf den Sendeausgang des Controllers** trifft. Danach erscheinen die Konsolenzeilen im Terminal (AK3a, Lauf 1).
      **Am DevKit ist dieses Kriterium nicht prüfbar** (das DevKit führt UART0 über die USB-UART-Brücke, nicht über `H6`) — es ist ein **Nachbau-Kriterium**.
- [ ] **AK39 — IR und DCF77 ohne Stecker: Pin mit Pull-up bleibt.** **(1) Papier (Task 0.1c):** Das Gutachten nennt aus der Netzliste des Nachbaus, **dass GPIO7 und GPIO15 je einen Pull-up tragen** (V2: `R8`/`R9`, 100 kΩ; die Zuordnung zu den zwei Signalen und die Schiene sind einzutragen, F3), und dass an
      diesen zwei Netzen **kein Stecker** hängt — **wie in V2**, mit der Zahl der betrachteten Netze. **Ein zusätzlicher Stecker an diesen Netzen ist eine Abweichung** (E8, E12) und braucht die Entscheidung des Nutzers (F20). **(2) Gerät:** Am DevKit-Testaufbau und am Nachbau (Draht am Pin) erfüllen AK19 und AK20 die
      Funktion der Pins; kein 5-V-Pegel erreicht je einen der beiden Pins (H7).

## Nicht Teil dieser Änderung

- **Das Boardlayout selbst.** Diese Spec fordert Beschaltung (siehe „Was die Software von der Hardware voraussetzt") und führt
  offene Fragen; die Hardware-Spezifikation und das Layout liegen bei den Hardware-Rollen unter `hardware/`.
- **Das Framework und die Verzeichnisaufteilung** (F12): Sie entscheidet `specs/multiboard-quelle/` (Empfehlung dort `design.md` §8.1, Entscheid Q5 beim Nutzer). Diese Spec legt sie nicht fest.
- **Die Klassifikation der Quelldateien, die Hash-Baseline, die Seam-Guards und die Prüfungen C1–C9**: Sie gehören `specs/multiboard-quelle/`. Diese Spec nutzt sie und baut sie nicht ein zweites Mal.
- **Das LED-Board** (E10): kein Wechsel von LED-Typ, Stecker, LDR, `R4`, `R5`–`R7`, `C116` oder Abblockung. Die
  Stücklisten-Unstimmigkeit bei `C116` (`HARDWARE.md`: 680 µF gegen 100 µF) bleibt **offen und unberührt**.
- **Der LDR-Pfad auf dem Controller** (E13): kein anderer Wert von `R22`, keine zusätzliche Last am Netz `LDR`, keine Änderung der Pfade von LED-Daten und Power-Safe (`U3`, `R11`, `Q1`, `Q2`, `R19`–`R23`). Jede Abweichung, auch die
  aus AK17 beantragte, braucht die **Freigabe des Nutzers**. **Ob `C32` (100 nF am Netz `LDR`) bestückt wird, ist offen** (F15 Teil b, OP-03).
- **Ein Stecker für IR-Empfänger oder DCF77-Modul** auf dem Controller (F20): V2 hat keinen, diese Spec fordert keinen. Ein neuer Stecker wäre eine Änderung am Format (E12) und eine neue Funktion (E8).
- **Die Belegung des vorhandenen Konsolenadapters.** Sie ist nicht belegt; diese Spec legt sie nicht fest, sondern fragt (F19).
- **Das Leiterplattenformat** (E12): kein anderer Umriss, keine verschobenen Löcher, Stecker oder Taster. Eine Änderung braucht eine
  eigene Entscheidung des Nutzers.
- **Eine zweite Display-Zustandsmaschine, ein neues Anzeige-Modul oder eine Kopie von `display.c` für den S3** (E11). Die
  vorhandene Logik wird übernommen.
- **Ein zweiter LED-Treiber mit eigenem Kern für den S3** (Nachtrag zu E11): Der Kern steht einmal; der S3 liefert das Backend.
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
| H7 | Eingänge IR und DCF77 sind **3,3-V-verträglich**. ✔K §5: Es gibt **keine Aussage „5 V tolerant"** und **keinen Grenzwert je Einzelpin**; behandle jeden GPIO als **höchstens 3,6 V** (Ableitung aus VIH max = VDD + 0,3 V, **keine** Herstellerangabe). Auf V2 haben beide **keinen Stecker**, nur 100-kΩ-Pull-ups (`R8`, `R9`) und den Controller-Pin (✔ Skill `kicad-schema`) — **so bleibt es** (Abschnitt „IR-Empfänger und DCF77", AK39); der Anschluss der Empfänger ist offen (F20) | F3, F20, AK39 |
| H8 | Die Stromversorgung liefert **mindestens 0,5 A** (✔K DS-M S. 27); die Spitze des Moduls beim WLAN-Senden beträgt **355 mA** (✔K Tab. 6-4); **22 µF + 0,1 µF am Modul**, **10 µF am Platineneingang** (✔K §8) | F1, AK37 |
| H9 | **`R22` auf dem Controller bleibt 1 kΩ** (✔ V2, E13; die Prüfung H8 in `tools/checks/hardware.py` erzwingt es). **Bis 17.10.2026 stand hier:** „wechselt von 1 kΩ auf einen Wert nach F15 (Startwert ca. 47 kΩ, E9)". Der ADC-Eingang `GPIO1` erhält **keinen** zusätzlichen Kondensator, solange der Nutzer `C32` nicht freigegeben hat: V2 hat keinen (✔K §6.3 (4)); ✔K HDG S. 18 empfiehlt **0,1 µF**, und die Datenblattwerte gelten mit 100 nF; gerechnet ändert er den Gleichwert nicht und fügt einen Tiefpass mit τ ≈ 91–100 µs hinzu (**Entscheid des Nutzers**, F15 Teil b, OP-03) | AK17, AK17c, F15 |
| H10 | Der Stecker `H7` bleibt **in Belegung und Spiegelung unverändert** (E10): Pin 1/2 5 V, 3 3,3 V, 4 Daten, 5 LDR, 6/7 GND (`HARDWARE.md`; gegen die Netzliste zu prüfen, Task 0.1c). Die 3,3 V an Pin 3 kommen vom Rail, den der SY8301 liefert (F1) | AK17a, AK32a |
| H11 | **Das Leiterplattenformat bleibt 1:1** (E12, Tabelle „Randbedingung") | AK36 |
| H12 | **Nur ADC1** für den LDR (✔K §6.1: ADC2 teilt sich den ADC mit dem WLAN); GPIO1 ist ADC1 | AK17 |
| H13 | Die Konsole `H6` ist **funktional wie V2** belegt: **Pin 2 = Controller-Empfang (`RXD0`, GPIO44), Pin 3 = Controller-Sendung (`TXD0`, GPIO43)**, damit ein für V2 verdrahteter Adapter nicht Ausgang auf Ausgang trifft (OP-35; **ob V3 es so hat, ist nicht belegt**) | F19, AK38 |

## Offene Fragen — nicht geraten

Jede Frage nennt, **wer sie beantwortet**, **woran** man sie beantwortet und **was bis dahin gilt**. Die Komponentenblätter beantworten einige Fragen **teilweise**; wo das der Fall ist, steht
es in der Frage. Quelle der Zahlen ist das Komponentenblatt (✔K), nicht mein Lesen des Datenblatts. **Die Fragen Q1–Q13 von `multiboard-quelle` beantwortet der Nutzer dort; wo sie diese Spec berühren, steht der Verweis in der betroffenen Zeile (F10, F12, F13, F14, F21) und gesammelt in `design.md` §12.**

| # | Frage | Beantwortet durch | Bis dahin |
|---|---|---|---|
| **F1** | **SY8301 (U10): trägt er den S3?** **Teilweise beantwortet (✔K `sy8301abc.md` §3, §4.3):** Ausgang **1 A**, Strombegrenzung 1,6–2,5 A; das Modul braucht eine Quelle von ≥ 0,5 A, **Spitze 355 mA** beim Senden; bei 600 mA Gesamtlast bleiben **mindestens 250 mA Reserve**. **Offen und kritischer: die Spannungsreserve bei knapper 5-V-Schiene** (FB hinter der Diode `D2`): Das Blatt rechnet bei 4,5 V Eingang und 0,6 A rund **3,0 V** am Verbraucher, bei 4,75 V rund 3,2 V (Rechnung, **Datenblatt schweigt zum Verhalten am Tastgradanschlag**) und nennt den Brownout als Risiko. Zusatz: `F1` (PTC 1,5 A im USB-Pfad) und `F2` (3 A) sind kleiner als 3 A LED + 0,6 A (nur aus Bezeichnungen gelesen) | `hw-architect` (Strombudget); **Messung AK37** an Prototyp und Nachbau | Kein Nachbau bestellt, bevor AK37 am DevKit-Verhalten des Moduls vorliegt; **AK37 am Nachbau entscheidet** |
| **F2** | **Pull-down am Gate von Q2 und Pin-Zustand beim Einschalten.** **Beantwortet:** Auf V2 existiert `R20` (100 kΩ) zwischen Gate und Masse (✔ `v2.xml`). ✔K §4.2: GPIO1–18 sind im Reset **hochohmig** und geben beim Einschalten **60 µs Low** aus (GPIO18 zusätzlich High); ein Low-Glitch ist für den N-Kanal-Q2 mit Pull-down **harmlos**. **Offen:** ob der Nachbau `R20` übernimmt; das Blatt nennt **GPIO21** als sauberen Kandidaten für ein solches Gate (hochohmig im Reset, kein Glitch, kein Strapping) — Entscheid `hw-architect`. **Beachte E13:** Der Power-Safe-Pfad bleibt wie V2; ein anderer Pin für `WS2812_POWER` ist eine Abweichung | `hw-architect`; Messung AK9 | GPIO6 bleibt der Vorschlag; LED-Versorgung am Testaufbau **nur** über einen Handschalter |
| **F3** | **5-V-Toleranz von IR (GPIO7) und DCF77 (GPIO15).** **Beantwortet (✔K §5):** keine Herstellerangabe „5 V tolerant", **kein Grenzwert je Pin** — behandle jeden GPIO als **höchstens 3,6 V**. **Offen:** Auf V2 haben beide **keinen Stecker**, nur `R8`/`R9` (100 kΩ) und den Controller-Pin (✔ Skill); **an welche Schiene die Pull-ups gehen und woher die angeschlossenen Empfänger gespeist werden, ist nicht belegt** (der Anschluss selbst: F20). Die Eingänge des STM32F411 sind an den fraglichen Pins ◌ 5-V-tolerant | `hw-architect` mit Netzliste (Task 0.1b); Datenblätter von TSOP und DCF77-Modul liefert der Nutzer | Testaufbau speist beide aus 3,3 V. Ein 5-V-Signal an einen S3-Pin ist ein **Hardware-Fehler**, kein Softwarefall |
| **F4** | **Antennenfreihaltung.** **Zahlen beantwortet (✔K §8, HDG):** Antennenfläche 6 mm tief, seitlich ≥ 15 mm frei, im Gehäuse ≥ 15 mm Abstand; Modul am Rand, **nicht in der Platinenmitte**; USB- und UART-Leitungen weit weg. **Offen:** ob das **im unveränderten Umriss** (E12) an der Stelle von `U4` (127,00 / 12,25, Antenne an der Oberkante) geht, mit den Nachbarn dort (`H6` bei 103,75 / 4,50) und im Gehäuse | `hw-architect`, `hw-reviewer` am Layout; Empfangsmessung (RSSI) am fertigen Gerät | Keine Aussage zur Reichweite |
| **F5** | **Genaue GPIO-Liste am DevKit-Header.** **Beantwortet (✔K §3.2, DK Tab. J1/J3):** Alle 36 Modul-GPIOs liegen am Header; **alle elf Pins des Vorschlags** liegen dort, keiner mit Fremdbelegung (GPIO38 = RGB-LED bei v1.1, GPIO0 BOOT, GPIO43/44 an der USB-UART-Brücke, GPIO19/20 am zweiten USB). Das **DevKit mit N8R8** sperrt zudem GPIO35–37 (✔K §3.3), E2 schliesst sie schon aus | `hw-reviewer` bestätigt am Nachbau-Schaltplan (Task 0.1c) | Gilt als **abgeglichen gegen das DevKit**, **ungeprüft gegen den Nachbau** |
| **F6** | **Zielmodul-Variante.** Das Komponentenblatt führt **ESP32-S3-WROOM-1-N8** (8 MB Quad-Flash, **kein PSRAM**, alle 36 GPIOs frei, ✔K §1) als **gewählt**, **N8R2** als Zweitwahl; JLCPCB-Bestand N8: 416 Stück am 17.10.2026 (**Beschaffungsrisiko**, ✔K §10). N8R8/N16R8 sperren GPIO35–37 | Nutzer bestätigt N8 | Der Entwurf rechnet mit **8 MB Flash** (jetzt ✔K, nicht mehr nur Annahme) |
| **F7** | **Pull-ups auf der Platine.** I²C (SDA/SCL), 1-Wire (DS18B20) und Taster: sind externe Pull-ups bestückt oder tragen heute die internen des STM32 (`HARDWARE.md`: „Entprellung und Pull-ups der Taster nicht geprüft")? Interne S3-Pull-ups (✔K typ. 45 kΩ) sind für I²C bei 400 kHz und für 1-Wire ◌ **zu schwach** | `hw-architect` mit Netzliste (Task 0.1b) | Testaufbau mit externen Pull-ups 4,7 kΩ an SDA, SCL und DQ |
| **F8** | **DevKit-Testaufbau.** Das DevKit hat weder U3 noch Q1/Q2 noch Sensoren. Wer baut den Aufbau: externer 74AHCT125 für die LED-Daten (AK10), EEPROM-Breakout (nicht produktiv, AK11), DS3231-Modul, DS18B20, TSOP, Taster, **LDR-Teiler mit dem echten LED-Board und `R22` = 1 kΩ auf dem Steckbrett (AK17a)**? | Nutzer | Stufen 2 bis 4 sind **blockiert**, bis der Aufbau steht |
| **F9** | **Toleranz der SKC6812-Zeiten.** Das Datenblatt der SKC-Variante (`HARDWARE.md`: nur Spannung, Ruhestrom, Datenrate) nennt keine Toleranz, die sich aus dem Repo ergibt. AK5 setzt ±150 ns | Nutzer: Datenblatt OPSCO SKC6812 (liegt **nicht** unter `hardware/komponenten/datenblaetter/`) | **±150 ns** als Setzung |
| **F10** | **Was geschieht mit den STM-Endpunkten und der PWA-Oberfläche dafür?** Heute: `/api/remote_stm32_flash`, `stm32_default` in `/api/update_status`, `/api/maintenance_reset_stm32` (✔ 18 Treffer dieser Namen in `app.js`). Empfehlung des Entwurfs: Endpunkte bleiben, antworten mit einem **definierten Fehlercode** („nicht unterstützt"); die PWA wird in einer **Folgespec** angepasst. `maintenance_reset_stm32` setzt am S3 den **ganzen Chip** zurück (nicht den Display-Task) — Gründe in `design.md` §6. **Das ist die einzige Stelle, an der „kein Funktionsverlust" nicht wörtlich gilt** (siehe „Ziel"). **Dieselbe Frage stellt `multiboard-quelle` als Q11 für die Steuerpin-Funktionen** `esp8266_reset/_powerdown/_powerup/_flash` in `esp8266.c`: Auf dem S3 gibt es keinen zweiten Chip, wie sie antworten, regelt diese Frage | Nutzer: Entscheidung | Die Empfehlung gilt; der Seam-Guard `WC_PEER_EXTERNAL` schneidet nur, er entscheidet das Verhalten nicht |
| **F11** | **Bleiben die alten Ziele baubar?** Annahme: **ja**, die Uhr im Keller und die produktiven Uhren laufen weiter mit STM+ESP8266 | Nutzer | Annahme gilt; sie bestimmt die Nähte in den Bestandsdateien (gemeinsamer Quelltext statt Fork) |
| **F12** | **Framework, Build-System, Verzeichnis, Besitz.** **Entscheidet `sw-architect` in `specs/multiboard-quelle/`, mit Preis** (ESP-IDF oder Arduino-ESP32). **Stand:** Die Empfehlung steht dort (`design.md` §8.1: ESP-IDF 5.5.x mit Arduino-ESP32 als Komponente, Preis und Alternativen genannt); **entschieden ist es nicht — Q5 beim Nutzer**, und `idf.py` ist auf dem Rechner nicht installiert (dort Task 0.6). Diese Spec **verweist darauf und legt nichts fest**; `design.md` §1.1 trägt meine Beobachtungen als **Eingabe**. Die Konsolenkonfiguration (AK3a) hängt am Framework (✔K §9). **Kein heutiger Agent besitzt `esp32s3/`**, der Besitz-Hook kennt es nicht; die Rolle legt allein der Lead an (dort Task 0.3). `H6` bleibt im Format (E12); ob und wie seine zwei Leitungen am Nachbau belegt sind, klärt F19 (bis 17.10.2026 stand hier: „Ob der Nachbau einen UART0-Stecker erhält, ist offen (F17)") | `sw-architect` (Architektur), Nutzer (Q5), Lead (Rolle, Hooks, Installation), `hw-architect` (Stecker) | **Task 1.1 beginnt nicht**, bevor `multiboard-quelle` freigegeben ist und das Framework festliegt |
| **F13** | **Versionierung und Tag.** `src/main.h` (`VERSION`) und `version.h` (`ESP_VERSION`) hängen heute an zwei Chips; das S3-Abbild enthält **beide Quellen plus `esp32s3/**`**. **Bis 17.10.2026 stand hier die Empfehlung:** „beide Symbole bleiben und werden gemeinsam geführt". **Sie ist zurückgenommen:** Ändert sich nur `esp32s3/**`, ändert sich weder `VERSION` noch `ESP_VERSION` (DIR-004), das Fabrikat wäre unzuordenbar; `ESP_VERSION` mitzuführen zwänge jeden STM-Commit zu einem ESP8266-Bump und böte ein OTA auf identische ESP8266-Firmware an (`multiboard-quelle` W7, Q2: Empfehlung dort ein **drittes Symbol** im selben `#define`-Format, Tag-Schema offen). Auch die **Versionspflicht bei nachweislich hash-gleicher Umstellung der Bestandsboards** ist dort offen (Q9) | Nutzer (`multiboard-quelle` Q2, Q9); `sw-architect`; `release-engineer` | **Keine Annahme.** Der S3-Release (Task 6.3) ist bis zum Entscheid blockiert; **DIR-004** gilt weiter, bis zum Entscheid **wird gebumpt** (Q9) |
| **F14** | **`F_INTERRUPTS` am S3.** 15'000 Hz teilen sich aus den Zeitgebertakten des S3 nicht ganzzahlig (◌ 80 MHz / 15'000 = 5'333,3). Kandidaten, die sich exakt teilen: 16'000 und 20'000 Hz. IRMP-Konstanten leiten sich aus `F_INTERRUPTS` ab (✔ `irmp.c:58` u. a.). **Das Makro steht hinter `#ifndef` (`irmpconfig.h`) und ist per Übersetzer-Schalter setzbar**, der S3 nimmt den Wert aus `BOARD_TICK_HZ` in `board.h` (`multiboard-quelle` Q8) — **kein Eingriff in `src/`** | `stm-developer` und `firmware-analyst` (Task 0.4); Nutzer (Q8) | Entwurf rechnet mit 20'000 Hz; AK16 und AK19 entscheiden; Bestandsboards bleiben 15'000 |
| **F15** | **Reicht `R22` = 1 kΩ bei 0 dB?** **Bis 17.10.2026 stand hier** die Frage „welcher Widerstand, welche Dämpfung" mit den Kandidaten 10 kΩ/6 dB und 47 kΩ/12 dB (Startwert 47 kΩ / 12 dB, 100 nF); sie ist durch E13 überholt. **Jetzt (✔K §6.3, gerechnet):** Mit 0 dB, Werkskalibrierung, Mittelung ≥ 16 und `N_STM = V_mV × 4095 / V_Schiene_mV` ist die Lesung **funktional gleichwertig, nicht zählergleich**; die Grenzen bleiben auf **±1 Stufe** gültig, eine **einmalige Nachkalibrierung** macht sie zählergleich. **Offen, in keiner der vier Unterlagen belegt (✔K §6.3, OP-26):** das **Rauschen** des S3-ADC mit und ohne WLAN, die **Linearität unter rund 5 mV**, die zulässige **Quellimpedanz** und Abtastkapazität, die **Toleranz der 3,3-V-Schiene** des Nachbaus (geht voll in den Zählwert ein), die **tatsächlichen Grenzwerte** des Nutzers, echte LDR-Werte (1 MΩ / 20 kΩ / 10 kΩ sind Annahmen). **Teil b:** Darf `C32` (100 nF am Netz `LDR`) bestückt werden? V2 hat keinen; gerechnet ändert er den Gleichwert nicht; ob das unter „100 % kompatibel“ fällt, entscheidet der Nutzer (OP-03; die Prüfung H8 meldet ihn als HOCH). **Hinweis an `multiboard-quelle`:** Dort steht `BOARD_LDR_ADC_ATTEN_DB` noch als „6 oder 12 (offen)" und AK17b als „neu gemessen" (`design.md` §3.5.3, §5.3) — das ist durch E13 überholt und **dort nachzuziehen (0 dB)** | **Messreihe AK17 am DevKit-Testaufbau** (Steckbrett-`R22` = 1 kΩ); Nutzer bei einer beantragten Abweichung und für `C32`; `sw-architect` für den Hinweis | `R22` = 1 kΩ, 0 dB, Mittelung 32; Dämpfung und Mittelungszahl sind **benannte Konstanten**. **Besteht AK17, ist die Frage mit „ja" beantwortet.** Besteht er nicht, entscheidet der Nutzer über die Abweichungen aus AK17 (zweiter ADC1-Pin oder `R22` = 4,7 kΩ/6 dB, beide „nicht freigegeben") |
| **F16** | **Hat der 16-polige USB-C-Stecker dieselben Aussenmasse wie der 6-polige von V2?** Heute „TYPE-C 6P", Footprint `TYPE-C-SMD_TYPE-C-6P` (✔ `v2.xml`). Der Skill `kicad-schema` hält fest: „**nicht belegt**"; das Komponentenblatt des Moduls behandelt den Stecker nicht. Betrifft Bauhöhe, Breite, Lage der Öffnung und damit das Gehäuse (E12) | `hw-datasheet-analyst`: Komponentenblatt der beiden Stecker mit Massen aus den Datenblättern (Task 0.1a); `hw-architect` entscheidet vor Layoutbeginn | **Layout beginnt nicht**, bevor die Frage beantwortet ist. Die Software ist davon unberührt |
| **F17** | **Wo sitzen BOOT- und EN-Taster, wenn das Format 1:1 bleibt?** Auf V2 sind `SW1`–`SW3` an drei Positionen belegt (✔ Skill; `SW3` setzt heute den STM zurück). Der S3 braucht **EN** (Reset) und **BOOT an GPIO0** (✔K §7: einziger Rückweg, AK3 Rückfall 2). `SW3` kann EN werden; der BOOT-Taster braucht **eine zusätzliche Stelle** oder eine Doppelbelegung von `SW1`/`SW2` — **GPIO0 darf nach E2 keine Anwendungsfunktion haben** | `hw-architect` | AK3 Rückfall 2 ist ohne BOOT-Taster **nicht nachweisbar**; am DevKit stehen beide Taster zur Verfügung |
| **F18** | **74AHCT1G125 mit 3,3-V-Eingang des S3.** Die Eingangsschwelle bei 5 V Versorgung steht im Datenblatt `sn74ahct1g125.pdf`; der Analyst hat es **nicht gelesen** (✔K §5, §14). Gegenüber steht VOH ≥ 2,64 V (✔K) | `hw-datasheet-analyst` (Task 0.1a) | V2 betreibt den Wandler mit dem STM32 (3,3 V) — der Wandler ist **bewährt**, aber mit dem **S3** nicht belegt |
| **F19** | **Konsole `H6`: Sende- und Empfangsleitung gegenüber V2 vertauscht?** Nach `hardware/dokumentation/offene-punkte.md` OP-35 hängt in V2 das Netz `TX_UART` an PA10 (STM-**Empfang**) auf `H6` Pin 2 und `RX_UART` an PA9 (STM-**Sendung**) auf Pin 3 — **Netznamen aus Sicht des Steckers**, also des Adapters. V3 ordnet nach dem Namen zu: `TXD0` (Sendeausgang, GPIO43) am Netz `TX_UART`, also an `H6` Pin 2 (Regel `nur_an_netzen` in `hardware-pinregeln.json`). **Stimmt das, wäre es gegenüber V2 vertauscht:** Ein für V2 verdrahteter Adapter träfe dann **Ausgang auf Ausgang**. Zu klären: (a) die richtige Zuordnung am Nachbau (**erwartet: Pin 2 = `RXD0`/GPIO44, Pin 3 = `TXD0`/GPIO43**, funktional wie V2); (b) wie der **vorhandene Adapter** verdrahtet ist (**nicht belegt**; die Pinbeschriftung an `H6` ist in den Symbolen nur numerisch). **Das ist bis jetzt nicht geprüft und nicht entschieden** | (a) `hw-architect`/`hw-designer` am Schaltplan, geprüft von `hw-reviewer` (Task 0.1c); (b) **Nutzer** | Die Konsole wird bis zur Antwort **nur über USB** angeschlossen, **kein Adapter an `H6`**. Nachweis: AK38 |
| **F20** | **Wo werden IR-Empfänger und DCF77-Modul angeschlossen?** Nach der V2-Netzliste hat der Controller dafür **keinen Stecker**, nur `R8`/`R9` und den Controller-Pin (✔ Skill `kicad-schema`; `BEFUNDE.md` L491/E48; OP-36). **Nicht belegt:** ob beide Geräte überhaupt an dieser Platine hängen, woher sie gespeist werden und wie sie verbunden sind (Lötpads, Draht, Stiftleiste, anderer Stecker). Sie hängen **nicht** an `H7`. Zu entscheiden: (a) Wie hängen sie heute an der Uhr? (b) Soll der Nachbau dafür etwas vorsehen? Das wäre **mehr als V2** (E8) und im unveränderten Format zu lösen (E12) | **Nutzer** (er liefert auch die Datenblätter von TSOP und DCF77-Modul); `hw-architect` für die Umsetzung | Es gilt die Anforderung „Controller-Pin mit Pull-up und Firmware-Funktion bleiben, **kein Stecker**" (AK39); Testaufbau am DevKit auf dem Steckbrett, am Nachbau Draht am Pin |
| **F21** | **Reihenfolge: LED-Ausgabe zuerst (E7) gegen Baseline-Pflicht der Architektur.** E7 verlangt das erste Licht auf dem S3 vor allem Übrigen. `multiboard-quelle` macht das Backend zum Teil des gemeinsamen Kerns; der Guard in `sk6812.c` (dort Task 3.3) ist eine Änderung am Bestand und darf erst nach der **reproduzierbaren Baseline** und ihrer Gegenprobe kommen (dort Stufe 0–1, Tasks 1.5/1.6), und 3.3 hängt an 3.1 und 3.2 (`tasks.md` dort). **Der S3-Stufe 2 geht damit Arbeit am Bestand voraus**, die E7 so nicht vorsah. Die Architektur-Spec sagt selbst: Ein Testprogramm **ohne** `sk6812.c` ist möglich, ist aber **kein** Nachweis des gemeinsamen Kerns (`tasks.md` dort, Abschnitt „Tor G0"). **Zu entscheiden:** (a) Stufe 2 wartet auf 3.3 (verzögert das erste Licht, belegt den Kern), oder (b) ein Vorab-Testprogramm ohne gemeinsamen Kern darf früh laufen und wird später durch den Kern ersetzt (E7 gewahrt, Nachweis AK6a/AK8-multiboard erst später) | Nutzer; Lead (R3b: die Abhängigkeitsspalte entscheidet über Parallelität) | **(a)**: Task 2.1 wartet auf `multiboard-quelle` Task 3.3. (b) nur mit ausdrücklicher Freigabe |

## Betroffene Laufzeiten

- [x] **ESP32-S3** (neu) — **Erst-Flash** über USB, kein OTA aus einem alten Stand möglich
- [x] STM32 (`src/**`) — **Änderungen an Bestandsdateien** (Seam-Guards, E1/E3 in `main.h`/`eeprom.c`, Zeiten in eine Kopfdatei) durch `multiboard-quelle` Tasks 3.1–3.8; **bei nachgewiesener Hash-Gleichheit aller Bestandsvarianten ist das Fabrikat dasselbe Abbild, und es gibt nichts zu flashen** (dort Q9). **Neu-Flashen nötig nur, wenn ein Task der Klasse F dabei ist.** `display.c` bleibt unberührt
- [x] ESP8266 (`ESP8266/ESP-uclock/*.cpp`) — **Neu-Flashen nötig**, einmal: Task 5.3 (Quelltext gemeinsam nutzbar machen)
- [ ] PWA (`data/app/**`) — **nicht berührt**; AK21 prüft es
- [x] Build/Release — neues Target, Release-ZIP enthält ein weiteres Abbild, `deploy.sh`-Serverlayout, Hooks,
      `guardrails.sh`, `smoke-device.sh`, `install-app.sh` (Prüfung), neues Flash-Skript (Task 6.2)
- [x] **Hardware** — Nachbau der **Controller-Platine** im **unveränderten Format** (Nutzer, Hardware-Rollen), Testaufbau am DevKit (F8).
      **LED-Board: nicht berührt.** **LDR-Pfad auf dem Controller: nicht berührt (`R22` = 1 kΩ, E13)**

**Flash-Reihenfolge für das Gesamtpaket** (CLAUDE.md, „Firmware zuerst"): Die alten Ziele erhalten ihre Nahtstellen-Releases
(Tasks 5.1r und 5.3r, **5.1r nur falls nicht hash-gleich**), **bevor** Task 5.4 beginnt. Auf dem S3 selbst gibt es nur **ein** Abbild; danach `./tools/install-app.sh --check` (DIR-017) —
das LittleFS des S3 ist leer, die PWA muss dort **erstmals** hochgeladen werden.

## Schreibregeln für die Umsetzung

Die Spec ist Deutsch mit echten Umlauten und Schweizer `ss`. **Die Quelldateien sind es nicht.**
`src/**` und `ESP8266/ESP-uclock/*.cpp` sind überwiegend ASCII oder ISO-8859-1 (DIR-015); `http.cpp` und
`stm32flash.cpp` sind UTF-8. **Vor jedem Patch Kodierung und Zeilenende der Datei feststellen** (CLAUDE.md):
`ESP-uclock.ino` hat gemischte Zeilenenden, ein Edit-Werkzeug vereinheitlicht sie still. Neue Dateien für den S3
sind ASCII, Kommentare in Umschrift (`Bruecke`, `waehrend`).
