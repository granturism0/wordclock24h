/* Pruefstand c65i (Paket 2026-10-13, E4.1; AKV.1, AKV.2; design-e4.md 19.2).
 * Echter Code: heap_low_note () samt heap_low_snapshot (), http_api_device_ready (), HttpRequestTimer,
 * http_server_loop () (ganz), httpclient () (httpclient.cpp), http_api_network_scan ().
 * Attrappen: lwIP-Listen (lwip_attrappe.h, TCP_TW_LIMIT woertlich aus dem Core), WiFiServer/WiFiClient wie c65a,
 * WiFi-Scan, ESP-Wertfolgen. WiFiClient::status () gibt nicht ab (WiFiClient.cpp:335-340), deshalb hier ohne
 * Abgabepunkt; availableForWrite () ist tcp_sndbuf (), ohne Verbindung 0 (ClientContext.h:161-164).
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
struct ServerA { bool bereit = false; bool hasClient () const { return bereit; } uint8_t unclaimed () const { return 0; }
                 const void * listen_arg () const { return nullptr; }                       /* R1.1 I-5; c72i prueft sie */
                 ClientA accept () { ClientA c; c.valid = bereit; bereit = false; return c; }
                 uint8_t pending () const { return (uint8_t) s_pending; } void backlog (uint8_t) {} } http_server;
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
static void http_clear_request_if_none_match (void) {}             /* R2 C70: Gegenstand von c70e */
static void http_capture_request_if_none_match (const String &) {}
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

/* ---- WiFi-Scan */
struct bss_info { uint8_t ssid[32]; int8_t rssi; };
#define HTTP_ESCAPE_SCAN 2
static char wifi_ssid[33] = "heim", wifi_ip_address[16] = "192.0.2.1"; static int wifi_ap_mode = 0;
static size_t http_escape_text (const char * s, unsigned int n, char * o, size_t m, uint_fast8_t, unsigned int *) { size_t k = std::min<size_t> (n, m - 1); memcpy (o, s, k); o[k] = 0; return k; }
struct WiFiS
{
    int netze = 3; unsigned long dauer = 2200; uint32_t last = 960, last_blk = 900; bool liste = false;
    uint32_t heap_nach = 0, heap_bei_delete = 0, blk_bei_delete = 0; std::vector<std::string> folge;
    bss_info info[40];
    int8_t scanNetworks () { folge.push_back ("scan"); fake_ms += dauer; liste = true; if (ESP.heap != 4294967295u) { ESP.heap -= last; ESP.blk -= last_blk; } heap_nach = ESP.heap; return (int8_t) netze; }
    const bss_info * getScanInfoByIndex (int i) { if (! liste || i >= 40) return nullptr; snprintf ((char *) info[i].ssid, 32, "netz%d", i); info[i].rssi = -50; return &info[i]; }
    void scanDelete () { folge.push_back ("delete"); heap_bei_delete = ESP.heap; blk_bei_delete = ESP.blk; if (liste && ESP.heap != 4294967295u) { ESP.heap += last; ESP.blk += last_blk; } liste = false; }
} WiFi;
#include "scan.inc"

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
    printf ("c65i (%s)\n", BEZEICHNUNG);
    std::map<std::string, std::string> m;
#ifndef HAVE_I4
    fall ("I-4 vorhanden (heap_low_snapshot, Felder)", false, "Auszug ohne I-4 - Stand vor E4");
#else
    /* --- A: Momentaufnahme nur beim NEUEN Block-Tiefstwert (strikt kleiner) */
    {
        dr ("1");
        String p1 ("/api/update_status?refresh=1");
        lwip_setzen ({ ESTABLISHED, ESTABLISHED, CLOSE_WAIT, LAST_ACK, FIN_WAIT_2, SYN_RCVD }, 3);
        s_pending = 2; http_backlog = 2; c_status = ESTABLISHED; c_avail = 1072 - 400; http_cur_req = &p1;
        note (HEAP_LOW_PATH_FLUSH, 3000, 6000);
        m = dr ();
        std::string erst = "2|2|[0,0,0,1,2,0,1,1,0,1,0]|3|400|/api/update_status";
        fall ("A1 Momentaufnahme beim ersten Tiefstwert", i4 (m) == erst, i4 (m));
        String p2 ("/api/settings_xml");
        lwip_setzen ({ ESTABLISHED }, 5); s_pending = 4; http_backlog = 1; c_avail = 1072; http_cur_req = &p2;
        note (HEAP_LOW_PATH_FS, 3000, 5000);                 /* gleich */
        m = dr ();
        fall ("A2 Gleichstand: unveraendert", i4 (m) == erst, i4 (m));
        note (HEAP_LOW_PATH_ACCEPT, 4000, 3000);             /* hoeher, freier Heap tiefer */
        m = dr ();
        fall ("A3 hoeher (und heap_low_free tiefer): unveraendert", i4 (m) == erst && m["heap_low_free"] == "3000", i4 (m) + " free " + m["heap_low_free"]);
        heap_line_low = 5000;                                /* nur heap_line_low faellt */
        note (HEAP_LOW_PATH_FLUSH, 3500, 9000);
        m = dr ();
        fall ("A4 nur low= der Heap-Zeile tiefer: unveraendert", i4 (m) == erst, i4 (m));
        note (HEAP_LOW_PATH_CLIENT, 2999, 9000);             /* strikt kleiner */
        m = dr ();
        fall ("A5 neuer Tiefstwert: neue Momentaufnahme", i4 (m) == "4|1|[0,0,0,0,1,0,0,0,0,0,0]|5|0|/api/settings_xml", i4 (m));
        fall ("A6 Messpunkt und Block zur Momentaufnahme", m["heap_low_block"] == "2999" && m["heap_low_path"] == "client", m["heap_low_block"] + "/" + m["heap_low_path"]);
        http_cur_req = nullptr;
    }
    /* --- B: Pfad ohne Query, 31 Zeichen, "-" ohne Anfrage; unacked -1 ohne Verbindung */
    {
        struct { const char * pfad; bool null; const char * soll; } F[] = {
            { "", true, "-" }, { "", false, "-" }, { "/api/network_client_set?ssid=geheim&key=x", false, "/api/network_client_set" },
            { "/api/abcdefghijklmnopqrstuvwxyz0123456789", false, "/api/abcdefghijklmnopqrstuvwxyz" }, { "/x?", false, "/x" }, { "?a", false, "" } };
        uint32_t blk = 2900;
        for (auto & f : F)
        {
            String p (f.pfad); http_cur_req = f.null ? nullptr : &p;
            note (HEAP_LOW_PATH_FLUSH, blk--, 9000); m = dr ();
            fall ((std::string ("B Pfad '") + f.pfad + "'" + (f.null ? " (kein Zeiger)" : "")).c_str (), m["heap_low_req_path"] == f.soll,
                  "'" + m["heap_low_req_path"] + "' statt '" + f.soll + "'");
        }
        fall ("B Query nie in device_ready", out.find ("geheim") == std::string::npos, out);
        http_cur_req = nullptr;
        c_status = CLOSED; note (HEAP_LOW_PATH_FLUSH, blk--, 9000); m = dr ();
        fall ("B unacked -1 ohne Verbindung (status CLOSED)", m["heap_low_unacked"] == "-1", m["heap_low_unacked"]);
        c_status = ESTABLISHED; c_avail = 1072; note (HEAP_LOW_PATH_FLUSH, blk--, 9000); m = dr ();
        fall ("B unacked 0 bei leerem Sendepuffer", m["heap_low_unacked"] == "0", m["heap_low_unacked"]);
        c_avail = 0; note (HEAP_LOW_PATH_FLUSH, blk--, 9000); m = dr ();
        fall ("B unacked 1072 bei vollem Sendepuffer", m["heap_low_unacked"] == "1072", m["heap_low_unacked"]);
        c_avail = 1072;
    }
    /* --- C: Ruecksetzen mit I-1; http_backlog und api_orphan_skipped bleiben */
    {
        String p ("/api/fs_list"); http_cur_req = &p;
        lwip_setzen ({ ESTABLISHED, TIME_WAIT }, 2); s_pending = 1; http_backlog = 2;
        note (HEAP_LOW_PATH_FLUSH, 100, 9000); http_cur_req = nullptr;
        api_orphan_skipped = 7;
        m = dr ("1");
        fall ("C1 Ruecksetzen gibt die GELESENEN Werte aus", i4 (m) == "1|2|[0,0,0,0,1,0,0,0,0,0,1]|2|0|/api/fs_list", i4 (m));
        m = dr ();
        fall ("C2 nach dem Ruecksetzen: Zahlen 0, unacked -1, Pfad leer", i4 (m) == "0|0|[0,0,0,0,0,0,0,0,0,0,0]|0|-1|", i4 (m));
        fall ("C3 http_backlog und api_orphan_skipped bleiben", m["http_backlog"] == "2" && m["api_orphan_skipped"] == "7", m["http_backlog"] + "/" + m["api_orphan_skipped"]);
        m = dr ("0");
        fall ("C4 heap_low_reset=0 setzt nichts zurueck (heap_low_path leer = nichts gemessen)", m["heap_low_path"] == "" && m["heap_low_req_path"] == "", m["heap_low_path"]);
        api_orphan_skipped = 0;
    }
    /* --- D: Feldreihenfolge: E3 unveraendert, E4 dahinter */
    {
        dr ();
        std::vector<std::string> soll = { "ready", "free_heap", "max_free_block", "write_lost_bytes", "write_lost_blocks", "write_lost_gone",
            "write_lost_failed", "no_request_timeouts", "no_request_aborts", "update_cache_hits", "update_server_down_count", "heap_low_block",
            "heap_low_free", "heap_low_path", "heap_low_ms", "httpclient_connects", "fs_write_short", "req_max_ms", "req_max_path", "upd_cache_max_ms",
            "heap_low_accepts_pending", "heap_low_backlog", "heap_low_pcb_states", "heap_low_tw", "heap_low_unacked", "heap_low_req_path",
            "http_backlog", "api_orphan_skipped" };
        std::vector<std::string> ist = schluessel ();
        std::string w; for (auto & k : ist) w += k + " ";
        /* R1.1 I-5: dahinter folgen die I-5-Felder; deren Namen und Reihenfolge prueft c72i A8 */
        fall ("D1 Felder: E3 unveraendert in Name und Reihenfolge, E4 hinter upd_cache_max_ms (Anfang der Liste)",
              ist.size () >= soll.size () && std::equal (soll.begin (), soll.end (), ist.begin ()), w);
        m = dr ();
        std::string a = m["heap_low_pcb_states"]; int zahlen = a.size () > 2 ? 1 : 0; for (char c : a) if (c == ',') zahlen++;
        fall ("D2 heap_low_pcb_states ist ein Feld aus genau 11 Zahlen", a.front () == '[' && a.back () == ']' && zahlen == 11, a);
        fall ("D3 Antwort endet mit }", out.back () == '}', out.substr (out.size () - 20));
    }
    /* --- E: im echten http_server_loop (): Pfad waehrend des Handlers, "-" am Messpunkt accept, Zeiger danach geloescht */
    {
        dr ("1"); lwip_setzen ({ ESTABLISHED }, 0); c_status = ESTABLISHED;
        Szenario s; s.daten = true; s.lese = 1; s.zeile = "GET /api/network_client_set?ssid=geheim&key=schluessel HTTP/1.1";
        ESP.blk_seq.push_back (5000);                       /* Messpunkt accept */
        handler_blk = 4000;
        verbindung (s);
        m = dr ();
        fall ("E1 GET: Pfad der bedienten Anfrage ohne Query", m["heap_low_req_path"] == "/api/network_client_set" && m["heap_low_path"] == "flush", m["heap_low_req_path"] + " @" + m["heap_low_path"]);
        fall ("E2 Zeiger nach dem Verlassen geloescht", http_cur_req == nullptr, "nicht NULL");
        dr ("1");
        s.zeile = "POST /api/app_file_upload?filename=geheim.js HTTP/1.1"; ESP.blk_seq.push_back (5000); handler_blk = 3000;
        verbindung (s); m = dr ();
        fall ("E3 POST: Pfad ohne Query", m["heap_low_req_path"] == "/api/app_file_upload", m["heap_low_req_path"]);
        dr ("1");
        ESP.blk_seq.push_back (2000); handler_blk = 0;
        s.zeile = "GET /api/settings_xml HTTP/1.1"; verbindung (s); m = dr ();
        fall ("E4 Messpunkt accept vor dem Lesen: Pfad '-'", m["heap_low_req_path"] == "-" && m["heap_low_path"] == "accept", m["heap_low_req_path"] + " @" + m["heap_low_path"]);
        dr ("1"); client.ok = true; ESP.blk_seq.push_back (1500); httpclient ("h", "p", "f"); m = dr ();
        fall ("E5 Messpunkt client ausserhalb einer Anfrage: Pfad '-'", m["heap_low_req_path"] == "-" && m["heap_low_path"] == "client", m["heap_low_req_path"] + " @" + m["heap_low_path"]);
        handler_blk = 0;
    }
#endif
    /* --- F: Scan-Zeile (AKV.2) */
#ifndef HAVE_SCAN
    fall ("Scan-Zeile vorhanden", false, "Auszug ohne http_api_network_scan");
#else
    {
        Serial.lines.clear (); ring.clear (); out.clear ();
        ESP.heap = 7000; ESP.blk = 5000; ESP.blk_seq.clear (); ESP.heap_seq.clear ();
        WiFi.netze = 30; WiFi.dauer = 2200; flush_kosten = 0; WiFi.folge.clear ();
        fake_ms = 100000;
        /* Die Ausgabe laesst den Heap waehrend des Schreibens weiter sinken: http_flush () kostet hier 40 ms */
        flush_kosten = 40; flush_aufrufe = 0;
        http_api_network_scan ();
        int scan_zeilen = 0; std::string z;
        for (auto & l : Serial.lines) if (! l.compare (0, 7, "- scan ")) { scan_zeilen++; z = l; }
        char soll[160];
        snprintf (soll, sizeof (soll), "- scan n=30 vor=7000/5000 nach=%u/%u flush=%u/%u ms=2200/%lu", WiFi.heap_nach, 5000 - WiFi.last_blk,
                  WiFi.heap_bei_delete, WiFi.blk_bei_delete, (unsigned long) (flush_aufrufe * 40));
        fall ("F1 genau eine Scan-Zeile im Mitschnitt", scan_zeilen == 1, std::to_string (scan_zeilen));
        fall ("F2 Format und Werte (n roh vor der Kappung, vor/nach/flush, ms Scan/Ausgabe)", z == soll, "'" + z + "' statt '" + soll + "'");
        fall ("F3 flush vor scanDelete gemessen (Liste liegt noch)", z.find (" flush=" + std::to_string (WiFi.heap_nach)) != std::string::npos && ESP.heap == 7000, z);
        fall ("F4 Mitschnitt und Ring textgleich, je einmal", ring.size () == 1 && ring[0] == z, ring.empty () ? "(Ring leer)" : ring[0]);
        fall ("F5 Antwort auf 24 Netze gekappt", std::count (out.begin (), out.end (), '{') == 25, std::to_string (std::count (out.begin (), out.end (), '{')));
        fall ("F6 keine SSID in der Zeile", z.find ("netz") == std::string::npos && z.find ("heim") == std::string::npos, z);
        /* laengste Form */
        Serial.lines.clear (); ring.clear (); out.clear ();
        ESP.heap = 4294967295u; ESP.blk = 4294967295u; WiFi.netze = -128; WiFi.dauer = 4294967295ul; flush_kosten = 0; fake_ms = 0;
        http_api_network_scan ();
        z = ring.empty () ? "" : ring[0];
        fall ("F7 laengste Form <= 119 Zeichen und vollstaendig", ! z.empty () && z.size () <= 119 && z.find ("ms=4294967295/0") != std::string::npos && z.find ("n=-128 ") != std::string::npos,
              std::to_string (z.size ()) + ": " + z);
        ESP.heap = 6000; ESP.blk = 5000;
    }
#endif
    return ende ("c65i", BEZEICHNUNG);
}
