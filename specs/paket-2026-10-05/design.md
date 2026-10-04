# Design — Paket 2026-10-05

**Erstellt:** 2026-10-04. Entscheidungen des Nutzers und der A35-Nachweis (L266)
eingearbeitet am 04.10.2026. Momentaufnahme (DIR-006), wird nicht fortgeschrieben.

**Rundenfolge:** V → W → E1 → E2 → **H** → S → P → U → F.

---

## 1. Warum Runde V zuerst kommt

Eine Spec, die Umfang aus einer Arbeitsliste nimmt, ist nur so gut wie deren Stand.
Beim Schreiben dieser Spec haben drei Einträge der Liste nicht mehr gestimmt — A32,
A38 und A36 —, und einer davon hätte eine eigene Runde erzeugt. **Alle drei sind
inzwischen nachgezählt, geschlossen und gestrichen.** Das ist keine Doku-Hygiene,
sondern Planungsgrundlage, und es ist der Beleg, dass der Durchgang trägt, noch bevor
er als Runde gelaufen ist.

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

**Ein Verweis auf einen anderen Befund ist kein Beleg.** Genau diese Kette hatte bei
A36 zu zwei widersprechenden Zeilen im selben Dokument geführt: Die Arbeitsliste
nannte ihn „höchste Priorität", L249 nannte ihn behoben. Wer `geschlossen` schreibt,
zitiert den Code.

### 1.2 Reihenfolge — die schwersten zuerst

**Welle 1 — alles, was heute den Status KRITISCH trägt.** Ihr Mitschleppen ist am
teuersten, weil sie Runden blockieren und Prioritäten verschieben. A32/L230,
A38/L255 und A36/L236 standen hier ursprünglich an erster, zweiter und dritter
Stelle; sie sind erledigt und aus der Liste genommen.

| Nr. | Befund | Was zu prüfen ist | Wo es entschieden wird |
|---|---|---|---|
| 1 | **L232 / A33** | Prüft `htoi()` heute `buf[i]` statt `*buf`? **Achtung:** Die STM-Seite derselben Gattung ist als L263 behoben worden, die Restgattung ist L265/A41 — das ist Runde H, nicht Nachzählstoff | `ESP8266/ESP-uclock/base.cpp:42` |
| 2 | **L42 / L103 / A6** | Vollabgleich nach ESP-Neustart — durch Weg B erledigt? | `src/main.c:2974`, `:3655`, L261. **Achtung L260:** Das Erfolgskriterium prüft nur das dritte von 194 Kommandos; A6 ist damit **verengt, nicht geschlossen** |
| 3 | **L205 / A30** | `overlay[0].type` noch falsch? | Gerätemessung nach einem STM-Reset; L232 nennt 164 geprüfte Felder, alle im Bereich — **ein Datenpunkt, kein Beleg** |
| 4 | **L174** | `network_scan` scannt weiterhin mitten in der Antwort? | `http_api_network_scan()` |
| 5 | **L178** | Schreibverluste — der Befund widerlegt sich im eigenen Text bereits zweimal | Zählerstand über eine Stunde Betrieb, Vergleich mit „drei Blöcke in 32 Minuten, beide durch Browserläufe" |
| 6 | **L179** | Legacy-Flashpfad ungeprüft? | `http.cpp:6909` |
| 7 | **L180 / C13** | Exception 9 beim OTA — tritt sie noch auf? | Jedes OTA seit dem 04.10.2026 ist ein Datenpunkt. **Ein gelungenes OTA ist `geschlossen` nur, wenn der vermutete Verursacher im Code nachweislich weg ist** — sonst `bestaetigt` |
| 8 | **L125 / L140 / C9 / C9a** | Absturz beim Ausliefern von `app.js` und bei `update_status` | Mitschnitt seit dem letzten Flash plus Paketzahl je Abruf |
| 9 | **L175 / C9c2** | Heap-Fragmentierung — L262 sagt „ist zurück", L254 mass 72 Byte Differenz. Zwei Messungen, zwei Bedingungen | Messreihe unter **einer** Bedingung; das Ergebnis entscheidet, wie dringend das eigene Heap-Paket ist |
| 10 | **A22 / A24 / A26** | **Gewichtsprüfung statt Nachzählung:** Trägt die Indexprüfung im Tabellentransfer nach A32 noch dasselbe Gewicht, wenn eine beschädigte Zeile schon an der Prüfsumme scheitert? | Analyse, Ergebnis ist eine Einstufung, keine Umsetzung |

**Welle 2 — der Abschnitt „Stand des grossen Pakets" in `BEFUNDE.md`.** Er ist die
Stelle, aus der diese Planung ihren Umfang genommen hätte, und er ist nachweislich
veraltet: Er führt Runde 4a als „noch nicht begonnen, blockiert durch A32", während
beides am Gerät steht. Nachzuzählen sind die dort genannten Tasks 1.3, 1.5, 0.11,
2.11, 3.4, 3.5, 3.6, 3.10 sowie die Runden 4a und 4b.

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

Dieses Paket erzeugt zwei davon: die Schutzseite für A37 (Runde E2) und die für A41
(Runde H). Sie gehören **vor** ihrer ersten Verwendung angelegt, deshalb steht die
Heimat in Runde W.

---

## 3. Runde E1 — der Absturz und die Beobachtbarkeit

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

**Was hier ausdrücklich nicht geschieht:** Die 36 übrigen `sanitize_*_string()`-Stellen
bleiben unberührt (L175). Das ist der Wurzelbefund der Fragmentierung und ein eigenes
Paket; ihn in dieselbe Runde wie den Absturzfix zu legen, hiesse zwei Eingriffe an
derselben Datei in einem Flash — und einen Fehlschlag danach nicht zuordnen zu können.

**Warum das nicht in der PWA umgangen wird** (Architektur-Checkliste Punkt 1): Der
Auslöser liegt zwar sichtbar in `loadSecondaryData()`, das den Scan bei jedem Wechsel
ins Netzwerk-Modul ruft. Aber ein Endpunkt, der bei Aufruf das Gerät abstürzen lässt,
ist ein ESP-Fehler. Ihn nicht mehr aufzurufen wäre die Umgehung eines
Stabilitätsproblems an der falschen Schicht.

### 3.2 Die zwei Beobachtbarkeitszeilen (C14 / L185)

`esp_heap_log()` und `- http write lost N` laufen heute am API-Ring vorbei und sind
nur mit Pi-Mitschnitt sichtbar. **Zweimal in zwei Tagen gebaut, beide Male
unerreichbar.** Sie zusätzlich durch `stm32_log_append()` zu schicken ist ein kleiner
Eingriff — und er ist der Grund, warum AKE1.3 ausdrücklich den Endpunkt als Instrument
nennt und nicht den seriellen Mitschnitt.

**Der Ertrag ist bereits belegt, bevor die Runde läuft:** Die erste Heap-Messung ohne
Zusatzgerät (L175, Nachtrag) war nur möglich, weil zwei Heap-Zeilen im Ring standen.
Genau das ist der Grund, warum der Ring **nicht** verkleinert wird — siehe §3.3.

### 3.3 Warum der Logring bleibt, wie er ist

Der ursprüngliche Entwurf dieser Spec sah vor, den Ring zu verkleinern (L176, Vorschlag
des Nutzers). **Der Nutzer hat das am 04.10.2026 zurückgezogen, und die Begründung ist
stärker als der Vorschlag:**

- **L262, am selben Tag gemessen:** Der Ring deckt rund **zwei Minuten** ab. Gefüllt
  wird er nicht von den HTTP-Zeilen — davon enthält er **null**, sie gehen nur auf die
  serielle Leitung —, sondern von der eigenen Geschwätzigkeit des STM:
  `main: call display_clock`/`returned` als Paar, dazu die beiden Temperaturzeilen,
  dreimal je Minute.
- **Für einen Hänger, der 90 Sekunden dauert, ist das knapp.** Weniger Rückschau
  kostet genau die Diagnosefähigkeit, die zuletzt mehrfach den Ausschlag gab.
- **Die Heap-Frage wird dadurch nicht gelöst, nur verschoben.** Der Hebel sind die
  36 `sanitize_*_string()`-Aufrufe (L175), nicht der Ring. Das gehört in ein eigenes
  Paket.

Der Punkt ist in `requirements.md` unter „Nicht Teil dieser Änderung" mit Begründung
festgehalten, damit der Vorschlag nicht in einem halben Jahr unbesehen wiederkommt.

---

## 4. Runde E2 — Eingang der Brücke und Parametervertrag

Sechs Punkte, alle in `ESP8266/ESP-uclock/`, alle auf demselben Pfad: **dem Weg, über
den jede Einstellung der Uhr zum ESP kommt.**

| Punkt | Lösung | Anmerkung |
|---|---|---|
| **A37 / L237** | **Eine** Längenprüfung der Kommandozeile vor dem Zerlegen, nicht 45 Einzelprüfungen | Rund 45 Stellen schieben `parameters` unbedingt um 2. Der Fix aus L232 begrenzt den Schaden, hebt ihn aber nicht auf: Hinter dem Terminator steht nicht zwingend eine weitere Null |
| **C24 / L249** | `parse_json()` nimmt die Länge bis zur nächsten Zeichengrenze zurück | Rund fünf Zeilen. **A36 ist geschlossen**, also ist das hier kein Notnagel mehr, sondern das zweite Netz: Es verhindert, dass ein halbes Mehrbyte-Zeichen überhaupt entsteht |
| **C22 / L206** | Sieben Zeichenkettenfelder weisen zu lange Werte ab, statt zu kürzen | Darunter der **Update-Host**: Eine stille Kürzung dort zeigt auf einen anderen Server, und genau das war L124 |
| **C20 / L199** | `http_get_param()` gibt nie NULL zurück — alle `if (! value)`-Prüfungen sind tote Zweige. `fs_remove` und `dfplayer_play` melden Erfolg beim Nichtstun | Die Ursache ist **eine**, die Symptome sind viele. Repariert wird die Ursache |
| **C17 / L197** | `dfplayer_alarm_set`: `idx`, `from`/`to`, `hour`/`minute` prüfen | **Die Lösung ist abzuschreiben, nicht zu erfinden** — `http_api_timer_set_common()` hat alle drei bereits |
| **C18 / L186** | 19 der 34 Abweisungen auf `<param> out of range (<min>..<max>)` nachziehen | Zwei Grenzen sind Laufzeitwerte und brauchen `snprintf` in einen Stackpuffer, **kein `String`** — der Heap ist der Engpass dieser Laufzeit |

**Dies ist die Runde, die am ehesten zu lang wird.** Wenn sie es wird, fällt **C18**
zuerst heraus: Es ist die einzige, die nur die Form der Meldung betrifft und kein
Verhalten.

---

## 5. Runde H — den STM-Eingang härten (A41 / L265)

### 5.0 Der Befund, und warum er in dieses Paket gehört

Auf jedes `htoi (x, n)` folgt ein **unbedingtes** `x += n`:

| Datei | Stellen |
|---|---|
| `src/main.c` | 39 |
| `src/tables/tables.c` | 22 |
| `src/esp-spiffs/esp-spiffs.c` | 4 |
| `src/tftled/tftled.c` | 1 |
| **Summe** | **rund 66** |

Stand der Zeiger schon auf dem Terminator, zeigt er danach dahinter. **Am
exponiertesten sind `tftled.c:122` und `esp-spiffs.c:206-231`:** Dort läuft direkt
hinter dem Vorrücken eine `while (*p)`-Schleife, die dann über die Pufferkante liest.

Die Korrektur an `htoi()` (L263, STM-Gegenstück zu L232) begrenzt den Schaden auch
hier, **hebt ihn aber nicht auf** — hinter dem Terminator steht nicht zwingend eine
weitere Null.

**Die Lösung ist dieselbe wie bei A37: eine Längenprüfung der Kommandozeile vor dem
Zerlegen, nicht 66 Einzelprüfungen.** 66 Einzelprüfungen wären 66 Gelegenheiten, eine
zu vergessen, und die vergessene fiele niemandem auf.

Die Aufnahme ins Paket ist in `requirements.md` begründet („Warum A41 / L265 doch in
dieses Paket gehört"): Nur eine Seite der Brücke zu härten, erzeugt das Muster aus
L179 — zwei Wege zum selben Ziel, einer mit Schutz, einer ohne.

### 5.1 Warum eine eigene Runde, und warum vor S

**Eigene Runde**, weil Runde S das **Protokoll** ändert und A41 den **Parser jedes
eingehenden Kommandos**. Beides in einem Flash macht einen Fehlschlag unzuordenbar.

**Vor S**, weil die Protokolländerung dann auf einem gehärteten Parser landet und
nicht umgekehrt. Runde S fügt der Kommandofolge eine Eröffnungs- und eine
Abschlusszeile hinzu und ändert die Quittung — alles Zeilen, die der Parser vorher
nicht gesehen hat. Ein Parser, der eine verkürzte Zeile sauber abweist, ist dafür die
bessere Grundlage.

**Keine Fähigkeitsmeldung ist hier im Spiel:** A41 ändert nichts, was der ESP sieht.
Die Reihenfolgefrage gegenüber dem ESP stellt sich also nicht — nur die gegenüber
Runde S, und die ist oben entschieden.

### 5.2 Die Abnahme braucht beides: Prüfstand und Gerät

Der Prüfstand mit Schutzseite (Verfahren aus L249, Typen angeglichen nach L256)
beweist die **Funktion**. Er beweist nicht ihre **Einbettung** — und genau dort liegt
das Risiko: `tftled.c` und `esp-spiffs.c` sind Anzeige- und Dateipfade, ein Fehler
dort zeigt sich an der Wand und nicht in der API.

Deshalb drei Abnahmen (AKH.1 bis AKH.3): Prüfstand, ein vollständiger Vollabgleich
über rund 194 Kommandos durch den neuen Parser, und ein Abgleich des Gerätezustands
gegen `tools/snapshots/soll-2026-10-04`.

---

## 6. Runde S — die Brücke fertigbauen

### 6.0 Zwei Richtungen, und eine davon war im ersten Entwurf verkehrt

Der erste Entwurf dieser Spec stützte drei Bausteine auf „der STM kündigt eine
Fähigkeit an". **Das gibt es nicht.** Am Code nachgesehen:

| Was | Richtung | Beleg |
|---|---|---|
| `CAP var-crc`, `FIRMWARE` | **ESP → STM** | `src/esp8266/esp8266.c:207`, `:379`, `:735-756`; der STM hält `esp8266.cap_var_crc` |
| Prüfsummen-Marke an der Variablenzeile | **STM → ESP** | Der STM markiert, **wenn** der ESP `CAP var-crc` gemeldet hat |
| Quittung `.` und Abweisung `!v` | **ESP → STM** | `ESP-uclock.ino:886` sendet, `src/esp8266/esp8266.c:287` und `:292` lesen |

**Alle drei Neuerungen dieser Runde laufen STM → ESP** — Eröffnungszeile,
Abschlussmarke und die Markenpflicht werden vom ESP *ausgewertet*. Der ESP müsste
also wissen, was der STM kann, und **dafür gibt es heute keinen Kanal.** Die
Fähigkeitsmeldung zeigt in die andere Richtung.

**Die Lösung ist in beiden Fällen dieselbe und braucht keinen neuen Kanal: die
Information reist im Strom mit**, nicht in einem Sitzungszustand. Ein alter ESP
verwirft eine unbekannte Kommandoart stillschweigend (`var_set_parameter()` hat keinen
`default:`-Zweig), ein alter STM verwirft eine unbekannte Antwortzeile ebenso. Das ist
die tragende Eigenschaft dieses Protokolls, und sie wird hier genutzt statt umgangen.

### 6.1 A39 / L259 — kein verschachtelter 194er-Stoss mehr

**Der Befund steht unverändert im Code.** `src/main.c:2926` ruft im
`ESP8266_IPADDRESS`-Zweig direkt `var_send_all_variables()`. Der `SYNCVARS`-Zweig
daneben (`:2952-2975`) macht es bereits richtig und trägt die Begründung im Kommentar:
`schedule_esp8266_messages()` wird **auch aus der Warteschleife von `var_send_buf()`**
gerufen (`src/vars/vars.c:586`); von dort liefe der Vollabgleich verschachtelt, alle
rund 194 Kommandos gingen ohne eine einzige Quittung hinaus, und die zurückkommenden
Punkte quittierten der Reihe nach fremde Kommandos — der Schaden aus L102.

**Die Reihenfolge gegenüber dem Ticker war die offene Frage. Der Nutzer hat Variante
(b) gewählt:**

| Variante | Was sie kostet | Was sie bringt |
|---|---|---|
| (a) nur `var_sync_pending = 1;` setzen, Ticker wie bisher sofort | eine Zeile, spart Flash | Der Lauftext **stockt** sichtbar, solange der Abgleich läuft |
| **(b) gewählt** — zusätzlich `ip_ticker_pending` vormerken und den Ticker **nach** dem Abgleich im Hauptloop setzen | ein Flag und drei Zeilen | Die heutige Reihenfolge bleibt **exakt** erhalten; nur die Verschachtelung fällt weg |

Der lokale Puffer entfällt dabei, weil `esp8266.ipaddress` im Hauptloop weiterhin
gültig ist und der Text dort neu gebildet werden kann.

**A39 trägt in dieser Runde doppelt**, und das ist kein Zufall: Es ist der einzige
Pfad, auf dem rund 194 Quittungen eintreffen könnten, ohne dass jemand sie abholt.
Die zusätzlichen Zuordnungszeilen aus §6.4 landen im selben 256-Byte-Empfangsring.
**Ohne A39 wäre §6.4 riskant; mit A39 ist der Ring nie unbeaufsichtigt.**

**Abnahme ist der Hauptloop-Zähler, nicht das Gefühl:** Referenz ist der in L226
gemessene Ruhewert von rund 1'482'480 Durchläufen je 10-Sekunden-Fenster. Dazu die
Beobachtung am Display — **dass der Lauftext vollständig durchläuft, nimmt nur das
Auge ab.**

### 6.2 L260 — ein Erfolgskriterium, das den ganzen Satz prüft

`var_sync_check()` im ESP prüft auf `HARDWARE_CONFIGURATION != 0xFFFF`, und dieses
Kommando ist das **dritte** von rund 194. Kommen 1 bis 3 an und der Rest nicht, hält
der ESP sich für geheilt. Die Lücke fällt dann auf A32 zurück: vier Nachsendeplätze
gegen 190 ausstehende Kommandos.

**Lösung: eine Eröffnungszeile und eine Abschlussmarke, beide im Strom.**

`var_send_all_variables()` sendet als **erstes** eine Eröffnungszeile, die sagt, was
dieser STM in diesem Abgleich tut — „ich markiere die Zeilen" und „ich schliesse mit
einer Marke ab" — und als **letztes** die Abschlussmarke. Der ESP wertet so:

| Lage | Verhalten des ESP |
|---|---|
| Eröffnungszeile gesehen | Der Abgleich gilt erst mit der Abschlussmarke als vollständig |
| keine Eröffnungszeile (alter STM) | heutiges Kriterium, `HARDWARE_CONFIGURATION != 0xFFFF` |

**Damit ist das Richtungsproblem aus §6.0 gelöst, ohne einen neuen Kanal.** Die
Aussage gilt genau für den Abgleich, in dem sie steht — kein Sitzungszustand, der nach
einem einseitigen Neustart falsch werden kann. Das ist zugleich die Antwort auf die
Gattung von L251: Dort musste ein Merker eigens an die `FIRMWARE`-Zeile gebunden
werden, weil er eine Sitzung überdauerte. Was nicht überdauert, kann nicht veralten.

Kosten: **zwei** Kommandos von dann rund 196.

**Die gute Seite der heutigen Lage bleibt erhalten** (L260): Weil die Hardwarekennung
ganz vorn im Stoss steht, darf der Vollabgleich ruhig zehn Sekunden brauchen. Die
Abschlussmarke verschiebt das Kriterium ans Ende — der ESP muss also **warten**
können, ohne vorschnell ein zweites Mal anzufordern. Die bestehende Taktung (bis zu
drei Versuche im Abstand von rund 10 s) bleibt unverändert; die Marke wird nur als
zweite Bedingung ergänzt, der Abstand nicht verkürzt.

### 6.3 C26 / L253 — Zähler **und** Markenpflicht, entschieden

**Gemessen:** 19,1 % der beschädigten Zeilen verlieren das Zeilenende samt Marke; sie
sehen unmarkiert aus und werden angenommen wie bisher. Vorher waren es 76,1 % — die
Prüfsumme hat die Lücke von drei Vierteln auf ein Fünftel gedrückt.

**Der Nutzer hat beide Teile entschieden:**

1. **Sichtbar machen, immer.** Der ESP zählt Zeilen, die **nach** der ersten gesehenen
   Marke dieser Sitzung **ohne** Marke eintreffen. Der Zähler ist über den Logring und
   die Zählerausgabe ablesbar. Das kostet nichts, bricht nichts, und es macht aus
   einer gerechneten Zahl (19,1 %) eine beobachtete.
2. **Markenpflicht für drei benannte Kommandoarten** — `HARDWARE_CONFIGURATION`,
   Update-Host, Update-Pfad.

**Wann die Pflicht gilt** — und hier trägt wieder die Eröffnungszeile aus §6.2, nicht
eine Fähigkeitsmeldung:

- **innerhalb eines Abgleichs mit Eröffnungszeile**, die „ich markiere" sagt: sofort,
  also auch für das dritte Kommando des Stosses. Ohne die Eröffnungszeile griffe die
  Pflicht genau dort nicht, wo `HARDWARE_CONFIGURATION` liegt — der Zustand „noch
  keine Marke gesehen" hält nämlich bis zur ersten markierten Zeile an;
- **ausserhalb eines Abgleichs**, sobald in dieser Sitzung mindestens eine Marke
  gesehen wurde.

**Wie abgewiesen wird: mit `!v`, nicht mit Schweigen.** Der erste Entwurf sah vor, die
Zeile unquittiert zu lassen, damit A32 nach dem Timeout nachsendet. **Das ist
unnötig** — der Mechanismus existiert bereits: `ESP-uclock.ino` sendet `!v`, wenn eine
Zeile gelesen, aber wegen falscher Prüfsumme **nicht angewandt** wurde, und
`src/esp8266/esp8266.c:292` wertet das aus; `var_send_buf()` unterscheidet es vom
Erfolg und merkt das Kommando **sofort** zur Nachsendung vor. Eine unmarkierte
pflichtige Zeile gehört in genau dieselbe Klasse. **Der Gewinn ist nicht kosmetisch:**
Schweigen kostete je Fall 3 Sekunden Hauptloop-Stillstand ohne `watchdog_reload()` —
dieselbe Mechanik, die L266 als Reset-Ursache nachgewiesen hat.

**Die Begründung, die L253 fehlte und die den Ausschlag gab:** Ein leerer Update-Host
hat zuletzt **fremde Firmware geholt**. Der ESP fällt dann auf seine eingebauten
Vorgaben zurück, und die zeigen auf den Server des **Ursprungsprojekts** (L42,
DIR-009) — ohne jede Fehlermeldung. Bei genau diesen drei Werten ist die
19,1-%-Lücke keine statistische Grösse, sondern ein Weg, auf dem fremder Code aufs
Gerät kommt.

**Warum der Dauerzustand nicht eintreten kann, vor dem L253 warnt:** Die Pflicht ist
auf drei Kommandoarten beschränkt, und sie greift nur, wenn der STM in **diesem**
Strom belegt hat, dass er markiert. Der schlimmste Fall ist, dass einer dieser drei
Werte nach den Nachsendeversuchen fehlt — **und das ist derselbe Zustand wie heute,
nur erkennbar statt still falsch.**

**Die Liste ist aufgezählt, nicht gemustert.** Ein Muster verschluckt irgendwann eine
vierte Kommandoart, und das wäre genau der Fehler, gegen den die Begrenzung gebaut ist
(dieselbe Überlegung wie bei `diff-snapshot.sh --soll`, L247).

### 6.4 A35 / L233 — Zuordnung der Quittung, nach dem Nachweis neu entworfen

`ESP-uclock.ino:886` sendet den Punkt, `src/esp8266/esp8266.c:287` liest ihn — **der
ESP quittiert, der STM wertet aus.** Die Quittung bestätigt den **Empfang**, nicht die
Übernahme, und sie trägt **keine Zuordnung**. Eine verspätete Quittung bestätigt damit
das **nächste** Kommando, und A32 hat das Problem vergrössert, weil Nachsendungen
verspätete Quittungen häufiger machen.

**Der erste Entwurf hing die Zuordnung an den Punkt (`.c3`). Der Nutzer hat ihn unter
den Vorbehalt eines Nachweises gestellt, und der Nachweis hat ihn umgeworfen
(L266).** Was dabei herauskam, ist wichtiger als die Korrektur selbst:

- **Die vier Fragen zum Rückfall waren unbedenklich** — kein Byteversatz, kein Rest im
  Ring, kein Teilkommando; die Zeilensynchronisation über `'\n'` hält, `answer_pos`
  wird vor der Auswertung auf 0 gesetzt.
- **Die Frage war falsch gestellt.** Nicht der Versatz ist gefährlich, sondern die
  Folge: `esp8266.c:287` vergleicht auf **genau ein Zeichen**
  (`answer[0] == '.' && answer[1] == '\0'`). Jede Form `.XY` fällt durch die gesamte
  Präfixkette und endet als `ESP8266_UNSPECIFIED`. `var_send_buf()` prüft nur auf `OK`
  und `NAK`, läuft also in den 3-Sekunden-Timeout, und `got_answer` bleibt 0 — womit
  `watchdog_reload()` **ausbleibt** (`vars.c:688`, bewusst so gebaut). **Bei 20 s
  Watchdog und 194 Kommandos: Reset nach rund sieben Kommandos**, selbstheilend nur
  über den Reset, mit verlorenem WLAN und dem Variablensatz-Problem aus L42/L103 im
  Schlepptau.

**Der neue Entwurf: die Zuordnung kommt als eigene Zeile VOR der Quittung.**

```
ACK c3
.
```

| Lage | Verhalten |
|---|---|
| **alter STM** | verwirft `ACK c3` als unbekannte Zeile und liest danach den Punkt **unverändert** als `OK`. Der Fehlfall oben kann gar nicht erst entstehen |
| **neuer STM** | merkt sich die Zuordnung und vergleicht sie beim folgenden Punkt mit dem ausstehenden Kommando. Passt sie nicht, wird die Quittung **verworfen**; das Kommando läuft in seinen Timeout und wird von A32 nachgesendet |

**Damit ist für A35 Teil 1 keine Fähigkeitsmeldung nötig** — und das ist gut so, denn
sie zeigte ohnehin in die falsche Richtung (§6.0). Der blockierende Nachweis-Task ist
damit erledigt und entfällt; er hat seinen Zweck erfüllt, indem er einen besseren
Entwurf erzwungen hat.

**Die Zuordnungszeile geht jeder Quittung voraus, dem Punkt wie dem `!v`.** Sonst
bliebe genau der Fall unzugeordnet, in dem Nachsendungen entstehen.

**Die Mehrkosten sind nachgerechnet, nicht geschätzt** (L266), und gehören hierher,
damit sie niemand später neu schätzt:

| Posten | Wert |
|---|---|
| Zuordnungszeile, rund 8 Zeichen je Kommando | **rund 1,5 kB je Vollabgleich**, Richtung ESP → STM |
| auf einem **alten** STM zusätzlich je Zeile ein `log_flush()`-Busy-Wait, bei 115200 Baud und acht Zeichen rund 0,7 ms | **rund 135 ms über 194 Kommandos** |

Beides ist vernachlässigbar gegen die 3-Sekunden-Timeouts, die der verworfene Entwurf
ausgelöst hätte. **Die 1,5 kB laufen allerdings in genau den 256-Byte-Empfangsring,
dessen Überlauf wir als `d=` messen** — strikt im Wechsel mit dem Warten des STM, also
nie angesammelt. Die einzige Lage, in der sie sich häufen könnten, ist der
verschachtelte Vollabgleich, und **den beseitigt A39 in derselben Runde** (§6.1).

**Teil 2 — Übernahme statt Empfang (nicht in diesem Paket):** Die Quittung soll
bestätigen, dass der Wert übernommen wurde. Das verlangt einen Rückgabewert aus
`var_set_parameter()`, also einen Eingriff in jeden Zweig seines `switch` — und der
hat keinen `default`-Zweig, was die Umstellung zusätzlich verbreitert. **Teil 1 löst
das dringende Problem**; Teil 2 löst ein zweites, das nicht schlimmer geworden ist.
Er bleibt benannt und ist der erste Kandidat der nächsten Brückenrunde. Für eine
Teilmenge ist er ohnehin schon da: `!v` sagt heute „gelesen, nicht angewandt".

### 6.5 Was von der Fähigkeitsmeldung übrig bleibt — und eine Warnung für jeden künftigen Entwurf

Nach §6.2 und §6.4 verlässt sich **keine** Neuerung dieser Runde auf eine
Fähigkeitsmeldung. Alle drei tragen ihre Voraussetzung im Strom mit. Das ist die
Lehre dieser Runde, und sie ist allgemeiner als ihr Anlass:

> **Ein Mechanismus, dessen Sicherheit an einem Hardware-Nebeneffekt hängt, ist nicht
> abgesichert — er hat bisher Glück gehabt.**

Der Nachweis zu A35 fand **keinen** Pfad, auf dem der ESP einen Merker behält, während
eine STM-Firmware ohne die Fähigkeit läuft. Aber diese Sicherheit hing an zwei Dingen,
die **nirgends als Voraussetzung benannt waren**: an einem GPIO-Puls
(`src/esp8266/esp8266.c:738-740`) **und** am Einschaltstrom der LED-Kette über `PB0`,
der bei einem STM-Reset den ESP mitreisst (L183). Fiele eines von beiden weg — ein
anderer Resetpfad, eine andere Platinenrevision —, wäre die Zusage still verloren,
ohne dass jemand den Zusammenhang kennt.

**Dasselbe gilt rückwirkend für `cap_var_crc`.** Die Begründung in L254, der
gefürchtete Fall „STM startet neu, ESP nicht" trete „in der Praxis gar nicht ein",
stützt sich genau auf L183 — also auf denselben Einschaltstrom. Das ist heute richtig
und war nie als Bedingung aufgeschrieben. **Es bleibt so bestehen** (ein Rückbau von
A32 steht nicht zur Debatte), aber es gehört benannt: `knowledge/` nimmt die Warnung
auf (Task W.9), und jeder künftige Entwurf, der eine Sitzungsfähigkeit braucht, nennt
die Voraussetzung, unter der sein Rückfall gilt.

### 6.6 A16 / L106 — Tetris und Snake bedienen den Watchdog nicht

Beide Module haben **null** `watchdog_reload()`. Nach rund 3,6 Steinen ist das
20-s-Fenster um, der Reset ist sicher. Das liegt in dieser Runde, weil der STM ohnehin
geflasht wird — zwei Flashvorgänge für eine Zeile wären unverhältnismässig.

**Die Abnahme braucht den Nutzer**, und er hat die 60 Sekunden zugesagt. Ohne diesen
Lauf gilt AKS.9 als **nicht erfüllt** — nicht als „vermutlich in Ordnung".

---

## 7. Runden P und U — PWA und UI

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

## 8. Runde F — Flash-Überwachung (Runde 4b aus dem alten Paket)

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

**Warum diese Runde zuletzt steht:** Ihre Abnahme ist ein STM-Flash. Nach den Runden H
und S ist der STM ohnehin frisch geflasht, und ein Flash, dessen Ergebnis man bereits
kennt, ist die ehrlichste Probe für einen veränderten Flashpfad.

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent | Runde |
|---|---|---|---|
| `BEFUNDE.md` | Nachzählvermerke, Arbeitsliste bereinigen, Standabschnitt auflösen | `doc-writer` | V |
| `knowledge/directives.md`, `knowledge/quick-reference.md` | E18, B21 (`grep -a`), L256, **die Warnung aus §6.5** | `doc-writer` | W |
| `tools/preview/diag.js`, `tools/preview/shot.sh`, `tools/check-pwa.mjs` | Messmodul: Geometrie, Ankreuzfelder, Bildpunktfarbe, Scrollen, Vollseite | Lead | W |
| `tools/hooks/file-ownership.py`, `tools/hooks/no-danger.py` | Schreibpositionen bzw. Gerätebezug | Lead | W |
| `tools/checks/` | Heimat der Prüfstände; neue Stufe S11 | Lead | W |
| `tools/guardrails.sh` | S11 einhängen | Lead | W |
| `.claude/agents/*.md` | Der Besitz-Hook ist eine Erinnerung, kein Zwang (B22) | Lead | W |
| `ESP8266/ESP-uclock/http.cpp` | `network_scan`, Verlustzeile in den Ring | `esp-developer` | E1 |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Heap-Zeile in den Ring | `esp-developer` | E1 |
| `ESP8266/ESP-uclock/vars.cpp` | Längenprüfung der Kommandozeile (A37) | `esp-developer` | E2 |
| `ESP8266/ESP-uclock/weather.cpp` | `parse_json()` an die Zeichengrenze (C24) | `esp-developer` | E2 |
| `ESP8266/ESP-uclock/http.cpp` | C22, C20, C17, C18 | `esp-developer` | E2 |
| `src/main.c`, `src/tables/tables.c`, `src/esp-spiffs/esp-spiffs.c`, `src/tftled/tftled.c` | **A41**: eine Längenprüfung vorn, dazu die beiden exponierten `while (*p)`-Stellen | `stm-developer` | **H** |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | Eröffnungszeile und Abschlussmarke auswerten, Zuordnungszeile senden, Markenpflicht mit `!v`, Zähler | `esp-developer` | S |
| `src/main.c` | A39 (vormerken, Ticker danach) | `stm-developer` | S |
| `src/vars/vars.c` | Eröffnungszeile und Abschlussmarke senden, Zuordnung auswerten | `stm-developer` | S |
| `src/esp8266/esp8266.c` | Zuordnungszeile lesen und dem Punkt bzw. `!v` zuordnen | `stm-developer` | S |
| `src/tetris/tetris.c`, `src/tetris/snake.c` | `watchdog_reload()` (A16) | `stm-developer` | S |
| PWA-Hauptdatei | B33, B24 | `pwa-developer` | P |
| Markup, Stilvorlage | B30, B28 | `ui-developer` | U |
| `ESP8266/ESP-uclock/http.cpp` | Legacy-Flashfilter, leere Auswahlliste | `esp-developer` | F |
| `tools/smoke-device.sh` | Stufe für `HARDWARE_CONFIGURATION` | Lead | F |
| Die vier Versionsdateien | Anheben je Runde | `release-engineer` | je Runde |

Die Zuordnung folgt der Besitztabelle in `CLAUDE.md` (R3). **Jede Datei hat genau
einen Schreiber, den Lead eingeschlossen.**

**`src/main.c` wird in zwei Runden angefasst — H und S, nie gleichzeitig.** Beide
Male vom `stm-developer`, durch eine Flash-Runde getrennt. Das ist der Fall, vor dem
R3b warnt: Der Besitz-Hook liesse beide durch, die Spec tut es nicht.

**Kodierung und Zeilenenden — vor jedem Patch feststellen, nicht annehmen.**
`http.cpp` und `stm32flash.cpp` sind **UTF-8**, die übrigen ESP-Quellen überwiegend
ASCII oder ISO-8859-1. `ESP-uclock.ino` hat **gemischte Zeilenenden** (überwiegend
CRLF), `http.cpp` genau eine CRLF-Zeile. Ein Patch, der das stillschweigend
vereinheitlicht, macht aus neun geänderten Zeilen 432 — dann ist der Diff nicht mehr
prüfbar und `git blame` zeigt für die ganze Datei den falschen Commit. In **neuem**
Text gilt durchgehend die Umschrift (`Geraet`, `waehrend`).

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
gesetzt (`src/main.c:2949`). Variante (b) verschiebt das Setzen des Tickers in den
Hauptloop, **also muss das Flag mitwandern**: Es gehört zu dem Ticker, den es
zurücknehmen soll, nicht zu dem Zweig, in dem es heute zufällig steht. Keine der vier
Teilbedingungen des Restores wird vereinfacht; die Umsetzung ändert ausschliesslich
den **Ort** des Setzens, und der `stm-developer` weist das im Bericht nach.
E17 (Umbenennung des Flags) ist **ausdrücklich nicht** Teil dieser Änderung — ein
Umbenennen mitten in einem Eingriff an derselben Stelle ist stilles Mitkorrigieren.

**Werden App-Assets ausschliesslich als `.gz` ausgeliefert?** Ja, unverändert. Runde P
ändert, was die PWA mit `fs_show` **anzeigt**, nicht wie sie ausgeliefert wird. Die
Existenzprüfung mit Dateigrösse > 0 bleibt.

**Greift die Änderung an der richtigen Schicht an?** Der wichtigste Punkt dieses
Pakets. Fünfmal wäre die bequemere Schicht die falsche gewesen:

- **L174** — Den Scan in der PWA nicht mehr zu rufen wäre eine Umgehung; der Endpunkt
  stürzt ab, und das ist ein ESP-Fehler.
- **L241** — Die PWA könnte „Datei leer" von „Datei fehlt" nicht selbst
  unterscheiden; nur der ESP kann das liefern.
- **A39** — Der Lauftext ist nicht das Problem; die Verschachtelung ist es.
- **A41** — 66 Einzelprüfungen an den Fundstellen wären die bequeme Schicht. Die
  richtige ist eine Prüfung **vor** dem Zerlegen, dort, wo die Zeile ankommt.
- **A35** — Die Zuordnung an den Punkt zu hängen war die bequeme Schicht: ein Zeichen
  mehr an einer vorhandenen Antwort. Die richtige ist eine **eigene Zeile**, weil das
  Protokoll zeilensynchron ist und nicht zeichensynchron (L266).

### Scalable systems

**Wie viele STM-Kommandos erzeugt die Aktion?** Der Vollabgleich wächst von rund 194
auf rund **196** — Eröffnungszeile und Abschlussmarke. A39 ändert die Zahl nicht,
sondern den **Zeitpunkt**: Die Kommandos laufen künftig nie mehr verschachtelt,
sondern sequenziell mit Quittung. Die Markenpflicht aus C26 kann einzelne Kommandos
nachsenden lassen; die Obergrenze aus A32 (vier Plätze, begrenzte Versuchszahl) gilt
unverändert und ist die Bremse. A41 erzeugt **keine** Kommandos.

**Wie viele Byte pro Minute zusätzlich auf der UART?** Hier liegen die einzigen
nennenswerten Mehrkosten dieses Pakets, und sie sind **gerechnet, nicht geschätzt**:

| Posten | Richtung | Kosten |
|---|---|---|
| Eröffnungszeile + Abschlussmarke | STM → ESP | je Vollabgleich rund 22 Byte |
| **Zuordnungszeile, rund 8 Zeichen je Kommando** | **ESP → STM** | **rund 1,5 kB je Vollabgleich** |
| auf einem **alten** STM zusätzlich ein `log_flush()`-Busy-Wait je Zeile (115200 Baud, acht Zeichen ≈ 0,7 ms) | — | **rund 135 ms über 194 Kommandos** |
| Erfolgszeile, Zähler | — | **0 Byte** — `stm32_log_append()` schreibt nur in den Ring (L261) |

Im Normalbetrieb ausserhalb eines Vollabgleichs sind es wenige Byte je Minute. **Die
1,5 kB laufen in genau den 256-Byte-Empfangsring, dessen Überlauf wir als `d=`
messen** — strikt im Wechsel mit dem Warten des STM, also nie angesammelt. Die einzige
Lage, in der sie sich häufen könnten, ist der verschachtelte Vollabgleich, und **den
beseitigt A39 in derselben Runde.** Diese Kopplung ist der Grund, warum A39 und A35
zusammen gehören und nicht einzeln.

Dem gegenüber stehen die Einsparungen aus C2, die dieses Paket **nicht** anfasst — die
Last sinkt hier also nicht, sie steigt um einen gerechneten Betrag.

**Bleibt jeder ausgelöste Pfad unter 20 s?** Der kritische Pfad ist
`var_send_all_variables()` mit rund 196 Kommandos à bis zu 3 s Quittungswartezeit.
Er läuft nach A39 **im Hauptloop**, wo der `watchdog_reload()` am Schleifenkopf liegt,
und `var_send_buf()` lädt den Watchdog nach eingetroffener Quittung bereits selbst
nach. **Den gültigen Bestand der Aufrufstellen nennt Guardrail S7, nicht dieses
Dokument.**

**Der gefährlichste Pfad dieses Pakets ist die ausbleibende Quittung, und L266 hat
ihn beziffert:** Bleibt `got_answer` 0, bleibt auch `watchdog_reload()` aus
(`vars.c:688`, bewusst so) — bei 3 s je Kommando und 20 s Watchdog ist der Reset nach
**rund sieben** Kommandos da. Deshalb zwei Festlegungen: die Zuordnung als eigene
Zeile (§6.4), damit der Punkt unverändert erkannt wird, und die Abweisung über `!v`
statt über Schweigen (§6.3).

A16 fügt zwei Pfade hinzu, die heute **garantiert** über 20 s laufen. A41 fügt eine
Längenprüfung je eingehender Zeile hinzu: ein `strlen` auf einen Puffer von unter
256 Byte, einmal statt 66-mal.

**Wie oft pollt die PWA?** Unverändert. Runde E1 senkt die Last pro Abruf (keine
`String`-Kette je Netz), nicht die Frequenz.

**Wie lange blockiert der Hauptloop?** Die messbare Zusage steht in AKS.2: Der
Hauptloop-Zähler darf zwischen zwei Diagnosezeilen nicht einbrechen, Referenz rund
1'482'480 Durchläufe je 10-Sekunden-Fenster (L226). Für Runde H gilt dieselbe
Referenz als Gegenprobe — eine Längenprüfung je Zeile darf dort nicht messbar
durchschlagen.

**Hartkodierte Grenzen?** Die Markenpflichtliste umfasst drei Kommandoarten und wird
im Code **benannt, nicht gemustert**. Die Längengrenze aus A41 ist die Grösse des
Empfangspuffers und wird aus ihm abgeleitet, nicht zweitgeschrieben — sonst laufen
beide auseinander, sobald jemand den Puffer ändert.

### Secure by design

**`escapeHtml` bei jedem `innerHTML`?** Runde P berührt die Anzeige von Dateiinhalten
— **und zwar genau, um sie zu unterbinden.** B33 verhindert, dass 123'000 Zeichen
Binärdaten ins DOM gelangen. Die verbleibende Anzeige echter Textdateien geht über
`textContent`, nicht über `innerHTML`.

**Fremddaten als nicht vertrauenswürdig?** Das ist der Kern der Runden E2 und H.
`parse_json()` verarbeitet die Beschreibung einer **fremden Website**; C24 nimmt die
Länge auf die Zeichengrenze zurück. Die Inhalte vom Update-Server bleiben
unvertrauenswürdig behandelt; der Host ist konfigurierbar, die Verbindung ist HTTP —
**und C22 schützt genau diesen Host davor, still gekürzt auf einen anderen Server zu
zeigen** (L124). **Die Markenpflicht aus C26 schützt denselben Host auf der anderen
Seite**, nämlich gegen einen auf der Brücke verfälschten oder verlorenen Wert; ein
leerer Update-Host holt Firmware vom Server des Ursprungsprojekts (L42, DIR-009).
Die Brücke selbst gilt dabei ausdrücklich **nicht** als vertrauenswürdiger Kanal —
sie verliert messbar Zeichen, und A41 behandelt ihre Eingaben entsprechend.

**Keine Credentials in Logausgaben?** Dieses Paket fügt keine neue Ausgabe mit
Nutzdaten hinzu. Die Zuordnungszeile überträgt zwei Hexzeichen einer Prüfsumme,
keinen Inhalt. Der Zähler aus C26 zählt, er protokolliert keine Werte.
**A20 (Geheimnisse in der Timeout-Meldung, L115) bleibt offen und ist nicht Teil
dieses Pakets** — das ist hier festzuhalten, weil Runde S genau an diesen Meldungen
vorbeiarbeitet und die Versuchung bestünde, es nebenbei zu erledigen.

**Neue schreibende Endpunkte?** Keine.

### Stable & reliable

**Wird jeder Fehler ausgewertet?** Das ist der rote Faden von Runde E2: C20 behebt,
dass `http_get_param()` nie NULL zurückgibt und damit **jede** `if (! value)`-Prüfung
ein toter Zweig ist — eine Erfolgsmeldung, die nicht vom Ergebnis abhängt, ist genau
der Fall aus der Checkliste. C17 behebt, dass ein Alarm als aktiv in der Liste steht
und nie auslösen kann. **Und §6.4 behebt den Grundfall dieser Gattung auf der
Brücke:** Eine Quittung ohne Zuordnung ist eine Erfolgsmeldung, die nicht sagt, wofür
sie gilt.

**Leere `catch`-Blöcke?** Keine neuen. Der `code-reviewer` prüft es je Runde.

**Stille Verwerfungen ohne Zähler?** C26 führt genau den fehlenden Zähler ein:
unmarkierte Zeilen nach der ersten Marke einer Sitzung. Die vier Verlustzähler aus A32
bleiben. **Zwei neue Verwerfungen entstehen in diesem Paket, und beide bekommen einen
Zähler:** die verworfene, nicht zuordenbare Quittung (§6.4) und die abgewiesene zu
kurze Kommandozeile (A41). Eine Härtung, die wortlos verwirft, tauscht einen Absturz
gegen ein unerklärliches Nichtverhalten — das ist kein Fortschritt, sondern ein
schlechterer Fehlerbericht.

**Ist der Zustand nach einem Abbruch mitten in einer Sequenz definiert?** Vier
Stellen:

- **Vollabgleich.** `var_sync_pending` wird **vor** dem Senden gelöscht — trifft
  währenddessen ein weiteres `SYNCVARS` ein, bekommt es seinen eigenen Durchlauf. Zwei
  Vollabgleiche nacheinander sind harmlos, zwei ineinander nicht. Steht so im Code und
  bleibt.
- **Abschlussmarke.** Bricht der Abgleich in der Mitte ab, kommt die Marke nicht, und
  der ESP fordert erneut an — **das ist die gewollte Wirkung**, und es ist der
  Unterschied zu heute, wo er sich beim dritten Kommando für geheilt hält.
- **Zuordnungszeile ohne folgende Quittung.** Kommt `ACK xy` an und der Punkt geht
  verloren, darf die gemerkte Zuordnung **nicht** für die nächste Quittung gelten. Sie
  wird deshalb mit der Auswertung verbraucht und beim nächsten gesendeten Kommando
  gelöscht — sonst entsteht genau der Fehler wieder, den A35 beseitigen soll.
- **Abgewiesene Kommandozeile (A41).** Eine zu kurze Zeile wird **ganz** verworfen,
  nicht halb ausgewertet. Halb ausgewertet ist der heutige Zustand und der Befund.

**Wird ein gesetztes Flag auf jedem Pfad wieder aufgelöst?** Die neuen Zustände
(`ip_ticker_pending`, die gemerkte Zuordnung, die Markenerwartung des laufenden
Abgleichs) werden alle im selben Durchlauf gelöscht, in dem sie gewertet werden.
Besonders die Markenerwartung: Sie gilt **nur** für den Abgleich, zu dessen
Eröffnungszeile sie gehört, und endet mit ihm. Für
`pending_weather_ticker_restore` gilt die Zusage aus „Proper architecture". Der
`firmware-analyst` prüft alle drei in der Review von Runde S ausdrücklich gegen diese
Frage.

---

## Verworfene Alternativen

**Runde V als Doku-Aufgabe an den `doc-writer`.** Verworfen: Was zu prüfen ist, steht
nicht im Dokument. Der `doc-writer` trägt ein, er stellt nicht fest. Hätte man L173 so
behandelt, stünde dort heute weiterhin „offen, KRITISCH".

**Die ganze Arbeitsliste nachzählen.** Verworfen: über hundert Einträge, und bei den
meisten hat sich seit dem Beleg nichts geändert. Der mechanische Filter trifft genau
die Fälle, in denen beiläufiges Mitreparieren überhaupt möglich war.

**S11 als Altersprüfung („KRITISCH seit mehr als 30 Tagen").** Verworfen: Das misst
Zeit statt Ereignis und erzeugt Rauschen bei jedem Befund, der zu Recht lange offen
ist. Eine Prüfung mit Fehlalarmen liest niemand mehr — dieselbe Begründung wie bei der
Umlautprüfung in S8.

**Den Logring verkleinern (L176).** Verworfen auf Entscheidung des Nutzers, mit seiner
eigenen Gegenmessung: Der Ring deckt bereits nur rund zwei Minuten ab (L262), und ein
Hänger dauert 90 Sekunden. Begründung in §3.3 und in `requirements.md`.

**A41 ins nächste Paket schieben.** Verworfen: A37 härtet in diesem Paket das ESP-Ende
derselben Gattung. Ein Ende zu härten und das andere offenzulassen, ist genau das
Muster aus L179, das die Uhr stillgelegt hat.

**A41 in Runde S mitnehmen.** Verworfen: Runde S ändert das Protokoll, A41 den Parser
jedes eingehenden Kommandos. In einem Flash wäre ein Fehlschlag nicht zuordenbar.

**A39 Variante (a)** — nur vormerken, Ticker wie bisher sofort. Verworfen zugunsten
von (b), weil sie zwei Dinge auf einmal ändert: die Verschachtelung **und** die
Reihenfolge. Eine Änderung, die zwei Wirkungen hat, ist bei einem Fehlschlag nicht
eindeutig zuzuordnen.

**Die Zuordnung an den Quittungspunkt hängen (`.c3`).** **Verworfen aufgrund des
Nachweises, den der Nutzer verlangt hat** (L266). `esp8266.c:287` vergleicht auf genau
ein Zeichen; jede Form `.XY` endet als `ESP8266_UNSPECIFIED`, `var_send_buf()` läuft
in den 3-Sekunden-Timeout, `got_answer` bleibt 0 und damit auch `watchdog_reload()`
aus — **Reset nach rund sieben Kommandos.** Die eigene Zeile vor dem Punkt vermeidet
das vollständig.

**Eine Fähigkeitsmeldung für die Abschlussmarke und die Markenpflicht.** Verworfen,
weil sie **in die falsche Richtung zeigt**: `CAP`/`FIRMWARE` laufen ESP → STM
(`esp8266.c:207`, `:379`), die drei Neuerungen laufen STM → ESP. Eine neue
Gegenrichtung zu bauen wäre ein zweiter Sitzungszustand mit allen Fragen aus L251.
Die Eröffnungszeile im Strom löst dasselbe Problem ohne Zustand.

**Markenpflicht für alle Kommandoarten (C26).** Verworfen aus dem Grund, den L253
nennt: Eine unmarkierte Zeile muss durchgehen, sonst bricht jeder Betrieb, in dem der
STM nicht markiert. Die Begrenzung auf drei Kommandoarten hält den Nutzen und lässt
das Risiko weg.

**Unmarkierte pflichtige Zeile unquittiert lassen.** Verworfen zugunsten von `!v`: Der
Mechanismus „gelesen, nicht angewandt, sofort nachsenden" existiert bereits
(`esp8266.c:292`). Schweigen kostete je Fall 3 Sekunden Hauptloop-Stillstand **ohne**
Watchdog-Nachladung — dieselbe Mechanik, die L266 als Reset-Ursache nachgewiesen hat.

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

**Runde F früher fahren.** Verworfen: Ihre Abnahme ist ein STM-Flash, und nach H und S
ist ohnehin einer gelaufen. Ein zusätzlicher Flash nur zur Abnahme eines Flashpfades
wäre Aufwand ohne Erkenntnis.

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
| **H** | ☐ | — | — | — |
| S | ☐ | ☐ | — | — |
| P | — | — | ☐ | ☐ |
| U | — | — | ☐ | ☐ |
| F | — | ☐ | — | — |

`APP_VERSION` und `CACHE_NAME` gehören **immer zusammen**. Nur der
`release-engineer` führt das aus (R4); Teammates bumpen nichts.

**Runde S hebt zwei Versionen in einer Runde** — das ist zulässig, weil zwei
Einspielschritte stattfinden (erst ESP, dann STM) und jeder seine eigene Version
trägt. Der Tag `release/<stm>-<esp>-<app>` wird **je Einspielschritt** gesetzt und
**gepusht** (DIR-011).
