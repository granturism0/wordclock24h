# Design — Die Uhr beobachtbar machen

## Leitsatz

**Was dauerhaft laufen soll, muss im Normalfall schweigen. Was viel sagt, läuft nur
auf ausdrückliche Anforderung.**

Daraus folgt alles Weitere: Die beiden Durchlaufmeldungen je Refresh verschwinden und
werden durch **Zähler** ersetzt, die nur beim Berichten Platz kosten. Die
Fehlermeldungen bleiben, weil sie im Normalbetrieb null Byte kosten. Und das, was
immer laufen muss — die Diagnosezeile —, ist eine einzige Zeile je Takt.

## 1 — Die Quelle der Flut: `sk6812.c`

### Was entfällt

`sk6812.c:476` (`sk6812_refresh: start …`) und `:495` (`sk6812_refresh: dma started
…`) werden **ersatzlos entfernt**. Beide melden, dass der Normalfall eingetreten ist.

### Was bleibt

`sk6812.c:463` (`sk6812_refresh: waiting %lus …`) bleibt unverändert. Sie feuert
höchstens einmal je Sekunde und nur, wenn der DMA tatsächlich wartet — sie meldet
eine Abweichung, nicht einen Durchlauf. Sie ist das Widerlegungsinstrument für
Gerätetest **M3**. Wer sie mitentfernt, nimmt `REVIEW.md` sein einziges Mittel, den
DMA-Stillstand auszuschliessen.

### Was an ihre Stelle tritt

Zwei Zähler in `sk6812.c`, gelesen über neue Zugriffsfunktionen:

| Zähler | Typ | Erhöht bei |
|---|---|---|
| Refreshes | `uint32_t` | jedem Eintritt in `sk6812_refresh()` |
| DMA-Wartefälle | `uint16_t`, **sättigend bei 65535** | jedem Durchlauf, bei dem die Warteschleife mindestens eine volle Sekunde gewartet hat — also genau dann, wenn `:463` ausgibt |

Damit ist der Informationsgehalt der entfernten Zeilen nicht verloren, sondern
verdichtet: Statt 50 Zeilen pro Sekunde zu sagen „der Refresh läuft", sagt die
Diagnosezeile alle zehn Sekunden, **wie oft** er lief. Und das Ausbleiben des
Refreshs — der eigentlich interessante Fall aus dem Hänger — ist an einem
stehenden Zähler ablesbar, statt am Fehlen von Zeilen, die ohnehin überschrieben
wären.

Sättigend statt umlaufend: Ein Zähler, der bei 65535 stehenbleibt, liest sich als
„mindestens 65535". Ein umlaufender liest sich als eine kleine Zahl und lügt dabei.

## 2 — Der Logring bleibt, wie er ist. Mit Zahlen begründet

Die naheliegende Antwort auf „der Ring ist zu klein" wäre, ihn zu vergrössern. Das
geht nicht, und das ist am Linker-Protokoll belegbar
(`build/esp8266/ESP-uclock.ino.map`):

| Grösse | Wert | Fundstelle |
|---|---|---|
| `dram0_0_seg` gesamt | 81'920 Byte | Map, Zeile 13195 |
| `_heap_start` | `0x3fff8f90` | Map, Zeile 30273 |
| **statisch belegt** | **69'520 Byte** | Differenz |
| **frei für Heap und Stack** | **12'400 Byte** | Differenz |
| `stm32_log_lines` heute | 7'744 Byte (`0x1e40`) | Map, Zeile 29715 |

Von 64 auf 128 Zeilen zu gehen kostet weitere 7'744 Byte — **62 % des gesamten
verbleibenden Arbeitsspeichers**, auf einem Gerät, das aus demselben Heap seine
HTTP-Antworten und den WLAN-Stapel bedient. Das ist keine knappe Entscheidung.

**IRAM ist hier nicht die Grenze.** Die im Auftrag genannten 92 % betreffen den
Befehlsspeicher; ein statisches Feld liegt in `.bss` im DRAM. Die Zahl, auf die es
ankommt, sind die 12'400 Byte oben.

Kürzere Zeilen wären die zweite Möglichkeit: 120 auf 100 Zeichen spart 1'280 Byte.
Sie fällt aus, weil die Warteschleifen-Zeile aus `sk6812.c:463` rund 105 Zeichen
lang ist — die Kürzung würde ausgerechnet das Instrument kappen, das bleiben soll.

**Entscheidung: Geometrie unverändert, 64 × 120.** Der Gewinn kommt ausschliesslich
aus der Quellrate. Das ist ohnehin der bessere Hebel: Eine grössere Ablage hätte
weder die 250 ms/s Hauptloop-Blockade beseitigt noch die Verdrängung im
Empfangspfad. Beides tut die Entlastung an der Quelle.

### Die Zahl, auf die es ankommt

| | Rate | Reichweite des 64-Zeilen-Rings |
|---|---|---|
| Ruhe, heute (gemessen) | 0,05 Zeilen/s | 21 min |
| Ruhe, nachher (0,05 + Diagnosezeile alle 10 s) | 0,15 Zeilen/s | **rund 7 min** |
| Ticker-Overlay, heute (L75) | rund 50 Zeilen/s | **1,2 s** |
| Ticker-Overlay, nachher | erwartet nahe am Ruhewert, weil die gemessene Flut **ausschliesslich** aus den beiden entfernten Zeilen bestand und `display.c` nur ereignisbezogen loggt (`display.c:3246`, `:4928`, `:5064`) | **erwartet mehrere Minuten, gefordert ≥ 60 s** |

Gefordert wird deutlich weniger als erwartet. Das ist Absicht: Eine Schwelle, die
knapp über der Erwartung liegt, besteht oder fällt nach Messrauschen. 60 s dagegen
ist um den Faktor 50 von heute entfernt — wird sie verfehlt, gibt es eine zweite,
unbekannte Quelle, und das ist dann der eigentliche Befund.

**Wenn ein Takt von 30 s statt 10 s gewählt wird**, liegt die Ruhe-Reichweite bei
rund 12,8 min statt 7 min, die Zeitauflösung im Hängerfall dagegen bei 30 s.
Vorschlag: **10 s**, weil die Auflösung im Fehlerfall mehr wert ist als die
Reichweite im Ruhezustand — im Ruhezustand ist ohnehin nichts zu sehen. Die Zahl
steht als eine benannte Konstante an einer Stelle.

## 3 — Die Diagnosezeile

Eine einzige Zeile, die alles trägt, was dieses Paket messen soll. Sie ersetzt
sowohl den „Herzschlag" als auch die Frage „wie bekomme ich den Verwurfszähler
heraus".

### Vorgeschlagenes Format

```
diag <seq> l=<loop> t=<tick> u=<uptime> r=<refresh> w=<dmawait> rx=<max>/<size> d=<drops>
```

Längenrechnung bei maximalen Feldbreiten: 5 + 10 + 13 + 13 + 13 + 13 + 8 + 11 + 8 =
**94 Zeichen**. Die Grenze liegt bei 120 (Ringzeile) beziehungsweise 123
(Kommandopuffer des ESP, `CMD_BUFFER_SIZE` 128 abzüglich `LOG `). Wer das Format
erweitert, rechnet neu — AK9 macht eine Überschreitung zwar sichtbar, aber eine
sichtbar gekappte Diagnosezeile ist trotzdem eine verlorene.

### Was die Felder beantworten

`l`, `t` und `u` zusammen sind der Kern. Sie trennen drei Fälle, die heute nicht zu
trennen sind:

| `l` Hauptloop | `t` Zeitgeber-ISR | `u` Uptime | Folgerung |
|---|---|---|---|
| steigt | steigt | steigt | Die Zeitbasis lebt. Der Ausfall liegt **hinter** den Flags, im Hauptloop-Zweig |
| steigt | steigt | steht | Die ISR läuft, aber ihr Sekundenzweig nicht — `clk_cnt` erreicht `F_INTERRUPTS` nicht mehr (`main.c:873`) |
| steigt | steht | steht | Die Zeitgeber-ISR selbst kommt nicht mehr dran |
| keine Zeile | — | — | Der Hauptloop steht, oder die Brücke sendet nicht mehr. Erkennbar an der Lücke in `<seq>` |

Das ist genau die offene Frage aus `haenger-2026-09-30.md`, in drei unterscheidbare
Fälle zerlegt. **Es ist keine Antwort darauf. Es ist das Mittel, die Antwort zu
bekommen.**

`t` zählt **jeden** Zeitgeber-Interrupt (`F_INTERRUPTS`, 15'000/s), nicht die
Sekunden. Nur so unterscheidet sich Zeile 2 von Zeile 3 der Tabelle. Ein `uint32_t`
läuft nach rund 3,3 Tagen um; der Umlauf ist als Rücksprung sichtbar und stört die
Auswertung nicht, weil ausgewertet wird, **ob** der Wert sich ändert.

### Takt — und warum er nicht an `uptime` hängen darf

Die Zeile wird am Kopf des Hauptloops ausgegeben, unmittelbar nach
`watchdog_reload()` (`main.c:3357`), neben der bestehenden `icon_freeze`-Prüfung.
Nicht in einem flaggesteuerten Zweig — der Zweig ist der Verdächtige.

Zwei Auslöser, verodert:

1. **Regulär:** `t` ist seit der letzten Zeile um `10 × F_INTERRUPTS` gestiegen.
2. **Rückfall:** `l` ist seit der letzten Zeile um mehr als ein Budget gestiegen.

Das Budget ist **selbstkalibrierend**: Nach jeder regulär ausgegebenen Zeile wird es
auf das Vierfache der seither gezählten Hauptloop-Durchläufe gesetzt, mindestens
1'000. Vor der ersten regulären Zeile gilt 1'000'000.

Warum nicht eine feste Zahl: Niemand weiss heute, wie viele Durchläufe der Hauptloop
pro Sekunde schafft — die Zahl hängt an Hardware, Anzeigezustand und Last. Eine fest
eingetragene Konstante wäre entweder zu klein (sie flutet) oder zu gross (sie meldet
nie). Das Vierfache des beobachteten Zehn-Sekunden-Pensums kann beides nicht: Es
flutet nicht, weil es per Konstruktion über der Normallast liegt, und es meldet
spätestens nach rund 40 s Arbeit ohne Zeitgeber. Zugleich veraltet die Zahl nicht
still, wenn sich die Last ändert.

### Kosten

- **Je Zeile:** rund 95 Byte, über zwei UARTs, davon die zum ESP mit blockierendem
  Flush: rund 8 ms. Bei einer Zeile je 10 s sind das **0,08 % der Rechenzeit**.
- **Zeitgeber-ISR:** eine Inkrementierung bei 15'000 Aufrufen/s. Bei rund 3 Takten
  und 100 MHz sind das **0,0045 %**.
- **Arbeitsspeicher STM:** rund 30 Byte für alle Zähler zusammen. Der F103 hat
  20 kB.

### Warum eine Logzeile und nicht eine numerische Variable

Geprüft, verworfen, aus drei Gründen:

1. **Eigentum.** Eine numerische Variable braucht eine Kennung im gemeinsamen
   Protokoll, Ablage im ESP, ein Feld in der API und eine Anzeige in der PWA — vier
   Besitzer für einen Diagnosewert. R3 verbietet einen Task über zwei Besitzer; das
   wären vier Tasks für das, was eine Zeile leistet.
2. **Der Melder würde den Fehler mitmachen.** Eine Variable geht durch
   `var_send_buf()` und **wartet dort auf die Quittung** — der Bericht über eine
   gestörte Brücke würde auf ebendieser Brücke blockieren.
3. **Richtungstrennung.** Verworfen wird in der Empfangsrichtung des STM. Die
   Logzeile geht in die Senderichtung und ist davon unberührt. Fällt auch die aus,
   ist das an der Lücke in `<seq>` zu sehen — also ebenfalls ein Ergebnis.

## 4 — Der Verwurfszähler

### Wo gezählt wird

In `src/uart/uart-driver.h`, in `UART_IRQ_HANDLER()`:

- **Verwürfe:** der heute fehlende `else`-Zweig zu `if (uart_rxsize < UART_RXBUFLEN)`
  (`:698`). Eine sättigende `uint16_t`-Inkrementierung. Im Normalfall wird dieser
  Zweig **nie betreten** — die Kosten sind exakt null, solange nichts verloren geht.
- **Höchster Füllstand:** ein Vergleich mit anschliessendem Speichern im Erfolgszweig,
  also bei jedem empfangenen Zeichen. Höchstens 11'520 Zeichen/s bei 115200 Baud,
  3 bis 4 Befehle je Zeichen, bei 100 MHz **unter 0,05 % Rechenzeit**. Das ist der
  Preis dafür, dass „0 Verwürfe" etwas aussagt.

Die Datei wird für jede UART einzeln eingebunden (`log`, `esp8266`, `dfplayer`,
weitere). Die Zähler entstehen damit je Instanz, benannt über das vorhandene
Präfixverfahren — wie `UART_PREFIX_RSIZE` (`uart-driver.h:434`). Zwei neue
Zugriffsfunktionen in `uart.h`, analog zu den bestehenden Deklarationen. Kosten je
Instanz: 6 Byte.

### Warum „0" ohne die zweite Zahl wertlos ist

Ein Zähler auf 0 hat zwei mögliche Ursachen: Es ging nichts verloren, oder es wird
nicht gezählt. Diese beiden sind am Zähler selbst nicht zu unterscheiden — und ein
Messmittel, dessen Gutfall von seinem Totalausfall ununterscheidbar ist, ist keines.

Der höchste Füllstand löst das: Steht er über 0 und verändert sich mit der Last,
dann läuft der Zählpfad, der Ring wurde benutzt, und die 0 bei den Verwürfen ist
ein **Ergebnis**. Deshalb steht in AK4, dass „beide auf 0" als nicht bestandene
Messung gilt.

Nebeneffekt, der nichts kostet: Der Füllstand sagt auch, **wie nah** es war. 12 von
256 ist etwas anderes als 240 von 256, und beide melden heute dasselbe, nämlich
nichts.

## 5 — `watchdog_reload()` in `var_send_buf()`, aber nur nach einer Quittung

### Die Stelle

In `var_send_buf()` (`vars.c:52-100`), **nicht** in der Schleife von
`var_send_all_variables()`. Begründung aus dem Code: Die Gruppenfunktionen senden
intern dutzende Kommandos — `var_send_overlays()` acht je Overlay,
`var_send_num8_array()` eines je Element der 16-stufigen Dimmkurve
(`vars.c:174-187`). Ein Reload zwischen den Gruppenaufrufen liesse im
ungünstigsten Fall 32 × 3 s = 96 s zwischen zwei Bedienungen. Das wäre ein Reload,
der nicht wirkt, und damit schlimmer als keiner — er stünde im Code und man hielte
den Pfad für abgesichert.

In `var_send_buf()` wirkt er ausserdem für **alle** Sender, nicht nur für den
Vollabgleich.

### Die Bedingung, und warum es sie braucht

```
Reload nur, wenn die Quittung eingetroffen ist — nicht nach einem Abbruch wegen
Zeitüberschreitung.
```

`guardrails.sh:171-177` hält heute ausdrücklich fest: „`var_send_buf()` bekommt
BEWUSST keinen Reload … der daraus folgende Watchdog-Reset hat die Uhr am
02.10.2026 nach sieben Sekunden wieder ins Leben gebracht (L25). Ein Reload an
dieser Stelle würde daraus wieder ein stilles Steckenbleiben machen."

**Dieser Einwand ist richtig — und er trifft nur den bedingungslosen Reload.**
Die Bedingung löst beides auf:

| Lage der Brücke | Heute | Nachher |
|---|---|---|
| antwortet zügig | kein Risiko | unverändert kein Risiko |
| antwortet, aber langsam (im Mittel über rund 90 ms je Kommando) | Reset mitten im Vollabgleich, obwohl nichts kaputt ist; der ESP behält einen halben Variablensatz (vgl. L42/A6) | Fortschritt wird belohnt, der Abgleich läuft zu Ende |
| antwortet nicht | Reset nach rund sieben Kommandos | **unverändert** Reset nach rund sieben Kommandos |

Der Reload belohnt Fortschritt, nicht Warten. Das ist der ganze Unterschied, und er
steht an genau einer Stelle im Code — AK8 verlangt, dass man ihn dort in einer
Bedingung ablesen kann.

Im verschachtelten Fall (`var_send_nested`, `vars.c:74-77`) wird **nicht** geladen:
Dort hat gar keine Wartezeit stattgefunden, und der äussere Aufruf erledigt es.

### Was das nicht heilt

- Den Hänger auf dem F411 — dazu sagt dieses Paket nichts.
- Die Dauer des Vollabgleichs. Die Uhr steht während des Sendens weiterhin still;
  der Reload hält sie am Leben, er beschleunigt sie nicht.
- Eine Blockade **ausserhalb** von `var_send_buf()`, etwa in der DMA-Warteschleife
  oder im EEPROM-Schreibpfad. Dort bleibt der Watchdog das Mittel, und das soll er.
- Den Fall, dass die Zeitgeber-ISR steht: Dann wächst `uptime` nicht, der
  3-Sekunden-Abbruch greift nie, und aus der Warteschleife wird wieder eine ohne
  Ende. Der Reload käme dann ebenfalls nie — **und das ist richtig so**, denn in
  diesem Fall soll der Watchdog auslösen. Es ist zugleich der Grund, warum die
  Diagnosezeile nicht an `uptime` hängen darf.

### Folgeänderung, die nicht vergessen werden darf

`guardrails.sh` führt `WD_EXPECTED=6` als Bestandswache mit einem Kommentar, der
genau diese Änderung ausschliesst. Beides muss mit — die Zahl auf 7, der Kommentar
auf die neue Begründung. Sonst liest der nächste Durchgang den alten Kommentar und
dreht die Änderung zurück. Das ist ein Lead-Task, kein Anhängsel.

## 6 — Kappung sichtbar machen (ESP)

Zweimal wird heute stillschweigend abgeschnitten:

- `ESP-uclock.ino:1148` — `cmd_buffer` fasst 127 Zeichen, alles darüber fällt
  zeichenweise weg, ohne Spur.
- `ESP-uclock.ino:90` — `strncpy` auf 120 Zeichen im Ringpuffer, ebenso.

Beide bekommen dieselbe Marke am Zeilenende (ein ASCII-Zeichen, kein Mehrbyte —
die `.ino` ist nicht UTF-8). Kosten: kein zusätzlicher Speicher, ein Vergleich je
Zeile. Nutzen: Eine gekappte Diagnosezeile sieht nicht mehr aus wie eine
vollständige. Bei rund 105 Zeichen der Warteschleifen-Zeile und 94 der
Diagnosezeile ist das keine theoretische Grenze.

## 7 — `DEBUG` einschaltbar machen (Build)

`option(WORDCLOCK_DEBUG "…" OFF)` in `CMakeLists.txt`, und bei `ON` wird `DEBUG` an
`target_compile_definitions()` durchgereicht — zusätzlich zu
`${TGT_COMPILE_DEFINITIONS}`, das die Zielargumente trägt und nicht umgewidmet wird.

Mehr nicht. Keine Auswahl unter den Aufrufstellen, keine neue Stufe, keine Änderung
an `log.h`.

**Zwei Dinge gehören in die Beschreibung des Schalters**, weil man sie dem Schalter
nicht ansieht:

1. Ein Diagnosebau flutet den Logring genauso wie die heutige `sk6812`-Flut — nur
   aus anderen Quellen. Er gehört an den Arbeitsplatz, nicht auf die Uhr im
   Dauerbetrieb.
2. Ein Diagnosebau schreibt Konfigurationswerte ins Logbuch, darunter den
   Wetter-Schlüssel im Klartext (`main.c:2199`). `/api/stm32_log` ist ohne Anmeldung
   aus dem ganzen LAN lesbar. Das ist für einen bewussten Diagnosebau vertretbar,
   für eine unbemerkte Vorgabe nicht — deshalb ist die Vorgabe `OFF`, und deshalb
   steht es im Klartext in der Beschreibung.

## Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `src/sk6812/sk6812.c` | zwei unbedingte `log_printf` entfernen, Refresh- und DMA-Wartezähler mit Zugriffsfunktionen | `stm-developer` |
| `src/sk6812/sk6812.h` | Deklaration der zwei Zugriffsfunktionen | `stm-developer` |
| `src/uart/uart-driver.h` | Verwurfszähler im `else`-Zweig, höchster Füllstand, zwei Zugriffsfunktionen je Instanz | `stm-developer` |
| `src/uart/uart.h` | Deklaration der zwei neuen Zugriffsfunktionen je Präfix | `stm-developer` |
| `src/main.c` | Zeitgeber-Zähler in der ISR; Diagnosezeile am Kopf des Hauptloops inkl. Folgenummer und selbstkalibrierendem Rückfall | `stm-developer` |
| `src/vars/vars.c` | Quittungs-Flag in `var_send_buf()`, `watchdog_reload()` nur bei Quittung | `stm-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Kappungsmarke an beiden Stellen | `esp-developer` |
| `CMakeLists.txt` | `option(WORDCLOCK_DEBUG … OFF)` und Durchreichen von `DEBUG` | Lead |
| `tools/guardrails.sh` | `WD_EXPECTED` auf 7, Kommentar zur neuen Begründung | Lead |
| `tools/measure-log.sh` | Auswertung nach Zeilenklassen (siehe Nachweis) | Lead |
| `tools/smoke-device.sh` | Diagnosezeile vorhanden, Folgenummer lückenlos | Lead |
| `knowledge/quick-reference.md` | Regelzeile 15 richtigstellen | `doc-writer` |
| `src/main.h`, `ESP8266/ESP-uclock/version.h` | Versionen | `release-engineer` |

`knowledge/quick-reference.md:15` empfiehlt heute, unbedingte Logzeilen im
Refresh-Pfad „auf `debug_log_printf` umzustellen". Das wäre hier die falsche
Antwort: In der Release-Firmware verschwindet die Zeile dann, im Diagnosebau flutet
sie weiter. Die Regel wird auf das ersetzt, was diese Spec vormacht — entfernen und
durch einen Zähler ersetzen, den die Diagnosezeile mitträgt.

## Nachweis — und wie ein Fehlschlag erkennbar bleibt

Alles rein lesend (DIR-008). Kein Endpunkt aus der Gefahrenliste in `CLAUDE.md`.

### Vorher (steht bereits)

`./tools/measure-log.sh 30` im Ruhezustand: **0,05 Zeilen/s, Reichweite 21 min.**
Fehlt noch: dieselbe Messung während eines vom Nutzer gestarteten Ticker-Overlays.
Die Last startet der Nutzer, nicht das Werkzeug — das Skript löst nichts aus.

### Nachher

1. `./tools/measure-log.sh 30` im Ruhezustand → AK3.
2. `./tools/measure-log.sh 15` während eines Ticker-Overlays → AK2. Kürzer messen
   als 30 s, weil das Skript bei vollständig umgelaufenem Ring nur noch eine
   Untergrenze ausweisen kann — und eine Untergrenze beantwortet AK2 nicht.
3. `curl -s http://$DEVICE_HOST/api/stm32_log` zweimal im Abstand von 60 s:
   Diagnosezeilen zählen, Folgenummern auf Lückenlosigkeit prüfen (AK5), Felder
   `rx=` und `d=` ablesen (AK4).
4. Dieselbe Abfrage während des Ticker-Overlays: `r=` muss deutlich steigen, `t=`
   und `u=` gleichmässig — die Gegenprobe, dass die Zähler das messen, was draufsteht.

### Die drei Arten, wie dieser Nachweis plausibel aussehen und trotzdem falsch sein kann

| Falle | Woran man sie erkennt |
|---|---|
| **„Es ist so schön ruhig geworden"** — in Wahrheit kommt vom STM gar nichts mehr an | Die Folgenummer der Diagnosezeile. Stille **mit** fortlaufender Nummer ist Ruhe, Stille **ohne** Zeile ist ein Ausfall. Ohne AK5 sehen beide gleich aus |
| **„Die Flut ist weg"** — in Wahrheit hat eine andere Quelle übernommen, nur langsamer | `measure-log.sh` wird um eine Auswertung nach Zeilenklassen erweitert: die drei häufigsten Zeilenanfänge mit Anzahl. Eine Reichweite von 70 s aus einer unerwarteten Quelle ist kein bestandenes AK2, sondern der nächste Befund |
| **„Null Verwürfe"** — in Wahrheit zählt niemand | Der höchste Füllstand aus AK4. Beide Felder auf 0 ist kein Bestehen |

Dazu zwei Prüfungen ohne Gerät: `./tools/guardrails.sh` für AK1 und AK11, und der
ELF-Vergleich für AK10 — ein Bau ohne Schalter muss dieselbe Firmware ergeben wie
vorher, sonst hat der Schalter mehr verändert als angekündigt.

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Die PWA bleibt parallel zur Legacy-Oberfläche; keine der
beiden wird berührt. Die Wetter-Endpunkte sind nicht im Umfang. Die
Restore-Bedingung um `pending_weather_ticker_restore` (`main.c:3944-3955`) wird nicht
angefasst — die Diagnosezeile wird **vor** diesem Block ausgegeben, am Kopf des
Loops, und liest nur Zähler. Die Auslieferung der App-Assets ist nicht berührt.

Zur Schicht: Das Problem entsteht im STM (zu viele Zeilen) und wirkt sich am ESP aus
(Ring voll). Behoben wird es **an der Quelle**. Den Ring zu vergrössern wäre genau
der Fehler, den die Checkliste benennt: ein Problem des Erzeugers beim Verbraucher
umgehen. Es wäre zudem wirkungslos gegen die 250 ms/s Hauptloop-Blockade, die
dieselbe Ursache hat.

**Scalable systems** — **Keine neuen STM-Kommandos.** Null, in beide Richtungen; das
Paket fügt der Brücke kein Protokoll hinzu, es nimmt ihr Last ab.

Byte pro Minute auf der UART, aus den gemessenen Raten hergeleitet:

| | heute | nachher |
|---|---|---|
| Ruhe | rund 0,2 kB/min | rund 0,9 kB/min |
| Ticker-Overlay | rund **180 kB/min** | erwartet **unter 1 kB/min** |

Im Ruhezustand steigt die Last also um gut das Vierfache — auf einen Wert, der rund
zweihundertmal unter der heutigen Lastspitze liegt. Das ist der Preis für eine
Aufzeichnung, die auch dann etwas sagt, wenn nichts passiert, und AK7 verlangt, dass
beide Zahlen genannt werden statt nur die günstige.

Watchdog: Jeder Pfad bleibt unter 20 s. `var_send_all_variables()` wird bei
antwortender Brücke bedient und läuft bei toter Brücke weiterhin in den Reset
(AK8). Neue blockierende Pfade entstehen nicht. Hauptloop-Blockade: unter Last rund
250 ms/s **weniger**, im Ruhezustand 8 ms je 10 s mehr. Polling der PWA: unverändert.
Hartkodierte Grenzen: Die Ringgeometrie bleibt; die neuen Zähler sättigen statt
umzulaufen, wo eine kleine Zahl in die Irre führen würde.

**Secure by design** — Kein neuer Endpunkt, kein `innerHTML`, keine Fremddaten. Die
Diagnosezeile enthält **ausschliesslich Zähler** — keine SSID, keinen Schlüssel,
keine Adresse, keinen Ortsnamen. Das ist eine Festlegung, keine Beobachtung: Wer sie
später um einen Konfigurationswert erweitert, veröffentlicht ihn über
`/api/stm32_log` im ganzen LAN, ohne Anmeldung.

Der `DEBUG`-Schalter macht Konfigurationswerte sichtbar, darunter den
Wetter-Schlüssel im Klartext (`main.c:2199`). Deshalb Vorgabe `OFF` und ein
ausdrücklicher Hinweis in der Beschreibung des Schalters. In der ausgelieferten
Firmware ändert sich nichts.

**Stable & reliable** — Dieses Paket beantwortet den Checklistenpunkt „stille
Verwerfungen ohne Zähler — etwa ein voller Puffer ohne `else`-Zweig" wörtlich: Der
gemeinte `else`-Zweig ist `uart-driver.h:698`, und er bekommt seinen Zähler.

Weiter: Die Folgenummer macht den Verlust der Meldung selbst sichtbar — eine
Erfolgsmeldung, die vom tatsächlichen Ergebnis abhängt. Leere `catch`-Blöcke gibt es
hier nicht (kein JavaScript). Gesetzte Flags: Die Diagnoseausgabe setzt keines; das
Quittungs-Flag in `var_send_buf()` ist lokal und lebt nur innerhalb des Aufrufs,
`var_send_busy` und `var_send_nested` werden wie bisher auf jedem Pfad
zurückgesetzt. Zustand nach Abbruch: unverändert, weil die Kommandofolge nicht
angefasst wird.

Ein Punkt bleibt bewusst offen und wird hier benannt statt verschwiegen: Steht die
Zeitgeber-ISR, greift der 3-Sekunden-Abbruch in `var_send_buf()` nicht mehr, weil er
mit `uptime` rechnet. Dieses Paket ändert das nicht — es macht den Fall erstmals
**erkennbar** (Zeile 3 der Fallunterscheidung oben). Ihn zu beheben wäre ein
Eingriff in die Zeitbasis und damit in den Gegenstand der Messung.

## Verworfene Alternativen

| Verworfen | Warum |
|---|---|
| Logring auf 128 Zeilen vergrössern | 7'744 Byte von 12'400 verbleibenden. Am Linker-Protokoll belegt |
| Ringzeilen auf 100 Zeichen kürzen | Kappt die Warteschleifen-Zeile aus `sk6812.c:463`, also das M3-Instrument |
| Ratenbegrenzung im Logpfad des STM | Verdeckt, dass die Quelle falsch ist, kostet Vergleichslogik im heissen Pfad, und „ist das dieselbe Zeile" ist nicht billig zu beantworten. Die beiden Zeilen sind Durchlaufmeldungen — die richtige Rate für sie ist null |
| `sk6812`-Zeilen auf `debug_log_printf` umstellen, wie Massnahme 13 vorschlägt | Dann fehlen sie im Normalbau **und** fluten im Diagnosebau. Der Zähler wirkt in beiden |
| Logstufe zur Laufzeit umschaltbar | Braucht einen Steuerweg über drei Besitzer und einen neuen schreibenden Endpunkt. Vor allem: Eine Stufe, die unter LED-Last eingeschaltet werden kann, ist eine geladene Waffe auf die Brücke. **Vorbedingung, unter der es wieder sinnvoll wird:** wenn nach diesem Paket gemessen ist, wie viele Byte pro Minute die Brücke verträgt, ohne zu verwerfen — diese Zahl liefert erst der Zähler aus Punkt 4 |
| Verwurfszähler als numerische Variable | Vier Besitzer, Protokolländerung, und der Melder würde auf der gestörten Brücke auf eine Quittung warten |
| `watchdog_reload()` bedingungslos in `var_send_buf()` | Macht aus dem Reset bei toter Brücke wieder ein stilles Steckenbleiben. Genau der Einwand aus `guardrails.sh:171-177` und L25 |
| `watchdog_reload()` in die Schleife von `var_send_all_variables()` | Zu grob: Die Gruppenfunktionen senden bis zu 32 Kommandos am Stück, also bis zu 96 s zwischen zwei Bedienungen |
| Diagnosezeile im Sekundentakt über `uptime` | Hängt an der ISR, die der Verdächtige ist. Sie würde ausgerechnet im interessanten Fall schweigen |
| Diagnosezeile mit fester Durchlaufzahl statt selbstkalibrierendem Budget | Die Durchlaufrate des Hauptloops ist unbekannt und lastabhängig. Eine feste Zahl flutet oder schweigt, und sie veraltet still |

## Versionsfolgen

- [x] STM `src/main.h` anheben
- [x] ESP `version.h` anheben
- [ ] App `APP_VERSION` — **nein**, die PWA wird nicht geändert
- [ ] `CACHE_NAME` in `sw.js` — **nein**, gehört zu `APP_VERSION`

Nur der `release-engineer` führt das aus (R4).
