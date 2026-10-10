/* Pruefstand L59 (A59, Paket 2026-10-10, AKL.1/AKL.2). Echte Stellen aus main.c, ldr.c, ldr.h
 * (extract.py); nachgebildet sind nur EEPROM (mit Seitenvergleich wie C3: ein Schreibzyklus nur bei
 * geaenderten Bytes), ADC und Log. -DZIELTYPEN: uint_fast8_t/uint_fast16_t 32 Bit wie arm-none-eabi. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pruefstand.h"
#ifdef ZIELTYPEN
#define uint_fast8_t    uint32_t
#define uint_fast16_t   uint32_t
typedef char zp_fast_pruefung[(sizeof (uint_fast8_t) == 4 && sizeof (uint_fast16_t) == 4) ? 1 : -1];
#endif

typedef struct { uint8_t ldr_value; uint16_t ldr_raw_value; uint16_t ldr_min_value; uint16_t ldr_max_value; } LDR_GLOBALS;
enum { LDR_RAW_VALUE_NUM_VAR = 16, LDR_MIN_VALUE_NUM_VAR, LDR_MAX_VALUE_NUM_VAR };
#define EEPROM_VERSION_2_1                  0x00020100
#define EEPROM_DATA_OFFSET_LDR_MIN_VALUE    225
#define EEPROM_DATA_OFFSET_LDR_MAX_VALUE    227
#define EEPROM_DATA_SIZE_LDR_MIN_VALUE      sizeof (uint16_t)
#define EEPROM_DATA_SIZE_LDR_MAX_VALUE      sizeof (uint16_t)
uint_fast8_t        eep_is_up = 1;
static uint8_t      eeprom[512];
static int          eep_calls, eep_cycles;
static uint_fast8_t eep_read (uint_fast16_t a, uint8_t * p, uint_fast16_t n) { memcpy (p, eeprom + a, n); return 1; }
static uint_fast8_t eep_write (uint_fast16_t a, uint8_t * p, uint_fast16_t n)
{ eep_calls++; if (memcmp (eeprom + a, p, n)) { eep_cycles++; memcpy (eeprom + a, p, n); } return 1; }
static uint16_t     adc_wert;
static uint_fast8_t adc_poll_conversion_value (uint16_t * p) { *p = adc_wert; return 1; }
#define debug_log_printf(...)
uint_fast8_t ldr_write_config_to_eep (void);

#include SNIPPET

static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)
static uint16_t ee16 (int a) { uint16_t v; memcpy (&v, eeprom + a, 2); return v; }
static void ausgang (uint16_t mn, uint16_t mx)
{ ldr.ldr_min_value = mn; ldr.ldr_max_value = mx; memcpy (eeprom + 225, &mn, 2); memcpy (eeprom + 227, &mx, 2); eep_calls = eep_cycles = 0; }

static void
setter (int is_min, uint32_t val, int angewandt)
{
    char name[120]; uint16_t vorher_mn = 5, vorher_mx = 26;   /* Ausgangslage wie am Geraet (L351) */
    ausgang (vorher_mn, vorher_mx);
    l59_case (is_min ? LDR_MIN_VALUE_NUM_VAR : LDR_MAX_VALUE_NUM_VAR, val);
    uint16_t ram = is_min ? ldr.ldr_min_value : ldr.ldr_max_value, ee = ee16 (is_min ? 225 : 227);
    uint16_t ram_o = is_min ? ldr.ldr_max_value : ldr.ldr_min_value, ee_o = ee16 (is_min ? 227 : 225);
    snprintf (name, sizeof name, "AKL.1 %s = %5u %s", is_min ? "min" : "max", (unsigned) val, angewandt ? "uebernommen, RAM und EEPROM" : "verworfen, nichts geschrieben");
    if (angewandt) CHECK (name, ram == val && ee == val && eep_cycles >= 1 && ram_o == (is_min ? vorher_mx : vorher_mn) && ee_o == ram_o);
    else           CHECK (name, ram == (is_min ? vorher_mn : vorher_mx) && ee == ram && eep_calls == 0 && eep_cycles == 0);
}

int
main (void)
{
    char name[120];
    static const uint32_t gut[] = { 0, 1, 2, 100, 2048, 4094, 4095 };
    static const uint32_t schlecht[] = { 4096, 4097, 8191, 32768, 65534, 65535 };
    for (int s = 0; s < 2; s++)
    {
        for (unsigned i = 0; i < sizeof gut / sizeof gut[0]; i++)          setter (s == 0, gut[i], 1);
        for (unsigned i = 0; i < sizeof schlecht / sizeof schlecht[0]; i++) setter (s == 0, schlecht[i], 0);
    }
    /* unveraenderter Wert: Aufruf erlaubt, aber kein Schreibzyklus */
    ausgang (5, 26); l59_case (LDR_MIN_VALUE_NUM_VAR, 5);
    CHECK ("AKL.1 min unveraendert = 5: kein Schreibzyklus", ldr.ldr_min_value == 5 && eep_cycles == 0);
    ausgang (5, 26); l59_case (LDR_MAX_VALUE_NUM_VAR, 26);
    CHECK ("AKL.1 max unveraendert = 26: kein Schreibzyklus", ldr.ldr_max_value == 26 && eep_cycles == 0);
    /* geaenderter Wert: genau EIN Schreibzyklus (nur der geaenderte Wert) */
    ausgang (5, 26); l59_case (LDR_MIN_VALUE_NUM_VAR, 0);
    CHECK ("AKL.1 min 5 -> 0: genau ein Schreibzyklus", eep_cycles == 1);
    ausgang (5, 26); l59_case (LDR_MAX_VALUE_NUM_VAR, 4095);
    CHECK ("AKL.1 max 26 -> 4095: genau ein Schreibzyklus", eep_cycles == 1);
    CHECK ("AKL.1 kein zweiter Schreibweg in den Zweigen (statisch)", L59_FREMDE_SCHREIBWEGE == 0);
    /* anderer Index bleibt unberuehrt (der Zweig darf nicht durchfallen) */
    ausgang (5, 26); l59_case (LDR_RAW_VALUE_NUM_VAR, 1000);
    CHECK ("LDR_RAW_VALUE bleibt readonly", ldr.ldr_min_value == 5 && ldr.ldr_max_value == 26 && eep_calls == 0);

    /* AKL.2: Reihenfolge nicht gesperrt (Backup-Import: min, dann max) */
    ausgang (0, 100); l59_case (LDR_MIN_VALUE_NUM_VAR, 3000);
    CHECK ("AKL.2 min 3000 bei max 100 angenommen", ldr.ldr_min_value == 3000 && ee16 (225) == 3000 && ldr.ldr_max_value == 100);
    adc_wert = 2000; ldr.ldr_value = 10; ldr_poll_brightness ();
    snprintf (name, sizeof name, "AKL.2 Laufzeit bei min >= max unveraendert: volle Helligkeit (ldr_value = %u)", (unsigned) ldr.ldr_value);
    CHECK (name, ldr.ldr_value == 31);
    l59_case (LDR_MAX_VALUE_NUM_VAR, 4000);
    CHECK ("AKL.2 danach max 4000: beide Werte stehen (RAM)", ldr.ldr_min_value == 3000 && ldr.ldr_max_value == 4000);
    CHECK ("AKL.2 danach max 4000: beide Werte stehen (EEPROM)", ee16 (225) == 3000 && ee16 (227) == 4000);
    /* Neustart: Werte kommen aus dem EEPROM zurueck */
    ldr.ldr_min_value = ldr.ldr_max_value = 0; ldr_read_config_from_eep (EEPROM_VERSION_2_1);
    CHECK ("Neustart: 3000/4000 aus dem EEPROM gelesen", ldr.ldr_min_value == 3000 && ldr.ldr_max_value == 4000);
    /* Start heilt min >= max (unveraendertes Verhalten, ldr_read_config_from_eep) */
    ausgang (0, 100); l59_case (LDR_MIN_VALUE_NUM_VAR, 3000);
    ldr.ldr_min_value = ldr.ldr_max_value = 7; ldr_read_config_from_eep (EEPROM_VERSION_2_1);
    CHECK ("AKL.2 Start heilt min 3000 >= max 100 auf 0/4095", ldr.ldr_min_value == 0 && ldr.ldr_max_value == 4095 && ee16 (225) == 0 && ee16 (227) == 4095);
    /* Bereinigung G1 (Ent-3): 0/4095 in beiden Reihenfolgen */
    ausgang (5, 26); l59_case (LDR_MIN_VALUE_NUM_VAR, 0); l59_case (LDR_MAX_VALUE_NUM_VAR, 4095);
    CHECK ("G1-Bereinigung min 0, dann max 4095", ee16 (225) == 0 && ee16 (227) == 4095);
    ausgang (5, 26); l59_case (LDR_MAX_VALUE_NUM_VAR, 4095); l59_case (LDR_MIN_VALUE_NUM_VAR, 0);
    CHECK ("G1-Bereinigung max 4095, dann min 0", ee16 (225) == 0 && ee16 (227) == 4095);

    printf ("L59: %d Faelle, %d FEHL\n", cases, fails);
    return fails ? 1 : 0;
}
