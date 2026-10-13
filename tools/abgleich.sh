#!/usr/bin/env bash
# Wertet jeden Vollabgleich des STM in einem Mitschnitt aus (A61, Paket 2026-10-11, Ent-8).
#
#   ./tools/abgleich.sh <mitschnitt.log>      eine Datei
#   ./tools/logger/log.sh grep "2026-10-11T0" 20000 | ./tools/abgleich.sh -
#
# Ein Abgleich beginnt mit einem Ausloeser und endet mit "info: configuration sent" (main.c).
# Ausloeser sind genau die Zeilen, die den Abgleich vormerken (main.c, esp8266.c):
#   "(IPADDRESS a.b.c.d)", "(SYNCVARS)" -- exakt, "(SYNCVARS 1)" merkt NICHT vor --
#   und "cmd abgewiesen #n len=l", diese aber nur, wenn seit der letzten vormerkenden
#   Abweisung ueber 60 s vergangen sind (Drossel in esp8266.c; Abweisungen kommen in Schueben).
#   Liegt der Abstand zwischen 58 und 62 s (Sekundenraster des STM), ist es unsicher und wird
#   nur als Hinweis gefuehrt. Abweisungen #5 bis #49 sind am STM stumm, merken aber ebenfalls
#   vor; ab #5 ist der Stand der Drossel deshalb unbekannt, und eine sichtbare Abweisung
#   (#50, #100 ...) ist nur ein Hinweis. Ein stummer Ausloeser bleibt "unbekannt" -- nie geraten.
#
# Zuordnung: Der ERSTE Ausloeser nach dem vorigen Ende gehoert zum naechsten Abgleich. Kommt
# waehrend eines laufenden Abgleichs ein weiterer (main.c loescht das Flag VOR dem Senden, der
# ESP wiederholt SYNCVARS alle rund 10 s), gehoert er zum FOLGENDEN Abgleich, dessen Beginn
# dann das vorige Ende ist. Kam er in Wahrheit vor dem Beginn, laeuft nur EIN Abgleich
# (zusammengefasst); das ist aus dem Log nicht sicher zu trennen und deshalb nur ein Hinweis.
#
# Ob ein Abgleich lief, entscheidet die diag-Zeile, nicht die Uhr: Sie steht nur am KOPF des
# Hauptloops (main.c), die Abgleichstelle in jedem Durchlauf danach. Ein Ausloeser kann aber in
# einer Warteschleife von var_send_buf() NACH der Abgleichstelle ankommen (IR, Nachsendung,
# LDR); dann folgt erst die diag-Zeile des naechsten Durchlaufs und danach der Abgleich.
# Deshalb beweist erst die ZWEITE diag-Zeile ohne Endzeile dazwischen "ohne Abgleich"
# (etwa im Spiel, A53). Ohne sie laeuft der Abgleich noch, wie lange auch immer. Review 0.2,
# Runde 3. Nach der ERSTEN diag-Zeile beginnt der Abgleich fruehestens dort; Dauer, dv und
# Zeitueberschreitungen zaehlen dann ab ihr.
#
# Neustart: die Zeile "Version: x.y.z", die jeder Start ausgibt (main.c), noch vor dem
# Abgleich beim Boot; Rueckfall ist eine fallende diag-Folge. Nach dem Start zaehlt dv ab 0,
# die Luecke heisst "Start". Ein offener Ausloeser vor dem
# Neustart ist "abgebrochen" -- der Ernstfall aus A34, Reset mitten im Abgleich.
#
# Je Abgleich: Ausloeser, Dauer (Ausloeser bis Ende; bei "unbekannt" eine Obergrenze ab der
# letzten diag-Zeile bzw. dem vorigen Ende, als "<=x"), dv (beide Zahlen von v=), du (Uptime),
# Zeitueberschreitungen ("keine Quittung" im Fenster) und die Luecke der diag-Folge. Die Luecke
# ist eine UNTERGRENZE des Stillstands (das diag-Raster verschiebt sich), darum "≥".
#
# Zeilen aus dem Firmware-Abbild, das beim STM-Flashen durch den Mitschnitt laeuft, enthalten
# dieselben Woerter als Formatzeichenketten. Gezaehlt wird nur, was dem Wortlaut GANZ entspricht.
#
# Erst ab STM 3.2.22 steht die Endzeile immer im Log (vorher beim IPADDRESS-Weg nur als
# Debugausgabe, bei SYNCVARS anders lautend, Commit 3ee0e0a). Entscheidend ist die Version ZUM
# ZEITPUNKT DES AUSLOESERS; davor -- oder unbekannt, solange noch keine Endzeile vorkam -- sind
# "ohne Abgleich" und "abgebrochen" nur Hinweise.
#
# Exit 0 ruhig; 1 ein Abgleich ueber 5 s, mit ueber 10 Zeitueberschreitungen, mit Luecke ueber
# 5 s, abgebrochen oder ohne Abgleich; 2 falscher Aufruf; 3 Endzeilen im Rohtext und
# ausgewertete Abgleiche weichen ab (DIR-014: nachzaehlen); 4 Fehler im Werkzeug selbst.
set -uo pipefail

[ $# -eq 1 ] || { echo "Aufruf: $0 <mitschnitt.log | ->" >&2; exit 2; }
QUELLE=$1
[ "$QUELLE" = "-" ] || [ -r "$QUELLE" ] || { echo "Datei nicht lesbar: $QUELLE" >&2; exit 2; }

# Das Skript steht in einer Variablen, nicht auf stdin: Mit "python3 - <<PY" las Python sein
# Skript aus dem Heredoc, und "-" fand danach ein leeres stdin -- die dokumentierte Pipe-Form
# meldete "0 Abgleich(e) ausgewertet" mit Exit 0 (L414).
SKRIPT=$(cat <<'PY'
import re, sys, traceback
from datetime import datetime

ENDE_TEXT = 'info: configuration sent'
STEMPEL = re.compile(r'^(\d{4}-\d\d-\d\dT\d\d:\d\d:\d\d\.\d{3})([+-]\d\d:\d\d)\s+(.*)$')
DIAG = re.compile(r'^diag (\d+) l=\d+ t=\d+ u=(\d+) .*\bv=(\d+)/(\d+)\b')
IPADR = re.compile(r'^\(IPADDRESS \d{1,3}(?:\.\d{1,3}){3}\)$')
ABGEW = re.compile(r'^cmd abgewiesen #(\d+) len=\d+$')
QUITT = re.compile(r'^var_send_buf: keine Quittung nach \d+s, ')
VERSION = re.compile(r'^Version: (\d+)\.(\d+)\.(\d+)$')

def zeit(stempel, offset):
    # fromisoformat nimmt "+02:00" ab Python 3.7, drei Nachkommastellen ueberall
    return datetime.fromisoformat(stempel + offset).timestamp()

class Ausloeser:
    def __init__(self, t, datum, uhr, art, diag_vor, belegt, zusammen=False):
        self.t, self.datum, self.uhr, self.art = t, datum, uhr, art
        self.diag_vor, self.belegt, self.zusammen = diag_vor, belegt, zusammen
        self.diags = 0             # diag-Zeilen seit dem Ausloeser bzw. seit dem vorigen Ende

def lauf(src):
    fh = sys.stdin if src == "-" else open(src, encoding="utf-8", errors="replace")
    zeilen, roh_enden = [], 0
    for roh in fh:
        roh = roh.rstrip("\n")
        if roh.rstrip().endswith(ENDE_TEXT) and '%' not in roh:
            roh_enden += 1
        m = STEMPEL.match(roh)
        if not m:
            continue
        zeilen.append((zeit(m.group(1), m.group(2)), m.group(1)[:10], m.group(1)[11:23], m.group(3).strip()))

    st = dict(alarm=False, enden=0, fw_ende=None)
    aus = []                       # Ausgabezeilen; Abgleiche als Tupel, Rest als Text
    letzte_diag = None             # (zeit, seq, u, vt, vn)
    offen = None                   # laufender bzw. erwarteter Abgleich
    naechster = None               # Ausloeser, der waehrend eines Abgleichs kam
    vorende_t = None
    cmc_letzt = None               # letzte vormerkende Abweisung (Drossel nachgebildet)
    quitt = []                     # Zeitpunkte von "keine Quittung"
    erste_t = zeilen[0][0] if zeilen else None

    def belegt():
        return st['fw_ende'] if st['fw_ende'] is not None else st['enden'] > 0

    def text(datum, uhr, ausl, dauer, dv, q, marke):
        aus.append(f"{datum} {uhr:12}  {ausl:22}  {dauer:>7}  {dv:>9}  {'–':>6}  {q:>6}  {'–':>7}{marke}")

    def schliessen(o, wie, t_bis):
        # wie: "ABGEBROCHEN" (Neustart) oder "OHNE ABGLEICH" (diag ohne Ende)
        q = str(sum(1 for z in quitt if o.t <= z <= t_bis))
        if o.zusammen:
            text(o.datum, o.uhr, o.art + " zusammengefasst?", "–", "–", q, "  (kein eigener Abgleich erkennbar, Hinweis)")
        elif o.belegt:
            marke = "  <-- Neustart vor dem Ende" if wie == "ABGEBROCHEN" else "  <-- Hauptloop lief, kein Abgleich"
            text(o.datum, o.uhr, f"{o.art} {wie}", f"<={t_bis-o.t:.1f}s" if wie == "ABGEBROCHEN" else "–",
                 "Neustart" if wie == "ABGEBROCHEN" else "–", q, marke)
            st['alarm'] = True
        else:
            text(o.datum, o.uhr, o.art + " ohne Endzeile", "–", "–", q, "  (Firmware ohne Endzeile, Hinweis)")

    def neustart(t):
        nonlocal offen, naechster, letzte_diag, cmc_letzt, quitt
        for o in (offen, naechster):
            if o:
                schliessen(o, "ABGEBROCHEN", t)
        offen = naechster = None
        letzte_diag = (t, 0, 0, 0, 0)   # Start: alle Zaehler beginnen bei 0, diag-Folge bei 1
        cmc_letzt = None
        quitt = []

    for (t, datum, uhr, txt) in zeilen:
        vm = VERSION.match(txt)
        if vm:
            neustart(t)
            st['fw_ende'] = tuple(int(x) for x in vm.groups()) >= (3, 2, 22)
            continue
        d = DIAG.match(txt)
        if d:
            seq, u, vt, vn = int(d.group(1)), int(d.group(2)), int(d.group(3)), int(d.group(4))
            if letzte_diag and seq <= letzte_diag[1]:
                neustart(t)          # Rueckfall, falls die Version-Zeile fehlt
            if offen:
                offen.diags += 1
                if offen.diags == 1:
                    # Der Abgleich beginnt fruehestens nach dieser diag-Zeile (Kopf des Durchlaufs).
                    offen.t = t
                    offen.diag_vor = (t, seq, u, vt, vn)
                if offen.diags >= 2:
                    # Zwei Durchlaeufe mit Abgleichstelle, kein Ende: es gab keinen Abgleich.
                    schliessen(offen, "OHNE ABGLEICH", t)
                    offen, naechster = naechster, None
                    if offen:
                        # Er kam VOR dieser diag-Zeile und ebenfalls ohne Abgleich davor: Diese
                        # Zeile zaehlt fuer ihn als erste (Review 0.2, Runde 4).
                        offen.diags, offen.t, offen.diag_vor = 1, t, (t, seq, u, vt, vn)
            letzte_diag = (t, seq, u, vt, vn)
            continue
        if QUITT.match(txt):
            quitt.append(t)
            continue
        art = None
        if txt == '(SYNCVARS)':
            art = 'SYNCVARS'
        elif IPADR.match(txt):
            art = 'IPADDRESS'
        elif ABGEW.match(txt):
            nr = int(ABGEW.match(txt).group(1))
            abstand = None if cmc_letzt is None else t - cmc_letzt
            if nr > 4:
                cmc_letzt = None   # stumme Abweisungen davor: Drossel unbekannt
                aus.append(f"{datum} {uhr:12}  {'CMC #' + str(nr):22}  {'–':>7}  {'–':>9}  {'–':>6}  {'–':>6}  {'–':>7}  (nach stummen Abweisungen, Drossel unbekannt, Hinweis)")
            elif abstand is None or abstand > 62:
                art, cmc_letzt = 'CMC-Abweisung', t
            elif abstand >= 58:
                cmc_letzt = t      # koennte vorgemerkt haben: Folgezeilen ebenfalls unsicher
                aus.append(f"{datum} {uhr:12}  {'CMC unsicher':22}  {'–':>7}  {'–':>9}  {'–':>6}  {'–':>6}  {'–':>7}  (Abstand {abstand:.0f} s an der Drossel, Hinweis)")
        if art:
            if offen is None:
                offen = Ausloeser(t, datum, uhr, art, letzte_diag, belegt())
            elif naechster is None:
                naechster = Ausloeser(t, datum, uhr, art, None, belegt())
            continue
        if txt == ENDE_TEXT:
            st['enden'] += 1
            if offen:
                start, ausl, dvor = offen.t, offen.art, offen.diag_vor
                dauer = t - start
                dauer_txt = f"{dauer:6.2f}s"
            else:
                grenzen = [x for x in (vorende_t, letzte_diag[0] if letzte_diag else None) if x is not None]
                start = max(grenzen) if grenzen else erste_t
                ausl, dvor, dauer = "unbekannt", letzte_diag, None
                dauer_txt = f"<={t-start:.1f}s"
            q = sum(1 for z in quitt if start <= z <= t)
            aus.append((t, datum, uhr, ausl, dauer, dauer_txt, dvor, q))
            vorende_t = t
            if naechster:
                offen = Ausloeser(t, naechster.datum, naechster.uhr, naechster.art, letzte_diag, naechster.belegt, zusammen=True)
                naechster = None
            else:
                offen = None
            continue

    # dv, du und Luecke brauchen die erste diag-Zeile NACH dem Ende: zweiter Durchgang.
    diags = [(t, int(m.group(1)), int(m.group(2)), int(m.group(3)), int(m.group(4)))
             for (t, _, _, txt) in zeilen for m in [DIAG.match(txt)] if m]
    def diag_nach(t):
        for x in diags:
            if x[0] > t:
                return x
        return None
    ausgabe = []
    for z in aus:
        if isinstance(z, str):
            ausgabe.append(z); continue
        t, datum, uhr, ausl, dauer, dauer_txt, dvor, q = z
        nach = diag_nach(t)
        if dvor and nach and nach[1] > dvor[1]:
            dv = f"{nach[3]-dvor[3]:+d}/{nach[4]-dvor[4]:+d}"
            du = f"{nach[2]-dvor[2]:+d}"
            luecke = max(0.0, nach[0] - dvor[0] - 10.0 * (nach[1] - dvor[1]))
            l_txt = f"≥{luecke:.1f}s"
            if dvor[1] == 0:
                luecke, l_txt = None, "Start"   # kein Raster vor der ersten diag-Zeile
        elif dvor and nach:
            dv, du, luecke, l_txt = "Neustart", "–", None, "–"
        else:
            dv, du, luecke, l_txt = "?", "?", None, "?"
        marke = ""
        if (dauer is not None and dauer > 5) or (luecke is not None and luecke > 5) or q > 10:
            marke = "  <-- ueber Grenze"
            st['alarm'] = True
        ausgabe.append(f"{datum} {uhr:12}  {ausl:22}  {dauer_txt:>7}  {dv:>9}  {du:>6}  {q:>6}  {l_txt:>7}{marke}")
    for o in (offen, naechster):
        if o:
            ausgabe.append(f"{o.datum} {o.uhr:12}  {o.art + ' offen':22}  {'?':>7}  {'?':>9}  {'?':>6}  {'?':>6}  {'?':>7}  (Mitschnittende, Hinweis)")

    print(f"{'Datum':10} {'Ende':12}  {'Ausloeser':22}  {'Dauer':>7}  {'dv':>9}  {'du':>6}  {'Quitt.':>6}  {'Luecke':>7}")
    for z in ausgabe:
        print(z)
    print(f"\n{st['enden']} Abgleich(e) ausgewertet, {roh_enden} Endzeile(n) im Rohtext, {len(zeilen)} Zeilen gelesen.")
    if not zeilen:
        print("FEHL: keine Zeile gelesen -- leere Eingabe ist kein Befund ohne Abgleich, sondern keine Aussage.", file=sys.stderr)
        return 2
    if st['enden'] != roh_enden:
        print("FEHL: Endzeilen im Rohtext und ausgewertete Abgleiche weichen ab (Zeile ohne Zeitstempel?).", file=sys.stderr)
        return 3
    if st['alarm']:
        print("FEHL: mindestens ein Abgleich ueber der Grenze, abgebrochen oder ohne Abgleich.", file=sys.stderr)
        return 1
    return 0

try:
    sys.exit(lauf(sys.argv[1]))
except SystemExit:
    raise
except Exception:
    traceback.print_exc()
    print("FEHL: Werkzeugfehler, keine Aussage ueber den Mitschnitt.", file=sys.stderr)
    sys.exit(4)
PY
)
exec python3 -c "$SKRIPT" "$QUELLE"
