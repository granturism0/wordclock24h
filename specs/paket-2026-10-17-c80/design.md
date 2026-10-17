# Design — Paket 2026-10-17 „C80: bleibende Heap-Zerstückelung nach frühem Verkehr — Firmware-Ursache“

**Erstellt:** 2026-10-17, `spec-writer`, Stand `353a693`. Anforderungen: `requirements.md`; Tasks: `tasks.md`.
Kennzeichen und Kennungen wie in `requirements.md`. Momentaufnahme (DIR-006).

**Nachtrag N1** (17.10.2026, Stand `6b3cbe9`): Freigabe; Ent-C1 = A, Ent-C2 ≤ 320 Byte, Ent-C4 bestätigt. Das Ergebnis der
Vorprobe V.2 (`BEFUNDE.md` L500) steht in §1, §2.2–§2.4, §3.4 und §5.4; die Ergebnisse des Machbarkeitsberichts 1.1 des
`esp-developer` in §4.2–§4.10, §8, §9 und §12. Jede Änderung trägt die Marke **N1**, der Text davor bleibt stehen.
Kennzeichen **✔ ED** / **◐ ED** wie in `requirements.md`. Die Lesart der Vorprobe durch den `firmware-analyst` (V.3) steht
noch aus; die Stellen, die auf sie warten, tragen **[offen: V.3]**, ihr Platz ist §3.5.

**Nachtrag N2** (17.10.2026, Stand `a803e98`, Task V.4) — **zur Freigabe durch Dich**: die Lesart V.3 des
`firmware-analyst` und die Zusatzprobe des Leads (`BEFUNDE.md` **L501**). §3.5 ist gefüllt; alle Stellen mit
**[offen: V.3]** verweisen auf N2. N2 ändert den **Umfang von Stufe 1** (§4.1–§4.8: Töpfe statt Startschritte, Karten
S/F/X/T, ein Satz Wegmarken bei X), die **Messung** (§5: Folge nach Ent-C5, Arm RST, Rangfolge der Fragen,
Widerlegungsregeln) und die Antworten in §8, §9, §10 und §12. Jede Änderung trägt die Marke **N2**; Überholtes bleibt
stehen und ist als „überholt (N2)“ gekennzeichnet. *Zum Stand N1 oben, überholt (N2):* V.3 steht nicht mehr aus.

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
| **N2:** `setup()` endet rund 0,11 s nach der Startzeile; der Haufen steht dort in A, E und B gleich bei `free=7840 max=7104` | ✔ FA | L501 (Startfolgen A5, E4, B6 aus L500) |
| **N2:** Der Fehlbetrag ist die dauerhafte Zunahme nach `setup()`: 7'840 − 7'336 = 504 = 7'104 − 6'600; E Lauf 8 672 = 672; in B 560 dazu, `max` bleibt 7'104 | ✔ FA (an L500 gerechnet) | L501 |
| **N2:** Im Anwendungscode belegt zwischen +3 und +30 s nichts dauerhaft | ✔ FA; Update-Cache ✔ SW | L501; `http.cpp:13935` (frühestens 60 s) |
| **N2:** Der `ClientContext` einer Anfrage ist im nächsten `loop()`-Durchlauf frei; ein PCB in TIME_WAIT bleibt (rund 192 Byte, 120 s oder bis `tcp_tw_limit()`) | ✔ FA, ✔ SW | `http.cpp:17376`, `:17384`, `:500-506`; `WiFiServer.cpp:150-151`; `tcp_priv.h:137` (`TCP_MSL` 60 s) |
| **N2:** Eine einzige Anfrage nach 8,1 / 10,1 / 12,1 / 14,1 s ⇒ 7'104 (4 von 4); mit L500 liegt die Hintergrundbelegung **rund zwischen +3 und +8 s** | ✔ L | L501, Zusatzprobe |
| **N2:** RST nach 3,1 s ohne Daten (kein TIME_WAIT auf der Uhr) ⇒ 7'104 und 6'928 | ✔ L | L501 |
| **N2:** Verdrängung: TIME_WAIT-PCB hält den kleinen freien Bereich, Best-Fit schneidet die Hintergrundbelegung vom Anfang des grossen Blocks ab | ◐ FA (gefolgert) | L501; `umm_malloc.cpp:729-733`, `:777-798` |
| **N2:** Wer die 504 Byte belegt | **offen** — vermutlich SDK/lwIP oder eine Zeile vom STM (? FA) | L501 |
| **N2:** `ESP.getFreeHeap()` ist O(1) — liest den Zähler der freien Blöcke | ✔ SW | `Esp.cpp:220-222`, `umm_local.c:186-191` (Core 3.1.2, `UMM_STATS`) |

**N2:** Die Zeile „Welche Belegung den Block teilt, und wann sie entsteht — offen“ ist zur Hälfte beantwortet: **wann**
rund +3 bis +8 s (✔ L); **was** offen.

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

**N2:** Eine dritte Abgabestelle, wichtig für die Töpfe (§4.3 N2): `WiFiServer::accept()` ruft ohne wartende Verbindung
`optimistic_yield(1000)` (`WiFiServer.cpp:150`, ✔ SW) — also in fast jedem `loop()`-Durchlauf innerhalb von
`http_server_loop()`. lwIP-Rückrufe können deshalb auch dort laufen, nicht nur zwischen zwei Durchläufen.

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
   ohne Anfrage fällt (§1 N1) — **und welche Lesart das stützt, liefert V.3** **[offen: V.3]** (→ N2) (§3.5).

**Nachtrag N2 (L501):**

- **Zu Punkt 4:** belegt (✔ FA) — `setup()` endet rund **0,11 s** nach der Startzeile, in allen drei untersuchten Läufen
  mit demselben Haufen (`free=7840 max=7104`). Der Satz „Gemessen wird das erst mit `ms` bei s8“ ist **überholt (N2)**; s8
  entfällt (§4.3 N2).
- **Zu Punkt 5:** beantwortet in §3.5 N2 — eine Hintergrundbelegung von 504 Byte zwischen rund +3 und +8 s, Verursacher
  offen.

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
  **[offen: V.3]** (→ N2).

**Nachtrag N2 (L501) — L-b geschärft, eine Gegenlesart:**

- **L-a′** trägt nach V.3 nicht mehr nur „◐“, sondern belegt nicht (✔ FA): Bis zum Ende von `setup()` sind alle Arme
  gleich.
- **L-b** trägt; der Satz in der Spalte „Was dafür spräche“ zu `U` ist **überholt (N2)** (U entfällt, §4.4 N2). L-b wird
  durch V.3 zu einer prüfbaren Form geschärft, und es gibt eine Gegenlesart:

| Lesart | Inhalt | Was dafür spricht bzw. spräche | Was dagegen spräche |
|---|---|---|---|
| **L-b′** (N2) — Verdrängung | Eine Hintergrundbelegung von rund 504 Byte entsteht **in allen Armen** rund +3 bis +8 s nach der Startzeile. Eine frühe Anfrage hinterlässt einen TIME_WAIT-PCB im kleinen freien Bereich; die Hintergrundbelegung passt dort nicht mehr, Best-Fit nimmt den grossen Block und schneidet vom Anfang ab. Nach Ablauf des PCB liegen die 504 Byte vorn im ehemals grossen Block | ✔ L: Anfrage ab 8,1 s sauber (4 von 4), Anfrage bis 3,2 s zerstückelt (3 von 3); in B liegt die Zunahme ausserhalb des grossen Blocks; Lauf 9: `free` −272 bei gleichem `max`. In Stufe 2: Wid-1 und Wid-2 greifen nicht (§5.4 N2) | Wid-1 oder Wid-2 greift; ✔ L schon heute: ein RST-Lauf ohne TIME_WAIT mit 176 Byte Fehlbetrag (das Modell trägt zur Hälfte) |
| **L-c** (N2) — Lazy-Init | Die 504 Byte entstehen beim **ersten Durchgang eines Wegs**, den die erste Anfrage auslöst — nicht im Hintergrund | in E der Rückgang im Topf `srv` bei der ersten Anfrage; in B der Rückgang erst bei der ersten Anfrage nach W | ✔ L schon heute: In B fällt der Haufen bis +30 s auch ohne Anfrage (L500) |

### 2.4 Kandidaten

| | Kandidat | Herkunft | Einschätzung |
|---|---|---|---|
| K1 | Kontext des Wetter-Clients bleibt bis zum nächsten Abruf (`weather.cpp:16`, `WiFiClient openweather_client`; C77) | ◐ FA | Läuft in den Armen A und B gleich; kann den Unterschied nur im Zusammenspiel mit der frühen Verbindung erklären. Laut L499 lag vor der ersten Anfrage der schlechten Starts kein Wetterabruf |
| K2 | Früher Aufbau während `setup()` unter den UDP-Kontexten | ◐ FA | = L-a; §2.2. **N1: in strenger Form widerlegt (L500, §2.2 N1)** |
| K3 | Spät aufgebautes WLAN | ? FA | In A und B gleich; WLAN steht zwingend vor der ersten Antwort |
| K4 | Interne Belegungen von SDK/lwIP | ? FA | Zusammengefasst mit K6 |
| K5 | Legacy `/network` ohne `scanDelete()` | ? FA | In keinem Arm aufgerufen |
| K6 | Einmalige Belegungen der ersten Sekunden — Schritte nach der Startzeile, erste Antworten des STM, erster NTP-Abgleich, SDK/lwIP (DHCP, ARP) —, die sich über eine gerade lebende frühe Verbindung legen | ◐ spec-writer | = L-a′ bzw. L-b; welche Belegungen das sind, ist nicht bekannt. **N1: der Teil „nach `setup()`“ (= L-b) bleibt; dass im Fenster bis +30 s im Hintergrund belegt wird, ist in Arm B gemessen (L500). Welche Belegungen, offen [offen: V.3] (→ N2)** |

**Statisch ausgeschlossen als Quelle einer Belegung (✔ SW):** das Nachführen des Backlogs — `tcp_backlog_set()` ändert
nur ein Feld des Listen-PCB (`http.cpp:42`, `:63`, `:466-476`); `String` auf Dateiebene (§1). **Kein Kandidat für sich,
aber Mitspieler:** `http_client` ist global (`http.cpp:100`) und hält den `ClientContext` der zuletzt bedienten
Verbindung bis zur nächsten Zuweisung in `http_server_loop()` (`:17376`); die Freigabe-Reihenfolge der Verbindungsobjekte
ist damit an den `loop()`-Takt gebunden.

**Logik der Arme (◐):** Alles, was in Arm A und Arm B gleich abläuft, kann den Unterschied nur im Zusammenspiel mit der
frühen Verbindung erklären. Da `free` gleich bleibt, ist die Menge der dauerhaften Belegungen in beiden Armen dieselbe;
gesucht ist die Belegung, die in Arm A **an anderer Stelle** liegt.

**Nachtrag N2 (L501):**

| | Stand nach V.3 |
|---|---|
| K1 | **für die 504 Byte ausgeschieden** (✔ FA): im Fenster bis +30 s kein Wetterabruf. Bleibt als Mitspieler im Betrieb (C77) |
| K2 | widerlegt (N1) |
| K3 | in allen Armen gleich; WLAN steht vor `S`. Als Ort der Hintergrundbelegung Teil von K4 |
| K4 | **Hauptkandidat für den Verursacher** (? FA): SDK/lwIP — erscheint im Topf `sys`, über die Abgabestellen aus §2.1 N2 auch in `srv`, `hlg`, `ovr` |
| K5 | ausgeschieden (in keinem Arm aufgerufen) |
| K6 | aufgegangen in L-b′: die Hintergrundbelegung ist gemessen (✔ L), ihr Inhalt offen |
| **K7** (neu) | **Eine Zeile vom STM** — im Mitschnitt fällt im Fenster +3 bis +8 s nur `display_clock flags=0x04` des STM auf, jedes Mal +7,43 bis +7,47 s (? FA, Zusammenhang Vermutung). Trägt sie oder ihre Folge die Belegung, erscheint diese im Topf `stm` (§4.3 N2) |

**Zum „Mitspieler“ oben (✔ FA, ✔ SW):** Die nächste Zuweisung an `http_client` geschieht **im nächsten
`loop()`-Durchlauf**, nicht bei der nächsten Anfrage — `accept()` liefert ohne wartende Verbindung einen leeren
`WiFiClient` (`WiFiServer.cpp:150-151`), und die Zuweisung (`http.cpp:17376`) gibt den alten Kontext frei. Die Lesart
„hält den Kontext bis zur nächsten Anfrage“ (so in §4.4, Zeile `F`) ist **überholt (N2)**. Von einer frühen Anfrage bleibt
nach einem Durchlauf nur der PCB in TIME_WAIT (rund 192 Byte; 120 s oder bis `tcp_tw_limit()`, `http.cpp:500-506`,
aufgerufen `:17384`).

**Zur „Logik der Arme“ (✔ FA):** bestätigt in der Form — die Menge ist in A und B gleich (504 Byte nach `setup()`), nur
ihr Ort verschieden. **Ergänzt aus V.3** (✔ FA, Fundstellen L501): Im Anwendungscode belegt zwischen +3 und +30 s nichts
dauerhaft — Update-Cache frühestens 60 s nach dem Start (`http.cpp:13935`, ✔ SW), Heap-Zeile nur auf dem Stack,
`var_sync_check()` schreibt in den statischen Ring, NTP nur auf Anforderung des STM, kein Wetter, kein Scan, kein mDNS.

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

**N2:** Erweitert um Arm `E<n>` (Anfrage n s nach der Startzeile) und Arm `R` (in dieser Spec **RST**: Verbindung
aufbauen und per RST abbrechen, ohne Daten), Commit `3b9528a`; ein Werkzeugfehler aus der Zusatzprobe ist behoben und
gegengeprüft (L501). Gegenproben (a) und (c) belegt (`requirements.md` AKV.1 N2).

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
  V.3 fest (Task V.4), **bevor** Task 1.3 beginnt **[offen: V.3]** (→ N2).

**Nachtrag N2:** Der Satz „In keinem Fall ändert die Vorprobe den Umfang von Stufe 1“ ist **überholt (N2)**. Mit V.3 und
der Zusatzprobe ändert sich der Umfang: Die Startschritte s0–s8 entfallen ganz — auch als Grundlinie, denn die Lage am
Ende von `setup()` liefert die Karte `S` im ersten `loop()`-Durchlauf, und Q4 ist beantwortet (§12 N2). An ihre Stelle
treten die Verursacher-Töpfe im Startfenster W und die Karten `X` und `T` (§4.3 N2, §4.4 N2). Der Satz „bleiben als
Grundlinie nützlich“ in N1 ist **überholt (N2)**.

### 3.5 Lesart der Vorprobe (V.3) — **[offen: V.3]** (→ N2)

*Platzhalter, N1.* Hier wird die Lesart des `firmware-analyst` zu L500 eingetragen, sobald sie vorliegt: welche Lesart
gestützt ist, was im Fenster nach `setup()` belegt wird (auch die Hintergrundbelegung bis +30 s in Arm B), welche Annahme
aus §2 nicht trägt, Hinweise für Stufe 2 und — falls nötig — Änderungen am Umfang von Stufe 1 (dann als Nachtrag N2,
Task V.4). Bis dahin gilt §3.4 mit dem Vorbehalt aus N1. Der `spec-writer` greift ihr nicht vor.

**Nachtrag N2 — Lesart V.3 (`firmware-analyst`, 17.10.2026, auf den Startfolgen der Läufe A5, E4, B6 aus L500) und
Zusatzprobe (Lead, 17.10.2026 18:23–18:52), zusammengefasst in L501.** Der Bericht V.3 selbst liegt nicht im Repository.

**3.5.1 Gestützte Lesart.** **L-b in der geschärften Form L-b′** (Verdrängung, §2.3 N2) — ◐ FA, gefolgert. L-a und L-a′
tragen nicht (✔ FA: bis zum Ende von `setup()` sind alle Arme gleich). L-c ist die Gegenlesart, die Stufe 2 mit Wid-1
prüft.

**3.5.2 Was im Fenster nach `setup()` belegt wird.**

- `setup()` endet rund 0,11 s nach der Startzeile; Haufen dort in allen Armen `free=7840 max=7104` (✔ FA).
- Der Fehlbetrag ist genau die dauerhafte Zunahme nach `setup()`: 7'840 − 7'336 = 504 = 7'104 − 6'600; E Lauf 8: 672 = 672
  (168 Byte mehr belegt); B: 560 dazu, `max` bleibt 7'104 — alles ausserhalb des grossen Blocks (✔ FA, gerechnet).
- Im Anwendungscode belegt zwischen +3 und +30 s nichts dauerhaft (✔ FA, §2.4 N2). Der Verursacher liegt vermutlich in
  SDK/lwIP (K4) oder in einer Zeile vom STM (K7) — ? FA, keine Fundstelle.
- **Zeitfenster (✔ L, Zusatzprobe):** Eine einzige Anfrage nach 8,1 / 10,1 / 12,1 / 14,1 s ergibt 7'104 (4 von 4); mit
  L500 (1,9–3,2 s ⇒ 3 von 3 zerstückelt) liegt die Belegung **rund zwischen +3 und +8 s**.
- Die Hintergrundbelegung in Arm B bis +30 s (§1 N1) ist dieselbe Belegung; sie landet dort ausserhalb des grossen
  Blocks (✔ FA).

**3.5.3 Welche Annahmen aus §2 nicht tragen.**

- §2.2 Punkt 4 (◐) ist belegt (0,11 s).
- §2.4 „Mitspieler“ in der Lesart „Kontext bis zur nächsten Anfrage“ (§4.4, Zeile `F`) — überholt (§2.4 N2): Der Kontext
  ist im nächsten Durchlauf frei; übrig bleibt der PCB in TIME_WAIT.
- §3.4 N1 „s0–s8 als Grundlinie“ — überholt (§3.4 N2).
- `u`/`p` je Schritt s2–s8 (F5) — überholt: In der ersten Antwort waren `tcp_now_unclaimed` und `http_accept_dead` 0.

**3.5.4 Modell und was es nicht erklärt.** Der TIME_WAIT-PCB der frühen Anfrage liegt nach Best-Fit im kleinen freien
Bereich (Lauf 9: `free` −272 bei unverändertem `max`). Die Hintergrundbelegung passt dort nicht mehr; Best-Fit nimmt den
grossen Block und schneidet vom Anfang ab (`umm_malloc.cpp:729-733`, `:777-798`). Ist der PCB weg, verschmilzt der kleine
Bereich; die 504 Byte liegen vorn im ehemals grossen Block (◐ FA). **Die RST-Probe stützt das zur Hälfte** (✔ L): ohne
TIME_WAIT einmal sauber (7'104), einmal ein kleinerer Fehlbetrag (6'928: 176 Byte im grossen Block, 56 Byte mehr belegt).
Das Modell erklärt nicht alles.

**3.5.5 Rangfolge der Fragen für Stufe 2.** **F3** (wann und wer belegt) vor **F2** (wer trennt; Vorhersage: `davor` von
`gross` in A/E ein belegter Eintrag von rund 504 Byte, in B ein gleich langer im kleinen Bereich) vor **F4** (gleich viele
belegte Einträge in A und B) vor **F1**. **F5 erledigt** (E wie A). → §5.3 N2.

**3.5.6 Hinweise für Task 1.3 — umgesetzt als Anforderungen in §4 N2:**

1. **Verursacher-Töpfe** je Aufruf in `loop()` (`ESP-uclock.ino:1548-1582`) über `ESP.getFreeHeap()`, dazu Topf `sys`;
   nur in den ersten 60 s nach dem ersten Durchlauf; je Topf Nettosumme, grösster Einzelrückgang und dessen `ms`; rund
   60 Byte BSS; Ausgabe danach, eine Zeile je Sekunde. Vorbehalt: lwIP-Rückrufe in `delay()`/`Serial.flush()` innerhalb
   eines Topfs — Arm B liefert das klare Bild. → §4.3 N2.
2. **Karte `X`** am Anfang von `loop()`: erstmals `free < free(S) − 400`, solange keine aktive TCP-Verbindung besteht;
   Wegmarke: Blockindex jedes PCB in `tcp_tw_pcbs` (wie `http.cpp:745`). → §4.4 N2, §4.5 N2.
3. **Karte `T`**, wenn `tcp_tw_pcbs` in den ersten 180 s von ≥ 1 auf 0 fällt. → §4.4 N2.
4. **`S`** im ersten `loop()`-Durchlauf; s0–s7 streichen oder kürzen — spart rund 110 Byte für 1–3 innerhalb von Ent-C2.
   → §4.3 N2 (gestrichen sind s0–s8 ganz), §4.4 N2.
5. **`F`** einen Durchlauf **nach** der ersten Anfrage erfassen. → §4.4 N2.
6. **`U`** kaum noch Gewicht. → entfällt, §4.4 N2.

**3.5.7 Widerlegungsbedingungen aus V.3** — als Zuordnungsregeln **Wid-1** und **Wid-2** in §5.4 N2.

**3.5.8 Was der `spec-writer` daraus abgeleitet hat und nicht vom Analysten stammt** (◐ spec-writer, zur Prüfung mit der
Freigabe): (a) die Ausgabesperre in W (§4.1 N2); (b) Ent-C5 — die Töpfe über 60 s und die Karte T setzen voraus, dass bis
nach dem Ablauf des frühen TIME_WAIT kein weiterer Verkehr läuft; mit der Folge ab +30 s wie in V.2 ist das nicht gegeben
(§5.1 N2); (c) ein einziger Satz Wegmarken bei X statt bei S und F (§4.5 N2); (d) die Summenprobe der Töpfe (§4.3 N2);
(e) die Bedingung „ohne neue Annahme“ für `F` (§4.4 N2); (f) Arm RST in Stufe 2 (§5.1 N2).

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

**N2:**

- **Keine Ausgabe des Instruments im Startfenster W.** Jede Ausgabe ruft `Serial.flush()` und gibt die CPU ab (§2.1); in W
  würden dabei lwIP-Rückrufe in den Topf der Ausgabe fallen und die Töpfe verfälschen. Alles in W Erfasste erscheint nach
  dem Ende von W, eine Zeile je Sekunde.
- **`setup()` bleibt unverändert** — auch kein Erfassen mehr dort (s0–s8 entfallen, §4.3 N2).
- **Startfenster W:** von t0 (Anfang des ersten `loop()`-Durchlaufs) 60 s lang; bei Ent-C5 = B 25 s.

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

**Nachtrag N2:**

- **Anlässe ab N2:** `m`, `S`, `F`, `X`, `T`; `U` entfällt (§4.4 N2). „Anlass `m` = nach jeder Heap-Zeile“ ist **überholt
  (N2)**: `m` nur zu Heap-Zeilen **ausserhalb** von W. Die erste Heap-Zeile (im ersten Durchlauf, `pending` in
  `esp_heap_log()`, `ESP-uclock.ino:632`) bekommt keine `m`-Karte; die Lage dort zeigt `S`.
- **Feld `ms=`** am Ende der Ereigniskarten `S`, `F`, `X`, `T`: Millisekunden seit t0 (`S`: 0). `m` trägt es nicht.

  ```
  - heap karte <anlass> ent=<belegt>/<frei> fsum=<n> gross=<blk>+<n> zweit=<blk>+<n> davor=<blk>+<n> danach=<blk>+<n> ms=<n>
  ```

- **Längste Form** (◐ spec-writer, gezählt): ohne `ms=` 108 Zeichen (`ent=1590/1590`, Indizes vierstellig, Längen
  fünfstellig); mit ` ms=180000` 118 Zeichen. `ms` hat höchstens sechs Stellen, weil `F`, `X`, `T` nur bis 180 s nach t0
  erfasst werden (§4.4 N2). Unter 119.
- Mehr Blockgänge als in N1 gibt es nicht: je Start höchstens vier Ereignisgänge (`S`, `F`, `X`, `T`), danach einer je
  Minute (`m`).

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
  Begründung für s5–s7 — sie liefern weiterhin die Grundlinie vor der ersten Anfrage. **[offen: V.3]** (→ N2)

**Nachtrag N2 — überholt ist der ganze Abschnitt oben samt N1** (Startschritte s0–s8, Werte je Schritt mit `u`, `p`,
`srv=`, Zeile `- heap start`, `us` bei s8, exportierte Funktion für `_unclaimed`). Grund: Bis zum Ende von `setup()` sind
alle Arme gleich (✔ FA, L501); die Lage dort liefert die Karte `S` (§4.4 N2). **M2 heisst ab N2 „Verursacher-Töpfe“:**

**Töpfe** — je einer um jeden Aufruf bzw. Block in `loop()` (`ESP-uclock.ino:1548-1582`, ✔ SW am Stand `a803e98`) und
einer für die Zeit zwischen zwei Durchläufen:

| Topf | umschliesst | Zeile |
|---|---|---|
| `wif` | `wifi_check_if_started()` | `:1548` |
| `srv` | `http_server_loop()` | `:1549` |
| `ota` | `http_ota_restart_poll()` | `:1550` |
| `upd` | `update_cache_poll()` | `:1551` |
| `udp` | `udp_server_loop()` | `:1552` |
| `ntp` | `ntp_poll_time()` | `:1553` |
| `var` | `var_sync_check()` | `:1555` |
| `ovr` | Block der Überlaufabfrage | `:1557-1576` |
| `hlg` | `esp_heap_log()` | `:1578-1580` |
| `stm` | `stm_bridge_poll()` | `:1582` |
| `sys` | vom Ende eines Durchlaufs bis zum Anfang des nächsten — Core (`loop_wrapper`), SDK, lwIP | — |

**Messung** (nur in W): `ESP.getFreeHeap()` — O(1), liest den Zähler der freien Blöcke (`Esp.cpp:220-222`,
`umm_local.c:186-191`, ✔ SW) — unmittelbar vor und nach jedem Topf; Änderung = nachher − vorher. Je Topf:

| Wert | Bedeutung |
|---|---|
| `netto` | Summe aller Änderungen in W, vorzeichenbehaftet; negativ = dauerhaft belegt |
| `rückgang` | grösster einzelner Rückgang (vorher − nachher > 0) in W, in Byte |
| `ms` | Millisekunden seit t0 beim grössten Rückgang; `-`, wenn es keinen gab |

Die Messung selbst belegt nichts und gibt die CPU nicht ab. Erfassen der Ereigniskarten am Anfang eines Durchlaufs
(§4.4 N2) ändert `free` nicht und fällt zwischen `sys` und `wif`.

**Summenprobe:** Summe der elf `netto` = `free1` − `free0`, **exakt** — `free0` am Anfang des ersten Durchlaufs, `free1` am
Anfang des ersten Durchlaufs nach W. Die Summe ist eine Teleskopsumme; sie reisst nur, wenn ein Weg in `loop()` in keinem
Topf liegt. Eine Abweichung ist ein Fehler des Instruments.

**Ausgabe** (nach W, eine Zeile je Sekunde, §4.1 N2):

```
- heap topf kopf t0=<ms> w=<ms> n=<durchläufe> free0=<n> free1=<n> us=<n>
- heap topf <i>/4 <name>=<netto>/<rückgang>@<ms> <name>=… <name>=…
```

`t0` = `millis()` am Anfang des ersten Durchlaufs (Bezug aller `ms=` dieses Pakets); `w` = Länge von W (60'000 bzw.
25'000, dokumentiert Ent-C5); `n` = Zahl der Durchläufe in W (zeigt nebenbei, ob `loop()` in W normal lief); `us` =
längster Blockgang seit dem Start (AKI.9). Elf Töpfe auf vier Zeilen (3 + 3 + 3 + 2). **Längste Form** (◐ gezählt): Kopf
84 Zeichen; Topf-Zeile mit drei Einträgen der Form `wif=-2147483648/12720@60000` 99 Zeichen — drei statt vier je Zeile,
weil `netto` 32 Bit hat.

**Vorbehalt (✔ FA, ✔ SW):** Ein Topf misst, was **während** seines Aufrufs belegt wird — auch lwIP-Rückrufe, die bei einer
CPU-Abgabe darin laufen: `optimistic_yield(1000)` in `accept()` (Topf `srv`, §2.1 N2), `Serial.flush()` (Töpfe `hlg`,
`ovr`, gegebenenfalls `stm`), `delay()`. `sys` ist deshalb nicht der einzige Ort für SDK/lwIP. **Arm B** — ohne jede
Anfrage in W — liefert das klare Bild.

**Vorhersagen** (für §5.4 N2): Nach **L-b′** zeigt in A, B und E derselbe Topf ein `netto` um −504 und einen `rückgang`
≥ 400 bei `ms` rund 3'000–8'000. Nach **K7** ist dieser Topf `stm` mit `ms` um 7'300 (+7,43 s nach der Startzeile minus
0,11 s). Nach **K4** ist es `sys` (oder `srv`/`hlg` über die Abgabe). Nach **L-c** gibt es in B keinen solchen Rückgang in
W, und in E liegt er in `srv` beim `ms` der ersten Anfrage.

**Abfolge am Anfang jedes Durchlaufs in W** (◐ spec-writer; Reihenfolge der Schritte 2–5 beliebig, solange sie vor `wif`
liegen): (1) `free` lesen, `sys` abschliessen; (2) im ersten Durchlauf `S`; (3) `F` prüfen; (4) `X` prüfen; (5) `T`
prüfen; (6) dieser `free`-Wert ist zugleich „vorher“ für `wif`.

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
Anfrage gebraucht wird, entscheidet der Nachtrag nach V.3 **[offen: V.3]** (→ N2).

**Nachtrag N2 — die Tabelle oben ist überholt (N2)** (Erfassungspunkte von `S` und `F`, Karte `U`, Begründung zu `F`
„bis zum nächsten `accept()`“, §2.4 N2). Ereigniskarten ab N2, je Start höchstens einmal, jede am **Anfang** eines
`loop()`-Durchlaufs erfasst (§4.3 N2, Abfolge), ausgegeben nach W bzw. — tritt das Ereignis nach W ein — im nächsten
freien Sekundentakt:

| Anlass | Bedingung | bis | warum |
|---|---|---|---|
| `S` | erster `loop()`-Durchlauf; `ms=0`; ihr `free` heisst `free(S)` | — | Lage am Ende von `setup()` (gleichwertig, ✔ FA: dazwischen liegt nichts) — Grundlinie für A, B, E |
| `F` | erster Durchlauf, vor dem `http_server_loop()` nach der Annahme der ersten Anfrage mindestens einmal **ohne neue Annahme** gelaufen ist | 180 s nach t0 | Lage, wenn vom Verkehr nur der PCB übrig ist (`ClientContext` frei, §2.4 N2) — zeigt, ob der TIME_WAIT-PCB im kleinen Bereich liegt (Vorbedingung von L-b′) |
| `X` | erster Durchlauf mit `free < free(S) − 400`, während `tcp_active_pcbs` leer ist (TIME_WAIT zählt nicht als aktiv) | Ende von W | Lage im Augenblick der Hintergrundbelegung; `ms` sagt, wann. Zusammen mit dem einzigen Satz Wegmarken (§4.5 N2) |
| `T` | erster Durchlauf, an dessen Anfang `tcp_tw_pcbs` leer ist, nachdem die Liste am Anfang des vorigen Durchlaufs mindestens einen Eintrag hatte | 180 s nach t0 | Lage, wenn der TIME_WAIT-PCB der frühen Anfrage abgelaufen ist — prüft „die 504 Byte liegen vorn im ehemals grossen Block“ ohne Verkehr dazwischen |

- **`U` entfällt** (✔ SW: Update-Cache frühestens 60 s nach dem Start, `http.cpp:13935`; nach V.3 kaum Gewicht).
- **`F` braucht eine Lesefunktion aus `http.cpp`** — den Zähler der Annahmen (`http_accept_count`, `http.cpp:679`) oder den
  Merker der ersten Anfrage (`:680`); Wahl beim `esp-developer`. Mit dem Zähler lässt sich „ohne neue Annahme“ direkt
  prüfen.
- **Schwelle 400 bei `X`** (Vorschlag des Analysten): Ein TIME_WAIT-PCB kostet rund 192 Byte, zwei 384 — knapp unter der
  Schwelle. Ein Fehlauslösen durch drei TIME_WAIT-PCBs wäre an den Wegmarken erkennbar (`tcp` zählt sie; Rückgang ≈
  Zahl × 192); in W ist es ohne Folge-Verkehr (Ent-C5 = A) nicht zu erwarten.
- **`T` bei Ent-C5 = B:** tritt in A und E praktisch nicht ein (die TIME_WAIT-PCBs der Folge halten die Liste bis über
  +180 s besetzt).
- **`F` in Arm B** fällt bei Ent-C5 = A auf den Beginn der Folge (+150 s) und zeigt dort nur, dass die Folge begonnen hat.

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

**Nachtrag N2:**

- **„Bei `S` und bei `F` je eine Zeile“ ist überholt (N2).** Ab N2 genau **ein Satz** je Start: erfasst mit der Karte `X`
  (Anlass `X`), oder — ist `X` bis zum Ende von W nicht eingetreten — am Ende von W (Anlass `W`). Begründung (◐
  spec-writer): Die dauerhaften Objekte (Listen-PCB, UDP-PCBs und -Kontexte, DHCP, `EEPROM`, Wetter) werden nicht
  verschoben — umm verlegt keine Belegung —, liegen bei `X` also dort, wo sie bei `S` lagen. Der TIME_WAIT-PCB der frühen
  Anfrage lebt 120 s und ist bei `X` (+3 bis +8 s) noch da. Ein Satz statt je einem bei `S`, `F` und `X` spart rund
  96 Byte BSS (2 × 48, §4.6 N2).
- **`tcp` zuerst.** Bei `X` besteht keine aktive Verbindung; die `tcp`-Einträge sind dann genau die PCBs in `tcp_tw_pcbs`
  (gelesen wie `http.cpp:745`), und genau sie trägt das Modell. Damit die Längengrenze sie nicht abschneidet, steht die
  Liste vorn:

  ```
  - heap marken <X|W> tcp=<pcb>:<zustand>/<ctx>,… lst=<blk> wet=<blk> dhcp=<blk> eep=<blk> udp=<pcb>/<ctx>,…
  ```

  Ein PCB in TIME_WAIT hat keinen `ClientContext` mehr; `ctx` steht dann als `-`. Übrige Regeln wie N1 (`-`, `+<n>`,
  ≤ 119, keine Gegenstelle). Kapazität im BSS: vier `tcp`- und vier `udp`-Einträge; was darüber hinausgeht, zählt `+<n>`.
- **Grössen zur Zuordnung über die Länge** (Tabelle N1): Ein Eintrag von 63 Blöcken (504 Byte) entspricht keinem einzelnen
  Objekt der Liste; ein UDP-PCB mit Kontext sind 16 Blöcke, ein TCP-PCB 24.

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

**Nachtrag N2 — Rechnung für den Umfang nach N2** (◐ spec-writer, Feld für Feld; die Tabelle oben und die Zahl 290 aus N1
sind **überholt (N2)**; der `esp-developer` rechnet in Task 1.3 nach, **bevor** gebaut wird):

| Posten | Inhalt | Byte BSS |
|---|---|---|
| Karten `m`, `S`, `F`, `X`, `T` | je 11 × 16 Bit (`ent` 2, `fsum`, `gross` 2, `zweit` 2, `davor` 2, `danach` 2), `ms` 32 Bit, Zustand/`abbruch` 8 Bit, Ausrichtung auf 4 ⇒ 28; × 5 | 140 |
| Wegmarken, ein Satz | `lst`, `wet`, `dhcp`, `eep` je 16 Bit (8); `tcp` 4 × (PCB 16, Kontext 16, Zustand 8) = 20; `udp` 4 × (PCB 16, Kontext 16) = 16; Zähler und Anlass 4 × 8 Bit (4) | 48 |
| Töpfe | 11 × (`netto` 32 Bit, `rückgang` 16 Bit, `ms` 16 Bit — W ≤ 60'000) | 88 |
| Zustand der Töpfe | `t0` 32, `n` 32, `free` vorher 16, `free0` 16, `free1` 16, `us` 16 (Bit) | 16 |
| Zustand der Ereignisse | TIME_WAIT-Zahl im vorigen Durchlauf, Merker (`S`/`F`/`X`/`T`, W vorbei), Stand des Annahmezählers für `F` | 4 |
| Takt der Ausgabe | letzte Ausgabe 32 Bit, Zeiger und offene Zeilen je 8 Bit, Ausrichtung | 8 |
| **zusammen** | | **304** |

- **Unter Ent-C2 (≤ 320), 16 Byte Luft.** Ein Abgleich Posten für Posten gegen die 290 Byte aus N1 ist nicht möglich — die
  Aufschlüsselung des Berichts 1.1 liegt nicht im Repository. Die Rechnung oben steht für sich und wird in Task 1.3
  nachgerechnet.
- **Kürzungsreihenfolge, falls die Nachrechnung über 320 liegt** (AKI.9 N2): **1.** `netto` auf 16 Bit mit Sättigung bei
  ±32'767, sichtbar als Grenzwert (−22); **2.** Karte `F` (−28) — ihre Hauptaussage, wo der TIME_WAIT-PCB liegt, steckt
  auch in Karte `X` und den Wegmarken bei `X`; **3.** Karte `T` (−28) — ihre Aussage liefert in Ruhe auch die `m`-Karte am
  Ruhewert, mit Verkehr dazwischen. Reicht das nicht ⇒ anhalten und vorlegen.
- **Brücke je Start:** Kopf, vier Topf-Zeilen, `S`, ein Satz Wegmarken, dazu je nach Arm `F`, `X`, `T` — **8 bis 11
  Zeilen**, höchstens rund 1,3 KB, frühestens ab dem Ende von W und eine je Sekunde. Im Betrieb unverändert eine
  Kartenzeile je Minute (rund 100 Byte).
- **Blockgänge je Start:** höchstens vier (`S`, `F`, `X`, `T`), dazu einer je Minute; „elf je Start“ oben ist **überholt
  (N2)**.
- **Laufzeit der Töpfe:** 22 Aufrufe von `ESP.getFreeHeap()` je Durchlauf, nur in W.

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

**Nachtrag N2:** M2 meint ab N2 die Töpfe. Mit `ESP_HEAP_MAP` 0 ist `loop()` Byte für Byte wie in 3.2.41. Die Töpfe-Rechnung
und die Bedingungen von `F`, `X`, `T` stehen als reine Funktionen in der eigenen Übersetzungseinheit, damit der Prüfstand
sie ohne Gerät mit synthetischen Folgen füttern kann (§4.8 N2); in `loop()` stehen nur die Messpunkte.

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

**Nachtrag N2 — überholt:** Fall 14 und die Sabotage „`srv=` fehlt“ (die `start`-Zeile entfällt); in Fall 13 gilt die
Zeilenform aus §4.5 N2 (`tcp` zuerst). **Zusätzliche Fälle (N2):**

15. Töpfe an einer synthetischen Folge von `free`-Werten je Topf und Durchlauf: `netto`, `rückgang` und `ms` exakt; `ms`
    gehört zum **grössten** Rückgang; ohne Rückgang `@-`; Summenprobe hält (AKI.13 (a));
16. statisch: jeder der zehn Aufrufe bzw. Blöcke in `ESP-uclock.ino:1548-1582` liegt in genau einem Topf, dazu `sys`;
    nach dem Ende von W keine Messung mehr;
17. `X` an synthetischen Folgen (`free`, aktive PCBs, TIME_WAIT-PCBs, `ms`): genau im ersten Durchlauf mit
    `free < free(S) − 400` ohne aktive Verbindung; nicht bei aktiver Verbindung; nicht nach W; höchstens einmal
    (AKI.14 (a));
18. `T`: genau beim ersten Übergang der TIME_WAIT-Zahl von ≥ 1 auf 0; nicht bei 0 → 0; nicht nach 180 s; höchstens einmal;
19. `F`: erst im Durchlauf nach einem Durchlauf ohne neue Annahme, nicht im annehmenden; nicht nach 180 s;
20. keine Zeile des Instruments in W; nach W höchstens eine je Sekunde und keine im Durchlauf der Heap-Zeile; keine
    `m`-Karte zu einer Heap-Zeile in W;
21. längste Formen: Karte mit `ms=` 118, Topf-Zeile 99, Kopf 84, `marken` ≤ 119 — je ≤ 119;
22. statisch: `setup()` gegenüber 3.2.41 unverändert; genau ein Satz Wegmarken je Start (`X` oder `W`); keine Zeile
    `- heap start`, keine Karte `U`.

**Zusätzliche Sabotagen (N2), jede ⇒ FEHL:** ein Aufruf ohne Topf (Summenprobe reisst); Messung nach W fortgesetzt; Topf
`sys` weggelassen; `ms` beim ersten statt beim grössten Rückgang; `X` ohne Prüfung der aktiven Verbindungen; `X` ohne
Abstand 400; `X` nach W; `T` bei jedem Rückgang der TIME_WAIT-Zahl; `T` nach 180 s; `F` im annehmenden Durchlauf; eine
Ausgabe in W; Wegmarken zweimal je Start.

**Werkzeugseitig (Lead, Task 1.6, nicht c80i):** Gegenproben zur Summenprobe im Werkzeug der Arme (AKI.13 (b)) und zu
„nicht eingetreten“ bzw. fehlender `marken`-Zeile (AKI.14 (c)), je mit Meldung in Konsole und Mitschrift.

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

**Nachtrag N2:** Ent-C1 = A bleibt. Die Töpfe brauchen den inneren Aufbau nicht — `ESP.getFreeHeap()` ist öffentlich —;
für F2 (`davor` von `gross`, Wid-2) bleibt A nötig.

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

**Nachtrag N2:** In `ESP-uclock.ino` wird ab N2 nur noch `loop()` (CRLF) und der Bereich um `ESP_HEAP_LOG` geändert,
nicht mehr `setup()`. Vor dem Patch erneut feststellen.

---

## 5. Stufe 2 — Messung (auf Release I)

### 5.1 Arme und Reihenfolge

Arme A, B, E wie in §3, Reihenfolge **A B E · E A B · B E A**. Arm D (OTA sofort) ist in Stufe 2 nicht nötig — L499 zeigt
OTA als gleichgültig; er kommt in der Abnahme von Stufe 3 zurück (AKA.3). In allen Armen ab +30 s Smoketest und
`check-pwa.sh`, Ruhewert ≥ 150 s danach mit `tcp=0/0/0/0/0/0`. Das Werkzeug sammelt je Lauf die Zeilen `start`,
`karte`, `marken` in die Mitschrift (AKM.2).

**Nachtrag N2:**

- **Beginn der Folge nach Ent-C5** — „In allen Armen ab +30 s“ oben gilt nur bei Ent-C5 = B. **Vorschlag A: Folge ab
  +150 s** nach der Startzeile. Begründung (◐ spec-writer): (1) Die Töpfe messen 60 s ab t0; Verkehr der Folge in W
  überdeckte in `srv` und `sys` den gesuchten Rückgang mit eigenen Belegungen (Empfangspuffer, PCBs) und verfälschte die
  Nettowerte um die TIME_WAIT-PCBs der Folge. (2) Karte `T` braucht den ungestörten Ablauf des TIME_WAIT der frühen
  Anfrage (rund +120 bis +123 s); die PCBs der Folge hielten `tcp_tw_pcbs` sonst bis über +180 s besetzt. (3) `X` fiele bei
  Verkehr nach +30 s leichter fehl. Preis: rund 7 statt 5 min je Lauf, und die Arme sind nicht mehr deckungsgleich mit V.2 —
  deshalb die Rückfalldiagnose in AKM.1 N2.
- **Arm B** heisst ab N2 „erste Anfrage mit der Folge“ — bei Ent-C5 = A ab +150 s (sauber nach M0 und L501).
- **Arm E** wie in V.2, gültig nur mit genau einer Anfrage **bis 3,2 s** nach der Startzeile (oberer Rand der
  zerstückelten E-Läufe, L500; ab 8,1 s sauber, L501). Die Grenze 5,9 s aus AKV.2 ist für Stufe 2 **überholt (N2)**.
- **Zwei Läufe RST** nach den neun (Verbindung nach 3,1 s aufbauen und per RST abbrechen, ohne Daten; im Werkzeug `R`).
  Sie gehen nicht in AKM.1 ein und beantworten F6.
- **Reihenfolge:** A B E · E A B · B E A · RST RST. Das Werkzeug sammelt je Lauf `topf`, `karte`, `marken` (das Wort
  `start` oben ist **überholt (N2)**) und prüft Zählsoll und Summenprobe (AKM.2 N2).

### 5.2 Ruhewert

Gültig nur an einer Heap-Zeile ≥ 150 s nach dem Ende der Folge mit `tcp=0/0/0/0/0/0` und ohne Flash-Schreiben,
Wetter-Overlay oder kalten `update_status` dazwischen — jede Ausnahme gekennzeichnet. Die Kartenzeile `m` derselben
Minute gehört dazu.

**N2:** unverändert; bei Ent-C5 = A beginnt die Folge bei +150 s, der Ruhewert liegt damit frühestens rund 6 min nach der
Startzeile.

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

**Nachtrag N2 — Rangfolge F3 vor F2 vor F4 vor F1** (V.3). Die Spalte „gelesen an“ oben ist für F2, F3 und F5 **überholt
(N2)**, ebenso der N1-Satz zu F3 („s0–s8“). Ab N2:

| Rang | | Frage | gelesen an | Vorhersage nach L-b′ |
|---|---|---|---|---|
| 1 | **F3** | Wann und in welchem Topf entsteht die dauerhafte Belegung? | Töpfe (`netto`, `rückgang`, `ms`), Karte `X` (`ms`); **Arm B zuerst** (kein Verkehr in W) | in A, B und E derselbe Topf mit `netto` um −504 und `rückgang` ≥ 400 bei `ms` rund 3'000–8'000; `X` in diesem Zeitraum |
| 2 | **F2** | Welcher Eintrag trennt? Entspricht er einer Wegmarke? | Karte `m` am Ruhewert; Karte `T`; Wegmarken `X` | in A und E ist `davor` von `gross` ein belegter Eintrag von der Länge des Fehlbetrags (504); in B liegt ein gleich langer belegter Eintrag im kleinen Bereich, nicht an `gross`; bei `T` liegt er schon vorn im grossen Block |
| 3 | **F4** | Gleich viele belegte Einträge in Ruhe, A gegen B? | `ent` am Ruhewert | gleich (Verschiebung, keine zusätzliche Belegung) |
| 4 | **F1** | Wo liegen `gross` und `zweit` in Ruhe, A gegen B? | Karte `m` am Ruhewert | — |
| — | **F5** | **erledigt** (L500: E wie A); unter dem Instrument mitgemessen, ohne eigene Frage. „`u` und `p` je Schritt s2–s8“ entfällt | — | — |
| — | **F6** (neu) | RST: Wo landet die Hintergrundbelegung ohne TIME_WAIT, und was sind im Fall 6'928 die 176 Byte im grossen Block und die 56 Byte mehr? | Töpfe, `X`, Wegmarken, `m` am Ruhewert der RST-Läufe | informativ; keine Bedingung für eine Zuordnung |

Zusätzlich bei F3: die Lage bei `F` — liegt der TIME_WAIT-PCB im kleinen freien Bereich (Vorbedingung von L-b′)?

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
Bedingung von Z2 schärft (etwa für die Hintergrundbelegung bis +30 s), bleibt offen **[offen: V.3]** (→ N2).

**Nachtrag N2 — Zuordnung ab N2.** Die Tabelle oben gilt mit diesen Änderungen; **Z1 ist überholt (N2)** — L-a und L-a′
tragen nicht (L500, V.3). Zuerst werden die beiden Widerlegungsregeln aus V.3 geprüft, je Lauf und mit Kennzeichen:

| Regel | trifft zu, wenn | Folge |
|---|---|---|
| **Wid-1** — Hintergrundbelegung widerlegt | (a) in **keinem** B-Lauf zeigt ein Topf in den ersten 30 s nach t0 einen `rückgang` ≥ 400 **und** `X` tritt in B in den ersten 30 s nicht ein; **oder** (b) in einem E-Lauf ist `netto` des Topfs `srv` ≤ −400, und sein grösster Rückgang liegt höchstens 1 s neben dem `ms` der ersten Anfrage | L-b′ trägt nicht. Zu prüfen ist **L-c** (Lazy-Init beim ersten Aufruf): In B müsste der Rückgang dann bei der ersten Anfrage nach W kommen |
| **Wid-2** — Abschneiden vom Anfang widerlegt | in mindestens einem A-Lauf ist an der `m`-Karte des Ruhewerts die Länge von `davor` (des belegten Eintrags vor `gross`) **nicht gleich** dem Fehlbetrag des Laufs — kleinstes `max` der B-Läufe derselben Reihe minus `max` des Laufs, beides Blöcke × 8, **exakt** —, oder `davor` ist `-` | „die 504 Byte liegen vorn im ehemals grossen Block“ trägt nicht; das Verdrängungsmodell in dieser Form ist falsch. Für E wird dasselbe berichtet (◐), ohne Regelwirkung |

**Zuordnungen ab N2:**

| Zuordnung | Bedingung ab N2 | Folge |
|---|---|---|
| **Z2** — L-b′ belegt | Wid-1 und Wid-2 treffen nicht zu; in B und A trägt **derselbe Topf** die Belegung bei vergleichbarem `ms` (✔ je Lauf); der Trenner ist `davor(gross)` in der Länge des Fehlbetrags (✔). **Benannt** heisst: Topf, Zeitpunkt und Länge; die Identität des Objekts (Wegmarke) ist **nicht** Bedingung | Nachtrag A.1 mit Optionen, Ent-C3. Ist der Verursacher nur als Topf `sys` (oder über eine Abgabe in `srv`/`hlg`) benannt, sagt der Nachtrag das ausdrücklich und bietet neben Optionen am Mechanismus eine Option „erst identifizieren“ an |
| **Z2** — L-c belegt | Wid-1 (b) trifft zu, und in B kommt der Rückgang ≥ 400 bei der ersten Anfrage nach W im Topf `srv` | Nachtrag A.1 mit Optionen, Ent-C3 |
| **Z3** — nicht zugeordnet | Wid-1 oder Wid-2 trifft zu, und weder L-b′ noch L-c deckt die Zahlen; oder A, B, E zeigen die Belegung in verschiedenen Töpfen ohne erkennbaren Grund | Nachtrag zum Instrument, Release I′, Stufe 2 wiederholt — **keine Abhilfe** |
| **Z4** — Instrument stört | AKM.1 misslingt **und** die Rückfalldiagnose aus AKM.1 N2 entlastet den Beginn der Folge; **oder** die Summenprobe reisst am Gerät | Stufe 2 hält an; Nachtrag zum Instrument |

F6 (RST) hat keine Regelwirkung; ihr Ergebnis gehört in den Nachtrag A.1, weil eine Abhilfe am TIME_WAIT allein den
RST-Fall mit 176 Byte nicht erklären würde.

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

**N2:** „Nach Z1 oder Z2“ heisst ab N2 „nach Z2“ (§5.4 N2). Das Verdrängungsmodell legt Richtungen nahe — am TIME_WAIT der
frühen Anfrage, am Zeitpunkt der Hintergrundbelegung, an ihrem Verursacher —, aber keine davon ist belegt, und der
RST-Fall (176 Byte ohne TIME_WAIT) zeigt, dass das Modell nicht alles erklärt. Die Regel dieses Abschnitts gilt
unverändert.

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

**N2:** M2 meint in Release I die Töpfe (§4.3 N2). Am Releaseschnitt ändert N2 nichts: nur ESP, nur `ESP_VERSION`.

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
>
> **N2:** Die Töpfe **umschliessen** die Aufrufe in `loop()`, ohne Reihenfolge oder Aufrufe zu ändern; `setup()` bleibt
> unverändert. Am Webserver ändert sich nichts — von den zwei Lesefunktionen in `http.cpp` aus N1 bleibt eine (für `F`);
> `tcp_tw_limit()` und der Umgang mit TIME_WAIT bleiben, auch wenn das Modell dort ansetzt. Der Verdacht auf die STM-Zeile
> (K7) wird auf der ESP-Seite im Topf `stm` geprüft; `src/**` bleibt unberührt.

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
>
> **N2:** **UART je Start:** 8 bis 11 Zeilen, höchstens rund 1,3 KB, erst **nach** dem Startfenster W und eine je Sekunde;
> **in W keine einzige Zeile des Instruments** — die Startphase der Brücke, in der der STM CAP/FIRMWARE auswertet und
> `setup()` rund 7 s nicht liest, bleibt damit ganz frei. Im Betrieb unverändert. **Blockgänge:** je Start höchstens vier
> statt elf, dann einer je Minute. **Töpfe:** 22 Aufrufe von `ESP.getFreeHeap()` je Durchlauf, jeder O(1) ohne Sperre
> (`umm_local.c:186-191`), nur 60 s lang; danach kein Aufwand. **Watchdog:** kein neuer Wartepfad; nichts im Instrument
> wartet. Die `start`-Zeilen aus N1 entfallen.

**Secure by design** — Fremddaten? Credentials in Logausgaben?

> Die Zeilen enthalten Blockindizes, Längen, Zähler und Zustände — **keine IP-Adresse, keinen Port, keinen Netznamen,
> kein Geheimnis** (AKI.1 (g); für TCP-PCBs nur Index und Zustand, AKI.6). `/api/stm32_log` ist im LAN lesbar; die
> Inhalte verraten nichts, was ein Angreifer nutzen könnte (der ESP8266 hat keine Adressraum-Zufallsverteilung, die
> Indizes schützen nichts). Kein neuer Endpunkt, keine Konfiguration.
>
> **N1:** Die Wegmarken `eep` und `dhcp` nennen nur den **Blockindex** ihres Objekts, nie dessen Inhalt — keine Adresse,
> kein Byte aus dem `EEPROM`-Puffer.
>
> **N2:** Die `topf`-Zeilen tragen nur Topfnamen, Byte-Zahlen und Millisekunden; die Wegmarken bei `X` nennen für die
> TIME_WAIT-PCBs nur Index und Zustand, nie die Gegenstelle — gerade dort, wo die Gegenstelle das Werkzeug des Leads wäre.

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
>
> **N2:** **Summenprobe der Töpfe** — Summe der Nettowerte = `free1` − `free0`, exakt; ein Weg ausserhalb jedes Topfs fällt
> auf, statt still zu fehlen, und das Werkzeug meldet die Abweichung mit Exit ≠ 0 (AKI.13). **Jedes Ereignis ist
> befristet** (`X` bis zum Ende von W, `F` und `T` bis 180 s) und höchstens einmal; die Merker sind nach dem Ereignis oder
> der Frist aufgelöst, und ein Ereignis nach der Frist hinterlässt nichts. **Wegmarken genau einmal** (`X` oder, wenn X
> ausbleibt, `W`) — das Werkzeug kann ihr Fehlen als Fehler werten statt als „nicht eingetreten“. **Fehlauslösen von `X`**
> durch mehrere TIME_WAIT-PCBs ist an den Wegmarken erkennbar, nicht still. Die Merker-Liste oben gilt ab N2 mit `S`, `F`,
> `X`, `T`, `marken`; `U` entfällt.

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

**Nachtrag N2 — die Zeilen oben zu `ESP-uclock.ino` und `http.cpp` sind überholt (N2):**

| Datei | Änderung ab N2 | Zuständiger Agent |
|---|---|---|
| `ESP8266/ESP-uclock/ESP-uclock.ino` | `ESP_HEAP_MAP`; **nur in `loop()`:** Messpunkte der Töpfe um die Aufrufe `:1548-1582` (Reihenfolge unverändert), am Anfang jedes Durchlaufs `sys` abschliessen und `S`/`F`/`X`/`T` prüfen, Ausgabe nach W; `m`-Karte zur Heap-Zeile ausserhalb W. **`setup()` unverändert** | `esp-developer` |
| `ESP8266/ESP-uclock/http.cpp` | **eine** exportierte Lesefunktion für `F` (Annahmezähler oder Merker der ersten Anfrage). Entfallen: die Funktion für `_unclaimed`, das Ereignis `U` | `esp-developer` |
| neue Übersetzungseinheit | zusätzlich: Töpfe-Rechnung und Bedingungen von `F`, `X`, `T` als reine Funktionen; Lesen von `tcp_active_pcbs` und `tcp_tw_pcbs` (wie `http.cpp:733-753`) | `esp-developer` |
| `tools/startprobe.py` | Beginn der Folge als Einstellung (Ent-C5); Sammeln von `topf`/`karte`/`marken` je Lauf; Zählsoll nach AKI.10 N2; Summenprobe mit Exit ≠ 0; „nicht eingetreten“ für `F`, `X`, `T`; Arm E gültig bis 3,2 s; je Gegenprobe | Lead |
| `tools/watch-log.sh` | `topf`-Zeilen und Karten `X`, `T` ohne Meldung; `abbruch=` weiter als Warnung | Lead |

Der Satz „`wifi.cpp` wird nicht geändert: s2 wird … in `setup()` erfasst“ gilt ab N2 mit anderer Begründung: In `setup()`
wird gar nichts mehr erfasst.

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
| **N2:** Startschritte s0–s8 beibehalten oder auf s8 kürzen | treffen den Schaden nicht (V.3: bis zum Ende von `setup()` sind alle Arme gleich); kosten 126 Byte BSS, die die Töpfe brauchen; `S` im ersten Durchlauf ist gleichwertig zu s8 |
| **N2:** Töpfe dauerhaft statt nur in W | Aufwand in jedem Durchlauf ohne Nutzen nach dem Start; Nettowerte liefen über; das Instrument sucht ein Startereignis |
| **N2:** Ausgabe des Instruments schon in W | jede Ausgabe gibt in `Serial.flush()` die CPU ab und schöbe lwIP-Rückrufe in den Topf der Ausgabe |
| **N2:** Wegmarken bei `S`, `F` und `X` | dreifaches BSS (rund 144 statt 48 Byte) für Objekte, die sich nicht bewegen; der TIME_WAIT-PCB ist bei `X` noch da |
| **N2:** Karte `U` | Update-Cache frühestens 60 s nach dem Start (`http.cpp:13935`), ausserhalb des Fensters; nach V.3 kaum Gewicht |
| **N2:** `X` am Rückgang von `max` statt von `free` | `max` fällt auch ohne neue Belegung, wenn ein PCB den grossen Block anschneidet; `free` misst die Belegung selbst |
| **N2:** Belegungsursache am STM instrumentieren (K7) | ändert `src/**` und ein zweites Release; der Topf `stm` zeigt schon, ob die Bearbeitung einer STM-Zeile auf dem ESP belegt |

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
| **Q8** (neu) | **Was belegt der ESP im Fenster nach `setup()`, und braucht Stufe 1 dort Erfassungspunkte?** — `firmware-analyst` (V.3), danach `spec-writer` (Nachtrag N2, Task V.4, falls nötig); **vor Task 1.3** **[offen: V.3]** (→ N2) |

**Nachtrag N2 — Stand am 17.10.2026:**

| | Stand |
|---|---|
| Q2 | Grenze bleibt; gerechnet für N2 rund **304 Byte** mit Kürzungsreihenfolge (§4.6 N2) |
| Q3 | **zur Hälfte (✔ L / ◐ FA):** eine Hintergrundbelegung rund +3 bis +8 s nach der Startzeile, die bei einer frühen Anfrage vorn im grossen Block landet; welches Objekt, offen — Stufe 2, Töpfe und F3 |
| Q4 | **beantwortet (✔ FA):** rund 0,11 s nach der Startzeile (L501). „Gemessen wird es mit `ms` bei s8“ ist **überholt (N2)** |
| Q5 | **überholt (N2):** `u` entfällt; ein Aufbau während `setup()` ist für den Schaden nicht nötig (L500) |
| Q7 | offen — AKI.9, AKM.5; ab N2 höchstens vier Gänge je Start |
| **Q8** | **beantwortet:** In W wird im Hintergrund belegt (504 Byte, rund +3 bis +8 s; Verursacher offen, ? FA SDK/lwIP oder STM-Zeile). Stufe 1 braucht dort Erfassungspunkte — umgesetzt als Töpfe, Karten `X`/`T` und `S` im ersten Durchlauf (§3.5 N2, §4 N2) |
| **Q9** (neu) | **Ent-C5 — Beginn der Folge in Stufe 2: +150 s (A, Vorschlag) oder +30 s (B)?** Optionen in `requirements.md`, Entscheidungen; Begründung §5.1 N2 — **Du, mit der Freigabe von N2; vor Task 1.3** (W ist eine Konstante in der Firmware) |
| **Q10** (neu) | Wer belegt die 504 Byte (K4 oder K7 oder anderes)? — Stufe 2, F3; reicht der Topf nicht zur Identität, entscheidet der Nachtrag A.1 über eine Option „erst identifizieren“ (§5.4 N2) |
| **Q11** (neu) | Was ist im RST-Fall 6'928 geschehen (176 Byte im grossen Block, 56 Byte mehr)? — Stufe 2, F6; informativ |
