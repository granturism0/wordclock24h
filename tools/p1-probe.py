#!/usr/bin/env python3
"""P1-Probe (Paket 2026-10-15, design.md §5.6, AKM.7): Laeuft FIN_WAIT_2 auf dem ESP nach ~20 s ab, wenn die
Gegenstelle ihre Seite NICHT schliesst?

Ablauf: Eine Verbindung sendet `GET /api/device_ready HTTP/1.0`, liest bis zum FIN des ESP und schliesst NICHT. Der Mac
quittiert das FIN selbsttaetig; der PCB auf dem ESP steht dann in FIN_WAIT_2 (Mac-Seite CLOSE_WAIT). Aus einer ZWEITEN
Verbindung wird device_ready nach 10 s und nach 25 s gelesen und in tcp_now_list der Eintrag mit dem lokalen Port der
Probe gesucht (port = Port der Gegenstelle aus Sicht des ESP).

Erwartet unter P1: nach 10 s FIN_WAIT_2 (6), arg = 0; nach 25 s weg. Noch da nach 25 s => P1 widerlegt, K2 zwingend.

Aufruf:  tools/p1-probe.py            gegen DEVICE_HOST aus tools/device.conf
         P1_PROBE_HOST=<host> ...     andere Adresse (Gegenprobe: 192.0.2.1 ist nie erreichbar)
Exit 0 P1 bestaetigt, 1 P1 widerlegt, 2 Werkzeugfehler/Vorbedingung, 3 nicht beobachtbar (Eintrag schon nach 10 s weg
oder nie gesehen -- dann ist die Frage offen, nicht beantwortet).
Schreibt nichts am Geraet.
"""
import json, os, re, socket, subprocess, sys, time, urllib.request

ZUSTAND = {0: "CLOSED", 1: "LISTEN", 2: "SYN_SENT", 3: "SYN_RCVD", 4: "ESTABLISHED", 5: "FIN_WAIT_1",
           6: "FIN_WAIT_2", 7: "CLOSE_WAIT", 8: "CLOSING", 9: "LAST_ACK", 10: "TIME_WAIT"}


def host_lesen():
    h = os.environ.get("P1_PROBE_HOST")
    if h:
        return h
    root = subprocess.run(["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True).stdout.strip()
    try:
        for z in open(os.path.join(root, "tools", "device.conf")):
            m = re.match(r'\s*DEVICE_HOST=["\']?([^"\'\s]+)', z)
            if m:
                return m.group(1)
    except OSError:
        pass
    print("DEVICE_HOST fehlt (tools/device.conf).", file=sys.stderr)
    sys.exit(2)


def zeit():
    return time.strftime("%H:%M:%S")


def lesung(host, port_probe, wann):
    try:
        with urllib.request.urlopen(f"http://{host}/api/device_ready", timeout=10) as r:
            d = json.load(r)
    except Exception as e:
        print(f"{zeit()}  {wann}: device_ready nicht lesbar ({e})")
        return None, False
    if "tcp_now_list" not in d:
        print(f"{zeit()}  {wann}: keine tcp_now_list -- Firmware vor R1 (ESP 3.2.38)")
        sys.exit(2)
    treffer = [e for e in d["tcp_now_list"] if len(e) >= 6 and e[2] == port_probe]
    mehr = d.get("tcp_now_more", 0)
    print(f"{zeit()}  {wann}: tcp_now_states={d.get('tcp_now_states')} tw={d.get('tcp_now_tw')} "
          f"Liste={d['tcp_now_list']} more={mehr}")
    if treffer:
        e = treffer[0]
        print(f"          Probe (Port {port_probe}): {ZUSTAND.get(e[0], e[0])}, ruhe_s={e[3]}, nrtx={e[4]}, arg={e[5]}")
    else:
        print(f"          Probe (Port {port_probe}): nicht in der Liste" + (" (Liste gekappt, more>0!)" if mehr else ""))
    return (treffer[0] if treffer else None), bool(mehr)


def mac_seite(host, port_probe):
    out = subprocess.run(["netstat", "-anv", "-p", "tcp"], capture_output=True, text=True).stdout
    z = [l.split() for l in out.splitlines() if f".{port_probe} " in l and host in l]
    print("          Mac-Seite: " + ("; ".join(f"{x[3]} {x[5]}" for x in z) if z else "keine Zeile"))


def main():
    host = host_lesen()
    hp = host.split(":")
    ziel = (hp[0], int(hp[1]) if len(hp) > 1 else 80)
    root = subprocess.run(["git", "rev-parse", "--git-dir"], capture_output=True, text=True).stdout.strip()
    s = socket.socket()
    s.settimeout(10)
    try:
        s.connect(ziel)
    except OSError as e:
        print(f"ABBRUCH: Uhr unter {host} nicht erreichbar ({e})", file=sys.stderr)
        sys.exit(2)
    if root and not os.environ.get("P1_PROBE_HOST"):
        open(os.path.join(root, "geraet-gemessen"), "a").close()
    port = s.getsockname()[1]
    print(f"=== P1-Probe gegen {host} -- {time.strftime('%Y-%m-%d %H:%M:%S')} -- lokaler Port {port} ===")
    s.sendall(b"GET /api/device_ready HTTP/1.0\r\n\r\n")
    n = 0
    while True:
        b = s.recv(4096)
        if not b:
            break
        n += len(b)
    t_fin = time.time()
    print(f"{zeit()}  FIN des ESP nach {n} Byte erhalten; die Probe schliesst NICHT")
    time.sleep(max(0, 10 - (time.time() - t_fin)))
    e10, mehr10 = lesung(host, port, "nach 10 s")
    mac_seite(host, port)
    time.sleep(max(0, 25 - (time.time() - t_fin)))
    e25, mehr25 = lesung(host, port, "nach 25 s")
    mac_seite(host, port)
    s.close()
    print(f"{zeit()}  Probe geschlossen")
    if e10 is None:
        print("ERGEBNIS: nicht beobachtbar -- der Eintrag war schon nach 10 s nicht (mehr) zu sehen"
              + (" (Liste gekappt)" if mehr10 else ""))
        sys.exit(3)
    if e25 is not None:
        print(f"ERGEBNIS: P1 WIDERLEGT -- nach 25 s noch {ZUSTAND.get(e25[0], e25[0])} (arg={e25[5]}); K2 wird zwingend")
        sys.exit(1)
    if mehr25:
        print("ERGEBNIS: nicht beobachtbar -- nach 25 s nicht in der Liste, aber die Liste war gekappt")
        sys.exit(3)
    zustand10 = ZUSTAND.get(e10[0], e10[0])
    print(f"ERGEBNIS: P1 bestaetigt -- nach 10 s {zustand10} (arg={e10[5]}), nach 25 s weg")
    sys.exit(0)


main()
