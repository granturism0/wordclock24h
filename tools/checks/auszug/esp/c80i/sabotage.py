#!/usr/bin/env python3
"""Sabotagen fuer c80i (Paket 2026-10-17-c80, Task 1.3; design.md 4.8 samt N1/N2; AKI.13 (a), AKI.14 (a)): jede muss
FEHL melden (DIR-014) - nicht OK und nicht KAPUTT. Aufruf: sabotage.py <ESP-Quellverzeichnis>. Exit 0, wenn JEDE
Sabotage anschlaegt. Anker sind zeilenendenneutral (\\r?\\n), jeder muss genau so oft treffen wie angegeben.
Ein optionales drittes Element nennt Meldungen (Teiltexte), die unter den FEHLER-Zeilen stehen MUESSEN: Dann genuegt
nicht, dass irgendeine Pruefung anschlaegt - es muss die gemeinte sein (Review 1.7 S1)."""
import os, re, shutil, subprocess, sys, tempfile
D = os.path.dirname(os.path.abspath(__file__)); Q = os.path.abspath(sys.argv[1])
K, H, I, P = 'heapkarte.cpp', 'heapkarte.h', 'ESP-uclock.ino', 'http.cpp'
S = [
  # --- design.md 4.8 (Grundstock)
  ('nur Serial', [(K, '        Serial.flush ();\n        stm32_log_append (line);\n', '        Serial.flush ();\n', 1)]),
  ('nur Ring', [(K, '        Serial.println (line);\n        Serial.flush ();\n', '', 1)]),
  ('Schranke Blockzahl entfernt', [(K, '        if (nb >= numblocks)\n        {\n            return k->abbruch = HK_AB_NB;\n        }\n', '', 1)]),
  ('Zeile ueber 119', [(K, 'PSTR (" ent=%u/%u fsum=%lu")', 'PSTR (" eintraege=%u/%u fsum=%lu")', 1), (K, '    char    line[120];', '    char    line[200];', 1)]),
  ('String statt snprintf', [(K, '    char    line[120];                                                      // STM32_LOG_LINE_LEN: hoechstens 119 Zeichen\n',
                              '    char    line[120];                                                      // STM32_LOG_LINE_LEN: hoechstens 119 Zeichen\n    String  s = "x";\n', 1)]),
  ('Ausgabe in setup ()', [(I, '    Serial.setRxBufferSize (1024);\n', '    Serial.setRxBufferSize (1024);\n    HEAPKARTE_AUSGABE ();\n', 1)]),
  ('zwei Zeilen im selben Durchlauf (Takt nicht gesetzt)', [(K, '    z->takt     = jetzt;\n    return hk_zeile (z, bit, buf, size);', '    return hk_zeile (z, bit, buf, size);', 1)]),
  ('davor/danach vertauscht', [(K, 'PSTR (" davor="), k->davor_b, k->davor_n', 'PSTR (" davor="), k->DANACH_B, k->DANACH_N', 1),
                               (K, 'PSTR (" danach="), k->danach_b, k->danach_n', 'PSTR (" danach="), k->davor_b, k->davor_n', 1),
                               (K, 'k->DANACH_B, k->DANACH_N', 'k->danach_b, k->danach_n', 1)]),
  ('Sperre auf dem abbruch-Weg nicht geloest', [(K, '    ps = xt_rsil (15);\n    hk_gang ((const void *) c->heap, (uint16_t) c->numblocks, bs, k);\n    xt_wsr_ps (ps);\n',
                                                 '    ps = xt_rsil (15);\n    if (! hk_gang ((const void *) c->heap, (uint16_t) c->numblocks, bs, k))\n    {\n        xt_wsr_ps (ps);\n    }\n', 1)]),
  ('Gleichstand zugunsten des groesseren Index', [(K, '            if (len > k->gross_n)', '            if (len >= k->gross_n)', 1)]),
  # --- N1
  ('Laengen als 8n - 4', [(K, '(unsigned long) n * 8UL);', '(unsigned long) n * 8UL - 4);', 1)]),
  ('Waechter sum entfernt', [(K, '    if (summe != (uint32_t) numblocks - 2)', '    if (summe != summe)', 1)]),
  ('getDataPtr () statt getConstDataPtr ()', [(K, 'EEPROM.getConstDataPtr ()', 'EEPROM.getDataPtr ()', 1)]),
  ('#error-Bindung entfernt', [(K, '#if ! defined (ARDUINO_ESP8266_RELEASE_3_1_2)\n#error ', '#if 0\n#error ', 1)]),
  # --- N2
  # S1: VOR liest nicht, ein Aufruf vor VOR bleibt im selben Topf - nur die Reihenfolge (B6) sieht ihn
  ('Aufruf vor VOR verschoben (srv, nur Reihenfolge)', [(I, '    HEAPKARTE_VOR ();\n    http_server_loop ();\n', '    http_server_loop ();\n    HEAPKARTE_VOR ();\n', 1)], ('B6 loop ()',)),
  ('Messung nach W fortgesetzt', [(K, '            z->merker  &= (uint8_t) ~HK_M_W;\n', '', 1)]),
  ('Topf sys weggelassen', [(K, '        hk_topf_buchen (&z->topf[HK_TOPF_SYS], (int32_t) frei - (int32_t) z->letzter, hk_ms16 (ms));\n', '', 1)]),
  ('ms beim ersten statt beim groessten Rueckgang', [(K, '        t->rueckgang    = rueck > 0xFFFFUL ? (uint16_t) 0xFFFF : (uint16_t) rueck;\n        t->ms           = ms;\n',
                                                      '        t->ms           = t->rueckgang ? t->ms : ms;\n        t->rueckgang    = rueck > 0xFFFFUL ? (uint16_t) 0xFFFF : (uint16_t) rueck;\n', 1)]),
  ('X ohne Pruefung der aktiven Verbindungen', [(K, ' && aktive == 0 && ', ' && ', 1)]),
  ('X ohne Abstand 400', [(K, '(uint32_t) frei + HK_X_ABSTAND < z->free0', '(uint32_t) frei < z->free0', 1)]),
  ('X nach W', [(K, ' && (z->merker & HK_M_W) && aktive', ' && aktive', 1)]),
  ('T bei jedem Rueckgang der TIME_WAIT-Zahl', [(K, ' && z->tw_vor && ! tw)', ' && tw < z->tw_vor)', 1)]),
  ('T nach 180 s', [(H, '#define HK_EREIGNIS_MS              180000UL', '#define HK_EREIGNIS_MS              1800000UL', 1)]),
  ('F im annehmenden Durchlauf', [(K, '                z->merker |= HK_M_ANGENOMMEN;\n', '                z->merker |= HK_M_ANGENOMMEN;\n                e |= HK_E_F;\n', 1)]),
  ('eine Ausgabe in W', [(K, '    if ((z->merker & (HK_M_GESTARTET | HK_M_W)) != HK_M_GESTARTET || ! (z->offen & HK_O_ALLE))', '    if (! (z->merker & HK_M_GESTARTET) || ! (z->offen & HK_O_ALLE))', 1),
                         (K, '    uint16_t    frei_geben = (z->merker & HK_M_W) ? 0 : 0xFFFF;', '    uint16_t    frei_geben = 0xFFFF;', 1)]),
  ('Wegmarken zweimal je Start', [(K, '            if (! (z->merker & HK_M_MARKEN))                                // X blieb aus: Wegmarken jetzt (Anlass W)\n            {\n                e |= HK_E_MARKEN;\n            }\n',
                                  '            e |= HK_E_MARKEN;\n', 1)]),
  # --- vom Lead fuer 1.3 genannt
  ('Summenprobe verletzt (letzter nach NACH nicht nachgefuehrt)', [(K, '    hk_topf_buchen (&z->topf[topf], (int32_t) frei - (int32_t) z->letzter, z->ms_lauf);\n    z->letzter = frei;\n',
                                                                   '    hk_topf_buchen (&z->topf[topf], (int32_t) frei - (int32_t) z->letzter, z->ms_lauf);\n', 1)]),
  ('falscher Blockindex im Gang (Folgeblock statt Eintrag)', [(K, '                k->gross_b  = b;\n', '                k->gross_b  = nb;\n', 1)]),
  ('falscher Blockindex der Wegmarken (/ 4 statt / 8)', [(K, '    return (uint16_t) ((size_t) (q - h) / sizeof (HK_BLOCK));', '    return (uint16_t) ((size_t) (q - h) / 4);', 1)]),
  ('Endwaechter fehlt (Gang erkennt next == 0 nicht als Ende)', [(K, '        if (nb == 0)                                                        // b ist der Endwaechter', '        if (nb == 0 && b == 0)', 1)]),
  ('Sperre fehlt ganz', [(K, '    ps = xt_rsil (15);\n', '    ps = 0x1234;\n', 1)]),
  # --- weitere
  ('Waechter ord entfernt', [(K, '        if (nb <= b)\n        {\n            return k->abbruch = HK_AB_ORD;\n        }\n', '', 1)]),
  ('Waechter it entfernt', [(K, '        if (++schritte > numblocks)\n        {\n            return k->abbruch = HK_AB_IT;\n        }\n', '        ++schritte;\n', 1)]),
  ('Waechter bs entfernt', [(K, '    if (blockgroesse != sizeof (HK_BLOCK))\n    {\n        return k->abbruch = HK_AB_BS;\n    }\n', '', 1)]),
  ('m-Karte auch in W', [(K, '    return (z->merker & (HK_M_GESTARTET | HK_M_W)) == HK_M_GESTARTET;', '    return (z->merker & HK_M_GESTARTET) != 0;', 1)]),
  ('Karte m nicht im Fenster der Heap-Zeile', [(I, '#if ESP_HEAP_MAP\n        heapkarte_heapzeile ();', '#if 0\n        heapkarte_heapzeile ();', 1),
                                             (I, '        tcp_now_tcp_field (tcp, sizeof (tcp));\n', '        tcp_now_tcp_field (tcp, sizeof (tcp));\n#if ESP_HEAP_MAP\n        heapkarte_heapzeile ();\n#endif\n', 1)]),
  ('Format der Heap-Zeile geaendert', [(I, 'PSTR ("- heap free=%lu max=%lu ovr=%lu low=%lu %s nrt=%u")', 'PSTR ("- heap free=%lu max=%lu ovr=%lu low=%lu %s nrt=%u k=1")', 1)]),
  ('Startfenster 25 s statt 60 s (Ent-C5 = B)', [(H, '#define HK_W_MS                     60000UL', '#define HK_W_MS                     25000UL', 1)]),
  ('F liest den falschen Zaehler', [(P, '    return http_accept_count;\n}\n#endif', '    return http_accept_dead;\n}\n#endif', 1)]),
  ('Kopf ohne free1 (Summenprobe nicht nachrechenbar)', [(K, ' free0=%u free1=%u us=%u"),', ' free0=%u free1=%u us=%u"),', 1), (K, '(unsigned) z->free0, (unsigned) z->free1, (unsigned) z->us);', '(unsigned) z->free0, (unsigned) z->free0, (unsigned) z->us);', 1)]),
  ('X nimmt free(S) aus dem falschen Feld', [(K, ' < z->free0)\n', ' < z->free1)\n', 1)]),
  # --- Review 1.7 S1, K2, K5
  ('S1 rueckgebaut: VOR liest erneut (Belegung aus der Unterbrechung faellt in keinen Topf)',
   [(H, '#define HEAPKARTE_VOR()             do { } while (0)                       // Marker, liest nicht (S1)\n#define HEAPKARTE_NACH',
        '#define HEAPKARTE_VOR()             do { if (heapkarte.merker & HK_M_W) heapkarte.letzter = (uint16_t) ESP.getFreeHeap (); } while (0)\n#define HEAPKARTE_NACH', 1)],
   ('Szenario Unterbrechung (S1): AKI.13 Summenprobe', 'Szenario E: K5', 'B10 S1')),
  ('Aufruf in die Luecke verschoben (srv nach NACH srv - zaehlt zu ota)', [(I, '    http_server_loop ();\n    HEAPKARTE_NACH (HK_TOPF_SRV);\n', '    HEAPKARTE_NACH (HK_TOPF_SRV);\n    http_server_loop ();\n', 1)],
   ('Szenario E: Fall 15/16 Toepfe',)),
  ('K2 Grenze tcp entfernt', [(K, '    for (; pcb && m->tcp_n < 0xFF; pcb = pcb->next)', '    for (; pcb; pcb = pcb->next)', 1)], ('(K2): abgebrochen',)),
  ('K2 Grenze udp entfernt', [(K, '    for (u = udp_pcbs; u && m->udp_n < 0xFF; u = u->next)', '    for (u = udp_pcbs; u; u = u->next)', 1)], ('(K2): abgebrochen',)),
  ('K2 Grenze lst entfernt', [(K, '    for (l = tcp_listen_pcbs.listen_pcbs, i = 0; l && i < 0xFF; l = l->next, i++)', '    for (l = tcp_listen_pcbs.listen_pcbs; l; l = l->next)', 1)], ('(K2): abgebrochen',)),
  ('K2 Grenze netif entfernt', [(K, '    for (n = netif_list, i = 0; n && i < 0xFF; n = n->next, i++)', '    for (n = netif_list; n; n = n->next)', 1)], ('(K2): abgebrochen',)),
  ('K5 ein zusaetzliches Lesen in W (Wert verworfen)', [(H, '#define HEAPKARTE_AUSGABE()         do { if (heapkarte.offen) heapkarte_ausgabe (); } while (0)\n',
        '#define HEAPKARTE_AUSGABE()         do { if (heapkarte.merker & HK_M_W) (void) ESP.getFreeHeap (); if (heapkarte.offen) heapkarte_ausgabe (); } while (0)\n', 1)],
   ('Szenario E: K5',)),
]
rc = 0
for eintrag in S:
    name, eingriffe = eintrag[0], eintrag[1]; muss = eintrag[2] if len(eintrag) > 2 else ()
    T = tempfile.mkdtemp(prefix='c80isab.')
    q2 = os.path.join(T, 'q'); shutil.copytree(Q, q2, ignore=shutil.ignore_patterns('data', 'build*', '*.bin'))
    kaputt = False
    for f, a, n, cnt in eingriffe:
        p = os.path.join(q2, f); b = open(p, 'rb').read()
        rx = re.compile(b'\r?\n'.join(re.escape(x.encode()) for x in a.split('\n')))
        found = len(rx.findall(b))
        if found != cnt:
            print(f"  ANKER FEHLT ({found} statt {cnt}): {name}: {a[:60]!r}"); kaputt = True; break
        eol = b'\r\n' if b'\r\n' in b and f != P else b'\n'
        b = rx.sub(n.encode().replace(b'\n', eol).replace(b'\\', b'\\\\'), b)
        open(p, 'wb').write(b)
    if kaputt:
        rc = 1; shutil.rmtree(T); continue
    r = subprocess.run(['sh', os.path.join(D, 'run.sh'), q2, 'sab'], capture_output=True, text=True)
    fehler = [l.strip() for l in r.stdout.split('\n') if l.strip().startswith(('FEHLER', 'UEBERSETZUNG'))]
    fehlt = [x for x in muss if not any(x in f for f in fehler)]
    if r.returncode == 1 and fehlt:
        print(f"  SCHLAEGT FALSCH AN: {name} - es fehlt {fehlt} ({len(fehler)} Meldungen, z. B. {fehler[0][:100] if fehler else '-'})"); rc = 1
    elif r.returncode == 1:
        print(f"  schlaegt an: {name}  ({len(fehler)} Meldungen{', darunter ' + ' | '.join(muss) if muss else ''}, z. B. {fehler[0][:120] if fehler else '-'})")
    else:
        last = [l for l in r.stdout.strip().split('\n') if l.strip()][-1:] or ['']
        print(f"  BLIEB {'GRUEN' if r.returncode == 0 else 'KAPUTT'}: {name} (rc={r.returncode}, {last[0][:100]})"); rc = 1
    shutil.rmtree(T)
print(f"{len(S)} Sabotagen, {'alle schlagen an' if rc == 0 else 'NICHT alle schlagen an'}")
sys.exit(rc)
