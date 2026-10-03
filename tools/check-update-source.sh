#!/usr/bin/env bash
# Prueft, ob die Uhr auf GENAU DEN Pfad zeigt, auf den wir ausrollen.
#
#   ./tools/check-update-source.sh          prueft und meldet
#   ./tools/check-update-source.sh --quiet  nur Rueckgabewert, keine Ausgabe
#
# Rueckgabe: 0 = stimmt, 1 = weicht ab, 2 = Konfiguration oder Geraet fehlt.
#
# WARUM ES DIESES SKRIPT GIBT
#
# Am 03.10.2026 stand update_path am Geraet auf "test7" statt "test8". Auf test7 liegt
# eine Firmware 3.2.4 -- ein Stand von vor dem Watchdog-Fix. flash-stm.sh prueft die
# Quelle nur darauf, ob sie NICHT LEER ist; ein falscher Pfad passiert das anstandslos.
# Das Skript holte die alte Version, meldete sie als gueltiges Ziel und flashte sie.
#
# Die Uhr lief danach auf einer Firmware von vor saemtlichen Korrekturen des Tages, und
# der Rollout, der kurz davor nach test8 gegangen war, lag unberuehrt daneben.
#
# Der Nutzer hat den Punkt praeziser gefasst, als der Befund ihn hatte: Wer nach test8
# ausrollt, muss vor JEDEM Update pruefen, ob das Geraet auch dorthin schaut. Nicht
# "ist etwas eingestellt", sondern "ist DAS eingestellt, was ich gerade gebaut habe".
#
# Deshalb haengt diese Pruefung an beiden Enden:
#   deploy.sh    nach dem Rollout  -- dann faellt es sofort auf, nicht erst beim Flash
#   flash-stm.sh vor dem Flash     -- dort traegt es die Folgen
#   flash-esp.sh vor dem OTA       -- dito
#
# AUSSCHLIESSLICH LESEND (DIR-008).

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

QUIET=0
[ "${1:-}" = "--quiet" ] && QUIET=1
say() { [ "$QUIET" -eq 1 ] || printf '%s\n' "$*"; }

[ -f tools/device.conf ] && . tools/device.conf
[ -f tools/deploy.conf ] && . tools/deploy.conf

HOST=${DEVICE_HOST:-}
SOLL_HOST=${DEVICE_UPDATE_HOST:-}
SOLL_PFAD=${DEVICE_UPDATE_PATH:-}

if [ -z "$HOST" ] || [ -z "$SOLL_HOST" ] || [ -z "$SOLL_PFAD" ]; then
  say "  Update-Quelle: nicht pruefbar — DEVICE_HOST, DEVICE_UPDATE_HOST oder"
  say "                 DEVICE_UPDATE_PATH fehlt in tools/device.conf."
  exit 2
fi

# Der Rollout schreibt nach DEPLOY_PATH, die Uhr liest aus DEVICE_UPDATE_PATH. Laufen
# die beiden auseinander, rollt man an der Uhr vorbei aus -- ohne dass irgendwo ein
# Fehler erschiene. Das faellt sonst erst auf, wenn eine alte Firmware geflasht wurde.
if [ -n "${DEPLOY_PATH:-}" ]; then
  ziel=$(basename "$DEPLOY_PATH")
  if [ "$ziel" != "$SOLL_PFAD" ]; then
    say "  WARNUNG: Der Rollout geht nach '$ziel', die Uhr soll laut device.conf"
    say "           auf '$SOLL_PFAD' schauen. Eine der beiden Angaben ist falsch."
  fi
fi

# NICHT EINMAL FRAGEN, SONDERN WARTEN -- und das ist aus Schaden gelernt.
#
# Nach einem ESP-Neustart ist der Variablensatz kurzzeitig leer: Der ESP hat ihn
# verloren, der STM liefert ihn nach, und dazwischen liegen Sekunden. Wer in diesem
# Fenster EINMAL fragt, sieht "leer" und haelt es fuer den Endzustand.
#
# Am 03.10.2026 ist genau das passiert: Direkt nach einem OTA meldete dieses Skript
# "LEER am Geraet", woraufhin der STM zurueckgesetzt wurde -- ein Eingriff in eine
# produktiv laufende Uhr, der vermutlich unnoetig war. Der Nutzer sah in der
# Oberflaeche alle Werte und hat widersprochen. Nachgemessen: Der Nachlieferung
# reichen wenige Sekunden.
#
# Deshalb wird bis RETRIES mal nachgefragt, bevor "leer" als Befund gilt. Ein echter
# Verlust ueberlebt diese Wartezeit; eine Nachlieferung nicht. Der Unterschied ist
# genau das, was die Meldung behaupten soll.
RETRIES=${RETRIES:-6}
WAIT=${WAIT:-5}

ist_host=""; ist_pfad=""
versuch=0
while [ "$versuch" -lt "$RETRIES" ]; do
  versuch=$((versuch+1))
  sx=$(curl -s -m 15 "http://$HOST/api/settings_xml" 2>/dev/null)
  if [ -z "$sx" ]; then
    [ "$versuch" -ge "$RETRIES" ] && { say "  Update-Quelle: Geraet antwortet nicht."; exit 2; }
    sleep "$WAIT"; continue
  fi
  ist_host=$(printf '%s' "$sx" | grep -o '<strvar idx="9" value="[^"]*"'  | sed 's/.*value="//;s/"//')
  ist_pfad=$(printf '%s' "$sx" | grep -o '<strvar idx="10" value="[^"]*"' | sed 's/.*value="//;s/"//')
  [ -n "$ist_host" ] && [ -n "$ist_pfad" ] && break
  if [ "$versuch" -lt "$RETRIES" ]; then
    say "  Update-Quelle: noch leer, warte auf die Nachlieferung durch den STM ($versuch/$RETRIES) ..."
    sleep "$WAIT"
  fi
done

if [ -z "$ist_host" ] || [ -z "$ist_pfad" ]; then
  say "  Update-Quelle: LEER am Geraet — auch nach $((RETRIES * WAIT)) Sekunden Wartezeit."
  say "                 Der ESP faellt dann auf seine eingebaute Vorgabe zurueck, und die"
  say "                 zeigt auf den Server des URSPRUNGSPROJEKTS (BEFUNDE.md, L42)."
  say "                 Abhilfe: STM zuruecksetzen, dann sendet er den Satz neu."
  exit 1
fi

if [ "$ist_host" != "$SOLL_HOST" ] || [ "$ist_pfad" != "$SOLL_PFAD" ]; then
  say "  Update-Quelle: FALSCH."
  say "                 Geraet schaut auf  $ist_host/$ist_pfad"
  say "                 ausgerollt wird auf $SOLL_HOST/$SOLL_PFAD"
  say ""
  # Ein falscher Pfad ist gefaehrlicher als ein leerer: Dort liegt oft eine AELTERE,
  # lauffaehige Firmware. Der Flash gelingt, meldet Erfolg -- und die Uhr faellt um
  # Monate zurueck. Genau so ist am 03.10.2026 STM 3.2.4 auf das Geraet gekommen.
  v=$(curl -s -m 10 "http://$ist_host/$ist_pfad/wc.txt" 2>/dev/null | tr -d '\r\n')
  [ -n "$v" ] && say "                 Dort liegt STM-Version $v — die wuerde beim naechsten Flash kommen."
  say "                 Abhilfe: curl -s \"http://$HOST/api/update_path_set?value=$SOLL_PFAD\""
  exit 1
fi

say "  Update-Quelle: $ist_host/$ist_pfad — stimmt mit dem Rollout ueberein."
exit 0
