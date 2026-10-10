# Anforderungen — Paket 2026-10-10 „Testbefunde und Altlasten" (A59, A2, A50, A55, C47, C55, C44, B44–B47)

**Status: freigegeben** — vom Nutzer am 10.10.2026, samt der Entscheidungen Ent-1 bis Ent-14, alle
nach Empfehlung (unten, „Entscheidungen").
**Erstellt:** 2026-10-10, Stand `675e3fd` (Zweig `pwa-decoupling`); am selben Tag mit den
Ergebnissen aus **Schritt V** (`firmware-analyst`) nachgeführt — neuer Sparschritt **S0**, neue
Varianten für A50 und A55, A2 präzisiert. Ausgangsstand am Gerät laut L370: STM 3.2.25,
ESP 3.2.30, PWA 1.4.94.
**Auslöser:** Auftrag des Nutzers vom 10.10.2026; Belege aus dem Testdurchlauf L370 (L364–L369)
und aus der ToDo-Liste in `BEFUNDE.md`.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** Zeilennummern gelten für `675e3fd`. Kennzeichen wie in `BEFUNDE.md`:
**✔ verifiziert** (am Code oder Gerät belegt), **● gemeldet** (noch zu prüfen). „✔ am Code" heisst:
vom `spec-writer` oder vom `firmware-analyst` (V) nachgelesen; „**neu**" kennzeichnet, was dabei
erstmals gefunden wurde.

**Kennungen.** Schritte: **V** (Vorbereitung, erledigt), **S0** (STM: Flash-Sparmassnahme
`my_gmtime()`), **S1** (STM: A59, A2), **S2** (STM: A50, A55), **E** (ESP: C47, C55, C44), **P**
(PWA: B44–B47, Regeln aus C44/C55, Anzeige aus A2), **T** (Testplan und `pwa-tester`), **Z**
(Doku). Gerätenachweise **G0 bis G4**, Entscheidungen **Ent-1 bis Ent-14**. Akzeptanzkriterien:
**AKG** (`my_gmtime`), **AKL** (LDR, A59), **AKT** (Temperatur, A2), **AKF** (Fehlerpfad, A50),
**AKP** (ESP-Neustart, A55), **AKX** (XSS, C47), **AKR** (Regeln, C55/C44), **AKB** (PWA,
B44–B47), **AKZ** (alle).

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Gerätenachweis |
|---|---|---|---|---|
| **V** | Flash-Kosten von A59/A2/A50/A55, Sparmöglichkeiten, Detailfragen — **erledigt 10.10.2026** (`design.md` §1) | — | — | — |
| **S0** | **Sparmassnahme:** `my_gmtime()` rechnet mit 32 statt 64 Bit — rund **950 Byte** Flash frei, Verhalten für 1970–2106 gleich (Ent-5) | STM | **zuerst** | **G0** (lesend: Uhrzeit, Wochentag) |
| **S1** | **A59** LDR-Zahlensetter beschreibbar, schreibt ins EEPROM; **A2** DS18xx: CRC für Scratchpad **und** ROM-ID, `is_up` nur bei gültiger ID, Neuerkennung, ungültig ⇒ 255 | STM | nach G0 | **G1** (LDR-Bereinigung schreibend, Temperatur lesend) |
| **S2** | **A50** `fault_reset()` setzt selbst zurück; **A55** ESP-Bootmeldung setzt den STM auf „offline" | STM | nach G1 | **G2** (lesend); A55 am Gerät erst in **G3** |
| **E** | **C47** Legacy-Seite maskiert Werte; **C55** Datumsformat mit Inhaltsregel; **C44** Koordinaten mit Inhaltsregel | ESP | nach G2 | **G3** — dabei A55 beim ESP-OTA |
| **P** | **B44–B47**; Regeln aus C55/C44 vor dem Senden; Anzeige „Messwert ungültig" (A2) | PWA | nach G3 | **G4** = `pwa-tester` (T) |
| **T** | `TESTPLAN-PWA.md` nachführen (S25/S26 rücknehmbar, neue Fälle); Durchlauf Phasen 0–4 und 9 | — | — | G4 |
| **Z** | `BEFUNDE.md`, `CLAUDE.md`, Nebenbefunde aus V | Doku | — | — |

**Warum diese Reihenfolge, und warum strikt nacheinander gearbeitet wird:** `design.md` §0.
Kurz: `make release-zip` baut STM, ESP und die PWA-Assets **immer zusammen**. Jeder
Release-Build darf deshalb nur Änderungen der Komponenten enthalten, deren Version er anhebt —
sonst landet ein Fabrikat mit alter Versionsnummer und neuem Inhalt auf dem Update-Server
(Lehre aus Paket 2026-10-09: Der STM-Teil von D kam dort nur deshalb zusammen mit ESP 3.2.29).
**A59 und A2 kommen vor A50 und A55**, wie vom Nutzer festgelegt; S0 macht diese Reihenfolge
flash-seitig erst möglich (`design.md` §1.2).

---

## Problem

### 0. Flash F103 — 96 Byte Spielraum

**✔ (Auftrag):** 1'120 Byte frei, Gate ≥ 1'024, also **96 Byte**. **✔ V, an der Map vom Lead
nachgeprüft:** `my_gmtime()` (`src/base/base.c:450-531`) rechnet mit 64-Bit-`time_t`; damit zieht
der Linker `__udivmoddi4` (0x318) und `__aeabi_ldivmod` (0xa0) herein, zusammen **952 Byte**
(`build/stm-rgbw-12h/wordclock_f103_rgbw.map:874`, `:1028`, Build 11:02 = 3.2.25). Einzige
64-Bit-Quelle ist `time_t` (`base.c`; einziger Aufrufer `src/timeserver/timeserver.c:250-256`,
`:307`); die Eingabe ist ohnehin `uint32` (`seconds_since_1900 - 2208988800U`, `:253`).

**Kosten laut V:** A59 rund 25 Byte; A2 rund 50 (CRC-Mindestvariante) bis 100 Byte; A55 rund
6 Byte; **A50 spart** rund 180–250 Byte (die Fault-Ausgabe entfällt). Reserve ohne
Verhaltensänderung: A52 (toter Zweig `FILE `) rund 25 Byte. **C42 und die Debugtexte sparen
nichts** — sie werden im Fabrikat schon weggeworfen. Ein eigener Ersatz für `atoi`/`strtoul`
brächte rund 450 Byte, **ändert aber das Verhalten** (Vorzeichen, Leerzeichen, Überlauf) — nur
als Reserve, nicht Teil dieses Pakets.

### 1. A59 / L351 / L364 — die LDR-Grenzen lassen sich über die Zahl nicht setzen

**✔ verifiziert (Testdurchlauf L370 und Code):** `ldr_min_value_set` und `ldr_max_value_set`
kommen als `CMC N11…` bzw. `N12…` am STM an. `schedule_esp8266_numeric_variable()` verwirft
beide als readonly (`src/main.c:1876-1886`); keine `eep`-Zeile folgt. „Messwert übernehmen"
(`ldr_min_set`/`ldr_max_set`) geht als RPC an `ldr_set_min_value()`/`ldr_set_max_value()`
(`src/main.c:1574-1588`, `src/ldr/ldr.c:168-226`) und schreibt RAM **und** EEPROM
(`eep a=225 n=2` bzw. `a=227`, L364).

**Folgen:** Es gibt **keinen** API-Weg, die Grenzen auf einen Zahlenwert zu setzen — weder für
den Rückweg aus S25/S26 noch für den **Backup-Import** (`app.js:6536-6538`: Minimum, 150 ms
später Maximum). **✔ V:** Heute macht der nächste Vollabgleich einen importierten Wert sogar
**still rückgängig**, weil er die STM-Werte an den ESP überträgt. `settings_xml` zeigt die
ESP-Kopie (L188); der STM hält nach dem Durchlauf **5/26**, die ESP-Kopie **16/18**. Ohne
Wirkung, solange die Auto-Helligkeit aus ist (`numvar[8]=0`, L351).

**✔ am Code, zu beachten:** Der ESP weist eine verdrehte Reihenfolge (Minimum ≥ Maximum)
**bewusst nicht** ab (`http.cpp:9841-9852`, L68). Die Laufzeit fängt den Zustand ab
(`ldr.c:72`, `:92-95`: volle Helligkeit), der Start heilt ihn (`ldr.c:128-138`: 0/4095,
geschrieben).

### 2. A2 / Massnahme 16 / CLAUDE.md „Offene technische Themen" 1 — DS18xx ohne Prüfung

**✔ verifiziert (L18; ✔ am Code `675e3fd`, vom `spec-writer` und in V unabhängig gelesen):**

- `ds18xx_read_raw_temp()` (`src/ds18xx/ds18xx.c:171-215`) wertet das Scratchpad **ohne
  CRC-Prüfung** aus; jede gelungene Präsenz liefert `rtc = 1`. **Im ganzen Code gibt es keine
  CRC-8.**
- **L18 ist anders zu lesen als bisher — neu, ✔ V:** CLAUDE.md und `BEFUNDE.md` sagten, „die
  Erkennung scheiterte beim Start". Am Code: `ds18xx_init()` setzt `ds18xx.is_up = 1`, sobald
  die **Präsenz** antwortet (`ds18xx.c:298-303`) — auch dann, wenn die **ROM-ID** falsch gelesen
  wurde; `onewire_get_rom_code()` prüft nichts (`src/onewire/onewire.c:211-228`). Danach
  adressiert jedes `MATCH ROM` niemanden, und es kommt **255 bis zum nächsten Reset**. Genau das
  Bild aus L18. **Eine Neuerkennung nur bei `is_up == 0` hätte L18 nicht behoben** — sie braucht
  die ROM-CRC als Voraussetzung.
- **neu, ✔ am Code:** Bei einem gescheiterten Lesen gibt `temp_read_temp_index()` 255 zurück,
  lässt aber `gtemp.index` **auf dem alten Wert** (`src/tempsensor/tempsensor.c:55-78`, `:76`).
  Die Logzeile in `main.c:4365` zeigt „127.5", an den ESP geht über
  `var_send_ds18xx_temp_index()` (`src/vars/vars.c:1452-1455`) der **veraltete** Wert, und die
  PWA zeigt ihn als gültig.
- `var_send_ds18xx_is_up()` ist `static` und läuft nur im Vollabgleich (`vars.c:1430-1434`,
  `:1847`). Der Messzweig liegt bei Sekunde 49/50 (`main.c:927-934`, `:4348-4376`).
- **Anzeige, ✔ am Code:** Die PWA zeigt 255 schon als „ungültig" (`app.js:14411-14413`, Text
  `:917`), die Zeile „DS18xx online" nur an/aus (`:7699-7700`). **Ein neues numvar ist nicht
  nötig.** Die **Legacy-Seite** formatiert 255 als „127.5 °C" (`http.cpp:3919-3932`) — Ent-11.
- **HARDWARE.md (✔ V):** Am Bus hängt genau ein Sensor. Damit wäre `SKIP ROM` statt `MATCH ROM`
  möglich — eine Protokolländerung am Bus, Ent-13.

### 3. A50 / L313 — `fault_reset()` setzt nie selbst zurück

**✔ am Code (`src/main.c:590-613`), ✔ V:** Nur die vier Handler `main.c:615-637` rufen
`fault_reset()`; drei davon sind unerreichbar, weil `SHCSR` sie nicht freischaltet — es läuft
also immer der **HardFault** (Priorität −1). Darin läuft die USART-ISR nicht, `log_flush()`
wartet ewig (`src/uart/uart-driver.h:752-759`), `NVIC_SystemReset()` (`main.c:607`) wird nie
erreicht. **Es ist nie ein Zeichen der Fault-Ausgabe angekommen** — die Ausgabe entfällt also
ohne Verlust. Läuft der Watchdog, folgt nach 20 s ein Reset; **bei einem Fehler vor
`watchdog_init()` (`main.c:3790`) bleibt die Uhr für immer stehen.**

**Risiko eines sofortigen Resets, ✔ V:** Ein deterministischer Fault nach `esp8266_init()`
erzeugte einen Reset-Takt von 3–4 s. Jeder STM-Start setzt den ESP zurück, der ESP braucht rund
10 s — er wäre **nie** erreichbar, und ein Web-Flash wäre nicht mehr möglich. Deshalb wartet der
Fehlerpfad vor dem Reset (Ent-7).

**Nebenbefund (V):** `tools/logger/README.md:352` beschreibt eine Fault-Ausgabe, die es nie gab —
Doku, Lead (Z.3).

**Am Gerät nicht herstellbar:** Ein Fault auf der produktiven Uhr wird nicht ausgelöst.

### 4. A55 / L327 — jeder ESP-Neustart kostet rund vier Zeitüberschreitungen

**✔ gemessen (`BEFUNDE.md`, A55, 10.10.2026):** Bei beiden ESP-OTAs (00:13:56 und 00:35:55)
stieg `v` mit STM 3.2.22 um je **4**. **✔ V:** Die vier sind **Sendungen des STM** (`N15`, `N31`,
`N17`, `T00`) in die rund **10 s Bootzeit** des ESP; `var_send_buf()` wartet je 3 s
(`src/vars/vars.c:683-694`). Gesendet werden sie nur, solange `esp8266.is_online` gilt (u. a.
`main.c:4034`, `:4073`).

**✔ V, ✔ am Code:** Das **erste verlässliche Lebenszeichen** eines startenden ESP ist die Zeile
`CAP var-crc`, rund 1 s nach dem Boot (`src/esp8266/esp8266.c:543-553`). Das Muster „offline
setzen, bis `IPADDRESS` oder `SYNCVARS` kommt" gibt es schon bei der AP- und der WPS-Taste
(`main.c:3945`, `:3953`); beide Zeilen setzen `is_online` wieder (`esp8266.c:590-596`, `:597-626`).

**✔ am Code, zu erhalten:** Bei toter Brücke ist der Watchdog-Reset die einzige Selbstheilung
(`vars.c:738-760`, L25).

### 5. C47 / L346 — die Legacy-Seite gibt gespeicherte Werte roh als HTML aus

**✔ verifiziert (Review F.6, vom `esp-developer` bestätigt 08.10.2026; ✔ am Code `675e3fd`):**
`input_field()` (`http.cpp:2667-2693`) schreibt den Wert mit `http_send (value)` (`:2687`)
unmaskiert ins `value`-Attribut — für **jeden** Legacy-Eingabewert, darunter Update-Host,
Update-Pfad (`:7453-7454`) und Ticker-Text. Alle sind aus dem LAN setzbar; ein `"` genügt.
Dazu `new_esp_version` und `new_wc_version` vom Update-Server (`:7463`, `:7481`) und die
**Release Notes**, die zeilenweise roh durchgereicht werden (`:7383-7414`) und **absichtlich
HTML** sind — eigene Entscheidung (Ent-2). Die PWA bereinigt dieselben Release Notes selbst
(`app.js:7415-7433`).

**neu, ✔ am Code — Falle beim Beheben:** Die vorhandene Maskierung `HTTP_ESCAPE_XML`
(`http.cpp:389`, `:7711-7835`) ersetzt **ungültiges UTF-8 durch `?`** (`:7768-7777`). Steht in
einem Wert ein ISO-8859-1-Byte, käme es im Formular als `?` zurück — und ein erneutes Speichern
des Legacy-Formulars schriebe das `?` ins Gerät. **Ein stiller Wertschaden durch die Korrektur
selbst.** Die Maskierung für die Legacy-Werte darf deshalb nur die HTML-Sonderzeichen ersetzen
(AKX.1).

### 6. C55 / L369 — Datumsformat ohne Inhaltsprüfung

**✔ verifiziert (Testdurchlauf 10.10.2026; ✔ am Code):** `http_api_date_ticker_format_set()`
(`http.cpp:8540-8566`) prüft nur die Byte-Länge (`MAX_DATE_TICKER_FORMAT_LEN` = 5); `yyyyy`,
`<scri`, `⏰` und `  ab` werden angenommen. **neu, ✔ am Code:** Der Legacy-Weg `savedtf`
(`http.cpp:4460-4466`) prüft **gar nichts**. Der STM deutet `d D m M y Y` als Platzhalter und
übernimmt jedes andere Zeichen wörtlich (`src/main.c:4560-4588`).

### 7. C44 / L340 — Koordinaten ohne Inhaltsprüfung

**● gemeldet, am Code teilweise widersprochen.** L340 (Testdurchlauf S.26): `8,5400` und `<"&`
werden angenommen, der Wetterabruf scheitert danach **still**.
✔ am Code: `http_api_weather_coordinates_set()` (`http.cpp:8680-8722`) prüft nur Länge, Paar und
Leerstand — `<"&` wird also angenommen. **Aber:** Dieselbe Funktion ersetzt `,` durch `.`
(`:8713-8714`); `8,5400` würde als `8.5400` gespeichert und wäre gültig. **Vor der Umsetzung zu
klären** (E.0): Wie kam `8,5400` in L340 zustande — über den Legacy-Weg `savelonlat`
(`:4002-4013`, ersetzt ebenfalls), über den Import, oder ist der Befund an dieser Stelle falsch?
**neu, ✔ am Code:** `savelonlat` prüft weder Länge noch Inhalt.

### 8. B44–B47 — vier PWA-Befunde aus dem Testdurchlauf 10.10.2026

| | Befund | Status | Fundstelle |
|---|---|---|---|
| **B44** (L365) | Ein geleertes Zeitfeld wird still als **aktiver Timer 00:00** gespeichert — am Gerät belegt (`nighttime[2]`, sofort zurückgestellt) | ✔ Gerät; ✔ am Code | `saveTimerRow()`, `app.js:13225` (`value \|\| "00:00"`); `parseTimeInput()` `:12631-12637` fällt **ebenfalls** auf `"00:00"` zurück; DFPlayer-Alarme `:13000` dasselbe Muster (✔ am Code, am Gerät nicht geprüft); „Alle speichern" `saveAllTimerRows()` `:13252-13274` |
| **B45** (L366) | Nach dem Speichern **eines** Felds gehen andere ungespeicherte Eingaben beim Modulwechsel **ohne Warnung** verloren | ✔ Testlauf; ✔ am Code | `markEditsPersisted()` leert die ganze Menge `unsavedEditFields` (`app.js:2176-2179`); B41 sicherte nur Auswahllisten (`:2152-2160`) |
| **B46** (L367) | AP-SSID im Netzwerkmodul leer, bis die Wartung einmal offen war | ✔ Testlauf; ✔ am Code | `eeprom_settings` wird nur bei `maintenanceActive` geladen (`app.js:2974-2995`); Anzeige `:3681`, `:4535` |
| **B47** (L368) | Nach einem Hintergrundklick im Kartenmodal geht der Fokus verloren | ✔ Testlauf; ✔ am Code | `handleModalBackdropClick` hängt an **`mousedown`** (`app.js:8558-8564`, `:8583`); die Standardaktion des Klicks legt den Fokus danach auf den Body |

---

## Ziel

1. **Flash:** Nach S0 ist für A59, A2, A50 und A55 genug Platz über der Reserve; die Uhr rechnet
   Datum, Uhrzeit und **Wochentag** für 1970–2106 genau wie vorher.
2. **LDR-Grenzen:** Ein Zahlenwert über die API kommt am STM an, steht im EEPROM und übersteht
   einen Neustart; der Backup-Import wirkt am STM. Die Grenzen am Gerät stehen danach auf dem
   Wert, den der Nutzer bestimmt (Ent-3), und S25/S26 sind im Testplan **rücknehmbar**.
3. **DS18xx:** Kein Messwert mit falscher Prüfsumme und keine falsch gelesene ROM-ID wird
   verwendet; ein ungültiger Messwert erscheint überall als **ungültig**, nie als alter Wert; ein
   nicht erkannter Sensor wird **ohne Reset** erkannt.
4. **Fehlerpfad:** Ein CPU-Fault führt **auf jedem Pfad** zu einem Reset, auch vor
   `watchdog_init()` — und nicht so schnell, dass der ESP nie erreichbar wäre.
5. **ESP-Neustart:** Ein ESP-Neustart kostet **höchstens eine** Zeitüberschreitung statt rund
   vier, und eine tote Brücke verhält sich **wie heute**.
6. **Legacy-Seite:** Kein gespeicherter oder vom Update-Server stammender Wert kann aus seinem
   HTML-Kontext ausbrechen; ein erneutes Speichern des Formulars ändert keinen Wert.
7. **Datumsformat und Koordinaten:** ESP **und** PWA weisen ungültige Werte nach **derselben**
   Regel ab, auf **allen** Schreibwegen (API, Legacy, Karte, Import).
8. **PWA:** Kein stiller Timer 00:00, keine still verlorene Eingabe, AP-SSID ohne Umweg, Fokus
   bleibt nach dem Schliessen des Modals erhalten.
9. Jede Wirkung ist **einzeln** nachgewiesen — am Gerät, wo es herstellbar ist, sonst am
   Prüfstand —, und jede neue Prüfung ist **einmal fehlgeschlagen** (DIR-014).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt**
(DIR-014). Prüfstände laufen über `tools/checks/auszug.sh` mit den Typbreiten des Ziels (L256)
und sind **einmal gegen den Ausgangsstand fehlgeschlagen**; die Fallzahl wird gemeldet und
nachgezählt. **Was am Gerät nicht herstellbar ist, wird nicht am Gerät verlangt.**

### Schritt S0 — `my_gmtime()` mit 32 Bit

- [ ] **AKG.1 (gleiches Ergebnis)** — `my_gmtime()` liefert für **jede** Eingabe 0..2³²−1 dieselbe
      `struct tm` wie heute, **alle** Felder einschliesslich `tm_wday` und `tm_yday`. *Instrument:*
      Prüfstand **GT** (alt gegen neu, Host, Typbreiten des Ziels): jeder Tagesbeginn von 1970 bis
      2106 (rund 49'710 Tage) mit den Sekunden 0, 1, 43'200, 86'399; dazu 2000-02-29,
      2038-01-19 03:14:07/08, 2100-02-28/03-01 (kein Schaltjahr), 2106-02-07 06:28:15
      (`0xFFFFFFFF`) und 100'000 Zufallswerte mit festem Startwert. **Zusätzlich** gegen das
      `gmtime_r()` des Hosts als Referenz; weicht schon die **alte** Fassung davon ab, wird das
      **berichtet, nicht still korrigiert** — Massstab bleibt „wie heute". **Gegenprobe:**
      eine Sabotage (Jahrhundertregel entfernt) meldet FEHL. **Zähl nach:** über 300'000 Fälle.
- [ ] **AKG.2 (64-Bit-Division weg)** — In der Map des F103-Testbaus stehen **weder**
      `__udivmoddi4` **noch** `__aeabi_ldivmod`. *Instrument:* statische Prüfung des Leads über die
      Map; **Gegenprobe:** gegen die Map des Ausgangsstands schlägt sie an (`:874`, `:1028`).
- [ ] **AKG.3 (Flash)** — Testbau F103: Rest um **mindestens 900 Byte** gestiegen; Zahl gemeldet.
- [ ] **AKG.4 (Gerät, G0, lesend)** — Nach dem S0-Flash: Uhrzeit der Uhr gleich der des Mac auf
      ±2 s (Zeitzone eingerechnet), **Wochentag** und Datum richtig (Datumsticker oder
      `/api/…`-Abfrage), die nächste Netzzeit-Synchronisierung ohne Sprung. Ein Timer mit
      Wochentagsbezug löst am erwarteten Tag aus — **lesend** über den Mitschnitt, nicht durch
      einen neuen Timer.

### Schritt S1 — A59 (LDR-Grenzen)

- [ ] **AKL.1 (Setter angewandt)** — `LDR_MIN_VALUE_NUM_VAR` und `LDR_MAX_VALUE_NUM_VAR` in
      `schedule_esp8266_numeric_variable()` (`main.c:1876-1886`): Ein Wert **0..4095** wird in
      `ldr.ldr_min_value` bzw. `ldr.ldr_max_value` übernommen und über **denselben** Schreibweg
      wie „Messwert übernehmen" ins EEPROM geschrieben (`ldr_write_config_to_eep()`,
      `ldr.c:147-162`). **Keine zweite Schreibfunktion.** Ein Wert **über 4095** wird nicht
      angewandt, nichts wird geschrieben. *Instrument:* Prüfstand **L59** (Auszug der beiden
      Zweige und von `ldr.c` mit EEPROM-Attrappe): 0, 1, 4095, 4096, 65535, unveränderter Wert
      (kein Schreibzyklus). **Gegenprobe:** gegen `release/3.2.25-3.2.30-1.4.94` wird kein Wert
      übernommen ⇒ FEHL.
- [ ] **AKL.2 (Reihenfolge nicht gesperrt)** — Minimum ≥ Maximum wird **angenommen** wie beim ESP
      (`http.cpp:9841-9852`), damit der Backup-Import (`app.js:6536-6538`) nicht auf halbem Weg
      scheitert. *Instrument:* Prüfstand L59, Fall „Minimum 3000 bei Maximum 100, danach Maximum
      4000" ⇒ beide Werte stehen. Das Verhalten der Laufzeit (`ldr.c:72`) und beim Start
      (`ldr.c:128-138`) bleibt unverändert und ist im Bericht benannt.
- [ ] **AKL.3 (Gerät, G1 — Bereinigung)** — Nach dem S1-Flash, **mit Freigabe** (Ent-1, Zielwert
      nach Ent-3): `ldr_min_value_set`, dann `ldr_max_value_set`. Im Mitschnitt **und** in
      `/api/stm32_log` je eine Zeile `eep a=225 n=2` bzw. `eep a=227 n=2` (sofern der STM-Wert
      sich ändert); `numvar[17]`/`numvar[18]` (ESP-Kopie) gleich dem Zielwert. **Der schlechte
      Wert liegt vor:** Derselbe Aufruf erzeugte unter STM 3.2.25 **keine** `eep`-Zeile (L364).
      Der Zielwert steht im Bericht **als vom Nutzer bestätigter Wert**, nicht als Vorschlag.
- [ ] **AKL.4 (Gerät — übersteht den Neustart)** — Nach dem **nächsten STM-Reset** (Ent-4: der
      S2-Flash oder ein eigener Reset) zeigen `numvar[17]`/`numvar[18]` nach dem Vollabgleich den
      Zielwert. Weil der Vollabgleich die **STM**-Werte überträgt, ist das der Beleg für RAM
      **und** EEPROM des STM — nicht nur für die ESP-Kopie (L188).

### Schritt S1 — A2 (DS18xx)

- [ ] **AKT.1 (Scratchpad-CRC)** — `ds18xx_read_raw_temp()` prüft die Dallas-CRC-8 (Polynom
      x⁸+x⁵+x⁴+1) über Byte 0–7 gegen Byte 8. Ungleich ⇒ Messung **ungültig**. **Eine** CRC-8-Funktion
      für Scratchpad und ROM-ID. *Instrument:* Prüfstand **DS** (Auszug von `ds18xx.c` und
      `tempsensor.c` mit Eindraht-Attrappe): ein gültiges Scratchpad aus Datenblatt oder Maxim
      Application Note 27, **jedes der 72 Bits einzeln gekippt** (alle 72 ⇒ ungültig), neun
      0xFF (getrennte Leitung), fehlende Präsenz. Neun Nullbyte bestehen die CRC — ob dafür eine
      Zusatzprüfung nötig ist, entscheidet das Review S1.5 und benennt es. **Gegenprobe:** der
      alte Stand nimmt die gekippten Bits an ⇒ FEHL. **Zähl nach:** mindestens 74 Fälle.
- [ ] **AKT.2 (ROM-ID-CRC, `is_up` nur bei gültiger ID)** — `ds18xx_init()` setzt `is_up = 1` **nur**,
      wenn die CRC-8 über Byte 0–6 der ROM-ID gleich Byte 7 ist; sonst `is_up = 0`, kein Messen
      mit dieser ID. *Instrument:* Prüfstand DS, ROM-ID mit gekipptem Bit ⇒ nicht erkannt;
      **Gegenprobe:** alter Stand meldet `is_up = 1` ⇒ FEHL. **Das ist die Korrektur von L18.**
- [ ] **AKT.3 (ungültig heisst 255, überall)** — Nach einem CRC- oder Lesefehler steht
      `gtemp.index` auf **255**, und genau dieser Wert geht an den ESP — nicht der vorige
      Messwert (heute `tempsensor.c:76`). **Kein neues numvar**; die PWA zeigt 255 schon als
      ungültig. Die Logzeile `main.c:4365` lautet dann **`DS18xxx temperature: ungueltig`** statt
      „127.5" (Umschrift, DIR-015; Wortlaut verbindlich, damit der Suchtext stimmt). Ein internes
      Merkmal „letzter Messwert gültig" trägt die Unterscheidung im STM. *Instrument:* Prüfstand
      DS; **Gegenprobe:** alter Stand überträgt nach gültig→ungültig den alten Wert ⇒ FEHL.
- [ ] **AKT.4 (Neuerkennung zur Laufzeit)** — Nach Ent-6: Ist kein Sensor erkannt, versucht der
      STM die Erkennung **einmal je Minute** im bestehenden Sekunde-49-Zweig (`main.c:4348-4356`)
      erneut. Jeder Wechsel von `is_up` geht **sofort** an den ESP (`var_send_ds18xx_is_up()`
      nicht mehr nur im Vollabgleich) und erzeugt **genau eine** Logzeile: **`ds18xx erkannt`**
      bzw. **`ds18xx verloren`** (Wortlaut verbindlich). Ohne Wechsel keine Zeile. *Instrument:*
      Prüfstand DS mit simulierter Zeit: Start mit falsch gelesener ROM-ID ⇒ erkannt im nächsten
      Minutenfenster (der L18-Fall); Start ohne Sensor, Sensor ab Minute 5 ⇒ erkannt in Minute 5
      oder 6; bei Ent-6 (a) zusätzlich: drei ungültige Messungen in Folge ⇒ `is_up = 0`, eine Zeile,
      Neuerkennung. **Gegenprobe:** alter Stand erkennt nie ⇒ FEHL.
- [ ] **AKT.5 (Blockade)** — Ein Erkennungsversuch blockiert den Hauptloop **höchstens 50 ms**
      (V: rund 20–40 ms; Rücksetzen, ROM-ID lesen, Auflösung schreiben nur, wenn sie abweicht,
      `ds18xx.c:320-323`). **Keine neue `watchdog_reload()`-Stelle** (S7 zeigt denselben
      Bestand). *Instrument:* Review S1.5 mit Rechnung aus den Eindraht-Zeiten.
- [ ] **AKT.6 (Gerät, G1, lesend)** — 60 Minuten nach dem S1-Flash: mindestens 55 Zeilen
      `DS18xxx temperature:` mit plausiblen Werten (15 bis 35 °C), **keine** Zeile
      `ungueltig`, keine Zeile `ds18xx verloren`; `numvar[20]` (`DS18XX_IS_UP`) = 1. Fällt doch
      ein `ungueltig`, ist das **kein** Fehlschlag, sondern ein Messergebnis — es wird mit
      Zeitpunkt berichtet. **Der schlechte Fall ist am Gerät nicht herstellbar** (Ent-14); ihn
      belegt der Prüfstand.

### Schritt S2 — A50 (Fehlerpfad)

- [ ] **AKF.1 (Reset auf jedem Pfad, ohne ISR)** — `fault_reset()` ruft **keine** Ausgabe über den
      Ring und kein `log_flush()` mehr und erreicht `NVIC_SystemReset()` auf jedem Pfad — bei
      vollem TX-Ring, vor `log_init()`, vor `watchdog_init()`. *Instrument:* Prüfstand **FR**:
      Attrappen, bei denen `uart_putc()` bei vollem Ring und `log_flush()` ohne laufende ISR nie
      zurückkehren (begrenzter Zähler meldet „hängt"); Fälle: Ring leer, Ring voll, UART nicht
      initialisiert. Dazu **statisch**: kein `log_`-Aufruf in `fault_reset()`. **Gegenprobe:**
      alter Stand erreicht den Reset im Fall „Ring voll" nicht ⇒ FEHL.
- [ ] **AKF.2 (Wartezeit nach Ent-7)** — Vor dem Reset wartet der Fehlerpfad **länger als die
      Bootzeit des ESP und kürzer als der Watchdog**: Vorgabe **rund 15 s** (über 12 s, unter
      20 s). **F1c (Empfehlung):** geeicht über einen Zähler, der im HardFault weiterläuft
      (SysTick-Zählregister pollen oder Zyklenzähler) — die Rechnung steht im Kommentar und im
      Bericht. **F1b:** die bestehende Schleife (`main.c:602`) verlängert, ungeeicht, 0 Byte —
      dann nennt der Bericht die **gerechnete** Dauer je Ziel (F103 72 MHz, F411) und ihre
      Unsicherheit. *Instrument:* Prüfstand FR (Zählerstand bis zum Reset), Review S2.5.
- [ ] **AKF.3 (Flash)** — Testbau F103 nach S2.1: Zahl gemeldet; erwartet laut V rund 180–250
      Byte **weniger** Verbrauch.
- [ ] **AKF.4 (kein Gerätenachweis)** — Ein Fault wird am Gerät **nicht** ausgelöst. Am Gerät nur:
      G2 ohne neue Exception, ohne Neustart (60 min lesend).

### Schritt S2 — A55 (ESP-Neustart)

- [ ] **AKP.1 (offline ab der Bootmeldung)** — Nach Ent-8. **P3 (Empfehlung, V):** Im Zweig
      `CAP var-crc` (`esp8266.c:543-553`) setzt der STM `esp8266.is_online = 0` — dasselbe Muster
      wie bei AP- und WPS-Taste (`main.c:3945`, `:3953`). `IPADDRESS` und `SYNCVARS` setzen es
      wieder (unverändert). Kein neuer Zustand, kein neues Flag, keine neue Logzeile — die
      Zeile `(CAP var-crc)` steht schon im Mitschnitt (`esp8266.c:538-541`). *Instrument:*
      Prüfstand **EP** (Auszug des Zeilenparsers und der `is_online`-Bedingung der periodischen
      Sendungen, simulierte Zeit): ESP-Neustart mit Folge „schweigt 1 s, `CAP`, `FIRMWARE`,
      schweigt 9 s, `SYNCVARS`" bei den vier periodischen Sendungen aus A55 ⇒ **höchstens eine**
      Zeitüberschreitung; danach `is_online = 1`. **Gegenprobe:** alter Stand ⇒ mehr als eine ⇒
      FEHL.
- [ ] **AKP.2 (kein Steckenbleiben)** — Bleiben `IPADDRESS` **und** `SYNCVARS` nach `CAP` aus, ist
      das Verhalten benannt und vertretbar: Der STM sendet dann nichts Periodisches an den ESP,
      bis eine der beiden Zeilen kommt — dieselbe Lage wie nach der AP-Taste heute und wie L255
      sie beschreibt. Das Review S2.5 belegt am Code, **wann** der ESP `SYNCVARS` sendet (nach
      jedem Start, weil seine Variablen auf den Vorgaben stehen) und ob ein **Zeilenverlust** hier
      einen Dauerzustand erzeugen kann; falls ja, wird das als Befund gemeldet, nicht still
      mitgebaut. Eine **tote** Brücke (kein `CAP`) verhält sich wie heute.
- [ ] **AKP.3 (A2 unberührt)** — Der Wetterpfad aus Paket 2026-10-09 (A2) verhält sich
      unverändert: Prüfstand **W2** besteht weiter mit seiner Fallzahl (37).
- [ ] **AKP.4 (Gerät, G3)** — Beim **ESP-OTA des Schritts E**, mit laufendem
      `./tools/watch-log.sh` (vor dem OTA gestartet): `v` steigt um **0 oder 1** (V: „nicht
      garantiert 0"); im Mitschnitt `(CAP var-crc)` und danach `SYNCVARS` oder `IPADDRESS`; der
      Vollabgleich läuft vollständig (Abschlussmarke), `diag` lückenlos, kein Watchdog-Reset.
      **Vorher-Wert:** +4 bei beiden OTAs vom 10.10.2026 (A55). **AKS.6** aus
      `specs/paket-2026-10-06/` gilt damit in der Fassung „steigt je ESP-Neustart um höchstens
      1" — die alte Spec bleibt als Momentaufnahme unverändert.
- [ ] **AKP.5 (Watchdog)** — Keine neue `watchdog_reload()`-Stelle; kein Pfad hält den Hauptloop
      länger als heute. *Instrument:* S7, Review S2.5.

### Flash-Gate (S0, S1, S2)

- [ ] **AKZ.F** — Testbau F103 nach **jedem** STM-Task (S0.1, S1.1, S1.2, S2.1, S2.2): Rest
      **≥ 1'024 Byte**. Ausgangslage 1'120 Byte frei. Darunter: **anhalten und dem Nutzer die
      Sparvarianten vorlegen** (Ent-5). Gegenprobe S8b im jeweiligen Release-Build; **kein
      Rollout unter 1'024 Byte**. Jeder Zuwachs steht neben der Schätzung aus V.

### Schritt E — C47 (Legacy maskiert)

- [ ] **AKX.1 (Maskierung ohne Wertschaden)** — `input_field()` gibt den Wert über eine Maskierung
      aus, die **genau** `&`, `<`, `>`, `"`, `'` ersetzt und **jedes andere Byte unverändert**
      lässt — keine UTF-8-Prüfung, kein `?` (Problem 5). *Instrument:* Prüfstand **t15**:
      `a"b<c>&'d`, ISO-8859-1-Byte 0xE4, UTF-8 „ä", Emoji, leerer Wert, 64-Byte-Wert; dazu
      **Rundlauf**: die Ausgabe, so dekodiert wie ein Browser das Attribut dekodiert, ist
      **byte-gleich** mit der Eingabe. **Gegenprobe:** alter Stand gibt `"` roh aus ⇒ FEHL.
- [ ] **AKX.2 (alle Werte, nicht nur `input_field()`)** — Jede Ausgabe eines gespeicherten oder
      fremden Werts in den Legacy-Seiten läuft über diese Maskierung; dazu `new_esp_version` und
      `new_wc_version`. Der `esp-developer` legt eine **Bestandsliste** vor (Datei:Zeile, Zahl).
      *Instrument:* **statische Prüfung** des Leads: kein `http_send (` mit nicht-literalem
      Argument in den Legacy-Seitenfunktionen ausser einer benannten Erlaubnisliste (Konstanten,
      Zahlenpuffer) mit Grund je Eintrag. **Gegenprobe:** schlägt gegen den Ausgangsstand an
      (mindestens `:2687`, `:7463`, `:7481`). **Zähl nach:** Zahl der geprüften Aufrufe gegen die
      Bestandsliste.
- [ ] **AKX.3 (Release Notes nach Ent-2)** — Umgesetzt wie entschieden; bei **R2 (Empfehlung)**:
      alles zwischen `<` und `>` entfällt, ein einzelnes `>` wird maskiert, Text und Entitäten
      bleiben; jede Zeile endet mit `<br>`. *Instrument:* t15 mit `<script>`, `<img onerror=…>`,
      `<a href="javascript:…">`, einem über zwei Zeilen geteilten Tag und einer Zeile über
      128 Zeichen (heute `linebuf[128]`, `:7387`); **Gegenprobe:** alter Stand reicht `<script>`
      durch ⇒ FEHL.
- [ ] **AKX.4 (Gerät, G3, schreibend mit Rückstellung)** — **Mit Freigabe (Ent-1):** Originalwert
      des Wetterorts lesend festhalten; Wetterort auf `x"y<z` setzen (stösst keinen Abruf an,
      Paket 2026-10-09 `design.md` §4.2); die Legacy-Seite mit dem Ortsfeld lesend abrufen:
      enthält `x&quot;y&lt;z`, **nicht** `x"y<z`; Original zurück. **Nicht** Update-Host oder
      -Pfad. Jede Legacy-URL hat Parameter **mit** `=` (R5: `GET /?a` bringt den ESP zum Absturz).

### Schritt E — C55 und C44 (Regeln im ESP)

- [ ] **AKR.1 (Datumsformat)** — Eine Prüffunktion nach Ent-10, gerufen vom API-Setter **und**
      vom Legacy-Weg `savedtf` (`http.cpp:4460-4466`). Abweisung über die API mit
      `HTTP_API_ERROR_OUT_OF_RANGE` und Meldung, im Legacy-Weg mit Meldung statt „successfully
      changed". Gültige Werte wie heute.
- [ ] **AKR.2 (Koordinaten)** — Eine Prüffunktion nach Ent-12, gerufen vom API-Setter **und** von
      `savelonlat` (`:4002-4013`); Komma wird **vor** der Prüfung zu Punkt (wie heute); die
      bestehenden Regeln (Paar, leer nur mit Ort, `:8693-8711`) bleiben. Abweisung wie AKR.1.
- [ ] **AKR.3 (Prüfstand t16)** — **Eine Fall-Tabelle** (Datei, maschinenlesbar) mit mindestens
      40 Fällen je Regel, darunter alle Werte aus L369 und L340 und die Grenzen (±90, ±180, acht
      Zeichen, `-0`, `.5`, `5.`, `+5`, `1e2`, Leerzeichen). **Gegenprobe:** alter Stand nimmt
      `<"&` und `<scri` an ⇒ FEHL.
- [ ] **AKR.4 (Gerät, G3)** — `date_ticker_format_set` mit `yyyyy` (falls nach Ent-10 ungültig),
      `<scri`, `⏰`, `  ab` ⇒ abgewiesen, Wert unverändert (lesend nachgeprüft);
      `weather_coordinates_set` mit `<"&` ⇒ abgewiesen. **Mit Freigabe (Ent-1):** je ein gültiger
      Wert angenommen, danach Original zurück. Vor dem Test liegt der Befund zu `8,5400` aus E.0
      vor.

### Schritt P — PWA

- [ ] **AKR.5 (gleiche Regel in der PWA)** — Die PWA prüft Datumsformat und Koordinaten **vor dem
      Senden** nach **derselben** Regel, auf allen Wegen: Speichern (`app.js:8341-8349`,
      `:8404-8418`), Kartenauswahl (`:8960-8985`), Backup-Import (`:6501`, `:6555-6582`).
      Abgewiesen ⇒ kein Aufruf, Meldung am Feld; beim Import eine Notiz wie bei
      `acceptImportTextOrNote` und kein Senden der Ortsangabe. *Instrument:* **Regelgleichheit**:
      dieselbe Fall-Tabelle aus AKR.3 läuft gegen die PWA-Funktion (Node, `.mjs`) — jede
      Entscheidung gleich der des ESP. **Gegenprobe:** alter Stand der PWA nimmt `<"&` an ⇒ FEHL;
      eine absichtlich abweichende Regel meldet den Unterschied (die Meldung kommt an, DIR-014).
- [ ] **AKB.1 (B44)** — Ein leeres Zeitfeld führt nach Ent-9 zu **keinem** Aufruf, sondern zu
      einer Meldung, die die Zeile nennt — in `saveTimerRow()`, in `saveDfplayerAlarm()` und in
      `saveAllTimerRows()`, dort **bevor** irgendeine Zeile gesendet wird. `parseTimeInput()`
      hat keinen stillen Rückfall auf `"00:00"` mehr. *Instrument:* Vorschau (`/pwa-vorschau`) mit
      **echten Tastenereignissen** (Feld leeren, aktiv setzen, Speichern): kein Aufruf im
      Netzprotokoll. **Gegenprobe:** alter Stand sendet `hour=0&minute=0` ⇒ FEHL.
- [ ] **AKB.2 (B45)** — Nach dem Speichern von Feld A bleibt die ungespeicherte Eingabe in Feld B
      **im Feld stehen** (auch nach dem Neuladen), und der Modulwechsel fragt nach. **Gegenfall
      (L33):** Nach dem Speichern des **einzigen** geänderten Felds fragt der Modulwechsel nicht,
      und die Selbstaktualisierung läuft wieder. *Instrument:* Vorschau mit echten Ereignissen,
      beide Fälle; **Gegenprobe:** alter Stand verliert B ohne Rückfrage ⇒ FEHL.
- [ ] **AKB.3 (B46)** — Wird das Netzwerkmodul direkt geöffnet, steht die AP-SSID nach dem ersten
      Ladezyklus im Feld, ohne Besuch der Wartung. Der WLAN-Schlüssel aus `eeprom_settings`
      (`http.cpp:8976-8996` liefert ihn im Klartext) erscheint in **keinem** neuen Feld, keiner
      Konsolenausgabe und keinem Log. *Instrument:* Vorschau; Review P.6. **Gegenprobe:** alter
      Stand zeigt `""` ⇒ FEHL.
- [ ] **AKB.4 (B47)** — Ein Klick auf den Hintergrund schliesst das Kartenmodal, und der Fokus
      steht danach auf dem Element, das es geöffnet hat (`modalReturnFocus`), **nicht** auf
      `body`. Ein Ziehen, das in der Karte beginnt und auf dem Hintergrund endet, schliesst
      **nicht**. Escape unverändert. *Instrument:* Vorschau mit echten Mausereignissen;
      `ui-reviewer`. **Gegenprobe:** alter Stand ⇒ `document.activeElement === body` ⇒ FEHL.
- [ ] **AKT.7 (Anzeige A2)** — PWA: drei Zustände in der Klima-Übersicht — „nicht gefunden"
      (`DS18XX_IS_UP` = 0), **„Messwert ungültig"** (`IS_UP` = 1 und Index 255), Messwert. Neuer
      Text in `app.js` **und** `i18n/en.json` (S8 prüft die Umlaute). Legacy nach Ent-11.
      *Instrument:* Vorschau mit den drei Zuständen.
- [ ] **AKB.5 (Browser)** — `./tools/check-pwa.sh` nach dem Hochladen: **kein** Fehler beim Laden
      von `app.js` (DIR-016).

### Für alle Schritte

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `./tools/guardrails.sh --full` vor jedem Release mit Exit 0 (Lead, R1);
      `./tools/checks/auszug.sh` meldet am Ende die neuen Prüfstände GT, L59, DS, FR, EP, t15, t16
      und die Regelgleichheit, alle OK, dazu die Map-Prüfung aus AKG.2.
- [ ] **AKZ.3 (Release-Inhalt)** — Vor **jedem** Release-Build zeigt
      `git diff --name-only <letztes Release-Tag>` samt Arbeitsbaum Produktänderungen **nur** in
      den Komponenten, deren Version dieser Build anhebt (`src/**` ⇔ STM; ESP-Quellen ⇔ ESP;
      `data/app/**` ⇔ PWA). Sonst **kein** Build. *Instrument:* Lead, Ausgabe im Bericht.
- [ ] **AKZ.4** — `./tools/smoke-device.sh` nach jedem Einspielen ohne Fehlschlag; nach dem
      ESP-Update **samt Update-Quelle** (DIR-009) und `./tools/install-app.sh --check` (DIR-017).
      **Der Smoketest ist nicht der Test** (DIR-012).
- [ ] **AKZ.5** — `./tools/watch-log.sh` läuft bei **jedem** Gerätelauf, **vor** jedem Einspielen
      gestartet und nach jeder Änderung an seinem Skript neu gestartet (DIR-013, L362).
      Erweiterung nach S1: Meldung bei `ds18xx verloren` und `DS18xxx temperature: ungueltig` —
      je mit Gegenprobe.
- [ ] **AKZ.6 (Abschluss, T)** — **Ein `pwa-tester`-Durchlauf Phasen 0–4 und 9** nach dem
      PWA-Release, mit dem nachgeführten `TESTPLAN-PWA.md` (T.1). Darin: **S25/S26 gefahren und
      über S27 zurückgestellt**, und nach einem **STM-Reset (Ent-4)** zeigt Phase 9 die
      LDR-Grenzen auf dem Wert vor dem Durchlauf — damit ist „rücknehmbar" am STM belegt, nicht
      nur an der ESP-Kopie. Unvollständig ⇒ **nicht abgenommen**.
- [ ] **AKZ.7** — Jedes ausgerollte Release **sofort** committet, getaggt, **gepusht** (DIR-011).
- [ ] **AKZ.8** — Die Einspielzeile nennt die **Reihenfolge**, fertig zum Einfügen.
- [ ] **AKZ.9** — Kodierung **und** Zeilenende jeder Datei vor dem Patch festgestellt (DIR-015):
      u. a. `src/base/base.c`, `src/main.c`, `src/ldr/ldr.c`, `src/ds18xx/ds18xx.c`,
      `src/tempsensor/tempsensor.c`, `src/onewire/onewire.c`, `src/vars/vars.c`,
      `src/esp8266/esp8266.c`, `http.cpp` (UTF-8), `app.js`.

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **Eigener Ersatz für `atoi`/`strtoul`** (rund 450 Byte, V) | Ändert das Verhalten bei Vorzeichen, Leerzeichen und Überlauf. **Nur Reserve**, falls S0 nicht kommt und der Platz trotzdem fehlt — dann eigene Entscheidung |
| **A52** (toter Zweig `FILE `, rund 25 Byte) | Nach S0 nicht nötig; bleibt eigener Befund. Nur bei Ent-5 ohne S0 als Reserve |
| **C42** | Spart nichts (V) und ist nicht beauftragt |
| **A57** (LDR-Minimum über Maximum zur Laufzeit) | Eigener Befund. AKL.2 lässt die verdrehte Reihenfolge bewusst zu; A57 wird weder behoben noch verschärft — **melden, nicht mitkorrigieren** |
| **Ein Zahlenfeld für die LDR-Grenzen in der PWA** | Bewusst kein Bedienelement (B12, `app.js:9953-9964`); A59 macht nur den API-Weg wirksam |
| **Negative DS18xx-Temperaturen** | Das Indexschema kennt nur 0..125 °C; ein negativer Messwert ergibt heute und künftig 255 und damit „Messwert ungültig". Eigener Befund, falls gewünscht (Muster: A5 für die RTC) |
| **Fault-Diagnose über den RAM oder eine Ausgabe** (`CFSR` usw.) | Es kam nie ein Zeichen an (V); A50 stellt nur den Reset her. Eine Diagnose wäre ein eigenes Vorhaben |
| **Eine Ankündigung des ESP vor einem geplanten Neustart** | Mit P3 nicht nötig; bräuchte ein ESP-Release vor S2 |
| **Inhaltsprüfung weiterer Textfelder** (Wetterort, Ticker, Update-Host) | Nur Datumsformat und Koordinaten sind beauftragt |
| **Eine Regelprüfung im STM** für Datumsformat oder Koordinaten | Die Regel sitzt beim ESP (Eingang) und in der PWA (vor dem Senden) |
| **Sanitizer der Release Notes in der PWA** (`app.js:7415-7433`) | Unverändert; Ent-2 betrifft nur die Legacy-Seite |
| **`eeprom_settings` ohne Klartext-Schlüssel** | Vorbestehend; B46 lädt den Endpunkt nur zusätzlich im Netzwerkmodul. **Melden** als Befund, falls der Review es empfiehlt |
| **A60/L360** (I²C-Treiber), **A61** (Vollabgleich nach Abweisung), **A56**, **A53** | Eigene Befunde |
| **Ein Fault am Gerät**, **ein DS18xx mit falscher Prüfsumme am Gerät** | Nicht herstellbar bzw. nicht freigegeben; Prüfstände FR und DS (Ent-14) |
| **Änderung an `specs/paket-2026-10-06/`** (AKS.6) | Momentaufnahme; die neue Fassung steht in AKP.4 |
| **Testplan-Phasen 5–8** | Bleiben beim Nutzer |

---

## Betroffene Laufzeiten

- [x] **STM32** (`src/**`) — S0: `src/base/base.c` (ggf. `base.h`, `src/timeserver/timeserver.c`);
      S1: `src/main.c`, `src/ds18xx/ds18xx.c`, `src/tempsensor/tempsensor.c`, ggf.
      `src/onewire/onewire.c`, `src/vars/vars.c`, `src/vars/vars.h`; S2: `src/main.c`,
      `src/esp8266/esp8266.c`; je ein Flash
- [x] **ESP8266** — E: `http.cpp`; ein OTA
- [x] **PWA** — P: `app.js`, `i18n/en.json`; `index.html`/`styles.css` nur, falls B47 Markup braucht
- [x] **Werkzeug** (`tools/**`) — Prüfstände GT, L59, DS, FR, EP, t15, t16, Regelgleichheit;
      Map-Prüfung AKG.2; statische Prüfungen AKF.1 und AKX.2; `watch-log.sh`
- [x] **Dokumentation** — `TESTPLAN-PWA.md`, `BEFUNDE.md`, `CLAUDE.md`, `tools/logger/README.md`
- [x] **Build/Release** — **fünf** Releases: STM (S0), STM (S1), STM (S2), ESP (E), PWA (P); je
      Commit, Tag, Push; dazu Testbauten F103 ohne Release

---

## Entscheidungen

Stand **10.10.2026, alle entschieden — vom Nutzer, alle nach Empfehlung.** Je Frage eine
Empfehlung; Begründung in `design.md`. Der Vermerk „**Entschieden**" unter jeder Frage hält die
Wahl fest.

- **Ent-1 — Freigabe und schreibende Gerätenachweise.** Freigabe der Spec samt: LDR-Bereinigung
  (G1), Wetterort-Testwert mit Rückstellung (G3, AKX.4), je ein gültiger Datumsformat- und
  Koordinatenwert mit Rückstellung (G3, AKR.4), `pwa-tester`-Durchlauf mit S25/S26 (T).
  **Empfehlung: freigeben.**
  **Entschieden 10.10.2026: freigegeben**, samt aller genannten schreibenden Gerätenachweise
  (G1, G3: AKX.4, AKR.4) und S25/S26 im Durchlauf (T).
- **Ent-2 — Release Notes auf der Legacy-Seite (C47).** R1 als Text maskieren (Tags sichtbar) ·
  **R2 Tags entfernen, Text und Zeilen behalten** · R3 Erlaubnisliste wie in der PWA · R4 roh
  lassen · R5 nicht mehr anzeigen, Hinweis auf die PWA. **Empfehlung: R2** — sicher ohne
  Erlaubnisliste in C, lesbar; R4 lässt die Lücke offen (der Update-Host ist aus dem LAN setzbar).
  **Entschieden 10.10.2026: R2** — Tags entfernen, Text behalten.
- **Ent-3 — Zielwert der LDR-Grenzen.** **0/4095** (Werkszustand, `ldr.c:21-29`; stand vor den
  Testläufen so, L351) · ein Wert des Nutzers · 16/18 (heutige ESP-Kopie) · 5/26 (heutiger
  STM-Stand). **Empfehlung: 0/4095.** Der bestätigte Wert steht im Bericht von G1.
  **Entschieden 10.10.2026: 0/4095** (Werkszustand). Das ist der vom Nutzer bestätigte Zielwert
  für AKL.3 und AKL.4.
- **Ent-4 — STM-Reset für den Persistenznachweis.** (a) Nach der Bereinigung in G1: **den Reset
  des S2-Flashs nutzen** oder einen eigenen Reset sofort; (b) nach dem `pwa-tester`-Durchlauf
  ein Reset für Phase 9 (AKZ.6). **Empfehlung: (a) S2-Flash, (b) ja.** Hält S2 am Flash-Gate
  an, braucht (a) einen eigenen Reset.
  **Entschieden 10.10.2026: (a)** Persistenznachweis über den Reset des S2-Flashs; **(b)**
  STM-Reset nach dem Durchlauf, danach Phase 9 erneut.
- **Ent-5 — Flash: Sparmassnahme `my_gmtime()` (S0).** **Ja, als eigenes Release vor S1** · ja,
  zusammen mit S1 · nein. **Empfehlung: ja, eigenes Release** — rund 950 Byte ohne
  Verhaltensänderung, aber in einer Funktion, die Uhrzeit, Datum und Wochentag trägt; ein Fehler
  muss sich eindeutig zuordnen lassen (R3b). **Bei Nein** muss die Reihenfolge kippen: A50 (spart
  180–250 Byte) **vor** A59 und A2, A52 als Reserve; reicht es dann noch nicht, entfällt A55.
  **Entschieden 10.10.2026: ja**, `my_gmtime()` auf 32 Bit als **eigenes Release S0** vor S1.
- **Ent-6 — Häufigkeit der DS18xx-Neuerkennung.** **(a) einmal je Minute im Sekunde-49-Zweig,
  solange kein Sensor erkannt ist, und nach 3 ungültigen Messungen in Folge auf „nicht
  erkannt"** · (b) wie (a), aber ohne die Regel „3 in Folge" (V-Mindestfassung) · (c) alle
  10 Minuten. **Empfehlung: (a)** — ein Versuch kostet 20–40 ms einmal je Minute; „3 in Folge"
  fängt auch einen Sensor, der **nach** dem Start ausfällt oder getauscht wird.
  **Entschieden 10.10.2026: (a)** — Neuerkennung je Minute, solange kein Sensor erkannt ist, und
  nach 3 ungültigen Messungen in Folge auf „nicht erkannt".
- **Ent-7 — Wartezeit im Fehlerpfad (A50).** F1 sofort `NVIC_SystemReset()` (Reset-Takt 3–4 s,
  ESP nie erreichbar, kein Web-Flash) · F1b bestehende Schleife auf über 12 s verlängern,
  ungeeicht, 0 Byte (Empfehlung V) · **F1c rund 15 s, geeicht über einen im HardFault
  weiterlaufenden Zähler**. **Empfehlung: F1c**, wenn der Review die Zählerquelle auf beiden
  Zielen bestätigt; sonst F1b mit gerechneter Dauer. In jeder Variante **ohne** Ausgabe.
  **Entschieden 10.10.2026: F1c** — rund 15 s geeicht warten, dann Reset; bestätigt das Review
  S2.5 die Zählerquelle **nicht auf beiden Zielen**, gilt **F1b**.
- **Ent-8 — Wie der STM einen ESP-Neustart erkennt (A55).** **P3: die Bootmeldung `CAP var-crc`
  setzt `is_online = 0`** (nur STM, rund 6 Byte, deckt auch Abstürze, `v` +0..1) · P2: die erste
  Zeitüberschreitung schaltet auf eine eigene Pause mit Höchstfrist (mehr Code, neuer Zustand).
  **Empfehlung: P3.**
  **Entschieden 10.10.2026: P3** — die `CAP`-Zeile setzt `is_online = 0`.
- **Ent-9 — Leeres Zeitfeld (B44).** **Immer abweisen** · nur bei aktivem Eintrag abweisen.
  **Empfehlung: immer** — ein inaktiver Eintrag mit stillem 00:00 wird beim späteren Aktivieren
  zum selben Fehler; zum Leeren gibt es „Leeren" (`clearTimerRow()`).
  **Entschieden 10.10.2026: immer abweisen.**
- **Ent-10 — Regel für das Datumsformat (C55).** **Erlaubt sind `d D m M y Y` und die Trennzeichen
  `.` `-` `/` und Leerzeichen; mindestens ein Platzhalter; jede Platzhalterart (Tag, Monat,
  Jahr) höchstens einmal; kein Leerzeichen am Anfang oder Ende.** Alternative: nur die
  Zeichenmenge ohne Wiederholungsregel (dann bliebe `yyyyy` gültig). **Empfehlung: die strenge
  Fassung.**
  **Entschieden 10.10.2026: die strenge Fassung** (`yyyyy` ist damit ungültig).
- **Ent-11 — Legacy-Anzeige des Fehlerwerts 255 (A2).** In Schritt E mitnehmen: „invalid" statt
  „127.5 °C" (`http.cpp:3919-3932`). **Empfehlung: ja** — dieselbe Datei wie C47, ohne
  Protokolländerung.
  **Entschieden 10.10.2026: ja** — der Fehlerwert 255 erscheint im ESP-Release (E) als „invalid".
- **Ent-12 — Regel für Koordinaten (C44).** **Dezimalzahl mit optionalem `-`, Ziffern, optional
  ein `.` (Komma wird vorher ersetzt) und Nachkommastellen; höchstens 8 Zeichen; Länge −180..180,
  Breite −90..90; kein `+`, kein Exponent, keine Leerzeichen.** **Empfehlung: so.**
  **Entschieden 10.10.2026: wie vorgeschlagen.**
- **Ent-13 — `SKIP ROM` statt `MATCH ROM` (A2).** Am Bus hängt genau ein Sensor (HARDWARE.md); mit
  `SKIP ROM` hinge keine Messung mehr an der gelesenen ID. **Empfehlung: nein** — mit der
  ROM-CRC und der Neuerkennung ist L18 behoben, ohne das Busprotokoll zu ändern; `SKIP ROM`
  verlöre die Fähigkeit, einen zweiten Sensor auszuschliessen.
  **Entschieden 10.10.2026: nein** — kein `SKIP ROM`, die ROM-ID wird per CRC geprüft.
- **Ent-14 — DS18xx am Gerät abziehen** als Gegenprobe der Neuerkennung, nur wenn steckbar.
  **Empfehlung: nein**, Prüfstand DS genügt.
  **Entschieden 10.10.2026: nein** — der Sensor wird nicht abgezogen, der Prüfstand DS genügt.
