/* Gemeinsame Attrappe fuer c65m und c65n (Paket 2026-10-13, E3.4/E3.4a, design.md 18.5, 18.5a, 18.7).
 *
 * Zwei Staende in EINEM Programm: namespace alt = ESP 3.2.35 (Tag 8445a10, Referenz fuer AKR.6 und die
 * Byte-Gleichheit), namespace neu = der gepruefte Baum. Beide sehen dieselben Attrappen: Update-Server
 * (httpclient ()), LittleFS im Speicher, WLAN, Variablen. Laeuft die Gegenprobe, ist neu = 3.2.35.
 */
#include "basis.h"
#include VARSHDR
#include "neu_have.inc"
#include "alt_have.inc"
#include "neu_defs.inc"
#include "alt_defs.inc"
#define ESP_VERSION "3.2.99"
static void sdk_abgabe (void) {}

/* ---- Variablen und Hardware */
static char sv_buf[MAX_STR_VARIABLES][80];
static STR_VAR sv[MAX_STR_VARIABLES];
STR_VAR * get_strvar (STR_VARIABLE i) { for (int k = 0; k < MAX_STR_VARIABLES; k++) { sv[k].str = sv_buf[k]; sv[k].maxlen = 79; } return &sv[i]; }
static void setze (STR_VARIABLE i, const char * v) { snprintf (sv_buf[i], sizeof sv_buf[i], "%s", v); }
static uint16_t hardware_configuration = HW_WC_24H;
#define WL_CONNECTED 3
struct WiFiA { int st = WL_CONNECTED; int status () { return st; } } WiFi;
static int wifi_ap_mode = 0;

/* ---- Antwort */
static std::string out;
static std::map<std::string, std::string> params;
void http_send (const char * s) { out += s; }
static void http_flush (void) {}
static char * http_get_param (const char * n) { static char b[8][300]; static int i; char * p = b[i++ % 8]; snprintf (p, 300, "%s", params.count (n) ? params[n].c_str () : ""); return p; }
static void http_send_json_escaped (const char * s)
{
    for (; *s; s++) { char c = *s; if (c == '"' || c == '\\') { out += '\\'; out += c; } else if ((unsigned char) c < 0x20) { char b[8]; snprintf (b, 8, "\\u%04x", c); out += b; } else out += c; }
}

/* ---- Stubs fuer update_status (die Feldnamen stehen IN update_status, die Werte sind hier gleichgueltig) */
static const char * http_get_local_update_message (uint32_t) { return "m"; }
static const char * http_get_fs_upload_filename (int) { return "f"; }
static const char * http_get_tables_family_prefix (void) { return "wc24h-tables-"; }
static const char * http_get_asset_prefix (void) { return "wc24h-"; }
static const char * http_get_hardware_label (void) { return "h"; }
static const char * http_get_processor_label (void) { return "p"; }
static const char * http_get_board_label (void) { return "b"; }
static const char * http_get_frequency_label (void) { return "q"; }
static const char * http_get_oscillator_label (void) { return "o"; }
static const char * http_get_display_label (void) { return "d"; }
static const char * http_get_display_led_mode (void) { return "rgbw"; }
static const char * http_get_default_layout_preview_filename (void) { return "l"; }
static unsigned int http_get_default_layout_columns (void) { return 11; }
static char aktuelle_tabelle[40] = "wc24h-tables-de.txt";
static char * tables_fname (void) { return aktuelle_tabelle; }
static void http_build_stm32_default_filename (char * b, size_t n, const char ** filter)
{
    *filter = (const char *) NULL; b[0] = 0;
    if (hardware_configuration == 0xFFFF) return;
    snprintf (b, n, "wc24h-ws2812-f411.hex"); *filter = "wc24h-";
}
static uint_fast8_t http_remote_stm32_filename_matches (const char * f)
{
    return hardware_configuration != 0xFFFF && ! strncmp (f, "wc24h-", 6);
}

/* ---- Update-Server: httpclient () und Lesen, mit Zaehlern */
struct Remote { bool da = true; std::string inhalt; long abbruch_nach = -1; unsigned long kosten = 0; };
static std::map<std::string, Remote> server;
static std::map<std::string, int> abrufe;
static std::vector<std::string> gefragte_hosts;
static int abrufe_gesamt = 0;
static const Remote * akt = nullptr; static size_t akt_pos = 0;
int httpclient (const char * host, const char * path, const char * file)
{
    gefragte_hosts.push_back (host); abrufe[file]++; abrufe_gesamt++;
    auto it = server.find (file);
    if (it == server.end () || ! it->second.da) { fake_ms += it == server.end () ? 0 : it->second.kosten; akt = nullptr; return -1; }
    fake_ms += it->second.kosten; akt = &it->second; akt_pos = 0;
    return (int) it->second.inhalt.size ();
}
int httpclient_read (int * lenp)
{
    if (*lenp <= 0 || ! akt) return -1;
    if (akt->abbruch_nach >= 0 && (long) akt_pos >= akt->abbruch_nach) return -1;
    int ch = (unsigned char) akt->inhalt[akt_pos++]; (*lenp)--; return ch;
}
void httpclient_stop (void) { akt = nullptr; }

/* ---- LittleFS im Speicher: Platz in Bloecken, Schreibfehler einspielbar, Zaehler fuer Schreibvorgaenge */
struct FSInfo { size_t totalBytes, usedBytes, blockSize, pageSize, maxOpenFiles, maxPathLength; };
struct FsN;
struct FileN
{
    std::string * d = nullptr; size_t pos = 0; bool schreibend = false;
    explicit operator bool () const { return d != nullptr; }
    int read () { if (! d || pos >= d->size ()) return -1; return (unsigned char) (*d)[pos++]; }
    int read (uint8_t * b, size_t n) { if (! d) return -1; size_t k = std::min (n, d->size () - pos); memcpy (b, d->data () + pos, k); pos += k; return (int) k; }
    int available () { return d && pos < d->size () ? (int) (d->size () - pos) : 0; }
    size_t write (const uint8_t * p, size_t n);
    size_t size () const { return d ? d->size () : 0; }
    void close () { d = nullptr; }
};
struct FsN
{
    std::map<std::string, std::string> f; size_t total = 655360, block = 8192;
    long schreibbudget = -1;                            /* Byte bis zum Schreibfehler, -1 = unbegrenzt */
    int oeffnen_w = 0, umbenennen = 0, entfernen = 0, gemountet = 0;
    bool begin () { gemountet++; return true; }
    void end () { gemountet--; }
    bool exists (const char * n) { return f.count (n) > 0; }
    FileN open (const char * n, const char * mode)
    {
        FileN x;
        if (mode[0] == 'w') { f[n] = ""; oeffnen_w++; x.d = &f[n]; x.schreibend = true; }
        else if (f.count (n)) x.d = &f[n];
        return x;
    }
    bool remove (const char * n) { entfernen++; return f.erase (n) > 0; }
    bool rename (const char * a, const char * b) { if (! f.count (a)) return false; umbenennen++; std::string t = f[a]; f.erase (a); f[b] = t; return true; }
    size_t belegt () { size_t u = 2 * block; for (auto & e : f) u += std::max<size_t> (1, (e.second.size () + block - 1) / block) * block; return u; }
    bool info (FSInfo & i) { i = FSInfo { total, belegt (), block, 256, 5, 32 }; return true; }
} LittleFS;
size_t FileN::write (const uint8_t * p, size_t n)
{
    size_t k = n;
    if (LittleFS.schreibbudget >= 0) { k = std::min<size_t> (n, (size_t) LittleFS.schreibbudget); LittleFS.schreibbudget -= (long) k; }
    if (LittleFS.belegt () + k > LittleFS.total) k = 0;
    d->append ((const char *) p, k); return k;
}
typedef FileN File;
static unsigned char http_download_buf[1024];

/* ---- beide Staende */
#define UPD_STAND(NS, PX)                                                                                             \
namespace NS {                                                                                                         \
    static int  http_fetch_remote_line (const char *, const char *, const char *, char *, size_t);                    \
    static void http_json_send_remote_update_support_fields ();                                                        \
    static void http_json_send_remote_update_url_fields ();                                                            \
    static bool http_fs_file_exists_and_nonempty (const char *);                                                      \
}
UPD_STAND (neu, neu_)
UPD_STAND (alt, alt_)
namespace neu {
#include "neu_upd_helpers.inc"
#include "neu_fsexists.inc"
#include "neu_i3.inc"
#include "neu_upd_text.inc"
#include "neu_upd_live_notes.inc"
#include "neu_upd_base.inc"
#include "neu_upd_cache.inc"
#include "neu_upd_status.inc"
#include "neu_upd_tables.inc"
#ifndef NEU_HAVE_CACHE
void update_cache_poll (void) {}
#endif
#ifndef NEU_HAVE_I3
static unsigned long http_last_request_millis = 0; static uint_fast8_t http_request_seen = 0;
#endif
}
namespace alt {
#include "alt_upd_helpers.inc"
#include "alt_fsexists.inc"
#include "alt_upd_text.inc"
#include "alt_upd_live_notes.inc"
#include "alt_upd_base.inc"
#include "alt_upd_cache.inc"
#include "alt_upd_status.inc"
#include "alt_upd_tables.inc"
}

/* ---- Hilfen */
static std::string antwort_neu_status (void) { out.clear (); neu::http_api_update_status (); return out; }
static std::string antwort_alt_status (void) { out.clear (); alt::http_api_update_status (); return out; }
static std::string antwort_neu_tabellen (void) { out.clear (); neu::http_api_update_table_files (); return out; }
static std::string antwort_alt_tabellen (void) { out.clear (); alt::http_api_update_table_files (); return out; }
static std::string koerper (const std::string & a) { size_t p = a.find ("\r\n\r\n"); return p == std::string::npos ? a : a.substr (p + 4); }

/* Schluessel und Typen eines flachen JSON-Objekts der Reihe nach: "name:typ" (s, n, b, a) */
static std::vector<std::string> schema (const std::string & antwort)
{
    std::string j = koerper (antwort); std::vector<std::string> r; size_t i = j.find ('{') + 1;
    while (i < j.size () && j[i] != '}')
    {
        if (j[i] == ',') { i++; continue; }
        size_t k1 = j.find ('"', i), k2 = j.find ('"', k1 + 1); std::string key = j.substr (k1 + 1, k2 - k1 - 1);
        i = k2 + 2; char t; 
        if (j[i] == '"') { t = 's'; i++; while (j[i] != '"') { if (j[i] == '\\') i++; i++; } i++; }
        else if (j[i] == '[') { t = 'a'; int d = 0; do { if (j[i] == '[') d++; else if (j[i] == ']') d--; else if (j[i] == '"') { i++; while (j[i] != '"') { if (j[i] == '\\') i++; i++; } } i++; } while (d > 0); }
        else if (j[i] == 't' || j[i] == 'f') { t = 'b'; i = j.find_first_of (",}", i); }
        else { t = 'n'; i = j.find_first_of (",}", i); }
        r.push_back (key + ":" + t);
    }
    return r;
}
static std::string feldwert (const std::string & antwort, const char * key)
{
    std::string j = koerper (antwort); std::string k = std::string ("\"") + key + "\":"; size_t p = j.find (k);
    if (p == std::string::npos) return "(fehlt)";
    p += k.size ();
    if (j[p] == '"') { size_t e = p + 1; while (j[e] != '"') { if (j[e] == '\\') e++; e++; } return j.substr (p + 1, e - p - 1); }
    if (j[p] == '[') { size_t e = j.find (']', p); return j.substr (p, e - p + 1); }
    return j.substr (p, j.find_first_of (",}", p) - p);
}
static void server_stand (const char * esp, const char * app, const char * wc, const std::string & notes, const std::string & liste, const std::string & tabellen)
{
    server.clear ();
    server["ESP-WordClock.txt"].inhalt = std::string (esp) + "\n";
    server["app-version.txt"].inhalt = std::string (app) + "\n";
    server["wc.txt"].inhalt = std::string (wc) + "\n";
    server["wc24h-icon.txt"].inhalt = "i\n"; server["wc24h-weather.txt"].inhalt = "w\n";
    server["wc12h-icon.txt"].inhalt = "i\n"; server["wc12h-weather.txt"].inhalt = "w\n";
    server["releasenote.html"].inhalt = notes;
    server["wc-list.txt"].inhalt = liste;
    server["wc-list-tables.txt"].inhalt = tabellen;
}
static void zaehler_null (void) { abrufe.clear (); abrufe_gesamt = 0; gefragte_hosts.clear (); LittleFS.oeffnen_w = LittleFS.umbenennen = LittleFS.entfernen = 0; Serial.lines.clear (); ring.clear (); }
