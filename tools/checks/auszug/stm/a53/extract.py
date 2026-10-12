# Pruefstand A53 (Paket 2026-10-11, S3.6): schneidet die Spielschleifen und die Auswertung WOERTLICH aus.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis> [sabotage]
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
sab = sys.argv[3] if len(sys.argv) > 3 else ''
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
tet = lies('tetris/tetris.c'); sna = lies('tetris/snake.c'); mc = lies('main.c')
SAB = {
    'quittung':  ('main', '(1UL << ESP8266_OK) | ', ''),
    'ohnesync':  ('main', '        if (esp8266.is_online)\n        {\n            var_sync_pending = 1;\n        }\n', ''),
    # Review S3.13 M1: Vormerken ohne is_online-Bedingung (Stand vor M1)
    'ohneonline': ('main', '        if (esp8266.is_online)\n        {\n            var_sync_pending = 1;\n        }\n', '        var_sync_pending = 1;\n'),
    'keinreset': ('main', '        game_dropped_lines  = 0;\n', ''),
    'snakeohne': ('snake', '        game_count_dropped (msg_rtc);       // A53: nach dem break oben -- GS zaehlt nie\n', ''),
    'timeout':   ('main', '((1UL << ESP8266_TIMEOUT) | ', '((0UL) | '),
}
if sab:
    if sab not in SAB: sys.exit('extract: unbekannte Sabotage ' + sab)
    wo, a, n = SAB[sab]; ref = {'main': mc, 'snake': sna}[wo]
    if ref.count(a) != 1: sys.exit('extract: Sabotage %s -- Anker %d-mal' % (sab, ref.count(a)))
    if wo == 'main': mc = mc.replace(a, n)
    else: sna = sna.replace(a, n)
def zeile(t, a): return '#line %d "%s"\n' % (t[:a].count('\n') + 1, 'quelle')
# Tetris: die innere Pollschleife
a = tet.index('        for (i = 0; i < loops; i++)\n'); b = tet.index('\n        }\n', a) + len('\n        }\n')
tetris_loop = tet[a:b]
# Snake: get_next_move() ganz
a = sna.index('static DIRECTION\nget_next_move (DIRECTION previous)'); b = sna.index('\n}\n', a) + 3
snake_fn = sna[a:b]
# main.c: game_count_dropped (falls da) und schedule_esp8266_games
teile = []
if 'game_count_dropped (uint_fast8_t msg_rtc)\n{' in mc:
    a = mc.index('#define GAME_NOT_DROPPED'); b = mc.index('\n}\n', mc.index('game_count_dropped (uint_fast8_t msg_rtc)\n{')) + 3
    teile.append(mc[a:b]); have = '-DHAVE_A53'
else:
    teile.append('void game_count_dropped (uint_fast8_t m) { (void) m; }   /* Stand ohne A53 */\n'); have = ''
g = re.search(r'^static uint_fast16_t\s+game_dropped_lines[^\n]*\n', mc, flags=re.M)
kopf = g.group(0) if g else 'static uint_fast16_t game_dropped_lines = 0;   /* Stand ohne A53 */\n'
a = mc.index('static void\nschedule_esp8266_games (char * parameters)'); b = mc.index('\n}\n', a) + 3
teile.append(mc[a:b])
open(os.path.join(w, 'auszug.c'), 'w').write(kopf + 'static int tetris_poll (void)\n{\n    int ch = 0; int msg_rtc; uint32_t i;\n' + tetris_loop + '    return ch;\n}\n' + snake_fn + '\n'.join(teile))
open(os.path.join(w, 'flags'), 'w').write(have)
print('extract: Tetris-Schleife, get_next_move, schedule_esp8266_games%s%s' % (', game_count_dropped' if have else ' (ohne A53)', (', Sabotage ' + sab) if sab else ''))
