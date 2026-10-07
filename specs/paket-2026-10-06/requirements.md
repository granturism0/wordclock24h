# Anforderungen — Paket 2026-10-06 (Runden P, P2, S, P3, F, C4, verteilte Runde K)

**Status: freigegeben am 07.10.2026** (Ent-1) für P2, S, F und K. Runde P ist auf
Entscheidung des Leads bereits ausgeliefert (PWA 1.4.91, Abnahme in L324).
**Erstellt:** 2026-10-06, Stand des Arbeitsbaums `401aae2` (Zweig `pwa-decoupling`).
**Nachgeführt:** am 06.10.2026 nach der Auslieferung von P und dem Gerätetest von 1.4.91;
am **07.10.2026** in zwei Durchgängen mit den Entscheidungen des Nutzers — darunter **A5 als
Protokolländerung** (echte Minusgrade in PWA **und Legacy**, Uhranzeige unverändert), das
**RTC-Nachkommabit** mit A5, **E17 mit S**, **C4 als eigener Schritt**, **P3** für A5;
**B19 und E2 erledigt**; **P2.1h** neu, **P2.2/P2.3 entfallen**.
**Auslöser:** Entscheidungen des Nutzers: Das nächste grosse Paket umfasst **P, S und F, in
dieser Reihenfolge**. U bleibt draussen; aus U wandern **B27 und B32 nach P**, B28 und B31
**warten auf den Nutzer**. Dazu eine **Kleinkram-Runde K** aus der Nachzählung vom
06.10.2026.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Es übernimmt aus
`specs/paket-2026-10-05/`, was für P, S und F weiter gilt; die alte Spec bleibt unverändert
stehen. Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** In `app.js` nennt diese Spec vorzugsweise **Funktionsnamen**. In den
Firmware-Quellen gelten die Zeilen für Stand `401aae2`. **`vars.h` ohne Pfad meint
`ESP8266/ESP-uclock/vars.h`**; die STM-Datei heisst ausdrücklich `src/vars/vars.h`.

**Kennungen.** „N1" und „N2" **ohne Zusatz** sind **Befunde aus der Nachzählung**; die zwei
Befunde aus dem Review von P2 heissen **„P2-Review N1/N2"**. **L321 / B40**, **L323 / C38**
und **L324** stammen aus dem Gerätetest von 1.4.91. **L303** betrifft
`runConfirmedButtonAction`, **L306** das Umschalten von Flags. **„L-Befund Nachkommabit"**
ist neu; seine Nummer vergibt der Lead. Die Entscheidungen heissen **Ent-1 bis Ent-8**.

---

## Überblick

| Runde | Inhalt | Laufzeit | Einspielen | Testdurchlauf |
|---|---|---|---|---|
| **P** | **ausgeliefert als PWA 1.4.91** (`release/3.2.21-3.2.24-1.4.91`): B34, B35 (ohne Overlay-Text), B36, B33, B27, B26, R1–R4, dazu L304, B37/L305, B38/L306. B32 erledigt | PWA | erledigt | lesender Gerätetest gelaufen (L324); Durchlauf mit M2 offen (P.18) |
| **P2** | Rest aus P: **Overlay-Text in `TEXT_FIELD_LIMITS`**, **B40/L321**, **SSID-Auswahl**, **Massnahme 4 — nur für die vier vorbefüllten Netzwerkfelder**, **P2-Review N1/N2**; dazu die **Vorschau-Probe für L303 und L306** | PWA | nächster PWA-Upload, **vor** S | **kein eigener**, begründet: Proben plus lesende Geräteprobe; Gerätenachweis von B35 am Zeitserver in S.26 |
| **S** | Brücke fertigbauen: L260/A42, A35 Teil 1, C26, A39, A16, **C31**, **A46**, **E17** (reine Umbenennung), **A5** (STM-, ESP- und Legacy-Teil) samt **Nachkommabit**; dazu **N1** und neun STM-K-Punkte — mit **zweistufigem Flash-Gate**, Reserve 1'024 Byte; **B17 Spur 3** als lesende Beobachtung | ESP **und** STM | **erst ESP, dann STM** | ja; dazu Gerätenachweis für B35 (Zeitserver) und B36 |
| **P3** | **A5, PWA-Teil**: Minusgrade anzeigen, mit Rückfall | PWA | **nach** S.24 | kein eigener; F.12 läuft gegen einen Stand mit P3 |
| **F** | Flash-Überwachung: C9c6/L179, Auswahlliste bei 65535, C28/L272, Smoketest-Stufe; **C38/L323**, **Ent-4**, **Ent-5**; dazu sieben ESP-K-Punkte (mit C23 und C6u) | ESP + Werkzeug | **nur** ESP; ein STM-Flash als Probe | ja |
| **C4** | Umschrift der acht ISO-8859-1-Dateien unter `src/**` auf ASCII | STM-Quellen | **kein Flash** — Nachweis über gleiche `.hex` | — |
| **K** | Kleinkram aus der Nachzählung — **kein eigener Flash**; Werkzeug ohne Flash; **B19 und E2 erledigt**; C2, C9c4 geschlossen; E3 wartet | je Punkt | mit dem Träger | im Durchlauf des Trägers |

**E31 / L302 ist erledigt.** **Ein Vorbehalt daraus trägt in diese Spec:** Ob die
STM-Indexguards greifen, lässt sich über HTTP nicht zeigen, weil der ESP jeden Index vorher
abweist. Die Abnahme von STM-Guards läuft deshalb über den Prüfstand (AKK.6).

---

## Problem

### 1. Die PWA hatte drei Befunde aus dem letzten Release — inzwischen ausgeliefert

| Befund | Status vor P | Kern |
|---|---|---|
| **B35 / L287** | ✔ verifiziert am Code (`http.cpp:10351-10363`; `index.html` `maxlength`) | `maxlength` zählt **Zeichen**, `http_check_strvar_len()` seit ESP 3.2.24 **Byte** |
| **B36 / L288** | ✔ verifiziert | Nach dem Speichern lief **unbedingt** die Nachprüfung und überschrieb die Abweisung |
| **B34 / L284** | ● gemeldet mit Messung; Ursache in `applyTranslations()` ✔ | Ein Sprachwechsel setzte **17** gefüllte Flächen auf ihren Platzhalter zurück |

**Ungenauigkeit in L287:** `:239` (SSID von Hand) gehört nicht dazu, `:405` (Datumsformat)
fehlt. Massgeblich sind die acht Endpunkte mit `http_check_strvar_len()`.

**Der neunte Endpunkt, offen für P2:** `http_api_overlay_set` kürzt den Overlay-Text still
(`http.cpp:11107`). Grenze `OVERLAY_MAX_TEXT_LEN` = 32 (**`ESP8266/ESP-uclock/vars.h:414`**).
`TEXT_FIELD_LIMITS` in 1.4.91 führt ihn nicht. PWA-Seite in P2.1, ESP-Seite in F.

### 1a. B40 / L321 — ein später Scan überschreibt die Eingabe

`updateNetworkControlsFromMeta()` setzt Zeitserver und Zeitzone nach jeder
`network_scan`-Antwort ohne Bedingung; „Speichern" schickt danach still den alten Wert. Für
die AP-SSID ist dieselbe Falle mit `prefillDeviceValue()` gelöst (L34). **Dasselbe Muster an
der SSID-Auswahl** (`network-ssid-select`, P2.1f). **Folge für B35:** Am Zeitserver erst
nach B40 nachgewiesen.

### 1b. Massnahme 4 — der Dialog bei unveränderten Feldern

`handleDirtyFormInteraction()` setzt `hasUnsavedEdits = true` bei **jeder** Eingabe. Am
Gerät beobachtet (L324). **Entscheidung des Leads (07.10.2026): Die Korrektur gilt nur für
die vier vorbefüllten Netzwerkfelder**, nicht allgemein. Für alle anderen Felder bleibt das
Verhalten, wie es ist — und `BEFUNDE.md` hält Massnahme 4 deshalb ausdrücklich als
**eingeschränkt** umgesetzt fest.

**Zwei Befunde aus dem P2-Review** betreffen genau diese Vergleichsregel: **P2-Review N1**
(der Ersatz-Ausgangswert `networks[0]`) und **P2-Review N2** (`unsavedEditFields` nach dem
Vorbefüllen). P2.1h.

### 1c. C38 / L323 — SSID und WLAN-Schlüssel werden still gekürzt

`toCharArray` mit fester Länge in `http.cpp:8664-8670` und `:8779-8785`.

### 1d. L303 und L306 — am Gerät nicht folgenlos nachweisbar

Nachweis über eine nachgebildete Fehlerantwort in der Vorschau (P2.1e).

### 1e. A5 / L38 — Minusgrade gibt es heute nicht

✔ am Code: `rtc_get_temperature_index()` (`src/rtc/rtc.c:352-384`) liest das vorzeichenbehaftete
Temperaturbyte des DS3231 als `uint8_t` und begrenzt das Ergebnis auf **0..250** halbe Grad;
unter 0 °C meldet die Uhr **0 °C**. Der Wert geht als `RTC_TEMP_INDEX_NUM_VAR` (**Index 21**)
über die Brücke und wird von der Legacy-Seite, der PWA und der Anzeige an der Uhr gelesen.

**Entscheidungen des Nutzers (07.10.2026):** echte Minusgrade, nicht klemmen — **in PWA und
Legacy**. **Die Uhranzeige bleibt, wie sie ist** (Ent-8), zeigt also bei Minusgraden 0 °C.
Dazu: Firmware vor PWA (L241), und eine alte PWA auf neuer Firmware darf nicht schlechter
werden.

### 1f. L-Befund Nachkommabit — die halben Grad gehen verloren

● gemeldet, Prüfung in S.7 (4): `src/rtc/rtc.c:364` bildet den Index aus
`(buffer[0] << 1) | ((buffer[1] & 0x02) >> 1)`. Beim DS3231 trägt das Register `0x12` die
Nachkommastelle nach Datenblatt in **Bit 7 (0,5 °C) und Bit 6 (0,25 °C)**; Bit 1 ist dort
immer 0. **Stimmt das, ist der Index nie ungerade**, und weder Legacy noch PWA haben je
„,5 °C" gezeigt. **Entscheidung des Nutzers (07.10.2026): Bestätigt S.7 den Fehler, wird er
mit A5 in S.18b korrigiert; die Freigabe liegt damit vor.** Die Befundzeile legt der Lead an.

### 2. Vier Punkte aus dem E2-Review — ausgeliefert

R1 bis R4 sind ausgeliefert. **Die ESP-Seite von R2** — Kennung 6 doppelt belegt — wird
nach Ent-4 (ja) in **F.2** getrennt.

### 3. Aus Runde U

B27 ausgeliefert, B32 erledigt; **B28** und **B31 warten auf den Nutzer**, nicht gestrichen.

### 4. Die Brücke ist weiter halb fertig — der Entwurf steht, gebaut ist nichts

A39 (`src/main.c:3217`), L260/A42 (`ESP-uclock.ino:753`), C26, A35 Teil 1, A16 (Ent-6),
**E17** (Umbenennung mit S, als eigener Diff nach A39).

### 5. Neu seit der alten Spec, und für Runde S bestimmend

C31 (`eepromdata.cpp:252-259`), **N1** (`http.cpp:4689-4717`, KRITISCH), **L296** (F103
1'924 Byte frei), L291/A46, L292/A47, C32 (nicht in S — **die Lehre aus C32 trägt A5**:
kein obsoleter Index wird wiederverwendet).

### 6. Runde F ist zum dritten Mal geplant und nie gefahren

C9c6/L179, C28/L272 (bleibt eigenständig), API-Auswahlliste nach **Ent-5 (ja)** leer.

### 7. Der Testdurchlauf war planbar unvollständig

**L302:** M2 entstand nur mit dem Passwort des Nutzers. **B19 ist umgesetzt** (`e121058`):
`snapshot-device.sh` liest das Passwort aus `SNAPSHOT_PASS`, dann aus
`~/.config/wordclock/snapshot.pass`, dann vom Terminal. **Die Datei legt der Nutzer an** —
bis dahin bleibt M2 ein Nutzerschritt, danach fährt der Agent Phase 0 selbst.

### 8. Kleinkram bleibt liegen — oder wird erledigt und bleibt trotzdem offen

Die vorletzte Nachzählung fand **26** längst erledigte und weiter als offen geführte Punkte
(L235). Runde K soll viele Punkte abhaken — **mit** Abnahme je Punkt.

---

## Ziel

1. Die PWA meldet nach jeder Speicheraktion das **tatsächliche** Ergebnis, prüft **jedes**
   Textfeld vor dem Absenden in Byte, **sendet den Wert, den der Nutzer eingegeben oder
   ausgewählt hat**, und warnt an den vier vorbefüllten Netzwerkfeldern nur bei **echten**
   Änderungen.
2. Die Brücke STM↔ESP hat kein halbes Protokoll mehr, und **kein Schlüssel geht mehr über
   die UART**.
3. Kein Schreibzugriff über den Rand von `overlays[]` mehr aus dem LAN (N1).
4. Der Legacy-Flashpfad kann kein fremdes Abbild mehr aufspielen, und **kein
   Zeichenkettenfeld des ESP kürzt mehr still**.
5. **Die RTC-Temperatur kommt mit Vorzeichen und mit halben Grad in PWA und Legacy an** —
   die Uhranzeige bleibt, wie sie ist, und keine bestehende Oberfläche wird schlechter.
6. Möglichst viele kleine Punkte sind **nachweislich** zu — auf dem F103 nur so viele, wie
   **gemessen** hineinpassen, bei mindestens 1'024 Byte Reserve.
7. Jede Runde ist **vollständig** abgenommen — oder ausdrücklich als unvollständig berichtet.

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt**
(L177/L185). Wo ein Prüfstand auf dem Host läuft, gelten die angeglichenen Typen (L256).
**Was am Gerät nicht herstellbar ist, wird nicht am Gerät verlangt.**

### Runde P — ausgeliefert, und P2

**Zuordnung:** AKP.1 bis AKP.12 sind die Abnahme der ausgelieferten Runde P (P.17, P.18).
**In P2** gelten AKP.1, AKP.2 und AKP.10 für den Overlay-Text, **AKP.14** (B40), **AKP.15**
(Massnahme 4), **AKP.16** (L303, L306), **AKP.17** (SSID-Auswahl) und **AKP.18**
(P2-Review N1/N2). **AKP.1 für Zeitserver und Zeitzone gilt erst mit AKP.14.** Den
Gerätenachweis für **AKP.3 (B36)** liefert S.26. **AKP.13 (B17)** ist aus P2 heraus und wird
in S beobachtet.

- [ ] **AKP.1 (B35)** — Für **jedes** Textfeld — die acht Endpunkte mit
      `http_check_strvar_len()` **und**, ab P2, den Overlay-Text — prüft die PWA vor dem
      Absenden die Länge **in UTF-8-Byte** gegen die Grenze aus
      `ESP8266/ESP-uclock/vars.h:183-194` bzw. `:414`. *Instrument:* Vorschau mit 17 und 16
      Umlauten, **beide Proben Pflicht**; Zählung aller Aufrufstellen (DIR-014).
      Gerätenachweis am Zeitserver: S.26.
- [ ] **AKP.2 (B35)** — `TEXT_FIELD_LIMITS` an **genau einer** Stelle; nach P2 **zehn**
      Werte. *Instrument:* Review.
- [ ] **AKP.3 (B36)** — Nach einer **abgewiesenen** Speicherung von Update-Host oder -Pfad
      bleibt die Fehlermeldung **stehen** und wird **nicht** von einer Erfolgsmeldung
      überschrieben. *Instrument:* Vorschau; **am Gerät in S.26** über einen zu langen
      Update-Host mit `error=2`, Phase 9 belegt den unveränderten Gerätewert.
- [ ] **AKP.4 (B36)** bis **AKP.10 (R4)** — wie ausgeliefert; **AKP.10 ab P2 auch für die
      Overlay-Texte**.
- [x] **AKP.11 (B27)**, **AKP.12 (B32)** — **erfüllt**.
- [ ] **AKP.13 (B17)** — **Aus P2 heraus** (07.10.2026): Die Spuren 1 und 2 aus L142 sind im
      Code abgedeckt. **Spur 3** — leerer Logring nach ESP-Neustart — ist beobachtet: Was
      zeigt das Logfenster ohne Reload, solange der Ring leer ist, und füllt es sich von
      selbst? *Instrument:* **lesende Beobachtung in S.11**, mit Zeitstempeln; das Ergebnis
      schliesst B17 oder lässt es mit Beleg offen (S.27).
- [ ] **AKP.14 (B40 / L321)** — Ein **fokussiertes** oder **geändertes** Zeitserver- oder
      Zeitzonenfeld wird von einer späten Scan-Antwort **nicht** überschrieben; **dieselbe**
      `prefillDeviceValue()`-Regel wie bei der AP-SSID. *Instrument:* Probe P2.1c, einmal
      gegen 1.4.91 fehlgeschlagen; am Gerät lesend (P2.8).
- [ ] **AKP.15 (Massnahme 4, eingeschränkt)** — **An den vier vorbefüllten Netzwerkfeldern:**
      wiederhergestellter Ausgangswert ⇒ kein Dialog; echte Änderung ⇒ Dialog. **An allen
      anderen Feldern bleibt das Verhalten wie heute.** Derselbe Ausgangswert wie in
      AKP.14. *Instrument:* Vorschau, beide Fälle **und** eine Gegenprobe an einem Feld
      ausserhalb (dort erscheint der Dialog weiterhin). `BEFUNDE.md` führt Massnahme 4 als
      **eingeschränkt** umgesetzt (P2.9).
- [ ] **AKP.16 (L303, L306)** — Formatier-Fehler meldet **nicht** „formatiert";
      fehlgeschlagenes Umschalten stellt den Knopf zurück. *Instrument:* Probe P2.1e, einmal
      gegen einen künstlich zurückgebauten Stand fehlgeschlagen (Scratchpad, L268). **Nicht
      am Gerät.**
- [ ] **AKP.17 (SSID-Auswahl)** — Eine ungespeicherte, abweichende Auswahl überlebt eine
      späte Scan-Antwort; SSID-Namen nie ungeschützt per `innerHTML`. *Instrument:* Probe
      P2.1c; am Gerät lesend (P2.8).
- [ ] **AKP.18 (P2-Review N1/N2)** — **N1** (Ersatz-Ausgangswert `networks[0]`) und **N2**
      (`unsavedEditFields` nach dem Vorbefüllen) sind behoben, mit Ursache und Funktionsname
      im Bericht. *Instrument:* Probe P2.1c läuft danach erneut grün; der `code-reviewer`
      bestätigt beide in P2.4.

### Runde S — Brücke

Übernommen aus `specs/paket-2026-10-05/requirements.md` (AKS.1 bis AKS.9); neu sind AKS.0
und AKS.10 bis AKS.17.

- [ ] **AKS.0 (Flash-Gate, Ent-2 entschieden)** — **Stufe 1:** Vorabschätzung je Teil (Kern,
      A5 samt Nachkommabit, jeder STM-K-Punkt). Passt **schon der Kern** nicht in 1'924 −
      **1'024** Byte, geht sie an den Nutzer. **Stufe 2:** serielle Testbauten (Ent-7) nach
      dem Kern und nach jedem weiteren Teil. Unterschreitung: bei **A5, A20, A13+A47,
      A3+A48 anhalten und vorlegen**, sonst **zurückstellen und weitermachen**.
- [ ] **AKS.1 (A39)** bis **AKS.14 (N1)** — wie in der vorigen Fassung: A39 Variante (b);
      Hauptloop und vollständiger IP-Lauftext; Abschlussmarke; Zuordnungszeile; unpassende
      Quittung verworfen und gezählt; Rückfall neuer ESP gegen alten STM (S.11); C26-Zähler
      über den Logring; Markenpflicht mit `!v`; A16 mit Spiellauf des Nutzers (S.24); C31
      „gesetzt"/„leer", **geprüft an der Form der Zeile**; C31-Zweitweg (S.7); A47 mit A13;
      A46 unter dem Gate; N1-Indexguard über den Prüfstand mit Zieltypen, **nicht am Gerät**.
- [ ] **AKS.15 (A5 — STM, ESP, Legacy)** — Der STM sendet die RTC-Temperatur **zusätzlich**
      als neue Variable `RTC_TEMP_HALF_DEG_NUM_VAR` am **Ende** des gemeinsamen Enums
      (**Index 49**, auf beiden Seiten gleich), als `int16` im Zweierkomplement in halben
      Grad, **`0x8000` = kein Messwert**; im Vollabgleich und bei **Änderung des
      vorzeichenbehafteten Werts**. Der ESP kennt den Index, belegt ihn beim Start mit
      `0x8000` vor, **zeigt auf der Legacy-Seite Minusgrade** (Rückfall auf Index 21 bei
      `0x8000`) und rechnet Index 49 bei einer Korrekturänderung **an denselben Stellen und
      über dieselbe Hilfsfunktion** nach wie Index 21. **Index 21 und die Anzeige an der Uhr
      bleiben unverändert** (Ent-8: bei Minusgraden 0 °C).
      *Instrument:* **Prüfstände** — STM-Umrechnung aus Registerbytes (−10,0 °C, −0,5 °C,
      0,0 °C, +24,5 °C, Korrektur ±20, Lesefehler; Index 21 bei allen negativen Werten 0;
      Wechsel −1 °C → −8 °C löst Versand aus) und Legacy-Formatierung (−20, −1, 0, +49,
      `0x8000`). **Minusgrade sind am Gerät nicht herstellbar.** Am Gerät lesend: zwischen
      S.10 und S.24 Index 49 = `0x8000`, Legacy und PWA unverändert (S.11); nach S.24
      Index 49 = Index 21 bei Raumtemperatur, Legacy und PWA zeigen denselben Wert (S.25).
- [ ] **AKS.16 (E17)** — `pending_weather_ticker_restore` heisst in `src/**`
      `pending_ticker_restore`; der Diff von S.15b enthält **nur** den ersetzten
      Bezeichner; `CLAUDE.md`, `knowledge/quick-reference.md`,
      `knowledge/architecture-checklist.md` und `tools/**` nennen den alten Namen nicht mehr.
      *Instrument:* `git diff --word-diff`; `grep`.
- [ ] **AKS.17 (L-Befund Nachkommabit)** — **Nur wenn S.7 (4) den Fehler bestätigt:** Der
      Index nimmt das halbe Grad aus **Bit 7** von Register `0x12`. Die halben Grad stimmen
      danach: Registerbytes für +24,0 °C mit Bit 7 ergeben +24,5 °C (Index 49), mit Bit 6
      allein +24,0 °C (Index 48), und dasselbe vorzeichenrichtig im negativen Bereich (−0,5 °C
      ⇒ −1). *Instrument:* **Prüfstand mit genau diesen Registerfällen**, einmal gegen das
      alte `rtc.c` fehlgeschlagen (DIR-014). Am Gerät **als Beobachtung**, nicht als
      Abnahme: ungerade Indizes treten bei Raumtemperatur auf, sofern die Temperatur sie
      hergibt. **Bestätigt S.7 den Fehler nicht,** steht die Begründung im Bericht, und der
      Befund wird damit geschlossen.

### Runde P3 — A5 in der PWA

- [ ] **AKP3.1 (A5, PWA-Teil)** — Die PWA zeigt die RTC-Temperatur aus Index 49 **mit
      Vorzeichen** und vorzeichenrichtig gerundet (−1 ⇒ „−0.5 °C", −3 ⇒ „−1.5 °C"); fehlt
      Index 49 oder trägt er `0x8000`, zeigt sie Index 21. *Instrument:* Probe P3.2, sechs
      Fälle, einmal gegen den Stand vor P3 fehlgeschlagen. Am Gerät: bei Raumtemperatur
      derselbe Wert wie auf der Legacy-Seite und wie Index 21 (P3.6).
- [ ] **AKP3.2 (A5, Rückfall)** — **Eine alte PWA auf neuer Firmware zeigt nichts
      Schlechteres als heute:** Sie liest weiter Index 21, unverändert auf 0..250 begrenzt —
      **mit der einzigen gewollten Ausnahme**, dass sie nach der Nachkommabit-Korrektur
      halbe Grad zeigen kann, die vorher fehlten. *Instrument:* Quelltext (S.21) und S.25.
- [ ] **AKP3.3 (gleiche Formatierung)** — PWA und Legacy-Seite formatieren denselben Wert
      gleich. *Instrument:* P3.3 (Review) und P3.6 am Gerät.

### Runde F — Flash-Überwachung

- [ ] **AKF.1 (C9c6)**, **AKF.2 (C28/L272)**, **AKF.3 (65535)**, **AKF.4 (Smoketest)**,
      **AKF.5 (Probe)** — wie in der vorigen Fassung. **Nicht scharf fahren.**
- [ ] **AKF.6 (C38 / L323)** — SSID und Schlüssel **abgewiesen** statt gekürzt.
      *Instrument:* **Nur Quelltext und Prüfstand**; `network_client_set` bleibt gesperrt.
- [ ] **AKF.7 (Ent-4)** — „remove failed" mit **eigener** Kennung; Kennung 6 nur noch für
      „nicht gefunden". *Instrument:* Quelltext; F.6 prüft die Darstellung in der PWA.
- [ ] **AKF.8 (Ent-5)** — API-Auswahlliste bei 65535 **leer**. *Instrument:* Quelltext; F.6
      prüft die Darstellung einer leeren Liste.

### Schritt C4 — Umschrift ohne Flash

- [ ] **AKC4.1** — Kein Nicht-ASCII-Byte mehr unter `src/**`; nur die acht Dateien aus L166
      geändert, Zeilenenden unverändert.
- [ ] **AKC4.2** — **`.hex` von F103 und F411 vor und nach C4 byteweise gleich.** Sonst
      verfehlt und zurückgenommen. **Kein Flash, keine Versionserhöhung.**

### Runde K — Kleinkram

- [ ] **AKK.1** — Jeder K-Punkt hat **vor** seiner Umsetzung einen Satz „erledigt, wenn …".
- [ ] **AKK.2** — Die Zahl der K-Punkte stimmt mit der Nachzählung (K.0).
- [ ] **AKK.3** — Wächst ein K-Punkt, wird er **umgestuft und zurückgemeldet**.
- [ ] **AKK.4** — Geschlossene K-Punkte tragen einen Beleg dieses Commits, einen Messwert —
      oder, wenn **durch Entscheidung** geschlossen (C2, C9c4), Datum und Entscheidung.
      **Erledigt gemeldete Werkzeugpunkte** (B19 `e121058`, E2 `8dfba36`) tragen ihren Commit.
- [ ] **AKK.5** — Jeder STM-K-Punkt ist **gemessen**.
- [ ] **AKK.6 (STM-Guards)** — Abnahme über den **Prüfstand** (S15), nicht am Gerät.

### Für jede Runde

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `./tools/guardrails.sh --full` vor dem Release mit Exit 0 (Lead, R1).
- [ ] **AKZ.3** — `./tools/smoke-device.sh` nach jedem Einspielen ohne Fehlschlag (DIR-009).
      **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.4** — **Ein `pwa-tester`-Durchlauf je Runde über die Phasen 0 bis 4 und 9**,
      nach M2. Unvollständig ⇒ **nicht abgenommen**. **Ausnahmen, begründet:** **P2**
      (Proben P2.1c und P2.1e, lesende Geräteprobe P2.8; S.26 läuft gegen einen Stand mit
      P2) und **P3** (Probe P3.2; F.12 läuft gegen einen Stand mit P3). **M2:** durch den
      Nutzer, bis er die Passwortdatei aus B19 angelegt hat; danach durch den `pwa-tester`.
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
| **Massnahme 4 allgemein** | **Entscheidung des Leads:** nur die vier vorbefüllten Netzwerkfelder. Alle anderen Felder bleiben, wie sie sind |
| **Weitere Felder in `updateNetworkControlsFromMeta()`** | Melden, nicht mitkorrigieren |
| **B17, Umsetzung in P2** | Entfällt (07.10.2026); Spur 3 wird in S.11 beobachtet |
| **Gerätenachweis für L303 und L306 mit echtem Fehlschlag** | Nicht folgenlos herstellbar; Probe P2.1e |
| **Schreibende WLAN-Aufrufe am Gerät** | Auch für C38 nicht |
| **Minusgrade an der Uhr selbst** | **Entschieden (Ent-8):** Die Uhranzeige bleibt, wie sie ist, und zeigt bei Minusgraden 0 °C |
| **Minusgrade beim DS18xx** | Gehört zu **A2**; der Entwurf aus A5 ist übertragbar |
| **C32 / L299** | Nicht in S. **Kein K-Punkt** |
| **A35 Teil 2**, **C6b / L22** | Eigene Abwägung |
| **C19 / L199** als Ganzes | `saveuphost` bleibt unberührt |
| **Löschen der Pi-Logdateien mit Schlüsseln** (L298) | Handlung des Nutzers |
| **Die alte Sicherung `vor-durchlauf-3.2.15.tar.gz.enc`** | **Bleibt** |
| **E3** | Wartet; der Nutzer sieht nach |

---

## Betroffene Laufzeiten

- [x] **PWA** — P ausgeliefert (1.4.91); **P2** vor S; **P3** nach S
- [x] **ESP8266** — S (N1, A5 samt Legacy-Anzeige) und F (C38, Ent-4, Ent-5, C23, C6u und
      fünf weitere K-Punkte), je ein OTA
- [x] **STM32** (`src/**`) — S, ein Flash (E17, A5 samt Nachkommabit, gemessene K-Punkte);
      F flasht denselben Stand als Probe; **C4 ohne Flash**
- [x] **Werkzeug** (`tools/**`) — Proben (P2, P3), Prüfstände und S15 (S), Prüfstand C38,
      Smoketest, C23-/C25-Leser (F), K.W; **B19 und E2 erledigt**
- [x] **Dokumentation** — `BEFUNDE.md` je Runde; **E17** in `CLAUDE.md` (Lead) und
      `knowledge/` (`doc-writer`); **L-Befund Nachkommabit** (Lead legt an)
- [x] **Build/Release** — je Einspielschritt ein Release-ZIP; dazu **serielle Testbauten**
      (S) und **Gleichheitsbauten** (C4) ohne Release

---

## Entscheidungen

Stand **07.10.2026** — **alle entschieden**.

- **Ent-1 — Freigabe. Entschieden: freigegeben** für P2, S, F und K.
- **Ent-2 — Flash. Entschieden:** Reserve **1'024 Byte**; Reihenfolge A20 → A13+A47 →
  A3+A48 → A24 → A45 → A14 → A46 → A49 → **A9**; bei A20, A13+A47, A3+A48 **anhalten und
  vorlegen**, die übrigen **zurückstellen**. **A5** steht vor diesen Punkten und wird
  ebenfalls angehalten und vorgelegt — Festlegung dieser Spec (`design.md` §7.6).
- **Ent-3 — Wortlaut B27. Erledigt.**
- **Ent-4 — Kennung 6 trennen. Entschieden: ja** (F.2).
- **Ent-5 — API-Auswahlliste bei 65535. Entschieden: ja, leere Liste** (F.3).
- **Ent-6 — A16 samt Spiellauf. Entschieden.**
- **Ent-7 — Serielle Testbauten. Entschieden vom Lead: ja.** R1 soll paralleles Bauen
  verhindern, und das bleibt gewahrt; „einmal am Ende" stammt aus einer Zeit ohne Flash-Gate.
- **Ent-8 — Minusgrade an der Uhr. Entschieden:** Minusgrade **nur in PWA und Legacy**; die
  Uhranzeige bleibt, wie sie ist, und zeigt bei Minusgraden 0 °C. A5 bleibt damit in S.

**Weitere Entscheidungen vom 07.10.2026, eingearbeitet:** RTC-Nachkommabit mit A5 korrigieren,
falls S.7 es bestätigt (S.18b); A9 ungeklammert (S.19i); C23 UTF-8 plus Escapen (F.5i); C6u
in F (F.5j); C2 und C9c4 geschlossen (P2.9); C4 als eigener Schritt; **B19 und E2 erledigt**
(`e121058`, `8dfba36`); E17 mit S (S.15b–d); **P2.1h** neu; **P2.2/P2.3 entfallen**, B17
Spur 3 in S.11; **Massnahme 4 nur für die vier vorbefüllten Netzwerkfelder**; **E3 offen**.
