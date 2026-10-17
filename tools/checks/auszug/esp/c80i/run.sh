#!/bin/sh
# Pruefstand c80i (Paket 2026-10-17-c80, Task 1.3; AKI.2-AKI.8, AKI.13 (a), AKI.14 (a); design.md 4.8 samt N1/N2).
# Aufruf: run.sh <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 OK, 1 mindestens ein FEHL, 2 KAPUTT.
# Nur nativ: Das Instrument rechnet ausschliesslich mit festen Breiten (uint16_t, uint32_t, int32_t), millis () ist im
# Pruefstand uint32_t wie auf dem Geraet. Teile: c80i (Haupt), c80i_it (Waechter it ohne ord), c80i_aus (ESP_HEAP_MAP 0),
# c80i_static.py (Quelltext).
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; B=${2:-neu}
T=$(mktemp -d "${TMPDIR:-/tmp}/c80i.XXXXXX"); trap 'case "$T" in "${TMPDIR:-/tmp}"*c80i.*) rm -rf "${T:?}";; esac' EXIT
schlecht=0
if python3 "$D/auszug_c80i.py" "$Q" "$T" > "$T/auszug.log" 2>&1; then
  CC="cc -x c++ -std=c++17 -O1 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-format-truncation -Wno-missing-field-initializers -I$T -DBEZEICHNUNG=\"$B\""
  $CC "$D/c80i.cpp" -o "$T/c80i.bin" -lc++ 2>"$T/cc.log" || { echo "UEBERSETZUNG FEHLGESCHLAGEN c80i ($B):"; grep -E 'error' "$T/cc.log" | head -15; echo "FEHLER c80i: der ausgeschnittene Stand uebersetzt nicht"; schlecht=1; }
  if [ -x "$T/c80i.bin" ]; then "$T/c80i.bin"; rc=$?; [ $rc -gt 1 ] && { echo "KAPUTT: c80i rc=$rc"; exit 2; }; [ $rc -eq 1 ] && schlecht=1; fi
  $CC "$D/c80i_it.cpp" -o "$T/c80i_it.bin" 2>"$T/cc_it.log" || { echo "FEHLER c80i_it: uebersetzt nicht"; grep error "$T/cc_it.log" | head -5; schlecht=1; }
  if [ -x "$T/c80i_it.bin" ]; then "$T/c80i_it.bin"; rc=$?; [ $rc -ne 0 ] && { [ $rc -gt 1 ] && echo "FEHLER Fall 6 it: Gang ohne ord und it endet nicht (rc=$rc)"; schlecht=1; }; fi
  sed 's/^#define ESP_HEAP_MAP  *1 /#define ESP_HEAP_MAP                0 /' "$T/heapkarte.h" > "$T/heapkarte_aus.h"
  $CC "$D/c80i_aus.cpp" -o "$T/c80i_aus.bin" -lc++ 2>"$T/cc_aus.log" || { echo "FEHLER c80i_aus: loop () mit ESP_HEAP_MAP 0 uebersetzt nicht"; grep error "$T/cc_aus.log" | head -5; schlecht=1; }
  if [ -x "$T/c80i_aus.bin" ]; then "$T/c80i_aus.bin" || schlecht=1; fi
else
  cat "$T/auszug.log"; schlecht=1
fi
python3 "$D/c80i_static.py" "$Q" "$B"; rs=$?
[ $rs -gt 1 ] && exit 2
[ $rs -eq 1 ] && schlecht=1
echo "c80i ($B): Exit $schlecht"; exit $schlecht
