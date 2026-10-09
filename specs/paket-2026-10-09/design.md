# Design — Paket 2026-10-09 „Brückenlast" (L338/C43, L339/A58)

**Erstellt:** 2026-10-09, Stand `dea187a`. **Zur Freigabe** (Ent-1). Momentaufnahme (DIR-006).
Grundlage ist die Analyse des `firmware-analyst` vom 09.10.2026; wo sie Zeilen mit `~`
angibt, sind sie hier nachgelesen und berichtigt.

**Schrittfolge:** **V (lesend) → E (ESP) → M (STM, Messzeile) → C3 (STM, seitenweise) → Z**.
Bei Ent-3 = V3 entfällt M als eigenes Release (§4.3).

---

## 0. Warum diese Reihenfolge

| Frage | Antwort |
|---|---|
| Warum ESP zuerst? | L338 liegt ganz auf dem ESP, und Teil C ändert, **was** über die Brücke zum STM kommt. Würde C nach der Vorher-Messung von L339 eingespielt, unterschieden sich Vorher und Nachher in **zwei** Dingen (Echo-Länge und C3), und die Zusage „nur C3 ändert sich" wäre nicht einlösbar (R3b) |
| Warum Teil C mit Teil A in **einem** ESP-Release? | Beide sind klein, berühren verschiedene Funktionen, und keiner verändert die Wirkung des anderen: Der Wetterabruf läuft nicht über `http_client`, das Echo nicht über den Wetterpfad. Der Nachweis G1 trennt sie über verschiedene Zeilen (`- weather` gegen `- request`) |
| Warum eine Messzeile **vor** C3? | DIR-014: Eine Messung, die nie den schlechten Wert gezeigt hat, ist nicht geprüft. Und L204: Dort war die EEPROM-Spur die erste Erklärung und falsch |
| Warum die Messzeile für den ESP **nicht** vorab? | Für L338 gibt es ein **unabhängiges Mass** schon im heutigen Mitschnitt: den Abstand zwischen dem `weather`-Kommando und der `WEATHER`-Antwort (S.26: 5,25 s und 5,3 s). Die neue Zeile wird nach dem Einspielen gegen genau diesen Abstand geprüft (AKE.6) |

---

## 1. Teil A — L338: Wetterabruf mit Gesamtgrenze (ESP)

### 1.1 Ausgangslage am Code

`query_weather()` (`ESP8266/ESP-uclock/weather.cpp:307-427`), aufgerufen aus **acht** Stellen
in `ESP-uclock.ino` (`:1517`, `:1525`, `:1583`, `:1591` Wetter und Vorhersage; `:1649`,
`:1657`, `:1715`, `:1723` Icons). Die Korrektur in `query_weather()` wirkt auf alle acht.

| Zeile | Heute | Folge |
|---|---|---|
| `:353` | `openweather_client.connect (hostname, 80)` | DNS und Verbindungsaufbau blockieren; ● ob der `WiFiClient`-Timeout sie begrenzt, klärt V.1 (5) |
| `:359` | `delay (200)` fest | 200 ms Blockade auch bei schneller Antwort |
| `:361` | `if (available ())` | Antwort nach mehr als 200 ms ⇒ **nicht gelesen**, still |
| `:367-392` | Kopfzeilen per `readStringUntil('\n')` bis Leerzeile, höchstens 20, Schleife nur solange `available ()` | Kopf in Teilsegmenten ⇒ Schleife endet zu früh |
| `:394-405` | erste Körperzeile; ist sie kürzer als 10 Zeichen (Chunk-Länge), die nächste | `readStringUntil` wartet ohne `\n` bis zum Timeout (5 s) |
| `:407-417` | `parse_weather (p, …)` bzw. `parse_weather_fc (p, …)` | — |

### 1.2 Entwurf

**Eine** Lesehilfe ersetzt die drei `readStringUntil`-Aufrufe und die `available ()`-Prüfungen
— keine zweite Variante daneben:

- Liest Zeichen, solange **`available () || connected ()`** und die **Gesamtfrist** nicht
  abgelaufen ist; endet bei `\n`, bei Verbindungsende ohne weitere Daten oder bei Fristablauf.
- Liefert, ob eine Zeile vollständig ist, ob die Verbindung zu Ende ist, oder ob die Frist
  abgelaufen ist.
- **Gibt in jedem Durchlauf ohne Zeichen die Kontrolle ab** (`yield ()` oder `delay (1)`) —
  sonst schlägt der Soft-Watchdog des ESP zu (AKE.5).

`delay (200)` und das nackte `if (available ())` entfallen: Gewartet wird auf das **erste
Byte** bis zur Gesamtfrist. Die **Parselogik bleibt unverändert**: Kopf bis zur Leerzeile
(höchstens 20 Zeilen), dann eine Körperzeile, bei weniger als 10 Zeichen die nächste. Endet der
Körper ohne `\n`, gilt das Verbindungsende als Zeilenende — genau der Fall, der heute 5 s kostet.

**Gesamtfrist:** gemessen ab dem Beginn von `query_weather()`. Ob sie auch `connect ()`
einschliesst, hängt an V.1 (5): Lässt sich `connect ()` im verwendeten Core über
`setTimeout ()` begrenzen, wird der Timeout vor `connect ()` auf die Restfrist gesetzt; sonst
steht die verbleibende Lücke im Bericht, und die Messzeile zeigt sie (`connfail ms=…`).

**Messzeile** am Ende **jedes** Pfads, einschliesslich `connfail`:

```
- weather fc=<0|1> ms=<n> <ok|timeout|connfail|leer>
```

Aufbau wie `esp_heap_log()` (`ESP-uclock.ino:483-511`): **einmal** per `snprintf` in einen
Stackpuffer formatieren, **zweimal** ausgeben — `Serial.println` **und**
`stm32_log_append ()`. Kein `String` (L175). Das Präfix `- ` erkennt der STM als
`ESP8266_DEBUGMSG` (`src/esp8266/esp8266.c:507`). **Keine** `appid`, kein Ort, keine
Koordinaten, keine URL.

**Bei `timeout`, `connfail` und `leer`** wird nichts geparst; der STM erhält dann dieselbe
Nicht-Antwort wie heute bei einem Verbindungsfehler. ● Wie der STM auf eine ausbleibende
`WEATHER`-Antwort reagiert, klärt V.1 (6); der Zustand muss danach definiert sein.

### 1.3 Die Gesamtgrenze

`WEATHER_TOTAL_TIMEOUT_MS` = **2'500 ms**. Herleitung:

- Der STM wartet in `var_send_buf()` **3 s** auf eine Quittung. Fällt ein quittungspflichtiges
  Kommando auf den Beginn des Abrufs, wartet es die ganze Blockade plus die Verarbeitung der
  Quittung. 2'500 ms lassen 500 ms Abstand.
- Ein gesunder Abruf liegt nach der Analyse deutlich unter 2 s (AKE.6 verlangt das am Gerät).
- **Abwägung, ausdrücklich:** Bei langsamem Netz fällt eher ein Wetter-Update aus (`timeout`).
  Das ist gewollt — es kommt nach rund 20 Minuten wieder. Ein verlorener Einmal-Wert (L42) kommt
  nicht wieder.

Der Wert ist eine **technische Festlegung dieser Spec**, keine Nutzerentscheidung; der
`code-reviewer` prüft sie in E.5 gegen die 3-s-Wartezeit im STM-Quelltext.

### 1.4 Prüfstand t12

Vom `esp-developer` im Scratchpad gebaut, vom Lead unter `tools/checks/auszug/esp/t12/`
abgelegt. Auszug von `query_weather()` aus dem echten `weather.cpp` (`extract.py`), dazu
nachgebildet: `WiFiClient` mit **Core-Semantik** (`readStringUntil` wartet bis zum Timeout,
auch nach Verbindungsende), `millis ()`/`delay ()`/`yield ()` auf **simulierter Zeit** — der
Prüfstand wartet nie wirklich —, `parse_weather*()` als Rekorder der übergebenen Zeichenkette.
Fälle: AKE.1 bis AKE.4. **Gegenprobe gegen den Ausgangsstand** über `auszug.sh --gegen`.

---

## 2. Teil B — L339: EEPROM seitenweise schreiben (STM)

### 2.1 Ausgangslage am Code

- `eeprom_write (start_addr, buffer, cnt)` (`src/eeprom/eeprom.c:201-240`): je Byte ein
  `i2c_read()` mit einem Byte; ist es gleich, weiter; sonst `i2c_write()` mit einem Byte und
  `eeprom_waitstates()` (15 bis 16 ms, `:38-57`). Lesefehler ⇒ schreiben; Schreibfehler ⇒
  `rtc = 0`, Abbruch.
- Aufgerufen über das Makro `eep_write()` (`src/eep/eep.h:39`); `grep` zählt **55 Vorkommen
  in 15 Dateien**, darunter die Makrodefinitionen selbst — also rund fünfzig Aufrufstellen in
  Anzeige, Overlays, IR, Wetter, Nachtzeiten, Wecker, LDR, DFPlayer, Zeitserver, RTC und
  `main.c`. **Jede davon ist von C3 betroffen.**
- `i2c_write()` und `i2c_read()` (`src/i2c/i2c.c:374-478`) übertragen beliebig viele Byte;
  `eeprom_read()` nutzt das bereits.
- Adressierung 16 Bit (`is_16_bit_addr = 1`); der AT24C32 hat 4'096 Byte.
- Die Bereiche liegen lückenlos hintereinander (`src/eeprom/eeprom-data.h:174-225`: der erste
  bei 0, jeder weitere als Summe aus Offset und Grösse des vorigen, z. B.
  `EEPROM_DATA_OFFSET_UPDATE_HOSTNAME` in `:208`). Sie sind **nicht** auf Seiten ausgerichtet —
  die Seitenrechnung muss mit beliebigen Startadressen umgehen.

### 2.2 Stufe 1 (Schritt M): die Messzeile

In `eeprom_write()`, **ohne jede Verhaltensänderung**:

```
eep a=<start_addr> n=<cnt> z=<schreibzyklen> ms=<dauer> d=<rxdrops vorher>/<rxdrops nachher>
```

- `z` zählt die Aufrufe von `eeprom_waitstates()`. Damit prüft sich die Zeile selbst:
  `15·z ≤ ms ≤ 17·z + 10` (AKM.2). Stimmt das nicht, ist die Zeitbasis falsch.
- `d` über `esp8266_uart_rxdrops()` (heute in `src/main.c:362` und `:3896` sichtbar gemacht).
  Die Abhängigkeit des EEPROM-Moduls vom Brücken-UART ist **bewusst** und im Code-Kommentar
  begründet; die **Aufrufstellen bleiben unberührt**.
- Zeitbasis: Der `stm-developer` nennt sie im Bericht (z. B. ein vorhandener Millisekundenzähler).
- Ausgabe nur, wenn `z ≥ 1`. **Schwelle `ms ≥ 20` zusätzlich nur dann**, wenn V.1 (2) einen
  periodischen Schreiber findet, der öfter als einmal je Minute schreibt — sonst bliebe die
  Zeile dort Dauerrauschen.
- **Kein Inhaltsbyte** in der Zeile (Geheimnisse wie die `appid` liegen im EEPROM).
- `log_printf()` geht über die Log-UART **und** als `LOG …` zum ESP (`src/log/log.c:31-33`) —
  also in Gegenrichtung zum überlaufenden Empfangsring. ● Ob `eeprom_write()` vor
  `log_init()` aufgerufen werden kann, klärt V.1 (3); falls ja, darf die Zeile dort nicht
  ausgegeben werden.
- Der Füllstand des Rings (`uart_rxsize`) ist heute nicht nach aussen geführt; die
  `diag`-Zeile zeigt den Höchststand `rx`. **Ein Füllstand in der Messzeile ist erwünscht, aber
  nicht Pflicht:** Kostet der neue Zugriff mehr als rund 40 Byte Flash, entfällt er, und der
  Bericht sagt es.

### 2.3 Stufe 2 (Schritt C3): seitenweise

```
EEPROM_PAGE_SIZE   32        // Quelle: Datenblatt, V.1 (1)

solange cnt > 0:
    seitenende  = (start_addr / P + 1) · P
    len         = min (cnt, seitenende − start_addr)
    lies len Byte ab start_addr in einen Puffer (eine Transaktion)
    lesen gescheitert  ⇒ erstes = 0, letztes = len − 1
    sonst              ⇒ erstes/letztes = erster/letzter Index mit Unterschied
    kein Unterschied   ⇒ nichts schreiben, kein Wartezyklus
    sonst              ⇒ i2c_write (start_addr + erstes, buffer + erstes, letztes − erstes + 1)
                          gescheitert ⇒ return 0, keine weitere Seite
                          eeprom_waitstates ()
    start_addr += len; buffer += len; cnt −= len
return 1
```

**Was erhalten bleibt:** Unveränderte Bytes kosten keinen Schreibzyklus — jetzt **je Seite**:
Eine Seite ohne Unterschied wird nicht geschrieben. Innerhalb einer Seite werden die Bytes
zwischen dem ersten und dem letzten Unterschied mitgeschrieben, mit ihrem **eigenen, gleichen**
Wert; das ändert den Inhalt nicht und kostet keinen zusätzlichen Zyklus. Die Fehlerrichtung
bleibt streng: **unlesbar gilt als verschieden**.

**Zustand nach Abbruch:** Heute ist nach einem Schreibfehler ein **Byte-Präfix** geschrieben,
ab jetzt ein **Seiten-Präfix**. Der Aufrufer erhält in beiden Fällen 0. Das Verhalten der
Aufrufer ändert sich nicht.

**Kosten:** Ein Puffer von 32 Byte auf dem Stack; Code ● geschätzt in V.1 (4), gemessen in C3.3.

**Kommentar in `eeprom.c`:** Der bestehende Kommentar (`:180-200`) spricht von „rund 15 ms
Busy-Wait je geschriebenem Byte" und einem Ring von 256 Byte. Der `stm-developer` führt ihn
in C3.1 auf „je Seite" nach; die historische Messung bleibt als solche kenntlich.

### 2.4 Tragweite und Risiko — ausdrücklich

**C3 ändert jeden EEPROM-Schreibvorgang der Uhr**: Farben, Helligkeit, Nachtzeiten,
IR-Codes, Overlays, Zeitzone, Zeitserver, Wetter, Update-Quelle. Ein Fehler in der
Seitenrechnung ist **kein Leistungsfehler, sondern Datenverlust**:

- **Seitenüberlauf:** Schreibt eine Transaktion über das Seitenende hinaus, bricht der Baustein
  **auf den Anfang derselben Seite** um und überschreibt dort fremde Einstellungen — **still**,
  denn `i2c_write()` meldet Erfolg. Trifft das die Update-Quelle, ist es der Weg aus L42.
- **Falsche Seitengrösse:** Ist `EEPROM_PAGE_SIZE` grösser als die echte Seite, entsteht genau
  dieser Überlauf. **Kleiner ist immer sicher**, nur langsamer. Deshalb verlangt AKC.1 das
  Datenblatt **beider** Bausteine — ● der F103-Aufbau nutzt ein RTC-Modul, dessen Baustein nicht
  in `HARDWARE.md` steht.
- **Abgefangen wird das durch den Prüfstand**, nicht durch den Gerätetest: Ein überlaufender
  Schreibzugriff fiele am Gerät erst auf, wenn jemand die betroffene Einstellung ansieht. Der
  Prüfstand bildet den Umbruch **wie die Hardware** nach und vergleicht das ganze Abbild nach
  jedem Aufruf (AKC.2, AKC.3). Die Integritätsprüfung am Gerät (AKC.7) ist die zweite Linie.

### 2.5 Prüfstand C3

Vom `stm-developer` im Scratchpad gebaut, vom Lead unter `tools/checks/auszug/stm/c3/`
abgelegt. Zwei Auszüge von `eeprom_write()` — **alt** (aus dem M-Stand, mit nachgebildetem
Log) und **neu** — gegen ein nachgebildetes EEPROM:

- 4'096 Byte, 16-Bit-Adresse; **Schreiben bricht am Seitenende auf den Seitenanfang um**,
  Lesen läuft über Seitengrenzen hinweg und bricht am Speicherende um — wie der Baustein.
- Protokoll jeder Transaktion (Adresse, Länge) und jedes Wartezyklus.
- Fehlereinspeisung für Lesen und Schreiben auf einer wählbaren Transaktion.
- **Bereiche aus den echten Headern**, soweit sich `eeprom-data.h` auf dem Host übersetzen
  lässt; sonst nennt der Bericht den Grund. **Die Zufallsfolge über den ganzen Adressraum ist
  unabhängig davon Pflicht** und deckt jeden Bereich ab.
- Typbreiten des Ziels (`pruefstand.h`, L256).
- Ausgabe: Fallzahl, Zahl der Vergleiche, Zyklen alt/neu für die S.26- und die G2/G3-Folge
  (AKC.5). **Zähl nach**, ob die Zahl der Fälle der Erwartung entspricht (`CLAUDE.md`, DIR-014).

### 2.6 Flash-Gate

**Lage:** 1'688 Byte frei, Reserve **1'024 Byte**, nutzbar **664 Byte**.

| Stufe | Teil | Was | Bei Unterschreitung der Reserve |
|---|---|---|---|
| 1 | V.1 (4) | Schätzung für Messzeile und C3, je mit Unter- und Obergrenze, hergeleitet aus gemessenen Zuwächsen und neuen Zeichenketten (L168) | geht an den Nutzer, bevor M.1 beginnt |
| 2 | M.2 | Testbau nach der Messzeile | **anhalten und vorlegen**; erste Sparstufe: Füllstand weglassen (§2.2) |
| 2 | C3.3 | Testbau nach C3 | **anhalten und vorlegen** |
| Gegenprobe | C3.6 | S8b im Release-Build | kein Rollout unter 1'024 Byte |

● nicht gemessen, nur zur Einordnung: Messzeile und Seitenrechnung sind eher Dutzende bis
wenige Hundert Byte.

---

## 3. Teil C — Debug-Echo ohne Querystring (ESP)

**Entwurf:** Eine Funktion gibt die Echo-Zeile aus; **beide** Stellen rufen sie
(`http.cpp:14071-14079` POST, `:14129-14143` GET). Form:

```
- request <IP> [<label>]: GET /api/weather_city_set?…
- request <IP> [<label>]: GET /app/
```

Methode und Pfad bleiben, alles ab `?` wird durch den Marker `?…` ersetzt, die
HTTP-Version entfällt. **Eine Regel für alle Anfragen** — keine Liste von Settern, die bei jedem
neuen Endpunkt nachgeführt werden müsste und still veraltet (dieselbe Gattung wie L179).

**Was bleibt sichtbar:** welcher Endpunkt, von welcher Adresse, mit welchem Browser, und
**ob** ein Querystring dabei war — `tools/watch-log.sh:128` zeigt damit vor einer
`Exception (29)` weiterhin, welcher Endpunkt zuletzt kam.

**Ersparnis auf der Brücke:** je Setter die Länge des Querystrings, bei Update-Host oder -Pfad
mit 63 Byte und URL-Kodierung weit über 70 Byte, beim Wetterort bis rund 40 Byte. Polling ohne
Query bleibt gleich lang.

**Was E.2 zusätzlich liefert:** eine Liste weiterer Stellen in `http.cpp`, die Anfragewerte
über `Serial` ausgeben (`grep` nach `Serial.print` mit `sParam`, `param`, `value`). **Gemeldet,
nicht mitkorrigiert.**

**Prüfstand t13** (Auszug der neuen Funktion) und eine **statische Prüfung** (kein
`Serial.print*` mit `sRequest`/`sParam` mehr in `http.cpp`). Die statische Prüfung ist der
Gegenproben-Träger: Gegen den Ausgangsstand schlägt sie an; t13 lässt sich gegen ihn nicht
übersetzen, weil die Funktion dort nicht existiert — das ist **kein** Fehlschlag im Sinn von
DIR-014 und wird im Bericht nicht als solcher gezählt.

---

## 4. Gerätenachweise

### 4.1 G1 — L338 und Teil C (nach E.8)

`watch-log.sh` läuft. Je **ein** `weather_get_now` und `weather_get_forecast`, mindestens 10 s
Abstand. Ausgewertet: `- weather`-Zeile (Mitschnitt **und** `/api/stm32_log`), Abstand
`weather`→`WEATHER`, `v` im 6-s-Fenster, kein Neustart. Danach **60 Minuten lesend**
mitschneiden (automatische Abrufe). `- request`-Zeilen ohne `=` (AKE.8), Zahl der geprüften
Zeilen gemeldet.

### 4.2 G2 / G3 — L339, Setter-Burst auf dem Wetterort

**Warum der Wetterort:** Er liegt im EEPROM (`src/weather/weather.c:89-100`, 32 Byte,
`weather_set_city()` füllt per `strncpy` mit Nullen auf), ist harmlos, und der ESP nimmt ihn mit
bis zu 32 Byte an (`http.cpp:8644-8678`). **Nicht** Update-Host oder -Pfad.

**Ablauf:**

1. **Vorbedingung:** Originalwert des Wetterorts und der Koordinaten lesend festhalten. Ist der
   Ort leer, akzeptiert der ESP die Rückkehr zu „leer" nur bei gesetzten Koordinaten
   (`http.cpp:8662-8668`). **Sind Ort und Koordinaten leer: Halt**, anderes Ziel mit dem Nutzer
   klären.
2. Sieben `weather_city_set` in rund 1,5 s, abwechselnd ein 32-Byte-Testwert aus
   ASCII-Buchstaben ohne Leerzeichen und ein 1-Byte-Testwert — jeder Wechsel ändert rund 31 Byte.
3. Mindestens 4 s warten, Originalwert zurückschreiben.
4. STM zurücksetzen (`maintenance_reset_stm32`), damit der ESP den STM-Stand **aus dem EEPROM**
   neu erhält; danach den Wetterort lesend prüfen — er muss dem Original entsprechen.

**Warum `d` im Vorher (G2) nicht steigen muss:** S.26 lief mit 64-Byte-Werten (rund 1 s je
Setter) **und** mit dem langen Echo. G2 läuft nach Teil C mit gekürztem Echo und 32-Byte-Werten
(rund 0,5 s je Setter). Ob der Ring dabei überläuft, ist offen. **Tragend ist die Messzeile**
(`z`, `ms`); bleibt `d` bei 0, ist eine Wiederholung mit 14 Aufrufen in rund 3 s freigegeben
(Ent-1). Bleibt `d` auch dann bei 0, steht im Bericht, dass der Überlauf-Teil von L339 über
S.26 und die Rechnung aus AKC.5 belegt ist, nicht über G3.

**Nachher (G3):** derselbe Ablauf; erwartet `z ≤ 2`, `ms ≤ 60`, `d` unverändert (AKC.6).

### 4.3 Bei Ent-3 = V3

Schritt M entfällt als Release. Die Messzeile fährt mit C3; G2 entfällt; AKM.3 wird durch
AKC.5 und S.26 ersetzt, AKM.2 gilt in G3. Der Bericht nennt die Schwäche aus Ent-3 wörtlich.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Schritt |
|---|---|---|---|
| `ESP8266/ESP-uclock/weather.cpp` | Lesehilfe mit Gesamtfrist, `delay (200)` weg, Messzeile | `esp-developer` | E |
| `ESP8266/ESP-uclock/http.cpp` | Echo über eine Funktion, ohne Querystring | `esp-developer` | E |
| `src/eeprom/eeprom.c` | M: Messzeile; C3: seitenweise, Kommentar nachgeführt | `stm-developer` | M, C3 |
| `tools/checks/auszug/esp/t12/`, `t13/`, `tools/checks/auszug/stm/c3/`, `tools/checks/auszug.sh` | Prüfstände ablegen und eintragen; statische Echo-Prüfung | Lead | E, C3 |
| `tools/watch-log.sh` | `- weather … timeout`, Wetter-`ms` ≥ 2'500, `eep … ms` ≥ 200 melden | Lead | E |
| `knowledge/architecture-checklist.md` | §2: EEPROM-Kosten je Seite statt je Byte; Ringgrösse nicht als Zahl, sondern mit Verweis auf `uart-driver.h` (heute steht dort „256 Byte", der Ring hat 1'024) | `doc-writer` | Z |
| `BEFUNDE.md` | L338/C43, L339/A58, C3, neuer Befund (Ent-4), Instrumentierung (Ent-2) | `doc-writer` | Z |
| `CLAUDE.md` | „Offene technische Themen" Nr. 2 (C3 erledigt), Hardware-Abschnitt („16 ms pro Byte" ⇒ je Schreibzyklus, seit C3 je Seite) | Lead | Z |
| Versionsdateien | ESP (E), STM (M), STM (C3) | `release-engineer` | je Schritt |

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015, AKZ.8).

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**PWA parallel zu Legacy?** Ja, beide unberührt. Teil C ändert nur eine Debugzeile, die keine
Oberfläche auswertet.

**Wetter-Endpunkte?** `/api/weather_get_now` und `/api/weather_get_forecast` bleiben der Weg;
geändert wird nur, **wie** der ESP danach beim Wetterdienst liest. Kein Rückbau auf
`/weather?action=…`.

**Restore-Bedingung um `pending_ticker_restore`?** Nicht berührt. Ein Abruf mit `timeout`
liefert keinen Wetterticker — derselbe Zustand wie heute bei einem Verbindungsfehler; ● V.1 (6)
bestätigt, dass dabei kein Flag offen bleibt.

**Nur `.gz`?** Nicht berührt.

**Richtige Schicht?** Ja, in beiden Fällen **an der Ursache**: L338 auf dem ESP, wo gelesen
wird — nicht im STM durch eine längere Wartezeit, die jeden anderen Ausfall verschleierte. L339
in `eeprom_write()`, der einzigen Stelle, die alle Aufrufer gemeinsam haben — nicht in den
zwei Aufrufern von S.26 und nicht über einen grösseren Ring.

### Scalable systems

**STM-Kommandos?** Keine zusätzlichen. Der Burst aus G2/G3 erzeugt sieben `CMD`, wie jeder
Nutzer, der schnell speichert.

**Byte je Minute auf der UART?** **Weniger:** Teil C spart je Setter die Länge des
Querystrings. Dazu: die Wetterzeile rund 35 Byte je Abruf (rund dreimal je Stunde), die
EEPROM-Zeile rund 45 Byte je echtem Schreibvorgang in Richtung STM→ESP. Der Empfangsring des
STM ist **1'024 Byte** (`uart-driver.h`); die Checkliste nennt noch 256 und wird in Z.2
berichtigt.

**Unter 20 s Watchdog?** Ja, kürzer als heute: Ein Setter mit 64 geänderten Byte kostet heute
rund 1 s Busy-Wait **ohne** `watchdog_reload()`, nach C3 höchstens drei Seiten, rund 50 ms. Der
grösste Bereich (IR-Codes, Overlays) profitiert am meisten. Auf dem ESP sinkt die längste
Blockade im Wetterpfad von über 5 s auf höchstens 2,5 s.

**Wie lange blockiert der Hauptloop?** Je EEPROM-Schreibvorgang **Seitenzahl × 16 ms** statt
**geänderte Byte × 16 ms**, plus rund 0,1 ms je gelesenem Byte.

**Hartkodierte Grenzen?** `EEPROM_PAGE_SIZE` — mit Datenblattbeleg (AKC.1). Wächst
`eeprom-data.h`, bleibt die Rechnung richtig, weil sie nicht an Bereichsgrenzen hängt.
`WEATHER_TOTAL_TIMEOUT_MS` hängt an der 3-s-Wartezeit des STM; ändert sich diese, gehört die
Grenze nachgeprüft — das steht als Kommentar an der Konstante.

### Secure by design

**`innerHTML`?** Nicht berührt.

**Fremddaten?** Die Antwort des Wetterdienstes kommt über HTTP und bleibt nicht
vertrauenswürdig; die Lesehilfe begrenzt **Zeit**, nicht Länge — der Parser bleibt unverändert.
● E.5 prüft, ob die Lesehilfe eine Obergrenze für die Zeilenlänge braucht, damit ein feindlicher
oder umgeleiteter Server den Heap nicht erschöpft; heute hat `readStringUntil` keine. Fällt die
Antwort „ja" aus, ist das ein **eigener Befund**, nicht Teil dieses Pakets, es sei denn, die
Grenze entsteht ohne Mehraufwand in derselben Lesehilfe.

**Credentials in Logausgaben?** **Teil C beseitigt** `appid` und WLAN-Schlüssel aus dem
Debug-Echo und damit aus Mitschnitt und STM32-Logbuch der PWA. Die neuen Messzeilen enthalten
**keine** Werte (AKE.1, AKM.1).

**Neue schreibende Endpunkte?** Keine.

### Stable & reliable

**Fehler ausgewertet?** Der Wetterpfad unterscheidet künftig `ok`, `timeout`, `connfail`,
`leer` — heute verschwand eine verspätete Antwort ohne jede Spur. `eeprom_write()` behält die
strenge Richtung: unlesbar gilt als verschieden, ein Schreibfehler liefert 0.

**Leere `catch`?** Nicht berührt.

**Stille Verwerfungen?** `uart_rxdrops` zählt weiter; die Messzeile macht den Zusammenhang
EEPROM ↔ Verwurf erstmals **je Schreibvorgang** sichtbar. Die verspätete Wetterantwort wird
nicht mehr still verworfen.

**Still zurechtgebogen?** Nein. Ein Schreibfehler wird nicht als „gespeichert" gemeldet.

**Zustand nach Abbruch?** Wetter: Bei Fristablauf wird `stop ()` gerufen wie heute, nichts
geparst, die Messzeile nennt den Grund. EEPROM: Seiten-Präfix statt Byte-Präfix (§2.3); der
Aufrufer erhält 0 wie heute.

**Flags aufgelöst?** Keine neuen Flags.

---

## Verworfene Alternativen

**L338: STM wartet länger als 3 s.** Verschleiert jeden echten Ausfall und verlängert jede
Blockade um denselben Betrag; die Ursache liegt auf dem ESP.

**L338: asynchroner Wetterabruf.** Beseitigte auch die verbleibenden bis zu 2,5 s, ist aber ein
Umbau des Kommandozweigs in `loop()` mit eigenem Zustand und eigenem Risiko. Die gemessene
Ursache — 5 s Warten auf ein `\n`, das nie kommt — behebt die Gesamtfrist allein.

**L338: nur `setTimeout()` herabsetzen.** Bliebe bei `readStringUntil` und damit beim Warten
nach Verbindungsende; jede Zeile hätte ihre eigene Frist, die Summe wäre unbegrenzt.

**L339: grösserer Ring (d).** Notbehelf; verschiebt die Schwelle, die Blockade bleibt.

**L339: nur `write_update_host_to_eep()` und `…path…` kürzen.** Trifft zwei der Aufrufer; der
nächste lange Setter hätte dasselbe Problem.

**L339: ganze Seite schreiben statt nur den geänderten Abschnitt.** Gleiche Zyklenzahl, aber
mehr I2C-Verkehr und mehr Schreibvorgänge auf unveränderte Zellen; kein Vorteil.

**L339: ESP rahmt aufeinanderfolgende Setter** (in A58 als Möglichkeit genannt). Behandelt das
Symptom auf der anderen Seite; jeder künftige Absender müsste sich daran halten.

**Teil C: Setter ohne Wert, andere Anfragen vollständig.** Braucht eine Setterliste, die bei
jedem neuen Endpunkt nachgeführt werden muss; ein vergessener Eintrag gibt das Geheimnis wieder
aus. **Teil C: Parameternamen ohne Werte.** Mehr Code im Pfad jeder Anfrage, für eine
Information, die der Marker `?…` für die Fehlersuche ausreichend liefert.

**Messzeile und C3 im selben Release (V3).** Siehe Ent-3; nicht empfohlen wegen L204.

---

## Versionsfolgen

Je Einspielschritt, **nur die geänderte Komponente** (DIR-004). Testbauten heben keine Version
an.

| Schritt | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` |
|---|---|---|---|---|
| E (Teil A, Teil C) | — | ☐ | — | — |
| M (Messzeile) — entfällt bei Ent-3 = V3 | ☐ | — | — | — |
| C3 | ☐ | — | — | — |

Nur der `release-engineer` führt das aus (R4). Der Tag `release/<stm>-<esp>-<app>` wird **je
Einspielschritt** gesetzt und gepusht (DIR-011).
