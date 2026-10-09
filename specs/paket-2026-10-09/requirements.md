# Anforderungen — Paket 2026-10-09 „Brückenlast" (L338/C43, L339/A58)

**Status: Zur Freigabe** (Ent-1). Der **Umfang** L338 + L339 ist vom Nutzer freigegeben
(„Genau", 09.10.2026); die Spec selbst und die schreibenden Gerätenachweise sind es noch nicht.
**Erstellt:** 2026-10-09, Stand `dea187a` (Zweig `pwa-decoupling`). Ausgangsstand am Gerät:
STM 3.2.22, ESP 3.2.26, PWA 1.4.94.
**Auslöser:** Testdurchlauf S.26 (L336) aus `specs/paket-2026-10-06/`; die Ursachen beider
Befunde sind seit dem 09.10.2026 belegt (Analyse des `firmware-analyst` und Mitschnitt S.26,
Nachtrag in den Zeilen L338 und L339 von `BEFUNDE.md`).

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** Zeilennummern gelten für `dea187a` und sind mit `~` markiert, wo sie aus der
Analyse übernommen und nicht einzeln nachgezählt sind. **`vars.h` ohne Pfad meint
`ESP8266/ESP-uclock/vars.h`.** Kennzeichen wie in `BEFUNDE.md`: **✔ verifiziert** (am Code oder
Gerät belegt), **● gemeldet** (noch zu prüfen).

**Kennungen.** Die drei Schritte heissen **E** (ESP-Release), **M** (STM-Release mit Messzeile)
und **C3** (STM-Release mit seitenweisem Schreiben), dazu **V** (Vorbereitung, lesend) und
**Z** (Abschluss). Gerätenachweise heissen **G1 bis G3**, Entscheidungen **Ent-1 bis Ent-4**.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Gerätenachweis |
|---|---|---|---|---|
| **V** | Vorbereitung, rein lesend: Seitengrösse der EEPROMs, periodische EEPROM-Schreiber, Begrenzung von `connect()`, Flash-Schätzung | — | — | — |
| **E** | **Teil A** (L338): Wetterabruf mit Gesamtgrenze und Messzeile; **Teil C**: Debug-Echo ohne Querystring | ESP | **zuerst**, ein Release | **G1** |
| **M** | **Teil B, Stufe 1** (L339): Messzeile in `eeprom_write()`, sonst keine Änderung | STM | nach E | **G2** (Vorher) |
| **C3** | **Teil B, Stufe 2** (L339): EEPROM seitenweise schreiben (C3) | STM | nach M | **G3** (Nachher), Integritätsprüfung, `pwa-tester` |
| **Z** | `BEFUNDE.md`, Checkliste, `CLAUDE.md` nachführen | Doku | — | — |

Die Aufteilung von Teil B in **M** und **C3** ist die Empfehlung aus **Ent-3** (Variante V1).
Entscheidet der Nutzer anders, fallen M.4 bis M.7 weg und die Messzeile fährt mit C3
(`tasks.md`, Hinweis bei Schritt M).

---

## Problem

### 1. L338 / C43 — jeder Wetterabruf hält den ESP rund 5 Sekunden fest

**✔ verifiziert (Mitschnitt S.26 und Code):** `diag` 217 `v=0/0` → 219 `v=1/0` → 221 `v=2/0`,
je unmittelbar nach `weather_get_now` und `weather_get_forecast`. Mitschnitt: `weather`
angestossen 00:56:20,564, `WEATHER` zurück 00:56:25,811 — **5,25 s**; die Vorhersage
**5,3 s**. In beiden Fenstern lief **`N10`** (LDR-Rohwert) in die 3-s-Wartezeit von
`var_send_buf()` des STM.

**Ursache am Code (✔ in `ESP8266/ESP-uclock/weather.cpp` nachgelesen):**

- `query_weather()` (`weather.cpp:307-427`) läuft **synchron** im Kommandozweig von `loop()`,
  aufgerufen über `get_weather*()` aus `ESP-uclock.ino:1517`, `:1525`, `:1583`, `:1591` und —
  in der Analyse nicht genannt — ebenso aus den Icon-Zweigen `:1649`, `:1657`, `:1715`,
  `:1723`. **Alle acht Aufrufe laufen durch dieselbe Funktion.** Während des Abrufs bedient der
  ESP die Brücke nicht.
- Sie liest mit `readStringUntil('\n')` (`:369`, `:396`, `:402`). Endet der Antwortkörper ohne
  `\n`, wartet der letzte Aufruf den vollen `WiFiClient`-Timeout von 5 s ab — ● laut Analyse
  prüft `Stream::timedRead` im Core `connected()` nicht.
- Davor steht ein festes `delay (200)` (`:359`), danach `if (openweather_client.available())`
  (`:361`): **Kommt die Antwort später als 200 ms, wird sie gar nicht gelesen** — das
  Wetter bleibt still auf dem alten Stand.

**Tragweite:** Das trifft nicht nur den Aufruf aus der PWA, sondern auch den automatischen
Abruf (Wetter und Icons) im laufenden Betrieb. Jeder Abruf kann einen nur **einmal**
angekündigten Wert kosten — derselbe Mechanismus wie in **L42**. **Nicht L327:** Dort steigt
`v`, weil der ESP neu startet; hier läuft er durch.

### 2. L339 / A58 — ein Setter-Burst lässt den Empfangsring des STM überlaufen

**✔ verifiziert (Mitschnitt S.26, Prüfungen S97–S99):** sieben Aufrufe in 1,5 s auf
Update-Host und -Pfad, zwei davon mit 63 Byte; `d` stieg von 0 auf **191**, `rx=1024/1024`.
Mit 4 s Abstand kein Verlust.

**Ursache am Code (✔ nachgelesen):**

- Die Empfangs-ISR nimmt weiter an (`src/uart/uart-driver.h:808-830`); ist der Ring voll,
  zählt sie `uart_rxdrops` hoch — das ist `d` in der `diag`-Zeile.
- Der Hauptloop steht derweil in `write_update_host_to_eep()` bzw. `write_update_path_to_eep()`
  (`src/main.c:1039-1052`, `:1082-1095`). Beide schreiben **immer den ganzen 64-Byte-Puffer**
  (`memset` + `strncpy`), auch beim Zurücksetzen auf einen kurzen Wert.
- `eeprom_write()` (`src/eeprom/eeprom.c:201-240`) schreibt **Byte für Byte**: je geändertem
  Byte ein `i2c_write()` mit einem Byte und danach `eeprom_waitstates()` (`:38-57`), also
  15 bis 16 ms Busy-Wait. Unveränderte Bytes werden seit L144 übersprungen. **Rund 1 s je
  Setter** bei 64 geänderten Byte.
- `i2c_write()` (`src/i2c/i2c.c:441-478`) kann bereits **mehrere Byte in einer Transaktion**.

**Was diesmal verloren ging, und warum das Glück war:** Laut Mitschnitt kamen alle Kommandos an;
verloren gingen die **Debug-Echos** `- request …` (verschmolzene Zeile 01:07:11.315). Für
Kommandos ESP→STM gibt es **weder Quittung noch Prüfsumme** — ein verstümmeltes `CMD S09…`
würde angenommen und ins EEPROM geschrieben. Es trifft ausgerechnet die **Update-Quelle**, den
Weg zu fremder Firmware aus **L42** (DIR-009).

**Verwandt, nicht dasselbe:** **L204** — dort war die EEPROM-Spur die erste Vermutung und hat
sich als **falsch** erwiesen (`ticker_set` schreibt gar nicht ins EEPROM). Das ist der Grund,
warum diese Spec die Blockade **erst misst** und dann behebt (Ent-3). **C3** (`BEFUNDE.md`,
Aufwand G) ist die Abhilfe an der Ursache und wird hier umgesetzt.

### 3. Teil C — jeder Wert reist doppelt über die Brücke, auch Geheimnisse

**✔ verifiziert (`http.cpp:14071-14079` für POST, `:14129-14143` für GET):** Der ESP schreibt
für jede Anfrage `- request <IP> [<label>]: <Anfragezeile>` auf die serielle Schnittstelle —
**mit vollständigem Querystring**. Jeder Setter-Wert geht damit ein zweites Mal über die Brücke
(Echo + `CMD`), bei langen Werten wie Update-Host und -Pfad über 100 Byte je Aufruf. Darunter
sind **Geheimnisse:** die Wetter-`appid` und, über die WLAN-Setter, der WLAN-Schlüssel.
Die Architektur-Checkliste §3 verlangt: „Keine WLAN-Credentials oder Gerätegeheimnisse in
Logausgaben, auch nicht im STM32-Logbuch der PWA."

**Abhängige Werkzeuge (✔ per `grep` am 09.10.2026):** Die volle Zeile wertet kein Werkzeug aus.
`tools/watch-log.sh:128` zeigt die Zeilen mit `request|GET|POST` vor einer `Exception (29)` als
Kontext — dafür genügt der Pfad. `tools/measure-log.sh`, `tools/diff-snapshot.sh`, `.claude/**`
und `app.js` lesen die Zeile nicht. Das Muster `- request` steht sonst nur in `http.cpp` und im
historischen `haenger-2026-09-30.md`.

---

## Ziel

1. Ein Wetterabruf blockiert den ESP **höchstens eine feste Gesamtgrenze unter 3 s**, im
   Normalfall deutlich unter 2 s, und eine verspätete Antwort wird gelesen statt verworfen.
2. Ein Zeichenketten-Setter blockiert den STM-Hauptloop **je EEPROM-Seite einen
   Schreibzyklus** statt je Byte — mit **byte-genau demselben EEPROM-Inhalt** wie heute.
3. Kein Anfragewert geht mehr als Debug-Echo über die Brücke.
4. Jede Wirkung ist **einzeln** am Gerät nachgewiesen, und jede neue Messzeile hat einmal den
   schlechten Wert gezeigt (DIR-014).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt**
(DIR-014, L177/L185). Prüfstände laufen über `tools/checks/auszug.sh` mit den Typbreiten des
Ziels (L256) und sind **einmal gegen die alte Fassung fehlgeschlagen**. **Was am Gerät nicht
herstellbar ist, wird nicht am Gerät verlangt.**

### Schritt E — ESP (Teil A und Teil C)

- [ ] **AKE.1 (Messzeile Wetter)** — Jeder Aufruf von `query_weather()` endet mit **genau
      einer** Zeile `- weather fc=<0|1> ms=<n> <ergebnis>`, `<ergebnis>` aus `ok`, `timeout`,
      `connfail`, `leer` (verbunden, aber keine auswertbare Zeile). Die Zeile geht **beide
      Wege** — `Serial` **und** `stm32_log_append()` — wie die Heap-Zeile
      (`ESP-uclock.ino:494-506`, C14/L185). Sie enthält **weder** `appid` **noch** Ort,
      Koordinaten oder URL. *Instrument:* Prüfstand t12 (Zeilenform je Ergebnis); am Gerät in
      G1 im Mitschnitt **und** in `/api/stm32_log`.
- [ ] **AKE.2 (Gesamtgrenze)** — `query_weather()` kehrt in **jedem** Fall spätestens nach der
      Gesamtgrenze `WEATHER_TOTAL_TIMEOUT_MS` (Vorgabe 2'500 ms, `design.md` §1.3) plus 100 ms
      zurück, gemessen ab Beginn des Verbindungsaufbaus — **soweit V.1 (5) bestätigt, dass
      `connect()` im verwendeten Core begrenzbar ist**; sonst ab Verbindungsaufbau erfolgreich,
      mit Beleg im Bericht. *Instrument:* Prüfstand t12 mit nachgebildeter Zeit, Fälle:
      Körper ohne abschliessendes `\n`; Server sendet nie; Server schliesst nie und sendet
      tröpfchenweise; Kopf ohne Leerzeile. **Gegenprobe:** Der Fall „Körper ohne `\n`" meldet
      gegen das alte `weather.cpp` ≥ 5'000 ms ⇒ FEHL.
- [ ] **AKE.3 (verspätete Antwort)** — Eine Antwort, deren erstes Byte zwischen 200 ms und der
      Gesamtgrenze eintrifft, wird **gelesen und geparst**. *Instrument:* Prüfstand t12, Fälle
      300 ms und 1'500 ms; **Gegenprobe:** gegen das alte `weather.cpp` wird nichts geparst ⇒
      FEHL.
- [ ] **AKE.4 (gleiches Parseergebnis)** — Für jede wohlgeformte Antwort, die das alte
      `weather.cpp` geparst hat, erhält `parse_weather()` bzw. `parse_weather_fc()` **dieselbe
      Zeichenkette** wie vorher: mit `Content-Length` ohne abschliessendes `\n`, mit
      `Transfer-Encoding: chunked`, Körper in einem und in mehreren Segmenten, Vorhersage mit
      grossem Körper. *Instrument:* Prüfstand t12 vergleicht die übergebene Zeichenkette alt
      gegen neu; Fallzahl gemeldet.
- [ ] **AKE.5 (Soft-Watchdog)** — Die Leseschleife gibt in jedem Durchlauf die Kontrolle ab
      (`yield()` oder `delay()`), wie es `Stream::timedRead` heute tut. *Instrument:* Review
      E.5 am Code; am Gerät kein Neustart in G1 (`watch-log.sh`).
- [ ] **AKE.6 (Gerät, L338)** — Nach dem ESP-Einspielen je **ein** `weather_get_now` und
      **ein** `weather_get_forecast` (G1): `v` in der `diag`-Folge steigt in den 6 s nach dem
      jeweiligen `weather`-Kommando **nicht**; die Messzeile zeigt `ok` und `ms` **unter
      2'000**; ihr Wert stimmt auf **±150 ms** mit dem Abstand zwischen `weather` und
      `WEATHER` im Mitschnitt überein. **Dieser Abstand ist das unabhängige Mass**, mit dem der
      Vorher-Wert aus S.26 (5,25 s / 5,3 s) bereits vorliegt. Dazu **lesend über mindestens
      60 Minuten Normalbetrieb** (≥ 3 automatische Abrufe): kein `v`-Anstieg im 6-s-Fenster
      nach einem `weather`-Kommando. Ein `v`-Anstieg durch einen ESP-Neustart (L327) zählt
      nicht und wird getrennt berichtet.
- [ ] **AKE.7 (Echo ohne Werte)** — Beide Echo-Stellen (GET und POST) laufen über **eine**
      Funktion. Die Zeile enthält Methode und Pfad und, falls ein Querystring vorhanden ist,
      den Marker `?…` — **kein Zeichen aus dem Querystring**. *Instrument:* Prüfstand t13 mit
      den Fällen Setter mit Wert, `appid`, `GET /?a`, ohne Query, nur `?`, POST mit Query;
      **statische Prüfung**, dass in `http.cpp` kein `Serial.print*` mehr `sRequest` oder
      `sParam` ausgibt — **gegen den Ausgangsstand schlägt sie an** (DIR-014).
- [ ] **AKE.8 (Gerät, Echo)** — Im Mitschnitt von G1 bis G3 enthält **keine** Zeile
      `- request` ein `=`. *Instrument:* `grep -a -- '- request' <mitschnitt> | grep '='`
      liefert nichts; die Zahl der geprüften `- request`-Zeilen wird gemeldet (sie muss
      grösser 0 sein, sonst hat die Prüfung nichts gesehen).

### Schritt M — STM, Messzeile

- [ ] **AKM.1 (Messzeile EEPROM)** — `eeprom_write()` gibt nach jedem Aufruf mit mindestens
      einem Schreibzyklus **genau eine** Zeile `eep a=<start> n=<anzahl> z=<zyklen>
      ms=<dauer> d=<vorher>/<nachher>` über `log_printf()` aus (Mitschnitt **und**
      `/api/stm32_log`, `src/log/log.c:31-33`). `d` sind die Werte von `uart_rxdrops` der
      ESP-Brücke vor und nach dem Schreiben. Kein Inhaltsbyte in der Zeile. Ohne Schreibzyklus
      keine Zeile. Wird über V.1 (2) ein periodischer Schreiber gefunden, der öfter als einmal
      je Minute schreibt, gilt zusätzlich eine Schwelle `ms ≥ 20` (`design.md` §2.2).
      *Instrument:* Quelltext (Review M.3); am Gerät G2.
- [ ] **AKM.2 (Selbstprüfung der Zeitbasis)** — In G2 gilt für jede Messzeile
      `15·z ≤ ms ≤ 17·z + 10`. *Instrument:* Auswertung der G2-Zeilen. **Verfehlt ⇒ Halt:** Dann
      misst die Zeile falsch, und kein Nachher-Wert aus ihr ist belastbar.
- [ ] **AKM.3 (Vorher, G2)** — Der Setter-Burst auf den Wetterort (`design.md` §4) liefert
      sieben Messzeilen mit `a` = Offset des Wetterorts, `n=32`, **`z ≥ 25`** bei jedem
      Wechsel lang↔kurz und **`ms ≥ 400`**. `d` wird berichtet; ein Anstieg ist **nicht**
      verlangt (`design.md` §4.2), bleibt `d` bei 0, darf G2 einmal mit 14 Aufrufen in rund
      3 s wiederholt werden (Ent-1).
- [ ] **AKM.4 (Flash)** — Testbau F103 nach M.1: Rest **≥ 1'024 Byte**. Darunter: Halt,
      Bericht an den Nutzer.

### Schritt C3 — STM, seitenweise schreiben

- [ ] **AKC.1 (Seitengrösse belegt)** — Die Konstante für die Seitengrösse entspricht dem
      Datenblatt **beider** verbauter Bausteine (V2: AT24C32M, `HARDWARE.md` U6; F103: der
      Baustein auf dem RTC-Modul) **oder** ist eine kleinere Zweierpotenz, die sie teilt. Quelle
      im Code-Kommentar und im Bericht von V.1. *Instrument:* Review C3.4.
- [ ] **AKC.2 (keine Seitengrenze überschritten)** — Jede I2C-Schreibtransaktion von
      `eeprom_write()` liegt **vollständig in einer Seite** (erste und letzte Adresse haben
      denselben Seitenindex), und je Seite gibt es **höchstens eine** Transaktion und **einen**
      Wartezyklus. *Instrument:* Prüfstand C3 mit nachgebildetem EEPROM, das **wie die Hardware**
      am Seitenende auf den Seitenanfang umbricht; jede Überschreitung ⇒ FEHL.
- [ ] **AKC.3 (byte-gleiches Ergebnis)** — Für eine Zufallsfolge von **mindestens 10'000**
      Schreibaufrufen (fester Startwert) über den **ganzen Adressraum 0..4095**, mit Längen von
      0 bis 600, und für **jeden Bereich aus `eeprom-data.h`** (voll, Teilbereich, ein Byte,
      unverändert, über Seitengrenzen) ist das EEPROM-Abbild **nach jedem Aufruf** identisch
      mit dem der alten Fassung, ebenso der Rückgabewert. Zyklen neu ≤ Zyklen alt; schreibt die
      alte Fassung 0 Zyklen, schreibt die neue 0. *Instrument:* Prüfstand C3, alte Fassung aus
      dem M-Stand als Referenz. **Gegenprobe (DIR-014):** zwei Sabotagen im Scratchpad —
      Seitengrösse 64 und Seitenende um eins verschoben — müssen **beide** FEHL melden; und das
      Zyklenkriterium aus AKC.2 meldet gegen die alte Fassung FEHL.
- [ ] **AKC.4 (Fehlerpfade)** — Scheitert das **Lesen** einer Seite, wird sie geschrieben (wie
      heute: unlesbar gilt als verschieden). Scheitert ein **Schreiben**, liefert
      `eeprom_write()` 0 und schreibt **keine** weitere Seite. `cnt == 0` liefert 1 ohne
      I2C-Verkehr; `eeprom_is_up == 0` liefert 0. *Instrument:* Prüfstand C3 mit eingespeisten
      Lese- und Schreibfehlern auf jeder Seite einer mehrseitigen Folge.
- [ ] **AKC.5 (Rechnung für S.26)** — Der Prüfstand meldet für die **S.26-Folge**
      (Update-Host 63 Byte ↔ kurz, Pfad 63 Byte ↔ kurz) und für die G2/G3-Folge (Wetterort)
      die Zyklenzahl alt und neu je Aufruf. Erwartet: alt bis 64 bzw. 32, neu **höchstens 3**
      bzw. **höchstens 2**. *Instrument:* Ausgabe des Prüfstands; die Zahlen gehen in den
      Bericht C3.8 neben die Gerätewerte.
- [ ] **AKC.6 (Nachher, G3)** — Derselbe Burst wie G2: jede Messzeile mit **`z ≤ 2`** und
      **`ms ≤ 60`**; `d` steigt **nicht**; die `diag`-Folge läuft ohne Lücke. *Instrument:*
      Mitschnitt mit `watch-log.sh`.
- [ ] **AKC.7 (Integrität am Gerät)** — (1) Der Abzug M2 **vor** dem C3-Flash und der Abzug
      **nach** Flash und STM-Reset sind gleich (`diff-snapshot.sh --soll`, nur flüchtige Felder
      verschieden). (2) Nach G3 wird der Originalwert des Wetterorts zurückgeschrieben und der
      STM zurückgesetzt; danach zeigt das Gerät den **Originalwert**, der STM hat ihn also
      **aus dem EEPROM** gelesen. (3) Nach dem `pwa-tester`-Durchlauf C3.9 ein STM-Reset, danach
      zeigt Phase 9 die Einstellungen, wie der Durchlauf sie hinterlassen hat.
- [ ] **AKC.8 (Flash)** — Testbau F103 nach C3.1: Rest **≥ 1'024 Byte**; Gegenprobe in
      C3.6 aus S8b. Darunter: Halt, Bericht an den Nutzer.

### Für alle Schritte

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `./tools/guardrails.sh --full` vor jedem Release mit Exit 0 (Lead, R1);
      `./tools/checks/auszug.sh` meldet **drei** Prüfstände mehr als vorher (t12, t13, C3).
- [ ] **AKZ.3** — `./tools/smoke-device.sh` nach jedem Einspielen ohne Fehlschlag, nach dem
      ESP-Update **samt Update-Quelle** (DIR-009); `./tools/install-app.sh --check` nach dem
      ESP-Update (DIR-017). **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.4** — **Ein `pwa-tester`-Durchlauf Phasen 0–4 und 9 nach dem C3-Release** (C3.9).
      Begründung: C3 wirkt auf **jeden** EEPROM-Schreibvorgang, Phase 2 speichert viele
      Einstellungen. Unvollständig ⇒ **nicht abgenommen**. Für E und M **kein** eigener
      Durchlauf: je eine Komponente, gezielte Nachweise G1/G2 (DIR-012 verlangt ihn erst ab zwei
      Komponenten in einem Release).
- [ ] **AKZ.5** — `./tools/watch-log.sh` läuft bei jedem Gerätelauf mit (DIR-013) und meldet
      nach E.4 auch `- weather … timeout` und Messzeilen mit `ms` über der Grenze.
- [ ] **AKZ.6** — Jedes ausgerollte Release **sofort** committet, getaggt, **gepusht**
      (DIR-011).
- [ ] **AKZ.7** — Die Einspielzeile nennt die **Reihenfolge**, fertig zum Einfügen; der Nutzer
      spielt **jedes** Update selbst ein.
- [ ] **AKZ.8** — Kodierung **und** Zeilenende jeder Datei vor dem Patch festgestellt und im
      Bericht genannt (DIR-015): `weather.cpp`, `http.cpp` (UTF-8, eine CRLF-Zeile laut
      `CLAUDE.md`), `src/eeprom/eeprom.c`.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **(f) Prüfsumme für Kommandos ESP→STM** | **Nur sie** schützt gegen einen verstümmelten, aber angenommenen `CMD S09…` (L42-Risiko). Gleiche Gattung wie **A32** in der Gegenrichtung. Eigene Entscheidung des Nutzers, als **neuer Befund und ToDo** vorgeschlagen (**Ent-4**). C3 senkt die Wahrscheinlichkeit eines Überlaufs, nicht die Folge eines verstümmelten Kommandos |
| **(d) Grösserer Empfangsring** | Nur Notbehelf: verschiebt die Schwelle, beseitigt die Blockade nicht; kostet RAM auf dem F103 |
| **(a) Quittung für `CMD`** | Jetzt nicht. Verändert das Protokoll beider Seiten; die Lehren aus L266 gelten |
| **Asynchroner Wetterabruf** (Zustandsmaschine in `loop()`) | Grösserer Umbau; die gemessene Ursache ist das ungebremste Lesen, nicht die Synchronität (`design.md`, Verworfene Alternativen) |
| **ACK-Polling statt fester 15 ms in `eeprom_waitstates()`** | Würde je Seite weitere Millisekunden sparen; eigener Schritt, eigene Messung. `EEPROM_WAITSTATES` bleibt unverändert |
| **`write_update_host_to_eep()` / `…path…` nur bis zum Abschlussbyte schreiben** | C3 behebt die Ursache für **alle** Schreiber; ein Sonderweg an zwei Aufrufern wäre eine zweite Regel |
| **Flash-Pfad `BLACK_BOARD`** (`flash_write()`) | Nicht berührt; C3 ändert nur `src/eeprom/eeprom.c` |
| **Weitere Stellen, die Anfragewerte über `Serial` ausgeben** | **Melden, nicht mitkorrigieren** (E.2 nennt sie im Bericht) |
| **L327 / A55** (`v` steigt beim ESP-Neustart) | Eigener Befund; in AKE.6 nur getrennt ausgewiesen |
| **L349 / C49** (Schreibfehler des ESP unter Testlast) | Eigener Befund |
| **Schreibende Tests auf Update-Host und -Pfad** | Ausdrücklich ausgeschlossen. Der Gerätenachweis läuft auf dem Wetterort (`design.md` §4) |
| **PWA, Legacy-Oberfläche** | Unverändert. Keine PWA-Version, kein `CACHE_NAME` |

---

## Betroffene Laufzeiten

- [x] **ESP8266** — Schritt E: `weather.cpp`, `http.cpp`; ein OTA
- [x] **STM32** (`src/**`) — Schritt M: `src/eeprom/eeprom.c` (Messzeile); Schritt C3:
      `src/eeprom/eeprom.c` (seitenweise); je ein Flash
- [ ] **PWA** — nicht betroffen
- [x] **Werkzeug** (`tools/**`) — Prüfstände t12, t13, C3 unter `tools/checks/auszug/`,
      Eintrag in `auszug.sh`, statische Echo-Prüfung, `watch-log.sh` (E.4)
- [x] **Dokumentation** — `BEFUNDE.md` (L338, C43, L339, A58, C3, neuer Befund nach Ent-4),
      `knowledge/architecture-checklist.md` (EEPROM-Kosten, Ringgrösse), `CLAUDE.md`
      (Offene Themen, Hardware-Hinweis „16 ms pro Byte")
- [x] **Build/Release** — drei Releases bei Ent-3 = V1 (ESP; STM Messzeile; STM C3), je mit
      Commit, Tag, Push; dazu zwei Testbauten F103 ohne Release

---

## Entscheidungen

Stand **09.10.2026** — **offen**.

- **Ent-1 — Freigabe der Spec und der schreibenden Gerätenachweise.** Freizugeben sind:
  **G1** (je ein `weather_get_now` und `weather_get_forecast`; schreibt keine Einstellung,
  löst aber den Wetterticker an der Uhr aus), **G2** und **G3** (je sieben
  `weather_city_set` mit eigenen Testwerten in 1,5 s, danach Originalwert mit ≥ 4 s Abstand
  zurück und **STM-Reset** über `maintenance_reset_stm32`; bei `d = 0` in G2 eine Wiederholung
  mit 14 Aufrufen), und der **`pwa-tester`-Durchlauf** C3.9 mit seinen üblichen schreibenden
  Phasen. **Nicht** Teil der Freigabe: Update-Host, Update-Pfad, WLAN-Setter.
- **Ent-2 — Bleibt die Instrumentierung im Fabrikat?** *Empfehlung: ja, beide Zeilen.* Die
  Wetterzeile erscheint je Abruf (rund dreimal je Stunde plus Aufrufe aus der PWA), die
  EEPROM-Zeile nur bei echten Schreibzyklen. Kosten: Flash auf dem F103 (gemessen in M.2),
  rund 30 Byte je Zeile auf der Brücke. **Bei „nein"** werden sie im **nächsten regulären
  Release** der jeweiligen Komponente entfernt, **nicht** in diesem Paket — sonst gibt es keinen
  Nachher-Nachweis.
- **Ent-3 — Wie wird der Vorher-Zustand für L339 belegt?** *Empfehlung: V1.*
  - **V1 — drei Releases:** ESP (E), dann STM **nur mit Messzeile** (M), messen (G2), dann
    STM mit C3. Die Messzeile zeigt einmal den schlechten Wert (DIR-014), und Vorher und Nachher
    laufen unter **demselben ESP** — nur C3 unterscheidet sie. **Grund für die Empfehlung:** In
    L204 war die EEPROM-Spur schon einmal die naheliegende Erklärung und **falsch**. Kosten: ein
    zusätzliches STM-Release mit Flash durch den Nutzer.
  - **V2 — zwei Releases:** ESP **und** Messzeile in einem Release, dann C3. Spart einen
    Release-Zyklus, aber ein Release mit zwei Komponenten verlangt einen `pwa-tester`-Durchlauf
    (DIR-012), und L338 ist nicht mehr isoliert zuzuordnen.
  - **V3 — zwei Releases ohne STM-Zwischenstufe:** Vorher aus S.26 (`d` 0 → 191) plus
    Zyklenzahl alt/neu aus dem Prüfstand (AKC.5); die Messzeile fährt mit C3 und wird nur
    über AKM.2 (`15·z ≤ ms ≤ 17·z + 10`) auf ihre Zeitbasis geprüft. **Schwäche:** S.26 lief auf
    Host und Pfad mit vollem Echo; der Nachweis in G3 läuft auf dem Wetterort mit gekürztem
    Echo. Dass der Hauptloop **tatsächlich** in `eeprom_write()` stand, bleibt dann aus der
    Analyse erschlossen, nicht gemessen.
- **Ent-4 — (f) Prüfsumme für Kommandos ESP→STM.** *Empfehlung:* Jetzt als **neuen Befund mit
  ToDo** anlegen (Kennung vergibt der Lead, Familie A32/L42), über den Umfang und den
  Zeitpunkt eines eigenen Pakets **nach Abschluss dieses Pakets** entscheiden. Grund: C3 senkt
  die Häufigkeit eines Ringüberlaufs, aber ein Überlauf aus anderer Ursache (L204-Gattung,
  künftige Last) träfe die Update-Quelle unverändert ohne Schutz.
