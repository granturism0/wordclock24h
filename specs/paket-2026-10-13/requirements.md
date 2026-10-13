# Anforderungen — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (A15, A56, A70, A79, A80, A81, A82, B8, B48, B55, C6, C6b, C6v, C8, C19, C46, C59, C60, E36, F3)

**Status: freigegeben** — vom Nutzer am 13.10.2026 (Ent-16), samt Ent-1 bis Ent-14 und Ent-17 (unten,
„Entscheidungen"). **Eine Entscheidung gegen die Empfehlung:** Ent-11 (C6 einstufig). **Ent-15 (Messbau
für A15) ist am 13.10.2026 aufgehoben** (L432): A15 bleibt, ohne Messbau.
**Ent-11 und Ent-12 sind am 13.10.2026 aufgehoben** (Ent-18, L439, L440): Der Schutz der zerstörerischen
Aufrufe (C6, C6v, C60, dazu C61, C62) entfällt aus diesem Paket und wird in einer eigenen Spec geklärt.
Release E1 entfällt; C46 und C19 gehen mit E2; EP bleibt ein Release mit ESP und PWA, wegen C6b.
**Erstellt:** 2026-10-13, Stand `159c9d0` (Zweig `pwa-decoupling`); am selben Tag mit den Entscheidungen
nachgeführt. Ausgangsstand am Gerät laut Endstand von `specs/paket-2026-10-11/`: STM 3.2.32, ESP 3.2.33,
PWA 1.4.97.
**Auslöser:** Auftrag des Nutzers vom 13.10.2026 — alle offenen ToDos mit Kritikalität „mittel" und
alle, die seine Entscheidung brauchten, **ausser F4** (eigenes Folgepaket). Nachträglich aufgenommen:
**C60** (L426, hoch), **A81** (L427) und **A82** (L428). Nachträglich herausgenommen (Ent-18): C6, C6v, C60.
**Grundlage:** `BEFUNDE.md` (ToDo-Liste, nachgezählt am 13.10.2026; Befundzeilen L22, L35, L99, L101,
L311, L312, L330, L343, L345, L382, L396, L399, L406, L417, L421–L428, L432, L437–L440),
`REVIEW-2026-09-29.md` Massnahme 14. Der `spec-writer` hat jede tragende Fundstelle am Code `159c9d0`
nachgelesen; Abweichungen von `BEFUNDE.md` sind am Befund vermerkt.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** Zeilennummern gelten für `159c9d0`; sie sind Fundstellen, keine Anker.
Kennzeichen: **✔ SW** — vom `spec-writer` am Code nachgelesen; **✔ B** — in `BEFUNDE.md` als belegt
geführt (Nachzählung 13.10.2026 oder Gerätebeleg), nicht einzeln nachgelesen; **◐** — am Code
abgeleitet, nicht belegt; **●** — gemeldet, vor der Umsetzung zu prüfen.

**Kennungen.** Schritte: **0** (ohne Firmware), **M0/M1** (Messbauten für A15 — **entfallen**, Ent-15
aufgehoben), **S1** (STM-Release 1, isoliert: A15), **S2** (STM-Release 2: B55-STM, A56, A81, A80, A82,
A70), **S3** (nur falls das Flash-Gate in S2 reisst: A70, gegebenenfalls A82, nach Flash-Analyse;
Ent-14), ~~**E1**~~ (ESP-Release 1, HTTP-Schicht — **entfallen**, Ent-18; C46 und C19 gehen mit E2),
**E2** (ESP-Release, Brücke und Netz: B55-ESP, C8, A79, C59, dazu C46 und C19), **EP** (**ein** Release
mit ESP und PWA, direkt hintereinander eingespielt: C6b, dazu in der PWA C6b, C46, B55), **T1**
(`pwa-tester`), **P2** (PWA-Release: B48, B8), **T2** (`pwa-tester`), **Z** (Doku).
Gerätenachweise **G0** bis **G6** (**G3 entfällt**, geht in G4 auf). Akzeptanzkriterien: **AKW** (Logwache,
E36), **AKB** (Backup-Umfang, F3), **AKD** (deutsche Texte, B8), **AKI** (UART-ISR, A15), **AKL**
(LDR-Ergebnis, B55), **AKA** (Ambilight, A56, A81, A82), **AKC** (Icon, A80), **AKN** (Nachtfenster, A70),
~~**AKH**~~ (Herkunft und POST, C6, C6v, C60 — **entfallen**, Ent-18), **AKS** (Schlüssel, C6b), **AKQ**
(Query-Länge, C46), **AKU** (Update-Quelle Legacy, C19), **AKK** (Kommandopuffer, C8), **AKE**
(ESP-Empfangspuffer, A79), **AKT** (NTP-Bindung, C59), **AKF** (Fehlerzeile, B48), **AKM** (Flash und
RAM), **AKZ** (alle). Entscheidungen **Ent-1 bis Ent-18**. Die Kürzel gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Gerätenachweis |
|---|---|---|---|---|
| **0** | **E36** Logwache mit Lebenszeichen · **F3** Umfangsprüfung Backup · **B8** Zählwerkzeug und Bestandszählung · Werkzeug für den lokalen STM-Flash · Grundlinien · neue Befunde eintragen | Werkzeug, Doku | — | **G0** |
| ~~**M0**~~ | ~~Messbau „vorher"~~ — **entfallen** (Ent-15 aufgehoben, L432) | — | — | — |
| **S1** | **A15** UART-ISR | STM | nach G0 | **G1** |
| **S2** | **B55-STM** · **A56** · **A81** · **A80** · **A82** · **A70** (c), (d), (e), L399 | STM | nach G1 | **G2** |
| **S3** | nur bei Gate-Riss in S2 (Ent-14): Flash-Analyse, dann A70 (und A82, falls dort zurückgestellt) | STM | nach G2 | **G2b** |
| ~~**E1**~~ | ~~HTTP-Schicht: Herkunftsprüfung für alle zerstörenden Wege — C6 (Herkunftsteil), C6v, C60 · C46 (ESP) · C19~~ — **entfallen** (Ent-18, L439); C46 und C19 gehen mit E2 | — | — | ~~G3~~ |
| **E2** | Brücke und Netz: **B55-ESP** · **C8** · **A79** · **C59**, dazu **C46** (ESP) und **C19** (nur L311), beide schon im Baum | ESP | nach G2 bzw. G2b | **G4** |
| **EP** | **Ein** Release, ESP **und** PWA: ESP **C6b**; PWA: **C6b** (Export), **C46**, **B55**. Werkzeuge für C6b im selben Commit | ESP + PWA | nach G4; **ESP-OTA und PWA-Upload unmittelbar nacheinander**, ESP zuerst | **G5** |
| **T1** | `pwa-tester` Phasen 0–4 und 9 (DIR-012) | — | nach G5 | — |
| **P2** | **B48** Fehlerzeile am Feld · **B8** deutsche Texte in die i18n-Tabelle | PWA | nach T1 | **G6** (samt iPhone, Nutzer) |
| **T2** | `pwa-tester` Phasen 0–4 und 9 | — | nach G6 | — |
| **Z** | `BEFUNDE.md`, `TESTPLAN-PWA.md`, Spec-Endstand (~~`CLAUDE.md` Gefahrentabelle, POST~~ entfällt) | Doku | — | — |

**Warum diese Reihenfolge:** `design.md` §0. Kurz: `make release-zip` baut alle drei Komponenten
zusammen; deshalb entsteht der Code des nächsten Releases erst **nach dem Release-Build** des vorigen,
und jeder Release-Build erst **nach dem Gerätenachweis** des vorigen (R3b). **Firmware vor PWA**
(DIR-002). **A15 allein**, weil die ISR jeden Pfad der Brücke berührt. ~~**E1 und E2 getrennt**~~ — E1
entfällt (Ent-18); C46 und C19 sind klein, am Prüfstand belegt und haben eigene Kennzeichen, sie gehen mit
E2. **EP einstufig wegen C6b** (Ent-18, `design.md` §8.4): ESP und PWA sind **ein** Release, gebaut in
**einem** `make release-zip`, und werden ohne Gerätenachweis und ohne anderes Release dazwischen
eingespielt — ESP zuerst, Minuten später die PWA. Sonst exportierte die alte PWA leere WLAN-Schlüssel.
*Stand der Freigabe:* „C6 einstufig (Ent-11): Die POST-Umstellung von ESP und PWA ist ein Release."

---

## Problem

### 1. A15 / L101 — die UART-ISR liest `SR` doppelt und schreibt es zweimal

**✔ SW:** `src/uart/uart-driver.h:773` und `:775` (`USART_GetITStatus`/`USART_ClearITPendingBit` für
RXNE), `:804` (`USART_ReceiveData`), `:833` und `:837` (dasselbe für TXE); ORE wird schon direkt aus
`SR` gelesen (`:796`). Die ISR ist eine Vorlage, die **drei** Instanzen erzeugt (ESP-Brücke,
Log-UART, DFPlayer, `uart-driver.h:733-737`).

**Berichtigung (L432, 13.10.2026):** Die Prämisse aus L101 — vier SPL-Aufrufe je Zeichen, 100 bis 130
Befehle, Gewinn „Faktor drei bis vier" — **gilt nicht mehr**. Seit LTO (STM 3.2.16) sind die SPL-Aufrufe
eingebettet; im Bau 3.2.32 hat keine ISR auf F103 oder F411 ein `bl` (90–95 Befehlszeilen je Instanz).
Zu gewinnen sind **wenige Befehle**: zwei Schreibzugriffe auf `SR` (`ClearITPendingBit`) und das doppelte
Lesen von `SR`. A15 bleibt im Umfang (Entscheidung des Nutzers), der Messbau entfällt.

**Was ein Ersatz einhalten muss, ✔ SW:** `USART_GetITStatus()` prüft Flag **und** Freigabebit. TXE ist
gesetzt, sobald das Senderegister leer ist, die Freigabe TXEIE nur während des Sendens (`:854`). Ein
Ersatz, der nur `SR & TXE` prüft, sendete bei jedem Empfangsinterrupt. **◐:** `USART_ClearITPendingBit`
für TXE ist wirkungslos, weil TXE nur lesbar ist (SPL-Vertrag, nicht am Projektcode nachgelesen).

**Weg für einen Messbau — nur noch als Werkzeug, nicht eingesetzt, ✔ SW:** `tools/flash-stm.sh` flasht
sonst **nur** vom Update-Server (`:117`). Der ESP kennt einen **lokalen** Weg: `/api/local_stm32_upload`
(POST, `http.cpp:13249-13307`, Dateiname muss `stm32_default` entsprechen, `:13261`) legt die Datei als
`stm32.hex` ins LittleFS, `/api/local_stm32_flash` (`:13310-13324`) flasht sie. `flash-stm.sh --lokal`
ist gebaut und bleibt als Werkzeug; in diesem Paket wird es nicht gebraucht.

### 2. A56 / L330 — Ambilight-Power; A81 / L427 und A82 / L428

**✔ SW:** `display_init()` setzt `ambilight_power_is_on = 1` (`src/display/display.c:6520`); einen
EEPROM-Platz gibt es nicht (`src/eeprom/eeprom-data.h`, Layout bis `EEPROM_DATA_END` = 1979,
Layoutversion 2.9). Geschaltet wird an genau einer Stelle, `display_set_ambilight_power()`
(`display.c:2907-2919`), aufgerufen aus `set_display_power()` (`src/main.c:1447`), dem Setter
`DISPLAY_AMBILIGHT_POWER_NUM_VAR` (`main.c:1785-1792`) und dem Ambilight-Timer (`main.c:4458-4460`).

**Migration, ✔ SW:** Gelesen wird die Konfiguration nur bei `eep_version <= EEPROM_VERSION`
(`main.c:1224-1225`). Hebt ein Release die Layoutversion an, liest jede **ältere** Firmware danach keine
Einstellung mehr und schreibt den Werkszustand (`upgrade_eep_version()`, `main.c:1300-1418`). Deshalb
Ent-9: zwei Byte hinter dem Layoutende, ohne Layoutversion.

**A81 (L427), ✔ SW:** `display_read_config_from_eep()` liest die Weissfarbe der Ambilight-Markierung vom
Offset der **Ambilight**-Weissfarbe (`display.c:3435`); geschrieben wird sie an ihren eigenen Offset
(`:3748`). Eine Konstante.

**A82 (L428), ✔ SW:** `dcf77_enabled` wird nur in `set_display_power()` neu bestimmt
(`main.c:1471-1478`); der Ambilight-Setter (`:1789`) und der Ambilight-Timer (`:4460`) lassen es
stehen. DCF77 empfängt nur bei Display **und** Ambilight aus (`main.c:891`). Folge am Gerät nicht belegt.

### 3. A70 / L396 (c, d, e) und L399 — Randfälle beim Nachholen des Nachtfensters

**✔ SW:**
- **(c)** `NIGHT_OFF_AFTER_IP` (`main.c:1423`) wird beim Start des IP-Lauftexts gesetzt
  (`:4213-4216`) und nur im Innenzweig der Restore-Bedingung eingelöst (`:4868-4871`); die Frist
  (`:4219`) erreicht diesen Wert nie. Kommt der Restore nicht, fehlt das Aus die ganze Nacht.
- **(d)** Das erste `TIME` bei ungültiger RTC ruft `night_apply (0xFFFF, 1)` (`:3431-3434`) und schaltet
  sofort — auch mitten im IP-Lauftext.
- **(e)** Andere Ticker (Wetter `:3520`, Vorhersage `:3527`, Datum `:4853` und weitere) setzen
  `pending_ticker_restore`; das vorgemerkte Aus wartet auf das Ende des fremden Tickers.
- **L399** Der DCF77-Zweig (`:4341-4372`) setzt `night_wm` nie; ohne gültige RTC und ohne `TIME` greift
  weder die Startauswertung noch die Sprungerkennung (`:4437-4442`).

**✔ SW:** Der DCF77-Empfang läuft nur, solange Display **und** Ambilight aus sind (`main.c:891`,
`:1471-1478`). Der L399-Fall tritt also erst ein, nachdem das Display aus ist; die Korrektur bewirkt vor
allem, dass danach die Sprungerkennung greift.

### 4. A80 / L422 — ein Icon bei ausgeschaltetem Display bleibt vorgemerkt

**✔ SW:** Der Zweig `ESP8266_ICONDATA` setzt `do_display_icon = 1` und `show_icon_stop_time`
(`main.c:3551-3558`), ohne auf die Display-Power zu sehen. `display_icon()` tut bei ausgeschaltetem
Display nichts (`display.c:5087`) und setzt `do_display_icon` erst nach Ablauf zurück (`:5229-5234`).
Die Restore-Bedingung (`main.c:4857-4861`) wartet auf `! display.do_display_icon`. **✔ B (L422):**
48 Minuten am Gerät. **◐:** Dasselbe tritt ein, wenn das Display **während** eines Icons ausgeht.

### 5. B55 / L417 — „Messwert übernehmen" meldet „gespeichert", auch wenn der STM abweist

**✔ SW:** `http_api_ldr_min_set()` und `http_api_ldr_max_set()` (`ESP8266/ESP-uclock/http.cpp:10151-10160`,
`:10224-10233`) stossen den RPC an und antworten sofort `{"ok":true}`. Der STM führt ihn aus
(`main.c:1620-1634`), `ldr_set_min_value()` weist ab (`src/ldr/ldr.c:194-197`) und meldet über
`var_send_ldr_min_value()` den **alten** Wert.

**Daraus folgt, ✔ SW:** Am unveränderten Wert allein ist eine Abweisung nicht erkennbar — unverändert
heisst auch „Messung fehlgeschlagen" (`ldr.c:177-188`) oder „derselbe Wert gemessen" (zweimal drücken
bei gleichem Licht). Es braucht eine **Rückmeldung des STM**. Zweitens: `var_send_*` wartet auf die
Quittung des ESP; ein HTTP-Handler, der wartet, ohne die Brücke zu lesen, liesse den STM in seine
3-s-Frist laufen. Gelesen wird die Brücke heute nur am Ende von `loop()` (`ESP-uclock.ino:1440-2255`).

### 6. C8 / L99 — lange Logzeilen werden gekappt

**✔ SW:** Der STM sendet `LOG ` und bis 255 Zeichen (`src/log/log.c:20`, `:35-45`;
`src/esp8266/esp8266.c:871-907`). Der ESP nimmt 127 in den Kommandopuffer (`CMD_BUFFER_SIZE 128`,
`ESP-uclock.ino:71`, Kappung mit `~` `:1446-1449`, `:2245-2252`), der Logring hält **120** je Zeile
(`:82`, `stm32_log_append()` `:93-114`). Ein Kommandopuffer allein ändert in `/api/stm32_log` nichts.
Für 255 Zeichen samt `LOG ` braucht der Puffer **260** Byte. Ent-10: 260 Byte und Folgeeinträge im Ring.

### 7. C6b / L22 — `/api/eeprom_settings` gibt den WLAN-Schlüssel im Klartext heraus

**✔ SW:** `http_api_eeprom_settings()` (`http.cpp:9311-9331`) sendet `key` und `ap_key`. Einziger Leser in
der PWA ist der Backup-Export (`app.js:4914-4917`, `buildNetworkBackupSettings()` `:4166-4178`);
`ensureBackupExportState()` (`:4993-5009`) bricht ab, wenn `eeprom_settings` fehlt. Der Import schreibt
den Schlüssel zurück, `eeprom_settings_set` übergeht leere Felder (`http.cpp:9379-9395`). ⇒ Ein leerer
Schlüssel im Export ginge erst beim Wiederherstellen auf ein **frisches** Gerät verloren, still. Weitere
Leser: `tools/snapshot-device.sh:154` (hasht ihn), `tools/preview/server.py:83`.

**Ergänzt 13.10.2026, ✔ SW:** Der ESP sendet keine CORS-Kopfzeile (`Access-Control-` kommt in
`ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` nicht vor); eine fremde Seite kann die Antwort im Browser nicht
lesen. Lesen kann den Schlüssel, wer im LAN direkt anfragt, und alles, was die Antwort mitprotokolliert.

### 8. C6 / R2-14, C6v / L312 und C60 / L426 — zerstörende Wege

**Stand 13.10.2026 (Ent-18): nicht mehr Teil dieses Pakets.** Der Befund bleibt als Stand der Freigabe
stehen. **Ergänzt (L439, am Gerät belegt, Chrome 155):** Browser senden `Sec-Fetch-Dest` und
`Sec-Fetch-Site` über `http://` an die Uhr nicht — die bestehende Prüfung (ESP 3.2.4) und die geplante
Erweiterung wirken im Browser nicht. Hinzugekommen sind **C61** (L438: Upload-Wege per POST und
`local_esp_restart` ohne Prüfung) und **C62** (L440: WLAN- und AP-Setter ohne Prüfung). Alle fünf klärt eine
eigene Spec; belegt ist für sie, dass Chrome bei POST `Origin` sendet (L439).

**✔ SW (Stand der Freigabe):**
- Die Herkunftsprüfung (`http_request_is_embedded_subresource()`, `http.cpp:476-499`) steht nur in
  `format_fs`, `reset_stm32`, `reset_eeprom` (`:9708-9760`), **nicht** in `fs_remove` (`:12349`) und
  nicht im Legacy-Zweig `/fs?action=format` (`:6718-6722`) und `action=remove` (`:6765`).
- **C60 (L426, hoch):** Die Legacy-Hauptseite löst mit `action=rsteeprom2` den EEPROM-Reset per GET aus,
  ohne jede Prüfung (`http.cpp:3464-3469`), ebenso `action=rststm32` den STM-Reset (`:3455-3459`,
  ausgeführt `:3565-3573`). Weitere Legacy-Wege zu `stm32_reset()` stehen bei `:7542`/`:7634` und
  `:13612`/`:13723` — Bestandsliste per `grep` im Task.
- Der POST-Zweig (`:14628-14659`) liest **keine** Kopfzeilen und kennt nur Upload-Pfade (`http_post()`,
  `:14374-14424`); ein POST auf einen anderen Pfad bekommt **keine Antwort**.
- PWA: alle Aufrufe über `apiFetch()` (`app.js:2533-2576`), das Optionen durchreicht; Aufrufer der
  Wartungsendpunkte `:5393`, `:11787`, `:11851`, `:11855`, `:11875`, `:12412`; `fs_remove` über
  `fs_remove_base_url` (`:10602`). Der Service Worker lässt Nicht-GET durch (`sw.js:121`).
- Werkzeuge mit GET: `tools/flash-stm.sh:131` (Reset nach dem Flashen), `tools/smoke-device.sh:229`.
- POST allein schützt nicht vor fremden Seiten — ein Formular-POST von fremder Herkunft ist ein
  „einfacher" Request ohne Vorabprüfung. Wirksam ist die Herkunft (`Sec-Fetch-Site`). *(Für http widerlegt:
  L439.)*

### 9. C19 / L311 — Legacy-Zwillinge der Update-Quelle prüfen nichts

**✔ SW, weicht von `BEFUNDE.md` ab:** zwei Stellen — Dateisystemseite (`http.cpp:6724-6731`) und
Update-Seite (`:7520-7527`). Beide setzen über `set_strvar()` ungeprüft, leer und überlang
eingeschlossen. Leer ⇒ Rückfall auf den Server des Ursprungsprojekts (L42, DIR-009).

### 10. C46 / L345 — Query über 255 Zeichen still gekürzt

**✔ SW:** `http()` kopiert mit `strncpy (param, const_param, 255)` (`http.cpp:13795-13798`), der
POST-Zweig ebenso (`:14643-14649`). Die Anfragezeile wird ungekürzt gelesen (`:760-786`). Belegt sind die
Fehlerkennungen 1 bis 8 (`:244-273`). **Rechnung:** Mit SSID 31 Byte und Schlüssel 63 Byte, jedes Byte
prozentkodiert, ist `ssid=…&key=…` über 300 Zeichen lang. Wer abweist, statt zu kürzen, bricht einen
solchen Backup-Import — die PWA muss **aufteilen**.

### 11. C59 / L406 — NTP-Antwort an keine Anfrage gebunden

**✔ SW:** `ntp_poll_time()` prüft Absender, Port, Länge, LI, Stratum und Transmit (`ntp.cpp:156-202`).
Kein Merker „Anfrage offen", das Transmit-Feld der Anfrage ist leer (`:117-136`), das Originate-Feld der
Antwort (Byte 24–31) wird nicht geprüft.

### 12. A79 / L421 — ESP-Empfangspuffer läuft beim Start einmal über

**✔ SW:** `Serial.begin(115200)` (`ESP-uclock.ino:160`), kein `setRxBufferSize`; die Vorgabe von 256 Byte
nennt nur ein Kommentar (`:474`). **✔ B:** `- ovr #1 t=5561` am 13.10.2026.

### 13. B48 / L382 — Fehlermeldungen auf dem iPhone von der Tastatur verdeckt

**✔ SW:** `reportInvalidInput()` (`app.js:6614-6622`) meldet über das Statusbanner und setzt danach den
Fokus. **✔ B:** kein `aria-invalid`, kein `aria-describedby`. Die Wirkung ist nur am iPhone prüfbar.

### 14. B8 — hartcodierte deutsche Texte

**●** Die Zahl ist nicht gezählt. „~176" ist überholt; die Nachzählung vom 13.10.2026 fand 15
Satzliterale, erfasste aber weder Texte ohne typische Satzwörter noch das Markup (✔ B). DE-Tabelle in
`app.js` (`I18N_DE`, `:20-1026`), EN in `data/app/i18n/en.json`; `index.html` trägt 309 Attribute
`data-i18n` (✔ SW, Anzahl).

### 15. F3 / L35 (d) — keine Prüfung, ob das Backup alle Setter abdeckt

**✔ B:** Es gibt keine. **✔ SW:** Der Dispatch in `http.cpp` hat 59 Zeilen der Form
`strcmp (path, "/api/…_set")`.

### 16. E36 / L423 — die Logwache stand still, ohne es zu melden

**✔ B:** 12.10.2026, 00:20 bis 01:33, Prozess lebte, keine Ausgabe. **✔ SW:** `tools/watch-log.sh` ruft
`tools/logger/log.sh tail` ohne Frist (`:74-76`, `:88` und je Runde) und schläft mit `sleep "$INTERVAL"`
(`:377`); keine Erkennung eines eigenen Aussetzers, kein Lebenszeichen nach aussen. **●** Ursache nicht
untersucht.

### 17. Budgets

- **Flash F103 (✔ B):** Teilbuild 1'656 Byte frei (S8b, 13.10.2026), engste Matrix-Variante
  `wc24h-stm32f103-sk6812-rgbw` 1'436 (L416). **Spielraum über dem Gate von 1'024: 412 Byte.**
- **ESP-RAM (✔ B, L421):** Heap frei 9'472 Byte, grösster Block 8'736 Byte.

---

## Ziel

1. **Logwache:** Eine stillstehende Wache meldet sich selbst — beim Weiterlaufen und von aussen prüfbar.
2. **Backup-Umfang:** Jeder `*_set`-Endpunkt steht im Backup oder mit Grund in einer Ausnahmeliste.
3. **UART-ISR:** `SR` einmal lesen, kein Schreiben auf `SR`; die Befehlszahl im Empfangs-Normalpfad sinkt
   auf F103 und F411 (statisch gemessen); gleiches Verhalten nachgewiesen; am Gerät keine Regression.
4. **Ambilight:** Die Power übersteht einen Neustart, ein Rückflash kostet keine Einstellung; die
   Marker-Weissfarbe kommt vom richtigen Platz; DCF77 folgt auch dem Ambilight.
5. **Nachtfenster:** (c), (d), (e), L399 nach Ent-2.
6. **Icon:** Bei ausgeschaltetem Display verworfen; nichts wartet mehr darauf.
7. **LDR-Übernahme:** „gespeichert" nur, wenn der STM übernommen hat; sonst eigene Meldung.
8. ~~**Zerstörende Wege:** jeder — API und Legacy — nur von gleicher Herkunft; die fünf API-Endpunkte der
   PWA nur noch per POST.~~ — **Entfallen** (Ent-18): eigene Spec.
9. **Schlüssel:** nicht mehr aus `/api/eeprom_settings`; das Backup enthält ihn weiterhin.
10. **Query-Länge:** über 255 abgewiesen statt gekürzt; die PWA sendet nichts, was der ESP abweisen müsste.
11. **Legacy-Update-Quelle:** leer und überlang abgewiesen.
12. **Brücke:** lange Logzeilen kommen vollständig an; der ESP-Empfangspuffer läuft beim Start nicht über.
13. **NTP:** nur noch eine Antwort auf die eigene, offene Anfrage.
14. **PWA:** Fehlerzeile am Feld; keine deutschen Texte ausserhalb der i18n-Tabellen.
15. **Nachweis:** jede Wirkung einzeln, jede neue Prüfung einmal fehlgeschlagen (DIR-014), jede Messung
    **bei der Messung** in `BEFUNDE.md` (L424).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt** (DIR-014).
Prüfstände liegen unter `tools/checks/auszug/`, laufen über `tools/checks/auszug.sh` mit den Typbreiten
des Ziels (L256) und sind **einmal gegen den Vorgängerstand fehlgeschlagen**; die Fallzahl wird gemeldet
und nachgezählt. **Was am Gerät nicht herstellbar oder dort gefährlich ist, wird nicht am Gerät
verlangt** — die Liste dessen, was **nie** am Gerät gefahren wird, steht in `design.md` §17.3.
Logzeilen im STM in Umschrift (DIR-015); der Wortlaut ist verbindlich, wo er hier steht.

**Messwerte sofort festhalten (L424, Skill `/release`):** Jede Messung, die ein Kriterium belegt —
Flash je Task, Matrix-Lauf, RAM, `--full`, Smoketest, Gerätewert, Befehlszahl —, steht **in dem Moment, in
dem sie entsteht,** im Messprotokoll in `BEFUNDE.md` (`design.md` §1.4). Ein Kriterium ohne diesen Beleg
gilt als **nicht** erfüllt.

**Wach bleiben (DIR-020):** Jeder Gerätetest, der auf später geplant ist (Nachtteil G2, Stundenläufe),
läuft mit `caffeinate -i -s -t <Sekunden>` des Leads, gestartet beim Planen; auf Akku sagt der Lead es
dem Nutzer.

### Schritt 0 — ohne Firmware

- [ ] **AKW.1 (E36, Ursache)** — Der Lead wertet `pmset -g log` für den 12.10.2026, 00:00–01:40, gegen
      das Stillstandsfenster aus und trägt das Ergebnis als „belegt" oder „nicht belegt" in L423 ein
      (DIR-003).
- [ ] **AKW.2 (Lebenszeichen)** — Die Wache schreibt je Runde eine Herzschlagdatei mit Zeitstempel,
      Intervall und Rundenzahl. `./tools/watch-log.sh --pruefe` gibt Exit 1 und „Wache steht seit N s"
      aus, wenn der Zeitstempel älter als 3 × Intervall ist oder die Datei fehlt, sonst Exit 0. Der
      Stop-Hook ruft `--pruefe` auf, wenn die Gerätemarke `geraet-gemessen` gesetzt ist.
- [ ] **AKW.3 (Selbstmeldung)** — Liegen zwischen zwei Runden mehr als 3 × Intervall Wanduhrzeit, meldet
      die Wache beim Weiterlaufen `*** Wache hat N s ausgesetzt ***` als Alarm, in Konsole **und**
      Mitschrift.
- [ ] **AKW.4 (Frist)** — Jeder Aufruf von `log.sh` hat eine Frist von 20 s; läuft sie ab, Alarm
      `*** Mitschnitt nicht erreichbar ***`, die Runde läuft weiter.
- [ ] **AKW.5 (Gegenproben)** — (i) `WATCHLOG_QUELLE`, die 200 s schläft ⇒ AKW.4 kommt an;
      (ii) Herzschlag künstlich 200 s alt ⇒ `--pruefe` Exit 1; (iii) Wache 100 s mit `SIGSTOP` angehalten
      bei Intervall 30, dann `SIGCONT` ⇒ AKW.3 kommt an. Je Probe Meldung in Konsole und Mitschrift,
      Exit-Code genannt.
- [ ] **AKB.1 (F3)** — Eine Guardrail-Stufe stellt die `*_set`-Endpunkte aus dem Dispatch von `http.cpp`
      gegen die Setter, die der Backup-Import der PWA aufruft, und meldet jeden Endpunkt, der weder dort
      noch in der Ausnahmeliste steht, mit Namen und **Exit ≠ 0**. Sie gibt die Zahl der betrachteten
      Endpunkte aus; **zähl nach** gegen `grep -c` über das Dispatch-Muster (Stand `159c9d0`: 59).
- [ ] **AKB.2 (Ausnahmeliste)** — eigene Datei unter `tools/checks/`, je Zeile Endpunkt und Grund. Ein
      Eintrag ohne Grund oder für einen Endpunkt, den es nicht gibt ⇒ FEHL.
- [ ] **AKB.3 (Gegenprobe)** — In einer Kopie von `app.js` einen Setter aus dem Import entfernt ⇒ FEHL mit
      Endpunktnamen; ein erfundener Eintrag in der Ausnahmeliste ⇒ FEHL.
- [ ] **AKD.1 (B8, Zählung)** — Ein Werkzeug listet mit Datei:Zeile alle Kandidaten für sichtbaren Text
      (Literale in `app.js` ausserhalb der i18n-Tabellen; Textknoten und `title`, `aria-label`,
      `placeholder`, `alt` in `index.html` ohne `data-i18n*`). Der Lead ordnet einmal zu: **deutscher
      UI-Text** oder **Ausnahme mit Grund**. Die Zahl steht sofort bei B8. *Gegenprobe:* neuer Text ⇒
      Exit ≠ 0.
- [ ] **AKG.0 (Grundlinien G0)** — vor S1, je sofort ins Messprotokoll: (1) Befehle im
      Empfangs-Normalpfad laut Disassembly, F103 **und** F411, aus dem Worktree-Bau des Tags
      `release/3.2.32-3.2.33-1.4.97`; (2) Setter-Burst 14 × Wetterort mit Rückstellung: `rx`-Höchststand,
      `d`, `o`, Δ`v`; (3) 30 Heap-Zeilen: kleinstes `free`, kleinstes `max`, `ovr`; (4) längste Zeile in
      `/api/stm32_log`, ob sie `~` trägt.
- [ ] **AKG.1 (Befunde)** — `BEFUNDE.md` führt an jedem Eintrag dieses Pakets „Träger: Paket 2026-10-13";
      die neuen Befunde aus dieser Spec, die noch keine Zeile haben (zweite C19-Stelle, 120er-Ring,
      Rechnung Netzwerkimport), stehen als eigene Zeilen. *Instrument:* S10 und S11 grün.
- [ ] **AKZ.12 (Werkzeug lokaler Flash)** — `tools/flash-stm.sh --lokal <hex>` lädt eine Datei über
      `/api/local_stm32_upload` hoch (Dateiname gleich `stm32_default`), flasht über
      `/api/local_stm32_flash`, setzt zurück und weist die gemeldete Version nach; vorher prüft es den
      freien Platz im LittleFS (`/api/fs_info`) gegen die Dateigrösse. *Gegenprobe:* falscher Dateiname ⇒
      Abbruch mit Meldung, Exit ≠ 0, ohne Flash. Gebaut; bleibt als Werkzeug, wird in diesem Paket nicht
      eingesetzt (Ent-15 aufgehoben).

### Schritt S1 — A15 (UART-ISR)

- [ ] **AKI.1 (Gleiches Verhalten)** — Prüfstand **a15** betreibt die alte und die neue ISR gegen dieselbe
      Registerattrappe (`SR`, `CR1`, `DR` mit Lesesemantik: `DR` lesen löscht RXNE und ORE; TXE nur
      lesbar; TXEIE in `CR1`) über mindestens 10'000 Zufallsfolgen und die Randfälle „Ring voll", „ORE
      gesetzt", „TXE gesetzt, TXEIE aus", „RXNE und TXE zugleich", „Sendering leer". Verglichen werden
      Empfangsring, `uart_rxdrops`, `uart_rxore`, `uart_rxmax`, gesendete Zeichen und TXEIE: **0
      Abweichungen**. Sabotagen „TXEIE nicht geprüft" und „ORE nach dem Lesen von DR geprüft" ⇒ je FEHL.
- [ ] **AKI.2 (statisch, beide Plattformen)** — Die Befehlszahl im Empfangs-Normalpfad (RXNE, kein ORE,
      Ring nicht voll, kein TXE) **sinkt** auf F103 **und** F411 gegenüber dem Vorher-Wert aus dem
      Worktree-Bau des Tags (AKG.0); `SR` wird je Interrupt einmal gelesen und nicht geschrieben. „Kein
      `bl`" ist schon an der Grundlinie erfüllt (L432) und kein Kriterium mehr. Beide Zahlenpaare sofort ins
      Messprotokoll.
- [x] ~~**AKI.4 (Messbau, Ent-15)**~~ — **Aufgehoben 13.10.2026** (Ent-15 aufgehoben, L432): kein Messbau
      M0/M1. Der Messcode-Patch `tools/checks/auszug/stm/a15/messbau.patch` bleibt liegen, wird aber nicht
      eingesetzt. Kein Kriterium.
- [ ] **AKI.3 (Gerät G1, Regressionslauf)** — Nach S1 derselbe Setter-Burst wie in G0, danach
      zurückgestellt: `d=0`, `o=0`, Δ`v` = 0, `rx`-Höchststand neben dem Wert aus G0. Danach 60 min lesend:
      kein Neustart, keine Exception, kein Anstieg von `o`; der Mitschnitt läuft lückenlos.

### Schritt S2 — B55-STM, A56, A81, A80, A82, A70

- [ ] **AKL.1 (B55, STM)** — Nach `LDR_MIN_VALUE_RPC_VAR` bzw. `LDR_MAX_VALUE_RPC_VAR` sendet der STM
      **nach** der Wertmeldung genau eine Zeile `RPCR xx r` (xx = RPC-Index zweistellig hex; r = `0`
      übernommen, `1` abgewiesen, `2` Messung fehlgeschlagen), ohne Quittung (`design.md` §6.1). Prüfstand
      **l55**: alle drei Ergebnisse für Minimum und Maximum, dazu „gleicher Wert gemessen" ⇒ `0`.
      Gegenprobe gegen 3.2.32 ⇒ FEHL.
- [ ] **AKA.1 (A56)** — Zwei Byte (Marke 0xA5, Wert 0/1) hinter dem bisherigen Layoutende, **ohne**
      Layoutversion (Ent-9, `design.md` §3). Jeder Wechsel über `display_set_ambilight_power()` schreibt; ein
      unveränderter Wert schreibt nicht. Beim Start gilt der gespeicherte Wert; ohne gültige Marke oder mit
      einem anderen Wert als 0/1 gilt „an". Nach einem EEPROM-Reset gilt „an". Prüfstand **a56** (Lesen,
      Schreiben, Marke fehlt, Wert 2, Reset, simulierter Rückflash: übriges Abbild byte-gleich). Gegenprobe
      gegen 3.2.32 ⇒ FEHL. Bestandsliste aller Schaltstellen per `grep`.
- [ ] **AKA.2 (A56, Last)** — Kein EEPROM-Schreiben aus einer ISR oder einem Animationspfad; je Wechsel
      höchstens ein Seitenschreiben (`eep a=… n=…` im Mitschnitt, rund 15 ms).
- [ ] **AKA.4 (A81)** — `display_read_config_from_eep()` liest die Marker-Weissfarbe von
      `EEPROM_DATA_OFFSET_AMBI_MARKER_W_COLOR`. Prüfstand **a81**: geschriebener Marker-Weisswert kommt nach
      dem Laden zurück, unabhängig vom Ambilight-Weisswert; Gegenprobe gegen 3.2.32 ⇒ FEHL.
- [ ] **AKA.5 (A82)** — Nach **jedem** Wechsel von Display- oder Ambilight-Power gilt
      `dcf77_enabled == (! display && ! ambilight)`, über **eine** Hilfsfunktion statt dreier Kopien.
      Prüfstand **a82** über alle Schaltstellen (Setter, Timer, `set_display_power()`); Gegenprobe ⇒ FEHL.
- [ ] **AKC.1 (A80)** — Kommt ein Icon bei ausgeschaltetem Display vollständig an, bleibt
      `do_display_icon` 0, `show_icon_stop_time` unverändert, und es erscheint `icon: verworfen, display
      aus`. Geht das Display während eines Icons aus, endet das Icon sofort, und das nächste beginnt von
      vorn. Prüfstand **a80**; in keinem Fall `icon_freeze: ENTER`. Gegenprobe gegen 3.2.32 ⇒ FEHL.
- [ ] **AKN.1 (A70 (c))** — Startet der IP-Lauftext mit vorgemerktem Aus, gilt zusätzlich eine Frist:
      Laufzeit des Lauftexts aus Länge, `TICKER_COLS` und `ticker_deceleration` plus 10 s (`design.md`
      §4.1). Kommt der Restore nicht, schaltet die Frist aus.
- [ ] **AKN.2 (A70 (d))** — Kommt das erste `TIME` bei ungültiger RTC, während der IP-Lauftext aussteht
      oder läuft, wird das Aus vorgemerkt wie beim Start; sonst wird sofort geschaltet wie heute.
- [ ] **AKN.3 (A70 (e))** — Startet ein anderer Ticker, während das Aus auf das Ende des IP-Lauftexts
      wartet, wird das Aus **vor** dem Start des Tickers angewandt; der Ticker erscheint dann nicht.
      Bestandsliste aller Tickerstarts per `grep display_set_ticker`.
- [ ] **AKN.4 (L399)** — Übernimmt der DCF77-Zweig die erste Zeit bei `night_wm == 0xFFFF`, wird wie beim
      ersten `TIME` ausgewertet (`night_apply (0xFFFF, 1)`, nur ein Aus); danach greift die Sprungerkennung.
- [ ] **AKN.5 (Invariante)** — Die Restore-Bedingung um `pending_ticker_restore` (`main.c:4857-4861`,
      vier Teilbedingungen, dazu `! display_clock_flag`) ist unverändert; `git diff` zeigt in diesen Zeilen
      keine Änderung, der Innenzweig nur den Austausch des Vergleichs gegen den Merker aus AKN.1. Prüfstand
      **a70** für (c), (d), (e), L399, je Gegenprobe gegen 3.2.32 FEHL; **a69** weiter OK.
- [ ] **AKA.3 (Gerät G2, A56)** — Ambilight-Power aus über die PWA, `/api/power_status` zeigt aus, im
      Mitschnitt das Seitenschreiben; STM-Reset über `maintenance_reset_stm32`; nach `info: configuration
      sent` meldet `/api/power_status` Ambilight **aus**. Danach zurück auf den Vorher-Wert, lesend geprüft.
- [ ] **AKC.2 (Gerät G2, A80)** — Display aus über die PWA, dann `/api/weather_get_now`: im Mitschnitt
      `icon: verworfen, display aus`, kein `icon_freeze: ENTER`, kein Lauftext. Display wieder an, `diag`
      lückenlos.
- [ ] **AKN.6 (Gerät G2, Nacht)** — Mit `caffeinate` (DIR-020): Die Timer der Nacht schalten wie bisher
      (`Found Timer:` am Abend und am Morgen). **A70 (e) am Gerät (freigegeben):** STM-Reset im
      Aus-Fenster, sobald der IP-Lauftext läuft `/api/weather_get_now` ⇒ im Mitschnitt das Aus
      (`set_display_power: new=0 …`) **vor** dem Wetterlauftext, kein Wetterlauftext am Display.

### ~~Schritt E1 — HTTP-Schicht (Herkunft für alle zerstörenden Wege, C46, C19)~~ — entfallen (Ent-18)

AKH.1–AKH.4 entfallen; ihr Text bleibt als Stand der Freigabe. **AKQ.1** und **AKU.1** gehen mit E2 (unten),
**AKQ.2** in G4.

- [x] ~~**AKH.1 (Herkunft, alle Wege)**~~ — **Entfallen 13.10.2026** (Ent-18, L439). *Stand der Freigabe:*
      Abgewiesen mit 403 und `{"ok":false,"error":"embedded-subresource"}` bzw. `"cross-site"` (Legacy:
      Hinweisseite mit 403) wird eine Anfrage mit eingebettetem `Sec-Fetch-Dest` (bestehende Liste) **oder**
      mit `Sec-Fetch-Site` ungleich `same-origin` und `none` — für `maintenance_format_fs`,
      `maintenance_reset_eeprom`, `maintenance_reset_stm32`, `fs_remove` (**C6v**), Legacy
      `/fs?action=format`, `/fs?action=remove` (**C6v**), `/?action=rsteeprom2` und `rststm32` (**C60**) und
      **jeden weiteren** Legacy-Weg zu `stm32_reset()` oder `RESET_EEPROM_RPC_VAR` (Bestandsliste per `grep`,
      im Bericht). Ohne Kopfzeile wie bisher zugelassen (curl, ältere Browser). Logzeile
      `- abgewiesen: <was> als …`.
- [x] ~~**AKH.2 (C60 eigens)**~~ — **Entfallen 13.10.2026** (Ent-18, L439). *Stand der Freigabe:*
      `/?action=rsteeprom2` mit `Sec-Fetch-Dest: image` und mit `Sec-Fetch-Site: cross-site` löst am
      Prüfstand **kein** `rpc (RESET_EEPROM_RPC_VAR)` aus; ohne Kopfzeile und mit `same-origin` wie bisher.
      Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [x] ~~**AKH.3 (Prüfstand c6h)**~~ — **Entfallen 13.10.2026** (Ent-18, L439); c6h liegt mit 484 Fällen auf
      `sicherung/e1-herkunft` (L437). *Stand der Freigabe:* alle Wege aus AKH.1 × Kopfzeilen (ohne, `image`,
      `document`, `same-origin`, `cross-site`, `same-site`, `none`): ausgeführt genau in den erlaubten
      Fällen. Gegenprobe gegen 3.2.33 ⇒ FEHL. Fallzahl gemeldet.
- [x] ~~**AKH.4 (Gerät G3)**~~ — **Entfallen 13.10.2026** (Ent-18, L439; G3 entfällt). *Stand der
      Freigabe:* `maintenance_reset_stm32`: GET ohne Kopfzeile ⇒ 200 und STM-Neustart; mit
      `Sec-Fetch-Site: cross-site` ⇒ 403, kein Neustart; mit `Sec-Fetch-Dest: image` ⇒ 403, kein Neustart.
      Legacy `/?action=rststm32` mit `Sec-Fetch-Site: cross-site` ⇒ 403, kein Neustart. Alles übrige aus
      AKH.1 **nur am Prüfstand** (`design.md` §17.3).

### Schritt E2 — Brücke und Netz (B55-ESP, C8, A79, C59), dazu C46 und C19

- [ ] **AKQ.1 (C46, ESP; aus E1)** — Ist die Query (roh) bei `/api/…` länger als 255 Zeichen, antwortet der
      ESP `{"ok":false,"error":9,"detail":"query too long"}` **ohne** den Handler auszuführen, für GET und
      POST, und schreibt `- abgewiesen: query <n> Zeichen`. Genau 255 ⇒ ausgeführt. Legacy-Pfade
      unverändert. Prüfstand mit 254, 255, 256, 1'000 Zeichen; Gegenprobe gegen 3.2.33 ⇒ FEHL. *Am Prüfstand
      c46 belegt (L437); gilt für den E2-Stand erneut (E2.0).*
- [ ] **AKU.1 (C19, nur L311; aus E1)** — **Beide** Legacy-Zwillinge weisen einen leeren Wert und einen Wert
      über 63 Byte ab, zeigen die Abweisung auf der Seite, und Host bzw. Pfad bleiben unverändert.
      Prüfstand; Gegenprobe gegen 3.2.33 ⇒ FEHL. *Am Prüfstand c19 belegt (L437); gilt für den E2-Stand
      erneut (E2.0).*
- [ ] **AKL.2 (B55, ESP)** — `ldr_min_set` und `ldr_max_set` warten bis 2'000 ms auf `RPCR` und lesen
      dabei die Brücke über **dieselbe** Funktion wie `loop()` (Quittungen gehen weiter hinaus; eine Sperre
      verhindert verschachteltes Lesen). Antwort: `0` ⇒ `{"ok":true,"value":n}`; `1` ⇒ Kennung 10; `2` ⇒
      Kennung 7; keine Antwort ⇒ Kennung 11 und die gedrosselte Zeile `- rpc keine Antwort #n idx=xx`.
      Prüfstand **l55e** mit Brückenattrappe (Antwort, keine Antwort, verstümmelte Zeile, dazwischen
      `var`-Zeilen, die quittiert werden müssen); Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [ ] **AKK.1 (C8)** — Kommandopuffer 260 Byte; eine Logzeile mit 255 Zeichen erreicht den ESP ungekappt;
      im Ring liegt sie in Folgeeinträgen (erster endet mit `~`, Folgeeinträge beginnen mit `~`),
      Ring-RAM unverändert (Ent-10). Prüfstand mit 119, 120, 121, 255 Zeichen; Gegenprobe ⇒ FEHL.
- [ ] **AKE.1 (A79)** — `Serial.setRxBufferSize (1024)` steht **vor** `Serial.begin`, und keine neue
      Ausgabe steht vor der `CAP`-Zeile (`ESP-uclock.ino:166-171`). Statische Prüfung des Leads, Review.
- [ ] **AKT.1 (C59)** — Jede Anfrage trägt 8 Byte Zufall im Transmit-Feld; angenommen wird nur, solange eine
      Anfrage offen ist (Frist 5 s) und das Originate-Feld genau diese Zahl trägt; danach ist die Anfrage
      geschlossen. Neue Gründe in `- ntp verworfen #n <grund>`: `ungefragt`, `originate`. Prüfstand **ntp**
      erweitert; Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [ ] **AKM.2 (RAM)** — „Globale Variablen" aus dem Build vor und nach jedem E2-Task sofort ins
      Messprotokoll. Am Gerät (G4) 30 Heap-Zeilen gegen G0: `free` sinkt um höchstens 1'200 Byte, das
      kleinste `max` liegt höchstens 1'200 Byte unter dem kleinsten `max` aus G0.
- [ ] **AKE.2 (Gerät G4)** — Bei drei ESP-Starts — dem OTA und zwei STM-Resets, die den ESP mit neu starten
      (L421) — bleibt `ovr=0`, und in den ersten 2 Minuten erscheint kein `- ovr #`.
- [ ] **AKK.2 (Gerät G4)** — Nach `/api/weather_get_forecast` steht `info: weather forecast = …` in
      `/api/stm32_log` vollständig (Folgeeinträge zusammengesetzt gleich dem Mitschnitt).
- [ ] **AKT.2 (Gerät G4, lesend)** — Zwei Stunden mit `caffeinate`: jede stündliche Netzzeit übernommen,
      0 × `ntp verworfen`.
- [ ] **AKQ.2 (Gerät G4, lesend; vorher G3)** — `/api/device_ready?x=` mit 300 Zeichen ⇒ Kennung 9; mit 240
      Zeichen ⇒ die gewohnte Antwort.

### Schritt EP — ein Release für ESP und PWA (C6b, C46, B55)

- [x] ~~**AKH.5 (ESP, nur POST, Ent-11)**~~ — **Entfallen 13.10.2026** (Ent-18, L439). *Stand der
      Freigabe:* `maintenance_format_fs`, `maintenance_reset_eeprom`, `maintenance_reset_stm32`, `fs_remove`
      und `network_secrets` werden **nur** per POST ausgeführt (Parameter in der URL, Rumpf leer); der
      POST-Zweig liest dafür die Kopfzeilen und wendet AKH.1 an. GET auf diese fünf ⇒ 405
      `{"ok":false,"error":"method"}`, ohne Ausführung. Ein POST auf einen Pfad, der weder Upload noch einer
      dieser fünf ist, bekommt 404 (heute: gar keine Antwort). Prüfstand **c6p** (Methode × Pfad ×
      Kopfzeilen); Gegenprobe gegen den E2-Stand ⇒ FEHL.
- [ ] **AKS.1 (C6b)** — `/api/eeprom_settings` liefert `key` und `ap_key` als **leere** Zeichenkette und
      zusätzlich `key_set` und `ap_key_set`; alle übrigen Felder unverändert.
- [ ] **AKS.2 (C6b)** — `/api/network_secrets` per **GET**, `Cache-Control: no-store`, liefert
      `{"ok":true,"key":…,"ap_key":…}`. Prüfstand: `eeprom_settings` ohne Wächterschlüssel, `network_secrets`
      mit; Gegenprobe ⇒ FEHL. *Stand der Freigabe: „nur POST, Herkunftsprüfung" — entfällt mit Ent-18; wie
      `network_secrets` geschützt wird, klärt die Schutz-Spec.*
- [x] ~~**AKH.6 (C6, PWA)**~~ — **Entfallen 13.10.2026** (Ent-18, L439). *Stand der Freigabe:* Die vier
      Wartungsaufrufe, das Löschen über `fs_remove` und `network_secrets` gehen mit `method: "POST"` hinaus.
      Prüfstand mit Mock: Methode je Aufruf gezählt, 0 GET auf diese Pfade.
- [ ] **AKS.3 (C6b, PWA)** — Der Backup-Export holt die Schlüssel über `network_secrets`; scheitert der
      Abruf, bricht der Export mit eigener Meldung ab. Bei gesetztem Schlüssel ist `wifi_key` nicht leer.
- [ ] **AKQ.3 (C46, PWA)** — Ist die kodierte Query länger als 255 Zeichen, geht kein Request hinaus, und die
      Meldung nennt Länge und Grenze; Kennung 9 vom Gerät erzeugt dieselbe Meldung. Der Backup-Import sendet
      `eeprom_settings_set` **je Feld** (jeweils mit `boot_as_ap`). Prüfstand: SSID 31 Byte und Schlüssel
      63 Byte aus Sonderzeichen ⇒ alle Felder gesetzt, kein Request über 255.
- [ ] **AKL.3 (B55, PWA)** — Kennung 10 ⇒ „LDR-Minimum nicht übernommen: Der Messwert liegt nicht unter dem
      Maximum." (Maximum sinngemäss); Kennung 7 ⇒ „Messung fehlgeschlagen"; Kennung 11 ⇒ „Keine Rückmeldung
      der Uhr — ob der Wert übernommen wurde, ist offen."; `ok` ⇒ wie heute. DE und EN; S8 grün.
- [ ] **AKZ.11 (Werkzeuge, im EP-Commit)** — `snapshot-device.sh` (`key_set` statt Schlüssel) und
      `preview/server.py` (`network_secrets`, `key_set`) bedienen C6b; vor dem EP-Build gegen die Vorschau
      gelaufen. Die Probe in `checks/hooks.sh`, dass `no-danger.py` die Gefahrenendpunkte auch per
      `curl -X POST` abweist, stammt aus Schritt 0 und bleibt. *Stand der Freigabe, entfallen (Ent-18):*
      `flash-stm.sh` (Reset) und `smoke-device.sh` (Herkunftsprobe) auf POST, gelaufen gegen Prüfstand c6p.
- [ ] **AKG.5 (Einspielen EP)** — ESP-OTA, **unmittelbar** danach `install-app.sh`, dazwischen kein
      Gerätenachweis, keine schreibende Aktion, **kein Backup-Export**. Danach Smoketest samt Update-Quelle,
      `install-app.sh --check`, `check-pwa.sh` ohne Fehler; Zeiten von OTA und Upload im Messprotokoll.
- [x] ~~**AKH.7 (Gerät G5)**~~ — **Entfallen 13.10.2026** (Ent-18, L439); der lesende Teil geht in AKS.4
      auf. *Stand der Freigabe:* POST `maintenance_reset_stm32` ⇒ 200 und Neustart; GET ⇒ 405, kein
      Neustart; POST mit `Sec-Fetch-Site: cross-site` ⇒ 403, kein Neustart. `/api/eeprom_settings` ohne
      Schlüssel, `key_set` true (lesend).
- [ ] **AKL.4 (Gerät G5)** — Über die PWA: Maximum per Zahl auf 1, „Minimum übernehmen" ⇒ „nicht
      übernommen", im Mitschnitt `ldr: minimum abgewiesen`; Maximum zurück auf 4095. Minimum per Zahl auf
      4094, „Maximum übernehmen" ⇒ „nicht übernommen"; Minimum zurück auf 0. Dann „Minimum übernehmen" bei
      0/4095 ⇒ „gespeichert"; Minimum per Zahl zurück auf 0. Endstand 0/4095 lesend; Δ`v` = 0.
- [ ] **AKS.4 (Gerät G5, lesend)** — `/api/eeprom_settings` ohne Schlüssel, `key_set` true (aus AKH.7).
      Backup-Export im Browser enthält `wifi_key`; im Bericht nur der Vergleich der Prüfsumme, nie der
      Klartext.

### Schritt P2 — B48 und B8

- [ ] **AKF.1 (B48)** — `reportInvalidInput()` und `readTimeInputOrReport()` setzen `aria-invalid="true"`,
      legen direkt unter dem Feld eine Fehlerzeile an oder aktualisieren sie und ergänzen
      `aria-describedby`, ohne vorhandene Kennungen zu verlieren; die nächste Eingabe entfernt beides.
      Prüfstand in der Vorschau für jeden Aufrufer (Bestandsliste per `grep`).
- [ ] **AKF.2 (iPhone, Nutzer)** — Fehlerzeile bei offener Tastatur sichtbar an drei Feldern
      (Datumsformat, Uhrzeit, WLAN-Schlüssel zu lang). Ohne Rückmeldung bleibt AKF.2 offen.
- [ ] **AKD.2 (B8)** — Das Werkzeug aus AKD.1 meldet **0** deutsche UI-Texte ausserhalb der Tabellen,
      ausser der Ausnahmeliste; DE und EN haben dieselben Schlüssel; S8 grün; das Werkzeug ist
      Guardrail-Stufe. *Gegenprobe:* neuer deutscher Text ⇒ FEHL.
- [ ] **AKD.3 (Gerät G6)** — `check-pwa.sh` ohne Fehler; mit Sprache Englisch zeigen Hauptmodul, Display,
      Netzwerk, Wartung und Backup keinen deutschen Text.

### Flash, Matrix, RAM

- [ ] **AKM.1 (Flash-Gate)** — Nach **jedem** STM-Task Teilbuild F103 WC12h SK6812-RGBW **und** F411: Byte
      frei und Differenz, **sofort** ins Messprotokoll. Unter 1'024 Byte (F103): bei A70 nach Ent-14
      auslagern, bei A82 als Nachrang zurückstellen (Ent-17), sonst anhalten und vorlegen; **kein Rollout**
      darunter. F103-Teilbuild unter 1'244 Byte ⇒ Matrix-Vorablauf vor dem nächsten Task.
- [ ] **AKM.3 (Matrix)** — Vor dem Release-Build von S1, S2 (und S3), **nach** der Versionsanhebung:
      `make matrix` 72/72, alle ≥ 1'024 Byte, engste Variante mit Byte frei — **sofort** ins Messprotokoll.

### Für alle Schritte

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0.
- [ ] **AKZ.2** — `guardrails.sh --full` vor jedem Release mit unverändertem Warnungsbestand; Ergebnis
      **sofort** ins Messprotokoll.
- [ ] **AKZ.3 (Release-Inhalt)** — Vor jedem Release-Build zeigt `git diff --name-only <letztes Tag>` samt
      Arbeitsbaum Produktänderungen nur in den angehobenen Komponenten (bei EP: ESP und PWA) und nur aus den
      Tasks dieses Releases; Ausgabe im Messprotokoll. Für E2 gehören C46 und C19 dazu, der zurückgebaute
      Herkunftsteil nicht (E2.0).
- [ ] **AKZ.4** — Smoketest nach jedem Einspielen, Ergebnis **sofort** ins Messprotokoll; nach ESP-Updates
      samt Update-Quelle (DIR-009) und `install-app.sh --check` (DIR-017). Der Smoketest ist nicht der Test
      (DIR-012).
- [ ] **AKZ.5 (Logwache)** — läuft bei jedem Gerätelauf, vor dem Einspielen gestartet (DIR-013); am Ende
      `watch-log.sh --pruefe` mit Exit 0 im Bericht. Neue Zeilen je mit eingespielter Gegenprobe:
      `icon: verworfen` (Hinweis), `icon_freeze: ENTER` (Alarm ab S2), `- rpc keine Antwort` (Alarm),
      `- abgewiesen: query` (Hinweis), `ntp verworfen … ungefragt|originate` (Alarm wie bisher).
- [ ] **AKZ.6** — `pwa-tester` Phasen 0–4 und 9 nach G5 (T1) und nach G6 (T2). Unvollständig ⇒ nicht
      abgenommen.
- [ ] **AKZ.7** — Jedes Release sofort committet, getaggt, **gepusht** (DIR-011); `CHANGELOG.md` schreibt
      der Lead.
- [ ] **AKZ.8** — Einspielzeilen mit Reihenfolge je Release, fertig zum Einfügen.
- [ ] **AKZ.9** — Kodierung **und** Zeilenende jeder Datei vor dem Patch festgestellt und im Taskbericht
      genannt (DIR-015), darunter `uart-driver.h`, `main.c`, `display.c`, `eeprom-data.h`, `ldr.c`,
      `ESP-uclock.ino` (**gemischt**), `http.cpp` (UTF-8), `ntp.cpp`, `app.js`, `en.json`, `index.html`,
      `styles.css`.
- [ ] **AKZ.10** — Keine Versionsnummer in lebender Doku (S9); jede zitierte Kennung im Katalog (S11).

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **F4** | Ent-8: eigenes Folgepaket |
| **Schutz der zerstörerischen Aufrufe:** C6 (Herkunft **und** POST), C6v, C60 (L426, hoch), C61 (L438), C62 (L440); der Schutz von `network_secrets`; die Antwort auf einen POST an einen unbekannten Pfad | Ent-18 (L439): eigene Spec. Der E1-Code liegt auf `sicherung/e1-herkunft` (`2662368`), nicht als Schutz zu verwenden. Die Prüfung über `Sec-Fetch-Dest` aus ESP 3.2.4 bleibt, wie sie ist — weder zurückbauen noch erweitern |
| **B53**, **A19**, **A63** | Nicht „mittel", ohne offene Entscheidung |
| **A77**, **A78** | Nicht beauftragt. Wer in `main.c` patcht, fasst den Kommentar aus A78 **nicht** an |
| **A70 (a)**, **(b)** | Ent-2: ohne Fix |
| **Display-Power im EEPROM** | Nicht entschieden; A56 betrifft nur das Ambilight |
| **L343 / C32** | Eigener Befund |
| **Übriger Legacy-Zweig** (roher `atoi`, C19 ausser L311) | Ent-7 |
| **Legacy auf POST** | Stand der Freigabe: Ent-12 (Legacy bleibt GET mit Herkunftsprüfung); Ent-12 aufgehoben, Gegenstand der Schutz-Spec |
| **POST für Update-Auslöser, `ir_code_set`, `learn_ir`, `test_display`, `eeprom_settings_set`, `local_stm32_flash`** | Stand der Freigabe: Ent-12; aufgehoben, Gegenstand der Schutz-Spec |
| **C46 für Legacy-Pfade** | Befund und Entscheidung nennen `/api/…` |
| **Ringzeilen mit 255 Zeichen** | Ent-10 |
| **Legacy-LDR-Seite** (`http.cpp:4317`, `:4322`) | bleibt „anstossen" |
| **Icon nicht anfordern, wenn das Display aus ist** | Verwerfen am Empfang genügt |
| **Messbau für A15 (M0/M1), Zyklenmessung am Gerät** | Ent-15 aufgehoben (L432). Der Patch `tools/checks/auszug/stm/a15/messbau.patch` bleibt liegen, wird nicht eingesetzt |
| **Fehlerfälle am Gerät** für Format, EEPROM-Reset, Löschen, `rsteeprom2`, leere Update-Quelle, C59, A70 (c), (d), L399 | Gefährlich oder nicht herstellbar; Prüfstände (`design.md` §17.3) |
| **Frühere Specs**, `REVIEW*.md` | Momentaufnahmen (DIR-006) |

---

## Betroffene Laufzeiten

- [ ] **STM32** (`src/**`) — S1: `uart/uart-driver.h`; S2: `main.c`, `display/display.c` (+ `.h`),
      `eeprom/eeprom-data.h`, `ldr/ldr.c` (+ `.h`); je ein Flash
- [ ] **ESP8266** — ~~E1~~ entfällt; E2: `ESP-uclock.ino`, `http.cpp` (B55, dazu C46 und C19 aus E1),
      `ntp.cpp` (ggf. `vars.cpp`); EP: `http.cpp` (C6b); je ein OTA
- [ ] **PWA** — EP: `app.js`, `i18n/en.json`; P2: `app.js`, `i18n/en.json`, `index.html`, `styles.css`
- [ ] **Werkzeug** (`tools/**`, `.claude/**`) — Logwache, Stop-Hook, F3, B8, lokaler STM-Flash (gebaut, nicht
      eingesetzt), Prüfstände, C6b in `snapshot-device.sh` und `preview/server.py` (~~POST-Umstellung der
      Werkzeuge~~ entfällt); der Messcode-Patch liegt, wird nicht eingesetzt
- [ ] **Dokumentation** — `BEFUNDE.md`, `TESTPLAN-PWA.md` (`doc-writer`; K25, K26 entfallen); `CHANGELOG.md`
      (Lead); ~~`CLAUDE.md` Gefahrentabelle (POST)~~ entfällt
- [ ] **Build/Release** — fünf Releases (S1, S2, E2, EP, P2), bei Gate-Riss sechs (S3); Matrix vor jedem
      STM-Release; je Commit, Tag, Push

---

## Entscheidungen

**Entschieden vom Nutzer am 13.10.2026.** Empfehlung in Klammern, wo abweichend.

- **Ent-1 — A56:** im EEPROM speichern; Platz nach Ent-9.
- **Ent-2 — A70:** alle vier Fälle beheben — (c), (d), (e), L399; (a) und (b) ohne Fix.
- **Ent-3 — A79:** `setRxBufferSize(1024)` vor `Serial.begin`; belegen, dass `ovr` beim Start 0 bleibt.
- **Ent-4 — A80:** Icon bei ausgeschaltetem Display verwerfen; Restore-Bedingung nicht vereinfachen.
- **Ent-5 — C8:** Brücke aufweiten, RAM messen; präzisiert durch Ent-10.
- **Ent-6 — B55:** Die API wartet das Ergebnis des STM ab (ESP und PWA, Firmware zuerst); dafür braucht es
  die STM-Rückmeldung `RPCR` (Problem §5).
- **Ent-7 — C19:** nur der L311-Teil; der übrige Legacy-Zweig bleibt.
- **Ent-8 — F4:** eigenes Folgepaket.
- **Ent-9 — A56 Speicherort:** zwei Byte (Marke und Wert) hinter dem Layoutende, ohne Layoutversion —
  rückflashsicher.
- **Ent-10 — C8:** Kommandopuffer 260 Byte, Folgeeinträge im Ring, kein zusätzliches Ring-RAM.
- ~~**Ent-11 — C6 einstufig**~~ — **aufgehoben 13.10.2026 (L439)**, Ent-18. *Stand der Freigabe*
  (Empfehlung war: zwei Stufen): Die umgestellten Endpunkte nur noch per POST; ESP und PWA als **ein**
  Release (EP), ESP zuerst, PWA unmittelbar danach; dazwischen kein Gerätenachweis und kein anderes
  Release; alle Werkzeuge vorher auf POST; zwischen ESP und PWA kein Backup-Export. **Was davon bleibt:**
  EP bleibt ein Release mit ESP und PWA, gleiche Einspielregel — der Grund ist jetzt C6b (`design.md` §8.4).
- ~~**Ent-12 — C6 Umfang**~~ — **aufgehoben 13.10.2026 (L439)**, Ent-18. *Stand der Freigabe:*
  Herkunftsprüfung über `Sec-Fetch-Site` (und `Sec-Fetch-Dest`) für **alle** zerstörenden Wege
  einschliesslich Legacy (`rsteeprom2`, `rststm32`, C60) und `/fs` (C6v); POST zusätzlich für die
  API-Endpunkte der PWA (vier Wartungsendpunkte und `network_secrets`). Legacy bleibt GET. Update-Auslöser,
  `ir_code_set`, `learn_ir`, `test_display` unverändert. C60 (L426) eigener Task, hoch.
- **Ent-13 — Gerätenachweise:** alle freigegeben, auch der STM-Reset im Nacht-Aus-Fenster für A70 (e); bei
  jedem auf später geplanten Test `caffeinate` durch den Lead (DIR-020). Dazu die bestehenden Freigaben
  für Updates und STM-Reset.
- **Ent-14 — Gate-Riss in S2:** A70 ohne Rückfrage in ein eigenes STM-Release (S3) mit Flash-Analyse.
- ~~**Ent-15 — A15 Messbau am Gerät**~~ — **aufgehoben 13.10.2026** (Entscheidung des Nutzers, L432): A15
  bleibt, **ohne** Messbau. Seit LTO (STM 3.2.16) sind die SPL-Aufrufe in der ISR eingebettet; im Bau
  3.2.32 hat keine ISR auf F103/F411 ein `bl`, der Gewinn sind wenige Befehle. Gemessen wird statisch
  (AKI.2), am Gerät läuft der Regressionslauf (AKI.3).
- **Ent-16 — Spec: freigegeben** am 13.10.2026.
- **Ent-17 — A81 und A82 aufgenommen** in S2, soweit das Flash-Budget es trägt; sonst Nachrang (A82 mit
  A70 nach S3; A81 kostet keinen Platz).
- **Ent-18 — Schutz der zerstörerischen Aufrufe zurückgestellt** (13.10.2026, Entscheidung des Nutzers;
  `BEFUNDE.md` L439, L440 und Abschnitt „Entscheidung: Schutz der zerstörerischen Aufrufe — zurückgestellt,
  eigene Spec"). Browser senden `Sec-Fetch-*` über http nicht (am Gerät belegt, Chrome 155); belegt ist, dass
  Chrome bei POST `Origin` sendet. Der Schutz für C6, C6v, C60, C61 und C62 wird in einer eigenen Spec
  geklärt. Folgen für dieses Paket: Ent-11 und Ent-12 aufgehoben; Release E1 entfällt, der Herkunftsteil
  ist auf `sicherung/e1-herkunft` (`2662368`) gesichert; C46 (ESP) und C19 gehen mit E2; EP verliert die
  POST-Umstellung und alles zur Herkunft und behält C6b, den PWA-Teil von C46 und B55; AKH.1–AKH.7 entfallen;
  G3 geht in G4 auf; die Testplanfälle K25 und K26 entfallen. Versionsfolge ESP: E2 3.2.34, EP 3.2.35.
