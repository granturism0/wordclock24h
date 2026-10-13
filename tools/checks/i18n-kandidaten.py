#!/usr/bin/env python3
"""B8 (Paket 2026-10-13, AKD.1): Kandidaten fuer sichtbaren Text ausserhalb der i18n-Tabellen.

Listet mit Datei:Zeile
  - in app.js: Zeichenkettenliterale ausserhalb von I18N_DE, die nach Satztext aussehen
    (mindestens zwei Woerter aus Buchstaben), ohne console.*-Zeilen, Selektoren, URLs, Schluessel;
  - in index.html: Textknoten und die Attribute title, aria-label, placeholder, alt an Elementen
    ohne passendes data-i18n*.
Jeder Kandidat wird eingeordnet: "deutsch" (Umlaut/sz oder deutsches Funktionswort), sonst
"andere". Ausnahmen mit Grund stehen in i18n-kandidaten.ausnahmen ("datei:muster  grund").

Aufruf: i18n-kandidaten.py [--liste] [<app.js> <index.html> <ausnahmen>]
Exit 0, wenn keine deutschen Kandidaten ohne Ausnahme bleiben, sonst 1; 2 bei Werkzeugfehler.
Solange B8 offen ist, zaehlt die Stufe nur (AKD.1); scharf wird sie mit AKD.2.
"""
import re, sys, pathlib, html.parser

ROOT = pathlib.Path(__file__).resolve().parents[2]
args = [a for a in sys.argv[1:] if not a.startswith("--")]
LISTE = "--liste" in sys.argv
APP  = pathlib.Path(args[0]) if len(args) > 0 else ROOT / "ESP8266/ESP-uclock/data/app/app.js"
HTML = pathlib.Path(args[1]) if len(args) > 1 else ROOT / "ESP8266/ESP-uclock/data/app/index.html"
AUSN = pathlib.Path(args[2]) if len(args) > 2 else ROOT / "tools/checks/i18n-kandidaten.ausnahmen"

DEUTSCH = re.compile(r"[äöüÄÖÜß]|\b(der|die|das|und|oder|nicht|ist|wird|werden|mit|fuer|für|auf|ein|eine|einen|kein|keine|bitte|wurde|vom|zum|zur|bei|nach|noch|schon|auch|nur|Datei|Geraet|Gerät|Uhr|gespeichert|Fehler|laden|Einstellung)\b")

def lies(p):
    try:
        return p.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError as e:
        print(f"FEHL Werkzeug: {p} nicht lesbar ({e})"); sys.exit(2)

app, htm = lies(APP), lies(HTML)
try:
    # Nur ganze Zeilen mit "#" am Anfang sind Kommentar -- ein Textausschnitt darf "#" enthalten.
    ausnahmen = [z.strip() for z in lies(AUSN) if not z.lstrip().startswith("#")] if AUSN.exists() else []
except SystemExit:
    ausnahmen = []
ausnahmen = [a for a in ausnahmen if a]

def ausgenommen(datei, text):
    for a in ausnahmen:
        teile = a.split(None, 1)
        if len(teile) == 2 and teile[0].startswith(datei + ":") and teile[0].split(":", 1)[1] in text:
            return True
    return False

kandidaten = []   # (datei, zeile, art, text)

# ------------------------------------------------------------------ app.js
start = next((i for i, z in enumerate(app) if z.startswith("const I18N_DE")), None)
ende = next((i for i in range(start or 0, len(app)) if app[i] == "};"), None) if start is not None else None
if start is None or ende is None:
    print("FEHL Werkzeug: I18N_DE in app.js nicht gefunden -- die Pruefung saehe alles als Kandidat"); sys.exit(2)
LIT = re.compile(r'"((?:[^"\\]|\\.){4,})"|\'((?:[^\'\\]|\\.){4,})\'|`((?:[^`\\]|\\.){4,})`')
im_block = False
for i, roh in enumerate(app):
    # Kommentare sind kein sichtbarer Text: Blockkommentare, ganze //-Zeilen, //-Reste am Zeilenende.
    z = roh
    if im_block:
        if "*/" in z: z = z.split("*/", 1)[1]; im_block = False
        else: continue
    z = re.sub(r"/\*.*?\*/", "", z)
    if "/*" in z: z = z.split("/*", 1)[0]; im_block = True
    if z.lstrip().startswith("//"): continue
    z = re.sub(r"(?<![:\\\"'])//\s.*$", "", z)
    if start <= i <= ende or re.search(r"console\.(log|warn|error|info|debug)", z):
        continue
    for m in LIT.finditer(z):
        t = next(g for g in m.groups() if g is not None)
        if re.search(r"^[.#\[]|/api/|https?:|\$\{|[{}<>=;]|^[a-z0-9_.-]+$|\.(js|css|json|gz|png|svg|txt|hex)\b", t):
            continue
        if len(re.findall(r"[A-Za-zÄÖÜäöüß]{2,}", t)) < 2:
            continue
        kandidaten.append(("app.js", i + 1, "deutsch" if DEUTSCH.search(t) else "andere", t[:90]))

# -------------------------------------------------------------- index.html
class P(html.parser.HTMLParser):
    def __init__(s):
        super().__init__(); s.stapel = []
    def handle_starttag(s, tag, attrs):
        a = dict(attrs)
        hat_i18n = any(k.startswith("data-i18n") for k in a)
        for attr in ("title", "aria-label", "placeholder", "alt"):
            v = a.get(attr)
            if v and not ("data-i18n-" + attr) in a and len(re.findall(r"[A-Za-zÄÖÜäöüß]{2,}", v)) >= 1:
                kandidaten.append(("index.html", s.getpos()[0], "deutsch" if DEUTSCH.search(v) else "andere", f'{attr}="{v[:80]}"'))
        if tag not in ("br", "img", "input", "meta", "link", "hr"):
            s.stapel.append((tag, hat_i18n))
    def handle_endtag(s, tag):
        while s.stapel:
            t, _ = s.stapel.pop()
            if t == tag: break
    def handle_data(s, d):
        t = d.strip()
        if not t or not s.stapel: return
        tag, hat_i18n = s.stapel[-1]
        if tag in ("script", "style", "title") or hat_i18n: return
        if len(re.findall(r"[A-Za-zÄÖÜäöüß]{2,}", t)) < 1: return
        kandidaten.append(("index.html", s.getpos()[0], "deutsch" if DEUTSCH.search(t) else "andere", t[:90]))
P().feed("\n".join(htm))

offen = [k for k in kandidaten if not ausgenommen(k[0], k[3])]
de = [k for k in offen if k[2] == "deutsch"]
an = [k for k in offen if k[2] == "andere"]
print(f"  i18n-Kandidaten: app.js {sum(1 for k in kandidaten if k[0]=='app.js')} "
      f"(I18N_DE Zeile {start+1}-{ende+1} ausgenommen), index.html {sum(1 for k in kandidaten if k[0]=='index.html')}; "
      f"ohne Ausnahme: {len(de)} deutsch, {len(an)} andere; Ausnahmen {len(ausnahmen)}")
if LISTE:
    for d, z, art, t in offen:
        print(f"  {art:8} {d}:{z}  {t}")
sys.exit(1 if de else 0)
