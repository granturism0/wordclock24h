#!/usr/bin/env python3
"""c72i statisch (AKI.1, AKI.2; design.md 3.1, 3.2). Aufruf: c72i_static.py <ESP-Quellverzeichnis> [bezeichnung].
Liest Code ohne Kommentare. Exit 0 OK, 1 FEHL.
  S1  #include <include/ClientContext.h> in http.cpp
  S2  im Listengang (tcp_now_arg, tcp_now_add, tcp_now_read, WcHttpServer::unclaimed/listen_arg) keine Ausgabe, keine
      Abgabe, kein Heap: kein http_send/http_flush/http_json_, Serial, yield, delay, optimistic_yield, esp_yield,
      connected, available, String, new, malloc
  S3  http_api_device_ready (): genau ein tcp_now_read (&tcp_now), und zwar VOR der ersten Ausgabe
  S4  http_server_loop (): hasClient () genau einmal und vor accept (); im Zweig if (!http_client) zuerst
      http_accept_dead_note (unclaimed_before), dann return
  S5  TCP_NOW_LIST_MAX 8, list[TCP_NOW_LIST_MAX], Kappung in tcp_now_add ()
  S6  base.h: Prototypen tcp_now_tcp_field und http_no_request_timeouts_get
  S7  esp_heap_log (): genau ein snprintf_P in line, je genau ein Serial.println (line) und stm32_log_append (line),
      tcp_now_tcp_field () vor dem snprintf_P, line mindestens 106 Byte
  S8  Kommentar zu I-4 nach P2 berichtigt: "Die Listen sind kurz (MEMP_NUM_TCP_PCB 5" steht nicht mehr da
"""
import os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
def roh(name):
    p = os.path.join(q, name)
    return open(p, 'rb').read().decode('utf-8', 'replace').replace('\r\n', '\n') if os.path.exists(p) else ''
def code(s):
    n0 = s.count('\n') + 1
    s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
    s = re.sub(r"'(?:\\.|[^'\\\n])'", "''", s)                # L477/E40: Zeichenliterale zuerst
    s = re.sub(r'"(?:\\.|[^"\\\n])*"', '""', s)              # Zeichenketten leeren, enden in ihrer Zeile
    L = [re.sub(r'//.*', '', l) for l in s.split('\n')]
    if len(L) != n0: print(f'KAPUTT: Bereinigung veraendert die Zeilenzahl ({n0} -> {len(L)}) - Pruefung saehe ihren Gegenstand nicht vollstaendig (L477/E40)'); sys.exit(2)
    return L
Hr, Ir, Br = roh('http.cpp'), roh('ESP-uclock.ino'), roh('base.h')
H, I, B = code(Hr), code(Ir), code(Br)
def body(L, kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return []
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return L[i:j + 1]
def methode(L, kopf):
    i = next((k for k, l in enumerate(L) if l.strip().startswith(kopf)), None)
    if i is None: return []
    tiefe, out = 0, []
    for l in L[i:]:
        out.append(l); tiefe += l.count('{') - l.count('}')
        if '{' in ''.join(out) and tiefe == 0: break
    return out
fails = []; n = 0
def chk(name, ok):
    global n; n += 1
    if not ok: fails.append(name)
chk('S1 #include <include/ClientContext.h> in http.cpp', any(re.match(r'\s*#\s*include\s*<include/ClientContext\.h>', l) for l in H))
verboten = re.compile(r'\b(http_send\w*|http_flush|http_json_send\w*|Serial|yield|delay|optimistic_yield|esp_yield|esp_delay|connected|available|String|new|malloc|calloc|realloc)\b')
gang = {'tcp_now_arg': body(H, 'tcp_now_arg ('), 'tcp_now_add': body(H, 'tcp_now_add ('), 'tcp_now_read': body(H, 'tcp_now_read ('),
        'unclaimed': methode(H, 'uint8_t unclaimed (void) const'), 'listen_arg': methode(H, 'const void * listen_arg (void) const')}
for f, b in gang.items():
    treffer = [l.strip() for l in b if verboten.search(l)]
    chk(f'S2 {f} () vorhanden und ohne Ausgabe/Abgabe/Heap' + (f': {treffer[0]}' if treffer else ''), b and not treffer)
dr = body(H, 'http_api_device_ready ()')
rd = [k for k, l in enumerate(dr) if re.search(r'\btcp_now_read\s*\(\s*&\s*tcp_now\s*\)', l)]
aus = next((k for k, l in enumerate(dr) if re.search(r'\bhttp_(send|flush|json_send\w*)\s*\(', l)), None)
chk(f'S3 device_ready: genau ein tcp_now_read (&tcp_now) (gefunden {len(rd)})', len(rd) == 1)
chk('S3 device_ready: tcp_now_read () VOR der ersten Ausgabe', len(rd) == 1 and aus is not None and rd[0] < aus)
sl = body(H, 'http_server_loop (void)')
hc = [k for k, l in enumerate(sl) if re.search(r'\bunclaimed_before\s*=\s*http_server\.hasClient\s*\(\s*\)', l)]
ac = [k for k, l in enumerate(sl) if re.search(r'http_server\.accept\s*\(', l)]
chk('S4 hasClient () genau einmal, vor accept ()', len(hc) == 1 and len(ac) == 1 and hc[0] < ac[0])
a = next((k for k, l in enumerate(sl) if re.search(r'if\s*\(\s*!\s*http_client\s*\)', l)), None)
if a is not None:
    e = next((k for k in range(a + 1, len(sl)) if re.match(r'^    }\s*$', sl[k])), None)
    zweig = [l.strip() for l in sl[a + 1:e] if l.strip() not in ('', '{')] if e else []
else:
    zweig = []
chk(f'S4 Zweig if (!http_client): http_accept_dead_note (unclaimed_before); return; ({zweig})',
    zweig == ['http_accept_dead_note (unclaimed_before);', 'return;'])
hs = '\n'.join(H)
chk('S5 #define TCP_NOW_LIST_MAX 8', re.search(r'^#define\s+TCP_NOW_LIST_MAX\s+8\b', hs, re.M))
chk('S5 list[TCP_NOW_LIST_MAX]', re.search(r'\blist\s*\[\s*TCP_NOW_LIST_MAX\s*\]', hs))
chk('S5 Kappung in tcp_now_add (): t->n >= TCP_NOW_LIST_MAX', any(re.search(r't->n\s*>=\s*TCP_NOW_LIST_MAX', l) for l in gang['tcp_now_add']))
bs = '\n'.join(B)
chk('S6 base.h: extern size_t tcp_now_tcp_field (char *, size_t)', re.search(r'extern\s+size_t\s+tcp_now_tcp_field\s*\(\s*char\s*\*\s*,\s*size_t\s*\)', bs))
chk('S6 base.h: extern uint16_t http_no_request_timeouts_get (void)', re.search(r'extern\s+uint16_t\s+http_no_request_timeouts_get\s*\(\s*void\s*\)', bs))
hl = body(I, 'esp_heap_log (void)')
sp = [k for k, l in enumerate(hl) if re.search(r'snprintf_P\s*\(\s*line\b', l)]
pr = [k for k, l in enumerate(hl) if re.search(r'Serial\.println\s*\(\s*line\s*\)', l)]
rg = [k for k, l in enumerate(hl) if re.search(r'stm32_log_append\s*\(\s*line\s*\)', l)]
tf = [k for k, l in enumerate(hl) if re.search(r'\btcp_now_tcp_field\s*\(', l)]
gr = next((int(m.group(1)) for l in hl for m in [re.search(r'char\s+line\s*\[\s*(\d+)\s*\]', l)] if m), 0)
chk(f'S7 esp_heap_log: ein snprintf_P (line), je ein Serial.println (line) und stm32_log_append (line) ({len(sp)}/{len(pr)}/{len(rg)})',
    len(sp) == 1 and len(pr) == 1 and len(rg) == 1)
chk('S7 esp_heap_log: tcp_now_tcp_field () vor dem snprintf_P', len(tf) == 1 and len(sp) == 1 and tf[0] < sp[0])
chk(f'S7 esp_heap_log: line mindestens 106 Byte (gefunden {gr})', gr >= 106)
chk('S8 Kommentar zu I-4 nach P2 berichtigt', 'Die Listen sind kurz (MEMP_NUM_TCP_PCB 5' not in Hr)
print(f"c72i statisch ({bez}): {n} Pruefungen")
for f in fails: print("      FEHLER: " + f)
print(f"{'FEHL' if fails else 'OK  '}  c72i statisch {bez}: {len(fails)} Fehler")
sys.exit(1 if fails else 0)
