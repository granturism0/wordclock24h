/* Pruefstand c46 (Paket 2026-10-13, E1.3, AKQ.1): Query ueber 255 Zeichen bei /api/... abgewiesen statt
 * still gekuerzt - im GET-Weg (Kopf von http () aus dem ECHTEN http.cpp) und im POST-Zweig von
 * http_server_loop () (echter Block). Laengen 254, 255, 256, 1000 (und 0, 1, 300), je /api/... und
 * Legacy. Erwartet:
 *   /api/... laenger als 255  => kein Handler, HTTP 200 mit {"ok":false,"error":9,"detail":"query too long"},
 *                               genau die Logzeile "- abgewiesen: query <n> Zeichen"
 *   /api/... bis 255          => Handler laeuft mit der VOLLSTAENDIGEN Query, keine Logzeile, keine Antwort hier
 *   Legacy (auch "/apix")     => unveraendert: Handler laeuft, Query auf 255 gekuerzt, keine Logzeile
 * Gegenprobe (DIR-014): 3.2.33 kuerzt auch bei /api/... still => FEHL.
 */
#include "attrappe.h"
#include "c46_defs.inc"
#include "c46_code.inc"

static int          handler_laeufe;
static std::string  handler_param, handler_path;
static void http_set_params (char * p) { handler_param = p ? p : ""; }

/* GET: Kopf von http () bis vor die Parameterzerlegung, danach "Handler" */
static uint_fast8_t http_kopf (const char * path, const char * const_param)
{
#include "c46_http.inc"
    handler_laeufe++; handler_path = path; handler_param = param;
    return 1;
}

/* POST-Zweig */
static String           post_path;
static void http_post (const String & sPath) { handler_laeufe++; handler_path = sPath.s; post_path = sPath; }
static void http_log_request (const String &, const char *, const String &) {}
static const char * http_get_request_browser_label (void) { return ""; }
static void post_zweig (const std::string & anfrage)
{
    String  sPath = "", sParam = "", sPoststart = "POST ", sRemoteIp = "192.0.2.20";
    String  sRequest (anfrage);
    int     start_position, end_position_space, end_position_question;
    start_position = sRequest.indexOf (sPoststart);
#include "c46_post.inc"
}

static int faelle, fails;
static void fehl (const std::string & w) { fails++; if (fails <= 30) printf ("FEHL %s\n", w.c_str ()); }

static std::string query (size_t n)
{
    std::string q = "x=";
    while (q.size () < n) q += (char) ('a' + q.size () % 26);
    return q.substr (0, n);
}

static void pruefe (const char * weg, const std::string & path, size_t n)
{
    faelle++;
    std::string q = query (n), wie = std::string (weg) + " " + path + " Query " + std::to_string (n);
    bool api = path.compare (0, 5, "/api/") == 0;
    bool weisen_ab = api && n > 255;
    handler_laeufe = 0; handler_param.clear (); http_out.clear (); serial_out.clear ();
    if (! strcmp (weg, "GET")) http_kopf (path.c_str (), q.c_str ());
    else post_zweig ("POST " + path + (n ? "?" + q : "") + " HTTP/1.1");
    if (weisen_ab)
    {
        if (handler_laeufe) { fehl (wie + ": Handler lief (Query " + std::to_string (handler_param.size ()) + " Zeichen), erwartet abgewiesen"); return; }
        std::string soll = "{\"ok\":false,\"error\":9,\"detail\":\"query too long\"}";
        if (http_out.rfind ("HTTP/1.0 200 OK", 0) != 0 || http_out.find (soll) == std::string::npos) fehl (wie + ": Antwort '" + http_out + "'");
        std::string log = "- abgewiesen: query " + std::to_string (n) + " Zeichen\r\n";
        if (serial_out != log) fehl (wie + ": Logzeile '" + serial_out + "' statt '" + log + "'");
        if (serial_out.find (q.substr (0, 8)) != std::string::npos) fehl (wie + ": Query-Inhalt im Log");
        return;
    }
    if (handler_laeufe != 1) { fehl (wie + ": Handler lief nicht"); return; }
    if (! http_out.empty ()) fehl (wie + ": Antwort vor dem Handler: " + http_out.substr (0, 60));
    if (! serial_out.empty ()) fehl (wie + ": Logzeile ohne Abweisung: " + serial_out);
    std::string erw = q.substr (0, 255);
    if (handler_param != erw) fehl (wie + ": Handler bekam " + std::to_string (handler_param.size ()) + " Zeichen statt " + std::to_string (erw.size ()));
}

int main (void)
{
    const size_t laengen[] = { 0, 1, 254, 255, 256, 300, 1000 };
    const char * pfade_get[]  = { "/api/device_ready", "/api/eeprom_settings_set", "/", "/fs", "/apix", "/api" };
    const char * pfade_post[] = { "/api/fs_upload_icon", "/api/local_esp_update", "/fs-icon", "/flash_stm32_local" };
    int api_ab = 0;
    for (auto p : pfade_get)  for (auto n : laengen) { pruefe ("GET", p, n);  api_ab += ! strncmp (p, "/api/", 5) && n > 255; }
    for (auto p : pfade_post) for (auto n : laengen) { pruefe ("POST", p, n); api_ab += ! strncmp (p, "/api/", 5) && n > 255; }
    printf ("c46 (%s): %d Faelle (GET %d Pfade, POST %d Pfade, je %d Laengen), davon %d abzuweisen\n",
            BEZEICHNUNG, faelle, 6, 4, 7, api_ab);
    if (fails) { printf ("FEHL (%s): %d von %d Faellen\n", BEZEICHNUNG, fails, faelle); return 1; }
    printf ("OK (%s): %d Faelle\n", BEZEICHNUNG, faelle);
    return 0;
}
