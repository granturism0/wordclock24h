#!/bin/sh
# Pruefstand a80 (Paket 2026-10-13, S2.7, A80, AKC.1/AKN.5): Icon bei ausgeschaltetem Display wird verworfen.
# Aufruf: run.sh <src-Verzeichnis>   Zwischendateien unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
# Dynamisch (a80.c, woertliche Auszuege, nativ und mit den Zieltypen) und statisch: die Restore-Bedingung
# um pending_ticker_restore (vier Teilbedingungen und "if (! display_clock_flag)") steht woertlich wie in
# restore-soll.txt, gewonnen aus HEAD 730ad5f (Architektur-Invariante; AKN.5).
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
[ -n "$ROOT" ] || ROOT=$(git rev-parse --show-toplevel 2>/dev/null) || exit 2
D=$(cd "$(dirname "$0")" && pwd); SRC=$1
[ -d "$SRC" ] || { echo "run.sh: src-Verzeichnis fehlt" >&2; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/a80.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" || exit 2
rc=0
echo "=== a80, statisch: Restore-Bedingung ==="
if cmp -s "$W/restore.txt" "$D/restore-soll.txt"; then echo "  OK    Restore-Bedingung woertlich unveraendert"
else echo "  FEHL  Restore-Bedingung weicht ab:"; diff "$D/restore-soll.txt" "$W/restore.txt"; rc=1; fi
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN -include $ROOT/tools/checks/pruefstand.h"
  cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -I "$W" $Z "$D/a80.c" -o "$W/a80" || exit 2
  echo "=== a80, $m ==="; "$W/a80" || rc=1
done
exit $rc
