#!/usr/bin/env bash
# Probe U (Paket 2026-10-15, AKW.3): Wird eine App-Datei auf dem Geraet ersetzt, kommt sie beim naechsten Laden sofort
# neu an -- mit 200 und neuem ETag --, waehrend alle anderen Dateien 304 bleiben. Danach das Original zurueck: 200 mit
# dem urspruenglichen ETag.
#
#   ./tools/probe-u.sh                     gegen DEVICE_HOST aus tools/device.conf
#   PROBE_U_OHNE_UPLOAD=1 ./tools/probe-u.sh   Gegenprobe (DIR-014): nichts hochladen -- muss FEHL melden
#
# Ablauf: Ladeprobe mit 5 warmen Ladungen in EINEM Profil. Vor warm #2 wird layout-previews.json mit angehaengtem
# Zeilenumbruch hochgeladen (gueltiges JSON, anderer Inhalt, also anderes ETag), vor warm #4 das Original aus dem Repo.
# Erwartet:  #1 304 (Original)  #2 200 (neu)  #3 304 (neu)  #4 200 (Original)  #5 304 (Original); alle anderen /app/ 304.
# Das Original wird beim Verlassen IMMER zurueckgelegt, danach install-app.sh --check.
#
# SCHREIBT AM GERAET: eine App-Datei (app_file_upload), zweimal, und stellt sie wieder her.
# Exit 0 bestanden, 1 Fehlschlag, 2 Werkzeugfehler/Vorbedingung.
set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2
[ -f tools/device.conf ] && . tools/device.conf
HOST=${DEVICE_HOST:-}; [ -n "$HOST" ] || { echo "DEVICE_HOST fehlt (tools/device.conf)." >&2; exit 2; }
U="http://$HOST"
PFAD=/app/layout-previews.json
ASSET=app/layout-previews.json
ORIG=ESP8266/ESP-uclock/data/app/layout-previews.json.gz

etag_von() {   # "<groesse>-<16 hex der letzten 8 Byte>" wie http_app_etag () (Variante T)
  python3 -c "import sys; b=open(sys.argv[1],'rb').read(); print('\"%d-%s\"' % (len(b), b[-8:].hex()) if len(b) >= 18 and b[:2] == b'\x1f\x8b' else '')" "$1"
}
lade_hoch() {  # $1 Datei
  local r; r=$(curl -s -m 60 -X POST --data-binary "@$1" -H "Content-Type: application/octet-stream" \
    "$U/api/app_file_upload?filename=$ASSET&encoding=gzip" 2>&1 | head -c 200)
  case "$r" in *'"ok":true'*) return 0 ;; *) echo "  Upload fehlgeschlagen: $r" >&2; return 1 ;; esac
}

[ -s "$ORIG" ] || { echo "ABBRUCH: $ORIG fehlt oder ist leer -- erst make app-gz" >&2; exit 2; }
E_ORIG=$(etag_von "$ORIG"); [ -n "$E_ORIG" ] || { echo "ABBRUCH: $ORIG ist keine gzip-Datei" >&2; exit 2; }
E_GERAET=$(curl -s -m 10 -D - -o /dev/null -H 'Accept-Encoding: gzip' "$U$PFAD" | tr -d '\r' | grep -i '^etag:' | sed 's/^[Ee][Tt][Aa][Gg]: *//')
[ -n "$E_GERAET" ] || { echo "ABBRUCH: Geraet liefert kein ETag fuer $PFAD (ESP vor 3.2.40?)" >&2; exit 2; }
[ "$E_GERAET" = "$E_ORIG" ] || { echo "ABBRUCH: ETag auf dem Geraet $E_GERAET passt nicht zum Repo $E_ORIG -- erst install-app.sh" >&2; exit 2; }

WORK=$(mktemp -d "${TMPDIR:-/tmp}/probe-u.XXXXXX") || exit 2
GEAENDERT=0
aufraeumen() {
  if [ "$GEAENDERT" = 1 ]; then
    if lade_hoch "$ORIG"; then echo "  Original zurueckgelegt"; else echo "  WARNUNG: Original NICHT zurueckgelegt -- ./tools/install-app.sh"; fi
  fi
  case "$WORK" in "${TMPDIR:-/tmp}"*probe-u.*) rm -rf "${WORK:?}" ;; esac
}
trap aufraeumen EXIT
trap 'exit 2' INT TERM HUP

gunzip -c "$ORIG" > "$WORK/neu.json" && printf '\n' >> "$WORK/neu.json" && gzip -9 -n -c "$WORK/neu.json" > "$WORK/neu.gz" || exit 2
python3 -c "import json,sys; json.load(open(sys.argv[1]))" "$WORK/neu.json" || { echo "ABBRUCH: geaenderte Datei ist kein JSON" >&2; exit 2; }
E_NEU=$(etag_von "$WORK/neu.gz")
[ -n "$E_NEU" ] && [ "$E_NEU" != "$E_ORIG" ] || { echo "ABBRUCH: kein neues ETag ($E_NEU)" >&2; exit 2; }
echo "=== Probe U gegen $HOST — $(date '+%Y-%m-%d %H:%M:%S') ==="
echo "    ETag Original $E_ORIG, geaendert $E_NEU"

cat > "$WORK/haken.sh" <<EOF
#!/bin/sh
[ -n "\${PROBE_U_OHNE_UPLOAD:-}" ] && { echo "    (Gegenprobe: kein Upload vor warm #\$1)"; exit 0; }
case "\$1" in
  2) curl -s -m 60 -X POST --data-binary "@$WORK/neu.gz" -H "Content-Type: application/octet-stream" "$U/api/app_file_upload?filename=$ASSET&encoding=gzip" | grep -q '"ok":true' && echo "    geaenderte Datei hochgeladen" && touch "$WORK/geaendert" ;;
  4) curl -s -m 60 -X POST --data-binary "@$PWD/$ORIG" -H "Content-Type: application/octet-stream" "$U/api/app_file_upload?filename=$ASSET&encoding=gzip" | grep -q '"ok":true' && echo "    Original hochgeladen" && rm -f "$WORK/geaendert" ;;
  *) exit 0 ;;
esac
EOF
chmod +x "$WORK/haken.sh"
GEAENDERT=1     # ab hier legt der Ausgang das Original sicherheitshalber zurueck

LADEPROBE_WARM=5 LADEPROBE_HAKEN="$WORK/haken.sh" LADEPROBE_ETAG_PFAD="$PFAD" LADEPROBE_AUSNAHME="$PFAD" \
  LADEPROBE_MITSCHRIFT="$WORK/mitschrift.txt" ./tools/ladeprobe.sh --erwarte-304 > "$WORK/lauf.txt" 2>&1
rc=$?
cat "$WORK/lauf.txt"
[ "$rc" -eq 2 ] && { echo "ABBRUCH: Ladeprobe Werkzeugfehler"; exit 2; }

FEHL=0
pruefe() {   # $1 Nummer, $2 erwarteter Status, $3 erwartetes ETag, $4 Name
  local z; z=$(awk -v n="#$1" '$2==n{f=1;next} /^[0-9:]+  #/{f=0} f && /ETag \/app\/layout-previews.json:/' "$WORK/lauf.txt" | head -1)
  local e s; e=$(printf '%s' "$z" | sed -E 's/.*: (.*) \((.*)\)$/\1/'); s=$(printf '%s' "$z" | sed -E 's/.*\((.*)\)$/\1/')
  if [ "$s" = "$2" ] && [ "$e" = "$3" ]; then echo "  OK    warm #$1: $s, ETag $e ($4)"
  else echo "  FEHL  warm #$1: ${s:-?}, ETag ${e:--} -- erwartet $2, $3 ($4)"; FEHL=$((FEHL+1)); fi
}
echo "--- Auswertung"
pruefe 1 304 "$E_ORIG" "vor dem Austausch"
pruefe 2 200 "$E_NEU"  "nach dem Austausch: sofort neu"
pruefe 3 304 "$E_NEU"  "danach wieder 304"
pruefe 4 200 "$E_ORIG" "Original zurueck"
pruefe 5 304 "$E_ORIG" "danach wieder 304"
[ "$rc" -eq 0 ] || { echo "  FEHL  Ladeprobe meldet Fehlschlaege (andere Dateien nicht 304?)"; FEHL=$((FEHL+1)); }

aufraeumen; GEAENDERT=0; trap - EXIT
echo "--- install-app.sh --check"
./tools/install-app.sh --check | tail -3 || FEHL=$((FEHL+1))
echo "=== Probe U: $FEHL Fehlschlag/-schlaege ==="
[ "$FEHL" -eq 0 ] && exit 0 || exit 1
