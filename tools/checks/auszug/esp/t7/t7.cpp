/* Pruefstand F.5b (E12): http_api_tft_flags_set () und http_api_dfplayer_bell_flags_set ().
 * Echter Code aus http.cpp (extract.py), Parameter und Variablenspeicher sind Attrappen.
 * 8 Faelle: vollstaendig on/off => geschrieben; fehlender Parameter => Kennung 1, unbekannter
 * Wert => Kennung 2, in beiden Faellen KEIN set_numvar (). Gegenprobe ESP 3.2.25: 5 Fehler.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <map>
#include <string>
#define FS(x) x
#define HTTP_API_ERROR_MISSING_VALUE 1
#define HTTP_API_ERROR_OUT_OF_RANGE 2
#define SSD1963_GLOBAL_FLAGS_RGB_ORDER 1
#define SSD1963_GLOBAL_FLAGS_FLIP_HORIZONTAL 2
#define SSD1963_GLOBAL_FLAGS_FLIP_VERTICAL 4
#define DFPLAYER_MODE_BELL_FLAG_NONE 0
#define DFPLAYER_MODE_BELL_FLAG_15 1
#define DFPLAYER_MODE_BELL_FLAG_30 2
#define DFPLAYER_MODE_BELL_FLAG_45 4
enum { SSD1963_FLAGS_NUM_VAR, DFPLAYER_BELL_FLAGS_NUM_VAR };
static std::map<std::string,std::string> P; static char empty[1];
static char * http_get_param (const char * n) { auto it = P.find (n); return it == P.end () ? empty : (char *) it->second.c_str (); }
static int err, writes, okc; static int lastval;
static void http_json_error (unsigned e, const char *) { err = e; }
static void set_numvar (int, int v) { writes++; lastval = v; }
static void http_send (const char *) {} static void http_flush () {} static void http_json_ok () { okc++; }
static int http_get_on_off_required (const char * param);
#include "e12_code.inc"
static int fails, n;
static void fall (int which, std::map<std::string,std::string> p, int soll_err, int soll_wert)
{ P = p; err = 0; writes = 0; n++;
  if (which) http_api_dfplayer_bell_flags_set (); else http_api_tft_flags_set ();
  bool ok = soll_err ? (err == soll_err && writes == 0) : (err == 0 && writes == 1 && lastval == soll_wert);
  if (! ok) { fails++; printf ("FEHL %s fall %d: err=%d writes=%d wert=%d (soll err=%d wert=%d)\n", which ? "bell" : "tft", n, err, writes, lastval, soll_err, soll_wert); } }
int main ()
{
  fall (0, {{"rgb","on"},{"hflip","off"},{"vflip","on"}}, 0, 5);
  fall (0, {{"rgb","off"},{"hflip","off"},{"vflip","off"}}, 0, 0);
  fall (0, {{"rgb","on"},{"vflip","on"}}, 1, 0);           /* hflip fehlt */
  fall (0, {}, 1, 0);
  fall (0, {{"rgb","on"},{"hflip","ja"},{"vflip","on"}}, 2, 0);
  fall (1, {{"m15","on"},{"m30","on"},{"m45","off"}}, 0, 3);
  fall (1, {{"m15","on"}}, 1, 0);
  fall (1, {{"m15","on"},{"m30","off"},{"m45","x"}}, 2, 0);
  printf ("%s (%s): %d Faelle, %d Fehler\n", fails ? "FEHLGESCHLAGEN" : "OK", BEZEICHNUNG, n, fails); return fails ? 1 : 0;
}
