# Gap-Analyse: Agenten-Team-Struktur für wordclock24h

**Stand:** 2026-09-29 · Branch `pwa-decoupling` · letzter Commit `1d7d54e` (2026-08-11)
**Grundlage:** `agenten-team-erweiterung.md`, angewendet nach Auftrag 10, Schritt 3b
**Status:** reine Bestandsaufnahme, **kein Schreibzugriff auf Projektcode**. Nichts angelegt, nichts verändert ausser dieser Datei.

---

## Schritt 1 — Kontextanalyse

### Projektart

Kein Web-/Odoo-Projekt, sondern ein **Embedded-System mit drei getrennten Laufzeiten und physischer Hardware**. Das ist der wichtigste Unterschied zum Ursprungsprojekt des Konzepts und verändert fast jede Ableitung.

| Komponente | Umfang | Laufzeit | Besonderheit |
|---|---|---|---|
| STM32-Firmware | 49 C-Dateien in `src/` | Bare-Metal, ein Hauptloop | Echtzeit, Watchdog 20 s, DMA, 1-Wire-Timing |
| ESP8266-Firmware | 13 `.cpp` + `.ino` | Arduino | `http.cpp` allein 11'364 Zeilen; Brücke ESP→STM |
| PWA | `app.js` 12'166 Z., `index.html`, `styles.css`, `sw.js` | Browser | Vanilla JS, kein Build-Step, kein Framework |
| Legacy-Web-UI | in `http.cpp` eingebettet | ESP | Stabilitäts-Referenz, bleibt bestehen |
| Build/Release | `Makefile`, CMake, arduino-cli | lokal | erzeugt `.gz`-Assets + Release-ZIP |
| Hardware | eine physische Uhr, ein Serial-Port | — | **exklusiv, nicht parallelisierbar** |

### Warum die generischen Vorlagen hier nicht greifen

- Es gibt **keine Datenbank, keine Migrationen, keine Multi-Tenant-Fähigkeit, keine Zugriffsrechte** — die Odoo-Beispielkriterien aus Auftrag 5 und 8 sind hier gegenstandslos.
- „Scalable systems" im Sinne von Datenvolumen und Nutzerzahl ist irrelevant. Die echte Skalierungsgrenze ist ein **256-Byte-UART-Ring** und ein **20-Sekunden-Watchdog**.
- Es gibt **keinen Azure Key Vault** und keine personenbezogenen Daten. Sicherheitsrelevant sind stattdessen WLAN-Credentials über `/eeprom_settings` und eine ungefilterte `innerHTML`-Stelle.
- Der „Kunde" ist der Entwickler selbst. Eine eigene Business-Analyst-Rolle mit Anforderungserhebung und Change-Impact-Tracking wäre Overhead ohne Gegenwert.

---

## Schritt 2 — Benötigte Agenten und Skills, aus den Komponenten abgeleitet

Nicht aus der Standardliste des Konzepts übernommen, sondern aus den Komponenten oben und aus den Befunden in `REVIEW.md` begründet.

### Vorgeschlagene Agenten

| Agent | Zuständig für | NICHT zuständig für | Warum diese Rolle |
|---|---|---|---|
| **firmware-analyst** | STM32-Analyse: Display-Zustandsmaschine, blockierende Pfade, Watchdog, DMA, EEPROM-Kosten, 1-Wire-Timing. Liefert Befunde mit `[BELEGT]`/`[PLAUSIBEL]`/`[SPEKULATION]` | Code schreiben, PWA, Builds, Flashen | Die schwersten Befunde des Reviews liegen hier, und sie brauchen eine eigene Denkweise: Timing und Nebenläufigkeit statt Applikationslogik |
| **esp-bridge-analyst** | `http.cpp`, `vars.cpp`, UART-Brücke ESP↔STM, Kommando-Mapping, Debug-Ausgaben | STM-Interna, PWA-UI, Builds | Eigene Rolle, weil `http.cpp` mit 11'364 Zeilen zwischen den beiden anderen Welten sitzt — und weil der grösste Stabilitätshebel (Kernbefund 4) genau dort liegt, nicht in der PWA |
| **pwa-developer** | `app.js`, `sw.js` — **der einzige Agent mit Schreibrecht auf `app.js`** | `index.html`/`styles.css` gleichzeitig, STM/ESP, Versionsbumps, Builds | Direkte Umsetzung von R3 in `CLAUDE.md`. `app.js` ist der Engpass, an dem sich zwei Agents zuverlässig gegenseitig überschreiben |
| **ui-reviewer** | `index.html`, `styles.css`, Manifest, Icons: A11y, Kontraste, iOS-Safari, Du-Form-Konsistenz | `app.js`-Logik, Firmware | Arbeitet auf Dateien, die zu `app.js` disjunkt sind — dadurch echt parallelisierbar zu `pwa-developer` |
| **release-engineer** | **Der einzige, der `make` ausführt.** Versionsbumps in allen vier Stellen, `CACHE_NAME`, Release-ZIP, Aussage was zu flashen ist | Fachliche Änderungen an Quellcode | Setzt R1 und R4 in `CLAUDE.md` in eine Rolle um, statt sie nur als Prosa zu haben |
| **librarian** | Auftrag 6: Direktiven aus „immer"/„ab jetzt"/„merke dir" erkennen, formulieren, zur Bestätigung vorlegen | Schreiben ohne Bestätigung, fachliche Entscheide | Siehe Einschränkung unten |

**Bewusst nicht vorgeschlagen:** ein `business-analyst-agent`. Die Spec-Phase aus Auftrag 2 bleibt sinnvoll, aber der Lead erstellt sie zusammen mit dem jeweiligen Fachagenten. Eine eigene Anforderungs-Rolle erzeugt in einem Ein-Personen-Hobbyprojekt Ablagestruktur ohne Nutzen.

**Einschränkung beim librarian:** Claude Code hat inzwischen ein eigenes dateibasiertes Gedächtnis pro Projekt. Ein librarian-Agent, der parallel `knowledge/directives.md` pflegt, dupliziert das teilweise. Empfehlung: **Auftrag 6 zurückstellen**, bis 2, 5 und 7 laufen — und dann entscheiden, ob `directives.md` noch gebraucht wird oder ob `CLAUDE.md` plus Memory reichen.

### Vorgeschlagene Skills

| Skill | Inhalt | Ersetzt aus dem Konzept |
|---|---|---|
| `wordclock-invariants` | Die Architektur-Invarianten aus `CLAUDE.md` als prüfbare Regeln: `.gz`-only, Grösse > 0, Wetter-Endpunkte, Restore-Bedingung | — |
| `embedded-timing-review` | Prüffragen für STM-Änderungen: blockiert der Pfad den Hauptloop, gibt es einen Watchdog-Reload, ist Logging bedingt, wird `display_clock_flag` überschrieben | ersetzt den generischen Review-Teil |
| `pwa-quality` | i18n-Key-Parität, `escapeHtml` bei `innerHTML`, `response.ok`, Timeout, In-Flight-Guard, A11y-Mindestsatz | ersetzt OWASP-Skill in reduzierter, projektpassender Form |

ISO 27001 wird hier **nicht** gebraucht. Von OWASP ist genau ein Punkt relevant (ungefiltertes `innerHTML` mit servergesteuertem Inhalt) — der gehört als Zeile in `quick-reference.md`, nicht als eigene Skill.

---

## Schritt 3b — Auftrag für Auftrag: vorhanden, teilweise, fehlt

### Auftrag 1 — Bestandsaufnahme · **ERLEDIGT, Ergebnis: nichts vorhanden**

- `.claude/agents/` — existiert nicht
- `~/.claude/agents/` — existiert nicht
- `.claude/skills/` — existiert nicht
- `.claude/` enthält ausschliesslich `settings.local.json` (8 Permission-Regeln)
- Der im Konzept vorausgesetzte `business-analyst-agent` sowie die ISO-27001- und OWASP-Skills stammen aus einem **anderen** Projekt und existieren hier nicht

**Folge:** Auftrag 3 („ergänze in jeder bestehenden Agent-Definition…") hat kein Objekt. Grenzen müssen beim Anlegen direkt mitgeschrieben werden, nicht nachgerüstet.

### Auftrag 2 — Spec-Phase · **FEHLT**

- Kein `specs/`-Verzeichnis, keine `requirements.md`/`design.md`/`tasks.md`
- Anforderungen existieren heute nur als Fliesstext in Chatverläufen und in den „Offenen technischen Themen" in `CLAUDE.md`
- **Risiko konkret belegt:** `REVIEW.md` listet 18 priorisierte Massnahmen ohne Akzeptanzkriterien. Bei Nummer 2 („Restore-Lücke schliessen") ist ohne Spec nicht festgelegt, was als Beweis der Behebung gilt — die Messung aus F2 oder nur „sieht gut aus"

### Auftrag 3 — Zuständigkeits-Grenzen · **FEHLT** (mangels Agenten)

Siehe Schritt 2 oben: Grenzen sind vorgeschlagen, aber nirgends hinterlegt.

### Auftrag 4 — Single-threaded writes · **VORHANDEN, und konkreter als im Konzept**

Einziger Auftrag, der hier bereits erfüllt ist. `CLAUDE.md` enthält R1 bis R6 mit projektspezifischer Begründung:

- R1: nur der Lead baut, weil `f103`/`f411` in dasselbe `build/stm-rgbw-12h` mit `-j4` bauen
- R2: `app-gz` nur bei stillem Arbeitsverzeichnis, sonst `.gz` einer halb geschriebenen Datei
- R3: `app.js` hat genau einen Besitzer
- R4: Versionsbumps nur durch den Lead, vier Stellen müssen zueinander passen
- R5: Hardware exklusiv
- R6: Worktree-Isolation als Ausnahme

**Lücke:** Die Regeln stehen nur in `CLAUDE.md`. Sie sind nicht in Agent-Definitionen eingearbeitet, wie es Auftrag 10 Schritt 2 verlangt. Ohne Agenten war das bisher auch nicht möglich.

### Auftrag 5 — Wissensbasis `knowledge/quick-reference.md` · **FEHLT, aber Rohmaterial ist da**

- Kein `knowledge/`-Verzeichnis
- **Aber:** `REVIEW.md` ist faktisch eine ungeordnete Vorstufe davon. Die Beispieleinträge des Konzepts (Key Vault, Odoo-Migrationsskript, ISO 27001 A.8.15) sind hier sämtlich gegenstandslos und müssen vollständig ersetzt werden

Projektspezifischer Ersatz, direkt aus belegten Befunden abgeleitet:

| Ich sehe… | Ich tue… | Schweregrad |
|---|---|---|
| Busy-Wait oder `delay_sec` im Web-Kommandopfad ohne `watchdog_reload()` | Pfad in den Hauptloop verlagern oder Reload ergänzen | Kritisch |
| `log_printf` statt `debug_log_printf` in einem Pfad, der pro Refresh läuft | Auf `debug_log_printf` umstellen | Kritisch |
| `display_clock_flag` wird zugewiesen statt verodert | Prüfen, ob ein anstehendes Update überschrieben wird | Kritisch |
| Existenzprüfung auf `.gz` ohne `size > 0` | Grössenprüfung ergänzen | Kritisch |
| `fetch()` ohne `response.ok` oder ohne Timeout | `fetchWithTimeout` verwenden, Status prüfen | Hoch |
| `innerHTML` mit Daten vom Gerät oder Update-Server | `escapeHtml` oder `textContent` | Hoch |
| `translate("…")` auf einen Key, der in keiner Tabelle steht | Key in **beiden** Tabellen ergänzen | Hoch |
| Neue Schleife mit mehreren `await apiFetch` in Folge | Kosten pro Kommando am STM prüfen, Pause erwägen | Hoch |
| Deutscher String direkt im Code statt über `translate()` | In die i18n-Tabelle, Du-Form beibehalten | Mittel |
| CSS-Klasse angelegt, aber im HTML nie gesetzt | Im HTML nachziehen oder CSS entfernen | Mittel |

### Auftrag 6 — Librarian-Agent · **FEHLT, Zurückstellung empfohlen**

Siehe Einschränkung in Schritt 2. Kein `knowledge/directives.md`. Überschneidet sich mit dem projektbezogenen Memory von Claude Code.

### Auftrag 7 — Guardrails nach jedem Task · **FEHLT, und so nicht umsetzbar**

Der kritischste Punkt der ganzen Analyse.

Das Konzept verlangt drei Schritte: Lint gegen `quick-reference.md`, **Tests ausführen**, Review. Ist-Zustand:

- **Keine Tests.** Kein `tests/`, kein Testframework, keine einzige Testdatei im ganzen Repository
- **Keine CI.** Kein `.github/workflows/`
- **Kein Linter.** Keine ESLint-Konfiguration, kein `package.json`, kein `.clang-format`
- Ein vollständiger Build braucht CMake **und** arduino-cli, dauert Minuten und darf nach R1 nur seriell laufen — als Guardrail nach *jedem* Task ungeeignet

**„Tests ausführen" muss für dieses Projekt neu definiert werden.** Vorschlag, gestaffelt nach Kosten:

| Stufe | Prüfung | Kosten | Hätte gefunden |
|---|---|---|---|
| 1 | `node --check app.js` | Sekunden | Syntaxfehler |
| 2 | Undefined-Function-Check über `app.js` | Sekunden | **beide `ReferenceError` aus dem Review** |
| 3 | i18n-Key-Parität DE/EN + benutzte-aber-undefinierte Keys | Sekunden | **die drei `common.*`-Keys, die als Rohtext auf dem Button stehen** |
| 4 | Versionskonsistenz über die vier Stellen | Sekunden | inkonsistente Releases |
| 5 | Alle `.gz` vorhanden und Grösse > 0 | Sekunden | White-Screen-Fall |
| 6 | CSS-Klassen ohne HTML-Verwendung | Sekunden | **die fünf Grid-Area-Klassen** |
| 7 | Grep-Lint gegen `quick-reference.md` (z. B. `log_printf` im Refresh-Pfad) | Sekunden | Logblockade |
| 8 | Voller Build `make release-zip` | Minuten | Kompilierfehler |

Stufen 1 bis 7 sind ein einzelnes Shell-Skript ohne neue Abhängigkeiten und laufen nach jedem Task. Stufe 8 bleibt beim `release-engineer` und läuft nicht nach jedem Task, sondern vor jedem Release.

**Bemerkenswert:** Die Stufen 2, 3 und 6 hätten drei der verifizierten Review-Befunde automatisch gefunden. Das ist das stärkste Argument für diesen Auftrag im gesamten Konzept.

### Auftrag 8 — Architektur-Checkliste · **FEHLT, Kriterien müssen übersetzt werden**

Kein `knowledge/architecture-checklist.md`. Die vier Kriterien bleiben gültig, ihre Prüffragen nicht:

| Kriterium | Konzept (Odoo) | Übersetzung für wordclock24h |
|---|---|---|
| Proper architecture | keine Umgehung bestehender Schichten | Bleibt die PWA parallel zu Legacy? Werden die Wetter-Endpunkte eingehalten, kein Legacy-Bypass? Wird die Restore-Bedingung nicht vereinfacht? |
| Scalable systems | Datenvolumen, Nutzerzahl | **Kommandorate statt Datenvolumen:** Wie viele STM-Kommandos erzeugt die Aktion? Wie viele Byte pro Minute auf der UART? Passt das in 256 Byte RX-Ring? Bleibt jeder Pfad unter 20 s Watchdog? |
| Secure by design | OWASP, Key Vault | `escapeHtml` bei jedem `innerHTML`; keine Credentials in Logausgaben; Update-Server-Inhalte als nicht vertrauenswürdig behandeln |
| Stable & reliable | Fehlerbehandlung, keine stillen Fehlschläge | **Direkt belegt:** `runButtonRequest` verschluckt Fehler, `catch (_) {}` beim Overlay-Löschen, stilles Verwerfen im UART-Ring ohne `else`-Zweig, „gespeichert" für stillschweigend zurechtgebogene Werte |

### Auftrag 9 — Plugin-Paketierung · **NICHT ANWENDBAR, korrekt zurückgestellt**

Das Konzept sagt selbst: erst angehen, nachdem sich 1 bis 8 bewährt haben. Zusätzlich fehlt die Infrastruktur — als Marketplace ist nur `claude-plugins-official` registriert, **kein BTPAG-Repository**. Ausserdem ist wordclock24h ein privates Embedded-Projekt; die generischen Teile liessen sich von hier aus zwar extrahieren, aber die Bewährung müsste in einem BTPAG-Projekt stattfinden, nicht hier.

---

## Zusätzlicher Befund ausserhalb des Konzepts

Zwei Dinge, die bei der Bestandsaufnahme aufgefallen sind und jede Prozessarbeit überlagern:

1. **Seit 2026-08-11 wurde nichts committet.** `CLAUDE.md` und `REVIEW.md` sind seit sieben Wochen **untracked**. Ein `git clean -fd` löscht beide. Sie sind die einzigen Prozessartefakte, die es gibt.
2. **Die Review-Befunde sind unverändert offen.** Stichprobe: `loadDebugOverrides()` hat weiterhin genau einen Treffer (die fehlerhafte Aufrufstelle, keine Definition), `normalizeUrlPath` ebenso, `watchdog_reload()` hat weiterhin genau eine Aufrufstelle im gesamten `src/`-Baum. Keine der 18 Massnahmen ist umgesetzt.

Ein Agenten-Prozess erhöht die Qualität künftiger Änderungen. Er arbeitet die bestehenden Befunde nicht ab.

---

## Priorisierter Migrationsplan

Reihenfolge nach Risiko, nicht nach Auftragsnummer — wie vom Konzept verlangt. Bestehender Projektcode wird dabei **nicht** angefasst.

| Stufe | Schritt | Auftrag | Begründung |
|---|---|---|---|
| **0** | `CLAUDE.md`, `REVIEW.md`, `gap-analysis.md` committen | — | Die einzigen Prozessartefakte sind ungesichert. Kostet eine Minute |
| **1** | Guardrail-Skript Stufen 1–7 anlegen, plus `knowledge/quick-reference.md` | 5 + 7 | Grösstes Risiko, kleinster Aufwand. Hätte drei verifizierte Befunde automatisch gefunden. Funktioniert sofort, auch ohne einen einzigen Agenten |
| **2** | `knowledge/architecture-checklist.md` mit den übersetzten Prüffragen | 8 | Ohne sie ist der Review-Schritt aus Stufe 1 inhaltsleer |
| **3** | Spec-Phase einführen, erste Spec für eine kleine reale Änderung | 2 | Kandidat: Massnahme 2 aus `REVIEW.md`, die Restore-Lücke. Klein, belegt, mit klarem Akzeptanzkriterium über F2 |
| **4** | Agenten anlegen, Grenzen und R1–R6 direkt eingearbeitet | 3 + 4 | Erst jetzt sinnvoll, weil die Agenten auf Stufe 1–3 verweisen können |
| **5** | Librarian, oder begründet weglassen | 6 | Überschneidung mit dem Memory zuerst klären |
| **6** | Plugin-Paketierung | 9 | Erst nach Bewährung, und eher in einem BTPAG-Projekt als hier |

---

## Was ich bewusst nicht getan habe

Nach Auftrag 10, „Ergebnis": nur dieser Plan, keine Umsetzung. Konkret **nicht** angelegt oder verändert:

- keine Agent-Definitionen in `.claude/agents/`
- kein `knowledge/`, kein `specs/`
- kein Guardrail-Skript
- kein Commit
- keine Zeile Projektcode

Einzige geschriebene Datei ist `gap-analysis.md` selbst.
