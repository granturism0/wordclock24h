# Entscheidungsübersicht — was entschieden ist, was offen ist, was auf wen wartet

Stand: **17.10.2026**. Dieses Dokument **fasst zusammen, es entscheidet nicht.** Die Quelle soll das Entscheidungsprotokoll des Architekten sein
(`hardware/architektur/entscheidungen.md`). **Diese Datei gibt es noch nicht, das Verzeichnis `hardware/architektur/` ist leer** (Stand 17.10.2026). Die Übersicht stützt sich deshalb auf

1. die **Festlegungen des Nutzers** (E1–E12 in `specs/esp32s3-port/requirements.md`, [vorgaben-des-nutzers.md](vorgaben-des-nutzers.md)),
2. die **offenen Fragen F1–F18** derselben Spec, die der Dokumentar gegen die Komponentenblätter und die Netzliste gelesen hat,
3. das, was der **Generator** (`hardware/esp32s3/gen.py`) bereits umsetzt, obwohl der Architekt es nicht protokolliert hat,
4. **Empfehlungen der Datenblatt-Analysten** in den Komponentenblättern (keine Entscheide).

Sobald `entscheidungen.md` existiert, ersetzt sie die Abschnitte 2 und 3; die Statusspalte hier wird dann gegen sie abgeglichen.

**Statuswörter:** *entschieden* (durch den Nutzer festgelegt), *umgesetzt, nicht protokolliert* (im Entwurf vorhanden, ohne Entscheid des Architekten), *offen — wartet auf …*, *Empfehlung* (Vorschlag eines Analysten, nicht gewählt), *überholt*.

---

## 1. Entschieden (durch den Nutzer)

| Nr. | Entscheid | Status | Quelle |
|---|---|---|---|
| E1 | Zielprodukt: Nachbau der Platine mit ESP32-S3-WROOM-1; dieselbe Software läuft auf dem DevKitC-1 v1.1 | entschieden | Spec requirements.md |
| E2 | Pin-Sperrliste: GPIO0, 3, 45, 46, 19/20, 43/44, 35–37, 38, 48 (und nur Header-Pins des DevKits) | entschieden; von H3 geprüft | Spec; `hardware-pinregeln.json` |
| E3 | Ein USB-C-Anschluss, 16-polig, mit ESD-Schutz; Konsole zusätzlich über UART0 | entschieden | Spec |
| E4 | EEPROM und RTC bleiben (AT24C32, DS3231), Datenformat unverändert | entschieden | Spec |
| E6 | STM32-Flash-Weg entfällt | entschieden | Spec |
| E8 | 1:1-Umbau, keine neuen Funktionen | entschieden | Spec |
| E9(b) | Flashen, Konsole, JTAG über den einen USB-C-Anschluss (Muss) | entschieden | Spec |
| E9(a) | Analoger LDR-Eingang an GPIO1/ADC1 (Muss) | entschieden; **der Teilsatz „R22 ca. 47 kΩ“ ist überholt** (siehe unten) | Spec |
| E10 | LDR und LEDs bleiben auf dem LED-Board; dieses wird nicht geändert | entschieden; H8 | Spec |
| E12 | Leiterplattenformat 1:1 | entschieden; H7 | Spec |
| Nr. 14 | **`R22` bleibt 1 kΩ**, bis der Nutzer eine Abweichung freigibt | entschieden (17.10.2026) | [vorgaben-des-nutzers.md](vorgaben-des-nutzers.md) |
| Nr. 5, 15 | LED-Ansteuerung, Power-Schalten, LDR-Lesung 100 % pin- und funktionskompatibel; Abweichung nur mit Freigabe und Eintrag | entschieden; H8; **Ausnahmendatei leer** | ebd. |

**Überholt:** E9(a) „`R22` wechselt auf ca. 47 kΩ“, Spec F15 (47 kΩ/12 dB oder 10 kΩ/6 dB), `design.md` §4.2 (Rechnung mit 47 kΩ), `jlcpcb-bestand.md` „Neue Werte“ und der Skill `jlcpcb-bauteile` („47 kΩ für R22“). Bis 17.10.2026 galt 47 kΩ als Startwert der Messreihe.
Das stimmt nicht mehr, weil der Nutzer die 100-%-Kompatibilität des LDR-Pfads verlangt hat. **Nicht nachgezogen** (Übergabeliste).

---

## 2. Offene Fragen der Spec (F1–F18), gelesen gegen die Blätter

„Antwort liegt vor“ heisst nur: Ein Blatt oder die Netzliste beantwortet den **Datenteil**; der **Entscheid** liegt beim Architekten oder beim Nutzer.

| Nr. | Frage (Kurzfassung) | Stand am 17.10.2026 | Wartet auf |
|---|---|---|---|
| F1 | Trägt der SY8301 den S3? | **Strom:** 60 % der 1 A bei 0,6 A, Reserve nur mit ungelesenen Restwerten (○). **Spannung:** bei knapper 5-V-Schiene (4,5 V, 0,6 A) rechnet das Blatt 2,82 … 3,18 V (frequenzabhängig); Verhalten am Tastgradanschlag nicht dokumentiert. `F1`/`F2` unter 3,6 A (○). Siehe W-15 | `hw-architect` (Strombudget); Messung am Prototyp (Spec AK37) |
| F2 | Pull-down am Gate von Q2; Pin-Zustand beim Einschalten | **Datenteil beantwortet:** `R20` existiert (✔R); GPIO1–18 im Reset hochohmig, 60-µs-Low-Glitch (✔K). Offen: welcher GPIO. **Blatt nennt GPIO21; der Generator setzt GPIO6** | `hw-architect`; Messung (Spec AK9) |
| F3 | 5-V-Toleranz IR und DCF77 | **Beantwortet:** keine Herstellerangabe „5 V tolerant“; jeden Pin als ≤ 3,6 V behandeln (✔K). **Neu beantwortet (✔R):** die Pull-ups `R8`/`R9` liegen an **3,3 V**. Offen: **woher die angeschlossenen Empfänger gespeist werden** — auf dem Controller gibt es keinen Stecker dafür (W-2, a1) | `hw-architect` mit Netzliste; der Nutzer (Datenblätter von TSOP und DCF77-Modul) |
| F4 | Antennenfreihaltung im unveränderten Umriss | **Zahlen beantwortet** (6 mm, 15 mm, nicht Platinenmitte, ✔K). Offen: ob das an der Stelle von `U4` (127,00 / 12,25) mit dem Nachbarn `H6` (103,75 / 4,50) und im Gehäuse geht | `hw-architect`, `hw-reviewer` am Layout; Empfangsmessung am Gerät |
| F5 | GPIO-Liste am DevKit-Header | **Beantwortet gegen das DevKit** (alle elf Signale am Header, keines mit Fremdbelegung, ✔K). Gegen den Nachbau-Schaltplan: H3 meldet die Pinregeln eingehalten, aber **kein Gutachten des `hw-reviewer`** | `hw-reviewer` |
| F6 | Zielmodul-Variante | N8 gewählt (Generator, Komponentenblatt); **Bestätigung durch den Nutzer fehlt**; Lager 416 | Nutzer |
| F7 | Pull-ups auf der Platine | **Beantwortet (✔R, `expected.json`):** I²C über `RN2` 4,7 kΩ nach 3,3 V; DS18B20 über `R5` 4,7 kΩ; **die Taster `SW1`/`SW2` haben keinen externen Pull-up** (`USER` über `R1` 330 Ω, `WPS` direkt). Die Pull-ups der Taster müssten am S3 die internen sein (typ. 45 kΩ, ✔K) | `hw-architect` (Entscheid), Software |
| F8 | DevKit-Testaufbau (externer Pegelwandler, EEPROM, RTC, LED-Board) | offen | Nutzer |
| F9 | Zeittoleranz SKC6812 | offen (Datenblatt nicht im Repo) | Nutzer |
| F10 | STM-Endpunkte und PWA-Oberfläche | Empfehlung der Spec: Endpunkte bleiben, antworten mit Fehlercode; **einzige Stelle, an der „kein Funktionsverlust“ nicht wörtlich gilt** | Nutzer |
| F11 | Bleiben die alten Ziele baubar? | Annahme der Spec: ja | Nutzer |
| F12 | Framework, Build, Verzeichnis, Besitz | **offen.** `specs/multiboard-quelle/requirements.md` liegt als **Entwurf** vor und empfiehlt ESP-IDF 5.5.x mit Arduino-ESP32 als Komponente (Frage Q5; auf dem Rechner liegt Arduino-ESP32 mit IDF 5.5.5, aber **kein `idf.py`**); `design.md` und `tasks.md` fehlen, keine Freigabe. Die Spec schlägt zudem vor, dass `boards/<id>/board.h` die Pins hält (statt `esp32s3/pins.h`), und braucht von `hw-architect` ein **maschinenlesbares** `pinbelegung.md` (Q7) | `sw-architect`, Lead, `hw-architect`, Nutzer |
| F13 | Versionierung (zwei Symbole, ein Abbild) | Empfehlung der Spec; DIR-004 gilt weiter | `sw-architect`, Nutzer |
| F14 | `F_INTERRUPTS` am S3 (20 kHz statt 15 kHz) | Entwurf rechnet mit 20 kHz | `stm-developer`, `firmware-analyst` |
| F15 | `R22`, Dämpfung des ADC, Abblockkondensator | **`R22` durch Nr. 14 entschieden (1 kΩ).** Das Blatt rechnet jetzt mit 1 kΩ (§6.3): 0 dB, Mittelung, Abbildung auf die STM-Skala, funktional gleichwertig, nicht zählergleich. **Offen:** Ob `C32` unter „100 % kompatibel“ fällt (Nutzer, OP-03), und ob die Lage angenommen wird (OP-02) | Nutzer |
| F16 | Aussenmasse 16-poliger gegen 6-poligen USB-C-Stecker | 16P: 8,94 × 7,35 × 6,28 mm (✔K). **Masse der 6-poligen Buchse nicht belegt**; Layout darf nicht beginnen | `hw-datasheet-analyst`, dann `hw-architect` |
| F17 | Wo sitzen BOOT- und EN-Taster bei 1:1-Format? | EN: `SW3` (V2-Reset-Taster, im Generator am Netz `EN`). **BOOT (`SW5`) hat keine Position** (Kopie von `SW1`) | `hw-architect` |
| F18 | 74AHCT1G125 mit 3,3-V-Eingang des S3 | **Beantwortet (✔K):** VIH min. 2 V gegen VOH min. 2,64 V, rund 0,64 V Reserve. Offen: SK6812-Datenblatt (VIH, Pegel 3,8 V) | `hw-datasheet-analyst` |

---

## 3. Im Entwurf umgesetzt, ohne protokollierten Entscheid

Der Generator hat folgende Festlegungen schon gezeichnet. **Entscheide des Architekten gibt es dazu nicht** (Reihenfolge laut `CLAUDE.md`, R3: Analyst → Architekt → Designer → Reviewer). Das ist keine Wertung, sondern eine Lücke in der Spur.

| Festlegung im Entwurf | Quelle | Ob entschieden | Beleg / Gegenstimme |
|---|---|---|---|
| Pinzuordnung der elf Signale (GPIO1, 2, 5–9, 15–18) | `gen.py`, Spec „Vorläufige Pinzuordnung“ | Spec: „Vorschlag, gegen den Schaltplan abzugleichen“; `pinbelegung.md` fehlt; `hardware-pinregeln.json` führt `"pinbelegung": {}` | ✔K: alle elf Pins sind Header-Pins ohne Fremdbelegung |
| `WS2812_POWER` an GPIO6 (nicht GPIO21) | `gen.py` | nicht protokolliert | Blatt `esp32s3-wroom-1.md` §4.2, §12 nennt GPIO21; GPIO6 ist mit `R20` am N-Kanal-Gate unkritisch (W-8) |
| `C1` 100 nF → 1 µF; `RESET` und `ESP_EN` → `EN` | `gen.py` | nicht protokolliert | ✔K EN-Beschaltung 10 kΩ + 1 µF |
| `SW5` (BOOT) und `R32` (Pull-up GPIO0) | `gen.py` | nicht protokolliert | ✔K; Position offen (F17) |
| `C30` 10 µF und `C31` 100 nF am Modul | `gen.py` | nicht protokolliert | Referenz im Modul-Datenblatt 22 µF + 0,1 µF; `C8` 22 µF auf dem Netz |
| **`C32` 100 nF am Netz `LDR`** | `gen.py` | nicht protokolliert; **H8 meldet HOCH** (zusätzliches Teil an einem Schnittstellennetz) | HDG empfiehlt 0,1 µF am ADC-Eingang; Nutzer-Freigabe `zusatz:C32` fehlt |
| `USB1` = HRO TYPE-C-31-M-12 (C165948) | `gen.py` | im Auftrag benannt | ✔K `usb-c-16p.md` |
| `U11` = USBLC6-2SC6 (C7519) | `gen.py` | im Auftrag benannt | ✔K `usblc6-2sc6.md` |
| Konsole `H6` am UART0, `TXD0` → `TX_UART` | `gen.py`, `hardware-pinregeln.json` | nicht protokolliert; **Orientierungsfrage** (OP-35) | W-2, a3 |
| `H6`/`D3` bleibt Einspeisung der 3,3-V-Schiene | unverändert aus V2 | — | — |
| Keine Reihenwiderstände an USB, kein D+-Pull-up, keine 499 Ω in `U0TXD` | Generator hat sie nicht | nicht protokolliert | HDG empfiehlt Platz bzw. Bauteile (OP-16, OP-17) |

---

## 4. Empfehlungen der Analysten (keine Entscheide)

| Empfehlung | Blatt | Auswirkung auf die Vorgaben |
|---|---|---|
| `Q2` (MMBF170, Shikues, Extended, Lager 1'355) → AO3400A (C20917, Basic) oder 2N7002 (C8545, Basic; **nicht belegt** bei 3,3 V Gate) | `mmbf170.md` §4, §6 | **`Q2` liegt im Schnittstellenpfad (H8 prüft Wert und Pins).** Umstellung = Abweichung von V2, braucht Nutzerfreigabe (OP-04) |
| `R19` 82 Ω (Extended) → 100 Ω (C17408, Basic) | `jlcpcb-bestand.md` | `R19` ist in H8; wie `Q2` |
| `R26` 22,1 kΩ → 22 kΩ (C17560, Basic); ILM +0,6 % | `tps2121.md` §10, `jlcpcb-bestand.md` | `R26` ist **nicht** in H8 (Versorgung) |
| `R29` 32 kΩ → bestelltes Teil ist 31,6 kΩ; Schaltplan und Teil angleichen | `sy8301abc.md`, `jlcpcb-bestand.md` | nicht in H8 |
| `PR1` des TPS2121 über 10 kΩ an `USB_5V_PROT`, oder Klemmspannung von `D4` prüfen | `tps2121.md` §6, §10 | nicht in H8 |
| Pull-ups an `32KHZ` und `SQW` (`RN2` Pin 5, 6) weglassen, spart rund 0,7 mA | `ds3231mz.md` §8 | nicht in H8; ändert `RN2`-Beschaltung |
| DS18B20: Maxim-Original (C9753) statt HXY-Nachbau (C5451632), Empfehlung ○ | `ds18b20.md` §8 | nicht in H8 |
| `R22` = 10 kΩ mit 6 dB, 22 kΩ oder 47 kΩ mit 12 dB (frühere Analyse des Moduls) | `esp32s3-wroom-1.md` §6.2 | **vom Analysten selbst zurückgezogen** (§6.2, §12): massgebend ist §6.3 mit `R22` = 1 kΩ |
| ADC mit 0 dB (0 … 850 mV, ±5 mV), eFuse-Kalibrierung, Mittelung ≥ 16, Abbildung `N_STM = V_mV · 4095 / 3300`, einmalige Nachkalibrierung | `esp32s3-wroom-1.md` §6.3, §12 | Empfehlung für die Software; Entscheid bei Architekt und Nutzer |
| Schiene über einen zweiten ADC1-Pin messen (Teiler 2 × 100 kΩ, 0,1 µF) oder `R22` = 4,7 kΩ mit 6 dB | `esp32s3-wroom-1.md` §6.3 | nur falls je nötig; **ändert V2, nicht freigegeben** |
| Reihenwiderstände 22/33 Ω und Kondensatoren an D+/D−, D+-Pull-up extern | `esp32s3-wroom-1.md` §7, §12 | nicht in H8 |

---

## 5. Prozess- und Rollenentscheide

| Entscheid | Quelle |
|---|---|
| Reihenfolge Analyst → Architekt → Designer → Reviewer; Dokumentar nach jedem Schritt | `CLAUDE.md` (R3), `.claude/agents/hw-*.md` |
| Schreiber je Verzeichnis (siehe [../README.md](../README.md)) | `CLAUDE.md` (R3-Tabelle), `tools/hooks/file-ownership.py` |
| Prüfungen H1–H8 sind Guardrail S19, Gegenproben S20 | `tools/guardrails.sh`, `tools/checks/hardware.py` |
| Das V2-Projekt bleibt nur lesend | `CLAUDE.md`, alle Rollenbeschreibungen |
| Keine Bestellung ohne Gutachten des `hw-reviewer`; bestellen tut der Nutzer | `hw-layouter.md`, Skill `jlcpcb-bauteile` |
| Der Nutzer erklärt jede Stufe der Spec für bestanden, danach beginnt die nächste (Tore G0–G5) | `specs/esp32s3-port/tasks.md` |
| **Freigabe der Spec `esp32s3-port`** | **steht aus** (Status der Spec: „Entwurf — zur Freigabe durch den Nutzer“, Task 0.6) |
