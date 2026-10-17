# Design — Paket 2026-10-17 „C80: bleibende Heap-Zerstückelung nach frühem Verkehr — Firmware-Ursache“

**Erstellt:** 2026-10-17, `spec-writer`, Stand `353a693`. Anforderungen: `requirements.md`; Tasks: `tasks.md`.
Kennzeichen und Kennungen wie in `requirements.md`. Momentaufnahme (DIR-006).

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

### 2.3 Drei Lesarten

| Lesart | Inhalt | Was dafür spräche | Was dagegen spräche |
|---|---|---|---|
| **L-a** | Aufbau während `setup()` **vor** `ntp_setup()`/`udp_server_setup()`; die UDP-Kontexte legen sich über die Verbindungsobjekte (K2 des Analysten) | Arm E sauber; in Arm A `_unclaimed` ≥ 1 bei s2–s4; Trenner ist ein UDP-PCB | Arm E zerstückelt |
| **L-a′** | Aufbau während `setup()` **nach** der Startzeile; eine Belegung aus `vars_init()` … `tables_init()` legt sich darüber | `_unclaimed` ≥ 1 erst bei s4–s8; Trenner entsteht zwischen s4 und s8 | Trenner liegt schon bei s4 |
| **L-b** | Belegung **nach** `setup()`, in den ersten Sekunden des Betriebs, während die frühe Verbindung lebt | Trenner erst bei `F`, `U` oder in einer `m`-Karte sichtbar; Arm E zerstückelt, obwohl seine Anfrage nach dem Ende von `setup()` kommt | Trenner liegt schon bei `S` |

**Hinweis aus M0, ◐:** Ein schlechter Start mit erster Anfrage nach 5,9 s passt schlecht zu L-a, sofern `setup()` dann
schon beendet war. Wann `setup()` relativ zur Startzeile endet, ist nicht gemessen — M2 liefert es mit `ms` bei s8. Wie
die Werkzeuge in M0 angefragt haben, steht im Protokoll nicht; der Fall ist ein Hinweis, kein Gegenbeleg.

### 2.4 Kandidaten

| | Kandidat | Herkunft | Einschätzung |
|---|---|---|---|
| K1 | Kontext des Wetter-Clients bleibt bis zum nächsten Abruf (`weather.cpp:16`, `WiFiClient openweather_client`; C77) | ◐ FA | Läuft in den Armen A und B gleich; kann den Unterschied nur im Zusammenspiel mit der frühen Verbindung erklären. Laut L499 lag vor der ersten Anfrage der schlechten Starts kein Wetterabruf |
| K2 | Früher Aufbau während `setup()` unter den UDP-Kontexten | ◐ FA | = L-a; §2.2 |
| K3 | Spät aufgebautes WLAN | ? FA | In A und B gleich; WLAN steht zwingend vor der ersten Antwort |
| K4 | Interne Belegungen von SDK/lwIP | ? FA | Zusammengefasst mit K6 |
| K5 | Legacy `/network` ohne `scanDelete()` | ? FA | In keinem Arm aufgerufen |
| K6 | Einmalige Belegungen der ersten Sekunden — Schritte nach der Startzeile, erste Antworten des STM, erster NTP-Abgleich, SDK/lwIP (DHCP, ARP) —, die sich über eine gerade lebende frühe Verbindung legen | ◐ spec-writer | = L-a′ bzw. L-b; welche Belegungen das sind, ist nicht bekannt |

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

Die Sperre wird auf **jedem** Weg gelöst, auch bei `abbruch=`.

**Zugriff auf Blockfeld und Blockzahl:** nach Ent-C1 (§4.9). Der Kontext des Haufens (`umm_heap_context_t`,
`umm_malloc_cfg.h:156`) ist erst in `umm_local.h` ausgeführt (Felder `heap` und `numblocks`, `umm_local.h:65`, `:73`,
✔ SW); die Form der Blöcke steht in `umm_malloc.cpp` (◐ FA, im Bericht 1.1 mit Fundstelle zu belegen).

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

### 4.4 M3 — Ereigniskarten

| Anlass | erfasst | warum |
|---|---|---|
| `S` | am Ende von `setup()`, im selben Augenblick wie s8 (ein Blockgang für beide) | Lage vor dem ersten `loop()` — trennt L-a/L-a′ von L-b |
| `F` | nach der Rückkehr aus dem `http_server_loop()`-Durchlauf, in dem die erste Anfrage angenommen wurde | Lage, solange die frühe Verbindung noch Spuren hat (`http_client` hält den Kontext bis zum nächsten `accept()`, §2.4) |
| `U` | nach der Zeile `- update cache nach` | Lage nach dem ersten Abgleich (N6: `frag=10` statt 4 beim Abgleich, L493) |

Je Start genau einmal; ein Ereignis, das nicht eintritt, hinterlässt keine Zeile (AKI.5).

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

### 4.7 Schalter und Rückbau

`#define ESP_HEAP_MAP 1` neben `ESP_HEAP_LOG` (`ESP-uclock.ino:618`). Mit 0 entfällt alles, auch das BSS. Ein
Übersetzungsfehler, wenn `ESP_HEAP_MAP` 1 und `ESP_HEAP_LOG` 0 ist. Bei Ent-C1 = A eine Übersetzungsprüfung auf Core
3.1.2.

**Rückbau (Ent-C4):** Release A trägt das Instrument noch — AKA.5 braucht die Karte. Im nächsten ESP-Release nach der
Abnahme von Stufe 3 gehen M2–M4 heraus; M1 geht heraus oder steht auf 0. Der Kommentarblock nennt das wörtlich, nach dem
Muster von `ESP_HEAP_LOG` (`:579-582`: „WEG DAMIT, sobald …“). `BEFUNDE.md` führt den Rückbau ab Release I als offenen
Punkt (AKZ.7) — eine Aufgabe ohne Eintrag erinnert niemanden (CLAUDE.md zu E4).

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

### 4.9 Ent-C1 — Weg für Karte und Wegmarken

| Option | Inhalt | sieht | Preis |
|---|---|---|---|
| **A** (Vorschlag) | Blockgang über den Kontext des Haufens (`umm_local.h`) und die Blockform aus `umm_malloc.cpp`, nachgebildet; Übersetzung bricht bei anderem Core ab | M1–M4 vollständig: *was* und *wann* | hängt an inneren Einzelheiten von Core 3.1.2; befristet und mit Rückbau |
| **B** | nur öffentliche Mittel: M2 mit `free`, `max`, `frag` (`ESP.getHeapStats`), `u`, `p`; M4 als rohe Adressen; keine Karte | *wann* (L-a/L-a′/L-b) | sagt nicht sicher *was*: ohne Gang ist der grosse Block nicht verortet |
| **C** | wie B, dazu eine Sondierbelegung: `malloc` in Grösse von `max`, Adresse lesen, sofort `free` | verortet den grossen Block öffentlich | greift kurz in den Haufen ein, den sie misst; ob eine Belegung aus einem Unterbrechungskontext in diesem Augenblick scheitert, ist nicht geprüft (●) |

R1.3 im Paket 2026-10-15 hat `frag=` ausdrücklich ohne privaten Kopf gebaut (L468). A weicht davon ab — deshalb die
Rückfrage an Dich. Für A spricht: Nur so lässt sich `davor`/`danach` bestimmen, und davon hängt Frage F2 ab.

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

### 5.4 Zuordnung

| Zuordnung | Bedingung | Folge |
|---|---|---|
| **Z1** — L-a bzw. L-a′ belegt | E sauber **und** in A `u` ≥ 1 an einem Schritt s2–s8 **und** der Trenner ist ein Objekt, das zwischen diesem Schritt und s8 entsteht (Wegmarke oder Zeitpunkt) | Nachtrag mit Optionen, Ent-C3 |
| **Z2** — L-b belegt | E zerstückelt **und** der Trenner wird erst bei `F`, `U` oder einer `m`-Karte sichtbar, benannt über Wegmarke oder Grösse und Zeitpunkt | Nachtrag mit Optionen, Ent-C3 |
| **Z3** — nicht zugeordnet | Trenner keiner Wegmarke zuzuordnen und nicht über den Zeitpunkt; oder E gemischt | Nachtrag zum Instrument (weitere Wegmarken), neues Release I′, Stufe 2 wiederholt — **keine Abhilfe** |
| **Z4** — Instrument stört | AKM.1 misslingt | Stufe 2 hält an; Nachtrag zum Instrument |

Jede Aussage der Zuordnung trägt ein Kennzeichen (✔ belegt an Zahl und Lauf, ◐ abgeleitet). „Ein PCB als Näherung für
seinen Kontext“ (§4.5) bleibt ◐.

---

## 6. Stufe 3 — Abhilfe

**Entscheidpunkt.** Nach Z1 oder Z2 schreibt der `spec-writer` einen Nachtrag zu dieser Spec: Beleg, Optionen mit Preis
und Risiko, betroffene Komponenten, Prüfstand. Der Lead legt Dir die Optionen per Rückfrage vor (Ent-C3). Erst nach
Deiner Freigabe des Nachtrags beginnt der `esp-developer`.

**Diese Spec legt keine Abhilfe fest.** L499 nennt als Beispiel die Reihenfolge in `setup()`; ob sie trägt, hängt am
Trenner. Eine Abhilfe ohne Beleg hat in diesem Projekt schon einmal eine plausible, aber falsche Lesart gebaut — H-CC zu
C71, widerlegt in L468.

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

**Scalable systems** — STM-Kommandos? Byte je Minute auf der UART? Unter 20 s Watchdog? Blockade des Hauptloops?

> **STM-Kommandos: keine.** Die neuen Zeilen tragen das Präfix `- ` und sind Meldungen des ESP
> (`ESP-uclock.ino:564-565`). **UART:** im Betrieb eine Kartenzeile je Minute, rund 100 Byte (1,7 Byte/s) zu den rund
> 105 Byte der Heap-Zeile; je Start einmalig rund 1,2 KB, **verteilt auf mindestens 14 s**, nie mehr als eine Zeile je
> Sekunde — der Empfangsring des STM (1'024 Byte, `UART_RXBUFLEN`) sieht nie mehr als eine Zeile am Stück. **Watchdog:**
> Der Blockgang ist auf die Blockzahl begrenzt (Wächter `it`) und kann nicht hängen bleiben. **Blockade:** je Gang
> gesperrte Unterbrechungen, gemessen als `us=`; über 1'000 µs wird vorgelegt (AKI.9). Im Betrieb ein Gang je Minute,
> beim Start elf. Die PWA pollt nicht anders als heute.

**Secure by design** — Fremddaten? Credentials in Logausgaben?

> Die Zeilen enthalten Blockindizes, Längen, Zähler und Zustände — **keine IP-Adresse, keinen Port, keinen Netznamen,
> kein Geheimnis** (AKI.1 (g); für TCP-PCBs nur Index und Zustand, AKI.6). `/api/stm32_log` ist im LAN lesbar; die
> Inhalte verraten nichts, was ein Angreifer nutzen könnte (der ESP8266 hat keine Adressraum-Zufallsverteilung, die
> Indizes schützen nichts). Kein neuer Endpunkt, keine Konfiguration.

**Stable & reliable** — Fehler ausgewertet? Stille Verwerfungen? Zustand nach Abbruch? Flags auf jedem Pfad aufgelöst?

> **Kein stilles Zurechtbiegen:** Ein beschädigter oder unerwarteter Haufen ergibt `abbruch=<grund>`, keine falschen
> Zahlen, und die Logwache meldet ihn (AKI.11). **Selbstprüfung:** `fsum` gegen `free=` und `gross` gegen `max=` decken
> einen falschen Gang am Gerät auf (AKI.3). **Sperre** auf jedem Weg gelöst, mit Sabotage im Prüfstand. **Merker:** Jedes
> Ereignis (`S`, `F`, `U`, `marken`) hat einen festen Platz, wird einmal gesetzt und bei der Ausgabe gelöscht; tritt es
> nicht ein, bleibt der Platz leer, und das Werkzeug meldet es als „nicht eingetreten“, nicht als Fehler (AKI.5). Ein
> Neustart setzt alles zurück (BSS). **Werkzeug:** fehlender Start, fehlende Anfrage oder fehlender Ruhewert ⇒ Exit ≠ 0
> mit Meldung (AKV.1) — heute endet es mit 0.

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
