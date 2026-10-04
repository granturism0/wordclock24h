# Anforderungen — Paket 2026-10-05

**Status:** Entwurf — **nicht freigegeben**
**Erstellt:** 2026-10-04. **Fortgeschrieben am 2026-10-05:** die vier Entscheidungen
des Nutzers und der A35-Nachweis (L266), der den Entwurf von A35 Teil 1 umgeworfen hat.
**Auslöser:** Arbeitsliste in `BEFUNDE.md` (über hundert Einträge), ausdrücklicher
Wunsch des Nutzers nach einem **grösseren** Paket, und die drei Vorfälle vom
04.10.2026, in denen erledigte Arbeit als offen mitgeschleppt wurde (L173, L235,
L248).

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Es wird nach der Freigabe nicht
mehr fortgeschrieben; der lebende Stand steht in `BEFUNDE.md`.

---

## Überblick — acht Runden, in dieser Reihenfolge

| Runde | Inhalt | Laufzeit | Einspielen |
|---|---|---|---|
| **V** | **Nachzählen** — trifft der Befund überhaupt noch zu? | keine | — |
| **W** | Werkzeug und Verfahren (E19, E20, E15, B22, Prüfstände, S11) | keine | — |
| **E1** | ESP: der reproduzierbare Absturz (L174) und die zwei unerreichbaren Beobachtungszeilen (C14) | ESP | OTA |
| **E2** | ESP: Eingang der Brücke und Parametervertrag (A37, C24, C22, C20, C17, C18) | ESP | OTA |
| **H** | **STM: Eingang härten** (A41/L265) — das Gegenstück zu A37 | STM | STM-Flash |
| **S** | Brücke fertigbauen (L260, A35 Teil 1, C26, A39, A16) | ESP **und** STM | OTA, **dann** STM |
| **P** | PWA (B33, B24) | PWA | LittleFS-Upload |
| **U** | UI (B30, B28) | PWA | LittleFS-Upload |
| **F** | Flash-Überwachung — Runde 4b aus dem alten Paket (C9c6/L179) | ESP + Werkzeug | OTA, dann ein STM-Flash |

**Runde V steht vor allem anderen.** Sie kann den Umfang jeder folgenden Runde
verkleinern — und genau darum geht es.

---

## Problem

### 1. Befunde werden als offen mitgeschleppt, obwohl sie erledigt sind

Am 04.10.2026 ist das **dreimal** aufgetreten, in drei verschiedenen Gestalten:

| Fall | Was | Warum keine Prüfung es fand |
|---|---|---|
| **L235** | 28 ToDo-Einträge waren erledigt — über ein Fünftel der Arbeitsliste | Das Muster der Prüfung erfasste Kennungen mit Buchstabensuffix nicht (`B1` ja, `B1b` nein). Die Stufe lief, meldete OK und sah den grössten Teil ihres Gegenstands nicht |
| **L248** | Fünf weitere Einträge verwiesen auf **Massnahmen** statt auf Befunde, und deren Stand in den Review-Tabellen war selbst veraltet | Die Prüfung war korrekt und verglich gegen veraltete Daten |
| **L173** | Ein Befund stand über **sechs Releases** als „offen, KRITISCH" und war beiläufig mitrepariert — am Gerät nachgemessen: 720 von 720 Zeichen, vollständig | **Keine Prüfung kann das finden.** Ob ein Befund noch zutrifft, steht in keinem Dokument, sondern nur im Code oder am Gerät |

**Die Vorprüfung beim Schreiben dieser Spec hat das bestätigt, bevor die Runde
überhaupt lief — drei Treffer in einer halben Stunde, alle drei inzwischen
nachgezählt, geschlossen und aus der Arbeitsliste gestrichen:**

- **A32 / L230** stand als „offen, KRITISCH, die Wurzel hinter A6, L42, L103 und
  L205" in der Liste und **blockierte dort Runde 4a**. Im Baum stehen
  `var_retry_slots`, `var_retry_ok_cnt`, `var_retry_gaveup_cnt`
  (`src/vars/vars.c:245 ff.`); L254 belegt die Wirkung am Gerät. **Geschlossen.**
- **A38 / L255** nannte als offene Entscheidung, ob der neue Zweig `is_online`
  mitsetzt. `src/esp8266/esp8266.c:452` setzt es, mit Begründung im Kommentar
  darüber. **Geschlossen.**
- **A36 / L236** stand als „höchste Priorität der Gruppe" in der Liste, während L249
  denselben Befund als behoben meldete — **zwei Zeilen desselben Dokuments
  widersprachen sich. Geschlossen.**

**Das ist der Beleg, dass der Durchgang trägt**, und zugleich der Grund, warum er als
eigene Runde geführt wird und nicht nebenbei läuft: Ohne ihn stünde in dieser Spec
eine eigene A32-Runde, die nichts zu tun hätte.

Dazu der Abschnitt „Stand des grossen Pakets" in `BEFUNDE.md`: Er führt Runde 4a als
**„noch nicht begonnen, blockiert durch A32"**. L261 belegt Weg B am Gerät, und
`src/main.c:2974` sowie `:3655` zeigen den gebauten Zweig. **Der Standabschnitt ist
selbst veraltet** — und er ist diejenige Stelle, aus der die Planung ihren Umfang
nimmt.

### 2. Die Brücke ist halb fertig

A32 (Nachsendung und Prüfsumme) und Weg B (Vollabgleich auf Anforderung) stehen am
Gerät. Vier benannte Lücken bleiben, alle aus derselben Arbeit heraus gefunden:

- **A39 / L259** — `schedule_esp8266_messages()` wird auch aus der Warteschleife von
  `var_send_buf()` gerufen (`src/vars/vars.c:586`). Trifft eine `IPADDRESS`-Zeile
  dort ein, läuft `var_send_all_variables()` **verschachtelt**, und alle rund 194
  Kommandos gehen ohne eine einzige Quittung hinaus — wörtlich der Schaden aus L102
  (`d=5148`). Der `SYNCVARS`-Zweig ist bereits geschützt (`src/main.c:2974`), der
  `IPADDRESS`-Zweig nicht (`src/main.c:2926`).
- **L260** — Das Erfolgskriterium von Weg B ist `HARDWARE_CONFIGURATION != 0xFFFF`,
  und dieses Kommando ist das **dritte** von rund 194. Kommen 1 bis 3 an und 4 bis
  194 nicht, hält der ESP sich für geheilt und fragt nicht nach.
- **C26 / L253** — Die Prüfsumme lässt 19,1 % der beschädigten Zeilen durch: Der
  Burst nimmt das Zeilenende samt Marke weg, die Zeile sieht unmarkiert aus und wird
  wie bisher angenommen. Gemessen, nicht geschätzt (554'121 Zeilen).
- **A35 / L233** — **Der ESP quittiert, der STM wertet aus**
  (`ESP-uclock.ino:886` sendet den Punkt, `src/esp8266/esp8266.c:287` liest ihn). Die
  Quittung bestätigt den **Empfang**, nicht die Übernahme, und sie ist ein **nackter
  Punkt ohne Zuordnung**: Eine verspätete Quittung bestätigt das **nächste**
  Kommando. A32 hat das Problem nicht nur offengelassen, sondern **vergrössert** —
  Nachsendungen machen verspätete Quittungen häufiger.

### 3. Dasselbe Messgerüst wurde dreimal von Hand nachgebaut

`tools/preview/diag.js` misst Überlauf, Textbeschnitt, Touchgrössen, Fokus, Modal und
einen Kontrastwert. Es misst **keine** Kachelgeometrie, **keine**
Ankreuzfeld-Geometrie, **keine** Farbe am Bildpunkt und nimmt **nicht** die ganze
Seite auf. Innerhalb eines Tages ist das dreimal aufgelaufen:

- **L228** — Die Zahlen hinter AK3.1 (Kachelhöhen) liessen sich nur mit einem selbst
  gebauten Messproxy erheben.
- **L243** — `shot.sh --module` gibt es seit demselben Tag (L239), **aber es scrollt
  nicht**; die TFT- und Favoritenfelder liegen weit unterhalb des Seitenkopfs.
- **L245** — Die Entscheidung über die Steuerfarbe fiel erst, nachdem jemand über
  7 Viewports × 4 Module × 2 Zustände **am Bildpunkt** gemessen hatte. Vier
  Fehlrechnungen davor hatten alle dieselbe Ursache: aus dem Stylesheet gerechnet
  statt im Bild gelesen.

Alle drei Gerüste verfielen danach im Scratchpad. Dazu zwei Werkzeugbefunde
derselben Gattung: **E20 / L258** (der Besitz-Hook blockiert nach Schreibweise statt
nach Absicht und lässt sich durch Zusammensetzen des Pfades trivial umgehen) und
**E15 / L189** (der Gefahren-Hook weist schon eine `grep`-Suche in einer lokalen
Logdatei ab).

### 4. Der ESP stürzt auf einem bekannten, reproduzierten Weg ab

**L174** — `GET /api/network_scan` sendet erst Kopfzeilen und erste Felder und ruft
**danach** `WiFi.scanNetworks()`, mitten in der laufenden Antwort. Zweimal
aufgetreten, **dieselbe Aufruferadresse `0x4021e571`, derselbe Abstand von 1,6 s**,
beide Male `Unhandled C++ exception: OOM`. Die PWA ruft den Scan bei **jedem** Wechsel
ins Netzwerk-Modul (`loadSecondaryData()`), was „Absturz beim Durchklicken der
Reiter" vollständig erklärt. Der Nutzer umgeht das heute, indem er den Reiter meidet.

### 5. Der Eingang der Brücke ist auf beiden Seiten ungeprüft — und nur eine Seite zu härten wiederholt L179

Auf jedes `htoi (x, n)` folgt ein **unbedingtes** `x += n`. Stand der Zeiger schon auf
dem Terminator, zeigt er danach dahinter, und der nächste Lesezugriff geht über die
Pufferkante.

| Seite | Stellen | Befund |
|---|---|---|
| ESP, `vars.cpp` | rund **45** | **A37 / L237** |
| STM, `main.c` (39), `tables.c` (22), `esp-spiffs.c` (4), `tftled.c` (1) | rund **66** | **A41 / L265** |

Am exponiertesten sind `tftled.c:122` und `esp-spiffs.c:206-231`: Dort läuft direkt
hinter dem Vorrücken eine `while (*p)`-Schleife, die dann über die Pufferkante liest.

**Der Auslöser ist nicht theoretisch.** Eine verkürzte Kommandozeile entsteht genau
dann, wenn die Brücke ein Zeichen verliert — und das messen wir täglich über `d=`.
Nur eine Seite zu härten, erzeugt genau das Muster, das L179 bestraft hat: zwei Wege
zum selben Ziel, einer mit Schutz, einer ohne.

### 6. Runde 4b ist nie gefahren worden

**C9c6 / L179** — Der Legacy-Zweig übernimmt den Flash-Dateinamen ungeprüft
(`http.cpp:6909-6913`), der API-Endpunkt prüft über
`http_remote_stm32_filename_matches()`. **Der ungeschützte Weg hat die Uhr am
04.10.2026 stillgelegt**, weil ein F103-Abbild auf dem F411 landete. Die Spec dazu
steht fertig in `specs/grosses-paket-2026-10-04/`, Runde 4b, und ist nie umgesetzt
worden.

---

## Ziel

Nach diesem Paket gilt:

1. Jeder als **KRITISCH** geführte Befund ist **nachgezählt** — bestätigt oder
   geschlossen, in beiden Fällen mit Beleg und Datum. Die Arbeitsliste bildet den
   Stand ab, den Code und Gerät zeigen, und nicht den, den sie einmal zeigten.
2. Die Brücke STM↔ESP hat kein halbes Protokoll mehr: kein verschachtelter
   194er-Stoss, ein Erfolgskriterium, das den **ganzen** Satz prüft, eine Quittung mit
   Zuordnung, und eine Markenpflicht für die drei Werte, bei denen ein still falscher
   Wert teuer ist.
3. **Beide** Enden der Brücke prüfen die Länge der Kommandozeile, bevor sie sie
   zerlegen.
4. Das Messgerüst für UI-Zahlen steht im Werkzeug statt im Scratchpad und läuft
   **sowohl** gegen die Vorschau **als auch** gegen das Gerät.
5. Der reproduzierte Absturz über `network_scan` tritt nicht mehr auf.
6. Der Legacy-Flashpfad kann kein fremdes Abbild mehr aufspielen.

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument**. Zwei Fallen sind bei jedem Kriterium
geprüft worden:

- **Die L177/L185-Falle** — eine Abnahme über einen Weg, den die Meldung nie nimmt.
  Die Heap-Zeile wurde zweimal in zwei Tagen gebaut und war beide Male unerreichbar,
  weil sie am API-Ring vorbeilief. Jedes Kriterium unten nennt deshalb den **Weg**,
  auf dem die Meldung ankommt, nicht nur den Wert.
- **Die L256-Falle** — ein Prüfstand auf dem Entwicklungsrechner ist nicht die
  Zielplattform. Wo eine Typbreite den Unterschied macht (`unsigned long` ist auf dem
  Host 64 Bit, auf dem ESP 32), ist sie anzugleichen, und der Prüfstand läuft in
  **beiden** Übersetzungen.

### Runde V — Nachzählen

- [ ] **AKV.1** — Jeder Befund, der zu Beginn des Pakets den Status **KRITISCH**
      trägt, hat in `BEFUNDE.md` einen Nachzählvermerk der Form
      `nachgezaehlt: JJJJ-MM-TT · <Verdikt> · <Beleg>`. **Auch bei Bestätigung** —
      sonst ist nicht unterscheidbar, ob nachgesehen wurde oder nichts passiert ist
      (die Unterscheidung aus L261).
      *Instrument:* `BEFUNDE.md`, abzählbar; Gegenprobe über Guardrail-Stufe S11
      (AKW.6).
- [ ] **AKV.2** — Jeder Nachzählvermerk trägt **eines von drei Verdikten**, und kein
      anderes: `bestaetigt`, `geschlossen` oder `praemisse-widerlegt`.
      **„nicht reproduziert" ist kein Verdikt.** Ein sporadischer Befund bleibt offen,
      solange sein Mechanismus im Code steht — dieselbe Unterscheidung, an der die
      Bewertung von L226, L229 und L232 am 04.10.2026 mehrfach gescheitert ist.
      *Instrument:* `BEFUNDE.md`, Textprüfung in S11.
- [ ] **AKV.3** — Jedes `geschlossen` nennt **Datei:Zeile dieses Commits** oder einen
      **Messwert mit Zeitstempel und Instrument**. Ein Verweis auf einen anderen
      Befund („L249 sagt behoben") genügt **nicht** — genau diese Kette hat bei A36 zu
      zwei widersprechenden Zeilen im selben Dokument geführt.
      *Instrument:* Stichprobe von fünf `geschlossen`-Zeilen durch den Lead, jede am
      Quelltext nachvollzogen.
- [ ] **AKV.4** — Der Abschnitt „Stand des grossen Pakets" in `BEFUNDE.md` ist Task
      für Task nachgezählt und danach entweder **fortgeschrieben** oder **aufgelöst**.
      Er verschwindet nur, wenn jeder seiner Tasks ein Verdikt trägt.
      *Instrument:* `BEFUNDE.md`; `specs/grosses-paket-2026-10-04/tasks.md` als
      Gegenliste.
- [ ] **AKV.5** — Die Arbeitsliste enthält nach dem Durchgang **keinen** Eintrag, der
      ausschliesslich auf Erledigtes verweist, und **jeder** offene Befund steht darin.
      *Instrument:* Guardrail S10, beide Richtungen.
- [ ] **AKV.6** — Für jeden Eintrag der Arbeitsliste, dessen genannte Dateien seit dem
      Datum seines Belegs **geändert** wurden, liegt ein Nachzählergebnis vor. Für die
      übrigen ausdrücklich **nicht** — der Durchgang ist endlich, nicht vollständig.
      *Instrument:* `git log -- <datei>` gegen das Belegdatum, Liste aus Task V.5.

### Runde W — Werkzeug und Verfahren

- [ ] **AKW.1** — Die Geometriemessung liegt als **ein** Modul vor, das sowohl
      `tools/preview/diag.js` (Vorschau) als auch `tools/check-pwa.mjs` (Gerät)
      ausführen. Sie liefert je Panel Höhe und Breite, je Ankreuzfeld Geometrie und
      gerenderte Farben **am Bildpunkt**, und sie scrollt.
      *Instrument:* Ein Lauf gegen die Vorschau und **ein Lauf gegen das Gerät**,
      beide mit Ergebnisdatei. Damit ist die Lücke aus L229 geschlossen —
      Vorschauzahlen und Gerätezahlen stehen nebeneinander.
- [ ] **AKW.2** — Die Zahlen aus L227 (302 / 406 / 594 px) lassen sich mit dem
      Werkzeug **ohne selbstgebautes Gerüst** reproduzieren, Abweichung ≤ 2 px.
      *Instrument:* Vorschaulauf über die Breiten 390, 600, 768, 900, 1240, 1512 px.
      Das ist zugleich die Eichung — reproduziert das Werkzeug die alten Zahlen nicht,
      misst es etwas anderes.
- [ ] **AKW.3** — `tools/hooks/file-ownership.py` wertet bei `Bash` nur Pfade in
      **Schreibposition** (Ziel von `>`, `>>`, `open(…,'w')`, `tee`, `sed -i`,
      `mv`/`cp`-Ziel). Ein fremder Pfad im reinen Meldungstext blockiert nicht mehr.
      *Instrument:* Zwei Gegenproben, beide Pflicht (DIR-014): ein Befehl, der einen
      fremden Pfad nur im Text führt, läuft durch; ein Befehl, der in eine fremde
      Datei schreibt, wird abgewiesen.
- [ ] **AKW.4** — `tools/hooks/no-danger.py` weist eine `grep`-Suche in einer lokalen
      Datei **nicht** mehr ab, einen Aufruf gegen das Gerät weiterhin schon.
      *Instrument:* dieselben zwei Gegenproben.
- [ ] **AKW.5** — Die Prüfstände zu A37 (Runde E2) und A41 (Runde H) liegen unter
      `tools/checks/` und nicht im Scratchpad, und sie laufen in **zwei**
      Übersetzungen: einmal mit den Typen des Hosts, einmal mit der mechanischen
      Angleichung an die Zielplattform (L256).
      *Instrument:* beide Läufe, beide grün, und der Nachweis, dass die angeglichene
      Übersetzung den Fall **herstellt**, den sie messen soll.
- [ ] **AKW.6** — Guardrail-Stufe S11 meldet jeden Befund mit Status **KRITISCH**,
      dessen Nachzählvermerk fehlt **oder** älter ist als der jüngste Commit auf eine
      der von ihm genannten Dateien.
      *Instrument:* Die Stufe muss **einmal fehlgeschlagen** sein (DIR-014), und es
      ist nachgewiesen, dass sie **ihren ganzen Gegenstand sieht** — die Ergänzung aus
      L235. Nachweis: Die Zahl der geprüften Befunde wird gemeldet und von Hand gegen
      die Tabelle abgezählt.

### Runde E1 — ESP: Absturz und Beobachtbarkeit

- [ ] **AKE1.1** — `WiFi.scanNetworks()` wird **vor** der ersten gesendeten Kopfzeile
      gerufen. Die Netzliste wird geströmt, nicht als `String` je Eintrag gebaut.
      *Instrument:* Quelltext, per `grep` nachgewiesen — die Reihenfolge von
      `scanNetworks` und dem ersten Sendeaufruf.
- [ ] **AKE1.2** — Zehn aufeinanderfolgende Aufrufe von `/api/network_scan` während
      eines **vollen Seitenaufbaus** erzeugen keine Exception und keinen Neustart.
      *Instrument:* `./tools/check-pwa.sh` mit geöffnetem Netzwerk-Modul, **und**
      `./tools/watch-log.sh` läuft mit und wird mitgelesen (DIR-013). **Das ist der
      Weg, auf dem die Meldung ankommt** — ein Absturz steht im seriellen Mitschnitt,
      nicht in der API-Antwort. Eine Abnahme allein über `curl` hat am 04.10.2026
      dreimal nichts gefunden (L174: „vier parallele Anfragen stürzen nicht ab"), weil
      `curl` den Browser nicht nachbildet.
- [ ] **AKE1.3** — Die Heap-Zeile und die Verlustzeile erscheinen in
      `/api/stm32_log`.
      *Instrument:* der Endpunkt selbst. **Das ist genau die L177/L185-Falle:** Beide
      Zeilen wurden bereits gebaut und liefen am Ring vorbei; eine Abnahme über den
      seriellen Mitschnitt hätte sie als vorhanden gemeldet, obwohl sie über die
      Oberfläche unerreichbar waren.
- [ ] **AKE1.4** — Die Runde **verschlechtert den Heap nicht**: `free_heap` und
      `max_free_block` liegen nach der Runde nicht unter den Werten davor.
      *Instrument:* `/api/device_ready`, gemessen unter **derselben Bedingung** wie
      die Vergleichsmessung (rund 20 Minuten nach dem Flash, Ruhebetrieb ohne offene
      Browsersitzung). Zwei Messungen unter verschiedenen Bedingungen zu vergleichen
      ist der Fehler aus L262 — dort erklärte sich der ganze Unterschied aus einer
      offenen PWA.

### Runde E2 — ESP: Eingang der Brücke und Parametervertrag

- [ ] **AKE2.1** — Eine Kommandozeile, die vor dem Ende ausgeht, führt an **keiner**
      der rund 45 Stellen zu einem Lesezugriff hinter dem Terminator.
      *Instrument:* Prüfstand mit **Schutzseite** nach dem Verfahren aus L249 — die
      echte Funktion eingebunden, nicht abgeschrieben, Eingabe so gelegt, dass ihr
      Terminator auf dem letzten Byte einer Seite steht, dahinter `PROT_NONE`. Die
      alte Fassung muss `SIGBUS` erzeugen, die neue nicht. **Eine Abnahme über die
      Ausgabe genügt nicht** — L249 hat gezeigt, dass der Fall `0xC2` harmlos
      *aussieht*, weil die gelesene Null im Ziel landet.
- [ ] **AKE2.2** — `parse_json()` schneidet die Beschreibung an der **Zeichengrenze**
      ab, nie mitten in einem Mehrbyte-Zeichen.
      *Instrument:* derselbe Prüfstand, mit einer Beschreibung, deren 31. Byte in der
      Mitte eines Zeichens liegt.
- [ ] **AKE2.3** — Alle sieben Zeichenkettenfelder weisen einen zu langen Wert mit
      `{"ok":false,"error":2,...}` ab und **kürzen nicht**. Der gespeicherte Wert
      bleibt unverändert.
      *Instrument:* Gerätetest **nur nach dem Einspielen** und **nur mit Freigabe**.
      Vor der Änderung darf dieser Test nicht laufen: `update_host_set` mit einem zu
      langen Wert würde heute still kürzen und die Update-Quelle auf einen anderen
      Server zeigen — das war L124. Gegenprobe danach über
      `./tools/check-update-source.sh`.
- [ ] **AKE2.4** — `fs_remove` auf eine nicht vorhandene Datei und `dfplayer_play`
      ohne Parameter melden **nicht** mehr Erfolg.
      *Instrument:* `/api/fs_remove?filename=<nicht vorhanden>` — unschädlich, weil es
      nichts zu löschen gibt; **kein** Aufruf mit einem vorhandenen Namen.
- [ ] **AKE2.5** — `dfplayer_alarm_set` weist `idx` ausserhalb des Bereichs,
      `from`/`to` ausserhalb von 0..7 und `hour`/`minute` ausserhalb von 0..23 / 0..59
      ab.
      *Instrument:* Gerätetest **nur nach dem Einspielen**, mit Freigabe. **Vor** der
      Änderung erzeugt derselbe Aufruf einen Alarm, der als aktiv in der Liste steht
      und nie auslösen kann — das ist der Befund selbst. Nach der Änderung ist der
      Aufruf wirkungslos; Gegenprobe über `./tools/diff-snapshot.sh --soll`.
- [ ] **AKE2.6** — Alle 34 Abweisungen nennen den Bereich in der Form
      `<param> out of range (<min>..<max>)`. Die zwei Grenzen, die Laufzeitwerte sind,
      werden mit `snprintf` in einen Stackpuffer formatiert, nicht über `String`.
      *Instrument:* `grep` über die Fehlertexte; Stichprobe von fünf Endpunkten am
      Gerät, alle fünf lesend abweisend.
- [ ] **AKE2.7** — Die PWA zeigt die neuen Abweisungen an, statt sie zu verschlucken.
      *Instrument:* Prüfung der Auswertepfade in der PWA-Hauptdatei durch den
      `code-reviewer`, **vor** dem Einspielen. Findet er eine Stelle, die nur auf
      `ok` prüft, wird daraus ein Task in Runde P — keine stille Mitkorrektur.

### Runde H — STM: Eingang härten

- [ ] **AKH.1** — Eine verkürzte Kommandozeile führt an **keiner** der rund 66
      Stellen zu einem Lesezugriff hinter dem Terminator. Die Prüfung steht **einmal
      vorn**, nicht 66-mal einzeln.
      *Instrument:* Prüfstand mit Schutzseite unter `tools/checks/`, nach demselben
      Verfahren wie AKE2.1, **mit angeglichenen Typen** (L256). Die beiden
      exponierten Stellen `tftled.c:122` und `esp-spiffs.c:206-231` sind als eigene
      Fälle aufgeführt, weil dort hinter dem Vorrücken eine `while (*p)`-Schleife
      läuft.
- [ ] **AKH.2** — Nach dem Flash läuft ein **vollständiger Vollabgleich** (rund 194
      Kommandos) durch den neuen Parser, ohne Fehlverhalten.
      *Instrument:* ESP-Neustart auslösen, dann die Erfolgszeile des Vollabgleichs im
      Logring; `d=` und die Verlustzähler vorher/nachher; Diagnosefolge lückenlos;
      `watch-log.sh` mitgelesen. **Das ist die Abnahme, die zählt** — ein Prüfstand
      beweist die Funktion, nicht ihre Einbettung.
- [ ] **AKH.3** — Die Anzeige ist nach dem Flash unverändert: Uhrzeit, Layout,
      Temperatur, TFT-Zustand.
      *Instrument:* `./tools/diff-snapshot.sh --soll` gegen
      `tools/snapshots/soll-2026-10-04`, und ein Blick auf die Uhr. `tftled.c` und
      `esp-spiffs.c` sind Anzeige- und Dateipfade; ein Fehler dort zeigt sich am
      Gerät, nicht in der API.
- [ ] **AKH.4** — Eine abgewiesene zu kurze Kommandozeile wird **gezählt und einmal
      protokolliert**, nicht wortlos verworfen.
      *Instrument:* Quelltext und der Logring. Eine Härtung, die still verwirft,
      tauscht einen Absturz gegen ein unerklärliches Nichtverhalten.

### Runde S — Brücke fertigbauen

- [ ] **AKS.1** — `src/main.c:2926` ruft `var_send_all_variables()` **nicht** mehr
      direkt. Der `IPADDRESS`-Zweig merkt vor; gesendet wird im Hauptloop.
      *Instrument:* Quelltext.
- [ ] **AKS.2** — Während eines ESP-Neustarts mit IP-Meldung bricht der
      Hauptloop-Zähler zwischen zwei Diagnosezeilen **nicht** ein, und der IP-Lauftext
      läuft **vollständig** durch.
      *Instrument:* Diagnosezeile im Mitschnitt, Referenz ist der in L226 gemessene
      Ruhewert von rund 1'482'480 Durchläufen je 10-Sekunden-Fenster; zulässig ist
      derselbe Einbruch wie beim Datums-Overlay, rund 2 %. Dazu die Beobachtung am
      Display — **der Lauftext ist der Teil, den nur das Auge abnimmt.**
- [ ] **AKS.3** — Der ESP hält den Vollabgleich erst dann für vollständig, wenn die
      **Abschlussmarke** eingetroffen ist, nicht schon beim dritten von 194 Kommandos.
      Hat er **keine Eröffnungszeile** gesehen, gilt unverändert das heutige
      Kriterium.
      *Instrument:* Quelltext **und** die Erfolgszeile im Logring — nach dem Muster
      von L261: `var sync: Variablensatz vollstaendig, keine Anforderung noetig`
      erscheint erst nach der Marke. Die Zeile kostet 0 Byte auf der Brücke, weil
      `stm32_log_append()` nur in den Ring schreibt.
- [ ] **AKS.4** — Die Zuordnung steht als **eigene Zeile vor** der Quittung
      (`ACK <xy>`, dann `.` bzw. `!v`). Die Quittung selbst bleibt **unverändert** ein
      nackter Punkt.
      *Instrument:* Quelltext beider Seiten. **Das ist das Ergebnis des Nachweises,
      den der Nutzer verlangt hat (L266)** — eine an den Punkt angehängte Zuordnung
      (`.c3`) fällt bei `esp8266.c:287` durch die ganze Präfixkette, `var_send_buf()`
      läuft in den 3-s-Timeout, `got_answer` bleibt 0 und damit auch
      `watchdog_reload()` aus: **Reset nach rund sieben Kommandos.**
- [ ] **AKS.5** — Eine Quittung, die nicht zum ausstehenden Kommando gehört, wird
      **verworfen** statt angenommen; das ausstehende Kommando läuft in seinen Timeout
      und wird von A32 nachgesendet. Eine gemerkte Zuordnung ohne folgende Quittung
      wird **verbraucht**, nicht auf das nächste Kommando übertragen.
      *Instrument:* Prüfstand auf dem Host mit angeglichenen Typen (L256): verspätete
      Quittung einspielen, `var_retry_ok_cnt` muss steigen, und das Kommando muss
      **einmal** wiederholt werden, nicht endlos.
- [ ] **AKS.6** — **Beide Rückfalllagen laufen wie heute**, ohne Fähigkeitsmeldung:
      Ein alter STM verwirft die Zuordnungszeile und liest den Punkt unverändert als
      `OK`; ein neuer STM ohne Zuordnungszeile wertet den Punkt wie bisher. Ebenso
      beim Vollabgleich: kein Eröffnungssatz ⇒ heutiges Kriterium.
      *Instrument:* **Die Zwischenabnahme nach dem ESP-Flash ist der Nachweis**
      (Task S.8) — zu diesem Zeitpunkt läuft der neue ESP gegen den noch alten STM.
      Ein Vollabgleich muss dort ohne Timeout und ohne Reset durchlaufen,
      `var_send_timeout_cnt` darf nicht steigen. Die gerechnete Mehrbelastung dieser
      Lage sind rund 135 ms über 194 Kommandos (L266) und ist damit nicht messbar —
      **messbar ist das Ausbleiben des Fehlers, und das genügt.**
- [ ] **AKS.7** — Der Zähler für „unmarkierte Zeile nach der ersten Marke dieser
      Sitzung" steht nach einem vollen Vollabgleich auf einem erklärbaren Wert und ist
      **ablesbar**.
      *Instrument:* `/api/device_ready` oder der Logring — **nicht** nur eine serielle
      Zeile. (L177/L185-Falle.)
- [ ] **AKS.8** — Eine **unmarkierte** Zeile einer der drei pflichtigen
      Kommandoarten (`HARDWARE_CONFIGURATION`, Update-Host, Update-Pfad) wird bei
      belegter Markierung **nicht übernommen** und mit **`!v`** abgewiesen; A32 sendet
      sie sofort nach.
      *Instrument:* Prüfstand, nicht am Gerät erzeugt. Dazu am Gerät die Gegenprobe,
      dass im Normalbetrieb **keine** dieser drei Abweisungen auftritt — eine
      Abweisung im gesunden Zustand wäre der Beleg, dass die Pflicht zu weit greift.
      **`!v` statt Schweigen ist Pflicht:** Schweigen kostete je Fall 3 Sekunden
      Hauptloop-Stillstand ohne `watchdog_reload()` (L266).
- [ ] **AKS.9** — Tetris und Snake bedienen den Watchdog. Ein Spiel läuft über
      60 Sekunden ohne Reset.
      *Instrument:* Quelltext **und** ein Lauf am Gerät durch den **Nutzer** — der
      Befund sagt, dass nach rund 3,6 Steinen das 20-s-Fenster um ist, also zeigt sich
      ein fehlender Reload innerhalb einer Minute. Ohne diesen Lauf gilt AKS.9 als
      **nicht erfüllt**, nicht als „vermutlich in Ordnung".

### Runde P — PWA

- [ ] **AKP.1** — „Anzeigen" bei einer `.gz`-Datei schreibt keine Binärdaten ins DOM,
      sondern meldet, dass die Datei binär ist, und nennt ihre Grösse.
      *Instrument:* `/api/fs_list` liefert Name und Grösse; Prüfung im Browser über
      `./tools/check-pwa.sh` mit geöffnetem Dateimodul, Konsole ohne Fehler.
- [ ] **AKP.2** — Übersicht und Auswahlfeld nennen denselben Display-Modus.
      *Instrument:* Browserlauf, beide Felder abgelesen; die fünf fest verdrahteten
      Namen kommen im Quelltext nicht mehr vor.

### Runde U — UI

- [ ] **AKU.1** — **Jedes** Ankreuzfeld rendert mindestens 20 × 20 px und nicht mehr
      13 × 44 px, in allen vier betroffenen Modulen, einschliesslich der drei
      TFT-Felder und der Favoritenfelder.
      *Instrument:* das Geometriemodul aus AKW.1, **einmal Vorschau, einmal Gerät**.
      Die Gerätemessung ist Pflicht — L229 hält ausdrücklich fest, dass
      Vorschauzahlen keine Gerätemessung sind.
- [ ] **AKU.2** — Die grösste Höhendifferenz benachbarter Wartungskacheln liegt über
      die Breiten 390, 600, 768, 900, 1240 und 1512 px bei **≤ 400 px**.
      *Instrument:* dasselbe Modul. **Die Breite 600 px ist Pflicht** — dort lag der
      schlechteste Wert (594 px), und AK3.1 des alten Pakets war auf 1512 px
      formuliert und hat genau deshalb den schlechtesten Fall verdeckt.
      **Wird das verfehlt, gilt es als verfehlt** und wird so berichtet (L227).

### Runde F — Flash-Überwachung

- [ ] **AKF.1** — Der Legacy-Flashpfad ruft **dieselbe** Prüffunktion wie der
      API-Endpunkt. Im Legacy-Zweig steht kein ungeprüftes `strncpy` des Dateinamens
      mehr.
      *Instrument:* `grep` findet den Aufruf an beiden Stellen. **Nicht scharf
      fahren.**
- [ ] **AKF.2** — Bei `HARDWARE_CONFIGURATION` = 65535 bietet die Auswahlliste
      **keine** Datei an, und der Hinweistext nennt `./tools/flash-stm.sh` als
      Rückweg.
      *Instrument:* lesend, über die Legacy-Seite. **Die Länge des Hinweistextes ist
      mitzuprüfen** — L257 zeigt, dass eine 123 Zeichen lange Meldung genau am
      Rückweg abgeschnitten worden wäre.
- [ ] **AKF.3** — `./tools/smoke-device.sh` meldet `HARDWARE_CONFIGURATION` = 65535
      als eigenen Fehlschlag.
      *Instrument:* Gegenprobe gegen einen **festen Testwert**, nicht gegen einen am
      Gerät erzeugten Zustand. Im gesunden Zustand schlägt die Stufe nicht an.
- [ ] **AKF.4** — Nach dem STM-Flash dieser Runde meldet `/api/update_status` die
      erwartete STM-Version, und `./tools/flash-stm.sh --check` lief vorher ohne
      Beanstandung.
      *Instrument:* `flash-stm.sh` (DIR-010), **nicht** der Endpunkt von Hand.

### Für jede Runde

- [ ] **AKZ.1** — `./tools/guardrails.sh` läuft mit Exit 0 durch.
- [ ] **AKZ.2** — `./tools/guardrails.sh --full` läuft vor dem Release mit Exit 0
      durch (Lead, R1).
- [ ] **AKZ.3** — `./tools/smoke-device.sh` nach dem Rollout ohne Fehlschlag
      (DIR-009). **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.4** — Während jedes Gerätelaufs läuft `./tools/watch-log.sh` mit und wird
      **mitgelesen** (DIR-013). Ein Neustart, eine Exception oder ein Sprung in `d=`
      macht das Ergebnis des Laufs ungültig und gehört in den Bericht.
- [ ] **AKZ.5** — Jedes ausgerollte Release ist **sofort** committet, getaggt und
      **gepusht** (DIR-011), einzeln je Runde.
- [ ] **AKZ.6** — Die Einspielzeile nennt die **Reihenfolge**, wo eine Komponente die
      andere voraussetzt (L241), und was sich für den Nutzer sichtbar ändert.

---

## Nicht Teil dieser Änderung

Ein Paket, das alles enthält, wird nie fertig. Diese Punkte sind **bewusst**
ausgeschlossen, jeder mit Grund:

| Ausgeschlossen | Grund |
|---|---|
| **L176 — den Logring verkleinern** | **Vom Nutzer entschieden: fällt aus dem Paket.** Der Vorschlag stammt ursprünglich von ihm selbst; seine eigene Beobachtung vom selben Tag ist das stärkere Argument dagegen. **L262: Der Ring deckt bereits nur rund zwei Minuten ab** — gefüllt nicht von HTTP-Zeilen (davon enthält er **null**), sondern von der eigenen Geschwätzigkeit des STM. Für einen Hänger, der 90 Sekunden dauert, ist das knapp. **Weniger Rückschau kostet genau die Diagnosefähigkeit, die zuletzt mehrfach den Ausschlag gegeben hat** — L175 wurde über zwei Heap-Zeilen im Ring gemessen, und das ging erst, seit C14 sie dorthin schickt. Der Heap bleibt eng; **das ist ein eigenes Paket wert** (L175: `sanitize_*_string()` baut 36-mal ein Ergebnis zeichenweise als `String` auf — dort liegt der Hebel, nicht beim Ring). **Hier festgehalten, damit der Vorschlag in einem halben Jahr nicht unbesehen wiederkommt** |
| **A35 Teil 2 — Quittung bestätigt Übernahme** | Verlangt einen Rückgabewert aus `var_set_parameter()` und damit einen Eingriff in jeden Zweig seines `switch`, der keinen `default` hat. Teil 1 löst das **dringende** Problem, weil A32 die verspäteten Quittungen gerade häufiger gemacht hat. **Für eine Teilmenge ist Teil 2 ohnehin schon da:** `!v` sagt heute „gelesen, nicht angewandt" |
| **A2 / A5 — DS18xx-CRC und RTC-Vorzeichen** | Der klarste nächste Fix, aber vier Module plus UI-Seite, und er braucht eine eigene Verifikation am Sensor. **Erster Kandidat für das nächste Paket.** Hier auszulassen ist eine Umfangsentscheidung, keine Bewertung |
| **B15 / B8 — Sprachen auslagern, hartcodierte Strings** | Eigenes Vorhaben, Spec liegt bereits unter `specs/i18n-auslagerung/`. Rund 61 kB Quelle, Weissliste im ESP, `install-app.sh` — das sprengt jede Runde hier |
| **C3 — EEPROM seitenweise** | Berührt den Schreibpfad der produktiven Uhr bei rund 16 ms je Byte. Eigene Spec, eigene Messung. Voraussetzung für F4, nicht für dieses Paket |
| **F4 — EEPROM-Abbild als Sicherung** | Gross, zwei neue destruktive Endpunkte, hängt an C3 |
| **C6 — destruktive Endpunkte auf POST** | Bricht die PWA, solange `apiFetch` ohne Methode aufruft. Beides müsste **gleichzeitig** geschehen — genau das, was Risiko 1 (OTA, C13/L180) verbietet |
| **L175 / C9c2 — Heap-Fragmentierung an der Wurzel** | Der Hebel sind die 36 `sanitize_*_string()`-Aufrufe, die ihr Ergebnis zeichenweise als `String` aufbauen. Das ist ein Umbau an der meistgenutzten Hilfsfunktion des ESP, und er gehört **nicht** in dieselbe Runde wie der Absturzfix, der an derselben Datei arbeitet |
| **A21 / A28 / A15 — Brückenlast, Empfangsring, UART-ISR** | Messvorhaben ohne entschiedene Massnahme. A32, Weg B und dieses Paket verändern die Lage gerade; eine Messung **vorher** misst einen Zustand, den es nicht mehr gibt. Gehört **nach** Runde S — und dann mit den rund 1,5 kB je Vollabgleich aus A35 im Blick |
| **A22 / A24 / A26 — Indexprüfungen im Tabellentransfer** | Die Gewichtung hat sich durch A32 verschoben: Eine beschädigte Zeile wird an der Prüfsumme abgewiesen, bevor sie die Tabellen erreicht. **Ob A22 noch dasselbe Gewicht trägt, ist eine Frage für Runde V**, nicht eine Umsetzung für dieses Paket |
| **C9c3 / C9c4 — periodische Heap-Zeile und ihr Rückbau** | Die Diagnosehilfe hängt an L175, und L178 hat die Fragmentierungs-These bereits widerlegt. **C14 (Runde E1) liefert den Nutzen ohnehin schon** — seit die Heap-Zeile im Ring steht, ist der Verlauf ohne Zusatzgerät lesbar (L175, Nachtrag) |
| **B31 — selbst gezeichnetes Ankreuzfeld** | Gestaltungsentscheidung mit eigener Abnahme. **Die eine Messung, die die Priorität entscheidet** — trägt der Umriss auch in WebKit? — kostet fast nichts und steht als Messauftrag W.8 in Runde W. Erst danach entscheidbar |
| **E2 / E3 / E7 / E14 — tote Bundle-Dateien, Stückliste, Kleinkram** | Kein Risiko, kein Nutzerproblem. Sie verlieren nichts dadurch, dass sie warten |
| **B19 — Phase 0 des Testplans** | Verfahrensfrage mit einem Passwort darin; gehört in eine Runde, in der der Nutzer ohnehin beteiligt ist |

### Warum A41 / L265 **doch** in dieses Paket gehört

Die Entscheidung lag bei mir. Sie lautet **aufnehmen**, als eigene Runde H, aus drei
Gründen:

1. **Nur eine Seite zu härten wiederholt L179.** A37 (ESP, rund 45 Stellen) steht in
   diesem Paket. Dieselbe Fehlergattung im STM mit **rund 66 Stellen** draussen zu
   lassen, erzeugt genau das Muster, das die Uhr am 04.10.2026 stillgelegt hat: zwei
   Wege zum selben Ziel, einer mit Schutz, einer ohne.
2. **Der Auslöser wird täglich gemessen.** Eine verkürzte Kommandozeile entsteht,
   wenn die Brücke ein Zeichen verliert. Das ist der Zähler `d=`, und er ist nicht
   null. Die beiden exponierten Stellen lesen danach mit `while (*p)` über die
   Pufferkante — `tftled.c:122` und `esp-spiffs.c:206-231`.
3. **Der STM wird in diesem Paket ohnehin geflasht.** Die Alternative wäre, A41 ins
   nächste Paket zu schieben und dort einen eigenen Flash dafür aufzusetzen.

**Warum trotzdem eine eigene Runde und nicht in Runde S:** Runde S ändert das
**Protokoll** — Eröffnungszeile, Abschlussmarke, Zuordnungszeile —, A41 ändert den
**Parser jedes eingehenden Kommandos**. Beides in einem Flash macht einen Fehlschlag
unzuordenbar. Und die Reihenfolge ist nicht beliebig: **H kommt vor S**, damit die
neuen Zeilenarten auf einem gehärteten Parser landen und nicht umgekehrt.

---

## Betroffene Laufzeiten

- [x] **STM32** (`src/**`) — Runden H und S. Je ein Flash über `./tools/flash-stm.sh`
- [x] **ESP8266** (`ESP8266/ESP-uclock/*`) — Runden E1, E2, S, F. Je ein OTA
- [x] **PWA** (`data/app/**`) — Runden P und U. LittleFS-Upload über
      `./tools/install-app.sh`
- [x] **Build/Release** — je Runde ein Release-ZIP, Rollout, Commit, Tag und Push
- [x] **Werkzeug** (`tools/**`, `.claude/**`) — Runde W, kein Flash
- [x] **Dokumentation** (`BEFUNDE.md`, `knowledge/**`) — Runden V und W

---

## Getroffene Entscheidungen

Alle vier offenen Punkte hat der Nutzer entschieden. Sie stehen hier, weil die
Begründung später niemand mehr rekonstruieren kann.

1. **C26 / L253 — Zähler **und** Markenpflicht.** Beides wird gebaut: der Zähler für
   unmarkierte Zeilen nach der ersten Marke einer Sitzung, **und** die Markenpflicht
   für die drei benannten Kommandoarten `HARDWARE_CONFIGURATION`, Update-Host und
   Update-Pfad. Unmarkiert heisst dort: **nicht übernehmen**, und abgewiesen wird mit
   **`!v`** — der Mechanismus „gelesen, nicht angewandt, sofort nachsenden" existiert
   bereits (`src/esp8266/esp8266.c:292`).
   **Die Begründung, die L253 fehlte:** Ein leerer Update-Host hat zuletzt **fremde
   Firmware geholt** — der ESP fällt dann auf seine eingebauten Vorgaben zurück, und
   die zeigen auf den Server des Ursprungsprojekts (L42, DIR-009). Bei genau diesen
   drei Werten ist die 19,1-%-Lücke keine statistische Grösse, sondern ein Weg, auf
   dem fremder Code aufs Gerät kommt.
2. **A35 Teil 1 — ja, und der verlangte Nachweis hat den Entwurf umgeworfen.**
   Der Nutzer hatte den Punkt unter den Vorbehalt gestellt, dass der Rückfall erst am
   Code belegt wird. **Das Ergebnis steht als L266 und ist besser als der
   ursprüngliche Entwurf:**
   - Die vier Fragen zum Rückfall waren unbedenklich — kein Byteversatz, kein Rest im
     Ring, kein Teilkommando.
   - **Die Frage war falsch gestellt.** Gefährlich ist nicht der Versatz, sondern die
     Folge: `esp8266.c:287` vergleicht auf genau ein Zeichen, jede Form `.XY` endet
     als `ESP8266_UNSPECIFIED`, `var_send_buf()` läuft in den 3-s-Timeout, und mit
     `got_answer == 0` bleibt `watchdog_reload()` aus — **Reset nach rund sieben
     Kommandos.**
   - **Die Lösung:** Die Zuordnung kommt als **eigene Zeile vor** der Quittung
     (`ACK c3`, dann `.`). Ein alter STM verwirft die unbekannte Zeile und liest den
     Punkt unverändert. **Damit ist eine Fähigkeitsmeldung entbehrlich**, der
     blockierende Nachweis-Task entfällt, und der Fehlfall kann gar nicht erst
     entstehen.
   - Kosten, gerechnet: rund **1,5 kB je Vollabgleich** und auf altem STM rund
     **135 ms** zusätzlich über 194 Kommandos.
3. **A39 — Variante (b).** Der IP-Lauftext wird **nach** dem Vollabgleich gesetzt,
   die heutige Reihenfolge bleibt exakt erhalten; nur die Verschachtelung fällt weg.
   Einzelheiten und die verworfene Variante (a) in `design.md` §6.1.
4. **A16 — bleibt im Paket.** Der Nutzer nimmt die 60 Sekunden Tetris in Kauf, weil
   der STM in Runde S ohnehin geflasht wird.

**Nichts davon ist mehr offen.** Was vor dem Bauen noch beim Nutzer liegt, ist allein
die **Freigabe der Spec** und die Freigabe jedes schreibenden Gerätezugriffs im
jeweiligen Gespräch (R5).
