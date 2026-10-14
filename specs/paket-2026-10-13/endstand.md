# Endstand — Paket 2026-10-13 „Mittlere Befunde und offene Entscheidungen" (Z.2)

**Erstellt:** 2026-10-14, `spec-writer`, Task Z.2 aus `tasks.md`. Stand `d1150b9` (Zweig `pwa-decoupling`), Quelle
`BEFUNDE.md` in diesem Stand: Messprotokolle L433, L435, L441, L443, L448, L450, L457, L466, dazu L423, L449,
L456, L460, L461 und die Befundzeilen mit den Z.1-Schliessungen.

Momentaufnahme (DIR-006). Der lebende Stand steht in `BEFUNDE.md`.

**Teil dieser Spec.** Dieses Dokument trägt den Endstand **jedes** Akzeptanzkriteriums aus `requirements.md` und
`requirements-e4.md` und **jedes** Tasks aus `tasks.md`, dazu die Abweichungen vom Design (`design.md`,
`design-e4.md`). Es steht in einer eigenen Datei aus demselben Grund wie `requirements-e4.md`: Dem `spec-writer`
stand in dieser Sitzung kein Werkzeug für Teiländerungen zur Verfügung, und ein Neuschreiben der freigegebenen
Dateien mit rund 330 bis 1'215 Zeilen hätte das Risiko einer stillen Abweichung im freigegebenen Text getragen.
**Die Kästchen `- [ ]` in `requirements.md` und `requirements-e4.md` sind deshalb nicht nachgezogen — massgeblich
ist diese Datei.** Die Statusspalten in `tasks.md` sind nachgezogen, dort steht auch die Kurzfassung
(„Endstand 14.10.2026").

**Endstände.** Jedes Kriterium hat genau einen:

| Endstand | heisst |
|---|---|
| **erfüllt** | Beleg mit L-Nummer, Uhrzeit, Messwert oder Prüfstand mit Fallzahl |
| **nicht erfüllt** | gemessen und verfehlt; Messwert und offener Träger in `BEFUNDE.md` |
| **entfallen** | durch einen Entscheid aus dem Paket genommen oder abgelöst |
| **nicht belegt** | ob es erfüllt ist, sagt kein Messprotokoll; der Grund steht dabei |

Wo ein Teil eines Kriteriums belegt ist und ein anderer nicht, gilt **nicht belegt** und der belegte Teil steht
dabei. Wo ein Kriterium mit einer festgehaltenen Abweichung erfüllt ist, steht die Abweichung dabei.

---

## 1. Releases

Sieben Releases, kein S3 (kein Gate-Riss in S2). Abgelesen aus `.git/packed-refs` und `.git/refs/tags/release/`
(der `spec-writer` hat kein `Bash`, also nicht über `git tag -l`); jedes Tag hat einen CHANGELOG-Eintrag mit
demselben Namen.

| Release | Tag | Komponenten | ZIP | Messprotokoll |
|---|---|---|---|---|
| Ausgangsstand | `release/3.2.32-3.2.33-1.4.97` | — | — | — |
| **S1** | `release/3.2.33-3.2.33-1.4.97` | STM 3.2.33 | `wordclock-release-2026-10-13-214013.zip` | L433 |
| **S2** | `release/3.2.34-3.2.33-1.4.97` | STM 3.2.34 | `wordclock-release-2026-10-13-225707.zip` | L435 |
| **E2** | `release/3.2.34-3.2.34-1.4.97` | ESP 3.2.34 | `wordclock-release-2026-10-14-004425.zip` | L441 |
| **EP** | `release/3.2.34-3.2.35-1.4.98` | ESP 3.2.35, PWA 1.4.98 | `wordclock-release-2026-10-14-074533.zip` | L443 |
| **P2** | `release/3.2.34-3.2.35-1.4.99` | PWA 1.4.99 | `wordclock-release-2026-10-14-101643.zip` | L448 |
| **E3** | `release/3.2.34-3.2.36-1.4.99` | ESP 3.2.36 | `wordclock-release-2026-10-14-153447.zip` | L450 |
| **E4** | `release/3.2.34-3.2.37-1.4.100` | ESP 3.2.37, PWA 1.4.100 | `wordclock-release-2026-10-14-180012.zip` | L457 |

---

## 2. Zählung

| Quelle | Kriterien | erfüllt | nicht erfüllt | entfallen | nicht belegt |
|---|---|---|---|---|---|
| `requirements.md` | 94 | 69 | 4 | 9 | 12 |
| `requirements-e4.md` | 21 | 15 | 3 | 1 | 2 |
| **zusammen** | **115** | **84** | **7** | **10** | **14** |

Gezählt ist jede Kennung einzeln (AKN.1 bis AKN.4 sind vier, AKH.1 bis AKH.7 sieben). Nachgezählt gegen die
Kennungen in den beiden Dateien: `requirements.md` 12 (Schritt 0) + 4 (S1) + 14 (S2) + 4 (E1) + 11 (E2) + 12 (EP)
+ 4 (P2) + 21 (E3) + 2 (Flash) + 10 (alle) = 94; `requirements-e4.md` AKV.1 bis AKV.21 = 21.

---

## 3. Kriterien aus `requirements.md`

### Schritt 0 — ohne Firmware

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKW.1 | erfüllt | L423: `pmset -g log` 12.10.2026 00:00–01:59 ohne Schlaf- oder Wachwechsel, „Caffeinated" durchgehend; Deckel 00:20:06, Wechsel auf Akku 00:20:18; als „belegt"/„nicht belegt" eingetragen |
| AKW.2 | erfüllt | L423: Herzschlag `.git/watch-log.lebt`, `--pruefe` Exit 0/1/3, Stop-Hook ruft `--pruefe`. Abweichung (Review M4): Grenze `max(3·Intervall, Intervall+Frist+10)` statt `3·Intervall` |
| AKW.3 | erfüllt | L423: Intervall 30, 100 s `SIGSTOP` ⇒ „hat 105 s ausgesetzt" |
| AKW.4 | erfüllt | L423: Frist 20 s je Abruf; Wortlaut „Mitschnitt nicht erreichbar: Abruf hing …" (Review M4) |
| AKW.5 | erfüllt | L423: hängender Erstabruf ⇒ „Grundlinie FEHLT" in der Mitschrift; hängende Runde ⇒ kein „ruhig"; 100 s `SIGSTOP` ⇒ `--pruefe` rc 1 und Selbstmeldung; `kill -9` ⇒ rc 3, Hook meldet „BEENDET". Probe (ii) über `SIGSTOP` statt über einen künstlich gealterten Herzschlag |
| AKB.1 | erfüllt | `BEFUNDE.md` F3: Stufe S17, 59 `*_set` im Dispatch, Gegenzählung gegen die URL-Tabelle der PWA 59 = 59, 54 vom Import aufgerufen, Namensliste ausgegeben |
| AKB.2 | erfüllt | F3: `tools/checks/backup-umfang.ausnahmen`, 5 Einträge mit Grund; grundloser oder erfundener Eintrag ⇒ FEHL |
| AKB.3 | erfüllt | F3: Setter aus einer `app.js`-Kopie entfernt ⇒ FEHL `display_it_is_set`; erfundener Eintrag ⇒ FEHL; über `guardrails.sh` BLOCKIERT, rc 1 |
| AKD.1 | erfüllt | `BEFUNDE.md` B8 (Task 0.5): 74 + 22 Kandidaten, 5 deutsche UI-Texte, Ausnahmeliste mit Grund; Gegenprobe (neuer `announceStatus`-Text) gemeldet |
| AKG.0 | erfüllt | L433: (1) F103 91/90/90, F411 87/90/89 aus dem Worktree-Bau des Tags; (2) G0 13.10.2026 21:38, `d=0 o=0 v=0/0 a=0`, `rx` 73/1024; (3) `free=7512 max=6992`, `ovr=1`; (4) längste Zeile 82 Zeichen. Wie viele Heap-Zeilen in (3) eingingen, nennt L433 nicht |
| AKG.1 | erfüllt | L429 (zweite C19-Stelle), L430 (120er-Ring), L431 (Rechnung Netzwerkimport); „Träger: Paket 2026-10-13" an den Einträgen; S10/S11 im `--full` vor S1 Exit 0 (L433) |
| AKZ.12 | nicht belegt | Werkzeug gebaut (CHANGELOG, Eintrag STM 3.2.33). Die Gegenprobe „falscher Dateiname ⇒ Abbruch ohne Flash" steht in keinem Messprotokoll. Im Paket nicht eingesetzt (Ent-15 aufgehoben) |

### Schritt S1 — A15

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKI.1 | erfüllt | L433, `BEFUNDE.md` A15: Prüfstand a15, 12'000 Zufallsfolgen, 11 Randfälle, 0 Abweichungen, gegen HEAD FEHL; S16 60/60 |
| AKI.2 | erfüllt | L433: F103 USART1/2/3 91/90/90 → 82/82/82, F411 USART1/2/6 87/90/89 → 80/82/82; `SR` einmal gelesen, nicht mehr geschrieben (`strh` 5 → 4) |
| AKI.4 | entfallen | Ent-15 aufgehoben (L432) |
| AKI.3 | erfüllt | L433: G1 13.10.2026 21:44–22:44, Setter-Burst `d=0 v=0/0 a=0`, 365 `diag`-Zeilen lückenlos, kein Neustart ausser dem Flash |

### Schritt S2 — B55-STM, A56, A81, A80, A82, A70

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKL.1 | erfüllt | L435: Prüfstand l55, 2'004 Fälle, gegen HEAD FEHL |
| AKA.1 | erfüllt | L435: Prüfstand a56, 65'576 Prüfungen samt Rückflash, gegen HEAD FEHL |
| AKA.2 | erfüllt | L435 (G2, K20): je Wechsel genau ein `eep a=1979 n=2 … ms=15`; beim Start ohne Wechsel kein Schreiben (I2) |
| AKA.4 | erfüllt | L435: Prüfstand a81, 8 Varianten je rund 6'900 Prüfungen, gegen HEAD FEHL. Am Gerät nicht prüfbar (kein Setter für die Marker-Farbe, K21) |
| AKA.5 | erfüllt | L435: Prüfstand a82, je 1'455'168 Prüfungen, gegen HEAD FEHL; G2 K23 `display=0 ambi=0 dcf77=1` / `display=1 ambi=1 dcf77=0` |
| AKC.1 | erfüllt | L435: Prüfstand a80, Restore-Bedingung statisch verglichen, gegen HEAD FEHL |
| AKN.1 | erfüllt | L435: Prüfstand a70 (1'453 Szenario-Prüfungen), gegen 3.2.32 FEHL. Abweichung des Umsetzers: Frist mit 9 statt `TICKER_COLS + 1` Spalten (nie zu kurz) |
| AKN.2 | erfüllt | L435: a70, gegen 3.2.32 FEHL. Abweichung: „IP-Lauftext läuft" zeitbasiert statt als Merker |
| AKN.3 | erfüllt | L435: a70, `start_ticker()` als einziger Startweg fremder Ticker (5 Stellen), gegen 3.2.32 FEHL |
| AKN.4 | erfüllt | L435: a70, gegen 3.2.32 FEHL |
| AKN.5 | erfüllt | L435: Restore-Bedingung wörtlich (Review S2, `main.c:4910-4916`), nur der Vergleich im Innenzweig getauscht; a69 weiter OK |
| AKA.3 | erfüllt | L435: K20 13.10.2026 22:59, STM-Reset 22:59:49 ⇒ `power_status` Ambilight off; zurück auf on |
| AKC.2 | erfüllt | L435 (K22a): Display aus, Icon-Overlay „Heart" ⇒ `icon: verworfen, display aus`, kein `icon_freeze: ENTER`, nach dem Einschalten kein Icon; in T2 wiederholt (K22a 21:32:41, K22b 21:32:54–58, L466). **Abweichung:** Auslöser Overlay statt `weather_get_now` — dieser löst kein Icon aus (L435) |
| AKN.6 | nicht belegt | Belegt ist A70 (e): STM-Reset 14.10.2026 01:05:11 im Aus-Fenster ⇒ `set_display_power: new=0` im selben Augenblick wie die Wetterantwort, kein Wetter-Lauftext (L435). Der Timerteil („Found Timer" am Abend und am Morgen) steht in keinem Messprotokoll (`BEFUNDE.md` A70) |

### Schritt E1 — entfallen (Ent-18)

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKH.1, AKH.2, AKH.3, AKH.4 | entfallen | Ent-18 (L439): Browser senden `Sec-Fetch-*` über `http://` nicht; Schutz in eigener Spec (C6, C6v, C60, C61, C62) |

### Schritt E2 — B55-ESP, C8, A79, C59, dazu C46 und C19

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKQ.1 | erfüllt | Prüfstand c46 (70 Fälle, L437), im E2-Stand S16 71/71 (L441); am Gerät 14.10.2026 01:08 Kennung 9 und `- abgewiesen: query 302 Zeichen` (L441) |
| AKU.1 | erfüllt | Prüfstand c19 (28 Fälle, gegen 3.2.33 FEHL, L437), S16 71/71 (L441). Am Gerät nie (`design.md` §17.3) |
| AKL.2 | erfüllt | L441: Prüfstand l55e (14 Fälle), gegen HEAD FEHL; G4 S25/S26/S25b über die API |
| AKK.1 | erfüllt | L441: Prüfstand c8, 20 Fälle, gegen HEAD FEHL |
| AKE.1 | erfüllt | L441: a79 statisch, Review E2.7 |
| AKT.1 | erfüllt | L441: Prüfstand ntp, 72 Fälle, 25 Sabotagen, gegen 3.2.33 FEHL. Abweichung (Review E2.7 M2): Frist 60 s statt 5 s |
| AKM.2 | erfüllt | L441 (G4 Teil 2): 129 Heap-Zeilen, kleinstes `free` 6'760 und `max` 6'376 gegen G0 7'512/6'992 ⇒ −752 bzw. −616, innerhalb 1'200. Offen gelegt: Lastspitze 01:19:19 `max` 5'648 vor dem Fenster (gegen G0 −1'344); RAM je E2-Task nur geschätzt, gemessen am Release-Build (BSS +176, RODATA +36) |
| AKE.2 | erfüllt | L441: OTA 01:08, STM-Resets 01:13 und 01:16, je `ovr=0`, kein `- ovr #` in den ersten 2 Minuten |
| AKK.2 | nicht belegt | L441: die Vorhersagezeile war kurz, Folgeeinträge wurden nicht ausgelöst; T2 K29 „nicht aussagekräftig", längste Zeile 83 Zeichen (L466). Träger **C8** |
| AKT.2 | erfüllt | L441: 01:21–03:30, Netzzeit 02:19:40 und 03:22:40 übernommen, 0 × `ntp verworfen` |
| AKQ.2 | erfüllt | L441: 300 Zeichen ⇒ Kennung 9, 240 ⇒ gewohnte Antwort |

### Schritt EP — C6b, C46-PWA, B55-PWA

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKH.5 | entfallen | Ent-18 |
| AKS.1 | erfüllt | L443: Prüfstand c6b, gegen HEAD FEHL; am Gerät 07:48 `key`/`ap_key` leer, `key_set`/`ap_key_set` true |
| AKS.2 | erfüllt | L443: `network_secrets` per GET mit `no-store`, ohne CORS; c6b mit drei Einzelsabotagen. Der Schutz des Endpunkts bleibt offen (Schutz-Spec, `BEFUNDE.md` Abschnitt „Entscheidung: Schutz …") |
| AKH.6 | entfallen | Ent-18 |
| AKS.3 | erfüllt | L443: Prüfstand ep (50 Fälle; Abbruch bei 404, Netzfehler, `ok:false`, Nicht-JSON); Export mit `wifi_key` (Länge 40) in T1 (L449) |
| AKQ.3 | erfüllt | L443: ep, längste Query 210 Zeichen, Import je Feld; der Netzwerkimport selbst ist am Gerät nicht gefahren (Backup-Import, R5; L431) |
| AKL.3 | erfüllt | L443: ep, Kennungen 10/7/11/ok in DE und EN; Banner wörtlich in T2 (L466) |
| AKZ.11 | erfüllt | L443: `preview/server.py` mit `network_secrets`, `snapshot-device.sh --restore` offline in sechs Fällen geprüft |
| AKG.5 | erfüllt | L443: OTA 07:46:44, PWA-Upload 07:47:10–07:47:32 ohne Aktion dazwischen; Smoketest 31/0 samt Update-Quelle, `--check` vollständig, `check-pwa.sh` 9/0 |
| AKH.7 | entfallen | Ent-18; lesender Teil in AKS.4 |
| AKL.4 | erfüllt | L466, `BEFUNDE.md` B55: T2 19:37–19:39, Banner wörtlich, `ldr: minimum abgewiesen (26 >= max 1)`, `ldr: maximum abgewiesen (22 <= min 4094)`, Übernahme mit `eep a=225`/`227`, Endstand 0/4095, `v=6/0` vorher und nachher. Abweichung: nur mit eingeschalteter Auto-Helligkeit fahrbar (L465) |
| AKS.4 | erfüllt | L443 (07:48); Export mit `wifi_key` in T1, Wert nie ausgegeben (L449) |

### Schritt P2 — B48, B8

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKF.1 | erfüllt | L448: Prüfstand b48, 102 Fälle, gegen HEAD FEHL; in T2 E1 an 13 Feldern Banner und Fehlerzeile (L466) |
| AKF.2 | nicht belegt | Die Rückmeldung des Nutzers vom iPhone liegt nicht vor (`BEFUNDE.md` B48). Träger **B48** |
| AKD.2 | erfüllt | L448: Stufe S18 meldet 0; Gegenproben ⇒ Exit 1; nach Review P2.6 liest der Tokenizer `app.js` bis zur letzten Zeile und gibt das aus |
| AKD.3 | nicht erfüllt | `check-pwa.sh` 9/0 (L448). EN-Stichprobe in T2 (K33, L466): die lokale App-Dateiliste bleibt deutsch („fehlt", `app.js:10829`), Statuswerte der Hauptseite bis zum nächsten Takt deutsch. Träger **B63** |

### Schritt E3 — C65

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKR.1 | erfüllt | L450 (E3.6): Prüfstand c65a, gegen 3.2.35 FEHL; 19 von 19 Sabotagen, nach Review E3.8 26 von 26 |
| AKR.2 | erfüllt | L450: Prüfstand c65b, gegen 3.2.35 FEHL |
| AKR.3 | erfüllt | L450: c65a, gegen 3.2.35 FEHL |
| AKR.4 | erfüllt | L450: Prüfstand c65m, gegen 3.2.35 FEHL |
| AKR.5 | erfüllt | L450: Prüfstände c65n und c65e (Review E3.8); am Gerät kalt Δ`httpclient_connects` 6, nicht 8 (G7R, L457) |
| AKR.6 | erfüllt | L450: c65m mit Schlüsselvergleich |
| AKR.7 | erfüllt | L450: Prüfstand c65t, gegen 3.2.35 FEHL |
| AKR.8 | erfüllt | L450: Build E3 +504 Byte „Globale Variablen" (Grenze 700), Programm +6'560. Je Task nur auf Objektebene gemessen; ein Zwischenstand lag bei +917 und wurde vor dem Build durch Verlagern der Literale gesenkt |
| AKR.9 | erfüllt | L450 (E3.7): Gegenprobe über `WATCHLOG_QUELLE`, alle drei gemeldet, `unveraendert` still |
| AKR.10 | erfüllt | L450 (E3.7): Gegenproben über `SMOKE_DR_OVERRIDE` |
| AKR.11 | entfallen | **Abgelöst durch AKV.15** (`requirements-e4.md`, „Was E4 an `requirements.md` ändert"). Auf dem Stand E3 nicht erfüllt: `check-pwa.sh` 8/1, `ERR_CONNECTION_RESET` bei `/api/network_scan` (L450); auf E4 9/0 (L457) |
| AKR.20 | erfüllt | L450: 15:36:42 `- update cache upd-releasenote.txt geschrieben n=720`, 15:36:43 `upd-wc-list.txt … n=69` |
| AKR.12 | nicht erfüllt | Gefahren als G7R auf dem Stand E4 (AKV.21): Ruhe nach Last `max` **4'464** < 5'120 (L457). (i) auf E3 nicht gemessen (G7 nach zwei Ladungen angehalten); auf E4 nach dem Start 7'184. Träger **C71** |
| AKR.13 | nicht belegt | G7R (L457): kalt Δ`httpclient_connects` 6, warm **ebenfalls** 6 — der Zwischenspeicher gilt 60 s, das Update-Modul fragt alle 15 s; kein Paar gültig. Die Wirkung von M-1 ist nur am Prüfstand c65m belegt |
| AKR.14 | erfüllt | Berichtet, kein Abnahmekriterium: G7R 1'752 gegen 1'448 (L449) in gleicher Messweise (L457) |
| AKR.15 | erfüllt | G7R 18:43–19:02: keine `keine Quittung` (L457) |
| AKR.16 | nicht erfüllt | G7.5 (L457): `- weather fc=1 ms=4702 timeout n=697 cl=3829 oom=0` — kein Speichermangel mehr, aber die Lesefrist lief ab statt `ok`. Träger **C54** |
| AKR.17 | nicht belegt | Zero-Window-Teil entfallen (Ent-39; vom Mac aus nicht herstellbar, L450). Ringteil: im G7R-Fenster kam keine Zeile `http write lost`/`fs write short` (L457); textgleich ist nur am Prüfstand c65b belegt |
| AKR.18 | nicht belegt | `req_max_ms` je G7R-Lauf steht nicht im Messprotokoll; belegt ist nur G8 Lauf 1, 1'613 ms bei `network_scan` (L457) |
| AKR.19 | erfüllt | G7R: keine Exception, kein Neustart, Wache lebendig (L457). `diag` und `ovr` überwacht die Logwache; als Zahl nicht protokolliert |
| AKR.21 | erfüllt | L466: V3 21:38:20, nach dem Start beide Cache-Dateien `unveraendert`, kein `geschrieben` |

### Flash, Matrix, RAM

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKM.1 | erfüllt | L433 (S1.2), L435 (S2.2–S2.12): je Task F103 und F411, F103 nach jedem Task ≥ 1'504 Byte frei |
| AKM.3 | erfüllt | L433: 72/72, engste Variante 1'520 Byte; L435: 72/72, engste 1'172 — 148 über dem Gate (⇒ A85, L451) |

### Für alle Schritte

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKZ.1 | nicht belegt | Guardrails nach jedem Task sind nicht einzeln protokolliert; belegt sind die `--full`-Läufe vor jedem Release (AKZ.2) |
| AKZ.2 | erfüllt | `--full` Exit 0 vor S1 (L433), S2 (L435), E2 (L441), EP (L443), P2 (L448), E3 (L450), E4 (L457). Warnungsbestand: ab EP drei ältere ESP-Warnungen (C64, L445), danach unverändert |
| AKZ.3 | nicht belegt | Keine Ausgabe von `git diff --name-only` in einem Messprotokoll, für keines der sieben Releases |
| AKZ.4 | erfüllt | Smoketest S1 31/0 (L433), S2 31/0 (L435), E2 31/0 (L441), EP 31/0 (L443), P2 30/1 — der Fehlschlag ist `write_lost_failed=2` aus T1 (L448) —, E3 32/0 (L450), E4 33/0 (L457); nach ESP-Updates Update-Quelle und `install-app.sh --check` |
| AKZ.5 | erfüllt | Wache bei den Geräteläufen belegt; Gegenproben für `icon` (L435), `- rpc keine Antwort` (L441), `- abgewiesen` (L437), `fs write short` und `update …` (L450), `- scan` (L457), dazu E38 (`fail=`, `low=`). Lücke: eine Gegenprobe für die neuen `ntp verworfen`-Gründe und `--pruefe` Exit 0 je Lauf stehen nicht wörtlich im Protokoll |
| AKZ.6 | nicht erfüllt | T1 an K30 gestoppt (L449). T2 abgenommen bis auf K30: **21:28:37 `low=752`** < 1'072, `max` 3'584, beim Neuladen der PWA in Englisch mit einem Client (L466); Phase 3 106 von 114. Träger **C65** |
| AKZ.7 | nicht belegt | Commit und Tag je Release vorhanden (§1, CHANGELOG). Push der Tags im Messprotokoll nur für E3 und E4 (L457); der Zweig steht gleichauf mit `origin` (`d1150b9`), ob alle sieben Tags am Remote liegen, konnte der `spec-writer` nicht prüfen |
| AKZ.8 | nicht belegt | Die Einspielzeilen gingen an den Nutzer und stehen in keinem Messprotokoll; Reihenfolge je Release im CHANGELOG |
| AKZ.9 | nicht belegt | Belegt für ESP-Dateien in E2 (`ESP-uclock.ino` CRLF 2'058 → 2'099, `http.cpp` 1, L441), EP (L443), E3 (L450); für STM- und PWA-Dateien und für E4 kein Beleg im Messprotokoll |
| AKZ.10 | erfüllt | S9 und S11 in jedem `--full` (AKZ.2) |

---

## 4. Kriterien aus `requirements-e4.md`

| Kriterium | Endstand | Beleg oder Grund |
|---|---|---|
| AKV.1 | erfüllt | L457 (E4.8): Prüfstand c65i; 47 von 47 Sabotagen, gegen 3.2.36 FEHL |
| AKV.2 | erfüllt | c65i; am Gerät `- scan n=12 vor=6448/4672 nach=5480/3704 flush=4136/3056 ms=1587/3` (L457) |
| AKV.3 | erfüllt | Prüfstand c65q, gegen 1.4.99 FEHL; am Gerät je `check-pwa` genau 1 `network_scan` (L457). Bekannte Lücke: Wechsel vor der ersten Kernladung (B60, L459) |
| AKV.4 | erfüllt | Prüfstand c65w, c65b angepasst und grün (L457) |
| AKV.5 | erfüllt | Prüfstand c65k (L457) |
| AKV.6 | erfüllt | c65q 18 von 22 FEHL gegen 1.4.99 (dort bis zu 6 gleichzeitige `/api/`); am Gerät Startphase 1, danach 2 (L457) |
| AKV.7 | erfüllt | c65q; am Gerät 0 × `display_power`/`ambilight_power`/`power_status`; fehlt ein Wert, „nicht verfügbar", nie „aus" (L457) |
| AKV.8 | erfüllt | c65q. Lead-Entscheid: die Ladesperre hält bis zum Ende der Nebenabrufe, das Promise von `loadData()` erfüllt sich nach der ersten Darstellung — Review E4.10: verletzt AKV.8 nicht (L457) |
| AKV.9 | entfallen | Lead-Entscheid E4.7 (L457): „Fristen als Literal unverändert (Frist ab Senden)". Die Tabelle aus `design-e4.md` §19.6 (+1'000 ms je lesende Frist, Ent-35) ist damit nicht umgesetzt |
| AKV.10 | erfüllt | Prüfstand c65z; Link gelungen (L457). Befund: Das Core-Makro `TCP_TW_LIMIT(l)` lässt l+1 übrig, umgesetzt als `TCP_TW_LIMIT (HTTP_TW_LIMIT - 1)` (Lead; Review am Core nachgerechnet) |
| AKV.11 | erfüllt | Prüfstand c65o, Weissliste 16 Einträge je Handler gelesen; Review M1: `update_status?refresh=1` ist ein Auslöser und wird nie übersprungen; G8.6 Δ`api_orphan_skipped` 1 bzw. 0 (L457) |
| AKV.12 | erfüllt | Objektebene +58, Build E4 +68 Byte (Grenze 200), Programm +2'704 (L457) |
| AKV.13 | nicht belegt | (ii) Smoketest-Gegenproben und (iii) Wache belegt; (i) nie endender Abruf ⇒ Exit 1 belegt. Nicht belegt: die Gegenprobe „alte Fassung FEHL bei 1'200 ms" trifft nicht zu — die alte besteht ebenfalls —, und die Zählung „Verbindungsaufbau ≥ 900 ms" ist ungeprüft (Review M2) (L457) |
| AKV.14 | nicht belegt | Wie AKZ.3: keine Ausgabe gegen das Tag von E3 im Messprotokoll |
| AKV.15 | erfüllt | L457: App 18:16:48, ESP-OTA 18:17:11, 3.2.37 um 18:17:38, dazwischen nichts; Smoketest 33/0, `--check` vollständig, `check-pwa` 9/0. Der Rückweg in den Einspielzeilen steht nicht im Messprotokoll |
| AKV.16 | erfüllt | L457: erster Lauf 18:21, `heap_low_block` 2'184, Tor bestanden. Abweichung G8.1: rund 20 s statt 150 s Ruhe vor dem Rücksetzen |
| AKV.17 | erfüllt | L457: drei Läufe `heap_low_block` 2'184 / 1'488 / 1'784, `low=` ebenso, Δ`fs_write_short` 0, Δ`write_lost_failed` 0, 0 × `keine Quittung` |
| AKV.18 | nicht erfüllt | L457, L460: Ruhewert `max` **4'672** < 5'120, gefallen schon beim Startabgleich des Zwischenspeichers. Träger **C71** |
| AKV.19 | nicht erfüllt | (b) erfüllt; (c) wartend ≤ Backlog in allen drei Läufen; (e) G8.6 bestanden. **(d) Lauf 1 `heap_low_tw` 3 > 2** (L457); (a) G8.5 nicht gefahren (kein Werkzeug), der Scan ist nur über `check-pwa` belegt; die Zählung ≥ 900 ms ist kein Beleg (Review M2). Träger **C65** |
| AKV.20 | erfüllt | L457 (G8.7): kein Neustart ausser dem OTA, keine Exception, kein Watchdog, Wache durchgehend lebendig |
| AKV.21 | nicht erfüllt | G7R 18:43–19:02 nach Nutzerentscheid (L460): AKR.12 (ii) 4'464 und AKR.16 Zeitüberschreitung nicht erfüllt, AKR.13 und AKR.18 nicht belegt, AKR.15 und AKR.19 erfüllt, AKR.14 berichtet (L457). Träger **C71**, **C54** |

---

## 5. Tasks aus `tasks.md`

Statusspalten in `tasks.md`: ☑ = erledigt mit Beleg, — = entfallen, ☐ = Abnahme ohne Beleg. Spalten ohne
Kästchen (Gerätenachweise, Messtasks) tragen ihren Endstand nur hier.

### Schritt 0 und M0

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| 0.1 | erledigt | L423; AKW.1–AKW.5 erfüllt |
| 0.2 | erledigt | Review im zweiten Durchgang bestanden (L423) |
| 0.3 | erledigt | S17 (`BEFUNDE.md` F3); AKB.1–AKB.3 erfüllt. **Die einmalige dynamische Gegenprobe aus `design.md` §15 ist nicht belegt**; F3 bleibt deshalb in `BEFUNDE.md` offen |
| 0.4 | erledigt | Review 0.4 bestanden, M1/M2 nachgerüstet (F3) |
| 0.5 | erledigt | `BEFUNDE.md` B8; AKD.1 erfüllt |
| 0.6 | nicht belegt | Werkzeug `--lokal` gebaut (CHANGELOG STM 3.2.33); AKZ.12 und die Hook-Probe mit `curl -X POST` stehen in keinem Messprotokoll |
| 0.7 | erledigt | G0 13.10.2026 21:38 und Befehlszahlen vorher (L433); AKG.0 erfüllt |
| 0.8 | erledigt | L429, L430, L431; AKG.1 erfüllt |
| M.1, M.2 | erledigt, nicht eingesetzt | `tools/checks/auszug/stm/a15/messbau.patch` liegt (Ent-15 aufgehoben) |
| M.3, M.4 | entfallen | Ent-15 aufgehoben |

### Schritt S1

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| S1.1 | erledigt | Prüfstand a15 (L433) |
| S1.2 | erledigt | F103 −88 Byte, frei 1'744; F411 −64 (L433) |
| S1.3 | erledigt | Befehlszahlen, a15 in S16 60/60 (L433) |
| S1.4 | erledigt | Freigabe, beide Plattformen, alle drei Instanzen (L433) |
| S1.5 | erledigt | STM 3.2.33 (Tag S1) |
| S1.6 | erledigt | CHANGELOG „STM 3.2.33" |
| S1.7 | nicht belegt | AKZ.3 |
| S1.8 | erledigt | `--full` Exit 0, 22/22 Hook-Proben, S16 60/60, S17 grün (L433) |
| S1.9, S1.10 | entfallen | Ent-15 aufgehoben |
| S1.11 | erledigt | 72/72, engste Variante 1'520 (L433) |
| S1.12 | erledigt | ZIP und Tag S1 (§1) |
| S1.13 | erledigt | `flash-stm.sh` 21:42:05–21:43:00, Smoketest 31/0 (L433) |
| S1.14 | erledigt | G1 21:44–22:44; AKI.3 erfüllt (L433) |

### Schritt S2 und S3

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| S2.1, S2.2 | erledigt | l55; F103 +52, frei 1'692 (L435) |
| S2.3, S2.4 | erledigt | a56; F103 +68, frei 1'624 (L435) |
| S2.5, S2.6 | erledigt | a81, Klemmung ergänzt; F103 +40, frei 1'584 (L435) |
| S2.7, S2.8 | erledigt | a80; F103 +48, frei 1'536 (L435) |
| S2.9, S2.10 | erledigt | a82; F103 +32, frei 1'504 — kein Riss (L435) |
| S2.11, S2.12 | erledigt | a70; F103 −36, frei 1'540 — kein Riss (L435) |
| S2.13 | erledigt | Prüfstände in S16 (66/66 beim Release-Build), je gegen HEAD FEHL (L435) |
| S2.14 | erledigt | `icon: verworfen` Hinweis, `icon_freeze: ENTER` Alarm, Gegenprobe an der Attrappe (L435) |
| S2.15 | erledigt | Review S2 Freigabe; Auflagen F1 → A72, F2 → E2, F3 → A84 (L435) |
| S2.16 | erledigt | STM 3.2.34 (Tag S2) |
| S2.17 | erledigt | CHANGELOG „STM 3.2.34" |
| S2.18 | erledigt | K20 bis K24 im Testplan, in G2 gefahren (L435) |
| S2.19 | nicht belegt | AKZ.3 |
| S2.20 | erledigt | `--full` Exit 0, 22/22, S16 66/66 (L435) |
| S2.21 | erledigt | 72/72, engste 1'172 (L435) |
| S2.22 | erledigt | ZIP und Tag S2 (§1) |
| S2.23 | erledigt | `flash-stm.sh` 22:58:14–22:59:08, Smoketest 31/0 (L435) |
| S2.24 | teilweise | G2: AKA.3, AKC.2 erfüllt; **AKN.6 nicht belegt** (Timerteil) |
| S3.1–S3.8 | entfallen | Kein Gate-Riss in S2 (Ent-14, Ent-17 nicht ausgelöst; F103 nie unter 1'504, L435) |

### Schritt E1

Endstand wie in der Tabelle in `tasks.md` (Stand 13.10.2026, Ent-18) — nichts nachzutragen.

### Schritt E2

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| E2.0 | erledigt, mittelbar belegt | S16 71/71 mit c46 und c19 (L441), Review E2.7 bestätigt C46/C19 und den Stand ohne Herkunftsteil; die eigene Zeile „Baum nach dem Rückbau" fehlt im Messprotokoll E2 |
| E2.1–E2.4 | erledigt | L441 |
| E2.5 | erledigt | l55e 14, c8 20, a79 statisch, ntp 72 Fälle; S16 71/71 (L441) |
| E2.6 | erledigt | `- rpc keine Antwort #` als Alarm, Gegenprobe an der Attrappe (L441) |
| E2.7 | erledigt | Review bestanden; M2, N3 vor dem Build behoben; M1 ⇒ L442/C73 |
| E2.8 | erledigt | ESP 3.2.34 (Tag E2) |
| E2.9 | erledigt | CHANGELOG „ESP 3.2.34", `--full` Exit 0 (L441); AKZ.3 nicht belegt |
| E2.10 | erledigt | `TESTPLAN-PWA.md`: K25/K26 als entfallen, K27/K28 E2 zugeordnet; K27a/b in T1 gefahren (L449) |
| E2.11 | nicht belegt | AKZ.8 |
| E2.12 | erledigt | ZIP und Tag E2 (§1) |
| E2.13 | erledigt | OTA 01:08:05, Smoketest 31/0, Update-Quelle, `--check` vollständig (L441) |
| E2.14 | teilweise | G4: AKQ.2, AKE.2, AKT.2, AKM.2 erfüllt; **AKK.2 nicht belegt** |

### Schritt EP und T1

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| EP.1, EP.14 | entfallen | Ent-18 |
| EP.2 | erledigt | L443 |
| EP.3 | erledigt | c6b, gegen HEAD 7 Fehler, drei Einzelsabotagen (L443) |
| EP.4–EP.6 | erledigt | L443 |
| EP.7 | erledigt | ep 42, nach Review 50 Fälle; Werkzeuge (L443) |
| EP.8 | erledigt | Freigabe mit Auflagen; M1 nach Nutzerentscheid, M2 behoben; N4 ⇒ C63 (L443) |
| EP.9 | erledigt | Testplan Backup-Export und B55; in T1 gefahren (L449) |
| EP.10 | erledigt | ESP 3.2.35, PWA 1.4.98 (Tag EP) |
| EP.11 | erledigt | CHANGELOG „ESP 3.2.35 und PWA 1.4.98" |
| EP.12 | nicht belegt | AKZ.3 |
| EP.13 | erledigt | `--full` Exit 0, S16 73/73; drei ESP-Warnungen älter als EP (C64, L443, L445) |
| EP.15 | nicht belegt | AKZ.8; Reihenfolge und Warnung „kein Backup-Export dazwischen" im CHANGELOG |
| EP.16 | erledigt | ZIP und Tag EP (§1) |
| EP.17 | erledigt | AKG.5 erfüllt (L443) |
| EP.18 | erledigt | G5: AKS.4 erfüllt (L443); AKL.4 ging an T2 und ist dort erfüllt (L466) |
| T1 | nicht abgenommen | An K30 gestoppt, lesender Teil 51 von 56 (L449); Rest an T2 |

### Schritt P2

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| P2.1 | erledigt | Kontrast Text 7,37–8,84:1, Rand 7,36:1 bzw. 6,54:1 (L448) |
| P2.2–P2.4 | erledigt | L448 (Lead-Entscheide `<span>`, Ausweitung der Meldefunktionen) |
| P2.5 | erledigt | S18, b48 102 Fälle, S16 74/74 (L448) |
| P2.6 | erledigt | erst keine Freigabe (K1: S18 sah 5'800 Zeilen nicht), Nachreview Freigabe mit Auflagen (L448) |
| P2.7 | erledigt | Testplan B48 (10.1 iPhone) und EN-Stichprobe; K33 in T2 gefahren (L466) |
| P2.8 | erledigt | PWA 1.4.99, `CACHE_NAME` v91 (L448) |
| P2.9 | erledigt | `--full` Exit 0, S16 74/74 (L448); AKZ.3 nicht belegt |
| P2.10 | nicht belegt | AKZ.8 |
| P2.11 | erledigt | ZIP und Tag P2 (§1) |
| P2.12 | erledigt | 10:47 `install-app.sh`, `--check` vollständig, `check-pwa.sh` 9/0, Smoketest 30/1 (Zähler aus T1) (L448) |
| P2.13 | offen | G6: **AKD.3 nicht erfüllt** (B63), **AKF.2 nicht belegt** (B48) |

### Schritt E3

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| E3.0 | erledigt | L450 (Listen, LittleFS, Platzschwelle 171'813, Server-Dateien, RAM des Ausgangsstands) |
| E3.1–E3.5, E3.4a | erledigt | L450; RAM auf Objektebene +505 |
| E3.6 | erledigt | S16 75/75, alle fünf gegen 3.2.35 FEHL, 19/19 Sabotagen (L450) |
| E3.7 | erledigt | Wache, Smoketest (L450). Die Zero-Window-Probe ist gebaut, vom Mac aus aber nicht herstellbar (Exit 3 „nicht prüfbar") |
| E3.8 | erledigt | Freigabe mit Auflagen, M1/M2/N4 behoben, c65e, 26/26 Sabotagen (L450) |
| E3.9 | erledigt | ESP 3.2.36 (Tag E3) |
| E3.10 | erledigt | `--full` Exit 0, S16 75/75 (L450); AKZ.3 nicht belegt |
| E3.11 | erledigt | K30 nach Ent-23 im Testplan; in T2 angewandt (L466) |
| E3.12 | erledigt | Träger an C65, C49, C54; C66–C69 (L452–L455) |
| E3.13 | nicht belegt | AKZ.8; Hinweis zum Startabruf im CHANGELOG |
| E3.14 | erledigt | ZIP und Tag E3 (§1) |
| E3.15 | erledigt | OTA 15:35:03, Smoketest 32/0, `--check` vollständig; `check-pwa.sh` 8/1 ⇒ AKR.11 auf E3 nicht erfüllt, abgelöst durch AKV.15 (L450) |
| E3.16 | angehalten | Nach zwei Ladungen an K30 (`heap_low_block` 280, `heap_low_free` 688, Pfad `flush`); AKR.20 erfüllt; Rest als G7R (E4.20) (L450) |
| E3.17 | erledigt | Angehalten, vorgelegt, Nutzerentscheid Ent-28 (L450, L456) |

### Schritt E4

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| E4.0 | erledigt | Ausgangswerte Build E3 (L457). Die Vorher-Werte der PWA stehen über c65q gegen 1.4.99 im Messprotokoll (E4.8: bis zu 6 gleichzeitige `/api/`, 30 Abrufe `display_power`/`ambilight_power`) |
| E4.1–E4.5 | erledigt | L457; Befund `TCP_TW_LIMIT`, Weissliste 16, RAM +58. **Zusatz ausserhalb der Spec:** E4.5a (Lead, nach Meldung des Nutzers) — Release Notes und `wc-list.txt` nach jedem Start und nach `refresh=1` abgeglichen |
| E4.6, E4.7 | erledigt | L457; Lead-Entscheide Ladesperre und „Fristen unverändert" (⇒ AKV.9 entfallen) |
| E4.8 | erledigt | 47/47 Sabotagen, c65q 22 Fälle (18 FEHL gegen 1.4.99), S16 76/76 (L457) |
| E4.9 | erledigt | Smoketest, Wache, `check-pwa` mit Netzruhe (L457); AKV.13 nicht belegt |
| E4.10 | erledigt | Freigabe mit Auflagen H1, M1, M2, N1–N3 (L457); N3 ⇒ B56 (L458) |
| E4.11 | erledigt | ESP 3.2.37, PWA 1.4.100, `CACHE_NAME` v92 (L457) |
| E4.12 | erledigt | `--full` Exit 0, S16 76/76, RAM +68 (L457); AKV.14 nicht belegt |
| E4.13 | erledigt | Testplan (`device_ready`-Felder, K30 mit Tor); in T2 angewandt (L466) |
| E4.14 | erledigt | „Nicht in E4" an C66, C67, C68, C70; M-2 an C65 (`BEFUNDE.md`) |
| E4.15 | nicht belegt | AKZ.8: Einspielzeilen samt Rückweg nicht im Messprotokoll; Reihenfolge im CHANGELOG |
| E4.16 | erledigt | Commit `f21d518`, ZIP und Tag E4 (§1, L457) |
| E4.17 | erledigt | AKV.15 erfüllt (L457) |
| E4.18 | teilweise | G8 18:21–18:29: AKV.16, AKV.17, AKV.20 erfüllt; **AKV.18 und AKV.19 nicht erfüllt** (L457) |
| E4.19 | erledigt | AKV.18 nicht erfüllt ⇒ vorgelegt; Nutzerentscheid: T2 trotzdem, Haltegrenze für den Ruhewert in diesem Lauf 4'096 (L460) |
| E4.20 | erledigt, Ziel verfehlt | G7R 18:43–19:02: sieben Ladungen, `heap_low_block` 1'336–1'792, Tor jedes Mal bestanden; **AKV.21 nicht erfüllt** (L457) |

### Schritt T2 und Z

| Task | Endstand | Beleg oder Grund |
|---|---|---|
| T2 | abgenommen mit Befund | 19:11–21:44 (L466): AKL.4 und AKR.21 erfüllt; **K30 gerissen** 21:28:37 `low=752`; Teil 1 hielt 20:05 an K30 (Safari des Nutzers, L461); **AKZ.6 nicht erfüllt** |
| Z.1 | erledigt | Commit `d1150b9` |
| Z.2 | erledigt | diese Datei und die Statusspalten in `tasks.md` |

---

## 6. Design — Abweichungen und Nicht-Belegtes

### `design.md`

- **§0.2 Release-Zuschnitt:** sieben Releases wie geplant (S1, S2, E2, EP, P2, E3, E4); S3 entfallen, kein Gate-Riss.
- **§1.1/§1.2 Flash F103:** Die engste Variante steht nach S2 bei 1'172 Byte frei, 148 über dem Gate; der Abstand
  Teilbuild ↔ engste Variante wuchs auf 368 Byte (L435) ⇒ A85 (L451).
- **§4 A70:** Frist mit 9 statt `TICKER_COLS + 1` Spalten, „IP-Lauftext läuft" zeitbasiert (Umsetzer, L435).
- **§5 A80 / §17.2 Zeile G2:** Der Gerätenachweis lief über ein Icon-Overlay; `weather_get_now` löst kein Icon aus
  (L435).
- **§11 C59:** Frist 60 s statt 5 s (Review E2.7 M2, L441).
- **§15 F3:** Die **einmalige dynamische Gegenprobe** (Import in der Vorschau, aufgerufene `*_set` gezählt) ist
  **nicht belegt**. Die statische Stufe S17 ist belegt (AKB.1–AKB.3).
- **§16 E36:** Grenze `max(3·Intervall, Intervall+Frist+10)` statt `3·Intervall` (Review M4, L423).
- **§18.3 I-2:** Die Entscheidungsregel trägt am Gerät: E3 `snd=1072` ⇒ Heap (L450), T2 `snd=0` ⇒ Gegenstelle
  (L463).
- **§18.5/§18.8 M-1 kalt gegen warm:** am Gerät nicht trennbar (AKR.13 nicht belegt); kalt Δ6 bestätigt den Sollwert
  aus E3.0 (L457).
- **§18.5a:** Die Lücke „geänderte Notes bei gleicher Version nie geholt" schloss E4.5a (L457). Ob
  `upd-wc-list.txt` nach jedem Start neu geschrieben wird: nein — nach V3 `unveraendert` (AKR.21, L466).
- **§18.8 G7.4 Zero-Window-Probe:** vom Mac aus nicht herstellbar (L450), mit Ent-39 entfallen.
- **§18.9 K30:** In T2 galt für den Ruhewert nach Nutzerentscheid 4'096 statt 5'120 (L460); die Grenze `low=` 1'072
  blieb und riss (L466).
- **§18.10 Folgebefunde:** C66 (L452), C67 (L453), C68 (L454), C69 (L455); M-2 ging in E4 auf.

### `design-e4.md`

- **§19.2 I-4:** Die Zustände werden nur am Tiefstwert festgehalten; in T2 ging die Momentaufnahme zum Tief
  `low=752` durch V3 verloren (L466), bei der Safari-Sitzung fehlte eine laufende Sicht (L461 ⇒ C72).
- **§19.3 Scan:** Wechsel vor der ersten Kernladung löst keinen Scan aus (B60, L459).
- **§19.6 Netzfristen:** nicht umgesetzt, Lead-Entscheid „Fristen als Literal unverändert" (L457) ⇒ AKV.9 entfallen.
- **§19.7 TIME_WAIT:** `TCP_TW_LIMIT(l)` lässt l+1 übrig ⇒ `TCP_TW_LIMIT (HTTP_TW_LIMIT - 1)`; die Grenze gilt nach
  jedem `accept()`, dazwischen kam in G8 Lauf 1 eine dritte dazu (L457).
- **§19.10 Werkzeuge:** Die Annahme „alte `check-pwa`-Fassung FEHL bei 1'200 ms" traf nicht zu; die Zählung
  „Verbindungsaufbau ≥ 900 ms" ist ungeprüft (Review M2, L457).
- **§19.13 G8:** G8.1 mit rund 20 s statt 150 s Ruhe; G8.5 nicht gefahren (kein Werkzeug); AKV.18 verfehlt.
- **§19.14 G7R und T2:** nach Nutzerentscheid trotz AKV.18 gefahren (L460).

---

## 7. Was offen bleibt

| Offen | Kriterium | Träger in `BEFUNDE.md` |
|---|---|---|
| Tief unter Last: ein volles Neuladen reisst K30 (`low=752`, T2) | AKZ.6, AKV.19, AKV.21 | **C65** (hoch); Hebel C70, C71, C72 |
| Ruhewert nach Start und Last unter 5'120 (4'672 bzw. 4'464) | AKV.18, AKR.12 | **C71** |
| Vorhersage während einer Ladung läuft in die Lesefrist | AKR.16 | **C54** |
| Antwortverluste an einen langsamen Client (`snd=0`) | — | **C49** (L463) |
| Lange STM-Zeile in `/api/stm32_log` am Gerät | AKK.2 | **C8** |
| Fehlerzeile bei offener Tastatur am iPhone | AKF.2 | **B48** |
| Englische Oberfläche: App-Dateiliste, Statuswerte | AKD.3 | **B63** |
| Dynamische Gegenprobe des Backup-Umfangs | design §15 | **F3** |

**Nicht belegt, ohne offenen Träger:** AKN.6 (Timerteil; A70 ist geschlossen), AKZ.12, AKZ.1, AKZ.3, AKZ.7, AKZ.8,
AKZ.9, AKR.13, AKR.17, AKR.18, AKV.13, AKV.14. Nachholbar ist davon nur, was ein Gerätelauf noch zeigen kann (AKN.6,
AKR.13 mit einem Messaufbau, der den 60-s-Zwischenspeicher berücksichtigt); die übrigen sind Nachweislücken wie in
L424.

**Neue Befunde aus dem Paket** (eigene Einträge, nicht Teil dieses Endstands): A83 (L434), A84 (L436), A85 (L451),
B58 (L446), B59 (L447), B60 (L459), B61 (L462), B62 (L464), B63, C63 (L444), C64 (L445), C66–C70, C71 (L460), C72
(L461), C73 (L442), E37; E38 ist erledigt. Nicht Teil dieses Pakets und weiter offen: der Schutz der zerstörerischen
Aufrufe (C6, C6v, C60, C61, C62; Ent-18).
