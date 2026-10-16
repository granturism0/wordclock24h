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
clone("C30", "C8", "22uF")          # Abblockung Modul 22 uF (AE-08, Referenzschema DS-M Abb. 9-1), Klon von C8
clone("C31", "C12", "100nF")        # Abblockung Modul 0,1 uF
clone("C32", "C12", "100nF")        # Filter am ADC-Eingang (Antrag A1, ungeklaert, siehe NOCH_OFFEN)
inst["C32"]["flags"]["dnp"] = "yes"          # A1 nicht freigegeben: Platz bleibt, unbestueckt (Auftrag des Lead 17.10.2026)
clone("C33", "C12", "100nF")        # AE-02: VBUS-Kondensator am USBLC6 (U11)
clone("R33", "R27", "510Ω")         # AE-05: Serienwiderstand in TXD0 (Konsole H6)
clone("R34", "R13", "0Ω")           # AE-02: Bruecke 0 Ohm in USB D+ (Modulseite)
clone("R35", "R13", "0Ω")           # AE-02: Bruecke 0 Ohm in USB D-
# AE-02: unbestueckte Plaetze (DNP): zwei Kondensatoren nach GND, D+-Pull-up. Wert offen, kein LCSC-Feld.
clone("C34", "C12", "DNP (Wert offen)"); clone("C35", "C12", "DNP (Wert offen)")
clone("R36", "R12", "DNP (Wert+Schiene ungeprueft)")
for _r in ("C34", "C35", "R36"): inst[_r]["flags"]["dnp"] = "yes"
inst["R29"]["props"] = [(k, ("31.6kΩ" if k == "Value" else v)) for k, v in inst["R29"]["props"]]   # AE-08: Wert des bestellten Teils C2930156
for r, v in (("C1", "1uF"),):   # R22 bleibt V2-Wert 1 kOhm (LDR-Pfad 1:1, Vorgabe 17.10.2026)
    inst[r]["props"] = [(k, (v if k == "Value" else val)) for k, val in inst[r]["props"]]

# ------------------------------------------------------------------ LCSC-Nummern (Feld "LCSC", wie in V2)
# Jede Nummer ist an der Quelle geprueft (hardware/komponenten/jlcpcb-bestand.md und die Komponentenblaetter,
# Pruefdatum 2026-10-17). Nur diese Zeilen aendern oder setzen ein LCSC-Feld; alle uebrigen behalten V2.
#  - C1: V2 = 100 nF (C49678). BEWUSSTE ABWEICHUNG VON V2: V3 setzt 1 uF (EN-Verzoegerung 10 kOhm + 1 uF nach
#    Espressif), also C28323 (1 uF 50 V X7R, Basic). C1 liegt nicht im LED-, Power- oder LDR-Pfad (H8 prueft das).
#  - R23: das Feld trug C17841 (= 82 Ohm, Extended); richtig ist C17520 (2,2 kOhm 0805, Basic).
#  - C30: 22 uF (AE-08), Klon von C8; Nummer C45783 (22 uF 25 V X5R 0805, Basic, schon in der Stueckliste).
#  - R33 (Klon von R27, 510 Ohm) C17734; R34, R35 (Klone von R13, 0 Ohm) C17477; C33 (Klon von C12) C49678.
#  - C34, C35, R36: unbestueckte Plaetze (dnp ja), das LCSC-Feld bleibt leer.
#  - R29: Wert 31,6 kOhm (AE-08), LCSC C2930156 unveraendert.
#  - U2, USB1, U11: neue Teile, das Feld fehlte (die Bibliothekssymbole haben keins) und wird neu angelegt.
#  - SW5 ist Klon von SW1 und traegt dessen Nummer (C2909692); R32 (Klon von R12) C17414; C31, C32 (Klone von C12) C49678.
LCSC_SET = {
    "C1": "C28323", "R23": "C17520",
    "U2": "C2913198", "USB1": "C165948", "U11": "C7519",
    "SW5": "C2909692", "R32": "C17414", "C31": "C49678", "C32": "C49678",
    "C30": "C45783", "C33": "C49678", "R33": "C17734", "R34": "C17477", "R35": "C17477",
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
modpin("USB_D-", "USB_DN_M"); modpin("USB_D+", "USB_DP_M")   # AE-02: Modulseite hinter den 0-Ohm-Bruecken
modpin("TXD0", "UART0_TXD"); modpin("RXD0", "TX_UART")          # AE-05: TXD0 ueber R33 an RX_UART (H6.3), RXD0 <- TX_UART (H6.2)
modpin("EN", "EN"); modpin("IO0", "BOOT")
modpin("3V3", "VCC_3.3V"); modpin("GND", "GND")
# Reset/EN: RESET und ESP_EN wurden ein Netz
for k, v in list(conn.items()):
    if v in ("RESET", "ESP_EN"): conn[k] = "EN"
# Konsole: H6 hing ueber SW4 an TX/RX
setnet("H6", "2", "TX_UART"); setnet("H6", "3", "RX_UART")
setnet("R33", "1", "UART0_TXD"); setnet("R33", "2", "RX_UART")
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
# AE-02: 0-Ohm-Bruecken, VBUS-Kondensator, unbestueckte Plaetze
setnet("R34", "1", "USB_DP"); setnet("R34", "2", "USB_DP_M")
setnet("R35", "1", "USB_DN"); setnet("R35", "2", "USB_DN_M")
setnet("C33", "1", "USB_5V_INPUT"); setnet("C33", "2", "GND")
setnet("C34", "1", "USB_DP_M"); setnet("C34", "2", "GND")
setnet("C35", "1", "USB_DN_M"); setnet("C35", "2", "GND")
setnet("R36", "1", "USB_DP_M"); setnet("R36", "2", "VCC_3.3V")   # Schiene UNGEPRUEFT (AE-02: offen, wartet auf Analyst)
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
# Anordnung wie V2: ein Blatt A3, Funktionsbloecke als gestrichelte Rahmen mit fettem Titel darueber.
# Der Abschnitt davor (Bauteile, Netze, Soll-Netzliste) ist von der Anordnung unabhaengig.
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
STUB = float(os.environ.get("GEN_STUB", "2.54"))   # Drahtstummel je Pin (ein Rasterschritt)
POWER = {"GND": "power:GND", "VCC_3.3V": "power:VCC", "VCC_5V": "power:VCC"}
FONT = 1.27           # Schriftgroesse wie V2 (Titel, Hinweise, Beschriftungen)
def tw(s): return 1.3 * len(s) + 1.0      # Textbreite in mm, gemessen an KiCad-Export: 'WS2812_POWER' 14,5 mm bei 12 Zeichen
def snap(v, g=2.54): return round(v / g) * g
def fmt(x): return ("%.2f" % x).rstrip("0").rstrip(".")

def gfx_points(lid):
    """Punkte der Symbolgrafik (Bibliotheksrichtung, y nach oben)"""
    pts = []
    def walk(n):
        if not isinstance(n, list) or not n: return
        h = n[0]
        if h in ("pin", "property"): return
        if h == "xy": pts.append((float(n[1]), float(n[2]))); return
        if h in ("start", "end", "mid") and len(n) >= 3: pts.append((float(n[1]), float(n[2]))); return
        if h == "circle":
            cx = cy = r = None
            for c in n[1:]:
                if isinstance(c, list) and c[0] == "center": cx, cy = float(c[1]), float(c[2])
                if isinstance(c, list) and c[0] == "radius": r = float(c[1])
            if cx is not None and r is not None:
                pts.extend([(cx - r, cy - r), (cx + r, cy + r)])
            return
        for c in n[1:]: walk(c)
    walk(parse(libblocks[lid]))
    return pts

def rotp(x, y, rot):
    th = math.radians(rot); c = round(math.cos(th)); s = round(math.sin(th))
    return x * c - y * s, -(x * s + y * c)          # Bildschirmkoordinaten (y nach unten)

def xf(p, rot):
    rx, ry = rotp(p["x"], p["y"], rot)
    a = math.radians(p["a"]); ox, oy = -math.cos(a), -math.sin(a)
    sx, sy = rotp(ox, oy, rot)
    return round(rx, 3), round(ry, 3), int(round(sx)), int(round(sy))

def rot_of(ref):
    """Zweipolige senkrechte Symbole mit Versorgungsnetz: GND unten, Versorgung oben, Signal gegenueber;
    sonst waagerecht (rot 90) bzw. wie in der Bibliothek (rot 0)"""
    lid = inst[ref]["lib"]; ps = pins_cache[lid]
    if len(ps) == 2 and all(p["a"] in (90, 270) for p in ps):
        nets = [conn.get((ref, p["num"])) for p in ps]
        if any(n in POWER for n in nets):
            role = []
            for i, n in enumerate(nets):
                o = nets[1 - i]
                if n == "GND": role.append("bot")
                elif n in POWER: role.append("top")
                else: role.append("top" if o == "GND" else "bot")
            if role[0] == role[1]: role = ["top", "bot"]
            for rot in (0, 180):
                if all((xf(p, rot)[1] < 0) == (role[i] == "top") for i, p in enumerate(ps)): return rot
        return 90
    return 0

def prop_angle(rot): return {0: 0, 90: 270, 180: 0, 270: 90}[rot]   # Feldwinkel ist relativ zum Symbol; bei 180 Grad bliebe der Text kopfstehend, dort kehrt KiCad stattdessen die Ausrichtung um

def R(x0, y0, x1, y1): return (min(x0, x1), min(y0, y1), max(x0, x1), max(y0, y1))

def pwr_rect(net, ex, ey, sx, sy):
    """Versorgungssymbol zeigt in Richtung des Drahtstummels nach aussen, der Wertetext steht dahinter"""
    if sx:
        a = ex + sx * (3.5 + tw(net)); return R(ex, ey - 1.25, a, ey + 1.25)
    half = max(1.5, tw(net) / 2); a = ey + sy * 5.3
    return R(ex - half, ey, ex + half, a)

def build(ref, ext=None):
    ext = ext or {}
    """Rechtecke (kind, group, rect) eines Bauteils samt Anschluessen, relativ zum Symbolursprung (mm, y unten),
    dazu die Anschlussliste und die Lage der Felder."""
    d = inst[ref]; lid = d["lib"]; rot = rot_of(ref)
    items = []; pins = []
    pts = [rotp(x, y, rot) for x, y in gfx_points(lid)]
    for p in pins_cache[lid]:
        dx, dy, sx, sy = xf(p, rot)
        pts.append((dx, dy))
        a = math.radians(p["a"]); pts.append(rotp(p["x"] + p["len"] * math.cos(a), p["y"] + p["len"] * math.sin(a), rot))
    bx0 = min(x for x, y in pts); bx1 = max(x for x, y in pts); by0 = min(y for x, y in pts); by1 = max(y for x, y in pts)
    items.append(("body", "body", R(bx0, by0, bx1, by1)))
    seen = set()
    for p in pins_cache[lid]:
        dx, dy, sx, sy = xf(p, rot)
        net = conn.get((ref, p["num"]))
        key = (dx, dy)
        if net is None:
            if p["type"] == "no_connect" or (key, "nc") in seen: pins.append((p, dx, dy, sx, sy, None, False)); continue
            seen.add((key, "nc")); pins.append((p, dx, dy, sx, sy, None, True))
            items.append(("nc", key, R(dx - 0.9, dy - 0.9, dx + 0.9, dy + 0.9)))
            continue
        if (key, net) in seen: pins.append((p, dx, dy, sx, sy, net, False)); continue
        seen.add((key, net)); pins.append((p, dx, dy, sx, sy, net, True))
        stub = ext.get(key, STUB)
        ex, ey = dx + sx * stub, dy + sy * stub
        items.append(("wire", key, R(dx, dy, ex, ey)))
        g = key
        if net in POWER:
            items.append(("pwr", g, pwr_rect(net, ex, ey, sx, sy)))
        else:
            L = tw(net)
            if sx == 1: r = R(ex, ey - 1.1, ex + L, ey + 0.3)
            elif sx == -1: r = R(ex - L, ey - 1.1, ex, ey + 0.3)
            elif sy == -1: r = R(ex - 1.7, ey - L, ex + 0.3, ey)
            else: r = R(ex - 1.7, ey, ex + 0.3, ey + L)
            items.append(("label", g, r))
    # Felder
    ref_t = ref; val_t = dict(d["props"]).get("Value", "")
    cx = (bx0 + bx1) / 2
    if len(pins_cache[lid]) == 2 and all(p["a"] in (90, 270) for p in pins_cache[lid]) and rot in (0, 180):
        jj = "left" if rot == 0 else "right"   # bei 180 Grad kehrt KiCad die Ausrichtung um
        rp = (bx1 + 1.8, -1.3, jj); vp = (bx1 + 1.8, 1.3, jj)
        items.append(("prop", "ref", R(rp[0], rp[1] - 0.8, rp[0] + tw(ref_t), rp[1] + 0.8)))
        items.append(("prop", "val", R(vp[0], vp[1] - 0.8, vp[0] + tw(val_t), vp[1] + 0.8)))
    elif rot == 90:
        rp = (cx, -6.4, None); vp = (cx, -3.7, None)
        items.append(("prop", "ref", R(cx - tw(ref_t) / 2, rp[1] - 0.8, cx + tw(ref_t) / 2, rp[1] + 0.8)))
        items.append(("prop", "val", R(cx - tw(val_t) / 2, vp[1] - 0.8, cx + tw(val_t) / 2, vp[1] + 0.8)))
    else:
        top = min(r[1] for k, g_, r in items)
        rp = (cx, top - 4.3, None); vp = (cx, top - 2.1, None)
        items.append(("prop", "ref", R(cx - tw(ref_t) / 2, rp[1] - 0.8, cx + tw(ref_t) / 2, rp[1] + 0.8)))
        items.append(("prop", "val", R(cx - tw(val_t) / 2, vp[1] - 0.8, cx + tw(val_t) / 2, vp[1] + 0.8)))
    box = (min(r[0] for k, g_, r in items), min(r[1] for k, g_, r in items),
           max(r[2] for k, g_, r in items), max(r[3] for k, g_, r in items))
    return dict(ref=ref, lid=lid, rot=rot, items=items, pins=pins, rp=rp, vp=vp, box=box, ext=ext)

def ov(a, b, eps=1e-6): return a[0] < b[2] - eps and a[2] > b[0] + eps and a[1] < b[3] - eps and a[3] > b[1] + eps
def exempt(i, j):
    (a, ka, ga, ra), (b, kb, gb, rb) = i, j
    if ra != rb: return False
    if ga == gb: return True
    return {ka, kb} <= {"body", "nc", "wire"} and ("body" in (ka, kb))
STUBS = [STUB, 5.08, 7.62, 10.16, 12.7, 15.24, 17.78, 20.32, 25.4, 30.48, 40.64]
def solve(ref):
    """Verlaengert Stummel von Versorgungspins, deren Symbol (samt Wertetext) sonst ein anderes Teil derselben Anordnung beruehrt"""
    ext = {}
    for _ in range(40):
        P = build(ref, ext)
        its = [(rc, k, g, ref) for k, g, rc in P["items"]]
        todo = set()
        for i in range(len(its)):
            for j in range(i + 1, len(its)):
                if ov(its[i][0], its[j][0]) and not exempt(its[i], its[j]):
                    for it in (its[i], its[j]):
                        if it[1] == "pwr": todo.add(it[2])
        if not todo: return P
        grew = False
        for key in todo:
            cur = ext.get(key, STUB); nxt = [s for s in STUBS if s > cur + 1e-9]
            if nxt: ext[key] = nxt[0]; grew = True
        if not grew: return P
    return P

parts = {ref: solve(ref) for ref in inst}

# ------------------------------------------------------------------ Bloecke (Gliederung wie V2)
# (Titel, Bauteile, Rahmen x, y, Breite, Hoehe | None = automatisch). Bauteilnamen der Kopfzeile in V2: PowerSupply,
# Reset, Led STM, ESP12F, LED Power, LED Controll, Switches, Oscillator, STM32 Prog., TSOP Ersatz, BOOT1, DCF77 Ersatz,
# DS18B20, STM32, DS3231, UART, LED-Board-Connector.
BLOCKS = {
 "PowerSupply": ["R3", "R4", "D4", "F1", "TP1", "TP4", "JP3", "C4", "C20", "D5", "F2", "H1", "H2", "TP5", "Q3", "R24", "R25",
     "C2", "C21", "C22", "TP6", "U9", "R26", "C24", "C23", "U10", "L2", "C25", "R28", "R29", "D2", "C27", "C28", "C29", "C26",
     "C7", "C8", "TP7", "TP8", "LED4", "R27", "LED5", "R30"],
 "USB-C": ["USB1", "U11", "C33", "R34", "R35", "C34", "C35", "R36"],
 "Reset": ["SW3", "R12", "C1"],
 "Led STM": ["LED1", "R6"],
 "ESP32-S3-WROOM-1 mit Strapping, BOOT und EN": ["U2", "SW5", "R32", "C30", "C31", "C12", "C15", "C16", "C19"],
 "LED Power": ["JP2", "Q1", "Q2", "R19", "R20", "R21", "R23", "C10"],
 "LED Controll": ["U3", "R11", "R13", "R17", "R31", "C6"],
 "Switches": ["SW1", "SW2", "R1"],
 "TSOP Ersatz": ["R8"],
 "DCF77 Ersatz": ["R9"],
 "DS18B20": ["U1", "R5"],
 "DS3231": ["U5", "U7", "U6", "RN1", "RN2", "C17", "C18"],
 "UART": ["H6", "D3", "R33"],
 "LED-Board-Connector": ["H4", "H5", "H7", "R22", "C32"],
 "Befestigung": ["H8", "H9", "H10"],
}
assigned = [r for rs in BLOCKS.values() for r in rs]
dup = {r for r in assigned if assigned.count(r) > 1}
if dup: print("WARNUNG: doppelt zugeordnet:", sorted(dup))
rest = [r for r in inst if r not in assigned]
if rest: print("WARNUNG: ohne Block:", rest); BLOCKS["Sonstiges"] = rest
missing = [r for r in assigned if r not in inst]
if missing: print("WARNUNG: Referenzen im Layout ohne Bauteil:", missing)

PADP = 2.4      # Rand um jedes Bauteil (Platz fuer das Einrasten aufs Raster)
PADF = 3.0      # Rand zwischen Rahmen und Bauteilen
def pack(refs, W):
    """Regalpackung, hoehensortiert; liefert Lagen (Kastenecke links oben) und Rahmeninnenmasse"""
    order = sorted(refs, key=lambda r: (-(parts[r]["box"][3] - parts[r]["box"][1]), r))
    pos = {}; x = 0.0; y = 0.0; rowh = 0.0; wmax = 0.0
    for r in order:
        b = parts[r]["box"]; w = b[2] - b[0] + 2 * PADP; h = b[3] - b[1] + 2 * PADP
        if x > 0 and x + w > W: x = 0.0; y += rowh; rowh = 0.0
        pos[r] = (x, y); x += w; rowh = max(rowh, h); wmax = max(wmax, x)
    return pos, wmax, y + rowh

# ------------------------------------------------------------------ Hinweistexte und Markierungen als Pseudo-Bauteile
import textwrap
def note_part(lines):
    w = max(tw(l) for l in lines); h = len(lines) * 2.54
    return dict(kind="note", lines=lines, box=(0, 0, w, h), items=[("note", "note", (0, 0, w, h))])

def flag_part(net):
    # Ursprung = oberes Drahtende. GND/Netzbezeichner: PWR_FLAG oben, Draht 5,08 mm nach unten. VCC: Symbol oben, PWR_FLAG (gedreht) unten.
    up = net in POWER and POWER[net] != "power:GND"
    items = [("wire", "w", R(0, 0, 0, 5.08))]
    if up:
        items.append(("pwr", "w", pwr_rect(net, 0, 0, 0, -1)))
        items.append(("flag", "f", R(-5.7, 5.08, 5.7, 9.9)))
    else:
        items.append(("flag", "f", R(-5.7, -4.7, 5.7, 0.0)))
        if net in POWER: items.append(("pwr", "w", pwr_rect(net, 0, 5.08, 0, 1)))
        else: items.append(("label", "w", R(0, 4.0, tw(net), 5.4)))
    box = (min(r[0] for k, g, r in items), min(r[1] for k, g, r in items), max(r[2] for k, g, r in items), max(r[3] for k, g, r in items))
    return dict(kind="flag", net=net, up=up, box=box, items=items)

HINTS_UART = ["1 > VCC_3.3V", "2 > TX", "3 > RX", "4 > GND"]          # Belegung H6 wie V2
FLAGFILE = HERE + "/flags.json"
FLAGS = json.load(open(FLAGFILE)) if os.path.exists(FLAGFILE) else []

notes = [
    "Entwurf V3: Umbau von V2 auf ESP32-S3-WROOM-1, erzeugt aus der V2-Netzliste (kicad-cli). Anordnung der Bloecke wie in V2 (ein Blatt, Funktionsbloecke mit Titel).",
    "Entfallen gegenueber V2: STM32F411 (U2), ESP-12F (U4), Quarz X1 samt Lastkondensatoren, VDDA-Filter, SWD-Stecker H3, Umschalter SW4, Boot-Widerstaende des ESP8266.",
    "Neu: ESP32-S3-WROOM-1 (U2), USB-C 16P (USB1), ESD-Schutz U11 mit C33 am VBUS-Pin, 0-Ohm-Bruecken R34/R35 in D+/D- (Modulseite USB_DP_M/USB_DN_M), BOOT-Taste SW5 mit Pull-up R32, Abblockung C30 (22 uF)/C31, R33 (510 Ohm) in TXD0.",
    "Unbestueckte Plaetze (DNP, AE-02): C34/C35 (USB-Leitungen nach GND), R36 (D+-Pull-up; Wert und Schiene UNGEPRUEFT, 3,3 V nur als Platzhalter). C32 (ADC-Filter, Antrag A1): UNBESTUECKT (dnp), Freigabe des Nutzers fehlt; Platz bleibt.",
    "Geaendert: C1 = 1 uF (EN-Verzoegerung, bewusste Abweichung von V2: dort 100 nF, LCSC C28323), H6 = UART0-Konsole (TXD0 -> R33 -> RX_UART/H6.3, RXD0 <- TX_UART/H6.2, wie V2-Belegung), R29 = 31,6 kOhm (Wert des Teils C2930156).",
    "Unveraendert: Versorgung, 3,3-V-ODER-Schaltung (D2/D3), LED-Pfad (U3, Q1, Q2), LDR-Pfad (R22 = 1 kOhm wie V2), RTC, EEPROM, DS18B20, Taster, Test-Pads.",
    "Noch nicht geprueft: Footprints neuer Teile gegen die Bestueckung, 3,3-V-Reserve des SY8301, Platzierung der Antenne.",
]
notes_lines = [ln for n in notes for ln in textwrap.wrap(n, 80, subsequent_indent="    ")]
ENTFALLT = [
    "Oscillator: X1, C3, C5, R7 (der S3 braucht keinen Quarz)",
    "STM32 Prog.: H3 (SWD), D1 (Programmierung ueber USB/UART0)",
    "BOOT1: R10 (kein BOOT1-Pin)",
    "STM32: U2 STM32F411, C9, C11, C14, L1 (VDDA-Filter)",
    "ESP12F: U4, R14, R15, R16, R18 (Boot-Beschaltung ESP8266)",
    "Reset: R2 (Pull-up) und Schalter SW4 (RUN/PROG) in UART",
]

pseudo = {}      # Name -> Pseudo-Bauteil
pseudo["hint_uart"] = note_part(HINTS_UART)
pseudo["hint_entf"] = note_part(ENTFALLT)
pseudo["hint_notes"] = note_part(notes_lines)
for i, n in enumerate(FLAGS): pseudo["flag%d" % i] = flag_part(n)
def part_of(name): return parts[name] if name in parts else pseudo[name]
BLOCKS["UART"] = BLOCKS["UART"] + ["hint_uart"]
BLOCKS["Entfaellt gegenueber V2"] = ["hint_entf"]
BLOCKS["Hinweise zum Entwurf V3"] = ["hint_notes"]
BLOCKS["PWR_FLAG (Markierung fuer die Pruefung der Versorgung)"] = ["flag%d" % i for i in range(len(FLAGS))]
TITLES = {"ESP32-S3-WROOM-1 mit Strapping, BOOT und EN": "ESP32-S3-WROOM-1 (U2) mit Strapping/BOOT/EN"}

def pack(refs, W, srt=False):
    order = sorted(refs, key=lambda r: (-(part_of(r)["box"][3] - part_of(r)["box"][1]), r)) if srt else list(refs)
    pos = {}; x = 0.0; y = 0.0; rowh = 0.0; wmax = 0.0
    for r in order:
        b = part_of(r)["box"]; w = b[2] - b[0] + 2 * PADP; h = b[3] - b[1] + 2 * PADP
        if x > 0 and x + w > W: x = 0.0; y += rowh; rowh = 0.0
        pos[r] = (x, y); x += w; rowh = max(rowh, h); wmax = max(wmax, x)
    return pos, wmax, y + rowh

# Blattaufbau wie V2: oben PowerSupply (und USB-C), darunter fuenf Spalten, ganz unten Hinweise
PAPER = os.environ.get("GEN_PAPER", "A2")
PAPERSIZE = {"A3": (420.0, 297.0), "A2": (594.0, 420.0)}[PAPER]
PACKW = {  # Packbreite je Block (mm) und ob nach Hoehe sortiert wird
 "PowerSupply": (410, True), "USB-C": (130, True),
 "Reset": (60, False), "Switches": (60, False), "DS18B20": (60, False),
 "Led STM": (60, False), "TSOP Ersatz": (60, False), "DCF77 Ersatz": (60, False), "Entfaellt gegenueber V2": (100, False),
 "ESP32-S3-WROOM-1 mit Strapping, BOOT und EN": (110, True),
 "LED Power": (110, True), "DS3231": (150, True),
 "LED Controll": (110, False), "UART": (100, False), "LED-Board-Connector": (100, True), "Befestigung": (100, False),
 "Hinweise zum Entwurf V3": (110, False), "PWR_FLAG (Markierung fuer die Pruefung der Versorgung)": (100, False),
}
TOP = ["PowerSupply", "USB-C"]
COLS = [["Reset", "Switches", "DS18B20"],
        ["Led STM", ["TSOP Ersatz", "DCF77 Ersatz"], "Entfaellt gegenueber V2", "Befestigung", "PWR_FLAG (Markierung fuer die Pruefung der Versorgung)"],
        ["ESP32-S3-WROOM-1 mit Strapping, BOOT und EN", "Hinweise zum Entwurf V3"],
        ["LED Power", "DS3231"],
        ["LED Controll", "UART", "LED-Board-Connector"]]
BOTTOM = []
GAPX = 10.0; GAPY = 11.0; X0 = 16.0; Y0 = 24.0
packed = {}
for b in BLOCKS:
    W, srt = PACKW[b]
    pos, wm, hm = pack(BLOCKS[b], W, srt)
    packed[b] = (pos, max(wm + 2 * PADF, tw(TITLES.get(b, b)) * 1.15 + 2), hm + 2 * PADF)
frames = {}      # Block -> (x, y, w, h)
x = X0; ytop_h = 0.0
for b in TOP:
    w, h = packed[b][1], packed[b][2]
    frames[b] = [x, Y0, w, h]; x += w + GAPX; ytop_h = max(ytop_h, h)
y1 = Y0 + ytop_h + GAPY + 2
x = X0; col_bottom = 0.0
for col in COLS:
    cells = [c if isinstance(c, list) else [c] for c in col]
    cw = max(sum(packed[b][1] for b in c) + GAPX * (len(c) - 1) for c in cells); y = y1
    for c in cells:
        n = len(c); extra = (cw - sum(packed[b][1] for b in c) - GAPX * (n - 1))
        xx = x; hh = max(packed[b][2] for b in c)
        for b in c:
            w = packed[b][1] + (extra if b == c[-1] else 0)
            frames[b] = [xx, y, w, hh]; xx += w + GAPX
        y += hh + GAPY
    col_bottom = max(col_bottom, y - GAPY); x += cw + GAPX
y2 = col_bottom + GAPY + 2; x = X0; bot_h = 0.0
for b in BOTTOM:
    frames[b] = [x, y2, packed[b][1], packed[b][2]]; x += packed[b][1] + GAPX; bot_h = max(bot_h, packed[b][2])
for b,f in frames.items(): print("  %-45s x=%5.0f y=%5.0f w=%5.0f h=%5.0f"%(b[:45],*f))
boxsum = sum((p["box"][2] - p["box"][0] + 2 * PADP) * (p["box"][3] - p["box"][1] + 2 * PADP) for p in parts.values())
print("Summe der Bauteilkaesten (Teile mit Anschlussstummeln, Beschriftungen, Feldern, Rand %.1f): %.0f mm2; nutzbare Flaeche A3 (ohne Rand und Schriftfeld) etwa %.0f mm2" % (PADP, boxsum, (420 - 28) * (297 - 28) - 120 * 45))
need_w = max(f[0] + f[2] for f in frames.values()) + 16.0
need_h = max(f[1] + f[3] for f in frames.values()) + 16.0
print("Platzbedarf der Anordnung: %.0f x %.0f mm, Blatt %s = %.0f x %.0f mm (Stummel %.2f)" % (need_w, need_h, PAPER, PAPERSIZE[0], PAPERSIZE[1], STUB))
if need_w > PAPERSIZE[0] or need_h > PAPERSIZE[1]:
    sys.exit("FEHLER: Die Anordnung passt nicht auf %s (Bedarf %.0f x %.0f mm, Blatt %.0f x %.0f mm)" % (PAPER, need_w, need_h, PAPERSIZE[0], PAPERSIZE[1]))

# Lagen der Bauteile (Symbolursprung auf dem Raster)
abs_items = []     # (rect absolut, kind, gruppe, Besitzer)
for b, (fx, fy, fw, fh) in frames.items():
    pos = packed[b][0]
    for r, (px, py) in pos.items():
        P = part_of(r); bb = P["box"]
        ox = snap(fx + PADF + px + PADP - bb[0]); oy = snap(fy + PADF + py + PADP - bb[1])
        P["at"] = (ox, oy); P["block"] = b
        for kind, grp, rc in P["items"]:
            abs_items.append(((rc[0] + ox, rc[1] + oy, rc[2] + ox, rc[3] + oy), kind, grp, r))

# ------------------------------------------------------------------ Ueberlappungspruefung der Anordnung (eigenes Modell)
bad = []
for i in range(len(abs_items)):
    for j in range(i + 1, len(abs_items)):
        if ov(abs_items[i][0], abs_items[j][0]) and not exempt(abs_items[i], abs_items[j]):
            bad.append((abs_items[i][3], abs_items[i][1], abs_items[j][3], abs_items[j][1]))
# Rahmen und Titel
frect = {b: (f[0], f[1], f[0] + f[2], f[1] + f[3]) for b, f in frames.items()}
trect = {b: (f[0], f[1] - 3.6, f[0] + tw(TITLES.get(b, b)) * 1.15, f[1] - 0.4) for b, f in frames.items()}
for b in frames:
    for c in frames:
        if b < c and ov(frect[b], frect[c]): bad.append(("Rahmen", b, "Rahmen", c))
    for c in frames:
        if b != c and (ov(trect[b], frect[c]) or (b < c and ov(trect[b], trect[c]))): bad.append(("Titel", b, "Rahmen/Titel", c))
for rc, kind, grp, r in abs_items:
    f = frect[part_of(r)["block"]]
    if not (rc[0] >= f[0] + 1.0 and rc[2] <= f[2] - 1.0 and rc[1] >= f[1] + 1.0 and rc[3] <= f[3] - 1.0): bad.append((r, kind, "ausserhalb Rahmen", part_of(r)["block"]))
    for b in frames:
        if b != part_of(r)["block"] and (ov(rc, frect[b]) or ov(rc, trect[b])): bad.append((r, kind, "in fremdem Rahmen/Titel", b))
    if rc[2] > PAPERSIZE[0] - 130 and rc[3] > PAPERSIZE[1] - 50: bad.append((r, kind, "im Schriftfeld", ""))
    if rc[0] < 12 or rc[1] < 12 or rc[2] > PAPERSIZE[0] - 12 or rc[3] > PAPERSIZE[1] - 12: bad.append((r, kind, "ausserhalb Blatt", ""))
print("Ueberlappungspruefung: %d Rechtecke, %d Rahmen, %d Meldungen" % (len(abs_items), len(frames), len(bad)))
for m in bad[:30]: print("  UEBERLAPPUNG", m)
if bad: sys.exit("FEHLER: Anordnung ueberlappt")

# ------------------------------------------------------------------ Ausgabe
ROWW = 0
out_wires = []; out_labels = []; out_nc = []; out_syms = []; out_texts = []; out_rects = []
pwr_n = [0]
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

def propline(name, val, x, y, hide=False, just=None, ang=0):
    j = (" (justify %s)" % just) if just else ""
    return ('(property "%s" "%s" (at %s %s %d) (effects (font (size 1.27 1.27))%s%s))'
            % (name, esc(val), fmt(x), fmt(y), ang, j, " (hide yes)" if hide else ""))

def text(s, x, y, bold=False, just="left bottom"):
    return ('(text "%s" (exclude_from_sim no) (at %s %s 0) (effects (font (size 1.27 1.27)%s) (justify %s)) (uuid "%s"))'
            % (esc(s), fmt(x), fmt(y), " (bold yes)" if bold else "", just, U()))

def power_symbol(net, X, Y, sx=0, sy=1):
    lid = POWER[net]
    gnd = lid == "power:GND"
    # GND zeigt im Bibliotheksbild nach unten, VCC nach oben; gedreht, bis die Spitze in Richtung des Stummels zeigt
    if sx: rot = (90 if sx == 1 else 270) if gnd else (270 if sx == 1 else 90)
    else:
        down = sy == 1
        rot = (0 if down else 180) if gnd else (180 if down else 0)
    if sx: vx, vy, vj = X + sx * 3.5, Y, ("left" if sx == 1 else "right")
    else: vx, vy, vj = X, Y + sy * 4.4, None
    pwr_n[0] += 1
    ref = "#PWR%03d" % pwr_n[0]
    s = ['(symbol (lib_id "%s") (at %s %s %d) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no) (uuid "%s")'
         % (lid, fmt(X), fmt(Y), rot, U()),
         propline("Reference", ref, X, Y, hide=True),
         propline("Value", net, vx, vy, just=vj, ang=prop_angle(rot)),
         propline("Footprint", "", X, Y, hide=True), propline("Datasheet", "", X, Y, hide=True),
         '(pin "1" (uuid "%s"))' % U(),
         '(instances (project "%s" (path "/%s" (reference "%s") (unit 1))))' % (PROJECT, ROOT_UUID, ref), ')']
    return " ".join(s)

def emit_net(net, ex, ey, sx, sy):
    if net in POWER: out_syms.append(power_symbol(net, ex, ey, sx, sy))
    else: out_labels.append(lab(net, ex, ey, sx, sy))

for b, (fx, fy, fw, fh) in frames.items():
    out_texts.append(text(TITLES.get(b, b), fx + 1.0, fy - 1.5, bold=True))
    out_rects.append('(rectangle (start %s %s) (end %s %s) (stroke (width 0.3) (type dash)) (fill (type none)) (uuid "%s"))'
                     % (fmt(fx), fmt(fy), fmt(fx + fw), fmt(fy + fh), U()))

for name in list(pseudo):
    P = pseudo[name]; ox, oy = P["at"]
    if P["kind"] == "note":
        for i, ln in enumerate(P["lines"]): out_texts.append(text(ln, ox, oy + 1.9 + i * 2.54))
    else:
        net = P["net"]; fx = ox; fy = oy
        pwr_n[0] += 1
        ref = "#FLG%03d" % pwr_n[0]
        py = fy + 5.08 if P["up"] else fy
        frot = 180 if P["up"] else 0
        vy = py + 3.81 if P["up"] else py - 3.81
        out_syms.append('(symbol (lib_id "power:PWR_FLAG") (at %s %s %d) (unit 1) (exclude_from_sim no) (in_bom yes) (on_board yes) (dnp no) (uuid "%s") %s %s %s %s (pin "1" (uuid "%s")) (instances (project "%s" (path "/%s" (reference "%s") (unit 1)))))'
                        % (fmt(fx), fmt(py), frot, U(), propline("Reference", ref, fx, py, hide=True), propline("Value", "PWR_FLAG", fx, vy),
                           propline("Footprint", "", fx, py, hide=True), propline("Datasheet", "", fx, py, hide=True), U(), PROJECT, ROOT_UUID, ref))
        out_wires.append(wire(fx, fy, fx, fy + 5.08))
        if P["up"]: emit_net(net, fx, fy, 0, -1)
        else: emit_net(net, fx, fy + 5.08, 0 if net in POWER else 1, 1)

for ref, P in parts.items():
    ox, oy = P["at"]; lid = P["lid"]; rot = P["rot"]; d = inst[ref]
    s = ['(symbol (lib_id "%s") (at %s %s %d) (unit 1) (exclude_from_sim %s) (in_bom %s) (on_board %s) (dnp %s) (uuid "%s")'
         % (lid, fmt(ox), fmt(oy), rot, d["flags"].get("exclude_from_sim", "no"), d["flags"].get("in_bom", "yes"),
            d["flags"].get("on_board", "yes"), d["flags"].get("dnp", "no"), U())]
    rp, vp = P["rp"], P["vp"]; ang = prop_angle(rot)
    for k, v in d["props"]:
        if k == "Reference": s.append(propline(k, v, ox + rp[0], oy + rp[1], just=rp[2], ang=ang))
        elif k == "Value": s.append(propline(k, v, ox + vp[0], oy + vp[1], just=vp[2], ang=ang))
        else: s.append(propline(k, v, ox, oy, hide=True))
    seen = set()
    for p in pins_cache[lid]:
        if p["num"] in seen: continue
        seen.add(p["num"]); s.append('(pin "%s" (uuid "%s"))' % (esc(p["num"]), U()))
    s.append('(instances (project "%s" (path "/%s" (reference "%s") (unit 1))))' % (PROJECT, ROOT_UUID, ref))
    s.append(")")
    out_syms.append(" ".join(s))
    for p, dx, dy, sx, sy, net, emit in P["pins"]:
        if not emit: continue
        X, Y = ox + dx, oy + dy
        if net is None:
            out_nc.append('(no_connect (at %s %s) (uuid "%s"))' % (fmt(X), fmt(Y), U())); continue
        ex, ey = X + sx * P["ext"].get((dx, dy), STUB), Y + sy * P["ext"].get((dx, dy), STUB)
        out_wires.append(wire(X, Y, ex, ey))
        emit_net(net, ex, ey, sx, sy)

W, H = PAPERSIZE
head = ('(kicad_sch (version 20250114) (generator "eeschema") (generator_version "9.0") (uuid "%s") (paper "%s") '
        '(title_block (title "WordClock Controller ESP32-S3") (date "2026-10-17") (rev "4.0-Entwurf") '
        '(comment 1 "(c) Daniel Kocher") (comment 2 "Entwurf aus V2-Netzliste, nicht gelayoutet"))'
        % (ROOT_UUID, PAPER))
libtxt = "(lib_symbols " + " ".join(libblocks[k] for k in sorted(libblocks)) + ")"
body = "\n".join(out_texts + out_rects + out_wires + out_labels + out_nc + out_syms)
tail = '(sheet_instances (path "/" (page "1"))) (embedded_fonts no))'
os.makedirs(os.path.dirname(OUT), exist_ok=True)
open(OUT, "w", encoding="utf-8").write(head + "\n" + libtxt + "\n" + body + "\n" + tail + "\n")
print("geschrieben:", OUT, "Teile:", len(inst), "Papier:", PAPER)
