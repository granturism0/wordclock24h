#!/usr/bin/env python3
"""c8: Ring aus ESP-uclock.ino (Konstanten, Ringspeicher, stm32_log_append/_get_count/_get_line), echter Code."""
import sys
ino, out = sys.argv[1], sys.argv[2]
L = open(ino, 'rb').read().decode('utf-8').replace('\r\n', '\n').split('\n')
def func(name):
    i = next(k for k, l in enumerate(L) if l.startswith(name + ' (') or l.startswith(name + '('))
    j = next(k for k in range(i, len(L)) if L[k].startswith('}'))
    return '\n'.join(L[i - 1:j + 1])
p = [l for l in L if l.startswith('#define CMD_BUFFER_SIZE') or l.startswith('#define STM32_LOG_') or l.startswith('#define LOG_TRUNC_MARK')]
p += [l for l in L if l.startswith('static char              stm32_log_lines') or l.startswith('static uint16_t          stm32_log_')]
p += [func(n) for n in ('stm32_log_append', 'stm32_log_clear', 'stm32_log_get_count', 'stm32_log_get_line')]
open(out, 'w').write('\n'.join(p) + '\n')
