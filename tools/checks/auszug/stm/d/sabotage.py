# sabotage.py <src-kopie> <nr>: eine Sabotage in die Kopie schreiben (genau ein Treffer, sonst Abbruch).
import sys
src, nr = sys.argv[1], sys.argv[2]
S = {
 '1': ('esp8266/esp8266.c', "        if (! strcmp (mark, p + len - 4))", "        if (strtol (p + len - 4, 0, 16) == (long) var_crc (p))", "Marke lax wie htoi/strtol (L232)"),
 '2': ('esp8266/esp8266.c', "len >= 6 && len <= ESP8266_MAX_CMD_LEN + 5 && ", "len >= 6 && ", "keine Laengengrenze fuer u.cmd"),
 '3': ('esp8266/esp8266.c', "    if (uptime >= esp8266_cmc_sync_next)", "    if (1)", "Drossel 60 s entfernt"),
 '4': ('vars/vars.c', "? 0x07 : 0x06)", "? 0x03 : 0x02)", "Eroeffnung ohne 0x04"),
 '5': ('esp8266/esp8266.c', "    if (esp8266_cmc_rejects < 0xFFFF)\r\n    {\r\n        esp8266_cmc_rejects++;", "    {\r\n        esp8266_cmc_rejects++;", "Zaehler nicht saettigend"),
 '6': ('esp8266/esp8266.c', "if (answer[2] == 'C' && ! esp8266_cmc_check", "if (answer[2] == 'D' && ! esp8266_cmc_check", "CMC ungeprueft angewandt"),
 '7': ('vars/vars.c', "    sum1 = (uint8_t) len;\r\n\r\n    for (i = 0; i < len; i++)\r\n    {\r\n        sum1 = (uint8_t) (sum1 + (uint8_t) payload[i]);", "    sum1 = 0;\r\n\r\n    for (i = 0; i < len; i++)\r\n    {\r\n        sum1 = (uint8_t) (sum1 + (uint8_t) payload[i]);", "var_crc ohne Laenge als Startwert"),
 '8': ('main.c', "            esp8266_cmc_sync = 0;\r\n            var_sync_pending = 1;", "            esp8266_cmc_sync = 0;", "Hauptloop merkt nicht vor"),
 '9': ('esp8266/esp8266.c', "    if (esp8266_cmc_rejects <= 4 || (esp8266_cmc_rejects % 50) == 0)", "    if (1)", "Abweisungszeile ungedrosselt"),
 '11': ('esp8266/esp8266.c', "        if (! strcmp (mark, p + len - 4))\r\n        {\r\n            return 1;\r\n        }",
        "        {\r\n            const char * q = p + len - 4; unsigned int v = 0, k;\r\n            for (k = 0; k < 4; k++) { char c = q[k]; v = (v << 4) | ((c >= '0' && c <= '9') ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : 0); }\r\n            if (v == var_crc (p)) return 1;\r\n        }",
        "Marke mit htoi-Semantik: Nicht-Hex still 0 (L232)"),
 '10': ('esp8266/esp8266.c', "len >= 6 && len <=", "len >= 5 && len <=", "leere Nutzlast zugelassen"),
}
f, o, n, was = S[nr]
p = src + '/' + f
b = open(p, 'rb').read()
assert b.count(o.encode()) == 1, (nr, f, b.count(o.encode()))
open(p, 'wb').write(b.replace(o.encode(), n.encode()))
print('Sabotage', nr, ':', was)
