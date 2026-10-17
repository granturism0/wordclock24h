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
# AKI.11 (Spec C80): gewoehnliche Zeilen des Instruments -> keine Meldung; abbruch= -> Warnung
sleep 4; z 01:20.000 '(- heap topf 1/4 wif=16/16@6993 srv=-192/800@1995 ota=0/0@-)'
z 01:20.100 '(- heap karte X ent=3/2 fsum=7144 gross=229+6848 zweit=129+296 davor=166+504 danach=1085+4032 ms=5005)'
z 01:20.200 '(- heap marken X tcp=6:10/- lst=2 wet=38 dhcp=34 eep=42 udp=14/18,22/26,30/-)'
z 01:20.300 '(- heap karte m abbruch=ord)'
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
# Instrument (AKI.11): genau eine Meldung, und zwar die Warnung zu abbruch=
instr=$(grep -cE 'Heap-Karte abgebrochen' "$T/aus.txt"); fremd=$(grep -E 'heap (topf|karte|marken)' "$T/aus.txt" | grep -cv 'Heap-Karte abgebrochen')
if [ "$instr" = 1 ] && [ "$fremd" = 0 ]; then echo "  ok    Instrument: abbruch= gemeldet, topf/karte X/marken still"; else
  echo "  FALSCH Instrument: $instr Warnung(en) zu abbruch= (Soll 1), $fremd Meldung(en) zu gewoehnlichen Zeilen (Soll 0)"; ins_bad=1; fi
if [ "$ist" = "$soll" ]; then echo "  ok    fail-Regel: 3 Alarme (1->8, 0->2, 0->2 nach Neustart), 3 Hinweise"; else
  echo "  FALSCH fail-Regel -- erwartet:"; printf '%s\n' "$soll" | sed 's/^/          /'
  echo "         erhalten:"; printf '%s\n' "$ist" | sed 's/^/          /'; fail_bad=1; fi
[ -z "${fail_bad:-}" ] && [ -z "${ins_bad:-}" ] && exit 0
exit 1
