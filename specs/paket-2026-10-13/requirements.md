# Anforderungen — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (A15, A56, A70, A79, A80, B8, B48, B55, C6, C6b, C6v, C8, C19, C46, C59, E36, F3)

**Status: Entwurf — zur Freigabe.** Ohne Freigabe des Nutzers beginnt keine Umsetzung. Sieben Fragen
stehen offen (Abschnitt „Offene Entscheidungen", N-1 bis N-7); bis sie beantwortet sind, gelten die
Empfehlungen aus `design.md` als Vorschlag, nicht als beschlossen.
**Erstellt:** 2026-10-13, Stand `159c9d0` (Zweig `pwa-decoupling`). Ausgangsstand am Gerät laut
Endstand von `specs/paket-2026-10-11/`: STM 3.2.32, ESP 3.2.33, PWA 1.4.97.
**Auslöser:** Auftrag des Nutzers vom 13.10.2026 — alle offenen ToDos mit Kritikalität „mittel" und
alle, die seine Entscheidung brauchten, **ausser F4** (eigenes Folgepaket). Die Entscheidungen stehen
an den Einträgen in `BEFUNDE.md` („Entscheidung des Nutzers 13.10.2026") und unten unter
„Entscheidungen".
**Grundlage:** `BEFUNDE.md` (ToDo-Liste, nachgezählt am 13.10.2026; Befundzeilen L22, L35, L99, L101,
L311, L312, L330, L343, L345, L382, L396, L399, L406, L417, L421, L422, L423, L424),
`REVIEW-2026-09-29.md` Massnahme 14. Der `spec-writer` hat jede tragende Fundstelle am Code `159c9d0`
nachgelesen; Abweichungen von `BEFUNDE.md` sind am Befund vermerkt.

Dieses Dokument ist eine **Momentaufnahme** (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Fundstellen.** Zeilennummern gelten für `159c9d0`; sie sind Fundstellen, keine Anker.
Kennzeichen: **✔ SW** — vom `spec-writer` am Code nachgelesen; **✔ B** — in `BEFUNDE.md` als belegt
geführt (Nachzählung 13.10.2026 oder Gerätebeleg), nicht einzeln nachgelesen; **◐** — am Code
abgeleitet, nicht belegt; **●** — gemeldet, vor der Umsetzung zu prüfen.

**Kennungen.** Schritte: **0** (ohne Firmware), **S1** (STM-Release 1, isoliert: A15), **S2**
(STM-Release 2: B55-STM, A56, A80, A70), **E1** (ESP-Release 1, HTTP-Schicht: C6 Stufe 1, C6v, C6b,
C46, C19), **E2** (ESP-Release 2, Brücke und Netz: B55-ESP, C8, A79, C59), **P1** (PWA-Release 1: C6,
C6b, C46, B55), **T1** (`pwa-tester`), **E3** (ESP-Release 3: C6 Stufe 2), **P2** (PWA-Release 2: B48,
B8), **T2** (`pwa-tester`), **Z** (Doku). Gerätenachweise **G0** (Grundlinien) bis **G7**.
Akzeptanzkriterien: **AKW** (Logwache, E36), **AKB** (Backup-Umfang, F3), **AKD** (deutsche Texte, B8),
**AKI** (UART-ISR, A15), **AKL** (LDR-Ergebnis, B55), **AKA** (Ambilight-Power, A56), **AKC** (Icon,
A80), **AKN** (Nachtfenster, A70), **AKH** (Herkunft und POST, C6/C6v), **AKS** (Schlüssel, C6b),
**AKQ** (Query-Länge, C46), **AKU** (Update-Quelle Legacy, C19), **AKK** (Kommandopuffer, C8),
**AKE** (ESP-Empfangspuffer, A79), **AKT** (NTP-Bindung, C59), **AKF** (Fehlerzeile, B48), **AKM**
(Flash und RAM), **AKZ** (alle). Offene Fragen **N-1 bis N-7**. Die Kürzel gelten nur in diesem Paket.

---

## Überblick

| Schritt | Inhalt | Laufzeit | Einspielen | Gerätenachweis |
|---|---|---|---|---|
| **0** | **E36** Logwache mit Lebenszeichen · **F3** Umfangsprüfung Backup · **B8** Zählwerkzeug und Bestandszählung · Grundlinien (A15 vorher, ESP-RAM, Brückenlast) · neue Befunde eintragen | Werkzeug, Doku | — | **G0** (Grundlinien) |
| **S1** | **A15** UART-ISR — eigenes Release, eigene Messung | STM | zuerst | **G1** |
| **S2** | **B55-STM** Ergebniszeile · **A56** Ambilight-Power im EEPROM · **A80** Icon ohne Display verwerfen · **A70** Randfälle (c), (d), (e), L399 | STM | nach G1 | **G2** |
| **E1** | HTTP-Schicht: **C6 Stufe 1** (POST zusätzlich, Herkunft auch über `Sec-Fetch-Site`) · **C6v** · Legacy-Zerstörer mit Herkunftsprüfung · **C6b** · **C46** · **C19** (nur L311) | ESP | nach G2 | **G3** |
| **E2** | Brücke und Netz: **B55-ESP** (wartet auf das Ergebnis) · **C8** · **A79** · **C59** | ESP | nach G3 | **G4** |
| **P1** | **C6** (POST in der PWA) · **C6b** (Export über den neuen Endpunkt) · **C46** (Längenprüfung, Aufteilung beim Import) · **B55** (Meldung) | PWA | nach G4 | **G5** |
| **T1** | `pwa-tester` Phasen 0–4 und 9 (DIR-012) | — | nach G5 | — |
| **E3** | **C6 Stufe 2:** GET auf die zerstörenden Endpunkte ⇒ 405 | ESP | nach T1 | **G6** |
| **P2** | **B48** Fehlerzeile am Feld · **B8** deutsche Texte in die i18n-Tabelle | PWA | nach G6 | **G7** (samt iPhone, Nutzer) |
| **T2** | `pwa-tester` Phasen 0–4 und 9 | — | nach G7 | — |
| **Z** | `BEFUNDE.md`, `CLAUDE.md` (Gefahrentabelle, POST), `TESTPLAN-PWA.md`, Spec-Endstand | Doku | — | — |

**Warum diese Reihenfolge:** `design.md` §0. Kurz: `make release-zip` baut alle drei Komponenten
zusammen; deshalb entsteht der Code des nächsten Releases erst **nach dem Release-Build** des vorigen,
und jeder Release-Build erst **nach dem Gerätenachweis** des vorigen (R3b). **Firmware vor PWA**
(DIR-002): B55 braucht STM, dann ESP, dann PWA; C6, C6b und C46 brauchen den ESP vor der PWA. **A15
allein**, weil die ISR jeden Pfad der Brücke berührt und ein Fehler dort sonst niemandem zuzuordnen
wäre. **E1 und E2 getrennt**, weil Fehlerbilder der HTTP-Schicht (Absturz bei einer Anfrage) und der
Brücke (Überlauf, Quittungen, RAM) sonst nicht auseinanderzuhalten sind. **E3 nach P1 und T1**, damit
es kein Zeitfenster gibt, in dem die eingespielte PWA einen Wartungsaufruf nicht mehr ausführen kann.

---

## Problem

### 1. A15 / L101 — die UART-ISR ruft vier SPL-Funktionen je Zeichen

**✔ SW:** `src/uart/uart-driver.h:773` und `:775` (`USART_GetITStatus`/`USART_ClearITPendingBit` für
RXNE), `:804` (`USART_ReceiveData`), `:833` und `:837` (dasselbe für TXE); ORE wird schon direkt aus
`SR` gelesen (`:796`). Die ISR ist eine Vorlage, die **drei** Instanzen erzeugt (ESP-Brücke,
Log-UART, DFPlayer, `uart-driver.h:733-737`). **✔ B (L101):** 100 bis 130 Befehle je Zeichen, Ziel unter
30 — eine Zahl vom 03.10.2026, in G0 neu zu messen.

**Was ein Ersatz einhalten muss, ✔ SW:** `USART_GetITStatus()` prüft Flag **und** Freigabebit. TXE ist
gesetzt, sobald das Senderegister leer ist, die Freigabe TXEIE nur während des Sendens (`:854`). Ein
Ersatz, der nur `SR & TXE` prüft, sendete bei jedem Empfangsinterrupt. **◐:** `USART_ClearITPendingBit`
für TXE ist wirkungslos, weil TXE nur lesbar ist (SPL-Vertrag, nicht am Projektcode nachgelesen).

### 2. A56 / L330 — die Ambilight-Power übersteht keinen Neustart

**✔ SW:** `display_init()` setzt `ambilight_power_is_on = 1` (`src/display/display.c:6520`); einen
EEPROM-Platz gibt es nicht (`src/eeprom/eeprom-data.h`, Layout bis `EEPROM_DATA_END` = 1979,
Layoutversion 2.9). Geschaltet wird an genau einer Stelle, `display_set_ambilight_power()`
(`display.c:2907-2919`), aufgerufen aus `set_display_power()` (`src/main.c:1447`), dem Setter
`DISPLAY_AMBILIGHT_POWER_NUM_VAR` (`main.c:1785-1792`) und dem Ambilight-Timer (`main.c:4458-4460`).

**Für die Migration, ✔ SW:** Gelesen wird die Konfiguration nur bei `eep_version <= EEPROM_VERSION`
(`main.c:1224-1225`). Hebt ein Release die Layoutversion an, liest jede **ältere** Firmware danach
**keine einzige** Einstellung mehr und schreibt den Werkszustand (`upgrade_eep_version()`,
`main.c:1300-1418`) — ein Rückflash kostet dann alle Einstellungen. Die Anzeige der Layoutversion setzt
die Unterversion als Ziffer zusammen (`main.c:1229`, `+ '0'`); eine Version 2.10 erschiene als „2.:".
L343 (Index 9 kippt) bleibt bei C32 und ist nicht Teil.

**Neue Nebenbefunde, ✔ SW (nicht Teil, in Schritt 0 einzutragen):**
- (a) `display_read_config_from_eep()` liest die Weissfarbe der Ambilight-Markierung vom Offset der
  **Ambilight**-Weissfarbe (`display.c:3435`); geschrieben wird sie an ihren eigenen Offset (`:3748`).
- (b) `dcf77_enabled` wird nur in `set_display_power()` neu bestimmt (`main.c:1471-1478`); der
  Ambilight-Setter (`:1789`) und der Ambilight-Timer (`:4460`) lassen es stehen.

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

**Neu, ✔ SW:** Der DCF77-Empfang läuft nur, solange Display **und** Ambilight aus sind (`main.c:891`,
`:1471-1478`). Der L399-Fall tritt also erst ein, nachdem das Display aus ist; die Korrektur bewirkt vor
allem, dass danach die Sprungerkennung greift.

### 4. A80 / L422 — ein Icon bei ausgeschaltetem Display bleibt vorgemerkt

**✔ SW:** Der Zweig `ESP8266_ICONDATA` setzt `do_display_icon = 1` und `show_icon_stop_time`
(`main.c:3551-3558`), ohne auf die Display-Power zu sehen. `display_icon()` tut bei ausgeschaltetem
Display nichts (`display.c:5087`) und setzt `do_display_icon` erst nach Ablauf zurück (`:5229-5234`).
Die Restore-Bedingung (`main.c:4857-4861`) wartet auf `! display.do_display_icon`. **✔ B (L422):**
48 Minuten am Gerät. **◐:** Dasselbe tritt ein, wenn das Display **während** eines Icons ausgeht —
`display_icon()` steht dann ebenso.

### 5. B55 / L417 — „Messwert übernehmen" meldet „gespeichert", auch wenn der STM abweist

**✔ SW:** `http_api_ldr_min_set()` und `http_api_ldr_max_set()` (`ESP8266/ESP-uclock/http.cpp:10151-10160`,
`:10224-10233`) stossen den RPC an und antworten sofort `{"ok":true}`. Der STM führt ihn aus
(`main.c:1620-1634`), `ldr_set_min_value()` weist ab (`src/ldr/ldr.c:194-197`) und meldet über
`var_send_ldr_min_value()` den **alten** Wert.

**Daraus folgt, ✔ SW:** Am unveränderten Wert allein ist eine Abweisung **nicht** erkennbar.
Unverändert heisst auch „Messung fehlgeschlagen" (`ldr.c:177-188`, nach 100 ms kein Wert) oder
„derselbe Wert gemessen" — Letzteres tritt ein, wenn man bei gleichem Licht zweimal drückt. Es braucht
eine **Rückmeldung des STM** (STM-Teil). Zweitens: `var_send_*` wartet auf die Quittung des ESP. Ein
HTTP-Handler, der wartet, ohne die Brücke zu lesen, liesse den STM in seine 3-s-Frist laufen (`v=`
steigt). Gelesen wird die Brücke heute nur am Ende von `loop()` (`ESP-uclock.ino:1440-2255`).

### 6. C8 / L99 — lange Logzeilen werden gekappt

**✔ SW:** Der STM sendet `LOG ` und bis 255 Zeichen (`src/log/log.c:20`, `:35-45`;
`src/esp8266/esp8266.c:871-907`). Der ESP nimmt 127 in den Kommandopuffer (`CMD_BUFFER_SIZE 128`,
`ESP-uclock.ino:71`, Kappung mit `~` `:1446-1449`, `:2245-2252`), der Logring hält **120** je Zeile
(`:82`, `stm32_log_append()` `:93-114`).

**Daraus folgt, ✔ SW:** Ein Kommandopuffer von 256 Byte **allein** ändert in `/api/stm32_log` nichts — der
Ring kappt weiter bei 120. Für eine Zeile mit 255 Zeichen samt `LOG ` braucht der Puffer **260** Byte.
Ringzeilen mit 255 Zeichen kosteten 32 × 135 = 4'320 Byte RAM, bei 9'472 Byte freiem Heap (L421) — der
Nutzer hat den Ring am 04.10.2026 aus genau diesem Grund halbiert (L176). Der Mitschnitt läuft über die
Log-UART des STM und ist nicht betroffen. ⇒ N-2.

### 7. C6b / L22 — `/api/eeprom_settings` gibt den WLAN-Schlüssel im Klartext heraus

**✔ SW:** `http_api_eeprom_settings()` (`http.cpp:9311-9331`) sendet `key` und `ap_key`. Einziger Leser in
der PWA ist der Backup-Export (`app.js:4914-4917`, `buildNetworkBackupSettings()` `:4166-4178`);
`ensureBackupExportState()` (`:4993-5009`) bricht ab, wenn `eeprom_settings` fehlt. Der Import schreibt
den Schlüssel zurück, `eeprom_settings_set` übergeht leere Felder (`http.cpp:9379-9395`). ⇒ Ein leerer
Schlüssel im Export ginge erst beim Wiederherstellen auf ein **frisches** Gerät verloren, still. Weitere
Leser: `tools/snapshot-device.sh:154` (hasht ihn), `tools/preview/server.py:83`.

### 8. C6 / R2-14 und C6v / L312 — zerstörende Endpunkte

**✔ SW:**
- Die Herkunftsprüfung (`http_request_is_embedded_subresource()`, `http.cpp:476-499`) steht nur in
  `format_fs`, `reset_stm32`, `reset_eeprom` (`:9708-9760`), **nicht** in `fs_remove` (`:12349`) und
  nicht im Legacy-Zweig `/fs?action=format` (`:6718-6722`) und `action=remove` (`:6765`).
- **Neu:** Die Legacy-Hauptseite löst mit `action=rsteeprom2` den EEPROM-Reset per GET aus, ohne jede
  Prüfung (`http.cpp:3464-3469`) — derselbe Schaden wie `maintenance_reset_eeprom`, ohne dessen Schutz.
- Der POST-Zweig (`:14628-14659`) liest **keine** Kopfzeilen und kennt nur Upload-Pfade (`http_post()`,
  `:14374-14424`); ein POST auf einen anderen Pfad bekommt **keine Antwort**.
- PWA: alle Aufrufe über `apiFetch()` (`app.js:2533-2576`), das Optionen durchreicht; Aufrufer der
  Wartungsendpunkte `:5393`, `:11787`, `:11851`, `:11855`, `:11875`, `:12412`; `fs_remove` über
  `fs_remove_base_url` (`:10602`). Der Service Worker lässt Nicht-GET durch (`sw.js:121`).
- Werkzeuge mit GET: `tools/flash-stm.sh:131` (Reset nach dem Flashen), `tools/smoke-device.sh:229`.
- **Wichtig für die Lösung:** POST allein schützt nicht vor fremden Seiten — ein Formular-POST von
  fremder Herkunft ist ein „einfacher" Request ohne Vorabprüfung. Wirksam ist die Herkunft
  (`Sec-Fetch-Site`). ⇒ N-3, N-4.

### 9. C19 / L311 — Legacy-Zwillinge der Update-Quelle prüfen nichts

**✔ SW, weicht von `BEFUNDE.md` ab:** Es sind **zwei** Stellen, nicht eine: die Dateisystemseite
(`http.cpp:6724-6731`) und die Update-Seite (`:7520-7527`). Beide setzen über `set_strvar()` ungeprüft,
leer und überlang eingeschlossen. Leer ⇒ Rückfall auf den Server des Ursprungsprojekts (L42, DIR-009).

### 10. C46 / L345 — Query über 255 Zeichen still gekürzt

**✔ SW:** `http()` kopiert mit `strncpy (param, const_param, 255)` (`http.cpp:13795-13798`), der
POST-Zweig ebenso (`:14643-14649`). Die Anfragezeile selbst wird ungekürzt gelesen
(`http_read_request_line()`, `:760-786`), die Länge ist also messbar. Belegt sind die Fehlerkennungen 1
bis 8 (`:244-273`).

**Rechnung, neu:** Der Backup-Import sendet `eeprom_settings_set` mit bis zu fünf Feldern. Mit einer
SSID von 31 Byte und einem Schlüssel von 63 Byte, jedes Byte prozentkodiert, ist schon `ssid=…&key=…`
über 300 Zeichen lang; ein Schlüssel aus 63 Sonderzeichen allein sind 189. Wer über 255 abweist, statt
zu kürzen, bricht einen solchen Import — die PWA muss **aufteilen**.

### 11. C59 / L406 — NTP-Antwort an keine Anfrage gebunden

**✔ SW:** `ntp_poll_time()` prüft Absender, Port, Länge, LI, Stratum und Transmit (`ntp.cpp:156-202`).
Es gibt keinen Merker „Anfrage offen", das Transmit-Feld der Anfrage ist leer (`send_ntp_packet()`,
`:117-136`), das Originate-Feld der Antwort (Byte 24–31) wird nicht geprüft.

### 12. A79 / L421 — ESP-Empfangspuffer läuft beim Start einmal über

**✔ SW:** `Serial.begin(115200)` (`ESP-uclock.ino:160`), kein `setRxBufferSize`; die Vorgabe von 256 Byte
nennt nur ein Kommentar (`:474`). **✔ B:** `- ovr #1 t=5561` am 13.10.2026, zeitgleich mit
Tabellenabfrage und Vollabgleich.

### 13. B48 / L382 — Fehlermeldungen auf dem iPhone von der Tastatur verdeckt

**✔ SW:** `reportInvalidInput()` (`app.js:6614-6622`) meldet über das Statusbanner und setzt danach den
Fokus. **✔ B:** kein `aria-invalid`, kein `aria-describedby` in `app.js` und `index.html`. Die Wirkung
ist nur am iPhone prüfbar.

### 14. B8 — hartcodierte deutsche Texte

**●** Die Zahl ist nicht gezählt. „~176" ist überholt; die Nachzählung vom 13.10.2026 fand 15
Satzliterale, erfasste aber weder Texte ohne typische Satzwörter noch das Markup (✔ B). Die DE-Tabelle
steht in `app.js` (`I18N_DE`, `:20-1026`), die EN-Tabelle in `data/app/i18n/en.json`; `index.html`
trägt 309 Attribute `data-i18n` (✔ SW, Anzahl). **Vor der Arbeit vollständig zählen.**

### 15. F3 / L35 (d) — keine Prüfung, ob das Backup alle Setter abdeckt

**✔ B:** Es gibt keine. **✔ SW:** Der Dispatch in `http.cpp` hat 59 Zeilen der Form
`strcmp (path, "/api/…_set")`. Ob jede davon ein Setter im Sinne des Backups ist, klärt die Prüfung
selbst.

### 16. E36 / L423 — die Logwache stand still, ohne es zu melden

**✔ B:** 12.10.2026, 00:20 bis 01:33, Prozess lebte, keine Ausgabe. **✔ SW:** `tools/watch-log.sh` ruft
`tools/logger/log.sh tail` ohne Frist (`:74-76`, `:88` und je Runde) und schläft mit `sleep "$INTERVAL"`
(`:377`). Einen eigenen Aussetzer erkennt sie nicht, ein Lebenszeichen nach aussen gibt es nicht.
**●** Die Ursache ist nicht untersucht (Ruhezustand des Mac? blockierender Netzaufruf?).

### 17. Budgets

- **Flash F103 (✔ B):** Teilbuild 1'656 Byte frei (S8b, 13.10.2026), engste Matrix-Variante
  `wc24h-stm32f103-sk6812-rgbw` 1'436 (L416). **Spielraum über dem Gate von 1'024: 412 Byte** an der
  engsten Variante. Die Schätzungen des Vorpakets lagen um den Faktor 1,5 bis 3 daneben (L393, L416).
- **ESP-RAM (✔ B, L421):** Heap frei 9'472 Byte, grösster Block 8'736 Byte. A79 kostet rund 768 Byte
  Heap, C8 rund 132 Byte BSS.

---

## Ziel

1. **Logwache:** Eine stillstehende Wache meldet sich selbst — beim Weiterlaufen und von aussen
   prüfbar; ein Aufruf des Mitschnitts kann sie nicht mehr unbegrenzt anhalten.
2. **Backup-Umfang:** Jeder `*_set`-Endpunkt steht im Backup oder mit Grund in einer Ausnahmeliste;
   eine Lücke schlägt in den Guardrails an.
3. **UART-ISR:** Kein SPL-Aufruf mehr im Empfangs-Normalpfad, auf beiden Plattformen gemessen, mit
   nachgewiesen gleichem Verhalten.
4. **Ambilight-Power:** übersteht einen Neustart; ein Rückflash kostet keine Einstellung (N-1).
5. **Nachtfenster:** (c) das Aus kommt spätestens nach einer zweiten Frist, (d) das erste `TIME`
   schneidet den IP-Lauftext nicht ab, (e) ein fremder Ticker verzögert das Aus nicht, L399 auch bei
   reinem DCF77-Betrieb.
6. **Icon:** Bei ausgeschaltetem Display wird ein Icon verworfen; nichts wartet mehr darauf.
7. **LDR-Übernahme:** Die PWA meldet „gespeichert" nur, wenn der STM übernommen hat; sonst
   „abgewiesen", „Messung fehlgeschlagen" oder „keine Rückmeldung".
8. **Zerstörende Endpunkte:** nur per POST und nur von gleicher Herkunft; Legacy bleibt bedienbar.
9. **Schlüssel:** `/api/eeprom_settings` gibt keinen Schlüssel mehr heraus; das Backup enthält ihn
   weiterhin.
10. **Query-Länge:** über 255 Zeichen abgewiesen statt gekürzt; die PWA sendet nichts, was der ESP
    abweisen müsste, und der Backup-Import funktioniert auch mit langen Schlüsseln.
11. **Legacy-Update-Quelle:** leer und überlang abgewiesen.
12. **Brücke:** lange Logzeilen kommen vollständig an (N-2); der ESP-Empfangspuffer läuft beim Start nicht
    mehr über.
13. **NTP:** nur noch eine Antwort auf die eigene, offene Anfrage.
14. **PWA:** Fehlerzeile am Feld (B48); keine deutschen Texte ausserhalb der i18n-Tabellen (B8).
15. **Nachweis:** jede Wirkung einzeln, jede neue Prüfung einmal fehlgeschlagen (DIR-014), jede Messung
    **bei der Messung** in `BEFUNDE.md` (L424).

---

## Akzeptanzkriterien

Jedes Kriterium nennt sein **Instrument** und den **Weg, auf dem die Meldung ankommt** (DIR-014).
Prüfstände liegen unter `tools/checks/auszug/`, laufen über `tools/checks/auszug.sh` mit den Typbreiten
des Ziels (L256) und sind **einmal gegen den Vorgängerstand fehlgeschlagen**; die Fallzahl wird gemeldet
und nachgezählt. **Was am Gerät nicht herstellbar oder dort gefährlich ist, wird nicht am Gerät
verlangt** — die Liste dessen, was **nie** am Gerät gefahren wird, steht in `design.md` §17.3.
Logzeilen im STM in Umschrift (DIR-015); der Wortlaut ist verbindlich, wo er hier steht, weil die
Logwache danach sucht.

**Messwerte sofort festhalten (L424, Skill `/release`):** Jede Messung, die ein Kriterium belegt —
Flash je Task, Matrix-Lauf, RAM, `--full`, Smoketest, Gerätewert —, steht **in dem Moment, in dem sie
entsteht,** als Eintrag in `BEFUNDE.md` (Messprotokoll je Release, `design.md` §1.4). Ein Kriterium,
dessen Beleg dort fehlt, gilt als **nicht** erfüllt — auch wenn die Arbeit getan ist.

### Schritt 0 — ohne Firmware

- [ ] **AKW.1 (E36, Ursache)** — Der Lead wertet `pmset -g log` für den 12.10.2026, 00:00–01:40,
      gegen das Stillstandsfenster aus und trägt das Ergebnis als „belegt" oder „nicht belegt" in L423
      ein (DIR-003); dazu, ob `log.sh` in diesem Fenster einen Netzaufruf offen hatte, soweit
      feststellbar.
- [ ] **AKW.2 (Lebenszeichen)** — Die Wache schreibt je Runde eine Herzschlagdatei mit Zeitstempel und
      Rundenzahl. `./tools/watch-log.sh --pruefe` gibt Exit 1 und „Wache steht seit N s" aus, wenn der
      Zeitstempel älter als 3 × Intervall ist oder die Datei fehlt, sonst Exit 0 mit Alter und
      Rundenzahl. Der Stop-Hook ruft `--pruefe` auf, wenn die Gerätemarke `geraet-gemessen` gesetzt ist,
      und meldet ein Exit ≠ 0.
- [ ] **AKW.3 (Selbstmeldung)** — Liegen zwischen zwei Runden mehr als 3 × Intervall Wanduhrzeit, meldet
      die Wache beim Weiterlaufen `*** Wache hat N s ausgesetzt ***` als Alarm, in Konsole **und**
      Mitschrift.
- [ ] **AKW.4 (Frist)** — Jeder Aufruf von `log.sh` hat eine Frist (Empfehlung 20 s). Läuft sie ab:
      Alarm `*** Mitschnitt nicht erreichbar ***`, die Runde läuft weiter.
- [ ] **AKW.5 (Gegenproben)** — (i) eine `WATCHLOG_QUELLE`, die 200 s schläft ⇒ AKW.4 kommt an;
      (ii) Herzschlag künstlich 200 s alt ⇒ `--pruefe` Exit 1 mit Meldung; (iii) Wache 100 s mit `SIGSTOP`
      angehalten bei Intervall 30, dann `SIGCONT` ⇒ AKW.3 kommt an. Je Probe: Meldung in Konsole und
      Mitschrift gesehen, Exit-Code genannt.
- [ ] **AKB.1 (F3, Prüfung)** — Eine Guardrail-Stufe stellt die `*_set`-Endpunkte aus dem Dispatch von
      `http.cpp` gegen die Setter, die der Backup-Import der PWA aufruft, und meldet jeden Endpunkt, der
      weder dort noch in der Ausnahmeliste steht, mit Namen und **Exit ≠ 0**. Sie gibt die Zahl der
      betrachteten Endpunkte aus; **zähl nach** gegen `grep -c` über das Dispatch-Muster (Stand `159c9d0`:
      59) und gegen die Zahl der im Import gefundenen Setter.
- [ ] **AKB.2 (Ausnahmeliste)** — eigene Datei unter `tools/checks/`, je Eintrag Endpunkt und Grund in
      einer Zeile. Ein Eintrag ohne Grund oder für einen Endpunkt, den es nicht gibt ⇒ FEHL.
- [ ] **AKB.3 (Gegenprobe)** — In einer Kopie von `app.js` einen Setter aus dem Import entfernt ⇒ FEHL mit
      Endpunktnamen; ein erfundener Eintrag in der Ausnahmeliste ⇒ FEHL. Meldung angekommen, Exit ≠ 0.
- [ ] **AKD.1 (B8, Zählung)** — Ein Werkzeug listet mit Datei:Zeile alle Kandidaten für sichtbaren Text:
      Zeichenkettenliterale in `app.js` ausserhalb der i18n-Tabellen, Textknoten und die Attribute
      `title`, `aria-label`, `placeholder`, `alt` in `index.html` ohne `data-i18n*`. Der Lead ordnet die
      Kandidaten einmal zu: **deutscher UI-Text** oder **Ausnahme mit Grund** (Ausnahmeliste). Die Zahl
      der deutschen UI-Texte steht sofort in `BEFUNDE.md` bei B8. *Gegenprobe:* ein eingefügter neuer
      Text ⇒ als nicht zugeordnet gemeldet, Exit ≠ 0.
- [ ] **AKG.0 (Grundlinien G0)** — vor S1, je sofort in `BEFUNDE.md`: (1) **A15 vorher** — Befehle im
      Empfangs-Normalpfad laut Disassembly, F103 **und** F411, aus dem Stand `release/3.2.32-3.2.33-1.4.97`
      (`design.md` §2.3); (2) **Brückenlast vorher** — Setter-Burst 14 × Wetterort mit Rückstellung (N-5):
      `rx`-Höchststand, `d`, `o`, Δ`v`; (3) **ESP-RAM vorher** — 30 Heap-Zeilen (30 min, lesend):
      kleinstes `free`, kleinstes `max`, `ovr`; (4) **längste Zeile** in `/api/stm32_log` und ob sie `~`
      trägt (lesend).
- [ ] **AKG.1 (Befunde)** — `BEFUNDE.md` führt die neuen Befunde dieser Spec als eigene Zeilen: die
      Marker-Weissfarbe (§2 (a)), `dcf77_enabled` (§2 (b)), Legacy `rsteeprom2` (§8), die zweite Legacy-Stelle
      von C19 (§9), den 120er-Ring (§6), die Rechnung zum Netzwerk-Import (§10), und an jedem Eintrag
      dieses Pakets „Träger: Paket 2026-10-13". *Instrument:* S10 und S11 grün.

### Schritt S1 — A15 (UART-ISR)

- [ ] **AKI.1 (Gleiches Verhalten)** — Prüfstand **a15** betreibt die alte und die neue ISR gegen dieselbe
      Registerattrappe (`SR`, `CR1`, `DR` mit Lesesemantik: `DR` lesen löscht RXNE und ORE; TXE nur
      lesbar; TXEIE in `CR1`) über mindestens 10'000 Zufallsfolgen und die Randfälle „Ring voll", „ORE
      gesetzt", „TXE gesetzt, TXEIE aus", „RXNE und TXE zugleich", „Sendering leer". Verglichen werden
      Empfangsring, `uart_rxdrops`, `uart_rxore`, `uart_rxmax`, gesendete Zeichen und TXEIE: **0
      Abweichungen**. Sabotagen „TXEIE nicht geprüft" und „ORE nach dem Lesen von DR geprüft" ⇒ je FEHL.
      Fallzahl gemeldet.
- [ ] **AKI.2 (Messung, beide Plattformen)** — Im Empfangs-Normalpfad (RXNE, kein ORE, Ring nicht voll,
      kein TXE) steht nachher **kein** `bl` mehr, und die Befehlszahl ist auf F103 **und** F411 kleiner als
      in G0. Beide Zahlenpaare sofort in `BEFUNDE.md`.
- [ ] **AKI.3 (Gerät G1, schreibend, rücknehmbar, N-5)** — Derselbe Setter-Burst wie in G0, danach
      zurückgestellt: `d=0`, `o=0`, Δ`v` = 0, `rx`-Höchststand neben dem Wert aus G0. Danach 60 min lesend:
      kein Neustart, keine Exception, kein Anstieg von `o`; der Mitschnitt (Log-UART-Instanz derselben
      Vorlage) läuft lückenlos.

### Schritt S2 — B55-STM, A56, A80, A70

- [ ] **AKL.1 (B55, STM)** — Nach `LDR_MIN_VALUE_RPC_VAR` bzw. `LDR_MAX_VALUE_RPC_VAR` sendet der STM
      **nach** der Wertmeldung genau eine Zeile `RPCR xx r` (xx = RPC-Index zweistellig hex; r = `0`
      übernommen, `1` abgewiesen, `2` Messung fehlgeschlagen), ohne Quittung (`design.md` §6.1). Prüfstand
      **l55**: alle drei Ergebnisse für Minimum und Maximum, dazu „gleicher Wert gemessen" ⇒ `0`.
      Gegenprobe gegen 3.2.32 ⇒ FEHL.
- [ ] **AKA.1 (A56)** — Die Ambilight-Power liegt im EEPROM nach N-1 (Empfehlung: Marke und Wert hinter
      dem bisherigen Ende, ohne Layoutversion; `design.md` §3). Jeder Wechsel über
      `display_set_ambilight_power()` schreibt; ein unveränderter Wert schreibt nicht. Beim Start gilt der
      gespeicherte Wert; ohne gültige Marke oder mit einem anderen Wert als 0/1 gilt „an". Nach einem
      EEPROM-Reset gilt „an". Prüfstand **a56** (Lesen, Schreiben, Marke fehlt, Wert 2, Reset, Rückflash
      simuliert: alter Stand liest das Layout unverändert). Gegenprobe gegen 3.2.32 ⇒ FEHL. Bestandsliste
      aller Schaltstellen per `grep`.
- [ ] **AKA.2 (A56, Last)** — Kein EEPROM-Schreiben aus einer ISR oder einem Animationspfad; je Wechsel
      höchstens ein Seitenschreiben (`eep a=… n=…` im Mitschnitt, rund 15 ms).
- [ ] **AKC.1 (A80)** — Kommt ein Icon bei ausgeschaltetem Display vollständig an, bleibt
      `do_display_icon` 0, `show_icon_stop_time` unverändert, und es erscheint `icon: verworfen, display
      aus`. Geht das Display während eines Icons aus, endet das Icon sofort, und das nächste Icon beginnt
      von vorn (Zustand von `display_icon()` zurückgesetzt). Prüfstand **a80**; in keinem Fall erscheint
      `icon_freeze: ENTER`. Gegenprobe gegen 3.2.32 ⇒ FEHL.
- [ ] **AKN.1 (A70 (c))** — Startet der IP-Lauftext mit vorgemerktem Aus, gilt zusätzlich eine Frist:
      Laufzeit des Lauftexts aus Länge, `TICKER_COLS` und `ticker_deceleration` plus 10 s
      (`design.md` §4.1). Kommt der Restore nicht, schaltet die Frist aus.
- [ ] **AKN.2 (A70 (d))** — Kommt das erste `TIME` bei ungültiger RTC, während der IP-Lauftext aussteht
      oder läuft, wird das Aus vorgemerkt wie beim Start (nach dem IP-Lauftext, Frist aus AKN.1); sonst
      wird sofort geschaltet wie heute.
- [ ] **AKN.3 (A70 (e))** — Startet ein anderer Ticker, während das Aus auf das Ende des IP-Lauftexts
      wartet, wird das Aus **vor** dem Start des Tickers angewandt; der Ticker erscheint dann nicht.
      Bestandsliste aller Tickerstarts in `main.c` per `grep display_set_ticker`.
- [ ] **AKN.4 (L399)** — Übernimmt der DCF77-Zweig die erste Zeit bei `night_wm == 0xFFFF`, wird wie beim
      ersten `TIME` ausgewertet (`night_apply (0xFFFF, 1)`, nur ein Aus); danach greift die
      Sprungerkennung.
- [ ] **AKN.5 (Invariante)** — Die Restore-Bedingung um `pending_ticker_restore` (`main.c:4857-4861`,
      vier Teilbedingungen, dazu `! display_clock_flag`) ist unverändert; `git diff` zeigt in diesen Zeilen
      keine Änderung, der Innenzweig nur den Austausch des Vergleichs gegen den Merker aus AKN.1. Prüfstand
      **a70** für (c), (d), (e), L399, je Gegenprobe gegen 3.2.32 FEHL; **a69** weiter OK.
- [ ] **AKA.3 (Gerät G2, A56, schreibend, rücknehmbar, N-5)** — Ambilight-Power aus über die PWA,
      `/api/power_status` zeigt aus, im Mitschnitt das Seitenschreiben; STM-Reset über
      `maintenance_reset_stm32` (Freigabe erteilt); nach `info: configuration sent` meldet
      `/api/power_status` Ambilight **aus**. Danach zurück auf den Vorher-Wert, lesend geprüft.
- [ ] **AKC.2 (Gerät G2, A80, schreibend, rücknehmbar, N-5)** — Display aus über die PWA, dann
      `/api/weather_get_now`: im Mitschnitt `icon: verworfen, display aus`, kein `icon_freeze: ENTER`, kein
      Lauftext. Display wieder an, `diag` lückenlos. Das Display ist dabei kurz dunkel.
- [ ] **AKN.6 (Gerät G2, lesend)** — Die nächsten Timer der Nacht schalten wie bisher (Aus-Timer am Abend,
      Ein-Timer am Morgen, je `Found Timer:`). Fall (e) am Gerät nur, wenn N-5 den Reset im Aus-Fenster
      freigibt; sonst bleibt er beim Prüfstand und wird so berichtet.

### Schritt E1 — HTTP-Schicht (C6 Stufe 1, C6v, C6b, C46, C19)

- [ ] **AKH.1 (POST)** — Für `maintenance_format_fs`, `maintenance_reset_eeprom`,
      `maintenance_reset_stm32`, `fs_remove` und den Endpunkt aus AKS.2 nimmt der ESP POST an (Parameter in
      der URL, Rumpf leer), liest dabei die Kopfzeilen (`User-Agent`, `Sec-Fetch-Dest`, `Sec-Fetch-Site`)
      und antwortet wie bisher. Ein POST auf einen Pfad, der weder Upload noch einer dieser ist, bekommt
      404 (heute: gar keine Antwort).
- [ ] **AKH.2 (Herkunft)** — Abgewiesen mit 403 und `{"ok":false,"error":"embedded-subresource"}` bzw.
      `"cross-site"` wird eine Anfrage mit eingebettetem `Sec-Fetch-Dest` (bestehende Liste) **oder** mit
      `Sec-Fetch-Site` ungleich `same-origin` und `none` — für die fünf Endpunkte aus AKH.1 und die
      Legacy-Aktionen `/fs?action=format`, `/fs?action=remove` und `/?action=rsteeprom2` (N-4). Ohne Kopfzeile
      wie bisher zugelassen (curl, ältere Browser). Logzeile `- abgewiesen: <was> als …` wie bisher.
- [ ] **AKH.3 (Übergang, N-3)** — GET auf die vier Wartungsendpunkte bleibt in E1 zugelassen, mit der
      Herkunftsprüfung aus AKH.2 und einer gedrosselten Zeile `- veraltet: GET <pfad>`.
- [ ] **AKH.4 (Prüfstand c6)** — Methode × Pfad × Kopfzeilen (ohne, `image`, `document`, `same-origin`,
      `cross-site`, `same-site`, `none`) für alle Pfade aus AKH.1 und AKH.2: ausgeführt genau in den
      erlaubten Fällen. Gegenprobe gegen 3.2.33 ⇒ FEHL (`fs_remove` mit `image` wird dort ausgeführt).
      Fallzahl gemeldet.
- [ ] **AKS.1 (C6b)** — `/api/eeprom_settings` liefert `key` und `ap_key` als **leere** Zeichenkette und
      zusätzlich `key_set` und `ap_key_set` (true/false); alle übrigen Felder unverändert.
- [ ] **AKS.2 (C6b)** — Neuer Endpunkt `/api/network_secrets`, nur POST, Herkunftsprüfung aus AKH.2,
      `Cache-Control: no-store`, liefert `{"ok":true,"key":…,"ap_key":…}`; GET ⇒ 405.
- [ ] **AKS.3 (Prüfstand)** — `eeprom_settings` enthält den Wächterschlüssel des Prüfstands nicht,
      `network_secrets` liefert ihn; Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [ ] **AKQ.1 (C46)** — Ist die Query (roh, so wie sie ankommt) bei `/api/…` länger als 255 Zeichen,
      antwortet der ESP `{"ok":false,"error":9,"detail":"query too long"}` **ohne** den Handler auszuführen,
      für GET und POST, und schreibt `- abgewiesen: query <n> Zeichen`. Genau 255 ⇒ ausgeführt. Legacy-Pfade
      unverändert. Prüfstand mit 254, 255, 256, 1'000 Zeichen; Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [ ] **AKU.1 (C19, nur L311)** — **Beide** Legacy-Zwillinge (§9) weisen einen leeren Wert und einen Wert
      über 63 Byte ab, zeigen die Abweisung auf der Seite, und Host bzw. Pfad bleiben unverändert. Der
      übrige Legacy-Zweig bleibt unverändert. Prüfstand; Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [ ] **AKH.5 (Gerät G3, Freigabe STM-Reset erteilt)** — **Nur** `maintenance_reset_stm32`: POST ohne
      Kopfzeile ⇒ 200 und STM-Neustart (`Version:` im Mitschnitt); POST mit `Sec-Fetch-Site: cross-site` ⇒
      403, kein Neustart; GET mit `Sec-Fetch-Dest: image` ⇒ 403, kein Neustart. Jeder andere Pfad aus AKH.1
      und AKH.2 **nur am Prüfstand** (`design.md` §17.3).
- [ ] **AKS.4 (Gerät G3, lesend)** — `/api/eeprom_settings` enthält keinen Schlüssel, `key_set` ist true.
- [ ] **AKQ.2 (Gerät G3, lesend)** — `/api/device_ready?x=` mit 300 Zeichen ⇒ Kennung 9; mit 240 Zeichen ⇒
      die gewohnte Antwort.

### Schritt E2 — Brücke und Netz (B55-ESP, C8, A79, C59)

- [ ] **AKL.2 (B55, ESP)** — `ldr_min_set` und `ldr_max_set` warten bis 2'000 ms auf `RPCR` und lesen
      dabei die Brücke über **dieselbe** Funktion wie `loop()` (Quittungen gehen weiter hinaus; kein
      verschachteltes Lesen, eine Sperre verhindert es). Antwort: `0` ⇒ `{"ok":true,"value":n}`; `1` ⇒
      Kennung 10 (abgewiesen); `2` ⇒ Kennung 7 (Aktion fehlgeschlagen); keine Antwort ⇒ Kennung 11 und
      die gedrosselte Zeile `- rpc keine Antwort #n idx=xx`. Prüfstand **l55e** mit Brückenattrappe
      (Antwort, keine Antwort, verstümmelte Zeile, dazwischen `var`-Zeilen, die quittiert werden
      müssen); Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [ ] **AKK.1 (C8)** — Kommandopuffer 260 Byte: Eine Logzeile mit 255 Zeichen erreicht den ESP ungekappt.
      Ring nach N-2 (Empfehlung: Folgeeinträge, RAM-neutral, `design.md` §7). Prüfstand mit 119, 120, 121,
      255 Zeichen; Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [ ] **AKE.1 (A79)** — `Serial.setRxBufferSize (1024)` steht **vor** `Serial.begin`, und keine neue
      Ausgabe steht vor der `CAP`-Zeile (`ESP-uclock.ino:166-171`). *Instrument:* statische Prüfung des
      Leads, Review.
- [ ] **AKT.1 (C59)** — Jede Anfrage trägt eine Zufallszahl von 8 Byte im Transmit-Feld; angenommen wird
      nur, solange eine Anfrage offen ist (Frist 5 s) und das Originate-Feld genau diese Zahl trägt; danach
      ist die Anfrage geschlossen. Neue Gründe in `- ntp verworfen #n <grund>`: `ungefragt`, `originate`.
      Prüfstand **ntp** erweitert; Gegenprobe gegen 3.2.33 ⇒ FEHL.
- [ ] **AKM.2 (RAM)** — Die Build-Ausgabe „Globale Variablen" vor und nach jedem E2-Task sofort in
      `BEFUNDE.md`. Am Gerät (G4) 30 Heap-Zeilen nach dem Einspielen gegen G0: `free` sinkt um höchstens
      1'200 Byte, das kleinste `max` liegt höchstens 1'200 Byte unter dem kleinsten `max` aus G0.
- [ ] **AKE.2 (Gerät G4)** — Bei drei ESP-Starts — dem OTA und zwei STM-Resets, die den ESP mit neu
      starten (L421) — bleibt `ovr=0`, und in den ersten 2 Minuten erscheint kein `- ovr #`.
- [ ] **AKK.2 (Gerät G4, auslösend, folgenlos, N-5)** — Nach `/api/weather_get_forecast` steht die Zeile
      `info: weather forecast = …` in `/api/stm32_log` vollständig, gleich dem Mitschnitt.
- [ ] **AKT.2 (Gerät G4, lesend)** — Zwei Stunden: jede stündliche Netzzeit übernommen (`cmd: set time
      to`), 0 × `ntp verworfen`.

### Schritt P1 — PWA (C6, C6b, C46, B55)

- [ ] **AKH.6 (C6, PWA)** — Die vier Wartungsaufrufe und `network_secrets` gehen mit `method: "POST"`
      hinaus. Prüfstand mit Mock: Methode je Aufruf gezählt, 0 GET auf diese Pfade.
- [ ] **AKS.5 (C6b, PWA)** — Der Backup-Export holt die Schlüssel über `network_secrets`; scheitert der
      Abruf, bricht der Export mit eigener Meldung ab, wie heute bei fehlenden `eeprom_settings`. Bei
      gesetztem Schlüssel ist `wifi_key` im Export nicht leer.
- [ ] **AKQ.3 (C46, PWA)** — Vor dem Senden: Ist die kodierte Query länger als 255 Zeichen, geht kein
      Request hinaus, und die Meldung nennt Länge und Grenze; Kennung 9 vom Gerät erzeugt dieselbe Meldung.
      Der Backup-Import sendet `eeprom_settings_set` **je Feld** (jeweils mit `boot_as_ap`). Prüfstand: SSID
      31 Byte und Schlüssel 63 Byte aus Sonderzeichen ⇒ alle Felder gesetzt, kein Request über 255.
- [ ] **AKL.3 (B55, PWA)** — Kennung 10 ⇒ „LDR-Minimum nicht übernommen: Der Messwert liegt nicht unter dem
      Maximum." (Maximum sinngemäss); Kennung 7 ⇒ „Messung fehlgeschlagen"; Kennung 11 ⇒ „Keine
      Rückmeldung der Uhr — ob der Wert übernommen wurde, ist offen."; `ok` ⇒ wie heute. DE und EN; S8
      grün. Prüfstand mit Mock je Kennung.
- [ ] **AKG.5 (Gerät G5)** — `install-app.sh --check` vollständig, `check-pwa.sh` ohne Fehler.
- [ ] **AKL.4 (Gerät G5, schreibend, rücknehmbar, N-5)** — Über die PWA: Maximum per Zahl auf 1, „Minimum
      übernehmen" ⇒ Meldung „nicht übernommen", im Mitschnitt `ldr: minimum abgewiesen`; Maximum zurück
      auf 4095. Minimum per Zahl auf 4094, „Maximum übernehmen" ⇒ „nicht übernommen"; Minimum zurück auf 0.
      Dann „Minimum übernehmen" bei 0/4095 ⇒ „gespeichert"; Minimum per Zahl zurück auf 0. Endstand 0/4095
      lesend geprüft; Δ`v` = 0 über alle Proben.
- [ ] **AKS.6 (Gerät G5, lesend)** — Backup-Export im Browser: Die Datei enthält `wifi_key`; im Bericht
      steht nur der Vergleich der Prüfsumme mit dem Abzug aus `snapshot-device.sh`, nie der Klartext.

### Schritt E3 — C6 Stufe 2

- [ ] **AKH.7** — GET auf die vier Wartungsendpunkte und `network_secrets` ⇒ 405 und
      `{"ok":false,"error":"method"}`, ohne Ausführung; die Zeile `- veraltet: GET` entfällt. Prüfstand c6
      umgestellt. Gerät (G6): GET `maintenance_reset_stm32` ⇒ 405, kein Neustart im Mitschnitt.
      Voraussetzung: Die Werkzeuge nutzen POST (AKZ.11).

### Schritt P2 — B48 und B8

- [ ] **AKF.1 (B48)** — `reportInvalidInput()` und `readTimeInputOrReport()` setzen `aria-invalid="true"`
      am Feld, legen direkt unter dem Feld eine Fehlerzeile an oder aktualisieren sie, und ergänzen
      `aria-describedby`, ohne vorhandene Kennungen darin zu verlieren; die nächste Eingabe im Feld
      entfernt beides. Das Statusbanner bleibt. Prüfstand in der Vorschau: für jeden Aufrufer
      (Bestandsliste per `grep`) Attribut und Zeile nach dem Fehler vorhanden, nach der Korrektur weg.
- [ ] **AKF.2 (iPhone, Nutzer)** — Der Nutzer prüft an drei Feldern (Datumsformat, Uhrzeit,
      WLAN-Schlüssel zu lang), dass die Fehlerzeile bei offener Tastatur sichtbar ist. Ohne seine Rückmeldung
      bleibt AKF.2 offen und wird so berichtet.
- [ ] **AKD.2 (B8)** — Das Werkzeug aus AKD.1 meldet **0** deutsche UI-Texte ausserhalb der Tabellen,
      ausser der Ausnahmeliste; DE und EN haben dieselben Schlüssel; S8 grün. Das Werkzeug ist
      Guardrail-Stufe. *Gegenprobe:* neuer deutscher Text in `app.js` ⇒ FEHL, Exit ≠ 0.
- [ ] **AKD.3 (Gerät G7)** — `check-pwa.sh` ohne Fehler; mit Sprache Englisch zeigen fünf Module
      (Hauptmodul, Display, Netzwerk, Wartung, Backup) keinen deutschen Text (Bildschirmfoto der Vorschau
      und am Gerät).

### Flash, Matrix, RAM

- [ ] **AKM.1 (Flash-Gate)** — Nach **jedem** STM-Task Teilbuild F103 WC12h SK6812-RGBW **und** F411:
      Byte frei und Differenz zum Stand davor, **sofort** in `BEFUNDE.md`. Unter 1'024 Byte (F103): anhalten
      und vorlegen, keine Einsparung ohne Rückfrage, **kein Rollout**. Liegt der F103-Teilbuild unter
      1'244 Byte (1'024 plus dem grössten gemessenen Abstand zur engsten Variante, 220 Byte), läuft vor dem
      nächsten Task ein Matrix-Vorablauf.
- [ ] **AKM.3 (Matrix)** — Vor dem Release-Build von S1 und von S2, **nach** der Versionsanhebung:
      `make matrix` 72/72, alle ≥ 1'024 Byte, engste Variante mit Byte frei — **sofort** in `BEFUNDE.md`.
      Darunter: kein Build.

### Für alle Schritte

- [ ] **AKZ.1** — `./tools/guardrails.sh` nach jedem Task mit Exit 0; Ergebnis im Taskbericht.
- [ ] **AKZ.2** — `guardrails.sh --full` vor jedem Release mit unverändertem Warnungsbestand; Ergebnis
      **sofort** in `BEFUNDE.md`.
- [ ] **AKZ.3 (Release-Inhalt)** — Vor jedem Release-Build zeigt `git diff --name-only <letztes Tag>` samt
      Arbeitsbaum Produktänderungen nur in der angehobenen Komponente und nur aus den Tasks dieses
      Releases; die Ausgabe steht im Messprotokoll.
- [ ] **AKZ.4** — Smoketest nach jedem Einspielen, Ergebnis **sofort** in `BEFUNDE.md`; nach jedem
      ESP-Update samt Update-Quelle (DIR-009) und `install-app.sh --check` (DIR-017). **Der Smoketest ist
      nicht der Test** (DIR-012).
- [ ] **AKZ.5 (Logwache)** — läuft bei jedem Gerätelauf, vor dem Einspielen gestartet, nach Änderungen
      neu gestartet (DIR-013); am Ende jedes Gerätelaufs steht `watch-log.sh --pruefe` mit Exit 0 im
      Bericht. Neue Zeilen kennt die Wache je mit eingespielter Gegenprobe: `icon: verworfen` (Hinweis),
      `icon_freeze: ENTER` (Alarm ab S2), `- rpc keine Antwort` (Alarm), `- veraltet: GET` (Hinweis),
      `- abgewiesen: query` (Hinweis), `ntp verworfen … ungefragt|originate` (Alarm wie bisher).
- [ ] **AKZ.6** — `pwa-tester` Phasen 0–4 und 9 nach G5 (T1) und nach G7 (T2), mit nachgeführtem
      Testplan. Unvollständig ⇒ nicht abgenommen.
- [ ] **AKZ.7** — Jedes Release sofort committet, getaggt, **gepusht** (DIR-011); `CHANGELOG.md` schreibt
      der Lead.
- [ ] **AKZ.8** — Einspielzeilen mit Reihenfolge je Release, fertig zum Einfügen; im Messprotokoll, wer
      wann eingespielt hat.
- [ ] **AKZ.9** — Kodierung **und** Zeilenende jeder Datei vor dem Patch festgestellt und im Taskbericht
      genannt (DIR-015), darunter `uart-driver.h`, `main.c`, `display.c`, `eeprom-data.h`, `ldr.c`,
      `ESP-uclock.ino` (**gemischt**), `http.cpp` (UTF-8, eine CRLF-Zeile), `ntp.cpp`, `app.js`,
      `en.json`, `index.html`, `styles.css`.
- [ ] **AKZ.10** — Keine Versionsnummer in lebender Doku (S9); jede zitierte Kennung im Katalog (S11).
- [ ] **AKZ.11 (Werkzeuge, C6)** — Nach G3 und vor E3 nutzen `flash-stm.sh` (Reset), `smoke-device.sh`
      (Herkunftsprobe) und `tools/preview/server.py` POST; `no-danger.py` weist die Gefahrenendpunkte auch
      per POST ab (Probe in `tools/checks/hooks.sh`).

---

## Nicht Teil dieser Änderung

| Ausgeschlossen | Grund |
|---|---|
| **F4** (Abbild des STM-EEPROM) | Entscheidung des Nutzers: eigenes Folgepaket mit eigener Spec |
| **B53** („klein bis mittel"), **A19** („eher klein als mittel"), **A63** (Beobachtung) | Nicht „mittel" und ohne offene Nutzerentscheidung; bleiben in der ToDo-Liste |
| **A77**, **A78** (reine Korrekturen, klein) | Nicht beauftragt. A78 ist ein Kommentar in `main.c`; wer dort für A70/A80 patcht, fasst ihn **nicht** an |
| **A70 (a)** Gleichstand, **(b)** Rücksprung über 3,5 Tage | Entscheidung des Nutzers: ohne Fix |
| **Display-Power im EEPROM** | Nicht entschieden; A56 betrifft nur das Ambilight. A69 regelt das Nachtfenster |
| **L343 / C32** (Index 9) | Eigener Befund |
| **Marker-Weissfarbe** (`display.c:3435`), **`dcf77_enabled` am Ambilight** (`main.c:1789`, `:4460`) | Neu gefunden, nur eingetragen (AKG.1); kein stilles Mitkorrigieren in `display.c`/`main.c` |
| **Übriger Legacy-Zweig** (roher `atoi`, eigene Klemmungen, C19 ausser L311) | Entscheidung des Nutzers: bleibt bewusst stehen |
| **Legacy auf POST** | Empfehlung N-4: Legacy bleibt GET mit Herkunftsprüfung (Stabilitäts-Referenz) |
| **POST für Update-Auslöser, `ir_code_set`, `learn_ir`, `test_display`, `eeprom_settings_set`** | N-4; Werkzeuge rufen sie per GET (DIR-010, AKZ.8 im Vorpaket) |
| **C46 für Legacy-Pfade** | Befund und Entscheidung nennen `/api/…` |
| **Ringzeilen mit 255 Zeichen** | N-2: 4'320 Byte RAM |
| **Legacy-LDR-Seite** (`http.cpp:4317`, `:4322`) | Bleibt „anstossen und neu laden" |
| **Icon gar nicht erst anfordern, wenn das Display aus ist** | Verwerfen am Empfang genügt (eine Stelle); Anfordern sparte nur Brückenlast |
| **Messbau mit Zyklenzähler für A15 auf der Uhr** | N-7; statische Messung beider Plattformen genügt |
| **Fehlerfälle am Gerät** für C6/C6v (Format, EEPROM-Reset, Löschen), C19 (leerer Host), C59, A70 (c), (d), L399 | Gefährlich oder nicht herstellbar; Prüfstände (`design.md` §17.3) |
| **Frühere Specs**, `REVIEW*.md` | Momentaufnahmen (DIR-006) |

---

## Betroffene Laufzeiten

- [ ] **STM32** (`src/**`) — S1: `uart/uart-driver.h`; S2: `main.c`, `display/display.c` (+ `.h`),
      `eeprom/eeprom-data.h`, `ldr/ldr.c` (+ `.h`), ggf. `vars/vars.c`; je ein Flash
- [ ] **ESP8266** — E1: `http.cpp`; E2: `ESP-uclock.ino`, `http.cpp`, `vars.cpp` (+ `.h`), `ntp.cpp`; E3:
      `http.cpp`; je ein OTA
- [ ] **PWA** — P1: `app.js`, `i18n/en.json`; P2: `app.js`, `i18n/en.json`, `index.html`, `styles.css`; je
      ein LittleFS-Upload
- [ ] **Werkzeug** (`tools/**`, `.claude/**`) — Logwache, Stop-Hook, F3-Stufe, B8-Werkzeug, Prüfstände,
      `flash-stm.sh`, `smoke-device.sh`, `snapshot-device.sh`, `preview/server.py`, `hooks/no-danger.py`
- [ ] **Dokumentation** — `BEFUNDE.md`, `TESTPLAN-PWA.md` (`doc-writer`); `CLAUDE.md` (Gefahrentabelle:
      POST), `CHANGELOG.md` (Lead)
- [ ] **Build/Release** — sieben Releases (S1, S2, E1, E2, P1, E3, P2), Matrix vor S1 und S2, je Commit,
      Tag, Push

---

## Entscheidungen

**Entschieden vom Nutzer am 13.10.2026** (an den Einträgen in `BEFUNDE.md`):

- **A56:** im EEPROM speichern — die Ambilight-Power übersteht den Neustart (neuer Platz, Layout und
  Migration beachten, Flash F103).
- **A70:** alle vier Fälle beheben — (c), (d), (e), L399; (a) und (b) ohne Fix.
- **A79:** Puffer vergrössern (`setRxBufferSize` vor `Serial.begin`, 1024 Byte), danach belegen, dass
  `ovr` beim Start 0 bleibt.
- **A80:** Ein Icon bei ausgeschaltetem Display wird verworfen, nicht vorgemerkt; der Ticker-Restore
  wartet nicht darauf; Restore-Bedingung nicht vereinfachen.
- **C8:** Brücke aufweiten — Kommandopuffer des ESP auf 256 Byte, RAM-Kosten messen. **Präzisiert durch
  N-2** (260 Byte und der Ring).
- **B55:** Die API wartet das Ergebnis des STM ab und meldet `ok`/abgewiesen (ESP und PWA, Firmware
  zuerst). **Am Code ergänzt:** Dafür braucht es eine Rückmeldung des STM (Problem §5).
- **C19:** nur der L311-Teil; der übrige Legacy-Zweig bleibt bewusst stehen.
- **F4:** eigenes Folgepaket.
- **Freigaben:** Updates einspielen und STM-Reset.

## Offene Entscheidungen

Je mit Empfehlung; Begründung in `design.md`.

- **N-1 — A56, Speicherort.** **(a) Empfehlung:** zwei Byte hinter dem bisherigen Ende (Marke und Wert),
  **ohne** Anhebung der Layoutversion — ein Rückflash auf eine ältere Firmware lässt alle Einstellungen
  stehen. (b) Layoutversion anheben (3.0): üblicher Weg, aber jeder Rückflash setzt danach **alle**
  Einstellungen auf den Werkszustand (Problem §2). `design.md` §3.
- **N-2 — C8, Ring.** Ein Kommandopuffer von 256 Byte allein ändert in `/api/stm32_log` nichts, weil der
  Ring je Zeile 120 Zeichen hält. **(a) Empfehlung:** Puffer 260 Byte und lange Zeilen im Ring als
  Folgeeinträge (RAM-neutral, weniger Rückschau bei langen Zeilen). (b) Nur der Puffer, wie entschieden —
  sichtbar ändert sich nichts. (c) Ringzeilen auf 255 Zeichen, +4'320 Byte RAM — nicht empfohlen.
  `design.md` §7.
- **N-3 — C6, zwei Stufen.** **(a) Empfehlung:** E1 nimmt POST **und** GET (GET mit Herkunftsprüfung und
  Hinweiszeile), P1 stellt die PWA um, E3 weist GET mit 405 ab — kein Zeitfenster, in dem die eingespielte
  PWA nicht warten kann; kostet einen OTA mehr. (b) Eine Stufe: Zwischen ESP-Update und PWA-Upload
  scheitern Wartungsaktionen der alten PWA (sichtbar, mit 405). `design.md` §8.4.
- **N-4 — C6, Umfang.** **Empfehlung:** POST für die vier Wartungsendpunkte und den neuen
  Schlüssel-Endpunkt; Herkunft zusätzlich über `Sec-Fetch-Site` für diese **und** für die
  Legacy-Zerstörer `/fs?action=format`, `/fs?action=remove`, `/?action=rsteeprom2` (Legacy bleibt GET).
  Update-Auslöser, `ir_code_set`, `learn_ir`, `test_display` bleiben unverändert. `design.md` §8.
- **N-5 — Freigabe der schreibenden Gerätenachweise.** Alle folgenlos rücknehmbar, Liste in
  `design.md` §17.2: Setter-Burst 14 × Wetterort (G0, G1); Ambilight aus und zurück um einen STM-Reset
  (G2); Display kurz aus, Wetterabruf, Display an (G2); Wettervorhersage-Abruf (G4); LDR-Proben mit
  Rückstellung (G5); `pwa-tester` mit S-Fällen (T1, T2). **Optional:** STM-Reset **im Aus-Fenster** mit
  einem Ticker für A70 (e) (G2) — ist das von der Freigabe „STM-Reset" gedeckt?
- **N-6 — Wenn das Flash-Gate in S2 reisst.** Spielraum an der engsten Variante 412 Byte; die vier Tasks
  sind auf 200–480 Byte geschätzt, mit dem gemessenen Faktor bis 1'400. **Empfehlung:** Reihenfolge der
  Tasks B55-STM, A56, A80, A70; reisst das Gate, wird **A70** in ein eigenes STM-Release mit
  vorgeschalteter Flash-Analyse verschoben, ohne neue Rückfrage — die übrigen Punkte bleiben. Sonst gilt:
  anhalten und vorlegen. `design.md` §1.2.
- **N-7 — A15, Messung.** **Empfehlung:** Befehlszahl aus dem Disassembly für F103 und F411 als bindende
  Messung, dazu am Gerät (F411) die Brückenwerte unter Last als Regression. Ein Messbau mit Zyklenzähler
  auf der produktiven Uhr ist nicht empfohlen (eigener Flash, Diagnosecode). `design.md` §2.3.
