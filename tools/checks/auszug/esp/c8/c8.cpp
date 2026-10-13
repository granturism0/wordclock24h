/* Pruefstand c8 (Paket 2026-10-13, E2.2, AKK.1). Echter Code: Ring (ring_code.inc) und Bruecke (l55e_code.inc).
 * Erwartung unabhaengig formuliert: zerlege() unten ist die Regel aus design.md 7, nicht eine Abschrift.
 */
#define PRUEFSTAND_SCHNELLE_TYPEN
#include "pruefstand.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <vector>
#include VARSHDR

static zp_millis fake_ms = 1000;
static zp_millis millis (void) { return fake_ms; }
static void yield (void) { fake_ms++; }
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
#include "ring_code.inc"
char         wifi_ip_address[20];
unsigned int numvars[MAX_NUM_VARIABLES];
uint_fast8_t var_set_parameter (char *) { return 1; }
static void http_send (const char *) {}
static void http_flush (void) {}
static void http_json_error (unsigned int, const char *) {}
void stm_cmd_printf (const char *, ...);
unsigned int rpc (RPC_VARIABLE var);
#include "l55e_code.inc"

static int fails = 0, faelle = 0;
static void expect (bool ok, const std::string & what) { if (! ok) { fails++; printf ("      FEHLER: %s\n", what.c_str ()); } }
static std::string text (int n, int seed) { std::string s; for (int i = 0; i < n; i++) s += (char) ('A' + (i * 7 + seed) % 26); return s; }
static std::vector<std::string> zerlege (const std::string & s)    /* Regel design.md 7 */
{
    std::vector<std::string> r;
    if (s.size () <= 120) { r.push_back (s); return r; }
    r.push_back (s.substr (0, 119) + "~");
    for (size_t p = 119; p < s.size (); p += 119) r.push_back ("~" + s.substr (p, 119));
    return r;
}
static std::vector<std::string> ring (void) { std::vector<std::string> v; for (uint16_t i = 0; i < stm32_log_get_count (); i++) v.push_back (stm32_log_get_line (i)); return v; }
static std::string zusammen (const std::vector<std::string> & v, size_t ab)
{
    std::string s;
    for (size_t i = ab; i < v.size (); i++)
    {
        std::string e = v[i];
        if (i > ab && ! e.empty () && e[0] == '~') e = e.substr (1);
        if (i + 1 < v.size () && v[i + 1].size () && v[i + 1][0] == '~' && e.size () && e.back () == '~') e.pop_back ();
        s += e;
    }
    return s;
}
static void fall_direkt (int n)
{
    faelle++;
    stm32_log_clear ();
    std::string s = text (n, n);
    stm32_log_append (s.c_str ());
    std::vector<std::string> soll = zerlege (s), ist = ring ();
    expect (ist == soll, "direkt " + std::to_string (n) + " Zeichen: " + std::to_string (soll.size ()) + " Eintraege erwartet, bekam " + std::to_string (ist.size ()) + (ist.empty () ? "" : " (erster: " + ist[0].substr (0, 20) + "... Laenge " + std::to_string (ist[0].size ()) + ")"));
    for (auto & e : ist) expect (e.size () <= 120, "Eintrag ueber 120 Zeichen");
    if (n > 0) expect (zusammen (ist, 0) == s, "direkt " + std::to_string (n) + ": zusammengesetzt gleich dem Original");
}
static void fall_bruecke (int n, bool kappen)
{
    faelle++;
    stm32_log_clear ();
    std::string s = text (n, 3);
    size_t o = Serial.out.size ();
    Serial.rx += "LOG " + s + "\r\n";
    stm_bridge_poll ();
    std::vector<std::string> ist = ring ();
    std::string z = zusammen (ist, 0);
    if (! kappen)
        expect (z == s && ist == zerlege (s), "Bruecke " + std::to_string (n) + " Zeichen: ungekappt im Ring, bekam " + std::to_string (z.size ()) + " Zeichen in " + std::to_string (ist.size ()) + " Eintraegen");
    else
        expect (z.size () == 255 && z.substr (0, 254) == s.substr (0, 254) && z.back () == '~', "Bruecke " + std::to_string (n) + " Zeichen: auf 255 gekappt mit '~' am Ende, bekam " + std::to_string (z.size ()));
    expect (Serial.out.size () == o, "LOG-Zeile ohne Antwort an den STM");
}
int main (void)
{
    printf ("c8 (%s)\n", BEZEICHNUNG);
    faelle++; expect (CMD_BUFFER_SIZE == 260, "CMD_BUFFER_SIZE 260, ist " + std::to_string (CMD_BUFFER_SIZE));
    faelle++; expect (sizeof (stm32_log_lines) == 32 * 121, "Ring-RAM unveraendert 32 x 121 = 3872 Byte, ist " + std::to_string (sizeof (stm32_log_lines)));
    for (int n : { 1, 119, 120, 121, 200, 238, 239, 255, 357, 358 }) fall_direkt (n);
    faelle++; stm32_log_clear (); stm32_log_append (""); stm32_log_append (0); expect (stm32_log_get_count () == 0, "leere Zeile und NULL legen nichts ab");
    for (int n : { 119, 120, 121, 255 }) fall_bruecke (n, false);
    fall_bruecke (256, true); fall_bruecke (300, true);
    {   /* Umlauf: 40 Zeilen mit 255 Zeichen, die letzten 32 Eintraege sind die Folge der letzten Zeilen */
        faelle++;
        stm32_log_clear ();
        std::vector<std::string> alle;
        for (int k = 0; k < 40; k++) { std::string s = text (255, k); stm32_log_append (s.c_str ()); for (auto & e : zerlege (s)) alle.push_back (e); }
        std::vector<std::string> ist = ring ();
        std::vector<std::string> soll (alle.end () - 32, alle.end ());
        expect (stm32_log_get_count () == 32 && ist == soll, "Umlauf: die juengsten 32 Eintraege in Reihenfolge");
    }
    printf ("  Faelle: %d\n", faelle);
    printf ("%s  c8 %s: %d Fehler\n", fails ? "FEHL" : "OK  ", BEZEICHNUNG, fails);
    return fails ? 1 : 0;
}
