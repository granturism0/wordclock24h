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
dispatch = sorted(set(re.findall(r'strcmp \(path, "/api/([a-z0-9_]+_set)"\)', http)))
roh_zahl = len(re.findall(r'strcmp \(path, "/api/[a-z0-9_]+_set"\)', http))

getter = dict(re.findall(r'const (get\w+Url) = createConfiguredUrlGetter\("(\w+)"\)', app))
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
           re.finditer(r'^(?:async )?function (\w+)\(.*?^\}', app, re.M | re.S)}

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
    ausnahmen[teile[0]] = teile[1].strip()

luecken = [e for e in dispatch if e not in im_import and e not in ausnahmen]
for e in luecken:
    fehl.append(f"/api/{e}: weder im Backup-Import noch in der Ausnahmeliste")

helfer_mit_setter = sorted({q.split(">",1)[1] for s in im_import.values() for q in s if ">" in q})
print(f"  Hilfsfunktionen mit Settern (gefolgt, Tiefe {TIEFE}): {', '.join(helfer_mit_setter) or '-'}")
print(f"  Dispatch: {len(dispatch)} *_set-Endpunkte (Rohtreffer {roh_zahl}); "
      f"im Import {len(im_import)}, Ausnahmen {len(ausnahmen)}, "
      f"Importfunktionen {len(IMPORTFUNKTIONEN)} benannt")
if len(dispatch) != roh_zahl:
    fehl.append(f"Dispatch doppelt gezaehlt? {len(dispatch)} verschiedene gegen {roh_zahl} Treffer")
for f in fehl:
    print(f"  FEHL  {f}")
if not fehl:
    print("  OK  jeder *_set-Endpunkt ist im Backup-Import oder begruendet ausgenommen")
sys.exit(1 if fehl else 0)
