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
# E20/L258 (08.10.2026): Nur die SCHREIBposition zaehlt, nicht jede Erwaehnung.
probe "fremde Datei als cp-Quelle"    file-ownership.py stm-developer Bash \
      "cp ESP8266/ESP-uclock/data/app/app.js /tmp/kopie.js" allow
probe "fremde Datei als cp-Ziel"      file-ownership.py stm-developer Bash \
      "cp /tmp/kopie.js ESP8266/ESP-uclock/data/app/app.js" deny
probe "Python: fremd lesen, eigen schreiben" file-ownership.py stm-developer Bash \
      "python3 - <<'PY'
q='ESP8266/ESP-uclock/data/app/app.js'
t=open(q).read()
p='src/main.c'
open(p,'w').write(t)
PY" allow
probe "Python: fremde Variable schreiben" file-ownership.py stm-developer Bash \
      "python3 - <<'PY'
p='ESP8266/ESP-uclock/data/app/app.js'
open(p,'w').write('x')
PY" deny

# Hardware-Team (17.10.2026): hardware/ hat je Unterverzeichnis genau einen Schreiber. Die
# .md-Muster von doc-writer und Lead muessen hardware/ ausschliessen, sonst haette eine Datei
# zwei Besitzer. Beide Richtungen: das Ausschliessen darf die alten .md-Rechte nicht brechen.
echo "  --- file-ownership.py: hardware/ (hw-designer, hw-datasheet-analyst, hw-architect, hw-reviewer)"
probe "hw-designer: eigener Generator"     file-ownership.py hw-designer Edit "$(pwd)/hardware/esp32s3/gen.py" allow
probe "hw-designer: fremdes Komponentenblatt" file-ownership.py hw-designer Edit "$(pwd)/hardware/komponenten/sy8301.md" deny
probe "hw-analyst: eigenes Komponentenblatt" file-ownership.py hw-datasheet-analyst Edit "$(pwd)/hardware/komponenten/sy8301.md" allow
probe "hw-analyst: fremder Schaltplan"     file-ownership.py hw-datasheet-analyst Edit "$(pwd)/hardware/esp32s3/WordClock.kicad_sch" deny
probe "hw-architect: eigener Entscheid"    file-ownership.py hw-architect Edit "$(pwd)/hardware/architektur/entscheidungen.md" allow
probe "hw-architect: Schaltplan per Bash"  file-ownership.py hw-architect Bash \
      "python3 -c \"open('hardware/esp32s3/gen.py','w').write('x')\"" deny
probe "hw-layouter: eigenes Layout"         file-ownership.py hw-layouter Edit "$(pwd)/hardware/esp32s3/WordClock-ESP32S3-V3.kicad_pcb" allow
probe "hw-layouter: Schaltplan (fremd)"      file-ownership.py hw-layouter Edit "$(pwd)/hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch" deny
probe "hw-designer: Layout (fremd)"          file-ownership.py hw-designer Edit "$(pwd)/hardware/esp32s3/WordClock-ESP32S3-V3.kicad_pcb" deny
probe "hw-designer: Schaltplan weiter"       file-ownership.py hw-designer Edit "$(pwd)/hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch" allow
probe "hw-layouter: Fertigungsdaten"         file-ownership.py hw-layouter Edit "$(pwd)/hardware/esp32s3/fertigung/bom.csv" allow
probe "hw-doc-writer: eigene Doku"           file-ownership.py hw-doc-writer Edit "$(pwd)/hardware/dokumentation/wissensstand.md" allow
probe "hw-doc-writer: README"                file-ownership.py hw-doc-writer Edit "$(pwd)/hardware/README.md" allow
probe "hw-doc-writer: Entscheid (fremd)"     file-ownership.py hw-doc-writer Edit "$(pwd)/hardware/architektur/entscheidungen.md" deny
probe "hw-architect: README (jetzt fremd)"   file-ownership.py hw-architect Edit "$(pwd)/hardware/README.md" deny
probe "hw-reviewer: eigenes Gutachten"     file-ownership.py hw-reviewer Edit "$(pwd)/hardware/review/2026-10-17-schema.md" allow
probe "hw-reviewer: Entwurf aendern"       file-ownership.py hw-reviewer Edit "$(pwd)/hardware/esp32s3/gen.py" deny
probe "doc-writer: hardware-Doku (fremd)"  file-ownership.py doc-writer Edit "$(pwd)/hardware/architektur/entscheidungen.md" deny
probe "lead: hardware-Doku (fremd)"        file-ownership.py lead Edit "$(pwd)/hardware/architektur/entscheidungen.md" deny
probe "doc-writer: CLAUDE.md weiter erlaubt" file-ownership.py doc-writer Edit "$(pwd)/CLAUDE.md" allow
probe "lead: CLAUDE.md weiter erlaubt"     file-ownership.py lead Edit "$(pwd)/CLAUDE.md" allow

# sw-architect (17.10.2026): specs/multiboard-quelle/ hat einen eigenen Schreiber; spec-writer und Lead
# duerfen dort nicht schreiben, behalten aber alle uebrigen Specs.
echo "  --- file-ownership.py: specs/multiboard-quelle/ (sw-architect)"
probe "sw-architect: eigene Spec"          file-ownership.py sw-architect Edit "$(pwd)/specs/multiboard-quelle/design.md" allow
probe "sw-architect: fremde Spec"          file-ownership.py sw-architect Edit "$(pwd)/specs/esp32s3-port/design.md" deny
probe "sw-architect: Code"                 file-ownership.py sw-architect Edit "$(pwd)/src/main.c" deny
probe "spec-writer: multiboard (fremd)"    file-ownership.py spec-writer Edit "$(pwd)/specs/multiboard-quelle/design.md" deny
probe "spec-writer: eigene Specs weiter"   file-ownership.py spec-writer Edit "$(pwd)/specs/esp32s3-port/design.md" allow
probe "lead: multiboard (fremd)"           file-ownership.py lead Edit "$(pwd)/specs/multiboard-quelle/design.md" deny
probe "doc-writer: multiboard (fremd)"      file-ownership.py doc-writer Edit "$(pwd)/specs/multiboard-quelle/design.md" deny
probe "lead: andere Specs weiter"          file-ownership.py lead Edit "$(pwd)/specs/esp32s3-port/design.md" allow

echo "  --- no-danger.py (AKW.4): Aufruf verboten, Erwaehnung erlaubt"
probe "lesender Endpunkt"             no-danger.py "" Bash 'curl -s http://1.2.3.4/api/update_status' allow
probe "test_display aufrufen"         no-danger.py "" Bash 'curl -s http://1.2.3.4/api/test_display' deny
probe "learn_ir aufrufen"             no-danger.py "" Bash 'curl -s http://1.2.3.4/api/learn_ir' deny
probe "fs_remove aufrufen"            no-danger.py "" Bash 'curl -s "http://1.2.3.4/api/fs_remove?filename=app.js.gz"' deny
probe "ir_code_set aufrufen"          no-danger.py "" Bash 'curl -s "http://1.2.3.4/api/ir_code_set?idx=0"' deny
probe "EEPROM-Reset aufrufen"         no-danger.py "" Bash 'curl -s http://1.2.3.4/api/maintenance_reset_eeprom' deny
probe "Parameter ohne Gleichheits="   no-danger.py "" Bash 'curl -s "http://1.2.3.4/?a"' deny
# Paket 2026-10-13 (0.6, Ent-11): Ab Release EP gehen die Wartungsendpunkte per POST.
# Ein Hook, der nur auf die URL im GET-Aufruf schaut, saehe den POST nicht.
probe "EEPROM-Reset per POST"         no-danger.py "" Bash 'curl -s -X POST http://1.2.3.4/api/maintenance_reset_eeprom' deny
probe "Formatieren per POST mit Daten" no-danger.py "" Bash 'curl -s -X POST --data "x=1" "http://1.2.3.4/api/maintenance_format_fs"' deny
probe "fs_remove per POST, -d vorn"   no-danger.py "" Bash 'curl -d "filename=app.js.gz" http://1.2.3.4/api/fs_remove' deny
# L426/C60: der Legacy-Weg loescht die STM-Einstellungen per GET, ohne Herkunftspruefung.
probe "Legacy rsteeprom2 aufrufen"    no-danger.py "" Bash 'curl -s "http://1.2.3.4/?action=rsteeprom2"' deny
probe "Legacy format aufrufen"        no-danger.py "" Bash 'curl -s "http://1.2.3.4/fs?action=format"' deny
probe "network_secrets abrufen"       no-danger.py "" Bash 'curl -s http://1.2.3.4/api/network_secrets' deny
probe "Endpunkt im Bericht erwaehnen" no-danger.py "" Bash 'echo "Schritt test_display ausgelassen (DIR-012)"' allow
probe "Endpunkt in Doku schreiben"    no-danger.py "" Bash 'python3 -c "open(\"BEFUNDE.md\",\"a\").write(\"learn_ir ist gefaehrlich\")"' allow

printf '      %d Proben bestanden, %d fehlgeschlagen\n' "$ok" "$schlecht"
[ "$schlecht" -eq 0 ] || exit 1
