/* Pruefstand c65z (Paket 2026-10-13, E4.4; AKV.10; design-e4.md 19.7).
 * Echter Code: tcp_tw_limit () ueber das Core-Makro TCP_TW_LIMIT (in lwip_attrappe.h woertlich), http_server_loop ()
 * (ganz), httpclient () (httpclient.cpp), heap_low_note ()/heap_low_snapshot () als Zeuge am Messpunkt accept.
 * tcp_kill_timewait () bildet upstream lwIP 2.1 nach: das AELTESTE PCB in TIME_WAIT zuerst. Statisch (c65z_static.py):
 * genau zwei Aufrufstellen, privater lwIP-Kopf nur in http.cpp.
 */
#include "basis.h"
#include "lwip_attrappe.h"
#include "have.inc"
#include "defs.inc"
#ifndef HEAP_LOW_PATH_FLUSH
#define HEAP_LOW_PATH_FLUSH 1
#define HEAP_LOW_PATH_FS 2
#define HEAP_LOW_PATH_ACCEPT 3
#define HEAP_LOW_PATH_CLIENT 4
#endif
static void sdk_abgabe (void) {}

#include "i1.inc"

/* ---- JSON- und Antwort-Attrappen */
static std::string out;
static std::map<std::string, std::string> params;
static int flush_aufrufe = 0; static unsigned long flush_kosten = 0;
static void http_send (const char * s) { out += s; }
static void http_flush (void) { flush_aufrufe++; fake_ms += flush_kosten; }
static char * http_get_param (const char * n) { static char b[8][300]; static int i; char * p = b[i++ % 8]; snprintf (p, 300, "%s", params.count (n) ? params[n].c_str () : ""); return p; }
static void http_json_send_field_prefix (const char * k) { out += std::string (",\"") + k + "\":"; }
static void http_json_send_uint_field (const char * k, unsigned long v) { out += std::string (",\"") + k + "\":" + std::to_string (v); }
static void http_json_send_string_field (const char * k, const char * v) { out += std::string (",\"") + k + "\":\"" + v + "\""; }
static uint16_t update_cache_hits = 7, update_server_down_count = 3;

#define TCP_MSS 536
#include "zaehler.inc"
#ifndef HAVE_FS_WRITE_SHORT
static uint16_t http_fs_write_short = 0;
#endif

struct IPAttrappe { String toString () const { return String ("192.0.2.1"); } };
struct Szenario { bool daten = false, verbunden = true; int lese = 0; std::string zeile; unsigned long kosten_http = 0; };
static Szenario sz;
static int c_status = ESTABLISHED, c_avail = 1072;
struct ClientA
{
    bool valid = false;
    explicit operator bool () const { return valid; }
    int available () { return sz.daten ? 1 : 0; }
    uint8_t connected () { return sz.verbunden ? 1 : 0; }
    int read () { return -1; }
    int peek () { return -1; }
    void stop () {} void abort () {} void flush () {}
    void setNoDelay (bool) {} void setTimeout (unsigned long) {}
    IPAttrappe remoteIP () { return IPAttrappe (); }
    uint8_t status () { return (uint8_t) c_status; }
    int availableForWrite () { return c_status == CLOSED ? 0 : c_avail; }
};
static int s_pending = 0;
struct ServerA { bool bereit = false; ClientA accept () { ClientA c; c.valid = bereit; bereit = false; return c; }
                 uint8_t pending () const { return (uint8_t) s_pending; } void backlog (uint8_t) {} } http_server;
static ClientA http_client;
#include "i4.inc"
#ifndef HAVE_I4
static void heap_low_snapshot (void) {}
#endif

static void http_clear_request_user_agent (void) {}
static void http_clear_request_fetch_dest (void) {}
static bool http_read_request_line (String & line, unsigned long) { line = String (sz.zeile.c_str ()); return sz.lese != 0; }
static bool read_line (String &) { return false; }
static void http_capture_request_user_agent (const String &) {}
static void http_capture_request_fetch_dest (const String &) {}
static void http_log_request (const String &, const char *, const String &) {}
static const char * http_get_request_browser_label (void) { return ""; }
static uint_fast8_t http_api_query_too_long (const char *, size_t) { return 0; }
static void http_set_params (char *) {}
static uint32_t handler_blk = 0;            /* != 0: der Handler misst mit diesem Block (Messpunkt flush) */
static void handler (void) { fake_ms += sz.kosten_http; if (handler_blk) { ESP.blk_seq.push_back (handler_blk); heap_low_note (HEAP_LOW_PATH_FLUSH); } }
static void http_post (const String &) { handler (); }
static uint_fast8_t http (const char *, const char *) { handler (); return 0; }

#include "i3.inc"
#ifndef HAVE_I3
static uint32_t req_max_ms = 0; static char req_max_path[32] = "";
#endif
static int tw_beim_connect = -1;
struct HClient { bool ok = true; bool connect (const char *, int) { tw_beim_connect = tw_laenge (); return ok; } void print (const String &) {} void stop () {} } client;
static unsigned int httpclient_grace_left;
#define HTTPCLIENT_PEER_GONE_GRACE 300
static bool httpclient_wait_for_data (void) { return false; }
int httpclient_read_header (int *) { return -1; }
#include "client.inc"
#ifndef HAVE_CONNECTS
uint16_t httpclient_connects_get (void) { return 0; }
#endif
#include "device_ready.inc"
#include "server_loop.inc"

/* ---- Hilfen */
static std::map<std::string, std::string> dr (const char * reset = nullptr)
{
    std::map<std::string, std::string> m;
    params.clear (); if (reset) params["heap_low_reset"] = reset;
    out.clear (); http_api_device_ready ();
    size_t i = out.find ('{');
    while (i != std::string::npos && i < out.size ())
    {
        size_t k = out.find ('"', i); if (k == std::string::npos) break;
        size_t e = out.find ('"', k + 1); std::string key = out.substr (k + 1, e - k - 1);
        size_t v = e + 2; std::string val;
        if (out[v] == '"') { size_t ve = out.find ('"', v + 1); val = out.substr (v + 1, ve - v - 1); i = ve + 1; }
        else if (out[v] == '[') { size_t ve = out.find (']', v); val = out.substr (v, ve - v + 1); i = ve + 1; }
        else { size_t ve = out.find_first_of (",}", v); val = out.substr (v, ve - v); i = ve; }
        m[key] = val;
    }
    return m;
}
static std::vector<std::string> schluessel (void)
{
    std::vector<std::string> k; size_t i = 0;
    while ((i = out.find (",\"", i)) != std::string::npos) { size_t e = out.find ('"', i + 2); k.push_back (out.substr (i + 2, e - i - 2)); i = e; }
    return k;
}
static void note (uint8_t pfad, uint32_t blk, uint32_t heap) { ESP.blk_seq.push_back (blk); ESP.heap_seq.push_back (heap); heap_low_note (pfad); }
static void verbindung (const Szenario & s) { sz = s; http_server.bereit = true; http_write_broken = 0; http_server_loop (); }
static std::string i4 (std::map<std::string, std::string> & m)
{
    return m["heap_low_accepts_pending"] + "|" + m["heap_low_backlog"] + "|" + m["heap_low_pcb_states"] + "|" + m["heap_low_tw"] + "|" +
           m["heap_low_unacked"] + "|" + m["heap_low_req_path"];
}


int main (void)
{
    printf ("c65z (%s)\n", BEZEICHNUNG);
#ifndef HAVE_I4
    fall ("tcp_tw_limit () vorhanden", false, "Auszug ohne E4 - Stand vor E4");
    lwip_setzen ({}, 5); client.ok = true; httpclient ("h", "p", "f");
    fall ("vor connect () hoechstens 2 in TIME_WAIT", tw_beim_connect <= 2, std::to_string (tw_beim_connect));
#else
    for (int n = 0; n <= 5; n++)
    {
        lwip_setzen ({ ESTABLISHED }, n); tw_kills = 0;
        tcp_tw_limit ();
        std::vector<u32_t> soll; for (int a = n; a > 2; a--) soll.push_back ((u32_t) a);
        std::vector<u32_t> rest; for (struct tcp_pcb * p = tcp_tw_pcbs; p; p = p->next) rest.push_back (p->alter);
        std::vector<u32_t> rest_soll; for (int a = 1; a <= std::min (n, 2); a++) rest_soll.push_back ((u32_t) a);
        std::string nn = "Z1 " + std::to_string (n) + " in TIME_WAIT";
        fall ((nn + ": hoechstens 2 uebrig").c_str (), tw_laenge () == std::min (n, 2), std::to_string (tw_laenge ()));
        fall ((nn + ": die AELTESTEN entfernt, die juengsten bleiben").c_str (), tw_getoetet == soll && rest == rest_soll, std::to_string (tw_getoetet.size ()));
        fall ((nn + ": aktive Liste unberuehrt").c_str (), tcp_active_pcbs && tcp_active_pcbs->state == ESTABLISHED && ! tcp_active_pcbs->next, "");
    }
    /* Messpunkt accept sieht den Stand OHNE die abgeraeumten PCBs */
    dr ("1"); lwip_setzen ({ ESTABLISHED }, 5);
    ESP.blk_seq.push_back (1000);
    Szenario s; s.daten = true; s.lese = 1; s.zeile = "GET /api/settings_xml HTTP/1.1";
    verbindung (s);
    std::map<std::string, std::string> m = dr ();
    fall ("Z2 nach accept (): Messpunkt accept sieht hoechstens 2 in TIME_WAIT", m["heap_low_path"] == "accept" && m["heap_low_tw"] == "2",
          m["heap_low_path"] + " tw=" + m["heap_low_tw"]);
    lwip_setzen ({}, 5); http_server.bereit = false; http_server_loop ();
    fall ("Z3 ohne Verbindung wird nichts abgeraeumt", tw_laenge () == 5, std::to_string (tw_laenge ()));
    lwip_setzen ({}, 4); client.ok = true; tw_beim_connect = -1; httpclient ("h", "p", "f");
    fall ("Z4 httpclient (): unmittelbar vor connect () hoechstens 2", tw_beim_connect == 2, std::to_string (tw_beim_connect));
    lwip_setzen ({}, 4); client.ok = false; tw_beim_connect = -1; httpclient ("h", "p", "f");
    fall ("Z5 auch wenn connect () scheitert: vorher begrenzt", tw_beim_connect == 2, std::to_string (tw_beim_connect));
#endif
    return ende ("c65z", BEZEICHNUNG);
}
