# Anforderungen — Die Kommandobrücke reparieren

**Status:** Entwurf
**Auslöser:** `BEFUNDE.md` A17 (L108), A6 (L42, L102, L103, L107), A19 (L109),
A18 (L110), A16 (L106). Auftrag des Nutzers vom 03.10.2026.

## Warum diese fünf zusammen stehen

Vier der fünf Punkte liegen auf **demselben Pfad**: der Kommandobrücke zwischen STM
und ESP. `var_send_buf()` sendet darauf, `esp8266_get_message()` empfängt darauf,
`log_vprintf()` spiegelt darauf, und die Diagnosezeile berichtet darüber. Wer einen
davon einzeln anfasst, verschiebt die Wirkung auf die anderen.

Der fünfte — A16, Tetris und Snake — liegt **nicht** auf diesem Pfad. Er teilt mit
A17 nur eine Lehre: Den Watchdog zu bedienen, ohne einen Ausweg zu schaffen, tauscht
einen Reset gegen eine unbegrenzte Blockade. Zur Zuschneidung siehe unten.

## Verifikationsstatus der übernommenen Befunde

Jeder Befund wird mit seinem Status übernommen, nicht mit einem besseren.

| Befund | Status laut `BEFUNDE.md` | Was daraus folgt |
|---|---|---|
| **L108** (A17) | offen, schwer. **Pfad belegt, nicht gemessen** (DIR-003) | Der zehnminütige Ablauf ist aus dem Code hergeleitet, nicht am Gerät beobachtet. Die Spec baut deshalb zuerst das Messmittel und begrenzt dann — sie behauptet nicht, dass der F411-Hänger daher kommt |
| **L107** (A6) | **Ursache belegt**, Fix offen | Commit `9a0514d`, die beiden Pfade (A) und (B) sind am Quelltext nachvollziehbar |
| **L102/L103** (A6) | L102 **belegt**, Ursache nicht abschliessend. L103 offen | Das Schadensbild ist dreimal am Gerät beobachtet und einmal durch ein ESP-OTA reproduziert. **Welcher der beiden Pfade** dabei lief, ist nicht belegt |
| **L109** (A19) | offen | Die Mitkopplung ist am Quelltext belegt (`log.c:33` → `esp8266.c:538`, blockierender Flush). Ihre Grössenordnung — rund 12 kB bei 194 Timeouts — ist gerechnet, nicht gemessen |
| **L110** (A18) | offen, klein | Reine Messaufgabe, nichts zu verifizieren |
| **L106** (A16) | offen, schwer | `grep -c watchdog_reload` → 0 und 0 ist **belegt**. Dass die Uhr dabei tatsächlich resettet, ist **aus dem Quelltext erschlossen und am Gerät nie geprüft** — L106 hält das ausdrücklich fest. Diese Spec provoziert den Reset auch nicht; siehe „Was nicht nachgewiesen wird" |

## Problem

### P1 — Der Reload nach Quittung kann den Hauptloop bis zu zehn Minuten blockieren

`vars.c:136` lädt den Watchdog nach, sobald eine Quittung eingetroffen ist. Die
Bedingung deckt zwei Lagen ab: Brücke antwortet, Brücke tot. **Die dritte — Brücke
antwortet sporadisch — fehlt.** Dann bedient jede eintreffende Quittung den Watchdog
erneut, und `var_send_all_variables()` darf mit rund 194 Kommandos à bis zu 3 s
(`VAR_SEND_TIMEOUT_SEC`, `vars.c:48`) bis zu zehn Minuten laufen. Die Uhr steht,
die Oberfläche zeigt eingefrorene Zeit, kein Reset räumt auf. Vor `9eb6dd9` hätte der
Watchdog nach 20 s abgeräumt. **Das ist eine Verschlechterung durch einen Fix von
heute.**

### P2 — Jede Zeile, die mit `OK` beginnt, gilt als Quittung

`esp8266.c:299` setzt `ESP8266_OK` für jede Antwortzeile mit dem Präfix `OK`. Die
Quittung für ein `var`-Kommando ist aber der **Punkt**: `ESP-uclock.ino:444` sendet
`Serial.println(".")` unmittelbar nach `var_set_parameter()`, und `esp8266.c:225-227`
übersetzt ihn nach `ESP8266_OK`.

Der ESP sendet daneben drei unaufgeforderte `OK`-Zeilen, alle während seines
Bootlaufs und keine davon eine Quittung:

| Zeile | Fundstelle | Anlass |
|---|---|---|
| `OK cap` | `wifi.cpp:68` | `statusmsg("OK","cap")` beim Hochfahren |
| `OK ap` | `wifi.cpp:144` | `statusmsg("OK","ap")` beim Wechsel in den AP-Modus |
| `OK time` | `ntp.cpp:101` | `Serial.println("OK time")` nach dem NTP-Abgleich |

Jede davon quittiert ein fremdes Kommando, verschiebt die Paarung dauerhaft um eins
und setzt seit `9eb6dd9` zusätzlich `got_ack = 1` — der Watchdog wird also auf eine
Falschmeldung hin nachgeladen. Die Fehlpaarung ist alt, die Watchdog-Folge ist neu.
Sie trifft ausserdem `main.c:3020`, wo `var_send_busy` auf **jedes** `ESP8266_OK`
zurückgesetzt wird.

### P3 — Der Vollabgleich verliert Variablen, auf zwei Wegen

**(A) Der 3-Sekunden-Abbruch.** Ein Kommando ohne Quittung ist endgültig weg. Es gibt
im ganzen Code keinen Wiederholversuch, keine Warteschlange und keinen zweiten
Auslöser für einen Vollabgleich (L107).

**(B) Der Massenverlust.** `var_send_all_variables()` wird aus
`schedule_esp8266_messages()` gerufen (`main.c:2903`), und das wiederum aus der
Warteschleife von `var_send_buf()` (`vars.c:96`). Trifft die IP-Meldung ein, während
ein anderer Sender wartet, läuft der **gesamte** Abgleich verschachtelt — und jedes
der rund 194 Kommandos kehrt bei `vars.c:75` sofort zurück, **ohne jede
Quittungsprüfung**. Der STM feuert sie mit Leitungsgeschwindigkeit in den 256-Byte-
Empfangspuffer des ESP, der während `http_server_loop()`, `udp_server_loop()` und
`ntp_poll_time()` nicht liest.

Bis April stand dort ein Lockstep; Verlust war strukturell unmöglich. Der Tausch war
nötig — derselbe Commit machte den Watchdog scharf —, nur die Datenverlustseite
fehlte.

**Die Tragweite** (L103, am 03.10.2026 belegt): `HARDWARE_CONFIGURATION` 782 → 65535
(danach schlägt jeder STM-Flash still fehl, DIR-010), Helligkeit 13 → 0, Zeitzone
513 → 0, Display- und Ambilight-Power → 0, alle Farbwerte → 0, Zeitserver,
Wetter-AppID, Ort, Koordinaten, Datumsformat → leer, Update-Host und -Pfad leer
(danach lädt das Gerät Firmware vom Server des Ursprungsprojekts). Dreimal
eingetreten, einmal durch ein ESP-OTA reproduziert.

### P4 — Welcher der beiden Wege lief, ist nicht belegt

Die beobachtete Zeile `var_send_buf: keine Quittung nach 3s` belegt **mindestens
einen** Durchlauf nach (A). Dass **alle** statischen Variablen fehlten, passt besser
zu (B). Das ist Plausibilität, keine Messung (DIR-003). Ohne diese Unterscheidung
weiss niemand, ob der Fix den richtigen Weg trifft.

### P5 — Die Logspiegelung ist eine Mitkopplung

`log_vprintf()` (`log.c:33`) spiegelt **jede** formatierte Zeile zusätzlich auf die
ESP-Brücke, über `esp8266_send_log_line()` (`esp8266.c:538`), mit abschliessendem
**blockierendem** `esp8266_uart_flush()` (`uart-driver.h:740-742`, wartet bis
`uart_txsize == 0`). Schon `uart_putc` blockiert, sobald der 128-Byte-Senderring voll
ist (`uart-driver.h:548`).

Die Timeout-Meldung in `var_send_buf()` ist ein `log_printf`. **Jede ausgebliebene
Quittung legt damit rund 60 Byte auf genau die Leitung, deren Überlastung die
Quittung gekostet hat — und blockiert, bis sie draussen sind.** Bei 194 Timeouts
rund 12 kB in einen 256-Byte-Puffer.

### P6 — Diese Meldung gibt den Wetter-Schlüssel im Klartext heraus

Beim Schreiben dieser Spec gefunden, **am Quelltext belegt, am Gerät nicht
gemessen**, noch ohne Nummer im Katalog:

```c
log_printf ("var_send_buf: keine Quittung nach %ds, weiter ohne: %s\r\n",
            VAR_SEND_TIMEOUT_SEC, buf);                       // vars.c:104-105
```

`buf` ist das vollständige Kommando. Für `var_send_weather_appid()` enthält es den
Wetter-Schlüssel, für `var_send_update_host()`/`-path()` die Update-Quelle, für
`var_send_timeserver()` den Zeitserver. Die Zeile landet über die Spiegelung im
Logring des ESP und ist dort über `/api/stm32_log` **ohne Anmeldung aus dem ganzen
LAN** lesbar. Anders als der bekannte Fall aus `main.c:2199` betrifft das **die
ausgelieferte Firmware**, nicht nur einen Diagnosebau — `log_printf` ist kein leeres
Makro. Ausgelöst wird es genau dann, wenn die Brücke klemmt, also im Schadensfall.

Der Punkt gehört hierher, weil A19 ohnehin genau diese Zeile anfasst.

### P7 — Tetris und Snake bedienen den Watchdog nicht

`grep -c watchdog_reload src/tetris/tetris.c src/tetris/snake.c` → **0 und 0**. Beide
blockieren den Hauptloop: `tetris()` wartet je Fallschritt `loops = 50` × 10 ms =
500 ms (`tetris.c:599`, `:649-674`), ein Tetromino fällt über rund elf Zeilen ≈ 5,5 s.
Nach rund 3,6 Steinen ist das 20-s-Fenster um. `snake()` ist gleichartig gebaut
(`snake.c:240-265`).

Der naheliegende Fix — `watchdog_reload()` in die Spielschleifen — wäre falsch, und
zwar aus demselben Grund wie P1: Ein Spiel, das den Watchdog bedient, blockiert die
Uhr **unbegrenzt**. Vorher gab es wenigstens den Reset als Rettung (L25).

Der vorhandene Ausstieg `GTq` trägt das nicht allein. Drei Gründe, alle am Quelltext
belegt:

1. **Er setzt eine lebende Brücke voraus.** Genau im Fall, in dem der Watchdog
   gebraucht wird, gibt es keinen Ausweg.
2. **Die Spielschleife verwirft alles, was nicht `GT`/`GS` ist.** Sie ruft
   `esp8266_get_message()` und wertet nur den eigenen Präfix aus
   (`tetris.c:651-671`); jede andere Zeile fällt ersatzlos weg. Der Ringpuffer kann
   währenddessen überlaufen und den Ausstieg mitnehmen.
3. **Im Schnellfall-Zweig wird gar nicht gepollt.** `case 'm'` lässt den Stein in
   `while (tetromino_down())` mit `delay_msec(50)` fallen (`tetris.c:689-693`), ohne
   eine einzige Abfrage.

Dazu ein Nebenbefund im Ausstiegspfad selbst: `snake.c:291` deklariert `STATUS
status;` ohne Initialisierung, `:353` liest `status == SNAKE_FAILURE` — und bei einem
`GSq` in der allerersten Runde ist der Wert nie geschrieben worden. Der linke Operand
wird zuerst ausgewertet, das Ergebnis stimmt nur zufällig.

## Ziel

Die Kommandobrücke verliert keine Variablen mehr, blockiert den Hauptloop nicht mehr
unbegrenzt, und man kann an der vorhandenen Diagnosezeile **ablesen**, welcher der
beiden Verlustwege gelaufen ist. Tetris und Snake bedienen den Watchdog und haben
einen Ausgang, der nicht von der Brücke abhängt.

## Reihenfolge — und warum sie zur Anforderung gehört

Erst messen, dann reparieren. Das ist hier keine Stilfrage: Ohne P4 weiss niemand, ob
der Fix für P3 den richtigen Weg trifft. Die Spec zerfällt deshalb in **zwei
Flash-Runden** mit einer Messung dazwischen.

| Runde | Inhalt | Warum hier |
|---|---|---|
| **1** | P2 (Fehlpaarung), P1 (Reload-Budget), P4 (zwei Messfelder) | P1 ist die akute Regression von heute und darf nicht auf eine Messrunde warten. Sie ändert **nicht**, welcher Verlustweg läuft — nur, wie lange die Uhr dabei steht. P2 macht die Messung erst belastbar: Ohne sie setzt eine fremde `OK`-Zeile `got_ack` und der Timeout-Zähler meldet zu wenig |
| **Messung M1** | ESP-Neustart, Diagnosezeile lesen | Beantwortet P4 |
| **2** | P3 (Vollabgleich), P5/P6 (Spiegelung, Klartext), P7 (Spiele) | P5 verändert, wie viele Timeouts überhaupt entstehen — es darf deshalb nicht in der Messrunde stehen |

**Zwischen Runde 1 und M1 liegt eine Flash-Runde, und während M1 läuft, wird nichts
anderes am Gerät getan.** Am 03.10.2026 hat der Lead zwei Aufträge parallel gestartet,
zwischen denen eine isolierende Messung stand; die Isolation war dahin.

## Akzeptanzkriterien

Alle Gerätenachweise sind **rein lesend** (DIR-008), mit genau zwei Ausnahmen, die
beide **der Nutzer** auslöst: der ESP-Neustart und der Start eines Spiels. Beide
werden als fertige Zeile geliefert, nicht als Aufforderung zum Selbersuchen.

### Runde 1

- [ ] **AK1** — `var_send_buf()` wartet im Quelltext ausschliesslich auf die
      Punkt-Quittung. Eine Zeile mit dem Präfix `OK` erzeugt einen anderen
      Rückgabewert und setzt weder `got_ack` noch `var_send_busy` zurück.
      Nachprüfbar durch Lesen von `esp8266.c`, `esp8266.h`, `vars.c`, `main.c:3020`.
- [ ] **AK2** — **Falsifikation zu AK1:** Im Ruhebetrieb über 10 Minuten bleibt das
      neue Feld der Diagnosezeile bei `v=0/0`. Steigt der Timeout-Zähler im
      Ruhebetrieb, kommt die Punkt-Quittung nicht an und AK1 ist falsch umgesetzt —
      nicht „fast richtig".
- [ ] **AK3** — Der Reload in `var_send_buf()` hängt an **einer** Bedingung, die
      zusätzlich zur Quittung ein **Zeitbudget je Hauptloop-Durchlauf** prüft. Das
      Budget wird am Kopf des Hauptloops zurückgesetzt, an derselben Stelle wie der
      reguläre `watchdog_reload()`. `grep -c 'watchdog_reload ()\s*;' src` bleibt
      bei **7**; es kommt keine Aufrufstelle dazu.
- [ ] **AK4** — Die Diagnosezeile trägt das Feld `v=<timeouts>/<verschachtelt>` mit
      zwei sättigenden Zählern. Die maximale Zeilenlänge bleibt **≤ 119 Zeichen**;
      im Logring des ESP trägt **keine** Diagnosezeile die Kappungsmarke `~`.
- [ ] **AK5** — `tools/measure-log.sh` meldet zusätzlich die **grösste Lücke `Δu`
      zwischen zwei aufeinanderfolgenden Diagnosezeilen**. Im Ruhebetrieb liegt sie
      über 5 Minuten bei höchstens `DIAG_INTERVAL_SEC + 2 s`.
- [ ] **AK6 (Messung M1)** — Nach einem vom Nutzer ausgelösten ESP-Neustart lässt
      sich aus `v=<t>/<n>` ablesen, welcher Weg lief:
      `n` springt um rund 190 → Weg **(B)**, der Massenverlust.
      `n` bleibt klein und `t` steigt → Weg **(A)**, einzelne Timeouts.
      Beide klein, Abzug trotzdem beschädigt → **keiner von beiden**, und das ist
      dann der eigentliche Befund.
      Das Ergebnis wird in `tasks.md` eingetragen, bevor Runde 2 beginnt.
- [ ] **AK7** — `./tools/guardrails.sh` läuft mit Exit 0 durch. Der Kommentar zu
      `WD_EXPECTED` nennt die neue Begründung („nach Quittung **und** innerhalb des
      Budgets"), nicht die alte.

### Runde 2

- [ ] **AK8** — `var_send_all_variables()` sendet selbst **kein einziges** Kommando
      mehr. Es vermerkt den Abgleich; gesendet wird im Hauptloop, höchstens ein
      Schritt je Durchlauf, wie beim IR-Abzug (`main.c:3506-3534`). Damit kann
      `vars.c:75` keinen Vollabgleich mehr quittungsfrei durchlaufen lassen.
- [ ] **AK9** — Kein Eintrag der Schritttabelle sendet mehr als **32** Kommandos.
      Der Umsetzer weist das je Eintrag nach und hält die Zahlen im Kopfkommentar der
      Tabelle fest. `var_send_overlays()` (bis zu 1 + 32 × 8 = 257 Kommandos) wird
      dafür zerlegt.
- [ ] **AK10** — Ein Kommando ohne Quittung wird **einmal** wiederholt. Scheitert es
      erneut, gilt der Abgleich als unvollständig und wird nach einer benannten
      Wartezeit erneut angestossen, höchstens eine benannte Zahl von Malen. Beides
      steht als benannte Konstante an genau einer Stelle.
- [ ] **AK11** — Jeder Abgleich hinterlässt **genau eine** Abschlusszeile im Logring,
      die sagt, ob er vollständig war, wie viele Kommandos er sendete und wie viele
      Wiederholungen nötig waren. Sie enthält **keine Variablenwerte**.
- [ ] **AK12 (Nachweis A6)** — `./tools/snapshot-device.sh referenz`, dann ein vom
      Nutzer ausgelöster ESP-Neustart, dann `./tools/snapshot-device.sh` und
      `./tools/diff-snapshot.sh`: Ausser den von Natur aus veränderlichen Feldern
      (Uptime, LDR-Rohwert, Temperatur, Uhrzeit) zeigt der Vergleich **keine**
      Abweichung. Ausdrücklich unverändert: `HARDWARE_CONFIGURATION`, Helligkeit,
      Zeitzone, Display- und Ambilight-Power, alle Farbwerte, Zeitserver,
      Wetter-AppID, Ort, Koordinaten, Datumsformat, Update-Host und -Pfad.
      **Dreimal hintereinander**, weil L107 die Sprunghaftigkeit ausdrücklich als
      Zufall des Zeitpunkts beschreibt — ein einzelner guter Durchlauf belegt nichts.
- [ ] **AK13** — Im selben Durchgang: das Feld `<verschachtelt>` steigt um **weniger
      als 10**, und im Ring steht die Abschlusszeile aus AK11 mit „vollständig".
- [ ] **AK14** — Im selben Durchgang: das grösste `Δu` aus AK5 bleibt unter
      `DIAG_INTERVAL_SEC + 5 s`. Der Abgleich friert die Uhr also nicht mehr ein.
      **Das ist die Kennzahl des ganzen Pakets.** Vorher war ein `Δu` bis 600 s
      möglich, nach Runde 1 bis Budget + 20 s, nach Runde 2 praktisch keines.
- [ ] **AK15** — `esp8266_send_log_line()` blockiert nicht mehr: kein
      `esp8266_uart_flush()`, und eine Zeile, die nicht vollständig in den
      Senderring passt, wird **ganz** verworfen statt halb geschrieben. Der Verwurf
      ist sichtbar — die nächste Zeile, die durchkommt, trägt eine Marke mit der
      Zahl der übersprungenen Zeilen.
- [ ] **AK16** — Die Timeout-Meldung in `var_send_buf()` gibt **Kennung und Index**
      des Kommandos aus, nicht seinen Wert. `grep` auf die Meldung zeigt kein `%s`
      mehr, das auf `buf` zeigt. Gegenprobe am Gerät: `/api/stm32_log` nach einem
      ESP-Neustart enthält keinen Wetter-Schlüssel, keinen Hostnamen und keine
      Koordinate.
- [ ] **AK17** — Tetris und Snake bedienen den Watchdog in **jeder** Schleife, die
      länger als eine Anzeigeperiode wartet — einschliesslich des Schnellfall-Zweigs
      `case 'm'`. `grep -c 'watchdog_reload ()\s*;' src` steigt von 7 auf **10**, und
      `WD_EXPECTED` in `guardrails.sh` zieht mit, mit Begründung im Kommentar.
- [ ] **AK18** — Beide Spiele beenden sich selbst, wenn für `GAME_IDLE_TIMEOUT_SEC`
      kein gültiges Spielkommando eingetroffen ist. Der Ausgang ist derselbe wie bei
      `q`: Punktestand als Ticker, Rückkehr in den Hauptloop. Die Grenze ist **eine**
      benannte Konstante, von beiden Spielen benutzt.
- [ ] **AK19** — `snake.c` liest `status` nicht mehr uninitialisiert.
- [ ] **AK20 (Nachweis A16)** — Der Nutzer startet ein Spiel und rührt es nicht an.
      Erwartung: Es endet nach der Inaktivitätsgrenze von selbst, und die
      Diagnosezeile danach führt `seq` und `u` **lückenlos fort** — sie beginnt nicht
      wieder bei 1. Ein neu beginnendes `seq` bedeutet Watchdog-Reset und damit
      Fehlschlag. Zweiter Durchgang: `GTq` beendet das Spiel weiterhin sofort.
- [ ] **AK21** — `./tools/guardrails.sh --full` läuft mit Exit 0 durch. Der
      Smoketest `./tools/smoke-device.sh` ist grün und meldet die neuen Felder.

## Was nicht nachgewiesen wird, und warum

**Dass die Uhr ohne den A16-Fix wirklich resettet, wird nicht provoziert.** Das wäre
der saubere Vorher-Nachher-Beleg, verlangt aber einen absichtlich herbeigeführten
Watchdog-Reset auf der produktiven Uhr — und nach einem STM-Reset sendet der ESP
keine IP-Meldung, der Vollabgleich läuft also gar nicht erst an, und STM und ESP
können danach auseinanderlaufen. Nachgewiesen wird stattdessen die Gegenrichtung:
Ein fünf Minuten laufendes Spiel überlebt ohne Reset (AK20). Das ist ohne
Watchdog-Bedienung nicht möglich und falsifiziert damit ebenso eindeutig.

**Dass der zehnminütige Ablauf aus P1 je stattgefunden hat, wird nicht behauptet.**
Er ist aus dem Code hergeleitet. Gemessen wird ab Runde 1 das `Δu` — und damit ist
die Frage ab sofort beantwortbar, statt plausibel zu bleiben.

## Nicht Teil dieser Änderung

- **Den F411-Hänger erklären oder beheben.** Kein pauschaler DMA-Fix, kein
  Recovery-Mechanismus (`CLAUDE.md`, Punkt 2). Das Paket liefert Messmittel, keine
  Ursachenbehauptung.
- **Keine Zeile ESP-Firmware.** Der Defekt liegt im STM; der ESP quittiert `var`
  bereits korrekt mit dem Punkt. Kein ESP-Flash nötig.
- **Keine Zeile PWA, keine Zeile Legacy-Oberfläche.**
- **Kein neuer Endpunkt, keine neue numerische Variable, keine Protokolländerung.**
  Insbesondere keine Sequenznummer in der Quittung — das wäre eine Änderung über
  zwei Besitzer.
- **Kein Rückbau auf den unbegrenzten Lockstep** von vor `9a0514d`.
- **L99 / C8** — die zweistufige Kappung langer Logzeilen bleibt, wie sie ist. Die
  Brücke wird nicht aufgeweitet.
- **A15** (UART-ISR entschlacken), **A13** (`strncpy` in `esp8266.c`), **A14**
  (Maskierung in `var_send_string`) bleiben offen, auch wenn dieselben Dateien
  angefasst werden.
- **Die Spiele inhaltlich** — Geschwindigkeit, Punktezählung, Steuerung, Startweg
  über UDP bleiben unverändert.
- **Die Ringgeometrie des Logbuchs** (64 × 120) und das Diagnoseformat über 119
  Zeichen hinaus. Was danach käme, steht in `design.md` mit Preisschild.
- **Versionsnummern** stehen in keiner lebenden Doku (DIR-006).

## Zuschnitt — ein Einwand und zwei Korrekturen am Auftrag

**A16 gehört fachlich nicht in dieses Paket, praktisch aber in dieselbe Flash-Runde.**
Der Auftrag sagt, alle fünf Punkte hingen an demselben Pfad. Für A16 stimmt das
nicht: Tetris und Snake berühren weder `vars.c` noch `esp8266.c` noch `log.c`, und
ihr Nachweis braucht eine **schreibende** Handlung am Gerät, die der Rest des Pakets
nicht braucht. Ich schlage trotzdem vor, ihn mitzunehmen — aber als letzten Task der
zweiten Runde, nach allen Brückenarbeiten und nach deren Nachweis. Begründung: Eine
eigene Flash-Runde für zwei Funktionen, die nur über UDP erreichbar sind, ist teurer
als der Aufwand, ihn sauber getrennt zu halten. **Er hat null Abhängigkeiten zum
Rest** — wer das Paket kürzen will, streicht ihn, ohne dass sich sonst etwas ändert.

**A17 ist nicht ein Punkt, sondern zwei.** Die Fehlpaarung (`esp8266.c:299`) und das
Reload-Budget (`vars.c:136`) sind verschiedene Defekte. Die Fehlpaarung ist älter,
breiter — sie trifft auch `var_send_busy` — und sie ist die **Voraussetzung** dafür,
dass das Budget überhaupt etwas bedeutet: Ein Reload auf eine Falschquittung hin ist
durch kein Budget gerechtfertigt, er ist schlicht falsch. Sie wird deshalb zuerst
umgesetzt, als eigener Task.

**A18 kommt vor A6 — aber nicht vor A17.** Der Auftrag bittet um „A18 vor A6, wenn
das geht". Das geht, und es geht sogar besser: A17 darf mit in die Messrunde, weil es
nicht ändert, *welcher* Verlustweg läuft, sondern nur, wie lange die Uhr dabei steht.
Die akute Regression von heute muss damit nicht auf eine Messrunde warten.

## Betroffene Laufzeiten

- [x] STM32 (`src/**`) — Neu-Flashen nötig, **zweimal** (Runde 1 und Runde 2)
- [ ] ESP8266 (`ESP8266/ESP-uclock/*.cpp`) — **nicht berührt**
- [ ] PWA (`data/app/**`) — **nicht berührt**
- [x] Build/Release — Versionen, `guardrails.sh`, `measure-log.sh`,
      `smoke-device.sh`

## Schreibregeln für die Umsetzung

Die Spec selbst ist Deutsch mit echten Umlauten und Schweizer `ss`. **Die
Quelldateien unter `src/**` sind es nicht.** Sie sind ASCII beziehungsweise
ISO-8859-1. Jeder Kommentar, der dort entsteht, wird in Umschrift geschrieben —
`Bruecke`, `waehrend`, `Quittung`, `zurueck`. Ein Werkzeug, das ein `ä` in eine
ISO-8859-1-Datei schreibt und dabei UTF-8 annimmt, beschädigt die Datei; das ist in
diesem Projekt bereits passiert. Das gilt auch für jeden neuen Text in einem
`log_printf`.
