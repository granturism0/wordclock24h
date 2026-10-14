/* Pruefstand c65b (Paket 2026-10-13, E3.2, AKR.2; design.md 18.3).
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
#include "fsfile.inc"

/* ---- Hilfen */
static void neu (void) { http_write_broken = 0; http_client = ClientB (); Serial.lines.clear (); ring.clear (); http_response_len = 0; http_response[0] = 0; }
static int zeilen (const char * praefix) { int n = 0; for (auto & l : Serial.lines) if (! l.compare (0, strlen (praefix), praefix)) n++; return n; }
static std::string zeile (const char * praefix) { for (auto & l : Serial.lines) if (! l.compare (0, strlen (praefix), praefix)) return l; return ""; }
static bool im_ring (const std::string & z) { return std::find (ring.begin (), ring.end (), z) != ring.end (); }
static std::string feld (const std::string & z, const char * k) { size_t p = z.find (k); if (p == std::string::npos) return "(fehlt)"; p += strlen (k); return z.substr (p, z.find (' ', p) - p); }
struct Stand { uint32_t bytes; uint16_t blocks, gone, failed, fsw; };
static Stand stand (void) { return { http_write_lost_bytes, http_write_lost_blocks, http_write_lost_gone, http_write_lost_failed, http_fs_write_short }; }

struct Lage { const char * name; std::vector<Versuch> plan; const char * snd; const char * ms; bool gone; const char * snd_fs; };

int main (void)
{
    printf ("c65b (%s)\n", BEZEICHNUNG);
    std::string block (1000, 'x');

    /* vier Lagen der Entscheidungsregel, je fuer http_flush () (API) */
    Lage lagen[] = {
        { "Gegenstelle weg",  { { 300, 40, true, 1072 } },                           "1072", "40",   true,  "1072" },
        { "Heap",             { { 0, 1000, false, 1072 } },                          "1072", "1000", false, "1072" },
        { "Quittung fehlt",   { { 0, 1000, false, 0 } },                             "0",    "1000", false, "0" },
        { "unklar",           { { 200, 50, false, 600 }, { 0, 1000, false, 500 } },  "500",  "1050", false, "600" },   /* fs: ein Versuch je Block */
    };
    for (auto & L : lagen)
    {
        std::string n = std::string ("flush/") + L.name;
        neu (); Stand v = stand ();
        for (auto & x : L.plan) http_client.plan.push_back (x);
        ESP.blk_seq = { 7000, 1234 };                   /* erst der Messpunkt flush, dann die Erhebung nach dem Versuch */
        http_send (block.c_str ()); http_flush ();
        http_send (block.c_str ()); http_flush ();       /* zweiter Block derselben Verbindung: keine zweite Zeile */
        std::string z = zeile ("- http write lost");
        Stand s = stand ();
        fall ((n + ": genau eine Zeile je Verbindung").c_str (), zeilen ("- http write lost") == 1, std::to_string (zeilen ("- http write lost")));
        fall ((n + ": Mitschnitt und Ring textgleich").c_str (), ! z.empty () && im_ring (z) && ring.size () == 1, z);
        fall ((n + ": blk nach dem letzten Versuch").c_str (), feld (z, "blk=") == "1234", z);
        fall ((n + ": snd").c_str (), feld (z, "snd=") == L.snd, z);
        fall ((n + ": ms vom ersten bis zum letzten Versuch").c_str (), feld (z, "ms=") == L.ms, z);
        fall ((n + ": Muster bleiben gueltig").c_str (), z.compare (0, 18, "- http write lost ") == 0 && z.find (" gone=") != std::string::npos && z.find (" fail=") != std::string::npos, z);
        fall ((n + ": Zaehler gone/fail").c_str (), L.gone ? (s.gone == v.gone + 2 && s.failed == v.failed) : (s.failed == v.failed + 2 && s.gone == v.gone),
              "gone " + std::to_string (s.gone - v.gone) + " fail " + std::to_string (s.failed - v.failed));
        fall ((n + ": Laenge <= 119").c_str (), z.size () <= 119, std::to_string (z.size ()));
    }
    {   /* laengste Form */
        neu (); http_write_lost_gone = 65534; http_write_lost_failed = 65535;
        http_client.plan.push_back ({ 0, 4294967295ul, false, 2147483647 });
        ESP.blk_seq = { 1, 4294967295u };
        http_send (block.c_str ()); http_flush ();
        std::string z = zeile ("- http write lost");
        fall ("flush: laengste Form vollstaendig und <= 119", z.size () <= 119 && z.find (" ms=") != std::string::npos && z.find ("blk=4294967295") != std::string::npos, std::to_string (z.size ()) + ": " + z);
        http_write_lost_gone = http_write_lost_failed = 0;
    }

    /* dieselben vier Lagen fuer http_send_fs_file (): Kopfzeilen gehen hinaus, Block 1 auch, Block 2 kurz */
    std::string inhalt (3000, 'g');
    LittleFS.f["/app-app.js.gz"] = inhalt;
    for (auto & L : lagen)
    {
        std::string n = std::string ("fs/") + L.name;
        neu (); Stand v = stand ();
        http_client.plan.push_back ({ (size_t) -1, 1, false, 1072 });     /* Kopfzeilen */
        http_client.plan.push_back ({ (size_t) -1, 5, false, 1072 });     /* Block 1: 1072 */
#ifdef HAVE_CHUNK
        http_client.plan.push_back ({ (size_t) -1, 0, false, 1072 });     /* E4: Block 1, zweites Stueck zu 536 */
#endif
        for (auto & x : L.plan) http_client.plan.push_back (x);
        ESP.blk_seq = { 7000, 7000, 7000, 1234 };      /* Kopfzeilen-flush, fs-Block 1 (Messpunkt), fs-Block 2 entfaellt (jeder achte), Erhebung */
        ESP.blk_seq = { 7000, 7000, 1234 };
        uint_fast8_t rtc = http_send_fs_file ("/app-app.js.gz", "application/javascript", 1);
        std::string z = zeile ("- fs write short: ");
        Stand s = stand ();
        size_t w = 0; for (auto & x : L.plan) w += std::min<size_t> (x.ret, 1072);
        if (L.plan.size () == 2) w = L.plan[0].ret;
        fall ((n + ": Rueckgabe 2").c_str (), rtc == 2, std::to_string (rtc));
        fall ((n + ": genau eine Zeile je Verbindung").c_str (), zeilen ("- fs write short: ") == 1 && zeilen ("- http write lost") == 0,
              std::to_string (zeilen ("- fs write short: ")) + "/" + std::to_string (zeilen ("- http write lost")));
        fall ((n + ": Mitschnitt und Ring textgleich").c_str (), ! z.empty () && im_ring (z), z);
        fall ((n + ": Datei, n und w").c_str (), z.compare (0, 33, "- fs write short: /app-app.js.gz ") == 0 && feld (z, "n=") == "1072", z);
        fall ((n + ": blk/snd").c_str (), feld (z, "blk=") == "1234" && feld (z, "snd=") == L.snd_fs, z);
        fall ((n + ": Zaehler").c_str (), s.blocks == v.blocks + 1 && s.fsw == v.fsw + 1 &&
              (L.gone ? (s.gone == v.gone + 1 && s.failed == v.failed) : (s.failed == v.failed + 1 && s.gone == v.gone)),
              "blocks " + std::to_string (s.blocks - v.blocks) + " fsw " + std::to_string (s.fsw - v.fsw) + " gone " + std::to_string (s.gone - v.gone) + " fail " + std::to_string (s.failed - v.failed));
        uint32_t erw = 3000 - (1072 + (uint32_t) std::stoul (feld (z, "w=") == "(fehlt)" ? "0" : feld (z, "w=")));
        fall ((n + ": write_lost_bytes = nicht ausgelieferte Byte der Datei").c_str (), s.bytes - v.bytes == erw, std::to_string (s.bytes - v.bytes) + " statt " + std::to_string (erw));
        fall ((n + ": Laenge <= 119").c_str (), z.size () <= 119, std::to_string (z.size ()));
        fall ((n + ": Sendezeitgrenze 2000 (M-3)").c_str (), http_client.timeout == 2000, std::to_string (http_client.timeout));
    }
    {   /* Datei vollstaendig: keine Zeile, kein Zaehler */
        neu (); Stand v = stand ();
        uint_fast8_t rtc = http_send_fs_file ("/app-app.js.gz", "application/javascript", 1);
        Stand s = stand ();
        fall ("fs vollstaendig: Rueckgabe 1, keine Zeile, keine Zaehler", rtc == 1 && Serial.lines.empty () && s.blocks == v.blocks && s.fsw == v.fsw,
              std::to_string (rtc) + " " + std::to_string (Serial.lines.size ()));
        fall ("fs vollstaendig: alle Byte beim Client", http_client.empfangen.size () > 3000 && http_client.empfangen.compare (http_client.empfangen.size () - 3000, 3000, inhalt) == 0,
              std::to_string (http_client.empfangen.size ()));
    }
    {   /* Kopfzeilen gehen nicht hinaus: eine Zeile (http write lost), kein Rumpfversuch */
        neu (); Stand v = stand ();
        http_client.plan.push_back ({ 0, 2000, false, 0 });
        uint_fast8_t rtc = http_send_fs_file ("/app-app.js.gz", "application/javascript", 1);
        Stand s = stand ();
        fall ("fs Kopf verloren: genau eine Zeile, Rueckgabe 2", Serial.lines.size () == 1 && zeilen ("- http write lost") == 1 && rtc == 2,
              std::to_string (Serial.lines.size ()) + " rtc " + std::to_string (rtc));
        fall ("fs Kopf verloren: kein Rumpfversuch (M-3, sonst zweimal die Frist)", http_client.writes == 1, std::to_string (http_client.writes) + " write ()");
        fall ("fs Kopf verloren: fs_write_short unveraendert", s.fsw == v.fsw, std::to_string (s.fsw - v.fsw));
    }
    {   /* laengste fs-Zeile mit 31 Zeichen Dateiname */
        neu (); std::string name = "/" + std::string (30, 'n');
        LittleFS.f[name] = inhalt;
        http_client.plan.push_back ({ (size_t) -1, 1, false, 1072 });
#ifdef HAVE_CHUNK
        http_client.plan.push_back ({ (size_t) -1, 0, false, 1072 });     /* E4: erstes Stueck des Blocks voll, das zweite kurz */
        http_client.plan.push_back ({ 535, 4294967295ul, false, 2147483647 });
#else
        http_client.plan.push_back ({ 1071, 4294967295ul, false, 2147483647 });
#endif
        ESP.blk_seq = { 1, 1, 4294967295u };
        http_send_fs_file (name.c_str (), "x", 1);
        std::string z = zeile ("- fs write short: ");
        fall ("fs: laengste Form <= 119 und vollstaendig", z.size () <= 119 && z.find (" ms=4294967295") != std::string::npos, std::to_string (z.size ()) + ": " + z);
    }
    return ende ("c65b", BEZEICHNUNG);
}
