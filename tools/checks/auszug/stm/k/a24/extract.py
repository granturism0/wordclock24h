import sys
t = open(sys.argv[1], 'rb').read().decode('latin-1').replace('\r\n', '\n')
def func(head):
    a = t.index(head); b = t.index('\n}\n', a) + 3
    return '#line %d "tables.c"\n' % (t[:a].count('\n') + 1) + t[a:b]
open(sys.argv[2], 'w').write(func('static __attribute__((noinline)) uint_fast8_t\ntables_idx_ok (') + '\n' +
                             func('uint_fast8_t\ntables_fill_words ('))
