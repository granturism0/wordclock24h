#!/usr/bin/env bash
# Vergleicht zwei Rohabzuege feldweise (Phase 9 aus TESTPLAN-PWA.md).
#
#   ./tools/diff-snapshot.sh referenz 2026-10-02-0312
#   ./tools/diff-snapshot.sh referenz               gegen den juengsten Abzug
#
# Das ist der Schritt, der aus einem Testdurchlauf einen Nachweis macht. "Sieht wieder
# normal aus" ist keine Wiederherstellung -- ein feldweiser Vergleich gegen den Stand
# VOR dem Durchlauf schon.
#
# Verglichen wird Feld fuer Feld, nicht Datei fuer Datei: Ein diff ueber settings_xml
# meldet sonst eine einzige lange Zeile und sagt nichts darueber, WELCHE Variable sich
# geaendert hat.
#
# WAS DIESER VERGLEICH NICHT LEISTET -- und das ist teuer geworden
#
# Er findet ausschliesslich, was der EIGENE Lauf veraendert hat. Der Referenzabzug
# entsteht zu Beginn desselben Durchlaufs; was ein FRUEHERER Lauf hinterlassen hat,
# steht dort bereits drin und faellt damit heraus.
#
# Am 04.10.2026 belegt: Ein Timer, der die Uhr jede Nacht eine Stunde zu frueh
# ausschaltete, stand seit dem 03.10. in ALLEN 15 nachfolgenden Abzuegen -- und kein
# einziger Abschlussvergleich hat ihn gemeldet. Dazu fuenf weitere Altlasten
# derselben Art, darunter die ueberschriebene Dimmkurve des Nutzers (BEFUNDE.md,
# L81 und L244). Jeder dieser Laeufe meldete "keine Konfigurationsabweichung", und
# jeder hatte damit recht -- gemessen am eigenen Anfang.
#
# Deshalb gibt es seit dem 04.10.2026 die zweite Betriebsart --soll: Sie vergleicht
# gegen einen FESTEN Sollzustand statt gegen den Laufanfang. Welcher Abzug der Soll
# ist, entscheidet der Nutzer -- das Skript kann es nicht wissen, und zu raten waere
# hier schlimmer als zu fragen.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

SNAP=tools/snapshots
SOLL=0
if [ "${1:-}" = "--soll" ]; then
  SOLL=1
  shift
fi
A=${1:-}
B=${2:-}

if [ -z "$A" ]; then
  echo "Aufruf: ./tools/diff-snapshot.sh <abzug-a> [abzug-b]" >&2
  echo "Vorhanden:" >&2
  ls -1 "$SNAP" 2>/dev/null | sed 's/^/  /' >&2
  exit 2
fi
if [ -z "$B" ]; then
  B=$(ls -1t "$SNAP" 2>/dev/null | grep -v "^$A\$" | head -1)
  [ -z "$B" ] && { echo "Kein zweiter Abzug vorhanden." >&2; exit 2; }
fi

[ -d "$SNAP/$A" ] || { echo "Abzug fehlt: $SNAP/$A" >&2; exit 2; }
[ -d "$SNAP/$B" ] || { echo "Abzug fehlt: $SNAP/$B" >&2; exit 2; }

export DIFF_SOLL="$SOLL"
exec python3 - "$SNAP/$A" "$SNAP/$B" <<'PY'
import json, os, re, sys

a_dir, b_dir = sys.argv[1], sys.argv[2]

# stm32_log ist ein Ringpuffer und aendert sich im Sekundentakt -- er wuerde jeden
# Vergleich zumuellen. Er gehoert in die Logbuch-Durchsicht, nicht hierher.
SKIP_FILES = {"stm32_log.txt", "_meta.txt"}

# Betriebsart --soll: Vergleich gegen einen festen Sollzustand statt gegen den
# Laufanfang. Dann interessieren ausschliesslich KONFIGURATIONSfelder -- Uhrzeit,
# Betriebszeit, Temperatur, Heap und Sensorrohwerte aendern sich zwangslaeufig und
# wuerden die Ausgabe zumuellen, bis niemand mehr hinsieht.
#
# Die Liste ist bewusst aufgezaehlt und nicht gemustert: Ein zu weites Muster
# verschluckt irgendwann ein Konfigurationsfeld, und das waere genau der Fehler,
# gegen den diese Betriebsart gebaut ist.
SOLL_MODE = os.environ.get("DIFF_SOLL") == "1"
FLUECHTIG = {
    "numvar[idx=16].value",   # LDR-Rohwert
    "numvar[idx=20].value",   # RTC-Temperatur roh
    "numvar[idx=21].value",   # RTC-Temperatur
    "numvar[idx=23].value",   # DS18xx-Temperatur
    "numvar[idx=49].value",   # RTC-Temperatur in halben Grad mit Vorzeichen (A5, ab ESP 3.2.25)
    "numvar[idx=47].value",   # Betriebszeit
    "numvar[idx=48].value",   # Betriebszeit, oberes Wort (faellt bei jedem STM-Neustart auf 0)
    "free_heap", "max_free_block", "heap_frag",
    "no_request_aborts", "no_request_timeouts",
    "write_lost_bytes", "write_lost_blocks", "update_cache_hits",
    "write_lost_gone", "write_lost_failed",                     # C25, ab der F-Firmware
    "update_server_down_count",
}
FLUECHTIG_PRAEFIX = ("tmvar[idx=0].",)   # Uhrzeit vollstaendig


def ist_fluechtig(schluessel):
    if schluessel in FLUECHTIG:
        return True
    return any(schluessel.startswith(pre) for pre in FLUECHTIG_PRAEFIX)


def flatten(prefix, value, out):
    if isinstance(value, dict):
        for k in sorted(value):
            flatten(f"{prefix}.{k}" if prefix else k, value[k], out)
    elif isinstance(value, list):
        out[f"{prefix}[]"] = json.dumps(value, ensure_ascii=False, sort_keys=True)
    else:
        out[prefix] = value


def read_fields(directory, name):
    path = os.path.join(directory, name)
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            text = fh.read()
    except OSError:
        return None

    stem = name[:-4]
    out = {}

    if stem == "settings_xml":
        # Jede Variable einzeln, damit der Bericht die Nummer nennt statt "Datei anders".
        #
        # Frueher wurden NUR numvar und strvar verglichen. Das war ein Loch genau dort,
        # wo ein Testdurchlauf arbeitet: Farben, Dimmkurven, Overlays, Timer, Alarme und
        # die Profil-Flags blieben unbesehen. Ein stehengebliebener Wert haette den
        # Abschlussvergleich mit "Kein Unterschied" passiert -- und der ist der Schritt,
        # der aus einem Durchlauf einen Nachweis macht. Gefunden im Durchlauf vom
        # 03.10.2026, der neun solcher Felder angefasst hat.
        #
        # Jetzt wird JEDES Element mit allen seinen Attributen erfasst, ohne Liste der
        # bekannten Typen: Kommt in einer kuenftigen Firmware ein Element dazu, ist es
        # automatisch dabei.
        for tag, attrs in re.findall(r'<([a-z0-9]+)\s+([^>]*?)/?>', text):
            pairs = dict(re.findall(r'([a-z0-9_]+)="([^"]*)"', attrs))
            # Der Index gehoert in den Schluessel, nicht in den Wert -- sonst
            # verschiebt sich beim Vergleich alles, wenn ein Eintrag wegfaellt.
            key = tag
            for id_attr in ("idx", "var"):
                if id_attr in pairs:
                    key += f"[{id_attr}={pairs.pop(id_attr)}]"
            for name, value in sorted(pairs.items()):
                out[f"{key}.{name}"] = value
        if not out:
            out["(roh)"] = text.strip()
        return out

    stripped = text.strip()
    if stripped[:1] in "{[":
        try:
            flatten("", json.loads(stripped), out)
            return out
        except ValueError:
            pass

    out["(roh)"] = stripped
    return out


names = sorted(
    {n for n in os.listdir(a_dir) if n.endswith(".txt")} |
    {n for n in os.listdir(b_dir) if n.endswith(".txt")}
)

print(f"=== Vergleich ===")
for label, d in (("A", a_dir), ("B", b_dir)):
    meta = os.path.join(d, "_meta.txt")
    stamp = ""
    if os.path.exists(meta):
        for line in open(meta, encoding="utf-8"):
            if line.startswith("zeitpunkt="):
                stamp = line.split("=", 1)[1].strip()
    print(f"  {label}: {d}   {stamp}")
print()

total = 0
for name in names:
    if name in SKIP_FILES:
        continue
    fa, fb = read_fields(a_dir, name), read_fields(b_dir, name)
    if fa is None or fb is None:
        print(f"  {name[:-4]}: nur in einem Abzug vorhanden")
        total += 1
        continue

    diffs = []
    for key in sorted(set(fa) | set(fb)):
        va, vb = fa.get(key, "<fehlt>"), fb.get(key, "<fehlt>")
        if str(va) != str(vb):
            if SOLL_MODE and ist_fluechtig(key):
                continue        # aendert sich zwangslaeufig, siehe FLUECHTIG
            diffs.append((key, va, vb))

    if diffs:
        print(f"  {name[:-4]}  ({len(diffs)} Abweichung{'en' if len(diffs) != 1 else ''})")
        for key, va, vb in diffs[:40]:
            print(f"      {key:<22} A={str(va)[:46]!r}  B={str(vb)[:46]!r}")
        if len(diffs) > 40:
            print(f"      ... und {len(diffs) - 40} weitere")
        total += len(diffs)

print()
if total == 0:
    if SOLL_MODE:
        print("=== Kein Unterschied in den Konfigurationsfeldern. ===")
        print("    Fluechtige Felder (Uhrzeit, Temperatur, Heap, Betriebszeit) sind ausgeblendet.")
    else:
        print("=== Kein Unterschied. Der Zustand ist wiederhergestellt. ===")
    sys.exit(0)

if SOLL_MODE:
    print(f"=== {total} Abweichung(en) vom SOLLZUSTAND. ===")
    print("    Das sind keine Laufspuren, sondern Abweichungen vom gewollten Zustand des")
    print("    Geraets -- einschliesslich Altlasten frueherer Durchlaeufe (BEFUNDE.md, L81).")
else:
    print(f"=== {total} Abweichung(en). Jede ist entweder erklaert oder ein Befund. ===")
sys.exit(1)
PY
