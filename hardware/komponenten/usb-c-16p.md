# Komponentenblatt: USB-C-Buchse 16-polig (USB 2.0) — HRO TYPE-C-31-M-12 und die Typ-C-Spezifikation

Stand der Prüfung: **17.10.2026**. Verfasser: Datenblatt-Analyst. Zeichen: ✔ im Datenblatt gelesen (Seite),
● Sekundärquelle (genannt), ○ Annahme/Erinnerung oder eigene Rechnung (nur als offene Frage bzw. als „gerechnet“ gekennzeichnet).
Die Quellen liegen unter `hardware/komponenten/datenblaetter/` (nicht im Git).

## 1. Quellen, Variante, Gehäuse

| Kürzel | Unterlage | Revision | Datum | Datei |
|---|---|---|---|---|
| **SPEC** | USB Type-C Cable and Connector Specification | **Release 2.0** | August 2019 (Kopfzeile jeder Seite), 373 S. | `usb-c-spec.pdf` (usb.org). Gedruckte Seite = PDF-Seite |
| **HRO** | HRO TYPE-C-31-M-12, Zeichnung „DETECTOR SWITCHS“, 1 Blatt, Revision **A** | A | gezeichnet 2020.12.08 | `hro-type-c-31-m-12.pdf`, Kopie von LCSC C165948 (Zeichnung ist ein Bild; mit PyMuPDF gerendert und gelesen) |

- **Bauteil:** HRO (Korean Hroparts Elec) **TYPE-C-31-M-12**, USB-2.0-Typ-C-Buchse, **16 Anschlussnamen, 12 Lötflächen**, SMD, Right Angle ✔ HRO (Pin-Tabelle und Layoutzeichnung).
  Strombelastung **5 A**, Spannung **20 V**, Steck-/Ziehkraft 0,5…2,0 kgf bzw. 0,8…2,0 kgf, **10'000 Zyklen**, Betrieb **−30 … +80 °C**, IR-Reflow 10 s bei 260 °C ✔ HRO (Hinweistext rechts).
- **Gehäuse:** Zeichnungsmasse **8,94 mm breit, 7,35 mm hoch, 6,28 mm tief** (Ansicht) ✔ HRO. Rastermass der Signalflächen 0,5 mm ✔ HRO (Layoutzeichnung).
- **Das HRO-Blatt trägt keinen Titel „Datasheet“, keine elektrischen Kennwerte ausser 5 A/20 V/100 MΩ/AC 100 V** ✔ HRO. Kontaktwiderstand, Hüllenwiderstand und CC-Eigenschaften stehen dort nicht (nicht gelesen, weil nicht vorhanden).

## 2. Absolute Maximalwerte und Betriebsbedingungen

| Grösse | Wert | Beleg |
|---|---|---|
| Strom je Verbinder | 5 A | ✔ HRO |
| Spannung | 20 V | ✔ HRO |
| Isolationswiderstand | ≥ 100 MΩ | ✔ HRO |
| Durchschlagfestigkeit | AC 100 V, 1 min (so steht es auf der Zeichnung) | ✔ HRO |
| Betriebstemperatur | −30 … +80 °C | ✔ HRO |
| Lebensdauer | 10'000 Steckzyklen | ✔ HRO |
| Default-Strom eines USB-2.0-Ports | **500 mA** | ✔ SPEC S. 218 (4.6.2.1) |
| Der Anschluss muss „default USB power“ vertragen | ja, jeder Typ-C-Port | ✔ SPEC S. 217 |

## 3. Pinfunktionen (16-polige Buchse)

### 3.1 Belegung

**Tabelle der Spezifikation für USB-2.0-Buchsen** (Table 3-5, SPEC S. 69) — identisch mit der Tabelle auf der HRO-Zeichnung ✔ HRO:

| A-Seite | Signal | B-Seite | Signal |
|---|---|---|---|
| A1 | GND | B12 | GND |
| A4 | VBUS | B9 | VBUS |
| A5 | **CC1** | B8 | SBU2 |
| A6 | **Dp1** (D+) | B7 | **Dn2** (D−) |
| A7 | **Dn1** (D−) | B6 | **Dp2** (D+) |
| A8 | SBU1 | B5 | **CC2** |
| A9 | VBUS | B4 | VBUS |
| A12 | GND | B1 | GND |

Die Positionen A2, A3, A10, A11, B2, B3, B10, B11 (die Hochgeschwindigkeits-Paare) gibt es bei der 16-poligen Buchse nicht ✔ SPEC S. 69 (Hinweis 1: unbenutzte Kontakte müssen elektrisch isoliert bleiben).

### 3.2 Was Du verbinden musst — und was nicht

| Pin | Muss verbunden werden? | Beleg |
|---|---|---|
| **VBUS** (A4, A9, B4, B9) | **Ja, alle vier zusammen** auf der Platine | ✔ SPEC S. 69, Hinweis 4 |
| **GND** (A1, A12, B1, B12) | **Ja, alle vier zusammen** | ✔ SPEC S. 69, Hinweis 5 |
| **CC1 / CC2** (A5 / B5) | **Ja, je Pin ein eigener Rd (5,1 kΩ nach GND)**, **nicht zusammenschalten** | ✔ SPEC S. 312 (A.4.2: „presents Rd on both of its CC pins“) und Tab. 4-25 S. 236; die Buchse nimmt an, dass je Pin nur einer von beiden belegt ist |
| **D+** (A6 und B6) | Ja, beide Pins, **kurz brücken**, so nah wie möglich an der Buchse | ✔ SPEC S. 69, Hinweis 2: Dp1 und Dp2 dürfen auf der Geräteseite kurzgeschlossen werden, Stichlänge **≤ 3,5 mm** |
| **D−** (A7 und B7) | Ja, beide Pins, ebenso brücken, ≤ 3,5 mm | ✔ SPEC S. 69, Hinweis 2 |
| **SBU1/SBU2** (A8, B8) | **Nein, offen lassen**, ausser ein Sondermodus (z. B. Audio-Adapter) verlangt sie | ✔ SPEC S. 69, Hinweis 3 |
| **Schirm** (Gehäuse, 4 Lötlaschen) | **Ja, an GND**, mit mehreren Anschlusspunkten zur GND-Fläche | ✔ SPEC S. 135 („The receptacle shell should have sufficient connection points to the system PCB GND plane“); ○ ob über 0 Ω/Kondensator oder direkt: nicht in der Spezifikation entschieden |

Hinweis zur Reihenfolge der Lötflächen ✔ HRO (Layoutzeichnung, Reihe von links): **A1/B12 · A4/B9 · B8 · A5 · B7 · A6 · A7 · B6 · A8 · B5 · B4/A9 · B1/A12**.
Die Buchse legt GND-Paare und VBUS-Paare **innen** auf eine Fläche zusammen (zwölf statt sechzehn Flächen). Daraus folgt: D− (B7) und D+ (A6) liegen **nebeneinander**, D+ (A6) und D+ (B6) dagegen nicht — das Brücken von A6 und B6 braucht eine Durchkontaktierung oder einen kurzen Abzweig, und das gilt für D− (A7/B7) genauso. Das ist der Grund für die 3,5-mm-Grenze der Spezifikation.

### 3.3 Rd 5,1 kΩ je CC — was zeigt er der Quelle an?

- **Rd zeigt nur „hier hängt eine Senke“ an, keinen Strom.** Ein Rd von 5,1 kΩ ±20 % (oder 1,1-V-Klemme) erlaubt der Quelle zu erkennen, dass etwas angeschlossen ist ✔ SPEC Tab. 4-25, S. 236.
- **Den Strom meldet die Quelle über ihren Rp**, und die Senke liest ihn als Spannung am eigenen CC ✔ SPEC Tab. 4-24, S. 235:

| Angebot der Quelle | Rp gegen 4,75…5,5 V | Rp gegen 3,3 V ±5 % | Stromquelle 1,7…5,5 V |
|---|---|---|---|
| Default-USB-Strom (500 mA bei USB 2.0) | 56 kΩ ±20 % | 36 kΩ ±20 % | 80 µA ±20 % |
| 1,5 A @ 5 V | 22 kΩ ±5 % | 12 kΩ ±5 % | 180 µA ±8 % |
| 3,0 A @ 5 V | 10 kΩ ±5 % | 4,7 kΩ ±5 % | 330 µA ±8 % |

- **Welche Spannung die Senke daraus sieht** ✔ SPEC Tab. 4-36, S. 241: vRd-USB 0,25…0,61 V (Schwelle 0,66 V), vRd-1,5 0,70…1,16 V (Schwelle 1,23 V), vRd-3,0 1,31…2,04 V. Eine Senke, die nur Default-Strom kennt, prüft nur vRd-Connect 0,25…2,18 V ✔ SPEC Tab. 4-35, S. 241.
  Gerechnet (○, aus den ✔-Werten): 5 V über 56 kΩ + 5,1 kΩ ergibt 0,42 V, über 22 kΩ ergibt 0,94 V, über 10 kΩ ergibt 1,69 V — alle drei liegen in ihrem Band.
- **Erst ein Rd mit ±10 % kann die Strombänder auswerten**: ✔ SPEC Tab. 4-25, S. 236 („± 10 % resistor to GND, 5.1 kΩ — Can detect power capability? Yes“; mit ±20 % „No“). Der Rd von V2 ist C27834 mit **±1 %** (● JLCPCB-Abfrage, Beschreibung der Teilebibliothek), erfüllt das also.
- **Folge für die Last:** Ohne Auswertung der Bänder und ohne USB-Enumeration darf die Senke bei „Default“ **bis 500 mA** ziehen („attaches as a USB Type-C Power Sinking Device (PSD), after which the Sink may draw up to 500 mA“) ✔ SPEC S. 218. Mehr als das ist nur erlaubt, wenn die Quelle 1,5 A oder 3 A anbietet **und** die Senke es über die Spannungsbänder erkannt hat. Das gilt unabhängig von der Datenseite (D+/D−).
- Die Datenseite (D+/D−) hat mit dem Stromangebot nichts zu tun. Ob am SoC zusätzliche Widerstände nötig sind, steht im Blatt `esp32s3-wroom-1.md`, nicht hier.

## 4. Empfohlene Beschaltung

- **CC1 und CC2: je ein Widerstand 5,1 kΩ nach GND**, ±1 % empfohlen, damit die Bänder lesbar bleiben ✔ SPEC Tab. 4-25 (S. 236).
- **D+/D−:** beide Seiten nahe der Buchse brücken, Stich ≤ 3,5 mm ✔ SPEC S. 69. Anschliessend ESD-Schutz (Blatt `usblc6-2sc6.md`) **so nah wie möglich an der Buchse** ✔ ST (Blatt dort).
- **VBUS:** alle vier Pins auf der Platine verbinden ✔ SPEC S. 69; der Stecker selbst verbindet sie ebenfalls ✔ SPEC S. 69. Die Quelle kann bis 3 A liefern; die Leiterbahn gehört auf den Strom ausgelegt, den die Last wirklich zieht (Entscheid beim Architekten).
- **SBU1/SBU2:** unbeschaltet ✔ SPEC S. 69.
- **Schirm:** mehrfach an die GND-Fläche ✔ SPEC S. 135.

## 5. Kritische Daten für dieses Projekt

- **V2 trägt heute eine 6-polige Buchse** (USB1 = `TYPE-C 6P`, LCSC **C456012**, SHOU HAN, Footprint `EasyEDA:TYPE-C-SMD_TYPE-C-6P`). In der V2-Netzliste (kicad-cli-Export, gelesen) hängen an USB1 nur **GND, VBUS (`USB_5V_INPUT`) und CC1/CC2** — **keine Datenleitungen** ✔ (Netzlistenauszug). Für Flashen, Konsole und JTAG über denselben Anschluss reicht diese Buchse nicht: Sie hat gar keine D+/D−-Kontakte. Gebraucht wird eine 16-polige.
- **Was die 16-polige Buchse mehr verlangt:** zwei Datenleitungen, brückenpflichtig; dazu den ESD-Baustein; sonst bleibt die Beschaltung wie bei V2 (je CC 5,1 kΩ, VBUS und GND gebündelt).
- **Mehr als 500 mA:** nur über die CC-Auswertung (siehe 3.3). Der V2-Aufbau vermerkt „über USB-C wird es bei heller Anzeige eng“ (`HARDWARE.md`, Leistungsabschnitt); die Aussage aus der Spezifikation: ohne Bandauswertung 500 mA, mit 1,5 A/3 A-Angebot der Quelle entsprechend mehr.

## 6. Fertigung bei JLCPCB

Prüfdatum **2026-10-17**. Klasse, Lager, Gehäuse aus der Teilebibliothek von JLCPCB über die Such-Schnittstelle von `jlcpcb.com/parts` (●, Schlüsselwort = LCSC-Nummer), LCSC-Lager aus `lcsc.com/product-detail/<Nr>.html` (●).

| LCSC | Bauteil | Hersteller | Klasse | Lager JLCPCB | Lager LCSC | Hinweis |
|---|---|---|---|---|---|---|
| **C165948** | **TYPE-C-31-M-12**, 16P, SMD, Right Angle | Korean Hroparts Elec | **Extended** (kein Preferred-Flag) | **367'528** | 350'640 | gewählt: im Auftrag benannt, 5 A, 16P, riesiger Bestand. keine Basic-Buchse mit 16 Kontakten gefunden (siehe Grund unten) |
| C5337088 | TYPE-C-31-M-12A, 16P, 6 A | Korean Hroparts Elec | Extended | 3'058 | nicht abgefragt | Variante „A“ (6 A, −30…+85 °C); Fussabdruck nicht verglichen ○ |
| C456012 | TYPE-C 6P (V2-Bestand) | SHOU HAN | Extended | 580'426 | 572'900 | 6 Pins ohne Daten — für den Umbau **nicht brauchbar** |

- **Grund für Extended:** Es gibt keine Basic-Typ-C-Buchse. Gesucht wurde mit den Begriffen „Type-C 16P“ (200 von 12'221 Treffern betrachtet), „USB Type-C“ (200 von 1'450) und „TYPE-C“ (200 von 2'089): **kein** Treffer mit `componentLibraryType = base` und kein Preferred-Flag. **Das ist keine vollständige Durchsicht** (die Such-Schnittstelle liefert je Begriff nur die ersten 200); eine Basic-Buchse ausserhalb dieser Treffer ist nicht ausgeschlossen.
- **Bestückbarkeit:** SMD-Gehäuse laut Quelle; ob die vier Schirmlaschen/Fixierstifte (Zeichnung: Pegs ø0,50 mm, Positionsbohrungen ø0,60 mm) JLCPCB als THT-Anteil führt, liefert die Quelle nicht — **nicht geprüft**.
- **Die Nummer C165948 ist geprüft (JLCPCB-Abfrage, Treffer mit `componentCode`), nicht aus der Erinnerung.**

## 7. Footprint-Hinweise

Aus der HRO-Layoutzeichnung („RECOMMEND P.C.B LAYOUT“, Bauteilseite, Toleranz ±0,05 mm) ✔ HRO:

- **Signalflächen 8 × 0,30 mm breit**, Mass 0,50 mm (Raster), **Leistungsflächen 4 × 0,60 mm breit** (GND/VBUS-Paare).
- Lage der Flächengruppen: 2,50 / 3,50 / 4,80 mm (Abstände zwischen Flächen laut Masslinien, zugeordnet nach Zeichnung — beim Zeichnen des Footprints gegen die PDF-Seite prüfen).
- **Zwei Positionsbohrungen ø0,60 mm**, Abstand **5,78 mm**; **vier Schirmschlitze** (1,70 mm lang, 0,60/0,90 mm breit), äusserer Abstand **8,65 mm** (weitere Masslinien 6,40 und 5,78 mm; Zuordnung zu den Schlitzen aus der Zeichnung gelesen).
- Abstand Flächenreihe zur Leiterplattenkante: Masslinie 5,79 mm (Zuordnung aus der Zeichnung gelesen, nicht unabhängig geprüft).
- **Nicht gelesen:** Keepout, Paste-Schablone, Lötstopp. Die Zeichnung nennt sie nicht.
- Die Typ-C-Spezifikation selbst gibt **informative Referenz-Footprints** an (Bild-Titel „Reference Footprint for a USB 2.0 Type-C Through Hole Right Angle Receptacle (Informative)“ S. 54, „Single Row Right Angle“ S. 55 laut Abbildungsverzeichnis, S. 9) — das ist der Footprint einer **anderen Bauform** (THT/einreihig), kein Ersatz für die HRO-Zeichnung. Nicht gelesen.

## 8. Fallen und Errata

- **Die 6-polige Buchse der V2 hat keine Datenkontakte.** Wer sie 1:1 „mitnimmt“, hat keinen USB-Datenweg.
- **CC1 und CC2 niemals zusammenlegen.** Die Spezifikation verlangt Rd an **beiden** CC-Pins einzeln (S. 312); dass eine Verbindung beider Pins die Orientierungserkennung der Quelle stört, ist ○ (Folgerung aus dem Prinzip, nicht als Satz gelesen).
- **Dp/Dn nicht kreuzen:** A6 = Dp1, A7 = Dn1, **B6 = Dp2**, **B7 = Dn2** — die B-Seite ist in der Reihenfolge der Lötflächen **vertauscht** gegenüber der A-Seite ✔ SPEC S. 69 und HRO. Wer die Pins der Reihe nach verbindet, kreuzt D+ und D−.
- **VBUS-Pins bei Hochstrom:** alle vier verbinden und die Bahn dick auslegen; der Stecker verbindet sie intern, die Spezifikation verlangt es zusätzlich auf der Platine ✔ SPEC S. 69.
- **HRO-Variante und LCSC-Beschreibung:** die LCSC-Beschreibung nennt „16P“ — die Zeichnung zeigt zwölf Flächen. Beides ist richtig (siehe 3.2).
- **Abgleich zweier Quellen:** Die Pin-Tabelle der Spezifikation (Tab. 3-5) und die Pin-Tabelle der HRO-Zeichnung stimmen in allen 16 Zeilen überein; ein Widerspruch wurde nicht gefunden.

## 9. Folgerung für unsere Platine

1. **USB1 ersetzen:** 16-polige Buchse C165948 statt 6-poliger C456012; Footprint aus der HRO-Zeichnung.
2. **Beschaltung:** A6/B6 → D+, A7/B7 → D− (kurze Brücken, ≤ 3,5 mm), CC1/CC2 je 5,1 kΩ ±1 % (C27834) gegen GND, VBUS/GND gebündelt, SBU offen, Schirm an GND.
3. **ESD-Baustein** USBLC6-2SC6 direkt hinter der Buchse (Blatt `usblc6-2sc6.md`).
4. **Strom:** Ohne Bandauswertung 500 mA; der Architekt entscheidet, ob eine Auswertung (Spannung an CC, ±1 %-Rd erfüllt die Anforderung) gebraucht wird.

## 10. Offene Punkte

- Footprint-Einzelmasse der HRO-Zeichnung (Schlitzbreiten, Lage der Schablone): beim Zeichnen gegen die PDF prüfen; die Zeichnung ist ein Bild.
- Wie JLCPCB die Schirmlaschen/Pegs bestückt (nur SMT oder mit THT-Schritt): nicht geprüft.
- Ob CC1/CC2 einen eigenen ESD-Schutz brauchen: Die Typ-C-Spezifikation sagt dazu nichts; das ist eine Designentscheidung. Der USBLC6-2SC6 schützt nur zwei Datenleitungen plus VBUS.
- USB-IF-Revision: Release 2.0 (08/2019) gelesen; ob eine neuere veröffentlicht ist, nicht geprüft.
