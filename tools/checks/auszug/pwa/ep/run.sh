#!/bin/sh
# Pruefstand ep (Paket 2026-10-13, EP.7): C6b (AKS.3), C46 (AKQ.3), B55 (AKL.3) in der PWA, Mock.
# Aufruf: run.sh <ESP-Verzeichnis mit data/app/app.js> [bezeichnung]   Exit 0/1/2
D=$(cd "$(dirname "$0")" && pwd)
command -v node >/dev/null 2>&1 || { echo "node fehlt"; exit 2; }
A="$1/data/app"
[ -s "$A/app.js" ] && [ -s "$A/index.html" ] && [ -s "$A/i18n/en.json" ] || { echo "KAPUTT ep: app.js/index.html/en.json unter $A fehlen"; exit 2; }
exec node "$D/ep.mjs" "$A/app.js" "$A/index.html" "$A/i18n/en.json" "${2:-PWA}"
