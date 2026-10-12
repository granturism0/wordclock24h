#!/bin/sh
# Pruefstand N6 (Paket 2026-10-11, S3.2): DFPlayer-Logausgabe alt (eingefroren, 3.2.30) gegen den Stand.
# Aufruf: run.sh <src-Verzeichnis> [sabotage]      Sabotagen: praefix zweizeilen grenze
# Vergleicht die Mitschrift von UART (U[..]) und ESP-Ring (E[..]) je Aufruf Byte fuer Byte.
# Zwischendateien unter $TMPDIR. Exit 0 = OK, 1 = FEHL, 2 = kaputt.
D=$(cd "$(dirname "$0")" && pwd); SRC=$1; SAB=${2:-}
W=$(mktemp -d "${TMPDIR:-/tmp}/n6.XXXXXX") || exit 2
trap 'rm -rf "$W"' EXIT
mkdir -p "$W/alt" "$W/neu"
for f in aufrufe.c helfer.c log.c log.h; do cp "$D/alt-$f" "$W/alt/$f" || exit 2; done
python3 "$D/extract.py" "$SRC" "$W/neu" neu $SAB || exit 2
for s in alt neu; do
  printf '#define ESP8266_H\n' > "$W/$s/esp8266.h"
  printf 'void log_uart_puts (const char *); void log_uart_putc (int); void log_uart_init (uint32_t); void log_uart_flush (void);\n' > "$W/$s/uart.h"
  printf 'void esp8266_send_log_line (const char *);\n' >> "$W/$s/esp8266.h"
  Z=""; [ $s = alt ] && Z="-DSEITE_ALT"
  cc -std=gnu99 -Wall -Wno-unused-function -Wno-format-extra-args -Wno-unused-variable -I "$W/$s" $Z "$D/n6.c" -o "$W/$s/n6" 2> "$W/$s/cc.txt" || { cat "$W/$s/cc.txt"; exit 2; }
  "$W/$s/n6" > "$W/$s/aus.txt" || exit 2
done
na=$(grep '^N_AUFRUFE' "$W/alt/aus.txt"); nn=$(grep '^N_AUFRUFE' "$W/neu/aus.txt")
zeilen=$(grep -c . "$W/alt/aus.txt")
fehl=$(diff "$W/alt/aus.txt" "$W/neu/aus.txt" | grep -c '^>')
echo "=== N6${SAB:+, Sabotage $SAB} ==="
echo "  Logaufrufe: alt ${na#N_AUFRUFE }, neu ${nn#N_AUFRUFE }; Vergleichszeilen $zeilen (7 Wertesaetze x Aufrufe + 33 Laengen x 3)"
rc=0
[ "$na" = "$nn" ] || { echo "  FEHL  Zahl der Logaufrufe verschieden"; rc=1; }
if [ "$fehl" -gt 0 ]; then
  diff "$W/alt/aus.txt" "$W/neu/aus.txt" | grep -E '^[<>]' | head -6 | cut -c1-200 | sed 's/^/  FEHL  /'
  rc=1
fi
echo "n6: $zeilen Vergleiche, $fehl FEHL -- $([ $rc = 0 ] && echo OK || echo FEHL)"
exit $rc
