# Design — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (A15, A56, A70, A79, A80, B8, B48, B55, C6, C6b, C6v, C8, C19, C46, C59, E36, F3)

**Erstellt:** 2026-10-13, Stand `159c9d0`. **Status: Entwurf — zur Freigabe.** Offene Fragen N-1 bis N-7
(`requirements.md`); wo sie berührt sind, steht hier die Empfehlung, nicht der Beschluss.
Momentaufnahme (DIR-006).

**Grundlage:** Fundstellen vom `spec-writer` am Code `159c9d0` nachgelesen (`requirements.md`,
Abschnitt „Problem"). Technische Detailentscheide, für die kein Analyse-Agent befragt wurde, sind als
**Empfehlung** gekennzeichnet und begründet; der zuständige Umsetzer darf mit Begründung am Code
abweichen und meldet es im Taskbericht, wie A60 im Vorpaket (`delay_counter`).

---

## 0. Reihenfolge und Release-Schnitt

### 0.1 Regeln

| Regel | Woher | Umsetzung |
|---|---|---|
| Code des **nächsten** Releases erst nach dem **Release-Build** des vorigen | `make release-zip` baut STM, ESP und PWA-Assets zusammen (Lehre Paket 2026-10-09) | S2.1 nach S1.10, E1.1 nach S2.18, E2.1 nach E1.13, P1.1 nach E2.10, E3.1 nach P1.9, P2.1 nach E3.5 |
| **Release-Build** erst nach dem **Gerätenachweis** des vorigen | R3b; ein Gerätenachweis soll genau eine Änderung zeigen | S2.18 nach S1.12 (G1), E1.13 nach S2.20 (G2), E2.10 nach E1.15 (G3), P1.9 nach E2.12 (G4), E3.5 nach T1, P2.10 nach E3.7 (G6) |
| **Firmware vor PWA**, STM vor ESP, wo eine Kette besteht | DIR-002, L241 | B55: S2 → E2 → P1. C6/C6b/C46: E1 → P1. C6 Stufe 2 (E3) erst nach P1 |
| STM-Release-Build erst nach **grünem Matrix-Lauf** auf dem angehobenen Stand | Ent-18 des Vorpakets, Festlegung des Leads | S1.9, S2.17 |
| Jederzeit erlaubt: Werkzeug, Doku, Spec, lesende Analysen | R3b | Schritt 0; Werkzeuganpassungen für C6 nach G3 (AKZ.11) |
| Messwerte **sofort** in `BEFUNDE.md` | L424, Skill `/release` | Jeder Gate-, Matrix-, RAM-, Smoketest- und `--full`-Task schreibt seine Zeile, bevor der nächste Task beginnt (§1.4) |

**Ausnahme für die lange P2-Arbeit (Empfehlung):** B8 ist umfangreich und von keiner Firmware abhängig.
Der Lead darf P2.3/P2.4 in einem eigenen Worktree (R6) vorziehen; eingespielt in den Hauptbaum wird
erst nach dem Release-Build von E3. Ohne Worktree bleibt es bei der Kette oben.

### 0.2 Release-Zuschnitt

| # | Release | Inhalt | Komponente | Nachweis | Warum so geschnitten |
|---|---|---|---|---|---|
| — | **Schritt 0** | E36, F3, B8-Zählung, Grundlinien, neue Befunde | Werkzeug, Doku | **G0** | Die Wache trägt jeden Gerätenachweis (DIR-013) — sie muss **vor** G0 halten. Die Grundlinien gehören vor jede Änderung |
| 1 | **S1** (STM) | A15 | STM | **G1** | Die ISR liegt unter **jedem** Brückenpfad und unter dem Mitschnitt. Ein Fehler dort sähe aus wie ein Fehler irgendwo — isoliert ist er zuzuordnen, wie A60 im Vorpaket |
| 2 | **S2** (STM) | B55-STM, A56, A80, A70 | STM | **G2** | Vier getrennte Kennzeichen (`RPCR`, EEPROM-Seitenschreiben bei Ambilight, `icon: verworfen`, `night: …`). A80 und A70 berühren denselben Restore-Bereich und werden **zusammen** gegen die Invariante geprüft |
| 3 | **E1** (ESP) | C6 Stufe 1, C6v, Legacy-Herkunft, C6b, C46, C19 | ESP | **G3** | Alles in der Anfrageverarbeitung von `http.cpp`. Ein Absturz bei einer Anfrage gehört hierher, nicht zur Brücke |
| 4 | **E2** (ESP) | B55-ESP, C8, A79, C59 | ESP | **G4** | Brücke, RAM, Netz. RAM-Messung (AKM.2) ohne Überlagerung durch E1 |
| 5 | **P1** (PWA) | C6, C6b, C46, B55 | PWA | **G5** | Schliesst die drei Ketten STM → ESP → PWA |
| — | **T1** | `pwa-tester` | — | AKZ.6 | P1 vollendet Funktionen über drei Komponenten (DIR-012) |
| 6 | **E3** (ESP) | C6 Stufe 2 (GET ⇒ 405) | ESP | **G6** | Erst wenn PWA und Werkzeuge POST nutzen (N-3) |
| 7 | **P2** (PWA) | B48, B8 | PWA | **G7** | Reine Oberfläche, grosser Umbau an `app.js`; getrennt von den Funktionsänderungen aus P1, damit ein Weissschirm eindeutig zuzuordnen ist |
| — | **T2** | `pwa-tester` | — | AKZ.6 | Abschluss |
| — | **Z** | Doku | — | — | — |

**Wer E1 und E2 zusammenlegen möchte** (ein OTA weniger), verliert die getrennte RAM-Messung und die
Zuordnung „Anfrage oder Brücke". Die Spec empfiehlt die Trennung; der Nutzer kann bei der Freigabe
anders entscheiden.

**Versionen** (erwartet, setzt der `release-engineer`, R4): S1 STM 3.2.33, S2 STM 3.2.34, E1 ESP 3.2.34,
E2 ESP 3.2.35, P1 PWA 1.4.98 (`CACHE_NAME` mit), E3 ESP 3.2.36, P2 PWA 1.4.99.

### 0.3 Einspielreihenfolge und Zeitfenster

| Nach | Zustand bis zum nächsten Einspielen | Folge | Massnahme |
|---|---|---|---|
| S2 | neuer STM sendet `RPCR`, alter ESP | Der alte ESP übergeht unbekannte Zeilen (`ESP-uclock.ino`, Kette der `strncmp`-Zweige) | keine |
| E1 | neuer ESP, alte PWA | Wartungsaktionen gehen per GET weiter (N-3 a). Der Backup-Export der alten PWA bekäme **leere Schlüssel** (C6b) — **stiller Verlust** beim späteren Wiederherstellen | **Zwischen E1 und P1 kein Backup exportieren.** Die Einspielzeile von E1 sagt es; der Lead hält das Fenster kurz |
| E2 | neuer ESP, alte PWA | `ldr_min_set` antwortet bei Abweisung mit Kennung 10 — die alte PWA zeigt ihren Rückfalltext statt „gespeichert". Besser als vorher | keine |
| P1 | neue PWA, ESP mit GET und POST | volle Funktion | — |
| E3 | GET ⇒ 405 | Werkzeuge müssen vorher auf POST sein (AKZ.11) | E3.1 hängt an AKZ.11 |

---

## 1. Budgets

### 1.1 Flash F103

| Stand | Teilbuild F103 WC12h SK6812-RGBW | engste Matrix-Variante |
|---|---|---|
| 3.2.32 (Ausgang) | **1'656** (S8b, 13.10.2026) | **1'436** (`wc24h-stm32f103-sk6812-rgbw`, L416) |

Spielraum über dem Gate an der engsten Variante: **412 Byte**. Der Abstand Teilbuild–engste Variante
schwankte im Vorpaket zwischen 156 und 220 Byte.

### 1.2 Planung S1 und S2 (Lehre des Vorpakets: Faktor 1,5 bis 3)

| Task | Schätzung | mit Faktor 3 |
|---|---|---|
| A15 | −80…+20 (SPL-Funktionen fallen ggf. ganz aus dem Bild) | — |
| B55-STM | +40…+100 | 300 |
| A56 | +60…+120 | 360 |
| A80 | +20…+60 | 180 |
| A70 | +80…+200 | 600 |

**Mit dem Faktor reicht der Spielraum nicht sicher.** Daraus:

1. **Geplant wird mit der Messung je Task**, nicht mit der Summe (AKM.1).
2. **Reihenfolge in S2 nach Nutzen und Abhängigkeit:** B55-STM (die ESP- und PWA-Hälfte hängen daran),
   A56, A80, A70.
3. **Vorwarnschwelle:** F103-Teilbuild unter 1'244 Byte ⇒ Matrix-Vorablauf vor dem nächsten Task.
4. **Reisst das Gate (N-6, Empfehlung):** A70 wandert in ein eigenes STM-Release mit vorgeschalteter
   Flash-Analyse nach dem Muster S3 des Vorpakets; B55-STM, A56 und A80 gehen aus. Ohne diese Freigabe:
   anhalten und vorlegen.
5. **Keine Einsparung ohne Rückfrage**, auch keine verhaltensneutrale — das wäre ein eigener Task mit
   eigener Messung.

### 1.3 ESP-RAM

Ausgang (L421): Heap frei 9'472, grösster Block 8'736. Kosten, gerechnet: A79 +768 Heap (1'024 statt 256,
angelegt in `begin`), C8 +132 BSS (Puffer 260 statt 128), C59 +12 (Zufallszahl, Merker, Frist), B55 +8,
C6 einige Dutzend für die Kopfzeile `Sec-Fetch-Site`. Zusammen rund **950 Byte**; die Grenze in AKM.2
(1'200) lässt 25 % Reserve. **Ring nach N-2 (a): RAM-neutral.**

### 1.4 Messprotokoll in `BEFUNDE.md` (L424)

**Empfehlung:** je Release **eine** Befundzeile „Messprotokoll <Release>", der der Lead jede Messung im
Augenblick der Messung anhängt: Zeitpunkt, Instrument, Wert, Differenz, Gate erfüllt ja/nein. Eine Zeile je
Release statt je Messung hält die Tabelle lesbar und erfüllt die Lehre, weil nichts erst am Ende
eingesammelt wird. **R3:** Die Messzeilen schreibt der Lead; Befundnachführungen schreibt der
`doc-writer` in eigenen Tasks — nie gleichzeitig.

---

## 2. A15 — UART-ISR

### 2.1 Lösungsweg

`SR` und `CR1` **einmal** lesen, danach nur noch aus den Kopien entscheiden:

```
sr  = UART_NAME->SR;
cr1 = UART_NAME->CR1;

if (sr & USART_FLAG_RXNE)                       // RXNEIE ist immer an (uart_init); trotzdem wie bisher geprueft:
{                                               // (cr1 & USART_CR1_RXNEIE) -- Gleichwertigkeit vor Ersparnis
    if (sr & USART_FLAG_ORE) { ...saettigend zaehlen... }
    ch = UART_NAME->DR & 0xFF;                  // loescht RXNE und ORE (Folge SR, dann DR)
    ...Ring wie bisher...
}

if ((cr1 & USART_CR1_TXEIE) && (sr & USART_FLAG_TXE))   // TXE ist immer gesetzt, wenn DR leer -- nur mit Freigabe
{
    ...wie bisher; USART_SendData durch UART_NAME->DR = ch ersetzen...
    ...Abschalten: UART_NAME->CR1 &= ~USART_CR1_TXEIE statt USART_ITConfig...
}
```

- **Kein `ClearITPendingBit`:** RXNE löscht das Lesen von `DR`, TXE das Schreiben. Der Kommentar bei
  `:777-787` wird nachgeführt (er begründet heute die Reihenfolge mit `USART_GetITStatus`).
- **ORE vor `DR` lesen** bleibt Pflicht — jetzt aus der Kopie `sr`, die vor dem `DR`-Lesen entstand.
- **`USART_ITConfig` und `USART_SendData` im Senderzweig:** gleichwertig durch Registerzugriff ersetzen,
  damit der Normalpfad ohne `bl` auskommt. Die Abschaltung im Senderzweig ist eine
  Lese-Ändere-Schreibe-Folge auf `CR1`; sie läuft in der ISR selbst, ein konkurrierender Schreiber auf
  `CR1` ist `uart_putc()` beim Einschalten von TXEIE — dort prüfen, ob es unter gesperrtem Interrupt
  geschieht (**Prüfpunkt für den Umsetzer**, im Bericht nennen).
- **Beide Plattformen:** F103 und F411 haben dieselben Bitlagen in `SR` und `CR1` (RXNE 5, TXE 7, ORE 3,
  TXEIE 7, RXNEIE 5). Der Umsetzer belegt das an den beiden Gerätedateien des SPL, nicht aus dem
  Gedächtnis.

### 2.2 Prüfstand a15

Differenzieller Prüfstand: alte und neue ISR aus `uart-driver.h` (alter Stand per `git show`) gegen eine
Registerattrappe, je mit eigenem Ring. Die Attrappe bildet die Hardware nach: `DR` lesen löscht RXNE und
ORE, TXE ist nur lesbar und wird durch Schreiben von `DR` gelöscht, TXEIE steht in `CR1`. Zufallsfolgen
aus Empfang, Überlauf, Sendeauftrag, gleichzeitigem RXNE und TXE. Sabotagen nach AKI.1.

### 2.3 Messung (N-7)

**Empfehlung:** bindend ist die **statische** Messung — Befehle im Empfangs-Normalpfad der
ESP-Brücken-Instanz, aus `arm-none-eabi-objdump -d` der ELF, für F103 **und** F411, vorher und nachher,
einschliesslich der aufgerufenen SPL-Funktionen (für den Vorher-Wert deren Rumpf mitzählen). Den
Vorher-Wert baut der Lead aus dem Tag `release/3.2.32-3.2.33-1.4.97` in einem Worktree (R6), weil das
Release-ZIP keine ELF enthält. Am Gerät (nur F411) zeigt G1 die **Regression**, nicht den Gewinn: Bei
115'200 Baud kommt ein Zeichen alle rund 87 µs, die ISR braucht in beiden Fassungen nur einen Bruchteil
davon — `o`, `d` und `rx` ändern sich voraussichtlich nicht, und genau das ist zu zeigen.

**Verworfen:** Messbau mit `DWT->CYCCNT` auf der produktiven Uhr — eigener Flash eines Diagnosestands,
und die F103-Hälfte bliebe trotzdem ungemessen (kein F103 am Gerät).

---

## 3. A56 — Ambilight-Power im EEPROM

### 3.1 Speicherort (N-1)

**Empfehlung (a): Marke und Wert hinter dem bisherigen Ende, ohne Layoutversion.**

```
#define EEPROM_DATA_SIZE_AMBI_POWER        2                              // Marke 0xA5, dann 0 oder 1
#define EEPROM_DATA_OFFSET_AMBI_POWER      (EEPROM_DATA_OFFSET_DIMMED_AMBILIGHT_COLORS + EEPROM_DATA_SIZE_DIMMED_AMBILIGHT_COLORS)
#define EEPROM_DATA_END                    (EEPROM_DATA_OFFSET_AMBI_POWER + EEPROM_DATA_SIZE_AMBI_POWER)
```

Offset 1979–1980; auf dem F103 in der Seite 1976–1983, auf dem F411 in 1952–1983 — **ein**
Seitenschreiben je Wechsel. Die Tabelle im Kopfkommentar von `eeprom-data.h` wird mitgeführt.

**Warum ohne Layoutversion:** `read_configuration_from_eep()` liest nur bei `eep_version <= EEPROM_VERSION`
(`main.c:1224-1225`). Eine Anhebung macht jeden **Rückflash** zum Werksreset — alle Einstellungen,
IR-Codes eingeschlossen (`upgrade_eep_version()` schreibt die Vorgaben). Die beiden Byte dagegen liest und
schreibt eine ältere Firmware nie; sie lässt das übrige Layout unberührt. Die Marke unterscheidet
„nie geschrieben" (0xFF aus dem leeren EEPROM, oder Altbestand) von einem gültigen Wert. Ein zufälliges
0xA5 an dieser Stelle setzt voraus, dass dort je etwas stand — hinter `EEPROM_DATA_END` hat bisher
nichts geschrieben.

**(b) Layoutversion 3.0** (nicht 2.10: Anzeige „2.:", `main.c:1229`) ist der übliche Weg des Projekts und
bleibt die Alternative, wenn der Nutzer Rückflash-Sicherheit nicht braucht.

**Prüfpunkt für den Umsetzer:** Auf dem STM32F407 (`BLACK_BOARD`, `eeprom.c:13`) liegt das EEPROM im Flash.
Belegen, dass dessen Bereich die Offsets 1979–1980 abdeckt; sonst anhalten und vorlegen.

### 3.2 Schreiben und Lesen

- **Schreiben:** in `display_set_ambilight_power()` (die eine Schaltstelle, `display.c:2907`), über eine
  neue `static`-Funktion `display_save_ambilight_power()` nach dem Muster `display_save_brightness()`
  (`:3687-3693`). `eep_write()` vergleicht seitenweise und schreibt Unverändertes nicht (C3). Zusätzlich
  in `display_write_config_to_eep()`, damit `upgrade_eep_version()` nach einem EEPROM-Reset den Vorgabewert
  „an" schreibt.
- **Lesen:** in `display_read_config_from_eep()`, **unabhängig** von `eep_version` (der Platz hängt nicht
  an der Layoutversion): Marke 0xA5 und Wert 0 oder 1 ⇒ übernehmen; sonst „an". **Nur die Variable
  setzen**, nicht `display_set_ambilight_power()` rufen — die LED-Ausgabe übernimmt der normale Startpfad;
  ein `display_refresh_ambilight_leds()` vor der LED-Initialisierung wäre ein neuer Pfad.
- **Schaltstellen** (Bestandsliste per `grep`, ✔ SW): `set_display_power()` bei `do_sync_ambilight`
  (`main.c:1447`), Setter (`:1789`), Ambilight-Timer (`:4460`). Alle laufen über
  `display_set_ambilight_power()`; eine zweite Schreibstelle entsteht nicht.
- **Last:** ein Seitenschreiben je Wechsel, rund 15 ms (Checkliste, Zeile 26). Ein Display-Timer, der das
  Ambilight mitschaltet, löst es höchstens zweimal am Tag aus.
- **Nach einem Neustart im Aus-Fenster** schaltet A69 das Ambilight mit aus und speichert damit „aus" —
  folgerichtig: Es ist der Zustand vor dem nächsten Neustart.

### 3.3 Prüfstand a56

Lesen und Schreiben gegen ein EEPROM-Abbild: Marke fehlt (0xFF), Wert 2, Marke falsch, gültig 0 und 1,
EEPROM-Reset (Version 0 ⇒ `upgrade_eep_version()` schreibt „an"), Altbestand ohne die beiden Byte
(simulierter Rückflash: das übrige Abbild byte-gleich). Gegenprobe gegen 3.2.32: Nach „aus" und Neustart
ist der Wert „an" ⇒ FEHL.

---

## 4. A70 — Randfälle beim Nachholen des Nachtfensters

### 4.1 Zustand (Empfehlung)

`night_off_due` trägt heute zwei Bedeutungen — eine Frist (uptime) **oder** den Wert `NIGHT_OFF_AFTER_IP`.
Getrennt:

```
static uint32_t     night_off_due    = 0;   // 0 = nichts vorgemerkt; sonst uptime, ab der das Aus gilt (immer eine Frist)
static uint_fast8_t night_off_ip     = 0;   // 1 = das Aus wartet auf das Ende des IP-Lauftexts
```

- **Start im Aus-Fenster (M2, unverändert in der Wirkung):** `night_off_due = uptime + 60`.
- **IP-Lauftext startet (`main.c:4204-4217`) mit Vormerkung — (c):** `night_off_ip = 1`, und
  `night_off_due = uptime + T_ip + 10`. **`T_ip`** ist die Laufzeit des Lauftexts: (Länge + 2) ×
  (`TICKER_COLS` + 1) × `display.ticker_deceleration` / 64 s, auf ganze Sekunden aufgerundet.
  **Begründung:** Eine feste Frist schnitte bei grossem `ticker_deceleration` (bis 255, rund 8 min für eine
  IP) den Lauftext ab; eine dynamische Frist kostet eine Multiplikation.
- **Frist (`:4219-4222`):** gilt jetzt **immer**, auch mit `night_off_ip` — sie ist das Netz für (c).
- **Restore-Innenzweig (`:4868`):** `if (night_off_ip)` statt `== NIGHT_OFF_AFTER_IP`. Die vier
  Teilbedingungen davor und `! display_clock_flag` bleiben **wörtlich** (AKN.5).
- **`night_off_apply()`** setzt beide zurück; `set_display_power()` ebenso (jedes Schalten verwirft).
- `NIGHT_OFF_AFTER_IP` entfällt.

### 4.2 (d) Erstes `TIME` bei ungültiger RTC

`:3431-3434`: Steht der IP-Lauftext aus (`ip_ticker_pending`) oder läuft er gerade (`night_off_ip` kann
dafür nicht dienen; **Empfehlung:** ein Merker `ip_ticker_running`, gesetzt beim Start des IP-Lauftexts,
gelöscht im Restore-Innenzweig und beim Start jedes anderen Tickers), wird mit `at_start = 2` vorgemerkt —
derselbe Weg wie beim Start. Läuft der IP-Lauftext gerade, setzt der Vormerkpfad zusätzlich `night_off_ip`
und die Frist aus §4.1. Sonst wie heute `night_apply (0xFFFF, 1)`.

### 4.3 (e) Fremder Ticker

Jede Stelle in `main.c`, die einen Ticker startet und `pending_ticker_restore = 1` setzt (Bestandsliste
per `grep display_set_ticker`, ✔ SW mindestens `:2315`, `:3520`, `:3527`, `:4719`, `:4853`), prüft **vor**
`display_set_ticker()`: Ist `night_off_ip` gesetzt, zuerst `night_off_apply()`. Danach ist das Display aus,
`display_set_ticker()` tut nichts (`display.c:4954`), der Restore läuft leer durch. **Empfehlung:** eine
kleine Hilfsfunktion `start_ticker (text)` in `main.c` statt fünf Kopien derselben Prüfung — sie spart Flash
und macht die Bestandsliste überprüfbar. Der IP-Lauftext selbst bleibt aussen vor.

### 4.4 L399 — reiner DCF77-Betrieb

Im DCF77-Zweig (`:4341-4372`) nach der Übernahme: `if (night_wm == 0xFFFF) night_apply (0xFFFF, 1);` —
dieselbe Regel wie beim ersten `TIME` (Ent-2 (2) des Vorpakets: beim ersten bestätigten Zeitsignal
auswerten, nur ein Aus). **Begründung für „nur Aus":** DCF77 empfängt nur bei ausgeschaltetem Display und
Ambilight (`main.c:891`, `:1471-1478`). Liegt laut Tabelle gerade ein Ein-Fenster, kann das Display vom
Nutzer **absichtlich** aus sein — gerade, damit DCF77 empfängt. Ein automatisches Einschalten widerspräche
dem; der nächste Ein-Timer schaltet regulär. Der eigentliche Gewinn: `night_wm` ist danach gesetzt, die
Sprungerkennung (`:4437-4442`) greift.

### 4.5 Prüfstand a70

Auf der Grundlage von a69 (Zeitachse mit `uptime`, Tickerzustand, Restore-Bedingung als Attrappe):
(c) Restore kommt nie ⇒ Aus nach `T_ip + 10`; (d) erstes `TIME` während des IP-Lauftexts ⇒ kein Aus
bis zu dessen Ende; erstes `TIME` lange nach dem IP-Lauftext ⇒ sofort; (e) Wetterticker während des Wartens
⇒ Aus vor dem Ticker; L399 ⇒ `night_wm` gesetzt, Sprung danach nachgeholt. Je Gegenprobe gegen 3.2.32.

---

## 5. A80 — Icon bei ausgeschaltetem Display verwerfen

- **Empfang (`main.c:3551-3558`):** Nach `display_read_icon () == 2`: Ist `display.display_power_is_on`
  0, `do_display_icon` **nicht** setzen, `show_icon_stop_time` **nicht** setzen,
  `log_message ("icon: verworfen, display aus")`. Sonst wie heute.
- **Display geht während eines Icons aus (Empfehlung, durch die Entscheidung gedeckt: „bei
  ausgeschaltetem Display wird ein Icon verworfen, nicht vorgemerkt"):** In `set_display_power()` beim
  Ausschalten `display_icon_cancel()` rufen — neue Funktion in `display.c`, die den statischen Zustand von
  `display_icon()` auf `DISPLAY_ICON_STATE_CLEAR_DISPLAY` setzt und `do_display_icon = 0`.
  `show_icon_stop_time` läuft von selbst ab (`main.c:4753-4767`, höchstens `icon_duration` Sekunden).
  **Warum nötig:** Sonst bliebe genau der Stillstand aus L422 für diesen Fall bestehen, und das nächste
  Icon begänne mit dem alten Zähler.
- **Restore-Bedingung:** unverändert. A80 sorgt nur dafür, dass `do_display_icon` bei ausgeschaltetem
  Display nicht gesetzt bleibt; die Teilbedingung `! display.do_display_icon` bleibt stehen (Invariante).
- **Diagnosezeile `icon_freeze`** (`main.c:4111-4122`) bleibt; nach A80 darf `ENTER` nicht mehr vorkommen —
  die Wache meldet es ab S2 als Alarm.

---

## 6. B55 — „Messwert übernehmen" meldet das Ergebnis des STM

### 6.1 Leitungsformat (STM → ESP)

```
RPCR xx r\r\n        xx = RPC-Index, zwei Hexziffern (LDR_MIN_VALUE_RPC_VAR, LDR_MAX_VALUE_RPC_VAR)
                     r  = 0 uebernommen, 1 abgewiesen, 2 Messung fehlgeschlagen
```

- Gesendet wie `LOG ` über `esp8266_uart_puts()` (`esp8266.c:871-907`), **ohne** Quittung, **nach**
  `var_send_ldr_*_value()` — so ist die ESP-Kopie des Werts schon nachgeführt, wenn die HTTP-Antwort
  hinausgeht.
- **Warum eine eigene Zeile statt einer neuen Variable:** Eine neue `NUM`-Variable verschöbe den
  Variablensatz und berührte Abgleich, Abschlussmarke und `var-crc` (A42, C26). Eine unquittierte Zeile
  kann verloren gehen — dann meldet der ESP „keine Rückmeldung" (Kennung 11), ehrlich statt geraten.
- **Warum nicht am unveränderten Wert erkennen:** Gleicher Wert heisst auch „gleich gemessen" (Problem §5).
- **Warum nicht die Logzeile `ldr: minimum abgewiesen` lesen:** Die Logspiegelung ist Diagnose, kein
  Protokoll; A19 sieht vor, sie umzubauen. B55 hinge dann still an einer Logzeile.
- **STM:** `ldr_set_min_value()`/`ldr_set_max_value()` (`ldr.c:170-245`) geben das Ergebnis zurück
  (Rückgabetyp ändern oder Ausgabeparameter; heute liefern sie den Grenzwert, den `main.c:1620-1634`
  nicht verwendet — **Prüfpunkt:** weitere Aufrufer per `grep`). `main.c` sendet danach `RPCR`.
- Der alte ESP übergeht die Zeile (unbekanntes Präfix, keine Ausgabe) — S2 kann vor E2 eingespielt werden.

### 6.2 ESP: warten, ohne die Brücke anzuhalten

- **Brücke als Funktion:** Der Block `while (Serial.available ()) { … }` am Ende von `loop()`
  (`ESP-uclock.ino:1440-2255`) wird zur Funktion `stm_bridge_poll()`; `loop()` ruft sie an derselben Stelle.
  Die drei `static`-Variablen (`cmd_buffer`, `cmd_len`, `cmd_trunc`, `:1403-1405`) wandern in die Funktion.
  **Der Rumpf bleibt, wo er ist** — es ändern sich nur Kopf und Ende (Diff klein halten, **gemischte
  Zeilenenden byte-genau erhalten**, CLAUDE.md).
- **Sperre:** ein `static`-Merker in `stm_bridge_poll()`; ein verschachtelter Aufruf kehrt sofort zurück.
- **`RPCR` auswerten:** Zweig neben `LOG ` — legt Ergebnis und eine laufende Nummer je RPC-Index ab.
- **Warten im Handler:** `rpc()` senden, Nummer merken, bis 2'000 ms: `stm_bridge_poll(); yield();` bis
  die Nummer wechselt. **2'000 ms**, weil die Messung bis 100 ms, das EEPROM-Schreiben rund 15 ms und die
  quittierte Wertmeldung im Normalfall Millisekunden braucht; die 3-s-Frist des STM für die Quittung wird
  durch das Pumpen gar nicht erst erreicht.
- **Antworten:** `0` ⇒ `{"ok":true,"value":<neuer Wert>}`; `1` ⇒ `http_json_error (10, "rejected: …")`;
  `2` ⇒ `http_json_error (HTTP_API_ERROR_ACTION_FAILED, "measurement failed")`; Frist ⇒
  `http_json_error (11, "no answer from stm")` und `- rpc keine Antwort #n idx=xx` (gedrosselt wie
  `var ignoriert`, Mitschnitt **und** `/api/stm32_log`, L185).
- **Neue Kennungen** (`http.cpp:244-273`, Kommentar dort nachführen): **9** Query zu lang (C46), **10**
  vom STM abgewiesen, **11** keine Antwort vom STM. Jede braucht einen Eintrag in **beiden** i18n-Tabellen
  (Kommentar `:240-241`).

### 6.3 PWA

Die Aufrufer (`app.js:13159-13165`, über `runSimpleAction()`) unterscheiden die Kennungen 10, 7, 11 und
zeigen die Texte aus AKL.3. Das Neuladen nach 1,2 s bleibt; bei `ok` steht der neue Wert dann schon in der
ESP-Kopie.

---

## 7. C8 — Kommandopuffer und Ring (N-2)

- **Puffer:** `CMD_BUFFER_SIZE` 260 (4 Byte `LOG `, 255 Zeichen, Nullbyte), Kommentar mit Herleitung. Die
  Entscheidung nannte 256 — mit 256 würden Zeilen von 252 bis 255 Zeichen weiter gekappt.
- **Ring, Empfehlung (a):** `stm32_log_append()` legt eine Zeile über 120 Zeichen in **Folgeeinträge**:
  der erste mit 119 Zeichen und `~` am Ende (wie heute „gekappt"), jeder folgende beginnt mit `~` und trägt
  bis zu 119 weitere. Eine 255-Zeichen-Zeile belegt drei der 32 Einträge. **RAM-neutral** — der Preis ist
  weniger Rückschau, solange lange Zeilen kommen; die sind selten (Wetter, Vorhersage, Dateinamen).
- **Warum nicht ein Ring mit variabler Zeilenlänge:** gleiche RAM-Wirkung, aber `stm32_log_get_line()`
  und `/api/stm32_log` müssten umgebaut werden; die Folgeeinträge brauchen dort nichts.
- Die PWA zeigt Folgeeinträge als eigene Zeilen; ein Zusammenfügen ist nicht Teil.
- **STM-Seite: nicht nötig.** `STRBUF_SIZE` bleibt 256.

---

## 8. C6, C6v, C6b — zerstörende Endpunkte und Schlüssel

### 8.1 POST im ESP (E1)

- **POST-Zweig (`http.cpp:14628-14659`):** Steht der Pfad in einer Liste `http_post_api_paths[]` (die vier
  Wartungsendpunkte und `/api/network_secrets`), liest der Zweig die Kopfzeilen bis zur Leerzeile —
  dieselbe Schleife wie der GET-Zweig (`:14662-14681`), um `Sec-Fetch-Site` ergänzt — und ruft dann
  `http (sPath, sParam)`. Upload-Pfade bleiben bei `http_post()`, das ihre Kopfzeilen selbst liest. Ein
  anderer POST-Pfad bekommt 404.
- **Methode merken:** `http_request_method` (GET/POST), damit Handler und E3 sie prüfen können.
- **Rumpf:** wird nicht gelesen; übrig Gebliebenes verwirft das bestehende Leeren vor `stop()` (`:14725`).

### 8.2 Herkunft

`http_request_is_cross_origin()`: wahr, wenn `Sec-Fetch-Site` gesetzt und weder `same-origin` noch `none`
ist. Abgewiesen wird mit 403, wenn `http_request_is_embedded_subresource()` **oder**
`http_request_is_cross_origin()` wahr ist. **Begründung:** POST allein hält fremde Seiten nicht ab — ein
Formular-POST von fremder Herkunft ist ein „einfacher" Request ohne Vorabprüfung, und eine
Formular-Navigation trägt `Sec-Fetch-Dest: document`, das die bestehende Liste nicht fängt. `Sec-Fetch-Site`
fängt beides. Die PWA und die Legacy-Seiten laufen vom Gerät selbst (`same-origin`), curl sendet keine
Kopfzeile (`none` bzw. leer) — beides bleibt zugelassen. **Grenze, bewusst:** ältere Browser ohne
`Sec-Fetch-*` bleiben ungeschützt, wie heute.

**Geprüft für:** die vier Wartungsendpunkte, `network_secrets`, Legacy `/fs?action=format`,
`/fs?action=remove`, `/?action=rsteeprom2` (N-4). Der GET-Zweig fängt die Kopfzeile bereits ab
(`:14677-14680`) und wird um `Sec-Fetch-Site` ergänzt.

### 8.3 C6b

- `/api/eeprom_settings`: `key` und `ap_key` als `""`, dazu `key_set`, `ap_key_set`. **Warum leer statt
  Platzhalter:** `eeprom_settings_set` übergeht leere Felder (`http.cpp:9379-9395`) — ein Weg, der den Wert
  versehentlich zurückschreibt, richtet keinen Schaden an. Ein Platzhalter wie `********` würde als echter
  Schlüssel gespeichert.
- `/api/network_secrets` (POST, Herkunft, `Cache-Control: no-store`): `{"ok":true,"key":…,"ap_key":…}`.
  In der URL-Liste des Geräts als `network_secrets_url` (bei `:12715-12723`), damit die PWA ihn über
  `createConfiguredUrlGetter()` holt.
- **PWA (P1):** `ensureBackupExportState()` holt zusätzlich `network_secrets` (POST); fehlt die Antwort,
  bricht der Export ab (eigene Kennung, Text in beiden Tabellen). `getNetworkBackupMeta()` liest die
  Schlüssel von dort.
- **Werkzeuge (Lead):** `snapshot-device.sh` liest den Schlüssel nicht mehr aus `eeprom_settings` — es
  vermerkt `key_set`; `tools/preview/server.py` liefert beide Endpunkte.

### 8.4 Zwei Stufen (N-3)

| Stufe | ESP | PWA, Werkzeuge |
|---|---|---|
| **E1** | POST und GET angenommen; GET mit Herkunftsprüfung und `- veraltet: GET <pfad>` (gedrosselt) | unverändert GET |
| **G3 → E3** | — | P1: PWA auf POST; nach G3: `flash-stm.sh`, `smoke-device.sh`, Vorschau auf POST (AKZ.11) |
| **E3** | GET auf die vier und `network_secrets` ⇒ 405 `{"ok":false,"error":"method"}` | — |

`network_secrets` ist neu und nimmt von Anfang an **nur** POST an.

### 8.5 PWA (P1)

`apiFetch (url, { method: "POST" })` an den sechs Aufrufstellen (`app.js:5393`, `:11787`, `:11851`,
`:11855`, `:11875`, `:12412`) und beim Löschen über `fs_remove_base_url`. **Empfehlung:** eine Hilfsfunktion
`apiPost (url, options)`, damit die Methode nicht an einer Stelle fehlt; der Prüfstand zählt die Aufrufe
der vier Pfade nach Methode.

---

## 9. C19 — Legacy-Update-Quelle (nur L311)

An **beiden** Stellen (`http.cpp:6724-6731`, `:7520-7527`): leer ⇒ abweisen; länger als
`MAX_UPDATE_HOST_LEN`/`MAX_UPDATE_PATH_LEN` (Feldgrösse 64, also 63 Byte) ⇒ abweisen; Meldung auf der Seite
wie die übrigen Legacy-Meldungen (`alert_message`-Muster, `:4681-4683`); Wert unverändert. **Empfehlung:**
dieselbe Prüffunktion, die die API für `update_host_set`/`update_path_set` verwendet (L48), statt einer
dritten Fassung. Sonst nichts am Legacy-Zweig.

---

## 10. C46 — Query über 255 Zeichen

### 10.1 ESP (E1)

In `http()` vor dem `strncpy` (`:13795-13798`) und im POST-Zweig vor `:14648`: Ist `strlen (const_param)`
grösser als 255 und beginnt der Pfad mit `/api/`, antwortet der ESP mit Kennung 9 und ruft keinen Handler.
Logzeile `- abgewiesen: query <n> Zeichen` (ohne Inhalt — die Query trägt Werte, L179). Legacy-Pfade
unverändert.

### 10.2 PWA (P1)

- **Prüfung vor dem Senden:** zentral in `apiFetch()`: Query-Teil der URL (nach `?`, so wie er gesendet
  wird) länger als 255 ⇒ kein Request, Fehler mit Kennung 9 (gleiche Meldung wie vom Gerät).
- **Aufteilen im Backup-Import:** `eeprom_settings_set` je Feld einzeln (`ssid`, `key`, `ap_ssid`,
  `ap_key`), **jedes** mit demselben `boot_as_ap` — der ESP setzt das Flag bei jedem Aufruf neu
  (`http.cpp:9397-9407`), und leere Felder übergeht er. Ein einzelnes Feld ist höchstens
  `key=` + 189 + `&boot_as_ap=on` ≈ 207 Zeichen.
- Andere Setter: die Prüfung aus dem ersten Punkt genügt; ein Wert, der darüber liegt, wird mit Meldung
  abgewiesen statt gekürzt.

---

## 11. C59 — NTP-Antwort an die Anfrage binden

- `send_ntp_packet()` (`ntp.cpp:117-136`): 8 Byte Zufall aus `RANDOM_REG32` (Hardware-Zufallsregister des
  ESP8266) in Byte 40–47 (Transmit), gemerkt in `ntp_nonce[8]`; `ntp_pending = 1`,
  `ntp_pending_since = millis ()`.
- `ntp_poll_time()`: nach den bestehenden Prüfungen und vor LI: `! ntp_pending` oder älter als 5'000 ms ⇒
  `ntp_reject ("ungefragt", -1)`; Byte 24–31 ungleich `ntp_nonce` ⇒ `ntp_reject ("originate", -1)`. Nach
  einer angenommenen Antwort `ntp_pending = 0`.
- **Warum Byte 40–47:** RFC 5905 — der Server kopiert das Transmit-Feld der Anfrage ins Originate-Feld der
  Antwort. **Grenze, bewusst:** Gegen einen Angreifer, der mitliest, hilft es nicht (L406).
- Die Bestätigungsregel des STM fragt nach 15 s erneut — jede Anfrage bekommt eine neue Zahl.

---

## 12. A79 — ESP-Empfangspuffer

`Serial.setRxBufferSize (1024);` unmittelbar **vor** `Serial.begin (115200)` (`ESP-uclock.ino:160`). Im Core
3.1.2 merkt sich `setRxBufferSize()` vor `begin()` nur die Grösse, `begin()` legt den Puffer an
(**Prüfpunkt für den Umsetzer:** an `HardwareSerial.cpp` des Core belegen). Keine Ausgabe vor der
`CAP`-Zeile (`:166-171`). Der Kommentar bei `:474-479` („KEIN setRxBufferSize") wird nachgeführt.

---

## 13. B48 — Fehlerzeile am Feld

- **PWA (`pwa-developer`):** `reportInvalidInput()` (`app.js:6614`) und `readTimeInputOrReport()`
  (`:13375`): `aria-invalid="true"`; Fehlerzeile als `<p class="field-error" id="<feld-id>-error"
  role="alert">` direkt nach dem Feld, per `textContent` (keine Fremddaten in `innerHTML`);
  `aria-describedby` um diese ID **ergänzen**, vorhandene IDs behalten. Hat das Feld keine ID, vergibt die
  Funktion eine. Ein `input`-Ereignis im Feld entfernt Attribut, ID-Ergänzung und Zeile. Banner und Fokus
  bleiben.
- **UI (`ui-developer`):** Klasse `.field-error` in `styles.css` (Farbe aus den bestehenden
  Fehlervariablen, Kontrast wie L245 gemessen), keine Änderung am Markup nötig. **Empfehlung:** die Zeile
  per JS anlegen statt 30 leerer Platzhalter im Markup.
- **Warum `role="alert"`:** Der Screenreader sagt die Zeile an, ohne dass der Fokus wandert.
- **Prüfbar nur am iPhone (Nutzer, AKF.2):** ob die Zeile bei offener Tastatur sichtbar bleibt.

---

## 14. B8 — deutsche Texte in die i18n-Tabelle

1. **Zählen (Lead, Schritt 0):** Werkzeug nach AKD.1 mit Kandidatenliste und einmaliger Zuordnung durch
   den Lead (Ausnahmeliste mit Grund: Logzeilen an die Konsole, Schlüssel, Kennungen, technische
   Bezeichner). **Warum nicht über Satzwörter oder Umlaute:** „Speichern" hat weder das eine noch das
   andere — genau so blieb die Zahl bisher ungenau.
2. **Umsetzen:** `app.js` und `i18n/en.json` (`pwa-developer`), `index.html` (`ui-developer`, über
   `data-i18n*`). Schlüssel nach dem bestehenden Schema (`modul.zweck`). Englische Texte vom
   `pwa-developer`; Umschrift in deutschen Texten ist verboten (S8).
3. **Halten:** Das Werkzeug wird Guardrail-Stufe (AKD.2) — ohne sie wächst der Bestand still wieder.

---

## 15. F3 — Umfangsprüfung des Backups

- **Quelle 1:** der Dispatch in `http.cpp` (`strcmp (path, "/api/…_set")`).
- **Quelle 2, Empfehlung:** die Setter, die der Backup-Import aufruft. Statisch über die Funktionen des
  Imports: Das Werkzeug führt eine **benannte Liste** der Importfunktionen (gibt sie aus) und sammelt darin
  jeden Aufruf eines `get…Url()`-Getters bzw. jedes `"/api/…_set"`; den Getter löst es über die
  Konfigurationstabelle (`app.js:10533` ff.) auf. Fehlt eine benannte Funktion, ist das ein FEHL — sonst
  sähe die Prüfung ihren Gegenstand nicht (Gattung L235).
- **Ausnahmeliste:** `tools/checks/backup-umfang.ausnahmen`, je Zeile `endpunkt  grund`. Kandidaten aus dem
  Code (zur Einordnung durch den Lead, ◐): Aktionen statt Zustand (`ticker_set`), Update-Quelle (wird
  bewusst separat geführt?), Testfunktionen.
- **Einmalige Gegenprobe dynamisch:** ein Import in der Vorschau (`tools/ui-mess`), die aufgerufenen
  `*_set` gezählt und mit Quelle 2 verglichen — belegt, dass die statische Sammlung vollständig ist.

---

## 16. E36 — Logwache mit Lebenszeichen

- **Ursache (AKW.1):** `pmset -g log` für das Fenster; **zusätzlich prüfen**, ob in der Sitzung
  `caffeinate` lief (L423 sagt: nein). Ergebnis in L423, belegt oder nicht belegt.
- **Herzschlag:** je Runde `printf '%s %s\n' "$(date +%s)" "$runde" > .git/watch-log.lebt$WL_SFX`.
- **`--pruefe`:** liest die Datei, Alter gegen 3 × Intervall (das Intervall steht mit in der Datei),
  Exit 0/1 mit Meldung. **Stop-Hook** (`.claude/`, Lead): ruft es, wenn `geraet-gemessen` gesetzt ist.
- **Selbstmeldung:** Wanduhr vor `sleep` merken; nach dem Aufwachen Differenz > 3 × Intervall ⇒ Alarm mit
  Dauer. Das deckt den Ruhezustand ab — die Wache meldet ihn beim Weiterlaufen, statt ruhig fortzufahren.
- **Frist für `log.sh`:** macOS hat kein `timeout`; **Empfehlung:** Aufruf im Hintergrund, `wait` mit
  eigenem Zähler und `kill` nach 20 s, Ausgabe über eine Zwischendatei. Abgelaufen ⇒ Alarm, Runde läuft
  weiter.
- **`caffeinate`:** **Empfehlung:** Die Wache startet sich selbst unter `caffeinate -i`, wenn verfügbar
  (macOS), und meldet es in der Kopfzeile — der Ruhezustand ist der naheliegende Verdacht.

---

## 17. Gerätenachweise

### 17.1 Ablauf je Gerätelauf

Wache vor dem Einspielen starten (DIR-013), `--pruefe` am Ende (AKZ.5), Smoketest sofort ins
Messprotokoll (AKZ.4). Nach jedem ESP-Update `install-app.sh --check` und die Update-Quelle (DIR-009,
DIR-017).

### 17.2 Was schreibt, was braucht Freigabe

| Nachweis | Art | Rückstellung | Freigabe |
|---|---|---|---|
| G0/G1 Setter-Burst 14 × Wetterort | schreibend | Wetterort zurück | **N-5** |
| G0/G3/G4 Heap, `stm32_log`, `eeprom_settings`, `device_ready` mit langer Query | lesend | — | — |
| Einspielen S1, S2, E1, E2, E3, P1, P2 | Update | — | **erteilt** |
| G2 Ambilight aus, STM-Reset, Ambilight zurück | schreibend + Reset | Vorher-Wert | Reset erteilt; Setter **N-5** |
| G2 Display aus, `weather_get_now`, Display an | schreibend, Display kurz dunkel | Display an | **N-5** |
| G2 optional: STM-Reset im Aus-Fenster mit Ticker, A70 (e) | Reset nachts | — | **N-5 (eigens)** |
| G3 `maintenance_reset_stm32` per POST und die beiden Abweisungsproben | Reset | — | **erteilt** |
| G4 zwei STM-Resets für A79 (ESP startet mit) | Reset | — | **erteilt** |
| G4 `weather_get_forecast` für C8 | auslösend (Lauftext) | — | **N-5** |
| G5 LDR-Proben (AKL.4) | schreibend, EEPROM | 0/4095 | **N-5** |
| G5 Backup-Export im Browser | lesend | — | — |
| G6 GET `maintenance_reset_stm32` ⇒ 405 | lesend; schlüge es fehl, wäre es ein STM-Reset | — | Reset erteilt |
| G7 iPhone, drei Felder | Nutzer | — | Nutzer |
| T1, T2 `pwa-tester` mit S-Fällen | schreibend nach Testplan | nach Testplan | **N-5** |

### 17.3 Nie am Gerät — nur am Prüfstand

`maintenance_format_fs`, `maintenance_reset_eeprom`, `fs_remove` (auch mit nicht vorhandener Datei:
der Hook `no-danger.py` weist es zu Recht ab), Legacy `/fs?action=format`, `/fs?action=remove`,
`/?action=rsteeprom2` — **auch nicht** mit Abweisungs-Kopfzeile: Versagt die Prüfung, sind PWA oder
Einstellungen weg. Legacy `saveuphost`/`saveuppath` mit leerem oder überlangem Wert — versagt die Prüfung,
zeigt die Update-Quelle auf den Server des Ursprungsprojekts. NTP-Fälschung (C59), A70 (c), (d), L399
(nicht herstellbar).

---

## Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `src/uart/uart-driver.h` | A15 | `stm-developer` |
| `src/main.c` | B55 (`RPCR`), A70, A80 (Empfang, Abbruch beim Ausschalten) | `stm-developer` |
| `src/ldr/ldr.c`, `ldr.h` | B55 (Ergebnis) | `stm-developer` |
| `src/display/display.c`, `display.h` | A56 (Speichern, Laden), A80 (`display_icon_cancel()`) | `stm-developer` |
| `src/eeprom/eeprom-data.h` | A56 (Platz) | `stm-developer` |
| `src/esp8266/esp8266.c` | B55 (Sendefunktion für `RPCR`, falls nicht in `main.c`) | `stm-developer` |
| `ESP8266/ESP-uclock/http.cpp` | C6, C6v, Legacy-Herkunft, C6b, C46, C19 (E1); B55 (E2); E3 | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | B55 (`stm_bridge_poll`, `RPCR`), C8, A79 | `esp-developer` |
| `ESP8266/ESP-uclock/vars.cpp`, `vars.h` | B55 (Ergebnisablage), falls dort | `esp-developer` |
| `ESP8266/ESP-uclock/ntp.cpp` | C59 | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | C6, C6b, C46, B55 (P1); B48, B8 (P2) | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/i18n/en.json` | Kennungen 9–11, B55-Texte (P1); B8 (P2) | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/styles.css` | B48 `.field-error` | `ui-developer` |
| `ESP8266/ESP-uclock/data/app/index.html` | B8 (`data-i18n*`) | `ui-developer` |
| `tools/watch-log.sh`, `.claude/` (Stop-Hook) | E36, neue Wachzeilen | Lead |
| `tools/checks/**` | F3, B8-Werkzeug, Prüfstände a15, l55, l55e, a56, a70, a80, c6, c46, c19, c8, ntp | Lead (Prüfstände vom Umsetzer im Scratchpad gebaut) |
| `tools/flash-stm.sh`, `smoke-device.sh`, `snapshot-device.sh`, `preview/server.py`, `hooks/no-danger.py`, `checks/hooks.sh` | C6, C6b | Lead |
| Versionsdateien | je Release | `release-engineer` |
| `BEFUNDE.md` | Messprotokoll (Lead), Befunde (`doc-writer`) | Lead / `doc-writer`, nie gleichzeitig |
| `TESTPLAN-PWA.md` | B55, C6, C46, B48, Wache | `doc-writer` |
| `CLAUDE.md`, `CHANGELOG.md` | Gefahrentabelle (POST), Einträge | Lead |

---

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Werden die Wetter-Endpunkte eingehalten?
Bleibt die Restore-Bedingung vollständig? Greift die Änderung an der richtigen Schicht an?

> **Legacy bleibt parallel und bedienbar.** Legacy behält GET; neu ist nur die Herkunftsprüfung an drei
> zerstörenden Aktionen (§8.2) und die Abweisung leerer und überlanger Update-Quellen an zwei Stellen
> (§9) — beides ändert nichts für einen Nutzer, der die Legacy-Seite selbst bedient (`same-origin`). Der
> übrige Legacy-Zweig bleibt nach Entscheidung des Nutzers stehen. **Wetter:** Kein Pfad berührt die
> Wetter-Endpunkte; G2 und G4 rufen `/api/weather_get_now` bzw. `…_forecast` als Auslöser, kein Rückbau.
> **Restore-Bedingung:** A70 und A80 lassen die vier Teilbedingungen und `! display_clock_flag` wörtlich
> stehen (AKN.5); geändert wird nur der Vergleich im Innenzweig (§4.1) und, dass `do_display_icon` bei
> ausgeschaltetem Display nicht mehr gesetzt bleibt (§5) — keine Teilbedingung wird überflüssig, keine
> wird entfernt. **`.gz` und Grösse > 0:** P1 und P2 gehen über `make app-gz` und `install-app.sh`
> wie bisher (R2). **Schicht:** B55 legt die Entscheidung dorthin, wo sie fällt (STM) und reicht sie
> durch, statt sie in der PWA aus einem neu geladenen Wert zu raten. C46 weist im ESP ab **und** prüft in
> der PWA vorher — der ESP ist die Grenze, die PWA vermeidet nur unnötige Fehler. A79 und C8 beheben
> Verluste der Brücke am ESP, nicht durch Drosselung in der PWA.

**Scalable systems** — Wie viele STM-Kommandos erzeugt die Aktion? Wie viele Byte pro Minute zusätzlich
auf der UART? Bleibt jeder Pfad unter 20 s Watchdog? Wie lange blockiert der Hauptloop?

> **STM-Kommandos:** B55 unverändert ein RPC je Druck; dazu **eine** unquittierte Zeile `RPCR` (rund
> 11 Byte) vom STM. A56 erzeugt kein Kommando (die ESP-Kopie läuft wie bisher über A68). C46 teilt den
> Netzwerkimport in bis zu vier Aufrufe statt einem — einmal je Import, auf der ESP-Seite (EEPROM des
> ESP), ohne STM-Kommando. **UART:** neu sind `RPCR` (selten), `icon: verworfen` (je verworfenes Icon),
> `- veraltet: GET` (gedrosselt, nur E1 bis E3), `- abgewiesen: query` (je Abweisung), `- rpc keine
> Antwort` (gedrosselt). Keine Zeile je Anfrage oder je Sekunde. Eine NTP-Antwort, die heute angenommen
> wird, wird auch danach angenommen — kein Zusatzverkehr. **Watchdog STM:** A56 schreibt eine Seite
> (~15 ms) je Wechsel; A70/A80 sind Zustandsänderungen ohne Schleife; A15 verkürzt die ISR. **ESP:** Das
> Warten in B55 ist auf 2'000 ms begrenzt und ruft `yield()` je Durchlauf; der ESP-Hauptloop steht in
> dieser Zeit für HTTP still (wie bei jedem blockierenden Handler) — eine Frage des Nutzens gegen die
> Entscheidung des Nutzers „API wartet ab", bewusst in Kauf genommen. **Hauptloop STM:** keine neue
> Blockade über die eine EEPROM-Seite hinaus. **Feste Grenzen:** 255 Zeichen Query (jetzt sichtbar statt
> still), 260 Byte Kommandopuffer, Ring 32 × 121 (unverändert), RX-Puffer 1'024.

**Secure by design** — `escapeHtml` bei jedem `innerHTML`? Fremddaten als nicht vertrauenswürdig
behandelt? Keine Credentials in Logausgaben?

> **Credentials:** C6b nimmt den Schlüssel aus dem allgemein lesbaren Endpunkt; der neue Endpunkt ist POST,
> herkunftsgeprüft und nicht zwischenspeicherbar. Kein neuer Log gibt einen Wert aus: `- abgewiesen:
> query` nennt nur die Länge, `- rpc keine Antwort` nur den Index; Prüfstände und Berichte nennen den
> Schlüssel nur als Prüfsumme (AKS.6). **Fremddaten:** Die NTP-Antwort ist an die eigene Anfrage gebunden
> (C59). Die Fehlerzeile aus B48 setzt `textContent`, nie `innerHTML`; B8 setzt Übersetzungen weiter über
> den bestehenden Weg (`data-i18n`, `textContent`). **Neue schreibende Endpunkte:** keiner; der neue
> Endpunkt liest. Die zerstörenden Endpunkte werden enger (POST, `Sec-Fetch-Site`), und die bisher
> ungeschützte Legacy-Aktion `rsteeprom2` bekommt erstmals eine Prüfung. **Grenze, benannt:** Gegen curl
> im LAN und gegen alte Browser schützt nichts davon (wie bisher, `http.cpp:471-473`).

**Stable & reliable** — Wird jeder Fehler ausgewertet? Leere `catch`? Stille Verwerfungen? Ist der Zustand
nach Abbruch mitten in einer Sequenz definiert? Wird ein gesetztes Flag auf jedem Pfad wieder aufgelöst?

> **Erfolgsmeldung hängt am Ergebnis:** B55 meldet „gespeichert" nur bei `RPCR … 0`; jede andere Antwort,
> auch das Ausbleiben, ist eine eigene Meldung. C46 kürzt nicht mehr still. C19 biegt nicht mehr still.
> **Stille Verwerfungen:** Ein verworfenes Icon wird geloggt (A80), eine verworfene NTP-Antwort mit Grund
> (C59), eine abgewiesene Query mit Länge (C46). **Flags auf jedem Pfad:** `night_off_ip` und
> `night_off_due` werden in `night_off_apply()` und `set_display_power()` gelöscht, und die Frist gilt
> jetzt immer (§4.1) — das war der Kern von (c). `ip_ticker_running` wird im Restore-Innenzweig und beim
> Start jedes anderen Tickers gelöscht. Der Merker „Anfrage offen" (C59) läuft nach 5 s ab, auch ohne
> Antwort. Die Sperre in `stm_bridge_poll()` wird auf jedem Rückweg gelöst (eine Ausgangsstelle).
> `display_icon_cancel()` setzt Zustand **und** `do_display_icon`. **Abbruch mitten in einer Sequenz:**
> Der aufgeteilte Netzwerkimport (C46) setzt je Feld; bricht er ab, sind die bis dahin gesetzten Felder
> vollständig, kein Feld halb — und `boot_as_ap` steht nach **jedem** Aufruf auf dem Wert des Backups, nicht
> auf einem Zwischenstand. **Leere `catch`:** keine neuen; der Export-Abbruch in C6b meldet mit Kennung.
> **Leseschleife über `WiFiClient`:** Die neue Kopfzeilenschleife im POST-Zweig (§8.1) übernimmt die
> Schleife des GET-Zweigs, die die Regel `available()` nach `connected() == 0` bereits einhält (C53) —
> **Prüfpunkt im Review.**

---

## Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| A56 mit Layoutversion 3.0 | Rückflash setzt alle Einstellungen zurück (N-1 b, bleibt wählbar) |
| A56 im ungenutzten Bereich der alten Overlay-Intervalle (Offset 407) | Dort steht Altbestand; ohne Layoutversion nicht von einem gültigen Wert zu unterscheiden |
| B55 am unveränderten Wert erkennen | „gleich gemessen" sähe aus wie „abgewiesen" (Problem §5) |
| B55 über die Logzeile `ldr: … abgewiesen` | Logspiegelung ist kein Protokoll (A19) |
| B55 als neue `NUM`-Variable | verschiebt den Variablensatz, berührt Abgleich und `var-crc` |
| B55 asynchron (anstossen, Ergebnis abfragen) | widerspricht der Entscheidung „API wartet ab" |
| C8 Ringzeilen 255 | 4'320 Byte RAM (N-2 c) |
| C6 nur POST ohne `Sec-Fetch-Site` | fremde Formular-POSTs gingen durch (§8.2) |
| C6 Legacy auf POST | POST-Zweig liest keine Formularrümpfe; Legacy ist die Stabilitäts-Referenz |
| C46 Puffer auf 512 vergrössern | Entscheidung „abweisen"; verschiebt die Grenze nur |
| C59 nur Merker ohne Zufallszahl | eine gefälschte Antwort im Fenster ginge weiter durch |
| A15 Messbau mit Zyklenzähler | N-7 |
| A70 feste zweite Frist | schnitte den IP-Lauftext bei grossem `ticker_deceleration` ab (§4.1) |
| A80 Icon gar nicht anfordern | zweite Stelle, die dieselbe Regel kennen müsste |

## Versionsfolgen

- [ ] STM `src/main.h` anheben — S1 und S2, je einzeln
- [ ] ESP `version.h` anheben — E1, E2 und E3, je einzeln
- [ ] App `APP_VERSION` anheben — P1 und P2
- [ ] `CACHE_NAME` in `sw.js` anheben — P1 und P2, je mit `APP_VERSION`

Nur der `release-engineer` führt das aus (R4).
