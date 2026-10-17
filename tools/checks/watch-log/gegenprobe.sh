#!/bin/bash
# Gegenproben fuer die fail-Regel der Logwache (BEFUNDE L497, DIR-014): eine kuenstliche Mitschnittquelle
# (WATCHLOG_QUELLE) liefert nacheinander "http write lost"-Zeilen und einen Neustart; die Wache muss genau
# die Anstiege des Zaehlers fail als Alarm melden und den Rest als Hinweis.
#   F1 gone, fail gleich (Grundlinie 1)     -> Hinweis      F4 nach Neustart mit Zwischenzeile fail=0 -> Hinweis
#   F2 Anstieg 1 -> 8                        -> Alarm        F5 Anstieg 0 -> 2                         -> Alarm
#   F3 gone, fail gleich 8                   -> Hinweis      F6 rst cause, danach fail=2 ohne Zwischenzeile -> Alarm 0 -> 2
# F6 traegt absichtlich DENSELBEN Wert wie vor dem Neustart (2 aus F5): Nur so trennt er die Fassung ohne
# Ruecksetzung bei rst cause -- mit einem kleineren Wert griffe dort schon die Regel "Zaehler gefallen".
# Spricht nicht mit dem Geraet, schreibt eigene Zustandsdateien (.probe). Laufzeit rund 40 s.
# Aufruf: tools/checks/watch-log/gegenprobe.sh [wache]   Exit 0 alle wie erwartet, 1 Abweichung, 2 Werkzeugfehler.
R=$(git -C "$(dirname "$0")" rev-parse --show-toplevel) || exit 2
WACHE=${1:-$R/tools/watch-log.sh}
T=$(mktemp -d "${TMPDIR:-/tmp}/watchlog-probe.XXXXXX") || exit 2
trap 'case "$T" in "${TMPDIR:-/tmp}"*watchlog-probe.*) rm -rf "${T:?}";; esac' EXIT
M=$T/mitschnitt.txt
printf '#!/bin/sh\ncat "%s"\n' "$M" > "$T/quelle.sh"; chmod +x "$T/quelle.sh"
z() { printf '2026-10-17T04:%s+02:00   %s\n' "$1" "$2" >> "$M"; }
: > "$M"; z 00:00.000 '(- http write lost 1072 gone=0 fail=1 blk=4336 snd=0 ms=1096)'
(cd "$R" && WATCHLOG_QUELLE=$T/quelle.sh "$WACHE" --interval 2 > "$T/aus.txt" 2>&1) & W=$!
sleep 4; z 00:10.000 '(- http write lost 1072 gone=1 fail=1 blk=4208 snd=0 ms=3)'
sleep 4; z 00:20.000 '(- http write lost 1072 gone=1 fail=8 blk=4208 snd=0 ms=5)'
sleep 4; z 00:30.000 '(- http write lost 1072 gone=2 fail=8 blk=4208 snd=0 ms=4)'
sleep 4; z 00:40.000 '(- http write lost 1072 gone=1 fail=0 blk=4208 snd=0 ms=4)'
sleep 4; z 00:50.000 '(- http write lost 1072 gone=1 fail=2 blk=4208 snd=0 ms=4)'
sleep 4; z 01:00.000 '( ets Jan  8 2013,rst cause:2, boot mode:(3,6))'
sleep 4; z 01:10.000 '(- http write lost 506 gone=0 fail=2 blk=4672 snd=0 ms=1051)'
sleep 5; kill "$W" 2>/dev/null; wait "$W" 2>/dev/null
ist=$(grep -E 'Antwort verloren' "$T/aus.txt" | sed -E 's/.*(\*\*\* Antwort verloren \(http write lost, fail [0-9]+ -> [0-9]+\)|Hinweis: Antwort verloren).*/\1/')
soll='Hinweis: Antwort verloren
*** Antwort verloren (http write lost, fail 1 -> 8)
Hinweis: Antwort verloren
Hinweis: Antwort verloren
*** Antwort verloren (http write lost, fail 0 -> 2)
*** Antwort verloren (http write lost, fail 0 -> 2)'
n=$(printf '%s\n' "$ist" | grep -c .)
echo "  Logwache-Gegenprobe: $n von 6 Faellen gemeldet"
if [ "$ist" = "$soll" ]; then echo "  ok    fail-Regel: 3 Alarme (1->8, 0->2, 0->2 nach Neustart), 3 Hinweise"; exit 0; fi
echo "  FALSCH fail-Regel -- erwartet:"; printf '%s\n' "$soll" | sed 's/^/          /'
echo "         erhalten:"; printf '%s\n' "$ist" | sed 's/^/          /'; exit 1
