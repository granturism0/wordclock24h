# Abläufe — Schritt für Schritt, mit den Befehlen

Stand: **17.10.2026** (Erstfassung). Alle Befehle laufen im Wurzelverzeichnis des Repos (`git rev-parse --show-toplevel`), auf macOS mit zsh. **Kein Ablauf hier braucht `make` oder die Uhr** (R1, R5).
Abläufe, die am Gerät **noch nie ausgeführt** wurden (es gibt kein Gerät), sind als **Planungsstand** gekennzeichnet; sie sind aus Datenblatt und Spec abgeleitet, nicht erprobt.

**Voraussetzungen** (✔R, am 17.10.2026 vorhanden): KiCad 10.0.2 mit `kicad-cli` unter `/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli` (**nicht im PATH**; die Prüfungen rufen es mit vollem Pfad);
Python 3. Die Zeile `Fontconfig error: Cannot load default config file` auf stderr ist harmlos, filtere sie (`2>&1 | grep -v Fontconfig`), damit eine echte Meldung nicht darin untergeht.

---

## 1. Den Stand prüfen

```
python3 tools/checks/hardware.py                  alle acht Prüfungen H1 bis H8
python3 tools/checks/hardware.py --nur H1,H3      nur einzelne
./tools/checks/hardware-gegenproben.sh            die Gegenproben (DIR-014), rund zehn Sekunden
./tools/guardrails.sh                             alle Stufen; Hardware sind S19 (Prüfungen) und S20 (Gegenproben)
```

**Lesart.** Jede Zeile beginnt mit `OK`, `KRITISCH` oder `HOCH`. **Exit 0** heisst: kein KRITISCH-Befund. `HOCH` sind Warnungen, die der Lauf **zählt, aber nicht blockiert** — und die man trotzdem lesen muss; am 17.10.2026
meldete der Lauf `HOCH` für fehlende Stückliste (H4), Haken ohne Seitenangabe (H5) und `C32` am LDR-Netz (H8). **Jede Prüfung nennt, wie viele Dinge sie betrachtet hat** (Netze, Pins, Bauteile, Positionen).
Vergleiche die Zahl mit dem, was es geben müsste; eine Prüfung, die nichts sieht, meldet sonst „OK“ (DIR-014, `BEFUNDE.md` L235).

`./tools/guardrails.sh --full` baut und ist den Hardware-Rollen **verboten** (Hook `tools/hooks/no-build.py`); der Lauf ohne `--full` ist erlaubt.

### Was jede Prüfung betrachtet und was bei einem Fund zu tun ist

| Prüfung | Betrachtet | Bei KRITISCH oder HOCH |
|---|---|---|
| **H1** ERC | Alle ERC-Meldungen des V3-Schaltplans; Fehler sind kritisch; Meldungsarten über der Baseline `tools/checks/hardware-erc.baseline` sind kritisch | Meldung beheben oder mit Begründung als bewusst führen (Designer). Die Baseline nie hochsetzen, ohne die Begründung zu schreiben (OP-30) |
| **H2** Netzliste | Schaltplan gegen Soll-Netzliste `hardware/esp32s3/expected.json`, nach **Pin-Mengen** (benannte Netze zusätzlich nach Namen), Bauteile und Werte, bewusst offene Pins | Ursache in `gen.py` oder im Schaltplan suchen (Designer). **Grenze siehe Abschnitt 3** |
| **H3** Pinregeln | Pins des Moduls `U2` gegen `tools/checks/hardware-pinregeln.json` (gesperrte Pins offen, feste Pins nur an ihrem Netz, kein 5-V-Netz am Modul, LDR an ADC1) | Pin umhängen (Designer, nach Entscheid des Architekten) |
| **H4** Stückliste | `hardware/esp32s3/bom.csv`: jede Zeile eine gültige LCSC-Nummer, Designatoren = Bauteile des Schaltplans (`in_bom`, nicht `dnp`), LCSC-Nummern im Beleg der Komponentenblätter, Alter der JLCPCB-Prüfung | Stückliste vervollständigen (OP-08); JLCPCB-Prüfung erneuern (Analyst) |
| **H5** Komponentenblätter | Pflichtabschnitte („Quelle“, „Fertigung bei JLCPCB“, „Folgerung“); Zeilen mit ✔ ohne Seiten-/Tabellenangabe | Blatt ergänzen (Analyst) |
| **H6** V2-Referenz | gespeicherter Export `hardware/esp32s3/v2/nets.json` gegen den **aktuellen** V2-Schaltplan | `export_v2.py` neu laufen lassen, Änderungen prüfen |
| **H7** Format und DRC | Sobald ein Layout existiert: Umriss und 13 Positionen gegen `V2.kicad_pcb`; DRC (nicht schlechter als V2); fehlen `.kicad_pro`/`.kicad_dru` neben dem Layout, **verweigert** H7 den DRC | Layout korrigieren (Layouter). **Bis ein Layout existiert, meldet H7 nur „noch kein Layout“** |
| **H8** LED-Schnittstelle | Wert und Pinanschluss von 18 Bauteilen und die Mitglieder von zehn Netzen gegen V2 (ohne den Controller) | Änderung zurücknehmen **oder** mit Freigabe des Nutzers in `tools/checks/hardware-led-schnittstelle.ausnahmen` eintragen (Abschnitt 8) |

### Die Gegenproben

`tools/checks/hardware-gegenproben.sh` baut je Prüfung eine **Verletzung in einer Kopie** (nie im echten Entwurf, nie im V2-Projekt) und verlangt Exit 1 **und** die richtige Meldung; dazu die Gegenrichtung (unveränderter Entwurf besteht).
Proben (Stand 17.10.2026): P1/P2 (H1/H2: ein No-Connect-Zeichen entfernt, ein Netzbezeichner umbenannt), P3–P6 (H3: gesperrter Pin belegt, 5-V-Netz am Modul, LDR nicht an ADC1, `IO0` an fremdem Netz), P7–P9 (H4: vollständige Stückliste, ein Bauteil fehlt,
Zeile ohne LCSC), P10 (H5), P11/P12 (H6), P13/P14/P14b (H7: Layout gleich, Loch 1 mm verschoben, Projektregeln fehlen), P15–P19 (H8: unverändert, `R22` mit anderem Wert, Pin umgehängt, zusätzliches Teil, `R20` fällt weg).
Die Proben P11 und P18 erwarten Exit 0 mit einer HOCH-Meldung — **das ist Absicht**: Diese Prüfungen warnen, statt zu blockieren.

---

## 2. Den V2-Export erneuern

```
python3 hardware/esp32s3/export_v2.py
```

Liest den V2-Schaltplan **nur lesend**, schreibt `hardware/esp32s3/v2/v2.xml` und `v2/nets.json`. H6 meldet HOCH, sobald V2 und gespeicherter Export auseinanderlaufen; **danach** prüfen, was sich geändert hat, und `gen.py` neu laufen lassen.
Das V2-Projekt wird dabei nie verändert.

---

## 3. Den Schaltplan neu erzeugen (Designer)

```
python3 hardware/esp32s3/gen.py                   schreibt WordClock-ESP32S3-V3.kicad_sch und expected.json
hardware/esp32s3/erc.sh                           ERC, Zusammenfassung auf stdout, Bericht nach erc.json
python3 hardware/esp32s3/vergleich.py             Netzliste gegen expected.json
python3 tools/checks/hardware.py                  alle Prüfungen
```

**Zwei Dinge, die man dem Skript nicht ansieht:**

1. **Jeder Lauf von `gen.py` vergibt neue UUIDs** (`uuid.uuid4()`). Die Schaltplandatei ändert sich damit **in jeder Zeile**, auch wenn sich inhaltlich nichts ändert; ein `git diff` ist dann nicht prüfbar. `gen.py` nur laufen lassen, wenn sich der Inhalt ändert (✔R: `gen.py`, Funktion `U()`).
2. **Die Soll-Netzliste `expected.json` entsteht im selben Lauf wie der Schaltplan.** H2 beweist deshalb, dass die Datei dem Generator entspricht, **nicht**, dass der Generator dem Entwurf des Architekten entspricht. Eine unabhängige Verankerung geben nur
   H3 (Pinregeln), H6 und H8 (beide gegen V2) und das Gutachten des `hw-reviewer`, der die Netzliste **selbst** erzeugt.

Das Skript schreibt nach `hardware/esp32s3/` (Schreiber: `hw-designer`). Der Pfad des Ausgabefiles ist das erste Argument (`python3 gen.py <ziel.kicad_sch>`); ohne Argument überschreibt es den Entwurf. **`expected.json` überschreibt es in beiden Fällen** (der Pfad ist fest `hardware/esp32s3/expected.json`).

### Netzliste, ERC von Hand

```
/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli sch export netlist --format kicadxml -o /pfad/netz.xml hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch
/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli sch erc --severity-all --format json -o /pfad/erc.json hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch
```

Der XML-Export liefert `components/comp` (Referenz, Wert, Footprint, Felder) und `nets/net` mit `node` (Referenz, **Pinnummer**, Pinfunktion). Nicht angeschlossene Pins stehen als Netz `unconnected-(<Ref>-<Pin>-Pad<N>)`.
**Automatisch vergebene Netznamen (`Net-(U6-E0)`) hängen am Pinnamen und ändern sich mit dem Symbol** — vergleiche nie danach, sondern nach Pin-Mengen. **Der V2-Pfad enthält Leerzeichen** (`WordClock-USB C - STM32F411 - V2`): immer in Anführungszeichen setzen.

---

## 4. Das Layout prüfen (Layouter) — Planungsstand

Ein V3-Layout gibt es noch nicht. Vorgehen laut Rollenbeschreibung `hw-layouter` und Skill `kicad-schema`:

1. **Kopie** von `V2.kicad_pcb` samt `.kicad_pro` und `.kicad_dru` unter `hardware/esp32s3/` anlegen (nie am Original in `~/Documents` arbeiten); Umriss, Löcher und Stecker bleiben, der Innenraum wird getauscht.
2. **DRC immer im Verzeichnis mit den Projektdateien** (`kicad-cli pcb drc` liest `.kicad_pro` und `.kicad_dru` neben der Platine):
   ```
   /Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli pcb drc --severity-all --format json -o /pfad/drc.json hardware/esp32s3/<layout>.kicad_pcb
   ```
   Fehlen die beiden Dateien, rechnet KiCad mit Standardregeln: am V2-Layout waren das **504 falsche Abstandsfehler und 35 Randabstände statt 0** (W-3b). H7 verweigert so einen Lauf.
3. `python3 tools/checks/hardware.py --nur H7` — Format 1:1 **und** DRC gegen `tools/checks/hardware-drc.baseline`. Das V2-Layout hat selbst 241 Meldungen (davon 5 Fehler `annular_width`); die Baseline ist die Obergrenze, **jede neue Meldungsart** steht im Bericht begründet.
4. **Eine Verletzung herstellen** (ein Loch 1 mm verschoben) und sehen, dass die Prüfung anschlägt (Gegenprobe P14).
5. Vor dem Layout: **Aussenmasse des 16-poligen USB-C-Steckers gegen den 6-poligen** belegen (OP-11) — das Layout beginnt nicht ohne.

---

## 5. Den ESP32-S3 flashen und in den Download-Modus bringen — Planungsstand

**Es gibt kein Gerät; dieser Ablauf ist aus dem Datenblatt und der Spec abgeleitet und nicht erprobt.** Das Framework (und damit der genaue Befehl) ist nicht entschieden (OP-24). Die Spec nennt als Beispiel `idf.py flash -p <PORT>` im Verzeichnis `esp32s3/` (Task 1.4); das Werkzeug dahinter ist `esptool`.

**Normaler Weg (automatisch, ohne Taster).** Das Werkzeug löst Download-Modus und Reset über die USB-Leitungen (DTR/RTS des CDC-Geräts) aus; es braucht keine Strapping-Beschaltung (✔K `esp32s3-wroom-1.md` §7; TRM Tab. 33.3-2 bis 33.4-4, S. 1247–1254).
Abnahme (Spec AK3): dreimal hintereinander ohne Boot-Taster.

**Manueller Weg** (wenn die Anwendung `GPIO19/20` umkonfiguriert hat oder die Firmware hängt und der automatische Weg versagt; ✔K DS-M Tab. 4-3, S. 14; HDG S. 35):

1. **BOOT-Taster (`SW5`, `GPIO0`) drücken und halten** (GPIO0 = Low).
2. **EN-Taster (`SW3`) kurz drücken** (EN kurz auf Low) und loslassen.
3. GPIO0 muss **mindestens 3 ms nach dem Anstieg von EN** noch Low sein (Haltezeit tH ≥ 3 ms, DS-M Tab. 4-2, S. 14); danach BOOT loslassen. Der Chip meldet sich als USB-Gerät im Download-Modus; GPIO46 muss dabei Low sein (Standard).
4. Flashen wie oben.

Ohne BOOT- und EN-Taster ist dieser Rückweg **nicht möglich**; ein Programmfehler, der `GPIO19/20` umstellt, wäre dann ein **Totalverlust des Zugangs** (`design.md` §8). Deshalb gehören beide Taster auf die Platine (OP-12).
**Nach dem Flashen** liegt das LittleFS des S3 leer da; die PWA muss dort erstmals hochgeladen werden (`./tools/install-app.sh`, DIR-017; Spec „Betroffene Laufzeiten“).

---

## 6. Bei JLCPCB bestellen — Planungsstand (es gibt noch kein Layout und keine V3-Stückliste)

**Bestellen tut der Nutzer, nach dem Gutachten des `hw-reviewer`.** Vorbereitung:

1. **`python3 tools/checks/hardware.py --nur H4,H5`**: Stückliste vorhanden, jede Zeile eine LCSC-Nummer, Designatoren gleich den Bauteilen des Schaltplans, Blätter vollständig. Heute **HOCH**, weil die Stückliste fehlt (OP-08).
2. **Bestände erneuern**, wenn die Prüfung älter als 14 Tage ist (H4 warnt dann). Quelle der Prüfung ist die Teilebibliothek von JLCPCB über die Such-Schnittstelle von `jlcpcb.com/parts`
   (`jlcpcb.com/api/overseas-pcb-order/v1/shoppingCart/smtGood/selectSmtComponentList`, Schlüsselwort = LCSC-Nummer, Treffer nur bei exakt gleicher `componentCode`; „Basic“ = `componentLibraryType = base`; „Preferred“ = `preferredComponentFlag = true`) — so in `jlcpcb-bestand.md` beschrieben, Ergebnis ●.
   Massgeblich ist der **JLCPCB-Bestand**, nicht der von LCSC (Beispiel C2913201: LCSC 0, JLCPCB 4934). Datum in `jlcpcb-bestand.md` („Prüfdatum“) nachtragen; H4 liest es dort.
3. **Stückliste im Format von V2:** Spalten `Designator,Footprint,Quantity,Value,LCSC Part #` mit UTF-8-Markierung am Dateianfang; gleiche Werte werden eine Zeile mit mehreren Designatoren (Skill `jlcpcb-bauteile`). Die Arbeitsfassung trägt zusätzlich **Klasse und Prüfdatum**,
   die Bestellfassung nur die fünf Spalten. Am Ende steht **die Zahl der Zeilen je Klasse**, damit der Nutzer sieht, wie viele Nicht-Basic-Nummern er bestellt.
4. **Fertigungsdaten** aus dem Layout: Format von V2 (`bom.csv`, `positions.csv`, `designators.csv`, `netlist.ipc`, ZIP), erzeugt mit dem KiCad-Plugin **Fabrication Toolkit** (`fabrication-toolkit-options.json` im V2-Projekt). **Achtung:** Das `production/` von V2 ist älter als Schaltplan und Layout
   und lässt `R23` und `C10` aus (W-4); die Fertigungsausgabe nach jedem Layout-Stand **neu** erzeugen und mit dem Schaltplan abzählen.
5. **Bestückbarkeit:** Das Funkmodul ist nur im **Standard-PCBA-Verfahren** möglich („Standard Only“, nicht Economic); die THT-Teile (Stiftleisten, DS18B20) und die Schirmlaschen der USB-Buchse sind **nicht geprüft** (OP-21).
6. Nach der Bestellung: Chronik und Entscheidungsübersicht nachtragen (Dokumentar).

---

## 7. Ein neues Komponentenblatt anlegen (Datenblatt-Analyst)

Ort: `hardware/komponenten/<bauteil>.md`. PDF per `curl -L -o` nach `hardware/komponenten/datenblaetter/` (nicht im Git), mit `Read` und `pages` lesen (höchstens 20 Seiten je Aufruf), Revision und Datum notieren.
Pflichtgliederung (H5 prüft die drei fett gedruckten): **Quelle** · Absolute Maximalwerte · Pinfunktionen und Strapping · Empfohlene Beschaltung · Kritische Daten · **Fertigung bei JLCPCB** · Mechanik · Fallen und Errata · **Folgerung für unsere Platine** · Offene Punkte.
Jede Zahl trägt ✔ (mit Seite), ● oder ○. Kritische Zahlen werden an **zwei** Stellen gegengelesen; Widersprüche werden benannt, nicht stillschweigend aufgelöst. Danach: `python3 tools/checks/hardware.py --nur H5`.

---

## 8. Eine Abweichung der LED-Schnittstelle freigeben (Lead, auf Anweisung des Nutzers)

Nur wenn der Nutzer eine Abweichung **ausdrücklich** freigegeben hat. Die Datei `tools/checks/hardware-led-schnittstelle.ausnahmen` (Schreiber: Lead) trägt je Ausnahme eine Zeile **mit Grund**:

```
wert:R22: <Grund>        das Teil darf einen anderen Wert haben
pin:Q2: <Grund>          das Teil darf andere Pinanschlüsse haben
zusatz:C32: <Grund>      das Teil darf zusätzlich an einem Schnittstellennetz hängen
fehlt:R20: <Grund>       das Teil darf an einem Schnittstellennetz fehlen
```

Stand 17.10.2026: **keine Ausnahme eingetragen.** Danach `python3 tools/checks/hardware.py --nur H8` und ein Eintrag in [vorgaben-des-nutzers.md](vorgaben-des-nutzers.md) (Datum, Wortlaut der Freigabe) und [chronik.md](chronik.md).

---

## 9. Eine neue Prüfung bauen (DIR-014)

Eine Prüfung ist erst fertig, wenn sie **einmal fehlgeschlagen ist und ihre Meldung ankommt.** Vorgehen wie in `tools/checks/hardware-gegenproben.sh`: Verletzung **in einer Kopie** herstellen (Umgebungsvariablen `HW_SCH`, `HW_NETLIST`, `HW_BOM`, `HW_PCB`, `HW_KOMP`, `HW_V2` gibt es **nur** dafür),
Exit 1 **und** das Muster der Meldung verlangen, die **Gegenrichtung** prüfen (die unveränderte Kopie besteht) und die **Zahl der betrachteten Dinge** gegen das halten, was es geben müsste. Die Prüfung gehört in `tools/checks/hardware.py` (Schreiber: Lead), die Probe in das Gegenproben-Skript.

---

## 10. Die Dokumentation nachführen (Dokumentar)

Nach **jedem** Arbeitsschritt des Hardware-Teams: Was ist neu gesichert → [wissensstand.md](wissensstand.md) (mit Beleg-Zeichen). Was ist entschieden → [entscheidungsuebersicht.md](entscheidungsuebersicht.md) (nicht neu entscheiden).
Was hat sich gegenüber V2 geändert → [aenderungen-gegenueber-v2.md](aenderungen-gegenueber-v2.md). Neue Vorgabe des Nutzers → [vorgaben-des-nutzers.md](vorgaben-des-nutzers.md), im Wortlaut. Was ist offen geworden oder erledigt → [offene-punkte.md](offene-punkte.md).
Datierter Eintrag → [chronik.md](chronik.md). Fehler früherer Fassungen werden **datiert korrigiert** („Bis … stand hier … Das stimmt nicht mehr, weil …“), nie still überschrieben.
Alles, was ausserhalb von `hardware/` zu ändern ist (`HARDWARE.md`, `CLAUDE.md`, `BEFUNDE.md`, `specs/`), geht als **Übergabeliste** an den zuständigen Schreiber.

---

## 11. Fallen bei der Arbeit

- **zsh auf macOS:** Ein ungequoteter Ausdruck wie `--include=*.c` bricht mit „no matches found“ ab. Muster quoten.
- **KiCad-Dateien** haben LF-Zeilenenden und sind UTF-8; prüfe Kodierung und Zeilenende vor dem Patchen, statt es anzunehmen (DIR-015, `CLAUDE.md`). Das Edit-Werkzeug vereinheitlicht gemischte Zeilenenden still.
- **Stock-Symbole mit `extends`** (`USBLC6-2SC6` baut auf `USBLC6-2P6` auf) lassen sich nicht einfach einbetten (siehe Skill `kicad-schema`).
- **Gestapelte Pins** (USB-C 16P: `A4`, `A9`, `B4`, `B9` auf derselben Koordinate; GND beim Modul: `1`, `40`, `41`) verbinden sich dort von selbst.
- **Netze aus Stromversorgungssymbolen:** V2 nutzt `power:VCC` mit dem **Wert** `VCC_3.3V` bzw. `VCC_5V`; der Netzname ist der Wert. `PWR_FLAG` an der richtigen Stelle (Liste in `hardware/esp32s3/flags.json`).
- **Das V2-Projekt wird nie beschrieben.** Auch nicht „ausnahmsweise“, auch nicht für einen Export in dessen Verzeichnis.
