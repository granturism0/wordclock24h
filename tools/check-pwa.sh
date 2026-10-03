#!/usr/bin/env bash
# Laedt die PWA VOM GERAET in einen echten Browser und prueft, was dabei herauskommt.
#
#   ./tools/check-pwa.sh            gegen DEVICE_HOST aus tools/device.conf
#   ./tools/check-pwa.sh --keep     DOM und Konsolenmitschnitt behalten
#
# WARUM ES DIESES SKRIPT GIBT
#
# Bis zum 03.10.2026 pruefte kein Durchlauf die PWA im Browser. Der Testagent
# verglich die API-Ebene gegen die Rohwerte -- also das, was UNTER der Oberflaeche
# liegt. Seine eigene Formulierung: "kein Browser in diesem Lauf, ich habe die
# API-Ebene gegen die Rohwerte geprueft, nicht die Anzeige."
#
# Das ist eine Luecke mit Ansage, denn an app.js wird staendig gearbeitet. Ein
# Tippfehler dort wirft beim Laden einen Fehler, die Seite bleibt halb leer -- und
# weder die API noch ein Screenshot zeigen das. Der Smoketest erst recht nicht: Er
# prueft, ob app.js.gz AUSGELIEFERT wird, nicht ob sie LAEUFT.
#
# tools/preview/ rendert die PWA zwar in 20 Viewports, aber gegen Attrappen-Daten
# aus server.py. Gut fuer das Layout, ungeeignet fuer die Frage, ob die Oberflaeche
# mit den ECHTEN Werten der Uhr zurechtkommt.
#
# GEPRUEFT WIRD DREIERLEI
#
#   1. Wirft die Seite beim Laden JavaScript-Fehler? Das ist der wichtigste Punkt.
#   2. Kommt der Inhalt ueberhaupt an -- oder bleibt die Seite beim Ladebildschirm?
#   3. Stehen die ECHTEN Geraetewerte in der gerenderten Seite? Dafuer wird die
#      gemeldete STM-Version aus /api/settings_xml geholt und im DOM gesucht.
#      Steht sie nicht da, hat die Oberflaeche die Daten nicht verarbeitet.
#
# AUSSCHLIESSLICH LESEND (DIR-008). Die Seite wird geladen, nichts angeklickt.
#
# Chrome-Eigenheit, dieselbe wie in tools/preview/shot.sh: Der Prozess beendet sich
# nach --dump-dom nicht zuverlaessig. Deshalb laeuft er im Hintergrund und wird nach
# KILL_AFTER Sekunden beendet; die Ausgabe ist dann vollstaendig.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

[ -f tools/device.conf ] && . tools/device.conf
HOST=${DEVICE_HOST:-}
if [ -z "$HOST" ]; then
  echo "DEVICE_HOST fehlt. tools/device.conf anlegen (Vorlage: tools/device.conf.example)." >&2
  exit 2
fi

CHROME=${CHROME:-/Applications/Google Chrome.app/Contents/MacOS/Google Chrome}
if [ ! -x "$CHROME" ]; then
  echo "Chrome nicht gefunden: $CHROME" >&2
  echo "Mit CHROME=... auf eine andere Installation zeigen." >&2
  exit 2
fi

KEEP=0
[ "${1:-}" = "--keep" ] && KEEP=1

U="http://$HOST"
WORK=$(mktemp -d)
DOM="$WORK/dom.html"
LOG="$WORK/konsole.txt"
PROFILE="$WORK/profil"
KILL_AFTER=${KILL_AFTER:-25}

OK=0
FAIL=0
pass() { printf '  OK    %-38s %s\n' "$1" "${2:-}"; OK=$((OK+1)); }
fail() { printf '  FEHLT %-38s %s\n' "$1" "${2:-}"; FAIL=$((FAIL+1)); }

printf '=== PWA im Browser gegen %s — %s ===\n\n' "$HOST" "$(date '+%Y-%m-%d %H:%M')"

# ------------------------------------------- Sollwert vom Geraet, vor dem Laden
sx=$(curl -s -m 15 "$U/api/settings_xml" 2>/dev/null)
stm=$(printf '%s' "$sx" | grep -o '<strvar idx="1" value="[^"]*"' | sed 's/.*value="//;s/"//')
if [ -z "$stm" ]; then
  echo "  ABBRUCH: Geraet meldet keine STM-Version — erst smoke-device.sh pruefen." >&2
  rm -rf "$WORK"; exit 1
fi
printf '  Geraet meldet STM %s — dieser Wert muss gleich in der Seite stehen.\n\n' "$stm"

# --------------------------------------------------------------- Seite laden
#
# Mit Debug-Port statt --dump-dom: Nur so laesst sich die Seite auch BEDIENEN.
# Gesteuert wird in tools/check-pwa.mjs ueber das DevTools-Protokoll; Node bringt
# seit v22 ein eingebautes WebSocket mit, es braucht kein Puppeteer.
CDP_PORT=${CDP_PORT:-9222}

printf '  starte Chrome und lade /app/'
"$CHROME" --headless=new --disable-gpu --no-sandbox \
  --user-data-dir="$PROFILE" --remote-debugging-port="$CDP_PORT" \
  "$U/app/" >"$LOG" 2>&1 &
CHROME_PID=$!

i=0
while [ "$i" -lt 20 ]; do
  curl -s -m 2 "http://127.0.0.1:$CDP_PORT/json/version" >/dev/null 2>&1 && break
  sleep 1; i=$((i+1)); printf '.'
done
printf '\n\n'

if ! curl -s -m 2 "http://127.0.0.1:$CDP_PORT/json/version" >/dev/null 2>&1; then
  echo "  ABBRUCH: Chrome hat den Debug-Port nicht geoeffnet." >&2
  kill "$CHROME_PID" 2>/dev/null
  rm -rf "$WORK"; exit 1
fi

STM_SOLL="$stm" CDP_PORT="$CDP_PORT" node tools/check-pwa.mjs
RC=$?

kill "$CHROME_PID" 2>/dev/null
wait "$CHROME_PID" 2>/dev/null

if [ "$KEEP" -eq 1 ]; then
  echo
  echo "  Chrome-Mitschnitt: $LOG"
else
  rm -rf "$WORK" 2>/dev/null
fi

[ "$RC" -eq 0 ] && echo "Die PWA laeuft auf dem Geraet, laesst sich bedienen und zeigt echte Werte."
exit $RC
