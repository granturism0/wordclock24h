# Quick Reference — Ich sehe / Ich tue / Schweregrad

Nachschlagewerk für jeden Review- und Guardrail-Schritt. Jeder Eintrag stammt aus
einem **belegten** Befund in `REVIEW.md` oder aus einer Architektur-Invariante in
`CLAUDE.md` — keine allgemeinen Ratschläge.

**Kritisch** blockiert den Task. **Hoch** blockiert, wenn nicht ausdrücklich begründet
abgewichen wird. **Mittel** wird gemeldet und darf bewusst offenbleiben.

## STM32-Firmware (`src/**`)

| Ich sehe… | Ich tue… | Schweregrad |
|---|---|---|
| Busy-Wait, `delay_sec`, `delay_msec` oder Warteschleife im Web-Kommandopfad ohne `watchdog_reload()` | Pfad in den Hauptloop verlagern oder Reload ergänzen. `watchdog_reload()` hat **genau eine** Aufrufstelle: `main.c:3170`. Alles über 20 s ist ein garantierter IWDG-Reset | Kritisch |
| `log_printf` statt `debug_log_printf` in einem Pfad, der pro Display-Refresh läuft | Auf `debug_log_printf` umstellen. Jede Logzeile blockiert über `esp8266_uart_flush()` und verzögert gleichzeitig das Lesen vom ESP | Kritisch |
| `display_clock_flag = …` als Zuweisung, wo ein anstehendes Update verloren gehen kann | Prüfen, ob verodert werden muss. Die Flags sind disjunkte Bits (`0x01`, `0x02`, `0x04`) | Kritisch |
| Ein Flag wird unbedingt gelöscht, seine Wirkung aber nur bedingt gesetzt | Erst löschen, wenn die Wirkung tatsächlich eingetreten ist. Siehe `pending_weather_ticker_restore`, `main.c:3699-3711` | Kritisch |
| `while (…) { }` auf ein ISR-gesetztes Flag ohne Timeout und ohne Reload | Timeout und Diagnoseausgabe ergänzen, oder bewusst dokumentieren | Hoch |
| 1-Wire- oder bitgenaues Timing ohne `__disable_irq()` | Interrupt-Schutz erwägen; ohne ihn sind Lesefehler erwartbar | Hoch |
| Sensorwert wird ohne CRC-Prüfung als gültig übernommen | CRC prüfen, „gefunden“ und „Messwert gültig“ trennen | Hoch |
| `__disable_irq(); … __enable_irq();` ohne PRIMASK-Sicherung | PRIMASK sichern und wiederherstellen | Mittel |

**Falle beim Durchsuchen von `src/**`:** Acht Dateien sind **ISO-8859-1**, nicht UTF-8 —
`base.c`, `base.h`, `display.c`, `ds18xx.c`, `irmp.c`, `rtc.c`, `tempsensor.c`, `w25qxx.c`.
`grep` stuft sie als **binär** ein und gibt **gar nichts** aus — nicht „0 Treffer", sondern
eine leere Ausgabe. Das sieht aus wie „nicht vorhanden", ist aber „nicht gelesen".

**Das Heilmittel ist `grep -a`, nicht das Locale.** `LC_ALL=C` allein behebt es
nachweislich **nicht**: `LC_ALL=C grep -c 'display_icon' display.c` liefert leer,
`grep -ac` liefert 29. Das ripgrep-basierte Grep-Tool ist nicht betroffen.
Betrifft direkt `display.c` (Display-Zustandsmaschine) und `ds18xx.c`/`tempsensor.c`
(offenes DS18xx-Thema).

## ESP8266 (`ESP8266/ESP-uclock/*.cpp`, `*.ino`)

| Ich sehe… | Ich tue… | Schweregrad |
|---|---|---|
| Neue unbedingte Debugausgabe pro HTTP-Request auf die STM-UART | Unterdrücken oder bedingt machen. Der RX-Ring des STM ist 256 Byte und verwirft bei Überlauf **still** (`uart-driver.h:698`, kein `else`-Zweig) | Kritisch |
| Neuer Endpunkt, der pro Aufruf mehrere STM-Kommandos erzeugt | Kommandozahl und EEPROM-Kosten pro Aufruf abschätzen und dokumentieren | Hoch |
| Warteschleife auf eine STM-Quittung ohne Abbruchbedingung | Timeout ergänzen | Hoch |
| Neuer Endpunkt ohne Eintrag in der Endpunktliste der PWA | Beide Seiten gemeinsam ändern, sonst schweigender 404 | Mittel |

## PWA (`data/app/app.js`, `sw.js`)

| Ich sehe… | Ich tue… | Schweregrad |
|---|---|---|
| Existenz- oder Vollständigkeitsprüfung auf `.gz` ohne `size > 0` | Grössenprüfung ergänzen. Eine 0-Byte-Datei führt zu White-Screen, und der Service Worker hält sie dauerhaft | Kritisch |
| Antwort wird gecacht, ohne die Länge zu prüfen | Länge prüfen. `response.ok` ist bei 0 Byte `true` | Kritisch |
| Aufruf einer Funktion, die nirgends definiert ist | Beheben. Guardrail-Stufe 2 findet das automatisch | Kritisch |
| `innerHTML` mit Daten vom Gerät oder vom Update-Server | `escapeHtml` oder `textContent`. Der Update-Host ist konfigurierbar und damit nicht vertrauenswürdig | Hoch |
| `fetch()` ohne `response.ok`-Prüfung oder ohne Timeout | `fetchWithTimeout` verwenden und Status prüfen | Hoch |
| `translate("…")` auf einen Key, der in keiner Tabelle steht | Key in **beiden** Tabellen ergänzen. Sonst steht der Key wörtlich auf dem Button | Hoch |
| Neue Schleife mit mehreren `await apiFetch` in Folge | Kosten pro Kommando am STM prüfen. 16 Kommandos am Stück sind rund 4,3 s Hauptloop-Stillstand | Hoch |
| Erfolgsmeldung, die nicht vom tatsächlichen Ergebnis abhängt | Ergebnis auswerten. Fehlerschluckende Wrapper machen jede aufrufende Schleife blind | Hoch |
| Leerer `catch (_) {}` | Mindestens zählen und melden | Hoch |
| Deutscher String direkt im Code statt über `translate()` | In die i18n-Tabelle. **Du-Form**, echte Umlaute, Schweizer „ss“ | Mittel |
| `setInterval`/`addEventListener` ohne Gegenstück | Aufräumen | Mittel |
| `CACHE_NAME` unverändert, obwohl `app.js` geändert wurde | `CACHE_NAME` anheben, sonst neue `index.html` mit alter `app.js` | Hoch |

## UI (`index.html`, `styles.css`, Manifest, Icons)

| Ich sehe… | Ich tue… | Schweregrad |
|---|---|---|
| Statusmeldung in ein Element, das nicht in jedem Bereich sichtbar ist | Globale Meldungsfläche verwenden | Hoch |
| Dynamischer Statusbereich ohne `aria-live` | Ergänzen | Hoch |
| Modal ohne `role="dialog"`, Fokus-Management und Escape | Ergänzen | Hoch |
| Rand- oder Umrissfarbe eines Bedienelements unter 3:1 | Anheben. WCAG 2.1 SC 1.4.11 | Hoch |
| `outline: none` ohne Ersatz | Eigenen Fokusstil setzen | Hoch |
| Neues `<select>`, `time`, `color` oder `range` ohne `color-scheme` | `color-scheme: dark` setzen, sonst heller Picker auf iOS | Mittel |
| CSS-Klasse angelegt, aber im HTML nie gesetzt | Im HTML nachziehen oder CSS entfernen. Guardrail-Stufe 6 meldet das | Mittel |
| Bedienelement kleiner als 44×44 px | Vergrössern | Mittel |
| „Sie“-Form irgendwo in der deutschen UI | Auf Du-Form ändern. Siehe `CLAUDE.md` | Mittel |

## Build und Release

| Ich sehe… | Ich tue… | Schweregrad |
|---|---|---|
| `make app-gz` bei nicht sauberem Arbeitsbaum | Abbrechen. Erzeugt `.gz` einer halb geschriebenen Datei — R2 in `CLAUDE.md` | Kritisch |
| Zwei Release-Builds in derselben Minute | Abbrechen. Der zweite überschreibt den ersten kommentarlos | Kritisch |
| Geändertes Format der Versionszeilen | Zurücknehmen. Der Makefile liest sie per `grep`; das Release bricht sonst still | Kritisch |
| `cat > datei` oder `Write` auf eine Datei, die ich nicht vorher gelesen habe | Erst lesen. Bei Konfigurationsdateien **anhängen statt ersetzen**. Ein `[ -f x ] && grep … \|\| echo "fehlt"` meldet auch dann „fehlt", wenn die Datei existiert und `grep` nur nichts findet | Kritisch |
| Release ohne vorherigen vollständigen Build | Vollständig bauen, nicht nur `app-gz` | Hoch |
