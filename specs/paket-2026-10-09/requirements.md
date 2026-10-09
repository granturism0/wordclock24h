# Anforderungen — Paket 2026-10-09 „Brückenlast" (L338/C43, L339/A58, Prüfsumme ESP→STM)

**Status: Freigegeben am 09.10.2026.** Alle Entscheidungen sind getroffen (Ent-1 bis Ent-8).
**Erstellt:** 2026-10-09, Stand `dea187a` (Zweig `pwa-decoupling`); am selben Tag zweimal
**nachgeführt**: mit Teil D, dann mit den Ergebnissen aus V.1 und den Entscheidungen Ent-5 bis
Ent-8 (Wetter zweistufig, Seitengrösse je Ziel). Ausgangsstand am Gerät: STM 3.2.22, ESP 3.2.26,
PWA 1.4.94.
**Auslöser:** Testdurchlauf S.26 (L336) aus `specs/paket-2026-10-06/`; die Ursachen von L338 und
L339 sind seit dem 09.10.2026 belegt (Analyse des `firmware-analyst` und Mitschnitt S.26,
Nachtrag in `BEFUNDE.md`). Teil D ist der in L339 benannte fehlende Schutz.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** Zeilennummern gelten für `dea187a`. **`vars.h` ohne Pfad meint
`ESP8266/ESP-uclock/vars.h`.** Kennzeichen wie in `BEFUNDE.md`: **✔ verifiziert** (am Code oder
Gerät belegt), **● gemeldet** (noch zu prüfen).

**Kennungen.** Die vier Schritte heissen **E** (ESP-Release), **M** (STM-Release mit
Messzeile und Wetterwarten), **C3** (STM-Release, seitenweises Schreiben) und **D**
(STM-Release, Prüfsumme ESP→STM), dazu **V** (Vorbereitung, erledigt) und **Z** (Abschluss).
**Teil A** hat zwei Stufen: **A1** (ESP liest die Wetterantwort vollständig) und **A2** (STM
wartet während eines Wetterabrufs auf die Antwort). Gerätenachweise heissen **G1 bis G4**,
Entscheidungen **Ent-1 bis Ent-8**. **„CMC"** ist das Präfix für markierte Kommandos
(`design.md` §5.3). **„L-Befund 200 ms"** ist neu; seine Nummer vergibt der Lead.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Gerätenachweis |
|---|---|---|---|---|
| **V** | Vorbereitung, rein lesend — **erledigt** (Ergebnisse in `design.md` §6) | — | — | — |
| **E** | **A1** (L338, L-Befund 200 ms): Wetterantwort lesen, bis der Dienst schliesst, Sicherheitsnetz 5 s, Messzeile, Endzeile; **Teil C**: Debug-Echo ohne Querystring; **Teil D, ESP-Seite**: ein Kommandoabsender, Marke nur nach angekündigter Fähigkeit — **ohne Wirkung bis zum STM-Teil** | ESP | **zuerst**, ein Release | **G1** |
| **M** | **Teil B, Stufe 1** (L339): Messzeile in `eeprom_write()`; **A2** (L338): `var_send_buf()` wartet während eines Wetterabrufs auf die Antwort, längstens 6 s | STM | nach E | **G2** (Vorher L339), Wetterabrufe |
| **C3** | **Teil B, Stufe 2** (L339): EEPROM seitenweise, **32 Byte auf F411, 8 Byte auf F103** | STM | nach M | **G3** (Nachher), Integritätsprüfung |
| **D** | **Teil D, STM-Seite**: Fähigkeit ankündigen, markierte Kommandos prüfen, abweisen, zählen, Vollabgleich vormerken | STM | nach C3, **eigenes Release** (Ent-6) | **G4** (lesend), `pwa-tester` |
| **Z** | `BEFUNDE.md`, Checkliste, `CLAUDE.md` nachführen | Doku | — | — |

---

## Problem

### 1. L338 / C43 — jeder Wetterabruf hält den ESP rund 5 Sekunden fest

**✔ verifiziert (Mitschnitt S.26 und Code):** `diag` 217 `v=0/0` → 219 `v=1/0` → 221 `v=2/0`,
je unmittelbar nach `weather_get_now` und `weather_get_forecast`. Mitschnitt: `weather`
angestossen 00:56:20,564, `WEATHER` zurück 00:56:25,811 — **5,25 s**; die Vorhersage
**5,3 s**. In beiden Fenstern lief **`N10`** (LDR-Rohwert) in die 3-s-Wartezeit von
`var_send_buf()` des STM (`src/vars/vars.c:48`, `:619`).

**Ursache am Code (✔ in `ESP8266/ESP-uclock/weather.cpp` nachgelesen):**

- `query_weather()` (`weather.cpp:307-427`) läuft **synchron** im Kommandozweig von `loop()`,
  aufgerufen über `get_weather*()` aus `ESP-uclock.ino:1517`, `:1525`, `:1583`, `:1591` und aus
  den Icon-Zweigen `:1649`, `:1657`, `:1715`, `:1723`. **Alle acht Aufrufe laufen durch dieselbe
  Funktion.** Während des Abrufs bedient der ESP die Brücke nicht.
- Sie liest mit `readStringUntil('\n')` (`:369`, `:396`, `:402`). Endet der Antwortkörper ohne
  `\n`, wartet der letzte Aufruf den vollen `WiFiClient`-Timeout von 5 s ab — ✔ laut V.1 (5)
  prüft `Stream::timedRead` im Core `connected()` nicht.

**Gemessen am 09.10.2026 (vom Mac):** `api.openweathermap.org` liefert DNS, Verbindung und
Antwort in **0,13 bis 0,24 s**. Die 5,25 s aus S.26 sind also fast ganz Warten auf ein `\n`,
das nie kommt — nicht Netz.

**Entscheidung des Nutzers (Ent-8):** Die zuerst vorgeschlagene Frist von 2'500 ms ist
**abgelehnt** — **das Wetter soll immer ankommen**. Daraus folgt der zweistufige Entwurf: Der ESP
liest vollständig (A1), und der STM wartet während eines Abrufs auf die Antwort, statt nach 3 s
aufzugeben (A2). Damit ist L338 **unabhängig von der Netzgeschwindigkeit** geschlossen.

### 1a. L-Befund 200 ms — eine Antwort nach mehr als 200 ms wird gar nicht gelesen

**✔ am Code:** Nach dem Senden der Anfrage steht ein festes `delay (200)` (`weather.cpp:359`),
danach `if (openweather_client.available ())` (`:361`). Ist bis dahin noch kein Byte da, wird
**nichts** gelesen, `stop ()` gerufen, und es gibt **kein Wetter** — still, ohne Zeile an den STM.
Bei gemessenen 0,13 bis 0,24 s liegt das **genau an der Grenze**: Ein Teil der Abrufe dürfte schon
heute leer ausgehen. **Eigener Befund**, Nummer vergibt der Lead; behoben im selben Task wie A1
(E.1).

### 2. L339 / A58 — ein Setter-Burst lässt den Empfangsring des STM überlaufen

**✔ verifiziert (Mitschnitt S.26, Prüfungen S97–S99):** sieben Aufrufe in 1,5 s auf
Update-Host und -Pfad, zwei davon mit 63 Byte; `d` stieg von 0 auf **191**, `rx=1024/1024`.
Mit 4 s Abstand kein Verlust.

**Ursache am Code (✔ nachgelesen):**

- Die Empfangs-ISR nimmt weiter an (`src/uart/uart-driver.h:808-830`); ist der Ring voll,
  zählt sie `uart_rxdrops` hoch — das ist `d` in der `diag`-Zeile.
- Der Hauptloop steht derweil in `write_update_host_to_eep()` bzw. `write_update_path_to_eep()`
  (`src/main.c:1039-1052`, `:1082-1095`). Beide schreiben **immer den ganzen 64-Byte-Puffer**.
- `eeprom_write()` (`src/eeprom/eeprom.c:201-240`) schreibt **Byte für Byte**: je geändertem
  Byte ein `i2c_write()` mit einem Byte und danach `eeprom_waitstates()` (`:38-57`), 15 bis
  16 ms Busy-Wait. **Rund 1 s je Setter** bei 64 geänderten Byte.
- `i2c_write()` (`src/i2c/i2c.c:441-478`) kann bereits **mehrere Byte in einer Transaktion**.
- **V.1 (2), ✔:** Es gibt **keine** periodischen EEPROM-Schreiber; jeder Schreibvorgang folgt
  einer Nutzeraktion oder dem Start.

**Was diesmal verloren ging, und warum das Glück war:** Verloren gingen die **Debug-Echos**;
für Kommandos ESP→STM gibt es **weder Quittung noch Prüfsumme** — ein verstümmeltes `CMD S09…`
würde angenommen und ins EEPROM geschrieben, ausgerechnet bei der **Update-Quelle** (L42,
DIR-009). Dagegen tritt Teil D an.

**Verwandt, nicht dasselbe:** **L204** — dort war die EEPROM-Spur die erste Vermutung und hat
sich als **falsch** erwiesen. Deshalb misst diese Spec die Blockade **zuerst** (Ent-3 = V1).

### 3. Teil C — jeder Wert reist doppelt über die Brücke, auch Geheimnisse

**✔ verifiziert (`http.cpp:14071-14079` für POST, `:14129-14143` für GET):** Der ESP schreibt
für jede Anfrage `- request <IP> [<label>]: <Anfragezeile>` — **mit vollständigem
Querystring**, auch die Wetter-`appid` und den WLAN-Schlüssel. Checkliste §3: „Keine
WLAN-Credentials oder Gerätegeheimnisse in Logausgaben." **✔ per `grep`:** Die volle Zeile wertet
kein Werkzeug aus; `tools/watch-log.sh:128` genügt der Pfad.

### 4. Teil D — Kommandos ESP→STM haben keinen Schutz gegen Verstümmelung

**✔ verifiziert am Code:**

- Die Gegenrichtung **ist** geschützt (A32): `var`-Zeilen tragen `*hhhh` (`src/vars/vars.c:532-538`),
  der ESP prüft (`ESP-uclock.ino:661-712`), quittiert mit `ACK xy` und `.` oder `!v`, der STM sendet
  auf `!v` nach. Die Fähigkeit meldet der ESP mit `CAP var-crc` vor seiner `FIRMWARE`-Zeile
  (`ESP-uclock.ino:184`; Festschreibung `esp8266.c:190-214`, `:604-609`, `:823-859`).
- **In Richtung ESP→STM gibt es nichts davon.** Der ESP sendet rund 40 Formen von `CMD …`-Zeilen
  direkt per `Serial.printf` (`vars.cpp:67-851`, `udpsrv.cpp:269`); der STM wendet jede Zeile mit
  `CMD ` an (`esp8266.c:611-620`, `main.c:3197-3201`).
- **Rückwärtsfalle:** Ein STM ohne Prüfung nimmt eine angehängte Marke als Teil des Werts —
  `meinhost*c328` als Update-Host (Risiko R-3, `esp8266.c:198-201`).
- **Was bereitliegt:** Eröffnung `var VBffnn` jedes Vollabgleichs (`vars.c:1750`), deren Flags der
  ESP bitweise liest und unbekannte Bits duldet (`ESP-uclock.ino:881-918`); Reset des ESP bei
  jedem STM-Start (`esp8266.c:974-989`); ein unbekanntes Präfix endet als `ESP8266_UNSPECIFIED`
  (`esp8266.c:638-641`), nicht angewandt.
- **Die `diag`-Zeile ist voll** (`main.c:3802-3814`).

---

## Ziel

1. **Das Wetter kommt immer an**, wenn der Dienst antwortet: Der ESP liest die Antwort, bis der
   Dienst schliesst; der STM wartet während eines Abrufs auf die Antwort. Ein Abruf dauert laut
   Messzeile typisch **unter 1 s**, und **während eines Abrufs steigt `v` nicht** — auch nicht,
   wenn gerade ein quittungspflichtiges Kommando unterwegs ist.
2. Ein Zeichenketten-Setter blockiert den STM-Hauptloop **je EEPROM-Seite einen
   Schreibzyklus** statt je Byte — mit **byte-genau demselben EEPROM-Inhalt** wie heute.
3. Kein Anfragewert geht mehr als Debug-Echo über die Brücke.
4. **Ein verstümmeltes Kommando ESP→STM wird nicht angewandt**, sondern abgewiesen, gezählt,
   gemeldet, und ESP und STM werden wieder gleichgezogen — und eine Seite, die die Prüfung nicht
   kennt, nimmt **nie** eine Marke als Teil eines Werts.
5. Jede Wirkung ist **einzeln** am Gerät nachgewiesen, und jede neue Messzeile hat einmal den
   schlechten Wert gezeigt (DIR-014).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt**
(DIR-014, L177/L185). Prüfstände laufen über `tools/checks/auszug.sh` mit den Typbreiten des
Ziels (L256) und sind **einmal gegen die alte Fassung fehlgeschlagen**. **Was am Gerät nicht
herstellbar ist, wird nicht am Gerät verlangt.**

### Schritt E — ESP (A1 und Teil C)

- [ ] **AKE.1 (Messzeile Wetter)** — Jeder Aufruf von `query_weather()` endet mit **genau
      einer** Zeile `- weather fc=<0|1> ms=<n> <ergebnis>`, `<ergebnis>` aus `ok`, `dns`,
      `connfail`, `timeout`, `leer`. Sie geht **beide Wege** — `Serial` **und**
      `stm32_log_append()` — wie die Heap-Zeile (`ESP-uclock.ino:494-506`, C14/L185), und
      enthält **weder** `appid` **noch** Ort, Koordinaten oder URL. *Instrument:* Prüfstand t12;
      am Gerät in G1 im Mitschnitt **und** in `/api/stm32_log`.
- [ ] **AKE.2 (vollständig lesen, Sicherheitsnetz)** — Der ESP liest die Antwort, **bis der
      Dienst die Verbindung schliesst**, ohne festes Warten. Die Gesamtfrist von **5 s** ist
      **nur ein Sicherheitsnetz**: DNS und Verbindungsaufbau sind **getrennt** über die Restfrist
      begrenzt (`WiFi.hostByName (…, rest)`, dann `connect (ip, 80)`, `setTimeout (rest)` vor
      `print ()`), und die rund 300 ms von `stop ()` sind eingerechnet — `query_weather()` kehrt
      in **jedem** Fall nach höchstens **5'000 ms + 100 ms** zurück. *Instrument:* Prüfstand t12
      mit nachgebildeter Zeit, Fälle: Körper ohne `\n` mit Verbindungsende (⇒ sofort fertig,
      `ok`); Server sendet nie; Server schliesst nie und sendet tröpfchenweise; DNS hängt;
      `connect` hängt; Kopf ohne Leerzeile. **Gegenprobe:** „Körper ohne `\n`" dauert gegen das
      alte `weather.cpp` ≥ 5'000 ms ⇒ FEHL.
- [ ] **AKE.3 (L-Befund 200 ms)** — Eine Antwort, deren erstes Byte nach **mehr als 200 ms**
      eintrifft, wird **gelesen und geparst**. *Instrument:* Prüfstand t12, Fälle 250 ms,
      1'500 ms und 4'000 ms; **Gegenprobe:** gegen das alte `weather.cpp` wird keiner geparst ⇒
      FEHL.
- [ ] **AKE.4 (gleiches Parseergebnis)** — Für jede wohlgeformte Antwort, die das alte
      `weather.cpp` geparst hat, erhält `parse_weather()` bzw. `parse_weather_fc()` **dieselbe
      Zeichenkette**: `Content-Length` ohne abschliessendes `\n`, `Transfer-Encoding: chunked`,
      Körper in einem und in mehreren Segmenten, Vorhersage mit grossem Körper. *Instrument:*
      Prüfstand t12, alt gegen neu; Fallzahl gemeldet.
- [ ] **AKE.5 (Soft-Watchdog)** — Die Leseschleife gibt in jedem Durchlauf ohne Zeichen die
      Kontrolle ab (`yield()` oder `delay()`). *Instrument:* Review E.5; am Gerät kein Neustart.
- [ ] **AKE.6 (Endzeile für den STM)** — Jeder Aufruf von `query_weather()` sendet **genau eine**
      Zeile, an der der STM das Ende des Abrufs erkennt: `WEATHER …`, `WEATHER_FC …`,
      `WICON …`, `WICON_FC …` — oder, wenn keine davon entstand, `ERROR weather <ergebnis>`.
      Heute endet nur `connfail` mit `ERROR` (`weather.cpp:425`); bei leerer oder nicht
      ausgewerteter Antwort kommt nichts. *Instrument:* Prüfstand t12 zählt je Fall die
      Endzeilen (genau eine). **Gegenprobe:** Der Fall „Antwort nach 250 ms" liefert gegen das
      alte `weather.cpp` **keine** Endzeile ⇒ FEHL.
- [ ] **AKE.7 (Gerät, A1)** — Nach dem ESP-Einspielen je **ein** `weather_get_now` und
      **ein** `weather_get_forecast` (G1): Die Messzeile zeigt `ok` und `ms` **unter 1'000**;
      ihr Wert stimmt auf **±150 ms** mit dem Abstand zwischen `weather` und `WEATHER` im
      Mitschnitt überein — **das unabhängige Mass**, mit dem der Vorher-Wert aus S.26 vorliegt.
      `v` steigt in den 6 s danach **nicht**. Dazu **lesend über mindestens 60 Minuten**
      (≥ 3 automatische Abrufe, Wetter und Icons): Abrufdauer laut Messzeile **typisch unter
      1 s** (Median und grösster Wert gemeldet), kein `v`-Anstieg im 6-s-Fenster nach einem
      Abruf. Ein Anstieg durch einen ESP-Neustart (L327) zählt nicht und wird getrennt berichtet.
- [ ] **AKE.8 (Echo ohne Werte)** — Beide Echo-Stellen laufen über **eine** Funktion; die Zeile
      enthält Methode und Pfad und, falls ein Querystring vorhanden ist, den Marker `?…` —
      **kein Zeichen aus dem Querystring**. *Instrument:* Prüfstand t13 (Setter mit Wert,
      `appid`, `GET /?a`, ohne Query, nur `?`, POST mit Query); **statische Prüfung**: kein
      `Serial.print*` mit `sRequest` oder `sParam` in `http.cpp` — **schlägt gegen den
      Ausgangsstand an**.
- [ ] **AKE.9 (Gerät, Echo)** — Im Mitschnitt von G1 bis G4 enthält **keine** Zeile
      `- request` ein `=`; Zahl der geprüften Zeilen gemeldet (> 0).

### Schritt M — STM: Messzeile (Teil B) und Wetterwarten (A2)

- [ ] **AKM.1 (Messzeile EEPROM)** — `eeprom_write()` gibt nach jedem Aufruf mit mindestens
      einem Schreibzyklus **genau eine** Zeile `eep a=<start> n=<anzahl> z=<zyklen>
      ms=<dauer> d=<vorher>/<nachher>` über `log_printf()` aus (Mitschnitt **und**
      `/api/stm32_log`). `d` = `uart_rxdrops` der ESP-Brücke vor und nach dem Schreiben. Kein
      Inhaltsbyte. Ohne Schreibzyklus keine Zeile; **keine weitere Schwelle** (V.1 (2): keine
      periodischen Schreiber). *Instrument:* Quelltext (Review M.3); am Gerät G2.
- [ ] **AKM.2 (unabhängige Zeitbasis)** — `ms` wird über eine Zeitbasis gemessen, die
      **unabhängig** von `eeprom_waitstates()` und `eeprom_ms_tick` ist (Vorschlag:
      `diag_tick_cnt`, `src/main.c:655`, umgerechnet über `F_INTERRUPTS`). Sonst ist die
      Selbstprüfung leer und kann nie fehlschlagen (V.1 (3), DIR-014). In G2 gilt für jede Zeile
      `15·z ≤ ms ≤ 17·z + 10`, bei einer gröberen Auflösung als 1 ms um diese Auflösung
      erweitert. **Verfehlt ⇒ Halt.**
- [ ] **AKM.3 (Vorher, G2)** — Der Setter-Burst auf den Wetterort (`design.md` §4.2) liefert
      sieben Messzeilen mit `a` = Offset des Wetterorts, `n=32`, **`z ≥ 25`** bei jedem Wechsel
      lang↔kurz und **`ms ≥ 400`**. `d` wird berichtet, ein Anstieg ist **nicht** verlangt;
      bleibt `d` bei 0, wird G2 einmal mit 14 Aufrufen in rund 3 s wiederholt.
- [ ] **AKW.1 (A2: Warten auf die Wetterantwort)** — Hat der STM einen Wetterabruf angestossen
      (`weather_query()`, `src/weather/weather.c:213-285`: `weather`, `weather_fc`, `wicon`,
      `wicon_fc`), wartet `var_send_buf()` bis zum Eintreffen der Endzeile (`WEATHER`,
      `WEATHER_FC`, `WICON`, `WICON_FC` oder `ERROR`), **längstens 6 s ab dem Anstoss**, statt nach
      `VAR_SEND_TIMEOUT_SEC` (3 s) aufzugeben. Danach gilt wieder die normale Wartezeit.
      *Instrument:* Prüfstand W2 (Auszug von `var_send_buf()` mit nachgebildeter Zeit und
      Nachrichtenfolge): Abruf läuft, ESP schweigt 5 s, dann Quittung ⇒ angenommen, **kein**
      Timeout gezählt. **Gegenprobe:** gegen den E-Stand Timeout nach 3 s ⇒ FEHL.
- [ ] **AKW.2 (A2: nichts bleibt hängen)** — Kommt **keine** Endzeile, endet das verlängerte
      Warten nach der Frist, der Abrufzustand ist danach gelöscht, und jedes weitere Kommando
      wartet wieder normal (V.1 (6): heute bleibt ohne `WEATHER` nichts hängen — das muss so
      bleiben). Kein Abruf angestossen ⇒ 3 s wie heute. Eine Endzeile ohne angestossenen Abruf
      ändert nichts. Ein zweiter Anstoss während eines laufenden Abrufs verlängert die Frist
      **nicht** über 6 s ab dem ersten hinaus. *Instrument:* Prüfstand W2, jeder dieser Fälle.
- [ ] **AKW.3 (A2: Watchdog)** — **Keine neue `watchdog_reload()`-Aufrufstelle** (Guardrail S7
      zeigt denselben Bestand). Das verlängerte Warten bleibt mit 6 s weit unter dem
      Watchdog-Timeout von 20 s; nach eingetroffener Antwort gilt der bestehende Reload im
      Rahmen von `VAR_SEND_RELOAD_BUDGET_SEC` (`vars.c:79`, `:698-701`). *Instrument:* Review M.3,
      S7.
- [ ] **AKW.4 (A2, Gerät)** — Nach dem M-Flash: je ein `weather_get_now` und
      `weather_get_forecast` und **60 Minuten lesend** (wie AKE.7): kein `v`-Anstieg nach einem
      Abruf, kein Watchdog-Reset. **Am Gerät nicht herstellbar** ist ein langsamer Dienst; dass
      `v` auch bei einer Antwort nach 5 s nicht steigt, belegt der Prüfstand W2.
- [ ] **AKM.4 (Flash)** — Testbau F103 nach M.1 und nach M.1b: Rest **≥ 1'024 Byte**. Darunter:
      anhalten und vorlegen.

### Schritt C3 — STM, seitenweise schreiben

- [ ] **AKC.1 (Seitengrösse je Ziel)** — Die Seitengrösse ist eine **Build-Konstante je Ziel**:
      **32 Byte auf F411 (V2)**, **8 Byte auf F103** (Entscheidung des Nutzers, Ent-7). Die
      Begründung steht im Code-Kommentar. *Instrument:* Review C3.4; Testbau beider Ziele.
- [ ] **AKC.2 (keine Seitengrenze überschritten)** — Jede I2C-Schreibtransaktion liegt
      **vollständig in einer Seite**; je Seite **höchstens eine** Transaktion und **ein**
      Wartezyklus. *Instrument:* Prüfstand C3 **für beide Seitengrössen**, mit nachgebildetem
      EEPROM, das **wie die Hardware** am Seitenende auf den Seitenanfang umbricht.
- [ ] **AKC.3 (byte-gleiches Ergebnis)** — **Für P = 32 und P = 8:** mindestens **10'000**
      Zufallsaufrufe (fester Startwert) über **0..4095**, Längen 0 bis 600, und **jeder Bereich
      aus `eeprom-data.h`** (voll, Teil, ein Byte, unverändert, über Seitengrenzen) — das Abbild
      ist **nach jedem Aufruf** identisch mit der alten Fassung, ebenso der Rückgabewert. Zyklen
      neu ≤ alt; alt 0 ⇒ neu 0. **Gegenprobe:** drei Sabotagen — Seitengrösse 64 bei einem Baustein
      mit 32; Seitengrösse 32 bei einem Baustein mit 8; Seitenende um eins verschoben — melden
      **alle** FEHL; das Zyklenkriterium meldet gegen die alte Fassung FEHL.
- [ ] **AKC.4 (Fehlerpfade)** — Lesefehler ⇒ Seite wird geschrieben; Schreibfehler ⇒ 0,
      **keine** weitere Seite; `cnt == 0` ⇒ 1 ohne I2C-Verkehr; `eeprom_is_up == 0` ⇒ 0.
      *Instrument:* Prüfstand C3 mit Fehlereinspeisung, beide Seitengrössen.
- [ ] **AKC.5 (Rechnung)** — Zyklen alt und neu je Aufruf, für beide Seitengrössen: **S.26-Folge**
      (Host/Pfad 63 Byte ↔ kurz) — neu **höchstens 3** bei P = 32, **höchstens 9** bei P = 8;
      **Wetterort-Folge** — neu **höchstens 2** bei P = 32, **höchstens 5** bei P = 8. Die Zahlen
      gehen in den Bericht C3.8.
- [ ] **AKC.6 (Nachher, G3)** — Derselbe Burst wie G2 **am Gerät des Nutzers (F411, P = 32)**:
      jede Messzeile mit **`z ≤ 2`** und **`ms ≤ 60`**; `d` steigt **nicht**; `diag` ohne Lücke.
      **Für F103** (P = 8, erwartet `z ≤ 5`, `ms ≤ 100`) gilt der Prüfstand; ein F103-Gerät steht
      nicht zur Verfügung.
- [ ] **AKC.7 (Integrität am Gerät)** — (1) Abzug M2 **vor** dem C3-Flash gleich dem Abzug
      **nach** Flash und STM-Reset (`diff-snapshot.sh --soll`). (2) Nach G3 Originalwert zurück,
      STM-Reset, das Gerät zeigt den **Originalwert** — aus dem EEPROM gelesen. (3) Nach dem
      `pwa-tester`-Durchlauf (D.9) ein STM-Reset, danach zeigt Phase 9 die Einstellungen, wie
      der Durchlauf sie hinterlassen hat.
- [ ] **AKC.8 (Flash)** — Testbau F103 nach C3.1: Rest **≥ 1'024 Byte**; Gegenprobe in C3.6
      aus S8b. Darunter: anhalten und vorlegen.

### Teil D — Prüfsumme ESP→STM

**ESP-Seite (mit Schritt E):**

- [ ] **AKD.1 (ein Absender)** — Jede `CMD`-Zeile des ESP entsteht in **genau einer**
      Funktion. *Instrument:* statische Prüfung, dass ausserhalb dieser Funktion kein
      `Serial.print*` eine Zeile mit `CMD` beginnt; Zahl der Stellen gegen V.1 (7).
      **Gegenprobe:** schlägt gegen den Ausgangsstand an.
- [ ] **AKD.2 (ohne Fähigkeit byte-gleich)** — Solange der ESP die Fähigkeit nicht gelernt hat,
      ist **jede** Kommandozeile **byte-gleich** mit der heutigen. *Instrument:* Prüfstand t14,
      jede Kommandoform (Fallzahl gegen V.1 (7)), alte gegen neue Fassung.
- [ ] **AKD.3 (mit Fähigkeit markiert)** — Mit Fähigkeit sendet der ESP jede Kommandozeile als
      `CMC <nutzlast>*hhhh\r\n`; `hhhh` = `var_crc()` über die Nutzlast — **dieselbe** Funktion
      wie für die `var`-Zeilen. *Instrument:* Prüfstand t14 gegen die Vektoren aus
      `tools/checks/var-crc.c`; **Gegenprobe:** Ausgangsstand ohne markierte Zeile ⇒ FEHL.
- [ ] **AKD.4 (Fähigkeit lernen und verlieren)** — Der ESP setzt die Fähigkeit **nur** bei einer
      Eröffnung `var VBffnn` mit Flag `0x04`. Er **löscht** sie (a) bei jedem eigenen Start,
      (b) bei jeder Eröffnung **ohne** `0x04`, (c) **vor** jedem STM-Reset und (d) **vor** jedem
      STM-Flash, den er selbst auslöst. Jeder Wechsel erzeugt eine Zeile im Logring
      (`stm32_log_append()`). *Instrument:* Prüfstand t14 mit allen Wegen, verlorener und
      nachgesendeter Eröffnung.

**STM-Seite (Schritt D):**

- [ ] **AKD.5 (ankündigen)** — Der STM setzt in **jeder** Eröffnung das Flag `0x04`.
- [ ] **AKD.6 (prüfen und anwenden)** — `CMC` mit **richtiger** Marke wird abgeschnitten und
      **genau so** angewandt wie die gleiche `CMD`-Zeile heute; `CMD` wird angewandt **wie
      heute**, auch mit Text auf `*` + vier Hexziffern. *Instrument:* Prüfstand D; **Gegenprobe:**
      gegen den C3-Stand nicht angewandt ⇒ FEHL.
- [ ] **AKD.7 (abweisen, zählen, melden, gleichziehen)** — `CMC` mit **falscher, fehlender oder
      formal ungültiger** Marke wird **nicht** angewandt, sättigend gezählt und gedrosselt
      gemeldet (die ersten vier einzeln, danach jede fünfzigste, **laufender Zähler in der
      Zeile**, ohne Wert) über `log_printf()`. **Ent-5 = N1:** Zusätzlich wird ein Vollabgleich
      vorgemerkt, **höchstens einer je 60 s**. *Instrument:* Prüfstand D: Marke falsch; fehlt;
      `*` mit drei Hexziffern; Nicht-Hex; verkürzte Nutzlast; zwei Zeilen verschmolzen; leere
      Nutzlast; längste Nutzlast samt Marke passt in `ESP8266_MAX_CMD_LEN`; zwei Abweisungen in
      10 s ⇒ **ein** Vormerken; zwei im Abstand von 61 s ⇒ zwei.
- [ ] **AKD.8 (alter STM nimmt nie eine Marke an)** — `CMC` ergibt im **heutigen** `esp8266.c`
      `ESP8266_UNSPECIFIED`, nicht angewandt. *Instrument:* Prüfstand D gegen den Ausgangsstand —
      eine Sicherheitsaussage, die dort **bestehen soll**.
- [ ] **AKD.9 (Flash)** — Testbau F103 nach D.1: Rest **≥ 1'024 Byte**. Darunter: **anhalten und
      vorlegen**.
- [ ] **AKD.10 (Gerät, G4, lesend)** — **In G1** (neuer ESP, alter STM): nur `CMD`, Eröffnungen
      ohne `0x04`, keine Logzeile „Fähigkeit an". **Nach D:** Eröffnung mit `0x04`; „Fähigkeit
      an" im Logring; jede Speicheraktion des `pwa-tester`-Durchlaufs als `(CMC …*hhhh)` im
      Mitschnitt; Phase 9 zeigt die Werte angewandt; **null** Abweisungen. Zahl der `CMC`- und
      `CMD`-Zeilen gemeldet (> 0). **Die Abweisung belegt der Prüfstand.**

### Für alle Schritte

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `./tools/guardrails.sh --full` vor jedem Release mit Exit 0 (Lead, R1);
      `./tools/checks/auszug.sh` meldet am Ende **sechs** Prüfstände mehr als vorher (t12, t13,
      t14, W2, C3, D).
- [ ] **AKZ.3** — `./tools/smoke-device.sh` nach jedem Einspielen ohne Fehlschlag, nach dem
      ESP-Update **samt Update-Quelle** (DIR-009); `./tools/install-app.sh --check` nach dem
      ESP-Update (DIR-017). **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.4** — **Ein `pwa-tester`-Durchlauf Phasen 0–4 und 9 nach dem letzten STM-Release**
      (D.9; wird D am Flash-Gate angehalten, nach C3). Unvollständig ⇒ **nicht abgenommen**.
- [ ] **AKZ.5** — `./tools/watch-log.sh` läuft bei jedem Gerätelauf mit (DIR-013) und meldet
      nach E.4 auch `- weather` mit Ergebnis ungleich `ok` oder `ms` ≥ 1'000, `eep`-Zeilen mit
      `ms` ≥ 200 und `cmd abgewiesen`.
- [ ] **AKZ.6** — Jedes ausgerollte Release **sofort** committet, getaggt, **gepusht** (DIR-011).
- [ ] **AKZ.7** — Die Einspielzeile nennt die **Reihenfolge**, fertig zum Einfügen.
- [ ] **AKZ.8** — Kodierung **und** Zeilenende jeder Datei vor dem Patch festgestellt (DIR-015):
      `weather.cpp`, `http.cpp` (UTF-8), `vars.cpp`, `udpsrv.cpp`, `stm32flash.cpp` (UTF-8),
      `ESP-uclock.ino` (**gemischt**), `src/eeprom/eeprom.c`, `src/vars/vars.c`, `vars.h`,
      `src/weather/weather.c`, `src/esp8266/esp8266.c`, ggf. `src/main.c`.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **Eine feste Wetterfrist unter 3 s** | **Abgelehnt** vom Nutzer (Ent-8): Das Wetter soll ankommen. Die 5 s sind nur Sicherheitsnetz |
| **Asynchroner Wetterabruf** | Grösserer Umbau; mit A1 und A2 nicht nötig |
| **(d) Grösserer Empfangsring** | Nur Notbehelf |
| **(a) Quittung für jedes `CMD`**, **N3 gezielte Nachsendung** | Bewertet in `design.md` §5.4; entschieden ist N1 (Ent-5) |
| **Ein eigenes `diag`-Feld für Teil D** | Die Zeile ist voll (`main.c:3802-3814`) |
| **Die STM-eigene Ausgabe `(CMD …)` mit Werten auf der Log-UART** (`esp8266.c:466-469`) | **Gemeldet**, neuer Befund (Z.1) |
| **Längenrechnung der `diag`-Zeile ohne `a=`** | **Gemeldet**, neuer Befund (Z.1) |
| **ACK-Polling statt fester 15 ms in `eeprom_waitstates()`** | Eigener Schritt, eigene Messung |
| **`write_update_host_to_eep()` / `…path…` nur bis zum Abschlussbyte schreiben** | C3 behebt die Ursache für **alle** Schreiber |
| **Flash-Pfad `BLACK_BOARD`** | Nicht berührt |
| **Weitere Stellen, die Anfragewerte über `Serial` ausgeben** | **Melden, nicht mitkorrigieren** (E.2) |
| **L327 / A55**, **L349 / C49** | Eigene Befunde |
| **Schreibende Tests auf Update-Host und -Pfad** | Ausgeschlossen |
| **Gerätetest auf einem F103** | Kein Gerät; Prüfstand für P = 8 |
| **Eine verstümmelte Zeile oder ein langsamer Wetterdienst am Gerät** | Nicht gezielt herstellbar; Prüfstände |
| **PWA, Legacy-Oberfläche** | Unverändert |

---

## Betroffene Laufzeiten

- [x] **ESP8266** — Schritt E: `weather.cpp`, `http.cpp`, `vars.cpp`, `udpsrv.cpp`,
      `ESP-uclock.ino`, `stm32flash.cpp` und die Reset- und Flash-Pfade aus V.1 (7); ein OTA
- [x] **STM32** (`src/**`) — M: `src/eeprom/eeprom.c`, `src/vars/vars.c`, `src/weather/weather.c`,
      ggf. `src/main.c`; C3: `src/eeprom/eeprom.c`; D: `src/esp8266/esp8266.c`, `src/vars/vars.c`,
      `vars.h`, ggf. `src/main.c`; je ein Flash
- [ ] **PWA** — nicht betroffen
- [x] **Werkzeug** (`tools/**`) — Prüfstände t12, t13, t14, W2, C3, D; `auszug.sh`; zwei
      statische Prüfungen; `watch-log.sh`
- [x] **Dokumentation** — `BEFUNDE.md`, `knowledge/architecture-checklist.md`, `CLAUDE.md`
- [x] **Build/Release** — **vier** Releases (ESP; STM M; STM C3; STM D), je mit Commit, Tag,
      Push; dazu Testbauten F103 (und F411 für C3) ohne Release

---

## Entscheidungen

Stand **09.10.2026** — **alle entschieden**.

- **Ent-1 — Freigabe. Entschieden: freigegeben**, mit den schreibenden Gerätenachweisen G1, G2
  (samt Wiederholung mit 14 Aufrufen bei `d = 0`), G3 und dem `pwa-tester`-Durchlauf. G4 ist
  lesend.
- **Ent-2 — Instrumentierung. Entschieden: Beide Messzeilen bleiben im Fabrikat.**
- **Ent-3 — Vorher-Nachweis L339. Entschieden: V1** (Messzeile und C3 getrennt).
- **Ent-4 — Prüfsumme ESP→STM. Entschieden: „Gleich mitnehmen"** (Teil D).
- **Ent-5 — Reaktion auf eine Abweisung. Entschieden: N1** — abweisen, zählen, melden und einen
  Vollabgleich vormerken, **höchstens einer je 60 s**.
- **Ent-6 — STM-Teil von D. Entschieden: eigenes, viertes Release nach C3.**
- **Ent-7 — EEPROM-Seitengrösse. Entschieden: 32 Byte auf F411 (V2), 8 Byte auf F103**, als
  Build-Konstante je Ziel. Der Prüfstand fährt beide.
- **Ent-8 — Wetter. Entschieden:** Die Frist von 2'500 ms ist **abgelehnt**; das Wetter soll
  immer ankommen. **Zweistufig:** A1 auf dem ESP (vollständig lesen, 5 s nur als Sicherheitsnetz,
  L-Befund 200 ms im selben Task), A2 auf dem STM (Warten auf die Antwort, längstens 6 s). A2
  fährt im **M-Release** (Vorschlag des Leads; Begründung nach R3b in `design.md` §1.6).
