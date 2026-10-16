/* Core-Attrappe fuer den Pruefstand c71e (Paket 2026-10-15, R1b, L7; design.md 7, 8).
 *
 * Bildet die Semantik des Core 3.1.2 nach, an der C71 haengt - und NICHT die Annahme, die L7 widerlegt:
 *   ClientContext   _refcnt; unref () auf 0 ruft discard_received (), close () und delete (ClientContext.h:115-127).
 *                   close () schliesst nur den PCB und laesst _rx_buf STEHEN (ClientContext.h:73-92).
 *                   _rx_buf ist eine pbuf-Kette; read () gibt einen pbuf erst frei, wenn er ganz gelesen ist.
 *   WiFiClient      connect (host): scheitert der DNS, bleibt der alte Kontext unberuehrt (WiFiClient.cpp:130-138);
 *                   sonst stop () + unref () des alten, neuer Kontext mit ref () (WiFiClient.cpp:145-167).
 *                   stop () = flush () + close () (WiFiClient.cpp:316-325), KEINE Freigabe.
 *                   operator= unref () alt, ref () neu (WiFiClient.cpp:117-128); Kopie ref (); Destruktor unref ().
 *                   available () liest den Pufferstand und gibt erst DANN ab (WiFiClient.cpp:246-257) - an dieser
 *                   Abgabe stellt die Attrappe die Antwort des Servers zu. connected (): CLOSE_WAIT gilt als CLOSED.
 * Heap: Kontext 56 Byte, je pbuf Nutzlast + 16 Byte. heap_belegt zaehlt mit.
 * Freigegebene Kontexte werden NICHT wirklich geloescht, sondern als tot markiert: Jeder spaetere Zugriff zaehlt in
 * tote_zugriffe - so prueft c71e, dass nach der Freigabe kein Code den alten Kontext mehr anfasst.
 */
#ifndef CORE_C71E_H
#define CORE_C71E_H
enum tcp_state { CLOSED = 0, LISTEN = 1, SYN_SENT = 2, SYN_RCVD = 3, ESTABLISHED = 4, FIN_WAIT_1 = 5, FIN_WAIT_2 = 6,
                 CLOSE_WAIT = 7, CLOSING = 8, LAST_ACK = 9, TIME_WAIT = 10 };
#define CTX_BYTES   56
#define PBUF_EXTRA  16
static long heap_belegt = 0, tote_zugriffe = 0, kontexte_neu = 0, kontexte_frei = 0, discards = 0, discard_bytes = 0;

/* Antwort des Servers je Datei */
struct Antwort { bool da = true; bool antwortet = true; std::string text; std::vector<size_t> pbufs; };
static std::map<std::string, Antwort> server;
static std::string letzte_anfrage;

struct ClientContext
{
    bool                    lebt = true;
    int                     refcnt = 0;
    bool                    pcb = true;             /* PCB offen */
    int                     state = ESTABLISHED;
    std::deque<std::string> rx;                     /* _rx_buf: pbuf-Kette */
    size_t                  rx_off = 0;
    const Antwort *         ant = nullptr;
    bool                    zugestellt = false;
    ClientContext () { heap_belegt += CTX_BYTES; kontexte_neu++; }
    void chk () { if (! lebt) tote_zugriffe++; }
    void ref () { chk (); ++refcnt; }
    /* discard_bytes zaehlt die UNGELESENEN Byte (tot_len minus schon gelesener Anfang des ersten pbufs); frei wird die
     * ganze Kette samt Rest des angelesenen ersten pbufs (ClientContext.h:302-314) */
    void discard_received () { long u = 0; for (auto & p : rx) { heap_belegt -= (long) (p.size () + PBUF_EXTRA); u += (long) p.size (); } u -= (long) rx_off; discard_bytes += u; if (! rx.empty ()) discards++; rx.clear (); rx_off = 0; }
    void close () { chk (); pcb = false; }
    void unref ()
    {
        chk ();
        if (--refcnt == 0)
        {
            discard_received (); close ();
            lebt = false; heap_belegt -= CTX_BYTES; kontexte_frei++;          /* "delete this", Speicher bleibt fuer die Zugriffspruefung */
        }
    }
    void zustellen ()                               /* im Systemkontext, an einer Abgabe */
    {
        if (zugestellt || ! ant || ! ant->antwortet) return;
        zugestellt = true; size_t pos = 0;
        for (size_t n : ant->pbufs) { if (pos >= ant->text.size ()) break; std::string s = ant->text.substr (pos, n); pos += s.size (); rx.push_back (s); heap_belegt += (long) (s.size () + PBUF_EXTRA); }
        if (pos < ant->text.size ()) { std::string s = ant->text.substr (pos); rx.push_back (s); heap_belegt += (long) (s.size () + PBUF_EXTRA); }
        state = CLOSE_WAIT;                          /* Connection: close - das FIN folgt den Daten */
    }
    size_t getSize () { chk (); size_t n = 0; for (auto & p : rx) n += p.size (); return n - rx_off; }
    int read ()
    {
        chk ();
        if (rx.empty ()) return -1;
        int ch = (unsigned char) rx.front ()[rx_off++];
        if (rx_off >= rx.front ().size ()) { heap_belegt -= (long) (rx.front ().size () + PBUF_EXTRA); rx.pop_front (); rx_off = 0; }
        return ch;
    }
    uint8_t zustand () { chk (); if (! pcb || state == CLOSE_WAIT || state == CLOSING) return CLOSED; return (uint8_t) state; }
};
static std::vector<ClientContext *> alle_kontexte;
static ClientContext * aktiver = nullptr;           /* zuletzt verbundener Kontext (fuer die Zustellung) */
static void sdk_abgabe (void) { for (auto * c : alle_kontexte) if (c->lebt) c->zustellen (); }

class WiFiClient
{
    public:
        ClientContext * _client = nullptr;
        unsigned long   _timeout = 5000;
        WiFiClient () {}
        WiFiClient (const WiFiClient & o) : _client (o._client), _timeout (o._timeout) { if (_client) _client->ref (); }
        ~WiFiClient () { if (_client) _client->unref (); }
        WiFiClient & operator= (const WiFiClient & o) { if (_client) _client->unref (); _client = o._client; _timeout = o._timeout; if (_client) _client->ref (); return *this; }
        int connect (const char * host, uint16_t)
        {
            if (! strcmp (host, "dns-fehler")) return 0;                       /* hostByName scheitert: alter Kontext bleibt */
            if (_client) { stop (); _client->unref (); _client = nullptr; }
            if (! strcmp (host, "verbindung-fehler")) return 0;
            ClientContext * c = new ClientContext (); alle_kontexte.push_back (c); c->ref (); _client = c; aktiver = c;
            return 1;
        }
        size_t print (const String & s)
        {
            if (! _client) return 0;
            _client->chk ();
            letzte_anfrage = s.s;
            size_t a = s.s.find (' '), b = s.s.find (' ', a + 1); std::string pfad = s.s.substr (a + 1, b - a - 1);
            std::string datei = pfad.substr (pfad.rfind ('/') + 1);
            auto it = server.find (datei);
            static Antwort keine; keine.antwortet = false;
            _client->ant = it != server.end () ? &it->second : &keine;
            return s.s.size ();
        }
        int available () { if (! _client) return 0; int r = (int) _client->getSize (); if (! r) sdk_abgabe (); return r; }
        uint8_t connected () { if (! _client || _client->zustand () == CLOSED) return 0; return _client->zustand () == ESTABLISHED || available (); }
        int read () { if (! _client) return -1; return _client->read (); }
        bool stop (unsigned int = 0) { if (! _client) return true; _client->close (); return true; }
        operator bool () { return available () || connected (); }
};
#endif
