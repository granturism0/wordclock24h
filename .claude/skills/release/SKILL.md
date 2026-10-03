---
name: release
description: Baut ein Release der WordClock und rollt es aus — Versionspflicht je Komponente (DIR-004), Build aller Komponenten, Guardrails, Rollout auf die Synology (DIR-005) und die Aussage, was zu flashen ist. Nutzen, sobald ein Fabrikat entstehen soll, auch bei "bau das mal" oder "roll das aus".
when_to_use: Wenn ein Build, ein Release-ZIP, ein Rollout auf den Update-Server oder eine Versionserhöhung ansteht.
allowed-tools: Read Grep Glob Bash
---

# Release der WordClock

Vier Schritte, in dieser Reihenfolge. Jeder Schritt hat einen Abbruchgrund.

## 1. Feststellen, was sich geändert hat

Bezugsgrösse ist das **letzte Release-Tag**, nicht der letzte Commit — ein Bump kann
mehrere Commits zurückliegen und wäre gegen `HEAD` unsichtbar.

```
git diff --name-only $(git describe --tags --abbrev=0 --match 'release/*')
```

## 2. Versionen anheben — nur die geänderten Komponenten (DIR-004)

**Kein Gleichschritt.** Ändert ein Release nur den STM-Code, steigt nur dessen Version.

| Geändert | Anheben | Datei |
|---|---|---|
| `src/**`, `CMakeLists.txt`, `cmake/**` | `VERSION` | `src/main.h` |
| `ESP8266/ESP-uclock/*.cpp`, `*.h`, `*.ino` (ohne `data/`) | `ESP_VERSION` | `version.h` |
| `data/app/**` (ohne `.gz`) | `APP_VERSION` **und** `CACHE_NAME` | `app.js`, `sw.js` |

Beide Richtungen sind falsch: Code geändert ohne Bump macht das Fabrikat keinem Commit
mehr zuordenbar; ein Bump ohne Codeänderung bietet dem Gerät ein OTA-Update auf
identische Firmware an.

`APP_VERSION` und `CACHE_NAME` gehören dagegen **immer** zusammen — ohne
`CACHE_NAME`-Bump liefert der Service Worker neue `index.html` mit alter `app.js`.
Das ist das einzige Kritisch-Finding in Guardrail-Stufe S4.

Die Trennung von ESP und PWA ist der wunde Punkt: beide liegen unter `ESP8266/`,
werden aber getrennt versioniert und getrennt ausgeliefert.

Der Makefile liest die Versionszeilen per `grep` (`stm-version-file`, `esp-version-file`,
`app-version-file`). **Das Format der Zeilen darf sich nicht ändern**, sonst bricht der
Release-Build still.

## 3. Bauen

```
make app-gz        # nur die .gz-Artefakte
make release-zip   # app-gz + app-version-file + f103 + f411 + esp + ZIP
```

Nach jeder relevanten Änderung **kompletter Build und Release-ZIP**, nicht nur `app-gz`.

**Vor `app-gz`: `git status` prüfen.** `gzip -9 -k -f` auf eine halb geschriebene
`app.js` erzeugt eine `.gz` einer unvollständigen Datei — genau der White-Screen-Fall.
Alle Editier-Tasks müssen abgeschlossen sein (R2).

**Falle: `RELEASE_ZIP` in `Makefile:18`** nutzt `date +"%Y-%m-%d-%H%M"`, also
Minutengenauigkeit, und `Makefile:74` macht `rm -f` darauf. Zwei Release-Builds in
derselben Minute erzeugen denselben Dateinamen; der zweite überschreibt den ersten
**kommentarlos, und beide melden Erfolg**. Das erklärt vermutlich frühere Beobachtungen,
dass Zeitstempel im Release „nicht aktuell" wirkten. Bis das behoben ist: nie zwei
Release-Builds in derselben Minute starten.

Anschliessend `./tools/guardrails.sh`. Mit `--full` laufen zusätzlich die
Compile-Smoke-Tests; das baut und darf nach R1 nur seriell laufen.

## 4. Ausrollen (DIR-005)

```
./tools/deploy.sh --dry-run    zeigt, was uebertragen wuerde
./tools/deploy.sh              uebertraegt
```

Ziel ist der Pfad aus `DEPLOY_PATH` auf dem Host aus `tools/deploy.conf` (SSH). Die Zugangsdaten stehen nicht im Repo — Vorlage: `tools/deploy.conf.example`.
Übertragen werden App-Assets, beide `.hex`, die ESP-`.bin` und die Versionsdateien.
Zusätzlich wird die H2-Kopfzeile in `releasenote.html` auf die aktuelle STM-Version
nachgezogen, und nach erfolgreichem Rollout entsteht ein Tag
`release/<stm>-<esp>-<app>`.

**Das Ziel ist zugleich der Update-Server, von dem die Uhr per OTA lädt.** Deshalb
prüft das Skript jedes Artefakt, bevor es irgendetwas überträgt, und bricht ab statt
einen kaputten Stand auszurollen.

Dort liegen ausserdem Dateien, die der Nutzer selbst pflegt: `wc-list.txt`,
`wc-list-tables.txt`, die Layout-Tabellen und `releasenote.html`. Das Skript löscht
nichts; **niemals `--delete` ergänzen.**

## 5. Abschluss

- **Immer explizit sagen, was zu flashen ist**: nur App/LittleFS, oder auch STM
  beziehungsweise ESP. Wurde eine Version ohne Codeänderung angehoben, sag dazu, dass
  der Flash keinen Gegenwert hat.
- `CHANGELOG.md` nachführen — ein Release gilt erst als fertig, wenn der Eintrag steht
  (DIR-006).
- Sind Befunde geschlossen worden, den Stand in `BEFUNDE.md` nachziehen.
- **Committen und taggen, sofort (DIR-011).** Siehe unten.

### DIR-011 — jedes Release wird einzeln committet und getaggt

**Ein ausgerolltes Release ohne Commit und Tag ist nicht abgeschlossen.** Das gehört
zum Rollout wie der Smoketest, nicht in eine Warteschlange und nicht ans Ende eines
Arbeitstages.

Reihenfolge: bauen → ausrollen → flashen → Smoketest → `CHANGELOG.md` und
`BEFUNDE.md` nachziehen → **committen** → **taggen**. Das Tag heisst
`release/<stm>-<esp>-<app>` und trägt genau die Versionen, die in diesem Commit
stehen.

**Warum das nicht nachholbar ist.** Am 03.10.2026 sind drei Releases
(`1.4.79`, `1.4.80`, `1.4.81`) in einem einzigen Commit gelandet, weil nach jedem
Rollout nur gemeldet wurde „nicht committet, sag Bescheid". Rekonstruieren liessen
sich die Zwischenstände danach nicht mehr: Die Versionsdateien tragen nur den
Endstand, und `app.js` enthielt längst alle drei Änderungen. Ein Tag
`release/3.2.11-3.2.9-1.4.79` auf einem Commit, in dem `1.4.81` steht, wäre eine
Falschaussage — und zwar genau die Sorte, die dieses Projekt sonst aus der
Dokumentation entfernt.

Die beiden verlorenen Tags sind damit endgültig weg. Es bleibt ein Commit mit drei
Releases, und das ist genau der Zustand, den das Tag-Schema verhindern soll.

**`deploy.sh` sagt es von selbst.** Bei unsauberem Arbeitsbaum meldet es

```
Arbeitsbaum nicht sauber — Tag release/<…> NICHT gesetzt.
Nach dem Commit nachholen:  git tag -a release/<…> -m '…'
```

samt fertigem Befehl. Diese Zeile dreimal zu lesen und weiterzuarbeiten war der
eigentliche Fehler — nicht das Vergessen, sondern das Übergehen eines Hinweises, der
jedes Mal dastand.

**Bezug zu S4.** Die Stufe misst DIR-004 gegen das **letzte Release-Tag**. Fehlt es,
misst sie gegen einen veralteten Stand und meldet berechtigt „Code geändert, Version
steht noch" — über Stunden, bei jedem Lauf. Auch das war ein Hinweis.

## STM32 flashen — immer über das Skript

```
./tools/flash-stm.sh
```

Nicht `/api/remote_stm32_flash` von Hand aufrufen. Ohne den Pflichtparameter
`filename` tut der Endpunkt **stillschweigend nichts**, und nach dem Flashen ist ein
**STM-Reset nötig**, den der ESP nicht selbst auslöst. Beides steht in `CLAUDE.md`
unter DIR-010; das Skript erzwingt es und weist die Version am Ende nach.
