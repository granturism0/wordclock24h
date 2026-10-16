#!/bin/sh
# Pruefstand c70p (Paket 2026-10-15; R2.3, AKC.6, Ent-3; nachgezogen in R2b.6, AKL.5, AKL.6): Die PWA
# holt layout-previews.json ohne Query mit cache "no-cache" (Ent-20 B) und die Sprachdatei mit
# ?v=<APP_VERSION>&h=<marke> -- die Marke aus <meta name="wc-asset-tokens">, nur nach Muster -- bzw.
# ohne gueltige Marke mit ?v=<APP_VERSION>, cache "default" (design.md §18.7).
# Aufruf: run.sh <ESP-Verzeichnis mit data/app> [bezeichnung]   Exit 0 OK, 1 FEHL, 2 Werkzeugfehler
# Gebaut vom pwa-developer (R2.3, R2b.6), abgelegt vom Lead (R2.4, R2b.7); Sabotagen: sabotage.sh.
D=$(cd "$(dirname "$0")" && pwd)
command -v node >/dev/null 2>&1 || { echo "node fehlt"; exit 2; }
[ -s "$1/data/app/app.js" ] || { echo "KAPUTT c70p: app.js unter $1/data/app fehlt oder ist leer"; exit 2; }
exec node "$D/c70p.mjs" "$1/data/app/app.js" "${2:-PWA}"
