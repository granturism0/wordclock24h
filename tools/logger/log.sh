#!/usr/bin/env bash
# Greift den Mitschnitt der WordClock vom Raspberry ab.
#
#   ./tools/logger/log.sh tail [n]        letzte n Zeilen (Vorgabe 200)
#   ./tools/logger/log.sh grep <muster>   im aktuellen und in den alten Logs suchen
#   ./tools/logger/log.sh since <zeit>    ab Zeitpunkt, z. B. "21:50" oder "2026-09-30T21"
#   ./tools/logger/log.sh follow [sek]    mitlesen, bricht nach sek Sekunden ab (Vorgabe 30)
#   ./tools/logger/log.sh mark <text>     Markierung im Mitschnitt setzen
#   ./tools/logger/log.sh stats           Datenrate, Groesse, Dienststatus
#   ./tools/logger/log.sh gaps [ms]       Luecken ueber ms finden (Vorgabe 500) -- Blockaden
#
# Ziel per Umgebungsvariablen: LOG_HOST LOG_USER LOG_PORT LOG_KEY LOG_FILE
set -uo pipefail

HOST=${LOG_HOST:-wordclock-pi.local}
USER=${LOG_USER:-pi}
PORT=${LOG_PORT:-22}
KEY=${LOG_KEY:-~/.ssh/wordclock_pi}
FILE=${LOG_FILE:-/var/log/wordclock/serial.log}
KEY=${KEY/#\~/$HOME}          # ~ expandiert in Variablen nicht von selbst

SSH="ssh -p $PORT -o BatchMode=yes -o ConnectTimeout=8"
[ -f "$KEY" ] && SSH="$SSH -i $KEY -o IdentitiesOnly=yes"
T="$USER@$HOST"
CMD=${1:-tail}; shift || true

case "$CMD" in
  tail)   $SSH "$T" "tail -n ${1:-200} '$FILE'" ;;
  grep)   [ $# -gt 0 ] || { echo "Muster fehlt" >&2; exit 2; }
          # -a, weil ein abgerissener Transfer Binaerzeichen hinterlassen kann
          $SSH "$T" "zgrep -ah -- '$1' '$FILE' ${FILE%.log}*.gz 2>/dev/null | tail -n ${2:-200}" ;;
  since)  [ $# -gt 0 ] || { echo "Zeit fehlt" >&2; exit 2; }
          $SSH "$T" "awk -v s='$1' 'index(\$1,s)==1{f=1} f' '$FILE' | tail -n ${2:-500}" ;;
  follow) $SSH "$T" "timeout ${1:-30} tail -n 5 -f '$FILE'" ;;
  mark)   [ $# -gt 0 ] || { echo "Text fehlt" >&2; exit 2; }
          $SSH "$T" "echo 'MARKE: $*' > /run/wordclock-mark" && echo "gesetzt: $*" ;;
  stats)  # Die Rate wird ueber die GESAMTE Laufzeit der Datei gebildet, nicht ueber
          # ein kurzes Fenster. Der Verkehr kommt schubweise -- eine Messung ueber
          # fuenf Sekunden landet regelmaessig bei null und ist damit irrefuehrend.
          $SSH "$T" "
            echo '--- Dienst';        systemctl is-active wordclock-logger.service
            echo '--- Groesse';       du -h '$FILE' | cut -f1
            echo '--- Zeilen';        wc -l < '$FILE'
            a=\$(date -d \"\$(head -1 '$FILE' | cut -d' ' -f1)\" +%s 2>/dev/null)
            b=\$(date -d \"\$(tail -1 '$FILE' | cut -d' ' -f1)\" +%s 2>/dev/null)
            n=\$(wc -l < '$FILE'); sz=\$(stat -c%s '$FILE'); d=\$(( b - a ))
            if [ \"\${d:-0}\" -gt 0 ]; then
              echo \"--- Rate ueber \$(( d / 60 )) min Laufzeit\"
              echo \"\$(( sz / d )) Byte/s  =  \$(( sz * 864 / d / 10000 )) MB/Tag  =  \$(( n * 60 / d )) Zeilen/min\"
            else
              echo '--- Rate: Laufzeit zu kurz'
            fi
            echo '--- Platz';         df -h / | tail -1" ;;
  gaps)   MS=${1:-500}
          # Zeitstempel in Millisekunden umrechnen und Abstaende suchen. Eine
          # Luecke im Log heisst: der Hauptloop hat in dieser Zeit nichts
          # ausgegeben -- der beste Hinweis auf eine Blockade.
          $SSH "$T" "tail -n ${2:-20000} '$FILE'" | python3 -c "
import sys, re
from datetime import datetime
prev=None; prevline=''
for ln in sys.stdin:
    m=re.match(r'(\S+?)[ ]', ln)
    if not m: continue
    try: t=datetime.fromisoformat(m.group(1))
    except ValueError: continue
    if prev is not None:
        d=(t-prev).total_seconds()*1000
        if d >= $MS:
            print(f'{d:9.0f} ms  zwischen')
            print(f'             {prevline.rstrip()}')
            print(f'             {ln.rstrip()}')
    prev=t; prevline=ln
" ;;
  *)      sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//' ;;
esac
