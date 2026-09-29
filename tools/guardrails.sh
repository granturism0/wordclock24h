#!/usr/bin/env bash
# Guardrails — Minimalbasis nach Auftrag 7.
#
#   ./tools/guardrails.sh          schnelle Stufe, laeuft nach JEDEM Task (Sekunden)
#   ./tools/guardrails.sh --full   zusaetzlich die Compile-Smoke-Tests (Minuten)
#
# Exit 0 = keine Kritisch-Findings, Task darf abgeschlossen werden.
# Exit 1 = mindestens ein Kritisch-Finding, Task blockiert.
#
# Die --full-Stufe baut und darf nach R1 in CLAUDE.md nur vom release-engineer
# und nur seriell ausgefuehrt werden.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

# Acht C-Dateien im Repo sind ISO-8859-1. grep stuft sie als BINAER ein und
# unterdrueckt die Ausgabe vollstaendig — nicht "0 Treffer", sondern gar nichts.
# Das sieht aus wie "nicht vorhanden", ist aber "nicht gelesen".
# Entscheidend ist -a (als Text behandeln), NICHT das Locale. LC_ALL=C allein
# behebt es nachweislich nicht.
export LC_ALL=C
GREP="grep -a"

APP="ESP8266/ESP-uclock/data/app"
FULL=0
[ "${1:-}" = "--full" ] && FULL=1

CRIT=0
WARN=0
step() { printf '\n[%s] %s\n' "$1" "$2"; }
crit() { printf '  KRITISCH  %s\n' "$1"; CRIT=$((CRIT+1)); }
warn() { printf '  HOCH      %s\n' "$1"; WARN=$((WARN+1)); }
ok()   { printf '  OK  %s\n' "$1"; }

printf '=== Guardrails wordclock24h — %s ===\n' "$(date '+%Y-%m-%d %H:%M')"

# ---------------------------------------------------------------- S1 Syntax
step S1 "Syntaxpruefung der PWA"
for f in "$APP/app.js" "$APP/sw.js"; do
  if node --check "$f" 2>/dev/null; then ok "$f"; else crit "$f: Syntaxfehler"; node --check "$f" 2>&1 | head -3; fi
done

# ------------------------------------------------- S2 Undefinierte Funktionen
step S2 "Undefinierte Funktionsaufrufe"
for f in "$APP/app.js" "$APP/sw.js"; do
  node tools/checks/undefined-functions.mjs "$f" || CRIT=$((CRIT+1))
done

# ------------------------------------------------------------- S3 i18n-Keys
step S3 "i18n-Schluessel"
node tools/checks/i18n-keys.mjs "$APP/app.js" "$APP/index.html" || CRIT=$((CRIT+1))

# ------------------------------------------- S4 Versionszeilen und CACHE_NAME
step S4 "Versionszeilen greppbar und CACHE_NAME aktuell"
check_version() {
  local n; n=$(grep -c "$2" "$1" 2>/dev/null || echo 0)
  if [ "$n" -eq 1 ]; then ok "$3: $(grep -m1 "$2" "$1" | tr -s ' ')"
  else crit "$3: Versionszeile in $1 nicht genau einmal gefunden ($n) — Makefile liest sie per grep"; fi
}
check_version src/main.h '^#define VERSION' "STM"
check_version ESP8266/ESP-uclock/version.h '^#define ESP_VERSION' "ESP"
check_version "$APP/app.js" '^const APP_VERSION' "App"
check_version "$APP/sw.js" '^const CACHE_NAME' "SW-Cache"
if ! git diff --quiet HEAD -- "$APP/app.js" 2>/dev/null; then
  if git diff --quiet HEAD -- "$APP/sw.js" 2>/dev/null; then
    warn "app.js geaendert, CACHE_NAME in sw.js unveraendert — neue index.html mit alter app.js moeglich"
  else ok "app.js geaendert und sw.js ebenfalls"; fi
fi

# ----------------------------------------------------------- S5 .gz-Artefakte
step S5 "gz-Artefakte vorhanden, nicht leer, nicht veraltet"
GZ_SOURCES="$APP/app.js $APP/styles.css $APP/index.html $APP/sw.js $APP/manifest.webmanifest $APP/layout-previews.json $APP/icons/icon-192.svg $APP/icons/icon-512.svg"
for src in $GZ_SOURCES; do
  gz="$src.gz"
  base=$(basename "$src")
  if [ ! -f "$src" ]; then crit "Quelle fehlt: $src"; continue; fi
  if [ ! -f "$gz" ]; then crit "$base.gz fehlt — App-Assets werden nur als .gz ausgeliefert"; continue; fi
  if [ ! -s "$gz" ]; then crit "$base.gz ist 0 Byte — fuehrt zu White-Screen und bleibt im Service-Worker-Cache"; continue; fi
  if [ "$src" -nt "$gz" ]; then warn "$base.gz ist aelter als die Quelle — 'make app-gz' noetig"; else ok "$base.gz"; fi
done

# ------------------------------------------------------------ S6 CSS-Klassen
step S6 "CSS-Klassen ohne Verwendung"
node tools/checks/unused-css.mjs "$APP/styles.css" "$APP/index.html" "$APP/app.js"

# ------------------------------------------- S7 Lint gegen quick-reference.md
step S7 "Muster aus knowledge/quick-reference.md"
n=$($GREP -c '^\s*log_printf' src/sk6812/sk6812.c 2>/dev/null || echo 0)
[ "$n" -gt 0 ] && warn "sk6812.c: $n unbedingte log_printf im Refresh-Pfad — blockieren pro Refresh, debug_log_printf verwenden" || ok "sk6812.c: kein unbedingtes log_printf"
n=$($GREP -c 'watchdog_reload ()\s*;' src/main.c 2>/dev/null || echo 0)
[ "$n" -le 1 ] && warn "watchdog_reload() hat nur $n Aufrufstelle — jeder Pfad ueber 20 s ist ein IWDG-Reset" || ok "watchdog_reload(): $n Aufrufstellen"
n=$($GREP -n 'innerHTML' "$APP/app.js" | $GREP -vc 'escapeHtml' || echo 0)
[ "$n" -gt 0 ] && warn "app.js: $n innerHTML-Zuweisungen ohne escapeHtml in derselben Zeile — einzeln pruefen" || ok "app.js: jedes innerHTML mit escapeHtml"
n=$($GREP -cE 'catch\s*\(\s*_\s*\)\s*\{\s*\}|catch\s*\{\s*\}' "$APP/app.js" 2>/dev/null || echo 0)
[ "$n" -gt 0 ] && warn "app.js: $n leere catch-Bloecke — verschlucken Fehler stillschweigend" || ok "app.js: keine leeren catch-Bloecke"

# ------------------------------------------------ S7b Kodierung der Quellen
step S7b "Kodierung der C-Quellen"
enc_bad=0
for f in $(find src -name "*.c" -o -name "*.h" | sort); do
  if ! iconv -f UTF-8 -t UTF-8 "$f" >/dev/null 2>&1; then
    enc_bad=$((enc_bad+1))
    [ "$enc_bad" -le 3 ] && printf '  INFO      nicht UTF-8: %s\n' "$f"
  fi
done
if [ "$enc_bad" -gt 0 ]; then
  printf '  INFO      %s Datei(en) sind nicht UTF-8. grep braucht dort -a, sonst stuft es sie\n' "$enc_bad"
  echo   '            als binaer ein und gibt GAR NICHTS aus. LC_ALL=C allein reicht nicht.'
else
  ok "alle C-Quellen sind UTF-8"
fi

# ---------------------------------------------------------- S8 Smoke-Tests
step S8 "Smoke-Tests pro Modul"
node tools/checks/smoke-pwa.mjs "$APP/app.js" || CRIT=$((CRIT+1))
if [ "$FULL" -eq 1 ]; then
  echo "  --full: Compile-Smoke-Tests (dauert Minuten, nur serieller Lauf erlaubt)"
  for t in f103 f411 esp; do
    if make "$t" >/tmp/guardrail-$t.log 2>&1; then ok "make $t uebersetzt"
    else crit "make $t schlaegt fehl — siehe /tmp/guardrail-$t.log"; tail -5 /tmp/guardrail-$t.log; fi
  done
else
  echo "  uebersprungen: STM- und ESP-Compile-Smoke-Test (nur mit --full)"
fi

# -------------------------------------------------------------------- Fazit
printf '\n=== Ergebnis: %d Pruefung(en) mit Kritisch-Findings, %d Hoch-Findings ===\n' "$CRIT" "$WARN"
if [ "$CRIT" -gt 0 ]; then
  echo "BLOCKIERT — Task gilt nicht als abgeschlossen, keine Schreibuebergabe."
  exit 1
fi
echo "BESTANDEN — Kritisch-Findings keine. Hoch-Findings pruefen, Abweichung begruenden."
exit 0
