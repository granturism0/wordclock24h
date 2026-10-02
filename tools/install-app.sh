#!/usr/bin/env bash
# Laedt die PWA-Assets ins LittleFS des Geraets.
#
#   ./tools/install-app.sh              laedt hoch
#   ./tools/install-app.sh --check      zeigt nur, was auf dem Geraet liegt
#
# Die Adresse der Uhr kommt aus tools/device.conf oder aus DEVICE_HOST.
#
# Warum es das braucht: tools/deploy.sh bringt die Assets auf den UPDATE-SERVER, nicht
# auf die Uhr. Dort liegen sie im LittleFS und muessen eigens hochgeladen werden -- und
# genau das ist lange nicht passiert: Am 02.10.2026 lief auf dem Geraet noch 1.4.69,
# waehrend im Repo und auf dem Server 1.4.71 stand. Saemtliche PWA-Korrekturen der
# vorangegangenen Tage waren damit nirgends wirksam.
#
# /api/update_download_assets hilft dabei NICHT: Der Name legt es nahe, aber der
# Endpunkt laedt ausschliesslich die Icon- und Wetterdatei nach (http.cpp:7987). Fuer
# die App-Dateien gibt es /api/app_file_upload, eine Datei je Aufruf.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

# Die Adresse der Uhr steht NICHT im Repo (oeffentlich) -- sie kommt aus
# tools/device.conf (gitignored, Vorlage: tools/device.conf.example) oder aus der
# Umgebung.
[ -f tools/device.conf ] && . tools/device.conf
HOST=${DEVICE_HOST:-}
if [ -z "$HOST" ]; then
  echo "DEVICE_HOST fehlt." >&2
  echo "tools/device.conf anlegen (Vorlage: tools/device.conf.example)" >&2
  echo "oder DEVICE_HOST=<ip> voranstellen." >&2
  exit 2
fi
U="http://$HOST"
D=ESP8266/ESP-uclock/data/app

# Reihenfolge und Pfade muessen zu APP_INSTALL_ASSETS in http.cpp passen; der ESP
# weist alles andere ab.
ASSETS="app/index.html app/styles.css app/layout-previews.json \
        app/icons/icon-192.svg app/icons/icon-512.svg \
        app/icons/icon-192.png app/icons/icon-512.png \
        app/icons/icon-180.png app/icons/icon-mask.png \
        app/manifest.webmanifest app/app.js app/sw.js"

device_version() {
  curl -s --compressed -m 20 "$U/app/app.js" 2>/dev/null \
    | grep -o 'const APP_VERSION = "[^"]*"' | head -1 | sed 's/.*"\(.*\)"/\1/'
}
local_version() {
  grep -m1 '^const APP_VERSION' "$D/app.js" | sed 's/.*"\(.*\)".*/\1/'
}

printf '=== PWA-Installation auf %s ===\n\n' "$HOST"
printf '  lokal:  %s\n  Geraet: %s\n\n' "$(local_version)" "$(device_version)"

if [ "${1:-}" = "--check" ]; then
  exit 0
fi

# Vor dem Hochladen pruefen: Eine leere .gz wuerde einen weissen Bildschirm erzeugen,
# und danach waere die Oberflaeche nicht mehr bedienbar, um es zu korrigieren.
for a in $ASSETS; do
  src="$D/${a#app/}.gz"
  [ -f "$src" ]                        || { echo "  ABBRUCH: $src fehlt — erst 'make app-gz'"; exit 1; }
  [ "$(wc -c < "$src" | tr -d ' ')" -gt 0 ] || { echo "  ABBRUCH: $src ist leer"; exit 1; }
done

i=0; total=$(set -- $ASSETS; echo $#); fail=0
for a in $ASSETS; do
  i=$((i+1))
  src="$D/${a#app/}.gz"
  r=$(curl -s -m 60 -X POST --data-binary "@$src" \
        -H "Content-Type: application/octet-stream" \
        "$U/api/app_file_upload?filename=$a&encoding=gzip&step=$i&total=$total" 2>&1 | head -c 120)
  printf '  %-26s %8s Byte  %s\n' "$a" "$(wc -c < "$src" | tr -d ' ')" "$r"
  case "$r" in *'"ok":true'*) ;; *) fail=$((fail+1));; esac
  sleep 1
done

echo
dv=$(device_version); lv=$(local_version)
if [ "$fail" -gt 0 ]; then
  echo "  $fail Datei(en) fehlgeschlagen."
  exit 1
elif [ "$dv" != "$lv" ]; then
  echo "  Hochgeladen, aber das Geraet meldet weiterhin $dv statt $lv."
  exit 1
fi
echo "  Geraet laeuft jetzt auf $dv."
