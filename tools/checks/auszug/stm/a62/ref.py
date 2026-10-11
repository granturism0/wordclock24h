# Pruefstand A62: Referenz UNABHAENGIG vom C-Code -- Python datetime.
# unix = n - 2208988800 + 2^32, falls n < 2208988800 (Aera 1); gueltig genau dann, wenn
# 1'767'225'600 <= unix < 2^32 (design.md 2.2, 2.5). Gibt ref.h fuer a62.c aus.
import datetime, random
E = 2208988800; M = 2**32; UNTEN = 1767225600
def unix(n): return n - E + (M if n < E else 0)
def gueltig(n): return UNTEN <= unix(n) < M
def felder(n):
    t = datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc) + datetime.timedelta(seconds=unix(n))
    tt = t.timetuple()
    return (t.year, t.month, t.day, t.hour, t.minute, t.second, (t.weekday() + 1) % 7, tt.tm_yday - 1)
# Benannte Grenzen aus der Spec (design.md 2.5, requirements AKN.1) und ihre Nachbarn
benannt = [4294967295, 0, 1, 123, 2208988799, 2208988800, 3976214399, 3976214400, 4000648905, 400064890,
           3304, 14, 15, 16]
for g in (0, M - 1, E, 3976214400):
    for d in range(-20, 21): benannt.append((g + d) % M)
r = random.Random(20261011)
zufall = [r.randrange(M) for _ in range(20000)]
# Jeder Jahresanfang 2026..2106 (UTC) +-1 s, als NTP-Sekunde
jahre = []
for y in range(2026, 2107):
    u = int(datetime.datetime(y, 1, 1, tzinfo=datetime.timezone.utc).timestamp())
    for d in (-1, 0, 1):
        if 0 <= u + d < M: jahre.append((u + d + E) % M)
alle = []
for n in benannt + jahre + zufall:
    if n not in alle: alle.append(n)
print('/* erzeugt von ref.py -- Python datetime, unabhaengig vom C-Code */')
print('typedef struct { uint32_t n; int gueltig; int g_vor; int f[8]; } REF;')
print('static const REF ref[] = {')
for n in alle:
    f = felder(n) if gueltig(n) else (0,) * 8
    print(' { %uu, %d, %d, { %s } },' % (n, 1 if gueltig(n) else 0, 1 if gueltig((n - 15) % M) else 0, ', '.join(map(str, f))))
print('};')
print('#define REF_N %d' % len(alle))
print('#define REF_BENANNT %d' % len(set(benannt)))
# Formfehler: Text, der IMMER verworfen werden muss (requirements AKN.1)
form = ['', ' 1', '+1', '-1', '1x', 'x1', '1 ', '12345678901', '04000648905', '4294967296', '9999999999',
        '99999999999999', '4294967300', '42949672950', '000000000000000', '0x10', '1.5', '\\t1', '1e9']
print('static const char * const form[] = {' + ', '.join('"%s"' % s for s in form) + '};')
print('#define FORM_N %d' % len(form))
