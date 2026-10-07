# Anforderungen — Paket 2026-10-06 (Runden P, P2, S, P3, F, C4, verteilte Runde K)

**Status: freigegeben am 07.10.2026** (Ent-1) für P2, S, F und K. Runde P ist auf
Entscheidung des Leads bereits ausgeliefert (PWA 1.4.91, Abnahme in L324).
**Erstellt:** 2026-10-06, Stand des Arbeitsbaums `401aae2` (Zweig `pwa-decoupling`).
**Nachgeführt:** am 06.10.2026 nach der Auslieferung von P und dem Gerätetest von 1.4.91;
am **07.10.2026** mit den Entscheidungen des Nutzers — darunter **A5 als Protokolländerung**
(echte Minusgrade), **E17 mit S**, **C4 als eigener Schritt**, **B19** und **E2** als
Werkzeug, **P3** als neue kleine PWA-Runde für A5.
**Auslöser:** Entscheidungen des Nutzers: Das nächste grosse Paket umfasst **P, S und F, in
dieser Reihenfolge**. U bleibt draussen; aus U wandern **B27 und B32 nach P**, B28 und B31
**warten auf den Nutzer**. Dazu eine **Kleinkram-Runde K** aus der Nachzählung vom
06.10.2026.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Es übernimmt aus
`specs/paket-2026-10-05/`, was für P, S und F weiter gilt; die alte Spec bleibt unverändert
stehen. Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** In `app.js` nennt diese Spec vorzugsweise **Funktionsnamen**: Die Datei
wurde beim Schreiben dieser Spec gerade bearbeitet, und ihre Zeilen sind dabei um rund 200
gewandert. In den Firmware-Quellen gelten die Zeilen für Stand `401aae2`. **`vars.h` ohne
Pfad meint `ESP8266/ESP-uclock/vars.h`**; die STM-Datei heisst ausdrücklich
`src/vars/vars.h`. (Eine frühere Fassung nannte `vars.h:414` ohne Pfad — gemeint war die
ESP-Datei.)

**Kennungen.** „N1" und „N2" sind **Befunde aus der Nachzählung**. **L321 / B40**
(network_scan überschreibt Eingabe), **L323 / C38** (SSID und WLAN-Schlüssel still
gekürzt) und **L324** (Abnahme von 1.4.91) stammen aus dem Gerätetest. **L303** betrifft
`runConfirmedButtonAction`, **L306** das Umschalten von Flags. Die Entscheidungen heissen
**Ent-1 bis Ent-8**.

---

## Überblick

| Runde | Inhalt | Laufzeit | Einspielen | Testdurchlauf |
|---|---|---|---|---|
| **P** | **ausgeliefert als PWA 1.4.91** (`release/3.2.21-3.2.24-1.4.91`): B34, B35 (ohne Overlay-Text), B36, B33, B27, B26, R1–R4, dazu L304, B37/L305, B38/L306. B32 erledigt | PWA | erledigt | lesender Gerätetest gelaufen (L324); Durchlauf mit M2 offen (P.18) |
| **P2** | Rest aus P: **Overlay-Text in `TEXT_FIELD_LIMITS`**, **B40/L321**, **SSID-Auswahl**, **Massnahme 4 in engerer Form**, B17 (wenn klein); dazu die **Vorschau-Probe für L303 und L306** | PWA | nächster PWA-Upload, **vor** S | **kein eigener**, begründet: Proben plus lesende Geräteprobe; Gerätenachweis von B35 am Zeitserver in S.26 |
| **S** | Brücke fertigbauen: L260/A42, A35 Teil 1, C26, A39, A16, **C31**, **A46**, **E17** (reine Umbenennung), **A5** (STM- und ESP-Teil); dazu **N1** und neun STM-K-Punkte — mit **zweistufigem Flash-Gate**, Reserve 1'024 Byte | ESP **und** STM | **erst ESP, dann STM** | ja; dazu Gerätenachweis für B35 (Zeitserver) und B36 |
| **P3** | **A5, PWA-Teil**: Minusgrade anzeigen, mit Rückfall | PWA | **nach** S.24 | kein eigener; F.12 läuft gegen einen Stand mit P3 |
| **F** | Flash-Überwachung: C9c6/L179, Auswahlliste bei 65535, C28/L272, Smoketest-Stufe; **C38/L323**, **Ent-4**, **Ent-5**; dazu sieben ESP-K-Punkte (mit C23 und C6u) | ESP + Werkzeug | **nur** ESP; ein STM-Flash als Probe | ja |
| **C4** | Umschrift der acht ISO-8859-1-Dateien unter `src/**` auf ASCII | STM-Quellen | **kein Flash** — Nachweis über gleiche `.hex` | — |
| **K** | Kleinkram aus der Nachzählung — **kein eigener Flash**, verteilt auf P2, S und F; Werkzeug ohne Flash (mit B19 und E2); C2 und C9c4 geschlossen; E3 wartet | je Punkt | mit dem Träger | im Durchlauf des Trägers |

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
06.10.2026). Grenze `OVERLAY_MAX_TEXT_LEN` = 32 (**`ESP8266/ESP-uclock/vars.h:414`**).
**`TEXT_FIELD_LIMITS` in PWA 1.4.91 führt den Overlay-Text nicht** — vom Lead bestätigt.
PWA-Seite in P2.1, ESP-Seite als K-Punkt in F.

### 1a. B40 / L321 — ein später Scan überschreibt die Eingabe

Am Gerät belegt um 23:58:46 (Meldung des Leads): `updateNetworkControlsFromMeta()` setzt
`network-timeserver-input.value` und `network-timezone-input.value` **ohne Bedingung**
(`app.js:3519-3520` laut Lead, Stand 1.4.91). Ausgelöst wird das nach jeder
`network_scan`-Antwort über `refreshNetworkUi()` (`app.js:2893-2897`). Kommt ein Scan
zurück, während der Nutzer tippt, steht danach wieder der alte Wert im Feld, und
„Speichern" schickt **still den alten Wert**. Für die AP-SSID ist dieselbe Falle mit
`prefillDeviceValue()` bereits gelöst (L34).

**Dasselbe Muster an der SSID-Auswahl** (Meldung des Leads vom 07.10.2026):
`network-ssid-select` (`app.js` um Zeile 3550) wird bei jedem Scan per `innerHTML` neu
aufgebaut, mit `selected` auf der Geräte-SSID. Ein später Scan setzt eine ungespeicherte
Auswahl zurück (P2.1f).

**Folge für B35:** Die Byteprüfung am Zeitserver ist erst nachgewiesen, wenn B40 behoben ist.

### 1b. Massnahme 4 — der Dialog bei unveränderten Feldern

`handleDirtyFormInteraction()` (`app.js:2119` laut Lead) setzt `hasUnsavedEdits = true` bei
**jeder** Eingabe, ohne mit dem Ausgangswert zu vergleichen. Am Gerät beobachtet am
06.10.2026 (L324): tippen, den Ausgangswert vollständig wiederherstellen — beim
Modulwechsel erscheint trotzdem der Dialog. Ein Dialog, der bei nichts warnt, wird
weggeklickt — auch dann, wenn er einmal recht hat.

### 1c. C38 / L323 — SSID und WLAN-Schlüssel werden still gekürzt

`http_api_network_client_set()` (`http.cpp:8664-8670`) und `http_api_eeprom_settings_set()`
(`:8779-8785`) übernehmen SSID und Schlüssel per `toCharArray` mit fester Länge — **zu
Langes wird still abgeschnitten**. Ein gekürzter WLAN-Schlüssel ist ein falscher Schlüssel.

### 1d. L303 und L306 — am Gerät nicht folgenlos nachweisbar

**L303** betrifft `runConfirmedButtonAction` (`app.js:9378` laut Lead): `downloadUpdateAssets`,
`resetStm32`, `formatLittleFsFromFiles`. Keine lässt sich am Gerät folgenlos zum Scheitern
bringen. **L306** betrifft `toggleFlagButton`/`finishButtonFeedback`; ein Umschalten
scheitert nicht über einen zu langen Text. Nachweis über eine **nachgebildete
Fehlerantwort in der Vorschau** (P2.1e).

### 1e. A5 / L38 — Minusgrade gibt es heute nicht

✔ am Code, Stand `401aae2`: `rtc_get_temperature_index()` (`src/rtc/rtc.c:352-384`) liest das
Temperaturregister des DS3231 und rechnet `(buffer[0] << 1) | …` — `buffer[0]` ist
`uint8_t`, beim DS3231 aber **vorzeichenbehaftet**. Seit L27 wird das Ergebnis auf 0..250
**begrenzt**: Unter 0 °C meldet die Uhr **0 °C**. Der Index (halbe Grad, 255 = Fehler) geht
als `RTC_TEMP_INDEX_NUM_VAR` über die Brücke — **Index 21** im gemeinsamen Enum
(`src/vars/vars.h:63`, `ESP8266/ESP-uclock/vars.h:112`; Index 20 ist `DS18XX_IS_UP`, Index 22
die Korrektur). Der ESP formatiert ihn auf der Legacy-Seite (`http.cpp:3739-3747`), die PWA
in `formatHalfDegreeValue()`, und der STM zeigt ihn an der Uhr selbst
(`display_temperature()`, `display_temperature_digits()`).

**Entscheidung des Nutzers (07.10.2026): echte Minusgrade, nicht klemmen.** Das ist keine
Korrektur an einer Stelle, sondern eine **Protokolländerung** über STM, ESP und PWA. Dazu
die Bedingung: **Firmware vor PWA** (L241), und **eine alte PWA auf neuer Firmware darf
nicht schlechter werden**.

**Ein Nebenbefund, ● nicht gegen das Datenblatt geprüft:** Das Nachkommabit wird als
`(buffer[1] & 0x02) >> 1` gelesen. Beim DS3231 stehen die Nachkommabits nach Datenblatt in
**Bit 7 und 6** des Registers `0x12`. Stimmt das, zeigt die Uhr nie „,5". Geklärt in S.7 (4).

### 2. Vier Punkte aus dem E2-Review — ausgeliefert

R1 bis R4 sind mit PWA 1.4.91 ausgeliefert. **Die ESP-Seite von R2** — Kennung 6 doppelt
belegt — wird nach Ent-4 (ja) in **F.2** getrennt.

### 3. Aus Runde U

- **B27 / L223** — **ausgeliefert** mit „Sekunden am Ambilight-Ring weich ausblenden" (Ent-3).
- **B32 / L235** — **erledigt**.
- **B28 / L227** — **wartet auf eine Gestaltungsvorgabe des Nutzers.** Nicht gestrichen.
- **B31 / L242** — **wartet auf die Safari-Messung des Nutzers.** Nicht gestrichen.

### 4. Die Brücke ist weiter halb fertig — der Entwurf steht, gebaut ist nichts

- **A39 / L259** — ✔ `src/main.c:3217` ruft im `ESP8266_IPADDRESS`-Zweig weiterhin direkt
  `var_send_all_variables()`.
- **L260 / A42** — ✔ `var_sync_check()` (`ESP-uclock.ino:753`) prüft nur
  `HARDWARE_CONFIGURATION`, das dritte von rund 194 Kommandos.
- **C26 / L253** — Zähler **und** Markenpflicht, `!v`. ToDo-Text in S.27 nachführen.
- **A35 / L233, L266** — Zuordnung als eigene Zeile vor der Quittung. ToDo-Text in S.27.
- **A16 / L106** — vom Nutzer bestätigt, einschliesslich der 60 Sekunden Spiellauf (Ent-6).
- **E17** — `pending_weather_ticker_restore` trägt seit L217 alle Ticker, nicht nur den
  Wetterticker. **Der Nutzer will die Umbenennung mit S** (07.10.2026) — als reine
  Umbenennung, so eingeordnet, dass der Diff von A39 lesbar bleibt.

### 5. Neu seit der alten Spec, und für Runde S bestimmend

- **C31 / L298** — ✔ `eepromdata.cpp:252-253`, `:258-259` schreiben beide Schlüssel im
  Klartext auf die UART. **● ungeprüft:** ein möglicher Zweitweg über die STM-Logausgabe.
- **N1 (Nachzählung), KRITISCH** — ✔ `http_overlays()` (`http.cpp:4689-4717`): `oidx` aus
  `atoi`, nur gegen `0xff` geprüft; Schreibzugriff über 32 Plätze, aus dem LAN, auch per
  `<img>`. Auf dem STM trägt der Fehler über A3 weiter — ● Task S.7.
- **L296** — ✔ F103: **1'924 Byte** frei. Dazu kommen neun STM-K-Punkte **und A5** — zu
  **messen, nicht zu schätzen**.
- **L291 / A46**, **L292 / A47** — ● aus dem H.2-Review.
- **C32 / L299** — Index 9 ist auf beiden Seiten etwas anderes. **Nicht in S.** **Für A5 ist
  C32 die Lehre:** ein Index, der auf beiden Seiten verschieden gelesen wird, kostet über
  Monate Fehlersuche. A5 nimmt deshalb **keinen** obsoleten Index wieder in Gebrauch,
  sondern hängt einen neuen an (`design.md` §7.2).

### 6. Runde F ist zum dritten Mal geplant und nie gefahren

- **C9c6 / L179** — ✔ Legacy flasht ohne Filter (`http.cpp:6969-6973`); hat die Uhr am
  04.10.2026 stillgelegt.
- **C28 / L272** — ✔ `http.cpp:7343` liest `fname + 6` ohne Längenprüfung. **L272 bleibt
  eigenständig**, F.1 schliesst sie.
- **API-Auswahlliste** (`http.cpp:12063-12071`) zeigt ohne Filter alle Dateien — nach
  **Ent-5 (ja)** künftig leer (F.3).

### 7. Der Testdurchlauf war planbar unvollständig

**L302:** Phase 3 und 4 entfielen zunächst, weil M2 fehlte — und M2 entstand **nur mit dem
Passwort des Nutzers**. **Mit B19 (07.10.2026) ändert sich das:** Das Passwort kommt aus
einer Datei ausserhalb des Repos, und der Agent darf Phase 0 selbst fahren. Bis B19 umgesetzt
ist, bleibt M2 ein Nutzerschritt.

### 8. Kleinkram bleibt liegen — oder wird erledigt und bleibt trotzdem offen

Die vorletzte Nachzählung fand **26** längst erledigte und weiter als offen geführte
Punkte (L235). Runde K soll viele Punkte abhaken — **mit** Abnahme je Punkt.

---

## Ziel

1. Die PWA meldet nach jeder Speicheraktion das **tatsächliche** Ergebnis, prüft **jedes**
   Textfeld vor dem Absenden in Byte, **sendet den Wert, den der Nutzer eingegeben oder
   ausgewählt hat**, und warnt nur bei **echten** ungespeicherten Änderungen.
2. Die Brücke STM↔ESP hat kein halbes Protokoll mehr, und **kein Schlüssel geht mehr über
   die UART**.
3. Kein Schreibzugriff über den Rand von `overlays[]` mehr aus dem LAN (N1).
4. Der Legacy-Flashpfad kann kein fremdes Abbild mehr aufspielen, und **kein
   Zeichenkettenfeld des ESP kürzt mehr still**.
5. **Die RTC-Temperatur kommt mit Vorzeichen in der PWA an** — und keine bestehende
   Oberfläche zeigt danach etwas Schlechteres als heute.
6. Möglichst viele kleine Punkte sind **nachweislich** zu, ohne zusätzlichen Flash — und auf
   dem F103 nur so viele, wie **gemessen** hineinpassen, bei mindestens 1'024 Byte Reserve.
7. Jede Runde ist **vollständig** abgenommen — oder ausdrücklich als unvollständig berichtet.

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt**
(L177/L185). Wo ein Prüfstand auf dem Host läuft, gelten die angeglichenen Typen (L256).
**Was am Gerät nicht herstellbar ist, wird nicht am Gerät verlangt.**

### Runde P — ausgeliefert, und P2

**Zuordnung:** AKP.1 bis AKP.12 sind die Abnahme der ausgelieferten Runde P (P.17, P.18).
**In P2** gelten AKP.1, AKP.2 und AKP.10 für den Overlay-Text, **AKP.13** (B17), **AKP.14**
(B40), **AKP.15** (Massnahme 4), **AKP.16** (L303, L306) und **AKP.17** (SSID-Auswahl).
**AKP.1 für Zeitserver und Zeitzone gilt erst mit AKP.14.** Den Gerätenachweis für **AKP.3
(B36)** liefert S.26.

- [ ] **AKP.1 (B35)** — Für **jedes** Textfeld — die acht Endpunkte mit
      `http_check_strvar_len()` **und**, ab P2, den Overlay-Text — prüft die PWA vor dem
      Absenden die Länge **in UTF-8-Byte** gegen die Grenze aus
      `ESP8266/ESP-uclock/vars.h:183-194` bzw. `:414`. Ein zu langer Wert wird **nicht**
      gesendet; die Meldung nennt Ist- und Höchstwert **in Byte**.
      *Instrument:* Vorschau mit 17 Umlauten (34 Byte) und mit 16 Umlauten, **beide Proben
      Pflicht**; Zählung aller Aufrufstellen (DIR-014). Gerätenachweis am Zeitserver: S.26.
- [ ] **AKP.2 (B35)** — `TEXT_FIELD_LIMITS` an **genau einer** Stelle, mit Verweis auf
      `ESP8266/ESP-uclock/vars.h`; nach P2 **zehn** Werte. *Instrument:* Review.
- [ ] **AKP.3 (B36)** — Nach einer **abgewiesenen** Speicherung von Update-Host oder -Pfad
      bleibt die Fehlermeldung **stehen** und wird **nicht** von einer Erfolgsmeldung
      überschrieben. *Instrument:* Vorschau; **am Gerät in S.26** über einen zu langen
      Update-Host mit `error=2`, Phase 9 belegt den unveränderten Gerätewert.
- [ ] **AKP.4 (B36)** — Kein anderer Aufrufer von `runButtonRequest()` ändert sein
      Verhalten. *Instrument:* Review.
- [ ] **AKP.5 (B34)** — Kein gefüllter Bereich fällt nach einem Sprachwechsel auf den
      Platzhalter zurück. *Instrument:* Vorschau; alle 17 Flächen beim Namen.
- [ ] **AKP.6 (B33)** — „Anzeigen" bei `.gz` meldet „binär" mit Grösse.
- [ ] **AKP.7 (R1)**, **AKP.8 (R2)**, **AKP.9 (R3)**, **AKP.10 (R4)** — wie ausgeliefert;
      **AKP.10 ab P2 auch für die Overlay-Texte**. *Instrument:* Vorschau.
- [x] **AKP.11 (B27)** — **Erfüllt** (PWA 1.4.91).
- [x] **AKP.12 (B32)** — **Erfüllt**.
- [ ] **AKP.13 (B17, P2, nur wenn klein)** — Ursache eingegrenzt und in **einer** Funktion
      behoben, **oder** begründet offen. Keine Umsetzung auf Verdacht.
- [ ] **AKP.14 (B40 / L321, P2)** — Ein **fokussiertes** oder **geändertes** Zeitserver-
      oder Zeitzonenfeld wird von einer späten Scan-Antwort **nicht** überschrieben; ein
      unberührtes übernimmt den Gerätewert. **Dieselbe** `prefillDeviceValue()`-Regel wie
      bei der AP-SSID. *Instrument:* Probe P2.1c, einmal gegen 1.4.91 fehlgeschlagen; am
      Gerät lesend (P2.8).
- [ ] **AKP.15 (Massnahme 4, P2)** — Wiederhergestellter Ausgangswert ⇒ kein Dialog; echte
      Änderung ⇒ Dialog. Derselbe Ausgangswert wie in AKP.14. *Instrument:* Vorschau.
- [ ] **AKP.16 (L303, L306, P2)** — Eine Formatier-Antwort mit Fehler meldet **nicht**
      „formatiert"; nach fehlgeschlagenem Umschalten steht der Knopf wieder auf seinem
      Zustand. *Instrument:* Probe P2.1e mit nachgebildeter Fehlerantwort, **einmal
      fehlgeschlagen gegen einen künstlich zurückgebauten Stand** (im Scratchpad, L268).
      **Nicht am Gerät.**
- [ ] **AKP.17 (SSID-Auswahl, P2)** — Eine **ungespeicherte, vom Gerätewert abweichende**
      Auswahl in `network-ssid-select` überlebt eine späte Scan-Antwort; neu gefundene
      Netze erscheinen weiterhin; SSID-Namen gehen nie ungeschützt per `innerHTML` ins DOM.
      **Dieselbe Regel** wie AKP.14/AKP.15. *Instrument:* Probe P2.1c (drei Fälle für die
      Auswahl), einmal gegen 1.4.91 fehlgeschlagen; am Gerät lesend (P2.8).

### Runde S — Brücke

Übernommen aus `specs/paket-2026-10-05/requirements.md` (AKS.1 bis AKS.9); neu sind AKS.0
und AKS.10 bis AKS.16.

- [ ] **AKS.0 (Flash-Gate, zwei Stufen, Ent-2 entschieden)** — **Stufe 1:** Vorabschätzung
      je Teil (Kern, A5, jeder STM-K-Punkt). Passt **schon der Kern** nicht in 1'924 −
      **1'024** Byte, geht sie an den Nutzer. **Stufe 2:** nach dem Kern und nach **jedem**
      weiteren STM-Teil ein serieller Testbau (Ent-7). Unterschreitet ein Teil die Reserve:
      bei **A5, A20, A13+A47, A3+A48 anhalten und vorlegen**, bei den übrigen
      **zurückstellen und weitermachen**. *Instrument:* Bericht S.12; Testbauten; Endmessung
      aus S8b in S.23.
- [ ] **AKS.1 (A39)** — Der `IPADDRESS`-Zweig merkt vor, gesendet wird im Hauptloop, der
      IP-Lauftext folgt **nach** dem Abgleich. *Instrument:* Quelltext.
- [ ] **AKS.2 (A39)** — Kein Einbruch des Hauptloop-Zählers über rund 2 % (L226), und **der
      IP-Lauftext läuft vollständig durch**. *Instrument:* Diagnosezeile; das Auge.
- [ ] **AKS.3 (L260/A42)** — Mit Eröffnungszeile gilt der Abgleich erst mit der
      **Abschlussmarke** als vollständig. *Instrument:* Quelltext **und** Logring.
- [ ] **AKS.4 (A35)** — Zuordnung als **eigene Zeile vor** jeder Quittung.
- [ ] **AKS.5 (A35)** — Unpassende Quittung **verworfen und gezählt**; **eine** Nachsendung.
      *Instrument:* Prüfstand, Typen angeglichen.
- [ ] **AKS.6 (Rückfall)** — **Neuer ESP gegen alten STM** läuft wie heute.
      *Instrument:* Zwischenabnahme S.11.
- [ ] **AKS.7 (C26)** — Zähler **über den Logring** ablesbar.
- [ ] **AKS.8 (C26)** — Markenpflicht für die drei Kommandoarten, `!v`, im Normalbetrieb
      **keine** Abweisung.
- [ ] **AKS.9 (A16)** — Spiel über 60 Sekunden ohne Reset. *Instrument:* Lauf durch den
      **Nutzer** (S.24, Ent-6). Ohne diesen Lauf **nicht erfüllt**.
- [ ] **AKS.10 (C31)** — nur **gesetzt**/**leer** statt der Schlüssel. **Geprüft wird die
      Form der Zeile, nicht die Abwesenheit des Schlüssels.**
- [ ] **AKS.11 (C31, Zweitweg)** — am Code beantwortet (S.7).
- [ ] **AKS.12 (A47, mit A13)** — `u.filedata` terminiert.
- [ ] **AKS.13 (A46)** — drei Verwerfungspfade am gemeinsamen Zähler. Unter dem Gate.
- [ ] **AKS.14 (N1)** — `http_overlays()` prüft `oidx < MAX_OVERLAYS && oidx <= n_overlays`
      vor jedem Schreibzugriff und vor `n_overlays++`. *Instrument:* Prüfstand mit
      Zieltypen. **Nicht am Gerät.**
- [ ] **AKS.15 (A5, STM- und ESP-Teil)** — Der STM sendet die RTC-Temperatur **zusätzlich**
      als neue Variable `RTC_TEMP_HALF_DEG_NUM_VAR` am **Ende** des gemeinsamen Enums
      (**Index 49**, auf beiden Seiten gleich), als `int16` im Zweierkomplement in halben
      Grad, **`0x8000` = kein Messwert**; im Vollabgleich und bei **Änderung des
      vorzeichenbehafteten Werts**. Der ESP kennt den Index und belegt ihn beim Start mit
      `0x8000` vor. **`RTC_TEMP_INDEX_NUM_VAR` (21), die Legacy-Seite und die Anzeige an der
      Uhr bleiben unverändert.**
      *Instrument:* **Prüfstand** für die Umrechnung aus den Registerbytes (−10,0 °C,
      −0,5 °C, 0,0 °C, +24,5 °C, Korrektur ±20, Lesefehler; Index 21 bleibt bei allen
      negativen Werten 0) — **Minusgrade sind am Gerät nicht herstellbar**. Am Gerät
      lesend: zwischen S.10 und S.24 steht Index 49 auf `0x8000` (S.11); nach S.24 gilt
      Index 49 = Index 21 bei Raumtemperatur (S.25); Diff-Snapshot zeigt Index 49 als
      einzige neue Zeile.
- [ ] **AKS.16 (E17)** — `pending_weather_ticker_restore` heisst in `src/**` `pending_ticker_restore`;
      der Diff von S.15b enthält **nur** den ersetzten Bezeichner; `CLAUDE.md`,
      `knowledge/quick-reference.md`, `knowledge/architecture-checklist.md` und `tools/**`
      nennen den alten Namen nicht mehr; der Inhalt der Invariante ist unverändert.
      *Instrument:* `git diff --word-diff` auf S.15b; `grep -a` über `src/**`, `grep` über
      die drei Dokumente und `tools/**`.

### Runde P3 — A5 in der PWA

- [ ] **AKP3.1 (A5, PWA-Teil)** — Die PWA zeigt die RTC-Temperatur aus Index 49 **mit
      Vorzeichen** und vorzeichenrichtig gerundet (−1 halbes Grad ⇒ „−0.5 °C", −3 ⇒
      „−1.5 °C"); **fehlt** Index 49 oder trägt er `0x8000`, zeigt sie wie bisher Index 21.
      *Instrument:* **Probe P3.2** in der Vorschau, sechs Fälle (−20, −1, 0, 49, `0x8000`,
      fehlend), einmal gegen den Stand vor P3 fehlgeschlagen. Am Gerät: bei Raumtemperatur
      **derselbe** Wert wie vor P3 (P3.6). **Minusgrade am Gerät nicht verlangt.**
- [ ] **AKP3.2 (A5, Rückfall)** — **Eine alte PWA auf neuer Firmware zeigt nichts
      Schlechteres als heute:** Sie liest weiter Index 21, der unverändert auf 0..250
      begrenzt ist. *Instrument:* Quelltext (Index 21 unverändert, S.21) und S.25 mit der
      PWA aus P2.

### Runde F — Flash-Überwachung

- [ ] **AKF.1 (C9c6)** — Legacy-Flashzweig prüft mit `http_remote_stm32_filename_matches()`.
      **Nicht scharf fahren.**
- [ ] **AKF.2 (C28/L272)** — Legacy-Liste über dieselbe Funktion; L272 erledigt.
- [ ] **AKF.3 (65535)** — Legacy-Liste leer, Hinweis auf `./tools/flash-stm.sh`, unter
      120 Zeichen im Logring (L257).
- [ ] **AKF.4 (Smoketest)** — `smoke-device.sh` meldet 65535 als Fehlschlag.
- [ ] **AKF.5 (Probe)** — STM-Flash über `flash-stm.sh` nach dem F-OTA.
- [ ] **AKF.6 (C38 / L323)** — SSID und Schlüssel **abgewiesen** statt gekürzt.
      *Instrument:* **Nur Quelltext und Prüfstand**; `network_client_set` bleibt gesperrt.
- [ ] **AKF.7 (Ent-4)** — `fs_remove` meldet „remove failed" mit **eigener** Kennung,
      Kennung 6 nur noch für „nicht gefunden". *Instrument:* Quelltext; F.6 prüft die
      Darstellung in der PWA (Rückfalltext mit Detail).
- [ ] **AKF.8 (Ent-5)** — Die API-Auswahlliste ist bei 65535 **leer**. *Instrument:*
      Quelltext; F.6 prüft die Darstellung einer leeren Liste in der PWA.

### Schritt C4 — Umschrift ohne Flash

- [ ] **AKC4.1** — Unter `src/**` gibt es kein Nicht-ASCII-Byte mehr; geändert sind **nur**
      die acht Dateien aus L166, die Zeilenenden je Datei unverändert.
      *Instrument:* `grep -aP '[^\x00-\x7F]'`, `git diff --stat`.
- [ ] **AKC4.2** — **Die `.hex` von F103 und F411 sind vor und nach C4 byteweise gleich.**
      Weicht eines ab, ist C4 verfehlt und wird zurückgenommen. **Kein Flash, keine
      Versionserhöhung.** *Instrument:* Lead-Bau beider Ziele, Vergleich (C4.2).

### Runde K — Kleinkram

- [ ] **AKK.1** — Jeder K-Punkt hat **vor** seiner Umsetzung einen Satz „erledigt, wenn …".
- [ ] **AKK.2** — Die Zahl der K-Punkte stimmt mit der Nachzählung; jeder hat genau einen
      Träger, ist erledigt, geschlossen oder wartet ausdrücklich. *Instrument:* K.0.
- [ ] **AKK.3** — Wächst ein K-Punkt über rund zehn Zeilen oder wirft er eine Entwurfsfrage
      auf, wird er **umgestuft und zurückgemeldet**.
- [ ] **AKK.4** — Jeder geschlossene K-Punkt trägt einen Beleg **dieses Commits** oder einen
      Messwert; **geschlossen durch Entscheidung** (C2, C9c4) trägt Datum und Entscheidung.
- [ ] **AKK.5** — Jeder STM-K-Punkt ist **gemessen**, ob er bleibt oder zurückgestellt wird.
- [ ] **AKK.6 (STM-Guards)** — Abnahme über den **Prüfstand** (S15), nicht am Gerät.

### Für jede Runde

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `./tools/guardrails.sh --full` vor dem Release mit Exit 0 (Lead, R1).
- [ ] **AKZ.3** — `./tools/smoke-device.sh` nach jedem Einspielen ohne Fehlschlag
      (DIR-009). **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.4** — **Ein `pwa-tester`-Durchlauf je Runde über die Phasen 0 bis 4 und 9**,
      nach M2. Unvollständig ⇒ **nicht abgenommen**. **Ausnahmen, begründet:** **P2**
      (Proben P2.1c und P2.1e, lesende Geräteprobe P2.8; S.26 läuft gegen einen Stand mit
      P2) und **P3** (Probe P3.2; F.12 läuft gegen einen Stand mit P3). **M2:** bis B19
      umgesetzt ist durch den Nutzer, danach durch den `pwa-tester` in Phase 0.
- [ ] **AKZ.5** — `./tools/watch-log.sh` läuft bei jedem Gerätelauf mit (DIR-013).
- [ ] **AKZ.6** — Jedes ausgerollte Release **sofort** committet, getaggt, **gepusht**
      (DIR-011).
- [ ] **AKZ.7** — Die Einspielzeile nennt die **Reihenfolge** (L241, DIR-002), **fertig zum
      Einfügen**.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **Runde U: B28, B31** | **Wartet auf den Nutzer**, nicht gestrichen |
| **B24 / L208** | Erledigt |
| **Weitere Felder in `updateNetworkControlsFromMeta()`** | P2.1b und P2.1f beheben, was belegt ist. Weiteres melden, nicht mitkorrigieren |
| **Gerätenachweis für L303 und L306 mit echtem Fehlschlag** | Nicht folgenlos herstellbar; Nachweis über P2.1e |
| **Schreibende WLAN-Aufrufe am Gerät** | Auch für C38 nicht |
| **Minusgrade an der Uhr selbst** (Wortanzeige, Ziffernanzeige) | **Entscheidung Ent-8**, Empfehlung: nicht in diesem Paket. Die Wortanzeige kennt ohnehin nur 10 °C bis 40 °C (`display_temperature()`, Index 20..79); die Ziffernanzeige zeigt heute „00". Minusgrade dort bräuchten ein Minuszeichen im Lauftextzeichensatz und Flash — eine eigene Entscheidung über das Ziffernbild |
| **Minusgrade beim DS18xx** | Gleiche Index-Logik (`DS18XX_TEMP_INDEX_NUM_VAR`), gehört aber zu **A2** (DS18xx-Spec). Der Entwurf aus A5 ist so gebaut, dass A2 ihn mit einer weiteren angehängten Variable übernehmen kann |
| **Der Nachkommabit-Fehler (S.7 (4))** | Wird geklärt; **bestätigt er sich, ist er ein eigener Befund** und wird nur mit Freigabe des Nutzers in S.18b mitkorrigiert, weil er die angezeigten Werte ändert |
| **C32 / L299 — Ambilight-Flag, Index 9** | Nicht in S: anderer Gegenstand, offene Entwurfsfrage, Flash, kein Sicherheitsbefund. **Kein K-Punkt** |
| **A35 Teil 2** | Rückgabewert aus `var_set_parameter()` |
| **C6b / L22** | Eigene Abwägung |
| **C19 / L199** | N1 und C6u fassen `http_overlays()` an, `saveuphost` bleibt unberührt — C19 als Ganzes ist nicht entschieden |
| **Löschen der Pi-Logdateien mit Schlüsseln** (L298) | Handlung des Nutzers |
| **Die alte Sicherung `vor-durchlauf-3.2.15.tar.gz.enc`** | **Bleibt** (Entscheidung zu B19) |
| **E3** | Wartet; der Nutzer sieht im KiCad-Projekt nach |

---

## Betroffene Laufzeiten

- [x] **PWA** — P ausgeliefert (1.4.91); **P2** vor S; **P3** nach S
- [x] **ESP8266** — S (mit N1 und A5-ESP-Teil) und F (mit C38, Ent-4, Ent-5, C23, C6u und
      fünf weiteren K-Punkten), je ein OTA
- [x] **STM32** (`src/**`) — S, ein Flash (mit E17, A5 und gemessenen K-Punkten); F flasht
      denselben Stand als Probe; **C4 ohne Flash**
- [x] **Werkzeug** (`tools/**`) — Proben (P2, P3), Prüfstände und S15 (S), Prüfstand C38,
      Smoketest, C23-/C25-Leser (F), K.W mit **B19** und **E2**
- [x] **Dokumentation** — `BEFUNDE.md` je Runde; **E17** in `CLAUDE.md` (Lead) und
      `knowledge/` (`doc-writer`)
- [x] **Build/Release** — je Einspielschritt ein Release-ZIP; dazu **serielle Testbauten**
      (S) und **Gleichheitsbauten** (C4) ohne Release

---

## Entscheidungen

Stand **07.10.2026**.

- **Ent-1 — Freigabe. Entschieden: freigegeben** für P2, S, F und K.
- **Ent-2 — Flash. Entschieden:** Reserve **1'024 Byte**; Reihenfolge A20 → A13+A47 →
  A3+A48 → A24 → A45 → A14 → A46 → A49, **danach A9** (neu); bei A20, A13+A47, A3+A48
  **anhalten und vorlegen**, die übrigen **zurückstellen**. **A5** steht vor diesen Punkten
  und wird ebenfalls **angehalten und vorgelegt** — Festlegung dieser Spec, weil A5 eine
  ausdrückliche Bestellung des Nutzers ist (`design.md` §7.6).
- **Ent-3 — Wortlaut B27. Erledigt.**
- **Ent-4 — Kennung 6 trennen. Entschieden: ja** (F.2).
- **Ent-5 — API-Auswahlliste bei 65535. Entschieden: ja, leere Liste** (F.3).
- **Ent-6 — A16 samt Spiellauf. Entschieden.**
- **Ent-7 — Serielle Testbauten. Entschieden vom Lead: ja.** R1 soll paralleles Bauen
  verhindern, und das bleibt gewahrt; „einmal am Ende" stammt aus einer Zeit ohne Flash-Gate.
- **Ent-8 — Zeigt die Uhr selbst Minusgrade an? Offen.** Empfehlung: **nein, nicht in diesem
  Paket.** A5 bringt die Minusgrade in die PWA; die Anzeige an der Uhr bleibt wie heute (Wortanzeige
  nur 10 °C bis 40 °C, Ziffern „00" unter null). Wer sie dort will, entscheidet über ein
  Minuszeichen im Zeichensatz und über Flash — beides gehört in eine eigene Runde.

**Weitere Entscheidungen vom 07.10.2026, eingearbeitet:** A5 echte Minusgrade (S.4b,
S.18b, P3); A9 ungeklammert (S.19i); C23 UTF-8 plus Escapen (F.5i); C6u ja, in F (F.5j); C2
und C9c4 lassen und schliessen (P2.9); C4 Umschrift ASCII als eigener Schritt (C4); B19
Passwort aus Datei ausserhalb des Repos, Agent fährt Phase 0 (K.W); E2 löschen (K.W); E17
mit S (S.15b–d); **E3 offen**.
