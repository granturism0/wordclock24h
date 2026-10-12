#!/bin/sh
# Pruefstand c35 (Paket 2026-10-11, P.1, AKC.1): Wie oft ruft die PWA update_status ab?
# Hauptmodul 30 min offen => hoechstens 7 Abrufe je update_status und update_table_files
# Update- und Wartungsmodul wie bisher bei jedem 15-s-Lauf; Wechsel dorthin holt sofort;
# ein Fehlschlag zaehlt als Abruf. Gegenprobe: der Ausgangsstand ruft im Hauptmodul rund
# 120-mal ab und muss FEHL melden.
# Aufruf: run.sh <ESP-Verzeichnis mit data/app/app.js> [bezeichnung]
# Exit 0 = OK, 1 = FEHL, 2 = Werkzeugfehler.
D=$(cd "$(dirname "$0")" && pwd)
command -v node >/dev/null 2>&1 || { echo "node fehlt"; exit 2; }
[ -n "$1" ] || { echo "Aufruf: run.sh <ESP-Verzeichnis> [bezeichnung]"; exit 2; }
exec node "$D/c35.mjs" "$1/data/app/app.js" "${2:-PWA}"
