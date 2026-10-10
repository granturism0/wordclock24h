/* Pruefstand A2-Nachtrag: Index 255 (Fehlerwert des STM) bleibt bei einer Korrekturaenderung 255, jeder
 * gueltige Index wird auf 0..250 geklammert wie im STM (tempsensor.c, rtc.c) - nie auf 255.
 * Wege: /api/temperature_*_correction_set (http_api_temperature_correction_set ()) fuer DS18xx und RTC,
 * Legacy savetcorrrtc und savetcorrds18xx. Erschoepfend: Index 0..255 x alte Korrektur x neue Korrektur
 * (-20..20 in Schritten von 5), je Weg und Sensor; erwartet wird unabhaengig formuliert.
 * Zieltypen wie t3: uint_fast8_t/uint_fast16_t -> unsigned int (xtensa).
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#define uint_fast8_t  unsigned int
#define uint_fast16_t unsigned int
#include VARSHDR
#define HTTP_API_ERROR_MISSING_VALUE 1
#define HTTP_API_ERROR_OUT_OF_RANGE  2

unsigned int numvars[MAX_NUM_VARIABLES];
unsigned int get_numvar (NUM_VARIABLE v) { return v < MAX_NUM_VARIABLES ? numvars[v] : 0; }
unsigned int set_numvar (NUM_VARIABLE v, unsigned int x) { if (v < MAX_NUM_VARIABLES) numvars[v] = x; return 1; }
static int   param_value; static int last_error;
static char  parambuf[16];
static int   http_get_int_param (const char *, int * v) { *v = param_value; return 1; }
static char * http_get_param (const char *) { snprintf (parambuf, sizeof parambuf, "%d", param_value); return parambuf; }
static void  http_json_error (int code, const char *) { last_error = code; }
static void  http_json_ok (void) { last_error = 0; }

#include "tidx_code.inc"

static void legacy (const char * act)
{
    char *          action = (char *) act;
    int8_t          rtc_temperature_correction;
    int8_t          temperature_correction;
#include "tidx_legacy.inc"
    (void) rtc_temperature_correction; (void) temperature_correction;
}

static int n, fails;
static int soll (int idx, int delta)
{
    if (idx == 255) return 255;
    long v = (long) idx - delta;
    return v < 0 ? 0 : v > 250 ? 250 : (int) v;
}
int main (void)
{
    const NUM_VARIABLE idxv[2]  = { DS18XX_TEMP_INDEX_NUM_VAR, RTC_TEMP_INDEX_NUM_VAR };
    const NUM_VARIABLE corrv[2] = { DS18XX_TEMP_CORRECTION_NUM_VAR, RTC_TEMP_CORRECTION_NUM_VAR };
    const char * legact[2] = { "savetcorrds18xx", "savetcorrrtc" };
    const char * name[2] = { "DS18xx", "RTC" };
    int gemeldet = 0;
    for (int s = 0; s < 2; s++)
    for (int weg = 0; weg < 2; weg++)
    for (int idx = 0; idx <= 255; idx++)
    for (int alt = -20; alt <= 20; alt += 5)
    for (int neu = -20; neu <= 20; neu += 5)
    {
        n++;
        memset (numvars, 0, sizeof numvars);
        numvars[RTC_TEMP_HALF_DEG_NUM_VAR] = 0x8000;
        numvars[idxv[s]] = idx;
        numvars[corrv[s]] = http_encode_temp_correction (alt);
        param_value = neu;
        if (weg == 0) http_api_temperature_correction_set (idxv[s], corrv[s]); else legacy (legact[s]);
        int ist = (int) numvars[idxv[s]], erw = soll (idx, neu - alt);
        if (ist != erw)
        {
            fails++;
            if (gemeldet++ < 12 || (idx == 255 && alt == 0 && neu == 5))
                printf ("FEHL %s %s: Index %d, Korrektur %d -> %d: %d statt %d\n", name[s], weg ? "Legacy" : "API", idx, alt, neu, ist, erw);
        }
    }
    printf ("%s (%s): %d Faelle (2 Sensoren x 2 Wege x 256 Indizes x 81 Korrekturpaare), %d Fehler\n",
            fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, n, fails);
    return fails ? 1 : 0;
}
