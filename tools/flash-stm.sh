#!/usr/bin/env bash
# Flasht die STM32-Firmware vom Update-Server und loest danach den Reset aus.
#
#   ./tools/flash-stm.sh            flashen und zuruecksetzen
#   ./tools/flash-stm.sh --check    nur pruefen, ob alles bereitliegt
#   ./tools/flash-stm.sh --lokal <hex> [<erwartete Version>]
#                                   flasht eine LOKALE Hex-Datei (Messbau, Paket 2026-10-13 §2.3)
#                                   ueber /api/local_stm32_upload und /api/local_stm32_flash --
#                                   NICHT ueber den Update-Server, damit ein Messbau nie zur
#                                   Quelle eines naechsten Updates wird. Ohne Smoketest (die
#                                   Version eines Messbaus weicht ab); zurueck mit --gleich.
#   ./tools/flash-stm.sh --gleich   auch flashen, wenn Geraet und Server gleich stehen --
#                                   nur als Probe des Flashwegs (z. B. nach einem ESP-Update,
#                                   Paket 2026-10-06 F.11). Sonst schuetzt die Gleichheit davor,
#                                   ohne Gegenwert zu flashen.
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
GLEICH=0
[ "${1:-}" = "--check" ] && CHECK_ONLY=1
[ "${1:-}" = "--gleich" ] && GLEICH=1
LOKAL=0
if [ "${1:-}" = "--lokal" ]; then
  LOKAL=1; HEX=${2:-}; ERWARTET=${3:-}
  [ "$ERWARTET" = "--check" ] && { ERWARTET=""; CHECK_ONLY=1; }
  [ "${4:-}" = "--check" ] && CHECK_ONLY=1
  [ -s "$HEX" ] || { echo "--lokal: Hex-Datei fehlt oder ist leer: ${HEX:-<keine>}" >&2; exit 2; }
fi

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
# Nicht "ist etwas eingestellt", sondern "ist DAS eingestellt, wohin wir ausrollen".
# Am 03.10.2026 stand hier test7 statt test8, dort lag eine Firmware 3.2.4 -- der Flash
# gelang, meldete Erfolg, und die Uhr fiel um Monate zurueck (BEFUNDE.md, L124).
# Ein falscher Pfad ist gefaehrlicher als ein leerer: Er traegt oft eine AELTERE,
# lauffaehige Firmware.
./tools/check-update-source.sh || fail "Update-Quelle stimmt nicht — siehe oben"

us=$(curl -s -m 20 "$U/api/update_status" 2>/dev/null)
name=$(printf '%s' "$us" | python3 -c 'import json,sys; print(json.load(sys.stdin).get("stm32_default",""))' 2>/dev/null)
[ -n "$name" ] || fail "Das Geraet meldet keinen stm32_default — ohne Dateinamen flasht der Endpunkt nichts"

# ------------------------------------------------- lokaler Flash (Messbau, AKZ.12)
if [ "$LOKAL" -eq 1 ]; then
  hsize=$(wc -c < "$HEX" | tr -d ' ')
  printf '  Lokale Datei           %s (%s Byte), hochgeladen als %s\n' "$HEX" "$hsize" "$name"
  fi=$(curl -s -m 20 "$U/api/fs_info" 2>/dev/null)
  frei=$(printf '%s' "$fi" | python3 -c 'import json,sys; d=json.load(sys.stdin); print(int(d["total"])-int(d["used"]))' 2>/dev/null)
  [ -n "$frei" ] || fail "fs_info liefert keinen freien Platz"
  printf '  LittleFS frei          %s Byte\n' "$frei"
  [ "$frei" -gt $((hsize + 65536)) ] || fail "zu wenig Platz im LittleFS ($frei frei, $hsize noetig plus 64 KiB Reserve)"
  if [ "$CHECK_ONLY" -eq 1 ]; then echo; echo "  Bereit: lokal $HEX"; exit 0; fi
  ./tools/logger/log.sh mark "STM-Flash LOKAL $cur -> ${ERWARTET:-?} ($HEX)" >/dev/null 2>&1
  echo; echo "=== Hochladen (lokal) ==="
  up=$(curl -s -m 120 -X POST -H 'Content-Type: application/octet-stream' --data-binary @"$HEX" \
        "$U/api/local_stm32_upload?filename=$name" 2>/dev/null)
  printf '  Antwort                %s\n' "$up"
  case "$up" in *'"ok":true'*) ;; *) fail "Upload abgewiesen ($up) — error 2 heisst: Dateiname passt nicht zur Hardware";; esac
  echo; echo "=== Flashen (lokal) ==="
  curl -s -m 300 "$U/api/local_stm32_flash" >/dev/null 2>&1
  for i in $(seq 1 40); do
    sleep 10
    r=$(curl -s -m 8 "$U/api/device_ready" 2>/dev/null)
    case "$r" in *'"ok":true'*) printf '  ESP wieder erreichbar nach %s s\n' "$((i*10))"; break;; esac
    printf '  warte... (%s s)\n' "$((i*10))"
  done
  echo; echo "=== STM zuruecksetzen ==="
  curl -s -m 30 "$U/api/maintenance_reset_stm32" | head -c 80; echo
  sleep 12
  got=$(curl -s -m 20 "$U/api/settings_xml" 2>/dev/null | grep -o '<strvar idx="1" value="[^"]*"' | sed 's/.*value="//;s/"//')
  printf '  STM meldet jetzt       %s\n' "${got:-nichts}"
  liegt=$(curl -s -m 20 "$U/api/fs_list" 2>/dev/null | grep -c '"stm32.hex"')
  [ "$liegt" -gt 0 ] && echo "  Hinweis: stm32.hex liegt weiter im LittleFS (wird nicht geloescht -- fs_remove ist ein Gefahrenendpunkt)"
  if [ -n "$ERWARTET" ] && [ "$got" != "$ERWARTET" ]; then
    printf '  erwartet war %s\n' "$ERWARTET" >&2; exit 1
  fi
  echo "  Kein Smoketest: ein Messbau meldet eine abweichende Version. Zurueck mit --gleich."
  exit 0
fi

printf '  Update-Quelle          %s/%s\n' "$uh" "$up"
printf '  Dateiname laut Geraet  %s\n' "$name"

code=$(curl -s -o /dev/null -w '%{http_code}' -m 25 "http://$uh/$up/$name" 2>/dev/null)
size=$(curl -s -o /dev/null -w '%{size_download}' -m 25 "http://$uh/$up/$name" 2>/dev/null)
[ "$code" = "200" ] || fail "Server liefert $name nicht aus (HTTP $code)"
printf '  Auf dem Server         HTTP 200, %s Byte\n' "$size"

want=$(curl -s -m 20 "http://$uh/$up/wc.txt" 2>/dev/null | tr -d '\r\n')
printf '  Version auf dem Server %s\n' "${want:-unbekannt}"

if [ "$want" = "$cur" ] && [ "$GLEICH" -eq 0 ]; then
  echo
  echo "  Geraet und Server stehen beide auf $cur — nichts zu flashen."
  echo "  (Probe des Flashwegs trotzdem: --gleich)"
  exit 0
fi
[ "$want" = "$cur" ] && echo "  --gleich: flashe denselben Stand $cur als Probe des Flashwegs"

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
