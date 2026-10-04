# Design — Paket 2026-10-05

**Erstellt:** 2026-10-04. Momentaufnahme (DIR-006), wird nicht fortgeschrieben.

---

## 1. Warum Runde V zuerst kommt

Eine Spec, die Umfang aus einer Arbeitsliste nimmt, ist nur so gut wie deren Stand.
Beim Schreiben dieser Spec haben drei Einträge der Liste nicht mehr gestimmt (A32,
A38, A36), und einer davon — A32 — hätte eine eigene Runde erzeugt. **Das ist keine
Doku-Hygiene, sondern Planungsgrundlage.**

Dass es kein Doku-Task ist, ergibt sich aus der Sache: `BEFUNDE.md` ist bereits
korrekt in dem Sinne, dass jede Zeile sagt, was sie bei ihrer Entstehung sagte. Falsch
ist der **Bezug zur Gegenwart**, und der steht nur im Code und am Gerät. Deshalb liegt
der Durchgang bei den Analyse-Agenten und beim Lead, nicht beim `doc-writer`; der
trägt nur ein, was sie belegt haben.

### 1.1 Die drei zulässigen Verdikte

Jeder nachgezählte Befund bekommt **genau eines**:

| Verdikt | Wann | Pflichtbeleg |
|---|---|---|
| `bestaetigt` | Der Mechanismus steht unverändert im Code, oder die Beobachtung lässt sich unter ihrer ursprünglichen Bedingung herstellen | Datei:Zeile dieses Commits mit dem zitierten Ausschnitt |
| `geschlossen` | Der Mechanismus ist nicht mehr da — gleich, ob gezielt behoben oder beiläufig mitrepariert (L173) | Datei:Zeile dieses Commits **oder** Messwert mit Zeitstempel und Instrument |
| `praemisse-widerlegt` | Der Befund beschreibt etwas, das nie so war oder anders war, als er annimmt (L250: der Sender stand in `#if 0`) | der widerlegende Beleg |

**„nicht reproduziert" ist kein Verdikt.** Ein sporadischer Befund bleibt
`bestaetigt`, solange sein Mechanismus im Code steht. Das ist die Unterscheidung, an
der die Bewertung am 04.10.2026 dreimal gescheitert ist: L226 („läuft normal" ist
nicht „die Änderung wirkt"), L229 (Vorschau ist nicht Gerät), L232 (ein sauberer
Durchlauf ist kein Nachweis bei einem sporadischen Fehler).

**Ein Verweis auf einen anderen Befund ist kein Beleg.** Genau diese Kette hat bei A36
zu zwei widersprechenden Zeilen im selben Dokument geführt: Die Arbeitsliste nennt ihn
„höchste Priorität", L249 nennt ihn behoben. Wer `geschlossen` schreibt, zitiert den
Code.

### 1.2 Reihenfolge — die schwersten zuerst

**Welle 1 — alles, was heute den Status KRITISCH trägt.** Ihr Mitschleppen ist am
teuersten, weil sie Runden blockieren und Prioritäten verschieben.

| Nr. | Befund | Was zu prüfen ist | Wo es entschieden wird |
|---|---|---|---|
| 1 | **L230 / A32** | Nachsendung und Prüfsumme gebaut? | Quelltext — `var_retry_*` in `src/vars/vars.c`. **Vorprüfung: gebaut.** L254 belegt es am Gerät |
| 2 | **L255 / A38** | Setzt der `SYNCVARS`-Zweig `is_online` mit? | `src/esp8266/esp8266.c:452`. **Vorprüfung: ja.** Dazu die berichtigte Zahl: 10 bewachte Sendepfade, nicht 15 |
| 3 | **L236 / A36** | Überlauf in `convert_utf8_to_iso8859()` noch da? | `ESP8266/ESP-uclock/base.cpp:242-255`. L249 sagt behoben — **am Code nachzulesen, nicht abzuschreiben** |
| 4 | **L232 / A33** | Prüft `htoi()` heute `buf[i]` statt `*buf`? | `ESP8266/ESP-uclock/base.cpp:42` |
| 5 | **L42 / L103 / A6** | Vollabgleich nach ESP-Neustart — durch Weg B erledigt? | `src/main.c:2974`, `:3655`, L261. **Achtung L260:** Das Erfolgskriterium prüft nur das dritte von 194 Kommandos; A6 ist damit **verengt, nicht geschlossen** |
| 6 | **L205 / A30** | `overlay[0].type` noch falsch? | Gerätemessung nach einem STM-Reset; L232 nennt 164 geprüfte Felder, alle im Bereich — **ein Datenpunkt, kein Beleg** |
| 7 | **L174** | `network_scan` scannt weiterhin mitten in der Antwort? | `http_api_network_scan()` |
| 8 | **L178** | Schreibverluste — der Befund widerlegt sich im eigenen Text bereits zweimal | Zählerstand über eine Stunde Betrieb, Vergleich mit „drei Blöcke in 32 Minuten, beide durch Browserläufe" |
| 9 | **L179** | Legacy-Flashpfad ungeprüft? | `http.cpp:6909` |
| 10 | **L180 / C13** | Exception 9 beim OTA — tritt sie noch auf? | Jedes OTA seit dem 04.10.2026 ist ein Datenpunkt. **Ein gelungenes OTA ist `geschlossen` nur, wenn der vermutete Verursacher im Code nachweislich weg ist** — sonst `bestaetigt` |
| 11 | **L125 / L140 / C9 / C9a** | Absturz beim Ausliefern von `app.js` und bei `update_status` | Mitschnitt seit dem letzten Flash plus Paketzahl je Abruf |
| 12 | **A22 / A24 / A26** | **Gewichtsprüfung statt Nachzählung:** Trägt die Indexprüfung im Tabellentransfer nach A32 noch dasselbe Gewicht, wenn eine beschädigte Zeile schon an der Prüfsumme scheitert? | Analyse, Ergebnis ist eine Einstufung, keine Umsetzung |

**Welle 2 — der Abschnitt „Stand des grossen Pakets" in `BEFUNDE.md`.** Er ist die
Stelle, aus der diese Planung ihren Umfang genommen hätte, und er ist nachweislich
veraltet: Er führt Runde 4a als „noch nicht begonnen, blockiert durch A32", während
beides am Gerät steht. Nachzuzählen sind die dort genannten Tasks 1.3, 1.5, 0.11, 2.11,
3.4, 3.5, 3.6, 3.10 sowie die Runden 4a und 4b.

**Welle 3 — der Rest der Arbeitsliste, mechanisch gefiltert.** Für jeden Eintrag wird
der jüngste Commit auf die von ihm genannten Dateien gegen das Datum seines Belegs
gestellt. Nur wo sich seither etwas geändert hat, wird nachgezählt. **Das macht den
Durchgang endlich.** Ein Eintrag, dessen Dateien seit dem Beleg unberührt sind, kann
nicht beiläufig mitrepariert worden sein — das ist der ganze Mechanismus hinter L173.

### 1.3 Damit es nicht wieder passiert — Guardrail S11

S10 prüft heute, dass **jeder offene Befund** in der Arbeitsliste steht. Die
Gegenrichtung ist seit L235 ergänzt. Was keine Stufe prüft, ist die **Aktualität des
Status selbst**. S11 schliesst das mechanisch:

> Für jeden Befund mit Status **KRITISCH**: Es existiert ein Nachzählvermerk, und sein
> Datum ist **nicht älter** als der jüngste Commit auf eine der in der Spalte
> „Fundstelle" genannten Dateien.

Das erzeugt keine Fehlalarme, weil es kein Alter misst, sondern ein konkretes
Ereignis: *die Datei hat sich geändert, seit zuletzt nachgesehen wurde.* Genau das ist
der Fall, in dem ein Befund beiläufig verschwinden kann.

**Zwei Pflichten aus DIR-014 und L235 gelten für diese Stufe:** Sie muss **einmal
fehlgeschlagen** sein, und es muss nachgewiesen sein, dass sie **ihren ganzen
Gegenstand sieht** — die Zahl der geprüften Befunde wird gemeldet und von Hand gegen
die Tabelle abgezählt. Das Muster `^\| \*\*([A-F]\d+[a-z]?\d*)\*\* \|` statt
`^\| \*\*([A-F]\d+)\*\* \|` ist die Lehre aus L235 und gehört bereits in den ersten
Entwurf.

---

## 2. Runde W — das Werkzeug, das dreimal nachgebaut wurde

### 2.1 Ein Modul, zwei Wirte

Der Kern ist **nicht**, `diag.js` um Funktionen zu erweitern. Der Kern ist, dass die
Messung an **zwei** Orten gebraucht wird und heute nur an einem existiert:

| Wirt | Heute | Danach |
|---|---|---|
| `tools/preview/diag.js` (Vorschau) | misst Überlauf, Text, Touch, Fokus, Modal, einen Kontrastwert | zusätzlich Geometrie, Ankreuzfelder, Farbe am Bildpunkt, Scrollen |
| `tools/check-pwa.mjs` (Gerät, echter Browser) | prüft Ladefehler, Module, Konsole | führt **dasselbe** Messmodul aus |

Deshalb wird die Messung als eigenständiges Skriptstück abgelegt, das beide einbinden.
**Damit ist L229 geschlossen** — die Lücke, dass die Zahlen aus L227 aus der Vorschau
stammen und nie am Gerät geprüft wurden.

### 2.2 Was das Modul können muss

Hergeleitet aus den drei Fällen, in denen es von Hand nachgebaut wurde:

| Fähigkeit | Aus welchem Fall | Wofür gebraucht |
|---|---|---|
| **Kachelgeometrie** je Panel, Höhendifferenz benachbarter Kacheln | L228 | AK3.1, AKU.2 |
| **Modulwechsel** (`?module=<name>`) und das Einblenden verborgener Panels für die Messung | L239, L243 | `#tft-panel` trägt `is-hidden`, solange kein TFT gemeldet wird |
| **Scrollen** zu einem Element, vor der Messung | L243 | TFT- und Favoritenfelder liegen weit unter dem Seitenkopf |
| **Ankreuzfeld-Geometrie** je Feld, mit Klasse und Modul | L240, L242 | AKU.1 |
| **Farbe am Bildpunkt** statt gerechnet aus dem Stylesheet | L245 | vier Fehlrechnungen an **einem** Befund entstanden alle aus Rechnung statt Messung |
| **Ganzseitige Aufnahme** statt Seitenkopf | L245 | der Radialverlauf auf `body` scrollt mit — wer je Feld scrollt, misst bei jedem eine andere Verlaufsstelle |
| **Echter `:focus-visible`** über Tabulator statt `.focus()` | L243 | `.focus()` erzeugt den Zustand nicht, den die Zusage betrifft |

### 2.3 Die beiden Hooks

**E20 / L258** — `file-ownership.py` wertet bei `Bash` heute **jedes Vorkommen** eines
fremden Pfades im Befehlstext. Das hat einen Patch auf die **eigene** Datei des
Agenten abgewiesen, nur weil `tools/flash-stm.sh` im Meldungstext stand. `CLAUDE.md`
beschreibt genau diesen Fehlalarmtyp bereits für die erste Hook-Fassung — er ist
wieder da, an anderer Stelle.

Gewertet werden künftig nur Pfade in **Schreibposition**: Ziel von `>`/`>>`,
`open(…, 'w'|'a')`, `tee`, `sed -i`, Ziel von `mv`/`cp`, `truncate`.

**Was dabei ehrlich zu bleiben hat:** Der Hook wird dadurch **nicht** zum Zwang. Ein
aus Teilstücken zusammengesetzter Pfad umgeht ihn weiterhin (B22 / L171) — und genau
das hat ein Agent getan, um weiterzukommen. Die Entscheidung dieser Spec: **Es bleibt
eine Erinnerung, und das wird in die Agentenanweisungen geschrieben**, damit niemand
den Hook für mehr hält, als er ist. Eine echte Durchsetzung hinge laut R3 an einer
Reihenfolge der Hook-Aufrufe, die nirgends zugesichert ist.

**E15 / L189** — `no-danger.py` weist heute schon eine `grep`-Suche in einer lokalen
Logdatei ab, weil der Endpunktname darin vorkommt. Dieselbe Falle, dieselbe Korrektur:
Nur Aufrufe werten, die **an das Gerät gehen** (`curl`/`wget` mit einer Host-Angabe,
`http://`-URL), nicht jedes Vorkommen einer Zeichenkette.

### 2.4 Prüfstände bekommen eine Heimat

Drei Prüfstände sind in zwei Tagen entstanden und im Scratchpad verfallen: die
Schutzseite aus L249, der Drei-Wege-Vergleich der Prüfsumme (von dem nur
`tools/checks/var-crc.c` überlebt hat), und die Millisekunden-Uhr aus L256.

**Regel:** Ein Prüfstand, der eine Abnahmebedingung dieser Spec trägt, liegt unter
`tools/checks/` und läuft reproduzierbar. **Und er läuft in zwei Übersetzungen**, wo
eine Typbreite den Unterschied macht — L256: Auf dem Host ist `unsigned long` 64 Bit,
auf dem ESP 32; der erste Lauf war grün und hat den Überlauf nie hergestellt.

---

## 3. Runde E1 — der Absturz und der Speicher

### 3.1 `network_scan` (L174)

`http_api_network_scan()` sendet Kopfzeilen und erste Felder, ruft **danach**
`WiFi.scanNetworks()` und baut je gefundenem Netz einen `String` über
`sanitize_xml_string(WiFi.SSID(idx))`. Gleichzeitig hält lwIP die Sendepuffer der
bereits begonnenen Antwort. Zweimal gemessen: frei rund 6'016 Byte, grösster Block
5'752, gescheiterte Anforderung 960 — **beide Male dieselbe Aufruferadresse**.

Zwei Teile, beide nötig:

1. **Der Scan läuft vor der ersten Kopfzeile.** Dann hält lwIP noch nichts, und die
   Anforderung von rund 960 Byte trifft auf den ungeteilten Heap.
2. **Die Netzliste wird geströmt** statt als `String` je Eintrag gebaut. Das ist
   dieselbe Korrektur wie bei L161 und senkt die Spitze zusätzlich.

**Warum das nicht in der PWA umgangen wird** (Architektur-Checkliste Punkt 1): Der
Auslöser liegt zwar sichtbar in `loadSecondaryData()`, das den Scan bei jedem Wechsel
ins Netzwerk-Modul ruft. Aber ein Endpunkt, der bei Aufruf das Gerät abstürzen lässt,
ist ein ESP-Fehler. Ihn nicht mehr aufzurufen wäre die Umgehung eines
Stabilitätsproblems an der falschen Schicht.

### 3.2 Logring (L176)

64 Zeilen × 121 Byte = 7'744 Byte BSS, mehr als der gesamte freie Heap. 24 Zeilen
bringen rund 4'840 Byte zurück.

**Die Zeilenlänge bleibt bei 120.** Die Diagnosezeile ist 118 Zeichen lang, und die
Brücke kappt ohnehin bei 123 (L99). Dort ist nichts zu holen, ohne das
aussagekräftigste Protokoll zu beschneiden — L257 zeigt, wie knapp das ist: Eine
123 Zeichen lange Abbruchmeldung wäre genau am Rückweg abgeschnitten worden.

**Die sichtbare Folge gehört in die Einspielzeile:** Das Logfeld der PWA zeigt weniger
Rückschau. Der Mitschnitt über die serielle Leitung ist davon nicht betroffen.

### 3.3 Die zwei Beobachtbarkeitszeilen (C14 / L185)

`esp_heap_log()` und `- http write lost N` laufen heute am API-Ring vorbei und sind
nur mit Pi-Mitschnitt sichtbar. **Zweimal in zwei Tagen gebaut, beide Male
unerreichbar.** Sie zusätzlich durch `stm32_log_append()` zu schicken ist ein kleiner
Eingriff — und er ist der Grund, warum AKE1.4 ausdrücklich den Endpunkt als Instrument
nennt und nicht den seriellen Mitschnitt.

---

## 4. Runde E2 — Eingang der Brücke und Parametervertrag

Sechs Punkte, alle in `ESP8266/ESP-uclock/`, alle auf demselben Pfad: **dem Weg, über
den jede Einstellung der Uhr zum ESP kommt.**

| Punkt | Lösung | Anmerkung |
|---|---|---|
| **A37 / L237** | **Eine** Längenprüfung der Kommandozeile vor dem Zerlegen, nicht 45 Einzelprüfungen | Rund 45 Stellen schieben `parameters` unbedingt um 2. Der Fix aus L232 begrenzt den Schaden, hebt ihn aber nicht auf: Hinter dem Terminator steht nicht zwingend eine weitere Null |
| **C24 / L249** | `parse_json()` nimmt die Länge bis zur nächsten Zeichengrenze zurück | Rund fünf Zeilen. Das ist der **Auslöser** von A36, nicht die Lücke — ein zweites Netz, kein Ersatz |
| **C22 / L206** | Sieben Zeichenkettenfelder weisen zu lange Werte ab, statt zu kürzen | Darunter der **Update-Host**: Eine stille Kürzung dort zeigt auf einen anderen Server, und genau das war L124 |
| **C20 / L199** | `http_get_param()` gibt nie NULL zurück — alle `if (! value)`-Prüfungen sind tote Zweige. `fs_remove` und `dfplayer_play` melden Erfolg beim Nichtstun | Die Ursache ist **eine**, die Symptome sind viele. Repariert wird die Ursache |
| **C17 / L197** | `dfplayer_alarm_set`: `idx`, `from`/`to`, `hour`/`minute` prüfen | **Die Lösung ist abzuschreiben, nicht zu erfinden** — `http_api_timer_set_common()` hat alle drei bereits |
| **C18 / L186** | 19 der 34 Abweisungen auf `<param> out of range (<min>..<max>)` nachziehen | Zwei Grenzen sind Laufzeitwerte und brauchen `snprintf` in einen Stackpuffer, **kein `String`** — der Heap ist der Engpass dieser Laufzeit |

**Dies ist die Runde, die am ehesten zu lang wird.** Wenn sie es wird, fällt **C18**
zuerst heraus: Es ist die einzige, die nur die Form der Meldung betrifft und kein
Verhalten.

---

## 5. Runde S — die Brücke fertigbauen

### 5.1 A39 / L259 — kein verschachtelter 194er-Stoss mehr

**Der Befund steht unverändert im Code.** `src/main.c:2926` ruft im
`ESP8266_IPADDRESS`-Zweig direkt `var_send_all_variables()`. Der `SYNCVARS`-Zweig
daneben (`:2952-2975`) macht es bereits richtig und trägt die Begründung im Kommentar:
`schedule_esp8266_messages()` wird **auch aus der Warteschleife von `var_send_buf()`**
gerufen (`src/vars/vars.c:586`); von dort liefe der Vollabgleich verschachtelt, alle
rund 194 Kommandos gingen ohne eine einzige Quittung hinaus, und die zurückkommenden
Punkte quittierten der Reihe nach fremde Kommandos — der Schaden aus L102.

**Die offene Frage ist die Reihenfolge gegenüber dem Ticker, und sie ist echt.** Heute
gilt: erst Vollabgleich, dann Lauftext. Vormerken allein dreht das um, weil der Ticker
im Zweig gesetzt wird und der Abgleich erst später im Hauptloop läuft. Während des
Abgleichs ruft der Hauptloop `display_animation()` nicht — der Lauftext bliebe stehen
und liefe danach weiter.

| Variante | Was sie kostet | Was sie bringt |
|---|---|---|
| **(a)** nur `var_sync_pending = 1;` setzen, Ticker wie bisher sofort | eine Zeile, spart Flash | Der Lauftext **stockt** sichtbar, solange der Abgleich läuft |
| **(b) empfohlen** — zusätzlich `ip_ticker_pending` vormerken und den Ticker **nach** dem Abgleich im Hauptloop setzen | ein Flag und drei Zeilen | Die heutige Reihenfolge bleibt **exakt** erhalten; nur die Verschachtelung fällt weg |

**Empfehlung ist (b).** Sie ändert genau eine Eigenschaft — die Verschachtelung — und
keine zweite. Der lokale Puffer entfällt dabei, weil `esp8266.ipaddress` im Hauptloop
weiterhin gültig ist und der Text dort neu gebildet werden kann.

**Abnahme ist der Hauptloop-Zähler, nicht das Gefühl:** Referenz ist der in L226
gemessene Ruhewert von rund 1'482'480 Durchläufen je 10-Sekunden-Fenster. Dazu die
Beobachtung am Display — **dass der Lauftext vollständig durchläuft, nimmt nur das
Auge ab.**

### 5.2 L260 — ein Erfolgskriterium, das den ganzen Satz prüft

`var_sync_check()` im ESP prüft auf `HARDWARE_CONFIGURATION != 0xFFFF`, und dieses
Kommando ist das **dritte** von rund 194. Kommen 1 bis 3 an und der Rest nicht, hält
der ESP sich für geheilt. Die Lücke fällt dann auf A32 zurück: vier Nachsendeplätze
gegen 190 ausstehende Kommandos.

**Lösung: eine Abschlussmarke.** `var_send_all_variables()` sendet als letztes eine
eigene Zeile; `var_sync_check()` verlangt diese Marke zusätzlich zur Hardwarekennung,
und zwar **seit der letzten eigenen Anforderung**. Kosten: ein Kommando von dann 195.

**Die gute Seite der heutigen Lage bleibt erhalten** (L260): Weil die Hardwarekennung
ganz vorn im Stoss steht, darf der Vollabgleich ruhig zehn Sekunden brauchen. Die
Abschlussmarke verschiebt das Kriterium ans Ende — der ESP muss also **warten**
können, ohne vorschnell ein zweites Mal anzufordern. Die bestehende Taktung (bis zu
drei Versuche im Abstand von rund 10 s) bleibt unverändert; die Marke wird nur als
zweite Bedingung ergänzt, der Abstand nicht verkürzt.

**Rückfall ohne Fähigkeitsmeldung:** Ein STM, der die Marke nicht sendet, würde den
ESP sonst dreimal anfordern lassen. Deshalb ist die Marke an die bestehende
Fähigkeitsmeldung gebunden — siehe §5.5.

### 5.3 C26 / L253 — die Restlücke der Prüfsumme

**Gemessen:** 19,1 % der beschädigten Zeilen verlieren das Zeilenende samt Marke; sie
sehen unmarkiert aus und werden angenommen wie bisher. Vorher waren es 76,1 % — die
Prüfsumme hat die Lücke von drei Vierteln auf ein Fünftel gedrückt.

L253 nennt eine Markenpflicht **gefährlich**, mit der Begründung: Ein STM-Reset löscht
`cap_var_crc`, und der ESP würde danach dauerhaft alles ablehnen. **L254 entkräftet
das zur Hälfte:** Ein STM-Reset reisst den ESP nach L183 ohnehin mit (Einschaltstrom
der LED-Kette über `PB0`); der ESP startet also mit, der STM meldet `CAP` erneut, und
das Flag steht wieder, bevor der Vollabgleich läuft. Der gefährliche Fall ist damit
nicht ausgeschlossen, aber selten.

**Entscheidung dieser Spec — zwei Teile, bewusst getrennt:**

1. **Sichtbar machen, immer.** Der ESP zählt Zeilen, die **nach** der ersten gesehenen
   Marke dieser Sitzung **ohne** Marke eintreffen. Der Zähler ist über den Logring und
   die Zählerausgabe ablesbar. Das kostet nichts, bricht nichts, und es macht aus
   einer gerechneten Zahl (19,1 %) eine beobachtete.
2. **Markenpflicht nur für eine kurze, benannte Liste** — die Kommandoarten, bei denen
   ein still falscher Wert teuer ist: `HARDWARE_CONFIGURATION`, Update-Host,
   Update-Pfad. Für diese gilt: unmarkiert **und** Fähigkeit in dieser Sitzung
   bekannt ⇒ **nicht übernehmen und nicht quittieren**. Damit greift die Nachsendung
   aus A32 — die Zeile ist nicht verloren, sie kommt noch einmal.

**Warum das vertretbar ist:** Der gefürchtete Dauerzustand („ESP lehnt alles ab")
kann nicht eintreten, weil die Pflicht auf drei Kommandoarten beschränkt ist und an
die sitzungsgebundene Fähigkeit hängt. Der schlimmste Fall ist, dass eine dieser drei
Werte nach drei Nachsendeversuchen fehlt — **und das ist derselbe Zustand wie heute,
nur erkennbar statt still falsch.**

**Das ist die Spec-Entscheidung, die der Nutzer bestätigen muss.** Die Alternative
(nur Teil 1, kein Teil 2) ist vollständig risikofrei und schliesst die Lücke nicht.

### 5.4 A35 / L233 — Zuordnung der Quittung

`ESP-uclock.ino:531-533`: `var_set_parameter (parameter); Serial.println (".");` — die
Quittung folgt bedingungslos, und sie ist ein **nackter Punkt ohne Zuordnung**. Eine
verspätete Quittung bestätigt das **nächste** Kommando. **A32 hat dieses Problem
vergrössert**, weil Nachsendungen verspätete Quittungen häufiger machen.

**Teil 1 — Zuordnung (in diesem Paket):** Hat der STM die Fähigkeit angekündigt,
hängt der ESP an die Quittung zwei Hexzeichen, die sich aus der empfangenen Zeile
ergeben (die beiden letzten Zeichen ihrer Prüfsumme, die ohnehin berechnet wird). Der
STM vergleicht mit dem ausstehenden Kommando; passt es nicht, **verwirft er die
Quittung**. Das ausstehende Kommando läuft in seinen Timeout und wird von A32
nachgesendet.

**Rückfall:** Ohne angekündigte Fähigkeit sendet der ESP den nackten Punkt wie heute,
und der STM wertet wie heute. Beide Richtungen sind damit verträglich.

**Teil 2 — Übernahme statt Empfang (nicht in diesem Paket):** Die Quittung soll
bestätigen, dass der Wert übernommen wurde. Das verlangt einen Rückgabewert aus
`var_set_parameter()`, also einen Eingriff in jeden Zweig seines `switch` — und der
hat heute keinen `default`-Zweig, was die Umstellung zusätzlich verbreitert. **Teil 1
löst das dringende Problem** (die verspätete Quittung, die A32 gerade häufiger gemacht
hat); Teil 2 löst ein zweites, das nicht schlimmer geworden ist. Er bleibt benannt und
ist der erste Kandidat der nächsten Brückenrunde.

### 5.5 Die Fähigkeitsmeldung entschärft die Reihenfolge — und das ist nachgesehen, nicht angenommen

Der Mechanismus existiert und ist am Gerät belegt (L254): `CAP` setzt den Merker, die
`FIRMWARE`-Zeile schreibt ihn fest, und eine `FIRMWARE`-Zeile **ohne** vorangegangenes
`CAP` löscht ihn. Dadurch gilt die Fähigkeit **je ESP-Sitzung**, und ein Rückrollen
einer Seite kann nicht zu einem halben Protokoll führen.

Für die drei Neuerungen dieser Runde ergibt sich damit:

| Lage | Verhalten |
|---|---|
| ESP neu, STM alt | Der STM kündigt die neue Fähigkeit nicht an ⇒ der ESP verlangt keine Abschlussmarke, quittiert mit nacktem Punkt, erzwingt keine Marke. **Verhalten wie heute** |
| ESP alt, STM neu | Der ESP ignoriert die Abschlussmarke (unbekannter Kommandobuchstabe, `switch` ohne `default` ⇒ stillschweigend verworfen) und quittiert sie wie jede Zeile. Der STM wertet die Quittung mangels angehängter Zeichen wie heute. **Verhalten wie heute** |
| beide neu | volle Wirkung |

**Folge für die Rundeneinteilung:** Zwischen dem ESP- und dem STM-Teil dieser Runde
braucht es **keine eigene Beobachtungsrunde** — die Reihenfolge ist durch die
Fähigkeitsmeldung entschärft. Zwei Einspielschritte bleiben trotzdem, weil jede Runde
**eine** Laufzeit aufspielt (Risiko 1 aus dem alten Paket, C13/L180); dazwischen läuft
nur der Smoketest, kein eigener Gerätelauf.

**Was die Fähigkeitsmeldung nicht entschärft, bleibt benannt:** Geht die
`FIRMWARE`-Zeile einer Rückroll-Sitzung verloren, bleibt das Flag stehen. Das braucht
Verlust **und** Rückrollen gleichzeitig (L251).

### 5.6 A16 / L106 — Tetris und Snake bedienen den Watchdog nicht

Beide Module haben **null** `watchdog_reload()`. Nach rund 3,6 Steinen ist das
20-s-Fenster um, der Reset ist sicher. Das liegt in dieser Runde, weil der STM ohnehin
geflasht wird — zwei Flashvorgänge für eine Zeile wären unverhältnismässig.

**Die Abnahme braucht den Nutzer.** Ein Spiel startet man am Gerät, und
`/api/test_display` sowie `/api/learn_ir` stehen aus gutem Grund in der Gefahrenliste.
Ohne diesen Lauf gilt AKS.7 als **nicht erfüllt** — nicht als „vermutlich in Ordnung".

---

## 6. Runden P und U — PWA und UI

**B33 / L246** — `fs_show` bietet „Anzeigen" für alle 16 Dateien an, auch für die 13
`.gz`-Assets; ein Klick schreibt 123'127 Zeichen Binärdaten ins DOM und meldet Erfolg.
Die Unterscheidung ist billig: Die Endung ist bekannt, und `fs_list` liefert die
Grösse mit. **Rein in der PWA zu lösen**, kein ESP-Anteil, keine Reihenfolgefrage.

**B24 / L208** — `getDisplayModeName()` führt fünf fest verdrahtete Namen, die mit den
Layout-Modi nichts zu tun haben; die Schwesterfunktion daneben macht es richtig.
Übersicht und Auswahlfeld widersprechen sich dadurch.

**B30 / L240** — `styles.css` zieht über `input { min-height: 44px; padding: 0 10px; }`
**jedes** Ankreuzfeld auf 13 × 44 px. Zurückgesetzt wird das nur für
`.chip-toggle input[type="checkbox"]`. Ohne Reset bleiben die drei TFT-Felder (ganz
ohne Klasse) und die Favoritenfelder je Animationsprofil. Lösung: den Reset auf einen
gemeinsamen Selektor heben.

**B28 / L227** — Die grösste Höhendifferenz benachbarter Wartungskacheln liegt bei
406 px (1512 px Breite) und **594 px bei 600 px Breite**; gefordert sind ≤ 400. Das
Paar ist `backup` (650 px) neben `service` (244 px), und die Ursache ist **reiner
Inhaltsunterschied**: Die Sicherungskachel trägt Infoliste, Exportknopf, Dateifeld,
Importknopf und Hinweiszeile, die Servicekachel zwei Knöpfe. Die beiden Wege über das
Raster sind bei L219 und L227 mit Begründung verworfen. **Zu lösen ist es über den
Inhalt** — die Infoliste gehört verdichtet, nicht die Kachel gespannt.

**Reihenfolge P vor U** ist nicht zwingend, aber beide schreiben in die PWA, und R3
erlaubt genau einen Schreiber je Datei. P berührt die Hauptdatei, U berührt Markup und
Stilvorlage — **disjunkt**, also theoretisch parallel. **Trotzdem nacheinander**, weil
beide dasselbe Fabrikat hochladen und `make app-gz` ein stilles Arbeitsverzeichnis
braucht (R2). Zwei Uploads nacheinander sind billiger als eine halb geschriebene
`.gz`, die zum Weisschirm führt.

---

## 7. Runde F — Flash-Überwachung (Runde 4b aus dem alten Paket)

Unverändert übernommen aus `specs/grosses-paket-2026-10-04/design.md` §4b, weil der
Entwurf dort vollständig ist und nie umgesetzt wurde:

1. **Der Legacy-Pfad wendet denselben Filter an wie die API.** `http.cpp:6909-6913`
   übernimmt den Namen heute ungeprüft per `strncpy`. Es ist **dieselbe Funktion**
   `http_remote_stm32_filename_matches()` zu rufen, nicht eine zweite Prüfung zu
   schreiben — zwei Prüfungen laufen auseinander, und genau das ist der Befund.
2. **Bei unbekannter Hardware bietet die Auswahlliste keine Datei an.** Heute zeigt
   sie bei `HARDWARE_CONFIGURATION` = 65535 **alle**, die F103 direkt neben der F411.

**Der Rückweg muss offen bleiben.** Bei leerem `HARDWARE_CONFIGURATION` weist der
geschützte Endpunkt **jeden** Namen ab, auch den richtigen. Wer beide Wege dicht macht,
nimmt sich die Reparatur. Die Reparatur läuft über `./tools/flash-stm.sh` (DIR-010),
das den STM zuerst zurücksetzt und damit die Hardwarekennung zurückholt — **und die
Hinweismeldung muss das nennen.**

**Neu gegenüber dem alten Entwurf, aus L257:** Die Länge der Hinweismeldung ist
mitzuprüfen. Eine 123 Zeichen lange Meldung wäre genau am Rückweg abgeschnitten
worden; der Leser hätte erfahren, dass etwas kaputt ist, aber nicht, was er tun kann.

**Warum diese Runde zuletzt steht:** Ihre Abnahme ist ein STM-Flash. Nach Runde S ist
der STM ohnehin frisch geflasht, und ein Flash, dessen Ergebnis man bereits kennt, ist
die ehrlichste Probe für einen veränderten Flashpfad.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Runde |
|---|---|---|---|
| `BEFUNDE.md` | Nachzählvermerke, Arbeitsliste bereinigen, Standabschnitt auflösen | `doc-writer` | V |
| `knowledge/directives.md`, `knowledge/quick-reference.md` | E18 (Führung des Direktivenbestands), B21 (`grep -a`), L256 (Prüfstand ≠ Zielplattform) | `doc-writer` | W |
| `tools/preview/diag.js`, `tools/preview/shot.sh`, `tools/check-pwa.mjs` | Messmodul: Geometrie, Ankreuzfelder, Bildpunktfarbe, Scrollen, Vollseite | Lead | W |
| `tools/hooks/file-ownership.py`, `tools/hooks/no-danger.py` | Schreibpositionen bzw. Gerätebezug | Lead | W |
| `tools/checks/` | Heimat der Prüfstände; neue Stufe S11 | Lead | W |
| `tools/guardrails.sh` | S11 einhängen | Lead | W |
| `.claude/agents/*.md` | Der Besitz-Hook ist eine Erinnerung, kein Zwang (B22) | Lead | W |
| `ESP8266/ESP-uclock/http.cpp` | `network_scan`, Verlustzeile in den Ring | `esp-developer` | E1 |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Ringgrösse, Heap-Zeile in den Ring | `esp-developer` | E1 |
| `ESP8266/ESP-uclock/vars.cpp` | Längenprüfung der Kommandozeile (A37) | `esp-developer` | E2 |
| `ESP8266/ESP-uclock/weather.cpp` | `parse_json()` an die Zeichengrenze (C24) | `esp-developer` | E2 |
| `ESP8266/ESP-uclock/http.cpp` | C22, C20, C17, C18 | `esp-developer` | E2 |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Abschlussmarke prüfen, Quittung mit Zuordnung, Markenpflicht, Zähler | `esp-developer` | S |
| `src/main.c` | A39 (vormerken, Ticker danach) | `stm-developer` | S |
| `src/vars/vars.c` | Abschlussmarke senden, Fähigkeit ankündigen, Quittung auswerten | `stm-developer` | S |
| `src/esp8266/esp8266.c` | Fähigkeitszeile ergänzen | `stm-developer` | S |
| `src/tetris/tetris.c`, `src/tetris/snake.c` | `watchdog_reload()` (A16) | `stm-developer` | S |
| PWA-Hauptdatei | B33, B24 | `pwa-developer` | P |
| Markup, Stilvorlage | B30, B28 | `ui-developer` | U |
| `ESP8266/ESP-uclock/http.cpp` | Legacy-Flashfilter, leere Auswahlliste | `esp-developer` | F |
| `tools/smoke-device.sh` | Stufe für `HARDWARE_CONFIGURATION` | Lead | F |
| Die vier Versionsdateien | Anheben je Runde | `release-engineer` | je Runde |

Die Zuordnung folgt der Besitztabelle in `CLAUDE.md` (R3). **Jede Datei hat genau
einen Schreiber, den Lead eingeschlossen.**

**Kodierung — vor jedem Patch prüfen, nicht annehmen.** `http.cpp` und
`stm32flash.cpp` sind **UTF-8**, die übrigen ESP-Quellen überwiegend ASCII oder
ISO-8859-1. Ein Patcher, der pauschal `latin-1` annimmt, beschädigt die beiden —
derselbe Schaden wie in der Gegenrichtung, nur umgekehrt. In **neuem** Text gilt
durchgehend die Umschrift (`Geraet`, `waehrend`).

---

## Prüfung gegen die Architektur-Checkliste

Siehe `knowledge/architecture-checklist.md`. Beantwortet, nicht abgehakt.

### Proper architecture

**Bleibt die PWA parallel zur Legacy-Oberfläche?** Ja, und Runde F stärkt die Legacy:
Der Legacy-Flashpfad bekommt denselben Schutz wie der API-Pfad, statt dass einer der
beiden Wege abgeschaltet würde. Legacy bleibt die Stabilitäts-Referenz beim Debuggen —
also genau der Weg, den man geht, wenn man der PWA nicht traut, und deshalb darf er
nicht der gefährlichere sein.

**Werden die Wetter-Endpunkte eingehalten?** Ja. Runde E2 fasst `weather.cpp` an, aber
nur in `parse_json()` — der Zerlegung der Antwort. Die Endpunkte
`/api/weather_get_now` und `/api/weather_get_forecast` bleiben unberührt, kein Rückbau
auf den entfernten Legacy-Bypass.

**Bleibt die Restore-Bedingung um `pending_weather_ticker_restore` vollständig?**
**Hier ist genau hinzusehen, weil Runde S sie berührt.** A39 setzt den IP-Lauftext neu
— heute wird im `IPADDRESS`-Zweig direkt danach `pending_weather_ticker_restore = 1`
gesetzt (`src/main.c:2949`). Verschiebt Variante (b) das Setzen des Tickers in den
Hauptloop, **muss das Flag mitwandern**: Es gehört zu dem Ticker, den es
zurücknehmen soll, nicht zu dem Zweig, in dem es heute zufällig steht. Keine der vier
Teilbedingungen des Restores wird vereinfacht; die Umsetzung ändert ausschliesslich
den **Ort** des Setzens, und der `stm-developer` weist das im Bericht nach.
E17 (Umbenennung des Flags) ist **ausdrücklich nicht** Teil dieser Änderung — ein
Umbenennen mitten in einem Eingriff an derselben Stelle ist stilles Mitkorrigieren.

**Werden App-Assets ausschliesslich als `.gz` ausgeliefert?** Ja, unverändert. Runde P
ändert, was die PWA mit `fs_show` **anzeigt**, nicht wie sie ausgeliefert wird. Die
Existenzprüfung mit Dateigrösse > 0 bleibt.

**Greift die Änderung an der richtigen Schicht an?** Der wichtigste Punkt dieses
Pakets. Dreimal wäre die bequemere Schicht die falsche gewesen:

- **L174** — Den Scan in der PWA nicht mehr zu rufen wäre eine Umgehung; der Endpunkt
  stürzt ab, und das ist ein ESP-Fehler.
- **L241** — Die PWA könnte „Datei leer" von „Datei fehlt" nicht selbst
  unterscheiden; nur der ESP kann das liefern.
- **A39** — Der Lauftext ist nicht das Problem; die Verschachtelung ist es.

### Scalable systems

**Wie viele STM-Kommandos erzeugt die Aktion?** Der Vollabgleich bleibt bei rund 194
und wächst um **eines** (die Abschlussmarke). A39 ändert die Zahl nicht, sondern den
**Zeitpunkt**: Die 194 Kommandos laufen künftig nie mehr verschachtelt, sondern
sequenziell mit Quittung — das ist der ganze Zweck. C26 Teil 2 kann einzelne Kommandos
nachsenden lassen; die Obergrenze aus A32 (vier Plätze, begrenzte Versuchszahl) gilt
unverändert und ist die Bremse.

**Wie viele Byte pro Minute zusätzlich auf der UART?** Die Abschlussmarke: einmal je
Vollabgleich, also je ESP-Neustart, rund 10 Byte. Die Quittungszuordnung: **zwei Byte
je Kommando** in der Richtung ESP→STM — bei 194 Kommandos eines Vollabgleichs rund
390 Byte einmalig, im Normalbetrieb wenige Byte je Minute. Der Erfolgs- und der
Zählerausdruck laufen über `stm32_log_append()` und kosten auf der Brücke **0 Byte**
(L261). Dem gegenüber stehen die Einsparungen aus C2, die dieses Paket **nicht**
anfasst — die Last sinkt hier also nicht, sie steigt minimal und messbar.

**Bleibt jeder ausgelöste Pfad unter 20 s?** Der kritische Pfad ist
`var_send_all_variables()` mit rund 195 Kommandos à bis zu 3 s Quittungswartezeit.
Er läuft nach A39 **im Hauptloop**, wo der `watchdog_reload()` am Schleifenkopf liegt,
und `var_send_buf()` lädt den Watchdog nach eingetroffener Quittung bereits selbst
nach. **Den gültigen Bestand der Aufrufstellen nennt Guardrail S7, nicht dieses
Dokument** — die Zahl hier aufzuschreiben wäre genau die Kopie, die still veraltet.
A16 fügt zwei Pfade hinzu, die heute **garantiert** über 20 s laufen.

**Wie oft pollt die PWA?** Unverändert. Runde E1 senkt die Last pro Abruf (keine
`String`-Kette je Netz), nicht die Frequenz.

**Wie lange blockiert der Hauptloop?** Die messbare Zusage steht in AKS.2: Der
Hauptloop-Zähler darf zwischen zwei Diagnosezeilen nicht einbrechen, Referenz rund
1'482'480 Durchläufe je 10-Sekunden-Fenster (L226). Der Logring im ESP schrumpft um
4'840 Byte und hat mit dem STM-Hauptloop nichts zu tun.

**Hartkodierte Grenzen?** Die Abschlussmarke und die Markenpflichtliste sind
Konstanten im Quelltext. Die Liste umfasst drei Kommandoarten und wird im Code
benannt, nicht gemustert — ein zu weites Muster verschluckt irgendwann eine vierte,
und das wäre genau der Fehler, gegen den die Begrenzung gebaut ist (dieselbe
Überlegung wie bei `diff-snapshot.sh --soll`, L247).

### Secure by design

**`escapeHtml` bei jedem `innerHTML`?** Runde P berührt die Anzeige von Dateiinhalten
— **und zwar genau, um sie zu unterbinden.** B33 verhindert, dass 123'000 Zeichen
Binärdaten ins DOM gelangen. Die verbleibende Anzeige echter Textdateien geht über
`textContent`, nicht über `innerHTML`.

**Fremddaten als nicht vertrauenswürdig?** Das ist der Kern von Runde E2: `parse_json()`
verarbeitet die Beschreibung einer **fremden Website**, und `convert_utf8_to_iso8859()`
lief dort über die Pufferkante (A36). C24 nimmt die Länge auf die Zeichengrenze
zurück. Die Inhalte vom Update-Server bleiben unvertrauenswürdig behandelt; der Host
ist konfigurierbar, die Verbindung ist HTTP — **und C22 schützt genau diesen Host
davor, still gekürzt auf einen anderen Server zu zeigen** (L124).

**Keine Credentials in Logausgaben?** Dieses Paket fügt keine neue Ausgabe mit
Nutzdaten hinzu. Die Quittungszuordnung überträgt zwei Hexzeichen einer Prüfsumme,
keinen Inhalt. Der Zähler aus C26 Teil 1 zählt, er protokolliert keine Werte.
**A20 (Geheimnisse in der Timeout-Meldung, L115) bleibt offen und ist nicht Teil
dieses Pakets** — das ist hier festzuhalten, weil Runde S genau an diesen Meldungen
vorbeiarbeitet und die Versuchung bestünde, es nebenbei zu erledigen.

**Neue schreibende Endpunkte?** Keine.

### Stable & reliable

**Wird jeder Fehler ausgewertet?** Das ist der rote Faden von Runde E2: `C20` behebt,
dass `http_get_param()` nie NULL zurückgibt und damit **jede** `if (! value)`-Prüfung
ein toter Zweig ist — eine Erfolgsmeldung, die nicht vom Ergebnis abhängt, ist genau
der Fall aus der Checkliste. C17 behebt, dass ein Alarm als aktiv in der Liste steht
und nie auslösen kann.

**Leere `catch`-Blöcke?** Keine neuen. Der `code-reviewer` prüft es je Runde.

**Stille Verwerfungen ohne Zähler?** C26 Teil 1 führt genau den fehlenden Zähler ein:
unmarkierte Zeilen nach der ersten Marke einer Sitzung. Die vier Verlustzähler aus A32
bleiben. **Was bewusst still bleibt, ist der Normalbetrieb der Prüfsumme** — der STM
hängt sie direkt auf die Leitung, nicht über `log_printf`, damit keine Last auf
derjenigen Leitung entsteht, deren Überlastung der Fehler ist (L109). Beleg bleibt
deshalb indirekt: null Abweisungen, null Nachsendungen.

**Ist der Zustand nach einem Abbruch mitten in einer Sequenz definiert?** Zwei Stellen:

- **Vollabgleich.** Das Flag `var_sync_pending` wird **vor** dem Senden gelöscht —
  trifft währenddessen ein weiteres `SYNCVARS` ein, bekommt es seinen eigenen
  Durchlauf. Zwei Vollabgleiche nacheinander sind harmlos, zwei ineinander nicht. Das
  steht so bereits im Code und bleibt.
- **Abschlussmarke.** Bricht der Abgleich in der Mitte ab, kommt die Marke nicht, und
  der ESP fordert erneut an — **das ist die gewollte Wirkung**, und es ist der
  Unterschied zu heute, wo er sich beim dritten Kommando für geheilt hält.

**Wird ein gesetztes Flag auf jedem Pfad wieder aufgelöst?** Die beiden neuen Flags
(`ip_ticker_pending`, der Marken-Erwartungszustand im ESP) werden im selben Hauptloop
gelöscht, in dem sie gewertet werden. Für `pending_weather_ticker_restore` gilt die
Zusage aus „Proper architecture": Das Flag wandert mit dem Ticker, zu dem es gehört.
Der `firmware-analyst` prüft beide Flags in der Review von Runde S ausdrücklich gegen
diese Frage.

---

## Verworfene Alternativen

**Runde V als Doku-Aufgabe an den `doc-writer`.** Verworfen: Was zu prüfen ist, steht
nicht im Dokument. Der `doc-writer` trägt ein, er stellt nicht fest. Hätte man L173 so
behandelt, stünde dort heute weiterhin „offen, KRITISCH".

**Die ganze Arbeitsliste nachzählen.** Verworfen: über hundert Einträge, und bei den
meisten hat sich seit dem Beleg nichts geändert. Der mechanische Filter (jüngster
Commit auf die genannten Dateien gegen das Belegdatum) trifft genau die Fälle, in
denen beiläufiges Mitreparieren überhaupt möglich war.

**S11 als Altersprüfung („KRITISCH seit mehr als 30 Tagen").** Verworfen: Das misst
Zeit statt Ereignis und erzeugt Rauschen bei jedem Befund, der zu Recht lange offen
ist. Eine Prüfung mit Fehlalarmen liest niemand mehr — dieselbe Begründung wie bei der
Umlautprüfung in S8.

**A39 Variante (a)** — nur vormerken, Ticker wie bisher sofort. Verworfen zugunsten
von (b), weil sie zwei Dinge auf einmal ändert: die Verschachtelung **und** die
Reihenfolge. Eine Änderung, die zwei Wirkungen hat, ist bei einem Fehlschlag nicht
eindeutig zuzuordnen.

**Markenpflicht für alle Kommandoarten (C26).** Verworfen aus dem Grund, den L253
nennt: Eine unmarkierte Zeile muss durchgehen, sonst bricht jeder Betrieb, in dem der
STM die Fähigkeit nicht kennt. Die Begrenzung auf drei Kommandoarten hält den Nutzen
und lässt das Risiko weg.

**Folgenummer in der Quittung (A35).** Verworfen zugunsten der zwei Hexzeichen aus der
ohnehin berechneten Prüfsumme: Eine Folgenummer braucht einen Zähler auf beiden
Seiten, der nach einem Neustart einer Seite auseinanderläuft — ein zweiter
Synchronisationszustand, den niemand heilt. Die Prüfsumme ist zustandslos.

**Teil 2 von A35 (Übernahme statt Empfang) mitnehmen.** Verworfen für dieses Paket:
Rückgabewert aus `var_set_parameter()` heisst Eingriff in jeden Zweig eines `switch`
ohne `default`. Das ist eine eigene Runde wert, und das dringende Problem löst Teil 1.

**L174 in der PWA umgehen** (den Scan nicht mehr bei jedem Modulwechsel rufen).
Verworfen: falsche Schicht. Als **Zwischenlösung bis zum Einspielen** bleibt sie
richtig und steht bereits in der Einspielzeile — den Netzwerk-Reiter meiden.

**Runde 4b (F) früher fahren.** Verworfen: Ihre Abnahme ist ein STM-Flash, und nach
Runde S ist ohnehin einer fällig. Ein zusätzlicher Flash nur zur Abnahme eines
Flashpfades wäre Aufwand ohne Erkenntnis.

**P und U parallel.** Verworfen trotz disjunkter Dateien: `make app-gz` braucht ein
stilles Arbeitsverzeichnis (R2), und eine `.gz` einer halb geschriebenen Datei ist der
belegte Weisschirm-Fall.

---

## Versionsfolgen

Je Runde, **nur die geänderte Komponente** (DIR-004, kein Gleichschritt):

| Runde | STM `src/main.h` | ESP `version.h` | App `APP_VERSION` | `CACHE_NAME` |
|---|---|---|---|---|
| V | — | — | — | — |
| W | — | — | — | — |
| E1 | — | ☐ | — | — |
| E2 | — | ☐ | — | — |
| S | ☐ | ☐ | — | — |
| P | — | — | ☐ | ☐ |
| U | — | — | ☐ | ☐ |
| F | — | ☐ | — | — |

`APP_VERSION` und `CACHE_NAME` gehören **immer zusammen**. Nur der
`release-engineer` führt das aus (R4); Teammates bumpen nichts.

**Runde S hebt zwei Versionen in einer Runde** — das ist zulässig, weil zwei
Einspielschritte stattfinden (erst ESP, dann STM) und jeder seine eigene Version
trägt. Der Tag `release/<stm>-<esp>-<app>` wird je Einspielschritt gesetzt (DIR-011).
