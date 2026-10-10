#!/bin/sh
# Sabotagen zu Pruefstand C3 (DIR-014). Aufruf: sabotage.sh <src-Verzeichnis>
# Jede der drei muss FEHL melden (Exit 1 des Pruefstands). Exit 0 nur, wenn alle drei anschlagen.
S=$(cd "$(dirname "$0")" && pwd); SRC=$1; rc=0
for n in 1 2 3; do
  case $n in 1) t="Firmware-Seite 64 gegen Baustein 32";; 2) t="Firmware-Seite 32 gegen Baustein 8";; 3) t="Seitenende um eins verschoben";; esac
  out=$(C3_SABOTAGE=$n sh "$S/run.sh" "$SRC" 2>&1); r=$?
  if [ $r -eq 1 ]; then echo "  schlaegt an  Sabotage $n ($t)"; else echo "  BLIEB STUMM  Sabotage $n ($t), rc=$r"; rc=1; fi
  printf '%s\n' "$out" | grep -a -E "^=== |<- FEHL|: (OK|FEHL)$" | sed 's/^/      /'
done
exit $rc
