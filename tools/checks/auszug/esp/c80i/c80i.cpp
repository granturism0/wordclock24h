/* Pruefstand c80i (Paket 2026-10-17-c80, Task 1.3; AKI.2-AKI.8, AKI.13 (a), AKI.14 (a); design.md 4.8 samt N1/N2).
 *
 * Uebersetzt die AUSGESCHNITTENEN Originalteile (auszug_c80i.py) auf dem Entwicklungsrechner:
 *   rein.inc     reine Funktionen aus heapkarte.cpp (Blockgang, Toepfe, Ereignisse, Takt, Zeilen)
 *   geraet.inc   Geraeteteil aus heapkarte.cpp (Sperre, Listen, Wegmarken, Ausgabe auf beiden Wegen)
 *   loop.inc     Rumpf von loop () aus ESP-uclock.ino - die echten Messpunkte
 *   heaplog.inc  esp_heap_log () aus ESP-uclock.ino - die echte Heap-Zeile samt Haken fuer die Karte m
 * gegen Attrappen von Core, lwIP und Uhr. Die Szenarien laufen je in einem eigenen Prozess (fork), weil loop () und
 * esp_heap_log () statische Lokale tragen. Typbreiten: Das Instrument rechnet nur mit festen Breiten (uint16_t,
 * uint32_t, int32_t); millis () ist hier uint32_t wie auf dem Geraet.
 * Exit 0 alles OK, 1 mindestens ein FEHLER, 2 KAPUTT.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <algorithm>

#ifndef BEZEICHNUNG
#define BEZEICHNUNG "neu"
#endif

/* ---------------------------------------------------------------- Pruefbuchhaltung */
static int g_fails = 0, g_checks = 0;
static void chk (bool ok, const char * fall, const char * fmt, ...)
{
    g_checks++;
    if (ok) return;
    g_fails++;
    char b[400]; va_list ap; va_start (ap, fmt); vsnprintf (b, sizeof b, fmt, ap); va_end (ap);
    printf ("FEHLER %s: %s\n", fall, b);
}

/* ---------------------------------------------------------------- Attrappen des Cores */
#define PROGMEM
#define PSTR(s) (s)
typedef const char * PGM_P;
#define snprintf_P snprintf
#define vsnprintf_P vsnprintf
#define memcpy_P memcpy

static uint32_t sim_ms = 0, sim_us = 0;
static uint32_t millis (void) { return sim_ms; }
static uint32_t micros (void) { sim_us += 37; return sim_us; }          // ein Blockgang "dauert" 37 us

static int sim_sperre = 0, sim_sperre_max = 0;
static uint32_t sim_rsil (int lvl) { if (lvl != 15) sim_sperre += 100; sim_sperre++; if (sim_sperre > sim_sperre_max) sim_sperre_max = sim_sperre; return 0x1234; }
static void sim_wsr (uint32_t ps) { if (ps == 0x1234) sim_sperre--; }
#define xt_rsil(l)      sim_rsil (l)
#define xt_wsr_ps(ps)   sim_wsr (ps)

/* Synthetischer Haufen: 1'590 Bloecke wie am Geraet (Bericht 1.1). Aufbau aus fake_free (Bloecke Fb = free / 8):
 *   E1 belegt 1+128 (darin liegen die Attrappen der Objekte), E2 frei 129+37, E3 belegt 166+63 (504 Byte),
 *   E4 frei 229+G mit G = Fb - 37, E5 belegt bis 1588, Endwaechter 1589.
 */
#define SIM_NB 1590
static uint8_t  sim_mem[SIM_NB * 8] __attribute__ ((aligned (8)));
struct umm_heap_context_t { void * heap; uint16_t numblocks; };
static umm_heap_context_t sim_ctx = { sim_mem, SIM_NB };
static int      sim_bs = 8;
static int      sim_kaputt = 0;                                             // 0, 1 nb, 2 ord, 3 sum
static uint32_t fake_free = 7840;
static int      sim_getfree_aufrufe = 0;

static void blk_set (uint16_t b, uint16_t next, uint16_t prev) { memcpy (sim_mem + 8 * b, &next, 2); memcpy (sim_mem + 8 * b + 2, &prev, 2); }
static uint16_t blk_next (const uint8_t * mem, uint16_t b) { uint16_t v; memcpy (&v, mem + 8 * b, 2); return v; }
static uint16_t sim_G (void) { return (uint16_t) (fake_free / 8 - 37); }
static void sim_heap_bauen (void)
{
    uint16_t G = sim_G (), e5 = 229 + G;
    blk_set (0, 1, 0);
    blk_set (1, 129, 0);
    blk_set (129, 166 | 0x8000, 1);
    blk_set (166, sim_kaputt == 1 ? 0x7FF0 : sim_kaputt == 2 ? 100 : 229, 129);
    blk_set (229, sim_kaputt == 3 ? 0 : (uint16_t) (e5 | 0x8000), 166);
    if (sim_kaputt != 3) { blk_set (e5, 1589, 229); blk_set (1589, 0, e5); }
}
static umm_heap_context_t * umm_get_current_heap (void) { sim_heap_bauen (); return &sim_ctx; }
static size_t umm_block_size (void) { return (size_t) sim_bs; }

struct SimEsp
{
    uint32_t getFreeHeap (void) { sim_getfree_aufrufe++; return fake_free; }
    uint32_t getMaxFreeBlockSize (void) { uint32_t G = sim_G (); return (G > 37 ? G : 37) * 8; }
} ESP;

struct SimZeile { uint32_t ms; uint32_t lauf; std::string text; };
static std::vector<SimZeile> sim_serial, sim_ring;
static uint32_t sim_lauf = 0;
static void sim_delta (int t);
struct SimSerial
{
    bool hasOverrun (void) { sim_delta (7); return false; }                 // Topf ovr
    size_t println (const char * s) { sim_serial.push_back ({ sim_ms, sim_lauf, s }); return strlen (s); }
    void flush (void) { }
} Serial;
static void stm32_log_append (const char * s) { sim_ring.push_back ({ sim_ms, sim_lauf, s }); }

/* lwIP-Attrappen (nur die Felder, die der Geraeteteil liest) */
struct tcp_pcb { struct tcp_pcb * next; int state; void * callback_arg; };
struct tcp_pcb_listen { struct tcp_pcb_listen * next; uint16_t local_port; };
struct udp_pcb { struct udp_pcb * next; void * recv_arg; };
struct netif { struct netif * next; void * dhcp; };
static struct tcp_pcb * tcp_active_pcbs = 0, * tcp_tw_pcbs = 0;
static struct { struct tcp_pcb_listen * listen_pcbs; } tcp_listen_pcbs = { 0 };
static struct udp_pcb * udp_pcbs = 0;
static struct netif * netif_list = 0;
#define netif_dhcp_data(n) ((n)->dhcp)
struct ClientContext;
class WiFiClient { protected: ClientContext * _client = 0; public: void sim_setze (void * p) { _client = (ClientContext *) p; } };
struct EEPROMClass
{
    uint8_t * _data = 0; int dirty = 0;
    const uint8_t * getConstDataPtr (void) const { return _data; }
    uint8_t * getDataPtr (void) { dirty = 1; return _data; }
} EEPROM;
static uint16_t sim_acc = 0;
static uint16_t http_accept_count_get (void);

/* ---------------------------------------------------------------- das Instrument, unveraendert aus dem Baum */
#include "heapkarte.h"
#ifndef ESP_HEAP_MAP
#error "heapkarte.h ohne ESP_HEAP_MAP"
#endif
#if ! ESP_HEAP_MAP
#error "c80i braucht ESP_HEAP_MAP 1 (fuer 0 gibt es c80i_aus.cpp)"
#endif
#include "rein.inc"
#include "geraet.inc"
static uint16_t http_accept_count_get (void) { return sim_acc; }
WiFiClient openweather_client;

/* ---------------------------------------------------------------- Stummel der Aufrufe in loop () */
static int32_t  sim_skript[HK_TOEPFE];                                     // Aenderung je Topf in diesem Durchlauf
static int      sim_in_w = 0;                                               // Sicht des Pruefstands: Durchlauf in W
static uint32_t sim_t0 = 0;
static int64_t  wahr_netto[HK_TOEPFE];
static uint32_t wahr_rueck[HK_TOEPFE], wahr_ms[HK_TOEPFE];
static void sim_buchen (int t, int32_t d, uint32_t ms)
{
    wahr_netto[t] += d;
    if (d < 0 && (uint32_t) -d > wahr_rueck[t]) { wahr_rueck[t] = (uint32_t) -d; wahr_ms[t] = ms; }
}
static void sim_delta (int t)
{
    int32_t d = sim_skript[t]; sim_skript[t] = 0;
    fake_free = (uint32_t) ((int32_t) fake_free + d);
    if (sim_in_w) sim_buchen (t, d, sim_ms - sim_t0);
}
static uint32_t esp_uart_overruns = 0;
#define ESP_HEAP_LOG                1
#define ESP_HEAP_LOG_INTERVAL       60000UL
static uint32_t heap_low_line_take (void) { sim_delta (HK_TOPF_HLG); return 0xFFFFFFFFUL; }
static size_t tcp_now_tcp_field (char * b, size_t n) { snprintf (b, n, "tcp=0/0/0/0/0/0"); return strlen (b); }
static uint16_t http_no_request_timeouts_get (void) { return 0; }

/* Server: Eine Anfrage legt PCB (aktiv) und ClientContext an (-800); im naechsten Durchlauf ist der Kontext frei (+608),
 * der PCB in TIME_WAIT (192 bleiben) - das Bild aus L501.
 */
static int sim_anfrage = 0, sim_frei_naechster = 0;
static tcp_pcb * sim_pcb_req = 0;
static void wifi_check_if_started (void) { sim_delta (HK_TOPF_WIF); }
static void http_server_loop (void)
{
    if (sim_frei_naechster)
    {
        sim_skript[HK_TOPF_SRV] += 608; sim_frei_naechster = 0;
        tcp_active_pcbs = 0; sim_pcb_req->state = 10; sim_pcb_req->callback_arg = 0; sim_pcb_req->next = tcp_tw_pcbs; tcp_tw_pcbs = sim_pcb_req;
    }
    if (sim_anfrage)
    {
        sim_anfrage = 0; sim_acc++; sim_skript[HK_TOPF_SRV] -= 800; sim_frei_naechster = 1;
        sim_pcb_req->state = 5; sim_pcb_req->next = 0; tcp_active_pcbs = sim_pcb_req;
    }
    sim_delta (HK_TOPF_SRV);
}
static void http_ota_restart_poll (void) { sim_delta (HK_TOPF_OTA); }
static void update_cache_poll (void) { sim_delta (HK_TOPF_UPD); }
static void udp_server_loop (void) { sim_delta (HK_TOPF_UDP); }
static void ntp_poll_time (void) { sim_delta (HK_TOPF_NTP); }
static void var_sync_check (void) { sim_delta (HK_TOPF_VAR); }
static void stm_bridge_poll (void) { sim_delta (HK_TOPF_STM); }

#include "heaplog.inc"
static void sim_loop (void)
{
#include "loop.inc"
}

/* ---------------------------------------------------------------- Bezug: Haufen aus einer Eintragsliste */
struct Ein { uint16_t s, n; bool frei; };
struct Bezug { unsigned belegt, frei, fsum, gb, gn, zb, zn, db, dn, nb_, nn; };
static const unsigned NIX = 0xFFFF;
static Bezug bezug (const std::vector<Ein> & E)
{
    Bezug r = { 0, 0, 0, NIX, 0, NIX, 0, NIX, 0, NIX, 0 };
    std::vector<size_t> fr;
    for (size_t i = 0; i < E.size (); i++) { if (E[i].frei) { r.frei++; r.fsum += E[i].n; fr.push_back (i); } else r.belegt++; }
    std::stable_sort (fr.begin (), fr.end (), [&] (size_t a, size_t b) { return E[a].n > E[b].n; });
    if (fr.size () >= 1)
    {
        size_t g = fr[0]; r.gb = E[g].s; r.gn = E[g].n;
        if (g > 0 && ! E[g - 1].frei) { r.db = E[g - 1].s; r.dn = E[g - 1].n; }
        if (g + 1 < E.size () && ! E[g + 1].frei) { r.nb_ = E[g + 1].s; r.nn = E[g + 1].n; }
    }
    if (fr.size () >= 2) { r.zb = E[fr[1]].s; r.zn = E[fr[1]].n; }
    return r;
}
/* Kette wie umm_malloc: Block 0 Kopf, Eintraege ab 1, Endwaechter n-1 mit next 0; Nutzlast mit Muell */
static std::vector<uint8_t> haufen (const std::vector<Ein> & E, uint16_t n)
{
    std::vector<uint8_t> m ((size_t) n * 8);
    for (auto & c : m) c = (uint8_t) (rand () & 0xFF);
    auto set = [&] (uint16_t b, uint16_t nx, uint16_t pv) { memcpy (&m[8 * b], &nx, 2); memcpy (&m[8 * b + 2], &pv, 2); };
    set (0, E.empty () ? (uint16_t) (n - 1) : E[0].s, 0);
    uint16_t pv = 0;
    for (auto & e : E) { set (e.s, (uint16_t) ((e.s + e.n) | (e.frei ? 0x8000 : 0)), pv); pv = e.s; }
    set ((uint16_t) (n - 1), 0, pv);
    return m;
}
/* umm_info () aus umm_info.c:62-112, woertlich in der Logik: maxFreeContiguousBlocks und freeBlocks */
static void umm_info_bezug (const std::vector<uint8_t> & m, unsigned & maxfrei, unsigned & frei)
{
    uint16_t b = blk_next (m.data (), 0) & 0x7FFF; maxfrei = 0; frei = 0;
    while (blk_next (m.data (), b) & 0x7FFF)
    {
        unsigned cur = (blk_next (m.data (), b) & 0x7FFF) - b;
        if (blk_next (m.data (), b) & 0x8000) { frei += cur; if (maxfrei < cur) maxfrei = cur; }
        b = blk_next (m.data (), b) & 0x7FFF;
    }
}
static std::vector<Ein> zufall_eintraege (uint16_t n)
{
    std::vector<Ein> E; uint16_t s = 1; bool vorher_frei = false;
    while (s < n - 1)
    {
        uint16_t rest = (uint16_t) (n - 1 - s), len = (uint16_t) (1 + rand () % (rand () % 4 ? 12 : 300));
        if (len > rest) len = rest;
        bool fr = vorher_frei ? false : (rand () % 2);
        E.push_back ({ s, len, fr }); vorher_frei = fr; s = (uint16_t) (s + len);
    }
    return E;
}
static std::string blk (unsigned b, unsigned n) { char t[32]; if (b == NIX || ! n) return "-"; snprintf (t, sizeof t, "%u+%u", b, n * 8); return t; }
static std::string karte_soll (char a, const Bezug & r, long ms)
{
    char t[200];
    snprintf (t, sizeof t, "- heap karte %c ent=%u/%u fsum=%u gross=%s zweit=%s davor=%s danach=%s", a, r.belegt, r.frei, r.fsum * 8,
              blk (r.gb, r.gn).c_str (), blk (r.zb, r.zn).c_str (), blk (r.db, r.dn).c_str (), blk (r.nb_, r.nn).c_str ());
    std::string s = t;
    if (ms >= 0) { snprintf (t, sizeof t, " ms=%ld", ms); s += t; }
    return s;
}
static bool gleich_bezug (const HK_KARTE & k, const Bezug & r)
{
    return k.abbruch == 0 && k.belegt == r.belegt && k.frei == r.frei && k.fsum == r.fsum && k.gross_b == r.gb && k.gross_n == r.gn
        && k.zweit_b == r.zb && k.zweit_n == r.zn && k.davor_b == r.db && k.davor_n == r.dn && k.danach_b == r.nb_ && k.danach_n == r.nn;
}
static std::string zeile_von (const HK_ZUSTAND * z, uint8_t bit) { char b[120]; size_t n = hk_zeile (z, bit, b, sizeof b); return std::string (b, n); }

/* ---------------------------------------------------------------- Fall 1-5, 9, 10: Blockgang an synthetischen Haufen */
static void fall_gang (void)
{
    struct { const char * name; uint16_t n; std::vector<Ein> E; } F[] = {
        { "Fall 1 frischer Haufen", 1590, { { 1, 1588, true } } },
        { "Fall 2 zwei freie, getrennt", 100, { { 1, 10, false }, { 11, 20, true }, { 31, 5, false }, { 36, 30, true }, { 66, 33, false } } },
        { "Fall 3 gross am Ende (danach=-)", 100, { { 1, 10, false }, { 11, 5, true }, { 16, 20, false }, { 36, 63, true } } },
        { "Fall 3 gross am Anfang (davor=-)", 100, { { 1, 50, true }, { 51, 10, false }, { 61, 5, true }, { 66, 33, false } } },
        { "Fall 4 Gleichstand gross", 100, { { 1, 10, false }, { 11, 30, true }, { 41, 5, false }, { 46, 30, true }, { 76, 23, false } } },
        { "Fall 4 Gleichstand zweit", 100, { { 1, 8, true }, { 9, 2, false }, { 11, 10, true }, { 21, 2, false }, { 23, 10, true }, { 33, 2, false }, { 35, 12, true }, { 47, 52, false } } },
        { "Fall 5 L499 sauber (B)", 1590, { { 1, 128, false }, { 129, 37, true }, { 166, 63, false }, { 229, 888, true }, { 1117, 472, false } } },
        { "Fall 5 L499 zerstueckelt (A)", 1590, { { 1, 128, false }, { 129, 100, true }, { 229, 63, false }, { 292, 825, true }, { 1117, 472, false } } },
        { "Fall 9 kein freier Eintrag", 50, { { 1, 48, false } } },
    };
    for (auto & f : F)
    {
        auto m = haufen (f.E, f.n); HK_KARTE k; Bezug r = bezug (f.E);
        uint8_t ab = hk_gang (m.data (), f.n, 8, &k);
        chk (ab == 0 && gleich_bezug (k, r), f.name, "ab=%u ent=%u/%u fsum=%u gross=%u+%u zweit=%u+%u davor=%u+%u danach=%u+%u (soll %s)",
             ab, k.belegt, k.frei, k.fsum, k.gross_b, k.gross_n, k.zweit_b, k.zweit_n, k.davor_b, k.davor_n, k.danach_b, k.danach_n, karte_soll ('?', r, -1).c_str ());
        unsigned mx, fr; umm_info_bezug (m, mx, fr);
        chk (k.gross_n * 8u == mx * 8u && k.fsum * 8u == fr * 8u, f.name, "AKI.3: gross %u = max %u, fsum %u = free %u", k.gross_n * 8, mx * 8, k.fsum * 8, fr * 8);
    }
    /* Fall 5: Zeilen exakt, und die Groessen aus L499 */
    {
        auto m = haufen (F[7].E, 1590); HK_ZUSTAND z; memset (&z, 0, sizeof z);
        hk_gang (m.data (), 1590, 8, &z.karte[HK_K_M]);
        std::string s = zeile_von (&z, 10);
        chk (s == "- heap karte m ent=3/2 fsum=7400 gross=292+6600 zweit=129+800 davor=229+504 danach=1117+3776", "Fall 5 Zeile m (A)", "%s", s.c_str ());
    }
    /* Fall 9: Frei-Bit in Bit 15, Endwaechter next 0, Block 0 Kopf; Zeiger -> Block ueber (ptr - heap) / 8 */
    {
        std::vector<uint8_t> m (8 * 5);
        uint16_t v;
        v = 1; memcpy (&m[0], &v, 2); v = 3 | 0x8000; memcpy (&m[8], &v, 2); v = 4; memcpy (&m[24], &v, 2); v = 0; memcpy (&m[32], &v, 2);
        HK_KARTE k; uint8_t ab = hk_gang (m.data (), 5, 8, &k);
        chk (ab == 0 && k.frei == 1 && k.belegt == 1 && k.gross_b == 1 && k.gross_n == 2 && k.danach_b == 3, "Fall 9 Blockform", "ab=%u frei=%u belegt=%u gross=%u+%u", ab, k.frei, k.belegt, k.gross_b, k.gross_n);
        chk (hk_index (&m[8 * 3 + 4], m.data (), 5) == 3 && hk_index (&m[0], m.data (), 5) == 0 && hk_index (&m[39], m.data (), 5) == 4, "Fall 9 Zeiger -> Block", "Index falsch");
        chk (hk_index (0, m.data (), 5) == HK_NIX && hk_index (&m[0] + 40, m.data (), 5) == HK_FREMD && hk_index (&m[0] - 1, m.data (), 5) == HK_FREMD, "Fall 9 Zeiger ausserhalb", "NIX/FREMD falsch");
    }
    /* Fall 10: Gleichheit und Bezug an 3'000 Zufallshaufen */
    srand (80);
    int falsch = 0;
    for (int i = 0; i < 3000 && falsch < 3; i++)
    {
        uint16_t n = (uint16_t) (3 + rand () % 1588);
        auto E = zufall_eintraege (n); auto m = haufen (E, n); HK_KARTE k; Bezug r = bezug (E); unsigned mx, fr;
        uint8_t ab = hk_gang (m.data (), n, 8, &k); umm_info_bezug (m, mx, fr);
        bool ok = ab == 0 && gleich_bezug (k, r) && k.gross_n == mx && k.fsum == fr;
        if (! ok) falsch++;
        chk (ok, "Fall 10 Zufallshaufen", "n=%u ab=%u gross=%u+%u (umm max %u) fsum=%u (umm %u) zweit=%u+%u davor=%u danach=%u", n, ab, k.gross_b, k.gross_n, mx, k.fsum, fr, k.zweit_b, k.zweit_n, k.davor_b, k.danach_b);
    }
}

/* ---------------------------------------------------------------- Fall 6, 11: beschaedigte Haufen */
static void fall_kaputt (void)
{
    std::vector<Ein> E = { { 1, 10, false }, { 11, 20, true }, { 31, 68, false } };
    auto m0 = haufen (E, 100);
    struct { const char * name; uint16_t blk, wert; uint16_t bs, n; uint8_t soll; const char * zeile; } K[] = {
        { "Fall 6 bs", 0, 0, 4, 100, HK_AB_BS, "- heap karte S abbruch=bs ms=0" },
        { "Fall 6 nb (Folgeindex)", 11, 0x8000 | 0x7FF0, 8, 100, HK_AB_NB, "- heap karte S abbruch=nb ms=0" },
        { "Fall 6 nb (erster Index)", 0, 200, 8, 100, HK_AB_NB, "- heap karte S abbruch=nb ms=0" },
        { "Fall 6 ord (Zyklus)", 31, 11, 8, 100, HK_AB_ORD, "- heap karte S abbruch=ord ms=0" },
        { "Fall 11 sum (Ende zu frueh)", 31, 0, 8, 100, HK_AB_SUM, "- heap karte S abbruch=sum ms=0" },
        { "Fall 11 sum (Blockzahl zu gross)", 0, 1, 8, 101, HK_AB_SUM, "- heap karte S abbruch=sum ms=0" },
    };
    for (auto & c : K)
    {
        auto m = m0;
        if (c.blk || c.wert) memcpy (&m[8 * c.blk], &c.wert, 2);
        std::vector<uint8_t> gross (8 * 102); memcpy (gross.data (), m.data (), m.size ());
        HK_ZUSTAND z; memset (&z, 0, sizeof z);
        uint8_t ab = hk_gang (gross.data (), c.n, c.bs, &z.karte[HK_K_S]);
        std::string s = zeile_von (&z, 5);
        chk (ab == c.soll && z.karte[HK_K_S].abbruch == c.soll && s == c.zeile, c.name, "abbruch=%u (soll %u), Zeile \"%s\"", ab, c.soll, s.c_str ());
    }
    /* Sperre auf JEDEM Weg geloest (Geraeteteil hk_karte_erfassen () ueber heapkarte_heapzeile ()), je in einem Kind */
    struct { const char * name; int kaputt, bs; uint8_t soll; } S[] = {
        { "Fall 6/11 Sperre: intakt", 0, 8, 0 }, { "Fall 6 Sperre: nb", 1, 8, HK_AB_NB }, { "Fall 6 Sperre: ord", 2, 8, HK_AB_ORD },
        { "Fall 11 Sperre: sum", 3, 8, HK_AB_SUM }, { "Fall 6 Sperre: bs", 0, 4, HK_AB_BS } };
    for (auto & c : S)
    {
        fflush (stdout);
        pid_t p = fork ();
        if (! p)
        {
            g_fails = 0; sim_kaputt = c.kaputt; sim_bs = c.bs;
            heapkarte.merker = HK_M_GESTARTET; sim_ms = 100000;
            heapkarte_heapzeile ();
            chk (sim_sperre == 0 && sim_sperre_max == 1, c.name, "Sperrtiefe danach %d, hoechstens %d (soll 0 und 1)", sim_sperre, sim_sperre_max);
            chk (heapkarte.karte[HK_K_M].abbruch == c.soll, c.name, "abbruch=%u (soll %u)", heapkarte.karte[HK_K_M].abbruch, c.soll);
            chk (heapkarte.offen & HK_O_M, c.name, "m-Karte nicht zur Ausgabe frei");
            fflush (stdout); _exit (g_fails ? 1 : 0);
        }
        int st; waitpid (p, &st, 0); g_checks += 3;
        if (! WIFEXITED (st) || WEXITSTATUS (st)) { g_fails++; if (! WIFEXITED (st)) printf ("FEHLER %s: Kind abgebrochen\n", c.name); }
    }
}

/* ---------------------------------------------------------------- Fall 7, 13, 21: Zeilenformen und Laengen */
static void fall_zeilen (void)
{
    HK_ZUSTAND z; memset (&z, 0, sizeof z);
    HK_KARTE * k = &z.karte[HK_K_F];
    k->ms = 180000; k->belegt = 1590; k->frei = 1590; k->fsum = 1590; k->gross_b = 1589; k->gross_n = 1590;
    k->zweit_b = 1589; k->zweit_n = 1590; k->davor_b = 1589; k->davor_n = 1590; k->danach_b = 1589; k->danach_n = 1590;
    std::string s = zeile_von (&z, 6);
    chk (s.size () == 118 && s == "- heap karte F ent=1590/1590 fsum=12720 gross=1589+12720 zweit=1589+12720 davor=1589+12720 danach=1589+12720 ms=180000",
         "Fall 21 Karte laengste Form 118", "%zu Zeichen: %s", s.size (), s.c_str ());
    z.karte[HK_K_M] = *k;
    s = zeile_von (&z, 10);
    chk (s.size () == 108 && s.find (" ms=") == std::string::npos, "Fall 21 Karte m ohne ms", "%zu: %s", s.size (), s.c_str ());
    for (int i = 0; i < HK_TOEPFE; i++) { z.topf[i].netto = INT32_MIN; z.topf[i].rueckgang = 65535; z.topf[i].ms = 65535; }
    s = zeile_von (&z, 1);
    chk (s.size () == 99 && s == "- heap topf 1/4 wif=-2147483648/65535@65535 srv=-2147483648/65535@65535 ota=-2147483648/65535@65535",
         "Fall 21 Topf laengste Form 99", "%zu: %s", s.size (), s.c_str ());
    s = zeile_von (&z, 4);
    chk (s == "- heap topf 4/4 stm=-2147483648/65535@65535 sys=-2147483648/65535@65535", "Fall 15 Topf 4/4 stm und sys", "%s", s.c_str ());
    z.t0 = 0xFFFFFFFF; z.n = 0xFFFFFFFF; z.free0 = z.free1 = z.us = 65535;
    s = zeile_von (&z, 0);
    chk (s.size () == 84 && s == "- heap topf kopf t0=4294967295 w=60000 n=4294967295 free0=65535 free1=65535 us=65535", "Fall 21 Kopf laengste Form 84", "%zu: %s", s.size (), s.c_str ());
    HK_TOPF leer = { 0, 0, 0 }; z.topf[HK_TOPF_UPD] = leer; z.topf[HK_TOPF_UDP] = { 504, 0, 0 }; z.topf[HK_TOPF_NTP] = { -504, 504, 7312 };
    s = zeile_von (&z, 2);
    chk (s == "- heap topf 2/4 upd=0/0@- udp=504/0@- ntp=-504/504@7312", "Fall 15 Topf ohne Rueckgang @-", "%s", s.c_str ());

    /* Wegmarken */
    HK_MARKEN * m = &z.marken;
    m->anlass = 'X'; m->lst = 2; m->wet = HK_NIX; m->dhcp = 34; m->eep = HK_FREMD; m->tcp_n = 1; m->tcp_pcb[0] = 6; m->tcp_zst[0] = 10; m->tcp_ctx[0] = HK_NIX;
    m->udp_n = 3; m->udp_pcb[0] = 14; m->udp_ctx[0] = 18; m->udp_pcb[1] = 22; m->udp_ctx[1] = 26; m->udp_pcb[2] = 30; m->udp_ctx[2] = HK_NIX;
    s = zeile_von (&z, 8);
    chk (s == "- heap marken X tcp=6:10/- lst=2 wet=- dhcp=34 eep=x udp=14/18,22/26,30/-", "Fall 13 marken tcp zuerst, - und x", "%s", s.c_str ());
    m->tcp_n = 0; m->udp_n = 0;
    s = zeile_von (&z, 8);
    chk (s == "- heap marken X tcp=- lst=2 wet=- dhcp=34 eep=x udp=-", "Fall 13 marken leere Listen", "%s", s.c_str ());
    m->anlass = 'W'; m->lst = m->wet = m->dhcp = m->eep = 1589; m->tcp_n = 255; m->udp_n = 255;
    for (int i = 0; i < 4; i++) { m->tcp_pcb[i] = m->tcp_ctx[i] = m->udp_pcb[i] = m->udp_ctx[i] = 1589; m->tcp_zst[i] = 10; }   /* lwIP-Zustaende <= 10 */
    s = zeile_von (&z, 8);
    size_t plus = s.rfind (" +");
    unsigned weg = plus == std::string::npos ? 0 : (unsigned) atoi (s.c_str () + plus + 2);
    size_t ntcp = 0, nudp = 0; { size_t a = s.find ("tcp="), b = s.find (" lst="); for (size_t i = a; i < b; i++) ntcp += s[i] == ':'; }
    { size_t a = s.find ("udp="); for (size_t i = a; i < s.size () && i < plus; i++) nudp += s[i] == '/'; }
    chk (s.size () <= 119 && s.find (" lst=1589 wet=1589 dhcp=1589 eep=1589") != std::string::npos && weg == 510 - ntcp - nudp && ntcp == 4,
         "Fall 13 marken Ueberlaenge +<n>", "%zu Zeichen, tcp %zu, udp %zu, +%u: %s", s.size (), ntcp, nudp, weg, s.c_str ());
    m->tcp_n = 2; m->udp_n = 6;
    for (int i = 0; i < 4; i++) { m->tcp_pcb[i] = m->udp_pcb[i] = m->udp_ctx[i] = 1589; m->tcp_ctx[i] = HK_NIX; m->tcp_zst[i] = 10; }
    s = zeile_von (&z, 8);
    chk (s.size () <= 119 && s.find ("tcp=1589:10/-,1589:10/- ") != std::string::npos && s.find (" +") != std::string::npos, "Fall 21 marken realistisch voll", "%zu: %s", s.size (), s.c_str ());
}

/* ---------------------------------------------------------------- Fall 15, 17, 18, 19: reine Rechnungen an Folgen */
static void fall_toepfe (void)
{
    HK_TOPF t = { 0, 0, 0 };
    hk_topf_buchen (&t, -100, 10); hk_topf_buchen (&t, 60, 20); hk_topf_buchen (&t, -300, 30); hk_topf_buchen (&t, -300, 40); hk_topf_buchen (&t, -200, 50);
    chk (t.netto == -840 && t.rueckgang == 300 && t.ms == 30, "Fall 15 Topf netto/rueckgang/ms", "netto=%d rueck=%u ms=%u (soll -840/300/30: ms gehoert zum GROESSTEN, bei Gleichstand zum ersten)", t.netto, t.rueckgang, t.ms);
    HK_TOPF u = { INT32_MIN + 5, 0, 0 }; hk_topf_buchen (&u, -10, 1);
    chk (u.netto == INT32_MIN && u.rueckgang == 10, "Fall 15 Topf saettigt", "netto=%d", u.netto);
    HK_TOPF v = { 0, 0, 0 }; hk_topf_buchen (&v, 504, 5); hk_topf_buchen (&v, 0, 6);
    chk (v.netto == 504 && v.rueckgang == 0, "Fall 15 Topf ohne Rueckgang", "netto=%d rueck=%u", v.netto, v.rueckgang);
}

struct Schritt { uint32_t ms; uint16_t frei; uint8_t aktive, tw; uint16_t acc; };
static std::vector<uint8_t> folge (HK_ZUSTAND & z, const std::vector<Schritt> & F)
{
    std::vector<uint8_t> r; memset (&z, 0, sizeof z);
    for (auto & s : F) { uint8_t e = hk_anfang (&z, 8000 + s.ms, s.frei, s.aktive, s.tw, s.acc); hk_erfasst (&z, e, s.ms); r.push_back (e); }
    return r;
}
static int erstes (const std::vector<uint8_t> & r, uint8_t bit) { for (size_t i = 0; i < r.size (); i++) if (r[i] & bit) return (int) i; return -1; }
static int anzahl (const std::vector<uint8_t> & r, uint8_t bit) { int n = 0; for (auto e : r) n += (e & bit) != 0; return n; }
static void fall_ereignisse (void)
{
    HK_ZUSTAND z; std::vector<uint8_t> r;
    /* X */
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 100, 7400, 1, 0, 1 }, { 200, 7400, 1, 0, 1 }, { 300, 7400, 0, 1, 1 }, { 400, 7000, 0, 1, 1 }, { 500, 6000, 0, 1, 1 } });
    chk (erstes (r, HK_E_X) == 3 && anzahl (r, HK_E_X) == 1 && (r[3] & HK_E_MARKEN) && z.karte[HK_K_X].ms == 300, "Fall 17 X erst ohne aktive Verbindung, einmal", "X bei %d (soll 3), %d mal", erstes (r, HK_E_X), anzahl (r, HK_E_X));
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 100, 7440, 0, 0, 0 }, { 200, 7432, 0, 0, 0 } });
    chk (erstes (r, HK_E_X) == 2, "Fall 17 X Abstand 400 streng", "X bei %d (soll 2: 7440 = 7840 - 400 loest nicht aus, 7432 schon)", erstes (r, HK_E_X));
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 59999, 7840, 0, 0, 0 }, { 60000, 7000, 0, 0, 0 }, { 60100, 6000, 0, 0, 0 } });
    chk (erstes (r, HK_E_X) == -1 && (r[2] & HK_E_MARKEN) && anzahl (r, HK_E_MARKEN) == 1, "Fall 17 X nicht nach W, Wegmarken dann bei W", "X bei %d, Wegmarken %d mal", erstes (r, HK_E_X), anzahl (r, HK_E_MARKEN));
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 100, 7000, 0, 0, 0 }, { 60005, 7000, 0, 0, 0 } });
    chk (anzahl (r, HK_E_MARKEN) == 1 && (r[1] & HK_E_MARKEN), "Fall 22 Wegmarken genau einmal (bei X, nicht noch einmal bei W)", "%d mal", anzahl (r, HK_E_MARKEN));
    /* T */
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 100, 7840, 0, 0, 0 }, { 200, 7840, 0, 2, 0 }, { 300, 7840, 0, 1, 0 }, { 400, 7840, 0, 0, 0 }, { 500, 7840, 0, 1, 0 }, { 600, 7840, 0, 0, 0 } });
    chk (erstes (r, HK_E_T) == 4 && anzahl (r, HK_E_T) == 1 && z.karte[HK_K_T].ms == 400, "Fall 18 T beim Uebergang auf 0, nicht bei 2->1, einmal", "T bei %d (soll 4), %d mal", erstes (r, HK_E_T), anzahl (r, HK_E_T));
    r = folge (z, { { 0, 7840, 0, 1, 0 }, { 179000, 7840, 0, 1, 0 }, { 181000, 7840, 0, 0, 0 } });
    chk (anzahl (r, HK_E_T) == 0, "Fall 18 T nicht nach 180 s", "%d mal", anzahl (r, HK_E_T));
    r = folge (z, { { 0, 7840, 0, 1, 0 }, { 100, 7840, 0, 0, 0 } });
    chk (anzahl (r, HK_E_T) == 1, "Fall 18 T: TIME_WAIT schon im ersten Durchlauf", "%d mal", anzahl (r, HK_E_T));
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 100, 7840, 0, 0, 0 }, { 200, 7840, 0, 0, 0 } });
    chk (anzahl (r, HK_E_T) == 0, "Fall 18 T nicht bei 0 -> 0", "%d mal", anzahl (r, HK_E_T));
    /* F */
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 10, 7840, 0, 0, 0 }, { 20, 7840, 1, 0, 1 }, { 30, 7840, 1, 0, 1 }, { 40, 7840, 0, 1, 1 } });
    chk (erstes (r, HK_E_F) == 3 && anzahl (r, HK_E_F) == 1, "Fall 19 F einen Durchlauf nach dem annehmenden", "F bei %d (soll 3)", erstes (r, HK_E_F));
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 10, 7840, 0, 0, 1 }, { 20, 7840, 0, 0, 2 }, { 30, 7840, 0, 0, 3 }, { 40, 7840, 0, 0, 3 } });
    chk (erstes (r, HK_E_F) == 4, "Fall 19 F erst nach einem Durchlauf ohne neue Annahme", "F bei %d (soll 4)", erstes (r, HK_E_F));
    r = folge (z, { { 0, 7840, 0, 0, 0 }, { 179990, 7840, 0, 0, 1 }, { 180000, 7840, 0, 0, 1 } });
    chk (anzahl (r, HK_E_F) == 0, "Fall 19 F nicht nach 180 s", "%d mal", anzahl (r, HK_E_F));
    /* W und Summenprobe an einer Folge (hk_nach + hk_anfang) */
    memset (&z, 0, sizeof z);
    hk_erfasst (&z, hk_anfang (&z, 1000, 8000, 0, 0, 0), 0);
    int64_t summe_soll = 0; uint16_t f = 8000;
    for (uint32_t ms = 7; ms < 61000; ms += 7)
    {
        f = (uint16_t) (f - 8); hk_anfang (&z, 1000 + ms, f, 0, 0, 0);        // sys -8
        if (! (z.merker & HK_M_W)) break;
        for (int t = 0; t < 10; t++) { z.letzter = f; f = (uint16_t) (f + (t == 3 ? 8 : 0)); hk_nach (&z, (uint8_t) t, f); }
    }
    int64_t s = 0; for (int t = 0; t < HK_TOEPFE; t++) s += z.topf[t].netto;
    summe_soll = (int64_t) z.free1 - z.free0;
    chk (s == summe_soll && z.topf[HK_TOPF_SYS].netto == -8 * (int32_t) z.n && z.topf[HK_TOPF_UPD].netto == 8 * ((int32_t) z.n - 1) && z.n == 8572,
         "Fall 15 Summenprobe an einer Folge", "Summe %lld, free1-free0 %lld, sys %d, upd %d, n %u", (long long) s, (long long) summe_soll, z.topf[HK_TOPF_SYS].netto, z.topf[HK_TOPF_UPD].netto, z.n);
    HK_ZUSTAND alt = z; hk_anfang (&z, 1000 + 70000, 100, 0, 0, 0);
    chk (memcmp (z.topf, alt.topf, sizeof z.topf) == 0 && z.n == alt.n && z.free1 == alt.free1, "Fall 16 nach W keine Messung mehr", "Toepfe/n/free1 nach W veraendert");
    /* Takt (Fall 20, rein) */
    memset (&z, 0, sizeof z); char b[120];
    hk_erfasst (&z, hk_anfang (&z, 1000, 8000, 0, 0, 0), 0);
    chk (hk_heapzeile (&z, 1000) == 0, "Fall 20 keine m-Karte in W", "hk_heapzeile () verlangt in W eine Karte");
    z.offen = HK_O_ALLE;
    chk (hk_ausgabe (&z, 50000, b, sizeof b) == 0, "Fall 20 keine Zeile in W", "Zeile in W: %s", b);
    hk_anfang (&z, 61000, 8000, 0, 0, 0);
    chk (hk_heapzeile (&z, 61000) == 1, "Fall 20 m-Karte nach W", "keine");
    chk (hk_ausgabe (&z, 61999, b, sizeof b) == 0 && hk_ausgabe (&z, 62000, b, sizeof b) > 0 && hk_ausgabe (&z, 62999, b, sizeof b) == 0 && hk_ausgabe (&z, 63000, b, sizeof b) > 0,
         "Fall 20 eine Zeile je Sekunde, ab der Heap-Zeile gezaehlt", "Takt falsch");
}

/* ---------------------------------------------------------------- Szenarien: echte loop () ueber 200 s */
struct Szenario { const char * name; long anfrage_ms; long hinter_ms; int32_t hinter; long tw_ab_ms; bool x_soll, f_soll, t_soll; };
static tcp_pcb * obj_tcp (uint16_t b) { return (tcp_pcb *) (sim_mem + 8 * b); }
static void objekte_anlegen (void)
{
    static tcp_pcb_listen * lst; static udp_pcb * u1, * u2, * u3; static netif ni, ap;
    lst = (tcp_pcb_listen *) (sim_mem + 8 * 2); lst->next = 0; lst->local_port = 80; tcp_listen_pcbs.listen_pcbs = lst;
    sim_pcb_req = obj_tcp (6);
    u1 = (udp_pcb *) (sim_mem + 8 * 14); u2 = (udp_pcb *) (sim_mem + 8 * 22); u3 = (udp_pcb *) (sim_mem + 8 * 30);
    u1->next = u2; u1->recv_arg = sim_mem + 8 * 18; u2->next = u3; u2->recv_arg = sim_mem + 8 * 26; u3->next = 0; u3->recv_arg = 0; udp_pcbs = u1;
    ap.next = &ni; ap.dhcp = 0; ni.next = 0; ni.dhcp = sim_mem + 8 * 34; netif_list = &ap;
    openweather_client.sim_setze (sim_mem + 8 * 38);
    EEPROM._data = sim_mem + 8 * 42 + 4;
    sim_pcb_req->callback_arg = sim_mem + 8 * 46;
}
static bool ist_instr (const std::string & s) { return ! s.compare (0, 13, "- heap karte ") || ! s.compare (0, 12, "- heap topf ") || ! s.compare (0, 14, "- heap marken "); }
static bool ist_heap (const std::string & s) { return ! s.compare (0, 12, "- heap free="); }
static long feld (const std::string & s, const char * name) { size_t p = s.find (name); return p == std::string::npos ? -999999 : atol (s.c_str () + p + strlen (name)); }

static void szenario (const Szenario & sz)
{
    const char * N = sz.name;
    objekte_anlegen ();
    fake_free = 7840; sim_ms = 8000; sim_t0 = 8000;
    const uint32_t schritt = 7;
    uint32_t n_w = 0, free1 = 0, x_ms = 0, f_ms = 0, t_ms = 0; bool x_war = false, f_war = false, t_war = false, req_lauf_gesehen = false;
    uint32_t req_lauf = 0; int32_t sys_naechster = 0; uint32_t lauf_vorher_in_w = 0;
    uint32_t rnd = 12345;
    for (sim_lauf = 1; sim_ms < 8000 + 200000; sim_lauf++)
    {
        uint32_t ms = sim_ms - sim_t0;
        bool erster = sim_lauf == 1;
        /* sys des vorigen Uebergangs (vom Ende des vorigen bis zum Anfang dieses Durchlaufs) */
        if (! erster)
        {
            fake_free = (uint32_t) ((int32_t) fake_free + sys_naechster);
            if (lauf_vorher_in_w) sim_buchen (HK_TOPF_SYS, sys_naechster, ms);
        }
        sim_in_w = erster || ms < 60000;
        if (sim_in_w) n_w++;
        if (! sim_in_w && ! free1) free1 = fake_free;
        /* Ereignisse, wie sie das Instrument sehen muss (Pruefstandssicht) */
        if (! erster && ms < 180000)
        {
            if (! f_war && req_lauf_gesehen && sim_lauf >= req_lauf + 2) { f_war = true; f_ms = ms; }
            if (! x_war && sim_in_w && ! tcp_active_pcbs && fake_free + 400 < 7840) { x_war = true; x_ms = ms; }
        }
        /* Skript dieses Durchlaufs: kleines Rauschen in vier Toepfen, das sich aufhebt; Rest je Szenario */
        rnd = rnd * 1103515245u + 12345u;
        int32_t r = (int32_t) ((rnd >> 16) % 5) * 8 - 16;
        sim_skript[HK_TOPF_STM] = r; sim_skript[HK_TOPF_UDP] = -r; sim_skript[HK_TOPF_VAR] = (sim_lauf % 2) ? -8 : 8; sim_skript[HK_TOPF_OVR] = (sim_lauf % 2) ? 8 : -8;
        sim_skript[HK_TOPF_WIF] = (sim_lauf % 1000 == 0) ? -16 : (sim_lauf % 1000 == 1) ? 16 : 0;
        sim_skript[HK_TOPF_HLG] = -24;                                          // nur wirksam in Durchlaeufen mit Heap-Zeile
        sys_naechster = (sim_lauf % 3 == 0) ? -8 : (sim_lauf % 3 == 1) ? 8 : 0;
        if (sz.hinter_ms >= 0 && ms <= (uint32_t) sz.hinter_ms && ms + schritt > (uint32_t) sz.hinter_ms) sys_naechster += sz.hinter;
        if (sz.anfrage_ms >= 0 && ms <= (uint32_t) sz.anfrage_ms && ms + schritt > (uint32_t) sz.anfrage_ms) { sim_anfrage = 1; req_lauf = sim_lauf; req_lauf_gesehen = true; }
        if (sz.tw_ab_ms >= 0 && ms <= (uint32_t) sz.tw_ab_ms && ms + schritt > (uint32_t) sz.tw_ab_ms) { tcp_tw_pcbs = 0; if (ms < 180000 && ! t_war) { t_war = true; t_ms = ms; } }
        size_t s0 = sim_serial.size ();
        sim_loop ();
        /* je Durchlauf: hoechstens eine Zeile des Instruments, keine im Durchlauf der Heap-Zeile, keine in W */
        int ni = 0, nh = 0;
        for (size_t i = s0; i < sim_serial.size (); i++) { ni += ist_instr (sim_serial[i].text); nh += ist_heap (sim_serial[i].text); }
        if (ni) chk (ni == 1 && ! nh && ! sim_in_w, N, "Fall 20 Durchlauf %u (ms %u): %d Zeilen des Instruments, %d Heap-Zeilen, in W %d", sim_lauf, ms, ni, nh, sim_in_w);
        if (ni > 1 || (ni && sim_in_w)) break;
        lauf_vorher_in_w = sim_in_w;
        sim_ms += schritt;
    }
    if (getenv ("C80I_ZEIGEN"))
        for (auto & z : sim_serial) if (! z.text.compare (0, 7, "- heap ")) printf ("    [%s +%6u ms] %3zu %s\n", N, z.ms - sim_t0, z.text.size (), z.text.c_str ());
    /* beide Wege, gleicher Text, gleiche Reihenfolge */
    std::vector<std::string> a, b;
    for (auto & z : sim_serial) if (! z.text.compare (0, 7, "- heap ")) a.push_back (z.text);
    for (auto & z : sim_ring) if (! z.text.compare (0, 7, "- heap ")) b.push_back (z.text);
    chk (a == b && ! a.empty (), N, "Fall 8 Mitschnitt und Ring nicht gleich (%zu gegen %zu Zeilen)", a.size (), b.size ());
    /* Takt: >= 1 s zwischen zwei Zeilen des Instruments und nach jeder Heap-Zeile */
    uint32_t letzte = 0; bool hat = false;
    for (auto & z : sim_serial)
    {
        if (ist_instr (z.text))
        {
            chk (z.ms - sim_t0 >= 60000, N, "Fall 20 Zeile vor dem Ende von W (ms %u): %s", z.ms - sim_t0, z.text.c_str ());
            chk (! hat || z.ms - letzte >= 1000, N, "Fall 20 Zeile %u ms nach der vorigen: %s", z.ms - letzte, z.text.c_str ());
            chk (z.text.size () <= 119, N, "Fall 7 Zeile ueber 119 (%zu): %s", z.text.size (), z.text.c_str ());
        }
        if (ist_instr (z.text) || ist_heap (z.text)) { letzte = z.ms; hat = true; }
    }
    /* Zaehlsoll (AKI.10 N2) */
    int nk = 0, nt = 0, nS = 0, nF = 0, nX = 0, nT = 0, nM = 0, nmk = 0, nheap_nach_w = 0;
    std::string kopf, topf[5], S, F, X, T, M;
    for (auto & z : sim_serial)
    {
        const std::string & s = z.text;
        if (ist_heap (s) && z.ms - sim_t0 >= 60000) nheap_nach_w++;
        if (! s.compare (0, 17, "- heap topf kopf ")) { nk++; kopf = s; }
        else if (! s.compare (0, 12, "- heap topf ")) { nt++; int i = s[12] - '0'; if (i >= 1 && i <= 4) topf[i] = s; }
        else if (! s.compare (0, 15, "- heap karte S ")) { nS++; S = s; }
        else if (! s.compare (0, 15, "- heap karte F ")) { nF++; F = s; }
        else if (! s.compare (0, 15, "- heap karte X ")) { nX++; X = s; }
        else if (! s.compare (0, 15, "- heap karte T ")) { nT++; T = s; }
        else if (! s.compare (0, 15, "- heap karte m ")) nmk++;
        else if (! s.compare (0, 14, "- heap marken ")) { nM++; M = s; }
        chk (s.compare (0, 13, "- heap start ") && s.compare (0, 15, "- heap karte U "), N, "Fall 22 ueberholte Zeile: %s", s.c_str ());
    }
    chk (nk == 1 && nt == 4 && nS == 1 && nM == 1, N, "Zaehlsoll kopf %d topf %d S %d marken %d (soll 1/4/1/1)", nk, nt, nS, nM);
    chk (nF == (int) sz.f_soll && nX == (int) sz.x_soll && nT == (int) sz.t_soll, N, "Zaehlsoll F %d X %d T %d (soll %d/%d/%d)", nF, nX, nT, sz.f_soll, sz.x_soll, sz.t_soll);
    chk (nmk == nheap_nach_w && nmk >= 3, N, "Fall 20 je Heap-Zeile nach W eine m-Karte: %d gegen %d", nmk, nheap_nach_w);
    /* Kopf und Toepfe exakt, Summenprobe aus dem Text (wie das Werkzeug des Leads) */
    char soll[200];
    snprintf (soll, sizeof soll, "- heap topf kopf t0=8000 w=60000 n=%u free0=7840 free1=%u us=37", n_w, free1);
    chk (kopf == soll, N, "Kopf \"%s\" (soll \"%s\")", kopf.c_str (), soll);
    static const char * namen[HK_TOEPFE] = { "wif", "srv", "ota", "upd", "udp", "ntp", "var", "ovr", "hlg", "stm", "sys" };
    long summe = 0; int gelesen = 0;
    for (int i = 1; i <= 4; i++)
    {
        std::string e = "- heap topf " + std::to_string (i) + "/4";
        for (int t = 3 * (i - 1); t < 3 * i && t < HK_TOEPFE; t++)
        {
            char x[64];
            if (wahr_rueck[t]) snprintf (x, sizeof x, " %s=%lld/%u@%u", namen[t], (long long) wahr_netto[t], wahr_rueck[t], wahr_ms[t]);
            else snprintf (x, sizeof x, " %s=%lld/0@-", namen[t], (long long) wahr_netto[t]);
            e += x;
            char nm[8]; snprintf (nm, sizeof nm, " %s=", namen[t]);
            size_t p = topf[i].find (nm);
            if (p != std::string::npos) { summe += atol (topf[i].c_str () + p + strlen (nm)); gelesen++; }
        }
        chk (topf[i] == e, N, "Fall 15/16 Toepfe: \"%s\" (soll \"%s\")", topf[i].c_str (), e.c_str ());
    }
    chk (gelesen == 11 && summe == feld (kopf, "free1=") - feld (kopf, "free0="), N, "AKI.13 Summenprobe aus der Ausgabe: Summe %ld, free1-free0 %ld", summe, feld (kopf, "free1=") - feld (kopf, "free0="));
    /* Karten: S exakt gegen den Bezug; ms von F, X, T; Wegmarken exakt */
    std::vector<Ein> E0 = { { 1, 128, false }, { 129, 37, true }, { 166, 63, false }, { 229, 943, true }, { 1172, 417, false } };
    chk (S == karte_soll ('S', bezug (E0), 0), N, "Karte S \"%s\" (soll \"%s\")", S.c_str (), karte_soll ('S', bezug (E0), 0).c_str ());
    if (sz.x_soll) chk (feld (X, " ms=") == (long) x_ms, N, "Karte X ms=%ld (soll %u)", feld (X, " ms="), x_ms);
    if (sz.f_soll) chk (feld (F, " ms=") == (long) f_ms, N, "Karte F ms=%ld (soll %u)", feld (F, " ms="), f_ms);
    if (sz.t_soll) chk (feld (T, " ms=") == (long) t_ms, N, "Karte T ms=%ld (soll %u)", feld (T, " ms="), t_ms);
    const char * msoll = sz.x_soll ? (sz.anfrage_ms >= 0 && sz.anfrage_ms < sz.hinter_ms ? "- heap marken X tcp=6:10/- lst=2 wet=38 dhcp=34 eep=42 udp=14/18,22/26,30/-" : "- heap marken X tcp=- lst=2 wet=38 dhcp=34 eep=42 udp=14/18,22/26,30/-")
                                   : "- heap marken W tcp=- lst=2 wet=38 dhcp=34 eep=42 udp=14/18,22/26,30/-";
    chk (M == msoll, N, "Fall 13/22 Wegmarken \"%s\" (soll \"%s\")", M.c_str (), msoll);
    /* AKI.3: jede m-Karte gegen die Heap-Zeile davor - gross = max, fsum = free, exakt */
    std::string heap_vor; int paare = 0;
    for (auto & z : sim_serial)
    {
        if (ist_heap (z.text)) heap_vor = z.text;
        if (! z.text.compare (0, 15, "- heap karte m ") && ! heap_vor.empty ())
        {
            long g = feld (z.text, "gross="); size_t p = z.text.find ("gross="); long glen = p == std::string::npos ? -1 : atol (strchr (z.text.c_str () + p, '+') + 1);
            (void) g; paare++;
            chk (glen == feld (heap_vor, "max=") && feld (z.text, "fsum=") == feld (heap_vor, " free="), N, "AKI.3 m-Karte \"%s\" gegen \"%s\"", z.text.c_str (), heap_vor.c_str ());
        }
    }
    chk (paare >= 3, N, "AKI.3: nur %d Paare Heap-Zeile/Karte", paare);
    chk (! EEPROM.dirty, N, "Wegmarke eep hat EEPROM veraendert (getDataPtr () statt getConstDataPtr ())");
    chk (sim_sperre == 0 && sim_sperre_max == 1, N, "Sperre nach dem Lauf %d, hoechstens %d (soll 0/1)", sim_sperre, sim_sperre_max);
    chk (heapkarte.us == 37, N, "us=%u (soll 37)", heapkarte.us);
}

int main (void)
{
    setvbuf (stdout, 0, _IOLBF, 0);
    /* Jede Gruppe in einem eigenen Prozess: Ein Absturz (etwa ein Gang ohne Schranke, der ueber das Feld hinaus liest)
     * ist ein FEHL dieser Gruppe, kein kaputter Pruefstand. */
    struct { const char * name; void (* fn) (void); } G[] = {
        { "Blockgang (Faelle 1-5, 9, 10)", fall_gang }, { "beschaedigte Haufen (Faelle 6, 11)", fall_kaputt },
        { "Zeilenformen (Faelle 7, 13, 21)", fall_zeilen }, { "Toepfe (Fall 15)", fall_toepfe },
        { "Ereignisse und Takt (Faelle 15-20, 22)", fall_ereignisse } };
    for (auto & g : G)
    {
        fflush (stdout);
        pid_t p = fork ();
        if (! p) { g_fails = 0; g_checks = 0; alarm (20); g.fn (); printf ("  %s: %d Pruefungen\n", g.name, g_checks); fflush (stdout); _exit (g_fails ? 1 : 0); }
        int st; waitpid (p, &st, 0); g_checks++;
        if (! WIFEXITED (st)) { printf ("FEHLER %s: abgebrochen (Signal %d)\n", g.name, WIFSIGNALED (st) ? WTERMSIG (st) : 0); g_fails++; }
        else if (WEXITSTATUS (st)) g_fails++;
    }
    static const Szenario SZ[] = {
        /* Arm E nach Ent-C5 = A: eine Anfrage nach 2 s, Hintergrundbelegung 504 bei 5 s, TIME_WAIT laeuft nach 120 s ab */
        { "Szenario E", 2000, 5000, -504, 122000, true, true, true },
        /* Arm B: keine Anfrage in W, Hintergrundbelegung bei 5 s, erste Anfrage bei 150 s (F nach W, T ausserhalb 180 s) */
        { "Szenario B", 150000, 5000, -504, 270000, true, true, false },
        /* kein X: Hintergrundbelegung klein - Wegmarken am Ende von W (Anlass W) */
        { "Szenario ohne X", -1, 5000, -200, -1, false, false, false },
    };
    for (auto & sz : SZ)
    {
        fflush (stdout);
        pid_t p = fork ();
        if (! p) { g_fails = 0; g_checks = 0; alarm (60); szenario (sz); printf ("  %s: %d Pruefungen\n", sz.name, g_checks); fflush (stdout); _exit (g_fails ? 1 : 0); }
        int st; waitpid (p, &st, 0);
        if (! WIFEXITED (st)) { printf ("FEHLER %s: Kind abgebrochen (Signal %d)\n", sz.name, WIFSIGNALED (st) ? WTERMSIG (st) : 0); g_fails++; }
        else if (WEXITSTATUS (st)) g_fails++;
        g_checks++;
    }
    printf ("c80i (%s): %d Gruppen und Szenarien, %d davon mit FEHLER\n", BEZEICHNUNG, g_checks, g_fails);
    return g_fails ? 1 : 0;
}
