#!/bin/bash
# Gegenprobe und Sabotagen fuer c70p (R2.3, nachgezogen in R2b.6). Arbeitet nur auf Temp-Kopien, nie im Baum.
# ROOT aus WORDCLOCK_ROOT, sonst aus dem eigenen Ablageort (abgelegt unter tools/checks/auszug/pwa/c70p).
SP=$(cd "$(dirname "$0")" && pwd)
ROOT=${WORDCLOCK_ROOT:-$(git -C "$SP" rev-parse --show-toplevel)} || exit 2
[ -s "$ROOT/ESP8266/ESP-uclock/data/app/app.js" ] || { echo "ROOT ohne app.js: $ROOT"; exit 2; }
T=$(mktemp -d "${TMPDIR:-/tmp}/c70p-sab.XXXXXX"); trap 'rm -rf "$T"' EXIT
BAD=0; N=0
erwarte() { # name soll(0/1) verzeichnis
  N=$((N+1))
  out=$(sh "$SP/run.sh" "$3" "$1" 2>&1); rc=$?
  printf '%s\n' "$out" | sed 's/^/    /'
  if [ "$rc" = "$2" ]; then echo "  erwartet  $1: exit $rc"; else echo "  FALSCH    $1: exit $rc, erwartet $2"; BAD=1; fi
}
kopie() { rm -rf "$T/$1"; mkdir -p "$T/$1/data/app"; cp "$ROOT/ESP8266/ESP-uclock/data/app/app.js" "$T/$1/data/app/app.js"; }
sab() { # name alt neu
  kopie "$1"
  python3 - "$T/$1/data/app/app.js" "$2" "$3" <<'PY' || exit 2
import sys
p,alt,neu=sys.argv[1:]; b=open(p,'rb').read(); a=alt.encode(); n=b.count(a)
if n!=1: sys.exit("Sabotage-Anker %r %d-mal" % (alt,n))
open(p,'wb').write(b.replace(a,neu.encode(),1))
PY
}
LAY='LAYOUT_PREVIEW_ROWS_URL, { cache: "no-cache" }'
SPR='fetchWithTimeout(url, { cache: "default" }, LANGUAGE_FETCH_TIMEOUT_MS)'
echo "Gegenprobe release/3.2.34-3.2.40-1.4.101"
mkdir -p "$T/alt"; git -C "$ROOT" archive release/3.2.34-3.2.40-1.4.101 ESP8266/ESP-uclock/data/app/app.js | tar -x -C "$T/alt" || exit 2
erwarte "1.4.101" 1 "$T/alt/ESP8266/ESP-uclock"
echo "Baum"; kopie baum; erwarte "baum" 0 "$T/baum"
echo "S1 layout-previews.json no-store zurueck"
sab s1 "$LAY" 'LAYOUT_PREVIEW_ROWS_URL, { cache: "no-store" }'; erwarte "S1" 1 "$T/s1"
echo "S2 layout-previews.json nicht mehr no-cache: Option ganz weg"
sab s2 "$LAY" 'LAYOUT_PREVIEW_ROWS_URL, {}'; erwarte "S2" 1 "$T/s2"
echo "S3 layout-previews.json reload statt no-cache"
sab s3 "$LAY" 'LAYOUT_PREVIEW_ROWS_URL, { cache: "reload" }'; erwarte "S3" 1 "$T/s3"
echo "S3b layout-previews.json nicht mehr no-cache: default"
sab s3b "$LAY" 'LAYOUT_PREVIEW_ROWS_URL, { cache: "default" }'; erwarte "S3b" 1 "$T/s3b"
echo "S4 Sprachdatei ohne ?v="
sab s4 '".json?v=" + encodeURIComponent(APP_VERSION)' '".json"'; erwarte "S4" 1 "$T/s4"
echo "S5 Sprachdatei mit ?<version> ohne = (ESP-Absturz)"
sab s5 '".json?v=" + encodeURIComponent(APP_VERSION)' '".json?" + encodeURIComponent(APP_VERSION)'; erwarte "S5" 1 "$T/s5"
echo "S6 Mitkorrektur: no-store einer anderen Stelle entfernt"
sab s6 'getFsShowBaseUrl() + encodeURIComponent(currentTable), { cache: "no-store" }' 'getFsShowBaseUrl() + encodeURIComponent(currentTable), { cache: "no-cache" }'; erwarte "S6" 1 "$T/s6"
echo "S7 Merker in loadLayoutPreviewRowsMap entfernt"
sab s7 '  if (layoutPreviewRowsMap) {
    return layoutPreviewRowsMap;
  }
  if (layoutPreviewRowsMapPromise)' '  if (layoutPreviewRowsMapPromise)'; erwarte "S7" 1 "$T/s7"
echo "S8 Sprachdatei mit no-cache"
sab s8 "$SPR" 'fetchWithTimeout(url, { cache: "no-cache" }, LANGUAGE_FETCH_TIMEOUT_MS)'; erwarte "S8" 1 "$T/s8"
echo "S9 Sprachdatei mit no-store"
sab s9 "$SPR" 'fetchWithTimeout(url, { cache: "no-store" }, LANGUAGE_FETCH_TIMEOUT_MS)'; erwarte "S9" 1 "$T/s9"
echo "S10 Marke ohne Musterpruefung uebernommen (ungueltige Marke wird verwendet)"
sab s10 'if (APP_ASSET_TOKEN_PATTERN.test(token)) {' 'if (token) {'; erwarte "S10" 1 "$T/s10"
echo "S11 Muster ohne Endanker (Marke mit Anhang wird verwendet)"
sab s11 'const APP_ASSET_TOKEN_PATTERN = /^[0-9]+-[0-9a-f]{16}$/;' 'const APP_ASSET_TOKEN_PATTERN = /^[0-9]+-[0-9a-f]{16}/;'; erwarte "S11" 1 "$T/s11"
echo "S12 Muster ohne Gross-/Kleinschreibung (Grossbuchstaben werden verwendet)"
sab s12 'const APP_ASSET_TOKEN_PATTERN = /^[0-9]+-[0-9a-f]{16}$/;' 'const APP_ASSET_TOKEN_PATTERN = /^[0-9]+-[0-9a-f]{16}$/i;'; erwarte "S12" 1 "$T/s12"
echo "S13 Parameter h ohne ="
sab s13 '"&h=" + token' '"&h" + token'; erwarte "S13" 1 "$T/s13"
echo "S14 Marke als Parameter ohne Namen und ohne ="
sab s14 '"&h=" + token' '"&" + token'; erwarte "S14" 1 "$T/s14"
echo "S15 layout-previews.json mit v und h"
sab s15 "$LAY" 'LAYOUT_PREVIEW_ROWS_URL + "?v=" + encodeURIComponent(APP_VERSION) + "&h=" + (getAppAssetTokens()["layout-previews.json"] || ""), { cache: "no-cache" }'; erwarte "S15" 1 "$T/s15"
echo "S16 layout-previews.json mit h nur bei vorhandener Marke"
sab s16 "$LAY" 'LAYOUT_PREVIEW_ROWS_URL + (getAppAssetTokens()["layout-previews.json"] ? "?h=" + getAppAssetTokens()["layout-previews.json"] : ""), { cache: "no-cache" }'; erwarte "S16" 1 "$T/s16"
echo "S17 Meta-Element bei jedem Abruf neu gelesen (Merker entfernt)"
sab s17 '  if (appAssetTokens) {
    return appAssetTokens;
  }
' ''; erwarte "S17" 1 "$T/s17"
echo "S18 Marke vom falschen Pfad gelesen"
sab s18 'getAppAssetTokens()["i18n/" + code + ".json"]' 'getAppAssetTokens()["layout-previews.json"]'; erwarte "S18" 1 "$T/s18"
echo "S19 Marke prozentkodiert statt geprueft (encodeURIComponent der Rohmarke)"
sab s19 'if (APP_ASSET_TOKEN_PATTERN.test(token)) {
      tokens[path] = token;' 'if (token) {
      tokens[path] = encodeURIComponent(token);'; erwarte "S19" 1 "$T/s19"
echo "S20 zweite Lesestelle fuer wc-asset-tokens"
sab s20 'async function ensureLanguageTable(code) {' 'const WC_TOKENS_META = typeof document !== "undefined" ? document.querySelector('"'"'meta[name="wc-asset-tokens"]'"'"') : null;
async function ensureLanguageTable(code) {'; erwarte "S20" 1 "$T/s20"
[ "$BAD" = 0 ] && echo "ALLE WIE ERWARTET ($N Laeufe)" || { echo "ABWEICHUNG ($N Laeufe)"; exit 1; }
