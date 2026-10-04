# Direktiven

Verbindliche Regeln, die aus Nutzeräusserungen abgeleitet und **ausdrücklich bestätigt**
wurden. Gepflegt vom `librarian`-Agenten. Nichts steht hier ohne Bestätigung.

**Dieser Katalog führt den Bestand der Kennungen.** Kommt eine Direktive dazu, steht sie
hier — sonst wiederholt sich L225: Der Katalog endete bei DIR-009, fünf geltende Regeln
standen nur in `CLAUDE.md`, und wer hier nachsah, schloss daraus, es gebe keine Regel.
Hier steht die **Regel**; Anlass, Datum und das Schadensbild dahinter stehen in `CLAUDE.md`
und in `BEFUNDE.md` und werden hier nicht verdoppelt.

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

DIR-009:
  regel: "Nach jedem Flashen laeuft ./tools/smoke-device.sh gegen das Geraet. Die
          Guardrail-Stufen sind ausnahmslos statisch und sagen nichts darueber, ob die
          Uhr nach einer Aenderung noch funktioniert -- besonders nach Eingriffen in
          http.cpp, dessen Parameter-Auswertung jeder Request durchlaeuft. Der Test ist
          ausschliesslich lesend; einzige Ausnahme ist maintenance_reset_stm32 mit
          Sec-Fetch-Dest: image, wo 403 erwartet wird."
  gilt_fuer: [release-engineer, alle]
  seit: 2026-10-02

DIR-008:
  regel: "Die produktive Uhr (Adresse aus tools/device.conf, DEVICE_HOST) darf LESEND abgefragt werden
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

DIR-010:
  regel: "Der STM32 wird ausschliesslich über ./tools/flash-stm.sh geflasht, niemals über
          einen Aufruf von /api/remote_stm32_flash von Hand. Drei Bedingungen sieht man dem
          Endpunkt nicht an, und jede lässt ihn stillschweigend nichts tun: filename ist
          Pflicht (fehlt er, setzt der ESP error_code = 2 und der Aufruf sieht erfolgreich
          aus; den gültigen Namen meldet das Gerät als stm32_default in /api/update_status),
          bei HARDWARE_CONFIGURATION = 65535 weist der ESP jeden Dateinamen ab (erst den STM
          zurücksetzen, dann flashen), und nach dem Flashen muss der STM zurückgesetzt werden
          — der ESP meldet nur, dass er wartet, und löst den Reset nicht aus. Das Skript
          erzwingt alle drei Punkte und weist die gemeldete Version am Ende nach."
  gilt_fuer: [alle]
  seit: 2026-10-03

DIR-011:
  regel: "Jedes ausgerollte Release wird sofort committet, getaggt und gepusht — einzeln je
          Rollout, nicht gesammelt und nicht auf Nachfrage. Das gehört zum Rollout wie der
          Smoketest. Das Tag heisst release/<stm>-<esp>-<app> und trägt die Versionen DIESES
          Commits; nachholen lässt es sich nicht, weil die Versionsdateien nur den Endstand
          tragen. deploy.sh nennt den fehlenden Tag-Befehl bei jedem Lauf, der Stop-Hook
          meldet Ungepushtes — auch bei sauberem Arbeitsbaum, denn nach dem Commit ist
          nichts mehr geändert."
  gilt_fuer: [release-engineer, alle]
  seit: 2026-10-03
  geaendert: 2026-10-04 — Push zum Remote ausdrücklich aufgenommen. Er stand vorher nirgends
             als Ablaufregel, sondern nur als Aufgabe im Befundkatalog, und eine Aufgabe
             erinnert niemanden: 23 Commits und 11 Tags lagen lokal.

DIR-012:
  regel: "Der Smoketest ist nicht der Test. ./tools/smoke-device.sh prüft, ob das Gerät LEBT
          — nicht, ob es noch tut, was es soll; ein Endpunkt, der {\"ok\":true} meldet und
          nichts tut, besteht ihn. Vor jedem Release, das mehr als eine Komponente berührt,
          läuft zusätzlich der pwa-tester über TESTPLAN-PWA.md. Eine Frage nach dem Verhalten
          wird am Gerät beantwortet, nicht aus dem Quelltext. Wird der Durchlauf bewusst
          ausgelassen, gehört das in den Bericht."
  gilt_fuer: [release-engineer, pwa-tester, alle]
  seit: 2026-10-03

DIR-013:
  regel: "Während jeder Messung am Gerät läuft ./tools/watch-log.sh parallel mit und wird
          LAUFEND mitgelesen, nicht erst am Schluss durchgesehen. Gemeldet werden Exceptions,
          Neustarts, Watchdog-Resets, Sprünge in den verworfenen Zeichen (d=) und ein
          Stillstehen der diag-Folge. Das gilt für jede Messung, nicht nur für den
          vollständigen Testdurchlauf. Das Problem ist nicht der übersehene Absturz, sondern
          der Bericht, der sauber meldet, während das Gerät zwischendurch neu gestartet ist —
          er erzeugt Vertrauen, das nicht gedeckt ist."
  gilt_fuer: [pwa-tester, release-engineer, alle]
  seit: 2026-10-03

DIR-014:
  regel: "Eine neu gebaute Prüfung ist erst fertig, wenn sie einmal fehlgeschlagen ist. Nicht
          'der Code sieht richtig aus', sondern: eine Verletzung herstellen und sehen, dass
          die Prüfung anschlägt UND dass ihre Meldung beim Empfänger ankommt — Kanal,
          Pufferung und Exitcode gehören zum Nachweis, nicht nur der Mechanismus. Dazu
          nachzählen, wie viele Fälle die Prüfung überhaupt betrachtet, und die Zahl mit dem
          vergleichen, was es geben müsste: Eine Prüfung mit zu engem Muster ist schlimmer
          als keine, weil sie genau das Vertrauen erzeugt, das sie nicht deckt."
  gilt_fuer: [alle]
  seit: 2026-10-04
