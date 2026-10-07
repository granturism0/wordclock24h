#!/usr/bin/env bash
# Legt einen Rohabzug des Geraetezustands ab (Phase 0/S2 und Phase 9 aus TESTPLAN-PWA.md).
#
#   ./tools/snapshot-device.sh                 Vergleichsabzug, Zeitstempel als Name
#   ./tools/snapshot-device.sh referenz        Vergleichsabzug unter festem Namen
#   ./tools/snapshot-device.sh --restore       WIEDERHERSTELLBARER Abzug, verschluesselt
#   ./tools/snapshot-device.sh --oeffnen <d>   verschluesselten Abzug wieder auspacken
#
# ZWEI ARTEN VON ABZUG, und der Unterschied ist wesentlich:
#
#   Vergleichsabzug (Vorgabe)  Schluessel nur als Pruefsumme. Taugt zum PRUEFEN, ob
#                              sich etwas geaendert hat -- NICHT zum Zurueckholen.
#                              Darf liegen bleiben, enthaelt keine Geheimnisse.
#
#   --restore                  Schluessel im Klartext, aber das Ganze verschluesselt
#                              (AES-256, PBKDF2). Taugt zum ZURUECKHOLEN. Ohne
#                              Passwort nicht lesbar, auch nicht von mir.
#
# Ein gehashter Schluessel laesst sich nicht wiederherstellen. Wer nur
# Vergleichsabzuege hat, hat kein vollstaendiges Backup.
#
# Warum nicht einfach das PWA-Backup: Das Backup ist in diesem Testplan selbst
# Pruefgegenstand. Eine Sicherung, die von der zu pruefenden Funktion abhaengt, ist
# keine Sicherung. Dieser Abzug fragt die Endpunkte direkt ab und ist davon unabhaengig.
#
# AUSSCHLIESSLICH LESEND (DIR-008). Kein Endpunkt hier hat eine Nebenwirkung.
#
# Das Konzept dahinter steht in TESTPLAN-PWA.md, Kapitel 2b.
#
# Zugangsdaten: /api/eeprom_settings liefert den WLAN-Schluessel IM KLARTEXT an jeden,
# der im LAN fragt. Dieser Abzug legt ihn NICHT ab, sondern nur eine Pruefsumme. Damit
# erkennt der Vergleich trotzdem, ob sich der Wert geaendert hat -- ohne dass das
# Passwort in einer Datei auf der Platte landet. Mit --mit-geheimnissen laesst sich das
# abschalten; dann gehoert der Abzug geloescht, sobald er nicht mehr gebraucht wird.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

[ -f tools/device.conf ] && . tools/device.conf
HOST=${DEVICE_HOST:-}
if [ -z "$HOST" ]; then
  echo "DEVICE_HOST fehlt. tools/device.conf anlegen (Vorlage: tools/device.conf.example)." >&2
  exit 2
fi
U="http://$HOST"
TIMEOUT=${TIMEOUT:-15}

# Woher das Passwort fuer --restore und --oeffnen kommt (B19, Entscheid des Nutzers vom
# 07.10.2026). Reihenfolge:
#
#   1. SNAPSHOT_PASS in der Umgebung
#   2. die Datei SNAPSHOT_PASS_FILE, Vorgabe ~/.config/wordclock/snapshot.pass
#   3. Abfrage im Terminal
#
# Weg 2 ist der, ueber den ein Agent Phase 0 selbst fahren darf: Das Passwort steht in
# keiner Sitzung, nur der Pfad. Die Datei muss AUSSERHALB des Repos liegen und darf nur
# fuer den Eigentuemer lesbar sein -- sonst Abbruch, nicht stiller Rueckfall auf das
# Terminal. Ein Rueckfall wuerde einen Konfigurationsfehler als "fragt halt nach"
# verkleiden.
#
# openssl bekommt das Passwort ueber env: bzw. file:, nicht mehr ueber pass:. Mit pass:
# stand es waehrend des Laufs in der Prozessliste, lesbar fuer jeden Nutzer der Maschine.
PASSARG=()
PASSQUELLE="Terminal"
PASSFILE=${SNAPSHOT_PASS_FILE:-$HOME/.config/wordclock/snapshot.pass}
if [ -n "${SNAPSHOT_PASS:-}" ]; then
  export SNAPSHOT_PASS
  PASSARG=(-pass env:SNAPSHOT_PASS)
  PASSQUELLE="SNAPSHOT_PASS"
elif [ -e "$PASSFILE" ]; then
  REPO=$(pwd -P)
  ABS=$(cd "$(dirname "$PASSFILE")" && pwd -P)/$(basename "$PASSFILE")
  case "$ABS" in
    "$REPO"/*) echo "Abbruch: Die Passwortdatei liegt im Repo ($ABS). Sie gehoert ausserhalb." >&2; exit 2 ;;
  esac
  MODUS=$(stat -f %Lp "$ABS" 2>/dev/null || stat -c %a "$ABS" 2>/dev/null)
  case "$MODUS" in
    ?00|?00?) : ;;
    *) echo "Abbruch: $ABS ist fuer andere lesbar (Modus $MODUS). Erwartet: chmod 600." >&2; exit 2 ;;
  esac
  [ -s "$ABS" ] || { echo "Abbruch: $ABS ist leer." >&2; exit 2; }
  PASSARG=(-pass "file:$ABS")
  PASSQUELLE="Datei $ABS"
fi

# --oeffnen zuerst: braucht kein Geraet.
if [ "${1:-}" = "--oeffnen" ]; then
  ENC=${2:-}
  [ -f "$ENC" ] || { echo "Datei fehlt: ${ENC:-<keine angegeben>}" >&2; exit 2; }
  TARGET="${ENC%.enc}"
  TARGET="${TARGET%.tar.gz}-entpackt"
  mkdir -p "$TARGET"
  if openssl enc -d -aes-256-cbc -pbkdf2 -iter 200000 -in "$ENC" \
       ${PASSARG[@]+"${PASSARG[@]}"} | tar xzf - -C "$TARGET"; then
    echo "Entpackt nach: $TARGET"
    echo "ACHTUNG: enthaelt Zugangsdaten im Klartext. Nach Gebrauch loeschen."
    exit 0
  fi
  rmdir "$TARGET" 2>/dev/null
  echo "Entschluesseln fehlgeschlagen — falsches Passwort oder beschaedigte Datei." >&2
  exit 1
fi

SECRETS=0
if [ "${1:-}" = "--restore" ]; then
  SECRETS=1
  shift

  # VOR dem ersten Abruf pruefen, ob ein Passwort ueberhaupt zu bekommen ist.
  #
  # Der Abzug schreibt eeprom_settings mit dem WLAN-Schluessel zuerst im Klartext hin
  # und verschluesselt erst am Ende. Ohne SNAPSHOT_PASS und ohne Terminal kann openssl
  # kein Passwort lesen, scheitert -- und das Klartextverzeichnis bleibt liegen. Der
  # pwa-tester hat das am 05.10.2026 am Quelltext erkannt und das Skript deshalb gar
  # nicht erst aufgerufen. Ein Agent, der Phase 0 woertlich nimmt, haette den
  # Schluessel auf die Platte gelegt.
  #
  # /dev/tty statt [ -t 0 ]: openssl liest das Passwort vom Terminal, nicht von stdin.
  if [ "${#PASSARG[@]}" -eq 0 ] && ! { : < /dev/tty; } 2>/dev/null; then
    echo "Abbruch vor dem ersten Abruf: kein Passwort (SNAPSHOT_PASS oder $PASSFILE) und kein Terminal." >&2
    echo "Ohne Passwort wuerde der WLAN-Schluessel unverschluesselt liegen bleiben." >&2
    echo "Im eigenen Terminal ausfuehren, dort wird das Passwort abgefragt." >&2
    exit 2
  fi
  echo "Passwort aus: $PASSQUELLE"
fi

NAME=${1:-$(date '+%Y-%m-%d-%H%M%S')}
OUT="tools/snapshots/$NAME"

# Nur Endpunkte ohne Nebenwirkung. Die Liste ist von Hand gepflegt, nicht aus app.js
# erzeugt: Dort stehen auch alle schreibenden, und eine Automatik, die sich vertut,
# loest hier echte Aktionen aus.
READ_ONLY="device_ready settings_xml display_power ambilight_power power_status
           update_status update_table_files fs_info fs_list overlay_icons stm32_log"

mkdir -p "$OUT"

code=$(curl -s -m "$TIMEOUT" -o /dev/null -w '%{http_code}' "$U/api/device_ready" 2>/dev/null)
if [ "$code" != "200" ]; then
  echo "  ABBRUCH: Geraet antwortet nicht (HTTP ${code:-keine Antwort})" >&2
  rmdir "$OUT" 2>/dev/null
  exit 1
fi

printf '=== Rohabzug von %s nach %s ===\n\n' "$HOST" "$OUT"

for e in $READ_ONLY; do
  curl -s -m "$TIMEOUT" -o "$OUT/$e.txt" "$U/api/$e" 2>/dev/null
  printf '  %-22s %7s Byte\n' "$e" "$(wc -c < "$OUT/$e.txt" | tr -d ' ')"
done

# eeprom_settings gesondert: Schluessel ersetzen, bevor irgendetwas geschrieben wird.
raw=$(curl -s -m "$TIMEOUT" "$U/api/eeprom_settings" 2>/dev/null)
if [ "$SECRETS" -eq 1 ]; then
  printf '%s' "$raw" > "$OUT/eeprom_settings.txt"
  printf '  %-22s %7s Byte  mit Zugangsdaten (wird verschluesselt)\n' "eeprom_settings" "$(wc -c < "$OUT/eeprom_settings.txt" | tr -d ' ')"
else
  printf '%s' "$raw" | python3 -c '
import sys, json, hashlib
try:
    d = json.load(sys.stdin)
except Exception:
    sys.stdout.write("{}")
    sys.exit(0)
for field in ("key", "ap_key"):
    if d.get(field):
        # Pruefsumme statt Wert: der Vergleich erkennt eine Aenderung, das Passwort
        # steht trotzdem nirgends.
        d[field] = "sha256:" + hashlib.sha256(str(d[field]).encode()).hexdigest()[:16]
json.dump(d, sys.stdout, ensure_ascii=False, sort_keys=True, indent=2)
' > "$OUT/eeprom_settings.txt"
  printf '  %-22s %7s Byte  (Schluessel als Pruefsumme)\n' "eeprom_settings" "$(wc -c < "$OUT/eeprom_settings.txt" | tr -d ' ')"
fi

{
  printf 'host=%s\n' "$HOST"
  printf 'zeitpunkt=%s\n' "$(date '+%Y-%m-%d %H:%M:%S %z')"
  printf 'commit=%s\n' "$(git rev-parse --short HEAD)"
  printf 'arbeitsbaum=%s\n' "$(git diff --quiet HEAD 2>/dev/null && echo sauber || echo geaendert)"
  printf 'geheimnisse=%s\n' "$([ "$SECRETS" -eq 1 ] && echo enthalten || echo maskiert)"
} > "$OUT/_meta.txt"

# Bei --restore wird das Verzeichnis verschluesselt und das Klartext-Original
# geloescht. Zwischen Schreiben und Loeschen liegt ein kurzer Moment, in dem die
# Zugangsdaten auf der Platte stehen -- deshalb passiert das sofort und nicht erst
# am Ende eines laengeren Ablaufs.
if [ "$SECRETS" -eq 1 ]; then
  ENC="tools/snapshots/$NAME.tar.gz.enc"
  if tar czf - -C tools/snapshots "$NAME" \
     | openssl enc -aes-256-cbc -pbkdf2 -iter 200000 -salt -out "$ENC" \
         ${PASSARG[@]+"${PASSARG[@]}"}; then
    rm -rf "$OUT"
    echo
    printf '  Verschluesselt: %s  (%s Byte)\n' "$ENC" "$(wc -c < "$ENC" | tr -d ' ')"
    echo "  Klartext-Verzeichnis geloescht."
    echo
    echo "  Oeffnen:  ./tools/snapshot-device.sh --oeffnen $ENC"
    echo
    echo "  Ohne dieses Passwort ist der Abzug verloren. Es steht nirgends im Repo"
    echo "  und auch nicht in der Sitzung — es gibt keinen Weg, es zu rekonstruieren."
    exit 0
  fi
  echo "  ABBRUCH: Verschluesseln fehlgeschlagen. Klartext-Verzeichnis bleibt:" >&2
  echo "  $OUT  — von Hand loeschen." >&2
  exit 1
fi

echo
echo "  Vergleichen:  ./tools/diff-snapshot.sh $NAME <anderer>"
echo "  Hinweis: Schluessel sind hier nur Pruefsummen. Fuer eine WIEDERHERSTELLBARE"
echo "           Sicherung:  ./tools/snapshot-device.sh --restore"
