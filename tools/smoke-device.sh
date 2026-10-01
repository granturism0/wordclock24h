#!/usr/bin/env bash
# Prueft nach einem Release, ob das Geraet noch antwortet wie erwartet.
#
#   ./tools/smoke-device.sh              gegen die Uhr unter DEVICE_HOST
#   DEVICE_HOST=192.168.1.184 ./tools/smoke-device.sh
#
# Warum es das braucht: Die elf Guardrail-Stufen sind ausnahmslos STATISCH. Sie lesen
# Quelltext, sie sprechen nicht mit dem Geraet. Nach einer Aenderung an http.cpp --
# einer Datei mit ueber 11'000 Zeilen, deren Parameter-Auswertung JEDER Request
# durchlaeuft -- sagt das nichts darueber, ob die Uhr noch funktioniert.
#
# Geprueft wird nur LESEND. Schreibende und destruktive Endpunkte bleiben aussen vor
# (DIR-008): Dies ist die Uhr des Nutzers im Dauerbetrieb, kein Testgeraet.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

HOST=${DEVICE_HOST:-192.168.1.184}
U="http://$HOST"
TIMEOUT=${TIMEOUT:-10}

OK=0; FAIL=0
pass() { printf '  OK    %-34s %s\n' "$1" "${2:-}"; OK=$((OK+1)); }
fail() { printf '  FEHLT %-34s %s\n' "$1" "${2:-}"; FAIL=$((FAIL+1)); }

printf '=== Geraete-Smoketest gegen %s — %s ===\n\n' "$HOST" "$(date '+%Y-%m-%d %H:%M')"

# ---------------------------------------------------------------- Erreichbarkeit
code=$(curl -s -m "$TIMEOUT" -o /dev/null -w '%{http_code}' "$U/api/device_ready" 2>/dev/null)
if [ "$code" != "200" ]; then
  echo "  ABBRUCH: Geraet antwortet nicht (HTTP ${code:-keine Antwort})"
  exit 1
fi
pass "Erreichbarkeit" "HTTP 200"

# ------------------------------------------------------------------- Versionen
sx=$(curl -s -m "$TIMEOUT" "$U/api/settings_xml" 2>/dev/null)
stm=$(printf '%s' "$sx" | grep -o '<strvar idx="1" value="[^"]*"' | sed 's/.*value="//;s/"//')
[ -n "$stm" ] && pass "STM-Version gemeldet" "$stm" || fail "STM-Version gemeldet"

esp=$(curl -s -m "$TIMEOUT" "$U/update" 2>/dev/null | grep -oE '3\.[0-9]+\.[0-9]+' | sed -n 2p)
[ -n "$esp" ] && pass "ESP-Version gemeldet" "$esp" || fail "ESP-Version gemeldet"

# -------------------------------------------------------- PWA-Assets ausliefern
for a in "" app.js styles.css index.html sw.js manifest.webmanifest; do
  path="/app/$a"
  read -r c s < <(curl -s -m "$TIMEOUT" -o /dev/null -w '%{http_code} %{size_download}' "$U$path" 2>/dev/null)
  if [ "$c" = "200" ] && [ "${s:-0}" -gt 0 ]; then
    pass "Asset $path" "$s Byte"
  else
    fail "Asset $path" "HTTP $c, $s Byte"
  fi
done

# ------------------------------------------------- Lesende API, Antwort gueltig
# Nur Endpunkte ohne Seiteneffekt. Die Liste ist bewusst von Hand gepflegt statt aus
# app.js erzeugt: Dort stehen auch alle schreibenden, und eine Automatik, die sich
# vertut, loest hier echte Aktionen aus.
# Nicht alle antworten strukturiert: display_power und ambilight_power liefern reinen
# Text ("on"/"off"), overlay_icons ein JSON-ARRAY mit '['. Eine Pruefung, die nur '{'
# und '<' kennt, meldet dort Fehler, wo keine sind -- genau das ist beim ersten Lauf
# passiert. Deshalb je Endpunkt die erwartete Form.
check_api() {                                               # $1 Endpunkt  $2 erwartete Form
  local body code data
  body=$(curl -s -m "$TIMEOUT" -w '\n%{http_code}' "$U/api/$1" 2>/dev/null)
  code=$(printf '%s' "$body" | tail -1)
  data=$(printf '%s' "$body" | sed '$d')

  if [ "$code" != "200" ]; then
    fail "/api/$1" "HTTP $code"
    return
  fi

  case "$2" in
    json)  printf '%s' "$data" | head -c1 | grep -q '[{[]' \
             && pass "/api/$1" "$(printf '%s' "$data" | wc -c | tr -d ' ') Byte" \
             || fail "/api/$1" "kein JSON: $(printf '%s' "$data" | head -c 30)" ;;
    xml)   printf '%s' "$data" | head -c1 | grep -q '<' \
             && pass "/api/$1" "$(printf '%s' "$data" | wc -c | tr -d ' ') Byte" \
             || fail "/api/$1" "kein XML" ;;
    onoff) printf '%s' "$data" | tr -d ' \r\n' | grep -qE '^(on|off)$' \
             && pass "/api/$1" "$(printf '%s' "$data" | tr -d ' \r\n')" \
             || fail "/api/$1" "weder on noch off: $(printf '%s' "$data" | head -c 20)" ;;
  esac
}

check_api device_ready       json
check_api settings_xml       xml
check_api display_power      onoff
check_api ambilight_power    onoff
check_api power_status       json
check_api update_status      json
check_api update_table_files json
check_api fs_info            json
check_api fs_list            json
check_api stm32_log          json
check_api overlay_icons      json

# --------------------------------------------------- Legacy bleibt erreichbar
code=$(curl -s -m "$TIMEOUT" -o /dev/null -w '%{http_code}' "$U/" 2>/dev/null)
[ "$code" = "200" ] && pass "Legacy-Oberflaeche" "HTTP 200" || fail "Legacy-Oberflaeche" "HTTP $code"

# ------------------------------------- Parameter ohne '=' stuerzt nicht ab (3.2.3)
for v in "/?a" "/?a&b" "/api/settings_xml?x"; do
  curl -s -m "$TIMEOUT" -o /dev/null "$U$v" 2>/dev/null
  sleep 0.3
  code=$(curl -s -m 5 -o /dev/null -w '%{http_code}' "$U/api/device_ready" 2>/dev/null)
  [ "$code" = "200" ] && pass "Parameter ohne = : $v" "ESP lebt" || fail "Parameter ohne = : $v" "ESP weg"
done

# ----------------------------- Wartungsendpunkte weisen eingebettete Aufrufe ab (3.2.4)
# Geprueft wird ausschliesslich maintenance_reset_stm32 -- der einzige der drei, bei dem
# ein versehentliches Durchkommen nichts zerstoert. Mit Sec-Fetch-Dest: image MUSS er
# ablehnen; kommt stattdessen 200, setzt das lediglich den STM zurueck.
code=$(curl -s -m "$TIMEOUT" -o /dev/null -w '%{http_code}' \
  -H "Sec-Fetch-Dest: image" "$U/api/maintenance_reset_stm32" 2>/dev/null)
[ "$code" = "403" ] && pass "Wartung weist <img> ab" "HTTP 403" || fail "Wartung weist <img> ab" "HTTP $code"

# ------------------------------------------------------------------------ Fazit
printf '\n=== %d bestanden, %d fehlgeschlagen ===\n' "$OK" "$FAIL"
[ "$FAIL" -gt 0 ] && exit 1
echo "Geraet verhaelt sich wie erwartet."
exit 0
