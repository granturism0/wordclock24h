/* Pruefstand A9 (S.19i): ldr_poll_brightness() aus src/ldr/ldr.c. (1) ldr.ldr_raw_value -- der Wert,
 * den var_send_ldr_raw_value() sendet -- ist ungeklammert. (2) Die Kennlinie der Automatik
 * (ldr.ldr_value) ist ueber eine lange Messfolge Schritt fuer Schritt gleich: run.sh vergleicht die
 * Spurpruefsumme zwischen alter und neuer Fassung. -DZIELTYPEN. */
#include <stdio.h>
#include <stdint.h>
#include "pruefstand.h"
#ifdef ZIELTYPEN
#define uint_fast8_t uint32_t
#define uint_fast16_t uint32_t
#endif
#include "ldr.h"
static void adc_start_single_conversion (void) {}
static uint16_t next_raw;
static uint_fast8_t adc_poll_conversion_value (uint16_t * v) { *v = next_raw; return 1; }
#include SNIPPET
static int fails, cases;
#define CHECK(name, cond) do { cases++; if (cond) printf ("  OK    %s\n", name); else { fails++; printf ("  FEHL  %s\n", name); } } while (0)
int
main (void)
{
    uint32_t spur = 0, x = 12345;
    int i;
    ldr.ldr_min_value = 1000; ldr.ldr_max_value = 3000;
    next_raw = 500;  ldr_poll_brightness ();
    CHECK ("Rohwert 500 unter min 1000: gesendet wird 500", ldr.ldr_raw_value == 500);
    next_raw = 3500; ldr_poll_brightness ();
    CHECK ("Rohwert 3500 ueber max 3000: gesendet wird 3500", ldr.ldr_raw_value == 3500);
    next_raw = 2000; ldr_poll_brightness ();
    CHECK ("Rohwert 2000 im Bereich: unveraendert", ldr.ldr_raw_value == 2000);
    ldr.ldr_value = MAX_LDR_BRIGHTNESS;
    for (i = 0; i < 20000; i++)
    {
        x = x * 1103515245u + 12345u; next_raw = (uint16_t) ((x >> 8) % 4096);
        if (i == 10000) { ldr.ldr_min_value = 3000; ldr.ldr_max_value = 3000; }   /* entartete Kalibrierung */
        if (i == 12000) { ldr.ldr_min_value = 0;    ldr.ldr_max_value = 4095; }
        ldr_poll_brightness ();
        spur = spur * 31 + (uint32_t) ldr.ldr_value;
    }
    printf ("  Spurpruefsumme Automatik: %08x\n", (unsigned) spur);
    printf ("\n%d Faelle, %d fehlgeschlagen\n", cases, fails);
    return fails ? 1 : 0;
}
