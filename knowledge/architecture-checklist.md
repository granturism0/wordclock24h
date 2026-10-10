# Architektur-Checkliste

Vier Kriterien, für dieses Projekt übersetzt. Der zuständige Review-Agent prüft
`design.md` **und** die fertige Umsetzung ausdrücklich gegen diese Fragen, bevor ein
Task als abgeschlossen gilt.

Die generischen Formulierungen aus dem Ursprungskonzept (Odoo-Migrationen,
Multi-Tenant, Datenvolumen) sind hier gegenstandslos und ersetzt.

## 1. Proper architecture

- Bleibt die PWA **parallel** zur Legacy-Oberfläche? Legacy bleibt funktionsfähig und ist die Stabilitäts-Referenz beim Debuggen
- Werden die Wetter-Endpunkte `/api/weather_get_now` und `/api/weather_get_forecast` verwendet? Kein Rückbau auf den entfernten Legacy-Bypass
- Bleibt die Restore-Bedingung um `pending_ticker_restore` vollständig? Keine der vier Teilbedingungen ist redundant
- Werden App-Assets ausschliesslich als `.gz` ausgeliefert, ohne Plain-Fallback?
- Greift die Änderung an der richtigen Schicht an? Ein Stabilitätsproblem, dessen Ursache auf dem ESP liegt, wird nicht in `app.js` umgangen

## 2. Scalable systems

Nicht Datenvolumen oder Nutzerzahl. Die harten Grenzen dieses Systems sind:

- **Wie viele STM-Kommandos** erzeugt die Aktion? Einzeln oder als Burst?
- **Wie viele Byte pro Minute** landen zusätzlich auf der UART? Der STM-Empfangsring für die ESP-Brücke fasst 1024 Byte (`UART_RXBUFLEN` in `src/esp8266/esp8266-uart.c`, BEFUNDE A21) — bei 115200 Baud rund 89 ms Hauptloop-Blockade. Was darüber hinausgeht, wird verworfen; gezählt wird es (`uart_rxdrops`, `d=` in der Diagnosezeile), quittiert nicht
- **Bleibt jeder ausgelöste Pfad unter 20 s?** Watchdog-Timeout. Regulär bedient wird der Watchdog nur am Kopf des Hauptloops; den Bestand aller Aufrufstellen nennt Guardrail S7
- **Wie oft pollt die PWA?** Jeder HTTP-Request kostet den STM Debugtext, auch ohne Kommando
- **Wie lange blockiert der Hauptloop?** `eeprom_write()` schreibt seitenweise (`src/eeprom/eeprom.c`, BEFUNDE C3): `EEPROM_PAGE_SIZE` ist 32 Byte auf dem F411 und 8 Byte auf dem F103. Je Seite ein Vergleichslesen, höchstens ein Schreiben vom ersten bis zum letzten geänderten Byte, dann ein Schreibzyklus; unveränderte Seiten werden übersprungen. **Faustregel: Blockade ≈ Anzahl geänderter Seiten × rund 24 ms, plus I²C-Zeit je Byte.** Ein Zyklus kostet rund 24 ms, nicht 16 ms — 16 ms sind nur der Wartezyklus, dazu kommen rund 8 ms I²C-Zeit (am Gerät gemessen, L358). Die I²C-Zeit je Byte ist hoch, weil `i2c_wait_for_flags()` je Flag bis 1 ms schläft (L359). Rechnerisch: Ein 32-Byte-Wert sollte rund 110 ms statt der gemessenen 767 ms kosten; der Overlay-Bereich mit 1280 Byte im schlimmsten Fall rund 3 s auf dem F411 (40 Seiten × rund 72 ms mit I²C-Zeit) und rund 6 s auf dem F103 (160 Seiten × rund 36 ms), statt vorher rund 30 s über dem 20-s-Watchdog. **Diese Werte sind gerechnet, nicht gemessen** — die Bestätigung am Gerät steht mit G3 aus
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
- **Nimmt eine ESP-Leseschleife über `WiFiClient` ein Verbindungsende erst an, nachdem sie `available()` nach `connected() == 0` noch einmal abgefragt hat?** Die Zusage „`connected()` liefert true, solange noch Daten anstehen“ ist falsch. Im Core 3.1.2 liest `WiFiClient::available()` den Pufferstand, **bevor** es mit `optimistic_yield(100)` die Kontrolle abgibt (`WiFiClient.cpp:246-257`); in dieser Abgabe stellt der Treiber die restlichen Segmente samt FIN zu, zurück kommt die alte 0. `connected()` meldet in CLOSE_WAIT 0, auch wenn noch Daten im Puffer liegen (`WiFiClient.cpp:327-333`, `ClientContext.h:363-371`) — das `|| available()` wird dann nie erreicht. Dreimal dieselbe Annahme: L173 (`httpclient.cpp`, Release Notes nach genau einem TCP-Segment von 536 Byte abgeschnitten), L353 (Wetterabruf nach dem ersten Segment abgeschnitten, „Parse Error“), L356/C53 (erste Anfragezeile in `http.cpp`, offen)
- **Bildet eine Attrappe von `WiFiClient` im Prüfstand diese Semantik nach?** Zustellung nur an Abgabepunkten, `connected()` nach FIN 0. Eine Attrappe, die `connected()` als `!geschlossen || available()` liefert, setzt genau die widerlegte Zusage voraus und sieht den Fehler nicht — so geschehen bei t12 (`tools/checks/auszug/esp/t12`, BEFUNDE L353)
