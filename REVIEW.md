# Code-Review PWA + Display-/Hardware-Analyse

**Stand:** 2026-08-12 · Branch `pwa-decoupling` · Commit `1d7d54e`
**Versionen:** STM 3.2.4 · ESP 3.2.1 · App 1.4.69 · SW-Cache `wordclock-app-v61`
**Umfang:** kein Code geändert, kein Build ausgeführt, nichts geflasht.

## Legende Verifikationsstatus

| Marke | Bedeutung |
|---|---|
| **✔ verifiziert** | Lead hat den Code selbst gelesen und die Behauptung bestätigt |
| **● gemeldet** | Befund des Review-Agents, plausibel begründet, vom Lead nicht einzeln nachgeprüft |
| `[BELEGT]` | Agent-Label: direkt aus dem Code ableitbar |
| `[PLAUSIBEL]` | Agent-Label: Indizienkette, Code stützt sie, Messwert fehlt |
| `[SPEKULATION]` | Agent-Label: Vermutung |

---

# Kernbefunde

Fünf Punkte tragen den Bericht. Alle fünf sind vom Lead am Code verifiziert.

### 1. `display_test()` resettet den STM zwangsläufig — auf jeder Hardware ✔

`watchdog_reload()` hat im gesamten `src/`-Baum **genau eine Aufrufstelle**: `src/main.c:3170`.
`display_test()` blockiert davon entkoppelt:

- SK6812 **RGBW**: `for (i = 1; i < 16; i++)` mit `delay_sec(3)` → **15 × 3 s = 45 s** (`src/display/display.c:6082-6096`)
- SK6812 RGB: `for (i = 1; i < 8; i++)` → **7 × 3 s = 21 s**

Gegen `WATCHDOG_TIMEOUT_MS 20000UL` (`src/main.c:382`). Beide Varianten liegen darüber.
`delay_sec()` → `delay_msec()` ist reiner Busy-Wait auf SysTick ohne Watchdog-Reload (`src/delay/delay.c:61-80`).

**Konsequenz:** Der Display-Test ist kein Diagnosewerkzeug, sondern ein garantierter IWDG-Reset. Betrifft PWA (`/api/test_display`) **und** Legacy gleichermassen. Die PWA weiss von den 45 s und pausiert korrekt ihre Poller (`app.js:6353` `startBackgroundPause(45000, true)`) — das verhindert den STM-Reset aber nicht.

Das erklärt einen Teil der Beobachtung „Display-Test aus der neuen App liess das Display stehen".

### 2. Restore-Degradierung ⇒ dauerhaft dunkles Display ✔

`src/main.c:3699-3711`:

```c
if (pending_weather_ticker_restore && ! display_ticker_active () && ! display.do_display_icon &&
    show_icon_stop_time == 0 && show_overlay_idx >= MAX_OVERLAYS)
{
    pending_weather_ticker_restore = 0;        // Flag wird UNBEDINGT gelöscht
    if (! display_clock_flag)                  // UPDATE_ALL nur wenn NICHTS anliegt
    {
        display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
    }
}
```

Die Flags sind **disjunkte Bitwerte** (`src/display/display.h:34-39`): `UPDATE_MINUTES 0x01`, `UPDATE_NO_ANIMATION 0x02`, `UPDATE_ALL 0x04`.

Steht bei Eintritt `0x01` oder `0x02`, ist das Restore **verbraucht ohne Wirkung** — ohne Wiederholung, ohne Log. `display_clock()` betritt den `UPDATE_ALL`-Zweig nie, `display_animation_flush(FALSE)` schreibt nach `TARGET_STATE`, und nach einem Ticker existiert kein `TARGET_STATE`-Bit mehr ⇒ **alle Display-LEDs aus**.

**Dauer:** bis zum nächsten `UPDATE_ALL`. Bei `WCLOCK24H == 1` jede Minute ⇒ bis 60 s. Bei `WCLOCK24H == 0` nur jede 5. Minute ⇒ **bis 5 Minuten**.

**Wahrscheinlichster Auslöser aus der PWA:** der Dimmkurven-Burst (Kernbefund 3) hält `display_clock_flag` rund 4 s auf `0x02`.
**Auslöser ohne PWA:** die LDR-Automatik setzt `0x02` bei jeder Helligkeitsänderung (`main.c:3238`), `ldr_conversion_flag` kommt 4×/s — das erklärt, warum das Symptom auch ohne PWA auftrat.

Die vier Teilbedingungen selbst sind **korrekt und keine ist redundant**. Die Lücke liegt ausschliesslich im unbedingten Löschen des Flags.

### 3. Dimmkurve speichern: 16 Kommandos am Stück ≈ 4,3 s Hauptloop-Stillstand ✔

PWA: `persistDimCurve()` (`app.js:9964-9984`) sendet in einer `for`-Schleife **16× `await apiFetch(...)`** ohne Pause dazwischen.
Legacy-Gegenstück: 16 Zeilen mit **je eigenem Submit-Button** — ein Kommando pro Seitenaufruf.

STM-Kosten pro einzelnem Kommando ● gemeldet: `eeprom_write()` schreibt **Byte für Byte** mit `EEPROM_WAITSTATES 15` Busy-Wait ⇒ ~16 ms/Byte × 16 Bytes ≈ **256 ms**, und es wird jedes Mal die **ganze** Kurve geschrieben. Plus 2 LED-Refreshes und `display_clock_flag = 0x02`.

⇒ 16 × ~270 ms ≈ **4,3 s**, in denen `display_clock_flag` praktisch durchgehend auf `0x02` steht. Das ist die Vorbedingung für Kernbefund 2.

**Reproduktionsrezept:** Wetter-Ticker starten und währenddessen in der PWA die Dimmkurve speichern.

### 4. Warum die PWA schlechter ist als Legacy — nicht die Kommandos, die UART-Last ● gemeldet `[BELEGT]`

Pro *Benutzeraktion* erzeugt die PWA in den Normalpfaden **nicht mehr** STM-Kommandos als Legacy. Farbe und Helligkeit senden je genau eines. Die naheliegende Hypothese „Live-Slider flutet den STM" ist **widerlegt**: `app.js:2097-2102` bindet `input` nur an eine lokale Vorschau, gesendet wird ausschliesslich per Button.

Der Unterschied liegt woanders:

- Der ESP schreibt **pro HTTP-Request zwei Debugzeilen auf die STM-UART** (`http.cpp:11196` und `11251`/`11305`), zusammen ~100 Byte — **unabhängig davon, ob der Endpunkt überhaupt ein STM-Kommando erzeugt**.
- **Legacy im Leerlauf: 0 Requests.** Kein `meta refresh`, kein XHR.
- **PWA im Leerlauf: ~32 Requests/min**, im Modul „System" **~56/min** (15-s-Auto-Refresh mit 3 parallelen + 2 seriellen Requests, 5-s-Farb-Polling, 2,5-s-Logbuch-Polling) ⇒ **~64 bis ~112 Debugzeilen/min**.
- Der STM-RX-Ring ist **256 Byte** und verwirft bei Überlauf **still** — `uart-driver.h:698` hat `if (uart_rxsize < UART_RXBUFLEN)` **ohne `else`-Zweig**. Kein Overrun-Flag, keine Prüfsumme, kein Retry.
- Ein 15-s-Tick feuert 3 Requests im gleichen Millisekundenfenster ⇒ ~300 Byte ⇒ **mehr als der Ring**. Und die PWA synchronisiert alle Clients aufs gleiche Raster (`app.js:2257`), zwei Tabs feuern gleichzeitig.

Dazu `reload: true` als Default nach jedem Write ⇒ pro Aktion: Legacy 1 Request, PWA ~6.

**Das ist der stärkste Kandidat für den Stabilitätsunterschied** — und der Hebel liegt auf der **ESP-Seite**, nicht in `app.js` und nicht im STM.

### 5. Statusmeldungen sind auf 10 von 11 Bereichen unsichtbar ✔

`announceStatus()` (`app.js:2301`) schreibt **jede** der rund 70 Meldungen nach `#updated-at`.
Dieses Element steht in `index.html:87`, also **innerhalb der Hauptseiten-Section**.
`styles.css:271` sagt `.module-section { display: none }`, sichtbar ist nur `.is-active`.

⇒ Sobald du in Netzwerk, Wartung, System, Umwelt oder Backup arbeitest, landen alle Meldungen in einem `display:none`-Element. Übrig bleibt nur der 1400-ms-Textblitz am Button.

Fix: ein Meldungselement **ausserhalb** der Sections plus eine Zeile in `announceStatus`. Grösster Einzelhebel im UI-Teil.

---

# Teil 1 — Display und Hardware (STM32)

## Blockierende Pfade — vollständige Liste ● gemeldet `[BELEGT]`

`watchdog_reload()` nur an `main.c:3170` ✔. Jeder Pfad über 20 s ist ein garantierter IWDG-Reset.

| Pfad | Datei:Zeile | Dauer | Reload |
|---|---|---|---|
| `display_test()` | `display.c:6082-6113` | **21 s / 45 s** ✔ | nein |
| `display_set_ticker(do_wait=1)` | `display.c:4802-4809` | **0,4 s – 144 s** | nein |
| `var_send_buf()` Warten auf Quittung | `vars.c:60-65` | **unbegrenzt** | nein |
| `sk6812_refresh()` DMA-Wait | `sk6812.c:453-474` | **unbegrenzt** ✔ | nein |
| `sk6812_clear_all()` DMA-Wait | `sk6812.c:306-309` | **unbegrenzt** | nein |
| `remote_ir_learn()` | `main.c:1517` | ungeprüft | nein |
| `ds18xx_start_conversion(1)` | `ds18xx.c:141-148` | 95–870 ms | nein |
| `eeprom_waitstates()` | `eeprom.c:38-57` | ~16 ms/Byte | nein |
| `power_on()` + `delay_msec(200)` | `display.c:2937` | 200 ms | nein |
| `esp8266_uart_flush()` / `log_flush()` | `uart-driver.h` ~690 | ~87 µs/Zeichen | nein |
| `onewire_*` Bit-Timing | `onewire.c:76-185` | 65 µs/Bit | nein |
| `display_clock()` im Temperaturmodus | `main.c:3722-3731` | ~6 ms 1-Wire | nein |

**Ticker mit `do_wait = 1`** verdient eigene Aufmerksamkeit: `MAX_TICKER_LEN 64`, `TICKER_COLS + 1 = 9` Schritte/Zeichen, `deceleration*1000/64` ms/Schritt. Bei Default 3 ergibt das ~27 s für 64 Zeichen, bei Deceleration 16 rund **144 s**. Ein langer Ticker-Text erzwingt den Reset.

Derselbe Pfad gilt für den **IP-Adress-Ticker** (`main.c:2705-2712`): bei 20 Zeichen und Deceleration 3 sind es 8,4 s (unkritisch), bei Deceleration ≥ 8 aber ≥ 22 s ⇒ Reset **genau während der IP-Anzeige**. Direkt davor steht `var_send_all_variables()`, das ~60 Variablen einzeln mit blockierendem Flush sendet. `[PLAUSIBEL]` — das erklärt „Hänger exakt bei Anzeige von IP" quantitativ und passt genau auf deine Beobachtung.

## Unbedingtes Logging im heissen Pfad ✔

`src/sk6812/sk6812.c:476` und `:495` sind **`log_printf`, nicht `debug_log_printf`** — also unbedingt, bei jedem Refresh. Zusammen ~103 Zeichen.

Über `log_vprintf` mit blockierendem `esp8266_uart_flush()` ⇒ ● gemeldet ~8,9 ms Blockade **pro Refresh**. Aufrufwege: Minuten-LEDs mit 64 Hz, Ticker mit 21 Hz bei Default, und `display_animation_flush(TRUE)` macht **zwei** Refreshes bei jedem Farb-/Helligkeitskommando.

Rechnung: 21 × 8,9 ms ≈ **187 ms/s reine Logblockade während eines Tickers**.

Verschärfend: `esp8266_get_message()` beginnt mit `log_flush()` und `esp8266_uart_flush()` (`esp8266.c:202-203`) — **der STM liest kein Zeichen vom ESP, solange Logausgabe aussteht**. Damit ist die Logmenge direkt an die Empfangslatenz und an das stille Verwerfen gekoppelt. Das ist der Kopplungspunkt zwischen „zu viel Logging" und „verlorene Kommandos".

**Bitter:** Das Logbuch-Feature, das die Diagnose liefern soll, ist selbst eine Hauptquelle der Blockade. `log.c:27-34` schickt jede Zeile zusätzlich per `esp8266_send_log_line()`, und das endet mit blockierendem Flush. Der Endpunkt `/api/stm32_log` ist **PWA-exklusiv** und wird alle 2,5 s gepollt.

## DMA — was der Code zeigt

**Kein Fehler in der DMA-Konfiguration gefunden** ● gemeldet `[BELEGT]`: Der nächste Transfer startet erst nach der Warteschleife, Pufferwechsel und Kopie erfolgen in getrennte Puffer, die ISR liest den anderen. **Keine Race auf den Pixeldaten.** Circular-Mode über Doppelpuffer, ISR-Priorität 0. Sauber konfiguriert.

**Aber:** `while (sk6812_dma_status != 0)` (`sk6812.c:453`) hat **kein Timeout, keinen Abbruch, keinen Watchdog-Reload** ✔. Zurückgesetzt wird `sk6812_dma_status` ausschliesslich in der TC-ISR. Ein verlorener TC-Interrupt ⇒ Hauptloop hängt dauerhaft ⇒ IWDG-Reset nach 20 s.

**Für die Beweisführung entscheidend:** Dieser Fall wäre im Log **sichtbar**. `sk6812.c:463-472` gibt `"sk6812_refresh: waiting %lus dma=... pos=... off=... pause=..."` aus ✔. **Fehlt diese Zeile in den Hänger-Mitschnitten, ist ein DMA-Stillstand widerlegt** — und deine Entscheidung gegen einen pauschalen DMA-Fix ist damit belegt statt nur vermutet.

## Verschachtelte Kommandoausführung ● gemeldet `[BELEGT]`

`var_send_buf()` (`vars.c:47-66`) wartet auf die ESP-Quittung mit
`while ((rtc = schedule_esp8266_messages ()) != ESP8266_OK) { ; }` — die Warteschleife ruft **die Dispatch-Funktion selbst** auf. Jedes in dieser Zeit eintreffende `CMD` wird **vollständig und verschachtelt ausgeführt**.

Drei Konsequenzen:

1. **Verlorenes Display-Update.** `main.c:1623` lautet `display_clock_flag = set_display_power (val, TRUE);`. `set_display_power()` löst intern `var_send_buf` aus. Setzt ein verschachteltes Kommando dort `display_clock_flag`, wird der Wert beim Rücksprung durch die Zuweisung **überschrieben und verworfen**. Gleiches Muster an `main.c:3363, 3367, 3798`. `display_clock_flag` ist ein `static` Global ohne Schutz.
2. **Geschlossene Rekursion** ohne Tiefenbegrenzung: `schedule_esp8266_messages` → `schedule_esp8266_cmd` → `schedule_esp8266_numeric_variable` → `set_display_power` → `var_send_display_power` → `var_send_buf` → zurück zum Anfang. `[BELEGT]` die Möglichkeit, `[SPEKULATION]` ob sie tief genug für einen Stack-Overflow wird. Ein solcher würde als HardFault → `fault_reset()` erscheinen — über F1 prüfbar.
3. **Unbegrenztes Warten ohne Reload.** Keine Abbruchbedingung, kein Timeout. Bleibt die Quittung aus, weil der ESP in HTTP-Bearbeitung steckt, verzögert sich der STM. Über 20 s ⇒ IWDG-Reset. **Hier zahlt die PWA-Requestrate direkt ein.**

## Weitere Kollisionen ● gemeldet

- **Overlay speichern/löschen:** `/api/overlay_set` sendet **8 Kommandos**, `/api/overlay_delete` schiebt in einer Schleife alle nachfolgenden Overlays ⇒ **bis 8×(n−idx) Kommandos**. Die EEPROM-Kosten der `overlay_*`-Setter wurden **nicht geprüft** — gilt dort das Muster von Kernbefund 3, ist das der zweite Multi-Sekunden-Burst. Wichtigste offene Lücke des Reviews.
- **Ticker ignoriert `display_power_is_on`** (`display.c:5087-5098`): Display AUS während ein Ticker läuft ⇒ **das Display geht nicht aus, bis der Ticker durch ist**.
- **`DISPLAY_TEMPERATURE_RPC_VAR` blockiert das Restore** (C4): `show_icon_stop_time == 0xFFFFFFFF` heisst „warte auf Animationsende", aufgelöst nur bei `!animation_start_flag && animation_stop_flag`. Der Animationszweig wird aber nicht erreicht, solange ein Ticker läuft. ⇒ Wetter-Ticker → „Temperatur anzeigen" → Restore blockiert bis Ticker-Ende **plus 5 s**. Kein permanenter Verlust, aber ≥ 5 s „kommt nicht sauber zurück".
- **1-Wire ohne Interrupt-Schutz** (`onewire.c:76-185`): `delay_usec()` ohne `__disable_irq()`, Bit-Slot 65 µs, DMA- und UART-ISRs mit Priorität 0 preempten mitten im Slot. Lesefehler sind erwartbar — und weil `ds18xx_read_raw_temp()` **keine CRC prüft** (`ds18xx.c:171-215`), gehen sie als gültiger Messwert durch. **Das ist die Bestätigung deines offenen Themas 1 aus der Gegenrichtung.**
- **`uart_putc` stellt PRIMASK nicht wieder her** (`uart-driver.h:553-555`): `__disable_irq(); ...; __enable_irq();` ohne Sicherung. `[BELEGT]` als Muster, `[SPEKULATION]` ob der Kontext auftritt.
- **`DISPLAY_OVERLAY_NUM_VAR` ohne Bereichsprüfung** (`main.c:1901-1906`): kein Out-of-Bounds, weil `main.c:3517` gegenprüft, aber der Seiteneffekt überspringt die Ablauf-Prüfung des Icon-Timers.

## F411 vs. F103 — kein erklärender Unterschied gefunden

Das ist ein **Negativergebnis, und es ist wertvoll.**

| | BluePill F103 | BlackPill F411 |
|---|---|---|
| Timer | TIM1 (APB2) | TIM3 (APB1) |
| Capture-Compare | CCR1 | CCR4 |
| GPIO | PA8 | PB1 |
| DMA | DMA1 Channel2 | DMA1 Stream2, Channel5 |
| Speicherbreite | 8 Bit | 16 Bit |

Die Zuordnung DMA1_Stream2/Channel5 ist für TIM3_CH4 **korrekt**. **Kein Fehler gefunden.**

- **TIM2-Rundungsfehler:** F103 15000,0 (0,00 %) gegen F411 15000,75 (0,005 %). **Kein Erklärungsbeitrag.**
- **SK6812-Bit-Timing:** F411 hat mit 10,0 ns die **feinere** Quantisierung, F103 13,9 ns. Beide innerhalb ±150 ns. F411 ist hier **genauer, nicht schlechter**. **Kein Erklärungsbeitrag.**
- **Keine board-abhängigen Puffergrössen oder Flags.** Keine `#ifdef`-Verzweigung nach Board bei `SK6812_MAX_LEDS`, `UART_RXBUFLEN`, `DMA_BUF_LEN`, `WATCHDOG_TIMEOUT_MS`, `MAX_TICKER_LEN`.

**Die einzige Ausnahme mit Richtung „F411 schlechter"** ● gemeldet `[BELEGT]`: Der Watchdog rechnet mit angenommenen LSI-Frequenzen — F103 mit 40 kHz → Reload 3125, F4 mit 32 kHz → Reload 2500 ✔ (`main.c:497-515`). Die LSI-Toleranz des STM32F4 ist aber mit **17 bis 47 kHz** spezifiziert. Bei 47 kHz liefe der IWDG auf F411 mit ≈ **13,6 s statt 20 s**.

⇒ F411 kann Blockaden **früher** in einen Reset umschlagen lassen. Das erklärt, *dass* der Reset früher kommt — nicht, *warum* es hängt.

**Fazit:** Der Software-Anteil (Kernbefunde 1–4, Logging, DMA-Wait) ist **boardunabhängig** und trifft F103 genauso, nur mit 20 s statt evtl. 13,6 s Schwelle. Aus Code-Sicht bleibt deine dokumentierte Vermutung **mechanische Kontaktprobleme** die stärkste verbleibende Hypothese für den hardwarespezifischen Anteil. Kein Grund, die Entscheidung gegen einen pauschalen DMA-Fix zu revidieren.

---

# Teil 2 — PWA-Code

## Kritisch

**Zwei Aufrufe nicht existierender Funktionen** ✔ — beide per Volltextsuche über alle Definitionen bestätigt:

- `app.js:11880` ruft `loadDebugOverrides()`. Definiert sind nur `getDebugOverrides()` (`11590`) und `loadDebugOverridesIntoUi()` (`11613`). Der Aufruf ist der **einzige** Treffer im File. Bei laufender Farbanimation wirft der 5-s-Poller jede Runde `ReferenceError`, `applyWordclockTheme` wird nie erreicht ⇒ **die WordClock-Vorschau friert auf der alten Farbe ein**. Fix: ein Wort.
- `app.js:8415` ruft `normalizeUrlPath()`. **Keine Definition, einzige Fundstelle.** Der Aufruf liegt **vor** dem `try`, das Promise wird verworfen ⇒ **keine Fehlermeldung**. Wählst du für die Layout-Tabelle eine unpassende Datei, wirkt der Button tot.

**`.gz`-Invariante verletzt** ● gemeldet: `renderLocalAppSelectionStatus` (`app.js:7183-7188`) bewertet Assets nur über `has(...)`, `installLocalAppFiles` (`7250-7277`) lädt ohne `file.size > 0` hoch und plant unverifiziert `reloadAppPage`. Eine abgebrochene `app-gz`-Ausführung erzeugt eine 0-Byte-`app.js.gz`; die UI meldet „8 von 8 gefunden", das leere Asset landet im LittleFS, die App lädt neu ⇒ **White Screen, und die PWA ist danach nicht mehr bedienbar, um es zu korrigieren.** Genau der in `CLAUDE.md` dokumentierte Realfall.

**`build-app-bundle.py` bricht im Normalzustand des Repos ab** ● gemeldet: `iter_files()` läuft über alle Dateien in `data/app`, also auch die vorhandenen `*.gz`. `.gz` fehlt in `GZIP_EXTENSIONS` ⇒ `raw.decode("utf-8")` auf Binärdaten ⇒ `UnicodeDecodeError`. Zusätzlich würde `app.js` erneut gezippt **und** die bestehende `app.js.gz` als Plain-Datei eingepackt. Kein Makefile-Aufrufer — toter, aber kaputter Pfad. Entweder `*.gz` überspringen und `st_size > 0` prüfen, oder das Script löschen.

## Hoch

- **`sw.js` hält leere Antworten dauerhaft** ✔ (`sw.js:66`, `86`): nur `response.ok`, nie die Länge. Eine 0-Byte-Datei liefert `200 OK` ⇒ White Screen bleibt **persistent, auch nach Reparatur auf dem Gerät**, bis `CACHE_NAME` steigt. Das erklärt, warum der Fehler damals so zäh war.
- **`cache.addAll` ist atomar** ✔ (`sw.js:25`): acht parallele Requests auf den ESP8266, ein einziger Timeout lässt die ganze Installation scheitern, ohne Retry. Sequenziell mit `cache.put` und Fehlertoleranz pro Asset.
- **`.catch(() => cached)` liefert `undefined`** ✔ (`sw.js:91`): ist nichts im Cache und das Netz weg, wirft `respondWith(undefined)`. Rethrow statt undefined.
- **Versionsdrift zwischen HTML und JS** ✔: `index.html` läuft über `networkFirst` (`sw.js:52`), `app.js` über `staleWhileRevalidate` (`sw.js:93`, `cached || networkPromise`), Pfade unversioniert. Ohne `CACHE_NAME`-Bump ⇒ neue `index.html` mit alter `app.js` ⇒ `getElementById` liefert null. `CACHE_NAME` sollte maschinell aus `APP_VERSION` abgeleitet werden.
- **„Alle Timer gespeichert" auch bei Totalausfall** ● gemeldet: `runButtonRequest` (`app.js:10125-10155`) fängt jeden Fehler und wirft nicht weiter; der `catch` in `saveAllTimerRows` (`10559`) kann nie erreicht werden. Antwortet die STM-UART nicht, laufen alle 8 Slot-Saves in Timeout, alle Buttons werden rot — und danach meldet die App grün „Alle Timer wurden gespeichert".
- **`reloadAppPage` löscht den Cache genau dann, wenn das Gerät weg ist** ● gemeldet (`app.js:9355-9374`): löscht alle `wordclock-app-*`-Caches, dann `location.replace` — aufgerufen u. a. nach ESP-Update, also während der ESP bootet. `networkFirst` hat dann keinen Fallback ⇒ harter Ladefehler. Cache erst nach erfolgreichem Reload verwerfen; `activate` räumt alte Caches ohnehin.
- **Release-Notes ungefiltert per `innerHTML`** ● gemeldet (`app.js:5667`): `releaseNotes` kommt aus `/update_status`, das der ESP vom konfigurierbaren Host über HTTP holt. Ein manipulierter Host oder MITM liefert `<img src=x onerror=…>` ⇒ Script im PWA-Origin mit Zugriff auf EEPROM-Reset und `/eeprom_settings` (WLAN-Credentials). Die 20 anderen `innerHTML`-Stellen escapen korrekt — das ist die einzige Ausnahme.

## Mittel — Auswahl

- **In-Flight-Guard nur bei `opts.auto`** (`app.js:2563`): schnelles Durchwischen der 11 Modul-Chips startet 11 parallele `loadData` ⇒ 30+ gleichzeitige Requests. `requestId` verhindert nur falsches Rendern, nicht den Sturm.
- **`importAmbilightSettings` hat kein einziges `sleep()`** (`app.js:5055-5084`), während jede andere Importfunktion 180–1800 ms pausiert — offensichtlich empirisch ermittelte Werte. Für Ambilight existiert zudem **keine** Verifikations-Retry-Stage ⇒ verlorene Werte bleiben unbemerkt.
- **`days: || 1` im Import gegen `days: || 0` im Export** (`app.js:5150` vs. `3834`): ein Overlay mit `days = 0` erzeugt **garantiert** einen Mismatch ⇒ kompletter Overlay-Reimport (~20 s) als Retry, der erneut scheitert. Jeder Import mit so einem Overlay dauert dauerhaft länger.
- **Stiller `catch (_) {}` beim Löschen aller 32 Overlays** (`app.js:5128-5132`): ein Timeout verschiebt alle folgenden Set-Indizes, der Import meldet trotzdem Erfolg.
- **Nominatim/ipapi ohne `response.ok` und ohne Timeout** (`app.js:6576`, `6654`, `6686`): im AP-Modus hängt `fetch` minutenlang, „Suchen" bleibt disabled. `fetchWithTimeout` (`9417`) existiert bereits.
- **STM32-Reset steht vor dem `try`** (`app.js:4326`): schlägt er fehl, meldet der Import „fehlgeschlagen", obwohl alle Werte geschrieben und verifiziert sind — der Nutzer importiert erneut.

## i18n

- **Drei Keys fehlen komplett** ✔: `common.uploaded`, `common.setting`, `common.set` werden an `8517`, `9924/9926`, `9949/9951`, `10174/10176` benutzt, sind aber in **keiner** der beiden Tabellen definiert. `translate()` (`1870`) gibt als letzten Fallback den Key selbst zurück ⇒ **auf dem Button steht wörtlich `common.setting`**, in beiden Sprachen. `common.uploading` und `common.applied` („gesetzt") existieren bereits — trivialer Fix.
- **14 Keys nur in `de`, nicht in `en`** ● gemeldet (DE 775, EN 761 Keys): ausnahmslos Fehlermeldungen, erscheinen in der englischen UI über den Fallback auf Deutsch.
- **21 vorhandene Keys werden nie benutzt**, weil der Code den deutschen Text hartcodiert — u. a. `maintenance.reset_eeprom_confirm_1/_2`, `maintenance.unsaved_reload_confirm`, `timers.saved_all`. Reines Ersetzen, keine Übersetzungsarbeit.
- **~176 hartcodierte deutsche Strings** brechen den EN-Modus, darunter `app.js:10130` — der **zentrale Fehler-Fallback** von `runButtonRequest` für jede Aktion ohne eigenen `errorText`.
- **`localizeAnimationName` deckt nur 8 Namen ab** (`11577-11588`): `animations.name_fade/_roll/_explode/_snake/_cube/_teletype` sind definiert, aber ungenutzt ⇒ das Select „Anzeigeanimation" zeigt in der deutschen UI englische Namen, obwohl die Übersetzung vorliegt.

---

# Teil 3 — UI/UX

## Kritisch

- **Statusmeldungen unsichtbar** ✔ — siehe Kernbefund 5.
- **Null `aria-live` im ganzen Frontend** ✔: `aria-live` in `index.html` = **0 Treffer**, `role` = genau **einer**. Ohne jede Ansage: alle Statusmeldungen, das STM32-Logbuch (`index.html:210`, ein `<pre>` ohne `role="log"`), der Fortschrittsbalken (`702`, `aria-hidden="true"` **ohne Ersatz**, kein `role="progressbar"`).
- **Modal ohne Dialog-Semantik** ● gemeldet (`index.html:822`): kein `role="dialog"`, kein `aria-modal`, kein `aria-labelledby`. **Kein einziges `.focus()` im ganzen `app.js`** ⇒ der Fokus bleibt hinter dem Overlay, Tab wandert durch die verdeckte Seite. **Kein `keydown`-Handler im ganzen File** ⇒ Escape schliesst nicht. Kein Backdrop-Klick.
- **iPhone-Installation kaputt** ✔: `icons/` enthält **ausschliesslich SVG**, `index.html:25` verweist als `apple-touch-icon` darauf. iOS Safari unterstützt SVG dafür nicht und ignoriert Manifest-Icons. Braucht PNG 180×180, zusätzlich 192/512 PNG im Manifest.

## Hoch

- **`hasUnsavedEdits` wird nach dem Speichern nie zurückgesetzt** ✔: gesetzt in `app.js:2298`, zurückgesetzt nur in `4063` (Backup-Import) und `10502` (Overlay verwerfen). Kein Reset nach erfolgreichem Speichern. Folge 1: der 15-s-Auto-Refresh (`2556`) ist ab der ersten Slider-Bewegung **für die restliche Sitzung** pausiert — Live-Status, Gerätezeit und Vorschau frieren ein, und die Meldung dazu ist nach Kernbefund 5 unsichtbar. Folge 2: „Neu laden" warnt danach **immer** vor ungespeicherten Änderungen.
- **`.button.primary` ist wirkungslos** ✔: `styles.css:162` und `176` deklarieren denselben Gradient und dieselbe Randfarbe. Die Klasse liegt auf ~35 Buttons und tut nichts. In Panels mit 6–8 gleich aussehenden Pills fehlt jede Führung.
- **Deaktivierte Buttons nicht erkennbar** ✔: `styles.css:181` `.button:disabled { opacity: 0.92 }`, kein `cursor: not-allowed`. `#local-app-install-button` startet `disabled` — der Nutzer tippt, nichts passiert, keine Erklärung.
- **Fünf Grid-Area-Klassen existieren nur im CSS** ✔ — nachgezählt, je 1× im CSS, **0× im HTML**: `panel-maintenance-source`, `panel-maintenance-remote`, `panel-display-summary`, `panel-display-controls`, `panel-display-dim`. Und `panel-maintenance-service` sitzt auf dem falschen Panel. Folge im Wartungsbereich ab 900 px: die Panels werden auto-platziert, **„STM32/EEPROM zurücksetzen" rutscht prominent nach oben rechts**, die Orientierungskarte nach unten.
- **Nicht-Text-Kontrast unter WCAG 1.4.11** ● gemeldet, gerechnet auf Panel-Grund ≈ `#0b1524`:

| Element | Datei:Zeile | Ist | Soll |
|---|---|---|---|
| Rand aller Eingabefelder/Selects | `styles.css:614` | **1,42:1** | 3:1 |
| Panel-/Bereichsrand `--line` | `styles.css:16`, `69` | **1,35:1** | 3:1 |
| Karten-/Statusrand | `styles.css:343` | **1,12:1** | 3:1 |
| `.value-pill`/`.state-pill`/`.chip-toggle` | `717`, `793`, `813` | **1,42:1** | 3:1 |

  Eingabefelder sind praktisch nicht als solche erkennbar; bei Sonnenlicht auf dem Smartphone verschwinden die Feldgrenzen ganz. **Der Textkontrast ist dagegen durchweg unauffällig** und ausdrücklich kein Mangel: `--text` auf Panel 16,6:1, `--muted` 8,3:1, `--accent` 11,2:1.

- **Kein designter Fokus-Indikator** ● gemeldet: 0 Treffer für `focus`, `focus-visible`, `outline` in `styles.css`. Positiv: `outline: none` wird **nirgends** gesetzt, SC 2.4.7 ist formal nicht verletzt. Aber der UA-Ring liegt auf `border-radius: 999px`-Pills über `backdrop-filter: blur(16px)` — 3:1 ist nicht sichergestellt.
- **Kein `color-scheme`** ✔: weder Meta-Tag noch CSS. Betroffen: 13 `<select>`, `input[type=time]`, 3× `color`, 6× `range`, 7× `file`, Scrollbars. Auf iOS Safari ⇒ **heller Picker mit schwarzer Systemschrift mitten im tiefdunklen Layout**.
- **Auto-Refresh reisst das Overlay-Feld unter den Fingern weg** ● gemeldet: `app.js:2294-2296` schliesst `#overlay-list` **explizit** vom Dirty-Tracking aus, der 15-s-Refresh pausiert dort also nicht. Die Werte überleben über `captureOverlayEditorState`/`restoreOverlayEditorState` — **der Fokus nicht**. Auf iOS klappt die Tastatur beim Tippen alle 15 s zu.

## Mittel — Auswahl

- **Tab-Muster ohne Tab-Semantik**: 11 Buttons steuern 11 Sections, ohne `role="tablist"`/`tab`/`tabpanel`, `aria-selected`, `aria-controls`. Kein Pfeiltasten-Handling ⇒ 11× Tab durch die Navigation.
- **Keine Formularvalidierung, stilles Zurechtbiegen**: keine `checkValidity`/`reportValidity`, kein `:invalid`-Styling, kein `<form>`. Die `min`/`max`-Attribute werden nie durchgesetzt. `clampNumber` (`app.js:12084`) biegt still zurecht: aus Tag 45 wird 31, aus leer wird 1 — **und der Nutzer bekommt „gespeichert" für einen Wert, den er nie eingegeben hat.** 31.02. wird akzeptiert.
- **Label verspricht 0,5-°C-Schritte, Control erlaubt nur ganze** (`index.html:321/323`, `step="1"`).
- **Rohe technische Fehlertexte**: `throw new Error("http-" + response.status)` landet sichtbar als „…: http-500" (`app.js:6884`); englischer Fallback „unknown error" in der deutschen UI (`7286`).
- **Kein `beforeunload`-Schutz**: das Dirty-Flag schützt nur den App-eigenen Reload-Button. Browser-Zurück, Tab schliessen, Pull-to-Refresh und der Service-Worker-Reload verwerfen Eingaben ohne Rückfrage.
- **Service-Worker-Update erzwingt stillen Reload**: `app.js:2228-2235` lädt bei `controllerchange` nach 150 ms **ohne Rückfrage** neu, und `reloadAppPage` prüft `hasUnsavedEdits` **nicht** — anders als `manualReloadApp`.
- **Kein Offline-Zustand in der UI**: kein `navigator.onLine`, keine `online`/`offline`-Listener, kein Offline-Fallback im Service Worker. Offline lädt die Shell aus dem Cache und zeigt dann leere Felder plus eine unsichtbare Fehlermeldung.
- **Manifest**: beide Icons SVG mit `"purpose": "any maskable"`, obwohl `icon-192.svg` transparente Ecken hat und der Statuspunkt **ausserhalb der Safe-Zone** liegt ⇒ wird beschnitten. Kein `id`, `lang`, `dir`, `description`, `orientation`. `"scope": "/app/"` gegen den Legacy-Link `href="/legacy"` ⇒ in der installierten PWA landet der Nutzer im In-App-Browser ohne Rückweg.
- **Safe-Area nur oben/unten** (`styles.css:43-45`): `safe-area-inset-left/right` fehlt, im iPhone-Landscape sind das 44–47 px ⇒ mit `viewport-fit=cover` liegt Inhalt hinter dem Notch.
- **`vh` statt `dvh` beim Modal** (`styles.css:1173`). Sonst wird `100vh` nirgends fürs Layout genutzt — der klassische Bug existiert gar nicht.
- **Kein `prefers-reduced-motion`**: 0 Treffer, obwohl Bewegung inkl. `transform` vorhanden ist.
- **Weisser Legacy-iframe in der dunklen UI** (`styles.css:946`, `background: #fff`). Die eigene Fortschrittsanzeige existiert bereits — der iframe müsste nur Transportkanal sein.

## Anrede — Du-Form ist eingehalten ✔

Systematisch geprüft: **0 Treffer** für „Sie", „Ihre", „Ihren", „Ihnen" im gesamten deutschen i18n-Block (`app.js:16-793`) und in `index.html`. Die Du-Form wird durchgehend gehalten. „ss" statt „ß" ist konsequente Schweizer Schreibung, echte Umlaute überall, keine ae/oe/ue-Umschriften.

Zwei Register-Ausreisser, beide harmlos aber uneinheitlich:

1. **Acht Aufforderungen im unpersönlichen Infinitiv** (`app.js:496, 513, 519, 589, 638, 669, 695, 723` und `9339`) — „Bitte zuerst eine Datei auswählen." statt Du-Imperativ.
2. **Sieben Statusmeldungen in der 1. Person Singular** (`app.js:594-607`) — „Starte Wiederherstellung…", „Prüfe importierte Einstellungen…". Problem ist nicht die Form, sondern die **Ambiguität**: da Anweisungen sonst im Du-Imperativ kommen, liest sich „Prüfe importierte Einstellungen…" als Aufforderung an dich. Direkt daneben steht `app.js:4334` „App wird neu geladen…" im Passiv. Vorschlag: die sieben Strings aufs Passiv umstellen.

## Was bereits gut gelöst ist

Ehrlich, nicht als Höflichkeit — das sind die Stellen, an denen vergleichbare Geräte-Oberflächen reihenweise scheitern:

- **Formular-Labels**: praktisch jedes Control hat ein echtes Label, meist als umschliessendes `<label class="field">`, bei Slidern korrekt mit `for=`; auch JS-generierte Felder behalten das Muster. Placeholder werden **zusätzlich**, nicht ersatzweise verwendet.
- **Viewport ohne Zoom-Verbot** und `input, select { font: inherit }` ⇒ 16 px, **kein iOS-Auto-Zoom beim Fokus**. Beides bewusst richtig.
- **`min-height: 44px` auf `.button`** für rund 90 Buttons.
- **Doppelklick-Schutz konsequent**: kein Aktions-Button ohne Busy-Behandlung gefunden.
- **Zustände nicht nur über Farbe**: der Button-Text wechselt zu „wird gespeichert…"/„gespeichert"/„Fehler". WCAG 1.4.1 erfüllt.
- **Destruktive Aktionen bestätigt**: 12 `confirm`-Stellen, beim EEPROM-Reset zweifach — angemessen.
- **XSS-Disziplin**: 23 `innerHTML` gegen 79 `escapeHtml`, eine Ausnahme.
- **Schreib-Requests haben `attempts: 1`** ⇒ **kein doppelt gesendetes Display-Kommando**. Retry nur bei 5 Lese-Endpunkten, keiner erzeugt STM-Kommandos.
- **`startBackgroundPause()`** stoppt alle Poller vor Display-Test und Wetterabfragen. Sauber gemacht.
- **Settings-Import durchgehend seriell** mit fein abgestuften `sleep()`-Werten — daran sieht man echte Feldarbeit.
- **Modernes CSS ohne Altlasten**: 74 `var()`-Nutzungen, Grid mit `grid-template-areas`, `clamp()`, `min()`, `minmax(0, 1fr)`, `aspect-ratio`, mobile-first. Nur 18 hartcodierte Hex-Werte, davon 12 in `:root`. Kein Float, kein `!important` ausser bei `.is-hidden`.
- **Sprachumschaltung technisch sauber**: `documentElement.lang` gesetzt, Persistenz in localStorage, Fallback pro Schlüssel, `data-i18n`/`-placeholder`/`-aria-label` getrennt behandelt. Die Mechanik ist gut, nur die Abdeckung lückenhaft.
- **Overflow-Disziplin**: bei 320 px kein horizontaler Überlauf gefunden.

---

# Priorisierte Massnahmen

Reihenfolge nach Wirkung geteilt durch Aufwand. **Nichts davon ist umgesetzt** — Entscheidung liegt bei dir.

| # | Massnahme | Betrifft | Aufwand |
|---|---|---|---|
| 1 | `watchdog_reload()` in die langen Busy-Wait-Schleifen, oder `display_test()`/Ticker in den Hauptloop verlagern | Kernbefund 1 | mittel |
| 2 | Restore-Lücke schliessen: Flag erst löschen, wenn `UPDATE_ALL` wirklich gesetzt wurde | Kernbefund 2 | **Einzeiler** |
| 3 | Globale Meldungsfläche ausserhalb der Sections, mit `role="status"` | Kernbefund 5, Offline, Fehlertexte | klein |
| 4 | `hasUnsavedEdits` nach erfolgreichem Speichern zurücksetzen | Auto-Refresh, Falschwarnung | **wenige Zeilen** |
| 5 | Die beiden `ReferenceError` beheben | Farbvorschau, Tabellen-Upload | **zwei Wörter** |
| 6 | HTTP-Debugzeilen für `/api/`-Pfade unterdrücken | Kernbefund 4, −90 % UART-Last | klein, ESP-Seite |
| 7 | `file.size > 0` bei App-Install, Längenprüfung im Service Worker | `.gz`-Invariante, White Screen | klein |
| 8 | Drei fehlende i18n-Keys ergänzen | sichtbare Roh-Keys auf Buttons | **Minuten** |
| 9 | `color-scheme: dark`, Fokus-Stil, `prefers-reduced-motion`, `dvh`, Safe-Area seitlich | iOS-Optik, Tastatur | klein |
| 10 | `.button.primary` und `:disabled` reparieren | Führung, Erkennbarkeit | **zwei Deklarationen** |
| 11 | Nicht-Text-Kontrast auf ≥ 3:1 | Bedienbarkeit bei Tageslicht | klein |
| 12 | Fünf Grid-Area-Klassen im HTML nachziehen | Desktop-Reihenfolge, Reset-Prominenz | klein |
| 13 | Unbedingte `log_printf` im Refresh-Pfad auf `debug_log_printf` | Logblockade, Kommandoverlust | klein |
| 14 | Modal auf Dialog-Standard, Fokus-Management, Escape | Barrierefreiheit | mittel |
| 15 | PNG-Icons, `apple-touch-icon`, Manifest ergänzen | iPhone-Installation | Asset-Arbeit |
| 16 | DS18xx: CRC-Prüfung im Read, Flag „letzter Messwert gültig" | offenes Thema 1 | mittel |
| 17 | Formularvalidierung statt stillem Clamping | „gespeichert" für falschen Wert | mittel |
| 18 | ~176 hartcodierte Strings in die i18n-Tabelle | EN-Modus | Fleiss |

---

# Diagnoseschritte für die F411-Hänger

Alle Vorschläge sind **Messungen, keine Fixes** — passend zur Entscheidung, erst zu erhärten.
Jeder Schritt nennt die Hypothese und was ihn **widerlegt**.

**F1 — Reset-Ursache auslesen. Kein Codeeingriff.**
Vorhanden: `log_reset_flags()` (`main.c:421-480`), per `var_send_reset_cause()` schon in der UI.
`"Watchdog reset"` ⇒ **bestätigt** einen blockierenden Pfad. `"Software reset"` ⇒ Web-UI-Reset **oder** `fault_reset()` nach HardFault — dann steht davor „fatal fault detected" mit CFSR/HFSR, was die Rekursions-Hypothese stützt. Kein Flag ⇒ Versorgung/Mechanik, kein Software-Hänger. **Billigster Schritt, grösster Informationsgewinn — hier anfangen.**

**F2 — Ein Log an der Restore-Degradierung.**
Stelle: `src/main.c`, im Block `3699-3711`, direkt **vor** `pending_weather_ticker_restore = 0;`. Zu loggen: `display_clock_flag` beim Verbrauch.
Hypothese: „Das Display bleibt dunkel, weil das Restore auf ein bereits gesetztes Flag trifft." `0x01`/`0x02` im Fehlerfall ⇒ bestätigt. Immer `0x00` ⇒ widerlegt.
**Zwingend** als `debug_log_printf` oder reiner Zähler, sonst verschärft das Log die Logblockade.
Reproduktion: Wetter-Ticker starten, währenddessen Dimmkurve speichern.

**F3 — Blockadedauer pro Kommando messen.**
Stelle: `schedule_esp8266_messages()` (`main.c:2648`), Zeitstempel um `schedule_esp8266_cmd()`, **nur** loggen bei > 50 ms.
Erwartete Treffer: `n` (~256 ms), `R`+`TEST_DISPLAY` (21/45 s), `S`+`TICKER_TEXT`, `O`-Serien. Bleiben alle klein ⇒ die Kommandokosten-Thesen sind widerlegt.

**F4 — Loop-Periode als Histogramm.**
Stelle: `main.c:3168-3171`, Delta in Buckets, Ausgabe **einmal pro Minute**.
Buckets ≥ 100 ms bei PWA-, nicht bei Legacy-Bedienung ⇒ bestätigt. Gleiche Verteilung ⇒ die Kommandorate ist nicht die Ursache.

**F5 — Verworfene UART-Zeichen zählen. Prüft Kernbefund 4 direkt.**
Stelle: `uart-driver.h:698`, im heute **fehlenden** `else`-Zweig einen Zähler; Ausgabe einmal pro Minute im Hauptloop, **nie** in der ISR.
Zähler > 0 bei offenem PWA-Tab, 0 bei Legacy ⇒ bestätigt. In beiden 0 ⇒ widerlegt.

**F6 — Gegenprobe ESP-seitig: `/api/`-Debugzeilen unterdrücken.**
Stelle: `http.cpp:11196` und `11251`/`11305`.
Verschwinden die Freezes bzw. geht F5 auf 0 ⇒ bestätigt, und der Hebel liegt auf der ESP-Seite — **ohne eine Zeile `app.js` und ohne STM-Änderung**. Rechnerische Erwartung: −90 % UART-Last.

**F7 — Gegenprobe STM-seitig: unbedingtes Logging abschalten.**
`sk6812.c:476`, `:495` und `main.c:3745-3753` auf `debug_log_printf` umstellen, dieselben Szenarien fahren.
Verschwinden die Freezes ⇒ bestätigt, und die Konsequenz ist ein Log-Konzept (nicht blockierend, ratebegrenzt), **kein DMA-Fix**.

**F8 — `"sk6812_refresh: waiting"` in den Mitschnitten suchen. Kein Codeeingriff.**
Das Log existiert bereits (`sk6812.c:463-472`) ✔.
Zeile vorhanden ⇒ DMA-Stillstand **bestätigt**, samt `pos`/`off`/`pause`. Zeile fehlt in allen Hänger-Mitschnitten ⇒ **widerlegt** — und deine Entscheidung gegen einen pauschalen DMA-Fix ist damit belegt statt nur vermutet. **Zweitbilligster Schritt, mach ihn zusammen mit F1.**

---

# Was nicht geprüft wurde

Ehrliche Grenzen dieses Reviews:

- **Kein Laufzeittest.** Nichts gebaut, nichts geflasht, nichts im Browser ausgeführt. Alle Zeitangaben sind aus Code und Konstanten **gerechnet, nicht gemessen** — insbesondere ~16 ms/Byte EEPROM, ~8,9 ms pro Logzeile und die 21/45 s von `display_test()` sollten per F3 verifiziert werden, bevor daraus Fixes abgeleitet werden.
- **`src/overlay/overlay.c`** — EEPROM-Kosten der `overlay_*`-Setter und ob `overlay_set_n_overlays()` gegen `MAX_OVERLAYS` prüft. **Wichtigste offene Lücke**, weil `/api/overlay_set` 8 und `/api/overlay_delete` bis 8×n Kommandos erzeugt.
- **Animationsfunktionen** in `display.c` — ob eine `display.animations[].func` `animation_start_flag` in einem Zustand hinterlässt, aus dem sie nicht herauskommt. Entscheidet, ob die Temperatur-Restore-Blockade vorübergehend oder **permanent** ist.
- **`remote_ir_learn()`** — Dauer nicht verifiziert, potenziell > 20 s.
- **ESP-seitiger Code** als Ganzes (`http.cpp` > 11'000 Zeilen), `weather.cpp`, `httpclient.cpp` — nur die für Kommando-Mapping und Debugzeilen relevanten Stellen. Damit auch offen: ob der Server `.gz` korrekt mit `Content-Encoding` ausliefert und Grösse > 0 prüft, und wie `overlay_delete`/`overlay_set` Indizes verschieben.
- **`src/night/`, `src/alarm/`, `src/dfplayer/`, `src/ldr/`, `src/rtc/`** — nicht gelesen.
- **Tatsächliche Darstellung** — kein Browser, kein Gerät, kein Lighthouse-/axe-/VoiceOver-Lauf. Die Kontrastwerte beruhen auf einer Blend-Rechnung über die Gradient-Layer, nicht auf gemessenen Pixeln: Zahlen ±0,2, AA-Bewertungen eindeutig.
- **Performance** — keine Messung von Ladezeit, Kosten des 15-s-Refreshs oder Speicherverhalten auf älteren iPhones. `app.js` ist 464 KB / 94 KB gzip; nicht bewertet.
- **Sicherheit** — kein CSP-, Auth- oder CSRF-Review. `index.html` hat keine CSP.
- **Legacy-Oberfläche** — nicht angesehen, also kein direkter Vergleich der Bedienlogik.
- **Übersetzungsqualität EN** — nur Schlüssel-Parität automatisiert geprüft, keine inhaltliche Durchsicht der 761 englischen Strings.
- **`.gz`-Artefakte gegen Quellstand** — nicht geprüft, ob sie zum geprüften Code passen (`app.js` 19.07. 23:05, `styles.css` 29.04.).

---

# Methodik

Drei parallele Review-Agents, rein lesend, mit Schreibverbot auf Projektcode und Build-Verbot (Regeln R1/R2 in `CLAUDE.md`). Achsen: PWA-Korrektheit · PWA↔STM-Display · UI/UX.

Der Display-Agent hatte die Auflage, jeden Punkt als `[BELEGT]`/`[PLAUSIBEL]`/`[SPEKULATION]` zu labeln, und ausdrücklich die Erlaubnis, „kein Beleg gefunden" als Ergebnis zu liefern — statt eine Ursache zu erfinden. Beim F411-Thema hat er genau das getan, und das Negativergebnis ist einer der wertvollsten Punkte des Berichts.

Der Lead hat die tragenden Behauptungen anschliessend selbst am Code nachgeprüft; diese sind mit **✔ verifiziert** markiert. Kein Fund wurde bei der Gegenprüfung widerlegt.
