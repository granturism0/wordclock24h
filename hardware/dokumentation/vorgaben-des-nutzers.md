# Vorgaben des Nutzers — vollständig, mit Datum, und was daraus folgt

Diese Liste wird bei **jeder** neuen Vorgabe ergänzt und **nie gekürzt**. Eine Vorgabe, die hier nicht steht, hat das Team nicht gehört.
Stand: **17.10.2026** (Erstfassung).

**Zur Wortlaut-Treue.** Das Gespräch, in dem der Nutzer die Vorgaben gemacht hat, liegt nicht im Repo. Im Wortlaut überliefert sind

- der **Grundstock** (Nr. 1–13) in der Rollenbeschreibung `.claude/agents/hw-doc-writer.md` (Stand 17.10.2026, vom Lead aus dem Gespräch übernommen),
- einzelne **Zitate** (kursiv, mit Fundstelle) in den Rollenbeschreibungen und im Skill,
- die **Entscheidungen E1–E12** in `specs/esp32s3-port/requirements.md`, vom `spec-writer` formuliert (Wortlaut der Spec, nicht des Nutzers).

Wo nur der Sinn überliefert ist, steht **„sinngemäss“**. Wer den Originalwortlaut hat, ersetzt die Zeile.

---

## A. Der Grundstock (17.10.2026, im Wortlaut der Rollenbeschreibung)

| Nr. | Vorgabe | Datum |
|---|---|---|
| 1 | Umbau des jetzigen Prints und seiner Funktion **1:1** auf einen ESP32-S3. | 17.10.2026 |
| 2 | ESP32-S3-WROOM-1, **nicht** das DevKit als Zielprodukt; die Software muss **auch auf dem DevKitC-1** laufen, deshalb nur Pins, die das DevKit am Header herausführt. | 17.10.2026 |
| 3 | **Ein** USB-C-Anschluss genügt; Flashen, Konsole und JTAG darüber. | 17.10.2026 |
| 4 | **LDR und LEDs bleiben auf dem LED-Board**; dasselbe LED-Board wird weiterverwendet. | 17.10.2026 |
| 5 | **LED-Ansteuerung, Power-Schalten und LDR-Lesung bleiben 100 % pin- und funktionskompatibel.** (Die Software-Seite derselben Vorgabe führt `specs/multiboard-quelle/requirements.md` als V11: Bitreihenfolge G, R, B, W, Zeiten 1250/300/750 ns, Pause 250 µs, 200 ms Einschwingzeit im Aufrufer, Rohwerte 0 … 4095 der STM-Skala; Quelle dort „Nutzer über Lead, 17.10.2026“.) | 17.10.2026 |
| 6 | Der **LDR hat einen analogen Eingang**. | 17.10.2026 |
| 7 | **Das Leiterplattenformat bleibt 1:1.** | 17.10.2026 |
| 8 | **Fertigung und Bestückung bei JLCPCB**; jedes Teil dort erhältlich, **wenn möglich aus dem Standard-Katalog**. | 17.10.2026 |
| 9 | **Schaltplan und Layout in KiCad.** | 17.10.2026 |
| 10 | Ein spezialisiertes Hardware-Team mit **Reviews und Guardrails**; Komponentenspezifikationen werden gelesen, verstanden und analysiert. | 17.10.2026 |
| 11 | **Eine Quelle für alle Boards**, die das neue abdeckt, damit nichts auseinanderdriftet; die LED-Logik so weit wie möglich übernehmen. | 17.10.2026 |
| 12 | **Alles Wissen wird dokumentiert**, damit nichts nur im Chat steht. | 17.10.2026 |
| 13 | **100 %ige Professionalität.** | 17.10.2026 |

## B. Nachträge zum Grundstock

| Nr. | Vorgabe | Datum | Fundstelle |
|---|---|---|---|
| 14 | **`R22`, der Teilerwiderstand des LDR, bleibt 1 kΩ**, solange der Nutzer keine Abweichung freigibt. Ein früherer Vorschlag von rund 47 kΩ **entfällt**. (Konkretisierung von Nr. 5.) *sinngemäss*; die Formulierung stammt aus dem Dokumentationsauftrag des Leads vom 17.10.2026 | 17.10.2026 | Auftrag an den Dokumentar; umgesetzt in `hardware/esp32s3/gen.py` (Kommentar „R22 bleibt V2-Wert 1 kOhm … Vorgabe 17.10.2026“), geprüft von H8 |
| 15 | **Jede Abweichung der LED-/Power-/LDR-Schnittstelle von V2 braucht die Freigabe des Nutzers**; ein Eintrag mit Grund in `tools/checks/hardware-led-schnittstelle.ausnahmen` hält sie fest. **Abgeleitet** aus Nr. 5 und Nr. 14: Das Verfahren hat der Lead in die Prüfung H8 gelegt, es ist **kein Wortlaut des Nutzers**. Wortlaut der Datei: „Ein Eintrag hier heisst: Der Nutzer hat diese Abweichung gesehen und freigegeben.“ | 17.10.2026 | `tools/checks/hardware-led-schnittstelle.ausnahmen`, Kopfzeilen |

## C. Zitate des Nutzers mit Fundstelle

| Datum | Zitat | Fundstelle | Wofür |
|---|---|---|---|
| 17.10.2026 | *„Alle Erkenntnisse sind zu dokumentieren und festzuhalten, damit wir keinen Chat-Drain haben.“* (im Dokumentationsauftrag: *„alles Wissen, alle Erkenntnisse sind zu dokumentieren und festzuhalten, damit wir keinen Chat-Drain haben“*) | `.claude/agents/hw-doc-writer.md`, Z. 21–22; Auftrag an den Dokumentar | Nr. 12; Grund dieses Verzeichnisses |
| 17.10.2026 | *„wenn möglich immer noch die gleiche Source für alle Varianten, damit wir nicht plötzlich auseinander driften — eine Entwicklung für alle Boards, welche aber auch das neue abdeckt“* (Originalwortlaut laut `specs/multiboard-quelle/requirements.md`, „Auslöser“); die Rollenbeschreibung gibt es geglättet wieder: *„Wenn möglich dieselbe Quelle für alle Varianten, damit wir nicht auseinanderdriften — eine Entwicklung für alle Boards, die auch das neue (ESP32-S3) abdeckt.“* | `specs/multiboard-quelle/requirements.md`; `.claude/agents/sw-architect.md` | Nr. 11 |
| 17.10.2026 | *„LED-Ansteuerungslogik so weit wie möglich übernehmen“* | `specs/multiboard-quelle/requirements.md` („Auslöser“, V2) | Nr. 11; E11 |
| 17.10.2026 | Der Nutzer verlangt *„100%ige Professionalität“* | `specs/multiboard-quelle/requirements.md` („Auslöser“) | Nr. 13 |
| 17.10.2026 | Die Platine wird bei JLCPCB **gefertigt und bestückt**. Jedes Bauteil muss dort erhältlich sein, **wenn möglich aus dem Standard-Katalog (Basic)**. | `.claude/skills/jlcpcb-bauteile/SKILL.md` („Vorgabe des Nutzers vom 17.10.2026“) | Nr. 8 |
| 17.10.2026 | Das **Format der Platine** bleibt gleich: Umriss, Befestigung, Lage der Stecker. | `.claude/skills/kicad-schema/SKILL.md` („Vorgabe vom 17.10.2026“) | Nr. 7 |
| 14.10.2026 | *„Somit ist eigentlich alles erlaubt und dies auch zu jeder Zeit.“* (über die Test-Uhr im Keller; **nicht** über das Hardware-Team, das nie am Gerät arbeitet, R5) | `CLAUDE.md`, R5 | Rahmen: Test-Uhr statt Wohnraumgerät |

## D. Die Entscheidungen E1–E12 (Spec `esp32s3-port`, 17.10.2026)

Der `spec-writer` führt sie als „Festgelegte Entscheidungen des Nutzers (nicht neu verhandelt)“. Kurzfassung hier; massgebend ist der Wortlaut in `specs/esp32s3-port/requirements.md`, Abschnitt gleichen Namens.

| # | Inhalt (Kurzfassung) | Hardware-Folge |
|---|---|---|
| E1 | Zielprodukt ist ein **Nachbau der Platine** mit dem Modul ESP32-S3-WROOM-1; dieselbe Software muss auf dem DevKitC-1 v1.1 laufen | Modul `U2` |
| E2 | **Nur GPIOs, die das DevKit am Header herausführt**; **nicht** GPIO0, 3, 45, 46, 19/20, 43/44, 35–37, 38, 48 | H3 sperrt sie; GPIO0 nur an `BOOT`, 19/20 nur USB, 43/44 nur UART0 |
| E3 | **Ein** USB-C-Anschluss mit Datenleitungen an GPIO19/20 (Flashen, Konsole, JTAG); die Konsole läuft **zusätzlich** über UART0; der 6-polige Stecker wird ein 16-poliger, **plus ESD-Schutz** | `USB1` 16P, `U11` |
| E4 | EEPROM bleibt der AT24C32 am I²C, Datenformat unverändert; RTC bleibt der DS3231 | `U5`, `U6` unverändert |
| E5 | Die Kommandobrücke bleibt als Schicht (zwei Tasks mit Warteschlange); `pending_ticker_restore` wird nicht vereinfacht | Software, keine Hardware |
| E6 | **Entfällt:** der STM32-Flash-Weg | kein STM32, kein SWD, kein `H3` |
| E7 | Reihenfolge: zuerst LED-Ausgabe isoliert, dann I²C/EEPROM/RTC, dann Sensoren, zuletzt Brücke | Testaufbau am DevKit |
| E8 | **1:1-Umbau**: kein Funktionsverlust, keine neuen Funktionen | Massstab für jede Änderung |
| E9 | Zwei **Muss-Kriterien**: **(a)** analoger LDR-Eingang (`GPIO1`, ADC1; **„`R22` wechselt von 1 kΩ auf ca. 47 kΩ“ — durch Nr. 14 überholt, siehe E)**; **(b)** Flashen, Konsole, JTAG über den einen USB-C-Anschluss | `R22`, `C32`; `USB1`, `U11`, `SW5` |
| E10 | **LDR und LEDs bleiben auf dem LED-Board**, über den gespiegelten `H7`; das LED-Board wird nicht geändert | Prüfung H8 |
| E11 | Wiederverwendung der LED-Ansteuerungslogik; Treiber-API als Naht | Software |
| E12 | **Das Leiterplattenformat bleibt 1:1** | Prüfung H7 |

## E. Überholte Festlegungen (Korrekturen)

**`R22` — von rund 47 kΩ zurück auf 1 kΩ.** Bis 17.10.2026 stand in `specs/esp32s3-port/requirements.md` (E9(a), H9, F15), in `design.md` (§2, §4.2) und im Komponentenblatt `jlcpcb-bestand.md`
(„Neue Werte für den Umbau“), `R22` wechsle von 1 kΩ auf rund 47 kΩ, damit der ADC das Dunkel auflösen kann. **Das stimmt nicht mehr, weil** der Nutzer am 17.10.2026 die 100-%-Kompatibilität des LDR-Pfads verlangt hat (Nr. 5, Nr. 14);
`R22` liegt in diesem Pfad. Die Spec ist **nicht nachgezogen** (Übergabeliste im Bericht des Dokumentars). Die technische Folge steht in [offene-punkte.md](offene-punkte.md), OP-02.

## F. Was aus den Vorgaben folgt (Tabelle)

| Vorgabe | Folge für den Entwurf | Prüfung / Beleg |
|---|---|---|
| 1, E8 | Kein Funktionsverlust, keine neue Funktion; ein Teil, das mehr könnte, wird nicht „nebenbei“ verbessert | Review; `aenderungen-gegenueber-v2.md` |
| 2, E1, E2 | Pin-Sperrliste (Strapping, USB, UART0, Flash/PSRAM, DevKit-LED) | H3, `hardware-pinregeln.json` |
| 3, E3 | 16-poliger Stecker, ESD, BOOT/EN-Taster als Rückweg | H3; Spec AK3, AK3a, AK3b |
| 4, 5, E10 | LED-Board bleibt; Pfad `U3`, `R11`, `Q1`, `Q2`, `R19`–`R23`, `R22` bleibt wie V2 | **H8**, Gegenproben P15–P19 |
| 6 | LDR an einem ADC1-Pin (GPIO1) | H3 (`adc1_pins`) |
| 7, E12 | Umriss, Löcher, Stecker, Taster wie V2; Layout beginnt mit einer Kopie von `V2.kicad_pcb` | **H7**, Gegenprobe P13/P14 |
| 8 | Jede Stücklistenzeile mit geprüfter LCSC-Nummer; Basic zuerst; Extended nur mit Grund | H4; Skill `jlcpcb-bauteile`; `komponenten/jlcpcb-bestand.md` |
| 9 | KiCad-Dateien; Generator als Quelle des Schaltplans | `hardware/esp32s3/gen.py` |
| 10 | Analyst → Architekt → Designer → Reviewer; Beleg-Zeichen an jeder Zahl; DIR-014 für jede Prüfung | `CLAUDE.md`, R3/R3b; H5; `hardware-gegenproben.sh` |
| 11 | Pins stehen im Code an **einer** Stelle (Board-Beschreibung) | Spec `specs/multiboard-quelle/` (**nur `requirements.md` als Entwurf, nicht freigegeben**, OP-24) |
| 12 | Dieses Verzeichnis | `hardware/dokumentation/` |
| 13 | Keine Behauptung ohne Beleg; eine Annahme bleibt Annahme | Beleg-Zeichen; offene Punkte |
| 14, 15 | `R22` = 1 kΩ; Abweichungen nur mit Freigabe | H8, `…ausnahmen` (Stand: leer) |

## G. Dauerhafte Rahmenvorgaben aus `CLAUDE.md`, die das Hardware-Team berühren

(Nicht neu vom Nutzer, aber bindend; hier nur, damit der Leser sie nicht suchen muss.)

- **Sprache:** Deutsch, **Du-Form**, echte Umlaute, Schweizer „ss“ (DIR-001) — in Berichten und Dokumentation. In KiCad-Dateien gilt Umschrift in Bezeichnern und Netznamen (DIR-015).
- **Das V2-Projekt unter `~/Documents` bleibt nur lesend**, für alle Rollen.
- **Kein `make`** für die Hardware-Rollen (R1); **kein Gerät** (R5): Hardware-Entwurf braucht keines.
- **Jede Datei hat genau einen Schreiber** (R3): siehe Tabelle in [../README.md](../README.md). Der Besitz-Hook ist eine Erinnerung, kein Zwang.
- **DIR-014:** Eine neu gebaute Prüfung ist erst fertig, wenn sie einmal fehlgeschlagen ist — und ihre Meldung ankommt.
- **DIR-006:** In lebende Dokumente gehören keine Versionsnummern und keine abgeschriebenen Zählungen ohne Quelle.
