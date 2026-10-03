#!/usr/bin/env bash
# Schneidet einen app.js-Abruf auf Paketebene mit und wertet ihn aus (Messung M1 zu L125).
#
#   sudo ./tools/measure-appjs.sh            ein Abruf
#   sudo ./tools/measure-appjs.sh --runs 5   fuenf Abrufe nacheinander
#   sudo ./tools/measure-appjs.sh --keep     Mitschnitt behalten
#
# WARUM ES DIESES SKRIPT GIBT
#
# Der ESP stuerzt beim Ausliefern von app.js ab (L125): memcpy mit Ziel NULL im
# Systemtask, fuenf belegte Exceptions am 03.10.2026. Die Ursache liegt in SDK- oder
# lwIP-Code -- belegt ist, dass im ganzen ESP-Projekt kein Codepfad im Systemkontext
# laeuft. Aenderbar ist nur die Last, die dorthin fuehrt.
#
# Diese Messung beantwortet die eine Frage, die den Unterschied macht:
#
#   Hoert der ESP SCHLAGARTIG auf  -> Absturz mitten im Frame, nichts baut sich auf
#   Gehen vorher Pakete VERLOREN   -> etwas erschoepft sich, und das waere der erste
#                                     echte Beleg fuer die Speicher-Vermutung
#
# Beides sieht man von aussen nicht: curl meldet in beiden Faellen nur "Zeitlimit".
# Auf Paketebene sind es zwei voellig verschiedene Bilder.
#
# WARUM SUDO
#
# Das BPF-Geraet (/dev/bpf*) gehoert root. Ohne sudo kein Mitschnitt. Das Skript gibt
# den Mitschnitt anschliessend dem aufrufenden Benutzer, damit die Auswertung ohne
# Sonderrechte laeuft -- und damit Claude die Datei lesen kann.
#
# RISIKO, UND ES IST NICHT NULL
#
# Der Abruf selbst ist rein lesend (DIR-008) und genau das, was die PWA bei jedem
# Oeffnen tut. Aber nach heutiger Rate stuerzt der ESP bei rund einem von fuenf
# app.js-Abrufen ab, und jeder Absturz loescht den Variablensatz (L42/L103):
# HARDWARE_CONFIGURATION auf 65535 -- danach schlaegt jeder STM-Flash still fehl --
# und die Update-Quelle leer, womit das Geraet auf den Server des Ursprungsprojekts
# zurueckfaellt. Das Skript prueft beides hinterher und sagt es deutlich.
#
# Ein einzelner Abruf ist Normalbetrieb. --runs 5 ist es nicht.

set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2

RUNS=1
KEEP=0
while [ $# -gt 0 ]; do
  case "$1" in
    --runs) RUNS="${2:-1}"; shift 2 ;;
    --keep) KEEP=1; shift ;;
    *) echo "Unbekannte Option: $1" >&2; exit 2 ;;
  esac
done

[ -f tools/device.conf ] && . tools/device.conf
HOST=${DEVICE_HOST:-}
if [ -z "$HOST" ]; then
  echo "DEVICE_HOST fehlt. tools/device.conf anlegen (Vorlage: tools/device.conf.example)." >&2
  exit 2
fi

# Ohne root geht der Paketmitschnitt nicht -- aber alles andere schon. Das Skript
# bricht deshalb NICHT ab: Abbruchstelle, Zeiten und der Zustand der Uhr vorher und
# nachher sind auch ohne pcap belegbar, und die serielle Seite sieht die Exception
# ohnehin. Nur die Frage "schlagartig oder Aufbau" bleibt dann offen.
CAPTURE=1
if [ "$(id -u)" -ne 0 ]; then
  CAPTURE=0
  echo "  HINWEIS: ohne root kein Paketmitschnitt (/dev/bpf* gehoert root)."
  echo "           Abrufe, Zeiten und Geraetezustand werden trotzdem gemessen."
  echo "           Mit Mitschnitt:  sudo ./tools/measure-appjs.sh --runs N"
  echo
fi

# Wem gehoert die Sitzung? Der Mitschnitt soll ihm gehoeren, nicht root.
OWNER=${SUDO_USER:-$(whoami)}
IFACE=$(route -n get "$HOST" 2>/dev/null | awk '/interface:/{print $2}')
IFACE=${IFACE:-en0}

PCAP=${PCAP:-/tmp/appjs-$(date +%H%M%S).pcap}
URL="http://$HOST/app/app.js"

printf '=== app.js auf Paketebene — %s ===\n\n' "$(date '+%Y-%m-%d %H:%M:%S')"
printf '  Ziel        %s\n  Interface   %s\n  Abrufe      %d\n  Mitschnitt  %s\n\n' \
  "$URL" "$IFACE" "$RUNS" "$PCAP"

if [ "$RUNS" -gt 1 ]; then
  printf '  ACHTUNG: %d Abrufe sind nach heutiger Rate rund %d Absturzgelegenheiten.\n' \
    "$RUNS" "$(( (RUNS + 4) / 5 ))"
  printf '           Jeder Absturz loescht den Variablensatz der Uhr.\n\n'
fi

# ------------------------------------------------------------------ Vorzustand
# Damit hinterher feststeht, ob der Variablensatz wirklich verloren ging -- und nicht
# schon vorher leer war. Ohne diesen Vergleich ist die Aussage danach wertlos.
vor_hw=$(curl -s -m 10 "http://$HOST/api/settings_xml" 2>/dev/null \
         | grep -o '<var idx="[0-9]*" value="[0-9]*"' | head -1)
vor_pfad=$(curl -s -m 10 "http://$HOST/api/settings_xml" 2>/dev/null \
         | grep -o '<strvar idx="10" value="[^"]*"' | sed 's/.*value="//;s/"//')
printf '  Update-Pfad vorher: %s\n\n' "${vor_pfad:-(leer)}"

# -------------------------------------------------------------------- Mitschnitt
# -s 100 reicht: Wir zaehlen Pakete und lesen Kopfzeilen, die Nutzlast interessiert
# nicht. Das haelt die Datei klein und den Mitschnitt schnell genug fuer 365 Pakete/s.
if [ "$CAPTURE" -eq 1 ]; then
  tcpdump -i "$IFACE" -s 100 -w "$PCAP" "host $HOST and port 80" >/dev/null 2>&1 &
  TD=$!
  trap 'kill "$TD" 2>/dev/null' EXIT
  # tcpdump braucht einen Moment, bis es wirklich mitschneidet. Ohne diese Pause fehlen
  # die ersten Pakete -- und genau die tragen den Verbindungsaufbau.
  sleep 2
fi

echo "=== Abrufe ==="
treffer=0
for i in $(seq 1 "$RUNS"); do
  out=$(curl -s -m 30 -o /dev/null \
        -w '%{http_code} %{size_download} %{time_total}' "$URL" 2>/dev/null)
  code=$(echo "$out" | cut -d' ' -f1)
  byte=$(echo "$out" | cut -d' ' -f2)
  zeit=$(echo "$out" | cut -d' ' -f3)
  if [ "$code" = "200" ] && [ "$byte" -ge 100000 ] 2>/dev/null; then
    printf '  %2d/%d  ok        HTTP %s  %7s Byte  %6ss\n' "$i" "$RUNS" "$code" "$byte" "$zeit"
  else
    printf '  %2d/%d  ABBRUCH   HTTP %-3s  %7s Byte  %6ss  <-- hier\n' \
      "$i" "$RUNS" "${code:-0}" "$byte" "$zeit"
    treffer=$((treffer+1))
  fi
  [ "$i" -lt "$RUNS" ] && sleep 3
done

if [ "$CAPTURE" -eq 1 ]; then
  sleep 2
  kill "$TD" 2>/dev/null
  wait "$TD" 2>/dev/null
  trap - EXIT
  chown "$OWNER" "$PCAP" 2>/dev/null
fi

# -------------------------------------------------------------------- Auswertung
if [ "$CAPTURE" -eq 1 ]; then
echo
echo "=== Was im Mitschnitt steht ==="

vom_geraet=$(tcpdump -r "$PCAP" -nn "src host $HOST and port 80" 2>/dev/null | wc -l | tr -d ' ')
mit_last=$(tcpdump -r "$PCAP" -nn "src host $HOST and port 80 and greater 100" 2>/dev/null | wc -l | tr -d ' ')
zu_geraet=$(tcpdump -r "$PCAP" -nn "dst host $HOST and port 80" 2>/dev/null | wc -l | tr -d ' ')
rst=$(tcpdump -r "$PCAP" -nn "host $HOST and tcp[tcpflags] & tcp-rst != 0" 2>/dev/null | wc -l | tr -d ' ')
fin=$(tcpdump -r "$PCAP" -nn "host $HOST and tcp[tcpflags] & tcp-fin != 0" 2>/dev/null | wc -l | tr -d ' ')

printf '  Pakete vom Geraet       %6s   (davon mit Nutzlast: %s)\n' "$vom_geraet" "$mit_last"
printf '  Pakete zum Geraet       %6s\n' "$zu_geraet"
printf '  RST (Verbindungsabriss) %6s\n' "$rst"
printf '  FIN (sauberes Ende)     %6s\n' "$fin"

# Wiederholungen: dieselbe Sequenznummer mehrfach vom Geraet. Das ist der Kern der
# Frage -- Wiederholungen heissen, dass etwas verlorenging, BEVOR es aufhoerte.
wdh=$(tcpdump -r "$PCAP" -nn -S "src host $HOST and port 80" 2>/dev/null \
      | grep -oE 'seq [0-9]+' | sort | uniq -d | wc -l | tr -d ' ')
printf '  wiederholte Sequenzen   %6s\n' "$wdh"

echo
echo "=== Deutung ==="
if [ "$treffer" -eq 0 ]; then
  echo "  Alle Abrufe vollstaendig. Der Mitschnitt zeigt den Normalfall --"
  echo "  als Vergleichsbild brauchbar, aber die Frage beantwortet er nicht."
elif [ "$wdh" -gt 3 ]; then
  echo "  WIEDERHOLUNGEN VOR DEM ABBRUCH ($wdh Sequenzen mehrfach gesendet)."
  echo "  Das Geraet hat Pakete verloren und nachgesendet, BEVOR es aufhoerte."
  echo "  -> Es baut sich etwas auf. Das stuetzt die Erschoepfungs-Spur (L125)."
else
  echo "  KEIN Aufbau erkennbar ($wdh wiederholte Sequenzen)."
  echo "  Das Geraet hat mitten im Senden schlagartig aufgehoert."
  echo "  -> Passt zum Absturz im Systemtask, nicht zu einer langsamen Erschoepfung."
fi
[ "$rst" -eq 0 ] && [ "$treffer" -gt 0 ] && \
  echo "  Kein RST: Die Verbindung wurde nie geschlossen, es kam nur nichts mehr."
fi

# ----------------------------------------------------------------- Nachzustand
echo
echo "=== Hat die Uhr Schaden genommen? ==="
sleep 3
nach_pfad=$(curl -s -m 15 "http://$HOST/api/settings_xml" 2>/dev/null \
            | grep -o '<strvar idx="10" value="[^"]*"' | sed 's/.*value="//;s/"//')
if [ -z "$nach_pfad" ] && [ -n "$vor_pfad" ]; then
  echo "  VARIABLENSATZ VERLOREN — Update-Pfad war '$vor_pfad', ist jetzt leer."
  echo "  Folge: Das Geraet schaut auf den Server des Ursprungsprojekts, und ein"
  echo "         STM-Flash schlaegt still fehl (HARDWARE_CONFIGURATION = 65535)."
  echo "  Abhilfe: STM zuruecksetzen, dann sendet er den Satz neu."
elif [ -z "$nach_pfad" ]; then
  echo "  Update-Pfad leer — war er aber schon vorher."
else
  printf '  Update-Pfad unveraendert: %s\n' "$nach_pfad"
fi

echo
if [ "$CAPTURE" -eq 1 ]; then
  printf 'Mitschnitt: %s  (gehoert %s)\n' "$PCAP" "$OWNER"
  echo "Naechster Schritt: Claude den Pfad nennen, dann wertet er ihn im Detail aus."
else
  echo "Ohne Mitschnitt. Fuer die Paketebene mit sudo wiederholen."
fi
