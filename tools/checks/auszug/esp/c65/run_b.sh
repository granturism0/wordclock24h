#!/bin/sh
# Pruefstand c65b (E3.2; AKR.2). Aufruf: run_b.sh <ESP-Quellverzeichnis> <bezeichnung>. Exit 0 OK, 1 FEHL, 2 KAPUTT.
D=$(cd "$(dirname "$0")" && pwd); Q=$1; B=${2:-neu}
T=$(mktemp -d "${TMPDIR:-/tmp}/c65b.XXXXXX"); trap 'rm -rf "$T"' EXIT
python3 "$D/auszug.py" "$Q" "$T" > "$T/auszug.log" 2>&1 || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; cat "$T/auszug.log"; exit 2; }
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format \
   -I"$T" -I"$D" -DBEZEICHNUNG="\"$B\"" "$D/c65b.cpp" -o "$T/c65b.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -15; exit 2; }
"$T/c65b.bin"
