/* Pruefstand F.5e (Overlay-Text, ESP / L319): http_api_overlay_set () aus http.cpp, mit den
 * echten http_get_int_param (), http_get_opt_int_param () und http_check_strvar_len ().
 * 5 Faelle: 32 Byte und 16 x ae angenommen; 33 Byte und 17 x ae (34 Byte) abgewiesen mit 2,
 * gespeicherter Text unveraendert, kein set_overlay_var (). Gegenprobe ESP 3.2.25: 2 Fehler.
 */
/* Overlay-Text, ESP: http_api_overlay_set() weist > 32 Byte mit Kennung 2 ab, gespeicherter Text bleibt. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <map>
#define uint_fast8_t  unsigned int
#define uint_fast16_t unsigned int
#include VARSHDR
#define HTTP_API_ERROR_MISSING_VALUE 1
#define HTTP_API_ERROR_OUT_OF_RANGE  2
OVERLAY overlays[MAX_OVERLAYS];
unsigned int numvars[MAX_NUM_VARIABLES];
unsigned int get_numvar (NUM_VARIABLE v) { return numvars[v]; }
unsigned int set_numvar (NUM_VARIABLE v, unsigned int x) { numvars[v] = x; return 1; }
static int sets; unsigned int set_overlay_var (uint_fast8_t) { sets++; return 1; }
void utf8_copy_truncated (char * d, const char * s, unsigned int n) { strncpy (d, s, n); d[n] = 0; }
static std::map<std::string, std::string> params;
static char * http_get_param (const char * n) { static char b[8][256]; static int i; char * p = b[i++ % 8]; snprintf (p, 256, "%s", params.count (n) ? params[n].c_str () : ""); return p; }
static int  last_error = -1; static std::string last_detail;
static void http_json_error (int code, const char * d) { last_error = code; last_detail = d; }
static void http_json_ok (void) { last_error = 0; }
static uint_fast8_t http_get_on_off_value (const char *, uint_fast8_t c) { return c; }
#include "ovl_code.inc"
static int fails, n;
static void fall (const std::string & text, int soll)
{ n++; strcpy (overlays[0].text, "alt"); numvars[OVERLAY_N_OVERLAYS_NUM_VAR] = 1; sets = 0;
  params = { {"idx","0"}, {"type","1"}, {"date_code","0"}, {"value", text} };
  http_api_overlay_set ();
  bool ok = soll ? (last_error == soll && ! strcmp (overlays[0].text, "alt") && sets == 0)
                 : (last_error == 0 && text == overlays[0].text && sets == 1);
  if (! ok) { fails++; printf ("FEHL fall %d (%zu Byte): err=%d text='%s' sets=%d\n", n, text.size (), last_error, overlays[0].text, sets); } }
int main ()
{
  fall (std::string (32, 'x'), 0);                    /* Grenzwert: angenommen */
  fall (std::string (33, 'x'), 2);                    /* ein Byte zu viel */
  fall ("", 0);
  std::string u; for (int i = 0; i < 16; i++) u += "\xc3\xa4"; fall (u, 0);          /* 16 x ae = 32 Byte */
  fall (u + "\xc3\xa4", 2);                                                            /* 17 x ae = 34 Byte */
  printf ("%s (%s): %d Faelle, %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, n, fails); return fails ? 1 : 0;
}
