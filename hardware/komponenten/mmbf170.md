# MMBF170 — N-Kanal-MOSFET 60 V, SOT-23 (Q2, schaltet das Gate von Q1)

Stand der Prüfung: **2026-10-17**. Zeichen: ✔ gelesen (Seite), ● Sekundärquelle, ○ Annahme/Rechnung.

## 1. Quelle — und eine Falle, die zuerst steht

Das bestellte Teil ist **nicht** das bekannte MMBF170 von Diodes oder onsemi, sondern der **Nachbau C130879 von „Shikues“**. Beide Datenblätter habe ich gelesen, **sie widersprechen sich in der Schwellenspannung**, und das ist für diese Schaltung der entscheidende Wert:

| Unterlage | Revision, Datum | Datei | VGS(th) | RDS(on) |
|---|---|---|---|---|
| **Shikues MMBF170** („N-Channel Enhancement Mode MOSFET“, 6 Seiten) — das **bestellte Teil** (C130879) | **REV.08**, ohne Datum | `datenblaetter/mmbf170_shikues.pdf` (von `datasheet.lcsc.com`, Verweis aus der LCSC-Seite von C130879) | **1,0 / 1,6 / 2,5 V** (✔ S. 2) | 3,0 Ω max bei 10 V/0,4 A; **4,0 Ω max bei 4,5 V/0,3 A** (✔ S. 2) |
| Diodes Incorporated, **MMBF170**, Dokument DS30104 **Rev. 16-2**, **Juli 2024**, 5 Seiten (Hersteller des Original-Typs) | Rev. 16-2, Juli 2024 | `datenblaetter/mmbf170_diodes.pdf`, `https://www.diodes.com/assets/Datasheets/MMBF170.pdf` | **0,8 / 2,1 / 3,0 V** (✔ S. 2) | 5,0 Ω max bei 10 V/0,2 A; 5,3 Ω max bei 4,5 V/50 mA (✔ S. 2) |

Das Shikues-Datenblatt trägt keinen Herstellerkopf mit Adresse und kein Datum; ich lese es als Datenblatt des Nachbaus, weil LCSC es für C130879 ausliefert. Sein Abgleich mit dem Original: Shikues-Teil ist **besser** bei kleiner Gate-Spannung (Schwelle ≤ 2,5 V gegen ≤ 3,0 V) und leitet mit geringerem RDS(on), liegt aber bei IGSS schlechter (±10 µA bei ±20 V gegen ±10 nA, ✔ Shikues S. 2 / Diodes S. 2).
**Für das 3,3-V-Gate gilt das Shikues-Datenblatt.** Wird jemals auf C151058 (MMBF170-7-F, Diodes) oder C28871 (onsemi MMBF170LT1G, „3 V“ Schwelle laut Quelle) umgestellt, ist die Garantie bei 3,3 V Gate nicht mehr gegeben.

Gehäuse: **SOT-23**, Pinbelegung **1 Gate, 2 Source, 3 Drain** ✔ (Shikues S. 1, Draufsicht; Diodes S. 1 zeigt dieselbe Anordnung). Der V2-Schaltplan: Q2 Pin 1 = Gate (`Net-(Q2-G)`), Pin 2 = Source (GND), Pin 3 = Drain (`Net-(Q2-D)`) ✔ (Netzliste selbst exportiert) — stimmt.

## 2. Absolute Maximalwerte und Kennwerte

**Shikues (✔ S. 1–2, TA = 25 °C):**

| Grösse | Wert |
|---|---|
| VDS | 60 V |
| VGS | ±20 V |
| ID | 0,5 A (VGS = 10 V); IDM 1,7 A |
| Ptot | 0,83 W (25 °C), 0,33 W (100 °C); RθJA 150 °C/W (1 in² Fläche) |
| Tj, Tstg | −55 … 150 °C |
| IS (Body-Diode) | 0,4 A; VSD 0,7 V typ, 1,3 V max (0,4 A) |
| BVDSS | ≥ 60 V |
| VGS(th) | 1,0 / **1,6** / **2,5 V** (ID = 250 µA) |
| IDSS | ≤ 1 µA (48 V), ≤ 30 µA (85 °C) |
| IGSS | ≤ ±10 µA (VGS = ±20 V) |
| RDS(on) | 1,9 Ω typ / 3,0 Ω max (10 V, 0,4 A); 2,4 / **4,0 Ω** (4,5 V, 0,3 A) |
| Rg | 130 Ω typ |
| Ciss / Coss / Crss | 30 / 4,2 / 3 pF |
| Qg | 305 pC (VGS = 4,5 V); Qgs 85 pC, Qgd 205 pC |
| td(on), tr, td(off), tf | 3,9 / 3,5 / 16 / 10 ns typ |
| ESD | „HBM > 2 kV“ (Merkmale S. 1) |

**Diodes (✔ S. 2):** VDSS 60 V, VGSS ±20 V, ID 200 mA, IDM 800 mA, PD 300 mW, RθJA 417 °C/W; VGS(th) 0,8 / 2,1 / 3,0 V; RDS(on) 3,2 Ω typ / 5,0 Ω max (10 V); 5,3 Ω max (4,5 V, 50 mA); gFS ≥ 80 mS; Ciss 22 pF.

Beide Teile führen **keinen RDS(on) für VGS = 3,3 V**. Die normierte Kurve RDS(on) gegen VGS beim Diodes-Typ (Bild 4, S. 3) liegt bei 3,3 V etwa beim 3- bis 4-Fachen des Wertes bei 10 V; für das Shikues-Teil zeigt Seite 4/5 nur Kurven bei VGS 4,5 und 10 V (ohne 3,3 V).

## 3. Beschaltung im Projekt und Rechnung

### 3.1 Schaltet Q2 mit 3,3 V am Gate (über 82 Ω) sicher durch?
- Gate-Spannung vom ESP32-S3-GPIO: **VOH ≥ 0,8 × VDD** (✔ ESP32-S3 Datenblatt v2.2, S. 65, DC-Kennwerte, Fussnote „VOH und VOL mit hochohmiger Last gemessen“). Bei VDD = 3,3 V: **≥ 2,64 V**; bei VDD = 3,0 V: ≥ 2,4 V. Im Gleichstrom nimmt das Gate keinen Strom auf (IGSS ≤ 10 µA × 82 Ω = 0,8 mV Spannungsfall) ⇒ in der Praxis liegt das Gate bei VDD (3,0 … 3,6 V) ○.
- Schwelle: Shikues **max 2,5 V** ⇒ Übersteuerung **≥ 0,14 V im ungünstigsten Papierfall (VOH-Minimum bei 3,3 V)**, im praktischen Fall **≈ 0,5 … 1,1 V (VDD 3,0 … 3,6 V)** ○. Das ist **ausreichend, weil Q2 nur 1,5 mA** schalten muss (siehe unten); die Schwelle ist bei 250 µA definiert (S. 2), mit 1,5 mA braucht es ein wenig mehr als VGS(th) — der Papierfall ist **knapp**, der praktische Fall sicher.
- Beim **Diodes-Typ** wäre die Schwelle 3,0 V max: Mit VOH-Minimum 2,64 V (oder 2,4 V) **kein positiver Überschuss** — dort wäre Q2 nicht garantiert leitend. Das Shikues-Teil rettet die Schaltung nur wegen seiner niedrigeren Schwelle.
- **Strom durch Q2 im EIN-Zustand:** nur R21 (3,3 kΩ) zieht den Drain-Knoten hoch (R23 führt in das Gate von Q1 und damit keinen Gleichstrom, siehe `si7149dp.md` 2): **ID ≈ VCC_5V / R21 = 5 V / 3,3 kΩ = 1,5 mA** (bei 5,25 V 1,6 mA) ○ — um den Faktor 300 unter dem Nennstrom (0,5 A), Verlust 1,5 mA² × 4 Ω = 9 µW.
  **Spannung am Drain (EIN):** RDS(on) ≤ 4 Ω bei 4,5 V; für 3,3 V rechne ich konservativ 10 Ω (○, aus dem Kurvenverlauf des Diodes-Typs hochgerechnet): **VDS ≈ 15 mV** ⇒ das Gate von Q1 liegt bei ≈ 15 mV, **VGS(Q1) ≈ −4,98 V** ○. Selbst bei 1 kΩ RDS(on) (Fall „kaum durchgesteuert“) wären es 1,5 V und VGS(Q1) = −3,5 V — Q1 liesse sich dann nicht mehr sicher im spezifizierten Bereich betreiben (`si7149dp.md`). Das ist der Grund, weshalb die niedrige Schwelle zählt.
- **Aus-Zustand und Reset:** R20 (100 kΩ) zieht das Gate nach GND; Q2 sperrt, R21 hebt den Drain-Knoten, R23 + R21 ziehen Q1 über das Gate ab ⇒ **LED-Versorgung aus**, solange der GPIO nicht aktiv „high“ ist. Hat der Pin im Reset einen internen Pull-up (45 kΩ, ✔ ESP32-S3 Datenblatt v2.2, S. 65), liegt das Gate über den Spannungsteiler 45 kΩ / R20 (100 kΩ) bei 3,3 × 100/(45 + 100) = **2,3 V** ○ — das liegt im Schwellenbereich des Shikues-Teils (1,0 … 2,5 V) und könnte Q2, und damit die LED-Versorgung, im Reset einschalten. Hat er einen Pull-down oder ist er hochohmig, bleibt das Gate unter 0,5 V. Welchen Zustand ein Pin im Reset hat, steht im Pin-Blatt des ESP32-S3, nicht hier. **Prüfauftrag an den Designer:** den GPIO für `WS2812_POWER` so wählen (oder R20 so verkleinern, z. B. 10 kΩ ⇒ 0,6 V), dass das Gate im Reset sicher unter 1,0 V bleibt.
- Schaltzeit: Qg 305 pC (✔ S. 2), mit 82 Ω und Rg 130 Ω Eigenwiderstand ≈ 20 ns — unkritisch. Der Gate-Strom beim Umschalten des GPIO: 3,3 V / 82 Ω = 40 mA kurz (liegt innerhalb der ESP32-Treiberstärke, ✔ IOH 40 mA bei PAD_DRIVER = 3, S. 65 des SoC-Datenblatts); R19 begrenzt Klingeln, nicht Gleichstrom.
- Q2 muss **Q1-Gate-Strom** liefern: Das Gate von Q1 samt C10 (100 nF) wird über R23 entladen: Spitzenstrom 5 V / 2,2 kΩ = 2,3 mA ⇒ Q2 trägt das problemlos.

### 3.2 Verlustleistung
Dauerverlust im Aus-Zustand null, im Ein-Zustand 9 µW. Kein thermisches Thema.

## 4. Fertigung bei JLCPCB (Prüfdatum **2026-10-17**)

| Teil | LCSC | Klasse | Lagerbestand JLCPCB | Gehäuse | Bemerkung |
|---|---|---|---|---|---|
| **MMBF170 (Shikues)** — in der Stückliste | **C130879** | **Extended** (nicht Preferred) | **1'355** (niedrigster Bestand der Transistoren; LCSC selbst zeigt 0) | SOT-23 | SMD, SMT-bestückbar |
| MMBF170-7-F (Diodes) | C151058 | Extended | 14'841 | SOT-23 | Vth max 3,0 V ✘ für 3,3 V Gate |
| MMBF170LT1G (onsemi) | C28871 | Extended | 39'332 | SOT-23 | „3 V“ Schwelle laut Quelle ✘ |
| MMBF170Q-13-F, -VB, LM-… | andere | Extended | 0 … 415 | | Lager nicht ausreichend |
| **AO3400A (Alpha & Omega)** | **C20917** | **Basic** | **718'151** | SOT-23 | siehe unten |
| 2N7002 (Jiangsu Changjing) | C8545 | **Basic** | 2'604'549 | SOT-23 | siehe unten |

Quelle ●: Teilebibliothek JLCPCB (Suchschnittstelle von `jlcpcb.com/parts`); LCSC-Datenblattverweis über `wmsc.lcsc.com`.

**Basic-Alternativen, Datenblätter gelesen (Stand 2026-10-17):**
- **AO3400A (C20917, Basic):** AOS „30V N-Channel MOSFET“, **Rev 3, Dez. 2011** (5 Seiten, `datenblaetter/ao3400a_aos.pdf`). **VGS(th) 0,65 / 1,05 / 1,45 V** ✔ S. 2; **RDS(on) < 48 mΩ bei VGS = 2,5 V**, < 32 mΩ bei 4,5 V ✔ S. 1; VDS 30 V, **VGS ±12 V** ✔ S. 1; ID 5,7 A; Qg 7 nC (Beschreibung ● JLCPCB, im Datenblatt S. 2). Bei 3,3 V Gate: Überschuss ≥ 1,2 V im Papierfall (2,64 − 1,45 V) und RDS(on) um 50 mΩ — **deutlich besser als MMBF170** für dieses Gate und für 5 V am Drain (30 V Reserve reicht). Gate: R19 (82 Ω) bleibt; VGS-Grenze 12 V ist mit 3,3 V eingehalten. **Pinbelegung SOT-23: im Datenblatt nicht eindeutig im Text gelesen** (die Zeichnung auf S. 1 trägt die Anschlüsse, ich habe sie nicht ausgemessen) — vor Umstellung gegen den Footprint prüfen: AOS belegt SOT-23 üblicherweise mit 1 = Gate, 2 = Source, 3 = Drain, das ist ○.
- **2N7002 (C8545, Basic):** Jiangsu Changjiang Electronics, „SOT-23 MOSFET (N-Channel)“, Ausgabe J, Sep. 2016 (`datenblaetter/2n7002_changjing.pdf`). VGS(th) 1 / 1,6 V (min/typ; **kein Maximum** in der Tabelle, ✔ S. 2); RDS(on) 5 Ω max bei 10 V, 7 Ω max bei **5 V/50 mA** ✔ S. 1–2; ID 115 mA; Pinbelegung 1 Gate, 2 Source, 3 Drain ✔ S. 1. Bei 3,3 V Gate ist nichts spezifiziert — **als zweite Wahl nicht belegt**.

**Empfehlung:** Q2 auf **AO3400A (C20917)** umstellen — Basic, Lager 718'151, Schwelle bis 1,45 V, belegt. Begründung für „Extended“ bei C130879 entfällt, wenn die Alternative nach Prüfung der Pins akzeptiert wird. Ein Grund für die Beibehaltung von C130879 ist nicht erkennbar (Stückliste V2 stammt unverändert von der alten Platine).

## 5. Fallen und Errata

- **Zwei Datenblätter, zwei Schwellen** (Abschnitt 1): das Shikues-Blatt ist ohne Datum und ohne Herstelleranschrift — eine Quelle mit schwacher Rückverfolgbarkeit. Der Lagerbestand bei LCSC ist 0, bei JLCPCB 1'355: **Verfügbarkeitsrisiko.**
- Der Auftrag hielt die Diodes-Zahlen für massgeblich; sie gelten nur für den Diodes-Typ.
- IGSS des Shikues-Teils ist ±10 µA (Diodes: ±10 nA) — für Q2 unkritisch, weil das Gate im Gleichstrom nur durch R20 und R19 belastet wird (3,3 V / 100 kΩ = 33 µA).

## 6. Folgerung für unsere Platine

1. **Q2 auf AO3400A (C20917, Basic) umstellen**, nach Pinprüfung des Footprints; das behebt die dünne Schwellenreserve und die Nicht-Basic-Klasse in einem Schritt.
2. Bleibt es beim MMBF170: nur das Shikues-Teil C130879 verwenden, **nie** den Diodes- oder onsemi-Typ mit 3,0 V Schwelle. Die Stückliste muss die Hersteller-Nummer tragen.
3. Reset-Zustand des Gate-Treibers (GPIO mit 45-kΩ-Pull) prüfen, damit die LED-Versorgung beim Booten nicht kurz einschaltet.

## 7. Offene Punkte

1. Pinbelegung AO3400A gegen den SOT-23-Footprint (Zeichnung S. 1 des Datenblatts).
2. Reset-/Boot-Zustand des gewählten GPIO für `WS2812_POWER` — Aufgabe des GPIO-Blatts.
3. Die LCSC-Lagerzahl 0 für C130879 gegen aktuellen JLCPCB-Bestand beim Bestellen erneut prüfen.
