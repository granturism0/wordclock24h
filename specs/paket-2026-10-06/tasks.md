# Tasks — Paket 2026-10-06 (Runden P, P2, S, F, verteilte Runde K)

**Erstellt:** 2026-10-06, Stand `401aae2`. Nachgeführt am selben Tag nach der Auslieferung
von Runde P und nach dem Gerätetest von 1.4.91 (L324). Momentaufnahme (DIR-006).

**Rundenfolge:** **P (ausgeliefert) → P2 → S → F**. Runde K hat **keinen eigenen Flash**;
ihre Punkte fahren mit P2, S und F mit (`design.md` §6) und sind dort mit **[K]** markiert.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wenn die Dateien disjunkt sind** (R3b).
Der Besitz-Hook prüft, **wer** schreibt, nicht **wann**. Die wichtigste Abhängigkeit dieses
Pakets ist baulich, nicht inhaltlich: **Jeder Release-Build nimmt den ganzen Arbeitsbaum
mit** (`design.md` §4.1). Deshalb beginnt S erst nach dem P2-Build und F erst nach dem
S-Build — auch wo die Inhalte nichts miteinander zu tun haben.

**`Nutzer` in der Agentenspalte heisst:** Der Lead liefert die Zeile fertig zum Einfügen,
der Nutzer führt sie aus. Er spielt **jedes** Update selbst ein und legt M2 mit seinem
Passwort an.

**Werkzeugvorbehalt:** `firmware-analyst`, `code-reviewer` und `ui-reviewer` haben kein
`Bash` (L238, L269). Git-Historie, Bauen und Gerätemessung gehören zum Lead.
**Prüfstände unter `tools/checks/` und Proben unter `tools/ui-mess/proben/` legt der Lead
ab** — `tools/**` ist sein Revier.

**Kennungen.** „N1", „N2" sind **Befunde aus der Nachzählung** vom 06.10.2026. **L321 / B40**
ist der Befund aus dem Gerätetest von 1.4.91 (network_scan überschreibt Eingabe), **L323 /
C38** die stille Kürzung von SSID und WLAN-Schlüssel, **L324** die Abnahme von 1.4.91.
**L303** betrifft `runConfirmedButtonAction`, **L306** das Umschalten von Flags. Die
Entscheidungen heissen **Ent-1 bis Ent-7** (`requirements.md`).

---

## Runde P — ausgeliefert

**Ausgeliefert als PWA 1.4.91**, Tag `release/3.2.21-3.2.24-1.4.91`. **Die Abnahme steht in
L324**; der lesende Gerätetest hat B40/L321 geliefert und die Beobachtung zu Massnahme 4
(P2.1d).

| Enthalten | Befund |
|---|---|
| B34, B35 (ohne Overlay-Text), B36, B33 | L284, L287, L288, L246 |
| R1–R4 aus dem E2-Review | Befundzeilen aus P.0 |
| B27 mit dem Wortlaut „Sekunden am Ambilight-Ring weich ausblenden" (Ent-3) | L223 |
| B26 — tote Funktion ohne Aufrufer entfernt, mit Gegenprobe; `files-panel` und `local-update-panel` **bleiben**, weil `tools/ui-mess` sie als Panelnamen führt | — |
| Umschalt- und Power-Knöpfe | L304 |
| feste Texte in `toggleFlagButton` | B37 / L305 |
| Busy-Text nach Fehlern | B38 / L306 |

**Ebenfalls erledigt:** **B32** — die Gegenprobe B1f/B1h ist gefahren.

**Nicht enthalten, deshalb in P2:** der Overlay-Text in `TEXT_FIELD_LIMITS`, B40/L321,
Massnahme 4 in engerer Form, B17 — und der **Nachweis für L303 und L306**, der am Gerät
nicht folgenlos herstellbar ist (P2.1e). **B35 gilt für Zeitserver und Zeitzone erst als
nachgewiesen, wenn B40 behoben ist** — vorher kann ein später Scan den geprüften Wert durch
den alten ersetzen, und „Speichern" schickt den alten.

**Was von P noch aussteht**, soweit nicht schon geschehen — der Lead vermerkt es:

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P.17 | Lesende Abnahme am Gerät — **gelaufen**, Ergebnis in L324 | Lead | — | `check-pwa.sh` ohne Fehler, Dateimodul geöffnet (AKP.6); Smoketest; `watch-log.sh` mitgelesen | — | ☐ |
| P.18 | **Testdurchlauf Phasen 0–4 und 9**, mit M2 des Nutzers | `pwa-tester` | P.17, M2 | **AKZ.4.** Unvollständig ⇒ P nicht abgenommen | — | — |
| P.19 | `BEFUNDE.md` nachführen: alles aus der Tabelle oben, B32, B17 bleibt offen, B35 für Zeitserver/Zeitzone mit Vorbehalt bis B40 | `doc-writer` | P.18 | S10 läuft durch; jedes „erledigt" mit Datei und Funktion dieses Commits | ☐ | — |

---

## Runde P2 — Rest aus P, mit dem nächsten PWA-Upload

**Warum vor S:** P2 schreibt `app.js`, und jeder Release-Build von S nimmt `app-gz` mit. Ist
P2 vor S.1 gebaut, liegt beim S-Build kein offener PWA-Task im Baum (§4.1).

**Testdurchlauf — neu geprüft, weil P2 jetzt echte Logik enthält.** Mit B40 und Massnahme 4
ändert P2 nicht mehr nur eine Tabellenzeile, sondern das Verhalten bei einer nebenläufigen
Antwort und bei der Erkennung ungespeicherter Änderungen. **Die Entscheidung bleibt: kein
eigener `pwa-tester`-Durchlauf.** Begründung:

- B40 ist eine **Wettlaufbedingung**, und die ist am Gerät nicht gezielt herstellbar — ob
  der Scan vor oder nach der Eingabe zurückkommt, hängt vom WLAN ab. **Herstellbar ist sie
  in der Vorschau**, mit einer absichtlich verzögerten `network_scan`-Antwort. Deshalb ist
  die Probe (P2.1c) das tragende Instrument, nicht der Gerätelauf.
- Die Probe liegt dauerhaft unter `tools/ui-mess/proben/` und muss **einmal gegen 1.4.91
  fehlschlagen** (DIR-014) — sonst misst sie nicht, was sie messen soll.
- Massnahme 4 ist in der Vorschau vollständig prüfbar: Eingabe, Ausgangswert wiederherstellen,
  Modulwechsel.
- **L303 und L306** brauchen eine Fehlerantwort, die am Gerät nicht folgenlos entsteht: Die
  drei Aktionen hinter `runConfirmedButtonAction` (`downloadUpdateAssets`, `resetStm32`,
  `formatLittleFsFromFiles`) lassen sich nicht gefahrlos scheitern lassen, und ein
  Flag-Umschalten scheitert nicht über einen zu langen Text. **Die Vorschau bildet die
  Fehlerantwort nach** (P2.1e).
- **Am Gerät zusätzlich, lesend und ohne Speichern** (P2.8). Das braucht kein M2, weil
  nichts geschrieben wird.
- Der volle Durchlauf S.26 läuft gegen einen Stand, der P2 enthält, und prüft **B35 am
  Zeitserver in Phase 2** sowie den Overlay-Text in Phase 3.

**Was dabei offen bleibt, ausdrücklich:** Zwischen dem P2-Upload und S.26 ist B40 am Gerät
nur durch die lesende Probe aus P2.8 belegt, nicht durch einen Durchlauf mit Speichern. Der
Ausfall wäre der heutige Fehler, kein neuer. **L303 und L306 werden nie am Gerät mit einem
echten Fehlschlag belegt** — der Nachweis ist die Probe.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P2.1 | **[K] Overlay-Text in `TEXT_FIELD_LIMITS`** (32 Byte, `OVERLAY_MAX_TEXT_LEN`, `vars.h:414`), angewandt im Overlay-Editor **und** in `importOverlaySettings()` | `pwa-developer` | — | **AKP.1** und **AKP.10** für den Overlay-Text: 17 Umlaute (34 Byte) ⇒ keine Anfrage, Meldung sichtbar; 16 Umlaute ⇒ Anfrage geht hinaus; präparierte Sicherung mit 33-Byte-Overlay-Text ⇒ übersprungen und genannt. **Das Gerät kürzt heute still** — die PWA-Prüfung ist bis zum F-OTA der einzige Schutz | ☐ | ☐ |
| P2.1b | **B40 / L321:** `updateNetworkControlsFromMeta()` setzt `network-timeserver-input.value` und `network-timezone-input.value` ohne Bedingung; ausgelöst nach jeder `network_scan`-Antwort über `refreshNetworkUi()`. Beide Felder über **dieselbe** `prefillDeviceValue()`-Regel wie die AP-SSID (L34) — keine zweite Regel daneben | `pwa-developer` | P2.1 | **AKP.14:** Ein **fokussiertes** oder **vom Nutzer geändertes** Zeitserver- oder Zeitzonenfeld wird von einer späten Scan-Antwort nicht mehr überschrieben; ein unberührtes Feld übernimmt den Gerätewert weiterhin. Bericht nennt, ob weitere Felder in `updateNetworkControlsFromMeta()` dieselbe Form haben — **gemeldet, nicht mitkorrigiert** | ☐ | ☐ |
| P2.1c | **Probe B40** unter `tools/ui-mess/proben/`: Vorschau mit verzögerter `network_scan`-Antwort; Eingabe in Zeitserver und Zeitzone, dann Antwort ausliefern, Felder ablesen. Drei Fälle: fokussiert, geändert und verlassen, unberührt | Lead | P2.1b | **Einmal fehlgeschlagen gegen 1.4.91** (DIR-014), bestanden gegen P2.1b; die Probe meldet die Zahl der geprüften Fälle (drei je Feld, sechs gesamt) | ☐ | — |
| P2.1d | **Massnahme 4 in engerer Form:** `handleDirtyFormInteraction()` setzt `hasUnsavedEdits = true` bei **jeder** Eingabe, ohne mit dem Ausgangswert zu vergleichen. Am Gerät beobachtet (L324): tippen, Ausgangswert vollständig wiederherstellen, beim Modulwechsel erscheint trotzdem der Dialog. Künftig gilt ein Feld nur als geändert, wenn sein Wert vom Ausgangswert abweicht | `pwa-developer` | P2.1b | **AKP.15** in der Vorschau: tippen und Ausgangswert wiederherstellen ⇒ **kein** Dialog beim Modulwechsel; echte Änderung ⇒ Dialog **weiterhin**. Der Ausgangswert ist **derselbe**, den `prefillDeviceValue()` aus P2.1b kennt — zwei Fassungen von „was ist der Gerätewert" wären das Muster aus L289 | ☐ | ☐ |
| P2.1e | **Probe L303 und L306** unter `tools/ui-mess/proben/`, mit **nachgebildeter Fehlerantwort**. **L303** (`runConfirmedButtonAction`): Eine Formatier-Antwort mit Fehler (`formatLittleFsFromFiles`) meldet **nicht** „formatiert". **L306** (`toggleFlagButton`/`finishButtonFeedback`): Nach einem fehlgeschlagenen Umschalten steht der Knopf wieder auf seinem Zustand, **nicht** auf „schaltet…" | Lead | — | **AKP.16.** Jede der beiden Proben ist **einmal fehlgeschlagen**, gegen einen **künstlich zurückgebauten** Stand (DIR-014) — 1.4.91 enthält die Korrekturen bereits, ein Lauf dagegen bewiese nichts. Der zurückgebaute Stand liegt im Scratchpad, nicht im Arbeitsbaum (L268). Fallzahl gemeldet | ☐ | — |
| P2.2 | **B17, Eingrenzung am Gerät:** Seite nach einem ESP-Neustart laden, `system`, Logfenster ohne Reload beobachten. Rein lesend | Lead | — | Bericht: welche der drei Spuren aus L142 trägt, oder keine. `watch-log.sh` mitgelesen | — | — |
| P2.3 | **B17, Umsetzung — nur wenn P2.2 eine Ursache belegt, die in einer Funktion behebbar ist** | `pwa-developer` | P2.1d, P2.2 | **AKP.13.** Sonst entfällt der Task **ausdrücklich**, mit Vermerk | ☐ | ☐ |
| P2.4 | Review | `code-reviewer` | P2.3, P2.1c, P2.1e | `TEXT_FIELD_LIMITS` gegen `vars.h` (jetzt **zehn** Werte, Koordinaten doppelt gezählt); **B40 nutzt `prefillDeviceValue()`, Massnahme 4 denselben Ausgangswert** — keine zweite Fassung; keine neuen leeren `catch` | — | ☐ |
| P2.5 | `APP_VERSION` **und** `CACHE_NAME` anheben | `release-engineer` | P2.4 | S4 zeigt den neuen Stand | ☐ | — |
| P2.6 | `git status` (kein offener Produktcode-Task), Build inkl. `app-gz`, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | P2.5 | R2; `guardrails.sh --full` Exit 0 | ☐ | — |
| P2.7 | **Nutzer:** `./tools/install-app.sh --check`, `./tools/install-app.sh` | **Nutzer** | P2.6 | Seite meldet die neue App-Version | — | — |
| P2.8 | Abnahme | Lead | P2.7 | `check-pwa.sh` ohne Fehler; **lesend, ohne Speichern:** Netzwerk-Modul öffnen, in Zeitserver und Zeitzone tippen, den nächsten Scan abwarten, Felder unverändert; dann Ausgangswert wiederherstellen und Modul wechseln, kein Dialog; `watch-log.sh` mitgelesen | — | ☐ |
| P2.9 | `BEFUNDE.md`: Overlay-Text PWA-Seite, B40/L321, Massnahme 4, **L303 und L306 (Nachweis über P2.1e)**, B35 für Zeitserver/Zeitzone (Vorbehalt bis S.26), B17 (erledigt oder mit Eingrenzung offen) | `doc-writer` | P2.8 | S10 läuft durch | ☐ | — |

**Einspielreihenfolge:** nur PWA. Keine Firmware vorausgesetzt.

---

## Runde S — Brücke

**Warum ESP zuerst:** Die neue Wirkung entsteht mit dem STM; dazwischen läuft der neue ESP
gegen den alten STM — der Rückfallnachweis AKS.6 (S.11).

**Warum S.1 erst nach P2.6:** gemeinsamer Arbeitsbaum, `release-zip` baut alle drei
Komponenten (`design.md` §4.1).

**Der STM-Teil ist zweigeteilt:** erst der **Kern** (S.14–S.17), dann ein **Testbau**
(S.18), dann die **K-Punkte einzeln, je mit Testbau danach** (S.19a–S.19h). Grund: Der
Prüfer der Nachzählung verlangt für die STM-K-Punkte **Messung statt Schätzung**, und nur
so ist jeder Punkt einzeln zurückstellbar (`design.md` §2.7). **Die seriellen Testbauten
hat der Lead freigegeben (Ent-7).**

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S.1 | ESP: Eröffnungszeile und **Abschlussmarke** auswerten; `var_sync_check()` verlangt die Marke **nur**, wenn die Eröffnungszeile kam | `esp-developer` | P2.6 | **AKS.3**, Rückfall ohne Eröffnungszeile im Quelltext ausgewiesen | ☐ | ☐ |
| S.2 | ESP: Zuordnungszeile `ACK <xy>` **vor** jedem `.` und `!v`. **Festlegen und im Bericht begründen**, woher die zwei Zeichen kommen und was bei einer unmarkierten Zeile gesendet wird (`design.md` §2.4) | `esp-developer` | S.1 | **AKS.4.** Der Punkt bleibt ein nackter Punkt | ☐ | ☐ |
| S.3 | ESP: Zähler „unmarkiert nach erster Marke" **in den Logring**; Markenpflicht für die **drei aufgezählten** Kommandoarten mit `!v` | `esp-developer` | S.2 | **AKS.7, AKS.8** (Quelltextteil). Kein Eingriff in `http.cpp` für C26 (`design.md` §4.2) | ☐ | ☐ |
| S.4 | ESP: **C31** — `eepromdata.cpp` gibt „gesetzt"/„leer" statt der Schlüssel aus | `esp-developer` | S.3 | **AKS.10** (Quelltextteil). Kodierung und Zeilenende der Datei vorher festgestellt und im Bericht genannt | ☐ | ☐ |
| S.5 | **[K] N1:** `http_overlays()` (Legacy) prüft `oidx < MAX_OVERLAYS && oidx <= n_overlays`, **bevor** irgendetwas geschrieben oder `n_overlays` erhöht wird. **Sicherheitsausnahme** von der Trägerregel (`design.md` §6.2) | `esp-developer` | S.4 | **AKS.14.** Prüfstand im Scratchpad mit den Typen des Ziels (L256), an S.20 übergeben | ☐ | ☐ |
| S.6 | Review ESP-Seite | `code-reviewer` | S.5 | Vier Punkte; besonders: Kann die Markenpflicht einen Dauerzustand erzeugen? Endet die Markenerwartung mit **ihrem** Abgleich? Bilden ESP und STM-Entwurf dieselben zwei Zuordnungszeichen? N1: Ist auch der `disp`-Zweig (`overlay_idx = atoi (…)`, `http.cpp:4684`) gesichert **oder** als eigener Befund gemeldet? **Die Antwort steht am Code** | — | ☐ |
| S.7 | **Analyse, rein lesend:** (1) **AKS.11** — kann eine unbekannte ESP-Zeile über die STM-Logausgabe in den ESP-Logring gelangen? (2) **N1, STM-Seite** — fängt der STM einen Overlay-Index ≥ 32 ab, und kann `n_overlays` über `overlay_set_n_overlays` (A3) über 32 gesetzt werden? (3) Welche Grenze muss A3+A48 nehmen, damit sie zu `overlay.c` passt („S2 im Katalog" laut Nachzählung, ● nicht nachgesehen) | `firmware-analyst` | — | Antworten mit Fundstellen. (1) bei ja: neuer Befund über S.27. (2) und (3) gehen als Vorgabe an S.19c | — | — |
| S.8 | ESP-Version anheben | `release-engineer` | S.6 | S4 zeigt den neuen Stand | ☐ | — |
| S.9 | Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S.8 | `git status` ohne offenen Produktcode-Task; `guardrails.sh --full` Exit 0 | ☐ | — |
| S.10 | **Nutzer-Sitzung S1:** ESP einspielen | **Nutzer** | S.9 | neue ESP-Version in `/api/update_status` | — | — |
| S.11 | **Zwischenabnahme: neuer ESP gegen alten STM** | Lead | S.10 | **AKS.6:** ESP-Neustart ⇒ Vollabgleich ohne Timeout und ohne Reset, `var_send_timeout_cnt` steigt nicht, Diagnosefolge lückenlos. **AKS.10:** beide Schlüsselzeilen im Mitschnitt und in `/api/stm32_log` tragen nur „gesetzt"/„leer". **AKS.14** lesend: Legacy-Overlayseite lädt. Dazu `install-app.sh --check`, Smoketest samt Update-Quelle (DIR-009). `watch-log.sh` mitgelesen. **Schlägt AKS.6 fehl, geht S zurück und wird nicht am STM fortgesetzt** | — | ☐ |
| S.12 | **Flash-Gate, Stufe 1 — Vorabschätzung** des F103-Bedarfs: Kern (S.14–S.17) **und** jeder Punkt S.19a–S.19h, je Unter- und Obergrenze, Herleitung aus gemessenen Zuwächsen (L165, L289, L296) und neuen unbedingten Zeichenketten (L168) | `firmware-analyst` | — | **AKS.0** Stufe 1. Bericht mit Summe der Obergrenzen gegen 1'924 Byte minus Reserve aus Ent-2. **Entscheidet nichts** — passt schon der Kern nicht, geht der Bericht über den Lead an den Nutzer | — | — |
| S.13 | **Nur wenn S.12 schon für den Kern nicht passt:** Entscheidung, was entfällt oder wo gespart wird | **Nutzer** | S.12 | Entscheidung im Gespräch; der Lead vermerkt entfallene Tasks hier **ausdrücklich** | — | — |
| S.14 | STM: Eröffnungszeile und Abschlussmarke senden — **als `var <freierBuchstabe>…` über `var_send_buf()`** | `stm-developer` | S.11, S.12, (S.13) | **Vier Auflagen aus `design.md` §2.2, einzeln im Bericht belegt:** (1) Buchstabe ausserhalb `N n S T D A C M O t a l I`; (2) Nutzlast endet nicht auf `*` + vier Hexziffern; (3) Nutzlast unter 72 Zeichen; (4) **kein eigenes Top-Level-Präfix**. Die Eröffnungszeile gilt nur für **ihren** Abgleich | ☐ | ☐ |
| S.15 | STM: **A39**, Variante (b) — `IPADDRESS`-Zweig merkt vor, Ticker **nach** dem Abgleich; `pending_weather_ticker_restore` wandert mit | `stm-developer` | S.14 | **AKS.1.** Bericht weist nach, dass keine der vier Teilbedingungen des Restores vereinfacht ist | ☐ | ☐ |
| S.16 | STM: Zuordnungszeile lesen, dem folgenden `.`/`!v` zuordnen, unpassende Quittung **verwerfen und zählen**; Prüfstand im Scratchpad, Typen angeglichen (L256) | `stm-developer` | S.15 | **AKS.5:** verspätete Quittung ⇒ **eine** Nachsendung; gemerkte Zuordnung ohne Quittung wird verbraucht. Prüfstand an S.20 | ☐ | ☐ |
| S.17 | STM: **A16** — `watchdog_reload()` in Tetris und Snake (vom Nutzer bestätigt, Ent-6) | `stm-developer` | S.16 | Quelltext: in beiden Spielschleifen | ☐ | ☐ |
| S.18 | **Flash-Gate, Stufe 2 — Testbau nach dem Kern:** nur Ziel F103, kein Release-ZIP, kein Rollout | Lead | S.17 | Gemessener Rest notiert. **Liegt er schon unter der Reserve:** Halt, Bericht an den Nutzer | — | — |
| S.19a | **[K] A20** — Geheimnisse aus der Timeout-Meldung | `stm-developer` | S.18 | Abnahmesatz A20 (K-Tabelle); danach Testbau (Lead), Zuwachs in die K-Tabelle | ☐ | ☐ |
| S.19b | **[K] A13 + A47** — `strncpy` ohne Abschlussbyte in `esp8266.c`, zusammen, rund 8 Zeilen | `stm-developer` | S.19a + Testbau | Abnahmesatz A13+A47, **AKS.12**; danach Testbau | ☐ | ☐ |
| S.19c | **[K] A3 + A48** — Bereichsprüfung `overlay_set_n_overlays` und Indexguard in `schedule_esp8266_overlay()`, Grenze nach S.7 (3) | `stm-developer` | S.19b + Testbau, S.7 | Abnahmesatz A3+A48 — **über den Prüfstand (AKK.6), nicht am Gerät**; danach Testbau | ☐ | ☐ |
| S.19d | **[K] A24** — Wortindizes in `tables_fill_words()` | `stm-developer` | S.19c + Testbau | Abnahmesatz A24 — **über den Prüfstand**; danach Testbau | ☐ | ☐ |
| S.19e | **[K] A45** — Typbereinigung in der Snake-Animation, 2 Zeilen | `stm-developer` | S.19d + Testbau | Abnahmesatz A45; danach Testbau | ☐ | ☐ |
| S.19f | **[K] A14** — Variablenindex in `var_send_string()` maskieren | `stm-developer` | S.19e + Testbau | Abnahmesatz A14; danach Testbau | ☐ | ☐ |
| S.19g | **A46** — drei Verwerfungspfade an den gemeinsamen Zähler | `stm-developer` | S.19f + Testbau | **AKS.13**; Prüffälle an S.20; danach Testbau | ☐ | ☐ |
| S.19h | **[K] A49** — Abweisungsmeldung je Grund (1-Byte-Maske) | `stm-developer` | S.19g + Testbau | Abnahmesatz A49; danach Testbau | ☐ | ☐ |
| S.20 | **Prüfstände ablegen und Guardrails nachziehen:** AKS.5 und AKS.14 unter `tools/checks/`; A46-Fälle in `htoi-laenge.c` (S13); **S15 (`idx-guard.sh`) auf A3+A48 und A24 erweitern**; **S7 nachführen** (zwei neue `watchdog_reload()` aus A16) | Lead | S.19h | **AKK.6:** S15 meldet für die neuen Guards nicht mehr „nicht abgedeckt", Fallzahl gemeldet. Jeder Prüfstand **einmal fehlgeschlagen** gegen die alte Fassung (DIR-014). S7 zeigt den neuen Bestand | ☐ | — |
| S.21 | Review STM-Seite | `firmware-analyst` | S.20 | Vier Punkte; **die vier Auflagen aus S.14 einzeln**; die drei neuen Zustände auf **jedem** Pfad aufgelöst; Restore-Bedingung vollständig; **jeder K-Punkt im Brückenpfad einzeln markiert** (`design.md` §6.2); jeder zurückgestellte Punkt ist **ganz** zurückgenommen | — | ☐ |
| S.22 | STM-Version anheben | `release-engineer` | S.21 | S4 zeigt den neuen Stand | ☐ | — |
| S.23 | Build, **Gegenprobe des Gates**, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S.22 | Gemessene F103-Grösse aus S8b neben die Schätzung S.12 und die Testbauten gestellt. **Rest unter der Reserve: kein Rollout**, Bericht an den Nutzer. Schätzung über ihrer Obergrenze ⇒ Befund an S.27. Release-Notiz nennt jeden enthaltenen und jeden zurückgestellten K-Punkt | ☐ | — |
| S.24 | **Nutzer-Sitzung S2:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`, **dann 60 s Tetris oder Snake**, **dann M2** | **Nutzer** | S.23 | Erwartete STM-Version; **AKS.9:** kein Watchdog-Reset im Mitschnitt — fällt der Spiellauf aus, gilt AKS.9 als **nicht erfüllt**; M2 liegt vor | — | — |
| S.25 | Abnahme am Gerät | Lead | S.24 | **AKS.2, AKS.3, AKS.5, AKS.7, AKS.8.** Hauptloop-Zähler gegen L226 während eines ESP-Neustarts; **IP-Lauftext vollständig** (Auge); Erfolgszeile erst nach der Marke; Zähler im Logring; null `!v`-Abweisungen im gesunden Zustand; `diff-snapshot.sh --soll`; **ein Blick auf die Uhr** (A24, A45 sind Anzeigepfade); `watch-log.sh` mitgelesen. **Nicht verlangt:** ein Nachweis der STM-Indexguards am Gerät (AKK.6) | — | ☐ |
| S.26 | **Testdurchlauf Phasen 0–4 und 9** — gegen einen Stand, der P2 enthält | `pwa-tester` | S.25 | **AKZ.4**, M2 aus S.24. Bericht nennt die enthaltenen K-Punkte. **Zusätzlich verlangt:** (1) **B35 am Zeitserver** in Phase 2 — der Nachweis, der seit B40 aussteht. (2) **B36** über einen Schreibaufruf, der **fehlschlägt, aber nichts verändert**: ein zu langer Update-Host, den der ESP mit `error=2` abweist — die Fehlermeldung bleibt stehen und wird **nicht** von einer Erfolgsmeldung überschrieben, und **Phase 9 belegt**, dass der Gerätewert unverändert ist. (3) **Phase 3** prüft den Overlay-Text mit 33 Byte in der PWA: vor dem Absenden abgewiesen. **Nicht hier:** L303 und L306 — sie lassen sich am Gerät nicht folgenlos zum Scheitern bringen, ihr Nachweis ist P2.1e | — | — |
| S.27 | `BEFUNDE.md` nachführen: A39, A42/L260, A35 Teil 1, C26 (ToDo-Text auf den Entscheidungsstand), A16, C31, A46, N1, alle K-Punkte mit Träger S (erledigt **oder** „zurückgestellt: Flash, gemessen +n Byte"), Gate-Ergebnis, Ergebnis S.7, B35 für Zeitserver/Zeitzone, **B36 (Gerätenachweis aus S.26)** | `doc-writer` | S.26 | S10 läuft durch; jedes „erledigt" mit Datei:Zeile dieses Commits | ☐ | — |

**Einspielreihenfolge: erst ESP (S.10), dann STM (S.24).** Dazwischen die Zwischenabnahme
S.11.

**Ein zurückgestellter K-Punkt wird ganz zurückgenommen**, nicht halb: Der `stm-developer`
setzt seine Änderung zurück, der Lead misst erneut, und der Punkt steht in der K-Tabelle
als „zurückgestellt: Flash, gemessen +n Byte". **Wann zurückgestellt und wann angehalten
wird, regelt Ent-2** — der Lead wendet die Regel an, er erfindet keine.

---

## Runde F — Flash-Überwachung

**Warum F.1 erst nach S.23:** F-Code im Baum während des S-STM-Builds landete im
ESP-Fabrikat jenes Release-ZIPs (`design.md` §4.2). **F.4 darf jederzeit laufen** — es ist
Werkzeug, nicht Fabrikat.

**Alle `esp-developer`-Tasks in F schreiben `http.cpp`** (bzw. F.5d `httpclient.cpp`) und
laufen deshalb **seriell** in der Reihenfolge der Tabelle.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| F.1 | Legacy-Flashzweig **und** Legacy-Auswahlliste über `http_remote_stm32_filename_matches()`; Hinweis mit Rückweg `./tools/flash-stm.sh` bei leerer Liste | `esp-developer` | S.23 | **AKF.1, AKF.2, AKF.3.** `grep` findet die Funktion im Flashzweig, in der Liste und im API-Endpunkt; `fname + 6` ohne Längenprüfung ist weg. **Nicht scharf fahren** | ☐ | ☐ |
| F.2 | **Nur bei Ent-4 = ja:** eigene Kennung für „remove failed" in `fs_remove` | `esp-developer` | F.1, Ent-4 | Quelltext: Kennung 6 nur noch für „nicht gefunden" | ☐ | ☐ |
| F.3 | **Nur bei Ent-5 = ja:** API-Auswahlliste über dieselbe Funktion | `esp-developer` | F.2, Ent-5 | Quelltext | ☐ | ☐ |
| F.4 | `tools/smoke-device.sh`: `HARDWARE_CONFIGURATION` = 65535 als eigene Stufe | Lead | — | **AKF.4**, Gegenprobe gegen festen Testwert, einmal fehlgeschlagen | ☐ | ☐ |
| F.5a | **[K] C27 + N2** — `fs_show` prüft die Dateigrösse | `esp-developer` | F.3 | Abnahmesatz C27+N2 | ☐ | ☐ |
| F.5b | **[K] E12** — `tft_flags_set` und `dfplayer_bell_flags_set` über `http_get_on_off_value` | `esp-developer` | F.5a | Abnahmesatz E12 | ☐ | ☐ |
| F.5c | **[K] C25** — Verlustzähler trennen | `esp-developer` | F.5b | Abnahmesatz C25 | ☐ | ☐ |
| F.5d | **[K] C9k** — `httpclient_read_header()` wartet über den Helfer aus L152 | `esp-developer` | F.5c | Abnahmesatz C9k | ☐ | ☐ |
| F.5e | **[K] Overlay-Text** — `http_api_overlay_set` weist zu lange Texte mit `http_check_strvar_len()` ab statt `utf8_copy_truncated` | `esp-developer` | F.5d | Abnahmesatz Overlay-Text | ☐ | ☐ |
| F.5g | **C38 / L323** — SSID und WLAN-Schlüssel **abweisen statt still kürzen**: `http_api_network_client_set()` (`http.cpp:8664-8670`) und `http_api_eeprom_settings_set()` (`:8779-8785`), über `http_check_strvar_len()` oder dieselbe Byte-Regel — keine zweite | `esp-developer` | F.5e | **AKF.6.** **Abnahme nur am Quelltext und am Prüfstand** — `network_client_set` bleibt für Gerätetests **gesperrt**, ein schreibender WLAN-Aufruf ist **nicht** Teil der Abnahme | ☐ | ☐ |
| F.5f | **Werkzeugteil zu C25:** was die Verlustzeile liest (`watch-log.sh` u. a.), auf das neue Format | Lead | F.5c | Die Logwache meldet beide Zähler; Gegenprobe mit einer eingespielten Zeile im neuen Format (DIR-014) | ☐ | — |
| F.5h | **Prüfstand C38** ablegen (vom `esp-developer` im Scratchpad gebaut): zu lange SSID bzw. zu langer Schlüssel ⇒ Abweisung, Grenzwert ⇒ angenommen | Lead | F.5g | Einmal fehlgeschlagen gegen die alte Fassung (DIR-014), Fallzahl gemeldet | ☐ | — |
| F.6 | Review | `code-reviewer` | F.5g, F.5h | Vier Punkte; besonders: **Bleibt der Rückweg offen?** **E12:** Sendet die PWA beide Setter **immer** mit Parameter? Sonst bricht E12 sie. **C27:** Braucht die PWA eine Anpassung? **C38:** Kann die PWA eine SSID oder einen Schlüssel mit **mehr Byte** senden, als der ESP annimmt (Umlaute in der SSID, `maxlength` in Zeichen)? **Wenn ja**, wird eine PWA-Grenze ein eigener Task **nach** dem F-OTA (L241). **Overlay-Text:** Fängt die PWA seit P2.1 jeden Weg ab? Bei Ent-5 = ja: Wie stellt die PWA eine leere Liste dar? | — | ☐ |
| F.7 | ESP-Version anheben | `release-engineer` | F.6 | S4 zeigt den neuen Stand | ☐ | — |
| F.8 | Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | F.7, F.5f | `guardrails.sh --full` Exit 0 | ☐ | — |
| F.9 | **Nutzer-Sitzung F1:** ESP einspielen | **Nutzer** | F.8 | neue ESP-Version in `/api/update_status` | — | — |
| F.10 | Abnahme am Gerät | Lead | F.9, F.4 | Smoketest **mit der neuen Stufe**, grün; Legacy-Seite **lesend**: Liste zeigt nur passende Dateien; `fs_show` auf eine vorhandene Datei liefert sie unverändert (C27-Gegenprobe); Release Notes laden (C9k-Gegenprobe); WLAN unverändert verbunden (C38 hat keinen schreibenden Gerätetest); `install-app.sh --check`; Update-Quelle (DIR-009); `watch-log.sh` mitgelesen, beide Verlustzähler sichtbar | — | ☐ |
| F.11 | **Nutzer-Sitzung F2:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` (derselbe STM-Stand wie nach S), **dann M2** | **Nutzer** | F.10 | **AKF.5:** erwartete STM-Version; M2 liegt vor. F1 und F2 dürfen eine Sitzung sein, wenn F.10 dazwischen grün ist | — | — |
| F.12 | **Testdurchlauf Phasen 0–4 und 9** | `pwa-tester` | F.11 | **AKZ.4.** Bericht nennt die enthaltenen K-Punkte. **Kein** schreibender Aufruf auf `network_client_set` | — | — |
| F.13 | Abschlussbilanz in `BEFUNDE.md`: C9c6, L179, **L272 (eigene Zeile, durch F.1 geschlossen)**, C38/L323, ggf. Ent-4/Ent-5, K-Punkte mit Träger F und W | `doc-writer` | F.12 | S10 läuft durch; jeder im Paket berührte Befund trägt Status und Beleg | ☐ | ☐ |

**Einspielreihenfolge:** nur ESP (F.9), danach der STM-Flash als Probe (F.11). **Braucht C38
eine PWA-Grenze (F.6), kommt sie danach** — Firmware zuerst (L241).

---

## Runde K — Kleinkram, verteilt auf die Flashes der anderen Runden

Inhalt aus der Nachzählung vom 06.10.2026 (drei Prüfer, rund 80 offene Punkte, Einstufung
K/M/G) und aus dem Testdurchlauf desselben Tages (Overlay-Text). **K heisst:** höchstens
rund zehn Zeilen und keine Entwurfsfrage. Gerüst und Regeln in `design.md` §6.

### Rahmen-Tasks

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| K.0 | **Abzählung:** K-Punkte aus dem Bericht der Prüfer gegen die Tabellen unten | Lead | — | **AKK.2.** Zahl im Bericht = Zeilen unten (eingeplant, erledigt, wartend; Mehrfachkennungen wie „A13 + A47" einzeln gezählt). **Offen:** Der Prüfer nennt 13 STM-K-Punkte, die Liste enthält 10 Kennungen — abgleichen. Abweichung ⇒ zurück an den `spec-writer` | — | — |
| K.W | **[K] Werkzeug:** E20, E11, E14-Rest, E24-Teil DIR-002, B20 | Lead | — | je Zeile ihr Abnahmesatz. **E11 und F.4 schreiben dieselbe Datei** (`smoke-device.sh`) — nacheinander | ☐ | — |

### K-Tabelle — eingeplant oder erledigt

**Träger:** **P2** = nächster PWA-Upload; **S-ESP** = ESP-OTA von S; **S-STM** = STM-Flash
von S, **gemessen** (Ent-2); **F** = ESP-OTA von F; **W** = Werkzeug ohne Flash.
**Messwert** wird beim Testbau eingetragen (nur S-STM).

| Kennung | Was | Datei | Träger | Task | Agent | Abnahme: erledigt, wenn … | Messwert |
|---|---|---|---|---|---|---|---|
| **N1** ✔ | Legacy-Overlayindex aus `atoi`, nur gegen `0xff` geprüft; Schreibzugriff über 32 Plätze, aus dem LAN, auch per `<img>` | `http.cpp`, `http_overlays()` | **S-ESP** (Sicherheitsausnahme) | S.5 | `esp-developer` | … `http_overlays()` vor dem ersten Schreibzugriff auf `overlays[]` **und** vor `n_overlays++` `oidx < MAX_OVERLAYS && oidx <= n_overlays` prüft und sonst nichts schreibt; der Prüfstand mit Zieltypen weist 32, 33 und 300 ab (AKS.14) | — |
| **A20** | Geheimnisse in der Timeout-Meldung | `src/vars/vars.c` | S-STM | S.19a | `stm-developer` | … die Timeout-Meldung Kennung, Index und Länge nennt und **keinen** Wert; `grep -a` auf die Formatzeile zeigt kein Wert-`%s` mehr | |
| **A13 + A47** | `strncpy` ohne Abschlussbyte, darunter `u.filedata` | `src/esp8266/esp8266.c` | S-STM | S.19b | `stm-developer` | … jede Stelle aus L96 und L292 terminiert ist **oder** mit Begründung „Verbraucher braucht es nicht" im Bericht steht (L96: je Stelle den Verbraucher prüfen) | |
| **A3 + A48** | `overlay_set_n_overlays` ohne Bereichsprüfung; `schedule_esp8266_overlay()` ohne Indexguard | `src/main.c`, `src/overlay/` | S-STM | S.19c | `stm-developer` | … `n_overlays` nicht über die Grösse von `overlay[]` gesetzt werden kann **und** ein Overlay-Index ausserhalb über `esp8266_idx_ok()` gezählt abgewiesen wird; Grenze aus `sizeof`, passend zu S.7 (3); **nachgewiesen in S15, nicht am Gerät** (AKK.6) | |
| **A24** | Wortindizes in `tables_fill_words()` | `src/tables/tables.c` | S-STM | S.19d | `stm-developer` | … `tables_fill_words()` jeden Wortindex über `tables_idx_ok()` prüft; Prüfstand: Index 255 abgewiesen, gültige Tabelle byte-gleich | |
| **A45** | Typfehler in der Snake-Animation | `src/display/display.c` | S-STM | S.19e | `stm-developer` | … `cc -fsyntax-only -Wall -Wextra` über `display.c` keine Warnung zu `animation_snake_search_next_word` mehr meldet | |
| **A14** | Variablenindex in `var_send_string()` nicht maskiert | `src/vars/vars.c` | S-STM | S.19f | `stm-developer` | … ein Prüfstand für Index 0..255 nie mehr als zwei Hexzeichen erzeugt | |
| **A49** | Abweisungsmeldung je Grund | `src/main.c` | S-STM | S.19h | `stm-developer` | … im Prüfstand nach vier Längenabweisungen eine folgende Indexabweisung **trotzdem** protokolliert wird und die Drosselung für die Summe bleibt | |
| **A16** | Tetris/Snake ohne `watchdog_reload()` — vom Nutzer mit „Drin lassen" bestätigt | `src/tetris/` | S-STM (Kern) | S.17, S7 in S.20 | `stm-developer`, Lead | … AKS.9 erfüllt ist **und** S7 die zwei neuen Stellen im Bestand führt | (im Kern) |
| **C27 + N2** | `fs_show` prüft die Dateigrösse nicht (N2 ● Inhalt nur aus der Meldung des Leads bekannt) | `http.cpp`, `http_api_fs_show()` | F | F.5a | `esp-developer` | … eine Datei mit 0 Byte **nicht** mehr als `200 OK` mit leerem Rumpf ausgeliefert, sondern als eigener benannter Zustand gemeldet wird; N2 nach seiner Befundzeile | — |
| **E12** | Zwei Flag-Setter löschen bei fehlendem Parameter | `http.cpp` | F | F.5b | `esp-developer` | … `tft_flags_set` und `dfplayer_bell_flags_set` einen fehlenden Parameter mit Kennung 1 abweisen, statt zu löschen, und F.6 bestätigt, dass die PWA beide immer mit Parameter sendet | — |
| **C25** | Verlustzähler bucht Browser-Abbau und Schreibversagen zusammen | `http.cpp`, `http_flush()` | F | F.5c, F.5f | `esp-developer`, Lead | … `http_flush()` zwei Zähler führt (`written == 0` und `! connected ()`), beide in der Verlustzeile im Logring stehen **und** `watch-log.sh` beide liest | — |
| **C9k** | `httpclient_read_header()` wartet gar nicht | `httpclient.cpp` | F | F.5d | `esp-developer` | … `httpclient_read_header()` über den Zeitgrenzen-Helfer aus L152 liest; am Gerät laden die Release Notes weiter (F.10) | — |
| **Overlay-Text, ESP** | `http_api_overlay_set` kürzt still, statt abzuweisen (Testdurchlauf 06.10.2026: 33 → 32 Byte, `{"ok":true}`) | `http.cpp` | F | F.5e | `esp-developer` | … ein Overlay-Text über 32 Byte mit `{"ok":false,"error":2,…}` abgewiesen wird und der gespeicherte Text unverändert bleibt | — |
| **Overlay-Text, PWA** | fehlt in `TEXT_FIELD_LIMITS` | `app.js` | **P2** | P2.1 | `pwa-developer` | … die Vorschauproben aus P2.1 bestehen und `TEXT_FIELD_LIMITS` zehn Werte führt | — |
| **B26** | `formatLittleFs()` und zwei Kennungen | `app.js` | **P — erledigt** | — | — | **Erledigt mit PWA 1.4.91**, mit Gegenprobe: tote Funktion ohne Aufrufer, entfernt. `files-panel` und `local-update-panel` **bleiben**, weil `tools/ui-mess` sie als Panelnamen führt | — |
| **E20** | Besitz-Hook auf Schreibpositionen | `tools/hooks/file-ownership.py` | W | K.W | Lead | … zwei Gegenproben bestehen: fremder Pfad nur im Meldungstext ⇒ läuft durch; Schreiben in eine fremde Datei ⇒ abgewiesen (DIR-014) | — |
| **E11** | Smoketest: Versionen gegen den Repo-Sollstand, leerer Logring als eigener Zustand | `tools/smoke-device.sh` | W | K.W | Lead | … der Smoketest eine Abweichung der gemeldeten Versionen vom Repo meldet und einen leeren Logring als „leer" statt als fehlendes Instrument; beide Fälle einmal mit festem Testwert ausgelöst | — |
| **E14-Rest** | Override-Option in `vermessen.mjs` für das DFPlayer-Modul | `tools/ui-mess/` | W | K.W | Lead | … ein Vorschaulauf mit der Option die drei `.chip-toggle` des DFPlayer-Moduls misst | — |
| **E24, Teil DIR-002** | Kurzregel „kompletter Build und Release-ZIP …" nennt ihre Kennung nicht | `CLAUDE.md` | W | K.W | Lead | … die Kurzregel in `CLAUDE.md` „DIR-002" nennt und die Direktiven-Stufe der Guardrails durchläuft | — |
| **B20** | Kein `git add -A`, solange Agenten laufen | Lead-Verfahren | W | K.W | Lead | … die Regel am Commit-Schritt des Skills `/release` steht, mit Verweis auf L158 | — |

Schon in den Kernrunden geführt und deshalb **nicht doppelt** gezählt: **C9c6, C28**
(F.1), **B33** (ausgeliefert mit P). **B40/L321**, **Massnahme 4**, **L303/L306** und
**C38/L323** sind keine K-Punkte aus der Nachzählung, sondern Befunde aus Gerätetest und
Spec-Arbeit — sie stehen als eigene Tasks in P2 bzw. F.

### K-Tabelle — wartet auf eine Entscheidung des Nutzers

**Nicht gestrichen und nicht eingeplant.** Der Lead legt sie gesammelt vor. Wird ein Punkt
entschieden, gilt die Trägerspalte; ein **S-STM**-Punkt nur, wenn die Entscheidung **vor
S.18** fällt — sonst sind die Testbauten schon gelaufen, und er wartet auf die nächste
STM-Runde.

| Kennung | Offene Frage | Träger, falls ja | Hinweis der Spec |
|---|---|---|---|
| **A9** | Gesendeten LDR-Wert ungeklammert anzeigen? | S-STM | — |
| **A5** | 0 °C statt 125 °C bei Minusgraden — RTC-Vorzeichen | S-STM | Gehört laut Arbeitsliste in dieselbe Spec wie A2 (DS18xx) |
| **C23** | Latin-1 in `/api/stm32_log` wandeln oder ersetzen? | F | — |
| **C2** | Request-Zeile bewusst unbedingt lassen? | F | — |
| **C4** | Zielkodierung der acht C-Dateien | eigener Schritt | **Nicht mit S:** Eine Umkodierung ändert acht Dateien vollständig, und der vorgesehene Nachweis (`.hex` vorher/nachher gleich) verlangt einen Bau **ohne** andere Änderung |
| **C6u** | Legacy-Overlay-Formular, hängt an C19 | F | Erst C19 entscheiden |
| **C9c4** | Rückbaukriterium der Heap-Zeile | F | — |
| **B19** | Alte Sicherung `vor-durchlauf-3.2.15.tar.gz.enc` löschen? | Nutzer | Handlung des Nutzers auf seinem System |
| **E2** | Vier tote Bundle-Dateien löschen oder behalten? | W | — |
| **E3** | Kondensatorwert `C116` in KiCad | Nutzer | Ausserhalb dieses Repos |
| **E17** | `pending_weather_ticker_restore` umbenennen — erzwingt einen STM-Bump | S-STM? | **Empfehlung: nicht mit S.** A39 verschiebt in S genau den Ort, an dem das Flag gesetzt wird; eine Umbenennung im selben Flash macht den Diff von S.15 unlesbar und die Prüfung „Restore-Bedingung vollständig" in S.21 schwerer. Dazu müssten `CLAUDE.md` und `knowledge/quick-reference.md` im selben Zug mit. Besser die nächste STM-Runde |

---

## Was gleichzeitig laufen darf — und was nicht

**Erlaubt:**

- P2.1, P2.1b und P2.1d nacheinander (ein `pwa-developer`, dieselbe Datei); P2.2 und P2.1e
  (Lead) gleichzeitig dazu — P2.1e arbeitet mit einem zurückgebauten Stand im Scratchpad,
  nicht im Arbeitsbaum.
- S.7 und S.12 (lesend) jederzeit, auch während P2.
- P.18 und P.19 parallel zu P2 und zu S.1 bis S.5 — sie schreiben keinen Produktcode.
- F.4 und K.W jederzeit; E11 und F.4 nacheinander (gleiche Datei).

**Nicht erlaubt:**

- **Ein Release-Build, während irgendein Produktcode-Task offen ist** (§4.1). Das betrifft
  P2.6, S.9, S.23, F.8. **Die Testbauten S.18 und nach jedem S.19x sind keine
  Release-Builds** — sie laufen zwischen zwei STM-Tasks, wenn der `stm-developer` seinen
  Task abgeschlossen hat, und erzeugen weder ZIP noch Rollout.
- **S.1 vor P2.6**, **F.1 vor S.23** — baulich, nicht inhaltlich.
- **S.14 vor S.11** — die Zwischenabnahme muss den reinen ESP-Stand sehen.
- **Zwei Schreiber in derselben Datei.** `app.js` nur `pwa-developer`; `src/**` in S.14 bis
  S.19h nacheinander; `http.cpp` in S.5 und in F.1 bis F.5g nacheinander.
- **Ein zurückgebauter Prüfstand im Arbeitsbaum** (L268). Der künstliche Fehlerstand für
  P2.1e liegt im Scratchpad.
- **Kein Teammate führt `make` aus** (R1); Versionen nur über den `release-engineer` (R4).
- **Hardware ist exklusiv** (R5). Ein `pwa-tester`-Lauf und eine Lead-Messung nie
  gleichzeitig. **`network_client_set` bleibt in jedem Gerätetest dieses Pakets gesperrt.**

---

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um.
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings.
3. Review gegen `knowledge/architecture-checklist.md`.
4. Erst dann nächster Task oder Schreibübergabe.

**Vor jedem Patch an einer Firmware-Quelle: Kodierung *und* Zeilenende feststellen**
(DIR-015). **Bei jeder Suche in `src/**`: `grep -a`** (B21). **In `app.js` keine
Zeilennummern übernehmen** — die Datei bewegt sich.

**Wird ein Task grösser als beschrieben** — ein K-Punkt über rund zehn Zeilen, eine
Entwurfsfrage, eine fremde Datei —, wird er **nicht** still erweitert, sondern an den Lead
zurückgemeldet (AKK.3).

---

## Abschluss

- [ ] Alle Tasks erledigt oder **ausdrücklich mit Begründung ausgelassen**
- [ ] `./tools/guardrails.sh --full` mit Exit 0
- [ ] Alle Akzeptanzkriterien erfüllt — **oder als verfehlt berichtet** (L227, DIR-012)
- [ ] Je Einspielschritt ein Release-ZIP, Flash-Umfang und Reihenfolge benannt, sofort
      committet, getaggt und **gepusht** (DIR-011)
- [ ] Je Runde ein **vollständiger** Testdurchlauf mit M2 — oder als unvollständig berichtet
      (P2 ausdrücklich ohne eigenen; Proben P2.1c, P2.1e und S.26 tragen ihn)
- [ ] Jeder K-Punkt: erledigt mit Beleg, zurückgestellt mit gemessener Zahl, umgestuft mit
      Grund, oder wartet ausdrücklich auf eine Entscheidung
