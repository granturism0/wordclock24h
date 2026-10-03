#!/usr/bin/env bash
# Rollt das fertige Release auf die Synology aus (DIR-005).
#
#   ./tools/deploy.sh --dry-run     zeigt nur, was uebertragen wuerde
#   ./tools/deploy.sh               uebertraegt
#
# Konfiguration in tools/deploy.conf (nicht im Repo, siehe deploy.conf.example)
# oder per Umgebungsvariablen: DEPLOY_HOST DEPLOY_USER DEPLOY_PORT DEPLOY_PATH DEPLOY_KEY
#
# WICHTIG: Das Ziel ist zugleich der Update-Server, von dem die Uhr per OTA laedt.
# Ein unvollstaendiges oder kaputtes Release trifft dort nicht nur ein Archiv,
# sondern moeglicherweise ein Geraet mitten im Update. Deshalb prueft dieses
# Skript ALLE Artefakte, bevor es irgendetwas uebertraegt, und bricht sonst ab.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

[ -f tools/deploy.conf ] && . tools/deploy.conf
HOST=${DEPLOY_HOST:-}
USER=${DEPLOY_USER:-}
DEST=${DEPLOY_PATH:-/volume1/web/wordclock/test8}
KEY=${DEPLOY_KEY:-}
PORT=${DEPLOY_PORT:-22}
# Tilde aus der Konfigurationsdatei expandieren — in einer Variablen bleibt sie sonst
# woertlich stehen und ssh findet den Schluessel nicht ("Permission denied").
KEY=${KEY/#\~/$HOME}
DRY=""
[ "${1:-}" = "--dry-run" ] && DRY="--dry-run"

BUILD=build/stm-rgbw-12h
ESPB=build/esp8266
APP=ESP8266/ESP-uclock/data/app

fail() { printf '  ABBRUCH: %s\n' "$1" >&2; exit 1; }

if [ -z "$HOST" ] || [ -z "$USER" ]; then
  echo "Host oder Benutzer fehlt." >&2
  echo "tools/deploy.conf anlegen (Vorlage: tools/deploy.conf.example)" >&2
  echo "oder DEPLOY_HOST und DEPLOY_USER setzen." >&2
  exit 2
fi

# ---------------------------------------------------------------- Artefakte
echo "=== Artefakte pruefen ==="
ARTIFACTS="
$ESPB/app-version.txt
$BUILD/wc.txt
$ESPB/ESP-WordClock.txt
$BUILD/wc12h-stm32f103-sk6812-rgbw.hex
$BUILD/wc12h-stm32f411ce-25-sk6812-rgbw.hex
$ESPB/ESP-WordClock-4M.bin
"
for f in $ARTIFACTS; do
  [ -f "$f" ] || fail "fehlt: $f — erst 'make release-zip' ausfuehren"
  [ -s "$f" ] || fail "ist 0 Byte: $f"
  printf '  ok  %-52s %s Byte\n' "$(basename "$f")" "$(wc -c < "$f" | tr -d ' ')"
done

GZ_SRC="app.js styles.css index.html sw.js manifest.webmanifest layout-previews.json"
for b in $GZ_SRC; do
  [ -f "$APP/$b.gz" ] || fail "fehlt: $APP/$b.gz — erst 'make app-gz'"
  [ -s "$APP/$b.gz" ] || fail "ist 0 Byte: $b.gz — das ergaebe einen Weisschirm auf dem Geraet"
  [ "$APP/$b" -nt "$APP/$b.gz" ] && fail "$b.gz ist aelter als die Quelle — erst 'make app-gz'"
done
for i in icon-192.svg icon-512.svg icon-192.png icon-512.png icon-180.png icon-mask.png; do
  [ -s "$APP/icons/$i.gz" ] || fail "fehlt oder leer: icons/$i.gz"
done
echo "  ok  alle .gz vorhanden, nicht leer, nicht veraltet"

# --------------------------------------------------------------- Versionen
echo
echo "=== Versionen ==="
printf '  STM  %s\n' "$(cat "$BUILD/wc.txt")"
printf '  ESP  %s\n' "$(cat "$ESPB/ESP-WordClock.txt")"
printf '  App  %s\n' "$(cat "$ESPB/app-version.txt")"
printf '  Commit %s%s\n' "$(git rev-parse --short HEAD)" \
  "$(git diff --quiet HEAD 2>/dev/null || echo '  ACHTUNG: Arbeitsbaum nicht sauber')"

# ------------------------------------------------------------------ Rsync
SSH="ssh -p $PORT -o BatchMode=yes -o ConnectTimeout=10"
[ -n "$KEY" ] && SSH="$SSH -i $KEY -o IdentitiesOnly=yes"
TARGET="$USER@$HOST"

echo
echo "=== Ziel pruefen: $TARGET:$DEST (Port $PORT) ==="
$SSH "$TARGET" "[ -d '$DEST' ] && [ -w '$DEST' ]" \
  || fail "Zielverzeichnis fehlt oder ist nicht beschreibbar: $DEST"
echo "  ok  Verzeichnis vorhanden und beschreibbar"

STAGE=$(mktemp -d); trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/app/icons"
for f in $ARTIFACTS; do cp "$f" "$STAGE/"; done
for b in $GZ_SRC; do cp "$APP/$b.gz" "$STAGE/app/"; done
cp "$APP/icons/"*.gz "$STAGE/app/icons/"

# tar statt rsync: das openrsync auf macOS honoriert -e nicht, die SSH-Optionen
# kommen dort nicht an und die Anmeldung scheitert. tar ueber die SSH-Verbindung
# funktioniert zuverlaessig und hat dieselbe Semantik wie rsync ohne --delete:
# ueberschreibt gleichnamige Dateien, laesst alles andere unangetastet. Das ist
# wichtig, weil auf dem Ziel wc-list.txt, wc-list-tables.txt, die Layout-Tabellen
# und releasenote.html liegen, die der Nutzer selbst pflegt.
echo
# COPYFILE_DISABLE: macOS-tar packt sonst AppleDouble-Dateien (._name) mit ein.
# Die haetten auf dem Update-Server nichts verloren und wuerden dort liegen bleiben.
export COPYFILE_DISABLE=1
TAR_OPT="--no-xattrs"   # sonst meldet das tar auf der Synology jede macOS-Metadatei

# --no-overwrite-dir ist hier KEIN Feinschliff, sondern Pflicht: Das Archiv enthaelt
# den Eintrag "." mit den Rechten des Staging-Verzeichnisses, und mktemp -d legt das
# mit 0700 an. Ohne diese Option setzt tar damit das ZIELverzeichnis auf 0700 --
# Apache kann dann nicht mehr hineinlesen und beantwortet jede Anfrage mit
# "403 Forbidden ... unable to read htaccess file". Genau das ist am 01.10.2026
# passiert: Der Update-Server war danach fuer die Uhr nicht mehr erreichbar.
mkdir -p "$STAGE" && chmod 755 "$STAGE"

if [ -n "$DRY" ]; then
  echo "=== Probelauf: diese Dateien wuerden geschrieben ==="
  tar $TAR_OPT -cf - -C "$STAGE" . | $SSH "$TARGET" "tar tvf -" | sed 's/^/  /' \
    || fail "Uebertragung fehlgeschlagen"
else
  echo "=== Uebertragung ==="
  tar $TAR_OPT -cf - -C "$STAGE" . | $SSH "$TARGET" "tar xv --no-overwrite-dir -f - -C '$DEST'" | sed 's/^/  /' \
    || fail "Uebertragung fehlgeschlagen"
fi

# ------------------------------------------------------- Releasenote-Kopfzeile
# Die Datei wird auf der Synology gepflegt und ist nicht Teil des Repositories.
# Angepasst wird ausschliesslich die H2-Kopfzeile auf die aktuelle STM-Version,
# der uebrige Inhalt bleibt unangetastet.
STM_VER=$(cat "$BUILD/wc.txt")
RN="$DEST/releasenote.html"
echo
echo "=== Releasenote-Kopfzeile ==="
OLD_H2=$($SSH "$TARGET" "grep -o '<H2>Release Notes WordClock [^<]*</H2>' '$RN' 2>/dev/null" | head -1)
if [ -z "$OLD_H2" ]; then
  warn_rn="  keine passende H2-Zeile gefunden, uebersprungen"
  echo "$warn_rn"
else
  NEW_H2="<H2>Release Notes WordClock $STM_VER</H2>"
  printf '  vorher:  %s\n  nachher: %s\n' "$OLD_H2" "$NEW_H2"
  if [ "$OLD_H2" = "$NEW_H2" ]; then
    echo "  bereits aktuell, nichts zu tun"
  elif [ -n "$DRY" ]; then
    echo "  Probelauf: nicht geschrieben"
  else
    # ueber eine temporaere Datei, damit ein Abbruch die Releasenote nicht abschneidet
    $SSH "$TARGET" "sed 's|<H2>Release Notes WordClock [^<]*</H2>|$NEW_H2|' '$RN' > '$RN.tmp' \
        && mv '$RN.tmp' '$RN'" || fail "Releasenote konnte nicht aktualisiert werden"
    echo "  aktualisiert"
  fi
fi

echo
if [ -n "$DRY" ]; then
  echo "Probelauf beendet. Nichts geschrieben. Ohne --dry-run wird uebertragen."
else
  # Tag als Bezugsgroesse fuer die Versionspflicht (DIR-004). Die Guardrails messen
  # dagegen, nicht gegen den letzten Commit — ein Bump kann mehrere Commits zurueckliegen.
  APP_VER=$(cat "$ESPB/app-version.txt")
  ESP_VER=$(cat "$ESPB/ESP-WordClock.txt")
  TAG="release/$STM_VER-$ESP_VER-$APP_VER"
  if git rev-parse "$TAG" >/dev/null 2>&1; then
    echo "  Tag $TAG existiert bereits, nicht neu gesetzt"
  elif ! git diff --quiet HEAD 2>/dev/null; then
    echo "  Arbeitsbaum nicht sauber — Tag $TAG NICHT gesetzt."
    echo "  Nach dem Commit nachholen:  git tag -a $TAG -m 'Release $TAG'"
  else
    git tag -a "$TAG" -m "Release $TAG" && echo "  Tag gesetzt: $TAG"
  fi

  echo "Rollout abgeschlossen."
  echo
  # Sofort pruefen, ob das Geraet ueberhaupt hierher schaut. Sonst liegt das Fabrikat
  # richtig und wird trotzdem nie geholt -- und das faellt erst beim naechsten Flash
  # auf, wenn eine alte Firmware kommt (BEFUNDE.md, L124).
  ./tools/check-update-source.sh || echo "  ^ Der Rollout liegt bereit, die Uhr schaut woanders hin."
  echo
  echo "Hinweis: wc-list.txt und wc-list-tables.txt pflegst du selbst auf der Synology —"
  echo "der ESP erwartet sie dort (http.cpp:47-54), sie sind nicht Teil des Releases."
fi
