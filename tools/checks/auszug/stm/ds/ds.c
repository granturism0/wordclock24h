/* Pruefstand DS (A2, Paket 2026-10-10, AKT.1-AKT.4). Echte Stellen aus ds18xx.c/.h, tempsensor.c/.h,
 * onewire_get_rom_code () und den beiden Temperaturzweigen aus main.c (extract.py).
 * Nachgebildet: der Eindrahtbus auf Byte-Ebene mit EINEM Sensor (MATCH ROM mit falscher ID -> Sensor schweigt,
 * Leitung liest 0xFF, wie L18), Log, ESP-Versand. Die Referenz-CRC ist unabhaengig gebaut (Polynomdivision
 * MSB-zuerst ueber den Bitstrom in Drahtreihenfolge, Ergebnis gespiegelt) und gegen drei veroeffentlichte
 * Vektoren geprueft. -DZIELTYPEN: uint_fast8_t/uint_fast16_t 32 Bit wie arm-none-eabi. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "pruefstand.h"
#ifdef ZIELTYPEN
#define uint_fast8_t    uint32_t
#define uint_fast16_t   uint32_t
#define int_fast8_t     int32_t
#define int_fast16_t    int32_t
typedef char zp_fast_pruefung[(sizeof (uint_fast8_t) == 4 && sizeof (uint_fast16_t) == 4) ? 1 : -1];
#endif

#define ONEWIRE_READ_ROM_CMD   0x33
#define ONEWIRE_MATCH_ROM_CMD  0x55

/* ---------- Referenz-CRC, unabhaengig von der Firmware ---------- */
static uint8_t rev8 (uint8_t v) { uint8_t r = 0; for (int i = 0; i < 8; i++) if (v & (1 << i)) r |= 0x80 >> i; return r; }
static uint8_t ref_crc (const uint8_t * d, int n)
{
    uint8_t reg = 0;
    for (int i = 0; i < n; i++) for (int k = 0; k < 8; k++)          /* Drahtreihenfolge: LSB zuerst */
    {
        int bit = (d[i] >> k) & 1, top = (reg >> 7) & 1;
        reg = (uint8_t) (reg << 1);
        if (top ^ bit) reg ^= 0x31;                                   /* x^8 + x^5 + x^4 + 1 */
    }
    return rev8 (reg);
}

/* ---------- Sensormodell und Bus-Attrappe ---------- */
static struct
{
    int     present;
    uint8_t rom[8];
    uint8_t sp[9];
    int     rom_flip_bit, rom_faults;          /* gekipptes Bit beim Lesen der ROM-ID, fuer die naechsten n Lesungen (-1: immer) */
    int     sp_flip_bit, sp_mode, sp_faults;   /* sp_mode: 0 Bit kippen, 1 neun 0xFF, 2 neun 0x00 */
    int     st, matchpos, matched, wrpos;
    int     rom_reads, conversions, copies;
    int     resets, fail_reset_at;             /* fail_reset_at = n: genau das n-te Ruecksetzen findet keine Praesenz */
} sen;
enum { ST_IDLE, ST_READROM, ST_MATCH, ST_FUNC, ST_RDSP, ST_WRSP, ST_DEAD };

static void sp_crc (void) { sen.sp[8] = ref_crc (sen.sp, 8); }
static void sensor_neu (uint16_t raw12, uint8_t conf)
{
    static const uint8_t rom[8] = { 0x28, 0xFF, 0x4B, 0x1D, 0x61, 0x16, 0x04, 0 };
    memset (&sen, 0, sizeof sen);
    sen.present = 1; memcpy (sen.rom, rom, 7); sen.rom[7] = ref_crc (rom, 7);
    sen.sp[0] = raw12 & 0xFF; sen.sp[1] = raw12 >> 8; sen.sp[2] = 0x4B; sen.sp[3] = 0x46; sen.sp[4] = conf;
    sen.sp[5] = 0xFF; sen.sp[6] = 0x0C; sen.sp[7] = 0x10; sp_crc ();
    sen.rom_flip_bit = sen.sp_flip_bit = -1;
}
uint_fast8_t onewire_reset (void) { sen.st = ST_IDLE; sen.resets++; if (sen.fail_reset_at && sen.resets == sen.fail_reset_at) return 0; return sen.present ? 1 : 0; }
uint_fast8_t onewire_read_bit (void) { return 1; }
void onewire_init (void) { }
void onewire_write_byte (uint8_t v)
{
    switch (sen.st)
    {
        case ST_IDLE:  if (v == ONEWIRE_READ_ROM_CMD) sen.st = ST_READROM; else if (v == ONEWIRE_MATCH_ROM_CMD) { sen.st = ST_MATCH; sen.matchpos = 0; sen.matched = 1; } else sen.st = ST_DEAD; break;
        case ST_MATCH: if (v != sen.rom[sen.matchpos]) sen.matched = 0; if (++sen.matchpos == 8) sen.st = ST_FUNC; break;
        case ST_FUNC:
            if (! sen.matched) { sen.st = ST_DEAD; break; }
            if (v == 0xBE) sen.st = ST_RDSP;
            else if (v == 0x44) { sen.conversions++; sen.st = ST_DEAD; }
            else if (v == 0x4E) { sen.st = ST_WRSP; sen.wrpos = 0; }
            else if (v == 0x48) { sen.copies++; sen.st = ST_DEAD; }
            else sen.st = ST_DEAD;
            break;
        case ST_WRSP:  if (sen.wrpos < 3) { sen.sp[2 + sen.wrpos] = (sen.wrpos == 2) ? (v | 0x1F) : v; sen.wrpos++; sp_crc (); } break;   /* Konfig.: Bit 0-4 lesen immer 1 */
        default: break;
    }
}
void onewire_write (uint8_t * p, uint8_t n) { while (n--) onewire_write_byte (*p++); }
void onewire_read (uint8_t * p, uint8_t n)
{
    if (sen.st == ST_READROM && n == 8)
    {
        memcpy (p, sen.rom, 8); sen.rom_reads++;
        if (sen.rom_faults && sen.rom_flip_bit >= 0) { p[sen.rom_flip_bit / 8] ^= 1 << (sen.rom_flip_bit % 8); if (sen.rom_faults > 0) sen.rom_faults--; }
    }
    else if (sen.st == ST_RDSP && n == 9)
    {
        memcpy (p, sen.sp, 9);
        if (sen.sp_faults)
        {
            if (sen.sp_mode == 1) memset (p, 0xFF, 9);
            else if (sen.sp_mode == 2) memset (p, 0x00, 9);
            else if (sen.sp_flip_bit >= 0) p[sen.sp_flip_bit / 8] ^= 1 << (sen.sp_flip_bit % 8);
            if (sen.sp_faults > 0) sen.sp_faults--;
        }
    }
    else memset (p, 0xFF, n);                    /* niemand antwortet: Leitung bleibt oben */
    sen.st = ST_DEAD;
}

/* ---------- Log und ESP ---------- */
static char loglines[512][64]; static int nlog;
static void log_add (const char * s) { if (nlog < 512) { snprintf (loglines[nlog], 64, "%s", s); char * e = strpbrk (loglines[nlog], "\r\n"); if (e) *e = 0; nlog++; } }
static void log_printf (const char * f, ...) { char b[128]; va_list ap; va_start (ap, f); vsnprintf (b, sizeof b, f, ap); va_end (ap); log_add (b); }
#define log_message(s) log_add (s)
static void log_uart_puts (const char * s) { log_add (s); }
#define debug_log_printf(...)
static struct { uint_fast8_t is_online; } esp8266 = { 1 };
static int sent_up_n, sent_up_val[64], sent_idx_n, sent_idx_last;
static char sent_seq[256]; static int sent_seq_n;        /* 'I' = Index, 'U' = IS_UP, in Sendereihenfolge */

/* Variablen aus main.c, die die beiden Zweige brauchen */
static uint_fast8_t measure_temperature_flag, read_temperature_flag;
static uint_fast8_t ds18xx_temperature_index = 0xFF;
#define eep_is_up 1
void var_send_ds18xx_is_up (void);
void var_send_ds18xx_temp_index (void);

#include "x-ds.c"

/* main.c ruft diese aus vars.c */
void var_send_ds18xx_is_up (void) { if (sent_seq_n < 255) sent_seq[sent_seq_n++] = 'U'; if (sent_up_n < 64) sent_up_val[sent_up_n] = ds18xx.is_up; sent_up_n++; }
void var_send_ds18xx_temp_index (void) { if (sent_seq_n < 255) sent_seq[sent_seq_n++] = 'I'; sent_idx_n++; sent_idx_last = gtemp.index; }

#if HAVE_READ_ERRORS
#define READ_ERRORS() ((int) gtemp.read_errors)
#else
#define READ_ERRORS() (-1)
#endif

static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)
static int count (const char * s, int from) { int n = 0; for (int i = from; i < nlog; i++) if (! strcmp (loglines[i], s)) n++; return n; }
static int count_prefix (const char * s, int from) { int n = 0; for (int i = from; i < nlog; i++) if (! strncmp (loglines[i], s, strlen (s))) n++; return n; }
static void reset_all (void) { memset (sent_seq, 0, sizeof sent_seq); sent_seq_n = 0; nlog = 0; sent_up_n = sent_idx_n = 0; sent_idx_last = -1; esp8266.is_online = 1; ds18xx.is_up = 0; gtemp.index = 0xFF; gtemp.correction = 0;
#if HAVE_READ_ERRORS
    gtemp.read_errors = 0;
#endif
}
static void start (void) { temp_init (); if (ds18xx.is_up) temp_read_temp_index (); }   /* wie main.c beim Start */
static void minute (void) { measure_temperature_flag = 1; ds_main_step (); read_temperature_flag = 1; ds_main_step (); }

#define RAW_21_5    0x0158      /* 21.5 Grad, 12-Bit-Rohwert */
#define IDX_21_5    43
#define CONF_9BIT   0x1F
#define CONF_12BIT  0x7F

int
main (void)
{
    char name[160];

    /* ===== CRC gegen veroeffentlichte Vektoren ===== */
    static const uint8_t an27[8]  = { 0x02, 0x1C, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xA2 };       /* Maxim AN27, ROM-Beispiel, CRC A2 */
    static const uint8_t pwron[9] = { 0x50, 0x05, 0x4B, 0x46, 0x7F, 0xFF, 0x0C, 0x10, 0x1C }; /* DS18B20 Einschalt-Scratchpad 85 Grad, CRC 1C */
    static const uint8_t check[9] = { '1','2','3','4','5','6','7','8','9' };                 /* CRC-8/MAXIM-DOW, check = 0xA1 */
    CHECK ("Referenz-CRC: AN27-ROM ergibt A2", ref_crc (an27, 7) == 0xA2);
    CHECK ("Referenz-CRC: Einschalt-Scratchpad ergibt 1C", ref_crc (pwron, 8) == 0x1C);
    CHECK ("Referenz-CRC: \"123456789\" ergibt A1", ref_crc (check, 9) == 0xA1);
#if HAVE_CRC8
    CHECK ("ds18xx_crc8: AN27-ROM (Byte 0-6) ergibt A2", ds18xx_crc8 (an27, 7) == 0xA2);
    CHECK ("ds18xx_crc8: Einschalt-Scratchpad (Byte 0-7) ergibt 1C", ds18xx_crc8 (pwron, 8) == 0x1C);
    CHECK ("ds18xx_crc8: \"123456789\" ergibt A1", ds18xx_crc8 (check, 9) == 0xA1);
    {
        uint32_t s = 12345; int ok = 1;
        for (int n = 0; n < 2000; n++) { uint8_t b[9]; int len = 1 + n % 9; for (int i = 0; i < len; i++) { s = s * 1103515245u + 12345u; b[i] = s >> 16; } if (ds18xx_crc8 (b, len) != ref_crc (b, len)) ok = 0; }
        CHECK ("ds18xx_crc8 gleich Referenz auf 2000 Zufallsfolgen (1-9 Byte)", ok);
    }
#else
    CHECK ("ds18xx_crc8 vorhanden (AN27-ROM)", 0);
    CHECK ("ds18xx_crc8 vorhanden (Einschalt-Scratchpad)", 0);
    CHECK ("ds18xx_crc8 vorhanden (\"123456789\")", 0);
    CHECK ("ds18xx_crc8 vorhanden (Zufallsfolgen)", 0);
#endif

    /* ===== AKT.1 / AKT.3: Scratchpad-CRC, 255 bei jedem Fehler ===== */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start ();
    CHECK ("AKT.1 gueltiges Scratchpad: 21.5 Grad (Index 43)", ds18xx.is_up == 1 && temp_read_temp_index () == IDX_21_5 && gtemp.index == IDX_21_5);
    for (int bit = 0; bit < 72; bit++)
    {
        reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start (); temp_read_temp_index ();       /* gtemp.index = 43 */
        sen.sp_mode = 0; sen.sp_flip_bit = bit; sen.sp_faults = 1;
        uint_fast8_t r = temp_read_temp_index ();
        snprintf (name, sizeof name, "AKT.1 Scratchpad Bit %2d (Byte %d) gekippt -> ungueltig, Index 255 statt alter Wert", bit, bit / 8);
        CHECK (name, r == 0xFF && gtemp.index == 0xFF && READ_ERRORS () == 1);
    }
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start (); temp_read_temp_index ();
    sen.sp_mode = 1; sen.sp_faults = 1;
    CHECK ("AKT.1 neun 0xFF (getrennte Leitung) -> Index 255", temp_read_temp_index () == 0xFF && gtemp.index == 0xFF && READ_ERRORS () == 1);
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start (); temp_read_temp_index ();
    sen.present = 0;
    CHECK ("AKT.1 fehlende Praesenz -> Index 255 statt alter Wert", temp_read_temp_index () == 0xFF && gtemp.index == 0xFF && READ_ERRORS () == 1);
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start (); temp_read_temp_index ();
    sen.sp_mode = 2; sen.sp_faults = 1;
    {
        uint_fast8_t r = temp_read_temp_index ();
        printf ("  HINWEIS neun 0x00 bestehen die CRC: Index %u (Review S1.5, design.md 4.1) -- kein Fall\n", (unsigned) r);
    }
    reset_all (); sensor_neu (0xFFF8 /* -0.5 Grad */, CONF_9BIT); start ();
    CHECK ("AKT.3 negativer Messwert: Index 255, aber KEIN Lesefehler gezaehlt", temp_read_temp_index () == 0xFF && READ_ERRORS () == 0);
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start (); temp_read_temp_index ();
    sen.sp_mode = 0; sen.sp_flip_bit = 3; sen.sp_faults = 1; temp_read_temp_index (); temp_read_temp_index ();
    CHECK ("AKT.3 gueltiger Wert nach Fehler setzt den Zaehler zurueck", gtemp.index == IDX_21_5 && READ_ERRORS () == 0);

    /* ===== AKT.2: ROM-ID-CRC und Familiencode ===== */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); temp_init ();
    CHECK ("AKT.2 gueltige ROM-ID -> erkannt", ds18xx.is_up == 1);
    for (int bit = 0; bit < 64; bit++)
    {
        reset_all (); sensor_neu (RAW_21_5, CONF_9BIT);
        sen.rom_flip_bit = bit; sen.rom_faults = 1;
        temp_init ();
        snprintf (name, sizeof name, "AKT.2 ROM-ID Bit %2d (Byte %d) gekippt -> nicht erkannt", bit, bit / 8);
        CHECK (name, ds18xx.is_up == 0);
    }
    {
        static const uint8_t fam[] = { 0x01, 0x00, 0x26, 0x29 };
        for (unsigned i = 0; i < sizeof fam; i++)
        {
            reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); sen.rom[0] = fam[i]; sen.rom[7] = ref_crc (sen.rom, 7); temp_init ();
            snprintf (name, sizeof name, "AKT.2 Familiencode 0x%02X mit gueltiger CRC -> nicht erkannt", fam[i]);
            CHECK (name, ds18xx.is_up == 0);
        }
        static const uint8_t ok_fam[] = { 0x10, 0x22, 0x28 };
        for (unsigned i = 0; i < sizeof ok_fam; i++)
        {
            reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); sen.rom[0] = ok_fam[i]; sen.rom[7] = ref_crc (sen.rom, 7); temp_init ();
            snprintf (name, sizeof name, "AKT.2 Familiencode 0x%02X mit gueltiger CRC -> erkannt", ok_fam[i]);
            CHECK (name, ds18xx.is_up == 1);
        }
    }
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); memset (sen.rom, 0, 8); temp_init ();
    CHECK ("AKT.2 ROM-ID acht 0x00 (CRC passt, Familie 0) -> nicht erkannt", ds18xx.is_up == 0);
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); memset (sen.rom, 0xFF, 8); temp_init ();
    CHECK ("AKT.2 ROM-ID acht 0xFF -> nicht erkannt", ds18xx.is_up == 0);
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); sen.present = 0; ds18xx.is_up = 1; temp_init ();
    CHECK ("AKT.2 keine Praesenz -> nicht erkannt (auch wenn vorher erkannt)", ds18xx.is_up == 0);

    /* ===== Sensor-EEPROM: Aufloesung nur nach gueltigem Scratchpad schreiben ===== */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT);
    sen.sp_mode = 0; sen.sp_flip_bit = 37; sen.sp_faults = 1;           /* Byte 4 Bit 5: 9 Bit erscheint als 10 Bit */
    temp_init ();
    CHECK ("EEPROM: gestoertes Scratchpad bei der Erkennung -> kein COPY SCRATCHPAD", sen.copies == 0);
    reset_all (); sensor_neu (RAW_21_5, CONF_12BIT); temp_init ();
    CHECK ("EEPROM: abweichende Aufloesung (12 Bit) -> genau ein COPY SCRATCHPAD", sen.copies == 1 && sen.sp[4] == CONF_9BIT);
    temp_init (); temp_init ();
    CHECK ("EEPROM: weitere Erkennungen danach -> kein weiteres COPY SCRATCHPAD", sen.copies == 1);

    /* ===== AKT.4: Neuerkennung mit simulierter Zeit ===== */
    /* L18: falsch gelesene ROM-ID beim Start */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); sen.rom_flip_bit = 13; sen.rom_faults = 1;
    start ();
    CHECK ("AKT.4 L18: falsche ROM-ID beim Start -> nicht erkannt", ds18xx.is_up == 0);
    minute ();
    CHECK ("AKT.4 L18: im naechsten Minutenfenster erkannt", ds18xx.is_up == 1);
    CHECK ("AKT.4 L18: genau eine Zeile \"ds18xx erkannt\"", count ("ds18xx erkannt", 0) == 1 && count ("ds18xx verloren", 0) == 0);
    CHECK ("AKT.4 L18: IS_UP = 1 genau einmal an den ESP", sent_up_n == 1 && sent_up_val[0] == 1);
    CHECK ("AKT.4 L18: danach gueltiger Messwert gesendet", gtemp.index == IDX_21_5 && sent_idx_last == IDX_21_5 && count ("DS18xxx temperature: 21.5", 0) == 1);
    for (int i = 0; i < 10; i++) minute ();
    CHECK ("AKT.4 L18: zehn weitere Minuten ohne Wechselzeile und ohne IS_UP", count ("ds18xx erkannt", 0) == 1 && sent_up_n == 1 && sen.rom_reads == 2);

    /* Start ohne Sensor, Sensor ab Minute 5 */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); sen.present = 0;
    start ();
    {
        int erkannt_in = -1, temp_vorher = 0, conv_vorher = 0;
        for (int mi = 0; mi < 10; mi++)
        {
            if (mi == 5) sen.present = 1;
            int before = nlog; minute ();
            if (erkannt_in < 0 && count ("ds18xx erkannt", before)) erkannt_in = mi;
            if (mi < 5) { temp_vorher += count_prefix ("DS18xxx temperature", before); conv_vorher = sen.conversions; }
        }
        snprintf (name, sizeof name, "AKT.4 ohne Sensor, Sensor ab Minute 5 -> erkannt in Minute %d (5 oder 6)", erkannt_in);
        CHECK (name, erkannt_in == 5 || erkannt_in == 6);
        CHECK ("AKT.4 ohne Sensor: vorher keine Temperaturzeile, keine Wandlung, kein IS_UP", temp_vorher == 0 && conv_vorher == 0 && sent_up_n == 1);
        CHECK ("AKT.4 ohne Sensor: genau eine Zeile \"ds18xx erkannt\", keine \"verloren\"", count ("ds18xx erkannt", 0) == 1 && count ("ds18xx verloren", 0) == 0);
    }

    /* Ent-6 (a): drei ungueltige Messungen in Folge -> verloren, Neuerkennung */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start ();
    {
        int verloren_in = -1, erkannt_in = -1, idx255_gesendet = 0, conv_weg = 0, temp_weg = 0;
        for (int mi = 0; mi < 30; mi++)
        {
            if (mi == 10) sen.present = 0;
            if (mi == 20) sen.present = 1;
            int before = nlog, conv = sen.conversions; minute ();
            if (count ("ds18xx verloren", before)) verloren_in = mi;
            if (count ("ds18xx erkannt", before)) erkannt_in = mi;
            if (mi >= 10 && mi <= 12 && sent_idx_last == 0xFF && count ("DS18xxx temperature: ungueltig", before) == 1) idx255_gesendet++;
            if (mi >= 13 && mi <= 19) { conv_weg += sen.conversions - conv; temp_weg += count_prefix ("DS18xxx temperature", before); }
        }
        snprintf (name, sizeof name, "AKT.4 drei Fehler in Folge (Minute 10-12) -> verloren in Minute %d", verloren_in);
        CHECK (name, verloren_in == 12 && count ("ds18xx verloren", 0) == 1);
        CHECK ("AKT.3 Minute 10-12: je \"DS18xxx temperature: ungueltig\" und 255 an den ESP", idx255_gesendet == 3);
        CHECK ("AKT.4 Minute 13-19: keine Wandlung, keine Temperaturzeile", conv_weg == 0 && temp_weg == 0);
        snprintf (name, sizeof name, "AKT.4 Sensor ab Minute 20 zurueck -> erkannt in Minute %d", erkannt_in);
        CHECK (name, erkannt_in == 20 && count ("ds18xx erkannt", 0) == 1);
        CHECK ("AKT.4 IS_UP genau zweimal: 0 beim Verlust, 1 beim Wiederfinden", sent_up_n == 2 && sent_up_val[0] == 0 && sent_up_val[1] == 1);
        CHECK ("AKT.4 nach dem Wiederfinden gueltiger Wert", gtemp.index == IDX_21_5 && sent_idx_last == IDX_21_5);
    }

    /* zwei Fehler, ein gueltiger Wert, im Wechsel: nie verloren */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start ();
    for (int mi = 0; mi < 15; mi++) { if (mi % 3 != 2) { sen.sp_mode = 0; sen.sp_flip_bit = 9; sen.sp_faults = 1; } minute (); }
    CHECK ("AKT.4 zwei Fehler, dann gueltig, fuenfmal: nie verloren, kein IS_UP", count ("ds18xx verloren", 0) == 0 && sent_up_n == 0 && ds18xx.is_up == 1);
    CHECK ("AKT.4 dabei zehn Zeilen \"ungueltig\", fuenf gueltige", count ("DS18xxx temperature: ungueltig", 0) == 10 && count ("DS18xxx temperature: 21.5", 0) == 5);

    /* 60 Minuten gesund: keine Wechselzeile, kein IS_UP, keine Neuerkennung */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start ();
    for (int mi = 0; mi < 60; mi++) minute ();
    CHECK ("AKT.4 60 Minuten gesund: keine Wechselzeile, kein IS_UP, ROM-ID nur beim Start gelesen",
           count ("ds18xx erkannt", 0) == 0 && count ("ds18xx verloren", 0) == 0 && sent_up_n == 0 && sen.rom_reads == 1);
    CHECK ("AKT.4 60 Minuten gesund: 60 Zeilen \"DS18xxx temperature: 21.5\"", count ("DS18xxx temperature: 21.5", 0) == 60 && sent_idx_n == 60);

    /* ESP nicht online: Zeilen ja, Versand nein */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start (); esp8266.is_online = 0;
    for (int mi = 0; mi < 8; mi++) { if (mi == 2) sen.present = 0; if (mi == 6) sen.present = 1; minute (); }
    CHECK ("AKT.4 ESP offline: verloren und erkannt geloggt, nichts gesendet",
           count ("ds18xx verloren", 0) == 1 && count ("ds18xx erkannt", 0) == 1 && sent_up_n == 0 && sent_idx_n == 0);

    /* Wackelkontakt nur auf den Daten: ROM-ID gut, Scratchpad dauerhaft gestoert, Konfigurationsbit gekippt */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start ();
    sen.sp_mode = 0; sen.sp_flip_bit = 37; sen.sp_faults = -1;
    for (int mi = 0; mi < 30; mi++) minute ();
    snprintf (name, sizeof name, "Wackelkontakt 30 Minuten: kein COPY SCRATCHPAD (%d), verloren %d, erkannt %d (N2: Erkennung braucht gueltiges Scratchpad)",
              sen.copies, count ("ds18xx verloren", 0), count ("ds18xx erkannt", 0));
    CHECK (name, sen.copies == 0 && count ("ds18xx verloren", 0) == 1 && count ("ds18xx erkannt", 0) == 0 && ds18xx.is_up == 0);
    sen.sp_faults = 0; minute ();
    CHECK ("Wackelkontakt behoben -> in der naechsten Minute erkannt, gueltiger Wert", ds18xx.is_up == 1 && count ("ds18xx erkannt", 0) == 1 && gtemp.index == IDX_21_5);

    /* ===== N2 (Review S1.5): Scratchpad-CRC-Fehler bei der Erkennung ===== */
    reset_all (); sensor_neu (RAW_21_5, CONF_12BIT);
    sen.sp_mode = 0; sen.sp_flip_bit = 3; sen.sp_faults = 1;           /* genau das Lesen in ds18xx_init () gestoert */
    start ();
    CHECK ("N2 Sensor auf 12 Bit, Scratchpad-CRC-Fehler bei der Erkennung -> nicht erkannt, nichts geschrieben", ds18xx.is_up == 0 && sen.copies == 0);
    minute ();
    CHECK ("N2 naechste Minute erkannt, Aufloesung auf 9 Bit gesetzt (ein COPY SCRATCHPAD)", ds18xx.is_up == 1 && sen.copies == 1 && sen.sp[4] == CONF_9BIT);
    snprintf (name, sizeof name, "N2 danach 21.5 Grad (Index %u), nicht auf 125 Grad geklammert", (unsigned) gtemp.index);
    CHECK (name, gtemp.index == IDX_21_5 && count ("DS18xxx temperature: 21.5", 0) == 1 && count ("DS18xxx temperature: 125", 0) == 0);
    for (int i = 0; i < 5; i++) minute ();
    CHECK ("N2 fuenf weitere Minuten: 21.5 Grad, kein weiteres COPY SCRATCHPAD", gtemp.index == IDX_21_5 && sen.copies == 1 && count ("DS18xxx temperature: 21.5", 0) == 6);
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); sen.rom[0] = 0x10; sen.rom[7] = ref_crc (sen.rom, 7);
    sen.sp_mode = 0; sen.sp_flip_bit = 3; sen.sp_faults = 1;
    start ();
    CHECK ("N2 DS18S20 (0x10), Scratchpad-CRC-Fehler bei der Erkennung -> nicht erkannt", ds18xx.is_up == 0);
    minute ();
    CHECK ("N2 DS18S20 naechste Minute erkannt, kein COPY SCRATCHPAD", ds18xx.is_up == 1 && sen.copies == 0 && count ("ds18xx erkannt", 0) == 1);
    /* Ruecksetzen in ds18xx_init (): 1 Start, 2+3 ROM-ID, 4+5 Scratchpad, 6 ds1822B20_set_resolution () */
    reset_all (); sensor_neu (RAW_21_5, CONF_12BIT); sen.fail_reset_at = 6;
    temp_init ();
    CHECK ("N2 keine Praesenz beim Setzen der Aufloesung (12 Bit bleibt) -> nicht erkannt", ds18xx.is_up == 0 && sen.sp[4] == CONF_12BIT);
    sen.fail_reset_at = 0; minute ();
    CHECK ("N2 danach naechste Minute erkannt, 9 Bit, 21.5 Grad", ds18xx.is_up == 1 && sen.sp[4] == CONF_9BIT && gtemp.index == IDX_21_5);

    /* ===== Sendestelle IS_UP (Sparvorschlag 3): Reihenfolge ===== */
    reset_all (); sensor_neu (RAW_21_5, CONF_9BIT); start ();
    for (int mi = 0; mi < 6; mi++) { if (mi == 1) sen.present = 0; if (mi == 5) sen.present = 1; minute (); }
    snprintf (name, sizeof name, "IS_UP: Verlust erst Index 255, dann IS_UP 0; Wiederfinden IS_UP 1 vor dem Index (Folge %s)", sent_seq);
    CHECK (name, ! strcmp (sent_seq, "IIIIUUI") && sent_up_n == 2 && sent_up_val[0] == 0 && sent_up_val[1] == 1);

    printf ("DS: %d Faelle, %d FEHL\n", cases, fails);
    return fails ? 1 : 0;
}
