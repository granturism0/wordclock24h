#!/bin/sh
# Pruefstand b48 (Paket 2026-10-13, P2.5, AKF.1): Fehlerzeile am Feld, Vorschau + headless Chrome.
# Aufruf: run.sh <ESP-Verzeichnis mit data/app> [bezeichnung]   Exit 0/1/2
D=$(cd "$(dirname "$0")" && pwd)
command -v node >/dev/null 2>&1 || { echo "node fehlt"; exit 2; }
[ -s "$1/data/app/app.js" ] && [ -s "$1/data/app/index.html" ] || { echo "KAPUTT b48: app.js/index.html unter $1/data/app fehlen"; exit 2; }
# Port je Lauf verschieden, damit eine offene Vorschau (8099) nicht stoert.
P=$((8200 + $$ % 500))
# Repo-Wurzel fuer tools/preview/server.py: von auszug.sh (Temp-Kopie!), sonst aus dem Arbeitsverzeichnis.
WORDCLOCK_ROOT=${WORDCLOCK_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null)}
[ -s "$WORDCLOCK_ROOT/tools/preview/server.py" ] || { echo "KAPUTT b48: tools/preview/server.py nicht gefunden (WORDCLOCK_ROOT=$WORDCLOCK_ROOT)"; exit 2; }
export WORDCLOCK_ROOT
exec node "$D/b48.mjs" "$1/data/app" "$P" "${2:-PWA}"
