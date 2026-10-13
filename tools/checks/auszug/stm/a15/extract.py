# Pruefstand a15 (A15, Paket 2026-10-13, S1.1): schneidet die ECHTE UART-ISR aus src/uart/uart-driver.h
# (Kandidat) und aus uart-driver-alt.h (Referenz = Stand ec19f33, vor A15) und macht aus beiden je eine
# Funktion, die gegen eine Registerattrappe laeuft. Bitlagen kommen aus den Geraete- und SPL-Headern
# BEIDER Plattformen (F103: wclock24h-F103/..., F411: inc/, SPL/) und muessen uebereinstimmen.
# Aufruf: extract.py <src-Verzeichnis> <arbeitsverzeichnis> <root> [sabotage]
#   Sabotagen: txeie rxneie ore-nach-dr dr-nicht-bei-ore txe-nicht-aus
import sys, re, os
src, W, root = sys.argv[1], sys.argv[2], sys.argv[3]
sab = sys.argv[4] if len(sys.argv) > 4 else ''
here = os.path.dirname(os.path.abspath(__file__))
def rd(p): return open(p, 'rb').read().decode('latin-1').replace('\r\n', '\n')
def one(t, s, what):
    n = t.count(s); assert n == 1, (what, s, n); return t.index(s)

statisch = []

# --- Bitlagen beider Plattformen -------------------------------------------------------------------
NAMEN = ['USART_FLAG_RXNE', 'USART_FLAG_ORE', 'USART_FLAG_TXE', 'USART_IT_RXNE', 'USART_IT_TXE',
         'USART_CR1_RXNEIE', 'USART_CR1_TXEIE', 'USART_SR_RXNE', 'USART_SR_ORE', 'USART_SR_TXE']
plattform = {
    'F103': [root + '/wclock24h-F103/inc/stm32f10x.h', root + '/wclock24h-F103/SPL/inc/stm32f10x_usart.h'],
    'F411': [root + '/inc/stm32f4xx.h',               root + '/SPL/inc/stm32f4xx_usart.h'],
}
werte = {}
for p, dateien in plattform.items():
    t = ''.join(rd(d) for d in dateien)
    for n in NAMEN:
        m = re.findall(r'^#define\s+' + n + r'\s+\(\(uint16_t\)(0x[0-9A-Fa-f]+)\)', t, re.M)
        assert len(m) == 1, (p, n, m)
        werte.setdefault(n, {})[p] = int(m[0], 16)
for n in NAMEN:
    if werte[n]['F103'] != werte[n]['F411']:
        statisch.append('Bitlage %s: F103 0x%04X, F411 0x%04X' % (n, werte[n]['F103'], werte[n]['F411']))
# SR-Bit == FLAG-Bit, CR1-Freigabe an derselben Stelle wie das Flag (RXNE/RXNEIE Bit 5, TXE/TXEIE Bit 7)
for a, b in [('USART_SR_RXNE', 'USART_FLAG_RXNE'), ('USART_SR_ORE', 'USART_FLAG_ORE'), ('USART_SR_TXE', 'USART_FLAG_TXE')]:
    if werte[a]['F411'] != werte[b]['F411']: statisch.append('%s != %s' % (a, b))
# SPL-Kodierung von USART_IT_x: Bit (x >> 8) in SR, Bit (x & 0x1F) im Register ((uint8_t) x >> 5) == 1 (CR1)
for it, fl, ie in [('USART_IT_RXNE', 'USART_FLAG_RXNE', 'USART_CR1_RXNEIE'), ('USART_IT_TXE', 'USART_FLAG_TXE', 'USART_CR1_TXEIE')]:
    v = werte[it]['F411']
    if (1 << (v >> 8)) != werte[fl]['F411'] or ((v & 0xFF) >> 5) != 1 or (1 << (v & 0x1F)) != werte[ie]['F411']:
        statisch.append('SPL-Kodierung %s passt nicht zu %s/%s' % (it, fl, ie))
open(W + '/flags', 'w').write(' '.join('-D%s=0x%04X' % (n, werte[n]['F411']) for n in NAMEN))

# --- ISR ausschneiden und umbauen ------------------------------------------------------------------
KOPF = 'void UART_IRQ_HANDLER (void)\n{\n'
def isr(t, pfad):
    a = one(t, KOPF, pfad); b = t.index('\n}\n', a) + 3
    return a, t[a:b], t[:a].count('\n') + 1

def ohne_kommentare(s):
    s = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), s, flags=re.S)
    return re.sub(r'//[^\n]*', '', s)

def sabotiere(body, s):
    def r(alt, neu):
        assert body.count(alt) == 1, ('Sabotage-Anker', s, alt, body.count(alt)); return body.replace(alt, neu)
    if s == 'txeie':            return r('(cr1 & USART_CR1_TXEIE) && ', '')
    if s == 'rxneie':           return r('(cr1 & USART_CR1_RXNEIE) && ', '')
    if s == 'txe-nicht-aus':    return r('UART_NAME->CR1 &= (uint16_t) ~USART_CR1_TXEIE;', ';')
    if s == 'dr-nicht-bei-ore': return r('ch = UART_NAME->DR & 0xFF;', 'ch = (sr & USART_FLAG_ORE) ? 0 : (UART_NAME->DR & 0xFF);')
    if s == 'ore-nach-dr':
        body = r('        ch = UART_NAME->DR & 0xFF;', '')
        return r('        if ((sr & USART_FLAG_ORE) != 0)',
                 '        ch = UART_NAME->DR & 0xFF;\n        if ((UART_NAME->SR & USART_FLAG_ORE) != 0)')
    raise SystemExit('unbekannte Sabotage: ' + s)

GLOBALE = ['uart_rxbuf', 'uart_rxsize', 'uart_rxmax', 'uart_rxdrops', 'uart_rxore', 'uart_txbuf', 'uart_txsize',
           'uart_rxstop', 'uart_txstart']
def baue(t, pfad, name, s):
    a, body, zeile = isr(t, pfad)
    body = ohne_kommentare(body)
    spl = re.findall(r'\bUSART_(GetITStatus|ClearITPendingBit|ReceiveData|SendData|ITConfig)\b', body)
    sr_lesen = len(re.findall(r'UART_NAME->SR\b(?!\s*=[^=])', body))
    if s: body = sabotiere(body, s)
    # die beiden static-Indizes der ISR in den Kontext verlegen (sonst nicht ruecksetzbar)
    body, n = re.subn(r'^[ \t]*static uint_fast16_t\s+(uart_rxstop|uart_txstart)\s*=\s*0;[ \t]*$', '', body, flags=re.M)
    assert n == 2, ('static-Indizes', pfad, n)
    body = body.replace(KOPF, 'static void\n%s (KTX * K)\n{\n' % name, 1)
    # Registerzugriffe auf die Attrappe umlenken; Zahl je Art festhalten (DIR-014: Faelle zaehlen)
    z = {}
    body, z['wr_DR']  = re.subn(r'UART_NAME->DR\s*=\s*([^;]+);', r'wr_DR (UART_NAME, \1);', body)
    body, z['and_CR1'] = re.subn(r'UART_NAME->CR1\s*&=\s*([^;]+);', r'wr_CR1 (UART_NAME, rd_CR1 (UART_NAME) & (\1));', body)
    body, z['or_CR1'] = re.subn(r'UART_NAME->CR1\s*\|=\s*([^;]+);', r'wr_CR1 (UART_NAME, rd_CR1 (UART_NAME) | (\1));', body)
    body, z['wr_SR']  = re.subn(r'UART_NAME->SR\s*=\s*([^;=][^;]*);', r'wr_SR (UART_NAME, \1);', body)
    body, z['rd']     = re.subn(r'UART_NAME->(SR|DR|CR1|CR2|CR3)\b', r'rd_\1 (UART_NAME)', body)
    assert 'UART_NAME->' not in body, ('unbehandelter Registerzugriff', pfad)
    kopf = ''.join('#define %s K->%s\n' % (g, g[5:]) for g in GLOBALE)
    fuss = ''.join('#undef %s\n' % g for g in GLOBALE)
    print('  %s: %s Zeile %d, SPL-Aufrufe %d, SR-Leseorte %d, umgelenkt %s' % (name, pfad, zeile, len(spl), sr_lesen,
          ', '.join('%s %d' % kv for kv in z.items())))
    return kopf + '#line %d "%s"\n' % (zeile, pfad) + body + fuss, spl, sr_lesen

alt, _, _ = baue(rd(here + '/uart-driver-alt.h'), 'uart-driver-alt.h', 'isr_alt', '')
neu, spl, sr_lesen = baue(rd(src + '/uart/uart-driver.h'), 'uart-driver.h', 'isr_neu', sab)
open(W + '/isr.inc', 'w').write(alt + '\n' + neu)
if spl:
    statisch.append('SPL-Aufrufe in der ISR: %d (%s)' % (len(spl), ', '.join('USART_' + s for s in spl)))
if sr_lesen != 1:
    statisch.append('SR wird in der ISR an %d Stellen gelesen, Soll 1 (design.md 2.1)' % sr_lesen)
open(W + '/statisch', 'w').write(''.join(s + '\n' for s in statisch))
