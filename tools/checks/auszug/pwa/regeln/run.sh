#!/bin/sh
# Pruefstand Regelgleichheit (Paket 2026-10-10, P.5): Die PWA entscheidet Datumsformat und
# Koordinaten genau wie der ESP -- gemessen an DERSELBEN Fall-Tabelle wie t17 (esp/t17/regeln.tsv).
# Aufruf: run.sh <ESP-Verzeichnis mit data/app/app.js> [bezeichnung]
D=$(cd "$(dirname "$0")" && pwd)
command -v node >/dev/null 2>&1 || { echo "node fehlt"; exit 2; }
exec node "$D/regeln.mjs" "$1/data/app/app.js" "$D/../../esp/t17/regeln.tsv" "${2:-PWA}"
