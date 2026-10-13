/* Pruefstand c19 (Paket 2026-10-13, E1.4, AKU.1; nur L311, Ent-7): Legacy saveuphost/saveuppath an BEIDEN
 * Stellen - Dateisystemseite /fs (http_fs) und Update-Seite /update (http_update), je der ECHTE Aktionsblock
 * und die ECHTE Anzeige auf der Seite. Werte: fehlend, leer, 1, 62, 63, 64, 200 Byte.
 * Erwartet: gespeichert genau dann, wenn 1 <= Laenge <= 63 (MAX_UPDATE_HOST_LEN/MAX_UPDATE_PATH_LEN) - dieselbe
 * Entscheidung wie /api/update_host_set bzw. _path_set (echter Code, mitgeprueft); abgewiesen heisst: Host
 * bzw. Pfad unveraendert, die Seite zeigt die Abweisung.
 * Gegenprobe (DIR-014): 3.2.33 speichert leer und kuerzt ueberlang still => FEHL.
 */
#include "attrappe.h"

#define POST_ICON_NONE              0
#include "c19_defs.inc"

static void http_json_error (unsigned code, const char *) { http_out += "ERR" + std::to_string (code); }
static void http_json_ok (void) { http_out += "OK"; }
static int download_file (const char *, const char *, const char *) { return 1; }
static void tables_init (void) {}
static bool http_remote_stm32_filename_matches (const char *) { return false; }

#include "c19_code.inc"

static uint_fast8_t leg_fs (void)
{
    int post = POST_ICON_NONE;
    char * action = http_get_param ("action");
    const char * fname_icon = "i.txt"; const char * fname_weather = "w.txt"; const char * show_fname = 0;
    char * update_host = (char *) "h"; char * update_path = (char *) "p";
    int download_rtc = 2; uint_fast8_t update_source_rejected = 0;
    (void) post;
#include "c19_fs.inc"
#include "c19_fs_anzeige.inc"
    (void) show_fname; (void) download_rtc; (void) update_source_rejected; return 1;
}
static uint_fast8_t leg_update (void)
{
    char * action = http_get_param ("action");
    char flash_stm32_filename[MAX_UPDATE_FILENAME_LEN];
    int do_update = 0; int do_reset = 0; int flash_rejected = 0; uint_fast8_t update_source_rejected = 0;
#include "c19_update.inc"
#include "c19_update_anzeige.inc"
    (void) do_update; (void) do_reset; (void) flash_rejected; (void) update_source_rejected; return 1;
}

static int faelle, fails;
static void fehl (const std::string & w) { fails++; if (fails <= 30) printf ("FEHL %s\n", w.c_str ()); }

int main (void)
{
    struct { const char * action; const char * feld; STR_VARIABLE var; unsigned max; int (*api) (void); } ziele[] = {
        { "saveuphost", "uphost", UPDATE_HOST_VAR, MAX_UPDATE_HOST_LEN, http_api_update_host_set },
        { "saveuppath", "uppath", UPDATE_PATH_VAR, MAX_UPDATE_PATH_LEN, http_api_update_path_set },
    };
    const int laengen[] = { -1, 0, 1, 62, 63, 64, 200 };                  // -1: Parameter fehlt
    for (auto & z : ziele)
        for (int seite = 0; seite < 2; seite++)
            for (int n : laengen)
            {
                faelle++;
                std::string wert = n < 0 ? "" : std::string (n, 'a' + (n % 26));
                std::string wie = std::string (seite ? "/update" : "/fs") + "?action=" + z.action + " " + z.feld + "="
                                + (n < 0 ? "(fehlt)" : std::to_string (n) + " Byte");
                bool soll = n >= 1 && n <= (int) z.max;
                /* API als Massstab: dieselbe Entscheidung */
                strvar_sets.clear (); http_out.clear ();
                params = n < 0 ? std::map<std::string, std::string> {} : std::map<std::string, std::string> { { "value", wert } };
                z.api ();
                bool api_gespeichert = ! strvar_sets.empty ();
                if (api_gespeichert != soll) fehl (wie + ": die API entscheidet anders als erwartet (Pruefstand falsch?)");
                /* Legacy */
                strvar_vals[z.var] = "alt.example";
                strvar_sets.clear (); http_out.clear ();
                params = { { "action", z.action } }; if (n >= 0) params[z.feld] = wert;
                if (seite) leg_update (); else leg_fs ();
                bool gespeichert = ! strvar_sets.empty ();
                bool meldung = http_out.find ("Abgewiesen") != std::string::npos;
                if (gespeichert != soll) { fehl (wie + (gespeichert ? ": gespeichert ('" + strvar_vals[z.var].substr (0, 20) + "...'), erwartet abgewiesen" : ": abgewiesen, erwartet gespeichert")); continue; }
                if (gespeichert && strvar_vals[z.var] != wert) fehl (wie + ": gespeichert, aber veraendert");
                if (! gespeichert && strvar_vals[z.var] != "alt.example") fehl (wie + ": alter Wert nicht erhalten");
                if (meldung != ! soll) fehl (wie + (meldung ? ": Abweisung angezeigt, obwohl gespeichert" : ": keine Abweisung auf der Seite"));
            }
    printf ("c19 (%s): %d Faelle (2 Felder x 2 Seiten x 7 Werte), API als Massstab je Fall\n", BEZEICHNUNG, faelle);
    if (fails) { printf ("FEHL (%s): %d von %d Faellen\n", BEZEICHNUNG, fails, faelle); return 1; }
    printf ("OK (%s): %d Faelle\n", BEZEICHNUNG, faelle);
    return 0;
}
