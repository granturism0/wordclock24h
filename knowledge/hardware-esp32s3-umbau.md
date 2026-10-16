# Umbau auf den ESP32-S3 — Wissensstand der Hardware-Arbeit

**Lebendes Dokument (DIR-006).** Es hält fest, **was bei diesem Umbau gilt und wo es steht** —
damit nichts nur im Gesprächsverlauf liegt (Vorgabe des Nutzers vom 17.10.2026: *alle Erkenntnisse
dokumentieren, kein Chat-Drain*). Es **kopiert nicht**, was anderswo gepflegt wird: Die Einzelheiten
des Entwurfs führt `hardware/README.md` mit `hardware/dokumentation/` (Schreiber: `hw-doc-writer`), die
Belege zu den Bauteilen stehen in `hardware/komponenten/`, die Anforderungen in `specs/esp32s3-port/`.
Hier steht der Rahmen, die Arbeitsweise, die Prüfungen und die Lehre aus dem Anfang.

**Keine Zahlen, die veralten.** Wie viele Netze, Bauteile oder Meldungen eine Prüfung betrachtet,
nennt die Prüfung selbst — `./tools/guardrails.sh`, Stufen S19 und S20, oder `python3 tools/checks/hardware.py`.
Wo unten eine Zahl steht, trägt sie Datum und Fundstelle.

## 1. Worum es geht

Ein **ESP32-S3** soll **beide** Chips der heutigen Uhr ersetzen, den STM32F411 (`U2`) und den ESP-12F
(`U4`). **1:1-Umbau: kein Funktionsverlust, keine neuen Funktionen.** Geändert wird nur die
**Controller-Platine**; das LED-Board bleibt unverändert. Die Software-Seite (zwei Tasks mit einer
Warteschlange statt der UART-Brücke) beschreibt `specs/esp32s3-port/`. Dieses Dokument betrifft die
**Hardware-Arbeit** und deren Werkzeug.

Der Bestand, von dem ausgegangen wird, ist die **V2-Platine** (`WordClock-USB C - STM32F411 - V2`, ein
KiCad-Projekt ausserhalb dieses Repositorys unter `~/Documents`). Es ist die Referenz und bleibt für alle
Rollen **nur lesend**; der Entwurf des Umbaus arbeitet an einer **Kopie** unter `hardware/esp32s3/`.

## 2. Rahmen und Vorgaben des Nutzers

Alle Vorgaben stammen vom **17.10.2026**; ihren **Wortlaut** und ihre Chronologie führt
`hardware/dokumentation/vorgaben-des-nutzers.md` (`hw-doc-writer`). Hier die Kurzfassung, damit der
Rahmen auch ohne jene Datei lesbar ist:

| # | Vorgabe | Folge |
|---|---|---|
| 1 | Umbau des jetzigen Prints und seiner Funktion **1:1** auf einen ESP32-S3 (Nachtrag: kein Funktionsverlust, keine neuen Funktionen) | Der Funktionsumfang wird nicht erweitert; was am zweiten Chip hing, entfällt (Spec E6, E8) |
| 2 | Zielprodukt ist ein **Nachbau der Platine** mit dem Modul ESP32-S3-WROOM-1; dieselbe Software muss **auch auf dem DevKitC-1** laufen | Nur GPIOs, die das DevKit am Header herausführt (Spec E1, E2) |
| 3 | **Ein** USB-C-Anschluss genügt: Flashen, Konsole und JTAG | `USB1` wechselt von 6 auf 16 Pole mit Datenleitungen und ESD-Schutz (Spec E3, E9b) |
| 4 | **LDR und LEDs bleiben auf dem LED-Board**, dasselbe Board wird weiterverwendet | Das LED-Board wird nicht geändert (Spec E10) |
| 5 | LED-Ansteuerung, Schalten der LED-Versorgung und LDR-Lesung bleiben **pin- und funktionskompatibel** zu V2 | Prüfung H8 (siehe 5) |
| 6 | Der LDR hat einen **analogen** Eingang | `R22` **bleibt 1 kΩ**, weil die LDR-Lesung nach Vorgabe des Nutzers 100 % kompatibel bleiben muss; die Lesung ist funktional gleichwertig, nicht zählergleich, und braucht eine einmalige Nachkalibrierung (`hardware/komponenten/esp32s3-wroom-1.md` §6.3). Bis 17.10.2026 stand hier „`R22` ändert sich, Kalibrierung wird neu gemessen“. Das stimmt nicht mehr, weil der Nutzer die Kompatibilität festgelegt hat |
| 7 | Das **Leiterplattenformat bleibt 1:1** | Umriss, Befestigungslöcher, Lage der Stecker und Taster wie V2 (Spec E12); Prüfung H7 |
| 8 | Fertigung und Bestückung bei **JLCPCB**; jedes Teil dort erhältlich, **wenn möglich aus dem Standard-Katalog (Basic)** | Skill `jlcpcb-bauteile`; Prüfung H4 |
| 9 | Schaltplan und Layout in **KiCad** | Skill `kicad-schema` |
| 10 | Ein spezialisiertes **Hardware-Team** mit Reviews und Guardrails; Datenblätter werden gelesen, verstanden und analysiert | Rollen unter 3; Beweispflicht-Zeichen |
| 11 | **Eine Quelle für alle Boards**, damit nichts auseinanderdriftet; die LED-Logik so weit wie möglich übernehmen | Rolle `sw-architect`, `specs/multiboard-quelle/` |
| 12 | **Alles Wissen wird dokumentiert**, damit nichts nur im Chat steht | Rolle `hw-doc-writer`; dieses Dokument; `BEFUNDE.md` |
| 13 | **100 %ige Professionalität** | jede Aussage mit Beleg, jede Prüfung einmal fehlgeschlagen (DIR-014) |

Die **Reihenfolge der Umsetzung** auf der Software-Seite (zuerst LED-Ausgabe isoliert nachweisen, dann
I²C/EEPROM/RTC, dann Sensoren, zuletzt Brücke und Task-Aufteilung) hat der Nutzer festgelegt; sie steht in
`specs/esp32s3-port/` (E7) und hängt an Toren, die er selbst freigibt.

## 3. Team und Reihenfolge der Rollen

Die Rollen und ihr Besitz stehen in der Tabelle der Schreibrechte in `CLAUDE.md` (R3); der Hook
`tools/hooks/file-ownership.py` erinnert daran, ersetzt die Regel aber nicht.

| Rolle | Schreibt | Aufgabe |
|---|---|---|
| `hw-datasheet-analyst` | `hardware/komponenten/**` | liest Datenblätter, Richtlinien und Errata; liefert je Bauteil ein belegtes Komponentenblatt |
| `hw-architect` | `hardware/architektur/**` | zerlegt in Funktionsblöcke, entscheidet Pinzuordnung, Versorgung, Reset/Boot, USB; hält Entscheide mit Begründung fest |
| `hw-designer` | `hardware/esp32s3/**` ausser Layout | setzt Entscheide in den Schaltplan um: Generator, ERC, Netzlistenvergleich, Stückliste |
| `hw-layouter` | `hardware/esp32s3/*.kicad_pcb`, `*.kicad_dru`, `layout/`, `fertigung/` | Platzierung, Leiterbahnen, DRC, Fertigungsdaten — erst nach abgenommenem Schaltplan |
| `hw-reviewer` | `hardware/review/**` | prüft den Entwurf gegen Entscheide und Blätter; ändert ihn nie |
| `hw-doc-writer` | `hardware/dokumentation/**`, `hardware/README.md` | hält das Wissen der Hardware-Arbeit fest |
| `sw-architect` | `specs/multiboard-quelle/**` | spezifiziert die eine Quelle für alle Boards |
| `doc-writer` | `*.md` ausserhalb `hardware/`, `knowledge/**` | `HARDWARE.md`, `BEFUNDE.md`, dieses Dokument |

**Reihenfolge nach R3b: Analyst → Architekt → Designer → Reviewer.** Der Designer zeichnet nichts, was der
Architekt nicht entschieden hat, und der Architekt entscheidet keine Zahl, die der Analyst nicht belegt hat.
Der Layouter beginnt erst, wenn Schaltplan und Architekturentscheide abgenommen sind; der Dokumentar
arbeitet nach jedem Arbeitsschritt. Dass zwei Rollen verschiedene Dateien schreiben, heisst nicht, dass sie
zugleich laufen dürfen — die Abhängigkeitsspalte der Spec zählt, nicht die Besitzspalte.

**Beweispflicht (nur dieses Team):** Jede Aussage über ein Bauteil trägt ein Zeichen — **✔** im Datenblatt
gelesen (mit Seite oder Tabelle), **●** Sekundärquelle (Distributor-Datenbank, Beschreibung), **○** Annahme
oder Rechnung. Ein Wert aus dem Gedächtnis ist keiner. Prüfung H5 sieht nach, ob jedes ✔ eine Seite trägt.
Korrekturen werden datiert: „Bis `<Datum>` stand hier … Das stimmt nicht mehr, weil …“.

## 4. Wo was liegt

| Was | Wo |
|---|---|
| Einstieg, Stand, Aufbau der Hardware-Arbeit | `hardware/README.md` |
| Wissensstand, Änderungen gegenüber V2, Entscheidungsübersicht, Abläufe, offene Punkte, Chronik, Glossar | `hardware/dokumentation/` |
| Entscheide (mit Begründung), Pinbelegung, Leistungsbudget | `hardware/architektur/` |
| Komponentenblätter mit Datenblattbelegen; Datenblätter als PDF; JLCPCB-Bestand der V2-Stückliste | `hardware/komponenten/`, dort `datenblaetter/` und `jlcpcb-bestand.md` |
| Schaltplan-Entwurf, Generator (`gen.py`, `sx.py`), Soll-Netzliste (`expected.json`), `vergleich.py`, ERC (`erc.sh`) | `hardware/esp32s3/` |
| Gespeicherter Export der **V2**-Netzliste (die Wahrheit über den Bestand) | `hardware/esp32s3/v2/v2.xml`, erzeugt von `hardware/esp32s3/export_v2.py` |
| Gutachten | `hardware/review/` |
| Anforderungen, Entwurf und Aufgaben der Software-Seite | `specs/esp32s3-port/` |
| Arbeitsweise in KiCad, Fallen, Beleg-Verfahren | Skill `.claude/skills/kicad-schema/SKILL.md` |
| Bauteilwahl, Stücklistenformat, Prüfung an der Quelle | Skill `.claude/skills/jlcpcb-bauteile/SKILL.md` |
| Prüfungen der Hardware | `tools/checks/hardware.py`, `tools/checks/hardware-gegenproben.sh` und die Dateien daneben (5) |
| Der Bestand: V2 | `HARDWARE.md` (mit den Berichtigungen vom 17.10.2026), die V2-Netzliste ist massgebend |
| Befunde aus der Hardware-Arbeit | `BEFUNDE.md`, L485 bis L491, ToDo E42 bis E48 |
| Die Idee als Erweiterung | `ERWEITERUNGEN.md`, X15 (der Eintrag zur freigegebenen Spec folgt mit Task 7.5 der Spec) |

## 5. Welche Prüfungen es gibt

Die Hardware hat **zwei eigene Guardrail-Stufen** in `tools/guardrails.sh`. Beide sind statisch, sprechen
nicht mit einem Gerät und brauchen `kicad-cli` (Pfad im Skill `kicad-schema`); fehlt es, sagen sie es.

**S19 — `python3 tools/checks/hardware.py`** (aufrufbar auch einzeln, `--nur H1,H3`). Acht Prüfungen; jede
nennt, **wie viele Dinge sie betrachtet hat** (DIR-014, `BEFUNDE.md` L235 — eine Prüfung, die nichts sieht,
meldet sonst OK):

| | Prüft |
|---|---|
| H1 | ERC: keine Fehler, keine Meldungsart über der Baseline (`tools/checks/hardware-erc.baseline`) |
| H2 | Netzliste des Schaltplans gegen die Soll-Netzliste (`hardware/esp32s3/vergleich.py`, nach **Mengen von `Ref.Pin`**, benannte Netze halten zusätzlich ihren Namen) |
| H3 | Pinregeln am Modul `U2` (`tools/checks/hardware-pinregeln.json`): gesperrte Pins offen, feste Pins nur an ihrem Netz, kein 5-V-Netz an einem Modulpin, LDR an einem ADC1-Pin |
| H4 | Stückliste: jede Zeile eine LCSC-Nummer, Designatoren gleich den Bauteilen des Schaltplans, JLCPCB-Prüfung nicht zu alt |
| H5 | Komponentenblätter: Pflichtabschnitte vorhanden, an jedem ✔ eine Seiten- oder Tabellenangabe |
| H6 | der gespeicherte V2-Export stimmt noch mit dem V2-Schaltplan überein |
| H7 | Leiterplattenformat 1:1 gegen V2 (sobald ein Layout existiert) und DRC nicht schlechter als V2 (`tools/checks/hardware-drc.baseline`); fehlen `.kicad_pro` und `.kicad_dru` neben dem Layout, ist der DRC wertlos und die Prüfung meldet kritisch (`BEFUNDE.md`, L490) |
| H8 | LED-/Power-/LDR-Schnittstelle zum LED-Board bauteil- und pinweise gleich V2; Abweichungen nur mit Eintrag samt Grund in `tools/checks/hardware-led-schnittstelle.ausnahmen` **und Freigabe des Nutzers** |

Zeilen mit `HOCH` zählen als Warnung, `KRITISCH` blockiert (Exit 1).

**S20 — `tools/checks/hardware-gegenproben.sh`** fährt jede Prüfung aus S19 gegen eine **Kopie mit
hergestellter Verletzung** und verlangt, dass sie anschlägt; wo es eine Gegenrichtung gibt, muss die
unveränderte Kopie bestehen. Das ist DIR-014 als Stufe: Eine neu gebaute Prüfung ist erst fertig, wenn sie
einmal fehlgeschlagen ist. Die Umgebungsvariablen `HW_SCH`, `HW_NETLIST`, `HW_BOM`, `HW_PCB`, `HW_KOMP`,
`HW_V2` und `VERGLEICH_SCH` gibt es **nur** für diese Gegenproben — wer sie sonst setzt, prüft eine Kopie
statt des Entwurfs.

Die Dateien daneben: `hardware-erc.baseline` und `hardware-drc.baseline` (Zählung der V2-Meldungen je Art —
der Entwurf darf nicht schlechter sein), `hardware-pinregeln.json` (Regeln am Modul, mit Grund je Pin),
`hardware-led-schnittstelle.ausnahmen` (freigegebene Abweichungen am LED-Board-Weg). Eine Baseline mit
Meldungen ist **keine Zusicherung, dass V2 sauber sei** — sie hält fest, wie schlecht es war.

## 6. Die Lehre vom Anfang: `HARDWARE.md` war nicht die Wahrheit

`HARDWARE.md` entstand aus einer Abschrift und nicht aus dem Netzlistenexport. Beim ersten echten Export am
17.10.2026 stellte sich heraus, dass es an mehreren Stellen nicht stimmte oder unvollständig war: IR-Empfänger
und DCF77 haben auf dem Controller keinen Stecker, der Gate-Pull-down von `Q2` existiert, `SW4` liegt auch am
ESP, `H6` und `H3` speisen das 3,3-V-Netz ein, `SQW` und `32KHZ` der RTC liegen nur an Pull-ups, und die
Schaltkette der LED-Versorgung hat fünf Teile mehr als beschrieben. Jede Berichtigung steht datiert in
`HARDWARE.md`.

Daraus folgt für die Arbeit:

- **Vor jeder Aussage über einen Pin das Netz aus dem Export lesen**, nicht aus `HARDWARE.md`, nicht aus
  einem Auftragstext. Der Export entscheidet in beide Richtungen: Der Auftrag zu den Berichtigungen führte
  den LDR-Teiler unter den Fehlern, der Export bestätigte ihn aber (`HARDWARE.md`, „Nachgeprüft
  17.10.2026“) — berichtigt wurde nur, was der Export auch widerlegte.
- **Eine Stückliste ist keine Wahrheit über die Platine.** Die V2-`bom.csv` fehlt `R23` und `C10`, obwohl der
  Schaltplan sie führt (`BEFUNDE.md`, L485). Der Vergleich geschieht gegen den Schaltplan, und ob ein Teil auf
  der Platine sitzt, klärt der Nutzer am Gerät.
- **Eine Prüfung, die nur den Mechanismus prüft, prüft nicht den Weg der Meldung** — deshalb S20.
- **Ein DRC ohne Projektregeln ist nicht „etwas schlechter“, sondern falsch** (Standardregeln statt
  Projektregeln): Am V2-Layout waren das 621 statt 241 Meldungen (nachgestellt 17.10.2026 an einer Kopie,
  `BEFUNDE.md`, L490).

## 7. Stand

Datiert **17.10.2026**; den laufenden Stand zeigen `hardware/README.md` und `./tools/guardrails.sh`.

- Es gibt einen **Schaltplan-Entwurf** (aus der V2-Netzliste erzeugt: STM32- und ESP-12F-Teile entfernt,
  S3-Teile ergänzt) mit Soll-Netzliste, ERC und Netzlistenvergleich.
- Es gibt **Komponentenblätter** für die Teile des Umbaus und der Versorgung; die Auflistung steht in
  `hardware/komponenten/`, der JLCPCB-Bestand der V2-Stückliste in `jlcpcb-bestand.md`.
- **Noch nicht** da (am 17.10.2026): ein Layout, die Stückliste des Umbaus (H4 meldet das ausdrücklich),
  Entscheide in `hardware/architektur/`, ein Gutachten in `hardware/review/`.
- Die Spec `specs/esp32s3-port/` ist ein **Entwurf zur Freigabe** durch den Nutzer; ohne Freigabe beginnt keine
  Implementierung. `specs/multiboard-quelle/` (die eine Quelle für alle Boards) lag am selben Tag noch nicht vor.

## 8. Offene Punkte

Was aus der Hardware-Arbeit offen ist, steht als Befund mit Beleg und in der ToDo-Liste in `BEFUNDE.md`
(Guardrail S10 prüft, dass es dort steht):

| Befund | Thema |
|---|---|
| L485 / E42 | `R23` und `C10` fehlen in der V2-Stückliste; der Nutzer prüft an der Platine, ob sie bestückt sind und `JP2` geschlossen ist |
| L486 / E43 | `SW4`: der gemeinsame Kontakt ist nicht belegt |
| L487 / E44 | `R29` 32 gegen 31,6 kΩ; `D1`–`D3` Footprint in Stückliste und Teil verschieden |
| L488 / E45 | `Q2` ist ein MMBF170 von Shikues (Schwelle bis 2,5 V); bei einem Teilewechsel nicht mehr garantiert durchgeschaltet |
| L489 / E46 | SY8301 mit Rückkopplung hinter `D2`, Reserve bei USB-Volllast, TPS2121-Stromgrenze, Sicherungen: nur gerechnet, **am Prototyp zu messen** |
| L490 / E47 | Das V2-Layout trägt DRC-Meldungen, darunter 5 Fehler; die Grundlinie des Umbaus ist festzulegen |
| L491 / E48 | Die Spec lässt IR und DCF77 „bleiben“, der Controller hat keinen Stecker dafür — **Entscheid des Nutzers** |

Ausserdem, am 17.10.2026 in der Spec und den Blättern gelesen und noch nicht als Befund geführt, weil es
zur laufenden Spec gehört: die Freigabe der Spec (Task 0.6), die Aussenmasse des 16-poligen USB-C-Steckers
gegen den 6-poligen im Gehäuse und ein Testaufbau am DevKit (Spec F8). Bis 17.10.2026 stand hier auch
„die Wahl der Paarung für `R22` (47 kΩ gegen 10 kΩ)“. Das stimmt nicht mehr, weil `R22` 1 kΩ bleibt
(Vorgabe 6; `hardware/komponenten/esp32s3-wroom-1.md` §6.3).
Neu seit dem Abgleich vom 17.10.2026 und nicht als Befund geführt: die Konsole `H6` könnte in V3
gegenüber V2 vertauscht sein (OP-35, ○; Netzname `TX_UART` hängt in V2 am STM-Empfang PA10, V3 ordnet
nach dem Namen `TXD0` zu), und das LCSC-Feld von `R23` trägt in der V2-Stückliste die Nummer von `R19`
(C17841, OP-06). Beides in `hardware/dokumentation/offene-punkte.md`. Dazu meldete Prüfung H8 an diesem Tag ein zusätzliches Teil (`C32`) am Netz `LDR`, das
ohne Eintrag und Freigabe des Nutzers in `hardware-led-schnittstelle.ausnahmen` nicht stehen darf. Diese
Punkte stehen in `specs/esp32s3-port/` und `hardware/dokumentation/offene-punkte.md`; wer einen schliesst,
trägt den Beleg dort ein. Wird einer zum Mangel am Bestand oder am Werkzeug, bekommt er hier in
`BEFUNDE.md` die nächste freie `L`-Nummer.

## 9. Regeln für die Weiterarbeit

1. **V2 nur lesend.** Das Projekt unter `~/Documents` wird nie geschrieben, auch nicht „nur eine Zeile“.
2. **Netzliste vor Doku.** Pins und Verbindungen aus dem Export lesen (`hardware/esp32s3/export_v2.py`,
   `kicad-cli sch export netlist --format kicadxml`).
3. **Beleg-Zeichen setzen, Annahmen nicht zu Tatsachen machen.** Ein ○ bleibt ○, bis jemand das Datenblatt
   oder das Gerät gelesen hat.
4. **Jede neue Prüfung einmal scheitern lassen** (DIR-014) und die Zahl der betrachteten Fälle gegen das
   Erwartete halten.
5. **Keine Versionsnummern und keine Zählungen ohne Quelle** in lebende Dokumente (DIR-006); verweisen statt
   abschreiben.
6. **Abweichungen am LED-Board-Weg brauchen die Freigabe des Nutzers** — ein Eintrag in
   `hardware-led-schnittstelle.ausnahmen` ohne seine Freigabe ist keiner.
7. **Gerät und Hardware sind exklusiv (R5).** Der Nutzer flasht und bestückt; ein Agent bestellt nichts.
