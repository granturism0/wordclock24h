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

# ------------------------------------------- S4 Versionszeilen und Versionspflicht
step S4 "Versionszeilen greppbar und Versionspflicht DIR-004"
check_version() {
  local n; n=$(grep -c "$2" "$1" 2>/dev/null); n=${n:-0}
  if [ "$n" -eq 1 ]; then ok "$3: $(grep -m1 "$2" "$1" | tr -s ' ')"
  else crit "$3: Versionszeile in $1 nicht genau einmal gefunden ($n) — Makefile liest sie per grep"; fi
}
check_version src/main.h '^#define VERSION' "STM"
check_version ESP8266/ESP-uclock/version.h '^#define ESP_VERSION' "ESP"
check_version "$APP/app.js" '^const APP_VERSION' "App"
check_version "$APP/sw.js" '^const CACHE_NAME' "SW-Cache"

# DIR-004: jede Komponente wird genau dann versioniert, wenn sich IHR Code
# geaendert hat. Kein Gleichschritt — aendert sich nur der STM, steigt nur
# dessen Version. Umgekehrt ist ein Bump ohne Codeaenderung ebenfalls falsch:
# er bietet dem Geraet ein OTA-Update auf identische Firmware an.
#
# Bezugsgroesse ist das letzte RELEASE, nicht der letzte Commit — ein Bump kann
# mehrere Commits zurueckliegen und waere gegen HEAD unsichtbar. tools/deploy.sh
# setzt nach jedem erfolgreichen Rollout ein Tag release/<stm>-<esp>-<app>.
LAST_TAG=$(git describe --tags --abbrev=0 --match 'release/*' 2>/dev/null)
vat() { git show "$1:$2" 2>/dev/null | $GREP -m1 "$3" | sed 's/.*"\(.*\)".*/\1/'; }
now() { $GREP -m1 "$2" "$1" | sed 's/.*"\(.*\)".*/\1/'; }

# Quellen je Komponente. Entscheidend ist die Trennung von ESP und PWA: beide
# liegen unter ESP8266/, werden aber getrennt versioniert und getrennt
# ausgeliefert. Die .gz sind Ableitungen und zaehlen nicht als Quelle.
STM_SRC="src CMakeLists.txt cmake"
# Markdown im Komponentenverzeichnis ist Dokumentation, kein Code -- sonst verlangt
# jede Korrektur an README-PWA.md einen ESP-Bump und damit ein OTA-Update ohne Gegenwert.
ESP_SRC="ESP8266/ESP-uclock :(exclude)ESP8266/ESP-uclock/data :(exclude)ESP8266/ESP-uclock/tools :(exclude)ESP8266/ESP-uclock/build :(exclude)ESP8266/ESP-uclock/*.md"
PWA_SRC="ESP8266/ESP-uclock/data/app :(exclude)ESP8266/ESP-uclock/data/app/**/*.gz"

# 0 = Code der Komponente hat sich geaendert.  $1 Quellen  $2 Versionsdatei  $3 Muster
#
# Die Versionszeile selbst liegt IN den Quellen (main.h, version.h, app.js).
# Wuerde sie mitzaehlen, waere jeder Bump automatisch eine "Codeaenderung" und
# die Gegenprobe — angehoben, obwohl sich nichts geaendert hat — koennte nie
# anschlagen. Deshalb wird die Versionsdatei getrennt betrachtet und in ihrem
# Diff die Versionszeile herausgefiltert.
changed() {
  local rest
  git diff --quiet "$LAST_TAG" HEAD -- $1 ":(exclude)$2" 2>/dev/null || return 0
  git diff --quiet HEAD -- $1 ":(exclude)$2" 2>/dev/null || return 0
  rest=$( { git diff -U0 "$LAST_TAG" HEAD -- "$2"; git diff -U0 HEAD -- "$2"; } 2>/dev/null \
          | $GREP -E '^[-+]' | $GREP -Ev '^(\+\+\+|---)' | $GREP -v "$3" )
  [ -n "$rest" ] && return 0
  return 1
}

if [ -z "$LAST_TAG" ]; then
  echo "  INFO      kein release/-Tag vorhanden — Versionspflicht nicht pruefbar."
  echo "            Das erste tools/deploy.sh setzt eines."
else
  # $1 Name  $2 Quellen  $3 Versionsdatei  $4 Muster
  check_comp() {
    local old new
    old=$(vat "$LAST_TAG" "$3" "$4"); new=$(now "$3" "$4")
    if changed "$2" "$3" "$4"; then
      if [ -n "$old" ] && [ "$old" = "$new" ]; then
        warn "$1: Code seit $LAST_TAG geaendert, Version steht weiter auf $new"
      else
        ok "$1: $old -> $new (Code geaendert)"
      fi
    else
      if [ -n "$old" ] && [ "$old" != "$new" ]; then
        warn "$1: Version $old -> $new, aber kein Code geaendert — DIR-004 hebt nur bei echter Aenderung an"
      else
        ok "$1: unveraendert bei $new, kein Bump noetig"
      fi
    fi
  }
  check_comp "STM" "$STM_SRC" src/main.h                   "define VERSION"
  check_comp "ESP" "$ESP_SRC" ESP8266/ESP-uclock/version.h "define ESP_VERSION"
  check_comp "PWA" "$PWA_SRC" "$APP/app.js"                "const APP_VERSION"

  # APP_VERSION und CACHE_NAME gehoeren zusammen, unabhaengig von DIR-004: ohne
  # CACHE_NAME-Bump liefert der Service Worker neue index.html mit alter app.js.
  a_old=$(vat "$LAST_TAG" "$APP/app.js" "const APP_VERSION"); a_new=$(now "$APP/app.js" "const APP_VERSION")
  c_old=$(vat "$LAST_TAG" "$APP/sw.js"  "const CACHE_NAME");  c_new=$(now "$APP/sw.js"  "const CACHE_NAME")
  if [ "$a_old" != "$a_new" ] && [ "$c_old" = "$c_new" ]; then
    crit "SW-Cache: APP_VERSION $a_old -> $a_new, CACHE_NAME bleibt $c_new — SW liefert neue index.html mit alter app.js"
  else
    ok "SW-Cache: $c_new passt zu APP_VERSION $a_new"
  fi
fi

# ----------------------------------------------------------- S5 .gz-Artefakte
step S5 "gz-Artefakte vorhanden, nicht leer, nicht veraltet"
# Die Sprachdateien kommen aus dem VERZEICHNIS, nicht aus einer festen Liste -- aus
# demselben Grund, den GZIP_SOURCES im Makefile nennt: Jede zusaetzliche Sprache ist
# eine eigene i18n/<code>.json, und eine Aufzaehlung veraltet beim naechsten Mal still.
#
# Dass das hier gefehlt hat, ist am 04.10.2026 aufgefallen und zwar dem Umsetzer von
# Task 1.4, nicht der Pruefung: Er aenderte en.json, und S5 meldete nur app.js.gz als
# veraltet. Waere der Hinweis nicht gekommen, haette der Build eine alte englische
# Tabelle ausgeliefert und die neuen Schluessel staenden woertlich auf dem Bildschirm.
# Der Makefile hatte diese Lehre schon gezogen (Zeile 23), diese Stufe nicht.
GZ_SOURCES="$APP/app.js $APP/styles.css $APP/index.html $APP/sw.js $APP/manifest.webmanifest $APP/layout-previews.json $APP/icons/icon-192.svg $APP/icons/icon-512.svg"
for f in "$APP"/i18n/*.json; do [ -e "$f" ] && GZ_SOURCES="$GZ_SOURCES $f"; done
for src in $GZ_SOURCES; do
  gz="$src.gz"
  base=$(basename "$src")
  if [ ! -f "$src" ]; then crit "Quelle fehlt: $src"; continue; fi
  if [ ! -f "$gz" ]; then crit "$base.gz fehlt — App-Assets werden nur als .gz ausgeliefert"; continue; fi
  if [ ! -s "$gz" ]; then crit "$base.gz ist 0 Byte — fuehrt zu White-Screen und bleibt im Service-Worker-Cache"; continue; fi
  if [ "$src" -nt "$gz" ]; then warn "$base.gz ist aelter als die Quelle — 'make app-gz' noetig"; else ok "$base.gz"; fi
done

# ------------------------------------------------------------ S6 CSS-Klassen
step S6 "CSS-Klassen: Regel und Verwendung"
# Gegenrichtung zu unused-css.mjs. Mit L59 stellte app.js sieben Schalter auf eine
# Klasse um, die es im Stylesheet nicht gab -- sie zeigten ihren Ein-Zustand gar
# nicht mehr an, und keine Stufe hat es gemeldet. Die Klasse war ja benutzt.
node tools/checks/css-ohne-regel.mjs "$APP/app.js" "$APP/styles.css" || WARN=$((WARN+1))
node tools/checks/unused-css.mjs "$APP/styles.css" "$APP/index.html" "$APP/app.js"

# ------------------------------------------- S7 Lint gegen quick-reference.md
step S7 "Muster aus knowledge/quick-reference.md"
# Bestandswache, nicht Nullforderung. Zwei der urspruenglich drei Zeilen fuellten den
# 64-Zeilen-Ring bei jedem Refresh -- unter LED-Last war er nach 1,2 s voll, waehrend
# die PWA nur alle 2500 ms abfragt (BEFUNDE.md, L75). Am Geraet gemessen: 96 % aller
# Ringzeilen waren diese beiden. Sie sind entfernt und durch Zaehler ersetzt.
#
# Die EINE verbliebene Zeile (sk6812.c:463) bleibt mit Absicht: Sie feuert hoechstens
# einmal je Sekunde und nur, wenn der DMA wartet -- sie ist das Widerlegungsinstrument
# fuer Geraetetest M3 (REVIEW.md:147). Eine Stufe, die sie dauerhaft anmahnt, wuerde
# frueher oder spaeter dazu fuehren, dass jemand sie entfernt.
# Geprueft werden BEIDE Richtungen, und die zweite ist die wichtigere.
#
# Eine reine Zaehlung "erwartet: 1" war der erste Entwurf und hat eine Luecke: Wer die
# Warteschleifen-Meldung entfernt und dafuer eine neue unbedingte Zeile einbaut, kommt
# wieder auf 1 und besteht. Deshalb wird die eine erlaubte Zeile NAMENTLICH ausgenommen
# und ihr Vorhandensein eigens geprueft. Aufgefallen ist die Luecke dem Agenten, der
# Task 2 umgesetzt hat -- an meiner eigenen Fassung.
n=$($GREP -c '^\s*log_printf' src/sk6812/sk6812.c 2>/dev/null); n=${n:-0}
w=$($GREP -c '^\s*log_printf ("sk6812_refresh: waiting' src/sk6812/sk6812.c 2>/dev/null); w=${w:-0}
if [ "$((n - w))" -gt 0 ]; then
  warn "sk6812.c: $((n - w)) unbedingte log_printf im Refresh-Pfad — fuellen den 64-Zeilen-Ring (L75)"
else
  ok "sk6812.c: kein unbedingtes log_printf im Refresh-Pfad"
fi
# Die Warteschleifen-Meldung MUSS bleiben: Sie feuert hoechstens einmal je Sekunde und
# nur bei wartendem DMA -- sie ist das Widerlegungsinstrument fuer Geraetetest M3
# (REVIEW.md:147). Ohne diese Haelfte der Pruefung bestuende "alle drei entfernt".
if [ "$w" -eq 1 ]; then
  ok "sk6812.c: Warteschleifen-Meldung vorhanden (M3-Instrument)"
else
  warn "sk6812.c: Warteschleifen-Meldung fehlt ($w statt 1) — Geraetetest M3 hat sein Instrument verloren"
fi
# Diese Stufe zaehlte frueher nur in src/main.c und meldete deshalb dauerhaft
# "nur 1 Aufrufstelle", obwohl display_test() seit 3.2.8 zwei eigene hat. Ein
# Dauer-Falschbefund ist schlimmer als keine Pruefung -- man liest ihn irgendwann
# nicht mehr. Gezaehlt wird jetzt ueber den ganzen Baum, und $GREP traegt das -a:
# src/display/display.c ist ISO-8859-1, ohne -a stuft grep sie als BINAER ein und
# gibt gar nichts aus. Nicht "0 Treffer", sondern "nicht gelesen".
#
# Die Aussage hat sich dabei umgedreht. Massnahme 1 ist fuer die Schleifen ohne
# Abbruchbedingung erledigt (display_test, remote_ir_learn). Die Stufe bewacht den
# Bestand: Faellt die Zahl, ist ein Fix verlorengegangen.
#
# ZU var_send_buf() STAND HIER BIS ZUM 03.10.2026 "bekommt BEWUSST keinen Reload".
# Der Einwand war richtig und bleibt es -- nur traf er den BEDINGUNGSLOSEN Reload:
# Dort gibt es seit 3.2.8 einen 3-s-Abbruch, und der daraus folgende Watchdog-Reset
# hat die Uhr am 02.10.2026 nach sieben Sekunden wieder ins Leben gebracht (L25).
# Ein Reload bei jedem Durchlauf haette daraus wieder ein stilles Steckenbleiben
# gemacht.
#
# Seit 03.10.2026 steht dort ein Reload, der NUR nach eingetroffener Quittung
# ausloest. Er belohnt Fortschritt, nicht Warten:
#
#   antwortende Bruecke -> Watchdog bedient, der lange Startpfad laeuft durch
#                          (var_send_all_variables: ~194 Kommandos, L85)
#   tote Bruecke        -> kein Reload, Reset nach rund sieben Kommandos wie bisher,
#                          die Selbstheilung aus L25 bleibt unangetastet
#
# Belegt und nicht nur beabsichtigt: ESP8266_OK entsteht seit dem Bruecken-Paket
# NUR NOCH aus der Zeile "." (esp8266.c:227). Der Praefixtest auf "OK" liefert
# seither ESP8266_STATUS -- der ESP schickt waehrend seines Bootlaufs unaufgefordert
# "OK cap", "OK ap" und "OK time", und jede dieser Zeilen quittierte vorher ein
# FREMDES Kommando und lud den Watchdog auf eine Falschmeldung hin nach.
# (Dieser Kommentar nannte bis dahin esp8266.c:301 als zweite Quelle. Die Stelle
# gibt es noch, sie liefert aber nicht mehr ESP8266_OK.)
#
# Dazu ein Zeitbudget: Der Reload greift nur nach Quittung UND innerhalb von
# VAR_SEND_RELOAD_BUDGET_SEC seit dem ersten wartenden Sender. Ohne das durfte eine
# sporadisch antwortende Bruecke den Hauptloop bis zu zehn Minuten blockieren --
# jede eintreffende Quittung hielt den Watchdog am Leben (BEFUNDE.md, L108).
#
# 8 seit 04.10.2026: Hauptloop, remote_ir_learn, display_test (2), Ticker-
# Warteschleife (2), var_send_buf (nach Quittung und innerhalb des Budgets) und
# display_wait_for_tables (A22). Die Zahl veraltet still, wenn sie beim Nachruesten
# vergessen wird -- der Schutz griffe dann erst, wenn mehrere Stellen fehlen.
#
# Diese Aufzaehlung stand bis zum 04.10.2026 auf sieben, waehrend WD_EXPECTED und der
# Warntext darunter bereits acht fuehrten. Die Pruefung war also richtig und ihre
# Begruendung falsch -- genau der Zustand, der eine Warnung ihre Ursache ueberleben
# laesst (CLAUDE.md, "Behobene Falle im Release-Build"). Gemeldet hat es ein Agent,
# der beim Nachzaehlen der Aufrufstellen hiergeblieben ist.
WD_EXPECTED=8
n=$($GREP -rc 'watchdog_reload ()\s*;' src --include='*.c' 2>/dev/null | $GREP -v ':0$' | awk -F: '{s+=$2} END {print s+0}')
n=${n:-0}
if [ "$n" -lt "$WD_EXPECTED" ]; then
  warn "watchdog_reload() hat nur $n Aufrufstellen, erwartet sind $WD_EXPECTED — ist ein Fix verlorengegangen? (Hauptloop, remote_ir_learn, display_test 2x, Ticker-Warteschleife 2x, var_send_buf nach Quittung, display_wait_for_tables)"
else
  ok "watchdog_reload(): $n Aufrufstellen, Bestand vollstaendig"
fi
# innerHTML: eigene Pruefung statt zeilenbasiertem grep. Die alte Fassung meldete
# 21 korrekte .map()-Stellen und uebersah app.js:5667, weil dort escapeHtml fuer den
# Fallback auf derselben Zeile steht, waehrend der Wert roh eingesetzt wird.
ih=$(node tools/checks/innerhtml.mjs "$APP/app.js" 2>&1)
if printf '%s' "$ih" | $GREP -q "HOCH"; then
  printf '%s\n' "$ih"
  WARN=$((WARN + $(printf '%s' "$ih" | $GREP -c "HOCH")))
else
  ok "app.js: jede innerHTML-Zuweisung ist gefiltert"
fi
n=$($GREP -cE 'catch\s*\(\s*_\s*\)\s*\{\s*\}|catch\s*\{\s*\}' "$APP/app.js" 2>/dev/null); n=${n:-0}
[ "$n" -gt 0 ] && warn "app.js: $n leere catch-Bloecke — verschlucken Fehler stillschweigend" || ok "app.js: keine leeren catch-Bloecke"

# ------------------------------------------------ S7b Kodierung der Quellen
step S7b "Kodierung der C- und ESP-Quellen"
#
# Zwei verschiedene Fallen, und sie zeigen in entgegengesetzte Richtungen:
#
#   nicht UTF-8 (ISO-8859-1)  -> grep ohne -a gibt GAR NICHTS aus statt "keine Treffer"
#   UTF-8 mit Nicht-ASCII     -> ein latin-1-Patcher BESCHAEDIGT die Datei
#
# Die zweite Gruppe fehlte hier, und CLAUDE.md behauptete pauschal, die ESP-Quellen
# seien ASCII oder ISO-8859-1. Fuer http.cpp und stm32flash.cpp stimmt das nicht --
# aufgefallen am 03.10.2026 einem Agenten, der nachgesehen hat, statt der Anweisung
# zu folgen. Deshalb nennt diese Stufe die Dateien jetzt beim Namen, statt nur zu
# zaehlen: Wer patcht, muss wissen, welche Gruppe vor ihm liegt.
#
# Erkannt wird ueber Bytes > 127, NICHT ueber "nicht druckbar". Der erste Entwurf
# nahm grep '[^ -~\t]' und meldete 130 Dateien -- er hatte das CR der CRLF-Zeilen
# miterfasst. Eine Liste, in der fast alles steht, sagt nichts.
python3 - <<'PYEOF'
import os
utf8, latin = [], []
for root in ("src", "ESP8266/ESP-uclock"):
    for dirpath, _, names in os.walk(root):
        for name in sorted(names):
            if not name.endswith((".c", ".h", ".cpp", ".ino")):
                continue
            path = os.path.join(dirpath, name)
            try:
                raw = open(path, "rb").read()
            except OSError:
                continue
            if not any(b > 127 for b in raw):
                continue                      # reines ASCII, in beiden Welten harmlos
            try:
                raw.decode("utf-8")
                utf8.append(path)
            except UnicodeDecodeError:
                latin.append(path)

if utf8:
    print("  INFO      UTF-8 MIT Umlauten -- ein latin-1-Patcher BESCHAEDIGT diese Dateien:")
    for p in sorted(utf8):
        print(f"            {p}")
if latin:
    print(f"  INFO      {len(latin)} Datei(en) nicht UTF-8 -- grep braucht dort -a, sonst stuft es sie")
    print( "            als binaer ein und gibt GAR NICHTS aus. LC_ALL=C allein reicht nicht.")
    for p in sorted(latin)[:3]:
        print(f"            {p}")
if not utf8 and not latin:
    print("  OK  alle C- und ESP-Quellen sind reines ASCII")
PYEOF

# ---------------------------------------------------------- S8 Smoke-Tests
step S8 "Smoke-Tests pro Modul"
node tools/checks/smoke-pwa.mjs "$APP/app.js" || CRIT=$((CRIT+1))

# Die PWA ist UTF-8 und schreibt echte Umlaute; die C-Dateien sind es nicht und
# benutzen Umschrift. Wer in einer Sitzung an beiden arbeitet, traegt die
# Gewohnheit hinueber -- so kamen "Schluessel" und "ungueltig" in die deutschen
# Fehlertexte (L52). Aufgefallen ist es dem Nutzer, nicht einer Pruefung.
node tools/checks/umlaute.mjs "$APP/app.js" || WARN=$((WARN+1))
if [ "$FULL" -eq 1 ]; then
  echo "  --full: Compile-Smoke-Tests (dauert Minuten, nur serieller Lauf erlaubt)"
  # ESP_WARNINGS=all ist der Kern dieses Schritts, nicht Beiwerk: Bis 03.10.2026 lief
  # "make esp" hier mit der arduino-cli-Vorgabe "none". Der Smoke-Test meldete
  # "uebersetzt" und konnte dabei GAR KEINE Warnung ausgeben -- sechs Bestandswarnungen
  # blieben jahrelang unsichtbar. Gefunden beim Bauen von F1, weil jemand gegenprueffte,
  # statt der gruenen Zeile zu glauben.
  # Zeitmarke VOR den Builds. Daran erkennt die Wache unten, ob ueberhaupt etwas
  # uebersetzt wurde -- siehe die Begruendung dort.
  BUILD_STAMP=$(mktemp)
  for t in f103 f411 esp; do
    if make "$t" ESP_WARNINGS=all >/tmp/guardrail-$t.log 2>&1; then ok "make $t uebersetzt"
    else crit "make $t schlaegt fehl — siehe /tmp/guardrail-$t.log"; tail -5 /tmp/guardrail-$t.log; fi
  done

  # Bestandswache statt Schwelle, dasselbe Mittel wie bei watchdog_reload in S7: Die
  # bekannten Warnungen sollen sichtbar bleiben, aber nur eine NEUE soll auffallen.
  # Wer eine behebt, senkt die Zahl hier mit -- sonst meldet die Stufe es.
  STM_WARN_EXPECTED=18
  ESP_WARN_EXPECTED=0
  for t in f103 esp; do
    [ -f /tmp/guardrail-$t.log ] || continue
    n=$(grep -c 'warning:' /tmp/guardrail-$t.log 2>/dev/null | tr -d ' ')
    case "$t" in
      f103) want=$STM_WARN_EXPECTED; objdir=build/stm-rgbw-12h;;
      esp)  want=$ESP_WARN_EXPECTED; objdir=build/esp8266;;
    esac

    # Wurde in diesem Lauf ueberhaupt uebersetzt? Ein Inkrementallauf laesst alle
    # Objektdateien unberuehrt und meldet deshalb NULL Warnungen -- eine Zahl, die
    # nichts belegt.
    #
    # Frueher haing diese Unterscheidung am Vergleich "weniger als erwartet". Das
    # traegt nur, solange der Erwartungswert ueber null liegt: Sinkt er auf 0 -- und
    # genau das ist mit dem Hygiene-Paket passiert --, meldet ein Inkrementallauf
    # 0 == 0 und damit ein gruenes "Bestand unveraendert". Die Wache haette sich
    # also ausgerechnet in dem Moment selbst abgeschaltet, in dem der Bestand
    # sauber ist. Aufgefallen ist das dem Agenten, der die Senkung vorbereitet hat,
    # nicht beim Schreiben dieser Stufe.
    # .obj ZUERST, denn das ist die Endung, die hier wirklich zaehlt: CMake legt im
    # STM-Baum 161 .obj an und genau EINE .o -- eine Compiler-Probedatei, die nur beim
    # Konfigurieren entsteht. Die erste Fassung suchte nur nach .o und .a und meldete
    # deshalb "19 Warnungen bei 1 uebersetzten Dateien" statt bei 67. Schlimmer: Laeuft
    # --full ohne vorheriges clean, ist auch diese eine Datei nicht frisch, fresh wird 0
    # und die Stufe meldet "in diesem Lauf wurde NICHTS uebersetzt", obwohl alles neu
    # gebaut wurde -- genau die Verwechslung, die der Kommentar oben verhindern soll.
    # Gemeldet vom release-engineer, der die Zahl 1 nicht als plausibel hingenommen hat.
    fresh=$(find "$objdir" -newer "$BUILD_STAMP" \( -name '*.obj' -o -name '*.o' -o -name '*.a' \) 2>/dev/null | wc -l | tr -d ' ')

    if [ "$n" -gt "$want" ]; then
      printf '  HOCH      %s: %s Compilerwarnungen, erwartet waren %s — die neuen stehen in /tmp/guardrail-%s.log\n' "$t" "$n" "$want" "$t"
      grep 'warning:' /tmp/guardrail-$t.log | tail -3 | sed 's/^/            /'
      WARN=$((WARN+1))
    elif [ "$fresh" -eq 0 ]; then
      printf '  INFO      %s: %s Warnungen, aber in diesem Lauf wurde NICHTS uebersetzt (0 frische Objektdateien).\n' "$t" "$n"
      echo   '            Die Zahl belegt damit gar nichts. Zum Nachpruefen: make clean-stm clean-esp,'
      echo   '            dann erneut --full.'
    elif [ "$n" -lt "$want" ]; then
      printf '  INFO      %s: nur %s Warnungen statt %s, bei %s uebersetzten Dateien — offenbar behoben.\n' "$t" "$n" "$want" "$fresh"
      echo   '            Dann ...WARN_EXPECTED in dieser Datei senken, sonst faellt die naechste neue nicht auf.'
    else
      ok "$t: $n Compilerwarnungen bei $fresh uebersetzten Dateien, Bestand unveraendert"
    fi
  done
else
  echo "  uebersprungen: STM- und ESP-Compile-Smoke-Test (nur mit --full)"
fi

# --------------------------------------------- S8b Flashbelegung des F103
# A25: Der F103 hatte am 03.10.2026 noch 296 Byte frei (L165, L170) -- also
# praktisch nichts. Geloest wurde das mit -flto=4, aber eine geloeste Enge bleibt
# eine Enge: Der naechste Funktionszuwachs sprengt sie wieder, und zwar erst beim
# Linken, lange nachdem der Code geschrieben ist.
#
# Die Stufe ist bewusst NICHT statisch wie die uebrigen -- sie braucht das
# Build-Artefakt. Fehlt es, meldet sie das und urteilt nicht. Eine Pruefung, die
# ohne Messgrundlage eine Zahl behauptet, waere schlimmer als keine.
#
# Gezaehlt werden die Nutzdatenbytes der Intel-HEX-Datensaetze vom Typ 00. Das ist
# der belegte Flash, nicht die Dateigroesse -- die ist rund dreimal so gross, weil
# HEX jedes Byte als zwei Zeichen ablegt.
step S8b "Flashbelegung STM32F103 (64 KiB)"
F103_HEX=build/stm-rgbw-12h/wc12h-stm32f103-sk6812-rgbw.hex
if [ ! -f "$F103_HEX" ]; then
  echo "  INFO      nicht gebaut - 'make f103' noetig, keine Aussage moeglich"
else
  belegt=$(python3 -c "
import sys
n=0
for l in open(sys.argv[1]):
    if l.startswith(':') and l[7:9]=='00': n+=int(l[1:3],16)
print(n)" "$F103_HEX" 2>/dev/null)
  [ -n "$belegt" ] || belegt=0
  frei=$((65536 - belegt))
  if   [ "$frei" -lt 512 ];  then crit "F103: nur $frei Byte frei ($belegt von 65536) - der naechste Zuwachs sprengt den Flash"
  elif [ "$frei" -lt 2048 ]; then warn "F103: $frei Byte frei ($belegt von 65536) - eng, vor neuen Funktionen pruefen"
  else ok "F103: $frei Byte frei ($belegt von 65536 belegt)"; fi
fi

# ------------------------------------------------- S9 Aktualitaet der Doku
step S9 "Versionsangaben in der lebenden Dokumentation"
# Momentaufnahmen sind ausgenommen (DIR-006): REVIEW*.md, gap-analysis.md,
# CHANGELOG.md und specs/** halten bewusst ihren Entstehungsstand fest.
LIVE_DOCS="CLAUDE.md BEFUNDE.md HARDWARE.md TESTPLAN-PWA.md README.md README-CMAKE.md ESP8266/ESP-uclock/README-PWA.md
           ESP8266/ESP-uclock/APP-BUNDLE.md tools/preview/README.md specs/README.md"
for d in knowledge/*.md .claude/agents/*.md .claude/skills/*/SKILL.md; do [ -f "$d" ] && LIVE_DOCS="$LIVE_DOCS $d"; done
node tools/checks/doc-versions.mjs $LIVE_DOCS || WARN=$((WARN+1))

# Absolute Benutzerpfade zeigen bei jedem anderen Klon ins Leere.
n=$(git ls-files | xargs $GREP -l '/Users/[a-z]' 2>/dev/null | wc -l | tr -d ' ')
if [ "${n:-0}" -gt 0 ]; then
  warn "absolute Benutzerpfade in $n versionierten Datei(en):"
  git ls-files | xargs $GREP -ln '/Users/[a-z]' 2>/dev/null | sed 's/^/            /'
else
  ok "keine absoluten Benutzerpfade in versionierten Dateien"
fi

# Interne Netzstruktur gehoert nicht in ein oeffentliches Repository. Adressen und
# Hostnamen kommen aus tools/device.conf und tools/deploy.conf -- beide gitignored.
# Ausgenommen: Dokumentationsadressen nach RFC 5737 (192.0.2.*, 198.51.100.*,
# 203.0.113.*) und wctris/, das aus dem Ursprungsprojekt stammt.
# Zwei getrennte Muster: eine vollstaendige IPv4 aus einem privaten Bereich, und
# Hostnamen mit einer Heimnetz-Endung. Die Oktettzahl muss je Praefix stimmen --
# ein verkuerztes "10\.[0-9]+\.[0-9]+" wuerde jede Versionsnummer wie 10.5.3 melden.
NET_IP='(^|[^0-9.])(10(\.[0-9]{1,3}){3}|192\.168(\.[0-9]{1,3}){2}|172\.(1[6-9]|2[0-9]|3[01])(\.[0-9]{1,3}){2})([^0-9.]|$)'
# Der Unterstrich muss in der Abgrenzung stehen: sonst meldet jeder i18n-Schluessel
# wie "maintenance.local_update" einen Treffer.
NET_HOST='[A-Za-z0-9-]+\.(lan|home|fritz\.box|local)([^A-Za-z0-9._-]|$)'
hits=$(git ls-files | $GREP -v '^wctris/' \
       | xargs $GREP -nE "$NET_IP|$NET_HOST" 2>/dev/null \
       | $GREP -vE '192\.0\.2\.|198\.51\.100\.|203\.0\.113\.|example\.(lan|local)|wordclock-pi\.local')
if [ -n "$hits" ]; then
  warn "interne Netzadressen in versionierten Dateien:"
  printf '%s\n' "$hits" | sed 's/^/            /' | head -10
else
  ok "keine internen Netzadressen in versionierten Dateien"
fi

# --------------------------------------------- S10 Vollstaendigkeit Katalog
step S10 "Massnahmenkatalog BEFUNDE.md vollstaendig"
node tools/checks/befunde-katalog.mjs || WARN=$((WARN+1))

# ------------------------------------------- S11 Direktivenkatalog vollstaendig
step S11 "Direktivenkatalog vollstaendig"
node tools/checks/direktiven.mjs || WARN=$((WARN+1))

# ------------------------------------------- S12 Gefahrenliste an einem Ort
step S12 "Gefahrenliste der Endpunkte an genau einem Ort"
node tools/checks/endpunkte.mjs || WARN=$((WARN+1))

# -------------------------------------------- S13 Pruefstaende uebersetzen
# Was diese Stufe leistet und was NICHT (W.6):
#
# Sie uebersetzt die Pruefstaende unter tools/checks/ und laeuft sie an. Das faengt
# den Fall, dass ein Pruefstand nach einer Aenderung gar nicht mehr baut -- und es
# faengt eine falsche Typbreite in pruefstand.h, weil der Header seine eigenen
# Breiten zur Uebersetzungszeit prueft und sonst abbricht.
#
# Sie prueft NICHT, ob die Firmware mit den Vektoren uebereinstimmt: var-crc.c
# ERZEUGT die Vektoren, der Abgleich gegen STM und ESP gehoert in die Runde, die
# die Pruefsumme anfasst. Eine Stufe, die hier mehr behauptete, waere eine
# Pruefung, die ihren Gegenstand nicht hat.
step S13 "Pruefstaende uebersetzen und anlaufen"
if command -v cc >/dev/null 2>&1; then
  # KEIN zusaetzliches WARN=$((WARN+1)) hier: warn() zaehlt selbst (Zeile 32). Der
  # erste Entwurf hatte beides und meldete bei EINEM kaputten Pruefstand drei
  # Hoch-Findings statt zwei. Das || WARN=$((WARN+1)) der Nachbarstufen gilt fuer
  # node-Aufrufe, die ueber ihren Exit-Code melden und nicht ueber warn().
  for src in tools/checks/*.c; do
    [ -e "$src" ] || continue
    if cc -I tools/checks -o "/tmp/gr-$(basename "$src" .c)" "$src" 2>/tmp/gr-cc.err; then
      if "/tmp/gr-$(basename "$src" .c)" >/dev/null 2>&1; then
        ok "$(basename "$src") uebersetzt und laeuft"
      else
        warn "$(basename "$src") uebersetzt, bricht aber beim Lauf ab"
      fi
    else
      warn "$(basename "$src") uebersetzt nicht: $(head -2 /tmp/gr-cc.err | tr '\n' ' ')"
    fi
  done
else
  ok "kein cc vorhanden, Pruefstaende uebersprungen"
fi

# ------------------------------------------------- S14 Hooks tun, was sie sollen
step S14 "Hook-Gegenproben in beide Richtungen"
./tools/checks/hooks.sh || WARN=$((WARN+1))

# -------------------------------------------------------------------- Fazit
printf '\n=== Ergebnis: %d Pruefung(en) mit Kritisch-Findings, %d Hoch-Findings ===\n' "$CRIT" "$WARN"
if [ "$CRIT" -gt 0 ]; then
  echo "BLOCKIERT — Task gilt nicht als abgeschlossen, keine Schreibuebergabe."
  exit 1
fi
echo "BESTANDEN — Kritisch-Findings keine. Hoch-Findings pruefen, Abweichung begruenden."

# Stempel fuer den Stop-Hook (tools/hooks/guardrails-before-stop.py): haelt fest,
# fuer WELCHEN Aenderungsstand dieser Lauf galt. Aendert sich danach etwas, gilt
# der Lauf nicht mehr. Liegt unter .git/ und wird deshalb nicht versioniert.
python3 tools/hooks/guardrails-before-stop.py --stamp 2>/dev/null || true

exit 0
