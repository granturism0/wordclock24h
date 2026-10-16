#!/bin/sh
# Gegenproben fuer tools/app-marken.py (Paket 2026-10-15, R2b.3, AKL.4; DIR-014): je Abbruchgrund eine hergestellte
# Verletzung in einer Kopie von data/app -- jede muss Exit 1 mit Meldung ergeben, der unveraenderte Stand Exit 0.
# Arbeitet nur auf Kopien. Aufruf: tools/checks/app-marken/gegenprobe.sh   Exit 0 alle wie erwartet, 1 Abweichung.
R=$(git -C "$(dirname "$0")" rev-parse --show-toplevel) || exit 2
A="$R/ESP8266/ESP-uclock/data/app"; W="$R/tools/app-marken.py"
T=$(mktemp -d "${TMPDIR:-/tmp}/app-marken.XXXXXX") || exit 2
trap 'case "$T" in "${TMPDIR:-/tmp}"*app-marken.*) rm -rf "${T:?}";; esac' EXIT
BAD=0; N=0
probe() {  # name, erwarteter Exit, befehl-zur-verletzung (in $K)
  K="$T/$1"; mkdir -p "$K"; cp -R "$A/." "$K/"; (cd "$K" && eval "$3")
  out=$(python3 "$W" "$K" "$K/kopie.html" 2>/dev/null); rc=$?; N=$((N+1))
  if [ "$rc" = "$2" ] && { [ "$2" = 0 ] || printf '%s' "$out" | grep -q "ABBRUCH"; }; then echo "  ok    $1: Exit $rc  ${out#app-marken: }"
  else echo "  FALSCH $1: Exit $rc, erwartet $2 -- $out"; BAD=1; fi
  [ "$2" != 0 ] && [ -e "$K/kopie.html" ] && { echo "  FALSCH $1: Kopie trotz Abbruch geschrieben"; BAD=1; }
}
probe unveraendert 0 ':'
probe gz-fehlt 1 'rm styles.css.gz'
probe gz-leer 1 ': > app.js.gz'
probe kein-gzip 1 'printf "kein gzip, aber lang genug fuer achtzehn" > manifest.webmanifest.gz'
probe verweis-mit-query 1 'sed -i "" "s|href=\"/app/styles.css\"|href=\"/app/styles.css?x=1\"|" index.html'
probe verweis-fehlt 1 'sed -i "" "s|src=\"/app/app.js\"|src=\"/app/app2.js\"|" index.html'
probe verweis-doppelt 1 'sed -i "" "s|</head>|<link rel=\"stylesheet\" href=\"/app/styles.css\"></head>|" index.html'
probe meta-fehlt 1 'sed -i "" "/wc-asset-tokens/d" index.html'
probe meta-nicht-leer 1 'sed -i "" "s|name=\"wc-asset-tokens\" content=\"\"|name=\"wc-asset-tokens\" content=\"x\"|" index.html'
probe sprache-gz-fehlt 1 'rm i18n/en.json.gz'
probe layout-mit-marke 1 'sed -i "" "s|</head>|<link rel=\"preload\" href=\"/app/layout-previews.json?v=1\"></head>|" index.html'
probe version-falsch 1 'sed -i "" "s|^const APP_VERSION = \"[^\"]*\";|const APP_VERSION = \"1.4.x\";|" app.js'
probe version-doppelt 1 'printf "\nconst APP_VERSION = \"9.9.9\";\n" >> app.js'
# AKL.5 (Review R2b.9 SOLL-1): ein sabotiertes Werkzeug, das "?v<version>" ohne "=" erzeugt, muss sich selbst abweisen.
WS="$T/app-marken-sabotiert.py"; sed 's|?v={version}&amp;h={m}|?v{version}\&amp;h={m}|' "$W" > "$WS"
if cmp -s "$W" "$WS"; then echo "  FALSCH query-ohne-gleich: Sabotage-Anker nicht gefunden"; BAD=1
else
  K="$T/q"; mkdir -p "$K"; cp -R "$A/." "$K/"; out=$(python3 "$WS" "$K" "$K/kopie.html" 2>/dev/null); rc=$?; N=$((N+1))
  if [ "$rc" = 1 ] && printf '%s' "$out" | grep -q "verletzt AKL.5" && [ ! -e "$K/kopie.html" ]; then echo "  ok    query-ohne-gleich: Exit 1  ${out#app-marken: }"
  else echo "  FALSCH query-ohne-gleich: Exit $rc -- $out"; BAD=1; fi
fi
echo "app-marken Gegenprobe: $N Faelle"
[ "$BAD" = 0 ] && { echo "ALLE WIE ERWARTET"; exit 0; } || { echo "ABWEICHUNG"; exit 1; }
