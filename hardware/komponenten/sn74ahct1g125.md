# Komponentenblatt: SN74AHCT1G125 (TI) — Pegelwandler 3,3 V → 5 V für die LED-Datenleitung

Stand der Prüfung: **17.10.2026**. Verfasser: Datenblatt-Analyst. Zeichen: ✔ im Datenblatt gelesen (Seite),
● Sekundärquelle (genannt), ○ Annahme/Erinnerung oder eigene Rechnung.
Die Quellen liegen unter `hardware/komponenten/datenblaetter/` (nicht im Git).

## 1. Quelle, Variante, Gehäuse

| Unterlage | Revision | Datum | Datei |
|---|---|---|---|
| TI **SN74AHCT1G125 Single Bus Buffer Gate With 3-State Output**, 26 S. | **SCLS378P** | „August 1997 – revised **March 2024**“ (S. 1); Gehäuseseiten tragen „28-Sep-2026“ (S. 13) | `sn74ahct1g125.pdf` (ti.com, direkt abgerufen). Gedruckte Seite = PDF-Seite |

- **Variante in V2:** **SN74AHCT1G125DBVR**, **SOT-23-5** (DBV), 2,9 × 2,8 mm mit Anschlüssen, Körper 2,9 × 1,6 mm ✔ S. 1; Netzliste: U3, Footprint `SOT-23-5_L3.0-W1.7-P0.95…`, LCSC C7484 (Auszug aus der V2-Netzliste/Schaltplan).
- Weitere Gehäuse derselben Typenreihe: DCK (SC-70-5), DRL (SOT-553) ✔ S. 1.

## 2. Absolute Maximalwerte und empfohlene Betriebsbedingungen

| Grösse | Wert | Beleg |
|---|---|---|
| Versorgung VCC (absolut) | −0,5 … **7 V** | ✔ S. 4 (5.1) |
| **Eingangsspannung VI (absolut)** | **−0,5 … 7 V**, **nicht an VCC gebunden** (Fussnote: darf überschritten werden, wenn die Strombegrenzungen eingehalten werden) | ✔ S. 4 (5.1, Fussnote 2) |
| Ausgangsspannung VO | −0,5 … VCC + 0,5 V | ✔ S. 4 |
| Eingangs-Klemmstrom IIK (VI < 0) | −20 mA — **es gibt keine Zeile für einen Klemmstrom bei VI > VCC** | ✔ S. 4 |
| Ausgangs-Klemmstrom IOK | ±20 mA | ✔ S. 4 |
| Dauerstrom Ausgang / durch VCC oder GND | ±25 mA / ±50 mA | ✔ S. 4 |
| ESD HBM / CDM | ±1000 V / ±1500 V | ✔ S. 4 (5.2) |
| **Empfohlen: VCC** | **4,5 … 5,5 V** | ✔ S. 4 (5.3) |
| **VIH** (High-Eingang) | **min. 2 V** (im gesamten VCC-Bereich 4,5…5,5 V) | ✔ S. 4 |
| **VIL** (Low-Eingang) | max. 0,8 V | ✔ S. 4 |
| Eingangsspannung VI (empfohlen) | 0 … 5,5 V | ✔ S. 4 |
| IOH / IOL | −8 mA / +8 mA | ✔ S. 4 |
| Anstiegszeit am Eingang Δt/Δv | max. 20 ns/V | ✔ S. 4 |
| Betriebstemperatur | −40 … +125 °C | ✔ S. 4 |

## 3. Pinfunktionen

SOT-23-5 (DBV) ✔ S. 3 (Tab. 4-1):

| Pin | Name | Funktion | V2-Beschaltung (Netzliste) |
|---|---|---|---|
| **1** | **OE** | Ausgang freigeben, **aktiv Low** | **GND** (Ausgang dauernd freigegeben) |
| 2 | A | Eingang | `WS2812_BUS` (STM-Pin, 3,3 V) |
| 3 | GND | Masse | GND |
| 4 | Y | Ausgang | `Net-(U3-Y)` → R11 330 Ω → LED-Kette |
| 5 | VCC | Versorgung | `Net-(U3-VCC)` — Zweig R13 (0 Ω) und R17 an der **geschalteten** 5-V-Leitung (Netzliste) |

**Wahrheitstabelle** ✔ S. 8 (Tab. 7-1): OE = L, A = H → Y = H; OE = L, A = L → Y = L; **OE = H → Y hochohmig (Z)**. Die Beschreibung auf S. 1 bestätigt: „The output is disabled when OE is high. When OE is low, data is passed from A to Y.“ **OE an GND ist damit die richtige Polarität für „immer an“** ✔.
Es gibt keine Strapping- oder Boot-Pins; das Bauteil ist rein kombinatorisch.

## 4. Empfohlene Beschaltung

- **Entkopplung:** 0,1 µF an VCC, so nah wie möglich am Pin; 0,1 µF parallel zu 1 µF ist üblich ✔ S. 10 (8.3).
- **OE:** Für ein **hochohmiges Verhalten beim Ein- und Ausschalten** der Versorgung empfiehlt TI, OE über einen Widerstand an VCC zu legen („To ensure the high-impedance state during power up or power down, OE should be tied to VCC through a pullup resistor“) ✔ S. 8 (7.1). V2 legt OE fest an GND, das ist **ausdrücklich nicht die empfohlene Beschaltung für diesen Fall**: Der Ausgang ist freigegeben, sobald VCC da ist. Ob das stört, hängt von der LED-Leitung ab (Pull-down R31, siehe 6).
- **Nicht benutzte Eingänge** nicht offen lassen ✔ S. 10 (8.4.1). Hier ist A belegt.
- **Typische Anwendung:** „Lowered switching threshold allows up translation 3.3 V to 5 V“ ✔ S. 8 (7.3); ausdrücklich „ideal for translating up from 3.3 V to 5 V“ ✔ S. 9 (8.1).
- **Ausgang:** Last ≤ 25 mA je Ausgang, ≤ 50 mA Summe; Ausgang nicht über VCC ziehen ✔ S. 9 (8.2.2). Schnelle Kanten bei kleiner Last → Leitungsführung beachten (Klingeln) ✔ S. 9 (8.2.1).

## 5. Kritische Daten für dieses Projekt

### 5.1 Reicht ein 3,3-V-Signal sicher?

- **Ja, mit rund 1 V Reserve.** VIH min. 2 V bei VCC 4,5…5,5 V ✔ S. 4. Ein 3,3-V-Ausgang der Steuerung liefert nach dessen Datenblatt mindestens **0,8 × VDD** (ESP32-S3: **2,64 V** bei VDD = 3,3 V, ✔ ESP32-S3 Series Datasheet v2.2, Tab. 5-4, PDF-S. 65: „VOH 0,8 × VDD“; „VOH ≥ 2,64 V“ für den Strom 40 mA). 2,64 V liegt 0,64 V über der Schwelle von 2 V. Bei 3,3 V −10 % (2,97 V) bleibt es ebenso über 2 V (gerechnet ○ aus ✔-Werten).
- **VIL:** 0,8 V max.; ein Low-Pegel der Steuerung liegt bei höchstens 0,1 × VDD (ESP32-S3: VOL ≤ 0,1 × VDD ✔ ESP-DS Tab. 5-4) = 0,33 V ✔ gerechnet.
- **Mehrstrom im High-Zustand:** Ein Eingang auf TTL-Pegel (Messpunkt 3,4 V, VCC 5,5 V) erhöht den Versorgungsstrom um **höchstens 1,35 mA bei 25 °C, 1,5 mA über den Temperaturbereich** (ΔICC) ✔ S. 5 (5.5). Bei 3,3 V ist der Wert ähnlich (○, Messpunkt des Datenblatts ist 3,4 V). Das trifft nur, solange die Datenleitung High ist und VCC anliegt.
- **Ausgangspegel:** VOH bei IOH = −8 mA, VCC = 4,5 V: **min. 3,94 V bei 25 °C, min. 3,8 V über −40…+125 °C**; bei −50 µA min. 4,4 V ✔ S. 5. VOL bei 8 mA: max. 0,36 V bei 25 °C, max. 0,44 V über den Temperaturbereich; bei 50 µA max. 0,1 V ✔ S. 5. **Ob 3,8 V für den LED-Eingang ausreicht, hängt vom Datenblatt der LED ab (SK6812): nicht gelesen, offen.**
- Laufzeit: 6,5 ns max. (−40…85 °C, 15 pF), 8,5 ns bei 50 pF ✔ S. 6 — für die 800-kHz-Datenleitung unkritisch (○ Folgerung).

### 5.2 Was passiert am Eingang, wenn VCC = 0 V und das 3,3-V-Signal anliegt?

Das ist der Fall, in dem V2 den LED-Strang abschaltet (VCC am **geschalteten** Netz) und der Prozessor weiter ein Signal ausgibt.

- **Absolute Grenze:** VI = −0,5…7 V, **ohne Bindung an VCC**; 3,3 V sind damit zulässig ✔ S. 4 (5.1). Die Fussnote 2 sagt, die Spannungsgrenzen dürften überschritten werden, wenn die Stromgrenzen eingehalten werden.
- **Messwert bei VCC = 0 V:** Der Eingangs-Leckstrom ist **mit VCC = 0 V … 5,5 V** und VI = 5,5 V oder GND angegeben: **höchstens ±0,1 µA bei 25 °C und ±1 µA über −40 … +125 °C** ✔ S. 5 (5.5, Zeile II). Das heisst: **Bei VCC = 0 V und 3,3 V (bis 5,5 V) am Eingang fliessen höchstens 1 µA** — es gibt keine nennenswerte Rückspeisung über den Eingang in das tote VCC-Netz.
- **TI sagt zusätzlich:** „Inputs are overvoltage tolerant allowing them to go as high as 5.5 V at **any valid VCC**“ ✔ S. 9 (8.2.2). „Valid“ bedeutet hier ○ den Bereich 4,5…5,5 V; die Zeile II der Tabelle deckt VCC = 0 V dagegen ausdrücklich ab. Ein Parameter „Ioff“ (Teilabschaltschutz) ist **nicht** in der Tabelle: Das Bauteil wirbt nicht mit „partial power down“. Die Messung bei VCC = 0 V in der Zeile II gilt damit als einzige Zusage; der Wert ist klein genug, dass ein 3,3-V-Pegel am Eingang bei VCC = 0 V unkritisch ist.
- **Ausgang bei VCC = 0 V:** **nicht spezifiziert** (die Zeile IOZ gilt für VCC = 5,5 V). In V2 hängt der Ausgang über R11 an `TO_SK6812_IN`, den **R31 (100 kΩ)** auf definiertem Pegel hält (`HARDWARE.md`). Weiteres: ○ nicht gelesen.
- **Fazit für die geschaltete Versorgung:** Der Eingang ist belegt, der Ausgang nicht.

## 6. Fertigung bei JLCPCB

Prüfdatum **2026-10-17**. Klasse/Lager aus der Teilebibliothek von JLCPCB über die Such-Schnittstelle von `jlcpcb.com/parts` (●, Schlüsselwort = LCSC-Nummer), LCSC-Lager aus `lcsc.com/product-detail/<Nr>.html` (●).

| LCSC | Bauteil | Hersteller | Klasse | Lager JLCPCB | Lager LCSC | Hinweis |
|---|---|---|---|---|---|---|
| **C7484** | **SN74AHCT1G125DBVR**, SOT-23-5 | **Texas Instruments** | **Extended** (kein Preferred-Flag) | **51'402** | 28'180 | V2-Bestand, passt; Bestand hoch |
| C350557 | SN74AHCT1G125DCKR, SC-70-5 | Texas Instruments | Extended | 16'165 | nicht abgefragt | anderes Gehäuse (kleiner), nur mit neuem Footprint |
| C20617903 | SN74AHCT1G125DBVR(UMW) | UMW (Youtai) | Extended | 28'924 | nicht abgefragt | Nachbau; Beschreibung nennt „VIH 2 V, VOH 3,8 V…“ (●); Gleichwertigkeit nicht gelesen ○ |
| C3040595 | 74LVC1G125GV-TP, SOT-23-5 | TECH PUBLIC | Extended | 22'950 | nicht abgefragt | **andere Familie** (74LVC, VCC 1,65…5,5 V laut Beschreibung); als Hochsetzer auf 5 V nicht geprüft ○ |

- **Grund für Extended:** Es gibt keinen Basic- oder Preferred-Pegelwandler dieser Art. Vollständig durchsucht wurden „74AHC1G125“ (47 von 47), „SN74LVC1G125“ (36 von 36), „LV1T125“ (22 von 22), „74HC125“ (99 von 99), „74LVC1G125“ (109 von 109), „SN74AHCT1G125“ (12 von 12): **kein** Basic, kein Preferred. „AHCT“ (200 von 1'130) und „74HCT“ (200 von 2'490) nur teilweise (erste 200), ebenfalls ohne Treffer.
- **SMT-bestückbar:** SOT-23-5 ist ein SMD-Gehäuse ✔ (Quelle); Bestückungsweg nicht geprüft.
- **C7484 ist geprüft** (Treffer mit `componentCode`).

## 7. Fallen und Errata

- **OE an GND ist nicht die von TI empfohlene Beschaltung** für das Ein-/Ausschalten (S. 8). Für V2 unkritisch, solange der Pull-down an der LED-Leitung den Pegel hält; dokumentiert, nicht behoben.
- **VCC am geschalteten Netz:** Ob das Einschalten glitchfrei ist (Ausgang freigegeben, sobald VCC steigt, VIH-Schwelle bei VCC < 4,5 V nicht spezifiziert): **nicht belegt**. Spezifiziert sind nur VCC = 4,5…5,5 V ✔ S. 4.
- **Abgleich zweier Stellen:** Merkmalsliste/Beschreibung (S. 1, S. 8, S. 9) und Tabellen (S. 4, S. 5) widersprechen sich nicht (VIH 2 V, VCC 4,5…5,5 V, OE aktiv Low).
- **ΔICC von 1,35 mA** ist kein Fehler, sondern TTL-Eingangs-Eigenschaft ✔ S. 5; sie belastet die geschaltete 5-V-Leitung, solange die Datenleitung High ist.
- **Nachbauten** (UMW) haben abweichende Kennwerte in der Beschreibung (z. B. „12 ns bei 5 V/50 pF“ gegen 8,5 ns max. bei TI ✔ S. 6). Nicht gleichwertig, bis das Datenblatt des Nachbaus gelesen ist.

## 8. Folgerung für unsere Platine

1. **Unverändert übernehmen:** U3 = SN74AHCT1G125DBVR (C7484), OE an GND, A am 3,3-V-Datenpin, Y über R11 an die Kette. **Die Zahlen decken den Fall:** VIH 2 V gegen 2,64 V (ESP32-S3 VOH min.).
2. **Eingang bei VCC = 0 V:** belegt unkritisch (≤ 1 µA). Der Ausgang ist dort unspezifiziert; R31 (100 kΩ) bleibt.
3. **Entscheiden (Architekt/Designer):** Soll OE zusätzlich über einen Widerstand an VCC (TI-Empfehlung) statt an GND? Dann wäre der Ausgang beim Ein-/Ausschalten hochohmig und R31 hielte den Pegel; nicht nötig für 1:1.
4. **Prüfen:** SK6812-Datenblatt zu VIH bei 5 V und zu VOH = 3,8 V (nicht gelesen).
5. **Entkopplung 100 nF** am VCC-Pin vorsehen (V2-Stückliste prüfen, ob C vorhanden: nicht geprüft).

## 9. Offene Punkte

- SK6812-VIH (nicht gelesen) → Gutes Signal bei 3,8 V?
- Verhalten des Ausgangs bei VCC = 0 V und Ausgang von aussen belegt: nicht spezifiziert.
- Ob C7484 in der gelieferten Charge das Datenblatt-Verhalten zeigt (Nachbauten sind im Katalog häufig): nicht prüfbar.
