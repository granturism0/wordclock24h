#!/bin/sh
# Pruefstand L408 (BEFUNDE L408, ESP-OTA 13.10.2026). Aufruf: run.sh <src-Verzeichnis>
#   (a) Dimmtabellen senden 16 Elemente (Elementzahl statt Bytezahl), dazu statisch alle Aufrufer
#   (b) nach is_online = 0: keine Sendung, keine Wartezeit, nichts gezaehlt, nichts vorgemerkt
#   (c) online: Verhalten unveraendert
# Nativ und mit PRUEFSTAND_SCHNELLE_TYPEN (uint_fast8_t 32 Bit wie arm-none-eabi). Zwischendateien
# unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
D=$(cd "$(dirname "$0")" && pwd); SRC=$1
ROOT=${ROOT:-$(git rev-parse --show-toplevel 2>/dev/null)}
[ -f "$ROOT/tools/checks/pruefstand.h" ] || { echo "l408: ROOT unbekannt ($ROOT), ROOT=... setzen"; exit 2; }
[ -f "$SRC/vars/vars.c" ] || { echo "l408: $SRC ist kein src-Verzeichnis"; exit 2; }
W=$(mktemp -d "${TMPDIR:-/tmp}/l408.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$ROOT/tools/checks/auszug/stm/extract.py" "$SRC" "$W/x-buf.c" > /dev/null || exit 2
MB=$(grep -a '^#define MAX_BRIGHTNESS ' "$SRC/display/display.h" | tr -d '\r' | awk '{print $3}')
DT=$(grep -a '^#define DATE_TICKER_FORMAT_LEN ' "$SRC/display/display.h" | tr -d '\r' | awk '{print $3}')
[ -n "$MB" ] && [ -n "$DT" ] || { echo "l408: MAX_BRIGHTNESS/DATE_TICKER_FORMAT_LEN nicht gefunden"; exit 2; }
echo "=== L408 statisch ==="
python3 "$D/extract.py" "$SRC" "$W/x-dim.c"; s=$?
[ $s -eq 2 ] && exit 2
rc=$s
for m in nativ ziel; do Z=""; [ $m = ziel ] && Z="-DPRUEFSTAND_SCHNELLE_TYPEN"
  CF="-std=c99 -Wall -Wno-unused-function -Wno-unused-variable -Wno-tautological-constant-out-of-range-compare $Z -I $SRC/esp8266 -I $SRC/vars -I $ROOT/tools/checks"
  cc $CF -DMAX_BRIGHTNESS=$MB -DDATE_TICKER_FORMAT_LEN=$DT -DSNIPPET="\"$W/x-dim.c\"" "$D/l408a.c" -o "$W/a" || exit 2
  cc $CF -DSNIPPET="\"$W/x-buf.c\"" "$D/l408b.c" -o "$W/b" || exit 2
  echo "=== L408, $m ==="
  "$W/a" || rc=1
  "$W/b" || rc=1
done
exit $rc
