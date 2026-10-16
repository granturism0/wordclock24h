# Komponentenblatt: USBLC6-2SC6 (ST) — ESD-Schutz für USB 2.0

Stand der Prüfung: **17.10.2026**. Verfasser: Datenblatt-Analyst. Zeichen: ✔ im Datenblatt gelesen (Seite),
● Sekundärquelle (genannt), ○ Annahme/Erinnerung oder eigene Rechnung.
Die Quellen liegen unter `hardware/komponenten/datenblaetter/` (nicht im Git).

## 1. Quelle, Variante, Gehäuse

| Unterlage | Revision | Datum | Datei |
|---|---|---|---|
| ST **USBLC6-2** „Very low capacitance ESD protection“, 14 S. | **Doc ID 11265 Rev 5** | **Oktober 2011** (Kopfzeile S. 1) | `usblc6-2sc6.pdf`, Kopie von LCSC C7519. `st.com` war vom Arbeitsplatz aus nicht abrufbar (Verbindungsfehler), **ob Rev 5 die neueste Revision ist, ist nicht geprüft** |

- **Variante:** **USBLC6-2SC6**, SOT23-6L (Schwester: USBLC6-2P6 in SOT-666) ✔ S. 1.
- **Gehäuse SOT23-6L:** Mass 2,80…3,05 × 1,50…1,75 mm (Körper), Rastermass **0,95 mm**, Höhe max. 1,45 mm ✔ S. 12 (Tab. 4). Empfohlene Lötfläche: Masse 3,50 mm und 2,30 mm (Reihen gegeneinander), Fläche 0,60 mm breit, Rastermass 0,95 mm, Flächenhöhe 1,20 bzw. 1,10 mm ✔ S. 12 (Bild 22; die Bezugskanten der Masslinien 3,50/2,30 sind im Bild nicht eindeutig, beim Zeichnen gegen die PDF-Seite prüfen). Markierung „UL26“ ✔ S. 12 (Bild 23).

## 2. Absolute Maximalwerte und Betriebsbedingungen

| Grösse | Wert | Beleg |
|---|---|---|
| ESD-Spitzenspannung IEC 61000-4-2 **Luft** | 15 kV | ✔ S. 2 (Tab. 1) |
| ESD IEC 61000-4-2 **Kontakt** | **15 kV** laut Tab. 1, aber **8 kV** laut Merkmalsliste („Level 4 … 8 kV contact discharge“, S. 1) | ✔ S. 2 und S. 1 — **die beiden Stellen widersprechen sich**; für die Auslegung gilt der kleinere Wert (8 kV = Level 4) |
| MIL-STD-883G Methode 3015-7 | 25 kV | ✔ S. 2 |
| Lagertemperatur | −55 … +150 °C | ✔ S. 2 |
| Sperrschichttemperatur Betrieb | −40 … +125 °C | ✔ S. 2 |
| Löttemperatur 10 s | 260 °C | ✔ S. 2 |
| Sperrstrom IRM bei VRM = 5,25 V | typ. 10 nA, **max. 150 nA** | ✔ S. 2 (Tab. 2) |
| Durchbruchspannung VBR **VBUS–GND** bei 1 mA | **min. 6 V** | ✔ S. 2 |
| Flussspannung VF bei 10 mA | max. 1,1 V | ✔ S. 2 |
| Klemmspannung VCL, 8/20 µs, I/O gegen GND | **12 V bei 1 A**, **17 V bei 5 A** (Höchstwerte) | ✔ S. 2 |
| Kapazität I/O–GND bei 1,65 V | **typ. 2,5 pF, max. 3,5 pF**; Abgleich ΔC 0,015 pF | ✔ S. 2 (Tab. 2) und S. 1 („3.5 pF max.“) |
| Kapazität I/O–I/O bei 1,65 V | typ. 1,2 pF, max. 1,7 pF; ΔC 0,04 pF | ✔ S. 2 |

**Eine Grenzspannung für den VBUS-Pin im Sinne „absolute maximum“ gibt es nicht** — die Tabelle 1 hat keine Zeile dafür. Aussagekräftig sind VRM = 5,25 V (Sperrbereich, bis dahin max. 150 nA) und VBR ≥ 6 V ✔ S. 2. Ein VBUS über 5,25 V liegt **ausserhalb des Sperrbereichs** und der Baustein beginnt zu klemmen (○ Folgerung aus den ✔-Werten).

## 3. Pinfunktionen

SOT23-6L, Aufsicht ✔ S. 1 (Bild 1, Funktionsschaltbild) und S. 9 (Bild 18):

| Pin | Name | Funktion | Beleg |
|---|---|---|---|
| **1** | I/O1 | Datenleitung 1, **Eingang** (Seite Stecker) | ✔ S. 1, S. 9 (D+in) |
| **2** | GND | Masse | ✔ S. 1, S. 9 |
| **3** | I/O2 | Datenleitung 2, **Eingang** (Seite Stecker) | ✔ S. 1, S. 9 (D−in) |
| **4** | I/O2 | Datenleitung 2, **Ausgang** (Seite Schaltung), intern mit Pin 3 verbunden | ✔ S. 1, S. 9 (D−out) |
| **5** | VBUS | Versorgung/Schutzpfad nach GND | ✔ S. 1, S. 9 |
| **6** | I/O1 | Datenleitung 1, **Ausgang** (Seite Schaltung), intern mit Pin 1 verbunden | ✔ S. 1, S. 9 (D+out) |

Pin 1↔6 und Pin 3↔4 sind je **dieselbe Leitung**, nach aussen auf zwei Pins geführt, damit die Leiterbahn **durch den Baustein hindurch** läuft ✔ S. 9 (Bild 18). Es gibt keine Strapping- oder Boot-Pins; es ist ein passiver Baustein.

## 4. Empfohlene Beschaltung

- **Anschlussschema für USB 2.0 Full-Speed** ✔ S. 8 (Bild 14): D+ und D− laufen von der Buchse **durch** die I/O-Paare (1→6, 3→4) zur Schaltung; **VBUS-Pin an die VBUS-Leitung**; **GND-Pin an die Masse**. Reihenwiderstände RS zur Transceiver-Seite gehören dahinter; die ST-Zeichnung zeigt sie als RS vor dem Sender. Das Bild zeigt den USBLC6-2SC6 und den -2P6 in derselben Rolle.
- **Entkopplung am VBUS-Pin:** Bild 18 (S. 9) zeigt einen Kondensator **C_BUS = 100 nF** am VBUS-Pin ✔ S. 9.
- **Lage:** „A good precaution to take is to put the protection device as close as possible to the disturbance source (generally the connector)“ ✔ S. 6. Die Bahnen vom Daten-Eingang zum I/O-Pin, von VCC zum VBUS-Pin und von der Masse zum GND-Pin müssen **so kurz wie möglich** sein ✔ S. 5 (2.3). Die ST-Rechnung zeigt den Grund: Mit je 10 mm Bahn (6 nH) und einem 8-kV-Kontaktimpuls (24 A) steigt die Klemmspannung durch Leitungsinduktivität von rund 31 V auf **über 300 V** ✔ S. 4 (2.1/2.2). Das ist keine Feinheit der Anordnung, sondern der Unterschied zwischen Schutz und Wirkungslosigkeit.
- **Masse:** GND-Pin direkt auf die Massefläche (kurze, breite Verbindung) ✔ S. 5 (2.3).

## 5. Kritische Daten für dieses Projekt

- **Kapazität 3,5 pF max.** (I/O–GND) ✔ S. 2: für Full-Speed (12 Mbit/s) und selbst für High-Speed (480 Mbit/s, ST-Angabe S. 1: „USB 2.0 ports up to 480 Mb/s (high speed) … compatible with USB 1.1 low and full speed“) unkritisch.
- **Zwei Leitungen + VBUS:** Eine USB-C-Buchse hat nach dem Brücken (A6/B6, A7/B7; Blatt `usb-c-16p.md`) **ein** D+/D−-Paar. **Ein** USBLC6-2SC6 genügt also ✔ (Pinzahl) ○ (Folgerung).
- **CC-Pins sind damit nicht geschützt:** Der Baustein schützt D+, D− und VBUS ✔ S. 1 („2 data-line protection, protects VBUS“). CC1/CC2 hängen an keinem Schutzpfad. Ob das nötig ist, ist eine Designentscheidung (offen).
- **VBUS höchstens ~5,25 V:** Die Schutzstruktur sperrt bis 5,25 V ✔ S. 2. USB-C mit **PD > 5 V** wäre damit nicht verträglich. Hier ist das nicht verlangt (die Senke handelt keine Spannung aus); wer später PD zulässt, muss den Baustein überprüfen.

## 6. Fertigung bei JLCPCB

Prüfdatum **2026-10-17**. Klasse/Lager aus der Teilebibliothek von JLCPCB über die Such-Schnittstelle von `jlcpcb.com/parts` (●, Schlüsselwort = LCSC-Nummer), LCSC-Lager aus `lcsc.com/product-detail/<Nr>.html` (●).

| LCSC | Bauteil | Hersteller | Klasse | Lager JLCPCB | Lager LCSC | Hinweis |
|---|---|---|---|---|---|---|
| **C7519** | **USBLC6-2SC6**, SOT-23-6L | **STMicroelectronics** | **Extended** (kein Preferred-Flag) | **36'415** | 34'225 | gewählt: das Original, das im Auftrag steht |
| C2969755 | USBLC6-2SC6Y (Automotive, −40…+150 °C, Ci/o typ. 2,5 pF) | STMicroelectronics | Extended | 11'824 | nicht abgefragt | gleiche Familie, andere Güte; nicht gelesen ○ |
| C2687116 | „USBLC6-2SC6“ | UMW (Youtai) | Extended | 143'251 | nicht abgefragt | Nachbau; Beschreibung nennt **0,35 pF/0,8 pF, 100 nA, 6 V, 5 V**, also **andere Kennwerte** als das ST-Teil; elektrische Gleichwertigkeit ○ nicht gelesen |
| C2827654 | „USBLC6-2SC6“ | TECH PUBLIC | Extended | 58'430 | nicht abgefragt | Nachbau, 0,35 pF; gleiche Einschränkung |

- **Grund für Extended:** Es gibt keinen Basic-Baustein dieser Art. Durchsucht wurden **vollständig** die Begriffe „USBLC6“ (45 von 45 Treffern), „SOT-23-6 TVS“ (14 von 14) — **kein** Basic, kein Preferred. Der Begriff „USB ESD“ liefert 16'266 Treffer; betrachtet wurden die ersten 200 (kein Basic, kein Preferred) — keine vollständige Durchsicht.
- **SMT-bestückbar:** SOT-23-6, SMD-Gehäuse ✔ (Quelle: Gehäusefeld „SOT-23-6L“). Bestückungsweg nicht geprüft, aber ein SMD-Gehäuse.
- **Nummer C7519 ist geprüft** (Treffer mit `componentCode`).

## 7. Fallen und Errata

- **Datenblatt und Tabelle widersprechen sich beim Kontakt-ESD:** 15 kV (Tab. 1, S. 2) gegen 8 kV (Merkmalsliste S. 1). Nimm 8 kV.
- **Die Leitungen müssen durch den Baustein laufen.** Wer D+ „an Pin 1 abzweigt“ und weiterführt, hat den Baustein an einem **Stichstück** — dann gilt die 300-V-Rechnung von S. 4.
- **VBUS-Pin nicht offen lassen:** ST beschreibt eine „rail to rail“-Topologie, in der die Datenleitung gegen VBUS und GND klemmt ✔ S. 4; dass ein offener VBUS-Pin die Klemmung nach oben unwirksam macht, ist ○ (Folgerung, nicht als Satz gelesen).
- **ST-Original gegen Nachbau:** Die Kapazität des Nachbaus (0,35 pF) ist **kleiner** als die des ST-Teils (2,5 pF typ.). Das ist nicht „gleichwertig“, sondern ein anderes Bauteil; die Klemmspannung des Nachbaus steht in dessen eigenem Datenblatt (nicht gelesen).
- **Revision 2011:** das Datenblatt ist alt; neuere ST-Fassungen können Werte ergänzen. Nicht geprüft, siehe 1.

## 8. Folgerung für unsere Platine

1. **Ein** USBLC6-2SC6 (C7519) **direkt an der USB-C-Buchse**: I/O1-Paar (Pin 1/6) in D+, I/O2-Paar (Pin 3/4) in D−, Pin 5 an VBUS (mit 100 nF), Pin 2 an die Massefläche.
2. D+/D− laufen durch den Baustein, nicht daran vorbei; Abzweige vermeiden.
3. **Entscheiden:** Soll CC1/CC2 auch geschützt werden? Dieses Teil kann es nicht.
4. **Entscheiden:** Ein Nachbau (UMW/TECH PUBLIC) nur mit eigenem Datenblatt, nicht als „gleich“.

## 9. Offene Punkte

- Neueste ST-Revision des Datenblatts nicht geprüft (`st.com` nicht erreichbar).
- Gleichwertigkeit der Nachbauten nicht gelesen.
- Ob JLCPCB den Baustein mit der Pin-1-Markierung richtig dreht (Rotationstabelle): nicht geprüft; beim Zeichnen der Stückliste mit der Rotation aus der Bestückungsvorschau abgleichen.
