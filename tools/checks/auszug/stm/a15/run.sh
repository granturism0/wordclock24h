#!/bin/sh
# Pruefstand a15 (A15, Paket 2026-10-13, S1.1, AKI.1). Aufruf: run.sh <src-Verzeichnis> [sabotage]
#   Sabotagen (extract.py): txeie rxneie ore-nach-dr dr-nicht-bei-ore txe-nicht-aus
# Statisch: keine USART_GetITStatus/ClearITPendingBit/ReceiveData/SendData/ITConfig in der ISR, SR genau
# einmal gelesen, Bitlagen F103 == F411. Dynamisch: alte gegen neue ISR an der Registerattrappe.
# Zwei Ringgroessen (8/4 fuer "Ring voll", 1024/128 wie die ESP-Bruecke), je nativ und mit den
# schnellen Typen des Ziels (pruefstand.h). Zwischendateien in einem eigenen Verzeichnis unter $TMPDIR.
# Exit 0 = OK, 1 = FEHL, 2 = kaputt.
D=$(cd "$(dirname "$0")" && pwd); SRC=$1; SAB=${2:-}
ROOT=${ROOT:-$(git -C "$(dirname "$0")" rev-parse --show-toplevel 2>/dev/null)}
[ -f "$ROOT/tools/checks/pruefstand.h" ] || { echo "a15: pruefstand.h nicht gefunden (ROOT=$ROOT)"; exit 2; }
[ -f "$SRC/uart/uart-driver.h" ] || { echo "a15: $SRC/uart/uart-driver.h fehlt"; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/a15.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" "$ROOT" $SAB || exit 2
rc=0
if [ -s "$W/statisch" ]; then sed 's/^/  FEHL  statisch: /' "$W/statisch"; rc=1; else echo "  OK    statisch: keine SPL-Aufrufe in der ISR, SR einmal gelesen, Bitlagen F103 == F411"; fi
for ring in "8 4" "1024 128"; do set -- $ring
  for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN"
    cc -std=c99 -Wall -Wno-unused-function -Wno-unused-variable -I "$W" -I "$ROOT/tools/checks" $Z -DRXN=$1 -DTXN=$2 $(cat "$W/flags") "$D/a15.c" -o "$W/a15" || exit 2
    echo "=== a15, $m, Ring $1/$2${SAB:+, Sabotage $SAB} ==="
    "$W/a15" > "$W/out.txt"; r=$?
    grep -a "FEHL\|Faelle\|Typbreiten\|abgedeckt\|Lesungen" "$W/out.txt"
    [ $r -ne 0 ] && rc=1
  done
done
exit $rc
