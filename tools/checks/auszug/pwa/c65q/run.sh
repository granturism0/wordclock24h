#!/bin/sh
# Pruefstand c65q (Paket 2026-10-13, E4.6/E4.7, AKV.3, AKV.6-AKV.9): Warteschlange, WLAN-Scan,
# Displaystrom, Ladesperre, Netzfristen der PWA -- Vorschau hinter einem zaehlenden Proxy und
# headless Chrome ueber CDP.
# Aufruf: run.sh <ESP-Verzeichnis mit data/app> [bezeichnung]   Exit 0 OK, 1 FEHL, 2 Werkzeugfehler
# Gebaut vom pwa-developer im Scratchpad; ablegen tut der Lead (design-e4.md §19.11).
D=$(cd "$(dirname "$0")" && pwd)
command -v node >/dev/null 2>&1 || { echo "node fehlt"; exit 2; }
[ -s "$1/data/app/app.js" ] && [ -s "$1/data/app/index.html" ] || { echo "KAPUTT c65q: app.js/index.html unter $1/data/app fehlen"; exit 2; }
# Je Lauf vier Ports (Proxy, Vorschau +1, CDP +1000): Schritt 4, sonst trifft ein zweiter Lauf den ersten.
P=$((8700 + ($$ % 100) * 4))
WORDCLOCK_ROOT=${WORDCLOCK_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null)}
[ -s "$WORDCLOCK_ROOT/tools/preview/server.py" ] || { echo "KAPUTT c65q: tools/preview/server.py nicht gefunden (WORDCLOCK_ROOT=$WORDCLOCK_ROOT)"; exit 2; }
export WORDCLOCK_ROOT
exec node "$D/c65q.mjs" "$1/data/app" "$P" "${2:-PWA}"
