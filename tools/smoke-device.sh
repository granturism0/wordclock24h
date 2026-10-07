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
# (DIR-008): Dies ist die Uhr des Nutzers im Dauerbetrieb, kein Testgeraet.

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

esp=$(curl -s -m "$TIMEOUT" "$U/update" 2>/dev/null | grep -oE '3\.[0-9]+\.[0-9]+' | sed -n 2p)
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
