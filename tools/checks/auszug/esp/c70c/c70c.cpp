/* Pruefstand c70c (Paket 2026-10-15, R2b.1, C70 Variante (c); AKL.1, AKL.2, AKL.3; design.md 18.3-18.6).
 *
 * Gegenstand: Regel I-L. Eine Antwort auf ein App-Asset traegt "Cache-Control: public, max-age=31536000, immutable"
 * GENAU DANN, wenn die Marke h nicht leer und zeichengleich mit dem ETag DIESER Antwort ohne Anfuehrungszeichen ist,
 * die Antwort im Zweig HTTP/1.1 mit ETag laeuft und es weder das Dokument noch sw.js ist - fuer 200 und 304 gleich.
 * Sonst ist die Antwort Byte fuer Byte die von R2 (no-cache), und h erscheint nie in der Antwort.
 *
 * Echter Code aus http.cpp: die Parameterauswertung (http_set_params (), normalize_http_parameters (),
 * http_get_param (), dazu htoi () aus base.cpp) und der Anfang von http () bis vor die Hardwareerkennung - also der
 * Weg der Marke von der Query bis in http_send_fs_file () -, der C70-Block, http_fs_file_exists_and_nonempty (),
 * http_send_fs_file (), http_app () samt Weissliste und Dateinamensuche, http_content_type (), die Kopfzeilenschleife
 * des GET-Zweigs; aus c65 http_flush (), http_send (), die Zaehler.
 * Attrappen wie c70e (Core 3.1.2): LittleFS-Datei mit seek () als bool, Position bleibt bei Fehlschlag; WiFiClient mit
 * FIN/RST. Stubs: http_api_query_too_long () (fuer /app/ nie wirksam), http_remote_app_files_available (),
 * http_try_auto_install_app_files (), http_send_attr (), die Erfassung von User-Agent und Sec-Fetch-Dest.
 * Die Erwartung wird UNABHAENGIG gerechnet: die Marke aus Groesse und den letzten 8 Byte der Datei (wie die Werkzeuge).
 * Gegen R2 (3.2.40) fehlen die Zaehler: Ersatz unten, FEHL statt KAPUTT.
 */
#include "basis.h"
#include <ctype.h>
#include "lwip_attrappe.h"
#include "have.inc"
#include "c70chave.inc"
#include "defs.inc"
#define TCP_MSS 536
#define strncasecmp_P(a, b, n)  strncasecmp ((a), (b), (n))
#define strncmp_P(a, b, n)      strncmp ((a), (b), (n))
enum SeekMode { SeekSet = 0, SeekCur = 1, SeekEnd = 2 };

/* ---- Verbindung (wie c70e) */
struct Versuch { size_t ret; unsigned long dauer; bool fin; int snd_danach; };
struct ClientC
{
    std::deque<Versuch> plan; bool fin_offen = false, fin = false; int snd = 1072; std::string empfangen;
    std::vector<std::string> stuecke; std::string ende; unsigned long timeout = 5000; bool pcb = true;
    size_t write (const uint8_t * p, size_t n)
    {
        Versuch v = plan.empty () ? Versuch { (size_t) -1, 0, false, 1072 } : plan.front ();
        if (! plan.empty ()) plan.pop_front ();
        size_t r = std::min (n, v.ret);
        if (fin || ! pcb) r = 0;
        empfangen.append ((const char *) p, r); stuecke.push_back (std::string ((const char *) p, r));
        fake_ms += v.dauer;
        if (v.fin) fin_offen = true;
        sdk_abgabe ();
        snd = v.snd_danach;
        return r;
    }
    uint8_t connected () { return (fin || ! pcb) ? 0 : 1; }
    int availableForWrite () { return pcb ? snd : 0; }
    void setNoDelay (bool) {}
    void setTimeout (unsigned long t) { timeout = t; }
    void flush () {}
    void stop () { if (pcb) { ende = "FIN"; pcb = false; } }
    void abort () { if (pcb) { ende = "RST"; pcb = false; } }
    int available () { return 0; }
    int peek () { return -1; }
    int read () { return -1; }
    uint8_t status () { return fin ? CLOSED : ESTABLISHED; }
} http_client;
struct ServerC { uint8_t pending () const { return 0; } void backlog (uint8_t) {} } http_server;
static void sdk_abgabe (void) { if (http_client.fin_offen) { http_client.fin = true; http_client.fin_offen = false; } }

/* ---- LittleFS (wie c70e) */
static long   fs_gelesen = 0, fs_seeks = 0, fs_offen = 0;
static std::set<long> seek_scheitert;
struct FileC
{
    std::string * d = nullptr; size_t pos = 0;
    explicit operator bool () const { return d != nullptr; }
    int available () { return d && pos < d->size () ? (int) (d->size () - pos) : 0; }
    int read (uint8_t * b, size_t n) { if (! d) return 0; size_t k = std::min (n, d->size () - pos); memcpy (b, d->data () + pos, k); pos += k; fs_gelesen += (long) k; return (int) k; }
    size_t size () const { return d ? d->size () : 0; }
    bool seek (uint32_t p, SeekMode m)
    {
        fs_seeks++;
        if (! d || m != SeekSet) return false;
        long ziel = (long) p;
        if (seek_scheitert.count (ziel) || (seek_scheitert.count (-1) && d->size () >= 8 && ziel == (long) d->size () - 8)) return false;
        if (p > d->size ()) return false;
        pos = p; return true;
    }
    size_t position () const { return pos; }
    void close () { if (d) fs_offen--; d = nullptr; }
};
struct FsC
{
    std::map<std::string, std::string> f; int gemountet = 0;
    bool begin () { gemountet++; return true; } void end () { gemountet--; }
    bool exists (const char * n) { return f.count (n) > 0; }
    FileC open (const char * n, const char *) { FileC x; if (f.count (n)) { x.d = &f[n]; fs_offen++; } return x; }
} LittleFS;
typedef FileC File;

#include "zaehler.inc"
#ifndef HAVE_FS_WRITE_SHORT
static uint16_t http_fs_write_short = 0;
#endif
#ifndef C70C_HAVE_ZAEHLER
static uint16_t http_app_immutable = 0, http_app_token_miss = 0;   /* Vorstand R2: nichts zaehlt */
#endif
#include "i1.inc"
#ifndef HAVE_I1
void heap_low_note (uint8_t) {}
#endif
#include "i4.inc"
#ifndef HAVE_I4
static void heap_low_snapshot (void) {}
#endif
static void http_flush (void);
static bool http_fs_file_exists_and_nonempty (const char *);
#include "flush.inc"
#include "par.inc"
#include "c70.inc"
#include "fs.inc"

/* ---- Stubs fuer http_app () */
static int  fern_gefragt = 0;
static bool http_remote_app_files_available (char * v, size_t n) { fern_gefragt++; if (n) v[0] = 0; return false; }
static bool http_try_auto_install_app_files (void) { return false; }
static void http_send_attr (const char * s) { http_send (s); }
#include "app.inc"

/* ---- http (): echter Anfang, dann wie der Verteiler fuer /app */
static uint_fast8_t http_api_query_too_long (const char * path, size_t n) { return n > 255 && ! strncmp (path, "/api/", 5); }
#include "vorhttp.inc"
    return http_app (path);
}

/* ---- Kopfzeilenschleife des GET-Zweigs */
static std::deque<std::string> kopfzeilen;
static bool read_line (String & l) { if (kopfzeilen.empty ()) return false; l = String (kopfzeilen.front ()); kopfzeilen.pop_front (); return true; }
static void http_capture_request_user_agent (const String &) {}
static void http_capture_request_fetch_dest (const String &) {}
static void kopfschleife (void)
{
    String sHeaderLine;
    if (false) {}
#include "kopf.inc"
    }
}

/* ---- Hilfen */
static const std::string CC_NO  = "Cache-Control: no-cache\r\n";
static const std::string CC_IMM = "Cache-Control: public, max-age=31536000, immutable\r\n";
static void neu (void) { http_write_broken = 0; http_client = ClientC (); Serial.lines.clear (); ring.clear (); http_response_len = 0; http_response[0] = 0; fs_gelesen = 0; fs_seeks = 0; seek_scheitert.clear (); }
static std::string sicht (const std::string & x) { std::string r; for (unsigned char c : x) r += (c == '\r') ? std::string ("\\r") : (c == '\n') ? std::string ("\\n") : (c >= 32 && c < 127) ? std::string (1, (char) c) : std::string ("."); return r; }
static std::string kopf_von (const std::string & e) { size_t k = e.find ("\r\n\r\n"); return k == std::string::npos ? e : e.substr (0, k + 4); }
static std::string hex8 (const std::string & d) { char b[17]; for (int i = 0; i < 8; i++) snprintf (b + 2 * i, 3, "%02x", (unsigned char) d[d.size () - 8 + i]); return b; }
static std::string marke (const std::string & d) { return std::to_string (d.size ()) + "-" + hex8 (d); }      /* Inhaltsmarke wie das Werkzeug */
static std::string gz (size_t n, uint32_t crc, uint32_t isize, char fuell = 'd')
{
    std::string s ("\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03", 10);
    s += std::string (n > 18 ? n - 18 : 0, fuell);
    for (int i = 0; i < 4; i++) s += (char) ((crc >> (8 * i)) & 0xff);
    for (int i = 0; i < 4; i++) s += (char) ((isize >> (8 * i)) & 0xff);
    return s;
}
static std::string ersetze (std::string s, const std::string & a, const std::string & b) { size_t k = s.find (a); if (k != std::string::npos) s.replace (k, a.size (), b); return s; }
static size_t zaehle (const std::string & s, const std::string & t) { size_t n = 0; for (size_t k = s.find (t); k != std::string::npos; k = s.find (t, k + 1)) n++; return n; }

/* eine Anfrage wie im Geraet: Kopfzeilen durch die echte Schleife, Query durch den echten Anfang von http () */
static std::string anfrage (const char * pfad, const std::string & query, const std::vector<std::string> & kopf)
{
    neu (); kopfzeilen.assign (kopf.begin (), kopf.end ()); kopfzeilen.push_back ("");
    kopfschleife ();
    (void) http (pfad, query.c_str ());
    return http_client.empfangen;
}
static std::string inm (const std::string & v) { return "If-None-Match: " + v; }
static const std::vector<std::string> ohne;

struct Delta { long imm, miss, full, nm; };
static Delta stand (void) { return { (long) http_app_immutable, (long) http_app_token_miss, (long) http_app_full, (long) http_app_not_modified }; }
static std::string dsicht (const Delta & a, const Delta & b) { char x[96]; snprintf (x, sizeof x, "d imm=%ld miss=%ld full=%ld nm=%ld", b.imm - a.imm, b.miss - a.miss, b.full - a.full, b.nm - a.nm); return x; }

/* Kern: Anfrage mit Query gegen dieselbe Anfrage OHNE Query (= R2, AKL.3).
 *   soll_imm:  die Antwort ist die ohne Query mit genau einer ersetzten Zeile (no-cache -> immutable), sonst Byte fuer Byte gleich
 *   d_imm, d_miss: erwartete Zaehlerschritte
 * Dazu immer: Der Wert von h steht in der Antwort hoechstens im ETag (bei Treffer), sonst nirgends. */
static void pruef (const char * name, const char * pfad, const std::string & query, const std::vector<std::string> & kopf,
                   bool soll_imm, long d_imm, long d_miss, const std::string & h_roh = "")
{
    std::string ref = anfrage (pfad, "", kopf);
    Delta a = stand ();
    std::string ist = anfrage (pfad, query, kopf);
    Delta b = stand ();
    std::string soll = soll_imm ? ersetze (ref, CC_NO, CC_IMM) : ref;
    std::string n = name;
    fall ((n + ": Antwort").c_str (), ist == soll && (! soll_imm || ref.find (CC_NO) != std::string::npos),
          "ist  " + sicht (kopf_von (ist).substr (0, 260)) + "\n             soll " + sicht (kopf_von (soll).substr (0, 260)));
    fall ((n + ": immutable " + (soll_imm ? "genau einmal" : "nirgends")).c_str (), zaehle (ist, "immutable") == (soll_imm ? 1u : 0u), sicht (kopf_von (ist)));
    fall ((n + ": Zaehler").c_str (), b.imm - a.imm == d_imm && b.miss - a.miss == d_miss, dsicht (a, b));
    fall ((n + ": keine Ausgabe auf Serial (UART)").c_str (), Serial.lines.empty () && Serial.cur.empty (), Serial.lines.empty () ? Serial.cur : Serial.lines[0]);
    if (! h_roh.empty () && h_roh.size () >= 4)
    {
        size_t im_etag = soll_imm ? 1 : 0;
        fall ((n + ": h nicht gespiegelt").c_str (), zaehle (kopf_von (ist), h_roh) <= im_etag, sicht (kopf_von (ist)));
    }
}

int main (void)
{
    printf ("c70c (%s)\n", BEZEICHNUNG);
    size_t n_assets = sizeof (APP_INSTALL_ASSETS) / sizeof (APP_INSTALL_ASSETS[0]);
    for (size_t i = 0; i < n_assets; i++)
    {
        char fn[128]; app_asset_storage_filename (APP_INSTALL_ASSETS[i], 1, fn, sizeof (fn));
        LittleFS.f[fn] = gz (300 + 37 * i, 0x11111111u * (uint32_t) (i + 1), 1000 + (uint32_t) i);
    }
    const std::string js = gz (5000, 0xdeadbeefu, 163621u);
    LittleFS.f["app-app.js.gz"] = js;
    const std::string M = marke (js), E = "\"" + M + "\"";
    fall ("Vorbedingung: Marke wie design.md 18.3 (<gz-groesse>-<16 Hex>)", M == "5000-efbeadde257f0200", M);
    fall ("Vorbedingung: Weissliste mit 13 Eintraegen", n_assets == 13, std::to_string (n_assets));
    const std::string V = "v=1.4.102&h=";

    /* ---- A: Treffer => immutable, 200 und 304, jedes versionierbare Asset (AKL.1) */
    {
        Delta a = stand ();
        std::string r = anfrage ("/app/app.js", V + M, ohne);
        Delta b = stand ();
        fall ("A app.js Treffer 200: Kopf genau so",
              kopf_von (r) == "HTTP/1.1 200 OK\r\nContent-Type: application/javascript; charset=utf-8\r\nContent-Encoding: gzip\r\nContent-Length: 5000\r\nETag: " + E +
                              "\r\nCache-Control: public, max-age=31536000, immutable\r\nConnection: close\r\n\r\n", sicht (kopf_von (r)));
        fall ("A app.js Treffer 200: Rumpf = Datei", r.substr (kopf_von (r).size ()) == js, std::to_string (r.size ()));
        fall ("A app.js Treffer 200: app_immutable +1, app_full +1, app_token_miss 0", b.imm - a.imm == 1 && b.full - a.full == 1 && b.miss == a.miss && b.nm == a.nm, dsicht (a, b));
        a = stand ();
        r = anfrage ("/app/app.js", V + M, { inm (E) });
        b = stand ();
        fall ("A app.js Treffer 304: genau diese Antwort",
              r == "HTTP/1.1 304 Not Modified\r\nETag: " + E + "\r\nCache-Control: public, max-age=31536000, immutable\r\nConnection: close\r\n\r\n", sicht (r));
        fall ("A app.js Treffer 304: app_immutable +1, app_not_modified +1", b.imm - a.imm == 1 && b.nm - a.nm == 1 && b.miss == a.miss && b.full == a.full, dsicht (a, b));
        fall ("A app.js Treffer 304: Datei nicht gelesen (2 + 8 Byte)", fs_gelesen == 10, std::to_string (fs_gelesen));
    }
    int n_vers = 0;
    for (size_t i = 0; i < n_assets; i++)
    {
        std::string ap = APP_INSTALL_ASSETS[i];
        if (ap == "app/index.html" || ap == "app/sw.js") continue;
        char fn[128]; app_asset_storage_filename (APP_INSTALL_ASSETS[i], 1, fn, sizeof (fn));
        std::string d = LittleFS.f[fn], m = marke (d), pf = "/" + ap;
        pruef (("A " + ap + " Treffer 200").c_str (), pf.c_str (), V + m, ohne, true, 1, 0);
        pruef (("A " + ap + " Treffer 304").c_str (), pf.c_str (), V + m, { inm ("\"" + m + "\"") }, true, 1, 0);
        n_vers++;
    }
    fall ("A versionierbare Weisslisteneintraege gezaehlt: 11 (13 ohne Dokument und sw.js)", n_vers == 11, std::to_string (n_vers));
    /* v wird nicht ausgewertet; Reihenfolge der Parameter egal */
    pruef ("A nur h, ohne v", "/app/app.js", "h=" + M, ohne, true, 1, 0);
    pruef ("A h vor v", "/app/app.js", "h=" + M + "&v=1.4.102", ohne, true, 1, 0);
    pruef ("A andere v", "/app/app.js", "v=0.0.1&h=" + M, ohne, true, 1, 0);
    pruef ("A dazu ein fremder Parameter", "/app/app.js", "x=1&v=1.4.102&h=" + M, ohne, true, 1, 0);

    /* ---- B: falsche Marke => no-cache, app_token_miss (AKL.1, AKL.2) */
    {
        std::string um1 = M; um1.back () = (um1.back () == '0') ? '1' : '0';
        std::string gross = M; for (auto & c : gross) c = (char) toupper ((unsigned char) c);
        char sfn[128]; app_asset_storage_filename ("app/styles.css", 1, sfn, sizeof (sfn));
        struct F { const char * name; std::string q; std::string roh; };
        F faelle[] = {
            { "um ein Zeichen verschieden (letzte Ziffer)", V + um1, um1 },
            { "andere Groesse", V + "5001-efbeadde257f0200", "5001-efbeadde257f0200" },
            { "Praefix der Marke", V + M.substr (0, M.size () - 1), "" },
            { "nur die Groesse", V + "5000", "" },
            { "Marke verlaengert", V + M + "0", M + "0" },
            { "Marke mit Anfuehrungszeichen (= das ETag selbst, steht deshalb im ETag-Kopf)", V + E, "" },
            { "Marke mit %22 kodiert", V + "%22" + M + "%22", "" },
            { "Marke mit W/", V + "W/" + E, "" },
            { "Hexziffern gross", V + gross, gross },
            { "Marke einer anderen Datei (styles.css)", V + marke (LittleFS.f[sfn]), marke (LittleFS.f[sfn]) },
            { "beliebiger Wert", V + "spiegel-zz9", "spiegel-zz9" },
            { "Marke mit Leerzeichen (+)", V + M + "+", "" },
        };
        for (auto & f : faelle)
        {
            pruef ((std::string ("B ") + f.name + " 200").c_str (), "/app/app.js", f.q, ohne, false, 0, 1, f.roh);
            pruef ((std::string ("B ") + f.name + " 304").c_str (), "/app/app.js", f.q, { inm (E) }, false, 0, 1, f.roh);
        }
        /* Query ueber 255: http () kuerzt fuer /app/ still auf 255 - die Marke beginnt bei 245 und behaelt 10 Zeichen */
        std::string lang = "v=" + std::string (240, 'x') + "&h=" + M;
        fall ("B Vorbedingung: Query ueber 255, Marke ab Zeichen 245", lang.size () > 255 && lang.find ("&h=") == 242, std::to_string (lang.size ()));
        pruef ("B Query ueber 255, Marke abgeschnitten", "/app/app.js", lang, ohne, false, 0, 1);
        lang = "v=" + std::string (250, 'x') + "&h=" + M;
        pruef ("B Query ueber 255, h= ganz abgeschnitten (leer, kein Zaehler)", "/app/app.js", lang, ohne, false, 0, 0);
    }

    /* ---- C: leeres oder fehlendes h => no-cache, KEIN Zaehler (AKL.2: nur nicht leeres h) */
    pruef ("C h leer", "/app/app.js", "v=1.4.102&h=", ohne, false, 0, 0);
    pruef ("C h leer, 304", "/app/app.js", "h=", { inm (E) }, false, 0, 0);
    pruef ("C h ohne = (Wert NULL, http_get_param liefert \"\")", "/app/app.js", "v=1.4.102&h", ohne, false, 0, 0);
    pruef ("C nur v", "/app/app.js", "v=1.4.102", ohne, false, 0, 0);
    pruef ("C h in anderem Namen (hh)", "/app/app.js", "hh=" + M, ohne, false, 0, 0);
    pruef ("C h gross geschrieben (H)", "/app/app.js", "H=" + M, ohne, false, 0, 0);
    {
        /* ohne Query: Kopf genau wie R2 (c70e) */
        Delta a = stand ();
        std::string r = anfrage ("/app/app.js", "", ohne);
        Delta b = stand ();
        fall ("C ohne Query: Kopf wie R2",
              kopf_von (r) == "HTTP/1.1 200 OK\r\nContent-Type: application/javascript; charset=utf-8\r\nContent-Encoding: gzip\r\nContent-Length: 5000\r\nETag: " + E +
                              "\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n", sicht (kopf_von (r)));
        fall ("C ohne Query: keine R2b-Zaehler", b.imm == a.imm && b.miss == a.miss, dsicht (a, b));
        r = anfrage ("/app/app.js", "", { inm (E) });
        fall ("C ohne Query, 304 wie R2", r == "HTTP/1.1 304 Not Modified\r\nETag: " + E + "\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n", sicht (r));
    }

    /* ---- D: Dokument und sw.js mit ihrer eigenen, treffenden Marke => no-cache, app_token_miss (AKL.1 (iv)) */
    {
        const std::string ix = LittleFS.f["app-index.html.gz"], sw = LittleFS.f["app-sw.js.gz"];
        const std::string MI = marke (ix), MS = marke (sw);
        const char * doks[] = { "/app/", "/app", "/app/index.html" };
        for (const char * p : doks)
        {
            pruef ((std::string ("D Dokument ") + p + " 200").c_str (), p, V + MI, ohne, false, 0, 1);
            pruef ((std::string ("D Dokument ") + p + " 304").c_str (), p, V + MI, { inm ("\"" + MI + "\"") }, false, 0, 1);
        }
        std::string r = anfrage ("/app/", V + MI, ohne);
        fall ("D Dokument: HTTP/1.1 200 mit eigenem ETag (Vorbedingung, sonst waere D leer)", r.compare (0, 15, "HTTP/1.1 200 OK") == 0 && r.find ("ETag: \"" + MI + "\"") != std::string::npos, sicht (kopf_von (r)));
        pruef ("D sw.js 200", "/app/sw.js", V + MS, ohne, false, 0, 1);
        pruef ("D sw.js 304", "/app/sw.js", V + MS, { inm ("\"" + MS + "\"") }, false, 0, 1);
        r = anfrage ("/app/sw.js", V + MS, ohne);
        fall ("D sw.js: HTTP/1.1 200 mit eigenem ETag (Vorbedingung)", r.compare (0, 15, "HTTP/1.1 200 OK") == 0 && r.find ("ETag: \"" + MS + "\"") != std::string::npos, sicht (kopf_von (r)));
        /* Index ohne vollstaendige Installation: Hinweisseite HTTP/1.0, keine Zaehler */
        LittleFS.f.erase ("app-sw.js.gz");
        Delta a = stand (); r = anfrage ("/app/", V + MI, ohne); Delta b = stand ();
        fall ("D Index ohne app_complete: Hinweisseite HTTP/1.0, kein immutable, kein Zaehler",
              r.compare (0, 15, "HTTP/1.0 200 OK") == 0 && r.find ("immutable") == std::string::npos && b.imm == a.imm && b.miss == a.miss, dsicht (a, b));
        LittleFS.f["app-sw.js.gz"] = sw;
    }

    /* ---- E: Zweig HTTP/1.1 ohne ETag und Zweig HTTP/1.0 (AKL.1 (iii), AKL.3) */
    {
        std::string roh (5000, 'g');
        LittleFS.f["app-app.js.gz"] = roh;
        pruef ("E ohne gzip-Kopf, Marke wie aus dem Trailer", "/app/app.js", V + "5000-6767676767676767", ohne, false, 0, 1);
        std::string r = anfrage ("/app/app.js", V + "5000-6767676767676767", ohne);
        fall ("E ohne gzip-Kopf: HTTP/1.1 ohne ETag (Vorbedingung)", r.compare (0, 15, "HTTP/1.1 200 OK") == 0 && r.find ("ETag") == std::string::npos, sicht (kopf_von (r)));
        std::string winzig ("\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03\x01\x02\x03\x04\x05\x06\x07", 17);
        LittleFS.f["app-app.js.gz"] = winzig;
        pruef ("E unter 18 Byte", "/app/app.js", V + "17-0a0b0c0d0e0f1011", ohne, false, 0, 1);
        pruef ("E unter 18 Byte, leere Marke", "/app/app.js", V, ohne, false, 0, 0);
        LittleFS.f["app-app.js.gz"] = js;
        std::string x = gz (900, 0x55aa55aau, 3u);
        LittleFS.f["app-extra.js.gz"] = x;
        pruef ("E ausserhalb der Weissliste (HTTP/1.0) mit eigener Marke", "/app/extra.js", V + marke (x), ohne, false, 0, 0, marke (x));
        r = anfrage ("/app/extra.js", V + marke (x), ohne);
        fall ("E ausserhalb der Weissliste: Kopf wie R2 (HTTP/1.0, ohne ETag)",
              kopf_von (r) == "HTTP/1.0 200 OK\r\nContent-Type: application/javascript; charset=utf-8\r\nContent-Encoding: gzip\r\nContent-Length: 900\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n", sicht (kopf_von (r)));
        LittleFS.f.erase ("app-extra.js.gz");
        /* Basisname-Rueckfall eines Weisslisteneintrags: weiter HTTP/1.1, Marke aus DIESER Datei */
        std::string m = LittleFS.f["app-manifest.webmanifest.gz"]; LittleFS.f.erase ("app-manifest.webmanifest.gz"); LittleFS.f["manifest.webmanifest.gz"] = m;
        pruef ("E Basisname-Rueckfall (Manifest) Treffer", "/app/manifest.webmanifest", V + marke (m), ohne, true, 1, 0);
        LittleFS.f.erase ("manifest.webmanifest.gz"); LittleFS.f["app-manifest.webmanifest.gz"] = m;
    }

    /* ---- F: Inhalt gewechselt (Probe U, Upload-Fenster): nur die eigene Marke trifft (design.md 18.6) */
    {
        std::string a1 = gz (5000, 0x01020304u, 777u), a2 = gz (5000, 0x0a0b0c0du, 777u, 'e');
        fall ("F Vorbedingung: gleiche Groesse, andere Marke", a1.size () == a2.size () && marke (a1) != marke (a2), marke (a1) + " " + marke (a2));
        LittleFS.f["app-app.js.gz"] = a2;
        pruef ("F neue Datei, alte Marke (200)", "/app/app.js", V + marke (a1), ohne, false, 0, 1, marke (a1));
        pruef ("F neue Datei, alte Marke, alter If-None-Match (200 mit neuem ETag)", "/app/app.js", V + marke (a1), { inm ("\"" + marke (a1) + "\"") }, false, 0, 1, marke (a1));
        pruef ("F neue Datei, neue Marke (200)", "/app/app.js", V + marke (a2), ohne, true, 1, 0);
        pruef ("F neue Datei, neue Marke (304)", "/app/app.js", V + marke (a2), { inm ("\"" + marke (a2) + "\"") }, true, 1, 0);
        std::string r = anfrage ("/app/app.js", V + marke (a2), ohne);
        fall ("F neue Marke: Rumpf ist die neue Datei", r.substr (kopf_von (r).size ()) == a2, "");
        LittleFS.f["app-app.js.gz"] = js;
    }

    /* ---- G: seek zurueck scheitert - nichts gesendet, also auch kein immutable und kein Zaehler */
    {
        neu (); kopfzeilen = { "" }; kopfschleife (); seek_scheitert.insert (0);
        Delta a = stand ();
        (void) http ("/app/app.js", (V + M).c_str ());
        Delta b = stand ();
        fall ("G seek zurueck scheitert mit Marke: 404 HTTP/1.0, kein immutable, kein Zaehler",
              http_client.empfangen.compare (0, 22, "HTTP/1.0 404 Not Found") == 0 && http_client.empfangen.find ("immutable") == std::string::npos && b.imm == a.imm && b.miss == a.miss,
              sicht (http_client.empfangen.substr (0, 60)) + " " + dsicht (a, b));
    }

    /* ---- H: Zaehler saettigen, nichts setzt sie zurueck */
    {
        http_app_immutable = 0xFFFF; http_app_token_miss = 0xFFFF;
        (void) anfrage ("/app/app.js", V + M, ohne); (void) anfrage ("/app/app.js", V + "falsch", ohne);
        (void) anfrage ("/app/app.js", V + M, { inm (E) }); (void) anfrage ("/app/sw.js", V + "x", ohne);
        fall ("H Zaehler saettigen bei 65535", http_app_immutable == 0xFFFF && http_app_token_miss == 0xFFFF, std::to_string (http_app_immutable) + " " + std::to_string (http_app_token_miss));
        http_app_immutable = 7; http_app_token_miss = 9;
        (void) anfrage ("/app/app.js", "", ohne); (void) anfrage ("/app/", "", ohne); (void) anfrage ("/app/extra.js", V + M, ohne);
        fall ("H ohne Marke und ausserhalb: Zaehler bleiben stehen", http_app_immutable == 7 && http_app_token_miss == 9, std::to_string (http_app_immutable) + " " + std::to_string (http_app_token_miss));
    }
    fall ("ueber alles: LittleFS ausgehaengt, keine Datei offen", LittleFS.gemountet == 0 && fs_offen == 0, std::to_string (LittleFS.gemountet) + " " + std::to_string (fs_offen));
    return ende ("c70c", BEZEICHNUNG);
}
