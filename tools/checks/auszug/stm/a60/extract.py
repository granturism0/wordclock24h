# Pruefstand A60 (Paket 2026-10-11, S2.1): schneidet die I2C-Ablauflogik WOERTLICH aus i2c.c.
# Aufruf: extract.py <src-Verzeichnis> <Arbeitsverzeichnis> [sabotage]
import sys, os, re
src, w = sys.argv[1], sys.argv[2]
sab = sys.argv[3] if len(sys.argv) > 3 else ''
def lies(p): return open(os.path.join(src, p), 'rb').read().decode('latin-1').replace('\r\n', '\n')
c = lies('i2c/i2c.c'); h = lies('i2c/i2c.h'); ee = lies('eeprom/eeprom.c')
SAB = {
    'fristnie':  [('    delay_counter = I2C_TIMEOUT_TICKS;\n', '')] * 2,                 # Frist nie gesetzt (beide Stellen)
    'busyohne':  [('    while (I2C_GetFlagStatus (I2C_CHANNEL, I2C_FLAG_BUSY))\n    {\n        if (delay_counter == 0)', '    while (I2C_GetFlagStatus (I2C_CHANNEL, I2C_FLAG_BUSY))\n    {\n        if (0)')],
    'ohnelog':   [('    log_puts ("i2c: frist abgelaufen\\r\\n");\n', '')],
    'ohnefrei':  [('        i2c_last_clockspeed = 0;\n        i2c_init (i2c_clockspeed);\n', '')],
    # heutiger Stand vor der Nachpruefung: Freitakten nur im BUSY-Pfad -- die Bedingung im gemeinsamen Weg
    # wird auf "nie" gesetzt und in die BUSY-Warte verlegt
    'nurbusy':   [('    if (! nack && (I2C_GetFlagStatus (I2C_CHANNEL, I2C_FLAG_BUSY) || GPIO_ReadInputDataBit (SDA_PORT, SDA_PIN) == Bit_RESET))\n', '    if (0)\n'),
                  ('            i2c_handle_timeout ();                                          // Zeile und Freitakten wie bei jeder Frist\n',
                   '            i2c_handle_timeout ();\n            i2c_last_clockspeed = 0;\n            i2c_init (i2c_clockspeed);\n')],
    'afignoriert': [('    uint_fast8_t    nack = I2C_GetFlagStatus (I2C_CHANNEL, I2C_FLAG_AF);\n', '    uint_fast8_t    nack = 0;\n')],
    # Review S3.13 M2 (A73): Flag in eeprom_init nie gesetzt bzw. nie zurueckgesetzt
    'flagnie':    [('    i2c_nack_quiet = 1;', '    i2c_nack_quiet = 0;')],
    'flagbleibt': [('    i2c_nack_quiet = 0;', '    i2c_nack_quiet = 1;')],
    'msec':      [('    while (! I2C_GetFlagStatus (I2C_CHANNEL, flag1) || (flag2 && ! I2C_GetFlagStatus (I2C_CHANNEL, flag2)))\n    {\n',
                   '    while (! I2C_GetFlagStatus (I2C_CHANNEL, flag1) || (flag2 && ! I2C_GetFlagStatus (I2C_CHANNEL, flag2)))\n    {\n        delay_msec (1); delay_counter = delay_counter ? delay_counter : 0;\n')],
}
if sab in ('flagnie', 'flagbleibt'):
    alt, neu = SAB[sab][0]
    if ee.count(alt) != 1: sys.exit('extract: Sabotage %s -- Anker %d-mal in eeprom.c' % (sab, ee.count(alt)))
    ee = ee.replace(alt, neu); sab_ee = sab; sab = ''
if sab:
    if sab not in SAB: sys.exit('extract: unbekannte Sabotage ' + sab)
    for alt, neu in SAB[sab]:
        if alt not in c: sys.exit('extract: Sabotage %s -- Anker fehlt' % sab)
        if c.count(alt) != 1 and sab != 'fristnie': sys.exit('extract: Sabotage %s -- Anker %d-mal' % (sab, c.count(alt)))
        c = c.replace(alt, neu, 1)
def fn(name, pflicht=True):
    m = re.search(r'\n(static [a-z_0-9]+|int_fast16_t|void|static void)\n' + name + r' \(', c)
    if not m:
        if pflicht: sys.exit('extract: %s fehlt' % name)
        return ''
    a = m.start() + 1; b = c.index('\n}\n', a) + 3
    return '#line %d "i2c.c"\n' % (c[:a].count('\n') + 1) + c[a:b]
defs = ''.join(l + '\n' for l in c.split('\n') if l.startswith('#define I2C_TIMEOUT'))
defs += ''.join(l + '\n' for l in h.split('\n') if re.match(r'#define I2C_(OK|ERROR_)', l))
glob = ''.join(l + '\n' for l in c.split('\n') if re.match(r'(static uint32_t\s+i2c_(last_)?clockspeed|uint_fast8_t\s+i2c_nack_quiet);', l))
if 'i2c_nack_quiet;' not in glob: glob += 'static uint_fast8_t i2c_nack_quiet;   /* Stand ohne A73 */\n'
# Adresssuche aus eeprom_init() woertlich: alles zwischen i2c_init (clockspeed); und #if SHOW_SIZES
ea = ee.index('    i2c_init (clockspeed);\n', ee.index('eeprom_init (uint32_t clockspeed)')) + len('    i2c_init (clockspeed);\n')
eb = ee.index('#if SHOW_SIZES', ea)
suche = 'static void eep_suche (void)\n{\n    uint_fast8_t i; uint8_t value;\n#line %d "eeprom.c"\n' % (ee[:ea].count('\n') + 1) + ee[ea:eb] + '}\n'
teile = [defs + glob + 'void i2c_init (uint32_t);\n', fn('i2c_handle_timeout'), fn('i2c_wait_for_flags'), fn('i2c_wait_not_busy', False),
         fn('i2c_send_address'), fn('i2c_read'), fn('i2c_write'), fn('i2c_reset_bus'), fn('i2c_init'), suche]
open(os.path.join(w, 'auszug.c'), 'w').write(''.join(teile))
open(os.path.join(w, 'flags'), 'w').write('-DHAVE_NOT_BUSY' if teile[3] else '')
