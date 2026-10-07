/* Pruefstand A3 + A48 + A51 (S.19c, AKK.6). Echte Stellen aus main.c, overlay.c und base.c (extract.py);
 * nachgebildet sind nur EEPROM, Log und die Overlay-Setter (zaehlen ihre Aufrufe).
 * -DZIELTYPEN: uint_fast8_t/uint_fast16_t 32 Bit wie arm-none-eabi -- PFLICHT fuer die Aussage (L256). */
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
#include "overlay.h"

OVERLAY_GLOBALS     overlay;
uint_fast8_t        eep_is_up = 1;
static uint8_t      eeprom[4096];
#define EEPROM_VERSION_2_8              1
#define EEPROM_DATA_OFFSET_N_OVERLAYS   0
#define EEPROM_DATA_SIZE_N_OVERLAYS     1
#define EEPROM_DATA_OFFSET_OVERLAY      1
#define EEPROM_OVERLAY_ENTRY_SIZE       OVERLAY_ENTRY_SIZE
static uint_fast8_t eep_read (uint_fast16_t a, uint8_t * p, uint_fast16_t n)  { memcpy (p, eeprom + a, n); return 1; }
static uint_fast8_t eep_write (uint_fast16_t a, uint8_t * p, uint_fast16_t n) { memcpy (eeprom + a, p, n); return 1; }
static int log_printf (const char * f, ...) { (void) f; return 0; }
#define debug_log_printf(...)
static struct { uint_fast16_t year; } gmain;

static int setter_calls;
void overlay_set_type (uint_fast8_t i, uint_fast8_t v)       { (void) i; (void) v; setter_calls++; }
void overlay_set_interval (uint_fast8_t i, uint_fast8_t v)   { (void) i; (void) v; setter_calls++; }
void overlay_set_duration (uint_fast8_t i, uint_fast8_t v)   { (void) i; (void) v; setter_calls++; }
void overlay_set_date_code (uint_fast8_t i, uint_fast8_t v)  { (void) i; (void) v; setter_calls++; }
void overlay_set_date_start (uint_fast8_t i, uint_fast16_t v){ (void) i; (void) v; setter_calls++; }
void overlay_set_days (uint_fast8_t i, uint_fast8_t v)       { (void) i; (void) v; setter_calls++; }
void overlay_set_text (uint_fast8_t i, char * v)             { (void) i; (void) v; setter_calls++; }
void overlay_set_flags (uint_fast8_t i, uint_fast8_t v)      { (void) i; (void) v; setter_calls++; }
void overlay_calc_dates (uint_fast8_t i, uint_fast16_t y)    { (void) i; (void) y; }

#include SNIPPET

static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)

static void
n_fall (uint_fast16_t n, int angenommen)
{
    char name[80];
    uint32_t rej = esp8266_cmd_reject_cnt;
    overlay.n_overlays = 5; eeprom[0] = 5;
    a3_case (n);
    snprintf (name, sizeof (name), "A3  n = %5u %s", (unsigned) n, angenommen ? "angenommen" : "abgewiesen und gezaehlt");
    if (angenommen) CHECK (name, overlay.n_overlays == n && eeprom[0] == n && esp8266_cmd_reject_cnt == rej);
    else            CHECK (name, overlay.n_overlays == 5 && eeprom[0] == 5 && esp8266_cmd_reject_cnt == rej + 1);
}

static void
idx_fall (unsigned idx, int angenommen)
{
    char name[80], line[16];
    uint32_t rej = esp8266_cmd_reject_cnt;
    overlay.n_overlays = 255;                  /* die Setter-Pruefung idx < n soll NICHT tragen -- geprueft wird der Aufrufer */
    setter_calls = 0;
    snprintf (line, sizeof (line), "T%02x05", idx);
    schedule_esp8266_overlay (line);
    snprintf (name, sizeof (name), "A48 var_idx = %3u %s", idx, angenommen ? "angenommen" : "ganze Zeile verworfen und gezaehlt");
    if (angenommen) CHECK (name, setter_calls == 1 && esp8266_cmd_reject_cnt == rej);
    else            CHECK (name, setter_calls == 0 && esp8266_cmd_reject_cnt == rej + 1);
}

static void
lese_fall (uint8_t n, int ueberlebt)
{
    char name[80];
    memset (&overlay, 0, sizeof (overlay));
    eeprom[0] = n;
    overlay_read_config_from_eep (EEPROM_VERSION_2_8);
    snprintf (name, sizeof (name), "A51 n = %u im EEPROM %s", n, ueberlebt ? "ueberlebt das Lesen" : "wird verworfen (0)");
    CHECK (name, overlay.n_overlays == (ueberlebt ? n : 0));
}

int
main (void)
{
    n_fall (31, 1); n_fall (32, 1); n_fall (33, 0); n_fall (255, 0); n_fall (256, 0); n_fall (288, 0); n_fall (65535, 0);
    idx_fall (31, 1); idx_fall (32, 0); idx_fall (33, 0); idx_fall (255, 0);
    lese_fall (31, 1); lese_fall (32, 1); lese_fall (33, 0); lese_fall (255, 0);
    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
