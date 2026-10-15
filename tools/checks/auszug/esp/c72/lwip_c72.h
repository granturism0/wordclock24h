/* lwIP- und Core-Attrappe fuer den Pruefstand c72i (Paket 2026-10-15, R1.1, I-5; design.md 3.1, 8).
 *
 * Nachgebildet, was der echte Code aus http.cpp braucht: WiFiServer mit _listen_pcb und _unclaimed (WiFiServer.h:71-76),
 * accept () wie im Core 3.1.2 (WiFiServer.cpp:136-150: holt den ERSTEN Kontext aus _unclaimed, auch einen toten, und
 * gibt nur ohne Eintrag ab), hasClient () (WiFiServer.cpp:104-108), ClientContext mit next () und state ()
 * (ClientContext.h:98-106, :363-371: CLOSE_WAIT und CLOSING gelten als CLOSED), WiFiClient mit available () (gibt bei
 * 0 ab, WiFiClient.cpp:246-257), connected () (WiFiClient.cpp:327-333), operator bool = available () || connected ()
 * (WiFiClient.cpp:342-345), remotePort () und status () ohne Abgabe.
 *
 * tcp_pcb traegt die Felder, die I-5 liest (lwip/tcp.h:212-305): next, state, callback_arg, remote_ip, remote_port,
 * tmr, nrtx; dazu accepts_pending fuer den Listener (tcp_pcb_listen ist hier per Makro dieselbe Struktur - der Code
 * castet _listen_pcb nur um, eine eigene Struktur braeuchte eine Annahme ueber das Speicherbild). tcp_ticks und
 * TCP_SLOW_INTERVAL wie tcp_priv.h:127-129, :327 (Core 3.1.2: 2 x TCP_TMR_INTERVAL 250 = 500 ms, am Ziel per
 * static_assert belegt). Die Adresse liegt wie im Core in Netzbytefolge; ip4_addr4 () liest das vierte Byte
 * (ip4_addr.h:176-180), unabhaengig vom Rechner. TCP_TW_LIMIT woertlich aus lwipopts.h:3678-3689 (wie
 * c65/lwip_attrappe.h).
 *
 * Abgabepunkte: sdk_abgabe () definiert der Pruefstand; dort (und in http_send ()) kann er die Listen veraendern, um
 * zu zeigen, dass I-5 VOR jeder Abgabe und jeder Ausgabe fertig gelesen hat.
 */
#ifndef LWIP_C72_H
#define LWIP_C72_H
typedef uint32_t u32_t;
typedef uint16_t u16_t;
typedef uint8_t  u8_t;
enum tcp_state { CLOSED = 0, LISTEN = 1, SYN_SENT = 2, SYN_RCVD = 3, ESTABLISHED = 4, FIN_WAIT_1 = 5, FIN_WAIT_2 = 6,
                 CLOSE_WAIT = 7, CLOSING = 8, LAST_ACK = 9, TIME_WAIT = 10 };
typedef struct { u32_t addr; } ip4_addr_t;
typedef ip4_addr_t ip_addr_t;
#define ip_2_ip4(ipaddr)                ((const ip4_addr_t *) (ipaddr))
#define ip4_addr_get_byte(ipaddr, idx)  (((const u8_t *) (&(ipaddr)->addr))[idx])
#define ip4_addr4(ipaddr)               ip4_addr_get_byte (ipaddr, 3)
#define TCP_TMR_INTERVAL                250
#define TCP_SLOW_INTERVAL               (2 * TCP_TMR_INTERVAL)

struct tcp_pcb
{
    ip_addr_t           local_ip;
    ip_addr_t           remote_ip;
    struct tcp_pcb *    next;
    void *              callback_arg;
    enum tcp_state      state;
    u16_t               remote_port;
    u32_t               tmr;
    u8_t                nrtx;
    u8_t                accepts_pending;            /* nur am Listener (tcp_pcb_listen) */
    u32_t               alter;                      /* fuer tcp_kill_timewait (), wie c65/lwip_attrappe.h */
};
#define tcp_pcb_listen tcp_pcb

static struct tcp_pcb * tcp_active_pcbs = nullptr;
static struct tcp_pcb * tcp_tw_pcbs = nullptr;
static u32_t            tcp_ticks = 0;
static int              backlog_gesetzt = -1;
static void tcp_backlog_set (struct tcp_pcb *, u8_t n) { backlog_gesetzt = n; }
static void tcp_kill_timewait (void)
{
    struct tcp_pcb ** pp, ** alt = nullptr;
    for (pp = &tcp_tw_pcbs; *pp; pp = &(*pp)->next)
        if (! alt || (*pp)->alter >= (*alt)->alter) alt = pp;
    if (alt) *alt = (*alt)->next;
}
#ifndef TCP_SND_BUF
#define TCP_SND_BUF (2 * TCP_MSS)
#endif
/* ---- woertlich aus lwipopts.h:3678-3689 (Core 3.1.2) */
#define TCP_TW_LIMIT(l)                \
  if (l) do {                          \
    u32_t count_plus_1 = 1;            \
    struct tcp_pcb* tmp = tcp_tw_pcbs; \
    if (tmp)                           \
      while ((tmp = tmp->next))        \
        ++count_plus_1;                \
    while (--count_plus_1 > (l))       \
      /* kill the oldest */            \
      /* pcb in TW state */            \
      tcp_kill_timewait();             \
  } while (0)

/* ---- Core-Attrappen */
class ClientContext
{
    public:
        ClientContext *     _next = nullptr;
        struct tcp_pcb *    _pcb  = nullptr;
        size_t              rx    = 0;                  /* gepufferte Byte */
        ClientContext * next () const { return _next; }
        ClientContext * next (ClientContext * n) { _next = n; return _next; }
        struct tcp_pcb * getPCB () { return _pcb; }
        size_t getSize () const { return rx; }
        uint16_t getRemotePort () const { return _pcb ? _pcb->remote_port : 0; }
        uint8_t state () const
        {
            if (! _pcb || _pcb->state == CLOSE_WAIT || _pcb->state == CLOSING) return CLOSED;
            return (uint8_t) _pcb->state;
        }
};

struct IPAttrappe { String toString () const { return String ("192.0.2.1"); } };
static int client_stops = 0;
static int client_avail_write = 1072;

class WiFiClient
{
    public:
        ClientContext * _client = nullptr;
        WiFiClient () {}
        explicit WiFiClient (ClientContext * c) : _client (c) {}
        int available () { if (! _client) return 0; int r = (int) _client->getSize (); if (! r) sdk_abgabe (); return r; }
        uint8_t connected () { if (! _client || _client->state () == CLOSED) return 0; return _client->state () == ESTABLISHED || available (); }
        operator bool () { return available () || connected (); }
        uint8_t status () { return _client ? _client->state () : (uint8_t) CLOSED; }
        uint16_t remotePort () { return _client ? _client->getRemotePort () : 0; }
        int availableForWrite () { return (_client && _client->_pcb) ? client_avail_write : 0; }
        int read () { if (_client && _client->rx) { _client->rx--; return 'x'; } return -1; }
        int peek () { return -1; }
        void stop () { client_stops++; }
        void abort () {}
        void flush () {}
        void setNoDelay (bool) {}
        void setTimeout (unsigned long) {}
        IPAttrappe remoteIP () { return IPAttrappe (); }
};

class WiFiServer
{
    protected:
        uint16_t            _port;
        struct tcp_pcb *    _listen_pcb = nullptr;
        ClientContext *     _unclaimed  = nullptr;
    public:
        WiFiServer (uint16_t port) : _port (port) {}
        WiFiClient accept ()
        {
            if (_unclaimed) { WiFiClient r (_unclaimed); _unclaimed = _unclaimed->next (); return r; }
            sdk_abgabe ();                                  /* optimistic_yield (1000) */
            return WiFiClient ();
        }
        bool hasClient () { return _unclaimed != nullptr; }
        /* nur fuer den Pruefstand */
        void t_listen (struct tcp_pcb * l) { _listen_pcb = l; }
        void t_unclaimed (ClientContext * c) { _unclaimed = c; }
        ClientContext * t_unclaimed_get () { return _unclaimed; }
};
#endif
