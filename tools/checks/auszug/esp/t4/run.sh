#!/bin/sh
# Pruefstand S.5 (N1). Aufruf: run.sh <verzeichnis mit http.cpp, vars.h> <bezeichnung>
D=$(cd "$(dirname "$0")" && pwd); S=$(dirname "$D"); Q=$1; B=$2
{ python3 "$S/extract.py" func "$Q/http.cpp" http_get_int_param http_get_opt_int_param   # F.5j/C6u: echter Code statt atoi-Attrappe
  python3 "$S/extract.py" func "$Q/http.cpp" http_n_overlays_for_read
  python3 - "$Q/http.cpp" <<'PY'
import sys
L = open(sys.argv[1], 'rb').read().decode('utf-8').split('\n')
a = next(i for i, l in enumerate(L) if l.startswith('#define N_OVERLAY_TYPES'))
f = next(i for i in range(a, len(L)) if L[i].startswith('http_overlays (void)'))
b = next(i for i in range(f, len(L)) if L[i].startswith('}'))
print('\n'.join(L[a:b + 1]))
PY
  python3 "$S/extract.py" func "$Q/http.cpp" http_get_overlay_idx_param http_api_overlay_display http_api_overlay_delete
} > "$D/n1_code.inc" 2>"$D/extract.log"
cc -x c++ -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -I"$D" $CCX -DVARSHDR="\"$Q/vars.h\"" -DBEZEICHNUNG="\"$B\"" "$D/t4.cpp" -o "$D/t4.bin" -lc++ 2>"$D/cc.log" \
  || { echo "UEBERSETZUNG FEHLGESCHLAGEN ($B):"; grep error "$D/cc.log" | head -15; exit 2; }
"$D/t4.bin"
