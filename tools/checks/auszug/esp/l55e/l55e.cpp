/* Pruefstand l55e (Paket 2026-10-13, E2.1, AKL.2): ldr_min_set/ldr_max_set warten auf "RPCR xx r" des STM.
 * Echter Code (gen.py): Bruecke samt var-Zweig, RPCR-Ablage und -Zweig, stm_bridge_poll(), rpc(), die Handler.
 * Attrappe des STM, wie src/main.c (schedule_esp8266_remote_procedure) und var_send_buf() es tun: Auf "R0x" sendet
 * er die Wertmeldung als var-Zeile und wartet bis 3 s auf deren Quittung ("." oder "!v"), DANN "RPCR xx r",
 * unquittiert. Quittiert der ESP nicht, kommt RPCR erst nach Ablauf der 3 s -- also nach der Frist des ESP.
 * Zielbreiten: uint_fast8_t 32 Bit (pruefstand.h), millis() 32 Bit, Deklarationen unsigned long -> uint32_t.
 * Jeder Fall in einem eigenen Prozess (static-Zustand).
 */
#define PRUEFSTAND_SCHNELLE_TYPEN
#include "pruefstand.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include VARSHDR

static zp_millis fake_ms = 1000;
static zp_millis millis (void) { return fake_ms; }
static void stm_tick (void);
static zp_millis t_call = 0;
static void yield (void)
{
    fake_ms++; stm_tick ();
    if ((zp_millis) (fake_ms - t_call) > 100000u) { printf ("      FEHLER: Warteschleife ohne Ende (ueber 100 s)\n"); fflush (stdout); _exit (1); }
}
#define PSTR(s)     (s)
#define snprintf_P  snprintf
#define strcpy_P    strcpy
#define FS(s)       (s)
struct SerialStub {
    std::string out, rx; size_t rpos = 0;
    int available (void) { return (int) (rx.size () - rpos); }
    int read (void) { return rpos < rx.size () ? (unsigned char) rx[rpos++] : -1; }
    void println (const char * s) { out += s; out += "\r\n"; }
    void print (const char * s) { out += s; }
    void flush (void) {}
} Serial;
static std::vector<std::string> logring;
static void stm32_log_append (const char * l) { logring.push_back (l); }
char         wifi_ip_address[20];
unsigned int numvars[MAX_NUM_VARIABLES];
static int reentry = 0, reentry_ok = 1, reentry_calls = 0;
void stm_bridge_poll (void);
uint_fast8_t var_set_parameter (char * p)
{
    if (p[0] == 'N' && strlen (p) == 7)
    {
        char h[3] = { 0, 0, 0 }; unsigned v[3];
        for (int k = 0; k < 3; k++) { h[0] = p[1 + 2 * k]; h[1] = p[2 + 2 * k]; v[k] = (unsigned) strtoul (h, 0, 16); }
        if (v[0] < MAX_NUM_VARIABLES) numvars[v[0]] = v[1] | (v[2] << 8);
    }
    if (reentry)                                    /* verschachtelter Aufruf darf nichts lesen */
    {
        Serial.rx += "LOG verschachtelt\r\n";    /* waehrend der Verarbeitung angekommen */
        size_t r0 = Serial.rpos; reentry_calls++;
        stm_bridge_poll ();
        if (Serial.rpos != r0) reentry_ok = 0;
    }
    return 1;
}
static std::string http_resp;
static const std::string HDR = "HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nCache-Control: no-cache\r\n\r\n";
static void http_send (const char * s) { http_resp += s; }
static void http_flush (void) {}
static void http_json_error (unsigned int code, const char * detail)
{
    char b[200]; snprintf (b, sizeof b, "{\"ok\":false,\"error\":%u,\"detail\":\"%s\"}", code, detail ? detail : ""); http_resp += b;
}
void stm_cmd_printf (const char *, ...);
unsigned int rpc (RPC_VARIABLE var);

#include "l55e_code.inc"

/* ------------------------------------------------------------------ Attrappe des STM */
struct StmPlan {
    std::vector<std::string> vars;      /* quittungspflichtig, eine nach der anderen */
    std::vector<std::string> extra;     /* unquittiert vor RPCR (verstuemmelte Zeilen, LOG) */
    std::string rpcr;                   /* leer = alter STM, keine Rueckmeldung */
    zp_millis delay = 20;               /* Messung im STM */
};
static StmPlan plan;
static int stm_state = 0;               /* 0 wartet auf R, 1 var gesendet, 2 fertig */
static size_t stm_seen = 0, var_i = 0, ack_from = 0;
static zp_millis t_cmd = 0, t_var = 0, ack_max = 0;
static int acks = 0, var_timeouts = 0;
static size_t out_at_rpcr = std::string::npos;
static zp_millis t_rpcr = 0;
static void push (const std::string & l) { Serial.rx += l + "\r\n"; }
static void send_rpcr (void)
{
    for (auto & x : plan.extra) push (x);
    if (! plan.rpcr.empty ()) { push (plan.rpcr); out_at_rpcr = Serial.out.size (); t_rpcr = fake_ms; }
    stm_state = 2;
}
static void stm_next_var (void)
{
    if (var_i < plan.vars.size ()) { push (plan.vars[var_i]); t_var = fake_ms; ack_from = Serial.out.size (); stm_state = 1; }
    else send_rpcr ();
}
static void stm_tick (void)
{
    if (stm_state == 0)
    {
        size_t p = Serial.out.find ("CMD R", stm_seen);
        if (p != std::string::npos) { stm_seen = p + 5; t_cmd = fake_ms; stm_state = 3; }
    }
    if (stm_state == 3 && fake_ms - t_cmd >= plan.delay) { var_i = 0; stm_next_var (); }
    if (stm_state == 1)
    {
        std::string n = Serial.out.substr (ack_from);
        if (n.find ("\r\n.\r\n") != std::string::npos || n.rfind (".\r\n", 0) == 0 || n.find ("!v\r\n") != std::string::npos)
        {
            acks++; if (fake_ms - t_var > ack_max) ack_max = fake_ms - t_var;
            var_i++; stm_next_var ();
        }
        else if (fake_ms - t_var >= 3000) { var_timeouts++; var_i++; stm_next_var (); }
    }
}
/* ------------------------------------------------------------------ Faelle */
static int fails = 0;
static std::string esc (const std::string & s) { std::string r; for (char c : s) { if (c == '\r') r += "\\r"; else if (c == '\n') r += "\\n"; else r += c; } return r; }
static void expect (bool ok, const char * what, const std::string & got = "")
{
    if (! ok) { fails++; printf ("      FEHLER: %s%s%s\n", what, got.empty () ? "" : " -- bekam: ", esc (got).c_str ()); }
}
static std::string varline (int idx, unsigned v) { char b[32]; snprintf (b, sizeof b, "var N%02x%02x%02x", idx, v & 0xFF, (v >> 8) & 0xFF); return b; }
static int call (int which)           /* 0 = min, 1 = max; Rueckgabe: Dauer in ms */
{
    http_resp.clear (); stm_state = 0; stm_seen = Serial.out.size (); zp_millis t0 = fake_ms; t_call = fake_ms;
    if (which == 0) http_api_ldr_min_set (); else http_api_ldr_max_set ();
    return (int) (zp_millis) (fake_ms - t0);
}
static bool log_has (const char * n) { for (auto & l : logring) if (l.find (n) != std::string::npos) return true; return false; }
static int count (const std::string & s, const char * n) { int k = 0; size_t p = 0; while ((p = s.find (n, p)) != std::string::npos) { k++; p++; } return k; }
static void kein_echo (void)
{
    expect (out_at_rpcr != std::string::npos, "RPCR wurde nicht gesendet (STM wartete auf eine Quittung, die nicht kam?)");
    if (out_at_rpcr != std::string::npos)
        expect (Serial.out.size () == out_at_rpcr, "nach RPCR sendet der ESP nichts an den STM (F2: kein Punkt, kein ACK)", Serial.out.substr (out_at_rpcr));
}

static void f_min_uebernommen (void)
{
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 1234) }; plan.rpcr = "RPCR 00 0";
    int d = call (0);
    expect (http_resp == HDR + "{\"ok\":true,\"value\":1234}", "Minimum uebernommen => {\"ok\":true,\"value\":1234}", http_resp);
    expect (acks == 1 && var_timeouts == 0, "die var-Zeile wurde waehrend des Wartens quittiert");
    expect (ack_max < 100, "Quittung innerhalb 100 ms", std::to_string (ack_max));
    expect (d < 200, "Antwort ohne die Frist abzuwarten", std::to_string (d));
    expect (count (Serial.out, "CMD R00\r\n") == 1, "genau ein RPC R00 an den STM", Serial.out);
    kein_echo ();
}
static void f_max_uebernommen (void)
{
    plan.vars = { varline (LDR_MAX_VALUE_NUM_VAR, 4000) }; plan.rpcr = "RPCR 01 0";
    call (1);
    expect (http_resp == HDR + "{\"ok\":true,\"value\":4000}", "Maximum uebernommen => {\"ok\":true,\"value\":4000}", http_resp);
    expect (count (Serial.out, "CMD R01\r\n") == 1, "genau ein RPC R01", Serial.out);
    kein_echo ();
}
static void f_gleicher_wert (void)        /* unveraendert gemessen heisst trotzdem uebernommen (L417) */
{
    numvars[LDR_MIN_VALUE_NUM_VAR] = 77;
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 77) }; plan.rpcr = "RPCR 00 0";
    call (0);
    expect (http_resp == HDR + "{\"ok\":true,\"value\":77}", "gleicher Wert gemessen => ok", http_resp);
}
static void f_abgewiesen (void)
{
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 10) }; plan.rpcr = "RPCR 00 1";
    call (0);
    expect (http_resp.find ("\"ok\":false,\"error\":10,") != std::string::npos, "r=1 => Kennung 10", http_resp);
    kein_echo ();
}
static void f_max_abgewiesen (void)
{
    plan.vars = { varline (LDR_MAX_VALUE_NUM_VAR, 10) }; plan.rpcr = "RPCR 01 1";
    call (1);
    expect (http_resp.find ("\"error\":10,") != std::string::npos, "Maximum r=1 => Kennung 10", http_resp);
}
static void f_messung_fehl (void)
{
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 10) }; plan.rpcr = "RPCR 00 2";
    call (0);
    expect (http_resp.find ("\"error\":7,") != std::string::npos, "r=2 => Kennung 7", http_resp);
    kein_echo ();
}
static void f_keine_antwort (void)          /* alter STM: Wertmeldung ja, RPCR nie */
{
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 10) }; plan.rpcr = "";
    int d = call (0);
    expect (http_resp.find ("\"error\":11,") != std::string::npos, "keine Antwort => Kennung 11", http_resp);
    expect (d >= 2000 && d <= 2010, "Frist 2'000 ms", std::to_string (d));
    expect (acks == 1, "auch ohne RPCR wird die var-Zeile quittiert");
    expect (count (Serial.out, "- rpc keine Antwort #1 idx=00\r\n") == 1, "Mitschnitt: \"- rpc keine Antwort #1 idx=00\"", Serial.out);
    expect (log_has ("- rpc keine Antwort #1 idx=00"), "Ring: \"- rpc keine Antwort #1 idx=00\"");
    /* gedrosselt: 10 s spaeter keine zweite Zeile, nach 60 s die dritte mit #3 */
    fake_ms += 10000; size_t lr = logring.size (); call (0);
    expect (http_resp.find ("\"error\":11,") != std::string::npos, "zweites Mal => Kennung 11", http_resp);
    expect (logring.size () == lr && count (Serial.out, "- rpc keine Antwort") == 1, "zweite Zeile innerhalb einer Minute gedrosselt");
    fake_ms += 60000; call (0);
    expect (log_has ("- rpc keine Antwort #3 idx=00") && count (Serial.out, "- rpc keine Antwort #3 idx=00\r\n") == 1, "nach einer Minute: \"#3\" (zaehlt alle)");
    fake_ms += 60000; plan.vars = {}; call (1);
    expect (log_has ("- rpc keine Antwort #4 idx=01"), "Maximum: \"idx=01\"");
}
static void f_verstuemmelt (void)
{
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 10) }; plan.rpcr = "";
    plan.extra = { "RPCR 00", "RPCR 0 0", "RPCR 00 3", "RPCR zz 0", "RPCR 00 0x", "RPCR 00  0", "RPCR 02 0", "RPCR 0g 0", "RPCR 01 0", "RPCR 00-0" };
    size_t o0 = Serial.out.size ();
    plan.rpcr = "LOG dazwischen";           /* als Marke fuer das Echo */
    call (0);
    expect (http_resp.find ("\"error\":11,") != std::string::npos, "nur verstuemmelte (oder fremder Index) => Kennung 11", http_resp);
    std::string n = Serial.out.substr (o0);
    expect (count (n, ".\r\n") == 1, "kein Echo auf die zehn RPCR-Zeilen (genau ein Punkt fuer die var-Zeile)", n);
    expect (log_has ("dazwischen"), "LOG-Zeile dazwischen erreicht den Ring");
    /* N3: Mitschnitt UND Ring, die erste sofort, dann hoechstens eine je Minute; #n zaehlt alle (neun verstuemmelt,
     * "RPCR 01 0" ist gueltig, nur fremder Index) */
    int v = 0; for (auto & l : logring) if (l.rfind ("- rpc Antwort verstuemmelt", 0) == 0) v++;
    expect (v == 1 && log_has ("- rpc Antwort verstuemmelt #1"), "Ring: genau eine Zeile \"- rpc Antwort verstuemmelt #1\" (gedrosselt)", std::to_string (v));
    expect (count (n, "- rpc Antwort verstuemmelt #1\r\n") == 1 && count (n, "- rpc Antwort verstuemmelt") == 1, "Mitschnitt: genau \"- rpc Antwort verstuemmelt #1\"", n);
    /* die erste Zeile fiel kurz nach dem Start des Wartens, das 2 s lief */
    fake_ms += 57000; size_t o1 = Serial.out.size (); push ("RPCR 0"); stm_bridge_poll ();
    expect (Serial.out.size () == o1, "rund 59 s nach der ersten: gedrosselt, nichts auf der Bruecke", Serial.out.substr (o1));
    fake_ms += 3000; push ("RPCR 0"); stm_bridge_poll ();
    expect (count (Serial.out.substr (o1), "- rpc Antwort verstuemmelt #11\r\n") == 1 && log_has ("- rpc Antwort verstuemmelt #11"), "rund 62 s nach der ersten: \"#11\" in Mitschnitt und Ring", Serial.out.substr (o1));
    fake_ms = 0xFFFFFFFFu - 100u; push ("RPCR 0"); stm_bridge_poll (); size_t o2 = Serial.out.size ();
    fake_ms += 30000; push ("RPCR 0"); stm_bridge_poll ();
    expect (Serial.out.size () == o2, "Wortrand: 30 s nach der Meldung gedrosselt", Serial.out.substr (o2));
}
static void f_mehrere_var (void)
{
    plan.vars = { "var N0201ff", varline (LDR_MIN_VALUE_NUM_VAR, 300), "var N0301ff" }; plan.rpcr = "RPCR 00 0";
    call (0);
    expect (acks == 3 && var_timeouts == 0, "drei var-Zeilen waehrend des Wartens quittiert", std::to_string (acks));
    expect (http_resp == HDR + "{\"ok\":true,\"value\":300}", "danach ok mit Wert", http_resp);
    kein_echo ();
}
static void f_wortrand (void)
{
    fake_ms = 0xFFFFFFFFu - 500u;
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 5) }; plan.delay = 1000; plan.rpcr = "RPCR 00 0";
    call (0);
    expect (http_resp == HDR + "{\"ok\":true,\"value\":5}", "Antwort nach 1 s ueber den Wortrand => ok", http_resp);
    fake_ms = 0xFFFFFFFFu - 500u; plan.rpcr = "";
    int d = call (0);
    expect (d >= 2000 && d <= 2010, "Frist ueber den Wortrand 2'000 ms, nicht sofort", std::to_string (d));
}
static void f_spaet_aber_in_frist (void)
{
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 9) }; plan.delay = 1900; plan.rpcr = "RPCR 00 0";
    call (0);
    expect (http_resp == HDR + "{\"ok\":true,\"value\":9}", "Antwort nach 1'900 ms => ok", http_resp);
}
static void f_sperre (void)
{
    reentry = 1;
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 42) }; plan.rpcr = "RPCR 00 0";
    plan.extra = { "LOG a", "LOG b" };
    call (0);
    expect (reentry_calls >= 1, "verschachtelter Aufruf hat stattgefunden");
    expect (reentry_ok, "verschachtelter stm_bridge_poll () liest nichts");
    expect (log_has ("verschachtelt"), "die waehrenddessen angekommene Zeile liest der aeussere Aufruf");
    expect (http_resp == HDR + "{\"ok\":true,\"value\":42}", "aeusserer Aufruf arbeitet weiter", http_resp);
    reentry = 0; size_t r = Serial.rpos; push ("LOG c"); stm_bridge_poll ();
    expect (Serial.rpos == r + 7 && log_has ("c"), "Sperre nach dem Aufruf wieder frei");
}
static void f_im_loop (void)               /* RPCR ohne Wartenden (Legacy-Knopf, spaete Antwort): still verworfen */
{
    push ("RPCR 00 0"); push ("RPCR 01 1");
    size_t o = Serial.out.size (); uint_fast8_t r0 = 9, r1 = 9;
    uint_fast8_t s0 = stm_rpcr_get (0, 0), s1 = stm_rpcr_get (1, 0);
    stm_bridge_poll ();
    expect (Serial.out.size () == o, "kein Echo auf RPCR im loop ()", Serial.out.substr (o));
    expect (stm_rpcr_get (0, &r0) != s0 && r0 == 0 && stm_rpcr_get (1, &r1) != s1 && r1 == 1, "Ablage je Index");
    uint_fast8_t rx; expect (stm_rpcr_get (2, &rx) == 0 && rx == 0xFF, "Index ausserhalb => 0, Ergebnis 0xFF");
}
static void f_lange_zeile_dazwischen (void)
{
    plan.vars = { varline (LDR_MIN_VALUE_NUM_VAR, 8) }; plan.rpcr = "RPCR 00 0";
    plan.extra = { std::string ("LOG ") + std::string (300, 'x') };
    call (0);
    expect (http_resp == HDR + "{\"ok\":true,\"value\":8}", "ueberlange LOG-Zeile vor RPCR stoert nicht", http_resp);
}

typedef void (*Fall) (void);
int main (void)
{
    printf ("l55e (%s)\n", BEZEICHNUNG);
    struct { const char * n; Fall f; } F[] = {
        { "Minimum uebernommen", f_min_uebernommen }, { "Maximum uebernommen", f_max_uebernommen },
        { "gleicher Wert", f_gleicher_wert }, { "abgewiesen", f_abgewiesen }, { "Maximum abgewiesen", f_max_abgewiesen },
        { "Messung fehlgeschlagen", f_messung_fehl }, { "keine Antwort, Drossel", f_keine_antwort },
        { "verstuemmelt", f_verstuemmelt }, { "mehrere var-Zeilen", f_mehrere_var }, { "Wortrand", f_wortrand },
        { "spaet in der Frist", f_spaet_aber_in_frist }, { "Sperre", f_sperre }, { "im loop", f_im_loop },
        { "lange Zeile dazwischen", f_lange_zeile_dazwischen },
    };
    int bad = 0, n = (int) (sizeof F / sizeof F[0]);
    for (int i = 0; i < n; i++)
    {
        fflush (stdout);
        pid_t p = fork ();
        if (p == 0) { F[i].f (); fflush (stdout); _exit (fails ? 1 : 0); }
        int st = 0; waitpid (p, &st, 0);
        int ok = WIFEXITED (st) && WEXITSTATUS (st) == 0;
        if (! ok) { bad++; printf ("    FEHL  %s%s\n", F[i].n, WIFSIGNALED (st) ? " (Signal)" : ""); }
        else printf ("    ok    %s\n", F[i].n);
    }
    printf ("  Faelle: %d\n", n);
    printf ("%s  l55e %s: %d von %d Faellen fehlgeschlagen\n", bad ? "FEHL" : "OK  ", BEZEICHNUNG, bad, n);
    return bad ? 1 : 0;
}
