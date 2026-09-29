# Design — <Feature-Name>

## Lösungsweg

Wie die Änderung technisch umgesetzt wird. Konkret genug, dass der zuständige Agent
sie ohne Rückfrage bauen kann.

## Betroffene Module

| Datei | Änderung | Zuständiger Agent |
|---|---|---|
| | | |

## Prüfung gegen die Architektur-Checkliste

Pflicht. Siehe `knowledge/architecture-checklist.md`. Jeder Punkt wird beantwortet,
nicht abgehakt.

**Proper architecture** — Bleibt die PWA parallel zu Legacy? Werden die
Wetter-Endpunkte eingehalten? Bleibt die Restore-Bedingung vollständig? Greift die
Änderung an der richtigen Schicht an?

> Antwort:

**Scalable systems** — Wie viele STM-Kommandos erzeugt die Aktion? Wie viele Byte pro
Minute zusätzlich auf der UART? Bleibt jeder Pfad unter 20 s Watchdog? Wie lange
blockiert der Hauptloop?

> Antwort:

**Secure by design** — `escapeHtml` bei jedem `innerHTML`? Fremddaten als nicht
vertrauenswürdig behandelt? Keine Credentials in Logausgaben?

> Antwort:

**Stable & reliable** — Wird jeder Fehler ausgewertet? Leere `catch`? Stille
Verwerfungen? Ist der Zustand nach Abbruch mitten in einer Sequenz definiert? Wird ein
gesetztes Flag auf jedem Pfad wieder aufgelöst?

> Antwort:

## Verworfene Alternativen

Was geprüft und warum nicht gewählt wurde. Schützt vor Wiederholung.

## Versionsfolgen

- [ ] STM `src/main.h` anheben
- [ ] ESP `version.h` anheben
- [ ] App `APP_VERSION` anheben
- [ ] `CACHE_NAME` in `sw.js` anheben

Nur der `release-engineer` führt das aus (R4).
