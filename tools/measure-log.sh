#!/usr/bin/env bash
# Misst, wie schnell der STM-Logring volllaeuft — und damit, wie weit man beim
# Debuggen zurueckschauen kann.
#
#   ./tools/measure-log.sh            misst 30 s
#   ./tools/measure-log.sh 120        misst 120 s
#
# WARUM ES DIESES SKRIPT GIBT
#
# Der Ring haelt 64 Zeilen. Im Ruhezustand reicht das lange, unter LED-Last nicht:
# Am 03.10.2026 wurde gemessen, dass er waehrend eines Ticker-Overlays nach
# 1,2 Sekunden voll ist — ausschliesslich mit sk6812_refresh-Zeilen (BEFUNDE.md,
# L75). Das heisst: Genau in den Situationen, die fuer die Haenger-Untersuchung
# interessant sind, ist das Logbuch nach gut einer Sekunde ueberschrieben.
#
# Die Zahl, auf die es ankommt, ist deshalb nicht "wie viele Zeilen stehen drin",
# sondern "wie viele Sekunden Vergangenheit halte ich fest". Dieses Skript rechnet
# das aus, statt es schaetzen zu lassen.
#
# AUSSCHLIESSLICH LESEND (DIR-008). Es loest nichts aus und schreibt nichts.
# Insbesondere startet es KEIN Overlay, um die Last zu erzeugen — wer unter Last
# messen will, laesst den Nutzer einen Ticker starten und misst waehrenddessen.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

[ -f tools/device.conf ] && . tools/device.conf
HOST=${DEVICE_HOST:-}
if [ -z "$HOST" ]; then
  echo "DEVICE_HOST fehlt. tools/device.conf anlegen (Vorlage: tools/device.conf.example)." >&2
  exit 2
fi

SECS=${1:-30}
case "$SECS" in ''|*[!0-9]*) echo "Messdauer muss eine Zahl in Sekunden sein." >&2; exit 2;; esac

U="http://$HOST"
printf '=== Logring-Fuellrate auf %s, Messdauer %s s ===\n\n' "$HOST" "$SECS"

A=$(curl -s -m 10 "$U/api/stm32_log" 2>/dev/null)
[ -n "$A" ] || { echo "  Geraet antwortet nicht." >&2; exit 1; }

printf '  messe'
left=$SECS
while [ "$left" -gt 0 ]; do
  step=$(( left > 5 ? 5 : left ))
  sleep "$step"; left=$(( left - step ))
  printf '.'
done
printf '\n\n'

B=$(curl -s -m 10 "$U/api/stm32_log" 2>/dev/null)
[ -n "$B" ] || { echo "  Geraet antwortet nicht mehr." >&2; exit 1; }

A="$A" B="$B" SECS="$SECS" python3 <<'PY'
import json, os, sys

def lines(raw):
    try:
        d = json.loads(raw)
    except ValueError:
        return None
    if isinstance(d, dict):
        for key in ("lines", "log", "stm32_log"):
            if key in d:
                d = d[key]; break
        else:
            return None
    return [str(x) for x in d] if isinstance(d, list) else None

a, b = lines(os.environ["A"]), lines(os.environ["B"])
secs = int(os.environ["SECS"])

if a is None or b is None:
    print("  Antwort hat nicht die erwartete Form — /api/stm32_log von Hand ansehen.")
    sys.exit(1)

# Wie viele Zeilen des ersten Abzugs stehen noch im zweiten? Der Ring schiebt
# vorne raus, also ist das die laengste Ueberlappung "Ende von A == Anfang von B".
overlap = 0
for i in range(len(a)):
    if a[i:] == b[:len(a) - i]:
        overlap = len(a) - i
        break

neu = len(b) - overlap
rate = neu / secs if secs else 0

# Der Ring MELDET nur, was aktuell drinsteht. Nach einem Neustart ist er noch nicht
# umgelaufen, und dann ist das weniger als seine Kapazitaet -- rechnet man die
# Reichweite gegen die Fuellung, kommt systematisch zu wenig heraus. Am 03.10.2026
# gemessen: 22 Zeilen im frisch gestarteten Ring ergaben 73 s statt der realen 213.
RING_KAPAZITAET = 64          # STM32_LOG_LINES im ESP
kap = max(len(b), RING_KAPAZITAET)
print(f"  Ring haelt            {len(b)} von {RING_KAPAZITAET} Zeilen"
      + ("  (noch nicht umgelaufen)" if len(b) < RING_KAPAZITAET else ""))
print(f"  nach {secs} s noch da      {overlap} der urspruenglichen Zeilen")
print(f"  neu dazugekommen      {neu}")
print()

if neu == 0:
    print(f"  Rate                  unter {1/secs:.3f} Zeilen/s — in diesem Fenster kam nichts dazu.")
    print("  Reichweite            laenger als die Messdauer; fuer eine Zahl laenger messen.")
elif overlap == 0:
    print(f"  Rate                  MINDESTENS {rate:.2f} Zeilen/s")
    print(f"  Reichweite            HOECHSTENS {kap/rate:.1f} s — der Ring ist in diesem Fenster")
    print("                        vollstaendig umgelaufen, die echte Rate kann hoeher sein.")
    print("                        Kuerzer messen, sonst ist die Zahl eine Untergrenze.")
else:
    print(f"  Rate                  {rate:.2f} Zeilen/s")
    print(f"  Reichweite            {kap/rate:.0f} s = {kap/rate/60:.1f} min Vergangenheit")
    print()
    print("  Das ist die Zahl, auf die es ankommt: so weit reicht das Logbuch zurueck,")
    print("  wenn etwas passiert. Unter LED-Last liegt sie deutlich niedriger — am")
    print("  03.10.2026 gemessen: 1,2 s waehrend eines Ticker-Overlays (BEFUNDE.md, L75).")

# ---------------------------------------------------------------- Zeilenklassen
#
# WER den Ring fuellt, ist wichtiger als WIE SCHNELL. Eine halbierte Rate sagt
# nichts, wenn stattdessen eine andere Quelle uebernommen hat -- und genau so
# saehe eine erfolgreiche Entlastung aus, wenn man nur auf die Gesamtzahl schaut.
# Deshalb die drei haeufigsten Zeilenanfaenge mit Anzahl.
import re
from collections import Counter

def klasse(zeile):
    # Zeitstempel und wechselnde Zahlen raus, damit gleichartige Zeilen
    # zusammenfallen: "sk6812_refresh: 1234" und "sk6812_refresh: 5678" sind
    # dieselbe Klasse. Danach die ersten Woerter als Kennung.
    z = re.sub(r"^\s*\S*\d{2}:\d{2}:\d{2}\S*\s*", "", zeile)
    z = re.sub(r"\b\d+\b", "#", z)
    return (" ".join(z.split()[:4]) or "(leer)")[:58]

zaehler = Counter(klasse(z) for z in b if z.strip())
if zaehler:
    print()
    print("  Wer den Ring fuellt (haeufigste Zeilenarten im letzten Abzug):")
    for text, anzahl in zaehler.most_common(3):
        anteil = 100 * anzahl / len(b)
        print(f"    {anzahl:>3}x  {anteil:>4.0f} %  {text}")
    if len(zaehler) > 3:
        print(f"    dazu {len(zaehler) - 3} weitere Zeilenart(en)")
PY
