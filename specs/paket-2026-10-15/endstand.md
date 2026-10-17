# Endstand — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (Z.2)

**Erstellt:** 2026-10-17, `spec-writer`, Task Z.2 aus `tasks.md`. Stand `ccdd88b` (Zweig `hardware/esp32s3-umbau`),
Quelle `BEFUNDE.md` in diesem Stand: Messprotokoll **L468** (Abschnitte 0.1, 0.4, Trennmessung, R1.1 bis R2b.2, G9.1,
G9.2, G9.6, G9b.1 bis G9b.5, G10.1 bis G10.4, Tor G10.6/G10.7, G10b.1 bis G10b.6), **L493** (T3), L456, L460, L461,
L472, L473, L475 bis L484, L497, L498 und die Befundzeilen C65, C70, C71, C72, C75, C80 mit den Schliessungen aus Z.1
(Commit `ccdd88b`). Dazu `CHANGELOG.md` (Einträge ESP 3.2.38 bis 3.2.41), `TESTPLAN-PWA.md` (Beschreibung der
Werkzeuge) und die Tags unter `.git/refs/tags/release/`.

Momentaufnahme (DIR-006), **Endstand vom 17.10.2026**. Der lebende Stand steht in `BEFUNDE.md`; was dort nach diesem
Tag nachgetragen wird, ändert diese Datei nicht.

**Teil dieser Spec, und warum eine eigene Datei.** Dieses Dokument trägt den Endstand **jedes** Akzeptanzkriteriums aus
`requirements.md` und die Abweichungen vom Design (`design.md`). Den Endstand **jedes Tasks** trägt `tasks.md` selbst,
im Abschnitt „Endstand 17.10.2026“ am Ende. Die eigene Datei hat denselben Grund wie im Paket 2026-10-13
(`specs/paket-2026-10-13/endstand.md`): Dem `spec-writer` steht in dieser Sitzung kein Werkzeug für Teiländerungen zur
Verfügung, und ein Neuschreiben der freigegebenen Dateien mit 699 (`requirements.md`) bzw. 1'013 Zeilen (`design.md`)
trüge das Risiko einer stillen Abweichung im freigegebenen Text. **Die Kästchen `- [ ]` in `requirements.md` sind
deshalb nicht nachgezogen — massgeblich ist diese Datei.**

**Endstände.** Jedes Kriterium hat genau einen:

| Endstand | heisst |
|---|---|
| **belegt** | Beleg mit L-Nummer und Abschnitt, Uhrzeit, Messwert, Commit oder Prüfstand mit Fallzahl |
| **entfallen** | durch einen Entscheid aus dem Paket genommen; Entscheid und Grund stehen dabei |
| **offen** | ein Teil des Belegs fehlt im Messprotokoll; was fehlt, steht dabei, und wie es nachzuholen wäre |

**Kein Kriterium ist gemessen und verfehlt.** Jedes offene ist offen, weil ein verlangter Beleg im Protokoll fehlt —
meist eine Gegenprobe oder ein Vergleich gegen den Vorstand —, nicht weil eine Messung ihr Soll gerissen hätte.

**Lesart, nach der eingeordnet ist:**

1. Ein Kriterium mit **Soll** (Grenzwert, Erwartung, „Gegenprobe ⇒ FEHL“) braucht für **jedes** Soll einen Beleg. Fehlt
   einer, ist es **offen**; der belegte Teil steht dabei.
2. Ein Kriterium, das **protokollieren oder berichten** verlangt, ist belegt, wenn das Protokoll die Grössen nennt, auf
   denen seine Auswertung oder ein Entscheid beruht — auch als Spanne. Hilfsgrössen (Ports, einzelne Felder je Laden)
   dürfen in der Mitschrift stehen.
3. Eine **Messbedingung**, die nicht ausgewiesen ist, steht als Vermerk. Belegt bleibt das Kriterium nur, wenn
   begründet ist, warum das Ergebnis nicht von ihr abhängt.
4. **„Ausreichend belegt“** steht nur mit Begründung, nie stillschweigend. So behandelt sind die Lücken, die Z.1
   benannt hat: C75 (bei AKS.1), U1b (ii) (bei AKG.3) und `heap_low_block` im warmen Fenster (bei AKG.2).
5. Nennt ein Kriterium ausdrücklich einen **Prüfstand** als Instrument, genügt dessen Beleg; ein Gerätenachweis wird
   nicht zusätzlich verlangt.

---

## 1. Releases

Vier Releases, R3 entfallen (Ent-8). Tags abgelesen aus `.git/refs/tags/release/` (der `spec-writer` hat kein `Bash`,
also nicht über `git tag -l`); ZIP und Commit aus `CHANGELOG.md` und L468.

| Release | Tag | Komponenten | ZIP | Release-Commit | Eingespielt (L468) |
|---|---|---|---|---|---|
| Ausgangsstand | `release/3.2.34-3.2.37-1.4.100` | — | — | — | — |
| **R1** | `release/3.2.34-3.2.38-1.4.100` | ESP 3.2.38 | `wordclock-release-2026-10-16-010338.zip` | im Protokoll nicht genannt | 16.10.2026 01:07:56 (OTA) |
| **R1b** | `release/3.2.34-3.2.39-1.4.100` | ESP 3.2.39 | `wordclock-release-2026-10-16-080958.zip` | `d4c5cee` (Code `cb01fcd`) | 16.10.2026 08:14 |
| **R2** | `release/3.2.34-3.2.40-1.4.101` | ESP 3.2.40, PWA 1.4.101 | `wordclock-release-2026-10-16-113409.zip` | `820a232` | 16.10.2026 11:39, Firmware zuerst |
| **R2b** | `release/3.2.34-3.2.41-1.4.102` | ESP 3.2.41, PWA 1.4.102 | `wordclock-release-2026-10-16-145745.zip` | `5f1491e` | 16.10.2026 15:02, Firmware zuerst |
| R3 | — | — | — | — | **entfallen** (Ent-8) |

Die vier Tags liegen **lokal** vor. Ob sie am Remote liegen, sagt das Messprotokoll nicht, und ohne `git ls-remote`
lässt es sich hier nicht prüfen — siehe AKZ.5.

---

## 2. Zählung

| Gruppe | Kriterien | belegt | entfallen | offen |
|---|---|---|---|---|
| AKZ (Schritt 0 und übergreifend) | 9 | 3 | 0 | 6 |
| AKI (R1) | 4 | 4 | 0 | 0 |
| AKM (G9) | 7 | 6 | 0 | 1 |
| AKS (L7, R1b/G9b) | 4 | 3 | 0 | 1 |
| AKC (R2) | 8 | 5 | 0 | 3 |
| AKW (G10) | 6 | 6 | 0 | 0 |
| AKE (R3/G11) | 3 | 0 | 3 | 0 |
| AKL (R2b) | 9 | 5 | 0 | 4 |
| AKG (G10b) | 7 | 2 | 0 | 5 |
| **zusammen** | **57** | **34** | **3** | **20** |

Nachgezählt gegen die Kennungen in `requirements.md`: AKZ.1, AKZ.2, AKZ.9 (Schritt 0) und AKZ.3 bis AKZ.8
(übergreifend) = 9; AKI.1–4; AKM.1–7; AKS.1–4; AKC.1–8; AKW.1–6; AKE.1–3; AKL.1–9; AKG.1–7 — zusammen 57, wie die
Zählzeile in `requirements.md` angibt. Keine weitere Kennung der Form `AK<Buchstabe>.<Zahl>` ist dort definiert.

---

## 3. Kriterien einzeln

### Schritt 0 — Grundlagen

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKZ.1 | offen | **Belegt** (L468, 0.1, 15.10.2026, ESP 3.2.37/PWA 1.4.100): Grössen von `app.js` 171'310, `i18n/en.json` 16'370, `styles.css` 13'992, `index.html` 12'002, `sw.js` 2'379, `layout-previews.json` 1'524 Byte, Icons 399–17'710; Kodierung und Zeilenende von `http.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`, `base.h`, `app.js`; `netstat`, `lsof`, `osascript` vorhanden; Prüfserver-Befund `HTTP/1.0`/`HTTP/1.1` (22:21–22:23); E4-Werte 69'060/531'552 (L457); A.1 in `design.md` §12. **Fehlt:** die Grösse **jeder** der 13 Dateien — das Manifest fehlt ganz, die Icons stehen nur als Spanne —; Kodierung und Zeilenende von `httpclient.cpp`. Am Gerät nicht mehr nachholbar (der Vorstand läuft dort nicht mehr); die Dateien des Vorstands liegen unter dem Tag `release/3.2.34-3.2.37-1.4.100` |
| AKZ.2 | offen | **Belegt:** (i) Ladeprobe mit dauerhaftem Profil, kalt/warm, Leitungsstatus über `responseReceivedExtraInfo`; Prüfserver-Gegenprobe `HTTP/1.1` ⇒ `304`, `HTTP/1.0` ⇒ kein `If-None-Match` (L468, 0.1); Fehlalarm behoben in `db47c92`, Gegenprobe „Server ohne Validator ⇒ FEHL für bekannte Pfade“ (G10.1). (ii) `browser-sitzung` in `osascript` und `manuell` gefahren (G9b.4, G10.3, G10b.5); Chrome-Wegwerfprofil, Sabotage `CHROME=/usr/bin/false` ⇒ 3 Fehlschläge, Exit 1; sichtbares Chrome ⇒ ABBRUCH, Exit 2 (L478); P1-Probe gegen Attrappen 2/1/0 (G9.2). (iii) Smoketest ab R1 mit den I-5-Feldern, Gegenprobe `SMOKE_DR_OVERRIDE` gegen 3.2.37 ⇒ FEHLT mit allen acht Feldern (L468, R1.6). (v) `check-pwa` mit Leitungsstatus, Byte, Verbindungsfehlern (`9774642`), Cache-Zählung berichtigt (`66803a3`). (vi) gegenstandslos mit Ent-2 T. (vii) Lochsumme `L` in jeder Ruhemessung ausgewiesen; Messung AKS.4 in G9b.1. **Fehlt:** (ii) Gegenprobe „Uhr nicht erreichbar ⇒ Exit ≠ 0 mit Meldung“; (iii) Gegenprobe des Smoketest-Blocks C70 (R2); (iv) Gegenprobe der Logwache über `WATCHLOG_QUELLE` mit `low=752 tcp=…`; (v) Gegenprobe der neuen INFO-Zeilen von `check-pwa`; (viii) siehe AKL.8; durchgehend der Nachweis „Konsole **und** Mitschrift“. Dazu ist das Review der Werkzeuge (Task 0.4) nicht belegt |
| AKZ.9 | belegt | L468, Trennmessung 15.10.2026 22:40–22:46: (b) `fs_remove` ohne Abruf und (c) `update_status?refresh=1` mit Schreiben, je Heap-Zeilen vorher und nachher, beide ohne bleibende Spur. Die Deutung von damals ist durch A.5 ersetzt; die Messung selbst steht |

### Schritt R1 — Instrumente

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKI.1 | belegt | L468, R1.1 (Commit `6231f86`): Prüfstand c72i, 47 Fälle × 2 und 19 statische, 17 Sabotagen schlagen an, gegen 3.2.37 FEHL; S16 77/77. Kommentar am I-4-Block berichtigt (L472). Am Gerät: Smoketest „Verbindungszustaende jetzt (R1)“ mit 11 Zuständen (L468, R1 ausgerollt). *Vermerk:* Welche der 17 Sabotagen den vier in AKI.1 genannten entsprechen, führt das Protokoll nicht einzeln |
| AKI.2 | belegt | Prüfstand c72i wie AKI.1 (Kriterium nennt c72i); am Gerät die Heap-Zeile 16.10.2026 01:08:16 `… tcp=0/0/0/0/0/0 nrt=0`. *Vermerk:* die Textgleichheit Mitschnitt/`/api/stm32_log` ist am Prüfstand belegt, am Gerät nicht eigens gezeigt |
| AKI.3 | belegt | L468, R1.3 (Commit `3643464`): Prüfstand c71i, 35 Fälle und 19 statische, 20 Sabotagen schlagen an, gegen R1.1 FEHL; S16 78/78. R1.1 ist der Stand 3.2.37 plus I-5 ohne I-6 — für c71i gleichwertig zur verlangten Gegenprobe gegen 3.2.37. Am Gerät gesehen: `- flash id=1660c4 vendor=c4`, `- heap erste anfrage`, `- update cache vorher`/`heap`/`nach` (L468). `frag=` statt `fe=` wie in `design.md` §3.3 |
| AKI.4 | belegt | L468, R1.3: „Globale Variablen“ +32 Byte gegen E4 (Grenze 60); c65* grün in S16 78/78 |

### Schritt G9 — Ausgangsstand ohne Cache (auf R1)

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKM.1 | belegt | L468, „R1 ausgerollt“ 16.10.2026: OTA 01:07:56, ESP meldet 3.2.38 um 01:08:17 (gleich dem Tag); Smoketest 34/0 samt Update-Quelle und I-5-Feldern; `install-app --check` 1.4.100 vollständig; `check-pwa` 9/0; `- flash id=1660c4 vendor=c4` im Mitschnitt; Logwache lief, meldete nur den OTA-Neustart |
| AKM.2 | belegt | L468, G9.1 (01:19–01:21): drei Läufe je 1 kalt + 5 warm, der dritte englisch; jedes Laden `200`, INM=0; deutsch 200'724–206'649, englisch 217'251–223'176 Byte; `heap_low_block` 1'720–2'960, Pfad fast immer `fs`; Vermerk Ruhewert im C71-Zustand `6912/4752`. Mit Ent-18 A als Vergleichswert für AKW.2 ersetzt durch G9b.3. *Vermerk (Lesart 2):* die I-4-Felder je Laden stehen in der Mitschrift, im Protokoll als Spanne |
| AKM.3 | belegt | Mit Ent-18 A auf R1b als G9b.4 (L468): Safari `osascript` 10:00–10:16 (zweiter Versuch; der erste war ungültig, L478) und Safari auf dem iPhone des Nutzers `manuell` 10:30–10:46; je Phase `free`/`max`, `tcp_now_list` mit `ruhe_s`, Δ`timeouts`/Δ`aborts`/Δ`accept_dead`, Mac-Seite; S2–S4 je Lücke 0. Freistellen des Clients entfällt mit L7 (`TESTPLAN-PWA.md`). *Vermerk:* Ob das iPhone das Gerät aus L461 ist, nennt das Protokoll nicht |
| AKM.4 | belegt | G9b.4 Chrome 09:43–09:59 (L468): Wegwerfprofil, S0..S4 = 180/120/180/300/180 s, 0 Fehlschläge; S2–S4 Lücke 0; Δ`accept_dead` 1 in S1. *Vermerk (Lesart 3):* Die 300 s Ruhe vor S0 sind nicht ausgewiesen. Das Ergebnis hängt nicht daran: Jede Phase wird gegen S0 derselben Sitzung gelesen, und S0 `6744/6496` ist derselbe Bezugswert wie in der Safari-Sitzung danach |
| AKM.5 | belegt | L468, G9.6 (01:29–01:48): vier Starts ohne Browser, Auswertung an `L`, Start 3 ohne Zerstückelung, Start 4 bindet 1'704 Byte; Abweichung (Start 4: `update_status` 4 s **nach** dem Abgleich) gekennzeichnet; Gegenmessung 01:57:31; Messungen M3 und M1 (02:04:49, +1'704). Ergebnis N5. Kein Start mit leerer Antwort. Das vollständige Protokoll steht in L468 |
| AKM.6 | offen | **Belegt** für G9b (G9b.5, `--pruefe` 10:47; keine Exception, kein Watchdog-Reset, keine `diag`-Lücke, kein `ovr`-Anstieg). **Fehlt** für das G9-Fenster auf R1 (16.10.2026 01:08–02:07): keine Gesundheitsauswertung und kein `watch-log.sh --pruefe` im Protokoll; dort steht nur „Logwache: nur der OTA-Neustart“ beim Einspielen. In genau diesem Zeitraum meldete die Wache Neustarts nicht zuverlässig (L475, gleitendes Fenster). Nachholbar aus dem Logger-Mitschnitt, soweit er den Zeitraum noch hält |
| AKM.7 | belegt | L468, G9.2 (01:28:32): nach 10 s FIN_WAIT_2 mit `arg=0`, `ruhe_s=9`, Mac-Seite CLOSE_WAIT; nach 25 s weg — P1 bestätigt; vor den Browser-Sitzungen protokolliert |

### Massnahme C71 — L7 (R1b, G9b)

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKS.1 | belegt | Commit `cb01fcd`; Code-Anker `httpclient.cpp:357-361` (L7a), `:203`, `:224`, `:232`, `:236-239` (L7b, L7c) nach der Befundzeile C71/C75; Prüfstand c71e in S16 (24 Fälle, 8 statische), gegen den Vorstand `e362d28` FEHL, 10 von 10 Sabotagen schlagen an, darunter „nur stop ()“, „Fehlerweg 404 mit direktem client.stop ()“ und „len == 0 offen (C75)“ (`tools/checks/auszug/esp/c71e/sabotage.py`). **C75, ausreichend belegt (Lesart 4, 5):** Eine leere Serverantwort ist am Gerät nie gezielt hergestellt worden (L473, C75). AKS.1 wird ausdrücklich am Prüfstand abgenommen, dessen Fall `len == 0` und Sabotage anschlagen; kein Kriterium dieses Pakets verlangt den Gerätenachweis für diesen Weg. *Vermerk:* Die Liste aller Aufrufer aus dem Bericht des `esp-developer` liegt nicht im Repository; das Review R1b.4 lief (Hinweise L476, L477) |
| AKS.2 | belegt | L468, G9b.2 (08:29–08:44): drei Starts, einer mit Schreiben beim Start, Ruhewert `max` je 7'152 ≥ 5'120 an Heap-Zeilen ≥ 130 s nach `nach` mit `tw=0`; nach `check-pwa.sh` und nach der Ladeprobe je 150 s später `7328/7152`. Daneben L460 (4'672), L457 (7'184), M1 (7'328/7'152) |
| AKS.3 | offen | **Belegt** (G9b.2): 8 Abgleichzeilen `unveraendert`/`geschrieben`, 0 `fehlgeschlagen`; `update_status` mit `assets_available=true` und gefüllten `esp_available`/`wc_available`/`app_available` (die Prüfung auf `app.js.gz` eingeschlossen); `install-app --check` vollständig; c65m, c65n, c71e (6) und (7) grün. **Fehlt:** „keine `*.tmp`“ (kein `fs_list` im Protokoll; von Z.1 benannt) und „`update_table_files` liefert wie vorher“ auf R1b (belegt nur auf R1 als Messung M1). Nachholbar am heutigen Stand, an dem L7 seit R1b unverändert ist: ein `/api/fs_list` ohne `*.tmp` und ein kalter `update_table_files` mit gefüllter Antwort |
| AKS.4 | belegt | L468, G9b.1 (08:19–08:29): dreimal ein kalter `update_status`, Δ`free` −56, ±0, ±0 und Δ`max` je ±0, jede Nachher-Zeile ≥ 130 s danach mit `tw=0`; auf R1 an derselben Stelle −1'704. Grundlinie der Ladeprobe auf R1b in G9b.3 (drei Läufe, 200'724–223'176 Byte, byte-gleich mit G9.1) |

### Schritt R2 — C70 Asset-Cache, Variante (a)

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKC.1 | belegt | Commit `aea887c`; Prüfstand c70e (63 Fälle, 23 statische, 27 Sabotagen), gegen den Vorstand rot (L468, R2.2; `CHANGELOG.md` 3.2.40); Smoketest-Block C70 beim Einspielen 38/0: `ETag` nach Variante T (etwa `"171315-91e4850b8a490a00"` für `app.js`), `304` als `HTTP/1.1` ohne Rumpf, falsches `ETag` ⇒ `200`. Variante V gegenstandslos (Ent-2 T). Der Vermerk aus A.6 gilt |
| AKC.2 | belegt | Prüfstand c70e; am Gerät G10.1: jede warme `/app/`-Anfrage mit `If-None-Match` und `304`, das Dokument eingeschlossen |
| AKC.3 | belegt | Variante T: c70e mit den Sabotagen aus AKC.3; am Gerät G10.2: geänderte `layout-previews.json` ⇒ neues `ETag` (`"1503-…"` statt `"1524-…"`), Original ⇒ ursprüngliches `ETag`. Variante V gegenstandslos (Ent-2 T) |
| AKC.4 | offen | **Belegt:** `app_not_modified` und `app_full` in `device_ready` (Smoketest „Asset-Zaehler (C70)“; Nachzählung G10.3a je Lauf Δ7/Δ69, deckungsgleich mit `check-pwa`). `app_etag_calc` und `- app etag` gegenstandslos (T). **Fehlt:** die Mehrdauer je Anfrage durch `seek` und 8 Byte Lesen, die das Kriterium „im Messprotokoll (◐ ms)“ verlangt — nicht gemessen. Die Ladezeiten aus G10.1 taugen dafür nicht, sie enthalten die festen Wartezeiten des Werkzeugs (L468, G10.1) |
| AKC.5 | offen | **Belegt:** Auslieferung weiter nur als `.gz` mit Grösse > 0 (c70e; Review R2.7 ohne Einwände, `CHANGELOG.md` 3.2.40); Legacy erreichbar (Smoketest „Legacy-Oberflaeche“ HTTP 200); `install-app --check` vollständig. **Fehlt:** die Stichprobe `/`, `/legacy`, `/fs`, `/update` per `curl` gegen den Vorstand — Kopf und Rumpf bis auf Zeit- und Messwerte, Statuszeile `HTTP/1.0`, ohne `ETag`. Der Smoketest prüft bei Legacy nur den Status. Statuszeile und fehlendes `ETag` sind am heutigen Stand nachholbar, der Vergleich gegen den Vorstand nur mit ihm oder statisch über den Diff der Legacy-Ausgaben zwischen den Tags von R1b und R2 |
| AKC.6 | belegt | Commit `8cf4b07`; Prüfstand c70p (30 Fälle, 7 Sabotagen), gegen den Vorstand rot; S16 81/81 |
| AKC.7 | belegt | c70e statisch (`HTTP/1.1` an genau zwei Stellen, beide mit `Connection: close`, Sabotagen in den 27) und dynamisch (Körperlänge gleich `Content-Length`); Smoketest: `304` als `HTTP/1.1`; Abbruch mit RST (`CHANGELOG.md` 3.2.40); Prüfserver-Gegenprobe mit `HTTP/1.0` (L468, 0.1) |
| AKC.8 | offen | **Belegt:** c65b, c65w und die LittleFS-Attrappe nachgezogen, S16 81/81 grün (`CHANGELOG.md` 3.2.40; L468, R2.2). **Fehlt:** die Fallzahl dieser drei gegen den Stand vor R2 — „keine Fälle verloren“ (L235) ist nicht ausgewiesen |

### Schritt G10 — mit Cache, und Tor (auf R2)

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKW.1 | belegt | L468, „R2 eingespielt“ 16.10.2026 11:39: erst ESP-OTA 3.2.40 (Neustart von der Logwache gemeldet), dann `install-app.sh` 1.4.101, `--check` vollständig; Smoketest 38/0; `check-pwa` 9/0 mit 75 Asset-Antworten (7 × `200`, 68 × `304`, 213'967 Byte). Versionen gleich dem Tag. *Vermerk:* Ob `--check` auch **vor** dem App-Upload lief, nennt das Protokoll nicht; die verlangte Reihenfolge „Firmware zuerst“ ist belegt |
| AKW.2 | belegt | L468, G10.1 (11:41–11:45): drei Läufe `--erwarte-304`, der dritte englisch; jede warme `/app/`-Anfrage mit INM und `304`, Dokument eingeschlossen; 626–835 Byte je warmem Laden (≤ 3'000); Δ`app_full` 0 ausser dem Erstabruf von `i18n/en.json`; `heap_low_block` 2'152–3'536 (≥ 1'072); kalt 7 × `200`, 206'873 Byte; gegen G9b.3 gestellt |
| AKW.3 | belegt | L468, G10.2 (11:47, `tools/probe-u.sh`, `5655780`): #2 `200` mit neuem `ETag`, übrige `304`; #4 `200` mit ursprünglichem `ETag`; danach `install-app --check` vollständig; Gegenprobe ohne Upload 3 FEHL, Exit 1 |
| AKW.4 | belegt | L468, G10.3: Chrome (11:53–12:10) S1 Δ`app_not_modified` +6, Δ`app_full` 0; Safari (12:39–12:56) S1 +4, Δ`app_full` 0 — erster Nachweis, dass Safari bei `HTTP/1.1` mit `ETag` nachfragt; je 0 Fehlschläge |
| AKW.5 | belegt | L468, G10.4: C72 auf R2 M5 (S2/S3 Lücke 0, S4 höchstens 168 unter der Schwelle 500, keine gehaltene Verbindung, Δ`timeouts` 0), daneben G9b.5 und G9.2; nachgeführt in G10.5 (`99cfc6f`); Tor G10.6 am 16.10.2026: Ent-8 kein Eingriff |
| AKW.6 | belegt | L468, G10.3a (12:56–12:59): drei `check-pwa.sh`, je 76 Asset-Antworten (7 × `200`, 69 × `304`), 214'071 Byte, 0 × `ERR_CONNECTION_RESET`, 0 × `ERR_EMPTY_RESPONSE`, Δ`write_lost_failed` und Δ`fs_write_short` 0; Gerät zählt dasselbe. Gesundheit G10.4: `--pruefe` 12:59, sieben Neustarts, alle gewollt und gemeldet, keine Exception, kein Watchdog-Reset. Grundlinie für AKG.7 |

### Schritt R2b — C70 Variante (c)

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKL.1 | belegt | Commit `ffd1184`; Prüfstand c70c (339 Fälle, 26 Sabotagen), gegen R2 rot; S16 82/82 (L468, R2b.2; `CHANGELOG.md` 3.2.41); Review R2b.9 bestanden, SOLL-1 bis SOLL-3 erledigt (`cf2d03b`); Smoketest-Block C70c beim Einspielen bestanden; am Gerät G10b.2: unter der alten Marke `no-cache`, kein `immutable` |
| AKL.2 | belegt | c70c zählt beide Zähler in jedem Fall; Smoketest C70c Δ`app_immutable` 1, Δ`app_token_miss` 3; Nachzählung in G10b.6 (je Lauf Δ`app_immutable` 5, Δ`app_token_miss` 0) |
| AKL.3 | offen | **Belegt:** c70e, c65b, c65w grün in S16 82/82; ohne `h` `no-cache` (Smoketest C70c). **Fehlt:** die `curl`-Stichprobe gegen R2 — die 13 Assets ohne `h` in Kopf und Rumpf gleich wie auf R2, Legacy (`/`, `/legacy`, `/fs`, `/update`), `/api/*`, Installations- und Hinweisseiten unverändert — und die Fallzahl der drei Prüfstände gegen R2 |
| AKL.4 | belegt | `Makefile` `057f83d`, Werkzeug `tools/app-marken.py` `9b9698a` mit 14 Gegenproben; `make` bricht bei einem Werkzeugfehler ab und lässt das Dokument unverändert (`CHANGELOG.md` 3.2.41; C70); Review R2b.9 SOLL-1 bis SOLL-3 (`cf2d03b`); am Gerät Markenprüfung 7/7 beim Einspielen, in T3 7 Ziele, 0 Abweichungen (L493). *Vermerk:* 14 Gegenproben bei neun Abbruchgründen; die Zuordnung je Grund steht nicht im Protokoll |
| AKL.5 | belegt | Review R2b.9 SOLL-1 (`cf2d03b`): `app-marken.py` prüft jede erzeugte Query (`name=wert`, `[0-9a-z.=&-]`, ≤ 64 Zeichen), Gegenprobe mit sabotiertem Werkzeug (`?v` ohne `=`) bricht ab; c70p prüft die Bildung in `app.js` (225 Fälle, 23 Sabotagen) |
| AKL.6 | belegt | Commit `28281a3`; c70p nachgezogen (225 Fälle, 23 Sabotagen), gegen 1.4.101 rot; S16 82/82 |
| AKL.7 | offen | **Belegt:** Meta-Element in Commit `d590c99`. **Fehlt:** der Lauf der PWA-Vorschau mit der Quelle — „ohne Ladefehler, die Verweise ohne Query“ — steht nicht im Protokoll. Nachholbar ohne Gerät (`/pwa-vorschau`) |
| AKL.8 | offen | **Belegt:** (i) `marken-pruefen.py` in `install-app.sh --check` (`8524d61`), am Gerät 7/7 und in T3 7/0, `--lokal` vor dem Upload (SOLL-2, SOLL-3); (ii) Block C70c (`5c6622f`) im Ernstfall bestanden; (iii) `--erwarte-cache` (`cbc80ec`) in G10b.1 bestanden, `LADEPROBE_DETAIL` (`89acb14`); (iv) `probe-u.sh` U1a–U3 (`cee7087`) in G10b.2–G10b.4 bestanden; (v) `check-pwa` INFO „aus dem Cache“ (55 je Lauf, nach `66803a3`). **Fehlt** jede verlangte Gegenprobe im Protokoll: (i) verfälschte Marke (Attrappe) ⇒ FEHL mit Dateiname; (ii) C70c gegen 3.2.40 bzw. `SMOKE_DR_OVERRIDE` ⇒ FEHLT; (iii) `--erwarte-cache` gegen R2 ⇒ „warm: Cache erwartet, 304 erhalten“, Prüfserver mit `immutable` ⇒ OK; (iv) `PROBE_U_OHNE_UPLOAD=1` ⇒ FEHL je Ablauf auf R2b — protokolliert ist nur die Gegenprobe zu AKW.3 in G10.2; dazu „Konsole **und** Mitschrift“ |
| AKL.9 | offen | **Belegt** (L468, R2b.2): „Globale Variablen“ ±0 gegen R2 (≤ 8), Paketsumme +80 (≤ 250), Programm 535'728 ⇒ 536'336 Byte (+608). **Fehlt:** die Dateiliste von R2b gegen das Tag von R2 (`git diff --stat release/3.2.34-3.2.40-1.4.101 release/3.2.34-3.2.41-1.4.102`, erwartet nur die in AKL.9 genannten Dateien) und die Grösse von `app.js.gz` vor und nach R2b |

### Schritt G10b — Wirkung von (c) (auf R2b)

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKG.1 | belegt | L468, „R2b eingespielt“ 16.10.2026 15:02: Firmware zuerst — OTA 3.2.41, danach `install-app.sh` 1.4.102 nach lokaler Markenprüfung; `--check` mit Markenprüfung 7/7; Smoketest 39/0 samt Update-Quelle und Block C70c; `check-pwa` 9/0 (gezählt nach der Berichtigung `66803a3`). Versionen gleich dem Tag. *Vermerk:* Ob der Smoketest vor dem App-Upload lief, nennt das Protokoll nicht |
| AKG.2 | offen | **Belegt** (L468, G10b.1, 15:04–15:07): drei Läufe `--erwarte-cache`, der dritte englisch; kalt 7 × `200`, versionierte Dateien mit `immutable`, 207'907 Byte; jedes warme Laden genau zwei Leitungsanfragen — Dokument und `layout-previews.json`, beide `304` —, zusammen 209 Byte (≤ 400); alles Versionierte aus dem Cache; je Laden Δ`app_not_modified` +2, Δ`app_full` 0, Δ`app_immutable` 0, Werkzeug und Gerät einig; Ausnahme `i18n/en.json`; gegen G10.1 und G9b.3 gestellt. **`heap_low_block` ≥ 1'072, ausreichend belegt (Lesart 4):** Im G10b.1-Fenster nicht ausgewiesen — `tools/ladeprobe.mjs` gibt ihn je Laden in der Zeile `Geraet:` aus, das Protokoll übernimmt ihn nicht. Auf demselben Stand R2b blieb `low=` der Heap-Zeilen über den ganzen T3-Lauf bei mindestens 1'784, PWA-Ladungen mit Cache eingeschlossen (L493: `check-pwa` mit 55 Antworten aus dem Cache), und ein warmes Laden öffnet auf R2b zwei statt sechs bis acht Verbindungen wie in G10.1, wo der Tiefstwert 2'152–3'536 betrug. **Fehlt:** der Abgleich der Pfadmenge je Laden gegen G10.1 („keiner fehlt, keiner kommt dazu“), von Z.1 benannt; dazu ist Δ`app_token_miss` = 0 im warmen Fenster nicht ausgewiesen. Nachholbar aus der Mitschrift der G10b.1-Läufe (Zeilen `Pfade:` und `Geraetezaehler`), falls sie erhalten ist, sonst mit einem neuen Lauf auf dem heutigen Stand |
| AKG.3 | offen | **Belegt** (L468, G10b.2): U1a — die ersetzte `layout-previews.json` kommt beim nächsten Laden sofort neu (Ent-20 B); U1b (i) — Abruf unter der alten Marke ⇒ `200`, neues `ETag` `13980-e49bbed14ebf0000`, `no-cache`, Δ`app_token_miss` +1; U1b (iii) — der warme Browser bleibt beim alten Inhalt aus dem Cache; Original zurück ⇒ wieder `immutable`; nach jedem Ablauf `marken-pruefen --erwartet` ohne Abweichung. **U1b (ii) frisches Profil, ausreichend belegt (Lesart 4):** Ein frisches Profil lädt das unveränderte Dokument, das `styles.css` unter der **alten** Marke nennt. Seine Anfrage ist — ohne Cache-Eintrag, ohne `If-None-Match` — dieselbe URL wie der Direktabruf in (i), und die Antwort des ESP hängt nur von Pfad, `h` und Datei ab (AKL.1), nicht vom Absender. (i) liefert den neuen Inhalt mit `no-cache`, genau das Soll von (ii). `probe-u.sh` fährt (ii) nicht eigens (`TESTPLAN-PWA.md`, Probe U). **Fehlt:** die Gegenprobe ohne Upload auf R2b (FEHL in U1a und in U1b (i)); ausserdem steht U1a nur als „sofort neu“ im Protokoll, nicht mit den Einzelschritten (#3 `304` mit neuem `ETag`, #4 `200` mit dem ursprünglichen) |
| AKG.4 | offen | **Belegt** (L468, G10b.3): #2 `styles.css` unter der neuen Marke mit altem Inhalt ohne `immutable`; #3 nach dem Datei-Upload `200` mit neuem `ETag` und `immutable`; #4 alles aus dem Cache; #5 Originale zurück, ursprüngliche URL aus dem Cache; danach `marken-pruefen --erwartet` ohne Abweichung. **Fehlt:** die Gegenprobe ohne Upload auf R2b; im Protokoll ausserdem nicht ausgewiesen: Δ`app_token_miss` = 1 in #2, der Status des Dokuments in #2, #3 und #5 und „in keinem Schritt eine andere versionierte Datei über die Leitung“ |
| AKG.5 | offen | **Belegt** (L468, G10b.4): Dokument mit `v=1.4.102.9` ⇒ jede versionierte URL `200` mit `immutable`, im folgenden Laden aus dem Cache; Original zurück ⇒ ursprüngliche URLs aus dem Cache — der Rückweg über die Version wirkt. **Fehlt:** die Gegenprobe ohne Upload auf R2b |
| AKG.6 | offen | **Belegt** (L468, G10b.5, je 0 Fehlschläge): Chrome (16.10.2026 15:18–15:35) S1 Δ`app_not_modified` +4, Δ`app_full` 0, Δ`app_immutable` 0; Safari am Mac (17.10.2026 00:41–00:58) S1 Δ`app_not_modified` +4, Δ`app_full` +1 auf eine unversionierte Datei, Δ`app_immutable` 0, Δ`app_token_miss` 0; Safari auf dem iPhone (00:58–01:15) kalt Δ`app_immutable` +3, beim Neuladen keine versionierte Datei, das Dokument jedes Mal voll (Δ`app_full` +6, rund 12 KB, gemessen und berichtet); S2–S4 Lücke höchstens 168, keine gehaltene Verbindung. Das versionierte Manifest wird **am iPhone** geladen. *Abweichungen:* „Neuladen“ in S1 bei Chrome als erneutes Öffnen (`7f1711a`, kein Cmd-R), bei Safari am Mac durch Neusetzen der Adresse. **Fehlt:** der Vermerk „versioniertes Manifest geladen?“ (Ent-22) **je Browser** — für Safari am Mac steht er nicht im Protokoll, für Chrome nur ableitbar (G10b.1 kalt 7 × `200`, dieselbe Zahl wie in G9.1, wo das Manifest genannt ist) |
| AKG.7 | belegt | L468, G10b.6 (17.10.2026 00:32–00:38): drei `check-pwa.sh` je 9/0, je 7 × `200`, 13 × `304`, 55 aus dem Cache, 209'270 Byte; 0 Verbindungsfehler gegen die Grundlinie G10.3a (je 0); Δ`write_lost_failed` und Δ`fs_write_short` 0; Ruhewert 150 s danach `7336/7104` ≥ 5'120; fünf Neustarts seit R2b, alle gewollt, keine Exception, kein Watchdog-Reset. *Vermerke:* (1) Vor G10b.6 lief ein `local_esp_restart` nach der damals vorgeschlagenen Ablaufregel aus L482. Der Nutzer hat am 17.10.2026 entschieden, diese Regel **nicht** einzuführen, sondern die Firmware-Ursache zu suchen (C80, eigene Spec). Der Ruhewert gilt damit für einen Stand nach einem Neustart; T3 zeigt auf R2b ohne diese Regel 6'112 (L493, N6), ebenfalls über K30. (2) Das Protokoll sagt „Logwache lebend“, nicht wörtlich `--pruefe` |

### Schritt R3 und G11 — entfallen

| Kriterium | Endstand | Grund und Entscheid |
|---|---|---|
| AKE.1 | entfallen | **Ent-8 kein Eingriff** (Nutzer, 16.10.2026, Tor G10.6; Zuordnung M5 auf G9b und G10, L468). Ent-10 und Ent-14 entfallen mit Ent-8; damit R3 und G11. L7 kam mit Ent-18 A in R1b, nicht in R3 |
| AKE.2 | entfallen | wie AKE.1. Die Wirkung, die AKE.2 nach einem Eingriff zeigen sollte, war ohne Eingriff schon da: Lücke 0 in S2/S3 auf R1b, R2 und R2b (G9b.4, G10.3, G10b.5) |
| AKE.3 | entfallen | wie AKE.1. Die Grundlinie, gegen die AKE.3 geprüft hätte (AKW.6), dient AKG.7 |

### Übergreifend

| Kriterium | Endstand | Beleg bzw. was fehlt |
|---|---|---|
| AKZ.3 | offen | **Belegt:** Kodierung und Zeilenende im Messprotokoll für `http.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`, `base.h`, `app.js` (L468, 0.1). **Fehlt:** die Feststellung **vor jedem Patch** im Taskbericht — die Berichte liegen nicht im Repository —, insbesondere für `httpclient.cpp` (L7) und für `index.html` und `Makefile` (R2b); dazu die Prüfung des Leads mit `git diff --stat`. Statisch nachholbar über `git diff --stat` je Paar aufeinanderfolgender Release-Tags |
| AKZ.4 | offen | **Belegt:** `CHANGELOG.md` nennt je Release die Komponenten (R1, R1b nur ESP; R2 ESP und PWA; STM unverändert). **Fehlt:** der Abgleich der **Dateien** je Release gegen das vorige Tag (keine Änderung an `src/**`, `index.html`, `styles.css`, `manifest.webmanifest` in R1, R1b, R2). Nachholbar mit `git diff --stat` zwischen `release/3.2.34-3.2.37-1.4.100`, `…-3.2.38-…`, `…-3.2.39-…` und `…-3.2.40-1.4.101`; R2b siehe AKL.9 |
| AKZ.5 | offen | **Belegt:** je Release ein ZIP, ein Tag nach DIR-011 (lokal vorhanden, Abschnitt 1) und ein CHANGELOG-Eintrag; Release-Commits R1b `d4c5cee`, R2 `820a232`, R2b `5f1491e`; Reihenfolge „Firmware zuerst“ in den Einspielzeilen von R2 und R2b; `install-app.sh --check` nach jedem OTA (L468); für R2b der Satz, dass ein ESP-Rückweg die Einträge im Browser nicht heilt, samt Rückweg über eine neue `APP_VERSION` (`CHANGELOG.md` 3.2.41). **Fehlt:** der Push samt Tag „im selben Zug“ — im Protokoll nicht vermerkt, hier ohne `git ls-remote` nicht prüfbar; der Rückweg samt Befehl für R1, R1b und R2 (der CHANGELOG nennt ihn nur für R2b); der Release-Commit von R1. Nachholbar: `git ls-remote --tags origin` gegen die vier Tags |
| AKZ.6 | belegt | L493, T3 17.10.2026 01:19–02:46 auf R2b (STM 3.2.34, ESP 3.2.41, PWA 1.4.102): Phasen 0, 1, 1b, 2, 3, 4 und 9 gefahren, „abgenommen mit Befund“; K30 nie erreicht — Tiefstwert `low=` 1'784, Ruhewert `max` am Ende 6'112, 74 Heap-Zeilen mit `ovr=0`; kein Halt, also kein I-4/I-5-Bericht fällig. Ausgelassene Fälle mit Grund in L493 (kein DFPlayer, Ambilight ausgeblendet, Dateigruppe, Flash, Display im Aus-Fenster). Die Phasen 5–8 gehören nicht zu AKZ.6 und bleiben beim Nutzer. *Vermerk:* Die Ankündigung von Beginn und Ende steht nicht im Protokoll |
| AKZ.7 | offen | **Belegt:** vor R1 `guardrails --full` 0 Kritisch, 78/78 Prüfstände, 23 Proben (L468); S16 je Release 78/78, 79/79, 81/81, 82/82 mit den Fallzahlen der neuen Prüfstände (c72i, c71i, c71e, c70e, c70p, c70c). **Fehlt:** `--full` vor R1b, R2 und R2b im Protokoll; der Guardrail-Lauf mit Exit 0 nach jedem Task (Spalte G in `tasks.md`) |
| AKZ.8 | belegt | L468: R1.1 ±0, R1.3 +32, R1b 0 (dazu `CHANGELOG.md` 3.2.39: statisch 0, Programm 534'480), R2.2 +48, R2b.2 ±0 — Paketsumme +80 von 250 (Ent-11). Programmgrössen je Schritt im Protokoll. `app.js.gz` vor und nach R2: 171'310 Byte (0.1, `fs_list`) und 171'315 (`ETag` beim Einspielen von R2). *Vermerk:* R1b steht im Protokoll nur als Teil der Paketsumme, nicht als eigene Zeile |

---

## 4. Offen — was fehlt, auf einen Blick

| Kriterium | Was fehlt | Nachholbar |
|---|---|---|
| AKZ.1 | Grösse jeder der 13 Dateien (Manifest fehlt, Icons als Spanne); Kodierung/Zeilenende `httpclient.cpp` | nur aus dem Tag des Vorstands |
| AKZ.2 | Gegenproben: `browser-sitzung` „Uhr nicht erreichbar“, Smoketest-Block C70, Logwache `low=752 tcp=…`, `check-pwa`-INFO; „Konsole und Mitschrift“; Review 0.4 | ja, ohne Gerät bzw. mit Attrappen |
| AKZ.3 | Kodierung/Zeilenende vor jedem Patch im Taskbericht; `git diff --stat` | statisch über die Tags |
| AKZ.4 | Dateiliste je Release gegen das vorige Tag | statisch über die Tags |
| AKZ.5 | Push der Tags; Rückweg für R1, R1b, R2; Release-Commit R1 | `git ls-remote`; Rückweg nicht nachträglich |
| AKZ.7 | `--full` vor R1b, R2, R2b; Guardrails je Task | nicht nachträglich für den damaligen Stand |
| AKM.6 | Gesundheit im G9-Fenster 01:08–02:07 auf R1 | aus dem Logger-Mitschnitt, soweit erhalten |
| AKS.3 | keine `*.tmp`; `update_table_files` auf R1b | ja, am heutigen Stand |
| AKC.4 | Mehrdauer je Anfrage durch `seek` und 8 Byte | ja, am heutigen Stand |
| AKC.5 | Legacy-Stichprobe per `curl` gegen den Vorstand | teilweise am heutigen Stand, sonst statisch |
| AKC.8 | Fallzahl c65b, c65w, LittleFS-Attrappe gegen vor R2 | statisch über die Tags |
| AKL.3 | `curl`-Stichprobe gegen R2; Fallzahlen gegen R2 | teilweise am heutigen Stand, sonst statisch |
| AKL.7 | PWA-Vorschau mit der Quelle | ja, ohne Gerät |
| AKL.8 | Gegenproben (i)–(iv) | mit Attrappe, `SMOKE_DR_OVERRIDE` bzw. `SMOKE_C70C_OVERRIDE`, Prüfserver, `PROBE_U_OHNE_UPLOAD=1` |
| AKL.9 | Dateiliste R2b gegen Tag R2; `app.js.gz` vor/nach R2b | statisch über die Tags |
| AKG.2 | Pfadmenge je Laden gegen G10.1; Δ`app_token_miss` im warmen Fenster | Mitschrift, sonst neuer Lauf |
| AKG.3 | Gegenprobe ohne Upload auf R2b; Einzelschritte U1a | ja, am heutigen Stand |
| AKG.4 | Gegenprobe ohne Upload auf R2b; Δ`app_token_miss` in #2, Dokumentstatus, keine andere versionierte Datei | ja, am heutigen Stand |
| AKG.5 | Gegenprobe ohne Upload auf R2b | ja, am heutigen Stand |
| AKG.6 | Manifest-Vermerk für Safari am Mac (und ausdrücklich für Chrome) | nur mit einer neuen Sitzung |

**Was davon die Schliessungen aus Z.1 berührt.** C70 ist in `BEFUNDE.md` als „erledigt“ geführt, während AKG.2 bis AKG.6
hier offen sind. Die **Wirkung** von (c) — Ziel 1b in `requirements.md` — ist belegt: warm zwei Leitungsanfragen statt
sechs bis acht, kein `immutable` unter einer fremden Marke, der Rückweg über die Version wirkt, Safari am Mac und am
iPhone halten `immutable` ein. Offen ist die **Vollständigkeit der Abnahme**: Gegenproben, ein Pfadabgleich, ein
Manifest-Vermerk. Ob C70 damit „erledigt“ bleibt oder einen Nachtrag braucht, entscheidet der Lead mit dem
`doc-writer`, nicht diese Datei. C71 (AKS.3 offen in zwei Nebenpunkten), C72 (ohne Eingriff geschlossen, AKE.*
entfallen) und C75 (statisch, siehe AKS.1) tragen ihre Schliessung mit den hier belegten Kriterien.

---

## 5. Abweichungen vom Design

| Stelle | Design | Endstand |
|---|---|---|
| `design.md` §2, §11 Releaseschnitt | R1 ⇒ R2 ⇒ R3 | **R1, R1b, R2, R2b**: R1b eingeschoben (Ent-18 A), R2b dazu (G10.7, A.6), R3 und G11 entfallen (Ent-8) |
| §4.4, AKC.3, AKZ.2 (vi) | Variante T oder V | **T** (Ent-2); alle V-Teile (CRC-32/MPEG-2, Invalidierung an 31 Stellen, `app_etag_calc`, `- app etag`) gegenstandslos |
| §5.3 Browser-Sitzung | Bedienung des vorhandenen Browsers | Chrome mit **eigenem Wegwerfprofil** (`7f1f2f7`), Vorwärmen (`6da322d`), Abbruch bei sichtbarem Chrome; Safari nur nach Sicherung der Tabs des Nutzers und mit `quit` (L478). Anlass: Die Safari-Sitzung des Nutzers ging im ersten Versuch von G9b.4 verloren |
| §5.3, §18.8 „Neuladen in S1“ | Neuladen | Chrome: erneutes Öffnen (`7f1711a`, kein Cmd-R); Safari am Mac: Neusetzen der Adresse (G10b.5) |
| §5.5 Startmessung, Start 4 | kalter `update_status` rund 50 s nach dem Start | 4 s **nach** dem Abgleich (gekennzeichnet in G9.6); am Ergebnis N5 ändert das nichts |
| §9 `check-pwa` | Leitungsstatus und Byte | zählte Treffer aus dem Speicher-Cache als `200`; berichtigt in `66803a3`, alle Werte ab R2b danach gezählt |
| §18.8 Probe U1b | auch frisches Profil | `probe-u.sh` fährt das frische Profil nicht eigens; Begründung, warum (i) es abdeckt, bei AKG.3 |
| — (nicht im Design) | — | Vor G10b.6 und vor T3 ein Neustart nach der vorgeschlagenen Ablaufregel aus L482; vom Nutzer am 17.10.2026 nicht als Regel übernommen (C80) |
| §5.4 C72, Rest aus L461 | Zuordnung des ganzen Befunds | Der Maximalanteil und die +600 Byte um 20:07 (L461) bleiben unerklärt; auf R1b, R2 und R2b traten beide nicht wieder auf (C72) |

---

## 6. Folgearbeit — nicht Teil dieses Pakets

An keinem dieser Punkte hängt ein Kriterium dieses Pakets.

- **C80** (L482) — dauerhafte Zerstückelung nach dem Einspielen, abgestuft und zufallsabhängig. **Entscheid des Nutzers
  17.10.2026: Firmware-Ursache suchen**, eigene Spec mit dem `firmware-analyst`; keine Ablaufregel „nach dem Einspielen
  neu starten“. In T3 als N6 gesehen (L493: Ruhewert 6'112 statt 7'104 nach V3 mit sofortiger PWA-Anfrage).
- **C65** bleibt offen (hoch), mit C49 und C54 (in G10b.6 einmal Vorhersage leer mit `oom=1049`).
- Aus den Reviews und Messungen dieses Pakets: C77 (Wetter-Client, L476; Normalweg ohne Befund), C78 (L479), C79
  ESP-Teil (L481), B64 (L480), C81 (L483), C82 (L484), die Hinweise H1 und H4 aus Review R2.7 (C70), E41 (Safari-Modus
  ohne Abbruch, L478), E51 (Aussetzer der Logwache, L498).
- Aus T3 (L493): L494 (Richtung der Temperaturkorrektur, B65), L495, L496; K33 (B63); die **Phasen 5–8**, Sichtprüfungen
  am Display, K18/K22/K23 bei Tag, K24 als Nachtlauf, DFPlayer und IR — bleiben beim Nutzer.
- **Task A.3** (Mitschnitt von L457/L460 auf einen Abruf ohne Rumpflesen prüfen) ist nicht ausgeführt bzw. nicht
  protokolliert. Seit L7 liegt der Ruhewert nach dem Start bei 7'152 (G9b.2); die damalige Lesart bleibt unbelegt, ohne
  Folge für ein Kriterium.
