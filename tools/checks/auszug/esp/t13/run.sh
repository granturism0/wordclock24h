#!/bin/sh
# Pruefstand E.2 (Teil C, Paket 2026-10-09). Aufruf: run.sh <verzeichnis mit http.cpp> <bezeichnung>
# http_log_request () aus dem ECHTEN http.cpp: Methode und Pfad, Querystring => "?...", AKE.8.
# Gegen den Ausgangsstand NICHT uebersetzbar (die Funktion fehlt dort) - kein Fehlschlag im Sinn
# von DIR-014; die Gegenprobe traegt die statische Pruefung (static.sh).
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
python3 "$S/extract.py" func "$Q/http.cpp" http_log_request > "$D/echo_code.inc" 2>"$D/extract.log"
[ -s "$D/echo_code.inc" ] || { echo "AUSZUG LEER ($B):"; cat "$D/extract.log"; exit 2; }
cc -x c++ -std=c++17 -Wall -I"$D" -DBEZEICHNUNG="\"$B\"" "$D/t13.cpp" -o "$D/t13.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t13.bin"
