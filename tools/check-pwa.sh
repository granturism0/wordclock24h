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
printf '  lade /app/ in Chrome'
"$CHROME" --headless=new --disable-gpu --no-sandbox \
  --user-data-dir="$PROFILE" --virtual-time-budget=15000 \
  --enable-logging=stderr --log-level=0 \
  --dump-dom "$U/app/" >"$DOM" 2>"$LOG" &
CHROME_PID=$!

i=0
while [ "$i" -lt "$KILL_AFTER" ]; do
  kill -0 "$CHROME_PID" 2>/dev/null || break
  [ -s "$DOM" ] && sleep 2 && break          # DOM ist da, kurz nachlaufen lassen
  sleep 1; i=$((i+1)); printf '.'
done
kill "$CHROME_PID" 2>/dev/null
wait "$CHROME_PID" 2>/dev/null
printf '\n\n'

# ------------------------------------------------------------------ Auswertung
if [ ! -s "$DOM" ]; then
  fail "Seite geladen" "kein DOM — Chrome hat nichts geliefert"
  printf '\n=== %d bestanden, %d fehlgeschlagen ===\n' "$OK" "$FAIL"
  [ "$KEEP" -eq 1 ] && echo "  Mitschnitt: $LOG" || rm -rf "$WORK"
  exit 1
fi
pass "Seite geladen" "$(wc -c < "$DOM" | tr -d ' ') Byte DOM"

# 1. JavaScript-Fehler. Chrome meldet sie auf stderr mit der Quelle dahinter.
#    "Uncaught" faengt die geworfenen, "SEVERE" die vom Logger eingestuften.
js=$(grep -aiE 'uncaught|severe:.*\.js|unhandled.*rejection' "$LOG" 2>/dev/null \
     | grep -avE 'favicon|DevTools|GPU|Fontconfig|dbus|gl_display' | head -5)
if [ -n "$js" ]; then
  fail "keine JavaScript-Fehler" "$(printf '%s' "$js" | wc -l | tr -d ' ') Meldung(en)"
  printf '%s\n' "$js" | sed 's/^/          /'
else
  pass "keine JavaScript-Fehler"
fi

# 2. Ist die Oberflaeche ueber den Ladebildschirm hinausgekommen? Bleibt app.js
#    stehen, liefert der ESP zwar Markup, aber der Inhalt wird nie gefuellt.
if grep -qa 'id="app-loading"[^>]*hidden\|class="[^"]*is-ready' "$DOM" 2>/dev/null; then
  pass "Oberflaeche aufgebaut"
elif grep -qac 'data-module=' "$DOM" 2>/dev/null; then
  n=$(grep -oa 'data-module="[a-z]*"' "$DOM" | sort -u | wc -l | tr -d ' ')
  pass "Oberflaeche aufgebaut" "$n Module im DOM"
else
  fail "Oberflaeche aufgebaut" "weder Module noch Bereitschaftsmerkmal gefunden"
fi

# 3. Der eigentliche Punkt: Stehen die ECHTEN Werte der Uhr in der Seite?
#    Nicht "wurde die API gerufen", sondern "ist das Ergebnis angekommen".
if grep -qa "$stm" "$DOM"; then
  pass "Geraetewerte angezeigt" "STM $stm steht in der Seite"
else
  fail "Geraetewerte angezeigt" "STM $stm steht NICHT in der Seite — Daten nicht verarbeitet"
fi

# 4. Leere Platzhalter. Die PWA setzt vor dem ersten Abruf Striche; bleiben viele
#    davon stehen, sind Abrufe fehlgeschlagen, ohne dass ein Fehler geworfen wurde.
striche=$(grep -oa '>—<\|>--<\|>…<' "$DOM" 2>/dev/null | wc -l | tr -d ' ')
if [ "${striche:-0}" -gt 25 ]; then
  fail "Platzhalter gefuellt" "$striche leere Felder — viele Abrufe ohne Ergebnis?"
else
  pass "Platzhalter gefuellt" "$striche offen"
fi

printf '\n=== %d bestanden, %d fehlgeschlagen ===\n' "$OK" "$FAIL"

if [ "$KEEP" -eq 1 ]; then
  echo
  echo "  DOM:      $DOM"
  echo "  Konsole:  $LOG"
else
  rm -rf "$WORK"
fi

[ "$FAIL" -gt 0 ] && exit 1
echo "Die PWA laeuft auf dem Geraet und zeigt echte Werte."
exit 0
