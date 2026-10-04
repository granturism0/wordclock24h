# Design — Grosses Befundpaket

**Momentaufnahme vom 2026-10-04** (DIR-006). Gehört zu
`specs/grosses-paket-2026-10-04/requirements.md`.

Zeilennummern in diesem Dokument sind **Fundhilfen, keine Anker**. Sie verschieben
sich durch die eigenen Patches — das ist in diesem Projekt belegt (F1: `main.c:3214`
lag nach dem Eingriff bei `:3295`). Wer patcht, sucht über den Inhalt und prüft, dass
der Anker **genau einmal** vorkommt.

---

## Rundeneinteilung

Sechs Rollouts, jeder mit eigenem Commit und eigenem Tag (DIR-011).

| Runde | Inhalt | Laufzeit | Warum hier |
|---|---|---|---|
| **0** | C14, E16, Prüfmethode | ESP + Doku | **Baut die Messinstrumente.** Die Abnahme der Runden 1 bis 4 liest `/api/stm32_log` und den Testplan. Ohne Runde 0 sind mehrere AK unerfüllbar — genau der Fehler von L177 |
| **1** | C15, C16, Vertragsdurchsetzung | ESP + PWA | Ändert den API-Vertrag. Muss **vor** Runde 2 liegen: Die PWA-Validierung aus B6 baut auf dem Vertrag auf, und umgekehrt wäre die PWA zuerst strenger als das Gerät |
| **2** | B1-Familie | PWA | Reine PWA-Runde, Rollout per LittleFS, Rückweg billig. Nach Runde 1, weil B6 den dort festgelegten Vertrag spiegelt |
| **3** | B13, B14, L114 | PWA-Markup | **Eigene Runde, obwohl auch PWA.** Der Umbau verschiebt sechs Kacheln im DOM. Läuft er mit Runde 2 zusammen, ist eine Regression nicht mehr zuzuordnen — das ist die Lehre aus R3b vom 03.10.2026 |
| **4a** | A6, Selbstheilung des Variablensatzes | ESP (ggf. STM) | **Spät, weil riskant.** Berührt den Variablensatz, an dem die Hardwarekennung und die Update-Quelle hängen |
| **4b** | Flash-Überwachung (C9c6, L179) | ESP + Werkzeug | **Eigene Runde, obwohl ebenfalls ESP.** Erst wenn die Selbstheilung aus 4a am Gerät steht, ist ein Fehlschlag im Flashpfad eindeutig zuzuordnen |

**Warum nicht alles in einem Zug:** Jeder Rollout dieses Pakets ist ein OTA, und
genau dieser Pfad trägt ein ungeklärtes Risiko (C13, L180 — siehe
`requirements.md`, Risiko 1). Eine Runde mit genau einer Laufzeit macht einen
gescheiterten OTA eindeutig zuordenbar. Aus derselben Begründung trennt `tasks.md`
die Runden 4a und 4b durch eine eigene Flash-Runde, obwohl beide im ESP liegen.

**Zwischen zwei Runden steht immer ein Gerätetest, nicht nur ein Smoketest**
(DIR-012). Der Smoketest prüft, ob das Gerät lebt — nicht, ob es noch tut, was es
soll.

---

## Lösungsweg

### Runde 0

#### C14 — die beiden ESP-Zeilen in den API-Ring

**Grundentscheid: dieselbe Zeichenkette, zweimal verwendet.** Jede der beiden
Stellen baut ihre Zeile **einmal** in einen Stackpuffer und gibt sie danach zweimal
aus — `Serial.println()` wie heute **und** `stm32_log_append()`. Damit können
Mitschnitt und API nicht auseinanderlaufen; eine zweite Formatierung wäre eine
zweite Wahrheit.

**Kein `String`.** Der Aufbau läuft über `snprintf` in einen lokalen `char`-Puffer.
Auf dem Haufen wird nichts angelegt — das ist nicht Geschmack, sondern L175: Der
grösste zusammenhängende Block fällt im Betrieb um 44 %, und `String`-Aufbau ist
dort der benannte Treiber.

**Prefix: `- `, unverändert.** Die Zeile wird so in den Ring gelegt, wie sie auf die
Leitung geht, einschliesslich des führenden `- `.

Drei Gründe:

1. Es ist bereits die Konvention dieser Brücke. `src/esp8266/esp8266.c:332` stuft
   jede Zeile mit `"- "` als `ESP8266_DEBUGMSG` ein — „`- ` heisst: vom ESP" ist im
   Protokoll schon verankert und muss nicht neu erfunden werden.
2. Der Ring enthält heute ausschliesslich STM-Zeilen, die über `LOG ` ankommen
   (`src/log/log.c:33` → `src/esp8266/esp8266.c:580`, Präfix wird beim Ablegen
   entfernt). Die Gegenrichtung ist dort frei.
3. Werkzeuge, die den Mitschnitt nach `- heap free=` durchsuchen, funktionieren
   unverändert weiter.

**Vor dem Umsetzen zu prüfen, nicht anzunehmen:** dass kein STM-Logtext mit `- `
beginnt. Nachweis per `grep -a` über `src/**` — **`-a` ist Pflicht**, sonst
überspringt `grep` acht Quelldateien stillschweigend, darunter die grösste (L166).
Findet sich eine solche Zeile, wird stattdessen `esp: ` als Präfix genommen und
`design.md` dieser Spec ist widerlegt, nicht die Umsetzung angepasst.

**Stelle 1 — `ESP-uclock.ino`, in `esp_heap_log()`**, heute `Serial.print`-Kette
(`:463-467`), innerhalb von `#if ESP_HEAP_LOG`:

- Puffer `char line[48]` — die längste Form ist
  `- heap free=4294967295 max=4294967295` = 37 Zeichen plus Abschluss.
- `snprintf (line, sizeof (line), "- heap free=%lu max=%lu", …)`
- `Serial.println (line); Serial.flush (); stm32_log_append (line);`

**Stelle 2 — `http.cpp`, im Zweig `if (! http_write_broken)`** (heute `:1211-1215`):

- Puffer `char line[40]` — längste Form `- http write lost 4294967295` = 28 Zeichen.
- `snprintf`, dann `Serial.println`, `Serial.flush`, `stm32_log_append`.
- `base.h` ist in `http.cpp` bereits eingebunden (`:18`), die Deklaration von
  `stm32_log_append` steht dort (`base.h:32`). Kein neuer Include.
- **Die Ein-Zeile-je-Verbindung-Regel bleibt.** `http_write_broken` schützt heute
  davor, dass die Folgeblöcke derselben Antwort je eine Zeile erzeugen. Das gilt für
  den Ring genauso — die Zeile kommt **innerhalb** dieses Zweigs dazu, nicht
  daneben.

**Ringhaushalt, gerechnet statt gehofft.** Der Ring fasst 32 Zeilen
(`ESP-uclock.ino:80`). Der STM liefert seine Diagnosezeile im 10-Sekunden-Takt, also
rund 6 Zeilen je Minute; die Heap-Zeile kommt mit einer je Minute dazu. Der
ESP-Anteil liegt damit bei rund **1/7**, und die sichtbare Rückschau schrumpft von
rund 5,3 auf 4,6 Minuten. Tragbar — aber nur, solange beide Intervalle so bleiben.
AK0.3 misst es.

**Was C14 ausdrücklich nicht leistet:** L177 wollte die Fragmentierung *über
Stunden* verfolgen. Das kann ein Ring von 32 Zeilen nicht, und dieses Design
behauptet es auch nicht. Über die API wird die Heap-Zeile **stichprobenfähig**; der
Verlauf über Stunden bleibt Sache des seriellen Mitschnitts. Diese Unterscheidung
gehört in die Abnahme, sonst entsteht erneut eine unerfüllbare Bedingung.

#### E16 — zwei Berichtigungen im Testplan

Beide Stellen über den Zeileninhalt ansteuern, nicht über die Zeilennummer.

**S47** (`| **S47** | Farbanimation wählen …`): Der Hinweis „Solange eine
Farbanimation läuft, ist die Display-Farbe aus S34 wirkungslos" wird ersetzt durch
die am Gerät belegte Wirkung — die Display-Farbe ist **nach** dem Moduswechsel
verloren, nicht nur während der Animation wirkungslos. Dazu derselbe Vermerk, den
S51 und S53 schon tragen: **Ausgangswert vorher aus dem Rohabzug notieren.**
Die Klasse bleibt **S**, weil der Schritt mit notiertem Ausgangswert rücknehmbar
ist — was fällt, ist die Zusage „folgenlos".

**S114** (`| **S114** | STM zurücksetzen …`): Die Wirkungsbeschreibung wird um den
ESP-Mitstart ergänzt: Alle ESP-Zähler (`write_lost_*`, `no_request_*`,
`update_cache_hits`) gehen auf 0, und der Variablenverlust aus L42/L103 tritt ein.
Dazu der Satz, der die eigentliche Falle schliesst: **Messreihen über einen
STM-Reset hinweg sind ungültig.** Belegt über `update_cache_hits` 5 → 0, einen
reinen ESP-Zähler ohne STM-Bezug.

Die Ursache ist nach heutigem Stand **elektrisch, nicht im Code**: Der Handler ruft
ausschliesslich `stm32_reset()`; der STM schaltet beim Start über `PB0` die 5-V-
Versorgung der LED-Kette, und der Einschaltstrom zieht die Versorgung so weit
herunter, dass der ESP mitgeht. Dieselbe Stromreserve, die bei `test_display` nach
44 s zum Brownout führt. Das gehört als **Vermutung mit Beleg** in den Testplan,
nicht als Tatsache.

#### Prüfmethode gegen den blinden Fleck (L188, L103)

Der Testplan bekommt einen eigenen, benannten Abschnitt. Er erklärt zuerst das
Problem in einem Satz — *`settings_xml` liefert die ESP-Kopie; der ESP setzt sie beim
Setter sofort und schickt das Kommando erst danach an den STM* — und nennt dann drei
Verfahren mit ihrem jeweiligen Preis.

**V1 — Rahmenmessung der Verlustzähler. Pflicht je Setter-Phase.**

Vor und nach jeder Phase wird der `d=`-Wert aus der Diagnosezeile des STM gelesen.
Die Zeile ist über `/api/stm32_log` erreichbar: Sie entsteht über `log_printf`, wird
in `src/log/log.c:33` auf die Brücke gespiegelt und kommt mit dem Präfix `LOG ` am
Ring an — anders als die ESP-eigenen Zeilen, und das ist nach C14 der sichtbare
Unterschied.

- `d` unverändert → in diesem Fenster ist kein Zeichen verworfen worden. Die
  Kommandos sind beim STM angekommen; ESP-Kopie und STM-Zustand dürfen als gleich
  gelten.
- `d` gestiegen → die Gegenprobe dieser Phase ist **ungültig**. Phase wiederholen,
  Zuwachs und Fenster im Bericht nennen (das speist zugleich den
  Beobachtungsauftrag zu A28).

**Grenzen, ausdrücklich:** V1 ist eine **notwendige, keine hinreichende**
Bedingung. `d` zählt die im Empfangsring des STM verworfenen Zeichen; ein Kommando,
das der ESP gar nicht erst absetzt, taucht dort nicht auf. Und `rx` ist ein
**Höchststand**, kein Füllstand (`src/main.c:3396`) — wer ihn als aktuellen Wert
liest, überschätzt die Lage, wie am 04.10.2026 geschehen. Ausserdem fasst der Ring
nur rund 4 bis 5 Minuten Rückschau; längere Phasen brauchen Zwischenablesungen.

**V2 — Wirkungsprobe am sichtbaren Verhalten.** Für Werte mit optischer Wirkung
(Helligkeit, Display-Farbe, Anzeigemodus, Ambilight) ist die Uhr selbst das
STM-seitige Anzeigeinstrument. Nicht automatisierbar, gehört dem Nutzer oder einem
Schritt, der ihn ausdrücklich um Bestätigung bittet. **Wertvoll genau dort, wo V1
nichts sagt.**

**V3 — Abgleich über den STM-Neustart. Höchstens einmal je Durchlauf, am Ende.**
Nach `maintenance_reset_stm32` kündigt der STM seinen **gesamten** Variablensatz neu
an; die ESP-Kopie wird damit **aus dem STM** aufgebaut. Ein Rohabzug danach zeigt
also, was der STM wirklich hält.

Der Preis ist hoch und muss im Testplan stehen: Der Reset reisst den ESP mit (L183),
alle ESP-Zähler gehen auf 0, und der Variablenverlust aus L42/L103 kann eintreten.
Deshalb **nie mitten in einer Messreihe**, nur als Abschlussvergleich, nur mit
Freigabe des Nutzers.

### Runde 1 — der Parametervertrag

#### Der Vertrag

> **Ein Wert ausserhalb seines Bereichs wird abgewiesen, nicht geklemmt.**
> `{"ok":true}` bedeutet: Der **gesendete** Wert gilt jetzt.
> `{"ok":false,"error":2,"detail":"<param> out of range (<min>..<max>)"}` bedeutet:
> Es hat sich **nichts** geändert.

Fünf Gründe für „abweisen" statt „klemmen":

1. **Die Architektur-Checkliste verlangt es.** Punkt 4 fragt wörtlich: „Werden Werte
   still zurechtgebogen und danach als ‚gespeichert' gemeldet?" Klemmen mit
   `{"ok":true}` ist genau das.
2. **Die Mehrheit tut es schon.** `animation_profile_set` weist ab, und alle im
   Durchlauf geprüften Endpunkte ausser `fs_show` liefern sauberes `{"ok":false}`.
   Abweisen ist die kleinere Änderung und die kleinere Überraschung.
3. **Die Oberfläche braucht eine verlässliche Aussage.** Sie müsste sonst je
   Endpunkt wissen, ob ein `{"ok":true}` den gesendeten Wert bedeutet — das ist der
   Kern von L186.
4. **Klemmen und die PWA-Seite widersprechen sich.** B6 (Massnahme 17) verlangt
   Validierung statt stillem Clamping in der PWA. Bliebe der ESP beim Klemmen,
   hätten wir zwei gegenläufige Verträge in einem System.
5. **Legacy und Backup-Import gehen durch dieselben Endpunkte.** Ein Klemmen
   schreibt dort still eine fremde Konfiguration um. Die PWA-Seite hat das mit L65
   bereits anders gelöst: melden, was zurechtgebogen wurde, mit **beiden** Zahlen.
   Der ESP soll gar nicht erst biegen.

**Der `detail`-Text nennt den Bereich.** Heute steht an mehreren Stellen nur
`"deceleration out of range"`. Die Form ist `<param> out of range (<min>..<max>)`,
wie sie `timezone out of range (-12..14)` schon vorlebt — die PWA zeigt diesen Text,
und eine Fehlermeldung ohne Bereich zwingt den Nutzer zum Raten.

#### Umfang in dieser Runde

**Nicht alle zwanzig.** Umgestellt wird, was dieses Merkmal erfüllt:

> Der Endpunkt klemmt einen Zahlenwert **still** und meldet danach `{"ok":true}`.

Der Bestand wird **gemessen, nicht geschätzt**: Ein lesender Durchgang durch
`http.cpp` listet jede Stelle, an der nach `http_get_int_param()` eine Zuweisung der
Form `if (x < min) x = min;` / `else if (x > max) x = max;` steht. Diese Liste ist
das Ergebnis von Task 1.1 und der verbindliche Umfang von Task 1.2 — kein Endpunkt
mehr, keiner weniger.

Bekannt und gesetzt: `ambilight_mode_profile_set` (`deceleration`,
`http.cpp:10081-10088`, mit dem Kommentar „bleibt absichtlich stehen — das ist
Massnahme 17 und nicht dieser Schritt"). Dieses Paket ist dieser Schritt.

#### `fs_show` — drei Fälle statt zwei

Heute gehen die Kopfzeilen hinaus, **bevor** die Datei geöffnet wird
(`http.cpp:11191`), und wenn `LittleFS.open()` scheitert, folgt nichts mehr. Das ist
dieselbe Form wie L161 und L174: erst senden, dann arbeiten.

Neu, in dieser Reihenfolge:

1. `LittleFS.begin()`
2. Existenz prüfen
3. **Dann erst** die Kopfzeilen senden, passend zum Ergebnis
4. Inhalt senden, `LittleFS.end()`

| Fall | Antwort |
|---|---|
| Parameter fehlt oder leer | `application/json`, `{"ok":false,"error":1,"detail":"filename required"}` |
| Datei existiert nicht | `application/json`, `{"ok":false,"error":6,"detail":"file not found"}` |
| Datei existiert, Grösse 0 | `text/plain`, Rumpflänge 0 — **wie heute, aber jetzt eindeutig** |
| Datei existiert mit Inhalt | `text/plain` mit Inhalt — unverändert |

**Neuer Fehlercode 6, `HTTP_API_ERROR_NOT_FOUND`**, neben den bestehenden 1 bis 5
(`http.cpp:235-244`). Das erweitert den API-Vertrag; die PWA muss ihn kennen.

**Nicht `http_fs_file_exists_and_nonempty()` benutzen.** Der Helfer prüft
zusätzlich auf Grösse > 0 und würde die leere Datei als „fehlt" melden — genau die
Verwechslung, die der Befund abschafft. Die Existenzprüfung ist eigens zu machen.

**Ein `begin()`, ein `end()`.** Die Prüfung gehört in dieselbe Klammer wie das Lesen,
nicht in eine eigene. C9b (L129) nennt unpaariges `LittleFS.begin()` und dreifaches
Mounten je Request als belegte Defekte — kein neuer dazu.

**PWA-Seite:** Sie unterscheidet am **`Content-Type`**, nicht am Rumpfanfang. Ein
`application/json` ist ein Fehler, ein `text/plain` ist Inhalt — auch leerer. Eine
Prüfung auf die Zeichenfolge `{"ok":false` am Anfang wäre falsch, sobald eine
angezeigte Datei genau so beginnt.

### Runde 2 — PWA-Eingabefelder

Alle Änderungen in der PWA-Hauptdatei und im Service Worker; ein Schreiber
(`pwa-developer`), **ein Task je Thema**, nicht ein Sammelpatch — die Datei hat über
12'000 Zeilen und ist der Sammelpunkt fast aller PWA-Arbeit (R3).

| Thema | Weg |
|---|---|
| **B1** (Massnahme 4) | Erst feststellen, **welche** Speicherpfade das Flag heute stehen lassen. L33 hat den Reset in `runButtonRequest()` eingebaut, aber **nur** im Zweig `reload === true`. Reine Auslöseaktionen lassen es bewusst stehen — das bleibt so, sonst verlöre ein „Wetter abrufen" eine offene Bearbeitung. Zu schliessen sind die Speicherpfade **ausserhalb** dieses Trichters |
| **B2** (R2-11) | `visibilitychange` bekommt den fehlenden `else`-Zweig: Poller beim Verbergen stoppen, beim Zurückkehren **sofort einmal** laden und dann weiterpollen. Ohne das Sofortladen zeigt die Oberfläche beim Zurückkommen veraltete Werte |
| **B3** (R2-4) | `apiFetch` statt rohem `fetch` im Flash-Pfad — damit greifen Zeitlimit, Fehlerauswertung und Meldungsfläche auch dort |
| **B4** (R2-5) | `finishProgressUi(2200)` statt `(0)` |
| **B5** (R2-10) | Hinweis bei `isSecureContext === false`. **Kein Fehler** — die PWA funktioniert, nur die Installation nicht. Der Text sagt beides |
| **B6** (Massnahme 17) | Die Bausteine stehen bereits: `clampNumber()` und `readNumberInputOrReport()` (L29). Offen sind die Felder, die daran vorbeigehen. Die Meldung nennt **beide Zahlen** (eingegeben → erlaubt), wie es der Backup-Import nach L65 tut |
| **B7** (Massnahme 7) | `file.size > 0` vor dem Hochladen, Längenprüfung im Service Worker. Die ESP-Seite ist seit Langem abgesichert; eine leere `.gz` hat schon einmal einen weissen Bildschirm erzeugt |
| **B1i** (L64) | Die fünf Overlay-Felder über denselben Pfad wie B6. **Vorher klären**, ob der ESP sie nach Runde 1 abweist — wenn ja, ist die PWA-Prüfung die Höflichkeit, nicht der Schutz |
| **B11** (L111) | Entscheiden und begründen. Vorschlag: den toten Getter **entfernen**. Ein Getter, der nie gerufen wird, ist kein Vorrat, sondern eine Behauptung über eine Fähigkeit, die niemand prüft |
| **B12** (L112) | Für die vier nur per Import erreichbaren Setter entweder Bedienelemente oder eine ausdrückliche Begründung im Code-Kommentar. Beides ist zulässig, **nichts** ist es nicht |
| **B17** (L142) | Die Ursache ist offen, drei Spuren stehen. **Unabhängig davon** gilt: ein leeres Feld ohne Meldung ist in allen drei Fällen der falsche Zustand. Also: Fehler aus `fetchStm32Log(true)` auswerten statt verschlucken, `stm32LogRefreshInFlight` auf **jedem** Pfad zurücksetzen (`finally`), und bei leerem Ring den Text „Logbuch leer" statt gar nichts — das unterscheidet „leer" von „nicht geladen", dieselbe Klasse wie C16 |
| **B18** (L151) | Warnung beim Modulwechsel mit ungespeicherten Änderungen. Kein Verbot — ein Hinweis mit „trotzdem wechseln" |

### Runde 3 — UI-Kacheln

**Variante 2 aus dem UX-Review, nicht Variante 1.** Das Markup kommt in die
gewollte Reihenfolge (`source, remote, local, backup, service, files`); danach
entfallen **alle drei** `grid-template-areas`-Blöcke und die sechs
`grid-area`-Zuweisungen.

Begründung: Die Area-Templates sind nicht die Krankheit, sondern der Verband. Sie
reparieren eine schiefe Markup-Reihenfolge an jedem Umbruchpunkt einzeln und
bezahlen das mit der Fokusreihenfolge (L116, WCAG 2.1 SC 1.3.2 und SC 2.4.3, beide
Stufe A). Jeder künftige Umbruchpunkt bräuchte ein viertes Bild von Hand.

Geprüft und Voraussetzung der Variante: `app.js` greift ausschliesslich über IDs zu,
`nth-of-type` kommt dort **nicht einmal** vor. **Noch nicht geprüft** und deshalb
Teil des Tasks: der Service Worker und der Backup-Import-Dialog.

Zur Lückenbildung (L114): Die Höhendifferenz ist **struktureller Normalzustand** —
die Fortschrittsanzeige in `remote` trägt `is-hidden`, solange kein Update läuft.
Ein Raster, das die Kacheln nach Höhe packt, löst das; eines, das nur die Reihenfolge
ändert, nicht. Gemessen wird mit `tools/preview/` und `&module=maintenance`, dem
Weg, der die 1248 px gegen 348 px belegt hat.

**B14 (L120) ist eine Bewertung, kein Patch.** `accent-color` trägt die 3:1-Zusage
für die Darstellung des Hakens (WCAG 1.4.11); ohne sie läge der Wert bei rund 1,6:1.
Wer die beigebraune Darstellung „nebenbei" wegnimmt, nimmt die Zusage mit. Ergebnis
des Tasks ist deshalb eine **gemessene Entscheidung**, nicht zwingend eine Änderung.

**B1f und B1h sind Gegenproben, keine Umsetzungen.** Beide Befunde stehen auf
erledigt (L43: von 46 auf 14 zurückgeführt; L59/L60: kein `primary` mehr in
`app.js`). Findet die Gegenprobe eine Abweichung, entsteht ein **neuer Befund** in
`BEFUNDE.md` — nicht ein stiller Nachbau.

### Runde 4a — A6, Selbstheilung des Variablensatzes

#### Erst messen

Die Ursachenanalyse ist offen, und der Nutzer sagt, es sei früher nicht so gewesen.
Vor jedem Eingriff steht deshalb eine Messung (Task 4a.1), die drei Zahlen liefert:
wie viele Kommandos `var_send_all_variables()` absetzt, wie viele davon in einen
Quittungs-Timeout laufen, und wie weit `d=` über das Fenster steigt. Ohne diese
Zahlen ist jeder Eingriff ein Versuch.

Der Auslöser ist bekannt und steht bereits im Code: `src/main.c:2896-2908` ruft
`var_send_all_variables()` im Zweig `ESP8266_IPADDRESS`. Der Mechanismus **existiert
also** — er ist unzuverlässig, nicht abwesend.

#### Weg A — rein ESP-seitig, bevorzugt

Der ESP meldet seine IP mit `statusmsg ("IPADDRESS", wifi_ip_address)`
(`wifi.cpp:101`, `:164`, `:186`, `:202`). Der STM wertet jede Zeile `IPADDRESS …`
aus (`src/esp8266/esp8266.c:343-349`) und ruft daraufhin den Vollabgleich.

**Daraus folgt: Der ESP kann den Vollabgleich selbst anfordern, ohne dass am STM
eine einzige Zeile geändert wird.** Er muss die Meldung nur erneut senden.

Umsetzung:

- Nach dem Hochlauf prüft der ESP, ob sein Variablensatz vollständig aussieht. Das
  schärfste verfügbare Merkmal ist `HARDWARE_CONFIGURATION` — es ist nach dem
  Verlust **65535** und hat sonst einen Gerätewert.
- Ist es unvollständig, sendet er `IPADDRESS` erneut, mit Abstand und **begrenzter**
  Zahl von Versuchen (Vorschlag: höchstens drei, je rund 10 s auseinander).
- Jeder Versuch schreibt eine Zeile in den Ring. **Das geht erst, weil Runde 0
  C14 gebaut hat** — ohne sie wäre die Selbstheilung erneut unbeobachtbar.
- Bleibt es nach dem letzten Versuch unvollständig, meldet der ESP das **einmal
  deutlich** statt stillschweigend weiterzulaufen.

**Sichtbare Nebenwirkung, die der Nutzer freigeben muss:** Der STM zeigt im selben
Zweig einen Lauftext mit der IP (`src/main.c:2906`). Jeder Wiederholversuch lässt
die Uhr diesen Lauftext erneut zeigen. Bei höchstens drei Versuchen, und nur nach
einem ESP-Neustart, ist das vertretbar — **aber es ist seine Uhr im Dauerbetrieb,
und er sieht es.** Steht ausdrücklich zur Freigabe.

Preis insgesamt: **keine STM-Änderung, kein STM-Flash.** Damit entfällt auch der
Rattenschwanz aus L183 (STM-Reset reisst den ESP mit und löst denselben Verlust neu
aus).

#### Weg B — ESP und STM, nur falls die Messung Weg A ausschliesst

Ein eigenes, stilles Kommando für den Vollabgleich, ohne Lauftext. Preis: STM-Flash,
eine eigene Flash-Runde, und die Abhängigkeit in `tasks.md`.

**In beiden Wegen nicht angefasst:** `var_send_all_variables()` selbst. Die rund 190
quittungspflichtigen Kommandos ohne `watchdog_reload()` (L85) und die Begrenzung des
Reloads nach Quittung (A17, L108) sind eigene Massnahmen mit eigener Spec. Wer sie
hier mitnimmt, ändert in einer ohnehin riskanten Runde zusätzlich das Zeitverhalten
des Startpfads.

### Runde 4b — Flash-Überwachung (C9c6, L179)

Zwei Teile, beide im ESP:

1. **Der Legacy-Pfad wendet denselben Filter an wie die API.** Heute übernimmt
   `http.cpp:6909-6913` den Namen ungeprüft per `strncpy`. Der API-Endpunkt prüft
   über `http_remote_stm32_filename_matches()` gegen die Hardwarekennung. Es ist
   **dieselbe Funktion** zu rufen, nicht eine zweite Prüfung zu schreiben — zwei
   Prüfungen laufen auseinander, und genau das ist der Befund.
2. **Bei unbekannter Hardware bietet die Auswahlliste keine Datei an.** Heute zeigt
   sie bei `HARDWARE_CONFIGURATION` = 65535 **alle**, die F103 direkt neben der F411.
   Eine leere Liste mit Hinweis ist ehrlicher als eine, die zum Griff ins Falsche
   einlädt.

**Der Rückweg muss offen bleiben.** Bei leerem `HARDWARE_CONFIGURATION` weist der
geschützte Endpunkt **jeden** Namen ab — auch den richtigen. Wer beide Wege dicht
macht, nimmt sich die Reparatur. Deshalb: Die Reparatur läuft über
`./tools/flash-stm.sh` (DIR-010), das den STM zuerst zurücksetzt und damit die
Hardwarekennung zurückholt. Das steht so bereits in `CLAUDE.md` und muss in der
Hinweismeldung der leeren Liste **genannt** werden, sonst führt die Härtung in eine
Sackgasse.

---

## Betroffene Module

| Datei | Änderung | Agent | Runde |
|---|---|---|---|
| `ESP8266/ESP-uclock/ESP-uclock.ino` | `esp_heap_log()` auf einen Stackpuffer, zusätzlich `stm32_log_append()` | `esp-developer` | 0 |
| `ESP8266/ESP-uclock/http.cpp` | Verlustzeile zusätzlich in den Ring | `esp-developer` | 0 |
| `TESTPLAN-PWA.md` | S47, S114, neuer Abschnitt Prüfmethode | `doc-writer` | 0 |
| `.claude/agents/pwa-tester.md` | Rahmenmessung V1 als Pflicht je Setter-Phase | Lead | 0 |
| `ESP8266/ESP-uclock/http.cpp` | Klemmen → Abweisen, `detail` mit Bereich, `fs_show`, Fehlercode 6 | `esp-developer` | 1 |
| PWA-Hauptdatei | `fs_show`-Auswertung über `Content-Type` | `pwa-developer` | 1 |
| PWA-Hauptdatei, Service Worker | B1 bis B7, B1i, B11, B12, B17, B18 | `pwa-developer` | 2 |
| Markup, Stilvorlage | Kachelreihenfolge, Wegfall der Area-Templates, B14 | `ui-developer` | 3 |
| `ESP8266/ESP-uclock/wifi.cpp` bzw. `ESP-uclock.ino` | Selbstheilung des Variablensatzes (Weg A) | `esp-developer` | 4a |
| `src/main.c`, `src/vars/vars.c` | **nur bei Weg B** | `stm-developer` | 4a |
| `ESP8266/ESP-uclock/http.cpp` | Legacy-Flashfilter, leere Auswahlliste | `esp-developer` | 4b |
| `tools/smoke-device.sh` | Prüfung auf `HARDWARE_CONFIGURATION` ≠ 65535 | Lead | 4b |
| `BEFUNDE.md` | Stand nach jeder Runde, acht veraltete B1-Zeilen streichen | `doc-writer` | je Runde |
| Die vier Versionsdateien | Anheben je Runde | `release-engineer` | je Runde |

Die Zuordnung folgt der Besitztabelle in `CLAUDE.md` (R3). Jede Datei hat genau einen
Schreiber, **den Lead eingeschlossen**.

**Kodierung:** `ESP-uclock.ino` und `wifi.cpp` sind vor dem Patchen auf ihre Kodierung
zu prüfen. Die pauschale Annahme „ISO-8859-1" ist für `http.cpp` und `stm32flash.cpp`
belegt **falsch** — beide sind UTF-8, und ein `latin-1`-Patcher beschädigt sie. Die
Umschrift in neuem Text schadet in keiner der beiden Welten; die **Kodierungsannahme
beim Patchen** ist das Gefährliche.

---

## Prüfung gegen die Architektur-Checkliste

Siehe `knowledge/architecture-checklist.md`. Beantwortet, nicht abgehakt.

### Proper architecture

**Bleibt die PWA parallel zu Legacy?** Ja, und Runde 4b stärkt die Legacy sogar: Der
Flashpfad bekommt dieselbe Prüfung wie die API, statt dass die Legacy-Seite
ungeschützt bleibt. Kein Schritt dieses Pakets entfernt Legacy-Funktion.

**Werden die Wetter-Endpunkte eingehalten?** Das Paket berührt die Wetterpfade
nicht. `/api/weather_get_now` und `/api/weather_get_forecast` bleiben unverändert;
kein Rückbau auf den entfernten Legacy-Bypass.

**Bleibt die Restore-Bedingung vollständig?** `pending_weather_ticker_restore` in
`src/main.c` wird nicht angefasst. Runde 4a Weg A berührt `src/**` gar nicht; Weg B
berührt den Variablen-Sendepfad, nicht die Display-Zustandsmaschine. Keine der vier
Teilbedingungen wird vereinfacht.

**Werden App-Assets ausschliesslich als `.gz` ausgeliefert?** Ja, unverändert. B7
verschärft die Gegenprobe sogar: Eine Datei der Grösse 0 wird PWA-seitig abgewiesen,
bevor sie hochgeladen wird — die ESP-Seite prüft das längst, die PWA-Seite nicht.
Nach Runde 2 ist `./tools/install-app.sh --check` zu fahren; die Weissliste
`APP_INSTALL_ASSETS` ändert sich nicht, es kommt kein Asset dazu.

**Greift die Änderung an der richtigen Schicht an?** Diese Frage hat das Paket an
drei Stellen geformt:

- C16 wird im **ESP** behoben, nicht in der PWA. Eine PWA, die „leere Antwort heisst
  Datei fehlt" annimmt, wäre eine Umgehung mit eingebauter Fehlinterpretation.
- Der Parametervertrag wird im **ESP** festgelegt, weil Legacy und Backup-Import
  durch dieselben Endpunkte gehen. Eine Prüfung nur in der PWA schützt zwei Drittel
  der Zugänge nicht.
- A6 wird im **ESP** geheilt, nicht in der PWA. Die PWA könnte den Verlust erkennen
  und anzeigen — das wäre eine Anzeige des Schadens, nicht seine Behebung.

Ein Punkt verdient die Gegenfrage: **C14 legt Diagnose in den Produktivpfad.** Das
ist bewusst und befristet — die Heap-Zeile trägt bereits die Rückbaupflicht aus
C9c4. Die Verlustzeile bleibt, weil sie keinen eigenen Takt hat, sondern nur im
Schadensfall feuert.

### Scalable systems

**Wie viele STM-Kommandos erzeugt die Aktion?** C14 erzeugt **keine zusätzlichen**.
Die beiden Zeilen gehen heute schon auf die Leitung; neu ist nur, dass sie
**zusätzlich** in einen lokalen Ring kopiert werden — eine Speicheroperation, kein
Byte mehr auf der UART.

Runde 4a Weg A erzeugt im Fehlerfall bis zu drei zusätzliche `IPADDRESS`-Meldungen,
und jede löst am STM den Vollabgleich mit rund 190 Kommandos aus. **Das ist die
teuerste Einzelposition des Pakets.** Deshalb die Begrenzung auf drei Versuche mit
Abstand, und deshalb die Bedingung: nur wenn der Satz nachweislich unvollständig
ist. Im Normalfall — vollständiger Satz nach dem ersten Hochlauf — kostet es **null**.

**Wie viele Byte pro Minute zusätzlich auf der UART?** C14: null. Runde 1: null, der
Fehlertext ersetzt einen Erfolgstext gleicher Grössenordnung. Runde 2 senkt die Last
sogar — B2 stoppt den Poller im Hintergrund, und jeder HTTP-Request kostet den STM
Debugtext (C2: rund 106 Byte je Request, am Gerät gemessen).

**Bleibt jeder Pfad unter 20 s Watchdog?** Keine neue Schleife, kein neues Warten.
`snprintf` und `stm32_log_append` sind konstanter Aufwand. Die Existenzprüfung in
`fs_show` kommt **vor** die Kopfzeilen und verlängert die Antwort nicht, sie
verschiebt nur die Reihenfolge. Runde 4a Weg A fügt keine Schleife hinzu, sondern
einen Zeitvergleich im Hauptloop — dieselbe Machart wie `esp_heap_log()`, mit
überlaufsicherer Differenzbildung.

**Wie lange blockiert der Hauptloop?** Unverändert. Die EEPROM-Schreibpfade
(16 ms/Byte) werden nicht angefasst.

**Hartkodierte Grenzen?** Zwei, beide benannt: der Ring mit 32 Zeilen zu 120 Zeichen,
und die Stackpuffer von 48 bzw. 40 Byte. Die Puffergrössen sind gegen die längste
mögliche Ausgabe gerechnet (37 bzw. 28 Zeichen), nicht geschätzt. `snprintf` kappt
auch dann sauber, wenn die Rechnung einmal nicht mehr stimmt.

### Secure by design

**`escapeHtml` bei jedem `innerHTML`?** Runde 2 und 3 fügen Text hinzu: die
Clamping-Meldungen (B6), den Hinweis zum unsicheren Kontext (B5), den Text „Logbuch
leer" (B17), die Warnung beim Modulwechsel (B18). Davon enthält **nur** die
Clamping-Meldung Fremddaten — die eingegebene Zahl. Sie geht über `textContent`,
nicht über `innerHTML`. Der Dateiname in der `fs_show`-Fehlermeldung ebenso.

**Fremddaten als nicht vertrauenswürdig?** Der `detail`-Text der neuen Fehlerantwort
läuft durch `http_send_json_escaped()`, wie bei `http_json_error()` heute schon. Der
Dateiname aus `fs_show` wird **nicht** in den `detail` übernommen — ein fester Text
„file not found" genügt, und damit entfällt die Frage nach seiner Maskierung ganz.

**Keine Credentials in Logausgaben?** Hier liegt die eine Stelle, die Aufmerksamkeit
braucht: C14 macht ESP-Zeilen über `/api/stm32_log` **ohne Anmeldung aus dem ganzen
LAN** lesbar. Die beiden Zeilen enthalten ausschliesslich Zahlen — freier Heap,
grösster Block, verlorene Byte. Kein Name, kein Wert, kein Geheimnis.

**Das ist kein Nebensatz.** L115 belegt den Gegenfall: Die Timeout-Meldung des STM
loggt das vollständige Kommando und damit den Wetter-Schlüssel, die Update-Quelle
und den Zeitserver im Klartext — über denselben Ring, aus demselben LAN lesbar. Wer
künftig eine weitere ESP-Zeile in den Ring legt, prüft zuerst, was sie enthält.
Dieses Paket behebt L115 nicht (A19/A20), aber es verschlimmert es auch nicht.

**Neue Endpunkte, die Konfiguration schreiben?** Keine. Runde 1 ändert
Antwortverhalten, Runde 4b verschärft eine bestehende Prüfung. Der Fehlercode 6 ist
eine Erweiterung des Vertrags, keine neue Reichweite.

### Stable & reliable

**Wird jeder Fehler ausgewertet?** Das ist der Kern von Runde 1: Eine Erfolgsmeldung
muss vom tatsächlichen Ergebnis abhängen. `{"ok":true}` nach stillem Klemmen ist
genau die Erfolgsmeldung, die es nicht tut. Gleiches bei C16 — eine leere Antwort
ist heute weder Erfolg noch Fehler, sondern nichts.

**Leere `catch`?** B17 beseitigt eine Stelle dieser Gattung: Der verschluckte Fehler
in `fetchStm32Log(true)` ist eine der drei offenen Spuren, und unabhängig davon,
welche zutrifft, ist ein leeres Feld ohne Meldung der falsche Zustand.

**Stille Verwerfungen ohne Zähler?** C14 behebt das Gegenstück: Die Verwerfung
**hat** einen Zähler (`write_lost_blocks`), und sie **hat** eine Zeile — nur war die
Zeile nicht erreichbar. Nach Runde 0 ist beides sichtbar.

**Ist der Zustand nach Abbruch mitten in einer Sequenz definiert?** Zwei Stellen:

- `fs_show`: Nach dem Umbau wird entweder eine JSON-Fehlerantwort **oder** ein
  `text/plain`-Rumpf gesendet, nie beides und nie nichts. `LittleFS.end()` läuft auf
  jedem Pfad, auch dem Fehlerpfad.
- Runde 4a Weg A: Schlagen alle Versuche fehl, bleibt ein definierter Zustand —
  unvollständiger Satz **mit** Meldung statt unvollständiger Satz ohne. Der
  Versuchszähler wird beim Erfolg zurückgesetzt, sonst heilt sich das Gerät genau
  einmal je Lebenszeit.

**Wird ein gesetztes Flag auf jedem Pfad wieder aufgelöst?** Drei Flags im Spiel, und
alle drei sind in diesem Paket genannt: `hasUnsavedEdits` (B1 — Reset auf den
Speicherpfaden ausserhalb des Trichters), `stm32LogRefreshInFlight` (B17 — Reset im
`finally`, nicht nur im Erfolgszweig) und `http_write_broken` (C14 — unverändert, die
neue Zeile liegt **innerhalb** seines Schutzes, nicht daneben).

---

## Verworfene Alternativen

**C14: eine eigene Ringstruktur für ESP-Zeilen.** Verworfen. Ein zweiter Ring kostet
BSS, und genau dort wurde gerade gespart: Der STM-Ring ging von 64 auf 32 Zeilen,
weil der grösste zusammenhängende Block das Problem ist (L175, L176). Ein zweiter
Ring gäbe das Ersparte zurück. Dazu bräuchte `/api/stm32_log` eine zweite Quelle und
eine Sortierung nach Zeit, die niemand hat.

**C14: den Ring wieder vergrössern, damit beides passt.** Verworfen aus demselben
Grund. Die Verkleinerung ist eine gemessene Massnahme gegen einen belegten
Absturzpfad; sie jetzt für Diagnosekomfort zurückzunehmen hiesse, den Befund gegen
sein eigenes Werkzeug zu tauschen.

**C14: das Präfix `esp: ` statt `- `.** Verworfen, solange die Prüfung aus dem
Lösungsweg (`grep -a` über `src/**`) keinen STM-Logtext mit `- ` findet. `- ` ist im
Protokoll bereits die Kennzeichnung für ESP-Zeilen (`src/esp8266/esp8266.c:332`), und
eine zweite Konvention für dieselbe Sache ist eine, die man verwechselt.

**Strang 1: klemmen, aber den geklemmten Wert in der Antwort mitgeben.** Reizvoll —
die Oberfläche wüsste Bescheid, und alte Clients liefen weiter. Verworfen, weil es
drei Verhalten statt zwei schafft (abweisen, klemmen-und-melden,
klemmen-und-schweigen) und weil `{"ok":true}` dann zwei Bedeutungen hätte. Der Befund
L186 ist genau diese Mehrdeutigkeit. Der Backup-Import braucht es nicht: Nach L65
meldet die PWA bereits selbst, was sie zurechtgebogen hat, und sendet dann einen
gültigen Wert.

**C16: HTTP 404 statt `{"ok":false}`.** Verworfen. `http_json_error()` sendet
bewusst HTTP 200 — ein echter Fehlerstatus erscheint in der Legacy-Oberfläche als
Verbindungsabbruch statt als Ursache; der Kommentar an `http.cpp:10141-10144` sagt
das. Die Unterscheidung läuft über den `Content-Type`, nicht über den Statuscode.

**C16: Erfolgsfall ebenfalls auf JSON umstellen.** Verworfen. Das brächte die
Maskierung beliebiger Dateiinhalte mit sich, auf einem Pfad, der heute zeichenweise
sendet. Viel Risiko für Formschönheit.

**Runde 4a: den Verlust in der PWA erkennen und anzeigen.** Verworfen. Es ist eine
Anzeige des Schadens, keine Behebung, und es widerspricht Punkt 1 der Checkliste —
ein Problem, dessen Ursache auf dem ESP liegt, wird nicht in der PWA umgangen.

**Runde 4a: `var_send_all_variables()` robuster machen.** Richtig, aber nicht hier.
Das ist A1 und A17 mit eigener Spec und eigener Geräteverifikation; der Pfad hat rund
190 quittungspflichtige Kommandos ohne `watchdog_reload()`. In einer ohnehin
riskanten Runde zusätzlich das Zeitverhalten des Startpfads zu ändern, macht einen
Fehlschlag unzuordenbar.

**Runde 2 und 3 in einer Runde.** Verworfen trotz disjunkter Dateien. R3b: Besitz
ist nicht Reihenfolge. Der Kachelumbau verschiebt sechs Elemente im DOM; läuft er mit
zwölf Logikänderungen zusammen, ist eine Regression nicht mehr zuzuordnen — und
genau diese Zuordenbarkeit war am 03.10.2026 die Zusage, die verlorenging.

**Runde 4a und 4b in einer Runde.** Verworfen aus demselben Grund, verschärft durch
C13: Beide sind ESP-Änderungen, beide kommen per OTA. Scheitert der OTA oder stürzt
das Gerät danach ab, wäre nicht zu sagen, welche der beiden es war.

---

## Versionsfolgen

Je Runde, nicht einmal für das ganze Paket. Kein Gleichschritt — es steigt nur, was
sich geändert hat (DIR-004).

| Runde | STM `src/main.h` | ESP `version.h` | `APP_VERSION` | `CACHE_NAME` |
|---|---|---|---|---|
| 0 | — | ☐ | — | — |
| 1 | — | ☐ | ☐ | ☐ |
| 2 | — | — | ☐ | ☐ |
| 3 | — | — | ☐ | ☐ |
| 4a (Weg A) | — | ☐ | — | — |
| 4a (Weg B) | ☐ | ☐ | — | — |
| 4b | — | ☐ | — | — |

`APP_VERSION` und `CACHE_NAME` gehören **immer** zusammen.

Ausgeführt wird das ausschliesslich vom `release-engineer`, beauftragt vom Lead
(R4). Kein Teammate bumpt.

**Nach jeder Runde mit PWA-Anteil:** `./tools/install-app.sh --check`, dann
`./tools/install-app.sh`. Der Rollout auf die Synology bringt die Assets nur auf den
Update-Server, nicht auf die Uhr.
