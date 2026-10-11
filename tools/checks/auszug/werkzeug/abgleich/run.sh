#!/usr/bin/env bash
# Pruefstand fuer tools/abgleich.sh (A61, Paket 2026-10-11, 0.1; nach Review 0.2 neu gefasst).
#   ./run.sh [werkzeug]     Vorgabe: tools/abgleich.sh des Baums
#
# Drei Mitschnitte, je mit eigenem erwarteten Exit -- ein Werkzeug, das immer gleich endet,
# besteht nicht (Review 0.2, M4a). Zeilen in der Reihenfolge, die das Geraet schreibt
# (Version -> IPADDRESS -> Ende -> diag 1 beim Start, Review 0.2 Runde 2, H-A).
#   alarm.log  Exit 1. Die Grenzen EINZELN (Dauer 6 s, 11 Zeitueberschreitungen, Luecke 7 s);
#              verschachtelter Ausloeser; CMC mit Formatzeichenkette davor; Fremdzeile mit
#              Endtext; "(SYNCVARS 1)" loest nicht aus; Ausloeser, dem eine diag-Zeile ohne Ende
#              folgt (zweite diag-Zeile, Review Runde 3); Abgleich ueber 70 s ohne diag-Zeile,
#              waehrenddessen ein zweiter Ausloeser, dessen Abgleich nach einer diag-Zeile folgt
#              (kein "zusammengefasst?", Beginn ab der diag-Zeile); Neustart mitten im Abgleich;
#              Doppel-Neustart ohne diag dazwischen; Neustart ohne Version-Zeile (nur die
#              diag-Folge faellt); zwei Ausloeser ohne Abgleich (etwa im Spiel), beide gemeldet
#              (Runde 4); offen am Mitschnittende.
#   ruhig.log  Exit 0. Firmware vor 3.2.22 ohne Endzeile (Hinweis); CMC-Schub #1-#4, nur #1
#              merkt vor (Drossel, H-B); #3 an der Drossel (Hinweis), danach #4, die deshalb
#              ebenfalls nicht ausloest (Drossel nachgefuehrt, Runde 3); zwei Ausloeser,
#              ein Abgleich (zusammengefasst, Hinweis, M-C); Ausloeser -> diag -> Ende (Ausloeser
#              in einer Warteschleife nach der Abgleichstelle, kein Fehlalarm); Abweisung #50 nach
#              stummen Abweisungen (Drossel unbekannt, Hinweis).
#   zaehl.log  Exit 3. Eine Endzeile ohne Zeitstempel -- die Zaehlung muss abweichen.
set -uo pipefail
HIER=$(cd "$(dirname "$0")" && pwd)
WERKZEUG=${1:-${ROOT:-$HIER/../../../../..}/tools/abgleich.sh}
fehl=0; faelle=0
for paar in alarm:1 ruhig:0 zaehl:3; do
  name=${paar%%:*}; soll_rc=${paar##*:}
  ist=$("$WERKZEUG" "$HIER/$name.log" 2>/dev/null); rc=$?
  faelle=$((faelle+1))
  printf '%s\n' "$ist" | cmp -s - "$HIER/$name.soll" || { echo "FEHL: $name: Ausgabe weicht ab"; printf '%s\n' "$ist" | diff - "$HIER/$name.soll" | head -6; fehl=1; }
  [ "$rc" -eq "$soll_rc" ] || { echo "FEHL: $name: Exit $rc statt $soll_rc"; fehl=1; }
done
# Nachzaehlen: alarm.log hat 13 echte Endzeilen und eine Fremdzeile mit dem Text.
n=$(grep -c '   info: configuration sent$' "$HIER/alarm.log")
grep -q "^$n Abgleich(e) ausgewertet, $n Endzeile" "$HIER/alarm.soll" || { echo "FEHL: alarm.soll zaehlt nicht $n"; fehl=1; }
[ $fehl -eq 0 ] && echo "OK: abgleich $faelle Mitschnitte, $n Endzeilen in alarm.log, Exit 1/0/3 wie erwartet"
exit $fehl
