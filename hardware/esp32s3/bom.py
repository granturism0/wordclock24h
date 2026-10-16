#!/usr/bin/env python3
"""Erzeugt die Stueckliste aus dem Schaltplan (Quelle der Wahrheit ist die .kicad_sch, nicht gen.py).

Aufnahme: nur Bauteile mit in_bom ja UND dnp nein (Strom- und Flag-Symbole 'power:' nie).
Gruppiert wird nach (Wert, Footprint, LCSC); die Zeilen sind nach Designatoren sortiert wie in V2.

Ausgabe (beide UTF-8 mit BOM, CRLF wie die V2-Datei):
  bom.csv         Designator,Footprint,Quantity,Value,LCSC Part #        (Bestellfassung, Format V2)
  bom-arbeit.csv  dieselben fuenf Spalten + Klasse, Pruefdatum            (Arbeitsfassung)
Klasse und Pruefdatum werden aus hardware/komponenten/ GELESEN (jlcpcb-bestand.md und Komponentenblaetter,
Tabellen mit Spalte 'LCSC' und 'Klasse'); fehlt eines, steht 'offen'. Ein fehlendes LCSC-Feld im Schaltplan
steht ebenfalls als 'offen' (und laesst Pruefung H4 scheitern). Die Zahl der Zeilen je Klasse kommt auf stdout.
"""
import os, re, sys, csv, glob, collections
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from sx import parse, sval
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
KOMP = os.path.join(ROOT, "hardware", "komponenten")
SCH = os.environ.get("BOM_SCH", os.path.join(HERE, "WordClock-ESP32S3-V3.kicad_sch"))
OUTDIR = os.environ.get("BOM_OUT", HERE)

def parts_of(sch):
    root = parse(open(sch, encoding="utf-8").read())
    out = []
    for el in root:
        if not (isinstance(el, list) and el and el[0] == "symbol"): continue
        lib = None; pr = {}; fl = {}
        for c in el[1:]:
            if isinstance(c, list) and c:
                if c[0] == "lib_id": lib = sval(c[1])
                elif c[0] == "property": pr[sval(c[1])] = sval(c[2])
                elif c[0] in ("in_bom", "dnp"): fl[c[0]] = c[1]
        if lib is None or lib.startswith("power:"): continue
        if fl.get("in_bom", "yes") != "yes" or fl.get("dnp", "no") != "no": continue
        out.append(dict(ref=pr["Reference"], value=pr.get("Value", ""), fp=pr.get("Footprint", ""), lcsc=(pr.get("LCSC") or "").strip()))
    return out

def short_fp(fp):
    fp = fp.split(":", 1)[-1]
    m = re.fullmatch(r"[CR]_(\d{4})_\d+Metric", fp)
    return m.group(1) if m else fp

def klasse(s):
    s = s.replace("*", "").strip().lower()
    if s.startswith("preferred"): return "Preferred"
    if s.startswith("extended") or s == "e": return "Extended"
    if s.startswith("basic") or s == "b": return "Basic"
    return None

def iso(s):
    m = re.search(r"(\d{4})-(\d{2})-(\d{2})", s)
    if m: return m.group(0)
    m = re.search(r"(\d{1,2})\.(\d{1,2})\.(\d{4})", s)
    return "%s-%02d-%02d" % (m.group(3), int(m.group(2)), int(m.group(1))) if m else None

def lookup():
    """LCSC-Nummer -> (Klasse, Pruefdatum, Quelle); die erste Tabellenzeile mit der Nummer in der LCSC-Spalte gilt."""
    res = {}
    files = [os.path.join(KOMP, "jlcpcb-bestand.md")] + sorted(f for f in glob.glob(os.path.join(KOMP, "*.md")) if not f.endswith("jlcpcb-bestand.md"))
    for f in files:
        if not os.path.exists(f): continue
        datum = None; cols = None
        for line in open(f, encoding="utf-8").read().splitlines():
            m = re.search(r"Pr(?:ü|ue)fdatum[^0-9]*(\d{4}-\d{2}-\d{2}|\d{1,2}\.\d{1,2}\.\d{4})", line)
            if m and not line.lstrip().startswith("|"): datum = iso(m.group(1))
            if not line.lstrip().startswith("|"): cols = None; continue
            cells = [c.strip() for c in line.strip().strip("|").split("|")]
            if cells and cells[0].lower().replace("*", "").startswith("lcsc") or any(c.lower().startswith("lcsc") for c in cells[:3]) and any(c.startswith("Klasse") for c in cells):
                cols = {"lcsc": next(i for i, c in enumerate(cells) if c.lower().startswith("lcsc")),
                        "klasse": next((i for i, c in enumerate(cells) if c.startswith("Klasse")), None),
                        "datum": next((i for i, c in enumerate(cells) if c.startswith("Prüfdatum")), None)}
                continue
            if not cols or cols["klasse"] is None or max(cols["lcsc"], cols["klasse"]) >= len(cells): continue
            k = klasse(cells[cols["klasse"]])
            d = iso(cells[cols["datum"]]) if cols["datum"] is not None and cols["datum"] < len(cells) else None
            for nr in re.findall(r"C\d+", cells[cols["lcsc"]]):
                if nr not in res: res[nr] = (k, d or datum, os.path.basename(f))
    return res

def main():
    parts = parts_of(SCH)
    look = lookup()
    groups = collections.OrderedDict()
    for p in parts:
        key = (p["value"], short_fp(p["fp"]), p["lcsc"] or "offen")
        groups.setdefault(key, []).append(p["ref"])
    rows = []
    for (val, fp, nr), refs in groups.items():
        refs = sorted(refs)
        k, d, _ = look.get(nr, (None, None, None))
        rows.append([", ".join(refs), fp, len(refs), val, nr, k or "offen", d or "offen"])
    rows.sort(key=lambda r: r[0])
    os.makedirs(OUTDIR, exist_ok=True)
    for name, cut, head in (("bom.csv", 5, ["Designator", "Footprint", "Quantity", "Value", "LCSC Part #"]),
                            ("bom-arbeit.csv", 7, ["Designator", "Footprint", "Quantity", "Value", "LCSC Part #", "Klasse", "Prüfdatum"])):
        with open(os.path.join(OUTDIR, name), "w", encoding="utf-8-sig", newline="") as fh:
            w = csv.writer(fh, lineterminator="\r\n"); w.writerow(head)
            for r in rows: w.writerow(r[:cut])
    cnt = collections.Counter(r[5] for r in rows)
    print("Stueckliste: %d Zeilen, %d Bauteile (in_bom ja, dnp nein), %d Bauteile im Schaltplan gelesen" % (len(rows), sum(r[2] for r in rows), len(parts)))
    print("Zeilen je Klasse: " + ", ".join("%s %d" % (k, cnt.get(k, 0)) for k in ("Basic", "Preferred", "Extended", "offen")))
    for r in rows:
        if "offen" in (r[4], r[5], r[6]): print("OFFEN: %s | %s | Klasse %s | Datum %s" % (r[0], r[4], r[5], r[6]))
    return 0
if __name__ == "__main__":
    sys.exit(main())
