#!/bin/sh
# Pruefstand c71e (Paket 2026-10-15, R1b, L7; AKS.1; design.md 7, 8).
# Aufruf: run.sh <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 OK, 1 mindestens ein FEHL, 2 KAPUTT.
# Braucht basis.h aus c65 (C65_DIR, Vorgabe ../c65). Nur nativ: L7 rechnet nichts Breitenabhaengiges (Laengen int,
# Zeit nur als Differenz kleiner Werte).
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; B=${2:-neu}
C65=${C65_DIR:-$D/../c65}; R=${ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null)}
[ -f "$C65/basis.h" ] || { echo "KAPUTT: c65/basis.h nicht gefunden ($C65)"; exit 2; }
T=$(mktemp -d "${TMPDIR:-/tmp}/c71e.XXXXXX"); trap 'rm -rf "$T"' EXIT
python3 "$D/auszug_c71e.py" "$Q" "$T" > "$T/auszug.log" 2>&1 || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; cat "$T/auszug.log"; exit 2; }
schlecht=0
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format \
   -I"$T" -I"$D" -I"$C65" -I"$Q" -DBEZEICHNUNG="\"$B\"" "$D/c71e.cpp" -o "$T/c71e.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -15; exit 2; }
"$T/c71e.bin"; rc=$?
[ $rc -gt 1 ] && { echo "KAPUTT: c71e rc=$rc"; exit 2; }
[ $rc -eq 1 ] && schlecht=1
python3 "$D/c71e_static.py" "$Q" "$B"; rs=$?
[ $rs -gt 1 ] && exit 2
[ $rs -eq 1 ] && schlecht=1
echo "c71e ($B): Exit $schlecht"; exit $schlecht
