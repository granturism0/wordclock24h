/* lwIP-Attrappe fuer die E4-Pruefstaende (Paket 2026-10-13, design-e4.md 19.2, 19.7). Nur, was die ausgeschnittenen
 * Stuecke aus http.cpp brauchen: die Listen tcp_active_pcbs und tcp_tw_pcbs, enum tcp_state (lwip/tcpbase.h:56-68),
 * tcp_kill_timewait () und das Makro TCP_TW_LIMIT WOERTLICH aus dem Core 3.1.2 (lwipopts.h:3677-3689).
 * tcp_kill_timewait () bildet upstream lwIP 2.1 nach: Es bricht das AELTESTE PCB in TIME_WAIT ab (groesstes
 * tcp_ticks - tmr); neue PCBs kommen vorne in die Liste (TCP_REG). Der Pruefstand fuehrt das Alter als Feld "alter".
 */
#ifndef LWIP_ATTRAPPE_H
#define LWIP_ATTRAPPE_H
typedef uint32_t u32_t;
typedef uint16_t u16_t;                                         /* R1.1 I-5 (Paket 2026-10-15) */
typedef uint8_t  u8_t;
typedef struct { u32_t addr; } ip_addr_t;
#define ip_2_ip4(ipaddr)                ((const ip_addr_t *) (ipaddr))
#define ip4_addr4(ipaddr)               (((const u8_t *) (&(ipaddr)->addr))[3])
#define TCP_SLOW_INTERVAL               500                     /* 2 x TCP_TMR_INTERVAL, tcp_priv.h:127-129 */
static u32_t tcp_ticks = 0;
enum tcp_state { CLOSED = 0, LISTEN = 1, SYN_SENT = 2, SYN_RCVD = 3, ESTABLISHED = 4, FIN_WAIT_1 = 5, FIN_WAIT_2 = 6,
                 CLOSE_WAIT = 7, CLOSING = 8, LAST_ACK = 9, TIME_WAIT = 10 };
struct tcp_pcb { struct tcp_pcb * next; enum tcp_state state; u32_t alter;
                 void * callback_arg; ip_addr_t remote_ip; u16_t remote_port; u32_t tmr; u8_t nrtx; };   /* R1.1 I-5; c72i prueft die Felder */
static struct tcp_pcb * tcp_active_pcbs = nullptr;
static struct tcp_pcb * tcp_tw_pcbs = nullptr;
static struct tcp_pcb   pcb_pool[32];
static int              tw_kills = 0;
static std::vector<u32_t> tw_getoetet;                          /* Alter der abgeraeumten PCBs, in Reihenfolge */
static int              tw_beim_kill_zaehlen = 0;               /* tcp_kill_timewait () wurde gerufen */

static void tcp_kill_timewait (void)
{
    struct tcp_pcb ** pp, ** alt = nullptr;
    tw_beim_kill_zaehlen++;
    for (pp = &tcp_tw_pcbs; *pp; pp = &(*pp)->next)
        if (! alt || (*pp)->alter >= (*alt)->alter) alt = pp;
    if (alt) { tw_getoetet.push_back ((*alt)->alter); *alt = (*alt)->next; tw_kills++; }
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

/* Listen aufbauen: aktiv nach Zustaenden, TIME_WAIT mit n Eintraegen, vorne das juengste (alter 1), hinten das aelteste (n) */
static void lwip_setzen (const std::vector<enum tcp_state> & aktiv, int tw)
{
    int k = 0;
    tcp_active_pcbs = nullptr; tcp_tw_pcbs = nullptr; tw_getoetet.clear ();
    for (auto it = aktiv.rbegin (); it != aktiv.rend (); ++it) { pcb_pool[k] = { tcp_active_pcbs, *it, 0 }; tcp_active_pcbs = &pcb_pool[k++]; }
    for (int i = tw; i >= 1; i--) { pcb_pool[k] = { tcp_tw_pcbs, TIME_WAIT, (u32_t) i }; tcp_tw_pcbs = &pcb_pool[k++]; }
}
static int tw_laenge (void) { int n = 0; for (struct tcp_pcb * p = tcp_tw_pcbs; p; p = p->next) n++; return n; }
#endif
