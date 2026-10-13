#!/usr/bin/env python3
"""l55e: erzeugt l55e_code.inc aus ESP-uclock.ino und http.cpp (neu oder alt).
Echter Code: Bruecke (VAR_CRC_MARK bis loop()), RPCR-Ablage, stm_bridge_poll() mit den Zweigen var/LOG/RPCR
(die uebrigen Zweige brauchen Wetter, WLAN, Dateisystem und werden weggelassen), aus http.cpp der Block
HTTP_RPC_WAIT_MSEC bis vor den LDR-ADC-Kommentar und http_api_ldr_max_set(). Fehlt der neue Code (Gegenprobe),
wird der alte Rumpf aus loop() als stm_bridge_poll() verpackt und die alten Handler genommen -- das Ergebnis
uebersetzt sich und schlaegt fehl (FEHL), statt nicht zu uebersetzen.
Zielbreite: Deklarationen "unsigned long" -> uint32_t (L256)."""
import sys, re
ino, http, out = sys.argv[1], sys.argv[2], sys.argv[3]
def lines(p): return open(p, 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
I = lines(ino); H = lines(http)
def idx(L, pred, start=0):
    return next((k for k in range(start, len(L)) if pred(L[k])), None)
def func(L, name):
    i = idx(L, lambda l: l.startswith(name + ' (') or l.startswith(name + '('))
    if i is None: return None
    j = idx(L, lambda l: l.startswith('}'), i)
    return '\n'.join(L[i - 1:j + 1]) + '\n'
parts = []
# Kopf: Puffergroesse, Ring-Konstanten, Kuerzungsmarke
a = idx(I, lambda l: l.startswith('#define CMD_BUFFER_SIZE'))
parts += [I[a]] + [l for l in I[a:a + 40] if l.startswith('#define STM32_LOG_') or l.startswith('#define LOG_TRUNC_MARK')]
# RPCR-Ablage (neu)
a = idx(I, lambda l: l.startswith('#define STM_RPCR_SLOTS'))
neu = a is not None
if neu:
    b = idx(I, lambda l: l.startswith('/*----'), a)
    parts += I[a:b]
else:
    parts += ['#define STM_RPCR_SLOTS 2', 'static uint8_t stm_rpcr_seq[2], stm_rpcr_result[2];',
              'uint_fast8_t stm_rpcr_get (uint_fast8_t, uint_fast8_t *) { return 0; }   /* alter Stand: gibt es nicht */']
# Bruecke (VAR_CRC_MARK bis vor loop())
a = idx(I, lambda l: l.startswith('#define VAR_CRC_MARK '))
lp = idx(I, lambda l: l.startswith('loop()'))
parts += I[a:lp - 1]
# stm_bridge_poll (neu) oder der Rumpf aus loop() (alt)
if neu:
    s = idx(I, lambda l: l.startswith('stm_bridge_poll (void)'))
    e = idx(I, lambda l: l.startswith('}'), s)
    body = I[s - 1:e + 1]
else:
    w = idx(I, lambda l: l.startswith('    while (Serial.available())'), lp)
    e = idx(I, lambda l: l.startswith('}'), w)
    body = ['void', 'stm_bridge_poll (void)', '{', '    static char cmd_buffer[CMD_BUFFER_SIZE];', '    static int  cmd_len = 0;',
            '    static int  cmd_trunc = 0;'] + I[w:e + 1]
# Zweige reduzieren: nur var, LOG, RPCR; Praeprozessorzeilen im Zweigbereich weg
z0 = next(k for k, l in enumerate(body) if l.startswith('            if (! strncmp (cmd_buffer, "var ", 4))'))
z1 = next(k for k, l in enumerate(body) if l.startswith("            cmd_buffer[0] = '\\0';"))
keep, cur, out_l = None, [], []
def flush():
    if cur and keep: out_l.extend(cur)
for l in body[z0:z1]:
    if l.startswith('#'): continue
    m = re.match(r'            (else )?if \(', l)
    if m:
        flush(); cur = [l]
        keep = any(k in l for k in ('"var ", 4', '"LOG ", 4', '"RPCR ", 5'))
    else:
        cur.append(l)
flush()
kept = ''.join(out_l)
for k in ('"var ", 4', '"LOG ", 4'):
    if k not in kept: sys.exit('Zweig %s nicht gefunden' % k)
parts += body[:z0] + out_l + body[z1:]
# http.cpp
parts += ['#define HTTP_API_ERROR_ACTION_FAILED 7']
for n in ('HTTP_API_ERROR_REJECTED', 'HTTP_API_ERROR_NO_ANSWER'):
    l = next((l for l in H if l.startswith('#define ' + n + ' ')), None)
    if l: parts.append(l)
a = idx(H, lambda l: l.startswith('#define HTTP_RPC_WAIT_MSEC'))
if a is not None:
    b = idx(H, lambda l: l.startswith('/* Der ADC des STM ist 12 bit.'), a)
    parts += H[a:b]
else:
    parts.append(func(H, 'http_api_ldr_min_set'))
parts.append(func(H, 'http_api_ldr_max_set'))
import os
d = os.path.dirname(ino)
B = lines(os.path.join(d, 'base.cpp')); V = lines(os.path.join(d, 'vars.cpp'))
pre = [func(B, 'htoi'), func(V, 'get_numvar')]
parts.append(func(V, 'rpc'))
code = '\n'.join(pre + parts) + '\n'
code = re.sub(r'\b(\d+)UL\b', r'\1U', code)                 # UL ist auf dem ESP 32 Bit
code = re.sub(r'^(\s*)(static\s+)?unsigned long(\s+)(?=\w+\s*(=|;))', r'\1\2uint32_t\3', code, flags=re.M)
open(out, 'w').write(code)
print('extrahiert:', 'neu (stm_bridge_poll)' if neu else 'alt (Rumpf aus loop())', '-- Zweige:', ', '.join(k for k in ('var', 'LOG', 'RPCR') if '"%s ' % k in kept))
