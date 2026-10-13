#!/bin/sh
# Pruefstand l55e (Paket 2026-10-13, E2.1, AKL.2): ldr_min_set/ldr_max_set warten auf "RPCR xx r", lesen dabei die
# Bruecke ueber stm_bridge_poll () (Quittungen gehen weiter hinaus), Sperre, Frist 2'000 ms, Kennungen 10/7/11,
# gedrosselte Zeile "- rpc keine Antwort #n idx=xx", kein Echo auf RPCR (F2).
# Aufruf: run.sh <verzeichnis ESP-uclock> <bezeichnung>    Exit 0 = OK, 1 = FEHL, 2 = KAPUTT
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; B=${2:-$1}
R=${ROOT:-$(git -C "$Q" rev-parse --show-toplevel 2>/dev/null)}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
python3 "$D/gen.py" "$Q/ESP-uclock.ino" "$Q/http.cpp" "$T/l55e_code.inc" || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; exit 2; }
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$T" -I"$D" -I"$R/tools/checks" \
   -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/l55e.cpp" -o "$T/l55e.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -15; exit 2; }
"$T/l55e.bin"
