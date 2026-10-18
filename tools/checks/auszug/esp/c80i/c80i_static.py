#!/usr/bin/env python3
"""c80i statisch (Paket 2026-10-17-c80, Task 1.3; AKI.1, AKI.6, AKI.7, AKI.11; design.md 4.8 Faelle 8, 12, 16, 22).
Aufruf: c80i_static.py <ESP-Quellverzeichnis> [bezeichnung]. Liest Code ohne Kommentare. Exit 0 OK, 1 FEHL.
  B1  Bindung an Core 3.1.2: #error ohne ARDUINO_ESP8266_RELEASE_3_1_2, static_assert Kontextgroesse 52 und
      offsetof (numblocks) 48, #error bei UMM_NUM_HEAPS != 1; umm_local.h letzter Include, danach #undef memcpy,
      memmove, memset
  B2  Sperre: genau ein xt_rsil (15) und ein xt_wsr_ps (ps), dazwischen genau der Aufruf hk_gang (...) und nichts sonst
      (keine Verzweigung); kein Destruktor (kein RAII) in heapkarte.cpp
  B3  EEPROM nur ueber getConstDataPtr (), nirgends getDataPtr ()
  B4  kein Heap, keine CPU-Abgabe im Instrument: kein String, new, malloc, calloc, realloc, std::, yield, delay,
      optimistic_yield, esp_yield in heapkarte.cpp/.h
  B5  beide Ausgabewege mit demselben Puffer: in heapkarte.cpp genau ein Serial.println (line), ein Serial.flush (),
      ein stm32_log_append (line), alle in heapkarte_ausgabe (), dort genau ein hk_ausgabe (...) in char line[120]
  B6  loop (): Reihenfolge und Aufrufe wie 3.2.41, jeder in genau einem Topf (Fall 16), ANFANG zuerst, AUSGABE im Topf hlg
  B7  esp_heap_log (): heapkarte_heapzeile () in #if ESP_HEAP_MAP nach getMaxFreeBlockSize () und vor snprintf_P/println;
      Format der Heap-Zeile und von "- heap erste anfrage" wie 3.2.41 (AKI.11)
  B8  setup () Zeichen fuer Zeichen wie 3.2.41 (Fall 22, AKI.1 (h))
  B9  Ausgabe nur aus loop (): HEAPKARTE_AUSGABE und HEAPKARTE_ANFANG je genau einmal in der .ino; kein anderer Aufruf
      einer heapkarte_-Funktion in der .ino ausser heapkarte_heapzeile () (einmal)
  B10 Schalter: ESP_HEAP_MAP 1 in heapkarte.h, #else mit vier leeren Makros; in der .ino #error bei ESP_HEAP_MAP && !
      ESP_HEAP_LOG. Review 1.7 S1: HEAPKARTE_VOR () auch im aktiven Zweig leer (liest nicht); K5: ESP.getFreeHeap ()
      genau einmal in heapkarte.h (NACH) und einmal in heapkarte.cpp (heapkarte_anfang ()), zehnmal NACH in loop ()
  B11 ueberholt (N2): keine Zeile "- heap start", keine Karte U; hk_marken_erfassen () genau einmal aufgerufen
  B12 http.cpp: heapkarte.h eingebunden; http_accept_count_get () liefert http_accept_count, in #if ESP_HEAP_MAP;
      HTTP_TW_LIMIT 2 und tcp_tw_limit () unveraendert
  B13 Konstanten: W 60000 (Ent-C5 = A), 180000, X-Abstand 400, Takt 1000
  B14 neue Dateien reines ASCII mit CRLF (DIR-015)
"""
import hashlib, os, re, sys
q = sys.argv[1]; bez = sys.argv[2] if len(sys.argv) > 2 else q
SETUP_SHA = '6990ea6fe68579b058c6cbeae1adf4f1579adb98f8ee2a6214e674452b61c974'      # setup () aus 3.2.41, LF
HEAPZEILE = 'PSTR ("- heap free=%lu max=%lu ovr=%lu low=%lu %s nrt=%u")'
ERSTE = 'PSTR ("- heap erste anfrage free=%lu max=%lu")'

def roh(name):
    p = os.path.join(q, name)
    return open(p, 'rb').read() if os.path.exists(p) else None
def text(b):
    return b.decode('utf-8', 'replace').replace('\r\n', '\n') if b is not None else ''
def code(s):
    n0 = s.count('\n') + 1
    s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
    s = re.sub(r"'(?:\\.|[^'\\\n])'", "''", s)
    s = re.sub(r'"(?:\\.|[^"\\\n])*"', '""', s)
    L = [re.sub(r'//.*', '', l) for l in s.split('\n')]
    if len(L) != n0:
        print(f'KAPUTT: Bereinigung veraendert die Zeilenzahl ({n0} -> {len(L)})'); sys.exit(2)
    return L
def ohne_komm(s):
    """Kommentare weg, Zeichenketten bleiben (fuer Formatvergleiche)"""
    s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
    return [re.sub(r'\s*//.*', '', l) for l in s.split('\n')]
def funktion(L, kopf):
    i = next((k for k, l in enumerate(L) if l.startswith(kopf)), None)
    if i is None: return []
    j = next((k for k in range(i, len(L)) if L[k].startswith('}')), len(L) - 1)
    return L[i:j + 1]

Hb, Cb, Ib, Pb = roh('heapkarte.h'), roh('heapkarte.cpp'), roh('ESP-uclock.ino'), roh('http.cpp')
Hr, Cr, Ir, Pr = text(Hb), text(Cb), text(Ib), text(Pb)
H, C, I, P = code(Hr), code(Cr), code(Ir), code(Pr)
Ik = ohne_komm(Ir)
fails = []; n = 0
def chk(name, ok):
    global n; n += 1
    if not ok: fails.append(name)

cj = '\n'.join(C); hj = '\n'.join(H); ij = '\n'.join(I); pj = '\n'.join(P)
# B1
chk('B1 heapkarte.cpp vorhanden', Cb is not None and Hb is not None)
chk('B1 #error ohne ARDUINO_ESP8266_RELEASE_3_1_2', re.search(r'#if\s*!\s*defined\s*\(\s*ARDUINO_ESP8266_RELEASE_3_1_2\s*\)\s*\n\s*#error', cj) is not None)
chk('B1 static_assert sizeof (umm_heap_context_t) == 52', re.search(r'static_assert\s*\(\s*sizeof\s*\(\s*umm_heap_context_t\s*\)\s*==\s*52\b', cj) is not None)
chk('B1 static_assert offsetof (umm_heap_context_t, numblocks) == 48', re.search(r'static_assert\s*\(\s*offsetof\s*\(\s*umm_heap_context_t\s*,\s*numblocks\s*\)\s*==\s*48\b', cj) is not None)
chk('B1 #error bei UMM_NUM_HEAPS != 1', re.search(r'#if\s+UMM_NUM_HEAPS\s*!=\s*1\s*\n\s*#error', cj) is not None)
inc = [k for k, l in enumerate(C) if re.match(r'\s*#\s*include\b', l)]
chk('B1 umm_local.h letzter Include', bool(inc) and 'umm_local.h' in Cr.split('\n')[inc[-1]])
if inc:
    nach = [l.strip() for l in C[inc[-1] + 1:inc[-1] + 4]]
    chk('B1 danach #undef memcpy, memmove, memset', nach == ['#undef memcpy', '#undef memmove', '#undef memset'])
# B2
k = funktion(C, 'hk_karte_erfassen (')
kj = '\n'.join(k)
chk('B2 genau ein xt_rsil (15) und ein xt_wsr_ps in heapkarte.cpp', len(re.findall(r'\bxt_rsil\s*\(', cj)) == 1 and len(re.findall(r'\bxt_wsr_ps\s*\(', cj)) == 1 and 'xt_rsil (15)' in kj)
m = re.search(r'ps\s*=\s*xt_rsil\s*\(\s*15\s*\)\s*;(.*?)xt_wsr_ps\s*\(\s*ps\s*\)\s*;', kj, re.S)
zwischen = m.group(1).strip() if m else '?'
chk('B2 zwischen Sperren und Freigeben genau hk_gang (...) ohne Verzweigung', m is not None and re.fullmatch(r'hk_gang\s*\([^;{}?]*\)\s*;', zwischen) is not None
    and not re.search(r'\b(if|return|goto|while|for|switch)\b|&&|\|\|', zwischen))
chk('B2 kein Destruktor (kein RAII)', '~' not in cj.replace('~HK_M_W', '').replace('~HK_M_LAEUFT', '').replace('~(1U', ''))
# B3
chk('B3 EEPROM ueber getConstDataPtr ()', 'EEPROM.getConstDataPtr ()' in cj)
chk('B3 kein getDataPtr ()', re.search(r'\bgetDataPtr\b', cj + hj) is None)
# B4
chk('B4 kein Heap, keine Abgabe im Instrument', re.search(r'\b(String|new|malloc|calloc|realloc|yield|delay|optimistic_yield|esp_yield)\b|std::', cj + hj) is None)
# B5
a = funktion(C, 'heapkarte_ausgabe (void)')
aj = '\n'.join(a)
chk('B5 genau ein Serial.println (line), Serial.flush (), stm32_log_append (line) - alle in heapkarte_ausgabe ()',
    cj.count('Serial.println (line)') == 1 and cj.count('stm32_log_append (line)') == 1 and cj.count('Serial.flush ()') == 1
    and len(re.findall(r'\bSerial\.', cj)) == 2 and len(re.findall(r'\bstm32_log_append\b', cj)) == 1
    and 'Serial.println (line)' in aj and 'stm32_log_append (line)' in aj and 'Serial.flush ()' in aj)
chk('B5 heapkarte_ausgabe (): ein hk_ausgabe () in char line[120], keine Schleife',
    'char    line[120];' in aj and aj.count('hk_ausgabe (') == 1 and re.search(r'\b(for|while)\b', aj) is None)
# B6
L = funktion(I, 'loop()')
stm = []; tiefe = 0; block = None
for l in L[2:-1]:
    s = l.strip()
    if not s or s.startswith('#'):
        if s.startswith('#'): stm.append(s.split()[0] + (' ' + s.split()[1] if len(s.split()) > 1 else ''))
        continue
    if tiefe == 0 and block is None and s.startswith('if ('):
        block = 'OVR-BLOCK'
    tiefe += s.count('{') - s.count('}')
    if block:
        if tiefe == 0 and '}' in s: stm.append(block); block = None
        continue
    if tiefe == 0: stm.append(re.sub(r'\s+', ' ', s))
soll = ['HEAPKARTE_ANFANG ();', 'wifi_check_if_started ();', 'HEAPKARTE_NACH (HK_TOPF_WIF);',
        'HEAPKARTE_VOR ();', 'http_server_loop ();', 'HEAPKARTE_NACH (HK_TOPF_SRV);',
        'HEAPKARTE_VOR ();', 'http_ota_restart_poll ();', 'HEAPKARTE_NACH (HK_TOPF_OTA);',
        'HEAPKARTE_VOR ();', 'update_cache_poll ();', 'HEAPKARTE_NACH (HK_TOPF_UPD);',
        'HEAPKARTE_VOR ();', 'udp_server_loop ();', 'HEAPKARTE_NACH (HK_TOPF_UDP);',
        'HEAPKARTE_VOR ();', 'ntp_poll_time ();', 'HEAPKARTE_NACH (HK_TOPF_NTP);',
        'HEAPKARTE_VOR ();', 'var_sync_check ();', 'HEAPKARTE_NACH (HK_TOPF_VAR);',
        'HEAPKARTE_VOR ();', 'OVR-BLOCK', 'HEAPKARTE_NACH (HK_TOPF_OVR);',
        '#if ESP_HEAP_LOG', 'HEAPKARTE_VOR ();', 'esp_heap_log ();', 'HEAPKARTE_AUSGABE ();', 'HEAPKARTE_NACH (HK_TOPF_HLG);', '#endif',
        'HEAPKARTE_VOR ();', 'stm_bridge_poll ();', 'HEAPKARTE_NACH (HK_TOPF_STM);']
chk('B6 loop (): jeder Aufruf in genau einem Topf, Reihenfolge wie 3.2.41', stm == soll)
if stm != soll:
    for x, (a1, b1) in enumerate(zip(stm + ['<ende>'] * 40, soll + ['<ende>'] * 40)):
        if a1 != b1:
            print(f'    B6 erste Abweichung an Stelle {x}: ist "{a1}", soll "{b1}"'); break
# B7
e = funktion(Ik, 'esp_heap_log (void)')
ej = '\n'.join(e)
p_max = ej.find('getMaxFreeBlockSize'); p_hz = ej.find('heapkarte_heapzeile ();'); p_sn = ej.find('snprintf_P (line'); p_pr = ej.find('Serial.println (line)')
chk('B7 heapkarte_heapzeile () genau einmal, nach max= und vor snprintf_P/println', ej.count('heapkarte_heapzeile') == 1 and -1 < p_max < p_hz < p_sn < p_pr
    and re.search(r'#if ESP_HEAP_MAP\s*\n\s*heapkarte_heapzeile \(\);\s*\n\s*#endif', ej) is not None)
chk('B7 Format der Heap-Zeile wie 3.2.41 (AKI.11)', ej.count(HEAPZEILE) == 1)
chk('B7 Format "- heap erste anfrage" wie 3.2.41', Pr.count(ERSTE) == 1)
# B8
s = funktion(Ir.split('\n'), 'setup()')
i0 = next((x for x, l in enumerate(Ir.split('\n')) if l.startswith('setup()')), None)
st = '\n'.join(Ir.split('\n')[i0 - 1:i0 - 1 + len(s) + 1]) if i0 else ''
chk('B8 setup () wie 3.2.41', hashlib.sha256(st.encode()).hexdigest() == SETUP_SHA)
# B9
chk('B9 HEAPKARTE_AUSGABE und HEAPKARTE_ANFANG je einmal in der .ino', ij.count('HEAPKARTE_AUSGABE') == 1 and ij.count('HEAPKARTE_ANFANG') == 1)
chk('B9 in der .ino keine heapkarte_-Funktion ausser heapkarte_heapzeile () (einmal)', re.findall(r'\bheapkarte_\w+', ij) == ['heapkarte_heapzeile'])
# B10
chk('B10 ESP_HEAP_MAP 1 in heapkarte.h', re.search(r'^#define ESP_HEAP_MAP\s+1\b', hj, re.M) is not None)
els = hj.split('#else')[-1] if '#else' in hj else ''
chk('B10 #else mit vier leeren Makros', all(re.search(r'#define ' + mm + r'\s+do \{ \} while \(0\)', els) for mm in (r'HEAPKARTE_ANFANG\(\)', r'HEAPKARTE_VOR\(\)', r'HEAPKARTE_NACH\(t\)', r'HEAPKARTE_AUSGABE\(\)')))
chk('B10 #error bei ESP_HEAP_MAP && ! ESP_HEAP_LOG in der .ino', re.search(r'#if ESP_HEAP_MAP && ! ESP_HEAP_LOG\s*\n\s*#error', ij) is not None)
aktiv = hj.split('#else')[0] if '#else' in hj else hj
chk('B10 S1: HEAPKARTE_VOR () im aktiven Zweig leer (liest nicht)', re.search(r'#define HEAPKARTE_VOR\(\)\s+do \{ \} while \(0\)', aktiv) is not None)
chk('B10 K5: getFreeHeap einmal in heapkarte.h, einmal in heapkarte.cpp, zehnmal NACH in loop ()',
    len(re.findall(r'\bgetFreeHeap\b', hj)) == 1 and len(re.findall(r'\bgetFreeHeap\b', cj)) == 1
    and sum(l.strip().startswith('HEAPKARTE_NACH (') for l in funktion(I, 'loop()')) == 10)
# B11
alles = Hr + Cr + Ir + Pr
chk('B11 keine Zeile "- heap start", keine Karte U', '- heap start' not in alles and "'U'" not in cj + ij and 'karte U' not in alles)
chk('B11 hk_marken_erfassen () genau einmal aufgerufen', len(re.findall(r'\bhk_marken_erfassen \(&', cj)) == 1)
# B12
chk('B12 http.cpp bindet heapkarte.h ein', any(re.match(r'#include "heapkarte\.h"', l) for l in Pr.split('\n')))
g = funktion(P, 'http_accept_count_get (void)')
gi = next((x for x, l in enumerate(P) if l.startswith('http_accept_count_get (void)')), None)
chk('B12 http_accept_count_get () liefert http_accept_count, in #if ESP_HEAP_MAP', gi is not None and 'return http_accept_count;' in '\n'.join(g)
    and any(l.startswith('#if ESP_HEAP_MAP') for l in P[max(0, gi - 8):gi]) and any(l.startswith('#endif') for l in P[gi:gi + 8]))
chk('B12 TIME_WAIT-Begrenzung unveraendert', re.search(r'#define HTTP_TW_LIMIT\s+2\b', pj) is not None and 'TCP_TW_LIMIT (HTTP_TW_LIMIT - 1);' in pj)
# B13
chk('B13 W 60000, Ereignisse 180000, X 400, Takt 1000', all(re.search(r, hj, re.M) for r in (r'^#define HK_W_MS\s+60000UL\b', r'^#define HK_EREIGNIS_MS\s+180000UL\b', r'^#define HK_X_ABSTAND\s+400\b', r'^#define HK_TAKT_MS\s+1000UL\b')))
# B14
for nm, b in (('heapkarte.h', Hb), ('heapkarte.cpp', Cb)):
    chk(f'B14 {nm} ASCII und nur CRLF', b is not None and all(c < 128 for c in b) and b.count(b'\n') == b.count(b'\r\n') and b.count(b'\r') == b.count(b'\r\n'))

for f in fails:
    print('FEHLER c80i statisch: ' + f)
print(f'  c80i statisch ({bez}): {n} Pruefungen, {len(fails)} FEHL')
sys.exit(1 if fails else 0)
