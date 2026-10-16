/* Pruefstand c65k (Paket 2026-10-13, E4.3; AKV.5; design-e4.md 19.5).
 * Echter Code: http_backlog_level/_want/_track (), http_server_begin (), http_server_loop () (ganz).
 * Attrappen wie c65i; ServerA zeichnet backlog () und accept () in ihrer Reihenfolge auf. getFreeHeap () ist ein
 * Zaehlerstand (ESP.heap). Statisch (c65k_static.py): Unterklasse, begin (80, HTTP_BACKLOG_HIGH), #error.
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
    uint16_t remotePort () { return 0; }                    /* R1.1 I-5 */
};
static int s_pending = 0;
static std::vector<std::string> ereignisse; static std::vector<int> gesetzt;
struct ServerA { bool bereit = false; bool hasClient () const { return bereit; } uint8_t unclaimed () const { return 0; }
                 const void * listen_arg () const { return nullptr; }                       /* R1.1 I-5; c72i prueft sie */
                 ClientA accept () { ereignisse.push_back ("accept"); ClientA c; c.valid = bereit; bereit = false; return c; }
                 uint8_t pending () const { return (uint8_t) s_pending; }
                 void backlog (uint8_t n) { ereignisse.push_back ("backlog"); gesetzt.push_back (n); }
                 int begin_port = -1, begin_backlog = -1;
                 void begin (uint16_t p, uint8_t b) { begin_port = p; begin_backlog = b; }
                 void begin (void) { begin_port = 80; begin_backlog = 5; } } http_server;
static ClientA http_client;
#include "i4.inc"
#ifndef HAVE_I4
static void heap_low_snapshot (void) {}
#endif
#include "i5.inc"                                           /* R1.1 I-5 (Paket 2026-10-15): tcp_now_*, http_accept_dead */

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
struct HClient { bool ok = true; bool connect (const char *, int) { return ok; } void print (const String &) {} void stop () {} } client;
static unsigned int httpclient_grace_left;
#define HTTPCLIENT_PEER_GONE_GRACE 300
static bool httpclient_wait_for_data (void) { return false; }
int httpclient_read_header (int *) { return -1; }
static void httpclient_stop (void) { client.stop (); }             /* R1b L7: Freigabe des Kontexts prueft c71e */
#include "client.inc"
#ifndef HAVE_CONNECTS
uint16_t httpclient_connects_get (void) { return 0; }
#endif
#include "device_ready.inc"
#include "server_loop.inc"

#include "begin.inc"

static void heap (uint32_t f) { ESP.heap = f; ESP.heap_seq.clear (); }
#ifdef HAVE_I4
static int schritt (uint32_t f) { heap (f); http_backlog_track (); return http_backlog; }
#endif

int main (void)
{
    printf ("c65k (%s)\n", BEZEICHNUNG);
#ifndef HAVE_I4
    fall ("Backlog nach Heap vorhanden", false, "Auszug ohne http_backlog_track () - Stand vor E4");
    http_server_begin ();
    fall ("Start mit Backlog 3", http_server.begin_backlog == 3, std::to_string (http_server.begin_backlog));
#else
    http_server_begin ();
    fall ("K1 Start: begin (80, 3), Zustand 3", http_server.begin_port == 80 && http_server.begin_backlog == 3 && http_backlog == 3,
          std::to_string (http_server.begin_port) + "/" + std::to_string (http_server.begin_backlog) + "/" + std::to_string (http_backlog));
    /* absteigend: sofort */
    struct { uint32_t f; int soll; } ab[] = { { 7000, 3 }, { 5900, 3 }, { 5899, 2 }, { 5100, 2 }, { 5099, 1 }, { 0, 1 } };
    gesetzt.clear ();
    for (auto & a : ab) { int b = schritt (a.f); fall (("K2 absteigend free=" + std::to_string (a.f)).c_str (), b == a.soll, std::to_string (b)); }
    fall ("K2 absteigend: genau zwei Setzvorgaenge (2, 1)", gesetzt == std::vector<int> ({ 2, 1 }), std::to_string (gesetzt.size ()));
    /* aufsteigend: erst ab Schwelle + 256 */
    struct { uint32_t f; int soll; } auf[] = { { 5100, 1 }, { 5355, 1 }, { 5356, 2 }, { 5900, 2 }, { 6155, 2 }, { 6156, 3 }, { 9000, 3 } };
    gesetzt.clear ();
    for (auto & a : auf) { int b = schritt (a.f); fall (("K3 aufsteigend free=" + std::to_string (a.f)).c_str (), b == a.soll, std::to_string (b)); }
    fall ("K3 aufsteigend: genau zwei Setzvorgaenge (2, 3)", gesetzt == std::vector<int> ({ 2, 3 }), std::to_string (gesetzt.size ()));
    gesetzt.clear (); schritt (1000); schritt (7000);
    fall ("K4 Sprung 1 -> 3 in einem Schritt", gesetzt == std::vector<int> ({ 1, 3 }), std::to_string (gesetzt.size ()));
    /* Pendeln um die Schwellen: die Hysterese verhindert es */
    struct { const char * name; uint32_t a, b; int start; size_t soll; } pd[] = {
        { "um 5100 ab 1", 5050, 5150, 1, 0 }, { "um 5900 ab 2", 5850, 5950, 2, 0 }, { "um 5100 ab 2", 5090, 5110, 2, 1 }, { "um 5900 ab 3", 5890, 5910, 3, 1 } };
    for (auto & p : pd)
    {
        schritt (p.start == 1 ? 0 : p.start == 2 ? 5500 : 9000); if (p.start == 1) schritt (0);
        if (p.start == 2) { schritt (0); schritt (5400); }
        gesetzt.clear ();
        for (int i = 0; i < 20; i++) { schritt (p.a); schritt (p.b); }
        fall ((std::string ("K5 Pendeln ") + p.name + ": Setzvorgaenge").c_str (), gesetzt.size () == p.soll, std::to_string (gesetzt.size ()) + " statt " + std::to_string (p.soll));
    }
    gesetzt.clear (); for (int i = 0; i < 100; i++) schritt (6500);
    fall ("K6 gleicher Heap: nur einmal gesetzt", gesetzt.size () <= 1, std::to_string (gesetzt.size ()));
    gesetzt.clear (); for (int i = 0; i < 100; i++) schritt (6500);
    fall ("K6 danach nie wieder gesetzt", gesetzt.empty (), std::to_string (gesetzt.size ()));
    /* in http_server_loop (): Nachfuehrung VOR accept (), auch ohne Verbindung */
    schritt (9000); ereignisse.clear (); heap (5000);
    http_server.bereit = false; http_server_loop ();
    fall ("K7 ohne Verbindung: nachgefuehrt, vor accept ()", ereignisse == std::vector<std::string> ({ "backlog", "accept" }), ereignisse.empty () ? "-" : ereignisse[0]);
    ereignisse.clear (); heap (9000);
    Szenario s; s.daten = true; s.lese = 1; s.zeile = "GET /api/settings_xml HTTP/1.1"; sz = s;
    http_server.bereit = true; http_write_broken = 0; http_server_loop ();
    fall ("K8 mit Verbindung: backlog () vor accept (), danach kein zweites", ereignisse.size () == 2 && ereignisse[0] == "backlog" && ereignisse[1] == "accept",
          std::to_string (ereignisse.size ()));
    fall ("K9 Zustand nach Heap 9000 wieder 3", http_backlog == 3, std::to_string (http_backlog));
#endif
    return ende ("c65k", BEZEICHNUNG);
}
