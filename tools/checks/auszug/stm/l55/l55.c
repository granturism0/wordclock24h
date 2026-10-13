/* Pruefstand l55 (Paket 2026-10-13, S2.1, B55-STM, AKL.1): nach "Messwert uebernehmen" genau eine Zeile
 * "RPCR xx r\r\n" NACH der Wertmeldung; r = 0 uebernommen, 1 abgewiesen, 2 Messung fehlgeschlagen.
 * Attrappen: ADC (Wert oder Ausfall), EEPROM (zaehlt), Log, UART (alles in EINEN Leitungspuffer, die
 * Wertmeldung als Marke "[var n=v]", damit die Reihenfolge mitgeprueft wird). */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#define EEPROM_DATA_OFFSET_LDR_MIN_VALUE 10
#define EEPROM_DATA_SIZE_LDR_MIN_VALUE  2
#define EEPROM_DATA_OFFSET_LDR_MAX_VALUE 12
#define EEPROM_DATA_SIZE_LDR_MAX_VALUE  2
static struct { uint16_t ldr_min_value, ldr_max_value; } ldr;
static uint_fast8_t eep_is_up = 1;
static int n_eep;
static uint_fast8_t eep_write (int off, uint8_t * p, int n) { (void) off; (void) p; (void) n; n_eep++; return 1; }
static uint16_t adc_wert; static int adc_ausfall;
static void adc_start_single_conversion (void) { }
static uint_fast8_t adc_poll_conversion_value (uint16_t * v) { if (adc_ausfall) return 0; *v = adc_wert; return 1; }
static void delay_msec (uint32_t ms) { (void) ms; }
static int logzeilen;
static void log_printf (const char * f, ...) { (void) f; logzeilen++; }
static void log_puts (const char * s) { (void) s; logzeilen++; }
static void log_putc (int c) { (void) c; }
static void log_hex (int c) { (void) c; }
static void log_flush (void) { }
#define debug_log_message(s)
static char leitung[512]; static size_t lpos;
static void esp8266_uart_puts (const char * s) { size_t n = strlen (s); if (lpos + n < sizeof leitung) { memcpy (leitung + lpos, s, n); lpos += n; leitung[lpos] = 0; } }
static void esp8266_uart_putc (int c) { char b[2] = { (char) c, 0 }; esp8266_uart_puts (b); }
static void esp8266_uart_flush (void) { }
static void var_send_ldr_min_value (void) { char b[32]; snprintf (b, sizeof b, "[var min=%u]", (unsigned) ldr.ldr_min_value); esp8266_uart_puts (b); }
static void var_send_ldr_max_value (void) { char b[32]; snprintf (b, sizeof b, "[var max=%u]", (unsigned) ldr.ldr_max_value); esp8266_uart_puts (b); }
#include "auszug.c"

static unsigned long faelle, fehl; static char was[400];
static void pruef (int ok, const char * w) { faelle++; if (! ok) { fehl++; if (fehl <= 8) printf ("  FEHL  %s\n", w); } }
static void null (uint16_t mi, uint16_t ma) { ldr.ldr_min_value = mi; ldr.ldr_max_value = ma; n_eep = 0; logzeilen = 0; lpos = 0; leitung[0] = 0; adc_ausfall = 0; }

/* erwartete Leitung: Wertmeldung, dann genau eine RPCR-Zeile, nichts sonst */
static void fall (int seite, uint16_t mi, uint16_t ma, int ausfall, uint16_t adc, int r_soll, const char * name)
{
    char soll[96]; int idx = seite ? LDR_MAX_VALUE_RPC_VAR : LDR_MIN_VALUE_RPC_VAR;
    uint16_t wert_soll = r_soll == 0 ? adc : (seite ? ma : mi);
    null (mi, ma); adc_wert = adc; adc_ausfall = ausfall;
    rpc ((uint_fast8_t) idx);
    snprintf (soll, sizeof soll, "[var %s=%u]RPCR %02X %d\r\n", seite ? "max" : "min", (unsigned) wert_soll, idx, r_soll);
    snprintf (was, sizeof was, "%s %s: min %u max %u Messwert %u%s -> Leitung \"%s\" statt \"%s\", EEPROM %d",
              seite ? "Maximum" : "Minimum", name, mi, ma, adc, ausfall ? " (Ausfall)" : "", leitung, soll, n_eep);
    pruef (! strcmp (leitung, soll) && n_eep == (r_soll == 0 ? 2 : 0)
           && (seite ? ldr.ldr_max_value : ldr.ldr_min_value) == wert_soll
           && (seite ? ldr.ldr_min_value : ldr.ldr_max_value) == (seite ? mi : ma), was);
}

int main (void)
{
    static const uint16_t W[] = { 0, 1, 2, 99, 100, 101, 2000, 4093, 4094, 4095 };
    int n = (int) (sizeof W / sizeof W[0]), i, j, k, seite;
    int zahl[3] = { 0, 0, 0 };
    for (i = 0; i < n; i++) for (j = 0; j < n; j++) for (k = 0; k < n; k++) for (seite = 0; seite < 2; seite++)
    {
        uint16_t mi = W[i], ma = W[j], adc = W[k];
        int r = (seite == 0 ? (adc < ma) : (adc > mi)) ? 0 : 1;
        zahl[r]++;
        fall (seite, mi, ma, 0, adc, r, r ? "Inversion" : "Uebernahme");
    }
    for (seite = 0; seite < 2; seite++)
    {
        fall (seite, 100, 2000, 1, 555, 2, "Messung fehlgeschlagen"); zahl[2]++;
        fall (seite, 100, 2000, 0, seite ? 2000 : 100, 0, "gleicher Wert gemessen");
    }
    printf ("l55: %lu Pruefungen (uebernommen %d, abgewiesen %d, fehlgeschlagen %d), %lu FEHL -- %s\n",
            faelle, zahl[0], zahl[1], zahl[2], fehl, fehl ? "FEHL" : "OK");
    return fehl ? 1 : 0;
}
