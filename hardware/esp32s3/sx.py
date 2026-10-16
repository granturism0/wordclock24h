import re
TOK=re.compile(r'\(|\)|"(?:\\.|[^"\\])*"|[^\s()]+')
def parse(text):
    toks=TOK.findall(text); pos=0
    def rd():
        nonlocal pos
        t=toks[pos]; pos+=1
        if t=='(':
            l=[]
            while toks[pos]!=')': l.append(rd())
            pos+=1; return l
        if t.startswith('"'): return ('s',bytes(t[1:-1],'utf-8').decode('unicode_escape') if '\\' in t else t[1:-1])
        return t
    return rd()
def sval(x): return x[1] if isinstance(x,tuple) else x
def block_at(text,i):
    d=0;q=False;j=i
    while True:
        c=text[j]
        if q:
            if c=='\\': j+=1
            elif c=='"': q=False
        else:
            if c=='"': q=True
            elif c=='(': d+=1
            elif c==')':
                d-=1
                if d==0: return text[i:j+1]
        j+=1
def find_symbol(text,name):
    m=re.search(r'\n\t+\(symbol "%s"\n'%re.escape(name),text)
    return block_at(text,m.start()+1)
def pins_of(symtext):
    t=parse(symtext); out=[]
    def walk(n):
        if isinstance(n,list) and n and n[0]=='pin' and len(n)>3:
            at=[c for c in n if isinstance(c,list) and c and c[0]=='at'][0]
            ln=[c for c in n if isinstance(c,list) and c and c[0]=='length'][0]
            nm=[c for c in n if isinstance(c,list) and c and c[0]=='name'][0]
            nu=[c for c in n if isinstance(c,list) and c and c[0]=='number'][0]
            hide=any(isinstance(c,list) and c and c[0]=='hide' for c in n) or 'hide' in n
            out.append(dict(x=float(at[1]),y=float(at[2]),a=int(float(at[3])),len=float(ln[1]),name=sval(nm[1]),num=sval(nu[1]),type=n[1],hide=hide))
        elif isinstance(n,list):
            for c in n: walk(c)
    walk(t); return out
