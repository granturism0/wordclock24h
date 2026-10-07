/* Pruefstand S.4b (A5, ESP-Teil, AKS.15): Index 49, Vorbelegung 0x8000, Legacy-Formatierung
 * vorzeichenrichtig mit Rueckfall auf Index 21, Nachrechnen bei einer Korrekturaenderung.
 * Uebersetzt den echten Code (extract.py): vars_init(), den RTC-Anzeigeblock aus http_temperature(),
 * die A5-Hilfsfunktionen und http_api_temperature_correction_set().
 * Zieltypen: uint_fast8_t/uint_fast16_t -> unsigned int (xtensa), int16_t fest.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#define uint_fast8_t  unsigned int
#define uint_fast16_t unsigned int
#include VARSHDR
#define ESP_VERSION "test"
#define HTTP_API_ERROR_MISSING_VALUE 1
#define HTTP_API_ERROR_OUT_OF_RANGE  2

unsigned int numvars[MAX_NUM_VARIABLES];
static char  strbuf[MAX_STR_VARIABLES][80];
STR_VAR      strvars[MAX_STR_VARIABLES];
unsigned int get_numvar (NUM_VARIABLE v) { return v < MAX_NUM_VARIABLES ? numvars[v] : 0; }
unsigned int set_numvar (NUM_VARIABLE v, unsigned int x) { if (v < MAX_NUM_VARIABLES) numvars[v] = x; return 1; }
void          utf8_copy_truncated (char * d, const char * s, unsigned int n) { strncpy (d, s, n); d[n] = 0; }
static int   param_value; static int last_error;
static int   http_get_int_param (const char *, int * v) { *v = param_value; return 1; }
static void  http_json_error (int code, const char *) { last_error = code; }
static void  http_json_ok (void) { last_error = 0; }

#include "a5_code.inc"

static std::string legacy_rtc_text (unsigned int rtc_is_up)
{
    char            rtc_temp[16];
    uint_fast8_t    temp_index;
#include "rtc_block.inc"
    (void) temp_index;
    return rtc_temp;
}

static int fails;
static const unsigned IDX49 = 49;
static void set49 (int v) { if (IDX49 < MAX_NUM_VARIABLES) numvars[IDX49] = (unsigned int) (uint16_t) (int16_t) v; }
static void set49raw (unsigned v) { if (IDX49 < MAX_NUM_VARIABLES) numvars[IDX49] = v; }
static int  get49 (void) { return IDX49 < MAX_NUM_VARIABLES ? (int) numvars[IDX49] : -999999; }
static void check (bool ok, const char * name, const std::string & got = "")
{
    printf ("  [%s] %s%s%s\n", ok ? " ok " : "FEHL", name, ok || got.empty () ? "" : " -- bekam: ", ok ? "" : got.c_str ());
    fails += ! ok;
}
static void fmt (int v49, unsigned v49raw, bool raw, unsigned v21, const char * want, const char * name)
{
    if (raw) set49raw (v49raw); else set49 (v49);
    numvars[RTC_TEMP_INDEX_NUM_VAR] = v21;
    std::string got = legacy_rtc_text (1);
    check (got == want, name, got);
}
int main (void)
{
    int n = 0;
    printf ("Pruefstand S.4b / A5 (%s)\n", BEZEICHNUNG);
    for (int i = 0; i < MAX_STR_VARIABLES; i++) { strvars[i].str = strbuf[i]; strvars[i].maxlen = 63; }

    /* Vorbelegung */
    check (IDX49 < MAX_NUM_VARIABLES, "Index 49 existiert (MAX_NUM_VARIABLES > 49)", std::to_string (MAX_NUM_VARIABLES)); n++;
    memset (numvars, 0, sizeof numvars); vars_init ();
    check (get49 () == 0x8000, "vars_init(): Index 49 = 0x8000, nicht 0", std::to_string (get49 ())); n++;

    /* Legacy-Formatierung: -20, -1, 0, +49, 0x8000 (Spec) und -3 (Beispiel AKS.15) */
    fmt (-20, 0, false, 0,  "-10&deg;C",   "-20 halbe Grad => -10"); n++;
    fmt (-1,  0, false, 0,  "-0.5&deg;C",  "-1 halbe Grad => -0.5"); n++;
    fmt (-3,  0, false, 0,  "-1.5&deg;C",  "-3 halbe Grad => -1.5"); n++;
    fmt (0,   0, false, 0,  "0&deg;C",     "0 halbe Grad => 0"); n++;
    fmt (49,  0, false, 49, "24.5&deg;C",  "+49 halbe Grad => 24.5"); n++;
    fmt (0, 0x8000, true, 48, "24&deg;C",  "0x8000 => Rueckfall auf Index 21 (48 => 24)"); n++;
    fmt (0, 0x8000, true, 49, "24.5&deg;C","0x8000 => Rueckfall auf Index 21 (49 => 24.5)"); n++;
    { std::string g = legacy_rtc_text (0); check (g == "offline", "RTC aus => offline", g); n++; }

    /* Nachrechnen bei Korrekturaenderung ueber den echten Endpunkt */
    numvars[RTC_TEMP_CORRECTION_NUM_VAR] = 0; numvars[RTC_TEMP_INDEX_NUM_VAR] = 48; set49 (48); param_value = 4;
    http_api_temperature_correction_set (RTC_TEMP_INDEX_NUM_VAR, RTC_TEMP_CORRECTION_NUM_VAR);
    check (numvars[RTC_TEMP_INDEX_NUM_VAR] == 44 && get49 () == 44, "Korrektur 0 -> +4: Index 21 und 49 beide 48 -> 44",
           std::to_string (numvars[RTC_TEMP_INDEX_NUM_VAR]) + "/" + std::to_string (get49 ())); n++;
    numvars[RTC_TEMP_CORRECTION_NUM_VAR] = 0; numvars[RTC_TEMP_INDEX_NUM_VAR] = 0; set49 (-3); param_value = -4;
    http_api_temperature_correction_set (RTC_TEMP_INDEX_NUM_VAR, RTC_TEMP_CORRECTION_NUM_VAR);
    check (get49 () == 1, "Korrektur 0 -> -4 bei -3 halben Grad: Index 49 => +1", std::to_string (get49 ())); n++;
    numvars[RTC_TEMP_CORRECTION_NUM_VAR] = 0; numvars[RTC_TEMP_INDEX_NUM_VAR] = 48; set49raw (0x8000); param_value = 4;
    http_api_temperature_correction_set (RTC_TEMP_INDEX_NUM_VAR, RTC_TEMP_CORRECTION_NUM_VAR);
    check (get49 () == 0x8000, "0x8000 bleibt bei einer Korrektur 0x8000", std::to_string (get49 ())); n++;
    numvars[DS18XX_TEMP_CORRECTION_NUM_VAR] = 0; set49 (48); param_value = 4;
    http_api_temperature_correction_set (DS18XX_TEMP_INDEX_NUM_VAR, DS18XX_TEMP_CORRECTION_NUM_VAR);
    check (get49 () == 48, "DS18xx-Korrektur laesst Index 49 unberuehrt", std::to_string (get49 ())); n++;

    printf ("Ergebnis: %d von %d Faellen bestanden\n", n - fails, n);
    return fails ? 1 : 0;
}
