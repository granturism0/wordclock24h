# Spezifikationen

Vor jeder Implementierung entsteht hier eine Spezifikation in drei Teilen. Sie ist die
**verbindliche Quelle**, gegen die alle weiteren Schritte geprüft werden — nicht das
Verständnis einzelner Agenten.

```
specs/<feature-name>/
  requirements.md   Was soll die Änderung leisten, welche Akzeptanzkriterien gelten
  design.md         Wie wird sie umgesetzt, welche Module sind betroffen,
                    Prüfung gegen knowledge/architecture-checklist.md
  tasks.md          Einzelschritte mit Abhängigkeiten und zuständigem Agenten
```

**Erst nach Freigabe durch den Nutzer beginnt die Implementierung.**

Nach jedem Task aus `tasks.md` läuft `./tools/guardrails.sh`. Erst bei Exit 0 gilt der
Task als abgeschlossen und die Schreibrechte dürfen an den nächsten Agenten übergehen.

Vorlage: `specs/_template/`
