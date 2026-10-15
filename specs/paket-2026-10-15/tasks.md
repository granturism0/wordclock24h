# Tasks — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7`. Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.
**Status: Entwurf, zur Freigabe.** Ohne Freigabe des Nutzers beginnt kein Task ausser den rein lesenden (0.1 ohne
Gerätelast, A.1). Anforderungen: `requirements.md`; Design: `design.md`.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst bei Exit 0
geht die Schreibberechtigung weiter. Wo „Nutzer“ steht, ist eine Handlung oder ein Entscheid von Dir gemeint, kein
Schreibzugriff.

**Die Spalte „Hängt ab von“ ist verbindlich, auch wo die Dateien disjunkt sind** (R3b):

1. **Messen vor Eingreifen:** R2.1 erst nach dem Entscheid am Tor (G9.7). Ergibt das Tor keinen Eingriff, entfällt
   Schritt R2 samt G10 vollständig.
2. **Analyse vor Code:** R1.1 erst nach A.1 und, falls A.1 eine Annahme der Spec widerlegt, nach A.2.
3. **Ein Schreiber je Datei, nacheinander:** I-5 (R1.1) und C70 (R1.3) schreibt **ein** `esp-developer` in
   `http.cpp` nacheinander; nach jedem ESP-Task misst der Lead „Globale Variablen“ (AKZ.9), bevor der nächste
   beginnt. Der PWA-Teil R1.5 (nur Ent-3) berührt disjunkte Dateien, zwischen ihm und dem ESP-Strang liegt kein
   Gerätenachweis — er **darf parallel** laufen. R1.6 wartet auf beide.
4. **Kein Bau und kein Einspielen während einer Messung:** Während G9 und G10 läuft kein OTA, kein Upload ausser
   Probe U, kein zweiter Browser (R5, `TESTPLAN-PWA.md` Abschnitt 0 Punkt 5). Code für R2 darf entstehen, sobald
   das Tor entschieden ist; gebaut und eingespielt wird erst nach dem Ende von G9.
5. **Firmware zuerst** (DIR-002): in R1 erst ESP-OTA, dann — mit Ent-3 — `install-app.sh`.
6. **Releaseschnitt nach Ent-9:** Mit **B** (Empfehlung) wie unten. Mit **A** wird R1 nach Teilen getrennt: R1.1,
   R1.2 und R1.6–R1.14 nur für **I-5** ⇒ G9.3–G9.7; danach R1.3–R1.5 und R1.6–R1.14 für **C70** ⇒ G9.1, G9.2,
   G9.8. Die Spalte „Task“ nennt den Teil.

**Messwerte sofort (L424):** Jeder messende Task hängt sein Ergebnis **vor dem nächsten Task** an das
Messprotokoll in `BEFUNDE.md` an. Messzeilen schreibt der Lead, Befundnachführungen der `doc-writer` — nie
gleichzeitig (R3).

**Einspielen:** Du spielst die Releases selbst ein; der Lead liefert die fertigen Zeilen samt Reihenfolge und
Rückweg (AKZ.6). Jeder Gerätelauf mit `./tools/watch-log.sh`, vorher gestartet, `--pruefe` am Ende (DIR-013); auf
später geplante Läufe mit `caffeinate` (DIR-020).

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. Prüfstände baut der Umsetzer im
Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab und fährt die Gegenproben (DIR-014). Vor jedem Patch
Kodierung **und** Zeilenende feststellen (AKZ.4) — `ESP-uclock.ino` gemischt, `http.cpp` und `stm32flash.cpp` UTF-8.

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — Grundlagen

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **Messprotokoll anlegen, Ausgangswerte ohne Last:** Zeile „Messprotokoll Paket 2026-10-15“ in `BEFUNDE.md` mit den Entscheiden; Grössen der 13 App-Dateien (`/api/fs_list`), „Globale Variablen“/Programmgrösse Build E4, Kodierung und Zeilenende von `http.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`, `base.h`, `app.js`; `netstat`, `lsof`, `osascript` vorhanden. Damit steht das Kriterium für **Ent-3** | Lead | Freigabe | **AKZ.1** (ohne Anfragenliste) | ☐ | — |
| 0.2 | **Werkzeug `ladeprobe`** nach `design.md` §5.1, mit `--erwarte-304`; Gegenproben gegen die Vorschau (FEHL) und einen Prüfserver mit `304` (OK); die Meldung kommt in Konsole und Mitschrift an | Lead | Freigabe | **AKZ.2 (i)** | ☐ | — |
| 0.3 | **Werkzeug `browser-sitzung`** nach `design.md` §5.3 (Safari und Chrome, `open`/`osascript`, Lesungen je Phase); Gegenprobe Uhr nicht erreichbar ⇒ Exit ≠ 0 | Lead | Freigabe, **Ent-6** | **AKZ.2 (ii)** | ☐ | — |
| 0.4 | **Ladeprobe auf dem Ausgangsstand** (ESP 3.2.37 / PWA 1.4.100): 1 kalt, 5 warm, Deutsch, unter dem Tor aus Ent-7; Wache läuft; Anfragenliste eines Öffnens (Deutsch und Englisch) für AKZ.1; alles sofort ins Messprotokoll | Lead | 0.1, 0.2, **Ent-7** | **AKZ.1**, **AKZ.3** | — | — |
| 0.5 | **Review der Werkzeuge** 0.2 und 0.3: Kommt jede Meldung beim Empfänger an? Sieht die Ladeprobe jede `/app/`-Anfrage (Fallzahl gegen die Anfragenliste aus 0.4)? | `code-reviewer` | 0.4 | DIR-014, L235 | — | ☐ |

## Schritt A — Analyse (lesend)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| A.1 | **Prüfpunkte P1–P8** nach `design.md` §11 am Core 3.1.2 und am Code `72cfda7`; je Punkt Antwort mit Fundstelle oder ausdrücklich „nicht belegbar, weil …“; der Lead trägt das Ergebnis ins Messprotokoll | `firmware-analyst` | Freigabe | Bericht mit P1–P8 | — | — |
| A.2 | **Nachtrag zur Spec**, nur falls A.1 eine Annahme widerlegt (etwa P1: FIN_WAIT_2 läuft nie ab; P2: PCB-Zahl unbegrenzt; P6: mehr Schreibstellen) — Änderung an Kriterien und Design als Nachtrag in diesem Ordner, Dir vorgelegt | `spec-writer` | A.1 | Nachtrag freigegeben | — | — |

---

## Schritt R1 — Release: Instrument I-5 und C70

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R1.1 | **I-5:** Felder `tcp_now_*` in `device_ready` hinter `api_orphan_skipped` nach `design.md` §3.1, Listen vollständig in den Stackpuffer vor dem ersten `http_send()`; Zählfunktion in `http.cpp` mit Prototyp in `base.h`; Heap-Zeile mit `tcp=`/`nrt=` nach Ent-4 (`ESP-uclock.ino`, gemischte Zeilenenden, byte-genau). **Prüfstand c72i** im Scratchpad | `esp-developer` | 0.1, **A.1** (A.2) | **AKI.1**, **AKI.2**, **AKI.3** am Prüfstand | ☐ | — |
| R1.2 | ESP-Teil bauen, „Globale Variablen“ und Programmgrösse sofort ins Messprotokoll | Lead | R1.1 | **AKZ.9** | — | — |
| R1.3 | **C70** nach `design.md` §4.4 (Variante nach Ent-1; mit (b)/(c) erst nach dem Nachtrag AKC.7): `If-None-Match` lesen, `ETag`-Tabelle, CRC bei Bedarf in derselben geöffneten Datei, `304`-Weg mit Grössenprüfung und `app_complete`, `app_etag_invalidate()` an **jeder** schreibenden LittleFS-Stelle (Bestand nach P6), Zähler `app_*`, Zeile `- app etag`. Im Bericht: Liste aller Schreibstellen mit Datei:Zeile und Invalidierung. **Prüfstand c70e** | `esp-developer` | R1.2 | **AKC.1–AKC.5** am Prüfstand | ☐ | — |
| R1.4 | ESP-Teil bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R1.3 | **AKZ.9** | — | — |
| R1.5 | **C70, PWA — nur mit Ent-3 = ja:** `layout-previews.json` mit `cache: "no-cache"` (`app.js:11655`), sonst nichts. **Prüfstand c70p** | `pwa-developer` | 0.1, **Ent-3** | **AKC.6** | ☐ | — |
| R1.6 | Prüfstände c72i, c70e (und c70p) unter `tools/checks/auszug/esp/c72/`, `…/esp/c70/` bzw. neben b48 ablegen, in S16 aufnehmen; Gegenproben gegen ESP 3.2.37 bzw. PWA 1.4.100; **je Massnahme mindestens eine Sabotage**; Fallzahlen nachzählen (L235); c65-Prüfstände weiter grün (AKI.3); alles ins Messprotokoll | Lead | **R1.4**, R1.5 (falls) | **AKI.1–AKI.3**, **AKC.1–AKC.6** am Prüfstand, **AKZ.8** | ☐ | — |
| R1.7 | **Werkzeuge** nach `design.md` §8: `check-pwa` INFO `304`/Byte; `smoke-device.sh` mit den neuen Feldern, Gegenprobe `SMOKE_DR_OVERRIDE`; `watch-log.sh` Heap-Zeile mit Zusatz und `- app etag`, Gegenprobe `WATCHLOG_QUELLE` (`low=752 tcp=…` ⇒ Alarm kommt an) | Lead | R1.6 | **AKZ.2 (iii)–(v)** | ☐ | — |
| R1.8 | **Review R1** gegen die Checkliste, ausdrücklich: kein `yield()` während der Listengänge; jede Schreibstelle invalidiert (gegen den Bericht aus R1.3 und P6); `304` nie ohne Grössenprüfung; `If-None-Match`-Puffer je Anfrage geleert und begrenzt; Legacy ohne `ETag`; Attrappen-Semantik in c70e; Kodierung und Zeilenenden (AKZ.4) | `code-reviewer` | R1.7 | Checkliste | — | ☐ |
| R1.9 | ESP-Version anheben; mit Ent-3 `APP_VERSION` und `CACHE_NAME` | `release-engineer` | R1.8 | S4 | ☐ | — |
| R1.10 | `CHANGELOG.md`-Eintrag R1; **AKZ.5** gegen das Tag von E4; `git status` (R2); `--full`; RAM des Release-Builds; alles sofort ins Messprotokoll | Lead | R1.9 | **AKZ.5**, **AKZ.8**, **AKZ.9** | ☐ | — |
| R1.11 | `TESTPLAN-PWA.md`: Felder `tcp_now_*` und `app_*` mit Lesart; Heap-Zeile mit `tcp=`/`nrt=`; `304` als Normalfall beim warmen Laden; Ladeprobe und Browser-Sitzung als Instrumente; Ruhewert-Grenze für T3 nach Ent-12. `BEFUNDE.md`: Träger „Paket 2026-10-15 (Release R1)“ an C65, C70, C72. **Nicht gleichzeitig mit einer Messzeile des Leads** (R3) | `doc-writer` | R1.10 | S9, S10, S11 | ☐ | — |
| R1.12 | **Einspielzeilen:** ESP-OTA zuerst, danach Smoketest, `install-app.sh --check`, mit Ent-3 `install-app.sh` und erneut `--check`, dann `check-pwa.sh`; **Rückweg** mit Befehl (`design.md` §10: ESP 3.2.37 aus `wordclock-release-2026-10-14-180012.zip`) — benannt, **bevor** eingespielt wird | Lead | R1.10 | **AKZ.6** | — | — |
| R1.13 | Build, ZIP, Rollout, **Commit + Tag + Push** (DIR-011) | Lead | R1.11, R1.12 | **AKZ.6** | ☐ | — |
| R1.14 | **Einspielen:** Wache läuft; Du spielst nach R1.12 ein; danach prüft der Lead Smoketest samt Update-Quelle und neuen Feldern, `install-app.sh --check`, `check-pwa.sh`; Versionen und Zeiten sofort ins Messprotokoll | Lead (Einspielen: Nutzer) | R1.13 | **AKM.1** | — | — |

---

## Schritt G9 — Messung und Tor

Der Lead kündigt Beginn und Ende an; die Uhr gehört der Messung. Jede Phase wird **ausgewertet, bevor** die
nächste beginnt.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G9.1 | **C70 — Ladeprobe nach R1:** drei Läufe, einer in Englisch, `--erwarte-304`; Tor nach Ent-7; je Lauf sofort ins Messprotokoll; Dauer der `- app etag`-Zeilen gegen AKC.4 | Lead | R1.14 | **AKM.2**, **AKC.4** (Dauer) | — | — |
| G9.2 | **C70 — Probe U** nach `design.md` §5.2 — nur mit **Ent-15** | Lead | G9.1, **Ent-15** | **AKM.3** | — | — |
| G9.3 | **I-5 — Browser-Sitzung Safari**, Phasen S0–S4, Bedienung nach Ent-6 (bei C danach einmal durch Dich auf Deinem Gerät); `caffeinate`, falls geplant | Lead (Bedienung nach Ent-6) | G9.2 (bzw. G9.1) | **AKM.4** | — | — |
| G9.4 | **I-5 — Browser-Sitzung Chrome**, mindestens 300 s Ruhe nach G9.3 | Lead | G9.3 | **AKM.5** | — | — |
| G9.5 | **Auswertung** nach der Entscheidungsregel (`design.md` §5.4): Zuordnung zu M1–M5 mit den tragenden Werten ins Messprotokoll; Gesundheit über das Fenster, `watch-log.sh --pruefe` | Lead | G9.4 | **AKM.6** (Teil 1), **AKM.7** | — | — |
| G9.6 | **C72 nachführen:** Befund mit Ergebnis M1–M5 und Verweis aufs Messprotokoll. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | G9.5 | S10, S11 | ☐ | — |
| G9.7 | **Tor:** Der Lead legt **Ent-8**, **Ent-10**, **Ent-14** einzeln vor, mit dem Ergebnis aus G9.5 und der Empfehlung aus `design.md` §5.4; Du entscheidest. Ergibt das Tor keinen Eingriff: R2 und G10 entfallen, weiter mit T3 | Lead (Entscheid: Nutzer) | G9.6 | **AKM.6** (Teil 2) | — | — |
| G9.8 | **C70 — weiter oder anhalten:** Bleibt AKM.2 warm unter 1'072, legt der Lead Dir Variante (c) als nächsten Schritt vor (Ent-1, `design.md` §4.3) — kein stilles Nachschieben | Lead (Entscheid: Nutzer) | G9.1 | **AKM.2** | — | — |

---

## Schritt R2 — Release: Eingriff C72 (nur nach dem Tor)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R2.0 | **Nachtrag**, nur wenn Ent-8 etwas wählt, das `design.md` §6 nicht festlegt, oder die Frist aus Ent-14 eine andere Messgrösse braucht | `spec-writer` | G9.7 | Nachtrag freigegeben | — | — |
| R2.1 | **Eingriff** nach Ent-8 und `design.md` §6 (K1 / K2 / K5); bei K2 Gang neu beginnen nach jedem Abbau (P3), PCBs mit `ClientContext` nie anfassen, Zähler `tcp_reaped`. **Prüfstand c72e** | `esp-developer` | **G9.7** (R2.0) | **AKE.1** am Prüfstand | ☐ | — |
| R2.2 | Bauen, „Globale Variablen“ sofort ins Messprotokoll; c72e ablegen, in S16; Gegenprobe gegen den Stand R1; Sabotagen; Fallzahlen | Lead | R2.1 | **AKE.1**, **AKZ.8**, **AKZ.9** | ☐ | — |
| R2.3 | **Review R2**, ausdrücklich: nie ein PCB mit `ClientContext`; keine Abgabe während des Gangs; ausgehende Abrufe (Wetter, Update) unberührt; RST nur an Verbindungen ohne Anfrage (K1) | `code-reviewer` | R2.2 | Checkliste | — | ☐ |
| R2.4 | ESP-Version anheben | `release-engineer` | R2.3 | S4 | ☐ | — |
| R2.5 | `CHANGELOG.md`-Eintrag R2; **AKZ.5** gegen das Tag von R1; `git status`; `--full`; RAM | Lead | R2.4 | **AKZ.5**, **AKZ.8**, **AKZ.9** | ☐ | — |
| R2.6 | `TESTPLAN-PWA.md`: Eingriff und `tcp_reaped`; `BEFUNDE.md`: Träger „Release R2“ an C72. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | R2.5 | S9, S10, S11 | ☐ | — |
| R2.7 | Einspielzeilen samt Rückweg (Stand R1 aus dessen ZIP); Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | R2.6 | **AKZ.6** | ☐ | — |
| R2.8 | **Einspielen:** Wache läuft; Du spielst ein; danach prüft der Lead Smoketest, `install-app.sh --check`, `check-pwa.sh` | Lead (Einspielen: Nutzer) | R2.7 | **AKZ.6** | — | — |

## Schritt G10 — Wirkung des Eingriffs

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G10.1 | **Browser-Sitzung Safari und Chrome** wie G9.3/G9.4 auf dem Stand R2; je Phase sofort ins Messprotokoll | Lead (Bedienung nach Ent-6) | R2.8 | **AKE.2** | — | — |
| G10.2 | **Regression:** drei `check-pwa.sh`, Ladeprobe warm, Smoketest; `ERR_CONNECTION_RESET` und Δ`write_lost_failed` auswerten | Lead | G10.1 | **AKE.3** | — | — |

---

## Schritt T3 — `pwa-tester` auf dem Endstand

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T3 | `pwa-tester` Phasen 0–4 und 9; Haltegrenze `low=` < 1'072, Ruhewert nach **Ent-12**; bei jedem Halt I-4- und I-5-Felder ohne Parameter in den Bericht, bevor zurückgesetzt wird; die Uhr exklusiv, Beginn und Ende angekündigt | `pwa-tester` | **G10.2**, oder **G9.7** bei „kein Eingriff“; R1.11 (bzw. R2.6) | **AKZ.7** | — | — |

## Schritt Z — Doku

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: Schliessungen **nur mit Beleg** aus dem Messprotokoll und T3 — C70 (AKM.2, AKM.3), C72 (AKE.2 oder Ergebnis M4/M5 als Grund), Beitrag zu C65 | `doc-writer` | T3 | S10, S11 | ☐ | — |
| Z.2 | Spec-Endstand: jedes Kriterium mit Beleg oder Grund | `spec-writer` | Z.1 | jedes AK belegt oder begründet | — | — |

---

## Abschluss

- [ ] Alle Tasks erledigt; R2, G10 und R1.5 entfallen nur mit dem Entscheid, der sie ausschliesst
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release, Ergebnis im Messprotokoll
- [ ] Alle 29 Akzeptanzkriterien aus `requirements.md` erfüllt oder mit Grund offen
- [ ] Ein oder zwei Releases (R1; R2 nach dem Tor), je Flash-Umfang und Reihenfolge benannt, je committet,
      getaggt, gepusht
