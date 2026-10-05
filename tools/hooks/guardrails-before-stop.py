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

Seit 04.10.2026 prueft der Hook drei Dinge, nicht mehr nur eines: die
Guardrails, die festgehaltene Erkenntnis nach einer Geraetemessung und den
Push zum Remote. Sie werden GESAMMELT und gemeinsam gemeldet.

Das Sammeln ist kein Feinschliff, sondern eine Fehlerbehebung: Die
Messmarken-Pruefung schrieb ihre Meldung auf stderr und gab danach `return 0`
zurueck, sobald die Guardrails sauber waren -- und bei exit 0 liest stderr
niemand. Sie konnte also nur anschlagen, wenn ohnehin schon blockiert wurde.
Dieselbe Gattung wie L181 (Logwache in gepufferter Pipe) und L185
(Heap-Zeile am API-Ring vorbei): der Mechanismus war gebaut, die Meldung kam
nie an. Dreimal in drei Tagen.

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


def ungepusht(root):
    """Was liegt noch lokal? Gibt eine Liste von Meldungszeilen zurueck.

    DIR-011 verlangt, jedes ausgerollte Release sofort zu committen und zu
    taggen. Was dort NICHT stand und deshalb dreimal unterging: es gehoert
    auch zum Remote. Am 04.10.2026 lagen 23 Commits und 11 release-Tags nur
    lokal -- gefragt hat der Nutzer, nicht die Pruefung. BEFUNDE.md fuehrte
    das als E4 ("sechs Tags liegen nur lokal"), also als Aufgabe statt als
    Ablaufregel, und eine Aufgabe erinnert niemanden.

    Die Commit-Pruefung laeuft rein lokal gegen @{upstream} und kostet nichts.
    Die Tag-Pruefung braucht das Netz; sie bekommt ein kurzes Zeitlimit und
    schweigt bei jedem Fehler -- ohne Verbindung ist "nicht gepusht" keine
    Aussage, die man treffen kann.
    """
    zeilen = []

    def git(*a, timeout=10):
        return subprocess.run(["git", *a], capture_output=True, text=True,
                              cwd=root, timeout=timeout)

    try:
        r = git("rev-list", "--count", "@{upstream}..HEAD")
        voraus = int(r.stdout.strip()) if r.returncode == 0 else 0
    except Exception:
        voraus = 0
    if voraus:
        zeilen.append(f"  {voraus} Commit(s) liegen nur lokal (git push)")

    try:
        lokal = {t for t in git("tag", "-l", "release/*").stdout.split()}
        if lokal:
            r = git("ls-remote", "--tags", "origin", timeout=8)
            if r.returncode == 0:
                draussen = {ln.split("refs/tags/")[-1].replace("^{}", "")
                            for ln in r.stdout.splitlines() if "refs/tags/" in ln}
                fehlt = sorted(lokal - draussen)
                if fehlt:
                    zeilen.append(f"  {len(fehlt)} release-Tag(s) nur lokal: "
                                  + ", ".join(fehlt[:3])
                                  + (" ..." if len(fehlt) > 3 else "")
                                  + "  (git push --tags origin)")
    except Exception:
        pass            # ohne Netz keine Aussage -- und erst recht keine Blockade

    return zeilen


# Wendungen, die eine Arbeit fuer SPAETER ankuendigen. Eng gehalten: Nur die
# Ich-Form mit klarem Vorsatz, kein blosses "als Naechstes steht X an" (das ist ein
# Bericht ueber die Planung und voellig richtig).
VORSATZ = [
    # Ich-Form mit Vorsatz
    "ich fange", "ich beginne", "ich mache mich", "ich starte jetzt",
    "ich nehme mir", "ich ziehe", "ich arbeite", "ich setze", "ich baue",
    "ich gehe", "ich kuemmere", "ich kümmere", "ich melde mich, sobald",
    # UNPERSOENLICHE Ankuendigungen -- nachgetragen am 05.10.2026, weil die Pruefung
    # ihren Gegenstand nicht ganz sah. Der Turn endete mit "Als Naechstes der Review
    # der STM-Seite, dann der Versionsbump und der Build." Kein "ich", also kein
    # Treffer -- und der Nutzer musste zum VIERTEN Mal nachfassen: "Brauchst du etwas
    # von mir, dass du wieder gestoppt hast?"
    #
    # Genau die Gattung, gegen die diese Pruefung gebaut wurde: ein Muster, das enger
    # ist als die Wirklichkeit (L235, L276). Eine Ankuendigung bleibt eine
    # Ankuendigung, ob sie ein "ich" traegt oder nicht.
    "als naechstes", "als nächstes", "danach kommt", "danach folgt",
    "jetzt kommt", "jetzt folgt", "dann kommt", "dann folgt",
    "steht als naechstes", "steht als nächstes", "weiter geht es",
]
# Gegenanzeigen: Wenn der Satz zugleich sagt, WARUM es nicht weitergeht, ist die
# Ankuendigung richtig und kein Fehler.
WARTET_AUF = [
    "sobald du", "wenn du", "deine entscheidung", "deine freigabe", "brauche ich von dir",
    "warte auf", "wartet auf dich", "gib mir", "sag mir", "bevor ich",
]


def letzter_vorsatz(transcript_path):
    """Letzter Textblock des Turns -- endet er mit einem Arbeitsvorsatz?"""
    if not transcript_path:
        return None
    try:
        zeilen = Path(transcript_path).read_text(encoding="utf-8").splitlines()
    except OSError:
        return None
    text = None
    for zeile in reversed(zeilen):
        try:
            satz = json.loads(zeile)
        except ValueError:
            continue
        if satz.get("type") != "assistant":
            continue
        inhalt = (satz.get("message") or {}).get("content") or []
        stuecke = [c.get("text", "") for c in inhalt if isinstance(c, dict) and c.get("type") == "text"]
        if stuecke:
            text = "\n".join(stuecke)
            break
    if not text:
        return None

    # NUR der letzte Satz. Der erste Entwurf nahm die letzten zwei und schlug damit
    # auf "Ich baue die Pruefung ein. Das Ergebnis: 13 Proben bestanden" an -- ein
    # Bericht ueber bereits Getanes, mit dem Ergebnis dahinter. Folgt auf einen
    # Vorsatz noch ein Satz, ist er fast immer die Ausfuehrung davon.
    saetze = [x.strip() for x in text.replace("\n", " ").split(".") if x.strip()]
    if not saetze:
        return None
    letzter = saetze[-1]
    klein = letzter.lower()
    if any(w in klein for w in VORSATZ) and not any(w in klein for w in WARTET_AUF):
        return letzter[:160]
    return None


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

    # Was ist offen? Gesammelt, nicht einzeln gemeldet -- siehe Kopf: eine
    # Meldung nach `return 0` erreicht niemanden.
    bloecke = []        # je Eintrag: (nag-schluessel, [zeilen])

    # ------------------------------------------------------------ Push (DIR-011)
    #
    # Laeuft BEWUSST unabhaengig von `relevant`. Genau das war die Luecke: Nach
    # dem Commit ist der Arbeitsbaum sauber, `relevant` leer, und der Hook
    # schwieg -- waehrend 23 Commits und 11 Tags lokal liegen blieben.
    offen = ungepusht(root)
    if offen:
        kopf = subprocess.run(["git", "rev-parse", "--short", "HEAD"],
                              capture_output=True, text=True, cwd=root).stdout.strip()
        bloecke.append((f"push-{kopf}-{len(offen)}",
                        ["Noch nicht beim Remote (DIR-011):", ""] + offen + [""]))

    # ------------------------------------- Ankuendigung statt Arbeit (L283)
    #
    # "Als Naechstes fange ich mit X an." -- und dann endet der Turn. Der Nutzer
    # muss daraufhin auffordern weiterzumachen, obwohl nichts auf ihn gewartet hat.
    # Am 05.10.2026 dreimal hintereinander passiert, zuletzt woertlich: "Ich fange
    # damit an", gefolgt von nichts. Seine Frage danach: "Wieso muss ich dich immer
    # wieder auffordern weiterzufahren?"
    #
    # Die Regel dahinter steht in CLAUDE.md: Eine Ankuendigung im Schlusssatz ist
    # eine Zusage fuer DIESEN Turn, nicht fuer den naechsten. Wer nicht weiterarbeiten
    # kann -- weil eine Entscheidung fehlt oder eine Freigabe --, sagt das; wer
    # weiterarbeiten kann, tut es, statt es anzukuendigen.
    #
    # Geprueft wird der LETZTE Textblock des Turns gegen eine knappe Liste von
    # Wendungen. Bewusst eng gehalten: Eine Pruefung mit Fehlalarmen liest niemand
    # mehr (DIR-014), und "als Naechstes steht X an" in einem Bericht UEBER die
    # Planung ist voellig in Ordnung -- deshalb zaehlt nur die Ich-Form mit Vorsatz.
    ankuendigung = letzter_vorsatz(payload.get("transcript_path"))
    if ankuendigung:
        bloecke.append((f"vorsatz-{hashlib.sha1(ankuendigung.encode()).hexdigest()[:8]}", [
            "Der Turn endet mit einer Ankuendigung statt mit der Arbeit:",
            "",
            f"  \u201e{ankuendigung}\u201c",
            "",
            "  Eine Ankuendigung im Schlusssatz ist eine Zusage fuer DIESEN Turn.",
            "  Wenn Du weiterarbeiten kannst, tu es jetzt -- der Nutzer wartet nicht",
            "  auf eine Bestaetigung, sondern auf das Ergebnis.",
            "",
            "  Wenn Du NICHT weiterarbeiten kannst, schreib warum: welche",
            "  Entscheidung, welche Freigabe, welches Geraet fehlt.",
            "",
        ]))

    # ------------------------------------------- Messung ohne Befund (L184)
    messmarke = root / ".git" / "geraet-gemessen"
    if messmarke.exists():
        try:
            befunde = (root / "BEFUNDE.md").stat().st_mtime
            gemessen = messmarke.stat().st_mtime
        except OSError:
            befunde = gemessen = 0
        if gemessen > befunde:
            bloecke.append((f"mess-{int(gemessen)}", [
                "Am Geraet gemessen, seither nichts in BEFUNDE.md eingetragen.",
                "",
                "  Hat die Messung etwas gezeigt, das noch nirgends steht?",
                "  Auch ein widerlegter Verdacht ist ein Befund -- gerade der.",
                "",
                "  Wenn sie nur bestaetigt hat, was schon dokumentiert ist:",
                "  nichts zu tun, diese Meldung ist dann richtig und folgenlos.",
                "",
            ]))

    # ---------------------------------------------------------- Guardrails
    stamp = root / ".git" / "guardrails-stamp"
    if relevant and not (stamp.exists() and stamp.read_text().strip() == state):
        zeilen = ["Die Guardrails sind fuer diesen Aenderungsstand noch nicht gelaufen.",
                  "", "Geaendert:"]
        zeilen += [f"  {ln}" for ln in relevant[:12]]
        if len(relevant) > 12:
            zeilen.append(f"  ... und {len(relevant) - 12} weitere")
        zeilen += ["", "Auszufuehren:  ./tools/guardrails.sh", "",
                   "Danach pruefen, was DIR-004 und DIR-006 verlangen: Versionen der",
                   "geaenderten Komponenten, CHANGELOG.md, und bei geschlossenen Befunden",
                   "den Stand in BEFUNDE.md.", ""]
        bloecke.append((f"guard-{state}", zeilen))

    if not bloecke:
        return 0

    # (2) Pro Grund und Stand nur EINMAL blockieren. Der Schluessel traegt den
    # Grund mit -- sonst verschluckt die erste Meldung alle spaeteren.
    nagdatei = root / ".git" / "guardrails-stop-nag"
    try:
        gesehen = set(nagdatei.read_text().split())
    except OSError:
        gesehen = set()

    neue = [b for b in bloecke if b[0] not in gesehen]
    if not neue:
        return 0
    nagdatei.write_text("\n".join(sorted(gesehen | {b[0] for b in neue})))

    for _, zeilen in neue:
        for z in zeilen:
            out(z)
    out("Ist alles bereits erledigt oder bewusst so gewollt, geht der naechste")
    out("Turn ohne erneute Meldung durch.")
    return 2


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:  # (3) Ein kaputter Hook darf nie blockieren.
        print(f"guardrails-before-stop: uebersprungen ({exc})", file=sys.stderr)
        sys.exit(0)
