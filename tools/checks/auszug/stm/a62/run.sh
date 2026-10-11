#!/bin/sh
# Pruefstand A62 (Paket 2026-10-11, S1.1): Netzzeit, Aera, Totbereich, Bestaetigung, Zeitzone.
# Aufruf: run.sh <src-Verzeichnis> [sabotage]
#   Sabotagen (extract.py): totbereich zehn bestaetigung abstand ueberlauf schnell toleranz zeitzone countdown
# Zwischendateien gehen in ein eigenes Arbeitsverzeichnis unter $TMPDIR, NIE in dieses Verzeichnis.
# Zwei Uebersetzungen: nativ und mit den schnellen Typen des Ziels (pruefstand.h, L256/L290).
# Exit 0 = OK, 1 = FEHL, 2 = kaputt (Uebersetzung/Extraktion).
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
D=$(cd "$(dirname "$0")" && pwd); SRC=$1; SAB=${2:-}
[ -f "$ROOT/tools/checks/pruefstand.h" ] || { echo "a62: pruefstand.h nicht gefunden (ROOT=$ROOT)"; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/a62.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" $SAB || exit 2
python3 "$D/ref.py" > "$W/ref.h" || exit 2
rc=0
if [ -s "$W/statisch" ]; then sed 's/^/  FEHL  statisch: /' "$W/statisch"; rc=1; fi
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN"
  O="-std=c99 -Wall -Wno-unused-variable -Wno-unused-function -Wno-pointer-bool-conversion -I $W/stub -I $W -I $ROOT/tools/checks $Z $(cat "$W/flags")"
  cc $O -c "$W/gm.c" -o "$W/gm.o" || exit 2
  cc $O -c "$D/a62.c" -o "$W/a62.o" || exit 2
  cc "$W/a62.o" "$W/gm.o" -o "$W/a62" || exit 2
  echo "=== A62, $m${SAB:+, Sabotage $SAB} ==="; "$W/a62" || rc=1
done
exit $rc
