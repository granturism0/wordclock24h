#!/usr/bin/env python3
"""Startprobe zu C80 (BEFUNDE L499): Haengt die bleibende Heap-Zerstueckelung am fruehen Verkehr nach dem Start?

Aufruf:  tools/startprobe.py <arme>      z. B. ABDCBA (Reihenfolge = Lauffolge)
  A = local_esp_restart, sofort abfragen    B = local_esp_restart, ruhig bis +30 s nach (FIRMWARE ...)
  D = OTA auf dasselbe Abbild, sofort       C = OTA auf dasselbe Abbild, ruhig bis +30 s
"Sofort": device_ready alle 0,3 s, bis das Geraet einmal NICHT antwortet (alte Firmware weg) und danach antwortet.
Danach in allen Armen ab +30 s dieselbe Folge smoke-device.sh, check-pwa.sh; Ruhewert ist die erste Heap-Zeile
>= 150 s danach mit tcp=0/0/0/0/0/0. Gemessen 17.10.2026 auf ESP 3.2.41: A und D max 6'600, B und C 7'104 (3/3).

SCHREIBT AM GERAET: startet den ESP neu bzw. loest ein OTA aus, je Arm einmal. Logwache parallel (DIR-013).
Zwei Fallen, die das Werkzeug umgeht: local_esp_restart antwortet nicht und blockiert bis zur Frist (deshalb im
eigenen Faden), und Abfragezeilen schieben die Startzeile aus einem kurzen tail (deshalb log.sh grep).
"""
import re, subprocess, time, datetime as D, urllib.request, sys, threading
R=subprocess.run(['git','-C',__import__('os').path.dirname(__import__('os').path.abspath(__file__)),'rev-parse','--show-toplevel'],capture_output=True,text=True).stdout.strip()
host=[re.match(r'\s*DEVICE_HOST=["\']?([^"\'\s]+)',z).group(1) for z in open(R+'/tools/device.conf') if re.match(r'\s*DEVICE_HOST=',z)][0]
RX=re.compile(r'^(\S+)\s+\(- heap free=(\d+) max=(\d+) .*tcp=([\d/]+)')
def log(s): print(time.strftime('%H:%M:%S'),s,flush=True)
def roh(n=1500): return subprocess.run([R+'/tools/logger/log.sh','tail',str(n)],capture_output=True,text=True,cwd=R).stdout.splitlines()
def ts(l):
    try: return D.datetime.fromisoformat(l.split()[0])
    except Exception: return None
def jetzt(): return D.datetime.now().astimezone()
def suche(m,n=400):
    # gezielt im Mitschnitt statt im Ende: Abfragezeilen schoben die Startzeile aus tail 400
    return sorted(subprocess.run([R+'/tools/logger/log.sh','grep',m,str(n)],capture_output=True,text=True,cwd=R).stdout.splitlines())
def mark(s): subprocess.run([R+'/tools/logger/log.sh','mark',s],capture_output=True,cwd=R)
def get(p,t=2):
    try: return urllib.request.urlopen(f'http://{host}{p}',timeout=t).read()
    except Exception: return None
def ruhe(nach,sek):
    tend=time.time()+700
    while time.time()<tend:
        z=[l for l in roh(400) if ts(l) and RX.match(l) and (ts(l)-nach).total_seconds()>=sek and RX.match(l).group(4)=='0/0/0/0/0/0']
        if z: m=RX.match(z[0]); return int(m.group(2)),int(m.group(3)),z[0].strip()
        time.sleep(20)
    return None,None,'kein Ruhewert'
def lauf(arm):
    ota=arm in 'CD'; sofort=arm in 'AD'
    mark(f'startprobe: Arm {arm} ({"OTA" if ota else "Neustart"}, {"sofort" if sofort else "ab +30 s"})')
    t0=jetzt(); log(f'== Arm {arm}')
    # Ausloeser im eigenen Faden: local_esp_restart antwortet nicht (Neustart vor der Antwort) und blockierte
    # sonst bis zur Frist -- die Abfrage im Arm 'sofort' begann dann erst 22 s nach dem Start (09:23:08).
    threading.Thread(target=get,args=('/api/remote_esp_update' if ota else '/api/local_esp_restart',30),daemon=True).start()
    erste=None
    if sofort:
        # erst warten, bis das Geraet NICHT antwortet (alte Firmware weg), dann bis zur ersten Antwort der neuen
        weg=False
        while (jetzt()-t0).total_seconds()<400:
            ok=get('/api/device_ready',1) is not None
            if not ok: weg=True
            elif weg: erste=jetzt(); break
            time.sleep(0.3)
        log(f'  sofort: alte Firmware weg={weg}, erste Antwort {erste.strftime("%H:%M:%S.%f")[:-3] if erste else "-"}')
    tf=None
    while tf is None and (jetzt()-t0).total_seconds()<420:
        time.sleep(2); z=[ts(l) for l in suche('(FIRMWARE ') if ts(l) and ts(l)>t0]
        if z: tf=z[0]
    if tf is None: log('ABBRUCH: kein Start im Mitschnitt'); return None
    while (jetzt()-tf).total_seconds()<30: time.sleep(1)
    ea=[l for l in suche('heap erste anfrage') if ts(l) and ts(l)>tf][:1]
    eas=f"{(ts(ea[0])-tf).total_seconds():.1f} s, {ea[0].split('(',1)[-1].rstrip(')')}" if ea else 'keine'
    log(f'  Start {tf.strftime("%H:%M:%S")}, erste Anfrage: {eas}')
    for c in (['./tools/smoke-device.sh'],['./tools/check-pwa.sh']):
        p=subprocess.run(c,capture_output=True,text=True,cwd=R); log(f'  {c[0]}: Exit {p.returncode}')
    f,m,l=ruhe(jetzt(),150); log(f'  Ruhewert: {l}')
    return (arm,eas,m)
erg=[]
if len(sys.argv)!=2 or not set(sys.argv[1])<=set('ABCD'):
    print(__doc__.split('\n\n')[1],file=sys.stderr); sys.exit(2)
for arm in sys.argv[1]:
    erg.append(lauf(arm))
log('ERGEBNIS')
for e in erg: log(f'  {e}')
mark('startprobe: Ende')
