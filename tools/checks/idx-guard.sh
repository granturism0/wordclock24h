#!/usr/bin/env bash
# Prueft, dass die Indexguards aus A43/L286 und A44/L289 im Quelltext stehen UND wirken.
#
# Der Pruefstand wird bei jedem Lauf frisch aus src/main.c und src/display/display.c
# erzeugt (idx-guard-gen.py): Die drei Handler, esp8266_idx_ok() und die fuenf Setter
# werden WOERTLICH herausgeschnitten, nicht nachgebaut. Entfernt jemand einen Guard,
# schlaegt diese Stufe an -- hergestellt und gemessen, nicht angenommen: Mit
# herausgeschnittenem CD-Guard meldet sie 253 Verletzungen und nennt das Kommando.
#
# ZWEI LAEUFE, UND DER ZWEITE IST DER WICHTIGERE
#
#   mit Guards   -> 0 Verletzungen   sonst ist der Schutz weg
#   ohne Guards  -> >0 Verletzungen  sonst prueft der Pruefstand NICHTS
#
# Die Gegenprobe gehoert in die Stufe, nicht nur in den Einbautag (DIR-014): Wird der
# Pruefstand irgendwann entschaerft, meldete er sonst fuer immer gruen.
#
# WAS ER MISST UND WAS NICHT
#
# Uebersetzt wird fuer den HOST, nicht fuer Cortex-M. Die absoluten Offsets und
# sizeof(DISPLAY_GLOBALS) gelten deshalb NICHT fuer das Geraet -- uint_fast8_t ist dort
# anders breit. Gemessen werden aber ausschliesslich RELATIVE Aussagen: die Arraygrenzen
# (aus sizeof), die Frage "schreibt der Handler ausserhalb seines Arrays", und die Zahl
# der Abweisungen. Die Warnung aus L256 greift hier also nicht -- keine Typbreite
# entscheidet ueber den geprueften Fall. Es steht hier, damit niemand spaeter die
# Offsetzahlen als Geraetewerte zitiert.
ROOT="$(git rev-parse --show-toplevel)"; cd "$ROOT" || exit 2
command -v cc >/dev/null 2>&1 || { echo "  OK  kein cc vorhanden, Indexpruefung uebersprungen"; exit 0; }

TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT   # fester Pfad hat schon einmal ein altes
                                                # Binary als eigenes Ergebnis untergeschoben

# inc/ traegt stm32f4xx.h, SPL/inc/ die misc.h, cmsis/ die core_cm4.h -- alle drei sind
# GESCHWISTER von src/, keine Unterverzeichnisse. Genau daran scheitert die Suche sonst.
INC=(-Isrc -Iinc -ISPL/inc -Icmsis -Itools/checks)
for d in src/*/; do INC+=("-I$d"); done

# Woertlich aus CMakeLists.txt, Ziel wordclock_f411_rgbw. Zwingend ist nur STM32F411xE
# (sonst #error in stm32f4xx.h) -- der Rest steht hier, damit der Pruefstand dieselbe
# Uebersetzung sieht wie das Fabrikat. Beide Varianten wurden gegeneinander laufen
# gelassen: identische Ausgabe. Ein Define, das morgen eine Arraygroesse beeinflusst,
# wuerde sonst still gegen etwas anderes messen als das Geraet.
DEF=(-DARM_MATH_CM4 -DSTM32F411CE -DSTM32F411xE -DSTM32F411xx -DSTM32F411 \
     -DSTM32F4XX -DBLACKPILL_BOARD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=25000000 -DWCLOCK24H=0)

python3 tools/checks/idx-guard-gen.py               > "$TMP/mit.c"  2>/dev/null || {
  echo "  HOCH      Pruefstand liess sich nicht erzeugen"; exit 1; }
python3 tools/checks/idx-guard-gen.py --strip-guard > "$TMP/ohne.c" 2>/dev/null || {
  echo "  HOCH      Pruefstand (Gegenprobe) liess sich nicht erzeugen"; exit 1; }
cc -std=c99 -w "${DEF[@]}" "${INC[@]}" "$TMP/mit.c"  -o "$TMP/mit"  2>"$TMP/cc1" || {
  echo "  HOCH      Pruefstand uebersetzt nicht: $(head -1 "$TMP/cc1")"; exit 1; }
cc -std=c99 -w "${DEF[@]}" "${INC[@]}" "$TMP/ohne.c" -o "$TMP/ohne" 2>/dev/null || {
  echo "  HOCH      Gegenprobe uebersetzt nicht"; exit 1; }

fehler=0
if "$TMP/mit" > "$TMP/mit.log" 2>&1; then
  # Geltungsbereich ausdruecklich: Der Pruefstand schneidet die drei A44-Handler aus,
  # NICHT die drei A43-Handler (night, ambinight, alarm). Ohne Sollwert in der Meldung
  # liest sich "OK" wie "alle sechs geprueft" (H.2-Review, 05.10.2026).
  echo "  OK  Indexpruefung: 0 Verletzungen ueber alle 256 Indizes (A44: 3 von 6 Aufrufstellen; A43 nicht abgedeckt)"
else
  echo "  KRITISCH  Indexpruefung: Schreibzugriff ueber den Arrayrand (A43/L286, A44/L289)"
  sed -n '1,6p' "$TMP/mit.log" | sed 's/^/            /'
  fehler=1
fi
if "$TMP/ohne" > "$TMP/ohne.log" 2>&1; then
  echo "  KRITISCH  Der Pruefstand schlaegt OHNE die Guards nicht an — er misst nichts"
  fehler=1
else
  echo "  OK  Gegenprobe ohne Guards schlaegt an ($(grep -a '^VERLETZUNGEN' "$TMP/ohne.log" | head -1))"
fi
exit $fehler
