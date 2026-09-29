# Tasks — <Feature-Name>

Einzelschritte mit Abhängigkeiten. **Pro Task genau ein schreibender Agent** (R3, R4).
Nach jedem Task läuft `./tools/guardrails.sh`; erst bei Exit 0 geht die Schreibberechtigung
weiter.

| # | Task | Agent | Hängt ab von | Guardrails | Review |
|---|---|---|---|---|---|
| 1 | | | — | ☐ | ☐ |
| 2 | | | 1 | ☐ | ☐ |
| 3 | | | 2 | ☐ | ☐ |

## Ablauf je Task

1. Der zuständige Agent setzt **nur** seinen Task um
2. `./tools/guardrails.sh` — blockiert bei Kritisch-Findings
3. Review durch den zuständigen Review-Agenten, ausdrücklich gegen
   `knowledge/architecture-checklist.md`
4. Erst dann nächster Task oder Schreibübergabe

## Abschluss

- [ ] Alle Tasks erledigt
- [ ] `./tools/guardrails.sh --full` mit Exit 0 (schliesst Compile-Smoke-Tests ein)
- [ ] Alle Akzeptanzkriterien aus `requirements.md` erfüllt
- [ ] Release-ZIP gebaut, Flash-Umfang benannt
