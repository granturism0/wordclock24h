# Anforderungen — Grosses Befundpaket

**Status:** Entwurf — zur Freigabe durch den Nutzer
**Momentaufnahme vom 2026-10-04.** `specs/**` wird nicht fortgeschrieben (DIR-006).
Den lebenden Stand führt `BEFUNDE.md`; weicht etwas ab, gilt dort das Neuere.
**Auslöser:** Arbeitsliste „ToDo" in `BEFUNDE.md`, Auftrag des Leads vom 04.10.2026.

Versionsnummern stehen hier bewusst nicht. Welcher Stand gemeint ist, sagt
`./tools/guardrails.sh` in Stufe S4; die historischen Stände der zitierten Befunde
stehen in `BEFUNDE.md`.

---

## Worum es geht

Vier Stränge aus der Arbeitsliste, mit einer vorgeschalteten Runde 0. Ausgerollt
wird in **Runden**, nicht in einem Zug. Die Reihenfolge ist nicht beliebig: Runde 0
baut die Messinstrumente, an denen die Abnahme aller folgenden Runden hängt. Ohne
sie wiederholen wir den Fehler aus L177 — eine Abnahmebedingung, die zu erfüllen gar
nicht möglich war, weil die zu lesende Zeile nie dort ankam, wo gelesen wurde.

| Runde | Inhalt | Laufzeit | Rollout |
|---|---|---|---|
| **0** | Beobachtbarkeit und Prüfmethode (C14, E16, Prüfmethode zu L188) | ESP, Doku | OTA + Doku |
| **1** | ESP-Parametervertrag (C15, C16, Vertragsfestlegung der C6-Familie) | ESP, PWA | OTA + LittleFS |
| **2** | PWA-Eingabefelder (B1-Familie) | PWA | LittleFS |
| **3** | UI-Kacheln (B13, B14, L114) | PWA-Markup | LittleFS |
| **4a** | A6 — Selbstheilung des Variablensatzes | ESP (ggf. STM) | OTA (ggf. STM-Flash) |
| **4b** | Flash-Überwachung (C9c6, L179) | ESP, Werkzeug | OTA |

Strang 4 zerfällt in **zwei** Rollouts, obwohl beide im ESP liegen. Begründung in
`design.md`, Abschnitt „Rundeneinteilung" — kurz: Scheitert ein OTA, muss eindeutig
sein, welche der beiden Änderungen es war (siehe Risiko 1).

---

## Problem

### Runde 0

#### C14 (L185) — zwei Beobachtbarkeits-Zeilen, die niemand lesen kann

`esp_heap_log()` schreibt die Heap-Zeile mit `Serial.print()`
(`ESP8266/ESP-uclock/ESP-uclock.ino:463-467`), die Verlustzeile
`- http write lost N` ebenso (`ESP8266/ESP-uclock/http.cpp:1212-1214`). Beide landen
auf der STM-UART und damit **nur** im seriellen Mitschnitt.

`stm32_log_append()` (`ESP-uclock.ino:92`) füllt den Ring, den `/api/stm32_log`
ausliest. Die Funktion hat genau **eine** Aufrufstelle (`ESP-uclock.ino:1218`), und
die greift ausschliesslich bei Zeilen, die der **STM** mit dem Präfix `LOG ` sendet
(`src/log/log.c:33` → `src/esp8266/esp8266.c:580`).

Gemessen am 04.10.2026: `curl …/api/stm32_log | grep -c "heap free"` → **0**, während
im seriellen Mitschnitt desselben Fensters **36** solcher Zeilen stehen.

Tragweite: Beide Massnahmen sind heute nur mit angeschlossenem Pi-Mitschnitt nutzbar.
Wer sie über die API sucht, findet nichts und schliesst, sie funktionierten nicht.
Es ist die **zweite** Beobachtbarkeits-Massnahme in zwei Tagen, deren Meldung
niemanden erreicht — nach L181, wo die Logwache in eine gepufferte Pipe schrieb.

Nebenbedingung: Der Ring fasst **32 Zeilen** zu je 120 Zeichen
(`ESP-uclock.ino:80-81`). Er wurde bewusst von 64 auf 32 verkleinert, weil nicht der
freie Heap knapp ist, sondern der grösste zusammenhängende Block (L175, L176). Jede
ESP-eigene Zeile verdrängt eine STM-Zeile. Das ist kein Nebenaspekt, sondern die
Grenze, innerhalb derer die Lösung bleiben muss.

#### E16 (L150, L183) — zwei falsche Angaben im Testplan

**(a) S47 steht unter „folgenlos rücknehmbar".** `TESTPLAN-PWA.md:512` führt
S47 (Farbanimation wählen) als Klasse **S** mit dem Hinweis, die Display-Farbe aus
S34 sei „während der Animation wirkungslos". Am Gerät belegt (L150): Vor dem Versuch
stand `dspcolor[0]` auf `0/0/0/63`; nach `color_animation_mode_set=2` und
**sofortigem Zurücksetzen auf 0** stand sie auf `0/0/31/0`. Die Farbe ist **danach
verloren**, nicht nur während der Animation wirkungslos. Der Testplan verspricht
damit eine Rücknehmbarkeit, die es nicht gibt — und genau in dieser Klasse arbeitet
der `pwa-tester` ohne Rückfrage.

**(b) S114 ist als reiner STM-Eingriff beschrieben.** `TESTPLAN-PWA.md:626` nennt als
Wirkung die Startsequenz des STM und die Reparatur von `HARDWARE_CONFIGURATION`.
Tatsächlich startet der ESP mit neu (L183, dreifach belegt): `update_cache_hits` —
ein reiner ESP-Zähler ohne STM-Bezug — stand vor `maintenance_reset_stm32` auf **5**
und danach auf **0**. Folgen: Alle ESP-Diagnosezähler (`write_lost_*`,
`no_request_*`, `update_cache_hits`) sind weg, und der Variablenverlust aus
L42/L103 tritt ein. **Messreihen über einen STM-Reset hinweg sind wertlos** — am
04.10.2026 wurde `write_lost_blocks: 0` fälschlich als Erfolg gelesen, obwohl der
Zähler nur zurückgesetzt war.

#### Prüfmethode (L188, L103) — der blinde Fleck jeder Setter-Gegenprobe

`settings_xml` liefert die **ESP-seitige** Variablenkopie. Der ESP setzt sie beim
Setter sofort und schickt das Kommando erst danach an den STM. Geht es auf der
Brücke verloren, meldet die Gegenprobe **trotzdem den neuen Wert**.

Jedes „bestanden" eines Setter-Tests belegt damit nur: der ESP hat gespeichert —
nicht, dass der STM es angewandt hat. Belegt ist, dass Verlust real vorkommt: Der
Empfangsring stieg innerhalb eines Boot-Zyklus auf `rx=1024/1024 d=5010`.

Das entwertet den Durchlauf nicht, aber es benennt, was er nicht zeigen kann. Heute
steht diese Einschränkung **nirgends im Testplan**, und der Testbericht liest sich
deshalb stärker, als er ist — dieselbe Gattung wie DIR-012 („Smoketest 27/0" zu
schreiben und „getestet" zu meinen).

### Strang 1 — ESP-Parameterprüfung (C6-Familie)

#### C15 (L186) — zwei Geschwister, gegensätzliches Verhalten

Am Gerät gemessen (04.10.2026), derselbe Parameter `deceleration`, Ausgangswert
bewusst auf 5 gesetzt, damit Klemmen und Ignorieren unterscheidbar bleiben:

| Aufruf | Antwort | Wert danach |
|---|---|---|
| `animation_profile_set?idx=5&deceleration=16` | `{"ok":false,"error":2,"detail":"deceleration out of range"}` | unverändert |
| `ambilight_mode_profile_set?idx=3&deceleration=16` | `{"ok":true}` | still auf **15** geklemmt |
| `ambilight_mode_profile_set?idx=3&deceleration=99` | `{"ok":true}` | still auf **15** geklemmt |

`idx` wird in **beiden** korrekt abgewiesen. Im Quelltext ist der Unterschied
ausdrücklich: `http.cpp:9670-9674` weist ab, `http.cpp:10081-10088` klemmt, mit dem
Kommentar „Die Klammerung von deceleration bleibt absichtlich stehen — das ist
Massnahme 17 und nicht dieser Schritt."

Der Einzelfall ist nicht das Problem. Das Problem ist: Die Oberfläche kann sich auf
**kein** einheitliches Fehlerverhalten stützen. Sie müsste je Endpunkt wissen, ob
ein `{"ok":true}` den gesendeten Wert bedeutet oder einen anderen.

#### C16 (L187) — `fs_show` schweigt bei fehlender Datei

`GET /api/fs_show?filename=gibtsnicht.txt` → Antwortlänge **0**, kein JSON, kein
`{"ok":false}`. Dieselbe Abfrage auf eine vorhandene Datei liefert 420 Byte Inhalt.
Im Quelltext sichtbar (`http.cpp:11187-11226`): Die Kopfzeilen gehen **vor** dem
Öffnen der Datei hinaus; schlägt `LittleFS.open()` fehl, folgt nichts mehr.

Die PWA kann damit „Datei leer" nicht von „Datei fehlt" unterscheiden — und eine
leere Datei ist hier kein theoretischer Fall, sondern genau der Weisschirm-Fehlerfall
der Architektur-Invariante. `fs_show` ist der **einzige** Ausreisser unter allen im
Durchlauf geprüften Endpunkten; alle anderen liefern sauberes
`{"ok":false,"error":N,"detail":"…"}`.

#### Der Rest der C6-Familie

Rund zwanzig Befunde derselben Klasse stehen in der Arbeitsliste: C6f bis C6u, E12.
Sie sind einzeln klein und zusammen ein Vertrag, den es heute nicht gibt. **Dieses
Paket legt den Vertrag fest und setzt ihn an den belegten Fällen durch**; die
vollständige Durchsicht aller Setter ist eine eigene Runde (siehe „Nicht Teil").

### Strang 2 — PWA-Eingabefelder (B1-Familie)

Offen nach `BEFUNDE.md` (Stand dieser Momentaufnahme):

| | Was | Fundstelle |
|---|---|---|
| B1 | `hasUnsavedEdits` nach dem Speichern zurücksetzen | Massnahme 4 — Reset nur an zwei Stellen |
| B2 | Poller bei `document.hidden` stoppen | R2-11, `visibilitychange` ohne `else` |
| B3 | `apiFetch` statt rohem `fetch` im Flash-Pfad | R2-4 |
| B4 | `finishProgressUi(2200)` statt `(0)` | R2-5 — ein Fehlschlag ist einen Frame lang sichtbar |
| B5 | Hinweis bei `isSecureContext === false` | R2-10 — ohne HTTPS scheitert die Installation wortlos |
| B6 | Formularvalidierung statt stillem Clamping | Massnahme 17 — Gegenstück zu Strang 1 |
| B7 | `file.size > 0` beim App-Install, Längenprüfung im SW | Massnahme 7 — ESP-Seite abgesichert, PWA-Seite nicht |
| B1i | Overlay-Zahlenfelder prüfen | L64 — fünf Felder als rohe `.value` gesendet |
| B11 | Toten `power_status`-Getter benutzen oder entfernen | L111 |
| B12 | Bedienelemente für vier nur per Import erreichbare Setter | L112 |
| B17 | STM-Logs erscheinen erst nach einem Reload | L142 — drei Spuren offen, der leere Zustand ist in allen falsch |
| B18 | Keine Warnung vor ungespeicherten Änderungen beim Modulwechsel | L151 |

**Befund beim Schreiben dieser Spec:** Die Arbeitsliste führt unter „B" zusätzlich
B1b, B1c, B1d, B1e, B1g, B1j, B1k und B1l. Deren Befundzeilen (L31, L29, L33, L34,
L52, L65, L74, L72) stehen in den Statustabellen durchweg auf **erledigt**. Die
Arbeitsliste ist dort veraltet — Guardrail S10 prüft nur, dass jeder *offene* Befund
in der Liste steht, nicht die Gegenrichtung. Diese acht sind **nicht** Teil dieses
Pakets; ihre Zeilen gehören in der Arbeitsliste gestrichen (eigener Doku-Task).

### Strang 3 — UI-Kacheln

| | Was | Fundstelle |
|---|---|---|
| B13 | Kachelraster und Fokusreihenfolge in `maintenance` | L114, L116 |
| B14 | Beigebraune Darstellung des unangekreuzten Felds bewerten | L120 |
| L114 | Das Kachelraster lässt auf breiten Bildschirmen grosse Lücken | vom Nutzer mit Bildschirmfoto gemeldet |

L114 ist am Gerät vermessen, nicht hergeleitet: Im Modul `maintenance` bei 1512 px
Breite misst die Kachel `source` **1248 px** gegen `remote` **348 px**; die Differenz
von rund 900 px ist die Leerfläche. Die Höhendifferenz ist **struktureller
Normalzustand** — die Fortschrittsanzeige in `remote` trägt `is-hidden`, solange kein
Update läuft, die Kachel ist also immer kurz.

L116 ist die Normseite desselben Befundes: Ab 900 px stellen drei handgestellte
`grid-template-areas` die Kacheln um, das Markup bleibt. Sichtbare gegen
DOM-Reihenfolge ab 1240 px: `1, 2, 5, 3, 6, 4` — zwei Rücksprünge. Wer aus „Vom
Server laden" heraustabbt, landet in **„Service-Aktionen"** mit „STM32 zurücksetzen"
und „EEPROM zurücksetzen". WCAG 2.1 SC 1.3.2 und SC 2.4.3, beide Stufe A.

Der UX-Review empfiehlt **Variante 2**: Markup in die gewollte Reihenfolge bringen
(source, remote, local, backup, service, files), danach entfallen alle drei
`grid-template-areas`-Blöcke und die sechs `grid-area`-Zuweisungen. Geprüft:
`app.js` greift ausschliesslich über IDs zu, `nth-of-type` kommt dort **nicht
einmal** vor.

**Befund beim Schreiben dieser Spec:** Der Auftrag nennt zusätzlich B1f und B1h.
Beide stehen auf erledigt — L43 („von 46 auf 14 zurückgeführt", je Karte höchstens
eine Hervorhebung) und L59/L60 („in `app.js` steht kein `primary` mehr"). Sie werden
hier **nicht umgesetzt, sondern einmal gegengeprüft**; findet die Gegenprobe etwas,
wird daraus ein neuer Befund, kein stiller Nachbau.

### Strang 4 — A6 und die Flash-Überwachung

#### A6 (L42, L102, L103) — der Variablensatz nach einem ESP-Neustart

Nach einem ESP-Neustart fehlt dem ESP der **gesamte statische** Variablensatz, nicht
nur die Update-Quelle. Belegt im Testdurchlauf vom 03.10.2026:
`HARDWARE_CONFIGURATION` 782 → **65535**, Helligkeit 13 → 0, Zeitzone 513 → 0,
Display- und Ambilight-Power 1 → 0, alle Farbwerte → 0, Zeitserver, Wetter-AppID,
Ort, Koordinaten und Datumsformat leer. Die zyklisch nachgesendeten Werte (Uptime,
LDR, Temperatur) stimmen weiter — die Brücke lebt, nur die statischen Werte kommen
nie wieder.

Drei Folgen, jede für sich schwer:

1. **Jeder STM-Flash schlägt danach still fehl** (DIR-010): Bei
   `HARDWARE_CONFIGURATION` = 65535 bildet der ESP keinen Dateinamenfilter und weist
   jeden Namen ab.
2. **Die Update-Quelle fällt auf die eingebauten Vorgaben zurück** — den Server des
   Ursprungsprojekts (`http.cpp:44-45`). Das nächste Update holte fremde Firmware,
   ohne jede Fehlermeldung. Am 03.10.2026 eingetreten.
3. **Der Rohabzug bildet den Gerätezustand nicht mehr ab.** Der Testagent brach
   Phase 3 deshalb ab — ein regelkonformes „setzen, gegenprüfen, zurückschreiben"
   hätte die Uhr dauerhaft auf Helligkeit 0 gestellt.

`maintenance_reset_stm32` heilt es, löst es aber zugleich neu aus (L183): Der Reset
reisst den ESP mit, und der Zyklus beginnt von vorn.

Der Mechanismus existiert bereits und ist unzuverlässig, nicht abwesend:
`src/main.c:2896-2908` ruft `var_send_all_variables()` im Zweig `ESP8266_IPADDRESS`.
Rund 190 quittungspflichtige Kommandos laufen dort gegen einen ESP, der noch
hochfährt; der 3-Sekunden-Abbruch macht „weiter ohne" (L42, L85, L107).

**Die Ursachenanalyse ist nicht abgeschlossen** — der Nutzer sagt, es sei früher
nicht so gewesen. Deshalb steht in Runde 4a eine Messung **vor** dem Eingriff.

#### Flash-Überwachung (C9c6, L179) — der Legacy-Pfad flasht ungeprüft

Am 04.10.2026 um 01:39 eingetreten: Ein F103-Abbild landete auf dem F411 und legte
die Uhr still. Der STM sendete danach Binärmüll über die Brücke; ein
`maintenance_reset_stm32` half nicht — die Firmware war nicht hängen geblieben, sie
war die falsche.

Die Kette aus drei bekannten Befunden:

1. Der OOM-Absturz um 00:54 (L174) löschte den Variablensatz, `stm32_default` wurde
   leer (L42).
2. Damit bot die Legacy-Auswahlliste **alle** Firmwaredateien ohne Vorauswahl an —
   die F103 direkt neben der F411.
3. Der Legacy-Pfad übernimmt den Namen **ungeprüft**
   (`http.cpp:6909-6913`: `strncpy (flash_stm32_filename, http_get_param
   ("stm32_filenames"), 63);`).

Der API-Endpunkt prüft sehr wohl, über `http_remote_stm32_filename_matches()` gegen
die Hardwarekennung — er hätte abgewiesen. **Zwei Wege zum selben Ziel, einer mit
Schutz, einer ohne.**

---

## Ziel

Nach diesem Paket gilt:

1. Die beiden Beobachtbarkeits-Zeilen des ESP sind **ohne Pi-Mitschnitt** über
   `/api/stm32_log` lesbar, ohne den STM-Anteil des Rings zu verdrängen.
2. Der Testplan beschreibt S47 und S114 mit ihren tatsächlichen Folgen und benennt
   ausdrücklich, was eine Setter-Gegenprobe über `settings_xml` **nicht** zeigt —
   mitsamt der Methode, die das Fehlende nachholt.
3. Für Wertebereiche gilt **ein** API-Vertrag: ausserhalb des Bereichs heisst
   abweisen, nicht klemmen. `{"ok":true}` bedeutet „der gesendete Wert gilt".
4. `fs_show` unterscheidet „leer" von „fehlt".
5. Die PWA biegt keine Eingabe still zurecht und blockiert ihre Selbstaktualisierung
   nicht dauerhaft.
6. Das Modul `maintenance` hat auf breiten Bildschirmen keine grossen Lücken mehr,
   und die Fokusreihenfolge entspricht der sichtbaren.
7. Ein ESP-Neustart hinterlässt den Variablensatz vollständig — oder meldet
   erkennbar, dass er es nicht tut. Der Legacy-Flashpfad wendet denselben
   Dateinamenfilter an wie die API.

---

## Akzeptanzkriterien

### Grundsatz: jede Bedingung muss heute messbar sein

Die Abnahmebedingung von L177 lautete „die Heap-Zeile am Ende aus `/api/stm32_log`
auslesen" und war **grundsätzlich nicht erfüllbar**, weil die Zeile dort nie ankommt.
Jedes Kriterium unten nennt deshalb das Messinstrument und ist gegen diese Frage
geprüft: *Kann jemand das heute, mit den vorhandenen Werkzeugen, nachvollziehen?*

Lesende Abfragen am Gerät sind frei (DIR-008). Alles Schreibende braucht die
ausdrückliche Freigabe des Nutzers im selben Gespräch (R5).

### Runde 0

- [ ] **AK0.1** — Binnen 120 s nach einem ESP-Neustart enthält
      `curl …/api/stm32_log` mindestens eine Zeile mit `heap free=`.
      *Instrument:* `curl`, lesend.
- [ ] **AK0.2** — Nach einem Lauf `./tools/check-pwa.sh` (der laut L178 reproduzierbar
      Schreibverluste durch Browser-Abbau erzeugt, gemessen +2 und +1 Block) steigt
      `write_lost_blocks` in `/api/update_status`, **und** `/api/stm32_log` enthält
      binnen 60 s danach mindestens eine Zeile `http write lost`.
      *Instrument:* `check-pwa.sh` plus zwei lesende Abrufe.
      *Wenn die Zähler nicht steigen, ist AK0.2 nicht gescheitert, sondern nicht
      gefahren* — dann mit einem zweiten Lauf wiederholen und das Ergebnis notieren.
- [ ] **AK0.3** — In derselben Ringabfrage wie AK0.1 stehen weiterhin STM-Zeilen:
      mindestens zwei Diagnosezeilen (Präfix der `diag`-Folge) unter den 32 Zeilen.
      Der ESP-Anteil liegt bei höchstens einem Viertel der Zeilen.
      *Instrument:* `curl …/api/stm32_log`, Zeilen zählen.
- [ ] **AK0.4** — ESP-eigene Zeilen sind im Ring **als solche erkennbar**, ohne den
      Text zu raten: Jede trägt das Präfix, das `design.md` festlegt.
      *Instrument:* Sichtprüfung derselben Ausgabe.
- [ ] **AK0.5** — `TESTPLAN-PWA.md` führt S47 mit dem Vermerk „Ausgangswert vorher
      notieren" in derselben Form wie S51/S53, und S114 nennt den ESP-Mitstart samt
      Verlust aller ESP-Zähler und des Variablensatzes.
      *Instrument:* `grep` im Testplan.
- [ ] **AK0.6** — `TESTPLAN-PWA.md` enthält einen benannten Abschnitt, der erklärt,
      dass `settings_xml` die ESP-Kopie liefert, und drei Verfahren nennt, mit denen
      eine STM-seitige Aussage entsteht (siehe `design.md`). Jede Setter-Phase trägt
      die Rahmenmessung der Verlustzähler.
      *Instrument:* Sichtprüfung des Testplans.
- [ ] **AK0.7** — Ein Probedurchlauf einer Setter-Phase nach neuer Methode liefert
      ein Protokoll, in dem vor und nach der Phase der `d=`-Wert aus der
      Diagnosezeile steht.
      *Instrument:* `pwa-tester`, lesender Teil, plus Freigabe für die Setter.

### Runde 1

- [ ] **AK1.1** — `ambilight_mode_profile_set?idx=<gültig>&deceleration=16` liefert
      `{"ok":false,"error":2,"detail":"…"}`, und der Wert bleibt **unverändert**.
      Gegenprobe mit einem bewusst gesetzten Ausgangswert ungleich der Grenze, damit
      Klemmen und Ignorieren unterscheidbar bleiben.
      *Instrument:* `curl`, Freigabe nötig (schreibend).
- [ ] **AK1.2** — Derselbe Aufruf mit einem **gültigen** Wert liefert `{"ok":true}`,
      und der Rohabzug zeigt genau diesen Wert.
      *Instrument:* `curl` plus `settings_xml`, mit der Einschränkung aus AK0.6.
- [ ] **AK1.3** — Jeder im Paket umgestellte Endpunkt nennt im `detail` den
      zulässigen Bereich in der Form `<param> out of range (<min>..<max>)`.
      *Instrument:* `curl` je Endpunkt; die Liste steht in `design.md`.
- [ ] **AK1.4** — `fs_show?filename=<nicht vorhanden>` liefert
      `Content-Type: application/json` mit `{"ok":false,…}`;
      `fs_show?filename=<vorhandene, leere Datei>` liefert
      `Content-Type: text/plain` mit Länge 0.
      *Instrument:* `curl -i`, lesend. Eine leere Datei ist für den Test anzulegen —
      **nicht** über `fs_remove` oder `format_fs`.
- [ ] **AK1.5** — Die PWA zeigt für den Fall „Datei fehlt" eine andere Meldung als
      für „Datei leer".
      *Instrument:* `./tools/check-pwa.sh` plus Sichtprüfung im Browser.
- [ ] **AK1.6** — Weder PWA noch Legacy senden im Normalbetrieb einen Wert, den der
      neue Vertrag abweist. Nachweis: ein Durchlauf der betroffenen Module ohne
      `{"ok":false}` in der Antwort.
      *Instrument:* `pwa-tester`, Phasen der betroffenen Module.

### Runde 2

- [ ] **AK2.1** — Nach einem normalen Speichern ist `hasUnsavedEdits` zurückgesetzt:
      Die Selbstaktualisierung läuft wieder an, ohne dass neu geladen wird.
      *Instrument:* Browser, beobachtbar an der aktualisierten Uhrzeit.
- [ ] **AK2.2** — Liegt das Fenster im Hintergrund, erreicht das Gerät **kein**
      Poll-Request mehr. Nachweis über den Request-Zähler des Geräts oder den
      Mitschnitt, über ein Fenster von mindestens 60 s.
      *Instrument:* `/api/update_status`-Zähler bzw. `tools/watch-log.sh`.
- [ ] **AK2.3** — Ein fehlgeschlagener Flash bleibt mindestens 2 s sichtbar.
      *Instrument:* Browser, Bildschirmaufnahme.
- [ ] **AK2.4** — Ohne sicheren Kontext erscheint ein Hinweis statt wortlosen
      Scheiterns.
      *Instrument:* Aufruf über `http://` ohne `localhost`.
- [ ] **AK2.5** — Ein Zahlenfeld ausserhalb des Bereichs wird **gemeldet**, nicht
      still zurechtgebogen; der Text nennt beide Zahlen (eingegeben → erlaubt), wie
      es der Backup-Import nach L65 bereits tut.
      *Instrument:* Browser, je ein Feld aus Display, Ambilight und Overlay.
- [ ] **AK2.6** — Eine Datei der Grösse 0 wird beim App-Install **PWA-seitig**
      abgewiesen, bevor sie gesendet wird.
      *Instrument:* Browser mit einer selbst angelegten leeren Datei.
- [ ] **AK2.7** — Die fünf Overlay-Zahlenfelder gehen nicht mehr als rohe `.value`
      hinaus.
      *Instrument:* Netzwerk-Mitschnitt des Browsers.
- [ ] **AK2.8** — Das STM-Logfeld zeigt beim ersten Laden entweder Zeilen oder eine
      Meldung — **nie** ein leeres Feld ohne Text.
      *Instrument:* Browser, Neuladen mit geleertem Ring.
- [ ] **AK2.9** — Ein Modulwechsel mit ungespeicherten Änderungen warnt.
      *Instrument:* Browser.
- [ ] **AK2.10** — `./tools/install-app.sh --check` meldet jede Datei der Weissliste
      mit Grösse > 0, und `./tools/check-pwa.sh` meldet keinen Fehler beim Laden von
      `app.js`.
      *Instrument:* beide Skripte.

### Runde 3

- [ ] **AK3.1** — Im Modul `maintenance` bei 1512 px Breite beträgt die grösste
      Höhendifferenz zweier nebeneinanderstehender Kacheln höchstens 400 px
      (heute 900 px).
      *Instrument:* `tools/preview/` mit `&module=maintenance`, Geometrie messen —
      derselbe Weg, der die 1248/348 px belegt hat.
- [ ] **AK3.2** — DOM-Reihenfolge und sichtbare Reihenfolge stimmen bei 1240 px,
      1512 px und 900 px überein; es gibt keinen Rücksprung.
      *Instrument:* `tools/preview/diag.js`.
- [ ] **AK3.3** — In `styles.css` steht kein `grid-template-areas`-Block und keine
      `grid-area`-Zuweisung mehr für `maintenance`.
      *Instrument:* `grep`.
- [ ] **AK3.4** — Die PWA startet nach dem Umbau fehlerfrei, und alle Schaltflächen
      des Moduls sind weiterhin über ihre IDs erreichbar.
      *Instrument:* `./tools/check-pwa.sh` (meldet einen Fehler beim Laden von
      `app.js`, den weder API noch Smoketest noch Screenshot zeigen).
- [ ] **AK3.5** — Für L120 liegt eine **Entscheidung mit Begründung** vor, kein
      Patch ohne Bewertung: Entweder bleibt `accent-color`, dann ist der
      Kontrastwert des Hakens gemessen und ≥ 3:1 (WCAG 1.4.11), oder es gibt einen
      Ersatz mit derselben gemessenen Zusage.
      *Instrument:* `ui-reviewer` mit Kontrastmessung.
- [ ] **AK3.6** — Gegenprobe B1f/B1h: Je `.panel` höchstens eine Hervorhebung, und
      `app.js` enthält kein `primary` ausser im erklärenden Kommentar.
      *Instrument:* `grep` plus Zählung im gerenderten Markup. Weicht etwas ab, wird
      ein **neuer Befund** geschrieben, nicht stillschweigend nachgebessert.

### Runde 4a

- [ ] **AK4.1** — Vor jedem Eingriff liegt eine Messung vor, die beantwortet, **an
      welcher Stelle** der Vollabgleich scheitert: Zahl der gesendeten Kommandos,
      Zahl der Quittungs-Timeouts, `d=`-Zuwachs über das Fenster.
      *Instrument:* serieller Mitschnitt plus `/api/stm32_log`, ausgelöst durch einen
      ESP-Neustart mit Freigabe des Nutzers.
- [ ] **AK4.2** — Nach einem ESP-Neustart (ohne STM-Reset) steht
      `HARDWARE_CONFIGURATION` binnen 60 s wieder auf dem Wert von vorher, nicht auf
      65535. Dreimal nacheinander gefahren, dreimal bestanden.
      *Instrument:* `/api/settings_xml` bzw. `numvar[idx=29]`, lesend; der Neustart
      braucht Freigabe.
- [ ] **AK4.3** — Im selben Fenster stimmen auch Helligkeit, Zeitzone und
      Display-Power mit dem vorher notierten Stand überein.
      *Instrument:* Rohabzug vorher/nachher. **Mit der Einschränkung aus AK0.6** —
      deshalb zusätzlich die STM-seitige Gegenprobe nach der in Runde 0 festgelegten
      Methode.
- [ ] **AK4.4** — `./tools/smoke-device.sh` meldet die Update-Quelle als
      übereinstimmend mit `DEVICE_UPDATE_HOST`/`DEVICE_UPDATE_PATH`, nicht leer und
      nicht auf dem Server des Ursprungsprojekts.
      *Instrument:* `smoke-device.sh`.

### Runde 4b

- [ ] **AK4.5** — Der Legacy-Flashpfad weist einen Dateinamen ab, der nicht zur
      erkannten Hardware passt — mit derselben Prüfung wie der API-Endpunkt.
      *Instrument:* **Nicht scharf fahren.** Nachweis über den Quelltext (dieselbe
      Prüffunktion, nachgewiesen per `grep`) **und** über die Auswahlliste: Bei
      `HARDWARE_CONFIGURATION` = 65535 bietet sie **keine** Datei an statt aller.
      Die Liste ist lesend prüfbar.
- [ ] **AK4.6** — Nach dem STM-Flash dieser Runde meldet `/api/update_status` die
      erwartete STM-Version, und `./tools/flash-stm.sh --check` lief vorher ohne
      Beanstandung.
      *Instrument:* `flash-stm.sh` (DIR-010), nicht der Endpunkt von Hand.

### Für jede Runde

- [ ] **AKZ.1** — `./tools/guardrails.sh` läuft mit Exit 0 durch.
- [ ] **AKZ.2** — `./tools/guardrails.sh --full` läuft vor dem Release mit Exit 0
      durch (Lead, R1).
- [ ] **AKZ.3** — `./tools/smoke-device.sh` nach dem Rollout ohne Fehlschlag
      (DIR-009). **Der Smoketest ist nicht der Test** (DIR-012) — er steht zusätzlich
      zu den Runden-AK, nicht an ihrer Stelle.
- [ ] **AKZ.4** — Während jedes Gerätelaufs läuft `./tools/watch-log.sh` mit und wird
      **mitgelesen** (DIR-013). Ein Neustart, eine Exception oder ein Sprung in `d=`
      während eines Laufs macht dessen Ergebnis ungültig und gehört in den Bericht.
- [ ] **AKZ.5** — Jedes ausgerollte Release ist sofort committet und getaggt
      (DIR-011), einzeln je Runde.

---

## Risiken

### Risiko 1 — Das Paket wird per OTA ausgerollt, und genau dieser Pfad ist betroffen (C13, L180)

Am 04.10.2026 trat eine **neue** Absturzsignatur auf: `Exception (9)` =
`LoadStoreAlignmentCause` mit `excvaddr=0x7470656f` — das ist keine Adresse, das ist
**Text** (`o e p t`). Ein Zeiger wurde mit Zeichen überschrieben: der Lehrbuchfall
eines Pufferüberlaufs, ein echter Programmierfehler, kein Speichermangel.

**Die Folge war genau die hier gefährliche:** Jeder OTA-Versuch scheiterte —
fünfmal probiert, auch bei frischem Heap. Der Aufruf meldete HTTP 200, danach liefen
`var_send_buf`-Timeouts, dann stürzte der ESP ab. Die Korrektur lag gebaut auf dem
Server und kam nicht aufs Gerät, **weil der Fehler, den sie behebt, das Aufspielen
verhinderte.**

Ein wahrscheinlicher Verursacher ist gefunden und inzwischen behoben
(`http_send_settings_xml()` formatierte maskierte Werte mit `sprintf` in `buff[255]`;
ein 63-Zeichen-Wert wird maskiert bis zu 378 Zeichen lang). **Dass das *die* Ursache
war, ist nicht belegt** — die Zuordnung ist zeitlich, nicht nachgewiesen.

**Konsequenzen für dieses Paket, verbindlich:**

- Jede Runde rollt **eine** Laufzeit aus, nicht mehrere gleichzeitig. Scheitert ein
  OTA, ist die Ursache eindeutig zuzuordnen. Darum auch die Trennung von 4a und 4b.
- Vor jedem OTA wird der freie Heap und der grösste Block notiert
  (`/api/device_ready`), damit ein Fehlschlag nicht mit Speichermangel verwechselt
  wird.
- `./tools/watch-log.sh` läuft während jedes OTA mit. Ein Fehlschlag ohne Mitschnitt
  ist nicht auswertbar.
- **Der Rückweg ist physisch.** Bleibt der ESP auf dem alten Stand und lässt sich
  nicht mehr beschreiben, hilft nur serielles Flashen durch den Nutzer. Das ist beim
  Planen jeder Runde einzukalkulieren — es ist seine Uhr im Dauerbetrieb.
- Tritt die Signatur `Exception (9)` mit Text in `excvaddr` erneut auf, **wird das
  Paket angehalten** und C13 als eigene Analyse aufgesetzt.

### Risiko 2 — Runde 4a kann zwei Laufzeiten berühren

A6 ist bevorzugt rein ESP-seitig zu lösen (Weg A in `design.md`). Fällt die
Entscheidung auf Weg B, berührt die Runde auch den STM. Ein STM-Flash setzt den ESP
mit zurück (L183) und löst damit genau den Zustand aus, der behoben werden soll. Die
Reihenfolge in `tasks.md` trägt das: erst messen, dann entscheiden, dann die
ESP-Seite, und — nur bei Weg B — nach einem eigenen Gerätetest die STM-Seite.

### Risiko 3 — Umstellung von Klemmen auf Abweisen ist eine Verhaltensänderung

Sendet die PWA oder die Legacy-Oberfläche heute irgendwo einen Wert, der ausserhalb
des Bereichs liegt und bisher stillschweigend geklemmt wurde, bricht das nach der
Umstellung sichtbar. Das ist gewollt — aber es muss **vor** dem Rollout gefunden
werden, nicht vom Nutzer. AK1.6 deckt das ab; der Prüfschritt steht als eigener Task.

### Risiko 4 — Runde 3 ändert Markup-Reihenfolge

Der Umbau nach Variante 2 verschiebt sechs Kacheln im DOM. Geprüft ist, dass `app.js`
ausschliesslich über IDs zugreift und `nth-of-type` nicht vorkommt. Nicht geprüft ist
der Service Worker und der Backup-Import-Dialog. Das gehört in den Task.

### Risiko 5 — Der Ring ist klein, und wir legen Last hinein

C14 verdrängt STM-Zeilen. Bei einer Heap-Zeile je 60 s und einer Diagnosezeile je
10 s liegt der ESP-Anteil bei rund einem Siebtel — tragbar. **Ändert sich eines der
beiden Intervalle, kippt das.** AK0.3 misst es, und die Rückbaupflicht aus C9c4
(L177) bleibt bestehen: Sobald L175 über mehrere Tage bestätigt ist, fällt die
periodische Heap-Zeile wieder weg, und mit ihr dieser Anteil.

---

## Beobachtungsaufträge (keine Umsetzungstasks)

### A28 (L188) — was füllt den Empfangsring?

**Nicht** als Umsetzungstask. Nach der Messung vom 04.10.2026 ist die Brücke **nicht
dauerhaft überlastet**: `rx` ist laut `src/main.c:3396` ein **Höchststand**, nicht
der aktuelle Füllstand — `rx=1024/1024` heisst „der Ring war einmal voll", nicht „er
ist verstopft". Und `d=5010` **stand über 16 Minuten still**, über 17
aufeinanderfolgende Diagnosezeilen, kein einziges weiteres verworfenes Zeichen. Ein
kontrollierter Burst von 60 Settern in 2,0 s erzeugte **null** Verlust.

Offen ist damit die Eingrenzung eines **einmaligen** Ereignisses kurz nach dem Boot,
nicht ein Dauerzustand.

**Auftrag:** Während jedes Gerätelaufs dieses Pakets wird der `d=`-Wert der
Diagnosezeile am Anfang und am Ende notiert. Steigt er, gehört in den Bericht: um
wie viel, in welchem Fenster, und welche Aufrufe in diesem Fenster liefen. Das ist
die Datengrundlage für die spätere Eingrenzung — gesammelt nebenbei, ohne eigenen
Eingriff.

---

## Nicht Teil dieser Änderung

- **C13 (L180) als Umsetzungstask.** Die Ursache ist unbekannt. Hier steht sie als
  Risiko, nicht als Aufgabe. Eine Korrektur ohne verstandene Ursache wäre ein Patch
  auf Verdacht an genau dem Pfad, über den wir ausrollen.
- **A28 (L188) als Umsetzungstask.** Siehe Beobachtungsauftrag oben.
- **A27 (L182).** Erledigt und am Gerät gemessen: 60 Diagnosezeilen in 590 Sekunden,
  exakt der 10-Sekunden-Takt, gegen 73 Zeilen je Sekunde vorher. Nichts zu tun.
- **Die vollständige Durchsicht aller Setter der C6-Familie.** Dieses Paket legt den
  Vertrag fest und setzt ihn an den belegten Fällen (C15, C16) sowie an den in
  `design.md` beschriebenen, gemessenen Endpunkten durch. C6f bis C6u und E12 im
  Ganzen sind eine eigene Runde — rund zwanzig Endpunkte in einem Zug wären nicht
  einzeln abnehmbar.
- **Die acht veralteten B1-Einträge** (B1b, B1c, B1d, B1e, B1g, B1j, B1k, B1l). Ihre
  Befunde stehen auf erledigt. Sie gehören aus der Arbeitsliste gestrichen; das ist
  ein Doku-Task, keine Umsetzung.
- **A25 / der F103-Flash-Füllstand in den Guardrails.** „Flash-Überwachung" meint in
  Strang 4 den **Firmware-Flashpfad** (A6, L179), nicht den Füllstand des
  F103-Programmspeichers. Letzterer gehört zu A25 und bleibt dort.
- **L175, L176, L178** (Heap, Logring, Schreibverluste). Berührt, aber nicht
  bearbeitet: C14 macht die Zeilen sichtbar, behebt keinen der drei Befunde.
- **Der Rückbau der periodischen Heap-Zeile (C9c4).** Er bleibt als eigener Eintrag
  bestehen und ist **nicht** Teil dieses Pakets — er kommt, wenn L175 über mehrere
  Tage bestätigt ist.
- **`fs_show` als Auslieferungspfad.** Die Funktion sendet heute Zeichen für Zeichen
  (`http.cpp:11207-11213`). Das ist ein eigener Befund und wird hier nicht
  mitkorrigiert.
- **`var_send_all_variables()` selbst** (A1, A17). Rund 190 quittungspflichtige
  Kommandos ohne `watchdog_reload()` — eigene Spec, eigene Geräteverifikation.
- **Jede Umstellung auf POST (C6).** Sie bricht die PWA, solange `apiFetch` ohne
  Methode aufruft, und muss mit der PWA-Seite zusammen geschehen.
- **Versionsnummern und `CACHE_NAME`.** Bumpt ausschliesslich der Lead (R4).

---

## Betroffene Laufzeiten

- [x] STM32 (`src/**`) — Neu-Flashen nötig. **Nur Runde 4a und nur bei Weg B**, also
      nur wenn die Messung aus AK4.1 einen STM-seitigen Eingriff als nötig ausweist
- [x] ESP8266 (`ESP8266/ESP-uclock/*.cpp`, `*.ino`) — Neu-Flashen nötig.
      Runden 0, 1, 4a und 4b
- [x] PWA (`data/app/**`) — nur LittleFS-Upload. Runden 1, 2 und 3.
      **Erinnerung:** `tools/deploy.sh` bringt die Assets nur auf den Update-Server;
      aufs Gerät kommen sie über `./tools/install-app.sh`
- [x] Build/Release — je Runde ein eigenes Release-ZIP, ein eigener Commit, ein
      eigenes Tag (DIR-011)
- [x] Dokumentation — `TESTPLAN-PWA.md` in Runde 0, `BEFUNDE.md` nach jeder Runde
