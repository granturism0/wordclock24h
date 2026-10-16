#!/bin/bash
# Gegenprobe und Sabotagen fuer c70p. Arbeitet nur auf Temp-Kopien, nie im Baum.
SP=$(cd "$(dirname "$0")" && pwd)
ROOT=$(git -C "$SP" rev-parse --show-toplevel) || exit 2
T=$(mktemp -d "${TMPDIR:-/tmp}/c70p-sab.XXXXXX"); trap 'rm -rf "$T"' EXIT
erwarte() { # name soll(0/1) verzeichnis
  out=$(sh "$SP/run.sh" "$3" "$1" 2>&1); rc=$?
  printf '%s\n' "$out" | sed 's/^/    /'
  if [ "$rc" = "$2" ]; then echo "  erwartet  $1: exit $rc"; else echo "  FALSCH    $1: exit $rc, erwartet $2"; BAD=1; fi
}
BAD=0
kopie() { rm -rf "$T/$1"; mkdir -p "$T/$1/data/app"; cp "$ROOT/ESP8266/ESP-uclock/data/app/app.js" "$T/$1/data/app/app.js"; }
sab() { # name python-ersetzung(alt) (neu)
  kopie "$1"
  python3 - "$T/$1/data/app/app.js" "$2" "$3" <<'PY' || exit 2
import sys
p,alt,neu=sys.argv[1:]; b=open(p,'rb').read(); a=alt.encode(); n=b.count(a)
if n!=1: sys.exit("Sabotage-Anker %r %d-mal" % (alt,n))
open(p,'wb').write(b.replace(a,neu.encode(),1))
PY
}
echo "Gegenprobe release/3.2.34-3.2.39-1.4.100"
mkdir -p "$T/alt"; git -C "$ROOT" archive release/3.2.34-3.2.39-1.4.100 ESP8266/ESP-uclock/data/app/app.js | tar -x -C "$T/alt" || exit 2
erwarte "1.4.100" 1 "$T/alt/ESP8266/ESP-uclock"
echo "Baum"; kopie baum; erwarte "baum" 0 "$T/baum"
echo "S1 no-store zurueck"
sab s1 'LAYOUT_PREVIEW_ROWS_URL, { cache: "no-cache" }' 'LAYOUT_PREVIEW_ROWS_URL, { cache: "no-store" }'; erwarte "S1" 1 "$T/s1"
echo "S2 Option ganz weg (Browser-Vorgabe default statt no-cache)"
sab s2 'LAYOUT_PREVIEW_ROWS_URL, { cache: "no-cache" }' 'LAYOUT_PREVIEW_ROWS_URL, {}'; erwarte "S2" 1 "$T/s2"
echo "S3 reload statt no-cache"
sab s3 'LAYOUT_PREVIEW_ROWS_URL, { cache: "no-cache" }' 'LAYOUT_PREVIEW_ROWS_URL, { cache: "reload" }'; erwarte "S3" 1 "$T/s3"
echo "S4 Sprachdatei ohne ?v="
sab s4 '".json?v=" + encodeURIComponent(APP_VERSION)' '".json"'; erwarte "S4" 1 "$T/s4"
echo "S5 Sprachdatei mit ?<version> ohne = (ESP-Absturz)"
sab s5 '".json?v=" + encodeURIComponent(APP_VERSION)' '".json?" + encodeURIComponent(APP_VERSION)'; erwarte "S5" 1 "$T/s5"
echo "S6 Mitkorrektur: no-store einer anderen Stelle entfernt (AKC.6 sonst keine Aenderung)"
sab s6 'getFsShowBaseUrl() + encodeURIComponent(currentTable), { cache: "no-store" }' 'getFsShowBaseUrl() + encodeURIComponent(currentTable), { cache: "no-cache" }'; erwarte "S6" 1 "$T/s6"
echo "S7 Merker entfernt (zweiter Aufruf holt erneut)"
sab s7 '  if (layoutPreviewRowsMap) {
    return layoutPreviewRowsMap;
  }
  if (layoutPreviewRowsMapPromise)' '  if (layoutPreviewRowsMapPromise)'; erwarte "S7" 1 "$T/s7"
[ "$BAD" = 0 ] && echo "ALLE WIE ERWARTET" || { echo "ABWEICHUNG"; exit 1; }
