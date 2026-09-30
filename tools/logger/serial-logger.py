#!/usr/bin/env python3
"""Schneidet den Debug-UART der WordClock durchgehend mit.

Warum ueberhaupt: Der Endpunkt /api/stm32_log der PWA versagt genau dann, wenn
man ihn braucht. Haengt die Uhr, antwortet der ESP nicht mehr und das Logbuch ist
weg. Dieser Mitschnitt laeuft durch -- auch durch einen Watchdog-Reset hindurch,
und faengt danach die Reset-Ursache aus der Startsequenz.

Jede Zeile bekommt einen Zeitstempel mit Millisekunden. Ohne den laesst sich
nicht sagen, ob zwischen zwei Zeilen 2 ms oder 20 s lagen -- und genau das ist
bei Blockaden die entscheidende Groesse.

Zusaetzlich liest der Logger eine FIFO. Wer dort hineinschreibt, setzt eine
Markierung im Mitschnitt. Damit werden Testschritte im Log auffindbar, ohne
hinterher raten zu muessen, wann man den Knopf gedrueckt hat.
"""

import argparse
import os
import sys
import threading
import time
from datetime import datetime

try:
    import serial
except ImportError:
    sys.exit("pyserial fehlt.  sudo apt install -y python3-serial")

FLUSH_SECONDS = 0.5      # SD-Karte schonen, ohne im Fehlerfall viel zu verlieren
FLUSH_LINES = 64


def stamp(when=None):
    return (when or datetime.now().astimezone()).isoformat(timespec="milliseconds")


class Writer:
    """Sammelt Zeilen und schreibt sie gebuendelt weg."""

    def __init__(self, path):
        self.path = path
        self.fh = open(path, "a", encoding="utf-8", errors="replace")
        self.lock = threading.Lock()
        self.pending = 0
        self.last = time.monotonic()

    def line(self, text, kind=" ", when=None):
        with self.lock:
            self.fh.write(f"{stamp(when)} {kind} {text}\n")
            self.pending += 1
            now = time.monotonic()
            if self.pending >= FLUSH_LINES or now - self.last >= FLUSH_SECONDS:
                self.fh.flush()
                os.fsync(self.fh.fileno())
                self.pending = 0
                self.last = now

    def reopen(self):
        """Nach einer Rotation zeigt das alte Handle ins Leere."""
        with self.lock:
            try:
                self.fh.flush()
                self.fh.close()
            except Exception:
                pass
            self.fh = open(self.path, "a", encoding="utf-8", errors="replace")


def mark_reader(fifo, writer):
    """Markierungen aus der FIFO in den Mitschnitt mischen."""
    while True:
        try:
            with open(fifo, "r", encoding="utf-8", errors="replace") as f:
                for line in f:
                    line = line.strip()
                    if line:
                        writer.line(line, kind="#")
        except Exception:
            time.sleep(1)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="/dev/wordclock")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--out", default="/var/log/wordclock/serial.log")
    ap.add_argument("--fifo", default="/run/wordclock-mark")
    args = ap.parse_args()

    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    writer = Writer(args.out)

    if args.fifo:
        try:
            if not os.path.exists(args.fifo):
                os.mkfifo(args.fifo, 0o666)
            os.chmod(args.fifo, 0o666)
            threading.Thread(target=mark_reader, args=(args.fifo, writer), daemon=True).start()
        except Exception as exc:
            writer.line(f"logger: FIFO nicht verfuegbar ({exc})", kind="!")

    # SIGHUP nach der Rotation: Datei neu oeffnen.
    import signal
    signal.signal(signal.SIGHUP, lambda *_: writer.reopen())

    writer.line(f"logger: gestartet auf {args.port} mit {args.baud} Baud", kind="#")

    while True:
        try:
            # Der Adapter kann abgezogen werden oder beim Booten noch fehlen --
            # das ist kein Grund, den Dienst zu beenden.
            with serial.Serial(args.port, args.baud, timeout=1) as ser:
                writer.line("logger: Port offen", kind="#")
                buf = bytearray()
                t0 = None
                while True:
                    # Das erste Byte einer Zeile wird einzeln gelesen und legt den
                    # Zeitstempel fest. Wuerde man in Bloecken lesen, bekaemen alle
                    # Zeilen eines Blocks dieselbe Zeit -- bei 700 Byte/s sind das
                    # leicht ein Dutzend Zeilen mit identischem Stempel, und jede
                    # Aussage ueber Abstaende waere wertlos.
                    if not buf:
                        first = ser.read(1)
                        if not first:
                            continue
                        t0 = datetime.now().astimezone()
                        buf.extend(first)
                    # Den Rest, der schon anliegt, ohne Warten dazunehmen.
                    waiting = ser.in_waiting
                    if waiting:
                        buf.extend(ser.read(waiting))
                    while b"\n" in buf:
                        raw, _, rest = buf.partition(b"\n")
                        writer.line(raw.decode("utf-8", "replace").rstrip("\r"), when=t0)
                        buf = bytearray(rest)
                        t0 = datetime.now().astimezone() if buf else None
        except Exception as exc:
            writer.line(f"logger: Port weg ({exc}) -- neuer Versuch in 2 s", kind="!")
            time.sleep(2)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
