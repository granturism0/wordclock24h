# Anforderungen — Paket 2026-10-17 „C80: bleibende Heap-Zerstückelung nach frühem Verkehr — Firmware-Ursache“

**Status: freigegeben am 17.10.2026 (Task 0.1); Ent-C1 = A, Ent-C2 ≤ 320 Byte, Ent-C4 bestätigt. Nachtrag N1
eingearbeitet.** **Nachtrag N1** (17.10.2026, Stand `6b3cbe9`): Deine Freigabe und die Entscheide Ent-C1 (Deiner, nach
dem Machbarkeitsbericht 1.1), Ent-C2 (Lead) und Ent-C4 (Deiner); das Ergebnis der Vorprobe V.2 (`BEFUNDE.md` **L500**) mit
dem Stand von AKV.1–AKV.4; die Ergebnisse des Machbarkeitsberichts 1.1 des `esp-developer` als Präzisierungen in
AKI.2–AKI.9, AKZ.1 und `design.md` §4. Jede Änderung trägt die Marke **N1**; der Text davor bleibt stehen. **Offen
gelassen:** die Lesart der Vorprobe durch den `firmware-analyst` (Task V.3) — überall gekennzeichnet als **[offen: V.3]**.

**Nachtrag N2** (17.10.2026, Stand `a803e98`, Task V.4) — **Entwurf, zur Freigabe durch Dich. Ohne Deine Freigabe von N2
und Deinen Entscheid Ent-C5 beginnt Task 1.3 nicht.** Grundlage: die Lesart V.3 des `firmware-analyst` und die Zusatzprobe
des Leads (`BEFUNDE.md` **L501**), dazu der Nachtrag Protokoll zu **L500**. N2 füllt alle Stellen mit **[offen: V.3]** und
**ändert den Umfang von Stufe 1:** Die Startschritte s0–s8 entfallen; an ihre Stelle treten **Verursacher-Töpfe** im
Startfenster nach `setup()` (M2 neu). Die Ereigniskarten heissen **S, F, X, T** (U entfällt), die Wegmarken werden einmal
je Start bei **X** erfasst. Neu sind **AKI.13** und **AKI.14**; neu gefasst AKI.1, AKI.2, AKI.4–AKI.6, AKI.8–AKI.11 und
AKM.1–AKM.4. Eine neue Entscheidung: **Ent-C5** (Beginn der Folge in Stufe 2). Jede Änderung trägt die Marke **N2**;
Überholtes bleibt stehen und ist als „überholt (N2)“ gekennzeichnet. *Zum Stand N1 oben, überholt (N2):* Die Lesart V.3
liegt vor; „offen gelassen“ gilt nicht mehr.

*Stand bei Erstellung, überholt durch N1:* Entwurf, zur Freigabe. Ohne Deine Freigabe beginnt keine Umsetzung.

**Erstellt:** 2026-10-17, `spec-writer`, Stand `353a693` (Zweig `hardware/esp32s3-umbau`). Ausgangsstand am Gerät: STM
3.2.34, ESP 3.2.41, PWA 1.4.102 (Release R2b, Tag `release/3.2.34-3.2.41-1.4.102`) — **kein** Sollwert; die Versionen
der Releases setzt der Lead über den `release-engineer` (R4).
**Auslöser:** Dein Entscheid vom 17.10.2026 zu C80: **„Firmware-Ursache suchen“** — keine Ablaufregel „nach dem
Einspielen neu starten“, kein Neustart als Abhilfe.
**Grundlage:** `BEFUNDE.md` C80 (ToDo-Zeile), **L482**, **L499**, L493 (N6), L468 (I-5, I-6, L7), C71; die
Ursachenanalyse des `firmware-analyst` vom 17.10.2026 (zusammengefasst in L499; der Bericht selbst liegt nicht im
Repository); das Werkzeug des Leads `vier-arme.py` (im Scratchpad des Leads, nicht im Repository). **N1:** dazu **L500**
(Vorprobe V.2), das Werkzeug `tools/startprobe.py` (Commit `f012309`, Nachfolger von `vier-arme.py`) und der
Machbarkeitsbericht 1.1 des `esp-developer` vom 17.10.2026 (liegt nicht im Repository; seine Ergebnisse stehen in
`design.md` §4, Marke N1). **N2:** dazu **L501** (Lesart V.3 und Zusatzprobe, Stand `3b9528a` des Werkzeugs) und der
Nachtrag Protokoll in L500. Der Bericht V.3 selbst liegt nicht im Repository; seine Ergebnisse stehen in L501 und in
`design.md` §3.5 N2.

**Zeilenangaben** beziehen sich auf `353a693`, Core-Angaben auf ESP8266-Core 3.1.2. **Massgeblich ist die genannte
Funktion bzw. das Symbol.** **N2:** Die in N2 neu genannten Zeilen in `ESP-uclock.ino` und `http.cpp` sind am Stand
`a803e98` nachgelesen; sie stimmen dort mit `353a693` überein (seither keine Änderung an ESP-Quellen).

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Kennzeichen:** **✔ SW** — vom `spec-writer` am Code `353a693` bzw. an der Core-Quelle 3.1.2 nachgelesen; **✔ B** — in
`BEFUNDE.md` belegt; **✔ L** — vom Lead am Gerät gemessen; **✔ FA** / **◐ FA** / **? FA** — vom `firmware-analyst`
belegt, als plausibel gemeldet bzw. als Vermutung; **◐** — abgeleitet; **●** — gemeldet, vor der Umsetzung zu prüfen.
**N1:** **✔ ED** / **◐ ED** — vom `esp-developer` im Machbarkeitsbericht 1.1 mit Fundstelle belegt bzw. geschätzt.

**Verifikationsstatus von C80:** Das **Phänomen** ist ✔ L — am Gerät reproduziert, 3 von 3, ohne Build (L499). Der
**Mechanismus** ist offen; alle Kandidaten sind ◐ FA oder ? FA. Diese Spec baut deshalb **zuerst ein Instrument** und
legt **keine Abhilfe** fest. **N1:** Ein Kandidat ist inzwischen am Gerät widerlegt — K2 in strenger Form (L500, Problem
§3 unten). Der Mechanismus bleibt offen. **N2:** Der **Zeitpunkt** ist eingegrenzt (✔ L, L501: Hintergrundbelegung rund
+3 bis +8 s nach der Startzeile); ein **Modell** des Mechanismus liegt vor (◐ FA, „Verdrängung“, Problem §3 N2); der
**Verursacher** ist offen (? FA: SDK/lwIP oder eine Zeile vom STM). Weiterhin keine Abhilfe vor dem Beleg.

**Kennungen dieses Pakets:** Stufen **V** (Vorprobe ohne Build), **1** (Instrument, Release **I**), **2** (Messung),
**3** (Abhilfe, Release **A**), **R** (Rückbau, Release **R**), **Z** (Abschluss). Akzeptanzkriterien **AKV**, **AKI**,
**AKM**, **AKA**, **AKZ**. Messungen **M1–M4** (Vorschläge des `firmware-analyst`, hier festgelegt), **M0** (rückwirkende
Auswertung des Leads, L499). Arme **A–E**. Lesarten **L-a**, **L-a′**, **L-b**; Fragen **F1–F5**; Zuordnungen **Z1–Z4**;
Entscheidungen **Ent-C1 bis Ent-C4**. Kandidaten **K1–K6**. **Trenner:** die Belegung, die nach einer frühen Anfrage den
grössten freien Block teilt. **ΔRAM:** Zuwachs „Globale Variablen“ eines Releases gegen 3.2.41, auf 8 Byte aufgerundet.
Die Nummern gelten nur in diesem Paket; AKI, AKM und AKZ sind nicht die gleichnamigen Kriterien aus dem Paket 2026-10-15.
**N1:** Nachträge vor Stufe 3 heissen **N1, N2, …**; die Kennung **A.1** bleibt dem Nachtrag zur Abhilfe vorbehalten
(Task 2.4).
**N2:** **t0** — Beginn des ersten `loop()`-Durchlaufs nach dem Start; **Startfenster W** — die ersten 60 s ab t0
(Ent-C5); **M2** heisst ab N2 **Verursacher-Töpfe** (die Startschritte entfallen); **Töpfe** `wif`, `srv`, `ota`, `upd`,
`udp`, `ntp`, `var`, `ovr`, `hlg`, `stm`, `sys` (`design.md` §4.3 N2); Karten **S, F, X, T** (`design.md` §4.4 N2);
**Summenprobe** (AKI.4 N2); Arm **RST** — im Werkzeug `R`; Verbindung ohne Daten, per RST abgebrochen; der Name RST
verhindert die Verwechslung mit Stufe und Release **R**; Arm **E<n>** — Arm E mit Anfrage n s nach der Startzeile; Lesarten
**L-b′** (Verdrängung) und **L-c** (Lazy-Init); Widerlegungsregeln **Wid-1**, **Wid-2** (`design.md` §5.4 N2); Frage
**F6**; Kandidat **K7**; Entscheidung **Ent-C5**.

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

**N1:** Stufe V ist am Gerät gelaufen (L500); ihr Stand steht bei AKV.1–AKV.4. Ob ihr Ergebnis den Umfang von Stufe 1
ändert, entscheidet sich nach V.3 (`design.md` §3.4 N1, §3.5) **[offen: V.3]** (→ N2).

**N2:** Stufe V ist abgeschlossen (AKV.1–AKV.4 erfüllt; dazu die Zusatzprobe, L501). V.3 ändert den Umfang von Stufe 1.
Die Zeilen der Tabelle oben gelten ab N2 so:

| Stufe | Inhalt ab N2 | Nachweis ab N2 |
|---|---|---|
| **1** | **Instrument:** Heap-Karte (M1), **Verursacher-Töpfe im Startfenster W (M2)**, Ereigniskarten **S, F, X, T** (M3), Wegmarken bei **X** (M4), befristet hinter `ESP_HEAP_MAP`; `setup()` bleibt unverändert | AKI.1–AKI.14 |
| **2** | **Messung** auf Release I: Arme A, B, E je dreimal, dazu zwei Läufe RST; Folge nach Ent-C5; Fragen in der Rangfolge **F3, F2, F4, F1** (F5 erledigt, F6 informativ); Zuordnung nach Z2–Z4 mit **Wid-1**, **Wid-2** | AKM.1–AKM.5 |

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

**Nachtrag N1 (17.10.2026) — Stand nach der Vorprobe V.2 (✔ L, L500; Lauf 16:35–17:14, ESP 3.2.41,
`tools/startprobe.py ABEEABBEA --abnahme 7104`):** Arm E — bis zur Startzeile keine Verbindung zur Uhr, vom Werkzeug im
Mitschnitt geprüft (0 Anfragen zwischen `rst cause` und Startzeile), danach genau eine `device_ready`-Anfrage 1,9 / 3,2 /
1,9 s nach der Startzeile — ergibt Ruhewerte `max` **6'600 · 6'600 · 6'432**, also zerstückelt wie Arm A (6'600 · 6'600 ·
6'600). Arm B bleibt sauber (7'104 · 7'104 · 7'104). In der ersten Antwort von Arm A waren jedes Mal `http_accept_dead`,
`tcp_now_pending` und `tcp_now_unclaimed` 0, in `tcp_now_list` nur die eigene Verbindung — beim ersten Abholen wartete
keine weitere Verbindung.

- **K2 ist in der strengen Form des Analysten widerlegt.** Der Schaden braucht keine Verbindung, die während `setup()`
  aufgebaut wird. Der Satz oben „K2 ist damit weder widerlegt noch belegt“ ist **überholt**.
- Er entsteht durch eine Anfrage in den ersten Sekunden **nach** dem Ende von `setup()` (Folgerung des Leads, L500;
  Begründung zum Ende von `setup()` in `design.md` §2.2 N1, ◐).
- **Neu beobachtet (✔ L, L500):** Auch ohne Anfrage fällt der Haufen in Arm B bis +30 s — `- heap erste anfrage` bei
  +30 s `free=6896 max=6568` gegen `7360/6896` bei +2 bis +4 s —, der Ruhewert bleibt dort aber sauber. Im Fenster nach
  `setup()` wird also im Hintergrund belegt.
- **Was in diesem Fenster belegt wird und welche Lesart das stützt, liefert V.3** **[offen: V.3]** (→ N2). Diese Spec
  greift dem nicht vor.

**Nachtrag N2 (17.10.2026) — Lesart V.3 und Zusatzprobe (L501):**

- **Ende von `setup()`** (✔ FA, an den Startfolgen der Läufe A5, E4, B6 aus L500): rund **0,11 s** nach der Startzeile.
  Der Haufen steht dort in allen drei Armen bei `free=7840 max=7104`. Bis zum Ende von `setup()` unterscheiden sich die
  Arme nicht. Die Begründung in `design.md` §2.2 N1 Punkt 4 (◐) ist damit belegt.
- **Der Fehlbetrag ist die dauerhafte Zunahme nach `setup()`** (✔ FA, an den Zahlen aus L500 gerechnet):
  7'840 − 7'336 = 504 = 7'104 − 6'600; E Lauf 8: 672 = 672 (dort 168 Byte mehr belegt). In B kommen nach L501 560 Byte
  dazu, `max` bleibt 7'104 — alles ausserhalb des grossen Blocks. Die Aussage aus §1 „eine Verschiebung, keine zusätzliche
  Belegung“ bleibt richtig: In A und B wird gleich viel belegt, nur an anderer Stelle.
- **Im Anwendungscode belegt zwischen +3 und +30 s nichts dauerhaft** (✔ FA, Fundstellen in L501; davon nachgelesen ✔ SW:
  Update-Cache frühestens 60 s nach dem Start, `http.cpp:13935`). Der Verursacher liegt vermutlich in SDK/lwIP oder in
  einer Zeile vom STM (? FA, keine Fundstelle).
- **Von einer frühen Anfrage bleibt nur ein PCB in TIME_WAIT** (rund 192 Byte; 120 s oder bis `tcp_tw_limit()`). Der
  `ClientContext` ist schon im nächsten `loop()`-Durchlauf frei: `http_server_loop()` weist `http_client` in jedem
  Durchlauf neu zu, und ohne wartende Verbindung liefert `accept()` einen leeren `WiFiClient` (✔ FA, ✔ SW:
  `http.cpp:17376`, `WiFiServer.cpp:150-151`; TIME_WAIT 2 × `TCP_MSL` = 120 s, `tcp_priv.h:137`; Begrenzung
  `http.cpp:500-506`, aufgerufen `:17384`).
- **Modell „Verdrängung“ (◐ FA, gefolgert; Lesart L-b′):** Der TIME_WAIT-PCB liegt nach Best-Fit im kleinen freien Bereich
  (Lauf 9: `free` −272 bei unverändertem `max`). Kommt dann die Hintergrundbelegung, passt sie dort nicht mehr; Best-Fit
  nimmt den grossen Block und schneidet vom Anfang ab (`umm_malloc.cpp:729-733`, `:777-798`). Läuft der PCB ab, verschmilzt
  der kleine Bereich; die 504 Byte liegen vorn im ehemals grossen Block.
- **Zusatzprobe (✔ L, L501, 17.10.2026 18:23–18:52, `tools/startprobe.py R,E8,E14,E10,R,E12,B`):** eine einzige Anfrage
  nach 8,1 / 10,1 / 12,1 / 14,1 s ⇒ `max` 7'104 (4 von 4 sauber); Kontrolle B ⇒ 7'112; **RST** (nach 3,1 s nur Verbindung
  aufbauen und per RST abbrechen, ohne Daten — auf der Uhr kein TIME_WAIT) ⇒ 7'104 und 6'928. Mit L500 (eine Anfrage nach
  1,9–3,2 s ⇒ 3 von 3 zerstückelt) liegt die Hintergrundbelegung **rund zwischen +3 und +8 s**. Im Mitschnitt fällt dort nur
  der Aufruf `display_clock flags=0x04` des STM auf, jedes Mal bei +7,43 bis +7,47 s (? FA, Zusammenhang Vermutung).
- **Was das Modell nicht erklärt (✔ L):** Ein RST-Lauf ohne TIME_WAIT war sauber, der andere zeigte einen kleineren
  Fehlbetrag — 176 Byte im grossen Block, 56 Byte mehr belegt. Das Modell trägt damit zur Hälfte; Stufe 2 misst RST mit
  (F6).
- **Folge für Stufe 1:** Das Raster s0–s8 liegt ganz in `setup()` und trifft den Schaden nicht. Das Instrument gehört in die
  ersten Sekunden **nach** `setup()`: Verursacher-Töpfe je Aufruf in `loop()`, Karte X beim ersten dauerhaften Rückgang,
  Karte T beim Ablauf der TIME_WAIT-PCBs, S im ersten `loop()`-Durchlauf (`design.md` §4 N2).

---

## Ziel

1. **Den Trenner benennen** — welche Belegung nach einer frühen Anfrage den grössten freien Block teilt und in welchem
   Schritt sie entsteht —, mit Beleg am Gerät (Stufen V, 1, 2).
2. **Nach Deinem Entscheid eine Abhilfe**, mit der eine frühe Anfrage den Ruhewert nicht mehr senkt: Arm A so sauber wie
   Arm B, ohne Rückschritt anderswo (Stufe 3).
3. **Das Instrument wieder zurückbauen** (Stufe R).

**N2:** Zu Ziel 1 heisst „in welchem Schritt“ ab N2 „in welchem Topf und zu welchem Zeitpunkt nach t0“ — und ob das
Verdrängungsmodell (L-b′) trägt.

---

## Entscheidungen

| Kennung | Frage | Stand | Wer | Wirkt auf |
|---|---|---|---|---|
| **Ent-C1** | Weg für Heap-Karte und Wegmarken (M1, M3, M4): **A** Blockgang über den inneren Aufbau des Haufens, an Core 3.1.2 gebunden; **B** nur öffentliche Mittel, ohne Karte; **C** öffentlich mit Sondierbelegung (`design.md` §4.9) | **N1: entschieden 17.10.2026 — A** (Dein Entscheid nach dem Machbarkeitsbericht 1.1): Blockgang über den inneren Aufbau des Core 3.1.2 (`design.md` §4.2 N1, §4.9 N1). *Vorher:* offen — Vorschlag A; vorgelegt nach dem Machbarkeitsbericht (Task 1.1). Rückfrage an Dich, weil R1.3 im Paket 2026-10-15 ausdrücklich „kein privater Kopf“ gewählt hat (L468) | Nutzer, nach Bericht 1.1 | AKI.2–AKI.6, AKI.8, Stufe 2 |
| **Ent-C2** | Grenze „Globale Variablen“ für Release I | **N1: bestätigt 17.10.2026 — ≤ 320 Byte** (Lead). Der Bericht 1.1 rechnet rund 290 Byte — darunter, aber knapp (`design.md` §4.6 N1). *Vorher:* offen — Vorschlag **≤ 320 Byte** (gerechnet rund 250, `design.md` §4.6); darüber anhalten und vorlegen. **N2:** Grenze bleibt; gerechnet für den Umfang nach N2 rund **304 Byte** (`design.md` §4.6 N2), mit fester Kürzungsreihenfolge | Lead, bei Freigabe | AKI.9 |
| **Ent-C3** | Abhilfe | **offen bis nach Stufe 2** — der Lead legt Dir die Optionen aus dem Nachtrag per Rückfrage vor | Nutzer | Stufe 3 |
| **Ent-C4** | Zeitpunkt des Rückbaus | **N1: bestätigt 17.10.2026** (Du, bei der Freigabe): Rückbau im nächsten ESP-Release **nach** der Abnahme von Stufe 3. *Vorher:* **gesetzt:** im nächsten ESP-Release **nach** der Abnahme von Stufe 3 — Release A trägt das Instrument noch, damit AKA.5 die Karte lesen kann. Du kannst das bei der Freigabe ändern | spec-writer, Freigabe durch Dich | AKZ.7 |
| **Ent-C5** (N2) | Beginn der Folge (Smoketest, `check-pwa.sh`) in den Läufen von Stufe 2 und damit Länge des Startfensters W. **A** (Vorschlag): Folge ab **+150 s** nach der Startzeile, W = 60 s — Töpfe und Karte X sehen nur den Auslöser des Arms, und der TIME_WAIT-PCB einer frühen Anfrage läuft bei rund +120 bis +123 s ungestört ab, sodass Karte T eintreten kann; ein Lauf dauert rund 7 statt 5 min. **B**: Folge wie in V.2 ab **+30 s**, W = **25 s** — vergleichbar mit L499/L500, aber Karte T tritt in A und E praktisch nie ein (die TIME_WAIT-PCBs der Folge halten `tcp_tw_pcbs` bis über +180 s besetzt); F2 hängt dann allein an der Ruhekarte (`design.md` §5.1 N2) | **offen — mit der Freigabe von N2** | Nutzer | W in der Firmware (AKI.4 N2), AKI.14, AKM.1, AKM.2, Tasks 1.3, 1.6, 2.1 |

---

## Akzeptanzkriterien

### Stufe V — Vorprobe ohne Build (auf ESP 3.2.41)

- [x] **AKV.1 (Werkzeug der Arme)** — Das Werkzeug `vier-arme.py` liegt unter `tools/` (Name beim Lead) und kann die Arme
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
      **Stand N1 — teilweise belegt:** Das Werkzeug liegt als **`tools/startprobe.py`** vor (Commit `f012309`, Task V.1),
      mit den Armen A–E, der Mitschrift der ersten Antwort, Exit-Codes 0/1/2 und `--abnahme`; eine Attrappe des
      Logger-Zugriffs (`STARTPROBE_LOG`) und die Doppelausgabe auf Konsole und Mitschrift sind im Kopf des Werkzeugs
      beschrieben (✔ SW). **Gegenprobe (b) eingelöst** (✔ L, L500): `--abnahme 7104` meldet jeden Lauf von A und E als
      „FEHL … max … unter dem Soll 7104“, Exit 1. **Gegenproben (a) und (c):** Ein Beleg, dass sie gefahren wurden und
      angeschlagen haben, steht nicht im Messprotokoll (Suche in `BEFUNDE.md`, Stand `6b3cbe9`) — der Lead trägt ihn nach;
      bis dahin offen.
      **Stand N2 — erfüllt** (✔ B, L500 „Nachtrag Protokoll“): (a) Attrappe ohne Startzeile für die Arme A, B und E ⇒ je
      „FEHLT Lauf n Arm X: Startzeile (FIRMWARE ...) binnen 6 s“, Exit 1; (c) Konsole und Mitschrift-Datei zeilengleich
      (`diff` ohne Unterschied); dazu Aufruf mit unbekanntem Arm oder `--abnahme` ohne Wert ⇒ Exit 2. Der Satz „bis dahin
      offen“ oben ist **überholt (N2)**. *Am Rand:* In der Zusatzprobe meldete das Werkzeug die Läufe E8–E14 fälschlich
      „ungültig (Soll 0)“ (Name `rst` doppelt verwendet); behoben, mit Gegenprobe an Attrappe und lokalem Server (L501). Der
      Fehler betraf die Prüfung aus AKV.2, nicht die Gegenproben (a)–(c); die Messwerte selbst waren gültig.
- [x] **AKV.2 (Arm E)** — Drei Läufe Arm E: Bis die Startzeile im Mitschnitt steht, öffnet das Werkzeug **keine**
      Verbindung zur Uhr (vom Werkzeug geprüft und gemeldet); danach genau **eine** `device_ready`-Anfrage. Abstand zur
      Startzeile und Ruhewert berichtet. **Ein Lauf mit Abstand über 5,9 s ist ungültig** (oberer Rand des schlechten
      Fensters aus M0) und wird wiederholt. Kein Soll für den Ruhewert — das Ergebnis trennt die Lesarten
      (`design.md` §2.3).
      **Stand N1 — erfüllt** (✔ L, L500): drei Läufe E (Läufe 3, 4, 8); vom Werkzeug im Mitschnitt geprüft 0 Anfragen
      zwischen `rst cause` und Startzeile, danach bis +29 s genau eine; Abstand 1,9 / 3,2 / 1,9 s — kein Lauf ungültig;
      Ruhewert `max` 6'600 · 6'600 · 6'432, `free` 7'336 · 7'336 · 7'168. **Ergebnis: E zerstückelt wie A.**
- [x] **AKV.3 (Arm A mit Rumpf, Arm B als Kontrolle)** — Je drei Läufe A und B, im Wechsel mit E in der Reihenfolge aus
      `design.md` §3.3. **Arm A reproduziert L499** — `max` je Lauf mindestens 256 Byte unter dem kleinsten `max` der
      B-Läufe derselben Reihe. Gelingt das nicht, ist die Reproduktion verloren; das steht zuerst im Messprotokoll, und
      Stufe 1 wartet auf einen Nachtrag. Je A-Lauf die Felder der ersten Antwort nach AKV.1.
      **Stand N1 — erfüllt** (✔ L, L500): Reihenfolge A B E · E A B · B E A; A 6'600 · 6'600 · 6'600 gegen B 7'104 ·
      7'104 · 7'104 — je 504 Byte unter dem kleinsten B-Wert, Soll 256 erfüllt. Erste Anfrage in A 0,3–0,6 s nach der
      Startzeile, nach 7–8 Versuchen. Felder der ersten Antwort je A-Lauf: `http_accept_dead` 0, `tcp_now_pending` 0,
      `tcp_now_unclaimed` 0, `tcp_now_list` eine Verbindung (die eigene). Lauf 5 (Arm A) hatte einen fremden
      Verursacher — siehe AKV.4.
- [x] **AKV.4 (Bedingungen der Vorprobe)** — Logwache läuft mit, `--pruefe` am Ende (DIR-013); Beginn und Ende
      angekündigt (R5); kein Browser an der Uhr, kein Upload, kein zweites Werkzeug (N6); ein Ruhewert gilt nur mit
      `tcp=0/0/0/0/0/0` und ≥ 150 s Abstand; jede Abweichung gekennzeichnet; das Ergebnis steht im Messprotokoll, bevor
      der nächste Task beginnt (L424).
      **Stand N1 — teilweise belegt** (L500): Logwache aktiv; Ruhewerte mit `tcp=0/0/0/0/0/0` und ≥ 150 s; Ergebnis als
      L500 im Messprotokoll (Commit `6b3cbe9`). **Abweichung, gekennzeichnet:** In Lauf 5 (Arm A) lud bei +14 s ein
      Safari-Tab aus der wiederhergestellten Sitzung des Nutzers, auf dem Mac, an dem die Messung lief, `settings_xml` und
      `stm32_log` und schickte `ambilight_online_set` (bekannter Abgleich aus L462, B61) — Exklusivität in diesem Lauf
      nicht gegeben. Ergebnis gleich den übrigen A-Läufen; ohne Lauf 5 bleibt A 2 von 2 bei 6'600, AKV.3 hält auch dann.
      **Nicht im Messprotokoll:** das Ergebnis von `watch-log.sh --pruefe` am Ende und die Ankündigung von Beginn und
      Ende — der Lead trägt beides nach; bis dahin offen.
      **Stand N2 — erfüllt, mit gekennzeichneten Abweichungen** (✔ B, L500 „Nachtrag Protokoll“): Beginn und Ende im
      Gespräch angekündigt und im Mitschnitt markiert (`startprobe: Lauf n Arm X`, `startprobe: Ende`). Die Logwache meldete
      über das Fenster 16:35–17:16, also bis nach dem Ende des Laufs, genau die neun gewollten Neustarts, keine Exception,
      keinen Watchdog-Reset, keinen Aussetzer und **einen Antwortverlust** (Lauf 9, 17:11:12, 60 s nach dem Neustart
      während des Smoketests, Bild von L463; für den Ruhewert ohne Belang). **Abweichungen, gekennzeichnet:** (1)
      `watch-log.sh --pruefe` lief **vor** dem Lauf („lebt, letzter Herzschlag vor 16 s“), nicht am Ende; das Ende ist durch
      die Auswertung bis 17:16 gedeckt. (2) Die Alarmwache des Leads lief um rund 17:05 ab und meldete Lauf 8 und 9 nicht;
      der Antwortverlust fiel erst beim Nachlesen auf, und ein erster Entwurf des Nachtrags behauptete „keinen
      Antwortverlust“ — berichtigt. (3) Lauf 5 wie oben. Der Satz „bis dahin offen“ oben ist **überholt (N2)**.

### Stufe 1 — Instrument (Release I, nur ESP)

- [ ] **AKI.1 (Grundsätze jeder neuen Messzeile)** — (a) **belegt keinen Heap**: Aufbau mit `snprintf` in einen
      Stackpuffer, kein `String`, kein `malloc`/`new`, keine Container; (b) **einmal formatiert, zweimal ausgegeben** —
      `Serial.println()` und `stm32_log_append()` mit demselben Puffer (Muster `esp_heap_log()`,
      `ESP-uclock.ino:639-641`); (c) längste Form **≤ 119 Zeichen** (`STM32_LOG_LINE_LEN` 120, `ESP-uclock.ino:652-653`);
      (d) Ausgabe ausschliesslich aus `loop()`, **keine neue Ausgabe in `setup()`**; (e) höchstens **eine** Messzeile
      dieses Pakets je Sekunde und keine im selben `loop()`-Durchlauf wie eine Heap-Zeile; (f) Präfix `- heap ` gefolgt
      von einem Wort, das keine bestehende Zeile trägt (`karte`, `start`, `marken`); (g) **keine IP-Adresse, kein Port,
      kein Name eines Netzes** in einer Zeile.
      **N2:** (e) gilt weiter und wird ergänzt: **Im Startfenster W gibt das Instrument keine einzige Zeile aus**; alles, was
      in W erfasst wird, erscheint erst nach dem Ende von W, eine Zeile je Sekunde. Grund: Jede Ausgabe ruft
      `Serial.flush()` und gibt damit die CPU ab (`design.md` §2.1) — eine Ausgabe in W verfälschte die Töpfe (AKI.4 N2).
      (f) Die Wörter ab N2: `karte`, `marken`, `topf`; **`start` entfällt**. Ausserdem ab N2: (h) `setup()` bleibt
      **unverändert** — auch kein Erfassen mehr dort (die Startschritte entfallen, AKI.4 N2).
- [ ] **AKI.2 (M1 Heap-Karte)** — Zeile `- heap karte <anlass> ent=<belegt>/<frei> fsum=<n> gross=<blk>+<len>
      zweit=<blk>+<len> davor=<blk>+<len> danach=<blk>+<len>` nach `design.md` §4.2. Anlass `m` zu **jeder** Heap-Zeile,
      erfasst im **selben** `loop()`-Durchlauf wie die Heap-Zeile, ausgegeben frühestens 1 s danach. Der Blockgang ist
      eine **reine Funktion** über Blockfeld und Blockzahl (Prüfstand ohne Gerät, AKI.8), läuft unter einer Sperre, die
      gleichzeitige Belegungen ausschliesst, und hat die Wächter aus §4.2: Blockgrösse ≠ 8, Index ≥ Blockzahl, nicht
      steigender Folgeindex, Iterationsgrenze ⇒ statt Werten `abbruch=<grund>`; die Sperre wird auf **jedem** Weg
      gelöst. Mit `ESP_HEAP_LOG` 0 entfällt M1.
      **N1 (Bericht 1.1, Ent-C1 = A):** (i) Alle Längen der Zeile (`fsum`, `gross`, `zweit`, `davor`, `danach`) stehen in
      **Blöcken × 8**, nicht als nutzbare Grösse 8n − 4; Blockindizes wie im Haufen (Block 0 = Kopf der Freiliste).
      (ii) „Im selben Durchlauf“ heisst genauer: im **selben Fenster ohne CPU-Abgabe** wie die Werte der Heap-Zeile — in
      `esp_heap_log()` vor `Serial.println`/`flush` (`ESP-uclock.ino:658-678`, ✔ ED). (iii) Fünfter Wächter **`sum`**:
      Summe der Längen aller Einträge, belegt und frei, in Blöcken ≠ Blockzahl − 2 ⇒ `abbruch=sum`. (iv) Die Sperre setzt
      ein Wrapper um die reine Gangfunktion und löst sie **ohne Verzweigung** (`design.md` §4.2 N1).
      **N2:** (v) „zu **jeder** Heap-Zeile“ ist **überholt (N2)**: Karte `m` entsteht nur zu Heap-Zeilen **ausserhalb** von
      W. Die erste Heap-Zeile (im ersten `loop()`-Durchlauf) bekommt keine `m`-Karte; die Lage dort zeigt `S`. (vi) Die
      Ereigniskarten `S`, `F`, `X`, `T` tragen am Ende das Feld **`ms=<n>`** — Millisekunden seit t0 (`S`: 0); `m` trägt es
      nicht. Weil jedes Ereignis nur bis 180 s nach t0 erfasst wird (AKI.5 N2), hat `ms` höchstens sechs Stellen; längste
      Form **118 Zeichen** (`design.md` §4.2 N2).
- [ ] **AKI.3 (M1 gegen die Heap-Zeile)** — Die Länge `gross` steht zu `max=` derselben Lesung in einer festen, im Bericht
      des `esp-developer` genannten Beziehung (gleich oder fester Unterschied), ebenso `fsum` zu `free=`; der Prüfstand
      prüft beide. **Am Gerät** gilt die Beziehung in zehn aufeinanderfolgenden Paaren aus Heap-Zeile und Kartenzeile in
      Ruhe; eine Abweichung ist ein Fehler des Instruments, kein Befund am Haufen.
      **N1 (Bericht 1.1, ✔ ED):** Die Beziehung ist **Gleichheit** — `gross` (Länge) = `max=` und `fsum` = `free=`,
      **exakt**, sofern AKI.2 N1 (i) und (ii) eingehalten sind. `max=` ist der grösste freie Eintrag × 8, `free=` ist
      `stats.free_blocks` × 8. Ein fester Unterschied ist damit nicht mehr zulässig; der Prüfstand prüft Gleichheit.
- [ ] **AKI.4 (M2 Startschritte)** — An den neun Schritten s0–s8 aus `design.md` §4.3 werden erfasst: `ms` seit dem
      Start, `free`, Beginn und Länge des grössten freien Blocks, Zahl der belegten Einträge, Zahl der Einträge in
      `_unclaimed` des Servers (vor dem Serverstart 0) und Zahl der TCP-PCBs ausserhalb LISTEN und TIME_WAIT. Ausgabe
      nach dem Ende von `setup()` als **eine Zeile je Schritt** `- heap start s<n> …` nach AKI.1 (e); die Zeile für s8
      trägt zusätzlich `us=<Dauer des Blockgangs bei s8>`.
      **N1 (Bericht 1.1):** (i) Jede `start`-Zeile trägt zusätzlich **`srv=<0|1>`** — ob der Server bei diesem Schritt
      lauscht. s2 heisst nicht immer „Server lauscht“: Scheitert `wifi_connect()`, startet der Server später aus `loop()`
      (`wifi.cpp:91-108`, ✔ ED). (ii) `u` liest eine **neue exportierte Funktion in `http.cpp`**; `_unclaimed` ist anders
      nicht erreichbar (✔ ED). (iii) Die erste Heap-Zeile kommt im ersten `loop()`-Durchlauf; die `start`-Zeilen
      erscheinen deshalb frühestens ab dem zweiten (AKI.1 (e)). **Vorbehalt aus V.2:** Nach L500 entsteht der Schaden
      **nach** `setup()`; das Raster s0–s8 allein trifft ihn nicht. Ob M2 so bleibt oder um Erfassungspunkte nach
      `setup()` ergänzt wird, legt ein Nachtrag nach V.3 fest (Task V.4) **[offen: V.3]** (→ N2).
      **N2 — überholt:** der ganze Text oben samt N1 (s0–s8, `u`, `p`, `srv=`, `us=` bei s8, die Zeile `- heap start`, die
      exportierte Funktion für `_unclaimed`). Grund (✔ FA, L501): Bis zum Ende von `setup()` sind alle Arme gleich; der
      Schaden entsteht danach. **Neu gefasst — M2 Verursacher-Töpfe** (`design.md` §4.3 N2): (a) In jedem
      `loop()`-Durchlauf innerhalb des Startfensters W wird `ESP.getFreeHeap()` (O(1), ✔ SW) unmittelbar vor und nach
      jedem der zehn Aufrufe bzw. Blöcke in `loop()` gelesen (`ESP-uclock.ino:1548-1582`, Töpfe `wif` bis `stm`), dazu der
      Topf **`sys`** = Unterschied zwischen dem Ende eines Durchlaufs und dem Anfang des nächsten (Core, SDK, lwIP). (b) Je
      Topf: **Nettosumme** aller Änderungen in W (vorzeichenbehaftet), **grösster Einzelrückgang** und dessen **`ms` seit
      t0** (ohne Rückgang `@-`). (c) Nach dem Ende von W wird nichts mehr gemessen. (d) Ausgabe nach W: eine Kopfzeile
      `- heap topf kopf t0=<ms> w=<ms> n=<durchläufe> free0=<n> free1=<n> us=<n>` und vier Zeilen
      `- heap topf <i>/4 <name>=<netto>/<rückgang>@<ms> …` mit höchstens drei Töpfen je Zeile (längste Form 99 bzw. 84
      Zeichen). `free0` = `free` am Anfang des ersten Durchlaufs, `free1` = `free` am Anfang des ersten Durchlaufs nach W,
      `n` = Zahl der Durchläufe in W, `us` = längster Blockgang seit dem Start (AKI.9). (e) **Summenprobe:** Summe aller
      elf Nettowerte = `free1` − `free0`, **exakt**. Eine Abweichung ist ein Fehler des Instruments (ein Weg ausserhalb
      jedes Topfs), kein Befund am Haufen. (f) Die Reihenfolge und die Aufrufe in `loop()` bleiben unverändert; die Töpfe
      umschliessen sie nur.
- [ ] **AKI.5 (M3 Ereigniskarten)** — Je eine Zeile im Format von AKI.2 mit Anlass **`S`** (Ende von `setup()`), **`F`**
      (nach der Rückkehr aus dem `http_server_loop()`-Durchlauf, in dem die erste Anfrage angenommen wurde) und **`U`**
      (nach der Zeile `- update cache nach`), je Start genau einmal, erfasst im Augenblick des Ereignisses, ausgegeben
      verzögert nach AKI.1 (e). Tritt ein Ereignis nicht ein, fehlt seine Zeile — das ist kein Fehler und wird vom
      Werkzeug (AKM.2) als „Ereignis nicht eingetreten“ gemeldet.
      **N1 (Bericht 1.1):** `F` braucht einen Merker aus `http.cpp`, weil `http_first_request_logged` dort `static` ist
      (`http.cpp:680`, ✔ SW) — über eine neue exportierte Funktion in `http.cpp`. Ob nach L500 ein weiteres Ereignis
      **vor** der ersten Anfrage gebraucht wird, entscheidet der Nachtrag nach V.3 **[offen: V.3]** (→ N2).
      **N2 — neu gefasst** (`design.md` §4.4 N2); überholt sind oben die Erfassungspunkte von `S` und `F` und die Karte `U`.
      Ereigniskarten ab N2, je Start höchstens einmal, alle am **Anfang** eines `loop()`-Durchlaufs erfasst, ausgegeben nach
      AKI.1 (e) N2:
      (a) **`S`** — im ersten `loop()`-Durchlauf (gleichwertig zum Ende von `setup()`, ✔ FA: dazwischen liegt nichts;
      `ms=0`). Ihr `free` heisst **`free(S)`**.
      (b) **`F`** — im ersten Durchlauf, vor dem `http_server_loop()` nach der Annahme der ersten Anfrage mindestens einmal
      **ohne** neue Annahme gelaufen ist. Dann ist der `ClientContext` frei und vom Verkehr nur der PCB übrig. `F` braucht
      eine Lesefunktion aus `http.cpp` (Zähler der Annahmen oder Merker der ersten Anfrage; Wahl beim `esp-developer`).
      (c) **`X`** — im ersten Durchlauf innerhalb von W, an dessen Anfang `free < free(S) − 400` gilt, **während keine
      aktive TCP-Verbindung besteht** (`tcp_active_pcbs` leer; PCBs in TIME_WAIT zählen nicht als aktiv).
      (d) **`T`** — im ersten Durchlauf bis 180 s nach t0, an dessen Anfang `tcp_tw_pcbs` leer ist, nachdem die Liste am
      Anfang des vorigen Durchlaufs mindestens einen Eintrag hatte.
      (e) `F`, `X`, `T` werden nach 180 s (`X`: nach dem Ende von W) nicht mehr erfasst. **`U` entfällt** — der Update-Cache
      läuft frühestens 60 s nach dem Start (`http.cpp:13935`, ✔ SW) und trägt nach V.3 kaum noch Gewicht.
      (f) Nicht eingetreten ⇒ keine Zeile; das Werkzeug meldet „nicht eingetreten“ (unverändert).
- [ ] **AKI.6 (M4 Wegmarken)** — Bei `S` und bei `F` je eine Zeile `- heap marken <anlass> …` mit dem Blockindex — in
      derselben Einheit wie die Karte — jedes Objekts aus der Liste nach Ent-C1 (`design.md` §4.5). Ein Objekt der Liste,
      das sich nicht erreichen lässt, steht mit Grund im Bericht 1.1, nicht geschätzt in der Zeile. Für TCP-PCBs nur
      Index und Zustand, keine Gegenstelle (AKI.1 (g)).
      **N1 (Bericht 1.1, ✔ ED):** Alle Objekte der Liste sind ohne Eingriff in den Core erreichbar, dazu genau: die
      **`UdpContext`** über `recv_arg` der UDP-PCBs, die **`ClientContext`** über `callback_arg` der TCP-PCBs, der
      `ClientContext` des Wetter-Clients über ein Zugriffsglied in einer Unterklasse und der Puffer von `EEPROM` über
      **`getConstDataPtr()`** (nicht `getDataPtr()`, das `_dirty` setzt). Liste, Grössen und Zeilenform: `design.md` §4.5
      N1. Die Näherung „PCB für seinen Kontext“ entfällt. `weather.cpp` bleibt unverändert.
      **N2:** „Bei `S` und bei `F` je eine Zeile“ ist **überholt (N2)**. Ab N2 genau **ein** Satz Wegmarken je Start:
      erfasst zusammen mit Karte `X` (Anlass `X`), oder — ist `X` bis zum Ende von W nicht eingetreten — am Ende von W
      (Anlass `W`). Die Zeile führt **`tcp` zuerst**; bei `X` sind das genau die PCBs in `tcp_tw_pcbs` (keine aktive
      Verbindung), gelesen wie `http.cpp:745`. Die übrigen Objekte und Regeln nach §4.5 N1 (`design.md` §4.5 N2).
- [ ] **AKI.7 (Schalter und Bindung)** — Schalter **`ESP_HEAP_MAP`** (in Release I 1). Mit 0 entsteht keine der neuen
      Zeilen, und „Globale Variablen“ ist gleich dem Stand ohne Instrument. Ein Übersetzungsfehler, wenn `ESP_HEAP_MAP` 1
      und `ESP_HEAP_LOG` 0 ist. Bei Ent-C1 = A bricht die Übersetzung mit einer Meldung ab, wenn der Core nicht 3.1.2 ist
      (Mittel bestimmt der `esp-developer`). Ein Kommentarblock nennt Zweck, Kosten und die Rückbaupflicht nach dem Muster
      von `ESP_HEAP_LOG` (`ESP-uclock.ino:579-582`).
      **N1 (Bericht 1.1, Ent-C1 = A):** Mittel der Bindung — eigene Übersetzungseinheit (z. B. `heapkarte.cpp`);
      `#error`, wenn `ARDUINO_ESP8266_RELEASE_3_1_2` nicht definiert ist; `static_assert` auf
      `sizeof(umm_heap_context_t) == 52` und `offsetof(umm_heap_context_t, numblocks) == 48`; Prüfung `UMM_NUM_HEAPS == 1`
      zur Übersetzungszeit; Blockgrösse 8 zur Laufzeit (Wächter `bs`). Der Prüfstand prüft statisch, dass alle vier
      Mittel vorhanden sind (AKI.8 N1).
      **N2:** Mit `ESP_HEAP_MAP` 0 entfallen auch die Töpfe — `loop()` ist dann Byte für Byte wie in 3.2.41. Die
      Töpfe-Rechnung steht als reine Funktion in der eigenen Übersetzungseinheit, damit der Prüfstand sie allein übersetzen
      kann.
- [ ] **AKI.8 (Prüfstand c80i)** — Prüfstand ohne Gerät mit den Fällen und Sabotagen aus `design.md` §4.8, darunter
      synthetische Haufen mit bekannter Lage (exakte Felder), beschädigte Ketten (je ein `abbruch=`-Grund), längste Form
      ≤ 119, „keine Belegung“ statisch, beide Ausgabewege, `ESP_HEAP_MAP` 0, keine Ausgabe in `setup()`, höchstens eine
      Zeile je Durchlauf. **Jede Sabotage ⇒ FEHL**; gegen 3.2.41 ⇒ FEHL; in S16, Fallzahl nachgezählt (L235).
      **N1:** zusätzlich die Fälle und Sabotagen aus `design.md` §4.8 N1 — Wächter `sum`, Gleichheit `gross`/`max` und
      `fsum`/`free` an synthetischen Haufen, die Bindungsmittel aus AKI.7 N1, `getConstDataPtr()`; Sabotagen „Längen
      als 8n − 4“, „Wächter `sum` entfernt“, „`getDataPtr()` statt `getConstDataPtr()`“.
      **N2:** zusätzlich die Fälle 15–22 und Sabotagen aus `design.md` §4.8 N2 (Töpfe, Summenprobe, Bedingungen von `F`,
      `X`, `T`, keine Ausgabe in W, `setup()` unverändert, ein Satz Wegmarken). **Überholt (N2):** Fall 14 und die
      Sabotage „`srv=` fehlt“ (die `start`-Zeile entfällt).
- [ ] **AKI.9 (Kosten)** — Δ „Globale Variablen“ gegen 3.2.41 ≤ Ent-C2, darüber anhalten und vorlegen; Programmgrösse
      vorher und nachher im Messprotokoll; Dauer eines Blockgangs am Gerät (`us=` aus AKI.4) in drei Starts berichtet,
      **über 1'000 µs ⇒ vorlegen** (◐ Vorschlag); Byte je Minute auf der Brücke im Betrieb und je Start berichtet.
      **N1 (Bericht 1.1, ◐ ED gerechnet):** BSS rund 290 Byte, in einer Struktur gefasst (Ausrichtung; TCP-Wegmarke
      3 Byte statt 2) — unter Ent-C2 (320), aber knapp; Programm 1,5–2,5 KB; Gangdauer geschätzt 70–200 µs, theoretisch
      bis rund 1 ms. Für die endgültige Zeilenform der Wegmarken (`design.md` §4.5 N1) und bei einem Nachtrag nach V.3
      rechnet der `esp-developer` neu, **bevor** gebaut wird.
      **N2:** (i) Gerechnet für den Umfang nach N2: **rund 304 Byte** (◐ spec-writer, `design.md` §4.6 N2) — 16 Byte unter
      Ent-C2. Der `esp-developer` rechnet in Task 1.3 nach, **bevor** gebaut wird. Liegt seine Rechnung über 320, kürzt er
      in dieser Reihenfolge, bis sie darunter liegt: **1.** Nettowerte der Töpfe auf 16 Bit mit Sättigung bei ±32'767
      (−22); **2.** Karte `F` (−28); **3.** Karte `T` (−28). Reicht das nicht ⇒ anhalten und vorlegen. Jede Kürzung steht im
      Taskbericht. (ii) `us=` steht ab N2 in der Kopfzeile der Töpfe (längster Blockgang seit dem Start), nicht mehr bei s8.
      (iii) Brücke je Start: 8–11 Zeilen, höchstens rund 1,3 KB, frühestens ab dem Ende von W und eine Zeile je Sekunde;
      im Betrieb unverändert eine Kartenzeile je Minute.
- [ ] **AKI.10 (Gegenprobe am Gerät, DIR-014)** — Im ersten Start nach Release I und in jedem Lauf von Stufe 2: jede neue
      Zeilenart im Mitschnitt **und** über `/api/stm32_log`, Text gleich. Je Start nachgezählt gegen das Soll — 9 ×
      `start`, je 1 × `S`, `F`, `U` (soweit eingetreten), 2 × `marken`, je Heap-Zeile 1 × `karte m` (L235). Kein
      Anstieg von `ovr=` und kein Sprung in `d=` durch die Startzeilen gegen einen Start auf 3.2.41. Kein `abbruch=` im
      Betrieb.
      **N2:** Das Zählsoll oben ist **überholt (N2)**. Soll je Start ab N2: **1 × `topf kopf`**, **4 × `topf`**,
      **1 × `karte S`**, je 0 oder 1 × `karte F`, `karte X`, `karte T` (soweit eingetreten), **genau 1 × `marken`** (Anlass
      `X` oder `W`), je Heap-Zeile ausserhalb W 1 × `karte m`; **keine** Zeile `start`, `karte U`, `karte m` in W und
      **keine Zeile des Instruments vor dem Ende von W**. Jede Zeilenart im Mitschnitt **und** über `/api/stm32_log`,
      textgleich. Die Summenprobe (AKI.4 N2 (e)) hält in jedem Start. `ovr=`/`d=` und `abbruch=` wie oben, verglichen mit
      einem Start auf 3.2.41.
- [ ] **AKI.11 (bestehende Auswertungen)** — Die Heap-Zeile bleibt Byte für Byte im bisherigen Format. `watch-log.sh`,
      `smoke-device.sh`, `mess-stabilitaet.sh` und das Werkzeug der Arme lesen ihre Werte unverändert. Die Logwache meldet
      `abbruch=` als Warnung und eine gewöhnliche Kartenzeile nicht. Gegenproben über eine eingespeiste Quelle: Zeile mit
      `abbruch=` ⇒ Meldung; gewöhnliche Kartenzeile ⇒ keine Meldung.
      **N2:** Auch `topf`-Zeilen und die Karten `X`, `T` lösen keine Meldung der Logwache aus; Gegenprobe: eingespeiste
      `topf`-Zeile ⇒ keine Meldung. Das Werkzeug der Arme sammelt je Lauf `topf`, `karte` und `marken` in die Mitschrift.
- [ ] **AKI.12 (Einspielen Release I)** — ESP-OTA; danach `install-app.sh --check` vollständig (DIR-017); Smoketest ohne
      FEHLT samt Update-Quelle (DIR-009); `check-pwa.sh` ohne FEHLT; Versionen gleich denen des Tags; Logwache meldet den
      OTA-Neustart. *Der Ruhewert nach diesem Einspielen ist keine Startmessung* — die Werkzeuge fragen sofort an (L499).
- [ ] **AKI.13 (Gegenprobe der Töpfe, DIR-014)** — *Neu mit N2.*
      (a) **Prüfstand** (c80i): Eine synthetische Folge von `free`-Werten je Topf und Durchlauf ergibt exakt die erwarteten
      Netto-, Rückgangs- und `ms`-Werte und eine haltende Summenprobe. **Sabotagen, jede ⇒ FEHL:** ein Aufruf in `loop()`
      ohne Topf (Summenprobe reisst); Messung nach dem Ende von W fortgesetzt; Topf `sys` weggelassen; `ms` des grössten
      Rückgangs beim ersten statt beim grössten Rückgang gesetzt. Statisch: Jeder der zehn Aufrufe bzw. Blöcke von
      `ESP-uclock.ino:1548-1582` liegt in genau einem Topf.
      (b) **Werkzeug** (Lead): Das Werkzeug der Arme rechnet die Summenprobe je Lauf nach und endet bei Abweichung mit
      Exit ≠ 0 und Meldung in Konsole **und** Mitschrift. Gegenprobe: eingespeiste `topf`-Zeilen mit falscher Summe ⇒ FEHL,
      Exit ≠ 0, Meldung an beiden Stellen; richtige Summe ⇒ keine Meldung.
      (c) **Gerät:** In jedem Start ab Release I hält die Summenprobe exakt; in drei Starts im Messprotokoll nachgerechnet.
      Die `topf`-Zeilen kommen im Mitschnitt **und** über `/api/stm32_log` an, textgleich.
- [ ] **AKI.14 (Gegenprobe der Karten X und T, DIR-014)** — *Neu mit N2.*
      (a) **Prüfstand** (c80i): synthetische Folgen von (`free`, Zahl aktiver PCBs, Zahl der TIME_WAIT-PCBs, `ms`) je
      Durchlauf. `X` genau im ersten Durchlauf mit `free < free(S) − 400` ohne aktive Verbindung; nicht, solange eine
      aktive Verbindung besteht; nicht nach dem Ende von W; höchstens einmal. `T` genau beim ersten Übergang von ≥ 1 auf 0
      TIME_WAIT-PCBs; nicht bei 0 → 0; nicht nach 180 s; höchstens einmal. **Sabotagen, jede ⇒ FEHL:** `X` ohne Prüfung der
      aktiven Verbindungen; `X` ohne den Abstand 400; `X` nach W; `T` bei jedem Rückgang der Zahl statt beim Übergang auf 0;
      `T` nach 180 s; `F` im annehmenden Durchlauf statt danach.
      (b) **Gerät, herbeigeführt:** In einem Start mit genau einer Anfrage bis +3,2 s nach der Startzeile und ohne weiteren
      Verkehr bis +180 s (Arm E nach Ent-C5 = A) erscheint `karte T`, und ihr `ms` liegt 118–124 s nach dem `ms` der Karte
      `F` (TIME_WAIT 120 s, ◐ Spanne für den Takt des lwIP-Zeitgebers). `karte X` erscheint in diesem Start in W — tritt sie
      nicht ein, ist das ein Befund für Wid-1 (`design.md` §5.4 N2), kein Fehler des Instruments.
      (c) **Gerät, ausbleibend:** In den Läufen B und RST erscheint vor der Folge keine `karte T`, und das Werkzeug meldet
      „T nicht eingetreten“. Werkzeug-Gegenprobe: Attrappe ohne `marken`-Zeile ⇒ FEHL, Exit ≠ 0; Attrappe ohne `karte T` ⇒
      „nicht eingetreten“, kein FEHL.
      (d) Jede der Zeilen `karte X`, `karte T`, `marken X`/`marken W` kommt im Mitschnitt **und** über `/api/stm32_log` an,
      textgleich.
      *Bei Ent-C5 = B* entfällt (b) in dieser Form; `T` wird dann nur am Prüfstand und in (c) belegt, und das steht so im
      Messprotokoll.

### Stufe 2 — Messung (auf Release I)

- [ ] **AKM.1 (Reproduktion unter dem Instrument)** — Arm A 3 von 3 zerstückelt: `max`(A) mindestens 256 Byte unter dem
      kleinsten `max` der B-Läufe derselben Reihe; Arm B 3 von 3 mit `max` ≥ 7'104 − ΔRAM(I); `free`(A) und `free`(B)
      höchstens 64 Byte auseinander. **Misslingt eines davon, gilt Z4** — Stufe 2 hält an, der `spec-writer` schreibt
      einen Nachtrag zum Instrument.
      **N2:** Bei Ent-C5 = A beginnt die Folge später als in V.2. Misslingt AKM.1, wird **vor** Z4 geklärt, ob die
      verschobene Folge oder das Instrument den Unterschied macht: drei Läufe A auf Release I mit der Folge ab +30 s wie in
      V.2. Zerstückelt A dort ⇒ der Beginn der Folge wirkt (Befund ins Messprotokoll, Nachtrag); sonst Z4.
- [ ] **AKM.2 (Läufe)** — Arme A, B und E je dreimal in der Reihenfolge aus `design.md` §5.1, je Lauf vollständig: neun
      Startzeilen, die Ereigniskarten `S`, `F`, `U`, die Wegmarken `S`, `F` und die Kartenzeilen bis zum Ruhewert. Das
      Werkzeug sammelt sie je Lauf in die Mitschrift und endet mit Exit ≠ 0, wenn eine erwartete Zeile fehlt (ausser
      „Ereignis nicht eingetreten“ nach AKI.5).
      **N2:** Die Liste „neun Startzeilen, … `U`, … Wegmarken `S`, `F`“ ist **überholt (N2)**. Ab N2: Arme A, B, E je
      dreimal (A B E · E A B · B E A), danach **zwei Läufe RST**; Folge nach Ent-C5. Arm E ist nur gültig mit genau einer
      Anfrage **bis 3,2 s** nach der Startzeile (oberer Rand der zerstückelten E-Läufe, L500; ab 8,1 s sauber, L501) — sonst
      wiederholt. Je Lauf vollständig nach dem Zählsoll von AKI.10 N2; das Werkzeug endet mit Exit ≠ 0, wenn eine erwartete
      Zeile fehlt (ausser „nicht eingetreten“) oder die Summenprobe reisst (AKI.13 (b)).
- [ ] **AKM.3 (Fragen F1–F5)** — Im Messprotokoll mit Zahlen je Lauf beantwortet (`design.md` §5.3): **F1** Lage von
      `gross` und `zweit` in Ruhe, A gegen B; **F2** der Trenner — `davor`/`danach` am grossen Block in Ruhe in Arm A —
      und ob er einer Wegmarke entspricht; **F3** seit welchem Schritt bzw. Ereignis er dort liegt; **F4** belegte
      Einträge in Ruhe, A gegen B (Verschiebung oder zusätzliche Belegung); **F5** Arm E wie A oder wie B, und in Arm A
      `_unclaimed` und PCBs je Schritt s2–s8.
      **N2:** Rangfolge ab N2 **F3 vor F2 vor F4 vor F1** (`design.md` §5.3 N2), je mit Zahlen je Lauf: **F3** wann und in
      welchem Topf die dauerhafte Belegung entsteht (Töpfe, Karte `X`; Arm B liefert das klare Bild); **F2** wer trennt —
      Vorhersage nach L-b′: in A und E ist `davor` von `gross` ein belegter Eintrag von der Länge des Fehlbetrags, in B
      liegt ein gleich langer belegter Eintrag im kleinen Bereich; **F4** gleich viele belegte Einträge in A und B; **F1**
      wie oben. **F5 ist erledigt** (L500: E wie A); „`_unclaimed` und PCBs je Schritt s2–s8“ ist **überholt (N2)** — in der
      ersten Antwort waren `tcp_now_unclaimed` und `http_accept_dead` 0. **F6** (neu, informativ, keine Bedingung für eine
      Zuordnung): was die RST-Läufe zeigen.
- [ ] **AKM.4 (Zuordnung)** — Der `firmware-analyst` ordnet aus dem Messprotokoll nach den Regeln Z1–Z4 zu
      (`design.md` §5.4), mit Kennzeichen ✔/◐ je Aussage. **„Nicht zugeordnet“ (Z3) ist ein zulässiges Ergebnis**; es
      führt zu einem Nachtrag am Instrument (weitere Wegmarken), nicht zu einer Abhilfe.
      **N1:** Z1 setzt „E sauber“ voraus; auf 3.2.41 war E zerstückelt (L500). Z1 ist damit nur erreichbar, wenn sich E
      unter Release I anders verhält (`design.md` §5.4 N1).
      **N2:** Zugeordnet wird ab N2 nach `design.md` §5.4 N2: zuerst die Widerlegungsregeln **Wid-1** (Hintergrundbelegung)
      und **Wid-2** (Abschneiden vom Anfang), dann Z2 (L-b′ oder L-c), Z3 oder Z4. **Z1 entfällt** (L-a und L-a′ tragen
      nicht, L500). Für Z2 ist die **Identität** des Verursachers (Wegmarke) nicht Bedingung — Topf, Zeitpunkt und Länge
      genügen; ist der Verursacher nur als Topf `sys` benannt, sagt der Nachtrag A.1 das ausdrücklich und bietet neben
      Optionen am Mechanismus eine Option „erst identifizieren“ an.
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

**N2 zu Stufe 3:** In AKA.1 heisst „Z1 oder Z2“ ab N2 „Z2“ (Z1 entfällt, AKM.4 N2). Im Übrigen ändert N2 an Stufe 3
nichts; den Beginn der Folge in der Abnahme legt der Nachtrag A.1 fest, innerhalb einer Reihe für alle Arme gleich.

### Übergreifend

- [ ] **AKZ.1 (Kodierung und Zeilenende, DIR-015)** — Vor jedem Patch je Datei im Taskbericht festgestellt: Kodierung
      (ASCII, ISO-8859-1 oder UTF-8, mit Zahl der Nicht-ASCII-Byte) und Zeilenende (Zahl der CRLF- und LF-Zeilen). Bekannt
      und dennoch neu festzustellen: `ESP-uclock.ino` ASCII mit **gemischten** Zeilenenden, `http.cpp` UTF-8 mit einer
      CRLF-Zeile (L468, 0.1); jede weitere Datei und jede neue Datei ebenso. Der Diff enthält nur inhaltlich geänderte
      Zeilen; der Lead prüft mit `git diff --stat`.
      **N1 (Bericht 1.1, Stand `353a693`):** festgestellt — Tabelle in `design.md` §4.10. Vor dem Patch erneut
      festzustellen; gepatcht wird byte-genau **je Abschnitt** (`ESP-uclock.ino` wechselt das Zeilenende in 15
      Abschnitten). Eine neue Moduldatei bekommt CRLF wie die übrigen Moduldateien.
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
      **N1:** Für V.2 nicht vollständig belegt — siehe AKV.4 (Lauf 5 mit fremdem Verursacher; `--pruefe` und
      Ankündigung nicht im Messprotokoll).
      **N2:** Für Stufe V belegt, mit den in AKV.4 N2 gekennzeichneten Abweichungen; die Zusatzprobe (L501) lief mit
      Logwache, bis zum Ende nachgelesen, ohne Exception, Watchdog-Reset oder Antwortverlust. Offen für die Läufe von
      Stufe 1 bis 3.
- [ ] **AKZ.7 (Rückbau, Ent-C4)** — Ab Release I führt `BEFUNDE.md` den Rückbau als offenen Punkt in der ToDo-Liste. Im
      nächsten ESP-Release nach der Abnahme von Stufe 3 (AKA.3–AKA.7) gehen M2–M4 heraus, und M1 geht heraus oder
      `ESP_HEAP_MAP` steht auf 0; c80i ist auf den Rückbau umgestellt (keine neue Zeile, Gegenprobe mit 1 ⇒ FEHL);
      „Globale Variablen“ liegt danach beim Stand ohne Instrument zuzüglich dem Anteil der Abhilfe.
      **N2:** M2 meint ab N2 die Töpfe; nach dem Rückbau ist `loop()` wieder ohne Topf-Messung.
- [ ] **AKZ.8 (Messprotokoll)** — Eine eigene L-Zeile in `BEFUNDE.md` als Messprotokoll dieses Pakets (Lead); jeder
      messende Task trägt sein Ergebnis ein, bevor der nächste beginnt (L424); Nachführung von C80 durch den `doc-writer`,
      nie gleichzeitig mit einer Messzeile (R3).
      **N1:** Das Ergebnis von V.2 steht als **L500** im Messprotokoll. Der Entscheid Ent-C1 = A (Task 1.2: „Entscheid ins
      Messprotokoll“) steht dort noch nicht (Suche nach `Ent-C` in `BEFUNDE.md`, Stand `6b3cbe9`) — der Lead trägt ihn
      nach.
      **N2:** Nachgetragen — Ent-C1 = A, Ent-C2 und Ent-C4 stehen im Nachtrag Protokoll von L500. Lesart V.3 und
      Zusatzprobe stehen als **L501**. Der Satz „steht dort noch nicht“ oben ist **überholt (N2)**.
- [ ] **AKZ.9 (Endstand)** — Zum Schluss ein `endstand.md` dieses Pakets mit dem Endstand jedes Kriteriums (belegt,
      entfallen, offen) und der Zählung.

**Zählung:** AKV 4, AKI 12, AKM 5, AKA 7, AKZ 9 — **37 Kriterien**. **N1:** Zählung unverändert — kein Kriterium neu,
keines entfallen. Davon erfüllt: **AKV.2, AKV.3** (2); teilweise belegt: AKV.1, AKV.4; alle übrigen offen.

**N2:** AKV 4, **AKI 14**, AKM 5, AKA 7, AKZ 9 — **39 Kriterien** (neu: AKI.13, AKI.14; keines entfallen — AKI.4 ist neu
gefasst, nicht gestrichen). Davon erfüllt: **AKV.1, AKV.2, AKV.3, AKV.4** (4); teilweise belegt: AKZ.6 (Stufe V), AKZ.8
(Stufe V); alle übrigen offen. Die Zählung von N1 oben ist **überholt (N2)**.

---

## Nicht Teil dieser Änderung

- **Keine Ablaufregel „nach dem Einspielen neu starten“** und kein Neustart in `install-app.sh`, `deploy.sh`,
  `flash-stm.sh`, `smoke-device.sh` oder einem anderen Werkzeug (Dein Entscheid vom 17.10.2026). Das Werkzeug der Arme
  startet die Uhr nur als Auslöser einer Messung neu.
- **Kein Umbau des Webservers vor dem Beleg** — `http_server_loop()`, `WcHttpServer`, Backlog nach freiem Heap,
  TIME_WAIT-Begrenzung, `http_server_begin()` und sein Aufrufort bleiben in Stufe 1 und 2 unverändert.
  **N1:** Die zwei neuen exportierten Funktionen in `http.cpp` (Zahl der Einträge in `_unclaimed`, Merker für `F`) sind
  reine Lesezugriffe für das Instrument, kein Umbau; das Verhalten des Servers bleibt.
  **N2:** Die Funktion für `_unclaimed` entfällt; es bleibt **eine** Lesefunktion für `F`. Insbesondere bleiben
  `tcp_tw_limit()`, `HTTP_TW_LIMIT` und der Umgang mit TIME_WAIT unverändert — auch wenn das Verdrängungsmodell dort
  ansetzt. Das wäre eine Abhilfe, keine Messung.
- **Keine Abhilfe in Release I.** Auch keine Umstellung der Reihenfolge in `setup()` — sie wäre eine Abhilfe, keine
  Messung.
- Keine Änderung am Core (Arduino-Paketverzeichnis), an Build-Optionen, `Makefile` oder `CMakeLists.txt`.
- **Die Heap-Zeile bleibt unverändert.** Der Vorschlag aus der ToDo-Zeile C80, sie „um die Zahl der Belegungen zu
  ergänzen“, geht in M1 auf — als eigene Zeile, weil Werkzeuge die Heap-Zeile lesen (`ESP-uclock.ino:642`).
- C65, C49 und C54 nicht. **C77** (Wetter-Client, K1) nur, wenn die Zuordnung ihn als Trenner benennt **und** Du das in
  Ent-C3 entscheidest. **N1:** Die Wegmarke `wet` liest den Kontext des Wetter-Clients nur; `weather.cpp` bleibt
  unverändert.
- K30, `TESTPLAN-PWA.md` und die Regeln für T3 bleiben.
- PWA, STM-Firmware und Legacy-Oberfläche bleiben unverändert (ausser ein freigegebener Nachtrag nennt sie).
- Die Auswertung M0 wird nicht wiederholt.
- Die veraltete Zeilenzahl zu `ESP-uclock.ino` in `CLAUDE.md` (1'043 von 1'240 CRLF; L468, 0.1: 2'116 von 2'443) wird
  hier nicht berichtigt — gemeldet an den Lead. **N1:** Der Bericht 1.1 zählt am Stand `353a693` 2'144 CRLF und 327 LF.
- **N1:** Der fremde Verursacher in Lauf 5 der Vorprobe (Safari-Tab der Nutzersitzung, L500) wird hier nicht behandelt;
  er ist ein Ablaufthema des Leads (R5, B61).
- **N2:** Der Verdacht auf die STM-Zeile `display_clock flags=0x04` (+7,43 bis +7,47 s) wird **nur auf der ESP-Seite**
  geprüft — über den Topf `stm`. Am STM wird nichts instrumentiert; `src/**` bleibt unverändert.
- **N2:** Die Reihenfolge und die Aufrufe in `loop()` bleiben unverändert; die Töpfe umschliessen sie nur.
- **N2:** Der kleinere Fehlbetrag im zweiten RST-Lauf (176 Byte, L501) und der um 8 Byte höhere Kontrollwert B 7'112
  werden über F6 beobachtet, nicht eigens verfolgt.

## Betroffene Laufzeiten

- [ ] STM32 (`src/**`) — nicht betroffen
- [x] ESP8266 (`ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino`) — Neu-Flashen nötig (Releases I, A, R)
- [ ] PWA (`data/app/**`) — nicht betroffen; nach jedem OTA nur `install-app.sh --check`
- [x] Build/Release — je Release ZIP, Tag, Rollout; keine Änderung an Build-Dateien
- [x] Werkzeuge (`tools/**`, Lead) — Werkzeug der Arme, Logwache, Prüfstand c80i. **N2:** dazu im Werkzeug der Arme der
  Beginn der Folge als Einstellung (Ent-C5), die Summenprobe und die Meldungen „nicht eingetreten“ für `F`, `X`, `T`
