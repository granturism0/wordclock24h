/* Pruefstand c65a (Paket 2026-10-13, E3.1/E3.3, AKR.1, AKR.3; design.md 18.2, 18.4).
 * Echter Code: heap_low_note (), heap_low_line_take (), heap_low_path_name (), http_api_device_ready (),
 * http_request_note (), HttpRequestTimer, http_server_loop () (ganz), httpclient () (httpclient.cpp),
 * esp_heap_log () (ESP-uclock.ino). Attrappen: ESP-Wertfolgen, millis (), WiFiServer/WiFiClient, die
 * Anfrage-Helfer von http_server_loop () und das JSON-Ausgeben.
 */
#include "basis.h"
#include "lwip_attrappe.h"                                  /* E4: Listen fuer I-4 und TIME_WAIT */
#include "have.inc"
#include "defs.inc"
#ifndef HEAP_LOW_PATH_FLUSH
#define HEAP_LOW_PATH_FLUSH 1
#define HEAP_LOW_PATH_FS 2
#define HEAP_LOW_PATH_ACCEPT 3
#define HEAP_LOW_PATH_CLIENT 4
#endif
static void sdk_abgabe (void) {}

/* ---- I-1 */
#include "i1.inc"
#ifndef HAVE_I1
static uint32_t heap_low_block = UINT32_MAX, heap_low_free = UINT32_MAX, heap_low_ms = 0, heap_line_low = UINT32_MAX;
static uint8_t heap_low_path = 0;
void heap_low_note (uint8_t) {}
uint32_t heap_low_line_take (void) { return UINT32_MAX; }
#endif

/* ---- JSON- und Antwort-Attrappen */
static std::string out;
static std::map<std::string, std::string> params;
static void http_send (const char * s) { out += s; }
static void http_flush (void) {}
static char * http_get_param (const char * n) { static char b[8][300]; static int i; char * p = b[i++ % 8]; snprintf (p, 300, "%s", params.count (n) ? params[n].c_str () : ""); return p; }
static void http_json_send_field_prefix (const char * k) { out += std::string (",\"") + k + "\":"; }   /* E4 */
static void http_json_send_uint_field (const char * k, unsigned long v) { out += std::string (",\"") + k + "\":" + std::to_string (v); }
static void http_json_send_string_field (const char * k, const char * v) { out += std::string (",\"") + k + "\":\"" + v + "\""; }
static uint16_t update_cache_hits = 7, update_server_down_count = 3;

/* ---- Zaehler, Verbindungsattrappen */
#define TCP_MSS 536
#include "zaehler.inc"
#ifndef HAVE_FS_WRITE_SHORT
static uint16_t http_fs_write_short = 0;
#endif

struct IPAttrappe { String toString () const { return String ("192.0.2.1"); } };
struct Szenario
{
    bool        daten = false;          /* available () > 0 */
    bool        verbunden = true;
    int         lese = 0;               /* http_read_request_line: 0 false, 1 true */
    std::string zeile;
    unsigned long kosten_lesen = 0, kosten_http = 0, kosten_stop = 0, kosten_abort = 0, kosten_flush = 0;
    bool        bricht = false;         /* http () setzt http_write_broken */
};
static Szenario sz;
static int abort_calls, stop_calls, timeout_set;
struct ClientA
{
    bool valid = false;
    explicit operator bool () const { return valid; }
    int available () { return sz.daten ? 1 : 0; }
    uint8_t connected () { return sz.verbunden ? 1 : 0; }
    int read () { return -1; }
    int peek () { return -1; }
    void stop () { stop_calls++; fake_ms += sz.kosten_stop; }
    void abort () { abort_calls++; fake_ms += sz.kosten_abort; }
    void flush () { fake_ms += sz.kosten_flush; }
    void setNoDelay (bool) {}
    void setTimeout (unsigned long t) { timeout_set = (int) t; }
    IPAttrappe remoteIP () { return IPAttrappe (); }
    uint8_t status () { return sz.verbunden ? ESTABLISHED : CLOSED; }      /* E4 I-4 */
    int availableForWrite () { return 1072; }
    uint16_t remotePort () { return 0; }                    /* R1.1 I-5 */
};
struct ServerA { bool bereit = false; bool hasClient () const { return bereit; } uint8_t unclaimed () const { return 0; }
                 const void * listen_arg () const { return nullptr; }                       /* R1.1 I-5; c72i prueft sie */
                 ClientA accept () { ClientA c; c.valid = bereit; bereit = false; return c; }
                 uint8_t pending () const { return 0; } void backlog (uint8_t) {} } http_server;   /* E4 */
static ClientA http_client;
#include "i4.inc"                                           /* E4: heap_low_snapshot (), Backlog, tcp_tw_limit () */
#ifndef HAVE_I4
static void heap_low_snapshot (void) {}
#endif
#include "i5.inc"                                           /* R1.1 I-5 (Paket 2026-10-15): tcp_now_*, http_accept_dead */

static void http_clear_request_user_agent (void) {}
static void http_clear_request_fetch_dest (void) {}
static bool http_read_request_line (String & line, unsigned long) { fake_ms += sz.kosten_lesen; line = String (sz.zeile.c_str ()); return sz.lese != 0; }
static bool read_line (String &) { return false; }
static void http_capture_request_user_agent (const String &) {}
static void http_capture_request_fetch_dest (const String &) {}
static void http_log_request (const String &, const char *, const String &) {}
static const char * http_get_request_browser_label (void) { return ""; }
static uint_fast8_t http_api_query_too_long (const char *, size_t) { return 0; }
static void http_set_params (char *) {}
static std::string letzter_pfad;
static void http_post (const String & p) { letzter_pfad = p.s; fake_ms += sz.kosten_http; if (sz.bricht) http_write_broken = 1; }
static uint_fast8_t http (const char * p, const char *) { letzter_pfad = p; fake_ms += sz.kosten_http; if (sz.bricht) http_write_broken = 1; return 0; }

/* ---- I-3 */
#include "i3.inc"
#ifndef HAVE_I3
static uint32_t req_max_ms = 0; static char req_max_path[32] = "";
#endif

/* ---- httpclient () aus httpclient.cpp */
struct HClient { bool ok = true; int connects = 0; bool connect (const char *, int) { connects++; return ok; } void print (const String &) {} void stop () {} } client;
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

/* ---- Heap-Zeile aus der .ino */
#define ESP_HEAP_LOG_INTERVAL 60000UL
#include "heaplog.inc"

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
        else { size_t ve = out.find_first_of (",}", v); val = out.substr (v, ve - v); i = ve; }
        m[key] = val;
    }
    return m;
}
static void note (uint8_t pfad, uint32_t blk, uint32_t heap, unsigned long ms) { fake_ms = ms; ESP.blk_seq.push_back (blk); ESP.heap_seq.push_back (heap); heap_low_note (pfad); }
static std::string heapzeile (void) { fake_ms += 60000; size_t n = ring.size (); esp_heap_log (); return ring.size () > n ? ring.back () : ""; }
static std::string feld (const std::string & z, const char * k) { size_t p = z.find (k); if (p == std::string::npos) return "(fehlt)"; p += strlen (k); return z.substr (p, z.find (' ', p) - p); }
static void verbindung (const Szenario & s) { sz = s; http_server.bereit = true; http_write_broken = 0; http_server_loop (); }

int main (void)
{
    printf ("c65a (%s)\n", BEZEICHNUNG);
    std::map<std::string, std::string> m;

    /* --- I-1: Minimum und Pfad ueber gemischte Folgen, Gleichstand, freier Heap fuer sich */
    dr ("1");
    note (HEAP_LOW_PATH_FLUSH,  3000, 6000, 100);
    note (HEAP_LOW_PATH_FS,     2000, 7000, 200);
    note (HEAP_LOW_PATH_ACCEPT, 2500, 4000, 300);
    note (HEAP_LOW_PATH_CLIENT, 2000, 5000, 400);     /* Gleichstand mit fs: fs bleibt */
    m = dr ();
    fall ("Minimum Block", m["heap_low_block"] == "2000", "heap_low_block=" + m["heap_low_block"]);
    fall ("Pfad des Minimums, Gleichstand bleibt beim ersten", m["heap_low_path"] == "fs", "heap_low_path=" + m["heap_low_path"]);
    fall ("Zeitpunkt des Minimums", m["heap_low_ms"] == "200", "heap_low_ms=" + m["heap_low_ms"]);
    fall ("free_heap fuer sich", m["heap_low_free"] == "4000", "heap_low_free=" + m["heap_low_free"]);
    note (HEAP_LOW_PATH_CLIENT, 1500, 9000, 500);
    m = dr ();
    fall ("neues Minimum ueber client", m["heap_low_block"] == "1500" && m["heap_low_path"] == "client" && m["heap_low_ms"] == "500",
          m["heap_low_block"] + "/" + m["heap_low_path"] + "/" + m["heap_low_ms"]);
    m = dr ();
    fall ("Lesen ohne Parameter setzt nichts zurueck", m["heap_low_block"] == "1500" && m["heap_low_path"] == "client", m["heap_low_block"]);
    m = dr ("0");
    fall ("heap_low_reset=0 setzt nichts zurueck", m["heap_low_block"] == "1500", m["heap_low_block"]);
    m = dr ("1");
    fall ("Ruecksetzen gibt die GELESENEN Werte aus", m["heap_low_block"] == "1500" && m["heap_low_path"] == "client", m["heap_low_block"]);
    m = dr ();
    fall ("nach dem Ruecksetzen leer", m["heap_low_block"] == "4294967295" && m["heap_low_path"] == "" && m["heap_low_free"] == "4294967295",
          m["heap_low_block"] + "/" + m["heap_low_path"]);

    /* --- Felder hinten, in der Reihenfolge der Spec */
    {
        dr ();
        const char * soll[] = { "update_server_down_count", "heap_low_block", "heap_low_free", "heap_low_path", "heap_low_ms",
                                "httpclient_connects", "fs_write_short", "req_max_ms", "req_max_path", "upd_cache_max_ms" };
        size_t pos = 0; bool ok = true; std::string w;
        for (const char * k : soll) { size_t p = out.find (std::string ("\"") + k + "\":"); if (p == std::string::npos || p < pos) { ok = false; w += k; w += " "; } else pos = p; }
        fall ("device_ready: Felder hinten, Reihenfolge", ok && out.compare (out.size () - 1, 1, "}") == 0, "falsch/fehlend: " + w);
        fall ("device_ready: bestehende Felder davor unveraendert", out.find ("\"write_lost_failed\":") < out.find ("\"update_cache_hits\":"), out.substr (0, 80));
    }

    /* --- Heap-Zeile: eigenes Minimum, unabhaengig vom Ruecksetzen ueber device_ready */
    {
        fake_ms = 1000000;
        dr ("1"); heapzeile ();                                  /* beide Fenster leer */
        ESP.blk = 6000;
        note (HEAP_LOW_PATH_FS, 1800, 6000, fake_ms + 10);
        ESP.blk = 6000;
        std::string z = heapzeile ();
        fall ("Heap-Zeile low= ist das Minimum seit der vorigen Zeile", feld (z, "low=") == "1800", z);
        fall ("Heap-Zeile: low= hinten, ovr= davor", z.find (" ovr=") != std::string::npos && z.find (" ovr=") < z.find (" low="), z);
        fall ("Heap-Zeile: Mitschnitt und Ring textgleich", ! Serial.lines.empty () && Serial.lines.back () == z, Serial.lines.empty () ? "" : Serial.lines.back ());
        m = dr ();
        fall ("Heap-Zeile setzt device_ready nicht zurueck", m["heap_low_block"] == "1800", m["heap_low_block"]);
        z = heapzeile ();
        fall ("Heap-Zeile ohne Messpunkt: low= gleich max=", feld (z, "low=") == feld (z, "max="), z);
        note (HEAP_LOW_PATH_FLUSH, 1700, 6000, fake_ms + 10);
        dr ("1");
        ESP.blk = 6000;
        z = heapzeile ();
        fall ("device_ready-Ruecksetzen laesst die Heap-Zeile unberuehrt", feld (z, "low=") == "1700", z);
        ESP.blk = 4294967295u; ESP.heap = 4294967295u;
#ifdef HAVE_I1
        esp_uart_overruns = 4294967295u;
#endif
        note (HEAP_LOW_PATH_FS, 4294967295u, 4294967295u, fake_ms + 10);
        z = heapzeile ();
        /* R1.1 I-5: dahinter darf " tcp=... nrt=..." stehen; dessen Form und die Gesamtlaenge prueft c72i (C1, C3) */
        fall ("laengste Heap-Zeile: low= ungekuerzt (67 Zeichen bis low=)", ! z.compare (0, 67, "- heap free=4294967295 max=4294967295 ovr=4294967295 low=4294967295") &&
              (z.size () == 67 || ! z.compare (67, 5, " tcp=")), z);
        ESP.blk = 5000; ESP.heap = 6000;
    }

    /* --- httpclient_connects: nur nach erfolgreichem connect, Messpunkt client, saettigend */
    {
        dr ("1");
        uint16_t vor = httpclient_connects_get ();
        client.ok = false; httpclient ("h", "p", "f");
        fall ("connect gescheitert: nicht gezaehlt", httpclient_connects_get () == vor, std::to_string (httpclient_connects_get ()));
        m = dr ();
        fall ("connect gescheitert: kein Messpunkt", m["heap_low_path"] == "", m["heap_low_path"]);
        client.ok = true; ESP.blk_seq.push_back (1234); httpclient ("h", "p", "f");
        fall ("connect erfolgreich: +1", httpclient_connects_get () == (uint16_t) (vor + 1), std::to_string (httpclient_connects_get ()));
        m = dr ();
        fall ("connect erfolgreich: Messpunkt client", m["heap_low_path"] == "client" && m["heap_low_block"] == "1234", m["heap_low_path"] + "/" + m["heap_low_block"]);
        for (int k = 0; k < 70000; k++) httpclient ("h", "p", "f");
        fall ("httpclient_connects saettigt bei 65535", httpclient_connects_get () == 65535, std::to_string (httpclient_connects_get ()));
        m = dr ("1");
        fall ("httpclient_connects wird nie zurueckgesetzt", dr ()["httpclient_connects"] == "65535", m["httpclient_connects"]);
    }

    /* --- I-3: alle fuenf Rueckkehrpfade, je allein nach dem Ruecksetzen */
    struct { const char * name; Szenario s; unsigned long soll; const char * pfad; } pfade[5];
    pfade[0] = { "keine Anfrage", {}, 0, "-" };
    pfade[0].s.daten = false; pfade[0].s.verbunden = false; pfade[0].s.kosten_stop = 11; pfade[0].soll = 11;
    pfade[1] = { "leere Anfrage (Lesen scheitert)", {}, 0, "-" };
    pfade[1].s.daten = true; pfade[1].s.lese = 0; pfade[1].s.kosten_lesen = 300; pfade[1].s.kosten_stop = 4; pfade[1].soll = 304;
    pfade[2] = { "leere Anfrage (leere Zeile)", {}, 0, "-" };
    pfade[2].s.daten = true; pfade[2].s.lese = 1; pfade[2].s.zeile = ""; pfade[2].s.kosten_lesen = 20; pfade[2].s.kosten_stop = 5; pfade[2].soll = 25;
    pfade[3] = { "Schreibabbruch (abort)", {}, 0, "/api/settings_xml" };
    pfade[3].s.daten = true; pfade[3].s.lese = 1; pfade[3].s.zeile = "GET /api/settings_xml?x=1 HTTP/1.1"; pfade[3].s.kosten_http = 1700;
    pfade[3].s.bricht = true; pfade[3].s.kosten_abort = 300; pfade[3].soll = 2000;
    pfade[4] = { "Regelfall (stop)", {}, 0, "/api/network_client_set" };
    pfade[4].s.daten = true; pfade[4].s.lese = 1; pfade[4].s.zeile = "GET /api/network_client_set?ssid=geheimnetz&key=geheimschluessel HTTP/1.1";
    pfade[4].s.kosten_http = 900; pfade[4].s.kosten_flush = 300; pfade[4].s.kosten_stop = 300; pfade[4].soll = 1500;
    for (auto & p : pfade)
    {
        dr ("1"); fake_ms = 100000;
        verbindung (p.s);
        m = dr ();
        fall (p.name, m["req_max_ms"] == std::to_string (p.soll) && m["req_max_path"] == p.pfad,
              "req_max_ms=" + m["req_max_ms"] + " (Soll " + std::to_string (p.soll) + "), req_max_path='" + m["req_max_path"] + "' (Soll '" + p.pfad + "')");
    }
    {
        dr ("1"); fake_ms = 200000;
        http_server.bereit = false; http_server_loop ();
        m = dr ();
        fall ("ohne Verbindung keine Messung", m["req_max_path"] == "" && m["req_max_ms"] == "0", m["req_max_path"] + "/" + m["req_max_ms"]);
        for (auto & p : pfade) verbindung (p.s);
        m = dr ();
        fall ("Maximum ueber alle fuenf Pfade", m["req_max_ms"] == "2000" && m["req_max_path"] == "/api/settings_xml", m["req_max_ms"] + " " + m["req_max_path"]);
        fall ("Query nie im Pfad", m["req_max_path"].find ('?') == std::string::npos && out.find ("geheim") == std::string::npos, out);
        Szenario lang = pfade[4].s; lang.kosten_http = 5000; verbindung (lang);
        m = dr ();
        fall ("Query nie im Pfad (laengste Anfrage mit Zugangsdaten)", m["req_max_path"] == "/api/network_client_set" && out.find ("geheim") == std::string::npos,
              m["req_max_path"]);
        Szenario gleich = pfade[3].s; gleich.kosten_http = 5600 - 300; verbindung (gleich);   /* gleich lang wie die vorige */
        m = dr ();
        fall ("Gleichstand: die erste bleibt", m["req_max_path"] == "/api/network_client_set", m["req_max_path"] + " " + m["req_max_ms"]);
        Szenario post = pfade[4].s; post.zeile = "POST /api/app_file_upload?filename=geheim.js HTTP/1.1"; post.kosten_http = 9000; verbindung (post);
        m = dr ();
        fall ("POST: Pfad ohne Query", m["req_max_path"] == "/api/app_file_upload", m["req_max_path"]);
        Szenario lp = pfade[4].s; lp.zeile = "GET /api/abcdefghijklmnopqrstuvwxyz0123456789?q=1 HTTP/1.1"; lp.kosten_http = 20000; verbindung (lp);
        m = dr ("1");
        fall ("Pfad auf 31 Zeichen gekuerzt", m["req_max_path"] == std::string ("/api/abcdefghijklmnopqrstuvwxyz0123456789").substr (0, 31), m["req_max_path"]);
        m = dr ();
        fall ("Ruecksetzen mit I-1: req_max leer", m["req_max_path"] == "" && m["req_max_ms"] == "0", m["req_max_path"]);
        req_max_upd_cache_ms = 4321; m = dr ();
        fall ("upd_cache_max_ms: gelesen, ohne Parameter nicht zurueckgesetzt", m["upd_cache_max_ms"] == "4321" && dr ()["upd_cache_max_ms"] == "4321", m["upd_cache_max_ms"]);
        m = dr ("1");
        fall ("upd_cache_max_ms: Ruecksetzen gibt den GELESENEN Wert aus", m["upd_cache_max_ms"] == "4321", m["upd_cache_max_ms"]);
        m = dr ();
        fall ("upd_cache_max_ms: nach dem Ruecksetzen 0, letztes Feld von E3 (E4 haengt dahinter an)", m["upd_cache_max_ms"] == "0" && (out.find ("\"upd_cache_max_ms\":0}") != std::string::npos || out.find ("\"upd_cache_max_ms\":0,\"heap_low_accepts_pending\":") != std::string::npos), out.substr (out.size () > 40 ? out.size () - 40 : 0));
        fall ("Sendezeitgrenze nach accept gesetzt (M-3)", timeout_set == 1000, std::to_string (timeout_set));
    }

    return ende ("c65a", BEZEICHNUNG);
}
