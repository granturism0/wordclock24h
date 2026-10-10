# Pruefstand C3 (Paket 2026-10-09, L339): zieht den ECHTEN Code aus src/eeprom/eeprom.c -- die
# Seitengroesse je Ziel (falls vorhanden) und eeprom_write() -- und die Bereiche aus eeprom-data.h.
# Aufruf: extract.py <src-Verzeichnis> <auszug.c> <bereiche.h> [sabotage]
#   sabotage 1: Firmware-Seitengroesse 64 statt 32 (Baustein bleibt 32)
#   sabotage 3: Seitenende um eins verschoben (len = P + 1 - ...)
#   (Sabotage 2 -- Firmware 32 gegen Baustein 8 -- setzt run.sh ueber -DBAUSTEIN_P=8.)
# Exit 2, wenn ein Anker fehlt oder nicht eindeutig ist: Ein Auszug, der still etwas anderes
# ausschneidet, waere schlimmer als keiner.
import sys, re
src, out, rout = sys.argv[1], sys.argv[2], sys.argv[3]
sab = sys.argv[4] if len(sys.argv) > 4 else '0'

def text(path):
    return open(path, 'rb').read().decode('latin-1').replace('\r\n', '\n')

def fail(msg):
    sys.stderr.write('extract: ' + msg + '\n'); sys.exit(2)

t = text(src + '/eeprom/eeprom.c')
# --- Seitengroesse je Ziel (gibt es erst ab C3) ---
pm = '#if defined (STM32F4XX)\n#define EEPROM_PAGE_SIZE'
if t.count(pm) > 1: fail('Seitengroessen-Block nicht eindeutig')
page = ''
if t.count(pm) == 1:
    a = t.index(pm); b = t.index('#endif\n', a) + len('#endif\n')
    page = '#line %d "%s"\n' % (t[:a].count('\n') + 1, 'eeprom.c') + t[a:b]
# --- eeprom_write() ---
fm = 'uint_fast8_t\neeprom_write (uint_fast16_t start_addr'
if t.count(fm) != 1: fail('eeprom_write() nicht genau einmal gefunden')
a = t.index(fm); b = t.index('\n}\n', a) + 3
func = '#line %d "%s"\n' % (t[:a].count('\n') + 1, 'eeprom.c') + t[a:b]

if sab == '1':
    o = '#define EEPROM_PAGE_SIZE        32'
    if page.count(o) != 1: fail('Sabotage 1: Anker fehlt')
    page = page.replace(o, '#define EEPROM_PAGE_SIZE        64')
elif sab == '3':
    o = 'len = EEPROM_PAGE_SIZE - (start_addr % EEPROM_PAGE_SIZE);'
    if func.count(o) != 1: fail('Sabotage 3: Anker fehlt')
    func = func.replace(o, 'len = EEPROM_PAGE_SIZE + 1 - (start_addr % EEPROM_PAGE_SIZE);')
    # Puffer um eins vergroessern: Sabotage 3 soll die SEITENLOGIK pruefen, nicht im Pufferueberlauf abstuerzen
    o = 'uint8_t         page[EEPROM_PAGE_SIZE];'
    if func.count(o) != 1: fail('Sabotage 3: Pufferanker fehlt')
    func = func.replace(o, 'uint8_t         page[EEPROM_PAGE_SIZE + 1];')
elif sab not in ('0', '2'):           # 2 setzt run.sh ueber -DBAUSTEIN_P=8
    fail('unbekannte Sabotage ' + sab)
open(out, 'w').write(page + func)

# --- Bereiche aus eeprom-data.h, Makros aus den Headern aufgeloest ---
hdrs = ['eeprom/eeprom-data.h', 'weather/weather.h', 'display/display.h', 'night/night.h',
        'alarm/alarm.h', 'overlay/overlay.h', 'remote-ir/remote-ir.h']
defs = {}
for h in hdrs:
    s = text(src + '/' + h)
    s = re.sub(r'/\*.*?\*/', ' ', s, flags=re.S).replace('\\\n', ' ')
    for line in s.split('\n'):
        line = re.sub(r'//.*', '', line)
        m = re.match(r'\s*#\s*define\s+(\w+)\s+(.*\S)\s*$', line)
        if m:
            defs.setdefault(m.group(1), []).append((h, m.group(2)))
d = text(src + '/eeprom/eeprom-data.h')
offs = re.findall(r'#define\s+EEPROM_DATA_OFFSET_(\w+)\s', d)
names = [n for n in offs if 'EEPROM_DATA_SIZE_' + n in defs]
if len(names) != len(offs): fail('Bereich ohne Groesse: ' + str(set(offs) - set(names)))
need, todo = [], ['EEPROM_DATA_OFFSET_' + n for n in names] + ['EEPROM_DATA_SIZE_' + n for n in names] + \
                   ['EEPROM_DATA_END', 'OVERLAY_ENTRY_SIZE', 'MAX_OVERLAYS']
while todo:
    k = todo.pop()
    if k in need: continue
    if k not in defs: fail('Makro nicht gefunden: ' + k)
    vals = set(v for _, v in defs[k])
    if len(vals) != 1: fail('Makro mehrdeutig: %s %s' % (k, defs[k]))
    need.append(k)
    for tok in re.findall(r'\b[A-Z_][A-Z0-9_]*\b', defs[k][0][1]):
        if tok not in need: todo.append(tok)
o = ['/* erzeugt von extract.py aus %s */' % src]
for k in sorted(need):
    o.append('#define %s %s' % (k, defs[k][0][1]))
o.append('static const struct { const char * name; long off; long size; } bereiche[] = {')
for n in names:
    o.append('    { "%s", EEPROM_DATA_OFFSET_%s, EEPROM_DATA_SIZE_%s },' % (n, n, n))
o.append('};')
o.append('#define N_BEREICHE %d' % len(names))
open(rout, 'w').write('\n'.join(o) + '\n')
print('extrahiert: Seitenblock %s, eeprom_write %d Zeilen, %d Bereiche, Sabotage %s'
      % ('ja' if page else 'NEIN (Stand vor C3)', func.count('\n'), len(names), sab))
