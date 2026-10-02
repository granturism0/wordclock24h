#!/usr/bin/env bash
# Rastert die App-Icons aus den SVG-Quellen.
#
#   ./tools/make-icons.sh
#
# Warum ueberhaupt PNG: iOS wertet <link rel="apple-touch-icon"> nur mit PNG aus.
# Zeigt es auf ein SVG, legt Safari beim "Zum Home-Bildschirm" NICHT das Icon ab,
# sondern einen verkleinerten Bildschirmabzug der Seite -- auf dem dunklen Layout
# ein unleserlicher blauer Fleck. Dasselbe gilt fuer aeltere Android-Launcher, die
# SVG-Eintraege im Manifest ueberspringen.
#
# Zusaetzlich entsteht ein eigenes Maskable-Icon. Android beschneidet maskable Icons
# auf einen Kreis mit 80 % Durchmesser. Beim bestehenden Motiv liegt der gruene Punkt
# bei (150|44), also 85 px von der Mitte -- ausserhalb der sicheren Zone von 76,8 px,
# er wuerde angeschnitten. Fuer diese Variante wird das Motiv auf 72 % verkleinert.
#
# Der kurze Name "icon-mask.png" ist kein Geschmacksfrage: Das LittleFS des ESP meldet
# maxPathLength 32, und die Ablage flacht "app/icons/..." zu "app-icons-..." ab. Aus
# icon-maskable-512.png waeren 34 Zeichen geworden -- die Datei haette sich nicht
# speichern lassen, und zwar erst auf dem Geraet, nicht hier.
#
# Gerastert wird mit demselben Chrome, den auch tools/preview/shot.sh benutzt. Eine
# zusaetzliche Abhaengigkeit (rsvg, ImageMagick) waere dafuer zu viel.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

CHROME=${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}
ICONS=ESP8266/ESP-uclock/data/app/icons
WAIT_MAX=${WAIT_MAX:-30}

if [ ! -x "$CHROME" ]; then
  echo "Chrome nicht gefunden: $CHROME" >&2
  echo "Pfad per CHROME=... ueberschreiben." >&2
  exit 2
fi

TMP=$(mktemp -d)
chmod 755 "$TMP"
trap 'rm -rf "$TMP"' EXIT

# $1 Quell-SVG  $2 Zieldatei  $3 Kantenlaenge  $4 Skalierung des Motivs (1 = voll)
render() {
  local src=$1 out=$2 size=$3 scale=$4
  local name inner page shot inner_px waited
  name=$(basename "$out")
  page="$TMP/$name.html"
  # Chrome schreibt den Abzug NICHT direkt ins Projekt: Unter einer Sandbox, die
  # Schreibzugriffe einzeln freigibt, haengt der Prozess dort stumm. Erst in den
  # Temporaerordner, dann kopieren -- das Kopieren macht die Shell.
  shot="$TMP/$name"

  inner=$(sed '/<!--/,/-->/d' "$src")
  inner_px=$(awk "BEGIN{printf \"%d\", $size * $scale}")

  {
    printf '<!doctype html><meta charset="utf-8"><style>'
    printf 'html,body{margin:0;padding:0}'
    printf '.wrap{width:%spx;height:%spx;background:#07101a;display:grid;place-items:center}' "$size" "$size"
    printf '.wrap > svg{width:%spx;height:%spx;display:block}' "$inner_px" "$inner_px"
    printf '</style><div class="wrap">%s</div>' "$inner"
  } > "$page"

  "$CHROME" --headless=new --disable-gpu --no-sandbox --hide-scrollbars \
    --user-data-dir="$TMP/profile-$name" --virtual-time-budget=3000 \
    --window-size="$size","$size" --screenshot="$shot" "file://$page" >/dev/null 2>&1 &
  local pid=$!

  # Chrome beendet sich nach --screenshot nicht zuverlaessig (dieselbe Eigenheit,
  # die tools/preview/shot.sh beschreibt). Deshalb auf die DATEI warten, nicht auf
  # den Prozess, und dann beenden.
  waited=0
  while [ "$waited" -lt "$WAIT_MAX" ]; do
    [ -s "$shot" ] && break
    sleep 1
    waited=$((waited + 1))
  done
  sleep 1
  kill $pid 2>/dev/null
  wait $pid 2>/dev/null

  if [ ! -s "$shot" ]; then
    printf '  %-28s FEHLGESCHLAGEN (keine Ausgabe nach %s s)\n' "$name" "$WAIT_MAX"
    return 1
  fi

  cp "$shot" "$out" || return 1
  printf '  %-28s %sx%s  %s Byte\n' "$name" "$size" "$size" "$(wc -c < "$out" | tr -d ' ')"
}

echo "=== App-Icons rastern ==="
fail=0
render "$ICONS/icon-192.svg" "$ICONS/icon-192.png"          192 1    || fail=1
render "$ICONS/icon-512.svg" "$ICONS/icon-512.png"          512 1    || fail=1
render "$ICONS/icon-192.svg" "$ICONS/icon-180.png"          180 1    || fail=1
render "$ICONS/icon-512.svg" "$ICONS/icon-mask.png"         512 0.72 || fail=1

echo
if [ "$fail" -gt 0 ]; then
  echo "Mindestens ein Icon fehlt."
  exit 1
fi
echo "Danach:  make app-gz  und  ./tools/install-app.sh"
