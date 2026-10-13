# Design — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (A15, A56, A70, A79, A80, A81, A82, B8, B48, B55, C6, C6b, C6v, C8, C19, C46, C59, C60, E36, F3)

**Erstellt:** 2026-10-13, Stand `159c9d0`. **Freigegeben** vom Nutzer am 13.10.2026 (Ent-16), samt Ent-1
bis Ent-14 und Ent-17 (`requirements.md`, „Entscheidungen"). **Gegen die Empfehlung entschieden:** Ent-11
(C6 einstufig, §8.4). **Ent-15 (Messbau für A15) ist am 13.10.2026 aufgehoben** (L432, §2.3).
Momentaufnahme (DIR-006).

**Grundlage:** Fundstellen vom `spec-writer` am Code `159c9d0` nachgelesen (`requirements.md`,
„Problem"). Technische Detailentscheide, für die kein Analyse-Agent befragt wurde, sind als **Empfehlung**
gekennzeichnet und begründet; der Umsetzer darf mit Begründung am Code abweichen und meldet es im
Taskbericht.

**Entscheidungsvermerk 13.10.2026.** Ent-1/Ent-9 A56 zwei Byte ohne Layoutversion (§3) · Ent-2 A70 alle
vier Fälle (§4) · Ent-3 A79 (§12) · Ent-4 A80 (§5) · Ent-5/Ent-10 C8 260 Byte und Folgeeinträge (§7) ·
Ent-6 B55 API wartet (§6) · Ent-7 C19 nur L311 (§9) · Ent-8 F4 ausgelagert · **Ent-11 C6 einstufig** —
gegen die Empfehlung (§8.4) · Ent-12 C6-Umfang samt C60 (§8) · Ent-13 alle Gerätenachweise freigegeben,
`caffeinate` (§17) · Ent-14 A70 bei Gate-Riss auslagern (§1.2) · ~~Ent-15 Messbau A15~~ **aufgehoben**
13.10.2026, L432 (§2.3) · Ent-16 freigegeben · Ent-17 A81, A82 in S2 (§3.4, §3.5).

---

## 0. Reihenfolge und Release-Schnitt

### 0.1 Regeln

| Regel | Woher | Umsetzung in `tasks.md` |
|---|---|---|
| Code des **nächsten** Releases erst nach dem **Release-Build** des vorigen | `make release-zip` baut STM, ESP und PWA-Assets zusammen (Lehre Paket 2026-10-09) | S2.1 nach S1.12, E1.1 nach S2.22 (bzw. S3.6), E2.1 nach E1.12, EP.1 nach E2.12, P2.1 nach EP.16 |
| **Release-Build** erst nach dem **Gerätenachweis** des vorigen | R3b | S2.22 nach S1.14 (G1), E1.12 nach S2.24 (G2) bzw. S3.8 (G2b), E2.12 nach E1.14 (G3), EP.16 nach E2.14 (G4), P2.11 nach T1 |
| **Firmware vor PWA** | DIR-002, L241 | B55: S2 → E2 → EP. C6/C6b/C46: ESP-Teil und PWA-Teil im selben Release EP, ESP zuerst eingespielt |
| STM-Release-Build erst nach **grünem Matrix-Lauf** auf dem angehobenen Stand | Ent-18 des Vorpakets | S1.11, S2.21, S3.5 |
| ~~Messbauten sind **keine** Releases~~ | ~~Ent-15~~ — **entfallen**, Ent-15 aufgehoben (L432) | keine Messbauten in diesem Paket |
| Messwerte **sofort** in `BEFUNDE.md` | L424 | jeder messende Task schreibt seine Zeile, bevor der nächste beginnt (§1.4) |
| Spät geplante Gerätetests mit `caffeinate` | DIR-020, Ent-13 | Nachtteil G2, Stundenläufe G1, G4 |

**Ausnahme für P2 (Empfehlung):** B8 ist umfangreich und firmwareunabhängig. Der Lead darf P2.3/P2.4 in
einem Worktree (R6) vorziehen; in den Hauptbaum kommt es erst nach dem Release-Build von EP.

### 0.2 Release-Zuschnitt

| # | Release | Inhalt | Komponente | Nachweis | Warum so geschnitten |
|---|---|---|---|---|---|
| — | **Schritt 0** | E36, F3, B8-Zählung, lokaler STM-Flash (Werkzeug), Grundlinien | Werkzeug, Doku | **G0** | Die Wache trägt jeden Gerätenachweis — sie muss vor G0 halten |
| — | ~~**M0**~~ | ~~Messbau vorher~~ — **entfallen** (Ent-15 aufgehoben, L432) | — | — | — |
| 1 | **S1** | A15 | STM | **G1** | Die ISR liegt unter jedem Brückenpfad und unter dem Mitschnitt — isoliert ist ein Fehler zuzuordnen |
| 2 | **S2** | B55-STM, A56, A81, A80, A82, A70 | STM | **G2** | Getrennte Kennzeichen (`RPCR`, Seitenschreiben bei Ambilight, Marker-Weiss nach Reset, `icon: verworfen`, DCF77-Merker am Prüfstand, `night: …`); A80 und A70 berühren denselben Restore-Bereich und werden zusammen gegen die Invariante geprüft |
| (3) | **S3** | nur bei Gate-Riss: Flash-Analyse, A70 (und A82) | STM | **G2b** | Ent-14 |
| 3 | **E1** | Herkunft für alle zerstörenden Wege (C6-Herkunftsteil, C6v, C60), C46-ESP, C19 | ESP | **G3** | Anfrageverarbeitung; ändert für die PWA nichts — sie ruft `same-origin` und weiter per GET |
| 4 | **E2** | B55-ESP, C8, A79, C59 | ESP | **G4** | Brücke, RAM, Netz; RAM-Messung ohne Überlagerung |
| 5 | **EP** | ESP: nur POST für fünf Endpunkte, C6b · PWA: POST, C6b-Export, C46, B55 · Werkzeuge auf POST im selben Commit | ESP + PWA | **G5** | Ent-11: kein Zeitfenster mit Nachweis oder Release zwischen POST-ESP und POST-PWA — darum **ein** Build, **ein** Tag, Einspielen direkt hintereinander |
| — | **T1** | `pwa-tester` | — | AKZ.6 | EP berührt zwei Komponenten (DIR-012) |
| 6 | **P2** | B48, B8 | PWA | **G6** | grosser Oberflächenumbau getrennt von EP — ein Weissschirm ist eindeutig zuzuordnen |
| — | **T2** | `pwa-tester` | — | AKZ.6 | Abschluss |

**Warum C6b in EP und nicht in E1:** Fehlt der Schlüssel in `eeprom_settings`, exportiert die **alte** PWA
leere Schlüssel. Sässe C6b in E1, wäre dieses Fenster drei Releases lang offen; in EP ist es so kurz wie das
POST-Fenster.

**Warum die Herkunftsprüfung schon in E1:** Sie ändert für die PWA und die Legacy-Seiten nichts (beide
`same-origin`, Werkzeuge ohne Kopfzeile) und schliesst C60 (hoch) früh. In EP kommt nur die Methode dazu.

**Versionen** (erwartet, setzt der `release-engineer`, R4): S1 STM 3.2.33, S2 STM 3.2.34 (S3 3.2.35),
E1 ESP 3.2.34, E2 ESP 3.2.35, EP ESP 3.2.36 **und** PWA 1.4.98 (`CACHE_NAME` mit), P2 PWA 1.4.99.

### 0.3 Einspielen und Zeitfenster

| Nach | Zustand | Folge | Massnahme |
|---|---|---|---|
| S2 | neuer STM sendet `RPCR`, alter ESP | der alte ESP übergeht unbekannte Zeilen | keine |
| E1 | Herkunftsprüfung | für PWA, Legacy und Werkzeuge keine Wirkung | keine |
| E2 | ESP wartet auf `RPCR` | die alte PWA zeigt bei Abweisung ihren Rückfalltext statt „gespeichert" | keine |
| **EP, ESP eingespielt, PWA noch nicht** | POST-only, Schlüssel nicht mehr in `eeprom_settings` | Wartungsaktionen der alten PWA ⇒ 405 (sichtbar); ein Backup-Export enthielte **leere Schlüssel** | **Minuten:** Lead spielt die PWA unmittelbar nach dem OTA ein; dazwischen keine Aktion am Gerät, **kein Backup-Export**. Die Einspielzeile sagt es |

---

## 1. Budgets

### 1.1 Flash F103

| Stand | Teilbuild F103 WC12h SK6812-RGBW | engste Matrix-Variante |
|---|---|---|
| 3.2.32 (Ausgang) | **1'656** (S8b, 13.10.2026) | **1'436** (L416) |

Spielraum an der engsten Variante: **412 Byte**. Abstand Teilbuild–engste Variante im Vorpaket 156–220 Byte.

### 1.2 Planung S1 und S2

| Task | Schätzung | mit Faktor 3 |
|---|---|---|
| A15 | −80…+20 | — |
| B55-STM | +40…+100 | 300 |
| A56 | +60…+120 | 360 |
| A81 | 0 (eine Konstante) | 0 |
| A80 | +20…+60 | 180 |
| A82 | +10…+40 | 120 |
| A70 | +80…+200 | 600 |

1. Geplant wird mit der **Messung je Task** (AKM.1).
2. **Reihenfolge in S2:** B55-STM (Kette zu E2 und EP), A56, A81, A80, A82, A70.
3. **Vorwarnschwelle:** F103-Teilbuild unter 1'244 Byte ⇒ Matrix-Vorablauf vor dem nächsten Task.
4. **Gate-Riss (Ent-14, Ent-17):** Reisst es bei **A82**, wird A82 zurückgestellt und wandert mit A70 nach
   S3. Reisst es bei **A70**, wandert A70 nach S3. S3 beginnt mit einer Flash-Analyse nach dem Muster S3.1
   des Vorpakets (Map des S2-Builds und der engsten Variante; Kandidaten mit Bytes laut Map) und setzt
   verhaltensneutrale Einsparungen nur einzeln gemessen um; eine Einsparung mit Verhaltensänderung nur nach
   Rückfrage. Reisst es vorher: anhalten und vorlegen.

### 1.3 ESP-RAM

Ausgang (L421): Heap frei 9'472, grösster Block 8'736. Gerechnet: A79 +768 Heap, C8 +132 BSS, C59 +12,
B55 +8, Kopfzeile `Sec-Fetch-Site` einige Dutzend — zusammen rund **950 Byte**; Grenze in AKM.2 1'200.
Der Ring bleibt bei 32 × 121 (Ent-10).

### 1.4 Messprotokoll in `BEFUNDE.md` (L424)

Je Release **eine** Befundzeile „Messprotokoll <Release>"; der Lead hängt jede Messung im Augenblick der
Messung an: Zeitpunkt, Instrument, Wert, Differenz, Gate ja/nein. **R3:** Messzeilen schreibt der Lead;
Befundnachführungen der `doc-writer` in eigenen Tasks — nie gleichzeitig.

---

## 2. A15 — UART-ISR

### 2.1 Lösungsweg

`SR` und `CR1` **einmal** lesen, danach nur aus den Kopien entscheiden:

```
sr  = UART_NAME->SR;
cr1 = UART_NAME->CR1;

if ((cr1 & USART_CR1_RXNEIE) && (sr & USART_FLAG_RXNE))   // gleichwertig zu USART_GetITStatus
{
    if (sr & USART_FLAG_ORE) { ...saettigend zaehlen... }  // aus der Kopie VOR dem DR-Lesen
    ch = UART_NAME->DR & 0xFF;                              // loescht RXNE und ORE
    ...Ring wie bisher...
}

if ((cr1 & USART_CR1_TXEIE) && (sr & USART_FLAG_TXE))      // TXE ist immer gesetzt, wenn DR leer
{
    ...wie bisher; USART_SendData durch UART_NAME->DR = ch...
    ...Abschalten: UART_NAME->CR1 &= ~USART_CR1_TXEIE statt USART_ITConfig...
}
```

- **Kein `ClearITPendingBit`:** RXNE löscht das Lesen von `DR`, TXE das Schreiben. Kommentar `:777-787`
  nachführen.
- **Lese-Ändere-Schreibe auf `CR1`:** Der zweite Schreiber ist `uart_putc()` beim Einschalten von TXEIE —
  **Prüfpunkt:** geschieht das unter gesperrtem Interrupt? Im Bericht nennen.
- **Beide Plattformen:** Bitlagen von `SR` und `CR1` an den SPL-Gerätedateien von F103 und F411 belegen.

### 2.2 Prüfstand a15

Differenziell: alte (`git show`) und neue ISR gegen eine Registerattrappe, je mit eigenem Ring. Attrappe:
`DR` lesen löscht RXNE und ORE; TXE nur lesbar, durch Schreiben von `DR` gelöscht; TXEIE in `CR1`.
Zufallsfolgen und Randfälle nach AKI.1, Sabotagen nach AKI.1.

### 2.3 Messung — statisch; Messbau aufgehoben (L432)

**Berichtigung (L432):** Seit LTO (STM 3.2.16) sind die SPL-Aufrufe in der ISR eingebettet; im Bau 3.2.32
hat keine ISR auf F103/F411 ein `bl` (90–95 Befehlszeilen je Instanz). Der Gewinn von A15 sind **wenige
Befehle** — zwei Schreibzugriffe auf `SR` und das doppelte Lesen von `SR` —, nicht der „Faktor drei bis
vier" aus L101.

**Statisch, beide Plattformen (AKI.2) — bindend:** Befehle im Empfangs-Normalpfad der ESP-Brücken-Instanz
aus `arm-none-eabi-objdump -d` der ELF, F103 und F411. Vorher-Wert aus dem Worktree-Bau (R6) des Tags
`release/3.2.32-3.2.33-1.4.97`, weil das Release-ZIP keine ELF enthält. Kriterium: Die Befehlszahl sinkt auf
beiden Plattformen; `SR` wird je Interrupt einmal gelesen und nicht geschrieben.

**Am Gerät (AKI.3):** Regressionslauf auf dem F411 — `o`, `d`, `rx` dürfen sich nicht verschlechtern.

**~~Messbau (AKI.4)~~ — aufgehoben 13.10.2026** (Entscheidung des Nutzers, L432). Bei der geringen
Grösse des Gewinns lohnt kein Messbau auf der produktiven Uhr. Was davon bleibt:
- Der Messcode-Patch `tools/checks/auszug/stm/a15/messbau.patch` (Zyklenzähler über `DWT->CYCCNT`) ist
  geschrieben und abgelegt; er **bleibt liegen, wird aber nicht eingesetzt**.
- `flash-stm.sh --lokal <hex>` ist gebaut (über `/api/local_stm32_upload` und `/api/local_stm32_flash`,
  nie über den Update-Server) und **bleibt als Werkzeug**; in diesem Paket wird es nicht gebraucht.

---

## 3. A56, A81, A82 — Ambilight

### 3.1 Speicherort (Ent-9)

```
#define EEPROM_DATA_SIZE_AMBI_POWER        2                              // Marke 0xA5, dann 0 oder 1
#define EEPROM_DATA_OFFSET_AMBI_POWER      (EEPROM_DATA_OFFSET_DIMMED_AMBILIGHT_COLORS + EEPROM_DATA_SIZE_DIMMED_AMBILIGHT_COLORS)
#define EEPROM_DATA_END                    (EEPROM_DATA_OFFSET_AMBI_POWER + EEPROM_DATA_SIZE_AMBI_POWER)
```

Offset 1979–1980, eine EEPROM-Seite auf F103 (1976–1983) und F411 (1952–1983). Kopfkommentar von
`eeprom-data.h` mitführen. **Ohne Layoutversion:** Eine ältere Firmware liest und schreibt die beiden Byte
nie und lässt das übrige Layout stehen — ein Rückflash kostet nichts. Die Marke unterscheidet „nie
geschrieben" (0xFF) von einem gültigen Wert.

**Prüfpunkt:** Auf `BLACK_BOARD` (F407, `eeprom.c:13`) liegt das EEPROM im Flash — belegen, dass dessen
Bereich 1979–1980 abdeckt; sonst anhalten und vorlegen.

### 3.2 Schreiben und Lesen

- **Schreiben:** in `display_set_ambilight_power()` (`display.c:2907`) über `display_save_ambilight_power()`
  nach dem Muster `display_save_brightness()` (`:3687-3693`); `eep_write()` überspringt Unverändertes (C3).
  Zusätzlich in `display_write_config_to_eep()`, damit `upgrade_eep_version()` nach einem EEPROM-Reset
  „an" schreibt.
- **Lesen:** in `display_read_config_from_eep()`, **unabhängig** von `eep_version`: Marke 0xA5 und Wert 0/1 ⇒
  übernehmen, sonst „an". **Nur die Variable setzen**, keine LED-Ausgabe vor der LED-Initialisierung.
- **Schaltstellen** (✔ SW): `main.c:1447`, `:1789`, `:4460` — alle über `display_set_ambilight_power()`.
- **Last:** ein Seitenschreiben je Wechsel, rund 15 ms.

### 3.3 Prüfstand a56

EEPROM-Abbild: Marke fehlt, Wert 2, Marke falsch, gültig 0 und 1, EEPROM-Reset, simulierter Rückflash
(übriges Abbild byte-gleich). Gegenprobe gegen 3.2.32 ⇒ FEHL.

### 3.4 A81 — Marker-Weissfarbe (Ent-17)

`display.c:3435`: `EEPROM_DATA_OFFSET_AMBI_W_COLOR` durch `EEPROM_DATA_OFFSET_AMBI_MARKER_W_COLOR` ersetzen.
Kein Platz. **Wirkung beim ersten Start danach:** Der Marker-Weisswert kommt jetzt aus seinem eigenen Platz;
dort steht, was zuletzt gespeichert wurde, oder 0xFF aus einem nie beschriebenen EEPROM — die bestehende
Klemmung (`display.c:3508-3513` für die Ambilight-Farbe; für den Marker **Prüfpunkt**: gibt es eine?) muss
einen Wert über `MAX_COLOR_STEPS - 1` abfangen; fehlt sie, im selben Task ergänzen und im Bericht nennen.
Prüfstand a81 nach AKA.4.

### 3.5 A82 — DCF77-Merker (Ent-17)

Eine Hilfsfunktion `dcf77_update ()` in `main.c` setzt `dcf77_enabled = ! display.display_power_is_on &&
! display.ambilight_power_is_on`; aufgerufen in `set_display_power()` (statt `:1471-1478`) und nach den
beiden Ambilight-Schaltstellen (`:1789`, `:4460`). Prüfstand a82 nach AKA.5. Reisst das Gate hier, wandert
A82 nach S3 (Ent-17).

---

## 4. A70 — Randfälle beim Nachholen des Nachtfensters

### 4.1 Zustand

```
static uint32_t     night_off_due    = 0;   // 0 = nichts vorgemerkt; sonst uptime, ab der das Aus gilt (immer eine Frist)
static uint_fast8_t night_off_ip     = 0;   // 1 = das Aus wartet auf das Ende des IP-Lauftexts
```

- **Start im Aus-Fenster (M2, Wirkung unverändert):** `night_off_due = uptime + 60`.
- **IP-Lauftext startet (`main.c:4204-4217`) mit Vormerkung — (c):** `night_off_ip = 1`,
  `night_off_due = uptime + T_ip + 10`; **`T_ip`** = (Länge + 2) × (`TICKER_COLS` + 1) ×
  `display.ticker_deceleration` / 64 s, aufgerundet. Eine feste Frist schnitte bei grossem
  `ticker_deceleration` den Lauftext ab.
- **Frist (`:4219-4222`)** gilt jetzt **immer** — das Netz für (c).
- **Restore-Innenzweig (`:4868`):** `if (night_off_ip)` statt `== NIGHT_OFF_AFTER_IP`. Die vier
  Teilbedingungen und `! display_clock_flag` bleiben **wörtlich** (AKN.5).
- `night_off_apply()` und `set_display_power()` setzen beide zurück. `NIGHT_OFF_AFTER_IP` entfällt.

### 4.2 (d) Erstes `TIME` bei ungültiger RTC

`:3431-3434`: Steht der IP-Lauftext aus (`ip_ticker_pending`) oder läuft er (Merker `ip_ticker_running`,
gesetzt beim Start des IP-Lauftexts, gelöscht im Restore-Innenzweig und beim Start jedes anderen Tickers),
wird mit `at_start = 2` vorgemerkt; läuft er gerade, zusätzlich `night_off_ip` und die Frist aus §4.1.
Sonst wie heute `night_apply (0xFFFF, 1)`.

### 4.3 (e) Fremder Ticker

Jede Tickerstart-Stelle in `main.c` (Bestandsliste per `grep display_set_ticker`, ✔ SW mindestens `:2315`,
`:3520`, `:3527`, `:4719`, `:4853`) läuft über eine Hilfsfunktion `start_ticker (text)`: Ist `night_off_ip`
gesetzt, zuerst `night_off_apply()`; danach ist das Display aus und `display_set_ticker()` tut nichts
(`display.c:4954`). Der IP-Lauftext selbst bleibt aussen vor.

### 4.4 L399 — reiner DCF77-Betrieb

Im DCF77-Zweig (`:4341-4372`) nach der Übernahme: `if (night_wm == 0xFFFF) night_apply (0xFFFF, 1);` —
dieselbe Regel wie beim ersten `TIME`, nur ein Aus. DCF77 empfängt nur bei ausgeschaltetem Display und
Ambilight; liegt laut Tabelle ein Ein-Fenster, kann das Display **absichtlich** aus sein — ein automatisches
Einschalten widerspräche dem. Gewinn: `night_wm` ist gesetzt, die Sprungerkennung (`:4437-4442`) greift.

### 4.5 Prüfstand a70

Auf der Grundlage von a69: (c) Restore kommt nie ⇒ Aus nach `T_ip + 10`; (d) erstes `TIME` während und lange
nach dem IP-Lauftext; (e) Wetterticker während des Wartens ⇒ Aus vor dem Ticker; L399. Je Gegenprobe gegen
3.2.32.

---

## 5. A80 — Icon bei ausgeschaltetem Display verwerfen

- **Empfang (`main.c:3551-3558`):** Display aus ⇒ `do_display_icon` und `show_icon_stop_time` nicht setzen,
  `log_message ("icon: verworfen, display aus")`.
- **Display geht während eines Icons aus:** `set_display_power()` ruft beim Ausschalten
  `display_icon_cancel()` (neu in `display.c`: Zustand von `display_icon()` auf
  `DISPLAY_ICON_STATE_CLEAR_DISPLAY`, `do_display_icon = 0`). `show_icon_stop_time` läuft von selbst ab
  (`main.c:4753-4767`).
- **Restore-Bedingung unverändert;** `icon_freeze: ENTER` darf danach nicht mehr vorkommen (Wache: Alarm ab S2).

---

## 6. B55 — „Messwert übernehmen" meldet das Ergebnis des STM

### 6.1 Leitungsformat (STM → ESP)

```
RPCR xx r\r\n        xx = RPC-Index, zwei Hexziffern; r = 0 uebernommen, 1 abgewiesen, 2 Messung fehlgeschlagen
```

- Gesendet wie `LOG ` (`esp8266.c:871-907`), **ohne** Quittung, **nach** `var_send_ldr_*_value()`.
- **Keine neue Variable:** verschöbe den Variablensatz und berührte Abgleich und `var-crc` (A42, C26).
- **Nicht am unveränderten Wert, nicht an der Logzeile:** gleich gemessen sähe aus wie abgewiesen; die
  Logspiegelung ist kein Protokoll (A19).
- **STM:** `ldr_set_min_value()`/`ldr_set_max_value()` (`ldr.c:170-245`) liefern das Ergebnis
  (Prüfpunkt: weitere Aufrufer per `grep`); `main.c:1620-1634` sendet `RPCR`.

### 6.2 ESP — warten, ohne die Brücke anzuhalten

- Der Block `while (Serial.available ()) { … }` am Ende von `loop()` (`ESP-uclock.ino:1440-2255`) wird zur
  Funktion `stm_bridge_poll()`; die drei `static`-Variablen (`:1403-1405`) wandern mit. **Der Rumpf bleibt,
  wo er ist**; gemischte Zeilenenden byte-genau erhalten.
- Sperre gegen verschachtelte Aufrufe; Zweig `RPCR` neben `LOG ` legt Ergebnis und laufende Nummer je Index ab.
- Handler: `rpc()` senden, bis 2'000 ms `stm_bridge_poll(); yield();`, bis die Nummer wechselt.
- Antworten: `0` ⇒ `{"ok":true,"value":n}`; `1` ⇒ Kennung **10**; `2` ⇒ Kennung 7; Frist ⇒ Kennung **11** und
  `- rpc keine Antwort #n idx=xx` (gedrosselt, Mitschnitt und `/api/stm32_log`, L185).
- Neue Kennungen (`http.cpp:244-273`): **9** Query zu lang (E1), **10** abgewiesen, **11** keine Antwort (E2);
  je ein Eintrag in **beiden** i18n-Tabellen (EP).

### 6.3 PWA (EP)

Die Aufrufer (`app.js:13159-13165`, `runSimpleAction()`) unterscheiden 10, 7, 11 und zeigen die Texte aus
AKL.3.

---

## 7. C8 — Kommandopuffer und Ring (Ent-10)

- `CMD_BUFFER_SIZE` 260 (4 Byte `LOG `, 255 Zeichen, Nullbyte), Kommentar mit Herleitung.
- `stm32_log_append()`: eine Zeile über 120 Zeichen als **Folgeeinträge** — der erste mit 119 Zeichen und `~`
  am Ende, jeder folgende beginnt mit `~` und trägt bis zu 119 weitere. RAM-neutral.
- Die PWA zeigt Folgeeinträge als eigene Zeilen. STM-Seite: nicht nötig.

---

## 8. C6, C6v, C60, C6b — zerstörende Wege und Schlüssel

### 8.1 Herkunft (E1)

`http_request_is_cross_origin()`: wahr, wenn `Sec-Fetch-Site` gesetzt und weder `same-origin` noch `none`.
Abgewiesen (403) wird bei eingebettetem `Sec-Fetch-Dest` **oder** fremder Herkunft. Begründung: POST allein
hält fremde Formular-POSTs nicht ab, und eine Formular-Navigation trägt `Sec-Fetch-Dest: document`.
PWA und Legacy laufen vom Gerät (`same-origin`), curl sendet keine Kopfzeile — beides bleibt zugelassen.
**Grenze, bewusst:** ältere Browser ohne `Sec-Fetch-*`.

**Wege (Ent-12):** die vier Wartungsendpunkte, `fs_remove` (C6v), Legacy `/fs?action=format`,
`/fs?action=remove` (C6v), `/?action=rsteeprom2`, `/?action=rststm32` (C60) und jeder weitere Legacy-Weg zu
`stm32_reset()` oder `RESET_EEPROM_RPC_VAR` (✔ SW Kandidaten `:7542`/`:7634`, `:13612`/`:13723`; Bestandsliste
per `grep`). Der GET-Zweig fängt `Sec-Fetch-Dest` bereits ab (`:14677-14680`) und bekommt `Sec-Fetch-Site`
dazu. Legacy antwortet mit einer kurzen 403-Seite statt JSON.

### 8.2 POST (EP)

- **POST-Zweig (`:14628-14659`):** Pfad in `http_post_api_paths[]` (die vier Wartungsendpunkte,
  `network_secrets`) ⇒ Kopfzeilen bis zur Leerzeile lesen — dieselbe Schleife wie im GET-Zweig
  (`:14662-14681`), die die Regel `available()` nach `connected() == 0` einhält (C53) — dann
  `http (sPath, sParam)`. Upload-Pfade bleiben bei `http_post()`. Anderer POST-Pfad ⇒ 404.
- **`http_request_method`** merken; die fünf Handler prüfen ihn: GET ⇒ 405 `{"ok":false,"error":"method"}`.
- Rumpf wird nicht gelesen; Reste verwirft das bestehende Leeren vor `stop()` (`:14725`).

### 8.3 C6b (EP)

- `/api/eeprom_settings`: `key` und `ap_key` als `""`, dazu `key_set`, `ap_key_set`. **Leer statt
  Platzhalter:** `eeprom_settings_set` übergeht leere Felder (`http.cpp:9379-9395`); ein Platzhalter würde
  als echter Schlüssel gespeichert.
- `/api/network_secrets` (POST, Herkunft, `Cache-Control: no-store`), in der URL-Liste als
  `network_secrets_url` (bei `:12715-12723`).
- PWA: `ensureBackupExportState()` holt `network_secrets` (POST); fehlt die Antwort, Export-Abbruch mit
  eigener Kennung in beiden Tabellen; `getNetworkBackupMeta()` liest die Schlüssel von dort.

### 8.4 Einstufig (Ent-11)

Der ESP nimmt die fünf Endpunkte in EP **nur noch** per POST an. Damit kein Zeitfenster mit halbem Stand
entsteht:

1. ESP-Teil (EP.1–EP.3) und PWA-Teil (EP.4–EP.7) entstehen nacheinander im Baum und gehen in **einen**
   `make release-zip`, **einen** Commit, **ein** Tag mit beiden Versionen.
2. Die Werkzeuge stellt der Lead **im selben Commit** um (AKZ.11): `flash-stm.sh`, `smoke-device.sh`,
   `snapshot-device.sh`, `preview/server.py`; `no-danger.py` mit Probe für `curl -X POST` schon in Schritt 0.
   Getestet gegen Vorschau und Prüfstand c6p **vor** dem Build. Vor EP (S1, S2) laufen die Werkzeuge noch
   mit GET gegen den alten ESP — deshalb wird nicht früher umgestellt.
3. Einspielen: ESP-OTA, **unmittelbar** danach `install-app.sh` — kein Gerätenachweis, keine schreibende
   Aktion, kein Backup-Export dazwischen. Danach Smoketest samt Update-Quelle, `--check`, `check-pwa.sh`.
4. **Rückweg, falls der PWA-Upload scheitert:** Die alte PWA bleibt auf dem Gerät und kann keine
   Wartungsaktion mehr ausführen (405). Die Legacy-Seite bleibt bedienbar (GET, `same-origin`) — sie ist der
   Rückweg für einen STM-Reset. Das gehört in die Einspielzeile.

### 8.5 PWA (EP)

Eine Hilfsfunktion `apiPost (url, options)` an den sechs Aufrufstellen (`app.js:5393`, `:11787`, `:11851`,
`:11855`, `:11875`, `:12412`), beim Löschen über `fs_remove_base_url` und für `network_secrets`. Der
Prüfstand zählt die Aufrufe nach Methode.

---

## 9. C19 — Legacy-Update-Quelle (nur L311)

An **beiden** Stellen (`http.cpp:6724-6731`, `:7520-7527`): leer oder über 63 Byte ⇒ abweisen, Meldung
auf der Seite (`alert_message`-Muster, `:4681-4683`), Wert unverändert. **Empfehlung:** dieselbe
Prüffunktion wie die API für `update_host_set`/`update_path_set` (L48). Sonst nichts am Legacy-Zweig.

---

## 10. C46 — Query über 255 Zeichen

- **ESP (E1):** in `http()` vor dem `strncpy` (`:13795-13798`) und im POST-Zweig vor `:14648`:
  `strlen (const_param) > 255` und Pfad `/api/…` ⇒ Kennung 9, kein Handler, `- abgewiesen: query <n>
  Zeichen` (ohne Inhalt, L179). Legacy unverändert.
- **PWA (EP):** zentral in `apiFetch()`: Query-Teil länger als 255 ⇒ kein Request, Fehler mit Kennung 9.
  Backup-Import: `eeprom_settings_set` **je Feld** einzeln, jedes mit demselben `boot_as_ap` (der ESP setzt
  das Flag bei jedem Aufruf, `http.cpp:9397-9407`, und übergeht leere Felder); höchstens rund 207 Zeichen je
  Aufruf.

---

## 11. C59 — NTP-Antwort an die Anfrage binden

`send_ntp_packet()` (`ntp.cpp:117-136`): 8 Byte aus `RANDOM_REG32` in Byte 40–47, gemerkt; `ntp_pending = 1`,
Zeitpunkt merken. `ntp_poll_time()`: nach den bestehenden Prüfungen `! ntp_pending` oder älter als 5'000 ms ⇒
`ungefragt`; Byte 24–31 ungleich ⇒ `originate`; nach Annahme `ntp_pending = 0`. RFC 5905: Der Server kopiert
Transmit in Originate. Gegen einen Mitleser hilft es nicht (L406).

---

## 12. A79 — ESP-Empfangspuffer

`Serial.setRxBufferSize (1024);` unmittelbar **vor** `Serial.begin (115200)` (`ESP-uclock.ino:160`).
**Prüfpunkt:** an `HardwareSerial.cpp` des Core 3.1.2 belegen, dass die Grösse vor `begin()` gemerkt wird.
Keine Ausgabe vor der `CAP`-Zeile (`:166-171`). Kommentar `:474-479` nachführen.

---

## 13. B48 — Fehlerzeile am Feld

- **PWA:** `reportInvalidInput()` (`app.js:6614`) und `readTimeInputOrReport()` (`:13375`):
  `aria-invalid="true"`; `<p class="field-error" id="<feld-id>-error" role="alert">` direkt nach dem Feld,
  per `textContent`; `aria-describedby` ergänzen, vorhandene IDs behalten; fehlende Feld-ID vergeben; ein
  `input`-Ereignis entfernt alles wieder. Banner und Fokus bleiben.
- **UI:** `.field-error` in `styles.css` (bestehende Fehlervariablen, Kontrast wie L245). Kein Markup nötig.
- **iPhone (Nutzer):** AKF.2.

---

## 14. B8 — deutsche Texte in die i18n-Tabelle

1. **Zählen (Lead, Schritt 0):** Kandidatenliste, einmalige Zuordnung, Ausnahmeliste mit Grund. Nicht über
   Satzwörter oder Umlaute — „Speichern" hat weder das eine noch das andere.
2. **Umsetzen:** `app.js` und `en.json` (`pwa-developer`), `index.html` (`ui-developer`, `data-i18n*`).
   Schlüssel nach dem Schema `modul.zweck`; keine Umschrift (S8).
3. **Halten:** Das Werkzeug wird Guardrail-Stufe (AKD.2).

---

## 15. F3 — Umfangsprüfung des Backups

- Quelle 1: Dispatch in `http.cpp` (`strcmp (path, "/api/…_set")`).
- Quelle 2: die Setter aus den Importfunktionen der PWA — eine **benannte Liste** der Importfunktionen im
  Werkzeug (ausgegeben), darin jeder `get…Url()`-Getter, aufgelöst über die Konfigurationstabelle
  (`app.js:10533` ff.). Fehlt eine benannte Funktion ⇒ FEHL (Gattung L235).
- Ausnahmeliste `tools/checks/backup-umfang.ausnahmen`, je Zeile `endpunkt  grund`.
- Einmalige dynamische Gegenprobe: Import in der Vorschau, aufgerufene `*_set` gezählt.

---

## 16. E36 — Logwache mit Lebenszeichen

- **Ursache:** `pmset -g log` für das Fenster; ob `caffeinate` lief.
- **Herzschlag:** je Runde Zeitstempel, Intervall und Rundenzahl in `.git/watch-log.lebt$WL_SFX`.
- **`--pruefe`:** Alter gegen 3 × Intervall, Exit 0/1. Stop-Hook ruft es bei gesetzter Gerätemarke.
- **Selbstmeldung:** Wanduhr vor `sleep` merken; Differenz über 3 × Intervall ⇒ Alarm.
- **Frist für `log.sh`:** Aufruf im Hintergrund, eigener Zähler, `kill` nach 20 s (macOS ohne `timeout`).
- **`caffeinate`:** Die Wache startet sich unter `caffeinate -i`, wenn verfügbar, und meldet es in der
  Kopfzeile; DIR-020 gilt für den Lead zusätzlich bei geplanten Tests.

---

## 17. Gerätenachweise

### 17.1 Ablauf je Gerätelauf

Wache vor dem Einspielen starten (DIR-013), `--pruefe` am Ende (AKZ.5), Smoketest sofort ins Messprotokoll.
Nach jedem ESP-Update `install-app.sh --check` und die Update-Quelle (DIR-009, DIR-017). Auf später geplante
Tests mit `caffeinate -i -s -t <Sekunden>` (DIR-020).

### 17.2 Was schreibt — alles freigegeben (Ent-13)

| Nachweis | Art | Rückstellung |
|---|---|---|
| G0, G1 Setter-Burst 14 × Wetterort | schreibend | Wetterort zurück |
| G0, G3, G4 Heap, `stm32_log`, `device_ready` mit langer Query | lesend | — |
| Einspielen S1, S2, (S3), E1, E2, EP, P2 | Update | — |
| G2 Ambilight aus, STM-Reset, zurück | schreibend + Reset | Vorher-Wert |
| G2 Display aus, `weather_get_now`, Display an | schreibend, Display kurz dunkel | Display an |
| G2 Nacht: STM-Reset im Aus-Fenster mit `weather_get_now` (A70 (e)) | Reset nachts | — (Ein-Timer am Morgen) |
| G3 `maintenance_reset_stm32` und Legacy `rststm32` mit Abweisungs-Kopfzeilen | Reset (nur ohne Kopfzeile) | — |
| G4 zwei STM-Resets (A79), `weather_get_forecast` (C8) | Reset, Auslöser | — |
| G5 POST/GET `maintenance_reset_stm32`, LDR-Proben, Backup-Export | Reset, EEPROM | 0/4095 |
| G6 iPhone, drei Felder | Nutzer | — |
| T1, T2 `pwa-tester` mit S-Fällen | nach Testplan | nach Testplan |

### 17.3 Nie am Gerät — nur am Prüfstand

`maintenance_format_fs`, `maintenance_reset_eeprom`, `fs_remove` (auch mit nicht vorhandener Datei),
Legacy `/fs?action=format`, `/fs?action=remove`, `/?action=rsteeprom2` — **auch nicht** mit
Abweisungs-Kopfzeile: Versagt die Prüfung, sind PWA oder Einstellungen weg. Legacy `saveuphost`/`saveuppath`
leer oder überlang — versagt die Prüfung, zeigt die Update-Quelle auf den Server des Ursprungsprojekts.
NTP-Fälschung (C59), A70 (c), (d), L399 (nicht herstellbar).

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `src/uart/uart-driver.h` | A15 | `stm-developer` |
| `src/main.c` | B55 (`RPCR`), A70, A80, A82 | `stm-developer` |
| `src/ldr/ldr.c`, `ldr.h` | B55 (Ergebnis) | `stm-developer` |
| `src/display/display.c`, `display.h` | A56, A80 (`display_icon_cancel()`), A81 | `stm-developer` |
| `src/eeprom/eeprom-data.h` | A56 | `stm-developer` |
| Messcode-Patch (`tools/checks/auszug/stm/a15/messbau.patch`) | liegt, wird nicht eingesetzt (Ent-15 aufgehoben) | — |
| `ESP8266/ESP-uclock/http.cpp` | Herkunft, C60, C6v, C46, C19 (E1); B55 (E2); POST, C6b (EP) | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | B55 (`stm_bridge_poll`, `RPCR`), C8, A79 | `esp-developer` |
| `ESP8266/ESP-uclock/ntp.cpp` | C59 | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | C6, C6b, C46, B55 (EP); B48, B8 (P2) | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/i18n/en.json` | Kennungen 9–11, B55 (EP); B8 (P2) | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/styles.css` | B48 | `ui-developer` |
| `ESP8266/ESP-uclock/data/app/index.html` | B8 | `ui-developer` |
| `tools/watch-log.sh`, `.claude/` (Stop-Hook) | E36, Wachzeilen | Lead |
| `tools/flash-stm.sh` | `--lokal` (Schritt 0, gebaut, bleibt als Werkzeug), POST (EP) | Lead |
| `tools/checks/**` | F3, B8, Prüfstände | Lead |
| `tools/smoke-device.sh`, `snapshot-device.sh`, `preview/server.py`, `hooks/no-danger.py`, `checks/hooks.sh` | POST, C6b | Lead |
| Versionsdateien | je Release | `release-engineer` |
| `BEFUNDE.md` | Messprotokoll (Lead), Befunde (`doc-writer`) | nie gleichzeitig |
| `TESTPLAN-PWA.md` | neue Fälle | `doc-writer` |
| `CLAUDE.md`, `CHANGELOG.md` | Gefahrentabelle (POST), Einträge | Lead |

---

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Werden die Wetter-Endpunkte eingehalten?
Bleibt die Restore-Bedingung vollständig? Greift die Änderung an der richtigen Schicht an?

> **Legacy bleibt parallel und bedienbar** — mit GET; neu sind nur die Herkunftsprüfung an den zerstörenden
> Aktionen (§8.1) und die Abweisung leerer und überlanger Update-Quellen (§9). Für jemanden, der die
> Legacy-Seite selbst bedient (`same-origin`), ändert sich nichts. Nach EP ist Legacy der Rückweg, falls der
> PWA-Upload scheitert (§8.4). **Wetter:** Kein Pfad berührt die Wetter-Endpunkte; G2 und G4 rufen sie als
> Auslöser. **Restore-Bedingung:** A70 und A80 lassen die vier Teilbedingungen und `! display_clock_flag`
> wörtlich stehen (AKN.5); geändert wird der Vergleich im Innenzweig und, dass `do_display_icon` bei
> ausgeschaltetem Display nicht gesetzt bleibt. **`.gz` und Grösse > 0:** EP und P2 gehen über
> `make app-gz` und `install-app.sh` (R2). **Schicht:** B55 entscheidet am STM und reicht durch; C46 weist am
> ESP ab, die PWA vermeidet nur unnötige Fehler; A79 und C8 beheben Verluste am ESP.

**Scalable systems** — Wie viele STM-Kommandos erzeugt die Aktion? Wie viele Byte pro Minute zusätzlich
auf der UART? Bleibt jeder Pfad unter 20 s Watchdog? Wie lange blockiert der Hauptloop?

> **STM-Kommandos:** B55 ein RPC je Druck wie bisher, dazu eine unquittierte Zeile `RPCR` (rund 11 Byte).
> A56 und A82 erzeugen kein Kommando. C46 teilt den Netzwerkimport in bis zu vier ESP-Aufrufe, ohne
> STM-Kommando. **UART:** neu sind `RPCR`, `icon: verworfen`, `- abgewiesen: query`, `- rpc keine Antwort`
> (gedrosselt) — keine Zeile je Anfrage oder je Sekunde. **Watchdog STM:** A56 ein Seitenschreiben (~15 ms)
> je Wechsel; A70, A80, A81, A82 ohne Schleife; A15 spart in der ISR wenige Befehle (L432). **ESP:** Das
> Warten in B55 ist auf 2'000 ms begrenzt, mit `yield()`; HTTP steht in dieser Zeit still — bewusst, weil
> Ent-6 das Warten verlangt. **Hauptloop STM:** keine Blockade über eine EEPROM-Seite hinaus. **Feste
> Grenzen:** Query 255 (sichtbar statt still), Kommandopuffer 260, Ring 32 × 121, RX-Puffer 1'024.

**Secure by design** — `escapeHtml` bei jedem `innerHTML`? Fremddaten als nicht vertrauenswürdig
behandelt? Keine Credentials in Logausgaben?

> **Credentials:** C6b nimmt den Schlüssel aus dem allgemein lesbaren Endpunkt; der neue Endpunkt ist POST,
> herkunftsgeprüft, nicht zwischenspeicherbar. Kein neuer Log gibt einen Wert aus; Berichte nennen den
> Schlüssel nur als Prüfsumme. **Fremddaten:** NTP an die eigene Anfrage gebunden (C59); B48 setzt
> `textContent`; B8 über den bestehenden `data-i18n`-Weg. **Zerstörende Wege:** Erstmals hat jeder Weg —
> auch Legacy `rsteeprom2` und `rststm32` (C60, bisher ganz ungeschützt) — eine Herkunftsprüfung; die fünf
> API-Endpunkte der PWA sind zusätzlich nur per POST erreichbar. **Grenze, benannt:** curl im LAN und alte
> Browser (`http.cpp:471-473`).

**Stable & reliable** — Wird jeder Fehler ausgewertet? Leere `catch`? Stille Verwerfungen? Ist der Zustand
nach Abbruch mitten in einer Sequenz definiert? Wird ein gesetztes Flag auf jedem Pfad wieder aufgelöst?

> **Erfolgsmeldung am Ergebnis:** B55 „gespeichert" nur bei `RPCR … 0`, jede andere Antwort, auch das
> Ausbleiben, ist eine eigene Meldung; C46 und C19 biegen nicht mehr still. **Stille Verwerfungen:** Icon,
> NTP-Antwort, Query je mit Logzeile. **Flags:** `night_off_ip`/`night_off_due` in `night_off_apply()` und
> `set_display_power()` gelöscht, die Frist gilt immer; `ip_ticker_running` im Restore-Innenzweig und bei
> jedem anderen Ticker gelöscht; „Anfrage offen" läuft nach 5 s ab; die Sperre in `stm_bridge_poll()` hat
> eine Ausgangsstelle; `display_icon_cancel()` setzt Zustand und `do_display_icon`; `dcf77_enabled` über
> eine Funktion an allen Schaltstellen (A82). **Abbruch in einer Sequenz:** Der aufgeteilte Netzwerkimport
> setzt je Feld vollständig; `boot_as_ap` steht nach jedem Aufruf auf dem Wert des Backups. Ein
> gescheiterter PWA-Upload in EP hat einen benannten Rückweg (§8.4). **Leseschleife über `WiFiClient`:** Die
> Kopfzeilenschleife im POST-Zweig übernimmt die des GET-Zweigs (C53) — Prüfpunkt im Review.

---

## Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| A56 mit Layoutversion 3.0 | Ent-9: Rückflash setzte alle Einstellungen zurück |
| A56 im Bereich der alten Overlay-Intervalle (Offset 407) | Altbestand, ohne Layoutversion nicht unterscheidbar |
| B55 am unveränderten Wert / an der Logzeile / als neue Variable / asynchron | §6.1; asynchron widerspricht Ent-6 |
| C8 Ringzeilen 255 | Ent-10, 4'320 Byte RAM |
| C6 zweistufig | Ent-11 (war die Empfehlung) |
| C6 POST ohne `Sec-Fetch-Site` | fremde Formular-POSTs gingen durch |
| C6 Legacy auf POST | Ent-12; POST-Zweig liest keine Formularrümpfe |
| C6b schon in E1 | öffnete das Fenster mit leeren Schlüsseln über drei Releases (§0.2) |
| Werkzeuge vor EP auf POST | sie liefen dann gegen den alten ESP ins Leere (§8.4) |
| Messbau für A15 am Gerät (Ent-15) | aufgehoben 13.10.2026: seit LTO kein `bl` in der ISR, der Gewinn sind wenige Befehle (L432) |
| Messbau über den Update-Server | belegte die Quelle der produktiven Updates mit einem Diagnosestand (§2.3) |
| A70 feste zweite Frist | schnitte den IP-Lauftext bei grossem `ticker_deceleration` ab |
| A80 Icon gar nicht anfordern | zweite Stelle, die dieselbe Regel kennen müsste |

## Versionsfolgen

- [ ] STM `src/main.h` anheben — S1, S2 (und S3), je einzeln
- [ ] ESP `version.h` anheben — E1, E2, EP
- [ ] App `APP_VERSION` anheben — EP und P2
- [ ] `CACHE_NAME` in `sw.js` anheben — EP und P2, je mit `APP_VERSION`

Nur der `release-engineer` führt das aus (R4).
