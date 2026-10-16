#!/usr/bin/env python3
"""Markenpruefung am Geraet (Paket 2026-10-15, Nachtrag A.6, R2b.8; AKL.8 (i)).

Aufruf:  tools/marken-pruefen.py <host> [--dokument <datei>] [--erwartet]
  liest das ausgelieferte Dokument /app/ (oder mit --dokument eine lokale Datei -- fuer Gegenproben), sammelt jeden
  versionierten Verweis (?v=...&h=...) und jeden Eintrag aus <meta name="wc-asset-tokens">, fragt jede dieser URLs am
  Geraet ab und verlangt: Antwort 200, Cache-Control mit immutable, ETag gleich "<marke>".
  --erwartet: ein Dokument OHNE Marken ist ein Fehlschlag (ab einem Stand, dessen gebautes index.html.gz Marken traegt).
Exit 0 alles passt, 1 mindestens eine Abweichung, 2 Werkzeugfehler. Jede Abweichung wird beim Namen genannt:
  "Marke passt nicht", "kein immutable", "Meta-Eintrag fehlt", "nicht erreichbar".
Liest nur.
"""
import gzip, html, re, sys, urllib.request

MARKE = re.compile(r"^[0-9]+-[0-9a-f]{16}$")


def hole(url, kopf=False):
    req = urllib.request.Request(url, headers={"Accept-Encoding": "gzip"})
    with urllib.request.urlopen(req, timeout=20) as r:
        b = r.read()
        if not kopf and r.headers.get("Content-Encoding") == "gzip":
            b = gzip.decompress(b)
        return r.status, r.headers, b


def main():
    a = sys.argv[1:]
    if not a:
        print(__doc__.split("\n\n")[1], file=sys.stderr); sys.exit(2)
    host = a[0]; dok = None; erwartet = "--erwartet" in a
    if "--dokument" in a:
        dok = open(a[a.index("--dokument") + 1], encoding="utf-8").read()
    try:
        if dok is None:
            dok = hole(f"http://{host}/app/")[2].decode("utf-8", "replace")
    except Exception as e:
        print(f"  FEHL  Dokument /app/ nicht erreichbar: {e}"); sys.exit(1)

    verweise = [html.unescape(x) for x in re.findall(r'(?:href|src)="(/app/[^"?]+\?[^"]*)"', dok)]
    meta = re.findall(r'<meta name="wc-asset-tokens" content="([^"]*)"', dok)
    eintraege = []
    if meta and meta[0]:
        for e in meta[0].split(";"):
            p, _, m = e.partition("=")
            eintraege.append((p, m))
    ziele = []   # (name, url, marke)
    for v in verweise:
        q = dict(x.split("=", 1) for x in v.split("?", 1)[1].split("&") if "=" in x)
        ziele.append((v.split("?")[0], v, q.get("h", "")))
    fehl = 0
    if not ziele and not eintraege:
        if erwartet:
            print("  FEHL  Dokument ohne Marken -- das ausgelieferte index.html ist nicht das gebaute (Meta-Eintrag fehlt)"); sys.exit(1)
        print("  --    Dokument ohne Marken (Stand vor R2b oder ungebaute Quelle) -- Verhalten (a), nichts zu pruefen"); sys.exit(0)
    if not meta:
        print("  FEHL  <meta name=\"wc-asset-tokens\"> fehlt im Dokument (Meta-Eintrag fehlt)"); fehl += 1
    elif not eintraege:
        print("  FEHL  Meta-Element leer (Meta-Eintrag fehlt)"); fehl += 1
    v = re.findall(r"[?&]v=([^&]+)", " ".join(x[1] for x in ziele))
    version = v[0] if v else ""
    for p, m in eintraege:
        ziele.append(("/app/" + p, f"/app/{p}?v={version}&h={m}", m))
    for name, url, m in ziele:
        if not MARKE.match(m or ""):
            print(f"  FEHL  {name}: Marke {m!r} hat nicht die Form ^[0-9]+-[0-9a-f]{{16}}$ (Marke passt nicht)"); fehl += 1; continue
        try:
            st, h, _ = hole(f"http://{host}{url}", kopf=True)
        except Exception as e:
            print(f"  FEHL  {name}: nicht erreichbar ({e})"); fehl += 1; continue
        et = (h.get("ETag") or "").strip('"'); cc = h.get("Cache-Control") or ""
        if st != 200:
            print(f"  FEHL  {name}: HTTP {st}"); fehl += 1
        elif et != m:
            print(f"  FEHL  {name}: Marke passt nicht (Marke {m}, ETag am Geraet {et or '-'})"); fehl += 1
        elif "immutable" not in cc:
            print(f"  FEHL  {name}: kein immutable (Cache-Control: {cc or '-'})"); fehl += 1
        else:
            print(f"  OK    {name}: immutable, ETag = Marke {m}")
    print(f"  Markenpruefung: {len(ziele)} Ziele ({len(verweise)} Verweise, {len(eintraege)} Meta-Eintraege), {fehl} Abweichung(en)")
    sys.exit(1 if fehl else 0)


main()
