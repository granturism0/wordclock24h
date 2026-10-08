# Changelog

## 2026-10-08 Runde F: Flash-Überwachung, Abweisen statt Kürzen (ESP 3.2.26)

Nur der ESP, STM und PWA unverändert. Release-ZIP
`wordclock-release-2026-10-08-015454.zip`, Tag `release/3.2.22-3.2.26-1.4.93`. Umfang:
Runde F aus `specs/paket-2026-10-06/`. Am 08.10.2026 um 01:56 per OTA eingespielt, Abnahme F.10 bestanden (Smoketest 31/0, zum ersten Mal ohne den Verlustzähler aus L270), Probeflash des STM über den neuen Flashweg bestanden (F.11).

**Einspielreihenfolge: nur der ESP.** Danach wird derselbe STM-Stand 3.2.22 noch einmal
geflasht, als Probe des neuen Flashwegs (F.11). Die PWA-Anpassungen — Meldungen für die
Kennungen 7 und 8, die leere Liste bei unbekannter Hardware, die Feldlängen für SSID und
Schlüssel — folgen in einer eigenen Runde **nach** diesem OTA, damit die Firmware zuerst
auf dem Gerät ist (L241). Bis dahin zeigt die PWA für 7 und 8 ihren allgemeinen
Fehlertext samt Detail.

### Die Legacy-Seite flasht nur noch passende STM-Dateien (C9c6, L179, C28/L272)

Am 04.10.2026 landete über die Legacy-Seite ein F103-Abbild auf dem F411 und legte die
Uhr still. Die API prüfte den Dateinamen, die Legacy-Seite nicht. **Jetzt gehen Flashzweig
und Auswahlliste der Legacy-Seite über dieselbe Prüfung wie die API.** Ein unpassender
Name wird abgewiesen, und die Seite sagt „Nicht geflasht“.

Ist die Hardware nicht erkannt (Kennung 65535, etwa nach einem ESP-Neustart), bietet die
Liste **keine** Datei an statt aller. Darunter steht ein Hinweis mit einem Knopf „Reset
STM32“, der die Kennung zurückholt, und dem Rückweg über „Local Update“, falls der Reset
nicht hilft.

Dazu passt jetzt der Präfix `uc-` (uclock-Layouts): Bisher setzte die Prüfung ein
sechs Zeichen langes Präfix voraus, und ein `uc-`-Name passte nie, auch der eigene
Standardname nicht (Review A2). Jeder Name, der bisher passte, passt weiter.

### Eigene Kennungen für „fehlgeschlagen“ und „leer“ (Ent-4, C27, C34)

- **Kennung 7 heisst „Aktion fehlgeschlagen“.** `fs_remove` meldete ein gescheitertes
  Löschen bisher mit Kennung 6, also wie „Datei gibt es nicht“. Kennung 6 heisst jetzt nur
  noch „nicht gefunden“. Auch eine Datei, die `fs_show` nicht öffnen kann, meldet 7.
- **Kennung 8 heisst „Datei ist leer“.** `fs_show` lieferte eine 0-Byte-Datei bisher als
  200 OK mit leerem Rumpf aus — ausgerechnet den Zustand, der in der PWA zum weissen
  Bildschirm führt (C27, L271).
- Bei einem Lesefehler drehte `fs_show` endlos, bis der Soft-Watchdog den ESP neu
  startete. Jetzt bricht es ab (C34, L309).

### Die API-Liste bei unbekannter Hardware ist leer (Ent-5)

Bei Kennung 65535 liefert auch `/api/update_status` keine STM-Dateien mehr, und der
Abruf beim Update-Server entfällt ganz, weil das Ergebnis ohnehin feststeht (F.3).

### Abweisen statt still kürzen

- **SSID und WLAN-Schlüssel** über 31 bzw. 63 Byte werden abgewiesen, statt gekürzt
  gespeichert und mit Erfolg quittiert (C38, L323, Review A4). Das gilt für den Client,
  den Accesspoint, den EEPROM-Satz und die Legacy-WLAN-Seite. Bisher verband sich das
  Gerät mit dem vollen Wert und speicherte den gekürzten — die nächste Verbindung kam
  dann nicht zustande, ohne dass eine Oberfläche einen Grund nannte. Beim EEPROM-Satz
  werden alle vier Werte geprüft, bevor einer geschrieben wird.
- **Der Overlay-Text** über der Grenze wird abgewiesen statt gekürzt (L319). Die PWA
  prüfte das schon, der ESP kürzte an ihr vorbei.
- **TFT- und DFPlayer-Glocken-Flags:** Fehlt einer der Parameter, wird der Aufruf
  abgewiesen. Bisher hiess „fehlt“ dort „aus“, und ein unvollständiger Aufruf löschte
  Flags (E12).
- **Das Legacy-Overlay-Formular** prüft `type`, `date_code` und `days` wie die API, mit
  demselben Wortlaut. Bisher wurde „abc“ zu 0, und Typ 99 ging unbesehen an den STM
  (C6u).

### Diagnose und Abrufe

- **Der Verlustzähler trennt die Ursachen (C25).** „Verbindung schon abgebaut“ — meist
  ein Browser, der den Abruf abbricht (L270) — und „Schreiben gescheitert“ werden getrennt
  gezählt. `device_ready` meldet beide Zahlen, und der Smoketest wertet nur noch das
  gescheiterte Schreiben als Fehler.
- **`/api/stm32_log` liefert gültiges JSON in UTF-8 (C23).** Bytes aus dem STM
  (ISO-8859-1, etwa „Überwiegend bewölkt“) werden gewandelt, Steuerzeichen als `\u00XX`
  maskiert. Strenge JSON-Leser brachen hier bisher ab.
- **Der Kopf einer Serverantwort wird abgewartet (C9k, Review A3).** Kam er in zwei
  Teilen, galt er bisher nach dem ersten als fertig. Jetzt wartet der ESP auf jedes
  Zeichen, und der Kopf als Ganzes ist zeitlich begrenzt; ein abgerissener Kopf gilt als
  Fehlschlag.
- **Dateinamen vom Update-Server werden auf der Legacy-Seite maskiert** (Review A6).
  Sie kommen von einem einstellbaren Host über HTTP und gingen bisher roh ins HTML.

### Intern

Die Prüfstufe S16 umfasst jetzt 21 Prüfstände. Die acht neuen der Runde F schlagen gegen
ESP 3.2.25 an und bestehen gegen 3.2.26. Der Prüfstand t11 prüft die Dateinamensregel
über alle 4096 Hardwarekennungen, zusammen 1'574'304 Einzelprüfungen.

**Gerätetest offen:** Abnahme F.10 und Testdurchlauf F.12 stehen aus; erst danach werden
die Befunde in `BEFUNDE.md` geschlossen (F.13).

## 2026-10-08 Runde P3: Minusgrade und halbe Grad in der PWA (PWA 1.4.93)

Nur die PWA. Setzt STM 3.2.22 und ESP 3.2.25 voraus — beide sind seit dem 07./08.10.2026
auf dem Gerät, die Reihenfolge Firmware vor PWA (L241) ist damit eingehalten. Release-ZIP
`wordclock-release-2026-10-08-011508.zip`, Tag `release/3.2.22-3.2.25-1.4.93`. Am
08.10.2026 um 01:43 per `install-app.sh` aufgespielt.

### Die RTC-Temperatur zeigt halbe Grad und Minusgrade (A5, PWA-Teil)

Die PWA liest die RTC-Temperatur aus der neuen Variable (Index 49, halbe Grad mit
Vorzeichen). Fehlt sie (alter ESP) oder meldet sie „kein Messwert“ (alter STM, Lesefehler),
gilt wie bisher Index 21. Bei negativen Werten rundete die Anzeige bisher falsch: Aus
−1,5 °C wurde „−2.5 °C“. Behoben. Minusgrade sind am Gerät nicht herstellbar; belegt sind sie
in der Vorschau (`tools/ui-mess/proben/runde-p3.mjs`). Am Gerät: PWA „29.0 °C“, Legacy
„29 °C“, Index 21 = Index 49 = 58.

### Eine WLAN-Auswahl geht nicht mehr verloren (B41, L326)

Steht das verbundene Netz nicht in der Scanliste, hing der Schutz einer gewählten SSID an
der Liste offener Änderungen — und die wurde beim Speichern eines anderen Feldes geleert.
Danach sprang die Auswahl auf den ersten Eintrag, und ein Speichern nur des Schlüssels hätte
die Uhr in ein fremdes Netz geschickt. Jetzt merkt sich die Liste eine echte Wahl, bis das
Gerät sie selbst meldet oder Du beim Modulwechsel verwirfst. Zwei Review-Befunde (M1, M2)
sind vor der Auslieferung behoben.

### Geprüft

Probe `runde-p3.mjs` 34/34, gegen 1.4.92 14 Fehlschläge. `runde-p2` 52/52,
`b40-spaeter-scan` 12/12, `runde-p` 84/84. Am Gerät `check-pwa.sh` 8/0, Logwache ruhig.

## 2026-10-08 Runde S, STM-Teil: Brücke mit Abschlussmarke, Spiele ohne Watchdog-Reset, halbe Grad (STM 3.2.22)

Nur der STM, ESP und PWA unverändert. Release-ZIP
`wordclock-release-2026-10-08-001731.zip`, Tag `release/3.2.22-3.2.25-1.4.92`. Umfang:
Runde S aus `specs/paket-2026-10-06/`, STM-Teil. Am 08.10.2026 um 00:18 per
`flash-stm.sh` auf den F411 geflasht.

**Einspielreihenfolge: erst ESP 3.2.25, dann der STM — beides ist erledigt.** Der ESP
musste zuerst kommen, weil er die neuen Brückenzeilen dieses Releases verstehen muss.

### Die Brücke weiss jetzt, wann der Abgleich vollständig ist

- **Der Vollabgleich hat einen Rahmen.** Der STM eröffnet ihn mit `VB` und schliesst ihn
  mit der Abschlussmarke `VE`. Bisher hielt sich der ESP für geheilt, sobald die
  Hardwarekennung angekommen war — das dritte von rund 194 Kommandos (A42, L260).
  **Die Marke heisst „STM durchgelaufen“, nicht „alles quittiert“** (Review H4).
- **Eine verspätete Quittung bestätigt nicht mehr das falsche Kommando.** Der STM liest
  die Zuordnungszeile `ACK xy` und verwirft eine Quittung mit fremder Zuordnung, statt
  sie dem wartenden Kommando gutzuschreiben. Gezählt wird das im neuen Diag-Feld `a=`
  (A35, Teil 1). Ein `!v` geht immer als Ablehnung durch (Review H1).
- **Die Markenpflicht des ESP greift jetzt** für Hardwarekennung, Update-Host und
  Update-Pfad (C26). Für die übrigen Zeilenarten bleibt es beim bisherigen Verhalten.

Am Gerät nach dem Flash: „Variablensatz vollständig … (Abschlussmarke)“, unmarkierte
Zeilen 0, `a=0`.

Offen bleibt A35 Teil 2: Der Punkt heisst weiter „gelesen“, nicht „übernommen“.

### Der IP-Lauftext kommt nach dem Abgleich

Beim Start lief die IP-Adresse bisher mitten in den Variablenabgleich hinein, und der
Abgleich lief dabei verschachtelt. Jetzt merkt der STM ihn nur vor und zeigt die
Adresse danach (A39). Dazu heisst `pending_weather_ticker_restore` jetzt
`pending_ticker_restore`, weil es längst alle Ticker trägt; die Restore-Bedingung ist
unverändert (E17).

### Tetris und Snake reissen den Watchdog nicht mehr

Beide Spiele hielten den Hauptloop an, ohne den Watchdog zu bedienen; nach rund 20 s
setzte er die Uhr zurück (A16). **Am Gerät belegt:** Tetris lief 75 s (00:22:25 bis
00:23:40) ohne Watchdog-Reset, die Uptime lief durch, `diag` 21 → 22. Ein erster Lauf
ohne Eingabe endete nach rund 18 s mit Game Over und belegt deshalb nichts; erst der
zweite Lauf mit Bewegungsbefehlen. Am Gerät belegt ist Tetris, nicht Snake.

Noch offen ist A53: Während eines Spiels gehen ESP-Zeilen verloren, und dieses Fenster
begrenzt jetzt kein Watchdog mehr.

### Die RTC-Temperatur kann jetzt halbe Grad zeigen — und Minusgrade

- **Das halbe Grad kommt aus dem richtigen Bit** (Bit 7 statt Bit 1). Die Uhr hat vorher
  nie ein halbes Grad gemeldet, belegt mit 5'028 Messungen (A54, L325).
- **Echte Minusgrade gehen als Index 49 über die Brücke**, in halben Grad mit Vorzeichen
  (A5). Die Legacy-Temperaturseite des ESP 3.2.25 zeigt sie damit an.
- **Die Anzeige an der Uhr bleibt, wie sie war** — mit einer Ausnahme: Bei Minusgraden
  zeigt sie jetzt 0 °C statt 125 °C (Ent-8).

Am Gerät: Index 49 = Index 21 = 58, also 29 °C; die Legacy-Seite zeigt dasselbe. Die
PWA zeigt Index 49 noch nicht, das folgt in Runde P3.

### Sicherheit und Robustheit

- **Keine Werte mehr in der Timeout-Meldung** (A20). Bisher konnte dort der Wetter-Schlüssel
  oder der Update-Host im Klartext stehen; jetzt nur Kennung, Index und Länge.
- **Overlays:** Die Anzahl ist auf 32 begrenzt und der Overlay-Index wird gegen die
  Feldgrösse geprüft (A3, A48). Das schliesst die STM-Seite der Overlay-Lücke, deren
  ESP-Seite ESP 3.2.25 geschlossen hat. **Wer genau 32 Overlays hat, behält sie jetzt
  über einen Neustart** — bisher gingen dann alle verloren (A51).
- **Abschlussbyte bei allen 16 Kopien** in den gemeinsamen Empfangspuffer (A13, A47).
- **Wortindizes** der Zeittabellen werden vor dem Schreiben geprüft (A24), der
  Variablenindex beim Senden maskiert (A14).
- **Drei Verwerfungspfade** laufen jetzt über den gemeinsamen Zähler (A46), und der erste
  Fall je Grund wird gemeldet statt nur gezählt (A49).
- **Der LDR-Rohwert geht ungeklammert an den ESP** (A9). Beim Kalibrieren siehst Du damit
  auch Werte jenseits der alten Grenzen. Die Kennlinie der Automatik ist byte-gleich.
- Dazu eine Typbereinigung in der Snake-Animation (A45), auf dem Ziel ohne Wirkung.

### Flash

F103: +236 Byte gegenüber 3.2.21, 1'688 Byte frei, die Reserve von 1'024 Byte ist
eingehalten. Kein Punkt zurückgestellt. Je Schritt gemessen: Kern +168, A5 +28,
A20 −40, A13+A47 −32, A3+A48+A51 +48, die übrigen sechs +92.

**Abweichung von der Spec:** Die letzten sechs Punkte liefen in einem Zug mit **einem**
Testbau statt je einem. Die Schätzung lag bei höchstens 320 Byte bei 728 Byte Luft; die
Gesamtmessung (+92) bestätigt die Einzelschätzung (+91).

### Intern

14 Auszugs-Prüfstände laufen als neue Guardrail-Stufe S16 (`tools/checks/auszug.sh`),
gegen den aktuellen Stand 14 von 14. Gegen den Stand vor Runde S
(`release/3.2.21-3.2.24-1.4.92`) schlagen alle 14 an, jeder an seinem eigenen Gegenstand.
Im ersten Anlauf liessen sich drei dort gar nicht übersetzen, weil ihr Aufruf den neuen
Zähler fest voraussetzte; jetzt leiten sie das aus dem geprüften Stand ab.
`tools/diff-snapshot.sh` führt Index 48 jetzt als flüchtig (L332).

Smoketest 30/0, `check-pwa.sh` 8/0.

### Was zu flashen ist

**Nur der STM**, bereits eingespielt — nach ESP 3.2.25. Die PWA bleibt, wie sie ist.


## 2026-10-07 Runde S, ESP-Teil: Overlay-Index geprüft, keine Schlüssel im Mitschnitt (ESP 3.2.25)

Nur der ESP, STM und PWA unverändert. Release-ZIP
`wordclock-release-2026-10-07-231825.zip`, Tag `release/3.2.21-3.2.25-1.4.92`. Umfang:
Runde S aus `specs/paket-2026-10-06/`, Tasks S.1 bis S.5. Am 07.10.2026 um 23:19 per
OTA eingespielt.

**Einspielreihenfolge: ESP zuerst — das ist erledigt.** Der STM folgt in derselben
Runde; die Brückenteile dieses Releases warten auf ihn.

### Eine Lücke aus dem ganzen LAN geschlossen

Die Overlay-Seite der Legacy-Oberfläche schrieb mit einem ungeprüften Index in die
Overlay-Tabelle des ESP. Auslösbar war das aus dem ganzen Heimnetz, auch über ein
eingebettetes Bild auf einer fremden Seite. **Jetzt prüft `http_overlays()` den Index,
bevor irgendetwas geschrieben wird**, auch im Zweig, der ein Overlay anzeigt. Zusätzlich
begrenzt der ESP eine vom STM gemeldete Overlay-Zahl über 32 beim Lesen auf 32 und weist
sie beim Löschen ab (`BEFUNDE.md`, L308). Die Gegenseite im STM folgt mit dem
STM-Release dieser Runde.

### Deine WLAN-Schlüssel stehen nicht mehr im Mitschnitt

Beim Start gab der ESP den WLAN- und den AP-Schlüssel im Klartext aus. Diese Zeilen
laufen über die Brücke zum STM und landeten so im seriellen Mitschnitt auf dem Pi.
**Jetzt steht dort nur noch „gesetzt“ oder „leer“.** Am Gerät belegt im Mitschnitt von
23:19:12 (`BEFUNDE.md`, L298). Bereits abgelegte Logdateien enthalten die Schlüssel
weiterhin; was mit ihnen geschieht, entscheidest Du.

### Die Brücke ist für den nächsten STM vorbereitet

Mit dem heutigen STM 3.2.21 ändert sich am Verhalten nichts. Der ESP versteht jetzt:

- die **Eröffnungszeile** `VBffnn` und die **Abschlussmarke** `VEnn` des Vollabgleichs.
  Daran erkennt er, ob der Variablensatz nach einem Neustart vollständig angekommen ist;
- eine **Zuordnungszeile** `ACK xy` vor jedem Quittungspunkt und jedem `!v`, damit eine
  verspätete Quittung nicht mehr das nächste Kommando bestätigt (A35, Teil 1);
- eine **Markenpflicht** für Hardwarekennung, Update-Host und Update-Pfad, sobald der
  STM seine Zeilen mit Prüfsumme markiert (C26). Eine beschädigte Zeile dieser drei Arten
  wird dann abgewiesen statt übernommen.

**Die Markenpflicht hebt sich nach drei unmarkierten Zeilen in Folge selbst auf.** Das
kam aus dem Review (H1): Ohne diese Rückfallregel hätte die Pflicht nach einem
ESP-Update Hardwarekennung und Update-Quelle sperren können — genau die beiden Werte,
deren Verlust die Uhr auf den Update-Server des Ursprungsprojekts zeigen lässt.

Am Gerät belegt: Der ESP meldete nach dem OTA „Variablensatz vollständig … (ohne
Eröffnungszeile)“, also den bisherigen Rückfallweg. Der alte STM schreibt die
`ACK`-Zeilen nur auf seine Debug-UART; im Mitschnitt stehen 325 davon.

### Minusgrade auf der Legacy-Temperaturseite, sobald der STM sie liefert

Der ESP kennt eine neue Variable, Index 49: die RTC-Temperatur in halben Grad **mit
Vorzeichen**, `0x8000` heisst „unbekannt“. Die Legacy-Temperaturseite zeigt damit
Minusgrade, sobald der STM diesen Wert sendet (A5, ESP-Teil). **Bis dahin fällt sie auf
den bisherigen Wert zurück** — am Gerät unverändert 29 °C.

### Intern

Vier Prüfstände, jeder einmal gegen die alte Fassung fehlgeschlagen und gegen die neue
bestanden: Brücke 22/22, Schlüsselausgabe 4/4, Index 49 14/14 und 2/2, Overlay-Index
21/21.

### Was zu flashen ist

**Nur der ESP**, bereits eingespielt. Die PWA bleibt, wie sie ist. Der STM folgt in
derselben Runde.


## 2026-10-07 Runde P2: Eingaben im Netzwerkmodul bleiben stehen (PWA 1.4.92)

Nur die PWA, STM und ESP unverändert. **Keine Firmware vorausgesetzt.** Release-ZIP
`wordclock-release-2026-10-07-223610.zip`, Tag `release/3.2.21-3.2.24-1.4.92`. Umfang:
Runde P2 aus `specs/paket-2026-10-06/`.

**Am Gerät (07.10.2026):** `install-app.sh --check` vollständig, `check-pwa.sh` 8/0,
Smoketest 29/1 (der bekannte Verlustzähler, L270). Lesende Abnahme P2.8 mit einem
Wächter, der nur lesende Abrufe durchlässt: Eingaben in Zeitserver und Zeitzone
überleben den nächsten Scan, die WLAN-Auswahl bleibt auf dem verbundenen Netz,
wiederhergestellter Ausgangswert ohne Dialog, kein Schreibabruf versucht — 8/0. Logwache
durchgehend ruhig.

### Ein später WLAN-Scan überschreibt Deine Eingabe nicht mehr

Beim Öffnen des Netzwerkmoduls lädt die PWA die WLAN-Liste. Kam die Antwort erst,
nachdem Du im Zeitserver oder in der Zeitzone getippt hattest, stand danach wieder der
alte Gerätewert im Feld — und „Speichern“ schickte still diesen alten Wert, ohne
Meldung. Bei der Zeitzone hiess das: Du hieltest die neue für gespeichert, die Uhr lief
weiter nach der alten. **Jetzt bleibt ein Feld stehen, in dem Du tippst oder getippt
hast.** Ein unberührtes Feld übernimmt den Gerätewert wie bisher (`BEFUNDE.md`, L321).

Dasselbe gilt für die **WLAN-Auswahl:** Ein Netz, das Du gewählt, aber noch nicht
gespeichert hast, springt bei einem neuen Scan nicht mehr auf das verbundene zurück. Neu
gefundene Netze erscheinen weiterhin in der Liste.

**Am Gerät mit Speichern noch nicht nachgewiesen.** Der Fehler ist ein Wettlauf und
lässt sich an der Uhr nicht gezielt herstellen. Belegt ist die Korrektur über eine Probe,
die die Scan-Antwort absichtlich zurückhält. Den Nachweis mit Speichern liefert der
nächste volle Testdurchlauf (S.26).

### Kein Dialog mehr, wenn Du eine Eingabe zurücknimmst — in vier Feldern

Wer in **AP-SSID, Zeitserver, Zeitzone oder WLAN-Auswahl** etwas ändert und danach den
Ausgangswert wiederherstellt, bekommt beim Modulwechsel keine Warnung „Es gibt
ungespeicherte Änderungen“ mehr. **Das gilt nur für diese vier Felder.** In allen
anderen zählt jede Eingabe weiter als Änderung, auch wenn danach wieder derselbe Wert
dasteht.

### Overlay-Text wird vor dem Absenden geprüft

Der Text eines Overlays — Icon-Name oder Lauftext — darf höchstens 32 Byte lang sein;
Umlaute zählen doppelt. Die PWA prüft das jetzt **im Overlay-Editor und beim Import**
einer Sicherung, mit derselben Byte-Prüfung wie bei den übrigen Textfeldern. Im Editor
geht ein zu langer Text nicht hinaus. Beim Import wird das Overlay übersprungen und in
der Abschlussmeldung mit Länge und Grenze genannt. Weil der Import die Overlay-Plätze
vorher leert, **fehlt ein übersprungenes Overlay danach auf der Uhr**; die Meldung sagt
das ausdrücklich.

**Das Gerät selbst kürzt einen zu langen Overlay-Text heute noch still** und meldet
Erfolg (`BEFUNDE.md`, L319). Bis zum ESP-Update ist die Prüfung in der PWA der einzige
Schutz.

### Übersetzt

„Keine WLANs gefunden“ und die Statuszeile im Netzwerkmodul (SSID, IP, Modus) standen
als feste deutsche Texte im Code. Sie erscheinen in der englischen Oberfläche jetzt auf
Englisch.

### Während der Entwicklung gefunden und vor der Auslieferung behoben

Ein Zwischenstand dieser Runde zeigte in der WLAN-Auswahl ein fremdes Netz, wenn das
verbundene im Scan nicht an erster Stelle stand. Wer danach nur einen neuen
WLAN-Schlüssel speicherte, hätte die Uhr ins falsche WLAN geschickt. Das war ein
Rückschritt innerhalb von P2, gefunden im Nachreview. **Ausgeliefert wurde er nie, und
1.4.91 war nicht betroffen.**

### Was offen bleibt

Steht das verbundene Netz gar nicht im Scan — etwa im AP-Rückfall —, kennt die
WLAN-Auswahl keinen Ausgangswert. Dann geht eine gewählte, aber nicht gespeicherte
SSID nach einem **anderen** Speichervorgang in derselben Sektion verloren, zum Beispiel
nach dem Speichern des Zeitservers. Die Auswahl springt auf den ersten Eintrag, und ein
folgendes Speichern des WLAN-Schlüssels ginge an diese SSID. **Prüf in diesem Fall die
Auswahl, bevor Du den Schlüssel speicherst.** Kein Rückschritt: 1.4.91 verlor eine
offene Auswahl bei jedem Scan (`BEFUNDE.md`, L326).

### Intern

Drei Proben gegen die Vorschau unter `tools/ui-mess/proben/`: `runde-p2.mjs` (52 Fälle),
`b40-spaeter-scan.mjs` (12 Fälle; hält die Scan-Antwort tatsächlich zurück und prüft
zuerst, dass sie ankommt) und `runde-p.mjs` (84 Fälle), die L303 jetzt auch in der
Gegenprobe prüft. Damit sind die beiden Korrekturen aus 1.4.91, die einen
fehlschlagenden Schreibaufruf brauchen — keine „formatiert“-Meldung nach misslungenem
Formatieren (L303) und kein Knopf, der nach einem Fehler auf „schaltet…“ stehen bleibt
(L306) —, über die Probe belegt. Am Gerät wird das bewusst nicht hergestellt.

### Was zu flashen ist

**Nur die PWA**, mit `./tools/install-app.sh`. Keine Firmware vorausgesetzt.


## 2026-10-05 Runde P: Sprachwechsel, Byte-Prüfung, Abweisungen (PWA 1.4.91)

Nur die PWA, STM und ESP unverändert. Release-ZIP
`wordclock-release-2026-10-05-234753.zip`, Tag `release/3.2.21-3.2.24-1.4.91`.
Nachgetragen am 07.10.2026.

### Was sich ändert

- **Ein Sprachwechsel leert keine gefüllten Anzeigen mehr.** Bisher fielen 21
  Anzeigeflächen auf „wird geladen…“ zurück, darunter das Logfenster und die
  Dateivorschau, und bei einigen kam der Inhalt nicht von selbst zurück
  (`BEFUNDE.md`, L284).
- **Textfelder werden vor dem Absenden in Byte geprüft, nicht in Zeichen.** Das Gerät
  zählt Byte, die Eingabefelder zählten Zeichen: Ein Lauftext aus 32 Umlauten passte
  ins Feld und wurde vom Gerät abgewiesen. Jetzt meldet die PWA die Überschreitung
  selbst und schickt nichts ab. Der Import einer Sicherung nutzt dieselben Grenzen und
  übernimmt einen zu langen Wert nicht, statt ihn zu kürzen — ein gekürzter Update-Host
  zeigt auf einen anderen Server (L287).
- **Nach einer Abweisung erscheint keine grüne Meldung mehr.** Beim Speichern von
  Update-Host und -Pfad überdeckte die anschliessende Nachprüfung den Fehler mit
  „abgeschlossen“ (L288). Derselbe Fehler beim Formatieren des Dateisystems ist
  mitbehoben: Nach misslungenem Formatieren steht nicht mehr „formatiert“ da (L303).
- **Umschaltknöpfe zeigen nach einem Sprachwechsel den richtigen Zustand** (L304), ihre
  Texte stehen in beiden Sprachen statt fest auf Deutsch (L305), und nach einem Fehler
  bleibt keiner mehr auf „schaltet…“ stehen (L306).
- **Die Dateivorschau zeigt Binärdateien nicht mehr als Text** und meldet sie nicht
  mehr als Erfolg. Bisher landeten bei `app.js.gz` rund 123'000 Zeichen Rohdaten in der
  Seite. Erkannt wird zuerst an der Endung, damit das Gerät die Datei gar nicht erst
  überträgt, dann am Inhalt. Eine leere `.gz` bleibt sichtbar — das ist der
  Weissbildschirm-Fall (L246).
- **Die Beschriftung sagt, worauf der Knopf wirkt:** „Sekunden am Ambilight-Ring weich
  ausblenden“ (L223).

Intern: eine tote Funktion entfernt (`formatLittleFs()`, L221). Probe
`tools/ui-mess/proben/runde-p.mjs`.

### Am Gerät abgenommen, mit Vorbehalten

Lesender Testdurchlauf am 05./06.10.2026 (`BEFUNDE.md`, L324): 29 Prüfungen, 26
bestanden, keine Exceptions, keine Neustarts, keine Watchdog-Resets. Abgenommen sind
Sprachwechsel, Binärsperre, Beschriftung und die Umschaltknöpfe nach Sprachwechsel.

- Die Byte-Prüfung ist für Lauftext und Update-Host belegt, **am Zeitserver nicht** —
  dort überschrieb ein spät eintreffender WLAN-Scan die Eingabe, bevor die Prüfung sie
  sah (L321, behoben in 1.4.92).
- L288, L303 und L306 brauchen einen fehlschlagenden Schreibaufruf und sind im lesenden
  Lauf nicht geprüft.
- Die Phasen 3 bis 8 des Testplans liefen nicht.

### Was zu flashen ist

**Nur die PWA.**


## 2026-10-05 Eingänge gehärtet, auf beiden Seiten der Brücke (STM 3.2.21, ESP 3.2.24, PWA 1.4.90)

Alle drei Komponenten. Release-ZIP `wordclock-release-2026-10-05-224844.zip`, Tag
`release/3.2.21-3.2.24-1.4.90`.

**Einspielreihenfolge: ESP zuerst, dann STM.** Die PWA kommt mit.

### Ein Absturz aus dem LAN, der als behoben galt (ESP)

Seit ESP 3.2.3 stürzt die Uhr nicht mehr ab, wenn eine Adresse einen Parameter **ohne**
`=` trägt (`GET /?a`). Das war nur die halbe Wahrheit: Die Korrektur hatte den Fehler
von der Zerlegung der Adresse in deren **Auswertung** verschoben. Drei Aufrufe konnten
das WLAN-Modul weiterhin zum Absturz bringen — aus dem ganzen Heimnetz, ohne Anmeldung:
der Flashvorgang der Legacy-Seite, das Setzen eines DFPlayer-Alarms und die
Overlay-Seite der Legacy-Oberfläche. **Das ist jetzt behoben**, an einer Stelle für alle
Aufrufer statt an jeder einzeln (`BEFUNDE.md`, L285).

### Einstellungen werden abgewiesen statt verfälscht (ESP)

- **Zu lange Texte werden abgewiesen, nicht mehr still gekürzt.** Betroffen sind
  Lauftext, Datumsformat, Wetter-AppID, Ort, Längen- und Breitengrad, Zeitserver,
  Update-Host und Update-Pfad. Bisher kam `{"ok":true}`, gespeichert wurde ein
  gekürzter Wert — beim Update-Host hiesse das ein anderer Server. Jetzt bleibt der
  alte Wert stehen, und die Meldung nennt die Grenze.
- **Die Grenze zählt Byte, nicht Zeichen.** Die Meldung sagt das ausdrücklich. Die
  Eingabefelder der PWA begrenzen dagegen auf Zeichen: Ein Lauftext mit vielen Umlauten
  kann deshalb abgewiesen werden, obwohl das Feld ihn zulässt. **Offen**, wird in der
  PWA mit einer Byteprüfung vor dem Absenden gelöst (`BEFUNDE.md`, L287).
- **Beim Speichern des Update-Hosts oder -Pfads kann die Abweisung verdeckt werden:**
  Die PWA zeigt danach sofort „abgeschlossen" an. Prüf nach dem Speichern, ob der
  Wert angekommen ist. **Offen** (`BEFUNDE.md`, L288).
- **Datei löschen meldet keinen Erfolg mehr, wenn nichts gelöscht wurde** — weder bei
  fehlendem Namen noch bei einer Datei, die es nicht gibt, noch bei einem Fehlschlag.
- **DFPlayer-Alarm:** Ein ungültiger Index, ein Wochentag ausserhalb von 0 bis 6 und
  eine Uhrzeit ausserhalb des Tages werden abgewiesen. Bisher entstand daraus ein
  Alarm, der als aktiv in der Liste stand und nie auslösen konnte; ein Wochentag 9
  wurde zum Montag.
- **Bereichsmeldungen einheitlich:** Abweisungen nennen den gültigen Bereich in der
  Form `<param> out of range (<min>..<max>)`. Zwei Meldungen zum Overlay-Index nennen
  bewusst keinen, weil die Bedingung dort „nicht belegt" heisst und kein Zahlenbereich
  ist.

### Kommandos vom WLAN-Modul werden geprüft, bevor sie wirken (STM)

- **Drei Zeittabellen-Handler schrieben mit einem ungeprüften Index** — Nachtzeiten,
  Ambilight-Nachtzeiten und Weckzeiten. Die Arrays haben acht Plätze, der Index
  reichte bis 255: ein Schreibzugriff bis rund 247 Einträge hinter das Array
  (`BEFUNDE.md`, L286).
- **Fünf Display-Setter hatten denselben Fehler** — Verzögerung und Flags der
  Animationen, der Farbanimationen und der Ambilight-Modi, schlimmstenfalls 5'658 Byte
  hinter der Struktur (`BEFUNDE.md`, L289).
- **Eine zu kurze Kommandozeile wird verworfen**, bevor sie zerlegt wird. Vorher konnte
  der Lesezeiger hinter das Zeilenende laufen.
- **`htoi()`** liest nicht mehr über das Zeilenende hinaus — dieselbe Korrektur, die
  der ESP schon hatte.
- **Verworfene Zeilen werden gezählt und gedrosselt protokolliert** (`cmd rejected #…`
  im Logbuch), damit eine Abweisung nicht als unerklärliches Nichtverhalten endet.

Bei gültigen Kommandos ändert sich nichts. Für die fünf Display-Setter ist das über den
ganzen Indexbereich 0 bis 255 nachgewiesen: ohne Prüfung 1'241 Schreibzugriffe ausserhalb,
mit Prüfung keiner, und keine gültige Zeile fällt heraus.

### Release-Notes-Verweise treffbar (PWA)

Die beiden Verweise „Release Notes STM32" und „Release Notes ESP8266" im Modul
Wartung waren 22 px hoch. Am Gerät nachgemessen (05.10.2026, 1512 px Breite): keine
Trefffläche mehr unter 44 px, vorher zwei (`BEFUNDE.md`, L282).

### Beim Einspielen beobachtet

Der ESP-Neustart hat den Variablensatz wie vorgesehen selbst nachgefordert: Ein
Vollabgleich nach **einer** Anforderung. Die stille Nachforderung aus dem letzten
Release hat sich damit zum ersten Mal im Ernstfall gezeigt.

### Was das für künftige STM-Runden heisst

Der F103-Build hat nach diesem Release **1'924 Byte frei**, vorher 2'608 — unter der
Warnschwelle von 2'048 Byte. Deine Uhr läuft auf dem F411 und ist davon nicht
betroffen. Jede weitere STM-Änderung muss den F103 aber mitrechnen
(`BEFUNDE.md`, L296).

### Intern

Neue dauerhafte Prüfungen: Die Indexguards der fünf Display-Setter stehen in Guardrail
S15 unter Prüfung, gegen den wörtlich ausgeschnittenen Quelltext und mit einer
Gegenprobe bei jedem Lauf; die drei Zeittabellen-Handler deckt sie noch nicht ab. Die
Längenprüfung der Kommandozeile läuft in S13. Dazu ein falscher Kommentar im ESP
berichtigt (L274) und `BEFUNDE.md` auf echte Umlaute umgestellt.


## 2026-10-04 Die Uhr merkt, wenn ihre Einstellungen nicht ankommen (STM 3.2.20, ESP 3.2.23, PWA 1.4.89)

### Das Problem

Nach einem Neustart des WLAN-Moduls ist dessen Kopie der Einstellungen leer. Gefüllt
wird sie, weil das Modul seine IP meldet und die Uhr daraufhin **alles** schickt.

**Geht diese eine Meldung verloren, passiert gar nichts** — und der Schaden ist grösser
als eine lückenhafte Kopie: An derselben Meldung hängt die Frage, ob die Uhr das Modul
überhaupt für erreichbar hält. Zehn Sendewege sind damit bewacht; fällt sie aus,
schweigt die Uhr vollständig. Die Verbindung ist dann halbtot, nicht lückenhaft, und
nichts heilt das ausser einem Neustart.

Die Prüfsumme aus dem letzten Release hilft hier nicht und kann es nicht: Sie sichert
die Richtung **von** der Uhr **zum** Modul. Diese Meldung läuft in der Gegenrichtung.

### Was jetzt passiert

Das Modul prüft nach dem Hochfahren, ob die Einstellungen angekommen sind — erkennbar
daran, ob die Hardware-Kennung noch auf ihrem Vorgabewert steht. Ist das so, fordert es
sie **still** erneut an: bis zu dreimal, im Abstand von zehn Sekunden, jeder Versuch mit
einer Zeile im Logbuch.

**Still** heisst wörtlich: Der frühere Entwurf hätte das Modul seine IP-Meldung
wiederholen lassen — und dabei wäre der IP-Lauftext jedes Mal sichtbar über die Uhr
gelaufen. Das war der Grund, diesen Weg nicht zu nehmen.

Im Normalfall kostet die Prüfung **nichts**: eine Abfrage wenige Sekunden nach dem
Start, und fertig. Sie ist zugleich die billigste Dauermessung für die Prüfsumme —
schlägt sie nie an, ist das der laufende Beleg, dass die funktioniert.

### Eine Falle, die seit jeher dort steckte

Beim Bauen zeigte sich, dass der vorgesehene Weg einen alten Fehler wiederholt hätte:
Trifft die IP-Meldung ein, **während** die Uhr auf eine Bestätigung wartet, laufen alle
rund 194 Kommandos auf einmal hinaus, ohne auf eine einzige Antwort zu warten. Die
Bestätigungen quittieren danach der Reihe nach **fremde** Kommandos und füllen den
Empfangspuffer über — genau der Schaden, gegen den das Ganze antritt.

Der neue Weg vermeidet das: Er merkt sich die Anforderung und schickt sie im normalen
Ablauf, nicht aus der Wartezeit heraus. **Der alte Weg hat die Falle weiterhin** — das
ist festgehalten und bleibt ein eigener Schritt, weil die Entscheidung dort an einer
anderen Frage hängt.

### Wetterbeschreibungen werden nicht mehr mitten im Zeichen abgeschnitten

Die Beschreibung von der Wetter-Website wird auf 31 Byte gekürzt — bisher ohne Rücksicht
auf Zeichengrenzen. Fiel der Schnitt auf einen Umlaut, entstand ein halbes Zeichen, und
das war der Auslöser des Pufferüberlaufs aus dem letzten Release. Jetzt wird bis zur
nächsten Zeichengrenze zurückgenommen; der Text ist dann ein bis zwei Zeichen kürzer.

### Kontrast der Ankreuzfelder: nachgemessen, nichts zu ändern

Eine frühere Messung hatte zu niedrige Werte ergeben. Am Bildpunkt nachgemessen — über
sieben Bildschirmbreiten und alle Module — liegen sie zwischen 3,2:1 und 5,1:1, also
über der Anforderung. Die Farbe bleibt; nur die Zahlen im Stylesheet waren falsch und
sind jetzt gemessene.

### Was zu flashen ist

**STM zuerst, dann ESP.** Die PWA enthält nur berichtigte Kommentare, kommt aber mit.


## 2026-10-04 Die Brücke wiederholt und prüft (STM 3.2.19, ESP 3.2.22)

STM und ESP. Die PWA bleibt auf 1.4.88.

**Einspielreihenfolge: STM zuerst, ESP danach.**

### Worum es geht

Zwischen Uhr und WLAN-Modul läuft eine serielle Leitung, über die **jede** Einstellung
geht. Sie hatte zwei Lücken, die beide denselben Schaden anrichten: einen **still
falschen Wert**.

**Die erste: Ein verlorenes Kommando wurde nicht wiederholt.** Die Leitung quittiert
jedes Kommando, und bei ausbleibender Quittung stand bisher sinngemäss „weiter ohne" —
das Kommando war weg, und niemand erfuhr davon. Daran hingen mehrere offene Befunde:
der Variablensatz, der nach einem Modulneustart unvollständig blieb, und damit auch der
Fall, in dem die Uhr sich nicht mehr flashen liess.

**Die zweite: Ein verfälschtes Kommando sah gültig aus.** Die Quittung bestätigte nur,
dass eine Zeile **gelesen** wurde — nicht, dass sie heil ankam. Ein Overlay-Typ, der
als 2 abgeschickt wurde, kam als 14 an. Der Mechanismus ist inzwischen reproduziert:
Fehlen Zeichen am Ende, liest der Empfänger **Reste des vorigen Kommandos** weiter.

### Was jetzt passiert

Bleibt eine Quittung aus, wird das Kommando **vorgemerkt und später erneut gesendet** —
höchstens zweimal, höchstens ein Versuch je Sekunde, und veraltende Messwerte wie
Temperatur oder Uhrzeit kommen gar nicht erst in die Liste. **Die Uhr wartet dabei
keine Millisekunde länger als vorher**: Vorgemerkt wird sofort, gesendet wird nebenher.

Dazu trägt jedes Kommando eine **Prüfsumme**. Erkennt das Modul eine verfälschte Zeile,
wendet es sie **nicht an** und meldet das zurück — die Uhr schickt sie erneut.

**Nachgestellt am belegten Schadensfall** (554'121 Zeilen mit echtem Blockverlust):
Keine einzige verfälschte Zeile hat die Prüfsumme passiert, und keine heile wurde
abgelehnt. Vorher wurden **76 %** der beschädigten Zeilen angenommen — jede mit einem
still falschen Wert.

**Was offen bleibt, benannt:** In 19 % der Schadensfälle geht die Prüfsumme **mit**
verloren. Die Zeile sieht dann unmarkiert aus und wird wie bisher angenommen. Das ist
der Preis dafür, dass unmarkierte Zeilen weiterhin durchgehen müssen — sonst bräche
jeder Betrieb, in dem eine Seite die Prüfsumme noch nicht kennt.

### Zwei Lücken, die beim Bauen auffielen

Die Prüfsumme darf erst fliessen, wenn die Gegenseite sie versteht. Dafür meldet das
Modul beim Start seine Fähigkeit. **Das reichte nicht:** Wird das Modul auf eine
ältere Fassung zurückgerollt, ohne dass die Uhr neu startet, hätte sie weiter
Prüfsummen angehängt — und das Modul hätte sie als **Teil des Werts** gespeichert.
Aus einem Servernamen wäre Servername-plus-Prüfsumme geworden, also genau der stille
falsche Wert, gegen den das Ganze antritt.

Die zweite war stiller: Die Uhr verwirft beim Start absichtlich alles, was in den
ersten Sekundenbruchteilen hereinkommt — und genau dort liegt die Fähigkeitsmeldung.
Dass sie heute ankommt, hing an einer Wartezeit im Modul-Code. Beides ist behoben.

### Zwei Lesefehler über Puffergrenzen

**Ein abgeschnittenes Sonderzeichen liess das Modul über das Ende eines Puffers hinaus
lesen.** Die Quelle ist eine fremde Wetter-Website; eine Antwort, die mitten in einem
Umlaut endet, genügte. Nachgewiesen mit einer gesperrten Speicherseite hinter dem
Puffer — die alte Fassung stürzt dort ab, die neue nicht.

**Und rund 45 Stellen schoben einen Lesezeiger weiter, ohne zu prüfen, ob die Zeile
überhaupt so lang ist.** Jetzt wird die Länge **einmal vorne** geprüft, bevor
zerlegt wird; zu kurze Zeilen werden verworfen statt halb ausgeführt.

### Was zu flashen ist

**STM zuerst, dann ESP.** Die PWA bleibt unverändert.


## 2026-10-04 Zwei Lesefehler über Puffergrenzen, und drei nachgeholte Aufgaben (ESP 3.2.21, PWA 1.4.88)

ESP und PWA. Der STM bleibt auf 3.2.18.

**Einspielreihenfolge: ESP zuerst, PWA danach.** Das ist diesmal keine Formalie —
siehe unten.

### Ein verlorenes Zeichen erzeugte einen falschen Wert, keinen fehlenden

`htoi()` liest Hexzahlen von der Brücke. Die Schleifenbedingung prüfte das **erste**
Zeichen, gelesen wurde das **i-te** — nach dem ersten Durchlauf war die Bedingung
bedeutungslos, und bei einer zu kurzen Zeichenkette las die Funktion über das
Zeilenende hinaus.

Die Folge ist schwerer als ein Absturz: Ging auf der Brücke ein Zeichen verloren,
entstand kein *fehlender* Wert, sondern ein **gültig aussehender falscher**,
aufgefüllt mit dem, was zufällig dahinterstand. Nachgerechnet: `OT000` ergab 0,
`OT002` ergab 32. Und weil er gültig aussah, fiel er nirgends auf. Das ist der
Mechanismus, über den ein Overlay-Typ als 14 ankam, obwohl der STM 2 geschickt hatte.

Bei vollständiger Eingabe ändert sich nichts — der Fix wirkt ausschliesslich auf
Fälle, die heute schon falsch sind. Eine Aufrufstelle war knapp: Sie übergibt einen
Puffer **ohne Abschlusszeichen** und trägt nur, weil dort genau zwei Ziffern erwartet
werden.

### `fs_show` unterscheidet jetzt drei Zustände

Der Endpunkt lieferte bei fehlender Datei einen **leeren Rumpf** — ununterscheidbar
von einer tatsächlich leeren Datei. Am Gerät gemessen fiel sogar ein dritter Zustand
in dieselbe Antwort: ein leerer Parameter.

Jetzt kommt bei fehlender Datei eine Fehlerantwort mit eigenem Code, bei leerer Datei
eine leere Textantwort. Die Oberfläche unterscheidet am Antworttyp, nicht am
Rumpfanfang, und sagt für die leere Datei ausdrücklich, dass das **kein Fehler** ist.

**Hier liegt der Grund für die Einspielreihenfolge:** Auf der alten Firmware würde die
neue Oberfläche bei einer fehlenden Datei melden „ist leer — das ist kein Fehler, die
Datei gibt es". Das wäre eine Falschaussage über den Gerätezustand — die Oberfläche
wäre für dieses Zeitfenster schlechter als ihre Vorgängerin. Die Unterscheidung kann
nur die Firmware liefern.

### Ankreuzfelder

Sie wurden über eine allgemeine Regel auf **13 × 44 px** gezogen — hohe schmale
Kästen, bei denen die Beschriftung unter das Feld rutschte statt daneben zu stehen.
Zurückgesetzt war das nur für einen Teil; die TFT-Felder und die Favoritenzeilen
blieben so. Das war vermutlich auch der Grund, warum die Felder „beigebraun" wirkten
— am Bildpunkt gemessen ist die Farbe neutral, der Eindruck kam aus der Geometrie.

Beim Beheben zeigte sich ein Fallstrick: Die 44 px waren das **Einzige**, was diesen
Zeilen überhaupt ein Antippziel von 44 px gab. Sie nur zurückzusetzen hätte die
Barrierefreiheit an anderer Stelle gebrochen. Die Höhe ist deshalb auf die Beschriftung
gewandert.

### Kontrast des Hakens

Die Akzentfarbe war so hell, dass der Haken nur dann genug Kontrast hat, wenn die
Darstellungsmaschine ihn dunkel zeichnet. Chrome tut das, Safari auf dem iPhone
möglicherweise nicht — und das ist die Hauptplattform. Eine Messung in Chrome hätte
„erfüllt" ergeben und wäre eine Zusage gewesen, die auf dem Telefon bricht.

Deshalb eine eigene Steuerfarbe, die in **beiden** Fällen über der Grenze liegt. Sie
gilt nur für Formularelemente; Text, Ränder und Fokusringe behalten die bisherige
Farbe. Sichtbare Nebenwirkung: Die Schieberegler sind jetzt kräftiger blau.

### Was zu flashen ist

**ESP zuerst, PWA danach.**


## 2026-10-04 Runde 3: Das Kachelraster im Wartungsmodul (PWA 1.4.87)

Nur die PWA. STM und ESP unverändert.

### Was sich ändert

Die sechs Kacheln des Wartungsmoduls standen bisher in drei von Hand gepflegten
Rastervorlagen — je eine für breite, sehr breite und schmale Bildschirme. Sie sind
weg; die Kacheln ordnen sich jetzt von allein. Das Markup folgt der Reihenfolge
`Quelle, Server-Update, Lokales Update, Sicherung, Service, Dateien`, und die
sichtbare Reihenfolge stimmt an **jeder** Breite mit ihr überein.

**Damit ist die Fokusreihenfolge berichtigt.** Bisher wich sie von der sichtbaren ab
und führte als Drittes zu den Service-Aktionen: Wer aus „Vom Server laden"
heraustabbte, stand direkt vor „STM32 zurücksetzen" und „EEPROM zurücksetzen". Die
Kachel liegt jetzt an fünfter Stelle.

### Die Leerfläche: von 900 auf 406 px — und das Kriterium bleibt verfehlt

Das Ziel war, dass nebeneinanderstehende Kacheln sich um höchstens 400 px in der Höhe
unterscheiden. Erreicht sind **406**.

Der grosse Abstand ist weg: Die Quellen-Kachel stand neben einer dreimal kleineren,
das sind jetzt 302 px statt 900. Was bleibt, ist ein anderes Paar — die Sicherungs-
neben der Service-Kachel. Deren Unterschied ist **kein Rasterproblem**: Die eine trägt
Infoliste, Exportknopf, Dateifeld, Importknopf und Hinweiszeile, die andere zwei
Knöpfe. Keine Anordnung dieser sechs Kacheln bringt das unter 400 px.

**Das wird als verfehlt berichtet, nicht als erfüllt.** Der Rückgang von 900 auf 406
ist der Gewinn dieser Runde; „erfüllt" wäre eine Falschaussage.

Eine Zahl, die dabei herauskam und mehr sagt als das Kriterium selbst: Gemessen wurde
an sieben Breiten, und der **schlechteste Wert liegt nicht bei der geprüften Breite**,
sondern bei 600 px — dort sind es 594. Je schmaler die Spalte, desto höher wird die
Sicherungskachel. Eine Abnahme an einer einzelnen Breite misst nicht, was der Nutzer
an seinem Gerät sieht.

### Zwei Fallen, die kein Werkzeug gemeldet hätte

Beim Entfernen der Vorlagen wären zwei Folgefehler entstanden, die in keiner
Aufgabenbeschreibung standen: Der Modulkopf hätte gegen nicht mehr existierende
Rasterlinien aufgelöst, und die Dateikachel — die die Breite wirklich braucht — wäre
ab 900 px zu einer halben Spalte geschrumpft. Beides fällt nirgends auf: Die
Browserprüfung sieht Ladefehler, nicht Geometrie. Gefunden hat sie die Vorprüfung,
die dieser Runde vorausging.

### Was zu flashen ist

**Nur die PWA.**


## 2026-10-04 Die Tickerblockade ist weg (STM 3.2.18, PWA 1.4.86)

STM und PWA, der ESP unverändert. **Das ist das Release, das den Hänger behebt.**

### Was sich an der Uhr ändert

Bis hierher hat jeder Ticker den **Hauptloop angehalten**, bis er durchgelaufen war —
13 bis 20 Sekunden bei 32 Zeichen. In dieser Zeit lief keine Zeitaktualisierung, keine
Temperaturmessung, keine Kommandoannahme vom ESP. Die Weboberfläche zeigte eine
eingefrorene Uhrzeit, und der Watchdog schlug nicht an, weil die Schleife ihn selbst
bediente.

Betroffen waren vier Stellen, und zwei davon feuern **ohne jede Benutzerhandlung**:
das Ticker-Overlay und der **Datumsticker**. Auf dieser Uhr ist ein Datums-Overlay
aktiv — sie stand also periodisch rund sieben Sekunden still, ohne dass jemand etwas
tat. Dazu der IP-Ticker der Startsequenz und der Tickertext aus der Oberfläche.

Alle vier laufen jetzt nach dem Muster, das der **Wetterticker** seit Langem produktiv
fährt: Der Ticker scrollt über den periodischen Zweig, der Display-Restore läuft über
die bestehende Bedingung. Gleiche Schrittweite, gleiche Dauer, gleiches Aussehen — nur
ohne Blockade.

**Die Uhr bleibt während eines Tickers bedienbar**, und mehrfaches schnelles Speichern
eines Textfelds legt sie nicht mehr lahm.

**Eine Nebenwirkung, die sichtbar sein kann:** Fällt ein Minutenwechsel oder eine
Helligkeitsanpassung in einen laufenden Ticker, kann es kurz flackern. Der
Wetterticker verhält sich seit Langem so; neu ist, dass es auch den häufigeren
Datumsticker betrifft.

**Nicht umgestellt** wurden fünf weitere Stellen ausserhalb: IR-Anlernen und die
Schlussmeldung der Spiele halten den Hauptloop **absichtlich** an — dort liefe der
Ticker gar nicht, die Meldung wäre schlicht unsichtbar.

### Wie der Befund gefunden wurde

Die zuerst naheliegende EEPROM-Spur war **falsch**: Für den Tickertext gibt es gar
keinen Ablageort. Die Ursache stand wörtlich im Code — zusammen mit zwei Messungen vom
02. und 03.10. in einem Kommentar über der Schleife, die niemand mit dem seit Monaten
offenen Hänger verbunden hatte.

Auch die Deutung des Messbilds war verkehrt: Dass der LED-Refresh weiterlief, galt als
Hinweis auf eine lebende Interrupt-Routine. Es gibt keine — der Refresh wird **aus der
blockierenden Schleife selbst** gerufen. Weiterlaufender Refresh bei stehendem
Hauptloop war also der stärkste Beleg, nicht das Gegenargument.

### Was in der PWA drin ist

Ein Schalter für den **Weisskanal**. Er fehlte bisher, und anders als bei den anderen
Endpunkten ohne Bedienelement gab es **keinen Rückweg**: Steht der Wert auf 0 — etwa
nach einem Backup-Import —, blendet die Oberfläche alle Weisskanal-Regler aus, auch den
Weg zurück. An dieser Uhr trägt der Weisskanal die gesamte Anzeigefarbe.

Der Schalter liegt bewusst im **Display**-Modul, nicht bei den Farbeinstellungen: Die
liegen im Ambilight-Modul, und das ist hier dauerhaft ausgeblendet, weil der
Ambilight-Ausgang fehlt. Der Rückweg hätte also genau dort gefehlt, wo er gebraucht
wird.

### Guardrails

Stufe S10 prüft jetzt, dass jede Kennung der Arbeitsliste **eindeutig** ist — an einem
Tag waren vier Kennungen doppelt vergeben worden, und ein „erledigt" hätte dann nur
einen von zwei Einträgen getroffen.

Neu als Regel festgehalten: **Eine neu gebaute Prüfung ist erst fertig, wenn sie einmal
fehlgeschlagen ist.** In zwei Tagen sind fünf Melder gebaut worden, deren Meldung nicht
ankam — jedes Mal war der Mechanismus geprüft und der Weg bis zum Empfänger nicht.

### Was zu flashen ist

**STM und PWA.** Der ESP bleibt auf 3.2.20.


## 2026-10-04 Runde 2 und der geklärte Hänger (STM 3.2.17, PWA 1.4.85)

STM und PWA, der ESP unverändert.

### Der Hänger hat eine Ursache

Das Problem, das seit Monaten als „sporadisch, Ursache offen" geführt wurde, ist
**reproduzierbar** und erklärt. Zehn schnelle `ticker_set` mit je 32 Zeichen legen
den Hauptloop für rund 90 Sekunden still — zwei Durchläufe in 38 Sekunden statt
140'000 je Sekunde, die Uhr bleibt stehen, **kein Watchdog-Reset**.

Die zunächst naheliegende EEPROM-Spur ist **widerlegt**: Für den Tickertext gibt es
gar keinen Ablageort. Die Ursache steht wörtlich im Code — `display_set_ticker()`
wird mit `do_wait = 1` gerufen und lässt den kompletten Text **im Hauptloop**
durchscrollen. 204 Iterationen bei 32 Zeichen, rund 93 ms je Iteration. Und in der
Schleife stehen zwei `watchdog_reload()`: **Die Blockade bedient den Watchdog
selbst.**

Der Beleg lag seit zwei Tagen im Quelltext. Ein Kommentar über der Schleife nennt
zwei Messungen vom 02. und 03.10. samt Rechnung — niemand hatte sie mit dem offenen
Befund verbunden. Gefunden hat es eine Analyse, die den Pfad gelesen hat statt
gegrept.

**Dieselbe Blockade steht an drei weiteren Stellen**, zwei davon feuern ohne jede
Benutzerhandlung: Ticker-Overlay und Datumsticker. Die vierte ist der IP-Ticker der
Startsequenz — genau der Fall, der in den Projektnotizen als Hängerstelle steht.
Die Korrektur kommt als eigener Schritt mit eigener Verifikation; dieses Release
bringt sie **nicht**.

### Was im STM drin ist

**Die Diagnosezeile findet nach einem Stillstand zurück.** Bisher kalibrierte sie
sich auf das Mindestbudget und flutete dauerhaft mit rund 69 Zeilen je Sekunde —
der 32-Zeilen-Logring deckte dann noch 0,45 Sekunden ab und enthielt nur noch
Diagnosezeilen. Die Beobachtbarkeit fiel also genau dann aus, wenn man sie brauchte.

Die Ursache lag tiefer als zunächst angenommen: Nicht nur der Loop-, auch der
Tick-Bezugspunkt wurde bei jeder Rückfallzeile verschoben — dadurch kam **gar keine
reguläre Zeile mehr**, und nichts konnte mehr kalibrieren. Behoben mit einem dritten
Bezugspunkt, einem echten Zehn-Sekunden-Kalibrierfenster und einer Dämpfung auf
höchstens Halbierung je Fenster. Gegengeprüft in einer Simulation: Die alte Fassung
erzeugt 70 Zeilen je Sekunde, am Gerät gemessen waren 69,3.

### Was in der PWA drin ist

**Die Selbstaktualisierung hielt nur zu einem Drittel an.** Zwei weitere Poller
liefen bei verborgenem Tab weiter — mit offenem Logbuch wären das 24 Requests in
einer Minute gewesen, und die Abnahme hätte den bereits vorhandenen Code als
wirkungslos erwiesen, ohne dass jemand gesehen hätte, woran es liegt.

**Die Weissschirm-Abwehr fehlte im Service Worker.** `cache.addAll()` prüft nur den
Status, und der ist bei einer 0-Byte-Datei in Ordnung. Jetzt scheitert die
Installation bei leerem Inhalt — der bisherige Service Worker bleibt dann aktiv, was
besser ist als ein Cache mit einer leeren Datei. Wichtiger noch: Es wird auch **beim
Lesen** geprüft, denn ein älterer Cache kann die leere Datei bereits enthalten und
würde sie sonst für immer ausliefern.

**Ein Gerätewert ausserhalb des gültigen Bereichs wird sichtbar, statt still ersetzt
zu werden.** Am Gerät steht `overlay[0].type` auf 14, gültig wäre 0..10. Das
Auswahlfeld zeigte deshalb „Keins" — und wer das Overlay gespeichert hätte, hätte
die Einstellung mit 0 überschrieben. Jetzt steht dort „Unbekannter Gerätewert (14)",
und ein Speicherversuch wird abgewiesen, bevor etwas gesendet wird.

Dazu: unsichere Eingaben an sieben weiteren Speicherpfaden, die Rückfrage beim
Modulwechsel mit offener Bearbeitung, die letzte stille Klemmung der Oberfläche, der
verschluckte Fehler beim Laden der Logs, und die Import-Nachkontrolle, die vier
Felder faktisch nicht verglichen hat.

### Guardrails

Neue Stufe **S8b** überwacht die Flashbelegung des F103 — er stand im Oktober schon
einmal bei 296 Byte Restreserve, ohne dass es jemand wusste. Beide Schwellen sind
gegengeprüft, auch die rote.

### Was zu flashen ist

**STM und PWA.** Der ESP bleibt auf 3.2.20.


## 2026-10-04 Runde 1: Der Parametervertrag (ESP 3.2.20, PWA 1.4.84)

ESP und PWA, der STM unverändert. Ab jetzt gilt: **Ein `{"ok":true}` bedeutet,
dass der gesendete Wert übernommen wurde** — nicht, dass irgendetwas gespeichert
wurde.

### Was vorher galt

An 15 Stellen bog die Firmware einen unzulässigen Wert still zurecht und meldete
Erfolg. Der belegte Anlass: `ambilight_mode_profile_set?deceleration=16` antwortete
`{"ok":true}` und schrieb **15**. Das Geschwister `animation_profile_set` wies
denselben Wert sauber ab. Drei Profil-Setter liegen im Code nebeneinander, teilen
Parameternamen und Wertebereich — und behandelten ihn auf drei Arten.

Die Bestandsaufnahme hat nachgezählt statt geschätzt: **34 saubere Abweisungen
gegen 15 Klemmstellen.** Abweisen war längst die Mehrheit; die Klemmungen waren
die Ausnahme, die niemand aufgeräumt hatte.

### Was sich ändert

Alle 15 Stellen antworten jetzt
`{"ok":false,"error":2,"detail":"<param> out of range (<min>..<max>)"}`.

**Keine Klemmung wurde ersatzlos gestrichen** — das war der eine Risikopunkt.
`set_numvar()` verkürzt still auf 16 Bit, und die Dezelerations-Setter senden mit
`%02x`, einer **Mindest**breite: Ein Wert über 255 erzeugt drei Hexziffern und
verschiebt damit das ganze Kommando auf der Brücke. Davor schützten bisher die
Klemmungen. Jede ist durch eine Abweisung mit **demselben Vergleich** ersetzt, die
Grenze wirkt also unverändert.

Zwei Vertragsentscheidungen: Ein unzulässiger Farbanteil verwirft den **ganzen**
Request statt eine halb übernommene Farbe zu hinterlassen. Und bei `overlay_set`
liegt die Prüfung jetzt vor der ersten Zuweisung — vorher hätte eine Abweisung ein
leeres Overlay in der Liste hinterlassen.

### Der Bruch, der vor dem Rollout gefunden wurde

Der strengere Vertrag hätte **den Backup-Import gebrochen**. `overlay_set` verlangt
neu `interval` 1..255, `days` 1..255 und `duration` 5..9 — und die PWA sendete an
fünf Stellen genau die jetzt verbotenen Nullen, an **beiden Enden** des
Backup-Pfades: beim Erzeugen der Sicherung und beim Zurückspielen. Ein zweiter
Bruchpunkt kam dazu: `animation_profile_set` verlangt `deceleration` ab 1, der
Import sendete 0, sobald das Feld fehlte — das hätte die komplette Animations-Stufe
abgebrochen.

Behoben wurde die PWA-Seite, nicht der Vertrag. Sie sendet jetzt die Vorgabewerte,
die der ESP ohne Parameter ohnehin setzt — das Gerät bekommt denselben Zustand wie
vorher, er steht nur in der Anfrage statt in einer stillen Korrektur.

**Das ist der Punkt, an dem die Spezifikation sich bezahlt gemacht hat.** Die
Verträglichkeitsprüfung war als eigener Schritt vor dem Rollout vorgesehen. Ohne
sie wäre der Bruch beim Nutzer aufgetreten, an dem Pfad, der ein Gerät ohne WLAN,
ohne AP und ohne Webserver zurücklassen kann.

### Die Oberfläche zeigt jetzt, was das Gerät sagt

`describeApiError()` gab bei **bekannter** Fehlerkennung nur den übersetzten Satz
zurück und warf das mitgelieferte Detail weg. Bei „ausserhalb des Bereichs" stand
damit genau das auf dem Bildschirm — ohne zu sagen, welcher Wert und welcher
Bereich. Das entwertete die halbe Arbeit dieser Runde: Der Zweck des Vertrags ist
eine brauchbare Auskunft, und die brauchbare Hälfte wurde verworfen.

### Guardrails

Stufe S5 führte eine **feste Liste** der gz-Quellen, in der keine Sprachdatei
vorkam — sie hätte eine veraltete englische Tabelle durchgelassen, und die neuen
Schlüssel stünden wörtlich auf dem Bildschirm. Der Makefile hatte dieselbe Lehre
schon gezogen und nutzt einen Glob; angewandt war sie nur an einer von zwei Stellen.
S5 nutzt ihn jetzt auch.

### Was zu flashen ist

**ESP und PWA.** Der STM ist unverändert. Nach dem ESP-Update gehört
`./tools/install-app.sh` gefahren — die PWA kommt nicht über den Rollout aufs Gerät.


## 2026-10-04 Runde 0 des grossen Pakets: die Messung erreichbar machen (ESP 3.2.19)

Nur der ESP. Die Runde ändert kein Verhalten der Uhr — sie macht zwei Messungen
sichtbar, die es seit zwei Tagen gibt und die niemand lesen konnte.

### Warum sie am Anfang steht

Das grosse Paket umfasst rund 37 Befunde über vier Stränge. Seine Abnahme hängt an
Messung, und ausgerechnet dort lag ein Fehler, der sich dreimal wiederholt hat:

- Die Logwache schrieb in eine gepufferte Pipe, ihre Ausgabedatei blieb bei 0 Byte
  (L181).
- `esp_heap_log()` und die Verlustzeile `- http write lost N` gingen als
  `Serial.print()` **am Logring vorbei** — messbar 36 Zeilen im seriellen
  Mitschnitt gegen **0** über `/api/stm32_log` (L185).
- Die Messmarken-Prüfung im Stop-Hook schrieb auf stderr und lief dann in
  `return 0` — bei exit 0 liest das niemand (L191).

Dreimal wurde ein Melder gebaut, dreimal kam die Meldung nicht an; zweimal hat es
der Nutzer bemerkt. **Geprüft wurde jedes Mal der Mechanismus, nie der Weg der
Meldung bis zum Empfänger.** Ein Paket dieser Grösse auf dieser Grundlage
abzunehmen hätte wieder „126 bestanden" ergeben, ohne zu wissen, wofür das bürgt.

### Was drin ist

**Beide Zeilen gehen jetzt zusätzlich durch `stm32_log_append()`** und sind über
`/api/stm32_log` lesbar. Die Zeichenkette wird genau einmal gebaut, per `snprintf`
in einen Stackpuffer — kein Arduino-`String`, dessen `realloc` in 16-Byte-Schritten
die Fragmentierung aus L175 speist. Die ausgegebene Bytefolge auf der UART ist
zeichenidentisch mit vorher, die Brückenlast also unverändert.

Die Vorbedingung „kein STM-Logtext beginnt mit `- `" wurde **geprüft statt
angenommen**: Nur `log_printf()` erreicht den Ring; die beiden einzigen Logtexte mit
führendem Minus laufen über Makros, die ausschliesslich auf die UART schreiben.

**Der Kommentar über `esp_heap_log()` behauptete die Lesbarkeit bereits** — „das ist
der ganze Zweck". Das Präfix allein leistet sie nicht. Wer nur den Kommentar las,
musste die Sache für erledigt halten, und genau so ist der Irrtum in L177
entstanden. Eine falsche Zusicherung im Code hält den Nachprüfenden davon ab,
nachzusehen; sie ist schlimmer als gar keine. Berichtigt, mit der Messung daneben.

### Testplan und Prüfmethode

Neuer Abschnitt **5b, „Was eine Setter-Gegenprobe nicht zeigt"**: `settings_xml`
liefert die **ESP-seitige** Kopie. Der ESP setzt sie beim Setter sofort und sendet
erst danach an den STM — geht es auf der Brücke verloren, meldet die Gegenprobe
trotzdem den neuen Wert. Jedes „bestanden" belegt bis dahin nur, dass der ESP
gespeichert hat. Drei Verfahren mit ihrem jeweiligen Preis; die Rahmenmessung über
`d=` ist ab sofort Pflicht je Setter-Phase und steht auch in der Anweisung des
`pwa-tester`.

**Sieben Schritte waren falsch als „folgenlos rücknehmbar" geführt.** Beim
Berichtigen der einen belegten Stelle (S47, die Display-Farbe ist nach einem
Animations-Moduswechsel dauerhaft verloren) fanden sich sechs weitere. Der
schwerste trifft das neue Verfahren selbst: **S7 leert den Logring und damit den
`d=`-Anfangswert**, auf dem die Rahmenmessung beruht — das Verfahren hätte sich im
ersten Durchlauf selbst untergraben. Dasselbe gilt für jeden Neustart im Fenster
(S110, S113, S114). Annotiert sind ausserdem S48 (löscht still das
Favoritenflag, während das Schwester-Szenario S49 den Hinweis trägt), S80, S54 und
die Dateischritte S100 bis S106.

### Zwei Fallen, die beim Umsetzen auffielen

**Gemischte Zeilenenden** (L193): `ESP-uclock.ino` hat 1043 CRLF von 1240 Zeilen.
Ein Patch über das Edit-Werkzeug vereinheitlicht sie stillschweigend — aus neun
inhaltlich geänderten Zeilen wurden 432. Zurückgesetzt und byte-genau nachgepatcht.
Ein Diff dieser Grösse ist nicht mehr prüfbar, und `git blame` zeigt danach für die
ganze Datei den falschen Commit. Die Regel in `CLAUDE.md` nannte bisher nur UTF-8
gegen ISO-8859-1 und nennt jetzt auch die Zeilenenden.

**Der Push gehört zum Release-Abschluss** und stand nirgends — 23 Commits und 11
`release/*`-Tags lagen nur lokal. Der Stop-Hook meldet Ungepushtes jetzt selbst,
auch bei sauberem Arbeitsbaum; genau dort lag die Lücke.

### Was zu flashen ist

**Nur der ESP.** STM und PWA sind unverändert.


## 2026-10-03 Die Uhr beobachtbar machen (STM 3.2.14, ESP 3.2.12)

STM und ESP, die PWA unverändert. Das Paket löst nichts — es schafft die Mittel,
mit denen sich das ungelöste Hauptproblem überhaupt untersuchen lässt.

### Warum es nötig war

Die sporadischen Hänger auf dem F411 sind seit Monaten offen. Der Mitschnitt vom
30.09.2026 hat die Blockade-Spur ausgeschlossen: kein Watchdog-Reset über 18
Minuten, der Hauptloop lief, ausgefallen war nur der zeitgesteuerte Zweig. Die
Frage lautet seither: Was macht den periodischen Zweig unerreichbar, während der
Rest weiterläuft?

Diese Frage war mit den vorhandenen Mitteln nicht zu beantworten — und das lag
nicht am Problem, sondern an den Werkzeugen.

**Der Logring hielt unter LED-Last 1,2 Sekunden**, während die Oberfläche ihn alle
2,5 Sekunden abfragt. Er lief zwischen zwei Abfragen zweimal um. Ein Vorfall unter
Last konnte nicht zufällig verpasst werden — er **musste** verpasst werden.

Am Gerät gemessen: **96 Prozent** aller Ringzeilen stammten aus zwei Logaufrufen
im LED-Refresh. Sie sind entfernt und durch Zähler ersetzt, die die neue
Diagnosezeile mitträgt. Die dritte Zeile im selben Modul bleibt: Sie feuert
höchstens einmal je Sekunde und ist das Widerlegungsinstrument für einen der
offenen Gerätetests.

### Die Diagnosezeile

```
diag 7 l=482913 t=1051050 u=70 r=1402 w=0 rx=37/256 d=0 o=0
```

Sie zerlegt die offene Frage in unterscheidbare Fälle: Hauptloop-Durchläufe,
Zeitgeber-Interrupts und Betriebszeit. Steigen die ersten beiden, während die
dritte steht, lebt die Zeitbasis und der Sekundenzweig nicht. Steht der zweite,
ist die Unterbrechung selbst tot.

**Ihr Auslöser hängt bewusst nicht an der Betriebszeit** — die kommt aus genau der
Unterbrechung, die im beobachteten Hänger ausfiel. Eine Zeile, die im Fehlerfall
aufhört zu erscheinen, misst nichts, sie bestätigt nur ihr eigenes Schweigen. Der
zweite Auslöser zählt deshalb Hauptloop-Durchläufe und kalibriert sich selbst.

Die Folgenummer ist kein Schmuck: Ohne sie liest sich ein überschriebener Ring wie
„es war ruhig". Der Smoketest prüft die Nummern auf Lückenlosigkeit.

### Der blinde Fleck der Kommandobrücke

Der Empfangsring verwarf bei Überlauf **spurlos** — kein Zähler, kein Flag, kein
Log. Jetzt werden Verwürfe und höchster Füllstand geführt. Beide auf null gilt
ausdrücklich als **nicht bestandene** Messung: Ein Zähler, der nie etwas sieht,
ist von einem, der nicht zählt, sonst nicht zu unterscheiden.

Dabei kam heraus, dass der gemessene Verlustpfad der unwahrscheinlichere ist. Der
Hardware-Überlauf des Sendebausteins wurde nirgends abgefragt — und beim Lesen
still gelöscht. Wird die Unterbrechung selbst verzögert, also genau im
untersuchten Zustand, geht das Zeichen schon dort verloren. Auch das wird jetzt
gezählt.

Die naheliegende Abfrage dafür wäre falsch gewesen, und zwar auf die stille Art:
Die Bibliotheksfunktion prüft zusätzlich ein Freigabebit, das in diesem Projekt
nie gesetzt wird. Sie hätte **immer** „kein Überlauf" gemeldet — ein Zähler, der
strukturell nie auslöst und dafür eine beruhigende Null liefert.

### Watchdog im Startpfad

`var_send_all_variables()` sendet rund 194 Kommandos mit je bis zu drei Sekunden
Wartezeit, ohne den Watchdog zu bedienen — im selben Startabschnitt, bei dem der
F411 hängt.

Ein Reload an dieser Stelle war bisher ausdrücklich ausgeschlossen, mit gutem
Grund: Der daraus folgende Reset hat die Uhr am 02.10.2026 nach sieben Sekunden
wieder ins Leben gebracht. Der Einwand bleibt richtig — er trifft nur den
*bedingungslosen* Reload.

Jetzt wird der Watchdog **nur nach eingetroffener Quittung** bedient. Antwortende
Brücke: Der lange Startpfad läuft durch. Tote Brücke: Reset wie bisher, die
Selbstheilung bleibt. Der Reload belohnt Fortschritt, nicht Warten.

### Diagnosebau

`-DWORDCLOCK_DEBUG=ON` schaltet die 172 abgeschalteten Logaufrufe scharf. Das war
vorher **gar nicht möglich** — die naheliegende Übergabe wirkte nicht, weil die
Variable beim Parsen der Zielbeschreibung überschrieben wird. Am Fabrikat
gemessen: byteidentisch.

Vorgabe bleibt `aus`, aus zwei Gründen, die bei der Option stehen: Die Zeilen gehen
blockierend über dieselbe Leitung, die zum STM führt — und ein Diagnosebau
schreibt Geheimnisse im Klartext in einen Ring, der ohne Anmeldung aus dem ganzen
Netz lesbar ist.

### Was dabei nebenbei sichtbar wurde

Lange Logzeilen wurden **zweistufig still gekappt** — der STM kann 255 Zeichen
senden, die Brücke nimmt 123, der Ring hält 120. Betroffen waren gerade die
aussagekräftigen Zeilen. Eine gekappte Zeile trägt jetzt eine Marke; behoben ist
der Verlust damit nicht, nur erkennbar.

## 2026-10-03 Aufräumen nach F1 (STM 3.2.13, ESP 3.2.11, PWA 1.4.81)

Alle drei Komponenten. Angetreten als Hygiene-Paket — vier latente Befunde, die
beim Bauen von F1 nebenbei aufgefallen waren. Herausgekommen ist ein echter
Fehler mit sichtbarer Wirkung.

### Ein Tickertext konnte ein Spiel starten

`udpsrv.cpp` hatte an drei Stellen ein vergessenes `break`. Sichtbar wurde das
erst, nachdem der ESP-Compile-Smoke überhaupt Warnungen melden konnte — vorher
lief er mit abgeschalteten Warnungen.

Nach einem Ticker-Paket überschrieb der Spielezweig das erste Zeichen und sendete
den Tickertext ein **zweites Mal** als Spielekommando. Der STM liest `Ts` als
„Tetris starten", `Ss` als „Snake starten". Ein Tickertext, der mit „Ts" beginnt —
„Tschüss" genügt —, startete damit ein blockierendes Spiel und riss die Anzeige an
sich. Aus dem ganzen LAN auslösbar, ohne Authentifizierung.

Über dieselbe Kette setzte jeder Tetris- oder Snake-Start zusätzlich einen
Abspielbefehl an den DFPlayer: Die beiden Spielstart-Pakete sind **exakt drei
Byte** lang und passierten damit den einzigen Längenwächter, der den Fall hätte
abfangen sollen.

Dass es kein beabsichtigter Durchfall war, liess sich belegen: Das Kommando war
im ersten Fall bereits abgesetzt — der Durchfall fügte nichts hinzu. Alle drei
stammen unverändert aus dem Ursprungscode.

**Zwei Verhaltensänderungen gegenüber der Android-App:** Ticker-Pakete lösen kein
zweites Kommando mehr aus, Spiele-Pakete keinen Abspielbefehl.

### Der Tickertext enthielt Stackmüll

Beim Beheben kam eine zweite Stelle heraus, die niemand gesucht hatte. Der
UDP-Puffer wurde nicht terminiert, und der Ticker-Zweig setzte den Terminator
**unbedingt** auf eine feste Position. Bei einem Paket „pHallo" lag er damit 27
Byte hinter dem Paketende — alles dazwischen ging als Tickertext mit aufs
Display. Der Kommentar daneben las sich wie eine Terminierung, war aber eine
Kürzung auf Verdacht.

Terminiert wird jetzt **einmal, unmittelbar nach dem Empfang und vor der
Fallunterscheidung**, damit die Zusicherung auch für künftige Zweige gilt. Die
alte Zeile kürzt nur noch, wenn zu kürzen ist. Gegengeprüft: Alle neunzehn Fälle
des Dispatchers enden jetzt mit `break`, auch die, bei denen der Compiler mangels
Seiteneffekt nie gewarnt hätte.

### Die übrigen vier

- **`snprintf` statt `sprintf`** in zwei Sendefunktionen des STM. Im Überlauffall
  wird **gar nicht gesendet** statt gekürzt: Eine gekappte Zeile wäre für den ESP
  syntaktisch gültig und würde dort als richtiger Wert übernommen — aus einem
  Stacküberlauf würde eine stille Verfälschung.
- **Nullterminierung** eines Kommandopuffers, der sich eine Union mit dem
  Dateipuffer teilt. Im Harness belegt: vorher las `strlen` vier Byte über das
  Feld hinaus.
- **Drei Compilerwarnungen** in `http.cpp`, darunter eine seit dem Umstieg auf
  `.gz`-Einzelassets tote Funktion.
- **Dreizehn Stellen** in der App, die eine bereits übersetzte Statuszeile mit
  hartcodiertem Deutsch überschrieben. Die richtige Korrektur war ersatzloses
  Entfernen, nicht Übersetzen: Die Aufrufer setzen die Zeile bereits, und zwar
  **spezifischer** („abschliessend", „erneut") — auch im Deutschen ging dabei
  Information verloren.

### Die Warnungswache hätte sich selbst abgeschaltet

Die gestern eingeführte Bestandswache unterschied „behoben" von „Inkrementallauf"
am Vergleich *weniger als erwartet*. Das trägt nur, solange der Erwartungswert
über null liegt — mit diesem Paket sank er für den ESP auf 0, und ein
Inkrementallauf hätte dann grünes „Bestand unverändert" gemeldet.

Sie zählt jetzt über eine Zeitmarke die tatsächlich übersetzten Objektdateien und
hängt ihre Aussage daran statt an der Warnungszahl. Beide Fälle gegengeprüft.

## 2026-10-03 IR-Codes im Backup — F1 (STM 3.2.12, ESP 3.2.10, PWA 1.4.80)

Alle drei Komponenten. Die gelernten IR-Fernbedienungscodes waren bis jetzt die
**einzige** Konfiguration der Uhr ohne jeden Rückweg: Nach einem EEPROM-Reset
musste man alle zwanzig Tasten neu anlernen, und `learn_ir` blockiert dabei
unbegrenzt — weshalb es nicht einmal automatisierbar ist.

### Was der Katalog falsch geplant hatte

Der Eintrag zu F1 stand seit Monaten als „160 Byte an Offset 4, hexkodiert 320
Zeichen — ein Lese- und ein Schreibkommando über die Brücke". Drei der vier
Angaben waren falsch, und die Umsetzung nach dieser Vorlage hätte Schaden
angerichtet:

- **Belegt sind 100 Byte, nicht 160.** Reserviert ist Platz für 32 Tasten, die
  Schleifen laufen aber über 20. Der Rest ist Polster.
- **Ein Kommando je Richtung geht nicht.** Der ESP schneidet nach 127 Zeichen
  **still** ab, der Empfänger auf der anderen Seite nach 123. Zweihundert
  Hexzeichen hätten also nicht einmal eine Fehlermeldung erzeugt, sondern falsche
  Daten — und STM-seitig vorher einen 160-Byte-Stackpuffer überschrieben, weil
  dort mit `sprintf` ohne Längenprüfung formatiert wird.
- **Der Watchdog war entgegen der Erwartung nicht das Problem.** Hundert Byte
  EEPROM sind rund 1,6 s gegen 20 s Budget. Gefährlich ist etwas anderes:
  Währenddessen wird die Kommandobrücke nicht bedient, und ihr Empfangsring
  verwirft bei Überlauf ohne Log, ohne Zähler, ohne Spur.

### Wie es stattdessen gebaut ist

Ein indiziertes Kommando je Taste, dreizehn Zeichen, in beide Richtungen. Der
Export läuft **ein Kommando pro Hauptloop-Durchlauf**, getrieben von einem Zähler,
den der Anstoss nur auf null setzt. Damit liegt zwischen zwei Kommandos garantiert
der reguläre Watchdog-Reload am Loop-Kopf — ohne eine einzige neue Aufrufstelle,
und die Uhr friert während des Abzugs nicht ein.

Die naheliegende Schleife wäre hier ein Fehler gewesen: Jedes Kommando wartet bis
zu 3 s auf die Quittung, zwanzig ohne Antwort sind 60 s gegen 20 s Watchdog.

Geschrieben wird je Taste einzeln, rund 80 ms statt 1,6 s am Stück. Die App
schickt zwanzig einzelne Requests, sequenziell — das ist die faktische
Flusskontrolle des Systems, auch wenn sie nirgends so heisst.

### Der STM prüft die Kommandozeile jetzt selbst

Ohne das hätte eine verstümmelte Zeile einen IR-Code **still gelöscht**. `htoi()`
hat zwar eine Abbruchbedingung auf das aktuelle Zeichen, prüft dabei aber
dauerhaft dasselbe erste Byte — ein eingebettetes Nullbyte beendet die Schleife
also nicht, und Nicht-Hex-Zeichen werden still zu null. Aus `"I05"` wurde damit
`protocol = 0`, und das ist genau die Kennung für „nie angelernt". Ein verlorenes
Zeichen verschiebt sogar die Felder und schreibt auf die falsche Taste.

Jetzt werden Länge, Hex-Form und `protocol` geprüft, bevor etwas ins EEPROM geht.
Der STM bietet kein Löschen an — also führt er auch keines aus.

### Export und Import in der App

Der Abschnitt `settings.ir` hat immer genau zwanzig Einträge. **Zugeordnet wird
über den Tastennamen, nicht über den Index**: In den Firmware-Quellen steht ein
auskommentierter Block für künftige Modi, der die Nummerierung verschieben würde —
eine indexbasierte Zuordnung importierte dann still auf die falschen Tasten.

Bleibt der Abzug unvollständig, fehlt der Abschnitt **ganz**. Ein Export, der
stillschweigend neunzehn von zwanzig Tasten sichert, sieht gültig aus, und die
fehlende merkt man erst beim Restore, wenn das Original längst weg ist.

Vor dem Import entsteht automatisch eine Rückfalldatei mit dem bisherigen Stand.
Gelingt sie nicht, kommt eine zweite Rückfrage. Nach dem Schreiben wird
gegengelesen — denn ein `{"ok":true}` heisst nur „abgeschickt": Wird ein Kommando
unterwegs verstümmelt, weist der STM es ab, und der ESP erfährt davon nichts.

`BACKUP_VERSION` steigt auf 3. Bestehende Sicherungen bleiben lesbar — das ist die
Vorarbeit vom selben Tag, die bis dahin nie auslösen konnte.

### Drei Befunde, die beim Bauen aufgefallen sind

- **Alle 172 `debug_log_*`-Aufrufe sind in der ausgelieferten Firmware
  wegkompiliert.** Das Makro hängt an einem `DEBUG`, das nirgends definiert wird.
  Am Binärobjekt nachgewiesen. Jede Diagnose, die jemand darüber eingebaut hat,
  war wirkungslos — auch in der Hänger-Untersuchung.
- **Der ESP-Compile-Smoke lief mit abgeschalteten Warnungen** und konnte deshalb
  gar keine melden. Sechs Bestandswarnungen waren dadurch unsichtbar. Behoben,
  mit Bestandswache gegen die bekannte Zahl.
- **Die Projektanweisung wies Agenten an, zwei UTF-8-Dateien als ISO-8859-1 zu
  patchen.** Die Wissensdatei sagte seit je das Richtige — die falsche Angabe
  stand in dem Dokument, das immer lädt.

## 2026-10-03 Backup-Import nimmt ältere Sicherungen an (PWA 1.4.79)

Nur die PWA. STM und ESP bleiben bei 3.2.11 und 3.2.9 — geändert wurde allein
`app.js`, also steigt allein deren Version (DIR-004).

### L82 — der Import wies auch ÄLTERE Sicherungen ab

`parseSettingsBackupFile` prüfte mit `!==` auf die aktuelle `BACKUP_VERSION`.
Eine Datei aus einer älteren App wurde damit abgewiesen, und zwar mit der
Meldung „Inkompatible Backup-Version – Datei mit einer neueren App erstellt."
Die Meldung behauptete das Gegenteil dessen, was vorlag.

Folgenlos war das nur, solange die Version nie gestiegen ist. Mit F1
(IR-Codes ins Backup) steigt sie — und in dem Moment wären alle bestehenden
Sicherungen des Nutzers unbrauchbar geworden, ausgerechnet in der Lage, für
die er sie angelegt hat. Deshalb vor F1 erledigt, nicht mit F1.

Jetzt: grösser als die aktuelle Version → ablehnen, gleich oder kleiner →
annehmen, fehlend oder kein gültiger Wert → als ungültiges Format ablehnen.
Beim Import einer älteren Datei sagt die App, dass Einstellungen fehlen
können, und läuft weiter.

Die eigentliche Falle lag woanders als im Vergleichsoperator: `Number(null)`,
`Number("")`, `Number(false)` und `Number([])` ergeben alle `0`. Das alte
`backup.version || 0` machte aus einer fehlenden Version stillschweigend eine
Zahl. `readBackupFileVersion` prüft deshalb den Typ **vor** der Umwandlung.

Dass fehlende Abschnitte den Import nicht stören, ist jetzt belegt statt
vermutet: Alle zehn Primärstufen beginnen mit einer Leerprüfung, und alle vier
Retry-Prädikate liefern bei fehlender Sektion `false` — eine fehlende Sektion
löst also auch keine zusätzlichen STM-Kommandos aus.

### Ohne Versionsbezug — Werkzeug und Dokumentation

- **S10 prüft jetzt beide Richtungen.** Bisher fiel auf, wenn ein offener
  Befund in der ToDo-Liste fehlte; nicht aber, wenn ein ToDo-Eintrag längst
  erledigt war. Sechs Einträge hatten das überlebt. Gemeldet wird nur, wenn
  jede genannte Kennung erledigt ist — ein Eintrag darf einen erledigten
  Befund als Begründung zitieren.
- **`README-CMAKE.md`**: Die drei namentlich genannten Referenz-ZIPs
  existierten nicht mehr — `build/` ist nicht versioniert. Der Abschnitt nennt
  keine Dateinamen daraus mehr; die Invariante zum Farbpfad bleibt.
- **Testplan und Testagent**: Aufgeräumt wird jeder *berührte* Index, nicht
  jeder geplante. Ein Timer-Slot war nach dem dritten Durchlauf aktiv geblieben
  (L81), während der Bericht „alle Testslots geleert" meldete.

## 2026-10-03 Befunde aus Testdurchlauf 3 (STM 3.2.11, ESP 3.2.9, PWA 1.4.78)

Alle drei Komponenten. Der dritte Testdurchlauf hat 137 Pruefungen gefahren,
105 bestanden und 18 Befunde geliefert; die schwersten sind behoben.

### L66 — Overlay-Werte ueber 255 verfaelschten das Protokoll

set_overlay_var() formatierte vier Felder mit %02x. Das ist eine MINDESTbreite,
der STM liest aber mit FESTER Breite. Ein Wert ueber 255 erzeugt drei
Hexziffern, und der STM nimmt nur die ersten beiden.

Am Geraet belegt: date_code=300 -> ESP zeigt "300", der STM meldet "invalid
date_code: 18" -- und 18 ist 0x12, die ersten zwei Ziffern von "12c". PWA,
settings_xml und die Sicherungsdatei zeigten einen Wert, den die Uhr nicht
benutzt.

Was es zum Befund und nicht zur Vermutung machte: Jeder andere Mehrbyte-Sender
in derselben Datei maskiert mit & 0xFF. Der Overlay-Sender war der einzige ohne.

Jetzt Maske UND Pruefung im Endpunkt. Der Umbau war noetig, nicht kosmetisch:
Bisher wuchs bei einem Anhaenge-Aufruf erst die Overlay-Zahl, eine Abweisung
danach haette ein leeres Overlay in der Liste hinterlassen.

### L70 — vier Endpunkte mit Vorgabeindex 0

overlay_display, overlay_delete, timer_set und ambilight_timer_set lasen
atoi(http_get_param("idx")). Ein FEHLENDER Parameter ist 0 -- und 0 ist ein
gueltiger Index. overlay_delete ohne idx haette das erste Overlay geloescht,
timer_set ohne idx den ersten Timer ueberschrieben.

Der Testagent hat diese beiden Varianten bewusst NICHT gesendet und den
Codepfad am gefahrlosen Zwilling belegt. Kein Hook haette ihn aufgehalten --
no-danger.py kennt diese Endpunkte nicht.

### L69 — der LDR-Rohwert wurde nie gemessen, nicht nur nie gesendet

Mein Befund nannte den fehlenden Sendeaufruf. Das war die zweite Haelfte: Ohne
aktive Automatik wurde gar keine ADC-Wandlung gestartet, der Wert stand auf
seinem Initialwert 0. Haette man nur den Sendeaufruf herausgezogen, waere
weiterhin konstant 0 gesendet worden -- also schlechter als vorher, weil der
Fehler dann plausibel ausgesehen haette.

Statt der Schwelle 16 bremst jetzt ein Sperrzaehler auf hoechstens alle 2 s;
die Schwelle sinkt auf 4, weil 16 auf einer 12-Bit-Skala bei einem Arbeitspunkt
von 12 bis 14 nicht konservativ ist, sondern blind. Der Schlimmstfall sinkt
dabei von vier Sendungen je Sekunde auf eine halbe.

### Ein zweiter Rollout-Blocker, wieder vom umsetzenden Agenten gemeldet

Die neue Indexpruefung haette den Backup-Import 29 Fehlschlaege melden lassen,
obwohl alles korrekt laeuft: importOverlaySettings loeschte alle 32 Plaetze,
und die unbelegten antworteten bisher still mit ok:true. Behoben, aber mit
einer besseren Begruendung als meiner: 32 Kommandos sind rund 8,6 s
Hauptloop-Stillstand. Die Fehlermeldung verschwindet als Nebenwirkung.

### Weiter

L67 vier Setter mit blankem atoi · L68 LDR-Obergrenze, und bei Minimum ueber
Maximum eine Warnung in der Antwort statt stummen Ausfalls -- bewusst keine
Abweisung, weil die Messknoepfe andere Endpunkte rufen und eine Sperre den
realen Entstehungsweg gar nicht getroffen haette · L71 Timer-Bereichspruefung
· L72 Konflikthinweis, weiter gefasst als beauftragt: Ueberschneidung statt
Gleichheit der Tagesmengen · L73 Overlay-Typ auf 0..10 · L74 Koordinaten
passen jetzt in acht Zeichen, mit Normalisierung fuer Leaflets zweite
Weltkopie · L65 der Import meldet, was er zurechtgebogen hat.

### Neu gefunden und offen

L75 stm32_log ist waehrend LED-Aktivitaet nach 1,2 s voll -- 500-mal die
Ruherate, das verstaerkt Massnahme 13 erheblich. L77 der gesendete LDR-Rohwert
ist geklammert, der kalibrierte nicht. L78 dfplayer_alarm_set hat L70 und L71
vollstaendig. L79 das Legacy-Overlay-Formular ebenso.

## 2026-10-03 Gruppe B und die Knoepfe (PWA 1.4.77)

Nur PWA geaendert, STM 3.2.10 und ESP 3.2.8 bleiben.

### Massnahme 17 geschlossen — die Oberflaeche biegt nichts mehr still zurecht

Neun Masken, zwoelf Felder. clampNumber ist ersatzlos entfallen. Der schlimmste
Fall war saveDateTime: Dort hatte clampNumber RUECKFALLWERTE -- ein leeres
Jahresfeld schrieb still 2026, Stunde 25 wurde 23, und die Uhr meldete
"gespeichert". Das war keine Unschoenheit, sondern eine falsch gestellte Uhr
mit Erfolgsmeldung.

Die Klammerung im ESP bleibt: Sie ist das Netz fuer Legacy und direkte
API-Aufrufe. Es ging darum, dass die Oberflaeche nicht mehr luegt.

Gegengeprueft statt angenommen: Schieberegler koennen nach dem
Value-Sanitization-Algorithmus von type="range" gar keinen Wert ausserhalb
liefern, auch nicht bei programmatischer Zuweisung -- dort war nichts zu tun.

### L43 — 46 von 99 Knoepfen waren hervorgehoben, jetzt 14

Eine Hervorhebung, die fuer fast die Haelfte gilt, fuehrt nicht mehr. Der
Nutzer hat es bemerkt, keine Pruefung.

Die beiden Firmware-Updates sind jetzt BEIDE plain: Sie sind gleichen Gewichts
und schliessen einander nicht aus, jede Wahl waere eine Behauptung gewesen.
Damit widersprechen sich Fern- und Lokalkarte nicht mehr, was der eigentliche
Befund war. Nebenbei zurueckgenommen: Die einzige Hervorhebung der Startansicht
zeigte aus der PWA heraus auf die Legacy-Seite.

### L59/L60 — "primary" hiess an zwei Stellen Verschiedenes

setActionToggleButton setzte dieselbe Klasse fuer "eingeschaltet". In der
RGBW-Karte konnten vier Schalter gleichzeitig leuchten wie eine Hauptaktion.
Jetzt is-on plus aria-pressed -- der Zustand stand bisher nur in dataset.state
und in der Farbe und war fuer Screenreader unsichtbar.

Die Gestaltung haengt bewusst an BEIDEN Selektoren, Klasse und aria-pressed:
Nur am Attribut waere classList.toggle("is-on") eine Klasse ohne Wirkung --
genau der stille Zustand, der den Auftrag ausgeloest hat, nur gespiegelt.

Gemessen: Rand is-on 5,02:1 gegen das Panel, zwischen Grundknopf (3,69:1) und
primary (6,37:1). Der Zustandspunkt kommt auf 6,26:1 und traegt die
Unterscheidung farbunabhaengig -- da oder nicht da statt Farbton.

### Massnahme 18 — 85 statt 38 Fundstellen

Zwoelf Literale waren Doppelungen bereits vorhandener Schluessel, die nur nie
benutzt wurden. Vier Stellen bleiben bewusst: Datums- und Startzeitformate sind
bereits zweisprachig und zeigen in englischer Oberflaeche kein Deutsch -- kein
Defekt, nur an der Tabelle vorbei.

### L62 — der Flash-Pfad lief an der Statuspruefung vorbei

autoResetStm32AfterFlash benutzte rohes fetch. Gewonnen ist vor allem der
Status: http_request_is_embedded_subresource() antwortet mit 403, und das galt
vorher als Erfolg -- die PWA meldete "zurueckgesetzt", obwohl nichts geschah.

### Werkzeug

Neue Guardrail-Stufe: Klassen, die app.js setzt, fuer die es aber keine
CSS-Regel gibt. Das ist die Gegenrichtung zu unused-css.mjs, und sie fehlte --
L59 ist genau so durchgerutscht: Die Klasse war benutzt, nur wirkungslos.
Gegenprobe gefahren, der erste Entwurf hatte einen Fehlalarm auf
zusammengesetzte Namen ("is-" + tone), jetzt ausgenommen.

quick-reference: styles.css in die Liste der Dateien mit gemischten
Zeilenenden aufgenommen. Das Edit-Werkzeug hatte sie auf LF vereinheitlicht --
0 statt 56 CR, Diff 89/56 statt der gemeinten 33 Zeilen.

## 2026-10-03 Gruppe C (ESP 3.2.8, PWA 1.4.76)

Dreizehn Befunde geschlossen. STM unveraendert bei 3.2.10.

### Der Rollout-Blocker kam vom umsetzenden Agenten, nicht aus dem Test

Mit L48 weist der ESP leere Textfelder ab, und seit L39 wirft apiFetch bei
ok:false. Der Backup-Import sendete aber bedingungslos "" -- ein Geraet ohne
Wetter-AppID oder ohne Update-Host, beides ueblich, haette damit den Rest
seiner Import-Stufe verloren. Der Import einer gueltigen Sicherung waere
fehlgeschlagen: ausgerechnet die Funktion, die im Notfall die Uhr rettet.

Gemeldet wurde das, BEVOR gebaut wurde. Behoben in L57: leere Werte werden
uebersprungen statt gesendet, die Stufe gilt dabei nicht als gescheitert, und
der Nutzer sieht am Ende, was uebersprungen wurde. Simulation ueber acht
Faelle: vorher 6 Abbrueche, nachher 8 von 8 gruen.

Nebenbefund derselben Form ausserhalb des Imports: applyWeatherMapSelection
sendete den Ort VOR den Koordinaten. Ein Kartenpunkt ohne Namen traf damit als
leerer Ort auf ein Geraet ohne Koordinaten, und die Koordinaten wurden nie
gesetzt. Reihenfolge getauscht.

### L46 — der Umlaut an der Byte-Grenze

An beiden Enden. utf8_truncated_len() in vars.cpp, verwendet in set_strvar()
und in beiden Overlay-Textkopien, die denselben Fehler hatten. Der STM bekommt
jetzt den GEKUERZTEN Wert -- vorher schnitt er ungekuerzte Daten ein zweites
Mal nach, also dasselbe halbe Zeichen erneut. Dazu sanitize_xml_string() als
Netz fuer ungueltige UTF-8-Sequenzen und in XML verbotene Steuerzeichen.

Die erste Fassung der Kuerzung war falsch: Die Rueckwaertspruefung & 0x80
verwarf auch ein gueltiges Folgebyte und reproduzierte den Fehler bei exakt
32 Byte. Richtig ist & 0xC0 == 0xC0. Gefunden durch neun durchgerechnete
Faelle, nicht durch Zufall.

### Zwei Entscheidungen, die ueber die Auftraege hinausgingen

ticker_set bleibt leerbar -- der leere Ticker ist der Auslieferungszustand und
der einzige Weg, ihn abzuschalten. Ort und Koordinaten sind Alternativen und
duerfen leer werden, solange die andere Angabe bleibt; "beides leer" wird
abgelehnt.

Bei L51 ist das Favoriten-Flag ganz entfallen: Ein "auf Vorgabe zuruecksetzen"
darf keinen Zustand erzeugen, der nie die Vorgabe war.

### Weiter

L37 fehlende Farbanteile bleiben stehen statt genullt zu werden · L40, L50,
L51 fehlender idx ist Pflicht statt stiller 0 · L41 network_scan liefert rssi
· L47 Sommerzeit-Setter · L49 zwei Animationssetter · L52 echte Umlaute, jetzt
geprueft ueber tools/checks/umlaute.mjs in S8 · L54 Wetterabruf meldet nicht
mehr Erfolg, wenn die Voraussetzungen fehlen (neue Kennung 5) · L55 der
ESP-Versionsstring wird gefuellt -- zustaendig waere der STM gewesen, dessen
var_send_esp8266_version() ein leerer Rumpf mit "nothing to do" ist.

### L58 — eine Werkzeugfalle fuer die Wissensbasis

Das Edit-Werkzeug schreibt Dateien mit dem DOMINANTEN Zeilenende zurueck.
vars.cpp bekam dadurch 121 stille LF->CRLF-Aenderungen, http.cpp verlor sein
einziges CR. quick-reference.md beschrieb das bisher nur fuer Python-Patches.
Gegenpruefung jetzt dokumentiert: Die Differenz der CR-Zahl muss der Zahl der
neu hinzugefuegten Zeilen entsprechen. Nachgerechnet: +68/+68, +6/+6, 0 Zeilen
die sich nur im Zeilenende unterscheiden.

## 2026-10-03 Zwoelf Befunde aus dem zweiten Testdurchlauf (L45 bis L56)

Reine Dokumentations- und Werkzeugaenderung ausser dem Ticker-Fix, der schon
in 3.2.10 steckt. Kein Versionsbump.

Der Durchlauf gegen STM 3.2.9 / ESP 3.2.7 / PWA 1.4.75 ist nach 87 Pruefschritten
am Watchdog-Reset abgebrochen -- und genau der war der wichtigste Fund (L45,
behoben mit 3.2.10). Vier von elf Modulen sind durch; Klima, Datum/Zeit,
DFPlayer, Overlays, Timer und der ganze Wartungsbereich stehen aus.

Keine einzige Einstellung blieb verstellt: 128 Schreibzugriffe, alle
zurueckgenommen und gegengeprueft.

### Der Abschlussvergleich hatte ein Loch genau dort, wo der Durchlauf arbeitet

tools/diff-snapshot.sh verglich nur numvar und strvar -- 62 Felder. Farben,
Dimmkurven, Overlays, Timer, Alarme und Profil-Flags blieben unbesehen, und
genau die verstellt Phase 3. Der Durchlauf hat neun solcher Felder angefasst;
waere eines stehengeblieben, haette der Vergleich "Kein Unterschied" gemeldet.
Das ist der Schritt, der aus einem Durchlauf einen Nachweis macht.

Jetzt wird jedes Element mit allen Attributen erfasst, ohne Liste bekannter
Typen -- rund 710 Felder. Gegenprobe: identische Abzuege melden nichts, eine von
Hand verstellte Farbe erscheint als dspcolor[idx=0].white A='63' B='99'. (L53)

### Der schwerste offene Befund: ein Umlaut an der Byte-Grenze

Die Textfelder werden auf BYTES gekuerzt, die Oberflaeche zaehlt ZEICHEN. Ein
Tickertext aus 31 A und einem ae endet mit einem halben Zeichen, und
settings_xml ist kein gueltiges UTF-8 mehr. Der DOMParser scheitert; dank L26
meldet die PWA das immerhin -- korrigieren laesst sich der Wert ueber sie aber
nicht, weil sie die Einstellungen nicht mehr lesen kann. Ein deutscher
Grusstext von 32 Zeichen, der auf einem Umlaut endet, reicht. (L46)

### Weiter

L47 network_summertime_set ohne Wert schaltet aus · L48 acht Stringsetter
loeschen bei leerer Eingabe, darunter update_host und update_path -- dieselbe
Luecke wie L42, nur von vorne · L49 zwei Animationssetter beim L29-Fix
uebersehen · L50 dim_level_set ohne idx schreibt auf Index 0 · L51 "Vorgaben
zuruecksetzen" setzt den Favoriten-Marker · L52 die vier neuen Fehlertexte
benutzen ASCII-Umschrift statt Umlauten · L54 weather_get_* melden immer
Erfolg und sind nicht rein lesend · L55 der ESP-Versionsstring im
Variablensatz ist immer leer · L56 zweite unbegrenzte Warteschleife in
display_set_display_mode(), bewusst ohne Reload -- dort waere ein Timeout der
richtige Hebel.

L25 ist neu bewertet: Der Tickerausloeser passt mit 20,85 s bei 20 s Timeout
besser als die urspruengliche These der kombinierten Last. Lueckenlos ist auch
das nicht -- eine show_time-Zeile zwei Sekunden vor dem Reset passt nicht zu
einer durchgehenden Blockade. Als Kandidat vermerkt, nicht als Beweis.

## 2026-10-03 Tickerschleife bedient den Watchdog (STM 3.2.10)

Nur STM-Code geaendert, also steigt nach DIR-004 nur dessen Version.

Einen Tickertext zu speichern konnte die Uhr neu starten. Die Warteschleife in
display_set_ticker() laesst den Text durchlaufen und ruft dabei kein
watchdog_reload(). Seit 3.2.8 laeuft der IWDG wirklich -- damit ist aus einer
jahrelang harmlosen Schleife ein Reset-Ausloeser geworden.

Zweimal am Geraet belegt. 03.10. 01:36:23 Tickerkommando mit 32 Zeichen, danach
14,5 s ohne eine einzige Hauptloop-Zeile, um 01:37:03 IWDGRST. Und rueckwirkend
02.10.: Tickerkommando um 03:44:46, Reset um 03:45:07 -- 20,85 s bei 20 s
Timeout. Damit ist L25 ein starker Kandidat fuer dieselbe Ursache; lueckenlos
ist es nicht, eine show_time-Zeile zwei Sekunden vor dem Reset passt nicht zu
einer durchgehenden Blockade.

Rechnung: Bei ticker_deceleration = 4 sind es 62 ms je Spaltenschritt, also rund
14,5 s fuer 32 Zeichen -- 73 % der Grenze. Das Feld laesst 0..255 zu; bei 255
waeren es knapp 14 Minuten.

Betroffen war nicht nur die Nutzereingabe: do_wait = 1 gilt auch fuer
Wetterticker, Overlay-Ticker und Datumsticker. Auf diesem Geraet laeuft ein
Datums-Overlay stuendlich.

Die Wartezeit ist jetzt in 100-ms-Stuecke zerlegt, mit einem Reload je Stueck --
ein Reload nur je Spaltenschritt haette bei ticker_deceleration = 255 rund 4 s
Abstand bedeutet, knapp unter der Grenze und ohne Reserve. Die Schleife bekommt
bewusst KEINEN Abbruch, sie endet von selbst.

var_send_buf() bleibt unberuehrt. Ebenso die zweite Schleife, die dabei
aufgefallen ist: do { schedule_esp8266_messages(); } while (! tables.complete)
in display_set_display_mode(). Das ist dasselbe Muster wie var_send_buf --
Warten auf eine ESP-Quittung --, und dort ist der Reset gewollt. Dort gehoert,
wenn ueberhaupt, ein Timeout hin.

Guardrail S7 bewacht jetzt sechs Aufrufstellen statt vier. Die Zahl war nach
dem Patch still veraltet: Der Schutz haette erst gegriffen, wenn drei Stellen
verlorengegangen waeren. Gegenprobe gefahren.

## 2026-10-03 Oktober-Paket (STM 3.2.9, ESP 3.2.7, PWA 1.4.75)

Alle drei Komponenten. 17 Befunde geschlossen, drei neue gefunden.

### Der wichtigste Punkt kam aus einer Rueckfrage

Der ESP hatte gelernt, ungueltige Eingaben abzuweisen -- mit HTTP 200 und
{"ok":false,...}, der Hausform. apiFetch() prueft aber nur response.ok, also
den HTTP-Status, und liest den Rumpf nie. runButtonRequest() wertete "keine
Ausnahme" als Erfolg.

Damit haette das halbe Paket das stille Scheitern durch eine FALSCHAUSSAGE
ersetzt: Das Geraet weist ab, die Oberflaeche meldet "gespeichert". Das waere
schlimmer gewesen als der Zustand vorher.

Gefunden, weil der Nutzer gefragt hat, ob im UI auch wirklich ein Fehler
erscheint. Behoben zentral in apiFetch() -- ueber response.clone(), weil der
Rumpf sonst fuer alle weiteren Leser verbraucht ist, und nur bei
JSON-Content-Type, weil display_power reinen Text liefert. Die Fehlerkennung
wird uebersetzt statt durchgereicht: Die Geraetetexte sind englisch, weil die
Legacy-Oberflaeche es ist. Als L39 festgehalten.

### ESP

L26 sanitize_xml_string maskiert jetzt auch " und ' -- ein Anfuehrungszeichen
zerlegte settings_xml und machte die PWA fuer alle nachfolgenden Werte blind.
L28 und L32 auf BEIDEN Wegen, API und Legacy: Der Legacy-Pfad war im ersten
Anlauf uebersehen worden und waere offen geblieben; aufgefallen ist es, weil
der umsetzende Agent den Befund nicht als erledigt gemeldet hat. L29 ueber eine
neue Hilfsfunktion, die "fehlt oder leer" von "ist 0" unterscheidet und
zusaetzlich nicht numerische Eingaben erkennt, die atoi stumm zu 0 gemacht
haette -- elf Setter, darunter drei zusaetzlich gefundene. L30 und L36.

### PWA

L26 parsererror-Pruefung, L29, L31 Markerindex, L33 und Massnahme 4, L34, L24
freies SSID-Feld fuer versteckte Netze, dazu R2-5, R2-10, R2-11 und
Massnahme 7. Leere Felder werden jetzt ABGEWIESEN statt mit einem erfundenen
Rueckfallwert gefuellt -- ein erfundener Wert waere nur eine andere stille
Verfaelschung gewesen.

### STM

L27: Das uebertragene Byte war bereits ein korrektes Zweierkomplement und wurde
nur vorzeichenlos gelesen. Zusaetzlich lief die Zwischenrechnung in
rtc_get_temperature_index() bei 255 auf einem uint_fast8_t ueber -- das erklaert
den Wert 2147483549.5 aus dem Mitschnitt. Drahtformat unveraendert.

Massnahme 1, neu zugeschnitten: watchdog_reload() in remote_ir_learn(), plus
ein Abbruch nach 30 s je Taste. var_send_buf() bleibt BEWUSST unberuehrt -- dort
gibt es seit 3.2.8 einen 3-s-Abbruch, und der daraus folgende Watchdog-Reset hat
die Uhr am 02.10. nach sieben Sekunden wieder ins Leben gebracht (L25). Ein
Reload an dieser Stelle wuerde daraus wieder ein stilles Steckenbleiben machen.

### Werkzeuge

L2: Der Zeitstempel im Release-ZIP ist jetzt sekundengenau. Dabei fiel auf, dass
RELEASE_ZIP mit "?=" eine rekursiv expandierte Variable war -- $(shell date) lief
bei jeder der vier Verwendungen neu. Mit Minuten fiel das kaum auf, mit Sekunden
haette das Archiv regelmaessig anders geheissen als das, was geloescht und
gemeldet wird. Jetzt RELEASE_STAMP := , genau einmal ausgewertet.

Drei Fehlalarme in den eigenen Guardrails beseitigt. S7 meldete seit Wochen
"watchdog_reload() hat nur 1 Aufrufstelle" -- gezaehlt wurde nur in src/main.c,
die zwei Aufrufe in display.c sah die Stufe nie. Beim Nachpruefen bin ich erst
selbst in die dokumentierte Falle getappt: display.c ist ISO-8859-1, und grep
ohne -a stuft sie als BINAER ein und gibt gar nichts aus. Nicht "0 Treffer",
sondern "nicht gelesen". Die Stufe zaehlt jetzt ueber den ganzen Baum und hat
die Aussage umgedreht -- von "diese Massnahme ist offen" zu "sind alle vier
Aufrufstellen noch da". S9 meldete 13 Fehlalarme je Lauf, alle aus
Statuszellen wie "**erledigt** PWA 1.4.72": Angaben ueber die Vergangenheit
veralten nicht.

### Drei neue Befunde

L39 (oben), L40 vier weitere Setter mit stiller Erfolgsmeldung, L41
network_scan liefert keine Feldstaerke.

## 2026-10-02 Erster vollstaendiger PWA-Testdurchlauf — zehn neue Befunde

Reine Dokumentations- und Werkzeugaenderung, kein Produktcode, deshalb kein
Versionsbump. Die gefundenen Fehler sind NICHT behoben, nur belegt.

Gefahren wurden die Phasen 0 bis 4 und 9 aus TESTPLAN-PWA.md: 32 lesende
Pruefungen, 109 schreibende der Klasse S, rund 115 Grenzfallsonden ueber 20
Felder. Backup-Import, Verbindungstrennung und die gefaehrlichen Funktionen
blieben wie vorgesehen aussen vor.

### Der Durchlauf endete mit einem Watchdog-Reset (L25)

03:45:07, Reset flags IWDGRST PINRST. Das ist der ERSTE echte Watchdog-Reset,
seit es mit 3.2.8 ueberhaupt einen funktionierenden Watchdog gibt (L15).
Ausloeser war doppelte Last: der Testtreiber und parallel die im Safari offene
PWA mit neun Endpunkten je 15 s.

In den letzten vier Sekunden davor steht im Mitschnitt alles, was den Haenger
von L14 ausgemacht hat -- var_send_buf-Timeout, eine zerrissene Logzeile als
Beleg verschachtelter Ausfuehrung, show_time vier Sekunden zu spaet.

Was dabei NICHT belegt ist: die Kette bis zum Ablauf der 20 Sekunden. Ich hatte
zwischenzeitlich eine Logstille von 20,5 s als Blockade gelesen -- das war
falsch. Die Firmware protokolliert nur bei Ereignissen, und Stillen von 44 s
zwischen show_time und read rtc sind im Normalbetrieb regelmaessig und harmlos.
Aus einer Logluecke laesst sich hier keine Blockade ableiten.

Der Code zeigt den Weg trotzdem: Die Warteschleife in var_send_buf() ruft
schedule_esp8266_messages(), aber NICHT watchdog_reload(). Damit ist dies der
erste direkte Beleg fuer Massnahme 1.

Und die gute Haelfte: Vor 3.2.8 waere dasselbe Ereignis ein minutenlanges
Einfrieren gewesen. Jetzt sind es sieben Sekunden bis zum selbsttaetigen
Wiederanlauf. Der Abschlussvergleich ueber alle Felder zeigte ausser Uptime und
Reset-Ursache KEINE Abweichung -- alle 109 Schreibpruefungen zurueckgenommen,
und die Werte haben einen EEPROM-Neuladevorgang ueberlebt.

### Der schwerste Befund: ein Anfuehrungszeichen (L26)

sanitize_xml_string maskiert &, < und > -- aber nicht ", obwohl jede
Zeichenkette in einem "-begrenzten XML-Attribut landet. parseSettings prueft das
Ergebnis von DOMParser nicht auf parsererror. Wer ein " in Ort, Tickertext,
AppID oder Update-Host eintraegt, verliert in der PWA alle danach folgenden
Werte, ohne jede Fehlermeldung -- und kann es ueber die PWA nicht rueckgaengig
machen, weil sie die Einstellung nicht mehr lesen kann.

Beide Haelften am Quelltext nachgeprueft. Beide muessen behoben werden.

### Der Testplan hat sich selbst korrigiert (L30)

network_ap_set war im Plan als "betrifft nur den AP-Modus, daher pruefbar"
eingestuft. Der Agent hat die beauftragte Pruefung von sich aus VERWEIGERT und
begruendet: Der Endpunkt setzt EEPROM_FLAG_BOOT_AS_AP, schreibt das EEPROM und
ruft sofort wifi_ap() -- die Uhr waere aus dem WLAN gewesen, auch nach dem
naechsten Start. Am Quelltext bestaetigt.

Dass er das musste, war die Luecke: Der Hook deckte nur eeprom_settings_set mit
boot_as_ap=1 ab. network_ap_set ist jetzt gesperrt, der Plan korrigiert.

Zweitbefund an derselben Stelle: Bei strlen(key) < 10 passiert gar nichts, der
Endpunkt meldet trotzdem {"ok":true}.

### Weitere Befunde

L27 negative Temperaturkorrektur liefert Unsinn (Vorzeichenbruch ESP/STM, im
Mitschnitt belegt mit "RTC temperature: 2147483549.5") · L28 Zeitzone ohne jede
Bereichspruefung im ESP · L29 leeres Zahlenfeld heisst still "0" · L31
ambilight_markers_set schreibt idx 1, die PWA liest idx 0 · L32 der 31. Februar
wird uebernommen · L33 hasUnsavedEdits legt zusaetzlich die Selbstaktualisierung
still · L34 drei kleinere Anzeigebefunde.

Bestaetigt, wie vorhergesagt: Massnahme 17 an ALLEN geprueften Zahlenfeldern,
Massnahme 4, Massnahme 7, die Zeitserver-Kuerzung (L23) und das versteckte WLAN
(L24).

Sauber bestanden und damit ebenfalls ein Ergebnis: Temperaturformatierung bei
positiver Korrektur, Geraetezeit auf 0 s genau, alle zwoelf .gz byte-genau,
Overlay-Datumskodierung, Timer-Dekodierung, Farbumrechnung, Umlaute und Emoji.

## 2026-10-02 Teststrategie, Sicherungskonzept und Testagent

Reine Werkzeug- und Dokumentationsaenderung, kein Produktcode, deshalb kein
Versionsbump.

### TESTPLAN-PWA.md

Vollstaendige Teststrategie fuer die PWA: 11 Module, 87 Schaltflaechen, 55
Eingabefelder, 103 Endpunkte -- aus dem Quelltext erhoben, nicht geschaetzt.
Zehn Phasen von der Sicherung bis zum feldweisen Abschlussvergleich, acht
Grenzfallklassen je Eingabefeld, und eine Liste der Befunde, die der
Durchlauf VORAUSSICHTLICH findet. Ein Testplan, der nur Bekanntes bestaetigt,
ist ueberfluessig; einer, der so tut, als sei alles offen, ist unehrlich.

Zwei Module haben null statische Bedienelemente -- overlays und timers bauen
ihre Oberflaeche zur Laufzeit. Eine Pruefung, die nur das Markup ablaeuft,
uebersieht sie vollstaendig.

### Backup- und Restore-Konzept (Kapitel 2b)

Drei Medien mit drei verschiedenen Zwecken, und der Unterschied ist wesentlich:

  M1  PWA-Sicherung       Klartext-Zugangsdaten, einziger automatischer Rueckweg
  M2  --restore-Abzug     Klartext, aber AES-256-verschluesselt, Handarbeit
  M3  Vergleichsabzug     Schluessel nur als Pruefsumme, kann NICHTS zurueckholen

Der erste Entwurf hatte nur M3. Ein gehashter Schluessel laesst sich nicht
wiederherstellen -- wer nur Vergleichsabzuege hat, hat kein Backup, sondern
einen Messpunkt. M2 schliesst die Luecke.

Festgehalten ist auch, was in KEINEM der drei Medien steht: die angelernten
IR-Codes. Fuer alles andere gibt es einen Weg zurueck, fuer sie nicht. Das ist
der Grund, warum maintenance_reset_eeprom nicht beilaeufig ausgeloest wird.

### Werkzeuge

- tools/snapshot-device.sh -- Rohabzug, ausschliesslich lesend. Hasht die
  Schluessel standardmaessig; mit --restore verschluesselt es stattdessen das
  Ganze und loescht das Klartextverzeichnis sofort danach.
- tools/diff-snapshot.sh -- vergleicht FELDWEISE. Ein diff ueber settings_xml
  meldet sonst eine einzige lange Zeile und sagt nicht, welche Variable sich
  geaendert hat. stm32_log ist ausgenommen: ein Ringpuffer im Sekundentakt.
  Gemessen: ueber 25 s Stillstand aendert sich sonst kein einziges Feld.

### Agent pwa-tester

Faehrt die Phasen 0 bis 4 und 9. Backup-Import, Verbindungstrennung und die
gefaehrlichen Funktionen bleiben ausdruecklich beim Nutzer -- ein Agent, der
unbeaufsichtigt Sicherungen importiert, ist genau das, was die Uhr lahmlegt.

tools/hooks/no-danger.py weist elf Endpunkte ab, bevor der Agent sie erreicht:
die Zugangsdaten der aktiven Verbindung, alles mit Datenverlust, die garantierten
Blockaden und die Firmware-Updates. Dazu boot_as_ap=1 und der Request mit einem
Parameter ohne '='. In beide Richtungen geprueft, 13 Faelle.

### Drei neue Befunde beim Erheben

- L22: /api/eeprom_settings gibt den WLAN-Schluessel IM KLARTEXT heraus, an
  jeden im LAN, ohne Authentisierung. Aufgefallen beim Bau des Abzugs -- der
  haette das Passwort sonst in eine Datei geschrieben.
- L23: Zeitserver-Feld erlaubt 32 Zeichen, der ESP speichert 16. Still
  gekuerzt, danach scheitert die Zeitsynchronisation stumm.
- L24: Verstecktes WLAN ist ueber die PWA nicht einrichtbar -- die SSID ist nur
  aus der Trefferliste waehlbar.

## 2026-10-02 UI/UX-Block abgeschlossen (ESP 3.2.6, PWA 1.4.74)

STM unveraendert bei 3.2.8 -- an `src/**` wurde nichts angefasst.

### Ultrabreit: die Oberflaeche endete bei 1540 px

Auf einem 3440er Monitor blieben links und rechts je rund 950 px leer. Zwei neue
Stufen (ab 1700 px und ab 2400 px) ziehen den Inhalt nicht breiter, sondern geben
ihm eine SPALTE mehr: die Statuskacheln stehen jetzt zu dritt statt zu zweit, bei
2400 px zu sechst, und System, Klima, Ambilight, Netzwerk und Wartung sind
dreispaltig. Die Textzeile bleibt dabei gleich lang -- eine Zeile ueber rund 90
Zeichen liest sich schlecht, egal wie viel Platz daneben ist.

Gemessen: 3440x1440 vorher 1540 px genutzt (45 %), jetzt 2280 px (66 %).
Bei 5120 px bleibt es bei 2280 px; mehr waere messbar breiter, aber nicht mehr
lesbar.

### Die Luecke zwischen 561 und 899 px ist zu (Review 2, Massnahme 15)

Darunter griff der Telefonblock, darueber das zweispaltige Raster -- dazwischen
nichts. Bei 852 px, also dem iPhone im Querformat, stand die ganze Oberflaeche als
EINE Spalte von 824 px da, bei 393 px Hoehe. Jetzt zweispaltig; die
Modulnavigation bleibt bewusst beim bisherigen Scrollverhalten, weil neun Reiter
nebeneinander bei 600 px nicht passen.

### Nicht-Text-Kontrast (Review 1, Massnahme 11)

Die Raender der Bedienelemente lagen bei 1,35:1 -- WCAG 1.4.11 verlangt 3:1. Neues
Token `--line-control`; `--line` bleibt fuer dekorative Trennlinien. Am gerenderten
Feld gemessen: **3,58:1 gegen die eigene Flaeche, 3,93:1 gegen die Umgebung.**
Regler, Kontrollkaestchen und Auswahlknoepfe bekommen `accent-color`, das Farbfeld
einen zweiten, dunklen Ring -- ein einzelner heller Ring verschwindet auf hellen
Farben.

### Modal auf Dialog-Standard (Review 1, Massnahme 14)

Das Kartenmodal war eine Ebene mit dunklem Hintergrund, mehr nicht. Mit Tab landete
man dahinter in Feldern, die man nicht sieht; Escape tat nichts; nach dem
Schliessen sass der Fokus wieder am Seitenanfang. Jetzt `role="dialog"`,
`aria-modal`, Fokus hinein, Tab-Falle, Escape, Fokus zurueck auf die oeffnende
Schaltflaeche, Hintergrund `inert`, Seite gesperrt.

Dazu `dvh` statt `vh`: 92vh rechnet mit der AUSGEKLAPPTEN Adressleiste. Auf dem
iPhone im Querformat ragte das Modal darunter, und "In Wetter uebernehmen" war
nicht erreichbar. Gemessen bei 393 px Viewport: 362 px statt 980 px Deckelung.

### PNG-Icons (Review 1, Massnahme 15)

`apple-touch-icon` zeigte auf ein SVG. iOS wertet das nicht aus und legt beim "Zum
Home-Bildschirm" statt des Icons einen verkleinerten Abzug der Seite ab. Neu:
`icon-192.png`, `icon-512.png`, `icon-180.png` fuer iOS und `icon-mask.png` als
eigenes Maskable-Icon -- Android beschneidet maskable Icons auf 80 % Durchmesser,
und der gruene Punkt des bisherigen Motivs lag ausserhalb dieser Zone.

Der kurze Name `icon-mask.png` ist kein Geschmack: Das LittleFS meldet
maxPathLength 32, und die Ablage flacht `app/icons/...` zu `app-icons-...` ab. Aus
`icon-maskable-512.png` waeren 34 Zeichen geworden -- und das waere erst auf dem
Geraet aufgefallen, nicht hier.

**Die Icons brauchen ESP 3.2.6 auf dem Geraet.** `APP_INSTALL_ASSETS` in `http.cpp`
ist die Weissliste fuer `/api/app_file_upload`; eine aeltere Firmware weist die vier
PNG-Dateien ab.

### Am Werkzeug

- `tools/make-icons.sh` rastert die Icons mit demselben Chrome wie die Vorschau.
  Chrome schreibt den Abzug dabei NICHT direkt ins Projekt: unter einer Sandbox,
  die Schreibzugriffe einzeln freigibt, haengt der Prozess dort stumm -- erst in
  den Temporaerordner, dann kopieren.
- `tools/preview/diag.js` misst zusaetzlich die genutzte Breite, den
  Nicht-Text-Kontrast am gerenderten Feld und das Modal (Rolle, Fokus, Escape,
  Fokusrueckgabe, Deckelung). Damit belegt die Messreihe die Korrekturen, statt
  dass ich sie behaupte.

### Messreihe ueber 20 Formate, 320x568 bis 5120x1440

0 px horizontaler Ueberlauf, 0 abgeschnittener Text, 0 Touch-Ziele unter 44 px.

## 2026-10-02 Interne Netzstruktur raus aus dem oeffentlichen Repo

Reine Werkzeug- und Dokumentationsaenderung, kein Produktcode, deshalb kein
Versionsbump.

Adresse der Uhr, NAS-Hostname mit Port und Benutzer sowie Client-IPs aus zwei
Logauszuegen standen in versionierten Dateien. Keine Zugangsdaten, aber die
komplette interne Netzstruktur. Sie kommen jetzt aus `tools/device.conf` und
`tools/deploy.conf`, beide gitignored; fehlt die Datei, brechen die Skripte mit
einem Hinweis ab. Guardrail S9 prueft das mit.

**Die Historie ist davon nicht beruehrt** -- in fuenf aelteren Commits stehen die
Werte weiterhin.

## 2026-09-30 Erste Messungen am laufenden Geraet

Reine Werkzeug- und Dokumentationsaenderung, kein Produktcode, deshalb kein Versionsbump.

Der Mitschnitt laeuft. Drei Ergebnisse, alle am Geraet gemessen statt gerechnet.

**Kernbefund 4 aus REVIEW.md ist bestaetigt, aber entschaerft.** 10 HTTP-Requests
erzeugen exakt 20 Debugzeilen auf der STM-UART, rund 106 Byte je Request -- die
Schaetzung "~100 Byte" stimmt. Die Burst-These stimmt dagegen nicht: 5 Runden zu je 3
parallelen Requests ergaben 30 von 30 erwarteten Zeilen, kein Verlust. Der 256-Byte-Ring
wird laufend geleert, die 300 Byte treffen nicht in einem Fenster ein. Massnahme 6
bleibt richtig, ist aber kein akuter Fehler.

**Die Logmenge ist weit kleiner als angenommen.** Gemessen: 37 Byte/s, 28 Zeilen/min,
rund 3 MB/Tag. Meine Schaetzung in der Logger-Anleitung lag bei mehreren hundert MB/Tag
-- Faktor 225 daneben. Sie stuetzte sich auf "Minuten-LEDs mit 64 Hz" aus REVIEW.md;
tatsaechlich erscheinen im Ruhezustand rund 0,1 sk6812_refresh-Paare je Sekunde. Die
Anleitung ist korrigiert, die Annahme als Befund L12 festgehalten. Offen bleibt die Rate
waehrend Ticker und Animation.

**Eine zerrissene Zeile beobachtet** (L13): zwei Ausgaben ineinandergeschoben. Im
gezielten Nachtest nicht reproduzierbar -- ein Einzelfall unter rund 200 Zeilen.

### Am Werkzeug nachgebessert

- `log.sh stats` bildet die Rate jetzt ueber die gesamte Laufzeit der Datei statt ueber
  ein Fenster von fuenf Sekunden. Der Verkehr kommt schubweise, die kurze Messung landete
  regelmaessig bei null und war damit irrefuehrend.
- In der Anleitung stand, eine Luecke im Log heisse, der Hauptloop habe gestanden. Das
  ist zu absolut: Die Firmware protokolliert nur bei Ereignissen, im Ruhezustand vergehen
  regelmaessig zehn Sekunden ohne eine Zeile. Aussagekraeftig ist eine Luecke erst
  zwischen Zeilen, die zusammengehoeren -- etwa `sk6812_refresh: start` und
  `dma started`. Entsprechend praezisiert.

## 2026-09-30 Dauerhafter Mitschnitt des Debug-UART

Reine Werkzeugaenderung, kein Produktcode, deshalb kein Versionsbump.

Ein Raspberry haengt ueber einen USB-TTL-Adapter am Stecker H6 und schneidet
durchgehend mit, was der STM32 sendet. Der Mitschnitt ist ueber SSH abrufbar.

**Warum das mehr ist als /api/stm32_log:** Der Endpunkt der PWA versagt genau dann,
wenn man ihn braucht -- haengt die Uhr, antwortet der ESP nicht mehr. Der Mitschnitt
laeuft durch, auch durch einen Watchdog-Reset hindurch, und faengt danach die
Reset-Ursache aus der Startsequenz. Vier der offenen Messungen in BEFUNDE.md sind
ohne ihn nicht zu beantworten.

Neu unter `tools/logger/`:

- `README.md` -- vollstaendige Anleitung von der Verkabelung bis zu den Testablaeufen,
  mit der Warnung zu H6 Pin 1: dort liegen 3,3 V hinter einer Schottky-Diode, die zur
  Platine zeigt. 5 V an dieser Stelle zerstoeren STM32, ESP, RTC und EEPROM.
- `serial-logger.py` -- Mitschnitt mit Zeitstempel auf Millisekunden, ueberlebt das
  Abziehen des Adapters, mischt Markierungen aus einer FIFO ein, damit Testschritte im
  Log auffindbar sind.
- `log.sh` -- Abruf vom Mac: tail, grep ueber rotierte Dateien, since, follow, mark,
  stats und **gaps**. Letzteres findet Luecken im Mitschnitt: Die Firmware gibt laufend
  aus, eine Pause heisst also, dass der Hauptloop stand. Das ist die direkteste Messung
  von Blockaden, die es gibt, und ohne durchgehenden Mitschnitt nicht moeglich.
- systemd-Unit, udev-Regel fuer den festen Geraetenamen und logrotate mit SIGHUP statt
  copytruncate, damit beim Rotieren keine Zeilen verlorengehen.

### Am laufenden Geraet festgestellt

Der Temperatursensor liefert nichts: Die Uhr meldet durchgehend
`DS18xxx temperature: 127.5`, und das ist kein Messwert, sondern der Fehlercode --
intern 255, geteilt durch zwei. Die RTC-Temperatur daneben ist plausibel, der I2C-Bus
ist also in Ordnung und nur der 1-Wire-Pfad gestoert. Das ist das offene Thema 1 aus
CLAUDE.md, erstmals am Geraet belegt statt hergeleitet.

Ausserdem laeuft auf der Uhr STM 3.2.4, waehrend der Projektstand bei 3.2.6 liegt. Die
icon_freeze-Instrumentierung ist damit noch nicht geflasht -- Test T6 geht erst danach.

## 2026-09-30 LED-Board erfasst, LED-Typ korrigiert

Reine Dokumentationsaenderung, kein Produktcode, deshalb kein Versionsbump.

`HARDWARE.md` um das LED-Board erweitert: 114 LEDs, das sind 110 Matrix (10x11) plus
4 Minutenpunkte -- exakt `DSP_DISPLAY_LEDS 110` und `DSP_MINUTE_LEDS 4` der Firmware.
Der LDR (GL5528) sitzt auf dem LED-Board, nicht auf dem Controller.

**Korrektur:** Verbaut sind **SKC6812RGBW-BW** von OPSCO (LCSC C5181320), nicht
SK6812RGBW. Die Unterschiede betreffen das Protokoll: Die SKC-Variante braucht eine
Reset-Pause von mehr als 200 us statt 80 us. Die Firmware traegt dem bereits Rechnung
-- `sk6812.c` fuehrt zwei Timing-Saetze, aktiv ist der mit 250 us Pause, kommentiert
mit "usable for SK6812 and SK6812C". Der abgeschaltete Zweig haette nur 100 us und
wuerde sporadische Anzeigefehler erzeugen, die wie ein Wackelkontakt aussehen.

Weiter dokumentiert: Der Stecker zwischen den Platinen ist **gespiegelt** belegt
(Controller-Pin n gehoert an LED-Board-Pin 8-n), und in der Datenleitung liegen zwei
Serienwiderstaende hintereinander -- 330 Ohm auf dem Controller plus 220 Ohm auf dem
LED-Board, zusammen 550 Ohm.

### Neue Befunde L9 bis L11

- **L9:** Die Rueckstellsicherung im USB-C-Zweig haelt 1,5 A. Bei heller Anzeige ueber
  R+G+B kommen nach ueblicher Annahme rund 2,4 A zusammen. Eine Rueckstellsicherung
  schaltet nicht ab, sondern laesst die Spannung allmaehlich sacken -- das passt zum
  Bild "laeuft meistens, haengt gelegentlich". Rechnung, keine Messung.
- **L10:** Zwei Unstimmigkeiten in der Stueckliste des LED-Boards (C116-Wert, falsche
  LCSC-Nummer bei R4).
- **L11:** Kein Ambilight-Ausgang am LED-Board -- die Kette endet auf der Platine.

## 2026-09-30 Hardware erfasst und gegen die Firmware abgeglichen

Reine Dokumentationsaenderung, kein Produktcode, deshalb kein Versionsbump.

### Neu: HARDWARE.md

Das KiCad-Projekt der Platine ausgelesen (Schaltplan, PCB, Stueckliste) und **jede
Pinangabe gegen den Firmware-Code abgeglichen**. Die Platine ist eine Eigenentwicklung
"WordClock USB-C / STM32F411 V2" und kein BlackPill-Modul mit Zusatzplatine -- aber
bewusst pinkompatibel dazu, weshalb `BLACKPILL_BOARD` passt.

Vier Erkenntnisse, die man dem Code allein nicht ansieht:

- **Das EEPROM haengt am I2C** (AT24C32M, zusammen mit der DS3231-RTC). Der STM32F411
  hat keins. Die oft zitierten "16 ms pro Byte" sind der Schreibzyklus dieses
  Bausteins, nicht Flash-Programmierung.
- **PB0 schaltet die 5-V-Versorgung der LED-Kette** ueber zwei MOSFETs. Der
  `delay_msec(200)` nach `power_on()` ist die Einschwingzeit, keine Willkuer.
- **U3 (SN74AHCT1G125) hebt die Datenleitung von 3,3 V auf 5 V.** Die Notiz in
  README.md, 3,3 V seien an 5-V-versorgten SK6812 grenzwertig, gilt nur fuer die
  Vorgaengerbestueckung. Korrigiert.
- **Der ESP hat nur einen vollwertigen UART, und das ist die Bruecke zum STM.** Jede
  Serial.print-Debugzeile des ESP landet deshalb zwangslaeufig auf der STM-UART. Damit
  ist Kernbefund 4 aus REVIEW.md hardwareseitig bestaetigt: Es ist keine
  Nachlaessigkeit im Code, sondern eine Folge der Verdrahtung.

### Neuer Befund L7

`eeprom_write()` schreibt Byte fuer Byte und wartet nach jedem 15 ms. Der Kommentar
dort dreht die Begruendung um -- man wartet **nach** einem Zyklus, und ein Zyklus
fasst beim AT24C32M eine ganze 32-Byte-Seite. `i2c_write` kann das bereits. Fuer die
Dimmkurve bedeutet das 240 ms statt 15 ms, Faktor 16. Da die PWA 16 solche Kommandos
sendet, erklaert das REVIEW.md Kernbefund 3 an der Wurzel. Heikel im Umbau, braucht
eigene Spec.

## 2026-09-30 Regeln maschinell erzwungen, Ablaeufe als Skills

Reine Werkzeug- und Dokumentationsaenderung, kein Produktcode, deshalb kein Versionsbump.

Nach einer Recherche zum aktuellen Stand der Claude-Code-Funktionen umgesetzt. Drei
Faehigkeiten waren ungenutzt, und alle drei zielen auf Probleme, die dieses Projekt hat.

### Regeln, die jetzt greifen statt nur dazustehen

- **R1 "nur der Lead baut"** haengt als PreToolUse-Hook im Frontmatter aller
  schreibenden Agenten ausser dem `release-engineer`. Abgewiesen werden `make`, `cmake`,
  `arduino-cli` und `guardrails.sh --full`; harmlose Ziele wie `make stm-version-file`
  bleiben erlaubt. Grund: f103 und f411 bauen beide mit -j4 in dasselbe Verzeichnis,
  parallele Builds korrumpieren den CMake-Cache.
- **Ein Stop-Hook** blockiert das Turn-Ende, wenn ueberwachte Dateien geaendert sind und
  die Guardrails dafuer nicht liefen. Drei Absicherungen verhindern Dauerblockaden:
  `stop_hook_active`, ein Hash pro Aenderungsstand (pro Stand wird hoechstens einmal
  blockiert) und ein Durchlassen bei jedem Fehler im Hook selbst. Die Hashberechnung
  liegt bewusst nur an einer Stelle — zwei Fassungen wuerden auseinanderlaufen und
  dauerhaft blockieren.

### Ablaeufe als Skills

`CLAUDE.md` war auf 248 Zeilen gewachsen und lud bei jeder Sitzung komplett, auch die
STM-Interna waehrend PWA-Arbeit. Die Ablaeufe liegen jetzt unter `.claude/skills/`:

- `/release` — Build, Versionspflicht, Rollout, was zu flashen ist
- `/pwa-vorschau` — PWA ohne Geraet ansehen, mit den drei Chrome-Eigenheiten
- `/doku-nachfuehren` — CHANGELOG, READMEs, Befundkatalog
- `stm-firmware` — belegtes Detailwissen zur Firmware, laedt automatisch ueber
  `paths: src/**`

`CLAUDE.md` ist damit auf 178 Zeilen. Die Kurzregeln bleiben dort, weil sie immer
gelten; die Prozeduren laden nur bei Bedarf.

### Weiteres

- Die zwoelf Agenten haben Farben nach Rolle: Firmware orange, PWA blau, nur lesend
  violett, pruefen und bauen gruen, Doku und Spec cyan.
- Projektweite Berechtigungen in `.claude/settings.json` statt verstreut und mit
  absoluten Pfaden in der lokalen Datei. Bewusst **ohne** `deny` auf `deploy.sh` — eine
  deny-Regel ist nicht ueberstimmbar, und der Rollout soll moeglich sein und nur
  nachfragen.
- S9 prueft jetzt auch die Skills.

## 2026-09-29 Versionierung je Komponente, Befundkatalog, Doku-Pruefungen

Reine Werkzeug- und Dokumentationsaenderung. **Kein Produktcode beruehrt, deshalb kein
Versionsbump** — genau der Fall, den die neue Regel beschreibt.

### Geaendert

- **Versionspflicht DIR-004 von Gleichschritt auf komponentenweise umgestellt.** Jede
  Komponente wird genau dann versioniert, wenn sich ihr Code geaendert hat. Aendert ein
  Release nur den STM-Code, steigt nur dessen Version. Vorher wurden alle drei bei jedem
  Build angehoben, was ein OTA-Update auf identische Firmware zur Folge hatte.
- Guardrail-Stufe S4 prueft das jetzt **je Komponente und in beiden Richtungen** — auch
  ein Bump ohne Codeaenderung ist ein Befund. Entscheidend war die Trennung von ESP und
  PWA: beide liegen unter `ESP8266/`, sind aber getrennt versioniert. Die vorherige
  Pruefung hielt jede PWA-Aenderung fuer eine ESP-Aenderung.
- Bei der Aenderungserkennung wird die Versionszeile aus dem Diff ihrer eigenen Datei
  gefiltert. Ohne das waere jeder Bump fuer sich schon eine "Codeaenderung" und die
  Gegenprobe koennte nie anschlagen.

### Neu

- **`BEFUNDE.md`** — lebender Massnahmenkatalog. Die beiden Reviews sind Momentaufnahmen
  und werden nicht fortgeschrieben; ihr Stand lebt jetzt an einer Stelle: alle 34
  Massnahmen mit Status und nachpruefbarem Beleg, dazu die Befunde aus der laufenden
  Arbeit unter eigenen `L`-Nummern.
- **Guardrail S9** — Aktualitaet der lebenden Dokumentation. Vergleicht Versionsangaben
  gegen die Quellen und meldet absolute Benutzerpfade.
- **Guardrail S10** — Vollstaendigkeit des Katalogs. Schlaegt an, wenn eine
  Massnahmennummer aus einem Review in `BEFUNDE.md` fehlt oder die `L`-Nummerierung eine
  Luecke hat.

### Behoben in der Dokumentation

- Der Kopf von `README-CMAKE.md` behauptete einen "aktuellen Abschlussstand", der rund
  dreissig PWA-Versionen zurueck lag. Versionsangaben aus den lebenden Dokumenten
  entfernt statt gepflegt — was nicht dasteht, kann nicht veralten.
- Sieben absolute Pfade `/Users/<name>/...` in `CHANGELOG.md`, `README-CMAKE.md` und
  `.claude/settings.json`. Sie zeigen bei jedem anderen Klon ins Leere.
- 14 Stellen mit `ss`-Verstoss gegen die Schweizer Schreibung in vier lebenden Dokumenten.
- Ein Abschnitt in `README-CMAKE.md` beschrieb einen Stand vom April als "aktuell
  verifiziert".

## 2026-09-29 Restore-Luecke, Guardrails und Werkzeugschicht

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-09-29-2341.zip`
- STM32: `3.2.6` · ESP8266: `3.2.2` · PWA: `1.4.70` · SW-Cache: `wordclock-app-v62`

### Behoben

- **Display blieb nach einem Wetter-Ticker bis zu 60 Sekunden dunkel**, bei
  `WCLOCK24H == 0` bis zu fuenf Minuten. Ursache: `pending_weather_ticker_restore` wurde
  unbedingt geloescht, `UPDATE_ALL` aber nur gesetzt, wenn gerade kein anderes Update
  anstand. Da die Flags disjunkte Bits sind, ging das Restore in diesem Fall verloren.
  Ausgeloest wurde es aus der PWA durch das Speichern der Dimmkurve, ohne PWA durch die
  Helligkeitsautomatik.
- **Zwei Aufrufe nicht vorhandener Funktionen in der PWA.** Beim Layout-Tabellen-Upload
  wirkte der Button tot, ohne Fehlermeldung. Bei laufender Farbanimation fror die
  WordClock-Vorschau auf der alten Farbe ein.
- **Drei fehlende Uebersetzungsschluessel** standen woertlich als `common.setting` auf
  Buttons, in beiden Sprachen. Dazu 14 Schluessel, die in der englischen Oberflaeche
  deutschen Text zeigten.
- **Release-Notes werden nicht mehr ungefiltert eingesetzt.** Der Inhalt kommt vom
  konfigurierbaren Update-Host ueber HTTP. Ersetzt durch einen Whitelist-Filter, der den
  Baum aus frisch erzeugten Elementen neu aufbaut; Attribute werden nicht uebernommen.
- **Drei stille Fehlerschlucker.** Beim Overlay-Loeschen im Backup-Import wird die Zahl
  fehlgeschlagener Plaetze jetzt sichtbar gemeldet statt verschluckt.

### Neu: Messung statt Vermutung

- **`do_display_icon`-Freeze wird sichtbar.** Wird das Display ausgeschaltet, waehrend
  ein Icon laeuft, bleibt ein Flag dauerhaft gesetzt — mit der Folge, dass
  Temperatur-Restore, Wetter-Ticker-Restore und Helligkeitsautomatik einfrieren, solange
  das Display aus ist. Eingebaut ist **nur die Messung**, kein Fix: eine Logzeile bei
  Zustandswechsel. Erscheint sie beim Ausschalten, ist der Freeze belegt; bleibt sie aus,
  ist er widerlegt.

### Neu: Werkzeuge

- **`./tools/guardrails.sh`** — acht Pruefstufen, Laufzeit Sekunden, ohne neue
  Abhaengigkeiten. Findet unter anderem undefinierte Funktionsaufrufe, fehlende
  i18n-Schluessel, veraltete `.gz` und ungefiltertes `innerHTML`.
- **`tools/preview/`** — die PWA laeuft ohne Geraet, inklusive Vermessung ueber mehrere
  Bildschirmgroessen. Ersetzt keinen Test am Geraet, faengt aber Layoutfehler ab.
- **`./tools/deploy.sh`** — Rollout auf den Update-Server. Prueft jedes Artefakt, bevor
  es etwas ueberträgt, und loescht auf dem Ziel nichts.

### Bekannt und bewusst offen

- Die unbedingten Logausgaben im Refresh-Pfad und das fehlende `watchdog_reload()` in den
  blockierenden Pfaden bleiben. Beide wuerden das Timing veraendern und damit die
  `do_display_icon`-Messung unbrauchbar machen. Eigener Schritt danach.
- Der App-Bundle-Weg (`app-bundle.txt`) wird **nicht mehr verwendet**: Der ESP meldet
  die Unterstuetzung ausdruecklich als nicht vorhanden, die zugehoerigen URLs sind leer,
  die Legacy-Seite bietet keinen Upload, und auf dem Update-Server liegt keine solche
  Datei. `build-app-bundle.py` laeuft ohnehin nicht mehr durch. `APP-BUNDLE.md` ist als
  historisch gekennzeichnet; die vier Dateien koennen entfernt werden.

### Konventionen

- Bei jedem Build werden **alle drei Komponenten im Gleichschritt** versioniert, auch
  wenn sich die jeweilige nicht geaendert hat.
- `tools/deploy.sh` setzt nach jedem Rollout ein Tag `release/<stm>-<esp>-<app>`.


## 2026-04-29 Bilingual PWA Finalization And Safari Reload Hardening

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-29-2252.zip`
- PWA-Version: `1.4.61`

Wichtige Punkte:

- Die PWA-Oberfläche ist jetzt durchgängig zweisprachig mit:
  - Default `Deutsch`
  - optional `English`
  - persistierter Sprachwahl im Browser
- Der Sprachumbau umfasst jetzt nicht nur statische Labels, sondern auch:
  - Save-/Busy-/Success-Buttontexte
  - Laufzeit- und Wartungsmeldungen
  - Remote-Update-/STM32-Flash-Fortschritt
  - LittleFS-/Update-/Dateiaktionen
  - Overlay-, Timer- und DFPlayer-Aktionen
- Mehrere hart codierte deutsche Rücksprungtexte in Button-Handlern wurden auf zentrale `i18n`-Keys umgestellt
- `restoreText`-Synchronisierung für Buttons wurde vereinheitlicht, damit Buttons nach einem Klick nicht wieder auf alte deutsche Idle-Texte zurückspringen
- Mobile-Safari-Reload wurde auf ESP-Seite weiter gehärtet:
  - kürzere Behandlung leerer/angebrochener Requests
  - eigene kurze Request-Line-Leselogik statt direkter Blockierung über `readStringUntil('\r')`
- Der Auto-Refresh-Takt der PWA liegt jetzt mit `+1 s` Versatz auf:
  - `:02`
  - `:17`
  - `:32`
  - `:47`

Wichtige technische Hinweise:

- Restliche Browser-Systemtexte an nativen Dateifeldern wie `Datei auswählen` oder `Keine Datei ausgewählt` kommen weiterhin vom Browser selbst und nicht aus der PWA.
- Der aktuelle Sprachstand wurde bewusst nicht nur über HTML-Attribute, sondern zusätzlich über die Laufzeitpfade in [app.js](/ESP8266/ESP-uclock/data/app/app.js) bereinigt, weil dort die meisten Rücksprungtexte sassen.

## 2026-04-29 Gzip PWA Rollout, Restore Timing And Safari Hardening

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-29-1957.zip`
- PWA-Version: `1.4.37`

Wichtige Punkte:

- PWA-App-Dateien werden jetzt gzip-komprimiert gebaut, hochgeladen und vom ESP bevorzugt als `.gz` ausgeliefert
- `/app` prüft installierte App-Dateien nicht mehr nur auf Existenz, sondern auch auf Dateigrösse `> 0`, damit Crash-Reste mit leeren Dateien nicht als gültige Installation gelten
- Remote-App-Installpfad für `/app/?action=install` wurde auf Stack- und Timing-Probleme gehärtet
- Mobile-Safari-Ladepfad wurde stabilisiert:
  - statische `/app`-Assets senden jetzt `Connection: close`
  - der ESP beendet die Verbindung nach dem Dateistream explizit
- Import/Restore wurde in mehreren empfindlichen Bereichen entschärft:
  - Overlay-Restore mit absteigendem Delete, sequentiellem Wiederaufbau und zusätzlicher Entkopplung
  - Timer-Restore mit grösseren Abständen und Reload-Schritten
  - `ticker_deceleration` mit zusätzlichem Schutzabstand vor und nach dem Write, damit serielle Kommandos nicht ineinanderlaufen
- Der globale Hinweis für fehlende Layout-Tabellen erscheint jetzt erst, wenn die Tabellen-Info wirklich geladen ist, und flackert nicht mehr beim Start auf

Wichtiger technischer Hinweis:

- Der iPhone-Safari-Fall war zuletzt kein klassischer Frontend-Fehler, sondern ein empfindlicher HTTP-/Connection-Fall beim Ausliefern der PWA-Assets.
- Der Restore-Fehler bei `ticker_deceleration` zeigte sich im Log als Kommando-Interleaving auf der seriellen STM32-Strecke und wurde deshalb bewusst über Timing-Abstände statt über ein Format- oder Mapping-Rework gelöst.

## 2026-04-15 PWA Update Visibility And UX Polish

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-15-0127.zip`
- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.55`

Wichtige Punkte:

- die PWA zeigt die Versionsstände jetzt in der Reihenfolge `WordClock`, `ESP`, `App`
- zusätzlich zur lokalen `App-Version` wird jetzt auch `App verfügbar` vom Update-Server angezeigt
- dafür wird beim Build eine `app-version.txt` erzeugt und zusammen mit `app-bundle.txt` ins Release aufgenommen
- der Update-Server muss für die PWA-Versionsprüfung jetzt sowohl `app-bundle.txt` als auch `app-version.txt` bereitstellen
- mehrere bislang stille PWA-Aktionen haben jetzt konsistentes Button-Feedback, darunter `Overrides anwenden`, `Overrides zurücksetzen`, Overlay-Aktionen und Teile des Wetterdialogs

## 2026-04-15 PWA ESP Update Timing Fix

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-15-0024.zip`
- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.45`

Wichtige Punkte:

- der ESP-Updatepfad der PWA wartet jetzt deutlich länger, bevor Reconnect und Reload gestartet werden
- damit orientiert sich die PWA wieder am Legacy-Verhalten des ESP-Updates mit rund `40` Sekunden Reconnect-Zeit
- der zu frühe PWA-Reload nach ca. `25-30` Sekunden wird vermieden und bricht das ESP-Update nicht mehr vorzeitig ab
- der PWA-Button für ESP-Updates verwendet jetzt wieder den bewährten Legacy-Updatepfad als Top-Level-Navigation statt eines versteckten `iframe`
- Legacy-Weboberfläche liefert HTML jetzt mit `UTF-8`-Charset aus und verwendet wieder gut lesbare Linkfarben statt Gelb/Weiss auf Weiss

## 2026-04-14 Runtime Recovery And Reset Visibility

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-15-0024.zip`
- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.44`

Wichtige Punkte:

- `STM32`-Schutzmassnahmen ergänzt: `IWDG` mit ca. `20 s` Timeout
- `HardFault`, `MemManage`, `BusFault` und `UsageFault` führen nicht mehr in eine Endlosschleife, sondern loggen kurz und starten das Board kontrolliert neu
- Reset-Ursachen aus den `RCC`-Flags werden beim Boot weiterhin geloggt
- relevante Reset-Ursachen werden jetzt zusätzlich ohne serielles Kabel in die PWA gespiegelt
- in der PWA erscheint im Überblick nur beim aktuellen Boot und nur bei relevanten Flags ein Eintrag `Letzter STM32-Neustart`
- frühe blockierende Watchdog-Initialisierung wurde korrigiert, damit der Controller nach `Reset flags:` normal weiter bootet

Hinweise:

- normale Einschalt-/Pin-Resets werden in der PWA bewusst nicht angezeigt
- für die Anzeige in `/app` müssen sowohl die `STM32`-Firmware als auch das aktuelle `app-bundle.txt` ausgerollt werden

## 2026-04-09 Finalized Restore Release

Aktueller verifizierter Abschlussstand:

- STM-Version: `3.2.0`
- ESP-Version: `3.2.0`
- PWA-Version: `1.2.43`

Wichtige Punkte:

- Backup/Restore läuft jetzt wieder sauber mit automatischem `STM32`-Reset und anschliessendem PWA-Reload
- `Update-Host/-Pfad` werden nach dem Boot nicht mehr durch ESP-Defaults auf den STM zurückgeschrieben
- `Zeitserver`, `Zeitzone`, `Sommerzeit`, `RTC`- und `DS18xx`-Korrektur bleiben nach dem Restore und Reboot erhalten
- Overlay-Restore ist stabil
- die temporären `EEPDBG`-Diagnoseausgaben wurden wieder entfernt
- harmlose Protokollreste für obsolete/TFT-fremde Variablen wurden bereinigt

## 2026-04-08 Current Working Release

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-08-2131.zip`

Ergänzungen gegenüber den früheren 2026-04-08-Ständen:

- PWA-Backup/Restore weiter stabilisiert
- Overlay-Restore repariert
- Asset-/Layout-Restore weiter gehärtet
- Netzwerk-/Zeiteinstellungen werden beim Restore nochmals verifiziert
- `Ambilight online/offline` wird nach dem `STM32`-Reset nochmals als Laufzeitstatus gesetzt
- Update-/Flash-Aktionen merken jetzt die Scroll-Position und springen bei Erfolg sauber zurück
- PWA-Quelldateien haben jetzt einheitliche Dateikopf-Kommentare

Wichtige technische Hinweise:

- Der Overlay-Fix liegt in [vars.cpp](/ESP8266/ESP-uclock/vars.cpp): `set_overlay_var()` macht jetzt am Ende ein `Serial.flush()`.
- Der Live-Farbpfad für `Rainbow` wurde in [display.c](/src/display/display.c) gedrosselt:
  - kein Versand bei jedem einzelnen Farbschritt mehr
  - stattdessen nur noch höchstens einmal pro Sekunde an den ESP
- `Daylight` sendet die Live-Farbe weiterhin nur beim echten Stundenwechsel.
- PWA-Version dieses Stands: `1.2.28`

## 2026-04-08 F411 SK6812 Output Fix

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-08-0031.zip`

Ergänzungen gegenüber dem Stand vom 2026-04-07:

- SK6812-Treiber für `STM32F411CE BlackPill` korrigiert
- `TIM3 / PB1` nutzt auf dem BlackPill jetzt konsistent `TIM3_CH4`
- vollständiger Release-Workflow erneut verifiziert über `make release-zip`

Wichtiger technischer Hinweis:

- Der Fehler lag im SK6812-Treiber in [src/sk6812/sk6812.c](/src/sk6812/sk6812.c).
- Für `BLACKPILL_BOARD` war das DMA-/GPIO-Mapping bereits auf `TIM3_CH4 / PB1` ausgelegt, die Timer-Output-Compare-Initialisierung lief aber noch fest über `TIM_OC1...`.
- Dadurch konnte auf `STM32F411CE BlackPill` trotz korrekter Pinbelegung kein gültiges SK6812-Ausgangssignal auf `PB1` entstehen.
- Der Treiber verwendet jetzt die zur Board-Konfiguration passende OC-Initialisierung pro Kanal.
- Ein externer Pull-up kann das Verhalten zusätzlich beeinflussen, weil ein SK6812-Eingang an `5V` Versorgung mit reinem `3.3V`-High am Datenpin im Grenzbereich liegen kann.

## 2026-04-06 Stable Baseline

Verifizierter stabiler Referenzstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-06-2339.zip`

Inhalt dieses Basisstands:

- funktionierender `12h RGBW`-STM-Build für `STM32F103`
- funktionierender `12h RGBW`-STM-Build für `STM32F411CE 25 MHz`
- ESP8266-Build als `ESP-WordClock-4M.bin`
- versioniertes `app-bundle.txt`
- gemeinsamer Release-Workflow über `make release-zip`

Wichtiger technischer Hinweis:

- Die Uhranzeige auf der Hardware funktioniert in diesem Stand wieder sauber.
- Der kritische Rückbau erfolgte in [src/display/display.c](/src/display/display.c), damit die STM-Farbpfade wieder dem funktionierenden Verhalten aus `3.1.5` entsprechen.
- Zusätzliche `var_send_display_colors();`-Aufrufe direkt in `display_init_color_animation_rainbow()` und `display_init_color_animation_daylight()` wurden als instabil verifiziert und bleiben in diesem Basisstand bewusst draussen.
- Änderungen in diesem Bereich sollten künftig nur schrittweise und testbar wieder eingeführt werden.

## 2026-04-07 Stable Runtime Color Hook

Aktueller verifizierter Arbeitsstand:

- Release-ZIP: `build/releases/wordclock-release-2026-04-07-0123.zip`

Ergänzungen gegenüber der Basis:

- PWA-Vorschau-Refresh sauber auf echte 5-Sekunden-Grenzen synchronisiert
- Auto-Refresh mit `+1.0s` Versatz, damit Minutenpunkte nach dem echten Umschalten zuverlässig erfasst werden
- Live-Farbpfad für `Rainbow` und `Daylight` über sichere Laufzeitstellen wieder aktiviert
- `/app`-Startpfad und automatische Erstinstallation der PWA auf dem ESP weiter gehärtet
- Status- und Fehlerseiten der Legacy-/Autoinstallationspfade sprachlich bereinigt und mit echten Umlauten versehen
- PWA-Kernladepfad auf echte Pflichtdaten reduziert, langsamere Nebenpfade werden nachgeladen

Wichtiger technischer Hinweis:

- `var_send_display_colors();` funktioniert stabil im laufenden `Rainbow`-Pfad
- `var_send_display_colors();` funktioniert stabil beim echten `Daylight`-Stundenwechsel
- `var_send_display_colors();` in den Init-Funktionen von `Rainbow` und `Daylight` bleibt weiterhin bewusst deaktiviert
- dieser Stand ist die aktuelle Referenz für funktionierende Live-Farben ohne Hardware-Ausfall
- PWA-Version dieses Stands: `1.2.11`
