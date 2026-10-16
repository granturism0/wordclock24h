/* Pruefstand c72i (Paket 2026-10-15, R1.1; AKI.1, AKI.2; design.md 3.1, 3.2, 8).
 *
 * Echter Code aus http.cpp: Klasse WcHttpServer (unclaimed (), listen_arg (), pending ()), der I-5-Block
 * (tcp_now_arg (), tcp_now_add (), tcp_now_read (), http_accept_dead_note (), tcp_now_tcp_field (),
 * http_no_request_timeouts_get ()), http_api_device_ready () und http_server_loop () ganz, dazu I-1, I-3, I-4 wie in
 * c65i (Auszug von c65). Aus ESP-uclock.ino: esp_heap_log ().
 * Attrappen: lwip_c72.h (lwIP-Listen mit allen Feldern, die I-5 liest; WiFiServer, WiFiClient, ClientContext mit der
 * Semantik des Core 3.1.2), basis.h aus c65 (String, Serial, Ring, ESP-Werte).
 *
 * "Ohne Abgabe" wird DYNAMISCH geprueft: Ist stoeren gesetzt, veraendert der naechste Abgabepunkt (sdk_abgabe (), also
 * yield (), available () bei 0, accept () ohne Eintrag) oder die naechste Ausgabe (http_send ()) alle Listen - so, wie
 * der Systemkontext es auf dem Geraet in einer Abgabe tun kann. Liest I-5 nicht vollstaendig VOR der ersten Ausgabe,
 * oder gibt es im Gang ab, zeigt device_ready den veraenderten Stand.
 * Uebersetzt zweimal: nativ und mit C72_ZIEL (uint_fast8_t/uint_fast16_t 32 Bit wie auf dem Xtensa, pruefstand.h).
 */
#ifdef C72_ZIEL
#define PRUEFSTAND_SCHNELLE_TYPEN
#endif
#include "basis.h"
#ifdef C72_ZIEL
#include "pruefstand.h"
#endif
static void sdk_abgabe (void);
#include "lwip_c72.h"
#include "have.inc"
#include "c72have.inc"
#include "defs.inc"
#ifndef HEAP_LOW_PATH_FLUSH
#define HEAP_LOW_PATH_FLUSH 1
#define HEAP_LOW_PATH_FS 2
#define HEAP_LOW_PATH_ACCEPT 3
#define HEAP_LOW_PATH_CLIENT 4
#endif

/* ---- Stoerung an Abgabe- und Ausgabepunkten */
static bool stoeren = false;
static int  stoerungen = 0;
static void lwip_stoeren (void);
static void sdk_abgabe (void) { if (stoeren) { stoeren = false; lwip_stoeren (); } }

#include "i1.inc"

/* ---- JSON- und Antwort-Attrappen: alles geht durch http_send () */
static std::string out;
static std::map<std::string, std::string> params;
static void http_send (const char * s) { if (stoeren) { stoeren = false; lwip_stoeren (); } out += s; }
static void http_flush (void) {}
static char * http_get_param (const char * n) { static char b[8][300]; static int i; char * p = b[i++ % 8]; snprintf (p, 300, "%s", params.count (n) ? params[n].c_str () : ""); return p; }
static void http_json_send_field_prefix (const char * k) { http_send ((std::string (",\"") + k + "\":").c_str ()); }
static void http_json_send_uint_field (const char * k, unsigned long v) { http_send ((std::string (",\"") + k + "\":" + std::to_string (v)).c_str ()); }
static void http_json_send_string_field (const char * k, const char * v) { http_send ((std::string (",\"") + k + "\":\"" + v + "\"").c_str ()); }
static uint16_t update_cache_hits = 7, update_server_down_count = 3;
uint16_t httpclient_connects_get (void) { return 0; }

#define TCP_MSS 536
#include "zaehler.inc"
#if __has_include ("c72r2b.inc")
#include "c72r2b.inc"                                                   /* R2b: C72_R2B_ZAEHLER, wenn zaehler.inc die R2b-Zaehler fuehrt */
#endif
#ifndef HAVE_FS_WRITE_SHORT
static uint16_t http_fs_write_short = 0;
#endif

#include "server_class.inc"
WcHttpServer        http_server (80);
static WiFiClient   http_client;
#include "i4.inc"
#ifndef HAVE_I4
static void heap_low_snapshot (void) {}
static uint16_t api_orphan_skipped = 0;
static uint8_t http_backlog = 3;
static void http_backlog_track (void) {}
void tcp_tw_limit (void) {}
#endif
#include "i5.inc"
#ifndef C72_HAVE_I5
static uint16_t http_accept_dead = 0;
size_t tcp_now_tcp_field (char * b, size_t n) { if (n) b[0] = '\0'; return 0; }
uint16_t http_no_request_timeouts_get (void) { return 0; }
#endif

/* ---- was http_server_loop () ausserdem braucht (wie c65i) */
static void http_clear_request_user_agent (void) {}
static void http_clear_request_fetch_dest (void) {}
static bool http_read_request_line (String & line, unsigned long) { line = String (""); return false; }
static bool read_line (String &) { return false; }
static void http_capture_request_user_agent (const String &) {}
static void http_capture_request_fetch_dest (const String &) {}
static void http_clear_request_if_none_match (void) {}             /* R2 C70: Gegenstand von c70e */
static void http_capture_request_if_none_match (const String &) {}
static void http_log_request (const String &, const char *, const String &) {}
static const char * http_get_request_browser_label (void) { return ""; }
static uint_fast8_t http_api_query_too_long (const char *, size_t) { return 0; }
static void http_set_params (char *) {}
static void http_post (const String &) {}
static uint_fast8_t http (const char *, const char *) { return 0; }
#include "i3.inc"
#ifndef HAVE_I3
static uint32_t req_max_ms = 0; static char req_max_path[32] = "";
#endif
#include "device_ready.inc"
#include "server_loop.inc"
#define ESP_HEAP_LOG_INTERVAL 60000UL
#include "heaplog.inc"

/* ---- Listen bauen */
static std::deque<struct tcp_pcb>   pcbs;
static std::deque<ClientContext>    ctxs;
static struct tcp_pcb               listen_pcb;
static void * const                 LISTEN_ARG = (void *) &http_server;     /* WiFiServer::begin (): tcp_arg (listen_pcb, this) */

static void lwip_stoeren (void)
{
    stoerungen++;
    for (auto & p : pcbs) { p.next = nullptr; p.state = CLOSED; p.callback_arg = nullptr; p.remote_port = 1; p.tmr = tcp_ticks; p.nrtx = 99; }
    tcp_active_pcbs = nullptr; tcp_tw_pcbs = nullptr;
    listen_pcb.accepts_pending = 77;
    http_server.t_unclaimed (nullptr);
}
static struct tcp_pcb * pcb (enum tcp_state st, void * arg, int o4, u16_t port, u32_t ruhe_ticks = 0, u8_t nrtx = 0)
{
    pcbs.emplace_back (); struct tcp_pcb * p = &pcbs.back (); memset (p, 0, sizeof (*p));
    const u8_t ip[4] = { 192, 168, 1, (u8_t) o4 }; memcpy (&p->remote_ip.addr, ip, 4);
    p->state = st; p->callback_arg = arg; p->remote_port = port; p->tmr = tcp_ticks - ruhe_ticks; p->nrtx = nrtx;
    return p;
}
static ClientContext * ctx (struct tcp_pcb * p, size_t rx = 0) { ctxs.emplace_back (); ClientContext * c = &ctxs.back (); c->_pcb = p; c->rx = rx; return c; }
static struct tcp_pcb * mit_ctx (struct tcp_pcb * p) { p->callback_arg = ctx (p); return p; }
static void kette (struct tcp_pcb ** kopf, const std::vector<struct tcp_pcb *> & v) { *kopf = nullptr; for (auto it = v.rbegin (); it != v.rend (); ++it) { (*it)->next = *kopf; *kopf = *it; } }
static void aktiv (const std::vector<struct tcp_pcb *> & v) { kette (&tcp_active_pcbs, v); }
static void tw (const std::vector<struct tcp_pcb *> & v) { kette (&tcp_tw_pcbs, v); }
static void unclaimed (const std::vector<ClientContext *> & v)
{
    ClientContext * k = nullptr;
    for (auto it = v.rbegin (); it != v.rend (); ++it) { (*it)->next (k); k = *it; }
    http_server.t_unclaimed (k);
}
static void listener (u8_t pending) { memset (&listen_pcb, 0, sizeof (listen_pcb)); listen_pcb.state = LISTEN; listen_pcb.callback_arg = LISTEN_ARG; listen_pcb.accepts_pending = pending; http_server.t_listen (&listen_pcb); }
static void leeren (void)
{
    tcp_active_pcbs = nullptr; tcp_tw_pcbs = nullptr; http_server.t_unclaimed (nullptr); http_client = WiFiClient ();
    pcbs.clear (); ctxs.clear (); tcp_ticks = 100000; listener (0); stoeren = false;
}

/* ---- device_ready lesen */
static std::vector<std::string> keys;
static std::map<std::string, std::string> dr (const char * reset = nullptr)
{
    std::map<std::string, std::string> m; keys.clear ();
    params.clear (); if (reset) params["heap_low_reset"] = reset;
    out.clear (); http_api_device_ready ();
    size_t i = out.find ('{'); if (i == std::string::npos) return m; i++;
    while (i < out.size ())
    {
        size_t k = out.find ('"', i); if (k == std::string::npos) break;
        size_t e = out.find ('"', k + 1); std::string key = out.substr (k + 1, e - k - 1);
        size_t v = e + 2; std::string val;
        if (out[v] == '"') { size_t ve = out.find ('"', v + 1); val = out.substr (v + 1, ve - v - 1); i = ve + 1; }
        else if (out[v] == '[') { int d = 0; size_t ve = v; for (; ve < out.size (); ve++) { if (out[ve] == '[') d++; else if (out[ve] == ']' && --d == 0) break; } val = out.substr (v, ve - v + 1); i = ve + 1; }
        else { size_t ve = out.find_first_of (",}", v); val = out.substr (v, ve - v); i = ve; }
        m[key] = val; keys.push_back (key);
    }
    return m;
}
/* "[[a,b,...],[...]]" -> Eintraege; leer bei Formfehler */
static std::vector<std::vector<long>> liste (const std::string & s, bool * form_ok = nullptr)
{
    std::vector<std::vector<long>> r; bool ok = s.size () >= 2 && s.front () == '[' && s.back () == ']';
    size_t i = 1;
    while (ok && i < s.size () - 1)
    {
        if (s[i] == ',') { i++; continue; }
        if (s[i] != '[') { ok = false; break; }
        size_t e = s.find (']', i); if (e == std::string::npos) { ok = false; break; }
        std::vector<long> z; std::string t = s.substr (i + 1, e - i - 1); size_t p = 0;
        while (p <= t.size ()) { size_t q = t.find (',', p); if (q == std::string::npos) q = t.size (); z.push_back (atol (t.substr (p, q - p).c_str ())); p = q + 1; }
        r.push_back (z); i = e + 1;
    }
    if (form_ok) *form_ok = ok;
    return ok ? r : std::vector<std::vector<long>> ();
}
static std::string i5 (std::map<std::string, std::string> & m)
{
    return m["tcp_now_states"] + "|tw " + m["tcp_now_tw"] + "|p " + m["tcp_now_pending"] + "|u " + m["tcp_now_unclaimed"] + "|self " +
           m["tcp_now_self_port"] + "|" + m["tcp_now_list"] + "|more " + m["tcp_now_more"];
}
static std::string heapzeile (void)
{
    fake_ms += 60000; size_t n = ring.size (), s = Serial.lines.size (); esp_heap_log ();
    if (ring.size () != n + 1 || Serial.lines.size () != s + 1) return "(nicht genau je eine Zeile: Ring " + std::to_string (ring.size () - n) + ", Serial " + std::to_string (Serial.lines.size () - s) + ")";
    if (ring.back () != Serial.lines.back ()) return "(Mitschnitt '" + Serial.lines.back () + "' != Ring '" + ring.back () + "')";
    return ring.back ();
}
static void A3_aufbauen (void)
{
    leeren (); listener (3);
    aktiv ({ mit_ctx (pcb (ESTABLISHED, nullptr, 23, 50001, 20)),
             pcb (SYN_RCVD, LISTEN_ARG, 24, 50002, 2),
             pcb (ESTABLISHED, nullptr, 25, 50003, 60, 3),          /* vom ESP geschlossen, FIN nicht eingereiht (TF_CLOSEPEND) */
             pcb (FIN_WAIT_2, nullptr, 26, 50004, 41),
             mit_ctx (pcb (CLOSE_WAIT, nullptr, 27, 50005, 0)),
             pcb (LAST_ACK, nullptr, 28, 50006, 10, 12) });
    tw ({ pcb (TIME_WAIT, nullptr, 29, 50007, 240) });
    struct tcp_pcb * eigen = pcbs.size () ? &pcbs[0] : nullptr;
    http_client = WiFiClient ((ClientContext *) eigen->callback_arg);     /* die bediente Verbindung: Port 50001 */
    unclaimed ({ ctx (pcb (ESTABLISHED, nullptr, 30, 50008)), ctx (nullptr), ctx (pcb (CLOSE_WAIT, nullptr, 31, 50009)) });
}
static const char * A3_LISTE = "[[4,23,50001,10,0,1],[3,24,50002,1,0,2],[4,25,50003,30,3,0],[6,26,50004,20,0,0],[7,27,50005,0,0,1],[9,28,50006,5,12,0],[10,29,50007,120,0,0]]";

int main (void)
{
    printf ("c72i (%s%s)\n", BEZEICHNUNG,
#ifdef C72_ZIEL
            ", Zieltypen"
#else
            ", nativ"
#endif
           );
    std::map<std::string, std::string> m;
#ifndef C72_HAVE_I5
    fall ("I-5 vorhanden (Block TCP_NOW_LIST_MAX ... http_no_request_timeouts_get)", false, "Auszug ohne I-5 - Stand vor R1.1");
#endif
    /* --- A: Felder in device_ready */
    {
        leeren ();
        std::vector<struct tcp_pcb *> v;
        for (int s = 0; s <= 10; s++) v.push_back (pcb ((enum tcp_state) s, nullptr, 100 + s, (u16_t) (40000 + s)));
        aktiv (v); tw ({ pcb (TIME_WAIT, nullptr, 120, 41000), pcb (TIME_WAIT, nullptr, 121, 41001) });
        m = dr ();
        fall ("A1 tcp_now_states: 11 Zahlen, Index = enum tcp_state, ueber tcp_active_pcbs", m["tcp_now_states"] == "[1,1,1,1,1,1,1,1,1,1,1]", m["tcp_now_states"]);
        fall ("A1 tcp_now_tw = Laenge von tcp_tw_pcbs", m["tcp_now_tw"] == "2", m["tcp_now_tw"]);
        bool form; auto l = liste (m["tcp_now_list"], &form);
        bool reihe = l.size () == 8; for (size_t k = 0; reihe && k < l.size (); k++) reihe = l[k].size () == 6 && l[k][0] == (long) k && l[k][2] == 40000 + (long) k;
        fall ("A2 Liste gekappt auf 8, erst aktive in Listenreihenfolge, je 6 Zahlen", form && reihe, m["tcp_now_list"]);
        fall ("A2 tcp_now_more = 11 + 2 - 8", m["tcp_now_more"] == "5", m["tcp_now_more"]);
        leeren ();
        aktiv ({ pcb (ESTABLISHED, nullptr, 1, 1) }); tw ({ pcb (TIME_WAIT, nullptr, 2, 2), pcb (TIME_WAIT, nullptr, 3, 3) });
        m = dr ();
        fall ("A2 TIME_WAIT hinter den aktiven", m["tcp_now_list"] == "[[4,1,1,0,0,0],[10,2,2,0,0,0],[10,3,3,0,0,0]]" && m["tcp_now_more"] == "0", m["tcp_now_list"]);
    }
    {
        A3_aufbauen (); m = dr ();
        auto l = liste (m["tcp_now_list"]);
        fall ("A3 Liste vollstaendig: [zustand, gegenstelle, port, ruhe_s, nrtx, arg]", m["tcp_now_list"] == A3_LISTE, m["tcp_now_list"]);
        fall ("A3 SYN_RCVD traegt den Listener: arg 2, nicht ClientContext", l.size () > 1 && l[1][0] == SYN_RCVD && l[1][5] == 2, m["tcp_now_list"]);
        fall ("A3 ESTABLISHED ohne callback_arg: arg 0", l.size () > 2 && l[2][0] == ESTABLISHED && l[2][5] == 0, m["tcp_now_list"]);
        fall ("A3 ClientContext: arg 1 (auch CLOSE_WAIT)", l.size () > 4 && l[0][5] == 1 && l[4][5] == 1, m["tcp_now_list"]);
        fall ("A3 Gegenstelle = letztes Oktett, Port der Gegenstelle", l.size () > 0 && l[0][1] == 23 && l[0][2] == 50001, m["tcp_now_list"]);
        fall ("A3 ruhe_s = (tcp_ticks - tmr) x 500 / 1000, abgerundet", l.size () > 6 && l[0][3] == 10 && l[3][3] == 20 && l[6][3] == 120 && l[4][3] == 0, m["tcp_now_list"]);
        fall ("A3 nrtx", l.size () > 5 && l[2][4] == 3 && l[5][4] == 12, m["tcp_now_list"]);
        fall ("A3 Zustaende jetzt", m["tcp_now_states"] == "[0,0,0,1,2,0,1,1,0,1,0]", m["tcp_now_states"]);
        fall ("A3 tcp_now_pending = accepts_pending", m["tcp_now_pending"] == "3", m["tcp_now_pending"]);
        fall ("A3 tcp_now_unclaimed zaehlt lebende UND tote Kontexte", m["tcp_now_unclaimed"] == "3", m["tcp_now_unclaimed"]);
        fall ("A3 tcp_now_self_port = Port der Gegenstelle dieser Anfrage", m["tcp_now_self_port"] == "50001", m["tcp_now_self_port"]);
        fall ("A3 die Liste aendert die Listen nicht", tcp_active_pcbs == &pcbs[0] && http_server.t_unclaimed_get () != nullptr && pcbs[0].state == ESTABLISHED, "Listen veraendert");
    }
    {
        /* A4 ruhe_s: Umlauf von tcp_ticks, Saettigung, kein 32-Bit-Ueberlauf im Produkt */
        leeren (); tcp_ticks = 5;
        struct tcp_pcb * a = pcb (ESTABLISHED, nullptr, 1, 1); a->tmr = 0xFFFFFFFBu;       /* 10 Ticks ueber den Umlauf */
        tcp_ticks = 300000;
        struct tcp_pcb * b = pcb (ESTABLISHED, nullptr, 2, 2, 130000);                     /* 65000 s, knapp unter der Saettigung */
        struct tcp_pcb * c = pcb (ESTABLISHED, nullptr, 3, 3, 131072);                     /* 65536 s */
        tcp_ticks = 8589935;
        struct tcp_pcb * d = pcb (ESTABLISHED, nullptr, 4, 4, 8589935);                    /* x 500 laeuft in 32 Bit auf 204 */
        tcp_ticks = 5; a->tmr = 0xFFFFFFFBu; b->tmr = tcp_ticks - 130000; c->tmr = tcp_ticks - 131072; d->tmr = tcp_ticks - 8589935;
        aktiv ({ a, b, c, d }); m = dr ();
        auto l = liste (m["tcp_now_list"]);
        fall ("A4 ruhe_s ueber den Umlauf von tcp_ticks", l.size () == 4 && l[0][3] == 5, m["tcp_now_list"]);
        fall ("A4 ruhe_s 65000 ungesaettigt", l.size () == 4 && l[1][3] == 65000, m["tcp_now_list"]);
        fall ("A4 ruhe_s saettigt bei 65535", l.size () == 4 && l[2][3] == 65535, m["tcp_now_list"]);
        fall ("A4 ruhe_s ohne 32-Bit-Ueberlauf im Produkt (8'589'935 Ticks)", l.size () == 4 && l[3][3] == 65535, m["tcp_now_list"]);
    }
    {
        /* A5 Saettigung: 300 + 300 PCBs, 300 Kontexte */
        leeren ();
        std::vector<struct tcp_pcb *> v, w; std::vector<ClientContext *> u;
        for (int k = 0; k < 300; k++) v.push_back (pcb (ESTABLISHED, nullptr, 1, 1));
        for (int k = 0; k < 300; k++) w.push_back (pcb (TIME_WAIT, nullptr, 2, 2));
        for (int k = 0; k < 300; k++) u.push_back (ctx (k % 2 ? nullptr : pcb (ESTABLISHED, nullptr, 3, 3)));
        aktiv (v); tw (w); unclaimed (u); m = dr ();
        fall ("A5 Zustaende saettigen bei 255", m["tcp_now_states"] == "[0,0,0,0,255,0,0,0,0,0,0]", m["tcp_now_states"]);
        fall ("A5 tcp_now_tw saettigt bei 255", m["tcp_now_tw"] == "255", m["tcp_now_tw"]);
        fall ("A5 tcp_now_unclaimed saettigt bei 255", m["tcp_now_unclaimed"] == "255", m["tcp_now_unclaimed"]);
        fall ("A5 Liste 8, tcp_now_more 592", liste (m["tcp_now_list"]).size () == 8 && m["tcp_now_more"] == "592", std::to_string (liste (m["tcp_now_list"]).size ()) + " / " + m["tcp_now_more"]);
    }
    {
        /* A6 leer, ohne Listener, ohne Client */
        leeren (); m = dr ();
        fall ("A6 leere Listen: Nullen, Liste []", i5 (m) == "[0,0,0,0,0,0,0,0,0,0,0]|tw 0|p 0|u 0|self 0|[]|more 0", i5 (m));
        http_server.t_listen (nullptr);
        aktiv ({ pcb (SYN_RCVD, LISTEN_ARG, 9, 9) }); m = dr ();
        fall ("A6 ohne Listen-PCB: pending 0, kein Vergleich mit NULL", m["tcp_now_pending"] == "0" && m["tcp_now_list"] == "[[3,9,9,0,0,1]]", m["tcp_now_pending"] + " " + m["tcp_now_list"]);
        leeren ();
        struct tcp_pcb * p = pcb (ESTABLISHED, nullptr, 1, 1);
        http_client = WiFiClient (ctx (nullptr)); aktiv ({ p }); m = dr ();
        fall ("A6 Client ohne PCB: self_port 0", m["tcp_now_self_port"] == "0", m["tcp_now_self_port"]);
    }
    {
        /* A7 VOR der ersten Ausgabe vollstaendig gelesen, ohne Abgabe im Gang */
        A3_aufbauen (); stoerungen = 0; stoeren = true; m = dr ();
        fall ("A7 Stoerung an der ersten Abgabe/Ausgabe ausgeloest (Pruefung wirksam)", stoerungen == 1, std::to_string (stoerungen));
        fall ("A7 device_ready zeigt den Stand VOR der ersten Ausgabe", m["tcp_now_list"] == A3_LISTE && m["tcp_now_states"] == "[0,0,0,1,2,0,1,1,0,1,0]" &&
              m["tcp_now_tw"] == "1" && m["tcp_now_pending"] == "3" && m["tcp_now_unclaimed"] == "3", i5 (m));
    }
    {
        /* A8 Feldreihenfolge: Bestand unveraendert, I-5 hinter api_orphan_skipped */
        leeren (); dr ();
        std::vector<std::string> soll = { "ok", "ready", "free_heap", "max_free_block", "write_lost_bytes", "write_lost_blocks", "write_lost_gone",
            "write_lost_failed", "no_request_timeouts", "no_request_aborts", "update_cache_hits", "update_server_down_count", "heap_low_block",
            "heap_low_free", "heap_low_path", "heap_low_ms", "httpclient_connects", "fs_write_short", "req_max_ms", "req_max_path", "upd_cache_max_ms",
            "heap_low_accepts_pending", "heap_low_backlog", "heap_low_pcb_states", "heap_low_tw", "heap_low_unacked", "heap_low_req_path",
            "http_backlog", "api_orphan_skipped",
            "tcp_now_states", "tcp_now_tw", "tcp_now_pending", "tcp_now_unclaimed", "tcp_now_self_port", "tcp_now_list", "tcp_now_more", "http_accept_dead" };
        std::string w; for (auto & k : keys) w += k + " ";
        std::vector<std::string> r2 = { "app_not_modified", "app_full" };       /* R2 C70 (AKC.4): hinter den I-5-Feldern */
        std::vector<std::string> r2b = { "app_not_modified", "app_full", "app_immutable", "app_token_miss" };   /* R2b C70 (c) (AKL.2): hinter app_full */
        auto mit = [&] (const std::vector<std::string> & z) { return keys.size () == soll.size () + z.size () && std::equal (soll.begin (), soll.end (), keys.begin ()) &&
                                                                     std::equal (z.begin (), z.end (), keys.begin () + soll.size ()); };
        bool passt = keys == soll || mit (r2) || mit (r2b);
        fall ("A8 Felder: Bestand unveraendert, I-5 dahinter in der Reihenfolge der Spec (ab R2 app_not_modified, app_full; ab R2b dahinter app_immutable, app_token_miss)", passt, w);
#ifdef C72_R2B_ZAEHLER
        /* A9 (R2b): jedes Feld gibt SEINEN Zaehler aus - verschiedene Werte, damit eine Vertauschung auffaellt */
        http_app_not_modified = 31; http_app_full = 32; http_app_immutable = 33; http_app_token_miss = 34;
        leeren (); std::map<std::string, std::string> m9 = dr ();
        fall ("A9 app_not_modified/app_full/app_immutable/app_token_miss geben ihren Zaehler aus",
              m9["app_not_modified"] == "31" && m9["app_full"] == "32" && m9["app_immutable"] == "33" && m9["app_token_miss"] == "34",
              m9["app_not_modified"] + " " + m9["app_full"] + " " + m9["app_immutable"] + " " + m9["app_token_miss"]);
        http_app_not_modified = 0; http_app_full = 0; http_app_immutable = 0; http_app_token_miss = 0;
#endif
        fall ("A8 Antwort endet mit }", ! out.empty () && out.back () == '}', out.substr (out.size () > 20 ? out.size () - 20 : 0));
    }
    /* --- B: http_accept_dead im echten http_server_loop () */
    {
        leeren (); http_accept_dead = 0; uint16_t nrt0 = http_no_request_timeouts;
        for (int k = 0; k < 5; k++) http_server_loop ();
        fall ("B1 leere Durchlaeufe zaehlen nicht", http_accept_dead == 0, std::to_string (http_accept_dead));
        unclaimed ({ ctx (nullptr) }); http_server_loop ();
        fall ("B2 toter Kontext (PCB weg) gezaehlt", http_accept_dead == 1 && http_server.t_unclaimed_get () == nullptr, std::to_string (http_accept_dead));
        unclaimed ({ ctx (pcb (CLOSE_WAIT, nullptr, 1, 1)) }); http_server_loop ();
        fall ("B3 toter Kontext (CLOSE_WAIT ohne Daten) gezaehlt", http_accept_dead == 2, std::to_string (http_accept_dead));
        unclaimed ({ ctx (pcb (ESTABLISHED, nullptr, 1, 1)) }); http_server_loop ();
        fall ("B4 lebende Verbindung nicht gezaehlt, aber bedient", http_accept_dead == 2 && http_no_request_timeouts == nrt0 + 1, std::to_string (http_accept_dead) + " nrt " + std::to_string (http_no_request_timeouts));
        unclaimed ({ ctx (pcb (CLOSE_WAIT, nullptr, 1, 1), 5) }); http_server_loop ();
        fall ("B5 CLOSE_WAIT mit gepufferter Anfrage nicht gezaehlt", http_accept_dead == 2, std::to_string (http_accept_dead));
        unclaimed ({ ctx (nullptr) }); for (int k = 0; k < 3; k++) http_server_loop ();
        fall ("B6 ein toter Kontext, danach leere Durchlaeufe: genau +1", http_accept_dead == 3, std::to_string (http_accept_dead));
        unclaimed ({ ctx (nullptr), ctx (nullptr) }); http_server_loop (); http_server_loop ();
        fall ("B7 zwei tote Kontexte nacheinander: +2", http_accept_dead == 5, std::to_string (http_accept_dead));
        m = dr ("1");
        fall ("B8 http_accept_dead in device_ready", m["http_accept_dead"] == "5", m["http_accept_dead"]);
        m = dr ();
        fall ("B8 heap_low_reset=1 setzt http_accept_dead nicht zurueck", m["http_accept_dead"] == "5", m["http_accept_dead"]);
        http_accept_dead = 0xFFFE; unclaimed ({ ctx (nullptr), ctx (nullptr) }); http_server_loop (); http_server_loop ();
        fall ("B9 saettigt bei 65535", http_accept_dead == 0xFFFF, std::to_string (http_accept_dead));
        http_accept_dead = 0;
    }
    /* --- C: Heap-Zeile */
    {
        leeren ();
        {
            /* je Zustand eine eigene Zahl, damit eine Vertauschung auffaellt: est 2, fw1 1, fw2 3, cw 4, la 5, tw 6;
             * dazu SYN_RCVD und CLOSING, die in tcp= nicht vorkommen */
            std::vector<struct tcp_pcb *> v, w;
            const int zahl[][2] = { { ESTABLISHED, 2 }, { FIN_WAIT_1, 1 }, { FIN_WAIT_2, 3 }, { CLOSE_WAIT, 4 }, { LAST_ACK, 5 }, { SYN_RCVD, 1 }, { CLOSING, 1 } };
            for (auto & z : zahl) for (int k = 0; k < z[1]; k++) v.push_back (pcb ((enum tcp_state) z[0], z[0] == SYN_RCVD ? LISTEN_ARG : nullptr, 1, 1));
            for (int k = 0; k < 6; k++) w.push_back (pcb (TIME_WAIT, nullptr, 1, 1));
            aktiv (v); tw (w);
        }
        http_no_request_timeouts = 3; http_no_request_aborts = 9;
        ESP.blk_seq.clear (); ESP.heap_seq.clear (); ESP.heap = 6000; ESP.blk = 5000; heap_low_line_take (); esp_uart_overruns = 0;
        std::string z = heapzeile ();
        fall ("C1 Heap-Zeile endet auf tcp=<est>/<fw1>/<fw2>/<cw>/<la>/<tw> nrt=<no_request_timeouts>", z == "- heap free=6000 max=5000 ovr=0 low=5000 tcp=2/1/3/4/5/6 nrt=3", z);
        m = dr ();
        char soll[64]; auto l = m["tcp_now_states"];
        snprintf (soll, sizeof (soll), "tcp=2/1/3/4/5/%s", m["tcp_now_tw"].c_str ());
        fall ("C2 tcp= zaehlt wie device_ready (eine Wahrheit)", z.find (soll) != std::string::npos && l == "[0,0,0,1,2,1,3,4,1,5,0]", std::string (soll) + " / " + l);
        /* laengste Form */
        leeren ();
        std::vector<struct tcp_pcb *> v, w;
        for (int s : { ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, CLOSE_WAIT, LAST_ACK }) for (int k = 0; k < 300; k++) v.push_back (pcb ((enum tcp_state) s, nullptr, 1, 1));
        for (int k = 0; k < 300; k++) w.push_back (pcb (TIME_WAIT, nullptr, 1, 1));
        aktiv (v); tw (w);
        http_no_request_timeouts = 0xFFFF; esp_uart_overruns = 4294967295u; ESP.heap = 4294967295u; ESP.blk = 4294967295u;
        z = heapzeile ();
        fall ("C3 laengste Form vollstaendig und <= 119 Zeichen (Ring = Mitschnitt)",
              z == "- heap free=4294967295 max=4294967295 ovr=4294967295 low=4294967295 tcp=255/255/255/255/255/255 nrt=65535" && z.size () <= 119,
              std::to_string (z.size ()) + ": " + z);
        char b[10]; memset (b, 'x', sizeof (b)); size_t n = tcp_now_tcp_field (b, sizeof (b));
        fall ("C4 tcp_now_tcp_field kappt am Puffer und schliesst ab", n == 9 && b[9] == '\0' && ! strncmp (b, "tcp=255/2", 9), std::to_string (n));
        memset (b, 'x', sizeof (b)); n = tcp_now_tcp_field (b, 0);
        fall ("C4 Puffer der Groesse 0 bleibt unberuehrt", n == 0 && b[0] == 'x', std::to_string (n));
        http_no_request_timeouts = 0; esp_uart_overruns = 0; ESP.heap = 6000; ESP.blk = 5000;
    }
    leeren ();
    return ende ("c72i", BEZEICHNUNG);
}
