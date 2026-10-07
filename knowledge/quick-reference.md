# Quick Reference — Ich sehe / Ich tue / Schweregrad

Nachschlagewerk für jeden Review- und Guardrail-Schritt. Jeder Eintrag stammt aus
einem **belegten** Befund in `REVIEW.md` oder aus einer Architektur-Invariante in
`CLAUDE.md` — keine allgemeinen Ratschläge.

**Kritisch** blockiert den Task. **Hoch** blockiert, wenn nicht ausdrücklich begründet
abgewichen wird. **Mittel** wird gemeldet und darf bewusst offenbleiben.

## STM32-Firmware (`src/**`)

| Ich sehe… | Ich tue… | Schweregrad |
|---|---|---|
| Busy-Wait, `delay_sec`, `delay_msec` oder Warteschleife im Web-Kommandopfad ohne `watchdog_reload()` | Pfad in den Hauptloop verlagern oder Reload ergänzen. Alles über 20 s ist ein garantierter IWDG-Reset. **Den gültigen Bestand an Aufrufstellen nennt S7** (`./tools/guardrails.sh`) — hier stand bis 03.10.2026 „genau eine Aufrufstelle, `main.c:3170`", und das war längst falsch: Es sind sechs, und `main.c:3170` ist nicht mehr die richtige Zeile. Eine abgeschriebene Zahl veraltet still, eine Prüfung nicht | Kritisch |
| `log_printf` in einem Pfad, der pro Display-Refresh läuft | **Entfernen und durch einen Zähler ersetzen**, den die Diagnosezeile mitträgt — **nicht** auf `debug_log_printf` umstellen. Jede Logzeile blockiert über `esp8266_uart_flush()` und verzögert gleichzeitig das Lesen vom ESP | Kritisch |
| Die Empfehlung „auf `debug_log_printf` umstellen" (bis 03.10.2026 die Regel hier) | **Sie löst das Problem nicht, sie verschiebt es.** Im Normalbau fehlt die Zeile dann ganz, im Diagnosebau (`-DWORDCLOCK_DEBUG=ON`) flutet sie wieder — und der 64-Zeilen-Ring reicht unter LED-Last ohnehin nur rund 1,2 s. Ein Zähler kostet nichts und ist in beiden Bauarten da. Am Gerät gemessen: 96 % aller Ringzeilen stammten aus zwei solchen Aufrufen in `sk6812.c` | Hoch |
| `display_clock_flag = …` als Zuweisung, wo ein anstehendes Update verloren gehen kann | Prüfen, ob verodert werden muss. Die Flags sind disjunkte Bits (`0x01`, `0x02`, `0x04`) | Kritisch |
| Ein Flag wird unbedingt gelöscht, seine Wirkung aber nur bedingt gesetzt | Erst löschen, wenn die Wirkung tatsächlich eingetreten ist. Siehe `pending_ticker_restore` in `src/main.c` (die Restore-Bedingung im Hauptloop) | Kritisch |
| `while (…) { }` auf ein ISR-gesetztes Flag ohne Timeout und ohne Reload | Timeout und Diagnoseausgabe ergänzen, oder bewusst dokumentieren | Hoch |
| 1-Wire- oder bitgenaues Timing ohne `__disable_irq()` | Interrupt-Schutz erwägen; ohne ihn sind Lesefehler erwartbar | Hoch |
| Sensorwert wird ohne CRC-Prüfung als gültig übernommen | CRC prüfen, „gefunden“ und „Messwert gültig“ trennen | Hoch |
| `__disable_irq(); … __enable_irq();` ohne PRIMASK-Sicherung | PRIMASK sichern und wiederherstellen | Mittel |

**Falle beim Durchsuchen von `src/**` (B21/L166):** Acht Dateien sind **ISO-8859-1**, nicht UTF-8 —
`base.c`, `base.h`, `display.c`, `ds18xx.c`, `irmp.c`, `rtc.c`, `tempsensor.c`, `w25qxx.c`.
**Den gültigen Bestand nennt S7b**, nicht diese Liste: Sie ist eine Kopie und veraltet still,
sobald eine Datei dazukommt oder umkodiert wird — dieselbe Gattung wie die Versionsnummern in
der Doku. Die Namen stehen hier nur, damit man die Falle beim Lesen wiedererkennt.
`grep` stuft sie als **binär** ein und gibt **gar nichts** aus — nicht „0 Treffer", sondern
eine leere Ausgabe. Das sieht aus wie „nicht vorhanden", ist aber „nicht gelesen".

**Das Heilmittel ist `grep -a`, nicht das Locale.** `LC_ALL=C` allein behebt es
nachweislich **nicht**: `LC_ALL=C grep -c 'display_icon' display.c` liefert leer,
`grep -ac` liefert 29. Das ripgrep-basierte Grep-Tool ist nicht betroffen.
Betrifft direkt `display.c` (Display-Zustandsmaschine) und `ds18xx.c`/`tempsensor.c`
(offenes DS18xx-Thema).

## ESP8266 (`ESP8266/ESP-uclock/*.cpp`, `*.ino`)

### Jede neue `static`-Funktion in der `.ino` braucht einen eigenen Prototyp

`arduino-cli` erzeugt für jede Funktion in einer `.ino`-Datei selbst einen Prototyp —
**ohne `static`**. Ist die Definition `static`, bricht der Bau ab:

```
error: 'void xyz()' was declared 'extern' and later 'static' [-fpermissive]
```

Nachzulesen im erzeugten Zwischenstand unter `build/esp8266/sketch/ESP-uclock.ino.cpp`.
Ein **handgeschriebener** Prototyp unterdrückt die Erzeugung — deshalb tragen `icon_info`
und `resolve_icon_asset_filename` einen, und deshalb brach `esp_heap_log()` am 04.10.2026
den Build (L177).

**Steht die Funktion in einem `#if`-Block, gehört der Prototyp in denselben Block.** Ihn
nach oben zu den anderen zu stellen scheitert, wenn das `#define` erst weiter unten steht:
Die Bedingung ist oben noch 0, der Prototyp entfällt, der Fehler kommt zurück.

**Die Zahl steht hier bewusst nicht mehr.** Hier stand „2 von 2“, und das war schon falsch, bevor es jemand las — `esp_heap_log()` war die dritte, inzwischen sind es sieben. Eine Zahl, die mit dem Code waechst, veraltet still; dieselbe Gattung wie die Versionsnummern in der Doku. **Nachzaehlen statt nachschlagen.** Die nächste ohne
bricht den Bau wieder.


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

## Quelldateien patchen — Kodierung und Zeilenenden (`src/**`, `ESP8266/ESP-uclock/`)

Für `stm-developer` und `esp-developer`. **Diese drei Zeilen standen bis zum 05.10.2026 im
UI-Abschnitt** — also unter einer Überschrift, unter der niemand nachschlägt, der eine
C-Quelle patcht (L279). Der Text ist unverändert, nur der Ort ist jetzt der richtige.
Die Regel dahinter trägt DIR-015.

| Ich sehe… | Ich tue… | Schweregrad |
|---|---|---|
| C-Quellen mit Python im **Textmodus** patchen | **Binär lesen und schreiben.** Die Quellen haben gemischte Zeilenenden — `vars.c` etwa 940 CRLF und 45 LF. Universal Newlines vereinheitlichen sie still, und aus einem Dreizeiler wird ein Diff über die ganze Datei | Kritisch |
| Suchmuster mit Umlauten binär patchen | **Umlaute aus dem Muster heraushalten.** Der Patcher kodiert `latin-1`, `http.cpp` und `stm32flash.cpp` sind UTF-8 — ein `ü` im Muster trifft nie. Kostete einen stillen Fehlschlag, der erst beim Nachzählen auffiel | Hoch |
| Vor dem Patchen die Kodierung **annehmen** statt nachsehen | **Nachsehen.** Welche Datei welcher Gruppe angehört, sagt S7b: „UTF-8 mit Umlauten" wird namentlich gelistet (dort beschädigt ein `latin-1`-Patcher die Datei), „nicht UTF-8" gezählt (dort braucht `grep` ein `-a`). Bis 03.10.2026 behauptete `CLAUDE.md` pauschal, alle ESP-Quellen seien ISO-8859-1 — während diese Zeile hier das Gegenteil sagte. Zwei Dokumente, zwei Aussagen, und die falsche stand in dem, das immer lädt | Hoch |

## Das Edit-Werkzeug vereinheitlicht Zeilenenden — still

**Kritisch.** Bisher stand hier nur die Falle für Python-Patches (Textmodus
vereinheitlicht Zeilenenden, Kodierung je Datei prüfen). Das Edit-Werkzeug hat
dieselbe: Es schreibt die Datei mit dem **dominanten** Zeilenende zurück.

Am 03.10.2026 bekam `ESP8266/ESP-uclock/vars.cpp` (1066 CRLF, rund 120 LF) dadurch
121 stille LF→CRLF-Änderungen, und `http.cpp` verlor sein einziges CR. Der Diff sah
danach nach einem grossen Umbau aus, obwohl nur wenige Zeilen gemeint waren.

**Nach jeder Bearbeitung einer Datei mit gemischten Zeilenenden gegenprüfen:**

```
git show HEAD:<datei> | grep -c $'\r'    # vorher
grep -c $'\r' <datei>                    # nachher
```

Die Differenz muss der Zahl der **neu hinzugefügten** Zeilen entsprechen. Stimmt sie
nicht, wurden unberührte Zeilen umgeschrieben. Betroffen sind in diesem Repo
`vars.cpp`, `vars.h`, `http.cpp`, `src/vars/vars.c`, `src/main.c` und
`ESP8266/ESP-uclock/data/app/styles.css` (56 CRLF-Zeilen). Bei der Stilvorlage trat es
am 03.10.2026 auf: Nach einem `Edit` standen 0 CR statt 56, und der Diff zeigte
`89 insertions, 56 deletions` statt der gemeinten 33 Zeilen.

## Ein Prüfstand auf dem Entwicklungsrechner ist nicht die Zielplattform

**Kritisch.** Wo eine **Typbreite** über den geprüften Fall entscheidet, gehört sie an die
Zielplattform angeglichen — sonst läuft die Prüfung, ist grün und hat den Fall nie
hergestellt.

Belegt am 05.10.2026 (L256): Ein Test sollte den Überlauf der Millisekundenuhr prüfen.
`unsigned long` ist auf dem ESP **32 Bit**, auf dem Entwicklungsrechner **64**. Die
simulierte Uhr lief einfach über 2³² hinaus weiter und wickelte **nie** — grün, und ohne
jede Aussage. Mit mechanischer Angleichung (`unsigned long` → `uint32_t`) wickelt sie
tatsächlich: `4294962345 ms → 5049 ms → 15049 ms`, Abstände weiterhin 10 Sekunden.

**Das ist die Gattung von DIR-014 in ihrer heimtückischsten Form.** Nicht eine Prüfung,
deren Meldung nicht ankommt, und nicht eine mit zu engem Muster — sondern eine, die auf
der **falschen Plattform** läuft. Bemerkt hat es der Umsetzer selbst; er lässt seitdem
**beide** Übersetzungen über alle Fälle laufen, die angeglichene und die native.

**Betroffen ist alles, was plattformabhängig breit ist:** `unsigned long`, `int`, `size_t`,
Zeigerbreite, `time_t`. Prüfstand bauen heisst: erst die Breiten festnageln, dann messen.

## Zwei Warnungen für jeden künftigen Entwurf an der STM↔ESP-Brücke

Beide stammen aus `specs/paket-2026-10-05/design.md` §6.5 und sind allgemeiner als ihr
Anlass. Sie stehen hier, weil sie **vor** dem Entwurf gebraucht werden, nicht danach.

### Hardware-Nebeneffekte sind keine Absicherung

> **Ein Mechanismus, dessen Sicherheit an einem Hardware-Nebeneffekt hängt, ist nicht
> abgesichert — er hat bisher Glück gehabt.**

**Hoch.** Der A35-Nachweis fand keinen Pfad, auf dem der ESP einen Sitzungsmerker behält,
während eine STM-Firmware ohne die zugehörige Fähigkeit läuft. Diese Sicherheit hing aber
an zwei Dingen, die **nirgends als Voraussetzung benannt waren**: am GPIO-Puls in
`esp8266_reset()` (`src/esp8266/esp8266.c`) **und** am Einschaltstrom der LED-Kette über
`PB0`, der bei einem STM-Reset den ESP mitreisst (L183). Fiele eines von beiden weg — ein
anderer Resetpfad, eine andere Platinenrevision —, wäre die Zusage still verloren, ohne
dass jemand den Zusammenhang kennt.

**Dasselbe gilt rückwirkend für `cap_var_crc`:** Die Begründung in L254, der Fall „STM
startet neu, ESP nicht" trete in der Praxis gar nicht ein, stützt sich auf genau diesen
Einschaltstrom. Das ist heute richtig und war nie als Bedingung aufgeschrieben.

**Ich tue:** Jeder Entwurf, der eine Sitzungsfähigkeit oder einen Merker über einen Reset
hinweg braucht, **nennt die Voraussetzung, unter der sein Rückfall gilt** — und zwar im
Entwurf, nicht im Nachhinein im Befundkatalog.

### Verworfen heisst nicht unquittiert

> **Eine Zeile, die auf der Wirkungsebene verworfen wird, kann auf der Protokollebene
> trotzdem quittiert sein — und ob sie es ist, entscheidet über die Watchdog-Bilanz.**

**Kritisch.** „Ein alter Empfänger verwirft die neue Zeile stillschweigend" ist eine
Aussage über die **Wirkung**. Auf der **Protokollebene** kann das Gegenteil gelten, und
das ist kein Detail: Die `var `-Verpackung wird von einem alten ESP regulär als
`var`-Kommando empfangen und **quittiert** — deshalb läuft `var_send_buf()` nicht in
seinen Timeout. Ein eigenes Top-Level-Präfix hätte genau diese Quittung verloren und damit
**3 Sekunden Stillstand je Zeile** erzeugt, ohne `watchdog_reload()` (L269).

**Ich tue:** Bei jeder Änderung an der Verpackung eines Kommandos getrennt beantworten —
(1) Was tut der alte Empfänger mit der Zeile? (2) **Quittiert er sie?** Nur die zweite
Frage entscheidet über Timeout und Watchdog. **Wer die Verpackung ändert, ändert nicht die
Form, sondern die Watchdog-Bilanz.**
