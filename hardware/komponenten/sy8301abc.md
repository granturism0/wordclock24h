# SY8301ABC — Abwärtswandler 5 V → 3,3 V (mit D2 PMEG2010ER und L2 FTC252010S4R7MBCA)

Stand der Prüfung: **2026-10-17**. Belegzeichen: ✔ im Datenblatt gelesen (Seite), ● Sekundärquelle, ○ Annahme/Rechnung
aus belegten Werten (als solche benannt). Dieses Blatt deckt drei Teile ab, die zusammen die 3,3-V-Schiene bilden.

## 1. Quelle

| Teil | Unterlage | Revision, Datum | Adresse, Datei |
|---|---|---|---|
| U10 SY8301ABC | Silergy, „Application Note: SY8301 — High Efficiency, 1A 40V Input Synchronous Step Down Regulator“ (das Dokument trägt im Kopf „AN_SY8301“, ist aber das Herstellerdatenblatt; es gibt kein getrenntes) | **Rev. 0.9A, 08.06.2020** (S. 13 Änderungsverlauf; Erstausgabe Rev. 0.9 vom 16.11.2018) | `datenblaetter/sy8301.pdf`, Spiegel `static.chipdip.ru/lib/164/DOC045164014.pdf`. Der Abruf direkt bei silergy.com lieferte nur eine HTML-Seite. Die Datei, auf die LCSC für C2927130 verweist, hat dieselbe Byte-Grösse (960123) |
| D2 PMEG2010ER | Nexperia, „PMEG2010ER — 20 V, 1 A low VF Schottky barrier rectifier“, Product data sheet | **1. Juli 2023** (13 Seiten) | `datenblaetter/pmeg2010er.pdf` (Kopie von `datasheet.lcsc.com`; nexperia.com selbst antwortete mit 403) |
| L2 FTC252010S4R7MBCA | Cjiang (SZ Cjiang Technology), „Specification for Approval — Wire Wound Molded SMD Power Inductors, FTC Series“, 61 Seiten **als Bilder ohne Text** | **Rev. 7.0, 05.11.2025** (Änderungsverlauf, PDF S. 2); Zeile des Teils auf PDF S. 18 | `datenblaetter/ftc252010s4r7mbca.pdf` (von `datasheet.lcsc.com`) |

Gehäuse: SY8301 SOT-23-6 (S. 1); PMEG2010ER SOD123W / CFP3, 2,6 × 1,7 × 1 mm (S. 2); L2 2,5 × 2,0 × 1,0 mm (Bezeichnung „252010“, Imperial 1008).

## 2. Bestand in der V2-Netzliste (nachgeprüft)

Ich habe die Netzliste aus dem V2-Schaltplan selbst exportiert (`kicad-cli`, nur lesend). Sie stimmt mit dem Auftrag überein, **bis auf einen Wert**:

- U10: IN(5) und EN(4) an `VCC_5V`; LX(6) an L2 Pin 1; L2 Pin 2 an Netz `Net-(D2-A)`; BS(1) über C25 (100 nF) an LX; FB(3) zwischen R28 (100 kΩ, nach `VCC_3.3V`) und R29 (32 kΩ, nach GND). D2: Anode am Netz hinter L2, Kathode an `VCC_3.3V`.
- **Abweichung: C29 ist 100 nF, nicht 22 µF.** Am Netz hinter L2 hängen C27 und C28 (je 22 µF, 25 V X5R 0805) und C29 (100 nF). Der Auftrag nannte alle drei mit 22 µF. Hinter der Diode folgt C8 (22 µF) an `VCC_3.3V`.
- Auf `VCC_5V` liegen C23 (47 µF), C26 (10 µF) und der Ausgang von U9 (TPS2121); der Eingangskondensator des Wandlers ist damit die Summe daraus, einen eigenen nahen 4,7-µF-Kondensator gibt es im Schaltplan nicht (Lage im Layout nicht geprüft).
- R29 trägt im Schaltplan den Wert „32 kΩ“; das bestellte Teil C2930156 ist laut `jlcpcb-bestand.md` ein **31,6 kΩ**-Widerstand (E96). Beide Werte sind unten gerechnet.

## 3. SY8301 — Grenzwerte und Betriebsbedingungen

| Grösse | Wert | Beleg |
|---|---|---|
| Ausgangsstrom | **1 A** („1A Output Current Capability“) | ✔ S. 1 |
| Eingangsspannung, empfohlen | **4,5 … 40 V** | ✔ S. 3 und S. 4 |
| Eingangsspannung, im Fliesstext | „4V to 40V“ | ✔ S. 8 — **widerspricht** S. 1/3/4; ich rechne mit 4,5 V |
| UVLO steigend / Hysterese | 3,6 / **4,0** / 4,4 V; Hysterese 0,6 V typ | ✔ S. 4 |
| Absolutes Maximum | IN, LX, FB, EN −0,3 … 40 V; BS−LX −0,3 … 4 V; Tj −40 … 150 °C | ✔ S. 3 |
| Umgebungstemperatur | −40 … 85 °C | ✔ S. 3 |
| Referenzspannung an FB | **0,792 / 0,8 / 0,808 V** (±1,0 %) | ✔ S. 4 und S. 1 |
| FB-Strom | ±50 nA max (VFB = 3,3 V) | ✔ S. 4 |
| Schaltfrequenz | **1,6 / 2,0 / 2,4 MHz** | ✔ S. 4 |
| Strombegrenzung (High-Side, Spitze) | **1,6 … 2,5 A** | ✔ S. 4 |
| RDS(on) oben / unten | 380 / 180 mΩ (typ) | ✔ S. 4 |
| Mindest-Ein- / Ausschaltzeit | 80 ns / **100 ns** (typ) | ✔ S. 4 |
| Softstart | 1 ms typ | ✔ S. 4 |
| Ruhestrom | 22 µA typ (28 µA max); Abschaltstrom 1 µA typ | ✔ S. 4 |
| Unterspannungsschutz | Ausgang < 50 % VREF für > 60 µs ⇒ Schutz | ✔ S. 4 |
| Kurzschluss | „Hiccup“: ca. 5 ms Pause, dann voller Softstart (ca. 1 ms), endlos wiederholt | ✔ S. 9 |
| Übertemperatur | Abschaltung 150 °C, Wiederanlauf nach ca. 15 °C Abkühlung | ✔ S. 4, S. 9 |
| Verlustleistung / Wärmewiderstand | PD 0,4 W bei 25 °C; θJA 100 °C/W, θJC 30 °C/W (Zweilagen-Evalboard) | ✔ S. 3 |
| EN | steigend ≥ 1,4 V, fallend ≤ 1 V; „nicht offen lassen“ | ✔ S. 4, S. 2 |

### Pins (SOT-23-6, S. 2)
1 BS, 2 GND, 3 FB, 4 EN, 5 IN, 6 LX ✔ — stimmt mit der Netzliste. Keine Strapping- oder Boot-Pins.

### Empfohlene Beschaltung (S. 8–9)
- Rückkopplungsteiler: VOUT = 0,8 V × (1 + R1/R2); R1, R2 zwischen **10 kΩ und 1 MΩ** ✔ S. 8. Das Datenblatt rechnet selbst das Beispiel „VOUT 3,3 V, R1 = 100 k ⇒ R2 = 32 k“ vor ✔ S. 8 — der Bestand folgt also dem Datenblatt-Beispiel.
- CIN: mindestens 1 µF am IN-Pin (S. 2), empfohlen **4,7 µF** X5R oder besser, dicht an IN und GND ✔ S. 8.
- COUT: **> 22 µF** X5R ✔ S. 8.
- Bootstrap: 100 nF zwischen BS und LX ✔ S. 8–9 (Bestand: C25 100 nF ✔).
- Induktivität: Welligkeitsstrom ca. 40 % von IOUT,MAX; L = VOUT·(1 − VOUT/VIN,MAX)/(fSW·IOUT,MAX·40 %); Sättigungsstrom grösser als der Spitzenstrom bei Volllast; **DCR < 50 mΩ „wünschenswert“** ✔ S. 8. Das Datenblatt sagt ausdrücklich, der Wandler sei „quite tolerant“ gegenüber abweichenden Induktivitäten ✔ S. 8.
- Zusätzlicher Kondensator parallel zu R1 kann die Lastsprungantwort beschleunigen ✔ S. 9.
- Layout: FB-Leiterbahn und R1/R2 nicht neben dem LX-Netz; LX-Fläche klein halten ✔ S. 9.
- Zur Rückkopplung **hinter** einer Ausgangsdiode sagt das Datenblatt **nichts** (nicht gelesen, weil nicht vorhanden).

## 4. Rechnungen (Zeichen ○ = Rechnung aus ✔-Werten; die Eingangswerte stehen oben)

### 4.1 Ausgangsspannung für R28 = 100 kΩ / R29 = 32 kΩ — **stimmt mit 3,3 V**
VOUT = 0,8 V × (1 + 100/32) = 0,8 × 4,125 = **3,300 V** (nominal).
Mit VREF-Toleranz 0,792 … 0,808 V: **3,267 … 3,333 V**. Mit ±1-%-Widerständen zusätzlich: 3,218 … 3,384 V ○.
Mit dem tatsächlich bestellten **31,6 kΩ**: 0,8 × (1 + 100/31,6) = **3,332 V** nominal, 3,298 … 3,365 V (VREF-Toleranz), mit ±1 % Widerständen 3,249 … 3,417 V ○.
Beides liegt im Bereich des ESP32-S3 (VDD33 3,0 … 3,6 V, **✔ ESP32-S3-WROOM-1 Datenblatt v1.8, S. 27**). Der Teilerstrom beträgt 3,3 V / 132 kΩ = 25 µA, weit über dem FB-Strom (±50 nA, S. 4).
Basic-Nachbarwerte für R29: 33 kΩ ⇒ 3,224 V, 30 kΩ ⇒ 3,467 V (nominal) ○ — beide noch im ESP-Bereich, aber nicht mehr 3,3 V.

### 4.2 Induktivität — **4,7 µH passt, mit zwei Anmerkungen**
Empfohlener Wert nach S. 8 für 1 A, 40 %, 2 MHz: L = 3,3 × (1 − 3,3/5,0)/(2 MHz × 1 A × 0,4) = **1,4 µH** (bei VIN 4,5 V: 1,1 µH; bei 5,5 V: 1,65 µH) ○.
Mit 4,7 µH ist die Welligkeit **kleiner**: ΔIL = VO(1 − VO/VIN)/(L·f) = 3,64 × (1 − 3,64/5)/(4,7 µH × 2 MHz) ≈ **0,10 A** (Spitze–Spitze, VIN 5 V) ○. Spitzenstrom bei 0,6 A Last ≈ 0,65 A, bei 1 A ≈ 1,06 A ○ — beide weit unter der kleinsten Strombegrenzung von 1,6 A (S. 4).
L2 (Cjiang, Rev. 7.0, PDF S. 18, Zeile FTC252010S4R7MBCA): L = 4,7 µH bei 0 A / 1 MHz; **Rdc 160 mΩ typ / 180 mΩ max; Irms 2,0 A typ / 1,6 A (zweite Spalte); Isat 2,6 A typ / 2,0 A (zweite Spalte)** ✔. Die Spaltenüberschrift führt „Typical“ und „Max“; bei den Strömen steht unter „Max“ der kleinere Wert, ich lese ihn als garantierten Mindestwert und rechne vorsichtig mit **Isat = 2,0 A**. Die Definition von Isat/Irms (Induktivitätsabfall, Erwärmung) habe ich **nicht gelesen** (steht nicht in den ausgewerteten Seiten).
- **Isat 2,0 A ≥ Spitzenstrom bei 1 A (1,06 A)** ✔ (Forderung S. 8 erfüllt). Aber **unter der Strombegrenzung des Wandlers von bis zu 2,5 A** (S. 4): Im Kurzschluss/Hiccup kann die Spule kurz in die Sättigung gehen. Das ist im Hiccup-Betrieb kein Dauerzustand, aber nicht belegt unschädlich.
- **Rdc 160 mΩ liegt 3,2-fach über der Empfehlung „DCR < 50 mΩ“** (S. 8). Verlust bei 0,6 A: 0,6² × 0,16 = **58 mW** (≈ 2,6 % von rund 2,2 W Ausgangsleistung samt Diode) ○, bei 1 A 160 mW. Kein Fehler, aber eine kleine Wirkungsgradeinbusse; thermisch unkritisch.

### 4.3 Reicht der Wandler für ESP32-S3-WROOM-1 plus Rest bei 600 mA? — **ja bei der Stromgrösse, knapp bei der Spannung**
- Stromgrösse: 600 mA = 60 % der 1 A (S. 1), Spitzenstrom 0,65 A ≪ 1,6 A (S. 4). Das Modul verlangt von der Versorgung **mindestens 0,5 A** ✔ (WROOM-1 Datenblatt v1.8, S. 27, Tabelle 6-2); der Chip nimmt beim WLAN-Senden **340 mA Spitze** auf (802.11b, 21 dBm) ✔ (ESP32-S3 Datenblatt v2.2, S. 66, Tabelle 5-7). Der Rest (DS18B20, DS3231, EEPROM, LEDs, Pegelwandler) ist klein (Werte nicht gelesen, ○); 600 mA hat damit mindestens 250 mA Reserve.
- Verlust im Wandler bei 0,6 A: Leitverluste ≈ 0,6² × (0,75 × 0,38 + 0,25 × 0,18) Ω ≈ **0,12 W** ○, dazu Schaltverluste (nicht angegeben). Bei 100 °C/W (S. 3) rund 12–20 K Erwärmung ○ — unkritisch.
- **Spannungsreserve ist das eigentliche Problem, siehe 4.5.**

### 4.4 D2 PMEG2010ER

| Grösse | Wert | Beleg |
|---|---|---|
| Sperrspannung | 20 V | ✔ S. 1, S. 2 |
| Mittlerer Durchlassstrom | **1 A** (δ = 0,5, Tamb ≤ 130 °C auf Keramik-Platine; Tsp ≤ 145 °C) | ✔ S. 1, S. 2 |
| Stossstrom IFSM | 50 A (8 ms) | ✔ S. 2 |
| Durchlassspannung 0,1 A | 220 mV typ / 250 mV max (25 °C) | ✔ S. 4 |
| Durchlassspannung 0,7 A | 290 / **320 mV** | ✔ S. 4 |
| Durchlassspannung 1 A | 310 / **340 mV** | ✔ S. 4, S. 1 |
| Durchlassspannung **0,5 A** | nicht angegeben; zwischen 0,1 A und 0,7 A, also **höchstens 320 mV**; Interpolation der Typwerte ≈ 0,27 V | ✔ Eckwerte S. 4, Interpolation ○ |
| Sperrstrom | 60 µA typ bei 5 V; 250 µA typ / **1000 µA max** bei 20 V (25 °C) | ✔ S. 4 |
| Wärmewiderstand j–a | 220 K/W (Standard-Pad, FR4), 130 K/W (1 cm² Kathode), 70 K/W (Keramik) | ✔ S. 3 |
| Gehäuse | SOD123W (CFP3), Kathode = Markierungsstrich, Pin 1 K, Pin 2 A; Markierung „B5“ | ✔ S. 2 |

Verlust bei 0,6 A: ≈ 0,6 × 0,30 V = **0,18 W** ○ ⇒ bei 220 K/W rund **40 K** Erwärmung ○ (23 K mit 1-cm²-Kupferfläche). Im Gehäuse bei 60 °C Umgebung wären das ≈ 100 °C Sperrschicht, erlaubt bis 150 °C (S. 2) — Sperrstrom steigt dort stark (Bild 5, nicht numerisch gelesen). Ausreichend, aber nicht grosszügig.

**Folge für die Spannung am Verbraucher:** Weil FB **hinter** D2 liegt, regelt der Wandler den Knoten hinter der Diode auf 3,3 V; der Spannungsfall der Diode wird im Gleichstrom **herausgeregelt**. Der Verbraucher sieht 3,3 V (3,27 … 3,33 V), der Knoten vor der Diode liegt entsprechend 0,27 … 0,34 V höher, also **etwa 3,6 V** ○. Läge FB vor der Diode, sähe der Verbraucher nur 3,3 − 0,3 ≈ 3,0 V, gerade an der unteren ESP32-Grenze (3,0 V, S. 27 WROOM-1). Der Preis der Schaltung ist **Spannungsreserve** (4.5) und eine Regelschleife, die die Diode enthält (4.6).

### 4.5 Spannungsreserve bei USB-Betrieb — **Auffälligkeit, am Prototyp messen**
Das Datenblatt nennt weder Dropout noch einen maximalen Tastgrad. Aus der Mindest-Ausschaltzeit (100 ns typ) und der Schaltfrequenz folgt ein **maximaler Tastgrad von 1 − 100 ns × fSW**: **0,80** bei 2 MHz, 0,76 bei 2,4 MHz, 0,84 bei 1,6 MHz ○. Der Knoten vor der Diode muss 3,3 V + VF ≈ 3,6 V erreichen; zusätzlich fällt an den inneren Schaltern und der Spule Spannung ab (380 mΩ, 180 mΩ, 160 mΩ). Rechnung für 0,6 A, typische Widerstände, konstante Frequenz ○:

| fSW | maximaler Tastgrad | VIN, ab der 3,3 V am Verbraucher noch gehalten werden | Ausgang hinter der Diode bei VIN = 4,75 V | bei VIN = 4,5 V |
|---|---|---|---|---|
| 1,6 MHz | 0,84 | 4,65 V | 3,39 V | 3,18 V |
| 2,0 MHz | 0,80 | **4,88 V** | 3,20 V | 3,00 V |
| 2,4 MHz | 0,76 | **5,13 V** | 3,01 V | **2,82 V** |

Bei 1 A (VF 0,34 V) steigt die nötige Eingangsspannung auf 4,94 … 5,44 V. `VCC_5V` ist der Ausgang des TPS2121 (RON 56–70 mΩ, S. 7) hinter der PTC-Sicherung F1 (laut Stückliste 1,5 A, 1206) — eine USB-Schiene mit 4,75–5,0 V minus diese Verluste liegt **in diesem Bereich**. Erwartung: Bei voller 3,3-V-Last und knapper 5-V-Schiene sinkt die 3,3-V-Schiene unter 3,3 V; unter 3,0 V (ESP32-Minimum) wäre es ein Brownout-Risiko. **Einschränkung:** Ob der Wandler bei erreichter Mindest-Ausschaltzeit die Frequenz senkt (dann entfällt das Problem weitgehend), steht nicht im Datenblatt — „nicht gelesen, nicht vorhanden“. Messen: 3,3-V-Schiene bei 4,5 / 4,75 / 5,0 V Eingang und 0,6 A.

### 4.6 Rückkopplung hinter der Diode — Folgen
- ✔/○ Gleichstromregelung korrekt (4.4). Die Diode steht **in der Regelschleife**; die interne Kompensation (S. 9: „integrates the compensation components“) ist für einen LC-Ausgang ohne Diode ausgelegt. Das Datenblatt macht dazu keine Aussage. Dynamisch trennt die Diode die Ausgangskondensatoren C27–C29 (vor der Diode, nominell 44 µF) vom Verbraucher; hinter der Diode bleiben C8 (22 µF) und die Modulkondensatoren. Der differentielle Widerstand der Diode (zwischen 0,1 A und 0,7 A ändert sich VF um höchstens 70 mV, S. 4 ⇒ ≤ 0,12 Ω) liegt als Serienwiderstand im Lastpfad.
- Anlauf: Der Knoten hinter der Diode folgt dem Knoten davor um VF, der Softstart (1 ms, S. 4) bleibt wirksam. Der Unterspannungsschutz (S. 4) überwacht FB, also den Knoten **hinter** der Diode; bei einem Kurzschluss von `VCC_3.3V` greift er wie beschrieben (Hiccup, S. 9).
- **Rückstromsperre**: Die Diode verhindert, dass eine andere 3,3-V-Quelle (z. B. DevKit-Versorgung oder Programmierer) in den Wandlerausgang zurückspeist. Das ist vermutlich ihr Zweck; der Grund steht nicht in den Unterlagen — Frage an den Architekten (offener Punkt 3).
- Empfehlung des Datenblatts bei grossen Lastsprüngen: kleiner Keramikkondensator parallel zu R1 (S. 9). Für unsere Schaltung **empfehle ich, die Lastsprungantwort zu messen** (Modul schaltet WLAN ein), bevor die Platine in Serie geht.

## 5. Fertigung bei JLCPCB (Prüfdatum **2026-10-17**)

Quelle: Teilebibliothek von JLCPCB über die Suchschnittstelle von `jlcpcb.com/parts` (Feld `componentLibraryType`, `preferredComponentFlag`, `stockCount`) und `wmsc.lcsc.com` für die Datenblattverweise. Alle Zahlen sind ● (Distributor-Datenbank).

| Teil | LCSC | Klasse | Lagerbestand JLCPCB | Gehäuse laut Quelle | SMT |
|---|---|---|---|---|---|
| U10 SY8301ABC | **C2927130** | **Extended** (nicht Preferred) | 20'824 | SOT-23-6 | ja, SMD |
| L2 FTC252010S4R7MBCA (cjiang) | **C5832360** | **Extended** | 392'763 | „1008“ (= 2,5 × 2,0 mm) | ja, SMD |
| D2 PMEG2010ER,115 (Nexperia) | **C82288** | **Extended** | 7'366 (LCSC zeigte 7'145) | SOD-123W | ja, SMD |

- Der Footprint im Schaltplan heisst `Nexperia_CFP3_SOD-123W` (Netzliste) — **das passt zu SOD-123W**; die in `jlcpcb-bestand.md` geäusserte Sorge „Footprint SOD-123“ bezieht sich auf die Stückliste, nicht auf den Schaltplan.
- L2: Der Footprint heisst `IND-SMD_L2.5-W2.0-VLS252012CX-150M` (EasyEDA); Aussenmasse 2,5 × 2,0 mm passen. Höhe 1,0 mm gegenüber 1,2 mm der Vorlage: kein Konflikt, aber die empfohlenen Lötflächen aus dem Cjiang-Datenblatt (PDF S. 5, Bild) habe ich nicht verglichen.
- **Preferred Extended:** bei keinem der drei gesetzt (Filter geprüft).
- **Basic-Alternativen**
  - **SY8301:** keine gleichwertige Basic. Suche „SY8301“ ergibt nur C2927130 (und eine JLCPCB-Assembly-Nummer ohne Lager). Basic-Wandler: C9864 TPS5430DDAR (3 A, Eingang erst ab 5,5 V ✘), C61063 XL1509-5.0E1 (fester Ausgang 5 V ✘). Als Linearregler wäre C6186 AMS1117-3.3 (SOT-223, Basic, 1'102'705) vorhanden: 1,1 V Dropout bei 800 mA, Verlust bei 0,6 A und 5 V ≈ **1 W** ○, Datenblatt nicht gelesen — **keine** Alternative ohne Entscheid des Architekten.
  - **L2:** keine Basic-Induktivität 4,7 µH gefunden (laut `jlcpcb-bestand.md`, nicht erneut geprüft).
  - **D2:** Basic-Schottky: C8598 B5819W SL (Jiangsu Changjing, **SOD-123**, 1 A / 40 V, 600 mV bei 1 A, Lager 367'155), C2480 SS14 (MDD, SMA, 550 mV bei 1 A, 1'261'878), C8678 SS34 (MDD, SMA, 3 A / 40 V, 550 mV bei 3 A, 3'887'128). Durchlassspannung laut Beschreibung der Quelle (●), Datenblätter nicht gelesen. **Begründung für Extended:** Die Basic-Dioden haben bei 1 A rund 0,2 bis 0,26 V mehr VF als die 0,34 V der PMEG2010ER ⇒ die Spannungsreserve von 4.5 würde um etwa denselben Betrag, geteilt durch den Tastgrad (≈ 0,25–0,33 V an VIN), schlechter. Mit FB hinter der Diode ist der niedrige VF der Grund für das Extended-Teil.

## 6. Fallen und Errata

- Datenblatt widerspricht sich bei der Mindest-Eingangsspannung (4 V auf S. 8, 4,5 V auf S. 1/3/4). Ich nehme 4,5 V.
- Das Silergy-Dokument trägt den Vermerk „Confidential – Prepared for Customer Use Only“ im Kopf; es ist über den Distributor frei verfügbar. Einen neueren Stand als Rev. 0.9A habe ich nicht gefunden.
- Der Wandler ist im Datenblatt nur für **1 A** spezifiziert. Wer später mehr Strom aus der 3,3-V-Schiene zieht (z. B. weitere Peripherie), hat keine Reserve ausser den 0,4 A zwischen 0,6 und 1 A.
- C29 (100 nF) im Netz vor der Diode zählt nicht zur Kapazität von „> 22 µF“ (S. 8); C27+C28 = 44 µF nominell genügen. Die Gleichspannungsabhängigkeit von X5R bei 3,6 V habe ich nicht nachgeschlagen (○).

## 7. Folgerung für unsere Platine

1. Die Beschaltung entspricht dem Datenblatt (Teiler, BS-Kondensator, EN an IN). Wert des FB-Teilers: 3,30 V (32 kΩ) bzw. 3,33 V (31,6 kΩ) — beide in Ordnung; die Stückliste und der Schaltplan müssen denselben Wert tragen.
2. **Spannungsreserve bei USB-Betrieb und Volllast messen oder rechnerisch absichern** (4.5). Wenn sie nicht reicht: kleinere VF (die Wahl der PMEG2010ER ist richtig), FB **vor** die Diode legen und den Verlust von 0,3 V akzeptieren (Verbraucher ≈ 3,0 V — nicht besser), oder die Diode entfernen, wenn der Rückspeiseschutz anders gelöst wird.
3. Ein Kondensator mit ≥ 4,7 µF dicht an IN/GND des Wandlers prüfen (Layout), `VCC_5V` hat C23/C26 an anderer Stelle.
4. Hinweis ausserhalb des Auftrags, aufgefallen beim Netzlistenlesen: Die Teile F1 (PTC 1,5 A) im USB-Pfad und F2 (PTC „1812L300“, 3 A) im Extern-Pfad sind **kleiner als 3 A LED + 0,6 A = 3,6 A** (nur aus den Bezeichnungen gelesen, Datenblätter nicht geprüft, ○). Das gehört zum Strombudget des Architekten.

## 8. Offene Punkte

1. Verhalten des SY8301 am Tastgradanschlag (Frequenzabsenkung oder nicht) — Datenblatt schweigt; Messung am Prototyp.
2. Isat/Irms-Definition des Cjiang-Datenblatts und die Lötflächenempfehlung (Bilder, nicht gelesen).
3. Zweck von D2 (Rückspeiseschutz?) — Frage an `hw-architect`.
4. Wirkungsgradkurve für VIN = 5 V, VOUT = 3,3 V: im Datenblatt nur für VIN 12/24 V → 5 V gezeigt (S. 1, S. 5); für unseren Arbeitspunkt nicht belegt.
5. Ob Reststrom von Peripherie (ausser dem Modul) unter 250 mA liegt — Strombudget des Architekten.
