/* Pruefstand c65o (Paket 2026-10-13, E4.5; AKV.11; design-e4.md 19.8).
 * Echter Code: die Weissliste http_api_read_only_paths, http_api_is_read_only () und http () bis VOR den Verteiler
 * (Query-Grenze, Parameter, Hardware, Pruefung auf verwaiste Lesende). An der Stelle des Verteilers haengt der
 * Pruefstand einen Ersatzverteiler an, der nur festhaelt, ob ein Handler lief - steht die Pruefung NACH dem Verteiler,
 * fehlt sie im Auszug, und die Faelle schlagen an. Statisch (c65o_static.py): Liste gegen den echten Verteiler.
 * WiFiClient::connected () zaehlt seine Aufrufe: Fuer nicht gelistete Pfade wird es gar nicht erst gefragt.
 */
#include "basis.h"
#include "lwip_attrappe.h"
#include "have.inc"
#define PROGMEM
#define strcmp_P strcmp
static std::map<std::string, std::string> params;        /* Review E4.10 M1: refresh */
static char * http_get_param (const char * n) { static char b[8][300]; static int i; char * p = b[i++ % 8]; snprintf (p, 300, "%s", params.count (n) ? params[n].c_str () : ""); return p; }
static void sdk_abgabe (void) {}
static std::string out; static std::vector<std::string> gelaufen; static int connected_aufrufe = 0; static bool verbunden = true;
struct ClientO { uint8_t connected () { connected_aufrufe++; return verbunden ? 1 : 0; } uint8_t status () { return verbunden ? ESTABLISHED : CLOSED; }
                 int availableForWrite () { return 1072; } } http_client;
struct ServerO { uint8_t pending () const { return 0; } void backlog (uint8_t) {} } http_server;
#define TCP_MSS 536
#include "i4.inc"                                       /* api_orphan_skipped */
#ifndef HAVE_I4
static uint16_t api_orphan_skipped = 0;
#endif
#include "orphan.inc"
static uint_fast8_t http_api_query_too_long (const char *, size_t) { return 0; }
static void http_set_params (char *) {}
static uint_fast16_t hardware_configuration = 0x0001; static const char * pgm_name, * hardware;
#define HARDWARE_CONFIGURATION_NUM_VAR 0
#define HW_WC_MASK 0x03
#define HW_WC_24H 0x01
#define HW_WC_12H 0x02
#define HW_UCLOCK 0x03
static int get_numvar (int) { return 1; }
static int verteiler (const char * path) { gelaufen.push_back (path); out += "HTTP/1.0 200 OK"; return 0; }
#include "http_vor.inc"
    rtc = verteiler (path);                             /* Ersatzverteiler an der Stelle des echten */
    return rtc;
}

static bool lauf (const char * pfad, bool verb) { gelaufen.clear (); out.clear (); verbunden = verb; http (pfad, ""); return ! gelaufen.empty (); }

int main (void)
{
    printf ("c65o (%s)\n", BEZEICHNUNG);
    const char * liste[] = { "/api/settings_xml", "/api/display_power", "/api/ambilight_power", "/api/power_status", "/api/network_scan",
        "/api/eeprom_settings", "/api/network_secrets", "/api/update_status", "/api/update_table_files", "/api/update_progress", "/api/stm32_log",
        "/api/overlay_icons", "/api/fs_info", "/api/fs_list", "/api/fs_show", "/api/ir_codes_get" };
    const char * immer[] = { "/api/ldr_min_set", "/api/display_power_set", "/api/ambilight_power_set", "/api/maintenance_reset_stm32", "/api/fs_remove",
        "/api/weather_get_now", "/api/weather_get_forecast", "/api/network_get_time", "/api/ir_codes_request", "/api/overlay_display",
        "/api/temperature_display", "/api/dfplayer_play", "/api/stm32_log_clear", "/api/live_display_color", "/api/overlay_delete",
        "/api/datetime_set", "/api/device_ready", "/api/reconnect_probe", "/api/app_file_upload", "/api/update_download_assets",
        "/api/remote_esp_update", "/api/local_esp_restart", "/api/settings_xml_x", "/api/settings_xm", "/app/app.js", "/", "/network" };
    api_orphan_skipped = 0;
    int n = 0;
    for (const char * p : liste)
    {
        int vor = api_orphan_skipped; bool lief = lauf (p, false); n++;
        fall ((std::string ("O1 verwaist, lesend ") + p + ": nicht ausgefuehrt, nichts gesendet, +1").c_str (),
              ! lief && out.empty () && api_orphan_skipped == vor + 1, std::string (lief ? "lief" : "-") + " out=" + out + " z=" + std::to_string (api_orphan_skipped));
    }
    fall ("O1 Zaehler gleich der Zahl der Faelle (16)", api_orphan_skipped == 16 && n == 16, std::to_string (api_orphan_skipped));
    for (const char * p : liste)
        fall ((std::string ("O2 verbunden, lesend ") + p + ": ausgefuehrt").c_str (), lauf (p, true), "nicht ausgefuehrt");
    fall ("O2 verbunden: Zaehler unveraendert", api_orphan_skipped == 16, std::to_string (api_orphan_skipped));
    for (const char * p : immer)
    {
        connected_aufrufe = 0;
        fall ((std::string ("O3 verwaist, nicht gelistet ") + p + ": IMMER ausgefuehrt (C53)").c_str (), lauf (p, false), "uebersprungen");
        fall ((std::string ("O4 nicht gelistet ") + p + ": connected () gar nicht gefragt").c_str (), connected_aufrufe == 0, std::to_string (connected_aufrufe));
    }
    fall ("O3 Zaehler unveraendert", api_orphan_skipped == 16, std::to_string (api_orphan_skipped));
    /* Review E4.10 M1: update_status MIT refresh ist ein Ausloeser (E4.5a) - nie uebersprungen; ohne oder mit 0 lesend */
    {
        int vor = api_orphan_skipped;
        params["refresh"] = "1";
        fall ("O6 verwaist update_status?refresh=1: ausgefuehrt (Ausloeser)", lauf ("/api/update_status", false) && api_orphan_skipped == vor, std::to_string (api_orphan_skipped - vor));
        params["refresh"] = "0";
        fall ("O6 verwaist update_status?refresh=0: uebersprungen", ! lauf ("/api/update_status", false) && api_orphan_skipped == vor + 1, std::to_string (api_orphan_skipped - vor));
        params.clear ();
        fall ("O6 verwaist update_status ohne refresh: uebersprungen", ! lauf ("/api/update_status", false) && api_orphan_skipped == vor + 2, std::to_string (api_orphan_skipped - vor));
        params["refresh"] = "1";
        fall ("O6 refresh wirkt nur bei update_status (fs_list bleibt lesend)", ! lauf ("/api/fs_list", false), "ausgefuehrt");
        params.clear ();
    }
    api_orphan_skipped = 0xFFFE; lauf ("/api/fs_list", false); lauf ("/api/fs_list", false);
    fall ("O5 saettigt bei 65535", api_orphan_skipped == 0xFFFF, std::to_string (api_orphan_skipped));
    return ende ("c65o", BEZEICHNUNG);
}
