---
name: doku-nachfuehren
description: Führt die Projektdokumentation nach — CHANGELOG, READMEs und den Befundkatalog BEFUNDE.md. Trennt lebende Dokumente von Momentaufnahmen und kennt die zwei Fehler, die hier schon passiert sind. Nutzen nach jedem Release und wenn Befunde geschlossen wurden.
when_to_use: Nach einem Release, nach dem Schliessen eines Befunds, bei neuen Werkzeugen oder Abläufen, und bei jeder Frage, ob die Doku noch stimmt.
allowed-tools: Read Grep Glob Bash
---

# Dokumentation nachführen (DIR-006)

Die Dokumentation zerfällt in zwei Sorten, und die Unterscheidung ist die ganze Regel.

**Lebend** — wird nachgeführt, darf nie veralten:
`CLAUDE.md`, `BEFUNDE.md`, `CHANGELOG.md`, alle `README*.md`, `knowledge/**`,
`.claude/agents/**`, `.claude/skills/**`.

**Momentaufnahme** — trägt ein Datum und wird **nicht** fortgeschrieben:
`REVIEW.md`, `REVIEW-2026-09-29.md`, `gap-analysis.md`, `specs/**`.

## Was wann nachzuführen ist

| Anlass | Was |
|---|---|
| Release | `CHANGELOG.md` — ein Release gilt erst als fertig, wenn der Eintrag steht |
| Befund geschlossen | `BEFUNDE.md` — Status **mit dem Beleg**, an dem die Prüfung hängt |
| Neuer Befund aus der Arbeit | `BEFUNDE.md`, nächste freie `L`-Nummer |
| Neues Werkzeug oder neuer Ablauf | die passende `README`, und bei einer Regel `knowledge/directives.md` |

Der Beleg in `BEFUNDE.md` ist kein Schmuck: Er macht die Prüfung wiederholbar. „offen"
ohne Beleg ist wertlos, „offen, `grep -c 'watchdog_reload ('` in `src/**` liefert
weiterhin 1" ist nachprüfbar.

## Zwei Fehler, die hier schon passiert sind

**Versionsnummern in lebenden Dokumenten.** Der Kopf von `README-CMAKE.md` behauptete
über Monate einen „aktuellen Abschlussstand", der rund dreissig PWA-Versionen zurück
lag. Niemand hat es bemerkt, weil nichts es geprüft hat.

⇒ Schreib **keine** Versionsnummern in lebende Dokumente. Wo ein Stand genannt werden
muss, verweise auf `./tools/guardrails.sh` (Stufe S4). Eine bewusst historische Angabe
bekommt `<!-- historisch -->` ans Zeilenende, sonst schlägt S9 an.

**Absolute Pfade.** `/Users/<name>/…` in Markdown-Links zeigt bei jedem anderen Klon
ins Leere. Es waren sieben Stück, verteilt über `CHANGELOG.md`, `README-CMAKE.md` und
`.claude/settings.json`.

⇒ Links relativ zum Repo-Wurzelverzeichnis.

## Schweizer Schreibung

`ss` statt `ß`, echte Umlaute, keine `ae`/`oe`/`ue`-Umschriften. Durchgehend Du-Form,
auch in UI-Texten und Fehlermeldungen. Bestehende Du-Formulierungen nicht auf „Sie"
umschreiben.

## Zum Schluss prüfen

```
./tools/guardrails.sh
```

**S9** vergleicht Versionsangaben in lebenden Dokumenten gegen die Quellen und meldet
absolute Benutzerpfade. **S10** schlägt an, wenn eine Massnahmennummer aus einem Review
in `BEFUNDE.md` fehlt oder die `L`-Nummerierung eine Lücke hat.

Beides ist bewusst maschinell: „Führe die Doku nach" ist eine Absichtserklärung und hat
in diesem Projekt nachweislich nicht gehalten.
