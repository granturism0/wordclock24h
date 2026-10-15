# Tasks — Paket 2026-10-15 „ESP-Heap unter PWA-Last, zweiter Hebel“ (C70, C71, C72)

**Erstellt:** 2026-10-15, Stand `72cfda7`; am selben Tag mit den Entscheiden des Nutzers, den Lead-Entscheiden und dem
Prüfserver-Befund zu `HTTP/1.0` nachgeführt. Momentaufnahme (DIR-006); der lebende Stand steht in `BEFUNDE.md`.
**Status: freigegeben am 15.10.2026 bis aufs Tor** — offen Ent-8, Ent-10, Ent-14, Ent-17 (Tor, G10.6) und Ent-16
(Platz von C71, vor R3). Anforderungen: `requirements.md`; Design: `design.md`.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft `./tools/guardrails.sh`; erst bei Exit 0
geht die Schreibberechtigung weiter. Wo „Nutzer“ steht, ist eine Handlung oder ein Entscheid von Dir gemeint, kein
Schreibzugriff.

**Die Spalte „Hängt ab von“ ist verbindlich, auch wo die Dateien disjunkt sind** (R3b):

1. **Drei Releases, jedes mit eigener Messung (Ent-9):** R1 ⇒ G9 (ohne Cache) ⇒ R2 ⇒ G10 (mit Cache) ⇒ Tor ⇒ R3 ⇒
   G11. Code für R2 darf entstehen, während G9 läuft; **gebaut und eingespielt** wird R2 erst nach G9.7. Code für R3
   entsteht erst nach dem Tor (G10.6).
2. **Analyse vor Code:** R1.1 erst nach A.1 (P1–P8) und, falls A.1 eine Annahme widerlegt, nach A.2. P9–P11 (C71)
   dürfen bis zum Tor nachgeliefert werden; ohne sie fällt Ent-17 nicht.
3. **Ein Schreiber je Datei, nacheinander:** Alle ESP-Tasks eines Releases schreibt **ein** `esp-developer`
   nacheinander; nach jedem misst der Lead „Globale Variablen“ (AKZ.8), bevor der nächste beginnt. Der PWA-Teil R2.3
   berührt disjunkte Dateien ohne Gerätenachweis dazwischen — er **darf parallel** zu R2.1 laufen; R2.4 wartet auf
   beide.
4. **Eine Messung zur Zeit:** Während G9, G10 und G11 läuft kein OTA, kein Upload ausser Probe U, kein zweiter
   Browser (R5, `TESTPLAN-PWA.md` Abschnitt 0 Punkt 5). **Keine Haltegrenze** (Ent-7): Neustarts und Exceptions sind
   Ergebnisse — sofort ins Messprotokoll, dann Update-Quelle prüfen (DIR-009), dann weiter.
5. **Firmware zuerst** (DIR-002): in R2 erst ESP-OTA, dann `install-app.sh`.
6. **C71 nach Ent-16:** Mit **A** (Empfehlung) wie unten in R3. Mit **B** gehen R3.3, R3.4 und die C71-Teile von R3.5,
   R3.6, G11.2 in ein eigenes R4 nach G11 (gleiche Abfolge wie R3); die Spalte „Task“ nennt den Teil.

**Messwerte sofort (L424):** Jeder messende Task hängt sein Ergebnis **vor dem nächsten Task** an das
Messprotokoll in `BEFUNDE.md` an. Messzeilen schreibt der Lead, Befundnachführungen der `doc-writer` — nie
gleichzeitig (R3).

**Einspielen:** Du spielst jedes Release selbst ein; der Lead liefert die fertigen Zeilen samt Reihenfolge und Rückweg
(AKZ.5). Jeder Gerätelauf mit `./tools/watch-log.sh`, vorher gestartet, `--pruefe` am Ende (DIR-013); auf später
geplante Läufe mit `caffeinate` (DIR-020). Schreibende Schritte laufen ohne Einzelfreigabe (`CLAUDE.md:416`, Ent-15
entfällt).

**Werkzeugvorbehalt:** `firmware-analyst` und `code-reviewer` haben kein `Bash`. Prüfstände baut der Umsetzer im
Scratchpad, der Lead legt sie unter `tools/checks/auszug/` ab und fährt die Gegenproben (DIR-014). Vor jedem Patch
Kodierung **und** Zeilenende feststellen (AKZ.3) — `ESP-uclock.ino` gemischt, `http.cpp` und `stm32flash.cpp` UTF-8.

**Spalten:** **G** = Guardrails nach dem Task mit Exit 0, **R** = Review bestanden.

---

## Schritt 0 — Grundlagen

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| 0.1 | **Messprotokoll anlegen, Ausgangswerte ohne Last:** Zeile „Messprotokoll Paket 2026-10-15“ in `BEFUNDE.md` mit den Entscheiden und dem Prüfserver-Befund zu `HTTP/1.0`/`HTTP/1.1` (22:21–22:23); Grössen der 13 App-Dateien (`/api/fs_list`); „Globale Variablen“/Programmgrösse Build E4; Kodierung und Zeilenende von `http.cpp`, `stm32flash.cpp`, `ESP-uclock.ino`, `base.h`, `app.js`; `netstat`, `lsof`, `osascript` vorhanden | Lead | Freigabe | **AKZ.1** | ☐ | — |
| 0.2 | **Werkzeug `ladeprobe`** (liegt vor, liest den Leitungsstatus) nach `design.md` §5.1 vervollständigen: `If-None-Match` je Anfrage, `--erwarte-304`; Gegenproben Vorschau ⇒ FEHL, Prüfserver `HTTP/1.1` ⇒ OK, Prüfserver `HTTP/1.0` ⇒ FEHL; Meldung in Konsole und Mitschrift | Lead | Freigabe | **AKZ.2 (i)** | ☐ | — |
| 0.3 | **Werkzeug `browser-sitzung`** nach `design.md` §5.3 (Safari und Chrome per `open`/`osascript`, Lesungen je Phase, Ablauf für die Sitzung auf dem Gerät des Nutzers); Gegenprobe Uhr nicht erreichbar ⇒ Exit ≠ 0. macOS-Erlaubnis zur Automation einmal durch Dich | Lead | Freigabe | **AKZ.2 (ii)** | ☐ | — |
| 0.4 | **Review der Werkzeuge** 0.2 und 0.3: Kommt jede Meldung an? Zählt die Ladeprobe den Leitungsstatus und sieht sie jede `/app/`-Anfrage (Fallzahl gegen die Vorschau)? | `code-reviewer` | 0.2, 0.3 | DIR-014, L235 | — | ☐ |

## Schritt A — Analyse (lesend)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| A.1 | **Prüfpunkte P1–P11** nach `design.md` §12 am Core 3.1.2 und am Code `72cfda7`; je Punkt Antwort mit Fundstelle oder ausdrücklich „nicht belegbar, weil …“. **P1–P8 vor R1.1**, **P9–P11 (C71)** parallel, spätestens vor G10.6; der Lead trägt jedes Ergebnis ins Messprotokoll | `firmware-analyst` | Freigabe | Bericht P1–P11 | — | — |
| A.2 | **Nachtrag zur Spec**, nur falls A.1 eine Annahme widerlegt (P1: FIN_WAIT_2 läuft nie ab; P2: PCB-Zahl unbegrenzt; P6: mehr Schreibstellen; P10/P11: Kandidat L1/L2 nicht machbar) — als Nachtrag in diesem Ordner, Dir vorgelegt | `spec-writer` | A.1 | Nachtrag freigegeben | — | — |

---

## Schritt R1 — Release: Instrumente I-5 und I-6

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R1.1 | **I-5:** Felder `tcp_now_*` in `device_ready` nach `design.md` §3.1, Listen vollständig in den Stackpuffer vor dem ersten `http_send()`; Zählfunktion in `http.cpp`, Prototyp in `base.h`; Heap-Zeile mit `tcp=`/`nrt=` (§3.2; `ESP-uclock.ino` byte-genau). **Prüfstand c72i** im Scratchpad | `esp-developer` | 0.1, **A.1 (P1–P8)**, A.2 | **AKI.1**, **AKI.2** am Prüfstand | ☐ | — |
| R1.2 | ESP-Teil bauen, „Globale Variablen“ und Programmgrösse sofort ins Messprotokoll | Lead | R1.1 | **AKZ.8** | — | — |
| R1.3 | **I-6 (C71):** Zeile `- update cache heap …` je Durchgang und `- heap erste anfrage …` einmal je Start nach `design.md` §3.3; bestehende `- update cache`-Zeilen unverändert. **Prüfstand c71i** | `esp-developer` | R1.2 | **AKI.3** am Prüfstand | ☐ | — |
| R1.4 | ESP-Teil bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R1.3 | **AKZ.8** | — | — |
| R1.5 | Prüfstände c72i, c71i ablegen, in S16; Gegenproben gegen ESP 3.2.37; Sabotagen; Fallzahlen (L235); c65-Prüfstände grün | Lead | R1.4 | **AKI.1–AKI.4** am Prüfstand, **AKZ.7** | ☐ | — |
| R1.6 | **Werkzeuge:** `smoke-device.sh` mit den I-5-Feldern (Gegenprobe `SMOKE_DR_OVERRIDE`); `watch-log.sh` Heap-Zeile mit Zusatz, `- update cache heap` als Hinweis (Gegenprobe `WATCHLOG_QUELLE`, `low=752 tcp=…` ⇒ Alarm kommt an) | Lead | R1.5 | **AKZ.2 (iii), (iv)** | ☐ | — |
| R1.7 | **Review R1:** kein `yield()` im Listengang; I-6 misst nach `LittleFS.end()` und bei Abbruch; keine Änderung an den bestehenden Cache-Zeilen; Kodierung und Zeilenenden (AKZ.3) | `code-reviewer` | R1.6 | Checkliste | — | ☐ |
| R1.8 | ESP-Version anheben | `release-engineer` | R1.7 | S4 | ☐ | — |
| R1.9 | `CHANGELOG.md`-Eintrag R1; **AKZ.4** gegen das Tag von E4; `git status` (R2); `--full`; RAM des Release-Builds; sofort ins Messprotokoll | Lead | R1.8 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R1.10 | `TESTPLAN-PWA.md`: Felder `tcp_now_*` mit Lesart; Heap-Zeile mit `tcp=`/`nrt=`; Zeilen `- update cache heap`, `- heap erste anfrage`; Ladeprobe und Browser-Sitzung als Instrumente; K30 für T3 nach Ent-12. `BEFUNDE.md`: Träger „Paket 2026-10-15 (Release R1)“ an C65, C70, C71, C72. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | R1.9 | S9, S10, S11 | ☐ | — |
| R1.11 | **Einspielzeilen:** ESP-OTA, Smoketest, `install-app.sh --check`, `check-pwa.sh`; **Rückweg** mit Befehl (ESP 3.2.37 aus `wordclock-release-2026-10-14-180012.zip`) — benannt, **bevor** eingespielt wird | Lead | R1.9 | **AKZ.5** | — | — |
| R1.12 | Build, ZIP, Rollout, **Commit + Tag + Push** (DIR-011) | Lead | R1.10, R1.11 | **AKZ.5** | ☐ | — |
| R1.13 | **Einspielen:** Wache läuft; Du spielst ein; danach prüft der Lead Smoketest samt Update-Quelle und neuen Feldern, `install-app.sh --check`, `check-pwa.sh`; Versionen und Zeiten sofort ins Messprotokoll | Lead (Einspielen: Nutzer) | R1.12 | **AKM.1** | — | — |

---

## Schritt G9 — Ausgangsstand ohne Cache (auf R1)

Der Lead kündigt Beginn und Ende an; jede Phase wird **ausgewertet, bevor** die nächste beginnt.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G9.1 | **Ladeprobe ohne Cache:** drei Läufe (je 1 kalt, 5 warm), einer in Englisch, ohne Haltegrenze; je Lauf sofort ins Messprotokoll | Lead | R1.13 | **AKM.2** | — | — |
| G9.2 | **Browser-Sitzung Safari** (Lead per `osascript`), S0–S4 | Lead | G9.1 | **AKM.3** (Teil Lead) | — | — |
| G9.3 | **Browser-Sitzung Safari auf Deinem Gerät** aus L461, S0–S4 auf Zuruf; der Lead liest die ESP-Seite | Lead (Bedienung: Nutzer) | G9.2 | **AKM.3** (Teil Nutzer) | — | — |
| G9.4 | **Browser-Sitzung Chrome** (Lead), mindestens 300 s Ruhe nach G9.3 | Lead | G9.3 | **AKM.4** | — | — |
| G9.5 | **Startmessung C71:** drei ESP-Starts ohne Browser nach `design.md` §5.5, einer mit erzwungenem Schreiben (`/upd-wc-list.txt` vorher entfernt); keine HTTP-Lesung in den ersten 5 min; danach Smoketest samt Update-Quelle; Zuordnung N1–N4 | Lead | G9.4 | **AKM.5** | — | — |
| G9.6 | **Gesundheit G9** über das ganze Fenster, `watch-log.sh --pruefe`; jeder Neustart als Ergebnis | Lead | G9.5 | **AKM.6** | — | — |
| G9.7 | **C71, C72 nachführen:** Messbild aus G9 (vorläufige Zuordnung M, Zuordnung N) als Befundzeilen. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | G9.6 | S10, S11 | ☐ | — |

---

## Schritt R2 — Release: C70 Asset-Cache (ESP und PWA)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R2.1 | **C70, ESP** nach `design.md` §4.4: `If-None-Match` lesen, `ETag`-Tabelle, CRC bei Bedarf in derselben geöffneten Datei, Statuszeile `HTTP/1.1` **nur** für die `200` und die `304` auf Weisslisten-Assets (`Connection: close`, exaktes `Content-Length` in der `200`, die `304` ohne Körper und ohne `Content-Length`), `304` mit Grössenprüfung und `app_complete`, `app_etag_invalidate()` an **jeder** schreibenden LittleFS-Stelle (Bestand nach P6), Zähler `app_*`, Zeile `- app etag`. Im Bericht: Liste aller Schreibstellen mit Datei:Zeile; Liste aller Statuszeilen `HTTP/1.1` (Soll: zwei). **Prüfstand c70e** | `esp-developer` | **R1.13**, A.1 (P5, P6) | **AKC.1–AKC.5**, **AKC.7** am Prüfstand | ☐ | — |
| R2.2 | ESP-Teil bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R2.1 | **AKZ.8** | — | — |
| R2.3 | **C70, PWA:** `layout-previews.json` mit `cache: "no-cache"` (`app.js:11655`), sonst nichts. **Prüfstand c70p** | `pwa-developer` | R1.13 | **AKC.6** | ☐ | — |
| R2.4 | Prüfstände c70e, c70p ablegen, in S16; Gegenproben gegen den Stand R1 bzw. PWA 1.4.100; Sabotagen; Fallzahlen; alles ins Messprotokoll | Lead | R2.2, R2.3 | **AKC.1–AKC.7** am Prüfstand, **AKZ.7** | ☐ | — |
| R2.5 | **Werkzeuge:** `check-pwa` INFO „`/app/` 304 je Laden“ aus `responseReceivedExtraInfo.statusCode` und „Byte je Laden“; `smoke-device.sh` mit den `app_*`-Feldern; `watch-log.sh` `- app etag` als Hinweis; je Gegenprobe | Lead | R2.4 | **AKZ.2 (iii), (v)** | ☐ | — |
| R2.6 | **Statuszeile in den Werkzeugen:** `tools/**` nach Prüfungen auf `HTTP/1.0` bei App-Assets durchsuchen und nachziehen; Ergebnis (Fundstellen oder „keine“) ins Messprotokoll | Lead | R2.5 | `design.md` §9 | ☐ | — |
| R2.7 | **Review R2:** jede Schreibstelle invalidiert (gegen R2.1 und P6); `304` nie ohne Grössenprüfung; `HTTP/1.1` an genau zwei Stellen, beide mit `Connection: close`; `Content-Length` exakt; Legacy, `/api/*`, Hinweisseiten weiter `HTTP/1.0`; `If-None-Match`-Puffer je Anfrage geleert und begrenzt; Attrappen-Semantik in c70e; Kodierung und Zeilenenden | `code-reviewer` | R2.6 | Checkliste | — | ☐ |
| R2.8 | ESP-Version, `APP_VERSION` und `CACHE_NAME` anheben | `release-engineer` | R2.7 | S4 zeigt beide | ☐ | — |
| R2.9 | `CHANGELOG.md`-Eintrag R2 (beide Komponenten); **AKZ.4** gegen das Tag von R1; `git status` (R2); `--full`; RAM; `app.js.gz` vorher/nachher; sofort ins Messprotokoll | Lead | R2.8 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R2.10 | `TESTPLAN-PWA.md`: `app_*`-Felder, `304` als Normalfall beim warmen Laden, Statuszeile `HTTP/1.1` für App-Assets, Zeile `- app etag`, Probe U. `BEFUNDE.md`: Träger „Release R2“ an C70. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | R2.9 | S9, S10, S11 | ☐ | — |
| R2.11 | **Einspielzeilen:** **erst** ESP-OTA, Smoketest, `install-app.sh --check`, **dann** `install-app.sh`, erneut `--check`, `check-pwa.sh`; Rückweg mit Befehl (ESP auf den Stand R1 aus dessen ZIP, PWA bleibt) — benannt, **bevor** eingespielt wird | Lead | R2.9 | **AKZ.5** | — | — |
| R2.12 | Build, ZIP, Rollout, **ein** Commit + Tag + Push | Lead | R2.10, R2.11, **G9.7** | **AKZ.5** | ☐ | — |
| R2.13 | **Einspielen** nach R2.11; danach prüft der Lead Smoketest, `--check`, `check-pwa.sh`, beide Versionen; sofort ins Messprotokoll | Lead (Einspielen: Nutzer) | R2.12 | **AKW.1** | — | — |

---

## Schritt G10 — mit Cache, und Tor (auf R2)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G10.1 | **Ladeprobe mit Cache:** drei Läufe wie G9.1, `--erwarte-304`; Gegenüberstellung mit G9.1; Dauer der `- app etag`-Zeilen gegen AKC.4 | Lead | R2.13 | **AKW.2**, **AKC.4** (Dauer) | — | — |
| G10.2 | **Probe U** nach `design.md` §5.2 | Lead | G10.1 | **AKW.3** | — | — |
| G10.3 | **Browser-Sitzungen mit Cache:** Safari (Lead) und Chrome, vor S0 je einmal öffnen und beenden; `app_full`/`app_not_modified` je Phase — für Safari der erste Nachweis, ob er bei `HTTP/1.1` mit `ETag` nachfragt; Gegenüberstellung mit G9 | Lead | G10.2 | **AKW.4** | — | — |
| G10.4 | **Auswertung:** C72 nach `design.md` §5.4 einem von M1–M5 zuordnen (auf G10, G9 daneben); C71-Zuordnung aus G9.5 mit P9–P11; Gesundheit G10, `watch-log.sh --pruefe`; sofort ins Messprotokoll | Lead | G10.3, A.1 (P9–P11) | **AKW.5** (Teil 1), **AKW.6** | — | — |
| G10.5 | **C71, C72 nachführen** mit den Zuordnungen. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | G10.4 | S10, S11 | ☐ | — |
| G10.6 | **Tor:** Der Lead legt **Ent-8**, **Ent-10**, **Ent-14**, **Ent-17** einzeln vor, dazu **Ent-16**, falls noch offen — mit den Zuordnungen und den Empfehlungen aus `design.md` §5.4, §5.5; Du entscheidest. Ergibt das Tor weder für C72 noch für C71 einen Eingriff: R3 und G11 entfallen, weiter mit T3 | Lead (Entscheid: Nutzer) | G10.5 | **AKW.5** (Teil 2) | — | — |
| G10.7 | **C70 — weiter oder anhalten:** Bleibt AKW.2 warm unter 1'072, oder fragt Safari nicht nach (AKW.4), legt der Lead Dir den nächsten Schritt vor — Variante (c) bzw. `Last-Modified` am Prüfserver gegenprüfen (`design.md` §14); kein stilles Nachschieben | Lead (Entscheid: Nutzer) | G10.1, G10.3 | **AKW.2**, **AKW.4** | — | — |

---

## Schritt R3 — Release: Eingriff C72 und Massnahme C71 (nach dem Tor)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| R3.0 | **Nachtrag**, nur wenn Ent-8 oder Ent-17 etwas wählt, das `design.md` §6/§7 nicht festlegt, oder Ent-17 L3 mit eigener RAM-Grenze wählt | `spec-writer` | G10.6 | Nachtrag freigegeben | — | — |
| R3.1 | **C72 — Eingriff** nach Ent-8 und `design.md` §6 (K1 / K2 / K5); bei K2 Gang neu beginnen nach jedem Abbau, PCBs mit `ClientContext` nie, Zähler `tcp_reaped`. **Prüfstand c72e**. Entfällt ohne Eingriff | `esp-developer` | **G10.6** (R3.0) | **AKE.1** am Prüfstand | ☐ | — |
| R3.2 | Bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R3.1 | **AKZ.8** | — | — |
| R3.3 | **C71 — Massnahme** nach Ent-17 und `design.md` §7 (L1 / L2 / L4 / L5); bestehende Sicherungen des Zwischenspeichers bleiben (Temporärdatei, Längenprüfung, E3.8 N4); c65n angepasst, falls L4 den E4.5a-Vergleich umstellt. **Prüfstand c71e**. Entfällt bei N4 oder ohne Massnahme | `esp-developer` | R3.2 (bzw. G10.6) | **AKS.1** am Prüfstand | ☐ | — |
| R3.4 | Bauen, „Globale Variablen“ sofort ins Messprotokoll | Lead | R3.3 | **AKZ.8** | — | — |
| R3.5 | c72e, c71e ablegen, in S16; Gegenproben gegen den Stand R2; Sabotagen; Fallzahlen; c65m, c65n grün | Lead | R3.4 | **AKE.1**, **AKS.1**, **AKZ.7** | ☐ | — |
| R3.6 | **Review R3:** nie ein PCB mit `ClientContext`; keine Abgabe während des Gangs; ausgehende Abrufe unberührt; RST nur an Verbindungen ohne Anfrage (K1); C71: Reihenfolge der Freigaben, nie zwei `File` gleichzeitig (L4), Verhalten beim Start (L1/L2), Sicherungen des Zwischenspeichers unverändert | `code-reviewer` | R3.5 | Checkliste | — | ☐ |
| R3.7 | ESP-Version anheben | `release-engineer` | R3.6 | S4 | ☐ | — |
| R3.8 | `CHANGELOG.md`-Eintrag R3; **AKZ.4** gegen das Tag von R2; `git status`; `--full`; RAM | Lead | R3.7 | **AKZ.4**, **AKZ.7**, **AKZ.8** | ☐ | — |
| R3.9 | `TESTPLAN-PWA.md`: Eingriff, `tcp_reaped`, geändertes Startverhalten (C71). `BEFUNDE.md`: Träger „Release R3“ an C71, C72. **Nicht gleichzeitig mit einer Messzeile** (R3) | `doc-writer` | R3.8 | S9, S10, S11 | ☐ | — |
| R3.10 | Einspielzeilen samt Rückweg (Stand R2 aus dessen ZIP; nimmt C71 und C72 zurück); Build, ZIP, Rollout, **Commit + Tag + Push** | Lead | R3.9 | **AKZ.5** | ☐ | — |
| R3.11 | **Einspielen:** Wache läuft; Du spielst ein; danach prüft der Lead Smoketest, `install-app.sh --check`, `check-pwa.sh` | Lead (Einspielen: Nutzer) | R3.10 | **AKZ.5** | — | — |

## Schritt G11 — Wirkung (auf R3)

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| G11.1 | **C72 — Browser-Sitzungen** Safari und Chrome wie G10.3; je Phase sofort ins Messprotokoll | Lead | R3.11 | **AKE.2** | — | — |
| G11.2 | **C71 — drei ESP-Starts** wie G9.5, danach Ruhewert 150 s nach `check-pwa.sh` und nach einer Ladeprobe; Zwischenspeicher-Zeilen und `*.tmp` prüfen | Lead | G11.1 | **AKS.2**, **AKS.3** | — | — |
| G11.3 | **Regression:** drei `check-pwa.sh`, Ladeprobe warm, Smoketest; `ERR_CONNECTION_RESET` und Δ`write_lost_failed` | Lead | G11.2 | **AKE.3** | — | — |

---

## Schritt T3 — `pwa-tester` auf dem Endstand

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| T3 | `pwa-tester` Phasen 0–4 und 9; Haltegrenze **K30** (Ent-12): Ruhewert `max` < 5'120 oder `low=` < 1'072 ⇒ anhalten; bei jedem Halt I-4- und I-5-Felder ohne Parameter in den Bericht, bevor zurückgesetzt wird; Uhr exklusiv, Beginn und Ende angekündigt | `pwa-tester` | **G11.3**, oder **G10.6** ohne R3; R1.10, R2.10 (R3.9) | **AKZ.6** | — | — |

## Schritt Z — Doku

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| Z.1 | `BEFUNDE.md`: Schliessungen **nur mit Beleg** aus Messprotokoll und T3 — C70 (AKW.2–AKW.4), C71 (AKS.2 oder N4 als Grund), C72 (AKE.2 oder M4/M5 als Grund), Beitrag zu C65 | `doc-writer` | T3 | S10, S11 | ☐ | — |
| Z.2 | Spec-Endstand: jedes Kriterium mit Beleg oder Grund | `spec-writer` | Z.1 | jedes AK belegt oder begründet | — | — |

---

## Abschluss

- [ ] Alle Tasks erledigt; R3, G11 und einzelne R3-Tasks entfallen nur mit dem Entscheid am Tor, der sie ausschliesst
- [ ] `./tools/guardrails.sh --full` mit Exit 0 vor jedem Release, Ergebnis im Messprotokoll
- [ ] Alle 37 Akzeptanzkriterien aus `requirements.md` erfüllt oder mit Grund offen
- [ ] Drei Releases (R1, R2, R3 nach dem Tor; R4 nur mit Ent-16 B), je Flash-Umfang und Reihenfolge benannt, je
      committet, getaggt, gepusht und eingespielt
