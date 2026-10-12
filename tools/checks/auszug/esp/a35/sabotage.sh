#!/bin/sh
# Sabotagen fuer a35: jede muss FEHL melden (DIR-014). Aufruf: sabotage.sh <verzeichnis mit vars.cpp, vars.h, base.cpp>
D=$(cd "$(dirname "$0")" && pwd); Q=$1; T=$(mktemp -d); rc=0
sab () {
    name=$1; rm -rf "$T/$name"; mkdir -p "$T/$name"; cp "$Q/vars.cpp" "$Q/vars.h" "$Q/base.cpp" "$T/$name/"
    python3 - "$T/$name/vars.cpp" "$2" "$3" <<'P' || { echo "ANKER FEHLT: $name"; rc=1; return; }
import sys; p = sys.argv[1]; a = sys.argv[2].encode().decode('unicode_escape').encode('latin-1'); b = sys.argv[3].encode().decode('unicode_escape').encode('latin-1')
s = open(p, 'rb').read(); assert s.count(a) >= 1, a
open(p, 'wb').write(s.replace(a, b, 1))
P
    r=$("$D/run.sh" "$T/$name" "sab-$name" | tail -1)
    case "$r" in FEHL*) echo "  schlaegt an: $name ($r)";; *) echo "  BLIEB GRUEN: $name ($r)"; rc=1;; esac
}
sab ohne_applied_O  'overlays[var_idx].days = days;\r\n                        applied = 1;' 'overlays[var_idx].days = days;'
sab ohne_applied_S  "= '\\\\0';\n                applied = 1;" "= '\\\\0';"
sab V_zaehlt        "if (! applied && cmd_line[0] != 'V')" 'if (! applied)'
sab keine_drossel   'if (! var_ignored_logged || (millis () - var_ignored_last) >= VAR_IGNORED_LOG_INTERVAL)' 'if (1)'
sab drossel_groesser '>= VAR_IGNORED_LOG_INTERVAL)' '> VAR_IGNORED_LOG_INTERVAL)'
sab ohne_mitschnitt '        Serial.println (line);\r\n        Serial.flush ();\r\n        stm32_log_append (line);' '        stm32_log_append (line);'
sab ohne_ring       '        Serial.flush ();\r\n        stm32_log_append (line);\r\n    }\r\n}' '        Serial.flush ();\r\n    }\r\n}'
sab nie_melden      '        var_ignored_note (cmd_line, min_len ? 1 : 0, var_idx);' '        ;'
sab idx_falsch      '(unsigned int) (var_idx & 0xFF)' '(unsigned int) ((var_idx + 1) & 0xFF)'
sab unbekannt_known 'min_len ? 1 : 0, var_idx' '1, var_idx'
sab rueckgabe       '    return 1;\r\n}\r\n\r\nvoid\nvars_init' '    return applied;\r\n}\r\n\r\nvoid\nvars_init'
sab zaehlt_nicht    'if (var_ignored_cnt < 0xFFFFFFFFUL)' 'if (0)'
rm -rf "$T"; exit $rc
