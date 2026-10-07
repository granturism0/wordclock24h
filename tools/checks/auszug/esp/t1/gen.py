#!/usr/bin/env python3
"""Erzeugt bruecke_code.inc aus einer ESP-uclock.ino (neu oder alt).
--ziel: Uhrvariablen mechanisch auf die Zielbreite (unsigned long -> uint32_t, L256)."""
import sys, re
sys.path.insert(0, sys.argv[1]); import importlib.util
spec = importlib.util.spec_from_file_location('ex', sys.argv[1] + '/extract.py')
src, out, ziel = sys.argv[2], sys.argv[3], '--ziel' in sys.argv
L = open(src, 'rb').read().decode('ascii').replace('\r\n', '\n').split('\n')
i = next(k for k, l in enumerate(L) if l.startswith('#define VAR_CRC_MARK '))
j = next(k for k, l in enumerate(L) if l.startswith('loop()'))
body = '\n'.join(L[i:j - 1])                     # ohne die Zeile "void" vor loop()
a = next(k for k, l in enumerate(L) if l.strip().startswith('if (! strncmp (cmd_buffer, "var ", 4))'))
b = next(k for k in range(a, len(L)) if L[k].strip().startswith('else if (! strcmp (cmd_buffer, "time"))'))
branch = '\n'.join(L[a:b])
if ziel:
    body = re.sub(r'static unsigned long(\s+)(\w+_millis)', r'static uint32_t\1\2', body)
import os, subprocess
htoi = subprocess.run(['python3', sys.argv[1] + '/extract.py', 'func', os.path.join(os.path.dirname(src), 'base.cpp'), 'htoi'], capture_output=True, text=True).stdout
flag = '#define HAVE_VAR_MARK_REQUIRED 1\n' if 'var_mark_required (const char' in body else ''
open(out, 'w').write(htoi + flag + body + '\n\nstatic void\nhandle_line (char * cmd_buffer)\n{\n' + branch + '\n}\n')
print('extrahiert:', src, 'Zeilen', i + 1, '-', j - 1, '+ var-Zweig', a + 1, '-', b, '(Zielbreite)' if ziel else '(nativ)')
