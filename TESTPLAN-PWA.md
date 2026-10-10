# Teststrategie PWA — vollständiger Funktionsdurchlauf

**Lebendes Dokument** (DIR-006). Es enthält bewusst **keine Versionsnummern** — den
gültigen Stand zeigt `./tools/guardrails.sh` in Stufe S4.

Dieser Plan beschreibt, wie die PWA **vollständig** geprüft wird: jede Anzeige, jede
Einstellung, jede Schaltfläche, die Grenzfälle jedes Eingabefelds, und Backup und
Restore als eigener Prüfgegenstand. Er ist so geschrieben, dass er ohne Vorwissen
ausführbar ist und dass das Ergebnis **nachprüfbar** ist statt behauptet.

---

## 0. Die Randbedingung, die alles bestimmt

**Es gibt genau eine Uhr, und sie läuft produktiv im Wohnraum des Nutzers.** Kein
Testgerät, kein zweites Exemplar, kein Rollback per Knopfdruck. Daraus folgt für jeden
Schritt dieses Plans:

1. **Erst sichern, dann anfassen.** Phase 0 ist nicht optional und nicht abkürzbar.
2. **Jede Änderung wird einzeln zurückgenommen**, nicht gesammelt am Ende. Bricht der
   Durchlauf in der Mitte ab, steht die Uhr trotzdem nahe am Ausgangszustand.
3. **Der Beweis ist der Abschlussvergleich** (Phase 9), nicht das Gefühl, alles
   zurückgesetzt zu haben. Er hat aber eine Grenze, die man ihm nicht ansieht: Er
   findet nur, was **dieser** Lauf verändert hat. Deshalb gehört die
   Altlastendurchsicht aus 12.2 dazu.
4. **Zehn Funktionen werden nicht scharf ausgeführt.** Welche und warum: Phase 8.
   Dazu gehören **WLAN-SSID und -Schlüssel**: Sie zu schreiben hiesse, die Verbindung
   zu kappen, über die geprüft wird.

Ein Durchlauf dauert geschätzt **4 bis 6 Stunden**, davon rund 90 Minuten
unbeaufsichtigt (Wartezeiten bei Neustarts und Netzwechseln). Er ist in Phasen
schneidbar; jede Phase endet in einem sauberen Zustand.

---

## 1. Was überhaupt zu prüfen ist — das Inventar

Aus dem Quelltext erhoben, nicht geschätzt:

| | Zahl |
|---|---|
| Module (Reiter) | 11 |
| Schaltflächen | 87 |
| Eingabefelder | 55 |
| Auswahlfelder | 14 |
| Dateiauswahlfelder | 8 |
| Schieberegler | 6 |
| API-Endpunkte, die die PWA nutzt | 103 |

Verteilung über die Module:

| Modul | Knöpfe | Eingaben | Auswahl | Dateien | Regler |
|---|---|---|---|---|---|
| `main` | 2 | – | – | – | – |
| `system` | 7 | 5 | 4 | – | – |
| `network` | 8 | 5 | 1 | – | – |
| `climate` | 12 | 6 | – | – | – |
| `display` | 11 | 9 | 2 | – | 2 |
| `animations` | 2 | – | 2 | – | – |
| `overlays` | 0 | – | – | – | – |
| `ambilight` | 12 | 7 | 2 | – | 3 |
| `timers` | 2 | – | – | – | – |
| `dfplayer` | 7 | 9 | 1 | – | 1 |
| `maintenance` | 24 | 14 | 2 | 8 | – |

**`overlays` und `timers` haben null statische Bedienelemente.** Beide Module bauen
ihre Oberfläche zur Laufzeit in JavaScript (`#overlay-list`, Timer-Karten). Eine
Prüfung, die nur das Markup abläuft, übersieht sie vollständig — sie brauchen einen
eigenen Durchgang (Phase 3.7).

**Beide sind inzwischen geprüft** — der Satz „bis heute ungeprüft" stand hier noch,
nachdem er längst überholt war. Der Durchlauf vom 03.10.2026 (ESP 3.2.15) hat S70 bis
S85 vollständig gefahren; das Ergebnisprotokoll liegt unter
`tools/snapshots/abschluss-3.2.15/ergebnisse.tsv`. **Der eigene Durchgang in Phase 3.7
bleibt trotzdem Pflicht:** Er hängt nicht daran, dass die Module neu wären, sondern
daran, dass sie kein statisches Markup haben und eine Prüfung über das DOM sie
übersieht.

**Nicht alle Module sind auf jedem Gerät sichtbar.** `getUiFeatureState()` blendet
Panels anhand von `HARDWARE_CONFIGURATION` und der Online-Erkennung aus — TFT,
DFPlayer und Ambilight. Vor dem Durchlauf ist festzuhalten, welche Teilsysteme die
geprüfte Uhr hat; Module ohne Hardware werden **als „nicht prüfbar" protokolliert**,
nicht als „bestanden".

---

## 2. Risikoklassen

Jede Funktion bekommt eine Klasse. Die Klasse bestimmt, **wie** geprüft wird — nicht,
**ob**.

| Klasse | Bedeutung | Vorgehen |
|---|---|---|
| **L** | rein lesend | frei, jederzeit, auch automatisiert |
| **S** | schreibend, folgenlos rücknehmbar | Wert setzen, gegenprüfen, zurücksetzen |
| **N** | schreibend, trifft das Netz | nur mit vorbereitetem Rückweg, und **nie** an den Zugangsdaten der aktiven Verbindung |
| **R** | löst einen Neustart aus | eingeplant, mit Wartezeit und Wiederanlaufprüfung |
| **G** | kann das Gerät unbedienbar machen oder Daten verlieren | **Phase 8** — Ersatzprüfung statt scharfer Ausführung |

### Aufgeräumt wird jeder berührte Index, nicht jeder geplante

Bei Klasse S heisst „zurücksetzen" **jeden Index, an den ein Aufruf ging** — auch den
eines Aufrufs, den das Gerät abgewiesen hat. Die Aufräumliste entsteht aus dem
Mitschnitt der gesendeten Aufrufe, nicht aus der Testplanung.

**Der Mitschnitt wird zu Beginn jedes Laufs geleert.** Er ist eine Protokolldatei, die
fortgeschrieben wird — im Durchlauf vom 04.10.2026 hiess sie `gesendet.log`, und beim
Start enthielt sie **838 Aufrufe aus zwei früheren Läufen**. Wer die Aufräumliste
daraus baut, räumt fremde Indizes auf: Er schreibt Werte zurück, die gar nicht zu
seinem Lauf gehören, und erzeugt damit genau die Altlast, die er verhindern soll. Das
Leeren gehört in Phase 0, zusammen mit den Sicherungen — und nicht ans Ende, wo es bei
einem Abbruch unterbleibt.

Der Grund ist gemessen (`BEFUNDE.md`, L81): Im Durchlauf vom 03.10.2026 gingen die
regulären Prüfungen auf die Timer-Slots 2 bis 4, die Edge-Case-Aufrufe aber auf 5
und 8. Aufgeräumt wurden 2 bis 4. Slot 5 blieb als **aktiver Timer auf 00:00**
stehen und hätte die Uhr jede Nacht zusätzlich ausgeschaltet. Ein abgewiesener
Aufruf hinterlässt nichts — aber ob er abgewiesen wurde, weiss man erst hinterher,
und im Durchlauf davor war derselbe Aufruf noch angenommen worden.

**Der Nachweis ist der Abschlussvergleich, nicht der Bericht.** Derselbe Agent
meldete „Alle Testslots sofort geleert", während der Slot stand. Das war keine
Unwahrheit: Er hatte seine Liste abgearbeitet. Ein Agent, der seine eigene
Aufräumarbeit bestätigt, bestätigt seine Absicht und nicht den Gerätezustand.
Gefunden hat es `./tools/diff-snapshot.sh` gegen den Referenzabzug — deshalb ist
Phase 9 keine Formsache und darf auch bei unauffälligem Bericht nie entfallen.

---

## 2b. Backup- und Restore-Konzept

Dieser Abschnitt steht vor den Phasen, weil ohne ihn keine davon beginnen darf. Er gilt
auch unabhängig vom Testen — es ist das Sicherungskonzept der Uhr.

### Drei Medien, drei verschiedene Zwecke

| | Medium | Erzeugt mit | Enthält Zugangsdaten | Taugt zum |
|---|---|---|---|---|
| **M1** | PWA-Sicherung (JSON) | Wartung → Sicherung → Exportieren | **ja, im Klartext** | **Zurückspielen** über die PWA |
| **M2** | Wiederherstellungsabzug | `./tools/snapshot-device.sh --restore` | ja, aber **verschlüsselt** (AES-256, PBKDF2) | **Zurückspielen von Hand**, unabhängig von der PWA |
| **M3** | Vergleichsabzug | `./tools/snapshot-device.sh` | **nein**, nur Prüfsummen | **Nachweisen**, dass der Zustand stimmt |

**M3 kann nichts wiederherstellen, und das ist Absicht.** Ein gehashter Schlüssel ist
nicht umkehrbar. M3 darf deshalb liegen bleiben, in Berichte wandern und von mir
gelesen werden, ohne dass dabei ein Passwort in Umlauf gerät. Wer nur M3 hat, hat kein
Backup — er hat einen Messpunkt.

**M2 schliesst genau diese Lücke.** Gleicher Inhalt wie M3, aber mit Klartext-Schlüssel
und als Ganzes verschlüsselt. Das Klartextverzeichnis wird unmittelbar nach dem
Verschlüsseln gelöscht.

> **Das Passwort für M2 kennt nur der Nutzer.** Es steht nicht im Repo, nicht in
> `device.conf` und in keiner Sitzung. Geht es verloren, ist der Abzug verloren — es
> gibt keinen Weg, ihn aufzubrechen. Das ist der Preis dafür, dass eine Datei mit dem
> WLAN-Schlüssel gefahrlos herumliegen darf.
>
> **Damit ein Agent M2 trotzdem anlegen kann** (B19, entschieden am 07.10.2026), liest
> `snapshot-device.sh` das Passwort aus einer Datei **ausserhalb des Repos**, Vorgabe
> `~/.config/wordclock/snapshot.pass`, anderer Ort über `SNAPSHOT_PASS_FILE`. Die Datei
> legt der Nutzer einmal an (`chmod 600`). Das Skript bricht ab, wenn sie im Repo liegt
> oder für andere lesbar ist, und nennt bei jedem Lauf, woher das Passwort kam. In
> der Sitzung steht nur der Pfad, nie das Passwort.

**M1 ist das einzige Medium mit einem automatischen Rückweg.** Nur die PWA kann eine
Sicherung wieder einspielen. M2 ist Handarbeit: entpacken, Werte ablesen, über die
Oberfläche oder die Legacy-Seite eintragen.

> **M1 liegt unverschlüsselt auf der Platte** und enthält `wifi_key` im Klartext
> (`buildNetworkBackupSettings()`). Das ist kein Fehler der PWA — ohne den Schlüssel
> wäre der Import nutzlos. Aber die exportierte Datei gehört behandelt wie ein
> Passwortzettel: nicht in Cloud-Ordner, nicht ins Repo, nicht in einen Chat.

### Was in keinem der drei Medien steht

| Was | Folge bei Verlust |
|---|---|
| **Angelernte IR-Codes** | Fernbedienung muss Taste für Taste neu angelernt werden. Sie liegen im STM-EEPROM und tauchen weder in `settings_xml` noch in der PWA-Sicherung auf |
| **Layout-Tabellendatei** (`wc12h-tables-*.txt`) | Nur der **Name** ist gesichert. Datei vom Update-Server nachladen |
| **Icon- und Wetterdatei** | dito |
| **Die PWA-Dateien im LittleFS** | `./tools/install-app.sh` — setzt voraus, dass der ESP erreichbar ist |
| **ESP- und STM-Firmware** | Release-ZIP beziehungsweise Update-Server |

**Die IR-Codes sind die echte Lücke.** Für alles andere gibt es einen Weg zurück; für
sie nicht. Das ist der Grund, warum `maintenance_reset_eeprom` in Phase 8 steht und
nicht beiläufig ausgelöst wird.

### Reihenfolge beim Zurückholen

Nach einem vollständigen Verlust, von unten nach oben:

1. **ESP erreichbar machen** — AP-Modus oder serieller Zugang. Ohne diesen Schritt
   wirkt keiner der folgenden.
2. **Netzwerk von Hand eintragen** (aus M1 oder M2). Erst danach ist die Uhr wieder
   im WLAN.
3. **PWA hochladen** — `./tools/install-app.sh`. Vorher gibt es keine Oberfläche.
4. **Layout-Tabelle und Assets** vom Update-Server laden.
5. **M1 importieren** — stellt alle übrigen Einstellungen her.
6. **IR-Codes neu anlernen.** Von Hand, es gibt keine Alternative.
7. **Prüfen:** `./tools/diff-snapshot.sh referenz <neu>` gegen einen M3-Abzug aus der
   Zeit davor.

Schritt 7 ist nicht optional. **Eine Sicherung, die nie zurückgespielt wurde, ist eine
Hoffnung, keine Sicherung** — deshalb prüft Phase 5 den Import ausdrücklich und
deshalb endet jeder Testdurchlauf mit dem feldweisen Vergleich.

### Was regelmässig zu tun ist, unabhängig vom Testen

- **M1 nach jeder bewussten Umstellung** exportieren und die Datei behalten.
- **M2 vor jedem Eingriff**, der Netzwerk, EEPROM oder Dateisystem berührt.
- **M3 vor und nach** jedem Testdurchlauf — das Paar ist der Nachweis.

---

## 3. Phase 0 — Sicherung

### 3.1 Alle drei Medien anlegen

Nach dem Konzept aus Kapitel 2b, in dieser Reihenfolge:

```bash
# M1 — PWA-Sicherung: Wartung → Sicherung → Exportieren, Datei sicher ablegen
./tools/snapshot-device.sh --restore vor-durchlauf    # M2, verschluesselt
./tools/snapshot-device.sh referenz                   # M3, Vergleichspunkt
```

**Und als vierter Schritt, mit demselben Gewicht: den Mitschnitt der gesendeten
Aufrufe leeren.** Aus ihm entsteht am Ende die Aufräumliste (Kapitel 2). Steht beim
Start noch der Inhalt früherer Läufe darin, zeigt die Liste auf fremde Indizes — am
04.10.2026 waren es 838 Aufrufe aus zwei vorangegangenen Läufen. Eine leere Datei ist
hier Teil der Sicherung, nicht Ordnung.

**M3 ist für den Durchlauf das wichtigste**, obwohl es nichts wiederherstellen kann:
Das PWA-Backup ist selbst Prüfgegenstand dieses Plans, und eine Sicherung, die von der
zu prüfenden Funktion abhängt, ist keine Sicherung. M3 fragt die Endpunkte direkt ab
und ist davon unabhängig — es liefert in Phase 9 den Abschlussvergleich.

**M2 ist die Rückversicherung** für den Fall, dass der Import in Phase 5 nicht
funktioniert. Das Passwort dafür wird **vor** dem Durchlauf festgelegt und notiert.

### 3.2 Was keine dieser Sicherungen abdeckt

Das muss **vor** dem Durchlauf klar sein, sonst wird in Phase 8 das Falsche riskiert:

- **Angelernte IR-Codes.** `learn_ir` schreibt in das STM-EEPROM. Weder das PWA-Backup
  noch `settings_xml` enthalten sie. Gehen sie verloren, hilft nur erneutes Anlernen
  der Fernbedienung — von Hand, Taste für Taste.
- **Die Layout-Tabellendatei selbst.** Das Backup merkt sich nur ihren **Namen**
  (`assets.layout_table`), nicht ihren Inhalt. Nach einem `format_fs` ist sie weg und
  muss vom Update-Server neu geladen werden.
- **Icon- und Wetterdatei** (`wc12h-icon.txt`, `wc12h-weather.txt`) — dasselbe.
- **Die PWA-Dateien im LittleFS.** Nach `format_fs` ist die Oberfläche weg, mit der man
  sie wieder hochladen würde. Rückweg ist dann nur noch die Legacy-Seite.

### 3.3 Abbruchbedingung

**Lässt sich eine der drei Sicherungen nicht erzeugen oder nicht verifizieren, beginnt
der Durchlauf nicht.** Verifiziert heisst: Datei existiert, ist nicht leer, und das
PWA-Backup lässt sich als JSON einlesen.

---

## 4. Phase 1 — Referenzzustand festhalten

1. `./tools/smoke-device.sh` — muss **0 Fehler** melden. Schlägt hier schon etwas
   fehl, ist das ein Befund und kein Teststart. (Die Zahl der Prüfungen steht hier
   bewusst nicht: Sie hängt an der Zahl der Assets und wächst mit dem Skript.)
2. `./tools/install-app.sh --check` — Version **und** abgelegte Dateien gegen die
   Weissliste.
3. Rohabzug S2 als **Referenzdatei** ablegen. Gegen sie läuft Phase 9.
4. Festhalten: welche Teilsysteme sind da (RTC, EEPROM, DS18B20, TFT, DFPlayer,
   Ambilight), welche Module sind folglich sichtbar.
5. **`./tools/watch-log.sh` starten und bis zum Ende des Durchlaufs laufen lassen**
   (DIR-013). Nicht am Schluss durchsehen — **währenddessen** mitlesen. Der Mitschnitt
   meldet Exceptions, Neustarts, Watchdog-Resets, Sprünge in den verworfenen Zeichen
   (`d=`) und ein Stillstehen der `diag`-Folge.

**Punkt 5 ist nicht Beiwerk.** Am 03.10.2026 lief ein vollständiger Durchlauf, und
mitten darin stürzte der ESP ab; bemerkt hat es der Nutzer im Mitschnitt, nicht die
Prüfung. **Das Problem ist nicht der übersehene Absturz, sondern der Bericht, der
sauber meldet, während das Gerät zwischendurch neu gestartet ist** — jede Messung vor
dem Neustart ist dann wertlos, und der Bericht sagt es nicht. Ein Neustart im Fenster
zerstört ausserdem die Rahmenmessung V1 aus Abschnitt 5b.

---

## 4b. Phase 1b — Die PWA im Browser

```
./tools/check-pwa.sh
```

**Vor allem anderen, und zwar bei jedem Durchlauf.** Der Smoketest sagt, dass das
Gerät lebt und `app.js.gz` ausliefert. Er sagt **nicht**, ob die Datei beim Laden
einen Fehler wirft — dann bleibt die Oberfläche halb leer, und weder die API noch
ein Screenshot zeigen das.

Bis zum 03.10.2026 hat das kein Durchlauf geprüft. Der Testagent verglich die
API-Ebene gegen die Rohwerte, also das, was **unter** der Oberfläche liegt; seine
eigene Formulierung lautete „kein Browser in diesem Lauf". Das ist eine Lücke mit
Ansage, denn an `app.js` wird ständig gearbeitet.

Geprüft wird in einem echten Browser gegen das **echte** Gerät:

| | Was | Warum |
|---|---|---|
| **P1b-1** | Seite lädt, Titel da | sonst ist alles Weitere gegenstandslos |
| **P1b-2** | Module im DOM vorhanden | |
| **P1b-3** | **echte Gerätewerte in der Seite** | die gemeldete STM-Version wird aus `/api/settings_xml` geholt und im gerenderten Text gesucht. Steht sie nicht da, hat die Oberfläche die Daten nicht verarbeitet — unabhängig davon, ob die API sie korrekt geliefert hat |
| **P1b-4** | **jedes Modul lässt sich öffnen und zeigt Inhalt** | das ist der Unterschied zwischen „lädt" und „funktioniert". Ein Modul kann im DOM stehen und sich trotzdem nicht öffnen lassen, oder beim Öffnen einen Abruf fahren, der scheitert |
| **P1b-5** | jedes Modul erreichbar | bewusst ausgeblendete zählen nicht als Fehler — die PWA versteckt `dfplayer`, wenn die Hardware fehlt, und sagt das selbst: „DFPlayer is offline and hidden" |
| **P1b-6** | **keine Fehler in der Konsole** | der wichtigste Punkt — ein Tippfehler in `app.js` lässt die Seite halb leer, ohne dass irgendwo ein Fehler erscheint |
| **P1b-7** | keine fehlgeschlagenen Abrufe | `ERR_ABORTED` zählt **nicht**: Das heisst „jemand hat abgebrochen", nicht „es ging schief". Beim Schliessen des Browsers brechen alle laufenden Poller ab |
| **P1b-8** | Platzhalter gefüllt | viele stehengebliebene Striche heissen: Abrufe ohne Ergebnis, ohne geworfenen Fehler |

**Bedient wird über das DevTools-Protokoll**, ohne Puppeteer oder Playwright — Node
bringt seit v22 ein eingebautes WebSocket mit, und mehr braucht es nicht. Die Module
werden tatsächlich angeklickt, nicht nur im DOM gezählt.

**Nicht zu verwechseln mit `tools/preview/`.** Das rendert die PWA in zwanzig
Viewports, aber gegen **Attrappen-Daten** aus `server.py` — gut für das Layout,
ungeeignet für die Frage, ob die Oberfläche mit den echten Werten der Uhr
zurechtkommt. Beides hat seinen Platz: `preview` für Phase 7, `check-pwa.sh` hier.

Rein lesend: Die Seite wird geladen, nichts angeklickt.

---

## 5. Phase 2 — Alles Lesende

**Klasse L. Risikofrei, deshalb zuerst und vollständig.**

Für jedes Modul: Jeder angezeigte Wert wird gegen den Rohwert aus `settings_xml`
beziehungsweise `eeprom_settings` geprüft. Nicht „sieht plausibel aus", sondern
**Zahl gegen Zahl**.

Besonders zu beachten, weil hier schon einmal falsch formatiert wurde:

- **Temperatur**: Der STM-Fehlerwert `255` wurde früher als „127,5 °C" angezeigt.
  Zeigt der Sensor `127.5`, ist das **kein** Messwert, sondern „keine Messung".
- **Zeit**: Gerätezeit gegen die eigene Uhr, **minutengenau — mehr ist aus dem
  Rohabzug nicht zu holen**. `tmvar.second` steht dort durchgehend auf `0` —
  nachzuzählen mit
  `grep -o 'second="[0-9]*"' tools/snapshots/*/settings_xml.txt | sort -u`, das
  liefert genau einen Wert. Eine Forderung „auf ±2 s" wäre aus `settings_xml` nicht
  einlösbar; wer sie trotzdem protokolliert, protokolliert eine Rechnung, keine
  Messung. Sekundengenau prüfen liesse sich nur an der Anzeige selbst (V2) oder am
  Mitschnitt. Eine eingefrorene Anzeige ist das Leitsymptom des behobenen Hängers
  (L14) — sie wäre ein Rückfall, und dafür reicht die Minute: Zwei Abfragen im Abstand
  von über einer Minute müssen verschiedene Werte liefern.
- **Versionen**: STM, ESP und App-Version gegen `guardrails.sh` S4.
- **Dateiliste**: Grössen gegen die lokalen `.gz`.
- **`stm32_log`**: Wird das Logbuch überhaupt gefüllt, und bricht die Anzeige bei
  langen Zeilen um?
- **AP-SSID im Modul `network`**: in einer frischen Sitzung **direkt** öffnen, ohne
  vorher die Wartung zu besuchen. Soll: Die AP-SSID steht da, Wert gegen
  `eeprom_settings`. Am 10.10.2026 blieb sie nach 1, 5, 10 und 20 s leer, weil
  `eeprom_settings` nur bei offener Wartung geladen wird; nach einem Besuch der Wartung
  erschien sie (`BEFUNDE.md`, L367). Die Reihenfolge ist der Prüfgegenstand — wer
  zuerst die Wartung öffnet, sieht den Fehler nie.

---

## 5b. Was eine Setter-Gegenprobe nicht zeigt

Dieser Abschnitt steht **vor** Phase 3 und Phase 4, weil er die Aussagekraft jeder
schreibenden Prüfung begrenzt, die danach kommt.

**Das Problem in einem Satz:** `settings_xml` liefert die **ESP-seitige**
Variablenkopie — der ESP setzt sie beim Setter sofort und schickt das Kommando erst
danach an den STM; geht es auf der Brücke verloren, meldet die Gegenprobe trotzdem
den neuen Wert.

Daraus folgt eine unbequeme Lesart für jedes Protokoll: **Ein „bestanden" in
Schritt 3 des Prüfmusters belegt, dass der ESP gespeichert hat — nicht, dass der STM
es angewandt hat.** Das entwertet einen Durchlauf nicht, aber es benennt, was er
nicht zeigen kann. Am Gerät belegt (L188, 04.10.2026, derselbe blinde Fleck wie
L103): Der Empfangsring des STM meldete `d=5010` verworfene Zeichen, während die
Gegenproben desselben Zeitraums durchweg den neuen Wert zeigten. Ein Teil des
damaligen Testberichts ist dadurch rückwirkend nur noch eine Aussage über den ESP.

Dagegen stehen drei Verfahren. Keines ersetzt die anderen, und **jedes hat einen
Preis** — der gehört zur Methode, nicht ins Kleingedruckte.

### V1 — Rahmenmessung der Verlustzähler. Pflicht je Setter-Phase

Vor und nach **jeder** Setter-Phase wird der `d=`-Wert aus der Diagnosezeile des STM
gelesen. Die Zeile ist über `/api/stm32_log` erreichbar; STM-Zeilen kommen dort mit
dem Präfix `LOG ` an, ESP-eigene Zeilen mit `- `. Beide Werte — vorher und nachher —
gehören ins Protokoll, auch wenn sie gleich sind.

**Die Nachher-Messung wartet auf eine frische Zeile.** Die Diagnosezeile kommt nur etwa
alle 10 s. Wer direkt nach dem letzten Setter liest, liest oft eine Zeile, die **vor**
den Settern entstand — und meldet „`d` unverändert“ für ein Fenster, das er gar nicht
gemessen hat. Gültig ist erst eine Zeile mit höherer `diag`-Nummer, deren Zeitstempel
nach dem letzten Setter liegt. Im Testdurchlauf S.26 (08.10.2026) genau so passiert und
vom Tester selbst bemerkt. Mitlesen: `d=`, `v=` und `a=`.

| Befund | Bedeutung |
|---|---|
| `d` unverändert | In diesem Fenster ist kein Zeichen verworfen worden. Die Kommandos sind beim STM angekommen; ESP-Kopie und STM-Zustand dürfen als gleich gelten |
| `d` gestiegen | Die Gegenprobe dieser Phase ist **ungültig**. Phase wiederholen, Zuwachs und Fenster im Bericht nennen — das speist zugleich den Beobachtungsauftrag zu A28 |

**Der Preis ist methodisch, nicht zeitlich:** V1 ist eine **notwendige, keine
hinreichende** Bedingung. Drei Grenzen, die man kennen muss, sonst liest man mehr
heraus, als dasteht:

- `d` zählt die im Empfangsring des STM **verworfenen** Zeichen. Ein Kommando, das
  der ESP gar nicht erst absetzt, taucht dort nicht auf. „`d` unverändert" heisst
  also nicht „angekommen", sondern nur „nichts verworfen".
- `rx` ist ein **Höchststand, kein Füllstand** (`src/main.c:3396`). `rx=1024/1024`
  heisst „der Ring war einmal voll", nicht „er ist verstopft". Wer ihn als aktuellen
  Wert liest, überschätzt die Lage — am 04.10.2026 genau so geschehen.
- Der Ring fasst nur rund **4 bis 5 Minuten** Rückschau. Längere Phasen brauchen
  Zwischenablesungen, sonst liegt der Anfangswert ausserhalb des sichtbaren Fensters.

Der laufende Aufwand ist dagegen klein: zwei zusätzliche lesende Abrufe je Phase,
ohne jede Wirkung auf den Gerätezustand. Deshalb Pflicht und nicht Empfehlung.

**Ein Schritt dieses Plans zerstört V1, und zwar genau den Rahmen:** **S7** leert den
Logring (`stm32_log_clear`). Danach steht der `d=`-Wert vom Phasenanfang nicht mehr
darin, und die Rahmenmessung der laufenden Phase ist unlesbar. S7 gehört deshalb
**vor** eine Phase oder **nach** deren Abschlussmessung, nie dazwischen. Dasselbe gilt
für jeden Neustart im Fenster — nach **S110**, **S113** oder **S114** ist der Ring neu,
und der Anfangswert ist unwiederbringlich.

### V1b — Zeilenform im Mitschnitt. Pflicht je Setter

Seit Teil D schickt der ESP jeden Setter als prüfsummengesicherte Zeile `CMC` an den
STM, und der STM weist eine Zeile ab, deren Prüfsumme nicht passt. Nach **jedem**
Setter gehört deshalb in den Mitschnitt geschaut:

- Die Zeile ging als **`CMC`**, nicht als `CMD`. Ein `CMD` hiesse, dass der ESP die
  Fähigkeit des STM nicht kennt — dann prüft niemand die Zeile.
- Danach folgt **keine** `cmd abgewiesen`. Eine Abweisung heisst: Der STM hat den Wert
  **nicht** übernommen, auch wenn die Gegenprobe ihn zeigt (5b, ESP-Kopie).

**Kein `\r` in Testwerten erzeugen** — auch nicht in Phase 4 unter E8. Der STM
verwirft `\r` im Zeilenstrom, der ESP rechnet es in die Prüfsumme mit; die Zeile wird
abgewiesen. Das ist die sichere Richtung, hat aber eine Nebenwirkung: Jede Abweisung
merkt einen **Vollabgleich** aller Variablen vor, rund 194 quittierte Zeilen, und der
läuft genau dann, wenn die Brücke ohnehin unter Last steht (`BEFUNDE.md`, L361). Ob er
dabei an den Watchdog stösst, ist nicht gemessen.

Am 10.10.2026 ergab der Mitschnitt des ganzen Durchlaufs 993 `CMC`, 0 `CMD` und
0 `cmd abgewiesen` (L370). Das ist der Vergleichswert für den nächsten Lauf.

### V2 — Wirkungsprobe am sichtbaren Verhalten

Für Werte mit optischer oder hörbarer Wirkung — Helligkeit, Display-Farbe,
Anzeigemodus, Ambilight, DFPlayer — ist die Uhr selbst das STM-seitige
Anzeigeinstrument. Sie zeigt, was der STM wirklich anwendet, und umgeht die ESP-Kopie
vollständig. **Wertvoll genau dort, wo V1 nichts sagt:** beim Kommando, das nie
abgesetzt wurde.

**Der Preis:** nicht automatisierbar. Die Beobachtung gehört dem Nutzer oder einem
Schritt, der ihn ausdrücklich um Bestätigung bittet — und sie ist an seiner Uhr im
Wohnraum sichtbar, nicht auf einem Prüfstand. Anwendbar ist sie ausserdem nur auf die
Teilmenge der Werte, die überhaupt etwas sichtbar machen; für Zeitserver,
Koordinaten oder Update-Pfad sagt sie nichts.

### V3 — Abgleich über den STM-Neustart. Höchstens einmal je Durchlauf, am Ende, mit Freigabe

Nach `maintenance_reset_stm32` (S114) kündigt der STM seinen **gesamten**
Variablensatz neu an; die ESP-Kopie wird damit **aus dem STM** aufgebaut. Ein
Rohabzug danach zeigt also, was der STM wirklich hält — das ist der einzige
vollständige Abgleich, den dieser Plan kennt.

**Der Preis ist hoch, und er fällt sofort an:** Der Reset reisst den ESP mit (L183),
alle ESP-Zähler gehen auf 0, und der Variablenverlust aus L42/L103 kann eintreten —
also genau der Zustand, in dem der Rohabzug den Gerätezustand nicht mehr abbildet
(6.0d). Dazu die sichtbare Startsequenz an der Uhr.

Daraus folgt die Regel, ohne Ausnahme:

- **Nie mitten in einer Messreihe.** Jede davor laufende Messung ist danach ungültig.
- **Höchstens einmal je Durchlauf, am Ende**, in Phase 9, nach der letzten
  Setter-Phase.
- **Nur mit Freigabe des Nutzers** im selben Gespräch (R5).
- Danach zwingend: Variablensatz prüfen und die Update-Quelle gegen
  `tools/device.conf` (S97/S98).

### Welches Verfahren wann

| Lage | Verfahren |
|---|---|
| jede Setter-Phase in Phase 3 und Phase 4 | **V1**, verpflichtend, vorher und nachher |
| jeder einzelne Setter | **V1b**, `CMC` und keine Abweisung im Mitschnitt |
| Wert mit sichtbarer Wirkung | **V1 + V2** |
| Wert ohne sichtbare Wirkung, Zweifel am Durchkommen | **V1**, und im Bericht offen benennen, dass die STM-Seite unbestätigt bleibt |
| Abschluss des Durchlaufs | **V3**, einmal, mit Freigabe |

**Was im Bericht steht, wenn keines der drei greift:** „ESP hat gespeichert,
STM-Seite unbestätigt". Das ist eine zulässige Aussage. „Bestanden" ist sie nicht.

---

## 6. Phase 3 — Schreibende Funktionsprüfung, modulweise

**Klasse S, soweit nicht anders vermerkt.** Für **jede** Einstellung dasselbe Muster:

```
1. Ausgangswert notieren (aus dem Rohabzug, nicht aus der Anzeige)
2. Neuen Wert setzen
3. Gegenprobe: Rohwert erneut abfragen — steht dort der neue Wert?
4. Gegenprobe an der Hardware, wo sichtbar (Display, LED, Ton)
5. Ausgangswert zurückschreiben
6. Gegenprobe: steht der Ausgangswert wieder?
```

**Schritt 3 ist der Kern.** Eine Schaltfläche, die „Gespeichert" meldet, beweist
nichts — die PWA kennt den Erfolg nur vom HTTP-Status. Geprüft wird am Rohwert.

**Und Schritt 3 hat eine Grenze, die man ihm nicht ansieht:** Der Rohwert ist die
ESP-Kopie, nicht der STM-Zustand. Jede Setter-Phase wird deshalb nach **V1** aus
Abschnitt 5b gerahmt — `d=` vorher und nachher. Ohne diese Rahmung ist ein
„bestanden" eine Aussage über den ESP allein.

### 6.0a Jede Prüfung hat eine Kennung

Die Abschnitte 6.0 bis 6.9 führen die schreibenden Prüfungen **einzeln auf**, mit
fortlaufender Kennung `S1` bis `S114` über alle Module hinweg. Vorher stand hier
Fliesstext: Abschnitt 6.4 nannte dreizehn Einstellungen in einem einzigen Satz. Daraus
liess sich keine Checkliste bilden, und **es fiel niemandem auf, wenn zwei davon
ausgelassen wurden.**

Die Folge ist gemessen. Der Durchlauf vom 03.10.2026 meldete 22 Prüfungen. Von den
damals **42 benannten** Prüfungen dieses Plans hat er genau **eine** gefahren (B16);
die übrigen 21 hatte er selbst erfunden und selbst benannt. Was nicht im Plan steht,
kann nicht fehlen — und was selbst benannt wird, lässt sich zwischen zwei Durchläufen
nicht vergleichen.

**Teilprüfungen tragen die Kennung der Hauptprüfung plus einen Kleinbuchstaben**, in
der Reihenfolge, in der sie in der Soll-Spalte stehen: `S13b`, `S41b`, `S76a` bis
`S76d`. Das ist keine Formsache — am 03.10.2026 hiess der Grenzwert
`ticker_deceleration=256` im Protokoll `S41b`, am 04.10.2026 `S41c`. Dieselbe
Prüfung unter zwei Namen ist zwischen zwei Durchläufen nicht vergleichbar, und genau
das sollte die durchgehende Nummerierung verhindern. Wer eine Teilprüfung braucht, die hier noch
keinen Buchstaben hat, vergibt ihn **und trägt ihn in diesen Plan nach**.

Die Liste der Einstellungen ist **aus dem Quelltext abgeleitet**, nicht aus der
früheren Prosa: aus den `*_set`-Endpunkten in `ESP8266/ESP-uclock/http.cpp` (das ist
die vollständige Liste dessen, was überhaupt schreibbar ist), aus den Aufrufstellen in
`ESP8266/ESP-uclock/data/app/app.js` (das ist die Teilmenge, die die Oberfläche
erreicht) und aus `ESP8266/ESP-uclock/vars.h` (das sind die Variablen, in denen die
Werte landen).

### 6.0b Das Soll steht am Rohwert, nicht an der Oberfläche

Die Spalte **Soll** nennt das Feld im Rohabzug in genau der Schreibweise, die
`./tools/diff-snapshot.sh` ausgibt — `numvar[idx=6].value`, `strvar[idx=4].value`,
`dspcolor[idx=0].white`, `nighttime[idx=2].minutes` und so fort. Damit ist ein
Prüfergebnis belegbar, ohne dass jemand eine Variablennummer von Hand nachschlagen
muss, und es ist zwischen zwei Durchläufen vergleichbar.

Die Indizes stammen aus den Aufzählungen in `ESP8266/ESP-uclock/vars.h`. Ändert sich
dort die Reihenfolge, verschieben sich die Nummern — dann ist diese Tabelle
nachzuziehen. Zur Sicherheit steht neben jeder Nummer der symbolische Name.

### 6.0c Die Abdeckung ist zu melden

Am Ende eines Durchlaufs gehört **eine Zahl** in den Bericht:

```
Phase 3: von 114 benannten Prüfungen X gefahren, Y ausgelassen.
Je ausgelassener Prüfung: Kennung und Grund.
```

Ohne diese Zahl sieht ein Bericht über 22 Prüfungen genauso vollständig aus wie einer
über 114. **Ein Durchlauf ohne Abdeckungszahl gilt als nicht abgeschlossen**, auch wenn
jede einzelne gemeldete Prüfung bestanden hat.

Gültige Gründe für ein Auslassen sind: Hardware nicht vorhanden (Modul als „nicht
prüfbar" protokolliert), Freigabe des Nutzers fehlt, oder die Prüfung steht in Phase 8.
„Keine Zeit mehr" ist ebenfalls ein gültiger Grund — aber er muss dastehen.

### 6.0d Abbrechen ja, aufgeben nein

Am 03.10.2026 hat ein Testagent Phase 3 abgebrochen, weil der Rohabzug den
Gerätezustand nicht mehr abbildete (L103): Nach einem ESP-Neustart standen dort
Helligkeit 0 und Zeitzone 0, während die Uhr richtig lief. Ein regelkonformes
„Ausgangswert zurückschreiben" hätte die Uhr dauerhaft dunkel gestellt, und die
Gegenprobe hätte es nicht gefunden, weil sie denselben kaputten Zwischenstand liest.

**Die Entscheidung war richtig und bleibt richtig.** Wer bemerkt, dass der Rohabzug
nicht mehr stimmt, hört sofort auf zu schreiben.

**Was fehlte, war der zweite Teil.** Der Durchlauf endete damit, obwohl der Zustand
reparierbar war — ein STM-Reset hätte genügt. Deshalb gilt ab jetzt:

1. **Melden.** Zustand beschreiben, Beleg nennen (welches Feld weicht wovon ab).
2. **Reparieren lassen.** Die Reparatur macht der Nutzer oder der Lead, nicht der
   prüfende Agent.
3. **Nachweisen, dass der Abzug wieder trägt.** Neuer Rohabzug, Vergleich gegen die
   Referenz aus Phase 1.
4. **Fortsetzen**, bei der Kennung, an der abgebrochen wurde.
5. Im Bericht steht die Unterbrechung als eigene Zeile, mit Dauer und Grund.

Ein Abbruch ist ein Zwischenstand, kein Ende. Nur wenn eine der Abbruchbedingungen aus
Kapitel 15 greift, ist der Durchlauf wirklich beendet.

### 6.0 `main`

Das Modul hat zwei Schaltflächen und stand bisher in keinem Abschnitt dieses Plans.
Die beiden Hauptschalter der Uhr liegen hier, nicht im Modul `display`.

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S1** | Display ein- und ausschalten (`display_power_set?value=on\|off`) | S | `numvar[idx=3].value` (`DISPLAY_POWER`) steht auf `1` bzw. `0` | Sofort am Display sichtbar. Zuletzt prüfen und sofort zurücknehmen — eine dunkle Uhr fällt im Wohnraum auf |
| **S2** | Ambilight ein- und ausschalten (`ambilight_power_set?value=on\|off`) | S | `numvar[idx=30].value` (`DISPLAY_AMBILIGHT_POWER`) steht auf `1` bzw. `0` | An dieser Uhr ohne sichtbare Wirkung (L11) — nur am Rohwert prüfbar |

### 6.1 `system`

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S3** | Gerätezeit setzen (`datetime_set?year=&month=&day=&hour=&minute=`) | S | `tmvar[idx=0].year/month/day/hour/minute` tragen die gesetzten Werte | Der Parameter heisst `minute`, nicht `min` — die Legacy-Seite benutzt `min`. **Zuletzt prüfen und sofort korrigieren**, eine falsche Zeit fällt auf dem Display auf |
| **S4** | 31. Februar setzen | S | Antwort `error=4` (`INVALID_DATE`), `tmvar[idx=0]` **unverändert** | Schaltjahr mitprüfen: 29.02. eines Schaltjahrs muss angenommen werden |
| **S5** | `datetime_set` ohne `minute` | S | Antwort `error=1` (`MISSING_VALUE`), `tmvar[idx=0]` unverändert | Jedes Feld ist Pflicht; ein fehlendes darf nicht zu `0` werden |
| **S6** | Zeit vom Netz holen (`network_get_time`) | S | `tmvar[idx=0]` stimmt binnen 10 s auf ±2 s mit der eigenen Uhr überein | Schlägt fehl, wenn der Zeitserver aus S9 unbrauchbar ist — Reihenfolge beachten |
| **S7** | Logbuch leeren (`stm32_log_clear`) | S | `/api/stm32_log` meldet unmittelbar danach ein kleineres `count` als davor; die nächste Zeile trägt eine lückenlos fortgesetzte Folgenummer | Der Ring füllt sich sofort weiter, `count=0` ist deshalb **kein** zulässiges Soll. **Nicht innerhalb einer Setter-Phase ausführen.** Der Schritt ist trotz Klasse S **nicht rücknehmbar** — der gelöschte Inhalt ist weg, es gibt keinen Ausgangswert zum Zurückschreiben. Vor allem löscht er genau die Diagnosezeile des STM, auf der **V1** aus Abschnitt 5b beruht: Der `d=`-Wert vom Phasenanfang liegt danach nicht mehr im Ring, und die Rahmenmessung dieser Phase ist unlesbar. Also **vor** einer Phase oder **nach** deren Abschlussmessung, nie dazwischen |
| **S8** | Debug-Ansichten umschalten (vier Auswahlfelder, Anwenden, Zurücksetzen) | S | Rohabzug vor und nach dem Umschalten **feldgleich** | **Kein Endpunkt dahinter** — die Umschaltung wirkt nur lokal in der Oberfläche. Genau das ist das Soll: Sie darf am Gerät nichts ändern |

### 6.2 `network`

**SSID und WLAN-Schlüssel werden nicht geschrieben. Punkt.**

`network_client_set` schreibt die Zugangsdaten und löst eine Neuanmeldung am Router
aus. Das trifft **genau die Verbindung, über die geprüft wird**. Selbst das
Zurückschreiben der unveränderten Werte bedeutet einen Verbindungsabbruch — und beim
kleinsten Vertipper ist die Uhr nicht nur für den Durchlauf weg, sondern dauerhaft,
bis jemand physisch an das Gerät geht. Ein Testplan, der sich dafür auf „Rückweg
vorbereitet" verlässt, setzt die Erreichbarkeit aufs Spiel, um die Erreichbarkeit zu
prüfen. Das ist kein akzeptabler Tausch.

Dasselbe gilt für **WPS**, für **AP-SSID und AP-Schlüssel** (`network_ap_set` setzt
`EEPROM_FLAG_BOOT_AS_AP`, schreibt das EEPROM und ruft **sofort** `wifi_ap()`), für
**„Als Zugangspunkt starten"** und für `eeprom_settings_set`. Alle fünf stehen in
**Phase 8**.

**Was die SSID-Prüfung ersetzt:** Dass der Weg grundsätzlich funktioniert, ist dadurch
belegt, dass die Uhr **jetzt** im WLAN ist — die Funktion ist in Benutzung. Geprüft
wird lesend in Phase 2: Liefert `network_scan` Treffer, erscheint die aktive SSID
darin, zeigt die Oberfläche den Verbindungszustand richtig an.

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S9** | Zeitserver setzen (`network_timeserver_set?value=`) | N | `strvar[idx=4].value` (`TIMESERVER`) trägt den gesetzten Namen | Trifft den Zeitabgleich, nicht die Verbindung — deshalb N und nicht G |
| **S10** | Zeitserver mit 17 Zeichen | N | `error=2`, Detail nennt die Grenze in Byte (`value too long (max. 16 bytes)`), `strvar[idx=4].value` **unverändert** | Seit ESP 3.2.24 abgewiesen statt gekürzt, am Gerät gemessen am 05.10.2026 (L302). Bis dahin stand hier die stille Kürzung auf 16 Zeichen als erwarteter Befund |
| **S11** | Zeitzone setzen (`network_timezone_set?value=`) | S | `numvar[idx=19].value` (`TIMEZONE`) = Betrag der Zeitzone, bei negativen Werten zusätzlich Bit `0x100`, Sommerzeitbit `0x200` **unverändert** | MEZ mit Sommerzeit ergibt `513`. **Wirkt nicht sofort auf die Anzeige**, sondern erst beim nächsten NTP-Abgleich — hier stand bis zum 05.10.2026 „wirkt sofort“ |
| **S12** | Zeitzone `15` und `-13` setzen | S | Antwort `error=2` (`OUT_OF_RANGE`), `numvar[idx=19].value` unverändert | Die Grenze prüft der ESP selbst (L28), nicht nur die Oberfläche |
| **S13** | Sommerzeit umschalten (`network_summertime_set?value=on\|off`) | S | Bit `0x200` in `numvar[idx=19].value` gesetzt bzw. gelöscht, die unteren Bits unverändert | **S13b**, Aufruf **ohne** `value`: Soll ist „Wert bleibt“, Bit `0x200` unverändert. Das Gerät antwortet dabei `ok:true`, ohne etwas zu ändern — das ist das Soll, belegt wird es am Rohwert, nicht an der Antwort (10.10.2026). Ein fehlender Parameter hat die Sommerzeit früher abgeschaltet und Erfolg gemeldet (L47) |

### 6.3 `climate`

**`weather_get_now` und `weather_get_forecast` sind nicht rein lesend** (L54). Sie
lösen eine Anzeige auf dem Display aus und melden anschliessend bedingungslos
`{"ok":true}` — auch wenn AppID oder Ort unbrauchbar sind. Ein `ok:true` ist dort
**kein** Beleg dafür, dass Daten angekommen sind.

Das **Kartenmodal** ist ein eigener Prüfgegenstand ohne eigenen Endpunkt: Suche, Klick
auf die Karte, aktueller Standort, Übernahme, Escape, Klick auf den Hintergrund,
Fokusrückgabe. Der Standortzugriff braucht einen sicheren Kontext — über `http://`
schlägt er fehl, und das ist **erwartetes** Verhalten.

**„In Wetter übernehmen“ speichert direkt am Gerät**, nicht nur in die Felder der
Hauptmaske: `applyWeatherMapSelection()` ruft `weather_coordinates_set` und danach
`weather_city_set`. Das ist ein Schreibvorgang wie S15 und S16, mit denselben
Variablen. **Den Rückweg sofort danach einplanen** — Ort, Längen- und Breitengrad
vorher aus dem Rohabzug notieren und unmittelbar nach der Übernahme zurückschreiben,
erst die Koordinaten, dann den Ort. Beim Fokus nach einem Klick auf den Hintergrund
ist ein Befund offen (`BEFUNDE.md`, L368): Mit Escape kehrt er richtig zurück, nach dem
Hintergrundklick liegt er auf dem Body.

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S14** | Wetter-AppID setzen (`weather_appid_set?value=`) | S | `strvar[idx=5].value` (`WEATHER_APPID`) trägt den Wert, Grenze 32 | Leerer Wert wird mit `error=1` abgewiesen |
| **S15** | Ort setzen (`weather_city_set?value=`) | S | `strvar[idx=6].value` (`WEATHER_CITY`), Grenze 32 | |
| **S16** | Koordinaten setzen (`weather_coordinates_set?lon=&lat=`) | S | `strvar[idx=7].value` (`WEATHER_LON`) und `strvar[idx=8].value` (`WEATHER_LAT`), je Grenze 8 | Nur eines von beiden gesetzt ⇒ `error=1` |
| **S17** | Ort leeren, während keine Koordinaten stehen | S | `error=1`, `strvar[idx=6].value` unverändert | Ort und Koordinaten sind Alternativen; die letzte Angabe darf nicht wegfallen |
| **S18** | Wetter jetzt abrufen (`weather_get_now`) | S | Am Display erscheint die Wetterzeile; der Ticker läuft danach wieder an | **Nicht L.** `{"ok":true}` kommt auch bei unbrauchbarer AppID (L54) — Beleg ist die Anzeige, nicht die Antwort |
| **S19** | Vorhersage abrufen (`weather_get_forecast`) | S | wie S18 | |
| **S20** | DS18xx-Korrektur setzen (`temperature_ds18xx_correction_set?value=`) | S | `numvar[idx=24].value` (`DS18XX_TEMP_CORRECTION`) trägt den Wert | Bereich −20..20 in halben Grad |
| **S21** | RTC-Korrektur setzen (`temperature_rtc_correction_set?value=`) | S | `numvar[idx=22].value` (`RTC_TEMP_CORRECTION`) | Negative Werte stehen im Rohwert als Zweierkomplement eines Bytes: **−2 erscheint als `254`**. Die PWA rechnet das für die Anzeige um. Kein Befund — im Protokoll Rohwert und umgerechneten Wert nennen (10.10.2026) |
| **S22** | Korrektur `21` und `-21` setzen | S | `error=2`, beide Variablen unverändert | |
| **S23** | Temperatur anzeigen (`temperature_display`) | S | Die Temperatur erscheint auf dem Display | Zeigt der Sensor `127.5`, ist das **keine Messung**, sondern der STM-Fehlerwert `255` |
| **S24** | Automatische Helligkeit umschalten (`auto_brightness_set?value=on\|off`) | S | `numvar[idx=8].value` (`DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE`) 1 bzw. 0 | **Das Bedienelement liegt im Modul `climate`, nicht in `display`** — es steht im LDR-Bereich. Solange es an ist, überschreibt der LDR die Helligkeit aus S29 |
| **S25** | Aktuellen Messwert als LDR-Minimum übernehmen (`ldr_min_set`) | S, **nicht rücknehmbar** | `numvar[idx=17].value` (`LDR_MIN_VALUE`) entspricht dem zuvor abgelesenen `numvar[idx=16].value` (`LDR_RAW_VALUE`) | **Nur mit ausdrücklicher Freigabe des Nutzers fahren**, bis A59 entschieden ist. Der STM schreibt die Grenze in RAM und EEPROM; der Rückweg über S27 wirkt am STM **nicht**, er verwirft beide Setter als readonly (`BEFUNDE.md`, L364). Der Abschlussvergleich sieht das nicht, weil `settings_xml` die ESP-Kopie liefert (5b) — nach dem Durchlauf vom 10.10.2026 zeigte der Rohabzug die alten Grenzen, der STM hielt die Testwerte. Ohne Freigabe als „ausgelassen, nicht rücknehmbar“ protokollieren. Mit Freigabe: der Rohwert schwankt, deshalb unmittelbar vorher ablesen |
| **S26** | Aktuellen Messwert als LDR-Maximum übernehmen (`ldr_max_set`) | S, **nicht rücknehmbar** | `numvar[idx=18].value` (`LDR_MAX_VALUE`) entsprechend | wie S25: nur mit ausdrücklicher Freigabe, Rückweg am STM nicht vorhanden (L364) |
| **S27** | LDR-Grenzen numerisch setzen (`ldr_min_value_set`, `ldr_max_value_set`, je 0..4095) | S | `numvar[idx=17].value` bzw. `numvar[idx=18].value` tragen genau den gesendeten Wert | **Kein Bedienelement in der Oberfläche.** Die beiden Endpunkte sind ausschliesslich über den Backup-Import erreichbar — Prüfung deshalb per direktem Aufruf. **Das Soll belegt nur die ESP-Kopie:** Der STM verwirft beide Setter als readonly (L364). Bis A59 entschieden ist, lautet das Ergebnis höchstens „ESP gespeichert, STM nicht angewandt“, nicht „bestanden“ |
| **S28** | LDR-Grenze `4096` setzen | S | `error=2`, Variable unverändert | Früher angenommen (L67/L68) |

### 6.4 `display`

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S29** | Helligkeit setzen (`display_brightness_set?value=`, 0..15) | S | `numvar[idx=6].value` (`DISPLAY_BRIGHTNESS`) trägt den Wert, Antwort enthält `display_brightness` mit demselben Wert | Vorher S24 ausschalten, sonst stellt der LDR den Wert gleich wieder um |
| **S30** | Helligkeit `16` setzen | S | `error=2`, Detail `value out of range (0..15)`, `numvar[idx=6].value` **unverändert** | **Weist heute ab.** Früher klemmte der ESP still auf 15; am Gerät gegengeprüft (04.10.2026, ESP 3.2.21). Die Prüfung bleibt stehen — ein Rückfall auf die Klemmung fiele sonst niemandem auf |
| **S31** | `display_brightness_set` ohne `value` | S | `error=1`, `numvar[idx=6].value` unverändert | Leer darf nicht `0` heissen — das stellt die Uhr dunkel (L29) |
| **S32** | Display-Modus wählen (`display_mode_set?value=`) | S | `numvar[idx=4].value` (`DISPLAY_MODE`) trägt den Index, Antwort enthält `display_mode` | Die Obergrenze hängt an der geladenen Layout-Tabelle, nicht an einer festen Zahl |
| **S33** | Display-Modus oberhalb der Modusliste setzen | S | `error=2`, Detail `value out of range (0..<höchster Index>)`, `numvar[idx=4].value` **unverändert** | **Weist heute ab**, wie S30. Die Obergrenze ist ein Laufzeitwert: Steht nur ein Modus bereit, lautet das Detail `(0..0)` — so am Gerät gemessen (04.10.2026, ESP 3.2.21) |
| **S34** | Display-Farbe setzen (`display_color_set?red=&green=&blue=`, je 0..63) | S | `dspcolor[idx=0].red/green/blue` tragen die Werte | Sofort am Display sichtbar. Fehlende Anteile behalten ihren alten Wert (L37) |
| **S35** | Weisskanal setzen (`display_color_set?white=`, 0..63) | S | `dspcolor[idx=0].white` trägt den Wert | **Nur wenn `numvar[idx=0].value` (`DISPLAY_USE_RGBW`) auf 1 steht.** Sonst setzt der ESP `white` bedingungslos auf 0 — dann als „nicht prüfbar" protokollieren |
| **S36** | `display_color_set` ohne jeden Farbanteil | S | `error=1`, `dspcolor[idx=0]` unverändert | Verhindert einen EEPROM-Schreibzyklus ohne Inhalt |
| **S37** | „ES IST" dauerhaft umschalten (`display_it_is_set?value=on\|off`) | S | Bit `0x01` in `numvar[idx=7].value` (`DISPLAY_FLAGS`) gesetzt bzw. gelöscht, **die übrigen Bits unverändert** | Vier Schalter teilen sich diese eine Variable — die Gegenprobe gehört auf das Bit, nicht auf den Zahlenwert |
| **S38** | Tickertext setzen (`ticker_set?value=`, 32 Zeichen) | S | `strvar[idx=0].value` (`TICKER_TEXT`) trägt den Text | Umlaute und Sonderzeichen mitprüfen: Gekürzt wird nach **Bytes**, nicht nach Zeichen (L46) |
| **S39** | Tickertext leeren | S | `strvar[idx=0].value` ist leer | **Bewusste Ausnahme:** Der leere Text ist zulässig und der einzige Weg, den Ticker abzuschalten. Hier darf **keine** Fehlermeldung kommen |
| **S40** | Datumsformat setzen (`date_ticker_format_set?value=`, 5 Zeichen) | S | `strvar[idx=11].value` (`DATE_TICKER_FORMAT`) trägt das Format | Leerer Wert ⇒ `error=1`. Gegenprobe am Display: `%d.%m` muss das Datum ergeben |
| **S41** | Tickerverzögerung setzen (`ticker_deceleration_set?value=`, 0..255) | S | `numvar[idx=31].value` (`TICKER_DECELRATION`) trägt den Wert, Antwort enthält `ticker_deceleration` | **Teilprüfung S41b** (`value=256`, im Protokoll vom 04.10.2026 als `S41c` geführt — siehe 6.0a): **weist heute ab** — `error=2`, Detail `value out of range (0..255)`, Variable unverändert. Früher klemmte der Wert still auf 255; am Gerät gegengeprüft (04.10.2026, ESP 3.2.21) |
| **S42** | Dimmkurve von Hand ändern (`display_dim_level_set?idx=&value=`, je 0..15) | S | `num8array[var=0][idx=N].value` trägt den Wert | Sechzehn Stufen. Mindestens Stufe 0, 7 und 15 prüfen; fehlender `idx` traf früher Stufe 0 (L50) ⇒ heute `error=1` |
| **S43** | Dimmkurven-Vorgabe anwenden | S | Alle sechzehn `num8array[var=0][idx=0..15].value` entsprechen der gewählten Vorgabe | Schreibt sechzehn Werte auf einmal — **Ausgangskurve vorher vollständig notieren**, sonst ist sie nicht rücknehmbar |
| **S44** | TFT-Flags setzen (`tft_flags_set?rgb=&hflip=&vflip=`) | S | `numvar[idx=5].value` (`SSD1963_FLAGS`) trägt die Bits `0x01`, `0x02`, `0x04` | Ohne TFT als „nicht prüfbar" protokollieren. Teilprüfung **S44a**, nur **ein** Parameter gesendet: **Abweisung** `error=1`, das Detail nennt den ersten fehlenden Parameter in der Reihenfolge `rgb`, `hflip`, `vflip` (etwa `hflip must be on or off`), `numvar[idx=5].value` unverändert. Seit ESP 3.2.26 (E12); bis dahin baute der Handler die Flags von `0` auf, und ein nicht gesendeter Parameter **löschte** das Flag |
| **S45** | RGBW-Umschaltung (`display_use_rgbw_set?value=on\|off`) | S | `numvar[idx=0].value` (`DISPLAY_USE_RGBW`) 1 bzw. 0 | **Kein Bedienelement in der Oberfläche** — nur über den Backup-Import erreichbar. Prüfung per direktem Aufruf. Ändert die Wirkung von S35 |

**Der Displaytest (`test_display`) ist Klasse G — siehe Phase 8.**

### 6.5 `animations`

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S46** | Anzeigeanimation wählen (`animation_mode_set?value=`) | S | `numvar[idx=10].value` (`ANIMATION_MODE`) trägt den Index | Nicht-numerischer Wert ⇒ `error=1`, früher wurde `"abc"` zu 0 (L49) |
| **S47** | Farbanimation wählen (`color_animation_mode_set?value=`) | S | `numvar[idx=15].value` (`COLOR_ANIMATION_MODE`) | **Die Display-Farbe aus S34 ist danach verloren**, nicht nur während der Animation wirkungslos — den Ausgangswert von `dspcolor[idx=0]` vorher aus dem Rohabzug notieren, wie bei S51/S53. Am Gerät belegt (L150, 04.10.2026): vorher `0/0/0/63`, nach `color_animation_mode_set=2` und **sofortigem** Zurücksetzen auf `0` stand dort `0/0/31/0`. Überschrieben wird **einmalig beim Moduswechsel**, nicht laufend — nach manuellem Zurückschreiben blieb der Wert über 60 s stabil. Die Klasse bleibt **S**, weil der Schritt mit notiertem Ausgangswert rücknehmbar ist; was fällt, ist die Zusage „folgenlos" |
| **S48** | Verzögerung eines Anzeigeprofils setzen (`animation_profile_set?idx=&deceleration=`, 1..15) | S | `dispanim[idx=N].dcl` trägt den Wert **und** Bit `0x02` in `dispanim[idx=N].flags` steht unverändert | Mindestens zwei verschiedene Profile prüfen. **Derselbe Aufruf schreibt das Favoritenflag mit** — ohne `favourite` wird Bit `0x02` **gelöscht** (siehe S49). Das Prüfmuster sichert sonst nur `.dcl`, und der Verlust fällt nicht auf, weil die Gegenprobe ihn nicht ansieht: **vorher auch `.flags` notieren, nachher mitprüfen, und beim Zurückschreiben `favourite` mitsenden, falls es gesetzt war.** Gleiche Bauart wie S44 und S90, wo der Hinweis längst steht |
| **S49** | Profil als Favorit markieren (`animation_profile_set?...&favourite=on`) | S | Bit `0x02` in `dispanim[idx=N].flags` gesetzt bzw. gelöscht | Das Flag wird aus demselben Aufruf mitgeschrieben — ohne `favourite` wird es **gelöscht** |
| **S50** | Verzögerung `0` und `16` setzen | S | `error=2`, `dispanim[idx=N].dcl` unverändert | Gültig ist 1..15, nicht 0..15 |
| **S51** | Profilvorgabe zurücksetzen (`animation_profile_default?idx=`) | S | `dispanim[idx=N].dcl` gleicht `dispanim[idx=N].def_dcl` | **Unumkehrbar für dieses Profil** — den Ausgangswert vorher aus dem Rohabzug notieren |
| **S52** | Verzögerung eines Farbprofils setzen (`color_animation_profile_set?idx=&deceleration=`) | S | `coloranim[idx=N].dcl` trägt den Wert | |
| **S53** | Farbprofilvorgabe zurücksetzen (`color_animation_profile_default?idx=`) | S | `coloranim[idx=N].dcl` gleicht `coloranim[idx=N].def_dcl` | wie S51 |

### 6.6 `ambilight`

**Zu beachten (L11):** Am LED-Board dieser Uhr ist kein Ambilight-Ausgang
herausgeführt. Einstellungen lassen sich speichern, bleiben aber ohne sichtbare
Wirkung. Das ist **kein Fehler** — Schritt 4 des Prüfmusters entfällt und wird als
„nicht sichtbar prüfbar" protokolliert, nicht als „bestanden".

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S54** | Ambilight-Erkennung umschalten (`ambilight_online_set?value=on\|off`) | S | `numvar[idx=9].value` (`AMBILIGHT_IS_UP`) 1 bzw. 0 | Steuert, ob die Oberfläche das Modul überhaupt zeigt — bei `off` verschwindet das Panel. **Damit verschwindet auch der Schalter selbst: Der Rückweg führt nur noch über den direkten API-Aufruf** `ambilight_online_set?value=on`, nicht über die Oberfläche. Rücknehmbar bleibt der Schritt, aber nicht auf dem Weg, auf dem er gegangen wurde — vor dem Umschalten festhalten, wie er zurückzunehmen ist |
| **S55** | Helligkeit setzen (`ambilight_brightness_set?value=`, 0..15) | S | `numvar[idx=14].value` (`AMBILIGHT_BRIGHTNESS`) | |
| **S56** | Modus wählen (`ambilight_mode_set?value=`, 0..4) | S | `numvar[idx=11].value` (`AMBILIGHT_MODE`) | Fünf Modi: Normal, Uhr, Uhr 2, Regenbogen, Tageslicht |
| **S57** | LED-Zahl setzen (`ambilight_leds_set?value=`, 0..999) | S | `numvar[idx=12].value` (`AMBILIGHT_LEDS`) | |
| **S58** | LED-Zahl `1000` setzen | S | `error=2`, Detail `value out of range (0..999)`, `numvar[idx=12].value` **unverändert** | **Weist heute ab**, wie S30; am Gerät gegengeprüft (04.10.2026, ESP 3.2.21) |
| **S59** | Versatz setzen (`ambilight_offset_set?value=`, 0..999) | S | `numvar[idx=13].value` (`AMBILIGHT_OFFSET`) | |
| **S60** | Ambilight-Farbe setzen (`ambilight_color_set?red=&green=&blue=&white=`) | S | `dspcolor[idx=1].red/green/blue/white` | Weisskanal nur bei RGBW, siehe S35 |
| **S61** | Markierungsfarbe setzen (`marker_color_set?...`) | S | `dspcolor[idx=2].red/green/blue/white` | |
| **S62** | Mit dem Display synchronisieren (`sync_ambilight_set?value=on\|off`) | S | Bit `0x02` in `numvar[idx=7].value` (`DISPLAY_FLAGS`) | Dieselbe Variable wie S37 — Gegenprobe auf das Bit |
| **S63** | Markierungen synchronisieren (`sync_markers_set?value=on\|off`) | S | Bit `0x04` in `numvar[idx=7].value` | |
| **S64** | Sekunden überblenden (`fade_clock_seconds_set?value=on\|off`) | S | Bit `0x08` in `numvar[idx=7].value` | |
| **S65** | Sekundenmarkierung (`ambilight_markers_set?value=on\|off`) | S | Bit `0x02` in `almode[idx=1].flags` (`CLOCK_AMBILIGHT_MODE`) | Einziger Schalter dieser Gruppe, der **nicht** in `DISPLAY_FLAGS` landet, sondern im Uhr-Modus des Ambilights |
| **S66** | Dimmkurve von Hand ändern (`ambilight_dim_level_set?idx=&value=`) | S | `num8array[var=1][idx=N].value` | wie S42 |
| **S67** | Dimmkurven-Vorgabe anwenden | S | Alle sechzehn `num8array[var=1][idx=0..15].value` | wie S43 — Ausgangskurve vorher vollständig notieren |
| **S68** | Verzögerung eines Ambilight-Profils (`ambilight_mode_profile_set?idx=&deceleration=`, 0..15) | S | `almode[idx=N].dcl` | Hier ist **0 gültig**, anders als bei S48 |
| **S69** | Ambilight-Profilvorgabe zurücksetzen (`ambilight_mode_profile_default?idx=`) | S | `almode[idx=N].dcl` gleicht `almode[idx=N].def_dcl` | unumkehrbar, wie S51 |

### 6.7 `overlays` und `timers` — die beiden dynamischen Module

Hier liegt die grösste Lücke einer oberflächlichen Prüfung: **null statische
Bedienelemente.** Beide Module bauen ihre Oberfläche zur Laufzeit. Eine Prüfung, die
nur das Markup abläuft, übersieht sie vollständig.

**Drei Anläufe endeten vorher** — am Hänger (02.10.2026), am Watchdog-Reset aus L45
und am Variablenverlust aus L103. **Der vierte kam durch:** Am 03.10.2026
(ESP 3.2.15) sind S70 bis S85 vollständig gefahren worden,
`tools/snapshots/abschluss-3.2.15/ergebnisse.tsv`. Dieselbe Reihe deckte 100 der 114
benannten Prüfungen ab; offen blieben S8 und die Dateigruppe S100 bis S114.

**Aufräumen nach dem Mitschnitt, nicht nach der Planung.** Jeder Index, an den ein
Aufruf ging, wird zurückgesetzt — auch der eines abgewiesenen Aufrufs. Im Durchlauf
vom 03.10.2026 blieb Timer-Slot 5 als aktiver Timer auf 00:00 stehen und hätte die Uhr
jede Nacht zusätzlich ausgeschaltet (L81).

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S70** | Overlay anlegen (`overlay_set?idx=<n_overlays>&type=&date_code=`) | S | `numvar[idx=46].value` (`OVERLAY_N_OVERLAYS`) um 1 höher, `overlay[idx=N].type` trägt den Typ | Ein neues Overlay entsteht nur, wenn `idx` **genau** dem bisherigen Zählerstand entspricht |
| **S71** | Jeden Overlay-Typ durchschalten (`type=0..10`) | S | `overlay[idx=N].type` trägt jeden Wert | **Elf Typen**, nicht drei: keiner, Icon, Datum, Temperatur, Wettericon, Wetter, Ticker, MP3, Vorhersage-Icon, Vorhersage, Temperatur als Ziffern. `type=11` ⇒ `error=2` |
| **S72** | Icon-Overlay mit jedem vom Gerät gemeldeten Icon | S | `overlay[idx=N].text` trägt den Iconnamen | Die Liste kommt aus `/api/overlay_icons` und hängt an der hochgeladenen Icondatei — **nicht auf eine feste Anzahl festlegen**, sondern die gemeldete Liste vollständig durchgehen |
| **S73** | Text- und Ticker-Overlay (`value=`, 32 Zeichen) | S | `overlay[idx=N].text` trägt den Text, nach **Bytes** gekürzt | Umlaute mitprüfen (L46) |
| **S74** | MP3-Overlay | S | `overlay[idx=N].type` = 7, Ton hörbar | Ohne DFPlayer als „nicht prüfbar" protokollieren |
| **S75** | Datumscode durchschalten (`date_code=0..6`) | S | `overlay[idx=N].date_code` trägt den Wert | Sieben Codes: keiner, Rosenmontag, Ostern, Advent 1 bis 4. `date_code=7` ⇒ `error=2` |
| **S76** | Grenzwerte prüfen: `interval=0` (**S76a**), `duration=3` (**S76b**), `duration=10` (**S76c**), `days=0` (**S76d**) | S | Alle vier `error=2` mit Bereichsangabe — `interval out of range (1..255)`, `duration out of range (5..9)` (zweimal), `days out of range (1..255)`; `overlay[idx=N]` **unverändert** | **Weisen heute ab.** Früher wurden die vier Werte still auf 5, 5, 9 und 1 zurechtgebogen; alle vier am Gerät gegengeprüft (04.10.2026, ESP 3.2.21). Die vier Teilkennungen stehen hier, damit das Protokoll sie einzeln führen kann — eine Sammelzeile verdeckt, wenn nur drei davon gefahren wurden |
| **S77** | Startdatum setzen (`month=&day=`) | S | `overlay[idx=N].date_start` = `month*256 + day`; `month=0&day=0` ergibt `date_start` = 0 | `month=13` oder `day=32` ⇒ `error=2`. **Nur einer von beiden 0** (`month=0&day=12`) ⇒ ebenfalls `error=2`, Detail „must both be set or both be 0“ — am Gerät gemessen am 05.10.2026 (L302); hier stand bis dahin, das ergebe `date_start` = 0 |
| **S78** | Overlay aktiv schalten (`active=on\|off`) | S | Bit `0x01` in `overlay[idx=N].flags` | Ohne `active` wird das Flag **gelöscht** |
| **S79** | Overlay anzeigen (`overlay_display?idx=`) | S | `numvar[idx=45].value` (`DISPLAY_OVERLAY`) trägt den Index, Overlay erscheint am Display | |
| **S80** | Overlay löschen (`overlay_delete?idx=`) | S | `numvar[idx=46].value` um 1 kleiner, die folgenden Einträge rücken auf | **Letzten Eintrag eigens prüfen** — und den ersten, wenn mehrere stehen. **Die Rücknahme ist kein Setter, sondern ein Neuaufbau: den vollständigen Eintrag vorher aus dem Rohabzug notieren** — `type`, `text`, `date_code`, `interval`, `duration`, `days`, `date_start` und `flags`, nicht nur den Index. Fehlt eines davon, ist das Overlay nicht wiederherstellbar. **Und die nachfolgenden Indizes rücken auf:** Nach dem Löschen von `idx=N` trägt der bisherige `N+1` die Nummer `N`. Eine vorher notierte Aufräumliste zeigt danach auf den falschen Eintrag — die Zuordnung geht über den Inhalt, nicht über die Nummer. Am Overlay eines Nutzers ausgeführt, trifft das echte Daten, nicht Testeinträge |
| **S81** | `overlay_display` und `overlay_delete` **ohne** `idx` | S | Beide `error=1`, `numvar[idx=46].value` unverändert | Früher zeigte beziehungsweise **löschte** das den ersten Eintrag und meldete Erfolg (L70) |
| **S82** | Display-Timer setzen (`timer_set?idx=&from=&to=&hour=&minute=&active=&switch_on=`) | S | `nighttime[idx=N].minutes` = `hour*60+minute`, Bit `0x80` (aktiv) und `0x40` (einschalten) in `.flags`, Wochentage in den unteren Bits | **Acht Slots** (0..7). Ein aktiver Testtimer schaltet die Uhr im Wohnraum — Zeiten weit vom aktuellen Zeitpunkt wählen |
| **S83** | Ambilight-Timer setzen (`ambilight_timer_set?...`) | S | `ambinighttime[idx=N].minutes` und `.flags` entsprechend | Gleiche Prüfung, eigener Variablensatz |
| **S84** | Timer-Grenzfälle: `idx=8`, `from=7`, `hour=24`, `minute=60` | S | Jeweils `error=2`, der Slot unverändert | Diese vier wurden früher angenommen (L71) |
| **S85** | Mitternachtsübergang und Start gleich Ende | S | Ein Paar 22:00 ein / 06:00 aus steht als zwei Einträge im Abzug und schaltet über die Nacht hinweg richtig | Braucht entweder Geduld oder eine verschobene Gerätezeit — in beiden Fällen **nach S3** einplanen und danach wieder korrigieren |

### 6.8 `dfplayer`

Ohne angeschlossenes Modul meldet die PWA „offline" und blendet das Panel aus — dann
sind S86 bis S96 als „nicht prüfbar" zu protokollieren, nicht als „bestanden".

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S86** | Lautstärke setzen (`dfplayer_volume_set?value=`, 0..30) | S | `numvar[idx=34].value` (`DFPLAYER_VOLUME`), Antwort enthält `dfplayer_volume` | Hörbare Gegenprobe über S96 |
| **S87** | Lautstärke `31` setzen | S | `error=2`, Detail `value out of range (0..30)`, `numvar[idx=34].value` **unverändert** | **Weist heute ab**, wie S30 — belegt im Quelltext (`http_api_dfplayer_volume_set`), **am 05.10.2026 erstmals am Gerät gemessen** (L302): `error=2`, Detail `value out of range (0..30)`, Wert unverändert. Am 03.10.2026 (ESP 3.2.15) klemmte er noch still auf 30. Der Rohwert ist auch ohne angeschlossenen DFPlayer prüfbar — nur die hörbare Gegenprobe entfällt |
| **S88** | Modus wählen (`dfplayer_mode_set?value=`, 0..2) | S | `numvar[idx=37].value` (`DFPLAYER_MODE`): 0 aus, 1 Glocke, 2 Zeitansage | Fehlender Wert ⇒ `error=1`; leer hätte den Ton ganz abgeschaltet (L29) |
| **S89** | Glockenflags setzen (`dfplayer_bell_flags_set?m15=&m30=&m45=`) | S | `numvar[idx=38].value` (`DFPLAYER_BELL_FLAGS`) trägt die Bits `0x01`, `0x02`, `0x04` | |
| **S90** | Glockenflags mit nur **einem** gesendeten Parameter | S | **Abweisung** `error=1` mit dem Detail `m15 must be on or off` (bzw. dem Namen des fehlenden Parameters), `numvar[idx=38].value` **unverändert** | Seit ESP 3.2.26 (E12). Bis dahin baute der Handler die Flags von `0` auf, und die nicht gesendeten Bits wurden **gelöscht** — wer das heute sieht, hat einen Rückfall. Die Oberfläche sendet immer alle drei, ein direkter Aufruf nicht |
| **S91** | Sprechzyklus setzen (`dfplayer_speak_cycle_set?value=`, 0..255) | S | `numvar[idx=39].value` (`DFPLAYER_SPEAK_CYCLE`) | |
| **S92** | Stille ab setzen (`dfplayer_silence_start_set?hour=&minute=`) | S | `numvar[idx=35].value` (`DFPLAYER_SILENCE_START`) = `hour*60+minute` | |
| **S93** | Stille bis setzen (`dfplayer_silence_stop_set?hour=&minute=`) | S | `numvar[idx=36].value` (`DFPLAYER_SILENCE_STOP`) | |
| **S94** | Stille mit `hour=24` oder `minute=60` | S | `error=2`, Variable unverändert | |
| **S95** | Alarm setzen (`dfplayer_alarm_set?idx=&active=&from=&to=&hour=&minute=`) | S | `alarmtime[idx=N].minutes` = `hour*60+minute`, Bit `0x80` in `.flags` | **Acht Slots.** Aufräumen nach Mitschnitt, nicht nach Planung — ein stehengebliebener Alarm weckt den Nutzer |
| **S96** | Titel abspielen (`dfplayer_play?folder=&track=`, je 0..255) | S | `numvar[idx=44].value` (`DFPLAYER_PLAY_FOLDER_TRACK`) = `folder*256 + track`, Ton hörbar | Lautstärke vorher auf einen erträglichen Wert stellen |

### 6.9 `maintenance`

Der grösste Bereich: 24 Schaltflächen, 14 Felder, 8 Dateiauswahlfelder.

Was hier **nicht** steht, weil es in Phase 8 steht: `maintenance_reset_eeprom`,
`maintenance_format_fs`, `fs_remove`, `eeprom_settings_set`. Was in Phase 5 steht:
Sicherung exportieren und importieren.

**S100 bis S106 sind Klasse S, aber nicht folgenlos.** Alle sieben **überschreiben
eine Datei im LittleFS**, und der Rückweg ist kein Zurückschreiben eines Werts,
sondern ein erneutes Hochladen. **Er setzt damit voraus, dass die ersetzte Datei
lokal noch vorliegt.** Stammt die bisherige Datei aus einem früheren lokalen Upload
und liegt sie nicht mehr auf dem Update-Server, ist sie nach dem Schritt weg — den
Namen zu notieren genügt dafür nicht, die **Datei** muss dasein.

Vor der Reihe deshalb: Jede Datei, die S100 bis S106 anfassen, vorher **lokal
sichern** und die Sicherung benennen. Ohne diese Sicherung wird der Schritt
übersprungen und als „nicht prüfbar" protokolliert, nicht gefahren.

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S97** | Update-Host setzen (`update_host_set?value=`, 63 Zeichen) | S | `strvar[idx=9].value` (`UPDATE_HOST`) trägt den Wert | **Danach sofort gegen `DEVICE_UPDATE_HOST` aus `tools/device.conf` prüfen.** Ein falscher Host holt beim nächsten Update fremde Firmware, ohne Fehlermeldung (L42) |
| **S98** | Update-Pfad setzen (`update_path_set?value=`, 63 Zeichen) | S | `strvar[idx=10].value` (`UPDATE_PATH`) | wie S97 |
| **S99** | Update-Host mit 64 Zeichen | S | `error=2`, Detail nennt die Grenze in Byte (`value too long (max. 63 bytes)`), `strvar[idx=9].value` **unverändert** | Leerer Wert ⇒ `error=1`. Seit ESP 3.2.24 abgewiesen statt gekürzt, am Gerät gemessen am 05.10.2026 (L302) |
| **S100** | Tabellendatei vom Server laden (`update_download_table`) | S | Die Datei erscheint in `/api/fs_list` mit Grösse > 0, danach stehen in `settings_xml` neue `dispmode`-Einträge | Wechselt das Layout der Uhr — **Ausgangstabelle vorher aus `/api/update_status` notieren**. Der Name allein reicht nicht: **Rückweg nur mit der lokal gesicherten Ausgangsdatei** (Vorbemerkung zu S100 bis S106). Kam die bisherige Tabelle aus S102 und liegt nicht auf dem Server, ist sie danach weg |
| **S101** | Icon- und Wetterdatei vom Server laden (`update_download_assets`) | S | Beide Dateien in `/api/fs_list` mit Grösse > 0 | Der Endpunkt meldet `{"ok":true}` auch dann, wenn er nur einen Teil geladen hat — Beleg ist die Dateiliste. **Rückweg nur mit den lokal gesicherten Ausgangsdateien** |
| **S102** | Tabellendatei lokal hochladen (`fs_upload_tables`) | S | Datei in `/api/fs_list`, Grösse gleich der lokalen Datei | **Rückweg nur mit der lokal gesicherten Ausgangsdatei** — die überschriebene ist nicht vom Gerät zurückzuholen |
| **S103** | Icondatei lokal hochladen (`fs_upload_icon`) | S | wie S102 | wie S102, einschliesslich des Rückwegs |
| **S104** | Wetterdatei lokal hochladen (`fs_upload_weather`) | S | wie S102 | wie S102, einschliesslich des Rückwegs |
| **S105** | Displaydatei lokal hochladen (`fs_upload_display`) | S | wie S102 | wie S102, einschliesslich des Rückwegs |
| **S106** | App-Dateien hochladen (`app_file_upload`) | S | `./tools/install-app.sh --check` meldet alle Dateien der Weissliste mit Grösse > 0 | Die PWA überschreibt sich selbst. Danach Seite neu laden und prüfen, dass sie noch startet. **Der heikelste Schritt der Gruppe:** Eine leere oder abgebrochene Datei ist der belegte Weisschirm-Fall — die Oberfläche ist dann weg, und mit ihr der Weg, sie wieder hochzuladen. **Der Rückweg heisst `./tools/install-app.sh`** und läuft vom Rechner aus, nicht über die PWA; er setzt die vollständigen `.gz`-Dateien lokal voraus. Vor dem Schritt prüfen, dass sie da sind, und den Rückweg danach mit `./tools/install-app.sh --check` nachweisen |
| **S107** | **Leere Datei** hochladen | S | Der ESP weist sie ab, die bestehende Datei bleibt unverändert | **Erwarteter Befund:** ESP-seitig abgefangen (Prüfung auf Grösse > 0), **PWA-seitig steht die Prüfung noch aus** (Massnahme 7). Eine leere `.gz` hat schon einmal einen weissen Bildschirm erzeugt |
| **S108** | Dateiliste und Speicherstand (`fs_list`, `fs_info`) | L | Jede Datei der Weissliste erscheint mit Grösse > 0; der freie Platz ist grösser als die grösste geplante Hochladedatei | Lesend — gehört eigentlich in Phase 2, steht hier als **Vorbedingung** für S102 bis S107 |
| **S109** | Datei anzeigen (`fs_show?filename=`) | L | Der Inhalt erscheint in der Vorschau, lange Zeilen brechen um | Lesend |
| **S110** | ESP-Firmware aktualisieren (`remote_esp_update`) | **R** | Nach dem Wiederanlauf meldet `/api/update_status` die neue ESP-Version | **Danach zwingend:** `./tools/install-app.sh --check`, `./tools/smoke-device.sh` und die Prüfung der Update-Quelle (S97/S98). Ein ESP-Neustart hinterlässt den Variablensatz unvollständig (L42, L103) |
| **S111** | STM32 flashen über den Server (`remote_stm32_flash`) | **R** | `/api/update_status` meldet die neue STM-Version | **Nicht von Hand aufrufen** — `./tools/flash-stm.sh` benutzen (DIR-010). `filename` ist Pflicht, und bei `HARDWARE_CONFIGURATION` = 65535 weist der ESP jeden Namen ab |
| **S112** | STM32 lokal flashen (`local_stm32_flash`) | **R** | wie S111 | Gleiche Fallen, zusätzlich die Dateigrösse der hochgeladenen Binärdatei |
| **S113** | ESP neu starten (`local_esp_restart`) | **R** | Das Gerät ist binnen 30 s wieder erreichbar, `reconnect_probe` antwortet | **Danach den Variablensatz prüfen** — genau hier tritt L103 auf |
| **S114** | STM zurücksetzen (`maintenance_reset_stm32`) | **R** | Die Uhr zeigt die Startsequenz, danach ist `numvar[idx=29].value` (`HARDWARE_CONFIGURATION`) wieder ungleich 65535 | Das ist zugleich die **Reparatur** für den Zustand aus 6.0d — aber der Schritt trifft **nicht nur den STM, er reisst den ESP mit** (L183). Alle ESP-Zähler (`write_lost_bytes`, `write_lost_blocks`, `no_request_aborts`, `update_cache_hits`) stehen danach auf **0**, und der Variablenverlust aus L42/L103 kann eintreten. **Messreihen über einen STM-Reset hinweg sind ungültig**, jede laufende Messreihe wird dadurch wertlos — am 04.10.2026 wurde `write_lost_blocks: 0` fälschlich als Erfolg gelesen, obwohl der Zähler nur zurückgesetzt war. Belegt durch drei unabhängige Fälle mit vollständiger ESP-Startsequenz danach und durch `update_cache_hits` 5 → 0, einen reinen ESP-Zähler ohne STM-Bezug. **Vermutete Ursache, elektrisch und nicht im Code:** Der Handler ruft ausschliesslich `stm32_reset()`; der STM schaltet beim Start über `PB0` die 5-V-Versorgung der LED-Kette, und der Einschaltstrom zieht die Versorgung so weit herunter, dass der ESP mitgeht — dieselbe Stromreserve, die bei `test_display` nach 44 s zum Brownout führt. **Danach zwingend:** Variablensatz prüfen und die Update-Quelle gegen `tools/device.conf` (S97/S98) |

---

## 7. Phase 4 — Grenzfälle

Für jedes Eingabefeld dieselben acht Klassen. Nicht stichprobenartig — **jedes Feld**.

**Auch hier gilt V1 aus Abschnitt 5b:** `d=` vor und nach jeder Feldreihe. Ein
abgewiesener Wert ist erst dann belegt abgewiesen, wenn im selben Fenster nichts
verworfen wurde.

**Zwischen zwei Fällen wird die Seite neu geladen** — oder das Feld nachweislich auf
seinen Ausgangswert gesetzt. Ein Prüfskript, das Werte in Nachbarfeldern stehen lässt,
schreibt sie beim nächsten Speichern mit. Im Testdurchlauf S.26 (08.10.2026) stand so die
Zeitzone auf 10, und weil die PWA nach dem Speichern der Zeitzone selbst die Netzzeit holt
(`saveTimezone()`), lief die Produktivuhr drei Minuten auf 10:18. Felder mit Nebenwirkung
nach dem Speichern — Zeitzone, Datum, Uhrzeit — **zuletzt** und einzeln.

**Eingaben gehen als echte Ereignisse ins Feld, nicht als Zuweisung.** Das Werkzeug
tippt über das DevTools-Protokoll (Tastenereignisse über CDP), es setzt nicht
`input.value` per Skript. Der Grund steht in `app.js`: `handleDirtyFormInteraction()`
ignoriert Ereignisse mit `isTrusted=false`. Ein programmatisch gesetzter Wert gilt damit
nicht als Bearbeitung, und der nächste Abfragezyklus überschreibt ihn mit dem
Gerätewert. Gespeichert wird dann etwas anderes, als das Protokoll behauptet. Am
10.10.2026 hat genau das einen **Scheinbefund** erzeugt — die Abweichung lag am
Werkzeug, nicht an der Oberfläche.

**Zeitfelder sind der gefährlichste Fall davon.** Ein „Leeren“ per Skript ist kein
echtes Ereignis: Die Abfrage füllt das Feld wieder mit der aktuellen Zeit auf, und
gespeichert wird diese Zeit **mit Sekunde 0**. Die RTC geht danach um die verstrichenen
Sekunden nach — am 10.10.2026 zweimal rund 17 s. Im Rohabzug sieht man das nicht, er ist
nur minutengenau (Phase 2). **Nach jedem solchen Fall sofort die Netzzeit holen**
(S6), nicht erst am Ende; so am 10.10.2026 korrigiert (`BEFUNDE.md`, L370).

| | Eingabe | Erwartung |
|---|---|---|
| **E1** | leer lassen und speichern | definierte Reaktion, keine stille Löschung |
| **E2** | Untergrenze | akzeptiert |
| **E3** | Untergrenze − 1 | abgewiesen **mit Meldung** |
| **E4** | Obergrenze | akzeptiert |
| **E5** | Obergrenze + 1 | abgewiesen **mit Meldung** |
| **E6** | maximale Länge voll ausschöpfen | vollständig gespeichert |
| **E7** | Länge + 1 (per Einfügen, nicht per Tippen — `maxlength` greift beim Tippen) | definiert gekürzt oder abgewiesen |
| **E8** | Sonderzeichen: `äöü`, `<script>`, `"`, `&`, `%20`, Emoji, führende Leerzeichen | kein Absturz, keine Zeichenverfälschung |

**Zu E3 und E5 — die Erwartung hat sich gedreht.** Hier stand, die Oberfläche biege
Werte ausserhalb des Bereichs **still zurecht** (Massnahme 17), der Durchlauf
bestätige das nur. Das trifft nicht mehr zu: `clampNumber()` ist aus `app.js`
entfallen, und auf der Geräteseite wurden acht Setter gemessen, die **abweisen**
statt zu klemmen (04.10.2026, ESP 3.2.21 — Einzelheiten in Kapitel 13).

**Erwartung heute:** abgewiesen **mit Meldung**, und zwar an jedem Feld. Festzuhalten
ist nicht mehr, wo geklemmt wird, sondern **wo noch geklemmt wird** — jede solche
Stelle ist jetzt ein Befund und keine Bestätigung. Zeitserver, Update-Host, Ticker und
Overlay-Zeile weisen seit ESP 3.2.24 nach Byte-Länge ab (S10, S99; am 10.10.2026 an allen
Feldern bestätigt, L370) — eine stille Kürzung dort wäre ein **Rückfall**. Ausdrücklich
noch offen und kein Rückfall ist nur der **Legacy-Zweig**, der unverändert zurechtbiegt
(L199).

Die konkreten Grenzen, aus dem Markup erhoben:

| Feld | Grenze |
|---|---|
| Tag / Monat / Jahr | 1..31 / 1..12 / 2000..2999 |
| Stunde / Minute | 0..23 / 0..59 |
| Zeitzone | −12..14 |
| Temperaturkorrektur RTC und DS18xx | −20..20 |
| Helligkeit Display und Ambilight | 0..15 |
| Weisskanal (drei Felder) | 0..63 |
| Tickerverzögerung, Sprechzyklus, Ordner, Titel | 0..255 |
| Ambilight LEDs und Versatz | 0..999 |
| DFPlayer-Lautstärke | 0..30 |
| AP-SSID, SSID (freies Feld), AppID, Ort, Tickertext | 32 Zeichen |
| WLAN-Schlüssel, AP-Schlüssel | 64 Zeichen |
| Zeitserver | 16 Zeichen |
| Längen- und Breitengrad | 8 Zeichen |
| Datumsformat | 5 Zeichen |
| Update-Host und -Pfad | 63 Zeichen |
| Kartenmodal: Suche / Ort / Längengrad / Breitengrad | 64 / 32 / 8 / 8 Zeichen |

**Zusätzlich, feldübergreifend:**

- Datum **31. Februar** — wird es abgewiesen?
- Längengrad `8,5400` mit Komma statt Punkt
- Zeitzone `+2` mit führendem Pluszeichen
- Zweimal schnell hintereinander auf dieselbe Speichern-Schaltfläche
- **Zeitfeld im Timer leeren**, aktiv setzen, speichern — mit echten Tastenereignissen.
  Soll: **Abweisung mit Meldung**, kein Aufruf ans Gerät. Am 10.10.2026 gespeichert als
  aktiver Timer 00:00, ohne Meldung (`BEFUNDE.md`, L365) — ein solcher Timer schaltet die
  Uhr jede Nacht um Mitternacht. Sofort zurückstellen und den Slot in die Aufräumliste
  (Kapitel 2). Dieselbe Rückfallstelle steht im Code bei den DFPlayer-Alarmen; dort am
  Gerät noch nicht geprüft.
- **Zwei Felder ändern, eines speichern, Modul wechseln.** Soll: **Warnung** für das
  ungespeicherte Feld. Am 10.10.2026 kam kein Dialog, die zweite Änderung ging still
  verloren (L366): Das Speichern des einen Felds leert die Merkliste aller Felder.
- Ohne offene Änderung das Modul wechseln — erscheint die Warnung trotzdem?
  **Hier lag die Erwartung, dass sie auch ohne offene Änderung erscheint**
  (Massnahme 4). Am 04.10.2026 (PWA 1.4.88) nicht beobachtet:
  vier Modulwechsel ohne Bearbeitung, null Dialoge; mit offener Änderung erschien er
  korrekt. **Der Pfad „nach normalem Speichern" ist am 05.10.2026 nachgeholt worden**
  (05.10.2026, PWA 1.4.90, L302): erst speichern, dann ohne weitere Eingabe das Modul wechseln —
  kein Dialog. Der Punkt bleibt im Plan, damit ein Rückfall auffällt.

---

## 8. Phase 5 — Backup und Restore als eigener Prüfgegenstand

Das ist kein Nebenschauplatz. Der Import kann die Uhr ohne WLAN, ohne AP und ohne
Webserver zurücklassen — er ist die einzige **Klasse-G**-Funktion, die dieser Plan
trotzdem scharf ausführt, weil sie sonst nie geprüft würde.

### 8.1 Was das Backup umfasst

Elf Abschnitte: `display`, `network`, `maintenance`, `climate`, `animations`, `tft`,
`ambilight`, `dfplayer`, `overlays`, `timers` und seit F1 `ir`, dazu ein Kopf mit
Quelle und Versionsstand und ein `assets`-Block, der **nur Namen** enthält
(Layout-Tabelle, verwendete Icons, Asset-Präfix) — **keine Dateien**.

`ir` ist der einzige Abschnitt, der **fehlen darf**, ohne dass die Datei ungültig ist:
Lässt sich kein vollständiger Abzug der zwanzig Tasten holen, wird er weggelassen statt
teilbefüllt geschrieben. Siehe 8.4.

### 8.2 Prüfreihe

| | Prüfung | Erwartung |
|---|---|---|
| **B1** | Export bei vollständig erreichbarem Gerät | Datei enthält alle zehn Abschnitte, gültiges JSON |
| **B2** | Export, während `eeprom_settings` nicht antwortet | **Abbruch mit Meldung**, keine halbe Datei. Die Netzwerkdaten fehlten sonst stillschweigend |
| **B3** | Export, wenn die WLAN-SSID leer ist | Hinweis „Netzwerkteil übersprungen", Export läuft weiter |
| **B4** | Zwei Exporte hintereinander ohne Änderung | byteweise identisch bis auf den Zeitstempel |
| **B5** | Import der **unveränderten** eigenen Sicherung | Uhr landet im Ausgangszustand, Rohabzug identisch |
| **B6** | Import mit **einer** geänderten Einstellung | genau diese ändert sich, keine andere |
| **B7** | Import mit **leeren** Netzwerkfeldern | Zugangsdaten bleiben erhalten. Das ist der Fix in ESP 3.2.5: leeres Feld heisst „nicht ändern", nicht „löschen". **Vorher hat genau das die Uhr aus dem Netz geworfen** |
| **B8** | Import einer Datei, die kein JSON ist | klare Fehlermeldung, nichts wird geschrieben |
| **B9** | Import eines JSON ohne `settings` | abgewiesen |
| **B10** | Import mit unbekanntem Zusatzfeld | ignoriert, Rest läuft |
| **B11** | Import abbrechen in der Bestätigungsabfrage | nichts geschrieben |
| **B12** | Import, der einen Neustart auslöst | Wiederanlauf wird abgewartet und erkannt |

**B5 ist der eigentliche Beweis der Restore-Fähigkeit** und gleichzeitig die
Wiederherstellung für Phase 9. **B7 ist der wichtigste Einzeltest des ganzen Plans** —
er prüft genau den Fehler, der die Uhr schon einmal unerreichbar gemacht hätte.

### 8.3 Die Netzwerksektion beim Import — die eine Stelle, an der doch geschrieben wird

Phase 8 schliesst aus, die WLAN-Zugangsdaten über das Netzwerkmodul zu setzen. **Der
Import umgeht dieses Modul**: `importNetworkSettings()` schreibt die Netzwerksektion
des Backups zurück. Damit ist der Import der einzige Weg, auf dem im ganzen Durchlauf
doch an den Zugangsdaten gerührt wird. Deshalb gelten hier drei harte Regeln:

1. **Importiert wird ausschliesslich die eigene, unmittelbar vorher erzeugte
   Sicherung.** Keine ältere Datei, keine von Hand bearbeitete, keine von einem
   anderen Gerät. Die Netzwerksektion darin enthält genau die Zugangsdaten, die
   ohnehin aktiv sind.
2. **Vor jedem Import wird die Netzwerksektion der Datei gelesen** und gegen
   `eeprom_settings` geprüft. Weicht sie ab, wird nicht importiert.
3. **B6 (eine geänderte Einstellung) ändert nie etwas aus `network`.** Dafür wird eine
   Anzeigeeinstellung genommen.

**B7 ist davon die Ausnahme und zugleich die Absicherung:** Er importiert eine
Sicherung mit **leeren** Netzwerkfeldern. Genau dieser Fall war früher gefährlich —
leer hiess „löschen". Seit ESP 3.2.5 heisst leer „nicht ändern". B7 weist nach, dass
der Schutz greift, und ist damit die Voraussetzung dafür, B5 und B12 überhaupt zu
wagen. **Reihenfolge ist deshalb nicht beliebig: B7 vor B5.**

**Vor B5, B7 und B12:** AP-Zugangsdaten griffbereit, serieller Zugang erreichbar,
Legacy-Oberfläche in einem zweiten Tab offen.

### 8.4 Die IR-Codes im Backup — eigene Prüfreihe

Seit F1 enthält die Sicherung einen elften Abschnitt, `settings.ir` mit genau zwanzig
Einträgen. Er bekommt eine eigene Reihe, weil er sich in zwei Punkten von allen
anderen unterscheidet.

**Erstens ist er die einzige Konfiguration ohne zweiten Rückweg.** Ein falsch
geschriebener IR-Code macht die Taste unbrauchbar, und zurück kommt man nur über
`learn_ir` — das unbegrenzt blockiert und deshalb in Phase 8 gesperrt ist.

**Zweitens entsteht er asynchron.** Alle anderen Abschnitte werden aus dem lokalen
Zustandsabbild abgeleitet; dieser hier braucht einen Geräte-Round-Trip über die
Kommandobrücke, zwanzig Einzelkommandos, je eines pro Hauptloop-Durchlauf. Der Export
dauert dadurch 1 bis 2 Sekunden länger, im schlechtesten Fall 12,5 Sekunden.

| | Prüfung | Erwartung |
|---|---|---|
| **B13** | Export bei erreichbarem Gerät | `settings.ir.keys` hat **genau 20** Einträge, jeder mit `name` und `index`. Nie angelernte Tasten: alle drei Wertfelder `null`, nicht nur `protocol` |
| **B14** | Export unmittelbar nach einem ESP-Neustart | Abschnitt `ir` **fehlt ganz**, Hinweis nennt den Grund. Der Puffer liegt nur im RAM — eine Datei mit zwanzig leeren Tasten, die wie eine gültige aussieht, wäre schlimmer als keine |
| **B15** | Zwei Exporte hintereinander | `ir`-Abschnitt identisch. Ergänzt B4 |
| **B16** | `/api/ir_codes_get` **ohne** vorherigen Anstoss | `requested:false`, `received:0`, alle Indizes in `missing[]`, `codes[]` leer — rein lesend, Klasse **L** |
| **B17** | Import einer Sicherung mit `null`-Einträgen | übersprungen, **nicht** als Löschung geschrieben. Gegenprobe: Abzug danach unverändert |
| **B18** | Gegenprobe nach dem Import (AK16) | Erneuter Abzug zeigt die importierten Werte. **Das ist der einzige Nachweis** — `{"ok":true}` heisst nur „abgeschickt", nicht „gespeichert" |
| **B19** | Import einer **Version-2-Datei** (ohne `ir`-Abschnitt) | Läuft durch, Hinweis „stammt aus einer älteren App-Version". Prüft den L82-Pfad, der **vor F1 nie feuern konnte** |

**B19 ist leicht zu übersehen und deshalb eigens genannt.** Die Vorarbeit aus L82
— ältere Sicherungen annehmen statt abweisen — war monatelang unauslösbar, weil
`BACKUP_VERSION` nie gestiegen ist. Mit F1 steigt sie auf 3, und ab diesem Moment
läuft **jede bestehende Sicherung des Nutzers** über diesen Pfad. Ein Fehler darin
träfe also nicht einen Randfall, sondern den Normalfall.

**Klasse S, mit einer Einschränkung:** B17 und B18 schreiben über
`/api/ir_code_set`. Der Endpunkt steht in der Gefahrenliste und wird von
`tools/hooks/no-danger.py` abgewiesen — bewusst, und nicht zu umgehen. Diese beiden
Prüfungen laufen deshalb **nur mit ausdrücklicher Freigabe des Nutzers im selben
Gespräch**. B13 bis B16 und B19 sind davon nicht betroffen.

`/api/ir_codes_request` ist **nicht** gesperrt und darf es auch nicht werden, sonst
ist der Export nicht prüfbar. Der Filter trifft exakt `ir_code_set`.

**Was auch mit Freigabe offen bleibt:** Ob die Uhr auf die wiederhergestellten Codes
tatsächlich *reagiert*, lässt sich nur mit der Fernbedienung in der Hand feststellen.
Das bleibt beim Nutzer und darf nicht stillschweigend als geprüft gelten.

---

## 9. Phase 6 — Verbindung, Offline, Service Worker

| | Prüfung |
|---|---|
| **V1** | Uhr im Betrieb vom Netz trennen — wie reagiert die Oberfläche? Erscheint ein Hinweis oder friert sie stumm ein? |
| **V2** | Wiederverbinden — findet `reconnect_probe` von selbst zurück? |
| **V3** | Fenster in den Hintergrund legen — **pausiert das Polling?** Am 04.10.2026 (PWA 1.4.88) kam bei verborgenem Tab über 60 s kein Request an, nach dem Sichtbarwerden 7 in 12 s. Die frühere Erwartung „läuft weiter" (R2-11) trifft damit nicht mehr zu. **Offen bleibt der Fall mit laufendem Update:** Gemessen wurde ohne. Zu prüfen ist, ob ein laufender Fortschrittsabruf die Pause aushebelt — und dass die Oberfläche nach dem Sichtbarwerden **aktuelle** Werte zeigt, nicht die eingefrorenen von vorher |
| **V4** | Tab offen lassen, ESP neu starten — erkennt die PWA das? |
| **V5** | Zwei Browser gleichzeitig, in beiden dieselbe Einstellung ändern — was gewinnt? |
| **V6** | Seite neu laden während eines laufenden Speichervorgangs |
| **V7** | Service Worker: Installation über `http://` schlägt fehl, weil kein sicherer Kontext. **Die Erwartung „kein Hinweis" (R2-10) ist fraglich geworden** — `reportInsecureContextOnce()` meldet den Fall heute einmal je Sitzung. Am Gerät nicht gegengeprüft: Erscheint der Hinweis beim ersten Laden, bleibt er beim Neuladen aus, und nennt er den Grund? |
| **V8** | Nach einem App-Update: Lädt der Service Worker die neue Fassung, oder bleibt eine alte im Cache? `CACHE_NAME` muss sich mit `APP_VERSION` ändern |
| **V9** | Legacy-Oberfläche bleibt parallel bedienbar |

---

## 10. Phase 7 — Darstellung

**Bereits automatisiert.** `./tools/preview/shot.sh --diag` misst 20 Formate von
320×568 bis 5120×1440, Hoch- und Querformat, und meldet je Format horizontalen
Überlauf, abgeschnittenen Text, Touch-Ziele unter 44 px, Nicht-Text-Kontrast und das
Modalverhalten.

Zu ergänzen bleibt, was sich nicht messen lässt: Lesbarkeit bei Sonnenlicht,
Bedienbarkeit mit einer Hand, Verhalten bei 200 % Systemschriftgrösse, Bedienung
ausschliesslich über die Tastatur, Durchgang mit einem Screenreader.

---

## 11. Phase 8 — Die Funktionen, die nicht scharf ausgeführt werden

**Zehn Funktionen werden bewusst nicht regulär ausgelöst.** Für jede gibt es eine
Ersatzprüfung, die nachweist, dass der Weg funktioniert, ohne den Schaden zu riskieren.

| Funktion | Warum nicht | Ersatzprüfung |
|---|---|---|
| **WLAN-SSID und -Schlüssel setzen** (`network_client_set`) | Schreibt die Zugangsdaten und löst eine Neuanmeldung aus — **an genau der Verbindung, über die geprüft wird**. Auch das Zurückschreiben unveränderter Werte trennt die Verbindung; ein Vertipper trennt sie dauerhaft. Es gibt keinen Rückweg, der das rechtfertigt | Dass die Funktion arbeitet, belegt der laufende Betrieb. Geprüft wird nur lesend: Liefert `network_scan` Treffer, erscheint die aktive SSID darin, stimmt die Anzeige des Verbindungszustands |
| **WPS** (`network_wps`) | Kann die gespeicherten Zugangsdaten ersetzen, ohne dass die PWA das Ergebnis kontrolliert. Gleiche Folge wie oben | Schaltfläche vorhanden und aktiv, Endpunkt erreichbar — nicht auslösen |
| **„Als Zugangspunkt starten"** (`boot_as_ap`) | Beim nächsten Neustart wäre die Uhr nicht mehr im WLAN, sondern spannt einen eigenen Zugangspunkt auf | Das Flag wird **gelesen** und im Backup geprüft (B7), nie gesetzt |
| `test_display` | Zieht bei voller Last so viel Strom, dass die Versorgung einbricht — am Gerät beobachtet: Brownout nach 44 s. **Kein Watchdog-Problem mehr**, der Reload-Fix ist drin und belegt wirksam; es ist die Stromreserve | Nur mit externer 5-V-Einspeisung und unter Beobachtung. Ohne diese Vorbereitung: nicht auslösen |
| `learn_ir` | Blockiert unbegrenzt, bis ein IR-Code kommt. Angelernte Codes sind **in keiner Sicherung** enthalten | Erreichbarkeit des Endpunkts prüfen, Dialog öffnen und abbrechen |
| `maintenance_reset_eeprom` | Setzt alle Geräteeinstellungen zurück. Wiederherstellbar über B5 — aber IR-Codes nicht | Nur ganz am Ende, nach bestätigt funktionierendem Restore, und nur wenn der Nutzer das IR-Anlernen in Kauf nimmt |
| `maintenance_format_fs` | Löscht die PWA vom Gerät. Danach ist die Oberfläche weg, mit der man sie hochladen würde | **Gar nicht.** Geprüft wird nur, dass der Endpunkt eingebettete Aufrufe mit 403 abweist — das tut der Smoketest bereits |
| `eeprom_settings_set` | Schreibt SSID, WLAN-Schlüssel, AP-Zugangsdaten und das Flag „als Zugangspunkt starten" in einem einzigen Aufruf. Ein leeres Feld heisst seit ESP 3.2.5 zwar „nicht ändern", aber ein gesetztes `boot_as_ap=on` genügt, um die Uhr beim nächsten Start aus dem WLAN zu nehmen — derselbe Weg, der das Gerät schon einmal ohne WLAN, ohne AP und ohne Webserver zurückgelassen hätte | Der Endpunkt wird im Durchlauf **ausschliesslich** über den Backup-Import berührt, in B5, B7 und B12, und nur mit der eigenen, unmittelbar vorher erzeugten Sicherung. Ein direkter Aufruf aus Phase 3 heraus findet nicht statt |
| **Tetris** (UDP-Kommando) | **Null** `watchdog_reload()` im ganzen Modul, und der Hauptloop ist währenddessen blockiert. Ein Tetromino fällt über rund 11 Zeilen in etwa 5,5 s; nach rund **3,6 Steinen** ist das 20-s-Fenster um, und seit 3.2.8 läuft der Watchdog wirklich. Der Reset ist damit nicht wahrscheinlich, sondern sicher (L106) | **Nicht starten.** Geprüft wird nur, dass das Modul vorhanden ist. Wer es trotzdem anfassen will, braucht ein vorbereitetes `GTq` **innerhalb von 20 Sekunden** nach dem Start — ohne diese Vorbereitung bleibt es beim Reset |
| **Snake** (UDP-Kommando) | Gleichartig gebaut wie Tetris, ebenfalls ohne eine einzige Watchdog-Bedienung (L106) | wie Tetris |

**Zusätzlich gilt:** `GET /?a` (Parameter ohne `=`) hat den ESP früher zum Absturz
gebracht. Der Smoketest sendet das bewusst und prüft, dass die Uhr weiterlebt. Von Hand
ist es **nicht** zu wiederholen.

**Zwei dieser zehn fängt kein Hook ab.** `tools/hooks/no-danger.py` weist die
gefährlichen **HTTP**-Endpunkte ab, bevor ein Agent sie erreicht. Tetris und Snake
laufen über **UDP** — der Filter sieht sie nicht. Für sie gibt es nur diese Zeile im
Plan und die Disziplin, sie zu lesen.

---

## 12. Phase 9 — Wiederherstellung und Abschlussvergleich

**Das ist der Schritt, der aus dem Durchlauf einen Nachweis macht.**

1. PWA-Backup aus Phase 0 importieren (entspricht B5)
2. Neuen Rohabzug erzeugen
3. **Feldweise gegen die Referenzdatei aus Phase 1 vergleichen**
4. Jede Abweichung ist entweder ein erklärter Rest (z. B. die Betriebszeit) oder ein
   **Befund**
5. **Altlastendurchsicht nach 12.2** — Vergleich gegen einen **alten** Abzug, nicht
   gegen die eigene Referenz. Findet, was frühere Läufe hinterlassen haben; Schritt 3
   kann das grundsätzlich nicht
6. `./tools/smoke-device.sh` — muss wieder **0 Fehler** melden
7. `./tools/install-app.sh --check` — Dateien vollständig
8. Mitschnitt auf dem Pi durchsehen: Watchdog-Resets, Lücken über 90 s, zerrissene
   Zeilen während des Durchlaufs

**Ohne Schritt 3 ist der Durchlauf nicht abgeschlossen.** „Sieht wieder normal aus" ist
keine Wiederherstellung.

### 12.1 Was der Abschlussvergleich leistet — und was nicht

Dieser Abschnitt steht hier, weil der Vergleich aus Schritt 3 der einzige Nachweis
dieses Plans ist und gleichzeitig eine Lücke hat, die man ihm nicht ansieht.

**Was er leistet:** Er findet jede Abweichung, die **dieser Lauf** erzeugt hat — auch
die, von der der Bericht des prüfenden Agenten nichts weiss. Genau dafür ist er da
(L81, Kapitel 2).

**Was er nicht leistet:** Er vergleicht gegen den Referenzabzug **desselben** Laufs.
Was ein **früherer** Lauf hinterlassen hat, stand beim Anlegen der Referenz bereits
drin — es ist Teil des Sollzustands geworden und damit unsichtbar. Der Vergleich
meldet es nie, egal wie oft er läuft.

**Das ist gemessen, nicht befürchtet** (`BEFUNDE.md`, L81 und L244): Ein Timer, der
die Uhr jede Nacht eine Stunde zu früh ausschaltete, entstand am 03.10.2026 in einem
Testlauf und stand danach in **allen 15 nachfolgenden Abzügen**. **Kein einziger
Abschlussvergleich hat ihn gemeldet.** Dazu fünf weitere Altlasten derselben Art —
ein aktiver DFPlayer-Alarm, zusammengeschnurrte LDR-Grenzen, zwei verbogene
Dimmkurven und verstellte Temperaturkorrekturen. Darunter **die Dimmkurve des
Nutzers**, überschrieben von einer abgebrochenen Vorgabe-Schleife; sie existierte
zuletzt nur noch in Abzügen von zwei Tagen davor.

**Und er vergleicht ESP-Kopien.** Was der STM anders hält als der ESP, sieht er auch
im eigenen Lauf nicht. Am 10.10.2026 meldete er 503 von 503 Feldern gleich, während der
STM nach S25/S26 andere LDR-Grenzen hielt als der Rohabzug (`BEFUNDE.md`, L364, L370).
Deshalb sind S25/S26 als nicht rücknehmbar eingestuft (6.3).

**Daraus folgt die Lesart jedes Berichts:** Ein Durchlauf, der „keine
Konfigurationsabweichung" meldet, sagt damit **nichts über Altlasten**. Er sagt nur,
dass er selbst sauber zurückgeräumt hat. Beides zu verwechseln erzeugt Vertrauen, das
nicht gedeckt ist — derselbe Mechanismus wie beim Bericht, der sauber meldet, während
das Gerät zwischendurch neu gestartet ist (DIR-013).

### 12.2 Altlastendurchsicht — Pflichtschritt, einmal je Durchlauf

Die Lücke aus 12.1 schliesst nur ein Vergleich, der **über den eigenen Lauf
hinausreicht**. Das ist Schritt 5 der Liste oben:

1. **Gegen einen alten Abzug vergleichen, nicht gegen die eigene Referenz.** Ältester
   verfügbarer Abzug, der noch als gesund gilt — `./tools/diff-snapshot.sh <alt> <neu>`.
2. **Jede Abweichung einer Ursache zuordnen**, über die Abzugsreihe datiert: Zwischen
   welchen beiden Abzügen ist der Wert gekippt? Daraus ergibt sich, welcher Lauf oder
   welches Ereignis ihn hinterlassen hat.
3. **Drei Ausgänge, und nur drei.** Gewollte Änderung des Nutzers ⇒ sie wird zur neuen
   Referenz. Altlast eines früheren Laufs ⇒ Befund in `BEFUNDE.md`, Rückweg benennen.
   Nicht entscheidbar ⇒ **unverändert lassen und als offen melden**, nicht raten.
4. **Nichts davon wird im selben Atemzug bereinigt.** Das Zurücksetzen trifft die Uhr
   des Nutzers im Wohnraum und braucht seine ausdrückliche Freigabe (R5) — ein
   Timer, ein Alarm oder eine Dimmkurve kann **gewollt** sein. Am 04.10.2026 wurden
   die sechs gefundenen Altlasten bewusst zunächst stehen gelassen, damit sie nicht
   dem laufenden Test zugerechnet wurden, und erst danach mit Freigabe bereinigt.

**Diese Durchsicht fällt nicht aus, wenn der Abschlussvergleich unauffällig ist.**
Sie prüft etwas anderes. Gerade ein sauberer Abschlussvergleich ist der Fall, in dem
sie gebraucht wird.

**Drei Wertgruppen sind dabei besonders zu prüfen**, weil sie in früheren Läufen
nachweislich hängen geblieben sind und im Alltag lange unbemerkt bleiben:

| Gruppe | Felder | Warum sie unbemerkt bleibt |
|---|---|---|
| Zeitschaltungen | `nighttime[idx=0..7]`, `ambinighttime[idx=0..7]`, `alarmtime[idx=0..7]` | wirkt nachts; der Nutzer sieht die Uhr dann nicht |
| Kurven und Grenzen | `num8array[var=0]` und `[var=1]` (sechzehn Stufen), LDR-Grenzen | wirkt erst, wenn die Automatik eingeschaltet wird |
| Korrekturwerte | Temperaturkorrektur RTC und DS18xx | verschiebt eine Anzeige, die niemand gegenmisst |

---

## 13. Was der Durchlauf voraussichtlich findet

Ein Testplan, der nur Bekanntes bestätigt, ist überflüssig; einer, der so tut, als sei
alles offen, ist unehrlich. Diese Befunde sind **vorhergesagt** — treten sie auf, ist
das kein neuer Erkenntnisgewinn, sondern eine Bestätigung des Katalogs.

**Eine Vorhersage, die sich erledigt hat, wird nicht gestrichen, sondern datiert.**
Ein Plan, der Behobenes weiter als „erwartet" führt, lässt den nächsten Durchlauf
daran vorbeisehen: Wer die Zeile liest, hakt sie ab, statt hinzusehen. Gestrichen
wäre sie aber ebenso verloren — ein Rückfall fiele dann niemandem mehr auf. Die
Spalte **Stand** sagt deshalb, **wann und woran** zuletzt gemessen wurde. Ohne
Datum und Messbedingung ist eine erledigte Vorhersage keine Information, sondern
eine Behauptung.

| Erwartet | Katalog | Stand |
|---|---|---|
| Werte ausserhalb des Bereichs werden still zurechtgebogen statt abgewiesen | Massnahme 17 | **Nicht mehr beobachtet** (04.10.2026, ESP 3.2.21). Acht Stellen am Gerät gemessen, alle weisen ab: `display_brightness_set=16` ⇒ `error=2 (0..15)`, `display_mode_set` oberhalb der Modusliste ⇒ `error=2 (0..0)`, `ticker_deceleration_set=256` ⇒ `error=2 (0..255)`, `ambilight_leds_set=1000` ⇒ `error=2 (0..999)`, Overlay `interval=0`, `duration=3`, `duration=10`, `days=0` ⇒ alle vier `error=2`. Keine einzige Klemmung |
| Warnung über ungespeicherte Änderungen erscheint auch ohne solche | Massnahme 4 | **Beobachtet in engerer Form** (06.10.2026, PWA 1.4.91, L324): Nach Eintippen und vollständigem Zurücksetzen auf den Ausgangswert erschien beim Modulwechsel trotzdem der Dialog, zweimal. `handleDirtyFormInteraction()` vergleicht nicht mit dem Ausgangswert. Ohne jede Eingabe dagegen **nicht beobachtet**, auch auf dem Pfad, an dem der Befund hing: am 05.10.2026 (PWA 1.4.90, L302) nach normalem Speichern und anschliessendem Modulwechsel kein Dialog. Davor am 04.10.2026 (PWA 1.4.88): vier Modulwechsel ohne Bearbeitung ⇒ **null** Dialoge; mit offener Änderung erscheint er korrekt |
| Polling läuft im Hintergrundtab weiter | R2-11 | **Nicht beobachtet** (04.10.2026, PWA 1.4.88). Bei verborgenem Tab und offenem Modul `system` kam über 60 s **kein einziger** Request an; nach dem Sichtbarwerden 7 Requests in 12 s. Gemessen **ohne** laufendes Update — ob ein laufender Fortschrittsabruf die Pause aushebelt, ist offen |
| Kein Hinweis bei unsicherem Kontext | R2-10 | **Fraglich, am Gerät nicht gegengeprüft.** `reportInsecureContextOnce()` in `app.js` meldet den Fall heute einmal je Sitzung. Prüfen statt voraussetzen: erscheint der Hinweis beim ersten Laden über `http://`, und bleibt er beim Neuladen aus? |
| Leere Datei beim App-Upload wird PWA-seitig nicht abgefangen | Massnahme 7 | **Fraglich, am Gerät nicht gegengeprüft.** Seit B7 prüft die PWA selbst: `sw.js` verwirft leere Antworten über `responseHasContent`, und `precacheAssets` bricht bei leeren Assets ab (`BEFUNDE.md`, Massnahme 7). Hier stand bis zum 05.10.2026 „Gilt weiter“. Am Gerät nachzuweisen bleibt es über S107 |
| Zeitserver wird bei über 16 Zeichen still gekürzt | **neu** — UI erlaubt 32, ESP speichert 16 | **Nicht mehr beobachtet** (05.10.2026, ESP 3.2.24, L302). 17 Zeichen ⇒ `error=2`, `value too long (max. 16 bytes)`, Wert unverändert (S10); ebenso der Update-Host mit 64 Byte (S99). **Aber:** Der Overlay-Text kürzt weiter still (L319) |
| Verstecktes WLAN lässt sich eintragen, erscheint aber nie in der Trefferliste | L24 — das freie Feld `network-ssid-manual-input` hat Vorrang vor der Auswahl. *Diese Zeile las sich bis zum 03.10.2026 als „nicht konfigurierbar"; seit L24 stimmt das nicht mehr* | ungeprüft |
| Fehlgeschlagener Flash ist nur einen Frame lang sichtbar | R2-5 | **Fraglich, am Gerät nicht gegengeprüft.** `failStm32Update()` ruft heute `finishProgressUi(2200)`; ein `finishProgressUi(0)` kommt in `app.js` nicht mehr vor. Nachzuweisen bleibt es an einem tatsächlich fehlgeschlagenen Flash — und der ist Klasse R |

**Alles andere wäre neu** — und gehört als `L`-Befund in `BEFUNDE.md`.

**Drei Lesarten, und sie sind nicht dasselbe.** „Nicht mehr beobachtet" heisst: am
Gerät gemessen, Vorhersage trifft nicht mehr zu. „Fraglich" heisst: Im Quelltext
liegt eine Korrektur vor, am Gerät ist sie nicht nachgewiesen — der Durchlauf hat
sie zu **messen**, nicht vorauszusetzen. „Gilt weiter" heisst: Der Durchlauf
bestätigt, er entdeckt nicht.

**Und was hier als erledigt steht, ist damit nicht im Katalog erledigt.** Dieser
Plan führt Messungen, `BEFUNDE.md` führt den Stand der Massnahmen. Eine Zeile hier
auf „nicht mehr beobachtet" zu setzen, schliesst dort keinen Befund — das ist der
Unterschied aus DIR-003, und er gilt in beide Richtungen.

---

## 14. Was dafür noch gebaut werden muss

Der Plan ist von Hand ausführbar, aber drei Werkzeuge machen ihn wiederholbar und
nehmen den grössten Teil der Fehlerquellen heraus:

| | Werkzeug | Stand | Was es tut |
|---|---|---|---|
| **W1** | `tools/snapshot-device.sh` | **da** | M2 und M3 erzeugen, Schlüssel hashen oder verschlüsseln, verschlüsselten Abzug wieder öffnen |
| **W2** | `tools/diff-snapshot.sh` | **da** | Zwei Abzüge **feldweise** vergleichen. Ein `diff` über `settings_xml` meldet sonst eine einzige lange Zeile und sagt nicht, *welche* Variable sich geändert hat |
| **W3** | `tools/testplan-run.mjs` | offen | Die Klasse-S-Prüfungen aus Phase 3 automatisch fahren: setzen, gegenprüfen, zurücksetzen, protokollieren |

W2 nimmt `stm32_log` vom Vergleich aus — ein Ringpuffer, der sich im Sekundentakt
ändert, würde jeden Bericht zumüllen. Gemessen: Über 25 Sekunden Stillstand ändert
sich sonst **kein einziges** Feld, der Vergleich ist also aussagekräftig.

---

## 15. Protokoll

Je Prüfung eine Zeile:

```
Kennung | Modul | Was | Klasse | Soll | Ist | Ergebnis | Beleg
```

`Beleg` ist der Rohwert oder die Logzeile, nicht „sah gut aus". Ein Durchlauf ohne
Belege ist ein Gefühl, kein Test.

**Abbruchkriterien** — bei einem davon wird der Durchlauf sofort beendet und
wiederhergestellt:

- Die Uhr ist über die PWA **und** über Legacy nicht mehr erreichbar
- Ein Watchdog-Reset tritt auf (seit dem 3.2.8-Fix wäre das ein Rückfall)
- Die Anzeige friert ein (Leitsymptom aus L14)
- Eine Sicherung erweist sich mitten im Durchlauf als unbrauchbar
