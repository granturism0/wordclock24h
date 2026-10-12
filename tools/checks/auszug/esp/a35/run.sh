#!/bin/sh
# Pruefstand a35 (Paket 2026-10-11, E.1, A35 (b)): still verworfene Zuweisungen in var_set_parameter().
# Aufruf: run.sh <verzeichnis mit vars.cpp, vars.h, base.cpp> <bezeichnung>
# Echter Code: vars.cpp von "OVERLAY      overlays" bis vor vars_init (var_cmd_min_len, var_cmd_reject,
# var_ignored_*, var_set_parameter), dazu htoi/dayofweek aus base.cpp. Angleichung an die Zielplattform:
# uint_fast8_t/uint_fast16_t -> unsigned int (xtensa), "static unsigned long" -> uint32_t (L256).
# Gegen den Ausgangsstand uebersetzbar => FEHL, nicht KAPUTT.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
R=${ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null)}; [ -n "$R" ] || R=$(git -C "$Q" rev-parse --show-toplevel 2>/dev/null)
X="$S/extract.py"; [ -f "$X" ] || X="$R/tools/checks/auszug/esp/extract.py"
python3 "$X" range "$Q/vars.cpp" "#define CMD_CODE_NUMERIC_VAR" "unsigned int" > "$D/defs.inc" 2>"$D/extract.log"
python3 "$X" range "$Q/vars.cpp" "OVERLAY      overlays" "vars_init" 2>>"$D/extract.log" \
  | sed -e '$d' | sed -e 's/^static unsigned long /static uint32_t /' > "$D/code.inc"
# letzte Zeile ist das "void" vor vars_init -- entfernt; pruefen, dass es wirklich weg ist
tail -3 "$D/code.inc" | grep -q '^void *$' && { echo "AUSZUG FALSCH ($B): void am Ende"; exit 2; }
python3 "$X" func "$Q/base.cpp" dayofweek htoi > "$D/base.inc" 2>>"$D/extract.log"
for f in defs code base; do [ -s "$D/$f.inc" ] || { echo "AUSZUG LEER ($B): $f"; cat "$D/extract.log"; exit 2; }; done
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -I"$R/tools/checks" \
   -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/a35.cpp" -o "$D/a35.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/a35.bin"
