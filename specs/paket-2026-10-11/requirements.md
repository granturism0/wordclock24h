# Anforderungen — Paket 2026-10-11 „Nachtfenster, Netzzeit, Spiele, I²C, Brücke" (A23, A69, A53, A57, A60, A61, A62, A63, A35, A21, A34, C35)

**Status: freigegeben** — vom Nutzer am 11.10.2026 (Ent-16), samt Ent-1 bis Ent-15 (unten,
„Entscheidungen"). **Zwei Entscheidungen gegen die Empfehlung:** Ent-3 (Ära-Rechnung jetzt) und
Ent-7 (C). **Ent-3b** (Bestätigungsregel, aus Ent-3) und **Ent-14e** (eigene Freigabe der
LDR-Abweisungsprobe AKE.2) am selben Tag entschieden. **Ent-17 (Flash-Gate, 11.10.2026): „Release
teilen"** — STM-Release 1 enthält A62, A69 und A68, dazu die Korrekturen M1 und M2 aus dem Review
S1 und die vorgezogene Einsparung N1; A23, A53, A57 (C) und A67-STM folgen in einem eigenen
STM-Release 3 nach einer Flash-Analyse. **Ent-18:** Vor dem Release-Build S1 läuft ein
Komplettbuild aller 72 STM-Varianten; das Flash-Gate gilt für alle 72 — **erster Lauf grün
(72/72), engste Variante `wc24h-stm32f103-sk6812-rgbw` mit 1'416 Byte frei**. **Offen:** der
zweite STM-Reset im Aus-Fenster in G3 — **vor G3 beim Nutzer zu bestätigen**.
**Erstellt:** 2026-10-11, Stand `5c34d80` (Zweig `pwa-decoupling`); am selben Tag mit den
Entscheidungen, den Flash-Messungen von S1, dem Review S1 und dem Matrix-Lauf nachgeführt.
Ausgangsstand am Gerät laut `specs/paket-2026-10-10/` (Endstand): STM 3.2.28, ESP 3.2.31,
PWA 1.4.95.
**Auslöser:** Auftrag des Nutzers vom 11.10.2026 — die zwölf Einträge A23, A69, A53, A57, A60,
A61, A62, A63, A35, A21, A34 und C35 aus der ToDo-Liste in `BEFUNDE.md`.
**Grundlage:** zwei Analysen des `firmware-analyst` vom 11.10.2026 (STM und Brücke), vom Lead
zusammengefasst; der `spec-writer` hat die tragenden Fundstellen stichprobenweise am Code
nachgelesen und für Ent-3 und Ent-7 eigens nachgeprüft.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** Zeilennummern gelten für `5c34d80`, sie sind Fundstellen, keine Anker; der
Arbeitsbaum hat sich seither durch S1 verschoben.
Kennzeichen: **✔ SW** — vom `spec-writer` am Code nachgelesen; **✔ A** — in der Analyse als
[BELEGT] gemeldet, nicht einzeln nachgelesen; **◐** — [PLAUSIBEL], nicht belegt; **●** —
gemeldet bzw. [SPEKULATION], vor der Umsetzung zu prüfen.

**Kennungen.** Schritte: **0** (ohne Firmware), **S1** (STM-Release 1: A62 mit Ära, A69, A68, N1,
M1, M2), **S2** (STM-Release 2, isoliert: A60 mit L360), **S3** (STM-Release 3: Flash-Analyse und
Einsparungen, dann A23, A53, A57, A67-STM), **E** (ESP-Release: A35, NTP, `ovr=`, A67-ESP), **P**
(PWA-Release: C35), **M** (Abschlusslauf A21), **T** (Testplan und `pwa-tester`), **Z** (Doku und
Spec-Endstand). Gerätenachweise **G1** (S1), **G2** (S2), **G3** (S3), **G4** (E), **G5** (P),
**G6** (M). Entscheidungen **Ent-1 bis Ent-18**, dazu **Ent-3b** und **Ent-14e**.
Akzeptanzkriterien: **AKN** (Netzzeit, A62), **AKW** (Nachtfenster, A69 samt M1/M2), **AKB**
(Ambilight-Kopie, A68), **AKP** (Platz schaffen: N1, S3), **AKD** (Displayfarbe, A23), **AKS**
(Spiele, A53), **AKE** (Einmessen, A57), **AKK** (OTA-Ankündigung, A67), **AKI** (I²C, A60),
**AKQ** (Übernahme, A35), **AKU** (NTP-Antwort), **AKO** (ESP-Empfangsüberlauf), **AKC** (C35),
**AKA** (Abgleich-Auswertung, A61/A34), **AKM** (Abschlusslauf, A21), **AKZ** (alle; **AKZ.M**
Matrix-Lauf). Die Kürzel gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Gerätenachweis |
|---|---|---|---|---|
| **0** | A61-Auswerteregel als Werkzeug; Abgleichsdauer aus vorhandenen Mitschnitten (A61, A34); A34 schliessen, A63 als Beobachtung; **L391** NTP-Puffer, Berichtigungen an L331, L360, L371, L233; Testplan V3/S1 um den Nachtzustand | Werkzeug, Doku | — | — (lesend, Mitschnitte) |
| **S1** | **A62** Netzzeit mit **Ära-Rechnung** und Bestätigungsregel · **A69** Nachtfenster samt **M1** (leere RTC) und **M2** („IP erst zeigen, dann aus") · **A68** Ambilight-Kopie · **N1** malloc-Stubs (Einsparung) · **Matrix-Lauf aller 72 Varianten** vor dem Release-Build | STM | **zuerst** | **G1** — Netzzeit, Reset im Aus-Fenster, Vorher-Messung für S2 |
| **S2** | **A60 + L360** I²C-Fristen — **eigenes Release, eigene Messung** | STM | nach G1 | **G2** |
| **S3** | **Flash-Analyse** und verhaltensneutrale Einsparungen, dann **A23** Displayfarbe · **A53** Spiele · **A57 (C)** Einmessen · **A67-STM** (Ent-17) | STM | nach G2 | **G3** — Farbanimation, Spiel, LDR-Abweisung; zweiter Reset im Aus-Fenster nur nach Bestätigung |
| **E** | **A35** (b) · **NTP-Puffer und Plausibilität** (nach Ent-3 tragend) · **`ovr=`** · **A67-ESP** | ESP | nach G3 | **G4** |
| **P** | **C35** (A) | PWA | nach G4 | **G5** |
| **M** | **Abschlusslauf A21** | — | nach G5 | **G6** |
| **T** | `TESTPLAN-PWA.md` nachführen; `pwa-tester` Phasen 0–4 und 9 (DIR-012) | — | nach G6 | — |
| **Z** | `BEFUNDE.md` (A57-Schliessung nach G3, Restpunkte aus Review S1), `knowledge/architecture-checklist.md`, `CLAUDE.md`, `README-CMAKE.md` (Matrix), Spec-Endstand | Doku | — | — |

**Warum diese Reihenfolge:** `design.md` §0. Kurz: `make release-zip` baut STM, ESP und die
PWA-Assets **immer zusammen**; darum entsteht der Code des nächsten Releases erst **nach dem
Release-Build** des vorigen, und jeder Release-Build erst **nach dem Gerätenachweis** des vorigen.
**S2 vor S3**, weil A60 isoliert messbar bleiben soll und die Flash-Analyse gegen die endgültige
Map arbeiten soll; **S3 vor E**, weil die ESP-Hälfte von A67 die STM-Hälfte voraussetzt.
**Firmware vor PWA** (DIR-002). `CHANGELOG.md` je Release schreibt der Lead.

---

## Problem

### 0. Flash F103 — gemessen statt geschätzt

**✔ (Endstand Paket 2026-10-10):** Ausgang **1'908 Byte frei**, Gate **≥ 1'024**. **Gemessen am
11.10.2026 (Testbauten des Leads, F103 WC12h SK6812-RGBW):**

| Stand | frei | Δ | geschätzt |
|---|---|---|---|
| nach S1.1 — A62 samt Bestätigungsregel | 1'700 | +208 | −110…−20 |
| nach S1.3 — A69 samt Sprüngen | 1'132 | +568 | +160…+310 |
| nach S1.5 — A68 | 1'088 | +44 | +10…+30 |
| nach S1.9 — N1 malloc-Stubs | 1'756 | −668 | — |
| nach S1.11 — M1 + M2 | **1'608** | +148 | 80–100 |
| **Matrix-Lauf S1.16, engste Variante `wc24h-stm32f103-sk6812-rgbw`** | **1'416** | — | — |

`strtoul` und die malloc-Module (`mallocr`, `freer`, `reallocr`, `msizer`, `sbrkr`, `mlock`) sind
aus der Map verschwunden, keine 64-Bit-Routinen. **Die Schätzungen lagen um den Faktor 1,5 bis 3
daneben** (bei A62 sogar im Vorzeichen). Daraus Ent-17 und der Grundsatz in `design.md` §1.2:
**Geplant wird mit der Messung je Task, nicht mit Summen von Schätzungen** — und, seit dem
Matrix-Lauf, **ab der engsten Variante** (1'416), nicht ab dem Teilbuild (1'608).

### 1. A62 / L371 — Zeittext vom ESP ohne Prüfung; Verfallsdatum 2036

**✔ SW:** `src/main.c:3274` liest die Netzzeit mit `strtoul (esp8266.u.time, &endptr, 10)` und
**prüft `endptr` nicht**. **Berichtigung an L371 (✔ A):** Fundstelle `main.c:3274`, nicht
`~:3228`. **Leer sendet der ESP nie.** Realistische Auslöser: der Wert **0** (`ntp.cpp` prüft
weder LI noch Stratum noch Transmit, ◐ Kiss-of-Death), eine **verstümmelte Zeile** (`TIME` trägt
keine Prüfsumme), **mehr als 10 Ziffern** (`strtoul` sättigt). Die Zeitzone wird beim Laden nicht
begrenzt (`timeserver.c:67`), der Setter begrenzt auf ±12 (`:196`) — ✔ SW.

**Für die Ära-Rechnung (Ent-3), ✔ SW:**

- Der ESP sendet `TIME` + `Serial.println(unsigned long)` (`ntp.cpp:78-83`) — **dezimal ohne
  führende Nullen**. Nach dem 07.02.2036 06:28:16 UTC beginnt die Zahl wieder bei 0; Ära-1-Werte
  haben **1 bis 10 Ziffern** (10 Ziffern erst ab 2067).
- Der ESP rechnet sonst **nicht** mit der Zeit (kein `gmtime`, `time_t`, kein Zeitstempel in
  `weather.cpp`); die Uhrzeit für Anzeige und API kommt vom STM.
- Der STM rechnet `(time_t) (seconds_since_1900 - 2208988800U)` (`timeserver.c:253`) — die
  Differenz zweier `uint32` läuft modulo 2³² um und ergibt für Ära-1-Werte **bereits die richtige
  Unixzeit**. `my_gmtime()` rechnet seit S0 mit `uint32_t` (`base.c:461`) — gültig bis
  2106-02-07, ohne 64-Bit-Code.
- Die RTC speichert das Jahr zweistellig (`rtc.c:204`, `:241`): **Die Kette endet am 31.12.2099.**

Das 2036-Problem war also die **Gültigkeitsfrage**, nicht die Arithmetik. Mit dem Pivot werden
aber zwei Fehlerbilder gültig: die **0** (= 2036-02-07 06:28:16) und **eine verstümmelte Zeile**
(fällt aus `4000648905` eine Ziffer, entsteht `400064890` = eine Zeit im Oktober 2048). Eine feste
Zeitgrenze am STM hilft dagegen nicht (`design.md` §2.3) — daher die ESP-Plausibilität (tragend)
und die Bestätigungsregel (Ent-3b).

### 2. A69 / L390 — Display bleibt nach einem Neustart im Nacht-Aus-Fenster an

**✔ SW:** `night_check_night_times()` greift nur bei exakter Minutengleichheit
(`src/night/night.c:155` — **nicht `:156`**). Aufgerufen nur aus dem `show_time_flag`-Zweig
(`main.c:4286`, `:4300`). Der Startpfad schaltet das Display bedingungslos ein (`main.c:3599`,
✔ A), liest die RTC (`:3820-3833`, ✔ A) und wertet die Timer **nie** aus. **Zusätzlich, ✔ A:**
Springt die Uhr **über** eine Timer-Minute (Sommerzeit, NTP-Korrektur, Stillstand), entfällt der
Timer. `minutes == 1440` ist zulässig und unwirksam (✔ SW). Ein **Aus**-Timer schaltet das
Ambilight mit (`main.c:4290`), ein **Ein**-Timer nicht (`:4294`) — ✔ SW.

**Aus dem Review S1 (11.10.2026, BESTANDEN mit zwei mittleren Punkten):**
- **M1:** Die RTC gilt bei leerer Batterie als gültig und liefert 2000-01-01 — eine Auswertung
  beim Start hätte das Fenster gegen ein falsches Datum bestimmt.
- **M2:** Ent-2 (4) „Startsequenz wie heute, danach aus" war so nicht erfüllt: Der IP-Lauftext
  kommt erst nach `IPADDRESS`, und `display_set_ticker` tut bei ausgeschaltetem Display nichts —
  ein sofortiges Ausschalten beim Start hätte den IP-Lauftext unterdrückt.

### 3. A68 / L389 — ESP-Kopie der Ambilight-Power veraltet

**✔ SW:** `set_display_power()` (`main.c:1425-1456`) schaltet bei `do_sync_ambilight` das
Ambilight mit, sendet aber nur `var_send_display_power()`. Die Sendefunktion der Ambilight-Power
ist `static` (`vars.c:1324`, laut BEFUNDE). A69 erzeugt denselben Fehler beim Start.

### 4. A23 / L150 — Farbanimation überschreibt die Displayfarbe

**✔ A:** Regenbogen und Tageslicht schreiben `display.display_colors` direkt (`display.c:3157-3173`,
`:5348-5418`, `:5458-5468`); beim Wechsel auf Modus 0 (`:4456-4473`, `:4480-4527`) bleibt die
Momentanfarbe stehen; die Sicherung wie beim Ambilight (`:3557`, `:6172`) fehlt. Kein
EEPROM-Schreiben im Animationspfad, aber die **ESP-Kopie** wird laufend überschrieben
(`:1211-1220`). ◐ Nach einem STM-Reset müsste die Nutzerfarbe zurückkommen — G3 klärt es, sofern
der zweite Reset bestätigt wird.
IR-Farbschritte ändern die Farbe im Modus 0 ohne EEPROM (`main.c:4911-4955`).

### 5. A53 / L316 — ESP-Zeilen während Tetris und Snake

**✔ SW:** Die Spielschleifen verwerfen alles ausser `GT`/`GS` (`tetris.c:655-680`; `snake.c:240-265`
✔ A). Verloren gehen Setter der PWA, die Rückgabe von `IPADDRESS`/`SYNCVARS` (◐ schwerer Pfad aus
A66) und die Quittung eines wartenden Kommandos (✔ A). „Mit A16 zusammen" ist überholt.
**Nebenbefund, ✔ SW:** `snake.c:291` `STATUS status;` uninitialisiert.

### 6. A57 / L331 — LDR-Minimum über Maximum

**Berichtigung an L331 (✔ SW):** Der Schluss „im EEPROM stand also nicht 18/17" **trägt nicht** —
der Start heilt `max <= min` auf 0/4095 **und schreibt das** (`ldr.c:128-137`). **✔ SW:**
`ldr_set_min_value()`/`ldr_set_max_value()` (`ldr.c:168-226`) übernehmen den Messwert ungeprüft
gegen die andere Grenze. Der RPC-Zweig meldet danach den aktuellen Wert an den ESP
(`main.c:1609`, `:1617`). Die API antwortet `{"ok":true}` **vor** der Messung
(`http.cpp:10077-10086`), die PWA meldet „gespeichert" (`app.js:13100`, `:13266`). ◐ Ursache:
S25/S26 bei gleichem Licht.

### 7. A67 / L380 — der STM sendet während des ESP-Downloads weiter

**✔ SW:** `ESPhttpUpdate.update()` blockiert synchron (`http.cpp:13009`; dazu `:7590`, `:13402`,
✔ A). `CAP var-crc` darf nicht als Ankündigung dienen (`esp8266.c:550`, ✔ A, Risiko R-3).

### 8. A60 / L359 + L360 — I²C-Treiber

**✔ SW:** `i2c_wait_for_flags()` ruft `delay_msec(1)` (`i2c.c:157`, `:173`), Frist 5 (`:26`);
**zwei** BUSY-Schleifen ohne Frist (`:413` Lesen, `:472` Schreiben). **Berichtigung an L360:** vor
`watchdog_init()` (`main.c:3836`) steht die Uhr bei hängendem BUSY **für immer**, nicht nur bis zum
Watchdog-Reset. Gerechnet: `n=2` von rund 24 auf 17 ms, 32 Byte von rund 110 auf 35–45 ms.

### 9. A61 / L361 und A34 / L234 — Vollabgleich

**✔ A:** Stelle heute `main.c:4037-4048`; Auslöser, Dauer und Zeitüberschreitungen sind **schon
aus dem Mitschnitt** messbar. A34: Tasks 1–3, 12 erledigt; 11 überholt; 13 ⇒ A19; 14 ⇒ A53; Task 10
im Kern überholt, übrig der Stillstand **während** eines Abgleichs.

### 10. A63 / L373 — 1-Wire-Leseslot

**✔ A:** Abtastung 15–25 µs; bisher kein `ungueltig` (G1 60/60, T.2 77/0).

### 11. A35 / L233 — Quittung bestätigt Empfang statt Übernahme

**✔ A:** Verfälschung und Zuordnung STM→ESP sind gelöst. Offen nur: `var_set_parameter()` liefert
1, auch wenn die Zuweisung still entfällt (Index, Bereich, unbekannter Buchstabe, `vars.cpp:971`,
`:1503`). **Berichtigung an L233:** Fundstelle `ESP-uclock.ino:1468-1479`; `a=9` vom 10.10. war
echt (vor STM 3.2.24). Kein hoher Befund mehr.

### 12. Neuer Befund L391 — NTP-Antwort überschreibt den Puffer (ESP)

**✔ SW:** `ntp.cpp:74` liest `packetsize` Byte in den 48-Byte-Puffer (`:18`) und prüft die Grösse
erst danach (`:76`). Eine Antwort über 48 Byte überschreibt globale Variablen. **Vom Netz
auslösbar.**

### 13. A21 / L141 — Brücke überlastet

**✔ A:** Mit Gerätedaten nicht mehr belegt (`d=0` über 47 h, T.2, Messpaket). **Blinder Fleck
(✔ SW):** Der ESP-Empfangspuffer ist nicht gemessen (kein `hasOverrun`).

### 14. C35 / L310 — zwei ungecachte Abrufe je `update_status`

**✔ A:** Release Notes und WC-Liste bei **jedem** Aufruf (`http.cpp:12771-12839`); die PWA ruft
`update_status` auch im Hauptmodul alle 15 s (`app.js:2131`, `:3187`, ✔ SW) — rund 8 ausgehende
Verbindungen je Minute bei einem offenen Fenster.

### 15. Nur eine von 72 STM-Varianten wurde gebaut und gemessen

Das Flash-Gate wurde bis zum 11.10.2026 nur am Teilbuild F103 WC12h SK6812-RGBW gemessen. Das
Original-ZIP enthält **72 HEX-Dateien**: {wc12h, wc24h} × {F103, F401CC-25/-8, F411CE-25/-8,
Nucleo401, Nucleo411} × fünf LED-Typen, dazu je F407-TFT (Ent-18). **Ergebnis des ersten
Matrix-Laufs (S1.16):** 72/72 gebaut, keine unter dem Gate; die engste Variante ist
**`wc24h-stm32f103-sk6812-rgbw` mit 1'416 Byte frei** — **192 Byte** enger als der Teilbuild.
Grösse gegen das Original +1,6 bis +7,4 %.

---

## Ziel

1. **Flash:** Jedes STM-Release hält das Gate von 1'024 Byte nach **jedem** Task und nach jeder
   Einsparung; vor dem Release-Build S1 hält es **jede der 72 Varianten**; S3 schafft den Platz für
   seine vier Punkte zuerst, verhaltensneutral und gemessen.
2. **Netzzeit:** Die Uhr nimmt gültige Netzzeiten **bis 2106** an (Kette bis 31.12.2099 durch die
   RTC); eine ungültige, eine 0 oder eine verstümmelte stellt sie **nie** — verworfen und gemeldet.
3. **Nachtfenster:** Nach einem Neustart im Aus-Fenster zeigt die Uhr den **IP-Lauftext und ist
   danach aus**, ohne auf den nächsten Timer zu warten; eine leere RTC bestimmt das Fenster nicht;
   übersprungene Timer werden nachgeholt; die ESP-Kopie stimmt.
4. **Displayfarbe:** Animation ein und aus gibt die vorherige Farbe zurück.
5. **Spiele:** Verworfene Zeilen sind sichtbar und durch einen Vollabgleich geheilt.
6. **Einmessen:** „Messwert übernehmen" erzeugt keine verdrehten Grenzen mehr; der Zahlen-Setter
   bleibt unverändert.
7. **I²C:** Keine Warteschleife ohne Frist; EEPROM-Schreiben messbar schneller.
8. **Brücke:** Still verworfene Zuweisungen gezählt und gemeldet; ESP-Empfangsüberlauf gezählt;
   A21 gemessen geschlossen oder neu gefasst.
9. **NTP:** Keine Antwort überschreibt den ESP-Speicher; unbrauchbare Antworten erreichen den STM
   nicht.
10. **Update-Abfrage:** Hauptmodul höchstens ein `update_status` je 5 Minuten.
11. **Ohne Firmware:** A34 geschlossen, A63 als Beobachtung, A61 gemessen, Befundzeilen berichtigt.
12. Jede Wirkung **einzeln** nachgewiesen, jede neue Prüfung **einmal fehlgeschlagen** (DIR-014).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt**
(DIR-014). Prüfstände liegen unter `tools/checks/auszug/`, laufen über `tools/checks/auszug.sh`
mit den Typbreiten des Ziels (L256) und sind **einmal gegen den Vorgängerstand
fehlgeschlagen**; die Fallzahl wird gemeldet und nachgezählt. **Was am Gerät nicht herstellbar
ist, wird nicht am Gerät verlangt.** Logzeilen im STM in Umschrift (DIR-015); der Wortlaut ist
verbindlich, wo er hier steht, weil die Logwache danach sucht.

### Schritt 0 — ohne Firmware

- [ ] **AKA.1 (A61-Auswerteregel)** — Ein Werkzeug des Leads meldet je Vollabgleich Auslöser
      (`SYNCVARS`, `IPADDRESS`, CMC-Abweisung, unbekannt), Dauer, Δ`v` (beide Zahlen), Zahl der
      `var_send_buf: keine Quittung`, Stillstand (Δ`u`). *Gegenprobe:* synthetischer Mitschnitt;
      Sabotage (falscher Auslöser) meldet FEHL mit Exit ≠ 0 (L191). **Zähl nach** gegen
      `grep -c 'info: configuration sent'`.
- [ ] **AKA.2 (Messung)** — Mit AKA.1: ESP-OTA vom 10.10.2026 23:06, STM-Reset vom 11.10.2026
      01:58:14 und jeder weitere Abgleich im Langzeitmitschnitt. **Regel A34 (Ent-9):** über 5 s
      Stillstand ⇒ Task 10 als **neuer Befund**, nicht hier gebaut.
- [ ] **AKA.3 (Befunde)** — `BEFUNDE.md`: **L391** NTP-Puffer mit Massnahme; Berichtigungen an L331,
      L360, L371 (samt Ära-Beschluss und Grenze 2099), L233 und an den Fundstellen von A69, A61,
      A62, A53; **A34 geschlossen** mit Beleg; **A63** offen als Beobachtung; **A57 offen** mit
      dem Vermerk „Ent-7 (C), Schliessung nach G3". *Instrument:* S10 und S11 grün.
- [ ] **AKA.4 (Testplan V3/S1)** — V3 und S1 nennen das **Soll im Nachtzustand** (nach einem Reset
      im Aus-Fenster ab STM-Release 1: IP-Lauftext, danach Display aus; ohne IP spätestens nach
      60 s) und die Zeile `time: sprung, bestaetigung angefordert` nach jedem Reset als Soll.
      **Vor G1 erledigt.**

### Schritt S1 — A62 (Netzzeit mit Ära)

- [ ] **AKN.1 (Parser und Gültigkeit)** — Der `ESP8266_TIME`-Zweig nimmt **1 bis 10 Ziffern** und
      dann das Nullbyte an; ein Wert über 4'294'967'295 (Überlauf) ist ungültig. Gültig ist eine
      Zeit genau dann, wenn `u = (uint32) (n − 2'208'988'800) ≥ 1'767'225'600` (Unix 2026-01-01) —
      damit gilt Ära 0 von 2026-01-01 bis 2036-02-07 06:28:15 und **Ära 1** (n < 2'208'988'800) von
      2036-02-07 06:28:16 bis 2106-02-07 06:28:15; der Totbereich
      `[2'208'988'800, 3'976'214'400)` wird verworfen. Ungültig ⇒ keine Änderung an Uhr und RTC,
      Logzeile **`time: ungueltig, verworfen`**, Countdown wie beim Anfordern gesetzt (3800 s).
      `strtoul` entfällt. *Instrument:* Prüfstand **a62**: `4294967295`, `0`, `1`, `123`,
      `2208988799`, `2208988800`, `3976214399`, `3976214400`, `4000648905`, `""`, `" 1"`, `"+1"`,
      `"1x"`, 11 Ziffern, `4294967296`, `9999999999`. **Referenz unabhängig vom C-Code:** Python
      `datetime` (`unix = n − 2208988800 + 2³²` für n < 2208988800), dazu die Felder über **gt**.
      **Gegenprobe:** alter Stand nimmt `""` an ⇒ FEHL; Sabotagen „Totbereich entfernt" und
      „genau 10 Ziffern" ⇒ FEHL.
- [ ] **AKN.2 (Zeitzone beim Laden)** — Ein geladener Betrag über 12 wird nicht übernommen
      (Vorgabe, Logzeile). *Instrument:* a62, `tz[1]` 0, 12, 13, 255 mit beiden Vorzeichen.
- [ ] **AKN.3 (kein 64-Bit-Pfad)** — Die Umrechnung NTP→Unix bleibt die `uint32`-Differenz
      (`timeserver.c:253`); keine neue 64-Bit-Arithmetik. *Instrument:* Review S1.8; AKN.4.
- [ ] **AKN.4 (Map)** — In der Map des F103-Testbaus steht kein `strtoul` mehr und weiter **kein**
      `__udivmoddi4`/`__aeabi_ldivmod`; Guardrail **S8b** weist es nach. **Gegenprobe:** gegen die
      Map von 3.2.28 schlägt die `strtoul`-Prüfung an. **Stand 11.10.2026:** im Testbau nach S1.1
      erfüllt (Lead).
- [ ] **AKN.5 (Bestätigungsregel, Ent-3b)** — Eine gültige Zeit wird sofort übernommen, wenn
      sie um höchstens 600 s von der fortgeschriebenen letzten übernommenen Zeit abweicht; sonst
      erst, wenn eine zweite Antwort sie auf ±5 s (fortgeschrieben über `uptime`) bestätigt. Bis
      dahin Logzeile **`time: sprung, bestaetigung angefordert`**, Schnellabfrage nach 15 s,
      höchstens drei in Folge. Nach einem Start gibt es keine Referenz: Die erste Antwort braucht
      immer die Bestätigung. *Instrument:* a62 mit simulierter `uptime`: Start + n + (n+15) ⇒
      übernommen bei der zweiten; Start + 0 + 0 ⇒ nie; stündlich ±30 s ⇒ sofort; ein Fehlwert ⇒
      verworfen, danach regulär; vier Fehlwerte ⇒ drei Schnellabfragen; Abgleich über die
      Ärengrenze ⇒ sofort. **Gegenprobe:** Sabotage „Bestätigung entfernt" ⇒ 0 nach dem Start
      angenommen ⇒ FEHL. **Umsetzung (Lead, 11.10.2026), im Rahmen von Ent-3b:** Schnellabfrage
      nach 15 s umgesetzt (`net_time_countdown = 15`, höchstens 3). Zusätzlich verlangt die
      Bestätigung einen **Mindestabstand von 10 s** zum Kandidaten — strenger als die Regel: zwei
      Antworten in derselben Sekunde bestätigen sich nicht. `cmd: set time to` wird **unbedingt**
      geloggt — Beobachtbarkeit für AKN.6, keine Entscheidung. Der Prüfstand a62 enthält einen
      Fall „zwei Antworten im Abstand unter 10 s ⇒ keine Bestätigung". **Review S1.8:
      BESTANDEN.**
- [ ] **AKN.6 (Gerät, G1, lesend)** — Nach dem S1-Flash: `time: sprung, bestaetigung angefordert`,
      danach `cmd: set time to …`; die nächste stündliche Netzzeit **ohne** Bestätigung;
      `read rtc` gleich der Wanduhr ±2 s; keine Zeile `time: ungueltig`.
- [ ] **AKN.7 (Gerät, G1, Reset im Fenster)** — Beim Reset aus AKW.4 dasselbe Bild wie AKN.6.

### Schritt S1 — A69 (Nachtfenster, samt M1/M2) und A68 (Ambilight-Kopie)

- [ ] **AKW.1 (geltendes Ereignis)** — Eine Funktion in `night.c` bestimmt das **jüngste
      vergangene**, aktive, für den Wochentag gültige Ereignis über 8 Timer × k = 0…7 Tage; die
      Wochentagsprüfung aus `night.c:166-180` wird **eine** Hilfsfunktion. *Instrument:* Prüfstand
      **a69** gegen eine Minuten-Referenz mit `night_check_night_times()`, zufällige Tabellen,
      **alle 10'080 Startpunkte** je Tabelle, Sonderfälle. **Gegenprobe:** Sabotagen `<=` → `<` und
      Wochentagsumlauf ⇒ FEHL. **Zähl nach:** Tabellen × 10'080.
- [ ] **AKW.2 (beim Start, Ent-2 (1), (2), (4), M1)** — Nach dem RTC-Lesen und vor
      `watchdog_init()` wird das Ergebnis **für die Displayliste** bestimmt — **nur wenn
      `gmain.year >= 2026`** (M1: eine RTC mit leerer Batterie meldet 2000-01-01 und gilt sonst als
      gültig). Andernfalls wertet das **erste bestätigte** `TIME` einmal aus. Liegt das Ergebnis
      auf „aus", wird beim Start **nicht sofort** geschaltet, sondern nach AKW.6 vorgemerkt.
      *Instrument:* a69 (Startpfad mit Attrappen, Fälle Jahr 2000 und 2026), Review S1.8,
      Nachprüfung S1.13.
- [ ] **AKW.3 (Zeitsprünge, Ent-2 (3))** — Springt die Uhrzeit **vorwärts** um mehr als eine
      Minute, wird das jüngste Ereignis **im übersprungenen Intervall** angewandt, und nur dieses;
      rückwärts nichts; ohne übersprungenes Ereignis bleibt ein manueller Zustand. *Instrument:*
      a69: Sprung über Aus-Timer ⇒ aus; ohne Timer ⇒ unverändert; rückwärts ⇒ unverändert;
      Sommerzeit 02:00 → 03:00 mit Timer 02:30. **Gegenprobe:** alter Stand verliert den Timer ⇒
      FEHL.
- [ ] **AKW.6 (M2 — „IP erst zeigen, dann aus", Ent-2 (4))** — Ergibt die Auswertung beim Start
      „aus", wird **vorgemerkt** (`night_off_due`). Angewandt wird **nach dem IP-Lauftext**, im
      Innenzweig der **unveränderten** Restore-Bedingung um `pending_ticker_restore`; kommt keine
      IP, nach einer **Frist von 60 s**. **Jedes** Schalten des Displays vorher verwirft die
      Vormerkung. Angewandt wird wie beim Timer (`set_display_power (FALSE, TRUE)`) mit der
      Logzeile `night: start aus (hh:mm)`. *Instrument:* a69 (erweitert): Start im Aus-Fenster mit
      IP ⇒ aus erst nach dem Restore; ohne IP ⇒ aus nach 60 s; Schalten vor dem Restore ⇒ keine
      Wirkung der Vormerkung; `git diff` zeigt die Restore-Bedingung unverändert.
      **Nachprüfung S1.13: BESTANDEN.**
- [ ] **AKB.1 (A68)** — Jede Stelle, die die Ambilight-Power ändert, sendet sie bei
      `esp8266.is_online`; Sendefunktion öffentlich, **keine zweite**; **Bestandsliste**
      (Datei:Zeile). *Instrument:* a69 (Sendeprotokoll). **Gegenprobe:** alter Stand ⇒ FEHL.
      **Stand 11.10.2026 (Lead):** umgesetzt (S1.5), Testbau +44 Byte; Review S1.8 BESTANDEN.
- [ ] **AKW.4 (Gerät, G1 — Reset im Aus-Fenster, Ent-14 a)** — Ein eigener STM-Reset nach dem
      Aus-Timer des Nutzers, nach den Tagtests von G1. Am Display: **IP-Lauftext, danach aus**. Im
      Mitschnitt: der IP-Lauftext, danach `night: start aus (hh:mm)` und
      `set_display_power: new=0 …` (diese Logzeile ist **geschützt**, AKP.1), in der folgenden
      Minute `show_time: … p=0 …`; nach dem Vollabgleich Display-Power = 0 und Ambilight-Power
      gleich dem STM (`settings_xml`). **Vorher-Wert:** L390. In G3 erneut, falls der zweite Reset
      bestätigt wird.
- [ ] **AKW.5 (Gerät, lesend)** — Am Morgen schaltet der Ein-Timer wie bisher.

### Schritt S1 — N1 und Schritt S3 — Platz schaffen (Ent-17)

- [ ] **AKP.0 (N1 — malloc-Stubs, in S1 vorgezogen)** — Starke Definitionen von `_malloc_r`,
      `_realloc_r` und `_free_r` in `src/stubs/stubs.c` mit `__attribute__((used,
      externally_visible))` (ohne das Attribut verwirft LTO `_realloc_r`, Linkfehler belegt). In
      der Map fehlen danach `mallocr`, `freer`, `reallocr`, `msizer`, `sbrkr`, `mlock`.
      Verhaltensneutral; alle bestehenden Prüfstände weiter OK; **keine neue Compilerwarnung**
      (die 7 Warnungen `unused parameter` beseitigt S1.16a mit `(void)`). *Instrument:* Map des
      Testbaus (Lead), Reviews, Warnungsbestand. **Stand 11.10.2026:** −668 Byte (1'756 frei), beide
      Reviews BESTANDEN; Warnungen in Arbeit.
- [ ] **AKP.1 (Flash-Analyse, gemessen)** — Der `firmware-analyst` legt aus der **Map des
      S2-Builds** — und der Map der **engsten Matrix-Variante** — eine Liste der Einsparkandidaten
      vor: je Kandidat Symbol bzw. Text, Abschnitt, **Bytes laut Map** (nicht geschätzt),
      Verhaltensänderung ja/nein und **Abhängigkeiten** — sucht ein Werkzeug (`watch-log.sh`,
      Auswerteregel 0.1, Smoketest), der Testplan, ein Akzeptanzkriterium dieser Spec oder
      `BEFUNDE.md` nach der Zeile? Mindestens geprüft: der tote Zweig `FILE ` (A52), doppelte und
      lange Texte, die **zweite** `log_printf` in `set_display_power()` (am Ende der Funktion),
      Debugtexte im normalen Bau — mit Beleg an der Map, ob sie überhaupt im Fabrikat landen.
      **Geschützt und kein Kandidat:** `set_display_power: new=…` (Nachweis in AKW.4). **Nicht mehr
      Kandidat:** N1 (AKP.0). *Instrument:* Review des Leads; die Summe steht neben dem Bedarf aus
      `design.md` §1.3.
- [ ] **AKP.2 (Einsparungen, einzeln gemessen)** — Der `stm-developer` setzt die Einsparungen
      **einzeln** um, nach jeder ein Testbau mit dem **gemessenen** Gewinn neben der Map-Grösse.
      Nur verhaltensneutrale; eine Einsparung mit Verhaltensänderung oder an einer Zeile mit
      Abhängigkeit aus AKP.1 **nur nach Rückfrage beim Nutzer**. *Instrument:* Testbau (Lead);
      alle bestehenden Prüfstände (a62, a69, a60, gt, EP, W2, L59) weiter OK; `guardrails.sh` grün;
      Review S3.13.
- [ ] **AKP.3 (keine Sparstufe ohne Rückfrage)** — Die Sparstufen aus `design.md` §1.4 (Ent-2 (3)
      zurück, A67 zurückstellen, A53 V2b, Logzeilen kürzen) werden **nicht** angewandt. Reicht der
      Platz nach AKP.2 für einen der vier Punkte nicht (Testbau unter 1'024), wird angehalten und
      vorgelegt.

### Schritt S3 — A23 (Displayfarbe)

- [ ] **AKD.1 (Sicherung, Ent-1)** — RAM-Sicherung nach dem Muster `saved_ambilight_colors`;
      gesetzt beim Laden **vor** `display_init_color_animation()`, in DC **auch während** einer
      Animation und in den IR-Farbschritten im Modus 0; zurück auf **jedem** Weg auf Modus 0 samt
      `display_calc_dimmed_display_colors()` und **`var_send_display_colors()`**. Kein
      EEPROM-Schreiben im Animationspfad. *Instrument:* Prüfstand **a23** (Regenbogen, Tageslicht,
      Inkrement/Dekrement, DC während Animation, IR-Schritte, Sendeprotokoll). **Gegenprobe:** alter
      Stand ⇒ FEHL.
- [ ] **AKD.2 (Gerät, G3, Ent-14 d)** — **Teil 1, am Tag:** `dspcolor[0]` lesen; Regenbogen ein,
      rund 30 s, aus ⇒ gleich dem Vorher-Wert, Farbsendung im Mitschnitt; dasselbe mit Tageslicht.
      **Teil 2, nach einem zweiten STM-Reset im Aus-Fenster:** `dspcolor[0]` gleich der
      Nutzerfarbe. **Dieser Reset ist vor G3 beim Nutzer zu bestätigen** — Ent-14 (a) galt dem
      einen Reset in G1. Ohne Bestätigung bleibt Teil 2 offen und wird so berichtet. Rückstellung:
      Animationsmodus wie vorher.

### Schritt S3 — A53 (Spiele)

- [ ] **AKS.1 (Ent-4 = V2)** — Beide Spielschleifen zählen verworfene Zeilen (nicht Zeitablauf,
      nicht `GT`/`GS`); danach bei Zähler > 0 `var_sync_pending = 1` und **`spiel: %u Zeilen
      verworfen`**. Keine Inaktivitätsgrenze. *Instrument:* Prüfstand **a53**. **Gegenprobe:** alter
      Stand ⇒ FEHL.
- [ ] **AKS.2 (Snake-Status)** — `snake.c:291` definiert vor dem ersten Lesen. *Instrument:* Review.
- [ ] **AKS.3 (Gerät, G3, Ent-14 b)** — Spiel starten, Helligkeit setzen, Spiel beenden:
      `spiel: n Zeilen verworfen` (n ≥ 1), `info: configuration sent`, `diag` lückenlos, ESP-Kopie
      der Helligkeit auf dem STM-Wert. Rückstellung: Helligkeit wie vorher.

### Schritt S3 — A57 (Einmessen, Ent-7 = C)

- [ ] **AKE.1 (Abweisung)** — `ldr_set_min_value()` übernimmt einen Messwert **≥ Maximum** nicht,
      `ldr_set_max_value()` einen Messwert **≤ Minimum** nicht (Gleichheit abgewiesen, wie
      `ldr.c:128`); Wert und EEPROM bleiben; Logzeile **`ldr: minimum abgewiesen`** bzw.
      **`ldr: maximum abgewiesen`** (mit Messwert und Gegengrenze). Die bestehende Rückmeldung
      (`main.c:1609`, `:1617`) meldet den **alten** Wert an den ESP. Der **Zahlen-Setter (A59,
      `main.c:1907-1914`) bleibt unverändert** und nimmt die Inversion weiter an. *Instrument:*
      Prüfstand **a57** mit ADC- und EEPROM-Attrappe: unter, gleich, über der Gegengrenze, je
      Minimum und Maximum; Schreibzyklen gezählt. **Gegenprobe:** gegen 3.2.28 ⇒ Inversion
      angenommen ⇒ FEHL. **L59 besteht weiter.**
- [ ] **AKE.2 (Gerät, G3, schreibend mit Rückstellung, Ent-14e — ausdrücklich freigegeben)** — Nur
      bei ausgeschalteter Auto-Helligkeit (`numvar[8]=0`) und einem LDR-Rohwert zwischen 2 und 4093:
      Maximum per Zahlen-Setter kurz auf 1, „Minimum übernehmen" ⇒ `ldr: minimum abgewiesen`,
      `numvar[17]` unverändert, **keine** `eep a=225`; Maximum zurück auf 4095. Minimum per
      Zahlen-Setter kurz auf 4094, „Maximum übernehmen" ⇒ `ldr: maximum abgewiesen`; Minimum zurück
      auf 0. Danach 17/18 = 0/4095 (lesend). **Erst danach wird A57 geschlossen** (Z.1).

### Schritt S3 — A67 STM-Hälfte (Ent-12)

- [ ] **AKK.1 (eigene Zeile)** — Eine **neue** Zeile (Wortlaut in S3.10 festgelegt, Vorschlag
      `OTA start`), exakter Vergleich, setzt **nur** `esp8266.is_online = 0`. *Instrument:* Prüfstand
      **a67**. **Gegenprobe:** alter Stand ⇒ FEHL.

### Schritt S2 — A60 und L360 (I²C, Ent-15)

- [ ] **AKI.1 (Fristen)** — Enges Pollen über `delay_counter` mit **derselben** Frist (5 ms);
      **beide** BUSY-Schleifen mit Frist; kein RM0008-Abschlussverfahren. **Voraussetzung**, am Code
      belegt: keine ISR ruft `delay_*`. *Instrument:* Prüfstand **a60**. **Gegenprobe:** Sabotage
      „Frist nie gesetzt" ⇒ FEHL; alter Stand hängt bei BUSY ⇒ FEHL.
- [ ] **AKI.2 (Gerät, G1 vorher / G2 nachher, Ent-14 d)** — S25/S26 (`eep a=225/227 n=2`, über S27
      zurück) und ein 32-Byte-Wert (Wetterort, zurück) auf beiden Ständen. **Erwartung:** `n=2`
      rund 17 statt 24 ms, 32 Byte 35–45 statt rund 110 ms. Abweichung **berichtet**, nicht
      passend gemacht.
- [ ] **AKI.3 (Gerät, G2, lesend)** — `read rtc` ±2 s, DS3231-Temperatur plausibel, **60 Minuten**
      ohne I²C-Fehlerzeile, Exception, Neustart.

### Schritt E — ESP

- [ ] **AKQ.1 (A35, Ent-5 = b)** — Zähler für still entfallene Zuweisungen, `V…` ausgenommen,
      Rückgabe unverändert; gedrosselte Zeile **`- var ignoriert #n code=X idx=yy`** ohne Wert,
      **im Mitschnitt und in `/api/stm32_log`** (`stm32_log_append()` allein reicht nicht,
      `ESP-uclock.ino:1247`). *Instrument:* Prüfstand **a35**. **Gegenprobe:** alter Stand ⇒ FEHL.
      **Zähl nach:** Bedingungen gegen Zählstellen.
- [ ] **AKU.1 (NTP-Puffer, L391)** — Höchstens 48 Byte in den Puffer; längere Pakete verworfen.
      *Instrument:* Prüfstand **ntp** (48, 47, 68, 1'024 Byte; Wächterbyte). **Gegenprobe:** alter
      Stand überschreibt das Wächterbyte ⇒ FEHL.
- [ ] **AKU.2 (NTP-Plausibilität, tragend)** — LI = 3, Stratum 0 oder Transmit 0 ⇒ **kein**
      `TIME` an den STM, Logzeile **`- ntp verworfen`** mit Grund. **Keine** Ära-Rechnung im ESP.
      *Instrument:* Prüfstand ntp. **Gegenprobe:** alter Stand sendet `TIME 0` ⇒ FEHL.
- [ ] **AKO.1 (`ovr=`)** — `Serial.hasOverrun()` sättigend gezählt, als `ovr=` an der Heap-Zeile.
      *Instrument:* Attrappe zählt und sättigt; Review E.
- [ ] **AKK.2 (A67 ESP)** — Alle drei OTA-Pfade senden die Zeile aus AKK.1 vor dem Download und
      bei Fehlschlag `SYNCVARS`. *Instrument:* statische Prüfung (Lead). Gerätenachweis erst beim
      nächsten ESP-OTA nach E.
- [ ] **AKQ.2 / AKU.3 / AKO.2 (Gerät, G4)** — ESP-OTA mit laufender Wache; `ovr=0`; 60 min ohne
      `var ignoriert` und `ntp verworfen`; Netzzeit übernommen; Smoketest samt Update-Quelle,
      `install-app.sh --check`; Δ`v` beim OTA berichtet (Vorher +2).

### Schritt P — PWA

- [ ] **AKC.1 (C35, Ent-6 = A)** — Hauptmodul: `update_status` beim Öffnen, dann höchstens alle
      5 min; Update-/Wartungsmodul wie heute, beim Wechsel sofort; Fehlschlag zählt als Abruf.
      *Instrument:* Prüfstand **c35** (Hauptmodul 30 min ⇒ ≤ 7). **Gegenprobe:** alter Stand ≈ 120 ⇒
      FEHL.
- [ ] **AKC.2 (Gerät, G5)** — `check-pwa.sh` ohne Fehler; Hauptmodul 15 min ⇒ höchstens 4 Abrufe im
      Mitschnitt.

### Schritt M — Abschlusslauf A21 (G6)

- [ ] **AKM.1 (Lauf)** — 30 min, Tab Hauptmodul, Tab System, **Tab Updatemodul**, Wetterabrufe,
      Setter-Burst 14 × Wetterort mit Rückstellung (Ent-14 c), Logwache parallel.
- [ ] **AKM.2 (Kriterium)** — `d=0`, Δ`a=0`, Δ`v=0` (beide Zahlen), `rx` ≤ 512/1024, 0 `keine
      Quittung`, 0 `cmd abgewiesen`, `ovr=0`, `diag` lückenlos ⇒ **A21 schliessen**; steigt `ovr`:
      neuer Befund, A21 neu fassen.

### Flash-Gate (S1, S2, S3)

- [ ] **AKZ.F** — Testbau F103 WC12h SK6812-RGBW nach **jedem** STM-Task (S1.1, S1.3, S1.5, S1.9,
      S1.11, S2.1, S3.4, S3.6, S3.8, S3.10) und nach **jeder** Einsparung aus S3.2: Rest
      **≥ 1'024 Byte**, gemessener Zuwachs je Task im Bericht. Darunter **anhalten und vorlegen**
      (`design.md` §1.3); **keine** Sparstufe ohne neue Rückfrage (Ent-17). **Kein Rollout unter
      1'024 Byte.** **Gemessen bisher:** S1.1 → 1'700, S1.3 → 1'132, S1.5 → 1'088, S1.9 (N1) →
      1'756, S1.11 (M1+M2) → 1'608. **Planung ab der engsten Matrix-Variante** (1'416, 192 Byte unter
      dem Teilbuild; `design.md` §1.3) — ein Teilbuild über 1'024 garantiert nicht, dass alle 72
      halten.

### Matrix-Lauf aller 72 STM-Varianten (Ent-18)

- [ ] **AKZ.M.1 (Make-Ziel)** — Ein **eigenes** Make-Ziel des `release-engineer` baut alle **72**
      Varianten wie im Original-ZIP: {wc12h, wc24h} × {F103, F401CC-25, F401CC-8, F411CE-25,
      F411CE-8, Nucleo401, Nucleo411} × fünf LED-Typen, dazu je F407-TFT. **Nur auf Anforderung;
      der normale Build (`make`, `make release-zip`) bleibt unverändert** — das zeigt ein Vergleich
      der Ziele vor und nach der Änderung. Die BlackPill-F411-Varianten verwenden
      `startup_stm32f411xe.S` wie der Teilbuild; die Abweichung vom Original
      (`startup_stm32f401xc.S`) ist im Makefile-Kommentar und in `README-CMAKE.md` benannt.
      **Stand 11.10.2026:** Make-Teil erfüllt — `make matrix`, `make clean-matrix`; geändert
      `CMakeLists.txt` (`EBP_TARGET`, `LED`, `STARTUP`, `OUTPUT_DIR`, Option `WORDCLOCK_MATRIX`),
      `Makefile`, neu `cmake/matrix-report.sh`; **normaler Build byte-identisch**, die
      Matrix-Varianten `wc12h-f103`- und `f411ce-25`-RGBW byte-identisch zum Teilbuild.
      `README-CMAKE.md` (Z.5) in Arbeit.
- [x] **AKZ.M.2 (Gate und Meldung)** — Das Ziel gibt je Variante den freien Flash aus (Tabelle) und
      endet mit **Exit ≠ 0**, wenn eine Variante nicht baut **oder** unter **1'024 Byte** frei
      liegt; die Meldung nennt die Variante beim Namen. *Gegenprobe (DIR-014):* mit einer künstlich
      höheren Grenze (über dem kleinsten gemessenen Rest) schlägt das Ziel an, und die Meldung
      kommt an (Exit ≠ 0, Name der Variante). **Zähl nach:** 72 HEX-Dateien und 72 Tabellenzeilen
      gegen die Liste. **Erfüllt 11.10.2026:** Schwelle 2000 ⇒ 3 Varianten GATE, Exit ≠ 0; eine
      Variante mit `-O0` baut nicht ⇒ Exit ≠ 0.
- [x] **AKZ.M.3 (Lauf vor S1)** — Vor dem Release-Build S1 (S1.19) ein Matrix-Lauf: **alle 72
      gebaut, alle ≥ 1'024 Byte frei**; Tabelle im Bericht, kleinster Rest mit Variante benannt. Rot
      ⇒ **kein** Release-Build, anhalten und vorlegen. Spätere Matrix-Läufe nur auf Anforderung; das
      Gate „alle 72" gilt für jeden. **Erfüllt 11.10.2026:** 72/72, 0 unter dem Gate; engste
      Variante **`wc24h-stm32f103-sk6812-rgbw` mit 1'416 Byte frei**; Grösse gegen das Original
      +1,6 bis +7,4 %.

### Für alle Schritte

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `guardrails.sh --full` vor jedem Release mit **unverändertem Warnungsbestand**
      (vor S1: die 7 Warnungen `unused parameter` aus N1 beseitigt, S1.16a); `auszug.sh` meldet
      a62, a69, a23, a53, a57, a67, a60, a35, ntp, c35 und die bestehenden (gt, EP, W2, L59) OK.
- [ ] **AKZ.3 (Release-Inhalt)** — Vor **jedem** Release-Build Produktänderungen **nur** in der
      angehobenen Komponente und — bei S1, S2, S3 — **nur** aus den Tasks dieses Releases (für S1
      einschliesslich N1, M1, M2). Ausgabe im Bericht.
- [ ] **AKZ.4** — Smoketest nach jedem Einspielen; nach dem ESP-Update samt Update-Quelle (DIR-009)
      und `install-app.sh --check` (DIR-017). **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.5 (Logwache)** — läuft bei **jedem** Gerätelauf, vor dem Einspielen gestartet,
      nach Änderungen neu gestartet (DIR-013, L362). Erweitert um `time: ungueltig` (Alarm),
      `time: sprung` (Hinweis; Alarm bei mehr als drei in Folge), `night: start aus` (S1),
      `spiel: … verworfen`, `ldr: … abgewiesen` (Hinweise, S3), `var ignoriert`, `ntp verworfen`,
      `ovr=` > 0 (Alarm, E) — **je mit eingespielter Gegenprobe**. Freigegebene Resets getrennt
      berichtet.
- [ ] **AKZ.6 (T)** — `pwa-tester` Phasen 0–4 und 9 nach M, mit dem nachgeführten Testplan.
      Unvollständig ⇒ **nicht abgenommen**.
- [ ] **AKZ.7** — Jedes Release **sofort** committet, getaggt, **gepusht** (DIR-011); den
      `CHANGELOG.md`-Eintrag schreibt der Lead.
- [ ] **AKZ.8** — Einspielzeilen mit Reihenfolge, fertig zum Einfügen; eingespielt vom Lead
      (ESP: `curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"`).
- [ ] **AKZ.9** — Kodierung **und** Zeilenende jeder Datei vor dem Patch festgestellt (DIR-015),
      u. a. `main.c`, `night.c`, `timeserver.c`, `stubs.c`, `display.c`, `tetris.c`, `snake.c`,
      `ldr.c`, `vars.c`, `esp8266.c`, `i2c.c`, `ESP-uclock.ino` (**gemischt**), `http.cpp` (UTF-8),
      `vars.cpp`, `ntp.cpp`, `app.js`.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **Die sechs niedrigen Restpunkte aus Review S1** (Gleichstand Aus/Ein in derselben Minute; Rücksprung über 3,5 Tage modulo Woche; `quick` nach drei Fehlwerten, bis rund 2 h bis zur ersten Netzzeit; bei `NIGHT_OFF_AFTER_IP` keine Frist, falls der Restore nie kommt; erstes `TIME` bei ungültiger RTC schaltet sofort und kann den IP-Lauftext abschneiden; ein anderer Ticker überschreibt den IP-Lauftext) | Benannt, kein Fix (`design.md` §3.4); in Z.1 als Befunde eingetragen |
| **RTC über 2099** (Jahrhundertbit des DS3231, `rtc.c:204`, `:241`) | Die neue Grenze der Kette; in L371 benannt, eigenes Vorhaben |
| **Ära-Rechnung im ESP** | Der ESP rechnet nicht mit der Zeit (`design.md` §2.1) |
| **Protokollwechsel an `TIME`** (feste 10 Stellen, Prüfsumme) | Ent-3 ohne Protokollwechsel; Ent-3b: Bestätigungsregel statt Protokollwechsel |
| **Die Sparstufen aus `design.md` §1.4** | Ent-17: nicht gewählt; nur nach neuer Rückfrage |
| **Einsparungen mit Verhaltensänderung** in S3; **`set_display_power: new=…`** | Nur nach Rückfrage (AKP.2); die Zeile ist geschützt |
| **Änderung am normalen Build** durch das Matrix-Ziel; **Matrix-Lauf vor jedem Release** | Ent-18: eigenes Ziel, nur auf Anforderung; Pflicht nur vor S1 |
| **Angleichen des F411-Startups an das Original** | Ent-18: `startup_stm32f411xe.S` bleibt, die Abweichung wird dokumentiert |
| **Sichtbare Meldung der A57-Abweisung in der PWA** | Geht nur mit einem neuen, wartenden Endpunkt (`http.cpp:10077-10086` antwortet vor der Messung); als Hinweis in Z.1 als neuer Befund |
| **Abweisung im Zahlen-Setter (A59)** | Bräche den Backup-Import (L68) |
| **Abschlussverfahren aus RM0008**, **ACK-Polling statt 16-ms-Wartezyklus** | Ent-15; Folgearbeit |
| **A34 Task 10** | Nur bei über 5 s Stillstand als neuer Befund |
| **A53 V1/V3**, **`GAME_IDLE_TIMEOUT_SEC`** | Ent-4 |
| **A35 (c)** NAK; **Übernahme-Bestätigung ESP→STM** | Ent-5; eigener Punkt |
| **C35 (B)/(C)**, **Cache der Release Notes** | Ent-6; ESP-RAM |
| **A63-Korrektur** | Ent-10 |
| **Ambilight-Liste beim Start** | Ent-2 (1); bleibt mit A56 |
| **`setRxBufferSize`** | Erst messen (`ovr=`) |
| **A56, A66, A64, A65, A19, A15, A26** | Eigene Befunde. **A52** nur, wenn S3.1 ihn als Einsparung vorschlägt |
| **Fehlerfälle von A62, A60, NTP am Gerät** | Nicht herstellbar; Prüfstände |
| **PWA-Befunde B48–B53**, **frühere Specs**, **Testplan-Phasen 5–8** | Nicht beauftragt bzw. Momentaufnahmen bzw. beim Nutzer |

---

## Betroffene Laufzeiten

- [ ] **STM32** (`src/**`) — S1: `main.c`, `night/night.c` (+ `.h`), `timeserver/timeserver.c`,
      `vars/vars.c` (+ `.h`), `stubs/stubs.c` (N1); S2: `i2c/i2c.c`; S3: Dateien der Einsparungen
      (aus S3.1), `display/display.c`, `tetris/tetris.c`, `tetris/snake.c`, `ldr/ldr.c`,
      `esp8266/esp8266.c`, `main.c`; je ein Flash
- [ ] **ESP8266** — E: `vars.cpp`, `ntp.cpp`, `ESP-uclock.ino`, `http.cpp`; ein OTA
- [ ] **PWA** — P: `app.js`
- [ ] **Werkzeug** (`tools/**`) — Auswerteregel, Prüfstände, `watch-log.sh`, statische Prüfung
- [ ] **Dokumentation** — `BEFUNDE.md`, `TESTPLAN-PWA.md`, `knowledge/architecture-checklist.md`,
      `README-CMAKE.md` (`doc-writer`); `CLAUDE.md`, `CHANGELOG.md` (Lead)
- [ ] **Build/Release** — Matrix-Ziel (`CMakeLists.txt`, `Makefile`, `cmake/matrix-report.sh`,
      `release-engineer`); **fünf** Releases (S1, S2, S3, E, P); je Commit, Tag, Push

---

## Entscheidungen

**Entschieden vom Nutzer am 11.10.2026.** Empfehlung jeweils in Klammern, wo abweichend.

- **Ent-1 — A23:** **(a) RAM-Sicherung; DC während der Animation aktualisiert sie: ja; IR-Schritte
  im Modus 0: ja.**
- **Ent-2 — A69:** **(1) nur die Displayliste; (2) beim ersten übernommenen `TIME` einmal
  auswerten; (3) ein übersprungenes Ereignis nachholen; (4) Startsequenz wie heute, danach aus.**
  **Präzisiert nach Review S1 (M2): „IP erst zeigen, dann aus"** — vormerken, nach dem IP-Lauftext
  anwenden, ohne IP nach 60 s (AKW.6).
- **Ent-3 — A62: Ära-Rechnung jetzt** (Empfehlung war: Verfallsdatum hinnehmen). Ausgearbeitet in
  `design.md` §2: Pivot ohne Protokollwechsel, eine Grenze auf der Unixzeit, ESP-Plausibilität
  tragend.
- **Ent-3b — Bestätigungsregel bei Zeitsprung: entschieden wie vorgeschlagen.** Weicht eine
  gültige Zeit um mehr als 600 s von der fortgeschriebenen letzten ab — und **immer nach dem
  Start** —, gilt sie erst, wenn eine zweite Antwort nach 15 s sie auf ±5 s bestätigt; höchstens 3
  Versuche in Folge (`design.md` §2.4). Verworfen: (i) keine Bestätigung, nur ESP-Plausibilität;
  (ii) Protokollwechsel mit festen 10 Stellen. **Umsetzung im Rahmen (Lead, 11.10.2026):** der
  zusätzliche Mindestabstand von 10 s macht die Regel strenger; das unbedingte
  `cmd: set time to` dient der Beobachtbarkeit — keine neue Rückfrage nötig.
- **Ent-4 — A53:** **V2, keine Inaktivitätsgrenze.**
- **Ent-5 — A35:** **(b) nur ESP, zählen und melden.**
- **Ent-6 — C35:** **(A) nur PWA; Hauptmodul beim Öffnen und alle 5 min; kein 15-s-Takt im
  Hauptmodul.**
- **Ent-7 — A57: (C) „Messwert übernehmen" weist eine Inversion ab** (Empfehlung war: schliessen).
  Nur `ldr_set_min_value()`/`ldr_set_max_value()`, **nicht** der Zahlen-Setter. Rückmeldung über
  die bestehende `var_send_ldr_*`; PWA-Meldung nur als Hinweis (keine neue API). A57 wird erst nach
  dem Gerätenachweis (G3) geschlossen.
- **Ent-8 — A61:** **Auswerteregel im Werkzeug.**
- **Ent-9 — A34:** **schliessen, Reste überführen.**
- **Ent-10 — A63:** **beobachten.**
- **Ent-11 — A68:** **ja.**
- **Ent-12 — A67:** **ja, STM- und ESP-Hälfte.**
- **Ent-13 — NTP:** **Puffer samt Plausibilität** — nach Ent-3 **tragend**.
- **Ent-14 — Freigaben:** **alle erteilt** — (a) STM-Reset im Nacht-Aus-Fenster, und zwar **der
  eine Reset nach den Tagtests von S1 (G1)**; (b) Setter während eines Spiels; (c) Setter-Burst im
  A21-Lauf; (d) Farbanimation, LDR S25/S26 und 32-Byte-Wert vor/nach S2, `pwa-tester`-Durchlauf mit
  S-Fällen.
- **Ent-14e — LDR-Abweisungsprobe AKE.2: ausdrücklich freigegeben** (eigene Freigabe des Nutzers,
  nicht aus Ent-14 (d) abgeleitet): Maximum per Zahlen-Setter kurz auf 1 bzw. Minimum auf 4094,
  danach zurück auf 0/4095.
- **Ent-15 — A60:** **nur die Fristen.**
- **Ent-16 — Spec: freigegeben.**
- **Ent-17 — Flash-Gate nach S1.3 (gemessen 1'132 Byte frei): „Release teilen".** STM-Release 1
  = A62, A69, A68; danach Version, Build, Rollout, G1 wie gehabt. A23, A53, A57 (C) und A67-STM in
  ein eigenes STM-Release 3, davor eine **Flash-Analyse** (Map lesen, Einsparungen mit gemessenen
  Grössen), deren verhaltensneutrale Einsparungen einzeln gemessen umgesetzt werden; erst dann die
  vier Punkte, einer nach dem anderen mit Gate. **Alle übrigen Entscheidungen bleiben erhalten**,
  auch Ent-2 (3), A53 V2 und A67. Die Sparstufen der Spec sind **nicht** gewählt und greifen nur
  nach einer neuen Rückfrage. Reihenfolge S1 → S2 → S3 → E (Begründung `design.md` §0.2).
  **Nachtrag:** N1 (malloc-Stubs) hat der Lead aus S3.1 in S1 vorgezogen, damit M2 in den Flash
  passt; verhaltensneutral, beide Reviews BESTANDEN.
- **Ent-18 — Komplettbuild aller 72 STM-Varianten.** Eigenes Make-Ziel, nur auf Anforderung,
  normaler Build unverändert; **erster Lauf vor STM-Release 1**; **das Flash-Gate (1'024 Byte)
  gilt für alle 72 Varianten**, für jeden Matrix-Lauf; die BlackPill-F411-Varianten verwenden
  `startup_stm32f411xe.S` wie der Teilbuild (das Original nimmt `startup_stm32f401xc.S`; die
  Abweichung wird dokumentiert). Die Teilbuild-Messung je Task bleibt auf F103 WC12h SK6812-RGBW.
  **Erster Lauf 11.10.2026: 72/72 grün, engste Variante `wc24h-stm32f103-sk6812-rgbw` mit
  1'416 Byte frei; geplant wird ab diesem Wert** (`design.md` §1.3).
- **Offen — zweiter STM-Reset im Aus-Fenster in G3** (für AKD.2 Teil 2, die EEPROM-Frage aus A23):
  **vor G3 beim Nutzer zu bestätigen**; nicht durch Ent-14 (a) gedeckt. Der Lead fragt vor G3
  nach.
