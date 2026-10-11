#!/bin/sh
# Auswertung von "make matrix": Tabelle, Flash-Gate, Vergleich mit dem Original.
#
#   cmake/matrix-report.sh <build-dir> <min-frei-byte> <original-zip> <build-exitcode>
#
# Exit 0 nur, wenn
#   - die erwarteten 72 Varianten (unten unabhaengig von CMake aufgezaehlt) genau
#     die sind, die CMake definiert hat (matrix-manifest.txt),
#   - alle 72 eine nicht leere HEX haben,
#   - jede mindestens <min-frei-byte> Flash frei hat,
#   - und der Build selbst mit 0 endete.
# Der Vergleich mit dem Original ist Information, kein Gate. Fehlt das ZIP, bleibt
# die Spalte leer.
#
# Gezaehlt wird gegen die ERWARTETE Zahl 72, nicht gegen die Zahl der HEX-Dateien,
# die zufaellig entstanden sind (DIR-014).

BUILD_DIR=$1
MIN_FREE=${2:-1024}
ZIP=$3
BUILD_RC=${4:-0}
EXPECTED=72

HEX_DIR="$BUILD_DIR/hex"
MANIFEST="$BUILD_DIR/matrix-manifest.txt"
REPORT="$BUILD_DIR/matrix-report.txt"
WORK="$BUILD_DIR/report-tmp"

fail=0
err() { printf 'FEHLER: %s\n' "$*" >&2; fail=1; }

cache_value() {
	grep "^$1:" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null | head -n 1 | cut -d= -f2-
}
OBJCOPY=$(cache_value CMAKE_OBJCOPY)
SIZE=$(cache_value CMAKE_SIZE)
[ -x "$OBJCOPY" ] || { echo "FEHLER: objcopy nicht gefunden (CMakeCache)" >&2; exit 2; }
[ -x "$SIZE" ] || { echo "FEHLER: size nicht gefunden (CMakeCache)" >&2; exit 2; }
[ -f "$MANIFEST" ] || { echo "FEHLER: $MANIFEST fehlt -- Matrix nicht konfiguriert?" >&2; exit 2; }

rm -rf "$WORK"
mkdir -p "$WORK"

# --- erwartete Varianten, unabhaengig von CMake ----------------------------
: > "$WORK/expected"
for mode in 12h 24h; do
	for board in stm32f103 stm32f401 stm32f401cc-25 stm32f401cc-8 stm32f411 stm32f411ce-25 stm32f411ce-8; do
		for led in apa102 sk6812-rgb sk6812-rgbw ws2812-grb ws2812-rgb; do
			echo "wc$mode-$board-$led" >> "$WORK/expected"
		done
	done
	echo "wc$mode-stm32f407-tft-rgb" >> "$WORK/expected"
done
sort -o "$WORK/expected" "$WORK/expected"
n_expected=$(wc -l < "$WORK/expected" | tr -d ' ')
[ "$n_expected" -eq "$EXPECTED" ] || err "Erwartungsliste hat $n_expected Eintraege statt $EXPECTED"

awk '{print $1}' "$MANIFEST" | sort > "$WORK/manifest"
if ! cmp -s "$WORK/expected" "$WORK/manifest"; then
	err "CMake definiert andere Varianten als erwartet:"
	diff "$WORK/expected" "$WORK/manifest" | grep '^[<>]' | sed 's/^</  fehlt in CMake:/; s/^>/  nur in CMake: /' >&2
fi

have_zip=0
if [ -n "$ZIP" ] && [ -f "$ZIP" ]; then
	have_zip=1
	unzip -Z1 "$ZIP" | grep '\.hex$' | sed 's/\.hex$//' | sort > "$WORK/zip"
	if ! cmp -s "$WORK/expected" "$WORK/zip"; then
		err "Original-ZIP enthaelt andere Varianten als erwartet:"
		diff "$WORK/expected" "$WORK/zip" | grep '^[<>]' | sed 's/^</  fehlt im ZIP:  /; s/^>/  nur im ZIP:   /' >&2
	fi
fi

flash_total() {
	# LENGTH der FLASH-Region aus dem Linkerskript, in Byte
	sed -n 's/.*FLASH *(rx) *: *ORIGIN *= *0x[0-9A-Fa-f]*, *LENGTH *= *\([0-9]*[KkMm]\{0,1\}\).*/\1/p' "$1" | head -n 1 |
		awk '{ v=$1; m=1; if (v ~ /[Kk]$/) m=1024; if (v ~ /[Mm]$/) m=1048576; sub(/[KkMm]$/,"",v); print v*m }'
}

built=0
gated=0
missing=""
under=""
tightest=""
tightest_free=999999999

{
printf '%-32s %-6s %9s %9s %9s %10s %9s %s\n' "Variante" "gebaut" "belegt" "gesamt" "frei" "Original" "Diff" ""
printf '%-32s %-6s %9s %9s %9s %10s %9s %s\n' "--------" "------" "------" "------" "----" "--------" "----" ""
} > "$REPORT"

while read -r name; do
	ld=$(awk -v n="$name" '$1==n {print $2}' "$MANIFEST")
	total=""
	[ -n "$ld" ] && [ -f "$ld" ] && total=$(flash_total "$ld")
	hex="$HEX_DIR/$name.hex"
	# Die ELF traegt im Build-Verzeichnis keine Endung; .elf nur als Rueckfall.
	elf="$HEX_DIR/$name"
	[ -f "$elf" ] || elf="$HEX_DIR/$name.elf"
	orig=""; diffpct=""; mark=""
	if [ "$have_zip" -eq 1 ]; then
		unzip -p "$ZIP" "$name.hex" > "$WORK/orig.hex" 2>/dev/null
		if [ -s "$WORK/orig.hex" ] && "$OBJCOPY" -I ihex -O binary "$WORK/orig.hex" "$WORK/orig.bin" 2>/dev/null; then
			orig=$(wc -c < "$WORK/orig.bin" | tr -d ' ')
		fi
	fi
	if [ -s "$hex" ] && [ -s "$elf" ]; then
		built=$((built + 1))
		used=$("$SIZE" "$elf" | awk 'NR==2 {print $1 + $2}')
		# Gegenprobe ueber die HEX selbst: Sie ist, was geflasht wird.
		"$OBJCOPY" -I ihex -O binary "$hex" "$WORK/our.bin"
		binsize=$(wc -c < "$WORK/our.bin" | tr -d ' ')
		[ "$binsize" -gt "$used" ] && used=$binsize
		if [ -z "$total" ]; then
			err "$name: Flash-Gesamt aus '$ld' nicht lesbar"
			free="?"
		else
			free=$((total - used))
			if [ "$free" -lt "$MIN_FREE" ]; then
				gated=$((gated + 1))
				under="$under $name"
				mark="GATE"
			fi
			if [ "$free" -lt "$tightest_free" ]; then
				tightest_free=$free
				tightest=$name
			fi
		fi
		if [ -n "$orig" ]; then
			diffpct=$(awk -v a="$used" -v b="$orig" 'BEGIN { printf "%+.1f%%", (a-b)*100.0/b }')
			if awk -v a="$used" -v b="$orig" 'BEGIN { d=(a-b)/b; if (d<0) d=-d; exit !(d > 0.10) }'; then
				mark="$mark >10%"
			fi
		fi
		printf '%-32s %-6s %9s %9s %9s %10s %9s %s\n' "$name" "ja" "$used" "$total" "$free" "${orig:--}" "${diffpct:--}" "$mark" >> "$REPORT"
	else
		missing="$missing $name"
		printf '%-32s %-6s %9s %9s %9s %10s %9s %s\n' "$name" "nein" "-" "${total:--}" "-" "${orig:--}" "-" "FEHLT" >> "$REPORT"
	fi
done < "$WORK/expected"

cat "$REPORT"
echo
{
	echo "Flash-Gate: mindestens $MIN_FREE Byte frei je Variante."
	[ -n "$tightest" ] && echo "Engste Variante: $tightest mit $tightest_free Byte frei."
	echo "$built von $EXPECTED gebaut, $gated unter dem Gate."
} | tee -a "$REPORT"

if [ -n "$missing" ]; then
	err "nicht gebaut:$missing"
fi
if [ -n "$under" ]; then
	err "unter dem Gate ($MIN_FREE Byte frei):$under"
fi
if [ "$BUILD_RC" -ne 0 ]; then
	err "Build endete mit Exit $BUILD_RC (Protokoll: $BUILD_DIR/build.log)"
fi
[ "$built" -eq "$EXPECTED" ] || fail=1

rm -rf "$WORK"
if [ "$fail" -ne 0 ]; then
	echo "make matrix: FEHLGESCHLAGEN" >&2
	exit 1
fi
echo "make matrix: OK -- Tabelle auch in $REPORT"
exit 0
