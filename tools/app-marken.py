#!/usr/bin/env python3
"""Inhaltsmarken fuer die App-Assets (Paket 2026-10-15, Nachtrag A.6, R2b.3; AKL.4, Ent-19 C, Ent-20 B, Ent-21 E2).

Aufruf:  tools/app-marken.py <data/app-Verzeichnis> <ausgabe-kopie-von-index.html>
         Exit 0 Kopie geschrieben, 1 Abbruchgrund (Meldung auf stdout UND stderr), 2 Aufruffehler.

Was es tut:
  - liest APP_VERSION aus `const APP_VERSION = "...";` in app.js (dieselbe Zeile wie das Make-Ziel app-version-file);
  - rechnet fuer jedes versionierte Asset die Inhaltsmarke aus seiner .gz nach der Formel von http_app_etag ()
    (Variante T): "<groesse der .gz>-<die letzten 8 Byte als 16 Hexziffern, Kleinbuchstaben>" -- ohne Anfuehrungszeichen;
  - schreibt eine KOPIE von index.html, in der genau die sechs bekannten Verweise ?v=<APP_VERSION>&h=<marke> tragen,
    und fuellt das Meta-Element wc-asset-tokens mit den Marken aller Sprachdateien unter i18n/ -- und nur dieser.

Die Quelle index.html aendert es nie. layout-previews.json bekommt nie eine Marke (Ent-20 B), index.html und sw.js
ebenfalls nicht. Eine Marke, die nicht zur ausgelieferten Datei passt, liesse einen Browser ein Jahr lang eine falsche
Datei behalten (Regel I-L, design.md 18.3) -- deshalb bricht das Werkzeug bei jeder Unstimmigkeit ab, statt
weiterzumachen.
"""
import os, re, sys

VERWEISE = ["manifest.webmanifest", "icons/icon-192.svg", "icons/icon-192.png", "icons/icon-180.png",
            "styles.css", "app.js"]
NIE = {"index.html", "sw.js", "layout-previews.json"}
MARKE = re.compile(r"^[0-9]+-[0-9a-f]{16}$")
VERSION = re.compile(r"^[0-9]+(\.[0-9]+)*$")


def abbruch(text):
    msg = "app-marken: ABBRUCH: " + text
    print(msg, flush=True)
    print(msg, file=sys.stderr, flush=True)
    sys.exit(1)


def marke(gz):
    if not os.path.isfile(gz):
        abbruch(f"{gz} fehlt")
    b = open(gz, "rb").read()
    if len(b) == 0:
        abbruch(f"{gz} hat 0 Byte")
    if len(b) < 18 or b[:2] != b"\x1f\x8b":
        abbruch(f"{gz} ist keine gzip-Datei")
    m = "%d-%s" % (len(b), b[-8:].hex())
    if not MARKE.match(m):
        abbruch(f"Marke {m} fuer {gz} hat nicht die Form ^[0-9]+-[0-9a-f]{{16}}$")
    return m


def main():
    if len(sys.argv) != 3:
        print(__doc__.split("\n\n")[1], file=sys.stderr)
        sys.exit(2)
    app, aus = sys.argv[1], sys.argv[2]
    quelle = os.path.join(app, "index.html")
    if os.path.abspath(aus) == os.path.abspath(quelle):
        abbruch("die Ausgabe waere die Quelle index.html selbst")

    js = open(os.path.join(app, "app.js"), encoding="utf-8").read()
    v = re.findall(r'^const APP_VERSION = "([^"]*)";', js, re.M)
    if len(v) != 1:
        abbruch(f"APP_VERSION in app.js {len(v)}-mal statt genau einmal")
    version = v[0]
    if not VERSION.match(version):
        abbruch(f"APP_VERSION {version!r} hat nicht die Form ^[0-9]+(\\.[0-9]+)*$")

    html = open(quelle, encoding="utf-8", newline="").read()     # Zeilenenden der Quelle erhalten

    # Die sechs Verweise: jeder genau einmal als href="/app/<pfad>" oder src="/app/<pfad>", ohne Query.
    n = 0
    for pfad in VERWEISE:
        mit_query = re.findall(r'(?:href|src)="/app/' + re.escape(pfad) + r'\?', html)
        if mit_query:
            abbruch(f"Verweis auf /app/{pfad} traegt schon eine Query")
        muster = re.compile(r'((?:href|src)=")/app/' + re.escape(pfad) + r'"')
        treffer = muster.findall(html)
        if len(treffer) != 1:
            abbruch(f"Verweis auf /app/{pfad} {len(treffer)}-mal statt genau einmal")
        m = marke(os.path.join(app, pfad + ".gz"))
        html = muster.sub(lambda t: f'{t.group(1)}/app/{pfad}?v={version}&amp;h={m}"', html)
        n += 1
    if n != 6:
        abbruch(f"{n} Verweise umgeschrieben statt 6")
    # Kein weiterer /app/-Verweis darf eine Query tragen, den das Werkzeug nicht kennt (er bliebe ohne Marke -- das ist
    # erlaubt und faellt auf Verhalten (a) zurueck --, aber eine fremde Query waere ein Fehler im Markup).
    for x in re.findall(r'(?:href|src)="/app/([^"?]+)\?', html):
        if x not in VERWEISE:
            abbruch(f"unbekannter Verweis /app/{x} mit Query")
    for x in NIE:
        if re.search(r'(?:href|src)="/app/' + re.escape(x) + r'\?', html):
            abbruch(f"{x} bekaeme eine Marke")

    # Meta-Element: in der Quelle genau einmal und leer; in der Kopie mit allen Sprachdateien.
    meta = re.compile(r'(<meta name="wc-asset-tokens" content=")(")')
    if len(meta.findall(html)) != 1:
        abbruch('<meta name="wc-asset-tokens" content=""> nicht genau einmal (leer) in index.html')
    i18n = os.path.join(app, "i18n")
    sprachen = sorted(f for f in os.listdir(i18n) if f.endswith(".json")) if os.path.isdir(i18n) else []
    if not sprachen:
        abbruch("keine Sprachdatei unter i18n/")
    eintraege = []
    for f in sprachen:
        pfad = "i18n/" + f
        if pfad in NIE or f in NIE:
            abbruch(f"{pfad} bekaeme eine Marke")
        eintraege.append(f"{pfad}={marke(os.path.join(app, pfad + '.gz'))}")
    inhalt = ";".join(eintraege)
    if "layout-previews.json" in inhalt:
        abbruch("layout-previews.json bekaeme eine Marke")
    html = meta.sub(lambda t: t.group(1) + inhalt + t.group(2), html)
    if html.count(inhalt) != 1 or len(eintraege) != len(sprachen):
        abbruch("Meta-Element nicht vollstaendig gefuellt")

    # AKL.5: jede erzeugte Query nur aus Paaren name=wert, Zeichen [0-9a-z.=&-], hoechstens 64 Zeichen. Ein Parameter
    # ohne "=" hat den ESP frueher abgestuerzt; Review R2b.9 SOLL-1 -- die Form wird geprueft, nicht nur angenommen.
    import html as _html
    qs = [_html.unescape(q) for q in re.findall(r'(?:href|src)="/app/[^"?]+\?([^"]*)"', html)]
    for q in qs:
        if len(q) > 64 or not re.match(r"^[a-z]+=[0-9a-z.-]+(&[a-z]+=[0-9a-z.-]+)*$", q):
            abbruch(f"erzeugte Query {q!r} verletzt AKL.5 (nur name=wert, [0-9a-z.=&-], hoechstens 64 Zeichen)")
    if len(qs) != 6:
        abbruch(f"{len(qs)} Queries im Dokument statt 6")

    tmp = aus + ".tmp"
    with open(tmp, "w", encoding="utf-8", newline="") as f:
        f.write(html)
    os.replace(tmp, aus)
    print(f"app-marken: {n} Verweise mit ?v={version}&h=..., {len(eintraege)} Sprachdatei(en) im Meta-Element "
          f"({inhalt}) -> {aus}", flush=True)
    sys.exit(0)


main()
