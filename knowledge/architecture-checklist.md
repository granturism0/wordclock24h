# Architektur-Checkliste

Vier Kriterien, für dieses Projekt übersetzt. Der zuständige Review-Agent prüft
`design.md` **und** die fertige Umsetzung ausdrücklich gegen diese Fragen, bevor ein
Task als abgeschlossen gilt.

Die generischen Formulierungen aus dem Ursprungskonzept (Odoo-Migrationen,
Multi-Tenant, Datenvolumen) sind hier gegenstandslos und ersetzt.

## 1. Proper architecture

- Bleibt die PWA **parallel** zur Legacy-Oberfläche? Legacy bleibt funktionsfähig und ist die Stabilitäts-Referenz beim Debuggen
- Werden die Wetter-Endpunkte `/api/weather_get_now` und `/api/weather_get_forecast` verwendet? Kein Rückbau auf den entfernten Legacy-Bypass
- Bleibt die Restore-Bedingung um `pending_weather_ticker_restore` vollständig? Keine der vier Teilbedingungen ist redundant
- Werden App-Assets ausschliesslich als `.gz` ausgeliefert, ohne Plain-Fallback?
- Greift die Änderung an der richtigen Schicht an? Ein Stabilitätsproblem, dessen Ursache auf dem ESP liegt, wird nicht in `app.js` umgangen

## 2. Scalable systems

Nicht Datenvolumen oder Nutzerzahl. Die harten Grenzen dieses Systems sind:

- **Wie viele STM-Kommandos** erzeugt die Aktion? Einzeln oder als Burst?
- **Wie viele Byte pro Minute** landen zusätzlich auf der UART? Der RX-Ring ist 256 Byte und verwirft still
- **Bleibt jeder ausgelöste Pfad unter 20 s?** Watchdog-Timeout, und `watchdog_reload()` hat genau eine Aufrufstelle
- **Wie oft pollt die PWA?** Jeder HTTP-Request kostet den STM Debugtext, auch ohne Kommando
- **Wie lange blockiert der Hauptloop?** EEPROM-Schreibzugriffe kosten rund 16 ms pro Byte
- Gibt es hartkodierte Grenzen, die beim Wachsen der Layouttabellen oder Overlays brechen?

## 3. Secure by design

- Jedes `innerHTML` mit Fremddaten über `escapeHtml` oder `textContent`
- Inhalte vom Update-Server gelten als **nicht vertrauenswürdig** — der Host ist konfigurierbar, die Verbindung ist HTTP
- Keine WLAN-Credentials oder Gerätegeheimnisse in Logausgaben, auch nicht im STM32-Logbuch der PWA
- Neue Endpunkte, die Konfiguration schreiben, brauchen eine bewusste Entscheidung über Bestätigung und Reichweite

## 4. Stable & reliable

- Wird jeder Fehler ausgewertet, oder verschluckt ein Wrapper ihn? Eine Erfolgsmeldung muss vom tatsächlichen Ergebnis abhängen
- Gibt es leere `catch`-Blöcke?
- Gibt es stille Verwerfungen ohne Zähler — etwa ein voller Puffer ohne `else`-Zweig?
- Werden Werte still zurechtgebogen und danach als „gespeichert“ gemeldet?
- Ist der Zustand nach einem Abbruch mitten in einer Sequenz definiert? Besonders bei Overlay-Import, wo ein Fehlschlag alle folgenden Indizes verschiebt
- Wird ein gesetztes Flag garantiert wieder aufgelöst, auf **jedem** Pfad?
