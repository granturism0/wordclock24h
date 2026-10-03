# Anforderungen — Die Uhr beobachtbar machen

**Status:** Entwurf
**Auslöser:** ToDo-Punkt **C7** in `BEFUNDE.md`, getragen von **L75**,
`REVIEW.md` Massnahme 13 (= **C5**), dem stillen Verwurf im UART-Empfangsring aus dem
F1-Review und **L85**. Dazu der Mitschnitt `haenger-2026-09-30.md`.

## Problem

Das Projekt hat eine offene Frage, die es mit den heutigen Mitteln nicht beantworten
kann. Der Mitschnitt vom 30.09.2026 hat die Blockade-Spur ausgeschlossen: kein
Watchdog-Reset über 18 Minuten, ein Ticker-Kommando wurde mitten im Hänger noch
vollständig ausgeführt — der Hauptloop lief also. Ausgefallen war nur der
**zeitgesteuerte Zweig**: `show_time`, `read rtc`, Temperatur, Refresh. Die Frage
lautet: Was macht diesen Zweig unerreichbar, während der Rest weiterläuft?

Vier Befunde stehen der Antwort im Weg. Sie bedingen sich gegenseitig, deshalb eine
einzige Spezifikation.

### b — Der Logring hält unter LED-Last 1,2 Sekunden (L75, ✔ am Gerät gemessen)

Der Ring auf dem ESP fasst 64 Zeilen à 120 Zeichen (`ESP-uclock.ino:72-78`).

| Zustand | Rate | Reichweite |
|---|---|---|
| Ruhe, Ausgangsmessung mit `tools/measure-log.sh` | 0,05 Zeilen/s | **21 min** |
| Ticker-Overlay (L75) | rund 50 Zeilen/s | **1,2 s** |

Die PWA fragt `/api/stm32_log` alle **2500 ms** ab (`app.js:3564`). Unter LED-Last
läuft der Ring zwischen zwei Abfragen **zweimal** um. Das Logbuch der Oberfläche
kann einen Vorfall unter Last also nicht nur zufällig verpassen — es **muss** ihn
verpassen. Genau in den Situationen, die für die Hänger-Untersuchung interessant
sind, gibt es keine lückenlose Aufzeichnung.

Der Faktor zwischen beiden Zeilen ist **tausendfach**. Das ist kein Puffer, der
knapp bemessen ist, sondern eine Aufzeichnung, die sich unter Last selbst löscht.

### c — `sk6812.c` erzeugt diese Flut (Massnahme 13, ✔ verifiziert)

Drei unbedingte `log_printf` im Refresh-Pfad:

| Stelle | Wann | Bewertung |
|---|---|---|
| `sk6812.c:463` | in der DMA-Warteschleife, höchstens **eine je Sekunde**, und nur wenn der DMA überhaupt wartet | **bleibt.** Das ist das Widerlegungsinstrument für einen DMA-Stillstand (`BEFUNDE.md`, Gerätetest M3) |
| `sk6812.c:476` | **jeder** Refresh, ohne Bedingung | Flut |
| `sk6812.c:495` | **jeder** Refresh, ohne Bedingung | Flut |

Die beiden letzten sind zusammen rund 103 Zeichen; mit `LOG `-Präfix und Zeilenende
rund 115 Byte je Refresh. Die gemessenen 50 Zeilen/s entsprechen rund 25 Refreshes/s
— das deckt sich mit einem Ticker bei Vorgabe-Verzögerung (rund 21 Hz). Rate und
Quelle passen also zusammen; L75 und Massnahme 13 beschreiben **dasselbe** Problem
von zwei Seiten.

Jede Logzeile geht über `log_vprintf` (`log.c:27-34`) **zusätzlich** per
`esp8266_send_log_line()` auf die UART zum ESP und endet dort mit blockierendem
`esp8266_uart_flush()` (`esp8266.c:575`). Bei 115200 Baud (86,8 µs je Byte) sind
115 Byte rund **10 ms Blockade je Refresh**, bei 25 Refreshes/s rund **250 ms pro
Sekunde**. Ein Viertel der Zeit schiebt der Hauptloop Zeichen hinaus, die niemand
lesen kann, weil sie 1,2 s später überschrieben sind.

Es ist das einzige verbliebene Hoch-Finding der Guardrails (`guardrails.sh:162-163`).

### d — Der UART-Empfangsring verwirft still (✔ verifiziert)

`src/uart/uart-driver.h:698-708`: `if (uart_rxsize < UART_RXBUFLEN)` hat **kein
`else`**. Kein Zähler, kein Flag, kein Log. Ringgrösse 256 Byte
(`esp8266-uart.c:69`), keine Flusskontrolle.

**Das hängt mit b und c unmittelbar zusammen, und das ist die Klammer des Pakets:**
`esp8266_get_message()` beginnt mit `log_flush()` und `esp8266_uart_flush()`
(`esp8266.c:202-203`) — der STM liest **kein einziges Zeichen** vom ESP, solange
Logausgabe aussteht. 256 Byte Empfangsring entsprechen bei 115200 Baud **22,2 ms**
Dauerempfang.

**Was hier belegt ist und was nicht:** Belegt ist der Mechanismus — fehlender
`else`-Zweig, Ringgrösse, die Reihenfolge in `esp8266_get_message()`. **Nicht
belegt ist, dass jemals ein Zeichen verlorenging.** Genau deshalb braucht es den
Zähler: Die Frage ist heute nicht mit „nein" beantwortbar, sondern gar nicht.

### e — `var_send_all_variables()` ohne einen einzigen `watchdog_reload()` (L85)

`vars.c:1026-1093`. Die Zahl der Kommandos habe ich nachgerechnet statt L85
abzuschreiben:

| Gruppe | Kommandos |
|---|---|
| Einzelwerte (`vars.c:1028-1066`) | 39 |
| `var_send_tm` inkl. Uptime | 3 |
| drei Farbvariablen | 3 |
| Display-Animationen (`ANIMATION_MODES` 13 × 4) | 52 |
| Farbanimationen (3 × 4) | 12 |
| Ambilight-Modi (5 × 4) | 20 |
| Overlays (1 + 8 je Overlay) | 1 + 8 × n |
| Nacht-, Ambilight-Nacht-, Alarmzeiten (3 × 8) | 24 |
| Dimmkurven (2 × 16, je Element ein Kommando) | 32 |
| DFPlayer | 8 |

**194 Kommandos ohne Overlays, rund 220 bei üblicher Konfiguration, über 450 bei
den möglichen 32 Overlays.** Jedes geht durch `var_send_buf()` (`vars.c:52-100`) und
wartet dort bis zu `VAR_SEND_TIMEOUT_SEC` = 3 s auf die Quittung. Watchdog-Timeout:
20 s (`main.c:382`).

Gerufen wird die Funktion aus dem `ESP8266_IPADDRESS`-Zweig (`main.c:2887`),
unmittelbar vor `display_set_ticker("IP …")` — also genau dem Startabschnitt, bei
dem der F411 laut `CLAUDE.md` „teils exakt bei Anzeige von IP" hängt.

**Was hier belegt ist und was nicht:** Belegt ist die Struktur — ein Pfad aus 194 bis
450 blockierenden Kommandos ohne eine einzige Watchdog-Bedienung, an der
Fundstelle, die zur Beobachtung passt. **Nicht belegt ist ein eingetretener Fall.**
Der Mitschnitt zeigt gerade *keinen* Watchdog-Reset. Daraus folgt **keine
Ursachenbehauptung** (DIR-003). Es ist eine Absicherung vor der Messung, nicht die
Behebung eines gemessenen Fehlers — und sie gehört dazu, weil man nicht auf einem
Pfad misst, der im Fehlerfall selbst zurücksetzt.

**Eine Kopplung, die dabei zu beachten ist:** Der 3-Sekunden-Abbruch in
`var_send_buf()` rechnet mit `uptime`. `uptime` wird in derselben Zeitgeber-ISR
hochgezählt (`main.c:878`), die auch `show_time_flag`, `ds3231_flag` und die
Temperatur-Flags setzt — also in genau dem Zweig, der im Hänger ausgefallen war.
Steht diese ISR, wird aus dem Abbruch wieder eine Schleife ohne Ende, und der
Befund, den dieser Abbruch geschlossen hat, ist zurück. **Jedes Messmittel dieses
Pakets darf deshalb nicht allein an `uptime` hängen.**

### Warum einzeln behoben nichts bringt

Die Flut abzustellen, ohne einen Zähler für die Verwürfe, zeigt nicht, ob die
Entlastung an der Brücke ankommt. Den Zähler einzubauen, ohne den Ring zu entlasten,
erzeugt eine Zahl, die 1,2 s später überschrieben ist. Beides zu tun, ohne den
`var_send`-Pfad abzusichern, heisst auf einem Pfad zu messen, der sich im Fehlerfall
selbst abschneidet.

### Nebenbefund, der hier nur mitgenommen wird

**`DEBUG` lässt sich nicht von aussen einschalten.** `TGT_COMPILE_DEFINITIONS`
(`CMakeLists.txt:69`) ist kein freier Schalter, sondern das von
`cmake_parse_arguments(TGT …)` (`:45-49`) geparste `COMPILE_DEFINITIONS`-Argument des
jeweiligen Zielaufrufs; eine Übergabe von aussen wird beim Parsen überschrieben. Ein
Bau mit `-DTGT_COMPILE_DEFINITIONS=DEBUG` erzeugt ein **byteidentisches** Fabrikat.

Dass die `debug_log_*`-Aufrufe in der Release-Firmware zu nichts übersetzen, ist
**kein Befund, sondern der Zweck des Makros**. Fehlend ist nur der Schalter. Das ist
eine Unbequemlichkeit am Build, kein Laufzeitproblem — deshalb steht es hier als
**ein Task** und nicht als Kapitel, und es gibt in dieser Spec keine Abwägung
darüber, welche der `debug_log_*`-Stellen „sichtbar werden sollen". Wer den Schalter
umlegt, will sie alle sehen; das ist dann seine Entscheidung für einen Diagnosebau,
der nicht auf die Uhr im Dauerbetrieb gehört.

### Einwand: Dieses Paket ist nicht die Folge von AK7, sondern dessen Voraussetzung

`BEFUNDE.md` führt **C5** als „zurückgestellt, mit Grund: würde die
`icon_freeze`-Messung verfälschen. Erst nach AK7". Diese Reihenfolge hält der
Prüfung nicht stand, und das gehört vor der Freigabe entschieden:

- **AK7 ist mit dem heutigen Ring nicht beweisbar.** Die gesuchte Zeile
  `icon_freeze: ENTER do_icon=1 power=0` entsteht während der Icon-Anzeige, also
  unter LED-Last. Dort hält der Ring 1,2 s, die Oberfläche fragt alle 2,5 s. Die
  Zeile ist überschrieben, bevor sie abgeholt werden kann. „In 24,5 Stunden trat die
  Bedingung nie ein" ist mit dieser Mechanik nicht zu unterscheiden von „sie trat ein
  und ging verloren".
- **Die `icon_freeze`-Zeile wird von diesem Paket nicht angefasst.** Sie feuert nur
  bei Zustandswechsel (`main.c:3362-3370`) und ist genau das Muster, das diese Spec
  zur Regel macht.

**Vorschlag: C5 kommt vor AK7, und AK7 wird danach wiederholt.** Entscheidet der
Nutzer anders, bleibt die Spec gültig — dann rutscht Task 2 ans Ende und AK2 wird
erst dort messbar.

## Ziel

Die Uhr bekommt eine Beobachtung, die **im Normalbetrieb schweigt und im Fehlerfall
spricht**: eine Aufzeichnung, die unter LED-Last Minuten statt 1,2 Sekunden
zurückreicht, einen Zähler für die bisher stillen Verwürfe der Brücke, und eine
Diagnosezeile, deren Ausbleiben selbst ein Befund ist. Das Paket schafft Messmittel.
Es löst den Hänger nicht.

## Akzeptanzkriterien

- [ ] **AK1 — Kein unbedingtes `log_printf` mehr im Refresh-Pfad.**
      `./tools/guardrails.sh` meldet in S7 „sk6812.c: kein unbedingtes log_printf".
      `sk6812.c:463` (Warteschleife) **bleibt** wirksam — fällt sie weg, verliert
      Gerätetest M3 sein Instrument, und das wäre ein Rückschritt, kein Fortschritt.
- [ ] **AK2 — Reichweite unter Last, gemessen.** `./tools/measure-log.sh 15`
      **während eines vom Nutzer gestarteten Ticker-Overlays** meldet eine Reichweite
      von **mindestens 60 s** (heute: 1,2 s). Erwartet werden mehrere Minuten; die
      Schwelle liegt bewusst tief, damit ein Nichtbestehen eindeutig ist und nicht
      vom Messzufall abhängt. 60 s ist das 24-Fache des PWA-Abfrageintervalls von
      2,5 s — die Grenze, ab der die Oberfläche den Ring lückenlos abholen kann.
- [ ] **AK3 — Reichweite im Ruhezustand bleibt brauchbar.** Dieselbe Messung ohne
      Last meldet **mindestens 5 Minuten**. Ausgangswert: 21 min bei 0,05 Zeilen/s.
      Die Diagnosezeile aus AK5 senkt diesen Wert — der Umsetzungsbericht nennt den
      neuen Wert und die Taktrate, aus der er folgt.
- [ ] **AK4 — Der Verwurfszähler sagt auch bei 0 etwas.** Die Diagnosezeile nennt
      **zwei** Werte: die Zahl der verworfenen Empfangszeichen **und** den höchsten je
      erreichten Füllstand des Empfangsrings. Steht der erste auf 0 und der zweite
      über 0 und ändert sich mit der Last, ist belegt: gemessen wurde, verloren ging
      nichts. **Beide auf 0 gilt als nicht bestandene Messung** — dann ist offen, ob
      der Zähler funktioniert.
- [ ] **AK5 — Die Diagnosezeile trägt eine fortlaufende Nummer.** Zwei aufeinander
      folgende Diagnosezeilen im Ring unterscheiden sich um genau 1. Ohne diese
      Nummer ist eine Lücke nicht von Ruhe zu unterscheiden, und „keine Meldung" wäre
      wieder kein Ergebnis.
- [ ] **AK6 — Die Diagnosezeile hängt nicht allein an der verdächtigen Zeitbasis.**
      Fällt die Zeitgeber-ISR aus, erscheint sie weiterhin, getaktet über die Zahl der
      Hauptloop-Durchläufe. Ohne Gerät prüfbar: In `design.md` und im Code steht, über
      welchen zweiten Weg sie ausgelöst wird und warum dieser Weg nicht fluten kann.
- [ ] **AK7 — Die UART-Last sinkt unter Last und steigt im Leerlauf höchstens
      geringfügig.** Der Umsetzungsbericht nennt beide Zahlen in Byte pro Minute,
      vorher und nachher, hergeleitet aus der gemessenen Zeilenrate. Ein Paket, das
      die Brücke entlasten soll und sie im Ruhezustand spürbar mehr belastet, hat sein
      Ziel verfehlt.
- [ ] **AK8 — `var_send_all_variables()` überlebt eine langsame Brücke, eine tote
      Brücke setzt weiterhin zurück.** Quittiert der ESP, wird der Watchdog bedient.
      Quittiert er nicht, läuft der Pfad weiterhin in den Watchdog-Reset — dieser
      Reset hat die Uhr am 02.10.2026 nach sieben Sekunden zurückgebracht (L25) und
      wird nicht wegoptimiert. Beide Richtungen sind im Code an **einer** Bedingung
      ablesbar.
- [ ] **AK9 — Gekappte Logzeilen sind als gekappt erkennbar.** Eine Zeile, die der
      ESP bei 120 Zeichen abschneidet, trägt eine Marke. Ohne sie sieht eine
      abgeschnittene Diagnosezeile aus wie eine vollständige — und die
      Warteschleifen-Zeile aus `sk6812.c:463` liegt mit rund 105 Zeichen dicht an
      dieser Grenze.
- [ ] **AK10 — `DEBUG` ist von aussen einschaltbar, Vorgabe aus.** Der Bau ohne
      Schalter erzeugt dieselbe Firmware wie heute (am ELF oder an der `.hex`
      gezeigt, nicht behauptet); mit Schalter sind die `debug_log_*`-Texte im ELF
      nachweisbar.
- [ ] **AK11 — `./tools/guardrails.sh` läuft mit Exit 0 durch**, nach dem letzten
      Task auch `--full`.

## Nicht Teil dieser Änderung

- **Keine Aussage zur Ursache des F411-Hängers.** Wer am Ende sagt „jetzt wissen wir
  warum", hat die Spec missverstanden.
- **Kein pauschaler DMA-Fix, kein Recovery-Mechanismus** (`CLAUDE.md`). Die
  Diagnosezeile berichtet, sie greift nicht ein. Kein Zurücksetzen, kein Neustart,
  kein Nachladen, keine Selbstheilung.
- **Keine Auswahl unter den `debug_log_*`-Stellen.** Der Schalter wird gebaut, die
  Stellen bleiben, wie sie sind.
- **Keine Schnittstelle zum Umschalten der Logstufe zur Laufzeit.** Geprüft,
  zurückgestellt, Begründung und Vorbedingung in `design.md`.
- **Keine Vergrösserung des Logrings auf dem ESP.** Mit Zahlen begründet in
  `design.md`; die 64 Zeilen bleiben.
- **Kein Umbau von `var_send_all_variables()`** — keine Umstellung auf getaktetes
  Senden, keine Änderung der Kommandofolge, keine Kürzung der Liste. Nur der
  Watchdog wird bedient.
- **Keine Änderung an `app.js` oder der Oberfläche.** Der Logbuch-Dialog zeigt
  weiterhin, was der Ring hergibt.
- **Keine Behebung von C2** (HTTP-Debugzeilen für `/api/` unterdrücken). Derselbe
  Kanal, aber eine eigene Entscheidung mit eigener Messung — und diese Spec braucht
  sie als **unveränderte** Vergleichsgrundlage für die Vorher-/Nachher-Messung.
- **Keine Flusskontrolle auf der UART.** Der Zähler misst den Verlust; ihn zu
  verhindern ist ein anderes, grösseres Thema.
- **Keine Änderung an der Zeitgeber-ISR**, ausser dem einen Zähler aus `design.md`.
  Sie ist der Gegenstand der Beobachtung. Wer sie umbaut, misst hinterher etwas
  anderes.

## Betroffene Laufzeiten

- [x] STM32 (`src/**`) — Neu-Flashen nötig
- [x] ESP8266 (`ESP8266/ESP-uclock/*.ino`) — Neu-Flashen nötig
- [ ] PWA (`data/app/**`) — nicht berührt
- [x] Build/Release — `CMakeLists.txt`, `tools/**`, Versionen
