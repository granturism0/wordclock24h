#!/usr/bin/env bash
# Prueft nach einem Release, ob das Geraet noch antwortet wie erwartet.
#
#   ./tools/smoke-device.sh              gegen die Uhr aus tools/device.conf
#   DEVICE_HOST=192.0.2.10 ./tools/smoke-device.sh
#
# Warum es das braucht: Die elf Guardrail-Stufen sind ausnahmslos STATISCH. Sie lesen
# Quelltext, sie sprechen nicht mit dem Geraet. Nach einer Aenderung an http.cpp --
# einer Datei mit ueber 11'000 Zeilen, deren Parameter-Auswertung JEDER Request
# durchlaeuft -- sagt das nichts darueber, ob die Uhr noch funktioniert.
#
# Geprueft wird nur LESEND. Schreibende und destruktive Endpunkte bleiben aussen vor
# (DIR-008): Er soll das Geraet beobachten, nicht veraendern -- auch auf der Test-Uhr,
# sonst misst er seine eigene Wirkung.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

# Marke fuer den Stop-Hook: hier wurde am GERAET gemessen. Er fragt beim Beenden
# nach, ob die Erkenntnis in BEFUNDE.md steht, falls die Datei seither unberuehrt
# blieb. Grund: Am 04.10.2026 musste der Nutzer dreimal nachfragen (L184).
mkdir -p "$(git rev-parse --git-dir 2>/dev/null)" 2>/dev/null \
  && touch "$(git rev-parse --git-dir)/geraet-gemessen" 2>/dev/null || true

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

# Aus /api/update_status, nicht mehr als "zweite Versionsnummer auf /update": Die Legacy-Seite zeigt dort
# die Release Notes vom Server, und seit dem 14.10.2026 steht darin "3.1.5" (Umstieg vom Original) --
# der Smoketest meldete damit ESP 3.1.5 und uebersprang die Pruefungen fuer E3/E4 still.
esp=$(curl -s -m "$TIMEOUT" "$U/api/update_status" 2>/dev/null | grep -o '"esp_version":"[^"]*"' | cut -d'"' -f4)
[ -n "$esp" ] && pass "ESP-Version gemeldet" "$esp" || fail "ESP-Version gemeldet"

# Gegen den Repo-Sollstand (E11/L105). Nur ein Hinweis, kein Fehlschlag: Zwischen Build
# und Flash weicht das Geraet gewollt ab, und das soll den Smoketest nicht rot machen.
# Aber man soll es SEHEN -- am 02.10.2026 lief auf dem Geraet tagelang 1.4.69, waehrend
# das Repo bei 1.4.71 stand, und niemand hat es bemerkt (CLAUDE.md, DIR-017).
repo_stm=$(grep '^#define VERSION' src/main.h | head -1 | cut -d'"' -f2)
repo_esp=$(grep '^#define ESP_VERSION' ESP8266/ESP-uclock/version.h | head -1 | cut -d'"' -f2)
for v in "STM|$stm|$repo_stm" "ESP|$esp|$repo_esp"; do
  IFS='|' read -r k ist soll <<< "$v"
  if [ -n "$ist" ] && [ -n "$soll" ] && [ "$ist" != "$soll" ]; then
    printf '  INFO  %-34s %s\n' "$k-Version gegen Repo" "Geraet $ist, Repo $soll — noch nicht eingespielt?"
  fi
done

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

# --------------------------------------- Update-Quelle stimmt noch (BEFUNDE L42)
# Verliert der ESP seinen Variablenspeicher, sind Host und Pfad LEER, und er faellt
# auf seine eingebauten Vorgaben zurueck -- die zeigen auf den Server des
# Ursprungsprojekts. Das naechste Update holte dann fremde Firmware, ohne dass
# irgendwo ein Fehler erschiene. Am 03.10.2026 genau so passiert, nach einem
# ESP-OTA; aufgefallen ist es nur, weil der Nutzer selbst nachgesehen hat.
uh=$(printf '%s' "$sx" | grep -o '<strvar idx="9" value="[^"]*"'  | sed 's/.*value="//;s/"//')
up=$(printf '%s' "$sx" | grep -o '<strvar idx="10" value="[^"]*"' | sed 's/.*value="//;s/"//')

if [ -z "$uh" ] || [ -z "$up" ]; then
  fail "Update-Quelle" "LEER — der ESP wuerde auf seine eingebaute Vorgabe zurueckfallen"
elif [ -n "${DEVICE_UPDATE_HOST:-}" ] && { [ "$uh" != "$DEVICE_UPDATE_HOST" ] || [ "$up" != "${DEVICE_UPDATE_PATH:-}" ]; }; then
  fail "Update-Quelle" "$uh/$up — erwartet $DEVICE_UPDATE_HOST/${DEVICE_UPDATE_PATH:-}"
else
  pass "Update-Quelle" "$uh/$up"
fi

# ----------------------------------- Hardwarekennung bekannt (DIR-010, AKF.4)
# Steht HARDWARE_CONFIGURATION (Index 29) auf 65535, bildet der ESP keinen
# Dateinamenfilter und weist JEDEN STM-Flash ab -- still, mit error_code. Diesen Zustand
# hinterlaesst ein ESP-Neustart, wenn der Vollabgleich die Kennung nicht durchbringt
# (BEFUNDE.md, L42). Bis zum 08.10.2026 sah man es erst beim naechsten flash-stm.sh.
# SMOKE_HW_OVERRIDE setzt fuer die Gegenprobe einen festen Wert ein (DIR-014) -- nur
# zum Pruefen der Stufe selbst, nie im echten Lauf.
hw=$(printf '%s' "$sx" | grep -o '<numvar idx="29" value="[0-9]*"' | sed 's/.*value="//;s/"//')
[ -n "${SMOKE_HW_OVERRIDE:-}" ] && hw=$SMOKE_HW_OVERRIDE
if [ -z "$hw" ]; then
  fail "Hardwarekennung" "nicht gemeldet"
elif [ "$hw" = "65535" ]; then
  fail "Hardwarekennung" "65535 — STM-Flash gesperrt (DIR-010). Erst STM zuruecksetzen, dann pruefen"
else
  pass "Hardwarekennung" "$hw${SMOKE_HW_OVERRIDE:+ (Testwert)}"
fi

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

  # Dritter Parameter: Pflichtfelder, durch Komma getrennt. GUELTIGES JSON IST NICHT
  # GENUG -- genau das ist am 03.10.2026 aufgefallen (L147/L148): /api/update_status
  # verlor einen vollen Pufferinhalt von 1072 Byte, 26 von 162 Feldern, und die Antwort
  # blieb syntaktisch gueltig. Dieser Smoketest meldete "OK 7012 Byte", die
  # Browserpruefung 8/0. Ein Endpunkt, der gueltiges JSON mit fehlendem Inhalt liefert,
  # bestand bis dahin jede Pruefung dieses Projekts.
  if [ -n "${3:-}" ]; then
    fehlend=""
    for feld in $(printf '%s' "$3" | tr ',' ' '); do
      printf '%s' "$data" | grep -q "\"$feld\"" || fehlend="$fehlend $feld"
    done
    if [ -n "$fehlend" ]; then
      fail "/api/$1 vollstaendig" "Pflichtfelder fehlen:$fehlend - Antwort abgeschnitten?"
    else
      pass "/api/$1 vollstaendig" "alle Pflichtfelder da"
    fi
  fi
}

check_api device_ready       json  ready,free_heap,max_free_block
check_api settings_xml       xml
check_api display_power      onoff
check_api ambilight_power    onoff
check_api power_status       json
check_api update_status      json  wc_version,esp_version,app_available,wc_available,stm32_default
check_api update_table_files json
check_api fs_info            json
check_api fs_list            json
check_api stm32_log          json
check_api overlay_icons      json

# ------------------------------------- Hat das Geraet je eine Antwort verloren?
# write_lost_bytes/-blocks zaehlen, was http_flush() nicht hinausbekommen hat. Solange
# beide 0 sind, hat jede Antwort dieses Geraets die Verbindung vollstaendig erreicht --
# das war vor der Korrektur zu L147 gar nicht feststellbar. Fehlen die Felder, laeuft
# eine aeltere Firmware; das ist kein Fehlschlag, sondern eine Luecke in der Aussage,
# und die gehoert benannt statt stillschweigend als "bestanden" verbucht.
#
# Seit ESP-Firmware mit C25 trennt der ESP die Ursache (write_lost_gone /
# write_lost_failed). "gone" heisst: Die Gegenstelle hatte die Verbindung schon
# abgebaut -- typisch ein Browser, der einen Abruf abbricht. Das ist L270 und kein
# Geraetefehler; es machte diese Stufe in jedem Lauf rot ("29/1"), bis man es nicht
# mehr las. Fehlschlag ist jetzt nur noch "failed": Verbindung stand, Schreiben scheiterte.
# SMOKE_DR_OVERRIDE setzt die device_ready-Antwort fuer die Gegenprobe ein (DIR-014).
dr=${SMOKE_DR_OVERRIDE:-$(curl -s -m "$TIMEOUT" "$U/api/device_ready" 2>/dev/null)}
lb=$(printf '%s' "$dr" | grep -o '"write_lost_bytes":[0-9]*'  | cut -d: -f2)
lk=$(printf '%s' "$dr" | grep -o '"write_lost_blocks":[0-9]*' | cut -d: -f2)
lg=$(printf '%s' "$dr" | grep -o '"write_lost_gone":[0-9]*'   | cut -d: -f2)
lf=$(printf '%s' "$dr" | grep -o '"write_lost_failed":[0-9]*' | cut -d: -f2)
if [ -z "$lb" ] || [ -z "$lk" ]; then
  printf '  --    %-34s %s\n' "Antwortverluste" "Zaehler fehlen, Firmware aelter als die L147-Korrektur"
elif [ "$lb" = "0" ] && [ "$lk" = "0" ]; then
  pass "Antwortverluste" "keine - kein Block ging verloren"
elif [ -n "$lf" ] && [ -n "$lg" ]; then
  if [ "$lf" = "0" ]; then
    pass "Antwortverluste" "kein Schreibfehler; $lg Mal Verbindung schon abgebaut (Browserabbruch, L270)"
  else
    fail "Antwortverluste" "$lf Mal Schreiben gescheitert bei stehender Verbindung (dazu $lg Abbrueche)"
  fi
else
  fail "Antwortverluste" "$lk Block(e), $lb Byte verloren - Antworten kamen unvollstaendig an"
fi

# ------------------------------------- C65 (E3): Heap-Tiefstwert und Messfelder
# Ab ESP 3.2.36 meldet device_ready den Tiefstwert des Heaps je Pfad (I-1), die laengste
# Anfrage (I-3), die ausgehenden Verbindungen und abgebrochene Assets (I-2). Der Smoketest
# verlangt die Felder und LIEST nur -- er setzt den Tiefstwert nie zurueck (Ent-21: das darf
# nur heap_low_reset=1, sonst loeschte jeder Smoketest die Messung eines laufenden Tests).
# Vor 3.2.36 fehlen sie zu Recht; mit SMOKE_DR_OVERRIDE gelten sie immer als Pflicht (Gegenprobe).
E3_FELDER="heap_low_block heap_low_free heap_low_path heap_low_ms httpclient_connects fs_write_short req_max_ms req_max_path upd_cache_max_ms"
if [ -n "${SMOKE_DR_OVERRIDE:-}" ] || [ "$(printf '%s\n%s\n' "3.2.36" "${esp:-0}" | sort -V | head -1)" = "3.2.36" ]; then
  fehlend=""
  for f in $E3_FELDER; do printf '%s' "$dr" | grep -q "\"$f\":" || fehlend="$fehlend $f"; done
  if [ -n "$fehlend" ]; then
    fail "Heap-Messfelder (E3)" "fehlen:$fehlend"
  else
    hb=$(printf '%s' "$dr" | grep -o '"heap_low_block":[0-9]*' | cut -d: -f2)
    hp=$(printf '%s' "$dr" | grep -o '"heap_low_path":"[^"]*"' | cut -d'"' -f4)
    fw=$(printf '%s' "$dr" | grep -o '"fs_write_short":[0-9]*' | cut -d: -f2)
    rm=$(printf '%s' "$dr" | grep -o '"req_max_ms":[0-9]*' | cut -d: -f2)
    if [ "${fw:-0}" != "0" ]; then
      fail "Heap-Messfelder (E3)" "$fw Asset(s) abgebrochen ausgeliefert (fs_write_short) - Tiefstwert $hb${hp:+ ($hp)}"
    else
      um=$(printf '%s' "$dr" | grep -o '"upd_cache_max_ms":[0-9]*' | cut -d: -f2)
      pass "Heap-Messfelder (E3)" "Tiefstwert $hb Byte${hp:+ ($hp)}, laengste Anfrage $rm ms, laengster Cache-Schritt $um ms, nichts zurueckgesetzt"
    fi
  fi
else
  printf '  --    %-34s %s\n' "Heap-Messfelder (E3)" "ESP ${esp:-?} vor 3.2.36, Felder gibt es noch nicht"
fi

# ------------------------------------- C65 (E4): Momentaufnahme beim Tief (I-4), Backlog, verwaiste Lesende
# Ab ESP 3.2.37: was beim Block-Tiefstwert los war (wartende Verbindungen, Backlog, PCBs je Zustand,
# TIME_WAIT, unquittierte Byte, Pfad), dazu der aktuelle Backlog und die uebersprungenen verwaisten
# Lesenden. Wieder NUR lesen, nichts zuruecksetzen. heap_low_pcb_states muss genau 11 Zahlen tragen
# (Index = lwIP enum tcp_state) -- eine andere Zahl hiesse, dass Firmware und Lesart auseinanderlaufen.
E4_FELDER="heap_low_accepts_pending heap_low_backlog heap_low_pcb_states heap_low_tw heap_low_unacked heap_low_req_path http_backlog api_orphan_skipped"
if [ -n "${SMOKE_DR_OVERRIDE:-}" ] || [ "$(printf '%s\n%s\n' "3.2.37" "${esp:-0}" | sort -V | head -1)" = "3.2.37" ]; then
  fehlend=""
  for f in $E4_FELDER; do printf '%s' "$dr" | grep -q "\"$f\":" || fehlend="$fehlend $f"; done
  zust=$(printf '%s' "$dr" | grep -o '"heap_low_pcb_states":\[[^]]*\]' | sed 's/.*\[//; s/\]//')
  anz=$(printf '%s' "$zust" | tr ',' '\n' | grep -c '^ *-\{0,1\}[0-9][0-9]* *$')
  if [ -n "$fehlend" ]; then
    fail "Momentaufnahme beim Tief (E4)" "fehlen:$fehlend"
  elif [ "$anz" != "11" ]; then
    fail "Momentaufnahme beim Tief (E4)" "heap_low_pcb_states traegt $anz statt 11 Zahlen"
  else
    ap=$(printf '%s' "$dr" | grep -o '"heap_low_accepts_pending":[0-9]*' | cut -d: -f2)
    tw=$(printf '%s' "$dr" | grep -o '"heap_low_tw":[0-9]*' | cut -d: -f2)
    bl=$(printf '%s' "$dr" | grep -o '"http_backlog":[0-9]*' | cut -d: -f2)
    os=$(printf '%s' "$dr" | grep -o '"api_orphan_skipped":[0-9]*' | cut -d: -f2)
    pass "Momentaufnahme beim Tief (E4)" "beim Tief $ap wartend, $tw TIME_WAIT; Backlog jetzt $bl; $os verwaiste Lesende uebersprungen"
  fi
else
  printf '  --    %-34s %s\n' "Momentaufnahme beim Tief (E4)" "ESP ${esp:-?} vor 3.2.37, Felder gibt es noch nicht"
fi

# ------------------------------------- C72 (Paket 2026-10-15, R1, I-5): Verbindungszustaende JETZT
# Ab ESP 3.2.38: device_ready meldet die lwIP-Listen im Moment der Anfrage -- Zustaende, TIME_WAIT,
# wartende und abgeholte tote Verbindungen, eine Liste von hoechstens 8 Eintraegen. tcp_now_states muss
# genau 11 Zahlen tragen (Index = enum tcp_state; LISTEN und TIME_WAIT stehen dort immer auf 0, TIME_WAIT
# zaehlt tcp_now_tw). Die eigene Abfrage des Smoketests steht selbst in der Liste -- das ist kein Befund.
R1_FELDER="tcp_now_states tcp_now_tw tcp_now_pending tcp_now_unclaimed tcp_now_self_port tcp_now_list tcp_now_more http_accept_dead"
if [ -n "${SMOKE_DR_OVERRIDE:-}" ] || [ "$(printf '%s\n%s\n' "3.2.38" "${esp:-0}" | sort -V | head -1)" = "3.2.38" ]; then
  fehlend=""
  for f in $R1_FELDER; do printf '%s' "$dr" | grep -q "\"$f\":" || fehlend="$fehlend $f"; done
  zust=$(printf '%s' "$dr" | grep -o '"tcp_now_states":\[[^]]*\]' | sed 's/.*\[//; s/\]//')
  anz=$(printf '%s' "$zust" | tr ',' '\n' | grep -c '^ *[0-9][0-9]* *$')
  if [ -n "$fehlend" ]; then
    fail "Verbindungszustaende jetzt (R1)" "fehlen:$fehlend"
  elif [ "$anz" != "11" ]; then
    fail "Verbindungszustaende jetzt (R1)" "tcp_now_states traegt $anz statt 11 Zahlen"
  else
    tw=$(printf '%s' "$dr" | grep -o '"tcp_now_tw":[0-9]*' | cut -d: -f2)
    pe=$(printf '%s' "$dr" | grep -o '"tcp_now_pending":[0-9]*' | cut -d: -f2)
    ad=$(printf '%s' "$dr" | grep -o '"http_accept_dead":[0-9]*' | cut -d: -f2)
    pass "Verbindungszustaende jetzt (R1)" "aktiv [$zust], $tw TIME_WAIT, $pe wartend, $ad tote Abholungen"
  fi
else
  printf '  --    %-34s %s\n' "Verbindungszustaende jetzt (R1)" "ESP ${esp:-?} vor 3.2.38, Felder gibt es noch nicht"
fi

# ------------------------------------- C70 (Paket 2026-10-15, R2): Asset-Cache mit ETag und 304
# Ab ESP 3.2.40 traegt jede 200-Antwort auf ein App-Asset ein ETag; ein If-None-Match mit genau diesem
# Wert ergibt 304 ohne Rumpf, und zwar als HTTP/1.1 -- bei HTTP/1.0 revalidiert Chrome nicht (AKC.7,
# L468). Ein falsches ETag muss die volle Datei liefern. SMOKE_C70_OVERRIDE=1 prueft auch aeltere
# Staende (Gegenprobe nach DIR-014: auf 3.2.39 muss der Block fehlschlagen).
if [ -n "${SMOKE_C70_OVERRIDE:-}" ] || [ "$(printf '%s\n%s\n' "3.2.40" "${esp:-0}" | sort -V | head -1)" = "3.2.40" ]; then
  for path in /app/ /app/app.js /app/styles.css; do
    et=$(curl -s -m "$TIMEOUT" -D - -o /dev/null -H 'Accept-Encoding: gzip' "$U$path" 2>/dev/null \
         | tr -d '\r' | grep -i '^etag:' | head -1 | sed 's/^[Ee][Tt][Aa][Gg]: *//')
    if [ -z "$et" ]; then fail "ETag/304 $path (C70)" "kein ETag in der 200-Antwort"; continue; fi
    read -r c s v < <(curl -s -m "$TIMEOUT" -o /dev/null -w '%{http_code} %{size_download} %{http_version}' \
         -H 'Accept-Encoding: gzip' -H "If-None-Match: $et" "$U$path" 2>/dev/null)
    read -r c2 s2 < <(curl -s -m "$TIMEOUT" -o /dev/null -w '%{http_code} %{size_download}' \
         -H 'Accept-Encoding: gzip' -H 'If-None-Match: "c70-falsch"' "$U$path" 2>/dev/null)
    if [ "$c" != "304" ] || [ "${s:-1}" != "0" ] || [ "$v" != "1.1" ]; then
      fail "ETag/304 $path (C70)" "mit passendem ETag: HTTP/$v $c, $s Byte (erwartet HTTP/1.1 304, 0 Byte)"
    elif [ "$c2" != "200" ] || [ "${s2:-0}" -le 0 ]; then
      fail "ETag/304 $path (C70)" "mit falschem ETag: $c, $s2 Byte (erwartet 200 mit Rumpf)"
    else
      pass "ETag/304 $path (C70)" "ETag $et, 304 als HTTP/1.1 ohne Rumpf, falsches ETag 200"
    fi
  done
else
  printf '  --    %-34s %s\n' "Asset-Cache ETag/304 (C70)" "ESP ${esp:-?} vor 3.2.40, gibt es noch nicht"
fi

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

# ------------------------------------- Diagnosezeile vorhanden und lueckenlos
#
# Die Zeile ist das Messinstrument des Beobachtbarkeits-Pakets. Sie zu pruefen heisst
# hier zweierlei, und der zweite Teil ist der wichtigere:
#
#   1. Sie ist ueberhaupt da. Fehlt sie, laeuft eine Firmware ohne das Instrument --
#      und niemand merkt es, weil ein leeres Logbuch genauso aussieht wie ein ruhiges.
#   2. Ihre FOLGENUMMERN sind lueckenlos. Genau dafuer traegt sie eine: Ein
#      ueberschriebener Ring liest sich sonst wie "es war ruhig". Eine Luecke bedeutet,
#      dass der Ring zwischen zwei Zeilen umgelaufen ist -- also dass etwas den Ring
#      geflutet hat, und das ist der Zustand, den das ganze Paket beseitigen sollte.
#
# Rein lesend (DIR-008).
# Gefordert wird die Zeile erst ab der Firmware, die sie einfuehrt. Sonst schlaegt der
# Smoketest auf jedem aelteren Stand fehl -- etwa nach einem bewussten Rueckschritt --
# und waere dort unbrauchbar statt streng. Fehlt sie auf einer NEUEN Firmware, ist das
# dagegen ein echter Fehlschlag: Dann laeuft ein Fabrikat ohne sein Messinstrument.
DIAG_SINCE_STM=3.2.14

diag=$(curl -s -m "$TIMEOUT" "$U/api/stm32_log" 2>/dev/null)
if [ -z "$diag" ]; then
  fail "Diagnosezeile" "stm32_log nicht lesbar"
else
  DIAG="$diag" STM="$stm" SINCE="$DIAG_SINCE_STM" python3 - <<'PY'
import json, os, re, sys

def lines(raw):
    try:
        d = json.loads(raw)
    except ValueError:
        return []
    if isinstance(d, dict):
        for key in ("lines", "log", "stm32_log"):
            if key in d:
                d = d[key]; break
    return [str(x) for x in d] if isinstance(d, list) else []

seqs = [int(m.group(1))
        for z in lines(os.environ["DIAG"])
        if (m := re.search(r"\bdiag\s+(\d+)\b", z))]

def ver(s):
    try:
        return tuple(int(x) for x in s.strip().split("."))
    except ValueError:
        return ()

stm, since = ver(os.environ.get("STM", "")), ver(os.environ["SINCE"])

if not lines(os.environ["DIAG"]):
    # E11/L104: Ein LEERER Ring ist kein fehlendes Messinstrument, sondern ein frisch
    # gestarteter ESP -- der Ring lebt im ESP-RAM und fuellt sich erst wieder.
    print("  INFO  Diagnosezeile                      Logring leer — ESP gerade neu gestartet? "
          "In einer Minute nochmals laufen lassen")
    sys.exit(0)

if not seqs:
    if stm and since and stm < since:
        print(f"  INFO  Diagnosezeile                      keine — STM {os.environ['STM']} ist "
              f"aelter als {os.environ['SINCE']}, das ist erwartet")
        sys.exit(0)
    print(f"  FAIL  Diagnosezeile                      keine im Ring, aber STM "
          f"{os.environ.get('STM','?')} sollte sie haben (Fabrikat ohne Messinstrument)")
    sys.exit(3)

luecken = [(a, b) for a, b in zip(seqs, seqs[1:]) if b != a + 1]
if luecken:
    a, b = luecken[0]
    print(f"  FAIL  Diagnosezeile                      {len(seqs)} Zeilen, Luecke {a} -> {b}"
          f" (Ring umgelaufen, etwas flutet ihn)")
    sys.exit(3)

print(f"  OK    Diagnosezeile                      {len(seqs)} Zeilen, Folgenummern lueckenlos")
PY
  case "$?" in
    0) OK=$((OK+1));;
    *) FAIL=$((FAIL+1));;
  esac
fi

# ------------------------------------------------------------------------ Fazit
printf '\n=== %d bestanden, %d fehlgeschlagen ===\n' "$OK" "$FAIL"
[ "$FAIL" -gt 0 ] && exit 1
echo "Geraet verhaelt sich wie erwartet."

# DIR-012. Diese Zeile steht hier, weil sie genau dort gelesen wird, wo der Irrtum
# entsteht: Wer "27/0 bestanden" sieht, haelt das Release fuer geprueft. Dieser Test
# prueft, ob das Geraet LEBT -- nicht, ob es noch tut, was es soll. Ein Endpunkt, der
# {"ok":true} meldet und nichts tut, besteht ihn (genau das war remote_stm32_flash
# ohne filename). Am 03.10.2026 gingen drei Releases mit diesem Test allein hinaus.
echo
echo "  Das war der Smoketest, nicht der Test (DIR-012). Er sagt: das Geraet lebt."
echo "  Ob es noch TUT, was es soll, sagt der Durchlauf nach TESTPLAN-PWA.md --"
echo "  faellig vor jedem Release, das mehr als eine Komponente beruehrt."
exit 0
