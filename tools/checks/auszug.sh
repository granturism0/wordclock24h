#!/usr/bin/env bash
# Auszugs-Pruefstaende: echter Firmware-Code, ausgeschnitten und auf dem Rechner
# uebersetzt -- nativ UND mit den Typbreiten des Ziels (pruefstand.h, L256).
#
#   ./tools/checks/auszug.sh            alle gegen den aktuellen Baum
#   ./tools/checks/auszug.sh --gegen R  alle gegen den Stand von Revision R (Gegenprobe)
#
# Entstanden in Runde S (Paket 2026-10-06, S.20), erweitert in Runde F (F.5h, sieben ESP-Pruefstaende) und in Paket 2026-10-09 (E.3, t12-t14). Jeder Pruefstand ist beim Bauen einmal
# gegen die alte Fassung fehlgeschlagen (DIR-014); --gegen macht das wiederholbar, z. B.
#   ./tools/checks/auszug.sh --gegen release/3.2.21-3.2.25-1.4.92
# muss fuer die STM-Pruefstaende anschlagen, weil dieser Stand die Korrekturen nicht hat.
#
# Die Pruefstaende schreiben Zwischendateien neben sich. Deshalb laufen sie in einer
# Kopie unter $TMPDIR, nicht im Arbeitsbaum.
#
# Ausgabe: je Pruefstand eine Zeile OK/FEHL, am Ende "N von M bestanden". Exit 1, wenn
# einer fehlschlaegt, 2, wenn einer sich nicht uebersetzen laesst.
set -uo pipefail
ROOT=$(git -C "$(dirname "$0")" rev-parse --show-toplevel) || exit 2
export ROOT
QUELLE="$ROOT"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/auszug.XXXXXX") || exit 2
trap 'rm -rf "$TMP"' EXIT
if [ "${1:-}" = "--gegen" ]; then
  REV=${2:?Revision fehlt}
  QUELLE="$TMP/baum"; mkdir -p "$QUELLE"
  git -C "$ROOT" archive "$REV" src ESP8266/ESP-uclock | tar -x -C "$QUELLE" || exit 2
  echo "Gegenprobe gegen $REV"
fi
cp -R "$ROOT/tools/checks/auszug" "$TMP/a"
A="$TMP/a"; S="$QUELLE/src"; E="$QUELLE/ESP8266/ESP-uclock"

# Name | Aufruf (ohne Kopie-Praefix). "neu" ist die Bezeichnung in der Ausgabe.
LAUF=(
  "AKS.5 Quittungszuordnung (STM)|stm/run.sh $S neu"
  "A13+A47 Abschlussbyte (STM)|stm/a13/run.sh $S neu"
  "A20 keine Werte im Timeout-Log (STM)|stm/a20/run.sh $S neu"
  "A5+L325 RTC-Temperatur (STM)|stm/a5/run.sh $S/rtc/rtc.c neu -DHAVE_HALF"
  "A3+A48+A51 Overlay-Grenzen (STM)|stm/a3/run.sh $S/main.c $S/overlay/overlay.c neu"
  "A14 Index maskiert (STM)|stm/k/a14/run.sh $S/vars/vars.c neu"
  "A24 Wortindizes (STM)|stm/k/a24/run.sh $S/tables/tables.c neu"
  "A46 Verwerfungspfade (STM)|stm/k/a46/run.sh $S/main.c $S/tftled/tftled.c $S/esp-spiffs/esp-spiffs.c $S/esp8266 neu"
  "A49 erste Meldung je Grund (STM)|stm/k/a49/run.sh $S/main.c neu"
  "A9 LDR ungeklammert, Kennlinie gleich (STM)|stm/k/a9/run.sh $S/ldr/ldr.c neu"
  "A2 Wetterwarten bis zur Endzeile, hoechstens 6 s (STM)|stm/w2/run.sh $S"
  "C3 EEPROM seitenweise, beide Seitengroessen (STM)|stm/c3/run.sh $S neu"
  "Teil D Pruefsumme CMC, Abweisung, Vormerken (STM)|stm/d/run.sh $S neu"
  "S0 my_gmtime 32 Bit gleich 64 Bit und gmtime_r (STM)|stm/gt/run.sh $S"
  "S.1-S.3 Bruecke (ESP)|esp/t1/run.sh $E/ESP-uclock.ino $E/vars.h neu"
  "C31 keine Schluessel im Log (ESP)|esp/t2/run.sh $E/eepromdata.cpp neu"
  "A5 Legacy-Temperatur (ESP)|esp/t3/run.sh $E neu"
  "N1 Overlay-Index, C6u Formularfelder (ESP)|esp/t4/run.sh $E neu"
  "C38 SSID und Schluessel abgewiesen, auch AP und Legacy (ESP)|esp/t5/run.sh $E neu"
  "C23 Logring als UTF-8-JSON (ESP)|esp/t6/run.sh $E neu"
  "E12 Flag-Setter ohne Parameter (ESP)|esp/t7/run.sh $E neu"
  "C25 Verlustzaehler getrennt (ESP)|esp/t8/run.sh $E neu"
  "C9k Kopf in zwei Segmenten (ESP)|esp/t9/run.sh $E neu"
  "Overlay-Text abgewiesen (ESP)|esp/t10/run.sh $E neu"
  "A2 Praefix uc-/wc beim STM-Dateinamen (ESP)|esp/t11/run.sh $E neu"
  "L338 Wetterabruf vollstaendig, 5-s-Netz (ESP)|esp/t12/run.sh $E neu"
  "Teil C Echo ohne Querystring (ESP)|esp/t13/run.sh $E neu|nurneu"
  "Teil C keine Anfragewerte ueber Serial, statisch (ESP)|esp/t13/static.sh $E neu"
  "Teil D ein Kommandoabsender CMD/CMC (ESP)|esp/t14/run.sh $E neu"
  "Teil D keine CMD-Zeile ausserhalb des Absenders, statisch (ESP)|esp/t14/static.sh $E neu"
  "C53 erste Anfragezeile nach FIN nicht verworfen (ESP)|esp/t15/run.sh $E neu"
)
# Drittes Feld "nurneu": Der Pruefstand ruft eine Funktion, die es vor der Korrektur nicht
# gab. Gegen eine alte Revision laesst er sich nicht uebersetzen, und das ist kein Befund --
# er wird dort ausgelassen und AUSDRUECKLICH genannt, nicht still und nicht als KAPUTT.
ok=0; fehl=0; kaputt=0; ausgelassen=0
for z in "${LAUF[@]}"; do
  name=${z%%|*}; cmd=${z#*|}; art=""
  case $cmd in *"|"*) art=${cmd##*|}; cmd=${cmd%|*} ;; esac
  if [ "$art" = "nurneu" ] && [ -n "${REV:-}" ]; then
    ausgelassen=$((ausgelassen+1)); printf '  AUSGELASSEN %s (gegen %s nicht uebersetzbar, zaehlt nicht)\n' "$name" "$REV"
    continue
  fi
  out=$(cd "$A" && sh "$A"/$cmd 2>&1); rc=$?
  case $rc in
    0) ok=$((ok+1)); printf '  OK    %s\n' "$name" ;;
    1) fehl=$((fehl+1)); printf '  FEHL  %s\n' "$name"; printf '%s\n' "$out" | grep -a -E "FEHL|Fail|FAIL" | head -3 | sed 's/^/          /' ;;
    *) kaputt=$((kaputt+1)); printf '  KAPUTT %s (rc=%s)\n' "$name" "$rc"; printf '%s\n' "$out" | tail -4 | sed 's/^/          /' ;;
  esac
done
echo "  $ok von ${#LAUF[@]} bestanden, $fehl fehlgeschlagen, $kaputt nicht uebersetzbar${ausgelassen:+, $ausgelassen ausgelassen}"
[ "$kaputt" -gt 0 ] && exit 2
[ "$fehl" -gt 0 ] && exit 1
exit 0
