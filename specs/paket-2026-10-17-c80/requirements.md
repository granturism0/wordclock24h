# Anforderungen — Paket 2026-10-17 „C80: bleibende Heap-Zerstückelung nach frühem Verkehr — Firmware-Ursache“

**Status:** Entwurf, zur Freigabe. Ohne Deine Freigabe beginnt keine Umsetzung.
**Erstellt:** 2026-10-17, `spec-writer`, Stand `353a693` (Zweig `hardware/esp32s3-umbau`). Ausgangsstand am Gerät: STM
3.2.34, ESP 3.2.41, PWA 1.4.102 (Release R2b, Tag `release/3.2.34-3.2.41-1.4.102`) — **kein** Sollwert; die Versionen
der Releases setzt der Lead über den `release-engineer` (R4).
**Auslöser:** Dein Entscheid vom 17.10.2026 zu C80: **„Firmware-Ursache suchen“** — keine Ablaufregel „nach dem
Einspielen neu starten“, kein Neustart als Abhilfe.
**Grundlage:** `BEFUNDE.md` C80 (ToDo-Zeile), **L482**, **L499**, L493 (N6), L468 (I-5, I-6, L7), C71; die
Ursachenanalyse des `firmware-analyst` vom 17.10.2026 (zusammengefasst in L499; der Bericht selbst liegt nicht im
Repository); das Werkzeug des Leads `vier-arme.py` (im Scratchpad des Leads, nicht im Repository).

**Zeilenangaben** beziehen sich auf `353a693`, Core-Angaben auf ESP8266-Core 3.1.2. **Massgeblich ist die genannte
Funktion bzw. das Symbol.**

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen:** **✔ SW** — vom `spec-writer` am Code `353a693` bzw. an der Core-Quelle 3.1.2 nachgelesen; **✔ B** — in
`BEFUNDE.md` belegt; **✔ L** — vom Lead am Gerät gemessen; **✔ FA** / **◐ FA** / **? FA** — vom `firmware-analyst`
belegt, als plausibel gemeldet bzw. als Vermutung; **◐** — abgeleitet; **●** — gemeldet, vor der Umsetzung zu prüfen.

**Verifikationsstatus von C80:** Das **Phänomen** ist ✔ L — am Gerät reproduziert, 3 von 3, ohne Build (L499). Der
**Mechanismus** ist offen; alle Kandidaten sind ◐ FA oder ? FA. Diese Spec baut deshalb **zuerst ein Instrument** und
legt **keine Abhilfe** fest.

**Kennungen dieses Pakets:** Stufen **V** (Vorprobe ohne Build), **1** (Instrument, Release **I**), **2** (Messung),
**3** (Abhilfe, Release **A**), **R** (Rückbau, Release **R**), **Z** (Abschluss). Akzeptanzkriterien **AKV**, **AKI**,
**AKM**, **AKA**, **AKZ**. Messungen **M1–M4** (Vorschläge des `firmware-analyst`, hier festgelegt), **M0** (rückwirkende
Auswertung des Leads, L499). Arme **A–E**. Lesarten **L-a**, **L-a′**, **L-b**; Fragen **F1–F5**; Zuordnungen **Z1–Z4**;
Entscheidungen **Ent-C1 bis Ent-C4**. Kandidaten **K1–K6**. **Trenner:** die Belegung, die nach einer frühen Anfrage den
grössten freien Block teilt. **ΔRAM:** Zuwachs „Globale Variablen“ eines Releases gegen 3.2.41, auf 8 Byte aufgerundet.
Die Nummern gelten nur in diesem Paket; AKI, AKM und AKZ sind nicht die gleichnamigen Kriterien aus dem Paket 2026-10-15.

---

## Überblick

| Stufe | Inhalt | Laufzeit | Einspielen | Nachweis |
|---|---|---|---|---|
| **V** | Vorprobe auf 3.2.41: Werkzeug nach `tools/`, Arm **E** (eine Anfrage erst nach der Startzeile), Arm A mit Mitschrift der ersten Antwort | — | — | AKV.1–AKV.4 |
| **1** | **Instrument:** Heap-Karte (M1), Startschritte (M2), Ereigniskarten (M3), Wegmarken (M4), befristet hinter `ESP_HEAP_MAP` | ESP | ESP-OTA (Release **I**) | AKI.1–AKI.12 |
| **2** | **Messung** auf Release I: Arme A, B, E; Fragen F1–F5; Zuordnung Z1–Z4 | — | — | AKM.1–AKM.5 |
| **3** | **Abhilfe** — erst nach Beleg und Deinem Entscheid (Ent-C3), über einen Nachtrag zu dieser Spec | nach Nachtrag | ESP-OTA (Release **A**), weitere Komponenten nur nach Nachtrag | AKA.1–AKA.7 |
| **R** | **Rückbau** des Instruments im nächsten ESP-Release nach der Abnahme von Stufe 3 | ESP | ESP-OTA (Release **R**) | AKZ.7 |
| **Z** | Befund nachführen, Endstand | — | — | AKZ.8, AKZ.9 |

---

## Problem

### 1. Eine einzige frühe Anfrage teilt den grössten freien Block dauerhaft (C80)

**✔ L (L499, Versuch mit vier Armen, 17.10.2026 09:30–09:57, ESP 3.2.41, ohne neuen Build):** Je Arm ein Auslöser, dann
in allen Armen ab +30 s nach der Startzeile `(FIRMWARE …)` dieselbe Folge — `smoke-device.sh`, `check-pwa.sh`, Ruhewert
≥ 150 s danach mit `tcp=0/0/0/0/0/0`. Einziger Unterschied: in den Armen „sofort“ ein `device_ready`-Abruf, sobald die
neue Firmware antwortet.

| Arm | Auslöser | erste Anfrage | Ruhewert `max` | `free` |
|---|---|---|---|---|
| A | `local_esp_restart` | sofort (0,2 s bzw. 0,3 s nach der Startzeile) | **6'600** (2 von 2) | 7'336 |
| B | `local_esp_restart` | ab +30 s | 7'104 (2 von 2) | 7'336 |
| C | OTA auf dasselbe Abbild | ab +30 s | 7'104 | 7'336 |
| D | OTA auf dasselbe Abbild | sofort (0,2 s) | **6'600** | 7'280 |

- Der Fehlbetrag ist **fest**: 504 Byte, dreimal exakt (A, D, A).
- `free` ist gleich wie im sauberen Arm — eine **Verschiebung**, keine zusätzliche Belegung.
- Bei der ersten Anfrage selbst stand `max` noch bei 7'104 (`- heap erste anfrage free=7568 max=7104`); der Fehlbetrag
  entsteht **mit** der frühen Anfrage, nicht davor.
- OTA oder Neustart ist gleichgültig (C sauber, D zerstückelt).

**✔ L (L499, M0, rückwirkend über 21 Starts vom 16. und 17.10.2026):** erste Anfrage 0,2–5,9 s nach der Startzeile ⇒ 5
von 5 zerstückelt (`max` 4'752, 6'456, 4'832, 6'408, 6'112); ab 16,9 s ⇒ 13 von 14 sauber. Die grösseren, streuenden
Fehlbeträge passen zu mehr Verkehr in den ersten Sekunden (◐, L499). Die frühere Bindung an „OTA“ (L482) kam daher, dass
die OTA-Werkzeuge sofort nach der Startzeile anfragten.

### 2. Folge

Nach einem gewöhnlichen Einspielen fragen Smoketest und `install-app.sh` sofort an. Der Ruhewert liegt danach bis zum
nächsten Neustart unter 7'104 — in zwei echten Einspielungen bei 4'832 und 4'024 (L482), also **unter K30** (5'120,
Ent-13 aus Paket 2026-10-15). Ein OTA braucht 4'096 Byte am Stück (C71); die Reserve ist dann dünn. Auch ein STM-Reset mit
offener PWA genügt (L493, N6: 6'112).

### 3. Stand der Ursachenanalyse und was ihr fehlt

Der `firmware-analyst` (nur Quelltext, L499) findet **keinen** Zweig für „erster Start eines neuen Abbilds“ (✔ FA). Er
nennt die Kandidaten K1–K5 und als Hauptkandidaten **K2** — eine frühe Verbindung während `setup()` legt PCB,
`ClientContext` und Empfangspuffer unter die dauerhaften UDP-Kontexte; nach der Freigabe bleibt dort ein Loch (◐ FA).

**Geprüft vom `spec-writer` (Einzelheiten `design.md` §2):** Die UDP-Kontexte stehen bei der Startzeile schon
(`ntp_setup()` und `udp_server_setup()` laufen in `ESP-uclock.ino:327-328`, die Startzeile in `:329-331`, ✔ SW). Die
„0,2 s nach `(FIRMWARE …)`“ messen aber den Zeitpunkt, an dem `loop()` die Verbindung **abholt** (`- heap erste anfrage`,
`http.cpp:17376-17386`), nicht den des Verbindungsaufbaus: Ab `http_server_begin()` (`wifi.cpp:93`) bekommt ein
eingehender Aufbau seinen `ClientContext` schon im lwIP-Rückruf und wartet in `_unclaimed` (`WiFiServer.cpp:187-202`,
Core 3.1.2, ✔ SW). Arm A fragt ab dem Wegfall der alten Firmware alle 0,3 s mit 1 s Frist an (`vier-arme.py`, ✔ SW).
**K2 ist damit weder widerlegt noch belegt.** Ebenso offen sind Belegungen **nach** der Startzeile — die restlichen
Schritte von `setup()` (`vars_init()`, `stm32_flash_init()`, `display_layout_init()`, `tables_init()`,
`ESP-uclock.ino:332-335`) und die ersten Sekunden im Betrieb.

**Was fehlt, ist ein Instrument, das zeigt, *was* wo liegt.** Heute meldet das Gerät `free`, `max`, `frag` und die Zeile
der ersten Anfrage — es sagt, *dass* der grösste Block kleiner ist, nicht, *welche* Belegung ihn teilt und *wann* sie
entstand.

---

## Ziel

1. **Den Trenner benennen** — welche Belegung nach einer frühen Anfrage den grössten freien Block teilt und in welchem
   Schritt sie entsteht —, mit Beleg am Gerät (Stufen V, 1, 2).
2. **Nach Deinem Entscheid eine Abhilfe**, mit der eine frühe Anfrage den Ruhewert nicht mehr senkt: Arm A so sauber wie
   Arm B, ohne Rückschritt anderswo (Stufe 3).
3. **Das Instrument wieder zurückbauen** (Stufe R).

---

## Entscheidungen

| Kennung | Frage | Stand | Wer | Wirkt auf |
|---|---|---|---|---|
| **Ent-C1** | Weg für Heap-Karte und Wegmarken (M1, M3, M4): **A** Blockgang über den inneren Aufbau des Haufens, an Core 3.1.2 gebunden; **B** nur öffentliche Mittel, ohne Karte; **C** öffentlich mit Sondierbelegung (`design.md` §4.9) | **offen** — Vorschlag A; vorgelegt nach dem Machbarkeitsbericht (Task 1.1). Rückfrage an Dich, weil R1.3 im Paket 2026-10-15 ausdrücklich „kein privater Kopf“ gewählt hat (L468) | Nutzer, nach Bericht 1.1 | AKI.2–AKI.6, AKI.8, Stufe 2 |
| **Ent-C2** | Grenze „Globale Variablen“ für Release I | **offen** — Vorschlag **≤ 320 Byte** (gerechnet rund 250, `design.md` §4.6); darüber anhalten und vorlegen | Lead, bei Freigabe | AKI.9 |
| **Ent-C3** | Abhilfe | **offen bis nach Stufe 2** — der Lead legt Dir die Optionen aus dem Nachtrag per Rückfrage vor | Nutzer | Stufe 3 |
| **Ent-C4** | Zeitpunkt des Rückbaus | **gesetzt:** im nächsten ESP-Release **nach** der Abnahme von Stufe 3 — Release A trägt das Instrument noch, damit AKA.5 die Karte lesen kann. Du kannst das bei der Freigabe ändern | spec-writer, Freigabe durch Dich | AKZ.7 |

---

## Akzeptanzkriterien

### Stufe V — Vorprobe ohne Build (auf ESP 3.2.41)

- [ ] **AKV.1 (Werkzeug der Arme)** — Das Werkzeug `vier-arme.py` liegt unter `tools/` (Name beim Lead) und kann die Arme
      A, B, C, D wie in L499 und dazu **Arm E** (`design.md` §3.2). Je Lauf eine Ergebniszeile mit: Arm; Zeitpunkt der
      Startzeile `(FIRMWARE …)`; Abstand der Zeile `- heap erste anfrage` zur Startzeile; in den Armen A und D die Zahl
      der Abfrageversuche vor der ersten Antwort; die Ruhewert-Zeile (`free`, `max`, `tcp=`) ≥ 150 s nach dem Ende der
      Folge mit `tcp=0/0/0/0/0/0`. In den Armen A und D zusätzlich aus dem **Rumpf der ersten `device_ready`-Antwort** die
      Felder `http_accept_dead`, `tcp_now_pending`, `tcp_now_unclaimed` und `tcp_now_list` (`http.cpp:15519-15544`).
      **Exit ≠ 0**, wenn in einem Lauf die Startzeile, die erste Anfrage oder der Ruhewert fehlt, mit Meldung, welcher
      Lauf und was fehlt. Modus `--abnahme <soll>`: Exit ≠ 0, wenn ein Lauf eines genannten Arms unter dem Soll liegt.
      **Gegenproben (DIR-014):** (a) Mitschnitt ohne Startzeile (Attrappe des Logger-Zugriffs) ⇒ Exit ≠ 0 mit Meldung;
      (b) `--abnahme 7104` für Arm A auf 3.2.41 ⇒ FEHL, Exit ≠ 0 — eingelöst im Lauf AKV.3; (c) jede Meldung in Konsole
      **und** Mitschrift.
- [ ] **AKV.2 (Arm E)** — Drei Läufe Arm E: Bis die Startzeile im Mitschnitt steht, öffnet das Werkzeug **keine**
      Verbindung zur Uhr (vom Werkzeug geprüft und gemeldet); danach genau **eine** `device_ready`-Anfrage. Abstand zur
      Startzeile und Ruhewert berichtet. **Ein Lauf mit Abstand über 5,9 s ist ungültig** (oberer Rand des schlechten
      Fensters aus M0) und wird wiederholt. Kein Soll für den Ruhewert — das Ergebnis trennt die Lesarten
      (`design.md` §2.3).
- [ ] **AKV.3 (Arm A mit Rumpf, Arm B als Kontrolle)** — Je drei Läufe A und B, im Wechsel mit E in der Reihenfolge aus
      `design.md` §3.3. **Arm A reproduziert L499** — `max` je Lauf mindestens 256 Byte unter dem kleinsten `max` der
      B-Läufe derselben Reihe. Gelingt das nicht, ist die Reproduktion verloren; das steht zuerst im Messprotokoll, und
      Stufe 1 wartet auf einen Nachtrag. Je A-Lauf die Felder der ersten Antwort nach AKV.1.
- [ ] **AKV.4 (Bedingungen der Vorprobe)** — Logwache läuft mit, `--pruefe` am Ende (DIR-013); Beginn und Ende
      angekündigt (R5); kein Browser an der Uhr, kein Upload, kein zweites Werkzeug (N6); ein Ruhewert gilt nur mit
      `tcp=0/0/0/0/0/0` und ≥ 150 s Abstand; jede Abweichung gekennzeichnet; das Ergebnis steht im Messprotokoll, bevor
      der nächste Task beginnt (L424).

### Stufe 1 — Instrument (Release I, nur ESP)

- [ ] **AKI.1 (Grundsätze jeder neuen Messzeile)** — (a) **belegt keinen Heap**: Aufbau mit `snprintf` in einen
      Stackpuffer, kein `String`, kein `malloc`/`new`, keine Container; (b) **einmal formatiert, zweimal ausgegeben** —
      `Serial.println()` und `stm32_log_append()` mit demselben Puffer (Muster `esp_heap_log()`,
      `ESP-uclock.ino:639-641`); (c) längste Form **≤ 119 Zeichen** (`STM32_LOG_LINE_LEN` 120, `ESP-uclock.ino:652-653`);
      (d) Ausgabe ausschliesslich aus `loop()`, **keine neue Ausgabe in `setup()`**; (e) höchstens **eine** Messzeile
      dieses Pakets je Sekunde und keine im selben `loop()`-Durchlauf wie eine Heap-Zeile; (f) Präfix `- heap ` gefolgt
      von einem Wort, das keine bestehende Zeile trägt (`karte`, `start`, `marken`); (g) **keine IP-Adresse, kein Port,
      kein Name eines Netzes** in einer Zeile.
- [ ] **AKI.2 (M1 Heap-Karte)** — Zeile `- heap karte <anlass> ent=<belegt>/<frei> fsum=<n> gross=<blk>+<len>
      zweit=<blk>+<len> davor=<blk>+<len> danach=<blk>+<len>` nach `design.md` §4.2. Anlass `m` zu **jeder** Heap-Zeile,
      erfasst im **selben** `loop()`-Durchlauf wie die Heap-Zeile, ausgegeben frühestens 1 s danach. Der Blockgang ist
      eine **reine Funktion** über Blockfeld und Blockzahl (Prüfstand ohne Gerät, AKI.8), läuft unter einer Sperre, die
      gleichzeitige Belegungen ausschliesst, und hat die Wächter aus §4.2: Blockgrösse ≠ 8, Index ≥ Blockzahl, nicht
      steigender Folgeindex, Iterationsgrenze ⇒ statt Werten `abbruch=<grund>`; die Sperre wird auf **jedem** Weg
      gelöst. Mit `ESP_HEAP_LOG` 0 entfällt M1.
- [ ] **AKI.3 (M1 gegen die Heap-Zeile)** — Die Länge `gross` steht zu `max=` derselben Lesung in einer festen, im Bericht
      des `esp-developer` genannten Beziehung (gleich oder fester Unterschied), ebenso `fsum` zu `free=`; der Prüfstand
      prüft beide. **Am Gerät** gilt die Beziehung in zehn aufeinanderfolgenden Paaren aus Heap-Zeile und Kartenzeile in
      Ruhe; eine Abweichung ist ein Fehler des Instruments, kein Befund am Haufen.
- [ ] **AKI.4 (M2 Startschritte)** — An den neun Schritten s0–s8 aus `design.md` §4.3 werden erfasst: `ms` seit dem
      Start, `free`, Beginn und Länge des grössten freien Blocks, Zahl der belegten Einträge, Zahl der Einträge in
      `_unclaimed` des Servers (vor dem Serverstart 0) und Zahl der TCP-PCBs ausserhalb LISTEN und TIME_WAIT. Ausgabe
      nach dem Ende von `setup()` als **eine Zeile je Schritt** `- heap start s<n> …` nach AKI.1 (e); die Zeile für s8
      trägt zusätzlich `us=<Dauer des Blockgangs bei s8>`.
- [ ] **AKI.5 (M3 Ereigniskarten)** — Je eine Zeile im Format von AKI.2 mit Anlass **`S`** (Ende von `setup()`), **`F`**
      (nach der Rückkehr aus dem `http_server_loop()`-Durchlauf, in dem die erste Anfrage angenommen wurde) und **`U`**
      (nach der Zeile `- update cache nach`), je Start genau einmal, erfasst im Augenblick des Ereignisses, ausgegeben
      verzögert nach AKI.1 (e). Tritt ein Ereignis nicht ein, fehlt seine Zeile — das ist kein Fehler und wird vom
      Werkzeug (AKM.2) als „Ereignis nicht eingetreten“ gemeldet.
- [ ] **AKI.6 (M4 Wegmarken)** — Bei `S` und bei `F` je eine Zeile `- heap marken <anlass> …` mit dem Blockindex — in
      derselben Einheit wie die Karte — jedes Objekts aus der Liste nach Ent-C1 (`design.md` §4.5). Ein Objekt der Liste,
      das sich nicht erreichen lässt, steht mit Grund im Bericht 1.1, nicht geschätzt in der Zeile. Für TCP-PCBs nur
      Index und Zustand, keine Gegenstelle (AKI.1 (g)).
- [ ] **AKI.7 (Schalter und Bindung)** — Schalter **`ESP_HEAP_MAP`** (in Release I 1). Mit 0 entsteht keine der neuen
      Zeilen, und „Globale Variablen“ ist gleich dem Stand ohne Instrument. Ein Übersetzungsfehler, wenn `ESP_HEAP_MAP` 1
      und `ESP_HEAP_LOG` 0 ist. Bei Ent-C1 = A bricht die Übersetzung mit einer Meldung ab, wenn der Core nicht 3.1.2 ist
      (Mittel bestimmt der `esp-developer`). Ein Kommentarblock nennt Zweck, Kosten und die Rückbaupflicht nach dem Muster
      von `ESP_HEAP_LOG` (`ESP-uclock.ino:579-582`).
- [ ] **AKI.8 (Prüfstand c80i)** — Prüfstand ohne Gerät mit den Fällen und Sabotagen aus `design.md` §4.8, darunter
      synthetische Haufen mit bekannter Lage (exakte Felder), beschädigte Ketten (je ein `abbruch=`-Grund), längste Form
      ≤ 119, „keine Belegung“ statisch, beide Ausgabewege, `ESP_HEAP_MAP` 0, keine Ausgabe in `setup()`, höchstens eine
      Zeile je Durchlauf. **Jede Sabotage ⇒ FEHL**; gegen 3.2.41 ⇒ FEHL; in S16, Fallzahl nachgezählt (L235).
- [ ] **AKI.9 (Kosten)** — Δ „Globale Variablen“ gegen 3.2.41 ≤ Ent-C2, darüber anhalten und vorlegen; Programmgrösse
      vorher und nachher im Messprotokoll; Dauer eines Blockgangs am Gerät (`us=` aus AKI.4) in drei Starts berichtet,
      **über 1'000 µs ⇒ vorlegen** (◐ Vorschlag); Byte je Minute auf der Brücke im Betrieb und je Start berichtet.
- [ ] **AKI.10 (Gegenprobe am Gerät, DIR-014)** — Im ersten Start nach Release I und in jedem Lauf von Stufe 2: jede neue
      Zeilenart im Mitschnitt **und** über `/api/stm32_log`, Text gleich. Je Start nachgezählt gegen das Soll — 9 ×
      `start`, je 1 × `S`, `F`, `U` (soweit eingetreten), 2 × `marken`, je Heap-Zeile 1 × `karte m` (L235). Kein
      Anstieg von `ovr=` und kein Sprung in `d=` durch die Startzeilen gegen einen Start auf 3.2.41. Kein `abbruch=` im
      Betrieb.
- [ ] **AKI.11 (bestehende Auswertungen)** — Die Heap-Zeile bleibt Byte für Byte im bisherigen Format. `watch-log.sh`,
      `smoke-device.sh`, `mess-stabilitaet.sh` und das Werkzeug der Arme lesen ihre Werte unverändert. Die Logwache meldet
      `abbruch=` als Warnung und eine gewöhnliche Kartenzeile nicht. Gegenproben über eine eingespeiste Quelle: Zeile mit
      `abbruch=` ⇒ Meldung; gewöhnliche Kartenzeile ⇒ keine Meldung.
- [ ] **AKI.12 (Einspielen Release I)** — ESP-OTA; danach `install-app.sh --check` vollständig (DIR-017); Smoketest ohne
      FEHLT samt Update-Quelle (DIR-009); `check-pwa.sh` ohne FEHLT; Versionen gleich denen des Tags; Logwache meldet den
      OTA-Neustart. *Der Ruhewert nach diesem Einspielen ist keine Startmessung* — die Werkzeuge fragen sofort an (L499).

### Stufe 2 — Messung (auf Release I)

- [ ] **AKM.1 (Reproduktion unter dem Instrument)** — Arm A 3 von 3 zerstückelt: `max`(A) mindestens 256 Byte unter dem
      kleinsten `max` der B-Läufe derselben Reihe; Arm B 3 von 3 mit `max` ≥ 7'104 − ΔRAM(I); `free`(A) und `free`(B)
      höchstens 64 Byte auseinander. **Misslingt eines davon, gilt Z4** — Stufe 2 hält an, der `spec-writer` schreibt
      einen Nachtrag zum Instrument.
- [ ] **AKM.2 (Läufe)** — Arme A, B und E je dreimal in der Reihenfolge aus `design.md` §5.1, je Lauf vollständig: neun
      Startzeilen, die Ereigniskarten `S`, `F`, `U`, die Wegmarken `S`, `F` und die Kartenzeilen bis zum Ruhewert. Das
      Werkzeug sammelt sie je Lauf in die Mitschrift und endet mit Exit ≠ 0, wenn eine erwartete Zeile fehlt (ausser
      „Ereignis nicht eingetreten“ nach AKI.5).
- [ ] **AKM.3 (Fragen F1–F5)** — Im Messprotokoll mit Zahlen je Lauf beantwortet (`design.md` §5.3): **F1** Lage von
      `gross` und `zweit` in Ruhe, A gegen B; **F2** der Trenner — `davor`/`danach` am grossen Block in Ruhe in Arm A —
      und ob er einer Wegmarke entspricht; **F3** seit welchem Schritt bzw. Ereignis er dort liegt; **F4** belegte
      Einträge in Ruhe, A gegen B (Verschiebung oder zusätzliche Belegung); **F5** Arm E wie A oder wie B, und in Arm A
      `_unclaimed` und PCBs je Schritt s2–s8.
- [ ] **AKM.4 (Zuordnung)** — Der `firmware-analyst` ordnet aus dem Messprotokoll nach den Regeln Z1–Z4 zu
      (`design.md` §5.4), mit Kennzeichen ✔/◐ je Aussage. **„Nicht zugeordnet“ (Z3) ist ein zulässiges Ergebnis**; es
      führt zu einem Nachtrag am Instrument (weitere Wegmarken), nicht zu einer Abhilfe.
- [ ] **AKM.5 (Gesundheit)** — `watch-log.sh --pruefe` am Ende im Bericht; Neustarts nur die gewollten (einer je Lauf),
      alle gemeldet; keine Exception, kein Watchdog-Reset, kein Sprung in `d=`, kein `abbruch=`.

### Stufe 3 — Abhilfe (Release A, nach Ent-C3)

- [ ] **AKA.1 (Entscheid vor dem Bau)** — Eine Abhilfe entsteht erst nach einer Zuordnung **Z1 oder Z2** und nach einem
      **Nachtrag** zu dieser Spec mit Optionen. Der Lead legt Dir die Optionen per Rückfrage vor; Ent-C3 und Deine
      Freigabe des Nachtrags stehen fest, **bevor** der `esp-developer` beginnt. Der Nachtrag nennt, welche Komponenten
      die Abhilfe berührt.
- [ ] **AKA.2 (Prüfstand der Abhilfe)** — Prüfstand nach dem Nachtrag mit mindestens einer Sabotage, die den alten Zustand
      wiederherstellt ⇒ FEHL; gegen Release I ⇒ FEHL; in S16, Fallzahl nachgezählt.
- [ ] **AKA.3 (Arm A und jeder schlechte Arm)** — Drei Läufe Arm A **und** drei Läufe Arm D: in jedem Lauf `max` ≥ 7'104
      − ΔRAM(A) **und** ≥ dem kleinsten `max` der drei B-Läufe derselben Reihe. Dasselbe für jeden weiteren Arm, der in
      Stufe 2 zerstückelt war (etwa E). Geprüft mit `--abnahme` des Werkzeugs (AKV.1) — dessen Gegenprobe ist Stufe 2
      selbst, wo Arm A dasselbe Soll reisst.
- [ ] **AKA.4 (Arm B unverändert)** — Drei Läufe Arm B: `max` je ≥ 7'104 − ΔRAM(A); `free` höchstens 64 Byte plus
      ΔRAM(A) − ΔRAM(I) unter dem Wert aus Stufe 2 (◐ Vorschlag; darüber vorlegen).
- [ ] **AKA.5 (Karte)** — In Arm A liegt der Trenner aus AKM.3 in Ruhe nicht mehr am grossen Block (`davor`/`danach`),
      oder der Nachtrag begründet, warum die Karte ein anderes Bild zeigt.
- [ ] **AKA.6 (keine Regression)** — Smoketest ohne FEHLT samt Update-Quelle; `install-app.sh --check` vollständig;
      `check-pwa.sh` ohne FEHLT; Ladeprobe warm wie G10b.1 (Paket 2026-10-15: zwei Leitungsanfragen, alles Versionierte
      aus dem Cache); `low=` ≥ 1'072 in den Heap-Zeilen während `check-pwa.sh`; Legacy `/` mit HTTP 200; Logwache ohne
      Exception und ohne Watchdog-Reset.
- [ ] **AKA.7 (Durchlauf vor dem Release, DIR-012)** — Berührt die Abhilfe mehr als eine Komponente, läuft vor dem
      Release der `pwa-tester` (Phasen 0–4 und 9). Bleibt es beim ESP, steht die Auslassung mit Grund im Bericht.

### Übergreifend

- [ ] **AKZ.1 (Kodierung und Zeilenende, DIR-015)** — Vor jedem Patch je Datei im Taskbericht festgestellt: Kodierung
      (ASCII, ISO-8859-1 oder UTF-8, mit Zahl der Nicht-ASCII-Byte) und Zeilenende (Zahl der CRLF- und LF-Zeilen). Bekannt
      und dennoch neu festzustellen: `ESP-uclock.ino` ASCII mit **gemischten** Zeilenenden, `http.cpp` UTF-8 mit einer
      CRLF-Zeile (L468, 0.1); jede weitere Datei und jede neue Datei ebenso. Der Diff enthält nur inhaltlich geänderte
      Zeilen; der Lead prüft mit `git diff --stat`.
- [ ] **AKZ.2 (Release-Inhalt)** — Releases I, A und R ändern nur `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` (samt
      `version.h`); **keine** Änderung an `src/**`, `ESP8266/ESP-uclock/data/**`, `Makefile`, `CMakeLists.txt`, `cmake/**`
      — es sei denn, ein freigegebener Nachtrag nennt sie für Release A. Geprüft gegen das vorige Tag, Liste im
      Messprotokoll; `tools/**`, Doku und `specs/` sind als Nicht-Fabrikat ausgenommen und als solche genannt.
- [ ] **AKZ.3 (Versionen, DIR-004, R4)** — Je Release steigt nur `ESP_VERSION`, einmal, durch den `release-engineer` im
      Auftrag des Leads; `VERSION` (STM), `APP_VERSION` und `CACHE_NAME` bleiben, das Format der Zeilen bleibt.
- [ ] **AKZ.4 (Releases, DIR-002, DIR-005, DIR-011)** — Je Release ein `make release-zip`, ein Commit, ein Tag
      `release/<stm>-<esp>-<app>` mit den Versionen dieses Commits, Push samt Tag im selben Zug; Einspielzeilen mit
      Reihenfolge und Rückweg samt Befehl, **bevor** eingespielt wird; ausdrücklich genannt, was zu flashen ist (nur ESP,
      kein STM, keine neue PWA) und dass `install-app.sh --check` folgt; `deploy.sh` ohne `--delete`.
- [ ] **AKZ.5 (Guardrails und Prüfstände)** — Nach jedem schreibenden Task `./tools/guardrails.sh` mit Exit 0; vor jedem
      Release `--full` mit Exit 0; die Fallzahl jedes neuen Prüfstands nachgezählt (L235).
- [ ] **AKZ.6 (Gerätelauf, DIR-013, R5, DIR-020)** — In jedem Gerätelauf läuft die Logwache mit, `--pruefe` am Ende im
      Bericht; Beginn und Ende angekündigt; genau ein Verursacher (kein Browser, kein zweites Werkzeug); ein auf später
      geplanter Lauf mit `caffeinate`.
- [ ] **AKZ.7 (Rückbau, Ent-C4)** — Ab Release I führt `BEFUNDE.md` den Rückbau als offenen Punkt in der ToDo-Liste. Im
      nächsten ESP-Release nach der Abnahme von Stufe 3 (AKA.3–AKA.7) gehen M2–M4 heraus, und M1 geht heraus oder
      `ESP_HEAP_MAP` steht auf 0; c80i ist auf den Rückbau umgestellt (keine neue Zeile, Gegenprobe mit 1 ⇒ FEHL);
      „Globale Variablen“ liegt danach beim Stand ohne Instrument zuzüglich dem Anteil der Abhilfe.
- [ ] **AKZ.8 (Messprotokoll)** — Eine eigene L-Zeile in `BEFUNDE.md` als Messprotokoll dieses Pakets (Lead); jeder
      messende Task trägt sein Ergebnis ein, bevor der nächste beginnt (L424); Nachführung von C80 durch den `doc-writer`,
      nie gleichzeitig mit einer Messzeile (R3).
- [ ] **AKZ.9 (Endstand)** — Zum Schluss ein `endstand.md` dieses Pakets mit dem Endstand jedes Kriteriums (belegt,
      entfallen, offen) und der Zählung.

**Zählung:** AKV 4, AKI 12, AKM 5, AKA 7, AKZ 9 — **37 Kriterien**.

---

## Nicht Teil dieser Änderung

- **Keine Ablaufregel „nach dem Einspielen neu starten“** und kein Neustart in `install-app.sh`, `deploy.sh`,
  `flash-stm.sh`, `smoke-device.sh` oder einem anderen Werkzeug (Dein Entscheid vom 17.10.2026). Das Werkzeug der Arme
  startet die Uhr nur als Auslöser einer Messung neu.
- **Kein Umbau des Webservers vor dem Beleg** — `http_server_loop()`, `WcHttpServer`, Backlog nach freiem Heap,
  TIME_WAIT-Begrenzung, `http_server_begin()` und sein Aufrufort bleiben in Stufe 1 und 2 unverändert.
- **Keine Abhilfe in Release I.** Auch keine Umstellung der Reihenfolge in `setup()` — sie wäre eine Abhilfe, keine
  Messung.
- Keine Änderung am Core (Arduino-Paketverzeichnis), an Build-Optionen, `Makefile` oder `CMakeLists.txt`.
- **Die Heap-Zeile bleibt unverändert.** Der Vorschlag aus der ToDo-Zeile C80, sie „um die Zahl der Belegungen zu
  ergänzen“, geht in M1 auf — als eigene Zeile, weil Werkzeuge die Heap-Zeile lesen (`ESP-uclock.ino:642`).
- C65, C49 und C54 nicht. **C77** (Wetter-Client, K1) nur, wenn die Zuordnung ihn als Trenner benennt **und** Du das in
  Ent-C3 entscheidest.
- K30, `TESTPLAN-PWA.md` und die Regeln für T3 bleiben.
- PWA, STM-Firmware und Legacy-Oberfläche bleiben unverändert (ausser ein freigegebener Nachtrag nennt sie).
- Die Auswertung M0 wird nicht wiederholt.
- Die veraltete Zeilenzahl zu `ESP-uclock.ino` in `CLAUDE.md` (1'043 von 1'240 CRLF; L468, 0.1: 2'116 von 2'443) wird
  hier nicht berichtigt — gemeldet an den Lead.

## Betroffene Laufzeiten

- [ ] STM32 (`src/**`) — nicht betroffen
- [x] ESP8266 (`ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino`) — Neu-Flashen nötig (Releases I, A, R)
- [ ] PWA (`data/app/**`) — nicht betroffen; nach jedem OTA nur `install-app.sh --check`
- [x] Build/Release — je Release ZIP, Tag, Rollout; keine Änderung an Build-Dateien
- [x] Werkzeuge (`tools/**`, Lead) — Werkzeug der Arme, Logwache, Prüfstand c80i
