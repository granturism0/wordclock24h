# Leistungsbudget der Controller-Platine (ESP32-S3-WROOM-1-N8)

Stand: **17.10.2026**. Schreiber: `hw-architect`. Lebendes Dokument (DIR-006). Die Entscheide dazu stehen in `entscheidungen.md` (AE-08 bis AE-10, AE-19).

**Beleg-Zeichen:** ✔ gelesen (Quelle, Seite; Komponentenblätter unter `hardware/komponenten/` heissen im Folgenden *Blatt*), ● Sekundärquelle (Beschreibungsfeld der Stückliste, Distributor-Datenbank),
○ Annahme oder **eigene Rechnung** (Formel und Eingangswerte stehen dabei). **Nichts hier ist am Gerät gemessen.** Die Messung ist AK37 der Spec und das Kriterium P18 in `entscheidungen.md`.

## 1. Spannungsdomänen und Pegel

```
USB-C (USB1) ─ D4 (TVS) ─ F1 (PTC) ─ USB_5V_PROT ─┐
                                                   ├─► U9 TPS2121 ─► VCC_5V ─┬─► Q1 ─► TO_VCC_RGB_5V ─► H7.1/2, H4.1, U3 (über R13)
H1/H2 ─ D5 (TVS) ─ F2 (PTC) ─ Q3 ─ ALT_5V_PROT ───┘   (USB hat Vorrang)       └─► U10 SY8301 ─► L2 ─► D2 ─► VCC_3.3V
H6.1 ─ D3 ───────────────────────────────────────────────────────────────────────────────────────────────► VCC_3.3V (Einspeisung)
```

| Domäne | Netze | Bereich | Verbraucher | Grenze | Beleg |
|---|---|---|---|---|---|
| USB-Eingang | `USB_5V_INPUT`, `USB_5V_PROT` | 4,75 … 5,5 V (Rp-Bereich der Quelle) | TPS2121 `IN1`/`PR1`, `U11` Pin 5 | `PR1` absolut 6 V, empfohlen 5,5 V; `U11` sperrt bis 5,25 V, `VBR` ≥ 6 V | ✔ Blatt `usb-c-16p.md` §3.3; `tps2121.md` §3; `usblc6-2sc6.md` §2 |
| Externe Einspeisung | `ALT_5V_INPUT`, `ALT_5V_PROT` | Netzteil ≈ 5 V | TPS2121 `IN2` (absolut 24 V) | `Q3` 30 V; TVS `D5` SMAJ5.0CA | ✔ Blatt `tps2121.md` §3, `si7149dp.md` §3 |
| 5-V-Schiene | `VCC_5V`, `TO_VCC_RGB_5V` | 4,5 … 5,5 V | `U10` (4,5 … 40 V, UVLO 4,0 V), `U3` (VCC 4,5 … 5,5 V), LED-Kette (3,5 … 5,5 V), `LED4` | `U3` ausserhalb von 4,5 V nicht spezifiziert | ✔ Blatt `sy8301abc.md` §3; `sn74ahct1g125.md` §2; ✔ `HARDWARE.md` (SKC6812 3,5 … 5,5 V) |
| 3,3-V-Schiene | `VCC_3.3V` | Soll 3,332 V (3,249 … 3,417 V mit ±1-%-Teilern, ○) | Modul (3,0 … 3,6 V), `U1`, `U5`, `U6`, Pull-ups, `H7.3` (LDR), `H5.1` | Modul absolut 3,6 V; empfohlen 3,0 / 3,3 / 3,6 V | ✔ DS-M Tab. 6-1/6-2, S. 27; ✔ Blatt `sy8301abc.md` §4.1 |

**Pegelwechsel.** Es gibt genau **einen** Übergang von 3,3 V nach 5 V: `GPIO5` → `U3` (74AHCT1G125, VIH ≥ 2 V bei VCC 4,5 … 5,5 V; Modul VOH ≥ 2,64 V bei 3,3 V, 2,4 V bei 3,0 V; Reserve ≥ 0,4 V,
✔ Blatt `sn74ahct1g125.md` §5.1). `GPIO6` treibt das Gate von `Q2` mit 3,3 V (N-Kanal, Schwelle bis 2,5 V, siehe `entscheidungen.md` A2); die 5 V liegen am Drain-Zweig (`R21`), nicht am Pin.
Kein 5-V-Netz berührt einen Modulpin (`pinbelegung.md` §6; H3 prüft es). Der LDR-Abgriff hat höchstens 3,3 V (Teiler an `VCC_3.3V`).
**Eingang `H6.2` (`GPIO44`) und Einspeisung `H6.1`: siehe `entscheidungen.md` R1** — ein 5-V-Adapter überschreitet die Grenze des Moduls.

## 2. Verbraucher an der 3,3-V-Schiene

| Verbraucher | Strom | Beleg |
|---|---|---|
| ESP32-S3-WROOM-1-N8, WLAN-Senden, Spitze | **355 mA** (802.11b, 1 Mbit/s, +20,5 dBm, 100 % Sendeanteil); 297 mA 802.11g/54 Mbit/s; 286 mA HT20/MCS7 | ✔ DS-M Tab. 6-4, S. 28 |
| Modul, Empfang | 95 mA | ✔ DS-M Tab. 6-4 |
| `U1` DS18B20 | höchstens 1,5 mA (Wert bei 5 V, bei 3,3 V ○) | ✔ Blatt `ds18b20.md` §2 |
| `U5` DS3231MZ, I²C aktiv | höchstens 0,2 mA | ✔ Blatt `ds3231mz.md` §2 |
| `U6` AT24C32, Schreiben | höchstens 2,0 mA | ✔ Blatt `at24c32.md` §2 |
| `RN2`-Pull-ups an `32KHZ`/`SQW` | rund 0,7 mA Dauerstrom | ✔ Rechnung im Blatt `ds3231mz.md` §5 |
| `LED5` (Betriebsanzeige über `R30` 510 Ω) | rund 2,5 mA (Vf 2 V angenommen, **nicht gelesen**) | ○ |
| `LED1` (Status über `R6` 510 Ω), wenn an | rund 2,5 mA | ○ |
| LDR-Teiler, hell (LDR 10 kΩ, `R22` 1 kΩ) | 0,3 mA | ✔ Blatt `esp32s3-wroom-1.md` §6.2 |
| I²C-/1-Wire-Pull-ups, wenn Leitung Low (SDA, SCL, DQ) | rund 0,7 mA je Leitung; für zwei gleichzeitig 1,4 mA | ○ (3,3 V / 4,7 kΩ) |

**Spitze der Summe (WLAN-Senden, Schreiben ins EEPROM, beide LEDs an): 355 + 1,5 + 0,2 + 2,0 + 0,7 + 2,5 + 2,5 + 0,3 + 1,4 = 366 mA ≈ 0,37 A.** (○ Rechnung aus ✔-Werten und ○ für die LEDs und die Pull-ups.)
Der **Entwurfswert** der Komponentenblätter und der Spec ist **0,6 A** (Annahme, sie deckt auch PSRAM-Module und Reserve); beide Werte werden unten geführt.
Mittlerer Strom bei Dauerempfang rund 0,1 A (95 mA + Kleinverbraucher, ○); das Senden ist ein Burst.

## 3. Verbraucher an der 5-V-Schiene

| Verbraucher | Strom | Beleg |
|---|---|---|
| 3,3-V-Wandler, bei 0,37 A am Ausgang | rund **0,30 A** (0,28 … 0,32 A bei Wirkungsgrad 0,85 … 0,75) | ○ `I5 = 3,3 V · I3 / (η · 5 V)`; **der Wirkungsgrad bei 5 V → 3,3 V ist nicht belegt** (das Datenblatt zeigt Kurven für 12/24 V → 5 V, ✔ Blatt `sy8301abc.md` §8 Nr. 4), η = 0,80 ○ |
| 3,3-V-Wandler, bei 0,6 A (Entwurfswert) | rund **0,50 A** (0,47 … 0,53 A) | ○ wie oben |
| `LED4` (Betriebsanzeige über `R27` 510 Ω an `VCC_5V`) | rund 6 mA (Vf 2 V ○) | ○ |
| `U3`, Mehrstrom bei High am Eingang | höchstens 1,5 mA | ✔ Blatt `sn74ahct1g125.md` §5.1 |
| `U9` TPS2121, Ruhestrom | 0,3 … 0,4 mA | ✔ Blatt `tps2121.md` §3 |
| **LED-Kette, alle 114 aus** | **0,033 A** (114 · 0,29 mA) | ✔ `HARDWARE.md` (Ruhestrom je LED 0,29 mA, Herstellerangabe) |
| LED-Kette, 40 LEDs weiss über W-Kanal | rund 0,8 A | ○ `HARDWARE.md` (20 mA je Kanal angenommen) |
| LED-Kette, 40 LEDs weiss über R+G+B | rund 2,4 A | ○ dito |
| LED-Kette, **Entwurfswert** | **3,0 A** | Annahme der Spec und der Blätter |
| LED-Kette, alle 114 LEDs auf vier Kanälen voll | rund 9,1 A | ○ dito; **kein Stromlimit in der Firmware gefunden** (Suche in `src/**` nach `max_current`, `current_limit`, `milliamp`: nur `MAX_BRIGHTNESS` 15 als Helligkeitsstufe) |

Logik ohne LED-Kette an `VCC_5V`: **realistisch 0,30 + 0,006 + 0,0015 + 0,0004 ≈ 0,31 A, Entwurf 0,50 + … ≈ 0,51 A.**

## 4. Szenarien und Grenzen der Pfade

Gesamtstrom an `VCC_5V` = Logik (0,31 A realistisch, 0,51 A Entwurf) + LED-Strom (im Szenario S0 der Ruhestrom 0,033 A). Gerundet auf 0,1 A, S0 auf 0,01 A.

| Szenario | LED | gesamt realistisch / Entwurf | PC-Port 0,5 A (✔) | `F1` hält 1,5 A (●) | TPS2121-Mindestbegrenzung 4,0 A (✔) | `F2` hält 3,0 A (●) |
|---|---|---|---|---|---|---|
| S0 alle LEDs aus, WLAN-Spitze | 0,03 A | 0,34 A / 0,54 A | realistisch **ja**, Entwurf **nein** | ja | ja | ja |
| S1 40 LEDs weiss (W-Kanal) | 0,8 A | 1,1 A / 1,3 A | **nein** | ja (Reserve 0,4 A / 0,2 A) | ja | ja |
| S2 40 LEDs weiss (RGB) | 2,4 A | 2,7 A / 2,9 A | nein | **nein** | ja | ja (Reserve 0,3 A / 0,1 A) |
| S3 Entwurfswert | 3,0 A | 3,3 A / **3,5 A** | nein | **nein** | ja (Reserve 17 % / 12,5 %) | **nein** (beide über 3,0 A) |
| S4 alle voll (○) | 9,1 A | 9,4 A | nein | nein | **nein** (Begrenzung greift bei 4,0 … 5,0 A; schnelle Abschaltung bei 2,4 · ILM ≈ 10,8 A in unter 1 µs, ✔ Blatt `tps2121.md` §4) | nein |

**Schlüsse (○ aus den ✔/●-Werten):**

1. **Über USB sind rund 1,0 … 1,2 A für die LED-Kette das Äusserste** (`F1` 1,5 A Haltestrom minus 0,3 … 0,5 A Logik, bei 23 °C; der Haltestrom sinkt mit der Temperatur, Kurve nicht gelesen). **Praktisch begrenzt früher die Spannungsreserve** (Abschnitt 5). Ein PC-Port (500 mA, ✔ SPEC S. 218) trägt die Logik, die LED-Kette kaum: Dimmt die Uhr dort stark oder aus, geht es.
2. **`F2` ist mit 3,0 A Haltestrom kleiner als die Entwurfslast von 3,5 A** (Risiko R8) und auch kleiner als S3 realistisch (3,3 A). Die Aussage hängt am Haltestrom über Temperatur, den ich aus der Stückliste (●) nicht habe.
3. **TPS2121:** ILM = 65,2 / R26^0,861 = 4,53 A (✔ Blatt `tps2121.md` §4), Streuung 4,0 … 5,0 A; Dauerstrom höchstens 4,5 A. Entwurf 3,5 A = **12,5 % unter dem Mindestwert** der Begrenzung. Verlust 3,5² · 0,085 Ω = **1,04 W** (RON ≤ 85 °C ✔ S. 7), Erwärmung 1,04 · 72,2 = **75 K** (JEDEC-Platine, unsere Kupferfläche ○ anders) → bei 40 °C Gehäuseluft **115 °C**, empfohlenes Maximum 125 °C, Abschaltung 160 °C: **schmale thermische Reserve**, deshalb Wärmekupfer (Auftrag an den Layouter).
4. **`Q1`** bei 3 A: 85 mW (RDS(on) max 9,4 mΩ), +5,5 K; **`Q3`** bei 3,6 A: 0,12 W; `F1`-Verlust bei 1,5 A: 1,5² · 0,13 Ω = 0,29 W (Pd 1 W ●). Alle unkritisch (✔ Blatt `si7149dp.md` §5).
5. **`H7`:** Der LED-Strom läuft über zwei Stifte `H7.1`/`H7.2` (5 V) und zwei Stifte `H7.6`/`H7.7` (GND), bei 3 A also 1,5 A je Stift. **Der Stiftstrom von `PZ254R-11-07P` ist nicht belegt** (○), ebenso der Querschnitt des Flachbandkabels; beides ist LED-Board-Seite und wird nicht geändert (Auftrag: Analyst, `entscheidungen.md` §6).
6. **Leiterbahnen:** `VCC_5V`, `TO_VCC_RGB_5V` und der GND-Rückleiter sind für 3,6 A auszulegen; Breite und Kupferfläche weist der Layouter mit einer belegten Rechnung nach (Zahl nicht aus dem Gedächtnis).
7. **Einschalten:** `C23` + `C26` = 57 µF nominell an `VCC_5V`, TPS2121-Softstart 780 V/s ⇒ rund 44 mA, 6,4 ms bis 5 V (✔ Blatt `tps2121.md` §6). Der SY8301 lädt beim Anlauf rund 88 µF nominell (`C27`, `C28`, `C29`, `C8`, `C30`, `C31`) in 1 ms Softstart: ≈ 0,29 A zuzüglich Last (○, ohne Kapazitätsverlust unter Vorspannung). Beides liegt unter 0,5 A.

## 5. Spannungsreserve der 3,3-V-Schiene (die eigentliche Frage)

**Eingangswerte (✔ Blatt `sy8301abc.md` §3, §4):** `RDS(on)` oben 0,38 Ω, unten 0,18 Ω (typ.), `L2` Rdc 0,16 Ω typ. (0,18 max.), Mindest-Ausschaltzeit 100 ns (typ.), Schaltfrequenz 1,6 / 2,0 / 2,4 MHz,
`D2` VF 0,22 V bei 0,1 A und 0,29 V bei 0,7 A (typ.), **`FB` hinter `D2`**, Soll 3,332 V (`R29` = 31,6 kΩ, AE-08).
**Rechenmodell (○, Annahme: konstante Frequenz bis zum Tastgradanschlag, den das Datenblatt nicht beschreibt):**
`Dmax = 1 − 100 ns · fSW`, `V_Knoten = D · VIN − I · (D · 0,38 + (1 − D) · 0,18 + 0,16)`, `V_Last = V_Knoten − VF(I)`; VF interpoliert (0,25 V bei 0,37 A, 0,28 V bei 0,6 A, ○). Das Modell reproduziert die Tabelle des Blattes (0,6 A: 4,89 V bei 2,0 MHz gegen 4,88 V dort).

### 5.1 Eingangsspannung `VIN` am `U10`, ab der 3,332 V gehalten werden

| Last 3,3 V | 1,6 MHz (D = 0,84) | 2,0 MHz (D = 0,80) | 2,4 MHz (D = 0,76) |
|---|---|---|---|
| 0,37 A (realistisch, Spitze) | 4,49 V | 4,71 V | **4,95 V** |
| 0,60 A (Entwurf) | 4,66 V | 4,89 V | **5,14 V** |

### 5.2 USB-Betrieb, LED dunkel, Quelle 4,75 V

Gesamtstrom an `VCC_5V`: 0,34 A (realistisch, mit LED-Ruhestrom 0,033 A) bzw. 0,54 A (Entwurf). Spannungsfall bis `U10`: `F1` bis 0,13 Ω ● + TPS2121 0,085 Ω ✔ = 0,215 Ω; **dazu Kabel und Stecker (Quellwiderstand unbekannt, ○: gerechnet mit 0 Ω und mit 0,3 Ω als Beispiel, nicht als Aussage)**.
Quelle 4,75 V: untere Grenze des Rp-Bereichs (✔ Blatt `usb-c-16p.md` §3.3); die Mindestspannung der Quelle nach Typ-C-Spezifikation ist nicht gelesen (○).

| | `VIN` an `U10` | Last am 3,3-V-Pin bei 1,6 MHz | bei 2,0 MHz | bei 2,4 MHz |
|---|---|---|---|---|
| realistisch (0,37 A), Kabel 0 Ω | 4,68 V | 3,33 V (geregelt) | 3,31 V | **3,12 V** |
| realistisch, Kabel 0,3 Ω | 4,58 V | 3,33 V (geregelt) | 3,22 V | **3,04 V** |
| Entwurf (0,6 A), Kabel 0 Ω | 4,63 V | 3,31 V | 3,13 V | **2,95 V** |
| Entwurf, Kabel 0,3 Ω | 4,47 V | 3,17 V | **3,00 V** | **2,83 V** |

**Lesung:** Bei der realistischen Last bleibt die Schiene in allen gerechneten Ecken über der Modulgrenze von 3,0 V, **die Reserve im ungünstigsten Eck (2,4 MHz) beträgt 0,04 … 0,12 V**. Bei der Entwurfslast von 0,6 A unterschreitet das 2,4-MHz-Eck 3,0 V, und das 2,0-MHz-Eck liegt mit schlechtem Kabel genau an der Grenze.
Kommt **LED-Strom über USB** dazu, fällt `VIN` weiter: bei 1 A LED rund 4,75 − 1,35 · (0,215 + 0,3) = 4,05 V (○, mit Kabel 0,3 Ω), also unter dem Bedarf von 4,5 … 5,1 V in 5.1 — **die 3,3-V-Schiene bricht vor `F1` ein.** Der LED-Strom über USB ist damit durch die Spannungsreserve begrenzt, nicht durch die Sicherung.

### 5.3 Externe Einspeisung bei voller LED-Last

`VCC_5V` = Netzteil − I · (`F2` 0,060 Ω ● + `Q3` 0,0094 Ω ✔ + TPS2121 0,085 Ω ✔ = 0,154 Ω, jeweils Höchstwerte). Kabel und `H1`/`H2` nicht gerechnet.

| Netzteil am Stecker | I = 3,3 A (S3 realistisch) | I = 3,5 A (Entwurf) | Last bei 0,37 A: 2,0 MHz / 2,4 MHz (bei `VCC_5V` = 4,49 V bzw. 4,74 V) |
|---|---|---|---|
| 5,00 V | `VCC_5V` = 4,49 V | 4,46 V | 3,16 V / **2,98 V** |
| 5,25 V | 4,74 V | 4,71 V | 3,33 V (geregelt) / 3,17 V |

**Lesung:** Bei einem Netzteil von genau 5,00 V und Höchstwerten der Widerstände liegt `VCC_5V` unter Volllast **an der Untergrenze von `U3` (4,5 V ✔)**, und die 3,3-V-Schiene fällt im 2,4-MHz-Eck unter 3,0 V. Typische Widerstände sind kleiner (○, die Typwerte von `F2` fehlen); ein Netzteil mit **5,1 … 5,25 V und kurzem, dickem Kabel** (Setzung ○, Obergrenze `VCC_5V` 5,5 V wegen `U3` ✔) schafft Reserve.

### 5.4 Folgerung

Die Reserve ist **knapp und nicht belegt**: Das Verhalten des SY8301 am Tastgradanschlag steht nicht im Datenblatt (✔ Blatt §4.5, §8 Nr. 1). Entscheid: **1:1 bauen und messen** (AK37, P18); Fallback-Kette in `entscheidungen.md` AE-08.
Messpunkte sind vorhanden: `TP8` (`VCC_3.3V`), `TP7` (`VCC_5V`), `TP4` (`USB_5V_PROT`), `TP1` (`USB_5V_INPUT`), `TP5`/`TP6` (extern). Gemessen wird das **Minimum** über 10 Minuten mit Oszilloskop (Spec AK37), bei 4,5 / 4,75 / 5,0 V am Eingang, bei WLAN-Dauersenden und dem hellsten Normalbild; am DevKit sagt die Messung nichts über den Nachbau.

## 6. Sicherungen und Schutzglieder

| Teil | Wert | Grundlage | Rolle | Urteil |
|---|---|---|---|---|
| `F1` SMD1206-150-16 (C46641031) | Halt 1,5 A, Auslösung 3 A, 25 … 130 mΩ, 16 V, Auslösezeit 300 ms, Imax 40 A, Pd 1 W | ● Beschreibungsfeld in `v2.xml`; **Datenblatt nicht gelesen** | begrenzt den USB-Zweig | bleibt (AE-10); schützt Quelle und Kabel; bewusst kleiner als die Entwurfslast |
| `F2` 1812L300/24GR (C20627123) | Halt 3 A, Auslösung 5 A, 10 … 60 mΩ, 24 V, 4 s, Imax 40 A, Pd 1,4 W | ● Beschreibungsfeld; **Datenblatt nicht gelesen** | begrenzt die externe Einspeisung | **kleiner als 3,5 A Entwurfslast**: Frage Q4, Antrag A7 |
| `D4`, `D5` SMAJ5.0CA (C10756) | bidirektional, 5 V | ● Name; **Klemmspannung nicht gelesen** | Spitzen am USB- und am Externeingang | bleibt; Klemmspannung gegen `PR1` 6 V offen (R5, A4) |
| `Q3` SI7149DP | 30 V, 9,4 mΩ max bei −4,5 V | ✔ Blatt `si7149dp.md` | Verpolschutz extern | bleibt |
| `JP2`, `JP3` | offene Lötbrücken | ✔ Netzliste | `JP2` umgeht `Q1` (LED dauernd an), `JP3` umgeht den TPS2121 auf der USB-Seite | bleiben, nicht in der Stückliste (V2 gleich) |
| `U9` Strombegrenzung | 4,0 / 4,53 / 5,0 A | ✔ `tps2121.md` §4 | elektronische Begrenzung | bleibt (AE-09) |

**Kette von aussen nach innen bei einem Kurzschluss an `H7`:** `F2` (Auslösung 5 A) und die TPS2121-Begrenzung (4,0 … 5,0 A, schnelle Abschaltung bei rund 10,8 A in unter 1 µs ✔ `tps2121.md` §4) liegen in derselben Grössenordnung; welche zuerst anspricht, hängt von Toleranzen ab (○). Die LED-Seite hat keine eigene Sicherung (V2 gleich).

## 7. Offene Punkte

| Punkt | Wer | Was daran hängt |
|---|---|---|
| Haltestrom über Temperatur, Auslösezeit von `F1`/`F2` (Datenblätter) | Analyst | `F2`-Entscheid (A7) |
| Klemmspannung `D4`/`D5`; ob sie 6 V an `PR1` überschreitet | Analyst | A4 |
| Wirkungsgrad des SY8301 bei 5 V → 3,3 V | Messung am Prototyp | Strom an der 5-V-Schiene (Abschnitt 3) |
| Verhalten des SY8301 am Tastgradanschlag (Frequenz) | Messung (AK37) | Spannungsreserve (Abschnitt 5) |
| Stiftstrom `H7`, Flachbandkabel | Analyst (Stift), Nutzer (Kabel) | LED-Strom 3 A |
| Typwerte `F2` (Widerstand im Betrieb) | Analyst | Abschnitt 5.3 |
| LED-Strom je Kanal (20 mA ist Annahme) | Nutzer: Datenblatt SKC6812 (OP-27) | alle LED-Zeilen in Abschnitt 3 |
| Kabelwiderstand USB und extern | Messung | Abschnitt 5.2, 5.3 |
