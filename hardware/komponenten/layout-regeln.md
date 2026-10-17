# Komponentenblatt: Layout-Regeln für die ESP32-S3-Controllerplatine (2 Lagen, 145 × 39 mm, JLCPCB)

Stand der Prüfung: **17.10.2026**. Verfasser: Datenblatt-Analyst. Gegenstand ist kein Bauteil, sondern das Wissen, das der
`hw-layouter` für ein fertigbares KiCad-Layout braucht. Alle Zahlen tragen ein Zeichen:
✔ im Dokument gelesen (Seite oder Abschnitt), ● Sekundärquelle (genannt), ○ Annahme, eigene Rechnung oder Erinnerung —
○ ist **keine** Regel, sondern ein Vorschlag oder eine offene Frage. Webseiten haben keine Seitenzahlen; der Beleg ist dann der **Abschnitt**.
Eigene Rechnungen stehen mit ihren Annahmen dabei. Ergänzend: `jlcpcb-fertigung.md` (Fertigungsgrenzen, Dateien, DRC-Tabelle),
`kicad-regeln-und-footprints.md` und `kicad-layout-werkzeug.md` (KiCad-Seite). Dort Gesagtes wiederhole ich nicht.

## 1. Quellen und Kürzel

| Kürzel | Unterlage | Revision / Datum | Adresse, Datei (Dateien unter `datenblaetter/`, nicht im Git) |
|---|---|---|---|
| **DS-M**, **HDG** | ESP32-S3-WROOM-1 Datasheet; ESP32-S3 Hardware Design Guidelines | siehe `esp32s3-wroom-1.md` §1 (DS-M v1.8, HDG „Release master“; HDG: gedruckte Seite = PDF-Seite − 3) | `esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf`, `esp32-s3_hardware_design_guidelines_en.pdf` |
| **TI-HS** | TI SPRAAR7J „High-Speed Interface Layout Guidelines“, 21 S. | Nov 2018, rev. Feb 2023 | `ti.com/lit/an/spraar7j/spraar7j.pdf`; `ti-spraar7j-usb2-layout.pdf`. Die Kennwerte in Tab. A-1 gelten für TI-Prozessoren (AM335x), sind hier **Richtwerte** (●) |
| **TI-1229** | TI SNVA054C „AN-1229 SIMPLE SWITCHER PCB Layout Guidelines“, 15 S. | Juni 2002, rev. Apr 2013 | `ti.com/lit/an/snva054c/snva054c.pdf`; `ti-snva054c-an1229-simpleswitcher-layout.pdf` |
| **TI-QFN** | TI SLUA271C „QFN and SON PCB Attachment“, 21 S. | Juni 2002, rev. Dez 2023 | `ti.com/lit/an/slua271d/slua271d.pdf` (Dateiname d, Kopfzeile C); `ti-slua271-qfn-pcb-attachment.pdf` |
| **TI-ADC** | TI SBAS526D ADS1018, Abschnitt 11 „Layout“ (S. 30), 44 S. | Nov 2012, rev. Sep 2019 | `ti.com/lit/ds/symlink/ads1018.pdf`; `ti-ads1018.pdf` (Beispiel für Analog-Layout eines ADC-Bausteins) |
| **SY-AN** | Silergy AN_SY8301 Rev. 0.9A, Abschnitt „Layout Design“ (S. 9–10), 14 S. | 2020 | `datenblaetter/sy8301.pdf` |
| **TPS2121** | TI SLVSEA3F TPS2120/TPS2121, 45 S. | Aug 2020 | `datenblaetter/tps2121.pdf` |
| **HRO** | HRO TYPE-C-31-M-12, Zeichnung, 1 Blatt | gezeichnet 2020-12-08 | `datenblaetter/hro-type-c-31-m-12.pdf` |
| **KC-DOC** | KiCad 10 Dokumentation, Kapitel PCB Editor | 10.0 | `docs.kicad.org/10.0/en/pcbnew/pcbnew.html`, abgerufen 2026-10-17, Klartext selbst durchsucht |
| **KC-FP** | KiCad-10-Standardbibliothek: `RF_Module:ESP32-S3-WROOM-1`, `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` (Version 20260206) | in `KiCad.app` installiert | `/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints/…` (Dateien selbst gelesen) |
| **V2-PCB** | `V2.kicad_pcb`, `V2.kicad_pro`, `V2.kicad_dru` (nur lesend, per Skript) | Mär 2026 | `~/Documents/WordClock-USB C - STM32F411 - V2/` |
| **IPC-F** | Formel nach IPC-2221, wiedergegeben von Sierra Circuits (`protoexpress.com/?p=12833`) und NextPCB (`nextpcb.com/blog/pcb-trace-width-calculator-ipc-2221-guide`) | beide abgerufen 2026-10-17 | ● Sekundärquellen — **IPC-2221 und IPC-2152 selbst sind kostenpflichtig und nicht gelesen** |
| **ALT-CHK**, **ALT-2221** | Altium (Z. Peterson) „A PCB Layout Review Checklist Gets You to Manufacturing Quickly“ (geändert 2026-08-15); „Using an IPC-2221 PCB Clearance Calculator for High Voltage Design“ (geändert 2025-09-09) | siehe links | `resources.altium.com/p/pcb-design-review-checklist-gets-you-manufacturing-quickly`, `…/using-an-ipc-2221-calculator-for-high-voltage-design` (●) |
| **SIE-DFA** | Sierra Circuits „PCB Design for Assembly Checklist“ | abgerufen 2026-10-17 | `protoexpress.com/blog/pcb-design-for-assembly-checklist-important-checks-for-layout-engineers/` (●) |
| **JLC-…** | JLCPCB-Seiten | siehe `jlcpcb-fertigung.md` §1 | wie dort |
| **Blätter** | `usblc6-2sc6.md`, `usb-c-16p.md`, `sy8301abc.md`, `tps2121.md`, `leistungsbudget.md`, `entscheidungen.md` | 2026-10-17 | im Repo |

## 2. Ausgangslage: was V2 tatsächlich ist (aus der Datei gelesen)

- Zwei Kupferlagen `F.Cu`/`B.Cu`, 1,6 mm, Kupfer 0,035 mm, 667 Segmente, 339 Vias, **10 Zonen**, Hilfsursprung `aux_axis_origin (81.5 115.75)` = linke untere Ecke ✔ V2-PCB, Abschnitte `setup`, `layers`, `segment`, `via`, `zone`. **Bis auf `U1` (DS18B20, `B.Cu`) sitzen alle 108 Bauteile auf `F.Cu`** ✔ V2-PCB, Abschnitt `footprint` (Seite je Bauteil gelesen).
- Die Zonen in V2 (Abstand 0,25 mm; Thermal-Lücke und -Steg 0,5 mm bei neun Zonen, 0,25 mm bei einer; acht Zonen nur `F.Cu`, eine nur `B.Cu`, eine beide Lagen) sind GND-/Versorgungsflächen; ihre Netzzuordnung ist ohne Netzbezeichner in der Datei nicht lesbar (○ nicht ausgewertet) ✔ V2-PCB, Abschnitt `zone`.
- **Der Umriss ist kein Rechteck.** Zwölf Linien auf `Edge.Cuts`, Koordinaten vom linken oberen Eckpunkt (mm): (0/0) → (118,75/0) → **(118,75/7,5) → (135,25/7,5) → (135,25/0)** → (145/0) → (145/31,25) → **(79/31,25) → (79/39) → (66/39) → (66/31,25)** → (0/31,25) → (0/0) ✔ V2-PCB, Abschnitt `gr_line` auf `Edge.Cuts` (per Skript ausgelesen). Das ist **eine Aussparung für die ESP-12F-Antenne (16,5 mm breit, 7,5 mm tief, Mitte x = 127,00)** und **ein Streifen für den USB-Stecker (13 mm breit, 7,75 mm tief, Mitte x = 72,50)**. Die Aussenmasse 145 × 39 mm sind nur die Hülle; die Prüfung H7 vergleicht nur diese Hülle (§ „Feste Orte“).
  **Das widerspricht der Annahme in `entscheidungen.md` AE-16** („damit entfällt die Aussparung der Platine, die das HDG empfiehlt“): V2 hat eine Aussparung, und sie bleibt, wenn der Umriss 1:1 bleibt.
- **Regeldatei von V2 = PCBWay-Vorlage**; die V3-Datei `WordClock-ESP32S3-V3.kicad_pro` enthält **noch keine Board-Einstellungen** (Schlüssel `meta`, `schematic`, `sheets`) ✔ V3 `.kicad_pro`, Abschnitt oberste Schlüssel. Der Layouter muss Netzklassen, Regeln und `.kicad_dru` anlegen (H7 verlangt beide Dateien neben der Platine).
- Footprints mit **Courtyard** fehlen bei 23 von 109 V2-Teilen (alle EasyEDA-Importe: `SW4`, `D2`–`D5`, `C20`, `C22`, `LED1`, `LED4`, `LED5`, `U3`, `U6`, `U7`, `U9`, `U10`, `F1`, `F2`, `L1`, `L2`, `Q1`, `Q3`, `X1`, `D1`) ✔ V2-PCB, Abschnitt `fp_*` auf `F.CrtYd` (per Skript gezählt); die Courtyard-Prüfung von KiCad ist dort stumm.

## 3. Lagenaufbau und Masseführung (2 Lagen)

| Regel | Beleg |
|---|---|
| Zweilagig ist zulässig: **Lage 1 Bauteile und Signale; Lage 2 ohne Bauteile, möglichst wenig Bahnen, mit einer durchgehenden Massefläche für Chip, RF und Quarz** | ✔ HDG S. 20 (PDF 23), Abschnitt 1.4.1 |
| „There are no official two-layer modules“ (die Abbildung zeigt einen ESP32-Chip); die Regeln der Vierlagen-Variante gelten sonst sinngemäss | ✔ HDG S. 22 (PDF 25) |
| Eine Seite einer doppelseitigen Platine **fast vollständig mit Masse füllen**; Rückströme wollen unter dem Signal laufen (Feldauslöschung, kleinere Induktivität); die Fläche **nicht aufteilen**, viel Schaltleistung durch sie fliessen lassen führt zu „ground bounce“ | ✔ TI-1229 S. 12–13, Abschnitt 7 Ground Plane |
| Getrennte Analog-/Digitalmasse ist für das Rauschen **nicht nötig**; eine geschlossene Massefläche oder Masseauffüllung in Bereichen ohne Bauteile ist wesentlich; eine Zweilagenplatine mit gemeinsamer Masse ist möglich | ✔ TI-ADC S. 30, Abschnitt 11.1 |
| Durchgehende Bezugsmasse unter den USB-Leitungen; Leitungen **mindestens 90 mil (2,3 mm) vom Rand der Bezugsfläche** und **mindestens 1,5 W von Löchern in der Bezugsfläche** | ✔ TI-HS S. 9, Abschnitte 3.1–3.2, Aufzählung; HDG S. 30 (PDF 33) |
| Beim Modul: **Massefläche beidseits und dicht unter den Randpads**, Masse-Vias dicht an der Antenne; keine Leitungen quer durch die Bezugsfläche unter RF/USB | ✔ HDG S. 30 (PDF 33), Abschnitt 1.4.7 |
| Stichelvias (Masse-Vias zwischen beiden Lagen): Abstand symmetrisch **innerhalb 200 mil (5,1 mm)** von Übergangsvias, „näher ist besser“ | ✔ TI-HS S. 8, Abschnitt 2.4 (Abb. 2-11) |
| Raster der Stichelvias entlang Bahnen und Massekanten (Wert ohne Beleg) | ○ Vorschlag: ≤ 5 mm entlang der Platinenkante und um die Antennenfreihaltung, ≤ 3 mm um das USB-Paar |

**Eigenschaften unserer Platine:** V2 hat Bauteile fast nur oben und Masse unten/oben als Zonen (zehn Stück). Für V3: **`B.Cu` als zusammenhängende Masse, ohne Bahnen unter dem Modul ausser den unvermeidbaren** (○ Vorschlag, im HDG zweilagig „Bahnen auf ein Minimum“), `F.Cu` zusätzlich Masse füllen und **Inseln entfernen** („Remove islands“ auf „always“; KiCad lässt Inseln sonst als unverbundene Kupferflächen stehen ✔ KC-DOC, Abschnitt Zone properties, „Remove islands“).

## 4. Entkopplung

| Regel | Beleg |
|---|---|
| Modul: **22 µF und 0,1 µF an 3V3 (Pad 2)**, EN: 10 kΩ gegen 3,3 V und 1 µF gegen Masse | ✔ DS-M S. 41, Abb. 9-1 und Text darunter; HDG S. 8 (PDF 11) |
| Masse-Vias **dicht an der Masseseite des Kondensators** für einen kurzen Rückweg; Kondensatoren an den übrigen Versorgungspins | ✔ HDG S. 22 (PDF 25), Aufzählung „Place appropriate decoupling capacitors“ |
| Ein Hochfrequenzkondensator (0,1 … 0,47 µF) gehört **sehr nahe an VIN und GND** des Schaltreglers; die Masse-Zuleitung des Reglers muss kurz sein | ✔ TI-1229 S. 6–7, Abschnitt 4 „Placing Components acap“ |
| Jeder Zoll Leiterbahn hat rund **20 nH**; ein 1-A-Sprung in 30 ns über 1 Zoll erzeugt rund 0,7 V | ✔ TI-1229 S. 5, Abschnitt 3 |
| Bypass-Kondensator an die Versorgungspins „so nahe wie möglich“ | ✔ TI-ADC S. 30, Abschnitt 11.1; ● ALT-CHK, Abschnitt Component spacing/Decoupling |

**Rechnung ○** (Annahmen: Zuleitung plus Via je Kondensator zusammen 4 mm; 20 nH je Zoll ⇒ 0,79 nH/mm ⇒ 3,2 nH): 100 nF + 3,2 nH haben eine Eigenresonanz bei 1/(2π√(L·C)) = 8,9 MHz; mit 1 mm Zuleitung (0,8 nH) 17,8 MHz. Der Kondensator wirkt oberhalb davon induktiv, **jeder Millimeter Zuleitung kostet Wirkung**. Folgerungen (○ Setzungen): Kondensator **vor** dem Pad, nicht dahinter (Strom fliesst durch den Kondensatorpad zum Modul); Masse-Via **an den Kondensatorpad**, nicht am Ende einer Bahn; Zuleitung ≤ 2 mm; `C30` (22 µF) und `C31` (100 nF) nach `entscheidungen.md` P9 am Pad 2.

## 5. Leistungsbahnen und Strombelastbarkeit (3,5 A LED-Versorgung, 5-V-Pfade)

**Formel (● IPC-F, Sierra und NextPCB gleichlautend):** I = k · ΔT^0,44 · A^0,725 mit I in A, ΔT in K, A als Leiterquerschnitt in mil²; **k = 0,048 aussen, 0,024 innen**. Auflösung nach der Breite W = A/(1,378 · Cu-Gewicht in oz) in mil. IPC-2221 gilt als **konservativ**, weil es Kühlung durch Flächen und Massekupfer ignoriert; IPC-2152 wertet dagegen Kupfergewicht, Leiterlage, Umgebung und Ebenen mit Kennlinien aus ● IPC-F, Abschnitt IPC-2221 vs. IPC-2152.

**Annahmen (offen gelegt):** Strom 3,0 / 3,5 / 3,6 A (Entwurfswert 3,5 A nach `leistungsbudget.md` §4; **3,6 A für Leitungen** nach ebd. Schluss 6 ✔ `leistungsbudget.md`, Abschnitt 4, Punkt 6), Aussenlage, **1 oz = 35 µm** (JLCPCB rechnet im Rechner 1,6 mil = 40,6 µm ✔ JLC-IMPG, Abschnitt Calculation Parameters; ich rechne mit den kleineren 35 µm, also vorsichtig), ΔT 10/20/30 K über Umgebung, Leiter frei auf FR-4. Für FR-4 gilt: **unter 120 °C bleiben**, das ist die Obergrenze des Laminats ✔ TI-1229 S. 10, Abschnitt 6.1. Bei 40 °C Gehäuseluft (○ Annahme, Quelle: `leistungsbudget.md` Punkt 3) heissen ΔT 20 K → 60 °C, ΔT 30 K → 70 °C.

| Strom | ΔT 10 K | ΔT 20 K | ΔT 30 K | Bemerkung (○ eigene Rechnung nach IPC-F, 1 oz, aussen, k = 0,048) |
|---|---|---|---|---|
| 0,5 A | 0,12 mm | 0,08 mm | 0,06 mm | Logik an 5 V |
| 1,5 A | 0,53 mm | 0,35 mm | 0,27 mm | Strom je Stift von `H7` bei 3 A |
| 3,0 A | 1,37 mm | 0,90 mm | 0,70 mm | LED-Kette, Entwurf |
| **3,5 A** | **1,69 mm** | **1,11 mm** | **0,87 mm** | Entwurfswert Summe an `VCC_5V` |
| **3,6 A** | **1,76 mm** | **1,15 mm** | **0,90 mm** | **Auslegung nach `leistungsbudget.md`** |
| 4,0 A | 2,03 mm | 1,33 mm | 1,04 mm | Obergrenze Dauerstrom TPS2121 (4,5 A) liegt darüber |
| 3,6 A, 2 oz | 0,88 mm | 0,58 mm | 0,45 mm | nur falls 2 oz gewählt (JLCPCB: 2 oz 2 Lagen, Bahn/Abstand ≥ 0,16 mm ✔ JLC-CAP, Abschnitt Traces) |
| 3,6 A, **innen**, 1 oz | 4,57 mm | 3,00 mm | 2,35 mm | zur Warnung: Innenlage braucht die doppelte Breite — hier nicht relevant (2 Lagen) |

Zur Gegenprobe die **Faustregel von TI: mindestens 12 mil Breite je Ampere bei 1 oz, bei Erwärmung < 30 °C und Strömen < 5 A** (3,6 A ⇒ 43 mil = **1,10 mm**; 4 A bei 20 °C ⇒ 54 mil = 1,4 mm) ✔ TI-1229 S. 10–11, Abschnitt 6.1. Beide Wege stimmen mit der Zeile ΔT 20 K (1,11 … 1,15 mm) überein.

**Spannungsabfall (○ Rechnung):** Flächenwiderstand 1 oz (35 µm, ρ = 1,72 · 10⁻⁸ Ω·m bei 20 °C) = **0,49 mΩ/sq**; bei 3,6 A über 100 mm: 1,0 mm breit **177 mV**, 2,0 mm **88 mV**, 3,0 mm 59 mV, 5,0 mm 35 mV (Verlust 0,64 / 0,32 / 0,21 / 0,13 W). Auf der 5-V-Schiene (3,3-V-Regler braucht Reserve, `leistungsbudget.md` §5) zählt jede 10 mV.

**Vias (○ Rechnung):** Hülsenkupfer ≈ π · d · t mit mittlerer Hülsendicke 18 µm ✔ JLC-CAP, Abschnitt Drilling Features. Via 0,3 mm: 0,0170 mm² (entspricht einer 0,48 mm breiten 1-oz-Bahn) ⇒ nach IPC-F bei ΔT 20 K **1,9 A** (k = 0,048) bis **0,96 A** (k = 0,024, vorsichtig); bei 0,4 mm Bohrung 2,4 / 1,2 A. Für 3,6 A also **mindestens 4 Vias parallel** (vorsichtig gerechnet; 2 genügen nur mit k = 0,048). HDG: „mindestens zwei Vias“, wenn eine Hauptversorgung die Lage wechselt ✔ HDG S. 22 (PDF 25). **Vorschlag:** den 3,6-A-Pfad ohne Lagenwechsel führen (oben `VCC_5V`/`TO_VCC_RGB_5V`, Rückleiter als Fläche) und an den Übergängen zu `H7` mindestens 6 Vias setzen.

**Folgerungen für die Netze:**
- `VCC_5V`, `TO_VCC_RGB_5V`, GND-Rückleiter, `USB_5V_PROT`/`ALT_5V_PROT` vor `U9`: **≥ 1,2 mm** (ΔT 20 K); wo Platz ist, ≥ 2 mm oder Fläche (V2 führt dafür eine Netzklasse `Power_1` mit 2,0 mm ✔ V2 `.kicad_pro`, Abschnitt `net_settings.classes`). Zwischen `Q1` und `H7` sind es zwei Stifte (je 1,5 A): zwei Bahnen von je ≥ 0,6 mm genügen rechnerisch (ΔT 20 K 0,35 mm bei 1,5 A), besser eine Fläche.
- 3,3-V-Schiene hinter `D2`: 0,6 A ⇒ ≥ 0,15 mm rechnerisch; **HDG fordert für die Hauptversorgung eines Chips ≥ 25 mil (0,64 mm)** und 10 mil (0,25 mm) für die übrigen Zweige ✔ HDG S. 22 (PDF 25); für das Modul als Vorsichtswert ≥ 0,64 mm von `U10`/`D2` bis zum Modul (○ Übertragung vom Chip auf das Modul).
- Kupfer-Gewicht: **1 oz bleibt** (wie V2); 2 oz kostet Mehrpreis und verlangt 0,16-mm-Regeln (○ Folgerung aus ✔ JLC-CAP, Abschnitt Traces).

## 6. Abstände und Spannungsfestigkeit

- **Kleinste Spannungen:** alle Netze ≤ 5,5 V; das TVS `D4`/`D5` (SMAJ5.0CA) klemmt höher, aber kurzzeitig (○). Für Leiterabstände nach IPC-9592B (Stromrichter): **< 15 V: 0,13 mm; 15 … 30 V: 0,25 mm** ● ALT-2221, Abschnitt IPC-9592B. IPC-2221 Tabelle 6-1 nennt für 0 … 30 V aussen unbeschichtet **0,1 mm** (Bild der Seite nicht lesbar, Wert aus einer Suchmaschinen-Zusammenfassung, **○**).
- Damit bestimmt **die Fertigung** (0,10 mm Mindestabstand ✔ JLC-CAP, Abschnitt Traces), nicht die Spannung, den kleinsten Abstand. **Vorschlag ○:** 0,15 mm allgemein (V2-Standardklasse 0,1524 mm), **0,2 mm** zwischen `VCC_5V`-Kupfer und Signalen am Modul, ≥ 0,3 mm zu Platinenkante und Aussparung.
- **Kriechstrecken** nach Netzspannung entfallen: die Platine führt nur Kleinspannung (○ Annahme: kein Netzteil auf der Platine).

## 7. Thermik

| Bauteil | Beleg | Folgerung |
|---|---|---|
| `U10` SY8301 | θJA 100 °C/W, θJC 30 °C/W (Zweilagen-Evalboard), PD 0,4 W bei 25 °C ✔ `sy8301abc.md` §3 (Datenblatt S. 3); SY-AN S. 9: **Kupferfläche am GND-Pin maximieren** (beste Wärme- und Rauschwerte), bei Platz Massefläche | Rechnung ○: 3,3 V · 0,6 A = 1,98 W Ausgang, Wirkungsgrad 85 … 90 % (Annahme, Kurve nicht gelesen) ⇒ 0,22 … 0,35 W Verlust ⇒ ΔT 22 … 35 K bei 100 °C/W; Obergrenze Tj 150 °C ✔ `sy8301abc.md` §3. Masse-Pin über mehrere Vias in die untere Massefläche |
| `U9` TPS2121 | 3,5 A ⇒ 1,04 W; θJA 72,2 °C/W auf einer **JEDEC-Platine** ⇒ 75 K ✔ `leistungsbudget.md` §4 Punkt 3; „short wide traces für Ein- und Ausgangsflächen, **Vias unter Ein-/Ausgangspins**“ ✔ TPS2121 S. 33, Abschnitt 12 | TI veröffentlicht θJA allgemein für eine **Vierlagenplatine mit Vias (JEDEC High-K)** ✔ TI-QFN S. 7, Abschnitt 3.4; dass das Datenblatt-θJA des TPS2121 auf diesem Aufbau beruht, folgt aus dem Wort „JEDEC“ (○); auf 2 Lagen ist der Wert dann schlechter: Wärmekupfer auf beiden Lagen, Vias nahe den Ein-/Ausgangspins |
| `Q1`, `Q3` PowerPAK SO-8 | 85 mW bzw. 0,12 W, RthJA 65 °C/W bei 1″ × 1″ FR-4 ✔ `si7149dp.md`, Abschnitt Temperaturanstieg (Datenblatt S. 1) | unkritisch; Drainfläche als Kupferfläche anbinden |
| `F1`, `F2` PTC | `F1` Verlust 0,29 W bei 1,5 A ✔ `leistungsbudget.md` Abschnitt 4, Punkt 4 | PTC-Haltestrom sinkt mit der Temperatur: **nicht** neben `U9`/`U10` und nicht unter dem Modul |
| Thermische Vias unter Pads | Bohrung 0,3 … 0,33 mm, Raster 1,0 … 1,2 mm ✔ TI-1229 S. 12, Abschnitt 6.3; Raster ≈ 1,0 mm, 0,3 mm Bohrung, kleineres Via mindert Zinnverlust ✔ TI-QFN S. 7, Abschnitt 3.4.1; AN-1229 empfiehlt Vias **neben**, nicht unter dem Pad (Zinnabfluss) ✔ TI-1229 S. 12 | JLCPCB haftet nicht für Folgen von Via im Pad (✔ JLC-TERMS, Abschnitt 5, Punkt 6, siehe `jlcpcb-fertigung.md` §5): Vias **neben** den Pads oder verkleinert, nicht offen im Pad |

## 8. Analoge Eingänge (LDR an `IO1`)

- Der ADC-Eingang bekommt **0,1 µF gegen Masse** am Pin ✔ HDG S. 18 (PDF 21); ADC1 verwenden ✔ ebd. Der Kondensator (`C32`) liegt **am Modulpad 39 (IO1)**, Masse über kurzes Via; Teiler `R22` (1 kΩ) ebenfalls dort (○ Setzung).
- **Analoge Teile fern von digitalen, HF, USB und Schaltreglern** halten; Rückströme digitaler Leitungen nicht unter analogen Knoten führen; Bypass am Versorgungspin; Masseauffüllung ohne Aufteilung ✔ TI-ADC S. 30, Abschnitt 11.1.
- **Schaltregler:** Rückkopplungsleitung nicht unter Induktivität/Diode und nicht parallel zu Leistungsleitungen führen, nur kreuzen ✔ TI-1229 S. 13, Abschnitt 8; das gilt als Muster für jede hochohmige Leitung neben `L2`/`D2`.
- **Quellimpedanz** des Teilers ist 0,9 … 1 kΩ (`esp32s3-wroom-1.md` §6.3); die Leitung von `H5`/`H7.5` bis zum Modulpad 39 (**rechte Spalte, 8,8 mm unter der Moduloberkante**, x = 135,75) ist **sehr lang** (|135,75 − 38,6| ≈ 97 mm in x, ○ gerechnet): ○ Setzung: Hin- und Rückleiter (Masse) eng beieinander; **Pad 39 liegt am Antennenende des Moduls** (1,3 mm unter Pad 40, rund 9 mm unter dem Antennenbereich): `C32` unmittelbar am Pad, Abgang **nach rechts (x > 136,5) und unten**, nicht entlang der Antennenzone; **nicht** parallel zum LED-Datenpfad (`WS2812_BUS`), nicht neben `L2` (x = 58,75/17,5) und nicht zwischen `U10` und `L2`.
- **Schirmung gegen den Schaltregler:** Es gibt **keinen Wert** für Mindestabstände in den gelesenen Unterlagen (○). Vorschlag mit Begründung: ≥ 5 mm zu `L2` und zu `LX`, Masse-Führungsleiter beidseits der Leitung (Breite zweimal Signalbreite, Rat einer Suchmaschinen-Zusammenfassung ○), Vias in der Masse entlang der Leitung.

## 9. Schaltregler-Layout (`U10` SY8301, `L2`, `D2`)

| Regel | Beleg |
|---|---|
| `CIN`, `L1`, `R1`, `R2` **dicht am IC**; `CIN` direkt an IN und GND, **Schleifenfläche CIN–GND minimal** | ✔ SY-AN S. 9, Abschnitt Layout Design, Punkte 2 |
| **Kupferfläche am LX-Pin minimal** halten (Störantenne) | ✔ SY-AN S. 9, Punkt 3; ✔ TI-1229 S. 12, Abschnitt 6.3 („Do not increase width of copper on switching node injudiciously“, S. 2, Regel 6) |
| **`R1`, `R2` und die FB-Leiterbahn nicht neben dem LX-Netz** | ✔ SY-AN S. 9, Punkt 4 |
| Fläche am GND-Pin so gross wie möglich; Massefläche | ✔ SY-AN S. 9, Punkt 1 |
| Die kritischen Leitungen sind die **„AC-Pfade“**, in denen der Strom springt (Eingangskondensator ↔ IC ↔ Diode), alle anderen sind gleichstromartig | ✔ TI-1229 S. 3–5, Abschnitt 3 |
| Fangdiode (hier `D2`) **sehr nahe an SW- und GND-Pin**, kurze und breite Leitungen | ✔ TI-1229 S. 7, Abschnitt 4 |
| Der Eingangskondensator darf grösser/weiter weg sitzen (bis rund 1 Zoll), wenn der HF-Kondensator am IC sitzt | ✔ TI-1229 S. 7, Abschnitt 4 |
| Rückkopplung hinter einer Diode: nach den Quellen **nicht beschrieben** (nicht gelesen, weil nicht vorhanden) | ✔ `sy8301abc.md` Abschnitt 3 |

**Folgerungen (○ Setzungen aus den belegten Regeln):** `C25` (BS) und `C26`/`C23` (Eingang) liegen am IC; `C26` 10 µF an `U10` nach `entscheidungen.md` (Auftrag Layouter); LX-Fläche nur Pad plus kurze Bahn zu `L2` und `D2`-Anode; **FB** über `R28`/`R29` mit Rückleitung zum Ausgangsknoten **hinter** `D2` (V2), Bahn nicht unter `L2`/`D2`, nur kreuzen; Mindestabstand 2 mm zu LX-Fläche (○). Platzbedarf in V2: `U10` (58,75/12,5), `L2` (58,75/17,5), `D2` (67,5/19,0).

## 10. USB 2.0 Full-Speed (Differentialpaar, ESD)

| Regel | Beleg |
|---|---|
| Paar parallel und **gleich lang**, **Impedanz 90 Ω ±10 %**, möglichst wenige Vias, an jedem Übergang **ein Paar Masse-Vias**, durchgehende Bezugsmasse, Paar von Masse umgeben | ✔ HDG S. 30 (PDF 33), Abschnitt 1.4.8 |
| Platz für **Reihenwiderstände (22/33 Ω) und Kondensatoren gegen Masse** nahe am Chip, zunächst unbestückt | ✔ HDG S. 18 (PDF 21); Entscheid AE-02: 2 × 0 Ω (`R34`/`R35`), Plätze für 2 C (`entscheidungen.md`) |
| Längenunterschied im Paar **≤ 50 mil (1,27 mm)**; Gesamtlänge ≤ 12 000 mil (305 mm); **Impedanz 81 … 99 Ω (90 Ω ±10 %)**, Gleichtakt 40,5 … 49,5 Ω; **≤ 4 Vias** je Leitung, **0 Stichleitungen, 0 Testpunkte** auf dem Paar; Abstand zu Takt/periodischen Signalen ≥ 50 mil (1,27 mm), zu jedem anderen Signal ≥ 30 mil (0,76 mm) | ● TI-HS S. 16, Tabelle A-1 (Richtwerte für TI-Chips) |
| **5W-Regel:** Abstand zwischen Paaren ≥ 5-fache Bahnbreite; **30 mil (0,76 mm)** Sperrbereich zu anderen Signalen; Winkel **> 135°**, keine 90°-Ecken | ✔ TI-HS S. 9 (Abschnitt 3.1) und S. 13 (Abb. 3-10) |
| Stichleitungen von Vias **< 15 mil (0,38 mm)**; bei Durchsteck-Buchse die Datenleitung auf der Unterseite anbinden, bei **SMD-Buchse auf der Oberseite ohne Via** | ✔ TI-HS S. 11 (Abschnitt 3.6) und S. 10 (Abschnitt 3.5) |
| **Nicht** unter oder nahe Quarzen, Takterzeugern, Schaltreglern, Befestigungslöchern führen; keine Prüf-/Testpunkte auf dem Paar | ✔ TI-HS S. 9, Abschnitte 3.1–3.2 |
| Schutzbaustein **so nahe wie möglich am Stecker**; **Durchlaufführung** der Datenleitungen durch den Baustein; ungeschützte Bahnen von geschützten fernhalten | ✔ TI-HS S. 14–15, Abschnitte 3.13–3.14 |
| `U11` USBLC6-2SC6: Datenleitungen durch die Paare (1→6, 3→4), Pin 5 an VBUS, Pin 2 an Masse; **10 mm Bahn = 6 nH heben die Klemmspannung bei 8 kV von rund 31 V auf über 300 V**; Bahn vom Stecker zum Baustein, vom Baustein zur Masse so kurz wie möglich; VBUS-Kondensator 100 nF (`C33`) am Pin 5 | ✔ `usblc6-2sc6.md` §4 (ST-Datenblatt S. 5–6, 8–9); `entscheidungen.md` AE-02 |
| `USB1`: Stichleitungen A6/B6 und A7/B7 **≤ 3,5 mm**, B-Seite nicht der Reihe nach verbinden, `CC1`/`CC2` je eigener 5,1 kΩ, `SBU` offen, Schirm **direkt an Masse** an allen Schirmstiften | ✔ `usb-c-16p.md` §3.2, §8 (USB-Typ-C-Spezifikation S. 69, S. 135) |

**Impedanz auf 2 Lagen — was geht, was nicht:** JLCPCB bietet **keine** Impedanzkontrolle für 2 Lagen an (`jlcpcb-fertigung.md` §4). Für 90 Ω differentiell über der durchgehenden Masse der Unterseite braucht es rechnerisch **~1,2 mm breite Leiter bei 0,2 mm Abstand** (○ Näherung nach IPC-2141, ±8 %; Dielektrikum 1,51 mm, εr 4,5): das passt nicht zum 0,5-mm-Raster der Buchse (Pads 0,3 mm breit, Lücke 0,2 mm ✔ KC-FP, Pads von `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12`, Abschnitt `pad`) und erst recht nicht zum Modulpad (0,9 mm breit, Raster 1,27 mm ✔ DS-M S. 45, Abb. 11-1). **Praktisches Vorgehen (○ Vorschlag, mit den ✔-Regeln oben):** Paar **kurz** (Strecke `USB1` → Modul rund 55 … 60 mm, ○ gemessen aus V2-Koordinaten: `USB1` x = 72,5, Modulpads 13/14 bei x ≈ 117,5 … 119 und y ≈ 22,8 … 24,1), **gleich lang (≤ 1,27 mm Unterschied)**, **parallel mit konstantem Abstand**, 0 Stichleitungen, ≤ 2 Vias, **beide Bahnen oben**, **durchgehende Masse darunter**, Masse-Stichelvias im Abstand ≤ 3 mm (○) neben dem Paar, 45°-Ecken. Wirkung auf den Betrieb bei 12 Mbit/s: **nicht belegt** (kein Dokument gibt eine Toleranzgrenze für Full-Speed-Leitungen). Der DevKitC-1 hat auf dem „ESP USB“-Anschluss nur ESD-Dioden und keine Reihenwiderstände und funktioniert ✔ `esp32s3-wroom-1.md` §7 (DK-Sch Blatt 2).

**Zwei Fassungen der Länge:** Entscheid P8 verlangt `U11` ≤ 5 mm von den `USB1`-Pads (○ Setzung des Architekten), siehe `entscheidungen.md` Abschnitt 8, P8, — mit TI (nahe am Stecker) vereinbar.

## 11. ESP32-S3-Modul (Antenne, Masse, Strapping, EN)

| Regel | Beleg |
|---|---|
| **Antennenbereich 6 mm tief** am oberen Modulrand, Modul 25,5 × 18 mm; **Landemuster** 40 Randpads 1,5 × 0,9 mm im Raster 1,27 mm, EPAD aus 3 × 3 Feldern zu je 0,9 mm im Abstand 0,5 mm (insgesamt 3,7 × 3,7 mm), **Abstand Moduloberkante bis Mitte Pad 1: 7,49 mm**, Pad-Abstand links/rechts 17,5 mm (Mitte–Mitte) | ✔ DS-M S. 45, Abb. 11-1; S. 42, Abb. 10-1 |
| **Die Modulantenne gehört über die Platinenkante hinaus**; wenn nicht möglich, den **Speisepunkt so nahe wie möglich an der Platinenkante** und die Trägerplatine **beidseits und unterhalb der Antenne ausschneiden**; das Modul nicht in die Mitte setzen; **ausreichend Massefläche und dichte Masse-Vias** auf der Trägerplatine nahe der Antenne | ✔ HDG S. 30 (PDF 33), Abschnitt 1.4.7 |
| **Freihaltefläche:** 6 mm tief über die Antennenbreite; seitlich auf der Seite der Aussparung **mindestens 15 mm**, auf der Gegenseite **höchstens 2 mm**, unten **höchstens 1 mm** | ✔ HDG S. 31 (PDF 34), Abb. 26 |
| Im Gehäuse **mindestens 15 mm Abstand in alle Richtungen** | ✔ HDG S. 30 (PDF 33) |
| **USB, USB-Seriell-Bausteine, UART-Leitungen (Bahnen, Vias, Prüfpunkte, Stifte) so weit wie möglich von der Antenne**; UART mit Masse und Masse-Vias umgeben | ✔ HDG S. 28 (PDF 31), Abschnitte 1.4.4 und 1.4.6 |
| **EPAD:** Löten nicht Pflicht, verbessert die Wärme; **richtige Pastenmenge**, zu viel hebt das Modul und lässt Randpads offen | ✔ DS-M S. 41, Text unter Abb. 9-1 |
| EPAD als **quadratisches Raster**, Lücken mit Lotpaste bedeckt, **Masse-Vias in den Lücken**; EPAD mit breiter Verbindung an eine **grosse Massefläche** | ✔ HDG S. 22 (PDF 25), letzte Aufzählungspunkte (Chip-Beschreibung, gilt sinngemäss) |
| **EN:** 10 kΩ + 1 µF, Leitung **so kurz wie möglich** (Störungen lösen sonst Resets aus), EN nie schweben lassen | ✔ HDG S. 8 (PDF 11), Abschnitt 1.3.3; DS-M S. 41, S. 11 |
| **GPIO0:** Pull-up, Taster nach Masse, **kein grosser Kondensator**; **GPIO45 nicht hoch** beim Einschalten (N8: Flash-Spannung); GPIO46 niedrig; GPIO3 unbeschaltet | ✔ `esp32s3-wroom-1.md` §4.1 (HDG S. 15, DS-M S. 39) |
| **Reihenwiderstand 499 Ω in U0TXD** gegen Oberwellen, Bahn kurz, von Masse umgeben | ✔ HDG S. 13 (PDF 16), S. 28 (PDF 31) |
| Quarz (im Modul) — nicht Layout-Aufgabe; Modul-HF-Leitung bis zur Antenne ist im Modul geführt | ✔ DS-M S. 39, Abb. 8-1 („50 ohm Impedance Control“ im Modul) |

**Was der KiCad-10-Standardfootprint `RF_Module:ESP32-S3-WROOM-1` mitbringt (Datei gelesen, Version 20260206; Koordinaten mit Ursprung Modulmitte):**
- 40 Randpads 1,5 × 0,9 mm bei x = ±8,75 mm (Pad 1 y = −5,26 mm; Pad 14 y = +11,25 mm; Pads 15 … 26 am unteren Rand bei y = +12,5 mm), **EPAD 3,9 × 3,9 mm auf `F.Cu`/`F.Mask` ohne Paste**, 9 Pastenflächen 0,9 × 0,9 mm im Raster 1,4 mm, **12 Durchkontaktierungen Pad 0,6 mm / Bohrung 0,2 mm** (Eigenschaft Wärmepad) — **in den Lücken zwischen den Pastenflächen**, so wie HDG und DS-M es zeichnen ✔ KC-FP, Abschnitt `pad` (per Skript ausgelesen). Pastenanteil: 9 · 0,81 mm² = 7,29 mm² gegen 15,21 mm² Kupfer = **48 %** (gegen 3,7 × 3,7 mm: 53 %) — TI nennt **50 … 70 %** der Fläche als gute Pastenabdeckung eines freiliegenden Pads ✔ TI-QFN S. 11, Abschnitt 4.4 (○ Rechnung der 48 %).
- **Eine Sperrfläche (Rule Area) im Footprint:** Rechteck x = −24 … +24 mm, y = −27,75 … −6,75 mm, **alle Kupferlagen**, `tracks`, `vias`, `pads`, `copperpour` und `footprints` **nicht erlaubt**; dieselbe Fläche ist der Courtyard der Antennenseite ✔ KC-FP, Abschnitt `zone` und `fp_line` auf `F.CrtYd`. Das sind 48 × 21 mm: **15 mm seitlich über das Modul hinaus** (18 + 2 · 15 = 48), **15 mm über die Moduloberkante** (12,75 + 15 = 27,75) und **bis 6 mm unterhalb der Moduloberkante** (12,75 − 6,75 = 6,0). Das entspricht der HDG-Zahl „15 mm“ auf **beiden** Seiten, strenger als HDG Abb. 26 (15 mm auf einer, höchstens 2 mm auf der anderen Seite).

**Die Kollisionen mit V2-Festpunkten (○ Lage des Moduls: Mitte x = 127,0 wie `U4`, Oberkante y ≈ 0,05 mm wie der Courtyard von `U4`; die Zahlen sind selbst gerechnet aus V2-Koordinaten, Abschnitt `footprint`, und KC-FP, Abschnitt `pad`):**

| Fall | Zahl | Folge |
|---|---|---|
| **Sperrfläche des Footprints gegen `H6`** | Sperrfläche in Platinenkoordinaten x = 103,0 … 151,0, y = −14,95 … +6,05 (bei Oberkante 0,05); `H6`-Pads 1, 2 und 3 liegen bei x = 107,56 / 105,02 / 102,48, y = 4,50, Pad-Radius 0,9 mm ⇒ **drei Pads und das Gehäuse in der Sperrfläche** | DRC-Fehler `items_not_allowed`; `H6` darf nicht wandern (1:1). **Die Sperrfläche muss im Projekt-Footprint verkleinert werden — Abweichung von HDG (15 mm) und vom Standardfootprint, die der Architekt entscheidet.** Das haben auch die Versuche in `kicad-regeln-und-footprints.md` §4.1 gezeigt. |
| **Abstand `H6` ↔ Modul** | nächste `H6`-Padkante x = 108,46; Modulkante 118,0 ⇒ **9,54 mm**; V2-`U4`: Pad-Aussenkante 118,15 ⇒ 9,69 mm, Courtyard 117,95 ⇒ 9,49 mm | praktisch wie V2 (Differenz 0,15 mm); die 15 mm von HDG sind **nicht erreichbar**, solange `H6` bleibt. Reichweite entscheidet die RSSI-Messung (Spec F4) |
| **Aussparung gegen Modulbreite** | Aussparung x = 118,75 … 135,25 (16,5 mm), Modul 118,0 … 136,0 (18,0 mm) ⇒ **je 0,75 mm Modul über Platinenmaterial**; Aussparungstiefe 7,5 mm gegen Antennenbereich 6 mm (Reserve 1,5 mm) | Antennenbereich des Moduls liegt in der Aussparung, die Ränder der Antennenfläche aber auf FR-4: **0,75 mm je Seite schlechter als der Idealfall** (HDG fordert das Ausschneiden beidseits). Verbreiterung der Aussparung ändert den Umriss (Entscheid Nutzer) |
| **Pads 1 und 40 gegen die Aussparungsecken** | Pad 1: Mitte y = T + 7,49, **Oberkante T + 7,04**; Aussparungsgrund y = 7,5. Mit T = 0,05: Oberkante 7,09 ⇒ **das Pad ragt 0,41 mm in die Aussparung** (Padbereich x = 118,75 … 119,0 über der Aussparung) | Kupfer über der Fräskante: DRC `copper_edge_clearance`, Fräser schneidet das Pad an. **Modul muss mit Oberkante T ≥ 0,66 mm sitzen** (Kupfer ≥ 0,2 mm vom Rand ✔ JLC-CAP, Abschnitt Outline; mit V2-Regel 0,3 mm: T ≥ 0,76 mm), oder die Aussparung wird tiefer (Entscheid Nutzer). Die V2-`U4`-Pads beginnen bei y = 8,25, also 0,75 mm unter dem Aussparungsgrund. |
| **`SW1` gegen Pad 14** (IO20, D+) | Modulpad 14: Mitte (118,25 / T + 24,0), Aussenkante **x = 117,5**; `SW1`-Pad 2: x = 115,0 … 117,5, y = 24,95 … 26,05 ⇒ **die Kanten fallen in x zusammen**; bei T = 0,05 ist der Abstand in y 0,45 mm, bei T = 0,76 **0 mm (Überlappung in y)** | Kurzschluss/Brückenrisiko am USB-Pin. Abhilfe: Modul um **≥ 0,75 mm nach +x** (Aussenkante ≥ 118,25 ⇒ Abstand 0,75 mm wie V2 0,65 mm) **oder** `SW1` um ≥ 0,5 mm nach −x (H7 erlaubt ≤ 1,0 mm; „exakt“ des Nutzers nicht erfüllt). **Entscheid beim Architekten/Nutzer.** |
| **Courtyard des Moduls** | Courtyard des Standardfootprints: Rechteck x = ±9,75 mm um die Mitte (117,25 … 136,75), y bis +13,45 mm (bei T = 0,05: 26,25), dazu die Sperrfläche oben ✔ KC-FP, Abschnitt `fp_line` auf `F.CrtYd`; `SW1`-Courtyard reicht bis x = 117,75 ⇒ **Überlappung 0,5 mm** | `courtyards_overlap` mit `SW1`, `H6`, `R12`; der Versuch in `kicad-regeln-und-footprints.md` §4.1 fand 7 Überlappungen |
| **`R12`** (EN-Pull-up, V3 behält ihn) | V2-Lage (116,5 / 13,75): Padkante x = 117,2 gegen Modul-Pad-Aussenkante 117,5 ⇒ 0,3 mm | verschieben: `R12`/`C1` gehören nach `esp32s3-wroom-1.md` §8 so kurz wie möglich an Pad 3 (EN, y = T + 10,03) |
| **`H10`** | Pad-Kante x = 139,05; Modul-Pad-Aussenkante 136,5 ⇒ 2,55 mm | frei |

**Wo die belegten Pads liegen** (○ Lage des Moduls wie oben: Mitte x = 127,0, Oberkante y = 0,05; bei Oberkante T verschieben sich alle y um T − 0,05; Pad-Koordinaten ✔ KC-FP, Abschnitt `pad`; Belegung ✔ `pinbelegung.md` §2–3):

| Pad | Pin | Netz | Mitte x / y (Platinenkoordinaten) |
|---|---|---|---|
| 3 | EN | `EN` (`R12`, `C1`, `SW3`) | 118.25 / 10.08 |
| 5 | IO5 | `WS2812_BUS` | 118.25 / 12.62 |
| 6 | IO6 | `WS2812_POWER` | 118.25 / 13.89 |
| 7 | IO7 | `TSOP` | 118.25 / 15.16 |
| 8 | IO15 | `DCF77` | 118.25 / 16.43 |
| 9 | IO16 | `USER` | 118.25 / 17.70 |
| 10 | IO17 | `WPS` | 118.25 / 18.97 |
| 11 | IO18 | `LED_STM` | 118.25 / 20.24 |
| 12 | IO8 | `SDA_DS3231` | 118.25 / 21.51 |
| 13 | IO19 | `USB_DN_M` (D−) | 118.25 / 22.78 |
| 14 | IO20 | `USB_DP_M` (D+) | 118.25 / 24.05 |
| 17 | IO9 | `SCL_DS3231` | 122.56 / 25.30 |
| 27 | IO0 | `BOOT` | 135.75 / 24.05 |
| 36 | RXD0 | `TX_UART` ← `H6.2` | 135.75 / 12.62 |
| 37 | TXD0 | `RX_UART` → `H6.3` (über `R33`) | 135.75 / 11.35 |
| 38 | IO2 | `DS18B20` | 135.75 / 10.08 |
| 39 | IO1 | `LDR` | 135.75 / 8.81 |

**Folgerungen (○, aus den Koordinaten):** Die linke Spalte (Pads 1 … 14, x = 118,25) schaut ins Platineninnere: LED-Daten, LED-Versorgung, IR, DCF77, Taster, I²C und **USB (Pads 13/14 im unteren Teil, nächster Weg zu `USB1`)**. Die **rechte Spalte** trägt dagegen **UART (Pads 36/37), DS18B20 (38), LDR (39) und BOOT (27)**; Pads 36 … 39 liegen **8,8 … 12,6 mm unter dem Antennenende**, ihre Gegenstellen (`H6` x ≈ 100 … 108, `U1` x = 71,2, `H5`/`H7` x = 26 … 46) liegen **auf der anderen Seite des Moduls**. Die Leitungen müssen unter dem Modul (`B.Cu`, hinter dem Antennenbereich) oder durch den 4,7-mm-Streifen zwischen Modulunterkante (y ≈ 26,8) und Platinenkante (31,25) laufen — **HDG verlangt UART-Leitungen weit weg von der Antenne und von Masse umgeben** (HDG S. 28). Das sind **vier lange Leitungen** (UART ×2, 1-Wire, ADC) durch denselben Engpass. Die Pinwahl ist an die DevKit-Pins gebunden (`pinbelegung.md`); ob einzelne Signale auf Pads der linken Spalte wechseln dürfen — **IO4 (Pad 4) ist ADC1-fähig** ✔ `esp32s3-wroom-1.md` §6.1 und liegt in der linken Spalte, weit vom Antennenende —, ist eine **Frage an den Architekten**, nicht an den Layouter (○ Vorschlag, die Pinregeln H3 müssten dann geändert werden).

## 12. Footprint- und Pad-Regeln

| Regel | Beleg |
|---|---|
| Footprints nach **IPC-7351B „medium“ oder „low density“**; „high density“ wird nicht unterstützt; Bauteilabstände ebenso | ✔ JLC-TERMS, Abschnitt 2, Punkte 1–2 (siehe `jlcpcb-fertigung.md` §7) |
| Footprint **muss den Massen des Datenblatts entsprechen**; **Pin 1 und Polarität im Footprint und in der Bestückungszeichnung markieren**; Pad-Form zum Anschluss passend; Stand des aktuellen Datenblatts | ● SIE-DFA, Abschnitt 1 |
| **NSMD-Pads** (Lackfenster 0,1 … 0,14 mm grösser als der Anschluss) statt maskendefiniert für gleichmässige Lötstellen; bei 0,5-mm-Raster Lack um alle Pads einer Seite mit ≤ 0,05 mm Abstand; Innenecken der Maske runden; **Routing- und Via-Sperre neben Pin 1 bei QFN** | ✔ TI-QFN S. 8, Abschnitt 3.5 (und S. 5, Abschnitt 3.1) |
| Lackfenster **1:1** zum Pad; JLCPCB: mindestens 0,09 mm zwischen Lackfenster und Nachbarbahn, Brücke ≥ 0,10 mm | ✔ JLC-CAP, Abschnitt Soldermask |
| KiCad: **Maske und Paste global auf 0 lassen**, „die meisten Hersteller erwarten Null und rechnen selbst“; Brücken kleiner als die Bahnbreite werden zusammengelegt | ✔ KC-DOC, Abschnitt Board Setup → Solder Mask/Paste |
| **Thermisches Pad:** Pad so gross wie das Gehäusepad, Abstand zu den Randpads ≥ 0,2 mm; **Paste auf 50 … 70 % der Padfläche** (Gesamtabdeckung 1:1 hebt das Bauteil an); Schablone 0,125 mm für 0,5-mm-Raster; Hohlräume ≤ 50 % | ✔ TI-QFN S. 7 (Abschnitt 3.4) und S. 10–11 (Abschnitte 4.2–4.4) |
| **TPS2121-Land-Pattern und Schablone des Herstellers** (VQFN-HR, `RUX0012A`): Beispiel mit freiliegendem Metall, Lackfenster 0,05 mm rundum, **Schablone „EXPOSED PAD 100 % PRINTED COVERAGE“ bei 0,1 mm Dicke** — **das widerspricht der Empfehlung 50 … 70 % von TI-QFN** (beides TI) | ✔ TPS2121 S. 39–40, Abschnitt Example Board Layout, Example Stencil Design |
| Der V2-Footprint von `U9` (EasyEDA `VQFN-HR-12…`) hat **12 Pads** (1 rechteckig, 11 oval) ✔ V2-PCB, Abschnitt `footprint` U9; ob er dem TI-Muster (S. 39) mit der freiliegenden Fläche entspricht, ist **nicht verglichen** | ○ offen |
| **USB-C `HRO TYPE-C-31-M-12` (KiCad-Standardfootprint):** SMD-Reihe 0,3-/0,6-mm-Pads, 1,45 mm lang bei y = −4,045 (Ursprung = Mitte des Körpers), **vier Schirmstifte** plattiert oval 0,6 × 1,7 und 0,6 × 1,2 mm bei x = ±4,32, y = −3,13 und +1,05, **zwei Fixierlöcher** NPTH Ø 0,65 bei (±2,89 / −2,6); Fab-Rechteck x ±4,47, y ±3,65 ✔ KC-FP, Abschnitt `pad` und `fp_line` | JLCPCB: Langloch **plattiert ab 0,5 mm Breite, Länge ≥ 2 × Breite** (0,6 × 1,2 mm genau am Minimum, 0,6 × 1,7 mm darüber) ✔ JLC-CAP, Abschnitt Drilling Features; Schirmstifte laut HRO-Zeichnung Ø 0,50, Bohrung Ø 0,60 ✔ HRO, Layout-Zeichnung |
| `USB1`: Abstand **Pad-Rückkante – Platinenkante 5,79 mm** und **Abstand der Schirmschlitze 8,64 / 8,65 mm, Reihenabstand 4,18 mm** laut HRO-Layout-Zeichnung (Zeichnung gesehen, Masslinien am „PCB EDGE“) | ✔ HRO, Abschnitt Zeichnung „Recommend PCB layout“ (Blatt 1) |
| **Gegenüber V2:** die Schirmstifte liegen auch in V2 bei x = ±4,32 (68,18 / 76,82); die Reihenabstände unterscheiden sich (V2 3,81 mm, HRO 4,18 mm) ✔ V2-PCB, Abschnitt `footprint` USB1 und KC-FP | Gehäuse-Öffnung: Breite 8,94 (HRO) gegen Breite des V2-Steckers (≈ 9,0, Silk ±4,5 mm) vergleichbar; Öffnungshöhe des V2-Steckers und **Lage der Frontfläche** nicht belegt (siehe § „Feste Orte“, Lücke 5) |
| Footprint-Ursprung für Bestückung: Standard „Mitte der Pads“ (Plugin) — `FT Origin` nur bei Abweichung | ● FT, README Abschnitt ⑤ |
| **Via im Pad** vermeiden (JLCPCB haftet nicht); Wärmevias daneben | ✔ JLC-TERMS, Abschnitt 5, Punkt 6 |
| Pad-Mindestgrössen: SMD-Pad ≥ 0,25 × 0,25 mm; Pad zu Pad (andere Netze) ≥ 0,15 mm; Ring PTH-Pad ≥ 0,25 mm empfohlen | ✔ JLC-CAP, Abschnitt Traces |

## 13. Beschriftung, Polarität, Testpunkte

- **Siebdruck:** Linie ≥ 0,15 mm, Texthöhe ≥ 1,0 mm (40 mil), Breite:Höhe 1:6, Pad zu Siebdruck ≥ 0,15 mm ✔ JLC-CAP, Abschnitt Legend. Texte nicht auf Pads, nicht unter Bauteilen, höchstens zwei Ausrichtungen, alle Referenzen lesbar ● ALT-CHK, Abschnitt Silkscreen. V2-DRC-Baseline: **142 Siebdruck-Überlappungen, 58 Randverletzungen, 14 + 9 Text-Meldungen** ✔ `tools/checks/hardware-drc.baseline`, Abschnitt Schlüssel `silk_overlap`, `silk_edge_clearance`, `text_thickness`, `text_height`; KiCad meldet Siebdruck unter Maske und über der Kante ✔ KC-DOC, Abschnitt DRC Readability checks.
- **Polarität:** SMD-Teile prüft der Kunde in der 2D-Datei, THT in der 3D-Datei ✔ JLC-TERMS, Abschnitt 2, Punkt 6. Pin 1 am Modul (Kerbe/Dreieck der Fabrikations-Lage `-9,7/-6,0`) ✔ KC-FP, Abschnitt `fp_poly` auf `F.SilkS`. Dioden (`D1`–`D5`, `D2`/`D3` Kathode), Elkos (keine, nur Keramik), LEDs (`LED1`, `LED4`, `LED5`), `U7`-Halter (+), Steckerstift 1 beschriften; Stift 1 aller `H`-Stecker mit „1“ am Rand (○ Setzung).
- **Testpunkte:** V2 hat **sechs** (`TP1`, `TP4`–`TP8`, Pad Ø 1,0 mm ✔ V2-PCB, Abschnitt `footprint`); jeder zugängliche Messpunkt für Versorgung und Busse ● ALT-CHK, Abschnitt Test points; **keine Prüfpunkte auf dem USB-Paar** ● TI-HS S. 16, Tabelle A-1 (0 Test Points); keine Prüfpunkte nahe der Antenne ✔ HDG S. 28 (PDF 31). Pad mindestens 0,25 mm (JLCPCB), besser ≥ 1 mm Ø wie V2; Pad länger als der Bauteilanschluss, damit die Prüfspitze den Pad trifft ● SIE-DFA, Abschnitt 8.
- Bestückungsseite: **alles SMD auf `F.Cu`**; `U1` (THT, V2 unten) — Entscheid Architekt/Nutzer (`jlcpcb-fertigung.md` §8).

## 14. Design for Manufacturing und Assembly, Panelisierung, Fiducials, Rand

Die Fertigungsgrenzen stehen in `jlcpcb-fertigung.md`; hier die Layout-Folgen:

- **Bauteilkörper zur Platinenkante ≥ 2,5 mm** (JLCPCB-Montage) ✔ JLC-TERMS, Abschnitt 2, Punkt 3; Sekundärquelle nennt 25 mil für kleine Teile ● SIE-DFA, Abschnitt 5.4 — **V2 hält das nicht ein:** laut Courtyard-Rechnung (Skript gegen den 12-Linien-Umriss) liegen im Abstand ≤ 0,3 mm zur Kante: `H4` 0,04, `H6`/`H3`/`H7` 0,05/0,05/0,08 (Gehäuse **6,0 mm über die Kante hinaus**), `SW1`–`SW3` 0,2, `H5` 0,25, `H10` 0,30 mm; weiter innen `C4`, `C21`, `R4` 0,55 und `TP1` 1,9 mm; `USB1` ragt **1,38 mm** über die Kante ✔ V2-PCB, Abschnitt `fp_*` auf `F.CrtYd` (per Skript gerechnet). Wie JLCPCB es handhabt, ist offen (`jlcpcb-fertigung.md` §15).
- **Innenecken des Umrisses sind scharf** (Aussparung, USB-Hals): Fräser rundet auf R 0,5 mm ✔ JLC-BLOG-RND, Abschnitt CNC Routing Techniques (siehe `jlcpcb-fertigung.md` §6); nichts Kupfer- oder Bauteil-Kritisches näher als 0,5 + 0,3 mm an die Ecke.
- **Fiducials/Randstreifen:** Standard-Bestückung verlangt sie; JLCPCB ergänzt sie, wenn sie fehlen ✔ JLC-TERMS, Abschnitt 5, Punkt 5. **Bei 39 mm Höhe unter dem Standardminimum von 70 mm** (`jlcpcb-fertigung.md` §6, §15). Fiducial Ø 1 mm, Lackfenster Ø 2 mm, ≥ 3,35 mm von der Kante ✔ JLC-RAIL, Abschnitt Fiducials. Lokale Fiducials an Feinraster-Teilen (Modul 1,27 mm, TPS2121 0,5 mm) ● SIE-DFA, Abschnitt 2 (3 globale + 2 lokale diagonal).
- **Platz für die Randstreifen:** Verhältnis Kante ↔ Stecker beachten: H3/H6/H7-Gehäuse stehen **6 mm über die obere Kante** (Randstreifen oben würde sie berühren, ○ offen).
- **Panelisierung** (falls Nutzen nötig): Abstand 1,6 oder 2 mm, Haltestege mit Mouse-Bites ≥ 4 mm, **Teile und Bahnen ≥ 118 mil (3 mm) von den Stegen**, V-Nut meiden, wenn Teile über der Kante hängen ● SIE-DFA, Abschnitt 9 (Entscheid der Fertigung: `jlcpcb-fertigung.md` §6).
- **Courtyards überall setzen** (23 V2-Teile haben keinen); Courtyard-Prüfung aktivieren (`missing_courtyard` steht in V2 auf „ignore“, siehe `kicad-regeln-und-footprints.md` §1.6).
- **Thermischer Ausgleich** der Pads (Grabstein/Verzug): Kupferdichte beider Lagen ausgleichen; Pads kleiner Bauteile gleichmässig mit der Fläche verbinden (Thermal-Speichen) ✔ JLC-TERMS, Abschnitt 2, Punkte 4–5; ● ALT-CHK, Abschnitt Thermal reliefs.

## 15. Review-Checkliste (für Layouter und `hw-reviewer`)

Jeder Punkt hat seinen Beleg oben; „Wie“ nennt die Prüfung.

| # | Prüfpunkt | Wie |
|---|---|---|
| 1 | Umriss = V2 **einschliesslich Aussparung und USB-Streifen** (12 Linien, Koordinaten §2) | Skript: Linienliste vergleichen; H7 prüft nur die Hülle |
| 2 | Lage von `H1`/`H2`/`H8`–`H10` (≤ 0,05 mm), `USB1`/`H4`–`H7`/`SW1`–`SW3` (≤ 1,0 mm laut H7; **gewollt exakt**) und **Drehung, Seite, Bohr-/Padgrösse** | H7 plus Zusatzskript (§ „Feste Orte“) |
| 3 | DRC mit den JLCPCB-Werten aus `jlcpcb-fertigung.md` §16, nicht mit der PCBWay-Datei; `.kicad_pro` und `.kicad_dru` neben der Platine | `kicad-cli pcb drc` im Projektordner |
| 4 | Leistungsbahnen ≥ 1,2 mm (3,6 A, ΔT 20 K) oder Fläche; ≥ 4 Vias je Lagenwechsel | Sichtprüfung, Breitenliste |
| 5 | Entkopplung: `C30`/`C31` am Modulpad 2, Massevia am Kondensator, Zuleitung ≤ 2 mm | Messung der Abstände |
| 6 | Antennenzone: kein Kupfer, keine Bauteile, **alle Lagen**; USB/UART/Testpunkte weit weg; Abweichung von 15 mm schriftlich begründet | Rule Area, Sichtprüfung |
| 7 | USB-Paar: Längenunterschied ≤ 1,27 mm, ≤ 2 Vias, keine Stichleitungen, durchgehende Masse; `U11` am Stecker, Durchlaufführung | `skew`, `via_count` in `.kicad_dru` |
| 8 | Strapping: `IO0` Pull-up und Taster ohne Kondensator, `IO45`/`IO46`/`IO3` frei; `EN`-Leitung kurz | Netzliste (H3) + Sichtprüfung |
| 9 | Schaltregler: LX-Fläche klein, FB nicht neben LX, `CIN` am IC | Sichtprüfung |
| 10 | Modul-EPAD: Pastenanteil 48 … 53 %, Vias in den Lücken, Masse breit angebunden | Pastenlage prüfen |
| 11 | Siebdruck ≥ 1,0 mm, nicht auf Pads, Pin-1-/Polarität sichtbar; Referenzen lesbar | DRC `silk_*`, Sichtprüfung |
| 12 | Courtyard bei jedem Teil; keine Überlappung | `courtyard_clearance`, `missing_courtyard` an |
| 13 | Bestückungsdaten: Hilfsursprung = linke untere Ecke, Bezeichner BOM = CPL, Drehungen gegen JLCPCB-Vorschau | `jlcpcb-fertigung.md` §9 |
| 14 | **Bauteil-Kante ≥ 2,5 mm**: Ausnahmen (Stecker, `USB1`) schriftlich | Warnregel §16 |
| 15 | Nach der Prüfung einmal **absichtlich** verletzen (ein Loch 1 mm verschoben, eine Bahn 0,05 mm zu schmal), bis die Prüfung anschlägt | DIR-014 |

Ergänzende Punkte gängiger Checklisten (●): Spur/Abstand getrennt für Aussen- und Innenlagen, Bauteilabstand (Gehäuse ragen über Pads hinaus), Bohrergrössen konsolidieren, Bohrabstände, fehlende oder dünne Lackbrücken (Lackschicht in der Endkontrolle einschalten), Wärmeverbindungen, Ring, Kupfer zum Rand, Dicke, Kupfergewicht, Impedanzstrukturen, Via-Füllung, Langlöcher, Passungen, Registrierung, Oberfläche ● ALT-CHK, Abschnitte DFM Review Points und Advanced checks; Footprint gegen Datenblatt, Fiducials, saubere Stückliste, Funktionsgruppen trennen, Abstände Teil–Teil/Teil–Loch/Teil–Ring, Lackfreiräume, Siebdruck, Prüfpunkte, Nutzenabstände ● SIE-DFA, Abschnitte 1–9.

## 16. Fertigung bei JLCPCB (Verweis; Prüfdatum 2026-10-17)

Alle Fertigungsgrenzen, Dateien, Drehkorrekturen, Gebühren und die **DRC-Tabelle für die `.kicad_dru`** stehen in **`jlcpcb-fertigung.md`** (Abschnitte 3 und 16). Kurz, was das Layout davon trägt:
- 2 Lagen, 1,6 mm, 1 oz; Bahn/Abstand ≥ 0,10 mm, Via ≥ 0,25 mm (ohne Mehrpreis ab Loch 0,3 mm), Ring PTH ≥ 0,18 mm (empfohlen 0,25 mm), Kupfer zum Rand ≥ 0,2 mm, Siebdruck ≥ 1,0 mm Texthöhe ✔ JLC-CAP, Abschnitte Traces, Drilling Features, Legend, Outline.
- **Standard-PCBA** wegen des Moduls (C2913198 „Standard Only“) ✔ JLC-PART, Abschnitt Attribute von C2913198.
- `H4`–`H7` und `U1` Bestückungsart „Wave Soldering“ ✔ JLC-PART, Abschnitt Attribute von C492415, C5451632; `USB1` C165948 „SMT Assembly“ ✔ JLC-PART, Abschnitt Attribute von C165948.

## 17. Feste Orte: Bauteile mit mechanischer Bindung (V2, per Skript gelesen)

Koordinaten in mm, **vom linken oberen Eckpunkt des Umrisses** (Ursprung x = 81,5 / y = 76,75 in KiCad-Koordinaten), Drehung = KiCad-`at`-Winkel (positiv gegen den Uhrzeigersinn in der Ansicht von oben), Seite = Lage des Footprints. Quelle jeweils ✔ V2-PCB, Abschnitt `footprint` (Position, Drehung, Seite, Pads, Bohrungen).

**Umriss (H7 vergleicht nur die Hülle 145,00 × 39,00 mm):** Linienzug in §2. Aussparung x = 118,75 … 135,25, y = 0 … 7,5; USB-Streifen x = 66 … 79, y = 31,25 … 39; ✔ V2-PCB, Abschnitt `gr_line` auf `Edge.Cuts`.

| Ref | Teil | Lage x / y | Drehung | Seite | Pads, Löcher (Mitte in Platinenkoordinaten) | Heutige H7-Prüfung | Bemerkung |
|---|---|---|---|---|---|---|---|
| `H8` | Loch M2,5 | 3,25 / 28,00 | 0° | F.Cu | Bohrung 2,7, Pad 5,4 | **fest, ≤ 0,05 mm** | Lage und Pad; Bohrdurchmesser ungeprüft |
| `H9` | Loch M2,5 | 72,50 / 3,25 | 0° | F.Cu | 2,7 / 5,4 | fest ≤ 0,05 | |
| `H10` | Loch M2,5 | 141,75 / 28,00 | 0° | F.Cu | 2,7 / 5,4 | fest ≤ 0,05 | |
| `H1` | Pad Netzeingang | 35,75 / 27,75 | 0° | F.Cu | Bohrung 2,2, Pad 3,8 | fest ≤ 0,05 | Netz `ALT_5V_INPUT` |
| `H2` | Pad Netzeingang | 30,25 / 27,75 | 0° | F.Cu | 2,2 / 3,8 | fest ≤ 0,05 | |
| `USB1` | USB-C (V2: 6-polig) | 72,50 / 35,38 | 0° | F.Cu | SMD-Reihe y = 33,35 (x 69,8 … 75,2); Schirmstifte x = 68,18 und 76,82, y = 33,60 und 37,41, Langloch 0,7 × 1,5; Courtyard y = 32,38 … 40,38 | Stecker ≤ **1,0 mm** | **ragt 1,38 mm über die Kante** (Courtyard), 1,0 mm (Siebdruck bis y = 40,0) |
| `H4` | Stift 1×3 | 12,00 / 4,29 | −90° | F.Cu | Pads x = 12,00, y = 1,75 / 4,29 / 6,83; Bohrung 1,1, Pad 1,8 | Stecker ≤ 1,0 mm | LED-Board; Courtyard 0,04 mm von der Kante |
| `H5` | Stift 1×2 (LDR) | 26,00 / 3,25 | −90° | F.Cu | Pads x = 26,00, y = 1,98 / 4,52 | Stecker ≤ 1,0 mm | |
| `H6` | Stift 1×4 (Konsole) | 103,75 / 4,50 | 180° | F.Cu | Pads x = 107,56 / 105,02 / 102,48 / 99,94, y = 4,50 | Stecker ≤ 1,0 mm | Gehäuse **6,0 mm über die obere Kante**; **in der Sperrfläche des Modulfootprints** (§11) |
| `H7` | Stift 1×7 (LED) | 38,63 / 4,75 | 180° | F.Cu | Pads x = 46,25 … 31,01 (Raster 2,54), y = 4,75 | Stecker ≤ 1,0 mm | Schnittstelle LED-Board; H8 prüft Netze, **nicht** die Lage; Gehäuse 6,0 mm über die Kante |
| `SW1` | Taster | 112,00 / 27,75 | 0° | F.Cu | Pads x = 107,75 / 116,25, y = 25,50 / 30,00, 2,5 × 1,1 | Stecker ≤ 1,0 mm | **0,5 mm vor dem Modulpad 14** (§11) |
| `SW2` | Taster | 99,00 / 27,75 | 0° | F.Cu | wie `SW1` | ≤ 1,0 mm | |
| `SW3` | Taster | 86,00 / 27,75 | 0° | F.Cu | wie `SW1` | ≤ 1,0 mm | Reset (EN) |
| `U2` neu | ESP32-S3-WROOM-1 | an der Stelle von V2-`U4`: 127,00 / 12,25 | 0° | F.Cu | Raster siehe §11 | **nicht erfasst** (V2-`U4` hat keine Prüfung) | Modul **ohne** V2-Ort; Mitte x = 127,0 vorgegeben (`entscheidungen.md` AE-16), Oberkante laut §11 |
| `SW5` neu | BOOT-Taster | **kein V2-Ort** | – | – | – | – | **neu**; Entscheid AE-03 Option A „in der freigewordenen STM32-Fläche“; **nicht 1:1 gebunden**, muss aber Antennenfreihaltung, USB-Leitungen und die Sperrfläche des Moduls meiden |
| `C32` neu | 100 nF am ADC | kein V2-Ort | – | – | – | – | neu |
| **entfallen** | `H3` (SWD) 92,49 / 4,50, 180°; `SW4` (Schiebeschalter) 113,75 / 2,00, 180° | – | – | – | – | – | `H3` steht nicht in der H7-Liste; `SW4` ebenso; in V3 gibt es beide nicht |
| nicht von H7 erfasst, aber mechanisch/funktional gebunden | `U7` CR2032-Halter 17,25 / 18,75, 180°; `LED1` 74,65 / 10,50, 180°; `LED4` 56,25 / 19,00, −90°; `LED5` 66,00 / 11,00, −90°; `U1` DS18B20 71,23 / 23,00, 0°, **B.Cu**; `TP1` 67,75 / 30,50; `TP4` 67,75 / 26,00; `TP5` 45,50 / 24,00; `TP6` 56,25 / 25,75; `TP7` 54,00 / 23,50; `TP8` 63,75 / 14,50; `JP2` 49,75 / 7,75; `JP3` 64,75 / 25,00 | | | | | **keine** | ob Gehäuse oder Batterietür an diesen Orten hängen (Lichtleiter der LEDs, Batteriefach, Sensorabstand), steht in den Unterlagen **nicht** (○); der Nutzer entscheidet, ob sie 1:1 bleiben |

**Lücken der heutigen Lageprüfung (H7 in `tools/checks/hardware.py`; Quelltext gelesen):**

1. **Es wird nur die Hülle des Umrisses verglichen** (Breite und Höhe des Begrenzungsrechtecks, Toleranz 0,05 mm). Aussparung (16,5 × 7,5 mm) und USB-Streifen (13 × 7,75 mm) sind **nicht** geprüft; ein Layout ohne Aussparung oder ohne Hals des USB-Streifens besteht die Prüfung, solange die Hülle 145 × 39 mm bleibt. Auch Innenschnitte oder Bögen würden nicht auffallen.
2. **Nur der Footprint-Ursprung wird verglichen** (`at x/y`), **nicht Drehung und nicht Seite.** Ein um 180° gedrehter `H7` (Stiftreihenfolge gespiegelt) oder ein auf die Unterseite gelegter Taster besteht. `H8` prüft die Netze, nicht die Geometrie. `U1` (B.Cu) und `USB1` (Öffnung zur Kante) sind drehungs- und seitenkritisch.
3. **Toleranz 1,0 mm für `USB1`, `H4`–`H7`, `SW1`–`SW3`** — der Auftrag lautet „exakt“. Für `H7` (Kabel zum LED-Board) und `SW1`–`SW3` (Gehäuseöffnungen) wären 1 mm sichtbar. Löcher und Pads (`H1`, `H2`, `H8`–`H10`) sind streng (0,05 mm).
4. **Der Footprint-Ursprung ist bei `USB1` nicht vergleichbar:** V2 hat den 6-poligen Footprint mit Ursprung rund 2,03 mm hinter der Pad-Mitte, die KiCad-HRO-Footprint hat ihn in der Körpermitte, 4,045 mm vor der Pad-Mitte. Bei **gleicher Pad-Reihe** (y = 33,35) läge der HRO-Ursprung bei y = 37,40 (Abweichung 2,0 mm ⇒ H7 **schlägt an**, obwohl die Pads gleich liegen); bei gleicher **Frontfläche** (Siebdruck y = 40,0) liegt er bei etwa y = 36,1 (Abweichung 0,7 mm ⇒ H7 **besteht**, obwohl die Pads über 1 mm weiter oben sitzen). Die Prüfung sagt über die Gehäuse-Passung des Steckers **nichts**. Nötig: Pad- und Frontlage gegen die Gehäusezeichnung (liegt nicht im Repo, ○).
5. **Der Stecker ragt über die Kante** (V2: Courtyard 1,38 mm; HRO nach Zeichnung: Pad-Rückkante 5,79 mm von der Kante bei 7,35 mm Gesamtlänge ⇒ Front ≈ 1,56 mm über der Kante ○ Zuordnung der Masslinie) — **die Lage der Frontfläche ist die gehäuserelevante Zahl** und wird nicht geprüft. Die HRO-Tiefe 7,35 mm (JLCPCB-Feld „Length 7,35 mm“ ● JLC-PART, Abschnitt Attribute von C165948) gegen 6,8 mm des 6-poligen (● C456012) ⇒ **0,55 mm tiefer**; Breite 8,94 mm (HRO) gegen rund 9,0 mm. Die Öffnungshöhe des 6-poligen Steckers ist **nicht gelesen** (○), beim HRO Öffnung **8,34 × 2,56 mm** (Frontansicht) und Bauhöhe **3,26 mm** (Schnitt A-A) ✔ HRO, Abschnitt Frontansicht und Schnitt A-A (Zeichnung, Blatt 1).
6. **Nicht erfasst:** `U7`, `LED1`/`LED4`/`LED5`, `U1`, `TP1`/`TP4`–`TP8`, `JP2`/`JP3`, das Modul `U2` (V3) und die neuen Teile `SW5`, `C32`. Wo Gehäuse und Platine zusammenhängen, ist nicht belegt.
7. **Bohrdurchmesser und Padgrösse der Befestigungslöcher** werden nicht verglichen (2,7 / 5,4 mm; 2,2 / 3,8 mm); ein M2-Loch statt M2,5 besteht die Prüfung.
8. **Kollision mit der Modul-Sperrfläche** ist nirgends geprüft; sie wird erst vom DRC (`items_not_allowed`) gemeldet, und nur, wenn die Rule Area im Layout steht (§11). `H6` liegt mit drei Pads darin.
9. **Das Teil muss in V2 vorkommen:** `if ref not in a["pos"]: continue` — ein Bezeichner, der in V2 fehlt, wird **übersprungen**; neu eingeführte Teile (`SW5`) sind damit nie geprüft, entfallene Teile ebenso. Die Zahl der Merkmale in der Ausgabe („Umriss und N Positionen“) ist 13 + Umriss, wenn alle da sind (5 fest + 8 Stecker/Taster = P14: 14 Merkmale).
10. **Alle Positionen sind relativ zur oberen linken Ecke der Kanten-Linien** (`min x`, `min y` aller `Edge.Cuts`-Punkte): Wird die Aussparung wegretuschiert, ändert sich diese Ecke nicht; wird der Umriss verschoben, aber nicht verformt, bleibt alles grün (gewollt).
11. **Footprint ohne Courtyard** (23 V2-Teile) werden von KiCad nicht gegen Nachbarn geprüft; die H7-Prüfung schaut auf keine Courtyards.
12. **Abstand Bauteil–Kante** (≥ 2,5 mm laut JLCPCB) ist nirgends geprüft; V2 verletzt ihn bei 15 Bauteilen (Courtyard-Rechnung §14; die 23 Teile ohne Courtyard sind nicht bewertbar).

## 18. Fallen und Widersprüche

1. **Der V3-Entwurf rechnet „Aussparung entfällt“, V2 hat sie** (`entscheidungen.md` AE-16 gegen V2-Umriss §2). Der Architekt prüft AE-16 neu.
2. **Standardfootprint des Moduls gegen V2-Festpunkte:** Sperrfläche trifft `H6`; Pad 1/40 liegen an der Aussparungskante; `SW1` hat 0,45 mm Abstand zum Modulpad 14 (§11). Alle drei hängen an derselben Setzung (Lage des Moduls) und gehören gemeinsam entschieden.
3. **TPS2121-Datenblatt (Paste 100 %) gegen TI-QFN (50 … 70 %)** (§12): beides TI; für die freiliegende Fläche gilt, was JLCPCB aus dem Pad-Layout als Schablone ableitet (`jlcpcb-fertigung.md` §5, JLC-PRICE „Stencil: SMT engineers generate the stencil file“) und was die X-Ray-Prüfung zeigt.
4. **HDG: 2 Lagen zulässig, aber „no official two-layer modules“** (HDG S. 22): die Antennenleistung am zweilagigen Träger ist **nicht zugesichert**; Messung am fertigen Gerät (Spec F4).
5. **`entscheidungen.md` AE-16 nennt das Modul „grösser als der ESP-12F“:** In V2 ist das Courtyard von `U4` 18,1 mm breit (x 117,95 … 136,05), das Modulgehäuse 18,0 mm, die Pad-Aussenkanten des S3 aber 19,0 mm (117,5 … 136,5) gegen 17,7 mm (118,15 … 135,85) beim ESP-12F: **die Pads sind 0,65 mm je Seite breiter**, nicht das Gehäuse (○ gerechnet aus ✔ V2-PCB und KC-FP).
6. **TI-HS Tabelle A-1 gehört zu TI-Prozessoren**; ihre Zahlen (50 mil Skew, 4 Vias, 0 Testpunkte) sind übertragbare Richtwerte, **keine** Espressif-Vorgabe. Espressif nennt nur „gleich lang, 90 Ω ±10 %, wenige Vias, Masse-Vias an Übergängen“ (HDG S. 30).
7. **Wellenlöten oder von Hand** und die 2,5-mm-Kantenregel: JLCPCB-Seiten widersprechen sich, siehe `jlcpcb-fertigung.md` §12.
8. **Zahl der Wärmevias im Modul-Footprint:** `kicad-regeln-und-footprints.md` §1.7 nennt 15, meine Zählung der Datei ergibt **12** (`KC-FP`, 62 Pads: 40 Rand, 1 EPAD, 9 Paste, 12 durchkontaktiert, Bohrung 0,2 / Pad 0,6 mm). Die Abweichung ist nicht geklärt; massgebend ist die Datei.

## 19. Folgerung für unsere Platine

1. **Zwei Lagen, Masse unten als zusammenhängende Fläche, Bauteile nur oben** (ausser `U1`, Entscheid offen); `F.Cu` mit Masse füllen, Inseln entfernen.
2. **Leistungspfad 3,6 A:** ≥ 1,2 mm (ΔT 20 K, 1 oz) oder Fläche; Vias ≥ 4 je Lagenwechsel; Spannungsabfall 100 mm/2 mm = 88 mV beachten.
3. **Modul:** Mitte x = 127,0, Oberkante **mindestens 0,66 … 0,76 mm unter der Platinenkante** (Pad 1/40 an der Aussparung), Modul eventuell ≥ 0,75 mm nach +x (`SW1`); **Rule Area des Footprints verkleinern** (Abweichung dokumentieren, Entscheid Architekt) und im Projekt-Footprint ablegen; Antennenbereich frei auf allen Lagen; USB/UART/Testpunkte hinter der Linie y ≥ 15 mm (○ Setzung).
4. **USB:** `U11` ≤ 5 mm von den Pads, Paar kurz, gleich lang (≤ 1,27 mm), ≤ 2 Vias, durchgehende Masse; **keine Impedanzzusage auf 2 Lagen**.
5. **Schaltregler:** `CIN`/`D2` am IC, LX-Fläche klein, FB abseits.
6. **Feste Orte** nach §17, **neu prüfen** mit einem Zusatzskript: Aussparung, Hals, Drehung, Seite, Bohrung, Frontfläche `USB1`.
7. **JLCPCB-Regeln** aus `jlcpcb-fertigung.md` §16 in `.kicad_dru`; nach dem Layout `hw-reviewer`.

## 20. Offene Punkte

- **Gehäusezeichnung** (Öffnung `USB1`, Taster, Batteriefach, Lichtleiter): nicht im Repo; ohne sie sind Front- und Höhenlage des Steckers nicht prüfbar.
- **Wie JLCPCB mit 145 × 39 mm, überstehenden Steckern und Rückseiten-THT umgeht:** `jlcpcb-fertigung.md` §15.
- **Antennenleistung mit 0,75 mm FR-4 beidseits** und dem Stecker `H6` in 9,5 mm Abstand: nur durch Messung (RSSI) zu klären.
- **IPC-2221/IPC-2152 und IPC-7351B selbst** nicht gelesen (kostenpflichtig); Formeln über Sekundärquellen.
- **Netzzuordnung der V2-Zonen** (zehn Stück) nicht ausgewertet.
- **TI-HS Tabelle A-1** gilt für TI-Chips; **Full-Speed-Toleranz auf 2 Lagen** unbelegt.
- **Rauschen/Streuung des LDR-Pfades** (Länge ≈ 80 mm, Quellimpedanz 1 kΩ): kein Grenzwert in den Unterlagen; Messung am Gerät.
- **JLCPCB-Impedanzrechner für 2 Lagen:** nicht abrufbar (Browser nötig).
