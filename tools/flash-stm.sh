#!/usr/bin/env bash
# Flasht die STM32-Firmware vom Update-Server und loest danach den Reset aus.
#
#   ./tools/flash-stm.sh            flashen und zuruecksetzen
#   ./tools/flash-stm.sh --check    nur pruefen, ob alles bereitliegt
#
# WARUM ES DIESES SKRIPT GIBT
#
# Am 03.10.2026 habe ich /api/remote_stm32_flash ohne Parameter aufgerufen. Der
# Endpunkt hat daraufhin GAR NICHTS getan und auch nichts gemeldet -- der Aufruf
# sah erfolgreich aus, die Firmware blieb alt. Der Nutzer hat es bemerkt, nicht ich.
#
# Drei Dinge, die man dem Endpunkt nicht ansieht und die hier erzwungen werden:
#
#   1. "filename" ist PFLICHT. Fehlt er, setzt http_api_remote_stm32_flash()
#      error_code = 2 und bricht ab. Der richtige Name haengt an der erkannten
#      Hardware; das Geraet meldet ihn selbst als "stm32_default" in
#      /api/update_status -- den nehmen wir, statt die Bitmasken nachzubauen.
#
#   2. Ist HARDWARE_CONFIGURATION gleich 65535, bildet der ESP ueberhaupt keinen
#      Dateinamenfilter (http_build_stm32_default_filename) und weist JEDEN
#      Dateinamen ab. Genau diesen Zustand hinterlaesst ein ESP-Neustart, wenn der
#      STM seine Einmal-Variablen waehrend der Ausfallzeit gesendet hat
#      (BEFUNDE.md L42). Dann hilft nur: erst STM zuruecksetzen, dann flashen.
#
#   3. NACH dem Flashen muss der STM zurueckgesetzt werden. Der ESP meldet selbst
#      "STM32-Flash abgeschlossen. Warte auf Reset." -- er loest ihn aber nicht aus.
#      Ohne Reset laeuft die alte Firmware weiter, und die gemeldete Version bleibt
#      die alte. Der Reset ist hier kein Nachklapp, sondern Teil des Vorgangs.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

[ -f tools/device.conf ] && . tools/device.conf
HOST=${DEVICE_HOST:-}
if [ -z "$HOST" ]; then
  echo "DEVICE_HOST fehlt. tools/device.conf anlegen (Vorlage: tools/device.conf.example)." >&2
  exit 2
fi
U="http://$HOST"
CHECK_ONLY=0
[ "${1:-}" = "--check" ] && CHECK_ONLY=1

fail() { printf '  ABBRUCH: %s\n' "$1" >&2; exit 1; }

printf '=== STM32-Flash auf %s — %s ===\n\n' "$HOST" "$(date '+%Y-%m-%d %H:%M')"

# ------------------------------------------------------------ Vorbedingungen
sx=$(curl -s -m 20 "$U/api/settings_xml" 2>/dev/null)
[ -n "$sx" ] || fail "Geraet antwortet nicht"

hw=$(printf '%s' "$sx" | grep -o '<numvar idx="29" value="[0-9]*"' | sed 's/.*value="//;s/"//')
cur=$(printf '%s' "$sx" | grep -o '<strvar idx="1" value="[^"]*"' | sed 's/.*value="//;s/"//')
uh=$(printf '%s' "$sx"  | grep -o '<strvar idx="9" value="[^"]*"'  | sed 's/.*value="//;s/"//')
up=$(printf '%s' "$sx"  | grep -o '<strvar idx="10" value="[^"]*"' | sed 's/.*value="//;s/"//')

printf '  STM jetzt              %s\n' "${cur:-unbekannt}"
printf '  HARDWARE_CONFIGURATION %s\n' "${hw:-unbekannt}"

if [ "$hw" = "65535" ]; then
  echo
  echo "  HARDWARE_CONFIGURATION ist 65535 — der ESP hat die Geraetekennung verloren."
  echo "  In diesem Zustand bildet er keinen Dateinamenfilter und weist JEDEN Flash ab,"
  echo "  ohne einen Fehler zu melden (BEFUNDE.md, L42)."
  echo
  echo "  Zuerst:  curl -s $U/api/maintenance_reset_stm32"
  echo "  Danach rund 10 s warten und dieses Skript erneut starten."
  exit 1
fi
[ -n "$uh" ] && [ -n "$up" ] || fail "Update-Quelle am Geraet ist leer (BEFUNDE.md, L42) — erst Host und Pfad setzen"

us=$(curl -s -m 20 "$U/api/update_status" 2>/dev/null)
name=$(printf '%s' "$us" | python3 -c 'import json,sys; print(json.load(sys.stdin).get("stm32_default",""))' 2>/dev/null)
[ -n "$name" ] || fail "Das Geraet meldet keinen stm32_default — ohne Dateinamen flasht der Endpunkt nichts"

printf '  Update-Quelle          %s/%s\n' "$uh" "$up"
printf '  Dateiname laut Geraet  %s\n' "$name"

code=$(curl -s -o /dev/null -w '%{http_code}' -m 25 "http://$uh/$up/$name" 2>/dev/null)
size=$(curl -s -o /dev/null -w '%{size_download}' -m 25 "http://$uh/$up/$name" 2>/dev/null)
[ "$code" = "200" ] || fail "Server liefert $name nicht aus (HTTP $code)"
printf '  Auf dem Server         HTTP 200, %s Byte\n' "$size"

want=$(curl -s -m 20 "http://$uh/$up/wc.txt" 2>/dev/null | tr -d '\r\n')
printf '  Version auf dem Server %s\n' "${want:-unbekannt}"

if [ "$want" = "$cur" ]; then
  echo
  echo "  Geraet und Server stehen beide auf $cur — nichts zu flashen."
  exit 0
fi

if [ "$CHECK_ONLY" -eq 1 ]; then
  echo
  echo "  Bereit: $cur -> $want"
  exit 0
fi

# ------------------------------------------------------------------- Flashen
./tools/logger/log.sh mark "STM-Flash $cur -> $want ($name)" >/dev/null 2>&1

echo
echo "=== Flashen ==="
curl -s -m 300 "$U/api/remote_stm32_flash?filename=$name" >/dev/null 2>&1

# Der ESP antwortet waehrend des Flashens nicht. Auf seine Rueckkehr warten, nicht
# auf die Antwort des Aufrufs.
for i in $(seq 1 40); do
  sleep 10
  r=$(curl -s -m 8 "$U/api/device_ready" 2>/dev/null)
  case "$r" in *'"ok":true'*) printf '  ESP wieder erreichbar nach %s s\n' "$((i*10))"; break;; esac
  printf '  warte... (%s s)\n' "$((i*10))"
done

# ------------------------------------------------- Reset: Pflicht, kein Nachklapp
echo
echo "=== STM zuruecksetzen (ohne diesen Schritt laeuft die alte Firmware weiter) ==="
curl -s -m 30 "$U/api/maintenance_reset_stm32" | head -c 80
echo
sleep 12

# ------------------------------------------------------------------- Nachweis
got=$(curl -s -m 20 "$U/api/settings_xml" 2>/dev/null | grep -o '<strvar idx="1" value="[^"]*"' | sed 's/.*value="//;s/"//')
echo
if [ "$got" = "$want" ]; then
  printf '  STM laeuft jetzt auf %s\n' "$got"
else
  printf '  STM meldet weiterhin %s, erwartet war %s\n' "${got:-nichts}" "$want" >&2
  echo "  Mitschnitt pruefen:  ./tools/logger/log.sh grep 'Flash STM32'" >&2
  exit 1
fi

echo
./tools/smoke-device.sh
