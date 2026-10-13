#!/bin/sh
# Pruefstand A70 (Paket 2026-10-13, S2.11): Randfaelle beim Nachholen des Nachtfensters (AKN.1-AKN.5).
# Aufruf: run.sh <src-Verzeichnis>
# Zwischendateien in einem eigenen Arbeitsverzeichnis unter $TMPDIR. Zwei Uebersetzungen: nativ und mit den
# schnellen Typen des Ziels (pruefstand.h). Statisch: Tickerstarts (AKN.3), Restore-Bedingung (AKN.5).
# Exit 0 = OK, 1 = FEHL, 2 = kaputt (Uebersetzung/Extraktion).
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
[ -n "$ROOT" ] || ROOT=$(git rev-parse --show-toplevel 2>/dev/null)
D=$(cd "$(dirname "$0")" && pwd); SRC=$1
[ -d "$SRC" ] || { echo "a70: src-Verzeichnis fehlt" >&2; exit 2; }
[ -f "$ROOT/tools/checks/pruefstand.h" ] || { echo "a70: pruefstand.h nicht gefunden (ROOT=$ROOT)"; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/a70.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" || exit 2
cp "$D/night-alt.c" "$W/" || exit 2
rc=0
echo "=== a70, statisch ==="
sed 's/^/  info  /' "$W/info"
if cmp -s "$W/restore.txt" "$D/restore-soll.txt"; then echo "  OK    AKN.5: Restore-Bedingung woertlich unveraendert"
else echo "  FEHL  AKN.5: Restore-Bedingung weicht ab:"; diff "$D/restore-soll.txt" "$W/restore.txt"; rc=1; fi
if [ -s "$W/statisch" ]; then sed 's/^/  FEHL  /' "$W/statisch"; rc=1; else echo "  OK    AKN.3 Tickerstarts, AKN.5 Innenzweig, TIME-Zweig"; fi
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN"
  O="-std=c99 -Wall -Wno-unused-variable -Wno-unused-function -Wno-pointer-bool-conversion -Wno-unused-but-set-variable -I $W/stub -I $W -I $ROOT/tools/checks $Z -DHAVE_APPLY -DHAVE_A69"
  cc $O "$D/a70.c" -o "$W/a70" || exit 2
  echo "=== A70, $m ==="; "$W/a70" || rc=1
done
exit $rc
