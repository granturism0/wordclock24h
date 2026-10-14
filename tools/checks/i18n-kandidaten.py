#!/usr/bin/env python3
"""B8 (Paket 2026-10-13, AKD.1): Kandidaten fuer sichtbaren Text ausserhalb der i18n-Tabellen.

Listet mit Datei:Zeile
  - in app.js: Zeichenkettenliterale ausserhalb von I18N_DE, die nach Satztext aussehen
    (mindestens zwei Woerter aus Buchstaben), ohne console.*-Zeilen, Selektoren, URLs, Schluessel;
  - in index.html: Textknoten und die Attribute title, aria-label, placeholder, alt an Elementen
    ohne passendes data-i18n*.
Jeder Kandidat wird eingeordnet: "deutsch" (Umlaut/sz oder deutsches Funktionswort), sonst
"andere". Ausnahmen mit Grund stehen in i18n-kandidaten.ausnahmen ("datei:muster  grund").

Aufruf: i18n-kandidaten.py [--liste] [<app.js> <index.html> <ausnahmen> [<en.json>]]
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
# Format: "datei:muster<2+ Leerzeichen oder Tab>grund". Das Muster darf Leerzeichen enthalten.
# Review P2.6 H1: Frueher endete das Muster am ERSTEN Leerraum -- aus "app.js:) weicht nach
# ..." wurde ")", und das nahm jedes Literal mit Klammer aus. Davor (14.10.2026) war ein
# leeres Muster durchgerutscht und nahm ganz app.js aus. Deshalb jetzt: Trennung an 2+
# Leerzeichen, Muster mindestens 4 Zeichen, und jede Ausnahme muss einen Kandidaten treffen.
AUSN_LISTE = []
for a in ausnahmen:
    t = re.split(r"\t+| {2,}", a, maxsplit=1)
    if len(t) < 2 or ":" not in t[0] or not t[1].strip():
        print(f"FEHL Werkzeug: Ausnahme ohne Grund oder ohne Datei (Muster und Grund durch 2+ Leerzeichen trennen): {a!r}"); sys.exit(2)
    datei, muster = t[0].split(":", 1)
    if len(muster.strip()) < 4:
        print(f"FEHL Werkzeug: Ausnahme mit zu kurzem Muster (mindestens 4 Zeichen): {a!r}"); sys.exit(2)
    AUSN_LISTE.append([datei, muster, a, 0])

def ausgenommen(datei, text):
    treffer = False
    for e in AUSN_LISTE:
        if e[0] == datei and e[1] in text:
            e[3] += 1; treffer = True
    return treffer

kandidaten = []   # (datei, zeile, art, text)

# ------------------------------------------------------------------ app.js
start = next((i for i, z in enumerate(app) if z.startswith("const I18N_DE")), None)
ende = next((i for i in range(start or 0, len(app)) if app[i] == "};"), None) if start is not None else None
if start is None or ende is None:
    print("FEHL Werkzeug: I18N_DE in app.js nicht gefunden -- die Pruefung saehe alles als Kandidat"); sys.exit(2)
# Wortschatz aus den Tabellen selbst (AKD.2, Nachschaerfung 14.10.2026): Ein Wort ab vier
# Buchstaben, das in den Werten von I18N_DE steht, in denen von en.json aber nicht, ist deutsch.
# Die Funktionswortliste allein sah "Minuten-Ecken", "WordClock Vorschau" und "Update
# Fortschritt" nicht -- und "Speichern" haette sie nie gesehen (design.md §14: nicht ueber
# Satzwoerter oder Umlaute). Woerter in beiden Tabellen ("Update", "Version") bleiben neutral.
import json as _json
EN = ROOT / "ESP8266/ESP-uclock/data/app/i18n/en.json" if len(args) < 4 else pathlib.Path(args[3])
WORT = re.compile(r"[A-Za-zÄÖÜäöüß]{3,}")
try:
    _en = _json.loads(EN.read_text(encoding="utf-8"))
except (OSError, ValueError) as e:
    print(f"FEHL Werkzeug: {EN} nicht lesbar ({e})"); sys.exit(2)
_de_werte = re.findall(r'^\s*"[^"]+"\s*:\s*"((?:[^"\\]|\\.)*)"', "\n".join(app[start:ende + 1]), re.M)
_en_woerter = {w.lower() for v in _en.values() if isinstance(v, str) for w in WORT.findall(v)}
DE_WORT = {w.lower() for v in _de_werte for w in WORT.findall(v)} - _en_woerter
if len(_de_werte) < 100 or len(DE_WORT) < 200:
    print(f"FEHL Werkzeug: Wortschatz zu klein ({len(_de_werte)} DE-Werte, {len(DE_WORT)} Woerter) -- die Pruefung saehe ihren Gegenstand nicht"); sys.exit(2)

def ist_deutsch(t):
    return bool(DEUTSCH.search(t)) or any(w.lower() in DE_WORT for w in WORT.findall(t))

# index.html zusaetzlich: Ein Wort, das im englischen Wortschatz gar nicht vorkommt (und nicht
# ganz in Grossbuchstaben steht), ist dort verdaechtig -- "Minuten-Ecken" steht in KEINER der
# beiden Tabellen und fiel deshalb durch. Sprachneutrale Namen (WordClock, OpenWeather) stehen
# mit Grund in der Ausnahmeliste.
def ist_deutsch_html(t):
    return ist_deutsch(t) or any(w.lower() not in _en_woerter and not w.isupper() for w in WORT.findall(t))

# Review P2.6 K1: Die zeilenweise Kommentarerkennung hielt das Regex-Literal /^\.?\/*/ in
# app.js:10411 fuer den Beginn eines Blockkommentars und uebersprang den Rest der Datei, rund
# 5'800 Zeilen -- mit echten deutschen Texten darin, und S18 meldete gruen. Jetzt ein kleiner
# Tokenizer, der Zeichenketten, Template-Literale (samt ${...}), Regex-Literale und Kommentare
# unterscheidet. Endet die Datei in einem offenen Konstrukt, ist das ein Werkzeugfehler, und
# die Zahl der gelesenen Zeilen muss die ganze Datei sein.
TECH = ROOT / "tools/checks/i18n-technisch.txt"
try:
    TECHNISCHE_WOERTER = {z.strip() for z in TECH.read_text(encoding="utf-8").splitlines() if z.strip() and not z.startswith("#")}
except OSError as e:
    print(f"FEHL Werkzeug: {TECH} nicht lesbar ({e})"); sys.exit(2)
QUELLE = "\n".join(app)
VOR_REGEX = set("(,=:[!&|?{};+-*%<>~^")
SCHLUESSELWOERTER_VOR_REGEX = ("return", "typeof", "case", "do", "else", "in", "of", "void", "yield", "await", "delete", "throw", "new")

SPALTE0 = set()   # Zeilen, an denen im Code-Zustand ein Bezeichner in Spalte 0 beginnt (Invariante M-B)

def literale(q, basis=1, oben=True):
    """Liefert (zeile, art, text, in_console) fuer jede Zeichenkette und jeden Template-Text.
    basis: Zeilennummer der ersten Zeile von q (fuer ${...}-Ausdruecke, rekursiv gelesen)."""
    out, i, n, zeile = [], 0, len(q), basis
    letztes = ""            # letztes bedeutsames Zeichen (fuer Regex-Erkennung)
    letztes_wort = ""
    tiefe_console = []      # Klammertiefen, an denen ein console.*( begann
    tiefe = 0
    while i < n:
        c = q[i]
        if c == "\n":
            zeile += 1; i += 1; continue
        if c in " \t\r":
            i += 1; continue
        if c == "/" and q.startswith("//", i):
            j = q.find("\n", i); i = n if j < 0 else j; continue
        if c == "/" and q.startswith("/*", i):
            j = q.find("*/", i + 2)
            if j < 0: raise ValueError(f"Blockkommentar ab Zeile {zeile} nicht geschlossen")
            zeile += q.count("\n", i, j); i = j + 2; continue
        if c == "/" and (letztes == "" or letztes in VOR_REGEX or letztes_wort in SCHLUESSELWOERTER_VOR_REGEX):
            # Regex-Literal bis zum ungeschuetzten "/" ausserhalb einer Zeichenklasse
            j, klasse = i + 1, False
            while j < n:
                d = q[j]
                if d == "\\": j += 2; continue
                if d == "\n": raise ValueError(f"Regex-Literal ab Zeile {zeile} nicht geschlossen")
                if d == "[": klasse = True
                elif d == "]": klasse = False
                elif d == "/" and not klasse: break
                j += 1
            i = j + 1
            while i < n and q[i].isalpha(): i += 1
            letztes, letztes_wort = ")", ""; continue
        if c in "\"'":
            j, start_z = i + 1, zeile
            while j < n and q[j] != c:
                if q[j] == "\\": j += 1
                elif q[j] == "\n": raise ValueError(f"Zeichenkette ab Zeile {zeile} nicht geschlossen")
                j += 1
            if j >= n: raise ValueError(f"Zeichenkette ab Zeile {zeile} nicht geschlossen")
            out.append((start_z, "str", q[i + 1:j], bool(tiefe_console)))
            i = j + 1; letztes, letztes_wort = ")", ""; continue
        if c == "`":
            # Template: Text ohne ${...}; verschachtelte Ausdruecke rekursiv ueberspringen
            j, start_z, teile, akt = i + 1, zeile, [], []
            while True:
                if j >= n: raise ValueError(f"Template ab Zeile {start_z} nicht geschlossen")
                d = q[j]
                if d == "\\": akt.append(q[j:j + 2]); j += 2; continue
                if d == "`": break
                if d == "\n": zeile += 1
                if d == "$" and q.startswith("${", j):
                    teile.append("".join(akt)); akt = []
                    # Review P2.6 M-C: Der Ausdruck wird nicht mehr nur uebersprungen, sondern
                    # rekursiv gelesen -- Zeichenketten darin ("${ok ? "Gespeichert" : …}") sind
                    # sonst unsichtbar. Das Ende sucht ein Klammerzaehler, der Literale ueberspringt.
                    k, t = j + 2, 1
                    while k < n and t:
                        if q[k] == "{": t += 1
                        elif q[k] == "}": t -= 1
                        elif q[k] in "\"'`":
                            e = q[k]; k += 1
                            while k < n and q[k] != e:
                                if q[k] == "\\": k += 1
                                k += 1
                        k += 1
                    if t: raise ValueError(f"${{...}} ab Zeile {zeile} nicht geschlossen")
                    innen = q[j + 2:k - 1]
                    unter, _ = literale(innen, zeile, oben=False)
                    out.extend(unter)
                    zeile += innen.count("\n")
                    j = k; continue
                akt.append(d); j += 1
            teile.append("".join(akt))
            out.append((start_z, "tpl", " … ".join(x for x in teile if x.strip()), bool(tiefe_console)))
            i = j + 1; letztes, letztes_wort = ")", ""; continue
        if c.isalpha() or c in "_$":
            j = i
            while j < n and (q[j].isalnum() or q[j] in "_$"): j += 1
            wort = q[i:j]
            if oben and (i == 0 or q[i - 1] == "\n"):
                SPALTE0.add(zeile)
            if wort == "console" and re.match(r"\.(log|warn|error|info|debug)\s*\(", q[j:j + 12]):
                k = q.index("(", j); tiefe_console.append(tiefe + 1)
                tiefe += 1; i = k + 1; letztes, letztes_wort = "(", ""; continue
            letztes, letztes_wort = "a", wort; i = j; continue
        if c in "([{":
            tiefe += 1
        elif c in ")]}":
            if tiefe_console and tiefe_console[-1] == tiefe: tiefe_console.pop()
            tiefe -= 1
        letztes, letztes_wort = c, ""; i += 1
    return out, zeile

try:
    LITERALE, GELESEN = literale(QUELLE)
except ValueError as e:
    print(f"FEHL Werkzeug: app.js nicht vollstaendig gelesen -- {e}"); sys.exit(2)
if GELESEN != len(app) and GELESEN != len(app) + 1:
    print(f"FEHL Werkzeug: app.js nur bis Zeile {GELESEN} von {len(app)} gelesen"); sys.exit(2)
# Review P2.6 M-B: "gelesen bis Zeile n von n" sagt nichts gegen ein verschlucktes Stueck --
# der Zeilenzaehler laeuft auch im Kommentar weiter. Echte Invariante: Jede Zeile, die in
# Spalte 0 mit function/async/const/let/var/class beginnt, muss der Tokenizer im CODE-Zustand
# erreichen. Liest er eine Regex als Division und oeffnet "\/*" einen Kommentar bis zum
# naechsten "*/", fehlen genau diese Zeilen.
SOLL0 = {k + 1 for k, z in enumerate(app) if re.match(r"(async |function |const |let |var |class )", z)}
verschluckt = sorted(SOLL0 - SPALTE0)
if verschluckt:
    print(f"FEHL Werkzeug: {len(verschluckt)} Deklarationen in Spalte 0 nicht im Code-Zustand erreicht, erste: Zeile {verschluckt[:5]} -- der Tokenizer hat dort Code als Kommentar, Zeichenkette oder Regex gelesen"); sys.exit(2)

class TextAusHtml(html.parser.HTMLParser):
    def __init__(s):
        super().__init__(); s.texte = []
    def handle_data(s, d):
        if d.strip(): s.texte.append(d.strip())
    def handle_starttag(s, tag, attrs):
        for k, v in attrs:
            if k in ("title", "aria-label", "placeholder", "alt") and v: s.texte.append(v)

TECHNISCH = re.compile(r"^[.#\[]|/api/|https?:|^[a-z0-9_.\-]+$|\.(js|css|json|gz|png|svg|txt|hex|bin)\b|^[a-z]+(-[a-z]+)+$")
literale_gesamt = 0
for zeile, art, roh, in_console in LITERALE:
    if start + 1 <= zeile <= ende + 1 or in_console:
        continue
    literale_gesamt += 1
    texte = [roh]
    if "<" in roh and ">" in roh:
        h = TextAusHtml(); h.feed(roh.replace(" … ", " ")); texte = h.texte
    for t in texte:
        t = t.strip()
        # Ein kleingeschriebenes Einzelwort ist meist ein Bezeichner ("idle", "manual") -- aber
        # nicht, wenn es im deutschen Wortschatz steht: "gefunden", "fehlt", "unbekannt" fielen
        # sonst genau hier durch (Review P2.6, app.js:10523, :14961).
        einzelwort = re.fullmatch(r"[a-zäöüß]{3,}(\.\.\.)?", t)
        # Review P2.6 M-D: dieselbe Regel fuer ein grossgeschriebenes Einzelwort ("Helligkeitskurve"),
        # das in keiner Tabelle steht -- es fiel bisher ganz durch.
        gross = re.fullmatch(r"[A-ZÄÖÜ][a-zäöüß]{2,}(\.\.\.)?", t)
        if gross and not ist_deutsch(t) and t.rstrip(".").lower() not in _en_woerter and t.rstrip(".") not in TECHNISCHE_WOERTER:
            kandidaten.append(("app.js", zeile, "deutsch", t + "   [unklassifiziert: in die Tabellen oder in i18n-technisch.txt]"))
            continue
        if einzelwort and not ist_deutsch(t) and t.rstrip(".") not in _en_woerter and t.rstrip(".") not in TECHNISCHE_WOERTER:
            # Weder deutsch noch englisch noch als technisch gefuehrt: "unbekannt" und
            # "angefragt" standen genau so da. Einordnen lassen statt raten.
            kandidaten.append(("app.js", zeile, "deutsch", t + "   [unklassifiziert: in die Tabellen oder in i18n-technisch.txt]"))
            continue
        # Review P2.6 N-1: Ein Satz (mit Leerzeichen) wird nie als technisch verworfen -- "Datei
        # app.js.gz fehlt" enthielt eine Dateiendung und fiel sonst ganz durch.
        if len(t) < 2 or (" " not in t and TECHNISCH.search(t) and not (einzelwort and ist_deutsch(t))):
            continue
        # Buchstabenraster der Wortuhr (Layoutdaten, keine Sprache): nur Grossbuchstaben, ohne
        # Leerzeichen. Benannte Regel statt Ausnahme je Zeile.
        # Auch die Zahlwoerter der Wortuhr-Logik ("VIER") und Rasterzeilen mit Fuellzeichen "#".
        if re.fullmatch(r"[A-ZÄÖÜ#]{3,}", t):
            continue
        woerter = len(re.findall(r"[A-Za-zÄÖÜäöüß]{2,}", t))
        if woerter < 1 or (woerter < 2 and not ist_deutsch(t)):
            continue
        kandidaten.append(("app.js", zeile, "deutsch" if ist_deutsch(t) else "andere", t[:90]))

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
                kandidaten.append(("index.html", s.getpos()[0], "deutsch" if ist_deutsch_html(v) else "andere", f'{attr}="{v[:80]}"'))
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
        kandidaten.append(("index.html", s.getpos()[0], "deutsch" if ist_deutsch_html(t) else "andere", t[:90]))
P().feed("\n".join(htm))

offen = [k for k in kandidaten if not ausgenommen(k[0], k[3])]
# Review P2.6 N-3: i18n-technisch.txt mit derselben Strenge wie die Ausnahmen -- ein Eintrag,
# der nirgends als Literal vorkommt, ist veraltet; ein deutsches Wort darf dort nicht stehen,
# sonst liesse sich jeder deutsche Einzelwort-Text stumm schalten.
_alle_literale = {t.rstrip(".") for _, _, t, _ in LITERALE}
for w in sorted(TECHNISCHE_WOERTER):
    if w not in _alle_literale:
        offen.append(("i18n-technisch.txt", 0, "deutsch", f"{w}   [veraltet: kommt in app.js nicht als Literal vor]"))
    elif w.lower() in DE_WORT:
        offen.append(("i18n-technisch.txt", 0, "deutsch", f"{w}   [deutsches Wort -- gehoert in die Tabellen, nicht in die technische Liste]"))
unbenutzt = [e[2] for e in AUSN_LISTE if e[3] == 0]
de = [k for k in offen if k[2] == "deutsch"]
an = [k for k in offen if k[2] == "andere"]
print(f"  i18n-Kandidaten: app.js {sum(1 for k in kandidaten if k[0]=='app.js')} "
      f"(I18N_DE Zeile {start+1}-{ende+1} ausgenommen), index.html {sum(1 for k in kandidaten if k[0]=='index.html')}; "
      f"ohne Ausnahme: {len(de)} deutsch, {len(an)} andere; Ausnahmen {len(ausnahmen)}; "
      f"Wortschatz {len(DE_WORT)} nur-deutsche Woerter aus {len(_de_werte)} Tabellenwerten; "
      f"app.js gelesen bis Zeile {GELESEN} von {len(app)}, {literale_gesamt} Literale ausserhalb der Tabelle")
for a in unbenutzt:
    print(f"  FEHL Ausnahme trifft keinen Kandidaten (veraltet oder Muster falsch): {a}")
if LISTE:
    for d, z, art, t in offen:
        print(f"  {art:8} {d}:{z}  {t}")
sys.exit(1 if de or unbenutzt else 0)
