"""Kennzahlen zur Lesbarkeit eines KiCad-Schaltplans (Pruefung H9, hardware.py).

Gemessen wird, was den ersten V3-Entwurf unlesbar machte: jedes Netz ein einzelner Drahtstummel mit
Bezeichner (Label-Stummel), sichtbare automatische Namen 'Net-(...)', kaum Junctions. Die Werte des
Vorbilds (V2-Plan des Nutzers) stehen in hardware/komponenten/schaltplan-gestaltung.md; die Schwellen
in hardware.py (h9) sind daraus abgeleitet und bis zur Bestaetigung durch den Nutzer Annahmen.
"""
import sys,re,math,os
sys.path.insert(0,os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','hardware','esp32s3'))
import sx
def lesen(path):
    root=sx.parse(open(path,encoding='utf-8').read())
    wires=[];labels=[];junc=0;syms=0;pw=0
    for n in root:
        if not isinstance(n,list) or not n: continue
        if n[0]=='wire':
            pts=[c for c in n if isinstance(c,list) and c[0]=='pts'][0]
            xy=[(round(float(c[1]),2),round(float(c[2]),2)) for c in pts[1:] if isinstance(c,list) and c[0]=='xy']
            wires.append(xy)
        elif n[0] in('label','global_label','hierarchical_label'):
            at=[c for c in n if isinstance(c,list) and c[0]=='at'][0]
            labels.append((sx.sval(n[1]),(round(float(at[1]),2),round(float(at[2]),2))))
        elif n[0]=='junction': junc+=1
        elif n[0]=='symbol' and any(isinstance(c,list) and c[0]=='lib_id' for c in n):
            lid=[sx.sval(c[1]) for c in n if isinstance(c,list) and c[0]=='lib_id'][0]
            if lid.startswith('power:'): pw+=1
            else: syms+=1
    return wires,labels,junc,syms,pw
def komp(wires):
    par={}
    def f(a):
        while par.setdefault(a,a)!=a: par[a]=par[par[a]]; a=par[a]
        return a
    for w in wires:
        f(w[0]);f(w[1]);par[f(w[0])]=f(w[1])
    c={}
    for i,w in enumerate(wires): c.setdefault(f(w[0]),[]).append(i)
    return c
def auswerten(path):
    wires,labels,junc,syms,pw=lesen(path)
    comps=komp(wires)
    lab_pts={}
    for nm,p in labels: lab_pts.setdefault(p,[]).append(nm)
    stub=0
    for k,idx in comps.items():
        if len(idx)==1:
            w=wires[idx[0]]
            L=math.hypot(w[0][0]-w[1][0],w[0][1]-w[1][1])
            if L<=7.7 and (w[0] in lab_pts or w[1] in lab_pts): stub+=1
    netlab=sum(1 for nm,_ in labels if nm.startswith('Net-('))
    return dict(draehte=len(wires),komp=len(comps),bauteile=syms,power=pw,junctions=junc,labels=len(labels),stubs=stub,
                stubanteil=round(stub/max(1,len(comps)),2),netlabels=netlab,draehte_je_bauteil=round(len(wires)/max(1,syms),2))
if __name__=='__main__':
    for p in sys.argv[1:]: print(p.split('/')[-1],auswerten(p))
