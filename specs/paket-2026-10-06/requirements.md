# Anforderungen — Paket 2026-10-06 (Runden P, P2, S, F, verteilte Runde K)

**Status:** Entwurf — **nicht freigegeben**. Runde P ist auf Entscheidung des Leads bereits
ausgeliefert (PWA 1.4.91, Abnahme in L324).
**Erstellt:** 2026-10-06, Stand des Arbeitsbaums `401aae2` (Zweig `pwa-decoupling`).
**Nachgeführt am selben Tag** nach der Auslieferung von P: P als ausgeliefert, Rest in P2;
Ent-3, Ent-6 und Ent-7 entschieden; B26, B32 und E30/L301 erledigt. **Danach** die Befunde
aus dem Gerätetest von 1.4.91: B40/L321 und Massnahme 4 in engerer Form nach P2, C38/L323
als Task in F, der Gerätenachweis für B36 in S.26 und der Nachweis für L303 und L306 als
Vorschau-Probe in P2.
**Auslöser:** Entscheidungen des Nutzers: Das nächste grosse Paket umfasst **P, S und F, in
dieser Reihenfolge**. U bleibt draussen; aus U wandern **B27 und B32 nach P**, B28 und B31
**warten auf den Nutzer**. Dazu eine **Kleinkram-Runde K** aus der Nachzählung vom
06.10.2026.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Es übernimmt aus
`specs/paket-2026-10-05/`, was für P, S und F weiter gilt; die alte Spec bleibt unverändert
stehen. Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** In `app.js` nennt diese Spec vorzugsweise **Funktionsnamen**: Die Datei
wurde beim Schreiben dieser Spec gerade bearbeitet, und ihre Zeilen sind dabei um rund 200
gewandert. In den Firmware-Quellen gelten die Zeilen für Stand `401aae2`.

**Kennungen.** „N1" und „N2" sind **Befunde aus der Nachzählung**. **L321 / B40**
(network_scan überschreibt Eingabe), **L323 / C38** (SSID und WLAN-Schlüssel still
gekürzt) und **L324** (Abnahme von 1.4.91) stammen aus dem Gerätetest. **L303** betrifft
`runConfirmedButtonAction`, **L306** das Umschalten von Flags. Die Entscheidungen heissen
**Ent-1 bis Ent-7**.

---

## Überblick

| Runde | Inhalt | Laufzeit | Einspielen | Testdurchlauf |
|---|---|---|---|---|
| **P** | **ausgeliefert als PWA 1.4.91** (`release/3.2.21-3.2.24-1.4.91`): B34, B35 (ohne Overlay-Text), B36, B33, B27, B26, R1–R4, dazu L304, B37/L305, B38/L306. B32 erledigt | PWA | erledigt | lesender Gerätetest gelaufen (L324); Durchlauf mit M2 offen (P.18) |
| **P2** | Rest aus P: **Overlay-Text in `TEXT_FIELD_LIMITS`**, **B40/L321**, **Massnahme 4 in engerer Form**, B17 (wenn klein); dazu die **Vorschau-Probe für L303 und L306** | PWA | nächster PWA-Upload, **vor** S | **kein eigener**, neu begründet: Proben in `tools/ui-mess/proben/` plus lesende Geräteprobe; Gerätenachweis von B35 am Zeitserver in S.26 |
| **S** | Brücke fertigbauen: L260/A42, A35 Teil 1, C26, A39, A16, **C31**, **A46**; dazu **N1** und acht STM-K-Punkte — mit **zweistufigem Flash-Gate** | ESP **und** STM | **erst ESP, dann STM** | ja, mit M2; dazu Gerätenachweis für B35 (Zeitserver) und B36 |
| **F** | Flash-Überwachung: C9c6/L179, Auswahlliste bei 65535, C28/L272, Smoketest-Stufe; **C38/L323**; dazu fünf ESP-K-Punkte | ESP + Werkzeug | **nur** ESP; ein STM-Flash als Probe | ja, mit M2 |
| **K** | Kleinkram aus der Nachzählung — **kein eigener Flash**, verteilt auf P2, S und F; Werkzeug ohne Flash; elf Punkte warten auf eine Entscheidung | je Punkt | mit dem Träger | im Durchlauf des Trägers |

**E31 / L302 ist erledigt:** Das Release `release/3.2.21-3.2.24-1.4.90` ist im Umfang des
`pwa-tester` abgenommen. **Ein Vorbehalt daraus trägt in diese Spec:** Ob die
STM-Indexguards greifen, lässt sich über HTTP nicht zeigen, weil der ESP jeden Index vorher
abweist. Die Abnahme von STM-Guards läuft deshalb über den Prüfstand (AKK.6).

---

## Problem

### 1. Die PWA hatte drei Befunde aus dem letzten Release — inzwischen ausgeliefert

**Stand:** B34, B35 und B36 sind mit PWA 1.4.91 ausgeliefert. Die Problembeschreibung bleibt
stehen, weil die Abnahmekriterien AKP.1 bis AKP.5 auf sie Bezug nehmen.

| Befund | Status vor P | Kern |
|---|---|---|
| **B35 / L287** | ✔ verifiziert am Code (`http.cpp:10351-10363`; `index.html` `maxlength`) | `maxlength` zählt **Zeichen**, `http_check_strvar_len()` seit ESP 3.2.24 **Byte**. Ein Ticker aus 32 Umlauten (64 Byte) passiert das Feld und wird vom Gerät abgewiesen |
| **B36 / L288** | ✔ verifiziert (`saveUpdateHost()`, `saveUpdatePath()`, `refreshUpdateServerAvailability()`, `runButtonRequest()`) | Nach dem Speichern lief **unbedingt** die Nachprüfung und überschrieb die Abweisung mit „abgeschlossen" |
| **B34 / L284** | ● gemeldet mit Messung; Ursache in `applyTranslations()` ✔ | Ein Sprachwechsel setzte **17** gefüllte Flächen auf ihren Platzhalter zurück |

**Ungenauigkeit in L287:** Die Befundzeile nennt `:239` (SSID von Hand), das nicht über
`http_check_strvar_len()` geht, und lässt `:405` (Datumsformat) aus. Massgeblich sind die
Endpunkte (`design.md` §1.1). ✔ Acht Endpunkte rufen `http_check_strvar_len()`:
`http.cpp:8298`, `:8326`, `:8402`, `:8427`, `:8457-8458`, `:8836`, `:8959`, `:8985`.

**Der neunte Endpunkt, offen für P2:** `http_api_overlay_set` kürzt den Overlay-Text still
(`utf8_copy_truncated`, `http.cpp:11107`; 33 Byte → 32, `{"ok":true}`, Testdurchlauf
06.10.2026). Grenze `OVERLAY_MAX_TEXT_LEN` = 32 (`vars.h:414`). **`TEXT_FIELD_LIMITS` in
PWA 1.4.91 führt den Overlay-Text nicht** — vom Lead bestätigt. PWA-Seite in P2.1,
ESP-Seite als K-Punkt in F.

### 1a. B40 / L321 — ein später Scan überschreibt die Eingabe

Am Gerät belegt um 23:58:46 (Meldung des Leads): `updateNetworkControlsFromMeta()` setzt
`network-timeserver-input.value` und `network-timezone-input.value` **ohne Bedingung**
(`app.js:3519-3520` laut Lead, Stand 1.4.91). Ausgelöst wird das nach jeder
`network_scan`-Antwort über `refreshNetworkUi()` (`app.js:2893-2897`). Kommt ein Scan
zurück, während der Nutzer tippt, steht danach wieder der alte Wert im Feld, und
„Speichern" schickt **still den alten Wert**. Für die AP-SSID ist dieselbe Falle mit
`prefillDeviceValue()` bereits gelöst (L34).

**Folge für B35:** Die Byteprüfung am Zeitserver ist erst nachgewiesen, wenn B40 behoben ist
— vorher kann der geprüfte Wert vor dem Absenden ersetzt werden.

### 1b. Massnahme 4 — der Dialog bei unveränderten Feldern

`handleDirtyFormInteraction()` (`app.js:2119` laut Lead) setzt `hasUnsavedEdits = true` bei
**jeder** Eingabe, ohne mit dem Ausgangswert zu vergleichen. Am Gerät beobachtet am
06.10.2026 (L324): tippen, den Ausgangswert vollständig wiederherstellen — beim
Modulwechsel erscheint trotzdem der Dialog „ungespeicherte Änderungen". Ein Dialog, der bei
nichts warnt, wird weggeklickt — auch dann, wenn er einmal recht hat.

### 1c. C38 / L323 — SSID und WLAN-Schlüssel werden still gekürzt

`http_api_network_client_set()` (`http.cpp:8664-8670`) und `http_api_eeprom_settings_set()`
(`:8779-8785`) übernehmen SSID und Schlüssel per `toCharArray` mit fester Länge — **zu
Langes wird still abgeschnitten**. Ein gekürzter WLAN-Schlüssel ist ein falscher Schlüssel;
das Gerät verbindet sich danach nicht mehr. Dieselbe Gattung wie C22 (L206), dort für die
anderen Zeichenkettenfelder bereits behoben.

### 1d. L303 und L306 — am Gerät nicht folgenlos nachweisbar

**L303** betrifft `runConfirmedButtonAction` (`app.js:9378` laut Lead). Darüber laufen nur
drei Aktionen: `downloadUpdateAssets`, `resetStm32` und `formatLittleFsFromFiles`. Keine
lässt sich am Gerät folgenlos zum Scheitern bringen. **L306** betrifft das Umschalten von
Flags (`toggleFlagButton`/`finishButtonFeedback`), und das scheitert nicht über einen zu
langen Text. Beide sind mit 1.4.91 ausgeliefert; ihr Nachweis braucht eine **nachgebildete
Fehlerantwort in der Vorschau** (P2.1e).

### 2. Vier Punkte aus dem E2-Review — ausgeliefert

R1 (`deleteFsFile()` mit festem Text), R2 (Layout-Restore, Kennung 6 — **mit dem Haken**,
dass `http.cpp:11545` auch „remove failed" mit Kennung 6 meldet), R3 (leerer `catch` in
`syncPersistedAmbilightState()`) und R4 (Backup-Import ohne Längenprüfung) sind mit
PWA 1.4.91 ausgeliefert. **Offen bleibt die ESP-Seite von R2:** Kennung 6 ist doppelt
belegt. Die Frage legt der Lead dem Nutzer vor (Ent-4).

### 3. Aus Runde U

- **B27 / L223** — **ausgeliefert** mit dem Wortlaut „Sekunden am Ambilight-Ring weich
  ausblenden" (Ent-3). Länger als der ursprüngliche Vorschlag, dafür eindeutig.
- **B32 / L235** — **erledigt**, die Gegenprobe ist gefahren.
- **B28 / L227** — **wartet auf eine Gestaltungsvorgabe des Nutzers.** Nicht gestrichen.
- **B31 / L242** — **wartet auf die Safari-Messung des Nutzers.** Nicht gestrichen.

### 4. Die Brücke ist weiter halb fertig — der Entwurf steht, gebaut ist nichts

- **A39 / L259** — ✔ verifiziert: `src/main.c:3217` ruft im `ESP8266_IPADDRESS`-Zweig
  weiterhin direkt `var_send_all_variables()`. Der `SYNCVARS`-Zweig (`:3243-3266`) merkt vor.
- **L260 / A42** — ✔ verifiziert: `var_sync_check()` (`ESP-uclock.ino:753`) prüft nur
  `HARDWARE_CONFIGURATION`, das dritte von rund 194 Kommandos.
- **C26 / L253** — Entscheidung des Nutzers steht (Zähler **und** Markenpflicht, `!v`).
  **Achtung:** Die ToDo-Zeile C26 führt noch „Eine Markenpflicht wäre gefährlich".
  Nachführen in S.27.
- **A35 / L233, L266** — Entwurf „Zuordnung als eigene Zeile vor der Quittung" steht.
  **Achtung:** Die ToDo-Zeile A35 führt noch „braucht eine Folgenummer". Nachführen in S.27.
- **A16 / L106** — Tetris und Snake ohne `watchdog_reload()`. **Vom Nutzer mit „Drin
  lassen" bestätigt**, einschliesslich der 60 Sekunden Spiellauf (Ent-6).

### 5. Neu seit der alten Spec, und für Runde S bestimmend

- **C31 / L298** — ✔ verifiziert: `ESP8266/ESP-uclock/eepromdata.cpp:252-253` und
  `:258-259` schreiben WLAN- und AP-Schlüssel per `Serial.println` im Klartext, ohne
  Debug-Schalter. Sie liegen im Pi-Mitschnitt. **Zusätzlich ● ungeprüft:** In einem Bau mit
  `ESP8266_DEBUG == 1` gibt der STM unbekannte Zeilen als `(…)` aus
  (`src/esp8266/esp8266.c:367-370`); ob das über die Logspiegelung (A19) in
  `/api/stm32_log` landen kann, ist offen.
- **N1 (Nachzählung), KRITISCH** — ✔ am Code bestätigt: `http_overlays()` (Legacy,
  `http.cpp:4689-4693`) liest `oidx` per `atoi`, prüft nur `oidx != 0xff` (`:4696`) und
  schreibt ab `:4710` in `overlays[oidx]`; bei `oidx == n_overlays` erhöht es zusätzlich
  `n_overlays` (`:4702-4705`). 32 Plätze. Aus dem LAN auslösbar, auch als `<img>` von
  einer fremden Seite. **Auf dem STM** trägt der Fehler weiter, wenn `n_overlays` über 32
  geht (A3, L293) — ● nicht nachgezählt, Task S.7.
- **L296** — ✔ Der F103 hat noch **1'924 Byte** frei, unter der Warnschwelle von 2'048.
  `-flto` ist gesetzt (L170); `my_gmtime` auf 32 Bit ist gesperrt (L167). **Dazu kommen
  acht STM-Punkte aus Runde K** — der Prüfer verlangt, sie **zu messen, nicht zu schätzen**.
- **L291 / A46** — ● Drei Verwerfungspfade laufen am gemeinsamen Zähler vorbei.
- **L292 / A47** — ● `u.filedata` ohne Nullbyte. Läuft zusammen mit A13 als K-Punkt.
- **C32 / L299** — ✔ Index 9 ist auf beiden Seiten etwas anderes. **Nicht in S** —
  „Nicht Teil dieser Änderung".

### 6. Runde F ist zum dritten Mal geplant und nie gefahren

- **C9c6 / L179** — ✔ Der Legacy-Zweig `action == "flash"` übernimmt den Dateinamen ohne
  Filter (`http.cpp:6969-6973`), die Legacy-Auswahlliste filtert eigenhändig
  (`:7341-7347`); der API-Endpunkt prüft über `http_remote_stm32_filename_matches()`
  (`:974-992`). Der ungeschützte Weg hat die Uhr am 04.10.2026 stillgelegt.
- **C28 / L272** — ✔ `http.cpp:7343` liest `fname + 6` ohne Längenprüfung. Fällt mit F.1
  weg; **L272 bleibt eigenständig**, F.1 schliesst sie.
- **● nicht am Gerät geprüft:** Die API-Auswahlliste (`http.cpp:12063-12071`) zeigt ohne
  Filter ebenfalls **alle** Dateien. Entscheidung Ent-5.

### 7. Der Testdurchlauf war planbar unvollständig

**L302:** Phase 3 und 4 entfielen zunächst, weil M2 fehlte — und M2 entsteht **nur mit dem
Passwort des Nutzers** (`TESTPLAN-PWA.md` §3, L300). Diese Spec plant M2 je Runde als
Nutzerschritt ein. **Dazu offen aus 1.4.91:** der Gerätenachweis für B36 — er braucht einen
Schreibaufruf, der fehlschlägt, aber nichts verändert (S.26). Für L303 und L306 gibt es
diesen Weg nicht (§1d).

### 8. Kleinkram bleibt liegen — oder wird erledigt und bleibt trotzdem offen

Die vorletzte Nachzählung fand **26** längst erledigte und weiter als offen geführte
Punkte (L235). Runde K soll viele Punkte abhaken — **mit** Abnahme je Punkt.

---

## Ziel

1. Die PWA meldet nach jeder Speicheraktion das **tatsächliche** Ergebnis, prüft **jedes**
   Textfeld vor dem Absenden in Byte — nach P2 auch den Overlay-Text —, **sendet den Wert,
   den der Nutzer eingegeben hat**, und warnt nur bei **echten** ungespeicherten Änderungen.
2. Die Brücke STM↔ESP hat kein halbes Protokoll mehr, und **kein Schlüssel geht mehr über
   die UART**.
3. Kein Schreibzugriff über den Rand von `overlays[]` mehr aus dem LAN (N1), weder auf dem
   ESP noch weitergereicht auf dem STM.
4. Der Legacy-Flashpfad kann kein fremdes Abbild mehr aufspielen, und **kein
   Zeichenkettenfeld des ESP kürzt mehr still** — auch SSID und WLAN-Schlüssel nicht.
5. Möglichst viele kleine Punkte sind **nachweislich** zu, ohne einen zusätzlichen Flash —
   und auf dem F103 nur so viele, wie **gemessen** hineinpassen.
6. Jede Runde ist **vollständig** abgenommen, einschliesslich Phase 3 und 4 — oder
   ausdrücklich als unvollständig berichtet.

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt**
(L177/L185). Wo ein Prüfstand auf dem Host läuft, gelten die angeglichenen Typen (L256).
**Was am Gerät nicht herstellbar ist, wird nicht am Gerät verlangt.**

### Runde P — ausgeliefert, und P2

**Zuordnung:** AKP.1 bis AKP.12 sind die Abnahme der ausgelieferten Runde P (Tasks P.17,
P.18). **In P2** gelten AKP.1, AKP.2 und AKP.10 für den Overlay-Text (P2.1), **AKP.13**
(B17), **AKP.14** (B40), **AKP.15** (Massnahme 4) und **AKP.16** (L303, L306). **AKP.1 für
Zeitserver und Zeitzone gilt erst mit AKP.14 als nachgewiesen.** **Den Gerätenachweis für
AKP.3 (B36)** liefert S.26 über einen Schreibaufruf, der fehlschlägt, aber nichts verändert.

- [ ] **AKP.1 (B35)** — Für **jedes** Textfeld — die acht Endpunkte mit
      `http_check_strvar_len()` **und**, ab P2, den Overlay-Text — prüft die PWA vor dem
      Absenden die Länge **in UTF-8-Byte** gegen die Grenze aus `vars.h:183-194` bzw.
      `:414`. Ein zu langer Wert wird **nicht** gesendet; die Meldung nennt Ist- und
      Höchstwert **in Byte**.
      *Instrument:* Vorschau mit 17 Umlauten (34 Byte): keine Anfrage, Meldung sichtbar;
      Gegenprobe mit 16 Umlauten: Anfrage geht hinaus. **Beide Proben Pflicht.** Dazu die
      **Zählung** aller Aufrufstellen gegen ein `grep` auf die URL-Getter (DIR-014).
      **Gerätenachweis am Zeitserver:** S.26, Phase 2.
- [ ] **AKP.2 (B35)** — Die Bytegrenzen stehen in `app.js` an **genau einer** Stelle
      (`TEXT_FIELD_LIMITS`), mit Verweis auf `vars.h`; nach P2 **zehn** Werte.
      `index.html` bleibt dafür unverändert.
      *Instrument:* Quelltext, Abgleich Zeile für Zeile im Review.
- [ ] **AKP.3 (B36)** — Nach einer **abgewiesenen** Speicherung von Update-Host oder
      -Pfad bleibt die Fehlermeldung **stehen** und wird **nicht** von einer Erfolgsmeldung
      überschrieben; nach erfolgreicher läuft die Nachprüfung.
      *Instrument:* Vorschau mit Attrappe; **am Gerät in S.26** über einen zu langen
      Update-Host, den der ESP mit `error=2` abweist — die Fehlermeldung bleibt stehen, und
      Phase 9 belegt, dass der Gerätewert unverändert ist.
- [ ] **AKP.4 (B36)** — Kein anderer Aufrufer von `runButtonRequest()` ändert sein
      Verhalten. *Instrument:* Review mit Zahl der Aufrufer.
- [ ] **AKP.5 (B34)** — Nach einem Sprachwechsel zeigt **keine** der 17 Flächen ihren
      Platzhalter, wenn sie gefüllt war; ein übersetzter Satz darf bis zur nächsten
      Aktualisierung in der alten Sprache stehen. `data-i18n` bleibt im Markup.
      *Instrument:* Vorschau; Bericht nennt alle Flächen beim Namen.
- [ ] **AKP.6 (B33)** — „Anzeigen" bei `.gz` meldet „binär" mit Grösse statt Binärdaten.
      *Instrument:* Vorschau; am Gerät `check-pwa.sh` mit Dateimodul.
- [ ] **AKP.7 (R1)** — `deleteFsFile()` zeigt den Grund des Geräts über
      `describeApiError()`; Kennung 6 nach AKP.8. *Instrument:* Vorschau. Am Gerät nicht.
- [ ] **AKP.8 (R2)** — Kennung 6 aus `fs_remove` gilt nur dann als „nichts zu tun", wenn
      die Datei im **danach** abgerufenen Verzeichnis fehlt; eine Hilfsfunktion für beide
      Aufrufer. *Instrument:* Vorschau, beide Fälle.
- [ ] **AKP.9 (R3)** — Kein leerer `catch` in `syncPersistedAmbilightState()`; ein
      Fehlschlag erscheint einmal sichtbar, nicht bei jedem Laden.
      *Instrument:* Vorschau, drei Ladezyklen.
- [ ] **AKP.10 (R4)** — Der Backup-Import prüft jedes Textfeld mit derselben Byteprüfung,
      überspringt ein zu langes **mit Namen** und schreibt die übrigen der Stufe; **ab P2
      auch die Overlay-Texte** (`importOverlaySettings()`).
      *Instrument:* Vorschau mit präparierter Sicherung. Am Gerät nicht (R5).
- [x] **AKP.11 (B27)** — Die Beschriftung von `fade-clock-seconds-button` nennt den
      Ambilight-Ring, in Deutsch und Englisch, im i18n-Text und im Rückfalltext. **Erfüllt**
      mit „Sekunden am Ambilight-Ring weich ausblenden" (PWA 1.4.91, laut Lead).
- [x] **AKP.12 (B32)** — Gegenprobe B1f/B1h gefahren. **Erfüllt** (laut Lead).
- [ ] **AKP.13 (B17, P2, nur wenn klein)** — Ursache von L142 am Gerät eingegrenzt und mit
      höchstens **einer** Funktion behoben, **oder** begründet offen. **Nicht zulässig ist
      eine Umsetzung auf Verdacht.**
      *Instrument:* Beobachtung durch den Lead (P2.2); `watch-log.sh` läuft mit.
- [ ] **AKP.14 (B40 / L321, P2)** — Ein **fokussiertes** oder **vom Nutzer geändertes**
      Zeitserver- oder Zeitzonenfeld wird von einer späten `network_scan`-Antwort **nicht**
      mehr überschrieben; ein unberührtes Feld übernimmt den Gerätewert weiterhin.
      Umgesetzt über **dieselbe** `prefillDeviceValue()`-Regel wie die AP-SSID (L34), keine
      zweite Fassung.
      *Instrument:* **Probe in `tools/ui-mess/proben/`** (P2.1c): Vorschau mit verzögerter
      `network_scan`-Antwort, drei Fälle je Feld (fokussiert, geändert und verlassen,
      unberührt); die Probe ist **einmal gegen 1.4.91 fehlgeschlagen** (DIR-014). Dazu am
      Gerät **lesend, ohne Speichern** (P2.8). **Warum nicht nur am Gerät:** Ob der Scan vor
      oder nach der Eingabe zurückkommt, hängt vom WLAN ab — der Wettlauf ist am Gerät nicht
      gezielt herstellbar, in der Vorschau schon.
- [ ] **AKP.15 (Massnahme 4, P2)** — Wer tippt und den **Ausgangswert vollständig
      wiederherstellt**, bekommt beim Modulwechsel **keinen** Dialog; eine **echte**
      Änderung löst ihn **weiterhin** aus. Der Ausgangswert ist derselbe, den
      `prefillDeviceValue()` aus AKP.14 kennt.
      *Instrument:* Vorschau, beide Fälle; am Gerät lesend in P2.8.
- [ ] **AKP.16 (L303, L306, P2)** — **L303:** Eine Formatier-Antwort mit Fehler
      (`formatLittleFsFromFiles` über `runConfirmedButtonAction`) meldet **nicht**
      „formatiert". **L306:** Nach einem fehlgeschlagenen Umschalten
      (`toggleFlagButton`/`finishButtonFeedback`) steht der Knopf wieder auf seinem
      Zustand, **nicht** auf „schaltet…".
      *Instrument:* **Probe des Leads in `tools/ui-mess/proben/`** (P2.1e) mit
      nachgebildeter Fehlerantwort in der Vorschau. Jede der beiden Proben ist **einmal
      fehlgeschlagen** gegen einen **künstlich zurückgebauten** Stand (DIR-014) — 1.4.91
      enthält die Korrekturen schon, ein Lauf dagegen bewiese nichts. Der zurückgebaute
      Stand liegt im Scratchpad, nicht im Arbeitsbaum (L268). **Nicht am Gerät:** Keine der
      drei Aktionen hinter `runConfirmedButtonAction` lässt sich folgenlos zum Scheitern
      bringen, und ein Flag-Umschalten scheitert nicht über einen zu langen Text.

### Runde S — Brücke

Übernommen aus `specs/paket-2026-10-05/requirements.md` (AKS.1 bis AKS.9) mit
aktualisierten Fundstellen; neu sind AKS.0 und AKS.10 bis AKS.14.

- [ ] **AKS.0 (Flash-Gate, zwei Stufen)** — **Stufe 1:** Vor dem ersten STM-Patch liegt
      eine Abschätzung je Teil vor (Kern und jeder STM-K-Punkt), mit Ober- und Untergrenze
      und Herleitung. Passt **schon der Kern** nicht in die Reserve (Ent-2), geht sie an den
      Nutzer. **Stufe 2:** Nach dem Kern und nach **jedem** STM-K-Punkt misst der Lead den
      F103-Stand mit einem seriellen Testbau (Ent-7). Ein Punkt, der die Reserve
      unterschreitet, wird nach der Regel aus Ent-2 behandelt. **Das Gate entscheidet
      nichts selbst.**
      *Instrument:* Bericht S.12; Testbauten S.18 und nach S.19a–S.19h; Endmessung aus S8b
      in S.23, neben die Schätzung gestellt.
- [ ] **AKS.1 (A39)** — Der `ESP8266_IPADDRESS`-Zweig ruft `var_send_all_variables()`
      **nicht** mehr direkt; er merkt vor, gesendet wird im Hauptloop, der IP-Lauftext folgt
      **nach** dem Abgleich (Variante b). *Instrument:* Quelltext.
- [ ] **AKS.2 (A39)** — Während eines ESP-Neustarts mit IP-Meldung bricht der
      Hauptloop-Zähler nicht ein (Referenz L226, zulässig rund 2 %), und **der IP-Lauftext
      läuft vollständig durch**. *Instrument:* Diagnosezeile; Lauftext **nur das Auge**.
- [ ] **AKS.3 (L260/A42)** — Mit Eröffnungszeile gilt der Abgleich erst mit der
      **Abschlussmarke** als vollständig; ohne gilt das heutige Kriterium.
      *Instrument:* Quelltext **und** Logring: die Erfolgszeile erscheint erst **nach** der
      Marke.
- [ ] **AKS.4 (A35)** — Die Zuordnung geht als **eigene Zeile vor** jeder Quittung
      (`ACK <xy>`, dann `.` bzw. `!v`). *Instrument:* Quelltext beider Seiten.
- [ ] **AKS.5 (A35)** — Eine unpassende Quittung wird **verworfen** und **gezählt**; A32
      sendet **einmal** nach. Eine gemerkte Zuordnung ohne Quittung wird **verbraucht**.
      *Instrument:* Prüfstand unter `tools/checks/`, Typen angeglichen.
- [ ] **AKS.6 (Rückfall)** — **Neuer ESP gegen alten STM** läuft wie heute: Vollabgleich
      ohne Timeout, ohne Reset, `var_send_timeout_cnt` steigt nicht.
      *Instrument:* Zwischenabnahme S.11.
- [ ] **AKS.7 (C26)** — Der Zähler „unmarkiert nach erster Marke" ist **über den Logring**
      ablesbar. *Instrument:* `/api/stm32_log`, nicht der serielle Mitschnitt.
- [ ] **AKS.8 (C26)** — Eine **unmarkierte** Zeile der drei pflichtigen Kommandoarten wird
      bei belegter Markierung **nicht übernommen** und mit **`!v`** abgewiesen; Liste
      **aufgezählt**; im Normalbetrieb **keine** Abweisung.
      *Instrument:* Prüfstand; am Gerät „null Abweisungen" über den Logring.
- [ ] **AKS.9 (A16)** — Tetris und Snake bedienen den Watchdog; ein Spiel läuft über
      60 Sekunden ohne Reset. *Instrument:* Quelltext **und** Lauf durch den **Nutzer**
      (S.24, Ent-6). Ohne diesen Lauf **nicht erfüllt**.
- [ ] **AKS.10 (C31)** — `eepromdata.cpp` gibt für beide Schlüssel nur **gesetzt** oder
      **leer** aus. *Instrument:* Quelltext und Mitschnitt nach dem ESP-OTA (S.11) sowie
      `/api/stm32_log`. **Geprüft wird die Form der Zeile, nicht die Abwesenheit des
      Schlüssels.**
- [ ] **AKS.11 (C31, Zweitweg)** — Am Code beantwortet, ob eine unbekannte ESP-Zeile über
      die STM-Logausgabe in den ESP-Logring gelangen kann; bei ja eigener Befund.
      *Instrument:* Bericht S.7.
- [ ] **AKS.12 (A47, mit A13)** — Alle Zweige, die `u.filedata` per `strncpy` füllen,
      terminieren den Puffer. *Instrument:* Quelltext, `grep -a`.
- [ ] **AKS.13 (A46)** — Die drei Verwerfungspfade aus L291 laufen über den gemeinsamen
      Zähler. *Instrument:* Quelltext und `tools/checks/htoi-laenge.c` (S13). Unter dem Gate.
- [ ] **AKS.14 (N1)** — `http_overlays()` prüft `oidx < MAX_OVERLAYS && oidx <= n_overlays`
      **vor** dem ersten Schreibzugriff und vor `n_overlays++`.
      *Instrument:* Prüfstand mit den Typen des Ziels: `oid32`, `oid33`, `oid300`
      abgewiesen, `oid<n_overlays>` legt an. **Nicht am Gerät.** Lesend nur: Die
      Legacy-Overlayseite lädt (S.11).

### Runde F — Flash-Überwachung

- [ ] **AKF.1 (C9c6)** — Der Legacy-Flashzweig prüft mit
      `http_remote_stm32_filename_matches()` **vor** jeder Flash-Aktion. **Dieselbe
      Funktion.** *Instrument:* `grep`. **Nicht scharf fahren.**
- [ ] **AKF.2 (C28/L272)** — Die Legacy-Auswahlliste filtert über dieselbe Funktion; das
      ungeprüfte `fname + 6` ist weg; L272 wird als erledigt geführt. *Instrument:* Quelltext.
- [ ] **AKF.3 (65535)** — Bei 65535 bietet die Legacy-Liste **keine** Datei an und nennt
      `./tools/flash-stm.sh`; über `stm32_log_append()` unter **120** Zeichen (L257).
      *Instrument:* Quelltext; im gesunden Zustand lesend. 65535 wird **nicht** hergestellt.
- [ ] **AKF.4 (Smoketest)** — `smoke-device.sh` meldet 65535 als Fehlschlag.
      *Instrument:* Gegenprobe gegen festen Testwert (DIR-014).
- [ ] **AKF.5 (Probe)** — STM-Flash über `flash-stm.sh` nach dem F-OTA, erwartete Version
      in `/api/update_status`. *Instrument:* `flash-stm.sh` (DIR-010).
- [ ] **AKF.6 (C38 / L323)** — `http_api_network_client_set()` und
      `http_api_eeprom_settings_set()` **weisen** eine zu lange SSID bzw. einen zu langen
      Schlüssel ab, statt sie still zu kürzen — über `http_check_strvar_len()` oder dieselbe
      Byte-Regel, keine zweite.
      *Instrument:* **Nur Quelltext und Prüfstand** (F.5h): zu lang ⇒ Abweisung, Grenzwert
      ⇒ angenommen; einmal fehlgeschlagen gegen die alte Fassung (DIR-014).
      **Kein schreibender WLAN-Aufruf am Gerät** — `network_client_set` bleibt für jeden
      Gerätetest gesperrt; ein falscher Schlüssel trennte die Uhr vom Netz. Dazu im Review
      (F.6): Kann die PWA mehr Byte senden, als der ESP annimmt? Wenn ja, folgt eine
      PWA-Grenze **nach** dem F-OTA (L241).

### Runde K — Kleinkram

Die Abnahmesätze **je Punkt** stehen in der K-Tabelle in `tasks.md`.

- [ ] **AKK.1** — Jeder K-Punkt hat **vor** seiner Umsetzung einen Satz „erledigt, wenn …"
      mit Instrument.
- [ ] **AKK.2** — Die Zahl der K-Punkte in den Tabellen entspricht der Zahl aus der
      Nachzählung; jeder hat genau einen Träger, ist erledigt, oder wartet ausdrücklich auf
      eine Entscheidung. *Instrument:* Abzählung K.0.
- [ ] **AKK.3** — Wird ein K-Punkt grösser als rund zehn Zeilen oder wirft er eine
      Entwurfsfrage auf, wird er **umgestuft und zurückgemeldet**.
- [ ] **AKK.4** — Jeder geschlossene K-Punkt trägt einen Beleg **dieses Commits** oder einen
      Messwert mit Zeitstempel; kein Verweis auf einen anderen Befund.
      *Instrument:* Stichprobe von fünf Zeilen durch den Lead.
- [ ] **AKK.5** — Jeder STM-K-Punkt ist **gemessen**, ob er bleibt oder zurückgestellt wird.
- [ ] **AKK.6 (STM-Guards)** — Indexguards auf dem STM (A3+A48, A24, und die schon
      ausgelieferten aus A43/A44) werden über den **Prüfstand** abgenommen; S15
      (`tools/checks/idx-guard.sh`) deckt danach auch die neuen Guards ab.
      *Instrument:* S15-Lauf mit Fallzahl, einmal fehlgeschlagen (DIR-014). **Begründung:**
      Über HTTP ist ein STM-Guard nicht zu zeigen, weil der ESP jeden Index vorher abweist.

### Für jede Runde

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `./tools/guardrails.sh --full` vor dem Release mit Exit 0 (Lead, R1).
- [ ] **AKZ.3** — `./tools/smoke-device.sh` nach jedem Einspielen ohne Fehlschlag
      (DIR-009). **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.4** — **Ein `pwa-tester`-Durchlauf je Runde über die Phasen 0 bis 4 und 9**,
      gegen den Endstand der Runde, **nach** M2 des Nutzers. Unvollständig ⇒ **nicht
      abgenommen**. **Ausnahme P2, neu begründet** (`tasks.md`, Runde P2): Seit P2 echte
      Logik enthält (B40, Massnahme 4), tragen die Proben P2.1c (Wettlauf in der Vorschau)
      und P2.1e (nachgebildete Fehlerantworten für L303, L306), beide einmal
      fehlgeschlagen, sowie die lesende Geräteprobe P2.8; der volle Durchlauf S.26 läuft
      gegen einen Stand mit P2 und prüft B35 am Zeitserver in Phase 2. **Offen bleibt**
      zwischen P2-Upload und S.26 ein Gerätenachweis **mit Speichern** — der Ausfall wäre
      der heutige Fehler, kein neuer.
- [ ] **AKZ.5** — `./tools/watch-log.sh` läuft bei jedem Gerätelauf mit und wird mitgelesen
      (DIR-013).
- [ ] **AKZ.6** — Jedes ausgerollte Release **sofort** committet, getaggt, **gepusht**
      (DIR-011), je Einspielschritt einzeln.
- [ ] **AKZ.7** — Die Einspielzeile nennt die **Reihenfolge** (L241, DIR-002) und was sich
      sichtbar ändert, **fertig zum Einfügen**.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **Runde U: B28, B31** | **Wartet auf den Nutzer**, nicht gestrichen: B28 auf eine Gestaltungsvorgabe, B31 auf die Safari-Messung |
| **B24 / L208** | Erledigt, beim Nachzählen am 04.10.2026 belegt |
| **Weitere Felder in `updateNetworkControlsFromMeta()`** | P2.1b behebt Zeitserver und Zeitzone. Haben weitere Felder dieselbe Form, meldet der `pwa-developer` sie — **nicht mitkorrigieren**, der Gerätetest hat nur diese beiden belegt |
| **Ein Gerätenachweis für L303 und L306 mit echtem Fehlschlag** | Keine der drei Aktionen hinter `runConfirmedButtonAction` lässt sich folgenlos zum Scheitern bringen; ein Flag-Umschalten scheitert nicht über einen zu langen Text. Nachweis über die Probe P2.1e |
| **Schreibende WLAN-Aufrufe am Gerät** | Auch für C38 nicht. `network_client_set` bleibt gesperrt; Abnahme am Quelltext und am Prüfstand |
| **C32 / L299 — Ambilight-Flag, Index 9** | **Nicht in S, bewusst.** (1) S ändert das **Protokoll**, C32 die **Bedeutung eines Variablenindex** — in einem Flash nicht zuzuordnen. (2) Offene Entwurfsfrage: Wo lebt die Wahrheit? (3) Der F103 ist mit S und K ausgeschöpft. (4) Keine Regression, kein Sicherheitsbefund. **Festgehalten:** `applyPersistedAmbilightState()` überdeckt C32 per `localStorage` — eine Umgehung an der falschen Schicht, beim Lösen von C32 zurückzubauen. **Kein K-Punkt** — er hat eine Entwurfsfrage |
| **A35 Teil 2** | Rückgabewert aus `var_set_parameter()`, Eingriff in jeden `switch`-Zweig |
| **C6b / L22** | Klartextschlüssel in `/api/eeprom_settings`; eigene Abwägung, weil der Export den Wert braucht |
| **C19 / L199** — Legacy biegt still | Dazu gehören `action=saveuphost` ohne Längenprüfung (`http.cpp:6961-6964`) und das Kürzen im Legacy-Overlayformular (`:4774`). N1 und F fassen dieselben Handler an; **nicht mitkorrigieren** |
| **Kennung 6 doppelt belegt auf dem ESP** (`http.cpp:11545`) | Die PWA-Seite ist mit R2 robust gelöst. Ob der ESP eine eigene Kennung bekommt, legt der Lead dem Nutzer vor (Ent-4) |
| **Löschen der Pi-Logdateien mit Schlüsseln** (L298) | Entscheidung und Handlung des Nutzers |
| **K-Punkte mit Entwurfsfrage oder über rund zehn Zeilen** | Gehören nicht in K (AKK.3) |

---

## Betroffene Laufzeiten

- [x] **PWA** — P ausgeliefert (1.4.91); P2 als nächster LittleFS-Upload, **vor** S
- [x] **ESP8266** — Runden S (mit N1) und F (mit C38 und fünf K-Punkten), je ein OTA
- [x] **STM32** (`src/**`) — Runde S, ein Flash (mit gemessenen STM-K-Punkten); Runde F
      flasht denselben Stand als Probe
- [x] **Werkzeug** (`tools/**`) — Proben B40 und L303/L306 (P2), Prüfstände und S15 (S),
      Prüfstand C38 und Smoketest (F), K-Punkte W
- [x] **Dokumentation** — `BEFUNDE.md` je Runde
- [x] **Build/Release** — je Einspielschritt ein Release-ZIP, Rollout, Commit, Tag, Push;
      dazu **serielle Testbauten** F103 ohne Release (S)

---

## Entscheidungen

- **Ent-1 — Freigabe dieser Spec.** Offen. Runde P ist auf Entscheidung des Leads bereits
  ausgeliefert; die Freigabe gilt für P2, S, F und K.
- **Ent-2 — Flashreserve, Reihenfolge und Regel für den F103.** Offen.
  - **Reserve:** Vorschlag **mindestens 1'024 Byte** frei nach S.
  - **Reihenfolge der STM-K-Punkte**, nach Schaden: A20 → A13+A47 → A3+A48 → A24 → A45 →
    A14 → A46 → A49.
  - **Regel bei Unterschreitung:** Vorschlag — bei A20, A13+A47 und A3+A48 **anhalten und
    vorlegen**; bei den übrigen **zurückstellen und weitermachen**, alles im Bericht S.23.
  - Kern und A16 stehen nicht zur Disposition; passt schon der Kern nicht, legt S.12 das vor.
- **Ent-3 — Wortlaut für B27. Erledigt:** „Sekunden am Ambilight-Ring weich ausblenden",
  ausgeliefert mit PWA 1.4.91.
- **Ent-4 — Kennung 6 auf dem ESP trennen?** Offen; der Lead legt die Frage dem Nutzer vor.
  Bei ja: Task F.2.
- **Ent-5 — API-Auswahlliste bei 65535 mitziehen?** Offen. Empfehlung: ESP-Teil in F, die
  PWA-Frage in den F-Review.
- **Ent-6 — A16 samt 60 Sekunden Spiellauf. Entschieden:** Der Nutzer hat A16 mit „Drin
  lassen" bestätigt, also auch den Spiellauf nach dem STM-Flash.
- **Ent-7 — Serielle Testbauten zwischen den STM-Tasks. Entschieden vom Lead: ja.**
  Begründung: R1 soll verhindern, dass parallel gebaut wird — das bleibt gewahrt, weil nur
  der Lead baut, nur seriell und nur zwischen abgeschlossenen Tasks. Der Wortlaut „einmal
  am Ende" stammt aus einer Zeit ohne Flash-Gate; mit einem Gate, das je K-Punkt messen
  soll, ist er nicht mehr der Zweck der Regel.
