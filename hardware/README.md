# hardware — Umbau der Controller-Platine auf einen ESP32-S3

Stand: **17.10.2026** (Erstfassung des Hardware-Dokumentars). Dieses Verzeichnis trägt die Hardware-Arbeit am Umbau der **Controller-Platine** der WordClock
von STM32F411 + ESP-12F (V2) auf **einen** ESP32-S3-WROOM-1 (V3): Schaltplan, Bauteilblätter mit Datenblattbelegen, Entscheide, Gutachten und — später — das Layout.
Das **LED-Board bleibt unverändert**, das **Leiterplattenformat bleibt 1:1**, gefertigt und bestückt wird **bei JLCPCB** in KiCad (Vorgaben vom 17.10.2026, vollständig und im Wortlaut in
[dokumentation/vorgaben-des-nutzers.md](dokumentation/vorgaben-des-nutzers.md)).

**Wer hier einsteigt, liest in dieser Reihenfolge:** 1. diese Datei, 2. [dokumentation/vorgaben-des-nutzers.md](dokumentation/vorgaben-des-nutzers.md), 3. [dokumentation/wissensstand.md](dokumentation/wissensstand.md),
4. [dokumentation/offene-punkte.md](dokumentation/offene-punkte.md). Alles, was das Team weiss und beschliesst, steht in einer Datei, nicht im Gesprächsverlauf.

**Die Hardware ist noch nicht gebaut.** Es gibt einen Schaltplan-Entwurf, keine Stückliste, kein Layout, kein Gerät. Jede Aussage über Hardware ist Papier, belegt durch Datenblatt, Netzliste oder Prüfung.

## Verzeichnisbaum

```
hardware/
├── README.md                          diese Datei (Schreiber: hw-doc-writer)
├── dokumentation/                     das gesammelte Wissen (Schreiber: hw-doc-writer)
│   ├── wissensstand.md                gesicherte Erkenntnisse mit Beleg, Korrekturen früherer Annahmen
│   ├── aenderungen-gegenueber-v2.md   was entfällt, neu ist, sich ändert — mit Grund
│   ├── vorgaben-des-nutzers.md        jede Vorgabe im Wortlaut, mit Datum, und was daraus folgt
│   ├── entscheidungsuebersicht.md     entschieden / offen / wartet auf …
│   ├── ablaeufe.md                    prüfen, Schaltplan erzeugen, flashen, bestellen — mit Befehlen
│   ├── offene-punkte.md               was nicht belegt ist, wer es klärt, was daran hängt
│   ├── chronik.md                     datierter Verlauf
│   └── glossar.md                     Begriffe und Kürzel
├── architektur/                       Entscheide, Pinbelegung, Leistungsbudget (Schreiber: hw-architect) — zur Zeit leer
├── komponenten/                       je Bauteil ein Komponentenblatt mit Datenblattbelegen (Schreiber: hw-datasheet-analyst)
│   ├── <bauteil>.md                   Gliederung siehe dokumentation/ablaeufe.md, Abschnitt 7
│   ├── jlcpcb-bestand.md              Klasse, Lager und Prüfdatum der V2-Stückliste bei JLCPCB
│   └── datenblaetter/                 die gelesenen PDFs (NICHT im Git, siehe .gitignore)
├── esp32s3/                           Schaltplan und Werkzeug (Schreiber: hw-designer; Layout: hw-layouter)
│   ├── WordClock-ESP32S3-V3.kicad_sch Schaltplan-Entwurf (vom Generator erzeugt)
│   ├── WordClock-ESP32S3-V3.kicad_pro/.kicad_prl   KiCad-Projekt
│   ├── gen.py, sx.py                  Generator und Parser: V2-Netzliste → V3-Schaltplan und Soll-Netzliste
│   ├── expected.json                  Soll-Netzliste (wird von gen.py geschrieben)
│   ├── vergleich.py, erc.sh           Netzlistenvergleich nach Pin-Mengen; ERC
│   ├── export_v2.py, v2/              Export der V2-Netzliste (nur lesend am V2-Projekt)
│   ├── flags.json, erc.json           PWR_FLAG-Stellen; letzter ERC-Bericht
│   └── (später) *.kicad_pcb, *.kicad_dru, layout/, fertigung/, bom.csv
└── review/                            Gutachten (Schaltplan, Layout) (Schreiber: hw-reviewer) — zur Zeit leer
```

Die **Prüfwerkzeuge** liegen ausserhalb, in `tools/checks/` (Schreiber: Lead): `hardware.py` (Prüfungen H1–H8), `hardware-gegenproben.sh`, `hardware-pinregeln.json`, `hardware-erc.baseline`,
`hardware-drc.baseline`, `hardware-led-schnittstelle.ausnahmen`. Die Besitzprüfung der Schreibzugriffe ist `tools/hooks/file-ownership.py`.

Verwandt, aber ausserhalb: `specs/esp32s3-port/` (Spec des Ports, `spec-writer`), `specs/multiboard-quelle/` (eine Quelle für alle Boards, `sw-architect`), die Skills `.claude/skills/kicad-schema/` und `.claude/skills/jlcpcb-bauteile/`,
die Rollenbeschreibungen `.claude/agents/hw-*.md` und `sw-architect.md`, `HARDWARE.md` (Bestandsaufnahme von V2; am 17.10.2026 gegen die Netzliste berichtigt, was die Berichtigung noch nicht trägt, steht in [dokumentation/wissensstand.md](dokumentation/wissensstand.md), W-2), `knowledge/hardware-esp32s3-umbau.md` (lebendes Rahmendokument des Lead: Rahmen, Rollen, Prüfungen, Lehre vom Anfang) und `BEFUNDE.md` (L485–L491, ToDo E42–E48).

## Rollen — wer schreibt was

Quelle: `CLAUDE.md` (R3-Tabelle) und `tools/hooks/file-ownership.py`. **Jede Datei hat genau einen Schreiber**; der Besitz-Hook ist eine Erinnerung, kein Zwang — fremde Dateien bleiben tabu, auch wo er sie durchliesse.

| Rolle | Schreibt | Aufgabe |
|---|---|---|
| `hw-datasheet-analyst` | `hardware/komponenten/**` | liest Datenblätter, liefert je Bauteil ein belegtes Blatt (✔ mit Seite, ● Sekundärquelle, ○ Annahme) |
| `hw-architect` | `hardware/architektur/**` | Funktionsblöcke, Pinzuordnung, Versorgung, Boot und USB; Entscheidungsprotokoll |
| `hw-designer` | `hardware/esp32s3/**` ausser Layout | Schaltplan, Generator, ERC, Netzlistenvergleich, Stückliste |
| `hw-layouter` | `hardware/esp32s3/*.kicad_pcb`, `*.kicad_dru`, `layout/`, `fertigung/` | Platzierung, Leiterbahnen, DRC, Fertigungsdaten für JLCPCB |
| `hw-reviewer` | `hardware/review/**` | Gutachten; ändert den Entwurf nie |
| `hw-doc-writer` | `hardware/dokumentation/**`, `hardware/README.md` | dokumentiert, entscheidet nichts |
| Lead | `tools/**`, `.claude/**`, Doku ausser `hardware/`, Build | Prüfwerkzeug, Rollen, Freigaben in der Ausnahmendatei |
| Nutzer | — | Vorgaben, Freigaben, **Bestellung**; Flashen und Messen am Gerät |

**Reihenfolge (R3b):** Analyst → Architekt → Designer → Reviewer; der Designer zeichnet nichts, was der Architekt nicht entschieden hat, und der Architekt entscheidet keine Zahl, die der Analyst nicht belegt hat.
Der Dokumentar arbeitet nach **jedem** Schritt. Das **V2-Projekt unter `~/Documents` bleibt für alle nur lesend.** Kein Hardware-Agent führt `make` aus (R1) oder arbeitet an der Uhr (R5).

## Den Stand prüfen

```
python3 tools/checks/hardware.py                 acht Prüfungen H1–H8 (ERC, Netzliste, Pinregeln, Stückliste, Blätter, V2-Referenz, Format/DRC, LED-Schnittstelle)
python3 tools/checks/hardware.py --nur H1,H3     einzelne
./tools/checks/hardware-gegenproben.sh           Gegenproben: jede Prüfung gegen eine hergestellte Verletzung (DIR-014)
./tools/guardrails.sh                            alle Stufen; die Hardware sind S19 (Prüfungen) und S20 (Gegenproben)
```

Exit 0 heisst: kein KRITISCH-Befund. **HOCH-Zeilen sind Warnungen, die man trotzdem liest.** Jede Prüfung nennt, **wie viele Dinge sie betrachtet hat** — vergleiche die Zahl mit dem, was es geben müsste.
Einzelheiten, Lesart und was bei einem Fund zu tun ist: [dokumentation/ablaeufe.md](dokumentation/ablaeufe.md). Den **aktuellen** Stand zeigt immer die Prüfung, nicht diese Datei (DIR-006).

## Regeln der Arbeit (Kurzfassung)

- **Die V2-Netzliste ist die Wahrheit über den Bestand, nicht `HARDWARE.md`** (siehe W-1, W-2 im Wissensstand).
- **Jede Zahl trägt ein Beleg-Zeichen**; ein Wert aus dem Gedächtnis ist keiner; aus einer Annahme wird nie eine Tatsache.
- **LED-Ansteuerung, Power-Schalten und LDR-Lesung bleiben 100 % pin- und funktionskompatibel** (Prüfung H8); `R22` bleibt 1 kΩ; Abweichungen nur mit Freigabe des Nutzers und Eintrag mit Grund.
- **Das Format bleibt 1:1** (Prüfung H7): Umriss, Löcher, Stecker, Taster.
- **Jedes Teil bei JLCPCB lieferbar, wenn möglich Basic;** jede Stücklistenzeile mit an der Quelle geprüfter LCSC-Nummer und Prüfdatum.
- **Eine Prüfung ist erst fertig, wenn sie einmal fehlgeschlagen ist** (DIR-014).
- **Sprache:** Deutsch, Du-Form, echte Umlaute, Schweizer „ss“ in Berichten und Dokumentation (DIR-001); in KiCad-Dateien Umschrift in Bezeichnern (DIR-015). Keine Versionsnummern und keine abgeschriebenen Zählungen ohne Quelle in lebenden Dokumenten (DIR-006).

## Was es noch nicht gibt

Layout, V3-Stückliste, Entscheidungsprotokoll, Gutachten, Gerät. Die vollständige, jeweils aktuelle Liste führt [dokumentation/offene-punkte.md](dokumentation/offene-punkte.md).
