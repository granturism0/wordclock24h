#!/bin/sh
# Pruefstaende c65m und c65n (E3.4/E3.4a; AKR.4, AKR.5, AKR.6). Aufruf: run_mn.sh <ESP-Quellverzeichnis> <bezeichnung> <m|n>
# Referenz "alt" ist immer ESP 3.2.35 (Tag 8445a10, ALT=<verzeichnis> uebersteuert). Exit 0 OK, 1 FEHL, 2 KAPUTT.
D=$(cd "$(dirname "$0")" && pwd); Q=$1; B=${2:-neu}; W=${3:-m}
T=$(mktemp -d "${TMPDIR:-/tmp}/c65$W.XXXXXX"); trap 'rm -rf "$T"' EXIT
if [ -z "$ALT" ]; then
  # WORDCLOCK_ROOT setzt auszug.sh (S16 laeuft aus einer Temp-Kopie, git -C "$D" findet dort nichts).
  R=${ROOT:-${WORDCLOCK_ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null || git rev-parse --show-toplevel 2>/dev/null)}}
  mkdir -p "$T/alt"
  for f in http.cpp httpclient.cpp base.h ESP-uclock.ino vars.h; do git -C "$R" show 8445a10:ESP8266/ESP-uclock/$f > "$T/alt/$f" || { echo "REFERENZ 8445a10 FEHLT"; exit 2; }; done
  ALT="$T/alt"
fi
python3 "$D/auszug.py" "$Q" "$T" neu_ > "$T/auszug.log" 2>&1 && python3 "$D/auszug.py" "$ALT" "$T" alt_ >> "$T/auszug.log" 2>&1 \
  || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; cat "$T/auszug.log"; exit 2; }
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format -Wno-macro-redefined \
   -I"$T" -I"$D" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/c65$W.cpp" -o "$T/c65$W.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -20; exit 2; }
"$T/c65$W.bin"
