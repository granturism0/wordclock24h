#!/usr/bin/env bash
# Gegenproben fuer die beiden PreToolUse-Hooks (W.5, AKW.3 und AKW.4).
#
# WARUM SIE IM REPO LIEGEN UND NICHT EINMAL VON HAND GEFAHREN WERDEN
#
# Die Hooks setzen R3 (jede Datei hat einen Schreiber) und R5 (die Uhr laeuft
# produktiv) durch. Beide bestehen aus Mustern, und ein Muster laesst sich in zwei
# Richtungen falsch denken:
#
#   zu eng  -- die Gefahr laeuft durch, und niemand merkt es
#   zu weit -- ein Fehlalarm, und dann wird der Hook umgangen statt befolgt
#
# Die zweite Richtung ist die unterschaetzte. In CLAUDE.md steht sie unter R3 bereits
# beschrieben (ein Entwurf von file-ownership.py blockierte die eigene Dokumentation),
# und am 05.10.2026 ist sie beim zweiten Hook erneut aufgetreten: Die Pruefung auf den
# blossen Endpunktnamen wies `echo "test_display ausgelassen"` ab -- ausgerechnet dem
# pwa-tester, der ausgelassene Schritte laut DIR-012 berichten SOLL.
#
# Deshalb prueft jede Probe hier BEIDE Richtungen. Eine Hookpruefung, die nur zeigt,
# dass die Gefahr abgewiesen wird, ist halb.
cd "$(git rev-parse --show-toplevel)" || exit 2
ok=0; schlecht=0

jsn() { python3 -c "
import json,sys
print(json.dumps({'tool_name': sys.argv[1], 'tool_input':
    ({'command': sys.argv[2]} if sys.argv[1] == 'Bash' else {'file_path': sys.argv[2]})}))" "$1" "$2"; }

probe() { # beschreibung, hook, agent-arg, tool, nutzlast, erwartung
  local args=""
  [ -n "$3" ] && args="--agent $3"
  jsn "$4" "$5" | python3 "tools/hooks/$2" $args >/dev/null 2>&1
  # Exit-Code SOFORT sichern. Der erste Entwurf hatte hier `local ist; [ $? -ne 0 ]`
  # -- und `local` setzt $? auf 0. Alle acht deny-Proben meldeten daraufhin "allow",
  # und das Skript behauptete, beide Hooks seien wirkungslos. Sie waren es nicht;
  # die einzeln gefahrene Probe davor hatte sie korrekt abweisen sehen.
  #
  # Ein Pruefskript, das aus einem eigenen Fehler auf einen fremden schliesst, ist
  # schlimmer als keines: Es haette hier beinahe dazu gefuehrt, an funktionierenden
  # Sicherheitshaken herumzubauen.
  local code=$?
  local ist; [ $code -ne 0 ] && ist=deny || ist=allow
  if [ "$ist" = "$6" ]; then printf '  OK    %-46s %s\n' "$1" "$ist"; ok=$((ok+1))
  else printf '  FEHLT %-46s %s, erwartet %s\n' "$1" "$ist" "$6"; schlecht=$((schlecht+1)); fi
}

echo "  --- file-ownership.py (AKW.3): eigene Datei, fremde Datei, Bash-Umweg, Lesen"
probe "eigene Datei schreiben"        file-ownership.py stm-developer Edit "$(pwd)/src/main.c" allow
probe "fremde Datei schreiben"        file-ownership.py stm-developer Edit "$(pwd)/ESP8266/ESP-uclock/data/app/app.js" deny
probe "fremde Datei per Bash patchen" file-ownership.py stm-developer Bash \
      "python3 -c \"import io; io.open('ESP8266/ESP-uclock/data/app/app.js','w').write(x)\"" deny
probe "fremde Datei lesen"            file-ownership.py stm-developer Read "$(pwd)/ESP8266/ESP-uclock/data/app/app.js" allow

echo "  --- no-danger.py (AKW.4): Aufruf verboten, Erwaehnung erlaubt"
probe "lesender Endpunkt"             no-danger.py "" Bash 'curl -s http://1.2.3.4/api/update_status' allow
probe "test_display aufrufen"         no-danger.py "" Bash 'curl -s http://1.2.3.4/api/test_display' deny
probe "learn_ir aufrufen"             no-danger.py "" Bash 'curl -s http://1.2.3.4/api/learn_ir' deny
probe "fs_remove aufrufen"            no-danger.py "" Bash 'curl -s "http://1.2.3.4/api/fs_remove?filename=app.js.gz"' deny
probe "ir_code_set aufrufen"          no-danger.py "" Bash 'curl -s "http://1.2.3.4/api/ir_code_set?idx=0"' deny
probe "EEPROM-Reset aufrufen"         no-danger.py "" Bash 'curl -s http://1.2.3.4/api/maintenance_reset_eeprom' deny
probe "Parameter ohne Gleichheits="   no-danger.py "" Bash 'curl -s "http://1.2.3.4/?a"' deny
probe "Endpunkt im Bericht erwaehnen" no-danger.py "" Bash 'echo "Schritt test_display ausgelassen (DIR-012)"' allow
probe "Endpunkt in Doku schreiben"    no-danger.py "" Bash 'python3 -c "open(\"BEFUNDE.md\",\"a\").write(\"learn_ir ist gefaehrlich\")"' allow

printf '      %d Proben bestanden, %d fehlgeschlagen\n' "$ok" "$schlecht"
[ "$schlecht" -eq 0 ] || exit 1
