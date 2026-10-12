#!/bin/sh
# Pruefstand N5 (Paket 2026-10-11, S3.2): snprintf/sprintf-Huellen gegen die libc, ueber alle Formate aus src.
# Aufruf: run.sh <src-Verzeichnis> [sabotage]      Sabotagen: abschneiden groesse rueckgabe
# Zwischendateien in einem eigenen Arbeitsverzeichnis unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
D=$(cd "$(dirname "$0")" && pwd); SRC=$1; SAB=${2:-}
W=$(mktemp -d "${TMPDIR:-/tmp}/n5.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
python3 "$D/extract.py" "$SRC" "$W" $SAB || exit 2
cc -std=gnu99 -Wall -Wno-format-security -Wno-format -Wno-deprecated-declarations -Wno-unknown-attributes -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -fno-builtin -I "$W" "$D/n5.c" -o "$W/n5" || exit 2
echo "=== N5${SAB:+, Sabotage $SAB} ==="; "$W/n5"
