#!/bin/sh
# Statische Pruefung E.2 (AKE.8): kein Serial.print*/write in http.cpp mit sRequest oder sParam.
# Aufruf: static.sh <verzeichnis mit http.cpp> <bezeichnung>. Exit 1 bei Treffer (Treffer werden genannt).
Q=$1; B=$2
T=$(grep -a -n -E 'Serial\.(print|println|write|printf)[[:space:]]*\([^;]*\b(sRequest|sParam)\b' "$Q/http.cpp")
N=$(grep -a -c -E 'Serial\.(print|println|write|printf)' "$Q/http.cpp")
if [ -n "$T" ]; then
  echo "FEHL ($B): Serial-Ausgabe mit sRequest/sParam in http.cpp ($N Serial-Aufrufe geprueft):"; echo "$T" | sed 's/^/  /'; exit 1
fi
echo "OK ($B): keine Serial-Ausgabe mit sRequest/sParam, $N Serial-Aufrufe geprueft"; exit 0
