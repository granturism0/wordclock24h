#!/usr/bin/env python3
"""F3 (Paket 2026-10-13, AKB.1-AKB.3): Umfangspruefung des Backups.

Stellt jeden *_set-Endpunkt aus dem Dispatch von http.cpp gegen die Setter, die der
Backup-Import der PWA aufruft. Ein Endpunkt, der weder im Import noch in der
Ausnahmeliste steht, ist eine stille Luecke: Eine neue Einstellung ist bedienbar,
wird aber nie gesichert und nie wiederhergestellt (L35 d, L425).

Aufruf: backup-umfang.py [<http.cpp> <app.js> <ausnahmen>]   Exit 0 OK, 1 FEHL, 2 Werkzeugfehler
"""
import re, sys, pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
HTTP = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "ESP8266/ESP-uclock/http.cpp"
APP  = pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "ESP8266/ESP-uclock/data/app/app.js"
AUSN = pathlib.Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "tools/checks/backup-umfang.ausnahmen"

# Benannte Liste der Importfunktionen (design.md §15). Fehlt eine davon in app.js, ist das
# ein FEHL und kein stilles Weniger: Sonst schrumpft der betrachtete Umfang unbemerkt (L235).
IMPORTFUNKTIONEN = [
    "importNetworkSettings", "importNetworkTimeSettings", "importDisplaySettings",
    "importMaintenanceSettings", "importClimateSettings", "importWeatherLocationSettings",
    "importSensorCorrectionSettings", "importRtcCorrectionSetting", "importAnimationSettings",
    "importTftSettings", "importAmbilightSettings", "importDfplayerSettings",
    "importOverlaySettings", "importTimerSettings", "importIrCodes",
    "restoreBackupLayoutTable", "restoreBackupOverlayAssets", "restoreBackupAssets",
]

def lies(p):
    try:
        return p.read_text(encoding="utf-8", errors="replace")
    except OSError as e:
        print(f"FEHL Werkzeug: {p} nicht lesbar ({e})"); sys.exit(2)

http, app = lies(HTTP), lies(APP)
# Kommentare sind kein Aufruf (Review 0.4, N2): "// ... getXSetUrl()" in einer Importfunktion
# erzeugte sonst Abdeckung, die es nicht gibt.
app_code = re.sub(r"/\*.*?\*/", "", app, flags=re.S)
app_code = re.sub(r"(?m)^\s*//.*$", "", app_code)
app_code = re.sub(r"(?m)(?<![:\\\"'])//\s.*$", "", app_code)
# Tolerant gegen Leerzeichen und strncmp (Review 0.4, M2): ein Setter in anderer Schreibweise
# waere sonst unsichtbar, und die Stufe bliebe gruen.
DISPATCH = r'!\s*strn?cmp\s*\(\s*path\s*,\s*"/api/([a-z0-9_]+_set)"'
dispatch = sorted(set(re.findall(DISPATCH, http)))
roh_zahl = len(re.findall(DISPATCH, http))
MINDEST = 50   # Stand 13.10.2026: 59. Faellt die Zahl darunter, ist eher das Muster kaputt als der Dispatch geschrumpft.

getter = dict(re.findall(r'const (get\w+Url) = createConfiguredUrlGetter\("(\w+)"\)', app))
url_set = sorted({p for p in re.findall(r'^\s*\w+_url: "/api/([a-z0-9_]+_set)"', app, re.M)})
pfade  = dict(re.findall(r'^\s*(\w+_url): "/api/([a-z0-9_]+)"', app, re.M))

# Hilfsfunktionen, die eine Importfunktion aufruft, gehoeren zum Import (writeIrRestorePlan
# ruft ir_code_set). Gefolgt wird hoechstens TIEFE Ebenen und nie in die Funktionen der
# Sperrliste: loadData & Co. beruehren fast jeden Getter, und wer ihnen folgt, saehe jeden
# Endpunkt als "im Import" -- eine Pruefung, die alles deckt, deckt nichts (DIR-014).
TIEFE = 3
SPERRE = {"loadData", "loadSecondaryData", "refreshVisibleModuleData", "applySettingsBackup",
          "importSettingsBackup", "setActiveModule", "apiFetch", "apiFetchValue", "apiFetchQuery",
          "settleFetchJson", "fetchWithTimeout", "translate", "translateFormat", "announceStatus"}
koerper = {m.group(1): m.group(0) for m in
           re.finditer(r'^(?:async )?function (\w+)\(.*?^\}', app_code, re.M | re.S)}

fehl = []
im_import = {}
gefolgt = set()
for fn in IMPORTFUNKTIONEN:
    if fn not in koerper:
        fehl.append(f"Importfunktion {fn} fehlt in app.js -- die Pruefung sieht ihren Gegenstand nicht mehr"); continue
    ebene, gesehen = [fn], {fn}
    for tiefe in range(TIEFE + 1):
        naechste = []
        for f in ebene:
            text = koerper[f]
            for g in set(re.findall(r'\b(get\w+Url)\(\)', text)):
                pfad = pfade.get(getter.get(g) or "")
                if pfad and pfad.endswith("_set"):
                    im_import.setdefault(pfad, set()).add(fn if f == fn else f"{fn}>{f}")
            if tiefe < TIEFE:
                for h in set(re.findall(r'\b([a-zA-Z_]\w*)\(', text)):
                    if h in koerper and h not in gesehen and h not in SPERRE and not h.startswith("get"):
                        gesehen.add(h); naechste.append(h)
        ebene = naechste
    gefolgt |= gesehen - {fn}

# M1 (Review 0.4): Eine benannte Importfunktion, die im Ablauf nicht mehr aufgerufen wird,
# deckte ihre Setter nur auf dem Papier. Erreichbarkeit ab applySettingsBackup, nur ueber Namen.
EINSTIEG = "applySettingsBackup"
if EINSTIEG not in koerper:
    fehl.append(f"Einstieg {EINSTIEG} fehlt in app.js -- Erreichbarkeit nicht pruefbar")
else:
    erreicht, offen_ = {EINSTIEG}, [EINSTIEG]
    while offen_:
        f = offen_.pop()
        for h in set(re.findall(r'\b([a-zA-Z_]\w*)\b', koerper[f])):
            if h in koerper and h not in erreicht:
                erreicht.add(h); offen_.append(h)
    for fn in IMPORTFUNKTIONEN:
        if fn in koerper and fn not in erreicht:
            fehl.append(f"Importfunktion {fn} ist ab {EINSTIEG} nicht mehr erreichbar -- ihre Setter werden nie wiederhergestellt")

ausnahmen = {}
for nr, zeile in enumerate(lies(AUSN).splitlines(), 1):
    z = zeile.split("#", 1)[0].strip()
    if not z:
        continue
    teile = z.split(None, 1)
    if len(teile) < 2 or not teile[1].strip():
        fehl.append(f"Ausnahmeliste Zeile {nr}: '{teile[0]}' ohne Grund")
        continue
    if teile[0] not in dispatch:
        fehl.append(f"Ausnahmeliste Zeile {nr}: '{teile[0]}' gibt es im Dispatch nicht")
    if teile[0] in im_import:
        fehl.append(f"Ausnahmeliste Zeile {nr}: '{teile[0]}' wird vom Import aufgerufen -- Ausnahme ueberfluessig")
    if teile[0] in ausnahmen:
        fehl.append(f"Ausnahmeliste Zeile {nr}: '{teile[0]}' steht doppelt")
    ausnahmen[teile[0]] = teile[1].strip()

luecken = [e for e in dispatch if e not in im_import and e not in ausnahmen]
for e in luecken:
    fehl.append(f"/api/{e}: weder im Backup-Import noch in der Ausnahmeliste")

helfer_mit_setter = sorted({q.split(">",1)[1] for s in im_import.values() for q in s if ">" in q})
print(f"  Hilfsfunktionen mit Settern (gefolgt, Tiefe {TIEFE}): {', '.join(helfer_mit_setter) or '-'}")
print(f"  Dispatch: {len(dispatch)} *_set-Endpunkte (Rohtreffer {roh_zahl}); "
      f"im Import {len(im_import)}, Ausnahmen {len(ausnahmen)}, "
      f"Importfunktionen {len(IMPORTFUNKTIONEN)} benannt")
print(f"  Importfunktionen: {', '.join(IMPORTFUNKTIONEN)}")
print(f"  Gegenzaehlung: URL-Tabelle der PWA kennt {len(url_set)} *_set-Endpunkte")
if len(dispatch) < MINDEST:
    fehl.append(f"nur {len(dispatch)} *_set-Endpunkte im Dispatch (Untergrenze {MINDEST}) -- Muster kaputt?")
for e in sorted(set(url_set) - set(dispatch)):
    fehl.append(f"/api/{e}: in der URL-Tabelle der PWA, aber nicht im Dispatch von http.cpp")
for e in sorted(set(dispatch) - set(url_set)):
    fehl.append(f"/api/{e}: im Dispatch, aber nicht in der URL-Tabelle der PWA")
for e in sorted(set(im_import) - set(dispatch)):
    fehl.append(f"/api/{e}: vom Import aufgerufen, aber nicht im Dispatch")
if len(dispatch) != roh_zahl:
    fehl.append(f"Dispatch doppelt gezaehlt? {len(dispatch)} verschiedene gegen {roh_zahl} Treffer")
for f in fehl:
    print(f"  FEHL  {f}")
if not fehl:
    print("  OK  jeder *_set-Endpunkt ist im Backup-Import oder begruendet ausgenommen")
sys.exit(1 if fehl else 0)
