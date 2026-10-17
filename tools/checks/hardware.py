#!/usr/bin/env python3
"""Guardrail S19: Hardware-Entwurf der ESP32-S3-Platine (hardware/esp32s3/).

Acht Pruefungen, alle statisch gegen die Dateien und gegen kicad-cli, keine mit dem Geraet:

  H1  ERC: keine Fehler, und keine Meldungsart ueber der Baseline (hardware-erc.baseline)
  H2  Netzliste des Schaltplans == Soll-Netzliste (hardware/esp32s3/vergleich.py, nach Pin-Mengen)
  H3  Pinregeln am Modul U2 (hardware-pinregeln.json): gesperrte Pins offen, feste Pins nur an
      ihrem Netz, kein 5-V-Netz an einem Modulpin, LDR an einem ADC1-Pin
  H4  Stueckliste (bom.csv): jede Zeile eine LCSC-Nummer, Designatoren == Bauteile des Schaltplans,
      JLCPCB-Pruefung nicht aelter als 14 Tage
  H5  Komponentenblaetter: Pflichtabschnitte und Seitenbeleg an jedem Haken
  H6  V2-Referenz: der gespeicherte Export stimmt noch mit dem V2-Schaltplan ueberein
  H7  Leiterplattenformat 1:1 (sobald ein Layout existiert) und DRC (nicht schlechter als V2)
  H8  LED-/Power-/LDR-Schnittstelle zum LED-Board bauteil- und pinweise gleich V2 (Vorgabe 17.10.2026)

Jede Pruefung nennt, WIE VIELE Dinge sie betrachtet hat. Eine Pruefung, die nichts sieht, meldet
sonst OK (DIR-014, BEFUNDE L235). Die Umgebungsvariablen HW_SCH, HW_NETLIST, HW_BOM, HW_PCB, HW_KOMP
und HW_V2 gibt es NUR fuer die Gegenproben in hardware-gegenproben.sh.

Exit 0: keine Kritisch-Findings. Exit 1: mindestens eines. Zeilen mit 'HOCH' sind Warnungen.
Aufruf:  python3 tools/checks/hardware.py [--nur H1,H3]
"""
import os, sys, re, json, csv, io, glob, subprocess, tempfile, datetime, collections
import xml.etree.ElementTree as ET

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
HW = os.path.join(ROOT, "hardware", "esp32s3")
KOMP = os.environ.get("HW_KOMP", os.path.join(ROOT, "hardware", "komponenten"))
sys.path.insert(0, HW)
CLI = "/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli"
SCH = os.environ.get("HW_SCH", os.path.join(HW, "WordClock-ESP32S3-V3.kicad_sch"))
V2DIR = os.environ.get("HW_V2", os.path.expanduser("~/Documents/WordClock-USB C - STM32F411 - V2"))
V2SCH = os.path.join(V2DIR, "WordClock-USB C - STM32F411 - V2.kicad_sch")
V2PCB = os.path.join(V2DIR, "WordClock-USB C - STM32F411 - V2.kicad_pcb")
BOM = os.environ.get("HW_BOM", os.path.join(HW, "bom.csv"))
PCB = os.environ.get("HW_PCB") or next(iter(sorted(glob.glob(os.path.join(HW, "*.kicad_pcb")))), None)

CRIT = 0
WARN = 0
def ok(m): print("  OK  " + m)
def crit(m):
    global CRIT; CRIT += 1; print("  KRITISCH  " + m)
def warn(m):
    global WARN; WARN += 1; print("  HOCH      " + m)
def info(m): print("  --  " + m)

def cli(*args):
    if not os.path.exists(CLI):
        return None
    return subprocess.run([CLI, *args], capture_output=True, text=True)

_netlist_cache = {}
def netlist(sch):
    """Wurzel des kicadxml-Exports; bei HW_NETLIST wird diese Datei gelesen (Gegenproben)."""
    if os.environ.get("HW_NETLIST"):
        return ET.parse(os.environ["HW_NETLIST"]).getroot()
    if sch in _netlist_cache: return _netlist_cache[sch]
    out = tempfile.mktemp(suffix=".xml")
    r = cli("sch", "export", "netlist", "--format", "kicadxml", "-o", out, sch)
    if r is None or r.returncode != 0 or not os.path.exists(out):
        return None
    root = ET.parse(out).getroot(); os.remove(out)
    _netlist_cache[sch] = root
    return root

def nets_of(root):
    d = {}
    for n in root.iter("net"):
        d[n.get("name").lstrip("/") if n.get("name").startswith("/") else n.get("name")] = {(x.get("ref"), x.get("pin"), x.get("pinfunction") or "") for x in n.findall("node")}
    return d

# ------------------------------------------------------------------------------------------ H1
def h1():
    if not os.path.exists(SCH): return crit("H1: Schaltplan fehlt: %s" % SCH)
    out = tempfile.mktemp(suffix=".json")
    r = cli("sch", "erc", "--severity-all", "--format", "json", "-o", out, SCH)
    if r is None: return warn("H1: kicad-cli nicht gefunden, ERC nicht pruefbar")
    if not os.path.exists(out): return crit("H1: ERC lieferte keinen Bericht: " + (r.stderr or "")[-200:])
    data = json.load(open(out)); os.remove(out)
    vs = [v for s in data["sheets"] for v in s["violations"]]
    cnt = collections.Counter((v["type"], v["severity"]) for v in vs)
    errs = sum(n for (t, s), n in cnt.items() if s == "error")
    base = json.load(open(os.path.join(ROOT, "tools", "checks", "hardware-erc.baseline")))
    bad = []
    by_type = collections.Counter(v["type"] for v in vs if v["severity"] != "error")
    for t, n in by_type.items():
        if t not in base: bad.append("neue Meldungsart '%s' (%d)" % (t, n))
        elif n > base[t]: bad.append("'%s': %d statt hoechstens %d" % (t, n, base[t]))
    if errs: crit("H1 ERC: %d Fehler" % errs)
    if bad: crit("H1 ERC ueber der Baseline: " + "; ".join(bad))
    if not errs and not bad:
        ok("H1 ERC: %d Meldungen betrachtet, 0 Fehler, Arten %s innerhalb der Baseline" % (len(vs), dict(by_type)))
    shrunk = [t for t in base if by_type.get(t, 0) < base[t]]
    if shrunk and not errs and not bad:
        info("H1 Baseline kann gesenkt werden fuer: " + ", ".join(shrunk))

# ------------------------------------------------------------------------------------------ H2
def h2():
    env = dict(os.environ, VERGLEICH_SCH=SCH)
    r = subprocess.run([sys.executable, os.path.join(HW, "vergleich.py")], capture_output=True, text=True, env=env)
    first = (r.stdout.splitlines() or ["(keine Ausgabe)"])[0]
    if r.returncode == 0: ok("H2 " + first)
    else:
        crit("H2 Netzliste weicht von der Soll-Netzliste ab (vergleich.py Exit %d)" % r.returncode)
        for l in (r.stdout or r.stderr).splitlines()[:12]: print("      " + l)

# ------------------------------------------------------------------------------------------ H3
def h3():
    rules = json.load(open(os.path.join(ROOT, "tools", "checks", "hardware-pinregeln.json")))
    root = netlist(SCH)
    if root is None: return crit("H3: Netzliste nicht erzeugbar (kicad-cli oder Schaltplan fehlt)")
    mod = rules["modul"]
    pins = {}   # Pinfunktion -> Menge Netznamen
    for name, nodes in nets_of(root).items():
        for ref, pin, pf in nodes:
            if ref == mod:
                # KiCad haengt die Pinnummer an: 'IO5_5', 'USB_D+_14', 'GND_40'
                base = pf[: -(len(pin) + 1)] if pf.endswith("_" + pin) else pf
                pins.setdefault(base, set()).add(name)
    # jede Regel muss sich auf einen echten Pin beziehen, sonst laeuft sie ins Leere
    named = set(rules["muss_offen_sein"]) | set(rules["nur_an_netzen"]) | set(rules["adc1_pins"])
    unknown = sorted(p for p in named if p not in pins)
    if unknown: crit("H3: Regel nennt Pins, die %s nicht hat: %s" % (mod, unknown))
    def conn(p): return {n for n in pins.get(p, set()) if not n.startswith("unconnected-")}
    errs = []
    for p in rules["muss_offen_sein"]:
        if conn(p): errs.append("%s darf nicht belegt sein (%s), ist an %s" % (p, rules["gruende"].get(p, "gesperrt"), sorted(conn(p))))
    for p, allowed in rules["nur_an_netzen"].items():
        extra = conn(p) - set(allowed)
        if extra: errs.append("%s darf nur an %s liegen, ist an %s" % (p, allowed, sorted(extra)))
    for p, nets in pins.items():
        for n in conn(p):
            if n in rules["netze_5v"]: errs.append("%s liegt am 5-V-Netz %s (S3-Pins sind nicht 5-V-tolerant)" % (p, n))
    adc_on = [p for p in pins if rules["adc_signal"] in conn(p)]
    if not adc_on: errs.append("Signal %s liegt an keinem Modulpin" % rules["adc_signal"])
    for p in adc_on:
        if p not in rules["adc1_pins"]: errs.append("%s (Signal %s) liegt nicht an einem ADC1-Pin" % (p, rules["adc_signal"]))
    for sig, p in rules.get("pinbelegung", {}).items():
        if sig not in conn(p): errs.append("Signal %s soll an %s liegen, ist an %s" % (sig, p, sorted(n for q in pins for n in conn(q) if n == sig and q != p) or "keinem Pin"))
    used = sum(1 for p in pins if conn(p))
    if errs:
        for e in errs: crit("H3 " + e)
    else:
        ok("H3 Pinregeln: %d Modulpins betrachtet (%d belegt), %d gesperrte offen, %d feste Pins am richtigen Netz, %d 5-V-Netze ausgeschlossen, LDR an %s"
           % (len(pins), used, len(rules["muss_offen_sein"]), len(rules["nur_an_netzen"]), len(rules["netze_5v"]), adc_on))

# ------------------------------------------------------------------------------------------ H4
def bom_parts():
    from sx import parse, sval
    root = parse(open(SCH, encoding="utf-8").read())
    parts = set()
    for el in root:
        if isinstance(el, list) and el and el[0] == "symbol":
            lib = None; ref = None; flags = {}
            for c in el[1:]:
                if isinstance(c, list) and c:
                    if c[0] == "lib_id": lib = sval(c[1])
                    elif c[0] == "property" and sval(c[1]) == "Reference": ref = sval(c[2])
                    elif c[0] in ("in_bom", "dnp"): flags[c[0]] = c[1]
            if lib and not lib.startswith("power:") and flags.get("in_bom", "yes") == "yes" and flags.get("dnp", "no") == "no":
                parts.add(ref)
    return parts

def h4():
    if not os.path.exists(BOM):
        return warn("H4 Stueckliste fehlt noch (%s) -- Pflicht vor jeder Bestellung" % os.path.relpath(BOM, ROOT))
    parts = bom_parts()
    t = open(BOM, encoding="utf-8-sig").read()
    rows = list(csv.DictReader(io.StringIO(t)))
    col = "LCSC Part #" if rows and "LCSC Part #" in rows[0] else "LCSC"
    refs = []; errs = []
    allmd = " ".join(open(p, encoding="utf-8").read() for p in glob.glob(os.path.join(KOMP, "*.md")))
    notdoc = []
    for r in rows:
        ds = [x.strip() for x in r["Designator"].split(",") if x.strip()]
        refs += ds
        lcsc = (r.get(col) or "").strip()
        if not re.fullmatch(r"C\d+", lcsc): errs.append("Zeile %s: keine gueltige LCSC-Nummer ('%s')" % (r["Designator"][:30], lcsc))
        elif lcsc not in allmd: notdoc.append(lcsc)
    dup = [k for k, n in collections.Counter(refs).items() if n > 1]
    if dup: errs.append("Designatoren doppelt: %s" % sorted(dup))
    if set(refs) - parts: errs.append("in der Stueckliste, nicht im Schaltplan (in_bom): %s" % sorted(set(refs) - parts))
    if parts - set(refs): errs.append("im Schaltplan (in_bom), nicht in der Stueckliste: %s" % sorted(parts - set(refs)))
    for e in errs: crit("H4 " + e)
    if not errs:
        ok("H4 Stueckliste: %d Zeilen, %d Designatoren == %d Bauteile des Schaltplans, alle mit LCSC-Nummer" % (len(rows), len(refs), len(parts)))
    if notdoc: warn("H4 LCSC-Nummern ohne Beleg in hardware/komponenten/: %s" % sorted(set(notdoc)))
    bestand = os.path.join(KOMP, "jlcpcb-bestand.md")
    if os.path.exists(bestand):
        m = re.search(r"Prüfdatum[^0-9]*(\d{4}-\d{2}-\d{2})", open(bestand, encoding="utf-8").read())
        if m:
            age = (datetime.date.today() - datetime.date.fromisoformat(m.group(1))).days
            if age > 14: warn("H4 JLCPCB-Pruefung ist %d Tage alt (%s), vor der Bestellung erneuern" % (age, m.group(1)))
        else: warn("H4 jlcpcb-bestand.md nennt kein Pruefdatum")
    else: warn("H4 hardware/komponenten/jlcpcb-bestand.md fehlt")

# ------------------------------------------------------------------------------------------ H5
def h5():
    files = [f for f in sorted(glob.glob(os.path.join(KOMP, "*.md"))) if os.path.basename(f) not in ("jlcpcb-bestand.md", "README.md")]
    if not files: return warn("H5 keine Komponentenblaetter vorhanden")
    need = [("Quelle", r"(?im)^#+.*quell"), ("Fertigung bei JLCPCB", r"(?im)^#+.*fertigung bei jlcpcb"), ("Folgerung", r"(?im)^#+.*folgerung")]
    seite = re.compile(r"S\.|Seite|Abschnitt|Kap\.|Tab\.|Abb\.|Fig\.|Table|Sec\.|p\.\s*\d")
    bad = 0; haken = 0; ohne = 0; ohne_zeilen = []
    wissen = ("layout-regeln.md", "jlcpcb-fertigung.md", "kicad-layout-werkzeug.md", "kicad-regeln-und-footprints.md", "schaltplan-gestaltung.md")   # Wissensblaetter, keine Bauteile: Pflichtabschnitte gelten nicht, der Seitenbeleg schon
    for f in files:
        t = open(f, encoding="utf-8").read(); n = os.path.basename(f)
        for name, rx in ([] if n in wissen else need):
            if not re.search(rx, t): crit("H5 %s: Pflichtabschnitt fehlt: %s" % (n, name)); bad += 1
        for line in t.splitlines():
            if "✔" in line and not line.startswith("✔ im Datenblatt gelesen"):    # Legendenzeile ausnehmen
                haken += 1
                if not seite.search(line): ohne += 1; ohne_zeilen.append("%s:%s" % (n, line.strip()[:70]))
    if ohne:
        warn("H5 %d von %d Zeilen mit Haken ohne Seiten- oder Tabellenangabe" % (ohne, haken))
        for z in ohne_zeilen[:5]: print("      " + z)
    if not bad: ok("H5 %d Komponentenblaetter, Pflichtabschnitte vorhanden, %d Haken geprueft (%d ohne Seitenangabe)" % (len(files), haken, ohne))

# ------------------------------------------------------------------------------------------ H6
def h6():
    stored = os.path.join(HW, "v2", "nets.json")
    if not os.path.exists(V2SCH): return info("H6 V2-Projekt nicht vorhanden, Referenz nicht pruefbar")
    if not os.path.exists(stored): return warn("H6 gespeicherter V2-Export fehlt: hardware/esp32s3/v2/nets.json")
    root = netlist(V2SCH) if not os.environ.get("HW_NETLIST") else None
    if root is None:
        out = tempfile.mktemp(suffix=".xml")
        r = cli("sch", "export", "netlist", "--format", "kicadxml", "-o", out, V2SCH)
        if r is None or not os.path.exists(out): return warn("H6 V2-Export nicht erzeugbar")
        root = ET.parse(out).getroot(); os.remove(out)
    now = {frozenset("%s.%s" % (a, b) for a, b, _ in v) for k, v in nets_of(root).items() if not k.startswith("unconnected-")}
    was = {frozenset("%s.%s" % (a[0], a[1]) for a in nodes) for name, nodes in json.load(open(stored)) if not name.startswith("unconnected-")}
    if now == was: ok("H6 V2-Referenz: %d Netze, Export stimmt mit dem V2-Schaltplan ueberein" % len(now))
    else: warn("H6 V2-Schaltplan hat sich seit dem gespeicherten Export geaendert (%d Netze nur im Export, %d nur im Schaltplan) -- export_v2.py neu laufen lassen und Aenderungen pruefen" % (len(was - now), len(now - was)))

# ------------------------------------------------------------------------------------------ H7
def pcb_geometry(path):
    from sx import parse, sval
    r = parse(open(path, encoding="utf-8").read())
    xs = []; ys = []; pos = {}
    for el in r:
        if not isinstance(el, list) or not el: continue
        if el[0] in ("gr_line", "gr_rect", "gr_arc", "gr_circle", "gr_poly"):
            ly = [c for c in el if isinstance(c, list) and c and c[0] == "layer"]
            if ly and sval(ly[0][1]) == "Edge.Cuts":
                for c in el:
                    if isinstance(c, list) and c and c[0] in ("start", "end", "mid", "center"): xs.append(float(c[1])); ys.append(float(c[2]))
                    if isinstance(c, list) and c and c[0] == "pts":
                        for p in c[1:]:
                            if isinstance(p, list) and p and p[0] == "xy": xs.append(float(p[1])); ys.append(float(p[2]))
        if el[0] == "footprint":
            ref = None; at = None
            for c in el:
                if isinstance(c, list) and c and c[0] == "at": at = (float(c[1]), float(c[2]))
                if isinstance(c, list) and c and c[0] == "property" and sval(c[1]) == "Reference": ref = sval(c[2])
            if ref: pos[ref] = at
    if not xs: return None
    x0, y0 = min(xs), min(ys)
    return dict(w=max(xs) - x0, h=max(ys) - y0, pos={k: (v[0] - x0, v[1] - y0) for k, v in pos.items()})

FEST = ["H1", "H2", "H8", "H9", "H10"]          # Loecher und Pads: streng
STECKER = ["USB1", "H4", "H5", "H6", "H7", "SW1", "SW2", "SW3"]   # Footprint-Ursprung darf abweichen
def h7():
    if not PCB: return info("H7 noch kein Layout (hardware/esp32s3/*.kicad_pcb) -- Format-Pruefung und DRC folgen mit dem Layout")
    if not os.path.exists(V2PCB): return warn("H7 V2-Layout nicht vorhanden, Format nicht pruefbar")
    a = pcb_geometry(V2PCB); b = pcb_geometry(PCB)
    if a is None or b is None: return crit("H7 Umriss (Edge.Cuts) nicht lesbar")
    errs = []; n = 0
    if abs(a["w"] - b["w"]) > 0.05 or abs(a["h"] - b["h"]) > 0.05:
        errs.append("Umriss %.2f x %.2f mm statt %.2f x %.2f mm" % (b["w"], b["h"], a["w"], a["h"]))
    n += 1
    for refs, tol in ((FEST, 0.05), (STECKER, 1.0)):
        for ref in refs:
            if ref not in a["pos"]: continue
            n += 1
            if ref not in b["pos"]: errs.append("%s fehlt im Layout" % ref); continue
            d = max(abs(a["pos"][ref][0] - b["pos"][ref][0]), abs(a["pos"][ref][1] - b["pos"][ref][1]))
            if d > tol: errs.append("%s verschoben um %.2f mm (erlaubt %.2f)" % (ref, d, tol))
    for e in errs: crit("H7 " + e)
    if not errs: ok("H7 Format 1:1: Umriss und %d Positionen gegen V2 verglichen, alle innerhalb der Toleranz" % (n - 1))
    # Das DRC liest die Regeln aus .kicad_pro und .kicad_dru NEBEN der Leiterplatte. Fehlen sie, nimmt
    # KiCad Standardregeln: am V2-Layout waren das 504 falsche Abstandsfehler und 35 Randabstaende
    # statt 0 (17.10.2026). Ein DRC ohne Projektregeln waere deshalb nicht schlechter, sondern falsch.
    base_pcb = PCB[:-len(".kicad_pcb")]
    fehlt = [e for e in (".kicad_pro", ".kicad_dru") if not os.path.exists(base_pcb + e)]
    if fehlt: return crit("H7 DRC nicht aussagekraeftig: neben dem Layout fehlen %s (Projektregeln)" % ", ".join(fehlt))
    out = tempfile.mktemp(suffix=".json")
    r = cli("pcb", "drc", "--severity-all", "--format", "json", "-o", out, PCB)
    if r is not None and os.path.exists(out):
        d = json.load(open(out)); os.remove(out)
        vs = d.get("violations", [])
        cnt = collections.Counter(v["type"] for v in vs)
        cnt["unconnected_items"] = len(d.get("unconnected_items", []))
        base = json.load(open(os.path.join(ROOT, "tools", "checks", "hardware-drc.baseline")))
        bad = ["'%s': %d statt hoechstens %d" % (t, n, base.get(t, 0)) for t, n in cnt.items() if n > base.get(t, 0)]
        if bad: crit("H7 DRC schlechter als V2: " + "; ".join(bad))
        else: ok("H7 DRC: %d Meldungen, keine Art ueber der V2-Baseline (V2 selbst hat sie, siehe hardware-drc.baseline)" % len(vs))

# ------------------------------------------------------------------------------------------ H8
# Vorgabe des Nutzers vom 17.10.2026: LED-Ansteuerung, Schalten der LED-Versorgung und LDR-Lesung
# bleiben pin- UND funktionskompatibel zu V2, weil dasselbe LED-Board weiterverwendet wird.
# Geprueft wird deshalb der GANZE Pfad zwischen Controller-Pin und Stecker H7, nicht nur der Stecker:
# Wert, Anschluss jedes Pins und die Mitglieder der Schnittstellennetze. Der Controller selbst
# (U2/U4) ist ausgenommen, denn dort aendert sich der Pin. Abweichungen brauchen einen Eintrag
# in hardware-led-schnittstelle.ausnahmen MIT Grund, und die Freigabe des Nutzers.
LED_REFS = ["U3", "R11", "R13", "R17", "R31", "JP2", "Q1", "Q2", "R19", "R20", "R21", "R23", "C10", "C6", "H4", "H5", "H7", "R22"]
LED_NETZE = ["TO_VCC_RGB_5V", "TO_SK6812_IN", "LDR", "WS2812_POWER", "WS2812_BUS", "Net-(U3-Y)", "Net-(U3-VCC)",
             "Net-(Q2-G)", "Net-(Q2-D)", "Net-(C10-Pad2)"]
def led_ausnahmen():
    f = os.path.join(ROOT, "tools", "checks", "hardware-led-schnittstelle.ausnahmen")
    d = {}
    if os.path.exists(f):
        for l in open(f, encoding="utf-8"):
            l = l.strip()
            if l and not l.startswith("#") and ":" in l:
                k, v = l.split(":", 1); d[k.strip()] = v.strip()
    return d

def parts_values(root):
    return {c.get("ref"): (c.findtext("value") or "") for c in root.iter("comp")}

def h8():
    v2xml = os.path.join(HW, "v2", "v2.xml")
    if not os.path.exists(v2xml): return warn("H8 gespeicherter V2-Export fehlt")
    r2 = ET.parse(v2xml).getroot(); r3 = netlist(SCH)
    if r3 is None: return crit("H8: Netzliste nicht erzeugbar")
    v2v, v3v = parts_values(r2), parts_values(r3)
    n2, n3 = nets_of(r2), nets_of(r3)
    def pinnets(nets, ref):
        d = {}
        for name, nodes in nets.items():
            for a, pin, _ in nodes:
                if a == ref: d[pin] = name
        return d
    ausn = led_ausnahmen(); errs = []; hoch = []; geprueft = 0
    for ref in LED_REFS:
        if ref not in v3v: errs.append("%s fehlt (V2: %s)" % (ref, v2v.get(ref))); continue
        geprueft += 1
        if v2v[ref] != v3v[ref] and ("wert:" + ref) not in ausn:
            errs.append("%s hat den Wert '%s' statt '%s' wie V2" % (ref, v3v[ref], v2v[ref]))
        a, b = pinnets(n2, ref), pinnets(n3, ref)
        for pin in sorted(set(a) | set(b)):
            if a.get(pin) != b.get(pin) and ("pin:" + ref) not in ausn:
                errs.append("%s Pin %s liegt an '%s' statt '%s' wie V2" % (ref, pin, b.get(pin), a.get(pin)))
    ctrl = {"U2", "U4"}
    for net in LED_NETZE:
        m2 = {a for a, _, _ in n2.get(net, set())} - ctrl
        m3 = {a for a, _, _ in n3.get(net, set())} - ctrl
        for x in sorted(m2 - m3):
            if ("fehlt:" + x) not in ausn: errs.append("Netz %s: %s fehlt gegenueber V2" % (net, x))
        for x in sorted(m3 - m2):
            if ("zusatz:" + x) not in ausn: hoch.append("Netz %s: zusaetzliches Teil %s (V2 hat es nicht) -- ohne Eintrag in hardware-led-schnittstelle.ausnahmen" % (net, x))
    for e in errs: crit("H8 " + e)
    for h in hoch: warn("H8 " + h)
    if not errs:
        ok("H8 LED-Board-Schnittstelle: %d Bauteile und %d Netze gegen V2 verglichen, Werte und Pinanschluesse gleich (%d Ausnahmen eingetragen)" % (geprueft, len(LED_NETZE), len(ausn)))

CHECKS = {"H1": h1, "H2": h2, "H3": h3, "H4": h4, "H5": h5, "H6": h6, "H7": h7, "H8": h8}
if __name__ == "__main__":
    sel = list(CHECKS)
    if "--nur" in sys.argv: sel = sys.argv[sys.argv.index("--nur") + 1].split(",")
    for k in sel:
        if k not in CHECKS: print("unbekannte Pruefung: " + k); sys.exit(2)
        try: CHECKS[k]()
        except Exception as e:      # ein Werkzeugfehler ist ein Fehler der Pruefung, nicht ein OK
            crit("%s: Werkzeugfehler: %s: %s" % (k, type(e).__name__, e))
    sys.exit(1 if CRIT else 0)
