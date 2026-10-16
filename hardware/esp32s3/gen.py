#!/usr/bin/env python3
"""Erzeugt aus der V2-Netzliste das Schema fuer den ESP32-S3-Umbau.

Quelle der Wahrheit ist die von kicad-cli exportierte V2-Netzliste (nets.json) und
der V2-Schaltplan (Symbole, Eigenschaften). Das Skript entfernt die STM32-/ESP-12F-Teile,
fuegt die S3-Teile hinzu und legt jeden Pin per Drahtstummel und Netzbezeichner an.
"""
import sys, re, json, uuid, math, os, copy
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sx import *

HERE = os.path.dirname(os.path.abspath(__file__))
V2DIR = os.path.expanduser("~/Documents/WordClock-USB C - STM32F411 - V2")
V2SCH = V2DIR + "/WordClock-USB C - STM32F411 - V2.kicad_sch"
SYMDIR = "/Applications/KiCad/KiCad.app/Contents/SharedSupport/symbols/"
OUT = sys.argv[1] if len(sys.argv) > 1 else HERE + "/WordClock-ESP32S3-V3.kicad_sch"
PROJECT = os.path.basename(OUT)[:-len(".kicad_sch")]

v2 = open(V2SCH, encoding="utf-8").read()
root = parse(v2)
ROOT_UUID = str(uuid.uuid4())
def U(): return str(uuid.uuid4())
def esc(s): return s.replace("\\", "\\\\").replace('"', '\\"')

# ------------------------------------------------------------------ V2 einlesen
inst = {}
for el in root:
    if isinstance(el, list) and el and el[0] == "symbol":
        lib = None; props = []; flags = {}
        for c in el[1:]:
            if isinstance(c, list) and c:
                if c[0] == "lib_id": lib = sval(c[1])
                elif c[0] == "property": props.append((sval(c[1]), sval(c[2])))
                elif c[0] in ("in_bom", "on_board", "dnp", "exclude_from_sim"): flags[c[0]] = c[1]
        pd = dict(props)
        if lib is None or lib.startswith("power:"): continue
        inst[pd["Reference"]] = dict(lib=lib, props=props, flags=flags)

nets_v2 = json.load(open(HERE + "/v2/nets.json"))
conn = {}
for name, nodes in nets_v2:
    if name.startswith("unconnected-"): continue
    for ref, pin, _ in nodes: conn[(ref, pin)] = name

# ------------------------------------------------------------------ Symbolbibliothek
libblocks = {}   # lib_id -> Blocktext fuer lib_symbols
def stock(lib, name):
    t = open(SYMDIR + lib + ".kicad_sym", encoding="utf-8").read()
    b = find_symbol(t, name)
    m = re.search(r'\(extends "([^"]+)"\)', b)
    if m:
        base = find_symbol(t, m.group(1))
        base = base.replace('(symbol "%s"' % m.group(1), '(symbol "%s"' % name, 1)
        base = base.replace('"%s_' % m.group(1), '"%s_' % name)
        # alle Eigenschaftsbloecke der abgeleiteten Definition uebernehmen (samt Lage), damit die
        # eingebettete Kopie der aufgeloesten Bibliotheksdefinition entspricht
        for pm_ in re.finditer(r'\n\t\t\(property "([^"]+)"', b):
            blk = block_at(b, pm_.start() + 1)
            om = re.search(r'\n\t\t\(property "%s"' % re.escape(pm_.group(1)), base)
            if om:
                ob = block_at(base, om.start() + 1)
                base = base.replace(ob, blk, 1)
        b = base
    return b.replace('(symbol "%s"' % name, '(symbol "%s:%s"' % (lib, name), 1)

def v2sym(lib_id):
    return find_symbol(v2, lib_id)

NEW = {  # ref -> (lib_id, Quelle, Wert, Footprint, Zusatzfelder)
}
def reg_stock(lib, name):
    lid = lib + ":" + name
    libblocks[lid] = stock(lib, name)
    return lid

def blockprops(lid):
    b = libblocks[lid]
    return dict(re.findall(r'\(property "(Value|Footprint|Datasheet|Description)" "((?:[^"\\]|\\.)*)"', b))

# ------------------------------------------------------------------ Delta
REMOVE = ["U2", "U4", "X1", "C3", "C5", "C9", "C11", "C14", "L1", "R7", "R2", "R10",
          "R14", "R15", "R16", "R18", "SW4", "H3", "D1", "USB1"]
for r in REMOVE:
    inst.pop(r)
for k in [k for k in conn if k[0] in REMOVE]:
    del conn[k]
# nach dem Entfernen verwaiste Netze: Pins, die nur noch an Netzen mit einem Pin haengen, pruefen wir unten

lid_mod = reg_stock("RF_Module", "ESP32-S3-WROOM-1")
lid_usb = reg_stock("Connector", "USB_C_Receptacle_USB2.0_16P")
lid_esd = reg_stock("Power_Protection", "USBLC6-2SC6")
pm = {p["num"]: p for p in pins_of(libblocks[lid_mod])}
modname = {}
for p in pins_of(libblocks[lid_mod]): modname.setdefault(p["name"], []).append(p["num"])
esdp = pins_of(libblocks[lid_esd])

def newpart(ref, lid, value=None, footprint=None, extra=()):
    bp = blockprops(lid)
    props = [("Reference", ref), ("Value", value or bp.get("Value", "")),
             ("Footprint", footprint if footprint is not None else bp.get("Footprint", "")),
             ("Datasheet", bp.get("Datasheet", "")), ("Description", bp.get("Description", ""))]
    props += list(extra)
    inst[ref] = dict(lib=lid, props=props, flags={})

newpart("U2", lid_mod, "ESP32-S3-WROOM-1-N8")
newpart("USB1", lid_usb, "USB-C 16P", "Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12")
newpart("U11", lid_esd, "USBLC6-2SC6")

def clone(ref, srcref, value=None):
    d = copy.deepcopy(inst[srcref])
    d["props"] = [(k, (ref if k == "Reference" else (value if (k == "Value" and value) else v))) for k, v in d["props"]]
    inst[ref] = d
clone("SW5", "SW1")                 # BOOT-Taster wie die uebrigen Taster
clone("R32", "R12", "10kΩ")         # Pull-up GPIO0
clone("C30", "C12", "10uF")         # Abblockung Modul
clone("C31", "C12", "100nF")        # Abblockung Modul
clone("C32", "C12", "100nF")        # Filter am ADC-Eingang
for r, v in (("C1", "1uF"),):   # R22 bleibt V2-Wert 1 kOhm (LDR-Pfad 1:1, Vorgabe 17.10.2026)
    inst[r]["props"] = [(k, (v if k == "Value" else val)) for k, val in inst[r]["props"]]

# ------------------------------------------------------------------ LCSC-Nummern (Feld "LCSC", wie in V2)
# Jede Nummer ist an der Quelle geprueft (hardware/komponenten/jlcpcb-bestand.md und die Komponentenblaetter,
# Pruefdatum 2026-10-17). Nur diese Zeilen aendern oder setzen ein LCSC-Feld; alle uebrigen behalten V2.
#  - C1: V2 = 100 nF (C49678). BEWUSSTE ABWEICHUNG VON V2: V3 setzt 1 uF (EN-Verzoegerung 10 kOhm + 1 uF nach
#    Espressif), also C28323 (1 uF 50 V X7R, Basic). C1 liegt nicht im LED-, Power- oder LDR-Pfad (H8 prueft das).
#  - R23: das Feld trug C17841 (= 82 Ohm, Extended); richtig ist C17520 (2,2 kOhm 0805, Basic).
#  - C30 ist als Klon von C12 (100 nF, C49678) entstanden; 10 uF = C15850 (25 V X5R, Basic;
#    Alternative C440198, 50 V).
#  - U2, USB1, U11: neue Teile, das Feld fehlte (die Bibliothekssymbole haben keins) und wird neu angelegt.
#  - SW5 ist Klon von SW1 und traegt dessen Nummer (C2909692); R32 (Klon von R12) C17414; C31, C32 (Klone von C12) C49678.
LCSC_SET = {
    "C1": "C28323", "R23": "C17520", "C30": "C15850",
    "U2": "C2913198", "USB1": "C165948", "U11": "C7519",
    "SW5": "C2909692", "R32": "C17414", "C31": "C49678", "C32": "C49678",
}
def set_lcsc(ref, nr):
    props = inst[ref]["props"]
    if any(k == "LCSC" for k, _ in props):
        inst[ref]["props"] = [(k, (nr if k == "LCSC" else v)) for k, v in props]
    else:
        props.append(("LCSC", nr))
    # V2 fuehrt bei manchen Teilen zusaetzlich "LCSC Part"; es darf der Nummer nicht widersprechen
    inst[ref]["props"] = [(k, (nr if k == "LCSC Part" else v)) for k, v in inst[ref]["props"]]
for _r, _n in LCSC_SET.items(): set_lcsc(_r, _n)

def setnet(ref, pin, net): conn[(ref, pin)] = net
def modpin(name, net):
    for n in modname[name]: setnet("U2", n, net)

# Signale 1:1 vom STM32 auf den S3
modpin("IO5", "WS2812_BUS"); modpin("IO6", "WS2812_POWER")
modpin("IO8", "SDA_DS3231"); modpin("IO9", "SCL_DS3231")
modpin("IO2", "DS18B20"); modpin("IO1", "LDR")
modpin("IO7", "TSOP"); modpin("IO15", "DCF77")
modpin("IO16", "USER"); modpin("IO17", "WPS"); modpin("IO18", "LED_STM")
modpin("USB_D-", "USB_DN"); modpin("USB_D+", "USB_DP")
modpin("TXD0", "TX_UART"); modpin("RXD0", "RX_UART")
modpin("EN", "EN"); modpin("IO0", "BOOT")
modpin("3V3", "VCC_3.3V"); modpin("GND", "GND")
# Reset/EN: RESET und ESP_EN wurden ein Netz
for k, v in list(conn.items()):
    if v in ("RESET", "ESP_EN"): conn[k] = "EN"
# Konsole: H6 hing ueber SW4 an TX/RX
setnet("H6", "2", "TX_UART"); setnet("H6", "3", "RX_UART")
# BOOT-Taste und Pull-up
setnet("SW5", "1", "BOOT"); setnet("SW5", "2", "BOOT"); setnet("SW5", "3", "GND")
setnet("R32", "1", "BOOT"); setnet("R32", "2", "VCC_3.3V")
# Abblockung
setnet("C30", "1", "VCC_3.3V"); setnet("C30", "2", "GND")
setnet("C31", "1", "VCC_3.3V"); setnet("C31", "2", "GND")
setnet("C32", "1", "LDR"); setnet("C32", "2", "GND")
# USB-C 16P
for p in ("A4", "A9", "B4", "B9"): setnet("USB1", p, "USB_5V_INPUT")
for p in ("A1", "A12", "B1", "B12", "SH"): setnet("USB1", p, "GND")
setnet("USB1", "A5", "Net-(USB1-CC1)"); setnet("USB1", "B5", "Net-(USB1-CC2)")
setnet("USB1", "A6", "USB_DP"); setnet("USB1", "B6", "USB_DP")
setnet("USB1", "A7", "USB_DN"); setnet("USB1", "B7", "USB_DN")
# ESD-Schutz USBLC6-2SC6
for p in esdp:
    n = p["name"]
    if n == "GND": setnet("U11", p["num"], "GND")
    elif n == "VBUS": setnet("U11", p["num"], "USB_5V_INPUT")
    elif n == "I/O1": setnet("U11", p["num"], "USB_DP")
    elif n == "I/O2": setnet("U11", p["num"], "USB_DN")

# Netz Net-(USB1-CC1/CC2) behalten ihre Namen aus V2 (R4.1, R3.1 hingen schon daran)
# TX_UART: aus V2 verbleiben keine Mitglieder ausser den neuen -> kein Eintrag noetig

# ------------------------------------------------------------------ Soll-Netzliste speichern (fuer den Vergleich)
expected = {}
for (ref, pin), net in conn.items(): expected.setdefault(net, set()).add("%s.%s" % (ref, pin))
json.dump({"nets": {k: sorted(v) for k, v in sorted(expected.items())},
           "parts": {r: [d["lib"], dict(d["props"]).get("Value", "")] for r, d in sorted(inst.items())}},
          open(HERE + "/expected.json", "w"), indent=1)
singles = {k: sorted(v) for k, v in expected.items() if len(v) < 2}
if singles: print("WARNUNG Netze mit einem Pin:", singles)

# ------------------------------------------------------------------ Geometrie
power_lib_blocks = {
    "power:GND": v2sym("power:GND"), "power:VCC": v2sym("power:VCC"), "power:PWR_FLAG": v2sym("power:PWR_FLAG")}
for r, d in inst.items():
    if d["lib"] not in libblocks: libblocks[d["lib"]] = v2sym(d["lib"])
for k, b in power_lib_blocks.items(): libblocks[k] = b

# Widerstandsnetzwerke: der EasyEDA-Import hat die Pins als "input" angelegt; das erzeugt einen
# ERC-Fehler "pin_not_driven" an den Adressleitungen des EEPROMs. Ein Widerstand ist passiv.
for _lid in list(libblocks):
    if _lid.startswith("EasyEDA:4D02WGJ"):
        libblocks[_lid] = libblocks[_lid].replace("(pin input line", "(pin passive line")
pins_cache = {lid: pins_of(b) for lid, b in libblocks.items()}
STUB = 5.08; CW = 1.2; MARG = 5.0
POWER = {"GND": "power:GND", "VCC_3.3V": "power:VCC", "VCC_5V": "power:VCC"}

def rot_of(lid):
    ps = pins_cache[lid]
    return 90 if len(ps) >= 2 and all(p["a"] in (90, 270) for p in ps) else 0

def xf(p, rot):
    th = math.radians(rot); c = round(math.cos(th)); s = round(math.sin(th))
    rx = p["x"] * c - p["y"] * s; ry = p["x"] * s + p["y"] * c
    a = math.radians(p["a"]); ox, oy = -math.cos(a), -math.sin(a)
    rox = ox * c - oy * s; roy = ox * s + oy * c
    return round(rx, 3), round(-ry, 3), int(round(rox)), int(round(-roy))

def fmt(x): return ("%.2f" % x).rstrip("0").rstrip(".")

parts = {}   # ref -> dict mit Lage
for ref, d in inst.items():
    lid = d["lib"]; rot = rot_of(lid)
    pts = []; items = []
    for p in pins_cache[lid]:
        dx, dy, sx, sy = xf(p, rot)
        net = conn.get((ref, p["num"]))
        items.append((p, dx, dy, sx, sy, net))
    parts[ref] = dict(lid=lid, rot=rot, items=items)

def extent(ref):
    P = parts[ref]
    xs = []; ys = []
    for p, dx, dy, sx, sy, net in P["items"]:
        xs += [dx]; ys += [dy]
    pminx, pmaxx, pminy, pmaxy = min(xs), max(xs), min(ys), max(ys)
    bx0, bx1, by0, by1 = pminx, pmaxx, pminy, pmaxy
    for p, dx, dy, sx, sy, net in P["items"]:
        if net is None: continue
        ex, ey = dx + sx * STUB, dy + sy * STUB
        if net in POWER:
            bx0 = min(bx0, ex - 4); bx1 = max(bx1, ex + 4)
            by0 = min(by0, ey - 8); by1 = max(by1, ey + 8)
        else:
            L = len(net) * CW + 3
            tx, ty = ex + sx * L, ey + sy * L
            bx0 = min(bx0, ex, tx); bx1 = max(bx1, ex, tx)
            by0 = min(by0, ey, ty); by1 = max(by1, ey, ty)
    # Platz fuer Kennzeichen und Wert
    if P["rot"] == 90: by0 -= 6
    else: by1 += 17
    P["pbox"] = (pminx, pmaxx, pminy, pmaxy)
    P["box"] = (bx0, bx1, by0, by1)
    return P["box"]

BLOCKS = [
    ("USB-C, Eingangsschutz und Power-Mux", ["USB1", "U11", "R3", "R4", "D4", "F1", "TP1", "TP4", "JP3", "C4", "C20",
        "D5", "F2", "H1", "TP5", "Q3", "R24", "R25", "C2", "C21", "C22", "TP6", "U9", "R26", "C24"]),
    ("Versorgung 5 V und 3,3 V", ["U10", "L2", "C25", "R28", "R29", "D2", "C27", "C28", "C29", "C23", "C26",
        "C7", "C8", "C12", "C15", "C16", "C17", "C18", "C19", "TP7", "TP8", "LED4", "R27", "LED5", "R30"]),
    ("ESP32-S3-WROOM-1 mit Reset, Boot und Konsole", ["U2", "C1", "R12", "SW3", "SW5", "R32", "C30", "C31",
        "H6", "D3", "SW1", "SW2", "R1", "LED1", "R6"]),
    ("LED-Ausgang, Pegelwandler und 5-V-Schalter", ["U3", "R11", "R13", "R17", "R31", "JP2", "Q1", "Q2",
        "R19", "R20", "R21", "R23", "C10", "C6", "H4", "H7", "H5"]),
    ("Sensoren, RTC und EEPROM", ["U1", "R5", "U5", "U7", "U6", "RN1", "RN2", "R22", "C32", "R8", "R9"]),
    ("Befestigung", ["H2", "H8", "H9", "H10"]),
]
assigned = {r for _, rs in BLOCKS for r in rs}
rest = [r for r in inst if r not in assigned]
if rest: BLOCKS.append(("Sonstiges", rest))
missing = [r for _, rs in BLOCKS for r in rs if r not in inst]
if missing: print("WARNUNG: Referenzen im Layout ohne Bauteil:", missing)

ROWW = 360.0
out_wires = []; out_labels = []; out_nc = []; out_syms = []; out_texts = []; out_rects = []
cur_y = 20.0; maxw = 0
pwr_n = [0]
def snap(v, g=2.54): return round(v / g) * g

for title, refs in BLOCKS:
    refs = [r for r in refs if r in inst]
    for r in refs: extent(r)
    x0 = 20.0; ytitle = cur_y + 12
    x = x0 + 5; y = ytitle + 6; rowh = 0; bw = 0
    placed = []
    for r in refs:
        bx0, bx1, by0, by1 = parts[r]["box"]
        w = bx1 - bx0 + 2 * MARG; h = by1 - by0 + 2 * MARG
        if x + w > x0 + ROWW and x > x0 + 5:
            x = x0 + 5; y += rowh; rowh = 0
        ox = snap(x + MARG - bx0); oy = snap(y + MARG - by0)
        parts[r]["at"] = (ox, oy)
        x += w; rowh = max(rowh, h); bw = max(bw, x)
    bh = y + rowh - cur_y
    out_texts.append('(text "%s" (exclude_from_sim no) (at %s %s 0) (effects (font (size 4 4) (bold yes)) (justify left bottom)) (uuid "%s"))'
                     % (esc(title), fmt(x0 + 2), fmt(ytitle), U()))
    out_rects.append('(rectangle (start %s %s) (end %s %s) (stroke (width 0.3) (type dash)) (fill (type none)) (uuid "%s"))'
                     % (fmt(x0), fmt(cur_y), fmt(bw + 5), fmt(cur_y + bh + 6), U()))
    maxw = max(maxw, bw + 5)
    cur_y = cur_y + bh + 6 + 14

def lab(net, X, Y, sx, sy):
    if sx == -1: ang, j = 180, "right bottom"
    elif sx == 1: ang, j = 0, "left bottom"
    elif sy == -1: ang, j = 90, "left bottom"
    else: ang, j = 270, "right bottom"
    return ('(label "%s" (at %s %s %d) (effects (font (size 1.27 1.27)) (justify %s)) (uuid "%s"))'
            % (esc(net), fmt(X), fmt(Y), ang, j, U()))

def wire(x1, y1, x2, y2):
    return ('(wire (pts (xy %s %s) (xy %s %s)) (stroke (width 0) (type default)) (uuid "%s"))'
            % (fmt(x1), fmt(y1), fmt(x2), fmt(y2), U()))

def propline(name, val, x, y, hide=False, just=None):
    j = (" (justify %s)" % just) if just else ""
    return ('(property "%s" "%s" (at %s %s 0) (effects (font (size 1.27 1.27))%s%s))'
            % (name, esc(val), fmt(x), fmt(y), j, " (hide yes)" if hide else ""))

def power_symbol(net, X, Y):
    lid = POWER[net]
    pwr_n[0] += 1
    ref = "#PWR%03d" % pwr_n[0]
    off = 3.5 if lid == "power:GND" else -3.5
    s = ['(symbol (lib_id "%s") (at %s %s 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no) (uuid "%s")'
         % (lid, fmt(X), fmt(Y), U()),
         propline("Reference", ref, X, Y, hide=True),
         propline("Value", net, X + 2, Y + off, just="left"),
         propline("Footprint", "", X, Y, hide=True), propline("Datasheet", "", X, Y, hide=True),
         '(pin "1" (uuid "%s"))' % U(),
         '(instances (project "%s" (path "/%s" (reference "%s") (unit 1))))' % (PROJECT, ROOT_UUID, ref), ')']
    return " ".join(s)

done_pts = set()
for ref, P in parts.items():
    ox, oy = P["at"]; lid = P["lid"]; rot = P["rot"]
    d = inst[ref]
    pminx, pmaxx, pminy, pmaxy = P["pbox"]
    # Symbol
    s = ['(symbol (lib_id "%s") (at %s %s %d) (unit 1) (exclude_from_sim %s) (in_bom %s) (on_board %s) (dnp %s) (uuid "%s")'
         % (lid, fmt(ox), fmt(oy), rot, d["flags"].get("exclude_from_sim", "no"), d["flags"].get("in_bom", "yes"),
            d["flags"].get("on_board", "yes"), d["flags"].get("dnp", "no"), U())]
    if rot == 90: rx, ry1, ry2 = ox, oy + pminy - 5.5, oy + pminy - 3.0
    else: rx, ry1, ry2 = ox + (pminx + pmaxx) / 2, oy + pmaxy + 12.7, oy + pmaxy + 14.6
    for k, v in d["props"]:
        if k == "Reference": s.append(propline(k, v, rx, ry1))
        elif k == "Value": s.append(propline(k, v, rx, ry2))
        else: s.append(propline(k, v, ox, oy, hide=True))
    seen = set()
    for p in pins_cache[lid]:
        if p["num"] in seen: continue
        seen.add(p["num"]); s.append('(pin "%s" (uuid "%s"))' % (esc(p["num"]), U()))
    s.append('(instances (project "%s" (path "/%s" (reference "%s") (unit 1))))' % (PROJECT, ROOT_UUID, ref))
    s.append(")")
    out_syms.append(" ".join(s))
    # Anschluesse
    for p, dx, dy, sx, sy, net in P["items"]:
        X, Y = ox + dx, oy + dy
        key = (round(X, 2), round(Y, 2))
        if net is None:
            if p["type"] == "no_connect": continue
            if (key, "nc") in done_pts: continue
            done_pts.add((key, "nc"))
            out_nc.append('(no_connect (at %s %s) (uuid "%s"))' % (fmt(X), fmt(Y), U()))
            continue
        if (key, net) in done_pts: continue
        done_pts.add((key, net))
        ex, ey = X + sx * STUB, Y + sy * STUB
        out_wires.append(wire(X, Y, ex, ey))
        if net in POWER: out_syms.append(power_symbol(net, ex, ey))
        else: out_labels.append(lab(net, ex, ey, sx, sy))

# PWR_FLAG-Stellen kommen aus flags.json (Ergebnis der ERC-Laeufe)
flagfile = HERE + "/flags.json"
FLAGS = json.load(open(flagfile)) if os.path.exists(flagfile) else []
fx = snap(30.0)
for net in FLAGS:
    fy = snap(cur_y + 10)
    pwr_n[0] += 1
    ref = "#FLG%03d" % pwr_n[0]
    out_syms.append('(symbol (lib_id "power:PWR_FLAG") (at %s %s 0) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no) (uuid "%s") %s %s %s %s (pin "1" (uuid "%s")) (instances (project "%s" (path "/%s" (reference "%s") (unit 1)))))'
                    % (fmt(fx), fmt(fy), U(), propline("Reference", ref, fx, fy, hide=True), propline("Value", "PWR_FLAG", fx, fy - 3.5),
                       propline("Footprint", "", fx, fy, hide=True), propline("Datasheet", "", fx, fy, hide=True), U(), PROJECT, ROOT_UUID, ref))
    if net in POWER: out_syms.append(power_symbol(net, fx, fy + 5.08))
    else: out_labels.append(lab(net, fx, fy + 5.08, 1, 0))
    out_wires.append(wire(fx, fy, fx, fy + 5.08))
    fx += snap(40.0)

# Hinweise
notes = [
    "Entwurf V3: Umbau von V2 auf ESP32-S3-WROOM-1, erzeugt aus der V2-Netzliste (kicad-cli).",
    "Entfallen gegenueber V2: STM32F411 (U2), ESP-12F (U4), Quarz X1 samt Lastkondensatoren, VDDA-Filter, SWD-Stecker H3, Umschalter SW4, Boot-Widerstaende des ESP8266.",
    "Neu: ESP32-S3-WROOM-1 (U2), USB-C 16P mit D+/D- (USB1), ESD-Schutz U11, BOOT-Taste SW5 mit Pull-up R32, Abblockung C30/C31, ADC-Filter C32.",
    "Geaendert: C1 = 1 uF (EN-Verzoegerung, bewusste Abweichung von V2: dort 100 nF, LCSC C28323), H6 = UART0-Konsole.",
    "Unveraendert: Versorgung, 3,3-V-ODER-Schaltung (D2/D3), LED-Pfad (U3, Q1, Q2), LDR-Pfad (R22 = 1 kOhm wie V2), RTC, EEPROM, DS18B20, Taster, Test-Pads.",
    "Noch nicht geprueft: Footprints neuer Teile gegen die Bestueckung, 3,3-V-Reserve des SY8301, Platzierung der Antenne.",
]
ny = cur_y + 30
for i, n in enumerate(notes):
    out_texts.append('(text "%s" (exclude_from_sim no) (at 25 %s 0) (effects (font (size 2.5 2.5)) (justify left bottom)) (uuid "%s"))'
                     % (esc(n), fmt(ny + i * 6), U()))
H = ny + len(notes) * 6 + 30
W = max(maxw + 30, 600)

head = ('(kicad_sch (version 20250114) (generator "eeschema") (generator_version "9.0") (uuid "%s") (paper "User" %s %s) '
        '(title_block (title "WordClock Controller ESP32-S3") (date "2026-10-17") (rev "4.0-Entwurf") '
        '(comment 1 "(c) Daniel Kocher") (comment 2 "Entwurf aus V2-Netzliste, nicht gelayoutet"))'
        % (ROOT_UUID, fmt(max(W, H)), fmt(min(W, H)) if False else fmt(H)))
# Papier: Breite W, Hoehe H
head = head.replace('(paper "User" %s %s)' % (fmt(max(W, H)), fmt(H)), '(paper "User" %s %s)' % (fmt(W), fmt(H)))
libtxt = "(lib_symbols " + " ".join(libblocks[k] for k in sorted(libblocks)) + ")"
body = "\n".join(out_texts + out_rects + out_wires + out_labels + out_nc + out_syms)
tail = '(sheet_instances (path "/" (page "1"))) (embedded_fonts no))'
os.makedirs(os.path.dirname(OUT), exist_ok=True)
open(OUT, "w", encoding="utf-8").write(head + "\n" + libtxt + "\n" + body + "\n" + tail + "\n")
print("geschrieben:", OUT, "Teile:", len(inst), "Papier: %sx%s" % (fmt(W), fmt(H)))
