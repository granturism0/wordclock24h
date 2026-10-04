#!/usr/bin/env python3
"""Stop-Hook: blockiert das Turn-Ende, wenn geaenderte Dateien ungeprueft bleiben.

Hintergrund: "Fuehre die Doku nach" und "lass die Guardrails laufen" sind
Absichtserklaerungen. Sie haben in diesem Projekt nachweislich nicht gehalten --
README-CMAKE.md lag monatelang rund dreissig PWA-Versionen hinter den Quellen,
und der CHANGELOG-Eintrag fuer drei Releases fehlte. Dieser Hook macht daraus
eine Pruefung.

Ablauf: Am Ende jedes Turns wird der Arbeitsbaum angesehen. Sind ueberwachte
Dateien geaendert und lief ./tools/guardrails.sh seither nicht erfolgreich
durch, endet der Turn nicht -- Claude bekommt den Hinweis und holt es nach.

Drei Absicherungen gegen Dauerblockaden, denn ein Stop-Hook, der sich selbst
nicht bremst, haelt die Sitzung fest:
  1. stop_hook_active (setzt Claude Code, wenn der Turn bereits wegen eines
     Stop-Hooks weiterlief) laesst sofort durch
  2. Pro Aenderungsstand wird hoechstens EINMAL blockiert. Der Stand wird
     gehasht; liegt derselbe Hash schon in der nag-Datei, geht der Turn durch
  3. Jeder Fehler im Hook selbst laesst durch (exit 0). Ein kaputter Hook darf
     die Arbeit nicht aufhalten
"""

import hashlib
import json
import subprocess
import sys
from pathlib import Path

# Nur diese Pfade loesen die Pruefung aus. tools/preview/ und build/ bleiben
# aussen vor -- dort aendert sich staendig etwas ohne Auswirkung aufs Fabrikat.
WATCHED = ("src/", "ESP8266/ESP-uclock/", "tools/checks/", "tools/hooks/", "tools/guardrails.sh",
           "CLAUDE.md", "BEFUNDE.md", "CHANGELOG.md", "README", "knowledge/")


def out(msg):
    print(msg, file=sys.stderr)


def repo_root():
    return Path(subprocess.run(["git", "rev-parse", "--show-toplevel"],
                               capture_output=True, text=True, check=True).stdout.strip())


def changed_state(root):
    """Die ueberwachten Aenderungen und ihr Hash.

    Einzige Quelle dieser Berechnung. guardrails.sh ruft dieselbe Funktion
    ueber --stamp auf; gaebe es die Logik zweimal, wuerden Stempel und Pruefung
    auseinanderlaufen und der Hook blockierte dauerhaft.
    """
    status = subprocess.run(["git", "status", "--porcelain"],
                            capture_output=True, text=True, cwd=root).stdout
    relevant = sorted(ln for ln in status.splitlines()
                      if any(w in ln[3:] for w in WATCHED))
    digest = hashlib.sha256("\n".join(relevant).encode()).hexdigest()[:16]
    return relevant, digest


def main():
    # guardrails.sh ruft sich so selbst den Stempel: fuer WELCHEN Stand galt der Lauf.
    if "--stamp" in sys.argv:
        root = repo_root()
        _, state = changed_state(root)
        (root / ".git" / "guardrails-stamp").write_text(state)
        return 0

    raw = sys.stdin.read()
    payload = json.loads(raw) if raw.strip() else {}

    # (1) Der Turn laeuft bereits wegen eines Stop-Hooks weiter.
    if payload.get("stop_hook_active"):
        return 0

    root = repo_root()
    relevant, state = changed_state(root)
    if not relevant:
        return 0

    # ------------------------------------------------- Messungen ohne Befund
    #
    # Am 04.10.2026 hat der Nutzer dreimal nachfragen muessen, ob die Erkenntnisse
    # festgehalten sind -- und dreimal fehlte etwas. Seine Forderung: "Wenn nein
    # nachholen und dies fuer die Zukunft sicherstellen."
    #
    # Automatisch erkennen, OB eine Messung etwas Neues ergeben hat, geht nicht.
    # Was geht: daran erinnern, wenn am Geraet gemessen wurde und BEFUNDE.md
    # seither unveraendert blieb. Das ist kein Beweis fuer ein Versaeumnis -- eine
    # Messung, die nur bestaetigt, braucht keinen Eintrag. Es ist eine Frage, und
    # die kostet weniger als eine verlorene Erkenntnis.
    messmarke = root / ".git" / "geraet-gemessen"
    if messmarke.exists():
        try:
            befunde = (root / "BEFUNDE.md").stat().st_mtime
            gemessen = messmarke.stat().st_mtime
        except OSError:
            befunde = gemessen = 0
        if gemessen > befunde:
            out("")
            out("Am Geraet gemessen, seither nichts in BEFUNDE.md eingetragen.")
            out("")
            out("  Hat die Messung etwas gezeigt, das noch nirgends steht?")
            out("  Auch ein widerlegter Verdacht ist ein Befund -- gerade der.")
            out("")
            out("  Wenn sie nur bestaetigt hat, was schon dokumentiert ist:")
            out("  nichts zu tun, diese Meldung ist dann richtig und folgenlos.")

    # guardrails.sh legt diese Datei bei jedem erfolgreichen Lauf an.
    stamp = root / ".git" / "guardrails-stamp"
    if stamp.exists() and stamp.read_text().strip() == state:
        return 0

    # (2) Pro Aenderungsstand nur einmal blockieren.
    nag = root / ".git" / "guardrails-stop-nag"
    if nag.exists() and nag.read_text().strip() == state:
        return 0
    nag.write_text(state)

    out("Die Guardrails sind fuer diesen Aenderungsstand noch nicht gelaufen.")
    out("")
    out("Geaendert:")
    for ln in relevant[:12]:
        out(f"  {ln}")
    if len(relevant) > 12:
        out(f"  ... und {len(relevant) - 12} weitere")
    out("")
    out("Auszufuehren:  ./tools/guardrails.sh")
    out("")
    out("Danach pruefen, was DIR-004 und DIR-006 verlangen: Versionen der")
    out("geaenderten Komponenten, CHANGELOG.md, und bei geschlossenen Befunden")
    out("den Stand in BEFUNDE.md. Ist alles bereits erledigt oder die Aenderung")
    out("bewusst ungeprueft, geht der naechste Turn ohne erneute Meldung durch.")
    return 2


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:  # (3) Ein kaputter Hook darf nie blockieren.
        print(f"guardrails-before-stop: uebersprungen ({exc})", file=sys.stderr)
        sys.exit(0)
