#!/bin/sh
# Pruefstaende c65a, c65b, c65m, c65n, c65e, c65t (Paket 2026-10-13, E3; AKR.1-AKR.7, design.md 18.7).
# Aufruf: run.sh <ESP-Quellverzeichnis> [bezeichnung]. Exit 0 alle OK, 1 mindestens ein FEHL, 2 mindestens einer KAPUTT.
# Referenz fuer c65m/c65n ist ESP 3.2.35 aus dem Tag-Commit 8445a10 (ALT=<verzeichnis> uebersteuert).
D=$(cd "$(dirname "$0")" && pwd); Q=$(cd "$1" && pwd) || exit 2; B=${2:-neu}; schlecht=0
lauf () { n=$1; shift; out=$("$@" 2>&1); r=$?; echo "$out" | grep -E '^(OK|FEHL|  Faelle|c65t|c65a statisch|      FEHLER|UEBERSETZUNG|AUSZUG|KAPUTT)' | sed "s/^/  [$n] /"
          [ $r -gt $schlecht ] && schlecht=$r; }
lauf c65a sh "$D/run_a.sh" "$Q" "$B"
lauf c65b sh "$D/run_b.sh" "$Q" "$B"
lauf c65m sh "$D/run_mn.sh" "$Q" "$B" m
lauf c65n sh "$D/run_mn.sh" "$Q" "$B" n
lauf c65e sh "$D/run_mn.sh" "$Q" "$B" e
lauf c65t python3 "$D/c65t.py" "$Q" "$B"
echo "c65 ($B): Exit $schlecht"; exit $schlecht
