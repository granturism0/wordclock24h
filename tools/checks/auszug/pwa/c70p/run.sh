#!/bin/sh
# Pruefstand c70p (Paket 2026-10-15, R2.3, AKC.6, Ent-3): Die PWA holt layout-previews.json mit
# cache "no-cache" statt "no-store", damit der Browser die Datei behaelt und per If-None-Match
# revalidiert (C70, design.md §4.6). Die Sprachdatei behaelt ihr "?v=" (design.md §8).
# Aufruf: run.sh <ESP-Verzeichnis mit data/app> [bezeichnung]   Exit 0 OK, 1 FEHL, 2 Werkzeugfehler
# Gebaut vom pwa-developer (R2.3), abgelegt vom Lead (R2.4); Sabotagen: sabotage.sh.
D=$(cd "$(dirname "$0")" && pwd)
command -v node >/dev/null 2>&1 || { echo "node fehlt"; exit 2; }
[ -s "$1/data/app/app.js" ] || { echo "KAPUTT c70p: app.js unter $1/data/app fehlt oder ist leer"; exit 2; }
exec node "$D/c70p.mjs" "$1/data/app/app.js" "${2:-PWA}"
