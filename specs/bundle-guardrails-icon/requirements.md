# Anforderungen — Guardrail-Befunde schliessen und do_display_icon messbar machen

**Status:** Zur Freigabe · **Auslöser:** Nutzerauftrag vom 2026-09-29

## Problem

Zwei Themen, die als ein Release zusammengehören, weil beide dieselben Dateien
berühren und ein gemeinsamer Build sie ausliefert.

### Teil A — die Guardrails stehen auf Rot

Zwei Prüfungen melden Kritisch, vier melden Hoch. Alle Befunde sind seit Juli
ausgerollt und verifiziert:

| Befund | Stelle | Wirkung |
|---|---|---|
| `normalizeUrlPath()` nirgends definiert | `app.js:8415` | `ReferenceError` beim Layout-Tabellen-Upload, **ohne Fehlermeldung** — der Aufruf liegt vor dem `try`, das Promise wird verworfen, der Button wirkt tot |
| `loadDebugOverrides()` nirgends definiert | `app.js:11880` | `ReferenceError` alle 5 s bei laufender Farbanimation, die WordClock-Vorschau friert auf der alten Farbe ein |
| `common.uploaded`, `common.setting`, `common.set` fehlen | beide Tabellen | Auf dem Button steht wörtlich `common.setting` — in **beiden** Sprachen |
| 14 Schlüssel fehlen in `en` | i18n-Tabelle | Englische UI zeigt deutsche Fehlermeldungen |
| 3 leere `catch (_) {}` | `app.js:2416, 2479, 5131` | Fehler werden stillschweigend verschluckt |
| `view.releaseNotes` roh in `innerHTML` | `app.js:5667` | Inhalt vom konfigurierbaren Update-Host über HTTP, ungefiltert im PWA-Origin |

**Zusätzlich ein Fehler in der Prüfung selbst.** Die S7-Regel „innerHTML ohne
escapeHtml" ist zeilenbasiert und irrt in **beide** Richtungen: Sie meldet 22 Stellen,
von denen 21 `escapeHtml` korrekt im Rumpf der `.map()`-Funktion verwenden — und sie
übersieht `app.js:5667`, weil dort `escapeHtml` für den *Fallback* auf derselben Zeile
steht, während der eigentliche Wert roh eingesetzt wird. Eine Prüfung, die die einzige
echte Lücke verschweigt und 21 Fehlalarme erzeugt, ist schlechter als keine.

### Teil B — `do_display_icon` friert permanent ein, unbewiesen

`display_icon()` hat seinen gesamten Rumpf in `if (display.display_power_is_on)`
(`display.c:4882`), und das einzige `do_display_icon = 0` steht **innerhalb** dieses
Blocks (`display.c:5028`). Wird das Display ausgeschaltet, während ein Icon läuft,
bleibt das Flag dauerhaft gesetzt. Belegte Folgen, alle permanent solange aus:
Animationsflags eingefroren, Temperatur-Restore blockiert, Wetter-Ticker-Restore löst
nie aus, Helligkeitsautomatik tot, Icon-LEDs leuchten weiter obwohl „aus".

Der billigste Nachweis wäre die Suche nach `"display_icon: start"` ohne zugehöriges
`"display_icon: finished"` in einem Mitschnitt. **Es existiert kein Mitschnitt.**
Deshalb wird stattdessen gezielt instrumentiert.

## Ziel

Die Guardrails laufen ohne Kritisch-Findings durch, die verbleibenden Hoch-Findings
sind entweder behoben oder begründet dokumentiert. Der `do_display_icon`-Freeze ist am
Gerät nachweisbar oder widerlegbar, ohne dass ein Fix eingebaut wird.

## Akzeptanzkriterien

- [ ] **AK1** — `./tools/guardrails.sh` liefert **Exit 0**
- [ ] **AK2** — Beide `ReferenceError` sind weg, per Guardrail-Stufe S2 nachgewiesen
- [ ] **AK3** — Kein `translate()`-Aufruf mehr auf einen Schlüssel, der in keiner Tabelle steht; DE und EN haben gleich viele Schlüssel
- [ ] **AK4** — `view.releaseNotes` wird nicht mehr roh in `innerHTML` geschrieben
- [ ] **AK5** — Die S7-Prüfung auf `innerHTML` meldet `app.js:5667` **vor** der Behebung und ist danach still; die 21 korrekten `.map()`-Stellen meldet sie **nicht**
- [ ] **AK6** — `make f103`, `make f411` und `make esp` übersetzen fehlerfrei
- [ ] **AK7, am Gerät** — Icon-Overlay starten, während es läuft das Display ausschalten. Im STM32-Logbuch erscheint die neue Diagnosezeile mit `do_display_icon=1` bei `power=0`. Erscheint sie nicht, ist der Freeze **widerlegt**

## Nicht Teil dieser Änderung

- **Kein Fix für den `do_display_icon`-Freeze.** Nur Messung (DIR-003). Erst AK7 auswerten
- **Kein Umbau der `log_printf` in `sk6812.c`.** Das ist die Gegenprobe F7 aus `REVIEW.md`. Sie gleichzeitig mit der Instrumentierung zu ändern macht beide Messungen unbrauchbar, weil die Logmenge das Timing verändert, das den Freeze auslösen kann. **Unmittelbar danach, als eigener Schritt**
- **Kein `watchdog_reload()` in die blockierenden Pfade.** Das ist Massnahme 1 aus `REVIEW.md`, ändert das Verhalten auf laufender Firmware und braucht eine eigene Spezifikation mit Geräteverifikation
- Keine der übrigen Massnahmen aus `REVIEW.md` oder `REVIEW-2026-09-29.md`

## Betroffene Laufzeiten

- [x] **PWA** (`data/app/app.js`) — nur LittleFS-Upload
- [x] **STM32** (`src/display/display.c`) — **Neu-Flashen nötig**
- [ ] ESP8266 — nicht betroffen
- [x] Build und Rollout
