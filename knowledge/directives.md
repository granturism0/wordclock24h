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
  regel: "Bei jedem Build werden ALLE drei Komponenten im Gleichschritt versioniert: VERSION
          (STM), ESP_VERSION (ESP) sowie APP_VERSION und CACHE_NAME (PWA) — auch wenn sich die
          jeweilige Komponente nicht geaendert hat. So ist jedes Fabrikat als Einheit
          identifizierbar und eindeutig einem Commit zuzuordnen."
  gilt_fuer: [release-engineer, pwa-developer, ui-developer, stm-developer, esp-developer]
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

