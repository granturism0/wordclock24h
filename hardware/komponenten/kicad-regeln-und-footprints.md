# KiCad-Grundlagen für das Layout: Design-Regeln, Board Setup, Footprints, Platzierung

Stand: 17.10.2026. Zielprogramm: **KiCad 10.0.2** (`kicad-cli` unter
`/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli`). Ergänzt `layout-regeln.md` und `jlcpcb-fertigung.md`
(allgemeine Regeln, JLC-Werte) sowie `kicad-layout-werkzeug.md` (Skript- und Router-Wege). Dort stehende
Werte wiederhole ich nicht; ich verweise.

## 0. Beleg-Kennzeichnung in diesem Blatt

| Zeichen | Bedeutung hier |
|---|---|
| ✔ Doku | gelesen in der KiCad-10-Dokumentation `https://docs.kicad.org/10.0/en/pcbnew/pcbnew.html` (am 17.10.2026 vollständig geladen und lokal durchsucht), mit Abschnittsname |
| ✔ Versuch | selbst ausprobiert, mit Befehl; Arbeitskopien lagen im Scratchpad, **nichts im Repo oder in V2 geändert** |
| ✔ Datei | gelesene Datei (Pfad genannt) |
| ✔ DB | Datenblatt/Zeichnung gelesen (Seite genannt) |
| ● | Sekundärquelle (GitHub-README, GitHub-API) |
| ○ | Annahme, **nicht verwenden** |

Reproduktion der Versuche: Arbeitskopie von `V2.kicad_pcb/.kicad_pro/.kicad_dru` nach `…/scratchpad/v2c` (Namen
vereinheitlicht), danach `kicad-cli pcb drc --severity-all --format json -o drc.json <platine>`. Footprints
wurden mit dem mitgelieferten Python (`/Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/Current/bin/python3`,
`import pcbnew`, `pcbnew.FootprintLoad(bibliothek, name)`) in leere Testplatinen gesetzt.

## 1. Benutzerdefinierte Design-Regeln (`.kicad_dru`) in KiCad 10

### 1.1 Datei und Auswertung ✔ Doku („Custom design rules“)

- Eigene Datei `<projekt>.kicad_dru` neben `.kicad_pcb`/`.kicad_pro`; **muss mit ins Backup/Git**. Die Doku verlangt,
  sie nur im Dialog *Board Setup → Custom Rules* zu bearbeiten, nicht mit einem externen Editor. (Für uns
  gilt: Skript-Erzeugung ist möglich, weil KiCad die Datei nur liest. Danach **immer** `kicad-cli pcb drc` laufen
  lassen: Ein Syntaxfehler „verhindert, dass der DRC läuft“ ✔ Doku.)
- Kopfzeile **`(version 1)`** (gilt unverändert für KiCad 10.0) ✔ Doku.
- **Auswertung in umgekehrter Reihenfolge: die letzte Regel der Datei wird zuerst geprüft, die erste passende gewinnt**
  (je Constraint-Typ). Spezielle Regeln gehören deshalb **ans Ende**, allgemeine an den Anfang ✔ Doku.
- Mindestwerte aus *Board Setup → Constraints* sind **absolute Untergrenzen**; keine Regel und keine Netzklasse kann
  sie unterschreiten ✔ Doku („Constraints“, „Net classes“).
- Kommentare: Zeile beginnt mit `#`. Textvariablen `${…}` werden vorab ersetzt, aber nicht `${LAYER}` o. ä. ✔ Doku.
- Einheiten in Bedingungen: `mm`, `mil`/`th`, `in`, `deg`, `rad`; ohne Suffix Nanometer ✔ Doku. Einfache Rechnung erlaubt
  (`(min 0.5mm + 0.1mm)`). Nullable-Eigenschaften werden gegen `null` verglichen.
- Zeichenketten in Bedingungen: Gross-/Kleinschreibung bei `Type` ist **egal**. Versuch: V2-Regel
  `A.Type == 'Pad'` (grosses P) greift ✔ Versuch (DRC meldet „Regel ‚Pad Size‘“).

### 1.2 Aufbau einer Regel ✔ Doku

```
(rule <name>
    [(severity <error|warning|ignore|exclusion>)]
    [(layer <layername | outer | inner>)]
    [(condition "<ausdruck>")]
    (constraint <typ> [(min x) (opt y) (max z)] ))
```

- `layer outer` = F.Cu+B.Cu, `inner` = alle Innenlagen; ohne Layer-Klausel gilt die Regel für alle Lagen. Die Layer-Klausel ist
  schneller als `A.Layer == …`.
- `severity ignore` schaltet die Regel nicht ab: Sie wird weiter zur Zuordnung ausgewertet und **überschreibt frühere**
  Regeln; nur ihre Wirkung (Meldung, Router-Beachtung) entfällt ✔ Doku.
- Objekte heissen `A` und `B` (Reihenfolge egal, DRC prüft beide), `AB` für Paar-Funktionen.
- Operatoren `== != > >= < <= && || !`, **alle gleichwertig, von links nach rechts** (Klammern setzen!). Boolesche
  Eigenschaften ohne `== true` abfragen (`A.Do_not_populate`, `!A.Do_not_populate`) ✔ Doku.

### 1.3 Constraint-Typen (vollständig aus der Doku) ✔ Doku („Constraint Clauses“)

| Typ | Argument | Wirkung |
|---|---|---|
| `clearance` | min | elektrischer Abstand Kupfer verschiedener Netze (negativer Wert erlaubt Überlappung) |
| `physical_clearance` | min | Abstand beliebiger Objekte auf physischen Lagen, **netzunabhängig** (langsam) |
| `edge_clearance` | min | Abstand zu Edge.Cuts („Fräsertoleranz“) |
| `hole_clearance` / `physical_hole_clearance` | min | Bohrung zu Kupfer anderer Netze / zu allem („Bohrtoleranz“) |
| `hole_to_hole` | min | Bohrer-Schutz, gemessen Rand zu Rand |
| `hole_size` | min/opt/max | Bohrdurchmesser Pad/Via (bei Langloch kleiner Wert gegen min, grosser gegen max) |
| `annular_width` | min/max | Restring Via/Pad |
| `via_diameter`, `track_width` | min/opt/max | opt = Router-Vorgabe |
| `via_count` | min/max | Vias je Netz |
| `track_angle`, `track_segment_length` | min/max | Winkel, Segmentlänge |
| `connection_width`, `min_resolved_spokes` | min / 0…4 | Anbindung Pad-Zone, Zahl der Speichen |
| `zone_connection` | solid / thermal_reliefs / none | Pad-Zonen-Anbindung |
| `thermal_relief_gap`, `thermal_spoke_width` | min / opt | Thermal-Geometrie |
| `diff_pair_gap` | min/opt/max | Abstand paralleler Bahnen eines Paares (nicht auf entkoppelten Abschnitten) |
| `diff_pair_uncoupled` | max | zulässige entkoppelte Länge je Paar (Fan-out, Ausweichen um Via) |
| `length` | min/opt/max | Gesamtlänge je Netz (opt = Ziel des Längentuners) |
| `skew` | min/opt/max/`within_diff_pairs` | Längendifferenz zum längsten passenden Netz |
| `silk_clearance` | min | Siebdruck zu anderem |
| `text_height`, `text_thickness` | min/max | Schrift |
| `courtyard_clearance` | min | Abstand Courtyards (ohne Courtyard keine Meldung) |
| `solder_mask_expansion` | opt | Lötstopp-Aufweitung |
| `solder_paste_abs_margin`, `solder_paste_rel_margin` | opt | Pastenöffnung (meist negativ) |
| `bridged_mask`, `via_dangling` | nur mit `severity` | erlaubt/verbietet Maskenstege-Lücken bzw. hängende Vias |
| `creepage` | min | Kriechstrecke |
| `disallow` | track, via, through_via, micro_via, blind_via, buried_via, pad, zone, text, graphic, hole, footprint | wie Rule Area, aber gezielt |
| `assertion` | Ausdruck | DRC-Fehler, wenn der Ausdruck falsch ist |

### 1.4 Bedingungen: Eigenschaften und Funktionen ✔ Doku („Object property and function reference“)

- Allgemein: `Layer`, `Type` (`Bitmap, Dimension, Footprint, Graphic, Group, Leader, Pad, Target, Text, Text Box, Track, Via, Zone`),
  `Position_X/Y`, `Locked`.
- Verbunden: `Net` (Nummer, schnell), **`NetName`** (stabil), **`NetClass`** (prioritätsgeordnete Liste),
  Teardrop-Eigenschaften (`Enable_Teardrops`, `Best_Length_Ratio`, `Max_Length` …).
- Footprint: `Reference`, `Value`, `Component_Class`, `Do_not_Populate`, `Orientation`, `Library_Link`, `Keywords`,
  `Exclude_From_Position_Files`, `Exclude_From_Bill_of_Materials`, `Solderpaste_Margin_Override` …
  (Kinder wie Pads erben Reference/Value/Component_Class des Footprints).
- Pad: `Pad_Type` (`Through-hole, SMD, Edge connector, NPTH, mechanical`), `Pad_Shape`, `Size_X/Y`, `Hole_Size_X/Y`,
  `Pad_Number`, `Pin_Name`, `Pin_Type`, `Soldermask_Margin_Override`, `Solderpaste_Margin_Override`,
  **`Fabrication_Property`** (`None, BGA pad, Fiducial global to board, Fiducial local to footprint, Test point pad,
  Heatsink pad, Castellated pad`), `Zone_Connection_Style`, `Pad_To_Die_Length`.
- Bahn: `Width`, `Origin_X/Y`, `End_X/Y`. Via: `Diameter`, `Hole`, `Via_Type` (`Blind, Buried, Micro, Through`).
  Zone/Rule Area: `Name`, `Priority`, `Min_Width`, `Pad_Connections` …
- Funktionen: `hasNetclass('x')`, `hasExactNetclass()`, **`inDiffPair('/USB')`** (wahr nur, wenn es das Gegenstück
  `/USB_P`+`/USB_N` bzw. `…_`-Paar tatsächlich gibt), **`isCoupledDiffPair()`** (nur `AB`), `intersectsArea('name')`
  (benannte Rule Area/Zone; `insideArea` und `enclosedByArea` sind veraltet bzw. langsamer), `intersectsCourtyard('U2')`,
  `memberOfFootprint('R*')`, `memberOfGroup()`, `existsOnLayer()`, `isPlated()`, `isBlindVia()` …, `hasComponentClass()`,
  `getField()`, **`fromTo('U1-A5','J1-2')`** (nur der Kupferpfad zwischen zwei Pads, z. B. für `length`/`skew`).

### 1.5 Beispiele (Doku, auf unsere Platine zugeschnitten markiert) ✔ Doku

```
# Doku-Beispiel: Netzklasse
(rule HV (condition "A.hasNetclass('HV')") (constraint clearance (min 1.5mm)))

# Doku-Beispiel: nur Oberseite
(rule "Top side footprints only" (layer B.Cu) (constraint disallow footprint))

# Doku-Beispiel USB-Pfad (Pad-Namen anpassen)
(rule "USB track width"  (condition "A.fromTo('J1-D*', 'U1-H*')")
      (constraint track_width (min 0.09mm) (opt 0.09mm) (max 0.09mm)))
(rule "USB path length"  (condition "A.fromTo('J1-D*', 'U1-H*')") (constraint length (max 80mm)))
```

Differenzialpaar, **für uns zu entwerfen, nicht aus der Doku kopiert** (Syntax nach 1.3/1.4; die Zahlen hier sind
Platzhalter ○ und kommen aus `layout-regeln.md`/Datenblatt):
`(rule "USB diff" (condition "A.inDiffPair('/USB_D')") (constraint diff_pair_gap (opt <gap>)) (constraint diff_pair_uncoupled (max <l>)))`
und `(rule "USB skew" (condition "A.inDiffPair('/USB_D')") (constraint skew (max <x>) (within_diff_pairs)))`.
**Namensfalle:** `inDiffPair` verlangt Netznamen, die sich nur im Endbuchstaben `P/N` oder `+/-` unterscheiden. Die V3-Netze heissen
`/USB_DP` und `/USB_DN` (✔ Datei, `kicad-cli sch export netlist`, V3-Netzliste), dazu `/USB_DP_M`/`/USB_DN_M`
(Modulseite hinter R34/R35). Ob `inDiffPair('/USB_D')` bei `…DP`/`…DN` anschlägt, ist **nicht ausprobiert** ○ — im DRC mit einer
absichtlich verletzten Regel prüfen (DIR-014), bevor man ihr traut. Zwischen R34/R35 wechselt das Netz; die Länge je Teilstück
braucht `fromTo`-Regeln je Abschnitt (Doku: „bei einem Reihenbauteil ändert sich das Netz“).

Aus der Doku: **Netzklassen-Zuordnung hat keine Min/Max-Wirkung** — Breite/Via der Klasse sind nur *Vorgaben* (optimal), der DRC
meldet bei Abweichung nichts. Erzwingen geht nur mit `track_width`/`via_diameter` in einer Custom-Regel ✔ Doku („Net classes“).

### 1.6 Was V2 hat, und die Lücken ✔ Datei (`…V2.kicad_dru`, 5600 Byte; `…V2.kicad_pro`) + ✔ Versuch

**V2 trägt die PCBWay-Vorlage** („#PCBWay Custom DRC for Kicad 7“), nicht eine JLCPCB-Vorlage. Aktiv sind: Bahnbreite/-abstand
aussen **0,127 mm**, innen 0,1 mm (2-Lagen-1-oz-Zeile), Via-Bohrung min 0,3 / Via-Ø min 0,5, Bohrung allgemein 0,15…6,3,
`PTH Hole Size` 0,2…6,35, NPTH min 0,5, **`Pad Size`: Bohrung min 0,5 und Restring min 0,25 (für alle plattierten Pads)**, Langlöcher
min 0,5/0,8, Loch-zu-Loch 0,5, Via-zu-Bahn 0,254, Pad-zu-Bahn 0,2, PTH-zu-Bahn 0,33, **Bahn zu Platinenrand 0,3**, Text min 0,8 mm/0,15 mm,
Pad-zu-Siebdruck 0,15. Die Werte stammen aus der **PCBWay**-Vorlage, nicht aus der JLCPCB-Fähigkeitenliste; die JLC-Werte stehen in
`jlcpcb-fertigung.md` und sind gegen sie abzugleichen (○, hier nicht geprüft).

**Der V2-Entwurf besteht seine eigene Regeldatei nicht.** Versuch: `kicad-cli pcb drc` auf einer Kopie von V2 (KiCad 10.0.2,
Regeldatei lädt ohne Änderung) meldet **241 Verstösse + 1 unverbundenes Element**: 142 `silk_overlap`, 58 `silk_edge_clearance`,
14 `text_thickness`, 9 `text_height`, 8 `silk_over_copper`, **5 `annular_width`** (davon 3 am TO-92 `U1`, 0,15 statt 0,25 mm;
2 am Taster `SW4`), 2 `track_dangling`, 2 `padstack`, 1 `lib_footprint_issues` (ESP-12F nicht in der Bibliothek),
**1 offenes Netz** (`Net-(Q2-D)`, R21 ↔ Bahn). Bis auf den Siebdruck ist nichts davon fertigungskritisch bekannt; der Siebdruck ist
**Warnung** (Schweregrad in `V2.kicad_pro`). Folge: Die V2-DRC-Meldungsliste taugt nicht als „sauber“-Referenz; eine
Abnahmeregel muss sagen, welche Meldungen bewusst bleiben (Ausnahme mit Begründung, siehe 5).

**Lücken von V2 gegenüber dem, was V3 braucht** (alles aus der Datei gelesen, daher ✔ Datei):

| Lücke | Folge |
|---|---|
| keine Regel für Differenzialpaar, Länge, Skew | USB D+/D− (und S3-Modulseite) sind ungeprüft; V2 hatte keine Datenleitungen am Stecker |
| `Pad Size` trifft **alle** plattierten Pads | siehe 1.7: schlägt bei der ESP32-S3-Modul-Masse, USB-Schirmpads und TO-92 an |
| keine Antennen-Rule-Area (V2: ESP-12F) | im S3-Footprint steckt eine (siehe 4.1) |
| keine Regel nach Netzklasse | Strom-Bahnbreiten werden nur über die Klasse vorgegeben, nicht geprüft |
| `missing_courtyard` auf **ignore** (V2.kicad_pro) | EasyEDA-Footprints ohne Courtyard bleiben still; siehe 3.4 |
| `lib_footprint_issues`/`footprint_symbol_mismatch` nur Warnung | Footprint-Vertauschungen (R27/R30/R33, siehe 3.5) bleiben unbemerkt |
| `solder_mask_to_copper_clearance 0.005`, `min_clearance 0.0`, `min_copper_edge_clearance 0.0`, `min_silk_clearance 0.0` im Board Setup | die eigentlichen Grenzen stehen nur in der .dru; fehlt diese Datei, prüft der DRC (fast) nichts |

### 1.7 Versuch: V2-Regeln gegen die Footprints, die V3 braucht ✔ Versuch

Testplatine (Rechteck 280 × 100 mm) mit je einem Exemplar aus der Standardbibliothek bzw. `EasyEDA.pretty`, V2-`.kicad_dru`:

| Footprint | Ergebnis unter V2-Regeln |
|---|---|
| `RF_Module:ESP32-S3-WROOM-1` | **12 × `drill_out_of_range`** (Regel „Pad Size“, Bohrung 0,2 < 0,5; die 15 Wärme-Durchkontaktierungen des Massepads 41) und 4 × `annular_width` (0,2) — **Falsch­alarm der Regel**, nicht der Footprint |
| `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` | ohne Netze 8 Meldungen (Pads A1/B12 usw. liegen übereinander, `clearance` + `solder_mask_bridge`); **mit Netzen (GND, VBUS) keine** — nur Schirmpads `SH` 4 × `annular_width` 0,2 |
| `Package_TO_SOT_THT:TO-92_Inline` | 3 × `annular_width` 0,15 (Pad 1,05 × 1,5, Bohrung 0,75) — wie in V2 |
| SOT-23-6, SOT-23, CFP3-SOD-123W, EasyEDA-SOT-23-6, EasyEDA-HDR 7P | keine Meldung |

**Vorgeschlagene Änderung, getestet:** Die Regel „Pad Size“ um `&& A.Fabrication_Property != 'Heatsink pad'` ergänzen
(`(condition "A.Type == 'Pad' && A.isPlated() && A.Fabrication_Property != 'Heatsink pad'")`). Ergebnis ✔ Versuch: die 12 Bohrungs-Meldungen
des Moduls verschwinden; es bleiben nur die 4 USB-Schirmpads und das TO-92 (je Restring < 0,25 mm). Ob 0,25 mm Restring für JLC nötig
ist, steht in `jlcpcb-fertigung.md`; hier nur: Die V2-Zahl 0,25 stammt von PCBWay. Die Thermal-Vias des Moduls tragen im Footprint
die Eigenschaft `pad_prop_heatsink` ✔ Datei (`ESP32-S3-WROOM-1.kicad_mod`). Ob 0,2-mm-Bohrungen bei JLC (zwei Lagen) herstellbar
sind, ist in `jlcpcb-fertigung.md` zu klären; hier **nicht geprüft**.

## 2. Board Setup in `.kicad_pro` / `.kicad_pcb`: Stackup, Netzklassen, Via-Grössen

### 2.1 Wo was steht

| Einstellung | Datei | Schlüssel/Abschnitt | Beleg |
|---|---|---|---|
| Lagenaufbau (Stackup), Dicke, Kupferdicke, Lötstopp, Finish | **`.kicad_pcb`** | `(setup (stackup …))`, `copper_finish` | ✔ Datei |
| Netzklassen, Muster-Zuordnung | `.kicad_pro` | `net_settings.classes`, `net_settings.netclass_patterns` | ✔ Datei |
| Vordefinierte Bahnbreiten, Via-Grössen, DP-Masse | `.kicad_pro` | `board.design_settings.track_widths`, `via_dimensions`, `diff_pair_dimensions` | ✔ Datei |
| DRC-Mindestwerte (`min_clearance`, `min_track_width`, `min_via_diameter`, `min_through_hole_diameter` …) | `.kicad_pro` | `board.design_settings.rules` | ✔ Datei |
| DRC-Schweregrade | `.kicad_pro` | `board.design_settings.rule_severities` | ✔ Datei |
| Teardrop-Vorgaben | `.kicad_pro` | `teardrop_options`, `teardrop_parameters` | ✔ Datei |
| Lötstopp/Paste global (`pad_to_mask_clearance`, Tenting) | `.kicad_pcb` | `(setup …)` | ✔ Datei |
| eigene Regeln | `.kicad_dru` | siehe 1 | ✔ Doku |
| Text-/Linienvorgaben, Zonenvorgaben | `.kicad_pro` | `design_settings.defaults` | ✔ Datei |

Das **V3-Projekt (`hardware/esp32s3/WordClock-ESP32S3-V3.kicad_pro`) ist ein Stub**: nur `meta`, `schematic`, `sheets` (✔ Datei, 5 Zeilen).
**Es gibt dort weder Board Setup noch Netzklassen noch eine `.kicad_dru`, und noch keine `.kicad_pcb`.** Das ist für das Layout die
Hauptaufgabe (Grundlage kopieren: siehe 2.4).

Die Doku sagt zu Netzklassen: Mehrere Klassen je Netz möglich, Priorität nach Reihenfolge, **„Default“ ist immer Rückfall** für nicht gesetzte
Eigenschaften; Zuordnung per Muster (`GND`, `VCC*`) im Board- oder Schaltplan-Setup oder per Schaltplan-Direktive ✔ Doku („Net classes“).
Per Skript lässt sich `.kicad_pro` als JSON bearbeiten (Wertebereich `design_settings`); danach **KiCad schliessen**, sonst schreibt es die
Datei beim Beenden zurück (○ aus der Beobachtung, dass `~V2.kicad_pro.lck` im V2-Ordner liegt; nicht systematisch geprüft).

### 2.2 Stackup in V2 ✔ Datei (`V2.kicad_pcb`, `(stackup …)`)

Zwei Kupferlagen, **FR4 Kern 1,51 mm** (εr 4,5, tanδ 0,02), Kupfer je **0,035 mm**, Lötstopp je 0,01 mm, Siebdruck/Paste ohne Dicke,
`copper_finish "None"` (die Voreinstellung; der Auftrag an JLC bestimmt die Oberfläche, nicht diese Zeile ○), `dielectric_constraints no`.
Gesamtdicke aus den Zeilen: 1,51 + 2·0,035 + 2·0,01 = **1,60 mm** (Rechnung). Das Stackup geht **nur** in 3D-Export und in Längenrechnung
über Vias ein ✔ Doku („Physical stackup“); es steuert nicht die Bestellung. Für USB-Impedanzrechnung hat KiCad 10 *Tuning Profiles*
(Rechner aus Stackup und Ziel-Impedanz, Zuweisung an Netzklassen; DRC-Prüfung `tuning_profile_track_geometries` optional) ✔ Doku
(„Tuning profiles“). V2 hat diese Möglichkeit leer (`tuning_profiles: []`).

### 2.3 Netzklassen und Via-Grössen in V2 ✔ Datei (`V2.kicad_pro`)

| Klasse | Bahnbreite | Abstand | Via Ø/Bohrung | Zuordnung (Priorität) |
|---|---|---|---|---|
| Default | 0,1524 mm | 0,1524 mm | 0,4572 / 0,254 mm | Rückfall |
| Power_1 | **2,0 mm** | 0,1524 | 2,0 / 1,4 | Muster `GND`, `VCC*`, `*5V*` (Priorität 0) |
| Power_2 | 0,508 | 0,1524 | 0,4572 / 0,254 | **keine Muster** — nie zugewiesen |

Auffälligkeiten (alles gelesen, Folgerung gerechnet):
- **`Power_1` hat 2,0-mm-Bahnen und 2,0/1,4-mm-Vias für GND und jedes `VCC*`/`*5V*`-Netz**; das passt für Stromschienen, nicht für
  jede 3,3-V-Stichleitung zum Pin. Das Muster `*5V*` erfasst auch `USB_5V_INPUT`, `ALT_5V_PROT` u. ä.
- **Default-Via 0,4572 / 0,254 mm unterschreitet die V2-eigene Regel (Bohrung min 0,3, Via-Ø min 0,5)**; die Netzklasse ist nur Vorgabe,
  der Router setzt sie trotzdem — im V2-DRC erscheint dazu **keine** Meldung (✔ Versuch, keine `via_diameter`-Meldung unter den 241), Ursache ○
  (die Klassenwerte sind nur Vorgaben, geprüft werden gesetzte Vias). Im V2-Board kommen tatsächlich Vias mit (Ø/Bohrung) 0,5032/0,3,
  0,61/0,3 und 1,0/0,7 vor (✔ Datei, 339 Vias gezählt). Die Standardgrösse der Klasse ist also nicht die verwendete.
- Vordefinierte Via-Liste: 0,5032/0,3 · 0,61/0,4 · 1,0/0,7 · 1,8/1,4 · 2,0/1,4. `min_via_diameter 0,5`, `min_through_hole_diameter 0,2`,
  `min_track_width 0,1016`, `min_via_annular_width 0,1016`, `min_hole_clearance 0,254`, `min_hole_to_hole 0,254`.
- Vordefinierte Bahnbreiten: 0,1016 · 0,127 · 0,1524 · 0,2032 · 0,254 · 0,3048 · 0,508 · 0,762 · 1,0 · 1,8 · 2,0.

### 2.4 Folgerung (V3)

1. **Grundlage:** V2-`.kicad_pcb` als Kopie (so verlangt der Skill `kicad-schema`) — dann erbt V3 Umriss (12 Edge-Cuts-Linien, 145,05 × 39,05 mm
   Bounding Box inklusive Linienbreite ✔ Versuch, `GetBoardEdgesBoundingBox`), Stackup und die GND-Zone.
2. **`.kicad_pro` für V3 neu aufbauen:** Netzklassen mit **gesetzten Mindest- und Vorgabewerten aus `jlcpcb-fertigung.md`** (nicht aus V2/PCBWay),
   Muster für 3,3 V/5 V/GND/USB; `Power_2` hat in V2 keine Zuordnung.
3. **`.kicad_dru` für V3:** Vorlage V2 übernehmen, `Pad Size` wie in 1.7 anpassen, JLC-Zahlen statt PCBWay, Antennen-Rule-Area (4.1),
   USB-Paarregeln (1.5).
4. Jede neue Regel einmal absichtlich verletzen und die Meldung ansehen (DIR-014).

## 3. Footprints

### 3.1 Was die KiCad-Standardbibliothek hat ✔ Versuch (`ls` in `/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints`)

| Bedarf | Standard-Footprint vorhanden | Anmerkung |
|---|---|---|
| ESP32-S3-WROOM-1 | **ja**: `RF_Module:ESP32-S3-WROOM-1` (auch `-1U`, `-2`) | Beschreibung verweist auf das Espressif-Datenblatt; **enthält Antennen-Rule-Area und Courtyard** (4.1) |
| USB-C 16P (JLC C165948 = HRO TYPE-C-31-M-12) | **ja**: `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` (daneben `…-M-17`, GCT, XKB, HCTL u. a.) | Beschreibung nennt `krhro.com`; der Name ist **der Teilname von C165948** (Komponentenblatt `usb-c-16p.md`) |
| USBLC6-2SC6 | **ja**: `Package_TO_SOT_SMD:SOT-23-6` | im Entwurf verwendet |
| SOT-23 (MMBF170) | ja: `Package_TO_SOT_SMD:SOT-23` | im Entwurf verwendet |
| SOD-123W (PMEG2010ER) | ja: `Diode_SMD:Nexperia_CFP3_SOD-123W` | im Entwurf verwendet |
| SMA (SMAJ5.0CA) | ja: `Diode_SMD:D_SMA` | Entwurf nutzt EasyEDA-Footprint |
| SOIC-8 150 mil | ja: `Package_SO:SOIC-8_3.9x4.9mm_P1.27mm` | Entwurf: `Samacsys:SOIC127P600X175-8N` (DS3231) und EasyEDA-SOP-8 (AT24C32) |
| PowerPAK SO-8 | ja: `Package_SO:PowerPAK_SO-8_Single` | Pads dort 1–4 und „5“ mehrfach; **Entwurf nutzt Snapeda mit Pads 1–9**, Austausch bricht die Pinzuordnung (3.3) |
| TPS2121 (VQFN-HR-12, RUX0012A) | ja: `Package_DFN_QFN:Texas_VQFN-HR-12_2x2.5mm_P0.5mm` | Entwurf nutzt EasyEDA-Footprint, Geometrie gleich (3.3) |
| Sicherung 1206/1812 | ja: `Fuse:Fuse_1206_3216Metric`, `Fuse_1812_4532Metric` | Entwurf: EasyEDA |
| LED 0805, Induktivität 2520, Widerstandsarray 4×0402 (`R_Array_Convex_4x0402` u. a.) | ja | Entwurf: EasyEDA |
| Batteriehalter SMTU2032-LF | **nein** (Keystone-Typen vorhanden, nicht dieses) | EasyEDA-Footprint, Gegenprobe fehlt (3.3) |
| Fiducial | ja: `Fiducial:Fiducial_1mm_Mask2mm` u. a. (10 Varianten) | siehe 4.4 |
| Befestigung | ja: `MountingHole:MountingHole_2.2mm_M2_DIN965_Pad`, `…_2.7mm_M2.5_Pad` (auch `_TopOnly`, `_Via`) | im Entwurf verwendet |

### 3.2 Footprints im Entwurf (V3-Netzliste, `kicad-cli sch export netlist --format kicadxml`) ✔ Datei/Versuch

Alle Footprints liegen **in Bibliotheken ausserhalb des Repos**: `EasyEDA`, `Snapeda`, `Samacsys` stehen in der **globalen**
`fp-lib-table` (`~/Library/Preferences/kicad/10.0/fp-lib-table`) mit Pfad `${KICAD_3RD_PARTY}/…pretty`, und `KICAD_3RD_PARTY` ist in
`kicad_common.json` auf **`/Users/daniel/Documents/KiCad/Libraries`** gesetzt ✔ Datei. Das V3-Projekt trägt **keine eigene Tabelle und keine
Footprint-Kopien**. Alle 14 Pfade der `EasyEDA`-Footprints sowie die beiden anderen wurden aufgelöst gefunden ✔ Versuch (`ls`, 43 Dateien in
`EasyEDA.pretty`). Folge: Auf einer anderen Maschine (oder beim `hw-layouter`) fehlen sie, bis `fp-lib-table` und die Bibliotheken vorhanden sind;
nach „PCB aus Schaltplan aktualisieren“ stehen sie aber **eingebettet in der `.kicad_pcb`** (V2 zeigt dasselbe: nur der fehlende
`RF_Module:ESP-12F` wird gemeldet ✔ Versuch). Vorschlag: die benötigten EasyEDA/Snapeda/Samacsys-Footprints als projekteigene Bibliothek
`hardware/esp32s3/…pretty` ins Repo legen (○ Entscheidung `hw-designer`/`hw-layouter`).

| Ref | Wert | Footprint | LCSC (Feld) | Herkunft |
|---|---|---|---|---|
| U2 | ESP32-S3-WROOM-1-N8 | `RF_Module:ESP32-S3-WROOM-1` | C2913198 | KiCad |
| USB1 | USB-C 16P | `Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12` | C165948 | KiCad |
| U11 | USBLC6-2SC6 | `Package_TO_SOT_SMD:SOT-23-6` | C7519 | KiCad |
| Q2 | MMBF170 | `Package_TO_SOT_SMD:SOT-23` | C130879 | KiCad |
| D2, D3 | PMEG2010ER | `Diode_SMD:Nexperia_CFP3_SOD-123W` | C82288 | KiCad |
| U1 | DS18B20 | `Package_TO_SOT_THT:TO-92_Inline` | C5451632 | KiCad |
| R*, C* (0805) | | `Resistor_SMD:R_0805_2012Metric`, `Capacitor_SMD:C_0805_2012Metric` | versch. | KiCad |
| JP2, JP3 | Lötbrücke | `Jumper:SolderJumper-2_P1.3mm_Open_RoundedPad1.0x1.5mm` | — | KiCad |
| H1, H2, H8–H10 | Befestigung | `MountingHole:…2.2mm_M2_DIN965_Pad`, `…2.7mm_M2.5_Pad` | — | KiCad |
| TP1, TP4–TP8 | Testpunkte | `TestPoint:TestPoint_Pad_D1.0mm` | — | KiCad |
| Q1, Q3 | SI7149DP | `Snapeda:POWERPAK_SO-8` | C553982 | Snapeda |
| U5 | DS3231MZ+ | `Samacsys:SOIC127P600X175-8N` | C107410 | SamacSys |
| U3 | SN74AHCT1G125 | `EasyEDA:SOT-23-5_L3.0-W1.7-P0.95-LS2.8-BR` | C7484 | EasyEDA/`easyeda2kicad` |
| U6 | AT24C32M | `EasyEDA:SOP-8_L4.9-W3.9-P1.27-LS6.0-BL` | C2688466 | EasyEDA |
| U9 | TPS2121 | `EasyEDA:VQFN-HR-12_L2.5-W2.0-P0.50-BL` | C485916 | EasyEDA |
| U10 | SY8301ABC | `EasyEDA:SOT-23-6_L2.9-W1.6-P0.95-LS2.8-BL` | C2927130 | EasyEDA |
| U7 | SMTU2032-LF | `EasyEDA:BAT-SMD_SMTU2032-LF` | C18212226 | EasyEDA |
| L2 | FTC252010S4R7MBCA | `EasyEDA:IND-SMD_L2.5-W2.0-VLS252012CX-150M` | C5832360 | EasyEDA |
| F1, F2 | Sicherungen | `EasyEDA:F1206`, `EasyEDA:F1812` | C46641031, C20627123 | EasyEDA |
| D4, D5 | SMAJ5.0CA | `EasyEDA:SMA_L4.4-W2.6-LS5.0-BI` | C10756 | EasyEDA |
| LED1, LED4, LED5 | LTST-C171 | `EasyEDA:LED0805-RD` | C98224, C284871 | EasyEDA |
| RN1, RN2 | 4,7 kΩ ×4 | `EasyEDA:RES-ARRAY-SMD_0402-8P-L2.0-W1.0-BL` | C29586 | EasyEDA |
| SW1–SW3, SW5 | K2-1157SP-I4SW-01 | `EasyEDA:SW-SMD_4P-L6.2-W6.2-P4.50-LS8.6` | C2909692 | EasyEDA |
| H4–H7 | PZ254R-11-xxP | `EasyEDA:HDR-TH_{3,2,4,7}P-P2.54-H-M-W10.4` | C492411/10/12/15 | EasyEDA |

Die Footprints der Gruppe „EasyEDA“ sind im Format **KiCad 5** (`(module easyeda2kicad:…)`, `tedit 5DC5F6A4`) ✔ Datei — sie stammen vom Werkzeug
`easyeda2kicad` und nicht aus der Standardbibliothek. KiCad 10 liest sie, speichert sie beim Platinen-Update aber im neuen Format.

### 3.3 Passt der Footprint zum Gehäuse? Gegenproben gegen Datenblatt/Zeichnung

| Teil | Gegenprobe | Ergebnis |
|---|---|---|
| **ESP32-S3-WROOM-1** | Datenblatt v1.8, Bild 11-1 (S. 45): Randpads 40 × 1,5 × 0,9, Raster 1,27, Padreihenabstand 17,5, 16,51 über 13 Raster, unten Pads 15–26, EPAD 3 × 3 Felder je 0,9 mm im Raster 1,4 (3,7 gesamt) | ✔ DB/✔ Datei: Randpads, Raster, 17,5 und Unterreihe (x = ±6,985; Pad 14 bei y = 11,25 = −5,26 + 13·1,27) **stimmen**. Masse-EPAD im Footprint **voll 3,9 × 3,9 mm** (Kupfer) mit 0,2/0,6-mm-Thermal-Vias und neun 0,9-mm-**Paste**-Inseln. Das Datenblatt zeichnet als Kupfer die neun 0,9-mm-Felder (3,7 gesamt), also **0,2 mm kleiner und gegliedert**. Lage des EPAD (x = −1,5 mm gegen Modulmitte) ✔ per Pixelmessung der Abbildung (±0,1 mm). Die Abweichung ist für das Löten nicht als kritisch belegt ○; Pastenfenster gleich |
| **USB-C HRO TYPE-C-31-M-12** | Zeichnung (Kopie von LCSC C165948, nur Zeichnungsblatt, `hro-type-c-31-m-12.pdf`) | ✔ DB/✔ Datei: Signalpads 0,3 breit, Leistungspads 0,6, Raster 0,5, Reihenfolge der 12 Flächen **identisch** (A1/B12 · A4/B9 · B8 · A5 · B7 · A6 · A7 · B6 · A8 · B5 · B4/A9 · B1/A12), Positionslöcher Abstand 5,78 (Footprint 5,78, NPTH **Ø0,65** statt Zeichnung Ø0,60), Schirmschlitze Abstand 8,64/8,65 ✔. **Abweichungen:** Schlitze im Footprint 1,0 breit × 2,1 bzw. 1,6 lang, Zeichnung 0,6/0,9 breit × 2,0/1,7 lang (Footprint grosszügiger); Körperbreite 8,94 ✔. Die Zeichnung trägt Masslinie 5,79 mm „Padreihe bis Platinenrand“; der Footprint legt die Padreihe 7,7 mm hinter die Steckerfront (Fab-Linie): Er rechnet mit **rund 1,9 mm Überstand** über die Platinenkante — **nicht aus der Zeichnung bestätigt** ○, gehört zur Gehäuse-Prüfung (Mechanik, `usb-c-16p.md`). Der Footprint hat **kein Netz**; Pads übereinander (A1/B12 usw.) sind nur bei gleichem Netz fehlerfrei (Versuch 1.7) |
| **USBLC6-2SC6** | ST DB (Rev 5), S. 12, Bild 22 „SOT23-6L footprint“: Pad 0,60 breit, 1,10–1,20 lang, Rastermass 0,95, Aussenmass 3,50, Mitte–Mitte 2,30 | ✔ DB/✔ Datei: Standard-`SOT-23-6` hat Pads 1,325 × 0,6 bei x = ±1,1375 (Aussenmass 3,6, Mitte–Mitte 2,275) — **innerhalb 0,1 mm**, passt |
| **PMEG2010ER** | Nexperia DB, S. 9, Bild 15 (Reflow-Footprint SOD123W): Lötfläche 1,2 × 1,2, Paste 1,1 × 1,1, Schablone 0,1 mm | ✔ DB/✔ Datei: `Nexperia_CFP3_SOD-123W` Pads 1,2 × 1,2 bei ±1,4; Flächengrösse **gleich**; Mittenabstand aus der Zeichnung nicht eindeutig lesbar (nicht verglichen); Paste im Footprint 1,2 (Datenblatt 1,1) |
| **SI7149DP** (PowerPAK SO-8) | Vishay AN826 „Recommended minimum pads“, S. 15 (Rev. 21-Jan-08, in `si7149dp.pdf`): Gesamtbreite 6,61, Source-Pads 1,27 × 0,61, Drain-Pads 1,02 × 0,61, Mittelpad 3,81 × 3,91 | ✔ DB/✔ Datei: **Standard-`PowerPAK_SO-8_Single` stimmt exakt**. Der **verwendete Snapeda-Footprint** hat Source-Pads bei x = −2,81 (0,14 mm weiter aussen), Drain-Pads 6/7 nur **0,71 lang statt 1,02**, Pads 5/8 1,03, Mittelpad 3,81 × 3,91 bei x = 0,55; Gesamtbreite stimmt (6,61); **kein Courtyard**. Austausch gegen den Standard-Footprint geht **nicht** ohne Symbolwechsel: dort tragen die Pads 5–8 und die Fahne dieselbe Nummer „5“, im V3-Symbol hängen die Pins **1–9** an Netzen (✔ Netzliste) |
| **TPS2121** (RUX0012A) | TI SLVSEA3F, S. 39 „Example board layout“: Randpads 0,2 × 0,6, Raster 0,5, 2,3 Gesamthöhe, Innenpads 1,05 × 0,4 im Abstand 0,7 und 1,35 | ✔ DB/✔ Datei: Standard-`Texas_VQFN-HR-12_2x2.5mm_P0.5mm` **gleich** der TI-Zeichnung; der verwendete EasyEDA-Footprint ist **dieselbe Geometrie um 90° gedreht** (Pin-Zuordnung Punkt für Punkt gerechnet, Koordinaten stimmen) |
| **AT24C32M (SOP-8)**, **SY8301 (SOT-23-6)**, **SN74AHCT1G125 (SOT-23-5)**, **DS3231M (SOIC-8)** | — | nur Pad-Koordinaten aus der Datei gelesen (SOP-8 EasyEDA Pads 0,63 × 1,86 bei y = ±2,68, d. h. Aussenmass 7,2 gegen 6,9 beim Standard-SOIC-8); **keine Gegenprobe gegen die Datenblattzeichnung gemacht**. Die Datenblätter nennen Seiten mit Land-Pattern (`sy8301.pdf` S. 9–11, `sn74ahct1g125.pdf` S. 18/21/24, `ds3231m.pdf` S. 19) — **nicht gelesen** |
| **FTC252010S4R7MBCA** (L2) | `ftc252010s4r7mbca.pdf` (61 Seiten) — Land-Pattern-Seite nicht gefunden | Footprint heisst nach einem **anderen Teil** („…VLS252012CX-150M“, TDK); Pads **nicht** gegen das FTC-Datenblatt geprüft |
| **SMTU2032-LF** (U7), **K2-1157SP-I4SW-01** (SW), **PZ254R-11-xxP** (H), **F1206/F1812**, **SMAJ5.0CA**, **LTST-C171**, **RN 0402×4**, **MMBF170** | — | **nicht gelesen** (Datenblatt liegt hier nicht oder keine Land-Pattern-Seite gefunden). Der Footprint ist nur ein EasyEDA-Zeichnung des gleichen LCSC-Teils (die Quelle von V2 wie V3 — in V2 gebaut, ob mit Erfolg am Teil bestückt, weiss ich nicht ○) |

**Folgerung:** ESP32-S3-Modul, USB-C, USBLC6, SOD-123W, TPS2121 und PowerPAK sind belegt oder tragfähig; das **Massepad-Kupfer des Moduls** und die
**USB-Schirmschlitze** sind Abweichungen zum Entscheiden. Alle anderen EasyEDA-Footprints sind **nicht gegengeprüft**; die Bestückung bei JLC
prüft die Bauteil-Vorschau („Parts Placement“) und die DFM-Prüfung, nicht dieses Blatt.

### 3.4 Qualität der EasyEDA-Footprints, die im Entwurf stecken ✔ Datei

- **Kein Courtyard** in den geprüften EasyEDA-Footprints (SOT-23-6, SOT-23-5, SOP-8, VQFN-HR-12, F1206, SMA, LED0805, `courtyard False`) und im Snapeda-PowerPAK;
  nur der Taster (`SW-SMD…`) trägt einen. V2 stellt `missing_courtyard` auf **ignore**, das `courtyards_overlap`-Netz greift dort nicht.
- Pads liegen auf `F.Cu`, `F.Paste`, `F.Mask` (kein eigener Masken-/Pasten-Abzug), 3D-Modelle `${KICAD_3RD_PARTY}/EasyEDA.3dshapes/….wrl`
  (maschinenabhängiger Pfad).
- Referenz- und Wertetext liegen im alten Stil auf `F.SilkS` (Referenz bei y = ±5,15 mm, **weit ausserhalb** des Bauteils; 1 mm Schrift).
  Siebdruck-Meldungen in V2 (142 `silk_overlap`, 58 `silk_edge_clearance`) kommen aus diesem Umfeld ✔ Versuch (Zählung, Ursache ○).

### 3.5 Befunde am Entwurf (für `hw-designer`)

1. **R27, R30, R33 tragen `Capacitor_SMD:C_0805_2012Metric`** statt `Resistor_SMD:R_0805_2012Metric` ✔ Netzliste V3 und V2 (R27/R30 sind in V2 schon so,
   R33 ist ein Klon von R27 in `gen.py`, Zeile 120). Geometrie 0805 in beiden Bibliotheken; es ist ein **Bibliotheks-Name-Fehler**, der in
   Stückliste (Footprint-Spalte) und in der Prüfung `footprint_symbol_mismatch` (nur Warnung) steht.
2. **R17: LCSC-Feld leer** (Netzliste: `None`), alle anderen R/C haben eines ✔ Netzliste V3. Die Stückliste ist darum bei R17 unvollständig.
3. **R36, C34, C35 stehen als „DNP“** mit Footprint und LCSC-Feld; ob das Attribut „Do not populate“ im Footprint gesetzt wird (sonst bestückt JLC sie
   und das Bestückungsfile führt sie), ist im Schaltplan nicht geprüft — **nicht gelesen** ○.
4. Die beiden Massepads (EPAD 41) des Moduls und Pads `SH` des USB-C-Steckers hängen in der V3-Netzliste an GND ✔ Netzliste (Pad 41, `SH`).
5. Die Netzliste bildet U2-Pins 4, 15, 16, 18–26, 28–35 als „unconnected“ ab (✔ Netzliste); **No-Connect-Flags** setzt der `hw-designer`, hier nicht geprüft.

### 3.6 JLC-Teile nach KiCad bringen: Werkzeuge ● GitHub-API/README (17.10.2026)

| Werkzeug | Lizenz | Stand (letzter Push / Release) | Leistung | Warnung des Herstellers |
|---|---|---|---|---|
| **easyeda2kicad** (`uPesy/easyeda2kicad.py`) | **AGPL-3.0** | 08.04.2026 / v1.0.1 (06.04.2026) | Symbol + Footprint + 3D (WRL/STEP) aus LCSC-Nummer, KiCad ≥ 6, `--full --lcsc_id=C…` | ● README: „Richtigkeit … nicht garantiert … immer Footprints und Symbole doppelt prüfen“ |
| **JLC2KiCadLib** (`TousstNicolas/JLC2KiCad_lib`) | **MIT** | 11.09.2026 / v1.3.2 | Symbol, Footprint, 3D; erzeugt via KicadModTree | ● README: „be careful and always check the output footprint and symbol“ |
| **KiCad-JLCPCB-Tools** (`Bouni/kicad-jlcpcb-tools`) | **MIT** | 14.10.2026 / 2026.04.03 | **nicht Footprint-Erzeugung**: Teilesuche in der JLC-Datenbank, LCSC-Feld setzen, Stückliste/CPL/Gerber, **Rotations- und Versatzkorrekturen** | ● README |
| **Fabrication Toolkit** (`bennymeg/Fabrication-Toolkit`) | **Apache-2.0** | 29.09.2026 / 5.3.1 | Gerber + Bohrung + BOM + `positions.csv` für JLC; V2 benutzt es (`fabrication-toolkit-options.json`) | ● README |
| JLCKicadTools (`matthewlai/JLCKicadTools`) | GPL-3.0 | 01.05.2025 | Ursprung der `rotations.cf`-Korrekturen, älter | ● GitHub-API |

Auf diesem Mac **installiert** (✔ Datei, `~/Documents/KiCad/10.0/3rdparty/plugins/`): Fabrication Toolkit (`com_github_bennymeg_JLC-Plugin-for-KiCad`),
KiCad-JLCPCB-Tools (VERSION **2026.04.03**), das Plugin `impartGUI` (Steffen-W; enthält `easyeda2kicad`, vermutlich die Quelle der `EasyEDA`-Bibliothek ○),
PCBWay-Plugin und `ViaStitching`/`CircularZone` aus `kicad-action-scripts`. Eine Kopie des Fabrication-Toolkit-Codes **liegt auf der Platte**, ich habe sie gelesen.

**Lizenzfolge:** Die Footprint-Dateien selbst entstehen aus dem LCSC/EasyEDA-Katalog; die Lizenz des **Werkzeugs** (AGPL bei `easyeda2kicad`) betrifft nur den Werkzeug-Code,
nicht automatisch dessen Ausgabe. Rechtlich geprüft ist das nicht ○ — für ein privates Projekt kein Hindernis, für ein veröffentlichtes Repo klären.
Die Standardbibliothek von KiCad steht unter CC-BY-SA 4.0 mit Ausnahme (Bauteilverwendung frei) — **nicht gelesen**, nur Erinnerung ○, nicht zu verwenden.

**Empfehlung (Qualität):** Wo die Standardbibliothek den **gleichen Gehäusetyp** hat und die Pins zum Symbol passen (SOT-23-6/-5, SOIC-8, SMA, 1206/1812-Sicherung,
LED 0805, Fiducial), den Standard-Footprint vorziehen (Courtyard, 3D-Modell, geprüfte Geometrie, kein Pfadproblem). EasyEDA-Footprints nur dort behalten, wo das
Teil in der Standardbibliothek fehlt (SMTU2032, K2-Taster, Header PZ254R) — und jeden gegen die Zeichnung gegenprüfen.

### 3.7 Rotation und Ursprung für die Bestückung (CPL) ✔ Datei/Doku, ● README

**Das Problem:** KiCad-Footprints und JLCs interne Bauteildaten haben nicht dieselbe Null-Lage; die Plugins korrigieren.
- **Fabrication Toolkit** ● README: „Rotation … stimmt nicht immer … meist über `rotations.cf`“ (im Plugin heisst die Datei hier **`transformations.csv`**, 51 Zeilen ✔ Datei);
  Ausnahmen über das Footprint-Feld **`FT Rotation Offset`** (Grad, positiv = gegen den Uhrzeigersinn) und **`FT Position Offset`** (x,y in mm), Ursprung über **`FT Origin`**
  (`Anchor` = Footprint-Anker, `Center` = Mitte der Pad-Hüllkurve; Standard automatisch), Lage über **`FT Layer Override`**. Eine Pfeiltaste in der JLC-Vorschau = 0,0635 mm.
  Alle diese Felder liegen **im Schaltplan**, danach „PCB aktualisieren“ (F8) ● README.
- **KiCad-JLCPCB-Tools** ● README: eigener *Corrections Manager* (CSV `Pattern,Rotation,Offset X,Offset Y`; Treffer für Referenz vor Wert vor Footprint, längster Treffer gewinnt) — das ist die **zweite**, unabhängige Korrekturstelle.
  **Nie beide gleichzeitig** nutzen, sonst doppelte Korrektur (○ aus Vergleich der Funktionen, nicht ausprobiert).

**Gelesen in der installierten Fassung des Fabrication Toolkit** (`process.py`, `transformations.csv`): Die Korrekturen suchen per **`re.search` auf den reinen Footprint-Namen**
(ohne Bibliothek). Die Tabelle enthält u. a. `^SOT-23` → 180°, `^SOP-(?!18_)` → 270°, `^SOIC-` → 270°, `^QFN-` → 90°, `^PowerPAK_SO-8_Single` → 270°;
**keine Regel für** `USB_C_Receptacle…`, `Texas_VQFN-HR…`, `VQFN-HR-12_L2.5…`, `ESP32-S3-WROOM-1`, `Nexperia_CFP3_SOD-123W`, `TO-92_Inline`.

**Folge, aus den V2-Fertigungsdaten belegt** (V2 `.kicad_pcb` gegen `production/positions.csv`, Rotation je Referenz; ✔ Versuch):
| Teil in V2 | KiCad-Drehung | `positions.csv` | Wirkung |
|---|---|---|---|
| Q2, Standard `SOT-23` | 180° | 0° | Korrektur +180° (Regel `^SOT-23`, gewollt) |
| U3, **EasyEDA** `SOT-23-5_L3.0…` | 0° | 180° | **dieselbe Regel `^SOT-23` trifft den EasyEDA-Namen** |
| U10, **EasyEDA** `SOT-23-6_L2.9…` | 90° | 270° | **ebenso** |
| U6, **EasyEDA** `SOP-8_L4.9…` | 0° | 270° | `^SOP-(?!18_)` |
| U2, STM32 `QFN-48…` | 0° | 90° | `^QFN-` |
| U1, `TO-92_Inline` (Unterseite) | 0° | 180° | Standard der Unterseite (Spiegelung) |
| `SW`, `LED`, `SMA`, `PowerPAK` (Snapeda), `VQFN-HR` (EasyEDA), `SOIC127P600…` | jeweils | unverändert | keine Regel |

Ob die Korrektur auf **EasyEDA-Footprints** richtig ist, ist **nicht belegt** ○: Diese Footprints stammen aus JLC-Daten, die Korrektur war für die KiCad-Standardnamen gedacht;
möglicherweise dreht sie sie falsch. **V2 lieferte diese Daten; ob die V2-Platine bei JLC richtig bestückt wurde, weiss ich nicht** — der Nutzer kann es sagen. Vor der V3-Bestellung
für **jedes** gepolte oder gerichtete Teil die JLC-Bauteil-/DFM-Vorschau gegen die Platine prüfen (Pin 1). Besonders gefährdet: U2 (Modul, keine Regel, Standard-Footprint,
Orientierung bei JLC ○), USB1 (keine Regel, Stecker muss zur Platinenkante zeigen), U9 (`VQFN-HR`, keine Regel), D2/D3 (Kathode).

## 4. Platzierung und Mechanik in KiCad

### 4.1 Antennen-Sperrfläche (Rule Area) ✔ Datei, ✔ Versuch, ✔ Doku

Der Standard-Footprint `ESP32-S3-WROOM-1` **enthält eine Rule Area**: Rechteck x = −24 … +24, y = −27,75 … −6,75 (Footprint-Koordinaten, Ursprung = Modulmitte),
**alle Kupferlagen**, `tracks`, `vias`, `pads`, `copperpour` (Zonen) und `footprints` **nicht erlaubt** ✔ Datei (`ESP32-S3-WROOM-1.kicad_mod`). Das ist **48 × 21 mm**:
Modul 18 mm breit, Oberkante bei y = −12,9; die Fläche beginnt also 6,15 mm unterhalb der Moduloberkante, reicht **15 mm seitlich** über das Modul hinaus und 14,85 mm über die
Oberkante hinaus. Dasselbe Rechteck ist der **Courtyard** der Antennenseite (Linie `F.CrtYd` bei y = −27,75/−6,75, x = ±24) ✔ Datei.
**Die Sperre wirkt im DRC wirklich:** In der V2-Kopie mit dem S3-Modul an der Stelle von `U4` (x = 127, Oberkante an der Platinenkante) meldet der DRC `items_not_allowed`
(„Sperrfläche von U2“) für Bahnen, Vias, die GND-Zone, den Taster `SW4`, den Stecker `H6` und die Pads von `H6` sowie 7 × `courtyards_overlap` (H6, SW1, R12, R14–R16, R18) ✔ Versuch
(Gegenprobe der Reichweite; die V2-Reste sind V3 teilweise fremd). Die Sperre schneidet also **H6** (V3: 103,75/4,50 laut Skill-Liste) und trifft die GND-Fläche oben rechts.
**Ob die Fläche so gross sein muss, ist eine Frage an `layout-regeln.md`/Espressif-HDG, nicht an KiCad.** Möglich ist, die Rule Area im Footprint-Editor zu verkleinern
(eigene Kopie des Footprints), **dann aber ohne die Belege der Standardbibliothek**; die Beschreibung („6 mm Antennenbereich“ ✔ DB S. 45, Bild 11-1) und der 1:1-Zwang der
Stecker (H6 an Ort und Stelle) stehen gegeneinander. **Entscheid beim Architekten.**

Zonen **respektieren** Rule Areas mit „Zonen nicht erlaubt“ beim Füllen automatisch ✔ Doku („Rule areas (keepouts)“). Rule Areas lassen sich mit Namen versehen; so kann eine
Custom-Regel sie benutzen (`A.intersectsArea('Antenne')`).

### 4.2 Kupferzonen, Thermals ✔ Doku („Working with zones“, Pad connection) und ✔ Datei (V2)

- Zonen werden von Hand (Taste `B`) gefüllt; **vor DRC und Fertigungsdaten neu füllen** (KiCad warnt, wenn ungefüllt). Fabrication Toolkit hat die Option „Apply automatic fill for all zones“ ● README.
- Priorität: höhere Priorität füllt zuerst, niedrigere hält Abstand. Zonen füllen nie ausserhalb des Umrisses oder in Aussparungen.
- Anbindung: *solid*, *thermal reliefs*, *reliefs for PTH* (SMD solid), *none*. Thermal-Parameter `thermal_relief_gap`, Speichenbreite; `min_resolved_spokes` (V2: 2) löst bei zu wenig Speichen eine DRC-Meldung (`starved_thermal`, in V2 als Fehler) aus.
- **V2:** eine GND-Zone auf F.Cu und B.Cu, Priorität 0, Abstand 0,25, Mindestbreite 0,25, Thermal 0,25/0,25; daneben vier weitere F.Cu-Zonen (Zählung ✔ Datei; Zweck ohne Netzname nicht
  zugeordnet ○). Der ESP-Footprint legt für das EPAD `zone_connect 2` (= Thermal) auf **allen** Pads fest (`(zone_connect 2)` ✔ Datei); eine solide Anbindung des Modul-EPAD an die Masse
  ist für den Wärmepfad und die Löt-Reflow-Wärme eine Entscheidung des Layouters.
- Per Custom-Regel: `(constraint zone_connection solid)` je Bedingung, `thermal_relief_gap`, `thermal_spoke_width` ✔ Doku.

### 4.3 Teardrops, Via-Stitching ✔ Doku

- **Teardrops** (*Edit → Edit Teardrops*): für PTH-Pads, SMD-Pads, Vias, Bahn-zu-Bahn; Parameter „Bestlänge“, „Bestbreite“, `Max_Length/Max_Width`, gekrümmt; sie sind **kleine Zonen** und werden bei jedem „Zonen neu füllen“, DRC und Fertigungsausgabe neu berechnet.
  Dokumentierter Zweck: mechanische Robustheit und Schutz gegen Bohrversatz. V2 hat Vorgaben (`td_onsmdpad`, `td_onpthpad`, `td_onvia` wahr, Länge 0,5, Höhe 1,0, max 1,0 × 2,0 ✔ Datei); ob V2 sie am Layout **angewendet** hat, ist
  nicht geprüft (`legacy_teardrops no`). Teardrops auf 0,5-mm-Pads des USB-Steckers erst nach Prüfung des Abstands setzen ○.
- **Via-Stitching:** Das Werkzeug *Freie Vias* (`Strg+Umschalt+X`) setzt Vias, die das Netz der darunterliegenden Zone übernehmen; „Netz automatisch aktualisieren“ ist bei freien Vias aus ✔ Doku.
  Für Sticheln im Raster gibt es das Aktionsplugin `ViaStitching` (installiert ✔ Datei; Qualität nicht geprüft). Die **Abstände** (Raster, Abstand zu Antenne/USB) stehen in `layout-regeln.md`.

### 4.4 Fiducials, Beschriftung, Siebdruck

- **Fiducials:** KiCad hat 10 Standard-Footprints `Fiducial:Fiducial_*` (z. B. `Fiducial_1mm_Mask2mm`) ✔ Versuch (`ls`). Als Pad-Eigenschaft `Fiducial, global to board` / `local to footprint`
  (wirkt nur auf Gerber-X2-Attribute) ✔ Doku („Fabrication property“). Ob **JLC für diese Platine Fiducials verlangt oder selbst Randmarken setzt**, steht in `jlcpcb-fertigung.md`; hier nicht beantwortet.
- **Siebdruck:** V2 hat Texthöhe 0,8 mm/Strich 0,15 mm (`min_text_height`, `min_text_thickness` und Regel `Minimum Text`), Siebdrucklinie 0,15, Pad-zu-Siebdruck 0,15, aber `min_silk_clearance 0,0` im Setup. Die Schrift der EasyEDA-Referenzen
  (3.4) ist 1,0 mm. Die Mindestwerte für JLC siehe `jlcpcb-fertigung.md`.
- **Beschriftung (Pin-1-Markierung, Polarität):** Die Prüfung gegen die JLC-Vorschau (3.7) bleibt die massgebende.

### 4.5 Edge.Cuts, Umriss, Befestigung ✔ Datei/Versuch

- V2-Umriss: **12 Linien auf Edge.Cuts**, Bounding Box **145,05 × 39,05 mm** (inkl. 0,05 mm Linienbreite) ✔ Versuch. Der Umriss muss **geschlossen** sein (`invalid_outline` ist Fehler ✔ Datei).
  Stecker-Aussparung und Antennenfläche: nicht aus dem V2-Umriss herauszulesen (nur Linien, keine Bögen gezählt); nicht ausgewertet.
- **Bohrungen für M2,5:** `MountingHole_2.7mm_M2.5_Pad` (H8–H10) und `MountingHole_2.2mm_M2_DIN965_Pad` (H1/H2) ✔ Datei; die Pad-Varianten liefern Kupferring (Masse/Schraubkontakt); `_TopOnly`/`_TopBottom` existieren ✔ Versuch.
- `copper_edge_clearance` (V2: Fehler) und die V2-Regel 0,3 mm bis Rand: Das Modul in der Versuchsplatine löste an den Pads 1 und 40 `copper_edge_clearance` aus (Ursache nicht geklärt, im V2-Umriss
  liegt vermutlich ein Innenschnitt ○). Beim Platzieren des Moduls an der Oberkante zuerst den Rand prüfen.

### 4.6 3D-Modelle / STEP

- **V2 trägt eine STEP-Datei** (`V2.step`, im Projektordner ✔ Datei). Zuletzt gespeicherter Exportpfad `V2.step` (`pcbnew.last_paths.step` in `V2.kicad_pro`).
- Der Standard-Footprint des Moduls verweist auf `${KICAD10_3DMODEL_DIR}/RF_Module.3dshapes/ESP32-S3-WROOM-1.step` ✔ Datei; **V2 verwendet noch `${KICAD9_3DMODEL_DIR}/…`** (96 Modelle, die ersten genannt)
  — die Variable `KICAD9_3DMODEL_DIR` ist in KiCad 10 nicht belegt, V2 würde dort ohne Modell erscheinen (○ nicht geprüft; in KiCad 10 setzt es vermutlich die Alias-Variable). EasyEDA-3D-Modelle sind **WRL**
  (grosse Dateien, keine STEP), liegen unter `${KICAD_3RD_PARTY}/EasyEDA.3dshapes`.
- Zweck der STEP: Gehäuse-Passprüfung (USB-C-Öffnung 8,94 × 3,26 mm? ○ Mass nicht aus STEP gelesen). Export per `kicad-cli pcb export step` (Befehl unterstützt `--subst-models`) — **nicht ausprobiert** ○.

## 5. Typische Fehlerquellen bei KiCad-Layouts, die in Reviews auffallen

Jeweils mit Beleg am eigenen Material, soweit vorhanden.

| Fehlerbild | Beleg / Gegenmassnahme |
|---|---|
| **Regel schlägt bei Bauteil an, das sie nicht meinen sollte** (Pad Size gegen Thermal-Vias, TO-92, Schirmpads) | ✔ Versuch 1.7; Regel mit `Fabrication_Property`/`Pad_Type` einschränken |
| **Offene Netze durch Footprint-Namen/Pin-Nummern-Diskrepanz** (Symbol ↔ Footprint) | `PowerPAK` Standard vs. Snapeda (3.3); DRC `footprint_symbol_mismatch` ist in V2 nur Warnung → auf **error** setzen ○ |
| **Footprint-Bibliotheks-Mismatch** (Widerstand mit C-Footprint) | R27/R30/R33 (3.5) |
| **Fehlendes Courtyard** → keine Überlappungsprüfung | `missing_courtyard` in V2 `ignore`; EasyEDA-Footprints ohne Courtyard (3.4) → auf **warning** und Courtyard ergänzen |
| **Pad-Maske und Paste** | Global leer lassen (Doku: Fertiger nimmt Null, „Most commercial PCB fabricators expect these values to be zero“ ✔ Doku „Solder mask/paste“); V2: `pad_to_mask_clearance 0`, Tenting vorne/hinten **ja**. **Lötstoppsteg:** Meldung `solder_mask_bridge` entsteht, wenn zwei Pads **verschiedener Netze** gemeinsame Öffnung haben (✔ Versuch beim USB-Stecker ohne Netze; mit Netzen weg) → Stecker-Pads vor dem DRC an Netze legen. JLC-Mindeststeg: `jlcpcb-fertigung.md` |
| **Ungleiche Paste** (EPAD voll bepastet) | Das S3-Modul ist belegt richtig: neun Pasteninseln statt Vollfläche ✔ Datei/✔ DB S. 45. Beim PowerPAK: Snapeda-Footprint ohne belegte Pastenaufteilung (Mittelpad 3,81 × 3,91, voll) ○ Pastenmenge am Footprint prüfen |
| **Pads mit zu kleinem Restring** | TO-92 (Pad 1,05 × 1,5, Bohrung 0,75: 0,15) und USB-Schirmschlitz (0,2) ✔ Versuch → Schwelle aus JLC-Liste, Footprint-Eigenschaft anpassen oder Ausnahme begründen |
| **DRC-Ausnahmen** | `drc_exclusions` in `.kicad_pro` (V2: leer ✔ Datei) — **jede Ausnahme mit Kommentar im Review dokumentieren**; Meldungen nicht per `ignore` in `rule_severities` verstecken (V2 ignoriert `missing_courtyard`, `footprint_filters_mismatch`, `npth_inside_courtyard`, `pth_inside_courtyard` ✔ Datei) |
| **Nicht aktuelle Kupferfüllung** | vor DRC/Export neu füllen; Fabrication Toolkit füllt auf Wunsch selbst (● README) |
| **Siebdruck über Pads / am Rand** | V2: 142 + 58 + 8 Meldungen (✔ Versuch), `silk_*` stehen nur auf Warnung → vor Abgabe auf Null oder Ausnahmen mit Grund |
| **Rotation/Ursprung in der Bestückungsdatei** | 3.7: Korrekturen gelten auch für EasyEDA-Footprints; jedes gerichtete Teil in der JLC-Vorschau prüfen |
| **Antennen-Sperre nicht beachtet / H6 darin** | 4.1 |
| **Bahn endet im Nichts** (`track_dangling`) | V2 hat 2 solche Stummel (0,237 mm VCC_3.3V und 0,0257 mm SCL) ✔ Versuch → aufräumen |
| **Netzklassenwerte nur Vorgabe** | 1.5; Erzwingen nur per `track_width`/`via_diameter`-Regel |
| **Zonenfüllung gegen Fehlalarm bei `isolated_copper`** | in V2 Warnung; Inseln entfernen oder mit Stitching an Masse legen ○ |
| **`inDiffPair`-Namen** | siehe 1.5 (`…DP/DN` nicht ausprobiert) |

## 6. Offene Punkte (nicht gelesen oder nicht ausprobiert)

1. `inDiffPair()` mit den Netznamen `/USB_DP`/`/USB_DN` und `/USB_DP_M`/`/USB_DN_M` am DRC erproben (absichtlicher Fehler, DIR-014).
2. Land-Pattern-Gegenproben für AT24C32M, SY8301, SN74AHCT1G125, DS3231M, FTC252010S4R7MBCA, SMTU2032-LF, K2-Taster, PZ254R-Header, SMAJ5.0CA, MMBF170, LTST-C171, F1206/F1812 — nicht gelesen.
3. Ob die V2-Platine bei JLC mit den korrigierten Rotationen richtig bestückt wurde (Nutzer fragen); Bauteilorientierung des S3-Moduls (C2913198) und des USB-C (C165948) in JLCs Vorschau.
4. Wie sich `KICAD9_3DMODEL_DIR` in KiCad 10 verhält; `kicad-cli pcb export step` auf der V3-Platine.
5. Ob die Antennen-Rule-Area des Standard-Footprints verkleinert werden darf (Architektenentscheid, abhängig von `layout-regeln.md`).
6. Lizenzfragen bei `easyeda2kicad` (AGPL) und der Standardbibliothek: nicht gelesen.
7. JLC-Grenzwerte (Restring, Bohrung 0,2 mm, Maskensteg, Abstand zum Rand) — stehen in `jlcpcb-fertigung.md`, hier nicht nachgeprüft; die V2-Zahlen sind PCBWay-Zahlen.
