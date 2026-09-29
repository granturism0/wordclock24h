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
  regel: "Bei jedem Build wird die Versionsnummer der geaenderten Komponente erhoeht, damit
          jedes Fabrikat eindeutig einem Commit zuzuordnen ist. PWA-Aenderung hebt APP_VERSION
          und CACHE_NAME, STM-Aenderung hebt VERSION, ESP-Aenderung hebt ESP_VERSION. Kein Build
          ohne mindestens eine Erhoehung."
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

