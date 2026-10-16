#!/usr/bin/env python3
"""Probe U fuer C70 Variante (c) (Paket 2026-10-15, Nachtrag A.6, R2b.8; AKL.8 (iv), AKG.3-AKG.5, design.md 18.8).

Aufruf:  tools/probe-u-c.py U1b|U2|U3        gegen DEVICE_HOST aus tools/device.conf
         PROBE_U_OHNE_UPLOAD=1 ...          Gegenprobe (DIR-014): nichts hochladen -- muss FEHL melden
Voraussetzung: ESP mit immutable (R2b) und das gebaute Dokument auf dem Geraet (marken-pruefen --erwartet besteht).

  U1b  styles.css ersetzt, Dokument gleich: warmer Browser bleibt beim alten Inhalt aus dem Cache; die alte Marken-URL
       liefert direkt 200 mit neuem ETag und no-cache, Δapp_token_miss +1; nach dem Zuruecklegen wieder immutable.
  U2   Dokument mit neuer styles-Marke ZUERST, Datei danach (Upload-Fenster): erst no-cache mit altem Inhalt, dann
       immutable mit neuem; zum Schluss beide Originale, die alte URL kommt wieder aus dem Cache.
  U3   Versionswechsel als Rueckweg: Dokument mit anderer v -- jede versionierte URL 200 mit immutable, danach aus dem
       Cache; Original zurueck: die urspruenglichen URLs aus dem Cache.

SCHREIBT AM GERAET (app_file_upload: index.html und styles.css) und legt beim Verlassen IMMER die gebauten Originale
zurueck, danach marken-pruefen --erwartet. Exit 0 bestanden, 1 Fehlschlag, 2 Werkzeugfehler/Vorbedingung.
"""
import gzip, json, os, re, shutil, subprocess, sys, tempfile, urllib.request

R = subprocess.run(["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True).stdout.strip()
A = os.path.join(R, "ESP8266/ESP-uclock/data/app")
HOST = next((re.match(r'\s*DEVICE_HOST=["\']?([^"\'\s]+)', z).group(1) for z in open(os.path.join(R, "tools/device.conf"))
             if re.match(r"\s*DEVICE_HOST=", z)), None)
OHNE = bool(os.environ.get("PROBE_U_OHNE_UPLOAD"))
fehl = 0


def aus(t): print(t, flush=True)
def ok(c, t):
    global fehl
    aus(("  OK    " if c else "  FEHL  ") + t)
    if not c: fehl += 1
def werkzeug(t): aus("ABBRUCH: " + t); sys.exit(2)
def marke(p): b = open(p, "rb").read(); return "%d-%s" % (len(b), b[-8:].hex())


def dr():
    with urllib.request.urlopen(f"http://{HOST}/api/device_ready", timeout=15) as r: return json.load(r)


def kopf(url):
    req = urllib.request.Request(f"http://{HOST}{url}", headers={"Accept-Encoding": "gzip"})
    with urllib.request.urlopen(req, timeout=20) as r:
        r.read(); return r.status, (r.headers.get("ETag") or "").strip('"'), r.headers.get("Cache-Control") or ""


def lade_hoch(datei, asset):
    if OHNE: aus(f"    (Gegenprobe: {asset} NICHT hochgeladen)"); return
    req = urllib.request.Request(f"http://{HOST}/api/app_file_upload?filename={asset}&encoding=gzip", data=open(datei, "rb").read(),
                                 headers={"Content-Type": "application/octet-stream"}, method="POST")
    with urllib.request.urlopen(req, timeout=60) as r: t = r.read().decode()
    if '"ok":true' not in t: werkzeug(f"Upload {asset}: {t[:120]}")
    aus(f"    hochgeladen: {asset} ({os.path.getsize(datei)} Byte, Marke {marke(datei)})")


def gz(quelle, ziel):
    with open(quelle, "rb") as f: b = f.read()
    with open(ziel, "wb") as f: f.write(gzip.compress(b, 9, mtime=0))


def dokument(tmp, name, aendern):
    """Dokument aus einer Kopie von data/app mit demselben Werkzeug wie im Build (Ent-21 E2)."""
    k = os.path.join(tmp, name); shutil.copytree(A, k); aendern(k)
    p = subprocess.run([os.path.join(R, "tools/app-marken.py"), k, os.path.join(k, "kopie.html")], capture_output=True, text=True)
    if p.returncode: werkzeug(f"app-marken fuer {name}: {p.stdout.strip()}")
    gz(os.path.join(k, "kopie.html"), os.path.join(tmp, name + ".gz")); return os.path.join(tmp, name + ".gz")


def lauf(warm, haken_cmds, tmp):
    """Ladeprobe mit Haken; haken_cmds: {nr: python-ausdruck-als-funktion} -- ausgefuehrt ueber dieses Skript selbst."""
    hk = os.path.join(tmp, "haken.sh")
    with open(hk, "w") as f:
        f.write("#!/bin/sh\nexec python3 %s --haken \"$1\"\n" % os.path.abspath(__file__))
    os.chmod(hk, 0o755)
    env = dict(os.environ, LADEPROBE_WARM=str(warm), LADEPROBE_HAKEN=hk, LADEPROBE_DETAIL="1", PROBE_U_TMP=tmp)
    p = subprocess.run([os.path.join(R, "tools/ladeprobe.sh")], cwd=R, env=env, capture_output=True, text=True)
    if p.returncode == 2: aus(p.stdout); werkzeug("Ladeprobe Werkzeugfehler")
    ladungen, nr = {}, None
    for z in p.stdout.splitlines():
        m = re.match(r"^\S+  #(\d+) ", z)
        if m: nr = int(m.group(1)); ladungen[nr] = []; aus(z); continue
        m = re.match(r"^  D (\S+) (leitung|cache) (\S+) cc=(.*) etag=(\S*)$", z)
        if m and nr is not None:
            u, modus, st, cc, et = m.groups(); ladungen[nr].append(dict(url=u, pfad=u.split("?")[0], q=u.partition("?")[2], modus=modus, st=st, cc=cc, etag=et))
        elif z.startswith("    Haken") or z.startswith("    hochgeladen") or "Gegenprobe" in z or z.startswith("    direkt"): aus(z)
    return ladungen


def finde(ld, pfad):
    return [a for a in ld if a["pfad"] == pfad]


def versioniert(ld):
    return [a for a in ld if "h=" in a["q"]]


# ------------------------------------------------------------------------------------------------- Haken (Unterprozess)
if len(sys.argv) == 3 and sys.argv[1] == "--haken":
    tmp = os.environ["PROBE_U_TMP"]; plan = json.load(open(os.path.join(tmp, "plan.json")))
    for schritt in plan.get(sys.argv[2], []):
        art = schritt[0]
        if art == "upload": lade_hoch(schritt[1], schritt[2])
        elif art == "direkt":
            vor = dr(); st, et, cc = kopf(schritt[1]); nach = dr()
            json.dump(dict(st=st, et=et, cc=cc, miss=nach.get("app_token_miss", 0) - vor.get("app_token_miss", 0)),
                      open(os.path.join(tmp, schritt[2]), "w"))
            aus(f"    direkt {schritt[1]}: {st} ETag {et} Cache-Control {cc}")
    sys.exit(0)

# ---------------------------------------------------------------------------------------------------------- Hauptteil
if len(sys.argv) != 2 or sys.argv[1] not in ("U1b", "U2", "U3"):
    print(__doc__.split("\n\n")[1], file=sys.stderr); sys.exit(2)
ABLAUF = sys.argv[1]
if not HOST: werkzeug("DEVICE_HOST fehlt (tools/device.conf)")
DOC, STY = os.path.join(A, "index.html.gz"), os.path.join(A, "styles.css.gz")
for f in (DOC, STY):
    if not os.path.getsize(f): werkzeug(f"{f} leer -- erst make app-gz")
pr = subprocess.run([os.path.join(R, "tools/marken-pruefen.py"), HOST, "--erwartet"], capture_output=True, text=True)
if pr.returncode: aus(pr.stdout); werkzeug("Vorbedingung: das Geraet traegt nicht das gebaute Dokument mit gueltigen Marken (install-app.sh?)")
m_sty = marke(STY)
version = re.search(r'^const APP_VERSION = "([^"]*)";', open(os.path.join(A, "app.js"), encoding="utf-8").read(), re.M).group(1)
url_alt = f"/app/styles.css?v={version}&h={m_sty}"
tmp = tempfile.mkdtemp(prefix="probe-u-c.")
aus(f"=== Probe U {ABLAUF} gegen {HOST} -- styles.css Marke {m_sty}, APP_VERSION {version}{' -- GEGENPROBE' if OHNE else ''} ===")

# geaenderte styles.css: Zeilenumbruch angehaengt, gueltiges CSS, andere Marke
sty_neu = os.path.join(tmp, "styles.css.neu.gz")
with gzip.open(STY) as f: css = f.read()
with open(os.path.join(tmp, "styles.css.neu"), "wb") as f: f.write(css + b"\n")
gz(os.path.join(tmp, "styles.css.neu"), sty_neu); m_neu = marke(sty_neu)
assert m_neu != m_sty

plan = {}
try:
    if ABLAUF == "U1b":
        plan = {"2": [["upload", sty_neu, "app/styles.css"]], "3": [["direkt", url_alt, "d1.json"]],
                "4": [["upload", STY, "app/styles.css"]]}
        json.dump(plan, open(os.path.join(tmp, "plan.json"), "w"))
        L = lauf(4, plan, tmp)
        for n in (2, 3):
            s = finde(L.get(n, []), "/app/styles.css")
            ok(bool(s) and all(a["modus"] == "cache" for a in s), f"U1b warm #{n}: styles.css aus dem Cache (warmer Browser bleibt beim alten Inhalt)")
        d = json.load(open(os.path.join(tmp, "d1.json"))) if os.path.exists(os.path.join(tmp, "d1.json")) else {}
        ok(d.get("st") == 200 and d.get("et") == m_neu and "immutable" not in d.get("cc", "") and "no-cache" in d.get("cc", ""),
           f"U1b direkt mit alter Marke: 200, neues ETag, no-cache (ist {d.get('st')}, {d.get('et')}, {d.get('cc')!r})")
        ok(d.get("miss") == 1, f"U1b direkt: Δapp_token_miss +1 (ist {d.get('miss')})")
        st, et, cc = kopf(url_alt)
        ok(st == 200 and et == m_sty and "immutable" in cc, f"U1b nach dem Zuruecklegen: immutable, urspruengliches ETag (ist {st}, {et}, {cc!r})")
    elif ABLAUF == "U2":
        doc_neu = dokument(tmp, "doc-u2", lambda k: shutil.copy(sty_neu, os.path.join(k, "styles.css.gz")))
        url_neu = f"/app/styles.css?v={version}&h={m_neu}"
        plan = {"2": [["upload", doc_neu, "app/index.html"]], "3": [["upload", sty_neu, "app/styles.css"]],
                "5": [["upload", DOC, "app/index.html"], ["upload", STY, "app/styles.css"]]}
        json.dump(plan, open(os.path.join(tmp, "plan.json"), "w"))
        L = lauf(5, plan, tmp)
        d2 = finde(L.get(2, []), "/app/"); s2 = [a for a in L.get(2, []) if a["url"] == url_neu]
        ok(bool(d2) and d2[0]["modus"] == "leitung" and d2[0]["st"] == "200", "U2 #2: Dokument 200 (neues Dokument)")
        ok(bool(s2) and s2[0]["modus"] == "leitung" and s2[0]["st"] == "200" and s2[0]["etag"] == m_sty and "immutable" not in s2[0]["cc"],
           f"U2 #2: styles.css mit neuer Marke 200, ALTER Inhalt, kein immutable (Fenster) -- {s2[:1]}")
        d3 = finde(L.get(3, []), "/app/"); s3 = [a for a in L.get(3, []) if a["url"] == url_neu]
        ok(bool(d3) and d3[0]["st"] == "304", "U2 #3: Dokument 304")
        ok(bool(s3) and s3[0]["modus"] == "leitung" and s3[0]["st"] == "200" and s3[0]["etag"] == m_neu and "immutable" in s3[0]["cc"],
           f"U2 #3: styles.css 200, neues ETag, immutable -- {s3[:1]}")
        l4 = [a for a in versioniert(L.get(4, [])) if a["modus"] != "cache"]
        ok(bool(L.get(4)) and not l4, f"U2 #4: alles Versionierte aus dem Cache (auf der Leitung: {[a['pfad'] for a in l4]})")
        d5 = finde(L.get(5, []), "/app/"); s5 = [a for a in L.get(5, []) if a["url"] == url_alt]
        ok(bool(d5) and d5[0]["st"] == "200", "U2 #5: Dokument 200 (Original zurueck)")
        ok(bool(s5) and s5[0]["modus"] == "cache", f"U2 #5: styles.css mit urspruenglicher Marke aus dem Cache -- {s5[:1]}")
    elif ABLAUF == "U3":
        v_neu = version + ".9"
        doc_v = dokument(tmp, "doc-u3", lambda k: open(os.path.join(k, "app.js"), "w", encoding="utf-8").write(
            re.sub(r'^const APP_VERSION = "[^"]*";', f'const APP_VERSION = "{v_neu}";', open(os.path.join(A, "app.js"), encoding="utf-8").read(), count=1, flags=re.M)))
        plan = {"2": [["upload", doc_v, "app/index.html"]], "4": [["upload", DOC, "app/index.html"]]}
        json.dump(plan, open(os.path.join(tmp, "plan.json"), "w"))
        L = lauf(4, plan, tmp)
        v2 = versioniert(L.get(2, []))
        ok(bool(v2) and all(f"v={v_neu}" in a["q"] and a["modus"] == "leitung" and a["st"] == "200" and "immutable" in a["cc"] for a in v2),
           f"U3 #2: jede versionierte URL mit v={v_neu} 200 und immutable ({len(v2)} URLs)")
        v3 = versioniert(L.get(3, []))
        ok(bool(v3) and all(a["modus"] == "cache" for a in v3), f"U3 #3: alles Versionierte aus dem Cache ({len(v3)})")
        v4 = versioniert(L.get(4, []))
        ok(bool(v4) and all(f"v={version}&" in a["q"] + "&" and a["modus"] == "cache" for a in v4),
           f"U3 #4: urspruengliche URLs aus dem Cache ({len(v4)})")
finally:
    if not OHNE:
        for f, a in ((DOC, "app/index.html"), (STY, "app/styles.css")):
            try: lade_hoch(f, a)
            except Exception as e: aus(f"  WARNUNG: {a} NICHT zurueckgelegt ({e}) -- ./tools/install-app.sh")
    shutil.rmtree(tmp, ignore_errors=True)
pr = subprocess.run([os.path.join(R, "tools/marken-pruefen.py"), HOST, "--erwartet"], capture_output=True, text=True)
ok(pr.returncode == 0, "danach marken-pruefen --erwartet ohne Abweichung")
aus(f"=== Probe U {ABLAUF}: {fehl} Fehlschlag/-schlaege ===")
sys.exit(1 if fehl else 0)
