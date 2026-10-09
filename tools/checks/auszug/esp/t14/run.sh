#!/bin/sh
# Pruefstand E.2b (Teil D ESP-Seite, Paket 2026-10-09). Aufruf: run.sh <verzeichnis ESP-uclock> <bezeichnung>
# AKD.2 byte-gleich gegen das letzte ESP-Release vor Teil D (REF), AKD.3 Marken gegen
# tools/checks/var-crc.c, AKD.4 Lernen und Verlieren auf allen Wegen. Braucht das Repo (ROOT) fuer
# REF und den Schiedsrichter. Dazu zwei Zaehlungen am Quelltext: Absendestellen gegen FORM_STELLEN,
# Resetwege (digitalWrite STM32_RESET_PIN LOW) nur in Funktionen, die zuerst die Faehigkeit loeschen.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
REF=release/3.2.22-3.2.26-1.4.94
ROOT=${ROOT:-$(git -C "$D" rev-parse --show-toplevel 2>/dev/null)}
[ -n "$ROOT" ] || { echo "ROOT unbekannt ($B)"; exit 2; }
CC="cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format-truncation"
rm -rf "$D/ref" && mkdir -p "$D/ref"
for f in vars.cpp vars.h base.h version.h udpsrv.cpp; do
  git -C "$ROOT" show "$REF:ESP8266/ESP-uclock/$f" > "$D/ref/$f" || { echo "REF $f fehlt ($B)"; exit 2; }
done
python3 "$S/extract.py" func "$ROOT/tools/checks/var-crc.c" var_crc | sed 's/^var_crc (/schiri_crc (/; s/^uint16_t$/static uint16_t/' > "$D/schiri.inc"
# Tetris-Fall: Referenz
python3 "$S/extract.py" block "$D/ref/udpsrv.cpp" "case LISTENER_TETRIS_CODE:" "case LISTENER_DISCOVER_CODE:" > "$D/ref/tetris_code.inc"
cp "$D/schiri.inc" "$D/forms.inc" "$D/t14.cpp" "$D/ref/"          # eigene Kopie: "..."-Includes suchen zuerst neben t14.cpp
$CC -DREF -DBEZEICHNUNG="\"ref\"" -I"$D/fake" -I"$D/ref" "$D/ref/t14.cpp" "$D/ref/vars.cpp" -o "$D/ref.bin" -lc++ 2>"$D/cc-ref.log" \
  || { echo "REFERENZ NICHT UEBERSETZBAR:"; grep error "$D/cc-ref.log" | head; exit 2; }
"$D/ref.bin" "$D/ref.out" >/dev/null || { echo "REFERENZ FEHLGESCHLAGEN"; "$D/ref.bin" "$D/ref.out"; exit 2; }
# Stand unter Pruefung
python3 "$S/extract.py" block "$Q/udpsrv.cpp" "case LISTENER_TETRIS_CODE:" "case LISTENER_DISCOVER_CODE:" > "$D/tetris_code.inc"
X=""
{ python3 "$S/extract.py" func "$Q/ESP-uclock.ino" var_crc var_crc_hexval
  if grep -a -q '^stm_cmd_send (' "$Q/ESP-uclock.ino"; then
    python3 "$S/extract.py" range "$Q/ESP-uclock.ino" "#define STM_CMD_BUF_LEN" "/*----"
  fi
  python3 "$S/extract.py" range "$Q/ESP-uclock.ino" "#define VAR_FRAME_CODE" "/* Prototypen von Hand"
  python3 "$S/extract.py" func "$Q/ESP-uclock.ino" var_hex2 var_frame_line
} > "$D/ino_code.inc" 2>"$D/extract.log"
grep -a -q '^stm_cmd_send (' "$Q/ESP-uclock.ino" || X="-DOHNE_ABSENDER"
{ python3 "$S/extract.py" range "$Q/stm32flash.cpp" "#define STM32_BOOT0_PIN1" "#define STM32_BEGIN"
  python3 "$S/extract.py" func "$Q/stm32flash.cpp" stm32_reset stm32_activate_bootloader; } > "$D/flash_code.inc" 2>>"$D/extract.log"
$CC $X -DBEZEICHNUNG="\"$B\"" -I"$D/fake" -I"$D" -I"$Q" "$D/t14.cpp" "$Q/vars.cpp" -o "$D/t14.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t14.bin" "$D/ref.out"; rc=$?
# Zaehlung 1: Absendestellen im Quelltext (aktiv, ausserhalb #if 0) gegen FORM_STELLEN
python3 - "$Q" "$D/forms.inc" <<'PY' || rc=1
import re, sys
q, forms = sys.argv[1], open(sys.argv[2]).read()
soll = int(re.search(r'#define FORM_STELLEN (\d+)', forms).group(1))
def aktiv(path):
    out, tot = [], 0
    for l in open(path, 'rb').read().decode('utf-8', 'replace').split('\n'):
        s = l.strip()
        if s.startswith('#if 0'): tot += 1; continue
        if tot and s.startswith('#if'): tot += 1; continue
        if tot and s.startswith('#endif'): tot -= 1; continue
        if not tot: out.append(l)
    return out
n = sum(1 for l in aktiv(q + '/vars.cpp') if re.search(r'(stm_cmd_printf|Serial\.printf)\s*\(\s*"(CMD )?[A-Za-z]', l) and ('stm_cmd_printf' in l or '"CMD ' in l))
n += sum(1 for l in aktiv(q + '/udpsrv.cpp') if 'stm_cmd_send' in l or 'Serial.print ("CMD ")' in l)
print('  Absendestellen im Quelltext: %d, im Pruefstand: %d' % (n, soll))
sys.exit(0 if n == soll else 1)
PY
# Zaehlung 2: jeder Weg, der den STM-Reset zieht, loescht vorher die Faehigkeit
python3 - "$Q" <<'PY' || rc=1
import re, sys, glob
q = sys.argv[1]; wege = 0; bad = 0
for p in sorted(glob.glob(q + '/*.cpp') + glob.glob(q + '/*.ino')):
    L = open(p, 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n').split('\n')
    for i, l in enumerate(L):
        if re.search(r'digitalWrite\s*\(\s*STM32_RESET_PIN\s*,\s*LOW', l):
            wege += 1; j = i
            while not re.match(r'^[a-z_0-9]+\s*\(', L[j]): j -= 1
            body = '\n'.join(L[j:i])
            if 'stm_cmd_cap_set (0' not in body.split('{', 1)[-1].lstrip()[:60]:
                bad += 1; print('  FEHL Resetweg ohne vorheriges Loeschen: %s:%d (%s)' % (p.split('/')[-1], i + 1, L[j].strip()))
print('  Resetwege (STM32_RESET_PIN LOW): %d, davon ohne Loeschen am Funktionsanfang: %d' % (wege, bad))
sys.exit(1 if bad or wege == 0 else 0)
PY
exit $rc
