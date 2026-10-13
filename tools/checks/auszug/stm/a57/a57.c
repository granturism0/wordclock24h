/* Pruefstand A57 (Paket 2026-10-11, S3.8): Einmessen weist eine Inversion ab, Zahlen-Setter nicht.
 * Attrappen: ADC (Messwert oder Ausfall), EEPROM (zaehlt Schreibvorgaenge, merkt Inhalt), Log, Rueckmeldung. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#define LDR_MAX_RAW_VALUE               4095
#define EEPROM_DATA_OFFSET_LDR_MIN_VALUE 10
#define EEPROM_DATA_SIZE_LDR_MIN_VALUE  2
#define EEPROM_DATA_OFFSET_LDR_MAX_VALUE 12
#define EEPROM_DATA_SIZE_LDR_MAX_VALUE  2
enum { LDR_MIN_VALUE_RPC_VAR = 1, LDR_MAX_VALUE_RPC_VAR, LEARN_IR_RPC_VAR };
enum { LDR_MIN_VALUE_NUM_VAR = 17, LDR_MAX_VALUE_NUM_VAR = 18, TIMEZONE_NUM_VAR };
static struct { uint16_t ldr_min_value, ldr_max_value; } ldr;
static uint_fast8_t eep_is_up = 1;
static int n_eep; static uint16_t eep_min, eep_max;
static uint_fast8_t eep_write (int off, uint8_t * p, int n)
{ uint16_t v; memcpy (&v, p, 2); (void) n; if (off == EEPROM_DATA_OFFSET_LDR_MIN_VALUE) eep_min = v; else eep_max = v; n_eep++; return 1; }
static uint16_t adc_wert; static int adc_ausfall;
static void adc_start_single_conversion (void) { }
static uint_fast8_t adc_poll_conversion_value (uint16_t * v) { if (adc_ausfall) return 0; *v = adc_wert; return 1; }
static void delay_msec (uint32_t ms) { (void) ms; }
static char logbuf[256]; static int logzeilen;
static void log_printf (const char * f, ...) { va_list ap; va_start (ap, f); vsnprintf (logbuf, sizeof logbuf, f, ap); va_end (ap); logzeilen++; }
#define debug_log_message(s)
#define debug_log_printf(...)
static int n_send; static uint16_t gesendet;
static void var_send_ldr_min_value (void) { n_send++; gesendet = ldr.ldr_min_value; }
static void var_send_ldr_max_value (void) { n_send++; gesendet = ldr.ldr_max_value; }
static void esp8266_send_cmd (const char * c, const char * a, uint_fast8_t l) { (void) c; (void) a; (void) l; }   /* S2.1/B55: RPCR-Rueckmeldung */
#include "auszug.c"

static unsigned long faelle, fehl; static char was[240];
static void pruef (int ok, const char * w) { faelle++; if (! ok) { fehl++; if (fehl <= 8) printf ("  FEHL  %s\n", w); } }
static void null (uint16_t mi, uint16_t ma) { ldr.ldr_min_value = mi; ldr.ldr_max_value = ma; n_eep = 0; logzeilen = 0; logbuf[0] = 0; n_send = 0; gesendet = 0xEEEE; adc_ausfall = 0; }

int main (void)
{
    static const uint16_t W[] = { 0, 1, 2, 99, 100, 101, 2000, 4093, 4094, 4095 };
    int n = (int) (sizeof W / sizeof W[0]), i, j, k, seite;
    char soll[96];
    for (i = 0; i < n; i++) for (j = 0; j < n; j++) for (k = 0; k < n; k++) for (seite = 0; seite < 2; seite++)
    {
        uint16_t mi = W[i], ma = W[j], adc = W[k];
        int nimmt = seite == 0 ? (adc < ma) : (adc > mi);
        null (mi, ma); adc_wert = adc;
        rpc (seite == 0 ? LDR_MIN_VALUE_RPC_VAR : LDR_MAX_VALUE_RPC_VAR);
        uint16_t jetzt = seite == 0 ? ldr.ldr_min_value : ldr.ldr_max_value;
        uint16_t andere = seite == 0 ? ldr.ldr_max_value : ldr.ldr_min_value;
        if (seite == 0) snprintf (soll, sizeof soll, "ldr: minimum abgewiesen (%u >= max %u)\r\n", (unsigned) adc, (unsigned) ma);
        else            snprintf (soll, sizeof soll, "ldr: maximum abgewiesen (%u <= min %u)\r\n", (unsigned) adc, (unsigned) mi);
        snprintf (was, sizeof was, "%s uebernehmen: min %u max %u Messwert %u -> Wert %u, EEPROM %d, Log %d \"%.50s\", gesendet %u",
                  seite ? "Maximum" : "Minimum", mi, ma, adc, jetzt, n_eep, logzeilen, logbuf, gesendet);
        if (nimmt)
            pruef (jetzt == adc && andere == (seite ? mi : ma) && n_eep == 2 && logzeilen == 0 && n_send == 1 && gesendet == adc, was);
        else
            pruef (jetzt == (seite ? ma : mi) && andere == (seite ? mi : ma) && n_eep == 0 && logzeilen == 1 && ! strcmp (logbuf, soll)
                   && n_send == 1 && gesendet == jetzt, was);
    }
    /* ADC faellt aus: unveraendert wie bisher, keine Zeile, kein EEPROM, Rueckmeldung mit dem alten Wert */
    for (seite = 0; seite < 2; seite++)
    {
        null (100, 2000); adc_ausfall = 1;
        rpc (seite == 0 ? LDR_MIN_VALUE_RPC_VAR : LDR_MAX_VALUE_RPC_VAR);
        pruef (ldr.ldr_min_value == 100 && ldr.ldr_max_value == 2000 && n_eep == 0 && logzeilen == 0 && n_send == 1, "ADC-Ausfall: veraendert, Zeile oder EEPROM");
    }
    /* Zahlen-Setter (A59): nimmt JEDE Reihenfolge an, auch die Inversion und Gleichheit -- unveraendert */
    for (i = 0; i < n; i++) for (j = 0; j < n; j++) for (seite = 0; seite < 2; seite++)
    {
        null (W[i], W[j]);
        zahl (seite ? LDR_MAX_VALUE_NUM_VAR : LDR_MIN_VALUE_NUM_VAR, W[(i + j + seite) % n]);
        uint16_t soll_v = W[(i + j + seite) % n];
        snprintf (was, sizeof was, "Zahlen-Setter %s = %u bei min %u max %u: nicht angenommen", seite ? "max" : "min", soll_v, W[i], W[j]);
        pruef ((seite ? ldr.ldr_max_value : ldr.ldr_min_value) == soll_v && n_eep == 2 && logzeilen == 0, was);
    }
    null (100, 2000); zahl (LDR_MIN_VALUE_NUM_VAR, 4096);
    pruef (ldr.ldr_min_value == 100 && n_eep == 0, "Zahlen-Setter 4096: angenommen (Grenze LDR_MAX_RAW_VALUE)");
    printf ("a57: %lu Pruefungen, %lu FEHL -- %s\n", faelle, fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
