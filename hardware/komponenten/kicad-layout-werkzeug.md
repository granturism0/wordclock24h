# KiCad-Layout automatisiert erzeugen und prüfen — Werkzeug-Machbarkeit

Stand: 17.10.2026. Verfasser: `hw-datasheet-analyst`. Gegenstand ist kein Bauteil, sondern das Werkzeug,
mit dem der `hw-layouter` aus `WordClock-ESP32S3-V3.kicad_sch` eine `.kicad_pcb` macht.

**Beleg-Zeichen in diesem Blatt**

| Zeichen | Bedeutung |
|---|---|
| ✔ | selbst ausprobiert, Befehl und Ausgabe stehen dabei (Rechner: macOS 27.0.1, arm64, KiCad 10.0.2) |
| ✔Q | Dokumentation gelesen, URL steht dabei (über `WebFetch`, also vom Abrufwerkzeug zusammengefasst, nicht Wort für Wort gelesen) |
| ● | Sekundärquelle (Forum, Suchergebnis) |
| ○ | Annahme, nicht verwenden |

Alle Versuche liefen im Scratchpad `…/scratchpad/layout-probe/` (Unterordner `a` bis `e`) an **Kopien**. Das V2-Projekt
und das Repo wurden nur gelesen; die Datei hier ist die einzige geschriebene im Repo. (Ein Fehlgriff: Ein relativer Pfad
legte kurz ein Verzeichnis `c/` im Repo-Wurzelverzeichnis an; es enthielt nur meine Probedateien und ist wieder gelöscht,
`git status` ist sauber.)

## 1. Welche Wege gibt es

| Weg | Befund | Zeichen |
|---|---|---|
| **pcbnew-Python (SWIG)** | In KiCad 10.0.2 vorhanden und gebündelt: `/Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/3.9/bin/python3`, Modul `pcbnew` liegt in `…/lib/python3.9/site-packages/pcbnew.py` + `_pcbnew.so`. `import pcbnew; pcbnew.Version()` gibt `10.0.2`. Läuft **ohne GUI**. | ✔ |
| SWIG-Zukunft | „deprecated as of KiCad 9.0 … The current plan is to remove the SWIG bindings in KiCad 11.0.“ Die IPC-API ist der empfohlene Ersatz. Eine Forensuche nennt zusätzlich, KiCad 11 bündle kein Python mehr (nur Suchtreffer, nicht belegt). | ✔Q https://dev-docs.kicad.org/en/apis-and-binding/pcbnew/ ; ● https://forum.kicad.info/t/kicad-10-99-win-missing-python-exe/71638 |
| **IPC-API / kicad-python (`kipy`)** | „remotely control a **running instance** of KiCad“, Protobuf über NNG-Socket, ein Thread, Anfragen können in die Zeitüberschreitung laufen. Kein Hinweis auf Headless-Betrieb. `kipy` ist hier **nicht installiert** (`python3 -c "import kipy"` → `ModuleNotFoundError`). Nicht probiert, weil es eine laufende GUI braucht — und der Nutzer hat V2 gerade geöffnet (Sperrdatei `~WordClock-…V2.kicad_pro.lck` am 17.10. 01:26). | ✔Q https://dev-docs.kicad.org/en/apis-and-binding/ipc-api/index.html ; ✔ (Import-Test) |
| **`kicad-cli pcb …`** | Unterbefehle: `drc`, `export`, `import`, `render`, `upgrade`. `export`: 3dpdf, brep, drill, dxf, gencad, gerbers, glb, ipc2581, ipcd356, odb, pdf, ply, pos, ps, stats, step, stl, stpz, svg, u3d, vrml, xao (`hpgl` „no longer supported as of KiCad 10.0“). **Kein** Befehl für „Update PCB from Schematic“, kein DSN-Export, kein Routen. | ✔ (`kicad-cli pcb export` ohne Argument, `pcb --help`) |
| Direkte Dateierzeugung (S-Ausdruck) | Möglich (das PCB ist Text, `version 20260206`, `generator_version "10.0"`), aber nicht nötig: `pcbnew.SaveBoard` schreibt dasselbe Format und erhält fremde Inhalte (siehe 2a). Nicht probiert, **nicht empfohlen**: kein Vorteil, aber jede Syntaxabweichung ist ein stiller Fehler. | ○ |
| **Autorouter Freerouting** | GitHub-Release **v2.5.0**, veröffentlicht 05.10.2026, Lizenz **GPL-3.0**. Befehlszeile `-de <in.dsn> -do <out.ses>` (u. a. `-mp` Durchläufe, `-mt` Threads, `-inc` Netzklassen ignorieren). Es gibt `freerouting-2.5.0.jar` (Java 25 verlangt, laut README) und **`freerouting-cli-2.5.0-macos-arm64.tar.gz`** (20,9 MB, GraalVM-Native, laut Release-Text „without a bundled JRE … eliminates the Java runtime requirement“). Release-Text: KiCad-10-IPC-Integration neu, Rückfall auf DSN/SES. | ✔Q https://github.com/freerouting/freerouting ; https://github.com/freerouting/freerouting/releases/tag/v2.5.0 ; ● Argumentliste: Suchergebnis zur Freerouting-CLI |
| **Plugin Fabrication Toolkit** | Erzeugt Gerber, BOM, Pick-and-Place, IPC-Netlist für JLCPCB; hat eine Befehlszeile `python3 -m plugins.cli -p <board>`; Lizenz Apache 2.0; Hinweis „Ensure your board is synchronized [F8]“. Hier lokal als `~/Documents/KiCad/10.0/3rdparty/plugins/com_github_bennymeg_JLC-Plugin-for-KiCad/` installiert (Versionsnummer nicht ermittelbar: kein `metadata.json` im Ordner). | ✔Q https://github.com/bennymeg/Fabrication-Toolkit ; ✔ (Ordner, `cli.py` gelesen) |

**Stolpersteine, die ich gefunden oder belegt habe**

1. **SWIG-Lärm und Aufräumen.** `board.Remove(x)` zerstört in der Python-Hülle das Objekt: Danach lieferte `GetFootprints()`
   Rohobjekte (`'SwigPyObject' object has no attribute 'GetReference'`). **`board.RemoveNative(x)`** funktioniert. ✔ (Fehler und Abhilfe selbst erlebt)
2. `Duplicate()` verlangt `Duplicate(False)` (Pflichtargument `addToParentGroup`); `RemoveUnusedNets(None)` verlangt das Argument `aCommit`;
   Footprint aus einer Bibliothek (`FootprintLoad`) trägt **keinen Bibliotheksnamen** in der FPID, `SetFPID(LIB_ID(nick, name))` ist nötig, sonst meldet die Paritätsprüfung `footprint_symbol_mismatch`. ✔
3. Beim Laden/Beenden schreibt `pcbnew` Zeilen wie `assert ""traits"" failed … create wxApp`, `Debug: Adding duplicate image handler`
   und `swig/python detected a memory leak of type 'PCB_TRACK *'`. Alle harmlos; filtern mit
   `2>&1 | grep -v -e Fontconfig -e wxApp -e stdpbase -e Debug -e "memory leak"`. ✔
4. **`pcbnew.SaveBoard("x.kicad_pcb")` legt daneben `x.kicad_pro` und `x.kicad_prl` an** (aus dem Namen des Ziels; ohne Vorlage leer/Standard).
   Das Layout-Verzeichnis braucht die **Projektdatei mit den Netzklassen** und `.kicad_dru` unter **demselben Basisnamen**, sonst rechnet der DRC mit Standardwerten (siehe `hw-layouter.md`: 504 falsche Fehler). ✔ (`moved.kicad_pro` entstand von selbst)
5. **Der Dateiname ist im PCB verankert** (`"filename"` in der `.kicad_pro`) und `--schematic-parity` sucht den Schaltplan **gleichen Basisnamens** im selben Ordner. ✔ (V3.kicad_sch neben V3.kicad_pcb)
6. `--exit-code-violations` gibt am **V2-Stand selbst Exit 5** zurück (241 Meldungen). Der Exit-Code taugt nur gegen die Baseline, nicht roh. ✔ (`kicad-cli pcb drc --exit-code-violations -o /dev/null V2.kicad_pcb` → `exit=5`)
7. **Netznamen: V3 trägt Schrägstriche, V2 nicht.** Der V3-Netzlistenexport nennt Netze `/ALT_5V_INPUT`, `/DCF77`, `/BOOT`, … (lokale Bezeichner), V2 hiess `ALT_5V_INPUT`. Nur `GND`, `VCC_3.3V`, `VCC_5V` (Leistungssymbole) haben keinen. Die Kupferzonen im V2-PCB hängen an den **alten** Namen (`ALT_5V_INPUT`, `TO_VCC_RGB_5V`, `Net-(F2-Pad2)` …) und treffen ohne Umbenennung kein Netz. Mein Skript bildet sie auf `/<name>` ab. **Das ist eine Rückmeldung an `hw-designer`**: Entweder gleiche Namen wie V2 (globale Bezeichner) oder bewusst mit Schrägstrich — dann muss das Layout damit leben. Für Programme, die Netznamen gegen V2 vergleichen (`vergleich.py`, `expected.json`), ist es ohnehin relevant. ✔ (v3.xml vs v2.xml verglichen)
8. **Footprint-Wechsel im Schaltplan ändern das Layout still.** Gegenüber V2 hat V3 für `D2`, `D3` (EasyEDA SOD-123 → `Diode_SMD:Nexperia_CFP3_SOD-123W`), `C20`, `C22` (EasyEDA CAP-SMD_L2.0-W1.2 → C_0805), `R11`, `R13`, `R17` (C_0805 → R_0805), `U2` (QFN-48 → ESP32-S3-WROOM-1) und `USB1` (6P → HRO TYPE-C-31-M-12) **andere Footprints**. Ob das beabsichtigt ist, muss der Designer sagen; geometrisch ist nur `D2`/`D3`, `U2`, `USB1` verschieden. ✔ (Vergleich v2.xml/v3.xml)
9. Auch in V3 tragen `R33` (510 Ω) und andere Widerstände den Footprint `Capacitor_SMD:C_0805…`: V2 hatte dasselbe bei R11/R13/R17/R27/R30. Gleiche Geometrie, aber BOM-Auswertung nach Footprint-Namen führt in die Irre. ✔ (Lesen; nicht bewertet)

## 2. Machbarkeitsprobe, Schritt für Schritt

Abkürzungen für die Befehle:

```
CLI=/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli
PY=/Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/3.9/bin/python3
```

(zsh: eine Variable mit mehreren Wörtern wird **nicht** aufgeteilt; `for c in "pcb drc"; do $CLI $c` scheitert. `${=c}` oder Bash nutzen.)

### (a) V2-PCB laden, auflisten, verschieben, speichern, prüfen — **gelingt** ✔

```
cp "<V2>/…V2.kicad_pcb" a/V2.kicad_pcb   (ebenso .kicad_pro, .kicad_dru, umbenannt auf gleichen Basisnamen V2)
$PY list.py V2.kicad_pcb        # b=pcbnew.LoadBoard(...); b.GetFootprints()
→ 109 footprints 1006 tracks 101 nets
```

V2 hat 109 Bauteile, 1006 Bahnen+Vias (340 Via-Einträge im Text), 23 Zonen-Zeilen im Text (10 Zonen), 39 Grafikelemente. Jede Zeile liefert Referenz, Footprint, X/Y in mm (absolute Seitenkoordinaten; die Platine beginnt bei (81,5 / 76,75), also `USB1` abs. (154,00 / 112,13) = relativ (72,50 / 35,38) wie im Skill), Drehung, Lage, gesperrt.

Verschieben von `TP8` um +1 mm in X, speichern, DRC:

```
f=b.FindFootprintByReference("TP8"); f.SetPosition(VECTOR2I(x+FromMM(1.0), y)); pcbnew.SaveBoard("moved.kicad_pcb", b)
$CLI pcb drc --severity-all --format json -o drc_orig.json V2.kicad_pcb     → 241 Verstöße, Unverbundene Elemente (1)
$CLI pcb drc … moved.kicad_pcb (mit moved.kicad_pro/.kicad_dru daneben)       → 248 Verstöße, Unverbundene Elemente (2)
```

- **Die 241 stimmen mit `tools/checks/hardware-drc.baseline` überein** (142+58+14+9+8+5+2+2+1 = 241; die Baseline führt `unconnected_items: 1` getrennt). Der DRC-Lauf dauert rund 1,6 s.
- Neu durch die Verschiebung: `clearance` +2, `courtyards_overlap` +1, `solder_mask_bridge` +1, `silk_*` +3, `unverbunden` +1: der DRC **sieht** die Änderung.
- **Speichern ist verlustfrei**: `diff` der beiden Dateien zeigt 8576 Zeilen, aber nur weil `SaveBoard` die Bahnen in anderer Reihenfolge schreibt; `diff <(sort V2) <(sort moved)` zeigt genau die **eine** geänderte `(at …)`-Zeile. Dateigrösse byte-gleich (1'271'885). Eine reine Reihenfolgeänderung macht `git diff` unlesbar — Vergleiche daher nie per Zeilen-Diff, sondern per Skript (Abschnitt 4).

### (b) Netzliste → PCB („Update PCB from Schematic“) — **kein CLI/API-Weg, Ersatz per Skript gelingt** ✔

Ehrliche Antwort: **Es gibt keinen skriptbaren Weg.**
- `kicad-cli pcb` hat keinen solchen Unterbefehl (siehe oben), `kicad-cli sch export` auch nicht (nur bom, dxf, hpgl, netlist, pdf, ps, python-bom, svg).
- Im Modul `pcbnew` gibt es keine `BOARD_NETLIST_UPDATER`/Netlist-Klasse: `[x for x in dir(pcbnew) if 'netlist' in x.lower() or 'updater' in x.lower()]` → nur `ExportSpecctraDSN`, `ImportSpecctraSES`, `netclasses_map`. ✔
- Websuche bestätigt: F8 in der GUI ist der vorgesehene Weg, ein CLI-Befehl wird nicht genannt (● https://forum.kicad.info/t/reload-netlist-or-update-pcb-from-schematic/20280).

**Ersatz, der funktioniert**: `kicad-cli sch export netlist --format kicadxml` → Skript, das aus der **V2-Kopie** das V3-PCB macht. Skript im Anhang A (`netlist2pcb.py`). Ergebnis:

```
$CLI sch export netlist --format kicadxml -o v3.xml WordClock-ESP32S3-V3.kicad_sch     → 104 Bauteile, 88 Netze
$PY netlist2pcb.py ../a/V2.kicad_pcb v3.xml V3.kicad_pcb
→ {'kept': 82, 'removed': 18, 'new_from_v2': 17, 'new_from_lib': 5, 'replaced': 9, 'new': 13}
  pads_assigned 343  pad_fehlt 0  netze_soll 88
```

Was das Skript tut: Bahnen und Vias löschen; Footprints, die V3 nicht mehr hat (18: STM32-Umfeld, ESP-12F, `H3`, `SW4`, …), entfernen; geänderte Footprints (9) **an derselben Stelle** neu setzen (Position, Drehung, Seite geerbt); neue (22) aus V2-Vorlagen (Bibliotheken `EasyEDA`, `Samacsys`, `Snapeda` gibt es nur dort) oder aus `SharedSupport/footprints/*.pretty` laden und auf einen Parkplatz (60/60 mm, ausserhalb) legen; Referenz, Wert, **Pfad (KIID des Schaltplansymbols)**, Felder (`LCSC`, `Datasheet`, …), DNP/Aus-BOM-Flags aus dem Export setzen; alle Pads (**auch mehrere gleicher Nummer**: `U2` Pad 41 GND, `USB1` Pad SH) einem Netz zuweisen; Zonen auf die neuen Netznamen abbilden; unbenutzte Netze entfernen; Zonen füllen (`ZONE_FILLER`, lief unter 10.0.2 ohne Absturz, alle 10 Zonen `IsFilled`).

**Nachweis, dass Platine und Schaltplan übereinstimmen** (`--schematic-parity`, die einzige Entsprechung zu „F8 meldet keine Änderungen“):

```
$CLI pcb drc --schematic-parity --severity-all --format json -o drc.json V3.kicad_pcb
→ erster Lauf: 102 Probleme (net_conflict 15, footprint_symbol_mismatch 11, field_mismatch 76)
→ nach Korrektur der drei Lücken oben (Mehrfach-Pads, FPID, Felder/DNP): 0 Probleme in Schaltplan-Parität
```

**Provozierter Fehler (DIR-014)**: `R1` Pad 1 per Skript auf `GND` gelegt (Schaltplan `/USER`), gespeichert:
`1 Probleme in Schaltplan-Parität gefunden` → `net_conflict: Das Pad-Netz (GND) stimmt nicht mit dem im Schaltplan (/USER) angegebenen Netz überein`. Die Prüfung schlägt an. ✔

Grenzen: Das Ergebnis ist ein **Rohentwurf**: DRC auf V3.kicad_pcb: 720 Meldungen, 128 unverbunden (alle Bahnen gelöscht), 170 `shorting_items`/`solder_mask_bridge` (nicht platzierte Teile, alte Zonen), `drill_out_of_range` 12 — **erwartet, nicht bewertet**. `--schematic-parity` prüft Netze, Footprints, Felder und Attribute, aber **nicht**, ob Footprint-Geometrie zum Bauteil passt.

### (c) Leiterbahn und Via per Skript — **gelingt** ✔

Auf einer V2-Kopie (`c/`), Netz `DCF77` (Pads `R9.1`, `U2.45`):

```
t=pcbnew.PCB_TRACK(b); t.SetStart(a); t.SetEnd(mid); t.SetWidth(FromMM(0.254)); t.SetLayer(pcbnew.F_Cu); t.SetNet(net); b.Add(t)
v=pcbnew.PCB_VIA(b); v.SetPosition(mid); v.SetDrill(FromMM(0.3)); v.SetWidth(FromMM(0.6)); v.SetNet(net); v.SetViaType(pcbnew.VIATYPE_THROUGH); b.Add(v)
→ Bahnen im Netz vorher: 2, nachher: 4        (Bahn + Via zählen als Einträge)
DRC: 241 → 246 Verstöße (+3 clearance, +2 hole_clearance)
```

Die Probe-Bahn war absichtlich unsauber (gerade Linie durch fremdes Kupfer); der DRC meldet das sofort. Bahnen, Vias, Zonen sind also programmierbar und prüfbar. **Das Routen als Entwurfsleistung (Wegfindung) ist damit nicht gelöst**, nur das Anlegen.

### (d) Freerouting — **nur geprüft, nichts installiert**

- Java: `java -version` → „Unable to locate a Java Runtime“ (nur die macOS-Hülle `/usr/bin/java`; `/usr/libexec/java_home -V` gleich). **Kein JRE vorhanden.** ✔
- `freerouting` nicht im PATH, nirgends in `/Applications/KiCad` ✔.
- Das KiCad-Python-Modul exportiert die Eingabe: `pcbnew.ExportSpecctraDSN(board, "v2.dsn")` → `True`, 185'483 Byte, `(host_version "10.0.2")`, Auflösung `um 10`, Umriss und Ebenen F.Cu/B.Cu enthalten. Auf dem V3-Rohentwurf ebenfalls `True` (97'899 Byte). ✔
- Der Rückweg `pcbnew.ImportSpecctraSES(board, "x.ses")` ist im Modul vorhanden (`dir(pcbnew)`), **nicht getestet** (keine SES-Datei ohne Freerouting). ✔ (nur Existenz)
- Download-Möglichkeit: `freerouting-cli-2.5.0-macos-arm64.tar.gz`, 20,9 MB, ohne JRE (✔Q, s. o.); Prüfsummendatei `freerouting-cli-2.5.0-checksums.txt` liegt bei. Der Rechner ist arm64 ✔. **Nicht heruntergeladen**; vor einem Einsatz braucht es die Freigabe des Nutzers (Fremdprogramm, GPL-3.0) und eine Prüfsumme.
- **Offen**: Ob `-de/-do` im nativen CLI gleich heissen, ist nur durch die Suchergebnisse zur JAR-Variante gestützt (●), nicht am 2.5.0-CLI geprüft. Die Routenqualität auf **dieser** Platine (145 × 39 mm, 2 Lagen, ESP32-S3-Modul, USB-Paar, 5-V-Leistungspfade) ist unbekannt; die Release-Zahlen (75 % voll geroutete Boards von 1157) sind Herstellerzahlen und sagen nichts über diese Platine (✔Q).
- Für ein 2-Lagen-Brett mit 1:1-Format ist ein Autorouter ohnehin nur für die Restverbindungen sinnvoll; Leistungsbahnen, USB-Paar und LDR-Pfad gehören von Hand (vom Layouter-Agenten programmiert) vorgegeben und für den Router **gesperrt** (`-inc` für Netzklassen).

### (e) Export Gerber / Bohrung / Position, und was das Fabrication Toolkit bei V2 erzeugt hat

**`kicad-cli` direkt** (auf einer V2-Kopie, `e/proj`), ✔:

```
$CLI pcb export gerbers --check-zones -o out/g/ V2.kicad_pcb      → 15 Dateien (F_/B_Cu, Mask, Paste, Silkscreen, Fab, Courtyard, Margin, Edge_Cuts, job.gbrjob)
$CLI pcb export drill --format excellon --excellon-separate-th --generate-map --map-format pdf -o out/g/ V2.kicad_pcb
$CLI pcb export pos --format csv --units mm --side both -o out/pos.csv V2.kicad_pcb
$CLI pcb export stats --format json -o out/stats.json V2.kicad_pcb
```

- `pos.csv` hat Spalten `Ref,Val,Package,PosX,PosY,Rot,Side` mit **absoluten Seitenkoordinaten** (Y negativ: `C1 172.75 -99.00`), nicht relativ zur Platinenecke. JLCPCB erwartet Mittelpunkte relativ zum Ursprung, das Toolkit rechnet sie um (s. u.).
- `stats.json`: Umriss „145,0000 mm × 39,0000 mm“, Dicke 1,6 mm, kleinste Bahn 0,2032 mm, kleinster Abstand 0,127 mm, kleinste Bohrung 0,3 mm; 329 Via-Bohrungen 0,3 mm, 20 Pads 1,1 mm, 3 Löcher 2,7 mm (H8–H10), 2 Löcher 2,2 mm (H1, H2), 4 Langlöcher 0,7×1,5 mm. Hier ablesbar, ob eine Fertigungsgrenze unterschritten wird (die Grenzen selbst aus belegter Quelle: Aufgabe des Analysten, **nicht in diesem Blatt gelesen**).
- 3D-Bild: `$CLI pcb render --side top --quality basic -w 2400 -h 700 --zoom 3.2 -o top2.png V2.kicad_pcb` (1,1 s). **Ein Agent kann das PNG mit `Read` ansehen**; ich habe es gelesen: Bauteile, Beschriftung, Stecker und Löcher sind klar erkennbar (zeigt auch, dass der USB-Stecker unter die Unterkante ragt). Ohne `--zoom` bleibt die Platine zu klein. `pcb export pdf/svg` laufen ebenfalls (nicht angesehen; kein `rsvg-convert`/`magick` vorhanden, `qlmanage`/`sips` schon).

**Fabrication Toolkit headless**, ✔ (Kopie des Plugin-Ordners als `plugins/` neben das Projekt gelegt):

```
cd e; cp -R ~/Documents/KiCad/10.0/3rdparty/plugins/com_github_bennymeg_JLC-Plugin-for-KiCad plugins
$PY -m plugins.cli -p proj/V2.kicad_pcb --autoTranslate --autoFill --allActiveLayers --nonInteractive     (1,2 s)
→ proj/production/{V2.zip (18 Gerber/Bohrdateien), bom.csv, designators.csv, netlist.ipc, positions.csv, backups/}
```

Es läuft ohne GUI (mit harmlosen `assert "PgmOrNull()" failed in register_action()` und `Debug:`-Zeilen). Die Optionen in V2: `fabrication-toolkit-options.json` = `{"ALL_ACTIVE_LAYERS": true, "AUTO TRANSLATE": true, "AUTO FILL": true, "EXCLUDE DNP": false, "EXTEND_EDGE_CUT": false, "ALTERNATIVE_EDGE_CUT": false, "OPEN BROWSER": false, "NO_BACKUP_OPT": false, "ARCHIVE_NAME": "", "EXTRA_LAYERS": ""}` — das entspricht genau meinen Flags. Der Lauf **reproduziert** den V2-Ordner `production/` (März 2026) bis auf drei Unterschiede, die erst später ins V2-PCB kamen: `C10`, `R23` (neu in BOM/Positionen/Designators) und `Q2` (Y 26,75 → 25,5). Bestätigt: `positions.csv` hat `Designator,Mid X,Mid Y,Rotation,Layer` **relativ zur linken oberen Ecke** (`C1 91.25,16.75`); `bom.csv` hat `Designator,Footprint,Quantity,Value,LCSC Part #` (LCSC aus dem Footprint-Feld `LCSC`).

Daneben liegt in V2 ein zweites Ausgabeverzeichnis `jlcpcb/` (`gerber/`, `production_files/` mit `BOM-…`, `CPL-…`, `GERBER-….zip`, `project.db`), vom Plugin **JLCPCB Tools** (bouni) erzeugt — also zwei Werkzeuge. Das V2-Format `production/` stammt vom Fabrication Toolkit; für V3 bietet sich dasselbe an. Das Format der JLCPCB-Dateien (BOM, CPL) ist hier nur an V2-Beispielen gelesen, **nicht** gegen die JLCPCB-Anforderungen geprüft.

**Zonenfüllung**: Gerber-Export mit `--check-zones` und Toolkit `--autoFill` füllen vorher. `kicad-cli pcb drc --refill-zones --save-board` lief auf dem V3-Rohentwurf mit Exit 0 und „Platine gespeichert“. ✔ Die Suche nennt zu `ZONE_FILLER` via SWIG einen Absturz in einem fremden Projekt (● https://glama.ai/mcp/servers/@mixelpixx/KiCAD-MCP-Server/blob/b9bd401b1185eabd4e835a84d9071e09ad7b645c/docs/KNOWN_ISSUES.md); bei mir lief es unter 10.0.2 durch — auf grösseren Platinen weiter beobachten.

## 3. Fazit

**Was ein Agent allein kann** (alles ✔ gezeigt):
- V2-PCB kopieren, lesen, ändern, speichern, Zonen füllen; Footprints platzieren/drehen/spiegeln/ersetzen.
- Netze und Footprints aus dem V3-Schaltplan ins PCB bringen (Ersatzskript, Anhang A) und mit `--schematic-parity` **gegen den Schaltplan belegen**.
- Bahnen und Vias programmatisch anlegen (auch aus einer Wegliste, die der Agent selbst berechnet).
- DRC mit Projektregeln gegen die Baseline; Gerber/Bohrung/Position/Stats/IPC-D-356; Fabrication-Toolkit-Paket wie bei V2.
- **Sichtprüfung** über `pcb render` und `Read` des PNG: Platzierung, Überlappungen, Antennenfreihaltung grob erkennbar. Das ersetzt keinen Blick in die GUI auf Bahnenführung in Zone-Gebieten, aber es ist mehr als nichts.

**Was ein Agent nicht (verlässlich) kann:**
- Interaktives Routen in der Qualität eines Menschen (Bahnen schieben, Abstände abwägen). Programmiertes Verlegen der **wenigen** Leistungs- und Signalpfade ist denkbar; ~200 Verbindungen auf 145 × 39 mm von Hand per Skript zu legen ist aufwendig und fehleranfällig.
- Beurteilen, ob ein Layout elektrisch **gut** ist (Rückstrompfade, EMV, Antennenumfeld); DRC prüft Regeln, nicht Eignung.
- Die IPC-API nutzen, solange keine GUI-Sitzung läuft.

**Was der Nutzer in der KiCad-GUI braucht:**
- Das fertige PCB einmal öffnen, F8 drücken (muss „keine Änderungen“ melden), DRC laufen lassen, Zonen füllen (`B`), die Bahnenführung ansehen und die 3D-Ansicht gegen das Gehäuse legen. Das ist die einzige Abnahme, die nicht aus Skripten kommt.
- Falls Autorouting gewünscht: Freigabe für Freerouting (Download, Fremdsoftware).
- Der **USB-C-Stecker** (16-polig, Gehäuseöffnung): mechanische Prüfung am Gerät/Gehäuse; per Datei nicht entscheidbar.

**Empfehlung** (mit Gründen):
1. **Ausgangspunkt `V2.kicad_pcb`-Kopie plus `netlist2pcb.py`** (Anhang A), nicht ein leeres PCB und nicht handgeschriebener S-Ausdruck. Grund: Umriss, Löcher, Stecker, Zonen und die EasyEDA-Footprints bleiben **byte-gleich** aus V2 erhalten; KiCad selbst schreibt die Datei.
2. **`pcbnew` (SWIG) für Platzierung/Bahnen/Zonen, `kicad-cli` für DRC, Parität, Export.** Grund: einzige Wege ohne GUI, beide am Rechner belegt. Die IPC-API entfällt (GUI nötig).
3. **Bahnen: Leistungspfade, USB-Paar und LDR-Pfad per Skript vorgeben**, danach DRC; Freerouting nur für Restverbindungen und nur nach Freigabe. Grund: Qualität auf dieser Platine unbekannt, kein JRE vorhanden, Rückweg `ImportSpecctraSES` ungetestet.
4. **Fertigung: Fabrication Toolkit per `python3 -m plugins.cli` mit den V2-Optionen.** Grund: reproduziert den V2-Ordner; Optionsdatei ist schon da.
5. **Abnahme-Kette** für `hw-reviewer`: (i) Format-Skript (Abschnitt 4) Exit 0, (ii) `--schematic-parity` 0 Probleme, (iii) DRC nicht über der V2-Baseline je Meldungsart, (iv) Render-PNG, (v) GUI-Öffnen durch den Nutzer.

**Rest-Risiken:**
- SWIG wird in KiCad 11 entfernt (✔Q): Die Skripte sind an 10.x gebunden; KiCad auf dem Rechner nicht aktualisieren, bis das Layout fertig ist, oder auf IPC umstellen.
- Die V2-Baseline-DRC (241 Meldungen, davon 142 `silk_overlap`) maskiert neue Meldungen **gleicher Art**; der Vergleich nach Art genügt nicht, wenn z. B. `clearance` von 0 auf >0 geht — Zahl je Art und je Position vergleichen. (Mein DRC-Lauf zeigte `silk_overlap` 142 → 143 nach einer Verschiebung, also Art-Zähler **ändern sich**.)
- `--schematic-parity` sagt nichts über Footprint-Geometrie. Footprints für `U2` (ESP32-S3-WROOM-1), `USB1` (HRO TYPE-C-31-M-12), `D2/D3` (Nexperia CFP3) kommen aus der **Stock-Bibliothek von KiCad 10**; ihre Masse sind mit den Datenblättern der Komponentenblätter abzugleichen (hier **nicht gelesen**).
- Netznamen V3 `/…` gegenüber V2 (Stolperstein 7) und Footprint-Wechsel (Stolperstein 8) sind **Designerfragen**.
- Der SWIG-Weg hängt von der Reihenfolge ab: `RemoveNative`, `Duplicate(False)`, `RemoveUnusedNets(None)` (Stolpersteine 1–2).

## 4. Format 1:1 — Prüfung gegen V2 (100 %-Vorgabe)

**Welche Daten lesbar sind** (alle ✔ per Skript gelesen): je Footprint Referenz, FPID, Position (nm), Drehung, Lage, `IsLocked()`, **alle Pads mit Nummer, Position und Bohrdurchmesser**; Umriss als `Edge.Cuts`-Elemente (Art, Start, Ende) plus `GetBoardEdgesBoundingBox()`. V2 hat **12 Edge.Cuts-Elemente**; Begrenzungsbox 145,05 × 39,05 mm (inkl. Linienbreite 0,05 mm; der Skill nennt 145,00 × 39,00).

**Vorschlag**: `format11.py` (Anhang B) vergleicht für die Referenzteile `H1 H2 H4 H5 H6 H7 H8 H9 H10 SW1 SW2 SW3 USB1` Position (Toleranz 1 µm), Drehung, Lage und, wo der Footprint **gleich** ist, **jedes Pad** (Nummer, Position, Bohrung) sowie die Menge der Edge.Cuts-Elemente. Exit 1 bei jeder Abweichung; bei Footprint-Wechsel (`USB1`) nur ein Hinweis, denn dort gilt die mechanische Prüfung.

Erprobt:

```
$PY format11.py ../a/V2.kicad_pcb V3.kicad_pcb
→ HINWEIS Footprint USB1 EasyEDA:TYPE-C-SMD_TYPE-C-6P -> Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12
  verglichen: 13 Referenzteile, 33 Pads (nur gleicher Footprint), 12 Edge.Cuts-Elemente; Umriss 145.05 x 39.05 mm; Abweichungen: 0     exit=0
Provokation: H9 um 1 mm nach rechts (shift.py)
→ ABWEICHUNG H9 x 154.0 155.0 / ABWEICHUNG Pads H9 … Abweichungen: 2     exit=1
```

Das Skript hat sich bei **dieser Provokation** bewährt (DIR-014). Lücken, die ich benenne:
- **Zahl**: 13 Referenzteile ist die Liste, die ich aus dem Skill-Text und `H`-Nummern ableitete. Der Skill nennt `H1`…`H10`; `H3` entfällt in V3 (der Schaltplan streicht ihn), das Skript lässt ihn aus; ob ausser diesen noch **weitere** Teile fix bleiben müssen (z. B. `LED`-Zugänge `JP`, Testpunkte `TP*`, `U7` Batterie), entscheidet der Nutzer/Architekt — **die Liste ist nur so vollständig wie diese Entscheidung**.
- Das Skript prüft **nicht** Grafik ausser Edge.Cuts (Silkscreen-Beschriftung, Aussparungen/Keepouts, Zonenränder).
- `USB1`: Position/Drehung/Lage und Edge.Cuts werden geprüft, die **Öffnung** hängt am Footprint (anderer Stecker). Hier ist **nicht gelesen**, ob die Aussenmasse des 16-poligen HRO TYPE-C-31-M-12 mit dem 6-poligen `TYPE-C-6P` übereinstimmen — Aufgabe des Komponentenblatts zum Stecker.
- Gesperrt-Zustand (`IsLocked`) wird gelesen, aber nicht gefordert. **Empfehlung**: Die Referenzteile im V3-PCB sperren (`f.SetLocked(True)`), damit ein Griff in der GUI sie nicht verschiebt; das Format-Skript bleibt die Instanz.
- Das Format-Skript sollte in `tools/checks/hardware.py` (H7 ist dort schon die Geometrieprüfung `pcb_geometry`, Zeile 235 ff.) aufgenommen werden; **Besitzer ist der Lead**, dort habe ich nichts geändert.

## 5. Offene Punkte

1. **Freerouting-CLI am Gerät nicht erprobt** (Download und Freigabe offen); `-de/-do` des nativen 2.5.0-CLI nicht gelesen.
2. **`ImportSpecctraSES` nicht erprobt.**
3. **IPC-API nicht erprobt** (GUI nötig, Nutzer hat V2 offen).
4. Fabrication-Toolkit-Version unbekannt (kein `metadata.json` im Ordner); Lieferumfang (JLCPCB-BOM/CPL-Format) nicht gegen JLCPCB geprüft.
5. Fertigungsgrenzen JLCPCB (Bahn, Abstand, Bohrung) **nicht gelesen** → Aufgabe des Analysten, bevor der Layouter die Regeln setzt.
6. Masse der Stock-Footprints `ESP32-S3-WROOM-1`, `USB_C_Receptacle_HRO_TYPE-C-31-M-12`, `Nexperia_CFP3_SOD-123W`, `SOT-23-6` gegen Datenblatt **nicht geprüft**.
7. Rückfragen an den Designer: Schrägstrich-Netznamen (Stolperstein 7); Footprint-Wechsel D2/D3/C20/C22/R11/R13/R17 (8); `R33` mit Kondensator-Footprint (9).
8. Rückfrage an den Architekten: **Liste der fixen Teile** für die 1:1-Prüfung (Abschnitt 4).
9. Der Rohentwurf `V3.kicad_pcb` aus (b) lebt nur im Scratchpad; er enthält Parkpositionen und ist **nicht** zur Weitergabe gedacht.

## Anhang A — `netlist2pcb.py` (getestet, Stand 17.10.2026)

Aufruf: `$PY netlist2pcb.py <V2-Kopie.kicad_pcb> <v3.xml> <Ziel.kicad_pcb>`; daneben im Zielordner müssen `<Ziel>.kicad_pro`, `.kicad_dru` und `<Ziel>.kicad_sch` (gleicher Basisname) liegen.

```python
import pcbnew, sys, xml.etree.ElementTree as ET
SRC, XML, OUT = sys.argv[1:4]
LIB = "/Applications/KiCad/KiCad.app/Contents/SharedSupport/footprints/"
mm = pcbnew.FromMM
b = pcbnew.LoadBoard(SRC)
root = ET.parse(XML).getroot()
comps = {c.get("ref"): dict(fp=c.findtext("footprint"), val=c.findtext("value"), uuid=c.findtext("tstamps"),
         fields={x.get("name"): (x.text or "") for x in c.find("fields")} if c.find("fields") is not None else {},
         dnp=c.find("property[@name='dnp']") is not None, nobom=c.find("property[@name='exclude_from_bom']") is not None,
         nopos=c.find("property[@name='exclude_from_pos_files']") is not None)
         for c in root.find("components").findall("comp")}
nets = {n.get("name"): [(x.get("ref"), x.get("pin")) for x in n.findall("node")] for n in root.find("nets").findall("net")}
tmpl = {}
for f in b.GetFootprints(): tmpl.setdefault(f.GetFPID().GetUniStringLibId(), pcbnew.Cast_to_FOOTPRINT(f.Duplicate(False)))
for t in list(b.GetTracks()): b.RemoveNative(t)
stat = dict(kept=0, removed=0, new_from_v2=0, new_from_lib=0, replaced=0, new=0)
place = {}; have = {}
for f in list(b.GetFootprints()):
    ref = f.GetReference()
    if ref not in comps: b.RemoveNative(f); stat["removed"] += 1
    elif f.GetFPID().GetUniStringLibId() != comps[ref]["fp"]:
        place[ref] = (f.GetPosition(), f.GetOrientation(), f.IsFlipped()); b.RemoveNative(f); stat["replaced"] += 1
    else: have[ref] = f; stat["kept"] += 1
for ref, c in comps.items():
    if ref in have: f = have[ref]
    else:
        lid = c["fp"]
        if lid in tmpl: f = pcbnew.Cast_to_FOOTPRINT(tmpl[lid].Duplicate(False)); stat["new_from_v2"] += 1
        else:
            nick, name = lid.split(":"); f = pcbnew.FootprintLoad(LIB + nick + ".pretty", name)
            assert f is not None, lid; f.SetFPID(pcbnew.LIB_ID(nick, name)); stat["new_from_lib"] += 1
        if ref in place:
            pos, ori, flip = place[ref]
            if flip: f.Flip(f.GetPosition(), pcbnew.FLIP_DIRECTION_TOP_BOTTOM)
            f.SetOrientation(ori); f.SetPosition(pos)
        else: f.SetPosition(pcbnew.VECTOR2I(mm(60), mm(60))); stat["new"] += 1
        b.Add(f); have[ref] = f
    f.SetReference(ref); f.SetValue(c["val"]); f.SetPath(pcbnew.KIID_PATH("/" + c["uuid"]))
    for k, v in c["fields"].items():
        if k != "Footprint": f.SetField(k, v)
    f.SetDNP(c["dnp"]); f.SetExcludedFromBOM(c["nobom"]); f.SetExcludedFromPosFiles(c["nopos"])
for f in b.GetFootprints():
    for p in f.Pads(): p.SetNetCode(0)
for name in nets:
    if b.FindNet(name) is None: b.Add(pcbnew.NETINFO_ITEM(b, name))
unk = n_assigned = 0
for name, nodes in nets.items():
    net = b.FindNet(name)
    for ref, pin in nodes:
        pads = [p for p in have[ref].Pads() if p.GetNumber() == pin]
        if not pads: unk += 1; print("Pad fehlt:", ref, pin, name); continue
        for pad in pads: pad.SetNet(net); n_assigned += 1
for z in b.Zones():
    old = z.GetNetname(); new = old if old in nets else "/" + old
    print("Zone", old, "->", new if new in nets else "KEIN NETZ")
    if new in nets: z.SetNet(b.FindNet(new))
b.RemoveUnusedNets(None)
pcbnew.ZONE_FILLER(b).Fill(b.Zones())
pcbnew.SaveBoard(OUT, b)
print(stat, "pads_assigned", n_assigned, "pad_fehlt", unk, "netze_soll", len(nets))
```

Bekannte Schwächen: Das Skript legt keine Netzklassen an (kommen aus der `.kicad_pro`); Parkpositionen sind fest; es löscht **alle** Bahnen; Zonenabbildung ist nur „`X` → `/X`“.

## Anhang B — `format11.py` (getestet, Stand 17.10.2026)

Aufruf: `$PY format11.py <V2.kicad_pcb> <V3.kicad_pcb>`; Exit 1 bei Abweichung.

```python
import pcbnew, sys
V2, V3 = sys.argv[1:3]
FIX = ["H1","H2","H4","H5","H6","H7","H8","H9","H10","SW1","SW2","SW3","USB1"]
TOL = 1000   # 1 um in nm
def load(p):
    b = pcbnew.LoadBoard(p); d = {}
    for f in b.GetFootprints():
        pos = f.GetPosition()
        d[f.GetReference()] = dict(x=pos.x, y=pos.y, rot=round(f.GetOrientationDegrees() % 360, 3), side=f.GetLayerName(),
            fp=f.GetFPID().GetUniStringLibId(),
            pads=sorted((p.GetNumber(), p.GetPosition().x, p.GetPosition().y, p.GetDrillSize().x) for p in f.Pads()))
    edge = sorted((g.GetShapeStr(), g.GetStart().x, g.GetStart().y, g.GetEnd().x, g.GetEnd().y)
                  for g in b.GetDrawings() if g.GetLayer() == pcbnew.Edge_Cuts)
    return d, edge, b.GetBoardEdgesBoundingBox()
a, ea, ba = load(V2); c, ec, bc = load(V3)
bad = n = npads = 0
for r in FIX:
    if r not in a or r not in c: print("FEHLT", r, r in a, r in c); bad += 1; continue
    n += 1; A, C = a[r], c[r]
    for k in ("x", "y"):
        if abs(A[k] - C[k]) > TOL: print("ABWEICHUNG", r, k, A[k]/1e6, C[k]/1e6); bad += 1
    for k in ("rot", "side"):
        if A[k] != C[k]: print("ABWEICHUNG", r, k, A[k], C[k]); bad += 1
    if A["fp"] != C["fp"]: print("HINWEIS Footprint", r, A["fp"], "->", C["fp"])
    else:
        npads += len(A["pads"])
        if A["pads"] != C["pads"]: print("ABWEICHUNG Pads", r); bad += 1
if ea != ec: print("ABWEICHUNG Edge.Cuts: %d vs %d Elemente" % (len(ea), len(ec))); bad += 1
print("verglichen: %d Referenzteile, %d Pads, %d Edge.Cuts-Elemente; Umriss %.2f x %.2f mm; Abweichungen: %d"
      % (n, npads, len(ea), bc.GetWidth()/1e6, bc.GetHeight()/1e6, bad))
sys.exit(1 if bad else 0)
```
