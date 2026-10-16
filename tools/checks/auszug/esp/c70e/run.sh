#!/bin/sh
# Pruefstand c70e (Paket 2026-10-15, R2.1, C70 Variante T; AKC.1-AKC.5, AKC.7; design.md 4.4, 8).
# Aufruf: run.sh <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 OK, 1 mindestens ein FEHL, 2 KAPUTT.
# Braucht c65 (basis.h, lwip_attrappe.h, auszug.py; C65_DIR, Vorgabe ../c65). Nur nativ: C70 rechnet nichts
# Breitenabhaengiges - Dateigroessen bis 5000, ETag-Groesse ueber %lu mit (unsigned long), Zaehler uint16_t.
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; B=${2:-neu}
C65=${C65_DIR:-$D/../c65}
[ -f "$C65/basis.h" ] || { echo "KAPUTT: c65/basis.h nicht gefunden ($C65)"; exit 2; }
T=$(mktemp -d "${TMPDIR:-/tmp}/c70e.XXXXXX"); trap 'rm -rf "$T"' EXIT
python3 "$D/auszug_c70e.py" "$Q" "$T" "$C65" > "$T/auszug.log" 2>&1 || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; cat "$T/auszug.log"; exit 2; }
schlecht=0
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format \
   -I"$T" -I"$D" -I"$C65" -DBEZEICHNUNG="\"$B\"" "$D/c70e.cpp" -o "$T/c70e.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -15; exit 2; }
"$T/c70e.bin"; rc=$?
[ $rc -gt 1 ] && { echo "KAPUTT: c70e rc=$rc"; exit 2; }
[ $rc -eq 1 ] && schlecht=1
python3 "$D/c70e_static.py" "$Q" "$B"; rs=$?
[ $rs -gt 1 ] && exit 2
[ $rs -eq 1 ] && schlecht=1
echo "c70e ($B): Exit $schlecht"; exit $schlecht
