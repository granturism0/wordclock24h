#!/usr/bin/env python3
"""Hilfsmodul fuer KiCad-Schaltplaene (.kicad_sch): Lesen, Schreiben (ein Element je Zeile), Geometrie und
geometrische Netzbildung.

Die Netzbildung hier ist NICHT die Wahrheit ueber die Platine (das ist kicad-cli, siehe vergleich.py). Sie dient
dem Zeichnen: Sie zeigt, welche Drahtenden lose sind, und erlaubt dem Generator, die Nachbarn eines Bauteils im
V2-Plan und im V3-Plan zu vergleichen, bevor KiCad gefragt wird. Ihre Gueltigkeit wird am V2-Plan gegen die
kicad-cli-Netzliste gemessen (siehe selbsttest()).
"""
import re, math, os, sys, uuid
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sx

# ------------------------------------------------------------------ Lesen und Schreiben
_ESC = re.compile(r'\\(.)')
def _unesc(s):
    return _ESC.sub(lambda m: {"n": "\n", "\\": "\\", '"': '"'}.get(m.group(1), m.group(1)), s)

def parse(text):
    toks = sx.TOK.findall(text); pos = 0
    def rd():
        nonlocal pos
        t = toks[pos]; pos += 1
        if t == "(":
            l = []
            while toks[pos] != ")": l.append(rd())
            pos += 1; return l
        if t.startswith('"'): return ("s", _unesc(t[1:-1]))
        return t
    return rd()

def esc(s): return s.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")

def dump(n):
    if isinstance(n, tuple): return '"%s"' % esc(n[1])
    if isinstance(n, list): return "(" + " ".join(dump(c) for c in n) + ")"
    return n

def S(s): return ("s", s)
def sval(x): return x[1] if isinstance(x, tuple) else x
def child(n, k):
    for c in n:
        if isinstance(c, list) and c and c[0] == k: return c
    return None
def children(n, k): return [c for c in n if isinstance(c, list) and c and c[0] == k]
def U(): return str(uuid.uuid4())
def fmt(x): return ("%.4f" % x).rstrip("0").rstrip(".") if x != int(x) else str(int(x))

def prop(sym, name):
    for c in sym:
        if isinstance(c, list) and c and c[0] == "property" and sval(c[1]) == name: return sval(c[2])
    return None
def set_prop(sym, name, value, template_from=None):
    for c in sym:
        if isinstance(c, list) and c and c[0] == "property" and sval(c[1]) == name:
            c[2] = S(value); return
    # neu anlegen, versteckt, an der Lage des Referenzfeldes
    at = child(child_prop(sym, "Reference"), "at")
    sym.insert(sym.index(child(sym, "pin")) if child(sym, "pin") else len(sym),
               ["property", S(name), S(value), ["at", at[1], at[2], "0"], ["effects", ["font", ["size", "1.27", "1.27"]], ["hide", "yes"]]])
def child_prop(sym, name):
    for c in sym:
        if isinstance(c, list) and c and c[0] == "property" and sval(c[1]) == name: return c

# ------------------------------------------------------------------ Bibliothekspins und Geometrie
def lib_pin_table(root):
    """lib_id -> Liste Pins (x, y, winkel, laenge, nummer, name, typ, einheit, stil)"""
    out = {}
    libs = child(root, "lib_symbols")
    for s in children(libs, "symbol"):
        lid = sval(s[1]); pins = []
        for sub in children(s, "symbol"):
            m = re.search(r"_(\d+)_(\d+)$", sval(sub[1]))
            unit, style = (int(m.group(1)), int(m.group(2))) if m else (0, 0)
            for p in children(sub, "pin"):
                at = child(p, "at"); ln = child(p, "length")
                pins.append(dict(x=float(at[1]), y=float(at[2]), a=int(float(at[3])), len=float(ln[1]),
                                 num=sval(child(p, "number")[1]), name=sval(child(p, "name")[1]), type=p[1],
                                 unit=unit, style=style, hide=("hide" in p) or child(p, "hide") is not None))
        for p in children(s, "pin"):    # Pins direkt im Symbol (selten)
            at = child(p, "at"); ln = child(p, "length")
            pins.append(dict(x=float(at[1]), y=float(at[2]), a=int(float(at[3])), len=float(ln[1]),
                             num=sval(child(p, "number")[1]), name=sval(child(p, "name")[1]), type=p[1], unit=0, style=0, hide=False))
        out[lid] = pins
    return out

def xform(x, y, rot, mir):
    """Symbolkoordinate (y nach oben) -> Verschiebung im Blatt (y nach unten). Erst drehen, dann spiegeln (an V2 geprueft)."""
    th = math.radians(rot); c = round(math.cos(th)); s = round(math.sin(th))
    rx, ry = x * c - y * s, -(x * s + y * c)       # Bildschirmkoordinaten (y nach unten)
    if mir == "x": ry = -ry
    elif mir == "y": rx = -rx
    return rx, ry

def placement(sym):
    at = child(sym, "at"); m = child(sym, "mirror"); u = child(sym, "unit")
    return dict(lib=sval(child(sym, "lib_id")[1]), x=float(at[1]), y=float(at[2]), rot=int(float(at[3])),
                mir=(m[1] if m else None), unit=int(u[1]) if u else 1, ref=prop(sym, "Reference"))

def pin_points(sym, pintab):
    """Liste (nummer, name, typ, (x, y) Anschlusspunkt im Blatt, Richtung nach aussen (dx, dy))"""
    p = placement(sym); out = []
    for q in pintab[p["lib"]]:
        if q["unit"] not in (0, p["unit"]) or q["style"] not in (0, 1): continue
        dx, dy = xform(q["x"], q["y"], p["rot"], p["mir"])
        a = math.radians(q["a"]); ox, oy = xform(-math.cos(a), -math.sin(a), p["rot"], p["mir"])
        out.append((q["num"], q["name"], q["type"], (round(p["x"] + dx, 2), round(p["y"] + dy, 2)), (int(round(ox)), int(round(oy))), q["hide"]))
    return out

# ------------------------------------------------------------------ Geometrische Netzbildung
def _on_seg(pt, a, b, eps=0.02):
    (x, y), (x1, y1), (x2, y2) = pt, a, b
    if abs(x1 - x2) < eps:  # senkrecht
        return abs(x - x1) < eps and min(y1, y2) - eps <= y <= max(y1, y2) + eps
    if abs(y1 - y2) < eps:
        return abs(y - y1) < eps and min(x1, x2) - eps <= x <= max(x1, x2) + eps
    return False

class Geo:
    """Punkte-Union: Drahtenden, Pins, Labels. Ein Draht verbindet sich mit einem Pin/Label/Drahtende, das auf ihm liegt;
    Drahtkreuzungen ohne Junction verbinden nicht (wie KiCad); Drahtende auf Drahtmitte verbindet (T-Stelle)."""
    def __init__(self, root):
        self.root = root; self.pintab = lib_pin_table(root)
        self.wires = []; self.labels = []; self.junc = []; self.nc = []; self.syms = []; self.pins = []; self.power = []
        for e in root:
            if not (isinstance(e, list) and e): continue
            if e[0] == "wire":
                xy = [(round(float(c[1]), 2), round(float(c[2]), 2)) for c in child(e, "pts")[1:] if isinstance(c, list) and c[0] == "xy"]
                self.wires.append((xy[0], xy[1]))
            elif e[0] in ("label", "global_label"):
                at = child(e, "at"); self.labels.append((sval(e[1]), (round(float(at[1]), 2), round(float(at[2]), 2))))
            elif e[0] == "junction":
                at = child(e, "at"); self.junc.append((round(float(at[1]), 2), round(float(at[2]), 2)))
            elif e[0] == "no_connect":
                at = child(e, "at"); self.nc.append((round(float(at[1]), 2), round(float(at[2]), 2)))
            elif e[0] == "symbol" and child(e, "lib_id"):
                self.syms.append(e)
        for s in self.syms:
            lid = sval(child(s, "lib_id")[1]); ref = prop(s, "Reference")
            for num, name, typ, pt, d, hide in pin_points(s, self.pintab):
                if lid.startswith("power:"):
                    if not (lid == "power:PWR_FLAG" and prop(s, "Value") == "PWR_FLAG"): self.power.append((prop(s, "Value"), pt, lid))
                    else: self.power.append((None, pt, lid))
                else: self.pins.append((ref, num, name, typ, pt))
        self.par = {}
        # Knoten: Drahtenden, Pins, Labels, Powerpunkte
        for a, b in self.wires: self.union(a, b)
        pts = {p for w in self.wires for p in w}
        for w in self.wires:
            for q in list(pts) + [p[4] for p in self.pins] + [l[1] for l in self.labels] + [p[1] for p in self.power] + self.junc:
                if q not in w and _on_seg(q, w[0], w[1]):
                    # eine Kreuzung ohne Junction: ein Drahtende (q ist Drahtende eines anderen Drahtes) auf der Mitte -> T-Stelle, verbindet
                    self.union(q, w[0])
        for q in [p[4] for p in self.pins] + [l[1] for l in self.labels] + [p[1] for p in self.power]:
            self.find(q)
    def find(self, a):
        self.par.setdefault(a, a)
        while self.par[a] != a:
            self.par[a] = self.par[self.par[a]]; a = self.par[a]
        return a
    def union(self, a, b): self.par[self.find(a)] = self.find(b)
    def groups(self):
        """Verbindung -> dict(pins=[(ref,num)], labels=[namen], power=[namen])"""
        g = {}
        for ref, num, name, typ, pt in self.pins: g.setdefault(self.find(pt), dict(pins=set(), labels=set(), power=set()))["pins"].add((ref, num))
        for nm, pt in self.labels: g.setdefault(self.find(pt), dict(pins=set(), labels=set(), power=set()))["labels"].add(nm)
        for nm, pt, lid in self.power:
            if nm is None: continue
            g.setdefault(self.find(pt), dict(pins=set(), labels=set(), power=set()))["power"].add(nm)
        return g
    def nets(self):
        """Netze nach Pin-Mengen. Labels gleichen Namens und Powersymbole gleichen Namens verbinden alle Gruppen (global).
        Rueckgabe: Liste (Name oder None, frozenset((ref,num)))"""
        g = self.groups(); byname = {}
        for k, v in g.items():
            for n in v["labels"] | v["power"]: byname.setdefault(n, []).append(k)
        par = {k: k for k in g}
        def f(a):
            while par[a] != a: par[a] = par[par[a]]; a = par[a]
            return a
        for n, ks in byname.items():
            for k in ks[1:]: par[f(k)] = f(ks[0])
        merged = {}
        for k, v in g.items():
            m = merged.setdefault(f(k), dict(pins=set(), names=set()))
            m["pins"] |= v["pins"]; m["names"] |= v["labels"] | v["power"]
        return [(sorted(m["names"]), frozenset(m["pins"])) for m in merged.values()]

def selbsttest(path, nets_json):
    """Netzbildung gegen die kicad-cli-Netzliste (nets.json: [[name, [[ref, pin, _]...]]...]) vergleichen."""
    import json
    root = parse(open(path, encoding="utf-8").read()); geo = Geo(root)
    mine = {pins for names, pins in geo.nets() if len(pins) > 0}
    ref = {frozenset((a[0], a[1]) for a in nodes) for name, nodes in json.load(open(nets_json)) if not name.startswith("unconnected-")}
    mine1 = {p for p in mine if len(p) > 1}; ref1 = {p for p in ref if len(p) > 1}
    return len(mine1), len(ref1), sorted(map(sorted, mine1 - ref1))[:5], sorted(map(sorted, ref1 - mine1))[:5]

if __name__ == "__main__":
    V2 = os.path.expanduser("~/Documents/WordClock-USB C - STM32F411 - V2/WordClock-USB C - STM32F411 - V2.kicad_sch")
    print(selbsttest(V2, os.path.join(os.path.dirname(os.path.abspath(__file__)), "v2", "nets.json")))
