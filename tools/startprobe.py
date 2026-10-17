#!/usr/bin/env python3
"""Startprobe zu C80 (BEFUNDE L499; Spec specs/paket-2026-10-17-c80, AKV.1): Haengt die bleibende Heap-Zerstueckelung
am fruehen Verkehr nach dem Start?

Aufruf:  tools/startprobe.py <arme> [--abnahme <soll>] [--folge-ab <s>]   z. B. ABE  oder  E8,E12,R  (Reihenfolge = Lauffolge)
  A = local_esp_restart, sofort abfragen     B = local_esp_restart, ruhig bis +30 s nach (FIRMWARE ...)
  D = OTA auf dasselbe Abbild, sofort        C = OTA auf dasselbe Abbild, ruhig bis +30 s
  E = local_esp_restart; bis zur Startzeile NUR den Mitschnitt lesen, keine Verbindung zur Uhr; danach genau EINE
      device_ready-Anfrage (Abstand zur Startzeile ueber 5,9 s => Lauf ungueltig). E<n> (z. B. E12): die eine Anfrage
      erst <n> s nach der Startzeile (ungueltig ab <n>+3 s) -- grenzt den Zeitpunkt der Hintergrundbelegung ein (V.3)
  R = wie E, aber statt der Anfrage nach 2 s nur eine Verbindung aufbauen und per RST abbrechen (SO_LINGER 0), ohne
      Daten: auf der Uhr bleibt kein PCB in TIME_WAIT (Probe des Verdraengungsmodells aus V.3)
"Sofort" (A, D): device_ready alle 0,3 s, bis das Geraet einmal NICHT antwortet (alte Firmware weg) und danach
antwortet; mitgeschrieben werden die Zahl der Versuche und aus dem Rumpf der ersten Antwort http_accept_dead,
tcp_now_pending, tcp_now_unclaimed, tcp_now_list.
Danach in allen Armen ab +30 s (--folge-ab, Stufe 2 der Spec C80: 150 nach Ent-C5) dieselbe Folge smoke-device.sh,
check-pwa.sh; Ruhewert ist die erste Heap-Zeile
>= 150 s danach mit tcp=0/0/0/0/0/0. Gemessen 17.10.2026 auf ESP 3.2.41: A und D max 6'600, B und C 7'104 (3/3).

Exit 0 alle Laeufe vollstaendig (und mit --abnahme: kein Lauf unter dem Soll); 1 ein Lauf unvollstaendig, ungueltig
oder unter dem Soll -- die Meldung nennt Lauf und Grund; 2 Aufruf- oder Werkzeugfehler.
Jede Meldung geht auf die Konsole UND in die Mitschrift (STARTPROBE_MITSCHRIFT, sonst .git/startprobe.log).

SCHREIBT AM GERAET: startet den ESP neu bzw. loest ein OTA aus, je Lauf einmal. Logwache parallel (DIR-013).
Gegenproben ohne Geraet (DIR-014): STARTPROBE_LOG ersetzt tools/logger/log.sh (Attrappe, antwortet auf tail/grep/mark),
STARTPROBE_HOST die Geraeteadresse, STARTPROBE_FRIST die Wartezeit auf die Startzeile (Standard 420 s),
STARTPROBE_OHNE_FOLGE=1 laesst Smoketest und check-pwa weg (sie sprechen sonst das echte Geraet an).
Zwei Fallen, die das Werkzeug umgeht: local_esp_restart antwortet nicht und blockierte bis zur Frist (deshalb im
eigenen Faden), und Abfragezeilen schieben die Startzeile aus einem kurzen tail (deshalb log.sh grep).
"""
import json, os, re, subprocess, sys, threading, time, datetime as D, urllib.request

R = subprocess.run(['git', '-C', os.path.dirname(os.path.abspath(__file__)), 'rev-parse', '--show-toplevel'],
                   capture_output=True, text=True).stdout.strip()
GITDIR = subprocess.run(['git', '-C', R, 'rev-parse', '--absolute-git-dir'], capture_output=True, text=True).stdout.strip()
LOGSH = os.environ.get('STARTPROBE_LOG') or R + '/tools/logger/log.sh'
MITSCHRIFT = os.environ.get('STARTPROBE_MITSCHRIFT') or GITDIR + '/startprobe.log'
FRIST = int(os.environ.get('STARTPROBE_FRIST', '420'))
RX = re.compile(r'^(\S+)\s+\(- heap free=(\d+) max=(\d+) .*tcp=([\d/]+)')
FELDER = ('http_accept_dead', 'tcp_now_pending', 'tcp_now_unclaimed', 'tcp_now_list')


def host_lesen():
    if os.environ.get('STARTPROBE_HOST'):
        return os.environ['STARTPROBE_HOST']
    for z in open(R + '/tools/device.conf'):
        m = re.match(r'\s*DEVICE_HOST=["\']?([^"\'\s]+)', z)
        if m:
            return m.group(1)
    raise SystemExit('DEVICE_HOST fehlt (tools/device.conf)')


def log(s):
    z = time.strftime('%H:%M:%S') + ' ' + s
    print(z, flush=True)
    with open(MITSCHRIFT, 'a') as f:
        f.write(z + '\n')


def logsh(*a):
    return subprocess.run([LOGSH, *map(str, a)], capture_output=True, text=True, cwd=R).stdout.splitlines()


def ts(l):
    try:
        return D.datetime.fromisoformat(l.split()[0])
    except Exception:
        return None


def suche(m, n=400):
    # gezielt im Mitschnitt statt im Ende: Abfragezeilen schoben die Startzeile aus tail 400
    return sorted(logsh('grep', m, n))


def jetzt():
    return D.datetime.now().astimezone()


def get(p, t=2):
    try:
        return urllib.request.urlopen(f'http://{HOST}{p}', timeout=t).read()
    except Exception:
        return None


def ruhe(nach, sek):
    tend = time.time() + 700
    while time.time() < tend:
        z = [l for l in logsh('tail', 400) if ts(l) and RX.match(l) and (ts(l) - nach).total_seconds() >= sek
             and RX.match(l).group(4) == '0/0/0/0/0/0']
        if z:
            m = RX.match(z[0])
            return int(m.group(2)), int(m.group(3)), m.group(4)
        time.sleep(20)
    return None


def rst_verbindung():
    import socket, struct
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(3)
    try:
        h, _, port = HOST.partition(':')
        s.connect((h, int(port or 80)))
        s.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
        return True
    except Exception:
        return False
    finally:
        s.close()


def lauf(nr, arm):
    verzug = int(arm[1:]) if len(arm) > 1 else None
    arm0 = arm[0]
    ota, sofort, einmal, rst = arm0 in 'CD', arm0 in 'AD', arm0 in 'ER', arm0 == 'R'
    if rst and verzug is None:
        verzug = 2
    e = {'lauf': nr, 'arm': arm, 'fehlt': []}
    logsh('mark', f'startprobe: Lauf {nr} Arm {arm}')
    t0 = jetzt()
    log(f'== Lauf {nr}, Arm {arm} ({"OTA" if ota else "Neustart"}, '
        f'{"sofort" if sofort else ("RST-Verbindung" if rst else "eine Anfrage") + " nach der Startzeile" + (f" +{verzug} s" if verzug else "") if einmal else f"ab +{FOLGE_AB} s"})')
    threading.Thread(target=get, args=('/api/remote_esp_update' if ota else '/api/local_esp_restart', 30),
                     daemon=True).start()
    if sofort:
        weg, versuche, rumpf = False, 0, None
        while (jetzt() - t0).total_seconds() < FRIST:
            versuche += 1
            r = get('/api/device_ready', 1)
            if r is None:
                weg = True
            elif weg:
                rumpf = r
                break
            time.sleep(0.3)
        e['versuche'] = versuche
        if rumpf is None:
            e['fehlt'].append('erste Antwort der neuen Firmware' + ('' if weg else ' (alte Firmware nie weg)'))
        else:
            try:
                d = json.loads(rumpf)
                e['erste_antwort'] = {k: d.get(k) for k in FELDER}
            except Exception:
                e['fehlt'].append('Rumpf der ersten Antwort nicht lesbar')
    tf = None
    while tf is None and (jetzt() - t0).total_seconds() < FRIST:
        # Arm E/R: schneller nachsehen, damit die eine Anfrage sicher unter 3,2 s nach der Startzeile liegt (N2)
        time.sleep(0.5 if einmal else 2)
        z = [ts(l) for l in suche('(FIRMWARE ') if ts(l) and ts(l) > t0]
        if z:
            tf = z[0]
    if tf is None:
        e['fehlt'].append(f'Startzeile (FIRMWARE ...) binnen {FRIST} s')
        return e
    e['start'] = tf.strftime('%H:%M:%S')
    if einmal:
        if verzug:
            while (jetzt() - tf).total_seconds() < verzug:
                time.sleep(0.1)
        if rst:
            ok = rst_verbindung()
        else:
            ok = get('/api/device_ready', 5) is not None
        e['eine_anfrage_s'] = round((jetzt() - tf).total_seconds(), 1)
        if not ok:
            e['fehlt'].append('Verbindung fuer den RST-Abbruch' if rst else 'Antwort auf die eine Anfrage')
        grenze = (verzug + 3) if verzug else 3.2   # N2: Arm E gilt nur mit der Anfrage bis 3,2 s (vorher 5,9 s)
        if e['eine_anfrage_s'] < 0:
            e['ungueltig'] = f'eine Anfrage {e["eine_anfrage_s"]} s VOR der Startzeile'
        elif e['eine_anfrage_s'] > grenze:
            e['ungueltig'] = f'eine Anfrage erst {e["eine_anfrage_s"]} s nach der Startzeile (Grenze {grenze} s)'
    while (jetzt() - tf).total_seconds() < FOLGE_AB:
        time.sleep(1)
    if einmal:
        # AKV.2: vom Werkzeug geprueft -- zwischen dem Neustart (rst cause) und der Startzeile keine Anfrage an die Uhr,
        # danach bis +29 s genau eine. Gezaehlt wird jede Anfrage, auch fremde: Ein Browser im Netz macht den Lauf ungueltig.
        z = logsh('since', t0.strftime('%Y-%m-%dT%H:%M'), 6000)
        # Name nicht 'rst': der bezeichnet den Arm R -- die Wiederverwendung machte jeden E-Lauf zum Soll 0 (L501)
        rst_z = [ts(l) for l in z if ts(l) and ts(l) > t0 and 'rst cause' in l][:1]
        req = [ts(l) for l in z if ts(l) and '- request ' in l]
        if not rst_z:
            e['fehlt'].append('Zeile rst cause nach dem Ausloeser')
        else:
            vor = sum(1 for x in req if rst_z[0] < x < tf)
            nach = sum(1 for x in req if tf <= x <= tf + D.timedelta(seconds=29))
            e['anfragen_vor_start'], e['anfragen_bis_30s'] = vor, nach
            soll_nach = 0 if rst else 1          # R schickt keine Daten, also keine Zeile "- request"
            if vor or nach != soll_nach:
                e['ungueltig'] = (e.get('ungueltig', '') + f' Anfragen zwischen Neustart und Startzeile {vor} (Soll 0), '
                                  f'danach bis +29 s {nach} (Soll {soll_nach})').strip()
    # STARTPROBE_OHNE_FOLGE=1 nur fuer Gegenproben mit Attrappen: Smoketest und check-pwa lesen tools/device.conf
    # und wuerden sonst das echte Geraet ansprechen.
    for c in ([] if os.environ.get('STARTPROBE_OHNE_FOLGE') else (['./tools/smoke-device.sh'], ['./tools/check-pwa.sh'])):
        p = subprocess.run(c, capture_output=True, text=True, cwd=R)
        log(f'  {c[0]}: Exit {p.returncode}')
    ea = [l for l in suche('heap erste anfrage') if ts(l) and ts(l) > tf][:1]
    if ea:
        e['erste_anfrage_s'] = round((ts(ea[0]) - tf).total_seconds(), 1)
        e['erste_anfrage'] = ea[0].split('(', 1)[-1].rstrip(')')
    else:
        e['fehlt'].append('Zeile "- heap erste anfrage"')
    rw = ruhe(jetzt(), 150)
    if rw:
        e['free'], e['max'], e['tcp'] = rw
    else:
        e['fehlt'].append('Ruhewert (Heap-Zeile >= 150 s danach mit tcp=0)')
    return e


def main():
    global HOST
    a = sys.argv[1:]
    soll = None
    global FOLGE_AB
    if '--folge-ab' in a:
        i = a.index('--folge-ab')
        try:
            FOLGE_AB = int(a[i + 1])
        except (IndexError, ValueError):
            print(__doc__.split('\n\n')[1], file=sys.stderr)
            return 2
        del a[i:i + 2]
    if '--abnahme' in a:
        i = a.index('--abnahme')
        try:
            soll = int(a[i + 1])
        except (IndexError, ValueError):
            print(__doc__.split('\n\n')[1], file=sys.stderr)
            return 2
        del a[i:i + 2]
    if len(a) != 1 or not a[0]:
        print(__doc__.split('\n\n')[1], file=sys.stderr)
        return 2
    arme = a[0].split(',') if (',' in a[0] or re.search(r'\d', a[0])) else list(a[0])
    if not all(re.fullmatch(r'[ABCD]|[ER]\d*', x) for x in arme):
        print(__doc__.split('\n\n')[1], file=sys.stderr)
        return 2
    HOST = host_lesen()
    log(f'=== Startprobe {a[0]}' + (f' --abnahme {soll}' if soll is not None else '') + f', Folge ab +{FOLGE_AB} s, Mitschrift {MITSCHRIFT}')
    erg = [lauf(n + 1, arm) for n, arm in enumerate(arme)]
    logsh('mark', 'startprobe: Ende')
    fehl = 0
    log('=== Ergebnis')
    for e in erg:
        log('  ' + json.dumps({k: v for k, v in e.items() if k != 'fehlt'}, ensure_ascii=False))
        for f in e['fehlt']:
            log(f'  FEHLT Lauf {e["lauf"]} Arm {e["arm"]}: {f}')
            fehl += 1
        if e.get('ungueltig'):
            log(f'  UNGUELTIG Lauf {e["lauf"]} Arm {e["arm"]}: {e["ungueltig"]}')
            fehl += 1
        if soll is not None and e.get('max') is not None and e['max'] < soll:
            log(f'  FEHL  Lauf {e["lauf"]} Arm {e["arm"]}: max {e["max"]} unter dem Soll {soll}')
            fehl += 1
    log(f'=== {len(erg)} Laeufe, {fehl} Abweichung(en)')
    return 1 if fehl else 0


HOST = None
FOLGE_AB = 30
if __name__ == '__main__':
    sys.exit(main())
