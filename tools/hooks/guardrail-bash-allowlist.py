#!/usr/bin/env python3
"""PreToolUse-Hook: gleicht Bash-Aufrufe des guardrail-runner gegen eine Positivliste ab.

Haertung zu Auftrag 7. Der guardrail-runner braucht Bash, um Lint und Tests
auszufuehren; Bash ist ein Allzweckwerkzeug. Dieser Hook faengt den offensichtlichen
Missbrauch ab. Er ist ausdruecklich KEINE vollstaendige Schranke — die uebrigen
Analyse- und Review-Agenten haben Write und Edit gar nicht erst.

Verhalten:
  - Kein Bash-Aufruf                  -> durchlassen
  - Nicht vom guardrail-runner        -> durchlassen (fail open)
  - Vom guardrail-runner, erlaubt     -> durchlassen
  - Vom guardrail-runner, verboten    -> Exit 2, Begruendung auf stderr
"""
import json
import os
import re
import sys

AGENT = "guardrail-runner"

# Agentenzuordnung. VERIFIZIERT am 2026-09-29 gegen eine echte Nutzlast:
# Subagenten-Aufrufe tragen das Feld "agent_type" mit dem Agentennamen, Aufrufe
# aus der Hauptsession tragen es nicht. session_id, prompt_id und
# transcript_path sind bei beiden IDENTISCH und taugen NICHT zur Unterscheidung.
# Die uebrigen Namen bleiben als Fallback, falls sich das Feld aendert.
# Bewusst eine enge Positivliste: eine Suche ueber die gesamte Nutzlast wuerde
# auch greifen, wenn der Name nur beilaeufig vorkommt, und fremde Aufrufe blockieren.
AGENT_FIELDS = ("agent_type", "agent", "agent_name", "subagent_type",
                "agentType", "subagentType", "source_agent", "invoked_by")

LOGFILE = os.environ.get("GUARDRAIL_HOOK_LOG", "")


def identify(payload: dict) -> str:
    for field in AGENT_FIELDS:
        value = payload.get(field)
        if isinstance(value, str) and value:
            return value
    meta = payload.get("metadata") or payload.get("context") or {}
    if isinstance(meta, dict):
        for field in AGENT_FIELDS:
            value = meta.get(field)
            if isinstance(value, str) and value:
                return value
    return ""

ALLOWED = (
    "./tools/guardrails.sh", "bash tools/guardrails.sh", "sh tools/guardrails.sh",
    "node tools/checks/",
    "git status", "git diff", "git log", "git rev-parse", "git ls-files",
    "grep", "rg", "find", "ls", "cat", "head", "tail", "wc", "stat", "file",
    "echo", "printf", "true", "pwd", "date", "sort", "uniq", "cut", "awk", "sed",
)

FORBIDDEN = (
    "sed -i", "tee", "mv ", "cp ", "rm ", "rmdir", "mkdir", "touch", "chmod", "chown",
    "ln ", "dd ", "truncate",
    "git add", "git commit", "git checkout", "git restore", "git reset", "git clean",
    "git push", "git stash",
    "make", "cmake", "arduino-cli", "npm install", "pip install", "curl", "wget",
    "python3 -c", "python -c", "node -e",
)


def split_commands(command: str):
    """Zerlegt an ; && || | — aber nur ausserhalb von Anfuehrungszeichen.

    Ein naives re.split zerreisst sonst zitierte Argumente wie das sed-Skript
    '8413,8417p;11878,11882p' und lehnt einen rein lesenden Befehl ab.
    """
    parts, buf, quote, i = [], [], None, 0
    while i < len(command):
        c = command[i]
        if quote:
            buf.append(c)
            if c == quote:
                quote = None
            elif c == "\\" and quote == '"' and i + 1 < len(command):
                i += 1
                buf.append(command[i])
        elif c in ("'", '"'):
            quote = c
            buf.append(c)
        elif c == ";":
            parts.append("".join(buf)); buf = []
        elif c in ("&", "|") and i + 1 < len(command) and command[i + 1] == c:
            parts.append("".join(buf)); buf = []; i += 1
        elif c == "|":
            parts.append("".join(buf)); buf = []
        else:
            buf.append(c)
        i += 1
    parts.append("".join(buf))
    return parts


def fail(reason: str) -> None:
    sys.stderr.write(
        f"Vom guardrail-runner abgelehnt: {reason}\n"
        "Dieser Agent prueft und meldet, er korrigiert nicht. "
        "Melde den Befund an den zustaendigen Agenten.\n"
    )
    sys.exit(2)


def main() -> None:
    try:
        raw = sys.stdin.read()
        payload = json.loads(raw)
    except Exception:
        sys.exit(0)  # unlesbare Nutzlast blockiert nichts

    if payload.get("tool_name") != "Bash":
        sys.exit(0)

    caller = identify(payload)

    # Einmalige Diagnose: schreibt die Feldnamen der echten Nutzlast mit, damit die
    # Agentenzuordnung verifiziert werden kann. Nur aktiv, wenn GUARDRAIL_HOOK_LOG gesetzt ist.
    if LOGFILE:
        try:
            with open(LOGFILE, "a", encoding="utf-8") as fh:
                fh.write(json.dumps({
                    "top_level_keys": sorted(payload.keys()),
                    "session_id": payload.get("session_id"),
                    "transcript_path": payload.get("transcript_path"),
                    "cwd": payload.get("cwd"),
                    "prompt_id": payload.get("prompt_id"),
                    "erkannter_aufrufer": caller,
                    "kommando": (payload.get("tool_input") or {}).get("command", "")[:60],
                }, ensure_ascii=False) + "\n")
        except Exception:
            pass

    if caller != AGENT:
        sys.exit(0)

    command = (payload.get("tool_input") or {}).get("command", "")
    if not command.strip():
        sys.exit(0)

    # harmlose Umleitungen nach /dev/null ausklammern, alle anderen sind verboten
    probe = re.sub(r"2?>&?\s*(/dev/null|&1|&2)", " ", command)

    for token in FORBIDDEN:
        if token in command:
            fail(f"'{token.strip()}' ist nicht erlaubt.")

    if ">" in probe or "<" in probe:
        fail("Datei-Umleitungen sind nicht erlaubt (ausser nach /dev/null).")

    for segment in split_commands(probe):
        seg = segment.strip()
        if not seg:
            continue
        if not any(seg.startswith(a) for a in ALLOWED):
            fail(f"'{seg.split()[0]}' steht nicht auf der Positivliste.")

    sys.exit(0)


if __name__ == "__main__":
    main()
