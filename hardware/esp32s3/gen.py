#!/usr/bin/env python3
"""Erzeugt den V3-Schaltplan (ESP32-S3-Umbau) aus dem V2-Schaltplan.

Vorgehen (Vorgabe des Nutzers vom 17.10.2026: ein Plan wie V2, mit gezeichneten Draehten):

  1. Die Soll-Netzliste (expected.json) entsteht wie bisher aus der V2-Netzliste (kicad-cli-Export, v2/nets.json)
     plus der Aenderungsliste unten. Sie haengt NICHT von der Zeichnung ab; vergleich.py (H2) prueft die
     Zeichnung dagegen, nach Pin-Mengen.
  2. Die Zeichnung beginnt mit einer KOPIE des V2-Blatts (A3, dieselben Koordinaten). Unveraenderte Bloecke bleiben
     Element fuer Element erhalten (Bauteile, Draehte, Junctions, Beschriftungen, Texte, Rahmen). Entfernt werden
     die Bloecke des STM32, des ESP-12F, des Oszillators, von SWD und BOOT1 samt Rahmen und Titel.
  3. In den frei gewordenen Flaechen entstehen neu gezeichnet: ESP32-S3-WROOM-1 (U2) mit EN-/BOOT-Beschaltung und
     Abblockung, USB-C (USB1, U11, C33, R34/R35, C34/C35/R36), dazu kleine Aenderungen an Reset, UART,
     PowerSupply (Buchse entfaellt dort) und LED-Board-Connector (C32).

Jede Abweichung vom V2-Blatt steht im Abschnitt AENDERUNGEN und im Bericht des Aufrufers. V2 wird nur gelesen.
"""
import sys, re, json, os, copy, math, random, textwrap
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import sx, sch
from sch import *

V2DIR = os.path.expanduser("~/Documents/WordClock-USB C - STM32F411 - V2")
V2SCH = V2DIR + "/WordClock-USB C - STM32F411 - V2.kicad_sch"
SYMDIR = "/Applications/KiCad/KiCad.app/Contents/SharedSupport/symbols/"
OUT = sys.argv[1] if len(sys.argv) > 1 else HERE + "/WordClock-ESP32S3-V3.kicad_sch"
PROJECT = os.path.basename(OUT)[:-len(".kicad_sch")]
LABELKIND = os.environ.get("GEN_LABELS", "local")      # 'local' (Voreinstellung, siehe Bericht) oder 'global' (wie V2)
WRITE_EXPECTED = os.environ.get("GEN_NO_EXPECTED") is None

rng = random.Random(20261017)
def U(): return str(uuid.UUID(int=rng.getrandbits(128), version=4))
ROOT_UUID = U()

v2text = open(V2SCH, encoding="utf-8").read()
root = parse(v2text)
v2sym = {}                    # Referenz -> V2-Symbolknoten (Original, wird nicht veraendert)
for el in root:
    if isinstance(el, list) and el and el[0] == "symbol" and child(el, "lib_id"):
        v2sym[prop(el, "Reference")] = el

# ====================================================================================================
# 1. Soll-Netzliste und Bauteilliste (unabhaengig von der Zeichnung)
# ====================================================================================================
inst = {}
for ref, el in v2sym.items():
    lib = sval(child(el, "lib_id")[1])
    if lib.startswith("power:"): continue
    flags = {c[0]: c[1] for c in el if isinstance(c, list) and c and c[0] in ("in_bom", "on_board", "dnp", "exclude_from_sim")}
    inst[ref] = dict(lib=lib, props=[(sval(c[1]), sval(c[2])) for c in el if isinstance(c, list) and c and c[0] == "property"], flags=flags)

nets_v2 = json.load(open(HERE + "/v2/nets.json"))
conn = {}
for name, nodes in nets_v2:
    if name.startswith("unconnected-"): continue
    for ref, pin, _ in nodes: conn[(ref, pin)] = name

def stock(lib, name):
    t = open(SYMDIR + lib + ".kicad_sym", encoding="utf-8").read()
    b = sx.find_symbol(t, name)
    m = re.search(r'\(extends "([^"]+)"\)', b)
    if m:
        base = sx.find_symbol(t, m.group(1))
        base = base.replace('(symbol "%s"' % m.group(1), '(symbol "%s"' % name, 1)
        base = base.replace('"%s_' % m.group(1), '"%s_' % name)
        for pm_ in re.finditer(r'\n\t\t\(property "([^"]+)"', b):
            blk = sx.block_at(b, pm_.start() + 1)
            om = re.search(r'\n\t\t\(property "%s"' % re.escape(pm_.group(1)), base)
            if om:
                ob = sx.block_at(base, om.start() + 1)
                base = base.replace(ob, blk, 1)
        b = base
    return b.replace('(symbol "%s"' % name, '(symbol "%s:%s"' % (lib, name), 1)

new_libs = {}                     # lib_id -> Symbolknoten (aus der KiCad-Standardbibliothek)
def reg_stock(lib, name):
    lid = lib + ":" + name
    new_libs[lid] = parse(stock(lib, name))
    return lid
lid_mod = reg_stock("RF_Module", "ESP32-S3-WROOM-1")
lid_usb = reg_stock("Connector", "USB_C_Receptacle_USB2.0_16P")
lid_esd = reg_stock("Power_Protection", "USBLC6-2SC6")
def libprops(lid):
    d = {}
    for c in children(new_libs[lid], "property"): d[sval(c[1])] = sval(c[2])
    return d
modpins = {}
for p in sx.pins_of(dump(new_libs[lid_mod])): modpins.setdefault(p["name"], []).append(p["num"])
esdpins = sx.pins_of(dump(new_libs[lid_esd]))

# ---- Entfallen gegenueber V2 (Referenz -> Grund), nichts verschwindet stillschweigend
ENTFALLEN = {
    "U2": "STM32F411 (ersetzt durch das Modul, das die Referenz U2 neu bekommt)", "U4": "ESP-12F (das S3-Modul ersetzt STM32 und ESP8266)",
    "X1": "Quarz (der S3 braucht keinen)", "C3": "Quarz-Lastkondensator", "C5": "Quarz-Lastkondensator",
    "C9": "VCAP1 des STM32", "C11": "VDD-Entkopplung STM32 (VDDA-Filter)", "C14": "VDDA-Entkopplung STM32", "L1": "VDDA-Filter STM32",
    "R7": "Quarz-Parallelwiderstand (V2 unbestueckt)", "R2": "Reset-Pull-up des STM32; R12 uebernimmt EN",
    "R10": "BOOT1-Widerstand (der S3 hat keinen BOOT1)", "R14": "Boot-Beschaltung ESP8266", "R15": "Boot-Beschaltung ESP8266",
    "R16": "Boot-Beschaltung ESP8266", "R18": "Boot-Beschaltung ESP8266", "SW4": "RUN/PROG-Umschalter (UART0 des Moduls, kein Umschalten)",
    "H3": "SWD-Stecker (Programmierung ueber USB/UART0)", "D1": "SWD-Diode", "USB1": "alte USB-C-Buchse 6P (V3: 16P, neues Symbol und Footprint)",
}
for r in ENTFALLEN: inst.pop(r)
for k in [k for k in conn if k[0] in ENTFALLEN]: del conn[k]

# ---- Korrekturen an Bestandsteilen (V3, V2 bleibt unberuehrt)
def setprop(ref, key, val):
    inst[ref]["props"] = [(k, (val if k == key else v)) for k, v in inst[ref]["props"]]
    if not any(k == key for k, _ in inst[ref]["props"]): inst[ref]["props"].append((key, val))
for r in ("R27", "R30"): setprop(r, "Footprint", "Resistor_SMD:R_0805_2012Metric")      # V2: C_0805 an einem Widerstand (Footprint-Analyst)
setprop("R17", "LCSC", "C17477")                                    # V2 ohne Nummer; 0 Ohm 0805 wie R13 (DNP, ohne Wirkung auf die Stueckliste)
setprop("R29", "Value", "31.6kΩ")                                   # AE-08: Wert des bestellten Teils C2930156
setprop("C1", "Value", "1uF"); setprop("C1", "LCSC", "C28323")      # EN-Verzoegerung 10 kOhm + 1 uF (V2: 100 nF); H8 prueft, dass C1 nicht im LED-Pfad liegt
setprop("R23", "LCSC", "C17520")                                    # V2 trug C17841 (= 82 Ohm); 2,2 kOhm 0805 ist C17520

TEMPLATE = {}                         # neues Bauteil -> V2-Bauteil, dessen Knoten als Vorlage dient
def clone(ref, src, value=None, lcsc=None):
    d = copy.deepcopy(inst[src]); TEMPLATE[ref] = src
    d["props"] = [(k, (ref if k == "Reference" else (value if (k == "Value" and value) else (lcsc if (k in ("LCSC", "LCSC Part") and lcsc) else v)))) for k, v in d["props"]]
    inst[ref] = d
def newpart(ref, lid, value, footprint, lcsc):
    bp = libprops(lid)
    inst[ref] = dict(lib=lid, flags={}, props=[("Reference", ref), ("Value", value), ("Footprint", footprint), ("Datasheet", bp.get("Datasheet", "")),
                                              ("Description", bp.get("Description", "")), ("LCSC", lcsc)])
newpart("U2", lid_mod, "ESP32-S3-WROOM-1-N8", libprops(lid_mod).get("Footprint", ""), "C2913198")
newpart("USB1", lid_usb, "USB-C 16P", "Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12", "C165948")
newpart("U11", lid_esd, "USBLC6-2SC6", libprops(lid_esd).get("Footprint", ""), "C7519")
clone("SW5", "SW1"); clone("R32", "R12", "10kΩ"); clone("C30", "C8", "22uF"); clone("C31", "C12", "100nF"); clone("C32", "C12", "100nF")
clone("C33", "C12", "100nF"); clone("R33", "R27", "510Ω"); clone("R34", "R13", "0Ω"); clone("R35", "R13", "0Ω")
clone("C34", "C12", "DNP (Wert offen)", ""); clone("C35", "C12", "DNP (Wert offen)", ""); clone("R36", "R12", "DNP (Wert+Schiene ungeprueft)", "")
for _r in ("C34", "C35", "R36", "C32"):                        # DNP wie V2 (R7, R17): dnp ja UND nicht in der Stueckliste/den Bestueckungsdateien
    inst[_r]["flags"]["dnp"] = "yes"; inst[_r]["flags"]["in_bom"] = "no"
for _r in ("C34", "C35", "R36"):
    inst[_r]["props"] = [(k, v) for k, v in inst[_r]["props"] if k not in ("LCSC", "LCSC Part")]
LCSC_SET = {"U2": "C2913198", "SW5": "C2909692", "R32": "C17414", "C31": "C49678", "C32": "C49678", "C30": "C45783",
            "C33": "C49678", "R33": "C17734", "R34": "C17477", "R35": "C17477"}
for _r, _n in LCSC_SET.items():
    props = inst[_r]["props"]
    inst[_r]["props"] = [(k, (_n if k in ("LCSC", "LCSC Part") else v)) for k, v in props]
    if not any(k == "LCSC" for k, _ in props): inst[_r]["props"].append(("LCSC", _n))

def setnet(ref, pin, net): conn[(ref, pin)] = net
def modpin(name, net):
    for n in modpins[name]: setnet("U2", n, net)
for n_, v_ in (("IO5", "WS2812_BUS"), ("IO6", "WS2812_POWER"), ("IO8", "SDA_DS3231"), ("IO9", "SCL_DS3231"), ("IO2", "DS18B20"), ("IO1", "LDR"),
               ("IO7", "TSOP"), ("IO15", "DCF77"), ("IO16", "USER"), ("IO17", "WPS"), ("IO18", "LED_STM"), ("USB_D-", "USB_DN_M"),
               ("USB_D+", "USB_DP_M"), ("TXD0", "UART0_TXD"), ("RXD0", "TX_UART"), ("EN", "EN"), ("IO0", "BOOT"), ("3V3", "VCC_3.3V"), ("GND", "GND")):
    modpin(n_, v_)
for k, v in list(conn.items()):
    if v in ("RESET", "ESP_EN"): conn[k] = "EN"
setnet("H6", "2", "TX_UART"); setnet("H6", "3", "RX_UART")
setnet("R33", "1", "UART0_TXD"); setnet("R33", "2", "RX_UART")
setnet("SW5", "1", "BOOT"); setnet("SW5", "2", "BOOT"); setnet("SW5", "3", "GND")
setnet("R32", "1", "BOOT"); setnet("R32", "2", "VCC_3.3V")
setnet("C30", "1", "VCC_3.3V"); setnet("C30", "2", "GND"); setnet("C31", "1", "VCC_3.3V"); setnet("C31", "2", "GND")
setnet("C32", "1", "LDR"); setnet("C32", "2", "GND")
for p in ("A4", "A9", "B4", "B9"): setnet("USB1", p, "USB_5V_INPUT")
for p in ("A1", "A12", "B1", "B12", "SH"): setnet("USB1", p, "GND")
setnet("USB1", "A5", "USB_CC1"); setnet("USB1", "B5", "USB_CC2")           # V2: Net-(USB1-CC1/CC2); sichtbare Namen an der Blockgrenze
for k in [k for k, v in conn.items() if v == "Net-(USB1-CC1)"]: conn[k] = "USB_CC1"
for k in [k for k, v in conn.items() if v == "Net-(USB1-CC2)"]: conn[k] = "USB_CC2"
setnet("USB1", "A6", "USB_DP"); setnet("USB1", "B6", "USB_DP"); setnet("USB1", "A7", "USB_DN"); setnet("USB1", "B7", "USB_DN")
setnet("R34", "1", "USB_DP"); setnet("R34", "2", "USB_DP_M"); setnet("R35", "1", "USB_DN"); setnet("R35", "2", "USB_DN_M")
setnet("C33", "1", "USB_5V_INPUT"); setnet("C33", "2", "GND")
setnet("C34", "1", "USB_DP_M"); setnet("C34", "2", "GND"); setnet("C35", "1", "USB_DN_M"); setnet("C35", "2", "GND")
setnet("R36", "1", "USB_DP_M"); setnet("R36", "2", "VCC_3.3V")
for p in esdpins:
    n = p["name"]
    if n == "GND": setnet("U11", p["num"], "GND")
    elif n == "VBUS": setnet("U11", p["num"], "USB_5V_INPUT")
    elif n == "I/O1": setnet("U11", p["num"], "USB_DP")
    elif n == "I/O2": setnet("U11", p["num"], "USB_DN")

expected = {}
for (ref, pin), net in conn.items(): expected.setdefault(net, set()).add("%s.%s" % (ref, pin))
if WRITE_EXPECTED:
    json.dump({"nets": {k: sorted(v) for k, v in sorted(expected.items())},
               "parts": {r: [d["lib"], dict(d["props"]).get("Value", "")] for r, d in sorted(inst.items())}},
              open(HERE + "/expected.json", "w"), indent=1)
singles = {k: sorted(v) for k, v in expected.items() if len(v) < 2}
if singles: print("Netze mit einem Pin (bewusst benannte Hinweis-Labels aus V2):", sorted(singles))

# ====================================================================================================
# 2. Zeichnung: V2-Blatt kopieren, entfernen, ergaenzen
# ====================================================================================================
def f2(x): return round(float(x), 2)
def pt_of(n):
    at = child(n, "at"); return (f2(at[1]), f2(at[2]))
def bbox_pts(pts):
    xs = [p[0] for p in pts]; ys = [p[1] for p in pts]; return (min(xs), min(ys), max(xs), max(ys))

# Blockrahmen und -titel des V2-Blatts
areas = {}; titles = {}
for e in root:
    if isinstance(e, list) and e and e[0] == "rule_area":
        pts = [(f2(c[1]), f2(c[2])) for c in child(child(e, "polyline"), "pts")[1:]]
        areas[id(e)] = bbox_pts(pts)
texts_v2 = [(sval(e[1]), pt_of(e), e) for e in root if isinstance(e, list) and e and e[0] == "text"]
name_of = {}
for k, a in areas.items():
    ts = [t for t in texts_v2 if a[0] - 1 <= t[1][0] <= a[2] and a[1] - 6 <= t[1][1] <= a[1] + 1]
    name_of[k] = ts[0][0]
def inb(p, a, m=0.01): return a[0] - m <= p[0] <= a[2] + m and a[1] - m <= p[1] <= a[3] + m
V2BLOCKS = {name_of[k]: a for k, a in areas.items()}
GONE = ["Oscillator", "STM32 Prog.", "BOOT1", "ESP12F", "STM32"]            # Bloecke, die entfallen (samt Rahmen, Titel, allem darin)
gone_boxes = [V2BLOCKS[n] for n in GONE]
def in_gone(p): return any(inb(p, a) for a in gone_boxes)

elems = []; stats = dict(kopiert=0, entfernt=0)
for e in root:
    if not (isinstance(e, list) and e): continue
    k = e[0]
    if k in ("wire",):
        a, b = [(f2(c[1]), f2(c[2])) for c in child(e, "pts")[1:]]
        if in_gone(a) and in_gone(b): stats["entfernt"] += 1; continue
        assert not (in_gone(a) or in_gone(b)), "Draht ueber eine Blockgrenze: %s %s" % (a, b)
    elif k in ("junction", "no_connect", "global_label"):
        if in_gone(pt_of(e)): stats["entfernt"] += 1; continue
    elif k == "text":
        if in_gone(pt_of(e)) or sval(e[1]).startswith("1-2 -> RUN") or sval(e[1]) in GONE: stats["entfernt"] += 1; continue
    elif k == "rule_area":
        if name_of[id(e)] in GONE: stats["entfernt"] += 1; continue
    elif k == "symbol" and child(e, "lib_id"):
        ref = prop(e, "Reference"); lid = sval(child(e, "lib_id")[1])
        if (not lid.startswith("power:") and ref in ENTFALLEN) or in_gone(pt_of(e)): stats["entfernt"] += 1; continue
    elif k in ("lib_symbols", "title_block", "version", "generator", "generator_version", "uuid", "paper", "sheet_instances", "embedded_fonts"): continue
    elems.append(e); stats["kopiert"] += 1

def find(kind, pred, n=1):
    r = [e for e in elems if e[0] == kind and pred(e)]
    assert len(r) == n, "%s: %d Treffer statt %d" % (kind, len(r), n)
    return r
def wire_ends(e): return [(f2(c[1]), f2(c[2])) for c in child(e, "pts")[1:]]
def drop_wire(a, b):
    r = find("wire", lambda e: set(wire_ends(e)) == {a, b} or wire_ends(e) in ([a, b], [b, a]))
    elems.remove(r[0])
def drop_junc(p): elems.remove(find("junction", lambda e: pt_of(e) == p)[0])
def drop_label(name, p): elems.remove(find("global_label", lambda e: sval(e[1]) == name and pt_of(e) == p)[0])
def drop_sym(ref): elems.remove(find("symbol", lambda e: prop(e, "Reference") == ref)[0])
def set_wire(a, b, na, nb):
    w = find("wire", lambda e: wire_ends(e) == [a, b])[0]
    pts = child(w, "pts"); pts[1][1] = fmt(na[0]); pts[1][2] = fmt(na[1]); pts[2][1] = fmt(nb[0]); pts[2][2] = fmt(nb[1])

new_wires = []; new_labels = []; new_syms = []; new_nc = []; new_texts = []; new_frames = []; extra = []
def W(*pts):
    for a, b in zip(pts, pts[1:]):
        assert abs(a[0] - b[0]) < 0.005 or abs(a[1] - b[1]) < 0.005, "diagonaler Draht %s %s" % (a, b)
        if abs(a[0] - b[0]) < 0.005 and abs(a[1] - b[1]) < 0.005: continue
        w = ["wire", ["pts", ["xy", fmt(f2(a[0])), fmt(f2(a[1]))], ["xy", fmt(f2(b[0])), fmt(f2(b[1]))]], ["stroke", ["width", "0"], ["type", "default"]], ["uuid", S(U())]]
        new_wires.append(w)
def eff(just=None, hide=False):
    e = ["effects", ["font", ["size", "1.27", "1.27"]]]
    if just: e.append(["justify"] + just.split())
    if hide: e.append(["hide", "yes"])
    return e
def label(name, x, y, ang):
    """Beschriftung an einem Drahtende; ang 0: Text nach rechts, 180: nach links"""
    just = {0: "left bottom", 180: "right bottom", 90: "left bottom", 270: "right bottom"}[ang]
    if LABELKIND == "global":
        new_labels.append(["global_label", S(name), ["shape", "input"], ["at", fmt(x), fmt(y), str(ang)], ["fields_autoplaced", "yes"], eff(just.split()[0]),
                           ["uuid", S(U())], ["property", S("Intersheetrefs"), S("${INTERSHEET_REFS}"), ["at", fmt(x), fmt(y), "0"], eff(just.split()[0], True)]])
    else:
        new_labels.append(["label", S(name), ["at", fmt(x), fmt(y), str(ang)], eff(just), ["uuid", S(U())]])
def nc(x, y): new_nc.append(["no_connect", ["at", fmt(x), fmt(y)], ["uuid", S(U())]])
def text(s, x, y, just="left bottom", bold=False):
    e = ["effects", ["font", ["size", "1.27", "1.27"]] + (["bold", "yes"] if bold else [])]
    if just: e.append(["justify"] + just.split())
    new_texts.append(["text", S(s), ["exclude_from_sim", "no"], ["at", fmt(x), fmt(y), "0"], e, ["uuid", S(U())]])
def frame(title, x0, y0, x1, y1):
    new_frames.append(["rule_area", ["polyline", ["pts", ["xy", fmt(x0), fmt(y0)], ["xy", fmt(x1), fmt(y0)], ["xy", fmt(x1), fmt(y1)], ["xy", fmt(x0), fmt(y1)]],
                                     ["stroke", ["width", "0"], ["type", "dash"]], ["fill", ["type", "none"]], ["uuid", S(U())]]])
    text(title, x0 + 0.0, y0 - 1.27)

# ---- Symbole
pwr_count = [200]
def translate(node, dx, dy):
    """verschiebt die Lage eines Symbolknotens samt Feldern"""
    for c in node:
        if isinstance(c, list) and c and c[0] in ("at",):
            c[1] = fmt(f2(float(c[1]) + dx)); c[2] = fmt(f2(float(c[2]) + dy))
        elif isinstance(c, list) and c and c[0] == "property":
            a = child(c, "at"); a[1] = fmt(f2(float(a[1]) + dx)); a[2] = fmt(f2(float(a[2]) + dy))
def set_instance(node, ref):
    old = child(node, "instances"); unit = "1"
    if old:
        try: unit = child(child(child(old, "project"), "path"), "unit")[1]
        except Exception: pass
        node.remove(old)
    node.append(["instances", ["project", S(PROJECT), ["path", S("/" + ROOT_UUID), ["reference", S(ref)], ["unit", unit]]]])
def retag(node):
    """neue uuid fuer Symbol und Pins"""
    for i, c in enumerate(node):
        if isinstance(c, list) and c and c[0] == "uuid": node[i] = ["uuid", S(U())]
        if isinstance(c, list) and c and c[0] == "pin": node[i] = ["pin", c[1], ["uuid", S(U())]]
def clone_sym(ref, src, x, y, mir=None):
    """Kopie des V2-Symbolknotens `src` mit neuer Referenz, an (x, y) (Drehung der Vorlage bleibt)"""
    n = copy.deepcopy(v2sym[src]); at = child(n, "at")
    translate(n, x - float(at[1]), y - float(at[2]))
    retag(n); set_prop(n, "Reference", ref); set_instance(n, ref)
    if mir: n.insert(n.index(child(n, "unit")), ["mirror", mir])
    apply_inst(n, ref)
    new_syms.append(n); return n
def apply_inst(n, ref):
    """Eigenschaften und Schalter aus der Bauteilliste (inst) uebernehmen"""
    d = inst[ref]
    for k, v in d["props"]:
        if k == "Reference": continue
        if prop(n, k) is not None: set_prop(n, k, v)
        elif v != "": set_prop(n, k, v)
    for k in ("in_bom", "on_board", "dnp", "exclude_from_sim"):
        c = child(n, k)
        if c is not None and k in d["flags"]: c[1] = d["flags"][k]
    # Bauteile ohne belegte Nummer (unbestueckte Plaetze mit offenem Wert) tragen kein LCSC-Feld, auch nicht das der Vorlage
    if not any(k in ("LCSC", "LCSC Part") for k, _ in d["props"]):
        for c in [c for c in n if isinstance(c, list) and c and c[0] == "property" and sval(c[1]) in ("LCSC", "LCSC Part")]: n.remove(c)
def textleft(n):
    """Referenz und Wert links vom Bauteil statt rechts"""
    ox = float(child(n, "at")[1])
    for c in n:
        if isinstance(c, list) and c and c[0] == "property" and sval(c[1]) in ("Reference", "Value"):
            a = child(c, "at"); a[1] = fmt(f2(2 * ox - float(a[1])))
            e = child(c, "effects"); j = child(e, "justify")
            if j: j[1] = "right"
def hide_field(n, name):
    c = child_prop(n, name); e = child(c, "effects")
    if child(e, "hide") is None: e.append(["hide", "yes"])
def move_field(n, name, x, y, just=None):
    c = child_prop(n, name); a = child(c, "at"); a[1] = fmt(x); a[2] = fmt(y)
    e = child(c, "effects"); j = child(e, "justify")
    if j: e.remove(j)
    if just: e.append(["justify"] + just.split())

pintab_new = {}
def pins_of_node(n):
    return pin_points(n, PINTAB)
def pin(ref, num):
    for n in new_syms + [e for e in elems if e[0] == "symbol"]:
        if prop(n, "Reference") == ref:
            r = [p for p in pin_points(n, PINTAB) if p[0] == num]
            assert r, "%s hat keinen Pin %s" % (ref, num)
            return r[0][3]
    raise KeyError(ref)

def power(net, x, y, kind=None, flip=False):
    """Versorgungssymbol mit dem Ursprung (= Pin) bei (x, y); Vorlage aus V2"""
    kind = kind or ("GND" if net == "GND" else "VCC")
    src = v2sym["#PWR04"] if kind == "GND" else v2sym["#PWR03"]
    n = copy.deepcopy(src); at = child(n, "at")
    translate(n, x - float(at[1]), y - float(at[2]))
    retag(n); pwr_count[0] += 1; ref = "#PWR%03d" % pwr_count[0]
    set_prop(n, "Reference", ref); set_prop(n, "Value", net); set_instance(n, ref)
    new_syms.append(n); return n

# lib_symbols: aus V2 uebernommen, was noch gebraucht wird; dazu die neuen
v2libs = {sval(s[1]): s for s in children(child(root, "lib_symbols"), "symbol")}
PINTAB = dict(lib_pin_table(root));
_tmp = ["kicad_sch", ["lib_symbols"] + list(new_libs.values())]
PINTAB.update(lib_pin_table(_tmp))
# Widerstandsnetzwerke (EasyEDA-Import): Pins als passiv (kein ERC-Fehler "Eingang nicht getrieben"), wie im bisherigen Entwurf
def fix_rn(s):
    t = dump(s).replace("(pin input line", "(pin passive line")
    return parse(t)
for lid in list(v2libs):
    if lid.startswith("EasyEDA:4D02WGJ"): v2libs[lid] = fix_rn(v2libs[lid])
PINTAB.update({lid: lib_pin_table(["kicad_sch", ["lib_symbols", v2libs[lid]]])[lid] for lid in v2libs if lid.startswith("EasyEDA:4D02WGJ")})

# ====================================================================================================
# AENDERUNGEN an kopierten Bloecken
# ====================================================================================================
# --- Reset: R2 entfaellt, R12 (EN-Pull-up, Pin 1 an EN) steht an seiner Stelle; Beschriftung RESET -> EN
r2 = v2sym["R2"]; r2at = pt_of(r2)
r12 = v2sym["R12"]; r12at = pt_of(r12)
n = clone_sym("R12", "R12", r2at[0], r2at[1])         # Vorlage R12 (rot 180, Pin 1 unten am EN-Knoten)
new_syms.remove(n); extra.append(n)                      # an die Stelle von R2 gesetzt: bleibt in der Reihenfolge der Kopie
lbl = find("global_label", lambda e: sval(e[1]) == "RESET")[0]; lbl[1] = S("EN")
# C1: Wert und LCSC aus der Bauteilliste (1 uF, C28323)
# --- UART: SW4 entfaellt; H6.2 liegt direkt an TX_UART; R33 in TXD0 (H6.3)
for nm, p in (("GPIO15_ESP", (280.67, 177.8)), ("TX_UART", (284.48, 177.8))): drop_label(nm, p)
for a, b in (((280.67, 176.53), (281.94, 176.53)), ((280.67, 177.8), (280.67, 176.53)), ((284.48, 176.53), (284.48, 177.8)),
             ((289.56, 176.53), (289.56, 181.61)), ((299.72, 173.99), (299.72, 176.53)), ((299.72, 171.45), (299.72, 173.99)), ((299.72, 168.91), (299.72, 171.45))):
    drop_wire(a, b)
for p in ((299.72, 171.45), (299.72, 173.99), (299.72, 176.53)): drop_junc(p)
drop_sym("#PWR039")
set_wire((289.56, 181.61), (326.39, 181.61), (294.64, 181.61), (326.39, 181.61))
label("TX_UART", 294.64, 181.61, 180)
# --- PowerSupply: alte USB-Buchse entfaellt; VBUS ueber das Netzsymbol USB_5V_INPUT (PWR_FLAG am Draht), CC1/CC2 als Beschriftung
for a, b in (((35.56, 24.13), (35.56, 26.67)), ((35.56, 26.67), (44.45, 26.67)), ((35.56, 36.83), (35.56, 39.37)), ((35.56, 39.37), (35.56, 43.18)),
             ((35.56, 43.18), (35.56, 46.99)), ((35.56, 46.99), (43.18, 46.99))):
    drop_wire(a, b)
for p in ((35.56, 26.67), (35.56, 43.18), (35.56, 39.37), (44.45, 26.67)): drop_junc(p)
label("USB_CC1", 35.56, 30.48, 180); label("USB_CC2", 35.56, 33.02, 180)
# --- LED-Board-Connector: C32 (ADC-Filter, unbestueckt) am LDR-Zweig; Beschriftung LDR nach links
set_wire((325.12, 217.17), (327.66, 217.17), (318.77, 217.17), (327.66, 217.17))
drop_label("LDR", (325.12, 217.17)); label("LDR", 318.77, 217.17, 180)

# --- UART: R33 (510 Ohm) in TXD0 zwischen Modul und H6.3; die V2-Beschriftung RX_UART bleibt als Netzname am Draht
r33 = clone_sym("R33", "R13", 312.42, 185.42)
W((303.53, 185.42), (308.61, 185.42)); label("UART0_TXD", 303.53, 185.42, 180); W((316.23, 185.42), (323.85, 185.42))
# --- LED-Board-Connector: C32 (unbestueckt) am LDR-Zweig
c32 = clone_sym("C32", "C12", 322.58, 220.98); textleft(c32)
move_field(c32, "Reference", 321.31, 219.71, "right"); move_field(c32, "Value", 321.31, 222.25, "right")
W((322.58, 224.79), (322.58, 227.33)); power("GND", 322.58, 227.33)

# ====================================================================================================
# NEU: ESP32-S3-WROOM-1 (U2)
# ====================================================================================================
OX, OY = 139.7, 152.4
def new_symbol(ref, lid, x, y, rot=0):
    d = inst[ref]; items = ["symbol", ["lib_id", S(lid)], ["at", fmt(x), fmt(y), str(rot)], ["unit", "1"], ["exclude_from_sim", "no"],
                            ["in_bom", d["flags"].get("in_bom", "yes")], ["on_board", "yes"], ["dnp", d["flags"].get("dnp", "no")], ["uuid", S(U())]]
    for k, v in d["props"]:
        hide = k not in ("Reference", "Value")
        items.append(["property", S(k), S(v), ["at", fmt(x), fmt(y), "0"], eff(None, hide)])
    seen = set()
    for q in PINTAB[lid]:
        if q["num"] in seen: continue
        seen.add(q["num"]); items.append(["pin", S(q["num"]), ["uuid", S(U())]])
    set_instance(items, ref); new_syms.append(items); return items
U2 = new_symbol("U2", lid_mod, OX, OY)
move_field(U2, "Reference", OX - 12.7, OY - 26.67, "left"); move_field(U2, "Value", OX + 3.81, OY + 28.6, "left")
# Pinsprache: Name steht im Symbol; die Signale laufen als Beschriftung an den Blockrand (Hub-IC wie der STM32 in V2)
left = {"EN": ("EN", None), "IO1": ("LDR", 1), "IO2": ("DS18B20", 1), "IO5": ("WS2812_BUS", 1), "IO6": ("WS2812_POWER", 1), "IO7": ("TSOP", 1),
        "IO8": ("SDA_DS3231", 1), "IO9": ("SCL_DS3231", 1), "IO15": ("DCF77", 1), "IO16": ("USER", 1)}
right = {"TXD0": "UART0_TXD", "RXD0": "TX_UART", "IO17": "WPS", "IO18": "LED_STM", "USB_D-": "USB_DN_M", "USB_D+": "USB_DP_M"}
pins_u2 = {p[1]: p for p in pin_points(U2, PINTAB) if p[0] not in ("40", "41")}
for name, (net, _) in left.items():
    p = pins_u2[name]; x, y = p[3]; W((x, y), (x - 2.54, y)); label(net, x - 2.54, y, 180)
for name, net in right.items():
    p = pins_u2[name]; x, y = p[3]; W((x, y), (x + 2.54, y)); label(net, x + 2.54, y, 0)
for p in pin_points(U2, PINTAB):
    if p[1] in ("GND", "3V3", "IO0", "EN") or p[1] in left or p[1] in right: continue
    nc(*p[3])
# GND (Pins 1, 40, 41 am selben Punkt) und 3V3 mit Abblockung
gx, gy = pins_u2["GND"][3]; W((gx, gy), (gx, gy + 2.54)); power("GND", gx, gy + 2.54)
vx, vy = pins_u2["3V3"][3]; rail_y = OY - 43.18
W((vx, vy), (vx, rail_y)); power("VCC_3.3V", vx, rail_y)
PX = 10.16                            # Rasterabstand der Kondensatoren (Referenz und Wert stehen daneben)
CR = [OX + 7.62 + k * PX for k in range(4)]; CL = [OX - 7.62 - k * PX for k in range(2)]
W((CL[-1], rail_y), (CR[-1], rail_y))
caps_r = list(zip(("C30", "C31", "C12", "C15"), CR)); caps_l = list(zip(("C16", "C19"), CL))
# vorhandene V2-Kondensatoren (C12, C15, C16, C19) stammen aus dem entfallenen STM32-Block: hier neu gesetzt, Referenzen bleiben
for ref, x in caps_r + caps_l:
    src = {"C30": "C8"}.get(ref, "C12")
    c = clone_sym(ref, src, x, rail_y + 3.81)
    if x < OX: textleft(c)
W((CR[0], rail_y + 7.62), (CR[-1], rail_y + 7.62)); W((CR[-1], rail_y + 7.62), (CR[-1], rail_y + 10.16)); power("GND", CR[-1], rail_y + 10.16)
W((CL[-1], rail_y + 7.62), (CL[0], rail_y + 7.62)); W((CL[-1], rail_y + 7.62), (CL[-1], rail_y + 10.16)); power("GND", CL[-1], rail_y + 10.16)
# BOOT (IO0): R32 Pull-up an 3,3 V, Taster SW5 nach GND; Zelle wie die Reset-Zelle in V2
io0x, io0y = pins_u2["IO0"][3]
JX = OX - 35.56
W((io0x, io0y), (JX, io0y), (JX - 8.89, io0y)); label("BOOT", io0x - 5.08, io0y, 180)
r32 = clone_sym("R32", "R12", JX, io0y - 7.62)
W((JX, io0y - 3.81), (JX, io0y)); W((JX, io0y - 11.43), (JX, io0y - 13.97)); power("VCC_3.3V", JX, io0y - 13.97)
sw5 = clone_sym("SW5", "SW1", JX - 8.89, io0y - 2.54, mir="y")
swp = {p[0]: p[3] for p in pin_points(sw5, PINTAB)}
W(swp["2"], (swp["2"][0], swp["2"][1] - 2.54), (swp["1"][0], swp["1"][1] - 2.54), swp["1"])
W(swp["3"], (swp["3"][0], swp["3"][1] + 2.54)); power("GND", swp["3"][0], swp["3"][1] + 2.54)
nc(*swp["4"])
move_field(sw5, "Reference", swp["2"][0], io0y - 6.35, "right"); hide_field(sw5, "Value")
frame("ESP32-S3-WROOM-1 (U2)", 83.82, 100.33, 190.5, 190.5)
for i_, ln_ in enumerate(["IO0 = BOOT (Strapping)", "IO3/IO45/IO46 offen", "EN: R12, C1, SW3 (Block Reset)"]): text(ln_, 86.36, 183.5 + i_ * 2.54)

# ====================================================================================================
# NEU: USB-C (USB1, U11, C33, R34/R35, C34/C35/R36)
# ====================================================================================================
UX, UY = 104.14, 228.6
USB1 = new_symbol("USB1", lid_usb, UX, UY)
move_field(USB1, "Reference", UX - 8.89, UY - 22.86, "left"); move_field(USB1, "Value", UX - 8.89, UY - 20.32, "left")
up = {}
for p in pin_points(USB1, PINTAB): up.setdefault(p[1], []).append(p[3])
X0 = UX + 15.24
vb = up["VBUS"][0]; W(vb, (vb[0] + 2.54, vb[1])); power("USB_5V_INPUT", vb[0] + 2.54, vb[1])
cc1 = up["CC1"][0]; W(cc1, (cc1[0] + 7.62, cc1[1])); label("USB_CC1", cc1[0] + 7.62, cc1[1], 0)
cc2 = up["CC2"][0]; W(cc2, (cc2[0] + 7.62, cc2[1])); label("USB_CC2", cc2[0] + 7.62, cc2[1], 0)
for c in up["SBU1"] + up["SBU2"]: nc(*c)
gp = sorted(up["GND"])     # A1/A12/B1/B12 am selben Punkt
W(gp[0], (gp[0][0], gp[0][1] + 2.54)); power("GND", gp[0][0], gp[0][1] + 2.54)
sh = up["SHIELD"][0]; W(sh, (sh[0], sh[1] + 2.54)); power("GND", sh[0], sh[1] + 2.54)
# Datenleitungen: D- oben, D+ unten (Reihenfolge wie an der Buchse)
def rot90(n, ref_off=(1.27, -2.54), val_off=(1.27, 2.54)):
    """Bauteil waagerecht (Drehung 90) mit den Feldlagen der V2-Widerstaende (R13)"""
    at = child(n, "at"); x, y = float(at[1]), float(at[2]); at[3] = "90"
    for nm, off in (("Reference", ref_off), ("Value", val_off)):
        c = child_prop(n, nm); a = child(c, "at"); a[1] = fmt(f2(x + off[0])); a[2] = fmt(f2(y + off[1])); a[3] = "90"
        e = child(c, "effects"); j = child(e, "justify")
        if not j: e.append(["justify", "left"])
dn = sorted(up["D-"], key=lambda p: p[1]); dp = sorted(up["D+"], key=lambda p: p[1])
XJ = X0 + 2.54
W(dn[0], (XJ, dn[0][1]), (XJ, dn[1][1]), dn[1]); ydn = (dn[0][1] + dn[1][1]) / 2
W(dp[0], (XJ, dp[0][1]), (XJ, dp[1][1]), dp[1]); ydp = (dp[0][1] + dp[1][1]) / 2
RXC = 135.89
r35 = clone_sym("R35", "R13", RXC, ydn); r34 = clone_sym("R34", "R13", RXC, ydp)
for r_, off in ((r35, ((1.27, -5.08), (1.27, -2.54))), (r34, ((1.27, 2.54), (1.27, 5.08)))):
    at = child(r_, "at"); 
    for nm, o in zip(("Reference", "Value"), off):
        a = child(child_prop(r_, nm), "at"); a[1] = fmt(f2(float(at[1]) + o[0])); a[2] = fmt(f2(float(at[2]) + o[1]))
XL = 166.37                       # Blockrand rechts: Beschriftung der Modulseite
W((XJ, ydn), (RXC - 3.81, ydn)); W((RXC + 3.81, ydn), (146.05, ydn), (XL, ydn)); label("USB_DN_M", XL, ydn, 0)
W((XJ, ydp), (RXC - 3.81, ydp)); W((RXC + 3.81, ydp), (XL, ydp)); label("USB_DP_M", XL, ydp, 0)
label("USB_DN", XJ + 1.0, ydn, 0); label("USB_DP", XJ + 1.0, ydp, 0)       # Netzname der Steckerseite (verbindet mit den Beschriftungen an U11)
# C34 (D+), C35 (D-) und R36 (D+-Pull-up) unbestueckt
JP1, JP2 = 146.05, 154.94
c34 = clone_sym("C34", "C12", JP1, ydp + 3.81)
W((JP1, ydp + 7.62), (JP1, ydp + 10.16)); power("GND", JP1, ydp + 10.16)
ky = ydp + 7.62
W((JP2, ydp), (JP2, ky + 0.0))
r36 = clone_sym("R36", "R13", JP2 + 3.81 + 3.81, ky)
W((JP2, ky), (JP2 + 3.81, ky)); W((JP2 + 15.24 - 3.81 + 0.0, ky), (JP2 + 15.24 - 3.81 + 2.54, ky))
power("VCC_3.3V", JP2 + 15.24 - 3.81 + 2.54, ky)
# C35 waagerecht oberhalb der D--Leitung: Abzweig nach oben, Kondensator liegend, GND am rechten Ende
c35 = clone_sym("C35", "C12", 153.67, ydn - 5.08); rot90(c35)
W((146.05, ydn), (146.05, ydn - 5.08), (153.67 - 3.81, ydn - 5.08)); W((153.67 + 3.81, ydn - 5.08), (153.67 + 3.81 + 2.54, ydn - 5.08))
power("GND", 153.67 + 3.81 + 2.54, ydn - 5.08)
# U11 und C33 (VBUS)
UXU, UYU = 165.1, 256.54
U11 = new_symbol("U11", lid_esd, UXU, UYU)
move_field(U11, "Reference", UXU + 3.81, UYU + 7.62, "left"); move_field(U11, "Value", UXU + 3.81, UYU + 10.16, "left")
u11 = {}
for p in pin_points(U11, PINTAB): u11.setdefault(p[0], []).append(p)
def u11pin(n): return u11[n][0][3]
for pn, net, side in (("1", "USB_DP", -1), ("3", "USB_DN", -1), ("6", "USB_DP", 1), ("4", "USB_DN", 1)):
    x, y = u11pin(pn); W((x, y), (x + side * 2.54, y)); label(net, x + side * 2.54, y, 0 if side == 1 else 180)
x, y = u11pin("5"); W((x, y), (x, y - 2.54)); power("USB_5V_INPUT", x, y - 2.54)
x, y = u11pin("2"); W((x, y), (x, y + 2.54)); power("GND", x, y + 2.54)
C33X, C33Y = 124.46, 247.65
c33 = clone_sym("C33", "C12", C33X, C33Y)
W((C33X, C33Y - 3.81), (C33X, C33Y - 6.35)); power("USB_5V_INPUT", C33X, C33Y - 6.35)
W((C33X, C33Y + 3.81), (C33X, C33Y + 6.35)); power("GND", C33X, C33Y + 6.35)
frame("USB-C", 83.82, 195.58, 190.5, 273.05)
for i, ln in enumerate(["U11: Pin 1/6 = USB_DP, Pin 3/4 = USB_DN,", "ueber gleichnamige Beschriftung verbunden.",
                        "R34/R35: 0-Ohm-Bruecken in D+/D-.", "C34, C35, R36 unbestueckt (DNP), Wert offen;", "R36: Schiene UNGEPRUEFT."]):
    text(ln, 86.36, 261.0 + i * 2.54)
# R36: Wert wird nicht angezeigt (lang), Hinweistext statt dessen
hide_field(r36, "Value")
for _c in (c34, c35, c32): hide_field(_c, "Value")

# --- PowerSupply: V2 meldet im ERC zwei Fehler "Eingang Versorgung nicht getrieben" (USB_5V_PROT, ALT_5V_PROT). Je eine PWR_FLAG-Marke
#     am Draht des Netzes beseitigt sie (V3-Aenderung; V2 bleibt unberuehrt)
for _i, (_x, _y) in enumerate(((91.44, 26.67), (91.44, 60.96))):
    n = copy.deepcopy(v2sym["#FLG05"]); at = child(n, "at")
    translate(n, _x - float(at[1]), _y - float(at[2])); retag(n)
    set_prop(n, "Reference", "#FLG2%02d" % _i); set_instance(n, "#FLG2%02d" % _i); new_syms.append(n)

# ====================================================================================================
# Hinweise (Platz der entfallenen Bloecke)
# ====================================================================================================
ent = V2BLOCKS["Oscillator"]
frame("Entfaellt gegenueber V2", ent[0], ent[1], ent[2], ent[3])
for i, ln in enumerate(["STM32F411 (U2 alt), C9,", "C11, C14, L1 (VDDA-Filter)", "ESP-12F (U4), R14-R16, R18", "Oszillator X1, C3, C5, R7",
                        "SWD H3, D1; BOOT1 R10", "Reset: R2; Umschalter SW4", "Alte USB-Buchse 6P (USB1)"]):
    text(ln, ent[0] + 2.54, ent[1] + 4.5 + i * 2.54)
notes = ["V3 aus V2: Bloecke 1:1 uebernommen, neu: ESP32-S3, USB-C, BOOT, UART (R33), Reset (R12).",
         "Neu: U2, USB1, U11, C30, C31, C33, R32-R35, SW5. Geaendert: C1 1 uF, R29 31,6k.",
         "DNP: C32, C34, C35, R36. Ungeprueft: Footprints, SY8301-Reserve, Antenne."]
nl = [l for n_ in notes for l in textwrap.wrap(n_, 46, subsequent_indent="  ")]
frame("Hinweise zum Entwurf V3", 19.05, 259.6, 82.55, 259.6 + 2.54 * len(nl) + 3.0)
for i, ln in enumerate(nl): text(ln, 21.59, 264.5 + i * 2.54)
print("Hinweisblock bis y =", 259.6 + 2.54 * len(nl) + 3.0)

# ====================================================================================================
# Zusammenbau und Ausgabe
# ====================================================================================================
final = []
for e in elems:
    if e[0] == "symbol" and child(e, "lib_id"):
        ref = prop(e, "Reference"); lid = sval(child(e, "lib_id")[1])
        set_instance(e, ref)
        if not lid.startswith("power:") and ref in inst: apply_inst(e, ref)
    final.append(e)
final += extra; final += new_syms
all_syms = [e for e in final if e[0] == "symbol"]
used = {sval(child(e, "lib_id")[1]) for e in all_syms}
libnodes = [v2libs[l] for l in sorted(used) if l in v2libs] + [new_libs[l] for l in sorted(used) if l in new_libs]
missing = used - {sval(s[1]) for s in libnodes}
assert not missing, missing
# globale V2-Beschriftungen ggf. in lokale wandeln (Voreinstellung; V2-Form mit GEN_LABELS=global)
def to_local(e):
    return ["label", e[1], child(e, "at"), child(e, "effects"), child(e, "uuid")]
if LABELKIND == "local":
    final = [to_local(e) if e[0] == "global_label" else e for e in final]
final += new_wires + new_labels + new_nc + new_texts + new_frames
# Junctions an neuen Verzweigungen (nur dort, wo neue Draehte oder Pins liegen)
tmp_root = ["kicad_sch", ["lib_symbols"] + libnodes] + final
geo = Geo(tmp_root)
nw = [tuple(wire_ends(w)) for w in new_wires]
cand = {p for w in nw for p in w}
cand |= {p[3] for n_ in new_syms + extra for p in pin_points(n_, PINTAB)}
have = set(geo.junc)
def count_at(p):
    c = 0
    for a, b in geo.wires:
        if p == a or p == b: c += 1
        elif sch._on_seg(p, a, b): c += 2
    c += sum(1 for pn in geo.pins if pn[4] == p) + sum(1 for pw in geo.power if pw[1] == p)
    return c
added = []
for p in sorted(cand):
    if p not in have and count_at(p) >= 3:
        final.append(["junction", ["at", fmt(p[0]), fmt(p[1])], ["diameter", "0"], ["color", "0", "0", "0", "0"], ["uuid", S(U())]]); added.append(p)
print("Junctions neu:", len(added))
head = ["kicad_sch", ["version", "20250114"], ["generator", S("eeschema")], ["generator_version", S("9.0")], ["uuid", S(ROOT_UUID)], ["paper", S("A3")],
        ["title_block", ["title", S("WordClock Controller ESP32-S3")], ["date", S("2026-10-17")], ["rev", S("4.0-Entwurf")],
         ["comment", "1", S("(c) Daniel Kocher")], ["comment", "2", S("V3: V2-Plan mit ESP32-S3-WROOM-1, nicht gelayoutet")]]]
lines = ["(kicad_sch (version 20250114) (generator \"eeschema\") (generator_version \"9.0\") (uuid \"%s\") (paper \"A3\")" % ROOT_UUID,
         dump(head[-1]), "(lib_symbols " + " ".join(dump(l) for l in libnodes) + ")"]
lines += [dump(e) for e in final]
lines.append('(sheet_instances (path "/" (page "1"))) (embedded_fonts no))')
open(OUT, "w", encoding="utf-8").write("\n".join(lines) + "\n")
print("geschrieben:", OUT, "Bauteile:", len([e for e in all_syms if not sval(child(e, 'lib_id')[1]).startswith('power:')]),
      "kopiert:", stats["kopiert"], "entfernt:", stats["entfernt"], "neue Draehte:", len(new_wires), "Beschriftungen neu:", len(new_labels), "Labelart:", LABELKIND)
