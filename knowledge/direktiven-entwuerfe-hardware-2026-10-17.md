# Direktiven-Entwürfe zur Hardware-Arbeit — Stand 17.10.2026, **nicht bestätigt**

Entwurf des `librarian` aus den Äusserungen des Nutzers vom 17.10.2026. **Nichts davon steht im Katalog
(`directives.md`).** Der Katalog bekommt einen Eintrag erst, wenn der Nutzer ihn ausdrücklich bestätigt; eintragen
tut der Lead. Die Entwürfe tragen deshalb bewusst **keine DIR-Nummer** (Guardrail S11 prüft, dass jede zitierte
Kennung im Katalog steht).

**Wo die Äusserungen sonst festgehalten sind:** wörtlich in `hardware/dokumentation/vorgaben-des-nutzers.md`
(Hardware-Dokumentar), als Projektvorgaben in `specs/esp32s3-port/requirements.md` (E1–E12).

## Sortierung der vierzehn Äusserungen

| Äusserung (Kurzfassung) | Zuordnung |
|---|---|
| 1:1-Umbau auf ESP32-S3; LDR analog, Flashen über USB-C; LDR und LEDs auf dem LED-Board; Format der Platine 1:1; LED/Power/LDR 100 % pin- und funktionskompatibel; LED-Logik so weit wie möglich übernehmen | **Projektvorgabe** dieses Umbaus, keine Direktive (sie gälte sonst für jede künftige Hardware, wo sie falsch wäre) |
| Bauteile bei JLCPCB erhältlich, wenn möglich Standard-Katalog | dauerhaft, **Entwurf A** |
| Team liest und analysiert Komponentenspezifikationen | dauerhaft, **Entwurf B** |
| Reviews und Guardrails beim Hardware-Design | dauerhaft, **Entwurf C** |
| KiCad ist das Standardwerkzeug | dauerhaft, aber kaum verletzbar, **Entwurf D** (optional) |
| Alles Wissen dokumentieren, kein Chat-Drain | **bestehende Regel zur lebenden Dokumentation erweitern** (DIR-006), keine neue |
| Eine Quelle für alle Boards | **CLAUDE.md** (Architektur-Invariante), nach der Entscheidung von `specs/multiboard-quelle/` |
| Vorgehen und Team überlässt der Nutzer dem Lead, 100 % Professionalität | ein Auftrag, keine Regel; „Professionalität“ ist nicht prüfbar. Was sie bedeutet, steckt in A bis C |

## Entwurf A — Bauteile aus JLCPCB, mit LCSC-Nummer

> Jedes Bauteil einer Platine, die gefertigt werden soll, ist bei JLCPCB bestückbar und trägt eine LCSC-Nummer in
> der Stückliste; eine Zeile ohne Nummer gilt als Fehler. Bevorzugt kommen Bauteile aus dem JLCPCB-Standardkatalog
> (Basic, danach Preferred). Ein Bauteil ausserhalb davon (Extended) braucht eine Begründung in der Stückliste oder im
> Komponentenblatt, und der Nutzer erfährt es vor der Bestellung. Der Lagerstand wird vor jeder Bestellung neu geprüft.

- Gilt für: `hw-designer`, `hw-datasheet-analyst`, `hw-reviewer`, `hw-architect`, `hw-layouter`.
- Geprüft von: `tools/checks/hardware.py` H4 (LCSC-Nummer je Zeile, Designatoren gleich Bauteile, Alter der Prüfung
  über 14 Tage als Warnung).
- **Lücke:** H4 unterscheidet **Basic gegen Extended nicht**, und das Alter der Prüfung ist nur eine Warnung. Der Teil
  „Standardkatalog“ und „Bestand frisch“ ist also **nicht erzwungen**. Vor der Bestätigung: H4 um die Klassenprüfung
  erweitern und eine Gegenprobe in S20 bauen.
- Frage an den Nutzer: Gilt das für **jede** künftige Platine oder nur für diese? Die Äusserung begründet es mit dem
  Fertigungsort.

## Entwurf B — Kein Bauteil ohne gelesenes Datenblatt

> Kein Bauteil wird verwendet, bevor sein Datenblatt gelesen und im Komponentenblatt (`hardware/komponenten/`)
> analysiert ist: Pflichtabschnitte vorhanden, jede Aussage mit Datenblattseite oder Tabelle belegt. Was nicht gelesen
> wurde, steht als „nicht gelesen“ da, nicht als Tatsache. Massgebend für den Bestand ist die exportierte Netzliste,
> nicht `HARDWARE.md`, solange die Netzliste die Aussage nicht bestätigt.

- Gilt für: `hw-architect`, `hw-datasheet-analyst`, `hw-designer`, `hw-reviewer`, `hw-doc-writer`.
- Geprüft von: H5 (Pflichtabschnitte, Seitenbeleg an jedem Haken), H6 (V2-Export gegen V2-Schaltplan), H4 (LCSC-Nummern
  ohne Beleg als Warnung).
- **Lücke:** „Haken ohne Seitenangabe“ ist nur eine Warnung; dass **jedes verwendete** Bauteil ein Blatt hat, ist nur
  teilweise erzwungen.
- Anlass: `HARDWARE.md` hat sich am 17.10.2026 an mehreren Stellen als falsch erwiesen (IR und DCF77 ohne Stecker,
  Gate-Pull-down von Q2 vorhanden, `SW4` an `H6`).

## Entwurf C — Hardware-Entwürfe nur mit Review und Guardrails

> Hardware-Entwürfe werden nur mit Review und Guardrails freigegeben. Entwurf und Review liegen bei verschiedenen
> Rollen (`hw-designer` bzw. `hw-layouter` schreiben, `hw-reviewer` beurteilt); der Designer gibt sich nicht selbst
> frei. Vor jeder Bestellung und jedem Layout-Stand läuft `./tools/guardrails.sh` mit S19 und S20. Jede neue
> Hardware-Prüfung gilt erst als fertig, wenn S20 sie einmal in beide Richtungen hat fehlschlagen lassen (DIR-014).

- Gilt für: `hw-designer`, `hw-layouter`, `hw-reviewer`, `hw-architect`.
- Geprüft von: S19 (`hardware.py` H1–H8) und S20 (`hardware-gegenproben.sh`). DIR-014 wird verwiesen, nicht dupliziert.
- **Lücken:** Dass der Designer sich nicht selbst freigibt, ist nicht per Hook erzwungen. H4 meldet eine fehlende
  Stückliste nur als Warnung; H7 greift erst mit einem Layout. Die ERC-Baseline kann Altlasten dauerhaft verdecken; ihre
  Zahl gehört in jeden Bericht.

## Entwurf D — Schaltplan und Leiterplatte in KiCad (optional)

> Schaltplan und Leiterplatte werden in KiCad geführt; die Projektdateien liegen unter `hardware/`. Das V2-Projekt unter
> `~/Documents` bleibt nur lesend, gearbeitet wird an Kopien. Keine Zweitquelle in einem anderen Format.

- Der Librarian rät **nicht** dazu: Es ist eine Werkzeugwahl, kaum verletzbar. Ein Satz im Skill `kicad-schema`
  genügt (steht dort schon).

## Änderung an einer bestehenden Regel — Dokumentation (DIR-006)

`hardware/**` steht in keiner der beiden Listen (lebend, Momentaufnahme). Vorschlag: **`hardware/**` ausdrücklich als
lebend führen** und `hw-doc-writer` in den Geltungsbereich aufnehmen; ein Satz: *Eine Erkenntnis, die nur im Chat steht,
gilt als nicht festgehalten.*

**Widerspruch Praxis gegen Regel:** `specs/**` gilt nach DIR-006 als Momentaufnahme, `specs/esp32s3-port/` wird aber als
Arbeitsstand fortgeschrieben. Entweder wird die Spec nach der Freigabe zur datierten Momentaufnahme, oder die laufende
Projektvorgabe zieht in ein lebendes Dokument um.

## Offene Entscheidungen des Nutzers

1. Entwurf A: für jede künftige Platine oder nur für diese?
2. Entwurf D: als Direktive oder nur im Skill?
3. DIR-006: `hardware/**` als lebend aufnehmen und `hw-doc-writer` in den Geltungsbereich?
4. Wohin gehört die laufende Projektvorgabe des Umbaus (lebendes Dokument, etwa `hardware/dokumentation/` oder
   `ERWEITERUNGEN.md`)?
5. Entwurf C: Soll die Trennung von Entwurf und Review per Hook erzwungen werden (Arbeit des Lead in `tools/hooks/`)?
