# Komponentenblatt: JLCPCB-Fertigung und -Bestückung der ESP32-S3-Controllerplatine

Stand der Prüfung: **17.10.2026**. Verfasser: Datenblatt-Analyst. Alle Zahlen tragen ein Zeichen:
✔ im Dokument gelesen (Seite oder Abschnitt der Webseite), ● Sekundärquelle (genannt), ○ Annahme, Rechnung oder Erinnerung —
○ gehört nicht in die Stückliste und nicht in eine Regeldatei, sondern ist eine offene Frage.
Webseiten haben keine Seitenzahlen; der Beleg ist der **Abschnitt** der Seite (so wie die Seite ihn nennt) und das Abrufdatum.

**Wie gelesen wurde:** Die JLCPCB-Seiten wurden per HTTP abgerufen und der Klartext der Seite selbst gelesen. Eine erste Abfrage über ein
Zusammenfassungswerkzeug lieferte **abweichende Zahlen** (zum Beispiel „Annular Ring 0,18 mm“ ohne die Empfehlung 0,25 mm, „Max. Platine 1020 × 600“ statt 670 × 600 für den Regelfall);
solche Zusammenfassungen sind hier **nicht** verwendet worden. Die Seiten tragen teils kein Änderungsdatum; wo eines dasteht, steht es in der Tabelle.

## 1. Quellen und Kürzel

| Kürzel | Unterlage | Stand der Seite | Adresse (abgerufen 2026-10-17) |
|---|---|---|---|
| **JLC-CAP** | „Rigid PCB Manufacturing Capabilities“ (Abschnitte PCB Specifications, Drilling Features, Traces, Soldermask, Legend, Outline) | ohne Datum (nennt „LDI equipment upgraded in June 2025“) | `jlcpcb.com/capabilities/Capabilities` |
| **JLC-ASM** | „PCB Assembly Capabilities“ (Tabelle Economic/Standard PCBA, FAQ) | ohne Datum | `jlcpcb.com/capabilities/pcb-assembly-capabilities` |
| **JLC-TERMS** | „Terms and Conditions of JLCPCB Assembly Service“ (Abschnitte 2 Notes on DFM, 5 Disclaimer) | 2026-09-09 | `jlcpcb.com/help/article/61-Terms-and-Conditions-of-JLCPCB-Assembly-Service` |
| **JLC-RAIL** | „How to Add Edge Rails and Fiducials for PCB Assembly Orders“ | 2026-09-09 | `jlcpcb.com/help/article/How-to-add-edge-rails-fiducials-for-PCB-assembly-order` |
| **JLC-PRICE** | „PCB Assembly Cost: What Does the Price Include?“ | 2026-09-09 | `jlcpcb.com/help/article/pcb-assembly-price` |
| **JLC-FAQ** | „PCB Assembly FAQs“ | 2026-09-09 | `jlcpcb.com/help/article/PCB-assembly-faqs` |
| **JLC-BOMCPL** | „Recommendations for BOM / CPL File Preparation“; „Pick & Place File for PCB Assembly“ | 2026-09-09 | `jlcpcb.com/help/article/advice-for-bom-and-cpl-files-preparation`, `…/pick-place-file-for-pcb-assembly` |
| **JLC-KC10** | „How To Export BOM and Pick & Place Files From KiCad 10 [2026 Updated]“ (geschrieben auf KiCad 10.0.3) | 2026-09-09 | `jlcpcb.com/help/article/How-to-generate-the-BOM-and-Centroid-file-from-KiCAD` |
| **JLC-GBR** | „How to Generate Gerber and Drill Files From KiCad 9 [Updated]“ | 2026-09-24 | `jlcpcb.com/help/article/how-to-generate-gerber-and-drill-files-in-kicad-9` |
| **JLC-IMP** | „Multilayer high precision PCB’s with impedance control“ (Stackup-Liste) | ohne Datum | `jlcpcb.com/impedance` |
| **JLC-IMPG** | „User Guide to the JLCPCB Impedance Calculator“ | 2026-09-16 | `jlcpcb.com/help/article/User-Guide-to-the-JLCPCB-Impedance-Calculator` |
| **JLC-BLOG-OUT**, **JLC-BLOG-RND** | Blog „PCB Board Outline“ (veröffentlicht 2026-05-15, geändert 2026-09-09); Blog „Rounded Corners“ (2026-07-28) | siehe links | `jlcpcb.com/blog/pcb-board-outline`, `…/blog/rounded-corners-pcb-design` |
| **JLC-PART** | Teileseiten `jlcpcb.com/partdetail/C165948`, `C456012`, `C492415`, `C5451632`, `C2913198` (Feld „JLCPCB Assembly Type“, „PCBA Type“) | tagesaktuell | `jlcpcb.com/partdetail/<Nr>` |
| **FT** | Fabrication Toolkit (bennymeg), README, `plugins/process.py`, `plugins/transformations.csv`; Release **5.3.1** vom 2026-05-15 (GitHub-Schnittstelle) | siehe links | `github.com/bennymeg/Fabrication-Toolkit` (● Sekundärquelle, Quelltext gelesen) |
| **KC-FP** | KiCad-10-Standardfootprints `RF_Module:ESP32-S3-WROOM-1`, `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` (Version 20260206), Datei selbst gelesen | in `KiCad.app` | `/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints/…` (siehe `layout-regeln.md` §1) |
| **V2-PCB** | `V2.kicad_pcb`, `V2.kicad_dru`, `V2.kicad_pro`, `fabrication-toolkit-options.json`, `production/`, `jlcpcb/production_files/` (nur lesend, per Skript gelesen) | Dateien 2026-03 | `~/Documents/WordClock-USB C - STM32F411 - V2/` |
| **DS-M**, **HDG** | ESP32-S3-WROOM-1 Datasheet v1.8; ESP32-S3 Hardware Design Guidelines (master) | siehe `esp32s3-wroom-1.md` §1 | wie dort |

## 2. Welche Leiterplattenart nutzt V2? Welche braucht V3?

- **V2 ist zweilagig:** Lagenliste der Datei enthält nur `F.Cu` und `B.Cu`; 667 Leiterbahnsegmente, 339 Vias, 10 Zonen ✔ V2-PCB, Abschnitte `layers`, `segment`, `via`, `zone` (per Skript gezählt).
- **Aufbau laut V2-Datei:** Dicke 1,6 mm, Kupfer je 0,035 mm, Kern 1,51 mm FR4, εr 4,5, `copper_finish "None"`, Lötstoppmaske 0,01 mm ✔ V2-PCB, Abschnitt `setup`/`stackup`. Das ist der KiCad-Standardaufbau, **keine Angabe des Herstellers**; die Oberfläche (HASL oder ENIG) der V2-Bestellung steht in den Dateien nicht (○ nicht geprüft).
- **Bahnbreiten in V2:** 0,2032 mm (393 Segmente), 0,254 (72), 0,3048 (54), 0,508 (22), 0,762 (126); Vias 0,5032/0,3 mm (327), 1,0/0,7 (10), 0,61/0,3 (2) ✔ V2-PCB, Abschnitte `segment`/`via`. Netzklassen: Default 0,1524 mm Bahn und Abstand, `Power_1` 2,0 mm, `Power_2` 0,508 mm ✔ V2 `.kicad_pro`, Abschnitt `net_settings.classes`.
- **Die V2-Regeldatei ist eine PCBWay-Datei, keine JLCPCB-Datei:** Kopfzeile „#PCBWay Custom DRC for Kicad 7“; aktiv sind u. a. Bahn/Abstand 0,127 mm, Via 0,5/0,3, Loch-zu-Loch 0,5 mm, Kantenabstand 0,3 mm, Texthöhe min. 0,8 mm ✔ V2 `.kicad_dru`, Abschnitt Regeln `Minimum Trace Width…` und `Minimum Text`. Mehrere Werte weichen von JLCPCB ab (siehe §12 und §16).
- **Vorgabe für V3: zwei Kupferlagen, Umriss 145,00 × 39,00 mm** ✔ `specs/esp32s3-port/requirements.md`, Abschnitt Umriss (Zeile 177: „zwei Kupferlagen“). Vier Lagen sind damit **nicht** vorgesehen; §4 gibt trotzdem die Zahlen für JLC04161H-7628, falls der Nutzer die Vorgabe ändert.
- **Folgerung:** Alle Werte unten gelten für **2 Lagen, 1,6 mm, 1 oz Aussenkupfer**.

## 3. Fertigungsfähigkeiten für 2 Lagen, 1 oz

Beleg für alle Zeilen: JLC-CAP (abgerufen 2026-10-17), Abschnitt in Klammern.

| Grösse | Wert | Beleg |
|---|---|---|
| Material | FR-4 Grade A (Nan Ya, KB, Shengyi) | ✔ JLC-CAP, Abschnitt PCB Specifications |
| Dicke | 0,4 / 0,6 / 0,8 / 1,0 / 1,2 / 1,6 / 2,0 mm; Toleranz ab 1,0 mm ±10 % (bei 1,6 mm: 1,44 … 1,76 mm) | ✔ JLC-CAP, Abschnitt PCB Specifications |
| Aussenkupfer, 2 Lagen | 1 / 2 / 2,5 / 3,5 / 4,5 oz fertig | ✔ JLC-CAP, Abschnitt PCB Specifications |
| Kupferdicke im Rechner | 1 oz aussen = 1,6 mil (= 40,6 µm) | ✔ JLC-IMPG, Abschnitt Calculation Parameters |
| **Min. Bahn/Abstand 1 oz, 1–2 Lagen** | **0,10 / 0,10 mm**; Breitentoleranz ±20 % (0,10 mm → 0,08 … 0,12 mm) | ✔ JLC-CAP, Abschnitt Traces |
| Min. Bahn/Abstand 2 oz, 2 Lagen | 0,16 / 0,16 mm | ✔ JLC-CAP, Abschnitt Traces |
| Pad zu Bahn (1 oz) | min. 0,10 mm | ✔ JLC-CAP, Abschnitt Traces |
| SMD-Pad zu SMD-Pad, verschiedene Netze | 0,15 mm | ✔ JLC-CAP, Abschnitt Traces |
| Kleinstes SMD-Pad | 0,25 × 0,25 mm | ✔ JLC-CAP, Abschnitt Traces |
| Via-Loch zu Bahn / PTH zu Bahn / NPTH zu Bahn | 0,2 / **0,28 (empfohlen 0,35)** / 0,2 mm | ✔ JLC-CAP, Abschnitt Traces |
| Bohrdurchmesser, 2 Lagen | 0,15 … 6,3 mm (≥ 6,3 mm wird aus kleinerem Loch gefräst); Loch- und Passtoleranz Durchkontaktierung +0,13 / −0,08 mm | ✔ JLC-CAP, Abschnitt Drilling Features |
| Lochlage; mittlere Hülsendicke | ±0,075 mm; 18 µm | ✔ JLC-CAP, Abschnitt Drilling Features |
| **Via 2 Lagen** | Loch/Durchmesser min. **0,15/0,25 mm**; Durchmesser ≥ Loch + 0,1 mm (0,15 bevorzugt); bevorzugtes Mindestloch 0,2 mm | ✔ JLC-CAP, Abschnitt Drilling Features |
| **Mehrpreis für Vias** | Loch 0,1 oder 0,15 mm (jeder Durchmesser) sowie Loch 0,2/0,25 mm mit Durchmesser < 0,45 mm **kosten mehr**; V2-Via 0,5/0,3 liegt darüber | ✔ JLC-CAP, Abschnitt Drilling Features, Fussnote ③ |
| Wärmevias des S3-Modul-Footprints (12 Stück) | Bohrung 0,2 mm, Pad 0,6 mm: Loch 0,2 mm mit Durchmesser ≥ 0,45 mm ⇒ **ohne Mehrpreis**; Loch entspricht dem bevorzugten Minimum; Ring 0,2 mm | ✔ JLC-CAP, Abschnitt Drilling Features (Fussnote ③); Bohrung/Pad ✔ KC-FP, Abschnitt `pad` |
| Via-Loch zu Via-Loch / Pad-Loch zu Pad-Loch | 0,2 / 0,45 mm | ✔ JLC-CAP, Abschnitt Drilling Features |
| PTH für Bauteile | empfohlen ≥ 0,5 mm („damit weder Lötstopplack noch Zinn im Loch stecken bleibt“) | ✔ JLC-CAP, Abschnitt Drilling Features |
| Ringbreite PTH | ≥ 0,20 mm; 2 Lagen/1 oz: empfohlen ≥ 0,25 mm, **absolutes Minimum 0,18 mm**; 2 oz: ≥ 0,254 mm | ✔ JLC-CAP, Abschnitt Traces |
| Ringbreite NPTH-Pad | ≥ 0,45 mm empfohlen (0,2 mm Kupfer um das Loch werden für die Schutzfolie entfernt) | ✔ JLC-CAP, Abschnitt Traces |
| NPTH min. | 0,50 mm (im Mechanik- oder Keep-out-Layer zeichnen) | ✔ JLC-CAP, Abschnitt Drilling Features |
| Gefräste Löcher/Langlöcher | durchkontaktiert min. Breite 0,5 mm (2 Lagen), Länge ≥ 2 × Breite; nicht durchkontaktiert min. 1,0 mm; Toleranz ±0,2 mm | ✔ JLC-CAP, Abschnitt Drilling Features |
| Rechteckige Löcher/Schlitze ohne gerundete Ecken | nicht unterstützt | ✔ JLC-CAP, Abschnitt Drilling Features |
| Kupfer zu gefrästem Rand / zu gefrästem Schlitz | ≥ 0,2 mm | ✔ JLC-CAP, Abschnitt Outline |
| Masse-/Gitterkupfer | Gitterbreite und -abstand 0,25 mm; „Same-net track spacing 0,25 mm“ (die Seite erläutert die Zeile nicht) | ✔ JLC-CAP, Abschnitt Traces |

## 4. Impedanzkontrolle und Stackups (USB-Paar 90 Ω)

- **Kontrollierte Impedanz führt die Seite nur für 4 und mehr Lagen auf** („Controlled Impedance 4/6/8/…/32 layers“), Toleranz **±10 %** ✔ JLC-CAP, Abschnitt PCB Specifications; ±5 % nur auf Sonderanfrage ✔ JLC-CAP, Abschnitt FAQs. **Eine Impedanzkontrolle für zweilagige Platinen steht nirgends** (gesucht in JLC-CAP, JLC-IMP, JLC-IMPG). Der Rechner nimmt Werte 20 … 90 Ω (einfach) und 50 … 150 Ω (differentiell) und nur 1 oz Aussenkupfer ✔ JLC-IMPG, Abschnitt User Interface. Die Rechenparameter nennt der Leitfaden nur für „4- bis 8-lagige“ Platinen (NP-155F) ✔ JLC-IMPG, Abschnitt Calculation Parameters.
- **Dielektrizitätskonstanten:** 2 Lagen 4,5; Prepreg 7628 4,4; 3313 4,1; 1080 3,91; 2116 4,16; Kern 4,6; Lötstopplack 3,8 ✔ JLC-CAP, Abschnitt PCB Specifications; JLC-IMP, Abschnitt Parameters.
- **JLC04161H-7628 (4 Lagen, 1,6 mm, 1 oz aussen/0,5 oz innen)**, von oben: Kupfer 0,035 mm / Prepreg 7628 **0,2104 mm** / Kern 1,065 mm (Kupfer 0,0152 mm beidseitig) / Prepreg 7628 0,2104 mm / Kupfer 0,035 mm ✔ JLC-IMP, Abschnitt 4-Layer Impedance Control Stackup, Eintrag 2). Weitere Namen: …-3313 (Prepreg 0,0994 mm, Kern 1,265 mm), …-1080 (0,0764 mm; 1,265 mm), …-2116 (0,1164 mm; 1,265 mm) ✔ JLC-IMP ebd. Eintrag 3, 4, 14.
- **Rechnung für 90 Ω differentiell, Randkopplung über durchgehender Bezugsmasse** (○ eigene Näherung nach der IPC-2141-Formel Zdiff ≈ 2·Z0·(1 − 0,48·e^(−0,96·s/h)) mit Hammerstad-Z0; Fehler gegen einen Feldlöser rund ±8 %, weil Lötstopplack und Ätzprofil fehlen):

  | Aufbau | Abstand s | Breite w | Zdiff (○) |
  |---|---|---|---|
  | **2 Lagen**, h = 1,51 mm (Masse auf der Unterseite), εr 4,5 | 0,20 mm | 1,20 mm | 88 Ω |
  | 2 Lagen | 0,50 mm | 1,50 mm | 90 Ω |
  | 4 Lagen JLC04161H-7628, h = 0,2104 mm, εr 4,4 | 0,127 mm | 0,24 mm | 90 Ω |
  | 4 Lagen JLC04161H-7628 | 0,15 mm | 0,26 mm | 90 Ω |

  Gegenprobe der Näherung: Eine Suchmaschinen-Zusammenfassung nennt für 7628 mit s = 4 mil und w = 8,09 mil rund **85 Ω** aus dem JLCPCB-Rechner (○ Seite nicht selbst geöffnet); meine Näherung läge dort bei rund 92 Ω. **Die endgültige Breite kommt aus dem JLCPCB-Rechner (`jlcpcb.com/pcb-impedance-calculator`) oder einem Feldlöser, nicht aus dieser Tabelle.**
- **Folgerung:** Auf 2 Lagen sind 90 Ω nur mit ~1,2 mm breiten Leitern erreichbar (○), was zum 0,5-mm-Raster von `USB1` und zum Modul nicht passt; JLCPCB sichert die Impedanz dort nicht zu. Die Layoutregeln in `layout-regeln.md` §10 behandeln das USB-Paar deshalb über Länge, Symmetrie und durchgehende Masse. Wirkung auf die Funktion bei 12 Mbit/s: **nicht belegt** (kein Datenblatt nennt eine Toleranzgrenze für Full-Speed-Leitungen auf Platine; Espressif nennt 90 Ω ±10 % ✔ HDG S. 30).

## 5. Oberfläche, Lötstopplack, Siebdruck

| Grösse | Wert | Beleg |
|---|---|---|
| Oberflächen | HASL (bleihaltig/bleifrei), ENIG, OSP; HASL nicht für Platinen ≤ 0,4 mm, 6+ Lagen, Hochfrequenz | ✔ JLC-CAP, Abschnitt PCB Specifications |
| Oberfläche bei Bestückung | ist die Platine bleihaltig, wird bleihaltig bestückt; bei bleifreier Platine richtet sich JLCPCB nach den Bauteileigenschaften; JLCPCB stellt HASL bleihaltig bisweilen auf bleifrei um und trägt die Kosten | ✔ JLC-TERMS, Abschnitt 5 Disclaimer, Punkt 4 |
| Lötstopplack | LPI; Farben grün, violett, rot, gelb, blau, weiss, schwarz; Dicke ≥ 10 µm | ✔ JLC-CAP, Abschnitt Soldermask |
| Öffnung gegen Pad | 1:1 (Umstellung der Belichtung Juni 2025); **mindestens 0,09 mm** zwischen Lackfenster und Nachbarbahn | ✔ JLC-CAP, Abschnitt Soldermask |
| Lackbrücke zwischen Pads | min. 0,10 mm (1 oz; grün, rot, gelb, blau, violett), **0,13 mm (schwarz, weiss)**, 2 oz: 0,20 mm | ✔ JLC-CAP, Abschnitt Soldermask |
| Mit Lack gefüllte Vias | Durchmesser ≤ 0,5 mm; kein Lackfenster auf beiden Seiten; ≥ 0,35 mm zu anderen Lackfenstern | ✔ JLC-CAP, Abschnitt Soldermask, „Plugged vias“ |
| Siebdruck | Linie ≥ 0,15 mm; **Texthöhe ≥ 1,0 mm (40 mil)**; Verhältnis Breite:Höhe 1:6; **Pad zu Siebdruck ≥ 0,15 mm** | ✔ JLC-CAP, Abschnitt Legend |
| Via im Pad | JLCPCB übernimmt keine Folgen (verrutschte oder aufgerichtete Teile), weil Paste in das Via fliessen kann | ✔ JLC-TERMS, Abschnitt 5 Disclaimer, Punkt 6 |
| Bestellnummer im Siebdruck | die Seiten JLC-CAP/JLC-TERMS nennen sie nicht (nicht gefunden); ob und wo JLCPCB eine Nummer aufdruckt: **nicht geprüft** | ○ |

## 6. Umriss, Fräsen, Kantenabstand, Platinengrösse

- **Umrissgenauigkeit:** CNC-Fräsen ±0,2 mm (Regel) oder ±0,1 mm (Präzision, dann ≥ 50 × 50 mm und mindestens drei Werkzeuglöcher ≥ 1,5 mm an verschiedenen Ecken); V-Nut ±0,4 mm ✔ JLC-CAP, Abschnitt Outline.
- **Kupfer zum Rand ≥ 0,2 mm;** der Blog empfiehlt 0,3 … 0,5 mm für sichere Fertigung ✔ JLC-BLOG-OUT, Abschnitt Edge Clearance, Keep-Out Zones.
- **Innenecken** (konkav) folgen dem Fräser: kleinster Radius **R 0,5 mm** (1-mm-Fräser); als Zeichenvorgabe „≥ 0,5 … 1,0 mm“ ✔ JLC-BLOG-RND, Abschnitt CNC Routing Techniques; JLC-BLOG-OUT, Abschnitt Critical Factors. Der Umriss von V2 hat **scharfe** Innenecken (Antennenaussparung, Hals des USB-Streifens, §2 in `layout-regeln.md`); der Fräser rundet sie, Bauteile dürfen dort nicht in die Rundung ragen.
- **Mindestgrösse Platine:** 3 × 3 mm; Standard-Bestückung: Einzelplatine **70 × 70 … 460 × 500 mm**, Nutzen 70 × 70 … 250 × 250 mm; **V2/V3 misst 145 × 39 mm, die kurze Seite liegt unter 70 mm** ✔ JLC-ASM, Tabelle Economic/Standard PCBA, Zeile Dimension. Was JLCPCB damit tut (Randstreifen ergänzen, Nutzen verlangen), steht auf den abgerufenen Seiten nicht; eine Suchmaschinen-Zusammenfassung sagt „JLCPCB ergänzt Prozessränder auf beiden Seiten“ (○ Seite beim Abruf mit 404 nicht lesbar). **Offen, im Bestelldialog zu klären** (§15).
- **Nutzen (Panel):** Abstand zwischen Platinen 1,6 oder 2 mm bei Mouse-Bites; Werkzeugrand min. 3 mm, **für SMT bei JLCPCB 5 mm Werkzeugrand, 2-mm-Werkzeuglöcher und 1-mm-Fiducials, Mitte 3,85 mm vom Nutzenrand**; Mouse-Bite-Bohrung 0,5 … 0,8 mm, Abstand 0,2 … 0,3 mm; Haltesteg min. 4 mm (mit Mouse-Bites 5 mm); V-Nut-Nutzen min. 70 × 70 mm, Nut 25°, Kupfer ≥ 0,4 mm von der Nut ✔ JLC-CAP, Abschnitt Outline, „Mouse bites“ und „V-Cut“.

## 7. Bestückung (PCBA): Economic oder Standard?

| Merkmal | Economic PCBA | Standard PCBA | Beleg |
|---|---|---|---|
| Bestückseiten | **nur eine Seite** (SMT/THT) | ein- und zweiseitig | ✔ JLC-ASM, Tab. Technical Capabilities, Zeile Assembly Types |
| Einzelplatine / Nutzen | 10 × 10 … 470 × 500 mm / 10 × 10 … 250 × 250 mm | 70 × 70 … 460 × 500 mm / 70 × 70 … 250 × 250 mm | ✔ JLC-ASM, Tab. Technical Capabilities, Zeile Dimension |
| Menge | 2 … 50 Stück | 2 … 80 000 | ✔ JLC-ASM, Tab. Technical Capabilities, Zeile Order Volume |
| Lagenaufbau | nur Standardaufbau | alle | ✔ JLC-ASM, Tab. Technical Capabilities, Zeile Layer Stackup |
| **Randstreifen / Fiducials** | nicht nötig | **nötig** | ✔ JLC-ASM, Tab. Technical Capabilities, Zeilen Edge Rails, Fiducials |
| Kleinstes Bauteil / Pin-Raster | 0402 / 0,4 mm | 01005 / 0,35 mm | ✔ JLC-ASM, Tab. Technical Capabilities, Zeilen Minimum Package, Minimum IC Pin Spacing |
| **Reflow** | 255 ± 5 °C, **nicht einstellbar** | 240 ± 5 °C | ✔ JLC-ASM, Tab. Technical Capabilities, Zeile Reflow temperature |
| Bauzeit | 1 … 3 Tage | ≥ 4 Tage | ✔ JLC-ASM, Tab. Technical Capabilities, Zeile Build time |
| Oberfläche | nur nach Tabelle (1,6 mm grün: HASL oder ENIG) | frei | ✔ JLC-ASM, Tab. PCB Specs for Economic PCB Assembly |

**Was das Modul erzwingt:** Das ESP32-S3-WROOM-1-N8 (C2913198) trägt bei JLCPCB „JLCPCB Assembly Type SMT Assembly, **PCBA Type Standard Only**, MSL Level 3“ ✔ JLC-PART, Abschnitt Attribute von C2913198. Das passt zum Modul-Datenblatt: Spitzentemperatur 235 … 250 °C, einmaliger Reflow ✔ DS-M S. 47 (Abb. 12-1); Economic löte mit 255 °C. **Die Platine wird damit im Standard-Verfahren bestückt**, also: Randstreifen und Fiducials nötig, Bauteile beidseitig möglich, Rüstgebühr je Bauteilart (§10).

**Weitere Regeln der Bestückung** (JLC-TERMS, Abschnitt 2 Notes on DFM):
- Footprints und Bauteilabstände nach **IPC-7351B „medium“ oder „low density“; „high density“ wird nicht unterstützt** ✔ JLC-TERMS, Abschnitt 2, Punkte 1–2.
- **Bauteilkörper zur Platinenkante ≥ 2,5 mm** ✔ JLC-TERMS, Abschnitt 2, Punkt 3. Die Randstreifen (≥ 5 mm) gibt es, um diesen Abstand zu schaffen ✔ JLC-RAIL, Abschnitt Edge Rails. **V2 hält das nicht ein** (§ „Feste Orte“ in `layout-regeln.md`: Stecker ragen bis 6 mm über die Kante, Taster liegen 0,2 mm von ihr); wie JLCPCB damit umgeht, ist **nicht belegt** (○ offen).
- SMD-Teile prüft der Kunde in der 2D-Produktionsdatei auf Lage und Polarität, **THT-Teile in der 3D-Datei** ✔ JLC-TERMS, Abschnitt 2, Punkt 6. Wärmeausgleich und Kupferverteilung gegen Grabstein und Verzug beachten ✔ JLC-TERMS, Abschnitt 2, Punkte 4–5.
- Ein Teil, dessen BOM-Footprint nicht zum Footprint der Platine passt, wird **nicht bestückt** ✔ JLC-TERMS, Abschnitt 4, Punkt 2.
- Randstreifen: **mindestens 5 mm**, werden standardmässig mitgeliefert (Entfernen kostet extra) ✔ JLC-RAIL, Abschnitt Edge Rails.
- **Fiducials:** unbedeckte runde Kupferfläche, Durchmesser **1 mm**, Lackfenster **doppelter Durchmesser**, drei bis vier am Rand, **mindestens 3,35 mm Abstand zur Platinenkante einschliesslich Freihaltefläche** ✔ JLC-RAIL, Abschnitt Fiducials. „Werkzeuglöcher, Randstreifen und Fiducials sind Pflicht; fehlen sie, ergänzt JLCPCB sie von sich aus“ ✔ JLC-TERMS, Abschnitt 5, Punkt 5; „Fiducials müssen Sie nicht selbst anbringen“ ✔ JLC-FAQ, Frage 12.
- **X-Ray** ist für Bauteile mit verdeckten Lötstellen (BGA, QFN, leitungslose Gehäuse) **Pflicht** und wird automatisch zugeschaltet ✔ JLC-ASM, Abschnitt FAQ „Do you provide X-ray inspection“; Gebühr je Bauteilmenge (1–10 Stück 1,64 USD je Stück) ✔ JLC-PRICE, Zeile X-ray Inspection. In unserer Stückliste verdeckt: Modul `U2` (Pad unten), `U9` (VQFN-HR), `Q1`/`Q3` (PowerPAK, Drainfläche unten) — **welche JLCPCB als „leitungslos“ einstuft, ist nicht geprüft** (○).
- Bestückung von Teilen **ohne Eintrag** in beiden Dateien oder mit abweichender Schreibweise der Bezeichner: nicht bestückt („Unselected Parts“) ✔ JLC-BOMCPL, Abschnitt II.

## 8. Durchsteckteile (THT) und gemischte Bestückung

- **Zwei Aussagen widersprechen sich:** JLC-ASM, FAQ „Can you assemble THT components?“: „SMD-Teile per Reflow, **THT-Teile per Wellenlöten**; gilt für Economic und Standard.“ JLC-FAQ, Frage 2: Teile mit Bestückungsart „Wave Soldering“ in der Bibliothek werden **von Hand** bestückt, Kosten **3,5 USD Handlöt-Arbeit + 0,0173 USD je Lötstelle, ein Tag mehr Bauzeit**. JLC-PRICE (neuer): „Manual Assembly 0,0164 USD je Lötstelle“ und „Hand-soldering Labor 3,58 USD je Bestellung“ ✔ JLC-ASM, Abschnitt FAQ; JLC-FAQ, Frage 2; JLC-PRICE, Tab. Fee Item.
- **Unbenutzte Durchkontaktierungen** bleiben beim Wellenlöten frei (werden geschützt) ✔ JLC-ASM, Abschnitt FAQ „What happens to unused through-holes“.
- **Bestückungsart unserer Teile (JLC-PART, „JLCPCB Assembly Type“):** `H7` PZ254R-11-07P **Wave Soldering**, Economic und Standard ✔ JLC-PART, Abschnitt Attribute von C492415; `U1` DS18B20 (C5451632) **Wave Soldering** ✔ JLC-PART, Abschnitt Attribute von C5451632; `USB1` HRO TYPE-C-31-M-12 (C165948) **SMT Assembly**, Economic und Standard, MSL 1 ✔ JLC-PART, Abschnitt Attribute von C165948; 6-poliger V2-Stecker C456012 **SMT Assembly** ✔ JLC-PART, Abschnitt Attribute von C456012. `H4`/`H5`/`H6` (C492411/C492410/C492412): wie `H7` zu erwarten, **nicht einzeln geprüft** (○).
- **`U1` liegt in V2 auf der Unterseite** (`B.Cu`, CPL-Zeile `bottom`) ✔ V2-PCB, Abschnitt `footprint` U1; V2-CPL `jlcpcb/production_files`, Zeile U1. Ein Durchsteckteil auf der Rückseite einer sonst einseitig bestückten Platine: ob JLCPCB das als zweite Bestückseite (Standard: Rüstgebühr 51,12 statt 25,56 USD, Schablone 16,42 statt 8,21 USD ✔ JLC-PRICE, Zeilen Setup fee, Stencil) rechnet, ist **nicht belegt** (○, im Bestelldialog klären).
- Die vier Schirmstifte von `USB1` (SMT-Teil mit Stiften in plattierten Langlöchern) werden in der Quelle nicht gesondert aufgeführt; ob sie mitgelötet werden, steht nirgends (○, offen).

## 9. Dateien: Gerber, Bohrdaten, Stückliste, Bestückungsdaten

**Gerber und Bohrdaten aus KiCad (JLC-GBR, Abschnitt „Requirements“):**
✔ Format RS-274X, Protel-Dateiendungen, Koordinatenformat 4.6 in mm; Lagen: `F.Cu`, `B.Cu`, `F.Mask`, `B.Mask`, `F.Silkscreen`, `B.Silkscreen`, **`F.Paste`/`B.Paste` (für Schablone und Bestückung)**, `Edge.Cuts` als **eine geschlossene Kontur** (ohne sie hält JLCPCB die Bestellung an); Bohrdaten Excellon, mm, Ursprung **absolut**, Nullen Dezimalformat, „PTH und NPTH in einer Datei“ **nicht** ankreuzen, „alternativer Bohrmodus für Langlöcher“ ankreuzen; Zonen vor dem Plotten neu füllen; „Include netlist attributes“ und „extended X2“ ankreuzen; DRC ohne Fehler vor dem Plotten ✔ JLC-GBR, Abschnitte Requirements, Step 3–5. Ein einziges `.zip` mit englischen Dateinamen ✔ JLC-GBR, Tabelle Upload format.

**Fabrication Toolkit (empfohlen) — Ausgabe wie in V2:** JLCPCB nennt das Plugin „the recommended path for JLCPCB orders“; es erzeugt Gerber-ZIP, `bom.csv` (Spalten **Designator, Footprint, Quantity, Value, LCSC Part #**), `positions.csv`, Designatorenliste und IPC-Netzliste; „Apply automatic component translations“ angekreuzt lassen ✔ JLC-KC10, Abschnitt Method 2, Schritte 5–6. V2 liegt genau so vor ✔ V2-PCB, `production/` (Dateien `bom.csv`, `designators.csv`, `netlist.ipc`, `positions.csv`, ZIP). Optionen der V2-Ausgabe: `ALL_ACTIVE_LAYERS true`, `EXTEND_EDGE_CUT false`, `ALTERNATIVE_EDGE_CUT false`, `AUTO TRANSLATE true`, `AUTO FILL true`, `EXCLUDE DNP false` ✔ V2 `fabrication-toolkit-options.json`.

**Bestückungsdaten (CPL) — Regeln von JLCPCB:**
- Spalten **Designator, Mid X, Mid Y, Rotation, Layer**; Einheit mm; **Drehung positiv gegen den Uhrzeigersinn**; Layer „Top/Bottom“ (oder T/B), **beide Seiten in einer Datei** ✔ JLC-BOMCPL, Abschnitte VII–VIII; JLC-BOMCPL (Pick & Place), Abschnitt Pick & Place File.
- **Bezeichner in BOM und CPL gleich geschrieben (Gross-/Kleinschreibung)**, je Stücklistenzeile höchstens **200 Bezeichner**, keine doppelten Bezeichner, Mindestabstand zweier Bezeichner in der CPL **≥ 0,2 mm** ✔ JLC-BOMCPL, Abschnitte II, III, V, VII.
- Stückliste mindestens **Comment, Designator, Footprint, JLCPCB/LCSC Part #** ✔ JLC-KC10, Abschnitt The two files; die Fabrication-Toolkit-Spalten (siehe oben) erkennt JLCPCB ohne Umbenennen ✔ JLC-KC10, Abschnitt The generated BOM.

**Ursprung, Y-Achse, Mitte — so macht es die V2-Ausgabe (Skript-Vergleich):**
- Plotten und Positionen laufen über den **Hilfsursprung** `aux_axis_origin (81.5 115.75)` = **linke untere Ecke des Umrisses** ✔ V2-PCB, Abschnitt `setup`; das Plugin rechnet `Mid X = x − Hilfsursprung.x`, `Mid Y = −(y − Hilfsursprung.y)` (Y nach oben) ● FT, `plugins/process.py` Zeilen 231–232. **Gegenprobe an V2:** `USB1` KiCad-y 35,38 mm vom oberen Rand ⇒ CPL-Y 3,62 = 39,0 − 35,38 ✔ V2 `production/positions.csv`, Zeile USB1. **Die Kopie von V2 muss den Hilfsursprung behalten**, sonst verschieben sich alle Bestückungspositionen.
- „Mid“ ist standardmässig die **Mitte des Pad-Rechtecks**, nicht der Footprint-Ursprung (`FT Origin` = `Anchor` oder `Center`) ● FT, README Abschnitt ⑤ und `process.py` Zeilen 146–157 (Beispiel: `U1` TO-92 KiCad-Ursprung x = 71,23, CPL-Mitte 72,50 = Mitte der drei Pads ✔ V2-CPL, Zeile U1).
- Rückseite: `Rotation = 180 − Rotation` ● FT, `process.py` Zeilen 252–254.

**Drehkorrekturen (KiCad → JLCPCB):**
- ● FT liest `plugins/transformations.csv` (50 Einträge, regulärer Ausdruck auf den Footprint-Namen, **Gross-/Kleinschreibung beachtet**, `re.search`); darin u. a. `^SOT-23` +180°, `^SOIC-8_` +270°, `^QFN-` +90°, `^SOP-` +270°, `^PowerPAK_SO-8_Single` +270°, `^R_Array_…` +90°. **Nicht enthalten:** `RF_Module:ESP32-S3-WROOM-1`, `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12`, `Package_TO_SOT_THT:TO-92_Inline`, die EasyEDA-Footprints. Einzelfälle: Feld **`FT Rotation Offset`** (Grad, positiv = gegen den Uhrzeigersinn) und **`FT Position Offset`** (x,y in mm) am Symbol; Rückfallnamen `Rotation Offset`, `RotOffset`, `Position Offset`, `PosOffset`; `FT Layer Override` = `top`/`bottom` ● FT, README Abschnitte ③–⑥. Ein Pfeil-Klick in der JLCPCB-Vorschau = 0,0635 mm ● FT, README Abschnitt ③.
- **V2 enthält zwei Bestückungsdatensätze, und sie widersprechen sich in sechs Drehungen** (92 Zeilen je Datei, per Skript verglichen ✔ V2 `production/positions.csv` (Fabrication Toolkit) gegen `jlcpcb/production_files/CPL-…V2.csv`, Abschnitt Spalte Rotation; letztere mit Spaltenkopf `Designator,Val,Package,Mid X,Mid Y,Rotation,Layer` und daneben `jlcpcb/project.db`, also Ausgabe eines anderen Plugins, vermutlich „kicad-jlcpcb-tools“ ○; beide Dateien vom 2026-03-23). Die Drehung gegen die KiCad-Drehung `at` (Abschnitt `footprint` der V2-Datei):

  | Bezeichner | Footprint | KiCad `at` | Fabrication Toolkit (`production/`) | `jlcpcb/production_files` |
  |---|---|---|---|---|
  | `Q1` | POWERPAK_SO-8 (Snapeda) | 90° | **90°** | **0°** |
  | `Q3` | POWERPAK_SO-8 | 180° | **180°** | **90°** |
  | `U2` | QFN-48 (STM32, entfällt in V3) | 0° | **90°** | **270°** |
  | `U3` | EasyEDA SOT-23-5 | 0° | **180°** | **0°** |
  | `U5` | SOIC127P600X175-8N (DS3231) | 90° | **90°** | **0°** |
  | `U10` | EasyEDA SOT-23-6 | 90° | **270°** | **90°** |

  Alle übrigen 86 Zeilen stimmen in Drehung und Seite überein (darunter `Q2` SOT-23 180° → 0° und `U6` SOP-8 0° → 270° in beiden Dateien, `U1` unten 180°); die Mitten weichen nur bei `U4` (4,27 mm in Y, anderer Bezugspunkt) und um höchstens 0,14 mm bei `Q1`, `Q3`, `USB1`, `SW4` ab. **Welcher Satz bestellt und wie in der JLCPCB-Vorschau korrigiert wurde, steht in den Dateien nicht.** Damit sind die Drehkorrekturen für `Q1`, `Q3`, `U5`, `U3`, `U10` **nicht belegt**: Das Fabrication Toolkit dreht die EasyEDA-Footprints `U3`/`U10` über die Regel `^SOT-23` um 180°, der andere Satz nicht (○ ob eine der beiden Fassungen falsch bestückt hätte; der Nutzer kann es sagen). Siehe auch `kicad-regeln-und-footprints.md` §3.7 (dort nur der Satz `production/`).
- **Für V3 zu prüfen** (in der Vorschau des Bestelldialogs „Confirm Parts Placement“, 0,45 USD ✔ JLC-PRICE, Tab. Fee Item, Zeile Confirm Parts Placement): jedes gepolte oder gerichtete Teil, besonders `U2` Modul (kein Datenbankeintrag), `USB1` (neuer Footprint, Öffnung zur Kante), `U9` (VQFN-HR), `U11` (SOT-23-6; `^SOT-23` dreht +180°), `Q1`/`Q3`, `U5`, `U3`, `U10`, `D2`/`D3` (Kathode).

## 10. Teileklassen, Gebühren, Zahl der Zeilen

- **Gebühren Standard PCBA** (Seite vom 2026-09-09): Rüstgebühr **25,56 USD (einseitig) / 51,12 USD (beidseitig)**; Schablone **8,21 / 16,42 USD**; Reflow-Vorlöten 0,016 USD je Lötstelle; SMT 0,0016 USD je Lötstelle (bis 50 000); Handmontage 0,0164 USD je Lötstelle; Handlöten 3,58 USD je Bestellung; **Feeder Loading 1,53 USD für Basic und Extended**; Packung 0,50 USD + 0,00003 USD × cm²; Bestätigung der Bestückungsposition 0,45 USD ✔ JLC-PRICE, Tab. Fee Item. **Economic:** Rüstgebühr 8,18 USD, Schablone 1,53 USD, **Extended 3,07 USD je Bauteilart**, Basic frei ✔ JLC-PRICE ebd.; Erklärung: Beim Standard-Verfahren brauchen **alle** Basic- und Extended-Teile Bandzuführung, beim Economic-Verfahren nur Extended ✔ JLC-PRICE, Abschnitt Fee Details and Explanations.
- **Basic / Preferred Extended / Extended:** Basic = auf den Bestückern dauerhaft geladen (698 Arten, keine Arbeit); Extended = Zuführung von Hand, **3 USD je Teil**; Preferred Extended ist **nur beim Economic-Verfahren** von der Zuführgebühr befreit ✔ JLC-FAQ, Fragen 5–7. Für uns (Standard) gilt: **jede verschiedene Teilenummer kostet 1,53 USD, gleich ob Basic oder Extended** — die Wahl Basic gegen Extended spart im Standard-Verfahren **keine** Rüstgebühr, sondern nur Bestand und Verfügbarkeit (○ Folgerung aus den beiden Seiten).
- **Unsere Arbeitsstückliste** (`hardware/esp32s3/bom-arbeit.csv`, Stand 2026-10-17): **46 Zeilen**, davon **16 Basic und 30 Extended**, 86 Bauteile ✔ Datei `bom-arbeit.csv`, Abschnitt Klasse (per Skript gezählt). **Rechnung ○:** 46 Teilearten × 1,53 USD = 70,38 USD Zuführgebühr zuzüglich 25,56 USD Rüstung und 8,21 USD Schablone (einseitige Bestückung); dazu rund 19 THT-Lötstellen (`H4` 3, `H5` 2, `H6` 4, `H7` 7, `U1` 3) × 0,0164 USD und 3,58 USD Handlöten. Der Preis hängt an der Annahme „je Teilenummer“; **die Seite sagt nicht, ob je Teilenummer oder je Bestückungsstelle abgerechnet wird** (○).

## 11. Fertigung bei JLCPCB — Zusammenfassung für diese Platine (Prüfdatum 2026-10-17)

| Frage | Antwort | Beleg |
|---|---|---|
| Leiterplattenart | 2 Lagen, 1,6 mm, 1 oz aussen, Umriss 145 × 39 mm mit Antennenaussparung und USB-Streifen | ✔ Abschnitt 2 dieses Blatts |
| Verfahren | **Standard PCBA** (Modul „Standard Only“) | ✔ JLC-PART, Abschnitt Attribute von C2913198 |
| Randstreifen/Fiducials | nötig; Platine liegt mit 39 mm unter dem Standard-Minimum von 70 mm | ✔ Abschnitt 6 und 7 dieses Blatts |
| Impedanzkontrolle | für 2 Lagen **nicht angeboten**; USB-Paar ohne Zusicherung | ✔ Abschnitt 4 dieses Blatts |
| Kleinste Bahn/Abstand | 0,10 / 0,10 mm | ✔ Abschnitt 3 dieses Blatts |
| Via | ab 0,3 mm Loch ohne Mehrpreis; V2-Via 0,5/0,3 mm | ✔ Abschnitt 3 dieses Blatts |
| Siebdruck | Texthöhe ≥ 1,0 mm, Linie ≥ 0,15 mm (V2-Regel 0,8 mm ist zu klein) | ✔ Abschnitt 5 und 12 dieses Blatts |
| Bestückseite | alles SMD oben; THT `H4`–`H7`, `U1` als „Wave Soldering“ | ✔ Abschnitt 8 dieses Blatts |
| Dateien | Gerber-ZIP, Bohrdaten, `bom.csv`, `positions.csv` aus dem Fabrication Toolkit, Hilfsursprung beibehalten | ✔ Abschnitt 9 dieses Blatts |
| Stückliste | 46 Zeilen: 16 Basic, 30 Extended; jeder Extended-Eintrag hat seinen Grund im Entscheidungsprotokoll | ✔ Abschnitt 10 dieses Blatts |

## 12. Widersprüche und Auffälligkeiten

1. **THT: Wellenlöten oder von Hand?** JLC-ASM (FAQ) sagt Wellenlöten, JLC-FAQ (Frage 2) sagt „von Hand bestückt“ mit 3,5 USD + 0,0173 USD/Stelle; JLC-PRICE nennt 3,58 USD und 0,0164 USD/Stelle. Die Preise stammen aus verschiedenen Ständen (alle Seiten tragen „2026-09-09“).
2. **Rüstgebühr:** JLC-FAQ nennt „Setup fee 8,00 USD, Stencil 1,50 USD, Extended 3 USD“ (Economic-Zahlen, ohne Hinweis); JLC-PRICE trennt Economic (8,18 / 1,53 / 3,07 USD) und Standard (25,56 / 8,21 / Feeder 1,53 USD).
3. **Fiducial-Lage:** JLC-CAP „Fiducial 1 mm, Mitte 3,85 mm vom Nutzenrand, 5-mm-Werkzeugrand“ gegen JLC-RAIL „mindestens 3,35 mm Abstand zur Kante einschliesslich Freihaltefläche, Lackfenster doppelter Durchmesser“. Bei Lackfenster Ø 2 mm und Mitte 3,85 mm wäre der Rand der Freihaltefläche 2,85 mm von der Kante — **unter** 3,35 mm. Die Seiten erklären nicht, ob 3,35 mm bis zur Fiducial-Mitte gelten.
4. **Fiducials selbst setzen oder nicht:** JLC-FAQ Frage 12 „nein, wir ergänzen sie“; JLC-RAIL „für Standard-Bestückung erforderlich“; JLC-TERMS „Pflicht, fehlend werden sie ergänzt“. Gemeinsame Lesart: erforderlich, JLCPCB ergänzt, wenn sie fehlen (○ Lesart).
5. **Impedanz:** JLC-CAP nennt kontrollierte Impedanz erst ab 4 Lagen; JLC-IMPG spricht von „Total number of copper layers“ ohne Untergrenze und rechnet nur 4 … 8 Lagen. Für 2 Lagen **keine** Zusicherung belegt.
6. **Ringbreite:** JLC-CAP nennt „PTH annular ring ≥ 0,20 mm“ und, im Kleingedruckten, **absolutes Minimum 0,18 mm** (2 Lagen, 1 oz) bei **Empfehlung 0,25 mm**; für Vias verlangt die Seite nur „Via-Durchmesser ≥ Loch + 0,1 mm“, also 0,05 mm Ring. Vias und Bauteil-Pads folgen verschiedenen Regeln.
7. **Bauteil zu Kante:** JLC-TERMS ≥ 2,5 mm; eine Sekundärquelle nennt 25 mil (0,64 mm) für kleine Bauteile ● Sierra Circuits, „PCB Design for Assembly Checklist“, Abschnitt 5.4 — gilt nicht für JLCPCB.
8. **V2-Regeln gegen JLCPCB:** Texthöhe 0,8 mm (V2) gegen 1,0 mm (JLC-CAP); Kantenabstand 0,3 mm (V2, auch Empfehlung JLC-BLOG-OUT) gegen 0,2 mm (JLC-CAP); Bahn/Abstand 0,127 mm (V2) gegen 0,10 mm (JLC-CAP); Loch-zu-Loch 0,5 mm (V2, PCBWay) gegen 0,45 mm (JLC-CAP); Via 0,5 mm (V2) gegen 0,25 mm (JLC-CAP).
9. **Zusammenfassungswerkzeug gegen Seitentext:** eine erste Zusammenfassung der JLC-CAP-Seite nannte „PTH Annular Ring 1 oz minimum 0,18 mm (0,25 mm empfohlen)“ richtig, „Max. FR4 2 Lagen 1020 × 600 mm“ unvollständig (Regelfall 670 × 600 mm, bis 1020 × 600 mm möglich) — Zusammenfassungen taugen hier nicht als Beleg.

## 13. Fallen

1. **Standard statt Economic** — wer die Preise des Economic-Verfahrens kalkuliert (Rüstung 8,18 USD), liegt um den Faktor 3 zu tief; das Modul lässt Economic nicht zu.
2. **Platine kürzer als 70 mm** — im Standard-Verfahren nicht als Einzelplatine bestückbar; ohne Randstreifen/Nutzen bricht der Bestelldialog ab oder JLCPCB ergänzt Ränder, die Bauteile an der Kante (Stecker, `SW4`-Platz) berühren können.
3. **Hilfsursprung** der V2-Datei gehört zur Ausgabe (`aux_axis_origin`); eine neue Datei ohne ihn liefert CPL-Positionen relativ zur Seitenmitte.
4. **Via 0,5/0,3 mm im Pad:** JLCPCB haftet nicht für Folgen (§5).
5. **THT auf der Rückseite** (`U1`): Bestückungsweg ungeklärt (§8).
6. **Texthöhe 0,8 mm** aus der PCBWay-Regeldatei: JLCPCB nennt Zeichen unter 1,0 mm „unidentifiable“ (✔ JLC-CAP, Abschnitt Legend).
7. **KiCad-Plugin-Version:** Stand 5.3.1 (2026-05-15, ● GitHub-Schnittstelle); die V2-Ausgabe entstand mit einem älteren Stand — Positionen und Drehungen in der Vorschau prüfen, nicht annehmen.

## 14. Folgerung für unsere Platine

1. **2 Lagen, 1,6 mm, 1 oz, Standard-PCBA.** Die Regelwerte für `.kicad_dru` stehen in §16; sie sind **JLCPCB-Werte, nicht die der V2-Datei** (PCBWay).
2. Vor der Bestellung im Dialog klären: Randstreifen/Nutzen bei 39 mm Höhe, Rückseiten-THT `U1`, Verhalten bei überstehenden Steckern (`H4`–`H7`, `USB1`), X-Ray-Teile (§15).
3. **Stückliste und Bestückung:** Bezeichner in `bom.csv` und `positions.csv` identisch; Hilfsursprung beibehalten; `U2`, `USB1` in der JLCPCB-Vorschau auf Drehung und Lage prüfen.
4. **Impedanz:** nicht bestellbar; USB-Paar nach `layout-regeln.md` §10.
5. **Siebdruck** ≥ 1,0 mm Texthöhe, Pad-zu-Siebdruck ≥ 0,15 mm; Texte nicht auf Pads.

## 15. Offene Punkte

- **Was macht JLCPCB mit 145 × 39 mm im Standard-PCBA** (Randstreifen ergänzen? welche Seiten? Nutzen verlangt?) — Seite beim Abruf nicht lesbar (404 bei `jlcpcb.com/quote/pcbOrderFaq/Panel by JLCPCB`); im Bestelldialog oder beim Support klären.
- **Bauteile über der Platinenkante** (Stecker, `USB1`) und die 2,5-mm-Regel: nicht belegt, wie JLCPCB es handhabt; war V2 so bestellt und bestückt? (Dateien von V2 zeigen Bestückungsdaten, aber keinen Auftrag).
- **Oberfläche der V2-Bestellung** (HASL/ENIG): nicht in den Dateien.
- **Wave-Soldering-Teile:** Ablauf (Hand oder Welle) und ob `H4`/`H5`/`H6` ebenso eingestuft sind.
- **Rückseiten-THT `U1`:** Abrechnung als zweite Bestückseite?
- **JLCPCB-Rechner für ein 2-Lagen-Paar:** liefert er Werte für 2 Lagen? (Seite braucht Browser; hier nicht abrufbar).
- **Bestellnummer im Siebdruck:** Position und Entfernen-Option nicht gelesen.
- **Preise** gelten am 2026-09-09; bei Bestellung erneut prüfen (H4 verlangt die Teileprüfung ≤ 14 Tage).

## 16. DRC-Regelwerte für die `.kicad_dru` (JLCPCB, 2 Lagen, 1 oz)

Spalten: **Grenze** = harter Wert von JLCPCB; **Projektwert** = Setzung (○) mit Reserve, die der Layouter ändern darf. KiCad-Beschränkungen heissen laut KiCad-10-Dokumentation (Abschnitt Custom Design Rules, Constraint types): `track_width`, `clearance`, `hole_size`, `hole_clearance`, `hole_to_hole`, `annular_width`, `edge_clearance`, `silk_clearance`, `text_height`, `text_thickness`, `courtyard_clearance`, `physical_clearance`, `diff_pair_gap`, `diff_pair_uncoupled`, `skew`, `via_count`, `length`, `track_angle`, `disallow`, `solder_mask_expansion`, `bridged_mask`.

| Regel | KiCad-Beschränkung und Bedingung | Grenze (JLCPCB) | Projektwert (○) | Beleg | Zeichen |
|---|---|---|---|---|---|
| Bahnbreite min. | `track_width (min …)` Bahn | 0,10 mm (±20 %) | **0,15 mm** (Default 0,1524 wie V2) | JLC-CAP, Abschnitt Traces | ✔ Grenze, ○ Projektwert |
| Abstand Kupfer–Kupfer | `clearance (min …)` | 0,10 mm | **0,15 mm** | JLC-CAP, Abschnitt Traces | ✔ / ○ |
| Pad zu Bahn | `clearance` mit `A.Type=='pad' && B.Type=='track'` | 0,10 mm | 0,15 mm | JLC-CAP, Abschnitt Traces | ✔ / ○ |
| SMD-Pad zu SMD-Pad, andere Netze | `clearance` mit `A.Type=='pad' && B.Type=='pad'` | **0,15 mm** | 0,15 mm | JLC-CAP, Abschnitt Traces | ✔ |
| Kleinstes SMD-Pad | `assertion` auf Pad-Grösse | 0,25 × 0,25 mm | – | JLC-CAP, Abschnitt Traces | ✔ |
| Via-Durchmesser | `via_diameter (min …)` Via | 0,25 mm | **0,5 mm** (V2); ab Loch 0,3 mm ohne Mehrpreis | JLC-CAP, Abschnitt Drilling Features | ✔ / ○ |
| Via-Loch | `hole_size (min …)` Via | 0,15 mm; bevorzugt 0,2 mm; ohne Mehrpreis ab 0,3 mm | **0,3 mm** | JLC-CAP, Abschnitt Drilling Features, Fussnote ③ | ✔ / ○ |
| Via-Ring | Via-Durchmesser − Loch | ≥ 0,1 mm (bevorzugt 0,15 mm), also Ring ≥ 0,05 mm | Ring 0,10 mm (0,5/0,3) | JLC-CAP, Abschnitt Drilling Features | ✔ / ○ |
| Bauteilloch (PTH) | `hole_size (min 0.5mm)` Pad plated | 0,15 mm absolut; empfohlen ≥ 0,5 mm | 0,5 mm | JLC-CAP, Abschnitt Drilling Features | ✔ / ○ |
| Ring PTH-Pad | `annular_width (min …)` Pad plated | **0,18 mm absolut (2 Lagen, 1 oz); empfohlen 0,25 mm** | **0,25 mm** (wie V2) | JLC-CAP, Abschnitt Traces | ✔ / ○ |
| NPTH | `hole_size (min 0.5mm)` Pad nicht plated; Ring ≥ 0,45 mm empfohlen | 0,5 mm | 0,5 mm | JLC-CAP, Abschnitte Drilling Features, Traces | ✔ |
| Langloch plattiert | `hole_size` Langloch plated | Breite ≥ 0,5 mm, Länge ≥ 2 × Breite | – | JLC-CAP, Abschnitt Drilling Features | ✔ |
| Langloch nicht plattiert | `hole_size` Langloch !plated | Breite ≥ 1,0 mm | – | JLC-CAP, Abschnitt Drilling Features | ✔ |
| Bohrer max. | `hole_size (max 6.3mm)` | 6,3 mm | – | JLC-CAP, Abschnitt Drilling Features | ✔ |
| Loch zu Loch (Via / Pad) | `hole_to_hole` | 0,2 mm (Vias) / 0,45 mm (Pads) | 0,25 / 0,5 mm | JLC-CAP, Abschnitt Drilling Features | ✔ / ○ |
| Loch zu Bahn | `hole_clearance` | Via 0,2; **PTH 0,28 (empfohlen 0,35)**; NPTH 0,2 mm | Via 0,25; PTH 0,35; NPTH 0,25 | JLC-CAP, Abschnitt Traces | ✔ / ○ |
| Kupfer zum Rand | `edge_clearance (min …)` | 0,2 mm | **0,3 mm** (V2; Blog empfiehlt 0,3 … 0,5) | JLC-CAP, Abschnitt Outline; JLC-BLOG-OUT | ✔ / ○ |
| Siebdruck-Text | `text_height (min 1.0mm)`, `text_thickness (min 0.15mm)` auf Silk | **1,0 mm**, Linie 0,15 mm | 1,0 mm / 0,15 mm (V2-Regel 0,8 ist falsch) | JLC-CAP, Abschnitt Legend | ✔ |
| Pad zu Siebdruck | `silk_clearance (min 0.15mm)` | 0,15 mm | 0,15 mm | JLC-CAP, Abschnitt Legend | ✔ |
| Lötstopplack-Aufweitung | `solder_mask_expansion` / Board `pad_to_mask_clearance` | 1:1 (0 mm) | 0 mm (V2 hat 0) | JLC-CAP, Abschnitt Soldermask | ✔ |
| Lackbrücke | Board „Solder mask minimum web width“; `bridged_mask` | 0,10 mm (schwarz/weiss 0,13 mm) | 0,10 mm | JLC-CAP, Abschnitt Soldermask | ✔ |
| Lackfenster zu Nachbarbahn | Board „Solder mask to copper clearance“ | ≥ 0,09 mm | 0,09 mm | JLC-CAP, Abschnitt Soldermask | ✔ |
| Bauteilkörper zur Kante | `physical_clearance (min 2.5mm)` Courtyard gegen `Edge.Cuts`, **Schweregrad Warnung** | 2,5 mm (Montage) | Warnung; Ausnahmen: Stecker, `USB1`, Antennenaussparung | JLC-TERMS, Abschnitt 2, Punkt 3 | ✔ Grenze, ○ Syntax (nicht ausgeführt) |
| Courtyard-Abstand | `courtyard_clearance (min 0mm)` | IPC-7351B „medium/low density“ | 0 mm | JLC-TERMS, Abschnitt 2, Punkte 1–2 | ✔ |
| USB-Paar Längenunterschied | `skew (max 1.27mm) (within_diff_pairs)` auf `USB_D*` | – (kein JLCPCB-Wert) | 1,27 mm (50 mil) | TI SPRAAR7J S. 16, Tabelle A-1 | ● |
| USB-Paar Vias | `via_count (max 4)` auf `USB_D*` | – | 4 (besser 0 … 2) | TI SPRAAR7J S. 16, Tabelle A-1 | ● |
| USB-Paar Länge | `length (max 305mm)` | – | ≤ 305 mm (12 000 mil) | TI SPRAAR7J S. 16, Tabelle A-1 | ● |
| USB-Paar Abstand zu Fremdsignal | `clearance` mit `A.inDiffPair('USB_D*')` | – | ≥ 0,76 mm (30 mil); zu Takt 1,27 mm (50 mil) | TI SPRAAR7J S. 16, Tabelle A-1 | ● |
| Antennen-Freihaltung | `disallow track via pad zone footprint` im Regelbereich (Rule Area) | – | siehe `layout-regeln.md` §11 | DS-M S. 45; HDG S. 31 | ✔ |
| Mindestleiterbreite Leistung (3,6 A) | Netzklasse `Power`, `track_width (min …)` | – | **≥ 1,2 mm** bei 20 K, 1 oz (○ Rechnung `layout-regeln.md` §5) | IPC-2221-Formel ● | ○ |
| V2-Regel „Pad Size“ (Bohrung ≥ 0,5 mm, Ring ≥ 0,25 mm für **alle** plattierten Pads) | `condition` um `&& A.Fabrication_Property != 'Heatsink pad'` ergänzen | – | Falschalarm an den 12 Wärmevias des Moduls (Bohrung 0,2 mm ist bei JLCPCB zulässig, §3); gilt nicht für Schirmpads von `USB1` und TO-92 `U1` (Ring 0,2 / 0,15 mm < 0,25 mm; JLC-Grenze 0,18 mm absolut) | `kicad-regeln-und-footprints.md` §1.7 (Versuch); JLC-CAP, Abschnitt Traces | ○ |
