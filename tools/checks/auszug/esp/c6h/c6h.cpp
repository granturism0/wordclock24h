/* Pruefstand c6h (Paket 2026-10-13, E1.1/E1.2, AKH.1-AKH.3): Herkunftspruefung an JEDEM zerstoerenden Weg.
 *
 * Echter Code aus http.cpp: die Kopfzeilenschleife des GET-Zweigs, die Erfassung von Sec-Fetch-Dest und
 * -Site, die Pruefung, die vier API-Handler (maintenance_format_fs, _reset_stm32, _reset_eeprom, fs_remove)
 * und die Aktionsbloecke der Legacy-Seiten / (http_main), /fs, /update und /flash_stm32_local.
 *
 * Teil M  jede Kombination Sec-Fetch-Dest x Sec-Fetch-Site gegen jeden Weg: ausgefuehrt genau dann, wenn
 *         Dest nicht eingebettet ist UND Site fehlt, "same-origin" oder "none" ist. Abgewiesen heisst: keine
 *         Wirkung, 403, JSON mit "embedded-subresource" bzw. "cross-site" (API) oder HTML (Legacy), und
 *         genau eine Logzeile "- abgewiesen: <was> als Sec-Fetch-Dest=|Site=<wert>". Zugelassen heisst:
 *         Wirkung, kein 403, keine Logzeile. Die Faelle laufen nacheinander - eine fremde Kopfzeile darf
 *         nicht in die naechste Anfrage nachwirken.
 * Teil P  /fs als POST (Upload-Zweig, Kopfzeilen nie erfasst): action=format/remove ohne Wirkung.
 *
 * Gegenprobe (DIR-014): 3.2.33 prueft Site nirgends und Legacy gar nicht => FEHL.
 */
#include "attrappe.h"

#define POST_ICON_NONE              0
#define POST_ICON_FILE              1
#include "c6h_defs.inc"

static void http_json_error (unsigned code, const char * d) { http_out += "HTTP/1.0 200 OK\r\n\r\n{\"ok\":false,\"error\":" + std::to_string (code) + ",\"detail\":\"" + d + "\"}"; }
static void http_json_ok (void) { http_out += "HTTP/1.0 200 OK\r\n\r\n{\"ok\":true}"; }
static uint_fast8_t http_get_int_param (const char * n, int * v) { char * p = http_get_param (n); if (! *p) return 0; *v = atoi (p); return 1; }
static uint_fast8_t http_days_in_month (int, int) { return 31; }
static int download_file (const char *, const char *, const char *) { return 1; }
static void tables_init (void) {}
static bool http_remote_stm32_filename_matches (const char *) { return false; }

#include "c6h_code.inc"

static void kopf_lesen (void)
{
    String sHeaderLine;
#include "c6h_loop.inc"
}

static int leg_flag;
static int leg_post = POST_ICON_NONE;

static uint_fast8_t leg_main (void)
{
    char year_str[16], mon_str[16], day_str[16], hour_str[16], minutes_str[16];
    const char * message = 0; const char * alert_message = 0;
    uint_fast8_t do_reset = 0; uint_fast8_t do_reset_eeprom = 0;
    char * action = http_get_param ("action");
#include "c6h_main.inc"
    (void) message; (void) alert_message; (void) do_reset_eeprom;
    leg_flag = do_reset; return 1;
}
static uint_fast8_t leg_fs (void)
{
    int post = leg_post;
    char * action = http_get_param ("action");
    const char * fname_icon = "i.txt"; const char * fname_weather = "w.txt"; const char * show_fname = 0;
    char * update_host = (char *) "h"; char * update_path = (char *) "p";
    int download_rtc = 2; uint_fast8_t update_source_rejected = 0;
    (void) post;
#include "c6h_fs.inc"
    (void) show_fname; (void) download_rtc; (void) update_source_rejected; return 1;
}
static uint_fast8_t leg_update (void)
{
    char * action = http_get_param ("action");
    char flash_stm32_filename[MAX_UPDATE_FILENAME_LEN];
    int do_update = 0; int do_reset = 0; int flash_rejected = 0; uint_fast8_t update_source_rejected = 0;
#include "c6h_update.inc"
    (void) do_update; (void) flash_rejected; (void) update_source_rejected;
    leg_flag = do_reset; return 1;
}
static uint_fast8_t leg_local (void)
{
    char * action = http_get_param ("action");
    int do_reset = 0;
#include "c6h_local.inc"
    leg_flag = do_reset; return 1;
}

struct Weg { const char * name; bool api; const char * was; std::map<std::string, std::string> p; int (*lauf) (void); int * zaehler; };
static int z_flag;
static int r_fmt (void) { http_api_maintenance_format_fs (); return 0; }
static int r_stm (void) { http_api_maintenance_reset_stm32 (); return 0; }
static int r_eep (void) { http_api_maintenance_reset_eeprom (); return 0; }
static int r_rm  (void) { http_api_fs_remove (); return 0; }
static int r_main (void) { leg_flag = 0; leg_main (); z_flag += leg_flag != 0; return 0; }
static int r_fs (void) { leg_fs (); return 0; }
static int r_upd (void) { leg_flag = 0; leg_update (); z_flag += leg_flag != 0; return 0; }
static int r_loc (void) { leg_flag = 0; leg_local (); z_flag += leg_flag != 0; return 0; }

static int faelle, fails;
static void fehl (const std::string & w) { fails++; if (fails <= 40) printf ("FEHL %s\n", w.c_str ()); }

static bool eingebettet (const std::string & d)
{
    static const char * e[] = { "image", "audio", "video", "font", "style", "script", "track", "embed", "object", "manifest" };
    for (auto x : e) if (d == x) return true;
    return false;
}

int main (void)
{
    std::vector<Weg> wege = {
        { "API maintenance_format_fs",    true,  "maintenance_format_fs",   {},                                        r_fmt,  &n_format },
        { "API maintenance_reset_stm32",  true,  "maintenance_reset_stm32", {},                                        r_stm,  &n_stm32_reset },
        { "API maintenance_reset_eeprom", true,  "maintenance_reset_eeprom",{},                                        r_eep,  &n_rpc_reset_eeprom },
        { "API fs_remove",                true,  "fs_remove",               { { "filename", "x.txt" } },               r_rm,   &n_remove },
        { "Legacy /?action=rststm32",     false, "legacy rststm32",         { { "action", "rststm32" } },              r_main, &z_flag },
        { "Legacy /?action=rsteeprom2",   false, "legacy rsteeprom2",       { { "action", "rsteeprom2" } },            r_main, &n_rpc_reset_eeprom },
        { "Legacy /fs?action=format",     false, "legacy fs format",        { { "action", "format" } },                r_fs,   &n_format },
        { "Legacy /fs?action=remove",     false, "legacy fs remove",        { { "action", "remove" }, { "filename", "x.txt" } }, r_fs, &n_remove },
        { "Legacy /update?action=reset",  false, "legacy update reset",     { { "action", "reset" } },                 r_upd,  &z_flag },
        { "Legacy /flash_stm32_local?action=reset", false, "legacy local reset", { { "action", "reset" } },            r_loc,  &z_flag },
    };
    const char * dests[] = { "", "empty", "document", "iframe", "image", "script", "font", "object" };
    const char * sites[] = { "", "same-origin", "none", "same-site", "cross-site", "fremd-und-unbekannt-lang" };
    int ausgefuehrt = 0, abgewiesen = 0;

    for (auto & w : wege)
        for (auto d : dests)
            for (auto s : sites)
            {
                faelle++;
                std::string wie = std::string (w.name) + " Dest=" + (*d ? d : "-") + " Site=" + (*s ? s : "-");
                kopfzeilen.clear ();
                kopfzeilen.push_back ("Host: 192.0.2.10");
                if (*d) kopfzeilen.push_back (std::string ("Sec-Fetch-Dest: ") + d);
                if (*s) kopfzeilen.push_back (std::string ("Sec-Fetch-Site: ") + s);
                kopfzeilen.push_back ("Accept: */*");
                kopfzeilen.push_back ("");
                http_clear_request_fetch_dest ();
                kopf_lesen ();
                params = w.p; http_out.clear (); serial_out.clear ();
                int vorher = *w.zaehler;
                w.lauf ();
                bool wirkung = *w.zaehler != vorher;
                bool erlaubt = ! eingebettet (d) && (! *s || ! strcmp (s, "same-origin") || ! strcmp (s, "none"));
                bool hat403 = http_out.rfind ("HTTP/1.0 403 Forbidden", 0) == 0;
                if (erlaubt)
                {
                    ausgefuehrt++;
                    if (! wirkung) fehl (wie + ": nicht ausgefuehrt, erwartet ausgefuehrt");
                    if (hat403) fehl (wie + ": 403 trotz Zulassung");
                    if (serial_out.find ("abgewiesen") != std::string::npos) fehl (wie + ": Logzeile trotz Zulassung: " + serial_out);
                    continue;
                }
                abgewiesen++;
                if (wirkung) { fehl (wie + ": AUSGEFUEHRT, erwartet abgewiesen"); continue; }
                if (! hat403) { fehl (wie + ": keine 403-Antwort: " + http_out.substr (0, 40)); continue; }
                bool per_dest = eingebettet (d);
                std::string log = std::string ("- abgewiesen: ") + w.was + (per_dest ? " als Sec-Fetch-Dest=" : " als Sec-Fetch-Site=")
                                + (per_dest ? d : std::string (s).substr (0, 15)) + "\r\n";
                if (serial_out != log) fehl (wie + ": Logzeile '" + serial_out + "' statt '" + log + "'");
                if (w.api)
                {
                    std::string body = per_dest ? "{\"ok\":false,\"error\":\"embedded-subresource\"}" : "{\"ok\":false,\"error\":\"cross-site\"}";
                    if (http_out.find ("Content-Type: application/json") == std::string::npos || http_out.find (body) == std::string::npos)
                        fehl (wie + ": JSON-Antwort falsch: " + http_out);
                }
                else if (http_out.find ("Content-Type: text/html") == std::string::npos || http_out.find ("abgewiesen") == std::string::npos)
                    fehl (wie + ": keine HTML-Hinweisseite: " + http_out.substr (0, 80));
            }

    /* Teil P: /fs als POST - die Kopfzeilen liest dort der Upload, Sec-Fetch-* ist nie erfasst */
    leg_post = POST_ICON_FILE;
    for (auto a : { "format", "remove" })
    {
        faelle++;
        http_clear_request_fetch_dest ();
        params = { { "action", a }, { "filename", "x.txt" } }; http_out.clear (); serial_out.clear ();
        int f = n_format, r = n_remove;
        leg_fs ();
        if (n_format != f || n_remove != r) fehl (std::string ("POST /fs-icon?action=") + a + ": AUSGEFUEHRT, erwartet ohne Wirkung");
    }
    leg_post = POST_ICON_NONE;

    /* Teil W: unsere Werkzeuge, ausdruecklich benannt (der Fall steckt auch in Teil M) */
    struct { const char * wer; const char * dest; bool soll403; } wz[] = {
        { "tools/smoke-device.sh: curl -H 'Sec-Fetch-Dest: image' /api/maintenance_reset_stm32", "image", true },
        { "tools/flash-stm.sh: curl ohne Kopfzeile /api/maintenance_reset_stm32", "", false },
    };
    for (auto & w : wz)
    {
        faelle++;
        kopfzeilen.clear (); kopfzeilen.push_back ("Host: 192.0.2.10"); kopfzeilen.push_back ("User-Agent: curl/8.7.1");
        if (*w.dest) kopfzeilen.push_back (std::string ("Sec-Fetch-Dest: ") + w.dest);
        kopfzeilen.push_back ("");
        http_clear_request_fetch_dest (); kopf_lesen ();
        params.clear (); http_out.clear (); serial_out.clear ();
        int vorher = n_stm32_reset; http_api_maintenance_reset_stm32 ();
        bool ist403 = http_out.rfind ("HTTP/1.0 403 Forbidden", 0) == 0, reset = n_stm32_reset != vorher;
        printf ("  W %s => %s, STM-Reset %s\n", w.wer, ist403 ? "403" : "200", reset ? "ja" : "nein");
        if (ist403 != w.soll403 || reset == w.soll403) fehl (std::string ("Werkzeug: ") + w.wer);
    }

    printf ("c6h (%s): %d Faelle (%d Wege x %d Dest x %d Site + 2 POST + 2 Werkzeug), %d erlaubt, %d abzuweisen\n",
            BEZEICHNUNG, faelle, (int) wege.size (), 8, 6, ausgefuehrt, abgewiesen);
    if (fails) { printf ("FEHL (%s): %d von %d Faellen\n", BEZEICHNUNG, fails, faelle); return 1; }
    printf ("OK (%s): %d Faelle\n", BEZEICHNUNG, faelle);
    return 0;
}
