#!/usr/bin/env python3
"""Sabotagen fuer c72i (Paket 2026-10-15, R1.1; AKI.1, AKI.2; design.md 8): jede muss FEHL melden (DIR-014), nicht OK
und nicht KAPUTT. Aufruf: sabotage.py <ESP-Quellverzeichnis>. Exit 0, wenn JEDE Sabotage anschlaegt.
Anker sind zeilenendenneutral (\\r?\\n), jeder muss genau so oft treffen wie angegeben."""
import os, re, shutil, subprocess, sys, tempfile
D = os.path.dirname(os.path.abspath(__file__)); Q = os.path.abspath(sys.argv[1])
S = [
  # (Name, [(Datei, alt, neu, Anzahl), ...])  -- die sechs aus design.md 8 zuerst
  ('Ausgabe innerhalb der Listenschleife', [('http.cpp',
      '        tcp_now_add (t, pcb, listen_arg);\n    }\n\n    for (pcb = tcp_tw_pcbs;',
      '        tcp_now_add (t, pcb, listen_arg);\n        http_send ("");\n    }\n\n    for (pcb = tcp_tw_pcbs;', 1)]),
  ('Kappung entfernt', [('http.cpp', '#define TCP_NOW_LIST_MAX                        8', '#define TCP_NOW_LIST_MAX                        1024', 1)]),
  ('SYN_RCVD als ClientContext gezaehlt', [('http.cpp', '        return TCP_NOW_ARG_LISTEN;', '        return TCP_NOW_ARG_CLIENT;', 1)]),
  ('toter Abholversuch ohne Zaehler', [('http.cpp', '        http_accept_dead_note (unclaimed_before);                           // R1.1 I-5 (P8): toter Kontext abgeholt\n', '', 1)]),
  ('leerer Durchlauf gezaehlt', [('http.cpp', '    if (unclaimed_before && http_accept_dead < 0xFFFF)', '    if (http_accept_dead < 0xFFFF)', 1)]),
  ('Heap-Zeile nur Serial', [('ESP-uclock.ino', '        stm32_log_append (line);                                            // C14 (L185)', '        // stm32_log_append (line);                                         // C14 (L185)', 1)]),
  # weitere
  ('Listen erst nach der ersten Ausgabe gelesen', [
      ('http.cpp', '    tcp_now_read (&tcp_now);                                        // R1.1 I-5: Listen VOLLSTAENDIG lesen, bevor gesendet wird\n', '', 1),
      ('http.cpp', '    http_send (FS("{\\"ok\\":true,\\"ready\\":true"));\n    http_json_send_uint_field (FS("free_heap"), free_heap);',
                   '    http_send (FS("{\\"ok\\":true,\\"ready\\":true"));\n    tcp_now_read (&tcp_now);\n    http_json_send_uint_field (FS("free_heap"), free_heap);', 1)]),
  ('Abgabe im Listengang (yield)', [('http.cpp', '        tcp_now_add (t, pcb, listen_arg);\n    }\n\n    t->pending', '        tcp_now_add (t, pcb, listen_arg);\n        yield ();\n    }\n\n    t->pending', 1)]),
  ('ruhe_s in 32 Bit gerechnet', [('http.cpp', '(((uint64_t) (u32_t) (tcp_ticks - pcb->tmr)', '(((uint32_t) (u32_t) (tcp_ticks - pcb->tmr)', 1)]),
  ('ruhe_s ungesaettigt', [('http.cpp', '    e->ruhe_s = (uint16_t) (ruhe > 0xFFFF ? 0xFFFF : ruhe);', '    e->ruhe_s = (uint16_t) ruhe;', 1)]),
  ('unclaimed () ungesaettigt', [('http.cpp', '            while (ctx && n < 0xFF)', '            while (ctx)', 1)]),
  ('unclaimed () nur lebende Kontexte', [('http.cpp', '                n++;\n                ctx = ctx->next ();', '                n += ((ClientContext *) ctx)->getPCB () ? 1 : 0;\n                ctx = ctx->next ();', 1)]),
  ('TIME_WAIT nicht in der Liste', [('http.cpp', '            t->tw++;\n        }\n\n        tcp_now_add (t, pcb, listen_arg);', '            t->tw++;\n        }', 1)]),
  ('nrt= aus no_request_aborts', [('http.cpp', '    return http_no_request_timeouts;\n}', '    return http_no_request_aborts;\n}', 1)]),
  ('tcp= ohne CLOSE_WAIT (LAST_ACK doppelt)', [('http.cpp', '(unsigned) t.states[CLOSE_WAIT], (unsigned) t.states[LAST_ACK]', '(unsigned) t.states[LAST_ACK], (unsigned) t.states[LAST_ACK]', 1)]),
  ('Heap-Zeile ohne tcp=/nrt= (Stand 3.2.37)', [('ESP-uclock.ino', 'PSTR ("- heap free=%lu max=%lu ovr=%lu low=%lu %s nrt=%u")', 'PSTR ("- heap free=%lu max=%lu ovr=%lu low=%lu")', 1)]),
  ('Heap-Zeilenpuffer zu klein (80)', [('ESP-uclock.ino', '        char            line[120];', '        char            line[80];', 1)]),
]
rc = 0
for name, eingriffe in S:
    T = tempfile.mkdtemp(prefix='c72sab.')
    q2 = os.path.join(T, 'q'); shutil.copytree(Q, q2, ignore=shutil.ignore_patterns('data', 'build*', '*.bin'))
    kaputt = False
    for f, a, n, cnt in eingriffe:
        p = os.path.join(q2, f); b = open(p, 'rb').read()
        rx = re.compile(b'\r?\n'.join(re.escape(x.encode()) for x in a.split('\n')))
        found = len(rx.findall(b))
        if found != cnt:
            print(f"  ANKER FEHLT ({found} statt {cnt}): {name}"); kaputt = True; break
        b = rx.sub(n.encode().replace(b'\\', b'\\\\'), b)
        open(p, 'wb').write(b)
    if kaputt:
        rc = 1; shutil.rmtree(T); continue
    r = subprocess.run(['sh', os.path.join(D, 'run.sh'), q2, 'sab'], capture_output=True, text=True)
    fehler = [l.strip() for l in r.stdout.split('\n') if l.strip().startswith('FEHLER')]
    if r.returncode == 1: print(f"  schlaegt an: {name}  ({len(fehler)} Meldungen, z. B. {fehler[0][:110] if fehler else '-'})")
    else:
        last = [l for l in r.stdout.strip().split('\n') if l.strip()][-1:] or ['']
        print(f"  BLIEB {'GRUEN' if r.returncode == 0 else 'KAPUTT'}: {name} (rc={r.returncode}, {last[0][:100]})"); rc = 1
    shutil.rmtree(T)
print(f"{len(S)} Sabotagen, {'alle schlagen an' if rc == 0 else 'NICHT alle schlagen an'}")
sys.exit(rc)
