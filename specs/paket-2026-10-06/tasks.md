# Tasks — Paket 2026-10-06 (Runden P, P2, S, P3, F, C4, verteilte Runde K)

**Erstellt:** 2026-10-06, Stand `401aae2`. Nachgeführt nach der Auslieferung von Runde P,
nach dem Gerätetest von 1.4.91 (L324) und nach den **Entscheidungen des Nutzers vom
07.10.2026** (zwei Durchgänge). Momentaufnahme (DIR-006).

**Status: freigegeben (Ent-1, 07.10.2026)** für P2, S, F und K.

**Rundenfolge:** **P (ausgeliefert) → P2 → S → P3 → F**, dazu **C4** als eigener Schritt
ohne Flash nach S. Runde K hat **keinen eigenen Flash**; ihre Punkte fahren mit P2, S und F
mit (`design.md` §6) und sind dort mit **[K]** markiert.

**Pro Task genau ein schreibender Agent** (R3, R4). Nach jedem Task läuft
`./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung weiter.

**Die Spalte „Hängt ab von" ist verbindlich, auch wenn die Dateien disjunkt sind** (R3b).
Der Besitz-Hook prüft, **wer** schreibt, nicht **wann**. Die wichtigste Abhängigkeit dieses
Pakets ist baulich, nicht inhaltlich: **Jeder Release-Build nimmt den ganzen Arbeitsbaum
mit** (`design.md` §4.1). Deshalb beginnt S erst nach dem P2-Build, P3 nach dem S-Build und
F nach dem P3-Build — auch wo die Inhalte nichts miteinander zu tun haben.

**`Nutzer` in der Agentenspalte heisst:** Der Lead liefert die Zeile fertig zum Einfügen,
der Nutzer führt sie aus. Er spielt **jedes** Update selbst ein. **M2:** B19 ist umgesetzt
(`e121058`); sobald der Nutzer die Passwortdatei `~/.config/wordclock/snapshot.pass`
angelegt hat, fährt der `pwa-tester` Phase 0 selbst. **Bis dahin bleibt M2 ein
Nutzerschritt** (`design.md` §5).

**Werkzeugvorbehalt:** `firmware-analyst`, `code-reviewer` und `ui-reviewer` haben kein
`Bash` (L238, L269). Git-Historie, Bauen und Gerätemessung gehören zum Lead.
**Prüfstände unter `tools/checks/` und Proben unter `tools/ui-mess/proben/` legt der Lead
ab** — `tools/**` ist sein Revier.

**Kennungen.** „N1", „N2" **ohne Zusatz** sind **Befunde aus der Nachzählung** vom
06.10.2026. Die zwei Befunde aus dem Review von P2 heissen hier **„P2-Review N1"** und
**„P2-Review N2"**, damit sie nicht verwechselt werden. **L321 / B40** ist der Befund aus dem
Gerätetest von 1.4.91 (network_scan überschreibt Eingabe), **L323 / C38** die stille Kürzung
von SSID und WLAN-Schlüssel, **L324** die Abnahme von 1.4.91. **L303** betrifft
`runConfirmedButtonAction`, **L306** das Umschalten von Flags. **„L-Befund Nachkommabit"**
ist der Befund zum RTC-Nachkommabit; seine Nummer vergibt der Lead. Die Entscheidungen heissen
**Ent-1 bis Ent-8** (`requirements.md`). **`vars.h` ohne Pfad meint
`ESP8266/ESP-uclock/vars.h`**; die STM-Datei heisst ausdrücklich `src/vars/vars.h`.

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
Massnahme 4 in engerer Form, die SSID-Auswahl (P2.1f), die beiden Befunde aus dem P2-Review
(P2.1h) — und der **Nachweis für L303 und L306** (P2.1e). **B35 gilt für Zeitserver und
Zeitzone erst als nachgewiesen, wenn B40 behoben ist.**

**Was von P noch aussteht**, soweit nicht schon geschehen — der Lead vermerkt es:

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P.17 | Lesende Abnahme am Gerät — **gelaufen**, Ergebnis in L324 | Lead | — | `check-pwa.sh` ohne Fehler, Dateimodul geöffnet (AKP.6); Smoketest; `watch-log.sh` mitgelesen | — | ☐ |
| P.18 | **Testdurchlauf Phasen 0–4 und 9**, mit M2 | `pwa-tester` | P.17, M2 | **AKZ.4.** Unvollständig ⇒ P nicht abgenommen | — | — |
| P.19 | `BEFUNDE.md` nachführen: alles aus der Tabelle oben, B32, B35 für Zeitserver/Zeitzone mit Vorbehalt bis B40 | `doc-writer` | P.18 | S10 läuft durch; jedes „erledigt" mit Datei und Funktion dieses Commits | ☐ | — |

---

## Runde P2 — Rest aus P, mit dem nächsten PWA-Upload

**Warum vor S:** P2 schreibt `app.js`, und jeder Release-Build von S nimmt `app-gz` mit. Ist
P2 vor S.1 gebaut, liegt beim S-Build kein offener PWA-Task im Baum (§4.1).

**Testdurchlauf — kein eigener `pwa-tester`-Durchlauf**, begründet in `design.md` §1.7:
B40 und P2.1f sind **Wettläufe**, am Gerät nicht gezielt herstellbar, in der Vorschau mit
verzögerter `network_scan`-Antwort schon (P2.1c). L303 und L306 brauchen eine
nachgebildete Fehlerantwort (P2.1e). Massnahme 4 ist in der Vorschau vollständig prüfbar.
Am Gerät zusätzlich lesend ohne Speichern (P2.8). Der volle Durchlauf S.26 läuft gegen einen
Stand mit P2 und prüft B35 am Zeitserver in Phase 2.

**Was offen bleibt, ausdrücklich:** Zwischen dem P2-Upload und S.26 sind B40 und P2.1f am
Gerät nur lesend belegt. L303 und L306 werden nie mit einem echten Fehlschlag am Gerät
belegt — der Nachweis ist die Probe.

**B17 ist aus P2 heraus** (Entscheidung des Leads vom 07.10.2026): Die Spuren 1 und 2 aus
L142 sind im Code abgedeckt; **P2.2 und P2.3 entfallen**. Die Spur 3 — ein leerer Logring
nach ESP-Neustart — wird beim ESP-OTA von S **lesend beobachtet** (S.11).

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P2.1 | **[K] Overlay-Text in `TEXT_FIELD_LIMITS`** (32 Byte, `OVERLAY_MAX_TEXT_LEN`, `ESP8266/ESP-uclock/vars.h:414`), angewandt im Overlay-Editor **und** in `importOverlaySettings()` | `pwa-developer` | — | **AKP.1** und **AKP.10** für den Overlay-Text: 17 Umlaute (34 Byte) ⇒ keine Anfrage, Meldung sichtbar; 16 Umlaute ⇒ Anfrage geht hinaus; präparierte Sicherung mit 33-Byte-Overlay-Text ⇒ übersprungen und genannt. **Das Gerät kürzt heute still** — die PWA-Prüfung ist bis zum F-OTA der einzige Schutz | ☐ | ☐ |
| P2.1b | **B40 / L321:** `updateNetworkControlsFromMeta()` setzt `network-timeserver-input.value` und `network-timezone-input.value` ohne Bedingung; ausgelöst nach jeder `network_scan`-Antwort über `refreshNetworkUi()`. Beide Felder über **dieselbe** `prefillDeviceValue()`-Regel wie die AP-SSID (L34) — keine zweite Regel daneben | `pwa-developer` | P2.1 | **AKP.14:** Ein **fokussiertes** oder **vom Nutzer geändertes** Zeitserver- oder Zeitzonenfeld wird von einer späten Scan-Antwort nicht mehr überschrieben; ein unberührtes Feld übernimmt den Gerätewert weiterhin. Bericht nennt, ob weitere Felder dieselbe Form haben — **gemeldet, nicht mitkorrigiert** | ☐ | ☐ |
| P2.1c | **Probe B40 und SSID-Auswahl** unter `tools/ui-mess/proben/`: Vorschau mit verzögerter `network_scan`-Antwort; Eingabe in Zeitserver und Zeitzone **und eine ungespeicherte Auswahl in `network-ssid-select`**, dann Antwort ausliefern, Felder ablesen. Drei Fälle je Feld: fokussiert, geändert und verlassen, unberührt | Lead | P2.1b, P2.1f | **Einmal fehlgeschlagen gegen 1.4.91** (DIR-014), bestanden gegen P2.1b/P2.1f; die Probe meldet die Zahl der geprüften Fälle (drei je Feld, **neun** gesamt). **Nach P2.1h erneut grün** | ☐ | — |
| P2.1d | **Massnahme 4 in engerer Form — nur für die vier vorbefüllten Netzwerkfelder:** `handleDirtyFormInteraction()` setzt `hasUnsavedEdits = true` bei **jeder** Eingabe, ohne mit dem Ausgangswert zu vergleichen. Am Gerät beobachtet (L324). Für die vier vorbefüllten Netzwerkfelder gilt ein Feld künftig nur als geändert, wenn sein Wert vom Ausgangswert abweicht. **Für alle anderen Felder bleibt das Verhalten wie heute** | `pwa-developer` | P2.1b | **AKP.15** in der Vorschau, an diesen vier Feldern: tippen und Ausgangswert wiederherstellen ⇒ **kein** Dialog beim Modulwechsel; echte Änderung ⇒ Dialog **weiterhin**. **Gegenprobe an einem Feld ausserhalb:** dort erscheint der Dialog nach Tippen und Wiederherstellen weiterhin. Der Ausgangswert ist **derselbe**, den `prefillDeviceValue()` aus P2.1b kennt | ☐ | ☐ |
| P2.1f | **SSID-Auswahl:** `network-ssid-select` (`app.js` um Zeile 3550 laut Lead) wird bei jedem Scan per `innerHTML` neu aufgebaut, mit `selected` auf der Geräte-SSID. Ein später Scan setzt eine **ungespeicherte Auswahl** zurück — dasselbe Muster wie B40. Die Auswahl des Nutzers überlebt den Neuaufbau, solange sie vom Gerätewert abweicht und nicht gespeichert ist; **dieselbe Regel** wie P2.1b/P2.1d, keine dritte | `pwa-developer` | P2.1d | **AKP.17**, abgenommen über die Probe P2.1c. Neu gefundene Netze erscheinen weiterhin in der Liste; die Optionen gehen weiter über `escapeHtml` oder DOM-Knoten, nie ungeschützt per `innerHTML` (Checkliste §3) | ☐ | ☐ |
| P2.1h | **Zwei Befunde aus dem P2-Review:** **P2-Review N1** — der Ersatz-Ausgangswert `networks[0]`; **P2-Review N2** — `unsavedEditFields` nach dem Vorbefüllen | `pwa-developer` | P2.1f | **AKP.18:** Jeder der beiden Befunde ist im Bericht mit Ursache und Fundstelle (Funktionsname) beschrieben und behoben; die Probe P2.1c läuft danach erneut grün; der `code-reviewer` bestätigt in P2.4, dass beide geschlossen sind. Wächst einer über die Vergleichsregel der vier Netzwerkfelder hinaus, wird er zurückgemeldet (AKK.3) | ☐ | ☐ |
| P2.1e | **Probe L303 und L306** unter `tools/ui-mess/proben/`, mit **nachgebildeter Fehlerantwort**. **L303** (`runConfirmedButtonAction`): Eine Formatier-Antwort mit Fehler (`formatLittleFsFromFiles`) meldet **nicht** „formatiert". **L306** (`toggleFlagButton`/`finishButtonFeedback`): Nach einem fehlgeschlagenen Umschalten steht der Knopf wieder auf seinem Zustand, **nicht** auf „schaltet…" | Lead | — | **AKP.16.** Jede der beiden Proben ist **einmal fehlgeschlagen**, gegen einen **künstlich zurückgebauten** Stand (DIR-014) — 1.4.91 enthält die Korrekturen bereits. Der zurückgebaute Stand liegt im Scratchpad, nicht im Arbeitsbaum (L268). Fallzahl gemeldet | ☐ | — |
| ~~P2.2~~ | ~~B17, Eingrenzung am Gerät~~ — **entfällt** (07.10.2026): Spuren 1 und 2 im Code abgedeckt, Spur 3 in S.11 | — | — | — | — | — |
| ~~P2.3~~ | ~~B17, Umsetzung~~ — **entfällt** (07.10.2026) | — | — | — | — | — |
| P2.4 | Review | `code-reviewer` | P2.1h, P2.1c, P2.1e | `TEXT_FIELD_LIMITS` gegen `ESP8266/ESP-uclock/vars.h` (jetzt **zehn** Werte, Koordinaten doppelt gezählt); **B40, Massnahme 4 und SSID-Auswahl nutzen dieselbe Regel und denselben Ausgangswert** — keine zweite Fassung; **Massnahme 4 wirkt nur auf die vier vorbefüllten Netzwerkfelder**; P2-Review N1 und N2 geschlossen; keine neuen leeren `catch`; kein ungeschütztes `innerHTML` mit SSID-Namen | — | ☐ |
| P2.5 | `APP_VERSION` **und** `CACHE_NAME` anheben | `release-engineer` | P2.4 | S4 zeigt den neuen Stand | ☐ | — |
| P2.6 | `git status` (kein offener Produktcode-Task), Build inkl. `app-gz`, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | P2.5 | R2; `guardrails.sh --full` Exit 0 | ☐ | — |
| P2.7 | **Nutzer:** `./tools/install-app.sh --check`, `./tools/install-app.sh` | **Nutzer** | P2.6 | Seite meldet die neue App-Version | — | — |
| P2.8 | Abnahme | Lead | P2.7 | `check-pwa.sh` ohne Fehler; **lesend, ohne Speichern:** Netzwerk-Modul öffnen, in Zeitserver und Zeitzone tippen und eine andere SSID auswählen, den nächsten Scan abwarten, alles unverändert; dann Ausgangswerte wiederherstellen und Modul wechseln, kein Dialog; `watch-log.sh` mitgelesen | — | ☐ |
| P2.9 | `BEFUNDE.md`: Overlay-Text PWA-Seite, B40/L321, SSID-Auswahl, P2-Review N1 und N2, **Massnahme 4 — ausdrücklich nur für die vier vorbefüllten Netzwerkfelder umgesetzt, nicht allgemein**; **L303 und L306** (Nachweis über P2.1e); B35 für Zeitserver/Zeitzone (Vorbehalt bis S.26); **B17: Spuren 1 und 2 im Code abgedeckt, Spur 3 offen bis S.11**. **Dazu die Entscheidungen vom 07.10.2026:** **C2 und C9c4 geschlossen** (mit Datum und Verweis auf die Entscheidung), **B19 erledigt** (`e121058`), **E2 erledigt** (`8dfba36`, die Dateien lagen unter `ESP8266/ESP-uclock/`, nicht unter `data/app/`), **E3 offen** | `doc-writer` | P2.8 | S10 läuft durch. Massnahme 4 steht **nicht** als „erledigt" ohne Einschränkung | ☐ | — |

**Einspielreihenfolge:** nur PWA. Keine Firmware vorausgesetzt.

---

## Runde S — Brücke, mit A5 und E17

**Warum ESP zuerst:** Die neue Wirkung entsteht mit dem STM; dazwischen läuft der neue ESP
gegen den alten STM — der Rückfallnachweis AKS.6 (S.11).

**Warum S.1 erst nach P2.6:** gemeinsamer Arbeitsbaum, `release-zip` baut alle drei
Komponenten (`design.md` §4.1).

**Der STM-Teil:** erst der **Kern** (S.14–S.17), darin **E17 als reine Umbenennung direkt
nach A39** (S.15b), dann ein **Testbau** (S.18), dann **A5** (mit dem Nachkommabit, falls
S.7 es bestätigt) und die **K-Punkte einzeln, je mit Testbau danach** (S.18b,
S.19a–S.19i). Die seriellen Testbauten hat der Lead freigegeben (Ent-7); Reserve und Regel
stehen in Ent-2.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| S.1 | ESP: Eröffnungszeile und **Abschlussmarke** auswerten; `var_sync_check()` verlangt die Marke **nur**, wenn die Eröffnungszeile kam | `esp-developer` | P2.6 | **AKS.3**, Rückfall ohne Eröffnungszeile im Quelltext ausgewiesen | ☐ | ☐ |
| S.2 | ESP: Zuordnungszeile `ACK <xy>` **vor** jedem `.` und `!v`. **Festlegen und im Bericht begründen**, woher die zwei Zeichen kommen und was bei einer unmarkierten Zeile gesendet wird (`design.md` §2.4) | `esp-developer` | S.1 | **AKS.4.** Der Punkt bleibt ein nackter Punkt | ☐ | ☐ |
| S.3 | ESP: Zähler „unmarkiert nach erster Marke" **in den Logring**; Markenpflicht für die **drei aufgezählten** Kommandoarten mit `!v` | `esp-developer` | S.2 | **AKS.7, AKS.8** (Quelltextteil). Kein Eingriff in `http.cpp` für C26 (`design.md` §4.2) | ☐ | ☐ |
| S.4 | ESP: **C31** — `eepromdata.cpp` gibt „gesetzt"/„leer" statt der Schlüssel aus | `esp-developer` | S.3 | **AKS.10** (Quelltextteil). Kodierung und Zeilenende der Datei vorher festgestellt und im Bericht genannt | ☐ | ☐ |
| S.4b | ESP: **A5, ESP-Teil** — (1) neue Variable `RTC_TEMP_HALF_DEG_NUM_VAR` **am Ende** des `NUM_VARIABLE`-Enums in `ESP8266/ESP-uclock/vars.h` (**Index 49**, unmittelbar vor `MAX_NUM_VARIABLES`; heute ist `UPTIME_SECONDS_HI_NUM_VAR` Index 48); in `vars_init()` mit **`0x8000` („unbekannt")** vorbelegen. (2) **Legacy-Seite zeigt Minusgrade** (Ent-8): Die RTC-Anzeige in `http.cpp` (heute `:3739-3747`) liest Index 49 vorzeichenrichtig und fällt auf Index 21 zurück, wenn Index 49 `0x8000` trägt. (3) Wo der ESP den Index bei einer Korrekturänderung **selbst nachrechnet** (Legacy `savetcorrrtc`, `:3686-3704`, und der Korrektur-Endpunkt, `:9549` ff.), rechnet er Index 49 **im selben Schritt** nach — **eine** Hilfsfunktion für beide Stellen | `esp-developer` | S.4 | **AKS.15** (ESP-Teil): Quelltext; Index 49 auf beiden Seiten gleich (Abgleich mit S.18b); Vorgabewert `0x8000`, nicht `0`; Legacy-Formatierung vorzeichenrichtig (−3 halbe Grad ⇒ „-1.5 °C"); Prüfstand für die Legacy-Formatierung mit −20, −1, 0, +49, `0x8000`. **Kodierung von `http.cpp` (UTF-8) und `vars.cpp` vor dem Patch feststellen** | ☐ | ☐ |
| S.5 | **[K] N1:** `http_overlays()` (Legacy) prüft `oidx < MAX_OVERLAYS && oidx <= n_overlays`, **bevor** irgendetwas geschrieben oder `n_overlays` erhöht wird. **Sicherheitsausnahme** von der Trägerregel (`design.md` §6.2) | `esp-developer` | S.4b | **AKS.14.** Prüfstand im Scratchpad mit den Typen des Ziels (L256), an S.20 übergeben | ☐ | ☐ |
| S.6 | Review ESP-Seite | `code-reviewer` | S.5 | Vier Punkte; besonders: Kann die Markenpflicht einen Dauerzustand erzeugen? Endet die Markenerwartung mit **ihrem** Abgleich? Bilden ESP und STM-Entwurf dieselben zwei Zuordnungszeichen? N1: Ist auch der `disp`-Zweig (`http.cpp:4684`) gesichert **oder** als eigener Befund gemeldet? **A5:** Liefert die Einstellungs-API das Feld mit Index 49 aus, behandelt eine alte PWA es als unbekannt, und rechnen beide Korrekturstellen Index 49 über **dieselbe** Hilfsfunktion nach? **Die Antwort steht am Code** | — | ☐ |
| S.7 | **Analyse, rein lesend:** (1) **AKS.11** — kann eine unbekannte ESP-Zeile über die STM-Logausgabe in den ESP-Logring gelangen? (2) **N1, STM-Seite** — fängt der STM einen Overlay-Index ≥ 32 ab, und kann `n_overlays` über `overlay_set_n_overlays` (A3) über 32 gesetzt werden? (3) Welche Grenze muss A3+A48 nehmen, damit sie zu `overlay.c` passt („S2 im Katalog" laut Nachzählung, ● nicht nachgesehen). **(4) Nachkommabit:** Liest `rtc_get_temperature_index()` das Nachkommabit richtig? Der Code nimmt `(buffer[1] & 0x02) >> 1` (`src/rtc/rtc.c:364`); beim DS3231 liegen die Nachkommabits nach Datenblatt in **Bit 7 (0,5 °C) und Bit 6 (0,25 °C)** des Registers `0x12` | `firmware-analyst` | — | Antworten mit Fundstellen. (1) bei ja: neuer Befund über S.27. (2) und (3) gehen als Vorgabe an S.19c. **(4): Bestätigt sich der Bitfehler, wird er in S.18b mit A5 korrigiert — die Freigabe des Nutzers liegt vor (07.10.2026).** Bestätigt er sich nicht, steht die Begründung im Bericht, und der L-Befund wird mit diesem Beleg geschlossen | — | — |
| S.8 | ESP-Version anheben | `release-engineer` | S.6 | S4 zeigt den neuen Stand | ☐ | — |
| S.9 | Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | S.8 | `git status` ohne offenen Produktcode-Task; `guardrails.sh --full` Exit 0 | ☐ | — |
| S.10 | **Nutzer-Sitzung S1:** ESP einspielen | **Nutzer** | S.9 | neue ESP-Version in `/api/update_status` | — | — |
| S.11 | **Zwischenabnahme: neuer ESP gegen alten STM** | Lead | S.10 | **AKS.6:** ESP-Neustart ⇒ Vollabgleich ohne Timeout und ohne Reset, `var_send_timeout_cnt` steigt nicht, Diagnosefolge lückenlos. **AKS.10:** beide Schlüsselzeilen im Mitschnitt und in `/api/stm32_log` tragen nur „gesetzt"/„leer". **AKS.14** lesend: Legacy-Overlayseite lädt. **AKS.15** lesend: Index 49 steht auf `0x8000` (32768); PWA **und Legacy-Seite** zeigen die RTC-Temperatur unverändert (Rückfall auf Index 21). **B17, Spur 3, lesende Beobachtung:** Unmittelbar nach dem ESP-Neustart die PWA laden, Modul `system`, Logfenster **ohne Reload** beobachten — was steht im Feld, solange der Ring leer ist, und füllt es sich von selbst? Ergebnis mit Zeitstempeln an S.27. Dazu `install-app.sh --check`, Smoketest samt Update-Quelle (DIR-009). `watch-log.sh` mitgelesen. **Schlägt AKS.6 fehl, geht S zurück und wird nicht am STM fortgesetzt** | — | ☐ |
| S.12 | **Flash-Gate, Stufe 1 — Vorabschätzung** des F103-Bedarfs: Kern (S.14–S.17, E17 kostet null) **und** A5 samt Nachkommabit (S.18b) **und** jeder Punkt S.19a–S.19i, je Unter- und Obergrenze, Herleitung aus gemessenen Zuwächsen (L165, L289, L296) und neuen unbedingten Zeichenketten (L168) | `firmware-analyst` | — | **AKS.0** Stufe 1. Bericht mit Summe der Obergrenzen gegen 1'924 Byte minus **1'024 Byte Reserve** (Ent-2). **Entscheidet nichts** — passt schon der Kern nicht, geht der Bericht über den Lead an den Nutzer | — | — |
| S.13 | **Nur wenn S.12 schon für den Kern nicht passt:** Entscheidung, was entfällt oder wo gespart wird | **Nutzer** | S.12 | Entscheidung im Gespräch; der Lead vermerkt entfallene Tasks hier **ausdrücklich** | — | — |
| S.14 | STM: Eröffnungszeile und Abschlussmarke senden — **als `var <freierBuchstabe>…` über `var_send_buf()`** | `stm-developer` | S.11, S.12, (S.13) | **Vier Auflagen aus `design.md` §2.2, einzeln im Bericht belegt:** (1) Buchstabe ausserhalb `N n S T D A C M O t a l I`; (2) Nutzlast endet nicht auf `*` + vier Hexziffern; (3) Nutzlast unter 72 Zeichen; (4) **kein eigenes Top-Level-Präfix**. Die Eröffnungszeile gilt nur für **ihren** Abgleich | ☐ | ☐ |
| S.15 | STM: **A39**, Variante (b) — `IPADDRESS`-Zweig merkt vor, Ticker **nach** dem Abgleich; `pending_weather_ticker_restore` wandert mit | `stm-developer` | S.14 | **AKS.1.** Bericht weist nach, dass keine der vier Teilbedingungen des Restores vereinfacht ist. **Noch mit dem alten Namen** — die Umbenennung ist S.15b | ☐ | ☐ |
| S.15b | STM: **E17 — reine Umbenennung** `pending_weather_ticker_restore` → `pending_ticker_restore` in `src/**`, **ohne jede andere Änderung** | `stm-developer` | S.15 | **AKS.16:** Der Diff besteht **ausschliesslich** aus dem ersetzten Bezeichner — der Lead prüft das mit `git diff --word-diff`, jede andere Zeile ist ein Fehlschlag. `grep -a` findet den alten Namen in `src/**` nicht mehr. Kein Zuwachs im Testbau S.18 durch S.15b (Bezeichner kosten keinen Flash) | ☐ | ☐ |
| S.15c | **E17, Lead-Teil:** `CLAUDE.md` (Architektur-Invarianten) auf den neuen Namen; `tools/**` und `.claude/**` nach dem alten Namen durchsuchen und nachziehen | Lead | S.15b | `grep` über `CLAUDE.md`, `tools/**`, `.claude/**` findet den alten Namen nicht mehr; Guardrails laufen durch | ☐ | — |
| S.15d | **E17, Doku-Teil:** `knowledge/quick-reference.md` und `knowledge/architecture-checklist.md` auf den neuen Namen | `doc-writer` | S.15b | `grep` über `knowledge/**` findet den alten Namen nicht mehr; der Sinn der Invariante (vier Teilbedingungen) ist unverändert | ☐ | — |
| S.16 | STM: Zuordnungszeile lesen, dem folgenden `.`/`!v` zuordnen, unpassende Quittung **verwerfen und zählen**; Prüfstand im Scratchpad, Typen angeglichen (L256) | `stm-developer` | S.15b | **AKS.5:** verspätete Quittung ⇒ **eine** Nachsendung; gemerkte Zuordnung ohne Quittung wird verbraucht. Prüfstand an S.20 | ☐ | ☐ |
| S.17 | STM: **A16** — `watchdog_reload()` in Tetris und Snake (Ent-6) | `stm-developer` | S.16 | Quelltext: in beiden Spielschleifen | ☐ | ☐ |
| S.18 | **Flash-Gate, Stufe 2 — Testbau nach dem Kern:** nur Ziel F103, kein Release-ZIP, kein Rollout | Lead | S.17 | Gemessener Rest notiert. **Liegt er schon unter 1'024 Byte:** Halt, Bericht an den Nutzer | — | — |
| S.18b | STM: **A5 — echte Minusgrade**, und **das Nachkommabit, falls S.7 (4) es bestätigt**. Vorzeichenbehaftete Rechnung in `rtc_get_temperature_index()` (`(int8_t) buffer[0]`); halbes Grad aus **Bit 7** von Register `0x12` statt aus Bit 1; neues Feld für den Wert in halben Grad mit Vorzeichen; neue Variable `RTC_TEMP_HALF_DEG_NUM_VAR` (**Index 49**, `src/vars/vars.h`, am Enum-Ende) als `int16` im Zweierkomplement, **`0x8000` = kein Messwert**; gesendet im Vollabgleich und bei **Änderung des vorzeichenbehafteten Werts** (heute vergleicht `src/main.c` um Zeile 4315 nur den begrenzten Index). **`RTC_TEMP_INDEX_NUM_VAR` (21) bleibt auf 0..250 begrenzt, die Anzeige an der Uhr unverändert** (Ent-8: bei Minusgraden 0 °C) | `stm-developer` | S.18, S.7 (4) | **AKS.15** (STM-Teil) und **AKS.17** (Nachkommabit), Prüfstand mit **Registerbytes**: −10,0 °C, −0,5 °C, 0,0 °C, +24,5 °C, **+24,0 °C mit Bit 6 allein (0,25 °C ⇒ kein halbes Grad)**, **+24,0 °C mit Bit 7 (⇒ +24,5 °C)**, Korrektur ±20, Lesefehler; Index 21 für alle negativen Werte 0; **ungerade Indizes treten auf** (vorher nie); ein Wechsel von −1 °C auf −8 °C löst einen Versand aus. **Danach Testbau (Lead)**, Zuwachs notiert. **Regel bei Unterschreitung: anhalten und vorlegen** | ☐ | ☐ |
| S.19a | **[K] A20** — Geheimnisse aus der Timeout-Meldung | `stm-developer` | S.18b + Testbau | Abnahmesatz A20 (K-Tabelle); danach Testbau, Zuwachs in die K-Tabelle. **Unterschreitung: anhalten** | ☐ | ☐ |
| S.19b | **[K] A13 + A47** — `strncpy` ohne Abschlussbyte in `esp8266.c`, zusammen, rund 8 Zeilen | `stm-developer` | S.19a + Testbau | Abnahmesatz A13+A47, **AKS.12**; danach Testbau. **Unterschreitung: anhalten** | ☐ | ☐ |
| S.19c | **[K] A3 + A48** — Bereichsprüfung `overlay_set_n_overlays` und Indexguard in `schedule_esp8266_overlay()`, Grenze nach S.7 (3) | `stm-developer` | S.19b + Testbau, S.7 | Abnahmesatz A3+A48 — **über den Prüfstand (AKK.6), nicht am Gerät**; danach Testbau. **Unterschreitung: anhalten** | ☐ | ☐ |
| S.19d | **[K] A24** — Wortindizes in `tables_fill_words()` | `stm-developer` | S.19c + Testbau | Abnahmesatz A24 — **über den Prüfstand**; danach Testbau. Unterschreitung: zurückstellen, weiter | ☐ | ☐ |
| S.19e | **[K] A45** — Typbereinigung in der Snake-Animation, 2 Zeilen | `stm-developer` | S.19d + Testbau | Abnahmesatz A45; danach Testbau. Unterschreitung: zurückstellen, weiter | ☐ | ☐ |
| S.19f | **[K] A14** — Variablenindex in `var_send_string()` maskieren | `stm-developer` | S.19e + Testbau | Abnahmesatz A14; danach Testbau. Unterschreitung: zurückstellen, weiter | ☐ | ☐ |
| S.19g | **A46** — drei Verwerfungspfade an den gemeinsamen Zähler | `stm-developer` | S.19f + Testbau | **AKS.13**; Prüffälle an S.20; danach Testbau. Unterschreitung: zurückstellen, weiter | ☐ | ☐ |
| S.19h | **[K] A49** — Abweisungsmeldung je Grund (1-Byte-Maske) | `stm-developer` | S.19g + Testbau | Abnahmesatz A49; danach Testbau. Unterschreitung: zurückstellen, weiter | ☐ | ☐ |
| S.19i | **[K] A9** — gesendeten LDR-Rohwert **ungeklammert** (Entscheidung vom 07.10.2026) | `stm-developer` | S.19h + Testbau | Abnahmesatz A9; danach Testbau. Unterschreitung: zurückstellen | ☐ | ☐ |
| S.20 | **Prüfstände ablegen und Guardrails nachziehen:** AKS.5, AKS.14, **AKS.15** und **AKS.17** unter `tools/checks/`; A46-Fälle in `htoi-laenge.c` (S13); **S15 (`idx-guard.sh`) auf A3+A48 und A24 erweitern**; **S7 nachführen** (zwei neue `watchdog_reload()` aus A16) | Lead | S.19i | **AKK.6:** S15 meldet für die neuen Guards nicht mehr „nicht abgedeckt", Fallzahl gemeldet. Jeder Prüfstand **einmal fehlgeschlagen** gegen die alte Fassung (DIR-014) — für AKS.17 heisst das: Der Registerfall mit Bit 7 schlägt gegen das alte `rtc.c` fehl. S7 zeigt den neuen Bestand | ☐ | — |
| S.21 | Review STM-Seite | `firmware-analyst` | S.20 | Vier Punkte; **die vier Auflagen aus S.14 einzeln**; die drei neuen Zustände auf **jedem** Pfad aufgelöst; Restore-Bedingung vollständig (**unter dem neuen Namen**); **jeder K-Punkt im Brückenpfad einzeln markiert**; jeder zurückgestellte Punkt ist **ganz** zurückgenommen; **A5:** Index 49 auf beiden Seiten gleich, **beide Enums gleich lang**, `0x8000` nie ein gültiger Messwert, Änderungserkennung am vorzeichenbehafteten Wert, Nachkommabit aus Bit 7 | — | ☐ |
| S.22 | STM-Version anheben | `release-engineer` | S.21 | S4 zeigt den neuen Stand | ☐ | — |
| S.23 | Build, **Gegenprobe des Gates**, Release-ZIP, Rollout, **Commit + Tag + Push** — **im selben Commit** wie S.15c und S.15d | Lead | S.22, S.15c, S.15d | Gemessene F103-Grösse aus S8b neben die Schätzung S.12 und die Testbauten gestellt. **Rest unter 1'024 Byte: kein Rollout**, Bericht an den Nutzer. Schätzung über ihrer Obergrenze ⇒ Befund an S.27. Release-Notiz nennt jeden enthaltenen und jeden zurückgestellten Punkt, **und dass die RTC-Temperatur jetzt halbe Grad zeigen kann** | ☐ | — |
| S.24 | **Nutzer-Sitzung S2:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh`, **dann 60 s Tetris oder Snake**, **dann M2** (entfällt als Nutzerschritt, sobald die Passwortdatei aus B19 angelegt ist) | **Nutzer** | S.23 | Erwartete STM-Version; **AKS.9:** kein Watchdog-Reset im Mitschnitt — fällt der Spiellauf aus, gilt AKS.9 als **nicht erfüllt**; M2 liegt vor | — | — |
| S.25 | Abnahme am Gerät | Lead | S.24 | **AKS.2, AKS.3, AKS.5, AKS.7, AKS.8.** Hauptloop-Zähler gegen L226 während eines ESP-Neustarts; **IP-Lauftext vollständig** (Auge); Erfolgszeile erst nach der Marke; Zähler im Logring; null `!v`-Abweisungen im gesunden Zustand; `diff-snapshot.sh --soll` (**Index 49 ist neu** und darf als einzige neue Zeile erscheinen; Index 21 darf sich um ±1 verschieben, wenn S.18b das Nachkommabit korrigiert hat); **AKS.15 lesend:** Index 49 = Index 21 bei Raumtemperatur, Legacy-Seite zeigt dieselbe Temperatur wie die PWA; **AKS.17 lesend:** über ein paar Minuten tritt bei Raumtemperatur auch ein **ungerader** Index auf, sofern die Temperatur ihn hergibt — **als Beobachtung, nicht als Abnahme**; tragend ist der Prüfstand; **ein Blick auf die Uhr** (A24, A45 sind Anzeigepfade); `watch-log.sh` mitgelesen. **Nicht verlangt:** STM-Guards am Gerät (AKK.6), Minusgrade am Gerät (nicht herstellbar) | — | ☐ |
| S.26 | **Testdurchlauf Phasen 0–4 und 9** — gegen einen Stand, der P2 enthält | `pwa-tester` | S.25 | **AKZ.4**, M2 aus S.24. Bericht nennt die enthaltenen Punkte. **Zusätzlich:** (1) **B35 am Zeitserver** in Phase 2. (2) **B36** über einen zu langen Update-Host, den der ESP mit `error=2` abweist — die Fehlermeldung bleibt stehen und wird **nicht** von einer Erfolgsmeldung überschrieben, und **Phase 9 belegt**, dass der Gerätewert unverändert ist. (3) **Phase 3** prüft den Overlay-Text mit 33 Byte in der PWA. **Nicht hier:** L303 und L306 (P2.1e) | — | — |
| S.27 | `BEFUNDE.md` nachführen: A39, A42/L260, A35 Teil 1, C26, A16, C31, A46, N1, **A5/L38 (STM- und ESP-Teil samt Legacy; PWA-Teil folgt in P3)**, **L-Befund Nachkommabit** (geschlossen mit Prüfstand, oder mit Begründung aus S.7 widerlegt), **E17**, **B17 Spur 3** (Beobachtung aus S.11; danach B17 geschlossen oder mit Beleg offen), alle K-Punkte mit Träger S (erledigt **oder** „zurückgestellt: Flash, gemessen +n Byte"), Gate-Ergebnis, Ergebnis S.7, B35 für Zeitserver/Zeitzone, **B36** | `doc-writer` | S.26 | S10 läuft durch; jedes „erledigt" mit Datei:Zeile dieses Commits | ☐ | — |

**Einspielreihenfolge: erst ESP (S.10), dann STM (S.24).** Dazwischen die Zwischenabnahme
S.11.

**Ein zurückgestellter Punkt wird ganz zurückgenommen**, nicht halb: Der `stm-developer`
setzt seine Änderung zurück, der Lead misst erneut, und der Punkt steht in der K-Tabelle
als „zurückgestellt: Flash, gemessen +n Byte". **Die Regel (Ent-2, entschieden):** Reserve
1'024 Byte; bei **A5**, A20, A13+A47 und A3+A48 **anhalten und dem Nutzer vorlegen**, bei
den übrigen **zurückstellen und weitermachen**. Für A5 ist „anhalten" eine Festlegung dieser
Spec, nicht des Nutzers — A5 ist seine ausdrückliche Bestellung, und sie still
zurückzustellen hiesse, eine Entscheidung für ihn zu treffen.

---

## Runde P3 — A5 in der PWA

**Warum eine eigene kleine Runde nach S:** Der PWA-Teil von A5 liest eine Variable, die erst
die Firmware aus S liefert. **Firmware vor PWA** (L241). Er gehört nicht in P2, weil P2
gebaut wird, bevor es Index 49 gibt, und nicht in F, weil F eine reine ESP-Runde ist. **Die
Legacy-Seite zeigt Minusgrade schon ab S** (S.4b), weil der ESP sie selbst ausliefert.

**Testdurchlauf:** **kein eigener**; F.12 läuft gegen einen Stand mit P3. Minusgrade sind am
Gerät nicht herstellbar — tragend ist die Vorschauprobe P3.2.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| P3.1 | **A5, PWA-Teil:** RTC-Temperatur aus Index 49 lesen (`int16` im Zweierkomplement, halbe Grad), **mit Rückfall auf Index 21**, wenn Index 49 fehlt (alter ESP) oder `0x8000` trägt (alter STM). Formatierung **vorzeichenrichtig** — `formatHalfDegreeValue()` rechnet heute mit `Math.floor` und `%`, das bei negativen Werten falsch rundet (−3 halbe Grad ergäbe „−2.5" statt „−1.5") | `pwa-developer` | S.23 | **AKP3.1** | ☐ | ☐ |
| P3.2 | **Probe A5** unter `tools/ui-mess/proben/`: Vorschau mit nachgebildeten `numvars` — Index 49 mit den Werten −20 (−10,0 °C), −1 (−0,5 °C), 0, +49 (+24,5 °C), `0x8000`, fehlend | Lead | P3.1 | Jeder Fall zeigt den erwarteten Text; **einmal fehlgeschlagen** gegen den Stand vor P3 (DIR-014), Fallzahl gemeldet (sechs) | ☐ | — |
| P3.3 | Review | `code-reviewer` | P3.2 | Rückfall auf Index 21 in **beiden** Fällen (fehlt, `0x8000`); keine zweite Formatierfunktion neben der bestehenden; **PWA und Legacy formatieren gleich** (−3 halbe Grad ⇒ „−1.5 °C" in beiden); DS18xx-Anzeige unverändert (A2 ist nicht Teil) | — | ☐ |
| P3.4 | `APP_VERSION` **und** `CACHE_NAME`; Build, Release-ZIP, Rollout, **Commit + Tag + Push** | `release-engineer`, dann Lead | P3.3 | wie P2.5/P2.6 | ☐ | — |
| P3.5 | **Nutzer:** `./tools/install-app.sh --check`, `./tools/install-app.sh` | **Nutzer** | P3.4 | neue App-Version | — | — |
| P3.6 | Abnahme | Lead | P3.5 | `check-pwa.sh` ohne Fehler; die RTC-Temperatur in der PWA ist bei Raumtemperatur **dieselbe** wie auf der Legacy-Seite und wie Index 21 | — | ☐ |
| P3.7 | `BEFUNDE.md`: A5/L38 geschlossen (alle drei Teile), mit dem Vermerk, dass Minusgrade nur am Prüfstand und in der Vorschau belegt sind und die Uhranzeige bei Minusgraden nach Ent-8 weiterhin 0 °C zeigt | `doc-writer` | P3.6 | S10 läuft durch | ☐ | — |

**Einspielreihenfolge:** nur PWA, **nach** S.24.

---

## Runde F — Flash-Überwachung

**Warum F.1 erst nach P3.4:** F-Code im Baum während eines anderen Release-Builds landete im
ESP-Fabrikat jenes ZIPs (`design.md` §4.2). **F.4 darf jederzeit laufen** — es ist Werkzeug,
nicht Fabrikat.

**Alle `esp-developer`-Tasks in F schreiben `http.cpp`** (bzw. F.5d `httpclient.cpp`) und
laufen deshalb **seriell** in der Reihenfolge der Tabelle.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| F.1 | Legacy-Flashzweig **und** Legacy-Auswahlliste über `http_remote_stm32_filename_matches()`; Hinweis mit Rückweg `./tools/flash-stm.sh` bei leerer Liste | `esp-developer` | P3.4 | **AKF.1, AKF.2, AKF.3.** `grep` findet die Funktion im Flashzweig, in der Liste und im API-Endpunkt; `fname + 6` ohne Längenprüfung ist weg. **Nicht scharf fahren** | ☐ | ☐ |
| F.2 | **Ent-4 = ja:** eigene Kennung für „remove failed" in `fs_remove` | `esp-developer` | F.1 | **AKF.7.** Kennung 6 nur noch für „nicht gefunden"; die PWA zeigt die neue Kennung über den Rückfalltext mit Detail (`describeApiError()`), bis sie eine Übersetzung bekommt | ☐ | ☐ |
| F.3 | **Ent-5 = ja:** API-Auswahlliste über dieselbe Funktion — bei 65535 **leer** | `esp-developer` | F.2 | **AKF.8.** F.6 prüft, wie die PWA die leere Liste darstellt | ☐ | ☐ |
| F.4 | `tools/smoke-device.sh`: `HARDWARE_CONFIGURATION` = 65535 als eigene Stufe | Lead | — | **AKF.4**, Gegenprobe gegen festen Testwert, einmal fehlgeschlagen | ☐ | ☐ |
| F.5a | **[K] C27 + N2** — `fs_show` prüft die Dateigrösse | `esp-developer` | F.3 | Abnahmesatz C27+N2 | ☐ | ☐ |
| F.5b | **[K] E12** — `tft_flags_set` und `dfplayer_bell_flags_set` über `http_get_on_off_value` | `esp-developer` | F.5a | Abnahmesatz E12 | ☐ | ☐ |
| F.5c | **[K] C25** — Verlustzähler trennen | `esp-developer` | F.5b | Abnahmesatz C25 | ☐ | ☐ |
| F.5d | **[K] C9k** — `httpclient_read_header()` wartet über den Helfer aus L152 | `esp-developer` | F.5c | Abnahmesatz C9k | ☐ | ☐ |
| F.5e | **[K] Overlay-Text** — `http_api_overlay_set` weist zu lange Texte mit `http_check_strvar_len()` ab statt `utf8_copy_truncated` | `esp-developer` | F.5d | Abnahmesatz Overlay-Text | ☐ | ☐ |
| F.5g | **C38 / L323** — SSID und WLAN-Schlüssel **abweisen statt still kürzen**: `http_api_network_client_set()` (`http.cpp:8664-8670`) und `http_api_eeprom_settings_set()` (`:8779-8785`), über `http_check_strvar_len()` oder dieselbe Byte-Regel — keine zweite | `esp-developer` | F.5e | **AKF.6.** **Abnahme nur am Quelltext und am Prüfstand** — `network_client_set` bleibt für Gerätetests **gesperrt** | ☐ | ☐ |
| F.5i | **[K] C23** — `/api/stm32_log` liefert **UTF-8 mit JSON-Escapen** statt ISO-8859-1 (Entscheidung vom 07.10.2026) | `esp-developer` | F.5g | Abnahmesatz C23 | ☐ | ☐ |
| F.5j | **[K] C6u** — Legacy-Overlay-Formular: `type`, `date_code`, `days` nicht mehr per blankem `atoi` (Entscheidung vom 07.10.2026). **Gleiche Funktion wie N1** (`http_overlays()`), deshalb erst nach S | `esp-developer` | F.5i | Abnahmesatz C6u | ☐ | ☐ |
| F.5f | **Werkzeugteil zu C25 und C23:** was die Verlustzeile und den Logring liest (`watch-log.sh`, `check-pwa.mjs` u. a.), auf die neuen Formate | Lead | F.5c, F.5i | Die Logwache meldet beide Zähler; ein Logring mit Umlauten wird ohne Ersatzzeichen gelesen; Gegenprobe je mit eingespielter Zeile (DIR-014) | ☐ | — |
| F.5h | **Prüfstand C38** ablegen (vom `esp-developer` im Scratchpad gebaut): zu lange SSID bzw. zu langer Schlüssel ⇒ Abweisung, Grenzwert ⇒ angenommen | Lead | F.5g | Einmal fehlgeschlagen gegen die alte Fassung (DIR-014), Fallzahl gemeldet | ☐ | — |
| F.6 | Review | `code-reviewer` | F.5j, F.5h | Vier Punkte; besonders: **Bleibt der Rückweg offen?** **E12:** Sendet die PWA beide Setter **immer** mit Parameter? **C27:** Braucht die PWA eine Anpassung? **C38:** Kann die PWA mehr Byte senden, als der ESP annimmt? **F.3:** Wie stellt die PWA eine leere Liste dar? **C23:** Liest die PWA den Logring weiterhin, und ist das Escapen vollständig (Steuerzeichen, Anführungszeichen, Backslash)? **C6u:** Gleiche Abweisungsform wie die API (C18), und passt sie zum Indexguard aus N1? Jede nötige PWA-Anpassung wird ein Task **nach** dem F-OTA (L241) | — | ☐ |
| F.7 | ESP-Version anheben | `release-engineer` | F.6 | S4 zeigt den neuen Stand | ☐ | — |
| F.8 | Build, Release-ZIP, Rollout, **Commit + Tag + Push** | Lead | F.7, F.5f, C4.3 | `guardrails.sh --full` Exit 0 | ☐ | — |
| F.9 | **Nutzer-Sitzung F1:** ESP einspielen | **Nutzer** | F.8 | neue ESP-Version in `/api/update_status` | — | — |
| F.10 | Abnahme am Gerät | Lead | F.9, F.4 | Smoketest **mit der neuen Stufe**, grün; Legacy-Seite **lesend**: Liste zeigt nur passende Dateien; `fs_show` auf eine vorhandene Datei liefert sie unverändert; Release Notes laden (C9k); `/api/stm32_log` ist gültiges JSON in UTF-8 (C23); WLAN unverändert verbunden; Legacy-Overlayseite lädt (C6u); `install-app.sh --check`; Update-Quelle (DIR-009); `watch-log.sh` mitgelesen | — | ☐ |
| F.11 | **Nutzer-Sitzung F2:** `./tools/flash-stm.sh --check`, `./tools/flash-stm.sh` (derselbe STM-Stand wie nach S), **dann M2** (entfällt als Nutzerschritt, sobald die Passwortdatei aus B19 angelegt ist) | **Nutzer** | F.10 | **AKF.5:** erwartete STM-Version; M2 liegt vor | — | — |
| F.12 | **Testdurchlauf Phasen 0–4 und 9** — gegen einen Stand mit P3 | `pwa-tester` | F.11 | **AKZ.4.** Bericht nennt die enthaltenen Punkte. **Kein** schreibender Aufruf auf `network_client_set` | — | — |
| F.13 | Abschlussbilanz in `BEFUNDE.md`: C9c6, L179, **L272 (eigene Zeile, durch F.1 geschlossen)**, C38/L323, Ent-4, Ent-5, C23, C6u, K-Punkte mit Träger F und W, **C4** | `doc-writer` | F.12 | S10 läuft durch; jeder im Paket berührte Befund trägt Status und Beleg | ☐ | ☐ |

**Einspielreihenfolge:** nur ESP (F.9), danach der STM-Flash als Probe (F.11). **Braucht eine
F-Änderung eine PWA-Anpassung (F.6), kommt sie danach** — Firmware zuerst (L241).

---

## Schritt C4 — Umschrift der STM-Quellen, ohne Flash

**Entscheidung vom 07.10.2026:** Die acht ISO-8859-1-Dateien unter `src/**` werden auf
**ASCII-Umschrift** gebracht, als eigener Schritt mit Nachweis über **gleiche `.hex`**,
**nicht** mit S. Die Nicht-ASCII-Zeichen stehen laut C4 ausschliesslich in Kommentaren —
ändert sich das Fabrikat, war diese Annahme falsch.

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| C4.1 | Die acht Dateien aus L166 (`display.c`, `base.c`, `base.h`, `irmp.c`, `rtc.c`, `ds18xx.c`, `tempsensor.c`, `w25qxx.c`) auf ASCII-Umschrift — **byteweise**, ohne Zeilenenden oder sonst etwas anzufassen | `stm-developer` | S.23 | **AKC4.1:** `grep -aP '[^\x00-\x7F]' src/**` findet nichts mehr; `git diff --stat` zeigt nur die acht Dateien; die Zeilenenden jeder Datei sind unverändert (DIR-015) | ☐ | ☐ |
| C4.2 | **Gleichheitsnachweis:** F103 **und** F411 vor und nach C4.1 bauen, `.hex` byteweise vergleichen | Lead | C4.1 | **AKC4.2:** Beide `.hex` **identisch**. **Weicht eines ab: Halt**, C4.1 zurück, Bericht — dann stand Nicht-ASCII doch ausserhalb von Kommentaren | ☐ | — |
| C4.3 | Commit (**kein** Release, **keine** Versionserhöhung, **kein** Flash — das Fabrikat ist dasselbe) | Lead | C4.2 | Commit-Botschaft nennt den `.hex`-Vergleich | — | — |

**Parallelität:** C4 schreibt nur `src/**` und darf **neben der F-Entwicklung** laufen (F
schreibt nur ESP-Dateien). Die Bauten in C4.2 sind reine STM-Ziele, kein `release-zip`.
**C4.3 muss vor F.8 abgeschlossen sein** — der F-Release-Build baut auch den STM aus dem Baum.

---

## Runde K — Kleinkram, verteilt auf die Flashes der anderen Runden

Inhalt aus der Nachzählung vom 06.10.2026, dem Testdurchlauf desselben Tages und den
Entscheidungen vom 07.10.2026. **K heisst:** höchstens rund zehn Zeilen und keine
Entwurfsfrage. Gerüst und Regeln in `design.md` §6.

### Rahmen-Tasks

| # | Task | Agent | Hängt ab von | Abnahme | G | R |
|---|---|---|---|---|---|---|
| K.0 | **Abzählung:** K-Punkte aus dem Bericht der Prüfer gegen die Tabellen unten | Lead | — | **AKK.2.** Zahl im Bericht = Zeilen unten (eingeplant, erledigt, geschlossen, wartend; Mehrfachkennungen einzeln gezählt). **Offen:** Der Prüfer nennt 13 STM-K-Punkte, die Liste enthält jetzt 11 Kennungen (mit A9; A5 ist kein K-Punkt mehr) — abgleichen. Abweichung ⇒ zurück an den `spec-writer` | — | — |
| K.W | **[K] Werkzeug:** E20, E11, E14-Rest, E24-Teil DIR-002, B20. **B19 und E2 sind erledigt** | Lead | — | je Zeile ihr Abnahmesatz. **E11 und F.4 schreiben dieselbe Datei** (`smoke-device.sh`) — nacheinander | ☐ | — |

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
| **A9** | Gesendeter LDR-Rohwert ist geklammert (L77) — **ungeklammert senden** (07.10.2026) | `src/ldr/ldr.c` | S-STM | S.19i | `stm-developer` | … der über `LDR_RAW_VALUE_NUM_VAR` gesendete Wert der ungeklammerte Rohwert ist; die **Automatik** rechnet unverändert mit ihrem eigenen, kalibrierten Wert (Quelltext belegt, dass nur der gesendete Wert sich ändert) | |
| **A16** | Tetris/Snake ohne `watchdog_reload()` — vom Nutzer bestätigt | `src/tetris/` | S-STM (Kern) | S.17, S7 in S.20 | `stm-developer`, Lead | … AKS.9 erfüllt ist **und** S7 die zwei neuen Stellen im Bestand führt | (im Kern) |
| **E17** | `pending_weather_ticker_restore` umbenennen — **mit S** (07.10.2026) | `src/**`, `CLAUDE.md`, `knowledge/` | S-STM (Kern, 0 Byte) | S.15b, S.15c, S.15d | `stm-developer`, Lead, `doc-writer` | … AKS.16 erfüllt ist und kein Dokument und kein Werkzeug den alten Namen mehr nennt | 0 erwartet |
| **C27 + N2** | `fs_show` prüft die Dateigrösse nicht | `http.cpp`, `http_api_fs_show()` | F | F.5a | `esp-developer` | … eine Datei mit 0 Byte **nicht** mehr als `200 OK` mit leerem Rumpf ausgeliefert, sondern als eigener benannter Zustand gemeldet wird; N2 nach seiner Befundzeile | — |
| **E12** | Zwei Flag-Setter löschen bei fehlendem Parameter | `http.cpp` | F | F.5b | `esp-developer` | … `tft_flags_set` und `dfplayer_bell_flags_set` einen fehlenden Parameter mit Kennung 1 abweisen, statt zu löschen, und F.6 bestätigt, dass die PWA beide immer mit Parameter sendet | — |
| **C25** | Verlustzähler bucht Browser-Abbau und Schreibversagen zusammen | `http.cpp`, `http_flush()` | F | F.5c, F.5f | `esp-developer`, Lead | … `http_flush()` zwei Zähler führt, beide in der Verlustzeile im Logring stehen **und** `watch-log.sh` beide liest | — |
| **C9k** | `httpclient_read_header()` wartet gar nicht | `httpclient.cpp` | F | F.5d | `esp-developer` | … `httpclient_read_header()` über den Zeitgrenzen-Helfer aus L152 liest; am Gerät laden die Release Notes weiter (F.10) | — |
| **Overlay-Text, ESP** | `http_api_overlay_set` kürzt still | `http.cpp` | F | F.5e | `esp-developer` | … ein Overlay-Text über 32 Byte mit `{"ok":false,"error":2,…}` abgewiesen wird und der gespeicherte Text unverändert bleibt | — |
| **C23** | `/api/stm32_log` liefert ISO-8859-1 in JSON — **UTF-8 plus Escapen** (07.10.2026) | `http.cpp` | F | F.5i, F.5f | `esp-developer`, Lead | … ein Logeintrag mit `ä`, Anführungszeichen, Backslash und Steuerzeichen als gültiges JSON in UTF-8 ankommt (Prüfstand mit genau diesen vier Fällen) **und** `watch-log.sh`/`check-pwa.mjs` ihn ohne Ersatzzeichen lesen | — |
| **C6u** | Legacy-Overlay-Formular: `type`, `date_code`, `days` mit blankem `atoi` — **ja, in F** (07.10.2026) | `http.cpp`, `http_overlays()` | F | F.5j | `esp-developer` | … die drei Felder ausserhalb ihres Bereichs mit derselben Abweisungsform wie die API (C18) abgewiesen werden und nichts geschrieben wird | — |
| **Overlay-Text, PWA** | fehlt in `TEXT_FIELD_LIMITS` | `app.js` | **P2** | P2.1 | `pwa-developer` | … die Vorschauproben aus P2.1 bestehen und `TEXT_FIELD_LIMITS` zehn Werte führt | — |
| **B26** | `formatLittleFs()` und zwei Kennungen | `app.js` | **P — erledigt** | — | — | **Erledigt mit PWA 1.4.91**, mit Gegenprobe. `files-panel` und `local-update-panel` **bleiben** | — |
| **B19** | Phase 0 ohne Nutzer | `tools/snapshot-device.sh` | W — **erledigt** | — | Lead | **Erledigt mit `e121058`** (Meldung des Leads): `snapshot-device.sh` liest `SNAPSHOT_PASS`, dann `~/.config/wordclock/snapshot.pass` (`SNAPSHOT_PASS_FILE`), dann das Terminal; Abbruch bei einer Datei im Repo, bei Modus ≠ `x00` und bei einer leeren Datei; das Passwort läuft über `env:`/`file:` statt über die Prozessliste. **Offen bleibt nur die Datei selbst — die legt der Nutzer an**; bis dahin bleibt M2 ein Nutzerschritt. Die alte Sicherung bleibt | — |
| **E2** | Vier tote Bundle-Dateien | `ESP8266/ESP-uclock/` | W — **erledigt** | — | Lead | **Erledigt mit `8dfba36`** (Meldung des Leads): Die Dateien lagen **nicht** unter `data/app/`, sondern unter `ESP8266/ESP-uclock/` — `APP-BUNDLE.md`, `data/app-bundle.txt`, `tools/*.py\|sh` — und gehörten damit in das Revier des Leads bzw. des `doc-writer` | — |
| **E20** | Besitz-Hook auf Schreibpositionen | `tools/hooks/file-ownership.py` | W | K.W | Lead | … zwei Gegenproben bestehen: fremder Pfad nur im Meldungstext ⇒ läuft durch; Schreiben in eine fremde Datei ⇒ abgewiesen (DIR-014) | — |
| **E11** | Smoketest: Versionen gegen den Repo-Sollstand, leerer Logring als eigener Zustand | `tools/smoke-device.sh` | W | K.W | Lead | … beide Fälle einmal mit festem Testwert ausgelöst und gemeldet | — |
| **E14-Rest** | Override-Option in `vermessen.mjs` für das DFPlayer-Modul | `tools/ui-mess/` | W | K.W | Lead | … ein Vorschaulauf mit der Option die drei `.chip-toggle` des DFPlayer-Moduls misst | — |
| **E24, Teil DIR-002** | Kurzregel nennt ihre Kennung nicht | `CLAUDE.md` | W | K.W | Lead | … die Kurzregel in `CLAUDE.md` „DIR-002" nennt und die Direktiven-Stufe durchläuft | — |
| **B20** | Kein `git add -A`, solange Agenten laufen | Lead-Verfahren | W | K.W | Lead | … die Regel am Commit-Schritt des Skills `/release` steht, mit Verweis auf L158 | — |

Schon in den Kernrunden geführt und deshalb **nicht doppelt** gezählt: **C9c6, C28**
(F.1), **B33** (ausgeliefert mit P). **B40/L321**, die **SSID-Auswahl**, **Massnahme 4**,
**P2-Review N1/N2**, **L303/L306**, **C38/L323**, **A5** und der **L-Befund Nachkommabit**
sind keine K-Punkte aus der Nachzählung — sie stehen als eigene Tasks in P2, S, P3 bzw. F.

### K-Tabelle — entschieden, ohne Umsetzung

| Kennung | Entscheidung vom 07.10.2026 | Vermerk durch |
|---|---|---|
| **C2** | Request-Zeile **bleibt bewusst unbedingt** — geschlossen | `doc-writer`, P2.9 |
| **C9c4** | Heap-Zeile **bleibt** — geschlossen | `doc-writer`, P2.9 |
| **C4** | Umschrift ASCII — **eigener Schritt C4**, nicht mit S | Schritt C4 |
| **A5** | **Echte Minusgrade in PWA und Legacy**, Uhranzeige unverändert (Ent-8) — kein K-Punkt mehr, sondern Protokolländerung: S.4b, S.18b, P3 | `design.md` §7 |
| **B17** | Spuren 1 und 2 im Code abgedeckt; Spur 3 wird in S.11 beobachtet; P2.2/P2.3 entfallen | `doc-writer`, P2.9 und S.27 |

### K-Tabelle — wartet auf den Nutzer

| Kennung | Offene Frage | Hinweis |
|---|---|---|
| **E3** | Kondensatorwert `C116` in KiCad | Der Nutzer sieht nach; ausserhalb dieses Repos |
| **B19, Rest** | Die Passwortdatei `~/.config/wordclock/snapshot.pass` anlegen | Handlung des Nutzers; danach fährt der Agent Phase 0 selbst |

---

## Was gleichzeitig laufen darf — und was nicht

**Erlaubt:**

- P2.1, P2.1b, P2.1d, P2.1f und P2.1h nacheinander (ein `pwa-developer`, dieselbe Datei);
  P2.1e (Lead) gleichzeitig dazu — P2.1e arbeitet mit einem zurückgebauten Stand im
  Scratchpad, nicht im Arbeitsbaum.
- S.7 und S.12 (lesend) jederzeit, auch während P2.
- P.18 und P.19 parallel zu P2 und zu S.1 bis S.5 — sie schreiben keinen Produktcode.
- S.15c (Lead) und S.15d (`doc-writer`) parallel zueinander und zu S.16 ff. — andere Dateien;
  **commitet werden sie mit S.23**.
- **C4.1/C4.2 parallel zur F-Entwicklung** — C4 schreibt `src/**`, F nur ESP-Dateien.
- F.4 und K.W jederzeit; E11 und F.4 nacheinander (gleiche Datei).

**Nicht erlaubt:**

- **Ein Release-Build, während irgendein Produktcode-Task offen ist** (§4.1). Das betrifft
  P2.6, S.9, S.23, P3.4, F.8. **Die Testbauten in S und die Gleichheitsbauten in C4.2 sind
  keine Release-Builds** — reine STM-Ziele, zwischen abgeschlossenen Tasks, ohne ZIP.
- **S.1 vor P2.6**, **P3.1 vor S.23**, **F.1 vor P3.4**, **F.8 vor C4.3** — baulich.
- **S.14 vor S.11** — die Zwischenabnahme muss den reinen ESP-Stand sehen.
- **S.15b zusammen mit einer anderen Änderung** — die Umbenennung ist ein eigener Diff.
- **Zwei Schreiber in derselben Datei.** `app.js` nur `pwa-developer`; `src/**` in S.14 bis
  S.19i nacheinander, C4.1 erst danach; `http.cpp` in S.4b, S.5 und in F.1 bis F.5j
  nacheinander.
- **Ein zurückgebauter Prüfstand im Arbeitsbaum** (L268).
- **Kein Teammate führt `make` aus** (R1); Versionen nur über den `release-engineer` (R4).
- **Hardware ist exklusiv** (R5). **`network_client_set` bleibt in jedem Gerätetest dieses
  Pakets gesperrt.**

---

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um.
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings.
3. Review gegen `knowledge/architecture-checklist.md`.
4. Erst dann nächster Task oder Schreibübergabe.

**Vor jedem Patch an einer Firmware-Quelle: Kodierung *und* Zeilenende feststellen**
(DIR-015). **Bei jeder Suche in `src/**`: `grep -a`** (B21) — bis C4 erledigt ist. **In
`app.js` keine Zeilennummern übernehmen** — die Datei bewegt sich.

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
      (P2 und P3 ausdrücklich ohne eigenen, siehe dort)
- [ ] Jeder K-Punkt: erledigt mit Beleg, zurückgestellt mit gemessener Zahl, umgestuft mit
      Grund, geschlossen mit Entscheidung, oder wartet ausdrücklich
