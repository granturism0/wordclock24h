#!/usr/bin/env python3
"""c71i statisch (AKI.3; design.md 3.3). Aufruf: c71i_static.py <ESP-Quellverzeichnis> [bezeichnung].
Liest Code ohne Kommentare. Exit 0 OK, 1 FEHL.
  S1  update_cache_step_run (): genau ein update_cache_vorher (), im Zweig case 0 nach dem Temporaerdatei-Block und
      vor "case 1:" - also vor dem ersten update_server_fetch_line ()
  S2  Schritte 1-3: update_cache_heap_mark (update_cache_step) zwischen update_server_fetch_line () und der Pruefung
      des Ergebnisses; Schritt 4/5: update_cache_heap_mark (4)/(5) unmittelbar nach update_cache_write ()
  S3  update_cache_finish (): update_cache_heap_report () als letzte Anweisung; update_cache_poll (): im Zweig "Host
      oder Pfad leer" update_cache_heap_report () vor update_cache_step = 0
  S4  Ausgabe: update_cache_i6_put () mit Serial.println (line) und stm32_log_append (line); vorher, heap_report und
      tmp_note geben nur darueber aus; http_accept_note () mit beiden
  S5  in den I-6-Funktionen kein String, kein "fehlgeschlagen", kein umm_local.h im Verzeichnis
  S6  update_cache_heap_mark (): ESP.getHeapStats (frag statt fe, Wahl im Bericht)
  S7  http_server_loop (): http_accept_note () unmittelbar nach heap_low_note (HEAP_LOW_PATH_ACCEPT)
  S8  setup (): genau ein Block mit ESP.getFlashChipId (), nach "CAP var-crc", vor EEPROM.begin () und vor dem WLAN
"""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
def roh(name):
    p = os.path.join(q, name)
    return open(p, 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n') if os.path.exists(p) else ''
def code(s, strings=True):
    s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
    if strings: s = re.sub(r'"(?:\\.|[^"\\])*"', '""', s)
    return [re.sub(r'//.*', '', l) for l in s.split('\n')]
Hr, Ir = roh('http.cpp'), roh('ESP-uclock.ino')
H, I, Hs, Is = code(Hr), code(Ir), code(Hr, False), code(Ir, False)
def body(L, kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return []
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return L[i:j + 1]
fails = []; n = 0
def chk(name, ok):
    global n; n += 1
    if not ok: fails.append(name)
def idx(b, rx, start=0):
    return next((k for k in range(start, len(b)) if re.search(rx, b[k])), None)
sr = body(H, 'update_cache_step_run (')
aufrufe = [k for k, l in enumerate(H) if re.search(r'\bupdate_cache_vorher\s*\(\s*\)\s*;', l)]
c0, c1 = idx(sr, r'^\s*case 0:'), idx(sr, r'^\s*case 1:')
v = idx(sr, r'\bupdate_cache_vorher\s*\(\s*\)\s*;')
tn = idx(sr, r'\bupdate_cache_tmp_note\s*\(')
fl = idx(sr, r'\bupdate_server_fetch_line\s*\(')
chk(f'S1 genau ein Aufruf update_cache_vorher () (gefunden {len(aufrufe)})', len(aufrufe) == 1)
chk('S1 vorher in case 0, nach dem Temporaerdatei-Block, vor case 1 und vor dem ersten Abruf',
    None not in (c0, c1, v, tn, fl) and c0 < tn < v < c1 < fl)
mk = idx(sr, r'\bupdate_cache_heap_mark\s*\(\s*update_cache_step\s*\)')
pr = idx(sr, r'if\s*\(\s*got\s*<=\s*0\s*\)')
chk('S2 Messpunkt 1-3 zwischen update_server_fetch_line () und der Ergebnispruefung', None not in (fl, mk, pr) and fl < mk < pr)
for nr, rx in ((4, r'update_cache_write\s*\(\s*RELEASENOTE_HTML'), (5, r'update_cache_write\s*\(\s*WC_LIST_TXT')):
    w = idx(sr, rx)
    nxt = next((k for k in range(w + 1, len(sr)) if sr[k].strip()), None) if w is not None else None
    chk(f'S2 Messpunkt {nr} unmittelbar nach update_cache_write ()', nxt is not None and re.search(rf'\bupdate_cache_heap_mark\s*\(\s*{nr}\s*\)\s*;', sr[nxt]))
fi = [l.strip() for l in body(H, 'update_cache_finish (') if l.strip()]
chk('S3 update_cache_finish (): update_cache_heap_report () als letzte Anweisung', len(fi) >= 2 and fi[-1] == '}' and fi[-2] == 'update_cache_heap_report ();')
po = body(H, 'update_cache_poll (')
ql = idx(po, r'if\s*\(\s*!\s*host\[0\]\s*\|\|\s*!\s*path\[0\]\s*\)')
rp = idx(po, r'\bupdate_cache_heap_report\s*\(\s*\)', ql or 0) if ql is not None else None
st = idx(po, r'update_cache_step\s*=\s*0\s*;', ql or 0) if ql is not None else None
chk('S3 Abbruch "Host oder Pfad leer": Bericht vor update_cache_step = 0', None not in (ql, rp, st) and ql < rp < st)
put = body(H, 'update_cache_i6_put (')
chk('S4 update_cache_i6_put (): Serial.println (line) und stm32_log_append (line)',
    any(re.search(r'Serial\.println\s*\(\s*line\s*\)', l) for l in put) and any(re.search(r'stm32_log_append\s*\(\s*line\s*\)', l) for l in put))
for f in ('update_cache_vorher (', 'update_cache_heap_report (', 'update_cache_tmp_note ('):
    b = body(H, f)
    chk(f'S4 {f.strip(" (")} () gibt nur ueber update_cache_i6_put () aus',
        b and any(re.search(r'\bupdate_cache_i6_put\s*\(\s*line\s*\)', l) for l in b) and not any(re.search(r'Serial\.|stm32_log_append', l) for l in b))
ea = body(H, 'http_accept_note (')
chk('S4 http_accept_note (): Serial.println (line) und stm32_log_append (line)',
    any(re.search(r'Serial\.println\s*\(\s*line\s*\)', l) for l in ea) and any(re.search(r'stm32_log_append\s*\(\s*line\s*\)', l) for l in ea))
i6 = []
for f in ('update_cache_i6_put (', 'update_cache_heap_mark (', 'update_cache_vorher (', 'update_cache_heap_report (', 'update_cache_tmp_mask (',
          'update_cache_tmp_note (', 'http_accept_note ('):
    i6 += body(Hs, f)
chk('S5 kein String in den I-6-Funktionen', i6 and not any(re.search(r'\bString\b', l) for l in i6))
chk('S5 keine I-6-Zeile enthaelt "fehlgeschlagen"', not any('fehlgeschlagen' in l for l in i6))
chk('S5 kein #include des privaten Kopfs umm_local.h', not any(re.search(r'^\s*#\s*include\s*[<"][^>"]*umm_local\.h', roh(f), re.M) for f in os.listdir(q) if f.endswith(('.cpp', '.h', '.ino'))))
chk('S6 update_cache_heap_mark (): ESP.getHeapStats (blk und frag aus einem Gang)', any(re.search(r'ESP\.getHeapStats\s*\(', l) for l in body(H, 'update_cache_heap_mark (')))
sl = body(H, 'http_server_loop (void)')
a = idx(sl, r'heap_low_note\s*\(\s*HEAP_LOW_PATH_ACCEPT\s*\)')
nxt = next((k for k in range(a + 1, len(sl)) if sl[k].strip()), None) if a is not None else None
chk('S7 http_accept_note () unmittelbar nach dem Messpunkt accept', nxt is not None and re.search(r'^\s*http_accept_note\s*\(\s*\)\s*;', sl[nxt]))
se = body(Is, 'setup()')
fid = [k for k, l in enumerate(se) if 'ESP.getFlashChipId' in l]
cap = idx(se, r'Serial\.println\s*\(\s*"CAP var-crc"\s*\)')
eep = idx(se, r'EEPROM\.begin\s*\(')
wl = idx(se, r'\bwifi_(connect|ap)\s*\(')
chk(f'S8 setup (): genau ein ESP.getFlashChipId () (gefunden {len(fid)})', len(fid) == 1)
chk('S8 Flash-Kennung nach CAP, vor EEPROM.begin () und vor dem WLAN', len(fid) == 1 and None not in (cap, eep, wl) and cap < fid[0] < eep and fid[0] < wl)
print(f"c71i statisch ({bez}): {n} Pruefungen")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c71i statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
