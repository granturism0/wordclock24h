/* Pruefstand c65w (Paket 2026-10-13, E4.2, AKV.4; design-e4.md 19.4) - Attrappen wie c65b.
 * Gegenstand: http_flush () und http_send_fs_file () rufen write () nie mit mehr als TCP_MSS Byte; Fortschrittsregel
 * (ganz -> naechstes Stueck, nichts -> Ende, teilweise -> EIN Nachversuch je Aufruf mit dem Rest des Stuecks);
 * ausgelieferte Byte gleich Eingabe. Urspruenglicher Kopf von c65b:
 * Echter Code: http_flush (), http_concat_response (), http_send (const char *), http_send_fs_file (),
 * http_fs_file_exists_and_nonempty (), heap_low_note (), die Zaehler.
 * WiFiClient-Attrappe nach der Architektur-Checkliste: Zustellung (hier: das FIN der Gegenstelle) NUR an
 * Abgabepunkten - im Warten von write () (esp_delay in _write_from_source) und in yield (); connected () ist
 * nach dem FIN 0, OHNE "|| available ()" (Core 3.1.2, WiFiClient.cpp:327-333, ClientContext.h:363-371).
 * availableForWrite () ist tcp_sndbuf () und bleibt nach einem FIN stehen, 0 erst ohne pcb (ClientContext.h:161-164).
 */
#include "basis.h"
#include "lwip_attrappe.h"                                  /* E4: Listen fuer I-4 */
#include "have.inc"
#include "defs.inc"
#define TCP_MSS 536
#define strncasecmp_P(a, b, n)  strncasecmp ((a), (b), (n))     /* R2 C70: Kopfname von If-None-Match */
enum SeekMode { SeekSet = 0, SeekCur = 1, SeekEnd = 2 };

/* ---- Verbindung */
struct Versuch { size_t ret; unsigned long dauer; bool fin; int snd_danach; };
struct ClientB
{
    std::deque<Versuch> plan; bool fin_offen = false, fin = false; int snd = 1072; std::string empfangen;
    int writes = 0; unsigned long timeout = 5000;
    size_t write (const uint8_t * p, size_t n)
    {
        writes++; laengen.push_back (n);
        Versuch v = plan.empty () ? Versuch { (size_t) -1, 0, false, 1072 } : plan.front ();
        if (! plan.empty ()) plan.pop_front ();
        size_t r = std::min (n, v.ret);
        if (fin) r = 0;                                /* CLOSE_WAIT: der Core schreibt nichts mehr */
        empfangen.append ((const char *) p, r);
        fake_ms += v.dauer;                            /* Warten in _write_from_source = Abgabepunkt */
        if (v.fin) fin_offen = true;
        sdk_abgabe ();
        snd = v.snd_danach;
        return r;
    }
    uint8_t connected () { return fin ? 0 : 1; }
    int availableForWrite () { return snd; }
    void setNoDelay (bool) {}
    void setTimeout (unsigned long t) { timeout = t; }
    void flush () {}
    void stop () {}
    int aborts = 0; void abort () { aborts++; }                       /* R2 C70 (AKC.7): Abbruch mit RST */
    uint8_t status () { return fin ? CLOSED : ESTABLISHED; }          /* E4 I-4 */
    std::vector<size_t> laengen;                                     /* E4 c65w: Laenge jedes write () */
} http_client;
struct ServerB { uint8_t pending () const { return 0; } void backlog (uint8_t) {} } http_server;   /* E4 I-4 */
static void sdk_abgabe (void) { if (http_client.fin_offen) { http_client.fin = true; http_client.fin_offen = false; } }

/* ---- LittleFS */
struct FileB
{
    std::string * d = nullptr; size_t pos = 0;
    explicit operator bool () const { return d != nullptr; }
    int available () { return d && pos < d->size () ? (int) (d->size () - pos) : 0; }
    int read (uint8_t * b, size_t n) { size_t k = std::min (n, d->size () - pos); memcpy (b, d->data () + pos, k); pos += k; return (int) k; }
    size_t size () const { return d ? d->size () : 0; }
    void close () { d = nullptr; }
    /* R2 C70: wie LittleFS.h:434-453 - bei Fehlschlag bleibt die Position, hinter das Ende geht es nicht */
    bool seek (uint32_t p, SeekMode m) { if (! d || m != SeekSet || p > d->size ()) return false; pos = p; return true; }
    size_t position () const { return pos; }
};
struct FsB
{
    std::map<std::string, std::string> f;
    bool begin () { return true; } void end () {}
    bool exists (const char * n) { return f.count (n) > 0; }
    FileB open (const char * n, const char *) { FileB x; if (f.count (n)) x.d = &f[n]; return x; }
} LittleFS;
typedef FileB File;

#include "zaehler.inc"
#ifndef HAVE_FS_WRITE_SHORT
static uint16_t http_fs_write_short = 0;
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
#include "fsfile.inc"
#ifdef HAVE_FS_APP_ASSET
#define SENDFS(f, ct, gz)   http_send_fs_file ((f), (ct), (gz), 1)       /* R2 C70: ein App-Asset aus der Weissliste */
#else
#define SENDFS(f, ct, gz)   http_send_fs_file ((f), (ct), (gz))
#endif


/* ---- Hilfen */
static void neu (void) { http_write_broken = 0; http_client = ClientB (); Serial.lines.clear (); ring.clear (); http_response_len = 0; http_response[0] = 0; }
static int zeilen (const char * praefix) { int n = 0; for (auto & l : Serial.lines) if (! l.compare (0, strlen (praefix), praefix)) n++; return n; }
static std::string zeile (const char * praefix) { for (auto & l : Serial.lines) if (! l.compare (0, strlen (praefix), praefix)) return l; return ""; }
static std::string feld (const std::string & z, const char * k) { size_t p = z.find (k); if (p == std::string::npos) return "(fehlt)"; p += strlen (k); return z.substr (p, z.find (' ', p) - p); }
static std::string L (void) { std::string r; for (size_t x : http_client.laengen) r += std::to_string (x) + " "; return r; }
static size_t groesste (size_t ab = 0) { size_t g = 0; for (size_t i = ab; i < http_client.laengen.size (); i++) g = std::max (g, http_client.laengen[i]); return g; }
static std::string muster (size_t n) { std::string r; for (size_t i = 0; i < n; i++) r += (char) ('a' + (i * 7 + i / 26) % 26); return r; }
static const size_t VOLL = (size_t) -1;

int main (void)
{
    printf ("c65w (%s)\n", BEZEICHNUNG);
    /* --- W1-W3: http_flush (), ganze Stuecke */
    struct { size_t n; std::vector<size_t> soll; } G[] = { { 1072, { 536, 536 } }, { 1000, { 536, 464 } }, { 536, { 536 } }, { 1, { 1 } } };
    for (auto & g : G)
    {
        neu (); std::string in = muster (g.n);
        http_send (in.c_str ()); http_flush ();
        fall (("W flush " + std::to_string (g.n) + " Byte: Stuecke").c_str (), http_client.laengen == g.soll, L ());
        fall (("W flush " + std::to_string (g.n) + " Byte: ausgeliefert = Eingabe").c_str (), http_client.empfangen == in, std::to_string (http_client.empfangen.size ()));
    }
    {   /* lange Antwort ueber http_send (): Pufferwechsel bei 1072 */
        neu (); std::string in = muster (5000);
        for (size_t i = 0; i < in.size (); i += 333) http_send (in.substr (i, 333).c_str ());
        http_flush ();
        fall ("W http_send 5000 Byte: nie mehr als 536 je write ()", groesste () <= 536 && http_client.laengen.size () == 10, L ());
        fall ("W http_send 5000 Byte: ausgeliefert = Eingabe, kein Verlust", http_client.empfangen == in && http_write_lost_blocks == 0, std::to_string (http_client.empfangen.size ()));
    }
    /* --- Fortschrittsregel in http_flush () */
    struct R { const char * name; std::vector<Versuch> plan; std::vector<size_t> soll; size_t ankommen; bool verlust; const char * ms; };
    R regeln[] = {
        { "nichts -> Ende",                          { { 0, 1000, false, 1072 } },                                        { 536 },            0,    true,  "1000" },
        { "teilweise -> Rest des Stuecks, dann weiter", { { 200, 10, false, 1072 }, { VOLL, 0, false, 1072 }, { VOLL, 0, false, 1072 } }, { 536, 336, 536 }, 1072, false, nullptr },
        { "teilweise, Nachversuch teilweise -> Ende", { { 200, 1000, false, 1072 }, { 100, 1000, false, 1072 } },          { 536, 336 },       300,  true,  "2000" },
        { "teilweise, Nachversuch nichts -> Ende",    { { 200, 1000, false, 1072 }, { 0, 1000, false, 1072 } },            { 536, 336 },       200,  true,  "2000" },
        { "ein Nachversuch je Aufruf, nicht je Stueck", { { 200, 1000, false, 1072 }, { VOLL, 0, false, 1072 }, { 100, 1000, false, 1072 } }, { 536, 336, 536 }, 636, true, "2000" },
        { "teilweise und Gegenstelle weg -> Ende",   { { 200, 10, true, 1072 } },                                         { 536 },            200,  true,  "10" },
        { "zweites Stueck nichts -> Ende",           { { VOLL, 0, false, 1072 }, { 0, 1000, false, 1072 } },              { 536, 536 },       536,  true,  "1000" },
    };
    for (auto & r : regeln)
    {
        neu (); uint16_t vb = http_write_lost_blocks; std::string in = muster (1072);
        for (auto & x : r.plan) http_client.plan.push_back (x);
        http_send (in.c_str ()); http_flush ();
        std::string z = zeile ("- http write lost");
        std::string n = std::string ("W Regel ") + r.name;
        fall ((n + ": Folge der write ()-Laengen").c_str (), http_client.laengen == r.soll, L ());
        fall ((n + ": angekommen").c_str (), http_client.empfangen == in.substr (0, r.ankommen), std::to_string (http_client.empfangen.size ()));
        fall ((n + ": Verlust gezaehlt, eine Zeile").c_str (), r.verlust ? (http_write_lost_blocks == vb + 1 && zeilen ("- http write lost") == 1) : (http_write_lost_blocks == vb && Serial.lines.empty ()),
              std::to_string (http_write_lost_blocks - vb) + " / " + std::to_string (zeilen ("- http write lost")));
        if (r.ms) fall ((n + ": ms (schlimmster Fall <= 2 x Frist)").c_str (), feld (z, "ms=") == r.ms, z);
        if (r.verlust) fall ((n + ": Verlustbyte = Rest").c_str (), feld (z, "lost ") == std::to_string (1072 - r.ankommen), z);
    }
    /* --- Dateiauslieferung */
    std::string datei = muster (3000);
    datei[0] = '\x1f'; datei[1] = '\x8b';                            /* R2 C70: gzip-Kennung - der ETag-Weg laeuft mit */
    LittleFS.f["/app-app.js.gz"] = datei;
    {
        neu ();
        uint_fast8_t rtc = SENDFS ("/app-app.js.gz", "application/javascript", 1);
        size_t kopf = http_client.empfangen.size () - std::min<size_t> (http_client.empfangen.size (), 3000);
        fall ("W fs vollstaendig: nie mehr als 536 je write ()", groesste () <= 536, L ());
        fall ("W fs vollstaendig: 1 + 6 write () (Kopf, 3 Bloecke zu je 2 Stuecken)", http_client.laengen.size () == 7, L ());
        fall ("W fs vollstaendig: Datei unverfaelscht, Rueckgabe 1", rtc == 1 && http_client.empfangen.compare (kopf, 3000, datei) == 0, std::to_string (http_client.empfangen.size ()));
        fall ("W fs vollstaendig (R2 C70): Kopf HTTP/1.1 mit ETag im ersten write ()", http_client.empfangen.compare (0, 17, "HTTP/1.1 200 OK\r\n") == 0 &&
              http_client.empfangen.find ("\r\nETag: \"3000-") < kopf && http_client.laengen[0] == kopf, L ());
    }
    struct FR { const char * name; std::vector<Versuch> plan; const char * w; size_t writes; };
    FR fr[] = {
        { "zweites Stueck kurz: w = Byte dieses Blocks", { { VOLL, 0, false, 1072 }, { 100, 2000, false, 1072 } }, "636", 3 },
        { "erstes Stueck kurz: kein Nachversuch in der Datei", { { 100, 2000, false, 1072 } }, "100", 2 },
        { "erstes Stueck nichts", { { 0, 2000, false, 1072 } }, "0", 2 },
    };
    for (auto & f : fr)
    {
        neu (); http_client.plan.push_back ({ VOLL, 0, false, 1072 });    /* Kopfzeilen */
        for (auto & x : f.plan) http_client.plan.push_back (x);
        uint_fast8_t rtc = SENDFS ("/app-app.js.gz", "application/javascript", 1);
        std::string z = zeile ("- fs write short: ");
        std::string n = std::string ("W fs ") + f.name;
        fall ((n + ": n=1072, w=").c_str (), feld (z, "n=") == "1072" && feld (z, "w=") == f.w, z);
        fall ((n + ": Zahl der write ()").c_str (), http_client.laengen.size () == f.writes, L ());
        fall ((n + ": Rueckgabe 2, eine Zeile").c_str (), rtc == 2 && zeilen ("- fs write short: ") == 1, std::to_string (rtc));
        fall ((n + ": ms <= eine Frist").c_str (), feld (z, "ms=") == "2000", z);
    }
    return ende ("c65w", BEZEICHNUNG);
}
