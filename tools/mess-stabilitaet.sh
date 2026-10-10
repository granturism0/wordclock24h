#!/usr/bin/env bash
# Messpaket "Stabilitaet ESP": prueft, ob die vier als KRITISCH gefuehrten Befunde mit dem
# heutigen ESP noch auftreten.
#
#   ./tools/mess-stabilitaet.sh                    alle Phasen, Browserphase 30 min
#   ./tools/mess-stabilitaet.sh --phasen 1,2,3     nur diese Phasen
#   ./tools/mess-stabilitaet.sh --browser-min 60   Browserphase verlaengern
#
# WARUM ES DIESES SKRIPT GIBT
#
# C9 (Exception 29, L125/L140/L169), C9e (parallele Verbindungen, L149), C9c2 (Heap-
# Fragmentierung, L175) und A28 (Brueckenverluste, L188) stammen alle vom 03./04.10.2026,
# gemessen auf ESP 3.2.14/3.2.15. Seither kamen ein Dutzend ESP-Releases. Ob sie noch
# auftreten, hat niemand gemessen -- und solange das so ist, stehen sie zu Recht auf
# KRITISCH. Der Nutzer am 10.10.2026: "Zuerst klaeren, ob die vier kritischen Befunde mit
# dem heutigen ESP ueberhaupt noch auftreten."
#
# Jede Phase stellt die Last nach, unter der der Befund damals auftrat:
#
#   1  C9/L125   20 x app.js nacheinander (die groesste Datei, damals 1 von 5 Abstuerzen)
#   2  C9/L140   60 x update_status im Sekundentakt (der heapteuerste Endpunkt)
#   3  C9e/L149  parallele Anfragen in Stufen 1,2,3,4,6,8 -- dieselbe Tabelle wie damals
#   4  C9/L169   Helligkeit aendern, waehrend vier Abrufe parallel laufen; Wert danach zurueck
#                (SCHREIBEND -- die einzige Phase, die etwas am Geraet aendert)
#   5  C9c2/L175 PWA im echten Browser offen lassen und den Heap mitschreiben
#
# A28 wird nicht eigens ausgeloest: d=, rx= und v= werden nach JEDER Phase gelesen. Steigt
# d irgendwo, ist das der Ort, an dem der Ring vollaeuft.
#
# Nach jeder Phase: neue Exceptions, Neustarts, Watchdog, Heap (kleinster Block), d/v/a,
# und ob der ESP noch antwortet. Stuerzt er ab, wird das gemeldet und gewartet, bis er
# wieder da ist -- danach gehoert die Update-Quelle geprueft (DIR-009, L42).
#
# Gefaehrliche Endpunkte (CLAUDE.md, R5) werden nicht beruehrt. Kein Parameter ohne "=".

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2
[ -f tools/device.conf ] || { echo "tools/device.conf fehlt" >&2; exit 2; }
. tools/device.conf
U="http://$DEVICE_HOST"

# MESS_QUELLE ersetzt den Mitschnitt durch ein Skript, das wie log.sh auf "tail <n>"
# antwortet -- nur fuer die Gegenprobe der Auswertung (DIR-014), nie im Geraetelauf.
LOG=${MESS_QUELLE:-./tools/logger/log.sh}

PHASEN="1,2,3,4,5"
BROWSER_MIN=30
while [ $# -gt 0 ]; do
  case "$1" in
    --phasen) PHASEN="$2"; shift 2 ;;
    --browser-min) BROWSER_MIN="$2"; shift 2 ;;
    *) echo "Unbekannte Option: $1" >&2; exit 2 ;;
  esac
done
hat() { case ",$PHASEN," in *",$1,"*) return 0 ;; *) return 1 ;; esac; }

[ -z "${MESS_QUELLE:-}" ] && touch "$(git rev-parse --git-dir)/geraet-gemessen" 2>/dev/null

ERG="$(git rev-parse --git-dir)/mess-stabilitaet.out"
exec > >(tee -a "$ERG") 2>&1
printf '\n=== Messpaket Stabilitaet ESP, %s ===\n' "$(date '+%d.%m.%Y %H:%M:%S')"

zaehle() { "$LOG" tail 4000 2>/dev/null | grep -ac "$1"; }
diag_letzte() { "$LOG" tail 400 2>/dev/null | grep -aE '(^|[^A-Za-z])diag [0-9]+ ' | tail -1; }
feld() { printf '%s' "$1" | grep -aoE "(^| )$2=[0-9/]+" | tail -1 | sed 's/^ //'; }
lebt() { curl -s -m 5 -o /dev/null -w '%{http_code}' "$U/api/power_status" 2>/dev/null | grep -q 200; }

# Heap seit einem Zeitpunkt: kleinster grosser Block und letzter Stand
heap_seit() {
  "$LOG" tail 4000 2>/dev/null | awk -v t="$1" '$1 >= t' | grep -a 'heap free=' \
    | sed -E 's/.*free=([0-9]+) max=([0-9]+).*/\1 \2/' \
    | awk 'NR==1{mf=$1;mb=$2} {if($2<mb){mb=$2;mf=$1} lf=$1; lb=$2} END{if(NR) printf "kleinster Block %s (frei %s), zuletzt %s/%s, %d Zeilen", mb, mf, lf, lb, NR; else printf "keine heap-Zeile"}'
}

ESP_V=$(curl -s -m 5 "$U/api/update_status" | grep -oE '"esp_version":"[^"]*"' | cut -d'"' -f4)
# Die STM-Version steht in update_status nicht als stm_version, sondern als strvar 1 in
# settings_xml -- der erste Lauf am 10.10.2026 zeigte deshalb "STM ?".
STM_V=$(curl -s -m 5 "$U/api/settings_xml" | grep -oE '<strvar idx="1" value="[^"]*"' | sed -E 's/.*value="([^"]*)"/\1/')
printf '  Geraet: ESP %s, STM %s\n' "${ESP_V:-?}" "${STM_V:-?}"

basis_exc=$(zaehle 'Exception (')
basis_rst=$(zaehle 'rst cause')
basis_wdt=$(zaehle 'wdt reset')
d0=$(diag_letzte)
printf '  Ausgang: %s %s %s %s\n\n' "$(feld "$d0" rx)" "$(feld "$d0" d)" "$(feld "$d0" v)" "$(feld "$d0" a)"

BEFUND=0
phase_ende() {   # $1 = Name, $2 = Startzeit (ISO-Praefix fuer den Heap)
  sleep 12                                   # die naechste diag-Zeile abwarten
  local exc rst wdt dz
  exc=$(zaehle 'Exception ('); rst=$(zaehle 'rst cause'); wdt=$(zaehle 'wdt reset')
  dz=$(diag_letzte)
  printf '  -> %s: Exceptions +%d, Neustarts +%d, Watchdog +%d | %s %s %s %s\n' "$1" \
    "$((exc - basis_exc))" "$((rst - basis_rst))" "$((wdt - basis_wdt))" \
    "$(feld "$dz" rx)" "$(feld "$dz" d)" "$(feld "$dz" v)" "$(feld "$dz" a)"
  printf '     Heap: %s\n' "$(heap_seit "$2")"
  if [ "$exc" -gt "$basis_exc" ] || [ "$rst" -gt "$basis_rst" ] || [ "$wdt" -gt "$basis_wdt" ]; then
    printf '     *** ABSTURZ ODER NEUSTART IN DIESER PHASE ***\n'
    "$LOG" tail 4000 2>/dev/null | grep -aB4 -A3 'Exception (' | tail -12 | sed 's/^/        /'
    BEFUND=1
    [ -z "${MESS_QUELLE:-}" ] && for i in $(seq 1 24); do lebt && break; sleep 5; done
    [ -z "${MESS_QUELLE:-}" ] && printf '     danach: %s -- Update-Quelle pruefen: ./tools/smoke-device.sh\n' "$(lebt && echo 'ESP antwortet wieder' || echo 'ESP ANTWORTET NICHT')"
  fi
  [ -z "${MESS_QUELLE:-}" ] && ! lebt && { printf '     *** ESP antwortet nicht ***\n'; BEFUND=1; }
  basis_exc=$exc; basis_rst=$rst; basis_wdt=$wdt
}
jetzt() { date '+%Y-%m-%dT%H:%M:%S'; }

if [ -n "${MESS_QUELLE:-}" ]; then          # Gegenprobe: nur die Auswertung
  phase_ende "Probe" "$(jetzt | cut -c1-10)"
  printf '\n=== Ergebnis: %s ===\n' "$([ $BEFUND -eq 0 ] && echo 'unauffaellig' || echo 'BEFUND')"
  exit $BEFUND
fi

# --------------------------------------------------------------- Phase 1: app.js
if hat 1; then
  t=$(jetzt); printf '[1] C9/L125: 20 x app.js nacheinander\n'
  groessen=""; langsam=0; fehl=0
  for i in $(seq 1 20); do
    r=$(curl -s -m 25 -o /dev/null -H 'Accept-Encoding: gzip' -w '%{http_code} %{size_download} %{time_total}' "$U/app/app.js")
    set -- $r
    [ "$1" = 200 ] || fehl=$((fehl+1))
    groessen="$groessen $2"
    awk -v z="$3" 'BEGIN{exit !(z>5)}' && langsam=$((langsam+1))
  done
  n_gr=$(printf '%s\n' $groessen | sort -u | wc -l | tr -d ' ')
  printf '     %d Fehler, %d ueber 5 s, %d verschiedene Groessen (%s)\n' "$fehl" "$langsam" "$n_gr" "$(printf '%s\n' $groessen | sort | uniq -c | tr '\n' ' ')"
  [ "$fehl" -gt 0 ] || [ "$n_gr" -gt 1 ] && BEFUND=1
  phase_ende "Phase 1" "$t"
fi

# --------------------------------------------------------- Phase 2: update_status
if hat 2; then
  t=$(jetzt); printf '[2] C9/L140: 60 x update_status im Sekundentakt\n'
  fehl=0; max=0
  for i in $(seq 1 60); do
    r=$(curl -s -m 15 -o /dev/null -w '%{http_code} %{time_total}' "$U/api/update_status"); set -- $r
    [ "$1" = 200 ] || fehl=$((fehl+1))
    max=$(awk -v a="$max" -v b="$2" 'BEGIN{print (b>a)?b:a}')
    sleep 1
  done
  printf '     %d Fehler, laengste Antwort %s s\n' "$fehl" "$max"
  [ "$fehl" -gt 0 ] && BEFUND=1
  phase_ende "Phase 2" "$t"
fi

# ------------------------------------------------------ Phase 3: parallel (L149)
EP=(settings_xml display_power ambilight_power update_status stm32_log power_status overlay_icons fs_info)
if hat 3; then
  t=$(jetzt); printf '[3] C9e/L149: parallele Anfragen, je Stufe zwei Runden, Zeitlimit 12 s\n'
  printf '     parallel  Anfragen  ok  Fehler  max.Dauer\n'
  for p in 1 2 3 4 6 8; do
    tmp=$(mktemp)
    for runde in 1 2; do
      for k in $(seq 0 $((p-1))); do
        e=${EP[$(( (k + runde) % ${#EP[@]} ))]}
        curl -s -m 12 -o /dev/null -w '%{http_code} %{time_total}\n' "$U/api/$e" >> "$tmp" &
      done
      wait
      sleep 2
    done
    n=$(wc -l < "$tmp" | tr -d ' '); okz=$(grep -c '^200 ' "$tmp"); mx=$(awk '{if($2>m)m=$2} END{print m}' "$tmp")
    printf '     %8d  %8d  %2d  %6d  %8s s\n' "$p" "$n" "$okz" "$((n-okz))" "$mx"
    [ "$okz" -lt "$n" ] && BEFUND=1
    rm -f "$tmp"; sleep 3
  done
  phase_ende "Phase 3" "$t"
fi

# --------------------------------------------- Phase 4: L169-Kette (schreibend)
if hat 4; then
  t=$(jetzt); printf '[4] C9/L169: Helligkeit aendern unter parallelen Abrufen (3 Runden, Wert danach zurueck)\n'
  alt=$(curl -s -m 10 "$U/api/settings_xml" | grep -oE '<numvar idx="6" value="[0-9]+"' | grep -oE '[0-9]+"$' | tr -d '"')
  if [ -z "$alt" ]; then
    printf '     ABBRUCH: Ausgangswert der Helligkeit nicht lesbar -- Phase uebersprungen\n'
  else
    neu=$(( alt >= 8 ? alt - 3 : alt + 3 ))
    printf '     Helligkeit: %s -> %s -> %s\n' "$alt" "$neu" "$alt"
    for r in 1 2 3; do
      for e in settings_xml display_power ambilight_power update_status; do
        curl -s -m 12 -o /dev/null "$U/api/$e" &
      done
      curl -s -m 12 -o /dev/null "$U/api/display_brightness_set?value=$neu" &
      wait; sleep 4
      for e in settings_xml display_power ambilight_power update_status; do
        curl -s -m 12 -o /dev/null "$U/api/$e" &
      done
      curl -s -m 12 -o /dev/null "$U/api/display_brightness_set?value=$alt" &
      wait; sleep 4
    done
    jetzt_w=$(curl -s -m 10 "$U/api/settings_xml" | grep -oE '<numvar idx="6" value="[0-9]+"' | grep -oE '[0-9]+"$' | tr -d '"')
    printf '     Wert danach: %s (%s)\n' "$jetzt_w" "$([ "$jetzt_w" = "$alt" ] && echo 'zurueck' || echo 'NICHT ZURUECK')"
    [ "$jetzt_w" = "$alt" ] || BEFUND=1
  fi
  phase_ende "Phase 4" "$t"
fi

# ----------------------------------------------- Phase 5: PWA im Browser (L175)
if hat 5; then
  t=$(jetzt); printf '[5] C9c2/L175: PWA %d min im Browser offen, Heap jede Minute\n' "$BROWSER_MIN"
  CHROME=${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}
  PROFILE=$(mktemp -d)
  "$CHROME" --headless=new --disable-gpu --no-sandbox --user-data-dir="$PROFILE" \
    "$U/app/" >/dev/null 2>&1 &
  CPID=$!
  for m in $(seq 1 "$BROWSER_MIN"); do
    sleep 60
    h=$("$LOG" tail 200 2>/dev/null | grep -a 'heap free=' | tail -1 | grep -oE 'free=[0-9]+ max=[0-9]+')
    [ $((m % 5)) -eq 0 ] && printf '     %3d min  %s\n' "$m" "$h"
    lebt || printf '     %3d min  *** ESP antwortet nicht ***\n' "$m"
  done
  kill "$CPID" 2>/dev/null; wait "$CPID" 2>/dev/null; rm -rf "$PROFILE"
  phase_ende "Phase 5" "$t"
fi

printf '\n=== Ergebnis: %s ===\n' "$([ $BEFUND -eq 0 ] && echo 'unauffaellig' || echo 'BEFUND -- siehe oben')"
printf 'Mitschrift: %s\n' "$ERG"
exit $BEFUND
