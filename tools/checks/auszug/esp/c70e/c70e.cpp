/* Pruefstand c70e (Paket 2026-10-15, R2.1, C70 Variante T; AKC.1-AKC.5, AKC.7; design.md 4.4, 8).
 *
 * Echter Code aus http.cpp: der C70-Block (If-None-Match-Puffer, Erfassung, ETag aus dem gzip-Trailer, Vergleich),
 * http_fs_file_exists_and_nonempty (), http_send_fs_file (), http_app () samt Weissliste und Dateinamensuche,
 * http_content_type (), die Kopfzeilenschleife des GET-Zweigs von http_server_loop (), dazu aus c65 http_flush (),
 * http_send (), die Zaehler und die Messpunkte.
 * Attrappen (Core 3.1.2):
 *   LittleFS-Datei: read () liefert int, 0 am Ende; seek () liefert bool und laesst die Position bei einem Fehlschlag
 *   unveraendert (LittleFS.h:434-453); Fehlschlag je seek-Ziel einstellbar; gezaehlt werden gelesene Byte.
 *   WiFiClient: write () nimmt hoechstens das Geplante; stop () sendet FIN und setzt den pcb auf null, abort () RST -
 *   danach ist der jeweils andere wirkungslos (ClientContext.h:58-92), wie im Core. Jeder write () wird einzeln
 *   festgehalten (Kopfzeilen in einem Schreibvorgang).
 *   Stubs: http_get_param (), http_remote_app_files_available (), http_try_auto_install_app_files (), http_send_attr (),
 *   die Erfassung von User-Agent und Sec-Fetch-Dest (gezaehlt), read_line () aus einer Zeilenliste.
 * Gegen den Vorstand (3.2.39) fehlen der C70-Block, die Zaehler und der vierte Parameter: Ersatz unten, FEHL statt KAPUTT.
 */
#include "basis.h"
#include "lwip_attrappe.h"
#include "have.inc"
#include "c70have.inc"
#include "defs.inc"
#define TCP_MSS 536
#define strncasecmp_P(a, b, n)  strncasecmp ((a), (b), (n))
#define strncmp_P(a, b, n)      strncmp ((a), (b), (n))
enum SeekMode { SeekSet = 0, SeekCur = 1, SeekEnd = 2 };

/* ---- Verbindung */
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

/* ---- LittleFS */
static long   fs_gelesen = 0, fs_seeks = 0, fs_offen = 0;
static std::set<long> seek_scheitert;                       /* Ziele, deren seek () scheitert (-1: das Ende - 8) */
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
        if (p > d->size ()) return false;                   /* LittleFS.h:448-451: Position bleibt */
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
#ifndef C70_HAVE_ZAEHLER
static uint16_t http_app_not_modified = 0, http_app_full = 0;      /* Vorstand: nichts zaehlt */
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
#include "c70.inc"
#ifndef C70_HAVE_BLOCK
static void http_clear_request_if_none_match (void) {}             /* Vorstand: liest If-None-Match nicht */
static void http_capture_request_if_none_match (const String &) {}
#endif
#include "fs.inc"
#ifdef C70_HAVE_4ARG
#define SENDFS(f, ct, gz, app)  http_send_fs_file ((f), (ct), (gz), (app))
#else
#define SENDFS(f, ct, gz, app)  http_send_fs_file ((f), (ct), (gz))
#endif

/* ---- Stubs fuer http_app () */
static char param_action[16] = "";
static char * http_get_param (const char *) { return param_action; }
static int  fern_gefragt = 0;
static bool http_remote_app_files_available (char * v, size_t n) { fern_gefragt++; if (n) v[0] = 0; return false; }
static bool http_try_auto_install_app_files (void) { return false; }
static void http_send_attr (const char * s) { http_send (s); }
#include "app.inc"

/* ---- Kopfzeilenschleife des GET-Zweigs */
static std::deque<std::string> kopfzeilen;
static int ua_gesehen = 0, fd_gesehen = 0;
static bool read_line (String & l) { if (kopfzeilen.empty ()) return false; l = String (kopfzeilen.front ()); kopfzeilen.pop_front (); return true; }
static void http_capture_request_user_agent (const String &) { ua_gesehen++; }
static void http_capture_request_fetch_dest (const String &) { fd_gesehen++; }
static void kopfschleife (void)
{
    String sHeaderLine;
    if (false) {}
#include "kopf.inc"
    }
}

/* ---- Hilfen */
struct Antwort { std::string status, kopf, rumpf; std::map<std::string, std::string> felder; int n_felder = 0; bool kopf_ganz = false; std::string ende; };
static Antwort zerlege (void)
{
    Antwort a; const std::string & e = http_client.empfangen; size_t k = e.find ("\r\n\r\n");
    a.ende = http_client.ende;
    if (k == std::string::npos) { a.kopf = e; return a; }
    a.kopf = e.substr (0, k); a.rumpf = e.substr (k + 4);
    size_t z = a.kopf.find ("\r\n"); a.status = a.kopf.substr (0, z);
    while (z != std::string::npos)
    {
        size_t s = z + 2, n = a.kopf.find ("\r\n", s); std::string l = a.kopf.substr (s, n == std::string::npos ? std::string::npos : n - s);
        size_t c = l.find (": "); if (c != std::string::npos) { a.felder[l.substr (0, c)] = l.substr (c + 2); a.n_felder++; }
        z = n;
    }
    a.kopf_ganz = ! http_client.stuecke.empty () && http_client.stuecke[0].size () >= k + 4 && http_client.stuecke[0].compare (0, k + 4, e, 0, k + 4) == 0;
    return a;
}
static std::string sicht (const std::string & x) { std::string r; for (unsigned char c : x) r += (c == '\r') ? std::string ("\\r") : (c == '\n') ? std::string ("\\n") : (c >= 32 && c < 127) ? std::string (1, (char) c) : std::string ("."); return r; }
static std::string feld (const Antwort & a, const char * n) { auto it = a.felder.find (n); return it == a.felder.end () ? "(fehlt)" : it->second; }
static bool hat (const Antwort & a, const char * n) { return a.felder.count (n) > 0; }
static void neu (void) { http_write_broken = 0; http_client = ClientC (); Serial.lines.clear (); ring.clear (); http_response_len = 0; http_response[0] = 0; fs_gelesen = 0; fs_seeks = 0; seek_scheitert.clear (); }
static std::string hex8 (const std::string & d) { char b[17]; for (int i = 0; i < 8; i++) snprintf (b + 2 * i, 3, "%02x", (unsigned char) d[d.size () - 8 + i]); return b; }
static std::string erwartet (const std::string & d) { return "\"" + std::to_string (d.size ()) + "-" + hex8 (d) + "\""; }   /* wie die Werkzeuge: aus den letzten 8 Byte */
static std::string gz (size_t n, uint32_t crc, uint32_t isize, char fuell = 'd')
{
    std::string s ("\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03", 10);
    s += std::string (n > 18 ? n - 18 : 0, fuell);
    for (int i = 0; i < 4; i++) s += (char) ((crc >> (8 * i)) & 0xff);
    for (int i = 0; i < 4; i++) s += (char) ((isize >> (8 * i)) & 0xff);
    return s;
}
/* eine Anfrage wie im Geraet: Kopfzeilen durch die echte Schleife, dann http_app () */
static Antwort anfrage (const char * pfad, const std::vector<std::string> & kopf)
{
    neu (); kopfzeilen.assign (kopf.begin (), kopf.end ()); kopfzeilen.push_back ("");
    kopfschleife ();
    (void) http_app (pfad);
    return zerlege ();
}
static std::string inm (const std::string & v) { return "If-None-Match: " + v; }
static const std::vector<std::string> ohne;

int main (void)
{
    printf ("c70e (%s)\n", BEZEICHNUNG);
    /* Installation: alle Eintraege der Weissliste als .gz, flach abgelegt wie nach install-app.sh */
    size_t n_assets = sizeof (APP_INSTALL_ASSETS) / sizeof (APP_INSTALL_ASSETS[0]);
    for (size_t i = 0; i < n_assets; i++)
    {
        char fn[128]; app_asset_storage_filename (APP_INSTALL_ASSETS[i], 1, fn, sizeof (fn));
        LittleFS.f[fn] = gz (300 + 37 * i, 0x11111111u * (uint32_t) (i + 1), 1000 + (uint32_t) i);
    }
    const std::string js = gz (5000, 0xdeadbeefu, 163621u);
    LittleFS.f["app-app.js.gz"] = js;
    const std::string E = erwartet (js);
    fall ("Vorbedingung: ETag-Form wie design.md 4.4", E == "\"5000-efbeadde257f0200\"", E);

    /* ---- AKC.1 / AKC.7: 200 auf ein Weisslisten-Asset als .gz */
    {
        uint16_t f0 = http_app_full, n0 = http_app_not_modified;
        Antwort a = anfrage ("/app/app.js", ohne);
        fall ("200: Statuszeile HTTP/1.1 200 OK", a.status == "HTTP/1.1 200 OK", a.status);
        fall ("200: ETag aus Groesse und gzip-Trailer", feld (a, "ETag") == E, feld (a, "ETag"));
        fall ("200: Content-Type, Content-Encoding unveraendert", feld (a, "Content-Type") == "application/javascript; charset=utf-8" && feld (a, "Content-Encoding") == "gzip", sicht (a.kopf));
        fall ("200: Cache-Control: no-cache, Connection: close", feld (a, "Cache-Control") == "no-cache" && feld (a, "Connection") == "close", sicht (a.kopf));
        fall ("200: genau diese sechs Felder in dieser Reihenfolge, kein Date",
              a.kopf == "HTTP/1.1 200 OK\r\nContent-Type: application/javascript; charset=utf-8\r\nContent-Encoding: gzip\r\nContent-Length: 5000\r\nETag: " + E + "\r\nCache-Control: no-cache\r\nConnection: close", sicht (a.kopf));
        fall ("200: Koerperlaenge = Content-Length, Koerper = Datei", feld (a, "Content-Length") == std::to_string (a.rumpf.size ()) && a.rumpf == js, std::to_string (a.rumpf.size ()));
        fall ("200: Kopfzeilen in EINEM Schreibvorgang", a.kopf_ganz, std::to_string (http_client.stuecke.size ()));
        fall ("200: regulaeres Ende (FIN)", a.ende == "FIN", a.ende);
        fall ("200: app_full +1, app_not_modified unveraendert", http_app_full == f0 + 1 && http_app_not_modified == n0, std::to_string (http_app_full - f0));
        fall ("200: LittleFS wieder ausgehaengt", LittleFS.gemountet == 0, std::to_string (LittleFS.gemountet));
    }
    /* ---- AKC.2: Treffer => 304 ohne Koerper, ohne Content-Length, Datei nicht gelesen */
    {
        uint16_t f0 = http_app_full, n0 = http_app_not_modified;
        Antwort a = anfrage ("/app/app.js", { "Host: uhr", inm (E), "Accept: */*" });
        fall ("304: genau diese Antwort, nichts danach", http_client.empfangen == "HTTP/1.1 304 Not Modified\r\nETag: " + E + "\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n",
              sicht (http_client.empfangen.substr (0, 200)));
        fall ("304: ohne Content-Length und ohne Koerper", ! hat (a, "Content-Length") && a.rumpf.empty (), sicht (a.kopf));
        fall ("304: Datei nicht gelesen (nur 2 + 8 Byte)", fs_gelesen == 10, std::to_string (fs_gelesen));
        fall ("304: regulaeres Ende (FIN)", a.ende == "FIN", a.ende);
        fall ("304: app_not_modified +1, app_full unveraendert", http_app_not_modified == n0 + 1 && http_app_full == f0, std::to_string (http_app_not_modified - n0));
        fall ("304: Kopfzeilen in EINEM Schreibvorgang", a.kopf_ganz && http_client.stuecke.size () == 1, std::to_string (http_client.stuecke.size ()));
    }
    /* ---- Fehltreffer, W/, Liste, *, ueberlang, Kopfname */
    {
        std::string falsch = E; falsch[falsch.size () - 2] = (falsch[falsch.size () - 2] == '0') ? '1' : '0';
        struct F { const char * name; std::vector<std::string> kopf; bool soll304; };
        std::string lang = "\"" + std::string (40, 'z') + "\", " + E;                    /* ueberlang, Treffer hinten */
        std::string lang_vorn = E + ", \"" + std::string (20, 'z') + "\"";                /* ueberlang, Treffer vorn */
        F faelle[] = {
            { "Fehltreffer gleicher Laenge => 200", { inm (falsch) }, false },
            { "Fehltreffer anderer Groesse => 200", { inm ("\"4999-efbeadde257f0200\"") }, false },
            { "W/ wird uebergangen => 304", { inm ("W/" + E) }, true },
            { "Liste mit Treffer hinten => 304", { inm ("\"a\", " + E) }, true },
            { "Liste mit Treffer vorn und W/ => 304", { inm ("W/" + E + ",\"b\"") }, true },
            { "Liste ohne Treffer => 200", { inm ("\"a\", W/\"b\"") }, false },
            { "* trifft nicht => 200", { inm ("*") }, false },
            { "ueberlang, Treffer hinten => 200", { inm (lang) }, false },
            { "ueberlang, Treffer vorn => 200 (nicht abgeschnitten)", { inm (lang_vorn) }, false },
            { "genau 35 Zeichen mit Treffer => 304", { inm (E + ",\"" + std::string (35 - E.size () - 3, 'q') + "\"") }, true },
            { "Kopfname klein => 304", { "if-none-match: " + E }, true },
            { "Kopfname gross, Leerraum => 304", { "IF-NONE-MATCH:   " + E + "  " }, true },
            { "ohne Leerzeichen nach dem Doppelpunkt => 304", { "If-None-Match:" + E }, true },
            { "leerer Wert => 200", { "If-None-Match: " }, false },
            { "ETag ohne Anfuehrungszeichen => 200", { inm (E.substr (1, E.size () - 2)) }, false },
            { "fremder Kopf mit dem ETag => 200", { "X-If-None-Match: " + E }, false },
            { "nach User-Agent und Sec-Fetch-Dest => 304", { "User-Agent: x", "Sec-Fetch-Dest: script", inm (E) }, true },
        };
        for (auto & f : faelle)
        {
            Antwort a = anfrage ("/app/app.js", f.kopf);
            bool ist304 = a.status == "HTTP/1.1 304 Not Modified";
            bool ist200 = a.status == "HTTP/1.1 200 OK" && a.rumpf == js;
            fall (f.name, f.soll304 ? (ist304 && a.rumpf.empty ()) : ist200, a.status + " / " + f.kopf.back ().substr (0, 60));
        }
        fall ("Kopfschleife: User-Agent und Sec-Fetch-Dest weiter erfasst", ua_gesehen >= 1 && fd_gesehen >= 1, std::to_string (ua_gesehen) + "/" + std::to_string (fd_gesehen));
#ifdef C70_HAVE_BLOCK
        fall ("Puffer 36 Byte (AKZ.8)", sizeof (http_request_if_none_match) == 36, std::to_string (sizeof (http_request_if_none_match)));
#else
        fall ("Puffer 36 Byte (AKZ.8)", false, "kein C70-Block");
#endif
    }
    /* ---- Puffer je Anfrage geleert */
    {
        Antwort a = anfrage ("/app/app.js", { inm (E) });
        Antwort b = anfrage ("/app/app.js", ohne);
        fall ("je Anfrage geleert: zweite Anfrage ohne Kopf => 200", a.status == "HTTP/1.1 304 Not Modified" && b.status == "HTTP/1.1 200 OK" && b.rumpf == js, a.status + " / " + b.status);
    }
    /* ---- AKC.3 Variante T */
    {
        std::string a1 = gz (5000, 0x01020304u, 777u), a2 = gz (5000, 0x0a0b0c0du, 777u, 'e');
        LittleFS.f["app-app.js.gz"] = a1; Antwort r1 = anfrage ("/app/app.js", ohne);
        LittleFS.f["app-app.js.gz"] = a2; Antwort r2 = anfrage ("/app/app.js", { inm (feld (r1, "ETag")) });
        fall ("T: zwei .gz gleicher Groesse, anderer Inhalt => verschiedene ETag", feld (r1, "ETag") != feld (r2, "ETag") && feld (r2, "ETag") == erwartet (a2), feld (r1, "ETag") + " " + feld (r2, "ETag"));
        fall ("T: das alte ETag trifft die neue Datei nicht => 200", r2.status == "HTTP/1.1 200 OK" && r2.rumpf == a2, r2.status);
        std::string ab = js.substr (0, js.size () - 100);
        LittleFS.f["app-app.js.gz"] = ab; Antwort r3 = anfrage ("/app/app.js", { inm (E) });
        fall ("T: abgeschnittene Datei => anderes ETag, 200 mit vollem (kurzem) Inhalt", feld (r3, "ETag") != E && r3.status == "HTTP/1.1 200 OK" && r3.rumpf == ab, feld (r3, "ETag"));
        std::string roh (5000, 'g');
        LittleFS.f["app-app.js.gz"] = roh; Antwort r4 = anfrage ("/app/app.js", { inm ("\"5000-6767676767676767\"") });
        fall ("T: ohne 1f 8b => kein ETag, HTTP/1.1 200, nie 304", ! hat (r4, "ETag") && r4.status == "HTTP/1.1 200 OK" && r4.rumpf == roh, sicht (r4.kopf));
        std::string winzig ("\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03\x01\x02\x03\x04\x05\x06\x07", 17);
        LittleFS.f["app-app.js.gz"] = winzig; Antwort r5 = anfrage ("/app/app.js", ohne);
        fall ("T: kuerzer als 18 Byte => kein ETag, nichts gelesen ausser dem Rumpf", ! hat (r5, "ETag") && r5.rumpf == winzig && fs_seeks == 0, sicht (r5.kopf) + " seeks " + std::to_string (fs_seeks));
        std::string kleinst = gz (18, 0, 0);
        LittleFS.f["app-app.js.gz"] = kleinst; Antwort r6 = anfrage ("/app/app.js", ohne);
        fall ("T: kleinstes gzip (18 Byte) => ETag", feld (r6, "ETag") == erwartet (kleinst), feld (r6, "ETag"));
        LittleFS.f["app-app.js.gz"] = js;
    }
    /* ---- leere .gz: nie 304, nie 200 */
    {
        LittleFS.f["app-app.js.gz"] = "";
        uint16_t n0 = http_app_not_modified;
        Antwort a = anfrage ("/app/app.js", { inm (E) });
        fall ("leere .gz: kein 304, kein 200, sondern 404 HTTP/1.0", a.status == "HTTP/1.0 404 Not Found" && http_app_not_modified == n0, a.status);
        neu (); kopfzeilen = { inm (E), inm ("\"0\""), "" }; kopfschleife ();
        uint_fast8_t r = SENDFS ("app-app.js.gz", "application/javascript", 1, 1);
        fall ("leere .gz direkt an http_send_fs_file (): nichts gesendet, Rueckgabe 0", r == 0 && http_client.empfangen.empty (), std::to_string (r) + " " + sicht (http_client.empfangen.substr (0, 60)));
        neu (); kopfzeilen = { inm ("\"0\""), "" }; kopfschleife ();
        r = SENDFS ("app-app.js.gz", "application/javascript", 1, 1);
        fall ("leere .gz, If-None-Match \"0\": nichts gesendet", r == 0 && http_client.empfangen.empty (), sicht (http_client.empfangen.substr (0, 60)));
        LittleFS.f["app-app.js.gz"] = js;
    }
    /* ---- Index: mit und ohne vollstaendige Installation */
    {
        const std::string ix = LittleFS.f["app-index.html.gz"]; std::string EI = erwartet (ix);
        Antwort a = anfrage ("/app/", ohne);
        fall ("Index vollstaendig: HTTP/1.1 200 mit ETag, text/html", a.status == "HTTP/1.1 200 OK" && feld (a, "ETag") == EI && feld (a, "Content-Type") == "text/html; charset=utf-8" && a.rumpf == ix, sicht (a.kopf));
        Antwort b = anfrage ("/app", { inm (EI) });
        fall ("Index vollstaendig, Treffer: 304", b.status == "HTTP/1.1 304 Not Modified" && b.rumpf.empty (), b.status);
        std::string sw = LittleFS.f["app-sw.js.gz"]; LittleFS.f.erase ("app-sw.js.gz");
        uint16_t n0 = http_app_not_modified;
        Antwort c = anfrage ("/app/", { inm (EI) });
        fall ("Index ohne app_complete, Treffer: Hinweisseite HTTP/1.0, kein 304, kein ETag",
              c.status == "HTTP/1.0 200 OK" && ! hat (c, "ETag") && c.rumpf.find ("noch nicht installiert") != std::string::npos && http_app_not_modified == n0, c.status);
        LittleFS.f["app-sw.js.gz"] = sw;
        LittleFS.f["app-index.html.gz"] = "";
        Antwort d = anfrage ("/app/", { inm (EI) });
        fall ("Index als leere .gz: Hinweisseite HTTP/1.0, kein 304", d.status == "HTTP/1.0 200 OK" && ! hat (d, "ETag") && d.rumpf.find ("noch nicht installiert") != std::string::npos, d.status);
        LittleFS.f["app-index.html.gz"] = ix;
        param_action[0] = 0;
    }
    /* ---- ausserhalb der Weissliste, Basisname-Rueckfall, 404 */
    {
        std::string x = gz (900, 0x55aa55aau, 3u);
        LittleFS.f["app-extra.js.gz"] = x;
        uint16_t f0 = http_app_full;
        Antwort a = anfrage ("/app/extra.js", { inm (erwartet (x)) });
        fall ("ausserhalb der Weissliste: HTTP/1.0 200 ohne ETag, auch mit passendem If-None-Match", a.status == "HTTP/1.0 200 OK" && ! hat (a, "ETag") && a.rumpf == x && http_app_full == f0, sicht (a.kopf));
        fall ("ausserhalb der Weissliste: Kopf wie bis 3.2.39", a.kopf == "HTTP/1.0 200 OK\r\nContent-Type: application/javascript; charset=utf-8\r\nContent-Encoding: gzip\r\nContent-Length: 900\r\nCache-Control: no-cache\r\nConnection: close", sicht (a.kopf));
        LittleFS.f.erase ("app-extra.js.gz");
        std::string m = LittleFS.f["app-manifest.webmanifest.gz"]; LittleFS.f.erase ("app-manifest.webmanifest.gz"); LittleFS.f["manifest.webmanifest.gz"] = m;
        Antwort b = anfrage ("/app/manifest.webmanifest", ohne);
        fall ("Basisname-Rueckfall (Weisslisteneintrag): HTTP/1.1 mit ETag", b.status == "HTTP/1.1 200 OK" && feld (b, "ETag") == erwartet (m), sicht (b.kopf));
        LittleFS.f.erase ("manifest.webmanifest.gz"); LittleFS.f["app-manifest.webmanifest.gz"] = m;
        Antwort c = anfrage ("/app/fehlt.js", { inm (E) });
        fall ("fehlende Datei: 404 HTTP/1.0 wie bisher", c.status == "HTTP/1.0 404 Not Found", c.status);
    }
    /* ---- seek scheitert */
    {
        neu (); kopfzeilen = { inm (E), "" }; kopfschleife (); seek_scheitert.insert (-1);
        uint_fast8_t r = SENDFS ("app-app.js.gz", "application/javascript", 1, 1);
        Antwort a = zerlege ();
        fall ("seek ans Ende scheitert: 200 ohne ETag, Koerper vom Dateianfang, kein 304", r == 1 && a.status == "HTTP/1.1 200 OK" && ! hat (a, "ETag") && a.rumpf == js, sicht (a.kopf));
        neu (); kopfzeilen = { "" }; kopfschleife (); seek_scheitert.insert (0);
        r = SENDFS ("app-app.js.gz", "application/javascript", 1, 1);
        fall ("seek zurueck an den Anfang scheitert: nichts gesendet, Rueckgabe 0, eine Zeile", r == 0 && http_client.empfangen.empty () && Serial.lines.size () == 1 && Serial.lines[0] == "- fs seek error: app-app.js.gz",
              std::to_string (r) + " " + std::to_string (http_client.empfangen.size ()) + " " + (Serial.lines.empty () ? "" : Serial.lines[0]));
        fall ("seek scheitert: Datei geschlossen, LittleFS ausgehaengt", fs_offen == 0 && LittleFS.gemountet == 0, std::to_string (fs_offen));
        neu (); kopfzeilen = { "" }; kopfschleife (); seek_scheitert.insert (0); (void) http_app ("/app/app.js"); Antwort b = zerlege ();
        fall ("seek zurueck scheitert ueber http_app (): 404 HTTP/1.0, kein Dateiinhalt", b.status == "HTTP/1.0 404 Not Found" && b.rumpf.find ("PWA asset not found") == 0, b.status);
    }
    /* ---- AKC.7: Abbruch endet mit RST */
    {
        neu (); kopfzeilen = { "" }; kopfschleife ();
        http_client.plan.push_back ({ (size_t) -1, 0, false, 1072 });     /* Kopfzeilen */
        http_client.plan.push_back ({ (size_t) -1, 0, false, 1072 });     /* Block 1, Stueck 1 */
        http_client.plan.push_back ({ 100, 2000, false, 1072 });          /* Block 1, Stueck 2 kurz */
        uint_fast8_t r = SENDFS ("app-app.js.gz", "application/javascript", 1, 1);
        Antwort a = zerlege ();
        fall ("Abbruch: Rueckgabe 2, Koerper kuerzer als Content-Length", r == 2 && feld (a, "Content-Length") == "5000" && a.rumpf.size () == 636, std::to_string (r) + " " + std::to_string (a.rumpf.size ()));
        fall ("Abbruch: Ende mit RST, nie regulaer", a.ende == "RST", a.ende);
        neu (); kopfzeilen = { inm (E), "" }; kopfschleife ();
        http_client.plan.push_back ({ 0, 2000, false, 0 });              /* 304-Kopf geht nicht hinaus */
        r = SENDFS ("app-app.js.gz", "application/javascript", 1, 1);
        fall ("304-Kopf verloren: Rueckgabe 2, RST", r == 2 && http_client.ende == "RST", std::to_string (r) + " " + http_client.ende);
    }
    /* ---- Zaehler saettigen */
    {
        http_app_not_modified = 0xFFFF; http_app_full = 0xFFFF;
        (void) anfrage ("/app/app.js", { inm (E) }); (void) anfrage ("/app/app.js", ohne);
        fall ("Zaehler saettigen bei 65535", http_app_not_modified == 0xFFFF && http_app_full == 0xFFFF, std::to_string (http_app_not_modified) + " " + std::to_string (http_app_full));
    }
    fall ("ueber alles: LittleFS ausgehaengt, keine Datei offen", LittleFS.gemountet == 0 && fs_offen == 0, std::to_string (LittleFS.gemountet) + " " + std::to_string (fs_offen));
    return ende ("c70e", BEZEICHNUNG);
}
