# Erweiterungen — geplante Features

Hier stehen **neue Funktionen**, die bewusst geplant, aber noch nicht beauftragt sind. Sie sind
getrennt von `BEFUNDE.md`: Dort stehen Mängel und Verbesserungen am Bestand, jeweils mit Beleg und
Status, und die ToDo-Liste dort ordnet die laufende Arbeit. Ein Feature hier ist kein Befund und
drängt sich nicht in diese Liste. Wird eines beauftragt, bekommt es eine Spec unter `specs/` und
wandert damit in die Arbeit; hier bleibt der Eintrag mit Verweis darauf stehen.

Kennung `X<n>`, fortlaufend, wird nie neu vergeben. Status: **Idee** (aufgenommen, nicht bewertet),
**geplant** (Abhängigkeiten und Rahmen geklärt), **beauftragt** (Spec vorhanden), **erledigt**,
**verworfen** (mit Grund).

| | Erweiterung | Status | Hängt ab von | Betrifft |
|---|---|---|---|---|
| **X1** | **Live-Display der echten LEDs in der PWA** | Idee | C65 (Heap des ESP unter PWA-Last) | STM, ESP, PWA |
| **X2** | **Ambilight: Haken „Use Display Brightness“** — unterschiedliche Farben, aber gleiche Helligkeit | Idee | — | STM, ESP, PWA |
| **X3** | **Hintergrundfarbe/-helligkeit für das Ambilight** | Idee | — | STM, ESP, PWA |
| **X4** | **Daylight-Farbanimation mit fliessendem Übergang** | Idee | — | STM |
| **X5** | **Neues Overlay „Time“** — lässt „Es ist hh:mm Uhr“ als Ticker durchlaufen | Idee | Flash-Reserve STM | STM, PWA |
| **X6** | **Overlays mit Uhrzeitbereichen** — neben von–bis-Daten auch z. B. 10:00–12:30 | Idee | Layout-Version EEPROM | STM, ESP, PWA |
| **X7** | **Daylight-Tabelle über das Webinterface änderbar** | Idee | X4 sinnvoll vorher | STM, ESP, PWA |
| **X8** | **Rainbow-Animation für SK6812 (Weiss) optimiert** | Idee | — | STM |
| **X9** | **Harlekin-Farbanimation** — jeder Buchstabe hat eine eigene Farbe | Idee | Flash-Reserve STM | STM, PWA |
| **X10** | **Farbanimation „jedes Wort eine eigene Farbe“**, evtl. mit Regenbogeneffekt | Idee | Flash-Reserve STM | STM, PWA |
| **X11** | **Neue Animation „Implode“** | Idee | — | STM, PWA |
| **X12** | **Mood-Light-Funktion** | Idee | — | STM, ESP, PWA |
| **X13** | **Hintergrundfarbe/-helligkeit für das Display** | Idee | X3 gemeinsam planen | STM, ESP, PWA |
| **X14** | **Legacy-Oberfläche durch eine kleine Notfallseite ersetzen** | Idee | C66, C70; Funktionsabgleich PWA ↔ Legacy | ESP |

## X1 — Live-Display der echten LEDs in der PWA

Aufgenommen am 14.10.2026, Idee des Nutzers.

**Heute:** Die WordClock-Vorschau der PWA ist nachgebildet. Sie errechnet die Wörter selbst aus der
Layout-Tabelle (`layout-previews.json`) und der Gerätezeit; die Farbe holt sie periodisch vom Gerät
(`liveDisplayColorTimer`). Sie zeigt also, was leuchten *müsste* — nicht, was die Uhr tatsächlich
anzeigt.

**Ziel:** Die PWA zeigt die Anzeige nach den LEDs, die wirklich an sind. Damit wären Ticker, Icons,
Overlays, das Nachtfenster und vor allem ein Hänger in der Oberfläche sofort sichtbar — wertvoll für
die Diagnose (siehe die sporadischen Hänger in `CLAUDE.md`, „Offene technische Themen“).

**Randbedingungen aus dem Bestand:**

- Die UART-Brücke ist der einzige vollwertige UART des ESP. Jede Dauerlast darauf zählt, auch wenn
  niemand die Oberfläche offen hat.
- Der Heap des ESP ist unter PWA-Last schon knapp (`BEFUNDE.md`, C65, L449). Eine weitere periodische
  Abfrage kostet genau die Verbindungen, die heute fehlen. Deshalb erst nach C65.
- Naheliegend: Abruf auf Anforderung oder kompaktes Format (Wortindizes statt 288 einzelner LEDs)
  statt eines laufenden Stroms.

**Offen für die Spec:**

- Umfang: ganzes Panel einschliesslich Ticker und Icons, oder nur „welche Wörter sind an“?
- Takt: laufend, solange die Seite offen ist, oder auf Knopfdruck?
- Wo das Abbild entsteht (STM sendet Rohzustand, ESP verdichtet, PWA zeichnet) und was es an Flash
  auf der engsten STM-Variante kostet (Reserve über dem Gate siehe `BEFUNDE.md`, L435).

## X14 — Legacy-Oberfläche durch eine kleine Notfallseite ersetzen

Aufgenommen am 14.10.2026 auf Wunsch des Nutzers, nach der Frage, was der Verzicht auf die
Legacy-Oberfläche an Heap und Speicher brächte.

**Was es brächte (Stand 14.10.2026):**

- **STM:** nichts — die Weboberfläche liegt vollständig auf dem ESP.
- **ESP-Flash:** rund 24 KB Code der Legacy-Seitenfunktionen (gemessen am Build E3 über die
  Symbolgrössen: `http_main`, `http_update`, `http_fs`, `http_network`, `http_display`,
  `http_overlays` …) plus deren Texte im Flash, grob 40–60 KB. Der Flash des ESP ist kein Engpass.
- **ESP-RAM statisch:** wenig. Die Seiten sind fast durchgehend mit `F()`/`FS()` gebaut, ihre Texte
  liegen im Flash. Geschätzt höchstens 1–2 KB, **nicht gemessen** — vor einer Entscheidung den
  Anteil an `.rodata`/`.data` je Funktion bzw. Objekt zuordnen.
- **ESP-Heap zur Laufzeit:** praktisch nichts. Die Legacy-Seite belegt Heap nur, wenn sie
  aufgerufen wird. Das Tief beim Laden der PWA (C65) entsteht durch wartende Verbindungen,
  WLAN-Scan, TIME_WAIT und Sendepuffer (`BEFUNDE.md`, L456) — daran ändert X14 nichts.

**Was verloren ginge, ohne Ersatz:**

- **Der Rettungsweg.** Die PWA lebt als `.gz` im LittleFS; ist dort etwas kaputt, bleibt der Schirm
  weiss (Architektur-Invariante in `CLAUDE.md`; C66 zeigt einen heute noch offenen Weg dorthin).
  Die Legacy-Seite ist fest in der Firmware und funktioniert dann noch.
- **Der Umstieg vom Original 3.1.5**, den die Release Note genau über die Legacy-Seite führt.
- Funktionen, die es nur dort gibt — ein Abgleich PWA ↔ Legacy steht aus.

**Ansatz:** Statt die Legacy-Oberfläche ganz zu streichen, eine kleine, fest eingebaute
**Notfallseite**: Status und Versionen, Update-Host und -Pfad, ESP-OTA, STM-Flash,
„App installieren“, STM-Reset. Das bringt den grössten Teil der Flash-Ersparnis, behält den
Rettungsweg und hält den Code, der ohne PWA laufen muss, klein und prüfbar. Erst sinnvoll, wenn
die PWA selbst robust genug ist: nach **C66** (Teildatei beim Download) und **C70** (Asset-Cache).
Berührt die Architektur-Invariante „Legacy läuft parallel zur PWA“ — vor einer Spec mit dem
Nutzer entscheiden.

## X2 bis X13 — aus der Liste „Weitere geplante Features“ des Ursprungsprojekts

Aufgenommen am 14.10.2026 auf Wunsch des Nutzers, im Wortlaut der Homepage des
Ursprungsprojekts. Je Eintrag steht, was der Code heute dazu schon hat (geprüft am Stand
dieses Tages); daraus ergibt sich der Ansatz, nicht die Spec.

**Gemeinsame Randbedingung für alles, was den STM berührt:** Die engste STM-Variante
(`wc24h-stm32f103-sk6812-rgbw`) hat über dem Flash-Gate nur noch wenig Reserve (`BEFUNDE.md`,
L435: 148 Byte über dem Gate). Neue Animationen und Overlay-Typen brauchen vorher eine Entscheidung
zwischen drei Wegen: **nur für den F411** (Compiler-Schalter, der F103 bleibt beim heutigen Umfang),
**vorher Platz schaffen** oder **F103-Varianten einfrieren** (nur noch Fehlerkorrekturen). Die
Grundlage dafür liefert die Flash-Analyse `BEFUNDE.md`, A85/L451.

- **X2 Ambilight „Use Display Brightness“.** Heute gibt es nur den vollständigen Abgleich
  `DISPLAY_FLAGS_SYNC_AMBILIGHT`: `sync_colors()` (`src/display/display.c`) übernimmt Farbe **und**
  Helligkeit gemeinsam. Gewünscht ist ein zweiter, getrennter Merker nur für die Helligkeit.
- **X3 Hintergrundfarbe/-helligkeit für das Ambilight.** Nichts vorhanden. Nicht leuchtende
  Ambilight-LEDs in einer schwachen Grundfarbe. Mit X13 gemeinsam planen (gleiches Prinzip).
- **X4 Daylight mit fliessendem Übergang.** Heute springt die Farbe einmal je Stunde: Tabellen
  `daylight_red/green/blue[24]` in `display.c`, Wechsel bei `gmain.hour != last_hour`, getrennt
  für Display und Ambilight. Gewünscht ist Interpolation zwischen den Stundenwerten.
- **X5 Overlay „Time“.** Die Overlay-Typen 0–10 (`src/overlay/overlay.h`) kennen keine Uhrzeit;
  das Datumsformat des Tickers kennt nur Tag, Monat, Jahr. Neuer Typ mit Ticker „Es ist hh:mm
  Uhr“; sprachabhängig (Layout-Tabellen), also Text je Sprache klären.
- **X6 Overlays mit Uhrzeitbereich.** Ein Overlay-Eintrag hat heute Typ, Intervall, Dauer,
  Datumscode, Startdatum, Tage und Merker (`overlay.h`) — keine Uhrzeit. Braucht neue Felder im
  EEPROM (Layout-Version, Rückflash und Backup beachten: `BEFUNDE.md`, F-Reihe „Backup
  versionsfest“).
- **X7 Daylight-Tabelle im Webinterface.** Die Tabelle ist im STM fest eingebaut; die PWA hält
  nur eine eigene Kopie für die Vorschau (`DAYLIGHT_RED/GREEN/BLUE`, `getDaylightPreviewColor`
  in `app.js`). Braucht Setter, EEPROM-Platz (3 × 24 Byte) und Aufnahme ins Backup (S17).
- **X8 Rainbow für SK6812 mit Weiss.** Die Rainbow-Phasen bilden Weiss heute aus RGB, der
  Weisskanal bleibt 0. Gewünscht: den Weisskanal in der Animation nutzen.
- **X9 Harlekin.** Nichts vorhanden. Heute gibt es drei Farbanimationen fürs Display: None,
  Rainbow, Daylight (`display.h`).
- **X10 Jedes Wort eine eigene Farbe.** Nichts vorhanden; mit X9 verwandt (Farbe je Buchstabe
  bzw. je Wort statt je Panel). Gemeinsam planen.
- **X11 Animation „Implode“.** Teilweise vorhanden: `display_animation_calc_implode()` in
  `display.c` existiert, dient heute aber nur der Animation „Explode“ (die Implosion wird
  rückwärts abgespielt). Ein eigener Modus fehlt — der Grossteil der Rechnung ist also schon da.
  Heute gibt es 13 Animationen: None, Fade, Roll, Explode, Random, Snake, Teletype, Cube,
  GreenMatrix, Drop, Squeeze, Flicker, Matrix.
- **X12 Mood-Light.** Nichts vorhanden. Ganzes Panel (und Ambilight) als Stimmungslicht; Umfang
  (feste Farbe, Verlauf, Zeitsteuerung) in der Spec klären. Berührt die Display-Zustandsmaschine
  und damit die Restore-Bedingung (`CLAUDE.md`, Architektur-Invarianten).
- **X13 Hintergrundfarbe/-helligkeit für das Display.** Nichts vorhanden. Mit X3 gemeinsam.

### Aus derselben Liste bereits umgesetzt

Nicht mehr als Erweiterung geführt — geprüft am 14.10.2026:

- **Negative Korrekturwerte für die Temperatur:** ±20 °C in Schritten zu 0,5 °C für RTC und
  DS18xx, durchgängig in STM (`RTC_TEMP_CORRECTION_LIMIT`, `TEMP_CORRECTION_LIMIT`), ESP
  (`temperature_correction_set`, Legacy-Felder) und PWA.
- **STM32-Logbuch im Webinterface:** `/api/stm32_log` und `/api/stm32_log_clear`, in der PWA das
  Logbuch-Panel im Modul System.
- **Export/Import der Einstellungen:** Backup-Export und -Import der PWA (Einstellungen, Netzwerk,
  IR-Codes). In der Legacy-Oberfläche gibt es das nicht.
