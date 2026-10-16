# Entscheidungsprotokoll — Controller-Platine ESP32-S3-WROOM-1

Stand: **17.10.2026**. Schreiber: `hw-architect`. Lebendes Dokument (DIR-006): Wo eine Zahl steht, trägt sie Datum und Fundstelle.
Dazu gehören `pinbelegung.md` (Signal, Netz, GPIO, Grund) und `leistungsbudget.md` (Ströme, Spannungsreserve, Sicherungen).

**Beleg-Zeichen:** ✔ gelesen (Quelle, Seite; Komponentenblätter unter `hardware/komponenten/` heissen im Folgenden *Blatt*), ● Sekundärquelle (Distributor-Datenbank, Beschreibungsfeld),
○ Annahme oder eigene Rechnung. **Ein Entscheid auf ○ ist ein offener Punkt** und steht dann als „offen“ im Status.

**Status:** *entschieden* · *offen, wartet auf Analyst* · *offen, wartet auf Nutzer* · *offen, wartet auf Messung* · *Antrag* (Abweichung von 1:1, nur im Abschnitt 4).

**Rahmen des Nutzers (nicht neu verhandelt):** Umbau des jetzigen Prints und seiner Funktion 1:1 auf den ESP32-S3-WROOM-1; dasselbe LED-Board über den gespiegelten Stecker `H7`
(„LED-Ansteuerung, Power-Safe und LDR-Lesung 100 % pin- und funktionskompatibel“); Leiterplattenformat 1:1; ein USB-C für Flashen, Konsole und JTAG; analoger LDR-Eingang an ADC1;
alles bei JLCPCB, wenn möglich Standard-Katalog (Basic); die Software läuft auch auf dem DevKitC-1 (nur Header-GPIOs, nicht 0/3/45/46/19/20/43/44/35–37/38/48 für Signale).
**Jede Abweichung von 1:1 steht im Abschnitt 4** und trägt die Zeile „braucht Freigabe des Nutzers“. Ein Eintrag in `tools/checks/hardware-led-schnittstelle.ausnahmen` ist erst nach dieser Freigabe zulässig und Sache des Lead.

**Die V2-Netzliste ist die Wahrheit über den Bestand** (`hardware/esp32s3/v2/v2.xml`, `nets.json`), nicht `HARDWARE.md`.

---

## 1. Funktionsblöcke und ihre Schnittstellen

Die Blöcke folgen der Gliederung des Schaltplan-Entwurfs (`hardware/esp32s3/gen.py`, `BLOCKS`); Bauteile aus V2, Neues ist als **neu** gekennzeichnet.

| Block | Bauteile | Schnittstelle nach aussen | Entscheide |
|---|---|---|---|
| **B1 USB-C-Eingang und Schutz** | `USB1` (16P, **neu**), `U11` USBLC6 (**neu**), `C33` (**neu**), `R3`/`R4` (Rd), `D4` TVS, `F1` PTC, `C4`, `C20`, `TP1`, `TP4`, `JP3` | `USB_5V_PROT` an B3; D+/D− an B5 (Netze `USB_DP`/`USB_DN`) | AE-02, AE-10 |
| **B2 Externe Einspeisung** | `H1`/`H2`, `D5`, `F2`, `Q3` (Verpolschutz), `R24`, `R25`, `C2`, `C21`, `C22`, `TP5`, `TP6` | `ALT_5V_PROT` an B3 | AE-10 |
| **B3 Power-Mux** | `U9` TPS2121, `R26`, `C24` | `VCC_5V` an B4, B6 | AE-09, AE-19 |
| **B4 3,3-V-Versorgung** | `U10` SY8301, `L2`, `D2`, `R28`, `R29`, `C25`, `C27`–`C29`, `C23`, `C26`, `C8`, Abblockkondensatoren, `TP7`, `TP8`, `LED4`/`LED5` | `VCC_3.3V` an B5, B7, B8 und `H7.3` | AE-08 |
| **B5 Controller** | `U2` Modul (**neu**), EN-Beschaltung (`R12`, `C1`, `SW3`), BOOT (`SW5`, `R32`, **neu**), Konsole `H6` (`R33`, **neu**, `D3`), Taster `SW1`/`SW2`, `R1`, `LED1`/`R6`, Entkopplung `C30`, `C31` | GPIO nach B6, B7, B8; USB aus B1; **Reset-/Boot-/Programmierkonzept: AE-03, AE-04** | AE-03 bis AE-07, AE-16 |
| **B6 LED-Pfad** | `U3` AHCT125, `R11`, `R13`, `R17` (DNP), `R31`, `Q1`, `Q2`, `R19`–`R21`, `R23`, `C10`, `C6`, `JP2`, `H4`, `H7` | `WS2812_BUS`, `WS2812_POWER` vom Controller; `TO_VCC_RGB_5V`, `TO_SK6812_IN` am Stecker | AE-06, AE-14, AE-17, Abschnitt 4 |
| **B7 LDR-Pfad** | `R22`, `C32` (**neu**, Antrag A1), `H5`, `H7.5` | Netz `LDR` an `GPIO1` | AE-01 |
| **B8 Sensorik und Bus** | `U1` DS18B20, `R5`, `U5` DS3231, `U7` CR2032, `U6` AT24C32, `RN1`, `RN2`, `R8`, `R9` | I²C (`GPIO8`/`GPIO9`), 1-Wire (`GPIO2`), IR/DCF77 (`GPIO7`/`GPIO15`) | AE-07, AE-12, AE-13, AE-18 |
| **Befestigung** | `H2`, `H8`, `H9`, `H10` | — | AE-16 |

**Entfallen gegenüber V2** (Auftrag des Nutzers, `jlcpcb-bestand.md`): `U2` STM32F411, `U4` ESP-12F, `X1` mit `C3`/`C5`, VDDA-Filter (`L1`, `C11`, `C14`), `C9`, SWD-Stecker `H3` mit `D1`,
Umschalter `SW4`, Boot-Widerstände des ESP8266 (`R10`, `R14`–`R16`, `R18`), `R2`, `R7`, 6-poliges `USB1`. Mit `H3`/`D1` entfällt eine der drei Einspeisungen des 3,3-V-Netzes (`H3.1`); `H6.1` über `D3` bleibt.

---

## 2. Risiken — was die Platine zerstören oder unbrauchbar machen kann (vor allen Vorlieben)

Geordnet nach Schwere der Folge. Jedes Risiko nennt die Konsequenz und wo es entschieden ist.

| # | Risiko | Konsequenz | Stand | Entscheid |
|---|---|---|---|---|
| **R1** | **5 V am Stecker `H6` zerstören das Modul.** (a) Pin 1 speist `VCC_3.3V` über `D3` ein: Ein Konsolenadapter, dessen Versorgungspin 5 V führt (viele Adapter haben einen 5-V/3,3-V-Steckbügel), hebt das 3,3-V-Netz auf rund 4,7 V; absolut erlaubt sind 3,6 V (✔ Blatt §2: VDD33 −0,3 … 3,6 V). (b) Pin 2 ist `RXD0` (`GPIO44`): ein Adapter mit 5-V-Pegel überschreitet VDD + 0,3 V (✔ Blatt §5, ein Wert je Einzelpin steht in keiner Unterlage). In V2 hing derselbe Stecker an einem STM32; ob dessen Pins 5 V vertragen, ist hier nicht gelesen (○) | Modul und Flash sofort oder schleichend defekt; kein Schutz in der Schaltung | **offen, wartet auf Nutzer** (welcher Adapter, welcher Pegel; Frage Q1) | AE-05, A11 |
| **R2** | **Konsolenrichtung am Stecker `H6` im Entwurf vertauscht** (`gen.py` Z. 132, `hardware-pinregeln.json`): Modul-Ausgang `TXD0` läge an `H6.2`, wo V2 den Empfangseingang des STM hatte (OP-35; ✔ `nets.json`, `HARDWARE.md`) | Konsole an `H6` tot; mit einem V2-Kabel treffen zwei Ausgänge aufeinander | **entschieden** (Richtung festgelegt, Designer korrigiert) | AE-05 |
| **R3** | **Strapping-Pins beschaltet.** `GPIO45` High beim Einschalten speist am N8 den Flash mit 1,8 V (✔ DS-M S. 39); `GPIO46` High verhindert den UART-/USB-Download; `GPIO3` und `GPIO0` mit Kondensator oder Pull ändern Boot-Modus bzw. JTAG | Modul läuft nicht an oder lässt sich nicht flashen, ohne dass die Schaltung falsch aussieht | **entschieden**: `IO3`, `IO45`, `IO46` bleiben offen, `IO0` nur `BOOT` (H3 erzwingt es) | AE-04 |
| **R4** | **LED-Versorgung schaltet nicht oder zur Unzeit.** (a) `R23` und `C10` fehlen in der V2-Fertigungsliste, obwohl der Schaltplan sie führt (`HARDWARE.md`, L485): Ohne `R23` hat `Q1` keine Verbindung zum Treiber. (b) Das LCSC-Feld von `R23` im V2-Schaltplan trägt C17841, die Nummer des 82-Ω-Widerstands von `R19` (OP-06). (c) Ohne `R20` (Pull-down am Gate von `Q2`) wäre das Gate im Reset schwebend | Kette dauernd aus (a) oder beim Booten kurz an (c); mit dem falschen Wert (b) ändert sich der Einschaltstrom | (a), (b) **offen, wartet auf Nutzer und Analyst**; (c) entschieden (`R20` bleibt) | AE-06, AE-17 |
| **R5** | **`PR1` des TPS2121 liegt direkt an `USB_5V_PROT`:** absolut 6 V, empfohlen 5,5 V (✔ Blatt `tps2121.md` §3); die Klemmspannung von `D4` ist nicht gelesen. Hot-Plug-Spitzen hinter der PTC könnten 6 V überschreiten | Mux defekt beim Stecken; in V2 ist kein Ausfall belegt (○) | **Antrag A4** (10 kΩ in Reihe), bis zur Freigabe wie V2 | AE-09 |
| **R6** | **3,3-V-Schiene unter 3,0 V** bei knapper 5-V-Schiene und WLAN-Spitze: `FB` hinter `D2`, Tastgrad begrenzt (✔ Blatt `sy8301abc.md` §4.5); Verhalten am Anschlag nicht dokumentiert. Eigene Rechnung mit den Werten des Blattes: Reserve 0,04 … 0,12 V über 3,0 V bei realistischer Last, **negativ im 2,4-MHz-Eck** bei der Entwurfslast von 0,6 A (`leistungsbudget.md` §5) | Brownout-Reset im Betrieb, vor allem an USB und unter LED-Last | **offen, wartet auf Messung** (AK37 am Prototyp); Fallback-Kette in AE-08 | AE-08 |
| **R7** | **USB ist Datenanschluss und hat Vorrang vor der externen Einspeisung.** Wer zum Flashen oder Debuggen den PC ansteckt, während die Uhr am Netzteil (`H1`) hängt, lässt den PC-Port die Uhr speisen: Der TPS2121 wählt `IN1` (USB) unabhängig von der Spannung (✔ Blatt `tps2121.md` §5). Ein PC-Port liefert 500 mA (✔ Blatt `usb-c-16p.md` §3.3), `F1` hält 1,5 A | Einbruch von `VCC_5V`, Umschalten zwischen den Quellen, Reset des Moduls während Flashen/Konsole bei hellem Display | **offen, wartet auf Nutzer** (Frage Q3); ohne Platinenänderung nur durch Umgang (eine Quelle) oder LED aus | AE-19, A9 |
| **R8** | **`F2` (3 A Haltestrom ●) liegt unter der Entwurfslast** 3 A LED + Logik (≈ 3,5 A, `leistungsbudget.md` §4) | PTC erwärmt sich, Spannung sackt, Neustarts bei hellem Display (so die Hypothese in `HARDWARE.md`, L9, ○) | **offen, wartet auf Nutzer** (maximale LED-Last) und Analyst (Teil) | AE-10, A7 |
| **R9** | **`CC1`/`CC2` ohne ESD-Schutz** (der USBLC6 schützt nur D+, D−, VBUS): V2 hatte ebenfalls keinen | Entladung an der Buchse kann `R3`/`R4` oder den Mux belasten; Ausfälle im Feld nicht belegt | **entschieden** (wie V2), Restrisiko benannt | AE-02 |
| **R10** | **Antenne und Format:** Das S3-Modul (18,0 × 25,5 mm ✔ Blatt §1) ist grösser als der ESP-12F (dessen Masse sind hier nicht gelesen), der Nachbar `H6` bleibt (OP-13); die Gehäuseöffnung für `USB1` ist für einen 6-Pol-Stecker gemacht, dessen Masse nicht belegt sind (OP-11) | Reichweite schlechter als V2 oder Platine passt nicht ins Gehäuse | **offen, wartet auf Analyst** (Masse) und **Messung** (RSSI) | AE-16 |

---

## 3. Entscheide

### AE-01 — LDR-Lesung: `R22`, `C32`, ADC-Dämpfung, Abbildung

- **Frage:** `R22` und Dämpfung des ADC für den LDR-Abgriff; bleibt `C32` (100 nF am ADC-Pin)?
- **Optionen:** (a) `R22` = 1 kΩ wie V2, 0 dB, kalibriert, gemittelt; (b) `R22` 10/22/47 kΩ mit 6 bzw. 12 dB (frühere Rechnung des Blattes §6.2, vom Analysten zurückgezogen); (c) zweiter ADC1-Pin für die 3,3-V-Schiene (ratiometrisch rechnen).
- **Wahl:** **(a).** `R22` = 1 kΩ, `LDR` an `GPIO1` (ADC1), Dämpfung **0 dB** (0 … 850 mV, ±5 mV kalibriert), Kalibrierung ab Werk (eFuse), Mittelung über mindestens 16 Abtastungen,
  Abbildung auf die STM-Skala `N_STM = V_mV · 4095 / V_rail` mit `V_rail` als **benannter Konstante** (Startwert 3300 mV; der Wert der Platine wird bei AK37 gemessen und eingesetzt).
  Dämpfung, Abtastzahl und `V_rail` liegen in der Software, nicht auf der Platine. `C32` siehe Abschnitt 4, Antrag A1 (**Frage an den Nutzer**).
- **Begründung:** Die Vorgabe verlangt identische Spannungen am Pin; `R22` liegt im LDR-Pfad, den H8 prüft. Alle drei Zustände (dunkel 3,3 mV, Zimmer 157 mV, hell 300 mV) liegen bei 0 dB im Bereich, hell mit 550 mV Reserve;
  der Bereich endet erst bei einem LDR unter rund 2,9 kΩ (✔ Blatt §6.3 (2)). Das Blatt urteilt: **funktional gleichwertig für die Helligkeitsautomatik, nicht zählergleich** (§6.3 (5)):
  Der S3 misst **absolut** (der STM ratiometrisch), sein Gesamtfehler ±5 mV entspricht ±6,2 STM-Zählern; `ldr.c` klemmt den Dunkelwert auf `ldr_min_value`, das tut es in V2 ebenso (STM-Fehler ±5 LSB).
  Mit den genannten Massnahmen bleiben die gespeicherten Grenzen auf ±1 Stufe gültig; eine einmalige Nachkalibrierung stellt Zählergleichheit her (✔ Blatt §6.3 (5) Nr. 3).
- **Folgen für die Spec** (Meldung an den Lead): E9(a), F15, H9, `design.md` §4.2 und AK17/AK17b verlangen noch 47 kΩ und neue Kalibrierung; das gilt nicht mehr (`R22` bleibt, die V2-Grenzen bleiben mit ±1 Stufe gültig, eine einmalige Nachkalibrierung ist ein Schritt, kein Umbau).
- **Zwei Hinweise aus der Prüfung:** (1) Der 0-dB-Bereich endet bei 1055 STM-Zählern (✔ Blatt §6.3 (2)); hat der Nutzer `ldr_max_value` auf einen höheren Wert gestellt (Sonnenlicht, LDR unter 2,9 kΩ), sind die obersten Stufen nicht erreichbar. Die gespeicherten Werte sind nicht bekannt (Frage Q8); 2,5 dB (0 … 1100 mV, 1365 Zähler, ±6 mV, ✔ Blatt §6.1 und Spec `design.md` §4.2) wäre die Software-Alternative.
  (2) Die Toleranz der 3,3-V-Schiene geht beim S3 voll in den Zähler: Mit `R29` = 31,6 kΩ rechnet das Blatt `sy8301abc.md` §4.1 3,249 … 3,417 V (±1-%-Widerstände und ±1 % Referenz, ○) um die Nominalspannung 3,332 V, das sind rund ±2,5 %; nach der Formel `t · N` des Blattes §6.3 (2) sind das rund ±5 bis ±9 STM-Zähler bei „Zimmer“ bis „hell“ (eigene Rechnung aus den Zählern 195 und 372).
- **Beleg:** ✔ `esp32s3-wroom-1.md` §6.1 (DS-C Tab. 5-6, S. 66), §6.3 (1)–(5); ✔ V2 `nets.json` (Netz `LDR` = `H5.2`, `H7.5`, `R22.2`, `U2.15`); ○ die LDR-Widerstände (Annahmen des Auftrags).
- **Umstösst die Wahl:** eine Messung am DevKit, die das Rauschen (nicht angegeben, Blatt §6.3 (1)) unter WLAN-Last als zu gross zeigt; ein `ldr_max_value` des Nutzers über 1055 Zähler; ein Nutzerentscheid für eine Abweichung (`R22` 4,7 kΩ bei 6 dB oder zweiter ADC1-Pin, beide „nicht freigegeben“, Blatt §6.3 Tabelle).
- **Status:** **entschieden** (Platine: `R22`); `C32` **Frage an den Nutzer** (A1); Dämpfung und Konstanten beim Software-Entwickler (Blatt §6.3 empfiehlt 0 dB).

### AE-02 — USB-Datenweg: Reihenwiderstände, Pull-up, VBUS, ESD, Schirm

- **Frage:** Vorwiderstände in D+/D− (drei Aussagen: HDG 22/33 Ω als Platzhalter, Referenzschema des Moduls 0 Ω, DevKit keine), externer Pull-up an D+, VBUS-Erkennung, Schutz von CC1/CC2, Bestückungsoptionen.
- **Optionen und Wahl je Punkt:**

| Punkt | Optionen | **Wahl** | Begründung | Beleg |
|---|---|---|---|---|
| Reihenwiderstände | 22/33 Ω bestückt · 0 Ω bestückt, tauschbar · keine | **2 × 0 Ω** (Basic, C17477) zwischen `USB_DP`/`USB_DN` (Steckerseite) und `USB_DP_M`/`USB_DN_M` (Modulseite), dicht am Modul; **Plätze für zwei Kondensatoren** nach GND an den Modulseiten-Netzen, **unbestückt** (DNP) | Das Referenzschema des Herstellers selbst zeigt 0 Ω und unbestückte C5/C6 (✔ DS-M Abb. 9-1, S. 41); das HDG verlangt Platz für 22/33 Ω und C, zunächst unbestückt (✔ HDG S. 18). Mit 0 Ω ist die Platine so wie die Referenz, und der Wert lässt sich ohne neues Layout auf 22/33 Ω ändern | ✔ Blatt §7 |
| Externer Pull-up an D+ | bestückt · Platz unbestückt · nichts | **Platz (DNP)** an `USB_DP_M`; Bestückung **offen** | Das HDG empfiehlt ihn für einen stabilen Anfangszustand (✔ HDG S. 19; die Glitches von 3,2 ms/2 ms bei GPIO19/20 stehen in Blatt §4.2). **Wert, Schiene und Verträglichkeit mit dem internen Pull-up (1,1/2,2 kΩ, schaltbar) sind nicht gelesen**; das DevKit hat keinen und funktioniert (✔ Blatt §7) | ✔ Blatt §7 (HDG S. 19); ○ Wert |
| VBUS-Erkennung | GPIO an VBUS · keine | **keine** | Der Chip hat keinen VBUS-Pin; das TRM beschreibt keine VBUS-Erkennung für Serial/JTAG; das DevKit führt VBUS nur auf die 5-V-Schiene (✔ Blatt §7). Ein GPIO an 5 V wäre zudem ein Pegelfehler (Regel 3 in `pinbelegung.md`) | ✔ Blatt §7 (TRM Kap. 33, DK-Sch Blatt 2) |
| ESD für `CC1`/`CC2` | zweiter USBLC6 · TVS · nichts (wie V2) | **nichts** | Der USBLC6-2SC6 sperrt bis 5,25 V (✔ Blatt `usblc6-2sc6.md` §2), `CC` führt bis 5,5 V (Rp gegen 4,75 … 5,5 V, ✔ Blatt `usb-c-16p.md` §3.3): ungeeignet, dazu eine zweite Extended-Nummer. V2 hatte keinen Schutz (✔ `nets.json`); 1:1. Restrisiko R9 | ✔ beide Blätter; ○ keine Felddaten |
| VBUS-Kondensator am ESD | keiner · 100 nF | **100 nF** (`C33`, **neu**, Basic C49678) zwischen `USB_5V_INPUT` und GND dicht an `U11` | ST zeigt `C_BUS` = 100 nF am VBUS-Pin (✔ Blatt `usblc6-2sc6.md` §4, S. 9 Bild 18). Wirkung am Prototyp nicht gemessen (○) | ✔ Blatt `usblc6-2sc6.md` §4 |
| ESD-Lage | beliebig · an der Buchse | **`U11` unmittelbar an `USB1`**, D+/D− laufen **durch** den Baustein (Pin 1 → 6, Pin 3 → 4), VBUS Pin 5, GND Pin 2 direkt auf die Massefläche | 10 mm Bahn (6 nH) heben die Klemmspannung bei 8 kV von rund 31 V auf über 300 V (✔ ST S. 4–5, Blatt §4) | ✔ Blatt `usblc6-2sc6.md` §4 |
| Brücken A6/B6, A7/B7 | — | kurze Brücken, Stich **≤ 3,5 mm**; B-Seite nicht der Reihe nach verbinden | ✔ SPEC S. 69, Blatt `usb-c-16p.md` §3.2, §8 | ✔ |
| Rd, SBU, Schirm | — | `CC1`/`CC2` je eigener 5,1 kΩ ±1 % (C27834, Basic) nach GND, nie zusammenlegen; `SBU1`/`SBU2` offen; alle vier VBUS- und GND-Pins verbinden; Schirm **direkt an GND** (wie V2) | ✔ SPEC S. 69, S. 135, S. 312; V2 hat den Schirm an GND (`USB1.7`) | ✔ Blatt `usb-c-16p.md` §3.2 |

- **Netze:** Steckerseite `USB_DP`/`USB_DN` (= `USB1.A6/B6`, `U11.1/6`; `USB1.A7/B7`, `U11.3/4`), Modulseite `USB_DP_M`/`USB_DN_M` (= `U2.IO20`/`U2.IO19`). Die Prüfregeln in `hardware-pinregeln.json` sind entsprechend anzupassen (`pinbelegung.md` §5).
- **Umstösst die Wahl:** Instabiles USB-Enumerieren beim Prototyp (dann 22/33 Ω und der Pull-up); ein Befund des Reviewers zum Layout (Differenzpaar 90 Ω ± 10 %, ✔ HDG S. 30).
- **Status:** **entschieden** bis auf den Pull-up an D+ (**offen, wartet auf Analyst:** HDG S. 19 neu lesen — Wert, Schiene, Wechselwirkung mit dem internen Pull-up — und ob das Ergebnis den Rückweg AK3 Fall 2 berührt).

### AE-03 — Reset, EN, BOOT-Taster, Programmierkonzept (F17)

- **Frage:** Wie sind EN und GPIO0 beschaltet; wo sitzt der BOOT-Taster bei unverändertem Format (`SW1`–`SW3` sind belegt); wie wird geflasht?
- **Optionen für den Ort:** (A) in der vom STM32 freigewordenen Fläche, neben dem Modul-Rand; (B) als vierte Position in der Tasterreihe (x ≈ 73, y = 27,75) — dort steht `USB1` mit seinem Gehäuse in der Nähe (Lage ○, nicht gemessen); (C) Doppelbelegung von `SW1`/`SW2`: verworfen, GPIO0 darf keine Anwendungsfunktion haben (Rahmen); (D) nur zwei Lötpads GPIO0/GND (Pinzette) als Rückfall, falls kein Platz für (A) bleibt.
- **Wahl:** EN: `R12` 10 kΩ nach 3,3 V, `C1` **1 µF** nach GND (V2: 100 nF), `SW3` (Position unverändert, V2-Reset-Taster) nach GND; `R2` entfällt. BOOT: `R32` 10 kΩ nach 3,3 V, **`SW5` nach GND**, **Ort (A)**, ausserhalb des Antennenbereichs und ohne Gehäuseöffnung (Servicetaste), GPIO0 trägt **nichts sonst**, keinen Kondensator.
  Kein externer Spannungswächter.
  **Programmierkonzept, drei Wege:** (1) normal: `esptool` über USB-Serial/JTAG, Reset und Download-Modus per DTR/RTS ohne Strapping (✔ TRM Tab. 33.3-2 ff.); (2) Rückweg: BOOT halten, EN kurz Low (Taster `SW5`, `SW3`); (3) zweiter Rückweg: UART0 über `H6` mit einem 3,3-V-Adapter und derselben Tasterfolge, weil der Joint-Download-Boot UART **und** USB bedient (✔ DS-M Tab. 4-3, S. 14). JTAG: über denselben USB-Anschluss (`GPIO3` offen, `GPIO39`–`42` frei).
- **Begründung:** EN-RC = 10 kΩ · 1 µF = 10 ms, EN erreicht 0,75 · 3,3 V nach rund 13,9 ms (✔ Blatt §8, Rechnung); die 3,3-V-Schiene steht nach rund 6 … 8 ms (TPS2121-Rampe 6,4 ms, der SY8301 läuft ab 4,0 V mit 1 ms Softstart an, ○ Rechnung aus ✔ `tps2121.md` §6 und `sy8301abc.md` §3), also vor EN. Ein Wächter (≈ 3,0 V) ist für langsam steigende oder wacklige Versorgung vorgesehen (✔ HDG S. 8–9); hier steigt die Schiene steil, und der Chip hat einen eigenen Brownout-Reset (AK25). Er wäre eine weitere Extended-Nummer. Der manuelle Rückweg ist der **einzige**, wenn die Anwendung `GPIO19/20` umkonfiguriert hat und kein UART-Adapter steckt (✔ HDG S. 36, Blatt §7).
  `SW3` setzt künftig den ganzen Chip zurück (F10: „Neustart“ bleibt erhalten, ✔ Spec `design.md` §6).
- **Beleg:** ✔ Blatt §4.1, §7, §8; ✔ V2 `nets.json` (`ESP_EN` = `R12`, `U2.PB3`, `U4.EN`; `RESET` = `C1`, `R2`, `SW3`, `U2.NRST`, `U4.GPIO14`; in V3 zu `EN` vereint).
- **Umstösst die Wahl:** Kein Platz für `SW5` im Layout (dann D); EN-Verhalten bei wackliger Speisung am Prototyp (dann Spannungswächter); ein Befund, dass `C1` = 1 µF die Reaktion von `SW3` unbrauchbar verzögert (tSTBL/tRST ≥ 50 µs ✔ Blatt §8, unkritisch).
- **Status:** **entschieden**; die genaue Lage von `SW5` legt der Layouter fest und nennt sie im Bericht (OP-12).

### AE-04 — Strapping-Pins und gesperrte GPIOs

- **Frage:** Wie werden `IO0`, `IO3`, `IO45`, `IO46` und die gesperrten Pins behandelt?
- **Wahl:** `IO3`, `IO45`, `IO46`, `IO35`–`IO38`, `IO48` **offen** (ohne Bauteil, ohne Stift), `IO0` nur `BOOT` (`R32`, `SW5`), `IO19`/`IO20` nur USB, `IO43`/`IO44` nur UART0. Die zwölf unbelegten Pins (`pinbelegung.md` §4) erhalten keinen Zugang.
- **Begründung:** `GPIO45` beim Einschalten High speist am N8 den Flash mit 1,8 V (✔ DS-M S. 39); `GPIO46` muss für den Download Low sein; `GPIO3` ist schwebend und wirkt nur nach `EFUSE_STRAP_JTAG_SEL` (nicht gebrannt, ✔ DS-M S. 15–16); der interne Pull-down von `IO45`/`IO46` hält sie ohne Bauteil richtig. Die Strapping-Pins lassen sich ohne Beschaltung nicht falsch setzen.
- **Beleg:** ✔ Blatt §4.1; H3 (`hardware-pinregeln.json`, `muss_offen_sein`) prüft die acht Pins gegen die Netzliste.
- **Umstösst die Wahl:** ein Wunsch nach Auto-Download über Transistoren (DTR/RTS auf EN/GPIO0 wie am DevKit): nicht nötig, weil USB-Serial/JTAG es ohne Strapping kann (✔ TRM Tab. 33.3-2 ff.).
- **Status:** **entschieden**.

### AE-05 — Konsole `H6`: Richtung, Serienwiderstand, 3,3-V-Einspeisung

- **Frage:** Wie hängen `TXD0`/`RXD0` am Stecker `H6`; 499 Ω laut HDG; Rolle von `H6.1`.
- **Wahl Richtung:** `H6.2` = **Eingang der Platine** (Netz `TX_UART`, an `RXD0` = `GPIO44`), `H6.3` = **Ausgang der Platine** (Netz `RX_UART`, von `TXD0` = `GPIO43` über `R33`), `H6.4` = GND. Die Netznamen behalten ihren V2-Sinn (Sicht des Adapters).
  **Das ist die Belegung von V2:** `TX_UART` hängt dort an `PA10` (USART1-Empfang, über `SW4` an `H6.2`), `RX_UART` an `PA9` (USART1-Sendung) und an `H6.3`. Ein für V2 verdrahteter Adapter passt damit ohne Änderung.
  **Der Entwurf legt es umgekehrt (Fehler):** `modpin("TXD0", "TX_UART")` und `modpin("RXD0", "RX_UART")` in `gen.py` Z. 132 und die gleiche Regel in `hardware-pinregeln.json`.
  Der Designer tauscht die beiden Zuordnungen; der Lead ändert die Regeln (`pinbelegung.md` §5). Antwort auf OP-35.
- **Wahl Serienwiderstand:** `R33` **510 Ω** (Basic, C17734) in Reihe in `TXD0`, `RXD0` ohne Widerstand.
  Das HDG nennt 499 Ω gegen Oberwellen, Leitung kurz (✔ Blatt §9, HDG S. 13); 499 Ω ist ein E96-Wert ohne Basic-Nummer, 510 Ω liegt 2,2 % darüber (○ Äquivalenz). Bei 115200 Baud (✔ `log-uart` laut REVIEW) und einer Adapterlast von einigen kΩ ohne Wirkung auf den Pegel (○).
- **Wahl `H6.1`:** bleibt wie V2 (3,3-V-Einspeisung über `D3`); Risiko R1, Frage Q1, Option A11.
- **Beleg:** ✔ V2 `nets.json` (`RX_UART` = `H6.3`, `U2.PA9`, `U4.GPIO13`; `TX_UART` = `SW4.2`, `U2.PA10`; `Net-(H6-Pad2)` = `H6.2`, `SW4.3`; `Net-(D3-A)` = `D3.2`, `H6.1`); ✔ `HARDWARE.md` (berichtigt 17.10.2026: `RX_UART` = `PA9` USART1-Sendung, `TX_UART` = `PA10` USART1-Empfang); ✔ Blatt §9; ○ der gemeinsame Kontakt von `SW4` ist nicht belegt (L486), für V3 ohne Belang, weil `SW4` entfällt.
- **Umstösst die Wahl:** Der Nutzer sagt, der Adapter sei anders verdrahtet; ein Konsolenadapter mit 5-V-Pegel (R1).
- **Status:** Richtung **entschieden**; `R33` **offen, wartet auf Analyst** (HDG S. 13 neu lesen: 499 Ω als Wert oder als Beispiel, Toleranz); bis dahin 510 Ω.

### AE-06 — LED-Versorgung: Pin, Pull-down, Schaltkette

- **Frage:** `GPIO6` oder `GPIO21` für `WS2812_POWER`; Pull-down am Gate von `Q2` (`R20`); bleibt die Kette?
- **Wahl:** **`GPIO6`**; die Kette `R19` → `Q2` → `R23`/`R21`/`C10` → `Q1` bleibt wie V2, **`R20` 100 kΩ bleibt**; die Software setzt `GPIO6` als allererste Anweisung auf Low (Spec `design.md` §4.5).
- **Begründung:** `Q2` ist ein N-Kanal-FET (High = EIN) mit Pull-down: Im Reset ist `GPIO6` hochohmig **ohne** internen Pull (✔ Blatt §4.2: GPIO4–8 „nichts“), das 60-µs-Low-Glitch schaltet nichts ein, `R20` hält das Gate auf Masse.
  Die Befürchtung des Blattes `mmbf170.md` §3.1 (2,3 V am Gate, falls der Pin im Reset einen Pull-up von 45 kΩ hätte) trifft auf `GPIO6` nicht zu. `GPIO21` hätte kein Glitch, wäre aber nur für ein P-Kanal-Gate oder ohne Pull-down wichtig (✔ Blatt §4.2 (b)); er bringt hier keinen Gewinn und bleibt als Reserve unbelegt.
  Die Gate-Ansteuerung bleibt bei VGS ≤ −4,5 V für `Q1` (✔ Blatt `si7149dp.md` §8); Einschaltflanke 0,2 … 0,5 ms bleibt (gewollter weicher Anlauf, ○ Rechnung im Blatt §5.1).
  **`R23` und `C10` müssen in die V3-Stückliste** (sie fehlen in der V2-Liste, R4); die Nummer von `R23` liefert der Analyst (AE-17). Der Teile `Q2` und `R19` selbst: Abschnitt 4 (A2, A3).
- **Beleg:** ✔ Blatt §4.2; ✔ `mmbf170.md` §3.1; ✔ `si7149dp.md` §5; ✔ V2 `nets.json` (`Net-(Q2-G)` = `Q2.1`, `R19.2`, `R20.1`; `R20.2` = GND).
- **Umstösst die Wahl:** AK9 (Oszilloskop: LED-Versorgung im Reset, beim Einschalten, beim Flashen nicht an) — bei Durchfall ändert sich die Platine (`GPIO21` oder kleinerer `R20`, Letzteres wäre eine Abweichung von H8), nicht die Software.
- **Status:** **entschieden** (Pin, Pull-down, Kette); **offen, wartet auf Messung** (AK9).

### AE-07 — Pull-ups und Pegel an den Eingängen (F3, F7)

- **Frage:** Welche Schiene haben die Pull-ups von IR und DCF77; sind externe Pull-ups für I²C, 1-Wire und Taster nötig?
- **Wahl und Beleg:**

| Leitung | Beschaltung | Begründung | Beleg |
|---|---|---|---|
| IR (`TSOP`, `GPIO7`), DCF77 (`GPIO15`) | `R8`/`R9` 100 kΩ nach **3,3 V** (wie V2); keine Stecker | F3 beantwortet: Der Pin sieht höchstens 3,3 V. Wer extern ansteckt, tut es mit einem 3,3-V-Empfänger; 5 V an diesen Pins wäre ein Bauteilschaden (Regel 3) | ✔ `nets.json` (`R8.2`, `R9.2` an `VCC_3.3V`) |
| I²C (`GPIO8`/`GPIO9`) | `RN2` 4,7 kΩ nach 3,3 V | Das Firmware-Zeitverhalten ist 100 kHz (Rückfall 50 kHz) ✔ REVIEW `i2c`; Anstiegszeit mit 30 … 60 pF rund 120 … 240 ns (○, aus ✔ `ds3231mz.md` §4), im Rahmen. Interne S3-Pull-ups (typ. 45 kΩ ✔ Blatt §2) genügen dafür nicht | ✔ `nets.json` |
| 1-Wire (`GPIO2`) | `R5` 4,7 kΩ nach 3,3 V | ✔ DS18B20-Blatt §4 (≈ 5 kΩ) | ✔ |
| Taster `USER`, `WPS` | **interner** Pull-up, kein externer (wie V2: der STM-Pull-up trug sie); `USER` mit `R1` 330 Ω in Reihe | 1:1; die Firmware schaltet den Pull-up an (`button.c`, `wpsbutton.c`: „Eingang mit Pull-up, aktiv low“, ✔ REVIEW). 45 kΩ am S3 (✔ Blatt §2); Entprellung in der Software (✔ REVIEW: 5-s-Frist) | ✔ `nets.json`, REVIEW |

- **Umstösst die Wahl:** ein Tastenprellen oder Fehlauslösen am Prototyp (dann externe 10 kΩ, ein Zusatzteil).
- **Status:** **entschieden** (F3, F7); IR/DCF77 ohne Stecker siehe AE-18.

### AE-08 — 3,3-V-Versorgung: SY8301, `D2`, `R29`, Modul-Entkopplung

- **Frage:** Trägt der SY8301 (1 A) den S3; Rückkopplung hinter `D2`; `R29` 32 kΩ gegen 31,6 kΩ; Entkopplung am Modul.
- **Wahl:** **`U10`/`L2`/`D2` bleiben wie V2** (1:1, Rückkopplung hinter `D2`). **`R29` = 31,6 kΩ** im Schaltplan (der Wert des bestellten Teils C2930156; „32 kΩ“ ist weder E24 noch E96, sondern eine Rundung). **`C30` = 22 µF** (Basic C45783, schon in der Stückliste) statt 10 µF im Entwurf, `C31` = 100 nF, beide dicht an Pad 2 des Moduls; `C8` (22 µF) auf demselben Netz bleibt. Ein Kondensator ≥ 4,7 µF dicht an `IN`/`GND` von `U10`: `C26` (10 µF, hängt an `VCC_5V`) wird dort platziert (Auftrag an den Layouter).
- **Begründung:** Strom: 1 A Nennstrom, Spitze bei 0,6 A rund 0,65 A gegen Begrenzung 1,6 … 2,5 A (✔ Blatt `sy8301abc.md` §3, §4.2); realistische Spitze des 3,3-V-Zweigs rund 0,37 A (355 mA Modul ✔ + Rest ○, `leistungsbudget.md` §2). `R29` = 31,6 kΩ liefert 3,332 V (3,298 … 3,365 V mit Referenztoleranz, ✔ Blatt §4.1) — **das ist die Spannung, die V2 heute hat**; sie bestimmt die Teilerspannung des LDR (`H7.3` hängt an `VCC_3.3V`). Ein Basic-Wert (33 kΩ → 3,224 V) senkte alle LDR-Spannungen um 3 %: eine Abweichung von V2 am LDR-Pfad ohne Not. Modul-Entkopplung: Referenz 22 µF + 0,1 µF an VDD33 (✔ DS-M Abb. 9-1, S. 41).
  **Spannungsreserve:** Die Rechnung im Blatt (4,5 V Eingang, 0,6 A: 3,18 / 3,00 / 2,82 V je nach Frequenz) und meine Ergänzung für die realistische Last (`leistungsbudget.md` §5: Reserve 0,04 … 0,12 V über 3,0 V am schlechtesten Punkt, bei 0,6 A darunter) zeigen: **knapp, nicht belegt.** Das Verhalten am Tastgradanschlag steht nicht im Datenblatt.
- **Fallback-Kette, falls AK37 am Prototyp unter 3,0 V fällt** (jeder Schritt eine Abweichung, Abschnitt 4): (1) LED-Strom über USB in der Software begrenzen; (2) `D2` durch 0 Ω ersetzen, `FB` bleibt am Verbraucher (gewinnt rund 0,25 … 0,28 V Dioden-Spannung, ✔ Blatt `sy8301abc.md` §4.4; **der Rückspeiseschutz gegen `H6.1` geht verloren**, A10); (3) ein kleinerer `R29` ist keine Lösung (senkt die Spannung, ändert den LDR).
- **Beleg:** ✔ `sy8301abc.md` §3–§5, §7; ✔ DS-M Abb. 9-1; ○ eigene Rechnung `leistungsbudget.md` §5.
- **Umstösst die Wahl:** AK37 am Prototyp (4,5 / 4,75 / 5,0 V Eingang, WLAN-Dauersenden, hellstes Normalbild, 10 min, Minimum erfassen): unter 3,0 V; ein Neustart mit Brownout-Wort.
- **Status:** **entschieden** (Bauteile, `R29`, `C30`); **offen, wartet auf Messung** (Spannungsreserve, AK37).

### AE-09 — Power-Mux TPS2121

- **Frage:** Strombegrenzung (4,53 A bei 3,6 A Last), `PR1`, `ST`, Beschaltung.
- **Wahl:** `U9` wie V2: `R26` 22,1 kΩ (ILM 4,53 A, Streuung 4,0 … 5,0 A ✔ Blatt `tps2121.md` §4), `C24` 100 nF, `CP2`/`OV1`/`OV2` an GND, `ST` offen, `PR1` direkt an `USB_5V_PROT`. **Anträge A4** (10 kΩ vor `PR1`) und **A5** (`R26` 22 kΩ Basic) im Abschnitt 4.
- **Begründung:** 3,5 A (3 A LED + 0,5 A Logik an 5 V, `leistungsbudget.md` §4) liegen rund 12 % unter dem **Mindestwert** der Begrenzung (4,0 A) und unter dem Dauerstrom-Höchstwert (4,5 A, ✔ `tps2121.md` §3); Verlust im Baustein bei 3,5 A rund 1,0 W (0,085 Ω bei ≤ 85 °C, ✔ S. 7), Erwärmung rund 75 K (72,2 °C/W, JEDEC-Platine ✔ S. 6) — Wärmekupfer unter und um den Baustein (Auftrag an den Layouter, ✔ `tps2121.md` §10 Nr. 3). `ST` offen weicht von „GND, wenn nicht benötigt“ ab, ohne elektrische Folge (✔ Blatt §6).
- **Beleg:** ✔ `tps2121.md` §3–§7; ✔ V2 `nets.json`.
- **Umstösst die Wahl:** Messung des Einschaltverhaltens (Umschaltung 100 µs, Einbruch ≈ 1 V bei 0,6 A auf 57 µF, ○ Blatt §5); die Frage Q3 zur Priorität.
- **Status:** **entschieden** (Beschaltung); A4, A5 **Anträge**.

### AE-10 — Sicherungen `F1`, `F2`

- **Frage:** `F1` (1,5 A) und `F2` (3 A) gegen 3 A LED + 0,6 A.
- **Befund:** Die Haltestrom-Werte stehen nur im Beschreibungsfeld der Stückliste (●: `F1` 1,5 A Halt, 3 A Auslösung, 25 … 130 mΩ, 16 V, 300 ms; `F2` 3 A Halt, 5 A Auslösung, 10 … 60 mΩ, 24 V, 4 s); **die Datenblätter sind nicht gelesen** (○). Rechnung: Entwurfslast aus dem Netzteil 3,5 A > 3 A Haltestrom von `F2`; über USB sind es höchstens 1,5 A Haltestrom von `F1` abzüglich Logik (`leistungsbudget.md` §4).
- **Wahl:** `F1` **bleibt 1,5 A** (1:1; ein PC-Port liefert 500 mA, ein Lader bis 1,5/3 A bei erkanntem Band — die Platine erkennt keine Bänder, ✔ Blatt `usb-c-16p.md` §3.3; `F1` begrenzt den USB-Zweig bewusst). `F2`: **offen** — der Entscheid hängt an der maximalen LED-Last aus dem Netzteil (Q4) und an einem 1812-PTC mit mehr Haltestrom und 24 V (Nummer vom Analysten, nicht geraten).
- **Beleg:** ● `v2.xml` (Felder `F1`, `F2`); ✔ `HARDWARE.md` (Stromaufnahme, Rechnung nicht gemessen); ○ Haltestrom bei Temperatur (Kurve nicht gelesen).
- **Umstösst die Wahl:** Datenblattwerte der PTC (Haltestrom bei 40 … 60 °C); eine Messung von `VCC_5V` unter hellem Display (Testpunkt `TP7`, ✔ `HARDWARE.md`).
- **Status:** `F1` **entschieden**; `F2` **offen, wartet auf Nutzer und Analyst** (A7).

### AE-11 — entfällt

`Q2` (Shikues-MMBF170 oder AO3400A) und `R19` liegen im LED-/Power-Pfad, den H8 sperrt; ein Ersatz zugunsten von Basic ist eine Abweichung von 1:1 und steht deshalb **nur** im Abschnitt 4 (A2, A3), nicht hier.
Bis zur Freigabe bleibt es bei den V2-Teilen (`Q2` C130879, `R19` C17841).

### AE-12 — DS18B20: Nachbau HXY oder Maxim-Original

- **Frage:** `C5451632` (HXY, 400 ms) oder `C9753` (Maxim, 750 ms); Folgen für die Firmware.
- **Wahl:** **`C5451632` (HXY) bleibt** (1:1, TO-92 wie V2). Der Maxim `C9753` ist die benannte Alternative.
- **Begründung:** Die Firmware wartet auf das Umsetzungsende, indem sie Lesebits abfragt, höchstens 13'333 Lesevorgänge, rund 0,87 s am STM (✔ REVIEW `onewire`, `ds18xx.c:181`, `:209-214`); fertig ist der Sensor beim ersten Lesebit 1. HXY (400 ms) hat doppelte Reserve. Beim Maxim (750 ms) hängt die Reserve an der Zeit je Lesevorgang des S3-Treibers: 0,75 s entsprechen 13'333 Lesevorgängen zu je 56 µs; ein Slot dauert nach DS18B20-Blatt mindestens 60 µs (✔ Blatt §4), also ≥ 0,80 s, **Reserve rund 7 %** (○ Rechnung; die Zeit je Lesevorgang des S3-Treibers ist noch nicht gemessen). Weiter: EEPROM des Sensors 1'000 Zyklen (HXY) gegen 50'000 (Maxim) — betrifft nur eine Firmware, die bei jedem Start schreibt (A65 offen, ✔ Blatt §7). **Entscheidend:** Die offenen Befunde A63/A64/A65 (ROM-ID falsch gelesen, 85 °C, Auflösung) darf der Port nicht verdecken (Spec „Nicht Teil dieser Änderung“); ein anderes Teil änderte das Fehlerbild und mischte zwei Ursachen.
  Das Verhalten ändert sich zwischen den Teilen messbar (Umsetzzeit, EEPROM-Zyklen, tWR 8 … 12 gegen 2 … 10 ms); die Firmware braucht für **HXY keine Änderung**, für den Maxim nur die Prüfung der Wartegrenze.
- **Extended-Grund:** kein Basic-Temperatursensor DS18B20 (66 von 66 Treffern durchsucht, ✔ Blatt §6). **Lager 3'465** (klein, für eine Kleinserie ausreichend, am Bestelltag neu prüfen).
- **Beleg:** ✔ `ds18b20.md` §2, §6, §7; ✔ REVIEW (Zeilen genannt).
- **Umstösst die Wahl:** Lager unter Bedarf; ein Befund, dass der HXY-Nachbau die ROM-CRC falsch liefert (Blatt §9, nicht gelesen).
- **Status:** **entschieden**; THT-Bestückung bei JLCPCB **offen, wartet auf Analyst** (OP-21).

### AE-13 — DS3231MZ und die Pull-ups an `32KHZ`/`SQW`

- **Frage:** Bleiben die Pull-ups an `32KHZ` und `SQW` (`RN2` Pin 5/6), die rund 0,7 mA ziehen?
- **Wahl:** **Sie bleiben** (1:1). `U5` = `DS3231MZ+TRL` (C107410, MEMS-Variante, ±5 ppm) unverändert.
- **Begründung:** Beide Ausgänge dürfen offen bleiben (✔ `ds3231mz.md` §3), der Gewinn ist 0,7 mA von rund 365 mA (0,2 %); der Array `RN2` hat die vier Elemente ohnehin, die beiden Pull-ups sind kein eigenes Teil. Die Auslassung wäre eine Änderung ohne Grund; als Antrag A8 nur auf Wunsch.
- **Extended-Grund:** keine Basic-RTC mit I²C. **Lager 2'885**, kleinster der ICs (Bestellung neu prüfen).
- **Beleg:** ✔ `ds3231mz.md` §3, §5, §6.
- **Status:** **entschieden**.

### AE-14 — Pegelwandler `U3`, `OE` an GND

- **Frage:** Reicht der 3,3-V-Ausgang des S3 für den 74AHCT1G125; ist `OE` an GND richtig?
- **Wahl:** **`U3` unverändert** (A am 3,3-V-Pin, `OE` an GND, VCC am geschalteten 5 V über `R13` 0 Ω, `R11` 330 Ω, `R31` 100 kΩ); kein zusätzlicher Pull-down an `GPIO5`.
- **Begründung:** VIH min. 2 V bei VCC 4,5 … 5,5 V gegen VOH min. 0,8 · VDD = 2,64 V bei 3,3 V (2,4 V bei 3,0 V): **Reserve ≥ 0,4 V** (✔ `sn74ahct1g125.md` §5.1; ✔ DS-M S. 27). Eingang bei VCC = 0 V: ≤ 1 µA (✔ AHCT-Blatt §5.2). `OE` an GND ist nicht die TI-Empfehlung für Ein-/Ausschalten (✔ TI S. 8), aber das Verhalten ist V2: `GPIO5` ist beim STM-Reset ebenfalls hochohmig (○); die LED-Versorgung fällt bei jedem Reset binnen rund 1 ms ab (`R20`, Abschaltzeit ○ aus ✔ Blatt `si7149dp.md` §5.1), die Kette kann in dieser Zeit zucken. Ein zusätzlicher Pull-down läge im Netz `WS2812_BUS` (H8 meldet es) und löste nichts, was V2 nicht auch hat.
  `U3` ist im Bereich 4,5 … 5,5 V spezifiziert; unter Volllast sackt `VCC_5V` an die 4,5-V-Grenze (`leistungsbudget.md` §5.3) — wie in V2.
- **Beleg:** ✔ AHCT-Blatt §3, §5; ✔ DS-M S. 27.
- **Offen:** das SK6812-Datenblatt (VIH bei 5 V, ob 3,8 V VOH reicht) ist nicht gelesen (OP-27); das LED-Board ist unverändert, und V2 läuft damit.
- **Status:** **entschieden**.

### AE-15 — Extended-Teile: Grund, Bestand, Alternative

Alle Zahlen: ● JLCPCB-Teilebibliothek, Prüfdatum **2026-10-17** (`jlcpcb-bestand.md` und Komponentenblätter). Basic sind nur die gewöhnlichen Widerstands- und Kondensatorwerte.

| Teil | LCSC | Bestand | Warum kein Basic-Teil | Alternative mit Nummer |
|---|---|---|---|---|
| `U2` ESP32-S3-WROOM-1-N8 | C2913198 | 416 | Kein Funkmodul im Standard-Katalog (✔ Blatt §10); nur „Standard Only“, MSL 3 | N8R2 C2913204 (712, Chip läuft aus); N16 C2913199 (1'439); N4 C2913197 (1'942) |
| `USB1` HRO TYPE-C-31-M-12 | C165948 | 367'528 | Keine Basic-Typ-C-Buchse (Suche unvollständig, je Begriff erste 200 Treffer, ✔ Blatt `usb-c-16p.md` §6) | C5337088 (3'058, 6 A, Footprint nicht verglichen) |
| `U11` USBLC6-2SC6 | C7519 | 36'415 | Kein Basic-ESD-Baustein (45 von 45 Treffern „USBLC6“) | Nachbauten C2687116 (143'251), C2827654 (58'430): andere Kennwerte (0,35 pF), **nicht** als gleich zu behandeln |
| `U10` SY8301ABC | C2927130 | 20'824 | Kein gleichwertiger Basic-Wandler (C9864 TPS5430 ab 5,5 V ✘, C61063 fest 5 V ✘; AMS1117 C6186 verlöre rund 1 W ○) | keine |
| `L2` FTC252010S4R7MBCA | C5832360 | 392'763 | Keine Basic-Induktivität 4,7 µH | keine |
| `D2`, `D3` PMEG2010ER | C82288 | 7'366 | Basic-Schottky (C2480, C8678, C8598) haben 0,2 … 0,26 V mehr VF; mit `FB` hinter `D2` wäre die Reserve um rund 0,25 … 0,33 V an `VIN` schlechter (✔ `sy8301abc.md` §5) | C2480 SS14 (SMA, 1'261'878) nur für `D3`, anderer Footprint |
| `D4`, `D5` SMAJ5.0CA | C10756 | 5'318 | Keine Basic-TVS (Suche „SMAJ5.0“: 0 Treffer) | keine |
| `F1` | C46641031 | 36'666 | Keine Basic-Sicherung | keine |
| `F2` | C20627123 | 45'582 | Keine Basic-Sicherung | PTC mit mehr Haltestrom: Analyst, offen (AE-10) |
| `H4`–`H7` PZ254R | C492411, C492410, C492412, C492415 | 57'418; 174'668; 43'984; 7'093 | Keine Basic-Stiftleiste | keine (`H7` hat den kleinsten Bestand) |
| `LED1` | C98224 | 177'572 | Keine Basic-LED in Grün | keine |
| `LED4`, `LED5` | C284871 | 76'170 | Basic-Rot C84256 ist deutlich heller (195 gegen 54 mcd): eine optische Änderung, Vorwiderstand müsste geprüft werden | C84256 NCD0805R1 (3'887'762) |
| `Q1`, `Q3` SI7149DP | C553982 | 10'088 | Basic-P-FETs sind SOT-23 (AO3401A C15127: rund 0,54 W bei 3 A, zu heiss) | Si7149ADP C554002 (5'610) |
| `Q2` MMBF170 (Shikues) | C130879 | 1'355 | Siehe Abschnitt 4, A2 (Abweichung) | AO3400A C20917 (718'151) |
| `R19` 82 Ω | C17841 | 72'948 | Siehe Abschnitt 4, A3 (Abweichung) | 100 Ω C17408 |
| `R26` 22,1 kΩ | C204311 | 2'095 | Siehe Abschnitt 4, A5 (Abweichung) | 22 kΩ C17560 |
| `R29` 31,6 kΩ | C2930156 | 56'127 | Kein Basic-Wert mit gleicher Zahl; 33 kΩ C17633 ändert die 3,3-V-Schiene (AE-08) | C17633, C17621 (30 kΩ) |
| `RN1`, `RN2` | C29586 | 236'564 | Basic-Array C1980 ist 0603x4, anderer Footprint | C1980 oder vier Einzelwiderstände C17673 |
| `SW1`–`SW3`, `SW5` | C2909692 | 3'938 | Kein Basic-Taster | keine; `SW5` teilt die Nummer, keine zusätzliche Zeile |
| `U1` DS18B20 (HXY) | C5451632 | 3'465 | Kein Basic-Sensor | C9753 Maxim (42'689); C376006 UMW (72'259, Datenblatt nicht gelesen) |
| `U3` SN74AHCT1G125DBVR | C7484 | 51'402 | Kein Basic-Pegelwandler | C350557 (SC-70-5, anderer Footprint) |
| `U5` DS3231MZ+TRL | C107410 | 2'885 | Keine Basic-RTC | keine mit demselben Registerlayout bekannt |
| `U6` AT24C32M/TR | C2688466 | 13'120 | Kein Basic-EEPROM | M24C64 C79988 (Preferred, **nicht** geprüft: Adressierung, Seitengrösse) |
| `U7` CR2032-Halter | C18212226 | **629** | Kein Basic-Halter | keine geprüft — **Bestand und Footprint-Passung offen** (OP-22) |
| `U9` TPS2121RUXR | C485916 | 27'942 | Kein Basic-Teil für Prioritäts-Mux mit 4,5 A | TPS2121RUXT C2156025 (730) |
| `C20`, `C22` Tantal 10 µF | C1969735 | 33'835 | Grund für Tantal in V2 nicht dokumentiert | C15850 (X5R 25 V 0805, Basic) — Wertänderung, siehe A6 |
| `C23` 47 µF 10 V | C124129 | 37'104 | Basic-47-µF-Werte sind 6,3 V oder 1206 | C16780 (6,3 V), C96123 (1206) — siehe A6 |

**Zählung Stand 2026-10-17:** Die V2-Liste nach Abzug der entfallenen Teile hat 27 Extended-Zeilen (`jlcpcb-bestand.md`); dazu kommen die neuen Teile `U2`, `USB1`, `U11` (30 Extended-Zeilen; `SW5` und `C33` teilen vorhandene Nummern). Bei Annahme der Anträge A2, A3, A5 sinkt die Zahl auf 27.
Alle Zeilen sind lieferbar (kleinster Bestand `U7` 629, `Q2` 1'355, `R26` 2'095, `U5` 2'885, `U1` 3'465, `SW` 3'938). H4 verlangt die Prüfung neu, wenn sie älter als 14 Tage ist (ab 2026-11-01).

### AE-16 — Modul, Platzierung, Format (F4, F6, F16)

- **Frage:** Welche Modulvariante, wo sitzt es, hält das Format mit dem 16-poligen Stecker?
- **Wahl Modul:** **ESP32-S3-WROOM-1-N8** (C2913198): 8 MB Quad-Flash, **alle 36 GPIOs frei**, −40 … 85 °C. Zweitwahl N8R2 (pinkompatibel, aber der Chip läuft aus, ✔ Blatt §10). Octal-Varianten (N8R8, N16R8) sperren GPIO35–37 und sind nur bis 65 °C (✔ Blatt §3.3).
- **Wahl Lage:** Das Modul sitzt **dort, wo `U4` (ESP-12F) sitzt**, mit der **Antenne an der Oberkante**: Mittellinie bei x = 127,00 mm, das Antennenende des Moduls dort, wo das Antennenende des ESP-12F lag (der Designer misst das im V2-Layout; **○**, weil der Ursprung des `U4`-Footprints nicht gelesen ist). Das Modul ist 18,0 × 25,5 × 3,1 mm (✔ Blatt §1).
  **Freihaltung:** Antennenfläche 6 mm tief über die ganze Modulbreite, **kein Kupfer und kein Bauteil auf allen Lagen** (✔ DS-M Abb. 11-1, S. 45; HDG Abb. 26, S. 31); keine USB-, UART- oder Testpunkt-Leiter in der Nähe (✔ HDG S. 28; Setzung ○: Abstand zur Antennenfläche ≥ 15 mm, so wie das Blatt es für das Gehäuse nennt).
  Der Umriss bleibt (145 × 39 mm), damit entfällt die Aussparung der Platine, die das HDG empfiehlt (✔ Blatt §8); das Modul sitzt **am Rand ohne Aussparung** — schlechter als die Referenz, aber wie V2 mit dem ESP-12F (○). Die seitlichen 15 mm des HDG (für den Fall der Aussparung) sind nicht einzuhalten: `H6` (103,75 / 4,50) steht rund 9 … 14 mm neben der Antennenzone (○ aus den V2-Koordinaten bei Modulbreite 18 mm, Footprint-Ausdehnung von `H6` nicht gelesen).
  **Messbare Forderung statt Zahl aus dem Gedächtnis:** Der Abstand von `H6`-Stiften und `H10` zur Antennenfläche ist **nicht kleiner als der Abstand dieser Teile zum Antennenende des ESP-12F in V2, abzüglich der halben Mehrbreite des Moduls je Seite** (Setzung ○; der Designer nennt beide Abstände); die **Reichweite entscheidet die RSSI-Messung am fertigen Gerät** (Spec F4).
  **USB-Leitungen:** `USB1` bleibt an der Unterkante in der Mitte (72,50 / 35,38); die Pads 13/14 (`IO19`/`IO20`) liegen auf der Seite des Moduls, die dem Stecker zugewandt ist (○ aus der Pad-Reihenfolge; der Designer prüft gegen die Pinbelegungs-Abbildung DS-M S. 10), die Leitung verläuft im unteren Platinenteil.
- **Wahl Stecker:** **HRO TYPE-C-31-M-12 (C165948)**, 8,94 × 7,35 × 6,28 mm laut Zeichnung (✔ Blatt `usb-c-16p.md` §1; ob „7,35“ Höhe, Tiefe oder Länge der Ansicht heisst, ist aus dem Blatt nicht eindeutig, ○). **Ob das Format 1:1 hält, entscheidet die Aussenmasse der 6-poligen Buchse (C456012) und die Gehäuseöffnung — das ist nirgends belegt** (OP-11, F16). Bis dahin gilt: **Das Layout beginnt für `USB1` nicht.** Gefordert wird vom Analysten: die Aussenmasse beider Stecker (Breite, Höhe der Öffnung, Tiefe, Lage der Öffnung zur Platinenkante), aus Datenblatt oder Zeichnung.
  Kriterium für AK36: Die Öffnung des Steckers liegt in Breite und Höhe innerhalb der Öffnung des 6-poligen (Lage der Mitte gleich); der Footprint-Ursprung darf abweichen (H7: Toleranz 1,0 mm für Stecker).
- **Beleg:** ✔ Blatt §1, §3.3, §8, §10; ✔ `usb-c-16p.md` §1, §7; ✔ Skill `kicad-schema` (Koordinaten); ○ Antennenlage, Nachbarabstände.
- **Umstösst die Wahl:** Aussenmasse des 16-poligen Steckers grösser als die der Gehäuseöffnung (dann entscheidet der Nutzer: Gehäuse ändern oder ein anderer 16-poliger Stecker); eine RSSI-Messung, die deutlich schlechter als V2 ist.
- **Status:** Modul **entschieden, Bestätigung durch den Nutzer angefragt** (F6/OP-23, Q7); Lage **entschieden** (Layout prüft); Stecker-Masse **offen, wartet auf Analyst**.

### AE-17 — Lücke der V2-Unterlagen: `R23`, `C10`, `JP2`

- **Frage:** Was ist an der V2-Platine bestückt?
- **Befund:** `R23` (2,2 kΩ) und `C10` (100 nF) stehen im V2-Schaltplan mit `in_bom` = ja und `dnp` = nein, aber **nicht** in `production/bom.csv` und nicht in den Positionsdaten (✔ `HARDWARE.md` L485; Dokumentation W-4); das LCSC-Feld von `R23` ist C17841 (die Nummer von `R19`, 82 Ω; ✔ `v2.xml`). Fehlt `R23` an der Platine, schaltet `Q1` nicht — ausser `JP2` ist geschlossen.
- **Wahl:** Die V3-Stückliste führt `R23` (2,2 kΩ) und `C10` (100 nF) als bestückte Teile; **die LCSC-Nummer von `R23` trage ich nicht ein, sie prüft der Analyst.** Ob V2 sie trägt, kann nur der Nutzer sagen (OP-01).
- **Umstösst die Wahl:** Der Nutzer meldet, V2 läuft ohne `R23` mit geschlossenem `JP2` — dann ist „1:1“ für diesen Pfad zu klären.
- **Status:** **offen, wartet auf Nutzer** (Q2) **und Analyst** (Nummer).

### AE-18 — IR-Empfänger und DCF77 ohne Stecker

- **Frage:** Die Spec lässt IR und DCF77 „bleiben“, der Controller hat dafür keinen Stecker (OP-36, L491).
- **Wahl:** **Wie V2:** Pull-up und Controller-Pin, kein Stecker, kein Pad (1:1); `GPIO7` und `GPIO15` bleiben für die Firmware belegt. Der Nutzer entscheidet, ob er einen Anschluss will; das wäre eine Änderung von Format und Funktion (neuer Zugang).
- **Beleg:** ✔ `nets.json` (`TSOP` = `R8.1`, `U2.40`; `DCF77` = `R9.1`, `U2.45`); auch `PB9` (PON des DCF77-Moduls) ist in V2 unbeschaltet.
- **Status:** **offen, wartet auf Nutzer** (Q5).

### AE-19 — Vorrang von USB vor der externen Einspeisung im Entwicklungsbetrieb

- **Frage:** Soll USB weiter Vorrang vor `H1` haben, jetzt, wo USB zugleich der Datenanschluss ist?
- **Optionen:** (A) 1:1 lassen (`IN1` = USB); (B) `IN1` und `IN2` tauschen (externe Einspeisung vorrangig; USB speist nur, wenn nichts anderes anliegt) — eine Änderung am Mux, nicht am LED-Pfad; (C) `ST` auswerten (neuer Pin, neue Funktion: verworfen).
- **Empfehlung:** (B) nur auf Wunsch des Nutzers (Abschnitt 4, A9). Bis zur Antwort gilt (A) und die Regel: **Zum Flashen oder Debuggen mit angestecktem PC die externe Einspeisung abstecken oder die LED-Versorgung aus lassen.**
- **Beleg:** ✔ `tps2121.md` §5 (USB gewinnt unabhängig von der Spannung); ✔ `usb-c-16p.md` §3.3 (500 mA ohne Bandauswertung).
- **Status:** **offen, wartet auf Nutzer** (Q3).

---

## 4. Abweichungen von 1:1 (alle brauchen die Freigabe des Nutzers)

Jede Zeile: **Grund**, **Beleg**, **Folge für H8**, und die Zeile **„braucht Freigabe des Nutzers“**. Bis zur Freigabe plant der Designer mit dem V2-Wert (bei A1: mit bestücktem `C32` als Antrag, im BOM als „Antrag A1“ gekennzeichnet). Der Eintrag in `hardware-led-schnittstelle.ausnahmen` ist Sache des Lead und erst nach der Freigabe zulässig.

**Zuerst: Was der Rahmen selbst ändert** (keine Abweichung des Architekten, aber gegenüber V2 nicht „gleich“): 16-poliges `USB1` mit `U11`, `C33`; `U2` Modul statt `U2`/`U4`; `SW5`, `R32`, `R33`, `C30` (22 µF), `C31`, `C1` 1 µF, `USB_DP_M`/`USB_DN_M` mit zwei 0-Ω-Brücken; `H6` an UART0 des Moduls. Keines dieser Teile liegt in `LED_REFS` von H8, mit Ausnahme von A1.

| # | Abweichung | Art | Grund | Beleg | Folge für H8 | Freigabe |
|---|---|---|---|---|---|---|
| **A1** | **`C32` 100 nF** (X7R 0805, C49678, Basic) am Netz `LDR` (Pin `GPIO1`) | **Zusatzteil** im LDR-Pfad | HDG empfiehlt 0,1 µF am ADC-Eingang, die Genauigkeitswerte des Datenblatts gelten mit 100 nF (✔ HDG S. 18, DS-C S. 66). Wirkung: **keine** auf den Gleichwert, Tiefpass τ ≈ 91 … 100 µs (Grenzfrequenz 1,59 … 1,75 kHz), die Messung wird nicht langsamer (✔ Blatt §6.3 (4)); Netzbrummen (100 Hz) wird nicht gefiltert. Der F411 in V2 liegt mit 1 kΩ Quelle und ohne Kondensator ausserhalb der Herstellerregel für Quellimpedanzen (✔ Blatt §6.3 (3)); auf dem S3 entfällt dieser Fehler mit `C32` (Abschätzung ○ des Analysten) | ✔ Blatt §6.3 (3), (4) | H8 meldet ein zusätzliches Teil (`HOCH`); Ausnahme `zusatz:C32` nötig. **Lehnt der Nutzer ab: `C32` wird nicht bestückt (DNP), der Platz bleibt** | **braucht Freigabe des Nutzers** (OP-03) |
| **A2** | **`Q2`: MMBF170 Shikues C130879 → AO3400A C20917 (Basic)** (Footprint SOT-23 gleich) | Teil im Power-Pfad | Das Shikues-Teil ist Extended, **Bestand 1'355** (LCSC selbst zeigt 0), Schwelle bis 2,5 V gegen VOH min. 2,64 V (Reserve 0,14 V im Papierfall), nur mit dem Datenblatt des Nachbaus (ohne Datum, ohne Herstelleranschrift) belegt; der AO3400A schaltet mit Schwelle bis 1,45 V und RDS(on) < 48 mΩ bei 2,5 V mit **≥ 1,2 V** Reserve; Bestand 718'151, Basic (✔ `mmbf170.md` §1, §3.1, §4). **Offen:** die Pinbelegung des AO3400A im SOT-23 ist im Blatt nicht eindeutig gelesen (○ 1 = Gate, 2 = Source, 3 = Drain); der 2N7002 C8545 hat kein belegtes Schwellen-Maximum und ist als Ersatz **nicht** nachgewiesen | ✔ `mmbf170.md` §4, §5, §7 | H8 meldet den Wert (`wert:Q2`); Ausnahme mit Grund nötig; Pins `Q2` unverändert, falls die Belegung stimmt (Analyst prüft) | **braucht Freigabe des Nutzers** (OP-04) |
| **A3** | **`R19`: 82 Ω C17841 (Extended) → 100 Ω C17408 (Basic)** | Wert im Power-Pfad | Einziger Widerstandswert der V2-Liste, der in 0805 nicht Basic ist (✔ `jlcpcb-bestand.md`); Funktion: Gate-Vorwiderstand von `Q2`, dämpft Klingeln, führt keinen Gleichstrom (✔ `mmbf170.md` §3.1: Gate-Strom beim Umschalten 40 mA bei 82 Ω; bei 100 Ω rund 33 mA, ○). **Elektrisch gleichwertig (○)**, keine Messung | ✔ `jlcpcb-bestand.md`; ○ Wirkung | H8 meldet `wert:R19` | **braucht Freigabe des Nutzers** (OP-04) |
| **A4** | **10 kΩ (Basic C17414) in Reihe zwischen `USB_5V_PROT` und `PR1` von `U9`** | Zusatzteil, Power-Pfad (nicht in `LED_REFS`) | `PR1` absolut 6 V, empfohlen 5,5 V (✔ `tps2121.md` §3); Pinstrom ≤ 0,1 µA (✔ S. 8): 10 kΩ verschieben die Schwelle nicht. Klemmspannung von `D4` nicht gelesen, liegt über 6 V (○). Gegen Hot-Plug-Spitzen hinter der PTC | ✔ `tps2121.md` §6; ○ Klemmspannung | keine (nicht in `LED_REFS`); H2 (Netzliste gegen Soll) braucht die Aufnahme in `expected.json` | **braucht Freigabe des Nutzers** |
| **A5** | **`R26`: 22,1 kΩ C204311 (Extended, Bestand 2'095) → 22 kΩ C17560 (Basic)** | Wert im Power-Pfad (nicht in `LED_REFS`) | Strombegrenzung 4,53 A → 4,56 A (+0,6 %), innerhalb der Streuung 4,0 … 5,0 A (✔ `tps2121.md` §4, §10) | ✔ Blatt | keine | **braucht Freigabe des Nutzers** |
| **A6** | **`C20`, `C22`** Tantal 10 V C1969735 (Extended) → C15850 (X5R 25 V, Basic); **`C23`** 47 µF 10 V C124129 → C16780 (6,3 V) | Option, Wert im Power-Pfad | Spart Extended-Zeilen. Gegenargument: Der Grund für Tantal in V2 ist nicht dokumentiert (✔ `jlcpcb-bestand.md`); 6,3 V an 5 V verliert bei X5R stark Kapazität (○, Kurve nicht gelesen); **kein Beleg für Gleichwertigkeit** | ✔ `jlcpcb-bestand.md` | keine | **braucht Freigabe des Nutzers**; Empfehlung: **nicht** tauschen |
| **A7** | **`F2` mit höherem Haltestrom** (1812, ≥ 24 V, Nummer vom Analysten) | Wert im Power-Pfad | Entwurfslast ≈ 3,5 A > 3 A Haltestrom (`leistungsbudget.md` §4, ● Haltestrom) | ● Beschreibungsfeld; ○ Temperaturgang | keine | **braucht Freigabe des Nutzers**; hängt an Q4 |
| **A8** | **Pull-ups an `32KHZ`/`SQW` weglassen** (`RN2` Pin 5, 6 unbeschaltet) | Option | Spart 0,7 mA (✔ Rechnung `ds3231mz.md` §5) und ein 32-kHz-Störsignal | ✔ `ds3231mz.md` §5, §8 | keine | **braucht Freigabe des Nutzers**; Empfehlung: nicht nötig |
| **A9** | **`IN1`/`IN2` des TPS2121 tauschen** (externe Einspeisung vorrangig) | Option, Power-Mux (nicht in `LED_REFS`) | Risiko R7: USB ist jetzt Datenanschluss | ✔ `tps2121.md` §5 | keine; H2-Soll-Netzliste ändert sich | **braucht Freigabe des Nutzers** (Q3) |
| **A10** | **`D2` durch 0 Ω ersetzen** (gewinnt rund 0,25 … 0,28 V Spannungsreserve) | Fallback bei Durchfall von AK37 | Verliert den Rückspeiseschutz gegen `H6.1` (○ Zweck von `D2`, OP-28) | ✔ `sy8301abc.md` §4.4, §4.6 | keine | **braucht Freigabe des Nutzers**; nur bei Messdurchfall |
| **A11** | **`H6.1` ohne 3,3-V-Einspeisung** (`D3` entfernen oder Pin 1 nicht verbinden) | Option gegen R1 | Entfernt die Möglichkeit, das 3,3-V-Netz über den Adapter auf 5 V zu heben; verliert die Fähigkeit, die Platine über `H6` zu speisen (Funktion V2) | ✔ `nets.json`; ✔ `HARDWARE.md` | keine | **braucht Freigabe des Nutzers** (Q1) |

**Keine Abweichung, sondern eine Berichtigung:** `R29` im Schaltplan von „32 kΩ“ auf „31,6 kΩ“ (der Wert des Teils, das V2 trägt, AE-08).

---

## 5. Fragen an den Nutzer (zusammengefasst)

| # | Frage | Was daran hängt |
|---|---|---|
| **Q1** | Welchen Konsolenadapter steckst Du an `H6` — Versorgungspin 3,3 V oder 5 V, Datenpegel 3,3 V oder 5 V? Soll `H6.1` weiter 3,3 V in die Platine speisen können? | R1 (zerstört das Modul), A11, Siebdruck |
| **Q2** | Ist an Deiner V2-Platine `R23` (2,2 kΩ) und `C10` (100 nF) bestückt, und ist die Lötbrücke `JP2` geschlossen? Kannst Du den Widerstand zwischen Drain `Q2` und Gate `Q1` messen? | AE-17, Stückliste V3, was „1:1“ für die LED-Versorgung heisst |
| **Q3** | Willst Du zum Flashen/Debuggen am PC die Uhr gleichzeitig am Netzteil haben? Dann ist der Vorrang des TPS2121 zu tauschen (A9). | R7, AE-19 |
| **Q4** | Welche maximale LED-Last soll das Netzteil (`H1`) tragen: 3 A LED (Entwurfswert) oder weniger? | `F2` (A7), Spannungsreserve |
| **Q5** | Sollen IR-Empfänger und DCF77 einen Anschluss am Controller bekommen (neu, wäre eine Abweichung) oder bleibt es wie V2? | AE-18 (L491) |
| **Q6** | Freigabe der Abweichungen A1 (`C32`), A2 (`Q2`), A3 (`R19`), A4, A5; die Optionen A6 bis A11 nur bei Bedarf | H8-Ausnahmen (Lead) |
| **Q7** | Bestätigung des Moduls ESP32-S3-WROOM-1-N8 (Bestand 416 am 2026-10-17) | Bestellmenge (F6) |
| **Q8** | Welche Werte `ldr_min_value`/`ldr_max_value` stehen im EEPROM der V2-Uhr? Liegt `ldr_max_value` über 1055? | AE-01, Dämpfung 0 dB oder 2,5 dB |

---

## 6. Aufträge an die anderen Rollen

| An | Auftrag | Blockiert |
|---|---|---|
| **`hw-datasheet-analyst`** | (1) **Aussenmasse** des 6-poligen Steckers C456012 (Breite, Höhe der Öffnung, Tiefe, Lage zur Kante) und des 16-poligen C165948 (Eindeutigkeit von „7,35“), mit Seite — **vor dem Layout**; (2) **richtige LCSC-Nummer für `R23` 2,2 kΩ 0805** (nicht raten), Klasse, Bestand; (3) HDG S. 19: Wert und Schiene des externen D+-Pull-ups, Verträglichkeit mit dem internen; HDG S. 13: 499 Ω als Wert oder Beispiel; (4) Datenblätter `F1` (1206, C46641031) und `F2` (1812L300, C20627123): Haltestrom über Temperatur, Auslösezeit; ein 1812-PTC ≥ 4 A Haltestrom, 24 V mit Nummer; (5) Pinbelegung AO3400A im SOT-23 gegen den Footprint; (6) Klemmspannung von `D4`; (7) THT-Bestückung (`H4`–`H7`, `U1`, Schirmlaschen `USB1`); (8) Alternativen für `U7` (Bestand 629) mit gleichem Footprint; (9) Stiftstrom `PZ254R-11-07P` (`H7`: 3 A über je zwei Stifte, `leistungsbudget.md` §4); (10) JLCPCB-Bestände neu vor jeder Bestellung (H4: 14 Tage) | Layout `USB1` (1), Stückliste (2), Bestückung (7) |
| **`hw-designer`** | `TXD0`/`RXD0` tauschen (AE-05); `R33`, `USB_DP_M`/`USB_DN_M` mit zwei 0-Ω-Brücken, unbestückte Plätze für zwei C und den D+-Pull-up, `C33` (AE-02); `C30` = 22 µF (AE-08); `R29` = 31,6 kΩ; `R23` und `C10` in die Stückliste mit den Nummern des Analysten (AE-06, AE-17); `expected.json` neu erzeugen; `D2`/`D3`-Footprint SOD-123W gegen das V2-Layout (OP-07) | Gutachten |
| **`hw-layouter`** | Lage `SW5` (AE-03); Antennenfreihaltung und Abstände (AE-16); `U11` an `USB1`, Stich ≤ 3,5 mm, Paar 90 Ω; `C26` an `U10`; Wärmekupfer unter `U9`; Leiterbahnen und Rückleiter für 3,6 A (Breite durch den Analysten zu belegen) | — |
| **Lead** | `hardware-pinregeln.json` nach `pinbelegung.md` §5 ändern (`pinbelegung` elf Einträge; `TXD0`/`RXD0`/USB-Netze); H3 einmal absichtlich verletzen; Ausnahmen in `hardware-led-schnittstelle.ausnahmen` erst nach der Freigabe des Nutzers (A1–A3) | H3, H8 |
| **`spec-writer`** (über den Lead) | E9(a), F15, H9, `design.md` §4.2, AK17/AK17b: `R22` bleibt 1 kΩ, 0 dB, Mittelung ≥ 16, V2-Grenzen mit ±1 Stufe gültig; F1, F2, F3, F7, F17: Antworten aus diesem Protokoll; Konsole `H6` Richtung (AE-05) | Spec-Freigabe |
| **`hw-reviewer`** | Prüfkriterien Abschnitt 8; Rückfragen hier beantwortet | — |

---

## 7. Schnittstelle zur Firmware — was die Platine liefern muss

Gelesen in `REVIEW-2026-10-17-hardware-naht.md` (Momentaufnahme, vom Analysten nach `src/**`, Zeilen dort genannt) und in den Komponentenblättern. **Ich habe `src/**` nicht Zeile für Zeile selbst gelesen**, die Zeilenangaben stammen aus dem Inventar (✔ dort).

| Baustein | Was die Firmware voraussetzt | Was die Platine liefert | Beleg |
|---|---|---|---|
| LED-Daten | 800 kbit/s, Periode 1250 ns, T0H 300 ns, T1H 750 ns, Pause 250 µs, Push-Pull-Ausgang; der STM-Treiber prüft beim Init den Pin (Pulldown, High → Open-Drain) | `GPIO5` Push-Pull an `U3` (nur Eingang A), kein Pull am Pin, `U3` erwartet VIH 2 V | ✔ REVIEW `sk6812`; ✔ AHCT-Blatt |
| LED-Versorgung | Push-Pull-Ausgang, Anfangszustand Low (`power.is_on` 0xFF), `delay_msec(200)` nach `power_on()` (`display.c:2954-2955`) | `GPIO6` → `R19` → `Q2`; Einschaltflanke 0,2 … 0,5 ms, `R20` im Reset | ✔ REVIEW `power`; ✔ `si7149dp.md` §5.1 |
| LDR | 12 Bit, Zähler 0 … 4095 gegen 3,3 V, Messung alle 250 ms, `ldr.c` klemmt Kopie auf `ldr_min_value`/`ldr_max_value` | `GPIO1` (ADC1), `R22` 1 kΩ, 0 dB, Mittelung, `N = V_mV · 4095 / V_rail` (Software) | ✔ REVIEW `adc`, `ldr`; AE-01 |
| I²C | 100 kHz, Rückfall 50 kHz (`main.c:3744`); 8-Bit-Adressen 0xA0 (Suchschleife bis 0xAE), 0xD0; Frist 5 ms je Flag; Bus-Freitakten | `GPIO8`/`GPIO9`, 4,7 kΩ nach 3,3 V; EEPROM-Adresse 7-Bit **0x57** (E0–E2 = 1), RTC 0x68 | ✔ REVIEW `i2c`; ✔ `at24c32.md` §3.1 |
| 1-Wire | Open-Drain-Pin, Slots mit 5-µs-Auflösung, Wartegrenze bis 0,87 s | `GPIO2`, 4,7 kΩ nach 3,3 V; HXY-Sensor | ✔ REVIEW `onewire`; AE-12 |
| IR | Eingang **ohne** Pull (`irmpconfig.h:127-141`), Abtastung mit `F_INTERRUPTS` (Spec F14: 20 kHz) | `GPIO7`, `R8` 100 kΩ nach 3,3 V, kein Stecker | ✔ REVIEW `irmp` |
| DCF77 | Eingang, 10-ms-Takt, `PON` (PB9) in V2 unbeschaltet | `GPIO15`, `R9`, kein Stecker, kein `PON` | ✔ REVIEW `dcf77`; ✔ `nets.json` |
| Taster, Status-LED | Eingang mit Pull-up, aktiv Low; LED aktiv Low | `GPIO16`/`GPIO17` interner Pull-up; `GPIO18` mit `LED1`/`R6` | ✔ REVIEW; AE-07 |
| Konsole | 115200 Baud (`log-uart.c`) | UART0 an `H6` (`GPIO43`/`GPIO44`), USB-Serial/JTAG am `USB1` | ✔ REVIEW `log`; AE-05 |
| Brücke, Flashen des STM, `RST_ESP`, `GPIO0_ESP`, `ESP_EN`, `RXD_ESP`/`TXD_ESP` | entfallen mit dem zweiten Chip | keine Hardware dafür | Spec E6 |
| Reset | `esp_restart()` für „Neustart“; Task-Watchdog, Brownout im Chip | `EN`: `SW3`; kein externer Wächter | AE-03; Spec F10 |

---

## 8. Prüfkriterien für den Reviewer (gegen die der Entwurf gemessen wird)

Jedes Kriterium ist **zählbar oder messbar**; die Prüfung nennt, wie viele Dinge sie betrachtet hat (DIR-014).

| # | Kriterium | Wie |
|---|---|---|
| **P1** | Elf Signale an den Pins aus `pinbelegung.md` §2 (Netz → Modulpin); kein Netz der sechs 5-V-Netze an einem Modulpin | H3 mit `pinbelegung`; Mengen von `Ref.Pin`, Zahl der Netze nennen |
| **P2** | `IO3`, `IO45`, `IO46`, `IO35`–`IO38`, `IO48` ohne Verbindung; zwölf Reserve-Pins ohne Stift oder Pad | Netzliste |
| **P3** | Netz `BOOT` = {`U2.IO0`, `SW5`, `R32`}, **kein Kondensator** | Netzliste |
| **P4** | Netz `EN` = {`U2.EN`, `R12` 10 kΩ nach 3,3 V, `C1` 1 µF nach GND, `SW3`}; nichts sonst | Netzliste |
| **P5** | Konsole: `U2.IO43` → `R33` 510 Ω → `H6.3` (Netz `RX_UART`); `U2.IO44` ← `H6.2` (Netz `TX_UART`); `H6.1` = `Net-(D3-A)`, `H6.4` = GND | Netzliste; Vergleich mit V2 zeigt dieselben Pins am Stecker |
| **P6** | USB: `A6`+`B6` am Netz `USB_DP`, `A7`+`B7` an `USB_DN`; `U11` Pins 1/6 an `USB_DP`, 3/4 an `USB_DN`, 5 an `USB_5V_INPUT`, 2 an GND; je Leitung eine 0-Ω-Brücke zwischen `USB_DP` und `USB_DP_M` bzw. `USB_DN` und `USB_DN_M`; `U2.IO20` an `USB_DP_M`, `U2.IO19` an `USB_DN_M`; `SBU1`/`SBU2` offen | Netzliste |
| **P7** | `CC1`/`CC2` je über **einen eigenen** 5,1 kΩ (±1 %, C27834) nach GND, nie verbunden; alle vier VBUS-, alle vier GND-Pins und der Schirm angeschlossen | Netzliste |
| **P8** | Layout: `U11` in ≤ 5 mm Abstand der `USB1`-Pads (Setzung ○), D+/D− laufen durch die Pins 1→6 und 3→4, Stich A6/B6 und A7/B7 ≤ 3,5 mm, Differenzpaar 90 Ω ± 10 % mit durchgehender Bezugsmasse | DRC und Sichtprüfung |
| **P9** | `U2` mit `C30` 22 µF und `C31` 100 nF am Pad 2, `C8` 22 µF im selben Netz; EPAD an GND; USB- und UART-Leitungen und Testpunkte ≥ 15 mm von der Antennenfläche (Setzung ○); Antennenzone 6 mm kupfer- und bauteilfrei auf allen Lagen | Netzliste, Layout |
| **P10** | LDR-Netz = {`H5.2`, `H7.5`, `R22.2` (1 kΩ), `U2.IO1`, `C32.1`} (`C32` nur bei freigegebenem A1); `R22.1` an GND | Netzliste, H8 |
| **P11** | H8 grün, ausser den **freigegebenen** Ausnahmen (A1–A3); `R23`, `C10`, `R19`, `R20`, `R21`, `Q1`, `Q2`, `U3`, `R11`, `R13`, `R31`, `JP2`, `C6`, `H4`, `H5`, `H7` mit den V2-Werten und -Anschlüssen; **`R23` und `C10` stehen in der Stückliste** mit der geprüften Nummer | `hardware.py --nur H8`, H4 |
| **P12** | `R29` = 31,6 kΩ im Schaltplan und in der Stückliste (C2930156) | Stückliste |
| **P13** | `U9`: `R26` 22,1 kΩ (oder 22 kΩ nach A5), `C24` 100 nF, `CP2`/`OV1`/`OV2` an GND, `ST` offen; `PR1` an `USB_5V_PROT` (oder über 10 kΩ nach A4) | Netzliste |
| **P14** | Format (H7, 14 Merkmale): Umriss 145,00 × 39,00 mm; `H8`/`H9`/`H10`, `H1`/`H2` ≤ 0,05 mm, Stecker und Taster ≤ 1,0 mm; `USB1`-Öffnung innerhalb der des 6-poligen Steckers, sobald deren Masse belegt sind | `hardware.py --nur H7`; Zahl der Merkmale nennen |
| **P15** | Stückliste: jede Zeile eine geprüfte LCSC-Nummer; Klasse und Bestand mit Prüfdatum ≤ 14 Tage; jede Extended-Zeile hat den Grund aus AE-15; Zeilenzahl je Klasse am Ende; `R23`/`C10`/`C32`/`C33`/`R33`/`R32`/`SW5`/`U11`/`USB1`/`U2` enthalten | H4 |
| **P16** | Die Entscheide AE-01 bis AE-19 (AE-11 entfällt) sind im Schaltplan so umgesetzt, wie sie hier stehen; jede Abweichung davon steht im Gutachten | Gutachten |
| **P17** | Nichts im Schaltplan gegen den Rahmen: kein Teil am LED-Board, kein anderer `H7`-Pin, kein anderer Umriss | H7, H8 |
| **P18** | **Am Gerät (nicht Papier):** AK9 (LED-Versorgung aus im Reset, beim Einschalten und Flashen, Glitch-Dauer eintragen), AK37 (3,3-V-Minimum ≥ 3,0 V bei 4,5/4,75/5,0 V, WLAN-Dauersenden, hellstes Normalbild, 10 min), AK3 mit Rückfall 2 (BOOT + EN), AK17 mit 0 dB | Messung (Lead, Nutzer) |

---

## 9. Rückfragen des Reviewers

Keine bis zum 17.10.2026 (`hardware/review/` ist leer). Hier eingetragen werden sie mit Datum und Antwort.
