#!/bin/sh
# Pruefstand F.5g (C38 / L323, AKF.6). Aufruf: run.sh <verzeichnis mit http.cpp, eepromdata.h> <bezeichnung>
# SSID und WLAN-Schluessel werden abgewiesen statt still gekuerzt -- in /api/network_client_set,
# /api/eeprom_settings_set, /api/network_ap_set und auf der Legacy-WLAN-Seite (Review F.6, A4). Am Geraet NICHT pruefbar: network_client_set bleibt in jedem
# Geraetetest gesperrt (tasks.md, Runde F). Deshalb dieser Pruefstand.
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
python3 "$S/extract.py" func "$Q/http.cpp" http_strvar_len_ok http_check_strvar_len http_api_network_client_set http_api_eeprom_settings_set \
  http_api_network_ap_set > "$D/c38_code.inc" 2>"$D/extract.log"
# Review F.6 A4: die beiden WLAN-Zweige der Legacy-Seite, als Block aus http_network () (bis vor savetimeserver)
python3 "$S/extract.py" block "$Q/http.cpp" 'if (! strcmp (action, "savewlanlist")' 'else if (! strcmp (action, "savetimeserver"))' \
  > "$D/c38_legacy.inc" 2>>"$D/extract.log"
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" -DEEPROMHDR="\"$Q/eepromdata.h\"" -DBEZEICHNUNG="\"$B\"" \
   "$D/t5.cpp" -o "$D/t5.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t5.bin"
