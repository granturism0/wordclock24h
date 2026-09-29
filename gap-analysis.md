# Gap-Analyse: Agenten-Team-Struktur für wordclock24h

**Stand:** 2026-09-29 · Branch `pwa-decoupling`
**Grundlage:** `agenten-team-erweiterung.md` in der überarbeiteten Fassung vom 2026-09-29
**Status:** Bestandsaufnahme durchgeführt, Migrationsstufen M0 bis M3 umgesetzt.
**Kein Projektcode angefasst** — ausschliesslich Prozess- und Agentenstruktur.

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

### Auftrag 2 — Spec-Phase · **STRUKTUR UMGESETZT, noch keine echte Spec**

**Umgesetzt:** `specs/README.md` mit dem Ablauf und `specs/_template/` mit den drei
Teilen. `design.md` enthält die vier Architektur-Kriterien als zu beantwortende Fragen,
`tasks.md` erzwingt genau einen zuständigen Agenten pro Task und den Guardrail-Lauf
dazwischen. **Noch offen:** die erste echte Spezifikation, siehe M4.

- Kein `specs/`-Verzeichnis, keine `requirements.md`/`design.md`/`tasks.md`
- Anforderungen existieren heute nur als Fliesstext in Chatverläufen und in den „Offenen technischen Themen" in `CLAUDE.md`
- **Risiko konkret belegt:** `REVIEW.md` listet 18 priorisierte Massnahmen ohne Akzeptanzkriterien. Bei Nummer 2 („Restore-Lücke schliessen") ist ohne Spec nicht festgelegt, was als Beweis der Behebung gilt — die Messung aus F2 oder nur „sieht gut aus"

### Auftrag 3 — Zuständigkeits-Grenzen · **UMGESETZT**

Ausgangslage: keine Agenten, also auch keine Grenzen. Der Auftrag „ergänze in jeder
bestehenden Agent-Definition" hatte kein Objekt — die Grenzen mussten beim Anlegen
mitgeschrieben werden statt nachgerüstet.

Umgesetzt sind elf Agenten unter `.claude/agents/`, jeder mit „Zuständig für" und
„NICHT zuständig für" samt Angabe, an welchen Agenten delegiert wird.

**Entscheidend ist der Grundsatz aus der überarbeiteten Fassung:** die Grenze steht
nicht nur im Prompt, sondern wird über die **Werkzeugliste technisch durchgesetzt**.

| Agent | Werkzeuge | Kann Code ändern |
|---|---|---|
| `stm-developer` | Read, Grep, Glob, Edit, Write, Bash | ja — `src/**` |
| `esp-developer` | Read, Grep, Glob, Edit, Write, Bash | ja — ESP-Firmware |
| `pwa-developer` | Read, Grep, Glob, Edit, Write, Bash | ja — `app.js`, `sw.js` |
| `ui-developer` | Read, Grep, Glob, Edit, Write, Bash | ja — HTML, CSS, Manifest |
| `release-engineer` | Read, Grep, Glob, Edit, Bash | nur Versionszeilen |
| `spec-writer` | Read, Grep, Glob, Write | nur unter `specs/` |
| `firmware-analyst` | Read, Grep, Glob | **nein, technisch unmöglich** |
| `code-reviewer` | Read, Grep, Glob | **nein, technisch unmöglich** |
| `ui-reviewer` | Read, Grep, Glob | **nein, technisch unmöglich** |
| `librarian` | Read, Grep, Glob | **nein, technisch unmöglich** |
| `guardrail-runner` | Read, Grep, Glob, Bash | nur per Anweisung eingeschränkt |

Die vier Rollen ohne `Write` und `Edit` können nicht schreiben, unabhängig davon, wie
sie ihre Anweisung interpretieren. Beim `guardrail-runner` bleibt eine Lücke — siehe
Auftrag 7.

### Auftrag 4 — Single-threaded writes · **VORHANDEN, und konkreter als im Konzept**

Einziger Auftrag, der hier bereits erfüllt ist. `CLAUDE.md` enthält R1 bis R6 mit projektspezifischer Begründung:

- R1: nur der Lead baut, weil `f103`/`f411` in dasselbe `build/stm-rgbw-12h` mit `-j4` bauen
- R2: `app-gz` nur bei stillem Arbeitsverzeichnis, sonst `.gz` einer halb geschriebenen Datei
- R3: `app.js` hat genau einen Besitzer
- R4: Versionsbumps nur durch den Lead, vier Stellen müssen zueinander passen
- R5: Hardware exklusiv
- R6: Worktree-Isolation als Ausnahme

**Lücke geschlossen:** Die Regeln standen nur in `CLAUDE.md`. Jetzt sind sie in jede
Agent-Definition eingearbeitet, wie Auftrag 10 Schritt 2 verlangt.

Nach der überarbeiteten Fassung ist Auftrag 4 **kein reines Koordinationsprotokoll
mehr**, sondern folgt direkt aus Auftrag 3: Reviewer, Analyst und Librarian haben kein
`Write` und `Edit` und **können** gar nicht parallel schreiben. Nur der zugewiesene
implementierende Agent besitzt Schreibrechte, und nur bis der nächste Task beginnt —
dieser Satz steht in jeder der fünf schreibenden Definitionen.

### Auftrag 5 — Wissensbasis · **UMGESETZT**

Angelegt sind `quick-reference.md`, `architecture-checklist.md` und `directives.md`.

- Ausgangslage: kein `knowledge/`-Verzeichnis
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

### Auftrag 6 — Librarian-Agent · **ANGELEGT, Bewährung offen**

Angelegt als `.claude/agents/librarian.md`, mit `knowledge/directives.md` und den drei
bereits bestätigten Direktiven.

Nach dem überarbeiteten Auftrag 3 hat der Librarian **kein `Write` und kein `Edit`** —
auch nicht für `directives.md`. Er legt Entwürfe vor, eingetragen werden sie vom Lead
nach ausdrücklicher Bestätigung. Die Überschneidung mit `CLAUDE.md` und dem
projektbezogenen Memory bleibt und muss sich im Betrieb zeigen, siehe M5.

### Auftrag 7 — Guardrails nach jedem Task · **MINIMALBASIS GESCHAFFEN**

Die überarbeitete Fassung stellt dem Auftrag eine Voraussetzung voran: ohne Test-,
Lint- oder CI-Basis ist „blockiert bei Fehlschlägen" reine Dekoration, weil nichts da
ist, das fehlschlagen könnte. Genau das war der Ist-Zustand.

**Ausgangslage:** keine Tests, kein `tests/`, kein Testframework, keine einzige
Testdatei. Keine CI, kein `.github/workflows/`. Kein Linter, keine
ESLint-Konfiguration, kein `package.json`, kein `.clang-format`. Ein vollständiger
Build braucht CMake **und** arduino-cli, dauert Minuten und darf nach R1 nur seriell
laufen — als Guardrail nach *jedem* Task untauglich.

Deshalb wird die Minimalbasis als **eigener, vorgezogener Migrationsschritt M1**
geführt, nicht als Unterpunkt von „Auftrag 7 fehlt". Siehe Migrationsplan unten.

### Auftrag 8 — Architektur-Checkliste · **UMGESETZT**

Angelegt als `knowledge/architecture-checklist.md`, verbindlich geprüft in `design.md`
und im Review-Schritt. Die vier Kriterien bleiben gültig, ihre Prüffragen nicht:

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

## Migrationsplan

Reihenfolge nach Risiko, nicht nach Auftragsnummer. Bestehender Projektcode wird
**nicht** angefasst.

### M0 — Prozessartefakte sichern · **ERLEDIGT**

`CLAUDE.md`, `REVIEW.md` und `gap-analysis.md` waren seit sieben Wochen untracked und
ein `git clean -fd` von der Löschung entfernt. Committet als `4fff45f`.

### M1 — Minimalbasis für Guardrails · **ERLEDIGT** (vorgezogener Schritt zu Auftrag 7)

Der aus dem Projektkontext abgeleitete Minimalstand nach Auftrag 10, Schritt 2. Ein
Shell-Skript ohne neue Abhängigkeiten, plus vier Node-Prüfungen.

`./tools/guardrails.sh` — schnelle Stufe, läuft nach jedem Task, Laufzeit Sekunden:

| Stufe | Prüfung | Belegter Nutzen |
|---|---|---|
| S1 | `node --check` auf `app.js` und `sw.js` | Syntaxfehler |
| S2 | Undefinierte Funktionsaufrufe | **findet beide `ReferenceError` aus `REVIEW.md`** |
| S3 | i18n: benutzte-aber-undefinierte Keys, DE/EN-Parität | **findet die drei `common.*`-Keys, die wörtlich in der UI stehen** |
| S4 | Versionszeilen greppbar, `CACHE_NAME`-Drift gegen `HEAD` | schützt den still brechenden Release-Build |
| S5 | `.gz` vorhanden, nicht leer, nicht veraltet | der White-Screen-Fall |
| S6 | CSS-Klassen ohne Verwendung | **findet die fünf Grid-Area-Klassen** |
| S7 | Grep-Lint gegen `knowledge/quick-reference.md` | unbedingtes `log_printf`, leere `catch`, `innerHTML` ohne `escapeHtml` |
| S8 | **Smoke-Test:** `app.js` wird mit gestubbtem Browser-Umfeld geladen | Fehler beim Laden, nicht nur Syntax |

`./tools/guardrails.sh --full` ergänzt die Compile-Smoke-Tests `make f103`, `f411`,
`esp`. Dauert Minuten, läuft nur beim `release-engineer` und nur seriell (R1).

Exit 0 = Task darf abgeschlossen werden. Exit 1 = blockiert, keine Schreibübergabe.

**Erster Lauf gegen den Bestand: Exit 1**, mit zwei Prüfungen im Kritisch-Bereich und
vier Hoch-Findings — sämtlich Befunde, die `REVIEW.md` bereits belegt hat. Die Basis
ist damit nachweislich wirksam und nicht dekorativ.

Drei der acht Stufen hätten verifizierte Review-Befunde **automatisch** gefunden. Das
ist das stärkste Argument für diesen Auftrag im gesamten Konzept.

### M2 — Wissensbasis und Architektur-Checkliste · **ERLEDIGT**

`knowledge/quick-reference.md`, `knowledge/architecture-checklist.md`,
`knowledge/directives.md`. Die Odoo- und Azure-Beispieleinträge des Konzepts sind
vollständig ersetzt; jeder Eintrag stammt aus einem belegten Befund.

### M3 — Spec-Phase und Agenten · **ERLEDIGT**

`specs/` mit Vorlage und Ablaufbeschreibung. Elf Agenten unter `.claude/agents/` mit
technisch durchgesetzten Grenzen. Zusätzlich
`tools/hooks/guardrail-bash-allowlist.py` als Härtung für den `guardrail-runner`
— getestet, aber **noch nicht registriert**.

### M4 — Erste echte Spec · **OFFEN**

Kandidat: Massnahme 2 aus `REVIEW.md`, die Restore-Lücke in `main.c:3699-3711`. Klein,
verifiziert, mit klarem Akzeptanzkriterium über den Diagnoseschritt F2.

### M5 — Librarian im Betrieb prüfen · **OFFEN**

Angelegt, aber die Abgrenzung zu `CLAUDE.md` und zum projektbezogenen Memory muss sich
erst zeigen. Gegebenenfalls begründet wieder entfernen.

### M6 — Plugin-Paketierung · **ZURÜCKGESTELLT**

Erst nach Bewährung. Zusätzlich fehlt die Infrastruktur — als Marketplace ist nur
`claude-plugins-official` registriert, kein BTPAG-Repository. Die Bewährung müsste
ohnehin in einem BTPAG-Projekt stattfinden, nicht in einem privaten Embedded-Projekt.

## Was bewusst nicht getan wurde

- **Kein Projektcode angefasst.** Keine Zeile in `src/**`, `ESP8266/**` oder
  `data/app/**`. Die 18 Massnahmen aus `REVIEW.md` sind unverändert offen
- **Der Hook ist nicht registriert.** `tools/hooks/guardrail-bash-allowlist.py` ist
  angelegt und getestet, aber in keiner `settings.json` eingetragen. Ein PreToolUse-Hook
  greift in **jeden** Bash-Aufruf der Session ein; die Aktivierung ist eine bewusste
  Entscheidung des Nutzers
- **Keine Spec geschrieben.** M4 wartet auf Freigabe
- **Auftrag 9 nicht begonnen**

## Verifiziert am 2026-09-29: der Hook greift

Die Agentenzuordnung wurde an einer **echten Nutzlast** geprüft, nicht angenommen.

- Subagenten-Aufrufe tragen das Feld **`agent_type`** mit dem Agentennamen
- Aufrufe aus der Hauptsession tragen es **nicht**
- `session_id`, `prompt_id` und `transcript_path` sind bei beiden **identisch** und
  taugen **nicht** zur Unterscheidung — der ursprüngliche Plan, darüber zu gehen,
  hätte nicht funktioniert

Nachweis im Betrieb: `sed -n '…' src/main.c` (rein lesend) passiert, während
`echo test > /tmp/…` mit Exit 2 abgelehnt wird und die Datei nachweislich **nicht**
angelegt wird.

Ein Fehlalarm der ersten Fassung ist dabei aufgefallen und behoben: das Zerlegen an
`;` zerriss zitierte Argumente wie ein `sed`-Zeilenskript. `split_commands()` zerlegt
jetzt zitatbewusst. Verstecktes `rm` nach `;` und `git commit` in einer Pipe werden
weiterhin geblockt.

**Grenze, die bleibt:** Der Hook ist eine Härtung, keine Schranke. Er kennt nur die
Positivliste; ein Befehl, der darauf steht und trotzdem schadet, käme durch. Die
eigentliche Trennung liegt bei den vier Agenten ohne `Write` und `Edit`.
