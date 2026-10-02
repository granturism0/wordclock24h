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
   zurückgesetzt zu haben.
4. **Sieben Funktionen werden nicht scharf ausgeführt.** Welche und warum: Phase 8.
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

1. `./tools/smoke-device.sh` — 25 Prüfungen, muss **0 Fehler** melden. Schlägt hier
   schon etwas fehl, ist das ein Befund und kein Teststart.
2. `./tools/install-app.sh --check` — Version **und** abgelegte Dateien gegen die
   Weissliste.
3. Rohabzug S2 als **Referenzdatei** ablegen. Gegen sie läuft Phase 9.
4. Festhalten: welche Teilsysteme sind da (RTC, EEPROM, DS18B20, TFT, DFPlayer,
   Ambilight), welche Module sind folglich sichtbar.

---

## 5. Phase 2 — Alles Lesende

**Klasse L. Risikofrei, deshalb zuerst und vollständig.**

Für jedes Modul: Jeder angezeigte Wert wird gegen den Rohwert aus `settings_xml`
beziehungsweise `eeprom_settings` geprüft. Nicht „sieht plausibel aus", sondern
**Zahl gegen Zahl**.

Besonders zu beachten, weil hier schon einmal falsch formatiert wurde:

- **Temperatur**: Der STM-Fehlerwert `255` wurde früher als „127,5 °C" angezeigt.
  Zeigt der Sensor `127.5`, ist das **kein** Messwert, sondern „keine Messung".
- **Zeit**: Gerätezeit gegen die eigene Uhr. Eine eingefrorene Anzeige ist das
  Leitsymptom des behobenen Hängers (L14) — sie wäre ein Rückfall.
- **Versionen**: STM, ESP und App-Version gegen `guardrails.sh` S4.
- **Dateiliste**: Grössen gegen die lokalen `.gz`.
- **`stm32_log`**: Wird das Logbuch überhaupt gefüllt, und bricht die Anzeige bei
  langen Zeilen um?

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

### 6.1 `system`

Gerätezeit setzen (Tag, Monat, Jahr, Stunde, Minute), Zeit vom Netz holen,
Debug-Ansichten umschalten, Logbuch leeren. **Zeit setzen ist Klasse S**, aber eine
falsche Zeit fällt sofort auf dem Display auf — zuletzt prüfen und sofort korrigieren.

### 6.2 `network` — teils Klasse N, teils gar nicht

**SSID und WLAN-Schlüssel werden nicht geschrieben. Punkt.**

`network_client_set` schreibt die Zugangsdaten und löst eine Neuanmeldung am Router
aus. Das trifft **genau die Verbindung, über die geprüft wird**. Selbst das
Zurückschreiben der unveränderten Werte bedeutet einen Verbindungsabbruch — und beim
kleinsten Vertipper ist die Uhr nicht nur für den Durchlauf weg, sondern dauerhaft,
bis jemand physisch an das Gerät geht. Ein Testplan, der sich dafür auf „Rückweg
vorbereitet" verlässt, setzt die Erreichbarkeit aufs Spiel, um die Erreichbarkeit zu
prüfen. Das ist kein akzeptabler Tausch.

Dasselbe gilt für **WPS**: Der Vorgang kann die gespeicherten Zugangsdaten ersetzen,
ohne dass die PWA das Ergebnis kontrolliert.

Beide stehen deshalb in **Phase 8**, nicht hier.

| Was | Klasse | Besonderheit |
|---|---|---|
| WLAN-Suche (`network_scan`) | **L** | Liefert die Liste? Wie reagiert sie, wenn keine Netze gefunden werden? Rein lesend, gefahrlos |
| **WLAN-SSID und -Schlüssel setzen** | **G** | **Nicht ausführen** — Phase 8 |
| **WPS** | **G** | **Nicht ausführen** — Phase 8 |
| AP-SSID und AP-Schlüssel | N | Betrifft **nur** den Zugangspunkt-Modus, nicht die laufende Client-Verbindung — daher prüfbar. Eingabefeld 32 Zeichen, EEPROM **64**: die PWA ist strenger als das Gerät. **Danach unbedingt zurückschreiben**, sonst ist der Notzugang mit falschen Daten hinterlegt |
| „Als Zugangspunkt starten" (`boot_as_ap`) | **G** | **Nicht setzen.** Beim nächsten Neustart wäre die Uhr nicht mehr im WLAN |
| Zeitserver | N | Eingabefeld **32** Zeichen, ESP speichert **16** (`MAX_TIMESERVER_NAME_LEN`). **Erwarteter Befund:** stille Kürzung |
| Zeitzone, Sommerzeit | S | Bereich −12..14, wirkt auf die Anzeige, sofort rücknehmbar |
| Zeit vom Netz holen | S | prüft, ob der Zeitserver erreichbar ist |

**Was die SSID-Prüfung ersetzt:** Dass der Weg grundsätzlich funktioniert, ist dadurch
belegt, dass die Uhr **jetzt** im WLAN ist — die Funktion ist in Benutzung. Geprüft
wird deshalb nur, was ohne Schreiben geht: Liefert die Suche Treffer, erscheint die
aktuelle SSID darin, zeigt die Oberfläche den Verbindungszustand richtig an.

**Eine Einschränkung, die dabei auffällt:** Die SSID ist nur aus der Trefferliste
wählbar (`network-ssid-select`), nicht frei eingebbar. Ein **verstecktes** Netz lässt
sich über die PWA nicht konfigurieren. Als Befund protokollieren, nicht als Fehler des
Durchlaufs.

### 6.3 `climate`

Wetter-AppID, Ort, Längen- und Breitengrad, Kartenauswahl (Modal), Wetter jetzt und
Vorhersage abrufen, Temperaturkorrekturen (−20..20), LDR-Minimum und -Maximum,
Temperatur anzeigen.

Das **Kartenmodal** ist ein eigener Prüfgegenstand: Suche, Klick auf die Karte,
aktueller Standort, Übernahme in die Felder, Escape, Klick auf den Hintergrund,
Fokusrückgabe. Der Standortzugriff braucht einen sicheren Kontext — über
`http://` schlägt er fehl, und das ist **erwartetes** Verhalten.

### 6.4 `display`

Ein- und Ausschalten, Modus, RGBW-Umschaltung, Helligkeit (0..15), automatische
Helligkeit, „ES IST" dauerhaft, Tickertext (32 Zeichen), Datumsformat (5 Zeichen),
Tickerverzögerung (0..255), Farbe inklusive Weisskanal (0..63), Dimmkurve, Sekunden-
und Markierungsoptionen, TFT-Flags.

**Der Display-Test ist Klasse G — siehe Phase 8.**

### 6.5 `animations`

Animationsmodus, Farbanimationsmodus, Profile, Profilvorgaben zurücksetzen. Die
Profilvorgabe ist unumkehrbar für dieses Profil — Ausgangswerte vorher aus dem
Rohabzug notieren.

### 6.6 `ambilight`

Online, Ein/Aus, Modus, LED-Zahl (0..999), Versatz (0..999), Helligkeit,
Ambilight-Farbe und Markierungsfarbe mit Weisskanal, Synchronisation mit dem Display,
Dimmkurve, Profile.

**Zu beachten (L11):** Am LED-Board dieser Uhr ist kein Ambilight-Ausgang
herausgeführt. Einstellungen lassen sich speichern, bleiben aber ohne sichtbare
Wirkung. Das ist **kein Fehler** — als „nicht sichtbar prüfbar" protokollieren.

### 6.7 `overlays` und `timers` — die beiden dynamischen Module

Hier liegt die grösste Lücke einer oberflächlichen Prüfung: **null statische
Bedienelemente.** Zu prüfen:

- Overlay anlegen, Typ wechseln (Icon / Text / MP3 — die Felder werden je Typ ein- und
  ausgeblendet), Wert setzen, speichern, anzeigen, löschen
- Alle sieben Icons aus `overlay_icons` durchschalten
- Mehrere Overlays gleichzeitig, Reihenfolge, Grenze der Anzahl
- Timer für Display und Ambilight: setzen, überlappende Zeiten, Mitternachtsübergang
  (22:00 bis 06:00), Start gleich Ende
- Löschen des letzten Eintrags

### 6.8 `dfplayer`

Lautstärke (0..30), Modus, Glockenflags, Sprechzyklus (0..255), Stille von/bis,
Alarme, Ordner und Titel (je 0..255), Abspielen.

Ohne angeschlossenes Modul meldet die PWA „offline" und blendet das Panel aus — dann
als „nicht prüfbar" protokollieren.

### 6.9 `maintenance`

Der grösste Bereich: 24 Schaltflächen, 14 Felder, 8 Dateiauswahlfelder.

| Gruppe | Klasse |
|---|---|
| Quelle und Versionen (`update_host` 63, `update_path` 63) | S |
| Vom Server laden: Tabellen, Assets | S |
| ESP-Firmware aktualisieren | **R** |
| STM32 flashen (remote und lokal) | **R** |
| ESP neu starten, STM zurücksetzen | **R** |
| Lokale Uploads: Display-, Icon-, Wetter-, Tabellendatei, App-Dateien | S, aber grössenkritisch |
| Dateiliste, Datei löschen | **G** |
| EEPROM zurücksetzen, Dateisystem formatieren | **G** |
| Sicherung exportieren und importieren | **Phase 5** |

Für die **Uploads** gilt eine eigene Prüfreihe: leere Datei, Datei mit falschem Typ,
Datei grösser als der freie Platz im LittleFS, Abbruch mitten in der Übertragung.
Gerade die leere Datei ist hier historisch belegt gefährlich — eine leere `.gz` erzeugt
einen weissen Bildschirm. ESP-seitig ist das seit der Prüfung auf Dateigrösse > 0
abgefangen, **PWA-seitig steht die Prüfung noch aus** (Massnahme 7). Erwarteter Befund.

---

## 7. Phase 4 — Grenzfälle

Für jedes Eingabefeld dieselben acht Klassen. Nicht stichprobenartig — **jedes Feld**.

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

**Erwarteter Befund bei E3 und E5:** Die Oberfläche biegt Werte ausserhalb des Bereichs
derzeit **still zurecht**, statt sie abzuweisen (Massnahme 17, offen). Der Durchlauf
bestätigt das, er entdeckt es nicht. Festzuhalten ist, **an welchen Feldern** es
auftritt — das ist die Grundlage für die Korrektur.

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
| WLAN-Schlüssel, AP-SSID, AP-Schlüssel, Zeitserver, AppID, Ort, Tickertext | 32 Zeichen |
| Längen- und Breitengrad | 8 Zeichen |
| Datumsformat | 5 Zeichen |
| Update-Host und -Pfad | 63 Zeichen |
| Kartenmodal: Suche, Ort | 64 Zeichen |
| Kartenmodal: Längen-/Breitengrad | 16 Zeichen |

**Zusätzlich, feldübergreifend:**

- Datum **31. Februar** — wird es abgewiesen?
- Längengrad `8,5400` mit Komma statt Punkt
- Zeitzone `+2` mit führendem Pluszeichen
- Zweimal schnell hintereinander auf dieselbe Speichern-Schaltfläche
- Zwei Felder ändern, nur eines speichern, Modul wechseln — kommt die Warnung über
  ungespeicherte Änderungen? **Erwarteter Befund:** `hasUnsavedEdits` wird nach
  normalem Speichern nicht zurückgesetzt (Massnahme 4, offen), die Warnung erscheint
  also auch ohne offene Änderung.

---

## 8. Phase 5 — Backup und Restore als eigener Prüfgegenstand

Das ist kein Nebenschauplatz. Der Import kann die Uhr ohne WLAN, ohne AP und ohne
Webserver zurücklassen — er ist die einzige **Klasse-G**-Funktion, die dieser Plan
trotzdem scharf ausführt, weil sie sonst nie geprüft würde.

### 8.1 Was das Backup umfasst

Zehn Abschnitte: `display`, `network`, `maintenance`, `climate`, `animations`, `tft`,
`ambilight`, `dfplayer`, `overlays`, `timers`, dazu ein Kopf mit Quelle und
Versionsstand und ein `assets`-Block, der **nur Namen** enthält (Layout-Tabelle,
verwendete Icons, Asset-Präfix) — **keine Dateien**.

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

---

## 9. Phase 6 — Verbindung, Offline, Service Worker

| | Prüfung |
|---|---|
| **V1** | Uhr im Betrieb vom Netz trennen — wie reagiert die Oberfläche? Erscheint ein Hinweis oder friert sie stumm ein? |
| **V2** | Wiederverbinden — findet `reconnect_probe` von selbst zurück? |
| **V3** | Fenster in den Hintergrund legen. **Erwarteter Befund:** Das Polling läuft weiter (R2-11, offen) |
| **V4** | Tab offen lassen, ESP neu starten — erkennt die PWA das? |
| **V5** | Zwei Browser gleichzeitig, in beiden dieselbe Einstellung ändern — was gewinnt? |
| **V6** | Seite neu laden während eines laufenden Speichervorgangs |
| **V7** | Service Worker: Installation über `http://` schlägt fehl, weil kein sicherer Kontext. **Erwarteter Befund:** Es gibt derzeit keinen Hinweis darauf (R2-10, offen) |
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

**Sieben Funktionen werden bewusst nicht regulär ausgelöst.** Für jede gibt es eine
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

**Zusätzlich gilt:** `GET /?a` (Parameter ohne `=`) hat den ESP früher zum Absturz
gebracht. Der Smoketest sendet das bewusst und prüft, dass die Uhr weiterlebt. Von Hand
ist es **nicht** zu wiederholen.

---

## 12. Phase 9 — Wiederherstellung und Abschlussvergleich

**Das ist der Schritt, der aus dem Durchlauf einen Nachweis macht.**

1. PWA-Backup aus Phase 0 importieren (entspricht B5)
2. Neuen Rohabzug erzeugen
3. **Feldweise gegen die Referenzdatei aus Phase 1 vergleichen**
4. Jede Abweichung ist entweder ein erklärter Rest (z. B. die Betriebszeit) oder ein
   **Befund**
5. `./tools/smoke-device.sh` — muss wieder 25/0 melden
6. `./tools/install-app.sh --check` — Dateien vollständig
7. Mitschnitt auf dem Pi durchsehen: Watchdog-Resets, Lücken über 90 s, zerrissene
   Zeilen während des Durchlaufs

**Ohne Schritt 3 ist der Durchlauf nicht abgeschlossen.** „Sieht wieder normal aus" ist
keine Wiederherstellung.

---

## 13. Was der Durchlauf voraussichtlich findet

Ein Testplan, der nur Bekanntes bestätigt, ist überflüssig; einer, der so tut, als sei
alles offen, ist unehrlich. Diese Befunde sind **vorhergesagt** — treten sie auf, ist
das kein neuer Erkenntnisgewinn, sondern eine Bestätigung des Katalogs:

| Erwartet | Katalog |
|---|---|
| Werte ausserhalb des Bereichs werden still zurechtgebogen statt abgewiesen | Massnahme 17 |
| Warnung über ungespeicherte Änderungen erscheint auch ohne solche | Massnahme 4 |
| Polling läuft im Hintergrundtab weiter | R2-11 |
| Kein Hinweis bei unsicherem Kontext | R2-10 |
| Leere Datei beim App-Upload wird PWA-seitig nicht abgefangen | Massnahme 7 |
| Zeitserver wird bei über 16 Zeichen still gekürzt | **neu** — UI erlaubt 32, ESP speichert 16 |
| Verstecktes WLAN ist über die PWA nicht konfigurierbar | **neu** — SSID nur aus der Trefferliste wählbar |
| Fehlgeschlagener Flash ist nur einen Frame lang sichtbar | R2-5 |

**Alles andere wäre neu** — und gehört als `L`-Befund in `BEFUNDE.md`.

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
