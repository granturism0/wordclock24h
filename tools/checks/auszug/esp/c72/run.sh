#!/bin/sh
# Pruefstand c72i (Paket 2026-10-15, R1.1, I-5; AKI.1, AKI.2; design.md 3.1, 3.2, 8).
# Aufruf: run.sh <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 OK, 1 mindestens ein FEHL, 2 KAPUTT.
# Uebersetzt zweimal (nativ und mit den schnellen Typen des Ziels, pruefstand.h), dazu die statische Pruefung.
# Braucht den Auszug und basis.h von c65 (C65_DIR, Vorgabe ../c65) und pruefstand.h (C72_PRUEFSTAND_H_DIR, Vorgabe
# <Repo>/tools/checks).
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; B=${2:-neu}
C65=${C65_DIR:-$D/../c65}; R=${ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null)}
PH=${C72_PRUEFSTAND_H_DIR:-$R/tools/checks}
[ -f "$C65/basis.h" ] && [ -f "$C65/auszug.py" ] || { echo "KAPUTT: c65 nicht gefunden ($C65)"; exit 2; }
[ -f "$PH/pruefstand.h" ] || { echo "KAPUTT: pruefstand.h nicht gefunden ($PH)"; exit 2; }
T=$(mktemp -d "${TMPDIR:-/tmp}/c72i.XXXXXX"); trap 'rm -rf "$T"' EXIT
python3 "$D/auszug_c72.py" "$Q" "$T" "$C65" > "$T/auszug.log" 2>&1 || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; cat "$T/auszug.log"; exit 2; }
schlecht=0
for v in nativ ziel; do
  X=""; [ $v = ziel ] && X="-DC72_ZIEL"
  cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format $X \
     -I"$T" -I"$D" -I"$C65" -I"$PH" -DBEZEICHNUNG="\"$B\"" "$D/c72i.cpp" -o "$T/c72i_$v.bin" -lc++ 2>"$T/cc_$v.log" \
    || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B, $v):"; grep error "$T/cc_$v.log" | head -15; exit 2; }
  "$T/c72i_$v.bin"; rc=$?
  [ $rc -gt 1 ] && { echo "KAPUTT: c72i ($v) rc=$rc"; exit 2; }
  [ $rc -eq 1 ] && schlecht=1
done
python3 "$D/c72i_static.py" "$Q" "$B"; rs=$?
[ $rs -gt 1 ] && exit 2
[ $rs -eq 1 ] && schlecht=1
echo "c72i ($B): Exit $schlecht"; exit $schlecht
