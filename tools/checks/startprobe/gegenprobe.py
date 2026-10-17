#!/usr/bin/env python3
"""Gegenprobe zur Instrumentpruefung von tools/startprobe.py (Spec C80, AKI.10/AKI.13 N2; DIR-014).
Ein vollstaendiger Lauf mit Zeilen in der Form des Instruments (Release I) muss ohne Fehler durchgehen, jede einzelne
hergestellte Verletzung muss einen Fehler mit Namen ergeben. Spricht weder mit dem Geraet noch mit dem Mitschnitt.
Aufruf: tools/checks/startprobe/gegenprobe.py    Exit 0 alle wie erwartet, 1 Abweichung."""
import datetime as D, importlib.util, os, sys
R = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
spec = importlib.util.spec_from_file_location('startprobe', os.path.join(R, 'tools', 'startprobe.py'))
sp = importlib.util.module_from_spec(spec); spec.loader.exec_module(sp)
tf = D.datetime(2026, 10, 18, 3, 0, 0).astimezone()
s = lambda sek: tf + D.timedelta(seconds=sek)
GUT = [(s(61), '- heap topf kopf t0=8000 w=60000 n=8572 free0=7840 free1=7144 us=37'),
       (s(62), '- heap topf 1/4 wif=16/16@6993 srv=-192/800@1995 ota=0/0@-'),
       (s(63), '- heap topf 2/4 ntp=0/0@- udp=0/0@- wet=0/0@-'),
       (s(64), '- heap topf 3/4 cac=0/0@- log=0/0@- stm=0/0@-'),
       (s(65), '- heap topf 4/4 sys=-520/520@5005 brk=0/0@-'),
       (s(66), '- heap karte S ent=3/2 fsum=7840 gross=229+7104 zweit=129+296 davor=166+56 danach=- ms=110'),
       (s(67), '- heap karte X ent=3/2 fsum=7144 gross=229+6848 zweit=129+296 davor=166+504 danach=1085+4032 ms=5005'),
       (s(68), '- heap marken X tcp=6:10/- lst=2 wet=38 dhcp=34 eep=42 udp=14/18,22/26,30/-'),
       (s(70), '- heap karte m ent=3/2 fsum=7152 gross=229+6856 zweit=129+296 davor=166+504 danach=1086+4024'),
       (s(130), '- heap karte m ent=3/2 fsum=7336 gross=229+6600 zweit=129+296 davor=166+504 danach=1086+4024')]
HEAP = [s(0.1), s(60), s(120)]
def ring_von(z): return [x for _, x in z]
def pruefe(name, zeilen, ring, soll_fehler):
    e = {'fehlt': []}; sp.instrument_pruefen(e, tf, zeilen, HEAP, ring, s(200))
    if not soll_fehler and not e['fehlt']: print(f'  ok    {name}: kein Fehler'); return 0
    if soll_fehler and any(soll_fehler in f for f in e['fehlt']): print(f'  ok    {name}: "{[f for f in e["fehlt"] if soll_fehler in f][0][:90]}"'); return 0
    print(f'  FALSCH {name}: erwartet {soll_fehler or "keinen Fehler"!r}, erhalten {e["fehlt"]}'); return 1
bad = 0
bad += pruefe('vollstaendiger Lauf', GUT, ring_von(GUT), None)
bad += pruefe('topf 3/4 fehlt', [z for z in GUT if '3/4' not in z[1]], ring_von(GUT), 'Zaehlsoll topf i/4')
bad += pruefe('Summenprobe verletzt', [(x, z.replace('sys=-520', 'sys=-500')) for x, z in GUT], None, 'Summenprobe reisst')
bad += pruefe('Zeile vor dem Ende von W', [(s(30), GUT[0][1])] + GUT[1:], None, 'vor dem Ende von W')
bad += pruefe('abbruch=', GUT + [(s(80), '- heap karte m abbruch=ord')], None, 'abbruch=')
bad += pruefe('Zeile fehlt im Ring', GUT, ring_von(GUT)[:-1], 'nicht textgleich')
bad += pruefe('karte m fehlt', [z for z in GUT if 'karte m' not in z[1]], None, 'karte m')
bad += pruefe('letzte karte m fehlt', [z for z in GUT if z[0] != s(130)], None, 'karte m')
bad += pruefe('zwei marken', GUT + [(s(69), '- heap marken W tcp=- lst=2')], None, 'Zaehlsoll marken')
print(f'  Gegenprobe Startprobe: 9 Faelle, {bad} falsch'); sys.exit(1 if bad else 0)
