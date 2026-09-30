# Befundstand

**Lebendes Dokument.** Anders als `REVIEW.md`, `REVIEW-2026-09-29.md` und
`gap-analysis.md` — das sind datierte Momentaufnahmen, die nicht fortgeschrieben
werden (DIR-006) — führt diese Datei den **aktuellen Stand** der dort erhobenen
Massnahmen.

Der Status ist am Code nachgeprüft, nicht aus dem Changelog abgeschrieben. Die
Spalte „Beleg" nennt, woran die Prüfung hängt, damit sie wiederholbar ist.

## Warum es zwei Review-Dokumente gibt

Keine Doppelung, sondern zwei Durchgänge mit verschiedenen Achsen.

| | `REVIEW.md` | `REVIEW-2026-09-29.md` |
|---|---|---|
| Stand | 2026-08-12, Commit `1d7d54e` | 2026-09-29, Commit `2546b84` |
| Achsen | PWA-Korrektheit · PWA↔STM-Display · UI/UX | PWA↔ESP-API-Vertrag · STM-Restmodule · UI über die Gerätespanne · Kodierung |
| Methodik | drei parallele Agenten | vier parallele Agenten |
| Massnahmen | 18 | 16 |

Review 2 wiederholt Review 1 **ausdrücklich nicht**, sondern deckt die Achsen ab,
die dort unter „Was nicht geprüft wurde" standen. Zwei Ergebnisse von Review 2
ändern die Bewertung von Review 1:

- **Der Service Worker registriert im realen Betrieb nie** (unsicherer Kontext über
  `http://192.168.x.x`). Damit sind fünf SW-Befunde aus Review 1 heute
  gegenstandslos — aber latent, sobald jemand HTTPS davorschaltet.
- **`do_display_icon` friert permanent ein**, wenn das Display während eines Icons
  ausgeschaltet wird. Das macht die Temperatur-Restore-Blockade aus Review 1
  permanent statt fünf Sekunden.

Die Namensgebung ist uneinheitlich: `REVIEW.md` trägt kein Datum im Namen, weil es
zuerst da war. Umbenannt wird es nicht — es ist an über zwanzig Stellen referenziert,
unter anderem aus `tools/guardrails.sh` und aus vier Agentendefinitionen.

---

## Review 1 — die 18 Massnahmen

| # | Massnahme | Status | Beleg |
|---|---|---|---|
| 1 | `watchdog_reload()` in die langen Busy-Waits | **offen** | `grep -c 'watchdog_reload ('` in `src/**` → weiterhin genau **eine** Aufrufstelle, `main.c:3170` |
| 2 | Restore-Lücke schliessen | **erledigt** 3.2.5 | `main.c:3699-3711`, Flag wird nur bei wirksamem Restore verbraucht · Spec `specs/restore-luecke/` |
| 3 | Globale Meldungsfläche ausserhalb der Sections | **offen** | genau ein `role="status"` in `index.html`, und der sitzt am Layout-Warnhinweis |
| 4 | `hasUnsavedEdits` nach dem Speichern zurücksetzen | **offen** | `hasUnsavedEdits = false` nur an `app.js:4092` (Backup-Import) und `:10675` (Overlay verwerfen) |
| 5 | Die beiden `ReferenceError` beheben | **erledigt** 1.4.70 | `normalizeUrlPath` definiert · `loadDebugOverrides()` null Fehlaufrufe |
| 6 | HTTP-Debugzeilen für `/api/` unterdrücken | **offen, aber entschärft** | `http.cpp:11196`, `:11251`, `:11305` unverändert. **Am Gerät gemessen (30.09.2026):** 10 Requests erzeugen exakt 20 Debugzeilen auf der STM-UART, rund **106 Byte je Request** — die Schätzung „~100 Byte" aus Review 1 stimmt. Die **Burst-These ist dagegen nicht bestätigt**: 5 Runden zu je 3 parallelen Requests ergaben 30 von 30 erwarteten Zeilen, **kein Verlust**. Der 256-Byte-Ring wird laufend geleert, die 300 Byte treffen nicht in einem Fenster ein. Damit sinkt die Dringlichkeit deutlich — der Hebel bleibt richtig, ist aber kein akuter Fehler |
| 7 | `file.size > 0` bei App-Install, Längenprüfung im SW | **offen** | siehe Review 2, Massnahme 2 — derselbe Befund ESP-seitig |
| 8 | Drei fehlende i18n-Keys | **erledigt** 1.4.70 | `common.uploaded`, `common.setting`, `common.set` in beiden Tabellen |
| 9 | `color-scheme`, Fokus-Stil, `prefers-reduced-motion`, `dvh`, Safe-Area | **offen** | `color-scheme` → **0 Treffer** in `styles.css` und `index.html` |
| 10 | `.button.primary` und `:disabled` reparieren | **offen** | `styles.css:162` und `:176` weiterhin identisch |
| 11 | Nicht-Text-Kontrast auf ≥ 3:1 | **offen** | — |
| 12 | Fünf Grid-Area-Klassen im HTML nachziehen | **offen** | S6 der Guardrails meldet sie |
| 13 | Unbedingte `log_printf` im Refresh-Pfad | **zurückgestellt** | S7 meldet sie. Gegenprobe F7 — würde die `icon_freeze`-Messung verfälschen, deshalb erst danach |
| 14 | Modal auf Dialog-Standard, Fokus, Escape | **offen** | — |
| 15 | PNG-Icons, `apple-touch-icon`, Manifest | **offen** | `icons/` enthält weiterhin ausschliesslich SVG |
| 16 | DS18xx: CRC-Prüfung, Flag „Messwert gültig" | **offen** | offenes Thema 1 in `CLAUDE.md`. UI-Seite bereits korrigiert |
| 17 | Formularvalidierung statt stillem Clamping | **offen** | — |
| 18 | ~176 hartcodierte Strings in die i18n-Tabelle | **teilweise** | DE 779 / EN 779 Schlüssel — Parität hergestellt, die hartcodierten Strings bleiben |

Zusätzlich erledigt, in Review 1 unter „Hoch" statt in der Massnahmenliste:

- **Release-Notes ungefiltert per `innerHTML`** (`app.js:5667`) → Sanitizer mit
  DOMParser-Whitelist, baut aus `createElement` neu auf, setzt nur `href`/`target`/`rel`
- **Drei leere `catch`-Blöcke** → mit neuem Schlüssel `backup.overlay_clear_failed`

---

## Review 2 — die 16 Massnahmen

| # | Massnahme | Status | Beleg |
|---|---|---|---|
| 1 | Null-Prüfung in `normalize_http_parameters` | **offen** | ESP-Absturz per `GET /?a`, aus dem ganzen LAN auslösbar |
| 2 | `http_fs_file_exists_and_nonempty` auf dem Auslieferungspfad | **offen** | Helfer existiert (`http.cpp:1090`), benutzt nur an `:1129`/`:1130`. Auslieferpfad `:1418`, `:1432`, `:1047` prüft weiter nur `exists()` |
| 3 | Backup-Export/-Import absichern | **offen** | kann das Gerät ohne WLAN, ohne AP und ohne Webserver zurücklassen |
| 4 | `apiFetch` statt rohem `fetch` im Flash-Pfad | **offen** | `app.js:9148` |
| 5 | `finishProgressUi(2200)` statt `(0)` | **offen** | `app.js:9126`, fehlgeschlagener Flash nur einen Frame sichtbar |
| 6 | `display_icon`-Freeze **messen**, dann entscheiden | **in Messung** 3.2.6 | Instrumentierung in `main.c:3171` eingebaut. **Ergebnis steht aus — Gerätetest AK7** |
| 7 | Safe-Area links/rechts, `dvh`, Kartenmodal-Höhe | **offen** | — |
| 8 | Globale Meldungsfläche | **offen** | identisch mit Review 1, Massnahme 3 |
| 9 | `target="_blank"` am Legacy-Link | **offen** | `index.html`, `href="/legacy"` ohne `target` |
| 10 | Hinweis bei `isSecureContext === false` | **offen** | Kernbefund 1 |
| 11 | Poller bei `document.hidden` stoppen | **offen** | `app.js:2170`, `visibilitychange` ohne `else` |
| 12 | `watchdog_reload()` in Busy-Waits, inkl. `remote_ir_learn()` | **zurückgestellt** | identisch mit Review 1, Massnahme 1. Ändert Verhalten auf laufender Firmware, braucht eigene Spec mit Geräteverifikation |
| 13 | Bereichsprüfungen `overlay_set_n_overlays`, `display_set_animation_flags` | **offen** | über PWA und Legacy nicht erreichbar, über verstümmeltes UART-Kommando schon |
| 14 | Destruktive Endpunkte auf POST plus Herkunftsprüfung | **offen** | `maintenance_format_fs` und `maintenance_reset_eeprom` als einfacher GET |
| 15 | Breakpoint-Lücke 561–899 px schliessen | **offen** | betrifft Phone-Querformat und Tablet-Hochformat |
| 16 | Acht C-Dateien auf UTF-8 | **offen, vorbereitet** | S7b meldet sie. 62 Zeilen mit Nicht-ASCII, **alle in Kommentaren, null in String-Literalen**. Verifikationsweg: Prüfsummenvergleich der `.hex` vor/nach |

---

## Offene Gerätetests

Nur am Gerät zu beantworten, nicht am Code.

| | Was | Ergebnis entscheidet |
|---|---|---|
| **AK7** | Icon-Overlay starten, Display ausschalten, STM32-Logbuch prüfen | `icon_freeze: ENTER do_icon=1 power=0` ⇒ Freeze belegt. Zeile bleibt aus ⇒ widerlegt |
| **AK6** | Wetter-Ticker starten, währenddessen Dimmkurve speichern | prüft die Restore-Lücke aus Review 1 gegen den 3.2.5-Fix |
| **M2** | Reset-Ursache auslesen (`log_reset_flags()`, schon in der UI) | `Watchdog reset` ⇒ blockierender Pfad · `Software reset` mit „fatal fault detected" ⇒ Rekursionsthese · kein Flag ⇒ Versorgung oder Mechanik |
| **M3** | `sk6812_refresh: waiting` in Hänger-Mitschnitten suchen | vorhanden ⇒ DMA-Stillstand belegt · fehlt in **allen** ⇒ widerlegt, und die Entscheidung gegen einen pauschalen DMA-Fix ist belegt statt vermutet |

---

## Was aus den Reviews inzwischen als überholt gilt

- **Review 1, F2 und F7** empfehlen `debug_log_printf`. Das ist wirkungslos:
  `debug_log_printf` hängt an `#ifdef DEBUG`, das nirgends gesetzt wird — **alle 141
  `debug_log_*`-Aufrufe in `main.c` übersetzen zu nichts**. Am ELF geprüft:
  `update display` 0×, `sk6812_refresh` 5×. Wer dort instrumentieren will, braucht
  entweder `log_printf` mit Zustandswechsel-Filter (so gelöst bei `icon_freeze`) oder
  einen Zähler.
- **Review 1 zum App-Bundle** („`build-app-bundle.py` bricht ab"). Der Weg ist nicht
  defekt, sondern **tot**: der ESP meldet alle drei Bundle-Fähigkeiten als `0`, sendet
  die drei URLs als leeren String, hat keinen Handler, und auf dem Update-Server liegt
  keine `app-bundle.txt`. Belege in `ESP8266/ESP-uclock/APP-BUNDLE.md`.

---

## Befunde aus der laufenden Arbeit

Nicht aus den beiden Reviews, sondern bei Umbau, Rollout und Prüfung aufgefallen.
Gleiche Verbindlichkeit wie die Massnahmen oben.

| # | Befund | Status | Beleg / Entscheidung |
|---|---|---|---|
| L1 | **App-Bundle-Weg ist tot**, nicht defekt | **Entscheidung offen** | ESP meldet alle drei Fähigkeiten als `0` (`http.cpp:9532`, `:9540`, `:9543`), sendet die URLs als leeren String (`:9622`, `:9625`, `:9635`), kein Handler, kein Legacy-Upload, auf dem Server keine `app-bundle.txt`. Löschkandidaten: `tools/build-app-bundle.py`, `tools/release-app-bundle.sh`, `data/app-bundle.txt`, `APP-BUNDLE.md`. **Löschen ist Sache des Nutzers** |
| L2 | **Release-ZIP kollidiert bei Minutengleichheit** | **offen** | `Makefile:18` nutzt `date +"%Y-%m-%d-%H%M"`, `Makefile:74` macht `rm -f` darauf. Zwei Release-Builds in derselben Minute erzeugen denselben Namen; der zweite überschreibt den ersten **kommentarlos, beide melden Erfolg**. Erklärt vermutlich frühere Beobachtungen „Zeitstempel im Release nicht aktuell". Fix: Sekunden in den Namen |
| L3 | **Alle `debug_log_*`-Aufrufe übersetzen zu nichts** | **offen** | 164 Aufrufe unter `src/**`, aber `DEBUG` wird in `src/**`, `CMakeLists.txt` und `cmake/**` **nirgends** definiert. Am ELF gegengeprüft. Entweder `DEBUG` baubar machen oder die toten Aufrufe entfernen — solange beides nicht passiert, ist jede Empfehlung „nimm `debug_log_printf`" wirkungslos |
| L4 | **ESP 3.2.2 wurde ohne Codeänderung angehoben** | **Altlast** | entstanden unter der vorherigen Gleichschritt-Regel. Folge: Die Uhr bietet ein OTA-Update auf identischen ESP-Code an. Harmlos, aber ein Flash ohne Gegenwert. Mit der neuen DIR-004 kann das nicht wieder entstehen |
| L5 | **`README-CMAKE.md` enthält Changelog-Inhalt** | **offen** | Die Abschnitte zu den Ständen vom April sind Release-Historie und gehören in `CHANGELOG.md`. Solange sie dort stehen, brauchen sie `<!-- historisch -->`-Marker, damit S9 sie durchlässt |
| L6 | **Mock-Server der Vorschau meldet Firmware 3.2.5** | **bewusst** | `tools/preview/server.py:28`, `:72`. Der Mock bildet einen festen Gerätestand ab und folgt den Quellen absichtlich nicht. Deshalb ist er von S9 ausgenommen |
| L7 | **EEPROM schreibt Byte für Byte, obwohl der Baustein Seiten kann** | **offen** | `eeprom.c:200` schreibt in einer Schleife je ein Byte und wartet danach 15 ms. Der Kommentar dort — „we must write every single byte, because we have to wait 15ms every cycle" — dreht die Begründung um: Man wartet 15 ms **nach** einem Schreibzyklus, und ein Zyklus fasst beim **AT24C32M** eine ganze **32-Byte-Seite**. `i2c_write` kann das bereits: `i2c.c:457` sendet `cnt` Bytes in **einer** Transaktion, der Lesepfad nutzt das auch. Gerechnet für die Dimmkurve (16 Byte, `MAX_BRIGHTNESS 15` + 1, ein Aufruf in `display.c:3907`): **240 ms statt 15 ms** — Faktor 16, bei ungünstiger Seitengrenze Faktor 8. Da die PWA 16 solche Kommandos sendet, erklärt das REVIEW.md Kernbefund 3 (rund 4,3 s Hauptloop-Stillstand) **an der Wurzel** und macht ihn behebbar, ohne die PWA zu ändern. **Heikel:** Ein Umbau des EEPROM-Schreibpfads riskiert Datenverlust und muss Seitengrenzen korrekt splitten — eigene Spec plus Geräteverifikation |
| L9 | **Stromreserve über USB-C ist knapp** | **offen, Hypothese** | `F1` (SMD1206-150-16) hat **1,5 A Haltestrom** im USB-C-Zweig, `F2` 3,0 A im externen. 114 SKC6812RGBW ziehen im Ruhezustand belegte 0,033 A; bei 40 hellen LEDs über R+G+B kommen nach der üblichen Annahme von 20 mA je Kanal rund 2,4 A zusammen. Eine Rückstellsicherung schaltet nicht ab, sondern erhöht bei Erwärmung allmählich den Widerstand — die Spannung sackt. Das passt zum Bild „läuft meistens, hängt gelegentlich" aus dem offenen Thema 2. **Rechnung, keine Messung, und der Strom je Kanal ist nicht aus dem Datenblatt belegt.** Prüfbar ohne Codeänderung: Spannung an `VCC_5V` (Testpunkt `TP7`) unter heller Anzeige messen, oder beobachten, ob Hänger bei dunkler Anzeige seltener sind |
| L10 | **Zwei Unstimmigkeiten in der Stückliste des LED-Boards** | **offen** | `C116` steht in beiden `bom.csv` mit **680 µF**, im PCB mit **100 µF**. Die Stücklisten stammen vom 12. März, das PCB wurde am 12. April zuletzt geändert — vermutlich wurde der Wert danach angepasst und die Stückliste nicht neu erzeugt. Für den Stützkondensator der LED-Versorgung ist das nicht egal. Zweitens trägt `R4` (220 Ω) in `production/bom.csv` die LCSC-Nummer `C17477`, und das ist die der 0-Ω-Widerstände; die JLCPCB-Stückliste hat mit `C17557` die richtige |
| L11 | **Kein Ambilight-Ausgang am LED-Board** | **offen, Einschränkung** | `V114.DOUT` ist nicht herausgeführt, die Kette endet auf der Platine. Der zweite Stecker `H4` am Controller liegt **parallel** zu `H7` auf demselben Datensignal — was dort hängt, bekäme die Daten ab LED 1, also die Uhrzeit, nicht die Ambilight-Daten ab Position 114. Die Firmware reserviert Platz (`DSP_AMBILIGHT_LEDS 120`), die tatsächliche Zahl steht im EEPROM. Kein Fehler, solange Ambilight auf 0 steht — aber die PWA bietet die Einstellung an |
| L12 | **Die Logmenge im Ruhezustand ist weit kleiner als angenommen** | **erledigt, Annahme korrigiert** | Am Gerät gemessen: **37 Byte/s, 28 Zeilen/min, rund 3 MB/Tag**. Meine Schätzung in der Logger-Anleitung lag bei mehreren hundert MB/Tag — **Faktor 225 daneben**. Sie stützte sich auf „Minuten-LEDs mit 64 Hz" aus `REVIEW.md`; tatsächlich erscheinen im Ruhezustand rund 0,1 `sk6812_refresh`-Paare je Sekunde. Die Firmware protokolliert nur bei Ereignissen. **Offen bleibt die Rate während Ticker und Animation** — dort könnte die 64-Hz-Angabe zutreffen, gemessen ist sie nicht |
| L13 | **Zerrissene Zeilen belegen die verschachtelte Kommandoausführung** | **belegt** | Dreimal beobachtet, alle bei dicht aufeinanderfolgenden Requests: `(- request 192.168.1.11- new client from 192.168.1.119)`. Eine Logausgabe wird mitten im Satz von einer zweiten unterbrochen — genau das geschieht, wenn während einer laufenden Ausgabe ein **verschachteltes** Kommando verarbeitet wird, das selbst protokolliert. Damit ist die Rekursion aus `var_send_buf()` (Review 1) **am Gerät belegt**. Offen bleibt, ob sie auch den Ausfall des periodischen Zweigs verursacht — siehe L14 |
| L14 | **Hänger vollständig mitgeschnitten — Blockade-These widerlegt** | **offen, neue Spur** | 30.09.2026, 23:18 bis 23:36, **18 Minuten**. Ab 23:18 kein `show_time`, kein `read rtc`, keine Temperatur, keine Refreshes; die Gerätezeit stand. **Aber:** Um 23:27 wurde mitten im Hänger ein Ticker-Kommando angenommen und vollständig ausgeführt (72 Refreshes über 5,8 s). Und die Startsequenz nach dem Reset meldet `Reset flags: PINRST` — **kein `IWDGRST`**. In 18 Minuten hat der Watchdog nie zugeschlagen, der Hauptloop lief also. ⇒ **Für diesen Hänger sind alle Blockade-Thesen widerlegt.** Ausgefallen ist ausschliesslich der zeitgesteuerte Zweig. Auslöser mit hoher Wahrscheinlichkeit `(CMD N030100)` um 23:17:46 — Display einschalten über die **Legacy**-Oberfläche, während die PWA pollte. **Am selben Abend reproduziert** (REPRO-3): volles PWA-Pollingmuster plus `/?action=poweron` ⇒ Hänger nach zwei Sekunden, erneut ohne Watchdog-Reset. Der Mitschnitt zeigt den Bruch **mitten in `set_display_power()`** — die Abschlusszeilen fehlen, und eine Logausgabe bricht mitten im Wort ab. Damit ist die verschachtelte Ausführung aus `var_send_buf()` als Ursache **stark gestützt**. Vollständige Auswertung in `haenger-2026-09-30.md` |
| L15 | **Der Watchdog ist auf dem Gerät gar nicht aktiv** | **offen, schwerwiegend** | Beim Start meldet die Firmware `IWDG init timeout, watchdog disabled` — bei **jedem** beobachteten Start. Ursache ist eine Reihenfolge in `watchdog_init()` (`main.c:493-529`): `IWDG_SetPrescaler()` und `IWDG_SetReload()` setzen die Flags `PVU`/`RVU`, die nur gelöscht werden, wenn der **LSI-Takt läuft**. Der startet aber erst mit `IWDG_Enable()` — und das steht **nach** der Warteschleife. Sie läuft in den Timeout, die Funktion kehrt mit `return` zurück, `IWDG_Enable()` wird nie erreicht. `RCC_LSICmd` kommt im gesamten `src/`-Baum **nicht vor**. **Tragweite:** Sämtliche „garantierter IWDG-Reset"-Aussagen aus `REVIEW.md` treffen auf dieses Gerät **nicht** zu. `display_test()` (45 s) und `remote_ir_learn()` (unbegrenzt) lösen keinen Reset aus — die Uhr hängt einfach. Der Schutzmechanismus, auf den sich die Firmware verlässt, fehlt vollständig. **Möglicher Erklärungsbeitrag zum F103/F411-Unterschied** — ob der LSI auf dem F103 aus anderem Grund läuft, ist nicht geprüft |
| L16 | **`var_send_busy` ist ein Rekursionsschutz, der nie prüft** | **offen** | Gesetzt in `vars.c:60`, zurückgesetzt in `main.c:2827` — und **an keiner Stelle abgefragt**. Die Absicht ist erkennbar, die Wirkung fehlt. Zusammen mit der Warteschleife in `vars.c:62`, die `schedule_esp8266_messages()` selbst aufruft, entsteht die verschachtelte Ausführung ohne jede Bremse |
| L8 | **`README.md` beschreibt die Hardware der Vorgängerbestückung** | **erledigt** | Die Notiz „3,3 V direkt vom STM32 kann an 5-V-versorgten SK6812 grenzwertig sein" gilt nicht für das V2-Board: dort sitzt `U3` (SN74AHCT1G125) als Pegelwandler mit TTL-Eingang. Korrigiert, Details in `HARDWARE.md` |

### In dieser Sitzung geschlossen

| Was | Wie abgesichert |
|---|---|
| DIR-004 von Gleichschritt auf komponentenweise Versionierung umgestellt | S4 prüft jetzt je Komponente **und beide Richtungen**; die Versionszeile wird aus dem Diff ihrer eigenen Datei gefiltert, sonst könnte die Gegenprobe nie anschlagen |
| ESP und PWA wurden in der Versionsprüfung als eine Einheit behandelt | getrennte Quellmengen, mit `:(exclude)` gegeneinander abgegrenzt. Am Einzelfall getestet: je Komponente schlägt genau eine an |
| `README-CMAKE.md` behauptete über Monate `STM 3.2.0 / ESP 3.2.0 / PWA 1.2.43` | **S9** vergleicht lebende Dokumente gegen die Quellen. Versionsnummern aus `CLAUDE.md` und dem README-Kopf entfernt statt gepflegt | <!-- historisch -->
| Sieben absolute Pfade `/Users/<name>/…` in `CHANGELOG.md`, `README-CMAKE.md` und `.claude/settings.json` | **S9** meldet sie in allen versionierten Dateien |
| 14 `ß` in vier lebenden Dokumenten gegen die Schweizer Schreibung | korrigiert; `REVIEW.md` als Momentaufnahme unberührt |
| R1 „nur der Lead baut" stand nur als Text in `CLAUDE.md` | PreToolUse-Hook `tools/hooks/no-build.py` im Frontmatter aller schreibenden Agenten ausser `release-engineer`. Deckt `make`, `cmake`, `arduino-cli` und `guardrails.sh --full` ab; harmlose Ziele wie `make stm-version-file` bleiben erlaubt |
| Nichts erzwang, dass die Guardrails vor Turn-Ende liefen | **Stop-Hook** `tools/hooks/guardrails-before-stop.py`. Blockiert das Turn-Ende, wenn überwachte Dateien geändert sind und der Lauf fehlt. Drei Absicherungen gegen Dauerblockaden: `stop_hook_active`, ein Hash pro Änderungsstand (nur einmal blockieren), und jeder Fehler im Hook lässt durch |
| `CLAUDE.md` war auf 248 Zeilen gewachsen und lud immer komplett | Abläufe in `.claude/skills/` ausgelagert: `/release`, `/pwa-vorschau`, `/doku-nachfuehren` laden bei Bedarf, `stm-firmware` automatisch über `paths: src/**`. Jetzt 178 Zeilen; die Kurzregeln bleiben, die Prozeduren wandern |
