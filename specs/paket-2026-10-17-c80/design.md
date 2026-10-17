# Design — Paket 2026-10-17 „C80: bleibende Heap-Zerstückelung nach frühem Verkehr — Firmware-Ursache“

**Erstellt:** 2026-10-17, `spec-writer`, Stand `353a693`. Anforderungen: `requirements.md`; Tasks: `tasks.md`.
Kennzeichen und Kennungen wie in `requirements.md`. Momentaufnahme (DIR-006).

**Nachtrag N1** (17.10.2026, Stand `6b3cbe9`): Freigabe; Ent-C1 = A, Ent-C2 ≤ 320 Byte, Ent-C4 bestätigt. Das Ergebnis der
Vorprobe V.2 (`BEFUNDE.md` L500) steht in §1, §2.2–§2.4, §3.4 und §5.4; die Ergebnisse des Machbarkeitsberichts 1.1 des
`esp-developer` in §4.2–§4.10, §8, §9 und §12. Jede Änderung trägt die Marke **N1**, der Text davor bleibt stehen.
Kennzeichen **✔ ED** / **◐ ED** wie in `requirements.md`. Die Lesart der Vorprobe durch den `firmware-analyst` (V.3) steht
noch aus; die Stellen, die auf sie warten, tragen **[offen: V.3]**, ihr Platz ist §3.5.

Diese Spec legt **Messungen** fest, keine Abhilfe. Sie nennt Kandidaten nur, um zu begründen, *was* das Instrument sehen
muss. Ob einer davon zutrifft, entscheidet Stufe 2.

---

## 1. Was belegt ist und was nicht

| Aussage | Kennzeichen | Quelle |
|---|---|---|
| Eine einzige `device_ready`-Anfrage 0,2–0,3 s nach der Startzeile ⇒ `max` 6'600 statt 7'104, 3 von 3, `free` gleich | ✔ L | L499, Versuch mit vier Armen |
| Ab +30 s ⇒ sauber 7'104, 3 von 3; OTA oder Neustart gleichgültig | ✔ L | L499 |
| Bei der ersten Anfrage selbst `max` noch 7'104 | ✔ L | L499 (`- heap erste anfrage free=7568 max=7104`) |
| Erste Anfrage 0,2–5,9 s ⇒ 5 von 5 zerstückelt; ab 16,9 s ⇒ 13 von 14 sauber | ✔ L | L499, M0 |
| Im ESP-Code kein Zweig für „erster Start eines neuen Abbilds“ | ✔ FA | L499 |
| Allokator Best-Fit, wenn nichts anderes gewählt ist | ✔ SW | `umm_malloc_cfgport.h:24-25` (Core 3.1.2); der Analyst nennt `umm_malloc_cfg.h:130-136` |
| Auf Dateiebene gibt es kein `String` | ✔ FA, ✔ SW | L468 (C71); Suche nach `^String`/`static String` in `ESP8266/ESP-uclock/` ohne Treffer |
| 504 Byte = 63 Blöcke zu 8 Byte — Grössenordnung einer Verbindung (PCB, `ClientContext`, Empfangspuffer) | ◐ | nur gerechnet, nicht zugeordnet |
| Welche Belegung den Block teilt, und wann sie entsteht | **offen** | Gegenstand dieser Spec |
| **N1:** Eine einzige Anfrage 1,9 / 3,2 / 1,9 s nach der Startzeile, **ohne jede Verbindung davor** (Arm E) ⇒ `max` 6'600 · 6'600 · 6'432; Arm A 6'600 × 3, Arm B 7'104 × 3 | ✔ L | L500 (Vorprobe V.2) |
| **N1:** In der ersten Antwort von Arm A `http_accept_dead`, `tcp_now_pending`, `tcp_now_unclaimed` je 0, in `tcp_now_list` nur die eigene Verbindung | ✔ L | L500 |
| **N1:** In Arm B fällt der Haufen bis +30 s auch ohne Anfrage (`- heap erste anfrage` bei +30 s `free=6896 max=6568` gegen `7360/6896` bei +2 bis +4 s); der Ruhewert bleibt 7'104 | ✔ L | L500 |
| **N1:** Ein Haufen, 1'590 Blöcke zu 8 Byte; freie Einträge nie benachbart; `max=` und `free=` sind Blockzahlen × 8 | ✔ ED, teils ✔ SW | Bericht 1.1; §4.2 N1 |

---

## 2. Prüfung der Ursachenanalyse

### 2.1 Ablauf von `setup()` (✔ SW, `ESP-uclock.ino`)

| Zeile | Schritt | dauerhafte Belegungen (bekannt) |
|---|---|---|
| `:259-264` | `Serial.setRxBufferSize(1024)`, `Serial.begin()`, `delay(1000)` | Empfangspuffer 1'024 Byte, ganz am Anfang des Haufens (Kommentar `:254-257`, A79) |
| `:284-302` | `CAP var-crc`, `- flash id` | — |
| `:314-316` | `EEPROM.begin(512)`, `eeprom_read()` | Puffer von `EEPROM` (◐, Core) |
| `:318-325` | `wifi_connect()` bzw. `wifi_ap()` — darin Warten bis 15 s mit `delay(500)` (`wifi.cpp:80-89`), dann **`http_server_begin()`** (`wifi.cpp:93`) und drei Statuszeilen mit `Serial.flush()` (`wifi.cpp:99-101`, `base.cpp:333-340`) | Listen-PCB des Servers; Strukturen von SDK und lwIP für WLAN und DHCP (◐) |
| `:327` | `ntp_setup()` ⇒ `ntp_udp.begin()` (`ntp.cpp:20`, `:125`) | UDP-Kontext und UDP-PCB (◐ FA) |
| `:328` | `udp_server_setup()` ⇒ `server_udp.begin()` (`udpsrv.cpp:45`, `:56`) | UDP-Kontext und UDP-PCB (◐ FA) |
| `:329-331` | **Startzeile** `FIRMWARE …` mit `Serial.flush()` | — |
| `:332-335` | `vars_init()`, `stm32_flash_init()`, `display_layout_init()`, `tables_init()` | keine bekannt; `stm32_flash_init()`, `display_layout_init()` und `tables_init()` hängen LittleFS je ein und wieder aus (`stm32flash.cpp:1403/1411`, `display.cpp:82/123`, `tables.cpp:693/743`) — vorübergehend |
| danach | `loop()` — erst hier holt `http_server_loop()` eine Verbindung ab (`http.cpp:17376`) | — |

Lwip arbeitet eingehende Segmente nur ab, wenn der Code die CPU abgibt. Das geschieht in `delay()` und in
`Serial.flush()`: `uart_wait_tx_empty()` ruft `esp_yield()`, solange der Sendepuffer nicht leer ist (`uart.cpp:564-571`,
✔ SW). Der Analyst nennt dafür `base.cpp:339` — dort steht nur der Aufruf; die Abgabe selbst geschieht im Core.

### 2.2 Der Einwand „die UDP-Kontexte stehen schon“

Der Auftrag fragt, ob K2 in der beschriebenen Form zum Versuch passt, wenn die Anfrage erst 0,2 s **nach** der
Startzeile kommt — zu einem Zeitpunkt, an dem `ntp_setup()` und `udp_server_setup()` schon gelaufen sind.

**Ergebnis: Der Einwand trifft die Zeile, aber nicht den Verbindungsaufbau.**

1. `- heap erste anfrage` entsteht in `http_accept_note()` (`http.cpp:779-794`), aufgerufen in `http_server_loop()`
   **nach** `http_server.accept()` (`http.cpp:17376-17386`) — also beim **Abholen** in `loop()`. ✔ SW
2. Den `ClientContext` legt der Core schon beim **Aufbau** an, im lwIP-Rückruf `WiFiServer::_accept()`, und hängt ihn an
   `_unclaimed` (`WiFiServer.cpp:187-202`, Core 3.1.2). Das geht, sobald der Server lauscht — ab `http_server_begin()`
   (`wifi.cpp:93`) — und sobald lwIP laufen darf (§2.1). ✔ SW
3. Arm A fragt ab dem Augenblick, in dem die alte Firmware nicht mehr antwortet, **alle 0,3 s mit 1 s Frist** an
   (`vier-arme.py`, Schleife „sofort“; ✔ SW am Werkzeug). Zwischen `http_server_begin()` und dem Ende von `setup()` sind
   eingehende Aufbauten deshalb **zu erwarten** — belegt sind sie nicht.
4. Dass die erste Abholung in allen drei schlechten Läufen 0,2–0,3 s nach der Startzeile lag, verträgt sich mit beidem:
   mit einer Verbindung, die schon während `setup()` wartete und beim ersten `loop()`-Durchlauf abgeholt wurde, und mit
   einer, die erst nach `setup()` aufgebaut wurde.

**Folge:** K2 bleibt Kandidat. Prüfbar ist er nur mit dem Zeitpunkt des **Aufbaus** — `_unclaimed` je Schritt (M2) und
die Felder `http_accept_dead`/`tcp_now_unclaimed` in der ersten Antwort (Vorprobe, AKV.1) — und mit einem Arm, der vor der
Startzeile nichts sendet (Arm E).

**Nachtrag N1 (17.10.2026) — was die Vorprobe V.2 an §2.2 ändert (✔ L, L500):**

1. **Zu Punkt 3:** Die erwarteten Aufbauten während `setup()` haben sich in der Form, die K2 braucht, nicht gezeigt. In
   der ersten Antwort von Arm A waren jedes Mal `http_accept_dead`, `tcp_now_pending` und `tcp_now_unclaimed` 0, in
   `tcp_now_list` nur die eigene Verbindung — beim ersten Abholen wartete keine weitere Verbindung.
2. **Arm E** öffnet bis zur Startzeile keine Verbindung (vom Werkzeug im Mitschnitt geprüft) und ist trotzdem
   zerstückelt: 6'600 · 6'600 · 6'432.
3. **Folge:** K2 in der Form des Analysten (L499) — eine während `setup()` aufgebaute Verbindung, über die sich die
   UDP-Kontexte legen — ist **in strenger Form widerlegt**: Der Schaden entsteht auch ohne sie. Der Satz „K2 bleibt
   Kandidat“ oben ist **überholt**. Folgerung des Leads (L500): Der Schaden entsteht durch eine Anfrage in den ersten
   Sekunden **nach** dem Ende von `setup()`.
4. **Zum Ende von `setup()` (◐, `spec-writer`):** In Arm A stand `- heap erste anfrage` 0,3–0,6 s nach der Startzeile im
   Mitschnitt; die Zeile entsteht in `loop()` (Punkt 1). `setup()` war in diesen Läufen also spätestens 0,3–0,6 s nach der
   Startzeile beendet — gemessen an den Zeitstempeln des Mitschnitts, nicht am Gerät. Bei vergleichbarer Dauer von
   `setup()` kam die Anfrage in Arm E (1,9–3,2 s) nach dessen Ende. Gemessen wird das erst mit `ms` bei s8 (Q4).
5. **Was im Fenster nach `setup()` belegt wird** — dazu gehört die Beobachtung, dass der Haufen in Arm B bis +30 s auch
   ohne Anfrage fällt (§1 N1) — **und welche Lesart das stützt, liefert V.3** **[offen: V.3]** (§3.5).

### 2.3 Drei Lesarten

| Lesart | Inhalt | Was dafür spräche | Was dagegen spräche |
|---|---|---|---|
| **L-a** | Aufbau während `setup()` **vor** `ntp_setup()`/`udp_server_setup()`; die UDP-Kontexte legen sich über die Verbindungsobjekte (K2 des Analysten) | Arm E sauber; in Arm A `_unclaimed` ≥ 1 bei s2–s4; Trenner ist ein UDP-PCB | Arm E zerstückelt |
| **L-a′** | Aufbau während `setup()` **nach** der Startzeile; eine Belegung aus `vars_init()` … `tables_init()` legt sich darüber | `_unclaimed` ≥ 1 erst bei s4–s8; Trenner entsteht zwischen s4 und s8 | Trenner liegt schon bei s4 |
| **L-b** | Belegung **nach** `setup()`, in den ersten Sekunden des Betriebs, während die frühe Verbindung lebt | Trenner erst bei `F`, `U` oder in einer `m`-Karte sichtbar; Arm E zerstückelt, obwohl seine Anfrage nach dem Ende von `setup()` kommt | Trenner liegt schon bei `S` |

**Hinweis aus M0, ◐:** Ein schlechter Start mit erster Anfrage nach 5,9 s passt schlecht zu L-a, sofern `setup()` dann
schon beendet war. Wann `setup()` relativ zur Startzeile endet, ist nicht gemessen — M2 liefert es mit `ms` bei s8. Wie
die Werkzeuge in M0 angefragt haben, steht im Protokoll nicht; der Fall ist ein Hinweis, kein Gegenbeleg.

**Nachtrag N1 (L500):**

- **L-a:** Die Spalte „Was dagegen spräche“ ist eingetreten — Arm E ist zerstückelt. **L-a trägt nicht mehr** (✔ L).
- **L-a′:** setzt einen Aufbau während `setup()` voraus. In Arm E gab es keinen, und in Arm A wartete beim ersten Abholen
  keine Verbindung. Als Erklärung für Arm E **trägt L-a′ nicht** (◐ — hängt am Ende von `setup()` nach §2.2 N1 Punkt 4).
- **L-b:** Die Spalte „Was dafür spräche“ ist zur Hälfte eingetreten (Arm E zerstückelt, Anfrage nach dem Ende von
  `setup()`, ◐). Welche Belegung es ist, ist offen.
- Ob L-b in dieser Form genügt oder V.2 eine Lesart verlangt, die die Tabelle nicht kennt, entscheidet V.3
  **[offen: V.3]**.

### 2.4 Kandidaten

| | Kandidat | Herkunft | Einschätzung |
|---|---|---|---|
| K1 | Kontext des Wetter-Clients bleibt bis zum nächsten Abruf (`weather.cpp:16`, `WiFiClient openweather_client`; C77) | ◐ FA | Läuft in den Armen A und B gleich; kann den Unterschied nur im Zusammenspiel mit der frühen Verbindung erklären. Laut L499 lag vor der ersten Anfrage der schlechten Starts kein Wetterabruf |
| K2 | Früher Aufbau während `setup()` unter den UDP-Kontexten | ◐ FA | = L-a; §2.2. **N1: in strenger Form widerlegt (L500, §2.2 N1)** |
| K3 | Spät aufgebautes WLAN | ? FA | In A und B gleich; WLAN steht zwingend vor der ersten Antwort |
| K4 | Interne Belegungen von SDK/lwIP | ? FA | Zusammengefasst mit K6 |
| K5 | Legacy `/network` ohne `scanDelete()` | ? FA | In keinem Arm aufgerufen |
| K6 | Einmalige Belegungen der ersten Sekunden — Schritte nach der Startzeile, erste Antworten des STM, erster NTP-Abgleich, SDK/lwIP (DHCP, ARP) —, die sich über eine gerade lebende frühe Verbindung legen | ◐ spec-writer | = L-a′ bzw. L-b; welche Belegungen das sind, ist nicht bekannt. **N1: der Teil „nach `setup()`“ (= L-b) bleibt; dass im Fenster bis +30 s im Hintergrund belegt wird, ist in Arm B gemessen (L500). Welche Belegungen, offen [offen: V.3]** |

**Statisch ausgeschlossen als Quelle einer Belegung (✔ SW):** das Nachführen des Backlogs — `tcp_backlog_set()` ändert
nur ein Feld des Listen-PCB (`http.cpp:42`, `:63`, `:466-476`); `String` auf Dateiebene (§1). **Kein Kandidat für sich,
aber Mitspieler:** `http_client` ist global (`http.cpp:100`) und hält den `ClientContext` der zuletzt bedienten
Verbindung bis zur nächsten Zuweisung in `http_server_loop()` (`:17376`); die Freigabe-Reihenfolge der Verbindungsobjekte
ist damit an den `loop()`-Takt gebunden.

**Logik der Arme (◐):** Alles, was in Arm A und Arm B gleich abläuft, kann den Unterschied nur im Zusammenspiel mit der
frühen Verbindung erklären. Da `free` gleich bleibt, ist die Menge der dauerhaften Belegungen in beiden Armen dieselbe;
gesucht ist die Belegung, die in Arm A **an anderer Stelle** liegt.

---

## 3. Stufe V — Vorprobe ohne Build

### 3.1 Zweck

Die Vorprobe kostet keinen Build und trennt L-a von L-a′/L-b, bevor das Instrument gebaut ist. Sie ersetzt Stufe 2 nicht:
Sie sagt *wann*, nicht *was*.

### 3.2 Werkzeug (Lead, `tools/**`)

Grundlage ist `vier-arme.py` aus L499. Ergänzt um:

- **Arm E:** Neustart wie Arm A/B; das Werkzeug liest bis zur Startzeile **nur den Mitschnitt** und öffnet keine
  Verbindung zur Uhr; sobald die Startzeile da ist, **eine** `device_ready`-Anfrage. Das Werkzeug liest den Mitschnitt
  heute alle 2 s; der Abstand der Anfrage zur Startzeile wird damit rund 1–3 s betragen und liegt im schlechten Fenster
  aus M0 (bis 5,9 s). Über 5,9 s ⇒ Lauf ungültig (AKV.2).
- **Mitschrift der ersten Antwort** in den Armen A und D: der Rumpf, daraus `http_accept_dead`, `tcp_now_pending`,
  `tcp_now_unclaimed`, `tcp_now_list`; dazu die Zahl der Abfrageversuche vor der ersten Antwort.
- **Exit-Codes und `--abnahme`** nach AKV.1. Heute gibt `ruhe()` bei fehlendem Ruhewert nur den Text „kein Ruhewert“
  zurück, und das Werkzeug endet trotzdem mit 0 — genau die Gattung aus DIR-014.

**N1:** Umgesetzt als `tools/startprobe.py` (Commit `f012309`). Stand der Gegenproben: `requirements.md` AKV.1 N1.

### 3.3 Reihenfolge

Neun Läufe, jeder Arm in jeder Dreiergruppe einmal und an jeder Stelle einmal: **A B E · E A B · B E A**. So fällt ein
Drift über die Zeit (Temperatur, WLAN, Server) nicht auf einen Arm.

### 3.4 Was die Vorprobe entscheidet

| Ergebnis | Lesart gestützt | Folge für Stufe 1 |
|---|---|---|
| E sauber wie B; in A `http_accept_dead` > 0 oder `tcp_now_unclaimed` > 0 in der ersten Antwort | L-a oder L-a′ | M2 wie geplant; Schwerpunkt s2–s8 |
| E zerstückelt wie A | L-a′ oder L-b | M2 wie geplant; Schwerpunkt s4–s8, `S`, `F`, `U` |
| gemischt | keine | M2 wie geplant; Befund ins Messprotokoll |

In keinem Fall ändert die Vorprobe den Umfang von Stufe 1; sie legt fest, worauf die Auswertung in Stufe 2 schaut.

**Nachtrag N1 — Ergebnis (✔ L, L500):** Eingetreten ist die **zweite Zeile: E zerstückelt wie A** (E 6'600 · 6'600 ·
6'432, A 6'600 × 3, B 7'104 × 3), und in der ersten Antwort von Arm A waren `http_accept_dead` und `tcp_now_unclaimed`
je 0. **Was an dieser Tabelle nicht mehr trägt:**

- **„L-a′ oder L-b“** — L-a′ trägt für Arm E nicht (§2.3 N1); gestützt ist L-b.
- **„Schwerpunkt s4–s8“** — Das Raster s0–s8 liegt **ganz in `setup()`**; nach L500 entsteht der Schaden **nach**
  `setup()`. **s0–s8 allein treffen den Schaden nicht.** Sie bleiben als Grundlinie nützlich (Lage vor der Anfrage, `ms`
  bei s8 für Q4), sind aber nicht mehr der Schwerpunkt. Von den Ereignissen liegt `S` vor dem Schaden; `F` liegt nach der
  ersten Anfrage; `U` und die `m`-Karten danach.
- **„In keinem Fall ändert die Vorprobe den Umfang von Stufe 1“** — Dieser Satz steht ab N1 **unter Vorbehalt**. Ob
  Stufe 1 Erfassungspunkte im Fenster nach `setup()` braucht und ob s0–s8 in dieser Form bleiben, legt ein Nachtrag nach
  V.3 fest (Task V.4), **bevor** Task 1.3 beginnt **[offen: V.3]**.

### 3.5 Lesart der Vorprobe (V.3) — **[offen: V.3]**

*Platzhalter, N1.* Hier wird die Lesart des `firmware-analyst` zu L500 eingetragen, sobald sie vorliegt: welche Lesart
gestützt ist, was im Fenster nach `setup()` belegt wird (auch die Hintergrundbelegung bis +30 s in Arm B), welche Annahme
aus §2 nicht trägt, Hinweise für Stufe 2 und — falls nötig — Änderungen am Umfang von Stufe 1 (dann als Nachtrag N2,
Task V.4). Bis dahin gilt §3.4 mit dem Vorbehalt aus N1. Der `spec-writer` greift ihr nicht vor.

---

## 4. Stufe 1 — Instrument (Release I)

### 4.1 Grundsätze

- **Das Instrument belegt nichts auf dem Haufen.** Sonst misst es sich selbst — dieselbe Regel wie für die Heap-Zeile
  (`ESP-uclock.ino:584-587`). Alle Werte stehen in festen Feldern im BSS.
- **Erfassen und Ausgeben sind getrennt.** Erfasst wird im Augenblick des Schritts oder Ereignisses; ausgegeben wird
  später aus `loop()`, **eine Zeile je Sekunde**, nie zwei im selben Durchlauf, nie im selben Durchlauf wie die
  Heap-Zeile. Grund: die Grenzen der Brücke (Zeile ≤ 119 Zeichen; laut Analyst ein Antwortpuffer des STM von 256 Byte,
  ●; Regel „nie mehr als eine Zeile am Stück“) und die Startphase, in der der STM die Folge CAP/FIRMWARE auswertet
  (`ESP-uclock.ino:266-283`). **In `setup()` kommt keine neue Ausgabe dazu.**
- **Einmal formatiert, zweimal ausgegeben** — Mitschnitt und `/api/stm32_log` (L185, `ESP-uclock.ino:570-574`).
- **Präfix `- `**: Der STM behandelt solche Zeilen als Meldung des ESP (`ESP-uclock.ino:564-565`); es entsteht **kein
  STM-Kommando**.
- **Befristet**, hinter `ESP_HEAP_MAP` (§4.7).

### 4.2 M1 — Heap-Karte

**Zeile** (Anlass `m` = nach jeder Heap-Zeile; `S`, `F`, `U` nach §4.4):

```
- heap karte <anlass> ent=<belegt>/<frei> fsum=<byte> gross=<blk>+<byte> zweit=<blk>+<byte> davor=<blk>+<byte> danach=<blk>+<byte>
```

| Feld | Bedeutung |
|---|---|
| `ent=<belegt>/<frei>` | Zahl der belegten bzw. freien Einträge der Blockkette |
| `fsum` | Summe der Längen aller freien Einträge in Byte — Gegenwert zu `free=` (AKI.3) |
| `gross` | grösster freier Eintrag: erster Block und Länge in Byte — Gegenwert zu `max=` (AKI.3) |
| `zweit` | zweitgrösster freier Eintrag, gleiche Form — hier liegt in Arm A vermutlich der Rest des Fehlbetrags |
| `davor`, `danach` | der belegte Eintrag unmittelbar vor bzw. nach `gross`, erster Block und Länge; `-` am Rand des Haufens |

Bei Gleichstand gilt der Eintrag mit dem kleineren Blockindex. Längste Form mit fünfstelligen Werten 111 Zeichen, unter
119. Freie Nachbarn eines freien Eintrags gibt es nicht, weil umm benachbarte freie Einträge zusammenlegt (◐ FA, im
Bericht 1.1 mit Fundstelle zu belegen).

**Blockgang.** Eine reine Funktion `(Blockfeld, Blockzahl) ⇒ Ergebnis`, damit der Prüfstand sie mit synthetischen Haufen
füttern kann. Sie läuft am Gerät unter derselben Art Sperre wie `umm_info()` (Mittel bestimmt der `esp-developer` im
Bericht 1.1). **Wächter**, je mit einem Grund statt Werten (`abbruch=<grund>`):

| Grund | Bedingung |
|---|---|
| `bs` | Blockgrösse nicht 8 (`umm_block_size()`, `umm_local.c:168`) |
| `nb` | ein Folgeindex ≥ Blockzahl |
| `ord` | ein Folgeindex nicht grösser als der aktuelle (Zyklus oder Beschädigung) |
| `it` | mehr Schritte als Blöcke |
| `sum` (**N1**) | Summe der Längen aller Einträge, belegt und frei, in Blöcken ≠ Blockzahl − 2 (Kopf der Freiliste und Endwächter nicht mitgezählt) |

Die Sperre wird auf **jedem** Weg gelöst, auch bei `abbruch=`.

**Zugriff auf Blockfeld und Blockzahl:** nach Ent-C1 (§4.9). Der Kontext des Haufens (`umm_heap_context_t`,
`umm_malloc_cfg.h:156`) ist erst in `umm_local.h` ausgeführt (Felder `heap` und `numblocks`, `umm_local.h:65`, `:73`,
✔ SW); die Form der Blöcke steht in `umm_malloc.cpp` (◐ FA, im Bericht 1.1 mit Fundstelle zu belegen).

**Nachtrag N1 — Ergebnisse des Berichts 1.1 (Ent-C1 = A):**

- **Haufen:** genau einer (DRAM), `UMM_NUM_HEAPS == 1`; 1'590 Blöcke zu 8 Byte, `_heap_start` 0x3FFF8E50 bis 0x3FFFC000,
  12'720 Byte (✔ ED). Kontext `umm_heap_context_t` 52 Byte, `heap` am Anfang, `numblocks` an Versatz 48
  (`umm_local.h:64-75`, ✔ ED; Aufbau der Struktur ✔ SW).
- **Blockform** — steht nur in `umm_malloc.cpp:104-120` und ist deshalb nachzubilden (✔ ED, ✔ SW): je Block ein Kopf
  `next`/`prev` zu je 16 Bit, danach 4 Byte Nutzlast bzw. die Verkettung der Freiliste. In `next` ist **Bit 15 das
  Frei-Bit** (`UMM_FREELIST_MASK` 0x8000), **Bits 0–14 der Index** des Folgeblocks (`UMM_BLOCKNO_MASK` 0x7FFF). Block 0
  ist der Kopf der Freiliste, der letzte Block der **Endwächter mit `next == 0`**. Ein Eintrag mit n Blöcken fasst
  8n − 4 Byte Nutzlast. Zeiger → Block über `(ptr − heap) / 8` (`umm_malloc.cpp:629`, ✔ SW).
- **Freie Nachbarn:** gibt es nicht — `umm_free()` legt in beide Richtungen zusammen (`umm_malloc.cpp:638-646`, ✔ ED,
  ✔ SW). Das **◐ FA oben ist damit belegt.**
- **Gegenwerte (✔ ED):** `max=` = grösster freier Eintrag in Blöcken × 8; `free=` = `stats.free_blocks` × 8. Daraus folgt:
  Gibt die Karte ihre Längen in **Blöcken × 8** aus und wird sie im **selben Fenster ohne CPU-Abgabe** erfasst wie die
  Werte der Heap-Zeile — in `esp_heap_log()` vor `Serial.println`/`flush` (`ESP-uclock.ino:658-678`) —, gilt
  **`gross` = `max` und `fsum` = `free` exakt** (AKI.3 N1). Alle Längen der Zeile (`fsum`, `gross`, `zweit`, `davor`,
  `danach`) stehen deshalb in Blöcken × 8, nicht als 8n − 4. Die Angabe „in Byte“ oben meint ab N1 genau das.
- **Selbstprüfung `sum`** (neue Zeile in der Wächtertabelle): deckt einen falsch nachgebildeten Gang auch dort auf, wo
  `gross`/`fsum` zufällig stimmen.
- **Sperre:** wie `umm_info()` — `xt_rsil(15)` zum Sperren, `xt_wsr_ps` zum Freigeben (✔ ED). **Aufbau:** eine reine
  Gangfunktion über `(const void*, uint16_t)` ohne Sperre; ein Wrapper sperrt, ruft sie auf und gibt **ohne Verzweigung**
  frei. **Kein RAII** — sonst liesse sich die Sabotage „Sperre auf dem `abbruch`-Weg nicht gelöst“ (§4.8) nicht mehr
  herstellen, und ihre Gegenprobe wäre leer.
- **Gangdauer:** geschätzt 70–200 µs, theoretisch bis rund 1 ms (◐ ED). Gemessen wird `us=` (AKI.9).
- **Längste Form** mit Blockindizes ≤ 1'589 und Längen ≤ 12'720: rund 106 Zeichen (◐ spec-writer gerechnet), unter 119.
- **Absicherung der Übersetzung:** §4.7 N1.

### 4.3 M2 — Startschritte

| Schritt | erfasst unmittelbar nach | Zeile |
|---|---|---|
| s0 | `- flash id` (vor `EEPROM.begin`) — Grundzustand | `:302` |
| s1 | `eeprom_read()` | `:316` |
| s2 | `wifi_connect()` bzw. `wifi_ap()` — der Server lauscht | `:325` |
| s3 | `ntp_setup()` | `:327` |
| s4 | `udp_server_setup()` — Stand bei der Startzeile | `:328` |
| s5 | `vars_init()` | `:332` |
| s6 | `stm32_flash_init()` | `:333` |
| s7 | `display_layout_init()` | `:334` |
| s8 | `tables_init()` — Ende von `setup()` | `:335` |

Der Analyst hatte s5–s7 nicht vorgesehen; sie kommen dazu, weil L-a′ genau dort liegt (§2.3).

**Werte je Schritt:** `ms` (`millis()`), `free`, `gross` (Beginn und Länge), belegte Einträge, `u` = Einträge in
`_unclaimed` des Servers (vor s2 per Definition 0; Zugriff über die eigene Unterklasse `WcHttpServer`, `http.cpp:53` —
Erreichbarkeit im Bericht 1.1), `p` = TCP-PCBs ausserhalb LISTEN und TIME_WAIT (wie `tcp_now`).

**Ausgabe:** nach `setup()` eine Zeile je Schritt, je Sekunde eine:

```
- heap start s<n> ms=<n> free=<n> gross=<blk>+<byte> ent=<n> u=<n> p=<n>        (s8 zusätzlich: us=<n>)
```

`us` ist die Dauer des Blockgangs bei s8 in Mikrosekunden (AKI.9). Längste Form rund 85 Zeichen.

**Nachtrag N1:**

- **s2 heisst nicht immer „der Server lauscht“** (✔ ED): Scheitert `wifi_connect()`, startet der Server später aus
  `loop()` (`wifi.cpp:91-108`). Jede `start`-Zeile trägt deshalb den Merker **`srv=<0|1>`**:

  ```
  - heap start s<n> ms=<n> free=<n> gross=<blk>+<byte> ent=<n> u=<n> p=<n> srv=<0|1>        (s8 zusätzlich: us=<n>)
  ```

  Längste Form rund 92 Zeichen.
- **`u`:** `_unclaimed` ist über `WcHttpServer` allein nicht erreichbar; es braucht eine **neue exportierte Funktion in
  `http.cpp`**, die die Zahl der Einträge zurückgibt (✔ ED).
- **Takt der Ausgabe:** Die erste Heap-Zeile kommt im ersten `loop()`-Durchlauf (✔ ED); die `start`-Zeilen erscheinen
  frühestens ab dem zweiten (AKI.1 (e)).
- **Vorbehalt aus V.2 (§3.4 N1):** s0–s8 liegen ganz in `setup()`, der Schaden entsteht danach (L500). M2 bleibt bis zum
  Nachtrag nach V.3 im bisherigen Umfang; der Satz oben „weil L-a′ genau dort liegt“ trägt nach §2.3 N1 nicht mehr als
  Begründung für s5–s7 — sie liefern weiterhin die Grundlinie vor der ersten Anfrage. **[offen: V.3]**

### 4.4 M3 — Ereigniskarten

| Anlass | erfasst | warum |
|---|---|---|
| `S` | am Ende von `setup()`, im selben Augenblick wie s8 (ein Blockgang für beide) | Lage vor dem ersten `loop()` — trennt L-a/L-a′ von L-b |
| `F` | nach der Rückkehr aus dem `http_server_loop()`-Durchlauf, in dem die erste Anfrage angenommen wurde | Lage, solange die frühe Verbindung noch Spuren hat (`http_client` hält den Kontext bis zum nächsten `accept()`, §2.4) |
| `U` | nach der Zeile `- update cache nach` | Lage nach dem ersten Abgleich (N6: `frag=10` statt 4 beim Abgleich, L493) |

Je Start genau einmal; ein Ereignis, das nicht eintritt, hinterlässt keine Zeile (AKI.5).

**Nachtrag N1:** `F` braucht einen Merker aus `http.cpp` — `http_first_request_logged` ist dort `static` (`http.cpp:680`,
✔ SW, ✔ ED); er wird über eine neue exportierte Funktion in `http.cpp` lesbar. Nach L500 liegt der Schaden im Fenster um
die erste Anfrage; `S` zeigt die Lage davor, `F` die danach. Ob eine weitere Karte im Fenster zwischen `S` und der ersten
Anfrage gebraucht wird, entscheidet der Nachtrag nach V.3 **[offen: V.3]**.

### 4.5 M4 — Wegmarken

Bei `S` und bei `F` je eine Zeile mit dem Blockindex bekannter Objekte, in derselben Einheit wie die Karte:

```
- heap marken <anlass> lst=<blk> udp=<blk>,<blk>,… tcp=<blk>:<zustand>,… dhcp=<blk>
```

| Feld | Objekt | Erreichbarkeit |
|---|---|---|
| `lst` | Listen-PCB des Servers an Port 80 | ● über die lwIP-Listen, die das Projekt für `tcp_now` schon liest; im Bericht 1.1 zu bestätigen |
| `udp` | jedes UDP-PCB (NTP, UDP-Server, gegebenenfalls interne) | ● über die Liste der UDP-PCBs von lwIP; zu bestätigen |
| `tcp` | jedes aktive TCP-PCB mit Zustand, **ohne Gegenstelle** | ● wie `tcp_now`; zu bestätigen |
| `dhcp` | DHCP-Daten der Station | ● über das Standard-netif; zu bestätigen |

Die **UDP-Kontexte** (`UdpContext` in `WiFiUDP`) und die **`ClientContext`-Objekte** sind private Felder des Cores und
ohne Eingriff nicht erreichbar (◐). Ihr PCB gilt als Näherung für ihre Lage (◐ — beide entstehen im selben Aufruf); das
steht so im Bericht und in der Auswertung, nicht als Gleichsetzung.

Höchstens acht Einträge je Zeile; was darüber hinausgeht, zählt `+<n>` am Ende. Längste Form ≤ 119.

**Nachtrag N1 — Korrektur nach dem Bericht 1.1 (✔ ED):** Der Absatz oben über die Kontexte ist **falsch** und überholt:
`UdpContext` und `ClientContext` sind **genau** erreichbar, ohne Eingriff in den Core. Die Näherung „PCB für seinen
Kontext“ entfällt. Die vier ● der Tabelle oben sind bestätigt. Liste ab N1:

| Feld | Objekt | Weg (✔ ED) | Grösse Byte / Blöcke (✔ ED) |
|---|---|---|---|
| `lst` | Listen-PCB des Servers an Port 80 | `tcp_listen_pcbs` oder `WcHttpServer::_listen_pcb` | 36 / 5 |
| `udp` | jedes UDP-PCB **und sein `UdpContext`** | Liste `udp_pcbs`; Kontext genau über `pcb->recv_arg` | PCB 40 / 6; Kontext 76 / 10 |
| `tcp` | jedes TCP-PCB mit Zustand, **ohne Gegenstelle**, **und sein `ClientContext`** | Listen wie `tcp_now`; Kontext genau über `pcb->callback_arg` | PCB 184 / 24; Kontext 52 / 7 |
| `wet` | `ClientContext` des Wetter-Clients (`weather.cpp:16`) | Zugriffsglied in einer Unterklasse von `WiFiClient` (Zeiger auf Glied); **`weather.cpp` bleibt unverändert** | 52 / 7 |
| `dhcp` | DHCP-Daten der Station | `netif_dhcp_data` | 52 / 7 |
| `eep` | Puffer von `EEPROM` | **`getConstDataPtr()`** — nicht `getDataPtr()`, das `_dirty` setzt und damit den Zustand von `EEPROM` ändert | im Bericht nicht genannt |
| — | Kopf eines `pbuf` | keine Wegmarke; Grösse als Hilfe bei der Zuordnung über die Länge (§5.4) | 16 / 3 |

**Zeile ab N1** (◐ spec-writer, Form; der `esp-developer` setzt sie so um oder meldet, warum nicht):

```
- heap marken <anlass> lst=<blk> wet=<blk> dhcp=<blk> eep=<blk> udp=<pcb>/<ctx>,… tcp=<pcb>:<zustand>/<ctx>,…
```

- Gibt es ein Objekt nicht (etwa keinen Wetter-Kontext oder ein UDP-PCB ohne `UdpContext`), steht `-` an seiner Stelle.
- Die festen Felder kommen zuerst; danach die Listen `udp` und `tcp`, bis die Zeile 119 Zeichen erreicht. Was nicht mehr
  passt, zählt `+<n>` am Ende. Die Regel „höchstens acht Einträge“ oben ist damit durch die Längengrenze ersetzt.
- Rechnung (◐): feste Felder und je zwei Einträge `udp` und `tcp` mit vierstelligen Indizes rund 106 Zeichen; ein
  dritter `tcp`-Eintrag passt knapp, ein vierter nicht.
- Für TCP-PCBs weiterhin nur Index und Zustand, keine Gegenstelle (AKI.1 (g)).

### 4.6 Kosten (gerechnet, ◐)

| Posten | Byte BSS | Brücke |
|---|---|---|
| M2: 9 Schritte × rund 14 Byte | rund 126 | 9 Zeilen je Start, rund 85 Zeichen |
| M3 und M1: 4 Karten (`S`, `F`, `U`, `m`) × rund 22 Byte | rund 88 | 3 Zeilen je Start; je Heap-Zeile eine, rund 100 Zeichen |
| M4: 2 Ereignisse × 8 Einträge × 2 Byte, dazu Zähler | rund 36 | 2 Zeilen je Start |
| Merker, Takt der Ausgabe | rund 8 | — |
| **zusammen** | **rund 258** | **im Betrieb rund 100 Byte je Minute (1,7 Byte/s); je Start rund 1,2 KB über mindestens 14 s** |

Jedes Byte BSS fehlt dem Haufen (◐): `max` in Arm B wird um rund ΔRAM(I) tiefer liegen. Die Schwellen sind deshalb relativ
formuliert (AKM.1, AKA.3, AKA.4). **Ent-C2:** Grenze ≤ 320 Byte, darüber anhalten und vorlegen.

Ein Blockgang läuft bis zur Blockzahl, mit gesperrten Unterbrechungen; die Dauer wird gemessen (`us=`), nicht geschätzt.
Über 1'000 µs ⇒ vorlegen (◐ Vorschlag). Ein Gang je Minute im Betrieb, elf je Start.

**Nachtrag N1 (Bericht 1.1, ◐ ED gerechnet):** BSS **rund 290 Byte** statt 258 — alle Felder in einer Struktur gefasst
(Ausrichtung), die TCP-Wegmarke mit 3 Byte statt 2 (Index und Zustand). Das liegt **unter Ent-C2 (≤ 320, bestätigt), aber
knapp**: 30 Byte Luft. **Programm** 1,5–2,5 KB. **Gangdauer** geschätzt 70–200 µs, theoretisch bis rund 1 ms — die
Schwelle 1'000 µs bleibt. **Zu prüfen in Task 1.3:** Die Zeilenform aus §4.5 N1 (Kontext je PCB, `wet`, `eep`) kann mehr
Felder brauchen, als die Rechnung des Berichts enthält; der `esp-developer` rechnet BSS für die endgültige Form nach.
Ebenso, wenn der Nachtrag nach V.3 den Umfang ändert. In beiden Fällen gilt: über 320 ⇒ anhalten und vorlegen, **bevor**
gebaut wird (AKI.9).

### 4.7 Schalter und Rückbau

`#define ESP_HEAP_MAP 1` neben `ESP_HEAP_LOG` (`ESP-uclock.ino:618`). Mit 0 entfällt alles, auch das BSS. Ein
Übersetzungsfehler, wenn `ESP_HEAP_MAP` 1 und `ESP_HEAP_LOG` 0 ist. Bei Ent-C1 = A eine Übersetzungsprüfung auf Core
3.1.2.

**Rückbau (Ent-C4):** Release A trägt das Instrument noch — AKA.5 braucht die Karte. Im nächsten ESP-Release nach der
Abnahme von Stufe 3 gehen M2–M4 heraus; M1 geht heraus oder steht auf 0. Der Kommentarblock nennt das wörtlich, nach dem
Muster von `ESP_HEAP_LOG` (`:579-582`: „WEG DAMIT, sobald …“). `BEFUNDE.md` führt den Rückbau ab Release I als offenen
Punkt (AKZ.7) — eine Aufgabe ohne Eintrag erinnert niemanden (CLAUDE.md zu E4).

**Nachtrag N1 — Mittel der Bindung an Core 3.1.2 (Bericht 1.1):**

- Blockgang, Karte und Wegmarken in einer **eigenen Übersetzungseinheit** (z. B. `heapkarte.cpp` / `.h`, CRLF wie die
  übrigen Moduldateien, §4.10).
- `umm_local.h` als **letzter** Include, danach `#undef memcpy`, `#undef memmove`, `#undef memset` (so im Bericht 1.1).
- `#error`, wenn `ARDUINO_ESP8266_RELEASE_3_1_2` nicht definiert ist.
- `static_assert(sizeof(umm_heap_context_t) == 52)`, `static_assert(offsetof(umm_heap_context_t, numblocks) == 48)`,
  Prüfung `UMM_NUM_HEAPS == 1` zur Übersetzungszeit.
- Blockgrösse 8 zur Laufzeit (Wächter `bs`, §4.2).

Bestätigt am 17.10.2026 (Ent-C4): Der Rückbau bleibt im nächsten ESP-Release nach der Abnahme von Stufe 3.

### 4.8 Prüfstand c80i

Der `esp-developer` baut ihn im Scratchpad, der Lead legt ihn unter `tools/checks/auszug/esp/c80i/` ab und fährt die
Gegenproben (Werkzeugvorbehalt wie im Paket 2026-10-15).

**Fälle (Auswahl, mindestens):**

1. ein einziger freier Eintrag (frischer Haufen);
2. zwei freie Einträge, getrennt durch einen belegten — `gross`, `zweit`, `davor`, `danach` exakt;
3. grösster freier Eintrag am Ende (`danach=-`) und am Anfang (`davor=-`);
4. Gleichstand zweier grösster Einträge — der mit dem kleineren Index;
5. das Bild aus L499 nachgestellt: grosser Block 7'104, eine Belegung verschoben ⇒ `gross` 6'600, Rest in `zweit`;
6. je ein beschädigter Haufen für `bs`, `nb`, `ord`, `it` ⇒ `abbruch=<grund>`, Sperre gelöst;
7. längste Form jeder Zeilenart ≤ 119;
8. statisch: kein `String`, `malloc`, `new` im neuen Code; beide Ausgabewege mit demselben Puffer; keine Ausgabe in
   `setup()`; höchstens eine Zeile je Durchlauf; `ESP_HEAP_MAP` 0 ⇒ keine Zeile.

**Sabotagen, jede ⇒ FEHL:** nur `Serial`; nur Ring; Schranke Blockzahl entfernt; Zeile über 119; `String` statt
`snprintf`; Ausgabe in `setup()`; zwei Zeilen im selben Durchlauf; `davor`/`danach` vertauscht; Sperre auf dem
`abbruch`-Weg nicht gelöst; Gleichstand zugunsten des grösseren Index. **Gegen 3.2.41 ⇒ FEHL.**

**Nachtrag N1 — zusätzliche Fälle:**

9. Blockform nach §4.2 N1 an synthetischen Haufen: Frei-Bit in Bit 15, Endwächter mit `next == 0`, Block 0 als Kopf; Zeiger
   → Block über `(ptr − heap) / 8`;
10. Gleichheit an synthetischen Haufen mit bekannter Belegung: Länge `gross` = grösster freier Eintrag × 8, `fsum` =
    Summe der freien Blöcke × 8 (AKI.3 N1);
11. beschädigter Haufen mit falscher Summe ⇒ `abbruch=sum`, Sperre gelöst;
12. statisch: `#error` auf `ARDUINO_ESP8266_RELEASE_3_1_2`, die beiden `static_assert`, die Prüfung `UMM_NUM_HEAPS == 1`,
    `umm_local.h` als letzter Include mit den drei `#undef`; `EEPROM` nur über `getConstDataPtr()`; Sperre ohne
    Verzweigung und ohne RAII;
13. Zeile `marken` nach §4.5 N1: `-` für fehlende Objekte; `+<n>` bei Überlänge, Zeile dann ≤ 119;
14. Zeile `start` mit `srv=` (§4.3 N1).

**Zusätzliche Sabotagen (N1), jede ⇒ FEHL:** Längen als 8n − 4 statt Blöcke × 8; Wächter `sum` entfernt;
`getDataPtr()` statt `getConstDataPtr()`; `#error`-Bindung entfernt; `srv=` fehlt.

### 4.9 Ent-C1 — Weg für Karte und Wegmarken

| Option | Inhalt | sieht | Preis |
|---|---|---|---|
| **A** (Vorschlag) | Blockgang über den Kontext des Haufens (`umm_local.h`) und die Blockform aus `umm_malloc.cpp`, nachgebildet; Übersetzung bricht bei anderem Core ab | M1–M4 vollständig: *was* und *wann* | hängt an inneren Einzelheiten von Core 3.1.2; befristet und mit Rückbau |
| **B** | nur öffentliche Mittel: M2 mit `free`, `max`, `frag` (`ESP.getHeapStats`), `u`, `p`; M4 als rohe Adressen; keine Karte | *wann* (L-a/L-a′/L-b) | sagt nicht sicher *was*: ohne Gang ist der grosse Block nicht verortet |
| **C** | wie B, dazu eine Sondierbelegung: `malloc` in Grösse von `max`, Adresse lesen, sofort `free` | verortet den grossen Block öffentlich | greift kurz in den Haufen ein, den sie misst; ob eine Belegung aus einem Unterbrechungskontext in diesem Augenblick scheitert, ist nicht geprüft (●) |

R1.3 im Paket 2026-10-15 hat `frag=` ausdrücklich ohne privaten Kopf gebaut (L468). A weicht davon ab — deshalb die
Rückfrage an Dich. Für A spricht: Nur so lässt sich `davor`/`danach` bestimmen, und davon hängt Frage F2 ab.

**Nachtrag N1 — entschieden am 17.10.2026: A** (Dein Entscheid nach dem Machbarkeitsbericht 1.1). Die Abweichung von R1.3
gilt damit ausdrücklich, und nur für dieses befristete Instrument; sie endet mit dem Rückbau (Ent-C4). Die Mittel der
Bindung an Core 3.1.2 stehen in §4.7 N1, die Blockform in §4.2 N1.

### 4.10 Kodierung und Zeilenende der Zieldateien (N1)

*Neu mit N1.* Festgestellt im Bericht 1.1 am Stand `353a693` (✔ ED). **Vor jedem Patch erneut festzustellen** (AKZ.1);
die Tabelle ersetzt das nicht.

| Datei | Kodierung | Zeilenende |
|---|---|---|
| `ESP-uclock.ino` | reines ASCII | **gemischt**: 2'144 CRLF / 327 LF, in 15 Abschnitten wechselnd; `setup()` (Zeilen 251–350) CRLF, `esp_heap_log()` und `loop()` CRLF |
| `http.cpp` | UTF-8, 37 Nicht-ASCII-Byte | 1 CRLF-Zeile (967) / 17'650 LF |
| `base.h` | im Bericht nicht genannt | Zeilen 1–27 CRLF, 28–57 LF |
| `base.cpp`, `wifi.cpp`, `weather.cpp`, `ntp.cpp`, `udpsrv.cpp`, `eepromdata.cpp` | im Bericht nicht genannt | nur CRLF |
| neue Datei (z. B. `heapkarte.cpp` / `.h`) | ASCII | CRLF wie die übrigen Moduldateien |

**Gepatcht wird byte-genau je Abschnitt**, mit dem Zeilenende des Abschnitts — nicht über ein Werkzeug, das die Datei
vereinheitlicht (CLAUDE.md, 04.10.2026). `wifi.cpp` und `weather.cpp` stehen in der Tabelle, weil der Bericht sie geprüft
hat; geändert werden sie nicht (§9).

---

## 5. Stufe 2 — Messung (auf Release I)

### 5.1 Arme und Reihenfolge

Arme A, B, E wie in §3, Reihenfolge **A B E · E A B · B E A**. Arm D (OTA sofort) ist in Stufe 2 nicht nötig — L499 zeigt
OTA als gleichgültig; er kommt in der Abnahme von Stufe 3 zurück (AKA.3). In allen Armen ab +30 s Smoketest und
`check-pwa.sh`, Ruhewert ≥ 150 s danach mit `tcp=0/0/0/0/0/0`. Das Werkzeug sammelt je Lauf die Zeilen `start`,
`karte`, `marken` in die Mitschrift (AKM.2).

### 5.2 Ruhewert

Gültig nur an einer Heap-Zeile ≥ 150 s nach dem Ende der Folge mit `tcp=0/0/0/0/0/0` und ohne Flash-Schreiben,
Wetter-Overlay oder kalten `update_status` dazwischen — jede Ausnahme gekennzeichnet. Die Kartenzeile `m` derselben
Minute gehört dazu.

### 5.3 Fragen

| | Frage | gelesen an |
|---|---|---|
| **F1** | Wo liegen `gross` und `zweit` in Ruhe, A gegen B? | Karte `m` am Ruhewert |
| **F2** | Welcher belegte Eintrag trennt in A `gross` von `zweit` (`davor`/`danach`)? Entspricht er einer Wegmarke? | Karte `m` am Ruhewert, Wegmarken `S`, `F` |
| **F3** | Seit wann liegt dieser Eintrag dort? | erster Schritt bzw. erstes Ereignis unter s0–s8, `S`, `F`, `U`, `m`, an dem `gross` in A von B abweicht |
| **F4** | Gleiche Zahl belegter Einträge in Ruhe, A gegen B? | `ent` am Ruhewert |
| **F5** | Arm E wie A oder wie B? In A: `u` und `p` je Schritt s2–s8 | Ruhewerte E; `start`-Zeilen A |

**N1:** F5 ist auf 3.2.41 beantwortet — E wie A (L500); in Stufe 2 wird sie unter dem Instrument wiederholt. Für F3 gilt
nach §3.4 N1: Der erste Abweichungspunkt wird nach L500 frühestens bei `F` erwartet (◐); weicht `gross` schon bei s0–s8
oder `S` ab, widerspricht das L500 und gehört als Befund ins Messprotokoll.

### 5.4 Zuordnung

| Zuordnung | Bedingung | Folge |
|---|---|---|
| **Z1** — L-a bzw. L-a′ belegt | E sauber **und** in A `u` ≥ 1 an einem Schritt s2–s8 **und** der Trenner ist ein Objekt, das zwischen diesem Schritt und s8 entsteht (Wegmarke oder Zeitpunkt) | Nachtrag mit Optionen, Ent-C3 |
| **Z2** — L-b belegt | E zerstückelt **und** der Trenner wird erst bei `F`, `U` oder einer `m`-Karte sichtbar, benannt über Wegmarke oder Grösse und Zeitpunkt | Nachtrag mit Optionen, Ent-C3 |
| **Z3** — nicht zugeordnet | Trenner keiner Wegmarke zuzuordnen und nicht über den Zeitpunkt; oder E gemischt | Nachtrag zum Instrument (weitere Wegmarken), neues Release I′, Stufe 2 wiederholt — **keine Abhilfe** |
| **Z4** — Instrument stört | AKM.1 misslingt | Stufe 2 hält an; Nachtrag zum Instrument |

Jede Aussage der Zuordnung trägt ein Kennzeichen (✔ belegt an Zahl und Lauf, ◐ abgeleitet). „Ein PCB als Näherung für
seinen Kontext“ (§4.5) bleibt ◐.

**Nachtrag N1:** (1) Z1 setzt „E sauber“ voraus; auf 3.2.41 war E zerstückelt (L500). Z1 ist damit nur erreichbar, wenn
sich E unter Release I anders verhält — das wäre selbst ein Befund. Zu erwarten sind Z2 oder Z3. (2) Der letzte Satz oben
ist überholt: Mit §4.5 N1 sind die Kontexte **genau** verortet; „PCB als Näherung“ kommt nicht mehr vor. (3) Ob V.3 die
Bedingung von Z2 schärft (etwa für die Hintergrundbelegung bis +30 s), bleibt offen **[offen: V.3]**.

---

## 6. Stufe 3 — Abhilfe

**Entscheidpunkt.** Nach Z1 oder Z2 schreibt der `spec-writer` einen Nachtrag zu dieser Spec: Beleg, Optionen mit Preis
und Risiko, betroffene Komponenten, Prüfstand. Der Lead legt Dir die Optionen per Rückfrage vor (Ent-C3). Erst nach
Deiner Freigabe des Nachtrags beginnt der `esp-developer`.

**Diese Spec legt keine Abhilfe fest.** L499 nennt als Beispiel die Reihenfolge in `setup()`; ob sie trägt, hängt am
Trenner. Eine Abhilfe ohne Beleg hat in diesem Projekt schon einmal eine plausible, aber falsche Lesart gebaut — H-CC zu
C71, widerlegt in L468.

**N1:** Das Beispiel aus L499 — die Reihenfolge in `setup()` — war auf K2 gemünzt, und K2 trägt in strenger Form nicht
mehr (L500). Ob eine Umstellung in `setup()` trotzdem etwas bewirkt, bleibt eine Frage an den Trenner, nicht an K2. Das
bestätigt die Regel dieses Abschnitts.

**Abnahme (fest, unabhängig von der Option):** AKA.2–AKA.7. Bezug für die Schwellen ist ΔRAM(A) — der Zuwachs „Globale
Variablen“ von Release A gegen 3.2.41, Instrument eingeschlossen. Zusätzlich gilt in jeder Reihe der Vergleich mit den
B-Läufen derselben Reihe, damit eine verschobene Grundlinie keinen falschen Erfolg ergibt.

---

## 7. Releaseschnitt und Einspielen

| Release | Inhalt | Komponenten | Tag |
|---|---|---|---|
| **I** | M1–M4 hinter `ESP_HEAP_MAP` | nur ESP | `release/3.2.34-<esp>-1.4.102` |
| **A** | Abhilfe nach Nachtrag; Instrument bleibt | ESP (weitere nur nach Nachtrag) | wie oben, bzw. mit neuer App-Version, falls der Nachtrag die PWA nennt |
| **R** | Rückbau (oder im nächsten ESP-Release mit anderem Inhalt) | nur ESP | wie oben |

Je Release: Version nur ESP (DIR-004), `make release-zip`, Commit, Tag, Push im selben Zug (DIR-011), `deploy.sh` ohne
`--delete` (DIR-005). Einspielzeilen samt Rückweg (OTA auf das vorige Tag) vom Lead, **bevor** Du einspielst. Nach jedem
OTA `install-app.sh --check` (DIR-017) und Smoketest samt Update-Quelle (DIR-009). Zu flashen ist **nur der ESP**; der STM
wird nicht geflasht, die PWA nicht neu hochgeladen.

**Der Start nach dem eigenen Einspielen ist keine Startmessung** — die Werkzeuge fragen sofort an (L499). Gemessen wird
erst in Stufe 2 bzw. in der Abnahme.

---

## 8. Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Wetter-Endpunkte? Restore-Bedingung? Richtige Schicht?

> PWA, Legacy-Oberfläche, Wetter-Endpunkte und die Restore-Bedingung um `pending_ticker_restore` werden nicht berührt —
> die Releases I und R ändern nur ESP-Quellen, Release A ebenso, es sei denn, ein freigegebener Nachtrag nennt mehr
> (AKZ.2). Die Auslieferung als `.gz` bleibt. **Schicht:** Die Ursache liegt im Haufen des ESP, also wird dort gemessen
> und — nach dem Beleg — behoben. Ausdrücklich nicht in den Werkzeugen (keine Ablaufregel, Dein Entscheid) und nicht in
> `app.js`.
>
> **N1:** Die Bindung an innere Einzelheiten von Core 3.1.2 (Ent-C1 = A) ist auf eine eigene Übersetzungseinheit begrenzt
> und bricht die Übersetzung bei jedem anderen Core ab (§4.7 N1) — ein stilles Weiterlaufen mit falscher Blockform ist
> ausgeschlossen. Die Wegmarken **lesen nur**: `EEPROM` über `getConstDataPtr()`, der Wetter-Client über ein
> Zugriffsglied, ohne `weather.cpp` zu ändern.

**Scalable systems** — STM-Kommandos? Byte je Minute auf der UART? Unter 20 s Watchdog? Blockade des Hauptloops?

> **STM-Kommandos: keine.** Die neuen Zeilen tragen das Präfix `- ` und sind Meldungen des ESP
> (`ESP-uclock.ino:564-565`). **UART:** im Betrieb eine Kartenzeile je Minute, rund 100 Byte (1,7 Byte/s) zu den rund
> 105 Byte der Heap-Zeile; je Start einmalig rund 1,2 KB, **verteilt auf mindestens 14 s**, nie mehr als eine Zeile je
> Sekunde — der Empfangsring des STM (1'024 Byte, `UART_RXBUFLEN`) sieht nie mehr als eine Zeile am Stück. **Watchdog:**
> Der Blockgang ist auf die Blockzahl begrenzt (Wächter `it`) und kann nicht hängen bleiben. **Blockade:** je Gang
> gesperrte Unterbrechungen, gemessen als `us=`; über 1'000 µs wird vorgelegt (AKI.9). Im Betrieb ein Gang je Minute,
> beim Start elf. Die PWA pollt nicht anders als heute.
>
> **N1:** Gangdauer geschätzt 70–200 µs, theoretisch bis rund 1 ms bei 1'590 Blöcken (Bericht 1.1) — unter der Schwelle,
> gemessen wird trotzdem. Die `start`-Zeilen werden mit `srv=` rund 7 Zeichen länger; an der Last auf der Brücke ändert
> das nichts Wesentliches.

**Secure by design** — Fremddaten? Credentials in Logausgaben?

> Die Zeilen enthalten Blockindizes, Längen, Zähler und Zustände — **keine IP-Adresse, keinen Port, keinen Netznamen,
> kein Geheimnis** (AKI.1 (g); für TCP-PCBs nur Index und Zustand, AKI.6). `/api/stm32_log` ist im LAN lesbar; die
> Inhalte verraten nichts, was ein Angreifer nutzen könnte (der ESP8266 hat keine Adressraum-Zufallsverteilung, die
> Indizes schützen nichts). Kein neuer Endpunkt, keine Konfiguration.
>
> **N1:** Die Wegmarken `eep` und `dhcp` nennen nur den **Blockindex** ihres Objekts, nie dessen Inhalt — keine Adresse,
> kein Byte aus dem `EEPROM`-Puffer.

**Stable & reliable** — Fehler ausgewertet? Stille Verwerfungen? Zustand nach Abbruch? Flags auf jedem Pfad aufgelöst?

> **Kein stilles Zurechtbiegen:** Ein beschädigter oder unerwarteter Haufen ergibt `abbruch=<grund>`, keine falschen
> Zahlen, und die Logwache meldet ihn (AKI.11). **Selbstprüfung:** `fsum` gegen `free=` und `gross` gegen `max=` decken
> einen falschen Gang am Gerät auf (AKI.3). **Sperre** auf jedem Weg gelöst, mit Sabotage im Prüfstand. **Merker:** Jedes
> Ereignis (`S`, `F`, `U`, `marken`) hat einen festen Platz, wird einmal gesetzt und bei der Ausgabe gelöscht; tritt es
> nicht ein, bleibt der Platz leer, und das Werkzeug meldet es als „nicht eingetreten“, nicht als Fehler (AKI.5). Ein
> Neustart setzt alles zurück (BSS). **Werkzeug:** fehlender Start, fehlende Anfrage oder fehlender Ruhewert ⇒ Exit ≠ 0
> mit Meldung (AKV.1) — heute endet es mit 0.
>
> **N1:** Die Selbstprüfung ist jetzt **exakt** (Gleichheit, AKI.3 N1) und um `sum` ergänzt — ein falsch nachgebildeter
> Gang kann nicht unbemerkt plausible Zahlen liefern. Die Sperre gibt der Wrapper ohne Verzweigung frei; der Weg „nicht
> gelöst“ bleibt als Sabotage herstellbar. `getDataPtr()` ist ausgeschlossen, weil es `_dirty` setzen und damit den
> Zustand ändern würde, den es nur lesen soll.

---

## 9. Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| `ESP8266/ESP-uclock/ESP-uclock.ino` | `ESP_HEAP_MAP`; Erfassen s0–s8 und `S` in `setup()`; Ausgabe aller Zeilen aus `loop()`; `m`-Karte zur Heap-Zeile | `esp-developer` |
| `ESP8266/ESP-uclock/http.cpp` | Ereignis `F` (nach dem ersten bedienten Durchlauf), `U` (nach `- update cache nach`); Zähler `u` über `WcHttpServer` | `esp-developer` |
| neue Datei, z. B. `ESP8266/ESP-uclock/heapkarte.cpp` / `.h` (Name beim `esp-developer`) | Blockgang als reine Funktion, Formatierung, Wegmarken — getrennt, damit der Prüfstand sie allein übersetzen kann | `esp-developer` |
| `ESP8266/ESP-uclock/version.h` | `ESP_VERSION` | `release-engineer` (im Auftrag des Leads) |
| `tools/` (Werkzeug der Arme), `tools/watch-log.sh`, `tools/checks/auszug/esp/c80i/` | Arm E, Mitschrift, Exit-Codes, `--abnahme`; `abbruch=` als Warnung; Prüfstand ablegen | Lead |
| `BEFUNDE.md` | Messprotokoll (Lead), Nachführung C80, Rückbau in der ToDo-Liste (`doc-writer`) | Lead / `doc-writer` |
| `CHANGELOG.md` | je Release | Lead |
| `specs/paket-2026-10-17-c80/**` | Nachträge, Endstand | `spec-writer` |

`wifi.cpp` wird **nicht** geändert: s2 wird nach der Rückkehr aus `wifi_connect()` in `setup()` erfasst.

**Nachtrag N1 (Bericht 1.1):**

- `http.cpp`: zwei **neue exportierte Funktionen** — Zahl der Einträge in `_unclaimed` (für `u`) und der Merker der ersten
  Anfrage (für `F`, weil `http_first_request_logged` `static` ist). Beides nur lesend.
- Neue Übersetzungseinheit: zusätzlich die Bindungsmittel aus §4.7 N1 und die Unterklasse mit dem Zugriffsglied für den
  Wetter-Client; CRLF.
- **`weather.cpp` wird nicht geändert** — der Kontext des Wetter-Clients wird über das Zugriffsglied gelesen.
- `ntp.cpp`, `udpsrv.cpp`, `eepromdata.cpp` werden nach dem Weg aus §4.5 N1 nicht gebraucht (UDP über `udp_pcbs`,
  `EEPROM` über das globale Objekt); ob `base.h` eine Deklaration bekommt, steht im Taskbericht 1.3 mit Kodierung und
  Zeilenende (§4.10).
- `tools/startprobe.py` ist das Werkzeug der Arme (Commit `f012309`).

---

## 10. Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| Neustart nach dem Einspielen als Regel (Vorschlag aus L482) | Dein Entscheid: Firmware-Ursache suchen |
| Abhilfe ohne Beleg, etwa `setup()` umstellen oder den Server später starten | Dein Entscheid; ohne Kenntnis des Trenners kann eine Umstellung eine Verschiebung durch eine andere ersetzen (H-CC, L468) |
| `umm_info(NULL, true)` | gibt jeden Block als eigene Zeile über die UART aus (Core `Notes.h:141`) — Hunderte Zeilen auf der Brücke, gegen die Regel „eine Zeile je Sekunde“ |
| Belegungsprotokoll über `-Wl,--wrap=malloc` oder Debug-Optionen des Cores | ändert Build und `Makefile`, schreibt je Belegung, verfälscht Zeitverhalten und Haufen |
| Heap-Zeile erweitern | Werkzeuge lesen sie (`ESP-uclock.ino:642`); sie hat schon 105 von 119 Zeichen |
| Startschritte direkt in `setup()` ausgeben | Zeilen in der Startphase treffen den STM während CAP/FIRMWARE (`ESP-uclock.ino:266-283`) und kämen im Stoss |
| Zweite UART des ESP (nur Senden) für einen vollständigen Abzug | ob der Pin auf der Platine erreichbar ist, ist nicht geprüft (●); ausserhalb dieser Spec |
| **N1:** Sperre über RAII (Objekt, dessen Destruktor freigibt) | macht die Sabotage „Sperre nicht gelöst“ unherstellbar und damit ihre Gegenprobe leer (Bericht 1.1) |
| **N1:** `EEPROM.getDataPtr()` für die Wegmarke | setzt `_dirty` — eine Messung, die den gemessenen Zustand ändert |

---

## 11. Versionsfolgen

- [ ] STM `src/main.h` anheben — **nein**
- [x] ESP `version.h` anheben — **ja, je Release I, A, R einmal**
- [ ] App `APP_VERSION` anheben — **nein** (ausser ein Nachtrag zu Stufe 3 nennt die PWA)
- [ ] `CACHE_NAME` in `sw.js` anheben — **nein** (nur zusammen mit `APP_VERSION`)

Nur der `release-engineer` führt das aus, im Auftrag des Leads (R4).

---

## 12. Offene Fragen

| | Frage | Wer beantwortet | Bis wann |
|---|---|---|---|
| Q1 | **Ent-C1** — Weg A, B oder C (§4.9) | Du, nach dem Bericht 1.1 | vor Task 1.3 |
| Q2 | **Ent-C2** — Grenze 320 Byte | Lead | bei der Freigabe |
| Q3 | Wofür stehen die 504 Byte? | Stufe 2 | AKM.3 |
| Q4 | Wann endet `setup()` relativ zur Startzeile? | M2, `ms` bei s8 | erster Start auf Release I |
| Q5 | Lässt `Serial.flush()` in `setup()` lwIP tatsächlich Aufbauten annehmen? (`uart.cpp:569-570` ruft `esp_yield()`, ◐) | M2, `u` je Schritt | Stufe 2 |
| Q6 | Sind die Wegmarken aus §4.5 ohne Eingriff in den Core erreichbar? | Bericht 1.1 | vor Task 1.3 |
| Q7 | Stört ein Blockgang mit gesperrten Unterbrechungen den Betrieb messbar? | AKI.9 (`us=`), AKM.5 | Stufe 2 |

**Nachtrag N1 — Stand am 17.10.2026:**

| | Stand |
|---|---|
| Q1 | **beantwortet: A** (Du, nach dem Bericht 1.1; §4.9 N1) |
| Q2 | **beantwortet: ≤ 320 Byte** (Lead); gerechnet rund 290 (§4.6 N1) |
| Q3 | offen — Stufe 2 |
| Q4 | **teilweise (◐):** In Arm A stand `- heap erste anfrage` 0,3–0,6 s nach der Startzeile; `setup()` war also spätestens dann beendet (§2.2 N1 Punkt 4). Gemessen wird es mit `ms` bei s8 |
| Q5 | **an Gewicht verloren:** Für den Schaden ist ein Aufbau während `setup()` nach L500 nicht nötig. Die Frage bleibt für die Lesart von `u` je Schritt offen |
| Q6 | **beantwortet: ja**, alle, dazu die Kontexte genau (§4.5 N1) |
| Q7 | offen — AKI.9, AKM.5 |
| **Q8** (neu) | **Was belegt der ESP im Fenster nach `setup()`, und braucht Stufe 1 dort Erfassungspunkte?** — `firmware-analyst` (V.3), danach `spec-writer` (Nachtrag N2, Task V.4, falls nötig); **vor Task 1.3** **[offen: V.3]** |
