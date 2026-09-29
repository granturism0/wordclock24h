# Design — Guardrail-Befunde und do_display_icon-Messung

## Vorab: DEBUG ist nicht definiert

Am Binary bewiesen. `debug_log_*` hängt an `#ifdef DEBUG` (`src/log/log.h:23-31`), und
`DEBUG` wird weder im `CMakeLists.txt`, im Toolchain-File noch in einem Header gesetzt.
Gegenprobe im ELF: der String `update display`, der ausschliesslich über
`debug_log_message` erreichbar ist, kommt **0×** vor; `sk6812_refresh` über unbedingtes
`log_printf` kommt **5×** vor.

**Alle 141 `debug_log_*`-Aufrufe in `main.c` kompilieren zu nichts.**

Zwei Folgen für die Diagnosestrategie, die in `REVIEW.md` anders steht:

- **F2** („zwingend als `debug_log_printf`, sonst verschärft das Log die Blockade")
  hätte in einem Release-Build **null Ausgabe** erzeugt. Die Messung hätte schweigend
  nichts gemessen
- **F7** („`sk6812.c` auf `debug_log_printf` umstellen") macht das Logging nicht
  bedingt, sondern entfernt es ersatzlos. Als Gegenprobe ist das brauchbar, aber die
  Formulierung war falsch

Zusätzlich: `log_message()` schreibt **nur** auf die Log-UART, nicht an den ESP. Nur
`log_printf()` läuft über `log_vprintf` → `esp8266_send_log_line` (`log.c:27-34`) und
landet damit im STM32-Logbuch der PWA.

## Teil A — PWA

| # | Datei:Zeile | Änderung |
|---|---|---|
| A1 | `app.js:8415` | `normalizeUrlPath()` implementieren. Vorbild ist `app.js:7370`: `new URL(u, location.origin).pathname`, mit `try/catch` für unparsbare Werte |
| A2 | `app.js:11880` | `loadDebugOverrides()` → `getDebugOverrides()` |
| A3 | i18n, beide Tabellen | `common.uploaded`, `common.setting`, `common.set` ergänzen. Nachbarn existieren bereits: `common.uploading`, `common.applied` |
| A4 | i18n, `en` | 14 fehlende Schlüssel übersetzen, Liste in `tasks.md` |
| A5 | `app.js:5667` | `view.releaseNotes` nicht mehr roh. Der Inhalt kommt vom konfigurierbaren Update-Host über HTTP |
| A6 | `app.js:2416, 2479, 5131` | Leere `catch (_) {}` mindestens protokollieren |

**Zu A5:** Die Release-Notes sind bewusst HTML — sie enthalten `<H2>`, `<BR>` und Links.
Vollständiges Escaping würde sie als Quelltext anzeigen. Der `pwa-developer` entscheidet
zwischen zwei Wegen und begründet seine Wahl: entweder Whitelist-Filter über `DOMParser`,
der nur unkritische Tags durchlässt, oder `textContent` mit dem Hinweis, dass die
Formatierung entfällt. **Kein `innerHTML` mit ungefiltertem Fremdinhalt.**

## Teil B — STM-Instrumentierung

Eine Stelle, im Hauptloop von `src/main.c` nach `watchdog_reload()`:

```c
static uint_fast8_t icon_freeze_state = 0;
uint_fast8_t icon_freeze_now = (display.do_display_icon && ! display.display_power_is_on) ? 1 : 0;

if (icon_freeze_now != icon_freeze_state)
{
    icon_freeze_state = icon_freeze_now;
    log_printf ("icon_freeze: %s do_icon=%d power=%d astart=%d astop=%d ovl=%d\r\n",
                icon_freeze_now ? "ENTER" : "LEAVE",
                display.do_display_icon, display.display_power_is_on,
                display.animation_start_flag, display.animation_stop_flag,
                show_overlay_idx);
}
```

**Warum `log_printf` und nicht `debug_log_printf`:** Letzteres kompiliert zu nichts,
siehe oben. **Warum das die Blockade nicht verschärft:** Es feuert ausschliesslich beim
Zustandswechsel, also typisch zweimal je Ausschaltvorgang — nicht pro Durchlauf. Damit
liegt die Zusatzlast bei rund 100 Byte pro Ereignis statt bei ~150 Byte pro
Display-Update wie bei den bestehenden unbedingten Zeilen.

## Teil C — Guardrails

Die S7-Regel „innerHTML ohne escapeHtml" wird ersetzt. Die zeilenbasierte Fassung irrt
in beide Richtungen: 21 Fehlalarme bei korrekten `.map()`-Stellen, und sie übersieht
`app.js:5667`, weil dort `escapeHtml` für den Fallback auf derselben Zeile steht.

Neue Prüfung als `tools/checks/innerhtml.mjs`: Für jede `innerHTML =`-Zuweisung wird der
Ausdruck bis zum Anweisungsende gelesen, alle `escapeHtml(...)`-Aufrufe **samt Argument**
entfernt, und danach geprüft, ob noch Bezeichner übrig sind, die keine String-Literale
sind. Bleibt etwas übrig, ist der Wert ungefiltert.

## Prüfung gegen die Architektur-Checkliste

**Proper architecture** — PWA bleibt parallel zu Legacy, Wetter-Endpunkte unberührt, die
Restore-Bedingung wird nicht angefasst. Teil B greift im STM an, wo die Ursache liegt,
statt sie in der App zu umgehen.

**Scalable systems** — Teil A erzeugt **keine** zusätzlichen STM-Kommandos und keine
zusätzliche UART-Last. Teil B fügt rund 100 Byte je Ausschaltvorgang hinzu, nicht je
Durchlauf. Kein Pfad wird verlängert, das Watchdog-Verhalten bleibt unverändert.

**Secure by design** — A5 schliesst die einzige bekannte Stelle, an der Fremdinhalt vom
Update-Host ungefiltert ins DOM gelangt. Die Instrumentierung gibt **keine**
Gerätegeheimnisse aus: nur Flags und einen Overlay-Index.

**Stable & reliable** — A6 beseitigt drei stille Fehlschläge. A1 und A2 beseitigen zwei
`ReferenceError`, von denen einer ohne jede Fehlermeldung auftritt. Teil C macht eine
Prüfung vertrauenswürdig, die bisher in beide Richtungen irrte. Teil B fügt **keinen**
Recovery-Mechanismus hinzu (DIR-003) — der Freeze bleibt bestehen und wird nur sichtbar.

## Versionsfolgen (DIR-004)

- [x] `APP_VERSION` 1.4.69 → **1.4.70**
- [x] `CACHE_NAME` v61 → **v62**
- [x] `VERSION` 3.2.5 → **3.2.6**
- [ ] `ESP_VERSION` unverändert — keine ESP-Quelle berührt
