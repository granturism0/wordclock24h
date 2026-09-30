# Direktiven

Verbindliche Regeln, die aus Nutzeräusserungen abgeleitet und **ausdrücklich bestätigt**
wurden. Gepflegt vom `librarian`-Agenten. Nichts steht hier ohne Bestätigung.

Format:

```
DIR-000:
  regel: "..."
  gilt_fuer: [agent1, agent2]
  seit: JJJJ-MM-TT
```

---

DIR-001:
  regel: "Antworten an den Nutzer und alle deutschen UI-Texte durchgehend in der Du-Form, niemals Sie."
  gilt_fuer: [alle]
  seit: 2026-08-12

DIR-002:
  regel: "Nach jeder relevanten Änderung vollständig bauen und ein Release-ZIP erzeugen, nicht nur app-gz. Immer explizit sagen, was zu flashen ist: nur App/LittleFS, oder auch STM bzw. ESP."
  gilt_fuer: [release-engineer]
  seit: 2026-08-12

DIR-003:
  regel: "Beim Thema der sporadischen F411-Hänger keinen pauschalen DMA-Fix und keinen Recovery-Mechanismus einbauen. Erst per gezielter Instrumentierung erhärten."
  gilt_fuer: [firmware-analyst, stm-developer]
  seit: 2026-08-12

DIR-004:
  regel: "Jede Komponente wird genau dann versioniert, wenn sich IHR Code geaendert hat —
          kein Gleichschritt. Aendert ein Release nur den STM-Code, steigt nur VERSION; ESP und
          PWA bleiben stehen. Quellen: STM = src/**, CMakeLists.txt, cmake/**; ESP =
          ESP8266/ESP-uclock/*.cpp|*.h|*.ino ohne data/; PWA = data/app/** ohne .gz. Beide
          Richtungen sind falsch: Code geaendert ohne Bump macht das Fabrikat unzuordenbar,
          Bump ohne Codeaenderung bietet ein OTA-Update auf identische Firmware an.
          APP_VERSION und CACHE_NAME gehoeren unabhaengig davon immer zusammen."
  gilt_fuer: [release-engineer, pwa-developer, ui-developer, stm-developer, esp-developer]
  seit: 2026-09-29
  geaendert: 2026-09-29 — vorher Gleichschritt aller drei Komponenten, auf Wunsch des Nutzers
             auf komponentenweise Versionierung umgestellt.

DIR-008:
  regel: "Die produktive Uhr unter http://192.168.1.184/app/ darf LESEND abgefragt werden
          (GET auf /api/-Endpunkte, die nur Zustand liefern). VERBOTEN ohne ausdrueckliche
          Freigabe des Nutzers im selben Gespraech: jeder schreibende Endpunkt, jeder Flash,
          jeder Reset, Backup-Import, maintenance_reset_eeprom, maintenance_format_fs,
          fs_remove, test_display (45 s Watchdog-Reset), learn_ir (unbegrenzter Reset).
          NIE senden: eine URL mit Parameter ohne Gleichheitszeichen, etwa GET /?a --
          das stuerzt den ESP nachweislich ab (Review 2, Kernbefund 3). Grund: Es ist die
          Uhr des Nutzers im Dauerbetrieb, nicht ein Testgeraet."
  gilt_fuer: [alle]
  seit: 2026-09-30

DIR-007:
  regel: "R1 (nur der release-engineer baut) ist per Hook erzwungen, nicht nur
          aufgeschrieben: tools/hooks/no-build.py haengt im Frontmatter aller schreibenden
          Agenten ausser release-engineer und weist make, cmake, arduino-cli sowie
          guardrails.sh --full ab. Zusaetzlich blockiert ein Stop-Hook
          (tools/hooks/guardrails-before-stop.py) das Turn-Ende, wenn ueberwachte Dateien
          geaendert sind und ./tools/guardrails.sh dafuer nicht lief."
  gilt_fuer: [alle]
  seit: 2026-09-30

DIR-006:
  regel: "Dokumentation zerfaellt in LEBEND (CLAUDE.md, BEFUNDE.md, CHANGELOG.md, alle
          README*.md, knowledge/**, .claude/agents/**) und MOMENTAUFNAHME (REVIEW*.md,
          gap-analysis.md, specs/**). Lebende Dokumente werden nachgefuehrt und duerfen nie
          veralten; Momentaufnahmen tragen ein Datum und werden stehen gelassen. In lebenden
          Dokumenten stehen KEINE Versionsnummern — eine Kopie des Standes veraltet still.
          Ausnahmen werden mit <!-- historisch --> markiert. Keine absoluten Benutzerpfade.
          Guardrail S9 prueft Aktualitaet und Pfade, S10 die Vollstaendigkeit des Katalogs
          in BEFUNDE.md."
  gilt_fuer: [doc-writer, release-engineer, spec-writer]
  seit: 2026-09-29

DIR-005:
  regel: "Das fertige Fabrikat wird immer auf die Synology nach /volume1/web/wordclock/test8
          ausgerollt: die App-Assets, die gebauten .hex, die ESP-.bin und die Versionsdateien.
          Zusaetzlich wird die H2-Kopfzeile der releasenote.html auf die aktuelle STM-Version
          nachgezogen. Werkzeug: ./tools/deploy.sh. Nichts loeschen, der Nutzer pflegt dort
          eigene Dateien."
  gilt_fuer: [release-engineer]
  seit: 2026-09-29

DIR-006:
  regel: "Nach jedem Release wird CHANGELOG.md nachgefuehrt, bei neuen Werkzeugen oder
          Ablaeufen auch die README-Dateien. Ein Release gilt erst als fertig, wenn der
          Changelog-Eintrag steht. REVIEW-Dateien und gap-analysis.md sind Momentaufnahmen
          und werden nicht fortgeschrieben."
  gilt_fuer: [doc-writer, release-engineer]
  seit: 2026-09-29

