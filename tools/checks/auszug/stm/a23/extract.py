# Pruefstand A23 (Paket 2026-10-11, S3.4): schneidet die Farbfunktionen WOERTLICH aus display.c.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis> [sabotage]
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
sab = sys.argv[3] if len(sys.argv) > 3 else ''
t = open(os.path.join(src, 'display/display.c'), 'rb').read().decode('latin-1').replace('\r\n', '\n')
SAB = {
    'ohnesendung': ('        display_calc_dimmed_display_colors ();\n        var_send_display_colors ();\n    }\n    else\n', '        display_calc_dimmed_display_colors ();\n    }\n    else\n'),
    'keindc':      ('    COPY_DSP_RGB (display.saved_display_colors, display.display_colors);    // A23: auch waehrend einer Animation\n', ''),
    'keinir':      ('    if (display.color_animation_mode == COLOR_ANIMATION_MODE_NONE)\n    {\n        COPY_DSP_RGB (display.saved_display_colors, display.display_colors);\n    }\n', ''),
    'inkrement':   ('    display_change_color_animation ();\n\n    if (do_sync)', '    display_init_color_animation ();\n\n    if (do_sync)'),
    'eeprom':      ('display_rgb_color_buf8[0] = display.saved_display_colors.red;', 'display_rgb_color_buf8[0] = display.display_colors.red;'),
}
if sab:
    if sab not in SAB or SAB[sab][0] not in t: sys.exit('extract: Sabotage %s -- Anker fehlt' % sab)
    t = t.replace(SAB[sab][0], SAB[sab][1], 1)        # bei 'inkrement' nur der ERSTE Treffer: das Inkrement
NAMEN = ['display_init_color_animation_rainbow', 'display_init_color_animation_daylight', 'display_init_color_animation',
         'display_change_color_animation', 'display_save_display_colors', 'display_set_color_animation_mode',
         'display_increment_color_animation_mode', 'display_decrement_color_animation_mode', 'display_color_step_done',
         'display_increment_display_color_red', 'display_decrement_display_color_red', 'display_increment_display_color_green',
         'display_decrement_display_color_green', 'display_increment_display_color_blue', 'display_decrement_display_color_blue',
         'display_increment_display_color_white', 'display_decrement_display_color_white', 'display_set_display_colors']
teile = []; flags = []
for n in NAMEN:
    m = re.search(r'\n((?:static\s*\n)?(?:static\s+)?(?:void|uint_fast8_t)\n' + n + r' \()', t)
    if not m:
        teile.append('/* %s fehlt in diesem Stand */\n' % n); continue
    a = m.start() + 1; b = t.index('\n}\n', a) + 3
    teile.append('#line %d "display.c"\n' % (t[:a].count('\n') + 1) + t[a:b])
    flags.append('-DHAVE_' + n.upper())
open(os.path.join(w, 'auszug.c'), 'w').write('\n'.join(teile))
# statisch: Laden setzt die Sicherung VOR display_init_color_animation()
r = t.index('display_read_config_from_eep (uint32_t eep_version)\n{'); re_ = t.index('\n}\n', r)
lade = t[r:re_]; i = lade.find('COPY_DSP_RGB (display.saved_display_colors, display.display_colors)'); j = lade.find('display_init_color_animation ();')
flags.append('-DLADEN_OK=%d' % (1 if (i >= 0 and j >= 0 and i < j) else 0))
ini = t.index('\ndisplay_init (void)\n{'); ine = t.index('\n}\n', ini)
flags.append('-DVORGABE_OK=%d' % (1 if 'COPY_DSP_RGB (display.saved_display_colors, display.display_colors)' in t[ini:ine] else 0))
open(os.path.join(w, 'flags'), 'w').write(' '.join(flags))
print('extract: %d von %d Funktionen%s' % (sum(1 for f in flags if f.startswith('-DHAVE_')), len(NAMEN), (', Sabotage ' + sab) if sab else ''))
