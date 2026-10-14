# Design — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (A15, A56, A70, A79, A80, A81, A82, B8, B48, B55, C6, C6b, C6v, C8, C19, C46, C59, C60, C65, E36, F3)

**Erstellt:** 2026-10-13, Stand `159c9d0`. **Freigegeben** vom Nutzer am 13.10.2026 (Ent-16), samt Ent-1
bis Ent-14 und Ent-17 (`requirements.md`, „Entscheidungen"). **Gegen die Empfehlung entschieden:** Ent-11
(C6 einstufig, §8.4). **Ent-15 (Messbau für A15) ist am 13.10.2026 aufgehoben** (L432, §2.3).
**Ent-11 und Ent-12 sind am 13.10.2026 aufgehoben** (Ent-18, L439, L440, §8): Der Schutz der zerstörerischen
Aufrufe entfällt aus diesem Paket und bekommt eine eigene Spec; Release E1 entfällt, C46 und C19 gehen mit
E2, EP bleibt ein Release mit ESP und PWA — wegen C6b.
Momentaufnahme (DIR-006).

**Nachtrag 14.10.2026 — Schritt E3 (C65): freigegeben 14.10.2026.** Umfang Ent-19 (L449); §18 beschreibt die
Umsetzung, §0, §1.3, §1.4, §17.2, §17.3, „Betroffene Module", die Checkliste, „Verworfene Alternativen" und
„Versionsfolgen" sind um E3 ergänzt. Fundstellen von E3 am Code `8445a10`. **Ent-20 ist mit Variante C
entschieden** (Release Notes und `wc-list.txt` im LittleFS, Nutzerentscheid); die Festlegungen dazu trifft
§18.5a. Ent-21 bis Ent-27 sind entschieden, Ent-24 entfällt (`requirements.md`, „Entscheidungen").

**Grundlage:** Fundstellen vom `spec-writer` am Code `159c9d0` nachgelesen (`requirements.md`,
„Problem"). Technische Detailentscheide, für die kein Analyse-Agent befragt wurde, sind als **Empfehlung**
gekennzeichnet und begründet; der Umsetzer darf mit Begründung am Code abweichen und meldet es im
Taskbericht. Für E3 liegt die statische Analyse des `firmware-analyst` vom 14.10.2026 zugrunde (L449); was
der `spec-writer` davon selbst nachgelesen hat, ist in `requirements.md` Problem §18 gekennzeichnet.

**Entscheidungsvermerk 13.10.2026.** Ent-1/Ent-9 A56 zwei Byte ohne Layoutversion (§3) · Ent-2 A70 alle
vier Fälle (§4) · Ent-3 A79 (§12) · Ent-4 A80 (§5) · Ent-5/Ent-10 C8 260 Byte und Folgeeinträge (§7) ·
Ent-6 B55 API wartet (§6) · Ent-7 C19 nur L311 (§9) · Ent-8 F4 ausgelagert · ~~**Ent-11 C6 einstufig**~~ —
gegen die Empfehlung; **aufgehoben** 13.10.2026, L439 (§8.4) · ~~Ent-12 C6-Umfang samt C60~~ **aufgehoben**
13.10.2026, L439 (§8) · Ent-13 alle Gerätenachweise freigegeben, `caffeinate` (§17) · Ent-14 A70 bei
Gate-Riss auslagern (§1.2) · ~~Ent-15 Messbau A15~~ **aufgehoben** 13.10.2026, L432 (§2.3) · Ent-16
freigegeben · Ent-17 A81, A82 in S2 (§3.4, §3.5) · **Ent-18** Schutz der zerstörerischen Aufrufe
zurückgestellt, eigene Spec; E1 entfällt (§0.2, §8).
**Entscheidungsvermerk 14.10.2026 (E3):** **Ent-19** C65 als E3 vor T2 und Z, I-1 bis I-3, M-1, M-3 (§18) ·
**Ent-20 C** Release Notes und `wc-list.txt` im LittleFS (§18.5a, Nutzer) · Ent-21 B Rücksetzen nur mit
Parameter (§18.2, Lead) · Ent-22 `low=` in der Heap-Zeile (§18.2, Lead) · Ent-23 Haltegrenze K30 nach
Empfehlung (§18.9, Nutzer) · ~~Ent-24~~ entfällt, P2 läuft schon auf der Uhr (§0.1) · Ent-25 1'000/2'000 ms,
OTA ≥ 5'000 ms (§18.6, Lead) · Ent-26 beide Eingriffe in G7 (§18.8, Nutzer) · Ent-27 einstufig (§18.8, Lead)
· N-5 Freigabe der Gerätenachweise: Nutzerentscheid N-5, 13.10.2026, Gespräch.

---

## 0. Reihenfolge und Release-Schnitt

### 0.1 Regeln

| Regel | Woher | Umsetzung in `tasks.md` |
|---|---|---|
| Code des **nächsten** Releases erst nach dem **Release-Build** des vorigen | `make release-zip` baut STM, ESP und PWA-Assets zusammen (Lehre Paket 2026-10-09) | S2.1 nach S1.12, E2.0/E2.1 nach S2.22 (bzw. S3.6), EP.2 nach E2.12, P2.1 nach EP.16, E3.1 nach P2.11 |
| **Release-Build** erst nach dem **Gerätenachweis** des vorigen | R3b | S2.22 nach S1.14 (G1), E2.12 nach S2.24 (G2) bzw. S3.8 (G2b), EP.16 nach E2.14 (G4), P2.11 nach T1; E3.14 nach P2.11 — G6 siehe unten |
| **Firmware vor PWA** | DIR-002, L241 | B55: S2 → E2 → EP. C46: ESP-Teil in E2, PWA-Teil in EP. C6b: ESP-Teil und PWA-Teil im selben Release EP, ESP zuerst eingespielt |
| STM-Release-Build erst nach **grünem Matrix-Lauf** auf dem angehobenen Stand | Ent-18 des Vorpakets | S1.11, S2.21, S3.5 |
| ~~Messbauten sind **keine** Releases~~ | ~~Ent-15~~ — **entfallen**, Ent-15 aufgehoben (L432) | keine Messbauten in diesem Paket |
| Messwerte **sofort** in `BEFUNDE.md` | L424 | jeder messende Task schreibt seine Zeile, bevor der nächste beginnt (§1.4) |
| Spät geplante Gerätetests mit `caffeinate` | DIR-020, Ent-13 | Nachtteil G2, Stundenläufe G1, G4; G7, falls geplant |

**Ausnahme für P2 (Empfehlung):** B8 ist umfangreich und firmwareunabhängig. Der Lead darf P2.3/P2.4 in
einem Worktree (R6) vorziehen; in den Hauptbaum kommt es erst nach dem Release-Build von EP.

**E3 und P2 (Ent-24 entfällt, 14.10.2026):** P2 (PWA 1.4.99) ist gebaut, getaggt
(`release/3.2.34-3.2.35-1.4.99`) und nach Nutzerentscheid auf der Uhr (P2.12 erledigt). E3.1 beginnt nach dem
Release-Build von P2 (R3b gewahrt). Ist **G6** (P2.13, samt iPhone) bis zum Release-Build von E3 nicht
abgeschlossen, darf E3 trotzdem gebaut werden: Das E3-ZIP enthält dieselben PWA-Assets wie P2 (AKZ.3: keine
PWA-Änderung), und die PWA kommt ohnehin nur über `install-app.sh` auf die Uhr (DIR-017) — die Regel schützt
hier nichts, was auf dem Spiel stünde. G7 misst mit PWA 1.4.99; P2 bringt keine neuen Anfragen (Angabe des
Leads), der Vergleich mit L449 (PWA 1.4.98) bleibt damit zulässig, wird aber mit dieser Einschränkung
berichtet (AKR.14).

### 0.2 Release-Zuschnitt

| # | Release | Inhalt | Komponente | Nachweis | Warum so geschnitten |
|---|---|---|---|---|---|
| — | **Schritt 0** | E36, F3, B8-Zählung, lokaler STM-Flash (Werkzeug), Grundlinien | Werkzeug, Doku | **G0** | Die Wache trägt jeden Gerätenachweis — sie muss vor G0 halten |
| — | ~~**M0**~~ | ~~Messbau vorher~~ — **entfallen** (Ent-15 aufgehoben, L432) | — | — | — |
| 1 | **S1** | A15 | STM | **G1** | Die ISR liegt unter jedem Brückenpfad und unter dem Mitschnitt — isoliert ist ein Fehler zuzuordnen |
| 2 | **S2** | B55-STM, A56, A81, A80, A82, A70 | STM | **G2** | Getrennte Kennzeichen (`RPCR`, Seitenschreiben bei Ambilight, Marker-Weiss nach Reset, `icon: verworfen`, DCF77-Merker am Prüfstand, `night: …`); A80 und A70 berühren denselben Restore-Bereich und werden zusammen gegen die Invariante geprüft |
| (3) | **S3** | nur bei Gate-Riss: Flash-Analyse, A70 (und A82) | STM | **G2b** | Ent-14 |
| — | ~~**E1**~~ | ~~Herkunft für alle zerstörenden Wege (C6-Herkunftsteil, C6v, C60), C46-ESP, C19~~ — **entfallen** (Ent-18, L439); C46-ESP und C19 gehen mit E2 | — | ~~G3~~ | Die Herkunftsprüfung über `Sec-Fetch-*` wirkt im Browser über http nicht (L439). Der Code liegt auf `sicherung/e1-herkunft` (`2662368`) |
| 3 | **E2** | B55-ESP, C8, A79, C59, dazu **C46-ESP** und **C19** (aus E1, im Baum) | ESP | **G4** | Brücke, RAM, Netz; RAM-Messung ohne Überlagerung. C46 und C19 sind am Prüfstand belegt (L437), kosten kein RAM und haben eigene Kennzeichen (Kennung 9, `- abgewiesen: query`, Abweisung auf der Legacy-Seite) — ein Brückenfehler bleibt zuzuordnen |
| 4 | **EP** | ESP: C6b · PWA: C6b-Export, C46, B55 · Werkzeuge für C6b im selben Commit | ESP + PWA | **G5** | C6b koppelt ESP und PWA (§8.4): Ohne die neue PWA exportierte die alte leere WLAN-Schlüssel — darum **ein** Build, **ein** Tag, Einspielen direkt hintereinander |
| — | **T1** | `pwa-tester` | — | AKZ.6 | EP berührt zwei Komponenten (DIR-012) |
| 5 | **P2** | B48, B8 | PWA | **G6** | grosser Oberflächenumbau getrennt von EP — ein Weissschirm ist eindeutig zuzuordnen. Auf der Uhr seit 14.10.2026 |
| 6 | **E3** | C65: I-1, I-2, I-3, M-1 (Notes und `wc-list.txt` im LittleFS, Tabellenliste im RAM), M-3 (Nachtrag 14.10.2026, Ent-19) | ESP | **G7** | Ein eigenes Release, damit ein verändertes Heapbild dem ESP zuzuordnen ist; Instrumentierung und Massnahmen zusammen, weil die Wirkung von M-1 im selben Stand über kalt/warm messbar ist (§18.8, Ent-27) |
| — | **T2** | `pwa-tester`, dazu AKL.4 und die in T1 ausgelassenen Setter-Phasen | — | AKZ.6 | Abschluss; erst nach G7, weil T1 an der Haltegrenze stand (L449) |

**Warum C6b in EP und nicht in E2:** Fehlt der Schlüssel in `eeprom_settings`, exportiert die **alte** PWA
leere Schlüssel. Sässe C6b in E2, wäre dieses Fenster über den ganzen G4 mit seinen Stundenläufen offen; in
EP ist es Minuten lang. (Stand der Freigabe: „nicht in E1", mit demselben Grund.)

**Warum E1 entfällt (Ent-18, L439):** Browser senden `Sec-Fetch-Dest` und `Sec-Fetch-Site` über `http://`
an die Uhr gar nicht — am Gerät belegt mit Chrome 155. Die Regel „fehlende Kopfzeile = zulässig" (nötig für
curl) liess damit jeden Browseraufruf durch, auch den einer fremden Seite. Ein Release, das als Schutz
auftritt und im Browser keiner ist, erzeugt Vertrauen, das nicht gedeckt ist. *Stand der Freigabe, nicht
mehr gültig:* „Warum die Herkunftsprüfung schon in E1: Sie ändert für die PWA und die Legacy-Seiten nichts
und schliesst C60 (hoch) früh."

**Warum E3 vor T2 (Ent-19):** T1 hat an der Haltegrenze K30 die Setter-Phasen und AKL.4 ausgelassen (L449).
Ohne E3 stünde T2 vor derselben Grenze, und ohne I-1 wüsste niemand, wie tief der Heap beim Laden wirklich
fällt — die Abfragewerte sind eine Obergrenze und tragen ihre eigene Last (M-A, M-D).

**Versionen** (erwartet, setzt der `release-engineer`, R4): S1 STM 3.2.33, S2 STM 3.2.34 (S3 3.2.35),
**E2 ESP 3.2.34**, **EP ESP 3.2.35** **und** PWA 1.4.98 (`CACHE_NAME` mit), P2 PWA 1.4.99, **E3 ESP 3.2.36**
(PWA bleibt 1.4.99; Tag erwartet `release/3.2.34-3.2.36-1.4.99`). E1 (vorher ESP 3.2.34) entfällt; die
Nummer geht an E2.

### 0.3 Einspielen und Zeitfenster

| Nach | Zustand | Folge | Massnahme |
|---|---|---|---|
| S2 | neuer STM sendet `RPCR`, alter ESP | der alte ESP übergeht unbekannte Zeilen | keine |
| ~~E1~~ | ~~Herkunftsprüfung~~ — entfallen (Ent-18) | — | — |
| E2 | ESP wartet auf `RPCR` | die alte PWA zeigt bei Abweisung ihren Rückfalltext statt „gespeichert" | keine |
| E2 | ESP weist Query über 255 Zeichen ab (C46), alte PWA | ein Netzwerkimport der alten PWA mit langer SSID und langem Schlüssel bekommt Kennung 9, statt still gekürzt zu speichern; die alte PWA zeigt ihre allgemeine Fehlermeldung | keine — sichtbar statt falsch gespeichert; die neue PWA teilt ab EP auf |
| **EP, ESP eingespielt, PWA noch nicht** | Schlüssel nicht mehr in `eeprom_settings` (C6b) | ein Backup-Export der alten PWA enthielte **leere Schlüssel**, ohne Warnung; auf ein frisches Gerät zurückgespielt, fehlte das WLAN | **Minuten:** Lead spielt die PWA unmittelbar nach dem OTA ein; dazwischen keine Aktion am Gerät, **kein Backup-Export**. Die Einspielzeile sagt es |
| **E3, erste rund 60 s nach dem OTA** | Cache-Dateien noch nicht vorhanden (§18.5a) | ein `update_status` in diesem Fenster liefert `"release_notes":""` — die PWA zeigt ihren Platzhalter — und holt `wc-list.txt` wie bisher live | keine — füllt sich mit dem Startabruf; G7 beginnt erst nach AKR.20 |
| **E3** | Sendezeitgrenze 1'000/2'000 ms (Ent-25) | langsame Gegenstellen verlieren früher eine Antwort oder ein Asset (`fail=`, `fs write short`) | beobachten über I-2 in G7, G6 und T2 (§18.6) |

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
B55 +8 — zusammen rund **920 Byte**; Grenze in AKM.2 1'200. Die Kopfzeile `Sec-Fetch-Site` (vorher „einige
Dutzend") entfällt mit E1 (Ent-18). Der Ring bleibt bei 32 × 121 (Ent-10).

**E3 (Nachtrag 14.10.2026).** Ausgang L449 (ESP 3.2.35): grösster Block nach dem Start 7'744, in Ruhe nach
Last 5'808, `free` nach 150 s ohne Abfrage 7'280. Gerechnet (Empfehlung, gemessen wird je Task, AKR.8):

| Teil | statisch | Bemerkung |
|---|---|---|
| I-1 (`heap_low_*`, `httpclient_connects`) | rund 16 | |
| Ent-22 (`low=` der Heap-Zeile) | rund 8 | |
| I-2 (`fs_write_short`) | 2 | Zeilenpuffer auf dem Stack |
| I-3 (`req_max_ms`, `req_max_path[32]`) | rund 36 | |
| M-1 Tabellenliste im RAM | höchstens 256 | Grösse nach E3.0 (§18.5) |
| M-1 LittleFS-Zwischenspeicher (Ent-20 C): Schlüssel der Dateien, Auftragsmerker, Zeitstempel | rund 64 | Inhalte liegen im LittleFS, nicht im RAM |
| M-3 | 0 | |
| **Summe** | **höchstens rund 380** | Grenze AKR.8: 700 |

Das ist **dauerhaft** gebundener Heap gegen **vorübergehende** Spitzen von mehreren KB (L449). Ob der
Tausch aufgeht, zeigen AKR.12 und AKR.13 — nicht die Rechnung. Der Startabruf und das Ausliefern aus der
Datei belegen den Heap nur **vorübergehend** (Mount von LittleFS, ◐; dieselbe Grössenordnung wie jede
Asset-Auslieferung).

### 1.4 Messprotokoll in `BEFUNDE.md` (L424)

Je Release **eine** Befundzeile „Messprotokoll <Release>"; der Lead hängt jede Messung im Augenblick der
Messung an: Zeitpunkt, Instrument, Wert, Differenz, Gate ja/nein. **R3:** Messzeilen schreibt der Lead;
Befundnachführungen der `doc-writer` in eigenen Tasks — nie gleichzeitig. Die Prüfstandsbelege für C46 und
C19 stehen im Messprotokoll E1 (L437) und bleiben dort; das Messprotokoll E2 verweist darauf. Für E3 legt der
Lead in E3.0 die Zeile „Messprotokoll E3" an; dort stehen auch die Listen- und Notes-Grössen vom
Update-Server, der Platz im LittleFS, die Fundstelle der Freigabe (**Nutzerentscheid N-5, 13.10.2026,
Gespräch**) und jeder Lauf aus G7 einzeln.

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
- Neue Kennungen (`http.cpp:244-273`): **9** Query zu lang (im Baum seit E1.3, Release E2), **10**
  abgewiesen, **11** keine Antwort (E2); je ein Eintrag in **beiden** i18n-Tabellen (EP).

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

**Stand 13.10.2026 (Ent-18, L439, L440): Der Schutz der zerstörerischen Wege ist nicht mehr Teil dieses
Pakets.** §8.1, §8.2 und der POST-Teil von §8.4 und §8.5 **entfallen**; ihr Text bleibt als Stand der
Freigabe stehen. Geblieben ist **C6b** (§8.3, §8.4 neu, §8.5 neu).

- **Warum (L439, am Gerät belegt):** Browser senden `Sec-Fetch-Dest` und `Sec-Fetch-Site` über `http://` an
  die Uhr nicht (Chrome 155, alle 13 Anfragen eines Seitenaufrufs ohne diese Kopfzeilen). Eine Prüfung, die
  bei fehlender Kopfzeile zulässt, lässt damit jeden Browseraufruf durch.
- **Was offen bleibt, eine eigene Spec:** C6, C6v, C60 (L426, hoch), C61 (L438, Upload-Wege und
  `local_esp_restart`), C62 (L440, WLAN- und AP-Setter). Bis dahin ist die Uhr gegen Aufrufe fremder Seiten im
  LAN **nicht geschützt** — bewusst offen, in `BEFUNDE.md` als Entscheidung geführt.
- **Was für die Spec belegt ist (L439):** Chrome sendet bei **POST** auch über http `Origin`; ein Abgleich
  `Origin` gegen `Host` ist tragfähig. Zweiter Weg: ein Token je Neustart. Werkzeuge senden kein `Origin`.
- **Der E1-Code** (Herkunft an allen 10 Wegen, Bestandsliste, Prüfstand c6h mit 484 Fällen) liegt auf
  `sicherung/e1-herkunft` (`2662368`) — Grundlage für die Spec, nicht als Schutz zu verwenden.
- **Unverändert im Baum:** die Prüfung über `Sec-Fetch-Dest` aus ESP 3.2.4 an `maintenance_format_fs`,
  `maintenance_reset_stm32`, `maintenance_reset_eeprom` (`http_request_is_embedded_subresource()`). Sie bleibt,
  wie sie ist; sie wirkt im Browser nicht (L439), der Smoketest prüft sie per curl mit gesetzter Kopfzeile.

### ~~8.1 Herkunft (E1)~~ — entfallen (Ent-18)

*Stand der Freigabe:*

`http_request_is_cross_origin()`: wahr, wenn `Sec-Fetch-Site` gesetzt und weder `same-origin` noch `none`.
Abgewiesen (403) wird bei eingebettetem `Sec-Fetch-Dest` **oder** fremder Herkunft. Begründung: POST allein
hält fremde Formular-POSTs nicht ab, und eine Formular-Navigation trägt `Sec-Fetch-Dest: document`.
PWA und Legacy laufen vom Gerät (`same-origin`), curl sendet keine Kopfzeile — beides bleibt zugelassen.
**Grenze, bewusst:** ältere Browser ohne `Sec-Fetch-*`. *(Widerlegt durch L439: auch aktuelle Browser
senden sie über http nicht.)*

**Wege (Ent-12):** die vier Wartungsendpunkte, `fs_remove` (C6v), Legacy `/fs?action=format`,
`/fs?action=remove` (C6v), `/?action=rsteeprom2`, `/?action=rststm32` (C60) und jeder weitere Legacy-Weg zu
`stm32_reset()` oder `RESET_EEPROM_RPC_VAR` (✔ SW Kandidaten `:7542`/`:7634`, `:13612`/`:13723`; Bestandsliste
per `grep`). Der GET-Zweig fängt `Sec-Fetch-Dest` bereits ab (`:14677-14680`) und bekommt `Sec-Fetch-Site`
dazu. Legacy antwortet mit einer kurzen 403-Seite statt JSON.

### ~~8.2 POST (EP)~~ — entfallen (Ent-18)

*Stand der Freigabe:*

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
- `/api/network_secrets` per **GET** (Ent-18: der POST-Weg aus §8.2 und die Herkunftsprüfung entfallen),
  `Cache-Control: no-store`, in der URL-Liste als `network_secrets_url` (bei `:12715-12723`). *Stand der
  Freigabe: „(POST, Herkunft, `Cache-Control: no-store`)".*
- PWA: `ensureBackupExportState()` holt `network_secrets`; fehlt die Antwort, Export-Abbruch mit eigener
  Kennung in beiden Tabellen; `getNetworkBackupMeta()` liest die Schlüssel von dort.
- **Was C6b ohne Schutz noch bringt, ✔ SW:** Der ESP sendet keine CORS-Kopfzeile (`Access-Control-` kommt in
  `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` nicht vor); eine fremde Seite kann die Antwort von
  `eeprom_settings` und von `network_secrets` im Browser nicht lesen — vorher wie nachher. Der Gewinn ist, dass
  der Schlüssel nicht mehr in der Antwort des allgemeinen Einstellungsabrufs steht, den Werkzeuge und
  Testprotokolle mitlesen (`snapshot-device.sh`, `TESTPLAN-PWA.md` K11). Wer im LAN curl hat, liest ihn
  weiterhin über `network_secrets`. Wie `network_secrets` geschützt wird, klärt die Schutz-Spec (Ent-18).

### 8.4 Einstufig — jetzt wegen C6b (Ent-11 aufgehoben, Ent-18)

**Geprüft (Auftrag 13.10.2026):** Ent-11 hing an der POST-Umstellung. Ohne sie bleibt die Kopplung von ESP
und PWA trotzdem bestehen — **C6b allein braucht sie**:

- **Neuer ESP, alte PWA:** `eeprom_settings` liefert leere Schlüssel; die alte PWA übernimmt sie ohne
  Warnung in den Export (`buildNetworkBackupSettings()`, `requirements.md` Problem §7). Wiederhergestellt
  auf ein frisches Gerät fehlt das WLAN — still. Das ist der Grund, warum der ESP-Teil von C6b nicht in E2
  liegen kann (§0.2).
- **Neue PWA, alter ESP:** Die neue PWA fragt `network_secrets`, den der alte ESP nicht kennt; der Export
  bricht ab (sichtbar). Das widerspräche „Firmware vor PWA" (DIR-002) und legte den Export für das Fenster
  still.
- **Aufteilen** (`network_secrets` zusätzlich schon in E2, Export der PWA in EP als reines PWA-Release, die
  Schlüssel aus `eeprom_settings` erst in einem weiteren ESP-Release) brauchte ein drittes ESP-Release in
  diesem Paket, und Ziel 9 wäre erst damit erfüllt. **Empfehlung: nicht so** — EP bleibt einstufig.

Damit kein Zeitfenster mit halbem Stand entsteht:

1. ESP-Teil (EP.2) und PWA-Teil (EP.4–EP.6) entstehen nacheinander im Baum und gehen in **einen**
   `make release-zip`, **einen** Commit, **ein** Tag mit beiden Versionen.
2. Die Werkzeuge für C6b stellt der Lead **im selben Commit** um (AKZ.11): `snapshot-device.sh` (`key_set`
   statt Schlüssel), `preview/server.py` (`network_secrets`, `key_set`). `flash-stm.sh` und
   `smoke-device.sh` bleiben unverändert — ohne POST-Umstellung gibt es dort nichts nachzuführen.
3. Einspielen: ESP-OTA, **unmittelbar** danach `install-app.sh` — kein Gerätenachweis, keine schreibende
   Aktion, kein Backup-Export dazwischen. Danach Smoketest samt Update-Quelle, `--check`, `check-pwa.sh`.
4. **Rückweg, falls der PWA-Upload scheitert:** Die alte PWA bleibt auf dem Gerät und voll bedienbar,
   Wartungsaktionen eingeschlossen; nur ihr Backup-Export enthielte leere Schlüssel. Bis zum erfolgreichen
   Upload **kein Backup-Export** — das gehört in die Einspielzeile.

*Stand der Freigabe (Ent-11, aufgehoben):* Der ESP nimmt die fünf Endpunkte in EP **nur noch** per POST an;
die Werkzeuge `flash-stm.sh`, `smoke-device.sh`, `snapshot-device.sh`, `preview/server.py` im selben Commit
auf POST, getestet gegen Vorschau und Prüfstand c6p; Rückweg bei gescheitertem PWA-Upload: die alte PWA kann
keine Wartungsaktion mehr ausführen (405), die Legacy-Seite bleibt der Rückweg für einen STM-Reset.

### 8.5 PWA (EP)

`network_secrets` über das bestehende `apiFetch()` (GET), nur aus dem Backup-Export. Der Prüfstand zählt,
dass der Export die Schlüssel von dort holt und `eeprom_settings` keine mehr liefert (AKS.2, AKS.3).

*Stand der Freigabe, entfallen (Ent-18):* Eine Hilfsfunktion `apiPost (url, options)` an den sechs
Aufrufstellen (`app.js:5393`, `:11787`, `:11851`, `:11855`, `:11875`, `:12412`), beim Löschen über
`fs_remove_base_url` und für `network_secrets`; der Prüfstand zählt die Aufrufe nach Methode.

---

## 9. C19 — Legacy-Update-Quelle (nur L311)

An **beiden** Stellen (`http.cpp:6724-6731`, `:7520-7527`): leer oder über 63 Byte ⇒ abweisen, Meldung
auf der Seite (`alert_message`-Muster, `:4681-4683`), Wert unverändert. **Empfehlung:** dieselbe
Prüffunktion wie die API für `update_host_set`/`update_path_set` (L48). Sonst nichts am Legacy-Zweig.
**Stand 13.10.2026:** umgesetzt in E1.4, im Baum, Prüfstand c19 in S16 (L437); Release mit **E2** (Ent-18).

---

## 10. C46 — Query über 255 Zeichen

- **ESP (E2; umgesetzt in E1.3, im Baum, Prüfstand c46 in S16, L437):** in `http()` vor dem `strncpy`
  (`:13795-13798`) und im POST-Zweig vor `:14648`: `strlen (const_param) > 255` und Pfad `/api/…` ⇒ Kennung
  9, kein Handler, `- abgewiesen: query <n> Zeichen` (ohne Inhalt, L179). Legacy unverändert.
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
| G0, G4 Heap, `stm32_log`, `device_ready` mit langer Query (G4 nimmt die lesende Probe aus G3 auf) | lesend | — |
| Einspielen S1, S2, (S3), E2, EP, P2, E3 | Update | — |
| G2 Ambilight aus, STM-Reset, zurück | schreibend + Reset | Vorher-Wert |
| G2 Display aus, `weather_get_now`, Display an | schreibend, Display kurz dunkel | Display an |
| G2 Nacht: STM-Reset im Aus-Fenster mit `weather_get_now` (A70 (e)) | Reset nachts | — (Ein-Timer am Morgen) |
| ~~G3 `maintenance_reset_stm32` und Legacy `rststm32` mit Abweisungs-Kopfzeilen~~ — entfallen (Ent-18) | — | — |
| G4 zwei STM-Resets (A79), `weather_get_forecast` (C8) | Reset, Auslöser | — |
| G5 LDR-Proben, Backup-Export (~~POST/GET `maintenance_reset_stm32`~~ entfällt, Ent-18) | EEPROM | 0/4095 |
| G6 iPhone, drei Felder | Nutzer | — |
| **G7** Ruhe, PWA-Ladungen kalt/warm, `device_ready` mit `heap_low_reset=1`, `update_status`, `update_table_files` und `fs_list` per curl | lesend (Nutzerentscheid N-5, 13.10.2026, Gespräch) | — |
| **G7** Zero-Window-Probe (Ent-26 a, freigegeben) | lesend, provoziert einen Schreibabbruch | — |
| **G7** ein `weather_get_forecast` während einer Ladung (Ent-26 b, freigegeben) | Auslöser | — (Wetterlauftext läuft durch) |
| **E3, beim ersten Start nach dem OTA** | der ESP schreibt selbst zwei Dateien ins LittleFS (§18.5a) | — |
| T1, T2 `pwa-tester` mit S-Fällen | nach Testplan | nach Testplan |

### 17.3 Nie am Gerät — nur am Prüfstand

`maintenance_format_fs`, `maintenance_reset_eeprom`, `fs_remove` (auch mit nicht vorhandener Datei),
Legacy `/fs?action=format`, `/fs?action=remove`, `/?action=rsteeprom2` — **auch nicht** mit
Abweisungs-Kopfzeile: Versagt die Prüfung, sind PWA oder Einstellungen weg. Legacy `saveuphost`/`saveuppath`
leer oder überlang — versagt die Prüfung, zeigt die Update-Quelle auf den Server des Ursprungsprojekts.
NTP-Fälschung (C59), A70 (c), (d), L399 (nicht herstellbar). **Für E3:** zwei Browsersitzungen gleichzeitig
(L449: 616) und Abfragen schneller als 2/s — beides drückt den Heap gezielt unter die Grenze, ohne etwas zu
belegen, das G7 nicht auch so zeigt; die Lesart „Heap" (`snd` frei, trotzdem kurz) wird nur am Prüfstand
c65b hergestellt. **Die Cache-Dateien löschen** (`fs_remove`, Gefahrenendpunkt) und **eine neue
Release-Version herstellen** (hiesse, den produktiven Update-Server zu verändern) — beides nur am Prüfstand
c65n.

---

## 18. C65 — Heap unter PWA-Last (E3, Nachtrag 14.10.2026)

**Status: freigegeben 14.10.2026.** Umfang Ent-19, Entscheidungen Ent-20 C bis Ent-27. Befund mit
Verifikationsstatus: `requirements.md` Problem §18.

### 18.1 Zuschnitt

Instrumentierung und Massnahmen gehen in **ein** Release (Ent-19, Ent-27). Damit die Wirkung von M-1
zuzuordnen ist, misst G7 kalt gegen warm im selben Stand (§18.8). M-3 wirkt auf die **Zeit**, nicht auf den
Heap; belegt wird es über I-2, I-3 und die Zero-Window-Probe (Ent-26 a). Alles in `http.cpp`,
`httpclient.cpp`, dem gemeinsamen Kopf und `ESP-uclock.ino` schreibt **ein** `esp-developer`, Task für Task
nacheinander (E3.1–E3.5); nach jedem Task misst der Lead „Globale Variablen" (AKR.8).

### 18.2 I-1 — Tiefstwert statt Stichprobe

```
static uint32_t heap_low_block      = UINT32_MAX;  // kleinster max_free_block seit dem letzten Ruecksetzen
static uint32_t heap_low_free       = UINT32_MAX;  // kleinster free_heap, unabhaengig vom Block
static uint8_t  heap_low_path       = 0;           // 1 flush, 2 fs, 3 accept, 4 client - Pfad des Block-Tiefstwerts
static uint32_t heap_low_ms         = 0;           // millis () beim Block-Tiefstwert
static uint16_t httpclient_connects = 0;           // ausgehende Verbindungen seit dem Start, saettigend
void            heap_low_note (uint8_t path);      // EIN Ort; Prototyp in einem Kopf, den httpclient.cpp schon einbindet (base.h)
```

- **Messpunkte, je genau einer** (AKR.1, statisch gezählt):
  - `flush` — in `http_flush()` vor dem ersten `http_client.write()` je Aufruf (`http.cpp:1346`);
  - `fs` — in `http_send_fs_file()` vor jedem `write()` der Leseschleife (`:1676`);
  - `accept` — in `http_server_loop()` unmittelbar nach `if (!http_client) return;` (`:14728-14731`);
  - `client` — in `httpclient()` nach erfolgreichem `client.connect()` (`httpclient.cpp:191-194`), dort auch
    `httpclient_connects++`. Damit sind alle ausgehenden Abrufe über `httpclient()` gezählt — die der
    Anfragen, des Startabrufs (§18.5a), der Legacy-Seiten und des STM-Flash-Downloads; der Wetterabruf hat
    einen eigenen Client und zählt nicht.
- **Ohne Rückwirkung:** kein `String`, keine Heap-Anforderung, keine Ausgabe im Messpunkt. Die Pfadkennung
  ist eine Zahl; erst `device_ready` setzt sie in `flush`, `fs`, `accept`, `client` um (leer, wenn seit dem
  Rücksetzen kein Messpunkt lief).
- **Ausgabe:** `/api/device_ready` hinter `update_server_down_count` (`:13201`): `heap_low_block`,
  `heap_low_free`, `heap_low_path`, `heap_low_ms`, `httpclient_connects`, dann `fs_write_short` (§18.3),
  `req_max_ms`, `req_max_path` (§18.4). Werte **vor** dem Senden einlesen, wie heute `:13166-13175`.
- **Rücksetzen (Ent-21 B):** `device_ready?heap_low_reset=1` liest alle I-1- und I-3-Werte, setzt sie
  zurück und gibt die **gelesenen** aus; ohne Parameter nur lesen. Der lesende Aufruf geht mit seinem
  `accept` noch in das gelesene Fenster ein, mit seinen eigenen Schreibvorgängen schon in das nächste — so im
  Kommentar festhalten. Smoketest, `flash-stm.sh`, `snapshot-device.sh` und die PWA setzen damit nichts
  zurück.
- **Heap-Zeile (Ent-22):** ein zweites, eigenes Minimum des Blocks, von denselben vier Messpunkten gespeist
  und von `esp_heap_log()` (`ESP-uclock.ino:611-641`) nach jeder Zeile zurückgesetzt:
  `- heap free=… max=… ovr=… low=…`. `low=` steht **hinten** — `tools/mess-stabilitaet.sh` liest
  `free=<n> max=<n>` und die Wache `ovr=`, beides bleibt gültig (Kommentar `:625`). Längste Form über 64
  Zeichen: der Puffer `line[64]` (`:627`) wird auf 80 vergrössert. Gemischte Zeilenenden der Datei
  byte-genau erhalten (`CLAUDE.md`, AKZ.9).
- **Prüfpunkt Kosten (◐):** `ESP.getMaxFreeBlockSize()` geht über die Freiliste des Heaps; im `fs`-Pfad wird
  sie je 1'072-Byte-Block einmal gerufen, bei einer `app.js.gz` also rund hundertmal. Am Core 3.1.2 belegen,
  ob dabei Interrupts gesperrt sind, und die Dauer je Aufruf abschätzen. Liegt sie über rund 50 µs, im
  `fs`-Pfad nur den ersten und danach jeden achten Block messen; das steht dann im Kommentar und im Bericht,
  weil es das Tief im `fs`-Pfad unterschätzen kann.

### 18.3 I-2 — Ursache je kurzem Schreibvorgang

**Zeilen** (Felder hinten angefügt; `- http write lost`, `fail=[1-9]` und `- fs write short: ` bleiben als
Muster gültig):

```
- http write lost <rest> gone=<n> fail=<n> blk=<max_free_block> snd=<availableForWrite> ms=<dauer>
- fs write short: <datei> n=<blocklaenge> w=<geschrieben> blk=<max_free_block> snd=<availableForWrite> ms=<dauer>
```

- `blk` und `snd` unmittelbar **nach** dem letzten Versuch erhoben, `ms` vom Beginn des ersten bis zum Ende
  des letzten Versuchs dieses Blocks. `availableForWrite()` liefert im Core den freien Platz im Sendepuffer
  (`tcp_sndbuf`), bei getrennter Verbindung 0 (◐, am Core nachlesen und im Bericht belegen).
- **`http write lost`:** wie bisher **eine** Zeile je Verbindung (`http_write_broken`, `:1387-1417`), Puffer
  `line[64]` auf 96 vergrössern; längste Form rund 90 Zeichen.
- **`fs write short`** in `http_send_fs_file()`: einmal formatieren, über `Serial` **und**
  `stm32_log_append()` (Muster `:1407-1416`, L185); Stackpuffer, kein `String`. Zähler:
  `write_lost_blocks` +1, `write_lost_gone` bzw. `write_lost_failed` nach `connected()` wie in
  `http_flush()`, `write_lost_bytes` um die **nicht ausgelieferten Byte der Datei** (`fp.size()` minus das bis
  dahin Ausgelieferte), neu `fs_write_short` +1. Längste Form mit einem Dateinamen von 31 Zeichen rund 100
  Zeichen, also ein Ringeintrag (≤ 119, C8). `fs read error` (`:1662-1669`) bleibt, wie es ist. Die
  gleichlautende Zeile in `download_file_to_local()` (`:6325`) meint einen Flash-Schreibfehler und bleibt
  unverändert — die Wache meldet beide als Alarm, das ist gewollt.
- **Der Smoketest beobachtet `write_lost_failed`** (C49). Ab E3 zählt dort auch ein abgebrochenes Asset mit;
  das ist gewollt — es war vorher ein stiller Verlust — und gehört in `TESTPLAN-PWA.md` (E3.11).

**Entscheidungsregel** (für G7, T2 und die Logzeilen-Tabelle im Testplan):

| `connected()` | `snd` | Lesart |
|---|---|---|
| 0 | (0) | **Gegenstelle weg** — der Client hat abgebrochen (`gone`) |
| 1 | gleich dem vollen Sendepuffer (1'072, `TCP_SND_BUF`) **oder** mindestens so gross wie der ungeschriebene Rest | **Heap** — Platz im Sendepuffer war da, `tcp_write()` bekam keinen Speicher (`ERR_MEM`); `blk` danebenstellen |
| 1 | 0 | **Quittung fehlt** — der Sendepuffer ist voll, die Gegenstelle nimmt nicht ab (Fenster 0, Funkloch, Energiesparbetrieb) |
| 1 | dazwischen | **unklar** — Rohwerte berichten, nicht deuten |

Der Auftrag nennt die Heap-Lesart „sndbuf voll + Fehlschlag"; gemeint ist der **voll verfügbare**
Sendepuffer, deshalb steht die Regel hier über die Zahl, nicht über das Wort.

### 18.4 I-3 — Längste Anfrage

- `req_max_ms` (`uint32_t`) und `req_max_path` (`char[32]`): Dauer von `accept()` bis zum Verlassen von
  `http_server_loop()` und der Pfad dieser Anfrage, **ohne Query** — die Query kann Zugangsdaten tragen
  (`network_client_set`, Checkliste „Secure"). Pfade über 31 Zeichen werden gekürzt; das ist eine
  Diagnose, kein Wert, der irgendwo gespeichert oder zurückgemeldet wird. Ohne Anfrage: `-`.
- **Jeder Rückkehrpfad** zählt: keine Anfrage (`:14833-14834`), leere Anfrage (`:14849-14850`,
  `:14861-14862`), Schreibabbruch (`:14964-14965`), Regelfall (`:14975`). **Empfehlung:** den bisherigen
  Rumpf in eine innere Funktion legen und in `http_server_loop()` nur noch messen — so kann kein künftiger
  `return` die Messung umgehen. Der Pfad wird gesetzt, sobald er bekannt ist.
- Rücksetzen mit I-1 (Ent-21 B).
- I-3 misst, was M-3 **nicht** begrenzt (§18.6): eine Antwort, die langsam, aber stetig abfliesst.

### 18.5 M-1 — ausgehende Abrufe aus den Anfragen

**Was sich ändert und was nicht — ausdrücklich:**

| Abruf in `update_status` / `update_table_files` | 3.2.35 | E3 kalt (Versions-Cache abgelaufen) | E3 warm (innerhalb 60 s) |
|---|---|---|---|
| drei Versionszeilen (`:12810-12812`) | kalt | **ja** | nein |
| zwei Existenzprüfungen Icon/Wetter (`:12846`, `:12852`) | kalt | **ja** | nein |
| Release Notes (`:13062`) | **immer** | **nein** — aus der Datei (§18.5a) | nein |
| `wc-list.txt` (`:13072`) | **immer** (bei bekannter Hardware) | **nein** — aus der Datei (§18.5a) | nein |
| `wc-list-tables.txt` (`:13357`) | **immer** | **ja** | nein — aus dem RAM |
| **Summe je Paar Aufrufe** | 8 / 3 | **6** | **0** |

Der Sollwert 6 für kalt gilt bei bekannter Hardware; E3.0 bestimmt ihn am Gerät, AKR.13 prüft ihn. Ein
kalter Aufruf — das gewöhnliche erste Öffnen der PWA nach einer Pause — holt also weiterhin die kurzen
Versionszeilen, die Existenzprüfungen und die Tabellenliste, aber **nie** mehr die bis zu 3 KB Release Notes
und die STM-Liste. Ziel 16 verspricht deshalb nicht mehr.

**(1) `table_files` in `update_table_files` (`WC_TABLES_LIST_TXT`, `:13314-13406`) — RAM-Zwischenspeicher:**
- Die **gefilterten** Namen (Präfix nach Hardware, `:13340-13348`) in einem statischen Puffer, getrennt durch
  `\n`. Gültigkeit `UPDATE_INFO_CACHE_MS` (60 s), Prüfsumme über Host, Pfad und Hardware wie die
  Versionsfelder (`update_source_checksum()`, `:12641-12659`), `?refresh=1` umgeht ihn.
- **`current_table` bleibt live** (`tables_fname()`, `:13320`) — die PWA prüft nach einer
  Tabelleninstallation genau dieses Feld (`app.js:6495`, `:6517`).
- Der Abruf läuft über `update_server_open()` (`:12701-12717`) statt `httpclient()` direkt: hinter
  `update_server_blocked()`, mit `update_server_note()`. Im Sperrfenster sofort `"table_files":[]`.
- Abgelegt wird nur ein **vollständig** gelesener Abruf (`len` bis 0, kein `ch < 0`). Ein fehlgeschlagener
  oder abgerissener Abruf wird als **leere** Liste mit dem kürzeren Fenster (`UPDATE_SERVER_DOWN_MS`)
  abgelegt — wie die Versionsfelder (`:12814-12820`). Heute liefert ein abgerissener Abruf still eine
  **Teilliste**; künftig eine leere, die nach 30 s neu geholt wird.
- **Puffergrösse (Empfehlung):** die in E3.0 gemessene gefilterte Länge × 1,5, auf 32 aufgerundet, höchstens
  256 Byte. Passt eine Liste nicht: **nicht kürzen**, nicht ablegen, beim nächsten Aufruf wieder live holen;
  gedrosselt (erste sofort, danach höchstens eine je 10 min) `- update liste zu gross wc-list-tables.txt
  n=<byte>` in Mitschnitt und Ring.
- **Warum nicht ins LittleFS:** Für die Tabellenliste gibt es keine Version, an der sich ein Neuschreiben
  festmachen liesse.

**(2) Release Notes und `stm32_files` — LittleFS (Ent-20 C), Einzelheiten §18.5a.**

**(3) Kopfkommentar `:12596-12620` nachführen:** Die Begründung „bewusst nicht zwischengespeichert, ein
Puffer wäre bis zu 3'072 Byte gross" gilt weiter für den RAM; neu liegen Notes und STM-Liste im LittleFS.

**Rückwärtsverträglichkeit, ✔ SW (Grundlage für AKR.6 und den Taskbericht E3.4):**

| Feld | Leser in der PWA | Wirkung von M-1 |
|---|---|---|
| `release_notes` | Wartung/Update (`app.js:11786`, `:11893`, Platzhalter `:8381`) | **unverändert sichtbar**, Stand des letzten Releases auf dem Server; leer nur in den rund 60 s nach dem ersten Start mit E3 und solange die Datei ungültig ist |
| `stm32_files` | Wartung/Update, Auswahl der STM-Datei (`:11803`) | aus der Datei, gefiltert wie live; nach einem neuen Release auf dem Server, sobald ein kalter `update_status` den neuen Schlüssel gesehen und der Auftrag geschrieben hat |
| `table_files` | Wartung, Update, Anzeige: Installationsangebot (`:11915-11925`) | bis 60 s alter Stand |
| `current_table` | Vorschau, Hinweis „Tabelle fehlt", Prüfung nach der Installation (`:5159`, `:6401-6406`, `:6495`, `:6517`) | **unverändert live** |
| Versionsfelder, `assets_available`, `update_info_cached` | wie bisher | unverändert (Zwischenspeicher seit L161) |
| `stm32_default` | `flash-stm.sh`, Smoketest, PWA | lokal gebildet, unberührt |

Keine neuen Felder in diesen beiden Antworten; ob ein Lauf kalt oder warm war, zeigt
`httpclient_connects` in `device_ready`. **Keine PWA-Änderung nötig.** Die Legacy-Seiten holen Notes und
Listen über eigene Wege (`http.cpp:6966`, `:7802`, `:7899`) und bleiben unverändert.

### 18.5a Release Notes und `wc-list.txt` im LittleFS (Ent-20 C)

**Grundsatz (Nutzerentscheid):** Beide Dateien werden **nicht in einer Anfrage** geholt, sondern durch einen
Auftrag in `loop()` — beim Start, solange der Heap noch unzerstückelt ist, und danach nur, wenn sich der
Stand auf dem Server geändert hat. `update_status` liefert aus der Datei.

**Dateinamen (festgelegt):** `/upd-releasenote.txt` und `/upd-wc-list.txt`, Temporärdateien
`/upd-releasenote.tmp` und `/upd-wc-list.tmp`, alle im Wurzelverzeichnis des LittleFS.
- Kein `app-`-Präfix und kein `.gz`: Die PWA-Assets heissen `app-*.gz` (`app_asset_storage_filename()`,
  Weissliste `APP_INSTALL_ASSETS`, `http.cpp:357` ff.); `http_app()` liefert ausschliesslich `.gz` aus
  (Invariante) — die Cache-Dateien sind über `/app/` nicht erreichbar.
- Keine Kollision mit den Upload-Zielen (`wc24h-…`, `wc12h-…`, `uc-…`, `stm32.hex`). **Prüfpunkt E3.4:**
  Bestandsliste aller festen Dateinamen in `http.cpp` und `ESP-uclock.ino` per `grep`, im Bericht; dazu, ob
  `install-app.sh --check`, die Dateiliste der PWA oder ein Upload-Filter auf `upd-` anspringen (Stand
  `8445a10`: `install-app.sh` kennt keine Prüfung auf fremde Dateien, ✔ SW per `grep`).

**Format (festgelegt):** eine Kopfzeile, dann der Rohinhalt vom Server:

```
U1 <schluessel> <laenge>\n
<genau laenge Byte, roh wie vom Server>
```

- `<schluessel>` = `<esp_available>|<app_available>|<wc_available>|<quelle>`: die drei Versionszeilen vom
  Server, wie sie `update_server_fetch_line()` liefert, und eine Prüfsumme über **Host und Pfad, ohne
  Hardware**. Ohne Hardware, weil `HARDWARE_CONFIGURATION` nach jedem ESP-Start zunächst 65535 ist (L42) —
  mit Hardware im Schlüssel schriebe jeder Start neu. Die Hardware braucht die Datei nicht: Sie hält die
  **ungefilterte** `wc-list.txt`; gefiltert wird beim Ausgeben, wie live (`http_remote_stm32_filename_matches()`,
  `:13100`).
- **Längen:** Notes höchstens 3'072 Zeichen wie heute (`:13062`), `wc-list.txt` höchstens nach E3.0
  (Empfehlung 2'048). Ist der Inhalt länger, wird er **nicht** abgelegt (kein stilles Kürzen), Zeile
  `fehlgeschlagen zu-gross`.
- **Gültig** ist eine Datei, wenn sie existiert, ihre Grösse > 0 ist (`http_fs_file_exists_and_nonempty()`,
  `:1704-1722`, Architektur-Invariante), die Kopfzeile mit `U1 ` beginnt und lesbar ist, und die
  Dateigrösse **genau** Kopfzeile plus `<laenge>` beträgt. Alles andere gilt als **fehlend**.

**Der Auftrag (`update_cache_poll()` in `http.cpp`, aufgerufen in `loop()` neben `http_server_loop()`):**
- **Merker „fällig"** gesetzt: beim Start; wenn beim Ausliefern eine Datei ungültig ist; wenn ein **kalter**
  `update_status` Versionen holt, deren Schlüssel von dem der Dateien abweicht (Vergleich mit dem
  vorhandenen Versions-Cache, `:12808-12821`); wenn Host oder Pfad geändert werden (die bestehende
  Prüfsumme der Quelle ändert sich).
- **Läuft nur**, wenn alles zutrifft: WLAN verbunden; **Host und Pfad als Variablen gesetzt**, nicht leer —
  sonst fiele `update_status` auf `DEFAULT_UPDATE_HOST` zurück, den Server des Ursprungsprojekts (L42,
  DIR-009); der Auftrag fragt diesen **nie**. Mindestens 60 s seit dem Start — der Startabgleich mit dem STM
  läuft dann nicht mehr (A79, L421). Mindestens 5 s seit der zuletzt bedienten Anfrage. Nicht im
  Sperrfenster (`update_server_blocked()`), und nach einem eigenen Fehlschlag frühestens nach **10 min**
  erneut (`UPDATE_CACHE_RETRY_MS`; das Sperrfenster greift nur bei langsamen Fehlschlägen, ein sofortiges
  404 liesse sonst jede Schleife neu fragen).
- **Ablauf, je Schleifendurchlauf höchstens ein Abruf** (kleine Zustandsmaschine), damit `loop()` die
  Brücke zwischen den Abrufen liest:
  1. drei Versionszeilen über `update_server_fetch_line()` ⇒ Schlüssel; nebenbei den Versions-Cache füllen;
  2. Schlüssel gleich dem beider Dateien und beide gültig ⇒ **nichts schreiben**, Zeile `unveraendert`,
     Merker löschen;
  3. sonst je Datei: `httpclient()` in die **Temporärdatei** schreiben (Muster
     `download_file_to_local()`, `:6266-6334`, mit dem statischen Puffer `http_download_buf`; die
     Kopfzeile mit der Länge zuerst); **umbenennen** (`LittleFS.rename`) erst, wenn der Abruf vollständig
     gelesen (`len` 0, kein `ch < 0`) **und** vollständig geschrieben ist; sonst Temporärdatei löschen, die
     alte Datei bleibt gültig und wird weiter ausgeliefert.
- **Zeilen** (Mitschnitt **und** Ring, einmal formatiert):
  `- update cache <datei> geschrieben n=<byte>`, `- update cache unveraendert`,
  `- update cache <datei> fehlgeschlagen <grund>` mit Grund `abruf`, `abgerissen`, `schreiben`, `platz`,
  `zu-gross`, `quelle-leer`.
- **Blockade:** Jeder Abruf hält `loop()` so lange an wie heute derselbe Abruf in einer Anfrage, im
  schlechtesten Fall bis zur Zeitgrenze von `httpclient.cpp` (`HTTPCLIENT_READ_TIMEOUT`, 5 s). Der
  Unterschied: Das geschieht einmal je Release, nicht in jeder Anfrage. Ob ein Startabruf eine `keine
  Quittung` kostet, zeigt AKR.20.

**Schreibhäufigkeit (Flashverschleiss):** Geschrieben wird **nur**, wenn sich der Schlüssel geändert hat
(ein neues Release auf dem Server oder eine andere Quelle) oder eine Datei ungültig ist — also je Release
höchstens einmal je Datei, plus nach einem gescheiterten oder formatierten Stand. Ein ESP-Start mit
gleichem Schlüssel schreibt nichts (AKR.21). c65n zählt die Schreibvorgänge.

**Platz im LittleFS (◐, in E3.0 gemessen):** Jede Datei belegt mindestens einen Block des Dateisystems;
während des Schreibens liegen Temporärdatei und alte Datei nebeneinander. Geschrieben wird nur, wenn nach dem
Schreiben noch mindestens **die Grösse der grössten PWA-Datei (`app-app.js.gz`) plus ein Block** frei bleibt
— sonst Zeile `fehlgeschlagen platz`, nichts geschrieben: Eine PWA-Installation darf nie am Cache scheitern.
Den freien Platz und die Blockgrösse liefert `/api/fs_info` bzw. das LittleFS (`FSInfo`), Werte in E3.0.

**Ausliefern in `update_status`:**
- **Notes, Datei gültig:** Inhalt hinter der Kopfzeile in Stücken von 64 Byte über einen **Stackpuffer**
  lesen, mit **derselben** JSON-Maskierung wie live (`http_json_send_remote_text_field()`, `:8399-8460`;
  Empfehlung: die Maskierung als gemeinsame Funktion, damit Datei und live byte-gleich sind), ausgegeben über
  `http_send()` wie jede Antwort. Der statische Antwortpuffer `http_response` taugt hier **nicht** als
  Lesepuffer — anders als in `http_send_fs_file()` ist er mitten in der JSON-Antwort belegt. Kein `String`,
  keine ausgehende Verbindung.
- **Notes, Datei ungültig:** `"release_notes":""` an derselben Stelle, Auftrag fällig, **kein** Live-Abruf.
- **`stm32_files`, Datei gültig:** zeilenweise lesen, filtern und ausgeben wie live (`:13074-13116`).
- **`stm32_files`, Datei ungültig:** live wie bisher über `update_server_open()` (Sperrfenster), Auftrag
  fällig — sonst fehlte die STM-Auswahl, bis der Auftrag gelaufen ist.
- LittleFS wird dafür gemountet wie bei jeder Asset-Auslieferung (`LittleFS.begin()`/`end()`, `:1604`,
  `:1699`); das belegt den Heap kurz (◐, wenige hundert Byte), dauerhaft nichts.

**Was passiert, wenn …**

| Lage | Verhalten |
|---|---|
| erster Start mit E3, keine Dateien | rund 60 s lang Notes leer, STM-Liste live; dann Startabruf schreibt beide |
| Startabruf scheitert (Server weg, 404) | Zeile `fehlgeschlagen abruf`; Notes bleiben leer, STM-Liste live mit Sperrfenster; neuer Versuch nach 10 min bzw. nach Ablauf des Sperrfensters — nicht bei jedem `update_status` |
| Abruf reisst mitten im Inhalt ab | Temporärdatei gelöscht, alte Datei bleibt (oder bleibt fehlend), `fehlgeschlagen abgerissen` |
| LittleFS voll oder Platzgrenze | nichts geschrieben, `fehlgeschlagen platz`; alte Datei bleibt |
| Datei 0 Byte oder Länge stimmt nicht | gilt als fehlend: wie „keine Dateien" |
| neues Release auf dem Server | der nächste kalte `update_status` sieht den neuen Schlüssel, liefert noch die alte Datei, setzt den Auftrag; der schreibt frühestens 5 s nach der Anfrage |
| Host/Pfad leer (Variablen verloren, L42) | Auftrag läuft nicht, `fehlgeschlagen quelle-leer` einmal; ausgeliefert wird, was im LittleFS liegt |
| `maintenance_format_fs` | Dateien weg ⇒ wie erster Start |

### 18.6 M-3 — Sendezeitgrenze (Ent-25)

- `http_client.setTimeout (1000)` unmittelbar nach `if (!http_client) return;` (`:14728-14731`), vor jedem
  Schreiben. In `http_send_fs_file()` neben `setNoDelay (0)` (`:1626`) `http_client.setTimeout (2000)`.
  Namen als Konstanten (`HTTP_WRITE_TIMEOUT_MS`, `HTTP_FILE_WRITE_TIMEOUT_MS`).
- **Rechnung** (Grundlage: Kommentar `:1320-1325`, `flush()` und `stop()` warten je bis zu 300 ms, L136, ◐):
  API-Fehlschlag 2 × 1'000 + 300 + 300 = **2'600 ms**; Datei 2'000 + 300 + 300 = **2'600 ms**. Nach einem
  Abbruch läuft `abort()` statt `flush()`/`stop()` (`:14955-14966`) — dann weniger. Beides unter der 3-s-Frist,
  nach der der STM `keine Quittung` meldet.
- **Was M-3 nicht begrenzt:** Jeder Fortschritt setzt die Frist neu (`_write_from_source`, Kommentar
  `:1300-1302`). Eine Antwort aus vielen Blöcken, die langsam, aber stetig abfliesst, kann weiter über 3 s
  dauern — I-3 zeigt es (AKR.18), die Abhilfe ist ein Folgebefund (§18.10).
- **OTA (Ent-25):** `ESPhttpUpdate.update (http_client, …)` (`:7737`, `:13289`) verwendet **dasselbe**
  Objekt für den Download der neuen Firmware. Unmittelbar vor beiden Aufrufen steht ausdrücklich
  `http_client.setTimeout (5000)` (Konstante, mindestens 5'000) — unabhängig davon, ob `HTTPClient` die
  Grenze beim Verbinden selbst setzt. Ein OTA, das an 1'000 ms scheitert, wäre genau der Schaden, den C65
  abwenden soll.
- **Bestandsliste (AKR.7):** alle Lesestellen auf `http_client`, die eine Stream-Zeitgrenze benutzen
  (`readBytes`, `readString*`, `find*`, `parse*`). Stand `8445a10` per `grep`: keine; die Uploads lesen mit
  eigener Frist (`READ_BODY_TIMEOUT`, `:6343`).
- **Kommentare nachführen:** `:1300-1302`, `:1315-1318` und `:8369-8371` nennen 5'000 ms als geltende Grenze.
  Eine überholte Zusage im Code hält vom Nachsehen ab — das war L185.
- **Nebenwirkung (Risiko, benannt):** Langsame Gegenstellen — ein Telefon im Energiesparbetrieb, schwaches
  WLAN — erreichen die Grenze früher: mehr `http write lost … fail=` und `fs write short`. Bei einem Asset
  bricht der Browser den Abruf ab (Content-Length ist gesetzt, `:1636-1637`; nichts wird still gekürzt), der
  Service Worker behält seinen Stand; bei einer Erstinstallation kann ein Asset fehlen, ein Neuladen holt es.
  Fortschrittsmeldungen während eines STM-Flashs (`stm32flash.cpp`, `http_flush()`) gehen an einen langsamen
  Browser früher verloren; der Flash selbst lädt über den eigenen Client in `httpclient.cpp` und ist nicht
  betroffen. Beobachtet über I-2 (Lesart „Quittung fehlt") in G7, G6 und T2.

### 18.7 Prüfstände (Lead, `tools/checks/auszug/esp/c65/`)

Gebaut vom Umsetzer im Scratchpad, abgelegt vom Lead, Gegenprobe vom Lead (Werkzeugvorbehalt `tasks.md`).

- **c65a (I-1, I-3):** Attrappen für `ESP.getMaxFreeBlockSize()`, `ESP.getFreeHeap()` (Wertfolgen) und
  `millis()`. Minimum und Pfad über gemischte Folgen, Gleichstand (der erste bleibt), Rücksetzen über
  `heap_low_reset=1` und über die Heap-Zeile unabhängig voneinander, Lesen ohne Parameter setzt nichts
  zurück, Sättigung von `httpclient_connects`; I-3 über alle fünf Rückkehrpfade, Query nie im Pfad.
  **Statisch:** genau vier Aufrufe von `heap_low_note` an den vier Orten (§18.2). Sabotagen: ein Messpunkt
  entfernt ⇒ FEHL; Lesen ohne Parameter setzt zurück ⇒ FEHL; Query im Pfad ⇒ FEHL. Gegenprobe gegen 3.2.35 ⇒
  FEHL.
- **c65b (I-2):** `WiFiClient`-Attrappe **mit der Semantik der Checkliste** (Zustellung nur an
  Abgabepunkten, `connected()` nach FIN 0 — die Lehre aus t12), steuerbar: Rückgabe von `write()`,
  `availableForWrite()`, `millis()`. Die vier Lagen der Entscheidungsregel je für `http_flush()` und
  `http_send_fs_file()`; je Verbindung genau eine Zeile, Mitschnitt und Ring textgleich, Zähler, Länge ≤ 119.
  Gegenprobe gegen 3.2.35 ⇒ FEHL.
- **c65m (M-1, Tabellenliste und Antwortform):** Attrappe für `httpclient()`/`httpclient_read()`, die
  Verbindungen zählt und Listen liefert, abbricht oder verzögert. Folgen nach AKR.4; **Schlüsselvergleich**
  beider Antworten gegen den Stand 3.2.35 aus derselben Attrappe (AKR.6). Gegenprobe gegen 3.2.35 ⇒ FEHL.
- **c65n (M-1, LittleFS, Ent-20 C):** LittleFS-Attrappe im Speicher (Platzgrenze, Schreibfehler,
  `rename`), Attrappe für `httpclient()` mit Zähler, steuerbare Zeit und WLAN-/Variablenlage. Fälle nach
  AKR.5: Datei fehlt ⇒ Auftrag holt und schreibt, Kopfzeile stimmt; Schlüssel gleich ⇒ 0 Abrufe von Notes und
  Liste, **0 Schreibvorgänge**; Schlüssel anders ⇒ genau ein Schreiben je Datei; Lesen bricht ab ⇒
  Temporärdatei gelöscht, alte Datei unverändert; Schreibfehler und Platzgrenze ⇒ dasselbe, Zeile; Inhalt zu
  gross ⇒ nicht abgelegt; Datei mit 0 Byte, falscher Länge, ohne `U1 ` ⇒ ungültig; Host leer ⇒ **kein**
  Abruf, `DEFAULT_UPDATE_HOST` nie gefragt; vor 60 s, innerhalb 5 s nach einer Anfrage, im Sperrfenster,
  innerhalb 10 min nach Fehlschlag ⇒ kein Abruf; Hardware wechselt auf 65535 ⇒ kein Neuschreiben;
  `update_status` mit gültiger Datei ⇒ **0** Abrufe von Notes und Liste und eine **byte-gleiche** Antwort
  gegenüber dem Live-Weg bei gleichem Serverinhalt (Maskierung, Filter); Notes-Datei ungültig ⇒ `""`, kein
  Abruf. Sabotagen: Umbenennen vor dem vollständigen Lesen ⇒ FEHL; Hardware im Schlüssel ⇒ FEHL (Schreiben
  beim Start). Gegenprobe gegen 3.2.35 ⇒ FEHL.
- **c65t (M-3, statisch):** `setTimeout (1000)` nach `accept()` vor dem ersten Schreiben, `2000` im
  Dateipfad, `5000` vor beiden OTA-Wegen. Sabotagen: eine der drei Zeilen entfernt ⇒ je FEHL.

Fallzahlen je Prüfstand gemeldet und gegen das gezählt, was es geben müsste (L235).

### 18.8 Gerätenachweis G7 — Wirkung von M-1 isoliert (Ent-27: einstufig)

**Das Problem mit „vorher":** 3.2.35 hat kein I-1. Einen Vorher-Wert mit **derselben** Messweise gibt es
nicht, und ein Abfragewert mit 2/s ist mit I-1 nicht vergleichbar (K30, L449). Zwei Wege, die sich ergänzen:

1. **Kalt gegen warm im selben Stand (tragend).** Kalt holt Versionen, Existenzprüfungen und
   Tabellenliste — Sollwert Δ`httpclient_connects` = 6 (§18.5) —, warm holt nichts. Der Unterschied der
   I-1-Tiefstwerte ist die Wirkung der verbliebenen ausgehenden Abrufe auf das Tief. Dass Notes und STM-Liste
   in **keinem** Lauf geholt werden, belegt derselbe Zähler: kalt **genau** 6, nicht 8. `check-pwa.sh` lädt
   jedes Mal mit frischem Profil, die Asset-Last ist also in beiden Läufen gleich.
2. **Alte Messweise gegen L449.** Ein kalter Lauf mit `device_ready` 2/s wie die Kontrollmessung von L449
   (1'448) — gleiche Messweise vorher und nachher, mit der Messrückwirkung beider. L449 lief mit PWA 1.4.98,
   G7 mit 1.4.99; P2 bringt laut Lead keine neuen Anfragen.

**Der Fall „Datei fehlt / Version neu"** ist am Gerät nur einmal herstellbar, beim **ersten Start nach dem
OTA** (die Dateien gibt es mit 3.2.35 noch nicht) — G7.0b. Eine neue Version auf dem Server und eine
gelöschte Datei werden nicht hergestellt (§17.3); sie belegt c65n.

**Ablauf** (lesend nach N-5; Eingriffe nach Ent-26; Dauer rund 25 min, bei Planung `caffeinate`):

| Schritt | Was | Ins Messprotokoll |
|---|---|---|
| G7.0 | Wache läuft seit vor dem OTA; Smoketest, `install-app.sh --check`, `check-pwa.sh` aus E3.15 abgeschlossen | Uhrzeit |
| G7.0b | **Startabruf (AKR.20):** frühestens 60 s nach dem Start je eine Zeile `- update cache … geschrieben`; `/api/fs_list` zeigt beide Dateien mit Grösse > 0; Δ`httpclient_connects` des Startabrufs (Soll 5: drei Versionen, Notes, Liste); keine `keine Quittung` im Umfeld; danach mindestens 70 s keine Anfrage | Zeilen, Grössen, Zähler |
| G7.1 | **Ruhe:** 150 s keine Anfrage, dann `device_ready?heap_low_reset=1` | Heap-Zeilen samt `low=`, Ruhe-Tief, Pfad |
| G7.2 | **Drei Paare** K1, W1, K2, W2, K3, W3. **Kalt:** mindestens 70 s seit dem letzten `update_status`/`update_table_files`; zurücksetzen; ein `check-pwa.sh`; 15 s nach dessen Ende lesen und zurücksetzen. **Warm:** `curl …/api/update_status` und `curl …/api/update_table_files` je einmal, 10 s warten, zurücksetzen, `check-pwa.sh`, 15 s danach lesen. Liegt ein `- weather` des STM im Fenster oder stimmt Δ`httpclient_connects` nicht (kalt = Sollwert aus E3.0, warm = 0), Paar wiederholen; kalt = 8 ⇒ FEHL (AKR.13) | je Lauf alle Felder aus AKR.13, dazu `keine Quittung`, `http write lost`, `fs write short`, `- update cache` aus dem Mitschnitt |
| G7.3 | **Alte Messweise:** ein kalter Lauf, `device_ready` alle 0,5 s (ohne Parameter) während `check-pwa.sh` | kleinster `max_free_block` der Abfragen, `keine Quittung` getrennt (AKR.14) |
| G7.4 | **Zero-Window-Probe (Ent-26 a):** je eine gegen `/app/app.js.gz` und `/api/settings_xml`, mindestens 30 s Abstand | Zeile, `ms`, Zählerstände, `req_max_ms` (AKR.17) |
| G7.5 | **Vorhersageabruf (Ent-26 b):** ein `weather_get_forecast` 5 s nach dem Start eines **warmen** `check-pwa.sh`; der Lauf zählt nicht in G7.2 | Zeile `- weather …` (AKR.16) |
| G7.6 | **Ruhe nach Last:** 150 s keine Anfrage | `max` der Heap-Zeile gegen 5'120, 5'808, 7'264 (AKR.12), `low=` |

**Die Zero-Window-Probe (Werkzeug des Leads, Ent-26 a):** ein TCP-Client mit kleinem Empfangspuffer
(`SO_RCVBUF` vor dem Verbinden), der die Anfrage sendet, danach **nicht liest**, die Verbindung 6 s hält und
schliesst. Der ESP füllt seinen Sendepuffer, die Gegenstelle meldet Fenster 0, `write()` macht keinen
Fortschritt mehr — erwartet ist die Lesart „Quittung fehlt" mit `snd=0`. Je Probe steht `loop()` höchstens
rund 2,6 s. Vor dem Einsatz am Gerät gegen einen lokalen Server gelaufen, der normal liest — dort muss das
Werkzeug „kein Abbruch" melden und mit Exit ≠ 0 enden (DIR-014). Der Smoketest läuft **vor** den Proben;
danach ist `write_lost_failed` um 1 höher — erwartet, im Messprotokoll vermerkt.

### 18.9 Haltegrenze K30 nach E3 (Ent-23, Nutzer)

- **Anhalten**, wenn der **Ruhewert** unter **5'120** Byte fällt. Ruhewert: `max` der ersten Heap-Zeile, die
  mindestens 60 s nach der letzten Aktion des Testers beginnt; die offene PWA darf dabei weiter abfragen — so
  liegt das Gerät da, wenn aus der PWA ein OTA gestartet wird. 5'120: ein OTA braucht 4 KB am Stück plus
  Reserve, und der Vorhersageabruf belegt 5'120 am Stück (`weather.cpp:376`), darunter ist `oom` zu
  erwarten.
- **Anhalten**, wenn der **Tiefstwert** unter **1'072** Byte fällt — `low=` der Heap-Zeile (Ent-22). Unter
  einem vollen Sendepuffer bringt lwIP keinen Block mehr unter; dann scheitert jede Antwort (◐, nicht
  gemessen).
- **Dazwischen kein Anhaltegrund:** ins Protokoll mit Uhrzeit, Pfad, Last und Messweise.
- **Nicht:** der I-1-Tiefstwert gegen 5 KB. Nach L449 läge schon jede einzelne PWA-Ladung darunter; T2 käme
  nie zu den Settern, ohne dass das Gerät gefährdet wäre.
- Werte verschiedener Messweisen nicht miteinander vergleichen (K30, L449).

### 18.10 Folgebefunde (nicht in E3; Kennungen vergibt der `doc-writer`, E3.12)

- **M-2** Verbindungen begrenzen: `http_server.begin (80, 2)`, PWA `power_status` statt zweier Abrufe
  (Ent-19).
- **M-4** `FS()` ohne Heap (Ent-19).
- **Teildatei in `download_file_to_local()`** (`http.cpp:6292-6331`, ◐): Bricht das Lesen ab, bleibt die
  Teildatei liegen; nur ein Schreibfehler löscht sie. Aufrufer prüfen, Befund eintragen. E3 ist nicht
  betroffen (Temporärdatei, §18.5a).
- **Brücke während langer Antworten lesen** — falls AKR.18 Anfragen über 3 s zeigt: `stm_bridge_poll()`
  (B55, §6.2) zwischen zwei Blöcken in `http_flush()`; die Sperre gegen Verschachtelung besteht schon.
- **Ursache der bleibenden Zerstückelung** (M-A) — nach G7 mit `low=` und I-1 neu bewerten.
- **Kalter `update_status`** — falls AKR.18 kalt über 3 s zeigt: sechs Abrufe nacheinander in einer Anfrage;
  der Auftrag aus §18.5a könnte die Versionszeilen ebenfalls übernehmen.

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
| `ESP8266/ESP-uclock/http.cpp` | C46, C19 (im Baum seit E1, Release E2); B55 (E2); C6b (EP); **C65 (E3): I-1, I-2, I-3, M-1 samt LittleFS-Zwischenspeicher und `update_cache_poll()`, M-3**. ~~Herkunft, C60, C6v (E1); POST (EP)~~ entfallen (Ent-18) | `esp-developer` |
| `ESP8266/ESP-uclock/httpclient.cpp` | **C65 (E3):** Messpunkt `client`, `httpclient_connects` | `esp-developer` |
| `ESP8266/ESP-uclock/base.h` (oder `http.h`) | **C65 (E3):** Prototypen `heap_low_note()`, `update_cache_poll()` | `esp-developer` |
| `ESP8266/ESP-uclock/ESP-uclock.ino` | B55 (`stm_bridge_poll`, `RPCR`), C8, A79; **E3:** `low=` in der Heap-Zeile (Ent-22), Aufruf von `update_cache_poll()` in `loop()` | `esp-developer` |
| `ESP8266/ESP-uclock/ntp.cpp` | C59 | `esp-developer` |
| `ESP8266/ESP-uclock/data/app/app.js` | C6b, C46, B55 (EP); B48, B8 (P2). ~~C6 (POST)~~ entfällt. **E3: keine Änderung** | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/i18n/en.json` | Kennungen 9–11, B55 (EP); B8 (P2) | `pwa-developer` |
| `ESP8266/ESP-uclock/data/app/styles.css` | B48 | `ui-developer` |
| `ESP8266/ESP-uclock/data/app/index.html` | B8 | `ui-developer` |
| LittleFS auf der Uhr | **E3:** `/upd-releasenote.txt`, `/upd-wc-list.txt` (vom ESP selbst geschrieben) | — |
| `tools/watch-log.sh`, `.claude/` (Stop-Hook) | E36, Wachzeilen; E3: `fs write short`, `update liste zu gross`, `update cache … fehlgeschlagen` | Lead |
| `tools/flash-stm.sh` | `--lokal` (Schritt 0, gebaut, bleibt als Werkzeug). ~~POST (EP)~~ entfällt | Lead |
| `tools/checks/**` | F3, B8, Prüfstände; E3: c65a, c65b, c65m, c65n, c65t | Lead |
| `tools/snapshot-device.sh`, `preview/server.py` | C6b (EP) | Lead |
| `tools/smoke-device.sh`, `hooks/no-danger.py`, `checks/hooks.sh` | ~~POST~~ entfällt; die Probe `curl -X POST` für `no-danger.py` aus Schritt 0 bleibt; E3: neue `device_ready`-Felder (AKR.10) | Lead |
| Zero-Window-Probe (neu unter `tools/`, Ent-26 a) | E3, G7 | Lead |
| Versionsdateien | je Release | `release-engineer` |
| `BEFUNDE.md` | Messprotokoll (Lead), Befunde (`doc-writer`) | nie gleichzeitig |
| `TESTPLAN-PWA.md` | neue Fälle; K25, K26 entfallen (E2.10); E3: K30, Logzeilen, `device_ready`-Felder, AKR.21 (E3.11) | `doc-writer` |
| `CLAUDE.md`, `CHANGELOG.md` | Einträge; ~~Gefahrentabelle (POST)~~ entfällt | Lead |

---

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Werden die Wetter-Endpunkte eingehalten?
Bleibt die Restore-Bedingung vollständig? Greift die Änderung an der richtigen Schicht an?

> **Legacy bleibt parallel und bedienbar** — mit GET; neu ist nur die Abweisung leerer und überlanger
> Update-Quellen (§9). Die Herkunftsprüfung an den zerstörenden Aktionen (§8.1) entfällt mit Ent-18. Für
> jemanden, der die Legacy-Seite selbst bedient, ändert sich nichts. Scheitert in EP der PWA-Upload, bleibt
> die alte PWA bedienbar, nur ohne Backup-Export (§8.4). **Wetter:** Kein Pfad berührt die Wetter-Endpunkte;
> G2 und G4 rufen sie als Auslöser. **Restore-Bedingung:** A70 und A80 lassen die vier Teilbedingungen und
> `! display_clock_flag` wörtlich stehen (AKN.5); geändert wird der Vergleich im Innenzweig und, dass
> `do_display_icon` bei ausgeschaltetem Display nicht gesetzt bleibt. **`.gz` und Grösse > 0:** EP und P2
> gehen über `make app-gz` und `install-app.sh` (R2). **Schicht:** B55 entscheidet am STM und reicht durch;
> C46 weist am ESP ab, die PWA vermeidet nur unnötige Fehler; A79 und C8 beheben Verluste am ESP.
>
> **E3 (C65):** Die Ursache liegt am ESP — Abrufe beim Update-Server mitten in der eigenen Antwort,
> Wartezeiten beim Schreiben —, und dort wird angesetzt; die PWA wird **nicht** umgangen und bleibt
> unverändert (AKZ.3). Die Legacy-Seiten holen Notes und Listen weiter über eigene Wege und bleiben die
> Stabilitäts-Referenz. Wetter: unberührt; G7 ruft `weather_get_forecast` nur als Auslöser (Ent-26 b).
> Restore-Bedingung: STM unberührt. **`.gz` und Grösse > 0:** `http_send_fs_file()` liefert weiter
> ausschliesslich `.gz` und prüft die Grösse > 0 (`:1611`); die Cache-Dateien sind keine Assets, tragen kein
> `.gz`, stehen nicht in `APP_INSTALL_ASSETS` und gelten nur mit Grösse > 0 **und** stimmender Länge
> (§18.5a).

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
>
> **E3 (C65):** Kein STM-Kommando. **UART:** `http write lost` wird um rund 30 Zeichen länger, aber
> weiter nur einmal je Verbindung und nur bei einem Verlust; `fs write short` stand schon auf der UART und
> bekommt dieselben Felder; `- update cache …` je Start eine bis drei Zeilen; `- update liste zu gross`
> höchstens alle 10 min; `low=` rund 10 Byte je Minute. **Hauptloop ESP:** Ein Schreibversagen hält
> `loop()` künftig höchstens rund 2,6 s an (§18.6) statt bis zu rund 10 s (zwei Versuche zu 5 s) — damit liest
> der ESP die Brücke wieder innerhalb der 3-s-Frist des STM. Ein warmer `update_status` macht keinen
> ausgehenden Abruf, ein kalter sechs statt acht. Der Startabruf blockiert je Abruf wie heute derselbe Abruf
> in einer Anfrage — aber einmal je Release, je Schleifendurchlauf höchstens einer, frühestens 60 s nach dem
> Start. **Watchdog:** keine neue Schleife ohne Zeitgrenze; der Messpunkt im `fs`-Pfad kostet je Block einen
> Gang über die Freiliste (◐, §18.2). **Flash:** Schreiben ins LittleFS nur bei einem neuen Schlüssel
> (§18.5a). **PWA-Polling:** unverändert. **Feste Grenzen:** Tabellenliste bis 256 Byte, Notes 3'072,
> STM-Liste nach E3.0 — Überlänge wird nie gekürzt; `req_max_path` 31 Zeichen (Diagnose).

**Secure by design** — `escapeHtml` bei jedem `innerHTML`? Fremddaten als nicht vertrauenswürdig
behandelt? Keine Credentials in Logausgaben?

> **Credentials:** C6b nimmt den Schlüssel aus dem allgemeinen Einstellungsabruf; der neue Endpunkt ist nicht
> zwischenspeicherbar und wird nur vom Backup-Export gerufen. Eine fremde Seite kann keinen der beiden lesen
> (keine CORS-Freigabe, §8.3); curl im LAN liest ihn weiterhin. Kein neuer Log gibt einen Wert aus; Berichte
> nennen den Schlüssel nur als Prüfsumme. **Fremddaten:** NTP an die eigene Anfrage gebunden (C59); B48
> setzt `textContent`; B8 über den bestehenden `data-i18n`-Weg. **Zerstörende Wege: in diesem Paket
> bewusst nicht geschützt** (Ent-18). Die Prüfung über `Sec-Fetch-*` wirkt im Browser über http nicht
> (L439); C6, C6v, C60, C61 und C62 klärt eine eigene Spec. Bis dahin können fremde Seiten im LAN diese Wege
> auslösen — ein benannter, offener Punkt in `BEFUNDE.md`, kein übersehener. *Stand der Freigabe:* „Erstmals
> hat jeder Weg eine Herkunftsprüfung; die fünf API-Endpunkte der PWA sind zusätzlich nur per POST
> erreichbar."
>
> **E3 (C65):** Notes und Listen vom Update-Server sind Fremddaten über HTTP. Sie werden **roh** abgelegt
> und erst beim Ausliefern maskiert bzw. gefiltert — mit **derselben** Maskierung und demselben Filter wie
> live; die Länge ist begrenzt, die Kopfzeile wird geprüft. Der Auftrag fragt **nie** den eingebauten Server
> des Ursprungsprojekts (Host leer ⇒ kein Abruf, L42, DIR-009). `req_max_path` trägt **nie** die Query —
> dort können SSID und Schlüssel stehen (`network_client_set`). Die Dateinamen in `fs write short` stammen
> aus der Weissliste der Assets. `heap_low_reset=1` setzt nur Diagnosewerte zurück — kein neuer Endpunkt, der
> Konfiguration schreibt; bewusst ohne Herkunftsprüfung, weil ein fremder Aufruf höchstens eine Messung
> stört.

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
> gescheiterter PWA-Upload in EP hat einen benannten Rückweg (§8.4). *(Der Prüfpunkt „Kopfzeilenschleife im
> POST-Zweig nach C53" entfällt mit §8.2.)*
>
> **E3 (C65):** `fs write short` war eine **stille Verwerfung** im Sinne der Checkliste — nur auf der UART,
> ohne Zähler, nicht im Ring (L185-Gattung); künftig gezählt und im Ring. **Abbruch mitten in einer
> Sequenz:** Der Cache wird über eine Temporärdatei geschrieben und erst nach vollständigem Lesen und
> Schreiben umbenannt; jeder Abbruch lässt die alte, gültige Datei stehen. Eine halbe Datei gilt über die
> Längenprüfung als fehlend. Eine abgerissene Tabellenliste wird nie als vollständig abgelegt; heute liefert
> sie still eine Teilliste, künftig eine leere mit kurzem Fenster. Überlänge wird nie gekürzt. **Flag
> „fällig":** gelöscht nur nach Erfolg oder bei gleichem Schlüssel; nach Fehlschlag bleibt es mit 10 min
> Abstand — kein Dauerfeuer, kein Vergessen. Das Rücksetzen der Messwerte geschieht im selben Handler wie das
> Lesen. I-3 misst auf **jedem** Rückkehrpfad. Die Leseschleifen in `httpclient.cpp` bleiben unverändert (der
> Messpunkt sitzt nach `connect()`, vor jedem Lesen); die Attrappe in c65b bildet `connected()` nach FIN 0
> ab. Die OTA-Wege behalten ihre 5-s-Grenze, ausdrücklich gesetzt (§18.6).

---

## Verworfene Alternativen

| Alternative | Warum nicht |
|---|---|
| A56 mit Layoutversion 3.0 | Ent-9: Rückflash setzte alle Einstellungen zurück |
| A56 im Bereich der alten Overlay-Intervalle (Offset 407) | Altbestand, ohne Layoutversion nicht unterscheidbar |
| B55 am unveränderten Wert / an der Logzeile / als neue Variable / asynchron | §6.1; asynchron widerspricht Ent-6 |
| C8 Ringzeilen 255 | Ent-10, 4'320 Byte RAM |
| C6 zweistufig | Ent-11 (war die Empfehlung); mit Ent-18 entfällt die POST-Umstellung ganz |
| C6 POST ohne `Sec-Fetch-Site` | fremde Formular-POSTs gingen durch; mit Ent-18 Gegenstand der Schutz-Spec (dort: `Origin` gegen `Host`, L439) |
| C6 Legacy auf POST | Ent-12; POST-Zweig liest keine Formularrümpfe; Ent-12 aufgehoben, Gegenstand der Schutz-Spec |
| Herkunftsprüfung über `Sec-Fetch-*` ausliefern (E1) | wirkt im Browser über http nicht (L439); ein Schutz, der keiner ist, erzeugt ungedecktes Vertrauen (Ent-18) |
| C6b schon in E2 (Stand der Freigabe: in E1) | öffnete das Fenster mit leeren Schlüsseln bis EP (§0.2) |
| EP als reines PWA-Release nach E2 | C6b koppelt ESP und PWA; die alte PWA exportierte leere Schlüssel (§8.4) |
| C6b in drei Schritten (`network_secrets` additiv in E2, Export in EP, Schlüssel aus `eeprom_settings` später) | braucht ein weiteres ESP-Release; Ziel 9 erst damit erfüllt (§8.4) |
| ~~Werkzeuge vor EP auf POST~~ | entfällt mit Ent-18 (Stand der Freigabe: sie liefen dann gegen den alten ESP ins Leere) |
| Messbau für A15 am Gerät (Ent-15) | aufgehoben 13.10.2026: seit LTO kein `bl` in der ISR, der Gewinn sind wenige Befehle (L432) |
| Messbau über den Update-Server | belegte die Quelle der produktiven Updates mit einem Diagnosestand (§2.3) |
| A70 feste zweite Frist | schnitte den IP-Lauftext bei grossem `ticker_deceleration` ab |
| A80 Icon gar nicht anfordern | zweite Stelle, die dieselbe Regel kennen müsste |
| C65: Release Notes im RAM zwischenspeichern | bis 3'072 Byte dauerhaft — genau der Heap, um den es geht (`http.cpp:12601-12604`) |
| C65: Release Notes nur auf Anforderung (`?notes=1`, Ent-20 A, Empfehlung im Entwurf) | Nutzerentscheid C: Die Notes sollen sichtbar bleiben, ohne PWA-Änderung |
| C65: Release Notes mit `?notes=0` abschaltbar (Ent-20 B) | ohne PWA-Release keine Wirkung |
| C65: Cache-Dateien bei jedem Start neu schreiben | Flashverschleiss je Neustart, auch je STM-Reset (L183); der Schlüssel macht es unnötig |
| C65: Hardware im Schlüssel der Cache-Dateien | nach jedem ESP-Start ist die Hardware zunächst 65535 (L42) — jeder Start schriebe neu |
| C65: Notes bei ungültiger Datei live in der Anfrage holen | genau die Last, die M-1 nehmen soll; der Auftrag füllt die Datei ausserhalb der Anfrage |
| C65: Tabellenliste ebenfalls ins LittleFS | keine Version, an der ein Neuschreiben hinge; der RAM-Zwischenspeicher genügt |
| C65: Listen nur auf Anforderung statt zwischenspeichern | die alte PWA zeigte keine STM-Dateien und keine Tabellen mehr |
| C65: Zwischenspeicher im Hintergrund periodisch vorwärmen | erzeugt ausgehende Last zu Zeitpunkten, die niemand steuert; der Auftrag läuft nur beim Start und bei neuem Schlüssel |
| C65: das Tief weiter mit `device_ready` 2/s messen | trägt seine eigene Last (M-A, M-D) und sieht das Tief zwischen zwei Anfragen nicht — eine Obergrenze, kein Messwert |
| C65: Sendezeitgrenze 500 ms | bräche bei jedem kurzen Funkloch ab; der Gewinn gegenüber 1'000 ms ist für die 3-s-Frist nicht nötig |
| C65: einheitlich 1'200 ms | zwei Versuche plus `flush()`/`stop()` ergäben 3'000 ms — ohne Spielraum |
| C65: M-2 und M-4 gleich mit in E3 | Ent-19; M-2 ändert das Verhalten gegenüber jeder Gegenstelle und braucht eine eigene Messung nach E3 |
| C65: Brücke in `http_flush()` lesen statt M-3 | grösserer Eingriff in die Brückenlogik mitten in jeder Antwort; erst nach I-3 entscheiden (§18.10) |
| C65: zweistufig E3a/E3b (Ent-27 B) | ein Build, ein OTA, ein Tag mehr; kalt/warm belegt die Annahme hinter M-1 im selben Stand |

## Versionsfolgen

- [ ] STM `src/main.h` anheben — S1, S2 (und S3), je einzeln
- [ ] ESP `version.h` anheben — E2, EP, **E3** (E1 entfällt)
- [ ] App `APP_VERSION` anheben — EP und P2 (E3 nicht)
- [ ] `CACHE_NAME` in `sw.js` anheben — EP und P2, je mit `APP_VERSION` (E3 nicht)

Nur der `release-engineer` führt das aus (R4).
