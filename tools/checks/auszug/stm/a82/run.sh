#!/bin/sh
# Pruefstand a82 (Paket 2026-10-13, S2.9, A82, AKA.5): DCF77-Merker nach jedem Schalten von Display oder Ambilight.
# Aufruf: run.sh <src-Verzeichnis>   Zwischendateien unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
# Dynamisch (a82.c, woertliche Auszuege, nativ und mit den Zieltypen) ueber alle Schaltwege (set_display_power
# mit/ohne Ambilight, Setter vom ESP, Ambilight-Timer) und statisch: jede display_set_ambilight_power()-Stelle
# in main.c ruft danach dcf77_update (), Power-Felder und dcf77_enabled werden nirgends sonst geschrieben.
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
D=$(cd "$(dirname "$0")" && pwd); SRC=$1
[ -n "$ROOT" ] || ROOT=$(git -C "$SRC" rev-parse --show-toplevel 2>/dev/null)
[ -n "$ROOT" ] || ROOT=$(git rev-parse --show-toplevel 2>/dev/null) || exit 2
[ -d "$SRC" ] || { echo "run.sh: src-Verzeichnis fehlt" >&2; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/a82.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" || exit 2
rc=0
echo "=== a82, statisch ==="
if [ -s "$W/statisch.txt" ]; then sed 's/^/  FEHL  /' "$W/statisch.txt"; rc=1
else echo "  OK    jede Ambilight-Schaltstelle fuehrt den Merker nach, keine Schreibstelle ausserhalb"; fi
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN -include $ROOT/tools/checks/pruefstand.h"
  cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -I "$W" $Z "$D/a82.c" -o "$W/a82" || exit 2
  echo "=== a82, $m ==="; "$W/a82" || rc=1
done
exit $rc
