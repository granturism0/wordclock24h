/* Pruefstand a82 (Paket 2026-10-13, S2.9, A82, AKA.5): Nach JEDEM Wechsel von Display- oder Ambilight-Power gilt
 * dcf77_enabled == (! display && ! ambilight). DCF77 empfaengt nur bei Display UND Ambilight aus (main.c, ISR).
 * WOERTLICH aus dem Stand (extract.py): dcf77_update (falls vorhanden), set_display_power, Setter vom ESP
 * (case DISPLAY_AMBILIGHT_POWER_NUM_VAR) und Ambilight-Timer. Attrappen: Log, ESP, Nachtschaltzeit.
 * Erschoepfend: 4 Startzustaende x ESP online/offline x alle Folgen bis Laenge 5 aus 8 Schaltwegen. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#define TRUE  1
#define FALSE 0
#define DISPLAY_CLOCK_FLAG_UPDATE_ALL           0x04
#define DISPLAY_CLOCK_FLAG_POWER_ON             0x08
#define DISPLAY_CLOCK_FLAG_POWER_OFF            0x10

static struct
{
    uint_fast8_t    display_power_is_on;
    uint_fast8_t    ambilight_power_is_on;
} display;

static void log_printf (const char * f, ...) { (void) f; }
#define debug_log_printf(...)
static void display_set_ambilight_power (uint_fast8_t on) { display.ambilight_power_is_on = on ? 1 : 0; }
static void display_icon_cancel (void) { }
static void var_send_display_power (void) { }
static void var_send_display_ambilight_power (void) { }
static struct { uint_fast8_t is_online; } esp8266;
static struct { int wday, hour, minute; } gmain;
static volatile uint_fast8_t dcf77_enabled;
static uint32_t         night_off_due;
static uint_fast8_t     night_off_ip;                               /* A70: set_display_power() setzt es zurueck */
static uint_fast8_t     display_clock_flag;
static int              nacht_treffer;                      /* Attrappe: meldet die Ambilight-Schaltzeit faellig */
static uint_fast8_t night_check_night_times (uint_fast8_t i, uint_fast8_t on, int wd, int m) { (void) i; (void) on; (void) wd; (void) m; return nacht_treffer; }

#include "auszug-main.c"

static unsigned long faelle, fehl;
static const char * wegname[8] = { "set_display_power(0,0)", "set_display_power(0,1)", "set_display_power(1,0)", "set_display_power(1,1)",
                                   "Setter Ambilight 0", "Setter Ambilight 1", "Timer Ambilight faellig", "Timer Ambilight nicht faellig" };
static void weg (int w)
{
    switch (w)
    {
        case 0: display_clock_flag = set_display_power (0, 0); break;
        case 1: display_clock_flag = set_display_power (0, 1); break;
        case 2: display_clock_flag = set_display_power (1, 0); break;
        case 3: display_clock_flag = set_display_power (1, 1); break;
        case 4: pf_setter (0); break;
        case 5: pf_setter (1); break;
        case 6: nacht_treffer = 1; pf_timer (); break;
        case 7: nacht_treffer = 0; pf_timer (); break;
    }
}
static unsigned long weg_fehl[8], weg_n[8];

int main (void)
{
    int start, online, len, folge, k;
    for (start = 0; start < 4; start++)
    for (online = 0; online < 2; online++)
    for (len = 1; len <= 5; len++)
    {
        int n = 1; for (k = 0; k < len; k++) n *= 8;
        for (folge = 0; folge < n; folge++)
        {
            int f = folge, schritt;
            display.display_power_is_on = start & 1; display.ambilight_power_is_on = (start >> 1) & 1;
            dcf77_enabled = ! display.display_power_is_on && ! display.ambilight_power_is_on;     /* konsistenter Start */
            esp8266.is_online = online;
            for (schritt = 0; schritt < len; schritt++, f /= 8)
            {
                int w = f % 8, soll;
                weg (w);
                soll = ! display.display_power_is_on && ! display.ambilight_power_is_on;
                faelle++; weg_n[w]++;
                if ((dcf77_enabled ? 1 : 0) != soll)
                {
                    fehl++; weg_fehl[w]++;
                    if (weg_fehl[w] <= 2)
                        printf ("  FEHL  nach %-30s display=%d ambi=%d dcf77_enabled=%d, soll %d (Start d=%d a=%d, online=%d, Schritt %d)\n",
                                wegname[w], (int) display.display_power_is_on, (int) display.ambilight_power_is_on, (int) dcf77_enabled, soll,
                                start & 1, (start >> 1) & 1, online, schritt + 1);
                }
            }
        }
    }
    for (k = 0; k < 8; k++)
        printf ("  %-5s %-30s %lu Pruefungen, %lu falsch\n", weg_fehl[k] ? "FEHL" : "OK", wegname[k], weg_n[k], weg_fehl[k]);
    printf ("  %s  %lu Pruefungen, %lu falsch\n", fehl ? "FEHL" : "OK  ", faelle, fehl);
    return fehl ? 1 : 0;
}
