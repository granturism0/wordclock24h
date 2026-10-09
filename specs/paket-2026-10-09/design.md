# Design — Paket 2026-10-09 „Brückenlast" (L338/C43, L339/A58, Prüfsumme ESP→STM)

**Erstellt:** 2026-10-09, Stand `dea187a`. **Freigegeben**, alle Entscheidungen getroffen
(Ent-1 bis Ent-8). Am selben Tag dreimal nachgeführt: mit Teil D, dann mit den Ergebnissen aus
V.1 und Ent-5 bis Ent-8, zuletzt mit dem Ergebnis des Reviews E.5 (§7: M1 Ergebnis `fehler`,
M2 als Befund ausgeklammert, N3, N4, Marker `?...`). Momentaufnahme (DIR-006).

**Schrittfolge:** **V (erledigt) → E (ESP: A1, C, D-ESP) → M (STM: Messzeile, A2) → C3 (STM,
seitenweise) → D (STM, Prüfsumme) → Z**.

---

## 0. Warum diese Reihenfolge

| Frage | Antwort |
|---|---|
| Warum ESP zuerst? | A1 liegt ganz auf dem ESP, und Teil C ändert, **was** über die Brücke kommt. Würde C nach der Vorher-Messung von L339 eingespielt, unterschieden sich Vorher und Nachher in **zwei** Dingen (R3b) |
| Warum A1, C und D-ESP in **einem** ESP-Release? | Sie berühren verschiedene Funktionen und verändern einander nicht; G1 trennt sie über verschiedene Zeilen (`- weather`, `- request`, `CMD`/Eröffnung). D-ESP hat ohne den STM-Teil **keine** Wirkung auf die Brücke (§5.2); sein Umbau ist byte-gleich geprüft (AKD.2) |
| Warum eine Messzeile **vor** C3? | DIR-014, und L204: Dort war die EEPROM-Spur die erste Erklärung und falsch (Ent-3 = V1) |
| Warum A2 im **M-Release**? | §1.6 — A2 wirkt nur, solange ein Wetterabruf läuft, und berührt `eeprom_write()` nicht; die Wirkungen von M bleiben über getrennte Zeilen und Zähler unterscheidbar |
| Warum D **nach** C3, als eigenes Release? | Ent-6: Beide wirken auf jede Speicheraktion; getrennt bleibt jede Wirkung zuordenbar |

---

## 1. Teil A — L338: Das Wetter kommt immer an (Ent-8)

### 1.1 Ausgangslage am Code und Messung

`query_weather()` (`ESP8266/ESP-uclock/weather.cpp:307-427`), aufgerufen aus **acht** Stellen in
`ESP-uclock.ino` (`:1517`, `:1525`, `:1583`, `:1591`, `:1649`, `:1657`, `:1715`, `:1723`).

| Zeile | Heute | Folge |
|---|---|---|
| `:353` | `connect (hostname, 80)` | DNS und Verbindungsaufbau blockieren, ✔ V.1 (5): **nicht** über `setTimeout()` begrenzt |
| `:359` | `delay (200)` fest | 200 ms Blockade auch bei schneller Antwort |
| `:361` | `if (available ())` | Antwort nach mehr als 200 ms ⇒ **nicht gelesen** (L-Befund 200 ms) |
| `:367-392` | Kopfzeilen per `readStringUntil('\n')`, Schleife nur solange `available ()` | Kopf in Teilsegmenten ⇒ Schleife endet zu früh |
| `:394-405` | erste Körperzeile; unter 10 Zeichen (Chunk-Länge) die nächste | wartet ohne `\n` bis zum Timeout (5 s), ✔ V.1 (5): `Stream::timedRead` prüft `connected()` nicht |
| `:407-417` | `parse_weather*()` | — |
| `:425` | bei `connect`-Fehler `ERROR connection to … failed` | die **einzige** Endzeile ohne Wetter |

**Messung vom 09.10.2026 (Mac):** DNS + Verbindung + Antwort **0,13 bis 0,24 s**. Die 5,25 s aus
S.26 sind fast ganz das Warten auf ein `\n`, das nie kommt. Und die 200-ms-Schwelle liegt **mitten
in der normalen Antwortzeit** — daher der eigene **L-Befund 200 ms**.

### 1.2 Stufe A1 (ESP): vollständig lesen

**Eine** Lesehilfe ersetzt die drei `readStringUntil`-Aufrufe und die `available ()`-Prüfungen —
keine zweite Variante daneben:

- liest Zeichen, solange **`available () || connected ()`** und die Gesamtfrist nicht abgelaufen
  ist; endet bei `\n`, bei **Verbindungsende** (der ESP sendet bereits `Connection: close`,
  `:356`) oder bei Fristablauf;
- **gibt in jedem Durchlauf ohne Zeichen die Kontrolle ab** (`yield ()` oder `delay (1)`),
  sonst schlägt der Soft-Watchdog des ESP zu (AKE.5).

`delay (200)` und das nackte `if (available ())` entfallen: Gewartet wird auf das erste Byte,
**solange die Verbindung steht**. Die **Parselogik bleibt unverändert** (Kopf bis zur Leerzeile,
höchstens 20 Zeilen; eine Körperzeile, unter 10 Zeichen die nächste). Endet der Körper ohne
`\n`, gilt das Verbindungsende als Zeilenende — der Abruf ist fertig, sobald der Dienst schliesst.

**Ohne Längengrenze je Zeile — bewusst ausgeklammert (Review E.5, M2, §7).**

### 1.3 Das Sicherheitsnetz: 5 s, ohne Doppelzählung

**`WEATHER_TOTAL_TIMEOUT_MS` = 5'000 ms**, gemessen ab dem Beginn von `query_weather()` —
**nur** für einen Dienst, der nicht antwortet oder nicht schliesst. Im Normalfall greift es nie.

**Getrennt begrenzen (V.1 (5)):** `connect (hostname, …)` löst den Namen intern auf und hat
**eigene** Wartezeiten; eine Frist davor und danach wirkte doppelt. Deshalb:

1. `WiFi.hostByName (hostname, ip, rest)` — Restfrist als DNS-Grenze; scheitert sie: `dns`.
2. `connect (ip, 80)` mit `setTimeout (rest)` **vorher** — scheitert sie: `connfail`.
3. `setTimeout (rest)` **vor** `print ()`, dann Lesen über die Lesehilfe bis zur Restfrist.
4. `stop ()` kostet rund **300 ms**; die Lesefrist endet entsprechend früher, damit die
   Gesamtdauer **5'000 ms + 100 ms** nicht überschreitet (AKE.2).

● Die genaue Signatur von `hostByName` mit Frist und das Verhalten von `connect (ip, …)` unter
`setTimeout` hängen an der Core-Version; der `esp-developer` belegt beides an der Core-Quelle.

### 1.4 Messzeile und Endzeile

**Messzeile** am Ende **jedes** Pfads:

```
- weather fc=<0|1> ms=<n> <ok|fehler|dns|connfail|timeout|leer>
```

Aufbau wie `esp_heap_log()` (`ESP-uclock.ino:483-511`): **einmal** per `snprintf` in einen
Stackpuffer, **zweimal** ausgegeben — `Serial.println` **und** `stm32_log_append ()`. Kein
`String` (L175). Keine `appid`, kein Ort, keine Koordinaten, keine URL. Bleibt im Fabrikat
(Ent-2).

**Die Ergebnisse, abschliessend (Nachtrag 09.10.2026, Review E.5 M1):**

| Ergebnis | Bedeutung | Endzeile |
|---|---|---|
| `ok` | Antwort gelesen, Parser hat die Endzeile in **Erfolgsform** gesendet: der Zweig `cod == 200` von `parse_weather()` bzw. `parse_weather_fc()` — `WEATHER Wetter heute: …`, `WEATHER_FC Wetter morgen: …`, `WICON <icon>`, `WICON_FC <icon>` | aus dem Parser |
| `fehler` | **Neu.** Antwort gelesen, Parser hat **genau eine** Endzeile gesendet, die aber einen **Fehler** meldet: `… Error <cod>` (cod ≠ 200, z. B. 401 bei falscher `appid`) oder `… Parse Error` (kein `cod` in der Antwort) | aus dem Parser, **unverändert** |
| `dns` | Namensauflösung gescheitert | `ERROR weather dns` |
| `connfail` | Verbindungsaufbau gescheitert | `ERROR weather connfail` |
| `timeout` | Sicherheitsnetz abgelaufen | `ERROR weather timeout` |
| `leer` | gelesen, aber keine Endzeile aus dem Parser (leerer Körper; Icon nicht gefunden) | `ERROR weather leer` |

**Warum `fehler` (M1):** Bis zum Review hiess `ok` nur „eine Endzeile ist entstanden". Eine
Antwort mit cod 401 oder ohne `cod` lieferte deshalb `ok`, und `tools/watch-log.sh` schlug nicht
an — genau der Fall, den die Messzeile sichtbar machen soll (falsche `appid`, Dienst liefert
Fehlerseite).

**Wie das Ergebnis entsteht:** Der Parser meldet es über seinen **Rückgabewert** —
drei Werte statt bisher zwei: keine Endzeile / Endzeile in Erfolgsform / Endzeile mit Fehler.
**Kein Textvergleich** der gesendeten Zeile. Die Namen der Werte wählt der `esp-developer`.
Die Zweige von `parse_weather()` und `parse_weather_fc()` bleiben in Reihenfolge und Ausgabe
**byte-gleich**; neu ist nur der Rückgabewert je Zweig. Ein `cod == 200`-Zweig, in dem `temp`
oder `description` fehlen, sendet heute trotzdem die Erfolgsform und zählt deshalb als `ok` —
die Parselogik bleibt unverändert (§1.2).

**Endzeile (neu, für A2):** Jeder Abruf sendet **genau eine** Zeile, an der der STM das Ende
erkennt: `WEATHER …`, `WEATHER_FC …`, `WICON …`, `WICON_FC …` (heute aus `parse_weather*()`,
`weather.cpp:196-299`) — oder, wenn keine davon entstand, **`ERROR weather <ergebnis>`**. Der STM
liest `ERROR …` schon heute als `ESP8266_ERROR`, ohne Folgeaktion. Ohne diese Zeile wartete A2 bei
jedem leeren Abruf die volle Frist ab. **Durch M1 ändert sich an der Endzeile nichts** — der STM
sieht bei `fehler` dieselben Bytes wie zuvor bei dem als `ok` gemeldeten Fehlerfall, und
`ERROR weather fehler` entsteht **nie** (die Fehler-Endzeile kommt ja aus dem Parser).

**`tools/watch-log.sh` braucht dafür keine Änderung:** Es meldet jedes Ergebnis ausser `ok`
(`watch-log.sh:149-150`, Muster `[a-z]+`), also auch `fehler`. Nur der Kommentar dort
(`:147`) nennt die Ergebnisliste ohne `fehler` — Sache des Leads, keine Prüfänderung.

### 1.5 Prüfstand t12

Vom `esp-developer` im Scratchpad gebaut, vom Lead unter `tools/checks/auszug/esp/t12/`
abgelegt. Auszug von `query_weather()` aus dem echten `weather.cpp`; nachgebildet: `WiFiClient`
mit **Core-Semantik** (`readStringUntil` wartet bis zum Timeout auch nach Verbindungsende),
`hostByName`, `millis ()`/`delay ()`/`yield ()` auf **simulierter Zeit**, `parse_weather*()` als
Rekorder, `Serial` als Rekorder der Endzeilen. Fälle: AKE.1 bis AKE.6b. **Gegenprobe gegen den
Ausgangsstand** über `auszug.sh --gegen`.

**Nachtrag M1:** Für AKE.6b bindet t12 die **echten** Parser ein (nicht den Rekorder), damit
der Rückgabewert und die unveränderten Endzeilen am selben Fall geprüft werden. **Gegenprobe**
gegen den Stand des Reviews E.5 (`95ba131`): Die Fälle cod 401 und „Parse Error" liefern dort
`ok` ⇒ FEHL.

### 1.6 Stufe A2 (STM): auf die Wetterantwort warten

**Ausgangslage, ✔ am Code:** `weather_query()` (`src/weather/weather.c:213-285`) sendet
`weather`, `weather_fc`, `wicon` oder `wicon_fc` über `esp8266_send_cmd()` und wartet nicht.
Kurz danach sendet der STM irgendein quittungspflichtiges Kommando — in S.26 `N10` —, und
`var_send_buf()` (`src/vars/vars.c:501-704`) gibt nach `VAR_SEND_TIMEOUT_SEC` = 3 s auf
(`:619`), zählt `v` und merkt die Zeile zur Nachsendung vor. **V.1 (6), ✔:** Bleibt `WEATHER`
aus, bleibt beim STM heute nichts hängen.

**Entwurf:**

- `weather_query()` merkt sich beim Senden den **Anstoss** (Zeitpunkt, Abruf läuft).
- `var_send_buf()` gibt, solange ein Abruf läuft, erst auf, wenn **beides** abgelaufen ist: die
  normalen 3 s ab dem eigenen Start **und** 6 s ab dem Anstoss. Die Warteschleife ruft ohnehin
  `schedule_esp8266_messages()`; trifft dort die Endzeile ein (`ESP8266_WEATHER`,
  `ESP8266_WEATHER_FC`, `ESP8266_WEATHER_ICON`, `ESP8266_WEATHER_FC_ICON`, `ESP8266_ERROR`), ist
  der Abruf beendet, und es gilt wieder die normale Wartezeit.
- **Frist 6 s**, mindestens **echte** 6 s: `uptime` zählt ganze Sekunden; der `stm-developer`
  wählt die Zeitbasis so, dass die Frist nicht zu kurz ausfällt (z. B. 7 Sekundenschritte oder
  eine Millisekundenbasis) und nennt sie im Bericht. **Herleitung:** Sicherheitsnetz des ESP
  5'000 + 100 ms, plus die Zeile über die Brücke — 6 s lassen Abstand.
- **Nach der Frist** ist der Abrufzustand gelöscht, auch ohne Endzeile — es bleibt **nichts
  hängen** (AKW.2). Eine Endzeile ohne Anstoss ändert nichts. Ein zweiter Anstoss während eines
  laufenden Abrufs verschiebt die Frist **nicht** (sonst könnte eine Kette von Abrufen das Warten
  beliebig verlängern).
- **Watchdog:** keine neue Aufrufstelle. 6 s Warten liegen weit unter den 20 s; nach der
  Quittung gilt der bestehende Reload im Rahmen von `VAR_SEND_RELOAD_BUDGET_SEC` (`vars.c:79`,
  `:698-701`).
- **Kein neues Logtext-Format**; die Wirkung zeigt sich am Ausbleiben von `v` und an der
  ESP-Messzeile.

**Warum im M-Release (R3b):** Der Vorschlag des Leads ist vertretbar. A2 und die EEPROM-Messzeile
teilen keine Funktion und keinen Zähler: A2 wirkt nur in `var_send_buf()` und nur, während ein
Wetterabruf läuft; die Messzeile wirkt nur in `eeprom_write()`. G2 (Setter-Burst auf den
Wetterort) stösst keinen Abruf an — `weather_set_city()` (`weather.c:174-180`) ruft
`weather_query()` nicht. Fällt in G2 zufällig ein automatischer Abruf, zeigt die ESP-Messzeile
ihn an, und G2 wird wiederholt. **Und:** Nach G1 ist A1 allein schon am Gerät nachgewiesen; A2
ist am Gerät nur als „unschädlich" belegbar (ein langsamer Dienst ist nicht herstellbar), seine
Wirkung belegt der Prüfstand W2. Ein eigenes Release dafür brächte keinen zusätzlichen
Gerätebeleg.

### 1.7 Prüfstand W2

Vom `stm-developer` im Scratchpad gebaut, vom Lead unter `tools/checks/auszug/stm/w2/` abgelegt.
Auszug von `var_send_buf()` — der Abschnitt, den der bestehende Tischprüfstand schon einbindet
(`vars.c:782`) — mit nachgebildeter Zeit und Nachrichtenfolge. Fälle aus AKW.1 und AKW.2.
**Gegenprobe:** gegen den E-Stand zählt der Fall „Abruf läuft, ESP schweigt 5 s, dann Quittung"
einen Timeout ⇒ FEHL.

---

## 2. Teil B — L339: EEPROM seitenweise schreiben (STM)

### 2.1 Ausgangslage am Code

- `eeprom_write (start_addr, buffer, cnt)` (`src/eeprom/eeprom.c:201-240`): je Byte ein
  `i2c_read()`; ist es gleich, weiter; sonst `i2c_write()` mit einem Byte und
  `eeprom_waitstates()` (15 bis 16 ms, `:38-57`). Lesefehler ⇒ schreiben; Schreibfehler ⇒
  `rtc = 0`, Abbruch.
- Aufgerufen über das Makro `eep_write()` (`src/eep/eep.h:39`); 55 Vorkommen in 15 Dateien, rund
  fünfzig Aufrufstellen. **Jede davon ist von C3 betroffen.** **V.1 (2): keine periodischen.**
- `i2c_write()` und `i2c_read()` (`src/i2c/i2c.c:374-478`) übertragen beliebig viele Byte.
- Adressierung 16 Bit; 4'096 Byte.
- Die Bereiche liegen lückenlos hintereinander (`src/eeprom/eeprom-data.h:174-225`) und sind
  **nicht** auf Seiten ausgerichtet.

### 2.2 Stufe 1 (Schritt M): die Messzeile

In `eeprom_write()`, **ohne jede Verhaltensänderung**:

```
eep a=<start_addr> n=<cnt> z=<schreibzyklen> ms=<dauer> d=<rxdrops vorher>/<rxdrops nachher>
```

- **Zeitbasis unabhängig von `eeprom_waitstates()` (V.1 (3)).** `eeprom_waitstates()` zählt
  `eeprom_ms_tick`; misst die Zeile mit demselben Zähler, ergibt sie per Konstruktion
  `ms ≈ 16·z`, und die Selbstprüfung `15·z ≤ ms ≤ 17·z + 10` kann **nie** fehlschlagen — eine
  Prüfung ohne Gegenstand (DIR-014). Vorschlag: `diag_tick_cnt` (`src/main.c:655`, `:817`,
  jeder Zeitgeber-Interrupt), über `F_INTERRUPTS` in Millisekunden umgerechnet; dafür braucht es
  einen Lesezugriff von aussen. Die Auflösung nennt der Bericht; ist sie gröber als 1 ms, wird die
  Toleranz in AKM.2 um sie erweitert.
- `z` zählt die Aufrufe von `eeprom_waitstates()`.
- `d` über `esp8266_uart_rxdrops()` (`src/main.c:362`, `:3896`). Die Abhängigkeit ist bewusst
  und im Kommentar begründet; die **Aufrufstellen bleiben unberührt**.
- Ausgabe bei `z ≥ 1`, **ohne weitere Schwelle** (V.1 (2)).
- **Kein Inhaltsbyte.** `log_printf()` geht über die Log-UART **und** als `LOG …` zum ESP.
- Ein Füllstand des Rings ist **erwünscht, nicht Pflicht** (höchstens rund 40 Byte Flash).
- **Bleibt im Fabrikat** (Ent-2).

### 2.3 Stufe 2 (Schritt C3): seitenweise

```
EEPROM_PAGE_SIZE   32 auf F411 (V2), 8 auf F103      // Build-Konstante je Ziel, Ent-7

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

**Seitengrösse je Ziel (Ent-7):** Die Unterscheidung läuft über das bestehende Zielmakro
(`STM32F4XX` / `STM32F10X`, wie in `src/i2c/i2c.c:257-264`). Auf F103 sind es 8 Byte — **kleiner
ist immer sicher**, nur langsamer; der Nutzer hat diese Grösse festgelegt, weil der Baustein des
F103-Aufbaus nicht belegt ist.

**Erhalten bleibt:** Unveränderte Bytes kosten keinen Schreibzyklus, jetzt **je Seite**;
**unlesbar gilt als verschieden**. **Zustand nach Abbruch:** Seiten-Präfix statt Byte-Präfix, der
Aufrufer erhält 0 wie heute. **Kosten:** ein Puffer von P Byte auf dem Stack.

**Kommentar in `eeprom.c`** (`:180-200`) auf „je Seite" nachgeführt; die historische Messung
bleibt als solche kenntlich.

### 2.4 Tragweite und Risiko — ausdrücklich

**C3 ändert jeden EEPROM-Schreibvorgang der Uhr.** Ein Fehler in der Seitenrechnung ist
**Datenverlust**: Eine Transaktion über das Seitenende hinaus bricht **auf den Anfang derselben
Seite** um und überschreibt dort fremde Einstellungen — **still**, denn `i2c_write()` meldet
Erfolg. Trifft das die Update-Quelle, ist es der Weg aus L42. Ist die Seitengrösse grösser als
die echte, entsteht genau dieser Überlauf. **Abgefangen wird das durch den Prüfstand** (beide
Seitengrössen, Umbruch wie die Hardware, Vergleich des ganzen Abbilds nach jedem Aufruf); die
Integritätsprüfung am Gerät (AKC.7) ist die zweite Linie, und nur für F411.

### 2.5 Prüfstand C3

Vom `stm-developer` im Scratchpad gebaut, vom Lead unter `tools/checks/auszug/stm/c3/` abgelegt.
Zwei Auszüge von `eeprom_write()` — **alt** (M-Stand, mit nachgebildetem Log) und **neu**, je
**für P = 32 und P = 8** übersetzt — gegen ein nachgebildetes EEPROM:

- 4'096 Byte, 16-Bit-Adresse; **Schreiben bricht am Seitenende des Bausteins auf den
  Seitenanfang um**, Lesen läuft über Seitengrenzen und bricht am Speicherende um.
- Protokoll jeder Transaktion und jedes Wartezyklus; Fehlereinspeisung für Lesen und Schreiben.
- **Bereiche aus den echten Headern**, soweit übersetzbar; die Zufallsfolge über den ganzen
  Adressraum ist **unabhängig davon Pflicht**.
- **Drei Sabotagen** im Scratchpad: Firmware-Seitengrösse 64 gegen Baustein 32; Firmware 32
  gegen Baustein 8; Seitenende um eins verschoben.
- Typbreiten des Ziels (L256). Ausgabe: Fallzahl je Seitengrösse, Zahl der Vergleiche, Zyklen
  alt/neu (AKC.5). **Zähl nach** (DIR-014).

---

## 3. Teil C — Debug-Echo ohne Querystring (ESP)

Eine Funktion gibt die Echo-Zeile aus; **beide** Stellen rufen sie (`http.cpp:14071-14079` POST,
`:14129-14143` GET):

```
- request <IP> [<label>]: GET /api/weather_city_set?...
- request <IP> [<label>]: GET /app/
```

Methode und Pfad bleiben, alles ab `?` wird zu `?...` — **drei ASCII-Punkte**, nicht das
Auslassungszeichen `…`, weil die Zeile über die Brücke geht und auf der Log-UART landet
(Nachtrag Review E.5; DIR-015). Die HTTP-Version entfällt. **Eine Regel
für alle Anfragen** — keine Setterliste (Gattung L179). `tools/watch-log.sh:128` zeigt vor einer
`Exception (29)` weiterhin den letzten Endpunkt. E.2 meldet weitere Stellen, die Anfragewerte
über `Serial` ausgeben — **nicht mitkorrigiert**.

**Prüfstand t13** und eine **statische Prüfung** (kein `Serial.print*` mit `sRequest`/`sParam`).
Die statische Prüfung trägt die Gegenprobe; t13 ist gegen den Ausgangsstand nicht übersetzbar —
**kein** Fehlschlag im Sinn von DIR-014.

---

## 4. Gerätenachweise G1 bis G3 und die Wetterabrufe

### 4.1 G1 — A1, Teil C und D-ESP gegen den alten STM (nach E.8)

`watch-log.sh` läuft. Je **ein** `weather_get_now` und `weather_get_forecast`, mindestens 10 s
Abstand: Messzeile (Mitschnitt **und** `/api/stm32_log`), `ms` unter 1'000, gleich dem Abstand
`weather`→`WEATHER` auf ±150 ms, Endzeile vorhanden, `v` im 6-s-Fenster unverändert. Danach
**60 Minuten lesend**, mit Median und grösstem Wert der Abrufdauer. `- request`-Zeilen ohne `=`.
**Teil D, lesend:** Eröffnungen ohne `0x04`, nur `CMD`-Zeilen, keine Logzeile
`- cmd Faehigkeit an` — **Suchtext in Umschrift**, so wie der ESP sie schreibt
(`ESP-uclock.ino:758`, DIR-015); eine Suche nach „Fähigkeit" fände nie etwas und bestünde damit
immer.

### 4.2 G2 / G3 — L339, Setter-Burst auf dem Wetterort

**Warum der Wetterort:** Er liegt im EEPROM (`src/weather/weather.c:89-100`, 32 Byte, mit Nullen
aufgefüllt), ist harmlos, der ESP nimmt ihn mit bis zu 32 Byte an (`http.cpp:8644-8678`), und
`weather_set_city()` stösst **keinen** Abruf an. **Nicht** Update-Host oder -Pfad.

**Ablauf:**

1. **Vorbedingung:** Originalwert des Wetterorts und der Koordinaten lesend festhalten. Sind Ort
   **und** Koordinaten leer: **Halt**, anderes Ziel mit dem Nutzer klären
   (`http.cpp:8662-8668`).
2. Sieben `weather_city_set` in rund 1,5 s, abwechselnd ein 32-Byte-Testwert aus
   ASCII-Buchstaben und ein 1-Byte-Testwert.
3. Mindestens 4 s warten, Originalwert zurückschreiben.
4. STM zurücksetzen (`maintenance_reset_stm32`); danach den Wetterort lesend prüfen.

**Fällt ein automatischer Wetterabruf in den Burst**, zeigt ihn die ESP-Messzeile; G2 bzw. G3
wird dann wiederholt.

**Warum `d` im Vorher nicht steigen muss:** S.26 lief mit 64-Byte-Werten und langem Echo; G2 mit
32-Byte-Werten und gekürztem Echo. **Tragend ist die Messzeile**; bleibt `d` bei 0, folgt die
Wiederholung mit 14 Aufrufen. Bleibt `d` auch dann bei 0, steht im Bericht, dass der
Überlauf-Teil von L339 über S.26 und AKC.5 belegt ist.

**Nachher (G3, F411):** `z ≤ 2`, `ms ≤ 60`, `d` unverändert. G2 und G3 laufen **vor** dem
STM-Teil von D — G3 misst allein C3.

### 4.3 Wetterabrufe nach M (A2)

Wie G1: je ein Abruf jeder Art und 60 Minuten lesend. Erwartet: weiterhin kein `v`-Anstieg nach
einem Abruf, kein Watchdog-Reset. **Mehr ist am Gerät nicht zu zeigen** — ein Dienst, der 5 s
braucht, lässt sich nicht herstellen; die Wirkung von A2 belegt W2.

---

## 5. Teil D — Prüfsumme für Kommandos ESP→STM

### 5.1 Was in der Gegenrichtung steht, und was übertragbar ist

| Baustein (A32, STM→ESP) | Fundstelle | Übertragbar? |
|---|---|---|
| Rechnung `var_crc()`, Länge als Startwert | STM `src/vars/vars.c:163` (static), ESP `ESP-uclock.ino:560-576` (static); Schiedsrichter `tools/checks/var-crc.c` | **Ja, unverändert** — ausserhalb der Datei sichtbar gemacht, **keine dritte** |
| Marke `*hhhh`, strenge Hexprüfung (`var_crc_hexval()`, nicht `htoi()`, L232) | STM `vars.c:532-538`; ESP `:583-602` | **Ja, gleiches Format** |
| Fähigkeitsmeldung, festgeschrieben mit der Sitzung | `esp8266.c:190-214`, `:471-480`, `:604-609`, `:823-859` | **Das Muster ja, der Träger nicht** (§5.2) |
| Gedrosselte Abweisungszeile mit laufendem Zähler | ESP `var_crc_reject()` (`:623-638`) | **Ja**, auf dem STM über `log_printf()` |
| `!v` und Nachsendung | ESP, STM `vars.c` | **Nein** — ersetzt durch N1 (§5.4) |
| Markenpflicht mit Selbstauflösung (H1) | `ESP-uclock.ino:746-781` | **Entfällt** durch das eigene Präfix (§5.3) |

### 5.2 Wie der ESP die Fähigkeit lernt — und verliert

**Träger: Flag `0x04` in der Eröffnung `var VBffnn`** (`vars.c:1750`), „dieser STM prüft
markierte Kommandos". Eine eigene Zeile `CAP …` STM→ESP wäre die Falle aus **L266**: unbekannte
Top-Level-Zeile, unquittiert, 3 s Leerlauf je Zeile ohne `watchdog_reload()`. Die Eröffnung ist
eine reguläre, quittierte, nachgesendete `var`-Zeile, und der heutige ESP **duldet unbekannte
Flag-Bits** (✔ `ESP-uclock.ino:894-916`).

**Lernen bei jedem STM-Neustart:** Der STM setzt beim Start den ESP zurück (`esp8266.c:974-989`);
der ESP startet ohne Fähigkeit und lernt sie beim Vollabgleich. **Verlorene Eröffnung ⇒ kein
Markieren** — die sichere Richtung, kein Dauerzustand.

**Nach einem reinen ESP-Neustart (Review E.5, N4) — gewollt, kein Fehler:** Startet nur der ESP
neu (OTA, Absturz, Watchdog des ESP), ohne dass der STM neu startet, beginnt der ESP nach Weg (a)
ohne Fähigkeit und sendet `CMD` **ohne** Prüfsumme, bis der STM das nächste Mal einen
Vollabgleich eröffnet. Das ist die sichere Richtung aus diesem Abschnitt: Der STM nimmt `CMD` wie
heute an. In G1 und G4 ist ein solches Fenster deshalb **kein** Befund; es wird nur getrennt
berichtet, wenn es in einen Gerätelauf fällt.

**Verlieren — vier Wege:** (a) ESP-Start; (b) Eröffnung ohne `0x04`; (c) **vor** jedem STM-Reset
und (d) **vor** jedem STM-Flash, den der ESP auslöst (Liste aus V.1 (7)). (c) und (d) stehen da,
damit die Sicherheit nicht allein am Reset-Puls hängt — „Ein Mechanismus, dessen Sicherheit an
einem Hardware-Nebeneffekt hängt, ist nicht abgesichert" (Paket 2026-10-05, Design §6.5).

**Verbleibende Lücke, benannt:** STM ohne Abgleichrahmen (vor Runde S), aufgespielt **ohne** ESP
(ST-Link), **und** ein Reset-Puls, der den ESP nicht erreicht. Auch dann gilt §5.3: Der alte STM
verwirft die Zeilen, er **speichert keine Marke**.

### 5.3 Das Format: `CMC <nutzlast>*hhhh`

Ohne Fähigkeit sendet der ESP `CMD <nutzlast>` byte-gleich wie heute. Mit Fähigkeit sendet er
**jedes** Kommando als `CMC <nutzlast>*hhhh`; `hhhh` = `var_crc()` über die Nutzlast ohne Präfix
und Marke, Länge als Startwert.

**Warum ein eigenes Präfix:**

1. **Der Fehlfall wird harmlos.** Ein alter STM verwirft `CMC` (`ESP8266_UNSPECIFIED`,
   `esp8266.c:638-641`), statt `meinhost*c328` zu speichern (R-3). Ein ausbleibendes Kommando
   fällt auf, ein still beschädigter Wert nicht (L205).
2. **Keine Mehrdeutigkeit:** `CMD` ist **nie** markiert, `CMC` **immer**; der Grenzfall
   „Nutztext auf `*hhhh`" (`ESP-uclock.ino:654-659`) entfällt.
3. **Keine Markenpflicht, keine H1-Falle:** Eine `CMC`-Zeile ohne Marke ist trotzdem als
   markierungspflichtig erkennbar.

**Die EEPROM-schreibenden Kommandos, besonders Update-Host und -Pfad, sind ohne eigene Liste
erfasst:** Der ESP markiert **alle**. **V.1 (8), ✔:** Kein bestehendes Präfix in `esp8266.c`
überdeckt `CMC `, und `ESP8266_UNSPECIFIED` ist im Hauptloop und in Tetris/Snake folgenlos.

### 5.4 Was der STM mit einer `CMC`-Zeile tut (Ent-5 = N1)

1. **Prüfen:** Marke formal gültig, Rechnung gleich?
2. **Richtig:** Marke ab, Nutzlast nach `u.cmd`, **derselbe** Weg wie `CMD`.
3. **Falsch, fehlend, formal ungültig:** **nicht** anwenden; Zähler sättigend; gedrosselte Zeile
   `cmd abgewiesen #<n> len=<l>` über `log_printf()` (die ersten vier einzeln, danach jede
   fünfzigste, kein Wert); **Vollabgleich vormerken** über `var_sync_pending`
   (`src/main.c:1487`, ausgeführt `:3957-3971`), **höchstens einer je 60 s**.

**Warum N1:** Der ESP setzt den neuen Wert vor dem Senden. Ohne N1 meldete er der PWA den neuen,
der STM hielte den alten — bis zum nächsten Vollabgleich, der den ESP still zurückzieht; beim
Update-Host die Abweichung aus L42. Mit N1 sind beide binnen Sekunden gleich, und die PWA zeigt
beim nächsten Lesen den alten Wert. **Nachgesendet wird nicht.** Gedrosselt, weil ein
Vollabgleich rund 200 quittierte Zeilen sind und eine Abweisung meist aus einer überlasteten
Brücke kommt (L109).

**Verworfen:** N3 (gezielte Nachsendung), (a) Quittung je Kommando — Bewertung in der vorigen
Fassung, Entscheidung Ent-5.

### 5.5 Zählen — warum nicht in der `diag`-Zeile

Die `diag`-Zeile ist voll (`main.c:3802-3814`). Der Zähler steht in der Abweisungszeile selbst;
`watch-log.sh` meldet jede (E.4). ● **Nebenbefund, gemeldet:** Die Längenrechnung
(`main.c:3802-3808`) kennt das später angehängte Feld `a=` (`:3887`, `:3900`) nicht; im
ungünstigsten Fall wird die Grenze von 119 Zeichen überschritten.

### 5.6 Wer was ändert

| Seite | Datei | Änderung | Task |
|---|---|---|---|
| ESP | `vars.cpp`, `udpsrv.cpp`, weitere Stellen aus V.1 (7) | **ein** Absender für alle Kommandoformen; ohne Fähigkeit `CMD …`, mit `CMC …*hhhh` | E.2b |
| ESP | `ESP-uclock.ino` | `var_crc()` sichtbar (ohne `static`, L177); `0x04` lernen und verlieren; Logzeile | E.2b |
| ESP | Reset- und Flash-Pfade aus V.1 (7), u. a. `http.cpp`, `stm32flash.cpp` | Fähigkeit **vor** dem Reset bzw. Flash löschen | E.2b |
| STM | `src/vars/vars.c`, `vars.h` | `var_crc()` sichtbar; Eröffnung setzt `0x04` | D.1 |
| STM | `src/esp8266/esp8266.c` | Zweig `CMC`: Prüfung, Abweisung, Zähler, Zeile | D.1 |
| STM | `src/main.c` | Vormerken über `var_sync_pending`, höchstens einmal je 60 s | D.1 |

### 5.7 Flash-Gate für M, C3 und D

**Lage:** 1'688 Byte frei, Reserve **1'024 Byte**, nutzbar **664 Byte**.

| Teil | untere | obere | Quelle |
|---|---|---|---|
| M — Messzeile **und** C3 | 130 | 340 | **V.1 (4)** |
| A2 — Wetterwarten (im M-Release) | 30 | 80 | ● Schätzung des `spec-writer`: ein Zeitstempel, ein Flag, zwei Vergleiche, Löschen bei fünf Nachrichtenarten; kein Text |
| D — Prüfung, Abweisung, `0x04`, N1 | 150 | 300 | **V.1 (4)** |
| **Summe** | **310** | **720** | **Die Obergrenze überschreitet 664 um rund 56 Byte.** |

**Reihenfolge und Regel:** Testbau nach M.1, nach M.1b (A2), nach C3.1 und nach D.1; jeder Teil
ist eine Bestellung des Nutzers ⇒ bei Unterschreitung der Reserve **anhalten und vorlegen**.
Gegenprobe S8b in M.5, C3.6 und D.6, kein Rollout unter 1'024 Byte.

**Sparstufen, in dieser Reihenfolge vorzuschlagen** (keine ohne Nutzer): (1) Füllstand in der
Messzeile weglassen; (2) Texte der Messzeile und der Abweisungszeile kürzen; (3) A2 über
`uptime` statt einer Millisekundenbasis (gröber, dafür ohne neuen Zugriff); (4) Messzeile **nach**
G3 entfernen — widerspricht Ent-2, braucht eine neue Entscheidung.

### 5.8 Prüfstände t14 (ESP) und D (STM)

**t14** — `tools/checks/auszug/esp/t14/`: ohne Fähigkeit jede Kommandoform byte-gleich (AKD.2);
mit Fähigkeit `CMC`, Marke gegen `tools/checks/var-crc.c` (AKD.3); Lernen und Verlieren auf allen
Wegen, verlorene und nachgesendete Eröffnung (AKD.4). **Gegenprobe:** Ausgangsstand ohne
markierte Zeile ⇒ FEHL.

**D** — `tools/checks/auszug/stm/d/`, Auszug des Zeilenparsers aus `esp8266.c` und von
`var_crc()`:

| Fall | Erwartet | Gegen den C3-Stand |
|---|---|---|
| `CMC` mit richtiger Marke | angewandt, Nutzlast gleich `CMD` | **FEHL** — Gegenprobe |
| `CMC` falsch / ohne Marke / drei Hexziffern / Nicht-Hex / verkürzt | abgewiesen, Zähler +1 | nicht angewandt |
| zwei `CMC`-Zeilen verschmolzen, jeder Schnitt | abgewiesen, keine beginnt mit `CMD ` | nicht angewandt |
| `CMD` ohne Marke; `CMD` mit Text auf `*hhhh` | angewandt wie heute | angewandt |
| längste Nutzlast (`S` + 63 Byte) + Marke | passt in `ESP8266_MAX_CMD_LEN` (127) | — |
| zwei Abweisungen in 10 s; zwei im Abstand von 61 s | ein bzw. zwei Vormerkungen | — |
| **alter STM**: `CMC` gegen den **Ausgangsstand** | `ESP8266_UNSPECIFIED`, nicht angewandt | — soll **bestehen** (AKD.8) |

### 5.9 G4 — Gerätenachweis, lesend

In G1: keine Fähigkeit, nur `CMD`. Nach dem D-Flash: Eröffnung mit `0x04`, Logzeile
`- cmd Faehigkeit an` (Suchtext in Umschrift, §4.1). Im `pwa-tester`-Durchlauf: jede
Speicheraktion als `(CMC …*hhhh)`, Phase 9 angewandt, null Abweisungen. **Die Abweisung belegt
der Prüfstand.** Ein Fenster mit `CMD` nach einem reinen ESP-Neustart ist gewollt (§5.2, N4) und
wird getrennt berichtet, nicht als Fehlschlag gewertet.

---

## 6. Ergebnisse aus V.1 (eingearbeitet)

| Frage | Ergebnis | Wirkung in dieser Spec |
|---|---|---|
| (1) Seitengrösse | durch den Nutzer entschieden: 32 auf F411, 8 auf F103 (Ent-7) | §2.3, AKC.1, Prüfstand mit beiden |
| (2) periodische Schreiber | **keine** | AKM.1 ohne Schwelle |
| (3) Zeitbasis | die Messzeile muss **unabhängig** von `eeprom_waitstates()` messen | §2.2, AKM.2 |
| (4) Flash | M + C3 ≈ 130–340 Byte; D 150–300 Byte | §5.7 |
| (5) ESP-Core | `setTimeout()` begrenzt DNS und `connect()` nicht; `timedRead` prüft `connected()` nicht | §1.3: DNS und Verbindung getrennt begrenzen |
| (6) STM ohne `WEATHER` | nichts bleibt hängen | AKW.2: muss mit A2 so bleiben |
| (7) `CMD`-Absender, Reset-/Flash-Pfade | Liste an E.2b | AKD.1, AKD.2, AKD.4 |
| (8) Präfix `CMC`, `ESP8266_UNSPECIFIED` | keine Kollision, folgenlos | §5.3 |

---

## 7. Ergebnis des Reviews E.5 (eingearbeitet, 09.10.2026)

**Bestanden**, mit zwei Befunden mittlerer Schwere. Entscheidungen des Nutzers:

| Punkt | Inhalt | Entscheidung | Wirkung in dieser Spec |
|---|---|---|---|
| **M1** | `ok` hiess nur „eine Endzeile ist entstanden", auch bei `… Parse Error` und `… Error <cod>`; die Logwache schlug dann nicht an | **Jetzt mitnehmen** | §1.4: neues Ergebnis `fehler`, gemeldet über den Rückgabewert des Parsers, Endzeile byte-gleich; AKE.1, AKE.6, **AKE.6b**; Nachtrag in E.1 |
| **M2** | `weather_read_line()` hat **keine Längengrenze je Zeile**. Ein feindlicher Server — nur über MITM oder DNS-Fälschung erreichbar, weil Klartext-HTTP — könnte in den 4,7 s Arbeitsfrist den Heap (rund 9 KB frei) ausschöpfen. **Keine Regression**: `readStringUntil` war genauso | **Später, als Befund** | **Ausdrücklich ausgeklammert** (requirements „Nicht Teil dieser Änderung"). Befund in `BEFUNDE.md`, Nummer vergibt der Lead. Eine Grenze setzt eine **Messung der grössten echten Vorhersageantwort am Gerät** voraus — ohne sie schnitte eine Grenze womöglich gültige Vorhersagen ab |
| **N3** | Die Logzeile heisst `- cmd Faehigkeit an/aus` (Umschrift, DIR-015); G4 und AKD.10 suchten „Fähigkeit an" | nur vermerken | Suchtext in §4.1, §5.9, AKD.10, E.9, D.8 auf `Faehigkeit an` |
| **N4** | Nach einem reinen ESP-Neustart sendet der ESP `CMD` ohne Prüfsumme, bis der STM wieder einen Vollabgleich eröffnet | nur vermerken: **gewollt** | §5.2, §5.9, AKD.10 — kein Fehler in G1/G4 |
| Marker | `?...` statt `?…`, weil ASCII auf der Brücke | nur vermerken | §3, AKE.8, E.2 |

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Schritt |
|---|---|---|---|
| `ESP8266/ESP-uclock/weather.cpp` | A1: Lesehilfe, DNS/Verbindung getrennt begrenzt, 5 s Sicherheitsnetz, Messzeile, Endzeile; **Nachtrag M1:** Ergebnis `fehler` über den Rückgabewert der Parser | `esp-developer` | E |
| `ESP8266/ESP-uclock/http.cpp` | Echo über eine Funktion; Fähigkeit vor STM-Reset löschen | `esp-developer` | E |
| `ESP8266/ESP-uclock/vars.cpp`, `udpsrv.cpp` | ein Kommandoabsender, `CMC` nach Fähigkeit | `esp-developer` | E |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | `var_crc()` sichtbar; `0x04`; Logzeile | `esp-developer` | E |
| `ESP8266/ESP-uclock/stm32flash.cpp` | Fähigkeit vor dem Flash löschen | `esp-developer` | E |
| `src/eeprom/eeprom.c` | M: Messzeile; C3: seitenweise, P je Ziel | `stm-developer` | M, C3 |
| `src/weather/weather.c`, `src/vars/vars.c` (ggf. `src/main.c`) | A2: Anstoss merken, verlängertes Warten, Endzeile löscht | `stm-developer` | M |
| `src/main.c` | Lesezugriff auf `diag_tick_cnt` für die Messzeile (M); Vormerken mit Drossel (D) | `stm-developer` | M, D |
| `src/vars/vars.c`, `vars.h` | `var_crc()` sichtbar; Flag `0x04` | `stm-developer` | D |
| `src/esp8266/esp8266.c` | Zweig `CMC` | `stm-developer` | D |
| `tools/checks/auszug/esp/t12/`, `t13/`, `t14/`, `stm/w2/`, `stm/c3/`, `stm/d/`, `auszug.sh` | Prüfstände; zwei statische Prüfungen; **t12 um AKE.6b erweitert** (Nachtrag M1) | Lead | E, M, C3, D |
| `tools/watch-log.sh` | Wetter-Ergebnis ≠ `ok` oder `ms` ≥ 1'000, `eep … ms` ≥ 200, `cmd abgewiesen`. **`fehler` braucht keine Änderung** — es ist ≠ `ok` | Lead | E |
| `knowledge/architecture-checklist.md` | EEPROM-Kosten je Seite; Ringgrösse mit Verweis; Frage nach dem zentralen Kommandoabsender | `doc-writer` | Z |
| `BEFUNDE.md` | L338/C43, L-Befund 200 ms, L339/A58, C3, Befund Teil D, Befund M2 (Längengrenze), Nebenbefunde | `doc-writer` | Z |
| `CLAUDE.md` | „Offene technische Themen" Nr. 2, Hardware-Abschnitt („16 ms pro Byte") | Lead | Z |
| Versionsdateien | ESP (E), STM (M, C3, D) | `release-engineer` | je Schritt |

**Kodierung und Zeilenenden vor jedem Patch feststellen** (DIR-015). `ESP-uclock.ino` hat
**gemischte** Zeilenenden.

---

## Prüfung gegen die Architektur-Checkliste

### Proper architecture

**PWA parallel zu Legacy?** Ja, beide unberührt; Legacy-Setter laufen über denselben
Kommandoabsender und sind gleich geschützt.

**Wetter-Endpunkte?** `/api/weather_get_now` und `/api/weather_get_forecast` bleiben der Weg;
geändert wird, **wie** der ESP liest und **wie lange** der STM während eines Abrufs auf Quittungen
wartet. Kein Rückbau auf `/weather?action=…`.

**Restore-Bedingung um `pending_ticker_restore`?** Nicht berührt. A2 hält nur die Quittungswartezeit
offen, nicht den Wetterticker. Ein Abruf mit `ERROR weather …` liefert keinen Ticker — derselbe
Zustand wie heute bei einem Verbindungsfehler; V.1 (6): kein Flag bleibt offen. Das Ergebnis
`fehler` ändert die Endzeile nicht und damit auch nichts an der Restore-Bedingung.

**Nur `.gz`?** Nicht berührt.

**Richtige Schicht?** L338 an **beiden** Enden, wo die Ursache sitzt: Der ESP wartete auf ein
`\n`, das nie kommt (A1), und der STM gab bei einem lebenden, aber beschäftigten ESP auf (A2). A2
allein verlängerte die Blockade nur; A1 allein liesse einen langsamen Dienst wieder `v` kosten.
L339 in `eeprom_write()`; das Echo, wo es entsteht; die Prüfsumme an der Leitung. Das Ergebnis
`fehler` entsteht im Parser, der den Fehler erkennt, und nicht durch einen Textvergleich danach.

### Scalable systems

**STM-Kommandos?** Keine zusätzlichen. N1 kann einen Vollabgleich auslösen, höchstens einen je 60 s.

**Byte je Minute?** **Weniger durch C**; **mehr durch D** (5 Byte je Kommando, weniger als C am
selben Burst spart); Wetterzeile rund 35 Byte je Abruf; Endzeile `ERROR weather …` nur bei
Fehlschlag; EEPROM-Zeile rund 45 Byte je Schreibvorgang. Der Empfangsring ist **1'024 Byte**.

**Unter 20 s Watchdog?** Ja. A2 verlängert das Warten von `var_send_buf()` auf **höchstens 6 s**,
nur während eines Abrufs und ohne neuen Reload — weit unter 20 s. Nach C3 kostet ein Setter
höchstens einige Seiten statt rund 1 s Busy-Wait. Auf dem ESP sinkt die längste Blockade im
Wetterpfad im Normalfall von über 5 s auf rund 0,2 s; das Sicherheitsnetz bleibt bei 5 s.

**Wie lange blockiert der Hauptloop?** EEPROM: **Seitenzahl × 16 ms**. **A2, ausdrücklich:**
Während eines Abrufs kann `var_send_buf()` den Hauptloop bis zu 6 s halten statt 3 s — aber nur,
wenn der ESP so lange schweigt, und der Normalfall dauert 0,2 s. Der Preis ist bewusst: ein
längeres Warten auf einen **lebenden** ESP statt eines verlorenen Einmal-Werts (L42).

**Hartkodierte Grenzen?** `EEPROM_PAGE_SIZE` je Ziel (Ent-7); `WEATHER_TOTAL_TIMEOUT_MS` (5 s)
und die A2-Frist (6 s) hängen aneinander — Kommentar an beiden; die längste markierte Nutzlast
muss in `ESP8266_MAX_CMD_LEN` passen. **Keine** Zeilenlänge in `weather_read_line()` — M2, §7.

### Secure by design

**`innerHTML`?** Nicht berührt.

**Fremddaten?** Die Wetterantwort bleibt nicht vertrauenswürdig. **E.5 (5) ist beantwortet:**
Eine Längengrenze je Zeile fehlt; ein feindlicher Server könnte den Heap ausschöpfen, aber nur
über MITM oder DNS-Fälschung, und der Ausgangsstand verhielt sich gleich. **Ausgeklammert als
Befund (M2, §7)**, weil eine Grenze erst nach einer Messung der grössten echten
Vorhersageantwort am Gerät gesetzt werden kann. Der STM prüft die Marke streng und das Präfix
exakt. **Die Prüfsumme ist kein Sicherheitsmerkmal**, nur ein Schutz gegen Übertragungsfehler. A2
wartet nur auf Zeilenarten, nicht auf Inhalte; ein ESP, der nie antwortet, kostet höchstens 6 s
je Abruf.

**Credentials in Logausgaben?** Teil C beseitigt sie aus dem Echo. Mess-, End- und
Abweisungszeilen enthalten **keine** Werte. ● Die STM-eigene Ausgabe `(CMD …)` mit Werten auf
der Log-UART bleibt — gemeldet.

**Neue schreibende Endpunkte?** Keine.

### Stable & reliable

**Fehler ausgewertet?** Wetter: `ok`, `fehler`, `dns`, `connfail`, `timeout`, `leer` und
**immer** eine Endzeile. **Seit M1** ist ein Dienst, der mit einer Fehlerseite antwortet
(cod ≠ 200, „Parse Error"), von einem Erfolg unterscheidbar, und die Logwache meldet ihn.
EEPROM: unlesbar gilt als verschieden, Schreibfehler liefert 0. Kommandos: falsche Marke ⇒ nicht
angewandt.

**Leere `catch`?** Nicht berührt.

**Stille Verwerfungen?** Eine verspätete Wetterantwort wird nicht mehr still verworfen
(L-Befund 200 ms). `uart_rxdrops` zählt weiter. Jede abgewiesene `CMC`-Zeile wird gezählt und
gemeldet. Ein alter STM verwirft `CMC` ohne Zähler — der Fall mit drei Bedingungen aus §5.2, die
sichere Richtung.

**Still zurechtgebogen?** Nein. N1 verhindert die stille Abweichung nach einer Abweisung. M1
beseitigt ein still beschönigtes `ok`.

**Zustand nach Abbruch?** Wetter: `stop ()`, Endzeile, nichts geparst; A2 endet mit der Endzeile
oder der Frist. EEPROM: Seiten-Präfix. Kommando: nicht angewandt, alter Wert bleibt, Abgleich
vorgemerkt.

**Flags aufgelöst?** **A2:** Der Abrufzustand wird durch die Endzeile **oder** nach 6 s gelöscht,
auf jedem Pfad; ein zweiter Anstoss verlängert nicht. **D:** Die Fähigkeit im ESP wird auf vier
Wegen gelöscht; eine verlorene Eröffnung lässt ihn in der sicheren Richtung, ebenso ein reiner
ESP-Neustart (N4). Die N1-Drossel ist ein Zeitstempel.

---

## Verworfene Alternativen

**L338: feste Frist von 2'500 ms.** **Vom Nutzer abgelehnt** (Ent-8): Bei langsamem Netz ginge
das Wetter verloren; der Nutzer will es immer. **STM wartet pauschal länger als 3 s.**
Verschleierte jeden echten Ausfall und verlängerte **jede** Blockade; A2 verlängert nur während
eines Abrufs. **Nur A1 oder nur A2.** Siehe „Richtige Schicht". **Asynchroner Wetterabruf.**
Umbau mit eigenem Risiko, nach A1 und A2 nicht nötig. **Nur `setTimeout()` herabsetzen.** Bliebe
beim Warten nach Verbindungsende, und DNS/`connect` blieben unbegrenzt (V.1 (5)).

**M1: Ergebnis per Textvergleich der gesendeten Zeile** (`Error`/`Parse Error` suchen). Verworfen
auf Wunsch des Nutzers: Der Parser weiss, welchen Zweig er nimmt; ein Textvergleich hinge an
Wortlaut, der sich ändern kann, und ginge still kaputt. **M1: eigene Endzeile `ERROR weather
fehler` statt der Parser-Zeile.** Verworfen: Der STM sähe andere Bytes als heute.

**A2 als eigenes Release.** Brächte keinen zusätzlichen Gerätebeleg (§1.6).

**L339:** grösserer Ring; nur zwei Aufrufer kürzen; ganze Seite schreiben; ESP rahmt Setter.
**Eine Seitengrösse für beide Ziele.** 32 auf F103 wäre unbelegt und im Fehlfall Datenverlust; 8
auf F411 verschenkte Faktor 4 — Ent-7.

**Teil C:** Setterliste; Parameternamen ohne Werte.

**Teil D:** `CMD …*hhhh` mit Markenpflicht; `CAP`-Zeile vom STM; Reset-Puls als alleinige
Absicherung; Pflichtliste nur für Host und Pfad; eigenes `diag`-Feld; N3; (a) — §5.

---

## Versionsfolgen

Je Einspielschritt, **nur die geänderte Komponente** (DIR-004). Testbauten heben keine Version
an.

| Schritt | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` |
|---|---|---|---|---|
| E (A1, C, D-ESP) | — | ☐ | — | — |
| M (Messzeile, A2) | ☐ | — | — | — |
| C3 | ☐ | — | — | — |
| D (STM-Teil) | ☐ | — | — | — |

Nur der `release-engineer` führt das aus (R4). Der Tag `release/<stm>-<esp>-<app>` wird **je
Einspielschritt** gesetzt und gepusht (DIR-011).
