# Design — Die Kommandobrücke reparieren

## Leitsatz

**Was lange dauert, gehört in den Hauptloop zerlegt — nicht mit einem Reload am Leben
gehalten.** Der Reload ist das Mittel für eine Blockade, die man nicht zerlegen kann.
Der Vollabgleich ist zerlegbar, also wird er zerlegt. Was danach an Reload
übrigbleibt, bekommt eine Obergrenze.

Das Muster steht bereits im Code, mit derselben Begründung: Der IR-Abzug sendet
**ein** Kommando je Hauptloop-Durchlauf, und `main.c:1632-1635` hält fest, warum —
„Ein Kommando je Hauptloop-Durchlauf löst das durch die Struktur statt durch
Sorgfalt: Zwischen zwei I-Kommandos liegt garantiert der reguläre
`watchdog_reload()` am Kopf des Loops, ohne eine einzige neue Aufrufstelle." Dieses
Design überträgt das auf den Vollabgleich. Es erfindet nichts, es zieht die
vorhandene Lösung auf den grösseren Fall.

---

## 1 — Die Quittung ist der Punkt, nicht „irgendwas mit OK" (P2)

### Der Befund in einem Satz

Der ESP quittiert ein `var`-Kommando mit `.` (`ESP-uclock.ino:444`). Er sendet
daneben drei unaufgeforderte `OK`-Zeilen, und **keine davon ist je eine Quittung**:

| Zeile | Fundstelle | Anlass |
|---|---|---|
| `OK cap` | `wifi.cpp:68` | Bootlauf |
| `OK ap` | `wifi.cpp:144` | Wechsel in den AP-Modus |
| `OK time` | `ntp.cpp:101` | nach dem NTP-Abgleich |

`statusmsg()` (`base.cpp:264`) kennt keinen weiteren `OK`-Aufruf. Die Behauptung
„keine `OK`-Zeile ist eine Quittung" ist damit nicht plausibel, sondern an allen
Aufrufstellen abgezählt.

### Die Änderung

`esp8266.c` bekommt einen **zweiten Rückgabewert**. Der Punkt bleibt die Quittung,
die `OK`-Zeile wird zu einer Statusmeldung:

| Zeile | heute | nachher |
|---|---|---|
| `.` (Zeile 225-227) | `ESP8266_OK` | `ESP8266_OK` — unverändert |
| `OK …` (Zeile 299-301) | `ESP8266_OK` | neuer Code, etwa `ESP8266_STATUS` |

Folgeänderungen, beide zwingend:

- `main.c:3020` setzt `var_send_busy = 0` heute auf **jedes** `ESP8266_OK`. Das
  bleibt beim Punkt. Der neue Statuscode bekommt einen eigenen `case`, der nichts
  tut ausser das `switch` sauber zu schliessen — die Zeile wird ohnehin schon in
  `esp8266.c:294-297` geloggt, es geht also nichts verloren.
- `vars.c:96` bleibt unverändert, weil es weiterhin auf `ESP8266_OK` wartet — das
  jetzt aber nur noch der Punkt erzeugt.

### Warum das **kein** ESP-Flash ist

Der ESP sendet den Punkt bereits heute, unbedingt, nach jedem `var`-Kommando. Die
Änderung ist reine STM-Seite. Wer hier eine Sequenznummer ins Protokoll einbauen
wollte, bräuchte zwei Besitzer und einen ESP-Flash — und löste ein Problem, das die
Punktregel bereits löst.

### Woran ein Fehlschlag erkennbar wird

Wenn die Punkt-Quittung aus irgendeinem Grund nicht ankommt — etwa weil sie im
Empfangsring verdrängt wurde —, läuft **jedes** `var`-Kommando in seinen
3-Sekunden-Timeout. Das ist an genau einer Zahl ablesbar: Der Timeout-Zähler aus
Abschnitt 3 müsste im Ruhebetrieb bei 0 bleiben. Tut er das nicht, ist die Änderung
falsch und nicht „fast richtig" (AK2).

---

## 2 — Der Reload bekommt ein Zeitbudget, keinen Zähler (P1)

### Warum Zeit und nicht Anzahl

Der Auftrag lässt die Wahl zwischen Gesamtzeit und Zahl der Reloads. Die Zahl sagt
nichts über das, was weh tut. 194 Kommandos mit je 2 ms Antwortzeit sind harmlos,
sieben mit je 3 s sind es nicht. **Geschützt werden soll die Uhr vor Stillstand,
nicht die Brücke vor Kommandos.** Also ein Zeitbudget.

### Wie es zugeschnitten ist

Das Budget gilt **je Hauptloop-Durchlauf**, nicht je Vollabgleich. Das ist der
entscheidende Zuschnitt, und er hat drei Gründe:

1. Er wirkt für **alle** Sender, nicht nur für den Vollabgleich. Auch ein
   Gruppensetzer aus einem RPC-Zweig kann dutzende Kommandos am Stück senden.
2. Er hat einen natürlichen Nullpunkt: den Kopf des Hauptloops, wo ohnehin
   `watchdog_reload()` steht (`main.c:3373`). Dort wird das Budget zurückgesetzt —
   **ohne eine neue Aufrufstelle für `watchdog_reload()`**, der Bestand bleibt bei 7
   (AK3).
3. Er formuliert die Zusicherung, die man prüfen will: *Zwischen zwei Durchläufen des
   Hauptloops vergehen höchstens Budget + Watchdog-Fenster.* Genau das misst das `Δu`
   aus Abschnitt 6.

### Die Mechanik

```
Kopf des Hauptloops:   watchdog_reload ();
                       var_send_reload_deadline_valid = 0;     // Budget neu

var_send_buf(), nach eingetroffener Quittung:
                       wenn kein Nullpunkt gesetzt: jetzt setzen
                       nur reloaden, solange uptime - Nullpunkt < VAR_SEND_RELOAD_BUDGET_SEC
```

Der Nullpunkt wird beim **ersten** wartenden Aufruf nach dem Loopkopf gesetzt, nicht
beim Loopkopf selbst. Sonst zählte das Budget auch die Zeit mit, die der Loop für
Anzeige und RTC braucht — und es wäre schon aufgebraucht, bevor das erste Kommando
rausgeht.

### Der Wert, und was er kostet

**`VAR_SEND_RELOAD_BUDGET_SEC = 30`.**

| Lage der Brücke | heute | nachher |
|---|---|---|
| antwortet zügig (Millisekunden) | kein Reload nötig | unverändert |
| antwortet langsam, aber sie antwortet (bis rund 150 ms je Kommando) | läuft durch | läuft durch — das Budget reicht für einen Burst von 194 Kommandos |
| antwortet sporadisch (Sekunden je Kommando) | **bis zu 600 s Stillstand** | **höchstens 30 s**, dann greift der Watchdog wie vor `9eb6dd9` |
| antwortet nicht | Reset nach rund sieben Kommandos | **unverändert** Reset nach rund sieben Kommandos |

Schlechtester Fall nach der Änderung: 30 s Budget plus bis zu 20 s Watchdog-Fenster =
**50 s Stillstand**. Vor `9eb6dd9` waren es 20 s, heute bis zu 600 s. Die 30 s sind
also bewusst eine Verschlechterung gegenüber dem Zustand von vorgestern und eine
Verbesserung um den Faktor zwanzig gegenüber heute — der Preis dafür, dass ein
langsamer, aber lebendiger ESP seinen Variablensatz vollständig bekommt. **Nach
Runde 2 ist der Wert fast gegenstandslos**, weil der Vollabgleich dann gar nicht mehr
am Stück sendet; er bleibt als Netz für die Gruppensetzer.

Die Konstante steht neben `VAR_SEND_TIMEOUT_SEC` in `vars.c`, mit dieser Tabelle als
Kommentar. Wer sie ändert, soll sehen, wogegen er tauscht.

---

## 3 — Zwei Felder, die sagen, welcher Weg lief (P4)

### Das Format

Die Diagnosezeile aus dem Beobachtbarkeits-Paket bekommt **ein** Feld mit zwei
Zahlen:

```
diag <seq> l=<loop> t=<tick> u=<uptime> r=<refresh> w=<dmawait> rx=<max>/<size> d=<drops> o=<ore> v=<timeouts>/<verschachtelt>
```

### Die Platzrechnung, nachgerechnet statt übernommen

| Teil | Zeichen |
|---|---|
| Bestand (`main.c:3440`), maximale Feldbreiten | **102** |
| ` v=` + 5 + `/` + 5 | **14** |
| **neu** | **116** |
| Grenze (120 des ESP-Rings minus Kappungsmarke) | 119 |
| **Rest** | **3** |

Die 102 des Bestands bestätigen sich beim Nachzählen nur, weil `rx=%u/%u` zwei
dreistellige Werte trägt (`UART_RXBUFLEN` ist 256, `esp8266-uart.c:69`), nicht zwei
fünfstellige. Wer das Format anfasst, rechnet neu.

### Was die beiden Zahlen zählen

| Zahl | Typ | Erhöht bei |
|---|---|---|
| `<timeouts>` | `uint16_t`, sättigend | jedem Verlassen der Warteschleife über den Zeitzweig in `var_send_buf()` |
| `<verschachtelt>` | `uint16_t`, sättigend | jedem Eintritt in `var_send_buf()`, der bei `vars.c:75` wegen `var_send_nested` sofort zurückkehrt |

Sättigend statt umlaufend, aus demselben Grund wie bei den bestehenden Zählern: Eine
umlaufende Zahl liest sich als kleine Zahl und lügt dabei. Beide sind seit dem
STM-Start kumulativ; nach einem Reset stehen sie auf 0, und dass ein Reset war, zeigt
`seq`, das wieder bei 1 beginnt.

### Die Deutungstabelle — das ist der eigentliche Inhalt von A18

Nach **einem** ESP-Neustart:

| `<timeouts>` | `<verschachtelt>` | Abzug beschädigt? | Folgerung |
|---|---|---|---|
| klein | springt um rund **190** | ja | **Weg (B)**, der Massenverlust. `var_send_all_variables()` lief verschachtelt und quittungsfrei |
| steigt deutlich | klein | ja | **Weg (A)**, einzelne Kommandos sind in den Timeout gelaufen |
| steigt | springt | ja | beide, nacheinander |
| klein | klein | ja | **keiner von beiden** — dann ist die Ursache nicht in `var_send_buf()`, und das ist der wertvollste Ausgang dieser Messung, weil er den gesamten Abschnitt 4 in Frage stellt |
| klein | klein | nein | Der Abgleich lief sauber durch. Ein Durchgang ohne Vorfall belegt nichts — deshalb dreimal (AK12) |

Die letzte Zeile ist der Grund, warum diese Messung überhaupt vor dem Fix steht: Sie
kann das geplante Vorgehen widerlegen, und sie kostet dafür zwei Zähler.

### Was nach dem Fix aus den Feldern wird

`<verschachtelt>` wird nach Abschnitt 4 **klein, aber nicht null** — Einzelkommandos
können weiterhin verschachtelt auftreten, der Vollabgleich nicht mehr. Daraus wird
eine **Bestandswache**: Springt die Zahl je wieder um ~190, ist der Vollabgleich
zurück im Burst-Modus. Das Feld bleibt also nicht als Ballast stehen, es wechselt die
Aufgabe.

### Wenn später mehr Platz gebraucht wird — der Preis

Nach diesem Paket sind 3 Zeichen frei; das ist kein Feld mehr. Wer eines braucht,
weitet die Brücke:

| Schritt | Kosten |
|---|---|
| Ringzeile des ESP von 120 auf 136, `CMD_BUFFER_SIZE` 128 → 144 | 64 × 16 = **1'024 Byte DRAM** von 12'400 freien, also rund **8 %** |

Das ist eine eigene Entscheidung mit einem eigenen Besitzer (`esp-developer`) und
einem ESP-Flash. Sie gehört **nicht** in dieses Paket — sie steht hier nur, damit
„kein Platz mehr" eine Zahl hat statt eine Ausrede zu sein. Nebeneffekt: Derselbe
Schritt entschärfte auch L99/C8.

---

## 4 — Der Vollabgleich wird vertagt, nicht abgekürzt (P3)

### Der Eingriff, den der Auftrag vermutet, und warum er nicht reicht

Der Auftrag nennt `vars.c:75` als naheliegende Stelle: Der verschachtelte Fall darf
einen Vollabgleich nicht quittungsfrei durchlaufen lassen, er müsste ihn vertagen.
**Das ist richtig, aber an dieser Stelle nicht umsetzbar.** `var_send_buf()` sieht
nur ein einzelnes Kommando; es weiss nicht, ob es Teil eines Vollabgleichs ist, und
es kann ein Kommando nicht vertagen, ohne eine Warteschlange zu bauen, die 194
Einträge fassen muss.

Vertagt wird deshalb eine Ebene höher: **nicht das Kommando, sondern der Abgleich.**

### Die Änderung

`var_send_all_variables()` sendet nichts mehr. Es vermerkt nur noch:

```
var_sync_step  = 0;          // nächster Schritt
var_sync_tries = 0;          // Wiederholungen dieses Abgleichs
var_sync_active = 1;
```

Gesendet wird im Hauptloop, **ein Schritt je Durchlauf**, direkt neben dem IR-Abzug
(`main.c:3506-3534`), der dasselbe tut und dessen Kommentar die Begründung bereits
trägt.

Damit lösen sich drei Dinge auf einmal:

- **Weg (B) verschwindet durch Konstruktion.** `var_send_all_variables()` blockiert
  nicht mehr, also kann es aus der Warteschleife heraus aufgerufen werden, ohne dass
  irgendetwas verschachtelt sendet. `vars.c:75` bleibt stehen — es betrifft dann nur
  noch Einzelkommandos, und das ist eine ungleich kleinere Angriffsfläche als 194.
- **Der Hauptloop läuft während des Abgleichs weiter.** Uhrzeit, RTC, Anzeige,
  Temperatur — alles arbeitet zwischen zwei Schritten. Die Uhr steht nicht mehr still.
- **Zwischen zwei Schritten liegt der reguläre `watchdog_reload()`**, ohne eine neue
  Aufrufstelle.

### Die Schritttabelle und ihre Obergrenze

Ein Schritt ist einer der rund fünfzig Aufrufe, die heute in
`var_send_all_variables()` untereinander stehen (`vars.c:1066-1119+`) — abgelegt als
Tabelle von Funktionszeigern, durchlaufen über `var_sync_step`.

**Ein Schritt darf höchstens 32 Kommandos senden** (AK9). Die Tabelle hat deshalb
eine Ausnahme:

| Schritt | Kommandos | Massnahme |
|---|---|---|
| `var_send_overlays()` | `1 + n_overlays × 8`, bei `MAX_OVERLAYS = 32` also bis **257** | wird zerlegt: ein Schritt für die Anzahl, danach **ein Schritt je Overlay-Index** (8 Kommandos) |
| `var_send_dimmed_display_colors()`, `var_send_dimmed_ambilight_colors()` | je ein Element der Dimmkurve | vom Umsetzer **abzuzählen**; liegt die Zahl über 32, gleich zerlegen |
| `var_send_display_animations()`, `var_send_color_animations()`, `var_send_ambilight_modes()`, `var_send_night_times()`, `var_send_alarm_times()` | je Eintrag mehrere | dito |

Die Zahlen gehören als Tabelle in den Kopfkommentar der Schritttabelle, damit der
nächste Leser sie nicht wieder abzählen muss. 32 Kommandos bei zügiger Brücke sind
unter 100 ms; bei langsamer Brücke greift das Budget aus Abschnitt 2.

### Wiederholung und zweiter Auslöser

L107 nennt drei Lücken: kein Wiederholversuch, keine Warteschlange, kein zweiter
Auslöser. Die Warteschlange braucht es nach der Zerlegung nicht mehr — der Abgleich
**ist** die Warteschlange, und sein Index ist sein Zustand. Die beiden anderen:

| Ebene | Regel | Konstante |
|---|---|---|
| **Kommando** | Bleibt die Quittung aus, wird das Kommando **einmal** wiederholt | — |
| **Abgleich** | Ist mindestens ein Kommando auch nach der Wiederholung ohne Quittung geblieben, gilt der Abgleich als unvollständig und wird nach `VAR_SYNC_RETRY_DELAY_SEC` neu gestartet, höchstens `VAR_SYNC_MAX_TRIES` mal | **20 s**, **3** |

Warum genau **eine** Wiederholung und nicht mehr: Ein zweiter Timeout heisst, dass
die Brücke nicht zuhört. Weitere Versuche fügen nur Last hinzu — das ist die Lehre
aus Abschnitt 5. Der Abgleich als Ganzes neu zu starten ist das wirksamere Mittel,
weil die Brücke in den 20 s dazwischen fertig booten kann.

Ein neuer Abgleich (neue IP-Meldung) **setzt den Index zurück und beginnt von vorn**.
Das ist zulässig, weil jedes Kommando ein unabhängiger Setzer ist: Ein Abbruch mitten
in der Folge hinterlässt keinen halben Datensatz, sondern einen Teilsatz, und ein
Neuanfang von 0 überschreibt ihn vollständig. **Das unterscheidet diesen Fall vom
Overlay-Import**, wo ein Fehlschlag alle folgenden Indizes verschiebt.

Die Wiedereintrittsfalle, die beim IR-Abzug schon gelöst ist (`main.c:3524-3527`),
gilt hier genauso: `var_send_buf()` ruft in seiner Warteschleife
`schedule_esp8266_messages()`, und wenn dabei eine neue IP-Meldung eintrifft, steht
`var_sync_step` bereits wieder auf 0. Dann darf der Hauptloop **nicht** hochzählen,
sonst fiele der erste Schritt des neuen Abgleichs aus. Derselbe Vergleich wie dort.

### Die Abschlusszeile

Jeder Abgleich hinterlässt **genau eine** Zeile im Logring:

```
var sync: vollstaendig, 194 Kommandos, 0 Wiederholungen
var sync: unvollstaendig, 194 Kommandos, 7 Wiederholungen, Versuch 2 von 3
var sync: aufgegeben nach 3 Versuchen
```

Rund 55 Zeichen, eine Zeile je ESP-Neustart. **Keine Variablenwerte** — nur Zahlen
(Abschnitt 5b). Das ist das Nachweismittel für AK11/AK13 und ersetzt jede Vermutung
darüber, ob der Abgleich durchlief.

### Was dieses Design **nicht** heilt

- **Den F411-Hänger.** Dazu sagt das Paket nichts.
- **Einen ESP, der gar nicht zuhört.** Dann scheitern alle drei Versuche, und die
  Abschlusszeile sagt es. Das ist besser als heute (schweigender Verlust), aber es
  ist keine Zustellgarantie.
- **Die Gegenrichtung.** Der STM kann nicht zählen, wie viele Zeichen der **ESP**
  verworfen hat; `d=` zählt den Empfangsring des STM. Der direkte Beleg für AK12
  ist deshalb der Abzugsvergleich, nicht `d=`. `d=` bleibt ein Korrelat für die
  Gesamtlast auf der Brücke — so hat es L102 auch benutzt.
- **Einzelkommandos ausserhalb des Abgleichs.** Ein verschachtelter Einzelsender
  läuft weiterhin ohne Quittungsprüfung durch `vars.c:75`. Das bleibt bewusst stehen
  und wird gezählt (Abschnitt 3).

---

## 5 — Die Spiegelung darf nicht blockieren (P5)

### Was heute passiert

`log_vprintf()` → `esp8266_send_log_line()` → `esp8266_uart_putc()` je Zeichen →
`esp8266_uart_flush()`. Zwei Blockaden hintereinander:

- `uart_putc` wartet, solange der 128-Byte-Senderring voll ist
  (`uart-driver.h:548`, `esp8266-uart.c:68`).
- `uart_flush` wartet, bis der Ring **leer** ist (`uart-driver.h:740-742`).

Bei 115200 Baud sind 60 Byte rund 5,2 ms. Bei 194 Timeouts rund 12 kB und rund eine
Sekunde reine Wartezeit — auf der Leitung, deren Überlastung die Timeouts überhaupt
erst erzeugt hat.

### Die Änderung

1. **Der Flush fällt ersatzlos weg.** Er kauft nichts: Der Senderring ist FIFO, die
   Reihenfolge gegenüber den `var`-Kommandos bleibt auch ohne ihn erhalten, und die
   Interrupt-Routine leert den Ring ohnehin (`uart-driver.h:826-835`). Der Flush
   wartet nur darauf, dass sie damit fertig ist.
2. **Passt die Zeile nicht ganz in den Ring, geht sie gar nicht raus.** Vor dem
   ersten Zeichen wird der freie Platz geprüft — dafür bekommt `uart-driver.h` eine
   Zugriffsfunktion auf `UART_TXBUFLEN - uart_txsize`, nach dem Muster der
   bestehenden Zähler-Zugriffsfunktionen. Ganz oder gar nicht: Eine halb
   geschriebene Zeile wäre schlimmer als keine, weil sie am ESP als vollständige
   ankommt.

### Der Verwurf bleibt sichtbar — ohne neues Diagnosefeld

Die Architektur-Checkliste verbietet stille Verwerfungen ohne Zähler. Für ein
weiteres Feld in der Diagnosezeile ist kein Platz (3 Zeichen, Abschnitt 3). Also
trägt die Marke die nächste Zeile, die durchkommt:

```
!7 diag 1234 l=... t=... u=...
```

„Vor dieser Zeile sind sieben verworfen worden." Der Zähler ist ein `uint8_t`,
sättigend bei 99, und wird nach jeder erfolgreich gespiegelten Zeile auf 0 gesetzt.

Das ist dem Hausbrauch entsprechend — L99 hat dieselbe Frage mit der Kappungsmarke
`~` gelöst — und es sagt **mehr** als ein globaler Zähler: Es ordnet den Verlust
zeitlich ein. Ein globaler Zähler hätte gesagt, dass etwas fehlt; die Marke sagt,
**wo**.

Zwei Nebenwirkungen, beide bedacht: Die Marke verlängert die Zeile um bis zu drei
Zeichen, was bei einer 119-Zeichen-Diagnosezeile die Kappungsmarke `~` auslösen kann
— dann sind beide Marken sichtbar und beide stimmen. Und `measure-log.sh` zählt
Zeilen nach ihrem Anfang; die Auswertung nach Zeilenklassen muss das Präfix
überspringen. Das ist ein Lead-Task, kein Anhängsel.

## 5b — Die Timeout-Meldung gibt Geheimnisse preis (P6)

Beim Schreiben dieser Spec gefunden, am Quelltext belegt:

```c
log_printf ("var_send_buf: keine Quittung nach %ds, weiter ohne: %s\r\n",
            VAR_SEND_TIMEOUT_SEC, buf);                       // vars.c:104-105
```

`buf` ist das ganze Kommando. Für `var_send_weather_appid()` steht dort der
Wetter-Schlüssel, für `var_send_update_host()`/`-path()` die Update-Quelle, für
`var_send_timeserver()` der Zeitserver. Über die Spiegelung landet das im Logring und
ist über `/api/stm32_log` **ohne Anmeldung aus dem ganzen LAN** lesbar. Das betrifft
die **ausgelieferte** Firmware — `log_printf` ist kein leeres Makro, anders als
`debug_log_printf` (L87).

**Die Meldung gibt künftig nur Kennung und Index aus**, also die ersten drei Zeichen
von `buf`, plus die Länge des Werts. Beispiel: `var_send_buf: keine Quittung nach 3s,
S12 (+41)`. Das genügt vollständig, um zu sagen *welches* Kommando verlorenging, und
verrät nicht *was* darin stand. Nebeneffekt: Die Zeile wird kürzer, was Abschnitt 5
entgegenkommt.

Der Punkt gehört in diesen Task, weil A19 genau diese Zeile ohnehin anfasst. Er
gehört in den Befundkatalog als eigener Eintrag — das ist Sache des Leads, nicht
dieser Spec.

---

## 6 — Woran man sieht, dass es wirkt (DIR-008, rein lesend)

### Die Kennzahl des Pakets: `Δu`

Die Diagnosezeile trägt `u=<uptime>`. Die Differenz von `u` zwischen zwei
aufeinanderfolgenden `seq`-Nummern ist die Zeit, die der Hauptloop für einen
Diagnosetakt gebraucht hat. **Sie ist damit das direkte Mass für „die Uhr stand
still".**

| Zustand | grösstes `Δu` |
|---|---|
| heute, schlechtester denkbarer Fall | bis **600 s** |
| nach Runde 1 | ≤ Budget + Watchdog = **50 s** |
| nach Runde 2, Ruhebetrieb | ≈ `DIAG_INTERVAL_SEC` |
| nach Runde 2, während eines Vollabgleichs | ≤ `DIAG_INTERVAL_SEC + 5 s` (AK14) |

`tools/measure-log.sh` bekommt dafür eine Zeile Auswertung: grösstes `Δu`, und
zwischen welchen `seq`-Nummern es auftrat. Das ist ein Lead-Task und die einzige
Werkzeugänderung, ohne die das Paket nicht nachweisbar ist.

**Die Grenze dieser Kennzahl, ausdrücklich:** Setzt der Watchdog zurück, beginnt
`seq` wieder bei 1, und die Lücke erscheint **nicht** als grosses `Δu`, sondern als
Neustart. Beide Ausgänge sind also unterscheidbar — aber man muss nach beiden sehen.

### Der Nachweis für A6: Abzug vor und nach einem ESP-Neustart

```
./tools/snapshot-device.sh referenz          Vergleichsabzug, vorher
<ESP-Neustart — Handlung des Nutzers>
./tools/snapshot-device.sh nachher
./tools/diff-snapshot.sh referenz nachher
```

Erwartet: Abweichungen **nur** in Uptime, LDR-Rohwert, Temperatur und Uhrzeit.
Jede Abweichung in `HARDWARE_CONFIGURATION`, Helligkeit, Zeitzone, Farben, Zeitserver,
Wetter-AppID, Ort, Koordinaten, Datumsformat, Update-Host oder -Pfad ist ein
Fehlschlag.

**Dreimal**, weil L107 die Sprunghaftigkeit ausdrücklich als Zufall des Zeitpunkts
beschreibt: „Im äusseren Hauptloop aufgegriffen greift der Lockstep und alles kommt
an; verschachtelt aufgegriffen ist der halbe bis ganze Satz weg." Ein einzelner guter
Durchlauf belegt deshalb gar nichts — er ist der Normalfall auch heute.

### Die vier Arten, wie dieser Nachweis plausibel aussehen und trotzdem falsch sein kann

| Falle | Woran man sie erkennt |
|---|---|
| **„Der Abzug stimmt"** — in Wahrheit hat der ESP gar nicht neu gestartet, der Abgleich lief nie | Die Abschlusszeile `var sync:` muss **im Ring stehen**. Fehlt sie, hat nichts stattgefunden, und der Vergleich misst nichts |
| **„Keine Timeouts mehr"** — in Wahrheit wartet niemand mehr auf eine Quittung | `<verschachtelt>` und die Abschlusszeile zusammen. Ein Abgleich mit 194 Kommandos und 0 Wiederholungen ist ein Ergebnis; eine Abschlusszeile mit einer deutlich kleineren Kommandozahl heisst, dass die Schritttabelle unvollständig ist |
| **„Die Uhr steht nicht mehr still"** — in Wahrheit hat der Watchdog zurückgesetzt | `seq`. Ein kleines `Δu` **mit** fortlaufendem `seq` ist Ruhe; ein kleines `Δu` nach einem `seq`-Neuanfang bei 1 ist ein Reset |
| **„Keine verworfenen Logzeilen"** — in Wahrheit greift die Platzprüfung nie, weil sie falsch rechnet | Die Marke `!n` muss unter Last **mindestens einmal** erscheinen. Erscheint sie nie, auch nicht während eines ESP-Neustarts, dann prüft die Prüfung nichts |

### Der Nachweis für A16

Der Nutzer startet ein Spiel und rührt es nicht an. Vorher und nachher je eine
Abfrage von `/api/stm32_log`.

- **Bestanden:** Das Spiel endet nach der Inaktivitätsgrenze von selbst, der
  Punkteticker läuft, und die Diagnosezeile danach führt `seq` und `u` lückenlos
  fort.
- **Nicht bestanden:** `seq` beginnt wieder bei 1 — Watchdog-Reset. Oder das Spiel
  endet nicht.
- **Zweiter Durchgang:** `GTq` beendet es weiterhin sofort.

Dass es **ohne** den Fix resettet, wird nicht provoziert. Begründung in
`requirements.md`, „Was nicht nachgewiesen wird".

### Ohne Gerät

`./tools/guardrails.sh` (AK7, AK21), `grep -c 'watchdog_reload ()\s*;' src` (7 nach
Runde 1, 10 nach Runde 2), und das Lesen der Schritttabelle gegen AK9.

---

## 7 — Tetris und Snake: beides, und zwar beides begründet (P7)

### Warum der Reload allein falsch wäre

Ein Spiel, das den Watchdog bedient, blockiert die Uhr **unbegrenzt**. Vorher gab es
wenigstens den Reset als Rettung — am 02.10.2026 hat genau der die Uhr nach sieben
Sekunden wieder ins Leben gebracht (L25). Es braucht deshalb beides: bedienen **und**
einen Weg heraus.

### Warum `GTq` als Weg heraus nicht genügt

Drei Gründe, alle am Quelltext (Belege in `requirements.md`, P7):

1. Er setzt eine lebende Brücke voraus — also genau das, was im Fehlerfall fehlt.
2. Die Spielschleife verwirft alles, was nicht ihr eigener Präfix ist
   (`tetris.c:651-671`, `snake.c:242-262`), und pollt nur alle 10 ms. Der
   Empfangsring kann in dieser Zeit überlaufen und den Ausstieg mitnehmen.
3. Der Schnellfall-Zweig `case 'm'` (`tetris.c:689-693`) pollt gar nicht — bis zu
   rund 550 ms ohne jede Abfrage.

Punkt 1 ist der entscheidende: Ein Ausgang, der von der Brücke abhängt, ist kein
Ausgang für einen Fehler der Brücke.

### Die Änderung

**(a) Watchdog bedienen**, in jeder Schleife, die länger als eine Anzeigeperiode
wartet — drei Stellen:

| Stelle | Datei |
|---|---|
| Abfrageschleife je Fallschritt | `tetris.c:649-674` |
| Schnellfall-Zweig | `tetris.c:689-693` |
| Abfrageschleife in `get_next_move()` | `snake.c:240-265` |

Damit steigt der Bestand von 7 auf **10**; `WD_EXPECTED` und der Kommentar in
`guardrails.sh:221-231` ziehen mit. Ohne diesen Schritt meldet die Bestandswache beim
nächsten Durchgang eine Abweichung und jemand dreht die Änderung zurück.

**(b) Eine Inaktivitätsgrenze**, nicht brückenabhängig:

```
GAME_IDLE_TIMEOUT_SEC = 30
```

Ist seit `GAME_IDLE_TIMEOUT_SEC` kein gültiges Spielkommando eingetroffen, endet das
Spiel wie bei `q`: Punktestand als Ticker, zurück in den Hauptloop. Gemessen über
`uptime`, in beiden Spielen über **dieselbe** Konstante.

**30 Sekunden**, weil ein Tetromino rund 5,5 s fällt: Wer eine halbe Minute nichts
drückt, hat fünf Steine ungesteuert fallen lassen und spielt nicht mehr. Bei Snake
ist es noch deutlicher — die Schlange stirbt ohne Eingabe binnen Sekunden.

**(c)** `snake.c:291` initialisiert `status`, damit `:353` ihn nicht uninitialisiert
liest. Zwei Zeilen, und sie liegen im Ausstiegspfad, den dieser Task härtet — ein
Ausgang mit undefiniertem Zustand ist keiner.

### Was ausdrücklich **nicht** kommt: eine Gesamtspieldauer

Eine harte Obergrenze für ein **aktiv gespieltes** Spiel wäre eine
Produktentscheidung, kein Stabilitätsgewinn. Wer spielt, hält die Uhr absichtlich
an; das ist seine Uhr. Die Inaktivitätsgrenze deckt jeden Fehlerfall ab — tote
Brücke, verlorenes Kommando, Nutzer weggegangen —, und sie deckt den einzigen Fall
**nicht** ab, in dem der Stillstand gewollt ist. Genau das soll sie.

### Der Preis, offen benannt

Ein verlassenes Spiel friert die Uhr jetzt bis zu 30 s ein statt bis zu 20 s (dann
Reset). Das ist etwas länger — aber es endet **geordnet**: kein Neustart, kein
Verlust des Laufzeitzustands, kein erneuter Startdurchlauf. Ein Reset ist keine
Rettung, auf die man hinarbeitet; er ist die Rettung für den Fall, dass alles andere
fehlt.

---

## Betroffene Module

| Datei | Änderung | Runde | Zuständiger Agent |
|---|---|---|---|
| `src/esp8266/esp8266.c` | `OK …` erzeugt einen eigenen Rückgabewert statt `ESP8266_OK` | 1 | `stm-developer` |
| `src/esp8266/esp8266.h` | neuer Rückgabewert | 1 | `stm-developer` |
| `src/main.c` | eigener `case` für den neuen Code; Budget-Nullpunkt am Kopf des Hauptloops; Feld `v=` in der Diagnosezeile | 1 | `stm-developer` |
| `src/vars/vars.c` | Reload-Budget; Timeout- und Verschachtelungszähler | 1 | `stm-developer` |
| `src/vars/vars.h` | Zugriffsfunktionen auf die beiden Zähler | 1 | `stm-developer` |
| `tools/measure-log.sh` | grösstes `Δu` zwischen zwei Diagnosezeilen; Zeilenklassen unterhalb der `!n`-Marke | 1 | Lead |
| `tools/guardrails.sh` | Kommentar zu `WD_EXPECTED` auf die Budget-Begründung | 1 | Lead |
| `src/vars/vars.c`, `vars.h` | Schritttabelle, Wiederholung, Abschlusszeile; Timeout-Meldung ohne Wert | 2 | `stm-developer` |
| `src/main.c` | Takt des Abgleichs im Hauptloop, neben dem IR-Abzug | 2 | `stm-developer` |
| `src/esp8266/esp8266.c` | Spiegelung ohne Flush, Ganz-oder-gar-nicht, `!n`-Marke | 2 | `stm-developer` |
| `src/uart/uart-driver.h`, `src/uart/uart.h` | Zugriffsfunktion auf den freien Platz im Senderring | 2 | `stm-developer` |
| `src/tetris/tetris.c`, `src/tetris/snake.c` | Watchdog in drei Schleifen, Inaktivitätsgrenze, `status` initialisieren | 2 | `stm-developer` |
| `tools/guardrails.sh` | `WD_EXPECTED` 7 → 10, Begründung | 2 | Lead |
| `tools/smoke-device.sh` | `v=`-Felder melden; `var sync:`-Zeile im Ring prüfen | 2 | Lead |
| `src/main.h` | STM-Version, **zweimal** | 1 und 2 | `release-engineer` |
| `BEFUNDE.md`, `CHANGELOG.md` | Stand nachführen, neuer Eintrag für P6 | nach 2 | `doc-writer` |

**Kein ESP-Flash.** Keine PWA-Änderung. Keine Legacy-Änderung.

---

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Die PWA bleibt parallel zur Legacy-Oberfläche; keine der
beiden wird angefasst. Die Wetter-Endpunkte sind nicht im Umfang, der entfernte
Legacy-Bypass wird nicht zurückgebaut. Die Restore-Bedingung um
`pending_weather_ticker_restore` (`main.c`) bleibt vollständig — der Abgleichstakt
wird **vor** diesem Block eingehängt, neben dem IR-Abzug, und liest nur einen Index.
Die Auslieferung der App-Assets ist nicht berührt.

Zur Schicht, und das ist hier die eigentliche Frage: Der Defekt liegt im **STM**, und
er wird im STM behoben. Die naheliegende Umgehung wäre gewesen, die PWA die Werte
direkt vom Gerät nachladen oder den ESP sie persistent halten zu lassen — beides
hätte das Schadensbild verdeckt und die Quelle der Wahrheit vom STM weggeschoben.
Genau der Fehler, den die Checkliste mit „ein Stabilitätsproblem, dessen Ursache auf
dem ESP liegt, wird nicht in `app.js` umgangen" meint, nur spiegelbildlich.

**Scalable systems** —

*Wie viele STM-Kommandos?* **Keine neuen.** Dieselben rund 194, aber nicht mehr als
Burst, sondern über rund 194 Hauptloop-Durchläufe verteilt, höchstens 32 je Durchlauf.
Der Spitzendurchsatz in den 256-Byte-Empfangsring des ESP sinkt damit von
Leitungsgeschwindigkeit auf das, was der ESP zwischen zwei Durchläufen abholen kann —
genau der Mechanismus, der den Verlust erzeugt hat.

*Byte pro Minute auf der UART?* Im Ruhebetrieb unverändert. Auf dem Fehlerpfad
**weniger**: Die Spiegelung blockiert nicht mehr und verwirft, statt zu warten;
die bis zu 12 kB Timeout-Meldungen eines gescheiterten Abgleichs entfallen weitgehend,
und was bleibt, ist kürzer (Abschnitt 5b). Neu hinzu kommen 14 Zeichen je
Diagnosezeile (alle 10 s) und eine Abschlusszeile je Abgleich.

*Bleibt jeder ausgelöste Pfad unter 20 s?* Nein — und das ist bewusst so. Der Reload
darf den Hauptloop um bis zu `VAR_SEND_RELOAD_BUDGET_SEC` = 30 s über das Fenster
hinaus halten, danach greift der Watchdog. Vorher waren es bis zu 600 s. Der
Vollabgleich selbst blockiert nach Runde 2 gar nicht mehr. Die Spiele blockieren bis
zu `GAME_IDLE_TIMEOUT_SEC` = 30 s, danach enden sie geordnet. **Jede dieser drei
Grenzen ist eine benannte Konstante, und `Δu` macht jede von ihnen messbar** — das
ist der Unterschied zu heute, wo keine Grenze existiert und keine messbar ist.

*Wie lange blockiert der Hauptloop?* Siehe `Δu`-Tabelle in Abschnitt 6.

*Wie oft pollt die PWA?* Unverändert.

*Hartkodierte Grenzen?* Die Diagnosezeile liegt nach diesem Paket bei 116 von 119
Zeichen — die Grenze ist benannt, nachgerechnet, und der Preis ihrer Anhebung steht
mit einer Zahl in Abschnitt 3. `MAX_OVERLAYS = 32` ist der Grund für die Zerlegung
von `var_send_overlays()`; die Schritttabelle wächst mit, weil sie je Index einen
Schritt hat statt eine feste Zahl.

**Secure by design** — Kein `innerHTML`, keine Fremddaten, kein neuer Endpunkt, keine
Änderung an einem bestehenden. **Der eine sicherheitsrelevante Punkt ist P6**, und er
wird behoben: Die Timeout-Meldung gibt heute den Wetter-Schlüssel, die Update-Quelle
und den Zeitserver im Klartext an `/api/stm32_log` — ohne Anmeldung aus dem ganzen
LAN lesbar, in der **ausgelieferten** Firmware. Künftig nur noch Kennung, Index und
Länge.

Daraus folgt eine Festlegung für alles Weitere in diesem Pfad: **Keine neue Logzeile
in `vars.c` gibt einen Variablenwert aus.** Die Abschlusszeile des Abgleichs nennt
Zahlen, nicht Werte. Die Diagnosefelder sind Zähler. Die `!n`-Marke trägt keine Daten.
Wer das später aufweicht, veröffentlicht den Inhalt im LAN.

Inhalte vom Update-Server gelten weiterhin als nicht vertrauenswürdig; dieses Paket
berührt den Update-Pfad nicht — ausser indirekt, und zwar zum Besseren: Solange Host
und Pfad verlorengehen können, holt das Gerät Firmware vom Server des
Ursprungsprojekts (L42). A6 schliesst genau das.

**Stable & reliable** —

*Wird jeder Fehler ausgewertet?* Bisher nicht: Ein Kommando ohne Quittung
verschwand, und nichts meldete es ausser einer Zeile, die niemand las. Künftig wird
es wiederholt, der Abgleich wird als unvollständig markiert, neu angestossen und am
Ende ausdrücklich als „vollständig" oder „aufgegeben" gemeldet. **Die Erfolgsmeldung
hängt damit am tatsächlichen Ergebnis** — das war der ungedeckte Punkt.

*Leere `catch`-Blöcke?* Kein JavaScript im Umfang.

*Stille Verwerfungen ohne Zähler?* Drei gab es, alle drei bekommen eine Spur: der
verschachtelte Sender (Zähler), der Timeout (Zähler), die verworfene Logzeile
(`!n`-Marke). Der verschachtelte Einzelsender bleibt bestehen — aber gezählt, nicht
still.

*Werte still zurechtgebogen und als gespeichert gemeldet?* Nicht in diesem Umfang.

*Zustand nach Abbruch mitten in einer Sequenz?* Definiert, und das ist der Punkt, der
hier ausdrücklich zu beantworten ist: Jedes Kommando des Abgleichs ist ein
**unabhängiger Setzer**. Ein Abbruch hinterlässt einen Teilsatz, kein verschobenes
Feld, und ein Neuanfang bei Index 0 überschreibt ihn vollständig. Das unterscheidet
den Fall vom Overlay-Import, wo ein Fehlschlag alle folgenden Indizes verschiebt.
Deshalb ist „von vorn beginnen" hier die richtige und ausreichende Wiederherstellung.

*Wird ein gesetztes Flag auf jedem Pfad wieder aufgelöst?* Zu prüfen sind fünf:
`var_send_busy` und `var_send_nested` (unverändert, auf jedem Pfad zurückgesetzt),
der Budget-Nullpunkt (am Kopf des Hauptloops, also unabhängig vom Ausgang jedes
Senders), `var_sync_active` (beim Erreichen des letzten Schritts **und** beim
Aufgeben nach `VAR_SYNC_MAX_TRIES` — beide Wege müssen zur Abschlusszeile führen),
und die `!n`-Zählung (nach jeder erfolgreich gespiegelten Zeile auf 0). Der
Umsetzer weist das je Flag nach; `var_sync_active` ist das riskante, weil ein
hängengebliebenes Flag den Hauptloop dauerhaft Schritte senden liesse.

---

## Verworfene Alternativen

| Verworfen | Warum |
|---|---|
| **Den Reload wieder entfernen** (A17) | Dann ist L85 zurück: Ein langsam, aber korrekt antwortender ESP bekommt mitten im Abgleich einen Reset und behält einen halben Variablensatz. Der Auftrag sagt es richtig — begrenzen, nicht zurücknehmen |
| **Obergrenze als Zahl der Reloads statt als Zeit** | Die Zahl sagt nichts über den Stillstand. 194 schnelle Kommandos sind harmlos, sieben langsame nicht. Geschützt wird die Uhr, nicht die Brücke |
| **Budget je Vollabgleich statt je Hauptloop-Durchlauf** | Wirkt nur für den Abgleich. Gruppensetzer aus einem RPC-Zweig bleiben ungeschützt, und es gäbe keinen natürlichen Nullpunkt |
| **Zum unbegrenzten Lockstep zurück** (Stand vor `9a0514d`) | Macht aus jedem tauben ESP einen stehenden STM — zweimal reproduziert am 30.09.2026, die Uhr blieb bis zum manuellen Reset stehen |
| **Sequenznummer in die Quittung aufnehmen** | Löst die Fehlpaarung sauberer, kostet aber eine Protokolländerung über zwei Besitzer und einen ESP-Flash. Der Punkt ist bereits eindeutig; es genügt, ihn ernst zu nehmen. **Vorbedingung, unter der es wieder sinnvoll wird:** wenn ein Fehlpaarungsfall auftaucht, den die Punktregel nicht abdeckt |
| **Warteschlange für Kommandos ohne Quittung** | 194 Einträge wollen Speicher, und sie lösen nichts, was die Zerlegung nicht besser löst: Der Abgleich **ist** nach der Zerlegung seine eigene Warteschlange, und sein Index ist sein ganzer Zustand |
| **Den ESP nach dem Boot um einen Abgleich bitten lassen** | Zwei Besitzer, ESP-Flash — und der Auslöser ist gar nicht das Problem. Die IP-Meldung kommt zuverlässig; verloren geht, was danach gesendet wird |
| **Variablen im ESP persistent halten** | Verschiebt die Quelle der Wahrheit vom STM weg und verdeckt den Verlust, statt ihn zu beheben |
| **`vars.c:75` so ändern, dass der verschachtelte Fall doch wartet** | Dann verschachtelt es sich beliebig tief — genau das, wogegen `var_send_nested` eingeführt wurde. Der Kommentar dort beschreibt es korrekt |
| **Die Spiegelung ganz abschalten** | Dann gibt es `/api/stm32_log` nicht mehr, und das ist das einzige Fernauge auf den STM |
| **Die Spiegelung ratenbegrenzen** | Verdeckt, dass die Quelle zu laut ist, und kostet Vergleichslogik im heissen Pfad. Die richtige Antwort ist „nicht blockieren und sichtbar verwerfen" |
| **Dritter Zähler in der Diagnosezeile für verworfene Logzeilen** | Kein Platz: 116 von 119. Die `!n`-Marke sagt ohnehin mehr, weil sie den Verlust zeitlich einordnet |
| **`watchdog_reload()` einfach in die Spielschleifen** | Unbegrenzte Blockade statt Reset. Genau die Lehre aus L25 |
| **Tetris und Snake sperren oder entfernen** | Der Nutzer hat sie, sie funktionieren, und sie sind mit drei Reloads und einer Konstanten reparierbar |
| **Zusätzliche Gesamtspieldauer** | Würgt ein aktiv gespieltes Spiel ab. Das ist eine Produktentscheidung ohne Stabilitätsgewinn; die Inaktivitätsgrenze deckt jeden Fehlerfall bereits ab |
| **Alles in eine Flash-Runde** | Dann steht das Messmittel aus A18 im selben Fabrikat wie der Fix aus A6, und niemand erfährt je, welcher Verlustweg gelaufen ist. „Erst messen, dann reparieren" ist hier keine Formel, sondern der Grund für den Zuschnitt |

---

## Versionsfolgen

- [x] STM `src/main.h` anheben — **zweimal**, je Flash-Runde einmal
- [ ] ESP `version.h` — **nein**, keine Zeile ESP-Code geändert
- [ ] App `APP_VERSION` — **nein**, die PWA wird nicht geändert
- [ ] `CACHE_NAME` in `sw.js` — **nein**, gehört zu `APP_VERSION`

Kein Gleichschritt (DIR-004). Nur der `release-engineer` führt das aus (R4).
