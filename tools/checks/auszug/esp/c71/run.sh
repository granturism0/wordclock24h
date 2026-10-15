#!/bin/sh
# Pruefstand c71i (Paket 2026-10-15, R1.3, I-6; AKI.3; design.md 3.3, 8).
# Aufruf: run.sh <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 OK, 1 mindestens ein FEHL, 2 KAPUTT.
# Baut auf dem Abgleich-Harness von c65 auf (upd_harness.h, basis.h, auszug.py; C65_DIR, Vorgabe ../c65) - der
# R1.3-Nachzug dort (getHeapStats, http_accept_count, httpclient_connects_get, tcp_now_tcp_field) muss angewandt sein.
# Nur nativ: Die I-6-Rechnungen haben feste Breiten (uint16_t-Kappung, uint16_t-Differenz fuer acc=); up= ist
# millis () / 1000 und wird nicht ueber den Umlauf geprueft.
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; B=${2:-neu}
C65=${C65_DIR:-$D/../c65}
[ -f "$C65/upd_harness.h" ] && [ -f "$C65/auszug.py" ] || { echo "KAPUTT: c65 nicht gefunden ($C65)"; exit 2; }
grep -q 'getHeapStats' "$C65/basis.h" || { echo "KAPUTT: R1.3-Nachzug in c65/basis.h fehlt"; exit 2; }
T=$(mktemp -d "${TMPDIR:-/tmp}/c71i.XXXXXX"); trap 'rm -rf "$T"' EXIT
# Der Harness braucht im Namensraum alt den Stand 3.2.35 (wie run_mn.sh, Tag-Commit 8445a10; ALT=<verzeichnis> uebersteuert).
if [ -z "$ALT" ]; then
  R=${ROOT:-${WORDCLOCK_ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null || git rev-parse --show-toplevel 2>/dev/null)}}
  mkdir -p "$T/alt"
  for f in http.cpp httpclient.cpp base.h ESP-uclock.ino vars.h; do git -C "$R" show 8445a10:ESP8266/ESP-uclock/$f > "$T/alt/$f" || { echo "REFERENZ 8445a10 FEHLT"; exit 2; }; done
  ALT="$T/alt"
fi
python3 "$D/auszug_c71.py" "$Q" "$T" "$C65" "$ALT" > "$T/auszug.log" 2>&1 || { echo "AUSZUG FEHLGESCHLAGEN ($B)"; cat "$T/auszug.log"; exit 2; }
cc -x c++ -std=c++17 -O1 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format -Wno-macro-redefined \
   -I"$T" -I"$D" -I"$C65" -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/c71i.cpp" -o "$T/c71i.bin" -lc++ 2>"$T/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$T/cc.log" | head -15; exit 2; }
"$T/c71i.bin"; rc=$?
[ $rc -gt 1 ] && { echo "KAPUTT: c71i rc=$rc"; exit 2; }
python3 "$D/c71i_static.py" "$Q" "$B"; rs=$?
[ $rs -gt 1 ] && exit 2
s=0; [ $rc -eq 1 ] && s=1; [ $rs -eq 1 ] && s=1
echo "c71i ($B): Exit $s"; exit $s
