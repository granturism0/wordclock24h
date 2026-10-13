#!/bin/sh
# Pruefstand c8 (Paket 2026-10-13, E2.2, AKK.1): Kommandopuffer 260, Logzeilen ueber 120 Zeichen als Folgeeintraege
# ('~' am Ende des ersten, '~' am Anfang jedes weiteren), Ring-RAM unveraendert 32 x 121. Echter Code: Ring aus
# ESP-uclock.ino (gen.py) und die Bruecke samt LOG-Zweig (../l55e/gen.py).
# Aufruf: run.sh <verzeichnis ESP-uclock> <bezeichnung>    Exit 0 = OK, 1 = FEHL, 2 = KAPUTT
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; B=${2:-$1}
R=${ROOT:-$(git -C "$Q" rev-parse --show-toplevel 2>/dev/null)}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
python3 "$D/gen.py" "$Q/ESP-uclock.ino" "$T/ring_code.inc" || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; exit 2; }
python3 "$D/../l55e/gen.py" "$Q/ESP-uclock.ino" "$Q/http.cpp" "$T/l55e_code.inc" >/dev/null || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; exit 2; }
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -Wno-macro-redefined -I"$T" -I"$D" -I"$R/tools/checks" \
   -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/c8.cpp" -o "$T/c8.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -15; exit 2; }
"$T/c8.bin"
