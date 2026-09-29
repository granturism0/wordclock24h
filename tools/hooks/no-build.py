#!/usr/bin/env python3
"""PreToolUse-Hook: verhindert Builds bei allen Agenten ausser dem release-engineer.

Setzt R1 aus CLAUDE.md durch. Bisher stand die Regel nur als Text dort, und Text
verhindert nichts.

Der Grund ist konkret: `f103` und `f411` haengen beide an `configure` und bauen
mit -j4 in DASSELBE build/stm-rgbw-12h. `esp` baut in dasselbe build/esp8266.
Zwei Agenten, die gleichzeitig bauen, korrumpieren den CMake-Cache -- und der
Fehler zeigt sich spaeter als unerklaerlicher Build-Abbruch, nicht als
Gleichzeitigkeitsproblem.

Registriert wird der Hook im Frontmatter der schreibenden Agenten, nicht global:
der release-engineer muss bauen duerfen, und die Hauptsitzung (der Lead) auch.
"""

import json
import re
import shlex
import sys

# Werkzeuge, die in die geteilten Build-Verzeichnisse schreiben.
BUILDERS = re.compile(r"^(make|gmake|cmake|arduino-cli)$|arduino-cli$|/cmake$")

# Lesende make-Ziele sind harmlos: sie schreiben nur eine Versionsdatei.
HARMLESS_TARGETS = {"stm-version-file", "esp-version-file", "app-version-file"}


def commands(line):
    """Zerlegt eine Kommandozeile an ;, &&, || und | in Einzelkommandos."""
    try:
        parts, cur = [], []
        for tok in shlex.split(line, posix=True):
            if tok in (";", "&&", "||", "|", "&"):
                if cur:
                    parts.append(cur)
                cur = []
            else:
                cur.append(tok)
        if cur:
            parts.append(cur)
        return parts
    except ValueError:
        # Unbalancierte Anfuehrungszeichen: lieber grob zerlegen als durchwinken.
        return [line.split()]


def main():
    raw = sys.stdin.read()
    payload = json.loads(raw) if raw.strip() else {}
    if payload.get("tool_name") != "Bash":
        return 0

    command = payload.get("tool_input", {}).get("command", "")

    # Sonderfall: guardrails.sh --full ruft intern make fuer die Compile-Smoke-Tests.
    # Im Kommandostring taucht kein "make" auf, der Build laeuft trotzdem.
    if "guardrails.sh" in command and "--full" in command:
        print(
            "Blockiert durch R1 (CLAUDE.md): './tools/guardrails.sh --full' baut "
            "intern f103, f411 und esp.\n"
            "Die schnelle Stufe ohne --full ist erlaubt und deckt S1 bis S10 ab.",
            file=sys.stderr,
        )
        return 2

    for argv in commands(command):
        if not argv:
            continue
        prog = argv[0].rsplit("/", 1)[-1]
        if not BUILDERS.match(prog):
            continue
        targets = {a for a in argv[1:] if not a.startswith("-")}
        if targets and targets <= HARMLESS_TARGETS:
            continue

        print(
            "Blockiert durch R1 (CLAUDE.md): Builds laufen ausschliesslich seriell, "
            "und nur der release-engineer baut.\n"
            f"Abgewiesen: {' '.join(argv)}\n\n"
            "Grund: f103 und f411 bauen beide mit -j4 in dasselbe "
            "build/stm-rgbw-12h, esp in dasselbe build/esp8266. Parallele Builds "
            "korrumpieren den CMake-Cache.\n\n"
            "Stattdessen: Quelldateien aendern und 'fertig' melden. Den Build "
            "macht der Lead beziehungsweise der release-engineer am Ende.",
            file=sys.stderr,
        )
        return 2
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:
        # Ein kaputter Hook darf die Arbeit nicht aufhalten.
        print(f"no-build: uebersprungen ({exc})", file=sys.stderr)
        sys.exit(0)
