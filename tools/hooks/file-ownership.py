#!/usr/bin/env python3
"""PreToolUse-Hook: jede Datei hat genau einen Schreiber.

Setzt R3 aus CLAUDE.md durch, und zwar in der geschaerften Fassung -- den Lead
eingeschlossen.

Warum es das braucht, konkret: Am 03.10.2026 hat der Lead waehrend eines laufenden
Auftrags selbst in app.js geschrieben. Der zustaendige Agent kam an, fand die
Korrektur schon vor und brach seinen Patch ab, weil SEIN Skript jeden Anker auf
"genau einmal" prueft. Haette er stur ersetzt, staenden zwei konkurrierende
Implementierungen derselben Pruefung in der Datei. Verhindert hat das seine eigene
Sorgfalt -- nicht die Regel.

R3 liess das zu, weil dort stand: "Pro Runde arbeitet EIN Agent an app.js." Das
regelt die Gleichzeitigkeit unter Agenten und sagt ueber den Lead nichts. Genau
diese Luecke wurde genutzt.

Dieses Projekt hat dieselbe Lektion zweimal gelernt: R1 "nur der Lead baut" stand
nur als Text da, bis no-build.py es erzwang. Phase 8 des Testplans stand nur im
Dokument, bis no-danger.py sie erzwang. Beides haelt, seit es nicht mehr auf
Disziplin angewiesen ist.

Registriert wird der Hook mit --agent <rolle> im Frontmatter des jeweiligen Agenten.

NICHT in .claude/settings.json registrieren, auch nicht mit --agent lead. Gemessen am
03.10.2026: Ein dort eingetragener Hook feuert AUCH innerhalb jeder Unteragenten-
Sitzung, und die Nutzlast enthaelt kein Feld, das beide unterscheidet -- session_id
und transcript_path sind identisch. Ein einziges Deny blockiert den Aufruf, also
sperrte der Hook, der dem stm-developer die Hoheit ueber src/** sichern soll, genau
ihn davon aus. Der erste Auftrag an ihn scheiterte daran.

Fuer den Lead bleibt damit die Regel in CLAUDE.md, nicht der Zwang. Das ist eine
bewusst benannte Luecke und keine vergessene: Erzwingen liesse sie sich nur ueber
eine Verstaendigung der beiden Hook-Aufrufe ueber tool_use_id, und die haengt an
einer Reihenfolge, die nicht zugesichert ist.

Geprueft werden ZWEI Wege, denn der zweite ist der, der hier tatsaechlich benutzt
wird: Write/Edit mit einem Dateipfad, und Bash -- die Agenten patchen durchgehend
ueber Python-Heredocs, ein Hook nur auf Write/Edit haette gar nichts gesehen.

Lesen bleibt immer frei. Geprueft wird nur, was schreibt.
"""

import json
import re
import sys

# Wer welche Dateien schreiben darf. Erste Treffer gewinnt, Reihenfolge ist egal,
# die Muster ueberschneiden sich nicht.
OWNERSHIP = {
    "stm-developer":    [r"^src/"],
    "esp-developer":    [r"^ESP8266/ESP-uclock/(?!data/app/).*\.(cpp|h|ino)$"],
    "pwa-developer":    [r"^ESP8266/ESP-uclock/data/app/(app|sw)\.js$",
                         r"^ESP8266/ESP-uclock/data/app/i18n/.*\.json$"],
    "ui-developer":     [r"^ESP8266/ESP-uclock/data/app/(index\.html|styles\.css|manifest\.webmanifest)$",
                         r"^ESP8266/ESP-uclock/data/app/icons/"],
    "doc-writer":       [r"\.md$", r"^knowledge/"],
    "spec-writer":      [r"^specs/"],
    "release-engineer": [r"^(Makefile|CMakeLists\.txt)$", r"^cmake/", r"^ESP8266/ESP-uclock/version\.h$",
                         r"^src/main\.h$", r"^ESP8266/ESP-uclock/data/app/(app|sw)\.js$"],
    # Der Lead baut, prueft und pflegt Werkzeug und Doku -- er schreibt keinen
    # Produktcode mehr. Das ist die Entscheidung vom 03.10.2026.
    "lead":             [r"^tools/", r"^\.claude/", r"\.md$", r"^knowledge/", r"^specs/",
                         r"^\.gitignore$", r"^(Makefile|CMakeLists\.txt)$", r"^cmake/"],
}

# Wem die Datei sonst gehoert -- nur fuer die Fehlermeldung, damit der Hinweis
# nuetzlich ist statt bloss abweisend.
def owner_of(path):
    for agent, patterns in OWNERSHIP.items():
        if agent in ("lead", "release-engineer"):
            continue
        for pattern in patterns:
            if re.search(pattern, path):
                return agent
    return None


def allowed(agent, path):
    for pattern in OWNERSHIP.get(agent, []):
        if re.search(pattern, path):
            return True
    return False


def normalize(path):
    path = path.strip().strip("'\"")
    # Absolute Pfade auf Repo-relativ zurueckschneiden.
    marker = "/wordclock24h/"
    if marker in path:
        path = path.split(marker, 1)[1]
    return path.lstrip("./")


# Dateien, die ueberhaupt jemandem gehoeren. Alles andere (Scratchpad, /tmp,
# build/) ist frei -- dort entsteht kein gemeinsamer Stand.
TRACKED = re.compile(
    r"^(src/|ESP8266/ESP-uclock/|tools/|\.claude/|knowledge/|specs/|cmake/|Makefile|CMakeLists\.txt|[A-Za-z0-9_-]+\.md)"
)

# Ein Pfad allein ist kein Schreibvorgang. Erst zusammen mit einem dieser Zeichen
# oder Befehle wird daraus einer. grep, sed ohne -i, cat, head und diff bleiben frei.
# Jedes Muster muss den PFAD enthalten. Eine Regel ohne ihn -- etwa "irgendwo steht
# open(p,'w')" -- trifft sonst jede fremde Datei, die im selben Kommando bloss
# ERWAEHNT wird. Genau das ist beim ersten Entwurf passiert: Eine Aenderung an
# CLAUDE.md wurde abgewiesen, weil im neuen Text das Wort app.js vorkam. Ein Hook,
# der die eigene Dokumentation blockiert, wird umgangen statt befolgt.
WRITE_HINTS = re.compile(
    r">>?\s*['\"]?%s"                   # Umlenkung DIREKT in die Datei
    r"|sed\s+-i[^|;]*%s"                # sed in-place
    r"|tee\s+[^|;]*%s"                  # tee
    r"|(?:cp|mv|install)\s+[^|;]*%s"    # kopieren/verschieben AUF die Datei
    r"|open\s*\(\s*['\"]%s['\"]\s*,\s*['\"][wa]"   # Python open('pfad', 'w'/'a')
)


def bash_writes_to(command, path):
    """Grob, aber bewusst auf der sicheren Seite: Ein reines Lesen soll durchgehen."""
    quoted = re.escape(path)
    pattern = WRITE_HINTS.pattern % (quoted, quoted, quoted, quoted, quoted)
    if re.search(pattern, command, re.S):
        return True
    # Python-Heredoc mit Variablenzuweisung: p='…/app.js' … open(p,'wb')
    if re.search(r"=\s*['\"][^'\"]*%s['\"]" % quoted, command) and re.search(r"open\s*\([^)]*['\"][wa]", command):
        return True
    return False


def refuse(agent, path, tool):
    owner = owner_of(path) or "den Lead"
    print(
        f"Blockiert durch R3 (CLAUDE.md): {path} gehoert nicht dir.\n\n"
        f"Du bist '{agent}', die Datei gehoert {owner}.\n"
        f"Abgewiesenes Werkzeug: {tool}\n\n"
        "Jede Datei hat genau EINEN Schreiber je Runde. Zwei Schreiber in derselben "
        "Datei ueberschreiben sich gegenseitig, und das faellt erst auf, wenn der "
        "Stand schon kaputt ist.\n\n"
        + ("Stattdessen: Beauftrage den zustaendigen Agenten damit. Der Lead baut, "
           "prueft und pflegt Werkzeug und Doku -- Produktcode schreibt er nicht "
           "mehr selbst (Entscheidung vom 03.10.2026). Wenn ein Auftrag nicht "
           "ankommt, schick ihn erneut und sag im Bericht, dass er zweimal noetig "
           "war; selbst Hand anzulegen ist die Abkuerzung, die diesen Hook noetig "
           "gemacht hat."
           if agent == "lead" else
           "Stattdessen: Melde den noetigen Eingriff in deinem Bericht -- mit Datei, "
           "Stelle und Begruendung. Der Lead gibt ihn an den zustaendigen Agenten "
           "weiter oder beauftragt dich ausdruecklich damit."),
        file=sys.stderr,
    )
    return 2


def main():
    argv = sys.argv[1:]
    agent = "lead"
    if "--agent" in argv:
        agent = argv[argv.index("--agent") + 1]

    raw = sys.stdin.read()
    payload = json.loads(raw) if raw.strip() else {}

    tool = payload.get("tool_name", "")
    tool_input = payload.get("tool_input", {}) or {}

    if tool in ("Write", "Edit", "NotebookEdit"):
        path = normalize(str(tool_input.get("file_path", "")))
        if not path or not TRACKED.match(path):
            return 0
        if not allowed(agent, path):
            return refuse(agent, path, tool)
        return 0

    if tool == "Bash":
        command = str(tool_input.get("command", ""))
        # Kandidaten einsammeln: alles, was wie ein Pfad im Repo aussieht.
        for match in re.finditer(r"[A-Za-z0-9_./-]+\.(?:c|h|cpp|ino|js|html|css|md|json|webmanifest|sh|py|mjs)\b", command):
            path = normalize(match.group(0))
            if not TRACKED.match(path):
                continue
            if allowed(agent, path):
                continue
            if bash_writes_to(command, match.group(0)) or bash_writes_to(command, path):
                return refuse(agent, path, "Bash")
        return 0

    return 0


if __name__ == "__main__":
    sys.exit(main())
