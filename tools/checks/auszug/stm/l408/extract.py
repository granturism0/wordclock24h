# Pruefstand L408 (a): schneidet var_send_num8_array() und die beiden Dimmtabellen-Sender aus vars.c
# und prueft statisch JEDEN Aufruf von var_send_num8_array/var_send_num16_array auf die Elementzahl.
# Aufruf: extract.py <src-Verzeichnis> <ausgabe.c>      Exit 0 OK, 1 FEHL (statisch), 2 kaputt.
import sys, re
src, out = sys.argv[1], sys.argv[2]
t = open(src + '/vars/vars.c', 'rb').read().decode('ascii').replace('\r\n', '\n')
def cut(start, end):
    if t.count(start) != 1 or t.count(end) != 1:
        print('l408 extract: Anker %d/%d-mal: %r %r' % (t.count(start), t.count(end), start, end)); sys.exit(2)
    a = t.index(start); b = t.index(end, a)
    return '#line %d "%s"\n' % (t[:a].count('\n') + 1, src + '/vars/vars.c') + t[a:b] + '\n'
s = cut('static void\nvar_send_num8_array (', '#if 0 // yet not used')
s += cut('static void\nvar_send_dimmed_display_colors (void)', 'static void\nvar_send_dfplayer_is_up (void)')
open(out, 'w').write(s)

# Statisch: alle Aufrufe ausserhalb von Kommentaren, ohne die Definitionen. #if 0-Bloecke zaehlen mit,
# weil sie beim Einschalten denselben Fehler traegen -- gezaehlt wird, was im Text steht.
m = re.sub(r'/\*.*?\*/|//[^\n]*', lambda k: '\n' * k.group(0).count('\n'), t, flags=re.S)   # Zeilen bleiben gezaehlt
calls = [c for c in re.finditer(r'\b(var_send_num(?:8|16)_array)\s*\(([^;]*?)\)\s*;', m, flags=re.S)]
calls = [c for c in calls if not re.search(r'\b(?:NUM8|NUM16)_ARRAY\s+var\b', c.group(2))]
defs = len(re.findall(r'^var_send_num(?:8|16)_array\s*\(', m, flags=re.M))
bad = 0
for c in calls:
    args = [x.strip() for x in re.split(r',(?![^()]*\))', ' '.join(c.group(2).split()))]
    n = args[-1]
    mm = re.fullmatch(r'sizeof \((.+)\) / sizeof \((.+)\[0\]\)', n)
    ok = bool(mm and mm.group(1) == mm.group(2) == args[1]) or bool(re.fullmatch(r'[A-Z0-9_ +()]+', n))
    line = m[:c.start()].count('\n') + 1
    print('  Aufruf %s Zeile %d: Elementzahl "%s" -- %s' % (c.group(1), line, n, 'OK' if ok else 'FEHL (Bytezahl statt Elementzahl?)'))
    bad += 0 if ok else 1
print('l408 statisch: %d Definitionen, %d Aufrufe von var_send_num8/16_array, %d FEHL' % (defs, len(calls), bad))
if len(calls) != 2:
    print('  FEHL  erwartet genau 2 Aufrufe (Dimmtabellen) -- Bestand hat sich geaendert, Pruefstand nachziehen')
    bad += 1
sys.exit(1 if bad else 0)
