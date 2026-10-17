#!/usr/bin/env bash
# Gegenproben fuer tools/checks/hardware.py (Guardrail S19), DIR-014.
#
# WARUM DIESE DATEI EXISTIERT
#
# Eine neu gebaute Pruefung ist erst fertig, wenn sie einmal fehlgeschlagen ist -- und ihre
# Meldung ankommt. hardware.py meldet OK, wenn alles stimmt; ob sie auch ANSCHLAEGT, zeigt nur
# eine hergestellte Verletzung. Jede Probe hier baut eine solche Verletzung in einer KOPIE
# (nie im echten Entwurf, nie im V2-Projekt) und verlangt Exit 1 und die richtige Meldung.
# Zusaetzlich prueft sie die Gegenrichtung, wo es sie gibt: Eine unveraenderte Kopie muss
# bestehen. Eine Pruefung, die nur zeigt, dass sie anschlaegt, koennte immer anschlagen.
#
# Die Netzlisten-Proben (P3 bis P5) aendern die exportierte XML statt des Schaltplans: Sie
# pruefen die Regeln. Den Weg Schaltplan -> Netzliste prueft P2 (H2) mit.
cd "$(git rev-parse --show-toplevel)" || exit 2
CLI=/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli
[ -x "$CLI" ] || { echo "  kicad-cli nicht gefunden -- Hardware-Gegenproben nicht ausfuehrbar"; exit 0; }
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
SCH=hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch
V2="$HOME/Documents/WordClock-USB C - STM32F411 - V2"
V2PCB="$V2/WordClock-USB C - STM32F411 - V2.kicad_pcb"
ok=0; schlecht=0

# probe "Beschreibung" erwarteter_exit "Muster in der Ausgabe" befehl...
probe() {
  local name=$1 want=$2 pat=$3; shift 3
  local out; out=$("$@" 2>&1); local code=$?
  local ist=ok; [ $code -ne 0 ] && ist=fail
  local wantw=ok; [ "$want" = 1 ] && wantw=fail
  if [ "$ist" = "$wantw" ] && printf '%s' "$out" | grep -q -- "$pat"; then
    printf '  OK    %-52s Exit %d, Meldung "%s"\n' "$name" "$code" "$pat"; ok=$((ok+1))
  else
    printf '  FEHLT %-52s Exit %d (erwartet %s), Muster "%s" %s\n' "$name" "$code" "$wantw" "$pat" \
      "$(printf '%s' "$out" | grep -q -- "$pat" && echo gefunden || echo NICHT gefunden)"; schlecht=$((schlecht+1))
  fi
}
H="python3 tools/checks/hardware.py"
export -n HW_SCH HW_NETLIST HW_BOM HW_PCB HW_KOMP HW_V2 2>/dev/null; unset HW_SCH HW_NETLIST HW_BOM HW_PCB HW_KOMP HW_V2

echo "  --- Gegenrichtung: der unveraenderte Entwurf besteht"
probe "H1 H2 H3 am echten Schaltplan"  0 "H3 Pinregeln" $H --nur H1,H2,H3

echo "  --- H1/H2: Schaltplan veraendert (Kopie)"
python3 - "$SCH" "$T" <<'PY'
import sys, re
s = open(sys.argv[1], encoding="utf-8").read().split("\n")
i = next(k for k, l in enumerate(s) if "(no_connect" in l)
open(sys.argv[2] + "/ohne_nc.kicad_sch", "w", encoding="utf-8").write("\n".join(s[:i] + s[i + 1:]))
t = open(sys.argv[1], encoding="utf-8").read()
assert t.count('(label "TSOP"') >= 1
open(sys.argv[2] + "/label.kicad_sch", "w", encoding="utf-8").write(t.replace('(label "TSOP"', '(label "DCF77"', 1))
PY
probe "P1 ein No-Connect-Zeichen entfernt"  1 "H1" env HW_SCH="$T/ohne_nc.kicad_sch" $H --nur H1
probe "P2 ein Netzbezeichner umbenannt"     1 "H2 Netzliste weicht" env HW_SCH="$T/label.kicad_sch" $H --nur H2

echo "  --- H3: Pinregeln gegen eine veraenderte Netzliste"
"$CLI" sch export netlist --format kicadxml -o "$T/echt.xml" "$SCH" >/dev/null 2>&1
python3 - "$T" <<'PY'
import sys, xml.etree.ElementTree as ET, copy
T = sys.argv[1]
def laden(): return ET.parse(T + "/echt.xml")
def netz(root, name):
    return next(n for n in root.iter("net") if n.get("name") in (name, "/" + name))
# P3: gesperrter Pin IO45 bekommt ein Netz
t = laden(); r = t.getroot()
n = next(n for n in r.iter("net") if n.get("name", "").startswith("unconnected-(U2-IO45"))
n.set("name", "/FEHLER"); t.write(T + "/p3.xml")
# P4: Modulpin IO7 zusaetzlich am 5-V-Netz
t = laden(); r = t.getroot()
ET.SubElement(netz(r, "VCC_5V"), "node", ref="U2", pin="7", pinfunction="IO7_7"); t.write(T + "/p4.xml")
# P5: LDR statt an IO1 an IO15 (kein ADC1-Pin)
t = laden(); r = t.getroot()
for x in netz(r, "LDR").findall("node"):
    if x.get("ref") == "U2": x.set("pin", "8"); x.set("pinfunction", "IO15_8")
t.write(T + "/p5.xml")
# P6: IO0 an einem fremden Netz
t = laden(); r = t.getroot()
for x in netz(r, "BOOT").findall("node"):
    if x.get("ref") == "U2": netz(r, "DCF77").append(x); netz(r, "BOOT").remove(x); break
t.write(T + "/p6.xml")
PY
probe "P3 gesperrter Pin IO45 belegt"        1 "IO45 darf nicht belegt"   env HW_NETLIST="$T/p3.xml" $H --nur H3
probe "P4 5-V-Netz an einem Modulpin"        1 "5-V-Netz VCC_5V"          env HW_NETLIST="$T/p4.xml" $H --nur H3
probe "P5 LDR nicht an ADC1"                 1 "nicht an einem ADC1-Pin"  env HW_NETLIST="$T/p5.xml" $H --nur H3
probe "P6 IO0 an fremdem Netz"               1 "IO0 darf nur an"          env HW_NETLIST="$T/p6.xml" $H --nur H3

echo "  --- H4: Stueckliste"
python3 - "$T" <<'PY'
import sys
sys.path.insert(0, "tools/checks"); import hardware as h
parts = sorted(h.bom_parts())
open(sys.argv[1] + "/bom_gut.csv", "w", encoding="utf-8-sig").write(
    "Designator,Footprint,Quantity,Value,LCSC Part #\n" + "".join('%s,0805,1,x,C49678\n' % p for p in parts))
open(sys.argv[1] + "/bom_luecke.csv", "w", encoding="utf-8-sig").write(
    "Designator,Footprint,Quantity,Value,LCSC Part #\n" + "".join('%s,0805,1,x,C49678\n' % p for p in parts[1:]))
open(sys.argv[1] + "/bom_ohne_lcsc.csv", "w", encoding="utf-8-sig").write(
    "Designator,Footprint,Quantity,Value,LCSC Part #\n" + "".join('%s,0805,1,x,%s\n' % (p, "" if i == 0 else "C49678") for i, p in enumerate(parts)))
PY
probe "P7 vollstaendige Stueckliste (Gegenrichtung)" 0 "H4 Stueckliste: "      env HW_BOM="$T/bom_gut.csv" $H --nur H4
probe "P8 ein Bauteil fehlt in der Stueckliste"      1 "nicht in der Stueckliste" env HW_BOM="$T/bom_luecke.csv" $H --nur H4
probe "P9 eine Zeile ohne LCSC-Nummer"               1 "keine gueltige LCSC"     env HW_BOM="$T/bom_ohne_lcsc.csv" $H --nur H4

echo "  --- H5: Komponentenblatt"
mkdir "$T/komp"; printf '# Komponentenblatt: Test\n\n## 1. Daten\nein Wert ✔ S. 3\n' > "$T/komp/test.md"
probe "P10 Pflichtabschnitte fehlen"                 1 "Pflichtabschnitt fehlt"  env HW_KOMP="$T/komp" $H --nur H5

echo "  --- H6: V2-Referenz"
mkdir "$T/v2"; python3 - "$V2" "$T/v2" <<'PY'
import sys, os
src = sys.argv[1] + "/WordClock-USB C - STM32F411 - V2.kicad_sch"
t = open(src, encoding="utf-8").read()
assert t.count('(label "TSOP"') + t.count('(global_label "TSOP"') >= 1
t = t.replace('(label "TSOP"', '(label "TSOP_X"', 1) if '(label "TSOP"' in t else t.replace('(global_label "TSOP"', '(global_label "TSOP_X"', 1)
open(sys.argv[2] + "/WordClock-USB C - STM32F411 - V2.kicad_sch", "w", encoding="utf-8").write(t)
PY
probe "P11 V2-Schaltplan hat sich geaendert"         0 "HOCH      H6"            env HW_V2="$T/v2" $H --nur H6
probe "P12 V2 unveraendert (Gegenrichtung)"          0 "OK  H6 V2-Referenz"     $H --nur H6

echo "  --- H7: Leiterplattenformat"
python3 - "$V2PCB" "$T" <<'PY'
import sys
t = open(sys.argv[1], encoding="utf-8").read()
open(sys.argv[2] + "/pcb_gleich.kicad_pcb", "w", encoding="utf-8").write(t)
a = "(at 84.75 104.75"; assert t.count(a) >= 1
open(sys.argv[2] + "/pcb_loch.kicad_pcb", "w", encoding="utf-8").write(t.replace(a, "(at 85.75 104.75", 1))
# Das DRC braucht die Projektregeln neben dem Layout (sonst 504 falsche Abstandsfehler)
import shutil, os
d = os.path.dirname(sys.argv[1]); b = os.path.basename(sys.argv[1])[:-len(".kicad_pcb")]
for name in ("pcb_gleich", "pcb_loch"):
    for e in (".kicad_pro", ".kicad_dru"): shutil.copy(os.path.join(d, b + e), os.path.join(sys.argv[2], name + e))
shutil.copy(sys.argv[2] + "/pcb_gleich.kicad_pcb", sys.argv[2] + "/pcb_ohne_regeln.kicad_pcb")
PY
probe "P13 Layout 1:1 wie V2 (Gegenrichtung)"        0 "OK  H7 Format 1:1"       env HW_PCB="$T/pcb_gleich.kicad_pcb" $H --nur H7
probe "P14 Befestigungsloch um 1 mm verschoben"      1 "H7 H8 verschoben"        env HW_PCB="$T/pcb_loch.kicad_pcb" $H --nur H7
probe "P14b Projektregeln fehlen neben dem Layout"   1 "DRC nicht aussagekraeftig" env HW_PCB="$T/pcb_ohne_regeln.kicad_pcb" $H --nur H7

echo "  --- H8: LED-Board-Schnittstelle 1:1 (gegen eine veraenderte Netzliste)"
python3 - "$T" <<'PY'
import sys, xml.etree.ElementTree as ET
T = sys.argv[1]
src = "hardware/esp32s3/v2/v2.xml"            # V2 selbst: die Schnittstelle ist trivial gleich
def laden(): return ET.parse(src)
def netz(root, name): return next(n for n in root.iter("net") if n.get("name") in (name, "/" + name))
laden().write(T + "/h8_gleich.xml")
# P15: R22 mit anderem Wert
t = laden(); r = t.getroot()
for c in r.iter("comp"):
    if c.get("ref") == "R22": c.find("value").text = "47k\u03a9"
t.write(T + "/h8_wert.xml")
# P16: R19 Pin 1 (WS2812_POWER) an einem anderen Netz
t = laden(); r = t.getroot()
for x in netz(r, "WS2812_POWER").findall("node"):
    if x.get("ref") == "R19": netz(r, "WS2812_BUS").append(x); netz(r, "WS2812_POWER").remove(x); break
t.write(T + "/h8_pin.xml")
# P17: zusaetzliches Teil am LDR-Netz
t = laden(); r = t.getroot()
ET.SubElement(netz(r, "LDR"), "node", ref="C32", pin="1", pinfunction="1"); t.write(T + "/h8_zusatz.xml")
# P18: Q2 faellt aus dem Pfad
t = laden(); r = t.getroot()
for x in netz(r, "Net-(Q2-G)").findall("node"):
    if x.get("ref") == "R20": netz(r, "Net-(Q2-G)").remove(x); break
t.write(T + "/h8_fehlt.xml")
PY
probe "P15 V2-Schnittstelle unveraendert (Gegenrichtung)" 0 "OK  H8 LED-Board"      env HW_NETLIST="$T/h8_gleich.xml" $H --nur H8
probe "P16 R22 mit anderem Wert"                         1 "R22 hat den Wert"       env HW_NETLIST="$T/h8_wert.xml" $H --nur H8
probe "P17 Pin der LED-Versorgung umgehaengt"            1 "R19 Pin 1 liegt an"     env HW_NETLIST="$T/h8_pin.xml" $H --nur H8
probe "P18 zusaetzliches Teil am LDR-Netz"               0 "HOCH      H8 Netz LDR"  env HW_NETLIST="$T/h8_zusatz.xml" $H --nur H8
probe "P19 Gate-Pull-down R20 faellt weg"                1 "R20 Pin 1 liegt an"     env HW_NETLIST="$T/h8_fehlt.xml" $H --nur H8

# H9 Lesbarkeit: der verworfene Stummel-Entwurf (Commit aa5ff3c) muss durchfallen, der V2-Plan des Nutzers bestehen
git show aa5ff3c:hardware/esp32s3/WordClock-ESP32S3-V3.kicad_sch > "$T/stummel.kicad_sch" 2>/dev/null
probe "P20 Stummel-Plan (verworfener Entwurf)"           1 "KRITISCH  H9"           env HW_SCH="$T/stummel.kicad_sch" $H --nur H9
probe "P21 V2-Plan des Nutzers besteht H9 (Gegenrichtung)" 0 "OK  H9 Lesbarkeit"    env HW_SCH="$V2/WordClock-USB C - STM32F411 - V2.kicad_sch" $H --nur H9

printf '      %d Proben bestanden, %d fehlgeschlagen\n' "$ok" "$schlecht"
[ "$schlecht" -eq 0 ] || exit 1
