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
  "A59 LDR-Grenzen per Zahl setzbar (STM)|stm/l59/run.sh $S"
  "A62 Netzzeit mit Aera, Totbereich, Bestaetigung, Zeitzone (STM)|stm/a62/run.sh $S"
  "A69 Nachtfenster beim Start, beim ersten TIME, bei Spruengen (STM)|stm/a69/run.sh $S"
  "A60 I2C-Fristen eng gepollt, BUSY mit Frist, Logzeile (STM)|stm/a60/run.sh $S"
  "N5 snprintf/sprintf als Huellen, Ausgabe gleich (STM)|stm/n5/run.sh $S"
  "N6 DFPlayer-Praefix einmal, Ausgabe byte-gleich (STM)|stm/n6/run.sh $S|nurneu"
  "A23 Displayfarbe nach Farbanimation zurueck (STM)|stm/a23/run.sh $S"
  "A53 Spiele zaehlen verworfene ESP-Zeilen, Abgleich (STM)|stm/a53/run.sh $S"
  "A57 Messwert uebernehmen weist Inversion ab (STM)|stm/a57/run.sh $S"
  "A67 OTA start setzt offline, CAP unberuehrt (STM)|stm/a67/run.sh $S"
  "L408 Dimmtabellen 16 Elemente, offline keine Sendung (STM)|stm/l408/run.sh $S"
  "A15 UART-ISR mit einem SR-Lesen, Verhalten gleich (STM)|stm/a15/run.sh $S"
  "B55 RPCR nach Messwert uebernehmen (STM)|stm/l55/run.sh $S"
  "A56 Ambilight-Power im EEPROM, rueckflashsicher (STM)|stm/a56/run.sh $S"
  "A81 Marker-Weissfarbe vom eigenen Platz, geklemmt (STM)|stm/a81/run.sh $S"
  "A80 Icon bei Display aus verworfen, Restore-Bedingung unveraendert (STM)|stm/a80/run.sh $S"
  "A82 DCF77-Merker folgt Display und Ambilight (STM)|stm/a82/run.sh $S"
  "A70 Randfaelle Nachtfenster: Frist nach IP, erstes TIME, fremder Ticker, DCF77 (STM)|stm/a70/run.sh $S"
  "A2 DS18xx CRC, ROM-ID, Neuerkennung (STM)|stm/ds/run.sh $S"
  "A50 fault_reset ohne Ausgabe, 15 s geeicht (STM)|stm/fr/run.sh $S"
  "A55 CAP setzt offline bis IPADDRESS (STM+ESP)|stm/ep/run.sh $S $E"
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
  "C47 Legacy-Werte maskiert, Release Notes ohne Tags (ESP)|esp/t16/run.sh $E neu"
  "C47 jede Legacy-Wertausgabe eingeordnet, statisch (ESP)|esp/t16/static.sh $E neu"
  "C55/C44 Datumsformat und Koordinaten, API und Legacy (ESP)|esp/t17/run.sh $E neu"
  "A2 Fehlerwert 255 bei Korrekturaenderung unangetastet (ESP)|esp/t18/run.sh $E neu"
  "A35 still verworfene Zuweisungen gemeldet (ESP)|esp/a35/run.sh $E neu"
  "C58 NTP: Laenge vor dem Lesen, Absender, LI/Stratum/Transmit (ESP)|esp/ntp/run.sh $E neu"
  "A21 ovr= zaehlt UART-Ueberlaeufe saettigend (ESP)|esp/ovr/run.sh $E neu"
  "A76 ovr-Zeitpunkt: erste Zeile sofort, 1/min, Heap-Zeile nach der Abfrage (ESP)|esp/ovrzeit/run.sh $E neu"
  "A67 OTA start in allen drei OTA-Pfaden, statisch (ESP)|esp/akk2/static.sh $E neu"
  "E.6 Rueckfall-Neustart nach lokalem Upload (ESP)|esp/otarueck/run.sh $E neu"
  "C46 Query ueber 255 bei /api/ abgewiesen, GET und POST (ESP)|esp/c46/run.sh $E neu"
  "C19 Legacy-Update-Quelle an beiden Stellen (ESP)|esp/c19/run.sh $E neu"
  "B55 ldr_min/max_set warten auf RPCR, Bruecke liest weiter, Kennungen 10/7/11 (ESP)|esp/l55e/run.sh $E neu"
  "C8 Kommandopuffer 260, Folgeeintraege im Ring (ESP)|esp/c8/run.sh $E neu"
  "A79 setRxBufferSize (1024) vor Serial.begin, statisch (ESP)|esp/a79/static.sh $E neu"
  "C6b Schluessel nur ueber network_secrets, no-store, keine CORS, statisch (ESP)|esp/c6b/static.sh $E neu"
  "C55/C44 PWA entscheidet wie der ESP (PWA)|pwa/regeln/run.sh $E neu"
  "C35 Hauptmodul holt update_status und update_table_files hoechstens alle 5 min (PWA)|pwa/c35/run.sh $E neu"
  "L411 Start in jedem Modul ohne ReferenceError (PWA)|pwa/tdz/run.sh $E neu"
  "C6b/C46/B55 Schluessel ueber network_secrets, Query-Grenze, LDR-Meldungen, Mock (PWA)|pwa/ep/run.sh $E neu"
  "A61 Auswerteregel Vollabgleich (Werkzeug)|werkzeug/abgleich/run.sh $ROOT/tools/abgleich.sh|nurneu"
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
