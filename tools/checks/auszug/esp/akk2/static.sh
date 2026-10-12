#!/bin/sh
# AKK.2 (Paket 2026-10-11, E.4): jeder der drei OTA-Pfade kuendigt mit "OTA start" an und meldet
# einen Fehlschlag mit SYNCVARS. Aufruf: static.sh <ESP-Quellverzeichnis> <bezeichnung>
exec python3 "$(cd "$(dirname "$0")" && pwd)/static.py" "$1/http.cpp" "$2"
