# Anforderungen — Paket 2026-10-11 „Nachtfenster, Netzzeit, Spiele, I²C, Brücke" (A23, A69, A53, A57, A60, A61, A62, A63, A35, A21, A34, C35)

**Status: freigegeben** — vom Nutzer am 11.10.2026 (Ent-16), samt Ent-1 bis Ent-15 (unten,
„Entscheidungen"). **Zwei Entscheidungen gegen die Empfehlung:** Ent-3 (Ära-Rechnung jetzt) und
Ent-7 (C). **Ent-3b** (Bestätigungsregel, aus Ent-3) und **Ent-14e** (eigene Freigabe der
LDR-Abweisungsprobe AKE.2) am selben Tag entschieden.
**Erstellt:** 2026-10-11, Stand `5c34d80` (Zweig `pwa-decoupling`); am selben Tag mit den
Entscheidungen nachgeführt. Ausgangsstand am Gerät laut `specs/paket-2026-10-10/` (Endstand):
STM 3.2.28, ESP 3.2.31, PWA 1.4.95.
**Auslöser:** Auftrag des Nutzers vom 11.10.2026 — die zwölf Einträge A23, A69, A53, A57, A60,
A61, A62, A63, A35, A21, A34 und C35 aus der ToDo-Liste in `BEFUNDE.md`.
**Grundlage:** zwei Analysen des `firmware-analyst` vom 11.10.2026 (STM und Brücke), vom Lead
zusammengefasst; der `spec-writer` hat die tragenden Fundstellen stichprobenweise am Code
nachgelesen und für Ent-3 und Ent-7 eigens nachgeprüft.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** Zeilennummern gelten für `5c34d80`, sie sind Fundstellen, keine Anker.
Kennzeichen: **✔ SW** — vom `spec-writer` am Code nachgelesen; **✔ A** — in der Analyse als
[BELEGT] gemeldet, nicht einzeln nachgelesen; **◐** — [PLAUSIBEL], nicht belegt; **●** —
gemeldet bzw. [SPEKULATION], vor der Umsetzung zu prüfen.

**Kennungen.** Schritte: **0** (ohne Firmware), **S1** (STM-Release 1: A62 mit Ära, A69, A68,
A23, A53, A57, A67-STM), **S2** (STM-Release 2, isoliert: A60 mit L360), **E** (ESP-Release: A35,
NTP, `ovr=`, A67-ESP), **P** (PWA-Release: C35), **M** (Abschlusslauf A21), **T** (Testplan und
`pwa-tester`), **Z** (Doku und Spec-Endstand). Gerätenachweise **G1 bis G5**. Entscheidungen
**Ent-1 bis Ent-16**, dazu **Ent-3b** und **Ent-14e**. Akzeptanzkriterien: **AKN** (Netzzeit,
A62), **AKW** (Nachtfenster, A69), **AKB** (Ambilight-Kopie, A68), **AKD** (Displayfarbe, A23),
**AKS** (Spiele, A53), **AKE** (Einmessen, A57), **AKK** (OTA-Ankündigung, A67), **AKI** (I²C,
A60), **AKQ** (Übernahme, A35), **AKU** (NTP-Antwort), **AKO** (ESP-Empfangsüberlauf), **AKC**
(C35), **AKA** (Abgleich-Auswertung, A61/A34), **AKM** (Abschlusslauf, A21), **AKZ** (alle). Die
Kürzel gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Gerätenachweis |
|---|---|---|---|---|
| **0** | A61-Auswerteregel als Werkzeug; Abgleichsdauer aus vorhandenen Mitschnitten (A61, A34); A34 schliessen, A63 als Beobachtung; **L391** NTP-Puffer, Berichtigungen an L331, L360, L371, L233; Testplan V3/S1 um den Nachtzustand | Werkzeug, Doku | — | — (lesend, Mitschnitte) |
| **S1** | **A62** Netzzeit mit **Ära-Rechnung** und Bestätigungsregel (spart Flash, zuerst) · **A69** Nachtfenster · **A68** Ambilight-Kopie · **A23** Displayfarbe · **A53** Spiele · **A57 (C)** Einmessen weist Inversion ab · **A67-STM** | STM | **zuerst** | **G1** — Tagtests, dann STM-Reset im Aus-Fenster, Vorher-Messung für S2 |
| **S2** | **A60 + L360** I²C-Fristen — **eigenes Release, eigene Messung** | STM | nach G1 | **G2** |
| **E** | **A35** (b) · **NTP-Puffer und Plausibilität** (nach Ent-3 tragend) · **`ovr=`** · **A67-ESP** | ESP | nach G2 | **G3** |
| **P** | **C35** (A) | PWA | nach G3 | **G4** |
| **M** | **Abschlusslauf A21** | — | nach G4 | **G5** |
| **T** | `TESTPLAN-PWA.md` nachführen; `pwa-tester` Phasen 0–4 und 9 (DIR-012) | — | nach G5 | — |
| **Z** | `BEFUNDE.md` (A57-Schliessung nach G1), `knowledge/architecture-checklist.md`, `CLAUDE.md`, Spec-Endstand | Doku | — | — |

**Warum diese Reihenfolge:** `design.md` §0. Kurz: `make release-zip` baut STM, ESP und die
PWA-Assets **immer zusammen**; darum entsteht der Code des nächsten Releases erst **nach dem
Release-Build** des vorigen, und jeder Release-Build erst **nach dem Gerätenachweis** des vorigen.
**Firmware vor PWA** (DIR-002). `CHANGELOG.md` je Release schreibt der Lead.

---

## Problem

### 0. Flash F103 — 884 Byte Spielraum

**✔ (Endstand Paket 2026-10-10):** **1'908 Byte frei**, Gate **≥ 1'024**, also **884 Byte**
Spielraum für S1 und S2. Im Vorpaket kostete S1 **rund 800 statt der geschätzten 75–125 Byte**.
**Belegt ist nur eine Zahl:** `strtoul` steht im F103-Fabrikat ausschliesslich für die Netzzeit und
kostet **240 Byte** (Map, ✔ A). Rechnung in `design.md` §1.

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

### 3. A68 / L389 — ESP-Kopie der Ambilight-Power veraltet

**✔ SW:** `set_display_power()` (`main.c:1425-1456`) schaltet bei `do_sync_ambilight` das
Ambilight mit, sendet aber nur `var_send_display_power()`. Die Sendefunktion der Ambilight-Power
ist `static` (`vars.c:1324`, laut BEFUNDE). A69 erzeugt denselben Fehler beim Start.

### 4. A23 / L150 — Farbanimation überschreibt die Displayfarbe

**✔ A:** Regenbogen und Tageslicht schreiben `display.display_colors` direkt (`display.c:3157-3173`,
`:5348-5418`, `:5458-5468`); beim Wechsel auf Modus 0 (`:4456-4473`, `:4480-4527`) bleibt die
Momentanfarbe stehen; die Sicherung wie beim Ambilight (`:3557`, `:6172`) fehlt. Kein
EEPROM-Schreiben im Animationspfad, aber die **ESP-Kopie** wird laufend überschrieben
(`:1211-1220`). ◐ Nach einem STM-Reset müsste die Nutzerfarbe zurückkommen — G1 klärt es.
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

---

## Ziel

1. **Flash:** S1 und S2 halten das Gate von 1'024 Byte nach **jedem** STM-Task.
2. **Netzzeit:** Die Uhr nimmt gültige Netzzeiten **bis 2106** an (Kette bis 31.12.2099 durch die
   RTC); eine ungültige, eine 0 oder eine verstümmelte stellt sie **nie** — verworfen und gemeldet.
3. **Nachtfenster:** Nach einem Neustart im Aus-Fenster ist das Display **aus**; übersprungene
   Timer werden nachgeholt; die ESP-Kopie stimmt.
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
      dem Vermerk „Ent-7 (C), Schliessung nach G1". *Instrument:* S10 und S11 grün.
- [ ] **AKA.4 (Testplan V3/S1)** — V3 und S1 nennen das **Soll im Nachtzustand** (Display aus nach
      Reset im Aus-Fenster, ab STM-Release 1) und die Zeile `time: sprung, bestaetigung
      angefordert` nach jedem Reset als Soll. **Vor G1 erledigt.**

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
      (`timeserver.c:253`); keine neue 64-Bit-Arithmetik. *Instrument:* Review S1.16; AKN.4.
- [ ] **AKN.4 (Map)** — In der Map des F103-Testbaus steht kein `strtoul` mehr und weiter **kein**
      `__udivmoddi4`/`__aeabi_ldivmod`; Guardrail **S8b** weist es nach. **Gegenprobe:** gegen die
      Map von 3.2.28 schlägt die `strtoul`-Prüfung an.
- [ ] **AKN.5 (Bestätigungsregel, Ent-3b)** — Eine gültige Zeit wird sofort übernommen, wenn
      sie um höchstens 600 s von der fortgeschriebenen letzten übernommenen Zeit abweicht; sonst
      erst, wenn eine zweite Antwort sie auf ±5 s (fortgeschrieben über `uptime`) bestätigt. Bis
      dahin Logzeile **`time: sprung, bestaetigung angefordert`**, Schnellabfrage nach 15 s,
      höchstens drei in Folge. Nach einem Start gibt es keine Referenz: Die erste Antwort braucht
      immer die Bestätigung. *Instrument:* a62 mit simulierter `uptime`: Start + n + (n+15) ⇒
      übernommen bei der zweiten; Start + 0 + 0 ⇒ nie; stündlich ±30 s ⇒ sofort; ein Fehlwert ⇒
      verworfen, danach regulär; vier Fehlwerte ⇒ drei Schnellabfragen; Abgleich über die
      Ärengrenze ⇒ sofort. **Gegenprobe:** Sabotage „Bestätigung entfernt" ⇒ 0 nach dem Start
      angenommen ⇒ FEHL.
- [ ] **AKN.6 (Gerät, G1, lesend)** — Nach dem S1-Flash: `time: sprung, bestaetigung angefordert`,
      rund 15 s später `cmd: set time to …`; die nächste stündliche Netzzeit **ohne** Bestätigung;
      `read rtc` gleich der Wanduhr ±2 s; keine Zeile `time: ungueltig`.
- [ ] **AKN.7 (Gerät, G1, Reset im Fenster)** — Beim Reset aus AKW.4 dasselbe Bild wie AKN.6.

### Schritt S1 — A69 (Nachtfenster) und A68 (Ambilight-Kopie)

- [ ] **AKW.1 (geltendes Ereignis)** — Eine Funktion in `night.c` bestimmt das **jüngste
      vergangene**, aktive, für den Wochentag gültige Ereignis über 8 Timer × k = 0…7 Tage; die
      Wochentagsprüfung aus `night.c:166-180` wird **eine** Hilfsfunktion. *Instrument:* Prüfstand
      **a69** gegen eine Minuten-Referenz mit `night_check_night_times()`, zufällige Tabellen,
      **alle 10'080 Startpunkte** je Tabelle, Sonderfälle. **Gegenprobe:** Sabotagen `<=` → `<` und
      Wochentagsumlauf ⇒ FEHL. **Zähl nach:** Tabellen × 10'080.
- [ ] **AKW.2 (beim Start, Ent-2 (1), (2), (4))** — Nach dem RTC-Lesen und vor `watchdog_init()`
      wird das Ergebnis **für die Displayliste** angewandt; „aus" ⇒ `set_display_power (FALSE,
      TRUE)` wie beim Timer, Logzeile **`night: start aus (hh:mm)`**. War die RTC beim Start nicht
      gültig, einmal beim **ersten übernommenen** `TIME`. Die Startsequenz läuft wie heute davor.
      *Instrument:* a69 (Startpfad mit Attrappen), Review S1.16.
- [ ] **AKW.3 (Zeitsprünge, Ent-2 (3))** — Springt die Uhrzeit **vorwärts** um mehr als eine
      Minute, wird das jüngste Ereignis **im übersprungenen Intervall** angewandt, und nur dieses;
      rückwärts nichts; ohne übersprungenes Ereignis bleibt ein manueller Zustand. *Instrument:*
      a69: Sprung über Aus-Timer ⇒ aus; ohne Timer ⇒ unverändert; rückwärts ⇒ unverändert;
      Sommerzeit 02:00 → 03:00 mit Timer 02:30. **Gegenprobe:** alter Stand verliert den Timer ⇒
      FEHL.
- [ ] **AKB.1 (A68)** — Jede Stelle, die die Ambilight-Power ändert, sendet sie bei
      `esp8266.is_online`; Sendefunktion öffentlich, **keine zweite**; **Bestandsliste**
      (Datei:Zeile). *Instrument:* a69 (Sendeprotokoll). **Gegenprobe:** alter Stand ⇒ FEHL.
- [ ] **AKW.4 (Gerät, G1 — Reset im Aus-Fenster, Ent-14 a)** — Ein eigener STM-Reset nach dem
      Aus-Timer des Nutzers, nach den Tagtests. Im Mitschnitt: `night: start aus (hh:mm)`,
      `set_display_power: new=0 …`, in der ersten Minute `show_time: … p=0 …`; nach dem
      Vollabgleich Display-Power = 0 und Ambilight-Power gleich dem STM (`settings_xml`). Am
      Display: aus. **Vorher-Wert:** L390.
- [ ] **AKW.5 (Gerät, lesend)** — Am Morgen schaltet der Ein-Timer wie bisher.

### Schritt S1 — A23 (Displayfarbe)

- [ ] **AKD.1 (Sicherung, Ent-1)** — RAM-Sicherung nach dem Muster `saved_ambilight_colors`;
      gesetzt beim Laden **vor** `display_init_color_animation()`, in DC **auch während** einer
      Animation und in den IR-Farbschritten im Modus 0; zurück auf **jedem** Weg auf Modus 0 samt
      `display_calc_dimmed_display_colors()` und **`var_send_display_colors()`**. Kein
      EEPROM-Schreiben im Animationspfad. *Instrument:* Prüfstand **a23** (Regenbogen, Tageslicht,
      Inkrement/Dekrement, DC während Animation, IR-Schritte, Sendeprotokoll). **Gegenprobe:** alter
      Stand ⇒ FEHL.
- [ ] **AKD.2 (Gerät, G1, Ent-14 d)** — **Teil 1, am Tag:** `dspcolor[0]` lesen; Regenbogen ein,
      rund 30 s, aus ⇒ gleich dem Vorher-Wert, Farbsendung im Mitschnitt; dasselbe mit Tageslicht.
      **Teil 2, nach dem Reset aus AKW.4:** `dspcolor[0]` gleich der Nutzerfarbe. Rückstellung:
      Animationsmodus wie vorher.

### Schritt S1 — A53 (Spiele)

- [ ] **AKS.1 (Ent-4 = V2)** — Beide Spielschleifen zählen verworfene Zeilen (nicht Zeitablauf,
      nicht `GT`/`GS`); danach bei Zähler > 0 `var_sync_pending = 1` und **`spiel: %u Zeilen
      verworfen`**. Keine Inaktivitätsgrenze. *Instrument:* Prüfstand **a53**. **Gegenprobe:** alter
      Stand ⇒ FEHL.
- [ ] **AKS.2 (Snake-Status)** — `snake.c:291` definiert vor dem ersten Lesen. *Instrument:* Review.
- [ ] **AKS.3 (Gerät, G1, Ent-14 b)** — Spiel starten, Helligkeit setzen, Spiel beenden:
      `spiel: n Zeilen verworfen` (n ≥ 1), `info: configuration sent`, `diag` lückenlos, ESP-Kopie
      der Helligkeit auf dem STM-Wert. Rückstellung: Helligkeit wie vorher.

### Schritt S1 — A57 (Einmessen, Ent-7 = C)

- [ ] **AKE.1 (Abweisung)** — `ldr_set_min_value()` übernimmt einen Messwert **≥ Maximum** nicht,
      `ldr_set_max_value()` einen Messwert **≤ Minimum** nicht (Gleichheit abgewiesen, wie
      `ldr.c:128`); Wert und EEPROM bleiben; Logzeile **`ldr: minimum abgewiesen`** bzw.
      **`ldr: maximum abgewiesen`** (mit Messwert und Gegengrenze). Die bestehende Rückmeldung
      (`main.c:1609`, `:1617`) meldet den **alten** Wert an den ESP. Der **Zahlen-Setter (A59,
      `main.c:1907-1914`) bleibt unverändert** und nimmt die Inversion weiter an. *Instrument:*
      Prüfstand **a57** mit ADC- und EEPROM-Attrappe: unter, gleich, über der Gegengrenze, je
      Minimum und Maximum; Schreibzyklen gezählt. **Gegenprobe:** gegen 3.2.28 ⇒ Inversion
      angenommen ⇒ FEHL. **L59 besteht weiter.**
- [ ] **AKE.2 (Gerät, G1, schreibend mit Rückstellung, Ent-14e — ausdrücklich freigegeben)** — Nur
      bei ausgeschalteter Auto-Helligkeit (`numvar[8]=0`) und einem LDR-Rohwert zwischen 2 und 4093:
      Maximum per Zahlen-Setter kurz auf 1, „Minimum übernehmen" ⇒ `ldr: minimum abgewiesen`,
      `numvar[17]` unverändert, **keine** `eep a=225`; Maximum zurück auf 4095. Minimum per
      Zahlen-Setter kurz auf 4094, „Maximum übernehmen" ⇒ `ldr: maximum abgewiesen`; Minimum zurück
      auf 0. Danach 17/18 = 0/4095 (lesend). **Erst danach wird A57 geschlossen** (Z.1).

### Schritt S1 — A67 STM-Hälfte (Ent-12)

- [ ] **AKK.1 (eigene Zeile)** — Eine **neue** Zeile (Wortlaut in S1.13 festgelegt, Vorschlag
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
- [ ] **AKQ.2 / AKU.3 / AKO.2 (Gerät, G3)** — ESP-OTA mit laufender Wache; `ovr=0`; 60 min ohne
      `var ignoriert` und `ntp verworfen`; Netzzeit übernommen; Smoketest samt Update-Quelle,
      `install-app.sh --check`; Δ`v` beim OTA berichtet (Vorher +2).

### Schritt P — PWA

- [ ] **AKC.1 (C35, Ent-6 = A)** — Hauptmodul: `update_status` beim Öffnen, dann höchstens alle
      5 min; Update-/Wartungsmodul wie heute, beim Wechsel sofort; Fehlschlag zählt als Abruf.
      *Instrument:* Prüfstand **c35** (Hauptmodul 30 min ⇒ ≤ 7). **Gegenprobe:** alter Stand ≈ 120 ⇒
      FEHL.
- [ ] **AKC.2 (Gerät, G4)** — `check-pwa.sh` ohne Fehler; Hauptmodul 15 min ⇒ höchstens 4 Abrufe im
      Mitschnitt.

### Schritt M — Abschlusslauf A21

- [ ] **AKM.1 (Lauf)** — 30 min, Tab Hauptmodul, Tab System, **Tab Updatemodul**, Wetterabrufe,
      Setter-Burst 14 × Wetterort mit Rückstellung (Ent-14 c), Logwache parallel.
- [ ] **AKM.2 (Kriterium)** — `d=0`, Δ`a=0`, Δ`v=0` (beide Zahlen), `rx` ≤ 512/1024, 0 `keine
      Quittung`, 0 `cmd abgewiesen`, `ovr=0`, `diag` lückenlos ⇒ **A21 schliessen**; steigt `ovr`:
      neuer Befund, A21 neu fassen.

### Flash-Gate (S1, S2)

- [ ] **AKZ.F** — Testbau F103 nach **jedem** STM-Task (S1.1, S1.3, S1.5, S1.7, S1.9, S1.11, S1.13,
      S2.1): Rest **≥ 1'024 Byte**, Zahl neben der Schätzung (`design.md` §1). Darunter **anhalten
      und vorlegen** (§1.3). **Kein Rollout unter 1'024 Byte.**

### Für alle Schritte

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `guardrails.sh --full` vor jedem Release; `auszug.sh` meldet a62, a69, a23, a53,
      a57, a67, a60, a35, ntp, c35 und die bestehenden (gt, EP, W2, L59) OK.
- [ ] **AKZ.3 (Release-Inhalt)** — Vor **jedem** Release-Build Produktänderungen **nur** in der
      angehobenen Komponente. Ausgabe im Bericht.
- [ ] **AKZ.4** — Smoketest nach jedem Einspielen; nach dem ESP-Update samt Update-Quelle (DIR-009)
      und `install-app.sh --check` (DIR-017). **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.5 (Logwache)** — läuft bei **jedem** Gerätelauf, vor dem Einspielen gestartet,
      nach Änderungen neu gestartet (DIR-013, L362). Erweitert um `time: ungueltig` (Alarm),
      `time: sprung` (Hinweis; Alarm bei mehr als drei in Folge), `spiel: … verworfen`,
      `night: start aus`, `ldr: … abgewiesen` (Hinweise), `var ignoriert`, `ntp verworfen`,
      `ovr=` > 0 (Alarm) — **je mit eingespielter Gegenprobe**. Freigegebene Resets getrennt
      berichtet.
- [ ] **AKZ.6 (T)** — `pwa-tester` Phasen 0–4 und 9 nach M, mit dem nachgeführten Testplan.
      Unvollständig ⇒ **nicht abgenommen**.
- [ ] **AKZ.7** — Jedes Release **sofort** committet, getaggt, **gepusht** (DIR-011); den
      `CHANGELOG.md`-Eintrag schreibt der Lead.
- [ ] **AKZ.8** — Einspielzeilen mit Reihenfolge, fertig zum Einfügen; eingespielt vom Lead
      (ESP: `curl -s -m 30 "http://$DEVICE_HOST/api/remote_esp_update"`).
- [ ] **AKZ.9** — Kodierung **und** Zeilenende jeder Datei vor dem Patch festgestellt (DIR-015),
      u. a. `main.c`, `night.c`, `timeserver.c`, `display.c`, `tetris.c`, `snake.c`, `ldr.c`,
      `vars.c`, `esp8266.c`, `i2c.c`, `ESP-uclock.ino` (**gemischt**), `http.cpp` (UTF-8),
      `vars.cpp`, `ntp.cpp`, `app.js`.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **RTC über 2099** (Jahrhundertbit des DS3231, `rtc.c:204`, `:241`) | Die neue Grenze der Kette; in L371 benannt, eigenes Vorhaben |
| **Ära-Rechnung im ESP** | Der ESP rechnet nicht mit der Zeit (`design.md` §2.1) |
| **Protokollwechsel an `TIME`** (feste 10 Stellen, Prüfsumme) | Ent-3 ohne Protokollwechsel; Ent-3b: Bestätigungsregel statt Protokollwechsel |
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
| **A56, A66, A64, A65, A52, A19, A15, A26** | Eigene Befunde; A52 nur Sparstufe |
| **Fehlerfälle von A62, A60, NTP am Gerät** | Nicht herstellbar; Prüfstände |
| **PWA-Befunde B48–B53**, **frühere Specs**, **Testplan-Phasen 5–8** | Nicht beauftragt bzw. Momentaufnahmen bzw. beim Nutzer |

---

## Betroffene Laufzeiten

- [ ] **STM32** (`src/**`) — S1: `main.c`, `night/night.c` (+ `.h`), `timeserver/timeserver.c`,
      `display/display.c`, `tetris/tetris.c`, `tetris/snake.c`, `ldr/ldr.c`, `vars/vars.c`
      (+ `.h`), `esp8266/esp8266.c`; S2: `i2c/i2c.c`; je ein Flash
- [ ] **ESP8266** — E: `vars.cpp`, `ntp.cpp`, `ESP-uclock.ino`, `http.cpp`; ein OTA
- [ ] **PWA** — P: `app.js`
- [ ] **Werkzeug** (`tools/**`) — Auswerteregel, Prüfstände, `watch-log.sh`, statische Prüfung
- [ ] **Dokumentation** — `BEFUNDE.md`, `TESTPLAN-PWA.md`, `knowledge/architecture-checklist.md`
      (`doc-writer`); `CLAUDE.md`, `CHANGELOG.md` (Lead)
- [ ] **Build/Release** — vier Releases (S1, S2, E, P); je Commit, Tag, Push

---

## Entscheidungen

**Entschieden vom Nutzer am 11.10.2026.** Empfehlung jeweils in Klammern, wo abweichend.

- **Ent-1 — A23:** **(a) RAM-Sicherung; DC während der Animation aktualisiert sie: ja; IR-Schritte
  im Modus 0: ja.**
- **Ent-2 — A69:** **(1) nur die Displayliste; (2) beim ersten übernommenen `TIME` einmal
  auswerten; (3) ein übersprungenes Ereignis nachholen; (4) Startsequenz wie heute, danach aus.**
- **Ent-3 — A62: Ära-Rechnung jetzt** (Empfehlung war: Verfallsdatum hinnehmen). Ausgearbeitet in
  `design.md` §2: Pivot ohne Protokollwechsel, eine Grenze auf der Unixzeit, ESP-Plausibilität
  tragend.
- **Ent-3b — Bestätigungsregel bei Zeitsprung: entschieden wie vorgeschlagen.** Weicht eine
  gültige Zeit um mehr als 600 s von der fortgeschriebenen letzten ab — und **immer nach dem
  Start** —, gilt sie erst, wenn eine zweite Antwort nach 15 s sie auf ±5 s bestätigt; höchstens 3
  Versuche in Folge (`design.md` §2.4). Verworfen: (i) keine Bestätigung, nur ESP-Plausibilität;
  (ii) Protokollwechsel mit festen 10 Stellen. S1.1 ist damit nicht mehr blockiert.
- **Ent-4 — A53:** **V2, keine Inaktivitätsgrenze.**
- **Ent-5 — A35:** **(b) nur ESP, zählen und melden.**
- **Ent-6 — C35:** **(A) nur PWA; Hauptmodul beim Öffnen und alle 5 min; kein 15-s-Takt im
  Hauptmodul.**
- **Ent-7 — A57: (C) „Messwert übernehmen" weist eine Inversion ab** (Empfehlung war: schliessen).
  Nur `ldr_set_min_value()`/`ldr_set_max_value()`, **nicht** der Zahlen-Setter. Rückmeldung über
  die bestehende `var_send_ldr_*`; PWA-Meldung nur als Hinweis (keine neue API). A57 wird erst nach
  dem Gerätenachweis geschlossen.
- **Ent-8 — A61:** **Auswerteregel im Werkzeug.**
- **Ent-9 — A34:** **schliessen, Reste überführen.**
- **Ent-10 — A63:** **beobachten.**
- **Ent-11 — A68:** **ja.**
- **Ent-12 — A67:** **ja, STM- und ESP-Hälfte.**
- **Ent-13 — NTP:** **Puffer samt Plausibilität** — nach Ent-3 **tragend**.
- **Ent-14 — Freigaben:** **alle erteilt** — (a) STM-Reset im Nacht-Aus-Fenster; (b) Setter
  während eines Spiels; (c) Setter-Burst im A21-Lauf; (d) Farbanimation, LDR S25/S26 und
  32-Byte-Wert vor/nach S2, `pwa-tester`-Durchlauf mit S-Fällen.
- **Ent-14e — LDR-Abweisungsprobe AKE.2: ausdrücklich freigegeben** (eigene Freigabe des Nutzers,
  nicht aus Ent-14 (d) abgeleitet): Maximum per Zahlen-Setter kurz auf 1 bzw. Minimum auf 4094,
  danach zurück auf 0/4095.
- **Ent-15 — A60:** **nur die Fristen.**
- **Ent-16 — Spec: freigegeben.**
