# Teststrategie PWA — vollständiger Funktionsdurchlauf

**Lebendes Dokument** (DIR-006). Es enthält bewusst **keine Versionsnummern** — den
gültigen Stand zeigt `./tools/guardrails.sh` in Stufe S4.

Dieser Plan beschreibt, wie die PWA **vollständig** geprüft wird: jede Anzeige, jede
Einstellung, jede Schaltfläche, die Grenzfälle jedes Eingabefelds, und Backup und
Restore als eigener Prüfgegenstand. Er ist so geschrieben, dass er ohne Vorwissen
ausführbar ist und dass das Ergebnis **nachprüfbar** ist statt behauptet.

---

## 0. Die Randbedingung, die alles bestimmt

**Es gibt genau eine Uhr — eine Test-Uhr im Keller mit angehängtem Logger** (R5, DIR-008).
Hier stand bis zum 14.10.2026, sie laufe produktiv im Wohnraum; der Nutzer hat das
richtiggestellt. **Wo dieser Plan unten noch eine „Freigabe des Nutzers im selben Gespräch“
verlangt, entfällt sie.** Es bleibt aber das einzige Exemplar, ohne Rollback per Knopfdruck,
und der nächste Lauf braucht einen bekannten Ausgangsstand. Daraus folgt für jeden Schritt
dieses Plans:

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
5. **Während eines Laufs gehört die Uhr dem Lauf.** Ein zweiter Browser lädt Assets,
   fragt ab, schreibt beim Laden unter Umständen selbst (L462) und senkt den Heap — am
   14.10.2026 hielt T2 deshalb an der Haltegrenze K30 (`BEFUNDE.md`, L461). Der Nutzer
   weiss das nicht von selbst: **Der Lead kündigt ihm Beginn und Ende eines Laufs an**
   und bittet, die Uhr bis dahin weder in der PWA noch in der Legacy-Oberfläche zu
   öffnen, auch nicht auf dem Telefon.

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
| **M3** | Vergleichsabzug | `./tools/snapshot-device.sh` | **nein** — seit dem Release EP nur, **ob** ein Schlüssel gesetzt ist (`key_set`, `ap_key_set`); davor eine Prüfsumme | **Nachweisen**, dass der Zustand stimmt |

**M3 kann nichts wiederherstellen, und das ist Absicht.** M3 darf deshalb liegen
bleiben, in Berichte wandern und von mir gelesen werden, ohne dass dabei ein Passwort in
Umlauf gerät. Wer nur M3 hat, hat kein Backup — er hat einen Messpunkt.

**Seit dem Release EP (C6b) sieht M3 eine Änderung des Schlüssels nicht mehr**, nur ein
Setzen oder Leeren. `/api/eeprom_settings` liefert `key` und `ap_key` seither immer leer,
dazu `key_set` und `ap_key_set` (`true`/`false`). Die Schlüssel selbst liefert nur noch
`/api/network_secrets`, und den fragt M3 bewusst nicht ab. Auf einem ESP ohne C6b steht
in M3 weiterhin die Prüfsumme; ein Vergleich über den Wechsel hinweg meldet deshalb an
`key`/`ap_key` eine erklärte Abweichung, keinen Befund.

**M2 schliesst die Lücke.** Gleicher Inhalt wie M3, aber mit Klartext-Schlüssel und als
Ganzes verschlüsselt. Seit EP holt `--restore` die Schlüssel über `network_secrets` und
**bricht ab**, wenn `eeprom_settings` einen Schlüssel als gesetzt meldet, den
`network_secrets` nicht liefert — ein M2 ohne Schlüssel sähe vollständig aus und liesse
beim Einspielen das WLAN weg. Das Klartextverzeichnis wird unmittelbar nach dem
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
> wäre der Import nutzlos. Seit dem Release EP holt der Export die Schlüssel eigens über
> `/api/network_secrets`. Scheitert das — ESP ohne den Endpunkt (404), Netzfehler, leere
> Antwort trotz `key_set` —, bricht der Export mit der Meldung „Die WLAN-Schlüssel liessen
> sich nicht von der Uhr lesen. …“ ab, und es entsteht **keine** Datei. Auf einem ESP ohne
> C6b ist das das erwartete Verhalten, kein Befund. **Zwischen ESP-OTA und PWA-Upload
> kein Export:** Die alte PWA auf dem neuen ESP läse die Schlüssel aus `eeprom_settings`,
> bekäme sie leer und exportierte still eine Sicherung ohne Schlüssel. Aber die exportierte Datei gehört behandelt wie ein
> Passwortzettel: nicht in Cloud-Ordner, nicht ins Repo, nicht in einen Chat.
>
> **Die M1-Datei eines Testdurchlaufs wird nach dem Lauf gelöscht** (Phase 9,
> Schritt 10). Sie ist ein Werkzeug des Laufs, keine Sicherung des Nutzers — die
> behält er selbst (siehe unten, „Was regelmässig zu tun ist“). Im Lauf vom 13.10.2026
> so gehandhabt (`BEFUNDE.md`, L412). Im Bericht steht, dass und wann sie gelöscht ist.

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
| **P1b-9** | **Start in jedem Modul** | die PWA stellt das zuletzt genutzte Modul aus `localStorage` wieder her. Das Skript belegt den Schlüssel der Reihe nach mit jedem Modul, lädt neu und zählt `ReferenceError`, `TypeError` und „before initialization“ aus der Startphase. Am Ende steht der Schlüssel wieder auf `main`. Findet es weniger als fünf Module, schlägt die Zeile fehl — sonst sähe die Prüfung ihren Gegenstand nicht (DIR-014) |

**Warum P1b-9 dazukam (L411, B54):** Mit frischem Profil startet jeder Lauf in `main`.
Ein Initialisierungsfehler, der nur beim Start in einem anderen Modul auftritt, war
damit für genau die Prüfung unsichtbar, die solche Fehler fangen soll (DIR-016). Am
13.10.2026 fand der Testagent so einen Fall von Hand: Beim Start in `system` warf
`app.js` einen `ReferenceError`, weil das Logbuch vor seiner Definition abgerufen wurde;
der nächste Takt heilte es nach 2,5 s. `check-pwa.sh` hatte mehrere Fassungen lang
sauber gemeldet. **Ein Fehler in P1b-9 ist ein Befund, auch wenn er sich selbst heilt** —
in der Zeile steht, in welchem Modul er auftrat.

**Bedient wird über das DevTools-Protokoll**, ohne Puppeteer oder Playwright — Node
bringt seit v22 ein eingebautes WebSocket mit, und mehr braucht es nicht. Die Module
werden tatsächlich angeklickt, nicht nur im DOM gezählt.

**Nicht zu verwechseln mit `tools/preview/`.** Das rendert die PWA in zwanzig
Viewports, aber gegen **Attrappen-Daten** aus `server.py` — gut für das Layout,
ungeeignet für die Frage, ob die Oberfläche mit den echten Werten der Uhr
zurechtkommt. Beides hat seinen Platz: `preview` für Phase 7, `check-pwa.sh` hier.

Rein lesend gegenüber dem Gerät: Die Module werden geöffnet, gespeichert wird nichts.
Was P1b-9 in `localStorage` schreibt, liegt im Profil des Prüfbrowsers, nicht auf der
Uhr. Weil das Skript dabei jedes Modul neu lädt, erzeugt es selbst Abrufe — **nicht
gleichzeitig mit K19** laufen lassen.

### Was `check-pwa.sh` seit dem Release E4 ausgibt

Seit dem Release E4 (C65, `BEFUNDE.md`, L457) wartet das Skript keine festen Zeiten mehr ab,
sondern auf **Netzruhe**: 1'500 ms lang kein Abruf an die Uhr unterwegs, höchstens 20 s lang.
Das gilt nach dem Laden und nach jedem Neuladen; nach dem Öffnen der Navigation wartet es,
bis das Element sichtbar ist, höchstens 3 s. Grund: Die PWA staffelt ihre Abrufe (unten),
und der ESP lässt bei knappem Heap Verbindungsaufbauten warten — feste Wartezeiten wären
einmal zu kurz und einmal zu lang. Chrome lädt die PWA erst, wenn das Skript mitliest; sonst
wäre die Startphase unsichtbar.

| Zeile | Lesart |
|---|---|
| `FEHLT Netzruhe … keine Netzruhe nach 20 s (<wo>) — offen: <Pfade>` | Nach 20 s war noch ein Abruf unterwegs; die Pfade nennen, wer das Netz belegte. **Befund**, der Lauf endet mit Exit ≠ 0. Er misst trotzdem weiter, damit der Rest der Ausgabe nicht verloren geht |
| `INFO Netzruhe nach dem Laden` | Sekunden vom Start bis zur ersten Netzruhe |
| `INFO /api/ gleichzeitig, Startphase` und `… danach` | höchstens gleichzeitig unterwegs gewesene `/api/`-Abrufe. Soll: in der Startphase 1, danach höchstens 2. **Nur Ausgabe, kein Fehlschlag** — das Soll prüft der Prüfstand c65q in der Vorschau. Ein höherer Wert am Gerät ist trotzdem ein Befund und gehört in den Bericht |
| `INFO Abrufe je Endpunkt` | Zahl der Abrufe von `network_scan`, `display_power`, `ambilight_power` und `power_status` über den ganzen Lauf. Soll für die drei letzten **0** — der Display- und Ambilight-Zustand kommt aus `settings_xml` (Ent-30). `network_scan` höchstens so oft, wie das Netzwerkmodul betreten wurde |
| `INFO als Takt nicht abgewartet` | Pfade, die im selben Warten zweimal endeten und deshalb als periodischer Takt gelten, etwa `stm32_log` alle 2,5 s — sonst käme nie Netzruhe zustande. **Bekannte Grenze:** Bricht ein solcher Pfad spät ab, sieht das Skript es nicht |
| `INFO Verbindungsaufbau >= 900 ms` | Zahl der neuen Verbindungen, deren Aufbau 900 ms oder länger dauerte, mit dem längsten Wert — die Spur eines SYN, das der ESP bei vollem Backlog verworfen hat. **Eine 0 ist kein Beleg** (Review E4, M2): Die Zählung ist am Gerät noch nie angeschlagen, die Vorschau kann kein verworfenes SYN nachstellen, und Verbindungen, die der Browser vorab geöffnet hat, sieht sie nicht. Ein Wert über 0 ist kein Fehler (unten), sondern eine Messung |
| `INFO App-Assets je Antwort` | Seit dem Werkzeugstand für R2 (C70): die Antworten der Uhr ausserhalb von `/api/` über den ganzen Lauf, aufgeteilt nach **Status auf der Leitung** — `200`, `304`, `Cache` (aus dem Browser-Cache oder vom Service Worker, ohne Leitungsstatus) und `andere`. Den Leitungsstatus nimmt das Skript wie die Ladeprobe aus `responseReceivedExtraInfo`, weil Chrome für eine `304` im gewöhnlichen Antwortereignis die `200` der Cache-Kopie meldet. **Nur Ausgabe, kein Soll** — das Soll prüft die Ladeprobe mit `--erwarte-304` (4c) |
| `INFO App-Assets uebertragen` | Byte, die Chrome für diese Antworten empfangen hat (`encodedDataLength`, Summe über den Lauf) |
| `INFO Verbindungsfehler` | Zahl **aller** fehlgeschlagenen Abrufe des Laufs, dazu die ersten drei mit Fehlertext und Pfad. Anders als P1b-7 zählt die Zeile auch `ERR_ABORTED` und das Favicon mit. Seit dem ESP-Release R2 endet ein Schreibabbruch mit RST: Ein `net::ERR_CONNECTION_RESET` auf einem App-Asset ist dann ein abgebrochenes Senden (`fs_write_short` in `device_ready` vorher und nachher vergleichen) oder C74. Die Zahl der `net::ERR_CONNECTION_RESET` und `net::ERR_EMPTY_RESPONSE` je Lauf ist die Grundlinie für AKE.3 (AKW.6, `specs/paket-2026-10-15/requirements.md`) |
| `INFO Laufzeit` | Dauer des ganzen Laufs |

Abrufe des alten Dokuments verwirft das Skript beim Neuladen, weil Chrome ihr Ende nie meldet;
die Startphase zählt erst ab der Navigation des neuen Dokuments (Review E4, N2).

**Grundlinie der App-Assets ohne Cache (16.10.2026, ESP 3.2.39):** 76 Antworten, alle `200` auf
der Leitung, keine `304`, keine aus dem Cache, 2'466'348 Byte, keine Verbindungsfehler
(`BEFUNDE.md`, L468). Jeder Modulstart in P1b-9 lädt alle Assets neu. Gegen diese Zahlen wird
ein Lauf nach dem ESP-Release R2 gestellt (G10); das Ergebnis gehört nach L468.

### Wie die PWA seit dem Release E4 abfragt — was kein Fehler ist

Seit dem Release E4 laufen alle `/api/`-Abrufe der PWA durch **eine Warteschlange**
(`fetchWithTimeout()` in `app.js`; `specs/paket-2026-10-13/design-e4.md` §19.6). Assets
und `/app/…` laufen nicht hindurch. Wer das Verhalten im Netzwerk-Tab der DevTools oder im
Mitschnitt verfolgt, sieht:

- **Höchstens 2 Abrufe gleichzeitig, in der Startphase 1.** Die Startphase endet, wenn das
  Fenster geladen **und** die erste Kernladung beendet ist, spätestens 15 s nach dem Start
  des Skripts.
- **Schreibende vor Lesenden**, innerhalb jeder Klasse in Ankunftsreihenfolge; ein
  Backup-Import bleibt in seiner Reihenfolge.
- **Lesende, die 15 s in der Schlange warten, werden verworfen**, ohne gesendet zu werden;
  die Anzeige behält ihren letzten Stand. Schreibende werden nie verworfen.
- Lesende Abrufe eines **überholten** Ladelaufs werden nicht gesendet.
- **Die Netzfrist zählt ab dem Senden**, nicht ab dem Einreihen. Ein lesender Abruf wird nach
  Fristablauf **nicht** wiederholt — die abgebrochene Anfrage läge beim ESP sonst noch im
  Backlog, und die Wiederholung wäre eine zweite Verbindung für dieselbe Antwort. Bei
  `Failed to fetch` und 5xx wird weiter wiederholt.
- **Verzögerte Verbindungsaufbauten sind kein Fehler.** Bei knappem Heap senkt der ESP seinen
  Backlog (`http_backlog`, 5b) und beantwortet überzählige SYN nicht; das Betriebssystem
  des Browsers wiederholt nach rund 1 s. Im Netzwerk-Tab steht der Abruf dann rund eine
  Sekunde im Verbindungsaufbau. Ein Befund ist es erst, wenn ein Abruf daran scheitert.
- **WLAN-Scan nur auf Anlass:** beim Wechsel ins Netzwerkmodul, beim Start dort und per Knopf
  „WLANs neu laden“ — nicht mehr im 15-s-Takt der Selbstaktualisierung (6.2).
- **Display- und Ambilight-Zustand** im Hauptmodul kommen aus `settings_xml` (`numvar` 3
  und 30), ohne eigenen Abruf. Fehlt ein Wert, zeigt die PWA „nicht verfügbar“, **nie
  „aus“**. Steht dort „aus“, während das Display leuchtet, ist das ein Befund — genau
  diesen Fehler hatte die PWA vor E4 bei einem abgelaufenen Abruf.
- **„Server erneut prüfen“** nach dem Speichern von Update-Host oder -Pfad (S97, S98): genau
  ein `update_status?refresh=1`. Grün („Server-Verfügbarkeit wurde neu geprüft“) nur, wenn er
  gesendet **und** beantwortet wurde, sonst die Fehlermeldung. Rund 9 s danach lädt die PWA
  `update_status` einmal nach — **erst dann** erscheint eine auf dem Server geänderte Release
  Note (5b, „Release Notes und STM-Liste aus dem Dateisystem“).

---

## 4c. Messwerkzeuge für Paketläufe — Ladeprobe, Browser-Sitzung, P1-Probe

Drei Werkzeuge aus dem Paket 2026-10-15 (`specs/paket-2026-10-15/design.md` §5.1, §5.3,
§5.6) messen, was Smoketest und `check-pwa.sh` nicht sehen: was ein **wiederholtes** Laden
der PWA noch über die Leitung schickt (C70), welche Verbindungen ein Browser auf der Uhr
stehen lässt (C72) und ob ein halb geschlossener Verbindungszustand auf dem ESP von selbst
abläuft. Sie gehören **nicht** zum Pflichtumfang der Phasen 0 bis 9, sondern zu den
Messschritten eines Pakets. Massgeblich für Aufruf und Ausgabe ist der Kopfkommentar des
jeweiligen Werkzeugs; hier steht, wie man die Ausgabe liest. Dazu kommt die Messung AKS.4
(weiter unten), ein Ablauf am Mitschnitt ohne eigenes Werkzeug.

**Für alle drei gilt Abschnitt 0, Punkt 5:** Während der Messung ist kein anderer Browser an
der Uhr offen, auch nicht auf dem Telefon, und es läuft weder `check-pwa.sh` noch K19 noch
eines der beiden anderen Werkzeuge. Jeder fremde Abruf landet in denselben Zählern und
Tiefstwerten und lässt sich nachträglich nicht herausrechnen. `./tools/watch-log.sh` läuft
mit (DIR-013); die Browser-Sitzung braucht die Heap-Zeile im Mitschnitt sogar für ihre
Auswertung.

### `tools/ladeprobe.sh` — kalt und warm laden (C70)

```
./tools/ladeprobe.sh                  1 kalt + 5 warm, deutsch, gegen DEVICE_HOST
./tools/ladeprobe.sh --englisch       nach dem kalten Laden Sprache Englisch (wie K33)
./tools/ladeprobe.sh --erwarte-304    ein warmes Laden mit 200 auf ein App-Asset ist ein Fehlschlag
```

Das Skript startet einen Headless-Chrome mit einem Profil, das **je Lauf frisch** ist und
**innerhalb des Laufs bestehen bleibt**: Das erste Laden ist kalt, die folgenden sind warm.
Die warmen Ladungen wechseln ab — die ungeraden laden neu (der Browser prüft nach), die
geraden öffnen die PWA über `about:blank` erneut, das „zweite Öffnen“ aus C70. Bei Ende und
Abbruch räumt das Skript Chrome und Profil ab; ein stehengebliebener Chrome hielte die PWA
offen und fragte die Uhr weiter ab.

| Umgebungsvariable | Wirkung |
|---|---|
| `LADEPROBE_WARM=n` | Zahl der warmen Ladungen, Vorgabe 5 |
| `LADEPROBE_HOST=host:port` | anderes Ziel, etwa Vorschau oder Prüfserver für die Gegenprobe (DIR-014) |
| `LADEPROBE_AUSNAHME=/app/pfad` | dieser Pfad ist von `--erwarte-304` ausgenommen |
| `LADEPROBE_MITSCHRIFT=datei` | jede Zeile zusätzlich in diese Datei |
| `CHROME=pfad` | anderer Chrome als der im Skript vorgegebene |

Je Laden drei Zeilen:

| Zeile | Lesart |
|---|---|
| `#n kalt` / `neuladen` / `oeffnen` `<ms> ms  /app/: <n> Anfr., 200= 304= cache= fehl= INM=  <n> Byte` | die `/app/`-Anfragen nach **Status über die Leitung**, die Zahl der Anfragen mit `If-None-Match` und die übertragenen Byte ohne Treffer im Browser-Cache. `(KEINE RUHE)`: nach 30 s war noch eine Anfrage offen. `ohne-Leitungsstatus=`: Anfragen, für die der Leitungsstatus fehlte |
| `Pfade: …` | jede Anfrage einzeln als `pfad=status`; `+INM`, wenn der Browser `If-None-Match` schickte, `cache` bei einem Treffer im Browser-Cache |
| `Geraet: …` | aus `/api/device_ready` nach dem Laden: der Tiefstwert seit dem Zurücksetzen samt I-4-Momentaufnahme (5b), dazu `free_heap`, `max_free_block` und `http_backlog`; liefert die Firmware `app_full` und `app_not_modified`, stehen sie dabei und in einer eigenen Zeile `Geraetezaehler` |

**Der Status ist der der Leitung, nicht der der Seite.** Bei einer Nachprüfung mit 304 meldet
Chrome im gewöhnlichen Antwortereignis (`responseReceived`) den Status der Cache-Kopie, also
200. Den Status über die Leitung liefert nur `responseReceivedExtraInfo`, und den nimmt das
Werkzeug. Ohne diese Unterscheidung sähe jeder Cache-Erfolg wie ein voller Abruf aus.

**INM gehört zur Lesart jeder 200.** Ohne die Angabe, ob der Browser `If-None-Match` schickte,
lässt sich eine 200 nicht zuordnen: Hat der Browser nicht gefragt, oder hat der ESP die Frage
übergangen? Ein Fehlschlag unter `--erwarte-304` nennt deshalb je Pfad „(mit INM)“ oder
„(ohne INM)“.

**Späte Anfragen werden erfasst, nicht verworfen.** Manche Assets fordert die PWA erst nach
einer `/api/`-Antwort an (`layout-previews.json` nach den Einstellungen). Kommt eine
`/app/`-Anfrage ausserhalb eines Messfensters, meldet das Werkzeug sie als `SPAET` und zählt
einen Fehlschlag.

Zwei Hinweise ohne Fehlschlag: `Tiefstwert in der eigenen Lesung (device_ready)` — dieser
Tiefstwert gehört nicht dem Laden. `kein /app/i18n/en.json in diesem Laden` unter
`--englisch` — Englisch ist in diesem Laden nicht nachgewiesen.

**Schreibt am Gerät nur den Rücksetzer der Tiefstwert-Messung** (`device_ready?heap_low_reset=1`
vor jedem Laden). Exit **0** ohne Fehlschlag, **1** mit Fehlschlag (späte oder fehlgeschlagene
Anfrage, keine Ruhe, `device_ready` nicht lesbar, `--erwarte-304` verletzt), **2**
Werkzeugfehler. Eine Haltegrenze hat die Ladeprobe nicht; sie protokolliert nur (Ent-7).

**Grundlinie ohne Cache:** `BEFUNDE.md`, L468, G9.1 — jedes Laden holt alles mit 200, INM=0.
So muss es bei einer Auslieferung ohne Validator aussehen. Ein Lauf mit Cache wird gegen diese
Grundlinie gestellt. Das Soll mit Cache ab dem ESP-Release R2 steht in AKW.2
(`specs/paket-2026-10-15/requirements.md`), wie die Antworten des ESP dann aussehen, in 5b unter
„Asset-Cache: `ETag`, `304` und die Zähler in `/api/device_ready`“.

### `tools/browser-sitzung.sh` — was ein Browser auf der Uhr stehen lässt (C72)

```
./tools/browser-sitzung.sh safari     Safari auf diesem Mac, über open und osascript bedient
./tools/browser-sitzung.sh chrome     Chrome auf diesem Mac
./tools/browser-sitzung.sh manuell    Gerät des Nutzers: das Skript nennt den Handgriff, der Nutzer bedient
```

Das Werkzeug stellt das Bild aus `BEFUNDE.md`, L461 kontrolliert nach — nach einer kurzen
Safari-Sitzung blieb der Heap minutenlang tiefer, bis Safari beendet war — und liest am Ende
jeder Phase:

| Phase | Was | Dauer (Vorgabe) |
|---|---|---|
| **S0** | Ruhe, kein Browser — Grundwerte | 180 s |
| **S1** | Browser öffnet `/app/`, Seite sichtbar | 120 s |
| **S2** | neuer Tab `about:blank` vorn, PWA verborgen | 180 s |
| **S3** | PWA-Tab geschlossen, Browser läuft weiter | 300 s |
| **S4** | Browser beendet | 180 s |

Zusammen rund 16 Minuten. `BROWSER_SITZUNG_DAUER="180 120 180 300 180"` ändert die Dauern
(genau fünf ganze Zahlen), `BROWSER_SITZUNG_HOST` das Ziel, `BROWSER_SITZUNG_MITSCHRIFT` den
Ort der Mitschrift (absoluter Pfad; Vorgabe ist der Temporärordner, der Pfad steht in der
zweiten Zeile der Ausgabe).

**Vorbedingungen.** Bei `safari` und `chrome` darf der Browser **nicht laufen** — geprüft am
Anfang und noch einmal unmittelbar vor dem Öffnen in S1. Das Skript beendet nie einen Browser,
den der Nutzer offen hat; läuft er schon, bricht es mit Exit 2 ab. Beim ersten Lauf fragt
macOS nach der Erlaubnis zur Automation. **Die gibt der Nutzer einmal**; ohne sie scheitern
alle Handgriffe über `osascript` (Tabs zählen, Tab öffnen, schliessen, Browser beenden) mit
`BEDIENUNG FEHLGESCHLAGEN`.

**Betriebsart `manuell`** ist für das Gerät des Nutzers, etwa Safari auf dem iPhone. Das Skript
nennt je Phase den Handgriff und wartet auf die Eingabetaste; ohne Konsole wartet es nicht,
und die Phasen laufen dem Handgriff davon. `netstat` und `lsof` lesen dabei nur diesen Mac —
die Verbindungen des Telefons erscheinen dort nicht, sondern nur in `tcp_now_list` der Uhr.

**Lesung am Phasenende, und nur dort:**

- `/api/device_ready`: `free_heap`, `max_free_block`, `http_backlog`, ausgewählte I-4-Felder
  (`heap_low_*`), `no_request_timeouts`, `no_request_aborts` und alle `tcp_now_*`-Felder (5b).
  Fehlen die `tcp_now_*`-Felder, meldet das Skript, dass die Entscheidungsregel nicht
  anwendbar ist.
- `netstat` und `lsof` auf dem Mac: jede Verbindung zur Uhr mit Zustand und Prozess — auch
  dann, wenn die Uhr nicht antwortet.

**Die eigene Lesung ist selbst eine Verbindung** und hinterlässt auf der Uhr ein TIME_WAIT von
rund 120 s. Deshalb liest das Skript zwischen den Phasen nicht, und es nennt je Lesung den
**lokalen Port** und bis wann ihr TIME_WAIT reicht; die nächste Lesung wiederholt das als
„TIME_WAIT der vorigen eigenen Lesung“. Ein TIME_WAIT mit diesem Port ist kein Rest des
Browsers. Die Minutenreihe zwischen den Lesungen liefert die Heap-Zeile im Mitschnitt
(`tcp=`, `nrt=`, 5b).

**Zusammenfassung je Phase** am Ende, jede Zeile gegen S0:

| Wert | Bedeutung |
|---|---|
| `L=` | Lochsumme `free − max`: freier Heap, der nicht im grössten Block liegt |
| `Luecke=` | `free_heap`(S0) − `free_heap`(Phase), die „Lücke“ aus §5.4 |
| `Δtimeouts`, `Δaborts`, `Δaccept_dead` | Zunahme von `no_request_timeouts`, `no_request_aborts` und `http_accept_dead` seit S0 |

Die Zahlen sind roh. Eigene Lesungen und ihr TIME_WAIT-Fenster zählen nach §5.4 nicht; ob
eines davon in eine Lesung fällt, zeigen Port und Uhrzeit der Lesung davor.

**Nicht rein lesend:** Beim Öffnen kann die PWA selbst schreiben (`ambilight_online`,
`BEFUNDE.md`, L462). Exit **0** Sitzung vollständig, **1** eine Phase gescheitert (Bedienung
oder Lesung, oder der Browser läuft nach S4 noch), **2** Vorbedingung nicht erfüllt oder
Abbruch. Bei Strg-C nennt das Skript die Phase und ob der Browser noch läuft — beenden musst
Du ihn dann selbst.

**Client freistellen — seit dem ESP-Release R1b entfallen, nur noch einmal zum Vergleich.**
Vor dem ESP-Release R1b band ein kalter `update_status` 1'704 Byte im httpclient, bis dieser
das nächste Mal verband (C71, `BEFUNDE.md`, L468). Die PWA fragt `update_status` beim Öffnen
ab; ohne Gegenmassnahme erschien die Belegung in S2/S3 als „Lücke“ und landete fälschlich in
M2 oder M5. Die Spec verlangte deshalb vor S0 jeder Sitzung, den Client freizustellen — ein
kalter `update_table_files`, der zu Ende liest, oder ein Durchgang des Abgleichs — und je
Phasenende `hc` und `update_info_cached` ins Protokoll (`specs/paket-2026-10-15/design.md`
§5.3, Nachtrag A.5). Das Werkzeug tut das nicht selbst. Seit dem ESP-Release R1b gibt
`httpclient_stop()` den Kontext samt ungelesener Antwort bei jedem Schliessen frei (L7); das
Freistellen **entfällt**. Es läuft nur noch **einmal**, in G9b.4, als Vergleichslauf gegen G9
(`specs/paket-2026-10-15/tasks.md`). Dass die Belegung tatsächlich ausbleibt, belegt erst die
Messung AKS.4 (unten) — bis sie vorliegt, ist L7 umgesetzt, nicht nachgewiesen.

**Ausgewertet wird nach der Entscheidungsregel M1–M5** in `specs/paket-2026-10-15/design.md`
§5.4; dort steht, welches Verbindungsbild zu welchem Eingriffskandidaten führt. Die Tabelle
steht hier bewusst nicht, damit es keine zweite Fassung gibt. Mit Nachtrag A.3 gilt für
**M2** (Strom von Vorab-Verbindungen): Er führt nur dann zum Eingriff K1, **wenn Δtimeouts den
Anstieg trägt**. Bleibt Δtimeouts bei rund 0, wäre K1 für L461 wirkungslos; das Skript
schreibt diesen Satz unter die Zusammenfassung. Widerlegt die P1-Probe P1, ist K2 zwingend.

### `tools/p1-probe.py` — läuft FIN_WAIT_2 auf dem ESP ab? (AKM.7)

```
./tools/p1-probe.py                          gegen DEVICE_HOST
P1_PROBE_HOST=<host> ./tools/p1-probe.py     anderes Ziel (Gegenprobe)
```

Die Probe prüft die Annahme P1: Eine Verbindung, die der ESP geschlossen hat, läuft in
FIN_WAIT_2 nach rund 20 s ab, **auch wenn die Gegenstelle ihre Seite nicht schliesst**. Eine
Verbindung schickt `GET /api/device_ready HTTP/1.0`, liest bis zum FIN des ESP und schliesst
nicht; der ESP steht dann in FIN_WAIT_2, der Mac in CLOSE_WAIT. Über eine zweite Verbindung
liest die Probe `device_ready` nach 10 s und nach 25 s und sucht in `tcp_now_list` den Eintrag
mit ihrem lokalen Port; dazu je die Mac-Seite aus `netstat`. Je Lesung gibt sie
`tcp_now_states`, `tcp_now_tw`, die ganze Liste und `tcp_now_more` aus, für den eigenen
Eintrag Zustand, `ruhe_s`, `nrtx` und `arg`.

| Exit | Bedeutung |
|---|---|
| **0** | P1 bestätigt: nach 10 s zu sehen, nach 25 s weg |
| **1** | P1 widerlegt: nach 25 s noch da — K2 wird zwingend (§5.4) |
| **2** | Werkzeugfehler oder Vorbedingung: Uhr nicht erreichbar, `DEVICE_HOST` fehlt, oder die Firmware liefert noch keine `tcp_now_list` (vor dem ESP-Release R1) |
| **3** | nicht beobachtbar: Der Eintrag war schon nach 10 s nicht zu sehen, oder er fehlte nach 25 s bei gekappter Liste (`tcp_now_more` > 0). **Die Frage ist dann offen, nicht beantwortet** |

Schreibt nichts am Gerät; die beiden Lesungen hinterlassen je ein TIME_WAIT. Ein Lauf dauert
rund 25 s. Ergebnis am Gerät: `BEFUNDE.md`, L468, G9.2 — P1 bestätigt.

### Messung AKS.4 — bleibt nach einem kalten `update_status` Heap liegen? (C71)

Kein eigenes Werkzeug, sondern ein Ablauf am Mitschnitt (`specs/paket-2026-10-15/design.md`
§5.5, `requirements.md` AKS.4; Schritt G9b.1). Er prüft die Wirkung von L7: Vor dem
ESP-Release R1b blieb nach einem kalten `update_status` die ungelesene Antwort der letzten
Existenzprüfung im httpclient liegen, 1'704 Byte in `free` und `max` (`BEFUNDE.md`, L468,
G9.6 Start 4 und M1).

1. ESP in Ruhe, kein Browser an der Uhr, `./tools/watch-log.sh` läuft (DIR-013). Warten, bis
   eine Heap-Zeile `tw=0` zeigt; ihr `free` und `max` sind der Wert **vorher**.
2. **Einen** kalten `update_status` senden: `curl -s "http://$DEVICE_HOST/api/update_status"`.
   Kalt heisst: Der letzte Abruf liegt mehr als 60 s zurück, und die Antwort trägt
   `"update_info_cached":false`. Steht dort `true`, war der Abruf warm und zählt nicht.
   **Ohne `refresh=1`** — der Parameter stösst zusätzlich einen Durchgang des Abgleichs an,
   und der kann ins Dateisystem schreiben.
3. Danach **keine HTTP-Lesung**, auch kein `device_ready`: Jede Lesung ist selbst eine
   Verbindung und hinterlässt ein TIME_WAIT. Gelesen wird nur der Mitschnitt.
4. Massgeblich ist die erste Heap-Zeile **mindestens 130 s** nach dem Abruf, die `tw=0` zeigt
   (vorher kann eine Verbindung in TIME_WAIT den Block scheinbar teilen, Spec H4). Ihr `free`
   und `max` sind der Wert **nachher**.
5. Dreimal, je mit eigenem Vorher-Wert.

| Ergebnis | Lesart |
|---|---|
| `free` und `max` in allen drei Durchgängen je höchstens 300 Byte unter dem Wert vorher | AKS.4 erfüllt: Der kalte `update_status` lässt nichts liegen |
| eine Lücke um 1'704 Byte | das Bild vor L7 — entweder wirkt L7 nicht, oder auf dem Gerät läuft noch der Stand vor dem ESP-Release R1b (gemeldete Version prüfen) |
| eine andere Lücke über 300 Byte | ein anderer Weg bindet Heap: Uhrzeit, Werte, `tcp=` und die Zeilen dazwischen ins Protokoll |

Vergleichswert auf dem Vorstand: −1'704 Byte (`BEFUNDE.md`, L468). Das Ergebnis gehört ins
Messprotokoll (L468) und erst danach, mit Beleg, an C71 und C75.

### Wann welches Werkzeug

| Lage | Werkzeug |
|---|---|
| jeder Durchlauf, jedes Einspielen | Smoketest, `install-app.sh --check` und `check-pwa.sh` (Phase 1 und 1b) — keines der drei hier |
| Grundlinie, bevor sich an der Auslieferung der Assets etwas ändert | Ladeprobe ohne Option, ein Lauf davon mit `--englisch` (Paket 2026-10-15, G9.1) |
| nach einer Änderung am Cache der Assets | Ladeprobe mit `--erwarte-304`, gegen die Grundlinie gestellt (G10.1) |
| was ein Browser an Verbindungen und Heap stehen lässt | Browser-Sitzung `safari` und `chrome`, dazu einmal `manuell` auf dem Gerät des Nutzers (G9b.4 — nach Ent-18 A an Stelle von G9.3 bis G9.5 —, G10.3, G11.1) |
| ob nach einem kalten `update_status` Heap liegen bleibt (Wirkung von L7) | Messung AKS.4, dreimal (G9b.1) |
| ob ein halb geschlossener Zustand von selbst abläuft | P1-Probe, einmal; das Ergebnis gehört zur Zuordnung M1–M5 (G9.2) |
| Regression nach einem Eingriff | Ladeprobe warm neben drei `check-pwa.sh` (G11.3) |

**Nie zwei davon gleichzeitig**, auch nicht mit `check-pwa.sh` oder K19 (Abschnitt 0,
Punkt 5). Zwischen zwei Browser-Sitzungen liegen mindestens 300 s Ruhe (G9.5) — sonst misst
S0 der nächsten Sitzung die Reste der vorigen. Jedes Ergebnis wird ausgewertet, bevor das
nächste Werkzeug startet (`specs/paket-2026-10-15/tasks.md`, Schritt G9).

---

## 5. Phase 2 — Alles Lesende

**Klasse L. Risikofrei, deshalb zuerst und vollständig.**

Für jedes Modul: Jeder angezeigte Wert wird gegen den Rohwert aus `settings_xml`
beziehungsweise `eeprom_settings` geprüft. Nicht „sieht plausibel aus", sondern
**Zahl gegen Zahl**.

Besonders zu beachten, weil hier schon einmal falsch formatiert wurde:

- **Temperatur**: Der STM-Fehlerwert `255` wurde früher als „127,5 °C" angezeigt.
  Zeigt der Sensor `127.5`, ist das **kein** Messwert, sondern „keine Messung".
  Seit A2 zeigt die PWA dafür „Messwert ungültig“; die drei Zustände der Anzeige
  prüft **K17** (7.1).
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
- **Dateiliste**: Grössen gegen die lokalen `.gz`. Seit dem ESP-Release E3 stehen dort
  zusätzlich `upd-releasenote.txt` und `upd-wc-list.txt` ohne lokales Gegenstück — der
  Zwischenspeicher für Release Notes und STM-Liste, kein Asset (5b, „Release Notes und
  STM-Liste aus dem Dateisystem“). Kein Befund.
- **`stm32_log`**: Wird das Logbuch überhaupt gefüllt, und bricht die Anzeige bei
  langen Zeilen um?
- **Schlüssel in `eeprom_settings`** (C6b, seit dem Release EP): `key` und `ap_key` sind
  leer, `key_set` steht auf `true` (bei gesetztem AP-Schlüssel auch `ap_key_set`), alle
  übrigen Felder wie zuvor. Ins Protokoll gehören nur die beiden Merker und die Länge der
  beiden Felder. **Befund:** ein nicht leeres `key` oder `ap_key` — dann steht der
  Schlüssel wieder im Klartext in der Antwort, und der Wert gehört gerade **nicht** in den
  Bericht. **`/api/network_secrets` wird im Durchlauf nicht abgerufen**, weder per curl
  noch im Browser von Hand: Die Antwort trägt die Schlüssel im Klartext und landete im
  Protokoll. `tools/hooks/no-danger.py` weist den Abruf ab. Den Endpunkt berühren nur
  der Backup-Export der PWA (M1) und `snapshot-device.sh --restore` (M2).
- **AP-SSID im Modul `network`**: in einer frischen Sitzung **direkt** öffnen, ohne
  vorher die Wartung zu besuchen. Der Fall steht mit Instrument und Soll als **K11**
  in 7.1; er ist rein lesend und gehört zeitlich hierher, in Phase 2.
- **Abrufzahl im Hauptmodul**: Die PWA holt `update_status` und `update_table_files`
  im Hauptmodul beim Öffnen und danach höchstens alle 5 Minuten, in Wartung und Anzeige
  bei jedem Lauf (C35). Gemessen über 15 Minuten im Mitschnitt, Soll höchstens 4 je
  Endpunkt. Der Fall steht als **K19** in 7.1. Er ist rein lesend und gehört hierher,
  aber er braucht ein **ruhiges** Fenster: kein zweiter Browser, kein `check-pwa.sh`,
  kein Modulwechsel — sonst zählt der Mitschnitt fremde Abrufe mit.

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
gelesen. Die Zeile ist über `/api/stm32_log` erreichbar; STM-Zeilen kommen dort
**ohne** Präfix an (der ESP entfernt `LOG ` beim Ablegen, am Gerät belegt am 14.10.2026:
`diag 683 …`), ESP-eigene Zeilen mit `- `. Beide Werte — vorher und nachher —
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
- **Die Vorlaufschritte aus K20 und K21 (7.1) gehören vor diesen Reset**, nicht vor
  einen eigenen. Beide prüfen, was ein STM-Neustart aus dem EEPROM zurückholt (A56,
  A81), und brauchen genau diesen einen Reset — am Tag, ausserhalb eines Aus-Fensters.
  Im Aus-Fenster schaltet der STM das Ambilight mit dem Display aus (A69), dann sagt
  K20 nichts; K20 als „nicht aussagekräftig“ führen, K21 bleibt gültig.
- **K30 und K31 (7.1) werten diesen Reset mit aus**, ohne eigenen: Der ESP startet mit
  neu, die ersten 2 Minuten zeigen, ob sein Empfangspuffer beim Start überläuft (A79), und
  die Bestätigung der ersten Netzzeit danach braucht zwei angenommene Antworten (C59).
- **Fällt der Reset in ein Aus-Fenster der Display-Timer**, gelten Soll und
  Rückstellung aus dem folgenden Absatz.

#### V3 im Nachtzustand

Der Testplan nannte den Nachtzustand bis zum 11.10.2026 nicht. In T.2 lief das
Display deshalb zweimal länger an als vorgesehen, bei S1 und nach dem Reset
(`BEFUNDE.md`, L390).

**Ob ein Aus-Fenster gilt, entscheidet die Timertabelle, nicht der Rohwert.**
Massgebend ist vor dem Reset das jüngste vergangene Ereignis der Displayliste
`nighttime[idx=0..7]`, das aktiv und an seinem Wochentag gültig ist — auch über
Mitternacht zurück: Ein Aus-Timer am Vortag um 23:00 gilt um 01:00 noch. Ist es ein Aus-Ereignis, liegt der Reset im
Aus-Fenster. `numvar[idx=3]` (`DISPLAY_POWER`) allein sagt das nicht: Ein von Hand
ausgeschaltetes Display ist kein Aus-Fenster und kommt nach dem Reset wieder an. Die
Ambilight-Liste `ambinighttime` zählt beim Start nicht; das Ambilight folgt dem Display.

| STM-Stand | Soll nach dem Reset im Aus-Fenster | Nachweis |
|---|---|---|
| ohne A69 | Display **an** bis zum nächsten Ein-Timer. Bekannt (L390), kein neuer Befund | im Mitschnitt `switching power on`, kein `Found Timer` |
| mit A69 (STM-Release 1 aus `specs/paket-2026-10-11/`) | Startsequenz wie bisher, **danach Display aus** | im Mitschnitt `night: start aus (hh:mm)`, `set_display_power: new=0 …`, in der ersten Minute `show_time: … p=0 …`; nach dem Vollabgleich `numvar[idx=3]` = 0 und die Ambilight-Power gleich dem STM. Am Display: aus |
| mit A70 (STM-Release S2 aus `specs/paket-2026-10-13/`) | wie mit A69, mit drei Änderungen: Das Aus wartet auf das Ende des IP-Lauftexts, aber **höchstens bis zu dessen Frist** (Laufzeit des Lauftexts plus 10 s) — kommt der Restore nicht, schaltet die Frist. Startet **während** dieses Wartens ein anderer Ticker (Wetter, `ticker_set`, Datum), gilt das Aus **sofort, vor dem Ticker**; der Ticker erscheint nicht. War die RTC ungültig und kommt das erste `TIME` während des IP-Lauftexts, geht das Display erst nach dem Lauftext aus | wie mit A69. Den Ticker-Fall prüft **K24** (7.1); dort steht auch, was sich am Gerät nicht herstellen lässt |

War die Uhrzeit des STM beim Start nicht gültig (RTC offline), wertet er das Fenster
erst beim ersten übernommenen `TIME` aus; das Display geht dann entsprechend später aus.
Auch das ist Soll.

**Die Rückstellung im Aus-Fenster ist „wieder aus“**, nicht „wie vor dem Durchlauf an“.
Mit A69 ist nichts zu tun, nur lesend nachzuprüfen. Ohne A69 bleibt das Display nach dem
Reset an; zurück über S1 mit `value=off`. Das ist ein eigener schreibender Aufruf — bei
der Anfrage nach der Freigabe für V3 mitnennen.

**Nach jedem Reset, auch ausserhalb des Aus-Fensters**, steht mit der
Bestätigungsregel aus A62 im Mitschnitt `time: sprung, bestaetigung angefordert` und
rund 15 s später `cmd: set time to …`. Das ist **Soll, kein Fehler**: Nach einem Start
hat der STM keine Referenz, die erste Netzzeit braucht eine zweite, passende Antwort.
Ein Befund ist erst `time: ungueltig, verworfen` oder ein `time: sprung`, dem kein
`cmd: set time to …` folgt. Auf einem STM ohne die Bestätigungsregel fehlt die Zeile;
auch das ist kein Befund.

#### Auswertung des Mitschnitts — die Zeilen aus den Paketen 2026-10-11, 2026-10-13 und 2026-10-15

Seit dem Paket 2026-10-11 melden STM und ESP Zustände, die vorher still blieben. Das
STM-Release S2 aus `specs/paket-2026-10-13/` brachte vier Zeilen dazu bzw. änderte ihre
Bewertung; sie stehen am Ende der Tabelle. Das ESP-Release E2 aus demselben Paket brachte
`- rpc keine Antwort` und `- rpc Antwort verstuemmelt` (B55), die Gründe `ungefragt` und
`originate` in `- ntp verworfen` (C59), die Folgeeinträge mit `~` (C8) und die neue
Erwartung an `ovr` beim ESP-Start (A79); diese Zeilen stehen ebenfalls am Ende, vor den
beiden `- abgewiesen: …`. Das ESP-Release E3 (C65) brachte `low=` in der Heap-Zeile, die
Felder `blk`, `snd` und `ms` in `- http write lost` und `- fs write short`, den Ring und die
Zähler für `- fs write short` sowie die Zeilen `- update liste zu gross` und
`- update cache …`; sie stehen danach, ebenfalls vor den beiden `- abgewiesen: …`. Das
ESP-Release E4 (C65) brachte `- scan …`; sie steht dahinter, ebenfalls vor den beiden. Das ESP-Release R1 aus `specs/paket-2026-10-15/` (C71, C72) brachte `tcp=` und `nrt=` in der Heap-Zeile sowie `- update cache vorher`, `heap`, `nach` und `tmp geloescht`, `- heap erste anfrage` und `- flash id`; sie stehen hinter `- scan`, ebenfalls vor den beiden. Das ESP-Release R2 (C70) brachte `- fs seek error`; sie steht zusammen mit der älteren `- fs read error` dahinter, ebenfalls vor den beiden. Die
Entscheidungsregel für die beiden Schreibzeilen steht unter der Tabelle. Die stehen ganz am Schluss: Die Abweisung über `Sec-Fetch-Dest`
gibt es seit ESP 3.2.4, die zu lange Query bringt das ESP-Release E2 (C46); E1 ist
zurückgestellt (L439). Die
Tabelle gilt für V3 und für den **ganzen** Durchlauf; am dichtesten treten die Zeilen
nach einem STM-Start auf. Die Logwache meldet einen Teil davon von selbst — **massgebend
ist diese Tabelle, nicht ob die Wache anschlägt.** Eine erwartete Zeile kommt mit ihrem
Anlass in den Bericht, eine Befundzeile mit Uhrzeit, Wortlaut und dem Fall, der gerade
lief.

Die STM-Zeilen stehen im Mitschnitt; `i2c: frist abgelaufen` steht **nur** dort, nicht in
`/api/stm32_log`. Die ESP-Zeilen mit vorangestelltem `- ` stehen im Mitschnitt **und**
in `/api/stm32_log` — **ausser** `- fs seek error` und `- fs read error`, die nur im
Mitschnitt stehen (`BEFUNDE.md`, L481).

| Zeile | Was sie bedeutet | Erwartet | Befund |
|---|---|---|---|
| `night: start aus (hh:mm)` | Der STM ist in einem Aus-Fenster der Display-Timer gestartet und schaltet nach der Startsequenz aus (A69). `hh:mm` ist die Minute des massgebenden Aus-Timers | Bei jedem STM-Start (V3, Flash) im Aus-Fenster, nach dem IP-Lauftext, sonst nach 60 s. War die RTC beim Start ungültig, erst nach dem ersten übernommenen `TIME` | Start im Aus-Fenster **ohne** die Zeile (das Display bleibt an); die Zeile bei einem Start ausserhalb des Fensters; `night: start ein` — beim Start wird nur ein Aus angewandt |
| `night: sprung aus (hh:mm)` / `night: sprung ein (hh:mm)` | Die Uhrzeit ist über mehr als eine Minute gesprungen, und im übersprungenen Intervall lag ein Display-Timer. Der STM holt ihn nach, wie der Timer selbst geschaltet hätte | Nach einer Zeitkorrektur, die über einen Timer hinwegspringt (`cmd: set time to` mit grossem Abstand zur vorigen Zeit), und bei der Zeitumstellung, wenn ein Timer in der übersprungenen Stunde liegt | **Ohne** vorangehende Zeitkorrektur: Dann ist die Minute ausgefallen, weil der Hauptloop länger als eine Minute stand — das Bild aus L204. `diag`-Folge und `l=` im selben Zeitraum prüfen |
| `time: sprung, bestaetigung angefordert` | Die gemeldete Netzzeit passt nicht zur Referenz des STM (über 10 Minuten daneben) oder es gibt noch keine Referenz. Der STM übernimmt sie erst, wenn eine zweite Antwort sie bestätigt (A62) | Nach **jedem** STM-Start, gefolgt nach rund 15 s von `cmd: set time to …` | Kein `cmd: set time to …` danach; im laufenden Betrieb **ohne** vorangehenden STM-Start — dann lag die Zeit um mehr als 10 Minuten daneben, Gerätezeit davor und danach notieren |
| `time: ungueltig, verworfen` | Die Netzzeit war formal ungültig oder liegt vor dem 01.01.2026; der STM verwirft sie | nie | jede |
| `cmd: set time to …` | Der STM hat eine Netzzeit übernommen. Die Zeile kommt seit A62 bei **jeder** Übernahme | Bei jedem Zeitabgleich und nach jeder bestätigten `time: sprung`-Zeile | Datum oder Uhrzeit darin falsch; fehlt nach `time: sprung` |
| `i2c: frist abgelaufen` | Ein I²C-Zugriff (EEPROM oder RTC) hat seine Frist überschritten; der STM taktet den Bus danach frei (A60) | **nicht mehr**, auch beim Start nicht: Seit A73 gibt die Adresssuche ohne Antwort keine Zeile (L402). Steht sie beim Start **vor** `rtc is online` und läuft danach alles, ist das die RTC-Suche — benennen, nicht als Fehlschlag werten | Jede andere. Folgt `error: cannot reset I2C bus`, ist der Bus tot: Dann scheitert jeder EEPROM-Zugriff, sofort melden |
| `spiel: N Zeilen verworfen` | Während eines Spiels hat der STM N Zeilen vom ESP verworfen und danach einen Vollabgleich vorgemerkt (A53) | Nur nach einem Spiel, also in K18; danach `info: configuration sent` | Ohne vorangegangenes Spiel; oder nach dem Spiel **kein** `info: configuration sent` |
| `ldr: minimum abgewiesen (x >= max y)` / `ldr: maximum abgewiesen (x <= min y)` | „Messwert übernehmen“ hat einen Rohwert abgewiesen, der die Grenzen vertauscht hätte (A57). Nichts gesetzt, nichts geschrieben | In S25b; in S25/S26 nur, wenn die Grenzen ungewöhnlich eng stehen | In S25/S26 bei den Grenzen 0/4095; jede, der ein `eep a=225` bzw. `eep a=227` folgt |
| `- var ignoriert #n code=… idx=…` | Der ESP hat eine Zeile des STM quittiert, aber nichts zugewiesen — Index oder Wert ausserhalb des Bereichs, unbekannter Buchstabe (A35). Gedrosselt: die erste sofort, danach höchstens eine je Minute; `#n` zählt alle. Ohne Wert, weil die Zeilen Zugangsdaten tragen können | **nie**, auch nicht im Vollabgleich (seit A75; im Lauf vom 13.10.2026 null, L412) | jede. `code` und `idx` in den Bericht, dazu der Stand von `#n` — der Zähler sagt, wie viele Zeilen die Drosselung verschluckt hat |
| `- ntp verworfen #n <grund>` | Der ESP hat eine Antwort auf seine Zeitanfrage verworfen und dem STM **kein** `TIME` geschickt. Gründe: `absender` und `port` (Paket nicht vom angefragten Server), `laenge` (falsche Paketgrösse), `li`, `stratum`, `transmit` (Server meldet keine gültige Zeit). Seit dem ESP-Release E2 zwei weitere (C59): `ungefragt` (keine Anfrage offen — keine gesendet, älter als 60 s, oder schon beantwortet) und `originate` (das Originate-Feld trägt nicht den Zufallswert der Anfrage). Eine `originate`-Antwort schliesst die Anfrage nicht; die echte Antwort kommt danach noch an. Gedrosselt wie `var ignoriert`; die Logwache meldet jede als Alarm | nie | jede, mit Grund — siehe **K31**. Bei `ungefragt` und `originate` zwei Lesarten: eine fremde Antwort im LAN (L406), oder ein Zeitserver, der das Originate-Feld nicht nach RFC 5905 zurückschreibt; dann fehlt **jede** stündliche Netzzeit. Bleibt danach auch der nächste Zeitabgleich aus (kein `cmd: set time to …`), läuft die Uhr auf ihrer RTC weiter — im Bericht nennen |
| `ovr=` in der Heap-Zeile | Zahl der Empfangsüberläufe der ESP-UART seit dem ESP-Start: Der ESP hat Bytes vom STM verloren. Die Heap-Zeile kommt einmal je Minute, nach jedem ESP-Neustart beginnt die Zahl bei 0. Seit dem ESP-Release E2 ist der Empfangspuffer 1024 Byte gross (A79); das kostet rund 768 Byte Heap, `free` liegt entsprechend tiefer — erwartet, kein Befund | `ovr=0` durchgehend (13.10.2026: so, L412), seit E2 **auch beim ESP-Start** (K30) | jede Erhöhung — sie gehört zu A21/A76, nach einem ESP-Start seit E2 zusätzlich zu A79. Mit der Zeile `- ovr #n t=…` zusammen auswerten. Anhalten nach **K30** (seit E3, Ent-23): Ruhewert `max` unter 5'120 oder `low=` unter 1'072 |
| `- ovr #n t=<ms>` | Der ESP hat einen Überlauf **erkannt**; `t` ist seine Betriebszeit in Millisekunden beim Erkennen, nicht beim Überlauf selbst. Ein kleines `t` heisst „während des Starts oder im ersten Durchlauf“. Gedrosselt, höchstens eine je Minute | nie — seit dem ESP-Release E2 ausdrücklich auch nicht beim ESP-Start (A79, K30); davor kam sie dort einmal vor (L421) | jede. **Sie ist zugleich der Beleg, auf den A76 wartet:** Uhrzeit, `n`, `t` und das, was gerade lief (ESP-Neustart, OTA, Vollabgleich, Setter-Reihe), vollständig in den Bericht. Ein kleines `t` nach einem ESP-Start heisst seit E2: Der grössere Puffer reicht nicht (A79) |
| `icon: verworfen, display aus` | Ein Icon kam vollständig an, während das Display aus war, und der STM hat es verworfen, statt es bis zum Einschalten vorzumerken (A80). **Hinweis**, kein Alarm; die Logwache meldet ihn als Hinweis | Im Aus-Fenster, wenn ein Icon- oder Wetter-Icon-Overlay fällig wird, und in **K22**. Unmittelbar davor steht `overlay icon <name>` bzw. `overlay weather icon` | Bei eingeschaltetem Display — dann ist ein Icon verloren gegangen, das hätte erscheinen sollen. Ein Icon, das nach dem Einschalten **doch noch** erscheint, ist ebenfalls ein Befund |
| `icon_freeze: ENTER` | Ein Icon wartet bei ausgeschaltetem Display auf das Einschalten (Diagnosezeile, L19, L422). Gegenstück `icon_freeze: LEAVE` | **seit A80 nie** | **jede** — ein Rückfall von A80. Vor dem STM-Release S2 war sie im Aus-Fenster erwartet; seither ist sie ein Befund, und die Logwache schlägt Alarm. Uhrzeit, die Felder der Zeile und das Overlay, das gerade lief, in den Bericht |
| `set_display_power: flags=0x… display=d ambi=a dcf77=f` | Zweite Zeile jedes Display-Schaltens: der Zustand danach. `dcf77` ist der Merker, ob der DCF77-Empfang läuft (A82) | Bei jedem Schalten des Displays (S1, Timer, Nachtfenster); **`f` = 1 genau dann, wenn `d` = 0 und `a` = 0** | jede Zeile, in der `f` davon abweicht. Ein Schalten **nur** des Ambilights gibt im Release-Build keine Zeile — der Merker ist dann am Gerät nicht sichtbar, siehe **K23** |
| `RPCR 0x r` | Rückmeldung des STM auf „Messwert übernehmen“ (B55): `x` = `0` Minimum, `1` Maximum; `r` = `0` übernommen, `1` abgewiesen, `2` Messung fehlgeschlagen. Geht über die **ESP-Brücke**, ohne Quittung, direkt nach der Wertmeldung. Im Mitschnitt steht sie **nur, wenn der ESP sie spiegelt** — ein ESP ohne die passende Auswertung verwirft sie still | Nach S25/S26 `r` = `0`, nach S25b `r` = `1`, jeweils unmittelbar nach der `eep`- bzw. `ldr:`-Zeile | `r` = `2` (die Messung ist fehlgeschlagen); `r` = `0` neben einer `ldr: … abgewiesen`-Zeile, oder `r` = `1` ohne sie. Fehlt die Zeile, ist das **kein** Befund, solange der ESP sie nicht spiegelt — dann tragen `eep` und `ldr:` den Nachweis. **Auch das ESP-Release E2 spiegelt sie nicht**, er wertet sie aus: Der Nachweis ist seither die API-Antwort von `ldr_min_set`/`ldr_max_set` (S25, S25b, S26) |
| `- rpc keine Antwort #n idx=xx` | „Messwert übernehmen“ (`ldr_min_set` bzw. `ldr_max_set`) hat binnen 2 s keine `RPCR`-Rückmeldung vom STM bekommen; die API hat Kennung 11 gemeldet (B55, ESP-Release E2). `xx` = `00` Minimum, `01` Maximum. Gedrosselt: die erste sofort, danach höchstens eine je Minute; `#n` zählt alle. Im Mitschnitt **und** in `/api/stm32_log`; die Logwache schlägt **Alarm** | nie | jede. Lesarten: ein STM ohne `RPCR` (vor dem STM-Release S2), der STM-Hauptloop stand länger als 2 s (Vollabgleich, EEPROM-Burst, das Bild aus L204), oder die Zeile ging auf der Brücke verloren. `diag`-Folge und `d=` im selben Zeitraum prüfen. Die nächste Rückmeldung desselben Index kann einem späteren Aufruf zugerechnet werden (L442) — einen solchen Aufruf im Bericht als fraglich führen |
| `- rpc Antwort verstuemmelt #n` | Der ESP hat eine Zeile mit `RPCR ` empfangen, die nicht genau die Form `RPCR xx r` hatte; er verwirft sie, der Wartende läuft in seine Frist (Kennung 11). Gedrosselt wie oben; die Logwache schlägt **Alarm** | nie | jede — die Brücke hat Zeichen verloren oder vertauscht. Mit `d=` der `diag`-Zeile und `ovr=` zusammen auswerten |
| Folgeeinträge mit `~` in `/api/stm32_log` | Eine STM-Zeile über 120 Zeichen steht seit dem ESP-Release E2 als **mehrere** Einträge im Ring (C8): der erste mit 119 Zeichen und `~` am Ende, jeder folgende beginnt mit `~`. Zusammengesetzt (die `~` an diesen Stellen weglassen) ergibt sich die Zeile, wie sie im Mitschnitt steht. Die PWA zeigt Folgeeinträge als eigene Zeilen. Vor E2 endete eine solche Zeile mit `~`, der Rest war weg (L99, L430) | Bei jeder Zeile über 120 Zeichen, gezielt in **K29** | Zusammengesetzt **ungleich** dem Mitschnitt; ein Rest fehlt. Eine Zeile über 255 Zeichen kappt schon die Brücke, ihr letzter Folgeeintrag endet dann mit `~` — das ist die benannte Grenze, kein Befund. Eine Logzeile, die selbst mit `~` beginnt, sähe aus wie ein Folgeeintrag; solche Zeilen gibt es heute nicht |
| `low=` in der Heap-Zeile | Seit dem ESP-Release E3 (Ent-22): der kleinste grösste freie Block **seit der vorigen Heap-Zeile**, gemessen an vier Stellen **während** des Betriebs — beim Senden einer API-Antwort (`flush`), beim Ausliefern einer Datei (`fs`, erster und jeder achte Block), beim Annehmen einer Verbindung (`accept`) und nach jedem ausgehenden Verbindungsaufbau (`client`). Die Heap-Zeile setzt dieses Minimum nach jeder Zeile selbst zurück, unabhängig von `heap_low_reset` in `device_ready`. `low=` steht hinten; `free=`, `max=` und `ovr=` behalten ihre Stellung. Lief seit der vorigen Zeile kein Messpunkt, steht `low=` gleich `max=`; grösser als `max=` ist es nie | In Ruhe gleich oder nahe `max=`; unter PWA-Last deutlich tiefer — die feine Abfrage aus L449 sah beim Laden der PWA 1'448 | **Anhalten** unter 1'072 (K30). Dazwischen **kein** Anhaltegrund, sondern ins Protokoll: Uhrzeit, Wert, Last (was lief), Messweise; den Pfad des Tiefs liefert `heap_low_path` aus `device_ready`. Ein `low=` unter 5 KB allein ist kein Befund — nach L449 läge jede PWA-Ladung darunter |
| `- http write lost <rest> gone=<n> fail=<n> blk=<n> snd=<n> ms=<n>` | Eine Antwort ging nicht vollständig hinaus; `rest` = nicht ausgelieferte Byte, `gone` und `fail` = Stand von `write_lost_gone` bzw. `write_lost_failed` nach diesem Verlust — welcher gestiegen ist, nennt den Grund (C25). Seit dem ESP-Release E3 hinten angefügt: `blk` = grösster freier Block und `snd` = freier Platz im Sendepuffer, beide unmittelbar nach dem letzten Versuch erhoben; `ms` = Dauer vom Beginn des ersten bis zum Ende des letzten Versuchs. Eine Zeile je Verbindung, Mitschnitt und Ring textgleich. Geht der **Kopf** einer Dateiauslieferung verloren, steht diese Zeile und nicht `fs write short`; der Rumpf wird dann nicht mehr versucht (Review E3, N1) | Mit gestiegenem `gone`: Der Browser hat abgebrochen (Tab zu, Abbruch, L270) — erwartet, nur zählen | Mit gestiegenem `fail`: jede — Lesart nach der Entscheidungsregel unter der Tabelle, dazu Uhrzeit, Endpunkt aus der `- request`-Zeile davor und alle Rohwerte. **`ms` über 3'000 ist ein Befund für sich**, auch bei `gone`: Dann stand `loop()` länger als die Frist des STM, und im Mitschnitt folgt `keine Quittung` (L449) |
| `- fs write short: <datei> n=<n> w=<n> blk=<n> snd=<n> ms=<n>` | Eine Dateiauslieferung (PWA-Asset) brach **mitten in der Datei** ab: `n` = Länge des Blocks, `w` = davon geschrieben, `blk`, `snd` und `ms` wie bei `http write lost`. Der Browser bekommt die Datei unvollständig und verwirft sie. Seit dem ESP-Release R2 bricht der ESP die Verbindung dabei hart ab (RST), und Chrome meldet `net::ERR_CONNECTION_RESET`; bis dahin schloss er regulär, und der Browser erkannte den Schaden erst an der Länge, die im Kopf angesagt war. der Service Worker behält seinen Stand, bei einer Erstinstallation fehlt das Asset bis zum Neuladen. Bis E3 stand die Zeile nur im Mitschnitt, ohne Ring und ohne Zähler; seither zählt sie in `fs_write_short`, `write_lost_blocks` und je nach Verbindung in `write_lost_gone` oder `write_lost_failed`, und `write_lost_bytes` wächst um den nicht ausgelieferten Rest der Datei. Die Logwache schlägt **Alarm**. Dieselbe Zeile **ohne** die Felder `n=` bis `ms=` kommt aus dem Laden einer Datei vom Update-Server ins Dateisystem und meint einen Schreibfehler im Flash — ebenfalls Alarm | nie | **jede**, mit Uhrzeit, Datei, Rohwerten und der Lesart nach der Entscheidungsregel unter der Tabelle. Welche Verbindungslage vorlag, steht nicht in der Zeile: `write_lost_gone` und `write_lost_failed` aus `device_ready` vor und nach dem Fall vergleichen. Danach meldet der Smoketest „FEHLT“, solange `fs_write_short` über 0 steht — bis zum nächsten ESP-Start |
| `- update liste zu gross wc-list-tables.txt n=<byte>` | Seit E3 hält der ESP die gefilterte Tabellenliste vom Update-Server bis 60 s im RAM (256 Byte). Hier passte sie nicht hinein: Sie wird **nicht** gekürzt und nicht abgelegt, sondern bei jedem Aufruf wieder vom Server geholt. `n` = Byte, die der Zwischenspeicher gebraucht hätte. Gedrosselt: die erste sofort, danach höchstens eine je 10 Minuten. **Hinweis** der Logwache | nie — die heutige Liste braucht rund 140 Byte (E3.0, L450) | jede: Die Liste auf dem Server ist gewachsen, und jeder Aufruf von `update_table_files` geht wieder hinaus. `n` und Uhrzeit in den Bericht |
| `- update cache <datei> geschrieben n=<byte> ms=<dauer>` | Der Hintergrundauftrag hat die Release Notes (`upd-releasenote.txt`) bzw. die STM-Liste (`upd-wc-list.txt`) vom Update-Server geholt und im Dateisystem des ESP abgelegt; `n` = Inhalt ohne Kopfzeile. **`ms` gilt für alle drei `update cache`-Zeilen gleich:** die Dauer des **einen** Schritts, in dem `loop()` stand, nicht des ganzen Durchgangs; 0 bei `quelle-leer` und bei einem Abbruch durch das Sperrfenster. Den längsten Schritt seit dem letzten Rücksetzen meldet `upd_cache_max_ms` in `device_ready` | Je Datei einmal rund 60 s nach dem **ersten** Start mit E3, nach einem neuen Release auf dem Server, nach einer Änderung von Update-Host oder -Pfad, und wenn die Datei fehlt oder ungültig ist (gelöscht, `maintenance_format_fs`) | ohne einen dieser Anlässe, insbesondere **nach einem ESP-Neustart bei unverändertem Server** (AKR.21): Dann schreibt jeder Start den Flash neu. `ms` über 3'000: Befund, weil der STM dann `keine Quittung` meldet |
| `- update cache <datei> unveraendert ms=<dauer>` | Der Auftrag hat die drei Versionszeilen vom Server geholt, der Stand passt zur abgelegten Datei, **nichts geschrieben** | Je Datei einmal frühestens rund 60 s nach jedem ESP-Start, wenn auf dem Server nichts Neues liegt — das ist das Soll von **AKR.21** in T2 (6.0e). Die Wache bleibt still | `ms` über 3'000 (wie oben). Fehlt die Zeile nach einem ESP-Start ganz und steht auch kein `fehlgeschlagen`, ist der Auftrag nicht gelaufen — WLAN, Update-Quelle (S97/S98) und den Mitschnitt der ersten Minuten prüfen |
| `- update cache <datei> fehlgeschlagen <grund> ms=<dauer>` | Der Auftrag hat **nichts** geschrieben; eine vorhandene gültige Datei bleibt und wird weiter ausgeliefert. Gründe: `abruf` (Server nicht erreichbar oder Datei fehlt dort, auch ein schnelles 404), `abgerissen` (Abruf mitten im Inhalt abgebrochen), `schreiben` (Schreibfehler im Flash), `platz` (danach bliebe weniger frei als die grösste PWA-Datei plus ein Block — eine PWA-Installation darf nie am Zwischenspeicher scheitern), `zu-gross` (Notes über 3'072 Zeichen bzw. Liste über 2'048 Byte; nicht abgelegt, nicht gekürzt), `quelle-leer` (Update-Host oder -Pfad leer — der Auftrag fragt dann nie den Server des Ursprungsprojekts, L42; einmal, solange die Lage besteht). Nächster Versuch frühestens nach 10 Minuten. **Hinweis** der Logwache | nur, wenn der Update-Server nicht erreichbar ist | `abruf` oder `abgerissen` **wiederholt bei erreichbarem Server** (die Versionsfelder in `update_status` kommen an) — dann fehlt auf dem Server eine Datei, und der ESP fragt alle 10 Minuten erneut. **Immer ein Befund:** `schreiben`, `platz` (dazu `/api/fs_info` lesend), `zu-gross` (die Notes erscheinen dann gar nicht, siehe unten) und `quelle-leer` — Letzteres heisst Variablenverlust, sofort die Update-Quelle gegen `tools/device.conf` prüfen (S97/S98). `ms` über 3'000 wie oben |
| `- scan n=<n> vor=<free>/<blk> nach=<free>/<blk> flush=<free>/<blk> ms=<scan>/<ausgabe>` | Seit dem ESP-Release E4: ein WLAN-Scan über `/api/network_scan`. `n` = gefundene Netze vor der Kappung (Rohwert des SDK). `vor` und `nach` = freier Heap / grösster freier Block unmittelbar vor und nach dem Scan, `flush` = nach dem letzten Senden der Antwort, solange die Ergebnisliste noch liegt — der Schritt von `nach` zu `flush` ist die Last der Ausgabe. `ms` = Dauer des Scans / Dauer vom Ende des Scans bis nach dem Senden; daran ist für C68 abzulesen, ob die Zeit im Scan oder im Schreiben liegt. Im Mitschnitt und in `/api/stm32_log` textgleich. Die Logwache meldet sie als **Hinweis** | einmal je Wechsel ins Netzwerkmodul (auch beim Start dort) und je Druck auf „WLANs neu laden“, nie im Takt der Selbstaktualisierung | **mehr als eine je Minute**, ohne dass so oft bedient wurde — dann scannt die PWA wieder im Takt, ein Rückfall von E4. Ein `flush`-Block unter 1'072 Byte gehört zu K30 |
| `tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<n>` in der Heap-Zeile | Seit dem ESP-Release R1 (I-5, C72) hinter `low=`; `free=`, `max=`, `ovr=` und `low=` behalten ihre Stellung. `tcp=` = Verbindungen **in diesem Augenblick**, gezählt wie `tcp_now_states` in `device_ready`: ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, CLOSE_WAIT und LAST_ACK aus der Liste der aktiven Verbindungen, `tw` = Länge der TIME_WAIT-Liste. Die Zeile entsteht in `loop()`, nicht in einer Anfrage — anders als in `device_ready` zählt hier keine eigene Abfrage mit. `nrt` = `no_request_timeouts` aus `device_ready`: angenommene Verbindungen, über die bis zur Frist keine Anfrage kam, seit dem ESP-Start, bleibt bei 65'535 stehen | In Ruhe ohne Browser `tcp=0/0/0/0/0/0` oder nur `tw` über 0, das rund 120 s nach der letzten Anfrage auf 0 fällt. Mit offener PWA zeitweise ESTABLISHED und TIME_WAIT. `nrt` steigt, wenn ein Browser Verbindungen auf Vorrat öffnet und nicht benutzt — mit Browser erwartet, Δ ins Protokoll | ESTABLISHED oder CLOSE_WAIT über mehrere Heap-Zeilen ohne bedienten Browser — gehaltene Verbindungen, das Bild von C72: `device_ready` **ohne Parameter** lesen und `tcp_now_list` ins Protokoll. FIN_WAIT_1 oder LAST_ACK über Minuten (die Gegenstelle quittiert nicht). `tw` dauerhaft über 2. `nrt` steigt, während kein Browser offen ist |
| `- update cache vorher up=<s> free=<n>/<blk> frag=<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> hc=<n>` | Seit dem ESP-Release R1 (I-6, C71): einmal je Durchgang des Abgleichs, unmittelbar vor dem ersten Abruf — im ersten Durchgang nach einem Start erst nach dem Löschen liegengebliebener Temporärdateien. `up` = Sekunden seit dem ESP-Start; `free`/`blk` = freier Heap / grösster freier Block (Messpunkt `v`). `frag` = Zerstückelung des Heaps nach dem Core, 0 bis 100: **0 heisst ein einziges Loch**, der Wert steigt mit der Zerstückelung. Auf ein zusätzliches kleines Loch neben einem grossen reagiert `frag` kaum — massgeblich für C71 ist `free − blk` je Messpunkt aus `- update cache heap`. Die Spec nennt `fe=` (Zahl der freien Einträge); der ESP meldet `frag=`, weil die Zahl der Einträge nur über einen privaten Kopf des Cores erreichbar ist. `tcp=` wie in der Heap-Zeile. `hc` = `httpclient_connects`: 0 heisst, der httpclient hat seit dem Start noch keine Verbindung angelegt. **`hc` erfasst den Wetter-Client nicht** — ein Wetter-Overlay vor dem Abgleich legt eine eigene Verbindung an; ob eines lief, gehört ins Protokoll | Je Durchgang genau einmal: rund 60 s nach jedem ESP-Start, nach `update_status?refresh=1` und bei jedem Wiederholversuch. Die Werte sind Ausgangspunkt für `heap` und `nach`, kein Soll | Fehlt sie vor einer `- update cache <datei> …`-Zeile, oder folgen ihr nicht `heap` und `nach` — dann lief der Durchgang anders als gebaut. `blk` schon hier deutlich unter dem Ruhewert nach dem Start: Die Zerstückelung lag vor dem Abgleich (Spec §5.5, N4) |
| `- update cache heap v=<free>/<blk> 1=… 2=… 3=… 4=… 5=… n=<free>/<blk>` | Am Ende des Durchgangs oder bei dessen Abbruch: freier Heap / grösster Block an sieben Messpunkten — `v` vor dem ersten Abruf (wie in `vorher`), `1` bis `3` nach je einer der drei Versionszeilen vom Update-Server (auch wenn sie scheiterte), `4` nach den Release Notes, `5` nach der STM-Liste — beide nach dem Schliessen des Abrufs und dem Aushängen des LittleFS —, `n` am Ende. `-` heisst: Schritt nicht gelaufen. Werte auf 65'535 gekappt. `free − blk` je Punkt ist der Teil des freien Heaps ausserhalb des grössten Blocks — für C71 die massgebliche Grösse, nicht `frag`. **Liefert der Server eine leere Antwort** (200 ohne Inhalt), blieb vor dem ESP-Release R1b dessen Verbindung offen, und die Punkte danach massen mit ihr (L473); seither schliesst `httpclient()` in diesem Fall selbst (C75) | Genau einmal je `vorher`, unmittelbar gefolgt von `nach`. Bei einem Durchgang ohne Schreiben (`unveraendert`) kehrt `blk` bis `n` auf den Wert von `v` zurück (L468) | Ein Schritt, in dem `blk` um 1'000 oder mehr fällt und bis `n` nicht zurückkommt — das Bild von C71: Schritt, Werte, die `- update cache <datei> …`-Zeilen und den Vermerk „Flash-Schreiben beim Start“ ins Protokoll. „Erholt sich nicht“ gilt erst an einer Heap-Zeile mindestens 130 s nach `n`, die `tw` = 0 zeigt — vorher kann eine Verbindung in TIME_WAIT den Block noch scheinbar teilen (Spec §5.5) |
| `- update cache nach frag=<v>/<1>/<2>/<3>/<4>/<5>/<n> tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> hc=<n> acc=<n>` | Unmittelbar nach `heap`: `frag` an denselben sieben Messpunkten (`-` = nicht gelaufen; Lesart wie in `vorher`), dazu `tcp=` und `hc` am Ende des Durchgangs. `acc` = eingehende Verbindungen, die der ESP **während** des Durchgangs angenommen hat. So ist zu sehen, ob eine Anfrage in den Durchgang fiel und ob eine Verbindung der eigenen Abrufe noch in TIME_WAIT steht. `tcp=` und `hc` sind rund 10 ms **nach** `n` erhoben, weil dazwischen die Ausgabe der `heap`-Zeile abgewartet wird (`Serial.flush`). **Lesart von `hc` seit dem ESP-Release R1b:** `hc` > 0 heisst nur noch „der httpclient hat seit dem Start so viele Verbindungen angelegt“, **nicht** mehr „ein Kontext liegt auf dem Heap“. Vorher hielt der statische httpclient seinen Kontext samt ungelesener Antwort bis zum nächsten Verbindungsaufbau; seit L7 gibt `httpclient_stop()` ihn bei jedem Schliessen frei. Eine Belegung zeigen `free` und `blk`, nicht `hc` | Genau einmal je `vorher`. In einer Messung ohne Browser `acc=0`; `tw` über 0 aus den eigenen Abrufen ist erwartet und verfällt nach rund 120 s | `frag` steigt in einem Schritt und bleibt bis `n` oben — zusammen mit `heap` auswerten (C71). `acc` über 0 in einer Messung, die ohne Anfrage laufen sollte (Startmessung): Die Messung ist gestört, den Start so kennzeichnen |
| `- update cache tmp geloescht <datei>` | Der erste Durchgang nach einem Start hat eine liegengebliebene Temporärdatei (`upd-releasenote.tmp` oder `upd-wc-list.tmp`) gelöscht; gemessen am Ergebnis — vorher da, danach weg —, nicht am Rückgabewert des Löschens. Das ist ein Flash-Schreiben beim Start, das bis dahin ohne Zeile blieb | nie — eine Temporärdatei bleibt nur liegen, wenn ein Schreibvorgang mittendrin abbrach | jede: Im Protokoll als „Flash-Schreiben beim Start: ja“ vermerken, und im Mitschnitt vor dem Start nachsehen, was den letzten Schreibvorgang unterbrochen hat (Neustart, Absturz, Stromunterbruch) |
| `- heap erste anfrage free=<n> max=<n>` | Seit dem ESP-Release R1 (I-6): einmal je ESP-Start beim ersten Annehmen einer **lebenden** Verbindung — auch einer, die ein Browser auf Vorrat öffnet und über die nie eine Anfragezeile kommt; eine tote Abholung (`http_accept_dead`) löst sie nicht aus. Inhalt: freier Heap und grösster freier Block in diesem Augenblick | Genau einmal je Start. In der Startmessung (keine Anfrage, bis `heap` erschienen ist, plus 130 s) kommt sie erst mit dem Smoketest danach | Mehr als eine je Start: Der ESP ist unbemerkt neu gestartet — Mitschnitt prüfen. Fehlt sie nach einem Start, obwohl Anfragen kamen. Kam die erste Anfrage **vor** dem Abgleich und liegt `max` schon deutlich unter dem Ruhewert: Zerstückelung vor dem Abgleich (Spec §5.5, N4) |
| `- flash id=<hex6> vendor=<hex2>` | Seit dem ESP-Release R1 (I-6): einmal je ESP-Start in `setup()`, vor dem WLAN und vor dem ersten Flash-Schreiben: die JEDEC-Kennung des Flash-Chips (3 Byte, hexadezimal) und das Herstellerbyte. **`vendor=85` heisst PUYA:** Für diesen Hersteller legt der Core beim ersten Flash-Schreiben einen 256-Byte-Puffer an und gibt ihn nicht mehr frei | Genau einmal je Start, am selben Gerät immer derselbe Wert. Die Uhr meldet `vendor=c4`, kein PUYA (16.10.2026, L468) — N0 entfällt für sie | `vendor=85`: Kandidat N0 bzw. L0 aus der Spec (§5.5) prüfen — im ersten Schreibschritt ΔFree rund −264 ohne Erholung. Ein anderer Wert als beim vorigen Start; fehlt die Zeile nach einem Start |
| `- fs seek error: <datei>` | Seit dem ESP-Release R2 (C70): Nach dem Lesen der beiden Dateienden für das `ETag` liess sich die Datei nicht an den Anfang zurückspulen. Der ESP liefert sie dann **nicht** aus, damit kein Rumpf hinausgeht, der nicht am Dateianfang beginnt — beim Index erscheint die Hinweisseite „WordClock App ist noch nicht installiert“, bei jedem anderen Asset `404`, obwohl die Datei da ist (`BEFUNDE.md`, L479). **Nur im Mitschnitt**, nicht in `/api/stm32_log` und in keinem Zähler (L481). Ebenso `- fs read error: <datei>`: Lesefehler mitten in der Datei, seit R2 mit RST abgebrochen, nicht in `fs_write_short`. Die Logwache kennt beide Zeilen nicht | nie | jede, mit Uhrzeit und Datei; danach `./tools/install-app.sh --check` |
| `- abgewiesen: <was> als Sec-Fetch-Dest=<wert>` | Der ESP hat einen Wartungsaufruf als eingebetteten Abruf (`Sec-Fetch-Dest` etwa `image`) mit 403 und `{"ok":false,"error":"embedded-subresource"}` abgewiesen und **nichts ausgeführt**. `<was>` ist einer der drei Wege, die das seit ESP 3.2.4 prüfen: `maintenance_format_fs`, `maintenance_reset_stm32`, `maintenance_reset_eeprom`. Die vorangehende Zeile `- request <IP> [<Browser>]: …` nennt den Absender. **Hinweis**, kein Alarm; die Logwache meldet sie als Hinweis. **Im Browser wirkt diese Prüfung nicht:** Browser senden `Sec-Fetch-*` über `http://` an die Uhr gar nicht (`BEFUNDE.md`, L439, am Gerät belegt) — die Zeile entsteht nur, wenn ein Werkzeug die Kopfzeile selbst setzt. Die Erweiterung um `Sec-Fetch-Site` und die Legacy-Wege (E1; C6, C6v, C60) ist **nicht ausgeliefert**, Entscheidung des Nutzers 13.10.2026; eine Zeile `als Sec-Fetch-Site=…` gibt es deshalb nicht | Nur noch vom Smoketest: `maintenance_reset_stm32 als Sec-Fetch-Dest=image` bei **jedem** Lauf (Phase 9, Schritt 6) — er löst sie absichtlich aus, wie seit ESP 3.2.4 | Jede ausserhalb eines laufenden Smoketests: Ein Werkzeug hat die Kopfzeile gesetzt — Uhrzeit, `<was>`, Wert und Absender aus der `- request`-Zeile in den Bericht. Umgekehrt ist das **Fehlen** dieser Zeile kein Zeichen von Schutz: Ein Aufruf von einer fremden Seite im Browser kommt ohne Kopfzeile an und wird ausgeführt (L439) |
| `- abgewiesen: query <n> Zeichen` | Eine Anfrage unter `/api/` hatte eine Query über 255 Zeichen; der ESP hat sie mit `{"ok":false,"error":9,"detail":"query too long"}` beantwortet und keinen Handler aufgerufen, statt still zu kürzen (C46). `n` ist die Länge der Query hinter `?`; der Inhalt steht bewusst nicht darin (L179). Legacy-Pfade kürzen weiter still und geben keine Zeile. **Hinweis**; die Logwache meldet sie als Hinweis | Nur in **K27a** | Jede andere. Kommt sie aus der PWA, hat die Oberfläche eine zu lange Anfrage gebaut — Endpunkt aus der `- request`-Zeile davor und `n` in den Bericht. Seit dem PWA-Teil von C46 (Release EP) prüft die PWA die Länge **vor** dem Senden: Eine Query über 255 Zeichen geht gar nicht hinaus, die Meldung nennt Länge und Grenze („Die Anfrage ist zu lang: … Zeichen, erlaubt sind höchstens 255. …“), und Kennung 9 vom Gerät ergibt dieselbe Meldung. Mit EP ist deshalb **jede** solche Zeile aus der PWA ein Befund, auch im Backup-Import (Phase 5): Er schreibt die WLAN-Felder seither einzeln (8.3). Vor EP war sie dort die bekannte Lücke aus L431 |

**Lesart von `http write lost` und `fs write short`** (seit dem ESP-Release E3,
`specs/paket-2026-10-13/design.md` §18.3). Massgebend sind die Verbindungslage und `snd`;
`blk` steht immer daneben. Bei `http write lost` zeigt die Zeile selbst, ob `gone` oder
`fail` gestiegen ist, bei `fs write short` der Vergleich von `write_lost_gone` und
`write_lost_failed` in `device_ready`. Der „ungeschriebene Rest“ ist bei `http write lost`
`rest`, bei `fs write short` `n` minus `w`.

| Verbindung | `snd` | Lesart |
|---|---|---|
| weg (`gone` gestiegen) | 0 | **Gegenstelle weg** — der Browser hat abgebrochen. Kein Befund (L270) |
| steht (`fail` gestiegen) | gleich dem vollen Sendepuffer (1'072) **oder** mindestens so gross wie der ungeschriebene Rest | **Heap** — Platz im Sendepuffer war da, das Schreiben bekam keinen Speicher. `blk`, `low=` und `heap_low_block` danebenstellen; gehört zu C65 |
| steht | 0 | **Quittung fehlt** — der Sendepuffer ist voll, die Gegenstelle nimmt nicht ab (Fenster 0, Funkloch, Energiesparbetrieb). Seit E3 greift die Sendezeitgrenze früher (1 s für API-Antworten, 2 s für Dateien); langsame Gegenstellen treffen sie häufiger — benanntes Risiko, kein Rückfall |
| steht | dazwischen | **unklar** — Rohwerte berichten, nicht deuten |

**Wie lange `loop()` dabei steht:** Seit E3 hält ein Schreibversagen den ESP im Regelfall
höchstens rund 2,6 s an, statt bis zu rund 10 s davor. Im seltenen Fall, dass `http_flush()`
einen Teil unterbringt und dann scheitert, sind es bis rund 4,6 s (Review E3) — dann ist ein
`ms` über 3'000 und ein `keine Quittung` möglich; beides bleibt ein Befund nach der Tabelle oben.

#### Die Messfelder in `/api/device_ready` (seit dem ESP-Release E3)

Das ESP-Release E3 (C65) hängt neun Felder **hinten** an `device_ready` an, in dieser
Reihenfolge; die bestehenden Felder bleiben in Name, Reihenfolge und Typ unverändert.

| Feld | Was es heisst |
|---|---|
| `heap_low_block` | Tiefstwert des grössten freien Blocks seit dem letzten Rücksetzen, aus denselben vier Messpunkten wie `low=` (`flush`, `fs`, `accept`, `client`). `4294967295`, solange seit dem Rücksetzen kein Messpunkt lief |
| `heap_low_free` | Tiefstwert des freien Heaps insgesamt im selben Fenster — eigenständig, **nicht** der Wert zum Zeitpunkt von `heap_low_block` |
| `heap_low_path` | wo das Block-Tief gemessen wurde: `flush` (API-Antwort), `fs` (Dateiauslieferung; nur erster und jeder achte Block, kann das Tief dort unterschätzen), `accept` (Verbindung angenommen), `client` (ausgehende Verbindung); leer, solange nichts gemessen wurde. Bei Gleichstand bleibt der erste Pfad stehen |
| `heap_low_ms` | Betriebszeit des ESP in Millisekunden beim Block-Tief — zum Zuordnen gegen den Mitschnitt |
| `httpclient_connects` | ausgehende Verbindungen seit dem ESP-Start: Anfragen an den Update-Server, Hintergrundauftrag, Legacy-Seiten, STM-Flash-Download; der Wetterabruf zählt nicht. Wird **nie** zurückgesetzt, bleibt bei 65'535 stehen. Die Differenz über einen `update_status` zeigt, ob er kalt (Soll 6) oder warm (0) lief |
| `fs_write_short` | abgebrochene Dateiauslieferungen seit dem ESP-Start (Teil von `write_lost_blocks`), nicht rücksetzbar |
| `req_max_ms` | längste Verbindung von der Annahme bis zum Ende ihrer Bedienung, seit dem letzten Rücksetzen |
| `req_max_path` | Pfad dieser Verbindung **ohne Query** (sie kann Zugangsdaten tragen), auf 31 Zeichen gekürzt. Leer = seit dem Rücksetzen nichts gemessen, `-` = Verbindung ohne Anfrage |
| `upd_cache_max_ms` | längster Schritt des Hintergrundauftrags für Release Notes und STM-Liste — dieselbe Dauer wie `ms=` der `- update cache`-Zeilen. Seit dem ESP-Release R1 rund 10–30 ms höher, weil die I-6-Zeilen im selben Schritt ausgegeben werden; beim Vergleich mit E4 berücksichtigen |

**Rücksetzen nur mit `?heap_low_reset=1`.** `/api/device_ready?heap_low_reset=1` gibt die
gelesenen Werte aus und setzt danach `heap_low_block`, `heap_low_free`, `heap_low_path`,
`heap_low_ms`, `req_max_ms`, `req_max_path` und `upd_cache_max_ms` zurück. Es ändert nur
Messwerte, keine Einstellung. **Ohne Parameter wird nur gelesen**, nichts zurückgesetzt. Der
rücksetzende Aufruf zählt mit seiner Annahme noch in das gelesene Fenster, mit seiner eigenen
Antwort schon in das nächste. Smoketest, `flash-stm.sh`, `snapshot-device.sh` und die PWA
setzen nie zurück — ein ESP-Neustart dagegen setzt alles auf den Anfang, wie alle ESP-Zähler
(S114).

**Der Smoketest verlangt die neun Felder ab dem ESP-Release E3** und meldet „FEHLT“, wenn
eines fehlt; vorher meldet er sie als „gibt es noch nicht“. Er liest nur, gibt Tiefstwert,
Pfad, längste Anfrage und längsten Schritt des Auftrags aus und meldet „FEHLT“, solange
`fs_write_short` über 0 steht.

**`write_lost_failed` zählt seit E3 auch abgebrochene Assets.** Ein `fs write short` bei
stehender Verbindung erhöht ihn genauso wie ein `http write lost … fail=`. Der Smoketest
beobachtet `write_lost_failed` (C49) und schlägt damit auch bei einem abgebrochenen Asset an
— gewollt: Bis E3 war das ein stiller Verlust. Geht dagegen schon der **Kopf** eines Assets
verloren, zählt es als `http write lost` und **nicht** in `fs_write_short` (Review E3, N1).

#### Die Momentaufnahme beim Tief in `/api/device_ready` (seit dem ESP-Release E4)

Das ESP-Release E4 (C65, `specs/paket-2026-10-13/design-e4.md` §19.2) hängt acht Felder
**hinter** `upd_cache_max_ms` an, in dieser Reihenfolge; alle bisherigen Felder bleiben in
Name, Reihenfolge und Typ. Die ersten sechs sind eine **Momentaufnahme** (I-4): Der ESP
schreibt sie nur in dem Augenblick, in dem `heap_low_block` einen **neuen** Tiefstwert
erreicht — ein gleich tiefer oder höherer Wert ändert sie nicht, ein neues Tief von
`heap_low_free` auch nicht. Sie sagen damit, **was beim Tief los war**: wer den Heap
belegte, nicht nur, wie wenig übrig blieb.

| Feld | Was es heisst | Lesart |
|---|---|---|
| `heap_low_accepts_pending` | wartende, vom Programm noch nicht angenommene Verbindungen am Port 80 | jede kostet rund 0,8 KB (L456). Soll: höchstens `heap_low_backlog` |
| `heap_low_backlog` | der Backlog, der in diesem Augenblick galt (3, 2 oder 1) | zeigt, ob die Absenkung beim Tief schon gegriffen hatte |
| `heap_low_pcb_states` | genau **11 Zahlen**: Zahl der Verbindungen je Zustand in der Liste der aktiven Verbindungen. Index = lwIP `enum tcp_state`: 0 `CLOSED`, 1 `LISTEN`, 2 `SYN_SENT`, 3 `SYN_RCVD`, 4 `ESTABLISHED`, 5 `FIN_WAIT_1`, 6 `FIN_WAIT_2`, 7 `CLOSE_WAIT`, 8 `CLOSING`, 9 `LAST_ACK`, 10 `TIME_WAIT` (Reihenfolge am Core nachgeprüft, `lwip/tcpbase.h`) | Verbindungen in TIME_WAIT führt lwIP in einer eigenen Liste, sie stehen in `heap_low_tw`. Viele in 7 (`CLOSE_WAIT`): Die Gegenstellen haben geschlossen, der ESP noch nicht |
| `heap_low_tw` | Zahl der Verbindungen in TIME_WAIT | seit E4 räumt der ESP vor jeder Annahme und vor jedem ausgehenden Verbindungsaufbau auf höchstens 2 ab; Soll im Gerätenachweis G8: höchstens 2. Jede kostet rund 192 Byte |
| `heap_low_unacked` | unquittierte Byte der gerade bedienten Verbindung (Sendepuffer minus freier Platz darin); **−1** = keine Verbindung bedient | nahe 1'072: Der ganze Sendepuffer war belegt, die Gegenstelle hatte noch nicht quittiert |
| `heap_low_req_path` | Pfad der Anfrage, die gerade bedient wurde, **ohne Query**, höchstens 31 Zeichen; `-` = Messpunkt ohne Anfrage (Annahme vor dem Lesen, ausgehender Abruf beim Start) | `heap_low_path` sagt, an welchem Messpunkt, dieses Feld, bei welcher Anfrage |
| `http_backlog` | **aktueller** Backlog nach freiem Heap: 3 ab 5'900 Byte, 2 ab 5'100, darunter 1; erhöht wird erst 256 Byte über der Schwelle | **Zustand, wird nie zurückgesetzt.** In Ruhe 3 — eine 2 oder 1 in Ruhe heisst, der freie Heap liegt dauerhaft tief. Die Schwellen sind gerechnet, nicht gemessen |
| `api_orphan_skipped` | lesende `/api/`-Abfragen seit dem ESP-Start, deren Gegenstelle schon weg war und die der ESP deshalb nicht ausgeführt hat | **Zähler, wird nie zurückgesetzt**, bleibt bei 65'535 stehen. Steigt, wenn die PWA einen Abruf abbricht, der beim ESP noch wartet. Setter, Wartung und Auslöser überspringt der ESP nie (C53), auch `update_status?refresh=1` nicht |

**Rücksetzen:** `device_ready?heap_low_reset=1` setzt die sechs Felder der Momentaufnahme
zusammen mit den Feldern aus E3 zurück — Zahlen auf 0, `heap_low_unacked` auf −1,
`heap_low_req_path` leer. Ob seit dem Rücksetzen etwas gemessen wurde, zeigt wie bisher
`heap_low_path` (leer = nichts). `http_backlog` und `api_orphan_skipped` bleiben stehen.
Wer ein Tief auswertet, liest **zuerst ohne Parameter** und setzt erst danach zurück.

**Der Smoketest verlangt die acht Felder ab dem ESP-Release E4** und meldet „FEHLT“, wenn
eines fehlt oder `heap_low_pcb_states` nicht genau 11 Zahlen trägt — dann liefen Firmware
und diese Lesart auseinander. Vorher meldet er „gibt es noch nicht“. Er liest nur, setzt
nichts zurück und gibt wartende Verbindungen und TIME_WAIT beim Tief, den aktuellen Backlog
und die übersprungenen Lesenden aus; einen Sollwert prüft er dabei nicht.

#### Die Verbindungen jetzt in `/api/device_ready` (seit dem ESP-Release R1, Paket 2026-10-15)

Das ESP-Release R1 (C72, `specs/paket-2026-10-15/design.md` §3.1) hängt acht Felder **hinter**
`api_orphan_skipped` an, in dieser Reihenfolge; alle bisherigen Felder bleiben in Name,
Reihenfolge und Typ. Anders als die Momentaufnahme aus E4 zeigen sie, was **im Augenblick der
Abfrage** besteht. Sie werden nie zurückgesetzt, auch nicht mit `heap_low_reset=1` — ausser
`http_accept_dead` sind es Augenblickswerte.

| Feld | Was es heisst | Lesart |
|---|---|---|
| `tcp_now_states` | genau **11 Zahlen**: Verbindungen je Zustand in der Liste der **aktiven** Verbindungen, Index = lwIP `enum tcp_state` wie bei `heap_low_pcb_states`; sättigend bei 255 | **Index 1 (`LISTEN`) und 10 (`TIME_WAIT`) stehen immer auf 0:** Der Listener steht in einer eigenen lwIP-Liste, die nicht gelesen wird, und TIME_WAIT zählt `tcp_now_tw`. Die eigene Abfrage zählt in Index 4 (`ESTABLISHED`) mit |
| `tcp_now_tw` | Länge der TIME_WAIT-Liste | seit E4 räumt der ESP vor jeder Annahme auf höchstens 2 ab; rund 120 s nach der letzten Anfrage 0 |
| `tcp_now_pending` | wartende, vom Programm noch nicht angenommene Verbindungen am Port 80 (`accepts_pending`) | in Ruhe 0 |
| `tcp_now_unclaimed` | Länge der Warteliste des Servers: Verbindungen mit angelegtem `ClientContext`, die noch auf die Abholung durch das Programm warten | in Ruhe 0 |
| `tcp_now_self_port` | Port der Gegenstelle **dieser** Abfrage | zum Wiederfinden der eigenen Verbindung in `tcp_now_list` |
| `tcp_now_list` | höchstens **8** Einträge `[zustand, gegenstelle, port, ruhe_s, nrtx, arg]`, erst die aktiven, dann die TIME_WAIT-Verbindungen. `zustand` = `enum tcp_state`; `gegenstelle` = letztes Oktett der IP-Adresse; **`port` = Port der Gegenstelle**, nicht 80; `nrtx` = Wiederholungen des laufenden Segments; `arg` siehe unten | **`ruhe_s` heisst „Sekunden seit dem letzten Segment der Gegenstelle vor unserem `close()`“**, nicht „seit dem Eintritt in den Zustand“: lwIP setzt den Zeitgeber nach dem Schliessen nicht mehr und auch nicht beim Zustandswechsel. Bei TIME_WAIT misst er also nicht die Zeit in TIME_WAIT (die endet rund 120 s nach dem Schliessen). **Die eigene Abfrage steht selbst in der Liste** — `ESTABLISHED`, `arg` 1, `port` gleich `tcp_now_self_port`; das ist kein Befund |
| `tcp_now_more` | Verbindungen, die nicht mehr in die Liste passten | über 0: mehr als 8 Verbindungen gleichzeitig — ins Protokoll |
| `http_accept_dead` | Zähler seit dem ESP-Start, bleibt bei 65'535 stehen: Verbindungen, die der ESP aus der Warteliste abholte, deren Gegenstelle aber schon geschlossen hatte — sie wurden ohne Anfrage verworfen. Gezählt nur, wenn vor der Abholung eine Verbindung wartete; leere Durchläufe zählen nicht | zusammen mit `no_request_timeouts` und `no_request_aborts` auswerten (C72, Spec §5.4) |

**`arg` in `tcp_now_list`:** **0** = ohne `ClientContext` — der ESP hat die Verbindung bereits
geschlossen; steht sie trotzdem in `ESTABLISHED` oder `CLOSE_WAIT`, konnte das FIN nicht
eingereiht werden. **1** = mit `ClientContext`: eine bediente, wartende oder ausgehende
Verbindung. **2** = das Argument des Listeners: eine Verbindung im Aufbau (`SYN_RCVD`), noch
ohne `ClientContext`.

**Heap-Zeile und `device_ready` nicht gleichsetzen:** `tcp=` der Heap-Zeile entsteht in
`loop()` ohne eigene Verbindung, `tcp_now_states` in einer Anfrage, die sich selbst mitzählt.
In Ruhe zeigt die Heap-Zeile deshalb `ESTABLISHED` 0, `device_ready` 1.

**Der Smoketest verlangt die acht Felder ab dem ESP-Release R1** und meldet „FEHLT“, wenn eines
fehlt oder `tcp_now_states` nicht genau 11 Zahlen trägt; vorher meldet er sie als „gibt es noch
nicht“. Er liest nur, gibt die aktiven Zustände, TIME_WAIT, wartende Verbindungen und
`http_accept_dead` aus und prüft keinen Sollwert.

#### Asset-Cache: `ETag`, `304` und die Zähler in `/api/device_ready` (seit dem ESP-Release R2, Paket 2026-10-15)

Bis zum ESP-Release R2 holte der Browser bei jedem Öffnen und Neuladen alle App-Dateien
vollständig vom ESP (C70, `BEFUNDE.md`, L456). Seit R2 (`specs/paket-2026-10-15/design.md` §4.4,
Ent-2 Variante T) gilt für die **App-Assets** — die `.gz`-Dateien aus der Weissliste
`APP_INSTALL_ASSETS`:

| Was | Wie |
|---|---|
| `ETag` | `"<grösse der .gz>-<16 Hexziffern>"`: die Grösse der `.gz` in Byte, ein Bindestrich und die **letzten 8 Byte** der `.gz` in Dateireihenfolge, Kleinbuchstaben, mit den Anführungszeichen. Diese 8 Byte sind CRC-32 und Länge des entpackten Inhalts (RFC 1952). Der ESP bildet den Wert bei jeder Antwort aus der Datei selbst; ein neues Hochladen ändert ihn damit ohne weiteres Zutun. Beginnt die Datei nicht mit `1f 8b` oder ist sie kürzer als 18 Byte, gibt es kein `ETag` und nie eine `304` |
| erwarteter Wert | aus derselben `.gz`, die auf dem Gerät liegt: `"$(wc -c < <datei>.gz \| tr -d ' ')-$(tail -c 8 <datei>.gz \| xxd -p)"` |
| `If-None-Match` trifft | `HTTP/1.1 304 Not Modified` mit `ETag`, `Cache-Control: no-cache` und `Connection: close`, **ohne Rumpf** und ohne `Content-Length`. Gelesen werden dafür nur die beiden Dateienden. Ein vorangestelltes `W/` wird übergangen, eine Komma-Liste ausgewertet; `*` trifft nie, ein Wert über 35 Zeichen ebenfalls nicht |
| trifft nicht oder fehlt | `HTTP/1.1 200 OK` mit `Content-Length` gleich der Dateigrösse, `ETag`, `Cache-Control: no-cache` und `Connection: close` |
| alles andere | **bleibt `HTTP/1.0` ohne `ETag`**: `/api/*`, die Legacy-Oberfläche, die Hinweisseiten unter `/app/` (Installation, „noch nicht installiert“) und Dateien unter `/app/` ausserhalb der Weissliste |

**Warum `HTTP/1.1` nur dort:** Chrome schickt bei `HTTP/1.0` für das Dokument kein
`If-None-Match` (AKC.7, L468). Keep-Alive entsteht trotzdem nicht — beide Antworten tragen
`Connection: close`, und der ESP schliesst danach wie bisher.

**Was das `ETag` nicht abdeckt:** nur den Rumpf. Ändert sich allein der Kopf einer Antwort —
etwa der Inhaltstyp, den `http_content_type()` einer Endung zuordnet —, antwortet der ESP weiter
mit `304`, und der Browser behält den alten Kopf, bis sich die Datei selbst ändert (Review R2.7,
H4; `BEFUNDE.md`, C70).

**Ein abgebrochenes Senden endet seit R2 mit RST.** Bis dahin schloss der ESP auch im Abbruch
regulär (FIN); das dafür vorgesehene `abort()` kam danach und fand keine Verbindung mehr. Der
Browser erkannte den Schaden erst an der falschen Länge. Jetzt meldet Chrome
`net::ERR_CONNECTION_RESET` — in `check-pwa.sh` unter `INFO Verbindungsfehler` (4b), in der
Ladeprobe als `fehl=` mit `FAIL net::ERR_CONNECTION_RESET` in der Pfadzeile (4c). Mit RST endet
jeder Abbruch in `http_send_fs_file()`: ein Schreibabbruch (`- fs write short`), ein verlorener
Kopf (`- http write lost`) und ein Lesefehler (`- fs read error`). **Für AKE.3 heisst das:** Resets werden gegen die Grundlinie
auf R2 gezählt (AKW.6), nicht gegen Läufe davor.

Zwei Felder **hinter** `http_accept_dead`, alle bisherigen bleiben in Name, Reihenfolge und Typ:

| Feld | Was es heisst | Lesart |
|---|---|---|
| `app_not_modified` | App-Assets, die der ESP seit seinem Start mit `304` beantwortet hat | in einer warmen Ladeprobe mit Cache steigt es um die Zahl der `/app/`-Anfragen |
| `app_full` | App-Assets, für die der ESP seit seinem Start eine `200` mit Rumpf **begonnen** hat. **Gezählt wird beim Kopf**, bevor der Rumpf läuft — ein Abbruch zählt also mit | Soll in einer warmen Ladeprobe: Δ`app_full` = 0 (AKW.2). **Vollständige Auslieferungen** sind Δ`app_full` minus Δ`fs_write_short`. Das ist eine Obergrenze: Geht schon der Kopf verloren (zählt als `http write lost`, nicht in `fs_write_short`) oder bricht das Lesen ab (`- fs read error`, in keinem Zähler), steht die Auslieferung in `app_full` und wird nicht abgezogen |

Beide zählen seit dem ESP-Start, bleiben bei 65'535 stehen und werden nie zurückgesetzt, auch
nicht mit `heap_low_reset=1`. Die Ladeprobe gibt sie nach jedem Laden in der Zeile
`Geraetezaehler` aus (4c).

**Der Smoketest prüft beides ab dem ESP-Release R2.** Im Block C70 holt er `/app/`,
`/app/app.js` und `/app/styles.css` je einmal; die Antwort muss ein `ETag` tragen. Derselbe Wert
als `If-None-Match` muss eine `304` als `HTTP/1.1` mit 0 Byte ergeben, ein falscher Wert
(`"c70-falsch"`) eine `200` mit Rumpf. „Asset-Zähler (C70)“ verlangt die beiden Felder in
`device_ready`. Vor R2 meldet er den Block als „gibt es noch nicht“; `SMOKE_C70_OVERRIDE=1`
erzwingt ihn auf einem älteren Stand — die Gegenprobe nach DIR-014, dort muss er fehlschlagen.
**Der Smoketest erhöht beide Zähler selbst.** Wer `app_full` oder `app_not_modified` über einen
Zeitraum vergleicht, lässt dazwischen keinen Smoketest laufen.

#### Release Notes und STM-Liste aus dem Dateisystem (seit dem ESP-Release E3)

Bis E3 holte der ESP die Release Notes und die STM-Dateiliste (`wc-list.txt`) bei **jedem**
`update_status` vom Update-Server, mitten in der eigenen Antwort. Seit E3 holt sie ein
Auftrag in `loop()` — einmal nach dem ESP-Start, frühestens rund 60 s danach und mindestens
5 s nach der letzten Anfrage — und legt sie im LittleFS ab: `/upd-releasenote.txt` und
`/upd-wc-list.txt`. **Neu geschrieben wird nur, wenn sich die Release-Versionen auf dem
Server oder Update-Host bzw. -Pfad geändert haben**, oder wenn eine Datei fehlt oder
ungültig ist. **Seit dem ESP-Release E4** gleicht der Auftrag im ersten Durchgang nach jedem
Start und nach `update_status?refresh=1` beide Dateien zusätzlich gegen den Server ab, auch
bei gleicher Version, und schreibt nur bei abweichendem Inhalt (E4.5a, `BEFUNDE.md`, L457).
Eine bei gleicher Version geänderte Release Note kommt damit an; bis dahin blieb die alte bis
zum nächsten Release stehen, auch über einen Neustart. Bei gleichem Inhalt meldet der Abgleich
`unveraendert` — AKR.21 gilt weiter. `update_status` liefert aus der Datei. Was das beim Prüfen heisst:

- **Release Notes leer:** in den ersten rund 60 s nach einem Start, solange noch keine
  gültige Datei liegt — beim ersten Start mit E3, nach `maintenance_format_fs` und nach dem
  Löschen der Datei. Liegt die Datei schon, bleiben die Notes auch nach einem Neustart
  sichtbar (AKR.21). Kein Befund.
- **Notes über 3'072 Zeichen** werden nicht abgelegt und erscheinen **gar nicht** mehr
  (`fehlgeschlagen zu-gross`); bis E3 wurden sie gekürzt angezeigt (Review E3, N3). Die
  heutigen Notes haben 720 Byte (E3.0, L450).
- **Fehlt eine der drei Versionsdateien auf dem Server dauerhaft**, bleibt der Zwischenspeicher
  ungeschrieben, die Notes bleiben leer, und der ESP versucht es alle 10 Minuten erneut —
  ein schnelles 404, im Mitschnitt `fehlgeschlagen abruf`. Ein Befund am Server, nicht am Gerät.
- **Die STM-Liste fehlt nie:** Ist ihre Datei ungültig, holt `update_status` sie wie bisher
  live.
- **Die beiden Dateien erscheinen in der Dateiliste** (`/api/fs_list`, Wartung). Sie sind
  keine PWA-Assets, tragen kein `.gz`, stehen nicht in der Weissliste und sind über `/app/`
  nicht erreichbar. Wer sie löscht, bekommt sie beim nächsten Durchgang neu — kein Befund.
  Die Temporärdateien `/upd-*.tmp` existieren nur während des Schreibens; liegen gebliebene
  löscht der erste Durchgang nach einem Start.
- **Seit dem ESP-Release R1** schreibt jeder Durchgang zusätzlich drei Messzeilen
  (`- update cache vorher`, `heap`, `nach`) und je gelöschter Temporärdatei
  `- update cache tmp geloescht <datei>` — Lesart in der Tabelle unter 5b. Sie sind kein
  Befund an sich und enthalten nie `fehlgeschlagen`. Die Ausgabe kostet Zeit: `ms=` der
  `- update cache`-Zeilen und `upd_cache_max_ms` liegen seither rund 10–30 ms höher. Beim
  Vergleich mit Werten aus E4 ist das kein Rückschritt.

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

### 6.0e Der Umfang von T2 — was T1 ausgelassen hat

Der Durchlauf T1 vom 14.10.2026 hielt nach dem lesenden Teil an der Haltegrenze K30 an
(`BEFUNDE.md`, L449): Phase 3 (113 von 114 Prüfungen), die Grenzfälle E1 bis E8 aus
Phase 4 und AKL.4 sind **nicht** gefahren. Der nächste vollständige Durchlauf, **T2**, fährt
nach den Releases E3 und E4 die Phasen 0 bis 4 und 9 und dazu ausdrücklich:

1. **AKL.4** — S25, S25b und S26 **über die Oberfläche**, wie in S25b unter „In der
   Oberfläche (Release EP, AKL.4)“ beschrieben: beide Abweisungen, danach „gespeichert“,
   Endstand 0/4095 lesend im Rohabzug, Δ`v` = 0.
2. **Die Setter-Phasen, die T1 ausgelassen hat**: den Rest von Phase 3 und E1 bis E8 aus
   Phase 4, gerahmt nach V1 und V1b (5b).
3. **AKR.21 am ESP-Neustart aus V3.** Der STM-Reset in Phase 9 reisst den ESP mit (L183).
   Frühestens rund 60 s danach muss im Mitschnitt je Datei
   `- update cache <datei> unveraendert ms=…` stehen und **kein** `geschrieben`; die Release
   Notes bleiben in der PWA sichtbar. Ist V3 nicht freigegeben, AKR.21 als „nicht
   aussagekräftig: kein ESP-Neustart“ führen.
4. **Bei jedem Halt an K30 die Momentaufnahme** aus `device_ready` in den Bericht — die
   I-4-Felder aus 5b („Die Momentaufnahme beim Tief“), gelesen ohne Parameter, bevor
   zurückgesetzt wird.

**Wann T2 läuft:** erst nach dem Gerätenachweis G8 zu E4, dem Entscheid danach (E4.19), dem
Rest von G7 auf dem Stand E4 (G7R) **und** G6 (`specs/paket-2026-10-13/tasks.md`). Verfehlt
G8 sein Ziel, läuft T2 nur nach Entscheidung des Nutzers (`design-e4.md` §19.13).

Die Haltegrenze ist K30 in der Fassung nach Ent-23 (7.1): Ruhewert `max` unter 5'120 oder
Tiefstwert unter 1'072 — dazwischen wird protokolliert, nicht angehalten. Die Schreibzeilen
`http write lost` und `fs write short` werden nach der Entscheidungsregel aus 5b gelesen.
Fehlt einer der vier Punkte im Bericht, ist T2 nicht abgenommen; die Abdeckungszahl aus 6.0c
nennt ihn dann mit Grund.

### 6.0 `main`

Das Modul hat zwei Schaltflächen und stand bisher in keinem Abschnitt dieses Plans.
Die beiden Hauptschalter der Uhr liegen hier, nicht im Modul `display`.

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S1** | Display ein- und ausschalten (`display_power_set?value=on\|off`) | S | `numvar[idx=3].value` (`DISPLAY_POWER`) steht auf `1` bzw. `0` | Sofort am Display sichtbar. Zuletzt prüfen und sofort zurücknehmen — eine dunkle Uhr fällt im Wohnraum auf. **Im Aus-Fenster eines Display-Timers** (nachts, Bestimmung wie in „V3 im Nachtzustand“) ist der Ausgangswert `0`: erst `on`, dann sofort `off` — die Rückstellung ist „wieder aus“, und `numvar[idx=3]` muss danach auf `0` stehen. In T.2 lief das Display hier 41 s an (L390) |
| **S2** | Ambilight ein- und ausschalten (`ambilight_power_set?value=on\|off`) | S | `numvar[idx=30].value` (`DISPLAY_AMBILIGHT_POWER`) steht auf `1` bzw. `0`. Seit dem STM-Release S2 aus `specs/paket-2026-10-13/` (A56) folgt jedem **Wechsel** im Mitschnitt `eep a=1979 n=2` — der STM speichert den Schaltzustand; ein unveränderter Wert schreibt nicht | An dieser Uhr ohne sichtbare Wirkung (L11) — nur am Rohwert prüfbar. Dieselbe `eep`-Zeile kommt auch nach S1, wenn das Ambilight dort mitschaltet. Ob der gespeicherte Wert einen Neustart übersteht, prüft **K20**, ob DCF77 dem Ambilight folgt, **K23** (7.1) |

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

**Seit dem Release E4 scannt die PWA nur auf Anlass** (4b, „Wie die PWA seit dem Release E4
abfragt“): beim Wechsel ins Modul, beim Start dort und per „WLANs neu laden“ — nicht mehr im
15-s-Takt. Soll je Anlass genau ein `network_scan` und eine Zeile `- scan …` im Mitschnitt
(5b); ein erneuter Klick auf das schon offene Modul scannt nicht. Läuft die Frist ab, wird
nicht wiederholt: Die alte Liste bleibt stehen, der Knopf meldet den Fehlschlag. **Bekannte
Lücke** (`BEFUNDE.md`, B60): Wer vor dem Ende der ersten Kernladung ins Netzwerkmodul
wechselt, sieht keine Netzliste, bis er den Knopf drückt oder das Modul neu betritt. Kein
Fehler dieses Durchlaufs, aber benennen, wenn es auftritt.

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
erst die Koordinaten, dann den Ort. Den Fokus nach einem Klick auf den Hintergrund
prüft **K12** (7.1): Am 10.10.2026 lag er danach auf dem Body, mit Escape kehrte er
richtig zurück (`BEFUNDE.md`, L368). Die Korrektur aus B47 ist in der Vorschau
nachgewiesen, am Gerät noch nicht.

| Kennung | Was | Klasse | Soll (nachprüfbar) | Besonderheit |
|---|---|---|---|---|
| **S14** | Wetter-AppID setzen (`weather_appid_set?value=`) | S | `strvar[idx=5].value` (`WEATHER_APPID`) trägt den Wert, Grenze 32 | Leerer Wert wird mit `error=1` abgewiesen |
| **S15** | Ort setzen (`weather_city_set?value=`) | S | `strvar[idx=6].value` (`WEATHER_CITY`), Grenze 32 | |
| **S16** | Koordinaten setzen (`weather_coordinates_set?lon=&lat=`) | S | `strvar[idx=7].value` (`WEATHER_LON`) und `strvar[idx=8].value` (`WEATHER_LAT`), je Grenze 8 | Nur eines von beiden gesetzt ⇒ `error=1`. Seit C44 prüfen ESP und PWA die Form (Ent-12): Dezimalzahl mit optionalem `-`, höchstens 8 Zeichen, Längengrad −180..180, Breitengrad −90..90, Komma wird zu Punkt. Abgewiesene und gültige Werte: **K15**, **K16** (7.1) |
| **S17** | Ort leeren, während keine Koordinaten stehen | S | `error=1`, `strvar[idx=6].value` unverändert | Ort und Koordinaten sind Alternativen; die letzte Angabe darf nicht wegfallen |
| **S18** | Wetter jetzt abrufen (`weather_get_now`) | S | Am Display erscheint die Wetterzeile; der Ticker läuft danach wieder an | **Nicht L.** `{"ok":true}` kommt auch bei unbrauchbarer AppID (L54) — Beleg ist die Anzeige, nicht die Antwort |
| **S19** | Vorhersage abrufen (`weather_get_forecast`) | S | wie S18 | |
| **S20** | DS18xx-Korrektur setzen (`temperature_ds18xx_correction_set?value=`) | S | `numvar[idx=24].value` (`DS18XX_TEMP_CORRECTION`) trägt den Wert | Bereich −20..20 in halben Grad |
| **S21** | RTC-Korrektur setzen (`temperature_rtc_correction_set?value=`) | S | `numvar[idx=22].value` (`RTC_TEMP_CORRECTION`) | Negative Werte stehen im Rohwert als Zweierkomplement eines Bytes: **−2 erscheint als `254`**. Die PWA rechnet das für die Anzeige um. Kein Befund — im Protokoll Rohwert und umgerechneten Wert nennen (10.10.2026) |
| **S22** | Korrektur `21` und `-21` setzen | S | `error=2`, beide Variablen unverändert | |
| **S23** | Temperatur anzeigen (`temperature_display`) | S | Die Temperatur erscheint auf dem Display | Zeigt der Sensor `127.5`, ist das **keine Messung**, sondern der STM-Fehlerwert `255` |
| **S24** | Automatische Helligkeit umschalten (`auto_brightness_set?value=on\|off`) | S | `numvar[idx=8].value` (`DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE`) 1 bzw. 0 | **Das Bedienelement liegt im Modul `climate`, nicht in `display`** — es steht im LDR-Bereich. Solange es an ist, überschreibt der LDR die Helligkeit aus S29 |
| **S25** | Aktuellen Messwert als LDR-Minimum übernehmen (`ldr_min_set`) | S | `numvar[idx=17].value` (`LDR_MIN_VALUE`) entspricht dem zuvor abgelesenen `numvar[idx=16].value` (`LDR_RAW_VALUE`); im Logring (`/api/stm32_log`, Präfix `LOG `) und im Mitschnitt folgt `eep a=225 n=2` | **Rücknehmbar über S27a**, seit A59 der STM die numerischen Setter übernimmt (`BEFUNDE.md`, L364). Vorher den Ausgangswert von `numvar[idx=17].value` aus dem Rohabzug notieren; seit der Bereinigung am 10.10.2026 stimmen ESP-Kopie und STM bei den Grenzen überein (L377). Nach der Gegenprobe sofort `ldr_min_value_set?value=<Ausgangswert>` — Soll wie S27a, also wieder mit `eep a=225 n=2`. Keine eigene Freigabe mehr, sondern wie jeder rücknehmbare S-Fall. Der Rohwert schwankt, deshalb unmittelbar vorher ablesen. Gleicht er zufällig dem Ausgangswert, schreibt der STM nichts, und die `eep`-Zeile entfällt — dann ist der Fall nicht aussagekräftig, so protokollieren. Ob der Rückweg am STM auch **gespeichert** ist, zeigt erst Phase 9 nach dem STM-Reset (12.1). **Abweisung (A57):** Seit dem STM-Release mit A57 weist „Messwert übernehmen“ einen Rohwert ab, der **nicht unter dem Maximum** liegt. Dann steht im Mitschnitt `ldr: minimum abgewiesen (<roh> >= max <max>)`, es folgt **kein** `eep`, und `numvar[idx=17]` bleibt unverändert. Bei den üblichen Grenzen 0/4095 tritt das nicht ein; erscheint die Zeile trotzdem, zuerst die Grenzen im Rohabzug nachsehen. Die Abweisung gezielt prüfen: siehe **S25b**. **Rückmeldung (B55):** Seit dem STM-Release S2 aus `specs/paket-2026-10-13/` sendet der STM nach der Wertmeldung `RPCR 00 r` an den ESP — `r` = `0` übernommen, `1` abgewiesen, `2` Messung fehlgeschlagen. Soll hier `RPCR 00 0`, auch wenn der Rohwert zufällig gleich dem Ausgangswert ist. **Antwort der API (B55, ESP-Release E2):** Seit E2 wartet `ldr_min_set` bis 2 s auf diese Rückmeldung und meldet das Ergebnis des STM, statt sofort `{"ok":true}` zu geben (`BEFUNDE.md`, L441). Soll `{"ok":true,"value":<n>}`, `n` gleich dem neuen `numvar[idx=17]`. Weitere Antworten: `{"ok":false,"error":10}` abgewiesen (A57, siehe S25b), Kennung `7` Messung fehlgeschlagen, Kennung `11` keine Antwort binnen 2 s — dann steht im Mitschnitt und in `/api/stm32_log` `- rpc keine Antwort #n idx=00`. In S25 sind 10, 7 und 11 ein Befund, 11 zusätzlich ein Alarm der Logwache. **Instrument:** `curl -s "http://$DEVICE_HOST/api/ldr_min_set"` oder der Antwortrumpf im Netzprotokoll des Browsers. **Meldung in der Oberfläche (B55, Release EP):** Seit EP zeigt der Knopf „Aktuellen Wert als Minimum setzen“ im Erfolgsfall „LDR-Minimum gespeichert“, bei Kennung 10 „LDR-Minimum nicht übernommen: Der Messwert liegt nicht unter dem Maximum.“, bei 7 „Messung fehlgeschlagen. Versuch es noch einmal.“ und bei 11 „Keine Rückmeldung der Uhr — ob der Wert übernommen wurde, ist offen.“ Beleg sind Meldung **und** Antwortrumpf zusammen; eine Erfolgsmeldung neben einem `ok:false` im Netzprotokoll ist ein Befund. Vor EP gab es diese Texte nicht, dort trägt der Rumpf allein. `n` kann ausnahmsweise noch der alte Wert sein, wenn der ESP die Wertzeile abgewiesen hat und der STM sie erst nach `RPCR` nachsendet (Review E2.7, N1); dann gilt der Rohabzug. **Nach einer Kennung 11 nicht sofort erneut drücken:** Eine verspätete Rückmeldung wird dem nächsten Aufruf desselben Index zugerechnet (L442) — mindestens einige Sekunden warten, `numvar[idx=17]` im Rohabzug nachsehen, dann erst wiederholen. Die Zeile `RPCR 00 r` selbst spiegelt auch das ESP-Release E2 nicht in den Mitschnitt; ihr Fehlen ist kein Befund |
| **S25b** | **Abweisung einer Inversion** (A57). Per API Auto-Helligkeit aus; **über die Oberfläche (AKL.4) Auto-Helligkeit an** — die PWA gibt die Knöpfe nur bei eingeschalteter Automatik frei (`canStoreBounds: autoBrightness`, `BEFUNDE.md`, L465), also S24 an und danach zurück, Rohwert `numvar[idx=16]` zwischen 2 und 4093. Maximum über S27b kurz auf `1`, dann `ldr_min_set`; danach Maximum zurück. Gegenstück: Minimum über S27a kurz auf `4094`, dann `ldr_max_set`; danach Minimum zurück | S | Im Mitschnitt `ldr: minimum abgewiesen (<roh> >= max 1)` bzw. `ldr: maximum abgewiesen (<roh> <= min 4094)`, **ohne** `eep a=225` bzw. `eep a=227` dazwischen; `numvar[idx=17]` bzw. `[idx=18]` unverändert. Die `eep`-Zeilen der beiden Zahlen-Setter davor und danach sind Soll. Seit dem STM-Release S2 (B55) folgt der `ldr:`-Zeile `RPCR 00 1` bzw. `RPCR 01 1`; der ESP wertet sie aus, spiegelt sie aber nicht in den Mitschnitt. **Seit dem ESP-Release E2 ist die API-Antwort der Beleg:** `ldr_min_set` bzw. `ldr_max_set` antwortet hier mit `{"ok":false,"error":10}` (abgewiesen), per curl oder im Netzprotokoll abgelesen. **Befund:** `{"ok":true,…}` neben einer `ldr: … abgewiesen`-Zeile (die Abweisung wird als Erfolg gemeldet, der Stand vor B55), Kennung 10 **ohne** `ldr:`-Zeile, oder Kennung 11 (keine Rückmeldung, siehe S25). **In der Oberfläche (Release EP, AKL.4)** wird der Fall über die Knöpfe gefahren, die Zahl über S27a/S27b (dafür gibt es kein Bedienelement): Maximum per `ldr_max_value_set?value=1`, dann „Aktuellen Wert als Minimum setzen“ ⇒ „LDR-Minimum nicht übernommen: Der Messwert liegt nicht unter dem Maximum.“ und im Mitschnitt `ldr: minimum abgewiesen`; Maximum zurück auf 4095. Spiegelbildlich Minimum auf 4094, „Aktuellen Wert als Maximum setzen“ ⇒ „LDR-Maximum nicht übernommen: Der Messwert liegt nicht über dem Minimum.“ und `ldr: maximum abgewiesen`; Minimum zurück auf 0. Danach bei 0/4095 „Aktuellen Wert als Minimum setzen“ ⇒ „LDR-Minimum gespeichert“ (das ist S25), Minimum per Zahl zurück auf 0. **Endstand 0/4095**, lesend im Rohabzug nachgewiesen. Erscheint bei Kennung 10 „gespeichert“ oder gar keine Meldung, ist das ein Befund. Die Zahlen stehen hier für die üblichen Grenzen 0/4095; stehen andere, gelten die notierten Ausgangswerte | Ausgangswerte von `numvar[idx=17]` und `[idx=18]` vorher notieren; Rückweg über S27a/S27b, Gegenprobe am Rohwert. **Der Zahlen-Setter selbst weist keine Inversion ab** (A59, siehe S27) — genau deshalb lässt sich der Prüfzustand überhaupt herstellen, und genau deshalb ist der Rückweg Pflicht: Bleibt Maximum 1 stehen, rechnet die Automatik nicht mehr. Am Gerät belegt am 12.10.2026 (`BEFUNDE.md`, L404) und im Testdurchlauf vom 13.10.2026 (L412) |
| **S26** | Aktuellen Messwert als LDR-Maximum übernehmen (`ldr_max_set`) | S | `numvar[idx=18].value` (`LDR_MAX_VALUE`) entsprechend; im Logring und im Mitschnitt folgt `eep a=227 n=2` | wie S25, Rückweg über **S27b** `ldr_max_value_set?value=<Ausgangswert>` mit `eep a=227 n=2`. **Abweisung (A57)** spiegelbildlich: ein Rohwert, der **nicht über dem Minimum** liegt, ergibt `ldr: maximum abgewiesen (<roh> <= min <min>)`, kein `eep`, `numvar[idx=18]` unverändert . **Rückmeldung (B55)** wie S25, mit Index `01`: `RPCR 01 0` bei Übernahme, `RPCR 01 1` bei Abweisung. **Antwort der API (ESP-Release E2)** wie S25: Soll `{"ok":true,"value":<n>}` mit `n` gleich dem neuen `numvar[idx=18]`; Kennung 10, 7 oder 11 ist hier ein Befund, bei 11 mit `- rpc keine Antwort #n idx=01`. Nach einer 11 nicht sofort erneut drücken (L442). **Meldung in der Oberfläche (Release EP)** wie S25 mit „LDR-Maximum gespeichert“ bzw. „LDR-Maximum nicht übernommen: Der Messwert liegt nicht über dem Minimum.“; 7 und 11 mit denselben Texten wie dort |
| **S27** | LDR-Grenzen numerisch setzen: Minimum (**S27a**, `ldr_min_value_set`) und Maximum (**S27b**, `ldr_max_value_set`), je 0..4095 | S | `numvar[idx=17].value` bzw. `numvar[idx=18].value` tragen genau den gesendeten Wert, **und** im Logring (`/api/stm32_log`, Präfix `LOG `) sowie im Mitschnitt steht danach `eep a=225 n=2` (S27a) bzw. `eep a=227 n=2` (S27b) | **Kein Bedienelement in der Oberfläche.** Die beiden Endpunkte sind ausschliesslich über den Backup-Import erreichbar — Prüfung deshalb per direktem Aufruf. **Die `eep`-Zeile ist der STM-Beleg:** `settings_xml` zeigt nur die ESP-Kopie (5b), die Zeile dagegen schreibt der STM, wenn er den Wert ins EEPROM übernimmt. Sie kommt nur, wenn sich der Wert ändert. Fehlt sie bei geändertem Wert, lautet das Ergebnis „ESP gespeichert, STM nicht angewandt“, nicht „bestanden“. Bis A59 kam sie nie, weil der STM beide Setter verwarf (L364). Minimum ≥ Maximum weist der STM bewusst nicht ab — der Backup-Import setzt die beiden nacheinander. Zugleich der Rückweg für S25/S26 |
| **S28** | LDR-Grenze `4096` setzen | S | `error=2`, Variable unverändert, **keine** `eep a=225`- oder `eep a=227`-Zeile | Früher angenommen (L67/L68) |

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
| **S40** | Datumsformat setzen (`date_ticker_format_set?value=`, 5 Zeichen) | S | `strvar[idx=11].value` (`DATE_TICKER_FORMAT`) trägt das Format | Leerer Wert ⇒ `error=1`. Seit C55 gilt eine Regel (Ent-10): nur `D M Y` (auch klein, jede Art höchstens einmal), `.` `-` `/` und Leerzeichen, kein Leerzeichen am Anfang oder Ende, mindestens ein Platzhalter. Hier stand bis zum 11.10.2026 `%d.%m` als Gegenprobe — das ist nach dieser Regel **ungültig**. Gegenprobe am Display: Der Datumsticker zeigt das Datum im gesetzten Format. Abgewiesene und gültige Werte: **K13**, **K14** (7.1) |
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
| **S47** | Farbanimation wählen (`color_animation_mode_set?value=`) und wieder auf `0` (aus) | S | `numvar[idx=15].value` (`COLOR_ANIMATION_MODE`) trägt den gesendeten Modus. **Nach dem Zurücksetzen auf `0` steht `dspcolor[idx=0]` wieder genau auf dem Ausgangswert** — die Animation darf ihre Momentanfarbe nicht in die gespeicherte Farbe schreiben (A23). Während der Animation zeigt `dspcolor[idx=0]` die Momentanfarbe; das ist Soll. Nach dem Zurücksetzen mindestens 60 s warten und erneut lesen: Der Wert bleibt stehen | **Ausgangswert von `dspcolor[idx=0]` vorher aus dem Rohabzug notieren, wie bei S51/S53** — er ist das Soll, nicht nur der Rückweg. Seit dem STM-Release mit A23 ist der Schritt folgenlos rücknehmbar; am Gerät belegt im Testdurchlauf vom 13.10.2026 (`0/0/0/63` nach über 60 s, `BEFUNDE.md` L412) und über einen STM-Reset hinweg (L407: die Farbe kommt aus dem EEPROM). **Weicht der Wert nach dem Zurücksetzen ab, ist das ein Rückfall von A23** und kein Rauschen: im Bericht mit Ausgangs- und Endwert, danach über S34 auf den Ausgangswert zurück und am Rohwert gegenprüfen. Vor A23 ging die Farbe verloren (L150, 04.10.2026: `0/0/0/63` → `0/0/31/0`, einmalig beim Moduswechsel) |
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
| **S61** | Markierungsfarbe setzen (`marker_color_set?...`) | S | `dspcolor[idx=2].red/green/blue/white` | Weisskanal nur bei RGBW, siehe S35. Ob der Weisswert einen STM-Neustart übersteht, zeigt dieser Fall nicht — er liest die ESP-Kopie. Das prüft **K21** (7.1, A81) |
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
| **S97** | Update-Host setzen (`update_host_set?value=`, 63 Zeichen) | S | `strvar[idx=9].value` (`UPDATE_HOST`) trägt den Wert | **Danach sofort gegen `DEVICE_UPDATE_HOST` aus `tools/device.conf` prüfen.** Ein falscher Host holt beim nächsten Update fremde Firmware, ohne Fehlermeldung (L42). Seit dem Release E4 folgt dem Speichern genau ein `update_status?refresh=1`: „Server-Verfügbarkeit wurde neu geprüft“ nur, wenn er gesendet und beantwortet wurde, sonst „… konnte nicht neu geprüft werden“; rund 9 s später lädt die PWA `update_status` einmal nach (4b) |
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

**„Mit Meldung“ heisst seit B48 zweierlei** (`BEFUNDE.md`, L448): das Banner wie bisher,
und **direkt unter dem Feld eine Fehlerzeile** mit demselben Text; der Fokus springt ins
Feld. Das gilt für Datumsformat, Koordinaten, zu lange Texte (auch den WLAN-Schlüssel),
Zahlenfelder, Uhrzeiten und „Alle speichern“ bei den Timern. Fehlt die Zeile bei einem
abgewiesenen Wert, ist das ein Befund — ausser an den Reglern der Dimmkurven, die bewusst
keine bekommen. Gezielt prüft das **K32** (7.1).

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
- **Zeitfeld leeren und speichern** — Timer, Ambilight-Timer, DFPlayer-Alarm und
  „Alle speichern“: **K1** bis **K4** (7.1).
- **Zwei Felder ändern, eines speichern, Modul wechseln**, und der Gegenfall: **K5**,
  **K6** (7.1).
- Ohne offene Änderung das Modul wechseln — erscheint die Warnung trotzdem?
  **Hier lag die Erwartung, dass sie auch ohne offene Änderung erscheint**
  (Massnahme 4). Am 04.10.2026 (PWA 1.4.88) nicht beobachtet:
  vier Modulwechsel ohne Bearbeitung, null Dialoge; mit offener Änderung erschien er
  korrekt. **Der Pfad „nach normalem Speichern" ist am 05.10.2026 nachgeholt worden**
  (05.10.2026, PWA 1.4.90, L302): erst speichern, dann ohne weitere Eingabe das Modul wechseln —
  kein Dialog. Der Punkt bleibt im Plan, damit ein Rückfall auffällt.

### 7.1 Bedienfälle mit echten Ereignissen — Kennung K

Die acht Klassen oben hängen an einem **Feld**. Die Fälle hier hängen an einem
**Ablauf** — leeren, speichern, warten, wechseln — und fallen deshalb durch jede
Feldliste. Sie stammen aus dem Durchlauf vom 10.10.2026 (`BEFUNDE.md`, L340, L365 bis
L369) und aus dem Review P.6 zum Paket 2026-10-10. Die Korrekturen B44 bis B47, C55/C44
und die Anzeige aus A2 sind in der Vorschau und am Prüfstand nachgewiesen, **am Gerät
noch nicht**. Der Durchlauf misst sie, er setzt sie nicht voraus (DIR-003). **K18** und
**K19** kamen mit dem Paket 2026-10-11 dazu (A53, C35); beide sind am Gerät belegt
(`BEFUNDE.md`, L404, L410) und werden seither je Durchlauf erneut gemessen. **K20** bis
**K24** kamen mit dem STM-Release S2 aus `specs/paket-2026-10-13/` dazu (A56, A81, A80, A82,
A70). Am Prüfstand sind sie belegt (`BEFUNDE.md`, L435), **am Gerät noch nicht** — der
Durchlauf misst sie, er setzt sie nicht voraus. **K25** bis **K28** wurden für das
ESP-Release E1 aus demselben Paket geschrieben (C6, C6v, C60, C46, C19); E1 ist entfallen
(L439). **K29** bis **K31** kamen mit dem ESP-Release E2 dazu (C8, A79, C59); am Prüfstand
belegt (`BEFUNDE.md`, L441), am Gerät noch nicht. **K30** hat seit dem ESP-Release E3 (C65)
zwei Haltegrenzen statt einer (Ent-23, Entscheid des Nutzers); Grenzen und Messweise stehen in
der Zeile. B55 (ESP) steht nicht hier, sondern in
S25, S25b und S26 (6.3). **K32** und **K33** kamen mit dem PWA-Release P2 aus demselben
Paket dazu (B48, B8): K32 ist am Prüfstand b48 belegt, K33 statisch über Stufe S18
(`BEFUNDE.md`, L448) — am Gerät beide noch nicht. Seit B48 gehört zu jeder Abweisung in
K1 bis K4, K13 und K15 zusätzlich die Fehlerzeile unter dem Feld; fehlt sie dort, ist das
ein Befund unter K32.

**K25 und K26 sind entfallen (13.10.2026, L439).** Die Herkunftsprüfung über `Sec-Fetch-*`
wird nicht ausgeliefert: Browser senden die Kopfzeilen über `http://` an die Uhr gar nicht,
am Gerät belegt. Der Schutz der zerstörerischen Aufrufe ist zurückgestellt und bekommt eine
eigene Spec (`BEFUNDE.md`, Abschnitt „Entscheidung: Schutz der zerstörerischen Aufrufe“);
bis dahin sind die Uhr und ihre Einstellungen gegen Aufrufe fremder Seiten im LAN **nicht
geschützt**. Die beiden Zeilen bleiben in der Tabelle, damit die Kennungen stabil bleiben.
Der Prüfstand c6h liegt nur noch auf dem Zweig `sicherung/e1-herkunft`.

**K27 und K28 bleiben**, ihr Träger ist jetzt das ESP-Release E2 (C46, C19); das Review E2.7
hat beide im Release bestätigt (`BEFUNDE.md`, L441). Belegt sind sie an den Prüfständen c46
und c19 (L437), am Gerät noch nicht.

**Instrumente**, auf die die Tabelle verweist:

| Instrument | Was es heisst |
|---|---|
| **Browser** | Die PWA vom Gerät in einem echten Browser, Eingaben und Klicks als **echte Ereignisse** über das DevTools-Protokoll (siehe oben, `isTrusted`). Dazu das **Netzprotokoll** derselben Sitzung: „kein Aufruf“ heisst, kein Request auf den genannten Endpunkt — nicht „keine Erfolgsmeldung“ |
| **API lesend** | Rohabzug bzw. `/api/settings_xml`, `/api/stm32_log`, `/api/eeprom_settings` — nur lesend (DIR-008). **Nicht** `/api/network_secrets` (Phase 2) |
| **Mitschnitt** | `./tools/watch-log.sh` läuft während des ganzen Durchlaufs (DIR-013) |
| **Vorschau** | `tools/preview/server.py` ohne Gerät (Skill `/pwa-vorschau`), mit denselben echten Ereignissen |
| **Reset** | `maintenance_reset_stm32` (S114, Klasse R). Nur mit Freigabe des Nutzers im selben Gespräch und nach den Regeln aus V3 (5b): höchstens einmal je Durchlauf, am Ende. K20 und K21 teilen sich diesen einen Reset; K24 braucht einen eigenen in der Nacht und ist deshalb ein eigener Lauf. **Ausgelöst wird er in der PWA, nicht per curl** — so belegt er zugleich, dass der Bedienweg aus der Oberfläche trägt |
| **curl mit Kopfzeile** | Ein einzelner Aufruf vom Rechner mit gesetzter Kopfzeile `Sec-Fetch-Site` bzw. `Sec-Fetch-Dest`, so wie ein Browser ihn von einer fremden Seite schicken würde: `curl -s -w '\n%{http_code}\n' -H 'Sec-Fetch-Site: cross-site' "http://$DEVICE_HOST/<pfad>"`. Antwortcode und Rumpf ins Protokoll. **Entfallen mit K25 (L439)** — im Durchlauf an keinem zerstörenden Weg, auch nicht mit Kopfzeile. Die Gegenprobe mit `Sec-Fetch-Dest: image` fährt allein der Smoketest |
| **UDP** | Ein einzelnes Paket an Port 2424 der Uhr, etwa `printf 'gTs' \| nc -u -w1 "$DEVICE_HOST" 2424`. **Kein Hook sieht diesen Weg** (Kapitel 11) — nur die hier genannten Kommandos senden |

**Rückstellung.** Wo das Soll „kein Aufruf“ lautet, genügt im Erfolgsfall: Seite neu
laden. Geht der Aufruf **doch** hinaus, ist das ein Rückfall, und dann zählt, was
**vorher** notiert wurde — deshalb nennt die Spalte, was vor dem Fall aus dem Rohabzug
festzuhalten ist. Jeder berührte Index gehört in die Aufräumliste (Kapitel 2). Die
schreibenden Fälle (K13b, K14, K15b, K16, K18, K20 bis K24) laufen in einer gerahmten Setter-Phase nach
V1 und V1b (5b). K25 und K26 sind entfallen (L439),
K27 ist lesend, K28 läuft nicht am Gerät. K29 nutzt den Wetterabruf aus S19 als Auslöser,
K30 und K31 sind lesend und hängen an Ereignissen, die ohnehin stattfinden (ESP-Start,
stündliche Netzzeit). K32 schreibt nichts aufs Gerät, solange die Abweisung greift, und
gehört trotzdem in eine gerahmte Setter-Phase — greift sie nicht, geht ein Setter hinaus.
K33 ändert nur die Sprachwahl im Prüfbrowser. **Kein Fall dieser Reihe ruft einen der gefährlichen Endpunkte aus
`CLAUDE.md` (R5) auf**, und keiner drückt einen Knopf, der flasht, eine Datei lädt oder
Zugangsdaten schreibt.

| Kennung | Ablauf | Instrument | Soll | Rückstellung |
|---|---|---|---|---|
| **K1** | **Display-Timer:** Zeitfeld einer Zeile leeren (Tasten), Zeile speichern. **K1a** mit Haken „aktiv“, **K1b** ohne — leer wird nach Ent-9 **immer** abgewiesen | Browser, API lesend | Meldung „Keine gültige Uhrzeit bei <Zeile>: Das Zeitfeld ist leer oder unvollständig. Es wurde nichts gespeichert …“; **kein** `timer_set` im Netzprotokoll; `nighttime[idx=N]` unverändert | Seite neu laden. Vorher den ganzen Slot `nighttime[idx=N]` notieren: Am 10.10.2026 stand danach ein aktiver Timer auf 00:00, der die Uhr jede Nacht um Mitternacht schaltet (L365). Geht ein Aufruf hinaus, sofort über S82 zurück |
| **K2** | **Ambilight-Timer:** wie K1 (**K2a** aktiv, **K2b** inaktiv) | Browser, API lesend | wie K1, **kein** `ambilight_timer_set`; `ambinighttime[idx=N]` unverändert | wie K1, Rückweg über S83 |
| **K3** | **DFPlayer-Alarm:** Zeitfeld leeren, aktiv setzen, speichern | Browser, API lesend | wie K1, **kein** `dfplayer_alarm_set`; `alarmtime[idx=N]` unverändert | wie K1, Rückweg über S95 — ein stehengebliebener Alarm weckt den Nutzer. Ohne DFPlayer ist das Panel ausgeblendet: „nicht prüfbar“ (6.8) |
| **K4** | **„Alle Timer speichern“:** zwei oder mehr Zeilen ändern, in **einer** davon das Zeitfeld leeren. **K4b** dasselbe mit „Alle Ambilight-Timer speichern“ | Browser, API lesend | Meldung „Keine gültige Uhrzeit bei <Zeilen>. Es wurde kein Timer gespeichert …“; **kein einziger** `timer_set` bzw. `ambilight_timer_set`, auch nicht für die gültigen Zeilen — sonst stünde das Gerät halb gespeichert da; alle berührten Slots unverändert | Seite neu laden. Alle berührten Slots vorher notieren, Rückweg wie K1 bzw. K2 |
| **K5** | **Zwei Felder ändern, eines speichern, Modul wechseln.** Im Modul `display` Tickerverzögerung (A) und Datumsformat (B, ein gültiger Wert wie `M/Y`) ändern, nur „Ticker-Verzögerung speichern“, mindestens 15 s warten (ein Takt der Selbstaktualisierung), dann das Modul wechseln | Browser, API lesend | Feld B behält die Eingabe, auch nach dem Takt; der Modulwechsel fragt: „Es gibt ungespeicherte Änderungen. Modul trotzdem wechseln? …“. Am Gerät: `numvar[idx=31].value` neu, `strvar[idx=11].value` **unverändert**. Am 10.10.2026 kam kein Dialog, B ging still verloren (L366) | Rückfrage bestätigen — das verwirft B. Danach A über S41 auf den Ausgangswert |
| **K6** | **Gegenfall (L33):** **ein** Feld ändern, speichern, Modul wechseln | Browser | **Keine** Rückfrage; die Selbstaktualisierung läuft wieder — im Netzprotokoll setzen die periodischen Abrufe wieder ein. Bliebe die Markierung stehen, ruhte sie für den Rest der Sitzung | Das Feld über seinen S-Fall auf den Ausgangswert zurück |
| **K7** | **Dimmkurven-Vorlage wählen, nicht speichern.** Im Modul `display` unter „Kurvenvorgabe“ eine andere Vorlage wählen, mindestens 15 s warten, dann das Modul wechseln. **K7b** dasselbe im Modul `ambilight` | Browser, API lesend | Die sechzehn Regler bleiben auf der Vorlage, auch nach dem Takt — sie springen **nicht** auf den Gerätewert zurück; der Modulwechsel fragt nach; **kein** `display_dim_level_set` bzw. `ambilight_dim_level_set`; `num8array[var=0]` bzw. `[var=1]` unverändert | **Verwerfen:** Rückfrage bestätigen. Danach im Rohabzug die Kurve unverändert. Vorher die Ausgangskurve vollständig notieren (wie S43) — geht doch ein Aufruf hinaus, ist sie sonst nicht rücknehmbar |
| **K8** | **„Voreinstellung anwenden und speichern“ bei „Individuell“** (im Markup als „Preset anwenden und speichern“). Steht die Kurve auf keiner Vorlage, zeigt die Liste schon „Individuell“; sonst zuerst einen Regler von Hand verschieben. Dann den Knopf drücken | Browser, API lesend | Abgewiesen: „„Individuell“ ist keine Voreinstellung. … Es wurde nichts gespeichert.“, der Fokus steht auf der Liste; **kein** `display_dim_level_set`; `num8array[var=0]` unverändert. Früher schrieb der Knopf in diesem Fall still „Linear“ und meldete Erfolg | Verwerfen (Rückfrage beim Modulwechsel bestätigen oder Seite neu laden). Ausgangskurve vorher notieren wie bei K7 |
| **K9** | **Update-Listen:** im Modul `maintenance` in „STM32-Firmware vom Server“ oder „Layout-Tabelle vom Server“ einen **anderen** Eintrag als den vorgewählten wählen, mindestens 15 s warten. **Rein lesend — keinen Knopf daneben drücken:** „STM32 flashen“ ist S111 (Klasse R), das Laden der Tabelle S100 | Browser | Die Wahl bleibt nach der Selbstaktualisierung stehen. Die Selbstaktualisierung läuft dabei weiter (die Liste ist von der Bearbeitungsmarkierung ausgenommen), keine Pausenmeldung; **kein** `remote_stm32_flash`, **kein** `update_download_table` im Netzprotokoll. Hat die Liste nur einen Eintrag: „nicht prüfbar“ | Seite neu laden; gesendet wurde nichts |
| **K10** | **Pausenmeldung:** ein Feld ändern (Tasten), nicht speichern, mindestens 45 s warten (drei Takte). **K10b:** das Feld auf den Ausgangswert zurücktippen, dann erneut ändern und wieder 45 s warten | Browser | „Automatische Aktualisierung pausiert, bis ungespeicherte Änderungen gespeichert sind“ erscheint **genau einmal je Bearbeitung** — in K10 einmal, in K10b noch einmal. Gezählt über die Textwechsel von `#status-banner` (Beobachter über das DevTools-Protokoll), nicht über einen Screenshot. Steht gerade eine Fehlermeldung im Banner, verdrängt die Pausenmeldung sie nicht und folgt im nächsten Takt — auch dann nur einmal | Seite neu laden |
| **K11** | **AP-SSID ohne Wartungsbesuch:** frische Sitzung, Modul `network` **direkt** öffnen, die Wartung vorher **nicht** besuchen. Die Reihenfolge ist der Prüfgegenstand — wer zuerst die Wartung öffnet, sieht den Fehler nie | Browser, API lesend | Nach dem ersten Ladezyklus steht in „AP SSID“ der Name aus `eeprom_settings` (`ap_ssid`), nicht leer. Am 10.10.2026 blieb das Feld nach 1, 5, 10 und 20 s leer (L367). **Nur `ap_ssid` ins Protokoll.** Bis zum Release EP lieferte `/api/eeprom_settings` den WLAN-Schlüssel im Klartext; seit EP (C6b) sind `key` und `ap_key` leer. Ein Schlüssel gehört trotzdem in kein Protokoll, keine Datei und darf auch in der Konsole nicht erscheinen — steht dort doch einer, ist das ein Befund (Phase 2) | Keine, nichts geschrieben. **Nicht speichern** — AP-Zugangsdaten sind Phase 8 |
| **K12** | **Kartenmodal:** „Ort auf Karte wählen“ öffnen, mit echtem Mausklick auf den **Hintergrund** schliessen. **K12b:** Ziehen in der Karte beginnen und auf dem Hintergrund loslassen. **K12c:** Escape | Browser (Mausereignisse über das DevTools-Protokoll, kein `element.click()`) | K12: Modal zu, `document.activeElement` ist der Knopf „Ort auf Karte wählen“, **nicht** `body` (L368). K12b: Modal bleibt offen. K12c: Modal zu, Fokus wie K12 | Keine. **Nicht „In Wetter übernehmen“** — das schreibt (6.3) |
| **K13** | **Datumsformat, abgewiesen:** im Feld „Datumsformat im Ticker“ je Wert eintippen und „Datumsformat speichern“: `yyyyy`, `<scri`, `⏰`, `  ab` (zwei Leerzeichen vorn), `DD`, `D ` (Leerzeichen hinten), `.`, `D,M`. **K13b**, direkt am ESP: `date_ticker_format_set?value=yyyyy` und `?value=%3Cscri` | Browser, API lesend; K13b direkter Aufruf | K13: Meldung „Ungültiges Datumsformat. …“, **kein** `date_ticker_format_set`, `strvar[idx=11].value` unverändert. K13b: `error=2`, Wert unverändert — der ESP prüft selbst (C55), nicht nur die Oberfläche | Ausgangswert vorher notieren. Wird ein Wert doch angenommen, sofort über S40 zurück |
| **K14** | **Datumsformat, gültig:** `M/Y`, danach `D.M.` speichern | Browser, API lesend | Beide angenommen, `strvar[idx=11].value` trägt jeweils genau den Wert | Ausgangswert zurückschreiben, Gegenprobe am Rohwert. **Ist der Ausgangswert nach der neuen Regel selbst ungültig**, lässt er sich nicht zurückschreiben — dann den Fall **nicht** fahren, sondern melden |
| **K15** | **Koordinaten, abgewiesen:** im Modul `climate` je Wert speichern, das andere Feld auf seinem Ausgangswert. Längengrad: `180.1`, `<"&`, `.5`, `+5`, `1e2`. Breitengrad: `90.00001`, `91`, `180`. **K15b**, direkt am ESP: `weather_coordinates_set?lon=%3C%22%26&lat=<Ausgangswert>` | Browser, API lesend; K15b direkter Aufruf | K15: Meldung „Ungültiger Längengrad. …“ bzw. „Ungültiger Breitengrad. …“, **kein** `weather_coordinates_set`, `strvar[idx=7]` und `[idx=8]` unverändert. K15b: Abweisung, beide unverändert | Ausgangswerte vorher notieren. Wird ein Wert doch angenommen, sofort zurück — erst die Koordinaten, dann den Ort (6.3) |
| **K16** | **Koordinaten, gültig:** Längengrad `8,5400` mit Breitengrad `47.3769`, danach Längengrad `-180` mit Breitengrad `-90` | Browser, API lesend | Beide Paare angenommen; `strvar[idx=7].value` trägt nach dem ersten Paar `8.5400` — **mit Punkt**, das Komma wird vor der Prüfung ersetzt | Ausgangswerte zurückschreiben, Gegenprobe am Rohwert. Dazwischen keinen Wetterabruf (S18, S19) auslösen. Standen keine Koordinaten, ist der Rückweg das Leeren beider Felder — das geht nur, solange ein Ort steht (S17) |
| **K17** | **DS18xx-Anzeige** in der Klima-Übersicht, drei Zustände (A2). **Am Gerät nur der gültige Fall** — der Sensor wird nicht abgezogen (Ent-14, Paket 2026-10-10). **K17b** und **K17c** nur in der Vorschau | Gerät: Browser, API lesend, Mitschnitt. K17b/K17c: Vorschau | **Gültig (Gerät):** „DS18xx“ zeigt den Messwert, „DS18xx online“ zeigt „online“; `numvar[idx=20].value` (`DS18XX_IS_UP`) = 1, `numvar[idx=23].value` ≠ 255; im Mitschnitt `DS18xxx temperature:` ohne `ungueltig`, keine Zeile `ds18xx verloren`. **K17b, nicht gefunden** (`[idx=20]` = 0): „–“ und „nicht gefunden“. **K17c, ungültig** (`[idx=20]` = 1, `[idx=23]` = 255): „Messwert ungültig“ und „online“ | Keine, rein lesend. Die Vorschauwerte setzt die Tabelle `NUM` in `tools/preview/server.py` (gehört dem Lead) oder ein Abfangen der `settings_xml`-Antwort im DevTools-Protokoll |
| **K18** | **Spiel mit Setter** (A53). Am Tag, Display an. Ausgangswert von `numvar[idx=6].value` (`DISPLAY_BRIGHTNESS`) notieren. Tetris starten (UDP `gTs`), **während das Spiel läuft** `display_brightness_set?value=<anderer Wert, 0..15>` senden (wie S29), einige Sekunden warten, Spiel beenden (UDP `gTq`) | UDP, API lesend, Mitschnitt | Im Mitschnitt `(CMC GTs)`, der Setter als `CMC N06…`, `(CMC GTq)`, danach **`spiel: N Zeilen verworfen`** mit N ≥ 1 und kurz darauf **`info: configuration sent`** — der STM hat die verworfenen Zeilen gezählt und einen Vollabgleich nachgeholt. Danach sind **STM und ESP-Kopie konsistent**: `numvar[idx=6]` steht wieder auf dem Ausgangswert, weil der STM den Setter während des Spiels verworfen hat und der Abgleich die ESP-Kopie aus dem STM neu aufbaut. Am 12.10.2026 belegt: N = 1, `numvar[6]` wieder 1 (L404). **Befund** ist: keine `spiel:`-Zeile, kein `configuration sent` danach, oder `numvar[idx=6]` auf dem gesendeten Wert, während das Display die alte Helligkeit zeigt — dann weichen ESP und STM still voneinander ab, genau der Zustand vor A53 (L316) | **`gTq` ist Teil des Falls, nicht Nachklapp:** Eine Inaktivitätsgrenze gibt es bewusst nicht (Ent-4), ohne `gTq` bleibt die Uhr im Spiel. Den Befehl vorher bereitlegen. Danach `numvar[idx=6]` lesen; steht er nicht auf dem Ausgangswert, über S29 `display_brightness_set?value=<Ausgangswert>` zurück und gegenprüfen. Seit A16 bedient das Spiel den Watchdog, ein Reset ist nicht mehr zu erwarten — trotzdem kurz halten, unter einer Minute. In der Aufräumliste: `numvar[idx=6]` |
| **K19** | **Abrufzahl im Hauptmodul** (C35). Frische Sitzung, Modul `main`, **15 Minuten** stehen lassen — kein Modulwechsel, kein Neuladen, Fenster im Vordergrund (Phase 6, V3: ein verborgener Tab pausiert die Abrufe und verfälscht die Zählung) | Browser, Mitschnitt | Im Mitschnitt die Aufrufe von `update_status` und `update_table_files` zwischen Öffnen und Ende zählen: **je Endpunkt höchstens 4** — einer beim Öffnen, danach höchstens alle 5 Minuten. Am 13.10.2026 gemessen: je 3 in 15 Minuten, statt vorher 60 (L410). Die übrigen Abrufe (`settings_xml`, `display_power`, `ambilight_power`) laufen unverändert alle 15 s; das ist Soll. **Gegenprobe:** In den Modulen Wartung und Anzeige holt die PWA `update_status` weiterhin bei jedem Lauf, beim Wechsel dorthin sofort — dort ist die hohe Zahl Soll und kein Rückfall | Keine, rein lesend. **Während der Messung kein zweiter Browser und kein `check-pwa.sh`:** Der Mitschnitt zeigt jeden Browser nur als `[Chrome]`, Abrufe zweier Sitzungen sind darin nicht zu trennen. Ein zweiter Browser verdoppelt die Zahl und täuscht einen Rückfall vor; `check-pwa.sh` lädt jedes Modul einmal neu und erzeugt dabei selbst Abrufe |
| **K20** | **Ambilight-Power über den STM-Neustart** (A56). Am Tag, **ausserhalb** eines Aus-Fensters (Bestimmung wie in „V3 im Nachtzustand“). Vorlauf zu V3: Ausgangswert von `numvar[idx=30].value` (`DISPLAY_AMBILIGHT_POWER`) notieren. Ambilight über S2 ausschalten (`ambilight_power_set?value=off`); steht es schon auf `0`, erst `on`, dann `off`, damit sicher geschrieben wird. Dann V3 (S114) | API, API lesend, Mitschnitt, Reset | Vor dem Reset: `eep a=1979 n=2` im Mitschnitt, `numvar[idx=30]` = 0, `/api/power_status` meldet `"ambilight_power":"off"`. **Nach dem Reset und nach `info: configuration sent`:** `numvar[idx=30]` = 0 und `/api/power_status` weiterhin `off` — der STM hat den Wert aus dem EEPROM geholt. **Befund** ist „an“ nach dem Reset: der Stand vor A56 (L330). Fehlte vor dem Reset die `eep`-Zeile, ist „an“ dagegen Soll (ohne gültige Marke gilt „an“) — dann ist der Fall nicht aussagekräftig, so protokollieren | Ambilight über S2 auf den Ausgangswert, Gegenprobe am Rohwert; bei einem Wechsel wieder `eep a=1979 n=2`. An dieser Uhr ohne sichtbare Wirkung (L11), Beleg ist der Rohwert. In der Aufräumliste: `numvar[idx=30]` |
| **K21** | **Marker-Weisswert über den STM-Neustart** (A81). Nur bei RGBW (`numvar[idx=0].value` = 1, siehe S35), sonst „nicht prüfbar“. Vorlauf zu V3, zusammen mit K20: `dspcolor[idx=1]` (Ambilight) und `dspcolor[idx=2]` (Marker) vollständig notieren. Über S61 den Marker-Weisswert auf einen Wert setzen, der **sich vom Ambilight-Weisswert unterscheidet**, die übrigen drei Anteile auf ihrem Ausgangswert mitsenden. Dann V3 (S114) | API, API lesend, Mitschnitt, Reset | Vor dem Reset: `dspcolor[idx=2].white` trägt den gesetzten Wert, im Mitschnitt eine `eep`-Zeile. **Nach dem Reset und nach `info: configuration sent`:** `dspcolor[idx=2].white` trägt **weiterhin den gesetzten Wert**, `dspcolor[idx=1].white` ist unverändert. **Befund** ist `dspcolor[idx=2].white` gleich `dspcolor[idx=1].white`: der STM liest wieder vom Platz des Ambilights, der Stand vor A81 (L427). Sind beide Weisswerte zufällig gleich, sagt der Fall nichts — deshalb der unterschiedliche Wert | Marker-Farbe über S61 auf den notierten Ausgangswert, Gegenprobe am Rohwert. **Massgebend ist der Wert vor dem Vorlauf**, nicht der nach dem Reset. In der Aufräumliste: `dspcolor[idx=2]` |
| **K22** | **Icon bei ausgeschaltetem Display** (A80). Am Tag. Ein Overlay vom Typ Icon (`type=1`) oder Wetter-Icon (`type=4`) muss stehen; sonst eines über S70 bis S72 anlegen, Icon aus `/api/overlay_icons`, kurze `duration`. Typ Icon ist vorzuziehen, er braucht keinen Wetterabruf. **K22a:** Display über S1 aus, `overlay_display?idx=<N>` (S79), mindestens `duration` + 5 s warten, Display über S1 an. **K22b:** Display an, `overlay_display?idx=<N>`, **während das Icon läuft** Display über S1 aus, einige Sekunden warten, an. **Nicht über `weather_get_now`:** Der Wetterabruf löst nur einen Text-Ticker aus, kein Icon (L435) — er erzeugt die Zeile nie | API, API lesend, Mitschnitt, Display | **K22a:** im Mitschnitt `overlay icon <name>` bzw. `overlay weather icon`, danach `icon: verworfen, display aus`; **kein** `icon_freeze: ENTER`. Nach dem Einschalten zeigt die Uhr sofort die Zeit, das Icon erscheint **nicht** nachträglich. **K22b:** das Icon endet mit dem Aus, kein `icon_freeze: ENTER`, nach dem Einschalten kein Rest des Icons; ein erneutes `overlay_display` zeigt es vollständig von vorn. In beiden Fällen `diag` lückenlos. **Befund:** `icon_freeze: ENTER` (Rückfall, die Logwache schlägt Alarm), ein Icon nach dem Einschalten, oder ein vorgemerkter Ticker-Restore, der erst nach dem Icon kommt | **Display an ist Teil des Falls**, nicht Nachklapp — die Uhr ist während des Falls dunkel, also kurz halten. Im Aus-Fenster eines Display-Timers nicht fahren; dort wäre die Rückstellung „wieder aus“ (S1). Ein eigens angelegtes Overlay über S80 löschen, Zuordnung über den Inhalt (S80). In der Aufräumliste: `numvar[idx=3]`, `numvar[idx=30]` (S1 schaltet das Ambilight mit), das angelegte Overlay |
| **K23** | **DCF77-Merker folgt dem Ambilight** (A82). Am Tag. Ausgangswerte von `numvar[idx=3]` und `numvar[idx=30]` notieren. Display über S1 aus; Ambilight über S2 an, dann wieder aus; Display über S1 an | API, API lesend, Mitschnitt | Jede Zeile `set_display_power: flags=… display=d ambi=a dcf77=f` erfüllt `f` = 1 genau dann, wenn `d` = 0 und `a` = 0 — nach dem Ausschalten `display=0 ambi=0 dcf77=1`, nach dem Einschalten `dcf77=0`. Die beiden Ambilight-Schritte dazwischen geben im Release-Build **keine** solche Zeile, nur `eep a=1979 n=2` (A56). **Was sich am Gerät nicht zeigen lässt:** dass der Merker beim Schalten **nur** des Ambilights mitgeht — das belegt der Prüfstand a82 (L435). Mit angeschlossenem und empfangendem DCF77-Modul darf, solange das Ambilight an ist, keine `dcf77:`-Zeile erscheinen; ohne Empfänger entfällt das | Display über S1 an, Ambilight über S2 auf den Ausgangswert — S1 schaltet das Ambilight mit ein, ein Ausgangswert `0` braucht danach S2 `off`. Gegenprobe am Rohwert. In der Aufräumliste: `numvar[idx=3]`, `numvar[idx=30]` |
| **K24** | **Fremder Ticker während des vorgemerkten Aus** (A70 (e)). **Eigener Nachtlauf, nicht Teil des Tagesdurchlaufs**: nur mit Freigabe des STM-Resets im Aus-Fenster, nachts, Mac wach über `caffeinate -i -s -t <Sekunden>` bis nach dem letzten Schritt, Logwache läuft mit und `./tools/watch-log.sh --pruefe` am Ende (DIR-020, DIR-013). Vorbedingung: Aus-Fenster laut Timertabelle, `numvar[idx=3]` = 0. STM-Reset (S114); **sobald im Mitschnitt `info: configuration sent` steht** — direkt danach startet der IP-Lauftext — `weather_get_now` (S18). Ersatzweise `ticker_set` (S38); das schreibt den Tickertext und braucht dessen Rückweg | Reset, API, Mitschnitt, Display | Im Mitschnitt nach dem Wetterabruf `set_display_power: new=0 sync_ambi=1 …`, `set_display_power: … display=0 ambi=0 dcf77=1` und `night: start aus (hh:mm)` — **vor** dem Ende des IP-Lauftexts, nicht erst nach einem Wetterlauftext. Am Display: Der IP-Lauftext bricht ab, **kein** Wetterlauftext. **Befund:** ein Wetterlauftext am Display, oder `night: start aus` erst nach dessen Ende — der Stand vor A70. **Nicht aussagekräftig**, so protokollieren: `night: start aus` stand schon vor dem Wetterabruf (der IP-Lauftext war vorbei), oder der Wetterabruf scheiterte (`- weather …` nicht ok). **Am Gerät nicht herstellbar** und deshalb nur am Prüfstand a70 belegt: (c) der Restore kommt nie — dann schaltet die Frist (Laufzeit des IP-Lauftexts plus 10 s), (d) das erste `TIME` bei ungültiger RTC während des IP-Lauftexts, und L399 (reiner DCF77-Betrieb). Läuft der Start doch einmal ohne gültige RTC (kein `rtc is online`), ist Soll: der IP-Lauftext läuft vollständig, das Aus kommt danach | **Wieder aus ist Soll**, nichts zurückzustellen; der Ein-Timer am Morgen schaltet ein (`Found Timer: on` im Mitschnitt, lesend nachprüfen). Nach dem Reset wie V3: Variablensatz und Update-Quelle (S97/S98). Bei `ticker_set` den Tickertext über S38 auf den notierten Ausgangswert. Im Tagesdurchlauf als „ausgelassen: eigener Nachtlauf“ melden |
| **K25** | **Entfallen 13.10.2026 (L439) — Schutz der zerstörerischen Aufrufe zurückgestellt.** Die Herkunftsprüfung über `Sec-Fetch-*` wird nicht ausgeliefert: Browser senden die Kopfzeilen über `http://` an die Uhr gar nicht (am Gerät belegt). Nicht fahren, im Bericht als „entfallen (L439)“ führen. Die Zeile bleibt stehen, damit die Kennungen stabil bleiben; der Rest hält den geplanten Ablauf fest. **Herkunftsprüfung, Gegenprobe ohne Schaden** (C6, C60). Nur an Wegen, deren fälschlicher Durchgang lediglich den STM neu startet (Einleitung zu 7.1). **Am Ende des Durchlaufs**, nach der letzten Setter-Phase und unmittelbar **vor** V3: Ein Versagen wäre ein STM-Neustart und machte jede laufende Messreihe ungültig. Je ein Aufruf: **K25a** `/api/maintenance_reset_stm32` mit `Sec-Fetch-Site: cross-site`. **K25b** dasselbe mit `Sec-Fetch-Site: same-site` — auch eine Nachbarseite im selben Netz gilt als fremd. **K25c** Legacy `/?action=rststm32` mit `Sec-Fetch-Site: cross-site`. **K25d** dasselbe mit `Sec-Fetch-Dest: image`. Den API-Weg mit `Sec-Fetch-Dest: image` fährt der Smoketest bei jedem Lauf; `/update?action=reset` und `/flash_stm32_local?action=reset` nur der Prüfstand c6h | curl mit Kopfzeile, Mitschnitt, API lesend, Display | K25a, K25b: HTTP 403, Rumpf `{"ok":false,"error":"cross-site"}`. K25c, K25d: HTTP 403 mit der kurzen Seite „403 - abgewiesen“. Im Mitschnitt und in `/api/stm32_log` je **genau eine** Zeile: `- abgewiesen: maintenance_reset_stm32 als Sec-Fetch-Site=cross-site` bzw. `=same-site`, `- abgewiesen: legacy rststm32 als Sec-Fetch-Site=cross-site`, `- abgewiesen: legacy rststm32 als Sec-Fetch-Dest=image`. **Und es passiert nichts:** keine Startsequenz an der Uhr, im Mitschnitt keine ESP-Startzeilen und kein `info: configuration sent`, die `diag`-Folge läuft lückenlos weiter, `update_cache_hits` fällt nicht auf 0 — ein STM-Reset reisst den ESP mit (S114, L183). **Befund:** HTTP 200, oder 403 und trotzdem ein Neustart. Dann hält die Prüfung an diesem Weg nicht, und an den sechs Wegen, die hier bewusst nicht laufen, ist sie ebenso fraglich: Durchlauf anhalten und melden, **nicht** an einem anderen Weg nachprüfen | **Freigabe nötig**, obwohl im Erfolgsfall nichts geschieht — ein Versagen startet den STM neu. Bei der Anfrage für V3 mitnennen. Im Erfolgsfall keine Rückstellung. Startet der STM doch neu, gilt alles aus S114: Variablensatz und Update-Quelle (S97/S98) prüfen, laufende Messreihen ungültig. Ein weiterer Reset für V3 braucht dann eine eigene Freigabe |
| **K26** | **Entfallen 13.10.2026 (L439) — Schutz der zerstörerischen Aufrufe zurückgestellt.** Die Herkunftsprüfung über `Sec-Fetch-*` wird nicht ausgeliefert: Browser senden die Kopfzeilen über `http://` an die Uhr gar nicht (am Gerät belegt). Nicht fahren, im Bericht als „entfallen (L439)“ führen. Die Zeile bleibt stehen, damit die Kennungen stabil bleiben; der Rest hält den geplanten Ablauf fest. **Der Bedienweg bleibt offen** (C6, C60): Die Herkunftsprüfung darf die Uhr nicht von ihren eigenen Seiten aussperren. **K26a:** Den Reset aus V3 in der PWA auslösen — Modul `maintenance`, „STM32 zurücksetzen“ mit echtem Klick, Rückfrage bestätigen. Das ist **derselbe** Reset wie V3, kein zusätzlicher. Das Netzprotokoll vorher auf „beibehalten“ stellen: Die PWA lädt sich nach dem Reset selbst neu. **K26b:** Auf der Legacy-Seite den Knopf „Restart STM32“ (`/?action=rststm32`). Das ist ein **zweiter** Reset, deshalb nur in einem eigenen Lauf mit eigener Freigabe, ausserhalb eines Aus-Fensters; im Tagesdurchlauf als „ausgelassen: zweiter Reset“ melden | Browser (Netzprotokoll mit Anfragekopfzeilen), Mitschnitt, Reset | **K26a:** Im Netzprotokoll `maintenance_reset_stm32` mit `Sec-Fetch-Site: same-origin` und Antwort 200, die PWA meldet „STM32-Reset wurde ausgelöst. Warte auf Abschluss...“; im Mitschnitt die Startsequenz und **keine** Zeile `- abgewiesen:`. **K26b:** keine 403-Seite, Startsequenz wie K26a, keine `- abgewiesen:`-Zeile. Dass Werkzeuge ohne Kopfzeile weiter durchkommen (`flash-stm.sh`), ist bei E1 belegt (L437) und hier nicht eigens zu fahren. **Befund:** 403 oder eine `- abgewiesen:`-Zeile auf einen Klick aus PWA oder Legacy — dann ist die Wartung von der Uhr aus gesperrt. Ein Aufruf per curl ohne Kopfzeile kommt in diesem Fall weiter durch; er ist bis zur Korrektur der Rückweg für einen STM-Reset | Wie V3: Variablensatz und Update-Quelle (S97/S98) prüfen; im Aus-Fenster gelten Soll und Rückstellung aus „V3 im Nachtzustand“ |
| **K27** | **Query über 255 Zeichen** (C46), rein lesend. **K27a:** `/api/device_ready?x=` mit 300 Zeichen Fülltext, also eine Query von 302 Zeichen: `curl -s "http://$DEVICE_HOST/api/device_ready?x=$(head -c 300 /dev/zero \| tr '\0' a)"`. **K27b**, Gegenprobe an der Grenze: 253 Zeichen Fülltext, Query genau 255. **Immer mit `=`** — ein Parameter ohne `=` ist der Absturzfall aus Kapitel 11. Ein lesender Endpunkt genügt: `http_api_query_too_long()` in `http.cpp` weist **jeden** Pfad unter `/api/` ab, bevor ein Handler läuft | curl, Mitschnitt | **K27a:** HTTP 200 mit `{"ok":false,"error":9,"detail":"query too long"}`, im Mitschnitt `- abgewiesen: query 302 Zeichen`. **K27b:** die gewohnte Antwort von `device_ready`, keine `- abgewiesen:`-Zeile. **Befund:** in K27a die gewohnte Antwort (dann wird still gekürzt, der Stand vor C46), in K27b Kennung 9 (die Grenze sitzt falsch) | Keine, nichts geschrieben |
| **K28** | **Legacy-Update-Quelle leer oder überlang** (C19, L311). **Am Gerät nicht gefahren**, auch nicht mit sofortigem Rückweg: Versagt die Prüfung, steht die Update-Quelle leer, und der ESP fällt auf den Server des Ursprungsprojekts zurück (L42) — das nächste Update holte fremde Firmware ohne Meldung | — | Belegt am Prüfstand c19, an beiden Stellen (`/fs` und `/update`), leer und über 63 Byte (L437). Im Bericht als „nur Prüfstand“ führen. Am Gerät bleibt die lesende Nachprüfung, dass Update-Host und -Pfad nach dem Durchlauf mit `tools/device.conf` übereinstimmen (S97/S98) | Keine |
| **K29** | **Lange STM-Logzeile in `/api/stm32_log`** (C8, L430). Vorher im Mitschnitt die Länge einer bekannten langen Zeile messen — erste Wahl `info: weather forecast = …` (AKK.2), sie kommt nach dem Vorhersageabruf. Auslöser S19 (`weather_get_forecast`). Danach `/api/stm32_log` lesen und die Einträge ab dieser Zeile zusammensetzen: `~` am Ende des ersten und am Anfang jedes Folgeeintrags weglassen. Ist die Vorhersagezeile 120 Zeichen oder kürzer, eine andere Zeile über 120 Zeichen aus dem Mitschnitt nehmen (etwa `set_display_power` oder die `diag`-Zeile, falls sie so lang wird), sonst den Fall als „nicht aussagekräftig“ führen | API lesend, Mitschnitt, Auslöser S19 | Bei einer Zeile von 121 bis 255 Zeichen: zwei oder drei Einträge im Ring, der erste mit 119 Zeichen und `~` am Ende, die folgenden mit `~` am Anfang; zusammengesetzt **zeichengleich** mit dem Mitschnitt. Die PWA zeigt die Teile als eigene Zeilen (Modul `maintenance`, Logbuch). **Befund:** ein einziger Eintrag mit `~` am Ende und der Rest fehlt (der Stand vor C8), oder zusammengesetzt ungleich. Über 255 Zeichen kappt die Brücke; dann endet der letzte Teil mit `~`, benannte Grenze | Keine eigene Freigabe über die des Durchlaufs hinaus; S19 ist kein lesender Aufruf (L54). Nichts zurückzustellen, der Wetterticker läuft danach wieder an. Nicht zwischen den beiden Paaren von K16 auslösen |
| **K30** | **ESP-Start ohne Empfangsüberlauf, Heap danach und während des Durchlaufs** (A79, L421; Haltegrenze seit dem ESP-Release E3 nach Ent-23, Entscheid des Nutzers). Gemessen wird an den ESP-Starts, die ohnehin stattfinden: dem OTA vor dem Durchlauf (falls in dieser Sitzung eingespielt) und dem STM-Reset aus V3, der den ESP mit neu startet (L183). **Kein eigener Reset.** Je Start die ersten 2 Minuten im Mitschnitt auf `- ovr #` durchsehen und die Heap-Zeilen ablesen. `free`, `max` und `low=` der Heap-Zeile notieren: nach dem Start, nach dem Vorhersageabruf (S19 bzw. K29), nach dem Laden der PWA (Phase 1b) und **vor jeder Setter-Phase den Ruhewert** | Mitschnitt, API lesend | Je Start in den ersten 2 Minuten **kein** `- ovr #`, jede Heap-Zeile `ovr=0`. `free` liegt rund 768 Byte unter dem Stand vor E2 — der grössere Puffer, erwartet. **Anhalten — zwei Grenzen, jede für sich:** (1) **Ruhewert** `max` unter **5'120** Byte: Ein OTA braucht 4 KB am Stück plus Reserve, und der Vorhersageabruf belegt 5'120 am Stück; darunter ist `oom` zu erwarten und das nächste Update gefährdet. (2) **Tiefstwert** unter **1'072** Byte: Darunter bringt das Gerät keinen vollen Sendepuffer mehr unter, jede Antwort scheitert (abgeleitet, nicht gemessen). Greift eine der beiden: keine weiteren Setter-Phasen, melden. **Dazwischen kein Anhaltegrund**, sondern ins Protokoll: Uhrzeit, Wert, Pfad (`heap_low_path`), Last, Messweise. Ausdrücklich **kein** Anhaltegrund ist ein Tiefstwert unter 5 KB — nach L449 läge schon jede einzelne PWA-Ladung darunter, und der Durchlauf käme nie zu den Settern, ohne dass das Gerät gefährdet wäre. **Befund:** `- ovr #` mit kleinem `t` nach einem Start, `ovr` über 0. Den Vergleich gegen die Heap-Zeilen vor E2 über 30 Zeilen (AKM.2) führt der Gerätenachweis G4, nicht dieser Durchlauf. **Messweise (L449, C65):** Der **Ruhewert** ist `max` der Heap-Zeile nach mindestens 150 s ohne eigene Anfrage des Testers — kein Klick, kein curl. Ent-23 verlangt mindestens 60 s nach der letzten Aktion; die 150 s aus dem Gerätenachweis G7 erfüllen beides. Eine offen gelassene PWA darf dabei weiter abfragen: So liegt das Gerät da, wenn aus der PWA ein OTA gestartet wird. Der **Tiefstwert** kommt aus den Messpunkten **während** der Antworten, nicht aus einer Stichprobe: entweder `low=` der Heap-Zeile (setzt sich je Zeile selbst zurück) oder `heap_low_block` über `device_ready` — zu Beginn eines Fensters `device_ready?heap_low_reset=1`, am Ende **ohne** Parameter lesen; Lesen ohne Parameter setzt nichts zurück (5b, „Die Messfelder in `/api/device_ready`“). **Werte verschiedener Messweisen nicht direkt vergleichen:** `max` der Heap-Zeile ist eine Stichprobe je Minute, `max_free_block` aus `device_ready` eine Stichprobe beim Abruf mit dessen eigener Rückwirkung (L449: 1'448 bei Abfragen alle 0,2 s), der Tiefstwert ein Minimum über die Messpunkte. Ein Tief der einen gegen ein Tief der anderen belegt weder einen Rückschritt noch einen Fortschritt. Feines Abfragen von `device_ready` ist seit E3 nicht mehr nötig, um das Tief zu sehen, und belastet das Gerät selbst; wer es trotzdem tut, nennt es im Protokoll. **Seit dem ESP-Release E4 zeigt die Momentaufnahme, was beim Tief los war** (I-4, 5b „Die Momentaufnahme beim Tief“): Bei jedem Halt an K30 und bei jedem protokollierten Tief dazwischen gehören `heap_low_accepts_pending`, `heap_low_backlog`, `heap_low_pcb_states`, `heap_low_tw`, `heap_low_unacked` und `heap_low_req_path` neben `heap_low_block` und `heap_low_path` ins Protokoll, gelesen ohne Parameter. Die beiden Grenzen bleiben wie nach Ent-23. **Nicht verwechseln mit dem Tor aus Ent-31:** Im Gerätenachweis G8 nach dem Einspielen von E4 ist die erste Ladung die Messung und K30 erst danach das Tor vor jeder weiteren — das gilt für G8, nicht für diesen Durchlauf. **Seit dem ESP-Release R1 (Paket 2026-10-15):** Zu jedem Ruhewert und jedem Tiefstwert gehört der Vermerk **„Flash-Schreiben seit Start / beim Start“** — `ja` mit Beleg (`- update cache <datei> geschrieben`, `- update cache tmp geloescht <datei>`, App-Upload), `nein` oder `unbekannt` (das automatische Formatieren beim Einhängen des LittleFS gibt keine Zeile) — und ob im Startfenster ein Wetter-Overlay lief (eigene Verbindung, die `hc` nicht erfasst). Grund: Nach L468 zerstückelt nur ein Schreiben **beim Start** den Heap (C71); ein Ruhewert ohne diesen Vermerk lässt sich nicht einordnen. Die Startmessung zu C71 (`specs/paket-2026-10-15/design.md` §5.5) zieht dafür die Zeilen `- update cache vorher`, `heap` und `nach` heran (5b), und „erholt sich nicht“ gilt erst an einer Heap-Zeile mindestens 130 s nach `n` mit `tw` = 0. Bei jedem Halt an K30 gehören neben den I-4-Feldern auch die Verbindungen jetzt (`tcp_now_*`, `http_accept_dead`, gelesen ohne Parameter) in den Bericht | Freigabe nur über V3 bzw. das OTA, kein zusätzlicher Eingriff. Keine Rückstellung; `heap_low_reset=1` setzt nur Messwerte zurück. Ist V3 nicht freigegeben und kein OTA gelaufen, den Start-Teil als „ausgelassen: kein ESP-Start“ führen — die Haltegrenze gilt trotzdem für den ganzen Durchlauf |
| **K31** | **Stündliche Netzzeit mit Bindung an die Anfrage** (C59, L406). Rein lesend über die ganze Dauer des Durchlaufs: Jede volle Stunde, in die der Durchlauf fällt, im Mitschnitt nachsehen. Den eingestellten Zeitserver aus dem Rohabzug notieren | Mitschnitt | Je Stunde `cmd: set time to …` mit richtiger Zeit, und im ganzen Durchlauf **keine** Zeile `- ntp verworfen #n ungefragt` oder `- ntp verworfen #n originate`. Nach dem Reset aus V3 die Folge `time: sprung, bestaetigung angefordert` und rund 15 s später `cmd: set time to …` (5b) — sie braucht zwei angenommene Antworten auf zwei Anfragen. **Befund:** jede `ungefragt`- oder `originate`-Zeile im Normalbetrieb — Uhrzeit, `#n` und Zeitserver in den Bericht. Fehlt danach auch `cmd: set time to …` zur vollen Stunde, schreibt der Server das Originate-Feld vermutlich nicht zurück; dann bekommt die Uhr mit diesem Server keine Netzzeit mehr, sofort melden. Die zweistündige Messung (AKT.2) führt G4 | Keine, nichts geschrieben. Läuft der Durchlauf über keine volle Stunde, als „nicht aussagekräftig“ führen |
| **K32** | **Fehlerzeile am Feld** (B48). Je Fall einen ungültigen Wert per Tasten eintippen und den zugehörigen Knopf drücken. **K32a:** Modul `display`, „Datumsformat im Ticker“ `yyyyy`, „Datumsformat speichern“. **K32b:** Modul `network`, „WLAN-Passwort“ mit 40-mal `ä` (80 Byte, die Uhr nimmt höchstens 63), „Als WLAN-Client verbinden“. Die SSID-Auswahl **nicht** anfassen; steht dort keine SSID, kommt nur „Bitte zuerst ein WLAN auswählen …“ ohne Zeile — dann den Fall als „nicht aussagekräftig“ führen, **nicht** eine SSID eintragen. **K32c:** Modul `timers`, Zeitfeld eines Display-Timers leeren, „Speichern“ dieser Zeile (wie K1). **K32d:** Modul `display`, „Ticker-Verzögerung“ `256`, „Ticker-Verzögerung speichern“. Danach an K32a: **K32e** denselben Wert noch einmal speichern; **K32f** ein Zeichen ins Feld tippen; **K32g** den Fehler wiederherstellen, das Modul wechseln und im Dialog verwerfen | Browser, API lesend | Je Fall: Banner mit der Meldung wie bisher, Fokus im Feld, und **direkt unter dem Feld** ein `span.field-error` mit demselben Text, `aria-hidden="true"` und **ohne** `role="alert"` (bewusst, sonst kommt die Ansage dreifach); am Feld `aria-invalid="true"` und `aria-describedby` mit der Kennung der Zeile, vorhandene Kennungen bleiben erhalten. K32b nennt 80 Byte und höchstens 63. **Kein** `date_ticker_format_set`, `network_client_set`, `timer_set` bzw. `ticker_deceleration_set` im Netzprotokoll, Rohwerte unverändert. K32e: weiterhin **genau eine** Zeile. K32f und K32g: Zeile weg, `aria-invalid` weg, die Kennung aus `aria-describedby` entfernt. **Befund:** Zeile fehlt, steht doppelt, bleibt nach der Eingabe stehen, trägt `role="alert"`, oder ein Setter geht hinaus. Am Gerät **nicht** gefahren und am Prüfstand b48 belegt: das Räumen bei Kartenübernahme, Backup-Import und Typwechsel eines Overlays, und dass die Regler der Dimmkurven keine Zeile bekommen | Vorher notieren: `strvar[idx=11].value` (Datumsformat), `nighttime[idx=N]` der gewählten Zeile, `numvar[idx=31].value` (Tickerverzögerung). Im Erfolgsfall Seite neu laden. Geht doch ein Setter hinaus, über S40, S82 bzw. S41 auf den notierten Wert zurück. **Geht in K32b `network_client_set` hinaus, sofort anhalten und den Nutzer verständigen** — die Uhr weist über 63 Byte zwar selbst ab (C38), aber ein `{"ok":true}` hiesse, dass sie sich neu verbindet. Nicht selbst reparieren |
| **K33** | **Englische Oberfläche, Stichprobe** (B8, AKD.3). Oben unter „Sprache“ auf „English“ stellen und der Reihe nach öffnen: **Overview** (`main`, deutsch „Hauptseite“), **Display**, **Network**, **Maintenance** samt Bereich „Backup“ — das sind die fünf aus AKD.3 —, dazu **System** und **Overlays**, weil dort die B8-Stellen liegen. Jedes Modul ganz durchscrollen, Aufklappbares aufklappen. **Nichts drücken, was speichert, lädt oder installiert** — insbesondere in der Wartung keinen App-Ordner wählen und nicht „Install app files directly“ | Browser | Kein deutscher Text in diesen Modulen. Gezielt hinsehen: **System**, Hardware-Liste „Board“, „Processor“, „Oscillator“, „Frequency“, „Hardware“ (vorher „Platine“, „Prozessor“, „Oszillator“, „Taktfrequenz“), ein unbekannter Wert als „unknown“; die **Gerätezeit-Vorschau** unter Datum und Uhrzeit mit englischem Wochentag („Monday, …“), ohne Gerätezeit „Device time is currently unavailable.“ **Hauptseite**, „Last start“ notfalls „not available“; die Beschriftungen von Wortraster und Minutenecken (nur im DOM: `aria-label` „WordClock preview“ und „Minute corners“). **Wartung**, Liste der lokalen App-Dateien: ohne gewählten Ordner steht jede Datei auf „missing“, nicht „fehlt“. **Overlays**, Icon-Auswahl: „No icons found“ erscheint nur, wenn die Uhr keine Icons meldet. **Hinweis Layout-Tabelle** oben auf der Seite: „Please install the layout table … on LittleFS.“ erscheint nur, wenn keine Tabelle installiert ist. Die beiden letzten am Gerät meist nicht herstellbar; dann als „nicht herstellbar, statisch über S18 belegt“ führen. **Kein Befund** sind Gerätedaten und Nutzertexte (Ort, Tickertext, SSID, Dateinamen), Logzeilen von STM und ESP und Eigennamen wie WordClock, Ambilight, DFPlayer, LittleFS. **Befund:** jedes andere deutsche Wort — Modul, Stelle und Wortlaut ins Protokoll | Sprache zurück auf „Deutsch“. Die Wahl liegt nur im `localStorage` des Prüfbrowsers, nicht auf der Uhr. **Niemals eine Datei auf der Uhr löschen, um den Hinweis zur Layout-Tabelle oder „No icons found“ herzustellen** |

**Bekannte Lücke, kein Testfall: L438.** *Geschrieben für die Herkunftsprüfung aus E1, die
nicht ausgeliefert wird (L439); seither ist der ganze Schutz offen, nicht nur dieser Teil.*
Die Herkunftsprüfung sass im GET-Zweig. Die
Upload-Wege über POST — `local_esp_update`, `local_stm32_upload`, `/flash_stm32_local`,
die `fs_upload_*`-Endpunkte (sie löschen vorher die alten Tabellendateien),
`app_file_upload`, `/fs-icon` — lesen keine `Sec-Fetch-*`-Kopfzeile, und
`local_esp_restart` startet den ESP per GET ohne Prüfung (`BEFUNDE.md`, L438, Massnahme
C61). Eine Gegenprobe hiesse, Firmware oder Dateien hochzuladen bzw. den ESP neu zu
starten; der Durchlauf fährt sie nicht. Fehlt dort eine Abweisung, ist das **kein neuer
Befund**, sondern L438.

**Die Werte in K13 bis K16 sind eine Auswahl.** Die Regel selbst steht in einer
einzigen Fall-Tabelle, `tools/checks/auszug/esp/t17/regeln.tsv`, für ESP und PWA
gemeinsam; vollständig prüft sie der Prüfstand `tools/checks/auszug/pwa/regeln/`
gegen die PWA und `tools/checks/auszug/esp/t17/` gegen den ESP. Die Auswahl hier deckt
je Regel die Fälle ab, die am Gerät schon einmal aufgefallen sind (L340, L369), dazu
die Grenzen und je zwei gültige Werte. Ändert sich die Regel, beginnt die Änderung in
der Tabelle, und diese Auswahl ist nachzuziehen. Die Wege über die Kartenauswahl und
den Backup-Import prüft der Prüfstand; im Durchlauf werden sie nicht eigens gefahren.

**Auch hier ist die Abdeckung zu melden**, wie in 6.0c:

```
Phase 4, Bedienfälle: von K1 bis K33 X gefahren, Y ausgelassen.
Je ausgelassenem Fall: Kennung und Grund.
```

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
| **B2b** | Export, wenn `network_secrets` scheitert (C6b, seit dem Release EP): 404 eines ESP ohne den Endpunkt, Netzfehler, oder leerer Schlüssel trotz `key_set` | **Abbruch** mit „Die WLAN-Schlüssel liessen sich nicht von der Uhr lesen. Der Export wurde abgebrochen, …“, **keine** Datei. Am Gerät mit EP nicht herstellbar, ohne den ESP zu tauschen — geprüft in der Vorschau (`PREVIEW_OHNE_SECRETS=1`) und am Prüfstand `tools/checks/auszug/pwa/ep/`. Im Durchlauf als „nur Vorschau/Prüfstand“ führen |
| **B3** | Export, wenn die WLAN-SSID leer ist | Hinweis „Netzwerkteil übersprungen", Export läuft weiter |
| **B4** | Zwei Exporte hintereinander ohne Änderung | byteweise identisch bis auf den Zeitstempel |
| **B4b** | Schlüssel im Export (C6b, AKS.4) | Meldet `eeprom_settings` `key_set: true`, ist `wifi_key` in der Datei **nicht leer**; bei `ap_key_set: true` ebenso `ap_key`. Ins Protokoll nur „nicht leer“ und höchstens eine Prüfsumme, nie der Klartext. Ein leerer `wifi_key` bei `key_set: true` ist ein Befund — genau den Fall soll der Abbruch aus B2b verhindern |
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
   `eeprom_settings` geprüft. Weicht sie ab, wird nicht importiert. Seit dem Release EP
   (C6b) sind die Schlüssel dort leer: Verglichen werden SSID, AP-SSID und
   `boot_as_ap`, dazu dass `wifi_key` bzw. `ap_key` in der Datei genau dann nicht leer
   ist, wenn `key_set` bzw. `ap_key_set` `true` meldet (B4b). Den Schlüssel selbst
   gleicht der Durchlauf nicht mit dem Gerät ab — dafür müsste er `network_secrets`
   abrufen (Phase 2).
3. **B6 (eine geänderte Einstellung) ändert nie etwas aus `network`.** Dafür wird eine
   Anzeigeeinstellung genommen.

**B7 ist davon die Ausnahme und zugleich die Absicherung:** Er importiert eine
Sicherung mit **leeren** Netzwerkfeldern. Genau dieser Fall war früher gefährlich —
leer hiess „löschen". Seit ESP 3.2.5 heisst leer „nicht ändern". B7 weist nach, dass
der Schutz greift, und ist damit die Voraussetzung dafür, B5 und B12 überhaupt zu
wagen. **Reihenfolge ist deshalb nicht beliebig: B7 vor B5.**

**Vor B5, B7 und B12:** AP-Zugangsdaten griffbereit, serieller Zugang erreichbar,
Legacy-Oberfläche in einem zweiten Tab offen.

**Seit dem Release EP schreibt der Import die WLAN-Felder einzeln** (C46): ein
`eeprom_settings_set` je nicht leerem Feld, in der Reihenfolge `ap_ssid`, `ap_key`,
`key`, `ssid`; leere Felder werden gar nicht gesendet. Vorher gingen alle in einem
Aufruf, und lange, prozentkodierte Schlüssel überstiegen die 255 Zeichen (L431). **Alle
Aufrufe ausser dem letzten tragen `boot_as_ap=on`, nur der letzte den Wert aus der
Sicherung** (Nutzerentscheid 14.10.2026). Bricht der Import mittendrin ab und die Uhr
startet danach neu, kommt sie als Zugangspunkt hoch statt unerreichbar. Die Meldung sagt
dann sinngemäss: **nicht neu starten, Import wiederholen** — ein Wiederholen
vervollständigt den Satz, und der letzte Aufruf setzt das Flag zurück.

Was daraus für den Durchlauf folgt:

- Im Netzprotokoll sind `boot_as_ap=on` in den vorderen Aufrufen **Soll**, kein Befund.
  Der Befund ist ein **letzter** Aufruf mit `on`, obwohl die Sicherung `false` trägt —
  dann startet die Uhr beim nächsten Mal als Zugangspunkt. Nach dem Import muss
  `eeprom_settings` `boot_as_ap: false` melden (bei der eigenen Sicherung des Nutzers).
- Kein Aufruf über 255 Zeichen und keine Zeile `- abgewiesen: query …` im Mitschnitt
  (5b). Eine Meldung „Die Anfrage ist zu lang …“ während des Imports ist ein Befund.
- **Bricht der Import ab: die Uhr nicht neu starten**, auch nicht über S114, sondern den
  Import mit derselben Datei wiederholen und danach `boot_as_ap` lesend prüfen. Bis dahin
  ist das Flag möglicherweise gesetzt. Abbruch, Meldung und den letzten gesendeten Aufruf
  (ohne Wert) in den Bericht.
- Der Import bleibt beim Nutzer, wie in `CLAUDE.md` (R5) festgehalten; der `pwa-tester`
  fährt ihn nicht.

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

### 10.1 Fehlerzeile bei offener Tastatur — am iPhone, durch den Nutzer (AKF.2)

Diesen Teil fährst **Du selbst** am iPhone; kein Agent kann ihn ersetzen, denn die
Vorschau zeigt keine Bildschirmtastatur. Die Frage ist eine einzige: **Siehst Du die
Fehlerzeile unter dem Feld, während die Tastatur offen ist?** Gespeichert wird dabei
nichts — alle drei Werte weist die Oberfläche ab, bevor etwas an die Uhr geht. Ohne
Deine Rückmeldung bleibt AKF.2 offen.

**Vorbereitung**

1. Das iPhone im selben WLAN wie die Uhr. Die PWA so öffnen, wie Du sie sonst nutzt
   (Safari oder vom Home-Bildschirm), und einmal neu laden. Oben steht „App-Version …“ —
   sie muss die Version sein, die Dir zum Einspielen genannt wurde. Steht dort eine ältere,
   App schliessen und neu öffnen.
2. Für Fall 3 in der Notizen-App eine Zeile aus **40-mal „ä“** anlegen (zehnmal tippen,
   markieren, kopieren, viermal einfügen) und die ganze Zeile kopieren. Das sind 80 Byte;
   die Uhr nimmt höchstens 63. **Keinen kürzeren Text nehmen** — der Abstand ist die
   Sicherung.
3. Vor Fall 2 im Modul „Timer“ einen Screenshot der Zeile machen, die Du nimmst.

**Die drei Fälle.** Je Fall: Feld antippen, Wert eingeben, den Knopf antippen. Danach
springt der Fokus zurück ins Feld, und die Tastatur geht in der Regel wieder auf. Geht sie
nicht auf, tippst Du das Feld einmal an — **ohne etwas einzugeben**, sonst verschwindet
die Zeile, und das ist richtig so.

| Fall | Wo | Was | Soll: unter dem Feld steht |
|---|---|---|---|
| 1 | Modul „Display“, Feld „Datumsformat im Ticker“ | Inhalt löschen, `yyyyy` tippen, „Datumsformat speichern“ | „Ungültiges Datumsformat. …“ |
| 2 | Modul „Timer“, Zeitfeld eines Display-Timers | Feld antippen, im Zeitwähler von iOS „Löschen“, dann „Speichern“ dieser Zeile | „Keine gültige Uhrzeit bei Slot … “ |
| 3 | Modul „Netzwerk“, Feld „WLAN-Passwort“ | die 80 Byte aus der Notiz einfügen, die SSID-Auswahl **nicht** ändern, „Als WLAN-Client verbinden“ | „Zu lang für WLAN-Passwort: Der Text belegt 80 Byte, die Uhr speichert höchstens 63. …“ |

Bei Fall 2 öffnet iOS statt der Tastatur ein Rad oder eine Auswahl; die Frage ist
dieselbe. Bietet Dein iOS keinen Weg, das Zeitfeld zu leeren, ist der Fall „am iPhone nicht
herstellbar“ — das gehört so in die Rückmeldung, nicht ein anderer Weg.

**Danach** jeweils nichts korrigieren und speichern, sondern die Seite neu laden. Dann
stehen wieder die Werte der Uhr in den Feldern.

**Abbrechen und melden**, wenn statt der Fehlermeldung etwas wie „gespeichert“,
„gestartet“ oder „Verbinde…“ erscheint. Bei Fall 3 heisst das, die Uhr könnte sich neu
verbinden; dann nichts weiter unternehmen. Bei Fall 2 die Zeile mit dem Screenshot
vergleichen.

**Rückmeldung** je Fall eine von drei Antworten, dazu ein Screenshot mit offener Tastatur
und die iOS-Version:

- **sichtbar** — die Zeile ist zu sehen, ohne dass Du scrollen musst
- **erst nach Scrollen** — sie ist da, liegt aber hinter der Tastatur, bis Du schiebst
- **nicht sichtbar** — sie ist auch mit Scrollen nicht zu sehen, oder es gibt keine

Nur „sichtbar“ an allen drei Feldern schliesst AKF.2. „Erst nach Scrollen“ ist ein Befund,
kein Durchfallen der Fehlerzeile selbst; ob er tragbar ist, entscheidest Du.

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
| `maintenance_format_fs` | Löscht die PWA vom Gerät. Danach ist die Oberfläche weg, mit der man sie hochladen würde | **Gar nicht**, auch nicht mit Abweisungs-Kopfzeile. Er weist seit ESP 3.2.4 eingebettete Aufrufe (`Sec-Fetch-Dest`) mit 403 ab; das prüft der Smoketest stellvertretend an `maintenance_reset_stm32`, nicht an `maintenance_format_fs`. **Im Browser wirkt diese Abweisung nicht** (L439): Browser senden die Kopfzeile über `http://` nicht. Ein Schutz gegen Aufrufe fremder Seiten besteht damit nicht; er ist zurückgestellt |
| `eeprom_settings_set` | Schreibt SSID, WLAN-Schlüssel, AP-Zugangsdaten und das Flag „als Zugangspunkt starten" — der Endpunkt nimmt alle in einem Aufruf an, der Backup-Import sendet sie seit dem Release EP einzeln, die vorderen bewusst mit `boot_as_ap=on` (8.3). Ein leeres Feld heisst seit ESP 3.2.5 zwar „nicht ändern", aber ein gesetztes `boot_as_ap=on` genügt, um die Uhr beim nächsten Start aus dem WLAN zu nehmen — derselbe Weg, der das Gerät schon einmal ohne WLAN, ohne AP und ohne Webserver zurückgelassen hätte | Der Endpunkt wird im Durchlauf **ausschliesslich** über den Backup-Import berührt, in B5, B7 und B12, und nur mit der eigenen, unmittelbar vorher erzeugten Sicherung. Ein direkter Aufruf aus Phase 3 heraus findet nicht statt |
| **Tetris** (UDP-Kommando) | Der Hauptloop ist während des Spiels blockiert, und es gibt bewusst keine Inaktivitätsgrenze (Ent-4): Ohne `gTq` bleibt die Uhr im Spiel. Den Watchdog bedient das Spiel seit A16; der früher sichere Reset nach rund 20 s (L106) ist damit weg | **Nur in K18** (7.1): kurz, am Tag, mit bereitgelegtem `gTq`. Sonst nicht starten |
| **Snake** (UDP-Kommando) | Gleichartig gebaut wie Tetris, gleicher Stand bei Watchdog und Zeilenverwerfung (A16, A53) | **Nicht starten.** K18 prüft den gemeinsamen Mechanismus an Tetris; geprüft wird hier nur, dass das Modul vorhanden ist |

**An diesen Endpunkten gibt es keine Gegenprobe mit Kopfzeile**, auch nicht mit einer, die
abgewiesen werden müsste: Hält die Prüfung nicht, ist der Schaden schon geschehen. Das gilt
ebenso für `fs_remove` und die Legacy-Wege `/?action=rsteeprom2`, `/fs?action=format` und
`/fs?action=remove`. Die Herkunftsprüfung aus E1 (C6, C6v, C60) wird nicht ausgeliefert, und
die Abweisung über `Sec-Fetch-Dest` seit ESP 3.2.4 wirkt im Browser nicht (L439) — eine
Gegenprobe belegte nur, was ein Werkzeug sendet. Der Schutz der zerstörerischen Aufrufe ist
zurückgestellt; K25 (7.1) ist entfallen.

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
9. **Mit Freigabe: V3** (5b) — der STM-Reset über S114 nach der letzten Setter-Phase,
   **ausgelöst in der PWA**, danach Schritte 2, 3 und 6 erneut. Die LDR-Grenzen `numvar[idx=17]` und
   `numvar[idx=18]` müssen auf dem Wert **vor** dem Durchlauf stehen (12.1). Danach
   zwingend Variablensatz und Update-Quelle prüfen, wie V3 es verlangt. Im
   Aus-Fenster eines Display-Timers gelten Soll und Rückstellung aus „V3 im Nachtzustand“.
   Die Herkunftsproben K25 und K26 sind entfallen (L439). An diesem Reset startet auch der
   ESP neu: die ersten 2 Minuten nach K30 auswerten, die Zeitfolge nach K31
10. **Die M1-Datei aus Phase 0 löschen**, sobald Schritt 1 und gegebenenfalls Phase 5
    sie nicht mehr brauchen. Sie enthält den WLAN-Schlüssel im Klartext (Kapitel 2b).
    M2 und M3 bleiben liegen: M2 ist verschlüsselt, M3 enthält nur Prüfsummen

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

**Für die STM-seitigen LDR-Grenzen ist dieser blinde Fleck mit A59 abgedeckt**, in zwei
Schritten. Im Lauf belegt die `eep`-Zeile nach S27, dass der STM den Rückweg aus S25/S26
**angewandt** hat (6.3). Am Ende zeigt Schritt 9, ob er ihn auch **gespeichert** hat:
Nach dem STM-Reset baut der ESP seine Kopie aus dem STM neu auf (V3), und Phase 9 muss
die Grenzen auf dem Wert vor dem Durchlauf zeigen. Weichen sie ab, hielt der STM etwas
anderes als die ESP-Kopie — ein Befund, kein erklärter Rest. Eine Einschränkung gehört
in den Bericht: Der Import aus Schritt 1 schreibt die Grenzen ebenfalls, seit A59 auch
am STM. Der Reset belegt deshalb, dass Rückweg und Import **zusammen** am STM halten; den
Rückweg allein belegt die `eep`-Zeile. **Für alle anderen Werte bleibt der blinde Fleck**,
und V3 ist dort der einzige vollständige Abgleich.

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
| **W1** | `tools/snapshot-device.sh` | **da** | M2 und M3 erzeugen, verschlüsselten Abzug wieder öffnen. M3 hält seit dem Release EP nur `key_set`/`ap_key_set` fest (davor eine Prüfsumme), M2 holt die Schlüssel über `network_secrets` und verschlüsselt sie |
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
