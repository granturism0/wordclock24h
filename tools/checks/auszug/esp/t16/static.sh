#!/bin/sh
# Statische Pruefung C47 / AKX.2, zweite Fassung (Review E, M3). Aufruf: static.sh <verzeichnis mit http.cpp> <bezeichnung>
# Die Logik steht in static.py daneben (Fixpunkt ueber die Ausgabehelfer, Pruefung jeder Aufrufstelle).
D=$(cd "$(dirname "$0")" && pwd); Q=$1; B=$2
python3 "$D/static.py" "$Q/http.cpp" "$D/bestand.tsv" "$B"
