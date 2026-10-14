#!/usr/bin/env python3
"""Zero-Window-Probe (Paket 2026-10-13, E3, design.md 18.8, Ent-26 a).

Ein TCP-Client mit kleinem Empfangspuffer sendet eine Anfrage, liest danach HALT Sekunden lang
nichts (die Gegenstelle sieht Fenster 0) und prueft dann, ob der Server die Uebertragung
ABGEBROCHEN hat: Verbindung geschlossen oder zurueckgesetzt, bevor Content-Length erreicht war.
Genau das soll die Sendezeitgrenze aus M-3 bewirken -- der ESP gibt nach 1 bzw. 2 s auf,
statt loop() 5 s anzuhalten.

  ./tools/zero-window-probe.py <host> <pfad> [--port 80] [--halt 6]

Exit 0  Abbruch beobachtet (Erwartung am Geraet ab ESP 3.2.36)
Exit 1  kein Abbruch: alles kam nach der Pause vollstaendig an (Erwartung gegen einen Server,
        der wartet, statt abzubrechen -- die Gegenprobe nach DIR-014)
Exit 2  Werkzeugfehler (keine Verbindung, keine Kopfzeilen, kein Content-Length)
Exit 3  nicht pruefbar: die ganze Antwort passte in den Empfangspuffer (unter macOS waechst er
        bis 4 MB, automatische Pufferanpassung von macOS) -- ohne Root nicht herstellbar

Am Geraet nur unter der Freigabe Ent-26 a (14.10.2026). Die Probe ist folgenlos: Die Verbindung
wird geschlossen, der ESP zaehlt einen Schreibfehler (write_lost_failed bzw. fs_write_short).
"""
import argparse, socket, time, re, sys, fcntl, termios, struct

ap = argparse.ArgumentParser()
ap.add_argument("host"); ap.add_argument("pfad")
ap.add_argument("--port", type=int, default=80)
ap.add_argument("--halt", type=float, default=6.0)
a = ap.parse_args()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Klein VOR dem Verbinden, damit das angekuendigte Fenster klein ist (das System rundet auf).
s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)
s.settimeout(5)
try:
    s.connect((a.host, a.port))
except OSError as e:
    print(f"FEHL Werkzeug: keine Verbindung zu {a.host}:{a.port} ({e})"); sys.exit(2)
# macOS vergroessert den Empfangspuffer selbst (net.inet.tcp.doautorcvbuf=1, bis autorcvbufmax
# 4 MB) -- gemessen 14.10.2026: 1'024 gesetzt, nach dem Connect 326'640, und nach 6 s lagen auch
# 300 KB vollstaendig im Puffer. SO_RCVBUF haelt das NICHT auf, auch nicht nach dem Verbinden
# gesetzt. Von einem Mac aus wird das Fenster also nie 0, solange die automatische Anpassung an
# ist (abschalten nur mit Root: sysctl net.inet.tcp.doautorcvbuf=0). Das Werkzeug erkennt den Fall
# selbst (Exit 3) statt ein Scheinergebnis zu melden.
s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)
rcv = s.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
s.sendall(f"GET {a.pfad} HTTP/1.0\r\nHost: {a.host}\r\nAccept-Encoding: gzip\r\nConnection: close\r\n\r\n".encode())
t0 = time.monotonic()
time.sleep(a.halt)                      # nicht lesen: Fenster laeuft voll
# Wie viel liegt nach der Pause schon im Empfangspuffer? Liegt dort die GANZE Antwort, ist das
# Fenster nie 0 geworden, und die Probe sagt nichts -- dann "nicht pruefbar" statt eines Ergebnisses.
try:
    wartend = struct.unpack("i", fcntl.ioctl(s.fileno(), termios.FIONREAD, b"\0\0\0\0"))[0]
except OSError:
    wartend = -1

s.settimeout(3)
daten, grund = b"", "Ende"
while True:
    try:
        b = s.recv(65536)
    except ConnectionResetError:
        grund = "zurueckgesetzt"; break
    except socket.timeout:
        grund = "Zeitueberschreitung beim Nachlesen"; break
    if not b:
        break
    daten += b
s.close()
dauer = time.monotonic() - t0

kopf, _, rumpf = daten.partition(b"\r\n\r\n")
m = re.search(rb"(?i)content-length:\s*(\d+)", kopf)
if not kopf.startswith(b"HTTP/"):
    print(f"FEHL Werkzeug: keine Kopfzeilen empfangen ({len(daten)} Byte, {grund}, Empfangspuffer {rcv})"); sys.exit(2)
if not m:
    print(f"FEHL Werkzeug: kein Content-Length in der Antwort -- Vollstaendigkeit nicht pruefbar ({grund})"); sys.exit(2)
soll = int(m.group(1))
print(f"  {a.pfad}: {len(rumpf)} von {soll} Byte nach {a.halt:.0f} s Pause, Ende: {grund}, Empfangspuffer {rcv}, nach der Pause wartend {wartend} Byte, {dauer:.1f} s")
if wartend >= len(kopf) + 4 + soll:
    print("  NICHT PRUEFBAR: die ganze Antwort lag schon im Empfangspuffer, das Fenster wurde nie 0 (automatische Pufferanpassung des Systems)")
    sys.exit(3)
if len(rumpf) < soll:
    print("  ABBRUCH beobachtet: der Server hat die Uebertragung waehrend des Fensters 0 aufgegeben")
    sys.exit(0)
print("  kein Abbruch: die Antwort kam nach der Pause vollstaendig an")
sys.exit(1)
